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
 *  @file memory.h
 *
 *  @brief main memory module header; memory management.
 */

#ifndef FSL_MEMORY_H
#define FSL_MEMORY_H

#include "../common/engine_info.h"
#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "memory_types.h"

/* ---- section: tools ------------------------------------------------------ */

#define fsl_arr_len(x) \
    ((u64)sizeof(x) / sizeof(x[0]))

#define fsl_mem_handle_get(type, handle) \
    (handle.arena ? (type*)((u8*)handle.arena->buf + handle.offset) : NULL)

#define fsl_mem_handle_get_i(type, handle, index) \
    (handle.arena ? (type*)((u8*)handle.arena->buf + handle.offset + (index) * sizeof(type)) : NULL)

/* ---- section: definitions ------------------------------------------------ */

#define FSL_OFFSET_INVALID FSL_U64_MAX

#define fsl_mem_alloc(x, size, name) \
    fsl_mem_alloc_internal(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_alloc_memb(x, memb, size, name) \
    fsl_mem_alloc_memb_internal(x, memb, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_alloc_buf(x, memb, size, name) \
    fsl_mem_alloc_buf_internal(x, memb, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_alloc_key_val(x, memb, size_key, size_val, name) \
    fsl_mem_alloc_key_val_internal(x, memb, size_key, size_val, name, __BASE_FILE__, __LINE__)

#define fsl_mem_realloc(x, size, name) \
    fsl_mem_realloc_internal(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_realloc_memb(x, memb, size, name) \
    fsl_mem_realloc_memb_internal(x, memb, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_free(x, size, name) \
    fsl_mem_free_internal(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_free_buf(x, name) \
    fsl_mem_free_buf_internal(x, name, __BASE_FILE__, __LINE__)

#define fsl_mem_free_key_val(x, name) \
    fsl_mem_free_key_val_internal(x, name, __BASE_FILE__, __LINE__)

#define fsl_mem_clear(x, size, name) \
    fsl_mem_clear_internal(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_map(x, size, name) \
    fsl_mem_map_internal(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_commit(x, offset, size, name) \
    fsl_mem_commit_internal(x, offset, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_remap(x, size_old, size_new, name) \
    fsl_mem_remap_internal(x, size_old, size_new, name, __BASE_FILE__, __LINE__)

#define fsl_mem_unmap(x, size, name) \
    fsl_mem_unmap_internal(x, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_arena_init(x, name) \
    fsl_mem_arena_init_internal(x, name, __BASE_FILE__, __LINE__)

#define fsl_mem_arena_push(arena, handle, size, name) \
    fsl_mem_arena_push_internal(arena, handle, size, name, __BASE_FILE__, __LINE__)

#define fsl_mem_arena_pop(handle, name) \
    fsl_mem_arena_pop_internal(handle, name, __BASE_FILE__, __LINE__)

#define fsl_mem_arena_free(x, name) \
    fsl_mem_arena_free_internal(x, name, __BASE_FILE__, __LINE__)

/* ---- section: declarations ----------------------------------------------- */

/*!
 *  @internal
 *
 *  @brief engine's global memory arena, used to manage all heap memory inside
 *  and optionally outside the engine.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_internal;

/*!
 *  @internal
 *
 *  @brief engine's global @ref fsl_asset.name memory arena, for storing runtime asset names.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_name_internal;

/*!
 *  @internal
 *
 *  @brief engine's global @ref fsl_asset.name_id memory arena, for storing runtime asset internal names.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_name_id_internal;

/*!
 *  @internal
 *
 *  @brief engine's global @ref fsl_asset.file memory arena, for storing runtime asset file names.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_file_internal;

/*!
 *  @internal
 *
 *  @brief engine's global @ref fsl_asset.path memory arena, for storing runtime asset parent directories.
 *
 *  initialized in @ref fsl_engine_init().
 */
extern fsl_mem_arena mem_arena_path_internal;

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_alloc_internal(void **x, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @param memb number of members.
 *  @param size member size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_alloc_memb_internal(void **x, u64 memb, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @param memb number of members.
 *  @param size member size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_alloc_buf_internal(fsl_buf *x, u64 memb, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @param memb number of members per buffer.
 *  @param size_key `x->key` member size, in bytes.
 *  @param size_val `x->val` member size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_alloc_key_val_internal(fsl_key_value *x, u64 memb, u64 size_key, u64 size_val,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_realloc_internal(void **x, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @param memb number of members.
 *  @param size member size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_realloc_memb_internal(void **x, u64 memb, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 */
FSLAPI void fsl_mem_free_internal(void **x, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @param name symbol name (for logging).
 */
FSLAPI void fsl_mem_free_buf_internal(fsl_buf *x, const str *name,
        const str *src_file, u64 src_line);

/*!
 *  @param name symbol name (for logging).
 */
FSLAPI void fsl_mem_free_key_val_internal(fsl_key_value *x,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_clear_internal(void *x, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief reserve a block of memory for `*x`.
 *  implemented in `platform_<PLATFORM>.c`.
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_map_internal(void **x, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief commit a block of mapped memory for `*x`.
 *  implemented in `platform_<PLATFORM>.c`.
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_commit_internal(void **x, void *offset, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief remap a block of memory for `*x`.
 *  implemented in `platform_<PLATFORM>.c`.
 *
 *  @param size_old old size, in bytes.
 *  @param size_new new size, in bytes.
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_remap_internal(void **x, u64 size_old, u64 size_new,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief unmap a block of memory `*x`.
 *  implemented in `platform_<PLATFORM>.c`.
 *
 *  @param size size, in bytes.
 *  @param name symbol name (for logging).
 */
FSLAPI void fsl_mem_unmap_internal(void **x, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief allocate and initialize a memory arena.
 *
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_arena_init_internal(fsl_mem_arena *x,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief reserve a block of available memory in arena `x` and grow arena if needed and
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
FSLAPI u32 fsl_mem_arena_push_internal(fsl_mem_arena *x, fsl_mem_handle *handle, u64 size,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief reserve a block of available memory in arena `x` and grow arena if needed and
 *  initialize `handle` metadata.
 *
 *  @brief remove a memory handle from arena `x` and invalidate it.
 *
 *  @param name symbol name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mem_arena_pop_internal(fsl_mem_handle *handle,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief free a memory arena.
 *  implemented in `platform_<PLATFORM>.c`.
 *
 *  @param name symbol name (for logging).
 */
FSLAPI void fsl_mem_arena_free_internal(fsl_mem_arena *x,
        const str *name, const str *src_file, u64 src_line);

/*!
 *  @brief similar to 'printf("%b\n", x)' but only output `bit_count` bits.
 */
FSLAPI void fsl_print_bits(u64 x, u8 bit_count);

/*!
 *  @brief swap bits of `c1` and `s2` with each other (without a `temp` variable).
 */
FSLAPI void fsl_swap_bits(char *c1, char *c2);

/*!
 *  @brief swap bits of 'a' and 'b' with each other (without a `temp` variable).
 */
FSLAPI void fsl_swap_bits_u8(u8 *a, u8 *b);

/*!
 *  @brief swap bits of 'a' and 'b' with each other (without a `temp` variable).
 */
FSLAPI void fsl_swap_bits_u32(u32 *a, u32 *b);

/*!
 *  @brief swap bits of 'a' and 'b' with each other (without a `temp` variable).
 */
FSLAPI void fsl_swap_bits_u64(u64 *a, u64 *b);

FSLAPI void fsl_sort_buf(fsl_buf *buf);

#endif /* FSL_MEMORY_H */
