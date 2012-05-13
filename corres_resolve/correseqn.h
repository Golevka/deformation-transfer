#ifndef __DT_CORRESPONDENCE_EQUATION_HEADER__
#define __DT_CORRESPONDENCE_EQUATION_HEADER__


/* The correspondence system solves a minimization problem to deform the source
   mesh into the target mesh. */


#include "mesh_model.h"
#include "adjacent.h"
#include "constraint.h"
#include "vertex_info.h"
#include "surface_matrix.h"
#include "closest_point.h"


/* Elementary term is a 9x4 matrix and a 9d vector to represent an equation on 
   a single triangle unit, elementary term can be intergrated to the overall
   large linear system with __dt_AppendElementaryTermToLinearSystem() to make
   the equation to make contribution to the shape of the entire deformed model.
*/
void __dt_CalculateElementaryTerm(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist, 
    const __dt_SurfaceInvVList *sinvlist, 
    dt_index_type i_triangle,
    __dt_ElementaryMatrix m, __dt_ElementaryVector c);


/* data structure to represent list of elementary terms */
typedef struct __dt_ElementaryTermList_struct {
    __dt_ElementaryMatrix *m_list; 
    __dt_ElementaryVector *c_list;
} __dt_ElementaryTermList;


/* Calculate all elementary terms of all triangle units in source_model and 
   stuff them into a list. 
   <trade space for time, avoid duplicated calculations> 
*/
void __dt_CreateElementaryTermList(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist, 
    const __dt_SurfaceInvVList *sinvlist,
    /* output params: */
    __dt_ElementaryTermList *termlist);

void __dt_DestroyElementaryTermList(__dt_ElementaryTermList *termlist);



/* Append the elementary matrix/vector of triangle i to the linear system. */
void __dt_AppendElementaryTermToLinearSystem(
    const dtMeshModel *source_model,
    const __dt_VertexInfoList *vtilist, 
    dt_index_type i_triangle,
    __dt_ElementaryMatrix m, __dt_ElementaryVector c,
    __dt_SparseMatrix M,     __dt_DenseVector C,
    dt_real_type weight,
    dt_index_type i_row);


/* Build phase1 equation: Es + Ei, closest point term Ec is not involved */
dt_index_type __dt_CorresEqn_Phase1(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_AdjacentTriangleList *adjlist,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist,
    __dt_SparseMatrix *M, __dt_DenseVector *C,  /* output param */
    dt_real_type weight_smooth,
    dt_real_type weight_identity);

/* Build phase2 equation: Es + Ei + Ec */
dt_index_type __dt_CorresEqn_Phase2(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_AdjacentTriangleList *adjlist,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist,
    const __dt_SpatialJoinList *spjlist,
    __dt_SparseMatrix *M, __dt_DenseVector *C,  /* output param */
    dt_real_type weight_smooth,
    dt_real_type weight_identity,
    dt_real_type weight_closest);



/* Integrate all elementary smoothness equations of source_model to the overall
   linear system M*x = C.

   Elementary terms were integrated one by another, the first one is integrated
   at the i_row-th row of M and C, and all following up terms are going downward
   until all terms has been integrated into the large linear system. The row 
   index of the next comming term is returned on success, with the help of this
   index, you can integrate more terms to the tail of currently integrated terms
   until the whole system is blown up.
*/
dt_index_type __dt_AppendSmoothnessEqn2LinearSystem(
    const dtMeshModel *source_model,
    const __dt_AdjacentTriangleList *adjlist,
    const __dt_VertexInfoList *vtilist,
    const __dt_ElementaryTermList *elemtermlist,
    __dt_SparseMatrix M, __dt_DenseVector C,
    dt_real_type smooth_term_weight, 
    dt_index_type i_row);


/* Integrate all elementary identity equations of source_model to the overall
   linear system M*x = C.

   Very similar to __dt_AppendSmoothnessEqn2LinearSystem(), I won't repeat
   the usage again because I think I've already made my point there.
*/
dt_index_type __dt_AppendIdentityEqn2LinearSystem(
    const dtMeshModel *source_model,
    const __dt_VertexInfoList *vtilist,
    const __dt_ElementaryTermList *elemtermlist,
    __dt_SparseMatrix M, __dt_DenseVector C,
    dt_real_type identity_term_weight, 
    dt_index_type i_row);


/* Integrate closest point terms: ||v - c||^2 to the overall linear system */
dt_index_type __dt_AppendSpatialJoinEqn2LinearSystem(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_VertexInfoList *vtilist,
    const __dt_SpatialJoinList *spjlist,
    __dt_SparseMatrix M, __dt_DenseVector C,
    dt_real_type closest_term_weight, 
    dt_index_type i_row);



#endif /* __DT_CORRESPONDENCE_EQUATION_HEADER__ */
