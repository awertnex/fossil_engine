#ifndef HHC_CHUNKING_H
#define HHC_CHUNKING_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/math/vector.h"

#include "../h/assets.h"
#include "../h/common.h"
#include "../h/raycast.h"

#define CHUNK_DIAMETER  16
#define CHUNK_LAYER     (CHUNK_DIAMETER * CHUNK_DIAMETER)
#define CHUNK_VOLUME    (CHUNK_DIAMETER * CHUNK_DIAMETER * CHUNK_DIAMETER)

#define CHUNK_REGION_DIAMETER   32
#define CHUNK_REGION_VOLUME     (CHUNK_REGION_DIAMETER * CHUNK_REGION_DIAMETER * CHUNK_REGION_DIAMETER)

#define WORLD_RADIUS            (2048 * CHUNK_DIAMETER)
#define WORLD_RADIUS_VERTICAL   (64 * CHUNK_DIAMETER)

#define WORLD_DIAMETER          (WORLD_RADIUS * 2 + CHUNK_DIAMETER)
#define WORLD_DIAMETER_VERTICAL (WORLD_RADIUS_VERTICAL * 2 + CHUNK_DIAMETER)
#define WORLD_CHUNKS_MAX        ((WORLD_DIAMETER * WORLD_DIAMETER * WORLD_DIAMETER_VERTICAL) / CHUNK_DIAMETER)

#define CHUNK_BUF_RADIUS_MAX    SET_RENDER_DISTANCE_MAX
#define CHUNK_BUF_DIAMETER_MAX  (CHUNK_BUF_RADIUS_MAX * 2 + 1)
#define CHUNK_BUF_LAYER_MAX     (CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX)
#define CHUNK_BUF_VOLUME_MAX    (CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX)

/* ---- section: chunk scheduler -------------------------------------------- */

typedef enum chunk_scheduler_id
{
    CHUNK_SCHEDULER_ID_NONE,
    CHUNK_SCHEDULER_ID_1ST,
    CHUNK_SCHEDULER_ID_2ND,
    CHUNK_SCHEDULER_ID_3RD
} chunk_scheduler_id;

#define CHUNK_SCHEDULER_ID_LAST CHUNK_SCHEDULER_3RD_ID

enum chunk_scheduler_len
{
    CHUNK_SCHEDULER_1ST_MAX = 256,
    CHUNK_SCHEDULER_2ND_MAX = 4096,
    CHUNK_SCHEDULER_3RD_MAX = 16384,
    CHUNK_SCHEDULERS_MAX = 3
}; /* chunk_scheduler_len */

/* ---- section: block mask ------------------------------------------------- */

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00111111 11111111] 00; */
#define MASK_BLOCK_DATA         0x0000000000003fff

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00000011 11111111] 00; */
#define MASK_BLOCK_ID           0x00000000000003ff

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00000000 00111100 00000000] 00; */
#define MASK_BLOCK_STATE        0x0000000000003c00

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00000000 00111111 00000000 00000000] 00; */
#define MASK_BLOCK_FACES        0x00000000003f0000

/*  63 [00000000 00000000 00000000 00000000] 32;
 *  31 [00111111 00000000 00000000 00000000] 00; */
#define MASK_BLOCK_LIGHT        0x000000003f000000

/*  63 [00000000 00000000 00001111 11111111] 32;
 *  31 [00000000 00000000 00000000 00000000] 00; */
#define MASK_BLOCK_COORDINATES  0x00000fff00000000

/*  63 [00000000 00000000 00000000 00001111] 32;
 *  31 [00000000 00000000 00000000 00000000] 00; */
#define MASK_BLOCK_X            0x0000000f00000000

/*  63 [00000000 00000000 00000000 11110000] 32;
 *  31 [00000000 00000000 00000000 00000000] 00; */
#define MASK_BLOCK_Y            0x000000f000000000

/*  63 [00000000 00000000 00001111 00000000] 32;
 *  31 [00000000 00000000 00000000 00000000] 00; */
#define MASK_BLOCK_Z            0x00000f000000000

