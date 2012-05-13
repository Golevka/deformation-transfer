#include <stdlib.h>
#include <stdio.h>

#include "mesh_seg.h"


/* Load specified component of some segmentation from file. This function
   returns 0 on success, -1 indicates an fopen failure. */
int __dt_LoadMeshSegComponent(
    const char *filename, __dt_MeshSegComponent *segcomp)
{
    FILE *fp = fopen(filename, "r");
    dt_index_type i_entry = 0;    /* loop var */

    if (fp != NULL)
    {
        /* read number of triangle units in this seg component from the 
           first line of file. */
        fscanf(fp, "%d", &(segcomp->n_segtriangle));

        /* allocate for triangle index list */
        segcomp->i_segtriangle = (dt_index_type*)__dt_malloc(
            (size_t)segcomp->n_segtriangle * sizeof(__dt_MeshSegComponent));

        /* read indexes of triangle units in this seg component */
        for ( ; i_entry < segcomp->n_segtriangle; i_entry++) {
            fscanf(fp, "%d", &(segcomp->i_segtriangle[i_entry]));
        }

        return 0;    /* successfully done */
    }
    else {
        return -1;   /* fopen() failed */
    }
}

/* Destroy the segmentation component object and free its memory */
void __dt_DestroyMeshSegComponent(__dt_MeshSegComponent *segcomp) {
    free(segcomp->i_segtriangle);
}

