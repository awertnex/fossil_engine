#include "h/assets.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/main.h"

#include "src/h/core.h"
#include "src/h/memory.h"
#include "src/h/shaders.h"
#include "src/h/string.h"

#include <stdio.h>

fsl_shader_program shader[SHADER_COUNT] =
{
    [SHADER_DEFAULT] =
    {
        .name = "default",
        .vertex.file_name = "default.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "default.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [SHADER_SKYBOX] =
    {
        .name = "skybox",
        .vertex.file_name = "skybox.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "skybox.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [SHADER_GIZMO] =
    {
        .name = "gizmo",
        .vertex.file_name = "gizmo.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "gizmo.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [SHADER_GIZMO_CHUNK] =
    {
        .name = "gizmo_chunk",
        .vertex.file_name = "gizmo_chunk.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .geometry.file_name = "gizmo_chunk.geom",
        .geometry.type = GL_GEOMETRY_SHADER,
        .fragment.file_name = "gizmo_chunk.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [SHADER_POST_PROCESSING] =
    {
        .name = "post_processing",
        .vertex.file_name = "post_processing.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "post_processing.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [SHADER_VOXEL] =
    {
        .name = "voxel",
        .vertex.file_name = "voxel.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .geometry.file_name = "voxel.geom",
        .geometry.type = GL_GEOMETRY_SHADER,
        .fragment.file_name = "voxel.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [SHADER_BOUNDING_BOX] =
    {
        .name = "bounding_box",
        .vertex.file_name = "bounding_box.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "bounding_box.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },
};

fsl_texture texture[TEXTURE_COUNT] = {0};
block *blocks = NULL;
static fsl_texture *block_textures = NULL;
static GLuint ssbo_texture_indices_id = 0;
static u32 ssbo_texture_indices[BLOCK_COUNT * 6] = {0};
static GLuint ssbo_texture_handles_id = 0;
static u64 ssbo_texture_handles[TEXTURE_BLOCK_COUNT] = {0};

u32 assets_init(void)
{
    u32 i = 0, j = 0;

    if (
            fsl_mem_map((void*)&block_textures, TEXTURE_BLOCK_COUNT * sizeof(fsl_texture),
                "assets_init().block_textures") != FSL_ERR_SUCCESS ||
            fsl_mem_map((void*)&blocks, BLOCK_COUNT * sizeof(block),
                "assets_init().blocks") != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- shaders --------------------------------------------------------- */

    if (
            fsl_shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_DEFAULT]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_GIZMO]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_GIZMO_CHUNK]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_SKYBOX]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_POST_PROCESSING]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_VOXEL]) != FSL_ERR_SUCCESS ||
            fsl_shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_BOUNDING_BOX]) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- textures -------------------------------------------------------- */

    if (
            fsl_texture_init(&texture[TEXTURE_CROSSHAIR], (v2i32){16, 16},
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                GAME_DIR_NAME_GUI"crosshair.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_ITEM_BAR], (v2i32){256, 256},
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                GAME_DIR_NAME_GUI"item_bar.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_VAL], (v2i32){512, 512},
                GL_RED, GL_RED, GL_NEAREST, FSL_COLOR_CHANNELS_GRAY, FALSE,
                GAME_DIR_NAME_ENV"skybox_val.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_HORIZON], (v2i32){512, 512},
                GL_RED, GL_RED, GL_NEAREST, FSL_COLOR_CHANNELS_GRAY, FALSE,
                GAME_DIR_NAME_ENV"skybox_horizon.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SKYBOX_STARS], (v2i32){512, 512},
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                GAME_DIR_NAME_ENV"skybox_stars.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_SUN], (v2i32){128, 128},
                    GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                    GAME_DIR_NAME_ENV"sun.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture[TEXTURE_MOON], (v2i32){128, 128},
                    GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                    GAME_DIR_NAME_ENV"moon.png") != FSL_ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < TEXTURE_COUNT; ++i)
        if (fsl_texture_generate(&texture[i], FALSE) != FSL_ERR_SUCCESS)
            goto cleanup;

    /* ---- block textures -------------------------------------------------- */

    if (
            block_texture_init(TEXTURE_BLOCK_GRASS_SIDE, (v2i32){16, 16},
                "grass_side.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_GRASS_TOP, (v2i32){16, 16},
                "grass_top.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_DIRT, (v2i32){16, 16},
                "dirt.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_DIRTUP, (v2i32){16, 16},
                "dirtup.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_STONE, (v2i32){16, 16},
                "stone.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_SAND, (v2i32){16, 16},
                "sand.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_GLASS, (v2i32){16, 16},
                "glass.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE, (v2i32){16, 16},
                    "wood_birch_log_side.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP, (v2i32){16, 16},
                    "wood_birch_log_top.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_PLANKS, (v2i32){16, 16},
                    "wood_birch_planks.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE, (v2i32){16, 16},
                    "wood_cherry_log_side.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP, (v2i32){16, 16},
                    "wood_cherry_log_top.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_PLANKS, (v2i32){16, 16},
                    "wood_cherry_planks.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE, (v2i32){16, 16},
                    "wood_oak_log_side.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_LOG_TOP, (v2i32){16, 16},
                    "wood_oak_log_top.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_PLANKS, (v2i32){16, 16},
                    "wood_oak_planks.png") != FSL_ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_BLOOD, (v2i32){16, 16},
                "block_blood.png") != FSL_ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
        ssbo_texture_handles[i] = block_textures[i].handle;

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
    if (block_textures)
        for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
            fsl_texture_free(&block_textures[i]);

    if (texture)
        for (i = 0; i < TEXTURE_COUNT; ++i)
            fsl_texture_free(&texture[i]);

    fsl_mem_unmap((void*)&block_textures, TEXTURE_BLOCK_COUNT * sizeof(fsl_texture),
            "assets_free().block_textures");

    fsl_mem_unmap((void*)&blocks, BLOCK_COUNT * sizeof(block),
            "assets_free().blocks");

    if (ssbo_texture_indices_id)
        glDeleteBuffers(1, &ssbo_texture_indices_id);

    if (ssbo_texture_handles_id)
        glDeleteBuffers(1, &ssbo_texture_handles_id);
}

