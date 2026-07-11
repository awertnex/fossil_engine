#include "deps/fossil/common/common_values.h"
#include "deps/fossil/common/config.h"
#include "deps/fossil/common/diagnostics.h"
#include "deps/fossil/common/engine_info.h"
#include "deps/fossil/common/limits.h"
#include "deps/fossil/common/types.h"
#include "deps/fossil/assets/assets.h"
#include "deps/fossil/engine/engine.h"
#include "deps/fossil/engine/engine_assets.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/math/matrix.h"
#include "deps/fossil/math/noise.h"
#include "deps/fossil/math/vector.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shader_types.h"
#include "deps/fossil/string/string.h"
#include "deps/fossil/ui/ui.h"

#include "deps/fossil/h/time.h"

#include "h/main.h"

#include "chunking/chunking.h"
#include "chunking/chunking_internal.h"
#include "chunking/chunking_debug_tools.h"
#include "gui/gui.h"
#include "gui/gui_menus.h"
#include "plugins/big_num_separator/big_num_separator.h"
#include "settings/settings.h"
#include "super_debugger/super_debugger.h"

#include "h/game_info.h"
#include "h/assets.h"
#include "h/config_internal.h"
#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/input.h"
#include "h/main.h"
#include "h/player.h"
#include "h/world.h"

#include <stddef.h>
#include <inttypes.h>
#include <math.h>

u32 *const GAME_ERR = (u32*)&fsl_err;
fsl_render *render = NULL;
struct hhc_core core = {0};
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
static void skybox_draw(void);
static void ui_hud_draw(void);
static void draw_world(void);
static void draw_debug_gizmo_axis(void);
static void world_draw(void);

static void callback_framebuffer_size(i32 size_x, i32 size_y)
{
    fsl_fbo *fbo_p = fsl_mem_handle_get(fbo);

    player.camera.ratio = (f32)size_x / (f32)size_y;
    player.camera_hud.ratio = (f32)size_x / (f32)size_y;

    fsl_fbo_realloc(&fbo_p[FBO_SKYBOX], render->size.x, render->size.y, FALSE, 0);
    fsl_fbo_realloc(&fbo_p[FBO_WORLD], render->size.x, render->size.y, FALSE, 0);
    fsl_fbo_realloc(&fbo_p[FBO_WORLD_MSAA], render->size.x, render->size.y, TRUE, 4);
    fsl_fbo_realloc(&fbo_p[FBO_HUD], render->size.x, render->size.y, FALSE, 0);
    fsl_fbo_realloc(&fbo_p[FBO_HUD_MSAA], render->size.x, render->size.y, TRUE, 4);
    fsl_fbo_realloc(&fbo_p[FBO_POST_PROCESSING], render->size.x, render->size.y, FALSE, 0);
    g_buffer_init(&g_buf, render->size.x, render->size.y);

    gui_update(render->size);
    super_debugger_update(render->size);
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

    if (core.flag.super_debug)
        super_debugger_logger_scroll((i32)yoffset);
    else if (player.flag & FLAG_PLAYER_ZOOMER)
        player.camera.zoom =
            fsl_clamp_f64(player.camera.zoom + yoffset * FSL_CAMERA_ZOOM_SPEED, 0.0f, FSL_CAMERA_ZOOM_MAX);
    else
        player_hotbar_selected_advance(&player, (i64)-yoffset);
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
        glGetUniformLocation(shader_p[SHADER_GIZMO_AXIS].asset.id, "gizmo_color");

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

    uniform.post_processing.texture_skybox =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "texture_skybox");
    uniform.post_processing.texture_world_pos =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "texture_world_pos");
    uniform.post_processing.texture_world_normal =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "texture_world_normal");
    uniform.post_processing.texture_world_albedo_specular =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "texture_world_albedo_specular");
    uniform.post_processing.texture_hud =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "texture_hud");
    uniform.post_processing.time =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "time");
    uniform.post_processing.mat_projection =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "mat_projection");
    uniform.post_processing.camera_far =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "camera_far");
    uniform.post_processing.camera_near =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "camera_near");
    uniform.post_processing.ssao_sample =
        glGetUniformLocation(shader_p[SHADER_POST_PROCESSING].asset.id, "ssao_sample");

    uniform.voxel.mat_view =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "mat_view");
    uniform.voxel.mat_perspective =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "mat_perspective");
    uniform.voxel.camera_position =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "camera_position");
    uniform.voxel.camera_far =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "camera_far");
    uniform.voxel.camera_near =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "camera_near");
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
    uniform.voxel.render_distance =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "render_distance");
    uniform.voxel.spotlight.pos =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "flashlight.pos");
    uniform.voxel.spotlight.direction =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "flashlight.direction");
    uniform.voxel.spotlight.cutoff =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "flashlight.cutoff");
    uniform.voxel.spotlight.feather_factor =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "flashlight.feather_factor");
    uniform.voxel.spotlight.intensity =
        glGetUniformLocation(shader_p[SHADER_VOXEL].asset.id, "flashlight.intensity");

    uniform.bounding_box.mat_perspective =
        glGetUniformLocation(shader_p[SHADER_BOUNDING_BOX].asset.id, "mat_perspective");
    uniform.bounding_box.position =
        glGetUniformLocation(shader_p[SHADER_BOUNDING_BOX].asset.id, "position");
    uniform.bounding_box.size =
        glGetUniformLocation(shader_p[SHADER_BOUNDING_BOX].asset.id, "size");
    uniform.bounding_box.color =
        glGetUniformLocation(shader_p[SHADER_BOUNDING_BOX].asset.id, "box_color");
}

