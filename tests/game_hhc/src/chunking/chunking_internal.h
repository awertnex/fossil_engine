#ifndef HHC_CHUNKING_INTERNAL_H
#define HHC_CHUNKING_INTERNAL_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/math/vector.h"

#include "../h/common.h"

#include "chunk_scheduler.h"
#include "chunking.h"

/* ---- section: definitions ------------------------------------------------ */

/*!
 *  @brief count of temporary static buffers in function @ref chunk_mesh_update_internal().
 */
#define BLOCK_BUFFERS_MAX 2

/*!
 *  @remark an entry for each render distance, one u32 for offset and one for size.
 */
#define CHUNK_ORDER_LOOKUP_OFFSET_TABLE_SIZE ((SET_RENDER_DISTANCE_MAX + 1) * sizeof(u32) * 2)

/* ---- section: block flag ------------------------------------------------- */

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000001 00000000 00000000] 00; */
#define FLAG_BLOCK_FACE_PX      0x0000000000010000

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000010 00000000 00000000] 00; */
#define FLAG_BLOCK_FACE_NX      0x0000000000020000

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000100 00000000 00000000] 00; */
#define FLAG_BLOCK_FACE_PY      0x0000000000040000

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00001000 00000000 00000000] 00; */
#define FLAG_BLOCK_FACE_NY      0x0000000000080000

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00010000 00000000 00000000] 00; */
#define FLAG_BLOCK_FACE_PZ      0x0000000000100000

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00100000 00000000 00000000] 00; */
#define FLAG_BLOCK_FACE_NZ      0x0000000000200000

/*!
 *  @brief run-length encoding, for chunk serialization.
 *
 *  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 10000000 00000000] 00; */
#define FLAG_BLOCK_RLE          0x0000000000008000

/* ---- section: chunk ------------------------------------------------------ */

enum chunk_shift_state
{
    STATE_CHUNK_SHIFT_PX = 1,
    STATE_CHUNK_SHIFT_NX = 2,
    STATE_CHUNK_SHIFT_PY = 3,
    STATE_CHUNK_SHIFT_NY = 4,
    STATE_CHUNK_SHIFT_PZ = 5,
    STATE_CHUNK_SHIFT_NZ = 6
}; /* chunk_shift_state */

/*!
 *  @brief chunk buffer, raw chunk data.
 */
typedef struct hhc_chunk_buffer
{
    /*!
     *  @brief position of first empty slot in `p`.
     */
    u64 cursor;

    fsl_mem_handle handle;
    hhc_chunk *p;           /* cached pointer from `handle` */
} hhc_chunk_buffer;

/*!
 *  @brief schedule of chunks to be processed.
 */
struct hhc_chunk_scheduler
{
    chunk_scheduler_id id;  /* scheduler ID */
    fsl_len count;          /* number of chunks scheduled */
    u32 offset;             /* offset of scheduler into @ref chunk_order.p */
    fsl_len len;            /* number of members in `p` */
    u32 cursor_push;        /* push position */
    u32 cursor_pop;         /* pop position */
    chunk_scheduler_budget budget;
    fsl_mem_handle schedule;
    hhc_chunk **p;        /* cached pointer from `schedule` */
}; /* hhc_chunk_scheduler */

/* ---- section: declarations ----------------------------------------------- */

extern hhc_chunk_scheduler chunk_sched[CHUNK_SCHEDULERS_MAX];

/* ---- section: signatures ------------------------------------------------- */

u32 chunks_max_init_internal(void);

/*!
 *  @brief initialize @ref chunk_order.
 *
 *  write lookup to disk, and load if exists.
 *
 *  format:
 *      offset-table:   offset of each lookup into file, after compression, in bytes,
 *      |               size of table is "(SET_RENDER_DISTANCE_MAX + 1) * 4 bytes * 2".
 *      | - entries:    4 bytes for offset, 4 bytes for size, after compression, in bytes.
 *
 *      data:           a flat u32 array of index values.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 chunk_order_init_internal(void);

/*!
 *  @brief initialize resources required by chunk debug tools.
 */
u32 chunk_debug_init_internal(fsl_len chunk_count);

void chunk_debug_free_internal(void);

/*!
 *  @brief get block faces based on neighboring blocks.
 *
 *  @return block with modified faces.
 */
u32 block_get_faces_internal(const hhc_chunk *ch,
        const hhc_chunk *px, const hhc_chunk *nx,
        const hhc_chunk *py, const hhc_chunk *ny,
        const hhc_chunk *pz, const hhc_chunk *nz,
        i32 x, i32 y, i32 z);

void block_add_internal(hhc_chunk *ch,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z, enum block_id block_id);

void block_remove_internal(hhc_chunk *ch,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z);

/*!
 *  @brief execute block logic on the block based on its ID (e.g., make grass
 *  turn to dirt when under another block).
 */
void block_evaluate_internal(hhc_chunk *ch,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z, enum block_id block_id);

/*!
 *  @brief generate chunk blocks.
 *
 *  @remark calls @ref chunk_mesh_update_internal() when done generating.
 *  @remark must be called before @ref chunk_mesh_update_internal().
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_load_internal(hhc_chunk *ch, chunk_scheduler_budget budget);

/*!
 *  @brief generate chunk blocks.
 *
 *  automatically called from @ref chunk_load_internal().
 *
 *  @remark calls @ref chunk_mesh_update_internal() when done generating.
 *  @remark must be called before @ref chunk_mesh_update_internal().
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_generate_internal(hhc_chunk *ch, chunk_scheduler_budget budget);

/*!
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_mesh_update_internal(hhc_chunk *ch);

/*!
 *  @brief write chunk into disk.
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_export_internal(hhc_chunk *ch);

/*!
 *  @brief read chunk from disk.
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_import_internal(const fsl_fs_path *path, hhc_chunk *ch);

void chunk_buf_push_internal(u32 index, v3i32 player_chunk_delta);
void chunk_buf_pop_internal(hhc_chunk *ch);

/*!
 *  @brief initialize chunk scheduler resources.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 chunk_scheduler_init_internal(hhc_chunk_scheduler *sched, chunk_scheduler_id id,
        u64 offset, chunk_scheduler_radius radius, chunk_scheduler_budget budget);
/*!
 *  @param len number of chunks from @ref chunk_order.p this scheduler is allowed to parse.
 */
void chunk_scheduler_update_internal(hhc_chunk_scheduler *sched, fsl_len len,
        b8 should_push, b8 should_pop);

void chunk_debug_chunk_gizmo_write_internal(hhc_chunk *ch);

#endif /* HHC_CHUNKING_INTERNAL_H */
