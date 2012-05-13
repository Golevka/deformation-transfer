#include <stdio.h>
#include "corres_problem.h"



/* Specify source mesh, target mesh and the vertex constraints you specified by
   placing marker points with Corres!, one more thing, you have to tell him the
   triangle unit adjacency of source model resolved with our adjtool.  */
void CreateCorrespondenceProblem(
    dtCorrespondenceProblem *problem,
    const char *source_mesh_name, const char *target_mesh_name,
    const char *vertex_constraint_name, 
    const char *source_adjacency_name)
{
    __dt_ReadObjFile_commit_or_crash(source_mesh_name, &(problem->source_model));
    __dt_ReadObjFile_commit_or_crash(target_mesh_name, &(problem->target_model));

    if (__dt_LoadConstraints(
            vertex_constraint_name, &(problem->conslist)) != 0)
    {
        perror("Loading vertex contraint list failed");
        exit(1);
    }

    if (source_adjacency_name != NULL)
    {
        /* load source model adjacency from file */
        if (__dt_LoadAdjacencies(
                source_adjacency_name, &(problem->adjlist)) == -1)
        {
            perror("Loading source model adjacency list failed");
            exit(1);
        }
    }
    else {
        /* resolve adjacency right here right now */
        printf("Resolving source model connectivity...\n");
        __dt_ResolveMeshAdjacencies(
            &(problem->source_model), &(problem->adjlist));
    }

    __dt_CreateVertexInfoList(
        &(problem->source_model), &(problem->conslist), &(problem->vtilist));

    __dt_CreateEmptyTriangleCorrsList(&(problem->result_tclist));
}


/* Free all resources allocated for the correspondence problem object.  */
void DestroyCorrespondenceProblem(dtCorrespondenceProblem *problem)
{
    __dt_ReleaseAdjacencies        (&(problem->adjlist));
    __dt_ReleaseConstraints        (&(problem->conslist));
    __dt_DestroyVertexInfoList     (&(problem->vtilist));
    __dt_DestroyTriangleCorrsList  (&(problem->result_tclist));
    DestroyMeshModel               (&(problem->source_model));
    DestroyMeshModel               (&(problem->target_model));
}

