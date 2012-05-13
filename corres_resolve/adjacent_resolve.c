#include <assert.h>


/* Adjacent triangles share a same edge. This is the key characteristics we 
   exploited to develop this fast adjacency resolving routine. We built a hash
   structure to enable quick lookups on edges and the triangle units holding 
   that edge, so we can find one's adjacent triangles by inspecting 3 edges
   of it. */

#include "mesh_model.h"
#include "adjacent.h"



/* An edge-triangle entry records an edge of the model and a pair of triangle
   units sharing this edge, the edge is represented with a pair of vertex 
   indexes <i_vertex0, i_vertex1>. For the uniqueness of the representation, 
   we assume that i_vertex0 should always be less than i_vertex1, so that 
   e' = <b,a> = e should be normalized to e = <a,b> when a < b.
*/
typedef struct __dt_EdgeTriangleEntry_struct
{
    dt_index_type i_vertex0,   i_vertex1;   /* endpoints of this edge */
    dt_index_type i_triangle0, i_triangle1; /* triangles sharing this edge */

    /* edges with the same i_vertex0 field are gathered into a linked list */
    struct __dt_EdgeTriangleEntry_struct *next;  /* pointer to the next entry
                                                  */
} __dt_EdgeTriangleEntry;

/* A dictionary data structure for fast searching of edges and adjacent 
   triangles, it is organized like a hash table using i_vertex0 as the hash
   value of edge-triangle entry <i_vertex0, i_vertex1>. For any patch of 
   vertices won't contain too many triangle units, the collision rate of this
   hash method is slim and acceptable.
*/
typedef struct __dt_EdgeTriangleDict_struct
{
    dt_size_type dict_size;   /* should be equal to the number of vertices */
    __dt_EdgeTriangleEntry *headlist;  /* list of head nodes of linked lists,
                                          each one represents a group of edges
                                          sharing the same starting vertex */
} __dt_EdgeTriangleDict;



/* Here comes the vertex order regulator: if v0 is larger than v1 then the value
   of them would be swapped. */
static void __regulate_vertex_order(dt_index_type *v0, dt_index_type *v1)
{
    dt_index_type tmp;

    __DT_ASSERT(
        *v0 != *v1, 
        "Invalid edge specified in __regulate_vertex_order");

    if (*v0 > *v1) {
        tmp = *v0; *v0 = *v1; *v1 = tmp;  /* swap *v0 and *v1 */
    }
}

/* Create an empty edge-triangle dictionary. This function only allocate memory
   space and build the basic structure of hash table, another procedure should
   be called to fill in edge-triangle information of the model to the dict.
*/
static void __dt_CreateEdgeTriangleDict(
    const dtMeshModel *model, __dt_EdgeTriangleDict *edict)
{
    dt_index_type i = 0;
    edict->dict_size = model->n_vertex;

    /* allocate for head nodes and initialize them */
    edict->headlist = (__dt_EdgeTriangleEntry*)__dt_malloc(
        (size_t)edict->dict_size * sizeof(__dt_EdgeTriangleEntry));

    /* mark head entries as "unavaliable" or "empty" so the list search 
       procedure won't bother them */
    for ( ; i < edict->dict_size; i++)
    {
        edict->headlist[i].i_vertex0    =  i;
        edict->headlist[i].i_vertex1    = -1;  /* marker for head entry */
        edict->headlist[i].i_triangle0  = -1;  /* -1 indicates an empty slot */
        edict->headlist[i].i_triangle1  = -1;
        edict->headlist[i].next         = NULL;
    }
}


/* search for entry with largest entry->i_vertex1 which satisfies entry->
   i_vertex1 <= k, this procedure only works with ordered linked lists */
static __dt_EdgeTriangleEntry* __search_for_lower_bound(
    dt_index_type k, __dt_EdgeTriangleEntry *list)
{
    __dt_EdgeTriangleEntry *prev, *cur = list;

    /* storm down the linked list, search for the item */
    while (cur != NULL && cur->i_vertex1 <= k)
    {
        prev = cur;
        cur  = cur->next;
    }

    return prev;
}

