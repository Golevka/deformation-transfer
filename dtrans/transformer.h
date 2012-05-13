#ifndef __DT_TRANSFORMER_HEADER__  /* (not that Transformer) */
#define __DT_TRANSFORMER_HEADER__


#include "dt_equation.h"


typedef struct __dt_Transformer_struct
{
    dtMeshModel source_ref;   /* source reference model */
    dtMeshModel target;       /* target reference/deformed model. 
                                 It represents the reference model when
                                 initializing the transformer object, 
                                 and turns into deformed target model in
                                 deformation transfer solving phase. */

    __dt_TriangleCorrsDict tcdict;  /* triangle units correspondence */

    /* the deformation equation: AtA * x = c, where c = At * C */
    cholmod_sparse *At, *AtA;
    cholmod_dense  *C, *c, *x;

    void *numeric_obj;     /* umfpack factorization result */

    __dt_SurfaceInvVList sinvlist;   /* inverse surface matrix list for 
                                        source reference model */

} dtTransformer;


/* Create a deformation transfer object, once created, this object can help 
   deforming the target mesh like the source mesh deformation quicky and 
   faithfully. There's a lot of initialization process so this procedure might
   take significiant amount of time.
*/
void CreateDeformationTransformer(
    const char *source_ref_name, const char *target_ref_name,
    const char *tricorrs_name, dt_size_type n_maxcorrs,
    dtTransformer *trans);

/* Transform the target model like source_ref==>source_deformed, trans->target
   is modified to deformed model.  */
void Transform2TargetMeshModel(
    const dtMeshModel *source_deformed, dtTransformer *trans);

/* Release the memory allocated for the transformer object */
void DestroyDeformationTransformer(dtTransformer *trans);



#endif /*__DT_TRANSFORMER_HEADER__*/
