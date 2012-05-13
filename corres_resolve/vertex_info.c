#include <assert.h>
#include <stdlib.h>
#include "vertex_info.h"



/* Construct a list to distinguish constrained vertices from others in the 
   model as well as providing a fast way to get the indexes of these vertices
*/
void __dt_CreateVertexInfoList(
    const dtMeshModel *model, const __dt_VertexConstraintList *constraint_list,
    __dt_VertexInfoList *vtilist)
{
    dt_index_type i_vertex = 0, i_free = 0, i_cons = 0;
    vtilist->list_length = model->n_vertex;

    /* allocate for index & type lists */
    vtilist->vertex_type = (__dt_VertexType*)__dt_malloc(
        (size_t)vtilist->list_length * sizeof(__dt_VertexType));
    vtilist->vertex_index = (dt_index_type*)__dt_malloc(
        (size_t)vtilist->list_length * sizeof(dt_index_type));

    for ( ; i_vertex < model->n_vertex; i_vertex++)
    {
        /* find if vertex index can be found in constraint list, if found, it 
           should be a constrained vertex, */
        if ((i_cons < constraint_list->list_length) && 
            (i_vertex == constraint_list->constraint[i_cons].i_src_vertex))
        {
            vtilist->vertex_index[i_vertex] = i_cons++;
            vtilist->vertex_type [i_vertex] = __DT_CONSTRAINED_VERTEX;
        }
        else   /* or it is a free vertex. */
        {
            vtilist->vertex_index[i_vertex] = i_free++;
            vtilist->vertex_type [i_vertex] = __DT_FREE_VERTEX;
        }
    }

    /* number of free vertices and constrained vertices were already figured 
       out in the previous pass of linear scan. */
    vtilist->n_free        = i_free;
    vtilist->n_constrained = i_cons;
}

/* Free allocated memory for the index list */
void __dt_DestroyVertexInfoList(__dt_VertexInfoList *vtilist)
{
    free(vtilist->vertex_type);
    free(vtilist->vertex_index);
}


/* the most basic way to query vertex properties in correspondence mapping 
   phase */
__dt_VertexInfo __dt_GetVertexInfo(
    const __dt_VertexInfoList *vtilist, dt_index_type i_vertex)
{
    __dt_VertexInfo vertex_info;

    __DT_ASSERT(i_vertex < vtilist->list_length,
        "Vertex index out of bounds in __dt_GetVertexInfo");

    vertex_info.vertex_type  = vtilist->vertex_type[i_vertex];
    vertex_info.vertex_index = vtilist->vertex_index[i_vertex];
    
    return vertex_info;
}

/* Query info of all three vertices on i_triangle-th triangle unit in model */
void __dt_GetTriangleVerticesInfo(
    const dtMeshModel *model,
    const __dt_VertexInfoList *vtilist, dt_index_type i_triangle,
    __dt_VertexInfo *vinfo)
{
    const dtTriangle *triangle = model->triangle + i_triangle;
    dt_index_type i_vertex = 0;

    /* Get vertex info of all vertices in the triangle unit */
    for ( ; i_vertex < 3; i_vertex++)
    {
        vinfo[i_vertex] = __dt_GetVertexInfo(vtilist, 
            triangle->i_vertex[i_vertex]);
    }
}


/* Get the index of components of free or phantom vertices (v4) in the linear 
   system of correspondence phase. */
dt_index_type __dt_GetFreeCoordVarIndex(const __dt_VertexInfoList *vtilist,
    dt_index_type i_vertex, dt_index_type i_dimension)
{
    __DT_ASSERT(
        vtilist->vertex_type[i_vertex] == __DT_FREE_VERTEX, 
        "i_vertex specified is not index of a free vertex.");

    return vtilist->vertex_index[i_vertex] * 3 + i_dimension;
}

dt_index_type __dt_GetPhantomCoordVarIndex(const __dt_VertexInfoList *vtilist,
    dt_index_type i_triangle, dt_index_type i_dimension)
{
    return (vtilist->n_free + i_triangle) * 3 + i_dimension;
}


/* An easy way to get the variable vector indexes of coordinate components of 
   all vertices (including the phantom vertex) in a triangle unit. 

   v_ind is a 2 dimensional array containing all retrieved variable vector 
   indexes, -1 indicates that the corresponding coordinate component belongs
   to a constrained vertex. 
*/
void __dt_GetTriangleVerticesVarIndex(
    const dtMeshModel *model, const __dt_VertexInfoList *vtilist, 
    dt_index_type i_triangle,
    __dt_TriangleVarIndexTable v_ind)
{
    const dtTriangle *triangle = model->triangle + i_triangle;
    __dt_VertexInfo vinfo;
    dt_index_type i_dim, i_vlocal;

    for (i_dim = 0; i_dim < 3; i_dim++)
    {
        for (i_vlocal = 0; i_vlocal < 3; i_vlocal++)
        {
            vinfo = __dt_GetVertexInfo(
                vtilist, triangle->i_vertex[i_vlocal]);

            /* get the variable vector index of i_dim-th component of this
               vertex if it is a free vertex, or mark it a component of a 
               constrained vertex with -1 */
            v_ind[i_dim][i_vlocal] = (vinfo.vertex_type == __DT_FREE_VERTEX ?
                __dt_GetFreeCoordVarIndex(
                    vtilist, triangle->i_vertex[i_vlocal], i_dim):
                -1);
        }
        v_ind[i_dim][3] = __dt_GetPhantomCoordVarIndex(vtilist, i_triangle, i_dim);
    }
}


/* Get the index of variable vector component from the var index table */
dt_index_type __dt_GetVarIndexFromTable(
    __dt_TriangleVarIndexTable var_ind, 
    dt_index_type i_localvertex, dt_index_type i_dimension)
{
    return var_ind[i_dimension][i_localvertex];
}
