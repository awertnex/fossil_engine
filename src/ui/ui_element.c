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
 *  @brief everything about drawing ui elements.
 */

#include "../common/diagnostics.h"
#include "../logger/logger.h"
#include "../logger/logger_messages_internal.h"

#include "ui_internal.h"
#include "ui_types.h"

void fsl_ui_element_set_texture(fsl_ui_element *element, fsl_texture *texture)
{
    element->flag |= FSL_FLAG_UI_VISIBLE;
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

void ui_element_bake_internal(fsl_ui_element *element, v2f32 ndc_scale)
{
    fsl_ui_transform *s = &element->transform;
    v2f32 texture_scale = {0};
    v2f32 size = {0};
    v2f32 size_half = {0};
    v2f32 alignment = {0};
    v2f32 pos = {0};

    element->flag &= ~FSL_FLAG_UI_DIRTY_TRANSFORM;

    texture_scale.x = 1.0f / element->texture->size.x;
    texture_scale.y = 1.0f / element->texture->size.y;
    size.x = s->size.x + s->size_scaled.x * s->scale.x;
    size.y = s->size.y + s->size_scaled.y * s->scale.y;
    size_half.x = size.x / 2.0f;
    size_half.y = size.y / 2.0f;
    alignment.x = size_half.x + s->align.x * size_half.x;
    alignment.y = size_half.y + s->align.y * size_half.y;
    pos.x = s->pos.x + s->offset.x + s->offset_scaled.x * s->scale.x;
    pos.y = s->pos.y + s->offset.y + s->offset_scaled.y * s->scale.y;

    s->uv_pos_baked.x = s->uv_pos.x * texture_scale.x;
    s->uv_pos_baked.y = s->uv_pos.y * texture_scale.y;
    s->uv_size_baked.x = s->uv_size.x * texture_scale.x;
    s->uv_size_baked.y = s->uv_size.y * texture_scale.y;
    s->pos_baked.x = (pos.x - alignment.x) * ndc_scale.x;
    s->pos_baked.y = (pos.y - alignment.y) * ndc_scale.y;
    s->size_baked.x = size.x * ndc_scale.x;
    s->size_baked.y = size.y * ndc_scale.y;

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_UI_ELEMENT_BAKE(s->pos_baked.x, s->pos_baked.y, s->size_baked.x, s->size_baked.y));
}
