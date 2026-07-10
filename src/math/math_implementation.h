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
 *  @file math_implementation.h
 *
 *  @brief implementation specification of math functions.
 */

#ifndef FSL_MATH_IMPLEMENTATION_H
#define FSL_MATH_IMPLEMENTATION_H

/* ---- section: scalar ----------------------------------------------------- */

#define CLAMP_FUNC_IMPL \
{ \
    return n < min ? min : n > max ? max : n; \
}

#define MOD_FUNC_IMPL \
{ \
    return (n % max + max) % max; \
}

#define ROUND_UP_FUNC_IMPL \
{ \
    return n + (max - (n % max)) % max; \
}

#define MAP_RANGE_FUNC_IMPL \
{ \
    return r_min + ((n - n_min) * (r_max - r_min)) / (n_max - n_min); \
}

#define IS_IN_BOUNDS_FUNC_IMPL \
{ \
    return (n >= min) && (n <= max); \
}

#define LERP_FUNC_IMPL \
{ \
    return a + (b - a) * t; \
}

#define BILERP_FUNC_IMPL(type, suffix) \
{ \
    type wx = 1.0##suffix - tx; \
    type wy = 1.0##suffix - ty; \
    return \
        a * wx * wy + \
        b * tx * wy + \
        c * wx * ty + \
        d * tx * ty; \
}

#define TRILERP_FUNC_IMPL(type, suffix) \
{ \
    type wx = 1.0##suffix - tx; \
    type wy = 1.0##suffix - ty; \
    type wz = 1.0##suffix - tz; \
    return \
        a * wx * wy * wz + \
        b * tx * wy * wz + \
        c * wx * ty * wz + \
        d * tx * ty * wz + \
        e * wx * wy * tz + \
        f * tx * wy * tz + \
        g * wx * ty * tz + \
        h * tx * ty * tz; \
}

