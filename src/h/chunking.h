#ifndef GAME_CHUNKING_H
#define GAME_CHUNKING_H

#include <engine/h/types.h>

#include "assets.h"
#include "main.h"

#define CHUNK_DIAMETER  16
#define CHUNK_LAYER     (CHUNK_DIAMETER * CHUNK_DIAMETER)
#define CHUNK_VOLUME    (CHUNK_DIAMETER * CHUNK_DIAMETER * CHUNK_DIAMETER)

#define WORLD_RADIUS            (2048 * CHUNK_DIAMETER)
#define WORLD_RADIUS_VERTICAL   (64 * CHUNK_DIAMETER)

#define WORLD_DIAMETER          (WORLD_RADIUS * 2 + CHUNK_DIAMETER)
#define WORLD_DIAMETER_VERTICAL (WORLD_RADIUS_VERTICAL * 2 + CHUNK_DIAMETER)
#define WORLD_CHUNKS_MAX        ((WORLD_DIAMETER * WORLD_DIAMETER * WORLD_DIAMETER_VERTICAL) / CHUNK_DIAMETER)

#define CHUNK_BUF_RADIUS_MAX    SET_RENDER_DISTANCE_MAX
#define CHUNK_BUF_DIAMETER_MAX  (CHUNK_BUF_RADIUS_MAX * 2 + 1)
#define CHUNK_BUF_LAYER_MAX     (CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX)
#define CHUNK_BUF_VOLUME_MAX    (CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX * CHUNK_BUF_DIAMETER_MAX)

#define CHUNK_QUEUE_1ST_ID      0
#define CHUNK_QUEUE_2ND_ID      1
#define CHUNK_QUEUE_3RD_ID      2
#define CHUNK_QUEUE_LAST_ID     CHUNK_QUEUE_3RD_ID
#define CHUNK_QUEUE_1ST_MAX     256
#define CHUNK_QUEUE_2ND_MAX     4096
#define CHUNK_QUEUE_3RD_MAX     16384
#define CHUNK_QUEUES_MAX        3

/*! @brief count of temporary static buffers in internal functions
 *  'chunk_mesh_init()' and 'chunk_mesh_update()'.
 */
#define BLOCK_BUFFERS_MAX       2

/*! @brief number of chunks to process per frame.
 */
#define CHUNK_PARSE_RATE_PRIORITY_LOW       64
#define CHUNK_PARSE_RATE_PRIORITY_MID       128
#define CHUNK_PARSE_RATE_PRIORITY_HIGH      CHUNK_QUEUE_1ST_MAX

/* number of blocks to process per chunk per frame */
#define BLOCK_PARSE_RATE                    512

#define CHUNK_COLOR_LOADED  color_hex_u32(18, 3, 7, 100)
#define CHUNK_COLOR_RENDER  color_hex_u32(62, 122, 8, 255)
#define CHUNK_COLOR_FACTOR_INFLUENCE 0.1

enum BlockFlag
{
    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000001 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_PX =        0x0000000000010000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000010 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_NX =        0x0000000000020000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000100 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_PY =        0x0000000000040000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00001000 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_NY =        0x0000000000080000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00010000 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_PZ =        0x0000000000100000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00100000 00000000 00000000] 00; */
    FLAG_BLOCK_FACE_NZ =        0x0000000000200000,

    /*! @brief run-length encoding, for chunk serialization.
     *
     * 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 10000000 00000000 00000000] 00; */
    FLAG_BLOCK_RLE =            0x0000000000800000,
}; /* BlockFlag */

enum BlockMask
{
    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000000 00111111 11111111] 00; */
    MASK_BLOCK_DATA =           0x0000000000003fff,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000000 00000011 11111111] 00; */
    MASK_BLOCK_ID =             0x00000000000003ff,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00000000 00111100 00000000] 00; */
    MASK_BLOCK_STATE =          0x0000000000003c00,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00000000 00111111 00000000 00000000] 00; */
    MASK_BLOCK_FACES =          0x00000000003f0000,

    /* 63 [00000000 00000000 00000000 00000000] 32;
     * 31 [00111111 00000000 00000000 00000000] 00; */
    MASK_BLOCK_LIGHT =          0x000000003f000000,

    /* 63 [00000000 00000000 00001111 11111111] 32;
     * 31 [00000000 00000000 00000000 00000000] 00; */
    MASK_BLOCK_COORDINATES =    0x00000fff00000000,

    /* 63 [00000000 00000000 00000000 00001111] 32;
     * 31 [00000000 00000000 00000000 00000000] 00; */
    MASK_BLOCK_X =              0x0000000f00000000,

    /* 63 [00000000 00000000 00000000 11110000] 32;
     * 31 [00000000 00000000 00000000 00000000] 00; */
    MASK_BLOCK_Y =              0x000000f000000000,

    /* 63 [00000000 00000000 00001111 00000000] 32;
     * 31 [00000000 00000000 00000000 00000000] 00; */
    MASK_BLOCK_Z =              0x00000f0000000000,
}; /* BlockMask */

