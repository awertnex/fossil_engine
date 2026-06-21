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
 *  @file math.h
 *
 *  @brief math stuff.
 */

#ifndef FSL_MATH_H
#define FSL_MATH_H

#include "../common/api.h"
#include "../common/types.h"

#define FSL_PI          3.14159265358979323846
#define FSL_HALF_PI     1.57079632679489661923
#define FSL_DEG2RAD     (FSL_PI / 180.0)
#define FSL_RAD2DEG     (180.0 / FSL_PI)
#define FSL_GRAVITY     9.7803267715
#define FSL_EPSILON     1e-5

FSLAPI i32 fsl_clamp_i32(i32 n, i32 min, i32 max);
FSLAPI u32 fsl_clamp_u32(u32 n, u32 min, u32 max);
FSLAPI f32 fsl_clamp_f32(f32 n, f32 min, f32 max);
FSLAPI i64 fsl_clamp_i64(i64 n, i64 min, i64 max);
FSLAPI u64 fsl_clamp_u64(u64 n, u64 min, u64 max);
FSLAPI f64 fsl_clamp_f64(f64 n, f64 min, f64 max);
FSLAPI u64 fsl_round_up_u64(u64 n, u64 max);
FSLAPI i32 fsl_mod_i32(i32 n, i32 max);
FSLAPI i64 fsl_mod_i64(i64 n, i64 max);
FSLAPI f32 fsl_map_range_f32(f32 n, f32 n_min, f32 n_max, f32 r_min, f32 r_max);
FSLAPI f64 fsl_map_range_f64(f64 n, f64 n_min, f64 n_max, f64 r_min, f64 r_max);
FSLAPI b8 fsl_is_in_range_i32(i32 n, i32 min, i32 max);
FSLAPI b8 fsl_is_in_range_f32(f32 n, f32 min, f32 max);
FSLAPI b8 fsl_is_in_range_i64(i64 n, i64 min, i64 max);
FSLAPI b8 fsl_is_in_range_f64(f64 n, f64 min, f64 max);
FSLAPI f32 fsl_lerp_f32(f32 a, f32 b, f64 t);
FSLAPI f32 fsl_lerp_exp_f32(f32 a, f32 b, f64 k, f32 t);
FSLAPI f32 fsl_easein_f32(f32 a, f32 b, f32 t);
FSLAPI f32 fsl_easeout_f32(f32 a, f32 b, f32 t);
FSLAPI f32 fsl_smoothstep_f32(f32 a, f32 b, f32 t);
FSLAPI u32 fsl_rand_u32(u32 n);
FSLAPI u64 fsl_rand_u64(u64 n);

#endif /* FSL_MATH_H */
