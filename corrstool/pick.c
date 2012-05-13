#include "gl_util.h"
#include "interact.h"



/* This routine iterates through all hitten objects and figure out which one is
   nearest to us. 

   This function would return the name of the nearest hitten object on success,
   or it would give an -1 to indicate that no objects has been picked up.
 */
static int __gl_ProcessHits(GLint hits, const GLuint *selectBuf)
{
    GLuint i_obj;          /* name(id, index) of the selected object */
    GLuint depth, min_z;   /* distance of the object from cam */
    int i;

    if (hits)
    {
        i_obj = selectBuf[3];  /* name of the first hitten object */
        min_z = selectBuf[1];  /* initialize min_z with the depth value of the 
                                  first hitten object */
        for (i = 1; i < hits; i++)
        {
            depth = selectBuf[i*4 + 1];
            if (depth < min_z)   /* if this is an upper layer object? */
            {
                i_obj = selectBuf[i*4 + 3]; /* name of this object */
                min_z = depth;
            }
        }
        return (int)i_obj;
    }
    else {
        return -1;  /* no object is hitten (picked up) */
    }
}



/* Pick up a trianular element at screen coordinate (sx, sy) on the model */
int __gl_PickNearestObject(const dtMeshModel *model, int sx, int sy)
{
#define PICK_TOL 0.0001

    GLuint selectBuf[512];
    GLint hits;
    GLint viewport[4];
    GLint x, y;

    glGetIntegerv(GL_VIEWPORT, viewport);

    /* Initialize object name stack */
    glSelectBuffer(sizeof(selectBuf)/sizeof(GLuint), selectBuf);
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(0);

    /* Start picking... specifying pick projection */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    /* Define an selection region, the coordnate specified in gluPickMatrix
       should be converted from screen coordinate to viewport coordinate */
    __gl_Screen2Panel(__gl_PANEL_BACKGND, sx, sy, &x, &y);
    gluPickMatrix(x, y, PICK_TOL, PICK_TOL, viewport);
    gluPerspective(cam_fovy,
        ((GLdouble)viewport[2])/((GLdouble)viewport[3]),
        cam_zNear, cam_zFar);

    NamedRenderMeshModel(model);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glFlush();

    /* Done! */
    hits = glRenderMode(GL_RENDER);
    return __gl_ProcessHits(hits, selectBuf);
}
