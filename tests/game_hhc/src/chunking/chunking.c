#include "deps/fossil/common/limits.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/string/string.h"

#include "deps/fossil/h/dir.h"

#include "../h/assets.h"
#include "chunking.h"
#include "chunking_internal.h"
#include "../h/common.h"
#include "../h/diagnostics.h"
#include "../h/main.h"
#include "../h/terrain.h"
#include "../h/world.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

/* ---- section: declarations ----------------------------------------------- */

/*!
 *  @internal
 *
 *  @brief memory arena used to store all chunk buffers and data.
 */
static fsl_mem_arena memory_arena_chunking_internal = {0};

u64 CHUNKS_MAX[SET_RENDER_DISTANCE_MAX + 1] = {0};
static chunk_buffer chunk_buf = {0};
chunk_table chunk_tab = {0};
chunk_order CHUNK_ORDER = {0};
chunk_scheduler chunk_sched[CHUNK_SCHEDULERS_MAX] = {0};
chunk_gizmo chunk_gizmo_loaded = {0};
chunk_gizmo chunk_gizmo_render = {0};

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
                CHUNK_BUF_VOLUME_MAX * sizeof(hhc_chunk**),
                "chunking_init().CHUNK_ORDER.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_tab.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(hhc_chunk*),
                "chunking_init().chunk_tab.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_buf.handle,
                CHUNK_BUF_VOLUME_MAX * sizeof(hhc_chunk),
                "chunking_init().chunk_buf.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_sched[0].schedule,
                CHUNK_SCHEDULER_1ST_MAX * sizeof(hhc_chunk*),
                "chunking_init().chunk_sched[0].schedule") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_sched[1].schedule,
                CHUNK_SCHEDULER_2ND_MAX * sizeof(hhc_chunk*),
                "chunking_init().chunk_sched[1].schedule") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunking_internal, &chunk_sched[2].schedule,
                CHUNK_SCHEDULER_3RD_MAX * sizeof(hhc_chunk*),
                "chunking_init().chunk_sched[2].schedule") != FSL_ERR_SUCCESS ||

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
    chunk_sched[0].p = fsl_mem_handle_get(chunk_sched[0].schedule);
    chunk_sched[1].p = fsl_mem_handle_get(chunk_sched[1].schedule);
    chunk_sched[2].p = fsl_mem_handle_get(chunk_sched[2].schedule);
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

    /* ---- init chunk parsing priority schedulers -------------------------- */

    chunk_scheduler_set_parameters_internal(&chunk_sched[0], CHUNK_SCHEDULER_ID_1ST,
        0,
        CHUNK_SCHEDULER_1ST_MAX,
        CHUNK_PARSE_RATE_PRIORITY_HIGH);

    chunk_scheduler_set_parameters_internal(&chunk_sched[1], CHUNK_SCHEDULER_ID_2ND,
        chunk_sched[0].len,
        CHUNK_SCHEDULER_2ND_MAX,
        CHUNK_PARSE_RATE_PRIORITY_MID);

    chunk_scheduler_set_parameters_internal(&chunk_sched[2], CHUNK_SCHEDULER_ID_3RD,
        chunk_sched[0].len + chunk_sched[1].len,
        CHUNK_SCHEDULER_3RD_MAX,
        CHUNK_PARSE_RATE_PRIORITY_LOW);

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
    hhc_chunk ***cursor = NULL;
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
    u32 chunk_push_index = 0;

    chunk_scheduler_update_internal(&chunk_sched[0],
            chunk_sched[0].offset + chunk_sched[0].len,
            TRUE, TRUE);

    chunk_scheduler_update_internal(&chunk_sched[1],
            chunk_sched[1].offset + chunk_sched[1].len,
            TRUE, !chunk_sched[0].count && chunk_sched[1].len);

    chunk_scheduler_update_internal(&chunk_sched[2],
            CHUNKS_MAX[settings.render_distance],
            TRUE, !chunk_sched[1].count && chunk_sched[2].len);

    DELTA.x = player_chunk.x - player_chunk_delta->x;
    DELTA.y = player_chunk.y - player_chunk_delta->y;
    DELTA.z = player_chunk.z - player_chunk_delta->z;

    if (!(DELTA.x || DELTA.y || DELTA.z))
        return;

