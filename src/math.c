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

/*  math.c - math stuff
 */

#include "h/math.h"

#include <math.h>

v3f32 fsl_add_v3f32(v3f32 a, v3f32 b)
{
    return (v3f32){a.x + b.x, a.y + b.y, a.z + b.z};
}

v3f32 fsl_sub_v3f32(v3f32 a, v3f32 b)
{
    return (v3f32){a.x - b.x, a.y - b.y, a.z - b.z};
}

i32 fsl_clamp_i32(i32 n, i32 min, i32 max)
{
    return n < min ? min : n > max ? max : n;
}

u32 fsl_clamp_u32(u32 n, u32 min, u32 max)
{
    return n < min ? min : n > max ? max : n;
}

f32 fsl_clamp_f32(f32 n, f32 min, f32 max)
{
    return n < min ? min : n > max ? max : n;
}

i64 fsl_clamp_i64(i64 n, i64 min, i64 max)
{
    return n < min ? min : n > max ? max : n;
}

u64 fsl_clamp_u64(u64 n, u64 min, u64 max)
{
    return n < min ? min : n > max ? max : n;
}

f64 fsl_clamp_f64(f64 n, f64 min, f64 max)
{
    return n < min ? min : n > max ? max : n;
}

u64 fsl_round_up_u64(u64 n, u64 val)
{
    return n + (val - (n % val)) % val;
}

i32 fsl_mod_i32(i32 n, i32 max)
{
    return (n % max + max) % max;
}

i64 fsl_mod_i64(i64 n, i64 max)
{
    return (n % max + max) % max;
}

f32 fsl_min_v3f32(v3f32 v)
{
    return v.x < v.y ? v.x < v.z ? v.x : v.z : v.y < v.z ? v.y : v.z;
}

f32 fsl_max_v3f32(v3f32 v)
{
    return v.x > v.y ? v.x > v.z ? v.x : v.z : v.y > v.z ? v.y : v.z;
}

u32 fsl_min_axis_v3f32(v3f32 v)
{
    return v.x < v.y ? v.x < v.z ? 1 : 3 : v.y < v.z ? 2 : 3;
}

u32 fsl_max_axis_v3f32(v3f32 v)
{
    return v.x > v.y ? v.x > v.z ? 1 : 3 : v.y > v.z ? 2 : 3;
}

f32 fsl_map_range_f32(f32 n, f32 n_min, f32 n_max, f32 r_min, f32 r_max)
{
    return r_min + ((n - n_min) * (r_max - r_min)) / (n_max - n_min);
}

f64 fsl_map_range_f64(f64 n, f64 n_min, f64 n_max, f64 r_min, f64 r_max)
{
    return r_min + ((n - n_min) * (r_max - r_min)) / (n_max - n_min);
}

