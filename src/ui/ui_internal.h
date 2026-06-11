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
 *  @file ui_internal.h
 *
 *  @brief UI internal definitions.
 */

#ifndef FSL_UI_INTERNAL_H
#define FSL_UI_INTERNAL_H

#include "../common/types.h"
#include "ui_element.h"

/*!
 *  @brief init text rendering settings (and engine default fonts at @ref fsl_font_buf).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 ui_text_init_internal(void);

void ui_text_align_x_none_internal(const str *text, i64 i, f32 advance);
void ui_text_align_x_center_internal(const str *text, i64 i, f32 advance);
void ui_text_align_x_right_internal(const str *text, i64 i, f32 advance);
void ui_text_align_y_none_internal(u64 end, f32 height);
void ui_text_align_y_center_internal(u64 end, f32 height);
void ui_text_align_y_bottom_internal(u64 end, f32 height);

/*!
 *  @internal
 *
 *  @brief bake a UI element's parameters to prepare for rendering.
 *
 *  @remark called automaically from @ref ui_element_draw().
 */
void ui_element_bake_internal(fsl_ui_element *element, v2f32 ndc_scale);

#endif /* FSL_UI_INTERNAL_H */
