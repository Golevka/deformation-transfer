#include <GL/glut.h>
#include "layout.h"



static __gl_LayoutManager __layout_manager;


/* Initializes constants which determines the overlookings of the user 
   interface */
static void __gl_InitLayoutManager_impl(__gl_LayoutManager *layout, 
    GLsizei border_size, GLdouble upper_lower_ratio)
{
    layout->border_size = border_size;
    layout->upper_lower_ratio = upper_lower_ratio;
}

void __gl_InitLayoutManager(GLsizei border_size, GLdouble upper_lower_ratio)
{
    __gl_InitLayoutManager_impl(
        &__layout_manager, border_size, upper_lower_ratio);
}


/* Recalculate panel positions and sizes according to new window size */
static void __gl_UpdateLayout_impl(__gl_LayoutManager *layout, 
    GLsizei wnd_width, GLsizei wnd_height)
{
    GLsizei border_size   = layout->border_size;
    GLsizei client_width  = (GLsizei)(wnd_width  - 2*border_size);
    GLsizei client_height = (GLsizei)(wnd_height - 3*border_size);

    /* update window metrics */
    layout->wnd_width  = (GLsizei)wnd_width;
    layout->wnd_height = (GLsizei)wnd_height;

    /* bottom panel */
    layout->panelB_x = layout->panelB_y = border_size;
    layout->panelB_width  = client_width;
    layout->panelB_height = 
        (GLsizei)(client_height/(1.0 + layout->upper_lower_ratio));

    /* left/right panel */
    layout->panelU_height = client_height - layout->panelB_height;
    layout->panelU_width  = (client_width - border_size) / 2;

    layout->panelL_x = border_size;
    layout->panelR_x = border_size + layout->panelU_width + border_size;
    layout->panelL_y = layout->panelR_y = 
        border_size + layout->panelB_height + border_size;
}

void __gl_UpdateLayout(GLsizei wnd_width, GLsizei wnd_height)
{
    __gl_UpdateLayout_impl(
        &__layout_manager, wnd_width, wnd_height);
}


/* Swith to specified viewport (panel) */
static void __gl_SetCurrentView_impl(
    const __gl_LayoutManager *layout, __gl_UIPanel view)
{
    GLint x = 0, y = 0;
    GLsizei width = 0, height = 0;

    switch (view)
    {
    case __gl_PANEL_LEFTVIEW: 
        x = layout->panelL_x, y = layout->panelL_y;
        width = layout->panelU_width, height = layout->panelU_height;
        break;

    case __gl_PANEL_RIGHTVIEW:
        x = layout->panelR_x, y = layout->panelR_y;
        width = layout->panelU_width, height = layout->panelU_height;
        break;

    case __gl_PANEL_BOTTOMVIEW: 
        x = layout->panelB_x, y = layout->panelB_y;
        width = layout->panelB_width, height = layout->panelB_height;
        break;

    case __gl_PANEL_BACKGND:
        x = y = 0;
        width  = layout->wnd_width; height = layout->wnd_height;
        break;
        
    default:
        break;
    }

    glViewport(x, y, width, height);
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, width, height);

    /* adjust frustrum */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
        cam_fovy, (GLdouble)width/(GLdouble)height, cam_zNear, cam_zFar);

    glMatrixMode(GL_MODELVIEW);   /* ready for rendering objects */
}

void __gl_SetCurrentView(__gl_UIPanel view) {
    __gl_SetCurrentView_impl(&__layout_manager, view);
}


/* Adjust panel layout when the window is reshaped, this callback function
   should be registered to GLUT event system. */
void __gl_callback_OnResize(int width, int height)
{
    __gl_UpdateLayout(width, height);
    glutPostRedisplay();
}


