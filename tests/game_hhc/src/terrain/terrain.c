#include "deps/fossil/common/limits.h"

#include "deps/fossil/math/math.h"
#include "deps/fossil/math/noise.h"
#include "deps/fossil/math/vector.h"

#include "../chunking/chunking.h"

#include "../h/world.h"

#include "terrain.h"

#include <stdio.h>
#include <math.h>

#define TERRAIN_SEED_DEFAULT 0

/* terrain shape */
#define FREQ_CONTINENTAL    (1.0f / 1033.0f)
#define FREQ_REGIONAL       (1.0f / 250.0f)
#define FREQ_LOCAL          (1.0f / 109.0f)
#define FREQ_DETAIL         (1.0f / 16.0f)

/* terrain modifiers (and/or biome selection) */
#define FREQ_TEMPERATURE    (1.0f / 1326.0f)
#define FREQ_HUMIDITY       (1.0f / 1726.0f)
#define FREQ_EXTREMITY      (1.0f / 953.34f)
#define FREQ_ROUGHNESS      (1.0f / 1368.2f)
#define FREQ_LIFE           (1.0f / 2043.04f)

#define TERRAIN_SEA_LEVEL           0
#define TERRAIN_CAVE_LEVEL          ((WORLD_RADIUS_VERTICAL / 2) * CHUNK_DIAMETER)
#define TERRAIN_SQUISH_MAGNITUDE    0.03f

static hhc_biome biome_buf[BIOME_COUNT] = {0};

static hhc_biome biome_init(str *name, f32 temperature, f32 humidity, f32 extremity,
    f32 roughness, f32 depth, f32 life)
{
    hhc_biome biome = {0};
    snprintf(biome.name, FSL_ID_CAP, "%s", name);
    biome.temperature = temperature;
    biome.humidity = humidity;
    biome.extremity = extremity;
    biome.roughness = roughness;
    biome.depth = depth;
    biome.life = life;
    return biome;
}

f32 biome_score_get(hhc_biome a, hhc_biome b)
{
    b.temperature -= a.temperature;
    b.humidity -= a.humidity;
    b.extremity -= a.extremity;
    b.roughness -= a.roughness;

    b.temperature *= b.temperature;
    b.humidity *= b.humidity;
    b.extremity *= b.extremity;
    b.roughness *= b.roughness;

    return
        b.temperature +
        b.humidity +
        b.extremity +
        b.roughness;
}

void terrain_init(void)
{
    biome_buf[BIOME_STONE] =
        biome_init("Stone", 0.35f, 0.0f, 0.334f, 0.3f, 0.0f, 0.0f);

    biome_buf[BIOME_HILLS] =
        biome_init("Hills", 0.39f, 0.1f, 0.6f, 0.5f, 70.0f, 50.0f);

    biome_buf[BIOME_SANDSTORM] =
        biome_init("Sandstorm", 0.557f, 0.0f, 0.1f, 0.2f, 0.0f, 0.3f);

    biome_buf[BIOME_DECAYING_LANDS] =
        biome_init("Decaying Lands", 0.163f, 0.04f, 0.9f, 0.953f, 0.0f, 7.0f);

    biome_buf[BIOME_JUNGLE] =
        biome_init("Jungle", 0.27f, 0.59f, 0.43f, 0.78f, 3.0f, 260.0f);

    biome_buf[BIOME_STONE].block = BLOCK_STONE;
    biome_buf[BIOME_HILLS].block = BLOCK_GRASS;
    biome_buf[BIOME_SANDSTORM].block = BLOCK_SAND;
    biome_buf[BIOME_DECAYING_LANDS].block = BLOCK_BLOOD;
    biome_buf[BIOME_JUNGLE].block = BLOCK_DIRTUP;
}

