#include "deps/fossil/common/limits.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/plugins/fsl_native/noise_sampler/noise_sampler.h"
#include "deps/fossil/plugins/fsl_native/noise_sampler/noise_sampler_sample.h"

#include "../h/world.h"

#include "terrain.h"

#include <stdio.h>

#define TERRAIN_SEED_DEFAULT 0

/* terrain shape */
#define FREQ_CONTINENTAL    (1.0 / 1033.0)
#define FREQ_REGIONAL       (1.0 / 250.0)
#define FREQ_LOCAL          (1.0 / 109.0)
#define FREQ_DETAIL         (1.0 / 16.0)

/* terrain modifiers (and/or biome selection) */
#define FREQ_TEMPERATURE    (1.0 / 326.0)
#define FREQ_HUMIDITY       (1.0 / 726.0)
#define FREQ_EXTREMITY      (1.0 / 953.34)
#define FREQ_ROUGHNESS      (1.0 / 368.2)
#define FREQ_LIFE           (1.0 / 443.04)

static hhc_terrain_noise_spec terrain_spec = {0};

static void terrain_spec_biome_set(hhc_biome_index biome_index, str *name,
        f64 temperature, f64 humidity, f64 extremity, f64 roughness, f64 depth, f64 life);

f64 biome_score_get(hhc_biome a, hhc_biome b)
{
    i32 i = 0;
    f64 sum = 0.0;
    for (i = BIOME_NOISE_OFFSET; i < TERRAIN_NOISE_COUNT; ++i)
    {
        b.spec[i] -= a.spec[i];
        b.spec[i] *= b.spec[i];
        sum += b.spec[i];
    }
    return sum;
}

void terrain_init(void)
{
    f64 coef = 2.0 / 3.0;

    terrain_spec_set(TERRAIN_NOISE_CONTINENTAL, 1000.0, FREQ_CONTINENTAL, 0.0);
    terrain_spec_set(TERRAIN_NOISE_REGIONAL, 200.0, FREQ_REGIONAL, 0.0);
    terrain_spec_set(TERRAIN_NOISE_LOCAL, 50.0, FREQ_LOCAL, 0.0);
    terrain_spec_set(TERRAIN_NOISE_DETAIL, 5.0, FREQ_DETAIL, 0.0);
    terrain_spec_set(TERRAIN_NOISE_TEMPERATURE, coef, FREQ_TEMPERATURE, coef);
    terrain_spec_set(TERRAIN_NOISE_HUMIDITY, coef, FREQ_HUMIDITY, coef);
    terrain_spec_set(TERRAIN_NOISE_EXTREMITY, coef, FREQ_EXTREMITY, coef);
    terrain_spec_set(TERRAIN_NOISE_ROUGHNESS, coef, FREQ_ROUGHNESS, coef);
    terrain_spec_set(TERRAIN_NOISE_LIFE, coef, FREQ_LIFE, coef);

    terrain_spec_biome_set(BIOME_STONE, "Stone",
            0.350, 0.000, 0.334, 0.300, 0.000, 0.000);

    terrain_spec_biome_set(BIOME_HILLS, "Hills",
            0.390, 0.100, 0.600, 0.500, 70.000, 50.000);

    terrain_spec_biome_set(BIOME_SANDSTORM, "Sandstorm",
            0.557, 0.000, 0.100, 0.200, 0.000, 0.300);

    terrain_spec_biome_set(BIOME_DECAYING_LANDS, "Decaying Lands",
            0.163, 0.040, 0.900, 0.953, 0.000, 7.000);

    terrain_spec_biome_set(BIOME_JUNGLE, "Jungle",
            0.270, 0.290, 0.430, 0.780, 3.000, 260.000);
}

void terrain_spec_set(hhc_terrain_noise_index noise_index, f64 amp, f64 freq, f64 post_offset)
{
    terrain_spec.amp[noise_index] = amp;
    terrain_spec.freq[noise_index] = freq;
    terrain_spec.post_offset[noise_index] = post_offset;
}

