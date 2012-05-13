#ifndef __GL_INTERACT_HEADER__
#define __GL_INTERACT_HEADER__


#include "gl_util.h"
#include "layout.h"


/* Pick up a trianular element at screen coordinate (sx, sy) on the model */
int __gl_PickNearestObject(const dtMeshModel *model, int sx, int sy);


/* Keystroke callback functions 
   Be advised: key stroke events are handled directly by __callback_OnKeyDown,
   and it simply calls these corresponding functions to response arrived 
   events. */
void __callback_OnKeyDown(unsigned char key, int sx, int sy);

void __callback_OnPickup(int sx, int sy);       /* p */
void __callback_AppendCurrent(void);            /* a */
void __callback_SwitchRenderMode(void);         /* m */
void __callback_SwitchVertex(int sx, int sy);   /* c */
void __callback_WriteToFile(void);              /* w */
void __callback_Undo(void);                     /* u */



#endif /* __GL_INTERACT_HEADER__ */