hhc_terrain_noise terrain_noise_make(v3i32 coordinates)
{
    hhc_terrain_noise noise = {0};
    f32 x = (f32)(coordinates.x + 1324);
    f32 y = (f32)(coordinates.y - 3272);
    f32 z = (f32)(coordinates.z - 50);
    f32 coef = 0.6f;

    noise.continental = fsl_perlin_noise_2d(x, y, 1000.0f, FREQ_CONTINENTAL, world.seed);
    noise.regional = fsl_perlin_noise_2d(x, y, 200.0f, FREQ_REGIONAL, world.seed + 10);
    noise.local = fsl_perlin_noise_2d(x, y, 50.0f, FREQ_LOCAL, world.seed + 20);
    noise.detail = fsl_perlin_noise_2d(x, y, 5.0f, FREQ_DETAIL, world.seed + 30);

    noise.temperature = fsl_perlin_noise_2d(x, y, coef, FREQ_TEMPERATURE, world.seed + 40) + coef;
    noise.humidity = fsl_perlin_noise_2d(x, y, coef, FREQ_HUMIDITY, world.seed + 50) + coef;
    noise.extremity = fsl_perlin_noise_2d(x, y, coef, FREQ_EXTREMITY, world.seed + 60) + coef;
    noise.roughness = fsl_perlin_noise_2d(x, y, coef, FREQ_ROUGHNESS, world.seed + 70) + coef;
    noise.life = fsl_perlin_noise_2d(x, y, coef, FREQ_LIFE, world.seed + 80) + coef;

    noise.temperature = fsl_clamp_f32(noise.temperature, 0.0f, 1.0f);
    noise.humidity = fsl_clamp_f32(noise.humidity, 0.0f, 1.0f);
    noise.extremity = fsl_clamp_f32(noise.extremity, 0.0f, 1.0f);
    noise.roughness = fsl_clamp_f32(noise.roughness, 0.0f, 1.0f);
    noise.life = fsl_clamp_f32(noise.life, 0.0f, 1.0f);

    noise.cost = CHUNK_WORK_COST_GENERATE_NON_AIR;
    return noise;
}

hhc_terrain_noise terrain_noise_lerp(const hhc_terrain_noise *a, const hhc_terrain_noise *b, f32 t)
{
    hhc_terrain_noise noise = {0};

    noise.continental = fsl_smoothstep_f32(a->continental, b->continental, t);
    noise.regional = fsl_smoothstep_f32(a->regional, b->regional, t);
    noise.local = fsl_smoothstep_f32(a->local, b->local, t);
    noise.detail = fsl_smoothstep_f32(a->detail, b->detail, t);

    noise.temperature = fsl_smoothstep_f32(a->temperature, b->temperature, t);
    noise.humidity = fsl_smoothstep_f32(a->humidity, b->humidity, t);
    noise.extremity = fsl_smoothstep_f32(a->extremity, b->extremity, t);
    noise.roughness = fsl_smoothstep_f32(a->roughness, b->roughness, t);
    noise.life = fsl_smoothstep_f32(a->life, b->life, t);

    noise.cost = CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE;
    return noise;
}

hhc_terrain_noise terrain_noise_bilerp(
        const hhc_terrain_noise *a, const hhc_terrain_noise *b,
        const hhc_terrain_noise *c, const hhc_terrain_noise *d,
        f32 tx, f32 ty)
{
    hhc_terrain_noise noise[2] = {0};
    chunk_work_cost cost = 0;

    noise[0] = terrain_noise_lerp(a, b, tx);
    cost += noise[0].cost;

    noise[1] = terrain_noise_lerp(c, d, tx);
    cost += noise[1].cost;

    noise[0] = terrain_noise_lerp(&noise[0], &noise[1], ty);
    cost += noise[0].cost;

    return noise[0];
}

hhc_terrain terrain_shape(v3i32 coordinates, const hhc_terrain_noise *noise)
{
    hhc_terrain terrain = {0};
    hhc_biome biome = {0};
    f32 biome_score[BIOME_COUNT] = {0};
    f32 biome_best_score = 10000.0f;
    i32 biome_best_index = 0;
    i32 i = BIOME_COUNT;

    biome.temperature = noise->temperature;
    biome.humidity = noise->humidity;
    biome.extremity = noise->extremity;
    biome.roughness = noise->roughness;
    biome.life = noise->life;

    while (i--)
    {
        biome_score[i] = biome_score_get(biome, biome_buf[i]);
        if (biome_score[i] < biome_best_score)
        {
            biome_best_score = biome_score[i];
            biome_best_index = i;
        }
    }

    terrain.value =
        ((noise->continental + noise->regional) * (1.0f - noise->temperature) +
        noise->local) * noise->extremity +
        noise->detail * noise->roughness;

    if ((f32)coordinates.z < terrain.value)
    {
        terrain.block_id = biome_buf[biome_best_index].block;
    }

    return terrain;
}

