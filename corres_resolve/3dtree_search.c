#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "3dtree.h"


/* calculate squared distance between v1 and v2 */
static dt_real_type __squared_distance(
    const dt_real_type *v1, const dt_real_type *v2)
{
    dt_real_type  dx = (v1[0] - v2[0]);
    dt_real_type  dy = (v1[1] - v2[1]);
    dt_real_type  dz = (v1[2] - v2[2]);

    return dx*dx + dy*dy + dz*dz;
}


/* This structure is to represent the bounds of hyperspace dominated by the 
   nodes in 3d tree */
typedef struct __3dtree_Hyperrect_struct
{
    dt_real_type min[3];   /* the lower left corner  */
    dt_real_type max[3];   /* the upper right corner */

} __3dtree_Hyperrect;

/* calculate squared distance from x0 to the hyperrect */
static dt_real_type __squared_hyperrect_distance(
    const __3dtree_Hyperrect *rect, const dt_real_type *x0)
{
    int i_dim;
    dt_real_type dist_sq = 0;

    for (i_dim = 0; i_dim < 3; i_dim++)
    {
        if (x0[i_dim] < rect->min[i_dim]) {
            dist_sq += 
                (rect->min[i_dim] - x0[i_dim]) *
                (rect->min[i_dim] - x0[i_dim]);
        }
        else if (x0[i_dim] > rect->max[i_dim]) {
            dist_sq +=
                (rect->max[i_dim] - x0[i_dim]) *
                (rect->max[i_dim] - x0[i_dim]);
        }
    }

    return dist_sq;
}


static void __3dtree_nearest(
    __3dtree_Node *node, const dt_real_type *x0, 
    __3dtree_Node **nearest_node, dt_real_type *nearest_dist_sq, 
    __3dtree_Hyperrect *rect, int(*cond)(__3dtree_Node *node))
{
    int i_dim = node->i_split;

    dt_real_type tmp, dist_sq;
    __3dtree_Node *nearer_part, *further_part;
    dt_real_type *nearer_hr_coord, *further_hr_coord;

    /* determine whether to go left or right in the tree */
    if (x0[i_dim] <= node->pt[i_dim])
    {
        nearer_part  = node->_left;
        further_part = node->_right;
        nearer_hr_coord  = &(rect->max[i_dim]);
        further_hr_coord = &(rect->min[i_dim]);
    }
    else
    {
        nearer_part  = node->_right;
        further_part = node->_left;
        nearer_hr_coord  = &(rect->min[i_dim]);
        further_hr_coord = &(rect->max[i_dim]);
    }

    if (nearer_part)
    {
        /* slice the hyperrect to get the hyperrect of the nearer subtree */
        tmp = *nearer_hr_coord;  /* save current hyperrect coordinate */
        *nearer_hr_coord = node->pt[i_dim];

        /* recurse down into nearer subtree */
        __3dtree_nearest(nearer_part, x0, 
            nearest_node, nearest_dist_sq, rect, cond);

        /* restore the hyperrect */
        *nearer_hr_coord = tmp;
    }


    /* check if custom condition is satisfied */
    if (cond == NULL || cond(node))
    {
        /* check the distance of the point at the current node, compare it with
           our best so far */
        dist_sq = __squared_distance(node->pt, x0);
        if (dist_sq < *nearest_dist_sq) {
            *nearest_node = node;
            *nearest_dist_sq = dist_sq;
        }
    }


    if (further_part)
    {
        /* make a slice on the hyperrect */
        tmp = *further_hr_coord;
        *further_hr_coord = node->pt[i_dim];

        /* Check if we have to recurse down by calculating the closest point of
           the hyperrect and see if it's closer than our minimum distance in 
           nearest_dist_sq. */
        if (__squared_hyperrect_distance(rect, x0) < *nearest_dist_sq)
        {
            /* recurse down into further subtree */
            __3dtree_nearest(further_part, x0, 
                nearest_node, nearest_dist_sq, rect, cond);
        }

        /* undo the slice on the hyperrect */
        *further_hr_coord = tmp;
    }
}


/* Find the nearest neighbour of x0 in the tree, the pointer to the nearest
   neighbour node and the squared distance is returned through the 2 last
   parameters */
void __3dtree_NearestPoint(
    __3dTree tree, const dt_real_type *x0, 
    __3dtree_Node **nearest_node, dt_real_type *nearest_dist_sq)
{
    __3dtree_NearestPoint_Cond(tree, x0, nearest_node, nearest_dist_sq, NULL);
}

/* Find closest neighbour which satisfying specified condition */
void __3dtree_NearestPoint_Cond(
    __3dTree tree, const dt_real_type *x0, 
    __3dtree_Node **nearest_node, dt_real_type *nearest_dist_sq, 
    int(*cond)(__3dtree_Node *node))
{
    __3dtree_Hyperrect rect = {
        {-DBL_MAX, -DBL_MAX, -DBL_MAX},
        { DBL_MAX,  DBL_MAX,  DBL_MAX}
    };
    *nearest_dist_sq = DBL_MAX;
    __3dtree_nearest(tree, x0, nearest_node, nearest_dist_sq, &rect, cond);
}


static int __3dtree_range(
    __3dtree_Node *node, const dt_real_type *x0, dt_real_type range,
    __3dtree_Node **res_node, dt_real_type *res_dist_sq,
    int *i_res,
    int(*cond)(__3dtree_Node *node))
{
    __3dtree_Node *nearer, *further;
    dt_real_type dist_sq, dx;
    int ret, in_bound = 0;

    if (!node) return 0;
    dist_sq = __squared_distance(node->pt, x0);

    if (dist_sq < range*range)
    {
        in_bound = 1;
        
        /* append this exemplar to result list if cond is satisfied */
        if (cond == NULL || cond(node)) {
            res_node[*i_res] = node; res_dist_sq[*i_res] = dist_sq;
            (*i_res) += 1;
        }
    }

    /* determine which partition is the nearer subspace. */
    if ((dx = x0[node->i_split] - node->pt[node->i_split]) <= 0) {
        nearer = node->_left; further = node->_right;
    }
    else {
        nearer = node->_right; further = node->_left;
    }
    
    /* storm down to the nearer bound */
    in_bound += (ret = 
        __3dtree_range(
            nearer, x0, range, res_node, res_dist_sq, i_res, cond));
    
    /* if the nearer down level is in bound, then the further one might be
       in bounds too. */
    if (ret >= 0 && fabs(dx) < range)
    {
        in_bound += 
            __3dtree_range(further, x0, range, 
                res_node, res_dist_sq, i_res, cond);
    }

    return in_bound;
}


/* Find all points lying on the disk centered at x0 with radius range, all 
   exemplars found was placed in custom buffer `res_node', squared distance 
   were placed in `res_dist_sq', the number of in bound exemplars was returned
   as function return value.
*/
int __3dtree_RangeSearch(
    __3dTree tree, const dt_real_type *x0, dt_real_type range,
    __3dtree_Node **res_node, dt_real_type *res_dist_sq)
{
    return 
        __3dtree_RangeSearch_Cond(
            tree, x0, range, res_node, res_dist_sq, NULL);
}

/* The conditional version of __3dtree_RangeSearch */
int __3dtree_RangeSearch_Cond(
    __3dTree tree, const dt_real_type *x0, dt_real_type range,
    __3dtree_Node **res_node, dt_real_type *res_dist_sq,
    int(*cond)(__3dtree_Node *node))
{
    int n_res = 0;
    __3dtree_range(tree, x0, range, res_node, res_dist_sq, &n_res, cond);

    return n_res;
}
