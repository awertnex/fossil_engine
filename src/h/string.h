/*  @file string.h
 *
 *  @brief string parsing, token searching.
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

#ifndef FSL_STRING_H
#define FSL_STRING_H

#include "common.h"
#include "types.h"

/*! @brief swap string buffers `s1` with `s2` (without a `temp` buffer).
 */
FSLAPI void fsl_swap_strings(str *s1, str *s2);

/*! @brief swap all occurrences of `c1` in `string` with `c2` (without a `temp` buffer).
 *
 *  @return processed `string`.
 */
FSLAPI void fsl_swap_string_char(str *string, char c1, char c2);

/*! @brief write temporary formatted string.
 *
 *  @remark use temporary static buffers internally.
 *  @remark inspired by Raylib: `github.com/raysan5/raylib`: `raylib/src/rtext.c/TextFormat()`.
 *
 *  @return static formatted string.
 */
FSLAPI str *fsl_stringf(const str *format, ...);

/*! @brief compare `arg` to any of `argv` entries.
 *
 *  @return `argc` of match if found, 0 otherwise.
 */
FSLAPI u64 fsl_find_token(str *arg, int argc, str **argv);

/*! @brief load tokens from file at `path` into a @ref fsl_key_value buffer as
 *  `str` and `u64` arrays respectively.
 *
 *  @return @ref fsl_key_value buffer of tokens.
 *  @return `(fsl_key_value){0}` on failure and @ref fsl_err is set accordingly.
 */
FSLAPI fsl_key_value fsl_get_tokens_key_val(const str *path);

/*! @brief convert an int into a string.
 *
 *  convert a signed 32-bit integer into a string and write into `dst`
 *  at most `size` bytes.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_i32_to_str(str *dst, i32 size, i32 n);

/*! @brief convert an int into a string.
 *
 *  convert an unsigned 64-bit integer into a string and write into `dst`
 *  at most `size` bytes.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_u64_to_str(str *dst, u64 size, u64 n);

#endif /* FSL_STRING_H */
