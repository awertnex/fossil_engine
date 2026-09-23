#ifndef HHC_BIOME_H
#define HHC_BIOME_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/common/limits.h"

typedef enum hhc_biome_noise_index
{
    BIOME_NOISE_TEMPERATURE,
    BIOME_NOISE_HUMIDITY,
    BIOME_NOISE_EXTREMITY, /* big detail height */
    BIOME_NOISE_ROUGHNESS, /* small detail height */
    BIOME_NOISE_DEPTH,
    BIOME_NOISE_LIFE,
    BIOME_NOISE_COUNT
} hhc_biome_noise_index;

typedef enum hhc_biome_index
{
    BIOME_STONE,
    BIOME_HILLS,
    BIOME_SANDSTORM,
    BIOME_DECAYING_LANDS,
    BIOME_JUNGLE,
    BIOME_COUNT
} hhc_biome_index;

typedef struct hhc_biome
{
    str name[FSL_ID_CAP];
    f64 spec[BIOME_NOISE_COUNT]; /* preferred value for each noise */
} hhc_biome;

/*!
 *  @brief initialize a biome.
 */
hhc_biome biome_init(str *name, f64 temperature, f64 humidity, f64 extremity,
        f64 roughness, f64 depth, f64 life);

/*!
 *  @brief initialize a single parameter for a biome.
 */
f64 biome_param_set(f64 param, f64 min, f64 max);

/*!
 *  @brief compare likelihood of biomes `a` and `b` matching.
 *
 *  euclidean distance between relevant parameters is used.
 *
 *  @return biome score.
 */
f64 biome_score_get(hhc_biome a, hhc_biome b);

#endif /* HHC_BIOME_H */
