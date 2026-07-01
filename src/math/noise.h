/*!
 *  Copyright 2026 Lily Awertnex
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*!
 *  @file noise.h
 *
 *  @brief noise and gradient functions.
 */

#ifndef FSL_MATH_NOISE_H
#define FSL_MATH_NOISE_H

#include "../common/api.h"
#include "../common/types.h"

#include "math.h"

#define FSL_RAND_TAB_DIAMETER   48
#define FSL_RAND_TAB_LAYER      (FSL_RAND_TAB_DIAMETER * FSL_RAND_TAB_DIAMETER)
#define FSL_RAND_TAB_VOLUME     (FSL_RAND_TAB_LAYER * FSL_RAND_TAB_DIAMETER)

/*!
 *  @brief can be used to shrink down big values when passed into trigonometrical
 *  functions, especially when generating random floats, it decreases workload.
 */
#define FSL_RAND_SCALE  (FSL_PI / ~(~0u >> 1))

/*!
 *  @brief random number look-up table.
 *
 *  the values are extremely small so to directly use in noise functions.
 *
 *  @remark read-only, initialized internally in @ref fsl_noise_init().
 */
FSLAPI extern f32 *fsl_rand_tab;

/*!
 *  @brief get a gradient value between two 1D points.
 *
 *  get a random number from global array @ref fsl_rand_tab for `x`,
 *  index is seeded by `seed`, `x` and some magic constant.
 *
 *  @return the dot product of `v - a` and the sampled random point.
 */
FSLAPI f32 fsl_gradient_1d(f32 v, i32 x, u64 seed);

/*!
 *  @brief get a gradient value between two 2D points.
 *
 *  get a random number from global array @ref fsl_rand_tab for each axis,
 *  index for each is seeded by `seed`, `x`, `y` and some magic constants.
 *
 *  @return the dot product of `v - a` and the sampled random vector.
 */
FSLAPI f32 fsl_gradient_2d(f32 vx, f32 vy, i32 x, i32 y, u64 seed);

/*!
 *  @brief get a gradient value between two 3D points.
 *
 *  get a random number from global array @ref fsl_rand_tab for each axis,
 *  index for each is seeded by `seed`, `x`, `y`, `z` and some magic constants.
 *
 *  @return the dot product of `v - a` and the sampled random vector.
 */
FSLAPI f32 fsl_gradient_3d(f32 vx, f32 vy, f32 vz, i32 x, i32 y, i32 z, u64 seed);

/*!
 *  @brief get a perlin-noise sample at specified point.
 *
 *  @param x point in 1D space to sample at.
 *  @param amplitude height, or contrast of the noise.
 *  @param frequency frequency of the noise.
 *
 *  @remark use quintic interpolation.
 *
 *  @return a value between [amplitude / 2, -amplitude / 2].
 */
FSLAPI f32 fsl_perlin_noise_1d(f32 x, f32 amplitude, f32 frequency, u64 seed);

/*!
 *  @brief get a perlin-noise sample at specified point.
 *
 *  @param x x coordinate of point in 2D space to sample at.
 *  @param y y coordinate of point in 2D space to sample at.
 *  @param amplitude height, or contrast of the noise.
 *  @param frequency frequency of the noise.
 *
 *  @remark use quintic interpolation.
 *
 *  @return a value between [amplitude / 2, -amplitude / 2].
 */
FSLAPI f32 fsl_perlin_noise_2d(f32 x, f32 y, f32 amplitude, f32 frequency, u64 seed);

/*!
 *  @brief get a perlin-noise sample at specified point.
 *
 *  @param x x coordinate of point in 3D space to sample at.
 *  @param y y coordinate of point in 3D space to sample at.
 *  @param z z coordinate of point in 3D space to sample at.
 *  @param amplitude height, or contrast of the noise.
 *  @param frequency frequency of the noise.
 *
 *  @remark use quintic interpolation.
 *
 *  @return a value between [amplitude / 2, -amplitude / 2].
 */
FSLAPI f32 fsl_perlin_noise_3d(f32 x, f32 y, f32 z, f32 amplitude, f32 frequency, u64 seed);

/*!
 *  @brief get a perlin-noise sample at specified point.
 *
 *  @param x point in 1D space to sample at.
 *  @param amplitude height, or contrast of the noise.
 *  @param frequency frequency of the noise.
 *  @param octaves number of noise iterations.
 *  @param amplitude_persistence scaling factor each iteration for `amplitude`.
 *  @param frequency_persistance scaling factor each iteration for `frequency`.
 *
 *  @remark call @ref perlin_noise_1d() for as many `octaves`.
 *
 *  @return a value between [amplitude / 2, -amplitude / 2].
 */
FSLAPI f32 fsl_perlin_noise_1d_ex(f32 x, f32 amplitude, f32 frequency,
        u32 octaves, f32 amplitude_persistence, f32 frequency_persistence, u64 seed);

/*!
 *  @brief get a perlin-noise sample at specified point.
 *
 *  @param x x coordinate of point in 2D space to sample at.
 *  @param y y coordinate of point in 2D space to sample at.
 *  @param amplitude height, or contrast of the noise.
 *  @param frequency frequency of the noise.
 *  @param octaves number of noise iterations.
 *  @param amplitude_persistence scaling factor each iteration for `amplitude`.
 *  @param frequency_persistance scaling factor each iteration for `frequency`.
 *
 *  @remark call @ref perlin_noise_2d() for as many `octaves`.
 *
 *  @return a value between [amplitude / 2, -amplitude / 2].
 */
FSLAPI f32 fsl_perlin_noise_2d_ex(f32 x, f32 y, f32 amplitude, f32 frequency,
        u32 octaves, f32 amplitude_persistence, f32 frequency_persistence, u64 seed);

/*!
 *  @brief get a perlin-noise sample at specified point.
 *
 *  @param x x coordinate of point in 3D space to sample at.
 *  @param y y coordinate of point in 3D space to sample at.
 *  @param z z coordinate of point in 3D space to sample at.
 *  @param amplitude height, or contrast of the noise.
 *  @param frequency frequency of the noise.
 *  @param octaves number of noise iterations.
 *  @param amplitude_persistence scaling factor each iteration for `amplitude`.
 *  @param frequency_persistance scaling factor each iteration for `frequency`.
 *
 *  @remark call @ref perlin_noise_3d() for as many `octaves`.
 *
 *  @return a value between [amplitude / 2, -amplitude / 2].
 */
FSLAPI f32 fsl_perlin_noise_3d_ex(f32 x, f32 y, f32 z, f32 amplitude, f32 frequency,
        u32 octaves, f32 amplitude_persistence, f32 frequency_persistence, u64 seed);

#endif /* FSL_MATH_NOISE_H */
