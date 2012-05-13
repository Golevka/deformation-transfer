#include <stdlib.h>
#include "3dtree.h"



static void __3dtree_swap_real(dt_real_type *a0, dt_real_type *a1)
{
    dt_real_type tmp = *a0;
    *a0 = *a1;
    *a1 = tmp;
}

static void __3dtree_swap_exemplar(
    __3dtree_Exemplar *ex0, __3dtree_Exemplar *ex1)
{
    int id;
    dt_real_type tmp;

    /* swap coordinates */
    tmp = ex1->pt[0]; ex1->pt[0] = ex0->pt[0]; ex0->pt[0] = tmp;
    tmp = ex1->pt[1]; ex1->pt[1] = ex0->pt[1]; ex0->pt[1] = tmp;
    tmp = ex1->pt[2]; ex1->pt[2] = ex0->pt[2]; ex0->pt[2] = tmp;

    /* swap identifiers */
    id = ex1->id; ex1->id = ex0->id, ex0->id = id;
}


/* set the dimension with maximum variant as split dimension */
int __3dtree_select_split_dimension(
    __3dtree_Exemplar *ex_begin, __3dtree_Exemplar *ex_end)
{
    dt_real_type variant[3] = {0, 0, 0}, mean_val;
    int n_ex = (int)(ex_end - ex_begin);

    int i_split, i_ex, i_dim = 0;
    for ( ; i_dim < 3; i_dim++)
    {
        /* calculate mean value of coordinate of i_dim-th dimension */
        mean_val = 0;
        for (i_ex = 0; i_ex < n_ex; i_ex++) {
            mean_val += ex_begin[i_ex].pt[i_dim];
        }
        mean_val = mean_val / n_ex;

        /* calculate variant of coordinate of i_dim-th dimension */
        for (i_ex = 0; i_ex < n_ex; i_ex++) {
            variant[i_dim] += 
                (ex_begin[i_ex].pt[i_dim] - mean_val) * 
                (ex_begin[i_ex].pt[i_dim] - mean_val);
        }
    }

    /* select dimension with maximum variant as split dimension */
    i_split = (variant[0] > variant[1])? 0: 1;
    return (variant[2] > variant[i_split])? 2: i_split;
}


/* median of 5 coordinate components */
static int __3dtree_median5(__3dtree_Exemplar* a, int i_split)
{
    dt_real_type
        a0 = a[0].pt[i_split],
        a1 = a[1].pt[i_split],
        a2 = a[2].pt[i_split],
        a3 = a[3].pt[i_split],
        a4 = a[4].pt[i_split];

    if (a1 < a0) __3dtree_swap_real(&a0, &a1);
    if (a2 < a0) __3dtree_swap_real(&a0, &a2);
    if (a3 < a0) __3dtree_swap_real(&a0, &a3);
    if (a4 < a0) __3dtree_swap_real(&a0, &a4);
    if (a2 < a1) __3dtree_swap_real(&a1, &a2);
    if (a3 < a1) __3dtree_swap_real(&a1, &a3);
    if (a4 < a1) __3dtree_swap_real(&a1, &a4);
    if (a3 < a2) __3dtree_swap_real(&a2, &a3);
    if (a4 < a2) __3dtree_swap_real(&a2, &a4);

    if (a2 == a[0].pt[i_split])  return 0;
    if (a2 == a[1].pt[i_split])  return 1;
    if (a2 == a[2].pt[i_split])  return 2;
    if (a2 == a[3].pt[i_split])  return 3;
    /* else if (a2 == a[4]) */
    return 4;
}

/* one pass of quicksort partition */
static int __3dtree_partition(
    __3dtree_Exemplar *exset, int size, int pivot, int i_split)
{
    dt_real_type pivot_val = exset[pivot].pt[i_split];
    int i_load, i_store = 0;

    __3dtree_swap_exemplar(exset + pivot, exset + size-1);

    for(i_load = 0; i_load < size-1; i_load++)
    {
        if (exset[i_load].pt[i_split] < pivot_val) {
            __3dtree_swap_exemplar(exset + i_load, exset + i_store++);
        }
    }

    __3dtree_swap_exemplar(exset + i_store, exset + size-1);
    return i_store;
}

/* find k-th median in i_split-th dimension */
void __3dtree_kth_split(
    __3dtree_Exemplar *a, int size, int k, int i_split)
{
    __3dtree_Exemplar *group = a;
    int groupNum = 0;
    int numMedians = size/5;
    int MOMIdx = numMedians/2;  /* Index of median of medians */
    int newMOMIdx;

    /* less than 4 base sort */
    int i, j;    
    if (size < 5) {
        for (i = 0; i < size; i++)
            for (j = i + 1; j < size; j++)
                if (a[j].pt[i_split] < a[i].pt[i_split])
                    __3dtree_swap_exemplar(a + i, a + j);
        return;
    }

    /* median5 phase */
    for( ; groupNum*5 <= size-5; group += 5, groupNum++)
    {
        __3dtree_swap_exemplar(
            group + __3dtree_median5(group, i_split), 
            a + groupNum);
    }

    /* find median in remaining partition */
    __3dtree_kth_split(a, numMedians, MOMIdx, i_split);
    newMOMIdx = __3dtree_partition(a, size, MOMIdx, i_split);
    if (k != newMOMIdx)
    {
        if (k < newMOMIdx) {
            __3dtree_kth_split(a, newMOMIdx, k, i_split);
        }
        else
        {
            __3dtree_kth_split(
                a + newMOMIdx + 1, size - newMOMIdx - 1, 
                k - newMOMIdx - 1, i_split);
        }
    }
}


