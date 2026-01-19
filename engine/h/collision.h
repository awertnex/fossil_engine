#ifndef ENGINE_COLLISION_H
#define ENGINE_COLLISION_H

#include "common.h"
#include "types.h"

#define COLLISION_EPSILON 1e-5

typedef struct BoundingBox
{
    v3f64 pos;
    v3f64 size;
} BoundingBox;

FSLAPI b8 is_intersect_aabb(BoundingBox a, BoundingBox b);

/*! @brief get collision status and stats between 'a' and 'b' using
 *  the 'Swept AABB' algorithm.
 *
 *  @param displacement = displacement of 'a', since this function assumes 'b' is static.
 *
 *  @return entry time.
 */
FSLAPI f32 get_swept_aabb(BoundingBox a, BoundingBox b, v3f32 displacement, v3f32 *normal);

#endif /* ENGINE_COLLISION_H */
