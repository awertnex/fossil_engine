#ifndef GAME_TERRAIN_H
#define GAME_TERRAIN_H

#include <engine/h/types.h>

#include "main.h"
#include "assets.h"

#define TERRAIN_SEED_DEFAULT 0

#define RAND_TAB_DIAMETER   128
#define RAND_TAB_LAYER      (RAND_TAB_DIAMETER * RAND_TAB_DIAMETER)
#define RAND_TAB_VOLUME     (RAND_TAB_DIAMETER * RAND_TAB_DIAMETER * RAND_TAB_DIAMETER)

enum Biome
{
    BIOME_HILLS,
    BIOME_SANDSTORM,
    BIOME_DECAYING_LANDS,
    BIOME_COUNT,
}; /* Biome */

typedef struct Terrain
{
    enum Biome biome;
    enum BlockID block_id;
    u32 block_light;
} Terrain;

/*! @brief random number look-up table.
 *
 *  the values are extremely small so to directly use in perlin noise functions.
 *
 *  @remark read-only, initialized internally in 'rand_init()'.
 */
extern f32 *RAND_TAB;

/*! @brief initialize and allocate resources for 'RAND_TAB'.
 *
 *  allocate resources for 'RAND_TAB' and load its look-up from disk if found
 *  and build if not found.
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 rand_init(void);

void rand_free(void);
v3f32 random_2d(i32 x, i32 y, u64 seed);
v3f32 random_3d(i32 x, i32 y, i32 z, u64 seed);

/*! @brief get a gradient value between two 2d points.
 *
 *  get a random number from global array 'RAND_TAB' for each axis,
 *  index for each is seeded by the trunc of 'ax', 'ay',
 *  some magic constants, plus 'dispx' and 'dispy'.
 *
 *  @param dispx, dispy = index displacement into 'RAND_TAB'.
 *
 *  @return the dot product between 'v - a' and the sampled random numbers vector.
 */
f32 gradient_2d(f32 vx, f32 vy, f32 ax, f32 ay, i32 dispx, i32 dispy);

/*! @brief get a gradient value between two 3d points.
 *
 *  get a random number from global array 'RAND_TAB' for each axis,
 *  index for each is seeded by the trunc of 'ax', 'ay', 'az',
 *  some magic constants, plus 'dispx', 'dispy' and 'dispz'.
 *
 *  @param dispx, dispy, dispz = index displacement into 'RAND_TAB'.
 *
 *  @return the dot product of the distance between 'v' and 'a' and the sampled
 *  random numbers.
 */
f32 gradient_3d(f32 vx, f32 vy, f32 vz, f32 ax, f32 ay, f32 az, i32 dispx, i32 dispy, i32 dispz);

/*! @param coordinates = current block to sample.
 *  @param intensity = height, or contrast of the noise.
 *  @param scale = frequency of the noise.
 *  @param dispx, dispy = displacement of random number selection index, on each axis.
 *
 *  @return a value between [frequency / 2, -frequency / 2].
 */
f32 perlin_noise_2d(v2i32 coordinates, f32 intensity, f32 scale, i32 dispx, i32 dispy);

/*! @brief calls 'perlin_noise_2d()' for as many 'octaves'.
 *
 *  @param coordinates = current block to sample.
 *  @param intensity = height, or contrast of the noise.
 *  @param scale = frequency of the noise.
 *  @param octaves = number of noise iterations.
 *  @param persistence = scaling factor per iteration for 'intensity'.
 *  @param gathering = scaling factor per iteration for 'scale'.
 *  @param dispx, dispy = displacement of random number selection index, on each axis.
 *
 *  @return a value between [frequency / 2, -frequency / 2].
 */
f32 perlin_noise_2d_ex(v2i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 persistence, f32 gathering, i32 dispx, i32 dispy);

/*! @param coordinates = current block to sample.
 *  @param intensity = height, or contrast of the noise.
 *  @param scale = frequency of the noise.
 *  @param dispx, dispy, dispz = displacement of random number selection index, on each axis.
 *
 *  @return a value between [frequency / 2, -frequency / 2].
 */
f32 perlin_noise_3d(v3i32 coordinates, f32 intensity, f32 scale, i32 dispx, i32 dispy, i32 dispz);

/*! @brief calls 'perlin_noise_3d()' for as many 'octaves'.
 *
 *  @param coordinates = current block to sample.
 *  @param intensity = height, or contrast of the noise.
 *  @param scale = frequency of the noise.
 *  @param octaves = number of noise iterations.
 *  @param persistence = scaling factor per iteration for 'intensity'.
 *  @param gathering = scaling factor per iteration for 'scale'.
 *  @param dispx, dispy, dispz = displacement of random number selection index, on each axis.
 *
 *  @return a value between [frequency / 2, -frequency / 2].
 */
f32 perlin_noise_3d_ex(v3i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 persistence, f32 gathering, i32 dispx, i32 dispy, i32 dispz);

/*! @brief default terrain of mountains valleys, caves and biomes.
 *
 *  @return terrain info (e.g. block ID at specified coordinates).
 */
Terrain terrain_land(v3i32 coordinates);

Terrain terrain_decaying_lands(v3i32 coordinates);

#endif /* GAME_TERRAIN_H */
