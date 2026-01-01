#ifndef GAME_H
#define GAME_H

#define GAME_RELEASE_BUILD  0

#define GAME_VERSION_STABLE "-stable"
#define GAME_VERSION_BETA   "-beta"
#define GAME_VERSION_ALPHA  "-alpha"
#define GAME_VERSION_DEV    "-dev"

#define GAME_AUTHOR         "Author: Lily Awertnex"
#define GAME_NAME           "Heaven-Hell Continuum"
#define GAME_VERSION        "0.4.0"GAME_VERSION_DEV

#include <engine/h/core.h>
#include <engine/h/diagnostics.h>
#include <engine/h/types.h>

/* ---- settings ------------------------------------------------------------ */

#define MODE_INTERNAL_VSYNC         0
#define MODE_INTERNAL_DEBUG         1
#define MODE_INTERNAL_LOAD_CHUNKS   1
#define MODE_INTERNAL_COLLIDE       1

/* ---- internal ------------------------------------------------------------ */

#define color_hex_u32(r, g, b, a) ((r << 24) | (g << 16) | (b << 8) | (a))

#define SET_MARGIN                      10
#define SET_CAMERA_DISTANCE_MAX         4.0f
#define SET_DAY_TICKS_MAX               24000
#define SET_RENDER_DISTANCE_DEFAULT     6
#define SET_RENDER_DISTANCE_MIN         2
#define SET_RENDER_DISTANCE_MAX         32
#define SET_FOV_DEFAULT                 70
#define SET_FOV_MIN                     30
#define SET_FOV_MAX                     160
#define SET_MOUSE_SENSITIVITY_DEFAULT   100
#define SET_MOUSE_SENSITIVITY_MIN       10
#define SET_MOUSE_SENSITIVITY_MAX       200
#define SET_TARGET_FPS_DEFAULT          60
#define SET_TARGET_FPS_MIN              1
#define SET_TARGET_FPS_MAX              241     /* 240 max, 241 = unlimited */
#define SET_GUI_SCALE_DEFAULT           2
#define SET_GUI_SCALE_0                 0 /* TODO: auto gui scale */
#define SET_GUI_SCALE_1                 1
#define SET_GUI_SCALE_2                 2
#define SET_GUI_SCALE_3                 3
#define SET_GUI_SCALE_4                 4
#define SET_LERP_SPEED_DEFAULT          25.0f
#define SET_LERP_SPEED_FOV_MODE         16.0f
#define SET_COLLISION_CAPSULE_PADDING   1.0f

#define COLOR_TEXT_DEFAULT      DIAGNOSTIC_COLOR_DEBUG
#define COLOR_TEXT_BRIGHT       DIAGNOSTIC_COLOR_DEFAULT
#define COLOR_TEXT_MOSS         0x6f9f3fff
#define COLOR_TEXT_RADIOACTIVE  0x3f9f3fff
#define COLOR_DIAGNOSTIC_NONE   0x995429ff
#define COLOR_DIAGNOSTIC_ERROR  0xec6051ff
#define COLOR_DIAGNOSTIC_INFO   0x3f6f9fff

#define FILE_NAME_SETTINGS      "settings.txt"
#define FILE_NAME_WORLD_SEED    "seed.txt"

enum MainFlag
{
    FLAG_MAIN_ACTIVE =          0x00000001,
    FLAG_MAIN_PAUSED =          0x00000002,
    FLAG_MAIN_HUD =             0x00000004,
    FLAG_MAIN_DEBUG =           0x00000008,
    FLAG_MAIN_SUPER_DEBUG =     0x00000010,
    FLAG_MAIN_FULLSCREEN =      0x00000020,
    FLAG_MAIN_MENU_OPEN =       0x00000040,
    FLAG_MAIN_DOUBLE_PRESS =    0x00000080,
    FLAG_MAIN_PARSE_TARGET =    0x00000100,
    FLAG_MAIN_WORLD_LOADED =    0x00000200,
    FLAG_MAIN_CHUNK_BUF_DIRTY = 0x00000400,
}; /* MainFlag */

enum DebugMode
{
    DEBUG_MODE_TRANS_BLOCKS,
    DEBUG_MODE_CHUNK_BOUNDS,
    DEBUG_MODE_BOUNDING_BOXES,
    DEBUG_MODE_CHUNK_GIZMO,
    DEBUG_MODE_CHUNK_QUEUE_VISUALIZER,
    DEBUG_MODE_COUNT,
}; /* DebugMode */

struct Settings
{
    /* ---- internal -------------------------------------------------------- */

    /*! @brief conversion from world-space to screen-space.
     *
     *  @remark read-only, updated internally in 'main.c/settings_update()'.
     */
    v2f32 ndc_scale;

    u32 fps;
    f32 lerp_speed;
    u32 chunk_buf_radius;
    u32 chunk_buf_diameter;
    u32 chunk_buf_layer;
    u32 chunk_buf_volume;
    u32 chunk_tab_center;

