#ifndef HHC_TERRAIN_COMMON_H
#define HHC_TERRAIN_COMMON_H

typedef enum hhc_terrain_noise_index
{
    TERRAIN_NOISE_CONTINENTAL,
    TERRAIN_NOISE_REGIONAL,
    TERRAIN_NOISE_LOCAL,
    TERRAIN_NOISE_DETAIL,
    TERRAIN_NOISE_TEMPERATURE,
    TERRAIN_NOISE_HUMIDITY,
    TERRAIN_NOISE_EXTREMITY, /* big detail height */
    TERRAIN_NOISE_ROUGHNESS, /* small detail height */
    TERRAIN_NOISE_LIFE,
    TERRAIN_NOISE_COUNT
} hhc_terrain_noise_index;

typedef enum hhc_biome_index
{
    BIOME_STONE,
    BIOME_HILLS,
    BIOME_SANDSTORM,
    BIOME_DECAYING_LANDS,
    BIOME_JUNGLE,
    BIOME_COUNT
} hhc_biome_index;

#endif /* HHC_TERRAIN_COMMON_H */
