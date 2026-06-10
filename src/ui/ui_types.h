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
 *  @file ui_types.h
 *
 *  @brief UI element types.
 */

#ifndef FSL_UI_TYPES_H
#define FSL_UI_TYPES_H

#include "../common/types.h"
#include "../assets/asset_types.h"
#include "../math/vector.h"
#include "../memory/memory_types.h"

typedef struct fsl_ui_event     fsl_ui_event;
typedef struct fsl_ui_callback  fsl_ui_callback;
typedef struct fsl_ui_sprite    fsl_ui_sprite;
typedef struct fsl_ui_element   fsl_ui_element;

/* ---- section: definitions ------------------------------------------------ */

enum fsl_ui_element_flag
{
    FSL_FLAG_UI_ACTIVE,
    FSL_FLAG_UI_VISIBLE,
    FSL_FLAG_UI_DIRTY_TRANSFORM,
    FSL_FLAG_UI_DIRTY_PARENT,
    FSL_FLAG_UI_DIRTY_CHILDREN
}; /* fsl_ui_element_flag */

typedef enum fsl_ui_element_type
{
    FSL_UI_ELEMENT_TYPE_NONE,
    FSL_UI_ELEMENT_TYPE_CONTAINER,
    FSL_UI_ELEMENT_TYPE_PANEL,
    FSL_UI_ELEMENT_TYPE_PANEL_9_SLICE,
    FSL_UI_ELEMENT_TYPE_BUTTON,
    FSL_UI_ELEMENT_TYPE_COUNT
} fsl_ui_element_type;

typedef enum fsl_ui_event_type
{
    FSL_UI_EVENT_TYPE_NONE,
    FSL_UI_EVENT_TYPE_ENTER,
    FSL_UI_EVENT_TYPE_LEAVE,
    FSL_UI_EVENT_TYPE_CLICK,
    FSL_UI_EVENT_TYPE_CLICK_DOUBLE,
    FSL_UI_EVENT_TYPE_HOLD,
    FSL_UI_EVENT_TYPE_HOLD_PERIOD,
    FSL_UI_EVENT_TYPE_RELEASE,
    FSL_UI_EVENT_TYPE_SCROLL,
    FSL_UI_EVENT_TYPE_DRAG,
    FSL_UI_EVENT_TYPE_DROP,
    FSL_UI_EVENT_TYPE_COUNT
} fsl_ui_event_type;

struct fsl_ui_event
{
    fsl_ui_event_type type;
    v2i64 pos;
}; /* fsl_ui_event */

struct fsl_ui_callback
{
    void (*callback)(fsl_ui_event event, void *data);
    fsl_ui_event event;
    void *data;
}; /* fsl_ui_callback */

struct fsl_ui_sprite
{
    v2i32 uv_pos;           /* position in texture, in pixels */
    v2i32 uv_size;          /* size in texture, in pixels */
    v2i32 pos;              /* position on screen, in pixels */
    v2i32 offset;           /* offset on screen from `pos`, in pixels */
    v2i32 offset_scaled;    /* offset on screen from `pos`, in pixels (scales with `scale`) */
    v2i32 size;             /* size on screen, in pixels */
    v2i32 size_scaled;      /* size on screen, in pixels (scales with `scale`) */
    v2f32 scale;            /* gui scaling, for sprite's 'scaled' parameters */

    /*!
     *  @brief alignment in respect to `pos`, `size` and `size_scaled`.
     *  <= -1: left-side/top of sprite is at `pos`.
     *  == 0: center of sprite is at `pos`.
     *  >= 1: right-side/bottom of sprite is at `pos`.
     */
    v2i32 align;

    v2f32 uv_pos_baked;     /* absolute, baked position in texture, in UV coordinates */
    v2f32 uv_size_baked;    /* absolute, baked size in texture, in UV coordinates */
    v2f32 pos_baked;        /* baked, absolute position on screen, in pixels */
    v2f32 size_baked;       /* baked, absolute size on screen, in pixels */
}; /* fsl_ui_sprite */

/* ---- section: definitions: elements -------------------------------------- */

struct fsl_ui_element
{
    fsl_ui_element_type type;
    fsl_texture *texture;
    fsl_ui_sprite sprite;

    u32 flag; /* enum @ref fsl_ui_element_flag */
    i32 layer;
    fsl_ui_callback action[FSL_UI_EVENT_TYPE_COUNT];
    fsl_ui_element *parent;
    fsl_mem_handle children_buf;
    fsl_ui_element **children;  /* cached pointer from `children_buf` */
}; /* fsl_ui_element */

#endif /* FSL_UI_TYPES_H */