/* append a new entry right after pos */
static void __append_edge_triangle_entry(
    dt_index_type i_vertex0, dt_index_type i_vertex1, dt_index_type i_triangle,
    __dt_EdgeTriangleEntry *pos)
{
    /* allocate for the new entry */
    __dt_EdgeTriangleEntry *new_entry = 
        (__dt_EdgeTriangleEntry*)__dt_malloc(sizeof(__dt_EdgeTriangleEntry));

    /* link the new entry to the linked list */
    new_entry->next =  pos->next;
    pos->next = new_entry;

    /* construct the entry */
    new_entry->i_vertex0    =  i_vertex0;
    new_entry->i_vertex1    =  i_vertex1;
    new_entry->i_triangle0  =  i_triangle;
    new_entry->i_triangle1  =  -1;          /* empty slot for the other one 
                                               on the opposite side <|> */
}

/* Append edge = <i_vertex0, i_vertex1> with winged triangle i_triangle to the 
   edge-triangle dictionary */
static void __dt_AppendEdgeTriangleRecord(
    dt_index_type i_vertex0, dt_index_type i_vertex1, dt_index_type i_triangle,
    __dt_EdgeTriangleDict *edict)
{
    __dt_EdgeTriangleEntry *entry;

    /* regulate edge representation and select the hash bucket to append the 
       specified entry. */
    __regulate_vertex_order(&i_vertex0, &i_vertex1);
    entry = &(edict->headlist[i_vertex0]);

    __DT_ASSERT(
        i_vertex1 < edict->dict_size, 
        "Vertex index out of bounds in __dt_AppendEdgeTriangleRecord");

    /* search for edge <i_vertex0, i_vertex1> */
    entry = __search_for_lower_bound(i_vertex1, entry);

    if (entry->i_vertex1 == i_vertex1) 
    {
        /* edge entry <i_vertex0, i_vertex1> found, 
           add triangle index to existing entry */
        (entry->i_triangle0 == -1)? 
            (entry->i_triangle0 = i_triangle): /* first slot is empty */
            (entry->i_triangle1 = i_triangle); /* first slot is used, we should
                                                  use the other one  */
    }
    else {  /* we have to append a new entry for this edge */
        __append_edge_triangle_entry(i_vertex0, i_vertex1, i_triangle, entry);
    }
}

/* Free all resources allocated for the edge-triangle dictionary */
static void __dt_DestroyEdgeTriangleDict(__dt_EdgeTriangleDict *edict)
{
    __dt_EdgeTriangleEntry *prev, *cur;
    dt_index_type i_list;

    /* free all entry lists (hash buckets) in the dictionary */
    for (i_list = 0; i_list < edict->dict_size; i_list++)
    {
        /* free i_list-th entry list */
        for (cur = edict->headlist[i_list].next; /* skip the head node */
             cur != NULL; )
        {
            /* free this entry (node) */
            prev = cur; cur = cur->next;
            free(prev);
        }
    }

    free(edict->headlist); /* free hash buckets */
}


/* Build an edge-triangle list for the specified mesh model */
static void __dt_BuildEdgeTriangleDict(
    const dtMeshModel *model, __dt_EdgeTriangleDict *edict)
{
    dt_index_type i_vertex0, i_vertex1;
    dt_index_type i_triangle = 0;

    __dt_CreateEdgeTriangleDict(model, edict);

    /* append all triangles with their 3 edges to the dictionary */
    for ( ; i_triangle < model->n_triangle; i_triangle++)
    {
        /* first edge: <v0, v1> */
        i_vertex0 = model->triangle[i_triangle].i_vertex[0];
        i_vertex1 = model->triangle[i_triangle].i_vertex[1];
        __dt_AppendEdgeTriangleRecord(
            i_vertex0, i_vertex1, i_triangle, edict);

        /* second edge: <v1, v2> */
        i_vertex0 = model->triangle[i_triangle].i_vertex[1];
        i_vertex1 = model->triangle[i_triangle].i_vertex[2];
        __dt_AppendEdgeTriangleRecord(
            i_vertex0, i_vertex1, i_triangle, edict);

        /* third edge: <v0, v2> */
        i_vertex0 = model->triangle[i_triangle].i_vertex[0];
        i_vertex1 = model->triangle[i_triangle].i_vertex[2];
        __dt_AppendEdgeTriangleRecord(
            i_vertex0, i_vertex1, i_triangle, edict);
    }
}

