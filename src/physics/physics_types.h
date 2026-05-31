#ifndef FSL_PHYSICS_TYPES_H
#define FSL_PHYSICS_TYPES_H

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

#include "../math/vector.h"

typedef struct transform_v2f32
{
    v2f32 pos;
    v2f32 rot;
    v2f32 scale;

    v2f32 pos_delta;
    v2f32 rot_delta;
    v2f32 scale_delta;
} transform_v2f32;

typedef struct transform_v2f64
{
    v2f64 pos;
    v2f64 rot;
    v2f64 scale;

    v2f64 pos_delta;
    v2f64 rot_delta;
    v2f64 scale_delta;
} transform_v2f64;

typedef struct transform_v3f32
{
    v3f32 pos;
    v3f32 rot;
    v3f32 scale;

    v3f32 pos_delta;
    v3f32 rot_delta;
    v3f32 scale_delta;
} transform_v3f32;

typedef struct transform_v3f64
{
    v3f64 pos;
    v3f64 rot;
    v3f64 scale;

    v3f64 pos_delta;
    v3f64 rot_delta;
    v3f64 scale_delta;
} transform_v3f64;

#endif /* FSL_PHYSICS_TYPES_H */
