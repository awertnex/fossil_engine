#include <engine/h/core.h>
#include <engine/h/math.h>
#include <engine/h/memory.h>
#include <engine/h/logger.h>
#include <engine/h/string.h>

#include "h/assets.h"
#include "h/dir.h"
#include "h/main.h"

ShaderProgram shader[SHADER_COUNT] =
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

Texture texture[TEXTURE_COUNT] = {0};
Block *blocks = NULL;
static Texture *block_textures = NULL;
static GLuint ssbo_texture_indices_id = 0;
static u32 ssbo_texture_indices[BLOCK_COUNT * 6] = {0};
static GLuint ssbo_texture_handles_id = 0;
static u64 ssbo_texture_handles[TEXTURE_BLOCK_COUNT] = {0};

u32 assets_init(void)
{
    u32 i = 0, j = 0;

    if (
            mem_map((void*)&block_textures, TEXTURE_BLOCK_COUNT * sizeof(Texture),
                "assets_init().block_textures") != ERR_SUCCESS ||
            mem_map((void*)&blocks, BLOCK_COUNT * sizeof(Block),
                "assets_init().blocks") != ERR_SUCCESS)
        goto cleanup;

    /* ---- shaders --------------------------------------------------------- */

    if (
            shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_DEFAULT]) != ERR_SUCCESS ||
            shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_GIZMO]) != ERR_SUCCESS ||
            shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_GIZMO_CHUNK]) != ERR_SUCCESS ||
            shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_SKYBOX]) != ERR_SUCCESS ||
            shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_POST_PROCESSING]) != ERR_SUCCESS ||
            shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_VOXEL]) != ERR_SUCCESS ||
            shader_program_init(GAME_DIR_NAME_SHADERS, &shader[SHADER_BOUNDING_BOX]) != ERR_SUCCESS)
        goto cleanup;

    /* ---- textures -------------------------------------------------------- */

    if (
            texture_init(&texture[TEXTURE_CROSSHAIR], (v2i32){16, 16},
                GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                GAME_DIR_NAME_GUI"crosshair.png") != ERR_SUCCESS ||

            texture_init(&texture[TEXTURE_ITEM_BAR], (v2i32){256, 256},
                GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                GAME_DIR_NAME_GUI"item_bar.png") != ERR_SUCCESS ||

            texture_init(&texture[TEXTURE_SKYBOX_VAL], (v2i32){512, 512},
                GL_RED, GL_RED, GL_NEAREST, 1, FALSE,
                GAME_DIR_NAME_ENV"skybox_val.png") != ERR_SUCCESS ||

            texture_init(&texture[TEXTURE_SKYBOX_HORIZON], (v2i32){512, 512},
                GL_RED, GL_RED, GL_NEAREST, 1, FALSE,
                GAME_DIR_NAME_ENV"skybox_horizon.png") != ERR_SUCCESS ||

            texture_init(&texture[TEXTURE_SKYBOX_STARS], (v2i32){512, 512},
                GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                GAME_DIR_NAME_ENV"skybox_stars.png") != ERR_SUCCESS ||

            texture_init(&texture[TEXTURE_SUN], (v2i32){128, 128},
                    GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                    GAME_DIR_NAME_ENV"sun.png") != ERR_SUCCESS ||

            texture_init(&texture[TEXTURE_MOON], (v2i32){128, 128},
                    GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                    GAME_DIR_NAME_ENV"moon.png") != ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < TEXTURE_COUNT; ++i)
        if (texture_generate(&texture[i], FALSE) != ERR_SUCCESS)
            goto cleanup;

    /* ---- block textures -------------------------------------------------- */

    if (
            block_texture_init(TEXTURE_BLOCK_GRASS_SIDE, (v2i32){16, 16},
                "grass_side.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_GRASS_TOP, (v2i32){16, 16},
                "grass_top.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_DIRT, (v2i32){16, 16},
                "dirt.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_DIRTUP, (v2i32){16, 16},
                "dirtup.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_STONE, (v2i32){16, 16},
                "stone.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_SAND, (v2i32){16, 16},
                "sand.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_GLASS, (v2i32){16, 16},
                "glass.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE, (v2i32){16, 16},
                    "wood_birch_log_side.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP, (v2i32){16, 16},
                    "wood_birch_log_top.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_BIRCH_PLANKS, (v2i32){16, 16},
                    "wood_birch_planks.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE, (v2i32){16, 16},
                    "wood_cherry_log_side.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP, (v2i32){16, 16},
                    "wood_cherry_log_top.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_CHERRY_PLANKS, (v2i32){16, 16},
                    "wood_cherry_planks.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE, (v2i32){16, 16},
                    "wood_oak_log_side.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_LOG_TOP, (v2i32){16, 16},
                    "wood_oak_log_top.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_WOOD_OAK_PLANKS, (v2i32){16, 16},
                    "wood_oak_planks.png") != ERR_SUCCESS ||

            block_texture_init(TEXTURE_BLOCK_BLOOD, (v2i32){16, 16},
                "block_blood.png") != ERR_SUCCESS)
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

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    assets_free();
    return *GAME_ERR;
}

void assets_free(void)
{
    u32 i = 0;
    for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
        texture_free(&block_textures[i]);

    mem_unmap((void*)&block_textures, TEXTURE_BLOCK_COUNT * sizeof(Texture),
            "assets_free().block_textures");

    mem_unmap((void*)&blocks, BLOCK_COUNT * sizeof(Block),
            "assets_free().blocks");

    for (i = 0; i < TEXTURE_COUNT; ++i)
        texture_free(&texture[i]);

    if (ssbo_texture_indices_id)
        glDeleteBuffers(1, &ssbo_texture_indices_id);

    if (ssbo_texture_handles_id)
        glDeleteBuffers(1, &ssbo_texture_handles_id);
}

u32 block_texture_init(u32 index, v2i32 size, str *name)
{
    if (!name)
    {
        LOGERROR(FALSE, ERR_POINTER_NULL,
                "Failed to Initialize Texture [%p], 'name' NULL\n",
                &block_textures[index]);
        goto cleanup;
    }

    block_textures[index].size = size;

    if (
            texture_init(&block_textures[index], block_textures[index].size,
                GL_RGBA, GL_RGBA, GL_NEAREST, 4, FALSE,
                stringf("%s%s", DIR_ROOT[DIR_BLOCKS], name)) != ERR_SUCCESS ||

            texture_generate(&block_textures[index], TRUE) != ERR_SUCCESS)
        goto cleanup;

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    texture_free(&block_textures[index]);
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

