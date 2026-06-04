#include "deps/fossil/fossil_engine.h"

#include "h/main.h"

#include "chunking/chunking.h"
#include "chunking/chunking_debug_tools.h"

#include "h/assets.h"
#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/input.h"
#include "h/player.h"
#include "h/terrain.h"
#include "h/world.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

i32 scrool = 0;
u32 *const GAME_ERR = (u32*)&fsl_err;
fsl_mem_arena memory_arena_internal = {0};
fsl_render *render = NULL;
struct hhc_core core = {0};
struct hhc_settings settings = {0};
struct hhc_uniform uniform = {0};
static hhc_player player = {0};

static struct /* skybox_data */
{
    f32 time;
    v3f32 sun_rotation;
    v3f32 sky_color;
    v3f32 horizon_color;
    v3f32 sky_light;
    v3f32 moon_light;
} skybox_data = {0};

static struct /* refresh_interval */
{
    u64 fps_string;
} refresh_interval = {0};

static void callback_framebuffer_size(i32 size_x, i32 size_y);
static void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods);
static void callback_scroll(GLFWwindow *window, double xoffset, double yoffset);

static void bind_shader_uniforms(void);

/*!
 *  @internal
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
static u32 settings_init(void);

void settings_update(void);
static void draw_gizmo(void);
static void draw_hotbar_items(void);
static void draw_everything(void);

static void callback_framebuffer_size(i32 size_x, i32 size_y)
{
    fsl_fbo *fbo_p = fsl_mem_handle_get(fbo);

    player.camera.ratio = (f32)size_x / (f32)size_y;
    player.camera_hud.ratio = (f32)size_x / (f32)size_y;

    fsl_fbo_realloc(&fbo_p[FBO_SKYBOX], render->size.x, render->size.y, FALSE, 4);
    fsl_fbo_realloc(&fbo_p[FBO_WORLD], render->size.x, render->size.y, FALSE, 4);
    fsl_fbo_realloc(&fbo_p[FBO_WORLD_MSAA], render->size.x, render->size.y, TRUE, 4);
    fsl_fbo_realloc(&fbo_p[FBO_HUD], render->size.x, render->size.y, FALSE, 4);
    fsl_fbo_realloc(&fbo_p[FBO_HUD_MSAA], render->size.x, render->size.y, TRUE, 4);
    fsl_fbo_realloc(&fbo_p[FBO_POST_PROCESSING], render->size.x, render->size.y, FALSE, 4);
}

static void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        fsl_request_engine_close();
}

static void callback_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;

    if (player.flag & FLAG_PLAYER_ZOOMER)
        player.camera.zoom =
            fsl_clamp_f64(player.camera.zoom + yoffset * FSL_CAMERA_ZOOM_SPEED, 0.0f, FSL_CAMERA_ZOOM_MAX);
    else
    {
        player.hotbar_slot_selected -= (i64)yoffset;
        if (player.hotbar_slot_selected >= PLAYER_HOTBAR_SLOTS_MAX)
            player.hotbar_slot_selected = 0;
        else if (player.hotbar_slot_selected < 0)
            player.hotbar_slot_selected = PLAYER_HOTBAR_SLOTS_MAX - 1;
    }

    if (core.flag.super_debug)
        scrool = fsl_clamp_i32(scrool + (i32)yoffset * SET_CONSOLE_SCROLL_SPEED, 0, logger_core.cursor);
}

static u32 settings_init(void)
{
    str tokens[4][24] =
    {
        "mouse_sensitivity",
        "field_of_view",
        "render_distance",
        "target_fps",
    };
    str *settings_file_contents = fsl_stringf(
            "%s = %d\n"
            "%s = %d\n"
            "%s = %d\n"
            "%s = %d\n",
            tokens[0], SET_MOUSE_SENSITIVITY_DEFAULT,
            tokens[1], SET_FOV_DEFAULT,
            tokens[2], SET_RENDER_DISTANCE_DEFAULT,
            tokens[3], FSL_TARGET_FPS_DEFAULT);

    if (fsl_is_dir_exists(GAME_DIR_NAME_CONFIG, TRUE) != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    if (fsl_is_file_exists(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS, FALSE) != FSL_ERR_SUCCESS)
    {
        fsl_write_file(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS,
                strlen(settings_file_contents),
                settings_file_contents, TRUE, TRUE);
    }

    settings_file_contents = NULL;
    fsl_get_file_contents(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS,
            (void*)&settings_file_contents, TRUE);
    if (*GAME_ERR != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    settings.lerp_speed = SET_LERP_SPEED_DEFAULT;

    settings.render_distance = 16;
    settings.chunk_buf_radius = settings.render_distance;
    settings.chunk_buf_diameter = settings.chunk_buf_radius * 2 + 1;

    settings.chunk_buf_layer =
        settings.chunk_buf_diameter * settings.chunk_buf_diameter;

    settings.chunk_buf_volume =
        settings.chunk_buf_diameter *
        settings.chunk_buf_diameter *
        settings.chunk_buf_diameter;

    settings.chunk_tab_center =
        settings.chunk_buf_radius +
        settings.chunk_buf_radius * settings.chunk_buf_diameter +
        settings.chunk_buf_radius * settings.chunk_buf_layer;

    settings.reach_distance = PLAYER_REACH_DISTANCE_MAX;
    settings.mouse_sensitivity = SET_MOUSE_SENSITIVITY_DEFAULT * 0.004f;
    settings.gui_scale = SET_GUI_SCALE_DEFAULT;
    settings.font_size = 20.0f;
    settings.target_fps = 0;
    settings.fov = SET_FOV_DEFAULT;
    settings.anti_aliasing = TRUE;

    fsl_mem_free((void*)&settings_file_contents, strlen(settings_file_contents),
            "settings_init().settings_file_contents");

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

void settings_update(void)
{
    if (fsl_on_time_interval(&refresh_interval.fps_string,
                FSL_SEC2NSEC / SET_TEXT_REFRESH_INTERVAL, render->time))
        settings.fps = 1 / ((f64)render->time_delta * FSL_NSEC2SEC);
}

static void bind_shader_uniforms(void)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    uniform.skybox.mat_translation =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "mat_translation");
    uniform.skybox.mat_rotation =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "mat_rotation");
    uniform.skybox.mat_sun_rotation =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "mat_sun_rotation");
    uniform.skybox.mat_orientation =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "mat_orientation");
    uniform.skybox.mat_projection =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "mat_projection");
    uniform.skybox.texture_sky =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "texture_sky");
    uniform.skybox.texture_horizon =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "texture_horizon");
    uniform.skybox.texture_stars =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "texture_stars");
    uniform.skybox.texture_sun =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "texture_sun");
    uniform.skybox.sun_rotation =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "sun_rotation");
    uniform.skybox.sky_color =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "sky_color");
    uniform.skybox.horizon_color =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "horizon_color");
    uniform.skybox.render_layer =
        glGetUniformLocation(shader_p[SHADER_SKYBOX].asset.id, "render_layer");

    uniform.gizmo.color =
        glGetUniformLocation(shader_p[SHADER_GIZMO].asset.id, "gizmo_color");

    uniform.gizmo_chunk.gizmo_offset =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "gizmo_offset");
    uniform.gizmo_chunk.render_size =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "render_size");
    uniform.gizmo_chunk.chunk_buf_diameter =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "chunk_buf_diameter");
    uniform.gizmo_chunk.mat_translation =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "mat_translation");
    uniform.gizmo_chunk.mat_rotation =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "mat_rotation");
    uniform.gizmo_chunk.mat_orientation =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "mat_orientation");
    uniform.gizmo_chunk.mat_projection =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "mat_projection");
    uniform.gizmo_chunk.camera_position =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "camera_position");
    uniform.gizmo_chunk.time =
        glGetUniformLocation(shader_p[SHADER_GIZMO_CHUNK].asset.id, "time");

    uniform.post_processing.time =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "time");

    uniform.voxel.mat_perspective =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "mat_perspective");
    uniform.voxel.camera_position =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "camera_position");
    uniform.voxel.sun_rotation =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "sun_rotation");
    uniform.voxel.sky_light =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "sky_light");
    uniform.voxel.moon_light =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "moon_light");
    uniform.voxel.chunk_position =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "chunk_position");
    uniform.voxel.color =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "voxel_color");
    uniform.voxel.opacity =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "opacity");
    uniform.voxel.flashlight_position =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "flashlight_position");
    uniform.voxel.toggle_flashlight =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "toggle_flashlight");
    uniform.voxel.render_distance =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "render_distance");

    uniform.bounding_box.mat_perspective =
        glGetUniformLocation(shader_p[SHADER_BOUNDING_BOX].asset.id, "mat_perspective");
    uniform.bounding_box.position =
        glGetUniformLocation(shader_p[SHADER_BOUNDING_BOX].asset.id, "position");
    uniform.bounding_box.size =
        glGetUniformLocation(shader_p[SHADER_BOUNDING_BOX].asset.id, "size");
    uniform.bounding_box.color =
        glGetUniformLocation(shader_p[SHADER_BOUNDING_BOX].asset.id, "box_color");
}

static void draw_gizmo(void)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    fsl_mesh *mesh_p = fsl_mem_handle_get(mesh);
    m4f32 transform = {0};

    transform = player.camera_hud.projection.projection;
    transform = fsl_matrix_multiply(player.camera_hud.projection.orientation, transform);
    transform = fsl_matrix_multiply(player.camera_hud.projection.rotation, transform);
    transform = fsl_matrix_multiply(player.camera_hud.projection.target, transform);

    glUseProgram(shader_p[SHADER_GIZMO].asset.id);

    glBindBuffer(GL_ARRAY_BUFFER, mesh_p[MESH_GIZMO].transform_buf.id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m4f32), &transform, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(mesh_p[MESH_GIZMO].vao);
    glUniform3f(uniform.gizmo.color, 1.0f, 0.0f, 0.0f);
    glDrawElementsInstanced(GL_TRIANGLES, mesh_p[MESH_GIZMO].index_buf.len,
            GL_UNSIGNED_INT, NULL, 1);
}

static void draw_hotbar_items(void)
{
    u32 i = 0;

    gui_start_ui_items();

    for (i = 0; i < PLAYER_HOTBAR_SLOTS_MAX; ++i)
    {
        if (player.hotbar_slots[i])
        {
            gui_draw_ui_item(i * 34.0f +
                    (f32)render->size.x / 2.0f - 84.5f * 2.0f,
                    4.0f);
        }
    }
}

static void draw_everything(void)
{
    static str engine_version[FSL_ID_CAP] = {0};
    fsl_fbo *fbo_p = fsl_mem_handle_get(fbo);
    fsl_texture *texture_p = fsl_mem_handle_get(texture);
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_mesh *mesh_p = fsl_mem_handle_get(mesh);
    fsl_mesh *fsl_mesh_p = fsl_mem_handle_get(fsl_mesh_buf);
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    fsl_shader_program *fsl_shader_p = fsl_mem_handle_get(fsl_shader_buf);
    block *blocks_p = fsl_mem_handle_get(blocks);
    fsl_asset_metadata metadata = {0};
    u32 block_id = 0;

    f32 delay_in_hours = 6.0f;
    f32 sun_time = skybox_data.time * FSL_PI;
    f32 sun_angle = sun_time + 90.0f * FSL_DEG2RAD;
    f64 mid_day = 0.0f;
    f64 burn_cold = 0.0f;
    f64 burn = 0.0f;
    f64 burn_boost = 0.0f;
    f64 mid_night = 0.0f;

    /* ---- draw skybox ----------------------------------------------------- */

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_SKYBOX].fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    skybox_data.time = fmodf((f32)world.tick / SET_DAY_TICKS_MAX, 1.0f);
    skybox_data.time = fmodf(skybox_data.time * 2.0f - delay_in_hours / 12.0f, 2.0f);
    skybox_data.sun_rotation.x = cos(skybox_data.time * FSL_PI);
    skybox_data.sun_rotation.y = cos(skybox_data.time * FSL_PI) * 0.3f;
    skybox_data.sun_rotation.z = sin(skybox_data.time * FSL_PI);

    mid_day =       (sin(sun_time) + 1.0) / 2.0;
    mid_day =       pow(sin((FSL_PI / 2.0) * mid_day), 2.0);
    mid_day =       pow(sin((FSL_PI / 2.0) * mid_day), 2.0);

    burn_cold =     pow((sin((FSL_PI / 2.0) * sin(sun_time + (FSL_PI / 2.0))) + 1.0) / 2.0, 24.0);
    burn_cold +=    pow((sin((FSL_PI / 2.0) * sin(sun_time - (FSL_PI / 2.0))) + 1.0) / 2.0, 24.0);

    burn =          pow((sin(sun_time + (FSL_PI / 2.0)) + 1.0) / 2.0, 64.0);
    burn +=         pow((sin(sun_time - (FSL_PI / 2.0)) + 1.0) / 2.0, 64.0);

    burn_boost =    pow(sin(sun_time + (FSL_PI / 2.0)), 128.0);
    burn_boost +=   pow(sin(sun_time - (FSL_PI / 2.0)), 128.0);

    mid_night =     pow((sin((FSL_PI / 2.0) * sin(sun_time + FSL_PI)) + 1.0) / 2.0, 4.0);

    skybox_data.sky_color.x = (mid_day * 171.0f + mid_night * 1.0f + burn_cold * 8.0f) / 0xff;
    skybox_data.sky_color.y = (mid_day * 229.0f + mid_night * 4.0f + burn_cold * 4.0f) / 0xff;
    skybox_data.sky_color.z = (mid_day * 255.0f + mid_night * 14.0f + burn_cold * 18.0f) / 0xff;
    skybox_data.horizon_color.x = (mid_day * 224.0f + mid_night * 1.0f + burn_cold * 8.0f + burn * 92.0f + burn_boost * 116.0f) / 0xff;
    skybox_data.horizon_color.y = (mid_day * 244.0f + mid_night * 4.0f + burn_cold * 4.0f + burn * 5.0f + burn_boost * 77.0f) / 0xff;
    skybox_data.horizon_color.z = (mid_day * 255.0f + mid_night * 14.0f + burn_cold * 18.0f) / 0xff;
    skybox_data.sky_light.x = skybox_data.sky_color.x + skybox_data.horizon_color.x;
    skybox_data.sky_light.y = skybox_data.sky_color.y + skybox_data.horizon_color.y;
    skybox_data.sky_light.z = skybox_data.sky_color.z + skybox_data.horizon_color.z;
    skybox_data.moon_light.x = mid_night;
    skybox_data.moon_light.y = mid_night;
    skybox_data.moon_light.z = mid_night;

    m4f32 translation =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    m4f32 rotation =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    glUseProgram(shader_p[SHADER_SKYBOX].asset.id);

    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE, (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_rotation, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.rotation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE, (GLfloat*)&rotation);
    glUniformMatrix4fv(uniform.skybox.mat_orientation, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.orientation);
    glUniformMatrix4fv(uniform.skybox.mat_projection, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.projection);
    glUniform3fv(uniform.skybox.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.skybox.sky_color, 1, (GLfloat*)&skybox_data.sky_color);
    glUniform3fv(uniform.skybox.horizon_color, 1, (GLfloat*)&skybox_data.horizon_color);
    glUniform1i(uniform.skybox.render_layer, 0);

    glUniform1i(uniform.skybox.texture_sky, 0);
    glUniform1i(uniform.skybox.texture_horizon, 1);
    glUniform1i(uniform.skybox.texture_stars, 2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_p[TEXTURE_SKYBOX_VAL].asset.id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_p[TEXTURE_SKYBOX_HORIZON].asset.id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_p[TEXTURE_SKYBOX_STARS].asset.id);
    glBindVertexArray(fsl_mesh_p[FSL_MESH_INDEX_SKYBOX].vao);
    glDrawElements(GL_TRIANGLES, fsl_mesh_p[FSL_MESH_INDEX_SKYBOX].index_buf.len, GL_UNSIGNED_INT, 0);

    /* ---- draw sun -------------------------------------------------------- */

    if (settings.anti_aliasing)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_WORLD_MSAA].fbo);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_WORLD].fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    translation = (m4f32){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        skybox_data.sun_rotation.x * 2.0f,
        skybox_data.sun_rotation.y * 2.0f,
        skybox_data.sun_rotation.z * 2.0f,
        1.0f,
    };

    rotation = (m4f32){
        cosf(FSL_PI / 2.0f), -sinf(FSL_PI / 2.0f), 0.0f, 0.0f,
        sinf(FSL_PI / 2.0f), cosf(FSL_PI / 2.0f), 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    rotation = fsl_matrix_multiply(rotation,
            (m4f32){
            cosf(sun_angle), 0.0f, sinf(sun_angle), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -sinf(sun_angle), 0.0f, cosf(sun_angle), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            });

    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE, (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE, (GLfloat*)&rotation);
    glUniform1i(uniform.skybox.render_layer, 1);

    glUniform1i(uniform.skybox.texture_sun, 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture_p[TEXTURE_SUN].asset.id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    translation = (m4f32){
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -skybox_data.sun_rotation.x * 2.0f,
        -skybox_data.sun_rotation.y * 2.0f,
        -skybox_data.sun_rotation.z * 2.0f,
        1.0f,
    };

    sun_angle = skybox_data.time * FSL_PI - 90.0f * FSL_DEG2RAD;
    rotation = (m4f32){
        cosf(FSL_PI / 2.0f), -sinf(FSL_PI / 2.0f), 0.0f, 0.0f,
        sinf(FSL_PI / 2.0f), cosf(FSL_PI / 2.0f), 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    rotation = fsl_matrix_multiply(rotation,
            (m4f32){
            cosf(sun_angle), 0.0f, sinf(sun_angle), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -sinf(sun_angle), 0.0f, cosf(sun_angle), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            });

    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE,
            (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE,
            (GLfloat*)&rotation);
    glUniform1i(uniform.skybox.render_layer, 2);

    glBindTexture(GL_TEXTURE_2D, texture_p[TEXTURE_MOON].asset.id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    /* ---- draw world ------------------------------------------------------ */

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_p[SHADER_VOXEL].asset.id);
    glUniformMatrix4fv(uniform.voxel.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.perspective);
    glUniform3f(uniform.voxel.camera_position,
            player.camera.pos.x, player.camera.pos.y, player.camera.pos.z);
    glUniform3f(uniform.voxel.flashlight_position,
            player.transform.pos.x, player.transform.pos.y, player.transform.pos.z + player.eye_height);
    glUniform3fv(uniform.voxel.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.voxel.sky_light, 1, (GLfloat*)&skybox_data.sky_light);
    glUniform3fv(uniform.voxel.moon_light, 1, (GLfloat*)&skybox_data.moon_light);
    glUniform1f(uniform.voxel.toggle_flashlight, player.flag & FLAG_PLAYER_FLASHLIGHT ? 1.0f : 0.0f);
    glUniform1i(uniform.voxel.render_distance, settings.render_distance * CHUNK_DIAMETER);

    if (core.debug.trans_blocks)
        glUniform1f(uniform.voxel.opacity, 0.7f);
    else
        glUniform1f(uniform.voxel.opacity, 1.0f);

    static hhc_chunk ***cursor = NULL;
    static hhc_chunk *ch = NULL;
    cursor = &CHUNK_ORDER.p[CHUNKS_MAX[settings.render_distance] - 1];
    for (; cursor >= CHUNK_ORDER.p; --cursor)
    {
        ch = **cursor;
        if (!ch || !(ch->flag & FLAG_CHUNK_RENDER))
            continue;

        glUniform3f(uniform.voxel.chunk_position,
                (f32)(ch->pos.x * CHUNK_DIAMETER),
                (f32)(ch->pos.y * CHUNK_DIAMETER),
                (f32)(ch->pos.z * CHUNK_DIAMETER));

        glBindVertexArray(ch->vao);
        glDrawArrays(GL_POINTS, 0, ch->vbo_len);
    }

    /* ---- draw player ----------------------------------------------------- */

    if (player.camera_mode != PLAYER_CAMERA_MODE_1ST_PERSON)
    {
        fsl_mesh_draw(&player.mesh, &player.camera,
                player.transform.pos.x, player.transform.pos.y, player.transform.pos.z,
                player.transform.rot.x, player.transform.rot.y, player.transform.rot.z,
                player.transform.scale.x, player.transform.scale.y, player.transform.scale.z);
    }

    /* ---- draw player target bounding box --------------------------------- */

    glUseProgram(shader_p[SHADER_BOUNDING_BOX].asset.id);
    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.perspective);

    if (player.hit.block && core.flag.hud)
    {
        glUniform3f(uniform.bounding_box.position,
                (f32)player.hit.pos.x,
                (f32)player.hit.pos.y,
                (f32)player.hit.pos.z);
        glUniform3f(uniform.bounding_box.size, 1.0f, 1.0f, 1.0f);
        glUniform4f(uniform.bounding_box.color, 0.0f, 0.0f, 0.0f, 1.0f);

        glBindVertexArray(mesh_p[MESH_CUBE_OF_HAPPINESS].vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

     glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
             (GLfloat*)&player.camera.projection.perspective);
 
    /* ---- draw player chunk bounding box ---------------------------------- */

    if (core.debug.chunk_bounds)
    {
        glUniform3f(uniform.bounding_box.position,
                (f32)player.ch.x * CHUNK_DIAMETER,
                (f32)player.ch.y * CHUNK_DIAMETER,
                (f32)player.ch.z * CHUNK_DIAMETER);
        glUniform3f(uniform.bounding_box.size,
                CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);
        glUniform4f(uniform.bounding_box.color, 0.9f, 0.6f, 0.3f, 1.0f);

        glBindVertexArray(mesh_p[MESH_CUBE_OF_HAPPINESS].vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    /* ---- draw player bounding box ---------------------------------------- */

    if (core.debug.bounding_boxes)
    {
        glUniform3f(uniform.bounding_box.position,
                player.bbox.pos.x, player.bbox.pos.y, player.bbox.pos.z);
        glUniform3f(uniform.bounding_box.size,
                player.bbox.size.x, player.bbox.size.y, player.bbox.size.z);
        glUniform4f(uniform.bounding_box.color, 1.0f, 0.3f, 0.2f, 1.0f);

        glBindVertexArray(mesh_p[MESH_CUBE_OF_HAPPINESS].vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    /* ---- draw chunk scheduler visualizer --------------------------------- */

    if (core.debug.chunk_scheduler_visualizer)
    {
        glClear(GL_DEPTH_BUFFER_BIT);

        chunk_debug_draw_scheduler_visualizer(&player.camera, 0.5f);
    }

    if (settings.anti_aliasing)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_p[FBO_WORLD_MSAA].fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_p[FBO_WORLD].fbo);
        glBlitFramebuffer(0, 0, render->size.x, render->size.y, 0, 0,
                render->size.x, render->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    /* ---- draw hud gizmo -------------------------------------------------- */

    if (settings.anti_aliasing)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_HUD_MSAA].fbo);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_HUD].fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (core.flag.hud && core.flag.debug)
        draw_gizmo();

    /* ---- draw hud chunk gizmo -------------------------------------------- */

    if (core.flag.hud && core.debug.chunk_gizmo)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_p[SHADER_GIZMO_CHUNK].asset.id);

        glUniform1f(uniform.gizmo_chunk.gizmo_offset, (f32)settings.chunk_buf_radius + 0.5f);
        glUniform2iv(uniform.gizmo_chunk.render_size, 1, (GLint*)&render->size);
        glUniform1i(uniform.gizmo_chunk.chunk_buf_diameter, settings.chunk_buf_diameter);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_translation,
                1, GL_FALSE, (GLfloat*)&player.camera_hud.projection.target);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_rotation,
                1, GL_FALSE, (GLfloat*)&player.camera_hud.projection.rotation);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_orientation,
                1, GL_FALSE, (GLfloat*)&player.camera_hud.projection.orientation);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_projection,
                1, GL_FALSE, (GLfloat*)&player.camera_hud.projection.projection);

        v3f32 camera_position =
        {
            -player.camera.yaw.cos * player.camera.pitch.cos,
            player.camera.yaw.sin * player.camera.pitch.cos,
            player.camera.pitch.sin,
        };

        glUniform3fv(uniform.gizmo_chunk.camera_position, 1, (GLfloat*)&camera_position);
        glUniform1f(uniform.gizmo_chunk.time, render->time);

        glDisable(GL_BLEND);
        glBindVertexArray(chunk_gizmo_loaded.vao);
        glDrawArrays(GL_POINTS, 0, settings.chunk_buf_volume);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(chunk_gizmo_render.vao);
        glDrawArrays(GL_POINTS, 0, settings.chunk_buf_volume);
        glEnable(GL_BLEND);
    }

    if (settings.anti_aliasing)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_p[FBO_HUD_MSAA].fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_p[FBO_HUD].fbo);
        glBlitFramebuffer(0, 0, render->size.x, render->size.y, 0, 0,
                render->size.x, render->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    /* ---- draw ui --------------------------------------------------------- */

    fsl_ui_start(FALSE, TRUE);

    if (core.flag.hud)
    {
        if (!core.flag.debug)
            fsl_ui_draw(&texture_p[TEXTURE_CROSSHAIR], render->size.x / 2, render->size.y / 2,
                    0, 0,
                    0.0f, 0.0f, -1, -1, 0xffffffff);

        fsl_ui_draw(&texture_p[TEXTURE_ITEM_BAR], render->size.x / 2, render->size.y,
                texture_p[TEXTURE_ITEM_BAR].size.x * 2,
                texture_p[TEXTURE_ITEM_BAR].size.y * 2,
                84.5f, 18.0f, 0, 0, 0xffffffff);

        fsl_ui_draw(&texture_p[TEXTURE_ITEM_BAR_SELECTED],
                render->size.x / 2 - 2 + player.hotbar_slot_selected * 34,
                render->size.y - 2,
                texture_p[TEXTURE_ITEM_BAR_SELECTED].size.x * 2,
                texture_p[TEXTURE_ITEM_BAR_SELECTED].size.y * 2,
                84.5f, 18.0f, 0, 0, 0xffffffff);

        /* ---- draw item bar items ----------------------------------------- */

        draw_hotbar_items();
    }

    /* ---- draw debug info ------------------------------------------------- */

    fsl_text_start(font[FONT_MONO_BOLD], settings.font_size, 0, FALSE);

    fsl_text_push(fsl_stringf("FPS         [%u]\n", settings.fps),
            SET_MARGIN, SET_MARGIN, 0, 0, 0,
            settings.fps > 60 ? COLOR_TEXT_MOSS : COLOR_DIAGNOSTIC_ERROR);

    fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);

    if (core.flag.hud && core.flag.debug)
    {
        fsl_text_push(fsl_stringf("\n"
                    "TIME        [%.2lf]\n"
                    "CLOCK       [%02"PRIu64":%02"PRIu64"]\n"
                    "DAYS        [%"PRIu64"]\n",
                    (f64)render->time * FSL_NSEC2SEC,
                    (world.tick % SET_DAY_TICKS_MAX) / 1000,
                    ((world.tick * 60) / 1000) % 60,
                    world.days),
                SET_MARGIN, SET_MARGIN, 0, 0, 0,
                COLOR_TEXT_MOSS);

        fsl_text_push(fsl_stringf(
                    "XYZ         [%5.2lf %5.2lf %5.2lf]\n"
                    "BLOCK       [%.0lf %.0lf %.0lf]\n"
                    "CHUNK       [%d %d %d]\n"
                    "REGION      [%.0f %.0f %.0f]\n"
                    "PITCH/YAW   [%5.2f][%5.2f]\n"
                    "ACCELERATION[%5.2f %5.2f %5.2f]\n"
                    "VELOCITY    [%5.2f %5.2f %5.2f]\n"
                    "SPEED       [%5.2f]\n",
                    player.transform.pos.x, player.transform.pos.y, player.transform.pos.z,
                    floor(player.transform.pos.x),
                    floor(player.transform.pos.y),
                    floor(player.transform.pos.z),
                    player.ch.x, player.ch.y, player.ch.z,
                    floorf((f32)player.ch.x / CHUNK_REGION_DIAMETER),
                    floorf((f32)player.ch.y / CHUNK_REGION_DIAMETER),
                    floorf((f32)player.ch.z / CHUNK_REGION_DIAMETER),
                    player.transform.rot.y, player.transform.rot.z,
                    player.acceleration.x, player.acceleration.y, player.acceleration.z,
                    player.velocity.x, player.velocity.y, player.velocity.z,
                    player.speed),
                SET_MARGIN, SET_MARGIN, 0, 0, 0,
                COLOR_TEXT_DEFAULT);

        fsl_text_push(fsl_stringf(
                    "OVERFLOW    [%s %s %s]\r",
                    (player.flag & FLAG_PLAYER_OVERFLOW_X) ?
                    (player.flag & FLAG_PLAYER_OVERFLOW_PX) ?
                    "        " : "        " : "NONE",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Y) ?
                    (player.flag & FLAG_PLAYER_OVERFLOW_PY) ?
                    "        " : "        " : "NONE",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Z) ?
                    (player.flag & FLAG_PLAYER_OVERFLOW_PZ) ?
                    "        " : "        " : "NONE"),
                SET_MARGIN, SET_MARGIN, 0, 0, 0,
                COLOR_DIAGNOSTIC_NONE);

        fsl_text_push(fsl_stringf(
                    "             %s %s %s\r",
                    (player.flag & FLAG_PLAYER_OVERFLOW_X) &&
                    (player.flag & FLAG_PLAYER_OVERFLOW_PX) ? "POSITIVE" : "    ",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Y) &&
                    (player.flag & FLAG_PLAYER_OVERFLOW_PY) ? "POSITIVE" : "    ",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Z) &&
                    (player.flag & FLAG_PLAYER_OVERFLOW_PZ) ? "POSITIVE" : "    "),
                SET_MARGIN, SET_MARGIN, 0, 0, 0,
                FSL_DIAGNOSTIC_COLOR_SUCCESS);

        fsl_text_push(fsl_stringf(
                    "             %s %s %s\n",
                    (player.flag & FLAG_PLAYER_OVERFLOW_X) &&
                    !(player.flag & FLAG_PLAYER_OVERFLOW_PX) ? "NEGATIVE" : "    ",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Y) &&
                    !(player.flag & FLAG_PLAYER_OVERFLOW_PY) ? "NEGATIVE" : "    ",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Z) &&
                    !(player.flag & FLAG_PLAYER_OVERFLOW_PZ) ? "NEGATIVE" : "    "),
                SET_MARGIN, SET_MARGIN, 0, 0, 0,
                FSL_DIAGNOSTIC_COLOR_ERROR);

        fsl_text_push(fsl_stringf(
                    "RATIO       [%.2f]\n"
                    "SKYBOX TIME [%.2f]\n"
                    "SKYBOX RGB  [%.2f %.2f %.2f]\n"
                    "SUN ANGLE   [%.2f %.2f %.2f]\n",
                    (f32)render->size.x / render->size.y,
                    skybox_data.time,
                    skybox_data.sky_color.x,
                    skybox_data.sky_color.y,
                    skybox_data.sky_color.z,
                    skybox_data.sun_rotation.x,
                    skybox_data.sun_rotation.y,
                    skybox_data.sun_rotation.z),
                SET_MARGIN, SET_MARGIN, 0, 0, 0,
                COLOR_DIAGNOSTIC_INFO);

        if (player.hit.hit)
        {
            block_id = GET_BLOCK_ID(*player.hit.block);
            metadata = fsl_asset_get_metadata(blocks_p[block_id].asset);
            fsl_text_push(fsl_stringf(
                        "TARGET      [%u][%s]\n"
                        "XYZ         [%"PRId64" %"PRId64" %"PRId64"]\n",
                        block_id,
                        metadata.name,
                        player.hit.pos.x, player.hit.pos.y, player.hit.pos.z),
                    SET_MARGIN, SET_MARGIN,
                    0, 0, 0,
                    COLOR_TEXT_DEFAULT);
        }

        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);

        fsl_text_push(fsl_stringf(
                    "CHUNK SCHEDULER 1 [%7d/%-7"PRIu64"][pop/push: %7"PRIu64"/%-7"PRIu64"]\n"
                    "CHUNK SCHEDULER 2 [%7d/%-7"PRIu64"][pop/push: %7"PRIu64"/%-7"PRIu64"]\n"
                    "CHUNK SCHEDULER 3 [%7d/%-7"PRIu64"][pop/push: %7"PRIu64"/%-7"PRIu64"]\n"
                    "TOTAL CHUNKS  [%15"PRIu64"]                           \n",
                    chunk_sched[0].count, chunk_sched[0].len,
                    chunk_sched[0].cursor_pop, chunk_sched[0].cursor_push,

                    chunk_sched[1].count, chunk_sched[1].len,
                    chunk_sched[1].cursor_pop, chunk_sched[1].cursor_push,

                    chunk_sched[2].count, chunk_sched[2].len,
                    chunk_sched[2].cursor_pop, chunk_sched[2].cursor_push,
                    CHUNKS_MAX[settings.render_distance]),
                render->size.x - SET_MARGIN, SET_MARGIN,
                FSL_TEXT_ALIGN_RIGHT, 0, 0,
                COLOR_TEXT_DEFAULT);

        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
        fsl_text_start(font[FONT_MONO], FSL_FONT_SIZE_DEFAULT, 0, FALSE);

        fsl_engine_get_string(engine_version, FSL_ENGINE_STR_INDEX_VERSION);
        fsl_text_push(fsl_stringf(
                    "Game:     %s %s\n"
                    "Author:   %s\n"
                    "Engine:   %s %s\n"
                    "OpenGL:   %s\n"
                    "GLSL:     %s\n"
                    "Vendor:   %s\n"
                    "Renderer: %s\n",
                    GAME_NAME, GAME_VERSION,
                    FSL_ENGINE_AUTHOR,
                    FSL_ENGINE_NAME, engine_version,
                    glGetString(GL_VERSION),
                    glGetString(GL_SHADING_LANGUAGE_VERSION),
                    glGetString(GL_VENDOR),
                    glGetString(GL_RENDERER)),
                SET_MARGIN, render->size.y - SET_MARGIN,
                0, FSL_TEXT_ALIGN_BOTTOM, render->size.x,
                FSL_DIAGNOSTIC_COLOR_TRACE);

        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
    }

    /* ---- draw logger strings --------------------------------------------- */

    if (core.flag.super_debug)
    {
        i32 i = 0;
        u32 index = 0;
        i32 logger_panel_height = 400;
        fsl_log_entry *log_entry = NULL;

        fsl_ui_start(TRUE, FALSE);
        fsl_ui_draw_nine_slice(&fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_INACTIVE],
                10, render->size.y - logger_panel_height - 30,
                render->size.x - 20, logger_panel_height + 20, 8, 0xffffff5f);

        fsl_text_start(font[FONT_MONO_BOLD], settings.font_size, 0, FALSE);

        log_entry = fsl_mem_handle_get(logger_core.buf);
        for (i = 20; i > 0; --i)
        {
            index = fsl_mod_i32(logger_core.cursor - i - scrool, FSL_LOGGER_HISTORY_MAX);
            fsl_text_push(fsl_stringf("%s\n", log_entry[index].message),
                    SET_MARGIN * 2, render->size.y - SET_MARGIN * 2,
                    0, 0, render->size.x - SET_MARGIN * 4,
                    log_entry[index].color);

            if ((i32)fsl_get_text_height() + SET_MARGIN * 2 >= logger_panel_height)
                break;
        }

        /* this "useless" function call aligns all the pushed strings correctly once at
         * the end of the loop, do not touch it. */
        fsl_text_push("", 0, 0, 0, FSL_TEXT_ALIGN_BOTTOM, 0, 0);
        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
    }

    fsl_ui_stop();

    /* ---- post processing ------------------------------------------------- */

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_POST_PROCESSING].fbo);
    glUseProgram(fsl_shader_p[FSL_SHADER_INDEX_UNIT_QUAD].asset.id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glBindTexture(GL_TEXTURE_2D, fbo_p[FBO_SKYBOX].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo_p[FBO_WORLD].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo_p[FBO_HUD].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    fsl_fbo_blit(fbo_p[FBO_POST_PROCESSING].fbo);

    /* ---- final ----------------------------------------------------------- */

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader_p[SHADER_POST_PROCESSING].asset.id);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glUniform1ui(uniform.post_processing.time, ((u32)(render->time) & 0x1ff) + 1);
    glBindTexture(GL_TEXTURE_2D, fbo_p[FBO_POST_PROCESSING].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, GAME_TITLE, 1280, 1054,
                GAME_RELEASE_BUILD | FSL_FLAG_MULTISAMPLE) != FSL_ERR_SUCCESS ||
            game_init() != FSL_ERR_SUCCESS)
        goto cleanup;

