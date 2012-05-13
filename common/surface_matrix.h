#ifndef __DT_SURFACE_MATRIX_HEADER__
#define __DT_SURFACE_MATRIX_HEADER__


#include "matrix3x3.h"


/* Each triangle unit has a unique corresponding matrix: assume that the 
   vertices made up of this triangle unit is v1, v2 and v3, then its 
   corresponding surface matrix V is defined as:

   V = [v2-v1, v3-v1, v4]   where v4 = sqrt normalized (v2-v1) x (v3-v1)

   we see that we can derive an unique v4 from each triangle (v1, v2, v3), so 
   we name v4 the "phantom vertex" of the triangle unit.
*/


/* Calculate unnormalized normal vector of specified triangle unit, notice that
   we do not guarantee the calculated normal to be a unit vector.  */
dtVector __dt_CalculateTriangleUnitNorm(
    const dtMeshModel *model, dt_index_type i_triangle);


/* Calculate the surface matrix of triangle unit indexed i_triangle in model,
   the result is written to parameter V. */
void __dt_CalculateTriangleUnitMatrix(
    const dtMeshModel *model, dt_index_type i_triangle, dtMatrix3x3 V);


/* A list containing INVERSE of surface matrices of all triangle units in a 
   mesh model. Precalculating these matrices all at once would eliminate lots 
   of redundant calculations in the coming large equation construction phases.
*/
typedef struct __dt_SurfaceInvVList_struct
{
    dt_index_type  list_length;   /* number of inverse matrix entries, should 
                                     be equal to the number of triangle units
                                     in the mesh model */
    dtMatrix3x3   *inV;           /* a list of inverse matrices of triangle 
                                     surface matrices */
} __dt_SurfaceInvVList;


/* Initialize the matrix list with inverse of surface matrices of all triangle
   units in the specified model */
void __dt_InitializeSurfaceInvVList(
    const dtMeshModel *model, __dt_SurfaceInvVList *sinvlist);

/* Release memory spaces allocated for sinvlist */
void __dt_DestroySurfaceInvVList(__dt_SurfaceInvVList *sinvlist);



#endif /* __DT_SURFACE_MATRIX_HEADER__ */
