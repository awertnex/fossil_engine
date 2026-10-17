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

/* ---- vector2 ------------------------------------------------------------- */

typedef struct v2u8
{
    u8 x, y;
} v2u8;

typedef struct v2i8
{
    i8 x, y;
} v2i8;

typedef struct v2u16
{
    u16 x, y;
} v2u16;

typedef struct v2i16
{
    i16 x, y;
} v2i16;

typedef struct v2u32
{
    u32 x, y;
} v2u32;

typedef struct v2i32
{
    i32 x, y;
} v2i32;

typedef struct v2f32
{
    f32 x, y;
} v2f32;

typedef struct v2u64
{
    u64 x, y;
} v2u64;

typedef struct v2i64
{
    i64 x, y;
} v2i64;

typedef struct v2f64
{
    f64 x, y;
} v2f64;

/* ---- vector3 ------------------------------------------------------------- */

typedef struct v3u8
{
    u8 x, y, z;
} v3u8;

typedef struct v3i8
{
    i8 x, y, z;
} v3i8;

typedef struct v3u16
{
    u16 x, y, z;
} v3u16;

typedef struct v3i16
{
    i16 x, y, z;
} v3i16;

typedef struct v3u32
{
    u32 x, y, z;
} v3u32;

typedef struct v3i32
{
    i32 x, y, z;
} v3i32;

typedef struct v3f32
{
    f32 x, y, z;
} v3f32;

typedef struct v3u64
{
    u64 x, y, z;
} v3u64;

typedef struct v3i64
{
    i64 x, y, z;
} v3i64;

typedef struct v3f64
{
    f64 x, y, z;
} v3f64;

/* ---- vector4 ------------------------------------------------------------- */

typedef struct v4u8
{
    u8 x, y, z, w;
} v4u8;

typedef struct v4i8
{
    i8 x, y, z, w;
} v4i8;

typedef struct v4u16
{
    u16 x, y, z, w;
} v4u16;

typedef struct v4i16
{
    i16 x, y, z, w;
} v4i16;

typedef struct v4u32
{
    u32 x, y, z, w;
} v4u32;

typedef struct v4i32
{
    i32 x, y, z, w;
} v4i32;

typedef struct v4f32
{
    f32 x, y, z, w;
} v4f32;

typedef struct v4u64
{
    u64 x, y, z, w;
} v4u64;

typedef struct v4i64
{
    i64 x, y, z, w;
} v4i64;

typedef struct v4f64
{
    f64 x, y, z, w;
} v4f64;

