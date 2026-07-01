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
 *  @file noise_sampler_sample.c
 *
 *  @brief general noise functions used to parse samples.
 */

#include "../../../math/noise.h"

#include "noise_sampler_sample.h"

#include <math.h>

/*!
 *  @remark these constants are the result of the mathematical expression "3^174"
 *  split into groups of 6 digits, I chose this power because it was the first
 *  power of 3 that included exactly the number of digits to fit into 14 6-digit
 *  long constants with none of them starting with a zero.
 */
#define RAND_CONST_0 104495
#define RAND_CONST_1 676331
#define RAND_CONST_2 778315
#define RAND_CONST_3 966103
#define RAND_CONST_4 878903
#define RAND_CONST_5 450701
#define RAND_CONST_6 989608
#define RAND_CONST_7 781073
#define RAND_CONST_8 244439
#define RAND_CONST_9 950619
#define RAND_CONST_10 431748
#define RAND_CONST_11 912396
#define RAND_CONST_12 904023
#define RAND_CONST_13 371769

f64 fsl_noise_sample_nolerp(const f64 *n, const f64 *t)
{
    (void)t;
    return n[0];
}

f64 fsl_noise_sample_lerp(const f64 *n, const f64 *t)
{
    return n[0] + (n[1] - n[0]) * t[0];
}

f64 fsl_noise_sample_bilerp(const f64 *n, const f64 *t)
{
    f64 w[2] = {0};
    w[0] = 1.0 - t[0];
    w[1] = 1.0 - t[1];
    return
        n[0] * w[0] * w[1] +
        n[1] * t[0] * w[1] +
        n[2] * w[0] * t[1] +
        n[3] * t[0] * t[1];
}

f64 fsl_noise_sample_trilerp(const f64 *n, const f64 *t)
{
    f64 w[3] = {0};
    w[0] = 1.0 - t[0];
    w[1] = 1.0 - t[1];
    w[2] = 1.0 - t[2];
    return
        n[0] * w[0] * w[1] * w[2] +
        n[1] * t[0] * w[1] * w[2] +
        n[2] * w[0] * t[1] * w[2] +
        n[3] * t[0] * t[1] * w[2] +
        n[4] * w[0] * w[1] * t[2] +
        n[5] * t[0] * w[1] * t[2] +
        n[6] * w[0] * t[1] * t[2] +
        n[7] * t[0] * t[1] * t[2];
}

