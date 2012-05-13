#include <stdlib.h>
#include <memory.h>

#include "correseqn.h"

/*
  Es: deformation smoothness term, indicates that the transformation for 
  adjacent triangles should be equal. For a mesh with N vertices, we let T[i]
  be the deformation matrix, V[i] be the surface matrix of triangle i in the 
  source model, then 

          || T[i] - T[j0] ||_F ^2 
        + || T[i] - T[j1] ||_F ^2 
        + || T[i] - T[j2] ||_F ^2

  should be minimized.
*/

static dt_index_type __dt_append_smoothness_term_to_linear_system(
    const dtMeshModel *source_model,
    const __dt_VertexInfoList *vtilist, 
    dt_index_type i_triangle, dt_index_type i_adjtriangle, 
    __dt_ElementaryMatrix m,     __dt_ElementaryVector c,
    __dt_ElementaryMatrix m_adj, __dt_ElementaryVector c_adj,
    __dt_SparseMatrix M,         __dt_DenseVector C,
    dt_real_type smooth_term_weight,
    dt_index_type i_row)
{
    __dt_AppendElementaryTermToLinearSystem(source_model, vtilist, 
        i_triangle, m,c, M,C, smooth_term_weight, i_row);

    __dt_AppendElementaryTermToLinearSystem(source_model, vtilist, 
        i_adjtriangle, m_adj,c_adj, M,C, -smooth_term_weight, i_row);

    return i_row + 9;
}


/* Integrate all elementary smoothness equations of source_model to the overall
   linear system M*x = C.

   Elementary terms where integrated one by another, the first one is integrated
   at the i_row-th row of M and C, and all following up terms are going downward
   until all terms has been integrated into the large linear system. The row 
   index of the next comming term is returned on success, with the help of this
   index, you can integrate more terms to the tail of currently integrated terms
   until the whole system is blown up.
*/
dt_index_type __dt_AppendSmoothnessEqn2LinearSystem(
    const dtMeshModel *source_model,
    const __dt_AdjacentTriangleList *adjlist,
    const __dt_VertexInfoList *vtilist,
    const __dt_ElementaryTermList *elemtermlist,
    __dt_SparseMatrix M, __dt_DenseVector C,
    dt_real_type smooth_term_weight, 
    dt_index_type i_row)
{
    dt_index_type i_triangle, i_adjtriangle;

    __dt_AdjacentTriangles adj;

    /* elementary term for this triangle and the adjacent one */
    __dt_ElementaryMatrix  *m, *m_adj, *m_list = elemtermlist->m_list;
    __dt_ElementaryVector  *c, *c_adj, *c_list = elemtermlist->c_list;


    for (i_triangle = 0; i_triangle < source_model->n_triangle; i_triangle++)
    {
        /* elementary term of current triangle */
        m = m_list + i_triangle;
        c = c_list + i_triangle;

        adj = __dt_GetAdjacentTriangles(adjlist, i_triangle);

        /* iterate through all adjacent triangles of i_triangle, 
           append T_i - T_adj to overall linear system. */
        for (i_adjtriangle = 0; i_adjtriangle < adj.n_adjtriangle; i_adjtriangle++)
        {
            /* grab the elementary term of the adjacent triangle */
            m_adj = m_list + adj.i_adjtriangle[i_adjtriangle];
            c_adj = c_list + adj.i_adjtriangle[i_adjtriangle];

            i_row = __dt_append_smoothness_term_to_linear_system(
                source_model, vtilist,
                i_triangle, adj.i_adjtriangle[i_adjtriangle],
                *m, *c, *m_adj, *c_adj, M, C, smooth_term_weight, i_row);
        }
    }

    return i_row;
}
