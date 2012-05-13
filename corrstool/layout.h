#ifndef __GL_LAYOUT_MANAGER_HEADER__
#define __GL_LAYOUT_MANAGER_HEADER__


#include "common.h"



/* The overall layout of the user interface of this application looks like
     ---------------------------------------------------------
     |                           |                           |
     |      LEFT VIEW PANEL      |     RIGHT VIEW PANEL      |
     |                           |                           |
     ---------------------------------------------------------
     |               STATUS/INFORMATION VIEW                 |
     ---------------------------------------------------------
*/
typedef enum __gl_UIPanel_enum
{
    __gl_PANEL_LEFTVIEW,
    __gl_PANEL_RIGHTVIEW,
    __gl_PANEL_BOTTOMVIEW,
    __gl_PANEL_BACKGND
} __gl_UIPanel;


/* Layout manager changes the layout of panels properly on window moving and 
   resizing  
*/
typedef struct __gl_LayoutManager_struct
{
    GLsizei wnd_width, wnd_height;  /* size of the entire client area */

    /* These values determines the looking and feel of the GUI interface.
     */
    GLsizei  border_size;           /* thickness of panel borders, in pixel */
    GLdouble upper_lower_ratio;     /* view_panel_size/info_panel_size  */

    /* Size of three main panels of the application window, all of these values
     * can would be recalculated when the window was resized.
     */
    GLint panelL_x, panelL_y;   /* lower-left corner of viewport for panelL*/
    GLint panelR_x, panelR_y;   /* ... viewport location for panelR */
    GLint panelB_x, panelB_y;   /* position of bottom panel should always be 
                                   (border_size, border_size) */

    GLsizei panelU_width, panelU_height;  /* size of upper model view panel */
    GLsizei panelB_width, panelB_height;  /* size of the status/info bar at the
                                             bottom */
} __gl_LayoutManager;



/* Initializes constants which determines the overlookings of the user 
   interface */
void __gl_InitLayoutManager(GLsizei border_size, GLdouble upper_lower_ratio);

/* Recalculate panel positions and sizes according to new window size */
void __gl_UpdateLayout(GLsizei wnd_width, GLsizei wnd_height);

/* Swith to specified viewport (panel) */
void __gl_SetCurrentView(__gl_UIPanel view);

/* Get the viewport metric of specified panel */
void __gl_GetViewportMetrics(__gl_UIPanel panel, GLint *viewport);

/* Convert from GLUT screen coordinate to panel viewport coordinate */
void __gl_Screen2Panel(__gl_UIPanel panel, int sx, int sy, GLint *x, GLint *y);

/* Figure out which panel the specified point is lying on
   Attention: the point (sx, sy) you specified is in GLUT coordinate system.
 */
__gl_UIPanel __gl_SelectPanelbyPoint(int sx, int sy);

/* Adjust panel layout when the window is reshaped, this callback function
   should be registered to GLUT event system. */
void __gl_callback_OnResize(int width, int height);



#endif /* __GL_LAYOUT_MANAGER_HEADER__ */
