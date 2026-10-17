#include "deps/fossil/common/api.h" /* for _GNU_SOURCE */
#include "deps/fossil/math/math.h"

#include "biome.h"

#include <stdio.h>

#define BIOME_NOISE_TEMPERATURE_MAX     513.0
#define BIOME_NOISE_TEMPERATURE_MIN     -273.15

#define BIOME_NOISE_HUMIDITY_MAX        50.0
#define BIOME_NOISE_HUMIDITY_MIN        0.0

#define BIOME_NOISE_EXTREMITY_MAX       1.0
#define BIOME_NOISE_EXTREMITY_MIN       0.0

#define BIOME_NOISE_ROUGHNESS_MAX       1.0
#define BIOME_NOISE_ROUGHNESS_MIN       0.0

#define BIOME_NOISE_LIFE_MAX            140.0
#define BIOME_NOISE_LIFE_MIN            2.0

hhc_biome biome_init(str *name,
        f64 temperature, f64 humidity, f64 extremity, f64 roughness, f64 depth, f64 life)
{
    hhc_biome biome = {0};

    snprintf(biome.name, FSL_ID_CAP, "%s", name);

    biome.spec[BIOME_NOISE_TEMPERATURE] = biome_param_set(temperature,
            BIOME_NOISE_TEMPERATURE_MIN, BIOME_NOISE_TEMPERATURE_MAX);

    biome.spec[BIOME_NOISE_HUMIDITY] = biome_param_set(humidity,
            BIOME_NOISE_HUMIDITY_MIN, BIOME_NOISE_HUMIDITY_MAX);

    biome.spec[BIOME_NOISE_EXTREMITY] = biome_param_set(extremity,
            BIOME_NOISE_EXTREMITY_MIN, BIOME_NOISE_EXTREMITY_MAX);

    biome.spec[BIOME_NOISE_ROUGHNESS] = biome_param_set(roughness,
            BIOME_NOISE_ROUGHNESS_MIN, BIOME_NOISE_ROUGHNESS_MAX);

    biome.spec[BIOME_NOISE_DEPTH] = depth;

    biome.spec[BIOME_NOISE_LIFE] = biome_param_set(life,
            BIOME_NOISE_LIFE_MIN, BIOME_NOISE_LIFE_MAX);

    return biome;
}

f64 biome_param_set(f64 param, f64 min, f64 max)
{
    param = fsl_clamp_f64(param, min, max);
    return fsl_map_range_f64(param, min, max, 0.0, 1.0);
}

f64 biome_score_get(hhc_biome a, hhc_biome b)
{
    u32 i = 0;
    f64 sum = 0.0;
    for (i = 0; i < BIOME_NOISE_COUNT; ++i)
    {
        b.spec[i] -= a.spec[i];
        sum += b.spec[i] * b.spec[i];
    }
    return sum;
}
