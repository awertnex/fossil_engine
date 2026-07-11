/*!
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
 *  limitations under the License.
 */

/*!
 *  @file physics_types.h
 *
 *  @brief physics types.
 */

#ifndef FSL_PHYSICS_TYPES_H
#define FSL_PHYSICS_TYPES_H

#include "../math/vector.h"

typedef struct fsl_transform_v2f32
{
    v2f32 pos;
    v2f32 rot;
    v2f32 scale;

    v2f32 pos_delta;
    v2f32 rot_delta;
    v2f32 scale_delta;
} fsl_transform_v2f32;

typedef struct fsl_transform_v2f64
{
    v2f64 pos;
    v2f64 rot;
    v2f64 scale;

    v2f64 pos_delta;
    v2f64 rot_delta;
    v2f64 scale_delta;
} fsl_transform_v2f64;

typedef struct fsl_transform_v3f32
{
    v3f32 pos;
    v3f32 rot;
    v3f32 scale;

    v3f32 pos_delta;
    v3f32 rot_delta;
    v3f32 scale_delta;
} fsl_transform_v3f32;

typedef struct fsl_transform_v3f64
{
    v3f64 pos;
    v3f64 rot;
    v3f64 scale;

    v3f64 pos_delta;
    v3f64 rot_delta;
    v3f64 scale_delta;
} fsl_transform_v3f64;

typedef struct fsl_physics_material
{
    v3f64 friction;
    v3f64 drag;
    f64 bounciness;
} fsl_physics_material;

typedef v3f64 fsl_physics_force;

typedef struct fsl_kinematics
{
    v3f64 acceleration;
    v3f64 velocity;
    f64 speed;
    f64 mass;
    f64 mass_inv; /* inverse mass */
} fsl_kinematics;

#endif /* FSL_PHYSICS_TYPES_H */