#define LERP_EXP_FUNC_IMPL(suffix, exp_func) \
{ \
    return a + (b - a) * (1.0##suffix - exp_func(-k * t)); \
}

#define FADE_EASE_IN_FUNC_IMPL \
{ \
    return t * t; \
}

#define FADE_SMOOTHSTEP_FUNC_IMPL(suffix) \
{ \
    return t * t * (3.0##suffix - 2.0##suffix * t); \
}

#define FADE_QUINTIC_FUNC_IMPL(suffix) \
{ \
    return t * t * t * (t * (t * 6.0##suffix - 15.0##suffix) + 10.0##suffix); \
}

/* ---- section: vector ----------------------------------------------------- */

#define ADD_V2_FUNC_IMPL \
{ \
    a.x += b.x; \
    a.y += b.y; \
    return a; \
}

#define ADD_V3_FUNC_IMPL \
{ \
    a.x += b.x; \
    a.y += b.y; \
    a.z += b.z; \
    return a; \
}

#define ADD_V4_FUNC_IMPL \
{ \
    a.x += b.x; \
    a.y += b.y; \
    a.z += b.z; \
    a.w += b.w; \
    return a; \
}

#define SUBTRACT_V2_FUNC_IMPL \
{ \
    a.x -= b.x; \
    a.y -= b.y; \
    return a; \
}

#define SUBTRACT_V3_FUNC_IMPL \
{ \
    a.x -= b.x; \
    a.y -= b.y; \
    a.z -= b.z; \
    return a; \
}

#define SUBTRACT_V4_FUNC_IMPL \
{ \
    a.x -= b.x; \
    a.y -= b.y; \
    a.z -= b.z; \
    a.w -= b.w; \
    return a; \
}

#define MIN_V2_FUNC_IMPL \
{ \
    return v.x < v.y ? v.x : v.y; \
}

#define MIN_V3_FUNC_IMPL \
{ \
    return v.x < v.y ? v.x < v.z ? v.x : v.z : v.y < v.z ? v.y : v.z; \
}

#define MIN_V4_FUNC_IMPL \
{ \
    return v.x < v.y ? v.x < v.z ? v.x < v.w ? v.x : v.w : v.z < v.w ? v.z : v.w : v.y < v.z ? v.y < v.w ? v.y : v.w : v.z < v.w ? v.z : v.w; \
}

#define MAX_V2_FUNC_IMPL \
{ \
    return v.x > v.y ? v.x : v.y; \
}

#define MAX_V3_FUNC_IMPL \
{ \
    return v.x > v.y ? v.x > v.z ? v.x : v.z : v.y > v.z ? v.y : v.z; \
}

#define MAX_V4_FUNC_IMPL \
{ \
    return v.x > v.y ? v.x > v.z ? v.x > v.w ? v.x : v.w : v.z > v.w ? v.z : v.w : v.y > v.z ? v.y > v.w ? v.y : v.w : v.z > v.w ? v.z : v.w; \
}

#define MIN_AXIS_V2_FUNC_IMPL \
{ \
    return v.x < v.y ? 1 : 2; \
}

#define MIN_AXIS_V3_FUNC_IMPL \
{ \
    return v.x < v.y ? v.x < v.z ? 1 : 3 : v.y < v.z ? 2 : 3; \
}

#define MIN_AXIS_V4_FUNC_IMPL \
{ \
    return v.x < v.y ? v.x < v.z ? v.x < v.w ? 1 : 4 : v.z < v.w ? 3 : 4 : v.y < v.z ? v.y < v.w ? 2 : 4 : v.z < v.w ? 3 : 4; \
}

#define MAX_AXIS_V2_FUNC_IMPL \
{ \
    return v.x > v.y ? 1 : 2; \
}

#define MAX_AXIS_V3_FUNC_IMPL \
{ \
    return v.x > v.y ? v.x > v.z ? 1 : 3 : v.y > v.z ? 2 : 3; \
}

#define MAX_AXIS_V4_FUNC_IMPL \
{ \
    return v.x > v.y ? v.x > v.z ? v.x > v.w ? 1 : 4 : v.z > v.w ? 3 : 4 : v.y > v.z ? v.y > v.w ? 2 : 4 : v.z > v.w ? 3 : 4; \
}

#define IS_IN_BOUNDS_V2_FUNC_IMPL \
{ \
    return \
        (v.x >= min.x) && (v.x <= max.x) && \
        (v.y >= min.y) && (v.y <= max.y); \
}

#define IS_IN_BOUNDS_V3_FUNC_IMPL \
{ \
    return \
        (v.x >= min.x) && (v.x <= max.x) && \
        (v.y >= min.y) && (v.y <= max.y) && \
        (v.z >= min.z) && (v.z <= max.z); \
}

#define IS_IN_BOUNDS_V4_FUNC_IMPL \
{ \
    return \
        (v.x >= min.x) && (v.x <= max.x) && \
        (v.y >= min.y) && (v.y <= max.y) && \
        (v.z >= min.z) && (v.z <= max.z) && \
        (v.w >= min.w) && (v.w <= max.w); \
}

#define LENGTH_V2_FUNC_IMPL \
{ \
    return v.x * v.x + v.y * v.y; \
}

#define LENGTH_V3_FUNC_IMPL \
{ \
    return v.x * v.x + v.y * v.y + v.z * v.z; \
}

#define LENGTH_V4_FUNC_IMPL \
{ \
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; \
}

#define DISTANCE_V2_FUNC_IMPL \
{ \
    return \
        (a.x - b.x) * (a.x - b.x) + \
        (a.y - b.y) * (a.y - b.y); \
}

#define DISTANCE_V3_FUNC_IMPL \
{ \
    return \
        (a.x - b.x) * (a.x - b.x) + \
        (a.y - b.y) * (a.y - b.y) + \
        (a.z - b.z) * (a.z - b.z); \
}

#define DISTANCE_V4_FUNC_IMPL \
{ \
    return \
        (a.x - b.x) * (a.x - b.x) + \
        (a.y - b.y) * (a.y - b.y) + \
        (a.z - b.z) * (a.z - b.z) + \
        (a.w - b.w) * (a.w - b.w); \
}

#define NORMALIZE_V2_FUNC_IMPL(type, sqrt_func, len_func, suffix) \
{ \
    type len = sqrt_func(len_func(v)); \
    if (len < FSL_EPSILON) \
    { \
        v.x = 0.0##suffix; \
        v.y = 0.0##suffix; \
        return v; \
    } \
    v.x /= len; \
    v.y /= len; \
    return v; \
}

#define NORMALIZE_V3_FUNC_IMPL(type, sqrt_func, len_func, suffix) \
{ \
    type len = sqrt_func(len_func(v)); \
    if (len < FSL_EPSILON) \
    { \
        v.x = 0.0##suffix; \
        v.y = 0.0##suffix; \
        v.z = 0.0##suffix; \
        return v; \
    } \
    v.x /= len; \
    v.y /= len; \
    v.z /= len; \
    return v; \
}

#define NORMALIZE_V4_FUNC_IMPL(type, sqrt_func, len_func, suffix) \
{ \
    type len = sqrt_func(len_func(v)); \
    if (len < FSL_EPSILON) \
    { \
        v.x = 0.0##suffix; \
        v.y = 0.0##suffix; \
        v.z = 0.0##suffix; \
        v.w = 0.0##suffix; \
        return v; \
    } \
    v.x /= len; \
    v.y /= len; \
    v.z /= len; \
    v.w /= len; \
    return v; \
}

#define DOT_V2_FUNC_IMPL \
{ \
    return a.x * b.x + a.y * b.y; \
}

#define DOT_V3_FUNC_IMPL \
{ \
    return a.x * b.x + a.y * b.y + a.z * b.z; \
}

#define DOT_V4_FUNC_IMPL \
{ \
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; \
}

#define SLIDE_MASK_V2_FUNC_IMPL(suffix) \
{ \
    b.x += b.y; \
    a.x = 1.0##suffix - a.x * b.x; \
    a.y = 1.0##suffix - a.y * b.x; \
    return a; \
}

#define SLIDE_MASK_V3_FUNC_IMPL(suffix) \
{ \
    b.x += b.y + b.z; \
    a.x = 1.0##suffix - a.x * b.x; \
    a.y = 1.0##suffix - a.y * b.x; \
    a.z = 1.0##suffix - a.z * b.x; \
    return a; \
}

#define SLIDE_MASK_V4_FUNC_IMPL(suffix) \
{ \
    b.x += b.y + b.z + b.w; \
    a.x = 1.0##suffix - a.x * b.x; \
    a.y = 1.0##suffix - a.y * b.x; \
    a.z = 1.0##suffix - a.z * b.x; \
    a.w = 1.0##suffix - a.w * b.x; \
    return a; \
}

/* ---- section: matrix ----------------------------------------------------- */

#define IDENTITY_M2_FUNC_IMPL(type, suffix) \
{ \
    type m = {0}; \
    m.a11 = 1.0##suffix; \
    m.a22 = 1.0##suffix; \
    return m; \
}

#define IDENTITY_M3_FUNC_IMPL(type, suffix) \
{ \
    type m = {0}; \
    m.a11 = 1.0##suffix; \
    m.a22 = 1.0##suffix; \
    m.a33 = 1.0##suffix; \
    return m; \
}

#define IDENTITY_M4_FUNC_IMPL(type, suffix) \
{ \
    type m = {0}; \
    m.a11 = 1.0##suffix; \
    m.a22 = 1.0##suffix; \
    m.a33 = 1.0##suffix; \
    m.a44 = 1.0##suffix; \
    return m; \
}

#define ADD_M2_FUNC_IMPL \
{ \
    a.a11 += b.a11; \
    a.a12 += b.a12; \
    a.a21 += b.a21; \
    a.a22 += b.a22; \
    return a; \
}

#define ADD_M3_FUNC_IMPL \
{ \
    a.a11 += b.a11; \
    a.a12 += b.a12; \
    a.a13 += b.a13; \
    a.a21 += b.a21; \
    a.a22 += b.a22; \
    a.a23 += b.a23; \
    a.a31 += b.a31; \
    a.a32 += b.a32; \
    a.a33 += b.a33; \
    return a; \
}

#define ADD_M4_FUNC_IMPL \
{ \
    a.a11 += b.a11; \
    a.a12 += b.a12; \
    a.a13 += b.a13; \
    a.a14 += b.a14; \
    a.a21 += b.a21; \
    a.a22 += b.a22; \
    a.a23 += b.a23; \
    a.a24 += b.a24; \
    a.a31 += b.a31; \
    a.a32 += b.a32; \
    a.a33 += b.a33; \
    a.a34 += b.a34; \
    a.a41 += b.a41; \
    a.a42 += b.a42; \
    a.a43 += b.a43; \
    a.a44 += b.a44; \
    return a; \
}

#define SUBTRACT_M2_FUNC_IMPL \
{ \
    a.a11 -= b.a11; \
    a.a12 -= b.a12; \
    a.a21 -= b.a21; \
    a.a22 -= b.a22; \
    return a; \
}

#define SUBTRACT_M3_FUNC_IMPL \
{ \
    a.a11 -= b.a11; \
    a.a12 -= b.a12; \
    a.a13 -= b.a13; \
    a.a21 -= b.a21; \
    a.a22 -= b.a22; \
    a.a23 -= b.a23; \
    a.a31 -= b.a31; \
    a.a32 -= b.a32; \
    a.a33 -= b.a33; \
    return a; \
}

#define SUBTRACT_M4_FUNC_IMPL \
{ \
    a.a11 -= b.a11; \
    a.a12 -= b.a12; \
    a.a13 -= b.a13; \
    a.a14 -= b.a14; \
    a.a21 -= b.a21; \
    a.a22 -= b.a22; \
    a.a23 -= b.a23; \
    a.a24 -= b.a24; \
    a.a31 -= b.a31; \
    a.a32 -= b.a32; \
    a.a33 -= b.a33; \
    a.a34 -= b.a34; \
    a.a41 -= b.a41; \
    a.a42 -= b.a42; \
    a.a43 -= b.a43; \
    a.a44 -= b.a44; \
    return a; \
}

#define SUBTRACT_IDENTITY_M2_FUNC_IMPL(suffix) \
{ \
    m.a11 = 1.0##suffix - m.a11; \
    m.a12 = 0.0##suffix - m.a12; \
    m.a21 = 0.0##suffix - m.a21; \
    m.a22 = 1.0##suffix - m.a22; \
    return m; \
}

#define SUBTRACT_IDENTITY_M3_FUNC_IMPL(suffix) \
{ \
    m.a11 = 1.0##suffix - m.a11; \
    m.a12 = 0.0##suffix - m.a12; \
    m.a13 = 0.0##suffix - m.a13; \
    m.a21 = 0.0##suffix - m.a21; \
    m.a22 = 1.0##suffix - m.a22; \
    m.a23 = 0.0##suffix - m.a23; \
    m.a31 = 0.0##suffix - m.a31; \
    m.a32 = 0.0##suffix - m.a32; \
    m.a33 = 1.0##suffix - m.a33; \
    return m; \
}

#define SUBTRACT_IDENTITY_M4_FUNC_IMPL(suffix) \
{ \
    m.a11 = 1.0##suffix - m.a11; \
    m.a12 = 0.0##suffix - m.a12; \
    m.a13 = 0.0##suffix - m.a13; \
    m.a14 = 0.0##suffix - m.a14; \
    m.a21 = 0.0##suffix - m.a21; \
    m.a22 = 1.0##suffix - m.a22; \
    m.a23 = 0.0##suffix - m.a23; \
    m.a24 = 0.0##suffix - m.a24; \
    m.a31 = 0.0##suffix - m.a31; \
    m.a32 = 0.0##suffix - m.a32; \
    m.a33 = 1.0##suffix - m.a33; \
    m.a34 = 0.0##suffix - m.a34; \
    m.a41 = 0.0##suffix - m.a41; \
    m.a42 = 0.0##suffix - m.a42; \
    m.a43 = 0.0##suffix - m.a43; \
    m.a44 = 1.0##suffix - m.a44; \
    return m; \
}

#define MULTIPLY_M2_FUNC_IMPL(type) \
{ \
    type m = {0}; \
    m.a11 = a.a11 * b.a11 + a.a12 * b.a21; \
    m.a12 = a.a11 * b.a12 + a.a12 * b.a22; \
    m.a21 = a.a21 * b.a11 + a.a22 * b.a21; \
    m.a22 = a.a21 * b.a12 + a.a22 * b.a22; \
    return m; \
}

#define MULTIPLY_M3_FUNC_IMPL(type) \
{ \
    type m = {0}; \
    m.a11 = a.a11 * b.a11 + a.a12 * b.a21 + a.a13 * b.a31; \
    m.a12 = a.a11 * b.a12 + a.a12 * b.a22 + a.a13 * b.a32; \
    m.a13 = a.a11 * b.a13 + a.a12 * b.a23 + a.a13 * b.a33; \
    m.a21 = a.a21 * b.a11 + a.a22 * b.a21 + a.a23 * b.a31; \
    m.a22 = a.a21 * b.a12 + a.a22 * b.a22 + a.a23 * b.a32; \
    m.a23 = a.a21 * b.a13 + a.a22 * b.a23 + a.a23 * b.a33; \
    m.a31 = a.a31 * b.a11 + a.a32 * b.a21 + a.a33 * b.a31; \
    m.a32 = a.a31 * b.a12 + a.a32 * b.a22 + a.a33 * b.a32; \
    m.a33 = a.a31 * b.a13 + a.a32 * b.a23 + a.a33 * b.a33; \
    return m; \
}

#define MULTIPLY_M4_FUNC_IMPL(type) \
{ \
    type m = {0}; \
    m.a11 = a.a11 * b.a11 + a.a12 * b.a21 + a.a13 * b.a31 + a.a14 * b.a41; \
    m.a12 = a.a11 * b.a12 + a.a12 * b.a22 + a.a13 * b.a32 + a.a14 * b.a42; \
    m.a13 = a.a11 * b.a13 + a.a12 * b.a23 + a.a13 * b.a33 + a.a14 * b.a43; \
    m.a14 = a.a11 * b.a14 + a.a12 * b.a24 + a.a13 * b.a34 + a.a14 * b.a44; \
    m.a21 = a.a21 * b.a11 + a.a22 * b.a21 + a.a23 * b.a31 + a.a24 * b.a41; \
    m.a22 = a.a21 * b.a12 + a.a22 * b.a22 + a.a23 * b.a32 + a.a24 * b.a42; \
    m.a23 = a.a21 * b.a13 + a.a22 * b.a23 + a.a23 * b.a33 + a.a24 * b.a43; \
    m.a24 = a.a21 * b.a14 + a.a22 * b.a24 + a.a23 * b.a34 + a.a24 * b.a44; \
    m.a31 = a.a31 * b.a11 + a.a32 * b.a21 + a.a33 * b.a31 + a.a34 * b.a41; \
    m.a32 = a.a31 * b.a12 + a.a32 * b.a22 + a.a33 * b.a32 + a.a34 * b.a42; \
    m.a33 = a.a31 * b.a13 + a.a32 * b.a23 + a.a33 * b.a33 + a.a34 * b.a43; \
    m.a34 = a.a31 * b.a14 + a.a32 * b.a24 + a.a33 * b.a34 + a.a34 * b.a44; \
    m.a41 = a.a41 * b.a11 + a.a42 * b.a21 + a.a43 * b.a31 + a.a44 * b.a41; \
    m.a42 = a.a41 * b.a12 + a.a42 * b.a22 + a.a43 * b.a32 + a.a44 * b.a42; \
    m.a43 = a.a41 * b.a13 + a.a42 * b.a23 + a.a43 * b.a33 + a.a44 * b.a43; \
    m.a44 = a.a41 * b.a14 + a.a42 * b.a24 + a.a43 * b.a34 + a.a44 * b.a44; \
    return m; \
}

#define MULTIPLY_VECTOR_M2_FUNC_IMPL(type) \
{ \
    type t = {0}; \
    t.x = m.a11 * v.x + m.a12 * v.x; \
    t.y = m.a21 * v.y + m.a22 * v.y; \
    return t; \
}

#define MULTIPLY_VECTOR_M3_FUNC_IMPL(type) \
{ \
    type t = {0}; \
    t.x = m.a11 * v.x + m.a12 * v.x + m.a13 * v.x; \
    t.y = m.a21 * v.y + m.a22 * v.y + m.a23 * v.y; \
    t.z = m.a31 * v.z + m.a32 * v.z + m.a33 * v.z; \
    return t; \
}

#define MULTIPLY_VECTOR_M4_FUNC_IMPL(type) \
{ \
    type t = {0}; \
    t.x = m.a11 * v.x + m.a12 * v.x + m.a13 * v.x + m.a14 * v.x; \
    t.y = m.a21 * v.y + m.a22 * v.y + m.a23 * v.y + m.a24 * v.y; \
    t.z = m.a31 * v.z + m.a32 * v.z + m.a33 * v.z + m.a34 * v.z; \
    t.w = m.a41 * v.w + m.a42 * v.w + m.a43 * v.w + m.a44 * v.w; \
    return t; \
}

#define TRANSPOSE_M2_FUNC_IMPL(type) \
{ \
    type t = {0}; \
    t.a11 = m.a11; \
    t.a12 = m.a21; \
    t.a21 = m.a12; \
    t.a22 = m.a22; \
    return t; \
}

#define TRANSPOSE_M3_FUNC_IMPL(type) \
{ \
    type t = {0}; \
    t.a11 = m.a11; \
    t.a12 = m.a21; \
    t.a13 = m.a31; \
    t.a21 = m.a12; \
    t.a22 = m.a22; \
    t.a23 = m.a32; \
    t.a31 = m.a13; \
    t.a32 = m.a23; \
    t.a33 = m.a33; \
    return t; \
}

#define TRANSPOSE_M4_FUNC_IMPL(type) \
{ \
    type t = {0}; \
    t.a11 = m.a11; \
    t.a12 = m.a21; \
    t.a13 = m.a31; \
    t.a14 = m.a41; \
    t.a21 = m.a12; \
    t.a22 = m.a22; \
    t.a23 = m.a32; \
    t.a24 = m.a42; \
    t.a31 = m.a13; \
    t.a32 = m.a23; \
    t.a33 = m.a33; \
    t.a34 = m.a43; \
    t.a41 = m.a14; \
    t.a42 = m.a24; \
    t.a43 = m.a34; \
    t.a44 = m.a44; \
    return t; \
}

#define OUTER_PRODUCT_M2_FUNC_IMPL(type) \
{ \
    type m = {0}; \
    m.a11 = a.x * b.x; \
    m.a12 = a.x * b.y; \
    m.a21 = a.y * b.x; \
    m.a22 = a.y * b.y; \
    return m; \
}

#define OUTER_PRODUCT_M3_FUNC_IMPL(type) \
{ \
    type m = {0}; \
    m.a11 = a.x * b.x; \
    m.a12 = a.x * b.y; \
    m.a13 = a.x * b.z; \
    m.a21 = a.y * b.x; \
    m.a22 = a.y * b.y; \
    m.a23 = a.y * b.z; \
    m.a31 = a.z * b.x; \
    m.a32 = a.z * b.y; \
    m.a33 = a.z * b.z; \
    return m; \
}

#define OUTER_PRODUCT_M4_FUNC_IMPL(type) \
{ \
    type m = {0}; \
    m.a11 = a.x * b.x; \
    m.a12 = a.x * b.y; \
    m.a13 = a.x * b.z; \
    m.a14 = a.x * b.w; \
    m.a21 = a.y * b.x; \
    m.a22 = a.y * b.y; \
    m.a23 = a.y * b.z; \
    m.a24 = a.y * b.w; \
    m.a31 = a.z * b.x; \
    m.a32 = a.z * b.y; \
    m.a33 = a.z * b.z; \
    m.a34 = a.z * b.w; \
    m.a41 = a.w * b.x; \
    m.a42 = a.w * b.y; \
    m.a43 = a.w * b.z; \
    m.a44 = a.w * b.w; \
    return m; \
}

#endif /* FSL_MATH_IMPLEMENTATION_H */