chunk_tab_shift:

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
                chunk_buf_pop_internal(**cursor);
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
            chunk_tab.p[i]->index = i;
            chunk_gizmo_write_internal(chunk_tab.p[i]);
            if (chunk_tab.p[i]->flag & FLAG_CHUNK_EDGE)
                chunk_tab.p[target_index] = NULL;

            chunk_tab.p[i]->flag &= ~FLAG_CHUNK_EDGE;
        }
    }

    if (DELTA.x || DELTA.y || DELTA.z)
    {
        DELTA.x = player_chunk.x - player_chunk_delta->x;
        DELTA.y = player_chunk.y - player_chunk_delta->y;
        DELTA.z = player_chunk.z - player_chunk_delta->z;
        goto chunk_tab_shift;
    }

chunk_buf_push:

    for (i = 0; i < (i64)CHUNKS_MAX[settings.render_distance]; ++i)
    {
        if (!*CHUNK_ORDER.p[i])
        {
            chunk_push_index = CHUNK_ORDER.p[i] - chunk_tab.p;
            chunk_buf_push_internal(chunk_push_index, *player_chunk_delta);
        }
    }
}

void chunking_free(void)
{
    if (chunk_tab.p)
    {
        u32 i = 0;
        for (; i < settings.chunk_buf_volume; ++i)
            if (chunk_tab.p[i])
                chunk_buf_pop_internal(chunk_tab.p[i]);
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
        glDeleteBuffers(1, &chunk_gizmo_render.vbo);
        glDeleteVertexArrays(1, &chunk_gizmo_render.vao);
    }

    fsl_mem_arena_free(&memory_arena_chunking_internal,
                "chunking_init().memory_arena_chunking_internal");
}

u32 block_get_faces_internal(const hhc_chunk *ch,
        const hhc_chunk *px, const hhc_chunk *nx,
        const hhc_chunk *py, const hhc_chunk *ny,
        const hhc_chunk *pz, const hhc_chunk *nz,
        i32 x, i32 y, i32 z)
{
    u32 block = GET_BLOCK_ID(ch->block[z][y][x]);

    if (x == CHUNK_DIAMETER - 1)
    {
        if (!px || !px->block[z][y][0])
            block |= FLAG_BLOCK_FACE_PX;
    }
    else if (!ch->block[z][y][x + 1])
        block |= FLAG_BLOCK_FACE_PX;

    if (x == 0)
    {
        if (!nx || !nx->block[z][y][CHUNK_DIAMETER - 1])
            block |= FLAG_BLOCK_FACE_NX;
    }
    else if (!ch->block[z][y][x - 1])
        block |= FLAG_BLOCK_FACE_NX;

    if (y == CHUNK_DIAMETER - 1)
    {
        if (!py || !py->block[z][0][x])
            block |= FLAG_BLOCK_FACE_PY;
    }
    else if (!ch->block[z][y + 1][x])
        block |= FLAG_BLOCK_FACE_PY;

    if (y == 0)
    {
        if (!ny || !ny->block[z][CHUNK_DIAMETER - 1][x])
            block |= FLAG_BLOCK_FACE_NY;
    }
    else if (!ch->block[z][y - 1][x])
        block |= FLAG_BLOCK_FACE_NY;

    if (z == CHUNK_DIAMETER - 1)
    {
        if (!pz || !pz->block[0][y][x])
            block |= FLAG_BLOCK_FACE_PZ;
    }
    else if (!ch->block[z + 1][y][x])
        block |= FLAG_BLOCK_FACE_PZ;

    if (z == 0)
    {
        if (!nz || !nz->block[CHUNK_DIAMETER - 1][y][x])
            block |= FLAG_BLOCK_FACE_NZ;
    }
    else if (!ch->block[z - 1][y][x])
        block |= FLAG_BLOCK_FACE_NZ;

    return block;
}

