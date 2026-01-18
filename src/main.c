#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <engine/h/common.h>
#include <engine/h/core.h>
#include <engine/h/diagnostics.h>
#include <engine/h/input.h>
#include <engine/h/logger.h>
#include <engine/h/math.h>
#include <engine/h/platform.h>
#include <engine/h/shaders.h>
#include <engine/h/string.h>
#include <engine/h/text.h>
#include <engine/h/time.h>
#include <engine/h/ui.h>

#include "h/main.h"

#include "h/assets.h"
#include "h/chunking.h"
#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/input.h"
#include "h/player.h"
#include "h/terrain.h"
#include "h/world.h"

u32 *const GAME_ERR = (u32*)&engine_err;
struct Core core = {0};
struct Settings settings = {0};
static struct Uniform uniform = {0};
u8 debug_mode[DEBUG_MODE_COUNT] = {0};
static Mesh mesh[MESH_COUNT] = {0};
static FBO fbo[FBO_COUNT] = {0};
Projection projection_world = {0};
Projection projection_hud = {0};

Font *font[FONT_COUNT] =
{
    [FONT_REG] =        &engine_font[ENGINE_FONT_DEJAVU_SANS],
    [FONT_REG_BOLD] =   &engine_font[ENGINE_FONT_DEJAVU_SANS_BOLD],
    [FONT_MONO] =       &engine_font[ENGINE_FONT_DEJAVU_SANS_MONO],
    [FONT_MONO_BOLD] =  &engine_font[ENGINE_FONT_DEJAVU_SANS_MONO_BOLD],
};

static Player player =
{
    .name = "Lily",
    .size = {0.6f, 0.6f, 1.8f},
    .eye_height = PLAYER_EYE_HEIGHT,
    .camera_mode = PLAYER_CAMERA_MODE_1ST_PERSON,
    .camera_distance = SET_CAMERA_DISTANCE_MAX,

    .container_state = 0,
    .hotbar_slots[0] = BLOCK_GRASS,
    .hotbar_slots[1] = BLOCK_DIRT,
    .hotbar_slots[2] = BLOCK_STONE,
    .hotbar_slots[3] = BLOCK_SAND,
    .hotbar_slots[4] = BLOCK_GLASS,
    .hotbar_slots[5] = BLOCK_WOOD_OAK_LOG,
    .hotbar_slots[6] = BLOCK_WOOD_BIRCH_LOG,
    .hotbar_slots[7] = BLOCK_WOOD_CHERRY_LOG,
};

static struct /* skybox_data */
{
    f32 time;
    v3f32 sun_rotation;
    v3f32 sky_color;
    v3f32 horizon_color;
    v3f32 sky_light;
    v3f32 moon_light;
} skybox_data;

static void callback_framebuffer_size(GLFWwindow* window, int width, int height);
static void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods);
static void callback_scroll(GLFWwindow *window, double xoffset, double yoffset);

