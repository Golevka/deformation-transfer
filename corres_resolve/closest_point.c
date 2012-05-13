#include <stdlib.h>
#include <math.h>

#include "closest_point.h"



/* Allocate space for spatial join object, this function won't perform any 
   closest point search so you have to call __dt_ResolveModelSpatialJoin()
   later. */
void __dt_CreateSpatialJoinList(
    const dtMeshModel *source_model, __dt_SpatialJoinList *spjlist)
{
    /* there should be n_freevertex effective entries in the list, however, 
       allocating for n_vertex entries would be enough and not so wasteful. */
    spjlist->list_length = source_model->n_vertex;
    spjlist->i_target_vertex = (dt_index_type*)__dt_malloc(
        (size_t)spjlist->list_length * sizeof(dt_index_type));
}

void __dt_DestroySpatialJoinList(__dt_SpatialJoinList *spjlist) {
    free(spjlist->i_target_vertex);
}


/* calculate dot(v1,v2) */
static dt_real_type __vector_dot(const dtVector *v1, const dtVector *v2)
{
    dt_real_type x1 = v1->x, x2 = v2->x;
    dt_real_type y1 = v1->y, y2 = v2->y;
    dt_real_type z1 = v1->z, z2 = v2->z;
    
    return  x1*x2 + y1*y2 + z1*z2;
}


/* Build a 3d tree with vertices of specified model for fast nearest point 
   search, it is quite handy in correspondence phase 2 - closest point 
   iteration. */
__3dTree __dt_Build3DTree_Vertex(const dtMeshModel *model)
{
    __3dTree  tree;
    __3dtree_Exemplar *exset;

    dt_index_type i_vertex = 0;

    /* 3d tree works with exemplar set, we need to construct exsets for target 
       model */
    exset = (__3dtree_Exemplar*)__dt_malloc(
        (size_t)model->n_vertex * sizeof(__3dtree_Exemplar));

    for ( ; i_vertex < model->n_vertex; i_vertex++)
    {
        exset[i_vertex].pt[0] = model->vertex[i_vertex].x;
        exset[i_vertex].pt[1] = model->vertex[i_vertex].y;
        exset[i_vertex].pt[2] = model->vertex[i_vertex].z;
        exset[i_vertex].id = i_vertex;
    }

    /* create the 3d tree */
    __3dtree_Create3DTree(exset, exset + model->n_vertex, &tree);

    /* free the exset allocated for building the tree */
    free(exset);
    return tree;
}


/* variables for currying */
static const dtVector      *__lambda_src_norm;
static const dtVector      *__lambda_target_normvec;
static const dt_index_type *__lambda_tgt_inorm_list;

/* compare source vertex normal with target vertex normal and a difference in 
   orientation of less than 90-deg indicates a valid point */
static int __norm_condition(__3dtree_Node *node)
{
    const dtVector *tgt_norm = 
        __lambda_target_normvec + __lambda_tgt_inorm_list[node->id];

    return (__vector_dot(__lambda_src_norm, tgt_norm) > 0);
}

/* Resolve the spatial join between source model and target model using a 
   3d tree of the target model, this routine is way more effiecient than
   the brute force one. */
void __dt_ResolveModelSpatialJoin(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const dt_index_type *src_inorm_list,
    const dt_index_type *tgt_inorm_list,
    __3dTree tree_tgt, __dt_SpatialJoinList *spjlist)
{
    __3dtree_Node *res_node;
    dt_real_type  x0[3], dist_sq;
    dt_index_type i_vertex;

    /* currying */
    __lambda_target_normvec = target_model->normvec;
    __lambda_tgt_inorm_list = tgt_inorm_list;

    /* nearest point search */
    for (i_vertex = 0; i_vertex < source_model->n_vertex; i_vertex++)
    {
        /* convert reference point to 3D array form */
        x0[0] = source_model->vertex[i_vertex].x;
        x0[1] = source_model->vertex[i_vertex].y;
        x0[2] = source_model->vertex[i_vertex].z;

        /* currying vertex normal vectors for orientation condition */
        __lambda_src_norm = 
            source_model->normvec + src_inorm_list[i_vertex];
        
        /* search for the nearest valid vertex */
        __3dtree_NearestPoint_Cond(
            tree_tgt, x0, &res_node, &dist_sq, __norm_condition);

        /* append result vertex index to the spatial join list */
        spjlist->i_target_vertex[i_vertex] = res_node->id;
    }
}


/* calculate squared distance between v1 and v2 */
static dt_real_type __squared_distance(const dtVertex *v1, const dtVertex *v2)
{
    dt_real_type  dx = (v1->x - v2->x);
    dt_real_type  dy = (v1->y - v2->y);
    dt_real_type  dz = (v1->z - v2->z);

    return dx*dx + dy*dy + dz*dz;
}

/* Resolve the spatial join between source model and target model, this routine
   uses a naive brute force O(m*n) method, which might be slow for even small
   or medium size models. KD tree might be much faster in 3D case. */
void __dt_ResolveModelSpatialJoin_BruteForce(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const dt_index_type *src_inorm_list,
    const dt_index_type *tgt_inorm_list,
    __dt_SpatialJoinList *spjlist)
{
    dt_index_type i_src_vertex, i_tgt_vertex;
    dtVertex *src_vertex, *tgt_vertex;
    dtVector *src_norm,   *tgt_norm;

    dt_real_type  dist, min_dist;
    dt_index_type i_tgt_closest;

    for (i_src_vertex = 0; i_src_vertex < source_model->n_vertex; i_src_vertex++)
    {
        src_vertex = source_model->vertex + i_src_vertex;
        src_norm   = source_model->normvec + src_inorm_list[i_src_vertex];

        i_tgt_closest = -1;

        for (i_tgt_vertex = 0; i_tgt_vertex < target_model->n_vertex; i_tgt_vertex++)
        {
            tgt_vertex = target_model->vertex + i_tgt_vertex;
            tgt_norm   = target_model->normvec + tgt_inorm_list[i_tgt_vertex];

            if (__vector_dot(src_norm, tgt_norm) > 0) /* direction constraint */
            {
                dist = __squared_distance(src_vertex, tgt_vertex);
                if (i_tgt_closest == -1 || dist < min_dist)
                {
                    min_dist = dist;
                    i_tgt_closest = i_tgt_vertex;
                }
            }
        }

        spjlist->i_target_vertex[i_src_vertex] = i_tgt_closest;
    }
}
