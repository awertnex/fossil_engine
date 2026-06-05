#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/math/vector.h"
#include "deps/fossil/memory/memory.h"

#include "deps/fossil/h/dir.h"

#include "../h/diagnostics.h"
#include "../h/dir.h"

#include "perlin_noise.h"

#include <stdio.h>
#include <math.h>

f32 *RAND_TAB = {0};

u32 rand_init(void)
{
    str path[FSL_PATH_CAP] = {0};
    f32 *file_contents = NULL;
    u64 file_len = 0;
    u32 i = 0;

    if (fsl_mem_map((void*)&RAND_TAB, RAND_TAB_VOLUME * sizeof(f32),
                "rand_init().RAND_TAB") != FSL_ERR_SUCCESS)
        goto cleanup;

    snprintf(path, FSL_PATH_CAP, "%slookup_rand_tab.bin", DIR_ROOT[DIR_LOOKUPS]);

    if (fsl_is_file_exists(path, FALSE) == FSL_ERR_SUCCESS)
    {
        file_len = fsl_get_file_contents(path, (void*)&file_contents, FALSE);
        if (*GAME_ERR != FSL_ERR_SUCCESS || file_contents == NULL)
            goto cleanup;

        for (i = 0; i < RAND_TAB_VOLUME; ++i)
            RAND_TAB[i] = file_contents[i];

        fsl_mem_free((void*)&file_contents, file_len,
                "rand_init().file_contents");
    }
    else
    {

        for (i = 0; i < RAND_TAB_VOLUME; ++i)
            RAND_TAB[i] = sin((f64)fsl_rand_u64(i));

        if (fsl_write_file(path, RAND_TAB_VOLUME * sizeof(f32),
                    RAND_TAB, TRUE, FALSE) != FSL_ERR_SUCCESS)
            goto cleanup;

        LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("`RAND_TAB` lookup '%s' Exported\n", path));
    }


    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    rand_free();
    return *GAME_ERR;
}

void rand_free(void)
{
    fsl_mem_unmap((void*)&RAND_TAB, RAND_TAB_VOLUME * sizeof(f32),
            "rand_free().RAND_TAB");
}

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

f32 gradient_2d(f32 vx, f32 vy, i32 x, i32 y, u64 seed)
{
    v3f32 sample = {0};
    sample.x = RAND_TAB[(seed + (234 + x) * (672 + y)) % RAND_TAB_VOLUME];
    sample.y = RAND_TAB[(seed + (862 + y) * (873 + x)) % RAND_TAB_VOLUME];
    return (vx - x) * sample.x + (vy - y) * sample.y;
}

f32 gradient_3d(f32 vx, f32 vy, f32 vz, i32 x, i32 y, i32 z, u64 seed)
{
    v3f32 sample = {0};
    sample.x = RAND_TAB[(seed + (834 + x) * (677 + y) * (834 + z)) % RAND_TAB_VOLUME];
    sample.y = RAND_TAB[(seed + (523 + y) * (343 + z) * (328 + x)) % RAND_TAB_VOLUME];
    sample.z = RAND_TAB[(seed + (118 + z) * (853 + x) * (953 + y)) % RAND_TAB_VOLUME];
    return (vx - x) * sample.x + (vy - y) * sample.y + (vz - z) * sample.z;
}

f32 perlin_noise_2d(v2i32 coordinates, f32 amplitude, f32 frequency, u64 seed)
{
    f32 vx = (f32)coordinates.x / frequency;
    f32 vy = (f32)coordinates.y / frequency;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 bx = ax + 1;
    i32 by = ay + 1;
    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;

    f32 g0 = gradient_2d(vx, vy, ax, ay, seed);
    f32 g1 = gradient_2d(vx, vy, bx, ay, seed);
    f32 l0 = fsl_lerp_cubic_f32(g0, g1, dx);
    f32 l1 = 0.0f;

    g0 = gradient_2d(vx, vy, ax, by, seed);
    g1 = gradient_2d(vx, vy, bx, by, seed);
    l1 = fsl_lerp_cubic_f32(g0, g1, dx);

    return fsl_lerp_cubic_f32(l0, l1, dy) * amplitude;
}

f32 perlin_noise_2d_ex(v2i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 intensity_persistence, f32 scale_persistence, u64 seed)
{
    f32 result = 0.0f;
    while (octaves--)
    {
        result += perlin_noise_2d(coordinates, intensity, scale, seed);
        intensity *= intensity_persistence;
        scale *= scale_persistence;
    }
    return result;
}

f32 perlin_noise_3d(v3i32 coordinates, f32 intensity, f32 scale, u64 seed)
{
    f32 vx = (f32)coordinates.x / scale;
    f32 vy = (f32)coordinates.y / scale;
    f32 vz = (f32)coordinates.z / scale;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 az = (i32)floorf(vz);
    i32 bx = ax + 1;
    i32 by = ay + 1;
    i32 bz = az + 1;
    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;
    f32 dz = vz - (f32)az;

    f32 g0 = gradient_3d(vx, vy, vz, ax, ay, az, seed);
    f32 g1 = gradient_3d(vx, vy, vz, bx, ay, az, seed);
    f32 l0 = fsl_lerp_cubic_f32(g0, g1, dx);
    f32 l1 = 0.0f;
    f32 ll0 = 0.0f;
    f32 ll1 = 0.0f;

    g0 = gradient_3d(vx, vy, vz, ax, by, az, seed);
    g1 = gradient_3d(vx, vy, vz, bx, by, az, seed);
    l1 = fsl_lerp_cubic_f32(g0, g1, dx);

    ll0 = fsl_lerp_cubic_f32(l0, l1, dy);

    g0 = gradient_3d(vx, vy, vz, ax, ay, bz, seed);
    g1 = gradient_3d(vx, vy, vz, bx, ay, bz, seed);
    l0 = fsl_lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_3d(vx, vy, vz, ax, by, bz, seed);
    g1 = gradient_3d(vx, vy, vz, bx, by, bz, seed);
    l1 = fsl_lerp_cubic_f32(g0, g1, dx);

    ll1 = fsl_lerp_cubic_f32(l0, l1, dy);

    return fsl_lerp_cubic_f32(ll0, ll1, dz) * intensity;
}

f32 perlin_noise_3d_ex(v3i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 intensity_persistence, f32 scale_persistence, u64 seed)
{
    f32 result = 0.0f;
    while (octaves--)
    {
        result += perlin_noise_3d(coordinates, intensity, scale, seed);
        intensity *= intensity_persistence;
        scale *= scale_persistence;
    }
    return result;
}
