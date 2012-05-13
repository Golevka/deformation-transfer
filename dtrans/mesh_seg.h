#ifndef __DT_MESH_SEGMENTATION_HEADER__
#define __DT_MESH_SEGMENTATION_HEADER__


#include "mesh_model.h"


/* Part of a segmented triangle mesh, it contains indexes of triangle units
   belonging to this component of segmentation.
*/
typedef struct __dt_MeshSegComponent_struct
{
    dt_index_type *i_segtriangle;   /* a list of triangle indexes belonging to 
                                       this component. */
    dt_size_type   n_segtriangle;   /* number of triangle units in this 
                                       component */
} __dt_MeshSegComponent;


/* Segmentation of a mesh model, triangle units in the mesh are partitioned 
   into several components represented by a list of __dt_MeshSegComponent 
   objects. 
   
   Attention: components are allowed to intersect with each other.
*/
typedef struct __dt_MeshSegmentation_struct
{
    __dt_MeshSegComponent *seglist;   /* a list of segmented components */
    dt_size_type    n_segcomponent;   /* number of components spawned by 
                                         this segmentation */
} __dt_MeshSegmentation;


/* Load specified component of some segmentation from file. This function
   returns 0 on success, -1 indicates an fopen failure. */
int __dt_LoadMeshSegComponent(
    const char *filename, __dt_MeshSegComponent *segcomp);

/* Destroy the segmentation component object and free its memory */
void __dt_DestroyMeshSegComponent(__dt_MeshSegComponent *segcomp);



#endif /* __DT_MESH_SEGMENTATION_HEADER__ */