static void bind_shader_uniforms(void);
static void generate_standard_meshes(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
static u32 settings_init(void);

void settings_update(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief update 'render.time', 'render.delta' of currently bound render
 *  and cap framerate to 'settings.target_fps' if 'core.flag.fps_cap' is set.
 */
void time_update(b8 fps_cap, u64 fps_target);

static void draw_everything(void);

static void callback_framebuffer_size(GLFWwindow* window, int width, int height)
{
    (void)window;

    engine_update_render_settings((i32)width, (i32)height);

    player.camera.ratio = (f32)width / (f32)height;
    player.camera_hud.ratio = (f32)width / (f32)height;

    fbo_realloc(&fbo[FBO_SKYBOX], FALSE, 4);
    fbo_realloc(&fbo[FBO_WORLD], FALSE, 4);
    fbo_realloc(&fbo[FBO_WORLD_MSAA], TRUE, 4);
    fbo_realloc(&fbo[FBO_HUD], FALSE, 4);
    fbo_realloc(&fbo[FBO_HUD_MSAA], TRUE, 4);
    fbo_realloc(&fbo[FBO_POST_PROCESSING], FALSE, 4);
}

static void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void callback_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;

    if (player.flag & FLAG_PLAYER_ZOOMER)
        player.camera.zoom =
            clamp_f64(player.camera.zoom + yoffset * CAMERA_ZOOM_SPEED, 0.0f, CAMERA_ZOOM_MAX);
    else
    {
        player.hotbar_slot_selected -= (i64)yoffset;
        if (player.hotbar_slot_selected >= PLAYER_HOTBAR_SLOTS_MAX)
            player.hotbar_slot_selected = 0;
        else if (player.hotbar_slot_selected < 0)
            player.hotbar_slot_selected = PLAYER_HOTBAR_SLOTS_MAX - 1;
    }
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
    str *settings_file_contents = stringf(
            "%s = %d\n"
            "%s = %d\n"
            "%s = %d\n"
            "%s = %d\n",
            tokens[0], SET_MOUSE_SENSITIVITY_DEFAULT,
            tokens[1], SET_FOV_DEFAULT,
            tokens[2], SET_RENDER_DISTANCE_DEFAULT,
            tokens[3], TARGET_FPS_DEFAULT);

    if (is_dir_exists(GAME_DIR_NAME_CONFIG, TRUE) != ERR_SUCCESS)
        return *GAME_ERR;

    if (is_file_exists(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS, FALSE) != ERR_SUCCESS)
    {
        write_file(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS,
                1, strlen(settings_file_contents),
                settings_file_contents, TRUE, TRUE);
    }

    settings_file_contents = NULL;
    get_file_contents(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS,
            (void*)&settings_file_contents, 1, TRUE);
    if (*GAME_ERR != ERR_SUCCESS)
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
    settings.target_fps = 3;
    settings.fov = SET_FOV_DEFAULT;
    settings.anti_aliasing = TRUE;

    mem_free((void*)&settings_file_contents, strlen(settings_file_contents),
            "settings_init().settings_file_contents");

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;
}

void settings_update(void)
{
    static u64 time_next = 0;
    if (time_next < render->time)
    {
        time_next += SEC2NSEC / SET_TEXT_REFRESH_INTERVAL;
        settings.fps = 1 / ((f64)render->time_delta * NSEC2SEC);
    }
}

void time_update(b8 fps_cap, u64 fps_target)
{
    static u64 time_next = 0;
    time_next += SEC2NSEC / clamp_u64(fps_target, TARGET_FPS_MIN, TARGET_FPS_MAX);

    render->time = get_time_nsec();
    if (fps_cap && render->time < time_next)
        sleep_nsec(time_next - render->time);
    else time_next = render->time;

    render->time_delta = get_time_delta_nsec();
}

static void bind_shader_uniforms(void)
{
    uniform.defaults.offset =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "offset");
    uniform.defaults.scale =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "scale");
    uniform.defaults.mat_rotation =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "mat_rotation");
    uniform.defaults.mat_perspective =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "mat_perspective");
    uniform.defaults.sun_rotation =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "sun_rotation");
    uniform.defaults.sky_color =
        glGetUniformLocation(shader[SHADER_DEFAULT].id, "sky_color");

    uniform.skybox.texture_scale =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "texture_scale");
    uniform.skybox.mat_translation =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "mat_translation");
    uniform.skybox.mat_rotation =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "mat_rotation");
    uniform.skybox.mat_sun_rotation =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "mat_sun_rotation");
    uniform.skybox.mat_orientation =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "mat_orientation");
    uniform.skybox.mat_projection =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "mat_projection");
    uniform.skybox.texture_sky =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "texture_sky");
    uniform.skybox.texture_horizon =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "texture_horizon");
    uniform.skybox.texture_stars =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "texture_stars");
    uniform.skybox.texture_sun =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "texture_sun");
    uniform.skybox.sun_rotation =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "sun_rotation");
    uniform.skybox.sky_color =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "sky_color");
    uniform.skybox.horizon_color =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "horizon_color");
    uniform.skybox.render_layer =
        glGetUniformLocation(shader[SHADER_SKYBOX].id, "render_layer");

    uniform.gizmo.ndc_scale =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "ndc_scale");
    uniform.gizmo.mat_translation =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "mat_translation");
    uniform.gizmo.mat_rotation =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "mat_rotation");
    uniform.gizmo.mat_orientation =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "mat_orientation");
    uniform.gizmo.mat_projection =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "mat_projection");
    uniform.gizmo.color =
        glGetUniformLocation(shader[SHADER_GIZMO].id, "gizmo_color");

    uniform.gizmo_chunk.gizmo_offset =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "gizmo_offset");
    uniform.gizmo_chunk.render_size =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "render_size");
    uniform.gizmo_chunk.chunk_buf_diameter =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "chunk_buf_diameter");
    uniform.gizmo_chunk.mat_translation =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "mat_translation");
    uniform.gizmo_chunk.mat_rotation =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "mat_rotation");
    uniform.gizmo_chunk.mat_orientation =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "mat_orientation");
    uniform.gizmo_chunk.mat_projection =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "mat_projection");
    uniform.gizmo_chunk.camera_position =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "camera_position");
    uniform.gizmo_chunk.time =
        glGetUniformLocation(shader[SHADER_GIZMO_CHUNK].id, "time");

    uniform.post_processing.time =
        glGetUniformLocation(shader[SHADER_POST_PROCESSING].id, "time");

    uniform.voxel.mat_perspective =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "mat_perspective");
    uniform.voxel.camera_position =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "camera_position");
    uniform.voxel.sun_rotation =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "sun_rotation");
    uniform.voxel.sky_light =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "sky_light");
    uniform.voxel.moon_light =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "moon_light");
    uniform.voxel.chunk_position =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "chunk_position");
    uniform.voxel.color =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "voxel_color");
    uniform.voxel.opacity =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "opacity");
    uniform.voxel.flashlight_position =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "flashlight_position");
    uniform.voxel.toggle_flashlight =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "toggle_flashlight");
    uniform.voxel.render_distance =
        glGetUniformLocation(shader[SHADER_VOXEL].id, "render_distance");

    uniform.bounding_box.mat_perspective =
        glGetUniformLocation(shader[SHADER_BOUNDING_BOX].id, "mat_perspective");
    uniform.bounding_box.position =
        glGetUniformLocation(shader[SHADER_BOUNDING_BOX].id, "position");
    uniform.bounding_box.size =
        glGetUniformLocation(shader[SHADER_BOUNDING_BOX].id, "size");
    uniform.bounding_box.color =
        glGetUniformLocation(shader[SHADER_BOUNDING_BOX].id, "box_color");
}

