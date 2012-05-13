#include <stdlib.h>
#include "3dtree.h"


/* select split dimension and split the exemplar set into 2 partitions */
static void __3dtree_split(
    __3dtree_Exemplar *ex_begin, __3dtree_Exemplar *ex_end, 
    int *pi_split, __3dtree_Exemplar **ex_pivot)
{
    int size = (int)(ex_end - ex_begin);
    int k = size / 2;   /* find the median */

    *pi_split = __3dtree_select_split_dimension(ex_begin, ex_end);
    __3dtree_kth_split(ex_begin, size, k, *pi_split);
    *ex_pivot = ex_begin + k;
}

/* Build a 3d tree using the exemplars in the specified array */
void __3dtree_Create3DTree(
    __3dtree_Exemplar *ex_begin, __3dtree_Exemplar *ex_end, __3dTree *tree)
{
    int i_split;
    __3dtree_Exemplar 
        *ex_lbeg = ex_begin, *ex_lend, 
        *ex_rbeg, *ex_rend = ex_end, 
        *ex_pivot;

    __3dtree_Node *newnode;

    if (ex_begin != ex_end)  /* exemplar range is not empty */
    {
        __3dtree_split(ex_begin, ex_end, &i_split, &ex_pivot);
     
        newnode = __3dtree_CreateNode(ex_pivot->pt, ex_pivot->id, i_split);
        *tree = newnode;

        ex_lend = ex_pivot;
        __3dtree_Create3DTree(ex_lbeg, ex_lend, &(newnode->_left));

        ex_rbeg = ex_pivot + 1;
        __3dtree_Create3DTree(ex_rbeg, ex_rend, &(newnode->_right));
    }
}

/* Destroy a 3d tree */
void __3dtree_Destroy3DTree(__3dTree tree)
{
    if (tree->_left)  __3dtree_Destroy3DTree(tree->_left);
    if (tree->_right) __3dtree_Destroy3DTree(tree->_right);
    free(tree);
}


/* Allocate a 3d tree node and initialize with specified parameters,
   this newly created node has no children so _left and _right are
   initialized as NULL */
__3dtree_Node* __3dtree_CreateNode(
    const dt_real_type *x0, int id, int i_split)
{
    __3dtree_Node *node = (__3dtree_Node*)__dt_malloc(sizeof(__3dtree_Node));
    node->i_split = i_split;
    node->id = id;

    node->pt[0] = x0[0];
    node->pt[1] = x0[1];
    node->pt[2] = x0[2];

    node->_left   = NULL;
    node->_right  = NULL;

    return node;
}

__3dtree_Node* __3dtree_FindParent(
    const dt_real_type *x0, __3dtree_Node *tree)
{
    __3dtree_Node *parent;
    __3dtree_Node *next = tree;
    int i_split;

    while (next)
    {
        i_split = next->i_split;
        parent = next;

        if (x0[i_split] > next->pt[i_split])
            next = next->_right;
        else
            next = next->_left;
    }

    return parent;
}

__3dtree_Node* __3dtree_Insert(
    const dt_real_type *x0, int id, 
    __3dtree_Node *tree)
{
    __3dtree_Node *parent = __3dtree_FindParent(x0, tree);
    int i_split = parent->i_split;

    __3dtree_Node *newnode = 
        __3dtree_CreateNode(x0, id, (i_split + 1) % 3);

    if (x0[i_split] > parent->pt[i_split]) {
        parent->_right = newnode;
    }
    else {
        parent->_left = newnode;
    }

    return newnode;
}
