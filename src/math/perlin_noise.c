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
 *  @file perlin_noise.c
 *
 *  @brief perlin noise functions and gradient samplers.
 */

#include "noise.h"
#include "vector.h"

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

f32 fsl_gradient_1d(f32 v, i32 x, u64 seed)
{
    u64 h = x * RAND_CONST_0;
    h ^= h >> 16;
    h *= RAND_CONST_1;
    return (v - x) * fsl_rand_tab[(seed + h) % FSL_RAND_TAB_VOLUME];
}

f32 fsl_gradient_2d(f32 vx, f32 vy, i32 x, i32 y, u64 seed)
{
    u64 h = {0};
    h = x * RAND_CONST_1;
    h ^= y * RAND_CONST_2;
    h ^= h >> 16;
    h *= RAND_CONST_0;
    return
        (vx - x) * fsl_rand_tab[(seed + ((h >> 0x00) & 0xffffffff))  % FSL_RAND_TAB_VOLUME] +
        (vy - y) * fsl_rand_tab[(seed + ((h >> 0x20) & 0xffffffff))  % FSL_RAND_TAB_VOLUME];
}

f32 fsl_gradient_3d(f32 vx, f32 vy, f32 vz, i32 x, i32 y, i32 z, u64 seed)
{
    u64 h = {0};
    h = x * RAND_CONST_5;
    h ^= y * RAND_CONST_6;
    h ^= z * RAND_CONST_7;
    h ^= h >> 16;
    h *= RAND_CONST_0;
    return
        (vx - x) * fsl_rand_tab[(seed + ((h >> 0) & 0xfffff)) % FSL_RAND_TAB_VOLUME] +
        (vy - y) * fsl_rand_tab[(seed + ((h >> 20) & 0xfffff)) % FSL_RAND_TAB_VOLUME] +
        (vz - z) * fsl_rand_tab[(seed + ((h >> 40) & 0xfffff)) % FSL_RAND_TAB_VOLUME];
}

f32 fsl_perlin_noise_1d(f32 x, f32 amplitude, f32 frequency, u64 seed)
{
    f32 v = x * frequency;
    i32 a = (i32)floorf(v);
    i32 b = a + 1;
    f32 d = v - (f32)a;
    f32 g0 = fsl_gradient_1d(v, a, seed);
    f32 g1 = fsl_gradient_1d(v, b, seed);
    d = d * d * d * (d * (6.0f * d - 15.0f) + 10.0f);
    return (g0 + (g1 - g0) * d) * amplitude;
}

f32 fsl_perlin_noise_2d(f32 x, f32 y, f32 amplitude, f32 frequency, u64 seed)
{
    f32 vx = x * frequency;
    f32 vy = y * frequency;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 bx = ax + 1;
    i32 by = ay + 1;
    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;
    f32 wx = 0.0f;
    f32 wy = 0.0f;
    f32 g[4];

    dx = dx * dx * dx * (dx * (6.0f * dx - 15.0f) + 10.0f);
    dy = dy * dy * dy * (dy * (6.0f * dy - 15.0f) + 10.0f);
    wx = 1.0f - dx;
    wy = 1.0f - dy;

    g[0] = fsl_gradient_2d(vx, vy, ax, ay, seed);
    g[1] = fsl_gradient_2d(vx, vy, bx, ay, seed);
    g[2] = fsl_gradient_2d(vx, vy, ax, by, seed);
    g[3] = fsl_gradient_2d(vx, vy, bx, by, seed);

    return (g[0] * wx * wy +
            g[1] * dx * wy +
            g[2] * wx * dy +
            g[3] * dx * dy) * amplitude;
}

f32 fsl_perlin_noise_3d(f32 x, f32 y, f32 z, f32 amplitude, f32 frequency, u64 seed)
{
    f32 vx = x * frequency;
    f32 vy = y * frequency;
    f32 vz = z * frequency;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 az = (i32)floorf(vz);
    i32 bx = ax + 1;
    i32 by = ay + 1;
    i32 bz = az + 1;
    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;
    f32 dz = vz - (f32)az;
    f32 wx = 0.0f;
    f32 wy = 0.0f;
    f32 wz = 0.0f;
    f32 g[8] = {0};

    dx = dx * dx * dx * (dx * (dx * 6.0f - 15.0f) + 10.0f);
    dy = dy * dy * dy * (dy * (dy * 6.0f - 15.0f) + 10.0f);
    dz = dz * dz * dz * (dz * (dz * 6.0f - 15.0f) + 10.0f);
    wx = 1.0f - dx;
    wy = 1.0f - dy;
    wz = 1.0f - dz;

    g[0] = fsl_gradient_3d(vx, vy, vz, ax, ay, az, seed);
    g[1] = fsl_gradient_3d(vx, vy, vz, bx, ay, az, seed);
    g[2] = fsl_gradient_3d(vx, vy, vz, ax, by, az, seed);
    g[3] = fsl_gradient_3d(vx, vy, vz, bx, by, az, seed);
    g[4] = fsl_gradient_3d(vx, vy, vz, ax, ay, bz, seed);
    g[5] = fsl_gradient_3d(vx, vy, vz, bx, ay, bz, seed);
    g[6] = fsl_gradient_3d(vx, vy, vz, ax, by, bz, seed);
    g[7] = fsl_gradient_3d(vx, vy, vz, bx, by, bz, seed);

    return (g[0] * wx * wy * wz +
            g[1] * dx * wy * wz +
            g[2] * wx * dy * wz +
            g[3] * dx * dy * wz +
            g[4] * wx * wy * dz +
            g[5] * dx * wy * dz +
            g[6] * wx * dy * dz +
            g[7] * dx * dy * dz) * amplitude;
}

f32 fsl_perlin_noise_1d_ex(f32 x, f32 amplitude, f32 frequency,
        u32 octaves, f32 amplitude_persistence, f32 frequency_persistence, u64 seed)
{
    f32 result = 0.0f;
    while (octaves--)
    {
        result += fsl_perlin_noise_1d(x, amplitude, frequency, seed);
        amplitude *= amplitude_persistence;
        frequency *= frequency_persistence;
    }
    return result;
}

f32 fsl_perlin_noise_2d_ex(f32 x, f32 y, f32 amplitude, f32 frequency,
        u32 octaves, f32 amplitude_persistence, f32 frequency_persistence, u64 seed)
{
    f32 result = 0.0f;
    while (octaves--)
    {
        result += fsl_perlin_noise_2d(x, y, amplitude, frequency, seed);
        amplitude *= amplitude_persistence;
        frequency *= frequency_persistence;
    }
    return result;
}

f32 fsl_perlin_noise_3d_ex(f32 x, f32 y, f32 z, f32 amplitude, f32 frequency,
        u32 octaves, f32 amplitude_persistence, f32 frequency_persistence, u64 seed)
{
    f32 result = 0.0f;
    while (octaves--)
    {
        result += fsl_perlin_noise_3d(x, y, z, amplitude, frequency, seed);
        amplitude *= amplitude_persistence;
        frequency *= frequency_persistence;
    }
    return result;
}
