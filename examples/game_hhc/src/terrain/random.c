#include "deps/fossil/math/math.h"
#include "deps/fossil/math/noise.h"
#include "deps/fossil/math/vector.h"

#include <math.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"

v3f32 random_2d(i32 x, i32 y, u64 seed)
{
    const u64 S = 64;
    u64 a = x + seed + 94580993;
    u64 b = y - seed - 35786975;
    f64 land_final = 0.0;
    v3f32 result = {0};

    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    a *= 2048419325;
    land_final = (f64)a * FSL_RAND_SCALE;
    result.x = sin(land_final);
    result.y = sin(land_final + FSL_HALF_PI);
    result.z = sin(land_final + FSL_PI);
    return result;
}

v3f32 random_3d(i32 x, i32 y, i32 z, u64 seed)
{
    const u64 S = 64;
    u64 a = x + seed + 7467244;
    u64 b = y - seed - 4909393;
    u64 c = z + seed + 2500462;
    f64 land_final = 0.0;
    v3f32 result = {0};

    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    a *= 2048419325;
    c ^= a << S | b >> S;
    c *= 3567382653;
    land_final = (f64)c * FSL_RAND_SCALE;
    result.x = sin(land_final);
    result.y = sin(land_final + FSL_HALF_PI);
    result.z = sin(land_final + FSL_PI);
    return result;
}

#pragma GCC diagnostic pop /* ignored "-Wshift-count-overflow" */
