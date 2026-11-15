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
 *  @file ui_element.c
 *
 *  @brief initializing, setting up, drawing and attaching/detaching UI
 *  elements.
 */


#include "../logger/logger.h"
#include "../logger/logger_messages_internal.h"

#include "ui_config.h"
#include "ui_element.h"
#include "ui_internal.h"
#include "ui_types.h"

#include <stddef.h>

/**
 *  @brief define capabilities of every element type.
 */
static u64 ui_element_capability_def_internal[FSL_UI_ELEMENT_TYPE_COUNT] =
{
    0,

    FSL_FLAG_UI_CAN_EXPAND_X |
        FSL_FLAG_UI_CAN_EXPAND_Y,

    FSL_FLAG_UI_CAN_TEXTURE |
        FSL_FLAG_UI_CAN_EXPAND_X |
        FSL_FLAG_UI_CAN_EXPAND_Y,

    FSL_FLAG_UI_CAN_TEXTURE |
        FSL_FLAG_UI_CAN_9_SLICE |
        FSL_FLAG_UI_CAN_EXPAND_X |
        FSL_FLAG_UI_CAN_EXPAND_Y,

    FSL_FLAG_UI_CAN_TEXTURE |
        FSL_FLAG_UI_CAN_9_SLICE |
        FSL_FLAG_UI_CAN_EXPAND_X |
        FSL_FLAG_UI_CAN_EXPAND_Y,

    FSL_FLAG_UI_CAN_EXPAND_X |
        FSL_FLAG_UI_CAN_EXPAND_Y |
        FSL_FLAG_UI_CAN_CUSTOM_DRAW
};

void fsl_ui_element_type_set_func(fsl_ui_element *element, fsl_ui_element_type type)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;

    element->type = type;
    element->capabilities = ui_element_capability_def_internal[type];
}

void fsl_ui_element_texture_set_func(fsl_ui_element *element, fsl_texture *texture)
{
    if (!(element->capabilities & FSL_FLAG_UI_CAN_TEXTURE))
    {
        LOGERROR(FSL_ERR_TYPE_MISMATCH, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Set UI Element Texture", "Element Does Support Textures"));
        return;
    }

    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;

    switch (element->type)
    {
        case FSL_UI_ELEMENT_TYPE_SPRITE:
            ((fsl_ui_element_sprite*)element)->texture = texture;
            break;

        case FSL_UI_ELEMENT_TYPE_PANEL_9_SLICE:
            ((fsl_ui_element_panel_9_slice*)element)->texture = texture;
            break;

        case FSL_UI_ELEMENT_TYPE_BUTTON:
            ((fsl_ui_element_button*)element)->texture = texture;
            break;

        default:
            break;
    }
}

void fsl_ui_element_uv_set_func(fsl_ui_element *element,
        i32 uv_pos_x, i32 uv_pos_y, i32 uv_size_x, i32 uv_size_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;

    element->transform.uv_pos.x = uv_pos_x;
    element->transform.uv_pos.y = uv_pos_y;
    element->transform.uv_size.x = uv_size_x;
    element->transform.uv_size.y = uv_size_y;
}

void fsl_ui_element_position_set_func(fsl_ui_element *element,
        i32 pos_x, i32 pos_y, i32 offset_x, i32 offset_y,
        i32 offset_scaled_x, i32 offset_scaled_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;

    element->transform.pos.x = pos_x;
    element->transform.pos.y = pos_y;
    element->transform.offset.x = offset_x;
    element->transform.offset.y = offset_y;
    element->transform.offset_scaled.x = offset_scaled_x;
    element->transform.offset_scaled.y = offset_scaled_y;
}

void fsl_ui_element_size_set_func(fsl_ui_element *element,
        i32 size_x, i32 size_y, i32 size_scaled_x, i32 size_scaled_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;

    element->transform.size.x = size_x;
    element->transform.size.y = size_y;
    element->transform.size_scaled.x = size_scaled_x;
    element->transform.size_scaled.y = size_scaled_y;
}

void fsl_ui_element_scale_set_func(fsl_ui_element *element,
        f32 scale_x, f32 scale_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;

    element->transform.scale.x = scale_x;
    element->transform.scale.y = scale_y;
}

void fsl_ui_element_alignment_set_func(fsl_ui_element *element,
        i32 align_x, i32 align_y)
{
    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;

    element->transform.align.x = align_x;
    element->transform.align.y = align_y;
}

void fsl_ui_element_9_slice_set_func(fsl_ui_element *element,
        b8 is_9_slice, i32 slice_size)
{
    (void)is_9_slice;

    if (!(element->capabilities & FSL_FLAG_UI_CAN_9_SLICE))
    {
        LOGERROR(FSL_ERR_TYPE_MISMATCH, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Set UI Element Slice Size", "Element Does Support 9-Slice"));
        return;
    }

    element->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;

    if (slice_size <= 0)
        slice_size = FSL_UI_SLICE_SIZE_DEFAULT;

    switch (element->type)
    {
        case FSL_UI_ELEMENT_TYPE_PANEL_9_SLICE:
            ((fsl_ui_element_panel_9_slice*)element)->slice_size = slice_size;
            break;

        case FSL_UI_ELEMENT_TYPE_BUTTON:
            ((fsl_ui_element_button*)element)->slice_size = slice_size;
            break;

        default:
            break;
    }
}

void fsl_ui_element_callback_set_func(fsl_ui_element *element,
        fsl_ui_event_type event_type, fsl_ui_event_func func, void *data)
{
    element->event_func[event_type].func = func;
    element->event_func[event_type].data = data;
}

void fsl_ui_element_draw_callback_set_func(fsl_ui_element *element,
        fsl_ui_draw_func func, void *data, fsl_fbo *fbo)
{
    if (!(element->capabilities & FSL_FLAG_UI_CAN_CUSTOM_DRAW))
    {
        LOGERROR(FSL_ERR_TYPE_MISMATCH, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Set UI Element Draw Callback", "Element Does Support Custom Draw"));
        return;
    }

    switch (element->type)
    {
        case FSL_UI_ELEMENT_TYPE_CUSTOM:
            ((fsl_ui_element_custom*)element)->draw_func.func = func;
            ((fsl_ui_element_custom*)element)->draw_func.data = data;
            ((fsl_ui_element_custom*)element)->draw_func.fbo = fbo;
            break;

        default:
            break;
    }
}

void fsl_ui_element_attach_func(fsl_ui_element *parent, fsl_ui_element *child)
{
    if (!parent)
        return;

    parent->flag |= FSL_FLAG_UI_DIRTY_CHILDREN;
    child->flag |= FSL_FLAG_UI_DIRTY_PARENT;
    child->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    child->parent = parent;
}

void fsl_ui_element_detach_func(fsl_ui_element *child)
{
    if (child->parent)
        child->parent->flag |= FSL_FLAG_UI_DIRTY_CHILDREN;

    child->flag |= FSL_FLAG_UI_DIRTY_PARENT;
    child->flag |= FSL_FLAG_UI_DIRTY_TRANSFORM;
    child->parent = NULL;
}