FSLAPI v2u8 fsl_add_v2u8(v2u8 a, v2u8 b);
FSLAPI v3u8 fsl_add_v3u8(v3u8 a, v3u8 b);
FSLAPI v4u8 fsl_add_v4u8(v4u8 a, v4u8 b);
FSLAPI v2i8 fsl_add_v2i8(v2i8 a, v2i8 b);
FSLAPI v3i8 fsl_add_v3i8(v3i8 a, v3i8 b);
FSLAPI v4i8 fsl_add_v4i8(v4i8 a, v4i8 b);
FSLAPI v2u16 fsl_add_v2u16(v2u16 a, v2u16 b);
FSLAPI v3u16 fsl_add_v3u16(v3u16 a, v3u16 b);
FSLAPI v4u16 fsl_add_v4u16(v4u16 a, v4u16 b);
FSLAPI v2i16 fsl_add_v2i16(v2i16 a, v2i16 b);
FSLAPI v3i16 fsl_add_v3i16(v3i16 a, v3i16 b);
FSLAPI v4i16 fsl_add_v4i16(v4i16 a, v4i16 b);
FSLAPI v2u32 fsl_add_v2u32(v2u32 a, v2u32 b);
FSLAPI v3u32 fsl_add_v3u32(v3u32 a, v3u32 b);
FSLAPI v4u32 fsl_add_v4u32(v4u32 a, v4u32 b);
FSLAPI v2i32 fsl_add_v2i32(v2i32 a, v2i32 b);
FSLAPI v3i32 fsl_add_v3i32(v3i32 a, v3i32 b);
FSLAPI v4i32 fsl_add_v4i32(v4i32 a, v4i32 b);
FSLAPI v2f32 fsl_add_v2f32(v2f32 a, v2f32 b);
FSLAPI v3f32 fsl_add_v3f32(v3f32 a, v3f32 b);
FSLAPI v4f32 fsl_add_v4f32(v4f32 a, v4f32 b);
FSLAPI v2u64 fsl_add_v2u64(v2u64 a, v2u64 b);
FSLAPI v3u64 fsl_add_v3u64(v3u64 a, v3u64 b);
FSLAPI v4u64 fsl_add_v4u64(v4u64 a, v4u64 b);
FSLAPI v2i64 fsl_add_v2i64(v2i64 a, v2i64 b);
FSLAPI v3i64 fsl_add_v3i64(v3i64 a, v3i64 b);
FSLAPI v4i64 fsl_add_v4i64(v4i64 a, v4i64 b);
FSLAPI v2f64 fsl_add_v2f64(v2f64 a, v2f64 b);
FSLAPI v3f64 fsl_add_v3f64(v3f64 a, v3f64 b);
FSLAPI v4f64 fsl_add_v4f64(v4f64 a, v4f64 b);

FSLAPI v2u8 fsl_sub_v2u8(v2u8 a, v2u8 b);
FSLAPI v3u8 fsl_sub_v3u8(v3u8 a, v3u8 b);
FSLAPI v4u8 fsl_sub_v4u8(v4u8 a, v4u8 b);
FSLAPI v2i8 fsl_sub_v2i8(v2i8 a, v2i8 b);
FSLAPI v3i8 fsl_sub_v3i8(v3i8 a, v3i8 b);
FSLAPI v4i8 fsl_sub_v4i8(v4i8 a, v4i8 b);
FSLAPI v2u16 fsl_sub_v2u16(v2u16 a, v2u16 b);
FSLAPI v3u16 fsl_sub_v3u16(v3u16 a, v3u16 b);
FSLAPI v4u16 fsl_sub_v4u16(v4u16 a, v4u16 b);
FSLAPI v2i16 fsl_sub_v2i16(v2i16 a, v2i16 b);
FSLAPI v3i16 fsl_sub_v3i16(v3i16 a, v3i16 b);
FSLAPI v4i16 fsl_sub_v4i16(v4i16 a, v4i16 b);
FSLAPI v2u32 fsl_sub_v2u32(v2u32 a, v2u32 b);
FSLAPI v3u32 fsl_sub_v3u32(v3u32 a, v3u32 b);
FSLAPI v4u32 fsl_sub_v4u32(v4u32 a, v4u32 b);
FSLAPI v2i32 fsl_sub_v2i32(v2i32 a, v2i32 b);
FSLAPI v3i32 fsl_sub_v3i32(v3i32 a, v3i32 b);
FSLAPI v4i32 fsl_sub_v4i32(v4i32 a, v4i32 b);
FSLAPI v2f32 fsl_sub_v2f32(v2f32 a, v2f32 b);
FSLAPI v3f32 fsl_sub_v3f32(v3f32 a, v3f32 b);
FSLAPI v4f32 fsl_sub_v4f32(v4f32 a, v4f32 b);
FSLAPI v2u64 fsl_sub_v2u64(v2u64 a, v2u64 b);
FSLAPI v3u64 fsl_sub_v3u64(v3u64 a, v3u64 b);
FSLAPI v4u64 fsl_sub_v4u64(v4u64 a, v4u64 b);
FSLAPI v2i64 fsl_sub_v2i64(v2i64 a, v2i64 b);
FSLAPI v3i64 fsl_sub_v3i64(v3i64 a, v3i64 b);
FSLAPI v4i64 fsl_sub_v4i64(v4i64 a, v4i64 b);
FSLAPI v2f64 fsl_sub_v2f64(v2f64 a, v2f64 b);
FSLAPI v3f64 fsl_sub_v3f64(v3f64 a, v3f64 b);
FSLAPI v4f64 fsl_sub_v4f64(v4f64 a, v4f64 b);

