#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "adjacent.h"



/* Allocate for adjacency_list with the size information specified in
   adjacency_list->list_length */
void __dt_CreateAdjacencyList(__dt_AdjacentTriangleList *adjacency_list)
{
    /* allocate for adjacency entries */
    adjacency_list->adjacency = (__dt_AdjacentTriangleEntry*)__dt_malloc(
            (size_t)adjacency_list->list_length 
            * sizeof(__dt_AdjacentTriangleEntry));
}


/* Load pre-calculated triangle mesh adjacencies stored in a textfile to an 
   __dt_AdjacentTriangleList object.

   This function returns total number of adjacencies, as well as the number of
   equations needed to illustrate transformation smoothness; if this function
   failed, an -1 is returned instead.
*/
int __dt_LoadAdjacencies(
    const char *filename, __dt_AdjacentTriangleList *adj_list)
{
    FILE *fd = fopen(filename, "r");
    dt_index_type i_entry = 0;  /* looping variable: entry index */

    if (fd != NULL)
    {
        /* first line: number of adjacency items (it should be equal to the 
           number of triangular units in the corresponding mesh model). */
        fscanf(fd, "%d", &(adj_list->list_length));
        __dt_CreateAdjacencyList(adj_list);

        for ( ; i_entry < adj_list->list_length; i_entry++)
        {
            /* each line contains three adjacencies to i_triangle
               i_triangle: [i_adjtriangle0, i_adjtriangle1, i_adjtriangle2] */
            fscanf(fd, "%*d [%d,%d,%d]", 
                &(adj_list->adjacency[i_entry].i_adjtriangle[0]),
                &(adj_list->adjacency[i_entry].i_adjtriangle[1]),
                &(adj_list->adjacency[i_entry].i_adjtriangle[2]));
        }

        /* last line: total number of all adjacencies */
        fscanf(fd, "%d", &(adj_list->n_adjacency));

        fclose(fd);
        return  (int)adj_list->n_adjacency;
    }
    else {
        return -1;    /* openning file has failed */
    }
}


/* Free the resources allocated for the adjacency list object */
void __dt_ReleaseAdjacencies(__dt_AdjacentTriangleList *adj_list) {
    free(adj_list->adjacency);
}


/* This is an easy way to get the indexes of all triangles adjacent to a 
   queried triangle unit */
__dt_AdjacentTriangles __dt_GetAdjacentTriangles(
    const __dt_AdjacentTriangleList *adjlist, dt_index_type i_triangle)
{
    __dt_AdjacentTriangles ret;

    __DT_ASSERT(i_triangle < adjlist->list_length, 
        "Triangle index out of bounds in __dt_GetAdjacentTriangles");

    /* fill in indexes of all adjacent triangles */
    memcpy(ret.i_adjtriangle, adjlist->adjacency[i_triangle].i_adjtriangle, 
        sizeof(ret.i_adjtriangle));   /* size of the entire array */

    ret.n_adjtriangle = 4;
    while (ret.i_adjtriangle[--ret.n_adjtriangle - 1] == -1);

    return ret;
}


/* Print all adjacencies to stdout, this function is only intent for debugging
   or testing purposes. */
void __dt_ShowAdjacencies(const __dt_AdjacentTriangleList *adj_list)
{
    dt_index_type i = 0;

    printf("%d\n", adj_list->list_length);
    for ( ; i < adj_list->list_length; i++)
    {
        printf("%d [%d, %d, %d]\n", i,
            adj_list->adjacency[i].i_adjtriangle[0],
            adj_list->adjacency[i].i_adjtriangle[1],
            adj_list->adjacency[i].i_adjtriangle[2]);
    }
}


/* Find adjacent triangles for each triangular unit in the specified model.
   This implementation is brute-force and quite slow in practice, please 
   use the python version "adjtool/adjtool.py" instead. 

   This function returns total number of adjacent triangles of all triangular
   units in the model. 
*/
dt_size_type __dt_ResolveMeshAdjacencies_BruteForce(
    const dtMeshModel *model, __dt_AdjacentTriangleList *adj_list)
{
    dtTriangle *reference, *current;
    dt_index_type i_triangle, j_triangle, item_ind;  /* loop variables */
    dt_size_type n_shared;        /* number of shared vertices */
    dt_size_type n_totaladj = 0;  /* total adjacent triangle number */

    adj_list->list_length = model->n_triangle;
    __dt_CreateAdjacencyList(adj_list);


    /* Iterate through all triangles to find adjacents */
    for (i_triangle = 0, reference = model->triangle; 
         i_triangle < model->n_triangle; i_triangle++, reference++)
    {
        item_ind = 0;
        for (j_triangle = 0, current = model->triangle; 
             j_triangle < model->n_triangle; j_triangle++, current++)
        {
            n_shared = ((
                    reference->i_vertex[0] == current->i_vertex[0] || 
                    reference->i_vertex[0] == current->i_vertex[1] ||
                    reference->i_vertex[0] == current->i_vertex[2]) ? 1: 0); 
            
            n_shared += ((
                    reference->i_vertex[1] == current->i_vertex[0] || 
                    reference->i_vertex[1] == current->i_vertex[1] ||
                    reference->i_vertex[1] == current->i_vertex[2]) ? 1: 0); 

            if (n_shared == 0) continue; else
                n_shared += ((
                        reference->i_vertex[2] == current->i_vertex[0] || 
                        reference->i_vertex[2] == current->i_vertex[1] ||
                        reference->i_vertex[2] == current->i_vertex[2]) ? 1: 0);

            /* Two shared vertices means one shared edge, so these 2 triangles
               are adjacent */
            if (n_shared == 2)
            {
                adj_list->adjacency[i_triangle].i_adjtriangle[item_ind++] = 
                    j_triangle;
                n_totaladj++;
            }
        }

        /* This triangular unit has less than three adjacent pieces, mark the 
           rest index entries as "empty" (0xffffffff)  */
        while (item_ind < 3)
        {
            adj_list->adjacency[i_triangle].i_adjtriangle[item_ind++] = 
                (dt_index_type)(-1);
        }
    }

    return n_totaladj;
}