v2f64 fsl_noise_sample_gradient_2d(i32 x, i32 y, u64 seed)
{
    v2f64 v = {0};
    u64 h = {0};
    h = x * RAND_CONST_1;
    h ^= y * RAND_CONST_2;
    h ^= h >> 16;
    h *= RAND_CONST_0;

    v.x = fsl_rand_tab[(seed + ((h >> 0) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    v.y = fsl_rand_tab[(seed + ((h >> 30) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    return v;
}

v3f64 fsl_noise_sample_gradient_3d(i32 x, i32 y, i32 z, u64 seed)
{
    v3f64 v = {0};
    u64 h = {0};
    h = x * RAND_CONST_1;
    h ^= y * RAND_CONST_2;
    h ^= z * RAND_CONST_3;
    h ^= h >> 16;
    h *= RAND_CONST_0;
    v.x = fsl_rand_tab[(seed + ((h >> 0) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    v.y = fsl_rand_tab[(seed + ((h >> 20) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    v.z = fsl_rand_tab[(seed + ((h >> 40) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
    return v;
}

void fsl_noise_sample_axis_init(fsl_noise_sample *s, u8 axis, f64 pos, f64 frequency)
{
    f64 v = pos * frequency;
    i64 a = (i64)floorf(v);
    i64 b = a + 1;
    f64 d = v - (f64)a;
    d = d * d * d * (d * (d * 6.0f - 15.0f) + 10.0f);

    s->v[axis] = v;
    s->a[axis] = a;
    s->b[axis] = b;
    s->dv[axis] = d;
    s->dw[axis] = 1.0 - d;
    s->da[axis] = v - (f64)a;
    s->db[axis] = v - (f64)b;
}

f64 fsl_noise_sample_make_2d(const fsl_noise_sample *s, f64 amplitude, u64 seed)
{
    const f64 dx = s->dv[0];
    const f64 dy = s->dv[1];
    const f64 wx = s->dw[0];
    const f64 wy = s->dw[1];
    f64 n[4] = {0};
    v2f64 g[4] = {0};

    g[0] = fsl_noise_sample_gradient_2d(s->a[0], s->a[1], seed);
    g[1] = fsl_noise_sample_gradient_2d(s->b[0], s->a[1], seed);
    g[2] = fsl_noise_sample_gradient_2d(s->a[0], s->b[1], seed);
    g[3] = fsl_noise_sample_gradient_2d(s->b[0], s->b[1], seed);

    n[0] = s->da[0] * g[0].x + s->da[1] * g[0].y;
    n[1] = s->db[0] * g[1].x + s->da[1] * g[1].y;
    n[2] = s->da[0] * g[2].x + s->db[1] * g[2].y;
    n[3] = s->db[0] * g[3].x + s->db[1] * g[3].y;

    return
        (n[0] * wx * wy +
         n[1] * dx * wy +
         n[2] * wx * dy +
         n[3] * dx * dy) * amplitude;
}

f64 fsl_noise_sample_make_3d(const fsl_noise_sample *s, f64 amplitude, u64 seed)
{
    const f64 dx = s->dv[0];
    const f64 dy = s->dv[1];
    const f64 dz = s->dv[2];
    const f64 wx = s->dw[0];
    const f64 wy = s->dw[1];
    const f64 wz = s->dw[2];
    f64 n[8] = {0};
    v3f64 g[8] = {0};

    g[0] = fsl_noise_sample_gradient_3d(s->a[0], s->a[1], s->a[2], seed);
    g[1] = fsl_noise_sample_gradient_3d(s->b[0], s->a[1], s->a[2], seed);
    g[2] = fsl_noise_sample_gradient_3d(s->a[0], s->b[1], s->a[2], seed);
    g[3] = fsl_noise_sample_gradient_3d(s->b[0], s->b[1], s->a[2], seed);
    g[4] = fsl_noise_sample_gradient_3d(s->a[0], s->a[1], s->b[2], seed);
    g[5] = fsl_noise_sample_gradient_3d(s->b[0], s->a[1], s->b[2], seed);
    g[6] = fsl_noise_sample_gradient_3d(s->a[0], s->b[1], s->b[2], seed);
    g[7] = fsl_noise_sample_gradient_3d(s->b[0], s->b[1], s->b[2], seed);

    n[0] = s->da[0] * g[0].x + s->da[1] * g[0].y + s->da[2] * g[0].z;
    n[1] = s->db[0] * g[1].x + s->da[1] * g[1].y + s->da[2] * g[1].z;
    n[2] = s->da[0] * g[2].x + s->db[1] * g[2].y + s->da[2] * g[2].z;
    n[3] = s->db[0] * g[3].x + s->db[1] * g[3].y + s->da[2] * g[3].z;
    n[4] = s->da[0] * g[4].x + s->da[1] * g[4].y + s->db[2] * g[4].z;
    n[5] = s->db[0] * g[5].x + s->da[1] * g[5].y + s->db[2] * g[5].z;
    n[6] = s->da[0] * g[6].x + s->db[1] * g[6].y + s->db[2] * g[6].z;
    n[7] = s->db[0] * g[7].x + s->db[1] * g[7].y + s->db[2] * g[7].z;

    return
        (n[0] * wx * wy * wz +
         n[1] * dx * wy * wz +
         n[2] * wx * dy * wz +
         n[3] * dx * dy * wz +
         n[4] * wx * wy * dz +
         n[5] * dx * wy * dz +
         n[6] * wx * dy * dz +
         n[7] * dx * dy * dz) * amplitude;
}
