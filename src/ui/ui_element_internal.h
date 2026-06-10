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
 *  @file ui_element_internal.h
 *
 *  @brief UI element internal definitions.
 */

#ifndef FSL_UI_ELEMENT_INTERNAL_H
#define FSL_UI_ELEMENT_INTERNAL_H

#include "ui_types.h"

/*!
 *  @internal
 *
 *  @brief bake a UI element's parameters to prepare for rendering.
 *
 *  @remark called automaically from @ref ui_element_draw().
 */
void ui_element_bake_internal(fsl_ui_element *element);

/*!
 *  @brief draw UI element and its children, recursively and in order of attachment.
 */
void ui_element_draw_internal(fsl_ui_element *element);

#endif /* FSL_UI_ELEMENT_INTERNAL_H */
