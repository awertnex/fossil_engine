#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H

#include <math.h>

#include "common.h"
#include "types.h"

#define EPSILON     1e-5f
#define PI          3.14159265358979323846f
#define DEG2RAD     (PI / 180.0f)
#define RAD2DEG     (180.0f / PI)
#define GRAVITY     9.7803267715f
#define RAND_SCALE  (PI / ~(~0u >> 1))

#define mod(n, max) (((n) % (max) + (max)) % (max))

FSLAPI v3f32 add_v3f32(v3f32 a, v3f32 b);
FSLAPI v3f32 sub_v3f32(v3f32 a, v3f32 b);
FSLAPI i32 clamp_i32(i32 n, i32 min, i32 max);
FSLAPI u32 clamp_u32(u32 n, u32 min, u32 max);
FSLAPI f32 clamp_f32(f32 n, f32 min, f32 max);
FSLAPI i64 clamp_i64(i64 n, i64 min, i64 max);
FSLAPI u64 clamp_u64(u64 n, u64 min, u64 max);
FSLAPI f64 clamp_f64(f64 n, f64 min, f64 max);
FSLAPI u64 round_up_u64(u64 x, u64 n);
FSLAPI f32 min_v3f32(v3f32 v);
FSLAPI f32 max_v3f32(v3f32 v);
FSLAPI f32 map_range_f32(f32 n, f32 n_min, f32 n_max, f32 r_min, f32 r_max);
FSLAPI f64 map_range_f64(f64 n, f64 n_min, f64 n_max, f64 r_min, f64 r_max);
FSLAPI u32 min_axis_v3f32(v3f32 v);
FSLAPI u32 max_axis_v3f32(v3f32 v);
FSLAPI f32 len_v3f32(v3f32 v);
FSLAPI f64 len_v3f64(v3f64 v);
FSLAPI v3f32 normalize_v3f32(v3f32 v);
FSLAPI v3f64 normalize_v3f64(v3f64 v);
FSLAPI f32 dot_v3f32(v3f32 a, v3f32 b);
FSLAPI f64 dot_v3f64(v3f64 a, v3f64 b);
FSLAPI f32 q_rsqrt(f32 n);
FSLAPI u32 distance_v3i32(v3i32 a, v3i32 b);
FSLAPI f32 distance_v3f32(v3f32 a, v3f32 b);
FSLAPI f64 distance_v3f64(v3f64 a, v3f64 b);
FSLAPI b8 is_in_range_i32(i32 n, i32 min, i32 max);
FSLAPI b8 is_in_range_f32(f32 n, f32 min, f32 max);
FSLAPI b8 is_in_range_i64(i64 n, i64 min, i64 max);
FSLAPI b8 is_in_range_f64(f64 n, f64 min, f64 max);
FSLAPI b8 is_in_area_i32(v2i32 v, v2i32 min, v2i32 max);
FSLAPI b8 is_in_area_f32(v2f32 v, v2f32 min, v2f32 max);
FSLAPI b8 is_in_volume_i32(v3i32 v, v3i32 min, v3i32 max);
FSLAPI b8 is_in_volume_f32(v3f32 v, v3f32 min, v3f32 max);
FSLAPI b8 is_in_volume_i64(v3i64 v, v3i64 min, v3i64 max);
FSLAPI b8 is_in_volume_f64(v3f64 v, v3f64 min, v3f64 max);
FSLAPI m4f32 matrix_add(m4f32 a, m4f32 b);
FSLAPI m4f32 matrix_subtract(m4f32 a, m4f32 b);
FSLAPI m4f32 matrix_multiply(m4f32 a, m4f32 b);
FSLAPI v4f32 matrix_multiply_vector(m4f32 a, v4f32 b);
FSLAPI f32 lerp_f32(f32 a, f32 b, f64 t);
FSLAPI f32 lerp_exp_f32(f32 a, f32 b, f64 k, f32 t);
FSLAPI f32 lerp_cubic_f32(f32 a, f32 b, f32 t);
FSLAPI v3f64 lerp_v3f64(v3f64 a, v3f64 b, f32 t);
FSLAPI f32 easein_f32(f32 a, f32 b, f32 t);
FSLAPI f32 easeout_f32(f32 a, f32 b, f32 t);
FSLAPI f32 smoothstep_f32(f32 a, f32 b, f32 t);
FSLAPI f32 rand_f32(i32 n);
FSLAPI u64 rand_u64(u64 n);

#endif /* ENGINE_MATH_H */
