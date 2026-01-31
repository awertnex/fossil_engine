#include <src/h/fossil_engine.h>

#define MARGIN 10

struct slice_data
{
    v2f32 pos;
    v2f32 size;
    v2f32 tex_coords_pos;
    v2f32 tex_coords_size;
}; /* slice_data */

typedef struct nine_slice_data
{
    struct slice_data slice[9];
} nine_slice_data;

fsl_fbo fbo = {0};
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
    b8 loaded;
    nine_slice_data panel;
} core;

void callback_framebuffer_size(i32 size_x, i32 size_y)
{
    (void)size_x;
    (void)size_y;

    fsl_fbo_realloc(&fbo, FALSE, 4);
}

nine_slice_data get_nine_slice(v2i32 texture_size, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y, i32 slice_size)
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

    return (nine_slice_data){
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
    nine_slice_data panel = get_nine_slice(fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE].size,
            MARGIN, MARGIN,
            render->size.x - MARGIN * 2, render->size.y - MARGIN * 2,
            render->time / 100000000L);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(nine_slice.id);

    glBindBuffer(GL_ARRAY_BUFFER, core.vbo_nine_slice);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(nine_slice_data), &panel);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(core.vao);
    glBindTexture(GL_TEXTURE_2D, fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE].id);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 9);
}

void draw_text(void)
{
    fsl_text_start(&fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD], 32.0f, 0, NULL, TRUE);

    fsl_text_push(fsl_stringf(
                "FPS [%u]",
                (u32)(1 / ((f64)render->time_delta * FSL_NSEC2SEC))),
            MARGIN, MARGIN, 0, 0,
            FSL_DIAGNOSTIC_COLOR_SUCCESS);

    fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
    fsl_text_stop();
    fsl_text_fbo_blit(0);
}

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, NULL, NULL, 1280, 720, NULL,
                FSL_FLAG_LOAD_DEFAULT_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (
            fsl_ui_init(FALSE) != FSL_ERR_SUCCESS ||
            fsl_text_init(FSL_FONT_RESOLUTION_DEFAULT, FALSE) ||
            fsl_fbo_init(&fbo, NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(".", &nine_slice) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (!core.loaded)
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(nine_slice_data),
                NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct slice_data), (void*)0);
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct slice_data), (void*)(2 * sizeof(f32)));
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct slice_data), (void*)(4 * sizeof(f32)));
        glVertexAttribDivisor(4, 1);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct slice_data), (void*)(6 * sizeof(f32)));
        glVertexAttribDivisor(5, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        core.loaded = TRUE;
    }

    while (fsl_engine_running(callback_framebuffer_size))
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        draw_nine_slice();
        draw_text();

        if (fsl_is_key_press(FSL_KEY_Q))
            fsl_request_engine_close();
    }

cleanup:

    if (core.loaded)
    {
        glDeleteVertexArrays(1, &core.vao);
        glDeleteBuffers(1, &core.vbo_unit_quad);
        glDeleteBuffers(1, &core.vbo_nine_slice);
        core.loaded = FALSE;
    }

    fsl_engine_close();
    return fsl_err;
}
