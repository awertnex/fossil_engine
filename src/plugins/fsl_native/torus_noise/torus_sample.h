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
 *  @file torus_sample.h
 *
 *  @brief general noise functions used to parse samples.
 */

#ifndef FSL_TORUS_SAMPLE_H
#define FSL_TORUS_SAMPLE_H

#include "../../../common/api.h"
#include "../../../common/types.h"
#include "../../../math/vector.h"

typedef struct fsl_torus_sample
{
    f64 v[3];   /* position */
    i64 a[3];   /* `v` floor */
    i64 b[3];   /* `v` ceil */
    f64 dv[3];  /* `v` delta */
    f64 dw[3];  /* `v` weight */
    f64 da[3];  /* `a` delta */
    f64 db[3];  /* `b` delta */
} fsl_torus_sample;

typedef f64 (*fsl_torus_sample_lerp_func)(const f64 *n, const f64 *t);

FSLAPI f64 fsl_torus_sample_nolerp(const f64 *n, const f64 *t);
FSLAPI f64 fsl_torus_sample_lerp(const f64 *n, const f64 *t);
FSLAPI f64 fsl_torus_sample_bilerp(const f64 *n, const f64 *t);
FSLAPI f64 fsl_torus_sample_trilerp(const f64 *n, const f64 *t);
FSLAPI v2f64 fsl_torus_sample_gradient_2d(i32 x, i32 y, u64 seed);
FSLAPI v3f64 fsl_torus_sample_gradient_3d(i32 x, i32 y, i32 z, u64 seed);
FSLAPI void fsl_torus_sample_axis_init(fsl_torus_sample *s, u8 axis, f64 pos, f64 frequency);
FSLAPI f64 fsl_torus_sample_make_2d(const fsl_torus_sample *s, f64 amplitude, u64 seed);
FSLAPI f64 fsl_torus_sample_make_3d(const fsl_torus_sample *s, f64 amplitude, u64 seed);

#endif /* FSL_TORUS_SAMPLE_H */