FSLAPI u8 fsl_min_v2u8(v2u8 v);
FSLAPI u8 fsl_min_v3u8(v3u8 v);
FSLAPI u8 fsl_min_v4u8(v4u8 v);
FSLAPI i8 fsl_min_v2i8(v2i8 v);
FSLAPI i8 fsl_min_v3i8(v3i8 v);
FSLAPI i8 fsl_min_v4i8(v4i8 v);
FSLAPI u16 fsl_min_v2u16(v2u16 v);
FSLAPI u16 fsl_min_v3u16(v3u16 v);
FSLAPI u16 fsl_min_v4u16(v4u16 v);
FSLAPI i16 fsl_min_v2i16(v2i16 v);
FSLAPI i16 fsl_min_v3i16(v3i16 v);
FSLAPI i16 fsl_min_v4i16(v4i16 v);
FSLAPI u32 fsl_min_v2u32(v2u32 v);
FSLAPI u32 fsl_min_v3u32(v3u32 v);
FSLAPI u32 fsl_min_v4u32(v4u32 v);
FSLAPI i32 fsl_min_v2i32(v2i32 v);
FSLAPI i32 fsl_min_v3i32(v3i32 v);
FSLAPI i32 fsl_min_v4i32(v4i32 v);
FSLAPI f32 fsl_min_v2f32(v2f32 v);
FSLAPI f32 fsl_min_v3f32(v3f32 v);
FSLAPI f32 fsl_min_v4f32(v4f32 v);
FSLAPI u64 fsl_min_v2u64(v2u64 v);
FSLAPI u64 fsl_min_v3u64(v3u64 v);
FSLAPI u64 fsl_min_v4u64(v4u64 v);
FSLAPI i64 fsl_min_v2i64(v2i64 v);
FSLAPI i64 fsl_min_v3i64(v3i64 v);
FSLAPI i64 fsl_min_v4i64(v4i64 v);
FSLAPI f64 fsl_min_v2f64(v2f64 v);
FSLAPI f64 fsl_min_v3f64(v3f64 v);
FSLAPI f64 fsl_min_v4f64(v4f64 v);

FSLAPI u8 fsl_max_v2u8(v2u8 v);
FSLAPI u8 fsl_max_v3u8(v3u8 v);
FSLAPI u8 fsl_max_v4u8(v4u8 v);
FSLAPI i8 fsl_max_v2i8(v2i8 v);
FSLAPI i8 fsl_max_v3i8(v3i8 v);
FSLAPI i8 fsl_max_v4i8(v4i8 v);
FSLAPI u16 fsl_max_v2u16(v2u16 v);
FSLAPI u16 fsl_max_v3u16(v3u16 v);
FSLAPI u16 fsl_max_v4u16(v4u16 v);
FSLAPI i16 fsl_max_v2i16(v2i16 v);
FSLAPI i16 fsl_max_v3i16(v3i16 v);
FSLAPI i16 fsl_max_v4i16(v4i16 v);
FSLAPI u32 fsl_max_v2u32(v2u32 v);
FSLAPI u32 fsl_max_v3u32(v3u32 v);
FSLAPI u32 fsl_max_v4u32(v4u32 v);
FSLAPI i32 fsl_max_v2i32(v2i32 v);
FSLAPI i32 fsl_max_v3i32(v3i32 v);
FSLAPI i32 fsl_max_v4i32(v4i32 v);
FSLAPI f32 fsl_max_v2f32(v2f32 v);
FSLAPI f32 fsl_max_v3f32(v3f32 v);
FSLAPI f32 fsl_max_v4f32(v4f32 v);
FSLAPI u64 fsl_max_v2u64(v2u64 v);
FSLAPI u64 fsl_max_v3u64(v3u64 v);
FSLAPI u64 fsl_max_v4u64(v4u64 v);
FSLAPI i64 fsl_max_v2i64(v2i64 v);
FSLAPI i64 fsl_max_v3i64(v3i64 v);
FSLAPI i64 fsl_max_v4i64(v4i64 v);
FSLAPI f64 fsl_max_v2f64(v2f64 v);
FSLAPI f64 fsl_max_v3f64(v3f64 v);
FSLAPI f64 fsl_max_v4f64(v4f64 v);

