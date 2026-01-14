#include "h/core.h"
#include "h/defaults.h"
#include "h/diagnostics.h"
#include "h/ui.h"

static struct /* ui_core */
{
    b8 multisample;
    v2f32 ndc_scale;

    struct /* uniform */
    {
        struct /* ui */
        {
            GLint ndc_scale;
            GLint position;
            GLint size;
            GLint texture_size;
            GLint offset;
            GLint alignment;
            GLint tint;
        } ui;

        struct /* nine_slice */
        {
            GLint ndc_scale;
            GLint position;
            GLint size;
            GLint texture_size;
            GLint alignment;
            GLint tint;
            GLint slice;
            GLint slice_size;
            GLint sprite_size;
        } nine_slice;

    } uniform;

    FBO fbo;
} ui_core;

u32 ui_init(b8 multisample)
{
    if (
            texture_init(&engine_texture[ENGINE_TEXTURE_PANEL_ACTIVE], (v2i32){32, 32},
                GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                "engine/assets/textures/panel_active.png") != ERR_SUCCESS ||

            texture_init(&engine_texture[ENGINE_TEXTURE_PANEL_INACTIVE], (v2i32){32, 32},
                GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                "engine/assets/textures/panel_inactive.png") != ERR_SUCCESS)
        goto cleanup;

    if (fbo_init(&ui_core.fbo, &engine_mesh_unit, multisample, 4) != ERR_SUCCESS)
        goto cleanup;

    ui_core.multisample = multisample;

    ui_core.uniform.ui.ndc_scale =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI].id, "ndc_scale");
    ui_core.uniform.ui.position =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI].id, "position");
    ui_core.uniform.ui.size =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI].id, "size");
    ui_core.uniform.ui.texture_size =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI].id, "texture_size");
    ui_core.uniform.ui.offset =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI].id, "offset");
    ui_core.uniform.ui.alignment =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI].id, "alignment");
    ui_core.uniform.ui.tint =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI].id, "tint");

    ui_core.uniform.nine_slice.ndc_scale =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "ndc_scale");
    ui_core.uniform.nine_slice.position =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "position");
    ui_core.uniform.nine_slice.size =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "size");
    ui_core.uniform.nine_slice.texture_size =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "texture_size");
    ui_core.uniform.nine_slice.alignment =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "alignment");
    ui_core.uniform.nine_slice.tint =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "tint");
    ui_core.uniform.nine_slice.slice =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "slice");
    ui_core.uniform.nine_slice.slice_size =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "slice_size");
    ui_core.uniform.nine_slice.sprite_size =
        glGetUniformLocation(engine_shader[ENGINE_SHADER_UI_9_SLICE].id, "sprite_size");

    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:

    ui_free();
    return engine_err;
}

void ui_start(FBO *fbo, b8 nine_slice, b8 clear)
{
    static v2i32 render_size = {0};
    FBO *_fbo = &ui_core.fbo;

    if (fbo) _fbo = fbo;
    if (render_size.x != render->size.x || render_size.y != render->size.y)
        fbo_realloc(_fbo, ui_core.multisample, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo->fbo);

    ui_core.ndc_scale.x = 2.0f / render->size.x;
    ui_core.ndc_scale.y = 2.0f / render->size.y;

    glUniform2fv(ui_core.uniform.ui.ndc_scale, 1, (GLfloat*)&ui_core.ndc_scale);
    glUniform2fv(ui_core.uniform.nine_slice.ndc_scale, 1, (GLfloat*)&ui_core.ndc_scale);

    if (nine_slice)
        glUseProgram(engine_shader[ENGINE_SHADER_UI_9_SLICE].id);
    else
        glUseProgram(engine_shader[ENGINE_SHADER_UI].id);
    glBindVertexArray(engine_mesh_unit.vao);
    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
}

void ui_push_panel()
{
}

void ui_render(void)
{
    glBindTexture(GL_TEXTURE_2D, engine_texture[ENGINE_TEXTURE_PANEL_ACTIVE].id);
    glDrawArrays(GL_POINTS, 0, 1);
}

void ui_draw(Texture texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint)
{
    glUniform2i(ui_core.uniform.ui.position, pos_x, pos_y);
    glUniform2i(ui_core.uniform.ui.size, size_x, size_y);
    glUniform2i(ui_core.uniform.ui.texture_size, texture.size.x, texture.size.y);
    glUniform2f(ui_core.uniform.ui.offset, offset_x, offset_y);
    glUniform2i(ui_core.uniform.ui.alignment, align_x, align_y);
    glUniform4f(ui_core.uniform.ui.tint,
            (f32)((tint >> 0x18) & 0xff) / 0xff,
            (f32)((tint >> 0x10) & 0xff) / 0xff,
            (f32)((tint >> 0x08) & 0xff) / 0xff,
            (f32)((tint >> 0x00) & 0xff) / 0xff);

    glBindTexture(GL_TEXTURE_2D, texture.id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void ui_stop(void)
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ui_fbo_blit(GLuint fbo)
{
    glUseProgram(engine_shader[ENGINE_SHADER_UNIT_QUAD].id);
    glBindVertexArray(engine_mesh_unit.vao);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, ui_core.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void ui_free(void)
{
    fbo_free(&ui_core.fbo);
}
