#ifndef ENGINE_STRING_H
#define ENGINE_STRING_H

#include "common.h"
#include "types.h"

/*! @brief swap string buffers 's1' with 's2'.
 *
 *  @remark doesn't use a temp buffer, swaps bits in-place.
 */
FSLAPI void swap_strings(str *s1, str *s2);

/*! @brief swap all occurrences of 'c1' in 'string' with 'c2'.
 *
 *  @remark doesn't use a temp buffer, swaps bits in-place.
 *
 *  @return processed 'string'.
 */
FSLAPI str *swap_string_char(str *string, char c1, char c2);

/*! @brief write temporary formatted string.
 *
 *  @remark uses temporary static buffers internally.
 *  @remark inspired by Raylib: 'github.com/raysan5/raylib': raylib/src/rtext.c/TextFormat().
 *
 *  @return static formatted string.
 */
FSLAPI str *stringf(const str *format, ...);

/*! @brief compare 'arg' to any of 'argv' entries.
 *
 *  @return 'argc' of match if match found.
 *  @return 0 if no matches found.
 */
FSLAPI u64 find_token(str *arg, int argc, str **argv);

/*! @brief load tokens from file at 'path' into a 'KeyValue' buffer as
 * 'str' and 'u64' arrays respectively.
 *
 *  @return 'KeyValue' buffer of tokens.
 *  @return (KeyValue){0} on failure and 'engine_err' is set accordingly.
 */
FSLAPI KeyValue get_tokens_key_val(const str *path);

/*! @brief convert an int into a string.
 *
 *  convert a signed 32-bit integer into a string and write into 'dst'
 *  at most 'size' bytes.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 convert_i32_to_str(str *dst, i32 size, i32 n);


/*! @brief convert an int into a string.
 *
 *  convert an unsigned 64-bit integer into a string and write into 'dst'
 *  at most 'size' bytes.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 convert_u64_to_str(str *dst, u64 size, u64 n);

#endif /* ENGINE_STRING_H */
