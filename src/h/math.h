/*  Copyright 2026 Lily Awertnex
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

/*  math.h - math stuff
 */

#ifndef FSL_MATH_H
#define FSL_MATH_H

#include <math.h>

#include "common.h"
#include "types.h"

#define FSL_PI          3.14159265358979323846f
#define FSL_DEG2RAD     (FSL_PI / 180.0f)
#define FSL_RAD2DEG     (180.0f / FSL_PI)
#define FSL_GRAVITY     9.7803267715f
#define FSL_EPSILON     1e-5f
#define FSL_RAND_SCALE  (FSL_PI / ~(~0u >> 1))

FSLAPI v3f32 fsl_add_v3f32(v3f32 a, v3f32 b);
FSLAPI v3f32 fsl_sub_v3f32(v3f32 a, v3f32 b);
FSLAPI i32 fsl_clamp_i32(i32 n, i32 min, i32 max);
FSLAPI u32 fsl_clamp_u32(u32 n, u32 min, u32 max);
FSLAPI f32 fsl_clamp_f32(f32 n, f32 min, f32 max);
FSLAPI i64 fsl_clamp_i64(i64 n, i64 min, i64 max);
FSLAPI u64 fsl_clamp_u64(u64 n, u64 min, u64 max);
FSLAPI f64 fsl_clamp_f64(f64 n, f64 min, f64 max);
FSLAPI u64 fsl_round_up_u64(u64 x, u64 n);
FSLAPI i32 fsl_mod_i32(i32 n, i32 max);
FSLAPI i64 fsl_mod_i64(i64 n, i64 max);
FSLAPI f32 fsl_min_v3f32(v3f32 v);
FSLAPI f32 fsl_max_v3f32(v3f32 v);
FSLAPI f32 fsl_map_range_f32(f32 n, f32 n_min, f32 n_max, f32 r_min, f32 r_max);
FSLAPI f64 fsl_map_range_f64(f64 n, f64 n_min, f64 n_max, f64 r_min, f64 r_max);
FSLAPI u32 fsl_min_axis_v3f32(v3f32 v);
FSLAPI u32 fsl_max_axis_v3f32(v3f32 v);
FSLAPI f32 fsl_len_v3f32(v3f32 v);
FSLAPI f64 fsl_len_v3f64(v3f64 v);
FSLAPI v3f32 fsl_normalize_v3f32(v3f32 v);
FSLAPI v3f64 fsl_normalize_v3f64(v3f64 v);
FSLAPI f32 fsl_dot_v3f32(v3f32 a, v3f32 b);
FSLAPI f64 fsl_dot_v3f64(v3f64 a, v3f64 b);
FSLAPI f32 fsl_q_rsqrt(f32 n);
FSLAPI u32 fsl_distance_v3i32(v3i32 a, v3i32 b);
FSLAPI f32 fsl_distance_v3f32(v3f32 a, v3f32 b);
FSLAPI f64 fsl_distance_v3f64(v3f64 a, v3f64 b);
FSLAPI b8 fsl_is_in_range_i32(i32 n, i32 min, i32 max);
FSLAPI b8 fsl_is_in_range_f32(f32 n, f32 min, f32 max);
FSLAPI b8 fsl_is_in_range_i64(i64 n, i64 min, i64 max);
FSLAPI b8 fsl_is_in_range_f64(f64 n, f64 min, f64 max);
FSLAPI b8 fsl_is_in_area_i32(v2i32 v, v2i32 min, v2i32 max);
FSLAPI b8 fsl_is_in_area_f32(v2f32 v, v2f32 min, v2f32 max);
FSLAPI b8 fsl_is_in_volume_i32(v3i32 v, v3i32 min, v3i32 max);
FSLAPI b8 fsl_is_in_volume_f32(v3f32 v, v3f32 min, v3f32 max);
FSLAPI b8 fsl_is_in_volume_i64(v3i64 v, v3i64 min, v3i64 max);
FSLAPI b8 fsl_is_in_volume_f64(v3f64 v, v3f64 min, v3f64 max);
FSLAPI m4f32 fsl_matrix_add(m4f32 a, m4f32 b);
FSLAPI m4f32 fsl_matrix_subtract(m4f32 a, m4f32 b);
FSLAPI m4f32 fsl_matrix_multiply(m4f32 a, m4f32 b);
FSLAPI v4f32 fsl_matrix_multiply_vector(m4f32 a, v4f32 b);
FSLAPI f32 fsl_lerp_f32(f32 a, f32 b, f64 t);
FSLAPI f32 fsl_lerp_exp_f32(f32 a, f32 b, f64 k, f32 t);
FSLAPI f32 fsl_lerp_cubic_f32(f32 a, f32 b, f32 t);
FSLAPI v3f64 fsl_lerp_v3f64(v3f64 a, v3f64 b, f32 t);
FSLAPI f32 fsl_easein_f32(f32 a, f32 b, f32 t);
FSLAPI f32 fsl_easeout_f32(f32 a, f32 b, f32 t);
FSLAPI f32 fsl_smoothstep_f32(f32 a, f32 b, f32 t);
FSLAPI f32 fsl_rand_f32(i32 n);
FSLAPI u64 fsl_rand_u64(u64 n);

#endif /* FSL_MATH_H */
