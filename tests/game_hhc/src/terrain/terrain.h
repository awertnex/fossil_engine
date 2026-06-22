#ifndef HHC_TERRAIN_H
#define HHC_TERRAIN_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/common/limits.h"

#include "../chunking/chunk_scheduler.h"

#include "../h/assets.h"

enum hhc_biome_index
{
    BIOME_STONE,
    BIOME_HILLS,
    BIOME_SANDSTORM,
    BIOME_DECAYING_LANDS,
    BIOME_JUNGLE,
    BIOME_COUNT
}; /* hhc_biome_index */

typedef struct hhc_biome
{
    str name[FSL_ID_CAP];
    f32 temperature;
    f32 humidity;
    f32 extremity;  /* big detail height */
    f32 roughness;  /* small detail height */
    f32 depth;
    f32 life;
    enum block_id block;
} hhc_biome;

typedef struct hhc_terrain_noise
{
    f32 continental;
    f32 regional;
    f32 local;
    f32 detail;

    f32 temperature;
    f32 humidity;
    f32 extremity;
    f32 roughness;
    f32 life;

    /*!
     *  @brief cost of noise work.
     */
    chunk_work_cost cost;

} hhc_terrain_noise;

typedef struct hhc_terrain
{
    enum hhc_biome_index biome;
    enum block_id block_id;

    /*!
     *  @brief value/density of block at generation position.
     */
    f32 value;
} hhc_terrain;

typedef hhc_terrain_noise (*hhc_terrain_noise_func)(v3i32 coordinates);
typedef hhc_terrain (*hhc_terrain_func)(v3i32 coordinates, const hhc_terrain_noise *noise);

/*!
 *  @brief initialize default terrain settings and biome parameters.
 */
void terrain_init(void);

/*!
 *  @brief make base noise maps for terrain.
 *
 *  @return terrain noises.
 */
hhc_terrain_noise terrain_noise_make(v3i32 coordinates);

/*!
 *  @brief interpolate all noises in `a` and `b` using cubic interpolation.
 *
 *  @return interpolated terrain noise.
 */
hhc_terrain_noise terrain_noise_lerp(const hhc_terrain_noise *a, const hhc_terrain_noise *b, f32 t);

/*!
 *  @brief interpolate all noises in `a`, `b`, `c` and `d` using bi-cubic interpolation.
 *
 *  @return interpolated terrain noise.
 */
hhc_terrain_noise terrain_noise_bilerp(
        const hhc_terrain_noise *a, const hhc_terrain_noise *b,
        const hhc_terrain_noise *c, const hhc_terrain_noise *d,
        f32 tx, f32 ty);

/*!
 *  @brief default terrain shape.
 *
 *  @return terrain info (e.g., biome) from specified coordinates and noises.
 */
hhc_terrain terrain_shape(v3i32 coordinates, const hhc_terrain_noise *noise);

/*!
 *  @brief compare likelihood of biomes `a` and `b` matching.
 *
 *  euclidean distance between relevant parameters is used.
 *
 *  @return biome score.
 */
f32 biome_score_get(hhc_biome a, hhc_biome b);

/*!
 *  @brief first attempt at terrain generation.
 *
 *  @return terrain info (e.g., block ID at specified coordinates).
 */
hhc_terrain terrain_land(v3i32 coordinates);

/*!
 *  @brief second attempt at terrain generation.
 *
 *  @return terrain info (e.g., block ID at specified coordinates).
 */
hhc_terrain terrain_decaying_lands(v3i32 coordinates);

/*!
 *  @brief third attempt at terrain generation.
 *
 *  @return terrain info (e.g., block ID at specified coordinates).
 */
hhc_terrain terrain_biome_blend_test(v3i32 coordinates);

#endif /* HHC_TERRAIN_H */
