#ifndef __DT_CLOSEST_POINT_HEADER__
#define __DT_CLOSEST_POINT_HEADER__


#include "mesh_model.h"
#include "3dtree.h"


/* A list of indexes of each source vertex's closest vertex on target mesh */
typedef struct __dt_SpatialJoinList_struct
{
    dt_size_type  list_length;
    dt_index_type *i_target_vertex;  /* i_target_vertex[i] is the index of the
                                        closest vertex on target mesh to 
                                        vertex i on source mesh */
} __dt_SpatialJoinList;


/* Allocate space for spatial join object, this function won't perform any 
   closest point search so you have to call __dt_ResolveModelSpatialJoin()
   later. */
void __dt_CreateSpatialJoinList(
    const dtMeshModel *source_model, __dt_SpatialJoinList *spjlist);

/* Build a 3d tree with vertices of specified model for fast nearest point 
   search, it is quite handy in correspondence phase 2 - closest point 
   iteration. */
__3dTree __dt_Build3DTree_Vertex(const dtMeshModel *model);

void __dt_DestroySpatialJoinList(__dt_SpatialJoinList *spjlist);



/* Resolve the spatial join between source model and target model using a 
   3d tree of the target model, this routine is way more effiecient than
   the brute force one. */
void __dt_ResolveModelSpatialJoin(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const dt_index_type *src_inorm_list,
    const dt_index_type *tgt_inorm_list,
    __3dTree tree_tgt, __dt_SpatialJoinList *spjlist);

/* Resolve the spatial join between source model and target model, this routine
   uses a naive brute force O(m*n) method, which might be slow for even small
   or medium size models. KD tree might be much faster in 3D case. */
void __dt_ResolveModelSpatialJoin_BruteForce(
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const dt_index_type *src_inorm_list,
    const dt_index_type *tgt_inorm_list,
    __dt_SpatialJoinList *spjlist);



#endif /* __DT_CLOSEST_POINT_HEADER__ */
