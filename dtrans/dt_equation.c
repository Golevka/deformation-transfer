#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "dt_equation.h"



/* calculate the size of the equation with the triangle correspondence list,
   it scans the list and counts the correspondence entries for each target 
   triangles to obtain the row number and column number of the coefficient 
   matrix of the deformation equation 
*/
static void __calculate_equation_size(
    const dtMeshModel *target_mesh, const __dt_TriangleCorrsDict *tcdict,
    /* output param */ dt_size_type *n_row, dt_size_type *n_col)
{
    dt_size_type  n_corrs, n_eqn = 0;
    dt_index_type i_tgt_triangle = 0;

    for ( ; i_tgt_triangle < target_mesh->n_triangle; i_tgt_triangle++)
    {
        n_corrs = __dt_GetTriangleCorrsNumber(tcdict, i_tgt_triangle);
        n_eqn += ((n_corrs > 0)? n_corrs: 1);
    }

    *n_row = 9 * n_eqn;
    *n_col = 3 * (target_mesh->n_vertex + target_mesh->n_triangle);
}

/* Allocate for coefficient matrix and rhs vector with proper size */
void __dt_AllocDeformationEquation(
    const dtMeshModel *target_mesh, const __dt_TriangleCorrsDict *tcdict,
    __dt_SparseMatrix *A_tri, __dt_DenseVector *C)
{
    dt_size_type n_row, n_col;
    __calculate_equation_size(target_mesh, tcdict, &n_row, &n_col);

    /* allocate for coefficient triplet matrix and rhs vector */
    *A_tri = __dt_CHOLMOD_allocate_triplet((size_t)n_row, (size_t)n_col, 0);
    *C     = __dt_CHOLMOD_dense_zeros     ((size_t)n_row, (size_t)1);
}


/* Get elementary matrix m from the inverse of surface (triangle unit) matrix,
   it is quite similar with the one in corres_resolve but we don't need to 
   bother with vertex constraints here. */
static void __calculate_elementary_matrix(
    dtMatrix3x3 inV, __dt_ElementaryMatrix m)
{
    dt_index_type i, j, row;  /* looping index: 
                                 dimension, equation and row index */
    for (i = 0, row = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++, row++)
        {
            /* coefficient of v1: -(a[0,i] + a[1,i] + a[2,i]) */
            m[row][0] = -(inV[0][j] + inV[1][j] + inV[2][j]);

            /* coefficient of v2..v4 */
            m[row][1] = inV[0][j];
            m[row][2] = inV[1][j];
            m[row][3] = inV[2][j];
        }
    }
}

/* Get the index of components of free or phantom vertices (v4) in the linear
   system of deformation transfer phase, similar with the correspondence phase
   procedure __dt_GetFreeCoordVarIndex(). */
static dt_index_type __get_variable_index(
    const dtMeshModel *model, dt_index_type i_triangle,
    dt_index_type i_vlocal, dt_index_type i_dim)
{
    const dtTriangle *triangle = model->triangle + i_triangle;
    dt_index_type i_vertex;

    if (i_vlocal < 3)  /* real vertex */
    {
        i_vertex = triangle->i_vertex[i_vlocal];
        return i_vertex * 3 + i_dim;
    }
    else {             /* phantom vertex */
        return 3*(model->n_vertex + i_triangle) + i_dim;
    }
}


static dt_index_type __append_elem_matrix_to_linear_system(
    const dtMeshModel *model, dt_index_type i_triangle,
    __dt_ElementaryMatrix m, __dt_SparseMatrix M, dt_index_type i_row)
{
    dt_index_type i_var;  /* index on variable vector */
    dt_index_type i_dim, i_vlocal, i_eqn; /* loop variable */

    dt_index_type j_row = 0;  /* loop index for row number of 
                                 elementary matrix/vector */

    /* construct 9 lines of equations */
    for (i_dim = 0; i_dim < 3; i_dim++)
    {
        for (i_eqn = 0; i_eqn < 3; i_eqn++, i_row++, j_row++)
        {
            /* fill in line i_row of coefficient matrix */
            for (i_vlocal = 0; i_vlocal < 4; i_vlocal++)
            {
                /* get the variable vector index of this vertex then append 
                   this elementary term */
                i_var = __get_variable_index(model, i_triangle, i_vlocal, i_dim);
                __dt_CHOLMOD_entry(M, i_row, i_var, m[j_row][i_vlocal]);
            }
        }
    }

    return i_row;
}


