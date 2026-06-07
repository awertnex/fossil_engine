/*!
 *  Copyright 2026 Lily Awertnex
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*!
 *  @file engine_assets.c
 *
 *  @brief loading and unloading engine's default assets.
 */

#include "../common/config.h"
#include "../common/diagnostics.h"
#include "../common/types.h"
#include "../assets/assets.h"
#include "../memory/memory.h"
#include "../shaders/shader_types.h"
#include "../shaders/shaders.h"

#include "engine_assets.h"

#include <stdio.h>

/* ---- section: declarations ----------------------------------------------- */

fsl_mem_handle fsl_texture_buf = {0};
fsl_mem_handle fsl_shader_buf = {0};
fsl_mem_handle fsl_font_buf = {0};
fsl_mem_handle fsl_mesh_buf = {0};
fsl_mesh fsl_mesh_unit_quad = {0};

/* ---- section: implementation --------------------------------------------- */

u32 fsl_engine_assets_init(void)
{
    fsl_texture *texture_p = NULL;
    fsl_shader_program *shader_p = NULL;
    fsl_font *font_p = NULL;
    fsl_mesh *mesh_p = NULL;

    /* ---- engine textures ------------------------------------------------- */

    if (fsl_mem_arena_push(&mem_arena_internal, &fsl_texture_buf,
                FSL_TEXTURE_INDEX_COUNT * sizeof(fsl_texture), "fsl_engine_assets_init().fsl_texture_buf") != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_mem_arena_push(&mem_arena_internal, &fsl_shader_buf,
                FSL_SHADER_INDEX_COUNT * sizeof(fsl_shader_program), "fsl_engine_assets_init().fsl_shader_buf") != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_mem_arena_push(&mem_arena_internal, &fsl_font_buf,
                FSL_FONT_INDEX_COUNT * sizeof(fsl_font), "fsl_engine_assets_init().fsl_font_buf") != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_mem_arena_push(&mem_arena_internal, &fsl_mesh_buf,
                FSL_MESH_INDEX_COUNT * sizeof(fsl_mesh), "fsl_engine_assets_init().fsl_mesh_buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    texture_p = fsl_mem_handle_get(fsl_texture_buf);
    shader_p = fsl_mem_handle_get(fsl_shader_buf);
    font_p = fsl_mem_handle_get(fsl_font_buf);
    mesh_p = fsl_mem_handle_get(fsl_mesh_buf);

    if (fsl_texture_init(&texture_p[FSL_TEXTURE_INDEX_PANEL_ACTIVE],
                "Panel Active", "panel_active", "panel_active.png", FSL_DIR_NAME_TEXTURES,
                GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_texture_init(&texture_p[FSL_TEXTURE_INDEX_PANEL_INACTIVE],
                "Panel Inactive", "panel_inactive", "panel_inactive.png", FSL_DIR_NAME_TEXTURES,
                GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_texture_init(&texture_p[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE],
                "Panel Debug 9-Slice", "panel_debug_9_slice", "panel_debug_nine_slice.png", FSL_DIR_NAME_TEXTURES,
                GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_texture_init(&texture_p[FSL_TEXTURE_INDEX_BUTTON_SELECTED],
                "Button Selected", "button_selected", "button_selected.png", FSL_DIR_NAME_TEXTURES,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_texture_init(&texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE],
                "Button Active", "button_active", "button_active.png", FSL_DIR_NAME_TEXTURES,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_texture_init(&texture_p[FSL_TEXTURE_INDEX_BUTTON_INACTIVE],
                "Button Inactive", "button_inactive", "button_inactive.png", FSL_DIR_NAME_TEXTURES,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- engine meshes --------------------------------------------------- */

    if (fsl_mesh_load(&mesh_p[FSL_MESH_INDEX_SKYBOX], "Skybox", "skybox", "skybox.obj", FSL_DIR_NAME_MODELS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_asset_set_metadata(&fsl_mesh_unit_quad.asset, FSL_ASSET_MESH, "Unit Quad", "unit_quad", NULL, NULL) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- engine shaders -------------------------------------------------- */

    if (fsl_shader_program_init_ex(&shader_p[FSL_SHADER_INDEX_UNIT_QUAD],
                "Unit Quad", "unit_quad", "unit_quad.vert", NULL, "unit_quad.frag",
                FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[FSL_SHADER_INDEX_TEXT],
                "Text", "text", "text.vert", NULL, "text.frag",
                FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[FSL_SHADER_INDEX_UI],
                "UI", "ui", "ui.vert", NULL, "ui.frag",
                FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[FSL_SHADER_INDEX_UI_9_SLICE],
                "UI 9-Slice", "ui_9_slice", "ui_9_slice.vert", NULL, "ui_9_slice.frag",
                FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[FSL_SHADER_INDEX_OBJECT],
                "Object", "object", "object.vert", NULL, "object.frag",
                FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- engine fonts ---------------------------------------------------- */

    if (
            fsl_font_init(&font_p[FSL_FONT_INDEX_DEJAVU_SANS], FSL_FONT_RESOLUTION_DEFAULT,
                "DejaVu Sans (ANSI)", "dejavu_sans_ansi", "dejavu-fonts-ttf/dejavu_sans_ansi.ttf",
                FSL_DIR_NAME_FONTS) != FSL_ERR_SUCCESS ||

            fsl_font_init(&font_p[FSL_FONT_INDEX_DEJAVU_SANS_BOLD], FSL_FONT_RESOLUTION_DEFAULT,
                "DejaVu Sans Bold (ANSI)", "dejavu_sans_bold_ansi", "dejavu-fonts-ttf/dejavu_sans_bold_ansi.ttf",
                FSL_DIR_NAME_FONTS) != FSL_ERR_SUCCESS ||

            fsl_font_init(&font_p[FSL_FONT_INDEX_DEJAVU_SANS_MONO], FSL_FONT_RESOLUTION_DEFAULT,
                "DejaVu Sans Mono (ANSI)", "dejavu_sans_mono_ansi", "dejavu-fonts-ttf/dejavu_sans_mono_ansi.ttf",
                FSL_DIR_NAME_FONTS) != FSL_ERR_SUCCESS ||

            fsl_font_init(&font_p[FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD], FSL_FONT_RESOLUTION_DEFAULT,
                "DejaVu Sans Mono Bold (ANSI)", "dejavu_sans_mono_bold_ansi", "dejavu-fonts-ttf/dejavu_sans_mono_bold_ansi.ttf",
                FSL_DIR_NAME_FONTS) != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    return fsl_err;
}

void fsl_assets_free(void)
{
    i32 i = 0;
    fsl_texture *texture_p = NULL;
    fsl_shader_program *shader_p = NULL;
    fsl_font *font_p = NULL;

    fsl_mesh_free(&fsl_mesh_unit_quad);

    if (fsl_texture_buf.arena)
    {
        texture_p = fsl_mem_handle_get(fsl_texture_buf);
        for (i = 0; i < FSL_TEXTURE_INDEX_COUNT; ++i)
            fsl_texture_free(&texture_p[i]);
    }
    if (fsl_shader_buf.arena)
    {
        shader_p = fsl_mem_handle_get(fsl_shader_buf);
        for (i = 0; i < FSL_SHADER_INDEX_COUNT; ++i)
            fsl_shader_program_free(&shader_p[i]);
    }
    if (fsl_font_buf.arena)
    {
        font_p = fsl_mem_handle_get(fsl_font_buf);
        for (i = 0; i < FSL_FONT_INDEX_COUNT; ++i)
            fsl_font_free(&font_p[i]);
    }
}