block_hit block_hit_get(v3f64 origin, f64 start_x, f64 start_y, f64 start_z,
        f64 end_x, f64 end_y, f64 end_z, f64 distance_max)
{
    const v3i64 WORLD_VOLUME_MIN = {-WORLD_DIAMETER, -WORLD_DIAMETER, -WORLD_DIAMETER_VERTICAL};
    const v3i64 WORLD_VOLUME_MAX = {WORLD_DIAMETER, WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL};
    block_hit hit = {0};
    v3f64 delta = {0};
    v3f64 direction = {0};
    v3f64 distance = {0};
    v3i32 step = {1, 1, 1};
    hhc_chunk *ch = NULL;
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;
    i32 chx = 0;
    i32 chy = 0;
    i32 chz = 0;

    direction.x = end_x - start_x;
    direction.y = end_y - start_y;
    direction.z = end_z - start_z;
    direction = fsl_normalize_v3f64(direction);
    delta.x = direction.x == 0.0 ? INFINITY : fabs(1.0 / direction.x);
    delta.y = direction.y == 0.0 ? INFINITY : fabs(1.0 / direction.y);
    delta.z = direction.z == 0.0 ? INFINITY : fabs(1.0 / direction.z);

    hit.pos.x = (i64)floor(start_x);
    hit.pos.y = (i64)floor(start_y);
    hit.pos.z = (i64)floor(start_z);

    if (direction.x < 0.0f)
    {
        distance.x = (start_x - hit.pos.x) * delta.x;
        step.x = -1;
    }
    else distance.x = (hit.pos.x + 1.0 - start_x) * delta.x;

    if (direction.y < 0.0f)
    {
        distance.y = (start_y - hit.pos.y) * delta.y;
        step.y = -1;
    }
    else distance.y = (hit.pos.y + 1.0 - start_y) * delta.y;

    if (direction.z < 0.0f)
    {
        distance.z = (start_z - hit.pos.z) * delta.z;
        step.z = -1;
    }
    else distance.z = (hit.pos.z + 1.0 - start_z) * delta.z;

    while (fsl_min_v3f64(distance) < distance_max)
    {
        switch (fsl_min_axis_v3f64(distance))
        {
            case 1:
                hit.pos.x += step.x;
                distance.x += delta.x;
                hit.normal.x = -step.x;
                hit.normal.y = 0.0;
                hit.normal.z = 0.0;
                break;

            case 2:
                hit.pos.y += step.y;
                distance.y += delta.y;
                hit.normal.x = 0.0;
                hit.normal.y = -step.y;
                hit.normal.z = 0.0;
                break;

            case 3:
                hit.pos.z += step.z;
                distance.z += delta.z;
                hit.normal.x = 0.0;
                hit.normal.y = 0.0;
                hit.normal.z = -step.z;
                break;
        }

        x = hit.pos.x - (i32)floorf((f32)origin.x / CHUNK_DIAMETER) * CHUNK_DIAMETER;
        y = hit.pos.y - (i32)floorf((f32)origin.y / CHUNK_DIAMETER) * CHUNK_DIAMETER;
        z = hit.pos.z - (i32)floorf((f32)origin.z / CHUNK_DIAMETER) * CHUNK_DIAMETER;

        chx = floorf((f32)x / CHUNK_DIAMETER);
        chy = floorf((f32)y / CHUNK_DIAMETER);
        chz = floorf((f32)z / CHUNK_DIAMETER);
        ch = chunk_tab.p[settings.chunk_tab_center +
            chx +
            chy * settings.chunk_buf_diameter +
            chz * settings.chunk_buf_layer];
        if (!ch || !(ch->flag & FLAG_CHUNK_GENERATED))
            continue;

        x = fsl_mod_i32(x, CHUNK_DIAMETER);
        y = fsl_mod_i32(y, CHUNK_DIAMETER);
        z = fsl_mod_i32(z, CHUNK_DIAMETER);
        if (!ch->block[z][y][x])
            continue;

        if (!fsl_is_in_volume_i64(hit.pos, WORLD_VOLUME_MIN, WORLD_VOLUME_MAX))
            break;

        hit.block = &ch->block[z][y][x];
        hit.hit = TRUE;
        break;
    }

    return hit;
}

