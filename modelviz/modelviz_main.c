#include <GL/glut.h>
#include "mesh_model.h"
#include "gl_util.h"


extern dtMeshModel model;
extern const char *model_filename;

static GLdouble cx, cy, cz;  /* model position correction */

/* View control variables */
static int mouseLeftDown = 0, mouseRightDown = 0;
static GLint mouseX, mouseY;
static GLfloat
    cameraAngleX   = 0,
    cameraAngleY   = 0,
    cameraDistance = 2.0f;

#define  MODEL_COLOR  0.5f, 0.5f, 0.5f  /* gray */


static void do_render(void)
{
    static gl_Material material = {
        {0.5f, 0.5f, 0.6f, 1.0f},  /* ambient */
        {0.5f, 0.5f, 0.5f, 1.0f},  /* diffuse */
        {0.5f, 0.5f, 0.5f, 1.0f},  /* specular */
        {20.f, 0, 0,  0}           /* shininess */
    };

    SetCamera(0, 0, 0, 0, 0, -1);

    glTranslatef(0, 0, -cameraDistance); /* push the object away from cam */
    glRotatef(cameraAngleX, 1, 0, 0);    /* pitch */
    glRotatef(cameraAngleY, 0, 1, 0);    /* heading */
    glTranslated(cx, cy, cz);            /* centric */

    glColor3f(MODEL_COLOR);
    SetMaterial(&material);
    RenderMeshModel(&model);

    glFlush();
}



static void __callback_OnReshape(GLsizei width, GLsizei height)
{
    glViewport(0,0, width, height);
    SetCamPerspective(60, (GLdouble)width/(GLdouble)height, 0.01, 50);
}

static void __callback_OnRender(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    do_render();
    glutSwapBuffers();
}


static void __callback_OnMouseDown(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

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

static void __callback_OnMouseMove(int x, int y)
{
    if (mouseLeftDown)           /* rotate view */
    {
        cameraAngleY += (GLfloat)(x - mouseX);
        cameraAngleX += (GLfloat)(y - mouseY);
        mouseX = x; mouseY = y;
    }
    else if (mouseRightDown)     /* zoom view */
    {
        cameraDistance += (GLfloat)((y - mouseY) * 0.05);
        mouseY = y;
    }

    glutPostRedisplay();
}


static void __callback_OnKeyDown(unsigned char key, int sx, int sy)
{
    switch (key)
    {
        case 'r':   /* reload model */
            DestroyMeshModel(&model);
            __dt_ReadObjFile_commit_or_crash(model_filename, &model);
            glutPostRedisplay();
            break;
    }
}


int ModelViz_Main(int *pargc, char **argv)
{
    glutInit(pargc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(300,200);
    glutCreateWindow("ModelViz!");

    glutMouseFunc   (__callback_OnMouseDown);
    glutMotionFunc  (__callback_OnMouseMove);
    glutReshapeFunc (__callback_OnReshape);
    glutKeyboardFunc(__callback_OnKeyDown);
    glutDisplayFunc (__callback_OnRender);

    InitGL();

    SetCamPerspective(60.0, 1, 0.01, 50);
    CalculateModelPosCorrection(&model, &cx, &cy, &cz);

    glutMainLoop();
    return 0;
}