static void skybox_draw(void)
{
    f32 delay_in_hours = 6.0f;
    f32 sun_time = skybox_data.time * FSL_PI;
    f32 sun_angle = sun_time + 90.0f * FSL_DEG2RAD;
    f64 mid_day = 0.0f;
    f64 burn_cold = 0.0f;
    f64 burn = 0.0f;
    f64 burn_boost = 0.0f;
    f64 mid_night = 0.0f;
    m4f32 translation = {0};
    m4f32 rotation_yaw = {0};
    m4f32 rotation_pitch = {0};
    fsl_fbo *fbo_p = fsl_mem_handle_get(fbo);
    fsl_texture *texture_p = fsl_mem_handle_get(texture);
    fsl_mesh *fsl_mesh_p = fsl_mem_handle_get(fsl_mesh_buf);
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_SKYBOX].fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    skybox_data.time = fmodf((f32)world.tick / SET_DAY_TICKS_MAX, 1.0f);
    skybox_data.time = fmodf(skybox_data.time * 2.0f - delay_in_hours / 12.0f, 2.0f);
    skybox_data.sun_rotation.x = cos(skybox_data.time * FSL_PI);
    skybox_data.sun_rotation.y = cos(skybox_data.time * FSL_PI) * 0.3f;
    skybox_data.sun_rotation.z = sin(skybox_data.time * FSL_PI);

    mid_day =       (sin(sun_time) + 1.0) / 2.0;
    mid_day =       pow(sin(FSL_HALF_PI * mid_day), 2.0);
    mid_day =       pow(sin(FSL_HALF_PI * mid_day), 2.0);

    burn_cold =     pow((sin(FSL_HALF_PI * sin(sun_time + FSL_HALF_PI)) + 1.0) / 2.0, 24.0) +
        pow((sin(FSL_HALF_PI * sin(sun_time - FSL_HALF_PI)) + 1.0) / 2.0, 24.0);

    burn =          pow((sin(sun_time + FSL_HALF_PI) + 1.0) / 2.0, 64.0) +
        pow((sin(sun_time - FSL_HALF_PI) + 1.0) / 2.0, 64.0);

    burn_boost =    pow(sin(sun_time + FSL_HALF_PI), 128.0) +
        pow(sin(sun_time - FSL_HALF_PI), 128.0);
    mid_night =     pow((sin(FSL_HALF_PI * sin(sun_time + FSL_PI)) + 1.0) / 2.0, 4.0);

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

    translation = fsl_identity_m4f32();
    rotation_yaw = fsl_identity_m4f32();
    rotation_pitch = fsl_identity_m4f32();

    glUseProgram(shader_p[SHADER_SKYBOX].asset.id);

    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE, (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_rotation, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.rotation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE, (GLfloat*)&rotation_yaw);
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
    glDrawElements(GL_TRIANGLES, fsl_mesh_p[FSL_MESH_INDEX_SKYBOX].index_buf.len,
            GL_UNSIGNED_INT, 0);

    /* ---- draw sun -------------------------------------------------------- */

    if (settings.anti_aliasing)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_WORLD_MSAA].fbo);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_WORLD].fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    translation = fsl_identity_m4f32();
    translation.a41 = skybox_data.sun_rotation.x * 2.0f;
    translation.a42 = skybox_data.sun_rotation.y * 2.0f;
    translation.a43 = skybox_data.sun_rotation.z * 2.0f;

    rotation_yaw = fsl_identity_m4f32();
    rotation_yaw.a11 = cosf(FSL_HALF_PI);
    rotation_yaw.a12 = -sinf(FSL_HALF_PI);
    rotation_yaw.a21 = sinf(FSL_HALF_PI);
    rotation_yaw.a22 = cosf(FSL_HALF_PI);

    rotation_pitch = fsl_identity_m4f32();
    rotation_pitch.a11 = cosf(sun_angle);
    rotation_pitch.a13 = sinf(sun_angle);
    rotation_pitch.a31 = -sinf(sun_angle);
    rotation_pitch.a33 = cosf(sun_angle);

    rotation_yaw = fsl_multiply_m4f32(rotation_yaw, rotation_pitch);

    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE, (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE, (GLfloat*)&rotation_yaw);
    glUniform1i(uniform.skybox.render_layer, 1);

    glUniform1i(uniform.skybox.texture_sun, 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture_p[TEXTURE_SUN].asset.id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    /* ---- draw moon ------------------------------------------------------- */

    translation.a41 = -skybox_data.sun_rotation.x * 2.0f;
    translation.a42 = -skybox_data.sun_rotation.y * 2.0f;
    translation.a43 = -skybox_data.sun_rotation.z * 2.0f;

    sun_angle = skybox_data.time * FSL_PI - FSL_HALF_PI;

    rotation_yaw = fsl_identity_m4f32();
    rotation_yaw.a11 = cosf(FSL_HALF_PI);
    rotation_yaw.a22 = -sinf(FSL_HALF_PI);
    rotation_yaw.a11 = sinf(FSL_HALF_PI);
    rotation_yaw.a22 = cosf(FSL_HALF_PI);

    rotation_pitch = fsl_identity_m4f32();
    rotation_pitch.a11 = cosf(sun_angle);
    rotation_pitch.a13 = sinf(sun_angle);
    rotation_pitch.a31 = -sinf(sun_angle);
    rotation_pitch.a33 = cosf(sun_angle);

    rotation_yaw = fsl_multiply_m4f32(rotation_yaw, rotation_pitch);

    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE,
            (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE,
            (GLfloat*)&rotation_yaw);
    glUniform1i(uniform.skybox.render_layer, 2);

    glBindTexture(GL_TEXTURE_2D, texture_p[TEXTURE_MOON].asset.id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void ui_hud_draw(void)
{
    u32 i = 0;
    f32 scale = settings.gui_scale;
    f32 item_bar_item_stride = 0.0f;

    if (fsl_on_time_interval(&refresh_interval.fps_string,
                FSL_SEC2NSEC / SET_TEXT_REFRESH_INTERVAL, render->time))
        settings.fps = 1 / render->time_delta_f;

    if (!core.flag.hud)
        return;

    if (!core.flag.debug)
        fsl_ui_element_draw(&ui_element[UI_ELEMENT_CROSSHAIR]);

    fsl_ui_element_draw(&ui_element[UI_ELEMENT_HOTBAR]);
    fsl_ui_element_draw(&ui_element[UI_ELEMENT_HOTBAR_SELECTED]);

    gui_start_ui_items(render->size);

    for (i = 0; i < CONTAINER_HOTBAR_SLOTS_MAX; ++i)
    {
        if (player.hotbar_slots[i].id)
        {
            item_bar_item_stride = i * 17.0f * scale + (f32)render->size.x / 2.0f - 84.5f * scale;

            gui_draw_ui_item(player.hotbar_slots[i].id,
                    item_bar_item_stride, 4.0f * scale, render->size);
        }
    }

    /* ---- draw menus n whatnot -------------------------------------------- */

    fsl_ui_start(FALSE);

    if (state_menu_depth)
    {
        /* ---- draw container inventory survival --------------------------- */

        switch (player.menu_state)
        {
            case STATE_PLAYER_MENU_INVENTORY_SURVIVAL:
                fsl_ui_element_draw(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL]);
                break;

            default:
                break;
        }
    }
}

static void draw_world(void)
{
    fsl_fbo *fbo_p = fsl_mem_handle_get(fbo);
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    hhc_chunk *chunk = NULL;
    static hhc_spotlight flashlight = {0};
    static hhc_spotlight flashlight_last = {0};
    f32 flashlight_flicker = 0.0f;
    f32 flashlight_flicker_intensity = 2.0f;
    f32 flashlight_flicker_gathering = 0.8f;
    i32 i = 0;
    f32 lerp_speed = 0.2f;
    f32 lerp_speed_toggle = 0.5f;
    f32 k = 1.0f - render->time_delta_f;

    flashlight.pos.x = player.transform.pos.x - player.yaw.sin * 0.3f;
    flashlight.pos.y = player.transform.pos.y - player.yaw.cos * 0.3f;
    flashlight.pos.z = player.transform.pos.z + player.eye_height - 0.3f;
    flashlight.direction.x = player.yaw.cos * player.pitch.cos;
    flashlight.direction.y = -player.yaw.sin * player.pitch.cos;
    flashlight.direction.z = -player.pitch.sin;
    flashlight.cutoff = cosf(35.0f * FSL_DEG2RAD);
    flashlight.feather_factor = 0.2f;

    if (player.flag & FLAG_PLAYER_FLASHLIGHT)
    {
        flashlight_flicker =
            fsl_perlin_noise_1d((f32)render->time * FSL_NSEC2SEC, 2.0f, 4.0f, 0) +
            fsl_perlin_noise_1d((f32)render->time * FSL_NSEC2SEC, 2.0f, 15.0f, 0);

        if (flashlight_flicker >= flashlight_flicker_gathering)
            flashlight_flicker = flashlight_flicker_intensity;
        else if (flashlight_flicker <= -flashlight_flicker_gathering)
            flashlight_flicker = -flashlight_flicker_intensity;
        else
            flashlight_flicker = 0.0f;

        flashlight.intensity = 20.0f + flashlight_flicker;
    }
    else
        flashlight.intensity = 0.0f;

    flashlight_last.pos.x = fsl_lerp_exp_f32(flashlight_last.pos.x, flashlight.pos.x,
            k, lerp_speed);
    flashlight_last.pos.y = fsl_lerp_exp_f32(flashlight_last.pos.y, flashlight.pos.y,
            k, lerp_speed);
    flashlight_last.pos.z = fsl_lerp_exp_f32(flashlight_last.pos.z, flashlight.pos.z,
            k, lerp_speed);

    flashlight_last.direction.x = fsl_lerp_exp_f32(flashlight_last.direction.x,
            flashlight.direction.x, k, lerp_speed);
    flashlight_last.direction.y = fsl_lerp_exp_f32(flashlight_last.direction.y,
            flashlight.direction.y, k, lerp_speed);
    flashlight_last.direction.z = fsl_lerp_exp_f32(flashlight_last.direction.z,
            flashlight.direction.z, k, lerp_speed);

    flashlight_last.direction = fsl_normalize_v3f32(flashlight_last.direction);
    flashlight_last.cutoff = flashlight.cutoff;
    flashlight_last.feather_factor = flashlight.feather_factor;
    flashlight_last.intensity = fsl_lerp_exp_f32(flashlight_last.intensity,
            flashlight.intensity, k, lerp_speed_toggle);

    if (settings.anti_aliasing)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_WORLD_MSAA].fbo);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_WORLD].fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, g_buf.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_p[SHADER_VOXEL].asset.id);

    glUniformMatrix4fv(uniform.voxel.mat_view, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.view);
    glUniformMatrix4fv(uniform.voxel.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.perspective);
    glUniform3fv(uniform.voxel.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.voxel.sky_light, 1, (GLfloat*)&skybox_data.sky_light);
    glUniform3fv(uniform.voxel.moon_light, 1, (GLfloat*)&skybox_data.moon_light);
    glUniform3f(uniform.voxel.camera_position,
            player.camera.pos.x, player.camera.pos.y, player.camera.pos.z);
    glUniform1f(uniform.voxel.camera_far, player.camera.far);
    glUniform1f(uniform.voxel.camera_near, player.camera.near);
    glUniform1i(uniform.voxel.render_distance, settings.render_distance * CHUNK_DIAMETER);

    glUniform3fv(uniform.voxel.spotlight.pos, 1, (GLfloat*)&flashlight_last.pos);
    glUniform3fv(uniform.voxel.spotlight.direction, 1, (GLfloat*)&flashlight_last.direction);
    glUniform1f(uniform.voxel.spotlight.cutoff, flashlight_last.cutoff);
    glUniform1f(uniform.voxel.spotlight.feather_factor, flashlight_last.feather_factor);
    glUniform1f(uniform.voxel.spotlight.intensity, flashlight_last.intensity);

    if (core.debug.trans_blocks)
        glUniform1f(uniform.voxel.opacity, 0.6f);
    else
        glUniform1f(uniform.voxel.opacity, 1.0f);

    for (i = chunk_order.chunks_max - 1; i >= 0; --i)
    {
        chunk = chunk_tab.p[chunk_order.p[i]];
        if (chunk && chunk->flag & FLAG_CHUNK_VISIBLE)
        {
            glBindVertexArray(chunk->mesh_deprecated.vao);
            glDrawArraysInstanced(GL_POINTS, 0, chunk->mesh_deprecated.vbo_len, 1);
        }
    }
}

