#ifndef __DT_MATRIX3X3_HEADER__
#define __DT_MATRIX3X3_HEADER__



#include "dt_type.h"


/* 3x3 matrix to represent deformation matrix (non-translational portion of 
   triangle unit deformation) or surface matrix of a particular triangle unit.
*/
typedef dt_real_type dtMatrix3x3[3][3];


/* Calculate the inverse matrix of a given 3x3 matrix, the calculated inverse
   matrix would replace the original matrix */
void __dt_InverseMatrix3x3(dtMatrix3x3 mat);

/* Calculate the product of A of B and restore the result in C, aka. C = A*B */
void __dt_Matrix3x3_Product(
    dtMatrix3x3 A, dtMatrix3x3 B, dtMatrix3x3 C);



#endif /* __DT_MATRIX3X3_HEADER__ */
