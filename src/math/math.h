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

FSLAPI u8 fsl_clamp_u8(u8 n, u8 min, u8 max);
FSLAPI i8 fsl_clamp_i8(i8 n, i8 min, i8 max);
FSLAPI u16 fsl_clamp_u16(u16 n, u16 min, u16 max);
FSLAPI i16 fsl_clamp_i16(i16 n, i16 min, i16 max);
FSLAPI u32 fsl_clamp_u32(u32 n, u32 min, u32 max);
FSLAPI i32 fsl_clamp_i32(i32 n, i32 min, i32 max);
FSLAPI f32 fsl_clamp_f32(f32 n, f32 min, f32 max);
FSLAPI u64 fsl_clamp_u64(u64 n, u64 min, u64 max);
FSLAPI i64 fsl_clamp_i64(i64 n, i64 min, i64 max);
FSLAPI f64 fsl_clamp_f64(f64 n, f64 min, f64 max);

FSLAPI u8 fsl_mod_u8(u8 n, u8 max);
FSLAPI i8 fsl_mod_i8(i8 n, i8 max);
FSLAPI u16 fsl_mod_u16(u16 n, u16 max);
FSLAPI i16 fsl_mod_i16(i16 n, i16 max);
FSLAPI u32 fsl_mod_u32(u32 n, u32 max);
FSLAPI i32 fsl_mod_i32(i32 n, i32 max);
FSLAPI u64 fsl_mod_u64(u64 n, u64 max);
FSLAPI i64 fsl_mod_i64(i64 n, i64 max);

FSLAPI u8 fsl_round_up_u8(u8 n, u8 max);
FSLAPI i8 fsl_round_up_i8(i8 n, i8 max);
FSLAPI u16 fsl_round_up_u16(u16 n, u16 max);
FSLAPI i16 fsl_round_up_i16(i16 n, i16 max);
FSLAPI u32 fsl_round_up_u32(u32 n, u32 max);
FSLAPI i32 fsl_round_up_i32(i32 n, i32 max);
FSLAPI u64 fsl_round_up_u64(u64 n, u64 max);
FSLAPI i64 fsl_round_up_i64(i64 n, i64 max);

FSLAPI u8 fsl_map_range_u8(u8 n, u8 n_min, u8 n_max, u8 r_min, u8 r_max);
FSLAPI i8 fsl_map_range_i8(i8 n, i8 n_min, i8 n_max, i8 r_min, i8 r_max);
FSLAPI u16 fsl_map_range_u16(u16 n, u16 n_min, u16 n_max, u16 r_min, u16 r_max);
FSLAPI i16 fsl_map_range_i16(i16 n, i16 n_min, i16 n_max, i16 r_min, i16 r_max);
FSLAPI u32 fsl_map_range_u32(u32 n, u32 n_min, u32 n_max, u32 r_min, u32 r_max);
FSLAPI i32 fsl_map_range_i32(i32 n, i32 n_min, i32 n_max, i32 r_min, i32 r_max);
FSLAPI f32 fsl_map_range_f32(f32 n, f32 n_min, f32 n_max, f32 r_min, f32 r_max);
FSLAPI u64 fsl_map_range_u64(u64 n, u64 n_min, u64 n_max, u64 r_min, u64 r_max);
FSLAPI i64 fsl_map_range_i64(i64 n, i64 n_min, i64 n_max, i64 r_min, i64 r_max);
FSLAPI f64 fsl_map_range_f64(f64 n, f64 n_min, f64 n_max, f64 r_min, f64 r_max);

FSLAPI b8 fsl_is_in_bounds_u8(u8 n, u8 min, u8 max);
FSLAPI b8 fsl_is_in_bounds_i8(i8 n, i8 min, i8 max);
FSLAPI b8 fsl_is_in_bounds_u16(u16 n, u16 min, u16 max);
FSLAPI b8 fsl_is_in_bounds_i16(i16 n, i16 min, i16 max);
FSLAPI b8 fsl_is_in_bounds_u32(u32 n, u32 min, u32 max);
FSLAPI b8 fsl_is_in_bounds_i32(i32 n, i32 min, i32 max);
FSLAPI b8 fsl_is_in_bounds_f32(f32 n, f32 min, f32 max);
FSLAPI b8 fsl_is_in_bounds_u64(u64 n, u64 min, u64 max);
FSLAPI b8 fsl_is_in_bounds_i64(i64 n, i64 min, i64 max);
FSLAPI b8 fsl_is_in_bounds_f64(f64 n, f64 min, f64 max);

FSLAPI f32 fsl_lerp_f32(f32 a, f32 b, f32 t);
FSLAPI f64 fsl_lerp_f64(f64 a, f64 b, f64 t);
FSLAPI f32 fsl_bilerp_f32(f32 a, f32 b, f32 c, f32 d, f32 tx, f32 ty);
FSLAPI f64 fsl_bilerp_f64(f64 a, f64 b, f64 c, f64 d, f64 tx, f64 ty);
FSLAPI f32 fsl_trilerp_f32(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h, f32 tx, f32 ty, f32 tz);
FSLAPI f64 fsl_trilerp_f64(f64 a, f64 b, f64 c, f64 d, f64 e, f64 f, f64 g, f64 h, f64 tx, f64 ty, f64 tz);
FSLAPI f32 fsl_lerp_exp_f32(f32 a, f32 b, f32 k, f32 t);
FSLAPI f64 fsl_lerp_exp_f64(f64 a, f64 b, f64 k, f64 t);
FSLAPI f32 fsl_fade_ease_in_f32(f32 t);
FSLAPI f64 fsl_fade_ease_in_f64(f64 t);
FSLAPI f32 fsl_fade_smoothstep_f32(f32 t);
FSLAPI f64 fsl_fade_smoothstep_f64(f64 t);
FSLAPI f32 fsl_fade_quintic_f32(f32 t);
FSLAPI f64 fsl_fade_quintic_f64(f64 t);
FSLAPI u32 fsl_rand_u32(u32 n);
FSLAPI u64 fsl_rand_u64(u64 n);

#endif /* FSL_MATH_H */
