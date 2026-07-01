#ifndef HHC_CHUNKING_INTERNAL_H
#define HHC_CHUNKING_INTERNAL_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/math/vector.h"
#include "deps/fossil/plugins/fsl_native/noise_sampler/noise_sampler.h"

#include "chunk_work.h"
#include "chunking.h"

/* ---- section: definitions ------------------------------------------------ */

/*!
 *  @brief count of temporary static buffers in function @ref chunk_mesh_update_internal().
 */
#define BLOCK_BUFFERS_MAX 2

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
    u32 cursor;

    fsl_mem_handle handle;
    hhc_chunk *p;           /* cached pointer from `handle` */
} hhc_chunk_buffer;

/*!
 *  @brief a chunk and all six neighbors surrounding it.
 */
typedef struct hhc_chunk_neighbors
{
    hhc_chunk *ch, *px, *nx, *py, *ny, *pz, *nz;
} hhc_chunk_neighbors;

/*!
 *  @brief chunk-scheduler bucket for a unique distance away from @ref chunk_tab center index.
 */
typedef struct hhc_chunk_bucket
{
    u32 count;  /* number of chunks scheduled */
    u32 pos;    /* start position of bucket into @ref chunk_order.p */
    u32 len;    /* total number of slots in bucket */
} hhc_chunk_bucket;

/*!
 *  @brief schedule of chunks to be processed.
 */
typedef struct hhc_chunk_scheduler
{
    fsl_len count;          /* number of chunks scheduled */

    u32 cursor_push;        /* push position */
    u32 cursor_pop;         /* pop position */
    fsl_mem_handle handle_p;
    fsl_mem_handle handle_bucket;
    hhc_chunk **p;          /* cached pointer from `schedule` */
    hhc_chunk_bucket *bucket; /* cached pointer from `schedule` */
    u32 buckets_max;        /* total number of members in `bucket` */
    u32 priority;           /* current parsing priority */
} hhc_chunk_scheduler;

typedef struct hhc_chunk_sampler
{
    fsl_noise_sampler sampler;
    fsl_noise_sampler_context context;
} hhc_chunk_sampler;

/* ---- section: declarations ----------------------------------------------- */

extern hhc_chunk_scheduler chunk_sched;

/* ---- section: signatures ------------------------------------------------- */

u32 chunks_max_init_internal(void);

/*!
 *  @brief initialize @ref chunk_order.
 *
 *  write lookup to disk, and load if exists.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 chunk_order_init_internal(void);

/*!
 *  @brief build @ref chunk_order look-up and write to disk.
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
u32 chunk_order_build_internal(void);

/*!
 *  @brief load @ref chunk_order look-up from disk.
 *
 *  can be called again when changing render distance.
 *
 *  @param render_distance current render distance to use in index-adjustment.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 chunk_order_load_internal(u32 render_distance);

/*!
 *  @brief load @ref chunk_sched.bucket look-up from disk.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 chunk_bucket_load_internal(void);

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
u32 block_faces_get_internal(hhc_chunk_neighbors *chunk_neighbors, i32 x, i32 y, i32 z);

void block_add_internal(hhc_chunk_neighbors *chunk_neighbors, i32 x, i32 y, i32 z,
        enum block_id block_id);

void block_remove_internal(hhc_chunk_neighbors *chunk_neighbors, i32 x, i32 y, i32 z);

/*!
 *  @brief execute block logic on the block based on its ID (e.g., make grass
 *  turn to dirt when under another block).
 */
void block_evaluate_internal(hhc_chunk_neighbors *chunk_neighbors,
        i32 x, i32 y, i32 z, enum block_id block_id);

/*!
 *  @brief get radius of sphere squared as per internal conventions.
 */
u32 chunk_sphere_radius_get_internal(u32 radius);

/*!
 *  @brief set new chunk position.
 *
 *  set chunk position and wrapped position.
 *  set chunk ID, cti and cpi.
 */
void chunk_pos_set_internal(hhc_chunk *chunk,
        v3i32 player_chunk_delta, v3u32 chunk_tab_coordinates);

/*!
 *  @brief generate chunk blocks.
 *
 *  @remark calls @ref chunk_mesh_update_internal() when done generating.
 *  @remark must be called before @ref chunk_mesh_update_internal().
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_load_internal(hhc_chunk *chunk, chunk_work_budget budget);

hhc_chunk_neighbors chunk_neighbors_get_internal(hhc_chunk *chunk);

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
chunk_work_cost chunk_generate_internal(hhc_chunk *chunk, chunk_work_budget budget);

/*!
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_mesh_update_internal(hhc_chunk *chunk);

/*!
 *  @brief write chunk into disk.
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_export_internal(hhc_chunk *chunk);

/*!
 *  @brief read chunk from disk.
 *
 *  @return cost of operation (used in @ref chunk_scheduler_update_internal()).
 */
chunk_work_cost chunk_import_internal(const fsl_fs_path *path, hhc_chunk *chunk);

void chunk_buf_update_internal(v3i32 *player_chunk_delta);
void chunk_buf_push_internal(u32 index, v3i32 player_chunk_delta);
void chunk_buf_pop_internal(hhc_chunk *chunk);
void chunk_scheduler_update_internal_deprecated(void);
void chunk_scheduler_update_internal(void);
chunk_work_cost chunk_scheduler_push_internal(hhc_chunk *chunk);
chunk_work_cost chunk_scheduler_pop_internal(u32 index);

void chunk_debug_chunk_gizmo_write_internal(hhc_chunk *chunk);

#endif /* HHC_CHUNKING_INTERNAL_H */
