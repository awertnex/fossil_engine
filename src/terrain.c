#include <engine/h/math.h>
#include <engine/h/memory.h>

#include "h/chunking.h"
#include "h/dir.h"
#include "h/main.h"
#include "h/terrain.h"
#include "h/world.h"

f32 *RAND_TAB = {0};

u32 rand_init(void)
{
    str file_name[PATH_MAX] = {0};
    f32 *file_contents = NULL;
    u64 file_len = 0;
    i32 i;

    if (mem_map((void*)&RAND_TAB, RAND_TAB_VOLUME * sizeof(f32),
                "rand_init().RAND_TAB") != ERR_SUCCESS)
        goto cleanup;

    snprintf(file_name, PATH_MAX, "%slookup_rand_tab.bin", DIR_ROOT[DIR_LOOKUPS]);

    if (is_file_exists(file_name, FALSE) == ERR_SUCCESS)
    {
        file_len = get_file_contents(file_name,
                (void*)&file_contents, sizeof(f32), FALSE);
        if (*GAME_ERR != ERR_SUCCESS || file_contents == NULL)
            goto cleanup;

        for (i = 0; i < RAND_TAB_VOLUME; ++i)
            RAND_TAB[i] = file_contents[i];

        mem_free((void*)&file_contents, file_len,
                "rand_init().file_contents");
    }
    else
    {

        for (i = 0; i < RAND_TAB_VOLUME; ++i)
            RAND_TAB[i] = rand_f32(i);

        if (write_file(file_name, sizeof(i32), RAND_TAB_VOLUME,
                    RAND_TAB, TRUE, FALSE) != ERR_SUCCESS)
            goto cleanup;
    }

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    rand_free();
    return *GAME_ERR;
}

