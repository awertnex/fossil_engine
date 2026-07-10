#include "deps/fossil/common/config.h"
#include "deps/fossil/common/types.h"
#include "deps/fossil/assets/assets.h"
#include "deps/fossil/engine/engine_assets.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/math/vector.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/physics/transform.h"
#include "deps/fossil/shaders/shaders.h"

#include "h/assets.h"
#include "h/diagnostics.h"
#include "h/main.h"

#include <stdio.h>

fsl_mem_arena memory_arena_assets_internal = {0};
fsl_mem_handle fbo = {0};
hhc_g_buffer g_buf = {0};
hhc_ssao ssao_buf = {0};
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
    fsl_mesh *mesh_p = NULL;
    fsl_shader_program *shader_p = NULL;
    hhc_block *blocks_p = NULL;
    fsl_texture *block_textures_p = NULL;
    fsl_font *font_p = NULL;

    const u32 VBO_LEN_COH = 24;
    const u32 EBO_LEN_COH = 36;

    GLfloat vbo_data_coh[] =
    {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f
    };

    GLuint ebo_data_coh[] =
    {
        0, 4, 5, 5, 1, 0,
        1, 5, 7, 7, 3, 1,
        3, 7, 6, 6, 2, 3,
        2, 6, 4, 4, 0, 2,
        4, 6, 7, 7, 5, 4,
        0, 1, 3, 3, 2, 0
    };

    font_p = fsl_mem_handle_get(fsl_font_buf);
    font[FONT_REG] =        &font_p[FSL_FONT_INDEX_DEJAVU_SANS];
    font[FONT_REG_BOLD] =   &font_p[FSL_FONT_INDEX_DEJAVU_SANS_BOLD];
    font[FONT_MONO] =       &font_p[FSL_FONT_INDEX_DEJAVU_SANS_MONO];
    font[FONT_MONO_BOLD] =  &font_p[FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD];


    if (fsl_mem_arena_init(&memory_arena_assets_internal,
                "assets_init().memory_arena_assets_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_assets_internal, &fbo,
                FBO_COUNT * sizeof(fsl_fbo),
                "assets_init().fbo") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_assets_internal, &texture,
                TEXTURE_COUNT * sizeof(fsl_texture),
                "assets_init().texture") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_assets_internal, &mesh,
                MESH_COUNT * sizeof(fsl_mesh),
                "assets_init().mesh") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_assets_internal, &shader,
                SHADER_COUNT * sizeof(fsl_shader_program),
                "assets_init().shader") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_assets_internal, &blocks,
                BLOCK_COUNT * sizeof(hhc_block),
                "assets_init().blocks") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&memory_arena_assets_internal, &block_textures,
                TEXTURE_BLOCK_COUNT * sizeof(fsl_texture),
                "assets_init().block_textures") != FSL_ERR_SUCCESS)
        goto cleanup;

    fbo_p = fsl_mem_handle_get(fbo);
    texture_p = fsl_mem_handle_get(texture);
    mesh_p = fsl_mem_handle_get(mesh);
    shader_p = fsl_mem_handle_get(shader);
    block_textures_p = fsl_mem_handle_get(block_textures);
    blocks_p = fsl_mem_handle_get(blocks);

    /* ---- framebuffers ---------------------------------------------------- */

    if (
            fsl_fbo_init(&fbo_p[FBO_SKYBOX],
                render->size.x, render->size.y, NULL, FALSE, 0) != FSL_ERR_SUCCESS ||

            fsl_fbo_init(&fbo_p[FBO_WORLD],
                render->size.x, render->size.y, NULL, FALSE, 0) != FSL_ERR_SUCCESS ||

            fsl_fbo_init(&fbo_p[FBO_WORLD_MSAA],
                render->size.x, render->size.y, NULL, TRUE, 4) != FSL_ERR_SUCCESS ||

            fsl_fbo_init(&fbo_p[FBO_HUD],
                render->size.x, render->size.y, NULL, FALSE, 0) != FSL_ERR_SUCCESS ||

            fsl_fbo_init(&fbo_p[FBO_HUD_MSAA],
                render->size.x, render->size.y, NULL, TRUE, 4) != FSL_ERR_SUCCESS ||

            fsl_fbo_init(&fbo_p[FBO_POST_PROCESSING],
                render->size.x, render->size.y, NULL, FALSE, 0) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (g_buffer_init(&g_buf, render->size.x, render->size.y) != FSL_ERR_SUCCESS)
        goto cleanup;

    ssao_init(&ssao_buf);

    /* ---- shaders --------------------------------------------------------- */

    if (fsl_shader_program_init_ex(&shader_p[SHADER_SKYBOX],
                "Skybox", "skybox",
                "skybox.vert", NULL, "skybox.frag",
                GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[SHADER_GIZMO_AXIS],
                "Gizmo Axis", "gizmo_axis",
                "gizmo_axis.vert", NULL, "gizmo_axis.frag",
                GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[SHADER_GIZMO_CHUNK],
                "Gizmo Chunk", "gizmo_chunk",
                "gizmo_chunk.vert", "gizmo_chunk.geom", "gizmo_chunk.frag",
                GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[SHADER_POST_PROCESSING],
                "Post-Processing", "post_processing",
                "post_processing.vert", NULL, "post_processing.frag",
                GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[SHADER_VOXEL],
                "Voxel", "voxel",
                "voxel.vert", "voxel.geom", "voxel.frag",
                GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_shader_program_init_ex(&shader_p[SHADER_BOUNDING_BOX],
                "Bounding Box", "bounding_box",
                "bounding_box.vert", NULL, "bounding_box.frag",
                GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- meshes ---------------------------------------------------------- */

    if (fsl_mesh_generate(&mesh_p[MESH_CUBE_OF_HAPPINESS],
                "Cube of Happiness", "cube_of_happiness", NULL, NULL,
                &fsl_attrib_vec3, GL_STATIC_DRAW,
                VBO_LEN_COH, EBO_LEN_COH, vbo_data_coh, ebo_data_coh) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_mesh_load(&mesh_p[MESH_GIZMO_AXIS], "Gizmo Axis", "gizmo_axis", "gizmo_axis.obj", GAME_DIR_NAME_MODELS) != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- textures -------------------------------------------------------- */

    if (
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
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_BUTTON],
                "Button", "button", "button.png", GAME_DIR_NAME_GUI,
                GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_CROSSHAIR],
                "Crosshair", "crosshair", "crosshair.png", GAME_DIR_NAME_GUI,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_HOTBAR],
                "Hotbar", "hotbar", "hotbar.png", GAME_DIR_NAME_GUI,
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&texture_p[TEXTURE_CONTAINER_INVENTORY_SURVIVAL],
                "Container Inventory Survival", "container_inventory_survival", "container_inventory_survival.png", GAME_DIR_NAME_GUI,
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
        ssbo_texture_handles[i] = block_textures_p[i].bindless_handle;

    blocks_init();

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

    if (block_textures.arena)
    {
        block_textures_p = fsl_mem_handle_get(block_textures);
        for (i = 0; i < TEXTURE_BLOCK_COUNT; ++i)
            fsl_texture_free(&block_textures_p[i]);
    }
    if (shader.arena)
    {
        shader_p = fsl_mem_handle_get(shader);
        for (i = 0; i < SHADER_COUNT; ++i)
            fsl_shader_program_free(&shader_p[i]);
    }
    if (mesh.arena)
    {
        mesh_p = fsl_mem_handle_get(mesh);
        for (i = 0; i < MESH_COUNT; ++i)
            fsl_mesh_free(&mesh_p[i]);
    }
    if (texture.arena)
    {
        texture_p = fsl_mem_handle_get(texture);
        for (i = 0; i < TEXTURE_COUNT; ++i)
            fsl_texture_free(&texture_p[i]);
    }
    if (fbo.arena)
    {
        fbo_p = fsl_mem_handle_get(fbo);
        for (i = 0; i < FBO_COUNT; ++i)
            fsl_fbo_free(&fbo_p[i]);
    }

    g_buffer_free(&g_buf);

    if (ssbo_texture_indices_id)
        glDeleteBuffers(1, &ssbo_texture_indices_id);

    if (ssbo_texture_handles_id)
        glDeleteBuffers(1, &ssbo_texture_handles_id);

    fsl_mem_arena_free(&memory_arena_assets_internal, "assets_free().memory_arena_assets_internal");
}

u32 g_buffer_init(hhc_g_buffer *buf, i32 size_x, i32 size_y)
{
    GLuint status = 0;
    GLuint attachments[3] =
    {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };

    if (!buf->asset.initialized)
    {
        buf->asset.type = FSL_ASSET_FBO;

        glGenFramebuffers(1, &buf->fbo);
        glGenTextures(1, &buf->color_buf_pos);
        glGenTextures(1, &buf->color_buf_normal);
        glGenTextures(1, &buf->color_buf_albedo_specular);
        glGenRenderbuffers(1, &buf->rbo);
        buf->asset.initialized = TRUE;

        glBindFramebuffer(GL_FRAMEBUFFER, buf->fbo);

        /* ---- color buffers ----------------------------------------------- */

        glBindTexture(GL_TEXTURE_2D, buf->color_buf_pos);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                buf->color_buf_pos, 0);

        glBindTexture(GL_TEXTURE_2D, buf->color_buf_normal);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                buf->color_buf_normal, 0);

        glBindTexture(GL_TEXTURE_2D, buf->color_buf_albedo_specular);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                buf->color_buf_albedo_specular, 0);

        glDrawBuffers(3, attachments);

        /* ---- render buffer ----------------------------------------------- */

        glBindRenderbuffer(GL_RENDERBUFFER, buf->rbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buf->rbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, buf->fbo);

    /* ---- color buffers --------------------------------------------------- */

    glBindTexture(GL_TEXTURE_2D, buf->color_buf_pos);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size_x, size_y, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, buf->color_buf_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size_x, size_y, 0, GL_RGB, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, buf->color_buf_albedo_specular);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_x, size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    /* ---- render buffer --------------------------------------------------- */

    glBindRenderbuffer(GL_RENDERBUFFER, buf->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size_x, size_y);

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGFATAL(FSL_ERR_FBO_REALLOC_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Failed to Initialize G-Buffer[%u], Status[%u]\n", buf->fbo, status));
        goto cleanup;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    g_buffer_free(buf);
    return *GAME_ERR;
}

