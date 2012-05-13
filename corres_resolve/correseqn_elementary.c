#include <stdlib.h>
#include <memory.h>
#include "correseqn.h"


/*
  The paper has developed a way to derive the equivalent version of the 
  equation above in vertex form, as far as we known, T[i] can be represented as

          T[i] = U[i] * inv V[i],  
  
    where U[i] is the surface matrix of deformed triangle i, verbosely 
  represented as:

          U[i] = [u2-u1, u3-u1, u4], u4 = sqrt normalized (u2-u1)x(u3-u1)

    where u1, u2, u3 are vertices of this deformed triangle unit, notice that 
  U[i] is linearly represented by the vertices of the triangle, so we can 
  rewrite the objective function into a squared sum of three linear expressions

          || U[i] * inv V[i] - U[j0] * inv V[j0] ||_F ^2
        + || U[i] * inv V[i] - U[j1] * inv V[j1] ||_F ^2
        + || U[i] * inv V[i] - U[j2] * inv V[j2] ||_F ^2
  
    Squared Frobenius norm happens to be the squared sum of all elements of the
  matrix, so that we can reshape each matrix in the expression to a column 
  vector and evaluate the squared L2 norm of them.

          U * V
      [u2x-u1x, u3x-u1x, u4x]   [v11, v12, v13]
    = [u2y-u1y, u3y-u1y, u4y] * [v21, v22, v23]
      [u2z-u1z, u3z-u1z, u4z]   [v31, v32, v33]

    = [-(v11 + v21 + v31)*u1x + v11*u2x + v21*u3x + v31*u4x, 
       -(v12 + v22 + v32)*u1y + v12*u2y + v22*u3y + v32*u4y,
       -(v13 + v23 + v33)*u1z + v13*u2z + v23*u3z + v33*u4z ]
      [......]
      [......]

    reshape to column vector form (inspect the following equation with a wider screen >_< ):

    = [-(v11 + v21 + v31), v11, v21, v31]                                                                               [u1x]
      [-(v12 + v22 + v32), v12, v22, v32]                 0                                                             [u2x]
      [-(v13 + v23 + v33), v13, v23, v33]                                                                               [u3x]
                                                                                                                        [u4x]
                                          [-(v11 + v21 + v31), v11, v21, v31]                                           [u1y]
                      0                   [-(v12 + v22 + v32), v12, v22, v32]                  0                    *   [u2y]
                                          [-(v13 + v23 + v33), v13, v23, v33]                                           [u3y]
                                                                                                                        [u4y]
                                                                              [-(v11 + v21 + v31), v11, v21, v31]       [u1z]
                      0                                   0                   [-(v12 + v22 + v32), v12, v22, v32]       [u2z]
                                                                              [-(v13 + v23 + v33), v13, v23, v33]       [u3z]
                                                                                                                        [u4z]
    However, rather than representing the 9x9 coefficient matrix "as is" in a
  dt_real_type[9][9], we align all meaningful element of the matrix to the
  left side, so that we can shrink this matrix and stuff it into a smaller 9x4
  matrix. That's what these code are all about.
*/


static void __calculate_v1_coefficients(
    const __dt_SurfaceInvVList *sinvlist, dt_index_type i_triangle,
    dt_real_type coefv1[3])
{
    dtMatrix3x3 *inV = &(sinvlist->inV[i_triangle]);  /* rhs matrix */

    dt_index_type i_eqn = 0;
    for ( ; i_eqn < 3; i_eqn++)
    {
        /* coefficient of v1: 
           -(a[0,i] + a[1,i] + a[2,i]), i = 0..2 */
        coefv1[i_eqn] = 
            -((*inV)[0][i_eqn] + (*inV)[1][i_eqn] + (*inV)[2][i_eqn]);
    }
}


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
    __dt_ElementaryMatrix m, __dt_ElementaryVector c)
{
    dtMatrix3x3 *inV = &(sinvlist->inV[i_triangle]);   /* rhs matrix */
    __dt_VertexInfo vinfo[3];   /* info of vertices in that triangle */
    dt_real_type    coefv1[3];  /* coefficient of v1 terms: v1.x, v1.y, v1.z */

    dt_real_type   coef;
    dt_index_type  i, j, row;  /* looping index: 
                                  dimension, equation and row index */

    memset(m, 0, sizeof(__dt_ElementaryMatrix)); /* reset matrix to 0 */
    __calculate_v1_coefficients(sinvlist, i_triangle, coefv1);
    __dt_GetTriangleVerticesInfo(source_model, vtilist, i_triangle, vinfo);

    for (i = 0, row = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++, row++)
        {
            c[row] = 0;

            /* coefficient of v1: -(a[0,i] + a[1,i] + a[2,i]) */
            coef = coefv1[j];
            (vinfo[0].vertex_type == __DT_FREE_VERTEX)?
                (m[row][0] = coef):
                (c[row]   -= coef *
                    __dt_GetMappedVertexCoord(
                        target_model, conslist, vinfo[0].vertex_index, i));

            /* coefficient of v2: a[0,i]*/
            coef = (*inV)[0][j];
            (vinfo[1].vertex_type == __DT_FREE_VERTEX)?
                (m[row][1] = coef): 
                (c[row]   -= coef *
                    __dt_GetMappedVertexCoord(
                        target_model, conslist, vinfo[1].vertex_index, i));

            /* coefficient of v3: a[1,i] */
            coef = (*inV)[1][j];
            (vinfo[2].vertex_type == __DT_FREE_VERTEX)?
                (m[row][2] = coef):
                (c[row]   -= coef *
                    __dt_GetMappedVertexCoord(
                        target_model, conslist, vinfo[2].vertex_index, i));

            /* coefficient of v4: a[2,i] */
            m[row][3] = (*inV)[2][j];
        }
    }
}


