#include "noise.h"
#include "noise_sampler.h"

enum hhc_sampler_blend_type
{
    SAMPLER_BLEND_TYPE_NONE,
    SAMPLER_BLEND_TYPE_FACE,
    SAMPLER_BLEND_TYPE_EDGE,
    SAMPLER_BLEND_TYPE_CORNER
}; /* hhc_sampler_blend_type */

hhc_noise_sampler noise_sampler_init(f64 map_radius_x, f64 map_radius_y, f64 map_radius_z,
        f64 map_diameter_x, f64 map_diameter_y, f64 map_diameter_z,
        f64 map_margin_x, f64 map_margin_y, f64 map_margin_z)
{
    hhc_noise_sampler sampler = {0};

    sampler.radius[0] = map_radius_x;
    sampler.radius[1] = map_radius_y;
    sampler.radius[2] = map_radius_z;
    sampler.diameter[0] = map_diameter_x;
    sampler.diameter[1] = map_diameter_y;
    sampler.diameter[2] = map_diameter_z;
    sampler.margin[0] = map_margin_x;
    sampler.margin[1] = map_margin_y;
    sampler.margin[2] = map_margin_z;
    sampler.t_scale[0] = 1.0 / (map_margin_x * 2.0);
    sampler.t_scale[1] = 1.0 / (map_margin_y * 2.0);
    sampler.t_scale[2] = 1.0 / (map_margin_z * 2.0);

    return sampler;
}

void noise_sampler_context_init(hhc_noise_sampler *sampler,
        hhc_noise_sampler_context *context,
        f64 base_x, f64 base_y, f64 base_z)
{
    hhc_noise_sampler_context nocontext = {0};
    enum hhc_sampler_blend_type blend_type = 0;
    u8 blend_mask = 0;
    u32 i = 0;

    *context = nocontext;

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
            context->noise_sample_lerp_func = lerp_f64;
            break;

        case SAMPLER_BLEND_TYPE_EDGE:
            context->sample_count = 4;
            context->noise_sample_lerp_func = bilerp_f64;
            break;

        case SAMPLER_BLEND_TYPE_CORNER:
            context->sample_count = 8;
            context->noise_sample_lerp_func = trilerp_f64;
            break;

        default:
            context->sample_count = 1;
            context->noise_sample_lerp_func = nolerp_f64;
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

void sampler_axis_init(hhc_noise_sampler_context *context, u8 axis, f64 pos)
{
    context->pos_tab[0][axis] = pos + context->sample_offset[axis];
    context->pos_tab[1][axis] = context->pos_tab[0][axis] + context->diameter[axis];
}

void sampler_axis_pre_update(hhc_noise_sampler_context *context, u8 axis)
{
    f64 t = 0.0;

    if (context->axis_active[axis])
    {
        t = 0.5 - (context->radius[axis] - context->pos_tab[1][axis]) * context->t_scale[axis];
        t = t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
        context->t[context->blend_index[axis]] = t;
    }
}

void sampler_axis_post_update(hhc_noise_sampler_context *context, u8 axis)
{
    ++context->pos_tab[0][axis];
    ++context->pos_tab[1][axis];
}
