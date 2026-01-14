#ifndef ENGINE_MEMORY_H
#define ENGINE_MEMORY_H

#include "types.h"
#include "limits.h"

#define arr_len(arr) ((u64)sizeof(arr) / sizeof(arr[0]))

#define mem_alloc(x, size, name) \
    _mem_alloc(x, size, name, __BASE_FILE__, __LINE__)

#define mem_alloc_memb(x, memb, size, name) \
    _mem_alloc_memb(x, memb, size, name, __BASE_FILE__, __LINE__)

#define mem_alloc_buf(x, memb, size, name) \
    _mem_alloc_buf(x, memb, size, name, __BASE_FILE__, __LINE__)

#define mem_alloc_key_val(x, memb, size_key, size_val, name) \
    _mem_alloc_key_val(x, memb, size_key, size_val, name, __BASE_FILE__, __LINE__)

#define mem_realloc(x, size, name) \
    _mem_realloc(x, size, name, __BASE_FILE__, __LINE__)

#define mem_realloc_memb(x, memb, size, name) \
    _mem_realloc_memb(x, memb, size, name, __BASE_FILE__, __LINE__)

#define mem_free(x, size, name) \
    _mem_free(x, size, name, __BASE_FILE__, __LINE__)

#define mem_free_buf(x, name) \
    _mem_free_buf(x, name, __BASE_FILE__, __LINE__)

#define mem_free_key_val(x, name) \
    _mem_free_key_val(x, name, __BASE_FILE__, __LINE__)

#define mem_clear(x, size, name) \
    _mem_clear(x, size, name, __BASE_FILE__, __LINE__)

#define mem_map(x, size, name) \
    _mem_map(x, size, name, __BASE_FILE__, __LINE__)

#define mem_commit(x, offset, size, name) \
    _mem_commit(x, offset, size, name, __BASE_FILE__, __LINE__)

#define mem_unmap(x, size, name) \
    _mem_unmap(x, size, name, __BASE_FILE__, __LINE__)

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size = size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_alloc(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb = number of members.
 *  @param size = member size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_alloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb = number of members.
 *  @param size = member size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_alloc_buf(Buf *x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb = number of members per buffer.
 *  @param size_key = 'key' member size in bytes.
 *  @param size_val = 'val' member size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_alloc_key_val(KeyValue *x, u64 memb, u64 size_key, u64 size_val,
        const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size = size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_realloc(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb = number of members.
 *  @param size = member size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_realloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size = size in bytes.
 *  @param name = pointer name (for logging).
 */
void _mem_free(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param name = pointer name (for logging).
 */
void _mem_free_buf(Buf *x, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param name = pointer name (for logging).
 */
void _mem_free_key_val(KeyValue *x, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size = size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_clear(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief reserve a block of memory for '*x'.
 *
 *  @param size = size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_map(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief commit a block of mapped memory for '*x'.
 *
 *  @param size = size in bytes.
 *  @param name = pointer name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _mem_commit(void **x, void *offset, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief unmap a block of memory '*x'.
 *
 *  @oaram size = size in bytes.
 *  @oaram name = pointer name (for logging).
 */
void _mem_unmap(void **x, u64 size, const str *name, const str *file, u64 line);

void print_bits(u64 x, u8 bit_count);

/*! @brief swap bits of 'c1' and 's2' with each other.
 *
 *  @remark doesn't use a temp variable, swaps bits in-place.
 */
void swap_bits(char *c1, char *c2);

/*! @brief swap bits of 'a' and 'b' with each other.
 *
 *  @remark doesn't use a temp variable, swaps bits in-place.
 */
void swap_bits_u8(u8 *a, u8 *b);

/*! @brief swap bits of 'a' and 'b' with each other.
 *
 *  @remark doesn't use a temp variable, swaps bits in-place.
 */
void swap_bits_u32(u32 *a, u32 *b);

/*! @brief swap bits of 'a' and 'b' with each other.
 *
 *  @remark doesn't use a temp variable, swaps bits in-place.
 */
void swap_bits_u64(u64 *a, u64 *b);

void sort_buf(Buf *buf);

#endif /* ENGINE_MEMORY_H */
