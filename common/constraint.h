#ifndef __DT_CONSTRAINT_HEADER__
#define __DT_CONSTRAINT_HEADER__


/* This header file defines basic data structures for representing vertex 
   location constraints in the triangle correspondence resolving algorithm. */


#include "dt_type.h"


/* 
   Vertex constraints forces the source mesh deforming to the shape of target
   mesh. Some "critical" vertices are specified to be deformed to the same 
   position of the corresponded one on the target mesh, which forces the 
   overall look of the deformed mesh similar to the target mesh. 

   CONSTRAINT (.cons) FILE FORMAT:
   --------------------------------------
   number of constraints
   source_vertex0,  target_vertex0
   source_vertex1,  target_vertex1
   source_vertex2,  target_vertex2
   ...
   --------------------------------------
*/
typedef struct __dt_VertexConstraintEntry_struct
{
    /* i_*_vertex: indexes of constrained vertices
       src: source mesh,  tgt: target mesh */
    dt_index_type i_src_vertex, i_tgt_vertex;

    /* source.vertex[i_src_vertex] should be deformed to 
       target.vertex[i_tgt_vertex], so the coordinate of deformed 
       i_src_vertex-th vertex is regarded as constant which equals to 
       i_tgt_vertex-th vertex on the target mesh. */

} __dt_VertexConstraintEntry;


typedef struct __dt_VertexConstraintList_struct
{
    __dt_VertexConstraintEntry *constraint;
    dt_size_type  list_length;  /* number of constraint entries, should be 
                                   equal to the number of marker points you 
                                   specified with corrstool. */
} __dt_VertexConstraintList;



/* Allocate for constraint_list with the size information specified in
   constraint_list->list_length */
void __dt_CreateConstraintList(__dt_VertexConstraintList *constraint_list);


/* Load vertex constraints from file, these constraints are often specifed by
   users with the graphical interactive tool "Corres!" in our package. The 
   entries are sorted to arrange source vertex indexes in ascending order, 
   which would accelerate the construction of vertex info (type-index) list.

   This function returns an 0 on success, or it would return an -1 to indicate
   the occurrence of an fopen() error.
*/
int __dt_LoadConstraints(const char *filename, 
    __dt_VertexConstraintList *constraint_list);

/* Release all resources allocated for the constraint list object */
void __dt_ReleaseConstraints(__dt_VertexConstraintList *constraint_list);


/* Save constraint list to file

   This function returns 0 on success, or it would return an -1 to indicate an
   fopen() error, go checking system variable errno for furture investivation.
*/
int __dt_SaveConstraints(const char *filename, 
    const __dt_VertexConstraintList *constraint_list);



/* Find corresponding target vertex with specified vertex constriant. */
dtVertex* __dt_GetMappedVertex(
    const dtMeshModel *target_model, const __dt_VertexConstraintList *conslist,
    dt_index_type i_cons);

/* Given a vertex correspondence constraint of one constrained vertex, find
   the coordinate of the corresponded vertex on the target mesh. */
dt_real_type __dt_GetMappedVertexCoord(
    const dtMeshModel *target_model, const __dt_VertexConstraintList *conslist,
    dt_index_type i_cons, dt_index_type i_dimension);




#endif /* __DT_CONSTRAINT_HEADER__ */
