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
 *  @file ui_element.c
 *
 *  @brief initializing, setting up, drawing and attaching/detaching UI elements
 *  to/from one another.
 */

#include "ui_element.h"
#include "ui_types.h"

#include <stddef.h>

void fsl_ui_element_set_texture(fsl_ui_element *element, fsl_texture *texture)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    element->texture = texture;
}

void fsl_ui_element_set_uv(fsl_ui_element *element,
        i32 uv_pos_x, i32 uv_pos_y, i32 uv_size_x, i32 uv_size_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    element->transform.uv_pos.x = uv_pos_x;
    element->transform.uv_pos.y = uv_pos_y;
    element->transform.uv_size.x = uv_size_x;
    element->transform.uv_size.y = uv_size_y;
}

void fsl_ui_element_set_position(fsl_ui_element *element, i32 pos_x, i32 pos_y,
        i32 offset_x, i32 offset_y, i32 offset_scaled_x, i32 offset_scaled_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    element->transform.pos.x = pos_x;
    element->transform.pos.y = pos_y;
    element->transform.offset.x = offset_x;
    element->transform.offset.y = offset_y;
    element->transform.offset_scaled.x = offset_scaled_x;
    element->transform.offset_scaled.y = offset_scaled_y;
}

void fsl_ui_element_set_size(fsl_ui_element *element,
        i32 size_x, i32 size_y, i32 size_scaled_x, i32 size_scaled_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    element->transform.size.x = size_x;
    element->transform.size.y = size_y;
    element->transform.size_scaled.x = size_scaled_x;
    element->transform.size_scaled.y = size_scaled_y;
}

void fsl_ui_element_set_scale(fsl_ui_element *element, f32 scale_x, f32 scale_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    element->transform.scale.x = scale_x;
    element->transform.scale.y = scale_y;
}

void fsl_ui_element_set_alignment(fsl_ui_element *element, i32 align_x, i32 align_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    element->transform.align.x = align_x;
    element->transform.align.y = align_y;
}

void fsl_ui_element_set_9_slice(fsl_ui_element *element, b8 is_9_slice, i32 slice_size)
{
    if (is_9_slice)
        element->flag |= FSL_FLAG_UI_9_SLICE;
    else
        element->flag &= ~FSL_FLAG_UI_9_SLICE;
    element->transform.slice_size = slice_size;
}

void fsl_ui_element_set_callback(fsl_ui_element *element,
        fsl_ui_event_type event_type,
        void (*func)(fsl_ui_event event, void *data), void *data)
{
    element->callback[event_type].func = func;
    element->callback[event_type].data = data;
}

void fsl_ui_element_attach(fsl_ui_element *parent, fsl_ui_element *child)
{
    parent->flag |= FSL_FLAG_UI_DIRTY_CHILDREN;
    child->flag |= FSL_FLAG_UI_DIRTY_PARENT;
    child->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    child->parent = parent;
}

void fsl_ui_element_detach(fsl_ui_element *child)
{
    if (child->parent)
        child->parent->flag |= FSL_FLAG_UI_DIRTY_CHILDREN;

    child->flag |= FSL_FLAG_UI_DIRTY_PARENT;
    child->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    child->parent = NULL;
}
