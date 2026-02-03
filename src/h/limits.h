/*  @file limits.h
 *
 *  @brief limits and data type value ranges.
 *
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
 *  limitations under the License.OFTWARE.
 */

#ifndef FSL_LIMITS_H
#define FSL_LIMITS_H

#include "common.h"

#if FSL_PLATFORM_WIN
#   define NAME_MAX 255
#   include <limits.h>
#else
#   include <linux/limits.h>
#endif /* FSL_PLATFORM */

#define FSL_U8_MAX 255
#define FSL_I8_MAX 127
#define FSL_I8_MIN (-FSL_I8_MAX - 1)

#define FSL_U16_MAX 65535
#define FSL_I16_MAX 32767
#define FSL_I16_MIN (-FSL_I16_MAX - 1)

#define FSL_U32_MAX 4294967295
#define FSL_I32_MAX 2147483647
#define FSL_I32_MIN (-FSL_I32_MAX - 1)

#define FSL_U64_MAX 18446744073709551615
#define FSL_I64_MAX 9223372036854775807
#define FSL_I64_MIN (-FSL_I64_MAX - 1)

#define FSL_STRING_MAX              2048
#define FSL_STRING_TOKEN_MAX        128
#define FSL_STRINGF_BUFFERS_MAX     4
#define FSL_TIME_STRING_MAX         128
#define FSL_OUT_STRING_MAX          (FSL_STRING_MAX + 256)
#define FSL_LOGGER_STRING_MAX       FSL_OUT_STRING_MAX
#define FSL_LOGGER_HISTORY_MAX      2048
#define FSL_GLYPH_MAX               256
#define FSL_INCLUDE_RECURSION_MAX   512
#define FSL_SCREENSHOT_RATE_MAX     128

#endif /* FSL_LIMITS_H */