FSLAPI u8 fsl_min_axis_v2u8(v2u8 v);
FSLAPI u8 fsl_min_axis_v3u8(v3u8 v);
FSLAPI u8 fsl_min_axis_v4u8(v4u8 v);
FSLAPI u8 fsl_min_axis_v2i8(v2i8 v);
FSLAPI u8 fsl_min_axis_v3i8(v3i8 v);
FSLAPI u8 fsl_min_axis_v4i8(v4i8 v);
FSLAPI u16 fsl_min_axis_v2u16(v2u16 v);
FSLAPI u16 fsl_min_axis_v3u16(v3u16 v);
FSLAPI u16 fsl_min_axis_v4u16(v4u16 v);
FSLAPI u16 fsl_min_axis_v2i16(v2i16 v);
FSLAPI u16 fsl_min_axis_v3i16(v3i16 v);
FSLAPI u16 fsl_min_axis_v4i16(v4i16 v);
FSLAPI u32 fsl_min_axis_v2u32(v2u32 v);
FSLAPI u32 fsl_min_axis_v3u32(v3u32 v);
FSLAPI u32 fsl_min_axis_v4u32(v4u32 v);
FSLAPI u32 fsl_min_axis_v2i32(v2i32 v);
FSLAPI u32 fsl_min_axis_v3i32(v3i32 v);
FSLAPI u32 fsl_min_axis_v4i32(v4i32 v);
FSLAPI u32 fsl_min_axis_v2f32(v2f32 v);
FSLAPI u32 fsl_min_axis_v3f32(v3f32 v);
FSLAPI u32 fsl_min_axis_v4f32(v4f32 v);
FSLAPI u64 fsl_min_axis_v2u64(v2u64 v);
FSLAPI u64 fsl_min_axis_v3u64(v3u64 v);
FSLAPI u64 fsl_min_axis_v4u64(v4u64 v);
FSLAPI u64 fsl_min_axis_v2i64(v2i64 v);
FSLAPI u64 fsl_min_axis_v3i64(v3i64 v);
FSLAPI u64 fsl_min_axis_v4i64(v4i64 v);
FSLAPI u64 fsl_min_axis_v2f64(v2f64 v);
FSLAPI u64 fsl_min_axis_v3f64(v3f64 v);
FSLAPI u64 fsl_min_axis_v4f64(v4f64 v);