void block_place(block_hit hit, enum block_id block_id)
{
    u32 index = chunk_tab.index;
    hhc_chunk **ch = NULL;
    hhc_chunk *px = NULL;
    hhc_chunk *nx = NULL;
    hhc_chunk *py = NULL;
    hhc_chunk *ny = NULL;
    hhc_chunk *pz = NULL;
    hhc_chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};

    if (!hit.hit)
        return;

    /* canonicalize block position */
    hit.pos.x -= chunk_tab.p[index]->pos.x * CHUNK_DIAMETER;
    hit.pos.y -= chunk_tab.p[index]->pos.y * CHUNK_DIAMETER;
    hit.pos.z -= chunk_tab.p[index]->pos.z * CHUNK_DIAMETER;

    /* ---- get block at direction of normals ------------------------------- */

    if (hit.normal.x == 0.0f && hit.normal.y == 0.0f && hit.normal.z == 0.0f)
        return;

    /* get block position at normal direction */
    hit.pos.x += (i32)hit.normal.x;
    hit.pos.y += (i32)hit.normal.y;
    hit.pos.z += (i32)hit.normal.z;

    /* get the chunk the new block index is in and make sure it's within bounds */
    index += (i32)floorf((f32)hit.pos.x / CHUNK_DIAMETER);
    index += (i32)floorf((f32)hit.pos.y / CHUNK_DIAMETER) * settings.chunk_buf_diameter;
    index += (i32)floorf((f32)hit.pos.z / CHUNK_DIAMETER) * settings.chunk_buf_layer;
    if (index >= settings.chunk_buf_volume)
        return;

    hit.pos.x = fsl_mod_i32(hit.pos.x, CHUNK_DIAMETER);
    hit.pos.y = fsl_mod_i32(hit.pos.y, CHUNK_DIAMETER);
    hit.pos.z = fsl_mod_i32(hit.pos.z, CHUNK_DIAMETER);

    ch = &chunk_tab.p[index];
    if ((*ch)->block[hit.pos.z][hit.pos.y][hit.pos.x] || !block_id)
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

    block_add_internal(*ch, px, nx, py, ny, pz, nz, hit.pos.x, hit.pos.y, hit.pos.z, block_id);
}

void block_add_internal(hhc_chunk *ch,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z, enum block_id block_id)
{
    ch->flag |= FLAG_CHUNK_DIRTY;
    SET_BLOCK_ID(ch->block[z][y][x], block_id);
    ch->block[z][y][x] |= 63 << SHIFT_BLOCK_LIGHT;

    if (x == CHUNK_DIAMETER - 1 && px && px->block[z][y][0])
        px->flag |= FLAG_CHUNK_DIRTY;

    if (x == 0 && nx && nx->block[z][y][CHUNK_DIAMETER - 1])
        nx->flag |= FLAG_CHUNK_DIRTY;

    if (y == CHUNK_DIAMETER - 1 && py && py->block[z][0][x])
        py->flag |= FLAG_CHUNK_DIRTY;

    if (y == 0 && ny && ny->block[z][CHUNK_DIAMETER - 1][x])
        ny->flag |= FLAG_CHUNK_DIRTY;

    if (z == CHUNK_DIAMETER - 1 && pz && pz->block[0][y][x])
        pz->flag |= FLAG_CHUNK_DIRTY;

    if (z == 0 && nz && nz->block[CHUNK_DIAMETER - 1][y][x])
        nz->flag |= FLAG_CHUNK_DIRTY;

    block_evaluate_internal(ch, px, nx, py, ny, pz, nz, x, y, z, block_id);
}

void block_break(block_hit hit)
{
    u32 index = chunk_tab.index;
    hhc_chunk **ch = NULL;
    hhc_chunk *px = NULL;
    hhc_chunk *nx = NULL;
    hhc_chunk *py = NULL;
    hhc_chunk *ny = NULL;
    hhc_chunk *pz = NULL;
    hhc_chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};

    if (!hit.hit)
        return;

    /* get the chunk the new block index is in and make sure it's within bounds */
    index += (i32)floorf((f32)hit.pos.x / CHUNK_DIAMETER);
    index += (i32)floorf((f32)hit.pos.y / CHUNK_DIAMETER) * settings.chunk_buf_diameter;
    index += (i32)floorf((f32)hit.pos.z / CHUNK_DIAMETER) * settings.chunk_buf_layer;
    if (index >= settings.chunk_buf_volume)
        return;

    hit.pos.x -= chunk_tab.p[index]->pos.x * CHUNK_DIAMETER;
    hit.pos.y -= chunk_tab.p[index]->pos.y * CHUNK_DIAMETER;
    hit.pos.z -= chunk_tab.p[index]->pos.z * CHUNK_DIAMETER;

    hit.pos.x = fsl_mod_i32(hit.pos.x, CHUNK_DIAMETER);
    hit.pos.y = fsl_mod_i32(hit.pos.y, CHUNK_DIAMETER);
    hit.pos.z = fsl_mod_i32(hit.pos.z, CHUNK_DIAMETER);

    ch = &chunk_tab.p[index];
    if (!(*ch)->block[hit.pos.z][hit.pos.y][hit.pos.x])
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

    block_remove_internal(*ch, px, nx, py, ny, pz, nz, hit.pos.x, hit.pos.y, hit.pos.z);
}

