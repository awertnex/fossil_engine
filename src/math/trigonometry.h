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
 *  @file trigonometry.h
 *
 *  @brief trigonometry functions and types.
 */

#ifndef FSL_MATH_TRIGONOMETRY_H
#define FSL_MATH_TRIGONOMETRY_H

#include "../common/api.h"
#include "../common/types.h"

typedef struct angle_f32
{
    f32 angle;
    f32 sin;
    f32 cos;
    f32 tan;
} angle_f32;

typedef struct angle_f64
{
    f64 angle;
    f64 sin;
    f64 cos;
    f64 tan;
} angle_f64;

FSLAPI angle_f32 fsl_angle_f32(f32 n);
FSLAPI angle_f64 fsl_angle_f64(f64 n);

#endif /* FSL_MATH_TRIGONOMETRY_H */
