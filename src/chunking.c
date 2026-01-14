#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <engine/h/diagnostics.h>
#include <engine/h/dir.h>
#include <engine/h/memory.h>
#include <engine/h/math.h>
#include <engine/h/logger.h>
#include <engine/h/platform.h>

#include "h/main.h"
#include "h/assets.h"
#include "h/chunking.h"
#include "h/dir.h"
#include "h/terrain.h"

u64 CHUNKS_MAX[SET_RENDER_DISTANCE_MAX + 1] = {0};

/*! @brief chunk buffer, raw chunk data.
 */
static Chunk *chunk_buf = NULL;

/*! @brief position of first empty slot in 'chunk_buf'
 *
 *  @remark updated internally.
 */
static u64 chunk_buf_cursor = 0;

Chunk **chunk_tab = NULL;
u32 chunk_tab_index = 0;
Chunk ***CHUNK_ORDER = NULL;
ChunkQueue CHUNK_QUEUE[CHUNK_QUEUES_MAX] = {0};

/*! @brief chunk gizmo render buffer data for opaque chunk colors.
 *
 *  for rendering chunk gizmo in one draw call.
 *
 *  format: 0xxxyyzz00, 0xrrggbbaa.
 */
v2u32 *chunk_gizmo_loaded = NULL;

/*! @brief chunk gizmo render buffer data for transparent chunk colors.
 *
 *  for rendering chunk gizmo in one draw call.
 *
 *  format: 0xxxyyzz00, 0xrrggbbaa.
 */
v2u32 *chunk_gizmo_render = NULL;

GLuint chunk_gizmo_loaded_vao = 0;
GLuint chunk_gizmo_loaded_vbo = 0;
GLuint chunk_gizmo_render_vao = 0;
GLuint chunk_gizmo_render_vbo = 0;

/*! -- INTERNAL USE ONLY --;
 */
static void _block_place(Chunk *ch,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 chunk_tab_coordinates, i32 x, i32 y, i32 z, enum BlockID block_id);

/*! -- INTERNAL USE ONLY --;
 */
static void _block_break(Chunk *ch,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 chunk_tab_coordinates, i32 x, i32 y, i32 z);

/*! @brief generate chunk blocks.
 *
 *  @param rate = number of blocks to process per chunk per frame.
 *
 *  @remark calls 'chunk_mesh_init()' when done generating.
 *  @remark must be called before 'chunk_mesh_update()'.
 */
static void chunk_generate(Chunk **chunk, u32 rate, Terrain terrain());

/*! -- INTERNAL USE ONLY --;
 *
 *  @param index = index into global array 'chunk_tab'.
 */
static void chunk_mesh_init(u32 index, Chunk *ch);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param index = index into global array 'chunk_tab'.
 */
static void chunk_mesh_update(u32 index, Chunk *ch);

/*! -- INTERNAL USE ONLY --;
 */
static void _chunk_serialize(Chunk *ch, str *world_name);

/*! -- INTERNAL USE ONLY --;
 */
static void _chunk_deserialize(Chunk *ch, str *world_name);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param index = index into global array 'chunk_tab'.
 */
static void _chunk_buf_push(u32 index, v3i32 player_chunk_delta);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param index = index into global array 'chunk_tab'.
 */
static void _chunk_gizmo_write(u32 index, Chunk *ch);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param index = index into global array 'chunk_tab'.
 */
static void _chunk_buf_pop(u32 index);

/*! -- INTERNAL USE ONLY --;
 */
static void _chunk_queue_update(ChunkQueue *q);

