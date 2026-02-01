#include <src/h/fossil_engine.h>

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

fsl_shader_program nine_slice =
{
    .vertex.file_name = "9s.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "9s.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static f32 vbo_data_unit_quad[] =
{
    0.0f, -1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f,
};

static struct core
{
    GLuint vao;
    GLuint vbo_unit_quad;
    GLuint vbo_nine_slice;
    b8 vao_loaded;
    panel_nine_slice panel;
} core;

panel_nine_slice get_nine_slice(v2i32 texture_size, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y, i32 slice_size)
{
    f32 _pos_x = (f32)pos_x;
    f32 _pos_y = (f32)pos_y;
    f32 _size_x = (f32)size_x;
    f32 _size_y = (f32)size_y;
    f32 _slice_size = (f32)slice_size;
    v2f32 _texture_scale =
    {
        1.0f / (f32)texture_size.x,
        1.0f / (f32)texture_size.y,
    };

    v2f32 _pos[3] =
    {
        [0] = {_pos_x, _pos_y},
        [1] = {_pos_x + _slice_size, _pos_y + _slice_size},
        [2] = {_pos_x + _size_x - _slice_size, _pos_y + _size_y - _slice_size},
    };

    v2f32 _size[3] =
    {
        [0] = {_slice_size, _slice_size},
        [1] = {_size_x - _slice_size * 2, _size_y - _slice_size * 2},
        [2] = {_slice_size, _slice_size},
    };

    v2f32 _tex_coords_pos[3] =
    {
        [0] = {0.0f, 0.0f},
        [1] = {_texture_scale.x * _slice_size, _texture_scale.y * _slice_size},
        [2] = {1.0f - _texture_scale.x * _slice_size, 1.0f - _texture_scale.y * _slice_size},
    };

    v2f32 _tex_coords_size[3] =
    {
        [0] = {_texture_scale.x * _slice_size, _texture_scale.y * _slice_size},
        [1] = {1.0f - _texture_scale.x * _slice_size * 2.0f, 1.0f - _texture_scale.y * _slice_size * 2.0f},
        [2] = {_texture_scale.x * _slice_size, _texture_scale.y * _slice_size},
    };

    return (panel_nine_slice){
        .slice[0] =
        {
            .pos.x = _pos[0].x,
            .pos.y = _pos[0].y,
            .size.x = _size[0].x,
            .size.y = _size[0].y,
            .tex_coords_pos.x = _tex_coords_pos[0].x,
            .tex_coords_pos.y = _tex_coords_pos[0].y,
            .tex_coords_size.x = _tex_coords_size[0].x,
            .tex_coords_size.y = _tex_coords_size[0].y,
        },

        .slice[1] =
        {
            .pos.x = _pos[1].x,
            .pos.y = _pos[0].y,
            .size.x = _size[1].x,
            .size.y = _size[0].y,
            .tex_coords_pos.x = _tex_coords_pos[1].x,
            .tex_coords_pos.y = _tex_coords_pos[0].y,
            .tex_coords_size.x = _tex_coords_size[1].x,
            .tex_coords_size.y = _tex_coords_size[0].y,
        },

        .slice[2] =
        {
            .pos.x = _pos[2].x,
            .pos.y = _pos[0].y,
            .size.x = _size[2].x,
            .size.y = _size[0].y,
            .tex_coords_pos.x = _tex_coords_pos[2].x,
            .tex_coords_pos.y = _tex_coords_pos[0].y,
            .tex_coords_size.x = _tex_coords_size[2].x,
            .tex_coords_size.y = _tex_coords_size[0].y,
        },

        .slice[3] =
        {
            .pos.x = _pos[0].x,
            .pos.y = _pos[1].y,
            .size.x = _size[0].x,
            .size.y = _size[1].y,
            .tex_coords_pos.x = _tex_coords_pos[0].x,
            .tex_coords_pos.y = _tex_coords_pos[1].y,
            .tex_coords_size.x = _tex_coords_size[0].x,
            .tex_coords_size.y = _tex_coords_size[1].y,
        },

        .slice[4] =
        {
            .pos.x = _pos[1].x,
            .pos.y = _pos[1].y,
            .size.x = _size[1].x,
            .size.y = _size[1].y,
            .tex_coords_pos.x = _tex_coords_pos[1].x,
            .tex_coords_pos.y = _tex_coords_pos[1].y,
            .tex_coords_size.x = _tex_coords_size[1].x,
            .tex_coords_size.y = _tex_coords_size[1].y,
        },

        .slice[5] =
        {
            .pos.x = _pos[2].x,
            .pos.y = _pos[1].y,
            .size.x = _size[2].x,
            .size.y = _size[1].y,
            .tex_coords_pos.x = _tex_coords_pos[2].x,
            .tex_coords_pos.y = _tex_coords_pos[1].y,
            .tex_coords_size.x = _tex_coords_size[2].x,
            .tex_coords_size.y = _tex_coords_size[1].y,
        },

        .slice[6] =
        {
            .pos.x = _pos[0].x,
            .pos.y = _pos[2].y,
            .size.x = _size[0].x,
            .size.y = _size[2].y,
            .tex_coords_pos.x = _tex_coords_pos[0].x,
            .tex_coords_pos.y = _tex_coords_pos[2].y,
            .tex_coords_size.x = _tex_coords_size[0].x,
            .tex_coords_size.y = _tex_coords_size[2].y,
        },

        .slice[7] =
        {
            .pos.x = _pos[1].x,
            .pos.y = _pos[2].y,
            .size.x = _size[1].x,
            .size.y = _size[2].y,
            .tex_coords_pos.x = _tex_coords_pos[1].x,
            .tex_coords_pos.y = _tex_coords_pos[2].y,
            .tex_coords_size.x = _tex_coords_size[1].x,
            .tex_coords_size.y = _tex_coords_size[2].y,
        },

        .slice[8] =
        {
            .pos.x = _pos[2].x,
            .pos.y = _pos[2].y,
            .size.x = _size[2].x,
            .size.y = _size[2].y,
            .tex_coords_pos.x = _tex_coords_pos[2].x,
            .tex_coords_pos.y = _tex_coords_pos[2].y,
            .tex_coords_size.x = _tex_coords_size[2].x,
            .tex_coords_size.y = _tex_coords_size[2].y,
        },
    };
}

void draw_nine_slice(void)
{
    panel_nine_slice panel = get_nine_slice(fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE].size,
            MARGIN, MARGIN,
            render->size.x - MARGIN * 2, render->size.y - MARGIN * 2,
            render->time / 100000000L);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(nine_slice.id);

    glBindBuffer(GL_ARRAY_BUFFER, core.vbo_nine_slice);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(panel_nine_slice), &panel);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(core.vao);
    glBindTexture(GL_TEXTURE_2D, fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE].id);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 9);
}

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, NULL, NULL, 1280, 720, NULL,
                FSL_FLAG_LOAD_DEFAULT_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (
            fsl_ui_init(FALSE) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(".", &nine_slice) != FSL_ERR_SUCCESS)
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

        /*
        fsl_ui_start(NULL, TRUE, TRUE);
        fsl_ui_draw_nine_slice(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE],
                MARGIN, MARGIN, render->size.x - MARGIN * 2, render->size.y - MARGIN * 2,
        0.0f, 0.0f, 0, 0, 64, 0xffffffff);
        fsl_ui_stop();
        fsl_ui_fbo_blit(0);
        */
        draw_nine_slice();

        if (fsl_is_key_press(FSL_KEY_Q))
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

    fsl_engine_close();
    return fsl_err;
}
