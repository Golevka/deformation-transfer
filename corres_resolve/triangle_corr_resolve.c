#include <stdlib.h>
#include <math.h>  /* for sqrt */

#include "3dtree.h"
#include "triangle_corr.h"
#include "surface_matrix.h"



/* calculate the coordinate of the centroid of specified triangle */
static void __calculate_triangle_centroid(
    const dtMeshModel *model, dt_index_type i_triangle, dt_real_type *centroid)
{
    /* get the coordinates of vertices in the triangle */
    dtTriangle *triangle = model->triangle + i_triangle;
    dtVertex 
        *v0 = model->vertex + triangle->i_vertex[0],
        *v1 = model->vertex + triangle->i_vertex[1],
        *v2 = model->vertex + triangle->i_vertex[2];

    /* cartesian coordinates of centroid are the means of the coordinates of
       the three vertices */
    centroid[0] = (v0->x + v1->x + v2->x) / 3;
    centroid[1] = (v0->y + v1->y + v2->y) / 3;
    centroid[2] = (v0->z + v1->z + v2->z) / 3;
}

static __3dTree __create_centroid_tree(const dtMeshModel *model)
{
    __3dTree centroid_tree;
    dt_size_type tree_size = model->n_triangle;

    /* create the exset of the tree, id of each centroid are named with the
       index of corresponded triangle  */
    __3dtree_Exemplar *exset = (__3dtree_Exemplar*)__dt_malloc(
        (size_t)tree_size * sizeof(__3dtree_Exemplar));

    dt_index_type i_tri = 0;
    for ( ; i_tri < tree_size; i_tri++)
    {
        __calculate_triangle_centroid(model, i_tri, exset[i_tri].pt);
        exset[i_tri].id = i_tri;
    }

    __3dtree_Create3DTree(exset, exset + tree_size, &centroid_tree);

    free(exset);
    return centroid_tree;
}



/* calculate dot(v1,v2) */
static dt_real_type __vector_dot(const dtVector *v1, const dtVector *v2)
{
    dt_real_type x1 = v1->x, x2 = v2->x;
    dt_real_type y1 = v1->y, y2 = v2->y;
    dt_real_type z1 = v1->z, z2 = v2->z;
    
    return  x1*x2 + y1*y2 + z1*z2;
}


/* variables for currying */
static const dtMeshModel *__lambda_model;
static       dtVector     __lambda_norm;

/* the angle between the normals of the two triangles should be less than 90-
   deg, this compatibility test prevents two nearby triangles with disparate
   orientation from entering the correspondence. */
static int __norm_condition(__3dtree_Node *node)
{
    dtVector norm1 = 
        __dt_CalculateTriangleUnitNorm(__lambda_model, node->id);

    return  (__vector_dot(&__lambda_norm, &norm1) > 0);
}


/* Resolving triangle correspondence by comparing the centroids of the deformed
   source and target triangles. Two triangles are compatible if their centroids
   are within a certain threshold of each other and the angle between their 
   normals is less than 90-deg.
*/
void __dt_ResolveTriangleCorres(
    const dtMeshModel *deformed_source, const dtMeshModel *target, 
    dt_real_type threshold,
    __dt_TriangleCorrsList *tclist)
{
    __3dTree centroid_tree = __create_centroid_tree(deformed_source);

    /* n_triangle buffer space is large enough, though it might waste a lot
       of memory space, at least it would never overflow. */
    __3dtree_Node **result = (__3dtree_Node**)__dt_malloc(
        (size_t)deformed_source->n_triangle * sizeof(__3dtree_Node*));

    dt_real_type *dist_sq = (dt_real_type*)__dt_malloc(
        (size_t)deformed_source->n_triangle * sizeof(dt_real_type));
        

    dt_real_type  x0[3];  /* centroid of triangle on target model */
    dt_size_type  n_result;
    dt_index_type i_tri = 0, i_entry;

    __dt_TriangleCorrsEntry entry;
    __lambda_model = deformed_source;

    for ( ; i_tri < target->n_triangle; i_tri++)
    {
        __calculate_triangle_centroid(target, i_tri, x0);
        __lambda_norm = __dt_CalculateTriangleUnitNorm(target, i_tri);

        n_result = __3dtree_RangeSearch_Cond(
            centroid_tree, x0, threshold, result, dist_sq, __norm_condition);

        /* append all found entries to the triangle corrs list */
        for (i_entry = 0 ; i_entry < n_result; i_entry++)
        {
            entry.i_src_triangle = result[i_entry]->id;
            entry.i_tgt_triangle = i_tri;
            entry.dist_sq = dist_sq[i_entry];
            __dt_AppendTriangleCorrsEntry(tclist, &entry);
        }
    }

    __3dtree_Destroy3DTree(centroid_tree);
    free(result); free(dist_sq);
}


#define __DT_LARGER(x,y)  ((x)>(y)?(x):(y))
#define __DT_SMALLER(x,y) ((x)<(y)?(x):(y))


/* Determine the threshold of triangle correspondence searching */
static dt_real_type __select_triangle_corrs_threshold(const dtMeshModel *model);

/* Easy to use version of __dt_ResolveTriangleCorres, users don't need to pick
   a threshold by hand, the threshold is estimated by a higher level process.
*/
void __dt_ResolveTriangleCorres_e(
    const dtMeshModel *deformed_source, const dtMeshModel *target, 
    __dt_TriangleCorrsList *tclist)
{
    dt_real_type 
        threshold_src = __select_triangle_corrs_threshold(deformed_source),
        threshold_tgt = __select_triangle_corrs_threshold(target);

    dt_real_type threshold = __DT_LARGER(threshold_src, threshold_tgt);

    __dt_ResolveTriangleCorres(deformed_source, target, threshold, tclist);
}


/* Determine the threshold of triangle correspondence searching with the surface
   area of the bounding box and number of triangle units in the model: 

       threshold = sqrt(4*surface_area / n_triangle)
*/
static dt_real_type __select_triangle_corrs_threshold(const dtMeshModel *model)
{
    dt_index_type iv = 1;
    dt_real_type x, y, z, dx, dy, dz,
        x_max = model->vertex[0].x, x_min = model->vertex[0].x, 
        y_max = model->vertex[0].y, y_min = model->vertex[0].y, 
        z_max = model->vertex[0].z, z_min = model->vertex[0].z;

    /* find the bounding box of the model */
    for ( ; iv < model->n_vertex; iv++)
    {
        x = model->vertex[iv].x;
        y = model->vertex[iv].y;
        z = model->vertex[iv].z;

        x_max = __DT_LARGER(x, x_max);  x_min = __DT_SMALLER(x, x_min);
        y_max = __DT_LARGER(y, y_max);  y_min = __DT_SMALLER(y, y_min);
        z_max = __DT_LARGER(z, z_max);  z_min = __DT_SMALLER(z, z_min);
    }

    /* width, height and depth of model's bounding box */
    dx = x_max - x_min;
    dy = y_max - y_min;
    dz = z_max - z_min;

    return sqrt(4 * (dx*dy + dy*dz + dx*dz) / model->n_triangle);
}

#undef __DT_LARGER
#undef __DT_SMALLER
