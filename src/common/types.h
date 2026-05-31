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
 *  @file types.h
 *
 *  @brief engine's data types.
 */

#ifndef FSL_TYPES_H
#define FSL_TYPES_H

#include <stdint.h>

#ifndef TRUE
#   define TRUE     1
#endif /* TRUE */

#ifndef FALSE
#   define FALSE    0
#endif /* FALSE */

/* ---- data types ---------------------------------------------------------- */

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef float       f32;
typedef double      f64;

typedef char        str;

typedef uint8_t     b8;
typedef uint32_t    b32;

/*!
 *  @brief usually size of each index in an array, or total bytes for a single datum.
 */
typedef u64         fsl_size;

/*!
 *  @brief usually number of indices in an array, or total bytes for a single datum.
 */
typedef u64         fsl_len;

/*!
 *  @brief raw data capacity, in bytes.
 */
typedef u64         fsl_cap;

/*!
 *  @brief relative or absolute file system path.
 */
typedef str         fsl_fs_path;

typedef struct fsl_array
{
    u64 cursor;     /* current usage */
    fsl_cap cap;    /* current capacity of `buf`, in bytes */
    void *buf;      /* raw data */
} fsl_array;

typedef struct fsl_buf
{
    void **i;       /* members of `buf` */
    void *buf;      /* raw data */
    fsl_size size;  /* size of each member, in bytes */
    fsl_len memb;   /* number of `i` members */
    u64 cursor;     /* for iteration, optional */
    b8 loaded;
} fsl_buf;

typedef struct fsl_key_value
{
    b8 loaded;
    void **key;     /* members of `buf_key` */
    void **val;     /* members of `buf_val` */
    void *buf_key;  /* raw data */
    void *buf_val;  /* raw data */
    fsl_len memb;   /* number of members */
    u64 size_key;   /* size of each key, in bytes */
    u64 size_val;   /* size of each val, in bytes */
} fsl_key_value;

#endif /* FSL_TYPES_H */