FSLAPI u8 fsl_max_axis_v2u8(v2u8 v);
FSLAPI u8 fsl_max_axis_v3u8(v3u8 v);
FSLAPI u8 fsl_max_axis_v4u8(v4u8 v);
FSLAPI u8 fsl_max_axis_v2i8(v2i8 v);
FSLAPI u8 fsl_max_axis_v3i8(v3i8 v);
FSLAPI u8 fsl_max_axis_v4i8(v4i8 v);
FSLAPI u16 fsl_max_axis_v2u16(v2u16 v);
FSLAPI u16 fsl_max_axis_v3u16(v3u16 v);
FSLAPI u16 fsl_max_axis_v4u16(v4u16 v);
FSLAPI u16 fsl_max_axis_v2i16(v2i16 v);
FSLAPI u16 fsl_max_axis_v3i16(v3i16 v);
FSLAPI u16 fsl_max_axis_v4i16(v4i16 v);
FSLAPI u32 fsl_max_axis_v2u32(v2u32 v);
FSLAPI u32 fsl_max_axis_v3u32(v3u32 v);
FSLAPI u32 fsl_max_axis_v4u32(v4u32 v);
FSLAPI u32 fsl_max_axis_v2i32(v2i32 v);
FSLAPI u32 fsl_max_axis_v3i32(v3i32 v);
FSLAPI u32 fsl_max_axis_v4i32(v4i32 v);
FSLAPI u32 fsl_max_axis_v2f32(v2f32 v);
FSLAPI u32 fsl_max_axis_v3f32(v3f32 v);
FSLAPI u32 fsl_max_axis_v4f32(v4f32 v);
FSLAPI u64 fsl_max_axis_v2u64(v2u64 v);
FSLAPI u64 fsl_max_axis_v3u64(v3u64 v);
FSLAPI u64 fsl_max_axis_v4u64(v4u64 v);
FSLAPI u64 fsl_max_axis_v2i64(v2i64 v);
FSLAPI u64 fsl_max_axis_v3i64(v3i64 v);
FSLAPI u64 fsl_max_axis_v4i64(v4i64 v);
FSLAPI u64 fsl_max_axis_v2f64(v2f64 v);
FSLAPI u64 fsl_max_axis_v3f64(v3f64 v);
FSLAPI u64 fsl_max_axis_v4f64(v4f64 v);

FSLAPI b8 fsl_is_in_bounds_v2u8(v2u8 v, v2u8 min, v2u8 max);
FSLAPI b8 fsl_is_in_bounds_v3u8(v3u8 v, v3u8 min, v3u8 max);
FSLAPI b8 fsl_is_in_bounds_v4u8(v4u8 v, v4u8 min, v4u8 max);
FSLAPI b8 fsl_is_in_bounds_v2i8(v2i8 v, v2i8 min, v2i8 max);
FSLAPI b8 fsl_is_in_bounds_v3i8(v3i8 v, v3i8 min, v3i8 max);
FSLAPI b8 fsl_is_in_bounds_v4i8(v4i8 v, v4i8 min, v4i8 max);
FSLAPI b8 fsl_is_in_bounds_v2u16(v2u16 v, v2u16 min, v2u16 max);
FSLAPI b8 fsl_is_in_bounds_v3u16(v3u16 v, v3u16 min, v3u16 max);
FSLAPI b8 fsl_is_in_bounds_v4u16(v4u16 v, v4u16 min, v4u16 max);
FSLAPI b8 fsl_is_in_bounds_v2i16(v2i16 v, v2i16 min, v2i16 max);
FSLAPI b8 fsl_is_in_bounds_v3i16(v3i16 v, v3i16 min, v3i16 max);
FSLAPI b8 fsl_is_in_bounds_v4i16(v4i16 v, v4i16 min, v4i16 max);
FSLAPI b8 fsl_is_in_bounds_v2u32(v2u32 v, v2u32 min, v2u32 max);
FSLAPI b8 fsl_is_in_bounds_v3u32(v3u32 v, v3u32 min, v3u32 max);
FSLAPI b8 fsl_is_in_bounds_v4u32(v4u32 v, v4u32 min, v4u32 max);
FSLAPI b8 fsl_is_in_bounds_v2i32(v2i32 v, v2i32 min, v2i32 max);
FSLAPI b8 fsl_is_in_bounds_v3i32(v3i32 v, v3i32 min, v3i32 max);
FSLAPI b8 fsl_is_in_bounds_v4i32(v4i32 v, v4i32 min, v4i32 max);
FSLAPI b8 fsl_is_in_bounds_v2f32(v2f32 v, v2f32 min, v2f32 max);
FSLAPI b8 fsl_is_in_bounds_v3f32(v3f32 v, v3f32 min, v3f32 max);
FSLAPI b8 fsl_is_in_bounds_v4f32(v4f32 v, v4f32 min, v4f32 max);
FSLAPI b8 fsl_is_in_bounds_v2u64(v2u64 v, v2u64 min, v2u64 max);
FSLAPI b8 fsl_is_in_bounds_v3u64(v3u64 v, v3u64 min, v3u64 max);
FSLAPI b8 fsl_is_in_bounds_v4u64(v4u64 v, v4u64 min, v4u64 max);
FSLAPI b8 fsl_is_in_bounds_v2i64(v2i64 v, v2i64 min, v2i64 max);
FSLAPI b8 fsl_is_in_bounds_v3i64(v3i64 v, v3i64 min, v3i64 max);
FSLAPI b8 fsl_is_in_bounds_v4i64(v4i64 v, v4i64 min, v4i64 max);
FSLAPI b8 fsl_is_in_bounds_v2f64(v2f64 v, v2f64 min, v2f64 max);
FSLAPI b8 fsl_is_in_bounds_v3f64(v3f64 v, v3f64 min, v3f64 max);
FSLAPI b8 fsl_is_in_bounds_v4f64(v4f64 v, v4f64 min, v4f64 max);

