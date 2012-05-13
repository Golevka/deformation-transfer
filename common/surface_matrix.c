#include <stdlib.h>
#include <math.h>
#include "surface_matrix.h"



/* scale the vector by the reciprocal of the square root of its length while 
   keeping its direction unchanged */
static void __3dvector_sqrt_norm(dt_real_type *v)
{
    dt_real_type sqrt_norm = 1.0 / pow(v[0]*v[0] + v[1]*v[1] + v[2]*v[2], 0.25);
    v[0] = v[0] * sqrt_norm;
    v[1] = v[1] * sqrt_norm;
    v[2] = v[2] * sqrt_norm;
}

/* calculate the cross product of v1 and v2, the result is stored in v3 */
static void __3dvector_cross_product(
    const dt_real_type *v1, const dt_real_type *v2, dt_real_type *v3)
{
    v3[0]  =  v1[1] * v2[2] - v1[2] * v2[1];
    v3[1]  =  v1[2] * v2[0] - v1[0] * v2[2];
    v3[2]  =  v1[0] * v2[1] - v1[1] * v2[0];
}

/* calculate the phantom vertex of triangle
       v4 = sqrt normalized (v2-v1) x (v3-v1) 
*/
static void __calculate_phantom_vertex(
    const dt_real_type *v1, const dt_real_type *v2, dt_real_type *v3)
{
    __3dvector_cross_product(v1, v2, v3);   /* v3 = v1 x v2 */
    __3dvector_sqrt_norm(v3);    /* replace v3 with v4 = sqrtnorm(v3) */
}

/* Calculate unnormalized normal vector of specified triangle unit */
dtVector __dt_CalculateTriangleUnitNorm(
    const dtMeshModel *model, dt_index_type i_triangle)
{
    dtVector   ret;
    dtTriangle *surf = model->triangle + i_triangle;
    dtVertex   *v_l, *v_r = model->vertex + surf->i_vertex[0];

    dt_real_type u[3], v[3];  /* u = v1 - v0, v = v2 - v0 */

    v_l = model->vertex + surf->i_vertex[1];
    u[0] = v_l->x - v_r->x;
    u[1] = v_l->y - v_r->y;
    u[2] = v_l->z - v_r->z;

    v_l = model->vertex + surf->i_vertex[2];
    v[0] = v_l->x - v_r->x;
    v[1] = v_l->y - v_r->y;
    v[2] = v_l->z - v_r->z;

    __3dvector_cross_product(u, v, &(ret.x) /* a bad hack */);
    return ret;
}

/* Each triangle unit has a unique corresponding matrix: assume that the 
   vertices made up of this triangle unit is v1, v2 and v3, then its 
   corresponding surface matrix should be:

       V = [v2-v1, v3-v1, v4],
           where v4 = sqrt normalized (v2-v1) x (v3-v1)

   This function calculates the surface matrix of triangle unit indexed 
   i_triangle in model, the result is written to the last parameter V.
*/
void __dt_CalculateTriangleUnitMatrix(
    const dtMeshModel *model, dt_index_type i_triangle, dtMatrix3x3 V)
{
    dtTriangle *surf = model->triangle + i_triangle;
    dt_real_type v2_1[3], v3_1[3], v4[3];   /* v2 - v1, v3 - v1, v4 */
    dtVertex *v_l, *v_r;

    /* first column: v2 - v1 */
    v_l = model->vertex + surf->i_vertex[1];
    v_r = model->vertex + surf->i_vertex[0];
    V[0][0] = v2_1[0] = v_l->x - v_r->x;
    V[1][0] = v2_1[1] = v_l->y - v_r->y;
    V[2][0] = v2_1[2] = v_l->z - v_r->z;

    /* second column: v3 - v1 */
    v_l = model->vertex + surf->i_vertex[2];
    V[0][1] = v3_1[0] = v_l->x - v_r->x;
    V[1][1] = v3_1[1] = v_l->y - v_r->y;
    V[2][1] = v3_1[2] = v_l->z - v_r->z;

    /* third column: v4 */
    __calculate_phantom_vertex(v2_1, v3_1, v4);
    V[0][2] = v4[0];
    V[1][2] = v4[1];
    V[2][2] = v4[2];
}


/* Initialize the matrix list with inverse of surface matrices of all triangle
   units in the specified model */
void __dt_InitializeSurfaceInvVList(
    const dtMeshModel *model, __dt_SurfaceInvVList *sinvlist)
{
    dt_index_type i_surf = 0;  /* looping index for triangle units */

    /* initialize sinvlist and allocate memory space for it */
    sinvlist->list_length = model->n_triangle;
    sinvlist->inV = (dtMatrix3x3*)__dt_malloc(
        (size_t)sinvlist->list_length * sizeof(dtMatrix3x3));

    /* calculate the surface matrix and its inverse of each triangle unit 
       in the model and save them into sinvlist. */
    for ( ; i_surf < model->n_triangle; i_surf++)
    {
        __dt_CalculateTriangleUnitMatrix(model, i_surf, sinvlist->inV[i_surf]);
        __dt_InverseMatrix3x3(sinvlist->inV[i_surf]);
    }
}


/* Release memory spaces allocated for sinvlist */
void __dt_DestroySurfaceInvVList(__dt_SurfaceInvVList *sinvlist) {
    free(sinvlist->inV);
}