/* Calculate all elementary terms of all triangle units in source_model and 
   stuff them into a list */
void __dt_CreateElementaryTermList(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_VertexConstraintList *conslist,
    const __dt_VertexInfoList *vtilist, 
    const __dt_SurfaceInvVList *sinvlist,
    /* output params: */
    __dt_ElementaryTermList *termlist)
{
    dt_index_type i_triangle;

    /* allocate for the list */
    __dt_ElementaryMatrix *m_list = termlist->m_list = 
        (__dt_ElementaryMatrix*)__dt_malloc(
        (size_t)source_model->n_triangle * sizeof(__dt_ElementaryMatrix));

    __dt_ElementaryVector *c_list = termlist->c_list = 
        (__dt_ElementaryVector*)__dt_malloc(
        (size_t)source_model->n_triangle * sizeof(__dt_ElementaryVector));

    /* precalculate all elementary terms of triangle units */
    for (i_triangle = 0; i_triangle < source_model->n_triangle; i_triangle++)
    {
        __dt_CalculateElementaryTerm(source_model, target_model, 
            conslist, vtilist, sinvlist, i_triangle, 
            m_list[i_triangle], c_list[i_triangle]);
    }
}

/* Yeah! There comes out boys! */
void __dt_DestroyElementaryTermList(__dt_ElementaryTermList *termlist)
{
    free(termlist->m_list);
    free(termlist->c_list);
}



/* Append the elementary matrix/vector of triangle i to the linear system. */
void __dt_AppendElementaryTermToLinearSystem(
    const dtMeshModel *source_model,
    const __dt_VertexInfoList *vtilist, 
    dt_index_type i_triangle,
    __dt_ElementaryMatrix m, __dt_ElementaryVector c,
    __dt_SparseMatrix M,  __dt_DenseVector C,
    dt_real_type weight,
    dt_index_type i_row)
{
    __dt_TriangleVarIndexTable var_ind;  /* var index table for triangle i */

    dt_index_type i_var;  /* index on variable vector */
    dt_index_type i_dim, i_vlocal, i_eqn; /* loop variable */

    dt_index_type j_row = 0;  /* loop index for row number of 
                                 elementary matrix/vector */

    __dt_GetTriangleVerticesVarIndex(
        source_model, vtilist, i_triangle, var_ind);

    /* construct 9 lines of equations */
    for (i_dim = 0; i_dim < 3; i_dim++)
    {
        for (i_eqn = 0; i_eqn < 3; i_eqn++, i_row++, j_row++)
        {
            /* fill in line i_row of coefficient matrix */
            for (i_vlocal = 0; i_vlocal < 4; i_vlocal++)
            {
                /* get the variable vector index of this vertex, -1 indicates
                   that specified vertex is not a free vertex. */
                i_var = __dt_GetVarIndexFromTable(var_ind, i_vlocal, i_dim);

                /* append this term if it is a free vertex */
                if (i_var != -1) {
                    __dt_CHOLMOD_entry(M, i_row, i_var, weight * m[j_row][i_vlocal]);
                }
            }
            /* element of right hand side vector */
            __dt_CHOLMOD_MODIFYVEC(C, i_row, 
                __dt_CHOLMOD_REFVEC(C, i_row) + weight * c[j_row]);
        }
    }
}

