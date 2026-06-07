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
 *  @file vector.h
 *
 *  @brief vector functions and types.
 */

#ifndef FSL_MATH_VECTOR_H
#define FSL_MATH_VECTOR_H

#include "../common/api.h"
#include "../common/types.h"

/* ---- vector2u ------------------------------------------------------------ */

typedef struct v2u8
{
    u8 x, y;
} v2u8;

typedef struct v2u16
{
    u16 x, y;
} v2u16;

typedef struct v2u32
{
    u32 x, y;
} v2u32;

typedef struct v2u64
{
    u64 x, y;
} v2u64;

/* ---- vector2i ------------------------------------------------------------ */

typedef struct v2i8
{
    i8 x, y;
} v2i8;

typedef struct v2i16
{
    i16 x, y;
} v2i16;

typedef struct v2i32
{
    i32 x, y;
} v2i32;

typedef struct v2i64
{
    i64 x, y;
} v2i64;

/* ---- vector2f ------------------------------------------------------------ */

typedef struct v2f32
{
    f32 x, y;
} v2f32;

typedef struct v2f64
{
    f64 x, y;
} v2f64;

/* ---- vector3u ------------------------------------------------------------ */

typedef struct v3u8
{
    u8 x, y, z;
} v3u8;

typedef struct v3u16
{
    u16 x, y, z;
} v3u16;

typedef struct v3u32
{
    u32 x, y, z;
} v3u32;

typedef struct v3u64
{
    u64 x, y, z;
} v3u64;

/* ---- vector3i ------------------------------------------------------------ */

typedef struct v3i8
{
    i8 x, y, z;
} v3i8;

typedef struct v3i16
{
    i16 x, y, z;
} v3i16;

typedef struct v3i32
{
    i32 x, y, z;
} v3i32;

typedef struct v3i64
{
    i64 x, y, z;
} v3i64;

/* ---- vector3f ------------------------------------------------------------ */

typedef struct v3f32
{
    f32 x, y, z;
} v3f32;

typedef struct v3f64
{
    f64 x, y, z;
} v3f64;

/* ---- vector4u ------------------------------------------------------------ */

typedef struct v4u8
{
    u8 x, y, z, w;
} v4u8;

typedef struct v4u16
{
    u16 x, y, z, w;
} v4u16;

typedef struct v4u32
{
    u32 x, y, z, w;
} v4u32;

typedef struct v4u64
{
    u64 x, y, z, w;
} v4u64;

/* ---- vector4i ------------------------------------------------------------ */

typedef struct v4i8
{
    i8 x, y, z, w;
} v4i8;

typedef struct v4i16
{
    i16 x, y, z, w;
} v4i16;

typedef struct v4i32
{
    i32 x, y, z, w;
} v4i32;

typedef struct v4i64
{
    i64 x, y, z, w;
} v4i64;

/* ---- vector4f ------------------------------------------------------------ */

typedef struct v4f32
{
    f32 x, y, z, w;
} v4f32;

typedef struct v4f64
{
    f64 x, y, z, w;
} v4f64;

FSLAPI v3f32 fsl_add_v3f32(v3f32 a, v3f32 b);
FSLAPI v3f32 fsl_sub_v3f32(v3f32 a, v3f32 b);
FSLAPI f32 fsl_min_v3f32(v3f32 v);
FSLAPI f64 fsl_min_v3f64(v3f64 v);
FSLAPI f32 fsl_max_v3f32(v3f32 v);
FSLAPI f64 fsl_max_v3f64(v3f64 v);
FSLAPI u32 fsl_min_axis_v3f32(v3f32 v);
FSLAPI u64 fsl_min_axis_v3f64(v3f64 v);
FSLAPI u32 fsl_max_axis_v3f32(v3f32 v);
FSLAPI u64 fsl_max_axis_v3f64(v3f64 v);
FSLAPI f32 fsl_len_v3f32(v3f32 v);
FSLAPI f64 fsl_len_v3f64(v3f64 v);
FSLAPI v3f32 fsl_normalize_v3f32(v3f32 v);
FSLAPI v3f64 fsl_normalize_v3f64(v3f64 v);
FSLAPI f32 fsl_dot_v3f32(v3f32 a, v3f32 b);
FSLAPI f64 fsl_dot_v3f64(v3f64 a, v3f64 b);
FSLAPI u32 fsl_distance_v3i32(v3i32 a, v3i32 b);
FSLAPI f32 fsl_distance_v3f32(v3f32 a, v3f32 b);
FSLAPI f64 fsl_distance_v3f64(v3f64 a, v3f64 b);
FSLAPI b8 fsl_is_in_area_i32(v2i32 v, v2i32 min, v2i32 max);
FSLAPI b8 fsl_is_in_area_f32(v2f32 v, v2f32 min, v2f32 max);
FSLAPI b8 fsl_is_in_volume_i32(v3i32 v, v3i32 min, v3i32 max);
FSLAPI b8 fsl_is_in_volume_f32(v3f32 v, v3f32 min, v3f32 max);
FSLAPI b8 fsl_is_in_volume_i64(v3i64 v, v3i64 min, v3i64 max);
FSLAPI b8 fsl_is_in_volume_f64(v3f64 v, v3f64 min, v3f64 max);
FSLAPI v3f64 fsl_lerp_v3f64(v3f64 a, v3f64 b, f32 t);

#endif /* FSL_MATH_VECTOR_H */