void g_buffer_free(hhc_g_buffer *buf)
{
    hhc_g_buffer nogbuf = {0};

    if (!buf)
        return;

    if (buf->asset.initialized)
    {
        buf->asset.initialized = FALSE;
        glDeleteTextures(1, &buf->color_buf_pos);
        glDeleteTextures(1, &buf->color_buf_normal);
        glDeleteTextures(1, &buf->color_buf_albedo_specular);
    }

    *buf = nogbuf;
}

void ssao_init(hhc_ssao *ssao)
{
    u32 kernel_size = 64;
    f32 scale = 0.0f;
    f32 rand_scale = 1.0f / FSL_U32_MAX;
    static u32 seed = 0;
    u32 i = 0;

    for (; i < kernel_size; ++i)
    {
        scale = (f32)i / kernel_size;
        scale = fsl_lerp_f32(0.1f, 1.0f, scale * scale);

        ssao->sample[i].x = (f32)fsl_rand_u32((u32)render->time + seed++) * rand_scale * 2.0f - 1.0f;
        ssao->sample[i].y = (f32)fsl_rand_u32((u32)render->time + seed++) * rand_scale * 2.0f - 1.0f;
        ssao->sample[i].z = (f32)fsl_rand_u32((u32)render->time + seed++) * rand_scale;
        ssao->sample[i] = fsl_normalize_v3f32(ssao->sample[i]);

        ssao->sample[i].x *= (f32)fsl_rand_u32((u32)render->time + seed++) * rand_scale;
        ssao->sample[i].y *= (f32)fsl_rand_u32((u32)render->time + seed++) * rand_scale;
        ssao->sample[i].z *= (f32)fsl_rand_u32((u32)render->time + seed++) * rand_scale;
        ssao->sample[i].x *= scale;
        ssao->sample[i].y *= scale;
        ssao->sample[i].z *= scale;
    }
}

