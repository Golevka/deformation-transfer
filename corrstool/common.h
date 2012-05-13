#include <GL/gl.h>
#include "mesh_model.h"



#define DEFAULT_CONS_FILENAME  "default.cons"

/* panel options */
#define  BORDER_SIZE        10
#define  BORDER_COLOR       0.1f, 0.1f, 0.1f  /* dark gray */
#define  VIEW_INFO_RATIO    5.0


/* camera perspective properties */
#define  CAM_FOVY           60
#define  CAM_ZNEAR          0.01
#define  CAM_ZFAR           50


/* pick-up indicators */
#define  DOT_CURRENT_SIZE   0.004
#define  DOT_PICKEDUP_SIZE  0.005    /* larger than DOT_CURRENT_SIZE so that
                                        picked-up dots can cover current dot*/
#define  DOT_CURRENT_COLOR  0.0f, 1.0f, 0.0f  /* green */
#define  DOT_PICKEDUP_COLOR 0.0f, 0.0f, 0.1f  /* blue */
#define  DOT_QUALITY        30      /* slices and stacks of the dot*/


/* status information text */
#define  INFOTEXT_POS_X     4
#define  INFOTEXT_POS_Y     75


/* object colors */
#define  MODEL_COLOR        0.5f, 0.5f, 0.5f  /* gray */
#define  MESH_COLOR         0.8f, 0.8f, 0.8f  /* light gray */
#define  TEXT_COLOR         1.0f, 1.0f, 1.0f  /* white */



/* GLUT window initial size and position */
#define  INIT_WND_WIDTH     800
#define  INIT_WND_HEIGHT    500
#define  INIT_WND_X         300
#define  INIT_WND_Y         200

/* GLUT window title */
#define  WND_TITLE          "Corres!"


/* camera perspective variables */
extern GLdouble cam_fovy, cam_zNear, cam_zFar;

/* two opened mesh models */
extern dtMeshModel modelL, modelR;
