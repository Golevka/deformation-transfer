#include "gl_util.h"
#include "render.h"
#include "interact.h"
#include "pickup_list.h"



/* Render a model with specified model-view parameters */
static void __gl_RenderModel(const dtMeshModel *model,
    GLfloat cam_pitch, GLfloat cam_heading, GLfloat cam_dist,
    GLdouble cx, GLdouble cy, GLdouble cz /* model position calibration */)
{
    extern  void(*__render_method)(const dtMeshModel *model);

    glTranslatef(0, 0, -cam_dist);    /* push the object away from cam */
    glRotatef(cam_pitch, 1, 0, 0);    /* pitch */
    glRotatef(cam_heading, 0, 1, 0);  /* heading */
    glTranslated(cx, cy, cz);         /* centric */
    
    __render_method(model);
}


/* Lighting properties of the models */
static gl_Material material = {
    {0.5f, 0.5f, 0.6f, 1.0f},     /* ambient */
    {0.5f, 0.5f, 0.5f, 1.0f},     /* diffuse */
    {0.5f, 0.5f, 0.5f, 1.0f},     /* specular */
    {20.f, 0, 0, 0}               /* shininess */
};


/* left-side model viewer */
void __gl_RenderPanelL(void)
{
    /* cam orientation and position */
    extern GLfloat  cameraAngleX, cameraAngleY, cameraDistance_l;
    extern GLdouble cx_l, cy_l, cz_l;  /* model position calibration */


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SetCamera(0, 0, 0, 0, 0, -1);

    glColor3f(MODEL_COLOR);
    SetMaterial(&material);
    __gl_RenderModel(&modelL, 
        cameraAngleX, cameraAngleY, cameraDistance_l, cx_l, cy_l, cz_l);


    /* render current vertex */
    glColor3f(DOT_CURRENT_COLOR);
    __dt_RenderCurrentPickupItem(&modelL, __gl_PANEL_LEFTVIEW, 
        DOT_CURRENT_SIZE * cameraDistance_l);

    /* render selected vertices */
    glColor3f(DOT_PICKEDUP_COLOR);
    __dt_RenderPickupList(&modelL, __gl_PANEL_LEFTVIEW,
        DOT_PICKEDUP_SIZE * cameraDistance_l);

    glFlush();
}

/* right-side model viewer */
void __gl_RenderPanelR(void)
{
    /* cam orientation and position */
    extern GLfloat  cameraAngleX, cameraAngleY, cameraDistance_r;
    extern GLdouble cx_r, cy_r, cz_r;  /* model position calibration */


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SetCamera(0, 0, 0, 0, 0, -1);

    glColor3f(MODEL_COLOR);
    SetMaterial(&material);
    __gl_RenderModel(&modelR,
        cameraAngleX, cameraAngleY, cameraDistance_r, cx_r, cy_r, cz_r);


    /* render current vertex */
    glColor3f(DOT_CURRENT_COLOR);
    __dt_RenderCurrentPickupItem(&modelR, __gl_PANEL_RIGHTVIEW,
        DOT_CURRENT_SIZE * cameraDistance_r);

    /* render selected vertices */
    glColor3f(DOT_PICKEDUP_COLOR);
    __dt_RenderPickupList(&modelR, __gl_PANEL_RIGHTVIEW, 
        DOT_PICKEDUP_SIZE * cameraDistance_r);

    glFlush();
}

/* status/information panel at the bottom */
void __gl_RenderPanelBottom(void)
{
    extern GLfloat  cameraAngleX, cameraAngleY;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.0, 0.0, 1.0);

    /* draw a model-view indicator */
    SetCamera(0, 0, 0, 0, 0, -1);
    glTranslatef(0, 0, -2);
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);
    glutWireOctahedron();

    /* display status information */
    __gl_DisplayStatusInformation();

    glFlush();
}

/* border of the client area */
void __gl_RenderPanelBorder(void)
{
    glClearColor(BORDER_COLOR, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glClearColor(0,0,0,1.0);  /* switch back to black */
}
