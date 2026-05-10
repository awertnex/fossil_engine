/*  @file memory.h
 *
 *  @brief main memory module header; memory management.
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
 *  limitations under the License.
 */

#ifndef FSL_MEMORY_H
#define FSL_MEMORY_H

#include "../h/common.h"
#include "../h/limits.h"
#include "memory_types.h"

/* ---- section: tools ------------------------------------------------------ */

#define fsl_arr_len(x) \
    ((u64)sizeof(x) / sizeof(x[0]))

#define fsl_mem_handle_get(type, handle) \
    ((handle.arena) ? ((type*)((u8*)handle.arena->buf + handle.offset)) : NULL)

#define fsl_mem_handle_get_i(type, handle, index) \
    ((handle.arena) ? ((type*)((u8*)handle.arena->buf + handle.offset + index * sizeof(type))) : NULL)

/* ---- section: definitions ------------------------------------------------ */

#define FSL_OFFSET_INVALID FSL_U64_MAX

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

#define fsl_mem_arena_init(x, name) \
    _fsl_mem_arena_init(x, name, __BASE_FILE__, __LINE__)

#define fsl_mem_arena_push(arena, handle, size, name) \
    _fsl_mem_arena_push(arena, handle, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_arena_free(x, name) \
    _fsl_mem_arena_free(x, name, __BASE_FILE__, __LINE__)

/* ---- section: declarations ----------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief global memory arena, used to manage all heap memory inside and optionally
 *  outside the engine.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_internal;

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief global @ref fsl_asset.name memory arena, for storing runtime asset names.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_name_internal;

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief global @ref fsl_asset.name_id memory arena, for storing runtime asset internal names.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_name_id_internal;

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief global @ref fsl_asset.file memory arena, for storing runtime asset file names.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_file_internal;

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief global @ref fsl_asset.path memory arena, for storing runtime asset parent directories.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_path_internal;

/* ---- section: signatures ------------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_alloc(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members.
 *  @param size member size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_alloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members.
 *  @param size member size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_alloc_buf(fsl_buf *x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members per buffer.
 *  @param size_key `x->key` member size, in bytes.
 *  @param size_val `x->val` member size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_alloc_key_val(fsl_key_value *x, u64 memb, u64 size_key, u64 size_val,
        const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_realloc(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members.
 *  @param size member size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_realloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 */
FSLAPI void _fsl_mem_free(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param name symbol name (for logging).
 */
FSLAPI void _fsl_mem_free_buf(fsl_buf *x, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param name symbol name (for logging).
 */
FSLAPI void _fsl_mem_free_key_val(fsl_key_value *x, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_clear(void *x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief reserve a block of memory for `*x`.
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
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
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
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
 *  @param size_old old size, in bytes.
 *  @param size_new new size, in bytes.
 *  @param name symbol name (for logging).
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
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 */
FSLAPI void _fsl_mem_unmap(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief allocate and initialize a memory arena.
 *
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_arena_init(fsl_mem_arena *x, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief reserve a block of available memory in `x` and grow arena if needed and
 *  initialize `handle` metadata.
 *
 *  handle's metadata:
 *      - a reference to the arena.
 *      - the handle's allocated offset from the arena's base pointer.
 *      - the handle's allocated size.
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 _fsl_mem_arena_push(fsl_mem_arena *x, fsl_mem_handle *handle, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief free a memory arena.
 *
 *  @param name symbol name (for logging).
 */
FSLAPI void _fsl_mem_arena_free(fsl_mem_arena *x, const str *name, const str *file, u64 line);

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