/* Look up the dictionary to find the entry of edge <i_vertex0, i_vertex1> */
static __dt_EdgeTriangleEntry *__dt_FindEdgeTriangleEntry(
    dt_index_type i_vertex0, dt_index_type i_vertex1,
    __dt_EdgeTriangleDict *edict)
{
    __dt_EdgeTriangleEntry *ret;
    __regulate_vertex_order(&i_vertex0, &i_vertex1);
    ret = __search_for_lower_bound(i_vertex1, &(edict->headlist[i_vertex0]));

    __DT_ASSERT(
        ret->i_vertex1 == i_vertex1,
        "__dt_FindEdgeTriangleEntry: Edge-triangle dictionary not complete");

    return ret;
}


/* get the index of the opposite triangle unit sharing edge 
   <triangle->i_vertex[i_v0], triangle->i_vertex[i_v1]> */
static dt_index_type __get_adjacent_triangle(
    const dtTriangle *triangle, dt_index_type i_v0, dt_index_type i_v1,
    dt_index_type i_triangle,
    __dt_EdgeTriangleDict *edict)
{
    __dt_EdgeTriangleEntry *ret = __dt_FindEdgeTriangleEntry(
        triangle->i_vertex[i_v0], 
        triangle->i_vertex[i_v1],
        edict);

    /* we want a different one, on the opposite side. */
    return  (ret->i_triangle0 == i_triangle)?
                ret->i_triangle1: ret->i_triangle0;
}

/* Find adjacent triangles for each triangle unit in the specified model. */
void __dt_ResolveMeshAdjacencies(
    const dtMeshModel *model, __dt_AdjacentTriangleList *adjlist)
{
    dtTriangle *triangle;
    dt_index_type 
        i_triangle = 0, i_adj = 0,
        i_adjtriangle, n_adjacency = 0;

    __dt_EdgeTriangleDict edict;
    __dt_BuildEdgeTriangleDict(model, &edict);

    adjlist->list_length = model->n_triangle;
    __dt_CreateAdjacencyList(adjlist);

    for ( ; i_triangle < model->n_triangle; i_triangle++)
    {
        triangle = &(model->triangle[i_triangle]);
        i_adj = 0;

        /* adjacent triangle sharing edge 0: <v0, v1> */
        i_adjtriangle = __get_adjacent_triangle(triangle, 0,1, i_triangle, &edict);
        if (i_adjtriangle != -1) {
            adjlist->adjacency[i_triangle].i_adjtriangle[i_adj++] = i_adjtriangle;
            n_adjacency += 1;
        }

        /* adjacent triangle sharing edge 1: <v1, v2> */
        i_adjtriangle = __get_adjacent_triangle(triangle, 1,2, i_triangle, &edict);
        if (i_adjtriangle != -1) {
            adjlist->adjacency[i_triangle].i_adjtriangle[i_adj++] = i_adjtriangle;
            n_adjacency += 1;
        }

        /* adjacent triangle sharing edge 2: <v0, v2> */
        i_adjtriangle = __get_adjacent_triangle(triangle, 0,2, i_triangle, &edict);
        if (i_adjtriangle != -1) {
            adjlist->adjacency[i_triangle].i_adjtriangle[i_adj++] = i_adjtriangle;
            n_adjacency += 1;
        }

        /* fill -1 to the rest mem */
        while (i_adj != 3)
            adjlist->adjacency[i_triangle].i_adjtriangle[i_adj++] = -1;
    }

    adjlist->n_adjacency = n_adjacency;
    __dt_DestroyEdgeTriangleDict(&edict);
}



/* Output the dictinary to stdout, it is designed for ease of debugging */
/*
static void __dt_DumpEdgeTriangleDict(const __dt_EdgeTriangleDict *edict)
{
    dt_index_type i = 0;
    __dt_EdgeTriangleEntry *cur;
    for ( ; i < edict->dict_size; i++)
    {
        printf("edge bucket [%d]\n", i);

        for (cur = edict->headlist[i].next; cur != NULL; cur = cur->next)
        {
            printf("\tedge: <%d %d>, triangle: [%d, %d]\n", 
                cur->i_vertex0,   cur->i_vertex1,
                cur->i_triangle0, cur->i_triangle1);
        }
    }
}
*/
