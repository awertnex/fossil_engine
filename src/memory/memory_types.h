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
 *  @file memory_types.h
 *
 *  @brief memory definitions and data structures.
 */

#ifndef FSL_MEMORY_TYPES_H
#define FSL_MEMORY_TYPES_H

#include "../h/types.h"

/*!
 *  @brief offset specific to memory arena pushes, it is a memory offset from
 *  an arena's base pointer.
 */
typedef u64 fsl_off;

typedef struct mem_handle fsl_mem_handle;
typedef struct mem_arena_handle fsl_mem_arena_handle;
typedef struct mem_arena fsl_mem_arena;

/*!
 *  @remark retrieve allocation address using @ref fsl_mem_handle_get() or @ref fsl_mem_handle_get_i().
 */
struct mem_handle
{
    fsl_mem_arena *arena;   /* address of arena the handle was allocated on */
    fsl_off offset;         /* handle's data offset from arena start (@ref mem_arena.buf) */
    u64 size;               /* handle's allocated size */
}; /* mem_handle */

struct mem_arena
{
    fsl_mem_arena_handle *entry; /* allocated zones in `buf` */
    u64 entry_cap;      /* current capacity of `entry` in bytes */
    u64 entry_count;    /* entry count */

    fsl_mem_arena_handle *freelist; /* unallocated zones in `buf` */
    u64 freelist_cap;   /* current capacity of `freelist` in bytes */

    void *buf;          /* raw data */
    u64 buf_cap;        /* current capacity of `buf` in bytes */
    fsl_off buf_cursor; /* -- DEPRECATED IN v0.8.0-beta --; current usage */
}; /* mem_arena */

#endif /* FSL_MEMORY_TYPES_H */
