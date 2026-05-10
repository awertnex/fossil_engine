#include "src/h/core.h"
#include "src/logger/log.h"
#include "src/h/memory.h"
#include "src/h/shaders.h"
#include "src/h/string.h"

#include "h/assets.h"
#include "h/dir.h"
#include "h/main.h"

#include <stdio.h>

fsl_off fbo_off = FSL_OFFSET_INVALID;
fsl_off texture_off = FSL_OFFSET_INVALID;
fsl_off mesh_off = FSL_OFFSET_INVALID;
fsl_off shader_off = FSL_OFFSET_INVALID;
fsl_off block_textures_off = FSL_OFFSET_INVALID;
fsl_off blocks_off = FSL_OFFSET_INVALID;

fsl_fbo *fbo = NULL;
fsl_texture *texture = NULL;
fsl_mesh *mesh = NULL;
fsl_shader_program *shader = NULL;
block *blocks = NULL;
fsl_texture *block_textures = NULL;
static GLuint ssbo_texture_indices_id = 0;
static u32 ssbo_texture_indices[BLOCK_COUNT * 6] = {0};
static GLuint ssbo_texture_handles_id = 0;
static u64 ssbo_texture_handles[TEXTURE_BLOCK_COUNT] = {0};
fsl_font *font[FONT_COUNT] = {0};

