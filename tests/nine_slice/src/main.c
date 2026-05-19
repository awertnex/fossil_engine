#include "deps/fossil/fossil_engine.h"

#define MARGIN 10

typedef struct panel_slice
{
    v2f32 pos;
    v2f32 size;
    v2f32 tex_coords_pos;
    v2f32 tex_coords_size;
} panel_slice;

typedef struct panel_nine_slice
{
    struct panel_slice slice[9];
} panel_nine_slice;

fsl_key_bind bind_quit = {0};
fsl_shader_program nine_slice = {0};

static f32 vbo_data_unit_quad[] =
{
    0.0f, -1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f
};

static struct /* core */
{
    GLuint vao;
    GLuint vbo_unit_quad;
    GLuint vbo_nine_slice;
    b8 vao_loaded;
    panel_nine_slice panel;
} core;

fsl_render *render = NULL;

panel_nine_slice get_nine_slice(fsl_texture *texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y, i32 slice_size)
{
    panel_nine_slice panel = {0};
    f32 _pos_x = (f32)pos_x;
    f32 _pos_y = (f32)pos_y;
    f32 _size_x = (f32)size_x;
    f32 _size_y = (f32)size_y;
    f32 _slice_size = (f32)slice_size;
    v2f32 _texture_scale = {0};
    v2f32 _pos[3] = {0};
    v2f32 _size[3] = {0};
    v2f32 _tex_coords_pos[3] = {0};
    v2f32 _tex_coords_size[3] = {0};

    _texture_scale.x = 1.0f / texture->size.x;
    _texture_scale.y = 1.0f / texture->size.y;

    _pos[0].x = _pos_x;
    _pos[0].y = _pos_y;
    _pos[1].x = _pos_x + _slice_size;
    _pos[1].y = _pos_y + _slice_size;
    _pos[2].x = _pos_x + _size_x - _slice_size;
    _pos[2].y = _pos_y + _size_y - _slice_size;

    _size[0].x = _slice_size;
    _size[0].y = _slice_size;
    _size[1].x = _size_x - _slice_size * 2;
    _size[1].y = _size_y - _slice_size * 2;
    _size[2].x = _slice_size;
    _size[2].y = _slice_size;

    _tex_coords_pos[0].x = 0.0f;
    _tex_coords_pos[0].y = 0.0f;
    _tex_coords_pos[1].x = _texture_scale.x * _slice_size;
    _tex_coords_pos[1].y = _texture_scale.y * _slice_size;
    _tex_coords_pos[2].x = 1.0f - _texture_scale.x * _slice_size;
    _tex_coords_pos[2].y = 1.0f - _texture_scale.y * _slice_size;

    _tex_coords_size[0].x = _texture_scale.x * _slice_size;
    _tex_coords_size[0].y = _texture_scale.y * _slice_size;
    _tex_coords_size[1].x = 1.0f - _texture_scale.x * _slice_size * 2.0f;
    _tex_coords_size[1].y = 1.0f - _texture_scale.y * _slice_size * 2.0f;
    _tex_coords_size[2].x = _texture_scale.x * _slice_size;
    _tex_coords_size[2].y = _texture_scale.y * _slice_size;

    panel.slice[0].pos.x = _pos[0].x;
    panel.slice[0].pos.y = _pos[0].y;
    panel.slice[0].size.x = _size[0].x;
    panel.slice[0].size.y = _size[0].y;
    panel.slice[0].tex_coords_pos.x = _tex_coords_pos[0].x;
    panel.slice[0].tex_coords_pos.y = _tex_coords_pos[0].y;
    panel.slice[0].tex_coords_size.x = _tex_coords_size[0].x;
    panel.slice[0].tex_coords_size.y = _tex_coords_size[0].y;

    panel.slice[1].pos.x = _pos[1].x;
    panel.slice[1].pos.y = _pos[0].y;
    panel.slice[1].size.x = _size[1].x;
    panel.slice[1].size.y = _size[0].y;
    panel.slice[1].tex_coords_pos.x = _tex_coords_pos[1].x;
    panel.slice[1].tex_coords_pos.y = _tex_coords_pos[0].y;
    panel.slice[1].tex_coords_size.x = _tex_coords_size[1].x;
    panel.slice[1].tex_coords_size.y = _tex_coords_size[0].y;

    panel.slice[2].pos.x = _pos[2].x;
    panel.slice[2].pos.y = _pos[0].y;
    panel.slice[2].size.x = _size[2].x;
    panel.slice[2].size.y = _size[0].y;
    panel.slice[2].tex_coords_pos.x = _tex_coords_pos[2].x;
    panel.slice[2].tex_coords_pos.y = _tex_coords_pos[0].y;
    panel.slice[2].tex_coords_size.x = _tex_coords_size[2].x;
    panel.slice[2].tex_coords_size.y = _tex_coords_size[0].y;

    panel.slice[3].pos.x = _pos[0].x;
    panel.slice[3].pos.y = _pos[1].y;
    panel.slice[3].size.x = _size[0].x;
    panel.slice[3].size.y = _size[1].y;
    panel.slice[3].tex_coords_pos.x = _tex_coords_pos[0].x;
    panel.slice[3].tex_coords_pos.y = _tex_coords_pos[1].y;
    panel.slice[3].tex_coords_size.x = _tex_coords_size[0].x;
    panel.slice[3].tex_coords_size.y = _tex_coords_size[1].y;

    panel.slice[4].pos.x = _pos[1].x;
    panel.slice[4].pos.y = _pos[1].y;
    panel.slice[4].size.x = _size[1].x;
    panel.slice[4].size.y = _size[1].y;
    panel.slice[4].tex_coords_pos.x = _tex_coords_pos[1].x;
    panel.slice[4].tex_coords_pos.y = _tex_coords_pos[1].y;
    panel.slice[4].tex_coords_size.x = _tex_coords_size[1].x;
    panel.slice[4].tex_coords_size.y = _tex_coords_size[1].y;

    panel.slice[5].pos.x = _pos[2].x;
    panel.slice[5].pos.y = _pos[1].y;
    panel.slice[5].size.x = _size[2].x;
    panel.slice[5].size.y = _size[1].y;
    panel.slice[5].tex_coords_pos.x = _tex_coords_pos[2].x;
    panel.slice[5].tex_coords_pos.y = _tex_coords_pos[1].y;
    panel.slice[5].tex_coords_size.x = _tex_coords_size[2].x;
    panel.slice[5].tex_coords_size.y = _tex_coords_size[1].y;

    panel.slice[6].pos.x = _pos[0].x;
    panel.slice[6].pos.y = _pos[2].y;
    panel.slice[6].size.x = _size[0].x;
    panel.slice[6].size.y = _size[2].y;
    panel.slice[6].tex_coords_pos.x = _tex_coords_pos[0].x;
    panel.slice[6].tex_coords_pos.y = _tex_coords_pos[2].y;
    panel.slice[6].tex_coords_size.x = _tex_coords_size[0].x;
    panel.slice[6].tex_coords_size.y = _tex_coords_size[2].y;

    panel.slice[7].pos.x = _pos[1].x;
    panel.slice[7].pos.y = _pos[2].y;
    panel.slice[7].size.x = _size[1].x;
    panel.slice[7].size.y = _size[2].y;
    panel.slice[7].tex_coords_pos.x = _tex_coords_pos[1].x;
    panel.slice[7].tex_coords_pos.y = _tex_coords_pos[2].y;
    panel.slice[7].tex_coords_size.x = _tex_coords_size[1].x;
    panel.slice[7].tex_coords_size.y = _tex_coords_size[2].y;

    panel.slice[8].pos.x = _pos[2].x;
    panel.slice[8].pos.y = _pos[2].y;
    panel.slice[8].size.x = _size[2].x;
    panel.slice[8].size.y = _size[2].y;
    panel.slice[8].tex_coords_pos.x = _tex_coords_pos[2].x;
    panel.slice[8].tex_coords_pos.y = _tex_coords_pos[2].y;
    panel.slice[8].tex_coords_size.x = _tex_coords_size[2].x;
    panel.slice[8].tex_coords_size.y = _tex_coords_size[2].y;

    return panel;
}

