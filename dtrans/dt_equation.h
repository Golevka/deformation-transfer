#ifndef __DT_DEFORMATION_EQUATION_HEADER__
#define __DT_DEFORMATION_EQUATION_HEADER__


#include "triangle_corr_dict.h"
#include "mesh_model.h"
#include "surface_matrix.h"



/* Allocate for coefficient matrix and rhs vector with proper size */
void __dt_AllocDeformationEquation(
    const dtMeshModel *target_mesh, const __dt_TriangleCorrsDict *tcdict,
    __dt_SparseMatrix *A_tri, __dt_DenseVector *C);


/* Build the coefficient matrix of deformation equations from target reference 
   mesh, the coefficient matrix can be built only once to deform for a lot of 
   deformed source meshes. 
*/
void __dt_BuildCoefficientMatrix(
    const dtMeshModel *target_ref, const __dt_TriangleCorrsDict *tcdict,
    __dt_SparseMatrix A);


/* Build rhs vector of the deformation equation for source_ref=>source_deform.
   You just need to build rhs vector for each deformation while keeping the 
   coefficient matrix unchanged.
*/
void __dt_BuildRhsConstantVector(
    const dtMeshModel *source_deformed, const dtMeshModel *target_ref,
    const __dt_SurfaceInvVList *sinvlist_ref,
    const __dt_TriangleCorrsDict *tcdict,
    __dt_DenseVector C);



#endif /*__DT_DEFORMATION_EQUATION_HEADER__*/
