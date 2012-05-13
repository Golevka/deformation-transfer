#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "mesh_model.h"



/* Skip current line / jump to the start of the next line */
static void __skip_this_line(FILE *fd)
{
    /* **skip all characters until encountering an '\n'.**

       It might be **PAINFULLY SLOW**, but using fgets would mess up the code 
       because you have to make sure that the entire line has been eventually 
       read into the temporary buffer.
    */
    int ret;
    while ((ret = fgetc(fd)) != '\n' && ret != EOF);
}

/* Count how many vertices, normal vectors and triangle surfaces were logged in
   the .obj file. 

   This function returns the file pointer of the .obj file on success, or it
   would return NULL to indicate an fopen() error. the user should check errno 
   to investigate why it failed.
 */
static FILE *__get_mesh_model_scale(
    const char *filename, 
    dt_size_type *n_vertex, dt_size_type *n_normvec, dt_size_type *n_triangle)
{
    char pref[3];  /* we need only 2 characters to identify the prefixes */

    FILE *fd = fopen(filename, "r");
    if (fd)
    {
        *n_vertex = *n_normvec = *n_triangle = 0;

        /* inspect the prefix of each line:
           #:comment   v:vertex   vn:norm vector   f:triangle */
        while (fscanf(fd, "%2s", pref) != EOF)
        {
            if      (strcmp(pref, "v")  == 0)  *n_vertex   += 1;
            else if (strcmp(pref, "vn") == 0)  *n_normvec  += 1;
            else if (strcmp(pref, "f")  == 0)  *n_triangle += 1;
            /* else if (*pref == '#');  <comment line>       */
            /* else:                    <unexpected prefix>  */

            __skip_this_line(fd);
        }

        /* reset file pointer to the beginning, bcos ReadObjFile() will make a
           second pass of the file and read the data into model structure. */
        fseek(fd, SEEK_SET, 0);
    }
    else {
        return NULL;   /* could not open file */
    }

    return fd;
}


/* ReadObjFile parse specified .obj model description file and read vertex, 
   normal vector and triangular surface information into *model

   This function returns 0 on success, otherwise it would return an -1 to 
   indicate an fopen() error (file not exist, privillage or some reason).
*/
int ReadObjFile(const char *filename, dtMeshModel *model)
{
    char pref[3];   /* we need only 2 characters to identify the prefixes */

    dtVertex *vertex; dtVector *normvec; dtTriangle *triangle;
    dt_index_type i_lv;

    FILE *fd = __get_mesh_model_scale(
        filename, &model->n_vertex, &model->n_normvec, &model->n_triangle);

    if (fd)
    {
        /* allocate memory space for mesh model structure */
        CreateMeshModel(model);
        vertex   = model->vertex;
        normvec  = model->normvec;
        triangle = model->triangle;

        /* parse .obj file */
        while (fscanf(fd, "%2s", pref) != EOF)
        {
            if (strcmp(pref, "v") == 0)        /* vertex */
            {
                fscanf(fd, "%lf %lf %lf", 
                    &(vertex->x), &(vertex->y), &(vertex->z));
                vertex++;
            }
            else if (strcmp(pref, "vn") == 0)  /* norm vector */
            {
                fscanf(fd, "%lf %lf %lf",
                    &(normvec->x), &(normvec->y), &(normvec->z));
                normvec++;
            }
            else if (strcmp(pref, "f") == 0)   /* triangle */
            {
                /* vertex/normvec indexes in .obj file are one-based, we have 
                   to convert them to zero-based array indexes latter */

                /* fscanf(fd, "%d//%d %d//%d %d//%d", */
                /*     &(triangle->i_vertex[0]), &(triangle->i_norm[0]), */
                /*     &(triangle->i_vertex[1]), &(triangle->i_norm[1]), */
                /*     &(triangle->i_vertex[2]), &(triangle->i_norm[2])); */
                for (i_lv = 0; i_lv < 3; i_lv++)
                {
                    fscanf(fd, "%d/", &(triangle->i_vertex[i_lv]));
                    if ((pref[0] = (char)fgetc(fd)) != '/') {
                        ungetc(pref[0], fd);  fscanf(fd, "%*d/");
                    }
                    fscanf(fd, "%d", &(triangle->i_norm[i_lv]));
                }

                /* one-based .obj file index => zero-based array index */
                triangle->i_vertex[0]--, triangle->i_norm[0]--;
                triangle->i_vertex[1]--, triangle->i_norm[1]--;
                triangle->i_vertex[2]--, triangle->i_norm[2]--;

                triangle++;
            }
            else if (*pref == '#')             /* comment line */
                __skip_this_line(fd);

            else  __skip_this_line(fd);        /* ignore other informations */
        }

        fclose(fd);
        return 0;
    }
    else {
        return -1; /* fopen() error propagated from __get_mesh_model_scale() */
    }
}


/* SaveObjFile saves specified mesh model to a file, it would return 0 on 
   success, otherwise it would return -1 to indicate fopen() was failed.
   You should check system variable errno for further investigation.
*/
int SaveObjFile(const char *filename, const dtMeshModel *model)
{
    const  dtVertex   *vertex   = model->vertex;
    const  dtVector   *normvec  = model->normvec;
    const  dtTriangle *triangle = model->triangle;
    dt_index_type ind;

    FILE *fd = fopen(filename, "w");
    if (fd != NULL)
    {
        /* save vertex information */
        for (ind = 0; ind < model->n_vertex; ind++, vertex++) {
            fprintf(fd, "v   %12.9f   %12.9f   %12.9f\n", 
                vertex->x, vertex->y, vertex->z);
        }
    
        /* save normal vector information */
        for (ind = 0; ind < model->n_normvec; ind++, normvec++) {
            fprintf(fd, "vn   %12.9f   %12.9f   %12.9f\n", 
                normvec->x, normvec->y, normvec->z);
        }

        /* save triangular unit information 
           
           Notice that vertex/normvec indexes in .obj file are one-based, so
           they need a incrementation before being written to filestream. */
        for (ind = 0; ind < model->n_triangle; ind++, triangle++)
        {
            fprintf(fd, "f %d//%d %d//%d %d//%d\n",
                triangle->i_vertex[0] + 1, triangle->i_norm[0] + 1,
                triangle->i_vertex[1] + 1, triangle->i_norm[1] + 1,
                triangle->i_vertex[2] + 1, triangle->i_norm[2] + 1);
        }

        fclose(fd);
        return 0;
    }
    else {
        return -1;
    }
}
