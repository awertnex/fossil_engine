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
 *  @file torus_noise.c
 *
 *  @brief Torus Noise plug-in implementation, for 'Fossil Engine'; seamless-tiling
 *  noise sampler, grid-based, 1D, 2D and 3D support, arbitrary blend margin
 *  thickness per axis and arbitrary final noise map size per axis.
 */

#include "../../../logger/logger.h"
#include "../../../logger/logger_messages_internal.h"
#include "../../../memory/memory.h"

#include "../../../common/diagnostics.h"

#include "torus_noise.h"

#include <stddef.h>
#include <inttypes.h>

enum fsl_sampler_blend_type
{
    SAMPLER_BLEND_TYPE_NONE,
    SAMPLER_BLEND_TYPE_FACE,
    SAMPLER_BLEND_TYPE_EDGE,
    SAMPLER_BLEND_TYPE_CORNER
}; /* fsl_sampler_blend_type */

u32 fsl_torus_sampler_init(fsl_torus_sampler *sampler,
        u64 noise_count, u64 sample_count,
        f64 map_radius_x, f64 map_radius_y, f64 map_radius_z,
        f64 map_diameter_x, f64 map_diameter_y, f64 map_diameter_z,
        f64 map_margin_x, f64 map_margin_y, f64 map_margin_z)
{
    fsl_torus_buffer *torus_buf = NULL;

    if (!sampler)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_POINTER_NULL_ACTION("Initialize Noise Sampler"));
        return fsl_err;
    }

    if (sampler->initialized)
        fsl_torus_sampler_free(sampler);

    torus_buf = &sampler->torus_buf;

    if (fsl_mem_map((void*)&torus_buf->sample_src_buf,
            noise_count * sample_count * sizeof(fsl_torus_sample),
            "torus_sampler_init().torus_buf->sample_src_buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_mem_map((void*)&torus_buf->sample_dst_buf,
            noise_count * sample_count * sizeof(f64),
            "torus_sampler_init().torus_buf->sample_dst_buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_mem_map((void*)&torus_buf->noise_dst_buf,
                noise_count * sizeof(f64),
            "torus_sampler_init().torus_buf->noise_dst_buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    sampler->radius[0] = map_radius_x;
    sampler->radius[1] = map_radius_y;
    sampler->radius[2] = map_radius_z;
    sampler->diameter[0] = map_diameter_x;
    sampler->diameter[1] = map_diameter_y;
    sampler->diameter[2] = map_diameter_z;
    sampler->margin[0] = map_margin_x;
    sampler->margin[1] = map_margin_y;
    sampler->margin[2] = map_margin_z;
    sampler->t_scale[0] = 1.0 / (map_margin_x * 2.0);
    sampler->t_scale[1] = 1.0 / (map_margin_y * 2.0);
    sampler->t_scale[2] = 1.0 / (map_margin_z * 2.0);

    sampler->torus_buf.noise_len = noise_count;
    sampler->torus_buf.sample_len = sample_count;

    sampler->initialized = TRUE;
    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("Sampler Initialized [noise_count: %"PRIu64"][sample_count: %"PRIu64"]\n", noise_count, sample_count));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_torus_sampler_free(sampler);
    return fsl_err;
}

void fsl_torus_sampler_free(fsl_torus_sampler *sampler)
{
    fsl_torus_sampler nosampler = {0};
    fsl_torus_buffer *torus_buf = NULL;

    if (!sampler || !sampler->initialized)
        return;

    sampler->initialized = FALSE;
    torus_buf = &sampler->torus_buf;

    fsl_mem_unmap((void*)&torus_buf->sample_src_buf,
            torus_buf->noise_len * torus_buf->sample_len * sizeof(fsl_torus_sample),
            "torus_sampler_free().torus_buf->sample_src_buf");
    fsl_mem_unmap((void*)&torus_buf->sample_dst_buf,
            torus_buf->noise_len * torus_buf->sample_len * sizeof(f64),
            "torus_sampler_free().torus_buf->sample_dst_buf");
    fsl_mem_unmap((void*)&torus_buf->noise_dst_buf,
            torus_buf->noise_len * sizeof(f64),
            "torus_sampler_free().torus_buf->noise_dst_buf");

    *sampler = nosampler;
}

