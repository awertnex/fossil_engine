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
                (void*)&file_contents, sizeof(f32), "rb", FALSE);
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
                    RAND_TAB, "wb", TRUE, FALSE) != ERR_SUCCESS)
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

f32 gradient_2d(f32 vx, f32 vy, f32 ax, f32 ay, i32 dispx, i32 dispy)
{
    v2f32 sample =
    {
        RAND_TAB[(u32)(world.seed + (28734 + (i32)ax) *
                (64372 + (i32)ay) + dispx) % RAND_TAB_VOLUME],

        RAND_TAB[(u32)(world.seed + (87262 + (i32)ax) *
                (85673 + (i32)ay) + dispy) % RAND_TAB_VOLUME],
    };

    return
        (vx - ax) * sample.x +
        (vy - ay) * sample.y;
}

f32 gradient_3d(f32 vx, f32 vy, f32 vz, f32 ax, f32 ay, f32 az, i32 dispx, i32 dispy, i32 dispz)
{
    v3f32 sample =
    {
        RAND_TAB[(u32)(world.seed + (87534 + (i32)ax) *
                (60977 + (i32)ay) * (87634 + (i32)az) + dispx) % RAND_TAB_VOLUME],

        RAND_TAB[(u32)(world.seed + (57623 + (i32)ax) *
                (32643 + (i32)ay) * (37628 + (i32)az) + dispy) % RAND_TAB_VOLUME],

        RAND_TAB[(u32)(world.seed + (12218 + (i32)ax) *
                (87453 + (i32)ay) * (95853 + (i32)az) + dispz) % RAND_TAB_VOLUME],
    };

    return
        (vx - ax) * sample.x +
        (vy - ay) * sample.y +
        (vz - az) * sample.z;
}

f32 perlin_noise_2d(v2i32 coordinates, f32 amplitude, f32 frequency, i32 dispx, i32 dispy)
{
    f32 vx = (f32)coordinates.x / frequency;
    f32 vy = (f32)coordinates.y / frequency;
    i32 ax = (i32)floorf(vx);
    i32 ay = (i32)floorf(vy);
    i32 bx = ax + 1;
    i32 by = ay + 1;

    f32 dx = vx - (f32)ax;
    f32 dy = vy - (f32)ay;

    f32 g0 = gradient_2d(vx, vy, ax, ay, dispx, dispy);
    f32 g1 = gradient_2d(vx, vy, bx, ay, dispx, dispy);
    f32 l0 = lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_2d(vx, vy, ax, by, dispx, dispy);
    g1 = gradient_2d(vx, vy, bx, by, dispx, dispy);
    f32 l1 = lerp_cubic_f32(g0, g1, dx);

    return lerp_cubic_f32(l0, l1, dy) * amplitude;
}

f32 perlin_noise_2d_ex(v2i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 persistence, f32 gathering, i32 dispx, i32 dispy)
{
    u32 i;
    f32 land_final = 0.0f;
    for (i = 0; i < octaves; ++i)
    {
        land_final += perlin_noise_2d(coordinates, intensity, scale, dispx, dispy);
        intensity *= persistence;
        scale *= gathering;
    }

    return land_final;
}

f32 perlin_noise_3d(v3i32 coordinates, f32 intensity, f32 scale, i32 dispx, i32 dispy, i32 dispz)
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

    f32 g0 = gradient_3d(vx, vy, vz, ax, ay, az, dispx, dispy, dispz);
    f32 g1 = gradient_3d(vx, vy, vz, bx, ay, az, dispx, dispy, dispz);
    f32 l0 = lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_3d(vx, vy, vz, ax, by, az, dispx, dispy, dispz);
    g1 = gradient_3d(vx, vy, vz, bx, by, az, dispx, dispy, dispz);
    f32 l1 = lerp_cubic_f32(g0, g1, dx);

    f32 ll0 = lerp_cubic_f32(l0, l1, dy);

    g0 = gradient_3d(vx, vy, vz, ax, ay, bz, dispx, dispy, dispz);
    g1 = gradient_3d(vx, vy, vz, bx, ay, bz, dispx, dispy, dispz);
    l0 = lerp_cubic_f32(g0, g1, dx);

    g0 = gradient_3d(vx, vy, vz, ax, by, bz, dispx, dispy, dispz);
    g1 = gradient_3d(vx, vy, vz, bx, by, bz, dispx, dispy, dispz);
    l1 = lerp_cubic_f32(g0, g1, dx);

    f32 ll1 = lerp_cubic_f32(l0, l1, dy);

    return lerp_cubic_f32(ll0, ll1, dz) * intensity;
}