static void generate_standard_meshes(void)
{
    const u32 VBO_LEN_SKYBOX =  120;
    const u32 EBO_LEN_SKYBOX =  36;
    const u32 VBO_LEN_COH =     24;
    const u32 EBO_LEN_COH =     36;
    const u32 VBO_LEN_PLAYER =  216;
    const u32 VBO_LEN_GIZMO =   51;
    const u32 EBO_LEN_GIZMO =   90;

    GLfloat vbo_data_skybox[] =
    {
        -1.0f, -1.0f, -1.0f, 3.0f, 2.0f,
        -1.0f, -1.0f, 1.0f, 3.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, 4.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 4.0f, 2.0f,

        1.0f, 1.0f, -1.0f, 1.0f, 2.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 2.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 2.0f, 2.0f,

        1.0f, -1.0f, -1.0f, 2.0f, 2.0f,
        1.0f, -1.0f, 1.0f, 2.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 3.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 3.0f, 2.0f,

        -1.0f, 1.0f, -1.0f, 0.0f, 2.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 2.0f,

        -1.0f, 1.0f, -1.0f, 1.0f, 3.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 2.0f,
        1.0f, -1.0f, -1.0f, 2.0f, 2.0f,
        -1.0f, -1.0f, -1.0f, 2.0f, 3.0f,

        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 2.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 2.0f, 1.0f,
    };

    GLuint ebo_data_skybox[] =
    {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };

    GLfloat vbo_data_coh[] =
    {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    GLuint ebo_data_coh[] =
    {
        0, 4, 5, 5, 1, 0,
        1, 5, 7, 7, 3, 1,
        3, 7, 6, 6, 2, 3,
        2, 6, 4, 4, 0, 2,
        4, 6, 7, 7, 5, 4,
        0, 1, 3, 3, 2, 0,
    };

    GLfloat vbo_data_player[] =
    {
        /* pos            normals */
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, /* px */
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, /* nx */
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, /* py */
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, /* ny */
        1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, /* pz */
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, /* nz */
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
    };

    const GLfloat THIC = 0.06f;
    GLfloat vbo_data_gizmo[] =
    {
        0.0f, 0.0f, 0.0f,
        THIC, THIC, 0.0f,
        THIC, 0.0f, THIC,
        0.0f, THIC, THIC,
        1.0f, 0.0f, 0.0f,
        1.0f, THIC, 0.0f,
        1.0f, 0.0f, THIC,
        0.0f, 1.0f, 0.0f,
        THIC, 1.0f, 0.0f,
        0.0f, 1.0f, THIC,
        0.0f, 0.0f, 1.0f,
        THIC, 0.0f, 1.0f,
        0.0f, THIC, 1.0f,
        THIC, THIC, THIC,
        1.0f, THIC, THIC,
        THIC, 1.0f, THIC,
        THIC, THIC, 1.0f,
    };

    GLuint ebo_data_gizmo[] =
    {
        0, 2, 6, 6, 4, 0,
        0, 4, 5, 5, 1, 0,
        1, 5, 14, 14, 13, 1,
        2, 13, 14, 14, 6, 2,
        4, 6, 14, 14, 5, 4,

        0, 7, 9, 9, 3, 0,
        0, 1, 8, 8, 7, 0,
        1, 13, 15, 15, 8, 1,
        3, 9, 15, 15, 13, 3,
        7, 8, 15, 15, 9, 7,

        0, 3, 12, 12, 10, 0,
        0, 10, 11, 11, 2, 0,
        2, 11, 16, 16, 13, 2,
        13, 16, 12, 12, 3, 13,
        10, 12, 16, 16, 11, 10,
    };

    if (mesh_generate(&mesh[MESH_SKYBOX], &attrib_vec3_vec2, GL_STATIC_DRAW,
                VBO_LEN_SKYBOX, EBO_LEN_SKYBOX,
                vbo_data_skybox, ebo_data_skybox) != ERR_SUCCESS)
    {
        LOG_MESH_GENERATE(ERR_MESH_GENERATION_FAIL, TRUE, "Skybox");
        goto cleanup;
    }
    LOG_MESH_GENERATE(ERR_SUCCESS, TRUE, "Skybox");

    if (mesh_generate(&mesh[MESH_CUBE_OF_HAPPINESS], &attrib_vec3, GL_STATIC_DRAW,
                VBO_LEN_COH, EBO_LEN_COH,
                vbo_data_coh, ebo_data_coh) != ERR_SUCCESS)
    {
        LOG_MESH_GENERATE(ERR_MESH_GENERATION_FAIL, TRUE, "Cube of Happiness");
        goto cleanup;
    }
    LOG_MESH_GENERATE(ERR_SUCCESS, TRUE, "Cube of Happiness");

    if (mesh_generate(&mesh[MESH_PLAYER], &attrib_vec3_vec3, GL_STATIC_DRAW,
                VBO_LEN_PLAYER, 0, vbo_data_player, NULL) != ERR_SUCCESS)
    {
        LOG_MESH_GENERATE(ERR_MESH_GENERATION_FAIL, TRUE, "Player");
        goto cleanup;
    }
    LOG_MESH_GENERATE(ERR_SUCCESS, TRUE, "Player");

    if (mesh_generate(&mesh[MESH_GIZMO], &attrib_vec3, GL_STATIC_DRAW,
                VBO_LEN_GIZMO, EBO_LEN_GIZMO,
                vbo_data_gizmo, ebo_data_gizmo) != ERR_SUCCESS)
    {
        LOG_MESH_GENERATE(ERR_MESH_GENERATION_FAIL, TRUE, "Gizmo");
        goto cleanup;
    }
    LOG_MESH_GENERATE(ERR_SUCCESS, TRUE, "Gizmo");

    *GAME_ERR = ERR_SUCCESS;
    return;

cleanup:

    mesh_free(&mesh[MESH_PLAYER]);
}

static void draw_everything(void)
{
    /* ---- draw skybox ----------------------------------------------------- */

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_SKYBOX].fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f32 delay_in_hours = 6.0f;
    skybox_data.time = fmodf((f32)world.tick / SET_DAY_TICKS_MAX, 1.0f);
    skybox_data.time = fmodf(skybox_data.time * 2.0f - delay_in_hours / 12.0f, 2.0f);
    skybox_data.sun_rotation =
        (v3f32){
            cos(skybox_data.time * PI),
            cos(skybox_data.time * PI) * 0.3f,
            sin(skybox_data.time * PI),
        };

    f32 sun_time = skybox_data.time * PI;

    f64 mid_day =       (sin(sun_time) + 1.0) / 2.0;
    mid_day =           pow(sin((PI / 2.0) * mid_day), 2.0);
    mid_day =           pow(sin((PI / 2.0) * mid_day), 2.0);

    f64 burn_cold =     pow((sin((PI / 2.0) * sin(sun_time + (PI / 2.0))) + 1.0) / 2.0, 24.0);
    burn_cold +=        pow((sin((PI / 2.0) * sin(sun_time - (PI / 2.0))) + 1.0) / 2.0, 24.0);

    f64 burn =          pow((sin(sun_time + (PI / 2.0)) + 1.0) / 2.0, 64.0);
    burn +=             pow((sin(sun_time - (PI / 2.0)) + 1.0) / 2.0, 64.0);

    f64 burn_boost =    pow(sin(sun_time + (PI / 2.0)), 128.0);
    burn_boost +=       pow(sin(sun_time - (PI / 2.0)), 128.0);

    f64 mid_night =     pow((sin((PI / 2.0) * sin(sun_time + PI)) + 1.0) / 2.0, 4.0);

    skybox_data.sky_color = (v3f32){
        (mid_day * 171.0f + mid_night * 1.0f + burn_cold * 8.0f) / 0xff,
        (mid_day * 229.0f + mid_night * 4.0f + burn_cold * 4.0f) / 0xff,
        (mid_day * 255.0f + mid_night * 14.0f + burn_cold * 18.0f) / 0xff,
    };

    skybox_data.horizon_color = (v3f32){
        (mid_day * 224.0f + mid_night * 1.0f + burn_cold * 8.0f + burn * 92.0f + burn_boost * 116.0f) / 0xff,
        (mid_day * 244.0f + mid_night * 4.0f + burn_cold * 4.0f + burn * 5.0f + burn_boost * 77.0f) / 0xff,
        (mid_day * 255.0f + mid_night * 14.0f + burn_cold * 18.0f) / 0xff,
    };

    skybox_data.sky_light = (v3f32){
        skybox_data.sky_color.x + skybox_data.horizon_color.x,
        skybox_data.sky_color.y + skybox_data.horizon_color.y,
        skybox_data.sky_color.z + skybox_data.horizon_color.z,
    };

    skybox_data.moon_light = (v3f32){mid_night, mid_night, mid_night};

    m4f32 translation =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    m4f32 rotation =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    glUseProgram(shader[SHADER_SKYBOX].id);

    glUniform1f(uniform.skybox.texture_scale, 0.25f);
    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE, (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_rotation, 1, GL_FALSE,
            (GLfloat*)&projection_world.rotation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE, (GLfloat*)&rotation);
    glUniformMatrix4fv(uniform.skybox.mat_orientation, 1, GL_FALSE,
            (GLfloat*)&projection_world.orientation);
    glUniformMatrix4fv(uniform.skybox.mat_projection, 1, GL_FALSE,
            (GLfloat*)&projection_world.projection);
    glUniform3fv(uniform.skybox.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.skybox.sky_color, 1, (GLfloat*)&skybox_data.sky_color);
    glUniform3fv(uniform.skybox.horizon_color, 1, (GLfloat*)&skybox_data.horizon_color);
    glUniform1i(uniform.skybox.render_layer, 0);

    glUniform1i(uniform.skybox.texture_sky, 0);
    glUniform1i(uniform.skybox.texture_horizon, 1);
    glUniform1i(uniform.skybox.texture_stars, 2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_SKYBOX_VAL].id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_SKYBOX_HORIZON].id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_SKYBOX_STARS].id);
    glBindVertexArray(mesh[MESH_SKYBOX].vao);
    glDrawElements(GL_TRIANGLES, mesh[MESH_SKYBOX].ebo_len, GL_UNSIGNED_INT, 0);

    /* ---- draw sun -------------------------------------------------------- */

    if (settings.anti_aliasing)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_WORLD_MSAA].fbo);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_WORLD].fbo);

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

    f32 angle = skybox_data.time * PI + 90.0f * DEG2RAD;
    rotation = (m4f32){
        cosf(PI / 2.0f), -sinf(PI / 2.0f), 0.0f, 0.0f,
        sinf(PI / 2.0f), cosf(PI / 2.0f), 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    rotation = matrix_multiply(rotation,
            (m4f32){
            cosf(angle), 0.0f, sinf(angle), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -sinf(angle), 0.0f, cosf(angle), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            });

    glUniform1f(uniform.skybox.texture_scale, 1.0f);
    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE,
            (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE,
            (GLfloat*)&rotation);
    glUniform1i(uniform.skybox.render_layer, 1);

    glUniform1i(uniform.skybox.texture_sun, 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_SUN].id);
    glBindVertexArray(engine_mesh_unit.vao);
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

    angle = skybox_data.time * PI - 90.0f * DEG2RAD;
    rotation = (m4f32){
        cosf(PI / 2.0f), -sinf(PI / 2.0f), 0.0f, 0.0f,
        sinf(PI / 2.0f), cosf(PI / 2.0f), 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    rotation = matrix_multiply(rotation,
            (m4f32){
            cosf(angle), 0.0f, sinf(angle), 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -sinf(angle), 0.0f, cosf(angle), 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            });

    glUniformMatrix4fv(uniform.skybox.mat_translation, 1, GL_FALSE,
            (GLfloat*)&translation);
    glUniformMatrix4fv(uniform.skybox.mat_sun_rotation, 1, GL_FALSE,
            (GLfloat*)&rotation);
    glUniform1i(uniform.skybox.render_layer, 2);

    glBindTexture(GL_TEXTURE_2D, texture[TEXTURE_MOON].id);
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

    glUseProgram(shader[SHADER_VOXEL].id);
    glUniformMatrix4fv(uniform.voxel.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection_world.perspective);
    glUniform3f(uniform.voxel.camera_position,
            player.camera.pos.x, player.camera.pos.y, player.camera.pos.z);
    glUniform3f(uniform.voxel.flashlight_position,
            player.pos.x, player.pos.y, player.pos.z + player.eye_height);
    glUniform3fv(uniform.voxel.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform.voxel.sky_light, 1, (GLfloat*)&skybox_data.sky_light);
    glUniform3fv(uniform.voxel.moon_light, 1, (GLfloat*)&skybox_data.moon_light);
    glUniform1f(uniform.voxel.toggle_flashlight, player.flag & FLAG_PLAYER_FLASHLIGHT ? 1.0f : 0.0f);
    glUniform1i(uniform.voxel.render_distance, settings.render_distance * CHUNK_DIAMETER);

    f32 opacity = 1.0f;
    if (debug_mode[DEBUG_MODE_TRANS_BLOCKS])
        opacity = 0.7f;

    glUniform1f(uniform.voxel.opacity, opacity);

    static Chunk ***cursor = NULL;
    static Chunk ***end = NULL;
    static Chunk *chunk = NULL;
    cursor = CHUNK_ORDER + CHUNKS_MAX[settings.render_distance] - 1;
    for (; cursor >= CHUNK_ORDER; --cursor)
    {
        chunk = **cursor;
        if (!chunk || !(chunk->flag & FLAG_CHUNK_RENDER))
                continue;

        glUniform3f(uniform.voxel.chunk_position,
                (f32)(chunk->pos.x * CHUNK_DIAMETER),
                (f32)(chunk->pos.y * CHUNK_DIAMETER),
                (f32)(chunk->pos.z * CHUNK_DIAMETER));

        glBindVertexArray(chunk->vao);
        glDrawArrays(GL_POINTS, 0, chunk->vbo_len);
    }

    /* ---- draw player ----------------------------------------------------- */

    if (player.camera_mode != PLAYER_CAMERA_MODE_1ST_PERSON)
    {

        glUseProgram(shader[SHADER_DEFAULT].id);
        glUniform3fv(uniform.defaults.scale, 1, (GLfloat*)&player.size);
        glUniform3f(uniform.defaults.offset,
                player.pos.x, player.pos.y, player.pos.z);
        glUniformMatrix4fv(uniform.defaults.mat_rotation, 1, GL_FALSE,
                (GLfloat*)(f32[]){
                player.cos_yaw, player.sin_yaw, 0.0f, 0.0f,
                -player.sin_yaw, player.cos_yaw, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f});
        glUniformMatrix4fv(uniform.defaults.mat_perspective, 1, GL_FALSE,
                (GLfloat*)&projection_world.perspective);
        glUniform3fv(uniform.defaults.sun_rotation, 1,
                (GLfloat*)&skybox_data.sun_rotation);
        glUniform3fv(uniform.defaults.sky_color, 1,
                (GLfloat*)&skybox_data.sky_color);

        glBindVertexArray(mesh[MESH_PLAYER].vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh[MESH_PLAYER].vbo_len);
    }

    /* ---- draw player target bounding box --------------------------------- */

    glUseProgram(shader[SHADER_BOUNDING_BOX].id);
    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection_world.perspective);

    if (core.flag.parse_target && core.flag.hud &&
            chunk_tab[chunk_tab_index] &&
            chunk_tab[chunk_tab_index]->block
            [(i64)player.target.z - chunk_tab[chunk_tab_index]->pos.z * CHUNK_DIAMETER]
            [(i64)player.target.y - chunk_tab[chunk_tab_index]->pos.y * CHUNK_DIAMETER]
            [(i64)player.target.x - chunk_tab[chunk_tab_index]->pos.x * CHUNK_DIAMETER])
    {
        glUniform3f(uniform.bounding_box.position,
                (f32)(player.target.x),
                (f32)(player.target.y),
                (f32)(player.target.z));
        glUniform3f(uniform.bounding_box.size, 1.0f, 1.0f, 1.0f);
        glUniform4f(uniform.bounding_box.color, 0.0f, 0.0f, 0.0f, 1.0f);

        glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    /* ---- draw player chunk bounding box ---------------------------------- */

    if (debug_mode[DEBUG_MODE_CHUNK_BOUNDS])
    {
        glUniform3f(uniform.bounding_box.position,
                (f32)(player.chunk.x * CHUNK_DIAMETER),
                (f32)(player.chunk.y * CHUNK_DIAMETER),
                (f32)(player.chunk.z * CHUNK_DIAMETER));
        glUniform3f(uniform.bounding_box.size,
                CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);
        glUniform4f(uniform.bounding_box.color, 0.9f, 0.6f, 0.3f, 1.0f);

        glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    /* ---- draw player bounding box ---------------------------------------- */

    if (debug_mode[DEBUG_MODE_BOUNDING_BOXES])
    {
        glUniform3f(uniform.bounding_box.position,
                player.bbox.pos.x, player.bbox.pos.y, player.bbox.pos.z);
        glUniform3f(uniform.bounding_box.size,
                player.bbox.size.x, player.bbox.size.y, player.bbox.size.z);
        glUniform4f(uniform.bounding_box.color, 1.0f, 0.3f, 0.2f, 1.0f);

        glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
    }

    /* ---- draw player chunk queue visualizer ------------------------------ */

    if (debug_mode[DEBUG_MODE_CHUNK_QUEUE_VISUALIZER])
    {
        glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
                (GLfloat*)&projection_world.perspective);
        glUniform3f(uniform.bounding_box.size,
                CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);

        cursor = CHUNK_ORDER;
        end = CHUNK_ORDER + CHUNK_QUEUE[0].size;
        for (; cursor < end; ++cursor)
        {
            chunk = **cursor;
            if (!chunk || !(chunk->flag & FLAG_CHUNK_QUEUED)) continue;
            glUniform3f(uniform.bounding_box.position,
                    (f32)(chunk->pos.x * CHUNK_DIAMETER),
                    (f32)(chunk->pos.y * CHUNK_DIAMETER),
                    (f32)(chunk->pos.z * CHUNK_DIAMETER));

            glUniform4f(uniform.bounding_box.color, 0.6f, 0.9f, 0.3f, 1.0f);
            glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
            glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
        }

        if (CHUNK_QUEUE[1].size)
        {
            end += CHUNK_QUEUE[1].size;
            for (; cursor < end; ++cursor)
            {
                chunk = **cursor;
                if (!(chunk->flag & FLAG_CHUNK_QUEUED)) continue;
                glUniform3f(uniform.bounding_box.position,
                        (f32)(chunk->pos.x * CHUNK_DIAMETER),
                        (f32)(chunk->pos.y * CHUNK_DIAMETER),
                        (f32)(chunk->pos.z * CHUNK_DIAMETER));

                glUniform4f(uniform.bounding_box.color, 0.9f, 0.6f, 0.3f, 1.0f);
                glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
                glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
            }
        }

        if (CHUNK_QUEUE[2].size)
        {
            end += CHUNK_QUEUE[2].size;
            for (; cursor < end; ++cursor)
            {
                chunk = **cursor;
                if (!chunk || !(chunk->flag & FLAG_CHUNK_QUEUED)) continue;
                glUniform3f(uniform.bounding_box.position,
                        (f32)(chunk->pos.x * CHUNK_DIAMETER),
                        (f32)(chunk->pos.y * CHUNK_DIAMETER),
                        (f32)(chunk->pos.z * CHUNK_DIAMETER));

                glUniform4f(uniform.bounding_box.color, 0.9f, 0.3f, 0.3f, 1.0f);
                glBindVertexArray(mesh[MESH_CUBE_OF_HAPPINESS].vao);
                glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);
            }
        }
    }

    if (settings.anti_aliasing)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[FBO_WORLD_MSAA].fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[FBO_WORLD].fbo);
        glBlitFramebuffer(0, 0, render->size.x, render->size.y, 0, 0,
                render->size.x, render->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    /* ---- draw hud gizmo -------------------------------------------------- */

    if (settings.anti_aliasing)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_HUD_MSAA].fbo);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_HUD].fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (core.flag.hud && core.flag.debug)
    {
        glUseProgram(shader[SHADER_GIZMO].id);

        glUniform2fv(uniform.gizmo.ndc_scale, 1, (GLfloat*)&render->ndc_scale);
        glUniformMatrix4fv(uniform.gizmo.mat_translation, 1, GL_FALSE,
                (GLfloat*)&projection_hud.target);
        glUniformMatrix4fv(uniform.gizmo.mat_rotation, 1, GL_FALSE,
                (GLfloat*)&projection_hud.rotation);
        glUniformMatrix4fv(uniform.gizmo.mat_orientation, 1, GL_FALSE,
                (GLfloat*)&projection_hud.orientation);
        glUniformMatrix4fv(uniform.gizmo.mat_projection, 1, GL_FALSE,
                (GLfloat*)&projection_hud.projection);

        glBindVertexArray(mesh[MESH_GIZMO].vao);
        glUniform3f(uniform.gizmo.color, 1.0f, 0.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, (void*)0);
        glUniform3f(uniform.gizmo.color, 0.0f, 1.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, (void*)(30 * sizeof(GLuint)));
        glUniform3f(uniform.gizmo.color, 0.0f, 0.0f, 1.0f);
        glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, (void*)(60 * sizeof(GLuint)));
    }

    /* ---- draw hud chunk gizmo -------------------------------------------- */

    if (debug_mode[DEBUG_MODE_CHUNK_GIZMO] && core.flag.hud)
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader[SHADER_GIZMO_CHUNK].id);

        glUniform1f(uniform.gizmo_chunk.gizmo_offset, (f32)settings.chunk_buf_radius + 0.5f);
        glUniform2iv(uniform.gizmo_chunk.render_size, 1, (GLint*)&render->size);
        glUniform1i(uniform.gizmo_chunk.chunk_buf_diameter, settings.chunk_buf_diameter);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_translation,
                1, GL_FALSE, (GLfloat*)&projection_hud.target);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_rotation,
                1, GL_FALSE, (GLfloat*)&projection_hud.rotation);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_orientation,
                1, GL_FALSE, (GLfloat*)&projection_hud.orientation);

        glUniformMatrix4fv(uniform.gizmo_chunk.mat_projection,
                1, GL_FALSE, (GLfloat*)&projection_hud.projection);

        v3f32 camera_position =
        {
            -player.camera.cos_yaw * player.camera.cos_pitch,
            player.camera.sin_yaw * player.camera.cos_pitch,
            player.camera.sin_pitch,
        };

        glUniform3fv(uniform.gizmo_chunk.camera_position, 1, (GLfloat*)&camera_position);
        glUniform1f(uniform.gizmo_chunk.time, render->time);

        glDisable(GL_BLEND);
        glBindVertexArray(chunk_gizmo_loaded_vao);
        glDrawArrays(GL_POINTS, 0, settings.chunk_buf_volume);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(chunk_gizmo_render_vao);
        glDrawArrays(GL_POINTS, 0, settings.chunk_buf_volume);
        glEnable(GL_BLEND);
    }

    if (settings.anti_aliasing)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[FBO_HUD_MSAA].fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[FBO_HUD].fbo);
        glBlitFramebuffer(0, 0, render->size.x, render->size.y, 0, 0,
                render->size.x, render->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    /* ---- draw ui --------------------------------------------------------- */

    if (core.flag.hud)
    {
        ui_start(NULL, FALSE, TRUE);

        if (!core.flag.debug)
            ui_draw(texture[TEXTURE_CROSSHAIR], render->size.x / 2, render->size.y / 2,
                    8, 8, 0.0f, 0.0f, 0, 0, 0xffffffff);

        ui_draw(texture[TEXTURE_ITEM_BAR], render->size.x / 2, render->size.y,
                texture[TEXTURE_ITEM_BAR].size.x, texture[TEXTURE_ITEM_BAR].size.y,
                84.5f, 18.0f, 1, 1, 0xffffffff);
        ui_stop();
    }
    else
    {
        ui_start(NULL, FALSE, TRUE);
        ui_stop();
    }

    /* ---- draw engine ui -------------------------------------------------- */

    if (core.flag.super_debug)
    {
        //ui_render();
        ui_start(NULL, TRUE, TRUE);
        ui_draw_nine_slice(engine_texture[ENGINE_TEXTURE_PANEL_ACTIVE],
                10, 10, 8.0f, 400, render->size.y - 20, 0, 0, -1, -1, 0xffffffef);
        ui_stop();
    }
    else /* ---- clear ui buffer -------------------------------------------- */
    {
        ui_start(NULL, TRUE, TRUE);
        ui_stop();
    }

    /* ---- draw debug info ------------------------------------------------- */

    text_start(font[FONT_MONO_BOLD], settings.font_size, 0, NULL, TRUE);

    text_push(stringf("FPS         [%u]\n", settings.fps),
            (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0,
            settings.fps > 60 ? COLOR_TEXT_MOSS : COLOR_DIAGNOSTIC_ERROR);

    text_render(TRUE, TEXT_COLOR_SHADOW);

    if (core.flag.hud && core.flag.debug)
    {
        text_push(stringf("\n"
                    "TIME        [%.2lf]\n"
                    "CLOCK       [%02"PRIu64":%02"PRIu64"]\n"
                    "DAYS        [%"PRIu64"]\n",
                    (f64)render->time * NSEC2SEC,
                    (world.tick % SET_DAY_TICKS_MAX) / 1000,
                    ((world.tick * 60) / 1000) % 60,
                    world.days),
                (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0,
                COLOR_TEXT_MOSS);

        text_push(stringf(
                    "XYZ         [%5.2lf %5.2lf %5.2lf]\n"
                    "BLOCK       [%.0lf %.0lf %.0lf]\n"
                    "CHUNK       [%d %d %d]\n"
                    "PITCH/YAW   [%5.2f][%5.2f]\n"
                    "ACCELERATION[%5.2f %5.2f %5.2f]\n"
                    "VELOCITY    [%5.2f %5.2f %5.2f]\n"
                    "SPEED       [%5.2f]\n",
                    player.pos.x, player.pos.y, player.pos.z,
                    floor(player.pos.x),
                    floor(player.pos.y),
                    floor(player.pos.z),
                    player.chunk.x, player.chunk.y, player.chunk.z,
                    player.pitch, player.yaw,
                    player.acceleration.x, player.acceleration.y, player.acceleration.z,
                    player.velocity.x, player.velocity.y, player.velocity.z,
                    player.speed),
                (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0,
                COLOR_TEXT_DEFAULT);

        text_push(stringf(
                    "OVERFLOW    [%s %s %s]",
                    (player.flag & FLAG_PLAYER_OVERFLOW_X) ?
                    (player.flag & FLAG_PLAYER_OVERFLOW_PX) ?
                    "        " : "        " : "NONE",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Y) ?
                    (player.flag & FLAG_PLAYER_OVERFLOW_PY) ?
                    "        " : "        " : "NONE",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Z) ?
                    (player.flag & FLAG_PLAYER_OVERFLOW_PZ) ?
                    "        " : "        " : "NONE"),
                (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0,
                COLOR_DIAGNOSTIC_NONE);

        text_push(stringf(
                    "             %s %s %s",
                    (player.flag & FLAG_PLAYER_OVERFLOW_X) &&
                    (player.flag & FLAG_PLAYER_OVERFLOW_PX) ? "POSITIVE" : "    ",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Y) &&
                    (player.flag & FLAG_PLAYER_OVERFLOW_PY) ? "POSITIVE" : "    ",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Z) &&
                    (player.flag & FLAG_PLAYER_OVERFLOW_PZ) ? "POSITIVE" : "    "),
                (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0,
                DIAGNOSTIC_COLOR_SUCCESS);

        text_push(stringf(
                    "             %s %s %s\n",
                    (player.flag & FLAG_PLAYER_OVERFLOW_X) &&
                    !(player.flag & FLAG_PLAYER_OVERFLOW_PX) ? "NEGATIVE" : "    ",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Y) &&
                    !(player.flag & FLAG_PLAYER_OVERFLOW_PY) ? "NEGATIVE" : "    ",
                    (player.flag & FLAG_PLAYER_OVERFLOW_Z) &&
                    !(player.flag & FLAG_PLAYER_OVERFLOW_PZ) ? "NEGATIVE" : "    "),
                (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0,
                DIAGNOSTIC_COLOR_ERROR);

        text_push(stringf(
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
                (v2f32){SET_MARGIN, SET_MARGIN}, 0, 0,
                COLOR_DIAGNOSTIC_INFO);

        text_render(TRUE, TEXT_COLOR_SHADOW);

        text_push(stringf(
                    "CHUNK QUEUE 0 [%d/%"PRIu64"]\n"
                    "CHUNK QUEUE 1 [%d/%"PRIu64"]\n"
                    "CHUNK QUEUE 2 [%d/%"PRIu64"]\n"
                    "TOTAL CHUNKS [%"PRIu64"]\n",
                    CHUNK_QUEUE[0].count, CHUNK_QUEUE[0].size,
                    CHUNK_QUEUE[1].count, CHUNK_QUEUE[1].size,
                    CHUNK_QUEUE[2].count, CHUNK_QUEUE[2].size,
                    CHUNKS_MAX[settings.render_distance]),
                (v2f32){render->size.x - SET_MARGIN, SET_MARGIN},
                TEXT_ALIGN_RIGHT, 0,
                COLOR_TEXT_DEFAULT);

        text_render(TRUE, TEXT_COLOR_SHADOW);
        text_start(font[FONT_MONO], FONT_SIZE_DEFAULT, 0, NULL, FALSE);

        text_push(stringf(
                    "Game:     %s v%s\n"
                    "Engine:   %s v%s\n"
                    "Author:   %s\n"
                    "OpenGL:   %s\n"
                    "GLSL:     %s\n"
                    "Vendor:   %s\n"
                    "Renderer: %s\n",
                    GAME_NAME, GAME_VERSION,
                    ENGINE_NAME, ENGINE_VERSION, ENGINE_AUTHOR,
                    glGetString(GL_VERSION),
                    glGetString(GL_SHADING_LANGUAGE_VERSION),
                    glGetString(GL_VENDOR),
                    glGetString(GL_RENDERER)),
                (v2f32){render->size.x - SET_MARGIN, render->size.y - SET_MARGIN},
                TEXT_ALIGN_RIGHT, TEXT_ALIGN_BOTTOM,
                DIAGNOSTIC_COLOR_TRACE);

        text_render(TRUE, TEXT_COLOR_SHADOW);
    }

    text_stop();

    /* ---- draw logger strings --------------------------------------------- */

    text_start(font[FONT_MONO_BOLD], settings.font_size, 0, NULL, FALSE);
    i32 i = 0;
    for (i = 24; i >= 0; --i)
        text_push(stringf("%s",
                    logger_tab[mod(logger_tab_index - i, LOGGER_HISTORY_MAX)]),
                (v2f32){SET_MARGIN, render->size.y - SET_MARGIN - settings.font_size * i},
                0, TEXT_ALIGN_BOTTOM, logger_color[logger_tab_index - i]);
    text_render(TRUE, TEXT_COLOR_SHADOW);
    text_stop();

    /* ---- post processing ------------------------------------------------- */

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[FBO_POST_PROCESSING].fbo);
    glUseProgram(engine_shader[ENGINE_SHADER_UNIT_QUAD].id);
    glBindVertexArray(engine_mesh_unit.vao);
    glBindTexture(GL_TEXTURE_2D, fbo[FBO_SKYBOX].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo[FBO_WORLD].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo[FBO_HUD].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    ui_fbo_blit(fbo[FBO_POST_PROCESSING].fbo);
    text_fbo_blit(fbo[FBO_POST_PROCESSING].fbo);

    /* ---- final ----------------------------------------------------------- */

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader[SHADER_POST_PROCESSING].id);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(engine_mesh_unit.vao);
    glUniform1ui(uniform.post_processing.time, ((u32)(render->time) & 0x1ff) + 1);
    glBindTexture(GL_TEXTURE_2D, fbo[FBO_POST_PROCESSING].color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main(int argc, char **argv)
{
    if (engine_init(argc, argv, GAME_DIR_NAME_LOGS, GAME_TITLE, 1280, 1054, NULL,
                GAME_RELEASE_BUILD | FLAG_ENGINE_LOAD_DEFAULT_SHADERS) != ERR_SUCCESS ||
            game_init() != ERR_SUCCESS)
        goto cleanup;

    core.flag.active = 1;

    if (!GAME_RELEASE_BUILD)
        LOGDEBUG(FALSE, TRUE, "%s\n", "DEVELOPMENT BUILD");

    if (!MODE_INTERNAL_DEBUG)
    {
        LOGWARNING(FALSE, TRUE, ERR_MODE_INTERNAL_DEBUG_DISABLE,
                "%s\n", "'MODE_INTERNAL_DEBUG' Disabled");
    }
    else LOGDEBUG(FALSE, TRUE, "%s\n", "Debugging Enabled");

    if (!MODE_INTERNAL_COLLIDE)
    {
        LOGWARNING(FALSE, TRUE, ERR_MODE_INTERNAL_COLLIDE_DISABLE,
                "%s\n", "'MODE_INTERNAL_COLLIDE' Disabled");
    }

    if (rand_init() != ERR_SUCCESS ||
            settings_init() != ERR_SUCCESS)
        goto cleanup;

#if !GAME_RELEASE_BUILD
    glfwSetWindowPos(render->window, 1920 - render->size.x, 24);
#endif /* GAME_RELEASE_BUILD */

    /* ---- set mouse input ------------------------------------------------- */

    glfwSetInputMode(render->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(render->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        LOGINFO(FALSE, TRUE, "%s\n", "GLFW: Raw Mouse Motion Enabled");
    }
    else LOGERROR(FALSE, TRUE, ERR_GLFW, "%s\n", "GLFW: Raw Mouse Motion Not Supported");

    /* ---- set callbacks --------------------------------------------------- */

    glfwSetFramebufferSizeCallback(render->window, callback_framebuffer_size);
    callback_framebuffer_size(render->window, render->size.x, render->size.y);

    glfwSetKeyCallback(render->window, callback_key);
    callback_key(render->window, 0, 0, 0, 0);

    glfwSetScrollCallback(render->window, callback_scroll);
    callback_scroll(render->window, 0.0f, 0.0f);

    /* ---- set graphics ---------------------------------------------------- */

    if (
            fbo_init(&fbo[FBO_SKYBOX],      NULL, FALSE, 4) != ERR_SUCCESS ||
            fbo_init(&fbo[FBO_WORLD],       NULL, FALSE, 4) != ERR_SUCCESS ||
            fbo_init(&fbo[FBO_WORLD_MSAA],  NULL, TRUE, 4) != ERR_SUCCESS ||
            fbo_init(&fbo[FBO_HUD],         NULL, FALSE, 4) != ERR_SUCCESS ||
            fbo_init(&fbo[FBO_HUD_MSAA],    NULL, TRUE, 4) != ERR_SUCCESS ||
            fbo_init(&fbo[FBO_POST_PROCESSING], NULL, FALSE, 4) != ERR_SUCCESS)
        goto cleanup;

    if (
            assets_init() != ERR_SUCCESS ||
            text_init(0, FALSE) != ERR_SUCCESS ||
            ui_init(FALSE) != ERR_SUCCESS)
        goto cleanup;

    /*temp off
    init_super_debugger(render->size);
    */

    player.camera =
        (Camera){
            .fovy = settings.fov,
            .fovy_smooth = 0.0f,
            .ratio = (f32)render->size.x / (f32)render->size.y,
            .far = CAMERA_CLIP_FAR_OPTIMAL,
            .near = CAMERA_CLIP_NEAR_DEFAULT,
        };

    player.camera_hud =
        (Camera){
            .fovy = (f32)SET_FOV_DEFAULT,
            .fovy_smooth = (f32)SET_FOV_DEFAULT,
            .ratio = (f32)render->size.x / (f32)render->size.y,
            .far = CAMERA_CLIP_FAR_UI,
            .near = CAMERA_CLIP_NEAR_DEFAULT,
        };

    bind_shader_uniforms();

section_menu_title:

section_menu_pause:

section_world_loaded:

    if (!core.flag.world_loaded &&
            world_init("Poop Consistency Tester", 0, &player) != ERR_SUCCESS)
            goto cleanup;

    generate_standard_meshes();

    while (engine_update() && core.flag.active)
    {
        glfwPollEvents();
        update_key_states();
        input_update(&player);
        update_mouse_movement();

        settings_update();
        world_update(&player);
        draw_everything();

        glfwSwapBuffers(render->window);
        time_update(core.flag.fps_cap, settings.target_fps);

        process_screenshot_request(GAME_DIR_NAME_SCREENSHOTS, world.name);

        if (!core.flag.world_loaded)
            goto section_menu_title;

        if (core.flag.paused)
            goto section_menu_pause;
    }

cleanup:

    assets_free();
    chunking_free();
    u32 i = 0;
    for (i = 0; i < MESH_COUNT; ++i)
        mesh_free(&mesh[i]);
    for (i = 0; i < FBO_COUNT; ++i)
        fbo_free(&fbo[i]);
    for (i = 0; i < SHADER_COUNT; ++i)
        shader_program_free(&shader[i]);
    rand_free();
    engine_close();
    return *GAME_ERR;
}
