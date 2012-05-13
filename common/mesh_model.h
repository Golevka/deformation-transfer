#ifndef __DT_MESH_MODEL_HEADER__
#define __DT_MESH_MODEL_HEADER__


#include "dt_type.h"


/* CreateMeshModel allocates memory space for model structure according to 
   the model size specified in model->n_vertex, model->n_normvec, model->
   n_triangle, and initializes pointers to vertex, norm vector and triangle 
   arrays.

   You MUST fill in n_vertex, n_normvec and n_triangle before calling 
   CreateMeshModel().
 */
void CreateMeshModel(dtMeshModel *model);

/* DestroyMeshModel frees all memory space allocated in CreateMeshModel.
 */
void DestroyMeshModel(dtMeshModel *model);


/* ReadObjFile parse specified .obj model description file and read vertex, 
   normal vector and triangular surface information into *model

   This function returns 0 on success, otherwise it would return an -1 to 
   indicate an fopen() error (file not exist, privillage or some reason).
*/
int ReadObjFile(const char *filename, dtMeshModel *model);

/* Read .obj model file into model object, if reading file failed, it would 
   simply print an error message to stderr and crash. */
void __dt_ReadObjFile_commit_or_crash(
    const char *filename, dtMeshModel *model);

/* SaveObjFile saves specified mesh model to a file, it would return 0 on 
   success, otherwise it would return -1 to indicate fopen() was failed.
   You should check system variable errno for further investigation.
*/
int SaveObjFile(const char *filename, const dtMeshModel *model);


/* Sort out a list containing the index of normal vectors of each vertex in the
   model, this list might be helpful in closest point iteration. 
   
   You have to manually free the pointer returned by this function after using 
   it. 
*/
dt_index_type *__dt_SortOutVertexNormalList(const dtMeshModel *model);



#endif /* __DT_MESH_MODEL_HEADER__ */
