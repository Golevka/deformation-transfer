#include <stdio.h>
#include "mesh_model.h"


int ModelViz_Main(int *pargc, char **argv);

dtMeshModel model;
const char *model_filename;

int main(int argc, char *argv[])
{
    int ret;
    if (argc == 2)
    {
        model_filename = argv[1];
        if ((ret = ReadObjFile(model_filename, &model)) == 0) {
            return ModelViz_Main(&argc, argv);
        }
        else if (ret == -1) {
            perror("Reading model file error");
        }
        else {
            fprintf(stderr, "Syntax error on line: %u\n", ret);
        }
        return -1;
    }
    else {
        printf(".OBJ model viewer\n"
               "Usage: %s <.obj model filename>\n", *argv);
    }

    return 0;
}