hhc_terrain terrain_land(v3i32 coordinates)
{
    i32 x = coordinates.x + 7324;
    i32 y = coordinates.y - 7272;
    i32 z = coordinates.z - 30;
    f32 crush = 0.0f;
    f32 mountains = 0.0f;
    f32 peaks = 0.0f;
    f32 hills = 0.0f;
    f32 ridges = 0.0f;
    f32 elevation = 0.0f;
    f32 influence = 0.0f;
    f32 gathering = 0.0f;
    f32 cave_frequency = 0.0f;
    f32 cave_spaghetti = 0.0f;
    f32 cave_features_big = 0.0f;
    f32 cave_features_small = 0.0f;
    f32 cave_entrances = 0.0f;
    f32 cave_level = 0.0f;
    f32 biome_blend = 0.0f;
    f32 land_final = 0.0f;
    f32 cave_final = 0.0f;
    hhc_terrain terrain = {0};

    crush = fabsf(((f32)(z + TERRAIN_CAVE_LEVEL) / (WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER)) + 0.4f) * 0.8f;

    biome_blend = fsl_perlin_noise_2d(x, y, 15.0f, 1000.0f, world.seed + 7543);

    /* ---- flow control ---------------------------------------------------- */

    elevation = fsl_perlin_noise_2d(x, y, 1.0f, 129.0f, world.seed + 34723) + 0.5f;
    elevation = fsl_clamp_f32(elevation, 0.0f, 1.0f);
    influence = fsl_perlin_noise_2d(x, y, 1.0f, 53.0f, world.seed - 3792374) + 0.5f;
    influence = fsl_clamp_f32(influence, 0.0f, 1.0f);
    gathering = fsl_perlin_noise_2d(x, y, 0.5f, 133.0f, world.seed + 4777348);

    /* ---- land shape ------------------------------------------------------ */

    mountains = fsl_perlin_noise_2d_ex(x, y, 250.0f, 255.0f, 3, 0.8f, 0.8f, world.seed + 3877334);
    peaks = expf(-fsl_perlin_noise_2d(x, y, 8.0f, 148.0f, world.seed - 93498));
    hills = fsl_perlin_noise_2d(x, y, 30.0f, 41.0f, world.seed - 2273);
    ridges = fsl_perlin_noise_2d(x, y, 10.0f, 12.0f + gathering, world.seed + 8579888);

    /* ---- caves ----------------------------------------------------------- */

    cave_frequency = fsl_perlin_noise_3d_ex(x, y, z, 1.0f, 208.0f, 2, 0.8f, 0.8f, world.seed + 45829);
    cave_spaghetti = fsl_perlin_noise_3d(x, y, z, 1.0f, 22.0f, world.seed + 263623);
    cave_features_big = fsl_perlin_noise_3d_ex(x, y, z, 0.1f, 190.0f, 3, 0.7f, 0.5f, world.seed + 387473899) + 0.04f;
    cave_features_small = fsl_perlin_noise_3d_ex(x, y, z, 1.0f, 20.0f, 3, 0.7f, 0.75f, world.seed + 34326);
    cave_entrances = fsl_perlin_noise_3d(x, y, z, 1.0f, 55.0f, world.seed - 3742339);
    cave_entrances *= fsl_perlin_noise_3d(x, y, z, 1.0f, 43.0f, world.seed - 57485);

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

    if (cave_level > (f32)z)
    {
        terrain.block_id = BLOCK_STONE;
        if (cave_final > crush)
            terrain.block_id = 0;
    }

    if (land_final < (f32)z || cave_entrances > 0.22f)
        terrain.block_id = 0;

    terrain.value = land_final;
    return terrain;
}

