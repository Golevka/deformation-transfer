#include <stdio.h>

#include "gl_util.h"
#include "layout.h"


dtMeshModel modelL, modelR;  /* opened mesh models */
int Corres_Main(void);

const char *cons_filename = DEFAULT_CONS_FILENAME;


int main(int argc, char *argv[])
{
    if (argc == 3 || argc == 4)
    {
        if (argc == 4) cons_filename = argv[3];

        /* Read source and target model files */
        __dt_ReadObjFile_commit_or_crash(argv[1], &modelL);
        __dt_ReadObjFile_commit_or_crash(argv[2], &modelR);

        __gl_InitLayoutManager(BORDER_SIZE, VIEW_INFO_RATIO);

        /* Initialize OpenGL */
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
        glutInitWindowSize(INIT_WND_WIDTH, INIT_WND_HEIGHT);
        glutInitWindowPosition(INIT_WND_X, INIT_WND_Y);
        glutCreateWindow(WND_TITLE);
        InitGL();

        return Corres_Main();
    }
    else {
        printf("usage: %s model1 model2 [optional:output_file_name]\n", *argv);
    }

    return 0;
}