enum block_shift
{
    SHIFT_BLOCK_DATA =          0,
    SHIFT_BLOCK_ID =            0,
    SHIFT_BLOCK_STATE =         10,
    SHIFT_BLOCK_FACES =         16,
    SHIFT_BLOCK_LIGHT =         24,
    SHIFT_BLOCK_COORDINATES =   32,
    SHIFT_BLOCK_X =             32,
    SHIFT_BLOCK_Y =             36,
    SHIFT_BLOCK_Z =             40
}; /* block_shift */

enum chunk_flag
{
    FLAG_CHUNK_LOADED =     0x01,
    FLAG_CHUNK_DIRTY =      0x02,
    FLAG_CHUNK_GENERATED =  0x04,
    FLAG_CHUNK_RENDER =     0x08,
    FLAG_CHUNK_IMPORTED =   0x10,

    /*!
     *  @brief chunk marking for @ref chunk_tab shifting logic.
     */
    FLAG_CHUNK_EDGE =       0x20
}; /* chunk_flag */

typedef struct hhc_chunk
{
    u8 flag; /* enum: chunk_flag */
    v3i16 pos; /* world position / @ref CHUNK_DIAMETER */

    /*!
     *  @brief chunk's unique id derived from its position.
     *
     * format:
     * (pos.x & 0xffff) << 0x00 |
     * (pos.y & 0xffff) << 0x10 |
     * (pos.z & 0xffff) << 0x20.
     */
    u64 id;

    /*!
     *  @brief debug color.
     *
     *  format: 0xrrggbbaa.
     */
    u32 color;

    /*!
     *  @brief debug color variant.
     *
     *  used as offset for 'color'.
     *
     *  format: 0xrrggbbaa.
     */
    u32 color_variant;

    /*!
     *  @brief block iterator for per-chunk generation progress.
     */
    u32 cursor;

    /*!
     *  @brief chunk's own index in @ref chunk_table.p.
     */
    u32 index;

    /*!
     *  @brief ID of @ref chunk_scheduler that scheduled this chunk.
     */
    u32 sched_id;

    GLuint vao;
    GLuint vbo;
    u64 vbo_len;

    u32 block[CHUNK_DIAMETER][CHUNK_DIAMETER][CHUNK_DIAMETER];
} hhc_chunk;

/*!
 *  @brief chunk pointer look-up table that points to @ref chunk_buffer.p addresses.
 *
 *  @ref chunk_buffer.p addresses ordered by their positions in 3d space relative to player position.
 */
typedef struct chunk_table
{
    /*!
     *  @brief player-relative `p` access.
     */
    u32 index;

    fsl_mem_handle handle;
    hhc_chunk **p;          /* cached pointer from `handle` */
} chunk_table;

/*!
 *  @brief chunk pointer pointer look-up table that points to @ref chunk_table.p addresses.
 *
 *  @ref chunk_table.p addresses ordered by distance from @ref chunk_tab center in ascending order.
 */
typedef struct chunk_order
{
    fsl_mem_handle handle;
    hhc_chunk ***p;
} chunk_order;

/*!
 *  @brief schedule of chunks to be processed.
 */
typedef struct chunk_scheduler
{
    chunk_scheduler_id id;  /* scheduler ID */
    fsl_len count;          /* number of chunks scheduled */
    u32 offset;             /* offset of scheduler into @ref chunk_order.p */
    fsl_len len;            /* number of members in `p` */
    u32 cursor_push;        /* push position */
    u32 cursor_pop;         /* pop position */
    u32 rate_chunk;         /* number of chunks to process per frame */
    u32 rate_block;         /* number of blocks to process per chunk */
    fsl_mem_handle schedule;
    hhc_chunk **p;        /* cached pointer from `scheduler` */
} chunk_scheduler;

/*!
 *  @brief chunk gizmo render buffer data for chunk colors.
 *
 *  for rendering chunk gizmo in one draw call.
 *
 *  format: 0xxxyyzz00, 0xrrggbbaa.
 */
typedef struct chunk_gizmo
{
    b8 initialized;
    GLuint vao;
    GLuint vbo;
    fsl_mem_handle handle;
    v2u32 *p;               /* cached pointer from `handle` */
} chunk_gizmo;