void fsl_torus_sampler_context_init(fsl_torus_sampler *sampler,
        fsl_torus_sampler_context *context,
        f64 base_x, f64 base_y, f64 base_z)
{
    fsl_torus_sampler_context nocontext = {0};
    enum fsl_sampler_blend_type blend_type = 0;
    u8 blend_mask = 0;
    u32 i = 0;

    *context = nocontext;

    context->sampler = sampler;
    context->sample_offset[0] = base_x;
    context->sample_offset[1] = base_y;
    context->sample_offset[2] = base_z;
    context->margin[0] = sampler->margin[0];
    context->margin[1] = sampler->margin[1];
    context->margin[2] = sampler->margin[2];

    if (base_x >= sampler->radius[0] - sampler->margin[0])
        context->sign[0] = 1;
    else if (base_x < -sampler->radius[0] + sampler->margin[0])
        context->sign[0] = -1;

    if (base_y >= sampler->radius[1] - sampler->margin[1])
        context->sign[1] = 1;
    else if (base_y < -sampler->radius[1] + sampler->margin[1])
        context->sign[1] = -1;

    if (base_z >= sampler->radius[2] - sampler->margin[2])
        context->sign[2] = 1;
    else if (base_z < -sampler->radius[2] + sampler->margin[2])
        context->sign[2] = -1;

    context->radius[0] = sampler->radius[0] * -context->sign[0];
    context->radius[1] = sampler->radius[1] * -context->sign[1];
    context->radius[2] = sampler->radius[2] * -context->sign[2];
    context->diameter[0] = sampler->diameter[0] * -context->sign[0];
    context->diameter[1] = sampler->diameter[1] * -context->sign[1];
    context->diameter[2] = sampler->diameter[2] * -context->sign[2];
    context->t_scale[0] = sampler->t_scale[0] * context->sign[0];
    context->t_scale[1] = sampler->t_scale[1] * context->sign[1];
    context->t_scale[2] = sampler->t_scale[2] * context->sign[2];
    context->axis_active[0] = context->sign[0] != 0;
    context->axis_active[1] = context->sign[1] != 0;
    context->axis_active[2] = context->sign[2] != 0;
    blend_type =
        context->axis_active[0] +
        context->axis_active[1] +
        context->axis_active[2];
    blend_mask =
        (context->axis_active[0] << 0) |
        (context->axis_active[1] << 1) |
        (context->axis_active[2] << 2);

    switch (blend_type)
    {
        case SAMPLER_BLEND_TYPE_FACE:
            context->sample_count = 2;
            context->torus_sample_lerp_func = fsl_torus_sample_lerp;
            break;

        case SAMPLER_BLEND_TYPE_EDGE:
            context->sample_count = 4;
            context->torus_sample_lerp_func = fsl_torus_sample_bilerp;
            break;

        case SAMPLER_BLEND_TYPE_CORNER:
            context->sample_count = 8;
            context->torus_sample_lerp_func = fsl_torus_sample_trilerp;
            break;

        default:
            context->sample_count = 1;
            context->torus_sample_lerp_func = fsl_torus_sample_nolerp;
            break;
    }

    switch (blend_mask)
    {
        case 0: /* none */
            break;

        case 1: /* x */
            context->blend_index[0] = 0;
            context->sample_index[0] = 0;
            context->sample_index[1] = 1;
            break;

        case 2: /* y */
            context->blend_index[1] = 0;
            context->sample_index[0] = 0;
            context->sample_index[1] = 2;
            break;

        case 3: /* xy */
            context->blend_index[0] = 0;
            context->blend_index[1] = 1;
            context->sample_index[0] = 0;
            context->sample_index[1] = 1;
            context->sample_index[2] = 2;
            context->sample_index[3] = 3;
            break;

        case 4: /* z */
            context->blend_index[2] = 0;
            context->sample_index[0] = 0;
            context->sample_index[1] = 4;
            break;

        case 5: /* xz */
            context->blend_index[0] = 0;
            context->blend_index[2] = 1;
            context->sample_index[0] = 0;
            context->sample_index[1] = 1;
            context->sample_index[2] = 4;
            context->sample_index[3] = 5;
            break;

        case 6: /* yz */
            context->blend_index[1] = 0;
            context->blend_index[2] = 1;
            context->sample_index[0] = 0;
            context->sample_index[1] = 2;
            context->sample_index[2] = 4;
            context->sample_index[3] = 6;
            break;

        case 7: /* xyz */
            context->blend_index[0] = 0;
            context->blend_index[1] = 1;
            context->blend_index[2] = 2;
            context->sample_index[0] = 0;
            context->sample_index[1] = 1;
            context->sample_index[2] = 2;
            context->sample_index[3] = 3;
            context->sample_index[4] = 4;
            context->sample_index[5] = 5;
            context->sample_index[6] = 6;
            context->sample_index[7] = 7;
            break;
    }

    for (i = 0; i < context->sample_count; ++i)
    {
        context->pos[i][0] = &context->pos_tab[(context->sample_index[i] >> 0) & 1][0];
        context->pos[i][1] = &context->pos_tab[(context->sample_index[i] >> 1) & 1][1];
        context->pos[i][2] = &context->pos_tab[(context->sample_index[i] >> 2) & 1][2];
    }
}

void fsl_torus_sampler_axis_init(fsl_torus_sampler_context *context, u8 axis, f64 pos)
{
    context->pos_tab[0][axis] = pos + context->sample_offset[axis];
    context->pos_tab[1][axis] = context->pos_tab[0][axis] + context->diameter[axis];
}

void fsl_torus_sampler_axis_pre_update(fsl_torus_sampler_context *context, u8 axis)
{
    f64 t = 0.0;

    if (context->axis_active[axis])
    {
        t = 0.5 - (context->radius[axis] - context->pos_tab[1][axis]) * context->t_scale[axis];
        t = t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
        context->t[context->blend_index[axis]] = t;
    }
}

void fsl_torus_sampler_axis_post_update(fsl_torus_sampler_context *context, u8 axis)
{
    ++context->pos_tab[0][axis];
    ++context->pos_tab[1][axis];
}
