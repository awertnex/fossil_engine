/*  @file collision.h
 *
 *  @brief collision detection and data types.
 *
 *  Copyright 2026 Lily Awertnex
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.OFTWARE.
 */

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