u32 block_texture_init(u32 index, v2i32 size, str *name)
{
    if (!name)
    {
        HHC_LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Initialize Texture [%p], 'name' NULL\n",
                &block_textures[index]);
        goto cleanup;
    }

    block_textures[index].size = size;

    if (
            fsl_texture_init(&block_textures[index], block_textures[index].size,
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE,
                fsl_stringf("%s%s", DIR_ROOT[DIR_BLOCKS], name)) != FSL_ERR_SUCCESS ||

            fsl_texture_generate(&block_textures[index], TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_texture_free(&block_textures[index]);
    return *GAME_ERR;
}

void blocks_init(void)
{
    snprintf(blocks[BLOCK_NONE].name, NAME_MAX, "%s", "block_none");

    snprintf(blocks[BLOCK_GRASS].name, NAME_MAX, "%s", "block_grass");
    blocks[BLOCK_BLOOD].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_BLOOD].texture_index[0] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[1] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[2] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[3] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[4] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].texture_index[5] = TEXTURE_BLOCK_BLOOD;
    blocks[BLOCK_BLOOD].friction = FRICTION_BLOCK_WET;

    snprintf(blocks[BLOCK_GRASS].name, NAME_MAX, "%s", "block_grass");
    blocks[BLOCK_GRASS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_GRASS].texture_index[0] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[1] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[2] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[3] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks[BLOCK_GRASS].texture_index[4] = TEXTURE_BLOCK_GRASS_TOP;
    blocks[BLOCK_GRASS].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_GRASS].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_DIRT].name, NAME_MAX, "%s", "block_dirt");
    blocks[BLOCK_DIRT].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_DIRT].texture_index[0] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[1] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[2] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[3] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[4] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks[BLOCK_DIRT].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_DIRTUP].name, NAME_MAX, "%s", "block_dirtup");
    blocks[BLOCK_DIRTUP].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_DIRTUP].texture_index[0] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[1] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[2] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[3] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[4] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].texture_index[5] = TEXTURE_BLOCK_DIRTUP;
    blocks[BLOCK_DIRTUP].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_STONE].name, NAME_MAX, "%s", "block_stone");
    blocks[BLOCK_STONE].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_STONE].texture_index[0] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[1] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[2] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[3] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[4] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].texture_index[5] = TEXTURE_BLOCK_STONE;
    blocks[BLOCK_STONE].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_SAND].name, NAME_MAX, "%s", "block_sand");
    blocks[BLOCK_SAND].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_SAND].texture_index[0] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[1] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[2] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[3] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[4] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].texture_index[5] = TEXTURE_BLOCK_SAND;
    blocks[BLOCK_SAND].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_GLASS].name, NAME_MAX, "%s", "block_glass");
    blocks[BLOCK_GLASS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_GLASS].texture_index[0] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[1] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[2] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[3] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[4] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].texture_index[5] = TEXTURE_BLOCK_GLASS;
    blocks[BLOCK_GLASS].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_BIRCH_LOG].name, NAME_MAX, "%s", "block_wood_birch_log");
    blocks[BLOCK_WOOD_BIRCH_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks[BLOCK_WOOD_BIRCH_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks[BLOCK_WOOD_BIRCH_LOG].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_BIRCH_PLANKS].name, NAME_MAX, "%s", "block_wood_birch_planks");
    blocks[BLOCK_WOOD_BIRCH_PLANKS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks[BLOCK_WOOD_BIRCH_PLANKS].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_CHERRY_LOG].name, NAME_MAX, "%s", "block_wood_cherry_log");
    blocks[BLOCK_WOOD_CHERRY_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks[BLOCK_WOOD_CHERRY_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks[BLOCK_WOOD_CHERRY_LOG].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_CHERRY_PLANKS].name, NAME_MAX, "%s", "block_wood_cherry_planks");
    blocks[BLOCK_WOOD_CHERRY_PLANKS].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks[BLOCK_WOOD_CHERRY_PLANKS].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_OAK_LOG].name, NAME_MAX, "%s", "block_wood_oak_log");
    blocks[BLOCK_WOOD_OAK_LOG].state = BLOCK_STATE_SOLID;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks[BLOCK_WOOD_OAK_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks[BLOCK_WOOD_OAK_LOG].friction = FRICTION_BLOCK_HARD;

    snprintf(blocks[BLOCK_WOOD_OAK_PLANKS].name, NAME_MAX, "%s", "block_wood_oak_planks");
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