void block_remove_internal(hhc_chunk *ch,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z)
{
    ch->flag |= FLAG_CHUNK_DIRTY;
    ch->block[z][y][x] = 0;

    if (x == CHUNK_DIAMETER - 1 && px && px->block[z][y][0])
        px->flag |= FLAG_CHUNK_DIRTY;

    if (x == 0 && nx && nx->block[z][y][CHUNK_DIAMETER - 1])
        nx->flag |= FLAG_CHUNK_DIRTY;

    if (y == CHUNK_DIAMETER - 1 && py && py->block[z][0][x])
        py->flag |= FLAG_CHUNK_DIRTY;

    if (y == 0 && ny && ny->block[z][CHUNK_DIAMETER - 1][x])
        ny->flag |= FLAG_CHUNK_DIRTY;

    if (z == CHUNK_DIAMETER - 1 && pz && pz->block[0][y][x])
        pz->flag |= FLAG_CHUNK_DIRTY;

    if (z == 0 && nz && nz->block[CHUNK_DIAMETER - 1][y][x])
        nz->flag |= FLAG_CHUNK_DIRTY;
}

void block_evaluate_internal(hhc_chunk *ch,
        hhc_chunk *px, hhc_chunk *nx,
        hhc_chunk *py, hhc_chunk *ny,
        hhc_chunk *pz, hhc_chunk *nz,
        i32 x, i32 y, i32 z, enum block_id block_id)
{
    if (z == CHUNK_DIAMETER - 1)
    {
        if (pz && pz->block[0][y][x])
        {
            pz->flag |= FLAG_CHUNK_DIRTY;

            if (GET_BLOCK_ID(ch->block[z][y][x]) == BLOCK_GRASS)
                SET_BLOCK_ID(ch->block[z][y][x], BLOCK_DIRT);
        }
    }

    if (z == 0)
    {
        if (nz && nz->block[CHUNK_DIAMETER - 1][y][x])
        {
            nz->flag |= FLAG_CHUNK_DIRTY;

            if (GET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x]) == BLOCK_GRASS)
                SET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x], BLOCK_DIRT);
        }
    }
    else if (GET_BLOCK_ID(ch->block[z - 1][y][x]) == BLOCK_GRASS)
    {
        ch->flag |= FLAG_CHUNK_DIRTY;

        SET_BLOCK_ID(ch->block[z - 1][y][x], BLOCK_DIRT);
    }
}

chunk_scheduler_cost chunk_load_internal(hhc_chunk *ch, u32 rate)
{
    chunk_scheduler_cost cost = 0;
    fsl_fs_path path[FSL_PATH_CAP] = {0};

    if (!ch || ch->flag & FLAG_CHUNK_GENERATED)
        return 0;

    snprintf(path, FSL_PATH_CAP,
            "%s"GAME_DIR_WORLD_NAME_CHUNKS FORMAT_FILE_NAME_HHCC,
            world.path, ch->pos.x, ch->pos.y, ch->pos.z);

    if (fsl_is_file_exists(path, FALSE) == FSL_ERR_SUCCESS)
        cost += chunk_import_internal(path, ch);
    cost += chunk_generate_internal(ch, rate);
    return cost;
}