FSLAPI u8 fsl_len_squared_v2u8(v2u8 v);
FSLAPI u8 fsl_len_squared_v3u8(v3u8 v);
FSLAPI u8 fsl_len_squared_v4u8(v4u8 v);
FSLAPI i8 fsl_len_squared_v2i8(v2i8 v);
FSLAPI i8 fsl_len_squared_v3i8(v3i8 v);
FSLAPI i8 fsl_len_squared_v4i8(v4i8 v);
FSLAPI u16 fsl_len_squared_v2u16(v2u16 v);
FSLAPI u16 fsl_len_squared_v3u16(v3u16 v);
FSLAPI u16 fsl_len_squared_v4u16(v4u16 v);
FSLAPI i16 fsl_len_squared_v2i16(v2i16 v);
FSLAPI i16 fsl_len_squared_v3i16(v3i16 v);
FSLAPI i16 fsl_len_squared_v4i16(v4i16 v);
FSLAPI u32 fsl_len_squared_v2u32(v2u32 v);
FSLAPI u32 fsl_len_squared_v3u32(v3u32 v);
FSLAPI u32 fsl_len_squared_v4u32(v4u32 v);
FSLAPI i32 fsl_len_squared_v2i32(v2i32 v);
FSLAPI i32 fsl_len_squared_v3i32(v3i32 v);
FSLAPI i32 fsl_len_squared_v4i32(v4i32 v);
FSLAPI f32 fsl_len_squared_v2f32(v2f32 v);
FSLAPI f32 fsl_len_squared_v3f32(v3f32 v);
FSLAPI f32 fsl_len_squared_v4f32(v4f32 v);
FSLAPI u64 fsl_len_squared_v2u64(v2u64 v);
FSLAPI u64 fsl_len_squared_v3u64(v3u64 v);
FSLAPI u64 fsl_len_squared_v4u64(v4u64 v);
FSLAPI i64 fsl_len_squared_v2i64(v2i64 v);
FSLAPI i64 fsl_len_squared_v3i64(v3i64 v);
FSLAPI i64 fsl_len_squared_v4i64(v4i64 v);
FSLAPI f64 fsl_len_squared_v2f64(v2f64 v);
FSLAPI f64 fsl_len_squared_v3f64(v3f64 v);
FSLAPI f64 fsl_len_squared_v4f64(v4f64 v);

