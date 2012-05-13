#include <stdlib.h>
#include <memory.h>

#include "correseqn.h"


/* Ei: identity term: it enforces the deformation not being too sharp by 
   minimizing:

       ||T[i] - I||_F^2

   where T[i] is the deformation matrix of triangle i in the source model.
*/

static dt_index_type __dt_append_identity_term_to_linear_system(
    const dtMeshModel *model, const __dt_VertexInfoList *vtilist,
    dt_index_type i_triangle, 
    __dt_ElementaryMatrix m, __dt_ElementaryVector c,    
    __dt_SparseMatrix M,     __dt_DenseVector C,
    dt_real_type identity_term_weight, 
    dt_index_type i_row)
{
    __dt_Vector9D c_identity = {1, 0, 0, 
                                0, 1, 0,
                                0, 0, 1};

    dt_index_type i = 0;
    for ( ; i < 9; i++) c_identity[i] += c[i]; /* construct T - I: 
                                                  append -I to term T */

    /* I didn't modify c because I don't want to hurt those elementary terms */

    __dt_AppendElementaryTermToLinearSystem(
        model, vtilist, i_triangle,
        m, c_identity, M, C, identity_term_weight, i_row);

    return i_row + 9;
}


/* Integrate all elementary identity equations of source_model to the overall
   linear system M*x = C.
*/
dt_index_type __dt_AppendIdentityEqn2LinearSystem(
    const dtMeshModel *source_model,
    const __dt_VertexInfoList *vtilist,
    const __dt_ElementaryTermList *elemtermlist,
    __dt_SparseMatrix M, __dt_DenseVector C,
    dt_real_type identity_term_weight, 
    dt_index_type i_row)
{
    dt_index_type i_triangle;

    __dt_ElementaryMatrix *m_list = elemtermlist->m_list;
    __dt_ElementaryVector *c_list = elemtermlist->c_list;

    /* append all identity terms to the linear system */
    for (i_triangle = 0; i_triangle < source_model->n_triangle; i_triangle++)
    {
        i_row = __dt_append_identity_term_to_linear_system(
            source_model, vtilist,
            i_triangle, m_list[i_triangle], c_list[i_triangle],
            M, C, identity_term_weight, i_row);
    }

    return i_row;
}