enum BlockShift
{
    SHIFT_BLOCK_DATA =          0,
    SHIFT_BLOCK_ID =            0,
    SHIFT_BLOCK_STATE =         10,
    SHIFT_BLOCK_FACES =         16,
    SHIFT_BLOCK_LIGHT =         24,
    SHIFT_BLOCK_COORDINATES =   32,
    SHIFT_BLOCK_X =             32,
    SHIFT_BLOCK_Y =             36,
    SHIFT_BLOCK_Z =             40,
}; /* BlockShift */

enum ChunkFlag
{
    FLAG_CHUNK_LOADED =     0x01,
    FLAG_CHUNK_DIRTY =      0x02,
    FLAG_CHUNK_QUEUED =     0x04,
    FLAG_CHUNK_GENERATED =  0x08,
    FLAG_CHUNK_RENDER =     0x10,
    FLAG_CHUNK_MODIFIED =   0x20,

    /*! @brief chunk marking for 'chunk_tab' shifting logic.
     */
    FLAG_CHUNK_EDGE =       0x40,
}; /* ChunkFlag */

enum ChunkShiftState
{
    STATE_CHUNK_SHIFT_PX = 1,
    STATE_CHUNK_SHIFT_NX = 2,
    STATE_CHUNK_SHIFT_PY = 3,
    STATE_CHUNK_SHIFT_NY = 4,
    STATE_CHUNK_SHIFT_PZ = 5,
    STATE_CHUNK_SHIFT_NZ = 6,
}; /* ChunkShiftState */

typedef struct Chunk
{
    v3i16 pos;      /* world position / CHUNK_DIAMETER */

    /*! @brief chunk's unique id derived from its position.
     *
     * format:
     * (pos.x & 0xffff) << 0x00 |
     * (pos.y & 0xffff) << 0x10 |
     * (pos.z & 0xffff) << 0x20.
     */
    u64 id;

    /*! @brief debug color.
     *
     *  format: 0xrrggbbaa.
     */
    u32 color;

    /*! @brief debug color variant.
     *
     *  used as offset for 'color'.
     *
     *  format: 0xrrggbbaa.
     */
    u32 color_variant;

    /*! @brief block iterator for per-chunk generation progress.
     */
    u32 cursor;

    GLuint vao;
    GLuint vbo;
    u64 vbo_len;
    u32 block[CHUNK_DIAMETER][CHUNK_DIAMETER][CHUNK_DIAMETER];
    u8 flag;
} Chunk;

typedef struct ChunkQueue
{
    u32 id;
    u32 count;          /* number of chunks queued */
    u32 offset;         /* first CHUNK_ORDER index to queue */
    u64 size;
    u32 cursor;         /* parse position */
    u32 rate_chunk;     /* number of chunks to process per frame */
    u32 rate_block;     /* number of blocks to process per chunk per frame */
    Chunk ***queue;
} ChunkQueue;

#define GET_BLOCK_ID(block)     (block & MASK_BLOCK_ID)
#define SET_BLOCK_ID(block, id) (block = (block & ~MASK_BLOCK_ID) | id)

/*! @brief look-up table to reduce redundant checks of untouched regions of 'chunk_buf'.
 *
 *  the sphere of chunks around 'chunk_tab' center are the only chunks that get processed,
 *  and since 'CHUNK_ORDER' is a look-up that orders 'chunk_tab' addresses based on
 *  is useful for iteration from 'CHUNK_ORDER[0]' to 'CHUNK_ORDER[CHUNKS_MAX[render_distance]]'.
 *
 *  @remark index 0 of this array is always 0 since render distance of 0 is not
 *  possible (it's possible, but goofy).
 *
 *  @remark read-only, initialized internally in 'chunking_init()'.
 */