#define GET_BLOCK_ID(block)     (block & MASK_BLOCK_ID)
#define SET_BLOCK_ID(block, id) (block = (block & ~MASK_BLOCK_ID) | id)

/*!
 *  @brief look-up table to reduce redundant checking of untouched indices of @ref chunk_buf.
 *
 *  the sphere of chunks around @ref chunk_tab center are the only chunks that get processed,
 *  and since @ref CHUNK_ORDER is a look-up that orders @ref chunk_tab addresses based on
 *  their distance from that tab's center index, it becomes easy to iterate from
 *  @ref CHUNK_ORDER[0] to @ref CHUNK_ORDER[CHUNKS_MAX[render_distance]] and get exactly that sphere.
 *
 *  @remark index 0 of this array is always 0 since render distance of 0 is not
 *  possible (it's possible, but goofy).
 *
 *  @remark read-only, initialized internally in @ref chunking_init().
 */
extern u64 CHUNKS_MAX[CHUNK_BUF_RADIUS_MAX + 1];

extern chunk_table chunk_tab;
extern chunk_order CHUNK_ORDER;
extern chunk_scheduler chunk_sched[CHUNK_SCHEDULERS_MAX];

/*!
 *  @brief buffer data for opaque chunk colors.
 */
extern chunk_gizmo chunk_gizmo_loaded;

/*!
 *  @brief buffer data for transparent chunk colors.
 */
extern chunk_gizmo chunk_gizmo_render;

/*!
 *  @brief initialize chunking resources.
 *
 *  - allocate @ref chunk_arena and push @ref chunk_buf, @ref chunk_tab,
 *    @ref CHUNK_ORDER and @ref chunk_sched[<x>] onto it.
 *  - load necessary look-ups from disk if found and build them if not.
 *
 *  @remark building the look-ups is very taxing currently.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 chunking_init(void);

/*!
 *  @update everything about chunks during gameplay.
 *
 *  1. load dirty chunks into their priority schedulers based on their distance from
 *     the player.
 *
 *  2. if @ref core.flag.chunk_buf_dirty, shift @ref chunk_tab to compensate for
 *     player crossing a chunk boundary.
 *
 *  3. check if player has crossed on more than one axis and go back to shift
 *     along that axis if true.
 *
 *  4. find empty chunk slots within @ref settings.render_distance distance, push chunks onto
 *     @ref chunk_buf and return the address to the respective index to @ref chunk_tab.
 *
 *  5. remove @ref core.flag.chunk_buf_dirty when no further processing is required.
 */
void chunking_update(v3i32 player_chunk, v3i32 *player_chunk_delta);

void chunking_free(void);

/*!
 *  @brief get first block pointed at by start point towards end point.
 *
 *  @param origin entity's origin point, used to get entity's current chunk.
 */
block_hit block_hit_get(v3f64 origin, f64 start_x, f64 start_y, f64 start_z,
        f64 end_x, f64 end_y, f64 end_z, f64 distance_max);

/*!
 *  @param index index into global array @ref chunk_tab.
 *  @param normal face direction to place block onto.
 */
void block_place(block_hit hit, enum block_id block_id);

void block_break(block_hit hit);

/*!
 *  @brief get block relative to chunk.
 *
 *  @return block address in chunk if `x`, `y` and `z` are within chunk bounds and
 *  return the correct block in the neighboring chunk otherwise.
 */
u32 *get_block_resolved(hhc_chunk *ch, i32 x, i32 y, i32 z);

/*!
 *  @brief get chunk relative to position.
 *
 *  @return chunk at index if `x`, `y` and `z` are within chunk bounds and
 *  return the correct neighboring chunk otherwise.
 */
hhc_chunk *get_chunk_resolved(u32 index, i32 x, i32 y, i32 z);

/*!
 *  @brief get index of chunk in @ref chunk_tab by world coordinates relative to chunk position.
 *
 *  @param chunk_pos chunk position in world coordinates.
 *  @param pos block position in world coordinates.
 *
 *  @return index into global array @ref chunk_tab.
 *  @return @ref settings.chunk_tab_center if index out of bounds.
 */
u32 get_chunk_index(v3i32 chunk_pos, v3f64 pos);

#endif /* HHC_CHUNKING_H */
