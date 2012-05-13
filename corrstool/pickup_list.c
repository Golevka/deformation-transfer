#include "constraint.h"
#include "gl_util.h"
#include "pickup_list.h"



typedef struct __dt_PickupItem_struct
{
    dt_index_type i_triangle;     /* which triangle is selected? */
    dt_index_type i_localvertex;  /* which vertex of the triangle? 
                                     i_vertex[0], i_vertex[1], or i_vertex[2]? */
} __dt_PickupItem;

typedef struct __dt_PickupList_struct
{
    __dt_PickupItem pickup_list[4096], current_item;
    dt_size_type list_length;

} __dt_PickupList;


/* list of picked up vertices */
static __dt_PickupList modelL_list = 
{
    {{0, 0}},          /* pickup_list[4096] */
    {0, 0},            /* current_item */
    0                  /* list_length */
}, 
    /* no need to explain again, maybe */
    modelR_list = {{{0, 0}}, {0, 0}, 0}; 



/* left model viewer  --- modelL_list
   right model viewer --- modelR_list
*/
static __dt_PickupList* __get_pickuplist_from_panel(__gl_UIPanel panel)
{
    switch (panel) {
        case __gl_PANEL_LEFTVIEW:   return &modelL_list;
        case __gl_PANEL_RIGHTVIEW:  return &modelR_list;

    default: return NULL;
    }
}


/* Append recently specified vertex to correspondence list */
static void __append_current(__dt_PickupList *list) {
    list->pickup_list[list->list_length++] = list->current_item;
}

/* Append recently specified vertex to correspondence list */
void __dt_AppendCurrentPickupItem(void)
{
    __append_current(&modelL_list);
    __append_current(&modelR_list);
}

/* Check if the pickup list is empty. */
int __dt_IsPickupListEmpty(void)
{
    return  
        ((modelL_list.list_length == 0) || 
        (modelR_list.list_length == 0));
}

/* Delete the newly appended item */
void __dt_PopPickupItem(void)
{
    modelL_list.list_length--;
    modelR_list.list_length--;
}

/* Set currently selected primitive */
static void __set_current(__dt_PickupList *list, dt_index_type i_triangle)
{
    list->current_item.i_triangle = i_triangle;
    list->current_item.i_localvertex = 0;
}

void __dt_SetCurrentPickupItem(__gl_UIPanel panel, dt_index_type i_triangle) {
    __set_current(__get_pickuplist_from_panel(panel), i_triangle);
}


/* Switch among three vertices of current selected triangle */
static void __switch_vertex(__dt_PickupList *list)
{
    list->current_item.i_localvertex = 
        (list->current_item.i_localvertex + 1) % 3;
}

void __dt_SwitchVertex(__gl_UIPanel panel) {
    __switch_vertex(__get_pickuplist_from_panel(panel));
}


/* Helper function: get global vertex from local vertex id in a specified  
   triangular unit 
*/
static dt_index_type __local_vertex_to_global_vertex(
    const dtMeshModel *model, 
    dt_index_type i_triangle, dt_index_type i_localvertex)
{
    return  model->triangle[i_triangle].i_vertex[i_localvertex];
}

static dtVertex* __local_vertex_to_dtVertex(
    const dtMeshModel *model, 
    dt_index_type i_triangle, dt_index_type i_localvertex)
{
    return model->vertex + 
        __local_vertex_to_global_vertex(model, i_triangle, i_localvertex);
}


/* Render each vertex in the pickup list to current OpenGL viewport */
static void __render_list(const dtMeshModel *model, __dt_PickupList *list,
    GLdouble dot_size)
{
    dt_index_type i_item = 0;
    dt_index_type i_triangle, i_localvertex;
    dtVertex *vertex;

    for ( ; i_item < list->list_length; i_item++)
    {
        i_triangle = list->pickup_list[i_item].i_triangle;
        i_localvertex = list->pickup_list[i_item].i_localvertex;
        vertex = __local_vertex_to_dtVertex(model, i_triangle, i_localvertex);

        RenderDot(vertex->x, vertex->y, vertex->z,
            dot_size, DOT_QUALITY, DOT_QUALITY);
    }
}

void __dt_RenderPickupList(
    const dtMeshModel *model, __gl_UIPanel panel, GLdouble dot_size) {
    __render_list(model, __get_pickuplist_from_panel(panel), dot_size);
}


/* Render current picked up item */
static void __render_current(const dtMeshModel *model, __dt_PickupList *list,
    GLdouble dot_size)
{
    dt_index_type i_triangle = list->current_item.i_triangle;
    dt_index_type i_localvertex = list->current_item.i_localvertex;
    dtVertex *vertex = 
        __local_vertex_to_dtVertex(model, i_triangle, i_localvertex);

    RenderDot(vertex->x, vertex->y, vertex->z,
        dot_size, DOT_QUALITY, DOT_QUALITY);
}

void __dt_RenderCurrentPickupItem(
    const dtMeshModel *model, __gl_UIPanel panel, GLdouble dot_size) {
    __render_current(model, __get_pickuplist_from_panel(panel), dot_size);
}


/* Save all hand-picked vertex correspondences to a constraint list file */
static int __save_handpicked_correspondences(
    const char *filename,
    const dtMeshModel *source_model, const dtMeshModel *target_model,
    const __dt_PickupList *source_list, const __dt_PickupList *target_list)
{
    __dt_VertexConstraintList constraint_list;
    dt_index_type i_entry = 0;
    int ret;

    /* create a temporary constraint list object */
    constraint_list.list_length = source_list->list_length;
    __dt_CreateConstraintList(&constraint_list);

    /* fill in constraint entries */
    for ( ; i_entry < constraint_list.list_length; i_entry++)
    {
        constraint_list.constraint[i_entry].i_src_vertex = 
            __local_vertex_to_global_vertex(source_model, 
                source_list->pickup_list[i_entry].i_triangle, 
                source_list->pickup_list[i_entry].i_localvertex);

        constraint_list.constraint[i_entry].i_tgt_vertex = 
            __local_vertex_to_global_vertex(target_model,
                target_list->pickup_list[i_entry].i_triangle,
                target_list->pickup_list[i_entry].i_localvertex);
    }

    ret = __dt_SaveConstraints(filename, &constraint_list);

    __dt_ReleaseConstraints(&constraint_list);
    return  ret;
}

int __dt_SaveHandPickedCorrespondences(
    const char *filename,
    const dtMeshModel *source_model, const dtMeshModel *target_model)
{
    return __save_handpicked_correspondences(filename, 
        source_model, target_model, &modelL_list, &modelR_list);
}
