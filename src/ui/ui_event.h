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
 *  @file ui_event.h
 *
 *  @brief UI event definitions.
 */

#ifndef FSL_UI_EVENT_H
#define FSL_UI_EVENT_H

#include "../math/vector.h"

#include "ui_types.h"

typedef struct fsl_ui_event             fsl_ui_event;
typedef struct fsl_ui_event_callback    fsl_ui_event_callback;

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

typedef void (*fsl_ui_event_func)(fsl_ui_event event, void *data);

struct fsl_ui_event
{
    fsl_ui_element *caller;
    v2f64 mouse_pos;
    v2f64 mouse_click_pos;

    /**
     *  difference between mouse position and element position on mouse click.
     */
    v2f64 mouse_click_pos_diff;

    b8 hover;
    b8 hover_last;
    b8 mouse_click;
    b8 mouse_click_last;
}; /* fsl_ui_event */

struct fsl_ui_event_callback
{
    fsl_ui_event_func func;
    void *data;             /* data to pass into `func` */
}; /* fsl_ui_event_callback */

#endif /* FSL_UI_EVENT_H */