u32 block_texture_init(u32 index, const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file)
{
    fsl_texture *block_texture = fsl_mem_handle_get(block_textures);

    if (!name_id)
    {
        LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Failed to Initialize Texture [%u], `name_id` `NULL`\n", index));
        goto cleanup;
    }

    if (fsl_texture_init(&block_texture[index], name, name_id, file, GAME_DIR_NAME_BLOCKS,
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
    hhc_block *blocks_p = fsl_mem_handle_get(blocks);

    fsl_asset_set_metadata(&blocks_p[BLOCK_NONE].asset, FSL_ASSET_CUSTOM,
            "None", "none", NULL, NULL);

    fsl_asset_set_metadata(&blocks_p[BLOCK_GRASS].asset, FSL_ASSET_CUSTOM,
            "Grass Block", "block_grass", "block_grass", NULL);
    blocks_p[BLOCK_GRASS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_GRASS].texture_index[0] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks_p[BLOCK_GRASS].texture_index[1] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks_p[BLOCK_GRASS].texture_index[2] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks_p[BLOCK_GRASS].texture_index[3] = TEXTURE_BLOCK_GRASS_SIDE;
    blocks_p[BLOCK_GRASS].texture_index[4] = TEXTURE_BLOCK_GRASS_TOP;
    blocks_p[BLOCK_GRASS].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_GRASS].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_DIRT].asset, FSL_ASSET_CUSTOM,
            "Dirt Block", "block_dirt", "block_dirt", NULL);
    blocks_p[BLOCK_DIRT].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_DIRT].texture_index[0] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[1] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[2] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[3] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[4] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].texture_index[5] = TEXTURE_BLOCK_DIRT;
    blocks_p[BLOCK_DIRT].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_DIRTUP].asset, FSL_ASSET_CUSTOM,
            "Dirtup", "block_dirtup", "block_dirtup", NULL);
    blocks_p[BLOCK_DIRTUP].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_DIRTUP].texture_index[0] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[1] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[2] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[3] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[4] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].texture_index[5] = TEXTURE_BLOCK_DIRTUP;
    blocks_p[BLOCK_DIRTUP].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_STONE].asset, FSL_ASSET_CUSTOM,
            "Stone", "block_stone", "block_stone", NULL);
    blocks_p[BLOCK_STONE].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_STONE].texture_index[0] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[1] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[2] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[3] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[4] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].texture_index[5] = TEXTURE_BLOCK_STONE;
    blocks_p[BLOCK_STONE].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_SAND].asset, FSL_ASSET_CUSTOM,
            "Sand", "block_sand", "block_sand", NULL);
    blocks_p[BLOCK_SAND].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_SAND].texture_index[0] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[1] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[2] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[3] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[4] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].texture_index[5] = TEXTURE_BLOCK_SAND;
    blocks_p[BLOCK_SAND].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_GLASS].asset, FSL_ASSET_CUSTOM,
            "Glass", "block_glass", "block_glass", NULL);
    blocks_p[BLOCK_GLASS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_GLASS].texture_index[0] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[1] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[2] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[3] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[4] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].texture_index[5] = TEXTURE_BLOCK_GLASS;
    blocks_p[BLOCK_GLASS].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_BIRCH_LOG].asset, FSL_ASSET_CUSTOM,
            "Birch Wood Log", "wood_birch_log", "wood_birch_log", NULL);
    blocks_p[BLOCK_WOOD_BIRCH_LOG].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP;
    blocks_p[BLOCK_WOOD_BIRCH_LOG].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_BIRCH_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Birch Wood Planks", "wood_birch_planks", "wood_birch_planks", NULL);
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_BIRCH_PLANKS;
    blocks_p[BLOCK_WOOD_BIRCH_PLANKS].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_CHERRY_LOG].asset, FSL_ASSET_CUSTOM,
            "Cherry Wood Log", "wood_cherry_log", "wood_cherry_log", NULL);
    blocks_p[BLOCK_WOOD_CHERRY_LOG].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP;
    blocks_p[BLOCK_WOOD_CHERRY_LOG].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_CHERRY_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Cherry Wood Planks", "wood_cherry_planks", "wood_cherry_planks", NULL);
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_CHERRY_PLANKS;
    blocks_p[BLOCK_WOOD_CHERRY_PLANKS].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_OAK_LOG].asset, FSL_ASSET_CUSTOM,
            "Oak Wood Log", "wood_oak_log", "wood_oak_log", NULL);
    blocks_p[BLOCK_WOOD_OAK_LOG].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks_p[BLOCK_WOOD_OAK_LOG].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_LOG_TOP;
    blocks_p[BLOCK_WOOD_OAK_LOG].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_WOOD_OAK_PLANKS].asset, FSL_ASSET_CUSTOM,
            "Oak Wood Planks", "wood_oak_planks", "wood_oak_planks", NULL);
    blocks_p[BLOCK_WOOD_OAK_PLANKS].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[0] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[1] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[2] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[3] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[4] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].texture_index[5] = TEXTURE_BLOCK_WOOD_OAK_PLANKS;
    blocks_p[BLOCK_WOOD_OAK_PLANKS].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD, FRICTION_BLOCK_HARD,
                0.0, 0.0, 0.0, 0.0);

    fsl_asset_set_metadata(&blocks_p[BLOCK_BLOOD].asset, FSL_ASSET_CUSTOM,
            "Blood Block", "block_blood", "block_blood", NULL);
    blocks_p[BLOCK_BLOOD].state = BLOCK_STATE_SOLID;
    blocks_p[BLOCK_BLOOD].texture_index[0] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[1] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[2] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[3] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[4] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].texture_index[5] = TEXTURE_BLOCK_BLOOD;
    blocks_p[BLOCK_BLOOD].physics_material =
        fsl_physics_material_init(FRICTION_BLOCK_WET, FRICTION_BLOCK_WET, FRICTION_BLOCK_WET,
                0.0, 0.0, 0.0, 0.0);
}

/* ---- special_blocks ------------------------------------------------------ */

/* ---- items --------------------------------------------------------------- */

/* ---- tools --------------------------------------------------------------- */