FSLAPI u8 fsl_distance_squared_v2u8(v2u8 a, v2u8 b);
FSLAPI u8 fsl_distance_squared_v3u8(v3u8 a, v3u8 b);
FSLAPI u8 fsl_distance_squared_v4u8(v4u8 a, v4u8 b);
FSLAPI i8 fsl_distance_squared_v2i8(v2i8 a, v2i8 b);
FSLAPI i8 fsl_distance_squared_v3i8(v3i8 a, v3i8 b);
FSLAPI i8 fsl_distance_squared_v4i8(v4i8 a, v4i8 b);
FSLAPI u16 fsl_distance_squared_v2u16(v2u16 a, v2u16 b);
FSLAPI u16 fsl_distance_squared_v3u16(v3u16 a, v3u16 b);
FSLAPI u16 fsl_distance_squared_v4u16(v4u16 a, v4u16 b);
FSLAPI i16 fsl_distance_squared_v2i16(v2i16 a, v2i16 b);
FSLAPI i16 fsl_distance_squared_v3i16(v3i16 a, v3i16 b);
FSLAPI i16 fsl_distance_squared_v4i16(v4i16 a, v4i16 b);
FSLAPI u32 fsl_distance_squared_v2u32(v2u32 a, v2u32 b);
FSLAPI u32 fsl_distance_squared_v3u32(v3u32 a, v3u32 b);
FSLAPI u32 fsl_distance_squared_v4u32(v4u32 a, v4u32 b);
FSLAPI i32 fsl_distance_squared_v2i32(v2i32 a, v2i32 b);
FSLAPI i32 fsl_distance_squared_v3i32(v3i32 a, v3i32 b);
FSLAPI i32 fsl_distance_squared_v4i32(v4i32 a, v4i32 b);
FSLAPI f32 fsl_distance_squared_v2f32(v2f32 a, v2f32 b);
FSLAPI f32 fsl_distance_squared_v3f32(v3f32 a, v3f32 b);
FSLAPI f32 fsl_distance_squared_v4f32(v4f32 a, v4f32 b);
FSLAPI u64 fsl_distance_squared_v2u64(v2u64 a, v2u64 b);
FSLAPI u64 fsl_distance_squared_v3u64(v3u64 a, v3u64 b);
FSLAPI u64 fsl_distance_squared_v4u64(v4u64 a, v4u64 b);
FSLAPI i64 fsl_distance_squared_v2i64(v2i64 a, v2i64 b);
FSLAPI i64 fsl_distance_squared_v3i64(v3i64 a, v3i64 b);
FSLAPI i64 fsl_distance_squared_v4i64(v4i64 a, v4i64 b);
FSLAPI f64 fsl_distance_squared_v2f64(v2f64 a, v2f64 b);
FSLAPI f64 fsl_distance_squared_v3f64(v3f64 a, v3f64 b);
FSLAPI f64 fsl_distance_squared_v4f64(v4f64 a, v4f64 b);

FSLAPI v2f32 fsl_normalize_v2f32(v2f32 v);
FSLAPI v3f32 fsl_normalize_v3f32(v3f32 v);
FSLAPI v4f32 fsl_normalize_v4f32(v4f32 v);
FSLAPI v2f64 fsl_normalize_v2f64(v2f64 v);
FSLAPI v3f64 fsl_normalize_v3f64(v3f64 v);
FSLAPI v4f64 fsl_normalize_v4f64(v4f64 v);

FSLAPI f32 fsl_dot_v2f32(v2f32 a, v2f32 b);
FSLAPI f32 fsl_dot_v3f32(v3f32 a, v3f32 b);
FSLAPI f32 fsl_dot_v4f32(v4f32 a, v4f32 b);
FSLAPI f64 fsl_dot_v2f64(v2f64 a, v2f64 b);
FSLAPI f64 fsl_dot_v3f64(v3f64 a, v3f64 b);
FSLAPI f64 fsl_dot_v4f64(v4f64 a, v4f64 b);

#endif /* FSL_MATH_VECTOR_H */
