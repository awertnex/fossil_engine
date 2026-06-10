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
 *  @file ui.h
 *
 *  @brief main UI module header.
 */

#ifndef FSL_UI_H
#define FSL_UI_H

#include "../common/api.h"
#include "../common/types.h"
#include "../assets/asset_types.h"

#include "ui_types.h"

FSLAPI void fsl_ui_element_set_texture(fsl_ui_element *element, fsl_texture *texture);

/*!
 *  @brief set UV coordinates of `element` in its texture.
 *
 *  @param pos_x horizontal position in texture, in pixels.
 *  @param pos_y vertical position in texture, in pixels.
 *  @param size_x width in texture, in pixels.
 *  @param size_y height in texture, in pixels.
 */
FSLAPI void fsl_ui_element_set_uv(fsl_ui_element *element,
        i32 pos_x, i32 pos_y, i32 size_x, i32 size_y);

/*!
 *  @brief set position of `element` on screen.
 *
 *  @param pos_x horizontal position on screen, in pixels.
 *  @param pos_y vertical position on screen, in pixels.
 *  @param offset_x horizontal offset from position, in pixels.
 *  @param offset_y vertical offset from position, in pixels.
 *
 *  @param offset_scaled_x horizontal offset from position, in pixels (scales with
 *  `element->sprite.scale`).
 *
 *  @param offset_scaled_y vertical offset from position, in pixels (scales with
 *  `element->sprite.scale`).
 */
FSLAPI void fsl_ui_element_set_position(fsl_ui_element *element, i32 pos_x, i32 pos_y,
        i32 offset_x, i32 offset_y, i32 offset_scaled_x, i32 offset_scaled_y);

/*!
 *  @brief set size of `element` on screen.
 *
 *  @param size_x width on screen, in pixels.
 *  @param size_y height on screen, in pixels.
 *  @param size_scaled_x width on screen, in pixels (scales with `element->sprite.scale`).
 *  @param size_scaled_y height on screen, in pixels (scales with `element->sprite.scale`).
 */
FSLAPI void fsl_ui_element_set_size(fsl_ui_element *element,
        i32 size_x, i32 size_y, i32 size_scaled_x, i32 size_scaled_y);

/*!
 *  @brief set scaling of `element` for its 'scaled' parameters.
 *
 *  @param scale_x horizontal gui scaling, for sprite's 'scaled' parameters.
 *  @param scale_y vertical gui scaling, for sprite's 'scaled' parameters.
 */
FSLAPI void fsl_ui_element_set_scale(fsl_ui_element *element, f32 scale_x, f32 scale_y);

/*!
 *  @brief set alignment of `element`.
 *
 *  @param align_x horizontal alignment in respect to its sprite position and size.
 *  @param align_y vertical alignment in respect to its sprite position and size.
 */
FSLAPI void fsl_ui_element_set_alignment(fsl_ui_element *element, i32 align_x, i32 align_y);

/*!
 *  @brief attach a UI element to another (e.g., button in a menu).
 *
 *  - `child` will draw using `parent` parameters + its own parameters as delta.
 *
 *  @param parent element to attach to.
 *  @param child element to attach.
 */
FSLAPI void fsl_ui_element_attach(fsl_ui_element *parent, fsl_ui_element *child);

#endif /* FSL_UI_H */