chunk_scheduler_cost chunk_generate_internal(hhc_chunk *ch, u32 rate)
{
    chunk_scheduler_cost cost = rate;
    hhc_chunk *px = NULL;
    hhc_chunk *nx = NULL;
    hhc_chunk *py = NULL;
    hhc_chunk *ny = NULL;
    hhc_chunk *pz = NULL;
    hhc_chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};
    v3i32 coordinates = {0};
    v3i32 block_pos_offset = {0};
    terrain terrain_info = {0};
    i32 x = 0, y = 0, z = 0;

    if (ch->cursor == CHUNK_VOLUME)
    {
        ch->flag |= FLAG_CHUNK_GENERATED;
        cost = chunk_mesh_update_internal(ch);
        return cost;
    }

    block_pos_offset.x = ch->pos.x * CHUNK_DIAMETER;
    block_pos_offset.y = ch->pos.y * CHUNK_DIAMETER;
    block_pos_offset.z = ch->pos.z * CHUNK_DIAMETER;

    chunk_tab_coordinates.x = ch->index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (ch->index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = ch->index / settings.chunk_buf_layer;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = chunk_tab.p[ch->index + 1];
    if (chunk_tab_coordinates.x > 0)
        nx = chunk_tab.p[ch->index - 1];
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = chunk_tab.p[ch->index + settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.y > 0)
        ny = chunk_tab.p[ch->index - settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = chunk_tab.p[ch->index + settings.chunk_buf_layer];
    if (chunk_tab_coordinates.z > 0)
        nz = chunk_tab.p[ch->index - settings.chunk_buf_layer];

    x = ch->cursor % CHUNK_DIAMETER;
    y = (ch->cursor / CHUNK_DIAMETER) % CHUNK_DIAMETER;
    z = ch->cursor / CHUNK_LAYER;

    /* x, y and z reset at the end of their loops because they should first pick up from
     * where `ch->cursor` left off last time. */
    for (; z < CHUNK_DIAMETER; ++z)
    {
        coordinates.z = z + block_pos_offset.z;
        for (; y < CHUNK_DIAMETER; ++y)
        {
            coordinates.y = y + block_pos_offset.y;
            for (; x < CHUNK_DIAMETER; ++x)
            {
                coordinates.x = x + block_pos_offset.x;

                /* heavy perlin-noise calls function */
                terrain_info = world.terrain_func(coordinates);

                if (terrain_info.block_id)
                {
                    block_add_internal(ch, px, nx, py, ny, pz, nz, x, y, z, terrain_info.block_id);
                    ch->block[z][y][x] |= terrain_info.block_light;
                }
                --rate;
                if (!rate)
                    goto finished_generation;
            }
            x = 0;
        }
        y = 0;
    }

finished_generation:

    ch->cursor = x + y * CHUNK_DIAMETER + z * CHUNK_LAYER;

    cost -= rate;
    return cost;
}

chunk_scheduler_cost chunk_mesh_update_internal(hhc_chunk *ch)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    hhc_chunk *px = NULL;
    hhc_chunk *nx = NULL;
    hhc_chunk *py = NULL;
    hhc_chunk *ny = NULL;
    hhc_chunk *pz = NULL;
    hhc_chunk *nz = NULL;
    v3u32 chunk_tab_coordinates = {0};

    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    u32 *start = (u32*)ch->block;
    u32 *end = start + CHUNK_VOLUME;
    u32 *curr = start;
    u64 block_index = 0;
    u32 block_cache = 0;
    v3u64 pos = {0};
    b8 should_render = FALSE;

    chunk_tab_coordinates.x = ch->index % settings.chunk_buf_diameter;
    chunk_tab_coordinates.y = (ch->index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_tab_coordinates.z = ch->index / settings.chunk_buf_layer;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = chunk_tab.p[ch->index + 1];
    if (chunk_tab_coordinates.x > 0)
        nx = chunk_tab.p[ch->index - 1];
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = chunk_tab.p[ch->index + settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.y > 0)
        ny = chunk_tab.p[ch->index - settings.chunk_buf_diameter];
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = chunk_tab.p[ch->index + settings.chunk_buf_layer];
    if (chunk_tab_coordinates.z > 0)
        nz = chunk_tab.p[ch->index - settings.chunk_buf_layer];

    for (; curr < end; ++curr)
    {
        if (*curr & MASK_BLOCK_ID)
        {
            block_index = curr - start;
            pos.x = block_index % CHUNK_DIAMETER;
            pos.y = (block_index / CHUNK_DIAMETER) % CHUNK_DIAMETER;
            pos.z = block_index / CHUNK_LAYER;
            block_cache = block_get_faces_internal(ch, px, nx, py, ny, pz, nz, pos.x, pos.y, pos.z);
            if (block_cache & MASK_BLOCK_FACES)
            {
                should_render = TRUE;
                *(cursor++) = block_cache |
                    (pos.x & 0xf) << SHIFT_BLOCK_X |
                    (pos.y & 0xf) << SHIFT_BLOCK_Y |
                    (pos.z & 0xf) << SHIFT_BLOCK_Z;
            }
        }
    }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;

    if (should_render)
    {
        ch->flag |= FLAG_CHUNK_RENDER;
        ch->color = CHUNK_COLOR_RENDER;

        if (ch->vbo_len)
        {
            glBindBuffer(GL_ARRAY_BUFFER, ch->vbo);
            glBufferData(GL_ARRAY_BUFFER, (cursor - buf) * sizeof(u64), buf, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            glGenVertexArrays(1, &ch->vao);
            glGenBuffers(1, &ch->vbo);

            glBindVertexArray(ch->vao);
            glBindBuffer(GL_ARRAY_BUFFER, ch->vbo);
            glBufferData(GL_ARRAY_BUFFER, (cursor - buf) * sizeof(u64), buf, GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)sizeof(u32));

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        ch->vbo_len = cursor - buf;
    }
    else
    {
        ch->flag &= ~FLAG_CHUNK_RENDER;
        ch->color = CHUNK_COLOR_LOADED;
    }

    ch->flag &= ~FLAG_CHUNK_DIRTY;
    chunk_gizmo_write_internal(ch);

    return CHUNK_PARSE_COST_MESHING;
}

chunk_scheduler_cost chunk_export_internal(hhc_chunk *ch)
{
    fsl_fs_path path[FSL_PATH_CAP] = {0};
    static u16 buf[CHUNK_VOLUME] = {0};
    u32 *blocks = (u32*)ch->block;
    u32 i = 0;
    u32 j = 0;
    u32 rle = 0; /* run-length */

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

    fsl_write_file(path, j * sizeof(u16), buf, TRUE, FALSE);

    return CHUNK_PARSE_COST_EXPORT;
}

chunk_scheduler_cost chunk_import_internal(const fsl_fs_path *path, hhc_chunk *ch)
{
    FILE *file = NULL;
    str file_name[FSL_ID_CAP] = {0};
    str *cursor = file_name;
    i64 pos_cache[3] = {0};
    static u16 buf[CHUNK_VOLUME] = {0};
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

    ch->cursor = j;

    return CHUNK_PARSE_COST_IMPORT;
}

void chunk_buf_push_internal(u32 index, v3i32 player_chunk_delta)
{
    hhc_chunk nochunk = {0};
    hhc_chunk *ch = NULL;
    hhc_chunk *end = NULL;
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
                ch->vbo_len = 0;
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

            ch->index = index;

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

void chunk_buf_pop_internal(hhc_chunk *ch)
{
    u32 index_popped = ch - chunk_buf.p;

    chunk_gizmo_write_internal(ch);

    if (ch->vbo_len)
    {
        glDeleteBuffers(1, &ch->vbo);
        glDeleteVertexArrays(1, &ch->vao);
        ch->vbo_len = 0;
    }

    ch->flag = 0;
    if (chunk_buf.cursor > index_popped)
        chunk_buf.cursor = index_popped;
    chunk_tab.p[ch->index] = NULL;
}

void chunk_scheduler_set_parameters_internal(chunk_scheduler *sched, chunk_scheduler_id id,
        u64 offset, fsl_len len, u32 rate_chunk)
{
    sched->id = id;
    sched->offset = offset;
    sched->len = (u64)fsl_clamp_i64(CHUNKS_MAX[settings.render_distance] - offset, 0, len);
    sched->rate_chunk = rate_chunk;
    sched->rate_block = BLOCK_PARSE_RATE;
}

void chunk_scheduler_update_internal(chunk_scheduler *sched, fsl_len len,
        b8 should_push, b8 should_pop)
{
    hhc_chunk ***start = NULL;
    hhc_chunk ***end = NULL;
    hhc_chunk *ch = NULL;
    u32 sched_len = sched->len;
    u32 push = sched->cursor_push;
    u32 pop = sched->cursor_pop;
    i32 rate_chunk = sched->rate_chunk;
    u32 rate_block = sched->rate_block;
    u32 i = 0;

    if (!should_push)
        goto pop;

    /* ---- push chunk scheduler -------------------------------------------- */

    start = &CHUNK_ORDER.p[sched->offset];
    end = CHUNK_ORDER.p + len;
    for (; start < end && rate_chunk > 0 && sched->count < sched_len; ++start)
    {
        if (**start)
        {
            ch = **start;
            if (ch->flag & FLAG_CHUNK_DIRTY &&
                    ch->sched_id != sched->id &&
                    !sched->p[push])
            {
                ch->sched_id = sched->id;
                sched->p[push] = ch;
                ++sched->count;
                ++push;
                if (push == sched_len)
                    push = 0;
                rate_chunk -= CHUNK_PARSE_COST_PUSH;
            }
        }
    }

    /* DISABLED IN v0.9.0-dev: better to use one budget for both push and pop loops per frame.
    rate_chunk = sched->rate_chunk;
    */
    sched->cursor_push = push;

    /* ---- pop chunk scheduler --------------------------------------------- */

pop:

    if (!should_pop)
        return;

    for (; sched->count && rate_chunk > 0 && i < sched_len; ++i)
    {
        if (sched->p[pop])
        {
            ch = sched->p[pop];

            if (ch->sched_id != sched->id)
            {
                sched->p[pop] = NULL;
                --sched->count;
                ++pop;
                if (pop == sched_len)
                    pop = 0;
                continue;
            }

            if (ch->flag & FLAG_CHUNK_GENERATED)
                rate_chunk -= chunk_mesh_update_internal(ch);
            else
                rate_chunk -= chunk_load_internal(ch, rate_block);

            if (!(ch->flag & FLAG_CHUNK_DIRTY))
            {
                if (!(ch->flag & FLAG_CHUNK_IMPORTED))
                    chunk_export_internal(ch);

                ch->flag &= ~FLAG_CHUNK_IMPORTED;
                ch->sched_id = 0;
                sched->p[pop] = NULL;
                --sched->count;
                ++pop;
                if (pop == sched_len)
                    pop = 0;
                rate_chunk -= CHUNK_PARSE_COST_POP;
            }
        }
    }

    sched->cursor_pop = pop;
}

void chunk_gizmo_write_internal(hhc_chunk *ch)
{
    v3u32 chunk_pos = {0};
    v4u32 chunk_color = {0};

    chunk_pos.x = ch->index % settings.chunk_buf_diameter;
    chunk_pos.y = (ch->index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter;
    chunk_pos.z = ch->index / settings.chunk_buf_layer;

    chunk_color.x = (ch->color >> 0x18) & 0xff;
    chunk_color.y = (ch->color >> 0x10) & 0xff;
    chunk_color.z = (ch->color >> 0x08) & 0xff;
    chunk_color.w = (ch->color >> 0x00) & 0xff;

    chunk_color.x = (chunk_color.x + ((ch->color_variant >> 0x18) & 0xff)) / 2;
    chunk_color.y = (chunk_color.y + ((ch->color_variant >> 0x10) & 0xff)) / 2;
    chunk_color.z = (chunk_color.z + ((ch->color_variant >> 0x08) & 0xff)) / 2;

    if (ch->flag & FLAG_CHUNK_RENDER)
    {
        chunk_gizmo_render.p[ch->index].x =
            (chunk_pos.x << 0x18) | (chunk_pos.y << 0x10) | (chunk_pos.z << 0x08);
        chunk_gizmo_render.p[ch->index].y =
            (chunk_color.x << 0x18) |
            (chunk_color.y << 0x10) |
            (chunk_color.z << 0x08) |
            (chunk_color.w << 0x00);
        chunk_gizmo_loaded.p[ch->index].y = 0;
    }
    else if (ch->flag & FLAG_CHUNK_LOADED)
    {
        chunk_gizmo_loaded.p[ch->index].x =
            (chunk_pos.x << 0x18) | (chunk_pos.y << 0x10) | (chunk_pos.z << 0x08);
        chunk_gizmo_loaded.p[ch->index].y =
            (chunk_color.x << 0x18) |
            (chunk_color.y << 0x10) |
            (chunk_color.z << 0x08) |
            (chunk_color.w << 0x00);
        chunk_gizmo_render.p[ch->index].y = 0;
    }
    else
    {
        chunk_gizmo_loaded.p[ch->index].y = 0;
        chunk_gizmo_render.p[ch->index].y = 0;
    }

    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_loaded.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, ch->index * sizeof(v2u32), sizeof(v2u32),
            &chunk_gizmo_loaded.p[ch->index]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_render.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, ch->index * sizeof(v2u32), sizeof(v2u32),
            &chunk_gizmo_render.p[ch->index]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

u32 *get_block_resolved(hhc_chunk *ch, i32 x, i32 y, i32 z)
{
    x = fsl_mod_i32(x, CHUNK_DIAMETER);
    y = fsl_mod_i32(y, CHUNK_DIAMETER);
    z = fsl_mod_i32(z, CHUNK_DIAMETER);
    return &ch->block[z][y][x];
}

hhc_chunk *get_chunk_resolved(u32 index, i32 x, i32 y, i32 z)
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
