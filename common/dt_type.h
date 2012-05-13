#ifndef __DT_TYPE_HEADER__
#define __DT_TYPE_HEADER__


#include "cholmod_wrapper.h"


/*  ASSERTION macros for ensuring some invariants in the program.
    When the assertion fails, it would dump the file name and line number as 
    well as the error message to stdout and make the program break down loudly.
*/
#define __DT_ASSERT(cond, error_msg) assert((cond) && error_msg)


typedef int dt_size_type;
typedef int dt_index_type;      /* triangles are represented with indexes of 
                                   vertices and norm vectors, */
typedef double dt_real_type;    /* type of coordinate components in vertices
                                   of 3D trimesh models */

/* Each instance of this type represents a single vertex in a 
   3D mesh model, or a vector in 3D space (ex. a norm vector of some vertex. 
*/
typedef struct __dtVertex_struct
{
    dt_real_type x, y, z;  /* the order of these fields is important because
                              code fragments similar to *(&(x) + i_dim) are
                              frequently used to get a coordinate component
                              of given dimensional index. */
} dtVertex, dtVector;


/* All 3D objects we take into consideration is represented by a bunch of
   triangular surfaces (through triangular tesselation), each triangular unit
   is made up with 3 vertices and their norm vectors.
*/
typedef struct __dtTriangle_struct
{
    dt_index_type i_vertex[3];   /* indexes of vertices */
    dt_index_type i_norm  [3];   /* indexes of their norm vectors */

} dtTriangle;


/* 3D mesh model made up with triangular units, it contains coordinates of all
   vertices of the object as well as norm vector of each vertex. Triangular
   surface units can be derived from indexes arrays of those vertices and norms.
*/
typedef struct __dtMeshModel_struct
{
    /* Vertices, norm vectors and triangular surfaces in the model */
    dtVertex   *vertex;       /* array of all vertices in the model */
    dtVector   *normvec;      /* array of norm vectors */
    dtTriangle *triangle;     /* array of all triangular units */

    /* Number of vertices, norm vectors and triangles in the model */
    dt_size_type n_vertex;
    dt_size_type n_normvec;
    dt_size_type n_triangle;

} dtMeshModel;


#define __dt_malloc malloc   /* wrapper for malloc with enhanced 
                                exception handling */


/* The story about elementary matrices of triangle units and the overall large
   sparse linear system can be found in corres_resolve/correseqn_elementary.c 
*/

typedef dt_real_type __dt_Matrix9x4[9][4];
typedef dt_real_type __dt_Vector9D [9];

typedef __dt_Matrix9x4 __dt_ElementaryMatrix;
typedef __dt_Vector9D  __dt_ElementaryVector;


/* Matrix and vector type for the overall sparse linear systems */
typedef  cholmod_triplet*  __dt_SparseMatrix;
typedef  cholmod_dense*    __dt_DenseVector;



#endif /* __DT_TYPE_HEADER__ */
