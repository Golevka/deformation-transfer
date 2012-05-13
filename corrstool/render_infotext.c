#include <stdarg.h>
#include <stdio.h>

#include "gl_util.h"
#include "render.h"


static char __info_string[1024];    /* buffer for status information */


/* Specify the information to be posted to the bottom panel */
void __gl_SetStatusInformation(const char *info, ...)
{
    va_list args;
    va_start(args, info);
    vsprintf(__info_string, info, args);
    va_end(args);
}

/* Render the information string, only __gl_RenderBottom wants to call it. */
void __gl_DisplayStatusInformation(void)
{
    float text_color[4] = {TEXT_COLOR, 1};
    DisplayText2D(__info_string, INFOTEXT_POS_X, INFOTEXT_POS_Y, 100, 100, 
        text_color, GLUT_BITMAP_8_BY_13);
}
