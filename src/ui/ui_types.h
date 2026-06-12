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

#include "../math/vector.h"

typedef struct fsl_ui_event     fsl_ui_event;
typedef struct fsl_ui_callback  fsl_ui_callback;
typedef struct fsl_ui_transform fsl_ui_transform;
typedef struct fsl_ui_element   fsl_ui_element;

typedef enum fsl_ui_event_type
{
    FSL_UI_EVENT_TYPE_NONE,
    FSL_UI_EVENT_TYPE_ENTER,
    FSL_UI_EVENT_TYPE_HOVER,
    FSL_UI_EVENT_TYPE_LEAVE,
    FSL_UI_EVENT_TYPE_CLICK,
    FSL_UI_EVENT_TYPE_HOLD,
    FSL_UI_EVENT_TYPE_RELEASE,
    FSL_UI_EVENT_TYPE_SCROLL,
    FSL_UI_EVENT_TYPE_DRAG,
    FSL_UI_EVENT_TYPE_DROP,
    FSL_UI_EVENT_TYPE_COUNT
} fsl_ui_event_type;

struct fsl_ui_event
{
    fsl_ui_element *caller;
    v2f64 mouse_pos;
    v2f64 mouse_click_pos;

    /* difference between mouse position and element
     * position on mouse click.
     */
    v2f64 mouse_click_pos_diff;

    b8 hover;
    b8 hover_last;
    b8 mouse_click;
    b8 mouse_click_last;
}; /* fsl_ui_event */

typedef void (*fsl_ui_callback_func)(fsl_ui_event event, void *data);

struct fsl_ui_callback
{
    fsl_ui_callback_func func;
    void *data;
}; /* fsl_ui_callback */

struct fsl_ui_transform
{
    v2i32 uv_pos;           /* position in texture, in pixels */
    v2i32 uv_size;          /* size in texture, in pixels */
    i32 slice_size;         /* 9-slice slice diameter, in pixels (optional) */
    v2i32 pos;              /* position on screen, in pixels */
    v2i32 offset;           /* offset on screen from `pos`, in pixels */
    v2i32 offset_scaled;    /* offset on screen from `pos`, in pixels (scales with `scale`) */
    v2i32 size;             /* size on screen, in pixels */
    v2i32 size_scaled;      /* size on screen, in pixels (scales with `scale`) */
    v2f32 scale;            /* gui scaling, for 'scaled' parameters */

    /*!
     *  @brief alignment in respect to `pos`, `size` and `size_scaled`.
     *  <= -1: left-side/top is at `pos`.
     *  == 0: center is at `pos`.
     *  >= 1: right-side/bottom is at `pos`.
     */
    v2i32 align;

    v2f32 uv_pos_baked;     /* baked absolute position in texture, in normalized coordinates */
    v2f32 uv_size_baked;    /* baked absolute size in texture, in normalized coordinates */
    v2f32 pos_local;        /* baked local position, in pixels */
    v2f32 pos_baked;        /* baked absolute position on screen, in pixels */
    v2f32 size_baked;       /* baked absolute size on screen, in pixels */
}; /* fsl_ui_transform */

#endif /* FSL_UI_TYPES_H */
