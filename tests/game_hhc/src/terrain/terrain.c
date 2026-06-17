#include "deps/fossil/math/math.h"
#include "deps/fossil/math/noise.h"
#include "deps/fossil/math/vector.h"

#include "../chunking/chunking.h"

#include "../h/world.h"

#include "terrain.h"

#include <math.h>

#define TERRAIN_SEED_DEFAULT 0

#define TERRAIN_SEA_LEVEL           0
#define TERRAIN_CAVE_LEVEL          (WORLD_RADIUS_VERTICAL / 2)
#define TERRAIN_SQUISH_MAGNITUDE    0.03f

terrain terrain_land(v3i32 coordinates)
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
    terrain result = {0};

    crush = fabsf(((f32)(z + TERRAIN_CAVE_LEVEL) / WORLD_DIAMETER_VERTICAL) + 0.4f) * 0.8f;

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
        result.biome = BIOME_HILLS;
        result.block_id = BLOCK_GRASS;
    }
    else
    {
        result.biome = BIOME_SANDSTORM;
        result.block_id = BLOCK_SAND;
    }

    if (cave_level > (f32)z)
    {
        result.block_id = BLOCK_STONE;
        if (cave_final > crush)
            result.block_id = 0;
    }

    if (land_final < (f32)z || cave_entrances > 0.22f)
        result.block_id = 0;

    result.value = land_final;
    return result;
}

terrain terrain_decaying_lands(v3i32 coordinates)
{
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
    terrain result = {0};

    crush = fabsf((((f32)z - (WORLD_RADIUS_VERTICAL / 2)) / WORLD_RADIUS_VERTICAL) + 0.4f) * 0.8f;

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

    result.biome = BIOME_DECAYING_LANDS;
    result.block_id = BLOCK_GRASS;

    if (cave_level > (f32)z)
        result.block_id = BLOCK_STONE;

    if (land_final < (f32)z || cave_final > crush)
        result.block_id = 0;

    result.cost = CHUNK_WORK_COST_GENERATE_NON_AIR;
    result.value = land_final;
    return result;
}

terrain terrain_biome_blend_test(v3i32 coordinates)
{
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
    terrain result = {0};

    crush = fabsf((f32)(z + TERRAIN_CAVE_LEVEL) / WORLD_DIAMETER_VERTICAL);
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

    result.biome = BIOME_HILLS;
    result.block_id = BLOCK_GRASS;

    if (biome_sandstorm > biome_hills + biome_decaying_lands)
    {
        result.biome = BIOME_SANDSTORM;
        result.block_id = BLOCK_SAND;
    }

    if (biome_decaying_lands > biome_sandstorm)
    {
        result.biome = BIOME_DECAYING_LANDS;
        result.block_id = BLOCK_BLOOD;
    }

    if (biome_rocks > biome_hills + biome_sandstorm + biome_decaying_lands)
    {
        result.block_id = BLOCK_STONE;
    }

    if (biome_decaying_lands > crush || biome_decaying_lands_caves > crush)
        result.block_id = 0;

    return result;
}
