#ifndef __GL_ULTILS_HEADER__
#define __GL_ULTILS_HEADER__


#include <GL/glut.h>
#include "mesh_model.h"


/* Lighting behaviors of a material, you can set the current material
   state of OpenGL by calling SetMaterial() 
*/
typedef struct __gl_Material_struct
{
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat shininess[4];
} gl_Material;

void SetMaterial(const gl_Material *material);


void InitGL(void);  /* Enable depth test, auto normal and lighting */

void SetCamera(
    GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,  /* cam position */
    GLdouble atX, GLdouble atY, GLdouble atZ);    /* target position */

void SetCamPerspective(
    GLdouble fov, GLdouble aspect, 
    GLdouble zNear, GLdouble zFar);


/* Draw a sphere at specified coordinate */
void RenderDot(GLdouble x, GLdouble y, GLdouble z, 
    GLdouble radius, GLint slices, GLint stacks);

/* Display 2D text using GLUT */
void DisplayText2D(
    const char *str, int x, int y, int viewport_width, int viewport_height, 
    float color[4], void *font);


/* Render the mesh model using OpenGL */
void RenderMeshModel(const dtMeshModel *model);

/* Render the model as a wire skeleton */
void RenderMeshModelWired(const dtMeshModel *model);

/* Load names for each triangular units of the model, this function should be 
   used in GL_SELECT mode. */
void NamedRenderMeshModel(const dtMeshModel *model);


/* Calculate correction of positions so that the model can be placed at the 
   central of the viewport by performing glTransform(). */
void CalculateModelPosCorrection(
    const dtMeshModel *model, 
    GLdouble *cx, GLdouble *cy, GLdouble *cz);



#endif /* __GL_ULTILS_HEADER__ */
