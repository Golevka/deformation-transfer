#ifndef __DT_PICKLIST_HEADER__
#define __DT_PICKLIST_HEADER__


#include "layout.h"


/* Append recently specified vertex to correspondence list */
void __dt_AppendCurrentPickupItem(void);

/* Set currently selected primitive */
void __dt_SetCurrentPickupItem(__gl_UIPanel panel, dt_index_type i_triangle);

/* Switch among three vertices of current selected triangle */
void __dt_SwitchVertex(__gl_UIPanel panel);

/* Check if the pickup list is empty. */
int __dt_IsPickupListEmpty(void);

/* Delete the newly appended item */
void __dt_PopPickupItem(void);


/* Render each vertex in the pickup list to current OpenGL viewport */
void __dt_RenderPickupList(
    const dtMeshModel *model, __gl_UIPanel panel, GLdouble dot_size);

/* Render current picked up item */
void __dt_RenderCurrentPickupItem(
    const dtMeshModel *model, __gl_UIPanel panel, GLdouble dot_size);


/* Save all hand-picked vertex correspondences to a constraint list file 
   Return value: 0 on success, -1 on failure. */
int __dt_SaveHandPickedCorrespondences(
    const char *filename,
    const dtMeshModel *source_model, const dtMeshModel *target_model);


#endif /* __DT_PICKLIST_HEADER__ */
