#include "transformer.h"
#include "umfpack.h"



/* Create a deformation transfer object, once created, this object can help 
   deforming the target mesh like the source mesh deformation quicky and 
   faithfully. 
*/
void CreateDeformationTransformer(
    const char *source_ref_name, const char *target_ref_name,
    const char *tricorrs_name, dt_size_type n_maxcorrs,
    dtTransformer *trans)
{
    __dt_TriangleCorrsList tclist;

    cholmod_sparse   *A;
    __dt_SparseMatrix A_tri;
    void *symbolic_obj;        /* for umfpack's symbolic analysis */

    /* Load data */
    __dt_ReadObjFile_commit_or_crash(source_ref_name, &(trans->source_ref));
    __dt_ReadObjFile_commit_or_crash(target_ref_name, &(trans->target));

    if (__dt_LoadTriangleCorrsList(
            tricorrs_name, &tclist) == -1) {
        perror("Loading triangle correspondence failed");
        exit(1);
    }

    /* Initialize triangle correspondence dictionary */
    __dt_StripTriangleCorrsList(&tclist, n_maxcorrs);
    __dt_CreateTriangleCorrsDict(&(trans->target), &tclist, &(trans->tcdict));

    /* Precalculate inverse of surface matrices of source reference model*/
    __dt_InitializeSurfaceInvVList(&(trans->source_ref), &(trans->sinvlist));

    /* Allocate for linear system */
    __dt_AllocDeformationEquation(&(trans->target), &(trans->tcdict), &A_tri, &(trans->C));
    trans->c = __dt_CHOLMOD_dense_zeros(A_tri->ncol, 1);      /* rhs vector: ncol*1 */
    trans->x = __dt_CHOLMOD_dense_zeros(A_tri->ncol, 1); /* solution vector: ncol*1 */

    /* Building coefficient matrix: 
       A_tri(triplet) ==> A(sparse) ==> At ==> AtA */
    printf("building equation...\n");
    __dt_BuildCoefficientMatrix(&(trans->target), &(trans->tcdict), A_tri);
    A = __dt_CHOLMOD_triplet_to_sparse(A_tri); __dt_CHOLMOD_free_triplet(&A_tri);
    trans->At  = __dt_CHOLMOD_transpose(A);    __dt_CHOLMOD_free_sparse(&A);
    trans->AtA = __dt_CHOLMOD_AxAt(trans->At);

    printf("factorizing...\n");
    /* factorize AtA */
    umfpack_di_symbolic(
        (int)trans->AtA->nrow, (int)trans->AtA->ncol, 
        (const int*)trans->AtA->p, (const int*)trans->AtA->i, (const double*)trans->AtA->x, 
        &symbolic_obj, NULL, NULL);

    umfpack_di_numeric(
        (const int*)trans->AtA->p, (const int*)trans->AtA->i, (const double*)trans->AtA->x, 
        symbolic_obj, &(trans->numeric_obj), NULL, NULL);

    umfpack_di_free_symbolic(&symbolic_obj);
}


static void __apply_deformation_to_model(dtMeshModel *model, __dt_DenseVector x);


/* Transform the target model like source_ref==>source_deformed, trans->target
   is modified to deformed model.  */
void Transform2TargetMeshModel(
    const dtMeshModel *source_deformed, dtTransformer *trans)
{
    __dt_BuildRhsConstantVector(source_deformed, &(trans->target), 
        &(trans->sinvlist), &(trans->tcdict), trans->C);

    __dt_CHOLMOD_Axc(trans->At, trans->C, trans->c);

    umfpack_di_solve(UMFPACK_A, 
        (const int*)(trans->AtA->p), (const int*)(trans->AtA->i), (const double*)(trans->AtA->x), 
        (double*)(trans->x->x), (const double*)(trans->c->x), 
        trans->numeric_obj, NULL, NULL);

    __apply_deformation_to_model(&(trans->target), trans->x);
}

/* Update the coordinates of vertices in specified model with solution vector x */
static void __apply_deformation_to_model(dtMeshModel *model, __dt_DenseVector x)
{
    dt_index_type i = 0, ind = 0;
    for ( ; i < model->n_vertex; i++)
    {
        model->vertex[i].x = __dt_CHOLMOD_REFVEC(x, ind); ind++;
        model->vertex[i].y = __dt_CHOLMOD_REFVEC(x, ind); ind++;
        model->vertex[i].z = __dt_CHOLMOD_REFVEC(x, ind); ind++;
    }
}


/* Release the memory allocated for the transformer object */
void DestroyDeformationTransformer(dtTransformer *trans)
{
    DestroyMeshModel(&(trans->source_ref));
    DestroyMeshModel(&(trans->target));
    __dt_DestroySurfaceInvVList(&(trans->sinvlist));
    __dt_DestroyTriangleCorrsDict(&(trans->tcdict));

    umfpack_di_free_numeric(&(trans->numeric_obj));
    __dt_CHOLMOD_free_dense(&(trans->x));
    __dt_CHOLMOD_free_dense(&(trans->c));
    __dt_CHOLMOD_free_dense(&(trans->C));
    __dt_CHOLMOD_free_sparse(&(trans->At));
    __dt_CHOLMOD_free_sparse(&(trans->AtA));
}