static void draw_debug_gizmo_axis(void)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    fsl_mesh *mesh_p = fsl_mem_handle_get(mesh);
    m4f32 transform = {0};

    transform = player.camera_hud.projection.projection;
    transform = fsl_multiply_m4f32(player.camera_hud.projection.orientation, transform);
    transform = fsl_multiply_m4f32(player.camera_hud.projection.rotation, transform);
    transform = fsl_multiply_m4f32(player.camera_hud.projection.target, transform);

    glUseProgram(shader_p[SHADER_GIZMO_AXIS].asset.id);

    glBindBuffer(GL_ARRAY_BUFFER, mesh_p[MESH_GIZMO_AXIS].transform_buf.id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m4f32), &transform, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(mesh_p[MESH_GIZMO_AXIS].vao);
    glUniform3f(uniform.gizmo.color, 1.0f, 0.0f, 0.0f);
    glDrawElementsInstanced(GL_TRIANGLES, mesh_p[MESH_GIZMO_AXIS].index_buf.len,
            GL_UNSIGNED_INT, NULL, 1);
}

static void world_draw(void)
{
    static str engine_version[FSL_ID_CAP] = {0};
    fsl_fbo *fbo_p = fsl_mem_handle_get(fbo);
    fsl_mesh *mesh_p = fsl_mem_handle_get(mesh);
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    fsl_texture *texture_p = fsl_mem_handle_get(texture);
    hhc_block *blocks_p = fsl_mem_handle_get(blocks);
    fsl_asset_metadata metadata = {0};
    u32 block_id = 0;

    skybox_draw();
    draw_world();

    if (settings.anti_aliasing)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_WORLD_MSAA].fbo);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_WORLD].fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* ---- draw player ----------------------------------------------------- */

    if (player.camera_mode != PLAYER_CAMERA_MODE_1ST_PERSON)
    {
        fsl_mesh_draw(&player.mesh, &player.camera,
                texture_p[TEXTURE_SUN].asset.id,
                player.transform.pos.x, player.transform.pos.y, player.transform.pos.z,
                0.0f, 0.0f, player.transform.rot.z,
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

    if (core.debug.chunk_bounds && core.flag.hud)
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

    if (core.debug.bounding_boxes && core.flag.hud)
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

    if (core.debug.chunk_scheduler_visualizer && core.flag.hud)
        chunk_debug_scheduler_visualizer_draw(&player.camera);

    if (settings.anti_aliasing)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_p[FBO_WORLD_MSAA].fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_p[FBO_WORLD].fbo);
        glBlitFramebuffer(0, 0, render->size.x, render->size.y, 0, 0,
                render->size.x, render->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    /* ---- draw hud gizmos ------------------------------------------------- */

    if (settings.anti_aliasing)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_HUD_MSAA].fbo);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_p[FBO_HUD].fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (core.flag.hud)
    {
        if (core.flag.debug)
            draw_debug_gizmo_axis();

        if (core.debug.chunk_gizmo)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            chunk_debug_chunk_gizmo_draw(&player.camera_hud);
        }
    }

    if (settings.anti_aliasing)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_p[FBO_HUD_MSAA].fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_p[FBO_HUD].fbo);
        glBlitFramebuffer(0, 0, render->size.x, render->size.y, 0, 0,
                render->size.x, render->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    /* ---- draw ui --------------------------------------------------------- */

    fsl_ui_start(TRUE);

    ui_hud_draw();

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
                    "SPEED       [%5.2f]\n\n",
                    player.transform.pos.x, player.transform.pos.y, player.transform.pos.z,
                    floor(player.transform.pos.x),
                    floor(player.transform.pos.y),
                    floor(player.transform.pos.z),
                    player.ch.x, player.ch.y, player.ch.z,
                    floorf((f32)player.ch.x / CHUNK_REGION_DIAMETER),
                    floorf((f32)player.ch.y / CHUNK_REGION_DIAMETER),
                    floorf((f32)player.ch.z / CHUNK_REGION_DIAMETER),
                    player.transform.rot.y, player.transform.rot.z,
                    player.kn.acceleration.x,
                    player.kn.acceleration.y,
                    player.kn.acceleration.z,
                    player.kn.velocity.x,
                    player.kn.velocity.y,
                    player.kn.velocity.z,
                    player.kn.speed),
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
                    "FRAME RATIO [%.2f]\n"
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

        fsl_text_push(fsl_stringf("%s", chunk_tab.receipt_center.printed),
                SET_MARGIN, SET_MARGIN, 0, 0, 0,
                COLOR_TEXT_ECONOMIC);

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
                    "CHUNK SCHEDULER [%7d/%-7"PRIu64"][pop/push: %7"PRIu64"/%-7"PRIu64"]\n"
                    "RENDER DISTANCE [%2d]\n",
                    chunk_sched.count,
                    chunk_order.chunks_max,
                    chunk_sched.cursor_pop, chunk_sched.cursor_push,
                settings.render_distance),
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

    fsl_fbo_blit(fbo_p[FBO_HUD].fbo);
    fsl_ui_stop();

    /* ---- draw super debugger --------------------------------------------- */

    if (core.flag.super_debug)
        super_debugger_draw(render->size, fbo_p[FBO_HUD].fbo);

    /* ---- post processing ------------------------------------------------- */

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader_p[SHADER_POST_PROCESSING].asset.id);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo_p[FBO_SKYBOX].color_buf);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_buf.color_buf_pos);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_buf.color_buf_normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, g_buf.color_buf_albedo_specular);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, fbo_p[FBO_HUD].color_buf);

    glUniform1i(uniform.post_processing.texture_skybox, 0);
    glUniform1i(uniform.post_processing.texture_world_pos, 1);
    glUniform1i(uniform.post_processing.texture_world_normal, 2);
    glUniform1i(uniform.post_processing.texture_world_albedo_specular, 3);
    glUniform1i(uniform.post_processing.texture_hud, 4);
    glUniform1ui(uniform.post_processing.time, ((u32)(render->time) & 0x1ff) + 1);
    glUniformMatrix4fv(uniform.post_processing.mat_projection, 1, GL_FALSE,
            (GLfloat*)&player.camera.projection.projection);
    glUniform1f(uniform.post_processing.camera_far, player.camera.far);
    glUniform1f(uniform.post_processing.camera_near, player.camera.near);
    glUniform3fv(uniform.post_processing.ssao_sample, 64, (GLfloat*)ssao_buf.sample);

    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
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

    if (settings_init() != FSL_ERR_SUCCESS)
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

    if (gui_init(render->size) != FSL_ERR_SUCCESS)
        goto cleanup;

    super_debugger_init(render->size);

    /* ---- end set graphics ------------------------------------------------ */

    player_init(&player, "Lily");
    input_init();
    bind_shader_uniforms();

