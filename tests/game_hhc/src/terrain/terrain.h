#ifndef HHC_TERRAIN_H
#define HHC_TERRAIN_H

#include "deps/fossil/common/types.h"

#include "../chunking/chunk_scheduler.h"

enum biome_index
{
    BIOME_HILLS,
    BIOME_SANDSTORM,
    BIOME_DECAYING_LANDS,
    BIOME_COUNT
}; /* biome_index */

struct biome
{
    f32 temperature;
    f32 humidity;
    f32 life;
}; /* biome */

typedef struct terrain
{
    enum biome_index biome;
    enum block_id block_id;
    u32 block_light;

    /*!
     *  @brief cost of terrain generation done on this block.
     */
    chunk_work_cost cost;
} terrain;

/*!
 *  @brief default terrain of mountains valleys, caves and biomes.
 *
 *  @return terrain info (e.g., block ID at specified coordinates).
 */
terrain terrain_land(v3i32 coordinates);

terrain terrain_decaying_lands(v3i32 coordinates);
terrain terrain_biome_blend_test(v3i32 coordinates);

#endif /* HHC_TERRAIN_H */
