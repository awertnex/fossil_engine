/**
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

/**
 *  @file ui_internal.h
 *
 *  @brief UI internal definitions.
 *
 *  @internal
 */

#ifndef FSL_UI_INTERNAL_H
#define FSL_UI_INTERNAL_H

#include "../common/limits.h"
#include "../common/types.h"

#include "ui_element.h"
#include "ui_types.h"

enum ui_element_flag_internal
{
    FSL_FLAG_UI_INACTIVE =          (1 << 0),
    FSL_FLAG_UI_DIRTY_TRANSFORM =   (1 << 1),
    FSL_FLAG_UI_DIRTY_PARENT =      (1 << 2),
    FSL_FLAG_UI_DIRTY_CHILDREN =    (1 << 3),
    FSL_FLAG_UI_EXPAND_X =          (1 << 4),
    FSL_FLAG_UI_EXPAND_Y =          (1 << 5)
}; /* fsl_ui_element_flag */

enum ui_element_capability_internal
{
    FSL_FLAG_UI_CAN_TEXTURE =       (1 << 0),
    FSL_FLAG_UI_CAN_9_SLICE =       (1 << 1),
    FSL_FLAG_UI_CAN_EXPAND_X =      (1 << 2),
    FSL_FLAG_UI_CAN_EXPAND_Y =      (1 << 3),
    FSL_FLAG_UI_CAN_CUSTOM_DRAW =   (1 << 4),
    FSL_FLAG_UI_CAN_TINT =          (1 << 5)
}; /* ui_element_capability_internal */

/**
 *  @internal
 */
extern fsl_texture ui_texture_rectangle_internal;

extern str str_element_type_name_internal[FSL_UI_ELEMENT_TYPE_COUNT][FSL_ID_CAP];

/* ---- section: signatures ------------------------------------------------- */

/**
 *  @brief general error-checking for element validity.
 *
 *  @internal
 *
 *  checks for out-of-range values and logs accordingly, and possibly checks
 *  other stuff.
 */
b8 ui_element_is_valid_internal(fsl_ui_element *element);

/**
 *  @brief poll event data and parse element's callbacks (e.g., button pressed).
 *
 *  @internal
 *
 *  @remark called automatically from @ref fsl_ui_element_draw().
 */
void ui_element_listen_internal(fsl_ui_element *element, v2f64 mouse_pos, v2f64 mouse_delta);

/**
 *  @brief bake nothing, placeholder for non-bake-able element types.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_none_bake_func(fsl_ui_element *element);

/**
 *  @brief bake a rectangle.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_rectangle_bake_func(fsl_ui_element *element);

/**
 *  @brief bake a sprite.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_sprite_bake_func(fsl_ui_element *element);

/**
 *  @brief bake a 9-slice panel.
 *
 *  @internal
 */
void ui_element_panel_9_slice_bake_func(fsl_ui_element *element);

/**
 *  @brief bake a button.
 *
 *  @internal
 */
void ui_element_button_bake_func(fsl_ui_element *element);

/**
 *  @brief make a 1-slice quad.
 *
 *  @internal
 *
 *  makes a render-ready quad from an element's baked transform.
 */
void ui_element_quad_1_slice_make_internal(fsl_ui_element *element,
        fsl_ui_quad_1_slice *quad, v2i32 texture_size);

/**
 *  @brief make a 9-slice quad.
 *
 *  @internal
 *
 *  makes 9 render-ready quads from an element's baked transform for 9-slice
 *  rendering.
 */
void ui_element_quad_9_slice_make_internal(fsl_ui_element *element,
        fsl_ui_quad_9_slice *quad, v2i32 texture_size);

/**
 *  @brief draw nothing, placeholder for non-drawable element types.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_none_draw_func(fsl_ui_element *element);

/**
 *  @brief draw a container.
 *
 *  @internal
 *
 *  only draws attached children.
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_container_draw_func(fsl_ui_element *element);

/**
 *  @brief draw a rectangle.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_rectangle_draw_func(fsl_ui_element *element);

/**
 *  @brief draw a sprite.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_sprite_draw_func(fsl_ui_element *element);

/**
 *  @brief draw a 9-slice panel.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_panel_9_slice_draw_func(fsl_ui_element *element);

/**
 *  @brief draw a button.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_button_draw_func(fsl_ui_element *element);

/**
 *  @brief draw a custom UI element.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_custom_draw_func(fsl_ui_element *element);

#endif /* FSL_UI_INTERNAL_H */
