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
 *  @file string.h
 *
 *  @brief string parsing, token searching.
 */

#ifndef FSL_STRING_H
#define FSL_STRING_H

#include "../common/engine_info.h"
#include "../common/types.h"

/*!
 *  @brief swap string buffers `s1` with `s2` (without a `temp` buffer).
 */
FSLAPI void fsl_swap_strings(str *s1, str *s2);

/*!
 *  @brief swap all occurrences of `c1` in `string` with `c2` (without a `temp` buffer).
 *
 *  @return processed `string`.
 */
FSLAPI void fsl_swap_string_char(str *string, char c1, char c2);

/*!
 *  @brief write temporary formatted string.
 *
 *  @note the use of @ref fsl_stringf more than once in a single expression is not advised.
 *
 *  @remark use temporary static buffers internally.
 *  @remark inspired by Raylib: `github.com/raysan5/raylib`: `raylib/src/rtext.c/TextFormat()`.
 *
 *  @return static formatted string.
 */
FSLAPI str *fsl_stringf(const str *format, ...);

/*!
 *  @brief skip leading spaces in `*string`.
 *
 *  @remark modifies `*string`.
 */
FSLAPI void fsl_skip_spaces(str **string);

/*!
 *  @brief strip `string` of non-printable ascii and non-ascii characters.
 */
FSLAPI void fsl_strip_non_printable(str *string);

/*!
 *  @brief compare `arg` to any of `argv` entries.
 *
 *  @return `argc` of match if found, 0 otherwise.
 */
FSLAPI u64 fsl_find_token(str *arg, int argc, str **argv);

/*!
 *  @brief load tokens from file at `path` into a @ref fsl_key_value buffer as
 *  `str` and `u64` arrays respectively.
 *
 *  @return @ref fsl_key_value buffer of tokens.
 *  @return `(fsl_key_value){0}` on failure and @ref fsl_err is set accordingly.
 */
FSLAPI fsl_key_value fsl_get_tokens_key_val(const str *path);

/*!
 *  @return FALSE on failure.
 */
FSLAPI b8 fsl_is_digit(char n);

/*!
 *  @brief convert a character representation of a digit into a signed 32-bit integer.
 *
 *  @return converted digit.
 */
FSLAPI i32 fsl_convert_char_to_int(char n);

/*!
 *  @brief convert a string representation of a number into an unsigned 32-bit
 *  integer and write into `dst`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_str_to_u32(const str *n, u32 *dst);

/*!
 *  @brief convert a string representation of a number into an unsigned 64-bit
 *  integer and write into `dst`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_str_to_u64(const str *n, u64 *dst);

/*!
 *  @brief convert a string representation of a number into a signed 64-bit
 *  integer and write into `dst`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_str_to_i64(const str *n, i64 *dst);

/*!
 *  @brief convert a string representation of a number into a 32-bit float and write into `dst`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_str_to_f32(const str *n, f32 *dst);

/*!
 *  @brief convert a string representation of a number into a 64-bit float and write into `dst`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_str_to_f64(const str *n, f64 *dst);

/*!
 *  @brief convert a signed 32-bit integer into a string and write into `dst`
 *  at most `size` bytes.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_i32_to_str(str *dst, i32 size, i32 n);

/*!
 *  @brief convert an unsigned 64-bit integer into a string and write into `dst`
 *  at most `size` bytes.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_convert_u64_to_str(str *dst, u64 size, u64 n);

/*!
 *  @brief use 'run-length-encoding' (RLE) algorithm on `data` using the first datum.
 *
 *  @param size size of each element in `data`.
 *  @param len number of elements in `data`.
 *
 *  @return run-length (1 = one occurrence), zero (0) on failure.
 */
FSLAPI u64 fsl_rle(const void *data, fsl_size size, fsl_len len);

/*!
 *  @brief create basic 'djb2' hash from `data`.
 *
 *  @param len data size, in bytes (0 auto-detects size, useful for strings).
 *
 *  @return hash.
 */
FSLAPI u64 fsl_hash_djb2_u64(const void *data, fsl_cap len);

/*!
 *  @brief create basic 'FNV-1a' hash from `data`.
 *
 *  @param len data size, in bytes (0 auto-detects size, useful for strings).
 *
 *  @return hash.
 */
FSLAPI u64 fsl_hash_fnv1a_u64(const void *data, fsl_cap len);

/*!
 *  @brief find an unsigned 64-bit hash within `buf` that matches `hash`.
 *
 *  @param dst recipient of hash index if found.
 *  @param len number of elements in `buf`.
 *
 *  @remark modifies `dst` only when hash is found.
 */
FSLAPI b8 fsl_find_hash_u64(u64 hash, u64 *buf, u64 *dst, fsl_len len);

#endif /* FSL_STRING_H */
