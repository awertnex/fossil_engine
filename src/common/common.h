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
 *  @file common.h
 *
 *  @brief main common header.
 */

#ifndef FSL_COMMON_H
#define FSL_COMMON_H

#include "engine_info.h"
#include "common_values.h"
#include "config.h"
#include "diagnostics.h"
#include "limits.h"
#include "session.h"
#include "types.h"

/* ---- color conversion macros --------------------------------------------- */

/*!
 *  @brief convert RGBA color to 4-Byte hex color.
 *  @remark color range [0.0f, 1.0f].
 */
#define fsl_color_v4_to_hex(r, g, b, a) \
    (((u32)((r) * 0xff) << 0x18) | \
     ((u32)((g) * 0xff) << 0x10) | \
     ((u32)((b) * 0xff) << 0x08) | \
     ((u32)(a) * 0xff))

/*!
 *  @brief convert 4-Byte hex color to RGBA color.
 *  @remark color range [0.0f, 1.0f].
 */
#define fsl_color_hex_to_v4(n) (v4f32){ \
    (f32)(((n) >> 0x18) & 0xff) / 0xff, \
    (f32)(((n) >> 0x10) & 0xff) / 0xff, \
    (f32)(((n) >> 0x08) & 0xff) / 0xff, \
    (f32)(((n) >> 0x00) & 0xff) / 0xff}

#endif /* FSL_COMMON_H */
