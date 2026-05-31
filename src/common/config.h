/*!
 *  Copyright 2026 Lily Awertnex
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
 *  limitations under the License.
 */

/*!
 *  @file config.h
 *
 *  @brief engine commonly used values.
 */

#ifndef FSL_CONFIG_H
#define FSL_CONFIG_H

#define FSL_DIR_NAME_FONTS      "fossil/assets/fonts/"
#define FSL_DIR_NAME_TEXTURES   "fossil/assets/textures/"
#define FSL_DIR_NAME_SHADERS    "fossil/assets/shaders/"
#define FSL_DIR_NAME_MODELS     "fossil/assets/models/"
#define FSL_DIR_NAME_LOGS       "fossil/logs/"

#define FSL_FILE_NAME_LOG_ERROR "log_error.log"
#define FSL_FILE_NAME_LOG_INFO  "log_info.log"
#define FSL_FILE_NAME_LOG_EXTRA "log_verbose.log"

#define FSL_FILE_FORMAT_NAME_FOSSIL_MESH "fmesh"

#define FSL_COLOR_CHANNELS_GRAY 1
#define FSL_COLOR_CHANNELS_RGB 3
#define FSL_COLOR_CHANNELS_RGBA 4

#define FSL_RENDER_WIDTH_DEFAULT 800
#define FSL_RENDER_WIDTH_MIN 512
#define FSL_RENDER_WIDTH_MAX 3840
#define FSL_RENDER_HEIGHT_DEFAULT 800
#define FSL_RENDER_HEIGHT_MIN 288
#define FSL_RENDER_HEIGHT_MAX 2160
#define FSL_CAMERA_CLIP_FAR_DEFAULT GL_CLIP_DISTANCE0
#define FSL_CAMERA_CLIP_FAR_OPTIMAL 2048.0
#define FSL_CAMERA_CLIP_FAR_UI 256.0
#define FSL_CAMERA_CLIP_NEAR_DEFAULT 0.03
#define FSL_CAMERA_ANGLE_MAX 90.0
#define FSL_CAMERA_RANGE_MAX 360.0
#define FSL_CAMERA_ZOOM_MAX 69.0
#define FSL_CAMERA_ZOOM_SPEED 10.0
#define FSL_CAMERA_ZOOM_SENSITIVITY 6.0
#define FSL_FONT_ATLAS_CELL_RESOLUTION 16
#define FSL_FONT_RESOLUTION_DEFAULT 64
#define FSL_FONT_SIZE_DEFAULT 22.0
#define FSL_TEXT_TAB_SIZE 4
#define FSL_TEXT_COLOR_SHADOW 0x00000060
#define FSL_TEXT_OFFSET_SHADOW 2.0
#define FSL_TARGET_FPS_DEFAULT 60
#define FSL_TARGET_FPS_MIN 1
#define FSL_TARGET_FPS_MAX 256

/* ---- shader bindings ----------------------------------------------------- */

#define FSL_SHADER_BUFFER_BINDING_UBO_NDC_SCALE 0
#define FSL_SHADER_BUFFER_BINDING_COUNT 1

#endif /* FSL_ENGINE_INFO_H */
