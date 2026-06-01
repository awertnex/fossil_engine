#include "deps/fossil/common/limits.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/string/string.h"

#include "deps/fossil/h/dir.h"

#include "h/assets.h"
#include "h/chunking.h"
#include "h/common.h"
#include "h/diagnostics.h"
#include "h/main.h"
#include "h/terrain.h"
#include "h/world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

/* ---- section: definitions ------------------------------------------------ */

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
    chunk *p;               /* cached pointer from `handle` */
} chunk_buffer;

/* ---- section: declarations ----------------------------------------------- */

/*!
 *  @internal
 *
 *  @brief memory arena used to store all chunking buffers.
 */
static fsl_mem_arena memory_arena_chunking_internal = {0};

u64 CHUNKS_MAX[SET_RENDER_DISTANCE_MAX + 1] = {0};
static chunk_buffer chunk_buf = {0};
chunk_table chunk_tab = {0};
chunk_order CHUNK_ORDER = {0};
chunk_queue CHUNK_QUEUE[CHUNK_QUEUES_MAX] = {0};
chunk_gizmo chunk_gizmo_loaded = {0};
chunk_gizmo chunk_gizmo_render = {0};

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @internal
 */
static void block_place_internal(chunk *ch,
        chunk *px, chunk *nx, chunk *py, chunk *ny, chunk *pz, chunk *nz,
        v3u32 chunk_tab_coordinates, i32 x, i32 y, i32 z, enum block_id block_id);

/*!
 *  @internal
 */
static void block_break_internal(chunk *ch,
        chunk *px, chunk *nx, chunk *py, chunk *ny, chunk *pz, chunk *nz,
        v3u32 chunk_tab_coordinates, i32 x, i32 y, i32 z);

/*!
 *  @internal
 *
 *  @brief generate chunk blocks.
 *
 *  @param rate number of blocks to process per chunk per frame.
 *
 *  @remark calls @ref chunk_mesh_init() when done generating.
 *  @remark must be called before @ref chunk_mesh_update().
 */
static void chunk_generate(chunk_cache ch, u32 rate);

/*!
 *  @internal
 *
 *  @brief generate chunk blocks.
 *
 *  automatically called from @ref chunk_generate().
 *
 *  @param rate number of blocks to process per chunk per frame.
 *
 *  @remark calls @ref chunk_mesh_init() when done generating.
 *  @remark must be called before @ref chunk_mesh_update().
 */
static void chunk_generate_internal(chunk_cache ch, u32 rate);

/*!
 *  @internal
 *
 *  @param index index into global array @ref chunk_tab.
 */
static void chunk_mesh_init(chunk_cache ch);

/*!
 *  @internal
 */
static void chunk_mesh_update(chunk_cache ch);

/*!
 *  @internal
 *
 *  @brief write chunk into disk.
 */
static void chunk_export_internal(chunk *ch);

/*!
 *  @internal
 *
 *  @brief read chunk from disk.
 */
static void chunk_import_internal(const fsl_fs_path *path, chunk *ch);

/*!
 *  @internal
 *
 *  @param index index into global array @ref chunk_tab.
 */
static void chunk_buf_push_internal(u32 index, v3i32 player_chunk_delta);

/*!
 *  @internal
 */
static void chunk_gizmo_write_internal(chunk_cache ch);

/*!
 *  @internal
 *
 *  @param index index into global array @ref chunk_tab.
 */
static void chunk_buf_pop_internal(u32 index);

/*!
 *  @internal
 *
 *  @param len number of chunks from @ref chunk_order.p this queue is allowed to parse.
 */
static void chunk_queue_update_internal(chunk_queue *q, fsl_len len,
        b8 should_push, b8 should_pop);

/*!
 *  @internal
 *
 *  @brief flush all enqueued chunks out of `q->queue_p` and reset cursors.
 *
 *  @note when @ref chunk_table shifts, the indices of enqueued chunks represent
 *  the wrong indices within @ref chunk_table, so, they need to be flushed in order
 *  to not process the wrong chunk.
 */
static void chunk_queue_reset_internal(chunk_queue *q);

/* ---- section: implementation --------------------------------------------- */