u32 assets_init(void)
{
    u32 i = 0, j = 0;

    font[FONT_REG] =        &fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS];
    font[FONT_REG_BOLD] =   &fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_BOLD];
    font[FONT_MONO] =       &fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_MONO];
    font[FONT_MONO_BOLD] =  &fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD];

    if (
            fsl_mem_push_arena(&_memory_arena_internal, &fbo_off,
                FBO_COUNT * sizeof(fsl_fbo),
                "assets_init().fbo_off") != FSL_ERR_SUCCESS ||

            fsl_mem_push_arena(&_memory_arena_internal, &texture_off,
                TEXTURE_COUNT * sizeof(fsl_texture),
                "assets_init().texture_off") != FSL_ERR_SUCCESS ||

            fsl_mem_push_arena(&_memory_arena_internal, &mesh_off,
                MESH_COUNT * sizeof(fsl_mesh),
                "assets_init().mesh_off") != FSL_ERR_SUCCESS ||

            fsl_mem_push_arena(&_memory_arena_internal, &shader_off,
                SHADER_COUNT * sizeof(fsl_shader_program),
                "assets_init().shader_off") != FSL_ERR_SUCCESS ||

            fsl_mem_push_arena(&_memory_arena_internal, &block_textures_off,
                TEXTURE_BLOCK_COUNT * sizeof(fsl_texture),
                "assets_init().block_textures_off") != FSL_ERR_SUCCESS ||

            fsl_mem_push_arena(&_memory_arena_internal, &blocks_off,
                BLOCK_COUNT * sizeof(block),
                "assets_init().blocks_off") != FSL_ERR_SUCCESS)
        goto cleanup;

    fbo = (fsl_fbo*)fsl_mem_arena_get_offset(&_memory_arena_internal, fbo_off);
    texture = (fsl_texture*)fsl_mem_arena_get_offset(&_memory_arena_internal, texture_off);
    mesh = (fsl_mesh*)fsl_mem_arena_get_offset(&_memory_arena_internal, mesh_off);
    shader = (fsl_shader_program*)fsl_mem_arena_get_offset(&_memory_arena_internal, shader_off);
    block_textures = (fsl_texture*)fsl_mem_arena_get_offset(&_memory_arena_internal, block_textures_off);
    blocks = (block*)fsl_mem_arena_get_offset(&_memory_arena_internal, blocks_off);

    /* ---- framebuffers ---------------------------------------------------- */

    if (
            fsl_fbo_init(&fbo[FBO_SKYBOX],      NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo[FBO_WORLD],       NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo[FBO_WORLD_MSAA],  NULL, TRUE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo[FBO_HUD],         NULL, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo[FBO_HUD_MSAA],    NULL, TRUE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&fbo[FBO_POST_PROCESSING], NULL, FALSE, 4) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- shaders --------------------------------------------------------- */

    if (
            fsl_asset_set_metadata(&shader[SHADER_DEFAULT].asset, FSL_ASSET_SHADER_PROGRAM, "Default", "default", NULL, GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_DEFAULT].vertex.asset, FSL_ASSET_SHADER, "Default", "default", "default.vert", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_DEFAULT].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_DEFAULT].fragment.asset, FSL_ASSET_SHADER, "Default", "default", "default.frag", GAME_DIR_NAME_SHADERS) == NULL ||

            fsl_asset_set_metadata(&shader[SHADER_SKYBOX].asset, FSL_ASSET_SHADER_PROGRAM, "Skybox", "skybox", NULL, GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_SKYBOX].vertex.asset, FSL_ASSET_SHADER, "Skybox", "skybox", "skybox.vert", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_SKYBOX].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_SKYBOX].fragment.asset, FSL_ASSET_SHADER, "Skybox", "skybox", "skybox.frag", GAME_DIR_NAME_SHADERS) == NULL ||

            fsl_asset_set_metadata(&shader[SHADER_GIZMO].asset, FSL_ASSET_SHADER_PROGRAM, "Gizmo", "gizmo", NULL, GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_GIZMO].vertex.asset, FSL_ASSET_SHADER, "Gizmo", "gizmo", "gizmo.vert", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_GIZMO].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_GIZMO].fragment.asset, FSL_ASSET_SHADER, "Gizmo", "gizmo", "gizmo.frag", GAME_DIR_NAME_SHADERS) == NULL ||

            fsl_asset_set_metadata(&shader[SHADER_GIZMO_CHUNK].asset, FSL_ASSET_SHADER_PROGRAM, "Gizmo Chunk", "gizmo_chunk", NULL, GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_GIZMO_CHUNK].vertex.asset, FSL_ASSET_SHADER, "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.vert", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_GIZMO_CHUNK].geometry.asset, FSL_ASSET_SHADER, "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.geom", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_GIZMO_CHUNK].fragment.asset, FSL_ASSET_SHADER, "Gizmo Chunk", "gizmo_chunk", "gizmo_chunk.frag", GAME_DIR_NAME_SHADERS) == NULL ||

            fsl_asset_set_metadata(&shader[SHADER_POST_PROCESSING].asset, FSL_ASSET_SHADER_PROGRAM, "Post Processing", "post_processing", NULL, GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_POST_PROCESSING].vertex.asset, FSL_ASSET_SHADER, "Post Processing", "post_processing", "post_processing.vert", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_POST_PROCESSING].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_POST_PROCESSING].fragment.asset, FSL_ASSET_SHADER, "Post Processing", "post_processing", "post_processing.frag", GAME_DIR_NAME_SHADERS) == NULL ||

            fsl_asset_set_metadata(&shader[SHADER_VOXEL].asset, FSL_ASSET_SHADER_PROGRAM, "Voxel", "voxel", NULL, GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_VOXEL].vertex.asset, FSL_ASSET_SHADER, "Voxel", "voxel", "voxel.vert", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_VOXEL].geometry.asset, FSL_ASSET_SHADER, "Voxel", "voxel", "voxel.geom", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_VOXEL].fragment.asset, FSL_ASSET_SHADER, "Voxel", "voxel", "voxel.frag", GAME_DIR_NAME_SHADERS) == NULL ||

            fsl_asset_set_metadata(&shader[SHADER_BOUNDING_BOX].asset, FSL_ASSET_SHADER_PROGRAM, "Bounding Box", "bounding_box", NULL, GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_BOUNDING_BOX].vertex.asset, FSL_ASSET_SHADER, "Bounding Box", "bounding_box", "bounding_box.vert", GAME_DIR_NAME_SHADERS) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_BOUNDING_BOX].geometry.asset, FSL_ASSET_SHADER, NULL, "NULL", NULL, NULL) == NULL ||
            fsl_asset_set_metadata(&shader[SHADER_BOUNDING_BOX].fragment.asset, FSL_ASSET_SHADER, "Bounding Box", "bounding_box", "bounding_box.frag", GAME_DIR_NAME_SHADERS) == NULL)
            goto cleanup;

    if (
            fsl_shader_program_init(&shader[SHADER_DEFAULT]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader[SHADER_GIZMO]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader[SHADER_GIZMO_CHUNK]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader[SHADER_SKYBOX]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader[SHADER_POST_PROCESSING]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader[SHADER_VOXEL]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(&shader[SHADER_BOUNDING_BOX]) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- textures -------------------------------------------------------- */

    if (
            fsl_texture_init(&texture[TEXTURE_CROSSHAIR],
                "Crosshair", "crosshair", "crosshair.png", GAME_DIR_NAME_GUI,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_ITEM_BAR],
                "Item Bar", "item_bar", "item_bar.png", GAME_DIR_NAME_GUI,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_VAL],
                "Skybox Val", "skybox_val", "skybox_val.png", GAME_DIR_NAME_ENV,
                GL_RED, GL_NEAREST, FSL_COLOR_CHANNELS_GRAY, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_HORIZON],
                "Skybox Horizon", "skybox_horizon", "skybox_horizon.png", GAME_DIR_NAME_ENV,
                GL_RED, GL_NEAREST, FSL_COLOR_CHANNELS_GRAY, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_STARS],
                "Skybox Stars", "skybox_stars", "skybox_stars.png", GAME_DIR_NAME_ENV,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SUN],
                "Sun", "sun", "sun.png", GAME_DIR_NAME_ENV,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_MOON],
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

    for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
        ssbo_texture_handles[i] = block_textures[i].bindless_handle;

    blocks_init();

    for (i = 0, j = 0; i < BLOCK_COUNT; ++i)
    {
        ssbo_texture_indices[j++] = blocks[i].texture_index[0];
        ssbo_texture_indices[j++] = blocks[i].texture_index[1];
        ssbo_texture_indices[j++] = blocks[i].texture_index[2];
        ssbo_texture_indices[j++] = blocks[i].texture_index[3];
        ssbo_texture_indices[j++] = blocks[i].texture_index[4];
        ssbo_texture_indices[j++] = blocks[i].texture_index[5];
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

    if (fbo)
        for (i = 0; i < FBO_COUNT; ++i)
            fsl_fbo_free(&fbo[i]);
    if (texture)
        for (i = 0; i < TEXTURE_COUNT; ++i)
            fsl_texture_free(&texture[i]);
    if (block_textures)
        for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
            fsl_texture_free(&block_textures[i]);
    if (mesh)
        for (i = 0; i < MESH_COUNT; ++i)
            fsl_mesh_free(&mesh[i]);
    if (shader)
        for (i = 0; i < SHADER_COUNT; ++i)
            fsl_shader_program_free(&shader[i]);

    if (ssbo_texture_indices_id)
        glDeleteBuffers(1, &ssbo_texture_indices_id);

    if (ssbo_texture_handles_id)
        glDeleteBuffers(1, &ssbo_texture_handles_id);
}

