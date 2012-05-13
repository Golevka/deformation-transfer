#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "mesh_model.h"


/*
   CreateMeshModel allocates memory space for model structure according to 
   the model size specified in model->n_vertex, model->n_normvec, model->
   n_triangle, and initializes pointers to vertex, norm vector and triangle 
   arrays.

   You MUST fill in n_vertex, n_normvec and n_triangle before calling 
   CreateMeshModel().
 */
void CreateMeshModel(dtMeshModel *model)
{
    size_t
        vertex_siz   = sizeof(dtVertex)   * (size_t)model->n_vertex,
        normvec_siz  = sizeof(dtVector)   * (size_t)model->n_normvec,
        triangle_siz = sizeof(dtTriangle) * (size_t)model->n_triangle;

    size_t total_mem_siz = vertex_siz + normvec_siz + triangle_siz;
    char *mem = (char*)__dt_malloc(total_mem_siz);

    /* Initialize pointers to vector, normvec and triangle arrays

       MeshModel data storage layout:
       [mem]=>[---vertex---|---norm_vector---|------triangle------]
    */
    model->vertex   = (dtVertex*)  (mem);
    model->normvec  = (dtVector*)  (mem + vertex_siz);
    model->triangle = (dtTriangle*)(mem + vertex_siz + normvec_siz);
}

/* DestroyMeshModel frees all memory space allocated in CreateMeshModel */
void DestroyMeshModel(dtMeshModel *model)
{
    free(model->vertex);  /* we just need to free once and ONLY once */
}


/* Read .obj model file into model object, if reading file failed, it would 
   simply print an error message to stderr and crash. */
void __dt_ReadObjFile_commit_or_crash(
    const char *filename, dtMeshModel *model)
{
    int ret;
    if ((ret = ReadObjFile(filename, model)) != 0)
    {
        fprintf(stderr, "file: %s - ", filename);
        if (ret == -1) {
            perror("Reading model file error");
        }
        else {
            fprintf(stderr, "Syntax error on line: %u\n", ret);
        }
        exit(-1);
    }
}


/* Sort out a list containing the index of normal vectors of each vertex in the
   model, this list might be helpful in closest point iteration. 

   You have to manually free the pointer returned by this function after using 
   it. */
dt_index_type *__dt_SortOutVertexNormalList(const dtMeshModel *model)
{
    dtTriangle   *triangle;
    dt_index_type i_vertex, i_normvec;
    dt_index_type i_triangle, iv; /* looping variable, we need to inspect every
                                     triangle unit to sort out vertex normal 
                                     informations. */
    dt_index_type *inorm_list = (dt_index_type*)__dt_malloc(
        (size_t)model->n_vertex * sizeof(dt_index_type));

    /* An initial guess of vertex normals, bullet proof for isolated vertices */
    memset(inorm_list, 0, (size_t)model->n_vertex * sizeof(dt_index_type));

    for (i_triangle = 0; i_triangle < model->n_triangle; i_triangle++)
    {
        triangle = model->triangle + i_triangle; /* inspect this one */
        for (iv = 0; iv < 3; iv++)
        {
            /* get the iv-th local vertex on this triangle unit */
            i_vertex  = triangle->i_vertex[iv];
            i_normvec = triangle->i_norm[iv];
   
            inorm_list[i_vertex] = i_normvec;  /* log it */
        }
    }

    return inorm_list;
}
