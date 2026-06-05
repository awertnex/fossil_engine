#ifndef HHC_PERLIN_NOISE_H
#define HHC_PERLIN_NOISE_H

#include "deps/fossil/common/types.h"

#define RAND_TAB_DIAMETER   128
#define RAND_TAB_LAYER      (RAND_TAB_DIAMETER * RAND_TAB_DIAMETER)
#define RAND_TAB_VOLUME     (RAND_TAB_DIAMETER * RAND_TAB_DIAMETER * RAND_TAB_DIAMETER)

/*!
 *  @brief random number look-up table.
 *
 *  the values are extremely small so to directly use in perlin noise functions.
 *
 *  @remark read-only, initialized internally in @ref rand_init().
 */
extern f32 *RAND_TAB;

/*!
 *  @brief initialize and allocate resources for @ref RAND_TAB.
 *
 *  allocate resources for @ref RAND_TAB and load its look-up from disk if found
 *  and build if not found.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 rand_init(void);

void rand_free(void);
v3f32 random_2d(i32 x, i32 y, u64 seed);
v3f32 random_3d(i32 x, i32 y, i32 z, u64 seed);

/*!
 *  @brief get a gradient value between two 2d points.
 *
 *  get a random number from global array @ref RAND_TAB for each axis,
 *  index for each is seeded by `seed`, `x`, `y` and some magic constants.
 *
 *  @return the dot product of `v - a` and the sampled random vector.
 */
f32 gradient_2d(f32 vx, f32 vy, i32 x, i32 y, u64 seed);

/*!
 *  @brief get a gradient value between two 3d points.
 *
 *  get a random number from global array @ref RAND_TAB for each axis,
 *  index for each is seeded by `seed`, `x`, `y`, `z` and some magic constants.
 *
 *  @return the dot product of `v - a` and the sampled random vector.
 */
f32 gradient_3d(f32 vx, f32 vy, f32 vz, i32 x, i32 y, i32 z, u64 seed);

/*!
 *  @param coordinates current block to sample.
 *  @param intensity height, or contrast of the noise.
 *  @param scale frequency of the noise.
 *
 *  @return a value between [intensity / 2, -intensity / 2].
 */
f32 perlin_noise_2d(v2i32 coordinates, f32 intensity, f32 scale, u64 seed);

/*!
 *  @brief calls @ref perlin_noise_2d() for as many `octaves`.
 *
 *  @param coordinates current block to sample.
 *  @param intensity height, or contrast of the noise.
 *  @param scale frequency of the noise.
 *  @param octaves number of noise iterations.
 *  @param intensity_persistence scaling factor per iteration for `intensity`.
 *  @param scale_persistance scaling factor per iteration for `scale`.
 *
 *  @return a value between [intensity / 2, -intensity / 2].
 */
f32 perlin_noise_2d_ex(v2i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 intensity_persistence, f32 scale_persistence, u64 seed);

/*!
 *  @param coordinates current block to sample.
 *  @param intensity height, or contrast of the noise.
 *  @param scale frequency of the noise.
 *
 *  @return a value between [intensity / 2, -intensity / 2].
 */
f32 perlin_noise_3d(v3i32 coordinates, f32 intensity, f32 scale, u64 seed);

/*!
 *  @brief calls @ref perlin_noise_3d() for as many `octaves`.
 *
 *  @param coordinates current block to sample.
 *  @param intensity height, or contrast of the noise.
 *  @param scale frequency of the noise.
 *  @param octaves number of noise iterations.
 *  @param intensity_persistence scaling factor per iteration for `intensity`.
 *  @param scale_persistance scaling factor per iteration for `scale`.
 *
 *  @return a value between [intensity / 2, -intensity / 2].
 */
f32 perlin_noise_3d_ex(v3i32 coordinates, f32 intensity, f32 scale,
        u32 octaves, f32 intensity_persistence, f32 scale_persistence, u64 seed);

#endif /* HHC_PERLIN_NOISE_H */
