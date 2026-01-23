#ifndef FSL_COLLISION_H
#define FSL_COLLISION_H

#include "common.h"
#include "types.h"

#define FSL_COLLISION_EPSILON 1e-5

typedef struct fsl_bounding_box
{
    v3f64 pos;
    v3f64 size;
} fsl_bounding_box;

FSLAPI b8 fsl_is_intersect_aabb(fsl_bounding_box a, fsl_bounding_box b);

/*! @brief get collision status and stats between `a` and `b` using the 'Swept AABB' algorithm.
 *
 *  @param displacement displacement of `a`, since this function assumes `b` is static.
 *
 *  @return entry time.
 */
FSLAPI f32 fsl_get_swept_aabb(fsl_bounding_box a, fsl_bounding_box b, v3f32 displacement, v3f32 *normal);

#endif /* FSL_COLLISION_H */
