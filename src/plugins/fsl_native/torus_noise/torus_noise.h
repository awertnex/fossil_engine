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
 *  @file torus_noise.h
 *
 *  @brief Torus Noise plug-in header, for 'Fossil Engine'; seamless-tiling
 *  noise sampler, grid-based, 1D, 2D and 3D support, arbitrary blend margin
 *  thickness per axis and arbitrary final noise map size per axis.
 */

#ifndef FSL_TORUS_NOISE_H
#define FSL_TORUS_NOISE_H

#include "../../../common/api.h"
#include "../../../common/types.h"

#include "torus_sample.h"

typedef struct fsl_torus_buffer
{
    u64 noise_len;  /* number of noise maps per sampler context */
    u64 sample_len; /* number of samples per noise */

    /*!
     *  @brief sample base data per noise type.
     *
     *  @remark entry count = `noise_len` * `sample_len`.
     */

    fsl_torus_sample *sample_src_buf;
    /*!
     *  @brief final sample values per noise type, after gradient interpolation,
     *  before sample interpolation.
     *
     *  @remark entry count = `noise_len` * `sample_len`.
     */
    f64 *sample_dst_buf;

    /*!
     *  @brief final noise values, after sample interpolation.
     *
     *  @remark entry count = `noise_len`.
     */
    f64 *noise_dst_buf;
} fsl_torus_buffer;

/*!
 *  @brief base sampler parameters for noise sampling, persistent as long as map
 *  size doesn't change.
 */
typedef struct fsl_torus_sampler
{
    f64 radius[3];      /* map radius, in world-space */
    f64 diameter[3];    /* map diameter, in world-space */

    /*!
     *  @brief margin thickness before/after map-edge to trigger blend types,
     *  in world-space.
     */
    f64 margin[3];

    f64 t_scale[3];     /* blend factor scalar */

    /*!
     *  @brief buffers of noise data.
     */
    fsl_torus_buffer torus_buf;

    b8 initialized;
} fsl_torus_sampler;

/*!
 *  @brief fat sampler for noise sampling from 1/2/4/8 location(s) of a map.
 */
typedef struct fsl_torus_sampler_context
{
    /*!
     *  @brief current sampler being used for this context.
     */
    fsl_torus_sampler *sampler;

    f64 sample_offset[3];   /* sampler base offset, in world-space */
    f64 radius[3];          /* map radius, copied from @ref fsl_torus_sampler */
    f64 diameter[3];        /* map diameter, copied from @ref fsl_torus_sampler */
    f64 margin[3];          /* map margin, copied from @ref fsl_torus_sampler */

    /*!
     *  @brief overflow sign when sample is within specified margin.
     */
    i8 sign[3];

    /*!
     *  @brief map-sampling position table.
     *
     *  pos[0] for sample position,
     *  pos[1] for sample position beyond map edge, wrapped around by map diameter.
     */
    f64 pos_tab[2][3];

    /*!
     *  @brief map-sampling positions that point to `pos_tab`, for each sample.
     */
    f64 *pos[8][3];

    u8 blend_index[3];      /* axis selector for blend types (e.g., XY, YZ) */
    b8 axis_active[3];      /* axis participation in blending */
    f64 t[3];               /* blend factor for each axis */
    f64 t_scale[3];         /* blend factor scalar */

    /*!
     *  @brief sample indices to re-map sample source to destination in a @ref
     *  fsl_torus_buffer for proper interpolation based on blend-type.
     */
    u32 sample_index[8];

    /*!
     *  @brief number of samples to sample and interpolate.
     */
    u32 sample_count;

    fsl_torus_sample_lerp_func torus_sample_lerp_func;
} fsl_torus_sampler_context;

/*!
 *  @brief initialize torus sampler static parameters.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
FSLAPI u32 fsl_torus_sampler_init(fsl_torus_sampler *sampler,
        u64 noise_count, u64 sample_count,
        f64 map_radius_x, f64 map_radius_y, f64 map_radius_z,
        f64 map_diameter_x, f64 map_diameter_y, f64 map_diameter_z,
        f64 map_margin_x, f64 map_margin_y, f64 map_margin_z);

/*!
 *  @brief free torus sampler buffers.
 */
FSLAPI void fsl_torus_sampler_free(fsl_torus_sampler *sampler);

/*!
 *  @brief initialize torus sampler context for a given sampler.
 *
 *  1. initialize basic sampler parameters (sign, radius, diameter).
 *  2. initialize axis-dependent parameters based on provided position
 *     (blend_type, axis, pos).
 *  3. initialize scalar translator from map-size to normalized coordinates,
 *  used in blending before/after map-edge.
 *
 *  @remark initialization valid as long as `base_x`, `base_y` and `base_z` don't change,
 *  and valid while advancing @ref fsl_torus_sampler.pos_tab, otherwise, must
 *  re-initialize.
 */
FSLAPI void fsl_torus_sampler_context_init(fsl_torus_sampler *sampler,
        fsl_torus_sampler_context *context, f64 base_x, f64 base_y, f64 base_z);

/*!
 *  @brief initialize a single axis for a given torus sampler context.
 */
FSLAPI void fsl_torus_sampler_axis_init(fsl_torus_sampler_context *context, u8 axis, f64 pos);

/*!
 *  @brief iterate relevant parameters of a single axis for a given torus sampler
 *  context at the beginning of its loop.
 */
FSLAPI void fsl_torus_sampler_axis_pre_update(fsl_torus_sampler_context *context, u8 axis);

/*!
 *  @brief iterate relevant parameters of a single axis for a given torus sampler
 *  context at the end of its loop.
 */
FSLAPI void fsl_torus_sampler_axis_post_update(fsl_torus_sampler_context *context, u8 axis);

#endif /* FSL_TORUS_NOISE_H */
