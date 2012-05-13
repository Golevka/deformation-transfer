#include "gl_util.h"


void InitGL(void)
{
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_CULL_FACE);
    /* glEnable(GL_MULTISAMPLE); */

    /* track material ambient and diffuse from surface color */
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}

void SetCamera(
    GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, /* cam position */
    GLdouble atX, GLdouble atY, GLdouble atZ)    /* target position */
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        eyeX, eyeY, eyeZ,
        atX,  atY,  atZ,
        0.0,  1.0,  0.0);
}

void SetCamPerspective(
    GLdouble fov, GLdouble aspect, 
    GLdouble zNear, GLdouble zFar)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
}


void SetMaterial(const gl_Material *material)
{
    /* define material properties of all polygons */
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   material->ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   material->diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  material->specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
}


/* Draw a sphere at specified coordinate */
void RenderDot(GLdouble x, GLdouble y, GLdouble z, 
    GLdouble radius, GLint slices, GLint stacks)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslated(x, y, z);
    glutSolidSphere(radius, slices, stacks);

    glPopMatrix();
}


/* Display 2D text using GLUT */
void DisplayText2D(
    const char *str, int x, int y, int viewport_width, int viewport_height, 
    float color[4], void *font)
{
    /* Save model-view matrix and reset to identity */
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* Save projection matrix and set to an othogonal view */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, viewport_width, 0, viewport_height);

    /* We need to disable lighting for a proper text color */
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);

    glColor4fv(color);          /* set text color */
    glRasterPos2i(x, y);        /* place text position */

    /* iterate through all characters in the string */
    while(*str) {
        glutBitmapCharacter(font, *str++);
    }

    /* Restore lighting */
    glEnable(GL_LIGHTING);
    glPopAttrib();

    /* Restore projection matrix */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    /* Restore model-view matrix */
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


/* Render the mesh model using OpenGL 
   The model is rendered as a solid object */
void RenderMeshModel(const dtMeshModel *model)
{
    dt_index_type i_triangle;

    const  dtVertex   *vertex   = model->vertex;
    const  dtVector   *normvec  = model->normvec;
    const  dtTriangle *triangle = model->triangle;

    glBegin(GL_TRIANGLES);
    for (i_triangle = 0; i_triangle < model->n_triangle; 
         i_triangle++, triangle++)
    {
        glNormal3dv((GLdouble*)(normvec + triangle->i_norm[0]));
        glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[0]));
        glNormal3dv((GLdouble*)(normvec + triangle->i_norm[1]));
        glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[1]));
        glNormal3dv((GLdouble*)(normvec + triangle->i_norm[2]));
        glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[2]));
    }
    glEnd();
}


/* Render the model as a wire skeleton */
void RenderMeshModelWired(const dtMeshModel *model)
{
    dt_index_type i_triangle;

    const  dtVertex   *vertex   = model->vertex;
    const  dtVector   *normvec  = model->normvec;
    const  dtTriangle *triangle = model->triangle;

    for (i_triangle = 0; i_triangle < model->n_triangle; 
         i_triangle++, triangle++)
    {
        glBegin(GL_LINE_LOOP);
            glNormal3dv((GLdouble*)(normvec + triangle->i_norm[0]));
            glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[0]));
            glNormal3dv((GLdouble*)(normvec + triangle->i_norm[1]));
            glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[1]));
            glNormal3dv((GLdouble*)(normvec + triangle->i_norm[2]));
            glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[2]));
        glEnd();
    }
}


/* Load names for each triangular units of the model, this function should be 
   used in GL_SELECT mode.
*/
void NamedRenderMeshModel(const dtMeshModel *model)
{
    dt_index_type i_triangle;

    const  dtVertex   *vertex   = model->vertex;
    const  dtTriangle *triangle = model->triangle;

    for (i_triangle = 0; i_triangle < model->n_triangle; 
         i_triangle++, triangle++)
    {
        glLoadName((GLuint)i_triangle);
        glBegin(GL_TRIANGLES);
            glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[0]));
            glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[1]));
            glVertex3dv((GLdouble*)(vertex  + triangle->i_vertex[2]));
        glEnd();
    }
}


/* Calculate correction of positions so that the model can be placed at the 
   central of the viewport by performing glTransform().  
*/
void CalculateModelPosCorrection(
    const dtMeshModel *model, 
    GLdouble *cx, GLdouble *cy, GLdouble *cz)
{
    const dtVertex *vertex = model->vertex;
    GLdouble 
        xmax = vertex->x, ymax = vertex->y, zmax = vertex->z,
        xmin = vertex->x, ymin = vertex->y, zmin = vertex->z;

    dt_index_type i = 1;
    for ( ; i < model->n_vertex; i++)
    {
        xmax = xmax < vertex[i].x? vertex[i].x: xmax;
        ymax = ymax < vertex[i].y? vertex[i].y: ymax;
        zmax = zmax < vertex[i].z? vertex[i].z: zmax;

        xmin = xmin > vertex[i].x? vertex[i].x: xmin;
        ymin = ymin > vertex[i].y? vertex[i].y: ymin;
        zmin = zmin > vertex[i].z? vertex[i].z: zmin;
    }

    *cx = -0.5 * (xmin + xmax);
    *cy = -0.5 * (ymin + ymax);
    *cz = -0.5 * (zmin + zmax);
}

