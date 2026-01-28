/*  Copyright 2026 Lily Awertnex
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.OFTWARE.
 */

/*  common.h - engine info and commonly shared values
 */

#ifndef FSL_COMMON_H
#define FSL_COMMON_H

#define FSL_ENGINE_AUTHOR           "Lily Awertnex"
#define FSL_ENGINE_NAME             "Fossil Engine"

#define FSL_ENGINE_VERSION_STABLE   "-stable"
#define FSL_ENGINE_VERSION_BETA     "-beta"
#define FSL_ENGINE_VERSION_ALPHA    "-alpha"
#define FSL_ENGINE_VERSION_DEV      "-dev"

#define FSL_ENGINE_VERSION_MAJOR    0
#define FSL_ENGINE_VERSION_MINOR    3
#define FSL_ENGINE_VERSION_PATCH    4
#define FSL_ENGINE_VERSION_BUILD    FSL_ENGINE_VERSION_DEV

#if defined(__linux__) || defined(__linux)
#   define _GNU_SOURCE

#   define FSL_PLATFORM_LINUX       1
#   define FSL_PLATFORM             "linux"
#   define FSL_FILE_NAME_LIB        "libfossil.so"
#   define FSL_FILE_NAME_PLATFORM   "platform_linux.c"
#   define FSL_EXE                  ""
#   define FSL_RUNTIME_PATH         "$ORIGIN"
#   define FSL_SLASH_NATIVE         '/'
#   define FSL_SLASH_NON_NATIVE     '\\'
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#   define FSL_PLATFORM_WIN         1
#   define FSL_PLATFORM             "win"
#   define FSL_FILE_NAME_LIB        "fossil.dll"
#   define FSL_FILE_NAME_PLATFORM   "platform_win.c"
#   define FSL_EXE                  ".exe"
#   define FSL_RUNTIME_PATH         "%CD%"
#   define FSL_SLASH_NATIVE         '\\'
#   define FSL_SLASH_NON_NATIVE     '/'
#endif /* FSL_PLATFORM */

#if FSL_PLATFORM_WIN
#   define FSLAPI __declspec(dllexport)
#else
#   define FSLAPI __attribute__((visibility("default")))
#endif /* FSL_PLATFORM */

#ifndef FSLAPI
#   define FSLAPI
#endif /* FSLAPI */

#include "types.h"

#define FSL_DIR_NAME_FONTS      "engine/assets/fonts/"
#define FSL_DIR_NAME_TEXTURES   "engine/assets/textures/"
#define FSL_DIR_NAME_SHADERS    "engine/assets/shaders/"
#define FSL_DIR_NAME_LOGS       "engine/logs/"

#define FSL_FILE_NAME_LOG_ERROR "log_error.log"
#define FSL_FILE_NAME_LOG_INFO  "log_info.log"
#define FSL_FILE_NAME_LOG_EXTRA "log_verbose.log"

#define FSL_COLOR_CHANNELS_RGBA 4
#define FSL_COLOR_CHANNELS_RGB 3
#define FSL_COLOR_CHANNELS_GRAY 1

#define FSL_RENDER_WIDTH_DEFAULT 1280
#define FSL_RENDER_WIDTH_MIN 512
#define FSL_RENDER_WIDTH_MAX 3840
#define FSL_RENDER_HEIGHT_DEFAULT 720
#define FSL_RENDER_HEIGHT_MIN 288
#define FSL_RENDER_HEIGHT_MAX 2160
#define FSL_CAMERA_CLIP_FAR_DEFAULT GL_CLIP_DISTANCE0
#define FSL_CAMERA_CLIP_FAR_OPTIMAL 2048.0f
#define FSL_CAMERA_CLIP_FAR_UI 256.0f
#define FSL_CAMERA_CLIP_NEAR_DEFAULT 0.03f
#define FSL_CAMERA_ANGLE_MAX 90.0
#define FSL_CAMERA_RANGE_MAX 360.0
#define FSL_CAMERA_ZOOM_MAX 69.0f
#define FSL_CAMERA_ZOOM_SPEED 10.0
#define FSL_CAMERA_ZOOM_SENSITIVITY 6.0
#define FSL_FONT_ATLAS_CELL_RESOLUTION 16
#define FSL_FONT_RESOLUTION_DEFAULT 64
#define FSL_FONT_SIZE_DEFAULT 22.0f
#define FSL_TEXT_TAB_SIZE 4
#define FSL_TEXT_CHAR_STRIDE 5
#define FSL_TEXT_COLOR_SHADOW 0x00000060
#define FSL_TEXT_OFFSET_SHADOW 2.0f
#define FSL_TARGET_FPS_DEFAULT 60
#define FSL_TARGET_FPS_MIN 1
#define FSL_TARGET_FPS_MAX 256

