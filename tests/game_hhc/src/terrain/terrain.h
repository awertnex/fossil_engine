#ifndef HHC_TERRAIN_H
#define HHC_TERRAIN_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/common/limits.h"

#include "../chunking/chunk_work.h"

#include "../h/assets.h"

#include "noise_sampler.h"
#include "terrain_common.h"

typedef struct hhc_biome
{
    str name[FSL_ID_CAP];
    f64 spec[TERRAIN_NOISE_COUNT]; /* preferred value for each noise */
} hhc_biome;

typedef struct hhc_terrain_noise_spec
{
    f64 amp[TERRAIN_NOISE_COUNT];
    f64 freq[TERRAIN_NOISE_COUNT];
    f64 post_offset[TERRAIN_NOISE_COUNT];
    hhc_biome biome[BIOME_COUNT];
} hhc_terrain_noise_spec;

typedef struct hhc_terrain
{
    hhc_biome_index biome;
    enum block_id block_id;
    f64 value;
} hhc_terrain;

/*!
 *  @brief initialize default terrain settings and biome parameters.
 */
void terrain_init(void);

/*!
 *  @brief set internal parameter preferences for a given terrain noise type.
 */
void terrain_spec_set(hhc_terrain_noise_index noise_index, f64 amp, f64 freq, f64 post_offset);

/*!
 *  @brief update a single axis from each sample of each pre-defined 2D noise,
 *  for as many samples per noise as specified in `ctx`.
 *
 *  @param pos position along specified axis.
 */
chunk_work_cost sampler_noise_axis_update_2d(hhc_noise_sampler_context *ctx, u8 axis);

/*!
 *  @brief finalize and bake all pre-defined 2D and 3D noise maps to ready for
 *  terrain generation.
 *
 *  @remark use quintic interpolation internally.
 */
chunk_work_cost sampler_noise_bake(hhc_noise_sampler_context *ctx);

/*!
 *  @brief default terrain shape.
 *
 *  @remark terrain info (e.g., biome) is stored inside `ctx`.
 */
chunk_work_cost terrain_shape(hhc_terrain *terrain, hhc_noise_sampler_context *ctx);

/*!
 *  @brief compare likelihood of biomes `a` and `b` matching.
 *
 *  euclidean distance between relevant parameters is used.
 *
 *  @return biome score.
 */
f64 biome_score_get(hhc_biome a, hhc_biome b);

#endif /* HHC_TERRAIN_H */
