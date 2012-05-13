#include <stdlib.h>
#include <stdio.h>
#include "cholmod_wrapper.h"
#include "umfpack.h"


static cholmod_common __dt_common, *cm = NULL;


/* Halt if an error occurs */
static void __dt_cholmod_error_handler(int status, const char *file, int line,
    const char *message)
{
    fprintf(stderr, "cholmod error: file: %s line: %d status: %d: %s\n",
        file, line, status, message) ;

    exit(1);
}


/* Start CHOLMOD and set working parameters */
void __dt_CHOLMOD_start(void)
{
    cholmod_start(&__dt_common);
    cm = &__dt_common;

    /* use default parameter settings, except for the error handler. It leads
     * the program to terminate if an error occurs (out of memory, not positive
     * definite,. etc) */
    cm->error_handler = __dt_cholmod_error_handler;
}

/* Terminate CHOLMOD */
void __dt_CHOLMOD_finish(void)
{
    cholmod_finish(cm);
}


/* A wrapper for cholmod_allocate_triplet, creating only unsymmetric real 
   sparse matrices */
cholmod_triplet* __dt_CHOLMOD_allocate_triplet(
    size_t nrow, size_t ncol, size_t nzmax)
{
    return 
        cholmod_allocate_triplet(
            nrow, ncol, nzmax, 
            0,     /* 0: unsymmetric;
                      1: symmetric with just upper part stored;
                     -1: symmetric with just lower part stored */
            CHOLMOD_REAL, 
            cm);
}

/* Append triplet entry (i,j,x) to specified cholmod_triplet matrix object */
int __dt_CHOLMOD_entry(cholmod_triplet *T, int i, int j, double x)
{
    if (T->nnz >= T->nzmax && 
        !cholmod_reallocate_triplet(2*(T->nzmax), T, cm))
    {
        return 0;  /* reallocation failed */
    }

    /* append subscript */
    if (T->itype == CHOLMOD_LONG)
    {
        ((UF_long*)T->i)[T->nnz] = i;
        ((UF_long*)T->j)[T->nnz] = j;
    }
    else
    {
        ((int*)T->i)[T->nnz] = i;
        ((int*)T->j)[T->nnz] = j;
    }
    /* append value */
    __dt_CHOLMOD_MODIFYVEC(T, T->nnz, x);


#define __DT_MAX(a,b) (((a)>(b))?(a):(b))
    T->nrow = __DT_MAX(T->nrow, (size_t)(i+1));
    T->ncol = __DT_MAX(T->ncol, (size_t)(j+1));
#undef __DT_MAX

    T->nnz += 1;

    return 1;
}

/* A wrapper for cholmod_triplet_to_sparse, it generates a new cholmod_sparse
   matrix in column-major form which is equivalent to the original triplet one.
*/
cholmod_sparse* __dt_CHOLMOD_triplet_to_sparse(cholmod_triplet *A_trip)
{
    return
        cholmod_triplet_to_sparse(
            A_trip, 
            A_trip->nnz,  /* make the size just fit */
            cm);
}

/* Convert a sparse matrix to dense form */
cholmod_dense*  __dt_CHOLMOD_sparse_to_dense(cholmod_sparse *A_sparse)
{
    return cholmod_sparse_to_dense(A_sparse, cm);
}

/* A wrapper for cholmod_zeros, it creates a dense nrow x ncol matrix and fill
   it with zero. */
cholmod_dense* __dt_CHOLMOD_dense_zeros(size_t nrow, size_t ncol)
{
    return cholmod_zeros(nrow, ncol, CHOLMOD_REAL, cm);
}

/* A wrapper for cholmod_free_sparse, it free the memory of specified sparse 
   matrix object */
int __dt_CHOLMOD_free_sparse(cholmod_sparse **A)
{
    return cholmod_free_sparse(A, cm);
}

/* Free triplet form sparse matrix object */
int __dt_CHOLMOD_free_triplet(cholmod_triplet **A_tri)
{
    return cholmod_free_triplet(A_tri, cm);
}

/* Free dense matrix object */
int __dt_CHOLMOD_free_dense(cholmod_dense **X)
{
    return cholmod_free_dense(X, cm);
}


/* calculate the transpose of A */
cholmod_sparse *__dt_CHOLMOD_transpose(cholmod_sparse *A) {
    return cholmod_transpose(A, 2, cm);
}

/* calculate A * At */
cholmod_sparse *__dt_CHOLMOD_AxAt(cholmod_sparse *A)
{
    cholmod_sparse *AAt = cholmod_aat(A, NULL, 0, 1, cm);
    cholmod_sort(AAt, cm);
    return AAt;
}