/* ---- shader bindings ----------------------------------------------------- */

#define FSL_SHADER_BUFFER_BINDING_UBO_NDC_SCALE 0
#define FSL_SHADER_BUFFER_BINDING_COUNT 1

/* ---- color conversion macros --------------------------------------------- */

/*! @brief convert RGBA color to 4-Byte hex color.
 *  @remark color range [0.0f, 1.0f].
 */
#define fsl_color_v4_to_hex(r, g, b, a) \
    (((u32)((r) * 0xff) << 0x18) | \
     ((u32)((g) * 0xff) << 0x10) | \
     ((u32)((b) * 0xff) << 0x08) | \
     ((u32)(a) * 0xff))

/*! @brief convert 4-Byte hex color to RGBA color.
 *  @remark color range [0.0f, 1.0f].
 */
#define fsl_color_hex_to_v4(n) (v4f32){ \
    (f32)(((n) >> 0x18) & 0xff) / 0xff, \
    (f32)(((n) >> 0x10) & 0xff) / 0xff, \
    (f32)(((n) >> 0x08) & 0xff) / 0xff, \
    (f32)(((n) >> 0x00) & 0xff) / 0xff}

enum fsl_flag
{
    FSL_FLAG_RELEASE_BUILD =        0x0001, /* output `TRACE` and `DEBUG` logs to console */
    FSL_FLAG_LOAD_DEFAULT_SHADERS = 0x0002, /* initialize default shaders (like 'text' and 'ui') */
    FSL_FLAG_MULTISAMPLE =          0x0004, /* use `GLFW` multisampling */
}; /* fsl_flag */

enum fsl_string_index
{
    FSL_STR_INDEX_ENGINE_TITLE, /* ENGINE_NAME: ENGINE_VERSION */
    FSL_STR_INDEX_ENGINE_VERSION,
    FSL_STR_INDEX_COUNT,
}; /* fsl_string_index */

enum fsl_shader_index
{
    FSL_SHADER_INDEX_UNIT_QUAD,
    FSL_SHADER_INDEX_TEXT,
    FSL_SHADER_INDEX_UI,
    FSL_SHADER_INDEX_UI_9_SLICE,
    FSL_SHADER_INDEX_COUNT,
}; /* fsl_shader_index */

enum fsl_font_index
{
    FSL_FONT_INDEX_DEJAVU_SANS,
    FSL_FONT_INDEX_DEJAVU_SANS_BOLD,
    FSL_FONT_INDEX_DEJAVU_SANS_MONO,
    FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD,
    FSL_FONT_INDEX_COUNT,
}; /* fsl_font_index */

enum fsl_text_alignment
{
    FSL_TEXT_ALIGN_LEFT = 0,
    FSL_TEXT_ALIGN_CENTER = 1,
    FSL_TEXT_ALIGN_RIGHT = 2,
    FSL_TEXT_ALIGN_TOP = 0,
    FSL_TEXT_ALIGN_BOTTOM = 2,
}; /* fsl_text_alignment */

enum fsl_texture_index
{
    FSL_TEXTURE_INDEX_PANEL_ACTIVE,
    FSL_TEXTURE_INDEX_PANEL_INACTIVE,
    FSL_TEXTURE_INDEX_COUNT,
}; /* fsl_texture_index */

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief POSIX timestamp of the main process' start in milliseconds.
 */
extern u64 fsl_init_time;

/*! @brief project root directory.
 *  
 *  @remark called from @ref fsl_init() and @ref fsl_logger_init() to change current working dirctory.
 *
 *  @remark declared internally.
 */
FSLAPI extern str *FSL_DIR_PROC_ROOT;

#endif /* FSL_COMMON_H */
