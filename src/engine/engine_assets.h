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
 *  @file engine_assets.h
 *
 *  @brief loading and unloading engine's default assets.
 */

#ifndef FSL_ENGINE_ASSETS_H
#define FSL_ENGINE_ASSETS_H

#include "../common/api.h"
#include "../common/types.h"
#include "../assets/mesh/mesh.h"
#include "../memory/memory_types.h"

enum fsl_shader_index
{
    FSL_SHADER_INDEX_UNIT_QUAD,
    FSL_SHADER_INDEX_TEXT,
    FSL_SHADER_INDEX_UI,
    FSL_SHADER_INDEX_UI_9_SLICE,
    FSL_SHADER_INDEX_OBJECT,
    FSL_SHADER_INDEX_COUNT
}; /* fsl_shader_index */

enum fsl_font_index
{
    FSL_FONT_INDEX_DEJAVU_SANS,
    FSL_FONT_INDEX_DEJAVU_SANS_BOLD,
    FSL_FONT_INDEX_DEJAVU_SANS_MONO,
    FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD,
    FSL_FONT_INDEX_COUNT
}; /* fsl_font_index */

enum fsl_texture_index
{
    FSL_TEXTURE_INDEX_PANEL_ACTIVE,
    FSL_TEXTURE_INDEX_PANEL_INACTIVE,
    FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE,
    FSL_TEXTURE_INDEX_BUTTON_SELECTED,
    FSL_TEXTURE_INDEX_BUTTON_ACTIVE,
    FSL_TEXTURE_INDEX_BUTTON_INACTIVE,
    FSL_TEXTURE_INDEX_COUNT
}; /* fsl_texture_index */

enum fsl_mesh_index
{
    FSL_MESH_INDEX_SKYBOX,
    FSL_MESH_INDEX_COUNT
}; /* fsl_mesh_index */

/* ---- section: declarations ----------------------------------------------- */

/*!
 *  @brief engine's default textures.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_engine_assets_init().
 */
FSLAPI extern fsl_mem_handle fsl_texture_buf;

/*!
 *  @brief engine's default shaders.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_engine_assets_init().
 */
FSLAPI extern fsl_mem_handle fsl_shader_buf;

/*!
 *  @brief engine's default fonts.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_engine_assets_init().
 */
FSLAPI extern fsl_mem_handle fsl_font_buf;

/*!
 *  @brief engine's default meshes.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_engine_assets_init().
 */
FSLAPI extern fsl_mem_handle fsl_mesh_buf;

/*!
 *  @brief engine's default unit quad, with texture coordinates.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_engine_assets_init().
 */
FSLAPI extern fsl_mesh fsl_mesh_unit_quad;

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @brief initialize engine's internal assets.
 *
 *  @remark called automatically from @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 fsl_engine_assets_init(void);

/*!
 *  @brief free all engine's internal assets.
 */
void fsl_assets_free(void);

#endif /* FSL_ENGINE_ASSETS_H */
