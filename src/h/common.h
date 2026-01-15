#ifndef GAME_COMMON_H
#define GAME_COMMON_H

#include "diagnostics.h"

#define GAME_RELEASE_BUILD  0

#define GAME_VERSION_STABLE "-stable"
#define GAME_VERSION_BETA   "-beta"
#define GAME_VERSION_ALPHA  "-alpha"
#define GAME_VERSION_DEV    "-dev"

#define GAME_AUTHOR         "Author: Lily Awertnex"
#define GAME_NAME           "Heaven-Hell Continuum"
#define GAME_VERSION        "0.4.0"GAME_VERSION_DEV
#define GAME_TITLE          GAME_NAME": "GAME_VERSION

/* ---- settings ------------------------------------------------------------ */

#define MODE_INTERNAL_VSYNC         0
#define MODE_INTERNAL_DEBUG         1
#define MODE_INTERNAL_LOAD_CHUNKS   1
#define MODE_INTERNAL_COLLIDE       1

#define color_hex_u32(r, g, b, a) ((r << 24) | (g << 16) | (b << 8) | (a))

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
#define GAME_DIR_NAME_LOGS          "logs/"
#define GAME_DIR_NAME_SCREENSHOTS   "screenshots/"
#define GAME_DIR_NAME_TEXT          "text/"
#define GAME_DIR_NAME_WORLDS        "worlds/"

#define GAME_DIR_WORLD_NAME_CHUNKS      "chunks/"
#define GAME_DIR_WORLD_NAME_ENTITIES    "entities/"
#define GAME_DIR_WORLD_NAME_PLAYER      "chunks/"

#define GAME_FILE_NAME_SETTINGS     "settings.txt"
#define GAME_FILE_NAME_WORLD_SEED   "seed.txt"

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

enum /* ShaderIndex */
{
    SHADER_DEFAULT,
    SHADER_SKYBOX,
    SHADER_GIZMO,
    SHADER_GIZMO_CHUNK,
    SHADER_POST_PROCESSING,
    SHADER_VOXEL,
    SHADER_BOUNDING_BOX,
    SHADER_COUNT,
}; /* ShaderIndex */

enum /* MeshIndex */
{
    MESH_SKYBOX,
    MESH_CUBE_OF_HAPPINESS,
    MESH_PLAYER,
    MESH_GIZMO,
    MESH_COUNT,
}; /* MeshIndex */

enum /* FBOIndex */
{
    FBO_SKYBOX,
    FBO_WORLD,
    FBO_WORLD_MSAA,
    FBO_HUD,
    FBO_HUD_MSAA,
    FBO_POST_PROCESSING,
    FBO_COUNT,
}; /* FBOIndex */

enum /* TextureIndex */
{
    TEXTURE_CROSSHAIR,
    TEXTURE_ITEM_BAR,
    TEXTURE_SKYBOX_VAL,
    TEXTURE_SKYBOX_HORIZON,
    TEXTURE_SKYBOX_STARS,
    TEXTURE_SUN,
    TEXTURE_MOON,
    TEXTURE_COUNT,
}; /* TextureIndex */

enum /* FontIndex */
{
    FONT_REG,
    FONT_REG_BOLD,
    FONT_MONO,
    FONT_MONO_BOLD,
    FONT_COUNT,
}; /* FontIndex */

enum /* DebugMode */
{
    DEBUG_MODE_TRANS_BLOCKS,
    DEBUG_MODE_CHUNK_BOUNDS,
    DEBUG_MODE_BOUNDING_BOXES,
    DEBUG_MODE_CHUNK_GIZMO,
    DEBUG_MODE_CHUNK_QUEUE_VISUALIZER,
    DEBUG_MODE_COUNT,
}; /* DebugMode */

#endif /* GAME_COMMON_H */
