#include "correseqn.h"



static dt_index_type __build_correseqn_phase1(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_AdjacentTriangleList *adjlist,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist,
    __dt_SparseMatrix M, __dt_DenseVector C,
    dt_real_type weight_smooth,
    dt_real_type weight_identity)
{
    __dt_SurfaceInvVList    sinvlist;
    __dt_ElementaryTermList elemtermlist;
    dt_index_type i_row = 0;

    /* prepare elementary terms */
    __dt_InitializeSurfaceInvVList(source_model, &sinvlist);
    __dt_CreateElementaryTermList(source_model, target_model, 
        conslist, vtilist, &sinvlist, &elemtermlist);
    __dt_DestroySurfaceInvVList(&sinvlist);

    /* append smoothness and identity equations to the linear system */
    i_row = __dt_AppendSmoothnessEqn2LinearSystem(
        source_model, adjlist, vtilist, &elemtermlist, 
        M, C, weight_smooth, i_row);

    i_row = __dt_AppendIdentityEqn2LinearSystem(
        source_model, vtilist, &elemtermlist, 
        M, C, weight_identity, i_row);

    /* no closest point term in this phase */
    __dt_DestroyElementaryTermList(&elemtermlist);
    return i_row;
}


/* Build phase 1 equation: Es + Ei, closest point term Ec is not involved */
dt_index_type __dt_CorresEqn_Phase1(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_AdjacentTriangleList *adjlist,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist,
    __dt_SparseMatrix *M, __dt_DenseVector *C,   /* output param */
    dt_real_type weight_smooth,
    dt_real_type weight_identity)
{
    dt_size_type  nrow, ncol;

    /* determine problem size this phase and allocate space for linear system */
    nrow = 9 * (adjlist->n_adjacency + source_model->n_triangle);
    ncol = 3 * (vtilist->n_free + source_model->n_triangle);

    *M = __dt_CHOLMOD_allocate_triplet((size_t)nrow, (size_t)ncol, 0);
    *C = __dt_CHOLMOD_dense_zeros((size_t)nrow, 1);

    return 
        __build_correseqn_phase1(
            source_model, target_model, 
            adjlist, conslist, vtilist, 
            *M, *C, 
            weight_smooth, weight_identity);
}


static dt_index_type __build_correseqn_phase2(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_AdjacentTriangleList *adjlist,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist,
    const __dt_SpatialJoinList *spjlist,
    __dt_SparseMatrix M, __dt_DenseVector C,
    dt_real_type weight_smooth,
    dt_real_type weight_identity,
    dt_real_type weight_closest)
{
    dt_index_type i_row = __build_correseqn_phase1(
            source_model, target_model, 
            adjlist, conslist, vtilist, 
            M, C, weight_smooth, weight_identity);

    /* closest point term */
    i_row = __dt_AppendSpatialJoinEqn2LinearSystem(
        source_model, target_model, 
        vtilist, spjlist, 
        M, C, weight_closest, i_row);

    return i_row;
}

/* Build phase 2 equation: Es + Ei + Ec */
dt_index_type __dt_CorresEqn_Phase2(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_AdjacentTriangleList *adjlist,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist,
    const __dt_SpatialJoinList *spjlist,
    __dt_SparseMatrix *M, __dt_DenseVector *C,  /* output param */
    dt_real_type weight_smooth,
    dt_real_type weight_identity,
    dt_real_type weight_closest)
{
    dt_size_type  nrow, ncol;

    /* determine problem size this phase and allocate space for linear system */
    nrow = 9*(adjlist->n_adjacency + source_model->n_triangle) + 
           3 * (vtilist->n_free);
    ncol = 3 * (vtilist->n_free + source_model->n_triangle);

    *M = __dt_CHOLMOD_allocate_triplet((size_t)nrow, (size_t)ncol, 0);
    *C = __dt_CHOLMOD_dense_zeros((size_t)nrow, 1);

    return 
        __build_correseqn_phase2(
            source_model, target_model, 
            adjlist, conslist, vtilist, spjlist,
            *M, *C, 
            weight_smooth, weight_identity, weight_closest);
}