void draw_nine_slice(void)
{
    panel_nine_slice panel = {0};
    fsl_texture *texture = fsl_mem_handle_get(fsl_texture_buf);
    texture = &texture[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE];
    panel = get_nine_slice(texture, MARGIN, MARGIN,
            render->size.x - MARGIN * 2, render->size.y - MARGIN * 2,
            render->time / 100000000UL);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(nine_slice.asset.id);

    glBindBuffer(GL_ARRAY_BUFFER, core.vbo_nine_slice);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(panel_nine_slice), &panel);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(core.vao);
    glBindTexture(GL_TEXTURE_2D, texture->asset.id);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 9);
}

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, NULL, 1280, 720, 0) != FSL_ERR_SUCCESS)
        goto cleanup;

    render = fsl_render_get();

    bind_quit = fsl_key_bind_init(FSL_KEY_Q, 0, 0, 0, 0, 0);

    if (
            fsl_asset_set_metadata(&nine_slice.asset, FSL_ASSET_SHADER_PROGRAM,
                "9-Slice", "nine_slice", NULL, NULL) != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&nine_slice.vertex.asset, FSL_ASSET_SHADER,
                "9-Slice", "nine_slice", "9s.vert", "./") != FSL_ERR_SUCCESS ||
            fsl_asset_set_metadata(&nine_slice.vertex.asset, FSL_ASSET_SHADER,
                "9-Slice", "nine_slice", "9s.frag", "./") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init(&nine_slice) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (!core.vao_loaded)
    {
        glGenVertexArrays(1, &core.vao);
        glBindVertexArray(core.vao);

        /* ---- unit quad --------------------------------------------------- */

        glGenBuffers(1, &core.vbo_unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, core.vbo_unit_quad);
        glBufferData(GL_ARRAY_BUFFER, fsl_arr_len(vbo_data_unit_quad) * sizeof(f32),
                &vbo_data_unit_quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(f32), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(f32), (void*)(2 * sizeof(f32)));

        /* ---- nine-slice data --------------------------------------------- */

        glGenBuffers(1, &core.vbo_nine_slice);
        glBindBuffer(GL_ARRAY_BUFFER, core.vbo_nine_slice);
        glBufferData(GL_ARRAY_BUFFER, sizeof(panel_nine_slice),
                NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                sizeof(panel_slice), (void*)0);
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                sizeof(panel_slice), (void*)(2 * sizeof(f32)));
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE,
                sizeof(panel_slice), (void*)(4 * sizeof(f32)));
        glVertexAttribDivisor(4, 1);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE,
                sizeof(panel_slice), (void*)(6 * sizeof(f32)));
        glVertexAttribDivisor(5, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        core.vao_loaded = TRUE;
    }

    while (fsl_engine_running(NULL))
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        draw_nine_slice();

        if (fsl_is_key_press(bind_quit))
            fsl_request_engine_close();
    }

cleanup:

    if (core.vao_loaded)
    {
        glDeleteVertexArrays(1, &core.vao);
        glDeleteBuffers(1, &core.vbo_unit_quad);
        glDeleteBuffers(1, &core.vbo_nine_slice);
        core.vao_loaded = FALSE;
    }

    fsl_shader_program_free(&nine_slice);
    fsl_engine_close();
    return fsl_err;
}