#ifndef HHC_RELEASE_BUILD
    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "DEBUG BUILD\n");

    glfwSetWindowPos(render->window, 1920 - render->size.x, 24);
#endif /* HHC_RELEASE_BUILD */

    if (!MODE_INTERNAL_COLLIDE)
    {
        LOGWARNING(HHC_ERR_COLLISIONS_DISABLED,
                FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "'MODE_INTERNAL_COLLIDE' Disabled\n");
    }

    if (rand_init() != FSL_ERR_SUCCESS ||
            settings_init() != FSL_ERR_SUCCESS)
        goto cleanup;

    /* ---- set mouse input ------------------------------------------------- */

    glfwSetInputMode(render->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(render->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "GLFW: Raw Mouse Motion Enabled\n");
    }
    else LOGERROR(FSL_ERR_GLFW,
            FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "GLFW: Raw Mouse Motion Not Supported\n");

    /* ---- set callbacks --------------------------------------------------- */

    glfwSetKeyCallback(render->window, callback_key);
    callback_key(render->window, 0, 0, 0, 0);

    glfwSetScrollCallback(render->window, callback_scroll);
    callback_scroll(render->window, 0.0f, 0.0f);

    /* ---- set graphics ---------------------------------------------------- */

    if (assets_init() != FSL_ERR_SUCCESS)
        goto cleanup;
    if (gui_init() != FSL_ERR_SUCCESS)
        goto cleanup;

    /*temp off
    init_super_debugger(render->size);
    */

    /* ---- end set graphics ------------------------------------------------ */

    player_init(&player, "Lily");
    input_init();
    bind_shader_uniforms();

section_menu_title:

section_menu_pause:

section_world_loaded:

    if (!core.flag.world_loaded &&
            world_init("Poop Consistency Tester", 0, &player) != FSL_ERR_SUCCESS)
            goto cleanup;

    while (fsl_engine_running(&callback_framebuffer_size))
    {
        input_update(&player);
        settings_update();
        world_update(&player);
        draw_everything();

        fsl_process_screenshot_request(GAME_DIR_NAME_SCREENSHOTS, world.name);
        fsl_limit_framerate(settings.target_fps, render->time);

        if (!core.flag.world_loaded)
            goto section_menu_title;

        if (core.flag.paused)
            goto section_menu_pause;
    }

cleanup:

    gui_free();
    assets_free();
    chunking_free();
    rand_free();
    fsl_mem_arena_free(&memory_arena_internal, "main().memory_arena_internal");
    fsl_engine_close();
    return *GAME_ERR;
}
