#ifndef __3DTREE_HEADER__
#define __3DTREE_HEADER__


#include "dt_type.h"


/* an exemplar is a point with an integer id in 3D space, this is the most
   basic element of a 3d tree */
typedef struct __3dtree_Exemplar_struct
{
    dt_real_type pt[3];  /* coordinate */
    int id;              /* ID */

} __3dtree_Exemplar;


/* 3d tree is a BST-like data structure for accelerating closest neighbour
   search. */
typedef struct __3dtree_Node_struct
{
    int i_split;        /* split dimension of this layer */

    /* exemplar */
    dt_real_type pt[3]; /* coordinate of this point */
    int id;

    /* child nodes */
    struct __3dtree_Node_struct *_left;
    struct __3dtree_Node_struct *_right;

} __3dtree_Node, *__3dTree;


/* Build a 3d tree using the exemplars in the specified array */
void __3dtree_Create3DTree(
    __3dtree_Exemplar *ex_begin, __3dtree_Exemplar *ex_end, __3dTree *tree);

/* Destroy a 3d tree */
void __3dtree_Destroy3DTree(__3dTree tree);


/* Find the nearest neighbour of x0 in the tree, the pointer to the nearest
   neighbour node and the squared distance is returned through the 2 last
   parameters */
void __3dtree_NearestPoint(
    __3dTree tree, const dt_real_type *x0, 
    __3dtree_Node **nearest_node, dt_real_type *nearest_dist_sq);

/* Find closest neighbour which satisfying specified condition */
void __3dtree_NearestPoint_Cond(
    __3dTree tree, const dt_real_type *x0, 
    __3dtree_Node **nearest_node, dt_real_type *nearest_dist_sq, 
    int(*cond)(__3dtree_Node *node));


/* Find all points lying on the disk centered at x0 with radius range, all 
   exemplars found was placed in custom buffer `res_node', squared distance 
   were placed in `res_dist_sq', the number of in bound exemplars was returned
   as function return value.
*/
int __3dtree_RangeSearch(
    __3dTree tree, const dt_real_type *x0, dt_real_type range,
    __3dtree_Node **res_node, dt_real_type *res_dist_sq);

/* The conditional version of __3dtree_RangeSearch */
int __3dtree_RangeSearch_Cond(
    __3dTree tree, const dt_real_type *x0, dt_real_type range,
    __3dtree_Node **res_node, dt_real_type *res_dist_sq,
    int(*cond)(__3dtree_Node *node));


/* routines not intended to be used */

__3dtree_Node* __3dtree_CreateNode(
    const dt_real_type *x0, int id, int i_split);

__3dtree_Node* __3dtree_FindParent(
    const dt_real_type *x0, __3dtree_Node *tree);

__3dtree_Node* __3dtree_Insert(
    const dt_real_type *x0, int id, 
    __3dtree_Node *tree);



/* private functions, do not touch. */

int __3dtree_select_split_dimension(
    __3dtree_Exemplar *ex_begin, __3dtree_Exemplar *ex_end);

void __3dtree_kth_split(
    __3dtree_Exemplar *a, int size, int k, int i_split);



#endif /* __3DTREE_HEADER__ */
