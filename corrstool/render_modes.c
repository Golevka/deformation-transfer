#include "gl_util.h"
#include "interact.h"
#include "render.h"
#include "pickup_list.h"



GLdouble cam_fovy  = CAM_FOVY, 
         cam_zNear = CAM_ZNEAR, 
         cam_zFar  = CAM_ZFAR;



/* Method to render the model, wired framework style or solid style. */
void(*__render_method)(const dtMeshModel *model);


/* __gl_RENDER_SOLID -- RenderMeshModel
   __gl_RENDER_WIRED -- RenderMeshModelWired */
__gl_RenderMode __render_mode;


/* Set current render mode to render_mode */
void __gl_SelectRenderMode(__gl_RenderMode render_mode)
{
    /* switch to the other render mode */
    __render_mode = render_mode;

    switch (render_mode)
    {
    case __gl_RENDER_WIRED:
        __render_method = RenderMeshModelWired; break;

    case __gl_RENDER_SOLID:
        __render_method = RenderMeshModel; break;

    default:
        break;
    }
}

/* Switch to the other render mode */
void __gl_SwitchRenderMode(void)
{
    __gl_SelectRenderMode(
        __render_mode == __gl_RENDER_SOLID?
        __gl_RENDER_WIRED: __gl_RENDER_SOLID);
}


/* Render/Refresh the entire client area */
void __gl_RenderPanel(__gl_UIPanel panel)
{
    __gl_SetCurrentView(panel);
    switch (panel) {
        case __gl_PANEL_BACKGND:    __gl_RenderPanelBorder(); break;
        case __gl_PANEL_LEFTVIEW:   __gl_RenderPanelL();      break;
        case __gl_PANEL_RIGHTVIEW:  __gl_RenderPanelR();      break;
        case __gl_PANEL_BOTTOMVIEW: __gl_RenderPanelBottom(); break;
    }
}