u32 chunking_init(void)
{
    str CHUNK_ORDER_lookup_file_name[FSL_PATH_CAP] = {0};
    str CHUNKS_MAX_lookup_file_name[FSL_PATH_CAP] = {0};
    u32 *CHUNK_ORDER_lookup_file_contents = NULL;
    u64 *CHUNKS_MAX_lookup_file_contents = NULL;
    u64 i = 0;
    u64 j = 0;
    u64 k = 0;
    u64 file_len = 0;
    u32 render_distance = 0;
    v3i32 center = {0};
    v3i32 coordinates = {0};
    u32 *distance = NULL;
    u32 *index = NULL;
    u64 chunk_buf_diameter = 0;
    u64 chunk_buf_volume = 0;
    u64 chunks_max = 0;

    if (core.flag.chunks_initialized)
        return FSL_ERR_SUCCESS;

    if (fsl_mem_arena_init(&memory_arena_chunking_internal,
                "chunking_init().memory_arena_chunking_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &CHUNK_ORDER.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(chunk**),
                "chunking_init().CHUNK_ORDER.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_tab.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(chunk*),
                "chunking_init().chunk_tab.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_buf.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(chunk),
                "chunking_init().chunk_buf.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &CHUNK_QUEUE[0].queue,
                CHUNK_QUEUE_1ST_MAX * sizeof(chunk_cache),
                "chunking_init().CHUNK_QUEUE[0].queue") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &CHUNK_QUEUE[1].queue,
                CHUNK_QUEUE_2ND_MAX * sizeof(chunk_cache),
                "chunking_init().CHUNK_QUEUE[1].queue") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &CHUNK_QUEUE[2].queue,
                CHUNK_QUEUE_3RD_MAX * sizeof(chunk_cache),
                "chunking_init().CHUNK_QUEUE[2].queue") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_gizmo_loaded.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(v2u32),
                "chunking_init().chunk_gizmo_loaded.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_gizmo_render.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(v2u32),
                "chunking_init().chunk_gizmo_render.handle") != FSL_ERR_SUCCESS)
        goto cleanup;

    chunk_buf.p = fsl_mem_handle_get(chunk_buf.handle);
    chunk_tab.p = fsl_mem_handle_get(chunk_tab.handle);
    CHUNK_ORDER.p = fsl_mem_handle_get(CHUNK_ORDER.handle);
    CHUNK_QUEUE[0].queue_p = fsl_mem_handle_get(CHUNK_QUEUE[0].queue);
    CHUNK_QUEUE[1].queue_p = fsl_mem_handle_get(CHUNK_QUEUE[1].queue);
    CHUNK_QUEUE[2].queue_p = fsl_mem_handle_get(CHUNK_QUEUE[2].queue);
    chunk_gizmo_loaded.p = fsl_mem_handle_get(chunk_gizmo_loaded.handle);
    chunk_gizmo_render.p = fsl_mem_handle_get(chunk_gizmo_render.handle);

    if (
            fsl_mem_map((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunking_init().distance") != FSL_ERR_SUCCESS ||

            fsl_mem_map((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunking_init().index") != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- build distance look-ups ----------------------------------------- */

    for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
    {
        chunk_buf_diameter = (i * 2) + 1;
        chunk_buf_volume =
            chunk_buf_diameter * chunk_buf_diameter * chunk_buf_diameter;
        render_distance = (i * i) + 2;
        center.x = i;
        center.y = i;
        center.z = i;

        snprintf(CHUNK_ORDER_lookup_file_name, FSL_PATH_CAP,
                "%slookup_chunk_order_0x%02"PRIx64".bin", GAME_DIR_NAME_LOOKUPS, i);

        if (fsl_is_file_exists(CHUNK_ORDER_lookup_file_name, FALSE) != FSL_ERR_SUCCESS)
        {
            LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("Building CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x]..\n",
                        i, SET_RENDER_DISTANCE_MAX));
            for (j = 0; j < chunk_buf_volume; ++j)
            {

                coordinates.x = j % chunk_buf_diameter;
                coordinates.y = (j / chunk_buf_diameter) % chunk_buf_diameter;
                coordinates.z = j / (chunk_buf_diameter * chunk_buf_diameter);
                distance[j] = fsl_distance_v3i32(coordinates, center);
                index[j] = j;
            }

            LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("Sorting CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x]..\n",
                    i, SET_RENDER_DISTANCE_MAX));

            for (j = 0; j < chunk_buf_volume; ++j)
            {
                for (k = 0; k < chunk_buf_volume; ++k)
                {
                    if (distance[j] < distance[k])
                    {
                        fsl_swap_bits_u32(&distance[j], &distance[k]);
                        fsl_swap_bits_u32(&index[j], &index[k]);
                    }
                }
            }

            LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("Writing CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x] To File..\n",
                    i, SET_RENDER_DISTANCE_MAX));

            if (fsl_write_file(CHUNK_ORDER_lookup_file_name, sizeof(u32) * chunk_buf_volume,
                        index, TRUE, FALSE) != FSL_ERR_SUCCESS)
                goto cleanup;
        }
    }

    snprintf(CHUNK_ORDER_lookup_file_name, FSL_PATH_CAP,
            "%slookup_chunk_order_0x%02x.bin",
            GAME_DIR_NAME_LOOKUPS, settings.render_distance);

    file_len = fsl_get_file_contents(CHUNK_ORDER_lookup_file_name,
            (void*)&CHUNK_ORDER_lookup_file_contents, FALSE);
    if (*GAME_ERR != FSL_ERR_SUCCESS ||
            CHUNK_ORDER_lookup_file_contents == NULL)
        goto cleanup;

    for (i = 0; i < settings.chunk_buf_volume; ++i)
        CHUNK_ORDER.p[i] = &chunk_tab.p[CHUNK_ORDER_lookup_file_contents[i]];
    fsl_mem_free((void*)&CHUNK_ORDER_lookup_file_contents, file_len,
            "chunking_init().CHUNK_ORDER_lookup_file_contents");

    /* ---- build CHUNKS_MAX look-up ---------------------------------------- */

    snprintf(CHUNKS_MAX_lookup_file_name, FSL_PATH_CAP,
            "%slookup_chunks_max.bin", GAME_DIR_NAME_LOOKUPS);

    if (fsl_is_file_exists(CHUNKS_MAX_lookup_file_name, FALSE) != FSL_ERR_SUCCESS)
    {
        for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
        {
            LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("Building CHUNKS_MAX Lookup, Progress [%"PRIu64"/%d]..\n",
                    i, SET_RENDER_DISTANCE_MAX));
            chunk_buf_diameter = (i * 2) + 1;
            chunk_buf_volume =
                chunk_buf_diameter * chunk_buf_diameter * chunk_buf_diameter;
            chunks_max = 0;
            render_distance = (i * i) + 2;
            center.x = i;
            center.y = i;
            center.z = i;

            for (j = 0; j < chunk_buf_volume; ++j)
            {
                coordinates.x = j % chunk_buf_diameter;
                coordinates.y = (j / chunk_buf_diameter) % chunk_buf_diameter;
                coordinates.z = j / (chunk_buf_diameter * chunk_buf_diameter);
                if (fsl_distance_v3i32(coordinates, center) < render_distance)
                    ++chunks_max;
            }

            CHUNKS_MAX[i] = chunks_max;
        }

        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                fsl_logger_stringf("%s\n", "Writing CHUNKS_MAX Lookup To File..\n"));

        if (fsl_write_file(CHUNKS_MAX_lookup_file_name,
                (SET_RENDER_DISTANCE_MAX + 1) * sizeof(u64),
                &CHUNKS_MAX, TRUE, FALSE) != FSL_ERR_SUCCESS)
            goto cleanup;
    }

    file_len = fsl_get_file_contents(CHUNKS_MAX_lookup_file_name,
            (void*)&CHUNKS_MAX_lookup_file_contents, FALSE);
    if (*GAME_ERR != FSL_ERR_SUCCESS ||
            CHUNKS_MAX_lookup_file_contents == NULL)
        goto cleanup;

    for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
        CHUNKS_MAX[i] = CHUNKS_MAX_lookup_file_contents[i];
    fsl_mem_free((void*)&CHUNKS_MAX_lookup_file_contents, file_len,
            "chunking_init().CHUNKS_MAX_lookup_file_contents");

    /* ---- init chunk parsing priority queues ------------------------------ */

    CHUNK_QUEUE[0].id = CHUNK_QUEUE_1ST_ID;
    CHUNK_QUEUE[0].offset = 0;
    CHUNK_QUEUE[0].len =
        (u64)fsl_clamp_i64(CHUNKS_MAX[settings.render_distance],
                0, CHUNK_QUEUE_1ST_MAX);
    CHUNK_QUEUE[0].rate_chunk = CHUNK_PARSE_RATE_PRIORITY_HIGH;
    CHUNK_QUEUE[0].rate_block = BLOCK_PARSE_RATE;

    CHUNK_QUEUE[1].id = CHUNK_QUEUE_2ND_ID;
    CHUNK_QUEUE[1].offset = CHUNK_QUEUE_1ST_MAX;
    CHUNK_QUEUE[1].len =
        (u64)fsl_clamp_i64(CHUNKS_MAX[settings.render_distance] -
                CHUNK_QUEUE[1].offset, 0, CHUNK_QUEUE_2ND_MAX);
    CHUNK_QUEUE[1].rate_chunk = CHUNK_PARSE_RATE_PRIORITY_MID;
    CHUNK_QUEUE[1].rate_block = BLOCK_PARSE_RATE;

    CHUNK_QUEUE[2].id = CHUNK_QUEUE_3RD_ID;
    CHUNK_QUEUE[2].offset = CHUNK_QUEUE_1ST_MAX + CHUNK_QUEUE_2ND_MAX;
    CHUNK_QUEUE[2].len =
        (u64)fsl_clamp_i64(CHUNKS_MAX[settings.render_distance] -
            CHUNK_QUEUE[2].offset, 0, CHUNK_QUEUE_3RD_MAX);
    CHUNK_QUEUE[2].rate_chunk = CHUNK_PARSE_RATE_PRIORITY_LOW;
    CHUNK_QUEUE[2].rate_block = BLOCK_PARSE_RATE;

    fsl_mem_unmap((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_init().distance");

    fsl_mem_unmap((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_init().index");

    /* ---- intialize chunk gizmo ------------------------------------------- */

    glGenVertexArrays(1, &chunk_gizmo_loaded.vao);
    glGenBuffers(1, &chunk_gizmo_loaded.vbo);

    glBindVertexArray(chunk_gizmo_loaded.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_loaded.vbo);
    glBufferData(GL_ARRAY_BUFFER, settings.chunk_buf_volume * sizeof(v2u32),
            chunk_gizmo_loaded.p, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(v2u32), (void*)0);

    glGenVertexArrays(1, &chunk_gizmo_render.vao);
    glGenBuffers(1, &chunk_gizmo_render.vbo);

    glBindVertexArray(chunk_gizmo_render.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_render.vbo);
    glBufferData(GL_ARRAY_BUFFER, settings.chunk_buf_volume * sizeof(v2u32),
            chunk_gizmo_render.p, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(v2u32), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    chunk_gizmo_loaded.initialized = TRUE;
    chunk_gizmo_render.initialized = TRUE;
    core.flag.chunks_initialized = TRUE;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_mem_unmap((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_init().distance");

    fsl_mem_unmap((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_init().index");

    chunking_free();
    return *GAME_ERR;
}

void chunking_update(v3i32 player_chunk, v3i32 *player_chunk_delta)
{
    chunk ***cursor = NULL;
    v3u32 _coordinates = {0};
    v3u32 _mirror_index = {0};
    v3u32 _target_index = {0};
    u32 mirror_index = 0;
    u32 target_index = 0;
    u8 is_on_edge = 0;
    v3u8 _is_on_edge = {0};
    i64 i = 0;
    v3i16 DELTA = {0};
    u8 AXIS = 0;
    i8 INCREMENT = 0;
    v3f32 DISTANCE = {0};
    i32 RENDER_DISTANCE = 0;
    chunk_cache cache = {0};

    chunk_queue_update_internal(&CHUNK_QUEUE[0],
            CHUNK_QUEUE[0].offset + CHUNK_QUEUE[0].len,
            TRUE, TRUE);

    chunk_queue_update_internal(&CHUNK_QUEUE[1],
            CHUNK_QUEUE[1].offset + CHUNK_QUEUE[1].len,
            TRUE, !CHUNK_QUEUE[0].count && CHUNK_QUEUE[1].len);

    chunk_queue_update_internal(&CHUNK_QUEUE[2],
            CHUNKS_MAX[settings.render_distance],
            TRUE, !CHUNK_QUEUE[1].count && CHUNK_QUEUE[2].len);

    if (!core.flag.chunk_buf_dirty)
        return;

    chunk_queue_reset_internal(&CHUNK_QUEUE[0]);
    chunk_queue_reset_internal(&CHUNK_QUEUE[1]);
    chunk_queue_reset_internal(&CHUNK_QUEUE[2]);

chunk_tab_shift:

    DELTA.x = player_chunk.x - player_chunk_delta->x;
    DELTA.y = player_chunk.y - player_chunk_delta->y;
    DELTA.z = player_chunk.z - player_chunk_delta->z;

    AXIS =
        DELTA.x > 0 ? STATE_CHUNK_SHIFT_PX :
        DELTA.x < 0 ? STATE_CHUNK_SHIFT_NX :
        DELTA.y > 0 ? STATE_CHUNK_SHIFT_PY :
        DELTA.y < 0 ? STATE_CHUNK_SHIFT_NY :
        DELTA.z > 0 ? STATE_CHUNK_SHIFT_PZ :
        DELTA.z < 0 ? STATE_CHUNK_SHIFT_NZ : 0;

    INCREMENT = (AXIS % 2 == 1) - (AXIS %2 == 0);
    DISTANCE.x = DELTA.x;
    DISTANCE.y = DELTA.y;
    DISTANCE.z = DELTA.z;
    RENDER_DISTANCE = settings.render_distance * settings.render_distance + 2;

    if ((i32)fsl_len_v3f32(DISTANCE) > RENDER_DISTANCE)
    {
        cursor = &CHUNK_ORDER.p[CHUNKS_MAX[settings.render_distance] - 1];
        for (; cursor >= CHUNK_ORDER.p; --cursor)
        {
            if (**cursor)
                chunk_buf_pop_internal(*cursor - chunk_tab.p);
        }

        *player_chunk_delta = player_chunk;
        goto chunk_buf_push;
    }

    /* this prevents `chunk_buf` from exploding when shifting `chunk_tab` */
    chunk_buf.cursor = 0;

    switch (AXIS)
    {
        case STATE_CHUNK_SHIFT_PX:
        case STATE_CHUNK_SHIFT_NX:
            player_chunk_delta->x += INCREMENT;
            break;

        case STATE_CHUNK_SHIFT_PY:
        case STATE_CHUNK_SHIFT_NY:
            player_chunk_delta->y += INCREMENT;
            break;

        case STATE_CHUNK_SHIFT_PZ:
        case STATE_CHUNK_SHIFT_NZ:
            player_chunk_delta->z += INCREMENT;
            break;

        default:
            goto chunk_buf_push;
    }

    /* ---- mark chunks on-edge --------------------------------------------- */

    for (i = 0; i < settings.chunk_buf_volume; ++i)
    {
        if (!chunk_tab.p[i])
            continue;

        _coordinates.x = i % settings.chunk_buf_diameter;
        _coordinates.y = (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
        _coordinates.z = i / settings.chunk_buf_layer;

        _mirror_index.x = i + settings.chunk_buf_diameter - 1 - _coordinates.x * 2;
        _mirror_index.y = _coordinates.z * settings.chunk_buf_layer +
                (settings.chunk_buf_diameter - 1 - _coordinates.y) *
                settings.chunk_buf_diameter + _coordinates.x;
        _mirror_index.z =
            (settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer +
                _coordinates.y * settings.chunk_buf_diameter + _coordinates.x;

        switch (INCREMENT)
        {
            case -1:
                _is_on_edge.x = _coordinates.x == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab.p[i + 1];
                _is_on_edge.y = _coordinates.y == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab.p[i + settings.chunk_buf_diameter];
                _is_on_edge.z = _coordinates.z == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab.p[i + settings.chunk_buf_layer];
                break;

            case 1:
                _is_on_edge.x = _coordinates.x == 0 || !chunk_tab.p[i - 1];
                _is_on_edge.y = _coordinates.y == 0 || !chunk_tab.p[i - settings.chunk_buf_diameter];
                _is_on_edge.z = _coordinates.z == 0 || !chunk_tab.p[i - settings.chunk_buf_layer];
                break;
        }

        switch (AXIS)
        {
            case STATE_CHUNK_SHIFT_PX:
            case STATE_CHUNK_SHIFT_NX:
                mirror_index = _mirror_index.x;
                is_on_edge = _is_on_edge.x;
                break;

            case STATE_CHUNK_SHIFT_PY:
            case STATE_CHUNK_SHIFT_NY:
                mirror_index = _mirror_index.y;
                is_on_edge = _is_on_edge.y;
                break;

            case STATE_CHUNK_SHIFT_PZ:
            case STATE_CHUNK_SHIFT_NZ:
                mirror_index = _mirror_index.z;
                is_on_edge = _is_on_edge.z;
                break;
        }

        if (is_on_edge)
        {
            chunk_tab.p[i]->flag &= ~(FLAG_CHUNK_LOADED | FLAG_CHUNK_RENDER);
            chunk_tab.p[i]->color = 0;
            if (chunk_tab.p[mirror_index])
                chunk_tab.p[mirror_index]->flag |= FLAG_CHUNK_EDGE;
        }
    }

    /* ---- shift `chunk_tab` ----------------------------------------------- */

    for (i = (INCREMENT == 1) ? 0 : settings.chunk_buf_volume - 1;
            i < settings.chunk_buf_volume && i >= 0; i += INCREMENT)
    {
        if (!chunk_tab.p[i])
            continue;

        _coordinates.x = i % settings.chunk_buf_diameter;
        _coordinates.y = (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
        _coordinates.z = i / settings.chunk_buf_layer;

        _mirror_index.x = i + settings.chunk_buf_diameter - 1 - _coordinates.x * 2;
        _mirror_index.y =
            _coordinates.z * settings.chunk_buf_layer +
                (settings.chunk_buf_diameter - 1 - _coordinates.y) *
                 settings.chunk_buf_diameter + _coordinates.x;
        _mirror_index.z =
            (settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer +
                _coordinates.y * settings.chunk_buf_diameter + _coordinates.x;

        switch (INCREMENT)
        {
            case -1:
                _target_index.x = _coordinates.x == 0 ? i : i - 1;
                _target_index.y = _coordinates.y == 0 ? i : i - settings.chunk_buf_diameter;
                _target_index.z = _coordinates.z == 0 ? i : i - settings.chunk_buf_layer;
                break;

            case 1:
                _target_index.x = _coordinates.x == settings.chunk_buf_diameter - 1 ? i : i + 1;
                _target_index.y = _coordinates.y == settings.chunk_buf_diameter - 1 ?
                            i : i + settings.chunk_buf_diameter;
                _target_index.z = _coordinates.z == settings.chunk_buf_diameter - 1 ?
                            i : i + settings.chunk_buf_layer;
                break;
        }

        switch (AXIS)
        {
            case STATE_CHUNK_SHIFT_PX:
            case STATE_CHUNK_SHIFT_NX:
                mirror_index = _mirror_index.x;
                target_index = _target_index.x;
                break;

            case STATE_CHUNK_SHIFT_PY:
            case STATE_CHUNK_SHIFT_NY:
                mirror_index = _mirror_index.y;
                target_index = _target_index.y;
                break;

            case STATE_CHUNK_SHIFT_PZ:
            case STATE_CHUNK_SHIFT_NZ:
                mirror_index = _mirror_index.z;
                target_index = _target_index.z;
                break;
        }

        chunk_tab.p[i] = chunk_tab.p[target_index];
        if (chunk_tab.p[i])
        {
            cache.ch = chunk_tab.p[i];
            cache.index = i;
            chunk_gizmo_write_internal(cache);
            if (chunk_tab.p[i]->flag & FLAG_CHUNK_EDGE)
                chunk_tab.p[target_index] = NULL;

            chunk_tab.p[i]->flag &= ~(FLAG_CHUNK_EDGE | FLAG_CHUNK_QUEUED);
        }
    }

    if (core.flag.chunk_buf_dirty)
    {
        if (DELTA.x || DELTA.y || DELTA.z)
            goto chunk_tab_shift;
        else
            core.flag.chunk_buf_dirty = 0;
    }


chunk_buf_push:

    for (i = 0; i < (i64)CHUNKS_MAX[settings.render_distance]; ++i)
    {
        cache.ch = *CHUNK_ORDER.p[i];
        cache.index = CHUNK_ORDER.p[i] - chunk_tab.p;
        if (!cache.ch)
            chunk_buf_push_internal(cache.index, *player_chunk_delta);
    }

    core.flag.chunk_buf_dirty = 0;
}

void chunking_free(void)
{
    if (chunk_tab.p)
    {
        u32 i = 0;
        for (; i < settings.chunk_buf_volume; ++i)
            if (chunk_tab.p[i])
                chunk_buf_pop_internal(i);
    }

    if (chunk_gizmo_loaded.initialized)
    {
        chunk_gizmo_loaded.initialized = FALSE;
        glDeleteBuffers(1, &chunk_gizmo_loaded.vbo);
        glDeleteVertexArrays(1, &chunk_gizmo_loaded.vao);
    }
    if (chunk_gizmo_render.initialized)
    {
        chunk_gizmo_render.initialized = FALSE;
        glDeleteVertexArrays(1, &chunk_gizmo_render.vao);
        glDeleteBuffers(1, &chunk_gizmo_render.vbo);
    }

    fsl_mem_arena_free(&memory_arena_chunking_internal,
                "chunking_init().memory_arena_chunking_internal");
}

void block_place(u32 index, i32 x, i32 y, i32 z, v3f64 normal, enum block_id block_id)
{
    chunk **ch = NULL;
    chunk *px = NULL;
    chunk *nx = NULL;
    chunk *py = NULL;
    chunk *ny = NULL;
    chunk *pz = NULL;
    chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};

    if (!(normal.x != 0.0f || normal.y != 0.0f || normal.z != 0.0f)) return;

    x += (i32)normal.x;
    y += (i32)normal.y;
    z += (i32)normal.z;
    index += (i32)floorf((f32)x / CHUNK_DIAMETER);
    index += (i32)floorf((f32)y / CHUNK_DIAMETER) * settings.chunk_buf_diameter;
    index += (i32)floorf((f32)z / CHUNK_DIAMETER) * settings.chunk_buf_layer;
    if (index >= settings.chunk_buf_volume)
        return;

    ch = &chunk_tab.p[index];
    if ((*ch)->block[z][y][x] || !block_id)
        return;

    chunk_tab_coordinates.x = index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = index / settings.chunk_buf_layer;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = *(ch + 1);
    if (chunk_tab_coordinates.x > 0)
        nx = *(ch - 1);
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = *(ch + settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.y > 0)
        ny = *(ch - settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = *(ch + settings.chunk_buf_layer);
    if (chunk_tab_coordinates.z > 0)
        nz = *(ch - settings.chunk_buf_layer);

    block_place_internal(*ch, px, nx, py, ny, pz, nz, chunk_tab_coordinates, x, y, z, block_id);
}

static void block_place_internal(chunk *ch,
        chunk *px, chunk *nx, chunk *py, chunk *ny, chunk *pz, chunk *nz,
        v3u32 chunk_tab_coordinates, i32 x, i32 y, i32 z, enum block_id block_id)
{
    u32 *block = NULL;

    x = fsl_mod_i32(x, CHUNK_DIAMETER);
    y = fsl_mod_i32(y, CHUNK_DIAMETER);
    z = fsl_mod_i32(z, CHUNK_DIAMETER);
    block = &ch->block[z][y][x];
    SET_BLOCK_ID(*block, block_id);

    if (x == CHUNK_DIAMETER - 1)
    {
        if (chunk_tab_coordinates.x != settings.chunk_buf_diameter - 1 &&
                px && px->block[z][y][0])
        {
            px->block[z][y][0] &= ~FLAG_BLOCK_FACE_NX;
            px->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
        else ch->block[z][y][x] |= FLAG_BLOCK_FACE_PX;
    }
    else if (ch->block[z][y][x + 1])
        ch->block[z][y][x + 1] &= ~FLAG_BLOCK_FACE_NX;
    else ch->block[z][y][x] |= FLAG_BLOCK_FACE_PX;

    if (x == 0)
    {
        if (chunk_tab_coordinates.x != 0 &&
                nx && nx->block[z][y][CHUNK_DIAMETER - 1])
        {
            nx->block[z][y][CHUNK_DIAMETER - 1] &= ~FLAG_BLOCK_FACE_PX;
            nx->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
        else ch->block[z][y][x] |= FLAG_BLOCK_FACE_NX;
    }
    else if (ch->block[z][y][x - 1])
        ch->block[z][y][x - 1] &= ~FLAG_BLOCK_FACE_PX;
    else ch->block[z][y][x] |= FLAG_BLOCK_FACE_NX;

    if (y == CHUNK_DIAMETER - 1)
    {
        if (chunk_tab_coordinates.y != settings.chunk_buf_diameter - 1 &&
                py && py->block[z][0][x])
        {
            py->block[z][0][x] &= ~FLAG_BLOCK_FACE_NY;
            py->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
        else ch->block[z][y][x] |= FLAG_BLOCK_FACE_PY;
    }
    else if (ch->block[z][y + 1][x])
        ch->block[z][y + 1][x] &= ~FLAG_BLOCK_FACE_NY;
    else ch->block[z][y][x] |= FLAG_BLOCK_FACE_PY;

    if (y == 0)
    {
        if (chunk_tab_coordinates.y != 0 &&
                ny && ny->block[z][CHUNK_DIAMETER - 1][x])
        {
            ny->block[z][CHUNK_DIAMETER - 1][x] &= ~FLAG_BLOCK_FACE_PY;
            ny->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
        else ch->block[z][y][x] |= FLAG_BLOCK_FACE_NY;
    }
    else if (ch->block[z][y - 1][x])
        ch->block[z][y - 1][x] &= ~FLAG_BLOCK_FACE_PY;
    else ch->block[z][y][x] |= FLAG_BLOCK_FACE_NY;

    if (z == CHUNK_DIAMETER - 1)
    {
        if (chunk_tab_coordinates.z != settings.chunk_buf_diameter - 1 &&
                pz && pz->block[0][y][x])
        {
            pz->block[0][y][x] &= ~FLAG_BLOCK_FACE_NZ;
            pz->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
        else ch->block[z][y][x] |= FLAG_BLOCK_FACE_PZ;
    }
    else if (ch->block[z + 1][y][x])
        ch->block[z + 1][y][x] &= ~FLAG_BLOCK_FACE_NZ;
    else ch->block[z][y][x] |= FLAG_BLOCK_FACE_PZ;

    if (z == 0)
    {
        if (chunk_tab_coordinates.z != 0 &&
                nz && nz->block[CHUNK_DIAMETER - 1][y][x])
        {
            nz->block[CHUNK_DIAMETER - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
            nz->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
        else ch->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;
    }
    else if (ch->block[z - 1][y][x])
        ch->block[z - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
    else ch->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;

    ch->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
}

void block_break(u32 index, i32 x, i32 y, i32 z)
{
    chunk **ch = &chunk_tab.p[index];
    chunk *px = NULL;
    chunk *nx = NULL;
    chunk *py = NULL;
    chunk *ny = NULL;
    chunk *pz = NULL;
    chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};

    if (!(*ch)->block[z][y][x])
        return;

    chunk_tab_coordinates.x = index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = index / settings.chunk_buf_layer;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = *(ch + 1);
    if (chunk_tab_coordinates.x > 0)
        nx = *(ch - 1);
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = *(ch + settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.y > 0)
        ny = *(ch - settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = *(ch + settings.chunk_buf_layer);
    if (chunk_tab_coordinates.z > 0)
        nz = *(ch - settings.chunk_buf_layer);

    block_break_internal(*ch, px, nx, py, ny, pz, nz, chunk_tab_coordinates, x, y, z);
    (*ch)->flag |= FLAG_CHUNK_MODIFIED;
}

static void block_break_internal(chunk *ch,
        chunk *px, chunk *nx, chunk *py, chunk *ny, chunk *pz, chunk *nz,
        v3u32 chunk_tab_coordinates, i32 x, i32 y, i32 z)
{
    x = fsl_mod_i32(x, CHUNK_DIAMETER);
    y = fsl_mod_i32(y, CHUNK_DIAMETER);
    z = fsl_mod_i32(z, CHUNK_DIAMETER);

    if (x == CHUNK_DIAMETER - 1)
    {
        if (chunk_tab_coordinates.x != settings.chunk_buf_diameter - 1 &&
                px && px->block[z][y][0])
        {
            px->block[z][y][0] |= FLAG_BLOCK_FACE_NX;
            px->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
    }
    else if (ch->block[z][y][x + 1])
        ch->block[z][y][x + 1] |= FLAG_BLOCK_FACE_NX;

    if (x == 0)
    {
        if (chunk_tab_coordinates.x != 0 &&
                nx && nx->block[z][y][CHUNK_DIAMETER - 1])
        {
            nx->block[z][y][CHUNK_DIAMETER - 1] |= FLAG_BLOCK_FACE_PX;
            nx->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
    }
    else if (ch->block[z][y][x - 1])
        ch->block[z][y][x - 1] |= FLAG_BLOCK_FACE_PX;

    if (y == CHUNK_DIAMETER - 1)
    {
        if (chunk_tab_coordinates.y != settings.chunk_buf_diameter - 1 &&
                py && py->block[z][0][x])
        {
            py->block[z][0][x] |= FLAG_BLOCK_FACE_NY;
            py->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
    }
    else if (ch->block[z][y + 1][x])
        ch->block[z][y + 1][x] |= FLAG_BLOCK_FACE_NY;

    if (y == 0)
    {
        if (chunk_tab_coordinates.y != 0 &&
                ny && ny->block[z][CHUNK_DIAMETER - 1][x])
        {
            ny->block[z][CHUNK_DIAMETER - 1][x] |= FLAG_BLOCK_FACE_PY;
            ny->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
    }
    else if (ch->block[z][y - 1][x])
        ch->block[z][y - 1][x] |= FLAG_BLOCK_FACE_PY;

    if (z == CHUNK_DIAMETER - 1)
    {
        if (chunk_tab_coordinates.z != settings.chunk_buf_diameter - 1 &&
                pz && pz->block[0][y][x])
        {
            pz->block[0][y][x] |= FLAG_BLOCK_FACE_NZ;
            pz->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
    }
    else if (ch->block[z + 1][y][x])
        ch->block[z + 1][y][x] |= FLAG_BLOCK_FACE_NZ;

    if (z == 0)
    {
        if (chunk_tab_coordinates.z != 0 &&
                nz && nz->block[CHUNK_DIAMETER - 1][y][x])
        {
            nz->block[CHUNK_DIAMETER - 1][y][x] |= FLAG_BLOCK_FACE_PZ;
            nz->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
        }
    }
    else if (ch->block[z - 1][y][x])
        ch->block[z - 1][y][x] |= FLAG_BLOCK_FACE_PZ;

    ch->block[z][y][x] = 0;
    ch->flag |= FLAG_CHUNK_DIRTY | FLAG_CHUNK_MODIFIED;
}

static void chunk_generate(chunk_cache ch, u32 rate)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    if (!ch.ch)
        return;

    snprintf(path, FSL_PATH_CAP,
            "%s"GAME_DIR_WORLD_NAME_CHUNKS FORMAT_FILE_NAME_HHCC,
            world.path, ch.ch->pos.x, ch.ch->pos.y, ch.ch->pos.z);

    if (fsl_is_file_exists(path, FALSE) == FSL_ERR_SUCCESS)
    {
        chunk_import_internal(path, ch.ch);
        chunk_mesh_init(ch);
    }
    else
        chunk_generate_internal(ch, rate);
}

static void chunk_generate_internal(chunk_cache ch, u32 rate)
{
    chunk *px = NULL;
    chunk *nx = NULL;
    chunk *py = NULL;
    chunk *ny = NULL;
    chunk *pz = NULL;
    chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};
    v3i32 coordinates = {0};
    terrain terrain_info = {0};
    i32 x = 0, y = 0, z = 0;

    chunk_tab_coordinates.x = ch.index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (ch.index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = ch.index / settings.chunk_buf_layer;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = chunk_tab.p[ch.index + 1];
    if (chunk_tab_coordinates.x > 0)
        nx = chunk_tab.p[ch.index - 1];
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = chunk_tab.p[ch.index + settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.y > 0)
        ny = chunk_tab.p[ch.index - settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = chunk_tab.p[ch.index + settings.chunk_buf_layer];
    if (chunk_tab_coordinates.z > 0)
        nz = chunk_tab.p[ch.index - settings.chunk_buf_layer];

    x = ch.ch->cursor % CHUNK_DIAMETER;
    y = (ch.ch->cursor / CHUNK_DIAMETER) % CHUNK_DIAMETER;
    z = ch.ch->cursor / CHUNK_LAYER;
    for (; z < CHUNK_DIAMETER && rate; ++z)
    {
        for (; y < CHUNK_DIAMETER && rate; ++y)
        {
            for (; x < CHUNK_DIAMETER && rate; ++x)
            {
                coordinates.x = x + ch.ch->pos.x * CHUNK_DIAMETER;
                coordinates.y = y + ch.ch->pos.y * CHUNK_DIAMETER;
                coordinates.z = z + ch.ch->pos.z * CHUNK_DIAMETER;

                terrain_info = world.terrain_func(coordinates);

                if (terrain_info.block_id)
                {
                    block_place_internal(ch.ch, px, nx, py, ny, pz, nz,
                            chunk_tab_coordinates, x, y, z, terrain_info.block_id);
                    ch.ch->block[z][y][x] |= terrain_info.block_light;

                    if (z == 0)
                    {
                        if (nz && GET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x]) == BLOCK_GRASS)
                        {
                            SET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x], BLOCK_DIRT);
                            nz->flag |= FLAG_CHUNK_DIRTY;
                        }
                    }
                    else if (ch.ch->block[z - 1][y][x] && GET_BLOCK_ID(ch.ch->block[z - 1][y][x]) == BLOCK_GRASS)
                    {
                        SET_BLOCK_ID(ch.ch->block[z - 1][y][x], BLOCK_DIRT);
                        ch.ch->flag |= FLAG_CHUNK_DIRTY;
                    }

                    if (z == CHUNK_DIAMETER - 1 && GET_BLOCK_ID(ch.ch->block[z][y][x]) == BLOCK_GRASS &&
                            pz && pz->block[0][y][x])
                        SET_BLOCK_ID(ch.ch->block[z][y][x], BLOCK_DIRT);
                }
                --rate;
            }
            x = 0;
        }
        y = 0;
    }
    ch.ch->cursor = x + y * CHUNK_DIAMETER + z * CHUNK_LAYER;

    if (ch.ch->cursor == CHUNK_VOLUME && !(ch.ch->flag & FLAG_CHUNK_GENERATED))
    {
        ch.ch->flag |= FLAG_CHUNK_MODIFIED;
        chunk_mesh_init(ch);
    }
}

static void chunk_mesh_init(chunk_cache ch)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    u64 *buf = &buffer[cur_buf][0];
    u64 buf_len = 0;
    u64 *cursor = buf;
    u32 *i = (u32*)ch.ch->block;
    u32 *end = i + CHUNK_VOLUME;
    u64 block_index;
    v3u64 pos;
    b8 should_render = FALSE;

    for (; i < end; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            should_render = TRUE;
            block_index = CHUNK_VOLUME - (end - i);
            pos.x = block_index % CHUNK_DIAMETER;
            pos.y = (block_index / CHUNK_DIAMETER) % CHUNK_DIAMETER;
            pos.z = block_index / CHUNK_LAYER;
            *(cursor++) = *i |
                (pos.x & 0xf) << SHIFT_BLOCK_X |
                (pos.y & 0xf) << SHIFT_BLOCK_Y |
                (pos.z & 0xf) << SHIFT_BLOCK_Z;
        }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;
    buf_len = cursor - buf;

    if (!ch.ch->vbo_len)
    {
        glGenVertexArrays(1, &ch.ch->vao);
        glGenBuffers(1, &ch.ch->vbo);
    }

    ch.ch->vbo_len = buf_len;

    glBindVertexArray(ch.ch->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ch.ch->vbo);
    glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(u64), buf, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)sizeof(u32));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    ch.ch->flag |= FLAG_CHUNK_GENERATED | FLAG_CHUNK_DIRTY;

    if (should_render)
    {
        ch.ch->flag |= FLAG_CHUNK_RENDER;
        ch.ch->color = CHUNK_COLOR_RENDER;
    }
    else
    {
        ch.ch->flag &= ~FLAG_CHUNK_RENDER;
        ch.ch->color = CHUNK_COLOR_LOADED;
    }

    chunk_gizmo_write_internal(ch);
}

static void chunk_mesh_update(chunk_cache ch)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    u64 *buf = &buffer[cur_buf][0];
    u64 buf_len = 0;
    u64 *cursor = buf;
    u32 *i = (u32*)ch.ch->block;
    u32 *end = i + CHUNK_VOLUME;
    u64 block_index;
    v3u64 pos;
    b8 should_render = FALSE;

    if (!ch.ch->vbo || !ch.ch->vao)
        return;

    for (; i < end; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            should_render = TRUE;
            block_index = CHUNK_VOLUME - (end - i);
            pos.x = block_index % CHUNK_DIAMETER;
            pos.y = (block_index / CHUNK_DIAMETER) % CHUNK_DIAMETER;
            pos.z = block_index / CHUNK_LAYER;
            *(cursor++) = *i |
                (pos.x & 0xf) << SHIFT_BLOCK_X |
                (pos.y & 0xf) << SHIFT_BLOCK_Y |
                (pos.z & 0xf) << SHIFT_BLOCK_Z;
        }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;
    buf_len = cursor - buf;
    ch.ch->vbo_len = buf_len;

    glBindBuffer(GL_ARRAY_BUFFER, ch.ch->vbo);
    glBufferData(GL_ARRAY_BUFFER, buf_len * sizeof(u64), buf, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    ch.ch->flag &= ~FLAG_CHUNK_DIRTY;

    if (should_render)
    {
        ch.ch->flag |= FLAG_CHUNK_RENDER;
        ch.ch->color = CHUNK_COLOR_RENDER;
    }
    else
    {
        ch.ch->flag &= ~FLAG_CHUNK_RENDER;
        ch.ch->color = CHUNK_COLOR_LOADED;
    }

    chunk_gizmo_write_internal(ch);
}

static void chunk_export_internal(chunk *ch)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};
    static u32 buf[CHUNK_VOLUME] = {0};
    u32 *blocks = (u32*)ch->block;
    u32 i = 0;
    u32 j = 0;
    u32 rle = 0; /* run-length */

    ch->flag &= ~FLAG_CHUNK_MODIFIED;

    snprintf(path, FSL_PATH_CAP,
            "%s"GAME_DIR_WORLD_NAME_CHUNKS FORMAT_FILE_NAME_HHCC,
            world.path, ch->pos.x, ch->pos.y, ch->pos.z);

    for (; i < CHUNK_VOLUME; ++j, i += rle, blocks += rle)
    {
        buf[j] = *blocks;
        rle = fsl_rle(blocks, sizeof(u32), CHUNK_VOLUME - i);
        if (rle > 1)
        {
            buf[j++] |= FLAG_BLOCK_RLE;
            buf[j] = rle;
        }
    }

    fsl_write_file(path, j * sizeof(u32), buf, TRUE, FALSE);
}

static void chunk_import_internal(const fsl_fs_path *path, chunk *ch)
{
    FILE *file = NULL;
    str file_name[FSL_ID_CAP] = {0};
    str *cursor = file_name;
    i64 pos_cache[3] = {0};
    static u32 buf[CHUNK_VOLUME] = {0};
    u32 *blocks = (u32*)ch->block;
    u32 i = 0;
    u32 j = 0;
    u32 rle = 0;

    fsl_get_base_name(path, file_name, FSL_ID_CAP);

    for (i = 0; i < 3; ++i)
    {
        fsl_convert_str_to_i64(cursor, &pos_cache[i]);
        cursor = strchr(cursor, '.') + 1;
        if (!cursor)
            break;
    }

    ch->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_IMPORTED | FLAG_CHUNK_DIRTY;
    ch->pos.x = pos_cache[0];
    ch->pos.y = pos_cache[1];
    ch->pos.z = pos_cache[2];

    ch->cursor = CHUNK_VOLUME;

    file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    i = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(buf, 1, i, file);
    fclose(file);

    for (i = 0; i < CHUNK_VOLUME && j < CHUNK_VOLUME; ++i)
    {
        if (buf[i] & FLAG_BLOCK_RLE)
        {
            buf[i] &= ~FLAG_BLOCK_RLE;
            rle = buf[i + 1];
            while (rle--)
                blocks[j++] = buf[i];
            ++i;
        }
        else
            blocks[j++] = buf[i];
    }
}

static void chunk_buf_push_internal(u32 index, v3i32 player_chunk_delta)
{
    chunk nochunk = {0};
    chunk *ch = NULL;
    chunk *end = NULL;
    v3u32 chunk_tab_coordinates = {0};
    v3u64 seed;
    v3u8 color_variant;

    chunk_tab_coordinates.x = index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = index / settings.chunk_buf_layer;

    ch = &chunk_buf.p[chunk_buf.cursor];
    end = &chunk_buf.p[CHUNKS_MAX[settings.render_distance]];
    while (ch < end)
    {
        if (!(ch->flag & FLAG_CHUNK_LOADED))
        {
            if (ch->vbo_len)
            {
                glDeleteBuffers(1, &ch->vbo);
                glDeleteVertexArrays(1, &ch->vao);
            }
            *ch = nochunk;

            ch->pos.x = player_chunk_delta.x + chunk_tab_coordinates.x - settings.chunk_buf_radius;
            ch->pos.y = player_chunk_delta.y + chunk_tab_coordinates.y - settings.chunk_buf_radius;
            ch->pos.z = player_chunk_delta.z + chunk_tab_coordinates.z - settings.chunk_buf_radius;

            ch->id =
                (u64)(ch->pos.x & 0xffff) << 0x00 |
                (u64)(ch->pos.y & 0xffff) << 0x10 |
                (u64)(ch->pos.z & 0xffff) << 0x20;

            ch->color = CHUNK_COLOR_LOADED;

            seed.x = chunk_tab_coordinates.x * index + player_chunk_delta.z;
            seed.y = chunk_tab_coordinates.y * index + player_chunk_delta.x;
            seed.z = chunk_tab_coordinates.z * index + player_chunk_delta.y;
            color_variant.x =
                (u8)(fsl_map_range_f64((f64)(fsl_rand_u64(seed.x) % 0xff), 0.0, 0xff,
                            1.0 - CHUNK_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);
            color_variant.y =
                (u8)(fsl_map_range_f64((f64)(fsl_rand_u64(seed.y) % 0xff), 0.0, 0xff,
                            1.0 - CHUNK_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);
            color_variant.z =
                (u8)(fsl_map_range_f64((f64)(fsl_rand_u64(seed.z) % 0xff), 0.0, 0xff,
                            1.0 - CHUNK_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);

            ch->color_variant = 0 |
                (color_variant.x << 0x18) |
                (color_variant.y << 0x10) |
                (color_variant.z << 0x08);

            ch->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_DIRTY;
            chunk_tab.p[index] = ch;
            ++chunk_buf.cursor;
            return;
        }
        ch++;
    }

    LOGERROR(FSL_ERR_BUFFER_FULL,
            FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            fsl_logger_stringf("'%s'\n", "`chunk_buf` Full"));
}

static void chunk_buf_pop_internal(u32 index)
{
    u32 index_popped = chunk_tab.p[index] - chunk_buf.p;
    chunk_cache cache = {0};

    cache.ch = chunk_tab.p[index];
    cache.index = index;
    chunk_gizmo_write_internal(cache);

    if (cache.ch->vbo_len)
    {
        glDeleteBuffers(1, &cache.ch->vbo);
        glDeleteVertexArrays(1, &cache.ch->vao);
        cache.ch->vbo_len = 0;
    }

    cache.ch->flag = 0;
    if (chunk_buf.cursor > index_popped)
        chunk_buf.cursor = index_popped;
    chunk_tab.p[index] = NULL;
}

static void chunk_queue_update_internal(chunk_queue *q, fsl_len len,
        b8 should_push, b8 should_pop)
{
    chunk ***ch = NULL;
    chunk ***end = NULL;
    chunk_cache cache = {0};
    u32 queue_len = q->len;
    u32 push = q->cursor_push;
    u32 pop = q->cursor_pop;
    u32 rate_chunk = q->rate_chunk;
    u32 rate_block = q->rate_block;
    u32 i = 0;

    if (!should_push)
        goto pop;

    /* ---- push chunk queue ------------------------------------------------ */

    ch = &CHUNK_ORDER.p[q->offset];
    end = CHUNK_ORDER.p + len;
    for (; ch < end && q->count < queue_len; ++ch)
    {
        cache.ch = **ch;
        if (cache.ch && (cache.ch->flag & FLAG_CHUNK_DIRTY) &&
                !(cache.ch->flag & FLAG_CHUNK_QUEUED) &&
                !q->queue_p[push].ch)
        {
            cache.ch->flag |= FLAG_CHUNK_QUEUED;
            q->queue_p[push].ch = cache.ch;
            q->queue_p[push].index = *ch - chunk_tab.p;
            ++q->count;
            ++push;
            if (push == queue_len)
                push = 0;
        }
    }

    q->cursor_push = push;

    /* ---- pop chunk queue ------------------------------------------------- */

pop:

    if (!should_pop)
        return;

    for (i = 0; q->count && rate_chunk && i < queue_len; ++i)
    {
        if (q->queue_p[pop].ch)
        {
            cache = q->queue_p[pop];
            if (cache.ch->flag & FLAG_CHUNK_GENERATED)
                chunk_mesh_update(cache);
            else
                chunk_generate(cache, rate_block);

            if (!(cache.ch->flag & FLAG_CHUNK_DIRTY))
            {
                if (cache.ch->flag & FLAG_CHUNK_MODIFIED)
                    chunk_export_internal(cache.ch);
                cache.ch->flag &= ~FLAG_CHUNK_QUEUED;
                q->queue_p[pop].ch = NULL;
                --q->count;
                ++pop;
                if (pop == queue_len)
                    pop = 0;
            }

            if (cache.ch->flag & FLAG_CHUNK_IMPORTED)
                cache.ch->flag &= ~FLAG_CHUNK_IMPORTED;
            else
                --rate_chunk;
        }
    }

    q->cursor_pop = pop;
}

static void chunk_queue_reset_internal(chunk_queue *q)
{
    u32 pop = q->cursor_pop;
    u32 count = q->count;

    while (count--)
    {
        q->queue_p[pop].ch->flag &= ~FLAG_CHUNK_QUEUED;
        q->queue_p[pop].ch = NULL;
        ++pop;
        if (pop == q->len)
            pop = 0;
    }

    q->count = 0;
    q->cursor_pop = pop;
}

static void chunk_gizmo_write_internal(chunk_cache ch)
{
    v3u32 chunk_pos = {0};
    v4u32 chunk_color = {0};

    chunk_pos.x = ch.index % settings.chunk_buf_diameter;
    chunk_pos.y = (ch.index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_pos.z = ch.index / settings.chunk_buf_layer;

    chunk_color.x = (ch.ch->color >> 0x18) & 0xff;
    chunk_color.y = (ch.ch->color >> 0x10) & 0xff;
    chunk_color.z = (ch.ch->color >> 0x08) & 0xff;
    chunk_color.w = (ch.ch->color >> 0x00) & 0xff;

    chunk_color.x = (chunk_color.x + ((ch.ch->color_variant >> 0x18) & 0xff)) / 2;
    chunk_color.y = (chunk_color.y + ((ch.ch->color_variant >> 0x10) & 0xff)) / 2;
    chunk_color.z = (chunk_color.z + ((ch.ch->color_variant >> 0x08) & 0xff)) / 2;

    if (ch.ch->flag & FLAG_CHUNK_RENDER)
    {
        chunk_gizmo_render.p[ch.index].x =
            (chunk_pos.x << 0x18) | (chunk_pos.y << 0x10) | (chunk_pos.z << 0x08);
        chunk_gizmo_render.p[ch.index].y =
            (chunk_color.x << 0x18) |
            (chunk_color.y << 0x10) |
            (chunk_color.z << 0x08) |
            (chunk_color.w << 0x00);
        chunk_gizmo_loaded.p[ch.index].y = 0;
    }
    else if (ch.ch->flag & FLAG_CHUNK_LOADED)
    {
        chunk_gizmo_loaded.p[ch.index].x =
            (chunk_pos.x << 0x18) | (chunk_pos.y << 0x10) | (chunk_pos.z << 0x08);
        chunk_gizmo_loaded.p[ch.index].y =
            (chunk_color.x << 0x18) |
            (chunk_color.y << 0x10) |
            (chunk_color.z << 0x08) |
            (chunk_color.w << 0x00);
        chunk_gizmo_render.p[ch.index].y = 0;
    }
    else
    {
        chunk_gizmo_loaded.p[ch.index].y = 0;
        chunk_gizmo_render.p[ch.index].y = 0;
    }

    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_loaded.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, ch.index * sizeof(v2u32), sizeof(v2u32),
            &chunk_gizmo_loaded.p[ch.index]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_render.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, ch.index * sizeof(v2u32), sizeof(v2u32),
            &chunk_gizmo_render.p[ch.index]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

u32 *get_block_resolved(chunk *ch, i32 x, i32 y, i32 z)
{
    x = fsl_mod_i32(x, CHUNK_DIAMETER);
    y = fsl_mod_i32(y, CHUNK_DIAMETER);
    z = fsl_mod_i32(z, CHUNK_DIAMETER);
    return &ch->block[z][y][x];
}

chunk *get_chunk_resolved(u32 index, i32 x, i32 y, i32 z)
{
    x = (i32)floorf((f32)x / CHUNK_DIAMETER);
    y = (i32)floorf((f32)y / CHUNK_DIAMETER);
    z = (i32)floorf((f32)z / CHUNK_DIAMETER);
    return chunk_tab.p[index + x +
        y * settings.chunk_buf_diameter +
        z * settings.chunk_buf_layer];
}

u32 get_chunk_index(v3i32 chunk_pos, v3f64 pos)
{
    v3i32 offset = {0};
    u32 index = 0;
    offset.x = (i32)floorf(pos.x / CHUNK_DIAMETER) - chunk_pos.x + settings.chunk_buf_radius,
    offset.y = (i32)floorf(pos.y / CHUNK_DIAMETER) - chunk_pos.y + settings.chunk_buf_radius,
    offset.z = (i32)floorf(pos.z / CHUNK_DIAMETER) - chunk_pos.z + settings.chunk_buf_radius,
    index =
        offset.x +
        offset.y * settings.chunk_buf_diameter +
        offset.z * settings.chunk_buf_layer;

    if (index >= settings.chunk_buf_volume)
        return settings.chunk_tab_center;
    return index;
}
