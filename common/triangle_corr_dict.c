#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "triangle_corr_dict.h"


/* Create an empty triangle correspondence dictionary, it only allocates space
   for the lookup table (argv[]) and intialize it as an empty one. */
void __dt_CreateEmptyTriangleCorrsDict(
    const dtMeshModel *target_model, __dt_TriangleCorrsDict *tcdict)
{
    dt_index_type i_entry = 0;

    /* allocate for the lookup table */
    tcdict->corrsv_size = target_model->n_triangle;
    tcdict->corrsv = 
        (__dt_TriangleCorrsDict_entv*)__dt_malloc(
            (size_t)tcdict->corrsv_size * 
            sizeof(__dt_TriangleCorrsDict_entv));

    /* make the lookup table empty */
    for ( ; i_entry < tcdict->corrsv_size; i_entry++)
    {
        tcdict->corrsv[i_entry].n_corrstriangle = 0;
        tcdict->corrsv[i_entry].corrs  =  NULL;
    }

    /* make tclist empty */
    tcdict->tclist.list_capacity = 0;
    tcdict->tclist.list_length   = 0;
    tcdict->tclist.corr          = NULL;
}


/* Construct a __dt_TriangleCorrsDict object from a __dt_TriangleCorrsList, 
   tclist is then migrated into tcdict and cannot be destroyed explicitly. */
void __dt_CreateTriangleCorrsDict(
    const dtMeshModel *target_model,
    __dt_TriangleCorrsList *tclist, __dt_TriangleCorrsDict *tcdict)
{
    dt_index_type i_corrs = 0;   /* index of correspondence entry in tclist */
    dt_index_type i_tgt_triangle, i_cur = -1;

    /* create the corrs dictionary and allocate for the lookup table */
    __dt_CreateEmptyTriangleCorrsDict(target_model, tcdict);

    /* migrate tclist to tcdict */
    __dt_SortUniqueTriangleCorrsList(tclist);
    tcdict->tclist = *tclist;

    /* perform a linear scan to build the lookup table of tcdict */
    for ( ; i_corrs < tclist->list_length; i_corrs++)
    {
        i_tgt_triangle = tclist->corr[i_corrs].i_tgt_triangle;

        /* if we encoutered an entry for the next target triangle unit */
        if (i_cur != i_tgt_triangle)
        {
            /* log the new corrs entry */
            i_cur = i_tgt_triangle;
            tcdict->corrsv[i_cur].corrs = &(tclist->corr[i_corrs]);
        }

        tcdict->corrsv[i_cur].n_corrstriangle += 1;
    }
}


/* Destroy specified dictionary object and free its memory */
void __dt_DestroyTriangleCorrsDict(__dt_TriangleCorrsDict *tcdict)
{
    __dt_DestroyTriangleCorrsList(&(tcdict->tclist));
    free(tcdict->corrsv);
}



/* Functions to access entries in the correspondence dictionary, the 
   implementations are quite straight forward. */

__dt_TriangleCorrsDict_entv* __dt_GetTriangleCorrsEntryVector(
    const __dt_TriangleCorrsDict *tcdict, dt_index_type i_tgt_triangle)
{
    __DT_ASSERT(
        i_tgt_triangle < tcdict->corrsv_size,
            "Target triangle index out of bounds in "
            "__dt_GetTriangleCorrsEntryVector");

    return tcdict->corrsv + i_tgt_triangle;
}

__dt_TriangleCorrsEntry* __dt_GetTriangleCorrsEntry(
    const __dt_TriangleCorrsDict *tcdict, dt_index_type i_tgt_triangle,
    dt_index_type i_corrs_entry)
{
    const __dt_TriangleCorrsDict_entv *v_entry = 
        __dt_GetTriangleCorrsEntryVector(tcdict, i_tgt_triangle);

    __DT_ASSERT(
        i_corrs_entry < v_entry->n_corrstriangle,
        "Corrs entry index out of bounds in __dt_GetTriangleCorrsEntry");

    return v_entry->corrs + i_corrs_entry;
}

dt_size_type __dt_GetTriangleCorrsNumber(
    const __dt_TriangleCorrsDict *tcdict, dt_index_type i_tgt_triangle)
{
    const __dt_TriangleCorrsDict_entv *v_entry = 
        __dt_GetTriangleCorrsEntryVector(tcdict, i_tgt_triangle);

    return v_entry->n_corrstriangle;
}
