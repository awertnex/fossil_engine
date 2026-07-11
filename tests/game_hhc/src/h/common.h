#ifndef HHC_COMMON_H
#define HHC_COMMON_H

/* ---- defaults ------------------------------------------------------------ */

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
#define SET_GUI_SCALE_0                 0.0f /* TODO: auto gui scale */
#define SET_GUI_SCALE_1                 1.0f
#define SET_GUI_SCALE_2                 2.0f
#define SET_GUI_SCALE_3                 3.0f
#define SET_GUI_SCALE_4                 4.0f
#define SET_GUI_SCALE_DEFAULT           SET_GUI_SCALE_2
#define SET_LERP_SPEED_DEFAULT          25.0f
#define SET_LERP_SPEED_FOV_MODE         16.0f
#define SET_COLLISION_CAPSULE_PADDING   1.0f
#define SET_TEXT_REFRESH_INTERVAL       4 /* unit: fps */
#define SET_CONSOLE_SCROLL_SPEED        4

#define COLOR_TEXT_DEFAULT      FSL_DIAGNOSTIC_COLOR_DEBUG
#define COLOR_TEXT_BRIGHT       FSL_DIAGNOSTIC_COLOR_DEFAULT
#define COLOR_TEXT_MOSS         0x6f9f3fff
#define COLOR_TEXT_RADIOACTIVE  0x3f9f3fff
#define COLOR_TEXT_ECONOMIC     0xdfdf00ff
#define COLOR_DIAGNOSTIC_NONE   0x995429ff
#define COLOR_DIAGNOSTIC_ERROR  0xec6051ff
#define COLOR_DIAGNOSTIC_INFO   0x3f6f9fff

/* ---- shader bindings ----------------------------------------------------- */

#define SHADER_BUFFER_BINDING_SSBO_TEXTURE_INDICES (0 + FSL_SHADER_BUFFER_BINDING_COUNT)
#define SHADER_BUFFER_BINDING_SSBO_TEXTURE_HANDLES (1 + FSL_SHADER_BUFFER_BINDING_COUNT)

/* ---- strings ------------------------------------------------------------- */

#define GAME_DIR_NAME_ASSETS        "assets/"
#define GAME_DIR_NAME_AUDIO         "assets/audio/"
#define GAME_DIR_NAME_FONTS         "assets/fonts/"
#define GAME_DIR_NAME_LOOKUPS       "assets/lookups/"
#define GAME_DIR_NAME_MODELS        "assets/models/"
#define GAME_DIR_NAME_SHADERS       "assets/shaders/"
#define GAME_DIR_NAME_TEXTURES      "assets/textures/"
#define GAME_DIR_NAME_BLOCKS        "assets/textures/blocks/"
#define GAME_DIR_NAME_ENTITIES      "assets/textures/entities/"
#define GAME_DIR_NAME_ENV           "assets/textures/env/"
#define GAME_DIR_NAME_GUI           "assets/textures/gui/"
#define GAME_DIR_NAME_ITEMS         "assets/textures/items/"
#define GAME_DIR_NAME_LOGO          "assets/textures/logo/"
#define GAME_DIR_NAME_CONFIG        "config/"
#define GAME_DIR_NAME_SCREENSHOTS   "screenshots/"
#define GAME_DIR_NAME_TEXT          "text/"
#define GAME_DIR_NAME_WORLDS        "worlds/"

#define GAME_DIR_WORLD_NAME_CHUNKS      "chunks/"
#define GAME_DIR_WORLD_NAME_ENTITIES    "entities/"
#define GAME_DIR_WORLD_NAME_PLAYER      "chunks/"

/* ---- file names ---------------------------------------------------------- */

#define GAME_FILE_NAME_SETTINGS         "settings.conf"
#define GAME_FILE_NAME_WORLD_METADATA   "metadata.conf"
#define GAME_FILE_NAME_LOOKUP_CHUNKS_MAX "chunks_max.lut"
#define GAME_FILE_NAME_LOOKUP_CHUNK_ORDER "chunk_order.lut"
#define GAME_FILE_NAME_LOOKUP_CHUNK_BUCKET "chunk_bucket.lut"

/* ---- name formats -------------------------------------------------------- */

#define FORMAT_FILE_NAME_HHCC "%d.%d.%d.hhcr"

enum mesh_index
{
    MESH_CUBE_OF_HAPPINESS,
    MESH_GIZMO_AXIS,
    MESH_COUNT
}; /* mesh_index */

enum fbo_index
{
    FBO_SKYBOX,
    FBO_WORLD,
    FBO_WORLD_MSAA,
    FBO_HUD,
    FBO_HUD_MSAA,
    FBO_POST_PROCESSING,
    FBO_COUNT
}; /* fbo_index */

enum font_index
{
    FONT_REG,
    FONT_REG_BOLD,
    FONT_MONO,
    FONT_MONO_BOLD,
    FONT_COUNT
}; /* font_index */

#endif /* HHC_COMMON_H */