/* calculate A*At and return a symmetric matrix storing only upper elements */
cholmod_sparse *__dt_CHOLMOD_AxAt_symm(cholmod_sparse *A)
{
    cholmod_sparse *AAt = cholmod_aat(A, NULL, 0, 1, cm);
    AAt->stype = 1;         /* symmetric */
    cholmod_sort(AAt, cm);
    return AAt;
}

/* calculate b = A*c, where A is sparse and c is dense */
void __dt_CHOLMOD_Axc(cholmod_sparse *A, cholmod_dense *c, cholmod_dense *b)
{
    double alpha[2] = {1,0};
    double beta [2] = {0,0};

    cholmod_sdmult(A, 0, alpha, beta, c, b, cm); /* b = A*c */
}

/* Solve least square problem: min||c - A*x||^2 */
cholmod_dense* __dt_UMFPACK_least_square(cholmod_sparse *A, cholmod_dense *c)
{
    void *symbolic_obj, *numeric_obj;
    cholmod_sparse *A_trans, *AtA;
    cholmod_dense  *x, *b = __dt_CHOLMOD_dense_zeros(A->ncol, 1);

    A_trans = __dt_CHOLMOD_transpose(A);       /* A_trans = A' */
    __dt_CHOLMOD_Axc(A_trans, c, b);           /* b = A'*c */
    AtA = __dt_CHOLMOD_AxAt(A_trans);          /* AtA = A'*A */

    __dt_CHOLMOD_free_sparse(&A_trans);

    /* preordering and factorization */
    umfpack_di_symbolic(
        (int)AtA->nrow, (int)AtA->ncol, 
        (const int*)AtA->p, (const int*)AtA->i, (const double*)AtA->x, 
        &symbolic_obj, NULL, NULL);

    umfpack_di_numeric(
        (const int*)AtA->p, (const int*)AtA->i, (const double*)AtA->x, 
        symbolic_obj, &numeric_obj, NULL, NULL);

    umfpack_di_free_symbolic(&symbolic_obj);
    x = cholmod_allocate_dense(AtA->ncol, 1, AtA->ncol, CHOLMOD_REAL, cm);

    /* solve problem with back-substitution */
    umfpack_di_solve(UMFPACK_A, 
        (const int*)AtA->p, (const int*)AtA->i, (const double*)AtA->x, 
        (double*)x->x, (const double*)b->x, 
        numeric_obj, NULL, NULL);

    /* free intermediates */
    umfpack_di_free_numeric(&numeric_obj);
    __dt_CHOLMOD_free_sparse(&AtA);
    __dt_CHOLMOD_free_dense(&b);

    return x;
}

/* Solve least square problem using CHOLMOD */
cholmod_dense* __dt_CHOLMOD_least_square(cholmod_sparse *A, cholmod_dense *c)
{
    cholmod_factor *L;
    cholmod_sparse *A_trans, *AtA;
    cholmod_dense  *x, *b = __dt_CHOLMOD_dense_zeros(A->ncol, 1);

    A_trans = __dt_CHOLMOD_transpose(A);            /* A_trans = A' */
    __dt_CHOLMOD_Axc(A_trans, c, b);                /* b = A'*c */
    AtA = __dt_CHOLMOD_AxAt_symm(A_trans);          /* AtA = A'*A */
    __dt_CHOLMOD_free_sparse(&A_trans);

    /* solve AtA * x = b, AKA A'*A*x = A'*c */
    L = cholmod_analyze(AtA, cm);
    cholmod_factorize(AtA, L, cm);
    x = cholmod_solve(CHOLMOD_A, L, b, cm);  /* we got the solution here */

    /* free intermediates */
    cholmod_free_factor(&L, cm);
    __dt_CHOLMOD_free_sparse(&AtA);
    __dt_CHOLMOD_free_dense(&b);

    return x;
}


/* Read a sparse matrix from file in MatrixMarket format. */
cholmod_sparse* __dt_CHOLMOD_read_sparse(const char *filename)
{
    FILE *fd = fopen(filename, "r");
    cholmod_sparse *spmat = cholmod_read_sparse(fd, cm);
    fclose(fd);

    return spmat;
}

/* Read a dense matrix from file in MatrixMarket format. */
cholmod_dense* __dt_CHOLMOD_read_dense(const char *filename)
{
    FILE *fd = fopen(filename, "r");
    cholmod_dense *dmat = cholmod_read_dense(fd, cm);
    fclose(fd);

    return dmat;
}

/* Dumpa sparse matrix to stdout in MatrixMarket triplet form */
void __dt_CHOLMOD_dump_sparse(cholmod_sparse *mat)
{
    cholmod_write_sparse(stdout, mat, NULL, "", cm);
}

/* Dump a dense matrix to stdout */
void __dt_CHOLMOD_dump_dense(cholmod_dense *mat)
{
    cholmod_write_dense(stdout, mat, "", cm);
}
