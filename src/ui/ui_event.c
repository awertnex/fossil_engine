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
 *  @file ui_event.c
 *
 *  @brief input event handling for UI.
 */

#include "../math/vector.h"
#include "../input/input_internal.h"

#include "ui_internal.h"
#include "ui_element.h"
#include "ui_types.h"

static void (*ui_event_process_func_internal[FSL_UI_EVENT_TYPE_COUNT])(fsl_ui_element *element,
        fsl_ui_callback *callback) =
{
    0,
    ui_event_enter_process_internal,
    ui_event_hover_process_internal,
    ui_event_leave_process_internal,
    ui_event_click_process_internal,
    ui_event_hold_process_internal,
    ui_event_release_process_internal
};

void ui_element_listen_internal(fsl_ui_element *element, v2f64 mouse_pos, v2f64 mouse_delta)
{
    v2f64 area_start = {0};
    v2f64 area_end = {0};
    fsl_key_bind bind_click_left = {0};
    fsl_key_bind bind_click_right = {0};
    fsl_key_bind bind_click_middle = {0};
    u32 i = 0;

    element->event.caller = element;
    element->event.mouse_pos = mouse_pos;
    area_start.x = element->transform.pos_baked.x;
    area_start.y = element->transform.pos_baked.y;
    area_end.x = area_start.x + element->transform.size_baked.x;
    area_end.y = area_start.y + element->transform.size_baked.y;

    bind_click_left = key_bind_init_internal(FSL_MOUSE_BUTTON_LEFT, 0, 0, 0, 0);
    bind_click_right = key_bind_init_internal(FSL_MOUSE_BUTTON_RIGHT, 0, 0, 0, 0);
    bind_click_middle = key_bind_init_internal(FSL_MOUSE_BUTTON_MIDDLE, 0, 0, 0, 0);

    element->event.hover_last = element->event.hover;
    element->event.mouse_click_last = element->event.mouse_click;

    if (fsl_is_in_area_f64(mouse_pos, area_start, area_end))
        element->event.hover = TRUE;
    else
        element->event.hover = FALSE;

    if (element->event.hover &&
            (fsl_is_mouse_press(bind_click_left) || fsl_is_mouse_hold(bind_click_left) ||
             fsl_is_mouse_press(bind_click_right) || fsl_is_mouse_hold(bind_click_right) ||
             fsl_is_mouse_press(bind_click_middle) || fsl_is_mouse_hold(bind_click_middle)))
        element->event.mouse_click = TRUE;
    else
        element->event.mouse_click = FALSE;

    for (; i < FSL_UI_EVENT_TYPE_COUNT; ++i)
    {
        if (element->callback[i].func)
            ui_event_process_func_internal[i](element, &element->callback[i]);
    }
}

void ui_event_enter_process_internal(fsl_ui_element *element, fsl_ui_callback *callback)
{
    if (element->event.hover && !element->event.hover_last)
    {
        callback->func(element->event, callback->data);
    }
}

void ui_event_hover_process_internal(fsl_ui_element *element, fsl_ui_callback *callback)
{
    if (element->event.hover && element->event.hover_last)
    {
        callback->func(element->event, callback->data);
    }
}

void ui_event_leave_process_internal(fsl_ui_element *element, fsl_ui_callback *callback)
{
    if (!element->event.hover && element->event.hover_last)
    {
        callback->func(element->event, callback->data);
    }
}

void ui_event_click_process_internal(fsl_ui_element *element, fsl_ui_callback *callback)
{
    if (element->event.hover &&
            element->event.mouse_click && !element->event.mouse_click_last)
    {
        element->event.mouse_click_pos = element->event.mouse_pos;

        element->event.mouse_click_pos_diff.x =
            element->event.mouse_pos.x - element->transform.pos_baked.x;
        element->event.mouse_click_pos_diff.y =
            element->event.mouse_pos.y - element->transform.pos_baked.y;

        callback->func(element->event, callback->data);
    }
}

void ui_event_hold_process_internal(fsl_ui_element *element, fsl_ui_callback *callback)
{
    if (element->event.hover &&
            element->event.mouse_click && element->event.mouse_click_last)
    {
        callback->func(element->event, callback->data);
    }
}

void ui_event_release_process_internal(fsl_ui_element *element, fsl_ui_callback *callback)
{
    if (element->event.hover &&
            !element->event.mouse_click && element->event.mouse_click_last)
    {
        callback->func(element->event, callback->data);
    }
}
