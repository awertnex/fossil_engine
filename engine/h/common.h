#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#include "types.h"

#define ENGINE_DIR_NAME_FONTS       "engine/assets/fonts/"
#define ENGINE_DIR_NAME_TEXTURES    "engine/assets/textures/"
#define ENGINE_DIR_NAME_SHADERS     "engine/assets/shaders/"
#define ENGINE_DIR_NAME_LOGS        "engine/logs/"

#define ENGINE_FILE_NAME_LOG_ERROR  "log_error.log"
#define ENGINE_FILE_NAME_LOG_INFO   "log_info.log"
#define ENGINE_FILE_NAME_LOG_EXTRA  "log_verbose.log"

#define RENDER_WIDTH_DEFAULT 1280
#define RENDER_WIDTH_MIN 512
#define RENDER_WIDTH_MAX 3840
#define RENDER_HEIGHT_DEFAULT 720
#define RENDER_HEIGHT_MIN 288
#define RENDER_HEIGHT_MAX 2160
#define CAMERA_CLIP_FAR_DEFAULT GL_CLIP_DISTANCE0
#define CAMERA_CLIP_FAR_OPTIMAL 2048.0f
#define CAMERA_CLIP_FAR_UI 256.0f
#define CAMERA_CLIP_NEAR_DEFAULT 0.03f
#define CAMERA_ANGLE_MAX 90.0
#define CAMERA_RANGE_MAX 360.0
#define CAMERA_ZOOM_MAX 69.0f
#define CAMERA_ZOOM_SPEED 10.0
#define CAMERA_ZOOM_SENSITIVITY 6.0
#define FONT_ATLAS_CELL_RESOLUTION 16
#define FONT_RESOLUTION_DEFAULT 64
#define FONT_SIZE_DEFAULT 22.0f
#define TEXT_TAB_SIZE 4
#define TEXT_COLOR_SHADOW 0x00000060
#define TEXT_OFFSET_SHADOW 2.0f
#define TARGET_FPS_DEFAULT 60
#define TARGET_FPS_MIN 1
#define TARGET_FPS_MAX 256

enum /* ShaderIndex */
{
    ENGINE_SHADER_UNIT_QUAD,
    ENGINE_SHADER_TEXT,
    ENGINE_SHADER_UI,
    ENGINE_SHADER_UI_9_SLICE,
    ENGINE_SHADER_COUNT,
}; /* ShaderIndex */

enum /* FontIndex */
{
    ENGINE_FONT_DEJAVU_SANS,
    ENGINE_FONT_DEJAVU_SANS_BOLD,
    ENGINE_FONT_DEJAVU_SANS_MONO,
    ENGINE_FONT_DEJAVU_SANS_MONO_BOLD,
    ENGINE_FONT_COUNT,
}; /* FontIndex */

enum /* TextureIndex */
{
    ENGINE_TEXTURE_PANEL_ACTIVE,
    ENGINE_TEXTURE_PANEL_INACTIVE,
    ENGINE_TEXTURE_COUNT,
}; /* TextureIndex */

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief POSIX timestamp of the main process' start in milliseconds.
 */
extern u64 init_time;

#endif /* ENGINE_COMMON_H */
