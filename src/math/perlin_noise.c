#include "math.h"
#include "noise.h"

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
    f32 g = fsl_rand_tab[(seed + RAND_CONST_0 + x) % FSL_RAND_TAB_VOLUME];
    return (v - x) * g;
}

f32 fsl_gradient_2d(f32 vx, f32 vy, i32 x, i32 y, u64 seed)
{
    f32 gx = fsl_rand_tab[(seed + (RAND_CONST_1 + x) * (RAND_CONST_2 + y)) % FSL_RAND_TAB_VOLUME];
    f32 gy = fsl_rand_tab[(seed + (RAND_CONST_3 + y) * (RAND_CONST_4 + x)) % FSL_RAND_TAB_VOLUME];
    return (vx - x) * gx + (vy - y) * gy;
}

f32 fsl_gradient_3d(f32 vx, f32 vy, f32 vz, i32 x, i32 y, i32 z, u64 seed)
{
    f32 gx = fsl_rand_tab[(seed + (RAND_CONST_5 + x) * (RAND_CONST_6 + y) * (RAND_CONST_7 + z)) % FSL_RAND_TAB_VOLUME];
    f32 gy = fsl_rand_tab[(seed + (RAND_CONST_8 + y) * (RAND_CONST_9 + z) * (RAND_CONST_10 + x)) % FSL_RAND_TAB_VOLUME];
    f32 gz = fsl_rand_tab[(seed + (RAND_CONST_11 + z) * (RAND_CONST_12 + x) * (RAND_CONST_13 + y)) % FSL_RAND_TAB_VOLUME];
    return (vx - x) * gx + (vy - y) * gy + (vz - z) * gz;
}

f32 fsl_perlin_noise_1d(i32 x, f32 amplitude, f32 frequency, u64 seed)
{
    f32 v = (f32)x / frequency;
    i32 a = (i32)floorf(v);
    i32 b = a + 1;
    f32 d = v - (f32)a;
    f32 g0 = fsl_gradient_1d(v, a, seed);
    f32 g1 = fsl_gradient_1d(v, b, seed);
    return fsl_lerp_cubic_f32(g0, g1, d) * amplitude;
}

f32 fsl_perlin_noise_1d_ex(i32 x, f32 amplitude, f32 frequency,
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

f32 fsl_perlin_noise_2d(i32 x, i32 y, f32 amplitude, f32 frequency, u64 seed)
{
    f32 vx = (f32)x / frequency;
    f32 vy = (f32)y / frequency;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 bx = ax + 1;
    i32 by = ay + 1;
    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;
    f32 g0 = 0.0f;
    f32 g1 = 0.0f;
    f32 l0 = 0.0f;
    f32 l1 = 0.0f;

    g0 = fsl_gradient_2d(vx, vy, ax, ay, seed);
    g1 = fsl_gradient_2d(vx, vy, bx, ay, seed);
    l0 = fsl_lerp_cubic_f32(g0, g1, dx);

    g0 = fsl_gradient_2d(vx, vy, ax, by, seed);
    g1 = fsl_gradient_2d(vx, vy, bx, by, seed);
    l1 = fsl_lerp_cubic_f32(g0, g1, dx);

    return fsl_lerp_cubic_f32(l0, l1, dy) * amplitude;
}

f32 fsl_perlin_noise_2d_ex(i32 x, i32 y, f32 amplitude, f32 frequency,
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

f32 fsl_perlin_noise_3d(i32 x, i32 y, i32 z, f32 amplitude, f32 frequency, u64 seed)
{
    f32 vx = (f32)x / frequency;
    f32 vy = (f32)y / frequency;
    f32 vz = (f32)z / frequency;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 az = (i32)floorf(vz);
    i32 bx = ax + 1;
    i32 by = ay + 1;
    i32 bz = az + 1;
    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;
    f32 dz = vz - (f32)az;
    f32 g0 = 0.0f;
    f32 g1 = 0.0f;
    f32 l0 = 0.0f;
    f32 l1 = 0.0f;
    f32 ll0 = 0.0f;
    f32 ll1 = 0.0f;

    g0 = fsl_gradient_3d(vx, vy, vz, ax, ay, az, seed);
    g1 = fsl_gradient_3d(vx, vy, vz, bx, ay, az, seed);
    l0 = fsl_lerp_cubic_f32(g0, g1, dx);

    g0 = fsl_gradient_3d(vx, vy, vz, ax, by, az, seed);
    g1 = fsl_gradient_3d(vx, vy, vz, bx, by, az, seed);
    l1 = fsl_lerp_cubic_f32(g0, g1, dx);

    ll0 = fsl_lerp_cubic_f32(l0, l1, dy);

    g0 = fsl_gradient_3d(vx, vy, vz, ax, ay, bz, seed);
    g1 = fsl_gradient_3d(vx, vy, vz, bx, ay, bz, seed);
    l0 = fsl_lerp_cubic_f32(g0, g1, dx);

    g0 = fsl_gradient_3d(vx, vy, vz, ax, by, bz, seed);
    g1 = fsl_gradient_3d(vx, vy, vz, bx, by, bz, seed);
    l1 = fsl_lerp_cubic_f32(g0, g1, dx);

    ll1 = fsl_lerp_cubic_f32(l0, l1, dy);

    return fsl_lerp_cubic_f32(ll0, ll1, dz) * amplitude;
}

f32 fsl_perlin_noise_3d_ex(i32 x, i32 y, i32 z, f32 amplitude, f32 frequency,
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
