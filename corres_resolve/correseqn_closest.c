#include <stdlib.h>
#include <memory.h>

#include "correseqn.h"


/* Integrate closest point terms: ||v - c||^2 to the overall linear system: 
       vx = cx
       vy = cy
       vz = cz
*/
dt_index_type __dt_AppendSpatialJoinEqn2LinearSystem(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_VertexInfoList *vtilist,
    const __dt_SpatialJoinList *spjlist,
    __dt_SparseMatrix M, __dt_DenseVector C,
    dt_real_type closest_term_weight, 
    dt_index_type i_row)
{
    dtVertex *tgt_vertex;
    dt_index_type i_x, i_y, i_z;
    dt_index_type i_vertex = 0;

    for ( ; i_vertex < source_model->n_vertex; i_vertex++)
    {
        if (vtilist->vertex_type[i_vertex] == __DT_FREE_VERTEX)
        {
            i_x = __dt_GetFreeCoordVarIndex(vtilist, i_vertex, 0);
            i_y = __dt_GetFreeCoordVarIndex(vtilist, i_vertex, 1);
            i_z = __dt_GetFreeCoordVarIndex(vtilist, i_vertex, 2);

            tgt_vertex = target_model->vertex + spjlist->i_target_vertex[i_vertex];

            __dt_CHOLMOD_entry(M, i_row, i_x, closest_term_weight);
            __dt_CHOLMOD_MODIFYVEC(C, i_row, closest_term_weight * tgt_vertex->x);
            i_row++;

            __dt_CHOLMOD_entry(M, i_row, i_y, closest_term_weight);
            __dt_CHOLMOD_MODIFYVEC(C, i_row, closest_term_weight * tgt_vertex->y);
            i_row++;

            __dt_CHOLMOD_entry(M, i_row, i_z, closest_term_weight);
            __dt_CHOLMOD_MODIFYVEC(C, i_row, closest_term_weight * tgt_vertex->z);
            i_row++;
        }
    }

    return i_row;
}