u32 chunking_init(void)
{
    str CHUNK_ORDER_lookup_file_name[PATH_MAX] = {0};
    str CHUNKS_MAX_lookup_file_name[PATH_MAX] = {0};
    u32 *CHUNK_ORDER_lookup_file_contents = NULL;
    u64 *CHUNKS_MAX_lookup_file_contents = NULL;
    u64 i, j, k, file_len;
    u32 render_distance = 0;
    v3i32 center = {0};
    v3i32 coordinates = {0};
    u32 *distance = NULL;
    u32 *index = NULL;
    u64 chunk_buf_diameter = 0;
    u64 chunk_buf_volume = 0;
    u64 chunks_max = 0;

    if (mem_map((void*)&chunk_buf, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk),
                "chunking_init().chunk_buf") != ERR_SUCCESS)
        return *GAME_ERR;

    if (
            mem_map((void*)&chunk_tab, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk*),
                "chunking_init().chunk_tab") != ERR_SUCCESS ||

            mem_map((void*)&CHUNK_ORDER, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_ORDER") != ERR_SUCCESS ||

            mem_map((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunking_init().distance") != ERR_SUCCESS ||

            mem_map((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
                "chunking_init().index") != ERR_SUCCESS ||

            mem_map((void*)&chunk_gizmo_loaded, CHUNK_BUF_VOLUME_MAX * sizeof(v2u32),
                "chunking_init().chunk_gizmo_loaded") != ERR_SUCCESS ||

            mem_map((void*)&chunk_gizmo_render, CHUNK_BUF_VOLUME_MAX * sizeof(v2u32),
                "chunking_init().chunk_gizmo_render") != ERR_SUCCESS)
        goto cleanup;

    /* ---- build distance look-ups ----------------------------------------- */

    for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
    {
        chunk_buf_diameter = (i * 2) + 1;
        chunk_buf_volume =
            chunk_buf_diameter * chunk_buf_diameter * chunk_buf_diameter;
        render_distance = (i * i) + 2;
        center = (v3i32){i, i, i};

        snprintf(CHUNK_ORDER_lookup_file_name, PATH_MAX,
                "%slookup_chunk_order_0x%02"PRIx64".bin",
                DIR_ROOT[DIR_LOOKUPS], i);

        if (is_file_exists(CHUNK_ORDER_lookup_file_name, FALSE) != ERR_SUCCESS)
        {
            for (j = 0; j < chunk_buf_volume; ++j)
            {
                LOGTRACE(FALSE,
                        "Building CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x] Progress [%"PRIu64"/%"PRIu64"]..\n",
                        i, SET_RENDER_DISTANCE_MAX, j, chunk_buf_volume);

                coordinates =
                    (v3i32){
                        j % chunk_buf_diameter,
                        (j / chunk_buf_diameter) % chunk_buf_diameter,
                        j / (chunk_buf_diameter * chunk_buf_diameter),
                    };
                distance[j] = distance_v3i32(coordinates, center);
                index[j] = j;
            }

            LOGTRACE(FALSE,
                    "Sorting CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x]..\n",
                    i, SET_RENDER_DISTANCE_MAX);

            for (j = 0; j < chunk_buf_volume; ++j)
                for (k = 0; k < chunk_buf_volume; ++k)
                    if (distance[j] < distance[k])
                    {
                        swap_bits_u32(&distance[j], &distance[k]);
                        swap_bits_u32(&index[j], &index[k]);
                    }

            LOGTRACE(FALSE,
                    "Writing CHUNK_ORDER Distance Lookup [0x%02"PRIx64"/0x%02x] To File..\n",
                    i, SET_RENDER_DISTANCE_MAX);

            if (write_file(CHUNK_ORDER_lookup_file_name, sizeof(u32),
                        chunk_buf_volume, index, TRUE, FALSE) != ERR_SUCCESS)
                goto cleanup;
        }
    }

    snprintf(CHUNK_ORDER_lookup_file_name, PATH_MAX,
            "%slookup_chunk_order_0x%02x.bin",
            DIR_ROOT[DIR_LOOKUPS], settings.render_distance);

    file_len = get_file_contents(CHUNK_ORDER_lookup_file_name,
            (void*)&CHUNK_ORDER_lookup_file_contents,
            sizeof(u32), FALSE);
    if (*GAME_ERR != ERR_SUCCESS ||
            CHUNK_ORDER_lookup_file_contents == NULL)
        goto cleanup;

    for (i = 0; i < settings.chunk_buf_volume; ++i)
        CHUNK_ORDER[i] = &chunk_tab[CHUNK_ORDER_lookup_file_contents[i]];
    mem_free((void*)&CHUNK_ORDER_lookup_file_contents, file_len,
            "chunking_init().CHUNK_ORDER_lookup_file_contents");

    /* ---- build CHUNKS_MAX look-up ---------------------------------------- */

    snprintf(CHUNKS_MAX_lookup_file_name, PATH_MAX,
            "%slookup_chunks_max.bin", DIR_ROOT[DIR_LOOKUPS]);

    if (is_file_exists(CHUNKS_MAX_lookup_file_name, FALSE) != ERR_SUCCESS)
    {
        for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
        {
            LOGTRACE(FALSE,
                    "Building CHUNKS_MAX Lookup, Progress [%"PRIu64"/%d]..\n",
                    i, SET_RENDER_DISTANCE_MAX);
            chunk_buf_diameter = (i * 2) + 1;
            chunk_buf_volume =
                chunk_buf_diameter * chunk_buf_diameter * chunk_buf_diameter;
            chunks_max = 0;
            render_distance = (i * i) + 2;
            center = (v3i32){i, i, i};

            for (j = 0; j < chunk_buf_volume; ++j)
            {
                coordinates =
                    (v3i32){
                        j % chunk_buf_diameter,
                        (j / chunk_buf_diameter) % chunk_buf_diameter,
                        j / (chunk_buf_diameter * chunk_buf_diameter),
                    };
                if (distance_v3i32(coordinates, center) < render_distance)
                    ++chunks_max;
            }

            CHUNKS_MAX[i] = chunks_max;
        }

        LOGTRACE(FALSE, "%s\n", "Writing CHUNKS_MAX Lookup To File..\n");

        if (write_file(CHUNKS_MAX_lookup_file_name,
                sizeof(u64), SET_RENDER_DISTANCE_MAX + 1,
                &CHUNKS_MAX, TRUE, FALSE) != ERR_SUCCESS)
            goto cleanup;
    }

    file_len = get_file_contents(CHUNKS_MAX_lookup_file_name,
            (void*)&CHUNKS_MAX_lookup_file_contents, sizeof(u64), FALSE);
    if (*GAME_ERR != ERR_SUCCESS ||
            CHUNKS_MAX_lookup_file_contents == NULL)
        goto cleanup;

    for (i = 0; i <= SET_RENDER_DISTANCE_MAX; ++i)
        CHUNKS_MAX[i] = CHUNKS_MAX_lookup_file_contents[i];
    mem_free((void*)&CHUNKS_MAX_lookup_file_contents, file_len,
            "chunking_init().CHUNKS_MAX_lookup_file_contents");

    /* ---- init chunk parsing priority queues ------------------------------ */

    CHUNK_QUEUE[0].id = CHUNK_QUEUE_1ST_ID;
    CHUNK_QUEUE[0].offset = 0;
    CHUNK_QUEUE[0].size =
        (u64)clamp_i64(CHUNKS_MAX[settings.render_distance],
                0, CHUNK_QUEUE_1ST_MAX);
    CHUNK_QUEUE[0].rate_chunk = CHUNK_PARSE_RATE_PRIORITY_HIGH;
    CHUNK_QUEUE[0].rate_block = BLOCK_PARSE_RATE;

    CHUNK_QUEUE[1].id = CHUNK_QUEUE_2ND_ID;
    CHUNK_QUEUE[1].offset = CHUNK_QUEUE_1ST_MAX;
    CHUNK_QUEUE[1].size =
        (u64)clamp_i64(CHUNKS_MAX[settings.render_distance] -
                CHUNK_QUEUE[1].offset, 0, CHUNK_QUEUE_2ND_MAX);
    CHUNK_QUEUE[1].rate_chunk = CHUNK_PARSE_RATE_PRIORITY_MID;
    CHUNK_QUEUE[1].rate_block = BLOCK_PARSE_RATE;

    CHUNK_QUEUE[2].id = CHUNK_QUEUE_3RD_ID;
    CHUNK_QUEUE[2].offset = CHUNK_QUEUE_1ST_MAX + CHUNK_QUEUE_2ND_MAX;
    CHUNK_QUEUE[2].size =
        (u64)clamp_i64(CHUNKS_MAX[settings.render_distance] -
            CHUNK_QUEUE[2].offset, 0, CHUNK_QUEUE_3RD_MAX);
    CHUNK_QUEUE[2].rate_chunk = CHUNK_PARSE_RATE_PRIORITY_LOW;
    CHUNK_QUEUE[2].rate_block = BLOCK_PARSE_RATE;

    if (
            mem_map((void*)&CHUNK_QUEUE[0].queue,
                CHUNK_QUEUE_1ST_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_QUEUE[0].queue") != ERR_SUCCESS ||

            mem_map((void*)&CHUNK_QUEUE[1].queue,
                CHUNK_QUEUE_2ND_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_QUEUE[1].queue") != ERR_SUCCESS ||

            mem_map((void*)&CHUNK_QUEUE[2].queue,
                CHUNK_QUEUE_3RD_MAX * sizeof(Chunk**),
                "chunking_init().CHUNK_QUEUE[2].queue") != ERR_SUCCESS)
        goto cleanup;

    mem_unmap((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_free().distance");

    mem_unmap((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_free().index");

    /* ---- intialize chunk gizmo ------------------------------------------- */

    glGenVertexArrays(1, &chunk_gizmo_loaded_vao);
    glGenBuffers(1, &chunk_gizmo_loaded_vbo);

    glBindVertexArray(chunk_gizmo_loaded_vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_loaded_vbo);
    glBufferData(GL_ARRAY_BUFFER, settings.chunk_buf_volume * sizeof(v2u32),
            chunk_gizmo_loaded, GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(v2u32), (void*)0);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &chunk_gizmo_render_vao);
    glGenBuffers(1, &chunk_gizmo_render_vbo);

    glBindVertexArray(chunk_gizmo_render_vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_render_vbo);
    glBufferData(GL_ARRAY_BUFFER, settings.chunk_buf_volume * sizeof(v2u32),
            chunk_gizmo_render, GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(v2u32), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    mem_unmap((void*)&distance, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_init().distance");

    mem_unmap((void*)&index, CHUNK_BUF_VOLUME_MAX * sizeof(u32),
            "chunking_init().index");

    chunking_free();
    return *GAME_ERR;
}

void chunking_update(v3i32 player_chunk, v3i32 *player_chunk_delta)
{
    v3u32 _coordinates, _mirror_index, _target_index;
    u32 mirror_index, target_index;
    u8 is_on_edge;
    v3u8 _is_on_edge;
    i64 i;
    v3i16 DELTA;
    u8 AXIS;
    i8 INCREMENT;
    i32 RENDER_DISTANCE;
    Chunk ***cursor;

    _chunk_queue_update(&CHUNK_QUEUE[0]);
    if (!CHUNK_QUEUE[0].count && CHUNK_QUEUE[1].size)
    {
        _chunk_queue_update(&CHUNK_QUEUE[1]);
        if (!CHUNK_QUEUE[1].count && CHUNK_QUEUE[2].size)
            _chunk_queue_update(&CHUNK_QUEUE[2]);
    }

    if (!core.flag.chunk_buf_dirty)
        return;

chunk_tab_shift:

    DELTA =
    (v3i16){
        player_chunk.x - player_chunk_delta->x,
        player_chunk.y - player_chunk_delta->y,
        player_chunk.z - player_chunk_delta->z,
    };

    AXIS =
        DELTA.x > 0 ? STATE_CHUNK_SHIFT_PX :
        DELTA.x < 0 ? STATE_CHUNK_SHIFT_NX :
        DELTA.y > 0 ? STATE_CHUNK_SHIFT_PY :
        DELTA.y < 0 ? STATE_CHUNK_SHIFT_NY :
        DELTA.z > 0 ? STATE_CHUNK_SHIFT_PZ :
        DELTA.z < 0 ? STATE_CHUNK_SHIFT_NZ : 0;

    INCREMENT = (AXIS % 2 == 1) - (AXIS %2 == 0);
    RENDER_DISTANCE = settings.render_distance * settings.render_distance + 2;

    if ((i32)len_v3f32(
                (v3f32){DELTA.x, DELTA.y, DELTA.z}) > RENDER_DISTANCE)
    {
        cursor = CHUNK_ORDER + CHUNKS_MAX[settings.render_distance] - 1;
        for (; cursor >= CHUNK_ORDER; --cursor)
            if (**cursor) _chunk_buf_pop(*cursor - chunk_tab);

        *player_chunk_delta = player_chunk;
        goto chunk_buf_push;
    }

    /* this keeps chunk_buf from exploding on a 'chunk_tab' shift */
    chunk_buf_cursor = 0;

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
        if (!chunk_tab[i]) continue;

        _coordinates =
        (v3u32){
            i % settings.chunk_buf_diameter,
            (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
            i / settings.chunk_buf_layer,
        };

        _mirror_index =
        (v3u32){
            i + settings.chunk_buf_diameter - 1 - _coordinates.x * 2,

            _coordinates.z * settings.chunk_buf_layer +
                (settings.chunk_buf_diameter - 1 - _coordinates.y) *
                settings.chunk_buf_diameter + _coordinates.x,

            (settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer +
                _coordinates.y * settings.chunk_buf_diameter + _coordinates.x,
        };

        switch (INCREMENT)
        {
            case -1:
                _is_on_edge =
                    (v3u8){
                        _coordinates.x == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab[i + 1],

                        _coordinates.y == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab[i + settings.chunk_buf_diameter],

                        _coordinates.z == settings.chunk_buf_diameter - 1 ||
                            !chunk_tab[i + settings.chunk_buf_layer],
                    };
                break;

            case 1:
                _is_on_edge =
                    (v3u8){
                        _coordinates.x == 0 || !chunk_tab[i - 1],

                        _coordinates.y == 0 ||
                            !chunk_tab[i - settings.chunk_buf_diameter],

                        _coordinates.z == 0 ||
                            !chunk_tab[i - settings.chunk_buf_layer],
                    };
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
            chunk_tab[i]->flag &= ~(FLAG_CHUNK_LOADED | FLAG_CHUNK_RENDER);
            chunk_tab[i]->color = 0;
            if (chunk_tab[mirror_index])
                chunk_tab[mirror_index]->flag |= FLAG_CHUNK_EDGE;
        }
    }

    /* ---- shift 'chunk_tab' ----------------------------------------------- */

    for (i = (INCREMENT == 1) ? 0 : settings.chunk_buf_volume - 1;
            i < settings.chunk_buf_volume && i >= 0; i += INCREMENT)
    {
        if (!chunk_tab[i]) continue;

        _coordinates =
        (v3u32){
            i % settings.chunk_buf_diameter,
            (i / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
            i / settings.chunk_buf_layer,
        };

        _mirror_index =
        (v3u32){
            i + settings.chunk_buf_diameter - 1 - _coordinates.x * 2,

            _coordinates.z * settings.chunk_buf_layer +
                (settings.chunk_buf_diameter - 1 - _coordinates.y) *
                 settings.chunk_buf_diameter + _coordinates.x,

            (settings.chunk_buf_diameter - 1 - _coordinates.z) * settings.chunk_buf_layer +
                _coordinates.y * settings.chunk_buf_diameter + _coordinates.x,
        };

        switch (INCREMENT)
        {
            case -1:
                _target_index = (v3u32){
                        _coordinates.x == 0 ? i : i - 1,
                        _coordinates.y == 0 ? i : i - settings.chunk_buf_diameter,
                        _coordinates.z == 0 ? i : i - settings.chunk_buf_layer};
                break;

            case 1:
                _target_index = (v3u32){
                        _coordinates.x == settings.chunk_buf_diameter - 1 ? i : i + 1,

                        _coordinates.y == settings.chunk_buf_diameter - 1 ?
                            i : i + settings.chunk_buf_diameter,

                        _coordinates.z == settings.chunk_buf_diameter - 1 ?
                            i : i + settings.chunk_buf_layer};
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

        chunk_tab[i] = chunk_tab[target_index];
        if (chunk_tab[i])
        {
            _chunk_gizmo_write(i, chunk_tab[i]);
            if (chunk_tab[i]->flag & FLAG_CHUNK_EDGE)
                chunk_tab[target_index] = NULL;

            chunk_tab[i]->flag &= ~(FLAG_CHUNK_EDGE | FLAG_CHUNK_QUEUED);
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
        Chunk **p = CHUNK_ORDER[i];
        u32 index = p - chunk_tab;
        if (!*p) _chunk_buf_push(index, *player_chunk_delta);
    }

    core.flag.chunk_buf_dirty = 0;
}

void chunking_free(void)
{
    if (chunk_tab)
    {
        u32 i = 0;
        for (; i < settings.chunk_buf_volume; ++i)
            if (chunk_tab[i])
                _chunk_buf_pop(i);
    }

    mem_unmap((void*)&chunk_buf, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk),
            "chunking_free().chunk_buf");

    mem_unmap((void*)&chunk_tab, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk*),
            "chunking_free().chunk_tab");

    mem_unmap((void*)&CHUNK_ORDER, CHUNK_BUF_VOLUME_MAX * sizeof(Chunk**),
            "chunking_free().CHUNK_ORDER");

    mem_unmap((void*)&CHUNK_QUEUE[0].queue, CHUNK_QUEUE_1ST_MAX * sizeof(Chunk**),
            "chunking_free().CHUNK_QUEUE[0].queue");

    mem_unmap((void*)&CHUNK_QUEUE[1].queue, CHUNK_QUEUE_2ND_MAX * sizeof(Chunk**),
            "chunking_free().CHUNK_QUEUE[1].queue");

    mem_unmap((void*)&CHUNK_QUEUE[2].queue, CHUNK_QUEUE_3RD_MAX * sizeof(Chunk**),
            "chunking_free().CHUNK_QUEUE[2].queue");

    if (chunk_gizmo_loaded_vao) glDeleteVertexArrays(1, &chunk_gizmo_loaded_vao);
    if (chunk_gizmo_loaded_vbo) glDeleteBuffers(1, &chunk_gizmo_loaded_vbo);
    if (chunk_gizmo_render_vao) glDeleteVertexArrays(1, &chunk_gizmo_render_vao);
    if (chunk_gizmo_render_vbo) glDeleteBuffers(1, &chunk_gizmo_render_vbo);

    mem_unmap((void*)&chunk_gizmo_loaded, CHUNK_BUF_VOLUME_MAX * sizeof(v2u32),
            "chunking_free().chunk_gizmo_loaded");

    mem_unmap((void*)&chunk_gizmo_render, CHUNK_BUF_VOLUME_MAX * sizeof(v2u32),
            "chunking_free().chunk_gizmo_render");
}

void block_place(u32 index, i32 x, i32 y, i32 z, v3f64 normal, enum BlockID block_id)
{
    if (!(normal.x != 0.0f || normal.y != 0.0f || normal.z != 0.0f)) return;

    x += (i32)normal.x;
    y += (i32)normal.y;
    z += (i32)normal.z;
    index += (i32)floorf((f32)x / CHUNK_DIAMETER);
    index += (i32)floorf((f32)y / CHUNK_DIAMETER) * settings.chunk_buf_diameter;
    index += (i32)floorf((f32)z / CHUNK_DIAMETER) * settings.chunk_buf_layer;
    if (index >= settings.chunk_buf_volume) return;

    v3u32 chunk_tab_coordinates =
    {
        index % settings.chunk_buf_diameter,
        (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
        index / settings.chunk_buf_layer,
    };
    Chunk **chunk = &chunk_tab[index];
    Chunk *px = NULL;
    Chunk *nx = NULL;
    Chunk *py = NULL;
    Chunk *ny = NULL;
    Chunk *pz = NULL;
    Chunk *nz = NULL;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = *(chunk + 1);
    if (chunk_tab_coordinates.x > 0)
        nx = *(chunk - 1);
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = *(chunk + settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.y > 0)
        ny = *(chunk - settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = *(chunk + settings.chunk_buf_layer);
    if (chunk_tab_coordinates.z > 0)
        nz = *(chunk - settings.chunk_buf_layer);

    if ((*chunk)->block[z][y][x] || !block_id) return;

    _block_place(*chunk, px, nx, py, ny, pz, nz, chunk_tab_coordinates, x, y, z, block_id);
    (*chunk)->flag |= FLAG_CHUNK_MODIFIED;
}

static void _block_place(Chunk *ch,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 chunk_tab_coordinates, i32 x, i32 y, i32 z, enum BlockID block_id)
{
    x = mod(x, CHUNK_DIAMETER);
    y = mod(y, CHUNK_DIAMETER);
    z = mod(z, CHUNK_DIAMETER);
    u32 *block = &ch->block[z][y][x];
    SET_BLOCK_ID(*block, block_id);

    if (x == CHUNK_DIAMETER - 1)
    {
        if (chunk_tab_coordinates.x != settings.chunk_buf_diameter - 1 &&
                px && px->block[z][y][0])
        {
            px->block[z][y][0] &= ~FLAG_BLOCK_FACE_NX;
            px->flag |= FLAG_CHUNK_DIRTY;
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
            nx->flag |= FLAG_CHUNK_DIRTY;
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
            py->flag |= FLAG_CHUNK_DIRTY;
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
            ny->flag |= FLAG_CHUNK_DIRTY;
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
            pz->flag |= FLAG_CHUNK_DIRTY;
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
            nz->flag |= FLAG_CHUNK_DIRTY;
        }
        else ch->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;
    }
    else if (ch->block[z - 1][y][x])
        ch->block[z - 1][y][x] &= ~FLAG_BLOCK_FACE_PZ;
    else ch->block[z][y][x] |= FLAG_BLOCK_FACE_NZ;

    ch->flag |= FLAG_CHUNK_DIRTY;
}

void block_break(u32 index, i32 x, i32 y, i32 z)
{
    v3u32 chunk_tab_coordinates =
    {
        index % settings.chunk_buf_diameter,
        (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
        index / settings.chunk_buf_layer,
    };
    Chunk **chunk = &chunk_tab[index];
    Chunk *px = NULL;
    Chunk *nx = NULL;
    Chunk *py = NULL;
    Chunk *ny = NULL;
    Chunk *pz = NULL;
    Chunk *nz = NULL;

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = *(chunk + 1);
    if (chunk_tab_coordinates.x > 0)
        nx = *(chunk - 1);
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = *(chunk + settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.y > 0)
        ny = *(chunk - settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = *(chunk + settings.chunk_buf_layer);
    if (chunk_tab_coordinates.z > 0)
        nz = *(chunk - settings.chunk_buf_layer);


    if (!(*chunk)->block[z][y][x]) return;

    _block_break(*chunk, px, nx, py, ny, pz, nz, chunk_tab_coordinates, x, y, z);
    (*chunk)->flag |= FLAG_CHUNK_MODIFIED;
}

static void _block_break(Chunk *ch,
        Chunk *px, Chunk *nx, Chunk *py, Chunk *ny, Chunk *pz, Chunk *nz, 
        v3u32 chunk_tab_coordinates, i32 x, i32 y, i32 z)
{
    x = mod(x, CHUNK_DIAMETER);
    y = mod(y, CHUNK_DIAMETER);
    z = mod(z, CHUNK_DIAMETER);

    if (x == CHUNK_DIAMETER - 1)
    {
        if (chunk_tab_coordinates.x != settings.chunk_buf_diameter - 1 &&
                px && px->block[z][y][0])
        {
            px->block[z][y][0] |= FLAG_BLOCK_FACE_NX;
            px->flag |= FLAG_CHUNK_DIRTY;
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
            nx->flag |= FLAG_CHUNK_DIRTY;
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
            py->flag |= FLAG_CHUNK_DIRTY;
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
            ny->flag |= FLAG_CHUNK_DIRTY;
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
            pz->flag |= FLAG_CHUNK_DIRTY;
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
            nz->flag |= FLAG_CHUNK_DIRTY;
        }
    }
    else if (ch->block[z - 1][y][x])
        ch->block[z - 1][y][x] |= FLAG_BLOCK_FACE_PZ;

    ch->block[z][y][x] = 0;
    ch->flag |= FLAG_CHUNK_DIRTY;
}

static void chunk_generate(Chunk **chunk, u32 rate, Terrain terrain())
{
    u32 index;
    v3u32 chunk_tab_coordinates;
    v3i32 coordinates;
    Terrain terrain_info;
    Chunk *ch = NULL,
          *px = NULL, *nx = NULL,
          *py = NULL, *ny = NULL,
          *pz = NULL, *nz = NULL;
    i32 x, y, z;

    if (!chunk || !*chunk || !terrain) return;

    ch = *chunk;
    index = chunk - chunk_tab;
    chunk_tab_coordinates =
        (v3u32){
            index % settings.chunk_buf_diameter,
            (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
            index / settings.chunk_buf_layer,
        };

    if (chunk_tab_coordinates.x < settings.chunk_buf_diameter - 1)
        px = *(chunk + 1);
    if (chunk_tab_coordinates.x > 0)
        nx = *(chunk - 1);
    if (chunk_tab_coordinates.y < settings.chunk_buf_diameter - 1)
        py = *(chunk + settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.y > 0)
        ny = *(chunk - settings.chunk_buf_diameter);
    if (chunk_tab_coordinates.z < settings.chunk_buf_diameter - 1)
        pz = *(chunk + settings.chunk_buf_layer);
    if (chunk_tab_coordinates.z > 0)
        nz = *(chunk - settings.chunk_buf_layer);

    x = ch->cursor % CHUNK_DIAMETER;
    y = (ch->cursor / CHUNK_DIAMETER) % CHUNK_DIAMETER;
    z = ch->cursor / CHUNK_LAYER;
    for (; z < CHUNK_DIAMETER && rate; ++z)
    {
        for (; y < CHUNK_DIAMETER && rate; ++y)
        {
            for (; x < CHUNK_DIAMETER && rate; ++x)
            {
                coordinates =
                (v3i32){
                    x + ch->pos.x * CHUNK_DIAMETER,
                    y + ch->pos.y * CHUNK_DIAMETER,
                    z + ch->pos.z * CHUNK_DIAMETER,
                };

                terrain_info = terrain(coordinates);

                if (terrain_info.block_id)
                {
                    _block_place(ch, px, nx, py, ny, pz, nz,
                            chunk_tab_coordinates, x, y, z, terrain_info.block_id);
                    ch->block[z][y][x] |= terrain_info.block_light;

                    if (z == 0)
                    {
                        if (nz && GET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x]) == BLOCK_GRASS)
                        {
                            SET_BLOCK_ID(nz->block[CHUNK_DIAMETER - 1][y][x], BLOCK_DIRT);
                            nz->flag |= FLAG_CHUNK_DIRTY;
                        }
                    }
                    else if (ch->block[z - 1][y][x] && GET_BLOCK_ID(ch->block[z - 1][y][x]) == BLOCK_GRASS)
                    {
                        SET_BLOCK_ID(ch->block[z - 1][y][x], BLOCK_DIRT);
                        ch->flag |= FLAG_CHUNK_DIRTY;
                    }

                    if (z == CHUNK_DIAMETER - 1 && GET_BLOCK_ID(ch->block[z][y][x]) == BLOCK_GRASS &&
                            pz && pz->block[0][y][x])
                        SET_BLOCK_ID(ch->block[z][y][x], BLOCK_DIRT);
                }
                --rate;
            }
            x = 0;
        }
        y = 0;
    }
    ch->cursor = x + y * CHUNK_DIAMETER + z * CHUNK_LAYER;

    if (ch->cursor == CHUNK_VOLUME && !(ch->flag & FLAG_CHUNK_GENERATED))
        chunk_mesh_init(index, ch);
}

static void chunk_mesh_init(u32 index, Chunk *ch)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    u32 *i = (u32*)ch->block;
    u32 *end = i + CHUNK_VOLUME;
    u64 block_index;
    v3u64 pos;
    b8 should_render = FALSE;

    for (; i < end; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            should_render = TRUE;
            block_index = CHUNK_VOLUME - (end - i);
            pos =
                (v3u64){
                    block_index % CHUNK_DIAMETER,
                    (block_index / CHUNK_DIAMETER) % CHUNK_DIAMETER,
                    block_index / CHUNK_LAYER,
                };
            *(cursor++) = *i |
                (pos.x & 0xf) << SHIFT_BLOCK_X |
                (pos.y & 0xf) << SHIFT_BLOCK_Y |
                (pos.z & 0xf) << SHIFT_BLOCK_Z;
        }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;
    u64 len = cursor - buf;
    ch->vbo_len = len;

    if (!ch->vao) glGenVertexArrays(1, &ch->vao);
    if (!ch->vbo) glGenBuffers(1, &ch->vbo);

    glBindVertexArray(ch->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ch->vbo);
    glBufferData(GL_ARRAY_BUFFER, len * sizeof(u64), buf, GL_DYNAMIC_DRAW);

    glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(u64), (void*)sizeof(u32));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    ch->flag |= FLAG_CHUNK_GENERATED | FLAG_CHUNK_DIRTY;

    if (should_render)
    {
        ch->flag |= FLAG_CHUNK_RENDER;
        ch->color = CHUNK_COLOR_RENDER;
    }
    else
    {
        ch->flag &= ~FLAG_CHUNK_RENDER;
        ch->color = CHUNK_COLOR_LOADED;
    }

    _chunk_gizmo_write(index, ch);
}

static void chunk_mesh_update(u32 index, Chunk *ch)
{
    static u64 buffer[BLOCK_BUFFERS_MAX][CHUNK_VOLUME] = {0};
    static u64 cur_buf = 0;

    if (!ch->vbo || !ch->vao) return;
    u64 *buf = &buffer[cur_buf][0];
    u64 *cursor = buf;
    u32 *i = (u32*)ch->block;
    u32 *end = i + CHUNK_VOLUME;
    u64 block_index;
    v3u64 pos;
    b8 should_render = FALSE;

    for (; i < end; ++i)
        if (*i & MASK_BLOCK_FACES)
        {
            should_render = TRUE;
            block_index = CHUNK_VOLUME - (end - i);
            pos =
                (v3u64){
                    block_index % CHUNK_DIAMETER,
                    (block_index / CHUNK_DIAMETER) % CHUNK_DIAMETER,
                    block_index / CHUNK_LAYER,
                };
            *(cursor++) = *i |
                (pos.x & 0xf) << SHIFT_BLOCK_X |
                (pos.y & 0xf) << SHIFT_BLOCK_Y |
                (pos.z & 0xf) << SHIFT_BLOCK_Z;
        }
    cur_buf = (cur_buf + 1) % BLOCK_BUFFERS_MAX;
    u64 len = cursor - buf;
    ch->vbo_len = len;

    glBindBuffer(GL_ARRAY_BUFFER, ch->vbo);
    glBufferData(GL_ARRAY_BUFFER, len * sizeof(u64), buf, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    ch->flag &= ~FLAG_CHUNK_DIRTY;

    if (should_render)
    {
        ch->flag |= FLAG_CHUNK_RENDER;
        ch->color = CHUNK_COLOR_RENDER;
    }
    else
    {
        ch->flag &= ~FLAG_CHUNK_RENDER;
        ch->color = CHUNK_COLOR_LOADED;
    }

    _chunk_gizmo_write(index, ch);
}

/* TODO: make chunk_serialize() */
static void _chunk_serialize(Chunk *ch, str *world_name)
{
}

/* TODO: make chunk_deserialize() */
static void _chunk_deserialize(Chunk *ch, str *world_name)
{
}

static void _chunk_buf_push(u32 index, v3i32 player_chunk_delta)
{
    v3u32 chunk_tab_coordinates =
    {
        index % settings.chunk_buf_diameter,
        (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
        index / settings.chunk_buf_layer,
    };
    v3u64 seed;
    v3u8 color_variant;

    Chunk *ch = &chunk_buf[chunk_buf_cursor];
    Chunk *end = chunk_buf + CHUNKS_MAX[settings.render_distance];
    for (; ch < end; ++ch)
        if (!(ch->flag & FLAG_CHUNK_LOADED))
        {
            if (ch->vbo) glDeleteBuffers(1, &ch->vbo);
            if (ch->vao) glDeleteVertexArrays(1, &ch->vao);
            *ch = (Chunk){0};

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
            color_variant.x = (u8)(map_range_f64((f64)(rand_u64(seed.x) % 0xff), 0.0, 0xff,
                        1.0 - CHUNK_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);
            color_variant.y = (u8)(map_range_f64((f64)(rand_u64(seed.y) % 0xff), 0.0, 0xff,
                        1.0 - CHUNK_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);
            color_variant.z = (u8)(map_range_f64((f64)(rand_u64(seed.z) % 0xff), 0.0, 0xff,
                        1.0 - CHUNK_COLOR_FACTOR_INFLUENCE, 1.0) * 0xff);

            ch->color_variant = 0 |
                (color_variant.x << 0x18) |
                (color_variant.y << 0x10) |
                (color_variant.z << 0x08);

            ch->flag = FLAG_CHUNK_LOADED | FLAG_CHUNK_DIRTY;
            chunk_tab[index] = ch;
            ++chunk_buf_cursor;
            return;
        }

    LOGERROR(FALSE, ERR_BUFFER_FULL, "'%s'\n", "'chunk_buf' Full");
}

static void _chunk_buf_pop(u32 index)
{
    u32 index_popped = chunk_tab[index] - chunk_buf;

    _chunk_gizmo_write(index, chunk_tab[index]);

    if (chunk_tab[index]->vbo)
    {
        glDeleteBuffers(1, &chunk_tab[index]->vbo);
        chunk_tab[index]->vbo = 0;
    }
    if (chunk_tab[index]->vao)
    {
        glDeleteVertexArrays(1, &chunk_tab[index]->vao);
        chunk_tab[index]->vao = 0;
    }

    chunk_tab[index]->flag = 0;
    if (chunk_buf_cursor > index_popped)
        chunk_buf_cursor = index_popped;
    chunk_tab[index] = NULL;
}

/* TODO: grab chunks from disk if previously generated */
static void _chunk_queue_update(ChunkQueue *q)
{
    if (!MODE_INTERNAL_LOAD_CHUNKS) return;

    u32 i;
    u64 size = q->size;
    u32 cursor = q->cursor;
    u32 rate_chunk = q->rate_chunk;
    u32 rate_block = q->rate_block;
    Chunk ***chunk = CHUNK_ORDER + q->offset;
    Chunk ***end = NULL;

    if (q->count == size)
        goto generate_and_mesh;

    /* ---- push chunk queue ------------------------------------------------ */

    if (q->id == CHUNK_QUEUE_LAST_ID)
        end = CHUNK_ORDER + CHUNKS_MAX[settings.render_distance];
    else end = CHUNK_ORDER + q->offset + size;
    for (; chunk < end && q->count < size; ++chunk)
    {
        if (**chunk && ((**chunk)->flag & FLAG_CHUNK_DIRTY) &&
                !((**chunk)->flag & FLAG_CHUNK_QUEUED) &&
                !q->queue[cursor])
        {
            q->queue[cursor] = *chunk;
            (**chunk)->flag |= FLAG_CHUNK_QUEUED;
            ++q->count;
            cursor = (cursor + 1) % size;
        }
    }

    q->cursor = cursor;
    if (!q->count) return;

generate_and_mesh:

    for (i = 0; i < size && rate_chunk; ++i)
        if (q->queue[i])
        {
            if ((*q->queue[i])->flag & FLAG_CHUNK_GENERATED)
                chunk_mesh_update(q->queue[i] - chunk_tab, *q->queue[i]);
            else chunk_generate(q->queue[i], rate_block, terrain_biome_blend_test);
            if (!((*q->queue[i])->flag & FLAG_CHUNK_DIRTY))
            {
                (*q->queue[i])->flag &= ~FLAG_CHUNK_QUEUED;
                q->queue[i] = NULL;
                if (q->count > 0) --q->count;
            }
            --rate_chunk;
        }
}

static void _chunk_gizmo_write(u32 index, Chunk *ch)
{
    v3u32 chunk_pos =
    {
        index % settings.chunk_buf_diameter,
        (index / settings.chunk_buf_diameter) % settings.chunk_buf_diameter,
        index / settings.chunk_buf_layer,
    };
    v4u32 chunk_color =
    {
        (ch->color >> 0x18) & 0xff,
        (ch->color >> 0x10) & 0xff,
        (ch->color >> 0x08) & 0xff,
        (ch->color >> 0x00) & 0xff,
    };

    chunk_color.x = (chunk_color.x + ((ch->color_variant >> 0x18) & 0xff)) / 2;
    chunk_color.y = (chunk_color.y + ((ch->color_variant >> 0x10) & 0xff)) / 2;
    chunk_color.z = (chunk_color.z + ((ch->color_variant >> 0x08) & 0xff)) / 2;

    if (ch->flag & FLAG_CHUNK_RENDER)
    {
        chunk_gizmo_render[index].x =
            (chunk_pos.x << 0x18) | (chunk_pos.y << 0x10) | (chunk_pos.z << 0x08);
        chunk_gizmo_render[index].y =
            color_hex_u32(chunk_color.x, chunk_color.y, chunk_color.z, chunk_color.w);
        chunk_gizmo_loaded[index].y = 0;
    }
    else if (ch->flag & FLAG_CHUNK_LOADED)
    {
        chunk_gizmo_loaded[index].x =
            (chunk_pos.x << 0x18) | (chunk_pos.y << 0x10) | (chunk_pos.z << 0x08);
        chunk_gizmo_loaded[index].y =
            color_hex_u32(chunk_color.x, chunk_color.y, chunk_color.z, chunk_color.w);
        chunk_gizmo_render[index].y = 0;
    }
    else
    {
        chunk_gizmo_loaded[index].y = 0;
        chunk_gizmo_render[index].y = 0;
    }

    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_loaded_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(v2u32), sizeof(v2u32),
            &chunk_gizmo_loaded[index]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_render_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(v2u32), sizeof(v2u32),
            &chunk_gizmo_render[index]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

u32 *get_block_resolved(Chunk *ch, i32 x, i32 y, i32 z)
{
    x = mod(x, CHUNK_DIAMETER);
    y = mod(y, CHUNK_DIAMETER);
    z = mod(z, CHUNK_DIAMETER);
    return &ch->block[z][y][x];
}

Chunk *get_chunk_resolved(u32 index, i32 x, i32 y, i32 z)
{
    x = (i32)floorf((f32)x / CHUNK_DIAMETER);
    y = (i32)floorf((f32)y / CHUNK_DIAMETER);
    z = (i32)floorf((f32)z / CHUNK_DIAMETER);
    return chunk_tab[index + x +
        y * settings.chunk_buf_diameter +
        z * settings.chunk_buf_layer];
}

u32 get_chunk_index(v3i32 chunk_pos, v3f64 pos)
{
    v3i32 offset =
    {
        (i32)floorf(pos.x / CHUNK_DIAMETER) - chunk_pos.x + settings.chunk_buf_radius,
        (i32)floorf(pos.y / CHUNK_DIAMETER) - chunk_pos.y + settings.chunk_buf_radius,
        (i32)floorf(pos.z / CHUNK_DIAMETER) - chunk_pos.z + settings.chunk_buf_radius,
    };
    u32 index =
        offset.x +
        offset.y * settings.chunk_buf_diameter +
        offset.z * settings.chunk_buf_layer;

    if (index >= settings.chunk_buf_volume)
        return settings.chunk_tab_center;
    return index;
}