#if MODE_INTERNAL_SKIP_TITLE_MENU
    goto section_gameplay;
#endif /* MODE_INTERNAL_SKIP_TITLE_MENU */

section_menu_title:

    enable_cursor;

    while (fsl_engine_running(&callback_framebuffer_size))
    {
        input_update(&player);
        fsl_ui_start(TRUE);

        gui_menu_title_draw();

        if (core.flag.super_debug)
            super_debugger_draw(render->size, 0);

        fsl_ui_stop();

        if (core.request.world_load)
        {
            core.request.world_load = FALSE;
            goto section_gameplay;
        }
    }

section_menu_pause:

    while (fsl_engine_running(&callback_framebuffer_size))
    {
        input_update(&player);
        fsl_ui_start(TRUE);

        gui_menu_pause_draw();

        if (core.flag.super_debug)
            super_debugger_draw(render->size, 0);

        fsl_ui_stop();

        if (core.request.menu_title_enter)
        {
            core.request.menu_title_enter = FALSE;
            core.flag.world_loaded = FALSE;
            goto section_menu_title;
        }

        if (core.request.menu_back)
        {
            core.request.menu_back = FALSE;
            core.flag.paused = FALSE;
            center_cursor;
            enable_cursor;
            goto section_gameplay;
        }
    }

section_gameplay:

    if (!core.flag.world_loaded &&
            world_init("Poop Consistency Tester", 0, &player) != FSL_ERR_SUCCESS)
            goto cleanup;

    while (fsl_engine_running(&callback_framebuffer_size))
    {
        input_update(&player);
        world_update(&player);
        world_draw();

        fsl_process_screenshot_request(GAME_DIR_NAME_SCREENSHOTS, world.name);
        fsl_limit_framerate(settings.target_fps, render->time);

        if (core.request.menu_back)
        {
            core.request.menu_back = FALSE;
            core.flag.paused = TRUE;
            goto section_menu_pause;
        }
    }

cleanup:

    gui_free();
    assets_free();
    chunking_free();
    fsl_engine_close();
    return *GAME_ERR;
}