u32 block_texture_init(u32 index, const str *name, const str *name_id, const str *file)
{
    if (!name_id)
    {
        LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Failed to Initialize Texture [%p], `name_id` `NULL`\n",
                &block_textures[index]));
        goto cleanup;
    }

    if (fsl_texture_init(&block_textures[index], name, name_id, file, GAME_DIR_NAME_BLOCKS,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_texture_free(&block_textures[index]);
    return *GAME_ERR;
}

void blocks_init(void)
{
    fsl_asset_set_metadata(&blocks[BLOCK_NONE].asset, FSL_ASSET_CUSTOM,
            "None", "none", NULL, NULL);

    fsl_asset_set_metadata(&blocks[BLOCK_BLOOD].asset, FSL_ASSET_CUSTOM,
            "Blood Block", "block_blood", "block_blood", NULL);
    blocks[BLOCK_BLOOD].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_BLOOD].texture_index[0] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[1] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[2] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[3] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[4] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[5] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].friction = FRICTION_BLOCK_WET;

    fsl_asset_set_metadata(&blocks[BLOCK_GRASS].asset, FSL_ASSET_CUSTOM,
            "Grass Block", "block_grass", "block_grass", NULL);
    blocks[BLOCK_GRASS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_GRASS].texture_index[0] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[1] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[2] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[3] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[4] = TEXTURE_BLOCK_GRASS_TOP;
    blocks[BLOCK_GRASS].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_GRASS].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_DIRT].asset, FSL_ASSET_CUSTOM,
            "Dirt Block", "block_dirt", "block_dirt", NULL);
    blocks[BLOCK_DIRT].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_DIRT].texture_index[0] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[1] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[2] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[3] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[4] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_DIRTUP].asset, FSL_ASSET_CUSTOM,
            "Dirtup", "block_dirtup", "block_dirtup", NULL);
    blocks[BLOCK_DIRTUP].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_DIRTUP].texture_index[0] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[1] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[2] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[3] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[4] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[5] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_STONE].asset, FSL_ASSET_CUSTOM,
            "Stone", "block_stone", "block_stone", NULL);
    blocks[BLOCK_STONE].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_STONE].texture_index[0] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[1] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[2] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[3] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[4] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[5] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_SAND].asset, FSL_ASSET_CUSTOM,
            "Sand", "block_sand", "block_sand", NULL);
    blocks[BLOCK_SAND].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_SAND].texture_index[0] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[1] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[2] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[3] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[4] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[5] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_GLASS].asset, FSL_ASSET_CUSTOM,
            "Glass", "block_glass", "block_glass", NULL);
    blocks[BLOCK_GLASS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_GLASS].texture_index[0] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[1] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[2] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[3] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[4] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[5] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_WOOD_BIRCH_LOG].asset, FSL_ASSET_CUSTOM,
            "Birch Wood Log", "wood_birch_log", "wood_birch_log", NULL);
    blocks[BLOCK_WOOD_BIRCH_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks[BLOCK_WOOD_BIRCH_LOG].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_WOOD_BIRCH_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Birch Wood Planks", "wood_birch_planks", "wood_birch_planks", NULL);
    blocks[BLOCK_WOOD_BIRCH_PLANKS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_WOOD_CHERRY_LOG].asset, FSL_ASSET_CUSTOM,
            "Cherry Wood Log", "wood_cherry_log", "wood_cherry_log", NULL);
    blocks[BLOCK_WOOD_CHERRY_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks[BLOCK_WOOD_CHERRY_LOG].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_WOOD_CHERRY_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Cherry Wood Planks", "wood_cherry_planks", "wood_cherry_planks", NULL);
    blocks[BLOCK_WOOD_CHERRY_PLANKS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_WOOD_OAK_LOG].asset, FSL_ASSET_CUSTOM,
            "Oak Wood Log", "wood_oak_log", "wood_oak_log", NULL);
    blocks[BLOCK_WOOD_OAK_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks[BLOCK_WOOD_OAK_LOG].friction = FRICTION_BLOCK_HARD;

    fsl_asset_set_metadata(&blocks[BLOCK_WOOD_OAK_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Oak Wood Planks", "wood_oak_planks", "wood_oak_planks", NULL);
    blocks[BLOCK_WOOD_OAK_PLANKS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks[BLOCK_WOOD_OAK_PLANKS].friction = FRICTION_BLOCK_HARD;
}

/* ---- special_blocks ------------------------------------------------------ */

/* ---- items --------------------------------------------------------------- */

/* ---- tools --------------------------------------------------------------- */

