#include <stdio.h>
#include "matrix3x3.h"



/* Calculate the inverse matrix of a given 3x3 matrix, the calculated inverse
   matrix would replace the original matrix */
void __dt_InverseMatrix3x3(dtMatrix3x3 mat)
{
    dt_real_type
        m00 = mat[0][0], m01 = mat[0][1], m02 = mat[0][2], 
        m10 = mat[1][0], m11 = mat[1][1], m12 = mat[1][2],
        m20 = mat[2][0], m21 = mat[2][1], m22 = mat[2][2];

    dt_real_type factor = 1.0 / ( 
        /* determinant value as denominator */
        m00*m11*m22 - m00*m12*m21 - m01*m10*m22 + 
        m01*m12*m20 + m02*m10*m21 - m02*m11*m20);

    /* calculate each element of inverse matrix */
    mat[0][0] = factor * (m11*m22-m12*m21);
    mat[0][1] = factor * (m02*m21-m01*m22);
    mat[0][2] = factor * (m01*m12-m02*m11);
    
    mat[1][0] = factor * (m12*m20-m10*m22);
    mat[1][1] = factor * (m00*m22-m02*m20);
    mat[1][2] = factor * (m02*m10-m00*m12);
    
    mat[2][0] = factor * (m10*m21-m11*m20);
    mat[2][1] = factor * (m01*m20-m00*m21);
    mat[2][2] = factor * (m00*m11-m01*m10);
}


/* Calculate the product of A of B and restore the result in C, aka. C = A*B */
void __dt_Matrix3x3_Product(
    dtMatrix3x3 A, dtMatrix3x3 B, dtMatrix3x3 C)
{
    /* <unrolled 3x3x3 loop code> */
    C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0] + A[0][2]*B[2][0];
    C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1] + A[0][2]*B[2][1];
    C[0][2] = A[0][0]*B[0][2] + A[0][1]*B[1][2] + A[0][2]*B[2][2];

    C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0] + A[1][2]*B[2][0];
    C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1] + A[1][2]*B[2][1];
    C[1][2] = A[1][0]*B[0][2] + A[1][1]*B[1][2] + A[1][2]*B[2][2];

    C[2][0] = A[2][0]*B[0][0] + A[2][1]*B[1][0] + A[2][2]*B[2][0];
    C[2][1] = A[2][0]*B[0][1] + A[2][1]*B[1][1] + A[2][2]*B[2][1];
    C[2][2] = A[2][0]*B[0][2] + A[2][1]*B[1][2] + A[2][2]*B[2][2];
}