static void terrain_spec_biome_set(hhc_biome_index biome_index, str *name,
        f64 temperature, f64 humidity, f64 extremity, f64 roughness, f64 depth, f64 life)
{
    static u32 seed = 4911577 + TERRAIN_SEED_DEFAULT;
    f64 sum = 0.0;
    u32 i = 0;

    snprintf(terrain_spec.biome[biome_index].name, FSL_ID_CAP, "%s", name);

    for (i = BIOME_NOISE_OFFSET; i < TERRAIN_NOISE_COUNT; ++i)
    {
        terrain_spec.biome[biome_index].spec[i] = (f64)fsl_rand_u32(seed++) / (f64)FSL_U32_MAX;
        sum += terrain_spec.biome[biome_index].spec[i];
    }

    for (i = BIOME_NOISE_OFFSET; i < TERRAIN_NOISE_COUNT; ++i)
        terrain_spec.biome[biome_index].spec[i] /= sum;
}

chunk_work_cost sampler_noise_axis_update_2d(fsl_noise_sampler_context *ctx, u8 axis)
{
    u32 i = 0;
    u32 j = 0;
    u64 noise_count = ctx->sampler->noise_buf.noise_len;
    u32 sample_count = ctx->sample_count;
    fsl_noise_sample *sample_src_buf = NULL;

    for (i = 0; i < noise_count; ++i)
    {
        sample_src_buf = &ctx->sampler->noise_buf.sample_src_buf[i * sample_count];
        for (j = 0; j < sample_count; ++j)
        {
            fsl_noise_sample_axis_init(&sample_src_buf[j], axis,
                    *ctx->pos[j][axis], terrain_spec.freq[i]);
        }
    }

    return CHUNK_WORK_COST_GENERATE_NOISE_INIT * i * j;
}

chunk_work_cost sampler_noise_bake(fsl_noise_sampler_context *ctx)
{
    u64 i = 0;
    u64 j = 0;
    fsl_noise_buffer *noise_buf = &ctx->sampler->noise_buf;
    u64 noise_count = noise_buf->noise_len;
    u64 sample_count = ctx->sample_count;
    fsl_noise_sample *sample_src_buf = NULL;
    f64 *sample_dst_buf = NULL;
    f64 *noise_dst_buf = noise_buf->noise_dst_buf;
    f64 *t = ctx->t;

    for (i = 0; i < noise_count; ++i)
    {
        sample_src_buf = &noise_buf->sample_src_buf[i * sample_count];
        sample_dst_buf = &noise_buf->sample_dst_buf[i * sample_count];
        for (j = 0; j < sample_count; ++j)
        {
            sample_dst_buf[j] = fsl_noise_sample_make_3d(&sample_src_buf[j], terrain_spec.amp[i],
                    world.seed + TERRAIN_SEED_DEFAULT + i * 10);
        }

        noise_dst_buf[i] = ctx->noise_sample_lerp_func(sample_dst_buf, t);
    }

    for (i = BIOME_NOISE_OFFSET; i < noise_count; ++i)
    {
        noise_dst_buf[i] = fsl_clamp_f64(noise_dst_buf[i] +
                terrain_spec.post_offset[i], 0.0, 1.0);
    }

    return CHUNK_WORK_COST_GENERATE_NOISE_INTERPOLATE_2D * ctx->sample_count;
}

chunk_work_cost terrain_shape(hhc_terrain *terrain, fsl_noise_sampler_context *ctx)
{
    chunk_work_cost cost = 0;
    f64 *noise_dst_buf = ctx->sampler->noise_buf.noise_dst_buf;
    hhc_terrain noterrain = {0};
    hhc_biome biome = {0};
    f64 biome_score = 0.0;
    f64 biome_best_score = 10000.0;
    i32 biome_best_index = 0;
    i32 i = 0;

    *terrain = noterrain;

    for (i = BIOME_NOISE_OFFSET; i < TERRAIN_NOISE_COUNT; ++i)
    biome.spec[i] = noise_dst_buf[i];

    i = BIOME_COUNT;
    while (i--)
    {
        biome_score = biome_score_get(biome, terrain_spec.biome[i]);
        if (biome_score < biome_best_score)
        {
            biome_best_score = biome_score;
            biome_best_index = i;
        }
    }

    terrain->biome = biome_best_index;
    terrain->value =
        ((noise_dst_buf[0] + noise_dst_buf[1]) * (1.0 - noise_dst_buf[4]) +
        noise_dst_buf[2]) * noise_dst_buf[6] +
        noise_dst_buf[3] * noise_dst_buf[7];

    if (noise_dst_buf[3] > 0.0)
    {
        terrain->block_id = BLOCK_GRASS;
    }

    return cost;
}
