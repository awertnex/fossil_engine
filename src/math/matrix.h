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

#include "../common/engine_info.h"
#include "../common/types.h"
#include "vector.h"

/* ---- matrix4f ------------------------------------------------------------ */

typedef struct m4f32
{
    f32
        a11, a12, a13, a14,
        a21, a22, a23, a24,
        a31, a32, a33, a34,
        a41, a42, a43, a44;
} m4f32;

FSLAPI m4f32 fsl_matrix_unit(void);
FSLAPI m4f32 fsl_matrix_add(m4f32 a, m4f32 b);
FSLAPI m4f32 fsl_matrix_subtract(m4f32 a, m4f32 b);
FSLAPI m4f32 fsl_matrix_multiply(m4f32 a, m4f32 b);
FSLAPI v4f32 fsl_matrix_multiply_vector(m4f32 a, v4f32 b);

#endif /* FSL_MATH_MATRIX_H */
