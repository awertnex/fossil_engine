#include "deps/fossil/common/types.h"
#include "deps/fossil/assets/asset_types.h"
#include "deps/fossil/assets/mesh/mesh.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shader_types.h"

#include "deps/fossil/external/glad/glad.h"

#include "../settings/settings.h"

#include "../h/common.h"
#include "../h/assets.h"
#include "../h/diagnostics.h"
#include "../h/main.h"

#include "chunking.h"
#include "chunking_debug_tools.h"
#include "chunking_internal.h"

/* ---- section: definitions ------------------------------------------------ */

/*!
 *  @brief chunk gizmo render buffer data for chunk colors.
 *
 *  for rendering chunk gizmo in one draw call.
 *
 *  format: 0xxxyyzz00, 0xrrggbbaa.
 */
typedef struct hhc_chunk_gizmo
{
    b8 initialized;
    GLuint vao;
    GLuint vbo;
    fsl_mem_handle handle;
    v2u32 *p;               /* cached pointer from `handle` */
} hhc_chunk_gizmo;

/* ---- section: declarations ----------------------------------------------- */

static fsl_mem_arena memory_arena_chunk_debug_internal = {0};

/*!
 *  @brief buffer data for opaque chunk colors.
 */
hhc_chunk_gizmo chunk_gizmo_loaded = {0};

/*!
 *  @brief buffer data for transparent chunk colors.
 */
hhc_chunk_gizmo chunk_gizmo_visible = {0};

/* ---- section: signatures ------------------------------------------------- */

static void chunk_debug_scheduler_visualizer_draw_internal(hhc_chunk_scheduler sched,
        const fsl_mesh *mesh_bounding_box, const fsl_camera *camera,
        f32 color_r, f32 color_g, f32 color_b, f32 color_a);

/* ---- section: implementation --------------------------------------------- */

u32 chunk_debug_init_internal(fsl_len chunk_count)
{
    if (fsl_mem_arena_init(&memory_arena_chunk_debug_internal,
                "chunk_debug_init().memory_arena_chunk_debug_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunk_debug_internal, &chunk_gizmo_loaded.handle,
                chunk_count * sizeof(v2u32),
                "chunk_debug_init().chunk_gizmo_loaded.handle") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_chunk_debug_internal, &chunk_gizmo_visible.handle,
                chunk_count * sizeof(v2u32),
                "chunk_debug_init().chunk_gizmo_visible.handle") != FSL_ERR_SUCCESS)
        goto cleanup;

    chunk_gizmo_loaded.p = fsl_mem_handle_get(chunk_gizmo_loaded.handle);
    chunk_gizmo_visible.p = fsl_mem_handle_get(chunk_gizmo_visible.handle);

    glGenVertexArrays(1, &chunk_gizmo_loaded.vao);
    glGenBuffers(1, &chunk_gizmo_loaded.vbo);

    glBindVertexArray(chunk_gizmo_loaded.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_loaded.vbo);
    glBufferData(GL_ARRAY_BUFFER, chunk_count * sizeof(v2u32), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(v2u32), (void*)0);

    glGenVertexArrays(1, &chunk_gizmo_visible.vao);
    glGenBuffers(1, &chunk_gizmo_visible.vbo);

    glBindVertexArray(chunk_gizmo_visible.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_visible.vbo);
    glBufferData(GL_ARRAY_BUFFER, chunk_count * sizeof(v2u32), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_UNSIGNED_INT, sizeof(v2u32), (void*)0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    chunk_gizmo_loaded.initialized = TRUE;
    chunk_gizmo_visible.initialized = TRUE;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    chunk_debug_free_internal();
    return *GAME_ERR;
}

void chunk_debug_free_internal(void)
{
    if (chunk_gizmo_loaded.initialized)
    {
        chunk_gizmo_loaded.initialized = FALSE;
        glDeleteBuffers(1, &chunk_gizmo_loaded.vbo);
        glDeleteVertexArrays(1, &chunk_gizmo_loaded.vao);
    }

    if (chunk_gizmo_visible.initialized)
    {
        chunk_gizmo_visible.initialized = FALSE;
        glDeleteBuffers(1, &chunk_gizmo_visible.vbo);
        glDeleteVertexArrays(1, &chunk_gizmo_visible.vao);
    }

    fsl_mem_arena_free(&memory_arena_chunk_debug_internal,
            "chunk_debug_free_internal().memory_arena_chunk_debug_internal");
}

