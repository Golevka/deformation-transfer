#ifndef __DT_CORRESPONDENCE_PROBLEM_HEADER__
#define __DT_CORRESPONDENCE_PROBLEM_HEADER__


#include "correseqn.h"
#include "triangle_corr.h"


/* This structure describles the problem we need to solve in the correspondence
   phase. */
typedef struct __dt_CorrespondenceProblem_struct
{
    dtMeshModel source_model, target_model;  /* mesh model to process */
    __dt_AdjacentTriangleList adjlist;       /* adjacency list */
    __dt_VertexConstraintList conslist;      /* marker points constraints */
    __dt_VertexInfoList       vtilist;       /* varvector/conslist index */

    dt_real_type   weight_smooth;      /* weight for smoothness term */
    dt_real_type   weight_identity;    /* weight for identity term */

    /* weight for closest point iteration: [start: step: end) */
    dt_real_type   weight_closest_start, weight_closest_end;
    dt_real_type   weight_closest_step;

    /* result: triangle units correspondences */
    __dt_TriangleCorrsList result_tclist;

} dtCorrespondenceProblem;


/* Specify source mesh, target mesh and the vertex constraints you specified by
   placing marker points with Corres!, one more thing, you have to tell him the
   triangle unit adjacency of source model resolved with our adjtool.  */
void CreateCorrespondenceProblem(
    dtCorrespondenceProblem *problem,
    const char *source_mesh_name, const char *target_mesh_name,
    const char *vertex_constraint_name, 
    const char *source_adjacency_name);


/* Free all resources allocated for the correspondence problem object.  */
void DestroyCorrespondenceProblem(dtCorrespondenceProblem *problem);

/* Solve the problem to make the source model deform into the target model */
void SolveCorrespondenceProblem(dtCorrespondenceProblem *problem);



#endif /* __DT_CORRESPONDENCE_PROBLEM_HEADER__ */