/* Build the coefficient matrix of deformation equations from target reference 
   mesh, the coefficient matrix can be built only once to deform for a lot of 
   deformed source meshes. 
*/
void __dt_BuildCoefficientMatrix(
    const dtMeshModel *target_ref, const __dt_TriangleCorrsDict *tcdict,
    __dt_SparseMatrix A)
{
    __dt_SurfaceInvVList  sinvlist;
    __dt_ElementaryMatrix mat;

    dt_index_type 
        i_triangle = 0, i_entry, i_row = 0;  /* looping index */

    dt_size_type n_corrs;

    __dt_InitializeSurfaceInvVList(target_ref, &sinvlist);


    for ( ; i_triangle < target_ref->n_triangle; i_triangle++)
    {
        n_corrs = __dt_GetTriangleCorrsNumber(tcdict, i_triangle);

        if (n_corrs == 0)  /* i_triangle has no corresponded pieces */
        {
            /* Triangle correspondence entry for this target triangle unit is
               absent, minimize the transformation with an identity matrix */
            __calculate_elementary_matrix(sinvlist.inV[i_triangle], mat);
            i_row = __append_elem_matrix_to_linear_system(
                target_ref, i_triangle, mat, A, i_row);
        }
        else
        {
            /* Append coefficient elements for minimizing the difference of
               transformation between corresponded triangle units */
            for (i_entry = 0; i_entry < n_corrs; i_entry++)
            {
                __calculate_elementary_matrix(sinvlist.inV[i_triangle], mat);
                i_row = __append_elem_matrix_to_linear_system(
                    target_ref, i_triangle, mat, A, i_row);
            }
        }
    }

    __dt_DestroySurfaceInvVList(&sinvlist);
}


/* It is quite straight forward, just place the elements of the transformation
   matrix in row-majored sequence */
static dt_index_type __append_rhs_vector_to_linear_system(
    dtMatrix3x3 T, __dt_DenseVector C, dt_index_type i_row)
{
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[0][0]);  i_row += 1;
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[0][1]);  i_row += 1;
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[0][2]);  i_row += 1;
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[1][0]);  i_row += 1;
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[1][1]);  i_row += 1;
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[1][2]);  i_row += 1;
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[2][0]);  i_row += 1;
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[2][1]);  i_row += 1;
    __dt_CHOLMOD_MODIFYVEC(C, i_row, T[2][2]);  i_row += 1;

    return i_row;
}

/* Build rhs vector of the deformation equation for source_ref=>source_deform.
   You just need to build rhs vector for each deformation while keeping the 
   coefficient matrix unchanged.
 */
void __dt_BuildRhsConstantVector(
    const dtMeshModel *source_deformed, const dtMeshModel *target_ref,
    const __dt_SurfaceInvVList *sinvlist_ref,
    const __dt_TriangleCorrsDict *tcdict,
    __dt_DenseVector C)
{
    dtMatrix3x3 V, T;

    dt_size_type n_corrs;
    dt_index_type 
        i_triangle = 0, i_entry = 0, i_src_triangle,
        i_row = 0;

    for ( ; i_triangle < target_ref->n_triangle; i_triangle++)
    {
        n_corrs = __dt_GetTriangleCorrsNumber(tcdict, i_triangle);

        if (n_corrs == 0)    /* i_triangle absent in tclist */
        {
            /* Triangle correspondence entry for this target triangle unit is
               absent, minimize the transformation with an identity matrix */
            T[0][0] = 1.0,  T[0][1] = 0.0,  T[0][2] =  0.0;
            T[1][0] = 0.0,  T[1][1] = 1.0,  T[1][2] =  0.0;
            T[2][0] = 0.0,  T[2][1] = 0.0,  T[2][2] =  1.0;

            /* append I to C */
            i_row = __append_rhs_vector_to_linear_system(T, C, i_row);
        }
        else 
        {
            for (i_entry = 0; i_entry < n_corrs; i_entry++)
            {
                /* get source-target triangle index */
                i_src_triangle = __dt_GetTriangleCorrsEntry(
                    tcdict, i_triangle, i_entry)->i_src_triangle;

                __dt_CalculateTriangleUnitMatrix(source_deformed, i_src_triangle, V);
                __dt_Matrix3x3_Product(V, sinvlist_ref->inV[i_src_triangle], T);

                /* append T to C */
                i_row = __append_rhs_vector_to_linear_system(T, C, i_row);
            }
        }
    }
}
