#ifndef __DT_CHOLMOD_WRAPPER_HEADER__
#define __DT_CHOLMOD_WRAPPER_HEADER__


#include "cholmod.h"



/* vec->x is a void*, pointing to a buffer storing elements of the dense
   matrix. What really hurts is that the actual type of elements is indicated
   by vec->dtype:
       dtype == 0: double
       dtype == 1: float

   which means that I need a bad cast before referencing any elements of vec!
   So comes the following mess.
*/
#define __dt_CHOLMOD_REFVEC(vec,i)                              \
    (((vec)->dtype == 0)?                                       \
        ((double*)((vec)->x))[i] : ((float*)((vec)->x))[i])

/* `ModifyVec(vec,i,val)' means `vec[i] = val' 
   Note that `__dt_CHOLMOD_REFVEC(vec,i) = val' does not compile because an 
   lvalue is required as left operand of assignment.
*/
#define __dt_CHOLMOD_MODIFYVEC(vec,i, val)                      \
    (((vec)->dtype == 0)?                                       \
        (((double*)((vec)->x))[i] = (val)) :                    \
        (((float*)((vec)->x))[i] = (float)(val)))


/* Start CHOLMOD and set working parameters */
void __dt_CHOLMOD_start(void);

/* Terminate CHOLMOD */
void __dt_CHOLMOD_finish(void);


/* A wrapper for cholmod_allocate_triplet, creating only unsymmetric real 
   sparse matrices */
cholmod_triplet* __dt_CHOLMOD_allocate_triplet(
    size_t nrow, size_t ncol, size_t nzmax);


/* Append triplet entry (i,j,x) to specified cholmod_triplet matrix object */
int __dt_CHOLMOD_entry(cholmod_triplet *T, int i, int j, double x);


/* A wrapper for cholmod_triplet_to_sparse, it generates a new cholmod_sparse
   matrix in column-major form which is equivalent to the original triplet one.
*/
cholmod_sparse* __dt_CHOLMOD_triplet_to_sparse(cholmod_triplet *A_trip);

/* Convert a sparse matrix to dense form */
cholmod_dense*  __dt_CHOLMOD_sparse_to_dense(cholmod_sparse *A_sparse);

/* A wrapper for cholmod_zeros, it creates a dense nrow x ncol matrix and fill
   it with zero. */
cholmod_dense* __dt_CHOLMOD_dense_zeros(size_t nrow, size_t ncol);

/* A wrapper for cholmod_free_sparse, it free the memory of specified sparse 
   matrix object */
int __dt_CHOLMOD_free_sparse(cholmod_sparse **A);

/* Free triplet form sparse matrix object */
int __dt_CHOLMOD_free_triplet(cholmod_triplet **A_tri);

/* Free dense matrix object */
int __dt_CHOLMOD_free_dense(cholmod_dense **X);


/* calculate the transpose of A */
cholmod_sparse *__dt_CHOLMOD_transpose(cholmod_sparse *A);

/* calculate A * At */
cholmod_sparse *__dt_CHOLMOD_AxAt(cholmod_sparse *A);

/* calculate b = A*c, where A is sparse and c is dense */
void __dt_CHOLMOD_Axc(cholmod_sparse *A, cholmod_dense *c, cholmod_dense *b);


/* Solve least square problem: min||c - A*x||^2 */
cholmod_dense* __dt_CHOLMOD_least_square(cholmod_sparse *A, cholmod_dense *c);
cholmod_dense* __dt_UMFPACK_least_square(cholmod_sparse *A, cholmod_dense *c);


/* Read a sparse matrix from file in MatrixMarket format. */
cholmod_sparse* __dt_CHOLMOD_read_sparse(const char *filename);

/* Read a dense matrix from file in MatrixMarket format. */
cholmod_dense* __dt_CHOLMOD_read_dense(const char *filename);

/* Dumpa sparse matrix to stdout in MatrixMarket triplet form */
void __dt_CHOLMOD_dump_sparse(cholmod_sparse *mat);

/* Dump a dense matrix to stdout */
void __dt_CHOLMOD_dump_dense(cholmod_dense *mat);




#endif /* __DT_CHOLMOD_WRAPPER_HEADER__ */
