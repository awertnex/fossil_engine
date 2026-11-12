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

#include "../common/types.h"

#include "ui_element.h"
#include "ui_types.h"

/**
 *  @brief poll event data and parse element's callbacks (e.g., button pressed).
 *
 *  @internal
 *
 *  @remark called automatically from @ref fsl_ui_element_draw().
 */
void ui_element_listen_internal(fsl_ui_element *element, v2f64 mouse_pos, v2f64 mouse_delta);

/**
 *  @brief draw nothing, placeholder for non-drawable element types.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_none_draw_func(fsl_ui_element *element);

/**
 *  @brief draw a panel.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_panel_draw_func(fsl_ui_element *element);

/**
 *  @brief draw a 9-slice panel.
 *
 *  @internal
 *
 *  @note called indirectly via a function pointer array.
 */
void ui_element_panel_9_slice_draw_func(fsl_ui_element *element);

#endif /* FSL_UI_INTERNAL_H */
