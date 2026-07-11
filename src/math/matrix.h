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
 *  @file matrix.h
 *
 *  @brief matrix functions and types.
 */

#ifndef FSL_MATH_MATRIX_H
#define FSL_MATH_MATRIX_H

#include "../common/api.h"
#include "../common/types.h"
#include "vector.h"

/* ---- matrix4f ------------------------------------------------------------ */

typedef struct m2f32
{
    f32
        a11, a12,
        a21, a22;
} m2f32;

typedef struct m2f64
{
    f64
        a11, a12,
        a21, a22;
} m2f64;

typedef struct m3f32
{
    f32
        a11, a12, a13,
        a21, a22, a23,
        a31, a32, a33;
} m3f32;

typedef struct m3f64
{
    f64
        a11, a12, a13,
        a21, a22, a23,
        a31, a32, a33;
} m3f64;

typedef struct m4f32
{
    f32
        a11, a12, a13, a14,
        a21, a22, a23, a24,
        a31, a32, a33, a34,
        a41, a42, a43, a44;
} m4f32;

typedef struct m4f64
{
    f64
        a11, a12, a13, a14,
        a21, a22, a23, a24,
        a31, a32, a33, a34,
        a41, a42, a43, a44;
} m4f64;

FSLAPI m2f32 fsl_identity_m2f32(void);
FSLAPI m3f32 fsl_identity_m3f32(void);
FSLAPI m4f32 fsl_identity_m4f32(void);
FSLAPI m2f64 fsl_identity_m2f64(void);
FSLAPI m3f64 fsl_identity_m3f64(void);
FSLAPI m4f64 fsl_identity_m4f64(void);
FSLAPI m2f32 fsl_add_m2f32(m2f32 a, m2f32 b);
FSLAPI m3f32 fsl_add_m3f32(m3f32 a, m3f32 b);
FSLAPI m4f32 fsl_add_m4f32(m4f32 a, m4f32 b);
FSLAPI m2f64 fsl_add_m2f64(m2f64 a, m2f64 b);
FSLAPI m3f64 fsl_add_m3f64(m3f64 a, m3f64 b);
FSLAPI m4f64 fsl_add_m4f64(m4f64 a, m4f64 b);
FSLAPI m2f32 fsl_sub_m2f32(m2f32 a, m2f32 b);
FSLAPI m3f32 fsl_sub_m3f32(m3f32 a, m3f32 b);
FSLAPI m4f32 fsl_sub_m4f32(m4f32 a, m4f32 b);
FSLAPI m2f64 fsl_sub_m2f64(m2f64 a, m2f64 b);
FSLAPI m3f64 fsl_sub_m3f64(m3f64 a, m3f64 b);
FSLAPI m4f64 fsl_sub_m4f64(m4f64 a, m4f64 b);
FSLAPI m2f32 fsl_sub_identity_m2f32(m2f32 m);
FSLAPI m3f32 fsl_sub_identity_m3f32(m3f32 m);
FSLAPI m4f32 fsl_sub_identity_m4f32(m4f32 m);
FSLAPI m2f64 fsl_sub_identity_m2f64(m2f64 m);
FSLAPI m3f64 fsl_sub_identity_m3f64(m3f64 m);
FSLAPI m4f64 fsl_sub_identity_m4f64(m4f64 m);
FSLAPI m2f32 fsl_multiply_m2f32(m2f32 a, m2f32 b);
FSLAPI m3f32 fsl_multiply_m3f32(m3f32 a, m3f32 b);
FSLAPI m4f32 fsl_multiply_m4f32(m4f32 a, m4f32 b);
FSLAPI m2f64 fsl_multiply_m2f64(m2f64 a, m2f64 b);
FSLAPI m3f64 fsl_multiply_m3f64(m3f64 a, m3f64 b);
FSLAPI m4f64 fsl_multiply_m4f64(m4f64 a, m4f64 b);
FSLAPI v2f32 fsl_multiply_vector_m2f32(m2f32 m, v2f32 v);
FSLAPI v3f32 fsl_multiply_vector_m3f32(m3f32 m, v3f32 v);
FSLAPI v4f32 fsl_multiply_vector_m4f32(m4f32 m, v4f32 v);
FSLAPI v2f64 fsl_multiply_vector_m2f64(m2f64 m, v2f64 v);
FSLAPI v3f64 fsl_multiply_vector_m3f64(m3f64 m, v3f64 v);
FSLAPI v4f64 fsl_multiply_vector_m4f64(m4f64 m, v4f64 v);
FSLAPI m2f32 fsl_transpose_m2f32(m2f32 m);
FSLAPI m3f32 fsl_transpose_m3f32(m3f32 m);
FSLAPI m4f32 fsl_transpose_m4f32(m4f32 m);
FSLAPI m2f64 fsl_transpose_m2f64(m2f64 m);
FSLAPI m3f64 fsl_transpose_m3f64(m3f64 m);
FSLAPI m4f64 fsl_transpose_m4f64(m4f64 m);
FSLAPI m2f32 fsl_outer_m2f32(v2f32 a, v2f32 b);
FSLAPI m3f32 fsl_outer_m3f32(v3f32 a, v3f32 b);
FSLAPI m4f32 fsl_outer_m4f32(v4f32 a, v4f32 b);
FSLAPI m2f64 fsl_outer_m2f64(v2f64 a, v2f64 b);
FSLAPI m3f64 fsl_outer_m3f64(v3f64 a, v3f64 b);
FSLAPI m4f64 fsl_outer_m4f64(v4f64 a, v4f64 b);

#endif /* FSL_MATH_MATRIX_H */