/* Get the viewport metric of specified panel */
void __gl_GetViewportMetrics_impl(
    const __gl_LayoutManager *layout, __gl_UIPanel panel, GLint *viewport)
{
    if (panel == __gl_PANEL_LEFTVIEW)
    {
        viewport[0] = layout->panelL_x;
        viewport[1] = layout->panelL_y;
        viewport[2] = layout->panelU_width;
        viewport[3] = layout->panelU_height;
    }
    else if (panel == __gl_PANEL_RIGHTVIEW)
    {
        viewport[0] = layout->panelR_x;
        viewport[1] = layout->panelR_y;
        viewport[2] = layout->panelU_width;
        viewport[3] = layout->panelU_height;
    }
    else if (panel == __gl_PANEL_BOTTOMVIEW)
    {
        viewport[0] = layout->panelB_x;
        viewport[1] = layout->panelB_y;
        viewport[2] = layout->panelB_width;
        viewport[3] = layout->panelB_height;
    }
    else if (panel == __gl_PANEL_BACKGND)
    {
        viewport[0] = viewport[1] = 0;
        viewport[2] = layout->wnd_width;
        viewport[3] = layout->wnd_height;
    }
}

void __gl_GetViewportMetrics(__gl_UIPanel panel, GLint *viewport) {
    __gl_GetViewportMetrics_impl(&__layout_manager, panel, viewport);
}


/* Convert from GLUT screen coordinate to panel viewport coordinate */
static void __gl_Screen2Panel_impl(const __gl_LayoutManager *layout, 
    __gl_UIPanel panel, int sx, int sy, GLint *x, GLint *y)
{
    sy = layout->wnd_height - sy;
    switch (panel)
    {
    case __gl_PANEL_LEFTVIEW: 
        *x = sx - layout->panelL_x, *y = sy - layout->panelL_y;
        break;

    case __gl_PANEL_RIGHTVIEW:
        *x = sx - layout->panelR_x, *y = sy - layout->panelR_y;
        break;

    case __gl_PANEL_BOTTOMVIEW:
        *x = sx - layout->panelB_x, *y = sy - layout->panelB_y;
        break;

    case __gl_PANEL_BACKGND:
        *x = sx, *y = sy;
        break;
    }
}

void __gl_Screen2Panel(__gl_UIPanel panel, int sx, int sy, GLint *x, GLint *y){
    __gl_Screen2Panel_impl(&__layout_manager, panel, sx, sy, x, y);
}


/* Figure out which panel the specified point is lying on
   Attention: the point (sx, sy) you specified is in GLUT coordinate system.
 */
static int __gl_helper_IsInRegion(int x, int y, 
    int x0, GLsizei width, int y0, GLsizei height)
{
    return (
        x >= x0 && x <= x0 + width &&    /* x is in range */
        y >= y0 && y <= y0 + height);    /* y is in range */
}

static __gl_UIPanel __gl_SelectPanelbyPoint_impl(
    const __gl_LayoutManager *layout, int sx, int sy)
{
    /* convert screen coordinate to viewport coordinate */
    int x = sx, y = layout->wnd_height - sy;

    if (__gl_helper_IsInRegion(x, y, 
            layout->panelB_x, layout->panelB_width,
            layout->panelB_y, layout->panelB_height))
    {
        return __gl_PANEL_BOTTOMVIEW;
    }
    else if (__gl_helper_IsInRegion(x, y,
            layout->panelL_x, layout->panelU_width,
            layout->panelL_y, layout->panelU_height))
    {
        return __gl_PANEL_LEFTVIEW;
    }
    else if (__gl_helper_IsInRegion(x, y,
            layout->panelR_x, layout->panelU_width,
            layout->panelR_y, layout->panelU_height))
    {
        return __gl_PANEL_RIGHTVIEW;
    }
    else {
        return __gl_PANEL_BACKGND;
    }
}

__gl_UIPanel __gl_SelectPanelbyPoint(int sx, int sy) {
    return __gl_SelectPanelbyPoint_impl(&__layout_manager, sx, sy);
}
