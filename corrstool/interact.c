#include <stdio.h>
#include "interact.h"
#include "render.h"
#include "pickup_list.h"


GLdouble cx_l, cy_l, cz_l;  /* left-side model position correction */
GLdouble cx_r, cy_r, cz_r;  /* same thing for the right-side one */

/* View control variables */
static int mouseLeftDown = 0, mouseRightDown = 0;
static GLint mouseX, mouseY;

GLfloat cameraAngleX = 0,
        cameraAngleY = 0,
        cameraDistance_l = 2.0f,
        cameraDistance_r = 2.0f;


/* Refresh client region */
static void __callback_OnDisplay(void)
{
    /* Render all panels*/
    __gl_RenderPanel(__gl_PANEL_BACKGND);
    __gl_RenderPanel(__gl_PANEL_LEFTVIEW);
    __gl_RenderPanel(__gl_PANEL_RIGHTVIEW);
    __gl_RenderPanel(__gl_PANEL_BOTTOMVIEW);

    /* Render complete */
    glutSwapBuffers();
}

/* Trigger rotate or zoom mode */
static void __callback_OnMouseDown(int button, int state, int sx, int sy)
{
    mouseX = sx;
    mouseY = sy;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)     mouseLeftDown = 1;
        else if (state == GLUT_UP)  mouseLeftDown = 0;
    }
    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)     mouseRightDown = 1;
        else if (state == GLUT_UP)  mouseRightDown = 0;
    }
}

/* Rotate or zoom view */
static void __callback_OnMouseMove(int sx, int sy)
{
    __gl_UIPanel panel;

    if (mouseLeftDown)    /* rotate view */
    {
        cameraAngleY += (GLfloat)(sx - mouseX);
        cameraAngleX += (GLfloat)(sy - mouseY);
        mouseX = sx; mouseY = sy;
    }

    if (mouseRightDown)   /* zoom view */
    {
        panel = __gl_SelectPanelbyPoint(sx, sy);

        if (panel == __gl_PANEL_LEFTVIEW)
            cameraDistance_l += (GLfloat)((sy - mouseY) * 0.05);
        else if (panel == __gl_PANEL_RIGHTVIEW)
            cameraDistance_r += (GLfloat)((sy - mouseY) * 0.05);

        mouseY = sy;
    }

    glutPostRedisplay();
}


int Corres_Main(void)
{
    /* Calculate model position correction factors, we need to move the models
       slightly so that they can appear at the centric of the viewports. */
    CalculateModelPosCorrection(&modelL, &cx_l, &cy_l, &cz_l);
    CalculateModelPosCorrection(&modelR, &cx_r, &cy_r, &cz_r);

    /* Specify event handlers */
    glutDisplayFunc (__callback_OnDisplay);    /* refresh scence */
    glutReshapeFunc (__gl_callback_OnResize);  /* handled by layout manager */
    glutMouseFunc   (__callback_OnMouseDown);  /* clicking */
    glutMotionFunc  (__callback_OnMouseMove);  /* dragging */
    glutKeyboardFunc(__callback_OnKeyDown);    /* keystroke */

    __gl_SelectRenderMode(__gl_RENDER_SOLID);
    __gl_SetStatusInformation("ready");
    
    glutMainLoop();  /* GLUT event loop */
    return 0;
}
