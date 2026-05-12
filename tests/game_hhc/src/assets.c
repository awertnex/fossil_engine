#include "src/common/config.h"
#include "src/common/types.h"
#include "src/assets/assets.h"
#include "src/engine/engine_default_assets.h"
#include "src/logger/logger.h"
#include "src/memory/memory.h"
#include "src/shaders/shaders.h"

#include "h/assets.h"
#include "h/diagnostics.h"
#include "h/main.h"

#include <stdio.h>

fsl_mem_handle fbo = {0};
fsl_mem_handle texture = {0};
fsl_mem_handle mesh = {0};
fsl_mem_handle shader = {0};
fsl_mem_handle blocks = {0};
static fsl_mem_handle block_textures = {0};
static GLuint ssbo_texture_indices_id = 0;
static u32 ssbo_texture_indices[BLOCK_COUNT * 6] = {0};
static GLuint ssbo_texture_handles_id = 0;
static u64 ssbo_texture_handles[TEXTURE_BLOCK_COUNT] = {0};
fsl_font *font[FONT_COUNT] = {0};

u32 assets_init(void)
{
    u32 i = 0;
    u32 j = 0;
    fsl_fbo *fbo_p = NULL;
    fsl_texture *texture_p = NULL;
    fsl_shader_program *shader_p = NULL;
    block *blocks_p = NULL;
    fsl_texture *block_textures_p = NULL;
    fsl_font *font_p = NULL;

    font_p = fsl_mem_handle_get(fsl_font, fsl_font_buf);
    font[FONT_REG] =        &font_p[FSL_FONT_INDEX_DEJAVU_SANS];
    font[FONT_REG_BOLD] =   &font_p[FSL_FONT_INDEX_DEJAVU_SANS_BOLD];
    font[FONT_MONO] =       &font_p[FSL_FONT_INDEX_DEJAVU_SANS_MONO];
    font[FONT_MONO_BOLD] =  &font_p[FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD];

    if (
            fsl_mem_arena_push(&memory_arena_internal, &fbo,
                FBO_COUNT * sizeof(fsl_fbo),
                "assets_init().fbo") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &texture,
                TEXTURE_COUNT * sizeof(fsl_texture),
                "assets_init().texture") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &mesh,
                MESH_COUNT * sizeof(fsl_mesh),
                "assets_init().mesh") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &shader,
                SHADER_COUNT * sizeof(fsl_shader_program),
                "assets_init().shader") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &blocks,
                BLOCK_COUNT * sizeof(block),
                "assets_init().blocks") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_internal, &block_textures,
                TEXTURE_BLOCK_COUNT * sizeof(fsl_texture),
                "assets_init().block_textures") != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- framebuffers ---------------------------------------------------- */

    fbo_p = fsl_mem_handle_get(fsl_fbo, fbo);
    if (
            fsl_fbo_init(&fbo_p[FBO_SKYBOX],     render->size.x, render->size.y, NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_WORLD],      render->size.x, render->size.y, NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_WORLD_MSAA], render->size.x, render->size.y, NULL, TRUE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_HUD],        render->size.x, render->size.y, NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_HUD_MSAA],   render->size.x, render->size.y, NULL, TRUE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo_p[FBO_POST_PROCESSING], render->size.x, render->size.y, NULL, FALSE, 4) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- shaders --------------------------------------------------------- */

    shader_p = fsl_mem_handle_get_i(fsl_shader_program, shader, SHADER_DEFAULT);
    if (
            fsl_asset_set_metadata(&shader_p->asset, FSL_ASSET_SHADER_PROGRAM,
                "Default", "default", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->vertex.asset, FSL_ASSET_SHADER,
                "Default", "default", "default.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->geometry.asset, FSL_ASSET_SHADER,
                NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->fragment.asset, FSL_ASSET_SHADER,
                "Default", "default", "default.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    shader_p = fsl_mem_handle_get_i(fsl_shader_program, shader, SHADER_SKYBOX);
    if (
            fsl_asset_set_metadata(&shader_p->asset, FSL_ASSET_SHADER_PROGRAM,
                "Skybox", "skybox", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->vertex.asset, FSL_ASSET_SHADER,
                "Skybox", "skybox", "skybox.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->geometry.asset, FSL_ASSET_SHADER,
                NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->fragment.asset, FSL_ASSET_SHADER,
                "Skybox", "skybox", "skybox.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    shader_p = fsl_mem_handle_get_i(fsl_shader_program, shader, SHADER_GIZMO);
    if (
            fsl_asset_set_metadata(&shader_p->asset, FSL_ASSET_SHADER_PROGRAM,
                "Gizmo", "gizmo", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->vertex.asset, FSL_ASSET_SHADER,
                "Gizmo", "gizmo", "gizmo.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->geometry.asset, FSL_ASSET_SHADER,
                NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->fragment.asset, FSL_ASSET_SHADER,
                "Gizmo", "gizmo", "gizmo.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    shader_p = fsl_mem_handle_get_i(fsl_shader_program, shader, SHADER_GIZMO_CHUNK);
    if (
            fsl_asset_set_metadata(&shader_p->asset, FSL_ASSET_SHADER_PROGRAM,
                "Gizmo Chunk", "gizmo_chunk", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->vertex.asset, FSL_ASSET_SHADER,
                "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->geometry.asset, FSL_ASSET_SHADER,
                "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.geom", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->fragment.asset, FSL_ASSET_SHADER,
                "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    shader_p = fsl_mem_handle_get_i(fsl_shader_program, shader, SHADER_POST_PROCESSING);
    if (
            fsl_asset_set_metadata(&shader_p->asset, FSL_ASSET_SHADER_PROGRAM,
                "Post Processing", "post_processing", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->vertex.asset, FSL_ASSET_SHADER,
                "Post Processing", "post_processing", "post_processing.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->geometry.asset, FSL_ASSET_SHADER,
                NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->fragment.asset, FSL_ASSET_SHADER,
                "Post Processing", "post_processing", "post_processing.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    shader_p = fsl_mem_handle_get_i(fsl_shader_program, shader, SHADER_VOXEL);
    if (
            fsl_asset_set_metadata(&shader_p->asset, FSL_ASSET_SHADER_PROGRAM,
                "Voxel", "voxel", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->vertex.asset, FSL_ASSET_SHADER,
                "Voxel", "voxel", "voxel.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->geometry.asset, FSL_ASSET_SHADER,
                "Voxel", "voxel", "voxel.geom", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->fragment.asset, FSL_ASSET_SHADER,
                "Voxel", "voxel", "voxel.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    shader_p = fsl_mem_handle_get_i(fsl_shader_program, shader, SHADER_BOUNDING_BOX);
    if (
            fsl_asset_set_metadata(&shader_p->asset, FSL_ASSET_SHADER_PROGRAM,
                "Bounding Box", "bounding_box", NULL, GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->vertex.asset, FSL_ASSET_SHADER,
                "Bounding Box", "bounding_box", "bounding_box.vert", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->geometry.asset, FSL_ASSET_SHADER,
                NULL, "NULL", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_set_metadata(&shader_p->fragment.asset, FSL_ASSET_SHADER,
                "Bounding Box", "bounding_box", "bounding_box.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    shader_p = fsl_mem_handle_get(fsl_shader_program, shader);
    for (i = 0; i < SHADER_COUNT; ++i)
        if (fsl_shader_program_init(&shader_p[i]) != FSL_ERR_SUCCESS)
            goto cleanup;

    /* ---- textures -------------------------------------------------------- */

    texture_p = fsl_mem_handle_get(fsl_texture, texture);
    if (
            fsl_texture_init(&texture_p[TEXTURE_CROSSHAIR],
                "Crosshair", "crosshair", "crosshair.png", GAME_DIR_NAME_GUI,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_ITEM_BAR],
                "Item Bar", "item_bar", "item_bar.png", GAME_DIR_NAME_GUI,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_SKYBOX_VAL],
                "Skybox Val", "skybox_val", "skybox_val.png", GAME_DIR_NAME_ENV,
                GL_RED, GL_NEAREST, FSL_COLOR_CHANNELS_GRAY, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_SKYBOX_HORIZON],
                "Skybox Horizon", "skybox_horizon", "skybox_horizon.png", GAME_DIR_NAME_ENV,
                GL_RED, GL_NEAREST, FSL_COLOR_CHANNELS_GRAY, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_SKYBOX_STARS],
                "Skybox Stars", "skybox_stars", "skybox_stars.png", GAME_DIR_NAME_ENV,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_SUN],
                "Sun", "sun", "sun.png", GAME_DIR_NAME_ENV,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_MOON],
                "Moon", "moon", "moon.png", GAME_DIR_NAME_ENV,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- block textures -------------------------------------------------- */

    if (
            block_texture_init(TEXTURE_BLOCK_GRASS_SIDE, "Grass Side", "grass_side", "grass_side.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_GRASS_TOP, "Grass Top", "grass_top", "grass_top.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_DIRT, "Dirt", "dirt", "dirt.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_DIRTUP, "Dirtup", "dirtup", "dirtup.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_STONE, "Stone", "stone", "stone.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_SAND, "Sand", "sand", "sand.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_GLASS, "Glass", "glass", "glass.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE, "Birch Wood Log Side", "wood_birch_log_side", "wood_birch_log_side.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP, "Birch Wood Log Top", "wood_birch_log_top", "wood_birch_log_top.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_PLANKS, "Birch Wood Planks", "wood_birch_planks", "wood_birch_planks.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE, "Cherry Wood Log Side", "wood_cherry_log_side", "wood_cherry_log_side.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP, "Cherry Wood Log Top", "wood_cherry_log_top", "wood_cherry_log_top.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_PLANKS, "Cherry Wood Planks", "wood_cherry_planks", "wood_cherry_planks.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE, "Oak Wood Log Side", "wood_oak_log_side", "wood_oak_log_side.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_LOG_TOP, "Oak Wood Log Top", "wood_oak_log_top", "wood_oak_log_top.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_PLANKS, "Oak Wood Planks", "wood_oak_planks", "wood_oak_planks.png") != FSL_ERR_SUCCESS ||
            block_texture_init(TEXTURE_BLOCK_BLOOD, "Blood Block", "block_blood", "block_blood.png") != FSL_ERR_SUCCESS)
        goto cleanup;

    block_textures_p = fsl_mem_handle_get(fsl_texture, block_textures);
    for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
        ssbo_texture_handles[i] = block_textures_p[i].bindless_handle;

    blocks_init();

    blocks_p = fsl_mem_handle_get(block, blocks);
    for (i = 0, j = 0; i < BLOCK_COUNT; ++i)
    {
        ssbo_texture_indices[j++] = blocks_p[i].texture_index[0];
        ssbo_texture_indices[j++] = blocks_p[i].texture_index[1];
        ssbo_texture_indices[j++] = blocks_p[i].texture_index[2];
        ssbo_texture_indices[j++] = blocks_p[i].texture_index[3];
        ssbo_texture_indices[j++] = blocks_p[i].texture_index[4];
        ssbo_texture_indices[j++] = blocks_p[i].texture_index[5];
    }

    glGenBuffers(1, &ssbo_texture_indices_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_texture_indices_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, BLOCK_COUNT * sizeof(u32) * 6,
                &ssbo_texture_indices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SHADER_BUFFER_BINDING_SSBO_TEXTURE_INDICES,
            ssbo_texture_indices_id);

    glGenBuffers(1, &ssbo_texture_handles_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_texture_handles_id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, TEXTURE_BLOCK_COUNT * sizeof(u64),
                &ssbo_texture_handles, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SHADER_BUFFER_BINDING_SSBO_TEXTURE_HANDLES,
            ssbo_texture_handles_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    assets_free();
    return *GAME_ERR;
}

void assets_free(void)
{
    u32 i = 0;
    fsl_fbo *fbo_p = NULL;
    fsl_texture *texture_p = NULL;
    fsl_mesh *mesh_p = NULL;
    fsl_shader_program *shader_p = NULL;
    fsl_texture *block_textures_p = NULL;
    fsl_font *font_p = NULL;

    if (block_textures.arena)
    {
        block_textures_p = fsl_mem_handle_get(fsl_texture, block_textures);
        for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
            fsl_texture_free(&block_textures_p[i]);
    }
    if (shader.arena)
    {
        shader_p = fsl_mem_handle_get(fsl_shader_program, shader);
        for (i = 0; i < SHADER_COUNT; ++i)
            fsl_shader_program_free(&shader_p[i]);
    }
    if (mesh.arena)
    {
        mesh_p = fsl_mem_handle_get(fsl_mesh, mesh);
        for (i = 0; i < MESH_COUNT; ++i)
            fsl_mesh_free(&mesh_p[i]);
    }
    if (texture.arena)
    {
        texture_p = fsl_mem_handle_get(fsl_texture, texture);
        for (i = 0; i < TEXTURE_COUNT; ++i)
            fsl_texture_free(&texture_p[i]);
    }
    if (fbo.arena)
    {
        fbo_p = fsl_mem_handle_get(fsl_fbo, fbo);
        for (i = 0; i < FBO_COUNT; ++i)
            fsl_fbo_free(&fbo_p[i]);
    }

    if (ssbo_texture_indices_id)
        glDeleteBuffers(1, &ssbo_texture_indices_id);

    if (ssbo_texture_handles_id)
        glDeleteBuffers(1, &ssbo_texture_handles_id);
}

u32 block_texture_init(u32 index, const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file)
{
    fsl_texture *block_texture = fsl_mem_handle_get_i(fsl_texture, block_textures, index);
    if (!name_id)
    {
        LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Failed to Initialize Texture [%u], `name_id` `NULL`\n", index));
        goto cleanup;
    }

    if (fsl_texture_init(block_texture, name, name_id, file, GAME_DIR_NAME_BLOCKS,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_texture_free(block_texture);
    return *GAME_ERR;
}

void blocks_init(void)
{
    block *blocks_p = fsl_mem_handle_get(block, blocks);

    fsl_asset_set_metadata(&blocks_p[BLOCK_NONE].asset, FSL_ASSET_CUSTOM,
            "None", "none", NULL, NULL);

    fsl_asset_set_metadata(&blocks_p[BLOCK_BLOOD].asset, FSL_ASSET_CUSTOM,
            "Blood Block", "block_blood", "block_blood", NULL);
    blocks_p[BLOCK_BLOOD].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_BLOOD].texture_index[0] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[1] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[2] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[3] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[4] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[5] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].friction = FRICTION_BLOCK_WET;

    fsl_asset_set_metadata(&blocks_p[BLOCK_GRASS].asset, FSL_ASSET_CUSTOM,
            "Grass Block", "block_grass", "block_grass", NULL);
    blocks_p[BLOCK_GRASS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_GRASS].texture_index[0] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks_p[BLOCK_GRASS].texture_index[1] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks_p[BLOCK_GRASS].texture_index[2] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks_p[BLOCK_GRASS].texture_index[3] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks_p[BLOCK_GRASS].texture_index[4] = TEXTURE_BLOCK_GRASS_TOP;
    blocks_p[BLOCK_GRASS].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_GRASS].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_DIRT].asset, FSL_ASSET_CUSTOM,
            "Dirt Block", "block_dirt", "block_dirt", NULL);
    blocks_p[BLOCK_DIRT].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_DIRT].texture_index[0] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[1] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[2] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[3] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[4] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_DIRTUP].asset, FSL_ASSET_CUSTOM,
            "Dirtup", "block_dirtup", "block_dirtup", NULL);
    blocks_p[BLOCK_DIRTUP].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_DIRTUP].texture_index[0] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[1] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[2] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[3] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[4] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[5] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_STONE].asset, FSL_ASSET_CUSTOM,
            "Stone", "block_stone", "block_stone", NULL);
    blocks_p[BLOCK_STONE].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_STONE].texture_index[0] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[1] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[2] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[3] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[4] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[5] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_SAND].asset, FSL_ASSET_CUSTOM,
            "Sand", "block_sand", "block_sand", NULL);
    blocks_p[BLOCK_SAND].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_SAND].texture_index[0] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[1] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[2] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[3] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[4] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[5] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_GLASS].asset, FSL_ASSET_CUSTOM,
            "Glass", "block_glass", "block_glass", NULL);
    blocks_p[BLOCK_GLASS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_GLASS].texture_index[0] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[1] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[2] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[3] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[4] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[5] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_BIRCH_LOG].asset, FSL_ASSET_CUSTOM,
            "Birch Wood Log", "wood_birch_log", "wood_birch_log", NULL);
    blocks_p[BLOCK_WOOD_BIRCH_LOG].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_BIRCH_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Birch Wood Planks", "wood_birch_planks", "wood_birch_planks", NULL);
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_CHERRY_LOG].asset, FSL_ASSET_CUSTOM,
            "Cherry Wood Log", "wood_cherry_log", "wood_cherry_log", NULL);
    blocks_p[BLOCK_WOOD_CHERRY_LOG].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_CHERRY_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Cherry Wood Planks", "wood_cherry_planks", "wood_cherry_planks", NULL);
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_OAK_LOG].asset, FSL_ASSET_CUSTOM,
            "Oak Wood Log", "wood_oak_log", "wood_oak_log", NULL);
    blocks_p[BLOCK_WOOD_OAK_LOG].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks_p[BLOCK_WOOD_OAK_LOG].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_OAK_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Oak Wood Planks", "wood_oak_planks", "wood_oak_planks", NULL);
    blocks_p[BLOCK_WOOD_OAK_PLANKS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].friction = FRICTION_BLOCK_HARD;
}

/* ---- special_blocks ------------------------------------------------------ */

/* ---- items --------------------------------------------------------------- */

/* ---- tools --------------------------------------------------------------- */

