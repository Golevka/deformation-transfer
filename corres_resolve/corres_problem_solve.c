#include <math.h>
#include "corres_problem.h"
#include "triangle_corr.h"


#define __dt_SOLVER_least_square __dt_UMFPACK_least_square


/* Apply the solution vector of the correspondence equation to the vertices of
   the source model, make the source model deform into the target model. */
static void __apply_deformation_to_source_model(
    dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_VertexInfoList *vtilist, 
    const __dt_VertexConstraintList *conslist,
    const __dt_DenseVector vec);

/*
static void __port_normal_vectors(
    dtMeshModel *source_model, const dtMeshModel *target_model);
*/

static void __solve_correspondence_problem_Launch(
    dtCorrespondenceProblem *problem)
{
    problem = problem;      /* get rid of `unused parameter' warning */
    __dt_CHOLMOD_start();   /* start CHOLMOD module */
}


static void __solve_correspondence_problem_Phase1(
    dtCorrespondenceProblem *problem)
{
    cholmod_triplet *M;
    cholmod_dense   *C, *x;
    cholmod_sparse  *A;

    /* building linear system */
    __dt_CorresEqn_Phase1(
        &(problem->source_model), &(problem->target_model), 
        &(problem->adjlist), &(problem->conslist), &(problem->vtilist), 
        &M, &C,
        sqrt(problem->weight_smooth), 
        sqrt(problem->weight_identity));

    /* solve the least square problem */
    printf("solving linear system...\n");
    A = __dt_CHOLMOD_triplet_to_sparse(M);
    __dt_CHOLMOD_free_triplet(&M);
    x = __dt_SOLVER_least_square(A, C);

    __dt_CHOLMOD_free_sparse(&A);
    __dt_CHOLMOD_free_dense(&C);

    /* deform the source mesh according to the solution we got */
    printf("applying deformation...\n");
    __apply_deformation_to_source_model(
        &(problem->source_model), &(problem->target_model),
        &(problem->vtilist), &(problem->conslist), x);

    __dt_CHOLMOD_free_dense(&x);
}


static void __solve_correspondence_problem_Phase2(
    dtCorrespondenceProblem *problem)
{
    cholmod_triplet *M;
    cholmod_dense   *C, *x;
    cholmod_sparse  *A;

    dt_index_type *i_src_norm_list, *i_tgt_norm_list;
    __dt_SpatialJoinList spjlist;
    __3dTree tree_tgt;

    dt_real_type weight_closest;

    /* build 3d tree for target model for fast closest point iteration */
    tree_tgt = __dt_Build3DTree_Vertex(&(problem->target_model));

    /* sort out vertex normals */
    i_src_norm_list = __dt_SortOutVertexNormalList(&(problem->source_model));
    i_tgt_norm_list = __dt_SortOutVertexNormalList(&(problem->target_model));

    /* closest point iteration */
    for (weight_closest = problem->weight_closest_start;
         weight_closest < problem->weight_closest_end;
         weight_closest += problem->weight_closest_step)
    {
        printf("current weight: %f\n", weight_closest);

        /* Resolving spatial join */
        printf("resolving spatial join...\n");
        __dt_CreateSpatialJoinList(&(problem->source_model), &spjlist);
        __dt_ResolveModelSpatialJoin(
            &(problem->source_model), &(problem->target_model),
            i_src_norm_list, i_tgt_norm_list, tree_tgt, &spjlist);

        /* build linear system */
        printf("building linear system...\n");
        __dt_CorresEqn_Phase2(
            &(problem->source_model), &(problem->target_model), 
            &(problem->adjlist), &(problem->conslist), &(problem->vtilist), 
            &spjlist, 
            &M, &C, 
            sqrt(problem->weight_smooth), 
            sqrt(problem->weight_identity), 
            /* sqrt(weight_closest)); */
            weight_closest);

        __dt_DestroySpatialJoinList(&spjlist);


        /* solve the least square problem */
        printf("solving linear system...\n");
        A = __dt_CHOLMOD_triplet_to_sparse(M);
        __dt_CHOLMOD_free_triplet(&M);
        x = __dt_SOLVER_least_square(A, C);

        __dt_CHOLMOD_free_sparse(&A);
        __dt_CHOLMOD_free_dense(&C);

        /* deform the source mesh according to the solution we got */
        printf("applying deformation...\n");
        __apply_deformation_to_source_model(
            &(problem->source_model), &(problem->target_model),
            &(problem->vtilist), &(problem->conslist), x);

        __dt_CHOLMOD_free_dense(&x);
    }

    free(i_src_norm_list);
    free(i_tgt_norm_list);
    __3dtree_Destroy3DTree(tree_tgt);
}


