#ifndef HHC_NOISE_SAMPLER_H
#define HHC_NOISE_SAMPLER_H

#include "deps/fossil/common/types.h"

#include "noise.h"
#include "terrain_common.h"

/*!
 *  @brief base sampler parameters for noise sampling, persistent as long as map
 *  size doesn't change.
 */
typedef struct hhc_noise_sampler
{
    f64 radius[3];      /* map radius, in world-space */
    f64 diameter[3];    /* map diameter, in world-space */

    /*!
     *  @brief margin thickness before/after map-edge to trigger blend types,
     *  in world-space.
     */
    f64 margin[3];

    f64 t_scale[3];     /* blend factor scalar */
} hhc_noise_sampler;

/*!
 *  @brief fat sampler for noise sampling from 1/2/4/8 location(s) of a map.
 */
typedef struct hhc_noise_sampler_context
{
    f64 sample_offset[3];   /* sampler base offset, in world-space */
    f64 radius[3];          /* map radius, copied from @ref hhc_noise_sampler */
    f64 diameter[3];        /* map diameter, copied from @ref hhc_noise_sampler */
    f64 margin[3];          /* map margin, copied from @ref hhc_noise_sampler */

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
     *  @brief sample indices to re-map from `sample_buf` into `sample_value`
     *  for proper interpolation based on blend-type.
     */
    u32 sample_index[8];

    /*!
     *  @brief number of samples to sample and interpolate.
     */
    u32 sample_count;

    /*!
     *  @brief sample base data per noise type.
     */
    hhc_noise_sample sample_buf[TERRAIN_NOISE_COUNT][8];

    /*!
     *  @brief final sample values per noise type, after gradient interpolation,
     *  before sample interpolation.
     */
    f64 sample_value[TERRAIN_NOISE_COUNT][8];

    /*!
     *  @brief final noise values, after sample interpolation, ready for terrain generation.
     */
    f64 n[TERRAIN_NOISE_COUNT];

    hhc_noise_sample_lerp_func noise_sample_lerp_func;
} hhc_noise_sampler_context;

/*!
 *  @brief initialize noise sampler static parameters.
 */
hhc_noise_sampler noise_sampler_init(f64 map_radius_x, f64 map_radius_y, f64 map_radius_z,
        f64 map_diameter_x, f64 map_diameter_y, f64 map_diameter_z,
        f64 map_margin_x, f64 map_margin_y, f64 map_margin_z);

/*!
 *  @brief initialize noise sampler context for a given sampler.
 *
 *  1. initialize basic sampler parameters (sign, radius, diameter).
 *  2. initialize axis-dependent parameters based on provided position
 *     (blend_type, axis, pos).
 *  3. initialize scalar translator from map-size to normalized coordinates,
 *  used in blending before/after map-edge.
 *
 *  @remark initialization valid as long as `base_x`, `base_y` and `base_z` don't change,
 *  and valid while advancing @ref hhc_noise_sampler.pos_tab, otherwise, must
 *  re-initialize.
 */
void noise_sampler_context_init(hhc_noise_sampler *sampler,
        hhc_noise_sampler_context *context, f64 base_x, f64 base_y, f64 base_z);

/*!
 *  @brief initialize a single axis for a given noise sampler context.
 */
void sampler_axis_init(hhc_noise_sampler_context *context, u8 axis, f64 pos);

/*!
 *  @brief iterate relevant parameters of a single axis for a given noise sampler
 *  context at the beginning of its loop.
 */
void sampler_axis_pre_update(hhc_noise_sampler_context *context, u8 axis);

/*!
 *  @brief iterate relevant parameters of a single axis for a given noise sampler
 *  context at the end of its loop.
 */
void sampler_axis_post_update(hhc_noise_sampler_context *context, u8 axis);

#endif /* HHC_NOISE_SAMPLER_H */
