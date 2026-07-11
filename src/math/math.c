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
 *  @file math.c
 *
 *  @brief math stuff.
 */

#include "../common/config.h"
#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../memory/memory.h"

#include "../h/dir.h"

#include "math.h"
#include "math_implementation.h"
#include "math_internal.h"
#include "matrix.h"
#include "noise.h"
#include "trigonometry.h"
#include "vector.h"

#include <stdio.h>
#include <math.h>

f32 *fsl_rand_tab = NULL;

u32 noise_init_internal(void)
{
    str path[FSL_PATH_CAP] = {0};
    f32 *file_contents = NULL;
    u64 file_len = 0;
    u32 i = 0;

    if (fsl_mem_map((void*)&fsl_rand_tab, FSL_RAND_TAB_VOLUME * sizeof(f32),
                "noise_init_internal().fsl_rand_tab") != FSL_ERR_SUCCESS)
        goto cleanup;

    snprintf(path, FSL_PATH_CAP, "%s%s", FSL_DIR_NAME_LOOKUPS, FSL_FILE_NAME_LOOKUP_RAND_TAB);

    if (fsl_is_file_exists(path, FALSE) == FSL_ERR_SUCCESS)
    {
        file_len = fsl_get_file_contents(path, (void*)&file_contents, FALSE);
        if (fsl_err != FSL_ERR_SUCCESS || file_contents == NULL)
            goto cleanup;

        for (i = 0; i < FSL_RAND_TAB_VOLUME; ++i)
            fsl_rand_tab[i] = file_contents[i];

        fsl_mem_free((void*)&file_contents, file_len,
                "noise_init_internal().file_contents");
    }
    else
    {
        for (i = 0; i < FSL_RAND_TAB_VOLUME; ++i)
            fsl_rand_tab[i] = sin((f64)(fsl_rand_u64(i) % 360));

        if (fsl_write_file(path, FSL_RAND_TAB_VOLUME * sizeof(f32),
                    fsl_rand_tab, TRUE, FALSE) != FSL_ERR_SUCCESS)
            goto cleanup;
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    noise_free_internal();
    return fsl_err;
}

void noise_free_internal(void)
{
    fsl_mem_unmap((void*)&fsl_rand_tab, FSL_RAND_TAB_VOLUME * sizeof(f32),
            "noise_free_internal().fsl_rand_tab");
}

/* ---- section: scalar ----------------------------------------------------- */

u8 fsl_clamp_u8(u8 n, u8 min, u8 max) CLAMP_FUNC_IMPL
i8 fsl_clamp_i8(i8 n, i8 min, i8 max) CLAMP_FUNC_IMPL
u16 fsl_clamp_u16(u16 n, u16 min, u16 max) CLAMP_FUNC_IMPL
i16 fsl_clamp_i16(i16 n, i16 min, i16 max) CLAMP_FUNC_IMPL
u32 fsl_clamp_u32(u32 n, u32 min, u32 max) CLAMP_FUNC_IMPL
i32 fsl_clamp_i32(i32 n, i32 min, i32 max) CLAMP_FUNC_IMPL
f32 fsl_clamp_f32(f32 n, f32 min, f32 max) CLAMP_FUNC_IMPL
u64 fsl_clamp_u64(u64 n, u64 min, u64 max) CLAMP_FUNC_IMPL
i64 fsl_clamp_i64(i64 n, i64 min, i64 max) CLAMP_FUNC_IMPL
f64 fsl_clamp_f64(f64 n, f64 min, f64 max) CLAMP_FUNC_IMPL

u8 fsl_mod_u8(u8 n, u8 max) MOD_FUNC_IMPL
i8 fsl_mod_i8(i8 n, i8 max) MOD_FUNC_IMPL
u16 fsl_mod_u16(u16 n, u16 max) MOD_FUNC_IMPL
i16 fsl_mod_i16(i16 n, i16 max) MOD_FUNC_IMPL
u32 fsl_mod_u32(u32 n, u32 max) MOD_FUNC_IMPL
i32 fsl_mod_i32(i32 n, i32 max) MOD_FUNC_IMPL
u64 fsl_mod_u64(u64 n, u64 max) MOD_FUNC_IMPL
i64 fsl_mod_i64(i64 n, i64 max) MOD_FUNC_IMPL

u8 fsl_round_up_u8(u8 n, u8 max) ROUND_UP_FUNC_IMPL
i8 fsl_round_up_i8(i8 n, i8 max) ROUND_UP_FUNC_IMPL
u16 fsl_round_up_u16(u16 n, u16 max) ROUND_UP_FUNC_IMPL
i16 fsl_round_up_i16(i16 n, i16 max) ROUND_UP_FUNC_IMPL
u32 fsl_round_up_u32(u32 n, u32 max) ROUND_UP_FUNC_IMPL
i32 fsl_round_up_i32(i32 n, i32 max) ROUND_UP_FUNC_IMPL
u64 fsl_round_up_u64(u64 n, u64 max) ROUND_UP_FUNC_IMPL
i64 fsl_round_up_i64(i64 n, i64 max) ROUND_UP_FUNC_IMPL

u8 fsl_map_range_u8(u8 n, u8 n_min, u8 n_max, u8 r_min, u8 r_max) MAP_RANGE_FUNC_IMPL
i8 fsl_map_range_i8(i8 n, i8 n_min, i8 n_max, i8 r_min, i8 r_max) MAP_RANGE_FUNC_IMPL
u16 fsl_map_range_u16(u16 n, u16 n_min, u16 n_max, u16 r_min, u16 r_max) MAP_RANGE_FUNC_IMPL
i16 fsl_map_range_i16(i16 n, i16 n_min, i16 n_max, i16 r_min, i16 r_max) MAP_RANGE_FUNC_IMPL
u32 fsl_map_range_u32(u32 n, u32 n_min, u32 n_max, u32 r_min, u32 r_max) MAP_RANGE_FUNC_IMPL
i32 fsl_map_range_i32(i32 n, i32 n_min, i32 n_max, i32 r_min, i32 r_max) MAP_RANGE_FUNC_IMPL
f32 fsl_map_range_f32(f32 n, f32 n_min, f32 n_max, f32 r_min, f32 r_max) MAP_RANGE_FUNC_IMPL
u64 fsl_map_range_u64(u64 n, u64 n_min, u64 n_max, u64 r_min, u64 r_max) MAP_RANGE_FUNC_IMPL
i64 fsl_map_range_i64(i64 n, i64 n_min, i64 n_max, i64 r_min, i64 r_max) MAP_RANGE_FUNC_IMPL
f64 fsl_map_range_f64(f64 n, f64 n_min, f64 n_max, f64 r_min, f64 r_max) MAP_RANGE_FUNC_IMPL

b8 fsl_is_in_bounds_u8(u8 n, u8 min, u8 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_i8(i8 n, i8 min, i8 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_u16(u16 n, u16 min, u16 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_i16(i16 n, i16 min, i16 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_u32(u32 n, u32 min, u32 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_i32(i32 n, i32 min, i32 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_f32(f32 n, f32 min, f32 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_u64(u64 n, u64 min, u64 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_i64(i64 n, i64 min, i64 max) IS_IN_BOUNDS_FUNC_IMPL
b8 fsl_is_in_bounds_f64(f64 n, f64 min, f64 max) IS_IN_BOUNDS_FUNC_IMPL

f32 fsl_lerp_f32(f32 a, f32 b, f32 t) LERP_FUNC_IMPL
f64 fsl_lerp_f64(f64 a, f64 b, f64 t) LERP_FUNC_IMPL
f32 fsl_bilerp_f32(f32 a, f32 b, f32 c, f32 d, f32 tx, f32 ty) BILERP_FUNC_IMPL(f32, f)
f64 fsl_bilerp_f64(f64 a, f64 b, f64 c, f64 d, f64 tx, f64 ty) BILERP_FUNC_IMPL(f64, 0)
f32 fsl_trilerp_f32(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h, f32 tx, f32 ty, f32 tz) TRILERP_FUNC_IMPL(f32, f)
f64 fsl_trilerp_f64(f64 a, f64 b, f64 c, f64 d, f64 e, f64 f, f64 g, f64 h, f64 tx, f64 ty, f64 tz) TRILERP_FUNC_IMPL(f64, 0)
f32 fsl_lerp_exp_f32(f32 a, f32 b, f32 k, f32 t) LERP_EXP_FUNC_IMPL(f, expf)
f64 fsl_lerp_exp_f64(f64 a, f64 b, f64 k, f64 t) LERP_EXP_FUNC_IMPL(0, exp)
f32 fsl_fade_ease_in_f32(f32 t) FADE_EASE_IN_FUNC_IMPL
f64 fsl_fade_ease_in_f64(f64 t) FADE_EASE_IN_FUNC_IMPL
f32 fsl_fade_smoothstep_f32(f32 t) FADE_SMOOTHSTEP_FUNC_IMPL(f)
f64 fsl_fade_smoothstep_f64(f64 t) FADE_SMOOTHSTEP_FUNC_IMPL(0)
f32 fsl_fade_quintic_f32(f32 t) FADE_QUINTIC_FUNC_IMPL(f)
f64 fsl_fade_quintic_f64(f64 t) FADE_QUINTIC_FUNC_IMPL(0)

/* ---- section: vector ----------------------------------------------------- */

v2u8 fsl_add_v2u8(v2u8 a, v2u8 b) ADD_V2_FUNC_IMPL
v3u8 fsl_add_v3u8(v3u8 a, v3u8 b) ADD_V3_FUNC_IMPL
v4u8 fsl_add_v4u8(v4u8 a, v4u8 b) ADD_V4_FUNC_IMPL
v2i8 fsl_add_v2i8(v2i8 a, v2i8 b) ADD_V2_FUNC_IMPL
v3i8 fsl_add_v3i8(v3i8 a, v3i8 b) ADD_V3_FUNC_IMPL
v4i8 fsl_add_v4i8(v4i8 a, v4i8 b) ADD_V4_FUNC_IMPL
v2u16 fsl_add_v2u16(v2u16 a, v2u16 b) ADD_V2_FUNC_IMPL
v3u16 fsl_add_v3u16(v3u16 a, v3u16 b) ADD_V3_FUNC_IMPL
v4u16 fsl_add_v4u16(v4u16 a, v4u16 b) ADD_V4_FUNC_IMPL
v2i16 fsl_add_v2i16(v2i16 a, v2i16 b) ADD_V2_FUNC_IMPL
v3i16 fsl_add_v3i16(v3i16 a, v3i16 b) ADD_V3_FUNC_IMPL
v4i16 fsl_add_v4i16(v4i16 a, v4i16 b) ADD_V4_FUNC_IMPL
v2u32 fsl_add_v2u32(v2u32 a, v2u32 b) ADD_V2_FUNC_IMPL
v3u32 fsl_add_v3u32(v3u32 a, v3u32 b) ADD_V3_FUNC_IMPL
v4u32 fsl_add_v4u32(v4u32 a, v4u32 b) ADD_V4_FUNC_IMPL
v2i32 fsl_add_v2i32(v2i32 a, v2i32 b) ADD_V2_FUNC_IMPL
v3i32 fsl_add_v3i32(v3i32 a, v3i32 b) ADD_V3_FUNC_IMPL
v4i32 fsl_add_v4i32(v4i32 a, v4i32 b) ADD_V4_FUNC_IMPL
v2f32 fsl_add_v2f32(v2f32 a, v2f32 b) ADD_V2_FUNC_IMPL
v3f32 fsl_add_v3f32(v3f32 a, v3f32 b) ADD_V3_FUNC_IMPL
v4f32 fsl_add_v4f32(v4f32 a, v4f32 b) ADD_V4_FUNC_IMPL
v2u64 fsl_add_v2u64(v2u64 a, v2u64 b) ADD_V2_FUNC_IMPL
v3u64 fsl_add_v3u64(v3u64 a, v3u64 b) ADD_V3_FUNC_IMPL
v4u64 fsl_add_v4u64(v4u64 a, v4u64 b) ADD_V4_FUNC_IMPL
v2i64 fsl_add_v2i64(v2i64 a, v2i64 b) ADD_V2_FUNC_IMPL
v3i64 fsl_add_v3i64(v3i64 a, v3i64 b) ADD_V3_FUNC_IMPL
v4i64 fsl_add_v4i64(v4i64 a, v4i64 b) ADD_V4_FUNC_IMPL
v2f64 fsl_add_v2f64(v2f64 a, v2f64 b) ADD_V2_FUNC_IMPL
v3f64 fsl_add_v3f64(v3f64 a, v3f64 b) ADD_V3_FUNC_IMPL
v4f64 fsl_add_v4f64(v4f64 a, v4f64 b) ADD_V4_FUNC_IMPL

v2u8 fsl_sub_v2u8(v2u8 a, v2u8 b) SUBTRACT_V2_FUNC_IMPL
v3u8 fsl_sub_v3u8(v3u8 a, v3u8 b) SUBTRACT_V3_FUNC_IMPL
v4u8 fsl_sub_v4u8(v4u8 a, v4u8 b) SUBTRACT_V4_FUNC_IMPL
v2i8 fsl_sub_v2i8(v2i8 a, v2i8 b) SUBTRACT_V2_FUNC_IMPL
v3i8 fsl_sub_v3i8(v3i8 a, v3i8 b) SUBTRACT_V3_FUNC_IMPL
v4i8 fsl_sub_v4i8(v4i8 a, v4i8 b) SUBTRACT_V4_FUNC_IMPL
v2u16 fsl_sub_v2u16(v2u16 a, v2u16 b) SUBTRACT_V2_FUNC_IMPL
v3u16 fsl_sub_v3u16(v3u16 a, v3u16 b) SUBTRACT_V3_FUNC_IMPL
v4u16 fsl_sub_v4u16(v4u16 a, v4u16 b) SUBTRACT_V4_FUNC_IMPL
v2i16 fsl_sub_v2i16(v2i16 a, v2i16 b) SUBTRACT_V2_FUNC_IMPL
v3i16 fsl_sub_v3i16(v3i16 a, v3i16 b) SUBTRACT_V3_FUNC_IMPL
v4i16 fsl_sub_v4i16(v4i16 a, v4i16 b) SUBTRACT_V4_FUNC_IMPL
v2u32 fsl_sub_v2u32(v2u32 a, v2u32 b) SUBTRACT_V2_FUNC_IMPL
v3u32 fsl_sub_v3u32(v3u32 a, v3u32 b) SUBTRACT_V3_FUNC_IMPL
v4u32 fsl_sub_v4u32(v4u32 a, v4u32 b) SUBTRACT_V4_FUNC_IMPL
v2i32 fsl_sub_v2i32(v2i32 a, v2i32 b) SUBTRACT_V2_FUNC_IMPL
v3i32 fsl_sub_v3i32(v3i32 a, v3i32 b) SUBTRACT_V3_FUNC_IMPL
v4i32 fsl_sub_v4i32(v4i32 a, v4i32 b) SUBTRACT_V4_FUNC_IMPL
v2f32 fsl_sub_v2f32(v2f32 a, v2f32 b) SUBTRACT_V2_FUNC_IMPL
v3f32 fsl_sub_v3f32(v3f32 a, v3f32 b) SUBTRACT_V3_FUNC_IMPL
v4f32 fsl_sub_v4f32(v4f32 a, v4f32 b) SUBTRACT_V4_FUNC_IMPL
v2u64 fsl_sub_v2u64(v2u64 a, v2u64 b) SUBTRACT_V2_FUNC_IMPL
v3u64 fsl_sub_v3u64(v3u64 a, v3u64 b) SUBTRACT_V3_FUNC_IMPL
v4u64 fsl_sub_v4u64(v4u64 a, v4u64 b) SUBTRACT_V4_FUNC_IMPL
v2i64 fsl_sub_v2i64(v2i64 a, v2i64 b) SUBTRACT_V2_FUNC_IMPL
v3i64 fsl_sub_v3i64(v3i64 a, v3i64 b) SUBTRACT_V3_FUNC_IMPL
v4i64 fsl_sub_v4i64(v4i64 a, v4i64 b) SUBTRACT_V4_FUNC_IMPL
v2f64 fsl_sub_v2f64(v2f64 a, v2f64 b) SUBTRACT_V2_FUNC_IMPL
v3f64 fsl_sub_v3f64(v3f64 a, v3f64 b) SUBTRACT_V3_FUNC_IMPL
v4f64 fsl_sub_v4f64(v4f64 a, v4f64 b) SUBTRACT_V4_FUNC_IMPL

u8 fsl_min_v2u8(v2u8 v) MIN_V2_FUNC_IMPL
u8 fsl_min_v3u8(v3u8 v) MIN_V3_FUNC_IMPL
u8 fsl_min_v4u8(v4u8 v) MIN_V4_FUNC_IMPL
i8 fsl_min_v2i8(v2i8 v) MIN_V2_FUNC_IMPL
i8 fsl_min_v3i8(v3i8 v) MIN_V3_FUNC_IMPL
i8 fsl_min_v4i8(v4i8 v) MIN_V4_FUNC_IMPL
u16 fsl_min_v2u16(v2u16 v) MIN_V2_FUNC_IMPL
u16 fsl_min_v3u16(v3u16 v) MIN_V3_FUNC_IMPL
u16 fsl_min_v4u16(v4u16 v) MIN_V4_FUNC_IMPL
i16 fsl_min_v2i16(v2i16 v) MIN_V2_FUNC_IMPL
i16 fsl_min_v3i16(v3i16 v) MIN_V3_FUNC_IMPL
i16 fsl_min_v4i16(v4i16 v) MIN_V4_FUNC_IMPL
u32 fsl_min_v2u32(v2u32 v) MIN_V2_FUNC_IMPL
u32 fsl_min_v3u32(v3u32 v) MIN_V3_FUNC_IMPL
u32 fsl_min_v4u32(v4u32 v) MIN_V4_FUNC_IMPL
i32 fsl_min_v2i32(v2i32 v) MIN_V2_FUNC_IMPL
i32 fsl_min_v3i32(v3i32 v) MIN_V3_FUNC_IMPL
i32 fsl_min_v4i32(v4i32 v) MIN_V4_FUNC_IMPL
f32 fsl_min_v2f32(v2f32 v) MIN_V2_FUNC_IMPL
f32 fsl_min_v3f32(v3f32 v) MIN_V3_FUNC_IMPL
f32 fsl_min_v4f32(v4f32 v) MIN_V4_FUNC_IMPL
u64 fsl_min_v2u64(v2u64 v) MIN_V2_FUNC_IMPL
u64 fsl_min_v3u64(v3u64 v) MIN_V3_FUNC_IMPL
u64 fsl_min_v4u64(v4u64 v) MIN_V4_FUNC_IMPL
i64 fsl_min_v2i64(v2i64 v) MIN_V2_FUNC_IMPL
i64 fsl_min_v3i64(v3i64 v) MIN_V3_FUNC_IMPL
i64 fsl_min_v4i64(v4i64 v) MIN_V4_FUNC_IMPL
f64 fsl_min_v2f64(v2f64 v) MIN_V2_FUNC_IMPL
f64 fsl_min_v3f64(v3f64 v) MIN_V3_FUNC_IMPL
f64 fsl_min_v4f64(v4f64 v) MIN_V4_FUNC_IMPL

u8 fsl_max_v2u8(v2u8 v) MAX_V2_FUNC_IMPL
u8 fsl_max_v3u8(v3u8 v) MAX_V3_FUNC_IMPL
u8 fsl_max_v4u8(v4u8 v) MAX_V4_FUNC_IMPL
i8 fsl_max_v2i8(v2i8 v) MAX_V2_FUNC_IMPL
i8 fsl_max_v3i8(v3i8 v) MAX_V3_FUNC_IMPL
i8 fsl_max_v4i8(v4i8 v) MAX_V4_FUNC_IMPL
u16 fsl_max_v2u16(v2u16 v) MAX_V2_FUNC_IMPL
u16 fsl_max_v3u16(v3u16 v) MAX_V3_FUNC_IMPL
u16 fsl_max_v4u16(v4u16 v) MAX_V4_FUNC_IMPL
i16 fsl_max_v2i16(v2i16 v) MAX_V2_FUNC_IMPL
i16 fsl_max_v3i16(v3i16 v) MAX_V3_FUNC_IMPL
i16 fsl_max_v4i16(v4i16 v) MAX_V4_FUNC_IMPL
u32 fsl_max_v2u32(v2u32 v) MAX_V2_FUNC_IMPL
u32 fsl_max_v3u32(v3u32 v) MAX_V3_FUNC_IMPL
u32 fsl_max_v4u32(v4u32 v) MAX_V4_FUNC_IMPL
i32 fsl_max_v2i32(v2i32 v) MAX_V2_FUNC_IMPL
i32 fsl_max_v3i32(v3i32 v) MAX_V3_FUNC_IMPL
i32 fsl_max_v4i32(v4i32 v) MAX_V4_FUNC_IMPL
f32 fsl_max_v2f32(v2f32 v) MAX_V2_FUNC_IMPL
f32 fsl_max_v3f32(v3f32 v) MAX_V3_FUNC_IMPL
f32 fsl_max_v4f32(v4f32 v) MAX_V4_FUNC_IMPL
u64 fsl_max_v2u64(v2u64 v) MAX_V2_FUNC_IMPL
u64 fsl_max_v3u64(v3u64 v) MAX_V3_FUNC_IMPL
u64 fsl_max_v4u64(v4u64 v) MAX_V4_FUNC_IMPL
i64 fsl_max_v2i64(v2i64 v) MAX_V2_FUNC_IMPL
i64 fsl_max_v3i64(v3i64 v) MAX_V3_FUNC_IMPL
i64 fsl_max_v4i64(v4i64 v) MAX_V4_FUNC_IMPL
f64 fsl_max_v2f64(v2f64 v) MAX_V2_FUNC_IMPL
f64 fsl_max_v3f64(v3f64 v) MAX_V3_FUNC_IMPL
f64 fsl_max_v4f64(v4f64 v) MAX_V4_FUNC_IMPL

u8 fsl_min_axis_v2u8(v2u8 v) MIN_AXIS_V2_FUNC_IMPL
u8 fsl_min_axis_v3u8(v3u8 v) MIN_AXIS_V3_FUNC_IMPL
u8 fsl_min_axis_v4u8(v4u8 v) MIN_AXIS_V4_FUNC_IMPL
u8 fsl_min_axis_v2i8(v2i8 v) MIN_AXIS_V2_FUNC_IMPL
u8 fsl_min_axis_v3i8(v3i8 v) MIN_AXIS_V3_FUNC_IMPL
u8 fsl_min_axis_v4i8(v4i8 v) MIN_AXIS_V4_FUNC_IMPL
u16 fsl_min_axis_v2u16(v2u16 v) MIN_AXIS_V2_FUNC_IMPL
u16 fsl_min_axis_v3u16(v3u16 v) MIN_AXIS_V3_FUNC_IMPL
u16 fsl_min_axis_v4u16(v4u16 v) MIN_AXIS_V4_FUNC_IMPL
u16 fsl_min_axis_v2i16(v2i16 v) MIN_AXIS_V2_FUNC_IMPL
u16 fsl_min_axis_v3i16(v3i16 v) MIN_AXIS_V3_FUNC_IMPL
u16 fsl_min_axis_v4i16(v4i16 v) MIN_AXIS_V4_FUNC_IMPL
u32 fsl_min_axis_v2u32(v2u32 v) MIN_AXIS_V2_FUNC_IMPL
u32 fsl_min_axis_v3u32(v3u32 v) MIN_AXIS_V3_FUNC_IMPL
u32 fsl_min_axis_v4u32(v4u32 v) MIN_AXIS_V4_FUNC_IMPL
u32 fsl_min_axis_v2i32(v2i32 v) MIN_AXIS_V2_FUNC_IMPL
u32 fsl_min_axis_v3i32(v3i32 v) MIN_AXIS_V3_FUNC_IMPL
u32 fsl_min_axis_v4i32(v4i32 v) MIN_AXIS_V4_FUNC_IMPL
u32 fsl_min_axis_v2f32(v2f32 v) MIN_AXIS_V2_FUNC_IMPL
u32 fsl_min_axis_v3f32(v3f32 v) MIN_AXIS_V3_FUNC_IMPL
u32 fsl_min_axis_v4f32(v4f32 v) MIN_AXIS_V4_FUNC_IMPL
u64 fsl_min_axis_v2u64(v2u64 v) MIN_AXIS_V2_FUNC_IMPL
u64 fsl_min_axis_v3u64(v3u64 v) MIN_AXIS_V3_FUNC_IMPL
u64 fsl_min_axis_v4u64(v4u64 v) MIN_AXIS_V4_FUNC_IMPL
u64 fsl_min_axis_v2i64(v2i64 v) MIN_AXIS_V2_FUNC_IMPL
u64 fsl_min_axis_v3i64(v3i64 v) MIN_AXIS_V3_FUNC_IMPL
u64 fsl_min_axis_v4i64(v4i64 v) MIN_AXIS_V4_FUNC_IMPL
u64 fsl_min_axis_v2f64(v2f64 v) MIN_AXIS_V2_FUNC_IMPL
u64 fsl_min_axis_v3f64(v3f64 v) MIN_AXIS_V3_FUNC_IMPL
u64 fsl_min_axis_v4f64(v4f64 v) MIN_AXIS_V4_FUNC_IMPL

u8 fsl_max_axis_v2u8(v2u8 v) MAX_AXIS_V2_FUNC_IMPL
u8 fsl_max_axis_v3u8(v3u8 v) MAX_AXIS_V3_FUNC_IMPL
u8 fsl_max_axis_v4u8(v4u8 v) MAX_AXIS_V4_FUNC_IMPL
u8 fsl_max_axis_v2i8(v2i8 v) MAX_AXIS_V2_FUNC_IMPL
u8 fsl_max_axis_v3i8(v3i8 v) MAX_AXIS_V3_FUNC_IMPL
u8 fsl_max_axis_v4i8(v4i8 v) MAX_AXIS_V4_FUNC_IMPL
u16 fsl_max_axis_v2u16(v2u16 v) MAX_AXIS_V2_FUNC_IMPL
u16 fsl_max_axis_v3u16(v3u16 v) MAX_AXIS_V3_FUNC_IMPL
u16 fsl_max_axis_v4u16(v4u16 v) MAX_AXIS_V4_FUNC_IMPL
u16 fsl_max_axis_v2i16(v2i16 v) MAX_AXIS_V2_FUNC_IMPL
u16 fsl_max_axis_v3i16(v3i16 v) MAX_AXIS_V3_FUNC_IMPL
u16 fsl_max_axis_v4i16(v4i16 v) MAX_AXIS_V4_FUNC_IMPL
u32 fsl_max_axis_v2u32(v2u32 v) MAX_AXIS_V2_FUNC_IMPL
u32 fsl_max_axis_v3u32(v3u32 v) MAX_AXIS_V3_FUNC_IMPL
u32 fsl_max_axis_v4u32(v4u32 v) MAX_AXIS_V4_FUNC_IMPL
u32 fsl_max_axis_v2i32(v2i32 v) MAX_AXIS_V2_FUNC_IMPL
u32 fsl_max_axis_v3i32(v3i32 v) MAX_AXIS_V3_FUNC_IMPL
u32 fsl_max_axis_v4i32(v4i32 v) MAX_AXIS_V4_FUNC_IMPL
u32 fsl_max_axis_v2f32(v2f32 v) MAX_AXIS_V2_FUNC_IMPL
u32 fsl_max_axis_v3f32(v3f32 v) MAX_AXIS_V3_FUNC_IMPL
u32 fsl_max_axis_v4f32(v4f32 v) MAX_AXIS_V4_FUNC_IMPL
u64 fsl_max_axis_v2u64(v2u64 v) MAX_AXIS_V2_FUNC_IMPL
u64 fsl_max_axis_v3u64(v3u64 v) MAX_AXIS_V3_FUNC_IMPL
u64 fsl_max_axis_v4u64(v4u64 v) MAX_AXIS_V4_FUNC_IMPL
u64 fsl_max_axis_v2i64(v2i64 v) MAX_AXIS_V2_FUNC_IMPL
u64 fsl_max_axis_v3i64(v3i64 v) MAX_AXIS_V3_FUNC_IMPL
u64 fsl_max_axis_v4i64(v4i64 v) MAX_AXIS_V4_FUNC_IMPL
u64 fsl_max_axis_v2f64(v2f64 v) MAX_AXIS_V2_FUNC_IMPL
u64 fsl_max_axis_v3f64(v3f64 v) MAX_AXIS_V3_FUNC_IMPL
u64 fsl_max_axis_v4f64(v4f64 v) MAX_AXIS_V4_FUNC_IMPL

b8 fsl_is_in_bounds_v2u8(v2u8 v, v2u8 min, v2u8 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3u8(v3u8 v, v3u8 min, v3u8 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4u8(v4u8 v, v4u8 min, v4u8 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2i8(v2i8 v, v2i8 min, v2i8 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3i8(v3i8 v, v3i8 min, v3i8 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4i8(v4i8 v, v4i8 min, v4i8 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2u16(v2u16 v, v2u16 min, v2u16 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3u16(v3u16 v, v3u16 min, v3u16 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4u16(v4u16 v, v4u16 min, v4u16 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2i16(v2i16 v, v2i16 min, v2i16 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3i16(v3i16 v, v3i16 min, v3i16 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4i16(v4i16 v, v4i16 min, v4i16 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2u32(v2u32 v, v2u32 min, v2u32 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3u32(v3u32 v, v3u32 min, v3u32 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4u32(v4u32 v, v4u32 min, v4u32 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2i32(v2i32 v, v2i32 min, v2i32 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3i32(v3i32 v, v3i32 min, v3i32 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4i32(v4i32 v, v4i32 min, v4i32 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2f32(v2f32 v, v2f32 min, v2f32 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3f32(v3f32 v, v3f32 min, v3f32 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4f32(v4f32 v, v4f32 min, v4f32 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2u64(v2u64 v, v2u64 min, v2u64 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3u64(v3u64 v, v3u64 min, v3u64 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4u64(v4u64 v, v4u64 min, v4u64 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2i64(v2i64 v, v2i64 min, v2i64 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3i64(v3i64 v, v3i64 min, v3i64 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4i64(v4i64 v, v4i64 min, v4i64 max) IS_IN_BOUNDS_V4_FUNC_IMPL
b8 fsl_is_in_bounds_v2f64(v2f64 v, v2f64 min, v2f64 max) IS_IN_BOUNDS_V2_FUNC_IMPL
b8 fsl_is_in_bounds_v3f64(v3f64 v, v3f64 min, v3f64 max) IS_IN_BOUNDS_V3_FUNC_IMPL
b8 fsl_is_in_bounds_v4f64(v4f64 v, v4f64 min, v4f64 max) IS_IN_BOUNDS_V4_FUNC_IMPL

u8 fsl_len_v2u8(v2u8 v) LENGTH_V2_FUNC_IMPL
u8 fsl_len_v3u8(v3u8 v) LENGTH_V3_FUNC_IMPL
u8 fsl_len_v4u8(v4u8 v) LENGTH_V4_FUNC_IMPL
i8 fsl_len_v2i8(v2i8 v) LENGTH_V2_FUNC_IMPL
i8 fsl_len_v3i8(v3i8 v) LENGTH_V3_FUNC_IMPL
i8 fsl_len_v4i8(v4i8 v) LENGTH_V4_FUNC_IMPL
u16 fsl_len_v2u16(v2u16 v) LENGTH_V2_FUNC_IMPL
u16 fsl_len_v3u16(v3u16 v) LENGTH_V3_FUNC_IMPL
u16 fsl_len_v4u16(v4u16 v) LENGTH_V4_FUNC_IMPL
i16 fsl_len_v2i16(v2i16 v) LENGTH_V2_FUNC_IMPL
i16 fsl_len_v3i16(v3i16 v) LENGTH_V3_FUNC_IMPL
i16 fsl_len_v4i16(v4i16 v) LENGTH_V4_FUNC_IMPL
u32 fsl_len_v2u32(v2u32 v) LENGTH_V2_FUNC_IMPL
u32 fsl_len_v3u32(v3u32 v) LENGTH_V3_FUNC_IMPL
u32 fsl_len_v4u32(v4u32 v) LENGTH_V4_FUNC_IMPL
i32 fsl_len_v2i32(v2i32 v) LENGTH_V2_FUNC_IMPL
i32 fsl_len_v3i32(v3i32 v) LENGTH_V3_FUNC_IMPL
i32 fsl_len_v4i32(v4i32 v) LENGTH_V4_FUNC_IMPL
f32 fsl_len_v2f32(v2f32 v) LENGTH_V2_FUNC_IMPL
f32 fsl_len_v3f32(v3f32 v) LENGTH_V3_FUNC_IMPL
f32 fsl_len_v4f32(v4f32 v) LENGTH_V4_FUNC_IMPL
u64 fsl_len_v2u64(v2u64 v) LENGTH_V2_FUNC_IMPL
u64 fsl_len_v3u64(v3u64 v) LENGTH_V3_FUNC_IMPL
u64 fsl_len_v4u64(v4u64 v) LENGTH_V4_FUNC_IMPL
i64 fsl_len_v2i64(v2i64 v) LENGTH_V2_FUNC_IMPL
i64 fsl_len_v3i64(v3i64 v) LENGTH_V3_FUNC_IMPL
i64 fsl_len_v4i64(v4i64 v) LENGTH_V4_FUNC_IMPL
f64 fsl_len_v2f64(v2f64 v) LENGTH_V2_FUNC_IMPL
f64 fsl_len_v3f64(v3f64 v) LENGTH_V3_FUNC_IMPL
f64 fsl_len_v4f64(v4f64 v) LENGTH_V4_FUNC_IMPL

u8 fsl_distance_v2u8(v2u8 a, v2u8 b) DISTANCE_V2_FUNC_IMPL
u8 fsl_distance_v3u8(v3u8 a, v3u8 b) DISTANCE_V3_FUNC_IMPL
u8 fsl_distance_v4u8(v4u8 a, v4u8 b) DISTANCE_V4_FUNC_IMPL
i8 fsl_distance_v2i8(v2i8 a, v2i8 b) DISTANCE_V2_FUNC_IMPL
i8 fsl_distance_v3i8(v3i8 a, v3i8 b) DISTANCE_V3_FUNC_IMPL
i8 fsl_distance_v4i8(v4i8 a, v4i8 b) DISTANCE_V4_FUNC_IMPL
u16 fsl_distance_v2u16(v2u16 a, v2u16 b) DISTANCE_V2_FUNC_IMPL
u16 fsl_distance_v3u16(v3u16 a, v3u16 b) DISTANCE_V3_FUNC_IMPL
u16 fsl_distance_v4u16(v4u16 a, v4u16 b) DISTANCE_V4_FUNC_IMPL
i16 fsl_distance_v2i16(v2i16 a, v2i16 b) DISTANCE_V2_FUNC_IMPL
i16 fsl_distance_v3i16(v3i16 a, v3i16 b) DISTANCE_V3_FUNC_IMPL
i16 fsl_distance_v4i16(v4i16 a, v4i16 b) DISTANCE_V4_FUNC_IMPL
u32 fsl_distance_v2u32(v2u32 a, v2u32 b) DISTANCE_V2_FUNC_IMPL
u32 fsl_distance_v3u32(v3u32 a, v3u32 b) DISTANCE_V3_FUNC_IMPL
u32 fsl_distance_v4u32(v4u32 a, v4u32 b) DISTANCE_V4_FUNC_IMPL
i32 fsl_distance_v2i32(v2i32 a, v2i32 b) DISTANCE_V2_FUNC_IMPL
i32 fsl_distance_v3i32(v3i32 a, v3i32 b) DISTANCE_V3_FUNC_IMPL
i32 fsl_distance_v4i32(v4i32 a, v4i32 b) DISTANCE_V4_FUNC_IMPL
f32 fsl_distance_v2f32(v2f32 a, v2f32 b) DISTANCE_V2_FUNC_IMPL
f32 fsl_distance_v3f32(v3f32 a, v3f32 b) DISTANCE_V3_FUNC_IMPL
f32 fsl_distance_v4f32(v4f32 a, v4f32 b) DISTANCE_V4_FUNC_IMPL
u64 fsl_distance_v2u64(v2u64 a, v2u64 b) DISTANCE_V2_FUNC_IMPL
u64 fsl_distance_v3u64(v3u64 a, v3u64 b) DISTANCE_V3_FUNC_IMPL
u64 fsl_distance_v4u64(v4u64 a, v4u64 b) DISTANCE_V4_FUNC_IMPL
i64 fsl_distance_v2i64(v2i64 a, v2i64 b) DISTANCE_V2_FUNC_IMPL
i64 fsl_distance_v3i64(v3i64 a, v3i64 b) DISTANCE_V3_FUNC_IMPL
i64 fsl_distance_v4i64(v4i64 a, v4i64 b) DISTANCE_V4_FUNC_IMPL
f64 fsl_distance_v2f64(v2f64 a, v2f64 b) DISTANCE_V2_FUNC_IMPL
f64 fsl_distance_v3f64(v3f64 a, v3f64 b) DISTANCE_V3_FUNC_IMPL
f64 fsl_distance_v4f64(v4f64 a, v4f64 b) DISTANCE_V4_FUNC_IMPL

v2f32 fsl_normalize_v2f32(v2f32 v) NORMALIZE_V2_FUNC_IMPL(f32, sqrtf, fsl_len_v2f32, f)
v3f32 fsl_normalize_v3f32(v3f32 v) NORMALIZE_V3_FUNC_IMPL(f32, sqrtf, fsl_len_v3f32, f)
v4f32 fsl_normalize_v4f32(v4f32 v) NORMALIZE_V4_FUNC_IMPL(f32, sqrtf, fsl_len_v4f32, f)
v2f64 fsl_normalize_v2f64(v2f64 v) NORMALIZE_V2_FUNC_IMPL(f64, sqrt, fsl_len_v2f64, 0)
v3f64 fsl_normalize_v3f64(v3f64 v) NORMALIZE_V3_FUNC_IMPL(f64, sqrt, fsl_len_v3f64, 0)
v4f64 fsl_normalize_v4f64(v4f64 v) NORMALIZE_V4_FUNC_IMPL(f64, sqrt, fsl_len_v4f64, 0)

f32 fsl_dot_v2f32(v2f32 a, v2f32 b) DOT_V2_FUNC_IMPL
f32 fsl_dot_v3f32(v3f32 a, v3f32 b) DOT_V3_FUNC_IMPL
f32 fsl_dot_v4f32(v4f32 a, v4f32 b) DOT_V4_FUNC_IMPL
f64 fsl_dot_v2f64(v2f64 a, v2f64 b) DOT_V2_FUNC_IMPL
f64 fsl_dot_v3f64(v3f64 a, v3f64 b) DOT_V3_FUNC_IMPL
f64 fsl_dot_v4f64(v4f64 a, v4f64 b) DOT_V4_FUNC_IMPL

/* ---- section: matrix ----------------------------------------------------- */

m2f32 fsl_identity_m2f32(void) IDENTITY_M2_FUNC_IMPL(m2f32, f)
m3f32 fsl_identity_m3f32(void) IDENTITY_M3_FUNC_IMPL(m3f32, f)
m4f32 fsl_identity_m4f32(void) IDENTITY_M4_FUNC_IMPL(m4f32, f)
m2f64 fsl_identity_m2f64(void) IDENTITY_M2_FUNC_IMPL(m2f64, 0)
m3f64 fsl_identity_m3f64(void) IDENTITY_M3_FUNC_IMPL(m3f64, 0)
m4f64 fsl_identity_m4f64(void) IDENTITY_M4_FUNC_IMPL(m4f64, 0)
m2f32 fsl_add_m2f32(m2f32 a, m2f32 b) ADD_M2_FUNC_IMPL
m3f32 fsl_add_m3f32(m3f32 a, m3f32 b) ADD_M3_FUNC_IMPL
m4f32 fsl_add_m4f32(m4f32 a, m4f32 b) ADD_M4_FUNC_IMPL
m2f64 fsl_add_m2f64(m2f64 a, m2f64 b) ADD_M2_FUNC_IMPL
m3f64 fsl_add_m3f64(m3f64 a, m3f64 b) ADD_M3_FUNC_IMPL
m4f64 fsl_add_m4f64(m4f64 a, m4f64 b) ADD_M4_FUNC_IMPL
m2f32 fsl_sub_m2f32(m2f32 a, m2f32 b) SUBTRACT_M2_FUNC_IMPL
m3f32 fsl_sub_m3f32(m3f32 a, m3f32 b) SUBTRACT_M3_FUNC_IMPL
m4f32 fsl_sub_m4f32(m4f32 a, m4f32 b) SUBTRACT_M4_FUNC_IMPL
m2f64 fsl_sub_m2f64(m2f64 a, m2f64 b) SUBTRACT_M2_FUNC_IMPL
m3f64 fsl_sub_m3f64(m3f64 a, m3f64 b) SUBTRACT_M3_FUNC_IMPL
m4f64 fsl_sub_m4f64(m4f64 a, m4f64 b) SUBTRACT_M4_FUNC_IMPL
m2f32 fsl_sub_identity_m2f32(m2f32 m) SUBTRACT_IDENTITY_M2_FUNC_IMPL(f)
m3f32 fsl_sub_identity_m3f32(m3f32 m) SUBTRACT_IDENTITY_M3_FUNC_IMPL(f)
m4f32 fsl_sub_identity_m4f32(m4f32 m) SUBTRACT_IDENTITY_M4_FUNC_IMPL(f)
m2f64 fsl_sub_identity_m2f64(m2f64 m) SUBTRACT_IDENTITY_M2_FUNC_IMPL(0)
m3f64 fsl_sub_identity_m3f64(m3f64 m) SUBTRACT_IDENTITY_M3_FUNC_IMPL(0)
m4f64 fsl_sub_identity_m4f64(m4f64 m) SUBTRACT_IDENTITY_M4_FUNC_IMPL(0)
m2f32 fsl_multiply_m2f32(m2f32 a, m2f32 b) MULTIPLY_M2_FUNC_IMPL(m2f32)
m3f32 fsl_multiply_m3f32(m3f32 a, m3f32 b) MULTIPLY_M3_FUNC_IMPL(m3f32)
m4f32 fsl_multiply_m4f32(m4f32 a, m4f32 b) MULTIPLY_M4_FUNC_IMPL(m4f32)
m2f64 fsl_multiply_m2f64(m2f64 a, m2f64 b) MULTIPLY_M2_FUNC_IMPL(m2f64)
m3f64 fsl_multiply_m3f64(m3f64 a, m3f64 b) MULTIPLY_M3_FUNC_IMPL(m3f64)
m4f64 fsl_multiply_m4f64(m4f64 a, m4f64 b) MULTIPLY_M4_FUNC_IMPL(m4f64)
v2f32 fsl_multiply_vector_m2f32(m2f32 m, v2f32 v) MULTIPLY_VECTOR_M2_FUNC_IMPL(v2f32)
v3f32 fsl_multiply_vector_m3f32(m3f32 m, v3f32 v) MULTIPLY_VECTOR_M3_FUNC_IMPL(v3f32)
v4f32 fsl_multiply_vector_m4f32(m4f32 m, v4f32 v) MULTIPLY_VECTOR_M4_FUNC_IMPL(v4f32)
v2f64 fsl_multiply_vector_m2f64(m2f64 m, v2f64 v) MULTIPLY_VECTOR_M2_FUNC_IMPL(v2f64)
v3f64 fsl_multiply_vector_m3f64(m3f64 m, v3f64 v) MULTIPLY_VECTOR_M3_FUNC_IMPL(v3f64)
v4f64 fsl_multiply_vector_m4f64(m4f64 m, v4f64 v) MULTIPLY_VECTOR_M4_FUNC_IMPL(v4f64)
m2f32 fsl_transpose_m2f32(m2f32 m) TRANSPOSE_M2_FUNC_IMPL(m2f32)
m3f32 fsl_transpose_m3f32(m3f32 m) TRANSPOSE_M3_FUNC_IMPL(m3f32)
m4f32 fsl_transpose_m4f32(m4f32 m) TRANSPOSE_M4_FUNC_IMPL(m4f32)
m2f64 fsl_transpose_m2f64(m2f64 m) TRANSPOSE_M2_FUNC_IMPL(m2f64)
m3f64 fsl_transpose_m3f64(m3f64 m) TRANSPOSE_M3_FUNC_IMPL(m3f64)
m4f64 fsl_transpose_m4f64(m4f64 m) TRANSPOSE_M4_FUNC_IMPL(m4f64)
m2f32 fsl_outer_m2f32(v2f32 a, v2f32 b) OUTER_PRODUCT_M2_FUNC_IMPL(m2f32)
m3f32 fsl_outer_m3f32(v3f32 a, v3f32 b) OUTER_PRODUCT_M3_FUNC_IMPL(m3f32)
m4f32 fsl_outer_m4f32(v4f32 a, v4f32 b) OUTER_PRODUCT_M4_FUNC_IMPL(m4f32)
m2f64 fsl_outer_m2f64(v2f64 a, v2f64 b) OUTER_PRODUCT_M2_FUNC_IMPL(m2f64)
m3f64 fsl_outer_m3f64(v3f64 a, v3f64 b) OUTER_PRODUCT_M3_FUNC_IMPL(m3f64)
m4f64 fsl_outer_m4f64(v4f64 a, v4f64 b) OUTER_PRODUCT_M4_FUNC_IMPL(m4f64)

/* ---- section: trigonometry ----------------------------------------------- */

angle_f32 fsl_angle_f32(f32 n)
{
    angle_f32 angle = {0};
    angle.angle = n;
    angle.sin = sinf(n);
    angle.cos = sinf(n + FSL_HALF_PI);
    angle.tan = tanf(n);
    return angle;
}

angle_f64 fsl_angle_f64(f64 n)
{
    angle_f64 angle = {0};
    angle.angle = n;
    angle.sin = sin(n);
    angle.cos = sin(n + FSL_HALF_PI);
    angle.tan = tan(n);
    return angle;
}

/* ---- section: random ----------------------------------------------------- */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"

u32 fsl_rand_u32(u32 n)
{
    const u32 S = 32;
    u32 a = n + 234678493574;
    u32 b = n - 879763936541;
    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    return a * 2048419325;
}

u64 fsl_rand_u64(u64 n)
{
    const u64 S = 64;
    u64 a = n + 234678493574;
    u64 b = n - 879763936541;
    a *= 3284157443;
    b ^= a << S | a >> S;
    b *= 1911520717;
    a ^= b << S | b >> S;
    return a * 2048419325;
}

#pragma GCC diagnostic pop /* ignored "-Wshift-count-overflow" */