f32 fsl_len_v3f32(v3f32 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

f64 fsl_len_v3f64(v3f64 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

v3f32 fsl_normalize_v3f32(v3f32 v)
{
    f32 len = sqrtf(fsl_len_v3f32(v));
    if (len < FSL_EPSILON)
        return (v3f32){0};

    return (v3f32){v.x / len, v.y / len, v.z / len};
}

v3f64 fsl_normalize_v3f64(v3f64 v)
{
    f64 len = sqrt(fsl_len_v3f64(v));
    if (len < FSL_EPSILON)
        return (v3f64){0};

    return (v3f64){v.x / len, v.y / len, v.z / len};
}

f32 fsl_dot_v3f32(v3f32 a, v3f32 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

f64 fsl_dot_v3f64(v3f64 a, v3f64 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#pragma GCC diagnostic ignored "-Warray-bounds"

f32 q_rsqrt(f32 n)
{
    i64 i;
    f32 x2, y;
    const f32 threehalfs = 1.5f;

    x2 = n * 0.5f;
    y = n;
    i = *(i64*)&y;                          /* evil floating point bit hack */
    i = 0x5f3759df - (i >> 1);              /* what the fuck? */
    y = *(f32*)&i;
    y = y * (threehalfs - (x2 * y * y));    /* 1st iteration */
    y = y * (threehalfs - (x2 * y * y));    /* 2nd iteration, can be removed */

    return y;
}

#pragma GCC diagnostic pop

u32 fsl_distance_v3i32(v3i32 a, v3i32 b)
{
    return
        (a.x - b.x) * (a.x - b.x) +
        (a.y - b.y) * (a.y - b.y) +
        (a.z - b.z) * (a.z - b.z);
}

f32 fsl_distance_v3f32(v3f32 a, v3f32 b)
{
    return
        (a.x - b.x) * (a.x - b.x) +
        (a.y - b.y) * (a.y - b.y) +
        (a.z - b.z) * (a.z - b.z);
}

f64 fsl_distance_v3f64(v3f64 a, v3f64 b)
{
    return
        (a.x - b.x) * (a.x - b.x) +
        (a.y - b.y) * (a.y - b.y) +
        (a.z - b.z) * (a.z - b.z);
}

b8 fsl_is_in_range_i32(i32 n, i32 min, i32 max)
{
    return (n - min >= 0) & (max - n >= 0);
}

b8 fsl_is_in_range_f32(f32 n, f32 min, f32 max)
{
    return (n - min >= 0.0f) & (max - n >= 0.0f);
}

b8 fsl_is_in_range_i64(i64 n, i64 min, i64 max)
{
    return (n - min >= 0) & (max - n >= 0);
}

b8 fsl_is_in_range_f64(f64 n, f64 min, f64 max)
{
    return (n - min >= 0.0f) & (max - n >= 0.0f);
}

b8 fsl_is_in_area_i32(v2i32 v, v2i32 min, v2i32 max)
{
    return
        (v.x - min.x >= 0) & (max.x - v.x >= 0) &
        (v.y - min.y >= 0) & (max.y - v.y >= 0);
}

b8 fsl_is_in_area_f32(v2f32 v, v2f32 min, v2f32 max)
{
    return
        (v.x - min.x >= 0.0f) & (max.x - v.x >= 0.0f) &
        (v.y - min.y >= 0.0f) & (max.y - v.y >= 0.0f);
}

b8 fsl_is_in_volume_i32(v3i32 v, v3i32 min, v3i32 max)
{
    return
        (v.x - min.x >= 0) & (max.x - v.x >= 0) &
        (v.y - min.y >= 0) & (max.y - v.y >= 0) &
        (v.z - min.z >= 0) & (max.z - v.z >= 0);
}

b8 fsl_is_in_volume_f32(v3f32 v, v3f32 min, v3f32 max)
{
    return
        (v.x - min.x >= 0.0f) & (max.x - v.x >= 0.0f) &
        (v.y - min.y >= 0.0f) & (max.y - v.y >= 0.0f) &
        (v.z - min.z >= 0.0f) & (max.z - v.z >= 0.0f);
}

b8 fsl_is_in_volume_i64(v3i64 v, v3i64 min, v3i64 max)
{
    return
        (v.x - min.x >= 0) & (max.x - v.x >= 0) &
        (v.y - min.y >= 0) & (max.y - v.y >= 0) &
        (v.z - min.z >= 0) & (max.z - v.z >= 0);
}

b8 fsl_is_in_volume_f64(v3f64 v, v3f64 min, v3f64 max)
{
    return
        (v.x - min.x >= 0.0f) & (max.x - v.x >= 0.0f) &
        (v.y - min.y >= 0.0f) & (max.y - v.y >= 0.0f) &
        (v.z - min.z >= 0.0f) & (max.z - v.z >= 0.0f);
}

m4f32 fsl_matrix_add(m4f32 a, m4f32 b)
{
    return (m4f32){
        a.a11 + b.a11, a.a12 + b.a12, a.a13 + b.a13, a.a14 + b.a14,
        a.a21 + b.a21, a.a22 + b.a22, a.a23 + b.a23, a.a24 + b.a24,
        a.a31 + b.a31, a.a32 + b.a32, a.a33 + b.a33, a.a34 + b.a34,
        a.a41 + b.a41, a.a42 + b.a42, a.a43 + b.a43, a.a44 + b.a44,
    };
}

m4f32 fsl_matrix_subtract(m4f32 a, m4f32 b)
{
    return (m4f32){
        a.a11 - b.a11, a.a12 - b.a12, a.a13 - b.a13, a.a14 - b.a14,
        a.a21 - b.a21, a.a22 - b.a22, a.a23 - b.a23, a.a24 - b.a24,
        a.a31 - b.a31, a.a32 - b.a32, a.a33 - b.a33, a.a34 - b.a34,
        a.a41 - b.a41, a.a42 - b.a42, a.a43 - b.a43, a.a44 - b.a44,
    };
}

m4f32 fsl_matrix_multiply(m4f32 a, m4f32 b)
{
    return (m4f32){
        a.a11 * b.a11 + a.a12 * b.a21 + a.a13 * b.a31 + a.a14 * b.a41,
        a.a11 * b.a12 + a.a12 * b.a22 + a.a13 * b.a32 + a.a14 * b.a42,
        a.a11 * b.a13 + a.a12 * b.a23 + a.a13 * b.a33 + a.a14 * b.a43,
        a.a11 * b.a14 + a.a12 * b.a24 + a.a13 * b.a34 + a.a14 * b.a44,

        a.a21 * b.a11 + a.a22 * b.a21 + a.a23 * b.a31 + a.a24 * b.a41,
        a.a21 * b.a12 + a.a22 * b.a22 + a.a23 * b.a32 + a.a24 * b.a42,
        a.a21 * b.a13 + a.a22 * b.a23 + a.a23 * b.a33 + a.a24 * b.a43,
        a.a21 * b.a14 + a.a22 * b.a24 + a.a23 * b.a34 + a.a24 * b.a44,

        a.a31 * b.a11 + a.a32 * b.a21 + a.a33 * b.a31 + a.a34 * b.a41,
        a.a31 * b.a12 + a.a32 * b.a22 + a.a33 * b.a32 + a.a34 * b.a42,
        a.a31 * b.a13 + a.a32 * b.a23 + a.a33 * b.a33 + a.a34 * b.a43,
        a.a31 * b.a14 + a.a32 * b.a24 + a.a33 * b.a34 + a.a34 * b.a44,

        a.a41 * b.a11 + a.a42 * b.a21 + a.a43 * b.a31 + a.a44 * b.a41,
        a.a41 * b.a12 + a.a42 * b.a22 + a.a43 * b.a32 + a.a44 * b.a42,
        a.a41 * b.a13 + a.a42 * b.a23 + a.a43 * b.a33 + a.a44 * b.a43,
        a.a41 * b.a14 + a.a42 * b.a24 + a.a43 * b.a34 + a.a44 * b.a44,
    };
}

v4f32 fsl_matrix_multiply_vector(m4f32 a, v4f32 b)
{
    return (v4f32){
        a.a11 * b.x + a.a12 * b.x + a.a13 * b.x + a.a14 * b.x,
        a.a21 * b.y + a.a22 * b.y + a.a23 * b.y + a.a24 * b.y,
        a.a31 * b.z + a.a32 * b.z + a.a33 * b.z + a.a34 * b.z,
        a.a41 * b.w + a.a42 * b.w + a.a43 * b.w + a.a44 * b.w,
    };
}

f32 fsl_lerp_f32(f32 a, f32 b, f64 t)
{
    return a + (b - a) * t;
}

f32 fsl_lerp_exp_f32(f32 a, f32 b, f64 k, f32 t)
{
    return a + (b - a) * (1.0f - expf(-k * t));
}

f32 fsl_lerp_cubic_f32(f32 a, f32 b, f32 t)
{
    return a + (b - a) * (3.0f - t * 2.0f) * t * t;
}

v3f64 fsl_lerp_v3f64(v3f64 a, v3f64 b, f32 t)
{
    return (v3f64){
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
    };
}

f32 fsl_easein_f32(f32 a, f32 b, f32 t)
{
    return a + (b - a) * t * t;
}

f32 fsl_smoothstep_f32(f32 a, f32 b, f32 t)
{
   t = fsl_clamp_f32((t - a) / (b - a), -1.0f, 1.0f);
   return t * t * (3.0f - 2.0f * t);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"

f32 fsl_rand_f32(i32 n)
{
    const u32 S = 32;
    u32 a = (i32)n + 234678493574;
    u32 b = (i32)n - 879763936541;

    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    a *= 2048419325;

    return sin((f32)a * FSL_RAND_SCALE);
}

u64 fsl_rand_u64(u64 n)
{
    const u64 S = 63;
    u64 a = (i64)n + 234678493574;
    u64 b = (i64)n - 879763936541;

    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;

    return a * 2048419325;
}

#pragma GCC diagnostic pop /* ignored "-Wshift-count-overflow" */