    f64 reach_distance; /* player reach (arm length) */

    /* ---- controls -------------------------------------------------------- */

    f32 mouse_sensitivity;

    /* ---- video ----------------------------------------------------------- */

    u32 gui_scale;
    f32 font_size;
    u32 target_fps;

    /* ---- graphics -------------------------------------------------------- */

    f32 fov;
    u32 render_distance;
    b8 anti_aliasing;
}; /* Settings */

struct Uniform
{
    struct /* defaults */
    {
        GLint offset;
        GLint scale;
        GLint mat_rotation;
        GLint mat_perspective;
        GLint sun_rotation;
        GLint sky_color;
    } defaults;

    struct /* ui */
    {
        GLint ndc_scale;
        GLint position;
        GLint offset;
        GLint texture_size;
        GLint size;
        GLint alignment;
        GLint tint;
    } ui;

    struct /* ui_9_slice */
    {
        GLint ndc_scale;
        GLint position;
        GLint size;
        GLint alignment;
        GLint tint;
        GLint slice;
        GLint slice_size;
        GLint texture_size;
        GLint sprite_size;
    } ui_9_slice;

    struct /* font */
    {
        GLint char_size;
        GLint font_size;
        GLint text_color;
    } font;

    struct /* skybox */
    {
        GLint texture_scale;
        GLint mat_translation;
        GLint mat_rotation;
        GLint mat_sun_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint texture_sky;
        GLint texture_horizon;
        GLint texture_stars;
        GLint texture_sun;
        GLint sun_rotation;
        GLint sky_color;
        GLint horizon_color;
        GLint render_layer;
    } skybox;

    struct /* gizmo */
    {
        GLint ndc_scale;
        GLint mat_translation;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint color;
    } gizmo;

    struct /* gizmo_chunk */
    {
        GLint gizmo_offset;
        GLint render_size;
        GLint chunk_buf_diameter;
        GLint mat_translation;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint camera_position;
        GLint time;
    } gizmo_chunk;

    struct /* post_processing */
    {
        GLint time;
    } post_processing;

    struct /* voxel */
    {
        GLint mat_perspective;
        GLint camera_position;
        GLint sun_rotation;
        GLint sky_light;
        GLint moon_light;
        GLint chunk_position;
        GLint color;
        GLint opacity;
        GLint flashlight_position;
        GLint toggle_flashlight;
        GLint render_distance;
    } voxel;

    struct /* bounding_box */
    {
        GLint mat_perspective;
        GLint position;
        GLint size;
        GLint color;
    } bounding_box;

}; /* Uniform */

enum ShaderIndices
{
    SHADER_FBO,
    SHADER_DEFAULT,
    SHADER_UI,
    SHADER_UI_9_SLICE,
    SHADER_TEXT,
    SHADER_SKYBOX,
    SHADER_GIZMO,
    SHADER_GIZMO_CHUNK,
    SHADER_POST_PROCESSING,
    SHADER_VOXEL,
    SHADER_BOUNDING_BOX,
    SHADER_COUNT,
}; /* ShaderIndices */

enum MeshIndices
{
    MESH_UNIT,
    MESH_SKYBOX,
    MESH_CUBE_OF_HAPPINESS,
    MESH_PLAYER,
    MESH_GIZMO,
    MESH_COUNT,
}; /* MeshIndices */

enum FBOIndices
{
    FBO_SKYBOX,
    FBO_WORLD,
    FBO_WORLD_MSAA,
    FBO_HUD,
    FBO_HUD_MSAA,
    FBO_UI,
    FBO_TEXT,
    FBO_TEXT_MSAA,
    FBO_POST_PROCESSING,
    FBO_COUNT,
}; /* FBOIndices */

enum TextureIndices
{
    TEXTURE_CROSSHAIR,
    TEXTURE_ITEM_BAR,
    TEXTURE_SDB_ACTIVE,
    TEXTURE_SDB_INACTIVE,
    TEXTURE_SKYBOX_VAL,
    TEXTURE_SKYBOX_HORIZON,
    TEXTURE_SKYBOX_STARS,
    TEXTURE_SUN,
    TEXTURE_MOON,
    TEXTURE_COUNT,
}; /* TextureIndices */

enum FontIndices
{
    FONT_REG,
    FONT_REG_BOLD,
    FONT_MONO,
    FONT_MONO_BOLD,
    FONT_COUNT,
}; /* FontIndices */

/*! @brief global pointer to variable for game/engine-specific error codes.
 *
 *  @remark must be initialized globally, tho the pointed to variable itself can be modified.
 */
extern u32 *const GAME_ERR;
extern struct Settings settings;
extern Texture texture[TEXTURE_COUNT];
extern Font font[FONT_COUNT];
extern u64 flag;
extern u8 debug_mode[DEBUG_MODE_COUNT];
extern Render render;
extern Projection projection_world;
extern Projection projection_hud;

#endif /* GAME_MAIN_H */