hhc_terrain terrain_decaying_lands(v3i32 coordinates)
{
    hhc_terrain terrain = {0};
    i32 x = coordinates.x + 7324;
    i32 y = coordinates.y - 7272;
    i32 z = coordinates.z + 30;
    f32 crush = 0.0f;
    f32 mountains = 0.0f;
    f32 ridges = 0.0f;
    f32 gathering = 0.0f;
    f32 cave_frequency = 0.0f;
    f32 cave_spaghetti = 0.0f;
    f32 cave_features_big = 0.0f;
    f32 cave_features_small = 0.0f;
    f32 cave_level = 0.0f;
    f32 land_final = 0.0f;
    f32 cave_final = 0.0f;

    crush = fabsf((((f32)z - ((WORLD_RADIUS_VERTICAL * CHUNK_DIAMETER) / 2)) / (WORLD_RADIUS_VERTICAL * CHUNK_DIAMETER)) + 0.4f) * 0.8f;

    gathering = fsl_perlin_noise_2d(x, y, 0.2f, 22.5f, world.seed + 75489);

    mountains = fsl_perlin_noise_2d_ex(x, y, 170.0f, 255.0f, 3, 0.8f, 0.8f, world.seed + 9584);

    ridges = fsl_perlin_noise_2d(x, y, 3.0f, 19.0f + gathering, world.seed - 5873956);

    cave_frequency = fsl_perlin_noise_3d_ex(x, y, z, 1.0f, 208.0f, 2, 0.8f, 0.8f, world.seed + 57394);
    cave_spaghetti = fsl_perlin_noise_3d(x, y, z, 1.0f, 22.0f, world.seed + 377779623);
    cave_features_big = fsl_perlin_noise_3d_ex(x, y, z, 0.1f, 190.0f, 3, 0.7f, 0.5f, world.seed + 7923847) + 0.04f;
    cave_features_small = fsl_perlin_noise_3d_ex(x, y, z, 1.0f, 20.0f, 3, 0.7f, 0.75f, world.seed + 87722234);

    land_final = mountains + ridges;
    cave_final = cave_spaghetti + cave_frequency + cave_features_big + cave_features_small;
    cave_level = land_final - 8.0f;

    terrain.biome = BIOME_DECAYING_LANDS;
    terrain.block_id = BLOCK_GRASS;

    if (cave_level > (f32)z)
        terrain.block_id = BLOCK_STONE;

    if (land_final < (f32)z || cave_final > crush)
        terrain.block_id = 0;

    terrain.value = land_final;
    return terrain;
}

hhc_terrain terrain_biome_blend_test(v3i32 coordinates)
{
    hhc_terrain terrain = {0};
    i32 x = coordinates.x + 7324;
    i32 y = coordinates.y - 7272;
    i32 z = coordinates.z;
    f32 crush = 0.0f;
    f32 squish = 0.0f;
    f32 biome_hills_mountains = 0.0f;
    f32 biome_sandstorm_mountains = 0.0f;
    f32 biome_decaying_lands_caves = 0.0f;
    f32 biome_rocks_mountains = 0.0f;
    f32 biome_hills = 0.0f;
    f32 biome_sandstorm = 0.0f;
    f32 biome_decaying_lands = 0.0f;
    f32 biome_rocks = 0.0f;
    f32 biome_blend = 0.0f;
    f32 land_final = 0.0f;

    crush = fabsf((f32)(z + TERRAIN_CAVE_LEVEL) / (WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER));
    squish = (f32)z * TERRAIN_SQUISH_MAGNITUDE;

    biome_hills_mountains = fsl_perlin_noise_2d_ex(x, y, 200.0f, 255.0f, 3, 0.6f, 0.4f, world.seed + 347);
    biome_sandstorm_mountains = fsl_perlin_noise_2d_ex(x, y, 20.0f, 60.0f, 3, 0.9f, 0.8f, world.seed + 3470);
    biome_decaying_lands_caves = fsl_perlin_noise_3d(x, y, z * 1.5f, 20.0f, 100.0f, world.seed - 34729222) - 3.0f;
    biome_decaying_lands_caves += fsl_perlin_noise_3d_ex(x, y, z * 1.5f, 20.0f, 50.0f, 3, 0.8f, 0.5f, world.seed - 349222) - 1.5f;
    biome_rocks_mountains = fsl_perlin_noise_2d_ex(x, y, 20.0f, 33.0f, 3, 0.9f, 0.7f, world.seed + 357644);

    biome_hills = fsl_perlin_noise_2d(x, y, 1.0f, 20.0f, world.seed + 374744);
    biome_sandstorm = fsl_perlin_noise_2d(x, y, 1.0f, 20.0f, world.seed + 3747440);
    biome_decaying_lands = fsl_perlin_noise_3d_ex(x, y, z * 1.5f, 1.0f, 47.3f, 3, 0.5f, 0.38f, world.seed + 6787888);
    biome_decaying_lands += fsl_perlin_noise_3d(x, y, z, 1.0f, 60.0f, world.seed + 374262287);
    biome_decaying_lands += squish;
    biome_rocks = fsl_perlin_noise_2d(x, y, 0.3f, 20.0f, world.seed + 32742383);

    biome_blend = biome_hills + biome_sandstorm + biome_decaying_lands + biome_rocks;
    biome_blend = fsl_lerp_f32(0.0f, 1.0f, biome_blend);
    land_final = (biome_hills_mountains + biome_sandstorm_mountains + biome_decaying_lands) * biome_blend;

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

    if (biome_decaying_lands > crush || biome_decaying_lands_caves > crush)
        terrain.block_id = 0;

    return terrain;
}
