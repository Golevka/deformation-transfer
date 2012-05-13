#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "constraint.h"



/* Allocate for constraint_list with the size information specified in
   constraint_list->list_length */
void __dt_CreateConstraintList(__dt_VertexConstraintList *constraint_list)
{
    /* allocate memory space for vertex constraint entries */
    constraint_list->constraint = (__dt_VertexConstraintEntry*)__dt_malloc(
            (size_t)constraint_list->list_length 
            * sizeof(__dt_VertexConstraintEntry));
}


/* Vertex constraint entries are sorted after being read from file. */
static void __sort_constraint_entries(
    __dt_VertexConstraintList *constraint_list);

/* Load vertex constraints from file, these constraints are often specifed by
   users with the graphical interactive tool "Corres!" in our package. The 
   entries are sorted to arrange source vertex indexes in ascending order, 
   which would accelerate the construction of vertex info (type-index) list.

   This function returns an 0 on success, or it would return an -1 to indicate
   the occurrence of an fopen() error.
*/
int __dt_LoadConstraints(const char *filename, 
    __dt_VertexConstraintList *constraint_list)
{
    FILE *fd = fopen(filename, "r");
    dt_index_type i_entry = 0;  /* looping variable: entry index */

    if (fd != NULL)
    {
        /* get constraint list size and allocate for it */
        fscanf(fd, "%d", &(constraint_list->list_length));
        __dt_CreateConstraintList(constraint_list);

        /* read vertex constraint entries */
        for ( ; i_entry < constraint_list->list_length; i_entry++)
        {
            fscanf(fd, "%d,%d", 
                &(constraint_list->constraint[i_entry].i_src_vertex),
                &(constraint_list->constraint[i_entry].i_tgt_vertex));
        }

        fclose(fd);

        /* sort all entries in ascending order, which would accelerate building
           the vertex type-index list */
        __sort_constraint_entries(constraint_list);
        return 0;
    }
    else {
        return -1;    /* fopen() blew up */
    }
}


/* Release all resources allocated for the constraint list object */
void __dt_ReleaseConstraints(__dt_VertexConstraintList *constraint_list) {
    free(constraint_list->constraint);
}


/* Save constraint list to file

   This function returns 0 on success, or it would return an -1 to indicate an
   fopen() error, go checking system variable errno for furture investivation
*/
int __dt_SaveConstraints(const char *filename, 
    const __dt_VertexConstraintList *constraint_list)
{
    FILE *fd = fopen(filename, "w");
    dt_index_type i_entry = 0;   /* looping variable: entry index */

    if (fd != NULL)
    {
        /* first line: number of constrant entries  */
        fprintf(fd, "%d\n", constraint_list->list_length);

        /* each following line contains a pair of unsigned integer, 
           representing constrained vertex indexes in source and target mesh.
        */
        for ( ; i_entry < constraint_list->list_length; i_entry++)
        {
            fprintf(fd, "%d, %d\n", 
                constraint_list->constraint[i_entry].i_src_vertex,
                constraint_list->constraint[i_entry].i_tgt_vertex);
        }

        fclose(fd);
        return 0;
    }
    else {
        return -1;   /* fopen() error brought this routine down */
    }
}


/* Find corresponding target vertex with specified vertex constriant. */
dtVertex* __dt_GetMappedVertex(
    const dtMeshModel *target_model, const __dt_VertexConstraintList *conslist,
    dt_index_type i_cons)
{
    dt_index_type i_tgtvertex = conslist->constraint[i_cons].i_tgt_vertex;

    __DT_ASSERT(i_cons < conslist->list_length, 
        "Constraint entry index out of bounds in __dt_GetMappedVertex");
        
    return target_model->vertex + i_tgtvertex;
}

/* Given a vertex correspondence constraint of one constrained vertex, find
   the coordinate of the corresponded vertex on the target mesh. */
dt_real_type __dt_GetMappedVertexCoord(
    const dtMeshModel *target_model, const __dt_VertexConstraintList *conslist,
    dt_index_type i_cons, dt_index_type i_dimension)
{
    const dtVertex *v = 
        __dt_GetMappedVertex(target_model, conslist, i_cons);

    /* Memory layout hack: structure fields x, y and z must be defined in
       certain order to make this trick work properly. */
    return *(&(v->x) + i_dimension);
}



/* a helper function for qsort to compare any two entries in constraint list,
   indexes of source vertices are key values to determine the partial order 
   relationship. */
static int __compare_constraint_entries(const void *_1, const void *_2)
{
    const __dt_VertexConstraintEntry 
        *entry1 = (__dt_VertexConstraintEntry*)_1,
        *entry2 = (__dt_VertexConstraintEntry*)_2;

    return entry1->i_src_vertex - entry2->i_src_vertex;
}

/* Sort all constraint entries in ascending order */
static void __sort_constraint_entries(
    __dt_VertexConstraintList *constraint_list)
{
    qsort(constraint_list->constraint, (size_t)constraint_list->list_length,
        sizeof(__dt_VertexConstraintEntry), __compare_constraint_entries);
}
