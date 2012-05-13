#ifndef __DT_VERTEX_INDEX_HEADER__
#define __DT_VERTEX_INDEX_HEADER__


/* In correspondence mapping phase, we need to solve an quadratic optimization
   problem to map all pieces of triangle units in the source mesh to the target
   mesh, we translate this problem to solving an equivalent large sparse linear
   system. 

   There's some vertex mapping correspondences which was assigned by user with 
   our Corres! tool, such correspondences forces some of the vertices on the 
   source mesh deform to the same location with their corresponding vertices on
   the target mesh, they contribute as [constraint conditions] in the original
   quadratic programming problem, in our equivalent linear system developed in
   the context of vertex framework, their coordinates became constants while 
   the coordinates of other "free" vertices are variables of the equation to be
   solved.

   It will be quite nessesary to distinguish constrained vertices from other
   free vertices to make building the linear system easier. We need to number 
   all free vertices to locate their coordinates properly in the unknown vector
   'x' of 'Ax = b', all constrained vertices should also be numbered to make 
   locating these constant coordinates faster to compute the right hand side 
   constant vector b.
*/


#include "constraint.h"


/* Data type to distinguish type of vertices */
typedef enum __dt_VertexType_enum
{
    __DT_CONSTRAINED_VERTEX,    /* constrained vertex */
    __DT_FREE_VERTEX            /* free vertex to be located by solving a
                                   linear system. */
} __dt_VertexType;


/* A lookup table to find the type and vector index / correspondence index of
   a vertex with specified index in constant time */
typedef struct __dt_VertexInfoList_struct
{
    dt_size_type     list_length;    /* should be equal to the number of 
                                        vertices in the model */
    dt_size_type     n_free;         /* number of free vertices */
    dt_size_type     n_constrained;  /* number of constrained vertices */

    __dt_VertexType *vertex_type;    /* vertex_type[i]: type of vertex i in 
                                        some model (some_model.vertex[i] */
    /* vertex_index[i]: 
         type = CONSTRAINED: index of its correspondency in constraint list
         type = INDEX: location at the unknown vector. */
    dt_index_type   *vertex_index;

} __dt_VertexInfoList;


typedef struct __dt_VertexInfo_struct
{
    __dt_VertexType vertex_type;  /* type of the vertex: constrained or free */
    dt_index_type   vertex_index; /* constrained vertex: index of the 
                                     correspondency entry in constraint list */
} __dt_VertexInfo;

    

/* Construct a list to distinguish constrained vertices from others in the 
   model as well as providing a fast way to get the indexes of these vertices
*/
void __dt_CreateVertexInfoList(
    const dtMeshModel *model, const __dt_VertexConstraintList *constraint_list,
    __dt_VertexInfoList *vtilist);

/* Free allocated memory for the index list */
void __dt_DestroyVertexInfoList(__dt_VertexInfoList *vtilist);



/* The most basic way to query vertex info in correspondence mapping phase */
__dt_VertexInfo __dt_GetVertexInfo(
    const __dt_VertexInfoList *vtilist, dt_index_type i_vertex);

/* Query info of all three vertices on i_triangle-th triangle unit in model */
void __dt_GetTriangleVerticesInfo(
    const dtMeshModel *model,
    const __dt_VertexInfoList *vtilist, dt_index_type i_triangle,
    __dt_VertexInfo *vinfo);


/* Get the index of components of free or phantom vertices (v4) in the linear 
   system of correspondence phase.

   Layout of variable vector in the linear system:

   [
     # coordinates of free vertices
     v[0].x, v[0].y, v[0].z,
     v[1].x, v[1].y, v[1].z, 
     v[2].x, v[2].y, v[2].z,  
     ......
     v[n_free-1].x, v[n_free-1].y, v[n_free-1].z,

     # coordinates of phantom vertices
     t[0].x, t[0].y, t[0].z,
     t[1].x, t[1].y, t[1].z, 
     ......
   ]

   where v is an array of free vertices, t is an array of phantom vertices, 
   t[i] is the phantom vertex of triangle i.
*/
dt_index_type __dt_GetFreeCoordVarIndex(const __dt_VertexInfoList *vtilist,
    dt_index_type i_vertex, dt_index_type i_dimension);

dt_index_type __dt_GetPhantomCoordVarIndex(const __dt_VertexInfoList *vtilist,
    dt_index_type i_triangle, dt_index_type i_dimension);



/* An easy way to get the variable vector indexes of coordinate components of 
   all vertices (including the phantom vertex) in a triangle unit. 

   v_ind is a 2 dimensional array containing all retrieved variable vector 
   indexes, -1 indicates that the corresponding coordinate component belongs
   to a constrained vertex.

   v_ind = [i_v1.x, i_v2.x, i_v3.x, i_v4.x]
           [i_v1.y, i_v2.y, i_v3.y, i_v4.y]
           [i_v1.z, i_v2.z, i_v3.z, i_v4.z]

           where i_v1, i_v2, i_v3 are indexes of i_triangle-th triangle unit, 
           i_v4 indicates the index of phantom vertex.
*/
typedef dt_index_type __dt_TriangleVarIndexTable[3][4];

void __dt_GetTriangleVerticesVarIndex(
    const dtMeshModel *model, const __dt_VertexInfoList *vtilist, 
    dt_index_type i_triangle,
    __dt_TriangleVarIndexTable var_ind);

/* Get the index of variable vector component from the var index table */
dt_index_type __dt_GetVarIndexFromTable(
    __dt_TriangleVarIndexTable var_ind, 
    dt_index_type i_localvertex, dt_index_type i_dimension);




#endif /* __DT_VERTEX_INDEX_HEADER__ */