void rand_free(void)
{
    mem_unmap((void*)&RAND_TAB, RAND_TAB_VOLUME * sizeof(f32),
            "rand_free().RAND_TAB");
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"

v3f32 random_2d(i32 x, i32 y, u64 seed)
{
    const u64 S = 64;
    u64 a = x + seed + 94580993;
    u64 b = y - seed - 35786975;

    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    a *= 2048419325;
    f64 land_final = (f64)a * RAND_SCALE;

    return (v3f32){
        sinf((f32)land_final),
        cosf((f32)land_final),
        sinf((f32)land_final + 0.25f),
    };
}

v3f32 random_3d(i32 x, i32 y, i32 z, u64 seed)
{
    const u64 S = 64;
    u64 a = x + seed + 7467244;
    u64 b = y - seed - 4909393;
    u64 c = z + seed + 2500462;

    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    a *= 2048419325;
    c ^= a << S | b >> S;
    c *= 3567382653;
    f64 land_final = (f64)c * RAND_SCALE;

    return (v3f32){
        sinf((f32)land_final),
        cosf((f32)land_final),
        sinf((f32)land_final + 0.25f),
    };
}

#pragma GCC diagnostic pop /* ignored "-Wshift-count-overflow" */

f32 gradient_2d(f32 vx, f32 vy, i32 x, i32 y, u64 seed)
{
    v2f32 sample =
    {
        RAND_TAB[(seed + (234 + x) * (672 + y)) % RAND_TAB_VOLUME],
        RAND_TAB[(seed + (862 + y) * (873 + x)) % RAND_TAB_VOLUME],
    };

    return (vx - x) * sample.x + (vy - y) * sample.y;
}

f32 gradient_3d(f32 vx, f32 vy, f32 vz, i32 x, i32 y, i32 z, u64 seed)
{
    v3f32 sample =
    {
        RAND_TAB[(seed + (834 + x) * (677 + y) * (834 + z)) % RAND_TAB_VOLUME],
        RAND_TAB[(seed + (523 + y) * (343 + z) * (328 + x)) % RAND_TAB_VOLUME],
        RAND_TAB[(seed + (118 + z) * (853 + x) * (953 + y)) % RAND_TAB_VOLUME],
    };

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
    f32 l0 = lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_2d(vx, vy, ax, by, seed);
    g1 = gradient_2d(vx, vy, bx, by, seed);
    f32 l1 = lerp_cubic_f32(g0, g1, dx);

    return lerp_cubic_f32(l0, l1, dy) * amplitude;
}

f32 perlin_noise_2d_ex(v2i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 intensity_persistence, f32 scale_persistence, u64 seed)
{
    u32 i = 0;
    f32 land_final = 0.0f;
    for (; i < octaves; ++i)
    {
        land_final += perlin_noise_2d(coordinates, intensity, scale, seed);
        intensity *= intensity_persistence;
        scale *= scale_persistence;
    }

    return land_final;
}

f32 perlin_noise_3d(v3i32 coordinates, f32 intensity, f32 scale, u64 seed)
{
    f32 vx = (f32)coordinates.x / scale,
        vy = (f32)coordinates.y / scale,
        vz = (f32)coordinates.z / scale;
    i32 ax = (i32)floorf(vx),
        ay = (i32)floorf(vy),
        az = (i32)floorf(vz);
    i32 bx = ax + 1,
        by = ay + 1,
        bz = az + 1;
    f32 dx = vx - (f32)ax,
        dy = vy - (f32)ay,
        dz = vz - (f32)az;

    f32 g0 = gradient_3d(vx, vy, vz, ax, ay, az, seed);
    f32 g1 = gradient_3d(vx, vy, vz, bx, ay, az, seed);
    f32 l0 = lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_3d(vx, vy, vz, ax, by, az, seed);
    g1 = gradient_3d(vx, vy, vz, bx, by, az, seed);
    f32 l1 = lerp_cubic_f32(g0, g1, dx);

    f32 ll0 = lerp_cubic_f32(l0, l1, dy);

    g0 = gradient_3d(vx, vy, vz, ax, ay, bz, seed);
    g1 = gradient_3d(vx, vy, vz, bx, ay, bz, seed);
    l0 = lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_3d(vx, vy, vz, ax, by, bz, seed);
    g1 = gradient_3d(vx, vy, vz, bx, by, bz, seed);
    l1 = lerp_cubic_f32(g0, g1, dx);

    f32 ll1 = lerp_cubic_f32(l0, l1, dy);

    return lerp_cubic_f32(ll0, ll1, dz) * intensity;
}

f32 perlin_noise_3d_ex(v3i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 intensity_persistence, f32 scale_persistence, u64 seed)
{
    u32 i = 0;
    f32 land_final = 0.0f;
    for (; i < octaves; ++i)
    {
        land_final += perlin_noise_3d(coordinates, intensity, scale, seed);
        intensity *= intensity_persistence;
        scale *= scale_persistence;
    }

    return land_final;
}

Terrain terrain_land(v3i32 coordinates)
{
    Terrain terrain = {0};
    coordinates.x += 7324;
    coordinates.y -= 7272;
    coordinates.z -= 30;
    v2i32 coordinates_2d = {coordinates.x, coordinates.y};

    f32 mountains,
        peaks,
        hills,
        ridges,
        elevation,
        influence,
        gathering,
        cave_frequency,
        cave_spaghetti,
        cave_features_big,
        cave_features_small,
        cave_entrances,
        cave_level,
        biome_blend = perlin_noise_2d(coordinates_2d, 15.0f, 1000.0f, world.seed + 7543),
        crush = fabsf(((f32)(coordinates.z + TERRAIN_CAVE_LEVEL) / WORLD_DIAMETER_VERTICAL) + 0.4f) * 0.8f,
        land_final = 0.0f,
        cave_final = 0.0f;

    /* ---- flow control ---------------------------------------------------- */

    elevation = perlin_noise_2d(coordinates_2d, 1.0f, 129.0f, world.seed + 34723) + 0.5f;
    elevation = clamp_f32(elevation, 0.0f, 1.0f);
    influence = perlin_noise_2d(coordinates_2d, 1.0f, 53.0f, world.seed - 3792374) + 0.5f;
    influence = clamp_f32(influence, 0.0f, 1.0f);
    gathering = perlin_noise_2d(coordinates_2d, 0.5f, 133.0f, world.seed + 4777348);

    /* ---- land shape ------------------------------------------------------ */

    mountains = perlin_noise_2d_ex(coordinates_2d, 250.0f, 255.0f, 3, 0.8f, 0.8f, world.seed + 3877334);
    peaks = expf(-perlin_noise_2d(coordinates_2d, 8.0f, 148.0f, world.seed - 93498));
    hills = perlin_noise_2d(coordinates_2d, 30.0f, 41.0f, world.seed - 2273);
    ridges = perlin_noise_2d(coordinates_2d, 10.0f, 12.0f + gathering, world.seed + 8579888);

    /* ---- caves ----------------------------------------------------------- */

    cave_frequency = perlin_noise_3d_ex(coordinates, 1.0f, 208.0f, 2, 0.8f, 0.8f, world.seed + 45829);
    cave_spaghetti = perlin_noise_3d(coordinates, 1.0f, 22.0f, world.seed + 263623);
    cave_features_big = perlin_noise_3d_ex(coordinates, 0.1f, 190.0f, 3, 0.7f, 0.5f, world.seed + 387473899) + 0.04f;
    cave_features_small = perlin_noise_3d_ex(coordinates, 1.0f, 20.0f, 3, 0.7f, 0.75f, world.seed + 34326);
    cave_entrances = perlin_noise_3d(coordinates, 1.0f, 55.0f, world.seed - 3742339);
    cave_entrances *= perlin_noise_3d(coordinates, 1.0f, 43.0f, world.seed - 57485);

    /* ---- land_final ------------------------------------------------------ */

    land_final += mountains * elevation;
    land_final += peaks;
    land_final += hills * elevation;
    land_final += ridges * influence * influence;

    cave_final = cave_spaghetti + cave_frequency + cave_features_big + cave_features_small;
    cave_level = land_final - 8.0f;

    /* ---- biome construction ---------------------------------------------- */

    if (biome_blend > 0.0f)
    {
        terrain.biome = BIOME_HILLS;
        terrain.block_id = BLOCK_GRASS;
    }
    else
    {
        terrain.biome = BIOME_SANDSTORM;
        terrain.block_id = BLOCK_SAND;
    }

    if (cave_level > (f32)coordinates.z)
    {
        terrain.block_id = BLOCK_STONE;
        if (cave_final > crush)
            terrain.block_id = 0;

        terrain.block_light = (u32)map_range_f64(
                    clamp_f64((f64)coordinates.z, -64.0, 0.0),
                    -64.0, 0.0, 2.0, 63.0) << SHIFT_BLOCK_LIGHT;
    }
    else
        terrain.block_light = 63 << SHIFT_BLOCK_LIGHT;

    if (land_final < (f32)coordinates.z || cave_entrances > 0.22f)
        terrain.block_id = 0;

    return terrain;
}

Terrain terrain_decaying_lands(v3i32 coordinates)
{
    Terrain terrain = {0};
    coordinates.x += 7324;
    coordinates.y -= 7272;
    coordinates.z += 30;
    v2i32 coordinates_2d = {coordinates.x, coordinates.y};

    f32 mountains,
        ridges,
        gathering,
        cave_frequency,
        cave_spaghetti,
        cave_features_big,
        cave_features_small,
        cave_level,
        crush = fabsf((((f32)coordinates.z - (WORLD_RADIUS_VERTICAL / 2)) / WORLD_RADIUS_VERTICAL) + 0.4f) * 0.8f,
        land_final = 0.0f,
        cave_final = 0.0f;

    gathering = perlin_noise_2d(coordinates_2d, 0.5f, 133.0f, world.seed + 75489);

    mountains = perlin_noise_2d_ex(coordinates_2d, 250.0f, 255.0f, 3, 0.8f, 0.8f, world.seed + 9584);
    ridges = perlin_noise_2d(coordinates_2d, 10.0f, 12.0f + gathering, world.seed - 5873956);

    cave_frequency = perlin_noise_3d_ex(coordinates, 1.0f, 208.0f, 2, 0.8f, 0.8f, world.seed + 57394);
    cave_spaghetti = perlin_noise_3d(coordinates, 1.0f, 22.0f, world.seed + 377779623);
    cave_features_big = perlin_noise_3d_ex(coordinates, 0.1f, 190.0f, 3, 0.7f, 0.5f, world.seed + 7923847) + 0.04f;
    cave_features_small = perlin_noise_3d_ex(coordinates, 1.0f, 20.0f, 3, 0.7f, 0.75f, world.seed + 87722234);

    land_final = mountains + ridges;
    cave_final = cave_spaghetti + cave_frequency + cave_features_big + cave_features_small;
    cave_level = land_final - 8.0f;

    terrain.biome = BIOME_DECAYING_LANDS;
    terrain.block_id = BLOCK_GRASS;

    if (cave_level > (f32)coordinates.z)
    {
        terrain.block_id = BLOCK_GRASS;
        terrain.block_light = (u32)map_range_f64(
                    clamp_f64((f64)coordinates.z, -64.0, 0.0),
                    -64.0, 0.0, 0.0, 63.0) << SHIFT_BLOCK_LIGHT;
    }
    else terrain.block_light = 63 << SHIFT_BLOCK_LIGHT;

    if (land_final < (f32)coordinates.z || cave_final > crush)
        terrain.block_id = 0;

    return terrain;
}

Terrain terrain_biome_blend_test(v3i32 coordinates)
{
    Terrain terrain = {0};
    v2i32 coordinates_2d = {coordinates.x, coordinates.y};
    v3i32 coordinates_shifted = coordinates;
    coordinates_shifted.z *= 1.5f;

    f32 crush = fabsf((f32)(coordinates.z + TERRAIN_CAVE_LEVEL) / WORLD_DIAMETER_VERTICAL);
    f32 squish = (f32)coordinates.z * TERRAIN_SQUISH_MAGNITUDE;

    f32 biome_hills_mountains = perlin_noise_2d_ex(coordinates_2d, 200.0f, 255.0f, 3, 0.6f, 0.4f, world.seed + 347);
    f32 biome_sandstorm_mountains = perlin_noise_2d_ex(coordinates_2d, 20.0f, 60.0f, 3, 0.9f, 0.8f, world.seed + 3470);
    f32 biome_decaying_lands_caves = perlin_noise_3d(coordinates_shifted, 20.0f, 100.0f, world.seed - 34729222) - 3.0f;
    biome_decaying_lands_caves += perlin_noise_3d_ex(coordinates_shifted, 20.0f, 50.0f, 3, 0.8f, 0.5f, world.seed - 349222) - 1.5f;
    f32 biome_rocks_mountains = perlin_noise_2d_ex(coordinates_2d, 20.0f, 33.0f, 3, 0.9f, 0.7f, world.seed + 357644);

    f32 biome_hills = perlin_noise_2d(coordinates_2d, 1.0f, 20.0f, world.seed + 374744);
    f32 biome_sandstorm = perlin_noise_2d(coordinates_2d, 1.0f, 20.0f, world.seed + 3747440);
    f32 biome_decaying_lands = perlin_noise_3d_ex(coordinates_shifted, 1.0f, 47.3f, 3, 0.5f, 0.38f, world.seed + 6787888);
    biome_decaying_lands += perlin_noise_3d(coordinates, 1.0f, 60.0f, world.seed + 374262287);
    biome_decaying_lands += squish;
    f32 biome_rocks = perlin_noise_2d(coordinates_2d, 0.3f, 20.0f, world.seed + 32742383);

    f32 biome_blend = biome_hills + biome_sandstorm + biome_decaying_lands + biome_rocks;

    biome_blend = lerp_f32(0.0f, 1.0f, biome_blend);
    f32 land_final = (biome_hills_mountains + biome_sandstorm_mountains + biome_decaying_lands) * biome_blend;

    terrain.biome = BIOME_HILLS;
    terrain.block_id = BLOCK_GRASS;

    if (biome_sandstorm > biome_hills + biome_decaying_lands)
    {
        terrain.biome = BIOME_SANDSTORM;
        terrain.block_id = BLOCK_SAND;
    }

    if (biome_decaying_lands > biome_sandstorm)
    {
        terrain.biome = BIOME_DECAYING_LANDS;
        terrain.block_id = BLOCK_BLOOD;
    }

    if (biome_rocks > biome_hills + biome_sandstorm + biome_decaying_lands)
    {
        terrain.block_id = BLOCK_STONE;
    }

    terrain.block_light = 63 << SHIFT_BLOCK_LIGHT;

    if (biome_decaying_lands > crush || biome_decaying_lands_caves > crush)
        terrain.block_id = 0;

    return terrain;
}
