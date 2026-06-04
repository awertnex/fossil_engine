#ifndef HHC_CHUNKING_INTERNAL_H
#define HHC_CHUNKING_INTERNAL_H

#include "deps/fossil/common/common.h"

/* ---- section: definitions ------------------------------------------------ */

#define CHUNK_COLOR_LOADED  fsl_color_v4_to_hex(0.70f, 0.01f, 0.02f, 0.39f)
#define CHUNK_COLOR_RENDER  fsl_color_v4_to_hex(0.24f, 0.47f, 0.3f, 1.0f)
#define CHUNK_COLOR_FACTOR_INFLUENCE 0.1

/*!
 *  @brief count of temporary static buffers in function @ref chunk_mesh_update_internal().
 */
#define BLOCK_BUFFERS_MAX       2

/* ---- section: chunk scheduler config ------------------------------------- */

/*!
 *  @brief budget of work given to each @ref chunk_scheduler to be consumed each frame.
 */
#define CHUNK_PARSE_RATE_PRIORITY_LOW 16384
#define CHUNK_PARSE_RATE_PRIORITY_MID 32768
#define CHUNK_PARSE_RATE_PRIORITY_HIGH 65536

/*!
 *  @brief number of blocks to process per chunk per frame.
 */
#define BLOCK_PARSE_RATE 512

/*!
 *  @brief cost of work requested by chunks in a @ref chunk_scheduler.
 */
typedef enum chunk_scheduler_cost
{
    CHUNK_PARSE_COST_PUSH = 4,
    CHUNK_PARSE_COST_POP = 4,
    CHUNK_PARSE_COST_IMPORT = 16,
    CHUNK_PARSE_COST_EXPORT = 16,
    CHUNK_PARSE_COST_MESHING = 64,
    CHUNK_PARSE_COST_GENERATION = 256
} chunk_scheduler_cost;

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
typedef struct chunk_buffer
{
    /*!
     *  @brief position of first empty slot in `p`.
     */
    u64 cursor;

    fsl_mem_handle handle;
    hhc_chunk *p;           /* cached pointer from `handle` */
} chunk_buffer;

/* ---- section: signatures ------------------------------------------------- */

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
 *  @param rate number of blocks to process per chunk per frame.
 *
 *  @remark calls @ref chunk_mesh_update_internal() when done generating.
 *  @remark must be called before @ref chunk_mesh_update_internal().
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_scheduler_cost chunk_load_internal(hhc_chunk *ch, u32 rate);

/*!
 *  @brief generate chunk blocks.
 *
 *  automatically called from @ref chunk_load_internal().
 *
 *  @param rate number of blocks to process per chunk per frame.
 *
 *  @remark calls @ref chunk_mesh_update_internal() when done generating.
 *  @remark must be called before @ref chunk_mesh_update_internal().
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_scheduler_cost chunk_generate_internal(hhc_chunk *ch, u32 rate);

/*!
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_scheduler_cost chunk_mesh_update_internal(hhc_chunk *ch);

/*!
 *  @brief write chunk into disk.
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_scheduler_cost chunk_export_internal(hhc_chunk *ch);

/*!
 *  @brief read chunk from disk.
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_scheduler_cost chunk_import_internal(const fsl_fs_path *path, hhc_chunk *ch);

void chunk_buf_push_internal(u32 index, v3i32 player_chunk_delta);
void chunk_buf_pop_internal(hhc_chunk *ch);

void chunk_scheduler_set_parameters_internal(chunk_scheduler *sched, chunk_scheduler_id id,
        u64 offset, fsl_len len, u32 rate_chunk);
/*!
 *  @param len number of chunks from @ref chunk_order.p this scheduler is allowed to parse.
 */
void chunk_scheduler_update_internal(chunk_scheduler *sched, fsl_len len,
        b8 should_push, b8 should_pop);

void chunk_gizmo_write_internal(hhc_chunk *ch);

#endif /* HHC_CHUNKING_INTERNAL_H */