void chunk_debug_chunk_gizmo_draw(const fsl_camera *camera)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    m4f32 transform = {0};
    v3f32 camera_position = {0};

    transform = camera->projection.projection;
    transform = fsl_matrix_multiply(camera->projection.orientation, transform);
    transform = fsl_matrix_multiply(camera->projection.rotation, transform);
    transform = fsl_matrix_multiply(camera->projection.target, transform);

    glUseProgram(shader_p[SHADER_GIZMO_CHUNK].asset.id);

    glUniform1f(uniform.gizmo_chunk.gizmo_offset, (f32)settings.chunk_buf_radius + 0.5f);
    glUniform2iv(uniform.gizmo_chunk.render_size, 1, (GLint*)&render->size);
    glUniform1i(uniform.gizmo_chunk.chunk_buf_diameter, settings.chunk_buf_diameter);

    glUniformMatrix4fv(uniform.gizmo_chunk.mat_translation,
            1, GL_FALSE, (GLfloat*)&camera->projection.target);

    glUniformMatrix4fv(uniform.gizmo_chunk.mat_rotation,
            1, GL_FALSE, (GLfloat*)&camera->projection.rotation);

    glUniformMatrix4fv(uniform.gizmo_chunk.mat_orientation,
            1, GL_FALSE, (GLfloat*)&camera->projection.orientation);

    glUniformMatrix4fv(uniform.gizmo_chunk.mat_projection,
            1, GL_FALSE, (GLfloat*)&camera->projection.projection);

    camera_position.x = -camera->yaw.cos * camera->pitch.cos;
    camera_position.y = camera->yaw.sin * camera->pitch.cos;
    camera_position.z = camera->pitch.sin;

    glUniform3fv(uniform.gizmo_chunk.camera_position, 1, (GLfloat*)&camera_position);
    glUniform1f(uniform.gizmo_chunk.time, render->time);

    glDisable(GL_BLEND);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(chunk_gizmo_loaded.vao);
    glDrawArrays(GL_POINTS, 0, settings.chunk_buf_volume);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(chunk_gizmo_visible.vao);
    glDrawArrays(GL_POINTS, 0, settings.chunk_buf_volume);
    glEnable(GL_BLEND);
}

void chunk_debug_chunk_gizmo_write_internal(hhc_chunk *ch)
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

    if (ch->flag & FLAG_CHUNK_VISIBLE)
    {
        chunk_gizmo_visible.p[ch->index].x =
            (chunk_pos.x << 0x18) | (chunk_pos.y << 0x10) | (chunk_pos.z << 0x08);
        chunk_gizmo_visible.p[ch->index].y =
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
        chunk_gizmo_visible.p[ch->index].y = 0;
    }
    else
    {
        chunk_gizmo_loaded.p[ch->index].y = 0;
        chunk_gizmo_visible.p[ch->index].y = 0;
    }

    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_loaded.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, ch->index * sizeof(v2u32), sizeof(v2u32),
            &chunk_gizmo_loaded.p[ch->index]);
    glBindBuffer(GL_ARRAY_BUFFER, chunk_gizmo_visible.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, ch->index * sizeof(v2u32), sizeof(v2u32),
            &chunk_gizmo_visible.p[ch->index]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void chunk_debug_scheduler_visualizer_draw(const fsl_camera *camera, f32 opacity)
{
    fsl_mesh *mesh_p = fsl_mem_handle_get(mesh);

    glClear(GL_DEPTH_BUFFER_BIT);
    chunk_debug_scheduler_visualizer_draw_internal(chunk_sched[2],
            &mesh_p[MESH_CUBE_OF_HAPPINESS], camera,
            0.9f, 0.3f, 0.3f, opacity);
    glClear(GL_DEPTH_BUFFER_BIT);
    chunk_debug_scheduler_visualizer_draw_internal(chunk_sched[1],
            &mesh_p[MESH_CUBE_OF_HAPPINESS], camera,
            0.9f, 0.6f, 0.3f, opacity);
    glClear(GL_DEPTH_BUFFER_BIT);
    chunk_debug_scheduler_visualizer_draw_internal(chunk_sched[0],
            &mesh_p[MESH_CUBE_OF_HAPPINESS], camera,
            0.9f, 0.6f, 0.3f, opacity);
}

static void chunk_debug_scheduler_visualizer_draw_internal(hhc_chunk_scheduler sched,
        const fsl_mesh *mesh_bounding_box, const fsl_camera *camera,
        f32 color_r, f32 color_g, f32 color_b, f32 color_a)
{
    u32 pop = sched.cursor_pop;
    u32 count = sched.count;

    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&camera->projection.perspective);
    glUniform3f(uniform.bounding_box.size,
            CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);

    while (count--)
    {
        glUniform3f(uniform.bounding_box.position,
                (f32)(sched.p[pop]->pos_world.x * CHUNK_DIAMETER),
                (f32)(sched.p[pop]->pos_world.y * CHUNK_DIAMETER),
                (f32)(sched.p[pop]->pos_world.z * CHUNK_DIAMETER));

        glUniform4f(uniform.bounding_box.color, color_r, color_g, color_b, color_a);
        glBindVertexArray(mesh_bounding_box->vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);

        ++pop;
        if (pop == sched.len)
            pop = 0;
    }
}