f32 perlin_noise_3d_ex(v3i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 persistence, f32 gathering, i32 dispx, i32 dispy, i32 dispz)
{
    u32 i;
    f32 land_final = 0.0f;
    for (i = 0; i < octaves; ++i)
    {
        land_final += perlin_noise_3d(coordinates, intensity, scale, dispx, dispy, dispz);
        intensity *= persistence;
        scale *= gathering * gathering;
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
        biome_blend = perlin_noise_2d(coordinates_2d, 15.0f, 1000.0f, 15, 374),
        crush = fabsf((((f32)coordinates.z - (WORLD_RADIUS_VERTICAL / 2)) / WORLD_CRUSH_FACTOR) + 0.4f) * 0.8f,
        land_final = 0.0f,
        cave_final = 0.0f;

    /* ---- flow control ---------------------------------------------------- */

    elevation = perlin_noise_2d(coordinates_2d, 1.0f, 129.0f, 3, -14) + 0.5f;
    elevation = clamp_f32(elevation, 0.0f, 1.0f);
    influence = perlin_noise_2d(coordinates_2d, 1.0f, 53.0f, 135, -7) + 0.5f;
    influence = clamp_f32(influence, 0.0f, 1.0f);
    gathering = perlin_noise_2d(coordinates_2d, 0.5f, 133.0f, 376, 921);

    /* ---- land shape ------------------------------------------------------ */

    mountains = perlin_noise_2d_ex(coordinates_2d, 250.0f, 255.0f, 3, 0.8f, 0.8f, 72, 853);
    peaks = expf(-perlin_noise_2d(coordinates_2d, 8.0f, 148.0f, 2, 346));
    hills = perlin_noise_2d(coordinates_2d, 30.0f, 41.0f, 273, 239);
    ridges = perlin_noise_2d(coordinates_2d, 10.0f, 12.0f + gathering, 983, 1652);

    /* ---- caves ----------------------------------------------------------- */

    cave_frequency = perlin_noise_3d_ex(coordinates, 1.0f, 208.0f, 2, 0.8f, 0.8f, 4923, 974, 456);
    cave_spaghetti = perlin_noise_3d(coordinates, 1.0f, 22.0f, 0, 299, -239);
    cave_features_big = perlin_noise_3d_ex(coordinates, 0.1f, 190.0f, 3, 0.7f, 0.5f, 133479, 356421, 483094) + 0.04f;
    cave_features_small = perlin_noise_3d_ex(coordinates, 1.0f, 20.0f, 3, 0.7f, 0.75f, 847234, 275613, 986233);
    cave_entrances = perlin_noise_3d(coordinates, 1.0f, 55.0f, 665, 6, -736);
    cave_entrances *= perlin_noise_3d(coordinates, 1.0f, 43.0f, 463, 6523, 3847);

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
        crush = fabsf((((f32)coordinates.z - (WORLD_RADIUS_VERTICAL / 2)) / WORLD_CRUSH_FACTOR) + 0.4f) * 0.8f,
        land_final = 0.0f,
        cave_final = 0.0f;

    gathering = perlin_noise_2d(coordinates_2d, 0.5f, 133.0f, 376, 921);

    mountains = perlin_noise_2d_ex(coordinates_2d, 250.0f, 255.0f, 3, 0.8f, 0.8f, 72, 853);
    ridges = perlin_noise_2d(coordinates_2d, 10.0f, 12.0f + gathering, 983, 1652);

    cave_frequency = perlin_noise_3d_ex(coordinates, 1.0f, 208.0f, 2, 0.8f, 0.8f, 4923, 974, 456);
    cave_spaghetti = perlin_noise_3d(coordinates, 1.0f, 22.0f, 0, 299, -239);
    cave_features_big = perlin_noise_3d_ex(coordinates, 0.1f, 190.0f, 3, 0.7f, 0.5f, 133479, 356421, 483094) + 0.04f;
    cave_features_small = perlin_noise_3d_ex(coordinates, 1.0f, 20.0f, 3, 0.7f, 0.75f, 847234, 275613, 986233);

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