static void __solve_correspondence_problem_Finalize(
    dtCorrespondenceProblem *problem)
{
    __dt_CHOLMOD_finish();  /* stop the CHOLMOD module */
    /* __port_normal_vectors(&(problem->source_model), &(problem->target_model)); */

    __dt_ResolveTriangleCorres_e(
        &(problem->source_model), &(problem->target_model), 
        &(problem->result_tclist));
}


/* Solve the problem to make the source model deform into the target model */
void SolveCorrespondenceProblem(dtCorrespondenceProblem *problem)
{
    __solve_correspondence_problem_Launch(problem);
    __solve_correspondence_problem_Phase1(problem);
    __solve_correspondence_problem_Phase2(problem);
    __solve_correspondence_problem_Finalize(problem);
}



/* Apply the solution vector of the correspondence equation to the vertices of
   the source model, make the source model deform into the target model. */
static void __apply_deformation_to_source_model(
    dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_VertexInfoList *vtilist, 
    const __dt_VertexConstraintList *conslist,
    const __dt_DenseVector vec)
{
    dt_index_type cons_ind, vertex_ind;
    dt_index_type i_v = 0;

    for ( ; i_v < vtilist->list_length; i_v++)
    {
        if (vtilist->vertex_type[i_v] == __DT_FREE_VERTEX)
        {
            source_model->vertex[i_v].x = 
                __dt_CHOLMOD_REFVEC(
                    vec, __dt_GetFreeCoordVarIndex(vtilist, i_v, 0));

            source_model->vertex[i_v].y = 
                __dt_CHOLMOD_REFVEC(
                    vec, __dt_GetFreeCoordVarIndex(vtilist, i_v, 1));

            source_model->vertex[i_v].z = 
                __dt_CHOLMOD_REFVEC(
                    vec, __dt_GetFreeCoordVarIndex(vtilist, i_v, 2));
        }
        else
        {
            cons_ind = vtilist->vertex_index[i_v];
            vertex_ind = conslist->constraint[cons_ind].i_tgt_vertex;
            source_model->vertex[i_v] = target_model->vertex[vertex_ind];
        }
    }

    /* FIXME: saving the deformed model in each iteration might be painful */
    SaveObjFile("out.obj", source_model);
}


/* Buggy code, proceed with caution */
/*
static void __port_normal_vectors(
    dtMeshModel *source_model, const dtMeshModel *target_model)
{
    __dt_SpatialJoinList spjlist;
    dt_index_type *i_src_norm_list, *i_tgt_norm_list, i;

    i_src_norm_list = __dt_SortOutVertexNormalList(source_model);
    i_tgt_norm_list = __dt_SortOutVertexNormalList(target_model);

    __dt_CreateSpatialJoinList(source_model, &spjlist);
    __dt_ResolveModelSpatialJoin_BruteForce(
        source_model, target_model, 
        i_src_norm_list, i_tgt_norm_list, &spjlist);

    for (i = 0; i < source_model->n_vertex; i++)
    {
        source_model->normvec[i] = 
            target_model->normvec[
                i_tgt_norm_list[
                    spjlist.i_target_vertex[i]
                    ]
                ];
    }


    free(i_src_norm_list);
    free(i_tgt_norm_list);
    __dt_DestroySpatialJoinList(&spjlist);
}
*/