extern u64 CHUNKS_MAX[CHUNK_BUF_RADIUS_MAX + 1];

/*! @brief chunk pointer look-up table that points to 'chunk_buf' addresses.
 *
 *  'chunk_buf' addresses ordered by their positions in 3d space relative to player position.
 */
extern Chunk **chunk_tab;

/*! @brief player relative 'chunk_tab' access.
 *
 *  @remark declared by the user.
 */
extern u32 chunk_tab_index;

/*! @brief chunk pointer pointer look-up table that points to 'chunk_tab' addresses.
 *
 *  'chunk_tab' addresses ordered by distance from 'chunk_tab' center in ascending order.
 *
 *  @remark read-only, initialized internally in 'chunking_init()'.
 */
extern Chunk ***CHUNK_ORDER;

/*! @brief queues of chunks to be processed.
 *
 *  @remark read-only, updated internally in 'chunking_update()'.
 */
extern ChunkQueue CHUNK_QUEUE[CHUNK_QUEUES_MAX];

/*! @brief chunk gizmo render buffer data for opaque chunk colors.
 *
 *  for rendering chunk gizmo in one draw call.
 *
 *  format: 0xxxyyzz00, 0xrrggbbaa.
 */
extern v2u32 *chunk_gizmo_loaded;

/*! @brief chunk gizmo render buffer data for transparent chunk colors.
 *
 *  for rendering chunk gizmo in one draw call.
 *
 *  format: 0xxxyyzz00, 0xrrggbbaa.
 */
extern v2u32 *chunk_gizmo_render;

extern GLuint chunk_gizmo_loaded_vao;
extern GLuint chunk_gizmo_loaded_vbo;
extern GLuint chunk_gizmo_render_vao;
extern GLuint chunk_gizmo_render_vbo;

/*! @brief initialize chunking resources.
 *
 *  allocate resources for 'chunk_buf', 'chunk_tab', 'CHUNK_ORDER' and
 *  'CHUNK_QUEUE[]' and load necessary look-ups from disk if found and build them if not found.
 *
 *  @remark building the look-ups is cpu-guzzling (performance-taxing).
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 chunking_init(void);

/*! @update everything about chunks during gameplay.
 *
 *  1. load dirty chunks into their priority queues based on their distance from
 *     the player.
 *  2. if '(flag & FLAG_MAIN_CHUNK_BUF_DIRTY)', shift 'chunk_tab'
 *     to compensate for the player crossing a chunk boundary.
 *  3. check if player has crossed more than one axis and go back to shift
 *     along that axis if true.
 *  4. find empty slots within 'settings.render_distance' and push chunks to
 *     'chunk_buf' and return that address to the respective index in 'chunk_tab'.
 *  5. remove flag 'FLAG_MAIN_CHUNK_BUF_DIRTY' when no further processing is required.
 */
void chunking_update(v3i32 player_chunk, v3i32 *player_chunk_delta);

void chunking_free(void);

/*! @param index = index into global array 'chunk_tab'.
 *  @param normal = face direction to place block onto.
 */
void block_place(u32 index, i32 x, i32 y, i32 z, v3f64 normal, enum BlockID block_id);

/*! @param index = index into global array 'chunk_tab'.
 */
void block_break(u32 index, i32 x, i32 y, i32 z);

/*! @brief get block relative to chunk.
 *
 *  @return block address in chunk if 'x', 'y' and 'z' are within chunk bounds and
 *  return the correct block in the neighboring chunk otherwise.
 */
u32 *get_block_resolved(Chunk *ch, i32 x, i32 y, i32 z);

/*! @brief get chunk relative to position.
 *
 *  @return chunk at index if 'x', 'y' and 'z' are within chunk bounds and
 *  return the correct neighboring chunk otherwise.
 */
Chunk *get_chunk_resolved(u32 index, i32 x, i32 y, i32 z);

/*! @brief get index of chunk in 'chunk_tab' by world coordinates relative to chunk position.
 *
 *  @param chunk_pos = chunk position in world coordinates.
 *  @param pos = block position in world coordinates.
 *
 *  @return index into global array 'chunk_tab'.
 *  @return 'settings.chunk_tab_center' if index out of bounds.
 */
u32 get_chunk_index(v3i32 chunk_pos, v3f64 pos);

#endif /* GAME_CHUNKING_H */
