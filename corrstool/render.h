#ifndef __GL_RENDER_HEADER__
#define __GL_RENDER_HEADER__


#include "layout.h"


/* Modes for rendering models, similar to the distinction of glutSolid* and 
   glutWired*. User can hit 'm' to switch among these render modes */
typedef enum __gl_RenderMode_enum
{
    __gl_RENDER_SOLID,   /* solid model */
    __gl_RENDER_WIRED    /* wired framework */
} __gl_RenderMode;


/* Set current render mode to render_mode */
void __gl_SelectRenderMode(__gl_RenderMode render_mode);

/* Switch to the other render mode */
void __gl_SwitchRenderMode(void);


/* Specify the information to be posted to the bottom panel */
void __gl_SetStatusInformation(const char *info, ...);

/* Render the information string, only __gl_RenderBottom wants to call it. */
void __gl_DisplayStatusInformation(void);


/* Render/Refresh a specified panel */
void __gl_RenderPanel(__gl_UIPanel panel); 

/* Render panel - subroutines */
void __gl_RenderPanelL(void);
void __gl_RenderPanelR(void);
void __gl_RenderPanelBottom(void);
void __gl_RenderPanelBorder(void);



#endif /* __GL_RENDER_HEADER__ */
