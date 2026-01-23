#include "h/common.h"
#include "h/core.h"
#include "h/diagnostics.h"
#include "h/logger.h"
#include "h/shaders.h"
#include "h/ui.h"

static struct /* fsl_ui_core */
{
    b8 multisample;
    b8 use_nine_slice;

    struct /* uniform */
    {
        struct /* ui */
        {
            GLint position;
            GLint size;
            GLint texture_size;
            GLint offset;
            GLint alignment;
            GLint tint;
        } ui;

        struct /* nine_slice */
        {
            GLint position;
            GLint size;
            GLint texture_size;
            GLint sprite_size;
            GLint alignment;
            GLint tint;
            GLint use_nine_slice;
            GLint slice_size;
        } nine_slice;

    } uniform;

    fsl_fbo fbo;
} fsl_ui_core;

u32 fsl_ui_init(b8 multisample)
{
    u32 i = 0;

    if (
            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_ACTIVE], (v2i32){32, 32},
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                FSL_DIR_NAME_TEXTURES"panel_active.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_INACTIVE], (v2i32){32, 32},
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                FSL_DIR_NAME_TEXTURES"panel_inactive.png") != FSL_ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < FSL_TEXTURE_INDEX_COUNT; ++i)
        if (fsl_texture_generate(&fsl_texture_buf[i], FALSE) != FSL_ERR_SUCCESS)
            goto cleanup;

    if (fsl_fbo_init(&fsl_ui_core.fbo, &fsl_mesh_unit_quad, multisample, 4) != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_ui_core.multisample = multisample;

    fsl_ui_core.uniform.ui.position =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "position");
    fsl_ui_core.uniform.ui.size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "size");
    fsl_ui_core.uniform.ui.texture_size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "texture_size");
    fsl_ui_core.uniform.ui.offset =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "offset");
    fsl_ui_core.uniform.ui.alignment =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "alignment");
    fsl_ui_core.uniform.ui.tint =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "tint");

    fsl_ui_core.uniform.nine_slice.position =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "position");
    fsl_ui_core.uniform.nine_slice.size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "size");
    fsl_ui_core.uniform.nine_slice.texture_size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "texture_size");
    fsl_ui_core.uniform.nine_slice.alignment =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "alignment");
    fsl_ui_core.uniform.nine_slice.tint =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "tint");
    fsl_ui_core.uniform.nine_slice.use_nine_slice =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "use_nine_slice");
    fsl_ui_core.uniform.nine_slice.slice_size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "slice_size");
    fsl_ui_core.uniform.nine_slice.sprite_size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "sprite_size");

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_ui_free();
    _LOGFATAL(FALSE, FSL_ERR_UI_INIT_FAIL, "%s\n", "Failed to Initialize UI, Process Aborted");
    return fsl_err;
}

void ui_start(fsl_fbo *fbo, b8 nine_slice, b8 clear)
{
    static v2i32 render_size = {0};
    fsl_fbo *_fbo = &fsl_ui_core.fbo;

    if (fbo) _fbo = fbo;
    if (render_size.x != render->size.x || render_size.y != render->size.y)
        fsl_fbo_realloc(_fbo, fsl_ui_core.multisample, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo->fbo);

    if (nine_slice)
    {
        fsl_ui_core.use_nine_slice = TRUE;
        glUseProgram(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id);
    }
    else
    {
        fsl_ui_core.use_nine_slice = FALSE;
        glUseProgram(fsl_shader_buf[FSL_SHADER_INDEX_UI].id);
    }
    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
}

void fsl_ui_push_panel()
{
}

void fsl_ui_render(void)
{
    glBindTexture(GL_TEXTURE_2D, fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_ACTIVE].id);
    glDrawArrays(GL_POINTS, 0, 1);
}

void fsl_ui_draw(fsl_texture *texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint)
{
    glUniform2i(fsl_ui_core.uniform.ui.position, pos_x, pos_y);
    glUniform2i(fsl_ui_core.uniform.ui.size, size_x, size_y);
    glUniform2i(fsl_ui_core.uniform.ui.texture_size, texture->size.x, texture->size.y);
    glUniform2f(fsl_ui_core.uniform.ui.offset, offset_x, offset_y);
    glUniform2i(fsl_ui_core.uniform.ui.alignment, align_x, align_y);
    glUniform4f(fsl_ui_core.uniform.ui.tint,
            (f32)((tint >> 0x18) & 0xff) / 0xff,
            (f32)((tint >> 0x10) & 0xff) / 0xff,
            (f32)((tint >> 0x08) & 0xff) / 0xff,
            (f32)((tint >> 0x00) & 0xff) / 0xff);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void fsl_ui_draw_nine_slice(fsl_texture *texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 slice_size, f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint)
{
    glUniform2i(fsl_ui_core.uniform.nine_slice.position, pos_x, pos_y);
    glUniform2i(fsl_ui_core.uniform.nine_slice.size, size_x, size_y);
    glUniform2i(fsl_ui_core.uniform.nine_slice.alignment, align_x, align_y);
    glUniform4f(fsl_ui_core.uniform.nine_slice.tint,
            (f32)((tint >> 0x18) & 0xff) / 0xff,
            (f32)((tint >> 0x10) & 0xff) / 0xff,
            (f32)((tint >> 0x08) & 0xff) / 0xff,
            (f32)((tint >> 0x00) & 0xff) / 0xff);
    glUniform2iv(fsl_ui_core.uniform.nine_slice.texture_size, 1, (GLint*)&texture->size);
    glUniform2i(fsl_ui_core.uniform.nine_slice.sprite_size, texture->size.x / 2, texture->size.y / 2);
    glUniform1i(fsl_ui_core.uniform.nine_slice.use_nine_slice, fsl_ui_core.use_nine_slice);
    glUniform1f(fsl_ui_core.uniform.nine_slice.slice_size, slice_size);

    glBindTexture(GL_TEXTURE_2D, texture->id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void fsl_ui_stop(void)
{
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void fsl_ui_fbo_blit(GLuint fbo)
{
    glUseProgram(fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, fsl_ui_core.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void fsl_ui_free(void)
{
    fsl_fbo_free(&fsl_ui_core.fbo);
}
