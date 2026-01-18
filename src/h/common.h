#ifndef GAME_COMMON_H
#define GAME_COMMON_H

#include <engine/h/types.h>

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
#define SET_TEXT_REFRESH_INTERVAL       10 /* unit: fps */

#define DEATH_STRING_MAX        128

#define COLOR_TEXT_DEFAULT      DIAGNOSTIC_COLOR_DEBUG
#define COLOR_TEXT_BRIGHT       DIAGNOSTIC_COLOR_DEFAULT
#define COLOR_TEXT_MOSS         0x6f9f3fff
#define COLOR_TEXT_RADIOACTIVE  0x3f9f3fff
#define COLOR_DIAGNOSTIC_NONE   0x995429ff
#define COLOR_DIAGNOSTIC_ERROR  0xec6051ff
#define COLOR_DIAGNOSTIC_INFO   0x3f6f9fff

/* ---- shader bindings ----------------------------------------------------- */

#define SHADER_BUFFER_BINDING_SSBO_TEXTURE_INDICES (0 + ENGINE_SHADER_BUFFER_BINDING_COUNT)
#define SHADER_BUFFER_BINDING_SSBO_TEXTURE_HANDLES (1 + ENGINE_SHADER_BUFFER_BINDING_COUNT)

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

/* ---- strings: death ------------------------------------------------------ */

#define DEATH_STRING_COLLISION_WALL_0           "died by headbutting a wall"
#define DEATH_STRING_COLLISION_WALL_1           "rammed a wall at high speed"
#define DEATH_STRING_COLLISION_WALL_2           "splat on a wall"
#define DEATH_STRING_COLLISION_WALL_COUNT       3

#define DEATH_STRING_COLLISION_FLOOR_0          "jumped off a cliff"
#define DEATH_STRING_COLLISION_FLOOR_1          "fell to their death"
#define DEATH_STRING_COLLISION_FLOOR_2          "splat on the ground"
#define DEATH_STRING_COLLISION_FLOOR_COUNT      3

#define DEATH_STRING_COLLISION_CEILING_0        "cracked their skull at a ceiling"
#define DEATH_STRING_COLLISION_CEILING_1        "flew into a ceiling"
#define DEATH_STRING_COLLISION_CEILING_2        "splat on a ceiling"
#define DEATH_STRING_COLLISION_CEILING_COUNT    3

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

enum /* FontIndex */
{
    FONT_REG,
    FONT_REG_BOLD,
    FONT_MONO,
    FONT_MONO_BOLD,
    FONT_COUNT,
}; /* FontIndex */

enum PlayerDeath
{
    PLAYER_DEATH_COLLISION_WALL = 1,
    PLAYER_DEATH_COLLISION_FLOOR,
    PLAYER_DEATH_COLLISION_CEILING,
    PLAYER_DEATH_COUNT,
}; /* PlayerDeath */

/*! @brief look-up table for 'str_death_' buffer sizes.
 *
 *  @remark read-only, initialized internally in 'src/common.c'.
 */
extern u32 DEATH_STRINGS_MAX[PLAYER_DEATH_COUNT];

extern str str_death_collision_wall[DEATH_STRING_COLLISION_WALL_COUNT][DEATH_STRING_MAX];
extern str str_death_collision_floor[DEATH_STRING_COLLISION_FLOOR_COUNT][DEATH_STRING_MAX];
extern str str_death_collision_ceiling[DEATH_STRING_COLLISION_CEILING_COUNT][DEATH_STRING_MAX];

#endif /* GAME_COMMON_H */
