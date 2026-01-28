/*  Copyright 2026 Lily Awertnex
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

/*  memory.h - memory management
 */

#ifndef FSL_MEMORY_H
#define FSL_MEMORY_H

#include "common.h"
#include "limits.h"
#include "types.h"

typedef struct fsl_mem_arena
{
    void ***i;      /* members of `buf` */
    void *buf;      /* raw data */
    u64 memb;       /* number of `i` members */
    u64 size_i;     /* total mapped size for `i` in bytes */
    u64 size_buf;   /* total mapped size for `buf` in bytes */
    u64 cursor;     /* current usage */
} fsl_mem_arena;

#define fsl_arr_len(x) ((u64)sizeof(x) / sizeof(x[0]))

#define fsl_mem_alloc(x, size, name) \
    _fsl_mem_alloc(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_alloc_memb(x, memb, size, name) \
    _fsl_mem_alloc_memb(x, memb, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_alloc_buf(x, memb, size, name) \
    _fsl_mem_alloc_buf(x, memb, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_alloc_key_val(x, memb, size_key, size_val, name) \
    _fsl_mem_alloc_key_val(x, memb, size_key, size_val, name, __BASE_FILE__, __LINE__)

#define fsl_mem_realloc(x, size, name) \
    _fsl_mem_realloc(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_realloc_memb(x, memb, size, name) \
    _fsl_mem_realloc_memb(x, memb, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_free(x, size, name) \
    _fsl_mem_free(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_free_buf(x, name) \
    _fsl_mem_free_buf(x, name, __BASE_FILE__, __LINE__)

#define fsl_mem_free_key_val(x, name) \
    _fsl_mem_free_key_val(x, name, __BASE_FILE__, __LINE__)

#define fsl_mem_clear(x, size, name) \
    _fsl_mem_clear(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_map(x, size, name) \
    _fsl_mem_map(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_commit(x, offset, size, name) \
    _fsl_mem_commit(x, offset, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_remap(x, size_old, size_new, name) \
    _fsl_mem_remap(x, size_old, size_new, name, __BASE_FILE__, __LINE__)

#define fsl_mem_unmap(x, size, name) \
    _fsl_mem_unmap(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_map_arena(x, size, name) \
    _fsl_mem_map_arena(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_push_arena(arena, p, size, name) \
    _fsl_mem_push_arena(arena, p, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_unmap_arena(x, name) \
    _fsl_mem_unmap_arena(x, name, __BASE_FILE__, __LINE__)

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief global page size variable.
 *
 *  initialized once in any of:
 *      @ref _fsl_mem_map().
 *      @ref _fsl_mem_remap().
 *      @ref _fsl_mem_commit().
 *      @ref _fsl_mem_map_arena().
 *      @ref _fsl_mem_remap_arena().
 *      @ref _fsl_mem_push_arena().
 */
extern u64 FSL_PAGE_SIZE;

/*! @brief like @ref fsl_round_up_u64() but only works on powers of two for `size`.
 */
FSLAPI u64 fsl_align_up_u64(u64 n, u64 size);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief initialize @ref FSL_PAGE_SIZE if not initialized.
 */
void fsl_mem_request_page_size(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief request memory page size for platform.
 *
 *  @return page size in bytes.
 */
u64 _fsl_mem_request_page_size(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_alloc(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members.
 *  @param size member size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_alloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members.
 *  @param size member size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_alloc_buf(fsl_buf *x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members per buffer.
 *  @param size_key `x->key` member size in bytes.
 *  @param size_val `x->val` member size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_alloc_key_val(fsl_key_value *x, u64 memb, u64 size_key, u64 size_val,
        const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_realloc(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members.
 *  @param size member size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_realloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 */
FSLAPI void _fsl_mem_free(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param name pointer name (for logging).
 */
FSLAPI void _fsl_mem_free_buf(fsl_buf *x, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param name pointer name (for logging).
 */
FSLAPI void _fsl_mem_free_key_val(fsl_key_value *x, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_clear(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief reserve a block of memory for `*x`.
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_map(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief commit a block of mapped memory for `*x`.
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_commit(void **x, void *offset, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief remap a block of memory for `*x`.
 *  
 *  @param size_old old size in bytes.
 *  @param size_new new size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_remap(void **x, u64 size_old, u64 size_new, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief unmap a block of memory `*x`.
 *
 *  @oaram size size in bytes.
 *  @oaram name pointer name (for logging).
 */
FSLAPI void _fsl_mem_unmap(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief reserve a memory arena for `x`.
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_map_arena(fsl_mem_arena *x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief push a block of mapped memory for `*p` hosted by available space in `x` and grow arena if needed.
 *
 *  @oaram size size in bytes.
 *  @oaram name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_push_arena(fsl_mem_arena *x, void **p, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief unmap a memory arena `x`.
 *
 *  @oaram name pointer name (for logging).
 */
FSLAPI void _fsl_mem_unmap_arena(fsl_mem_arena *x, const str *name, const str *file, u64 line);

/*! @brief similar to `printf("%b\n", x)` but only output `bit_count` bits.
 */
FSLAPI void fsl_print_bits(u64 x, u8 bit_count);

/*! @brief swap bits of `c1` and `s2` with each other (without a `temp` variable).
 */
FSLAPI void fsl_swap_bits(char *c1, char *c2);

/*! @brief swap bits of 'a' and 'b' with each other (without a `temp` variable).
 */
FSLAPI void fsl_swap_bits_u8(u8 *a, u8 *b);

/*! @brief swap bits of 'a' and 'b' with each other (without a `temp` variable).
 */
FSLAPI void fsl_swap_bits_u32(u32 *a, u32 *b);

/*! @brief swap bits of 'a' and 'b' with each other (without a `temp` variable).
 */
FSLAPI void fsl_swap_bits_u64(u64 *a, u64 *b);

FSLAPI void fsl_sort_buf(fsl_buf *buf);

#endif /* FSL_MEMORY_H */
