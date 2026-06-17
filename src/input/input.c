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
 *  @file input.c
 *
 *  @brief keyboard and mouse input handling.
 */

#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../common/types.h"
#include "../engine/engine.h"
#include "../logger/logger.h"
#include "../logger/logger_messages_internal.h"
#include "../math/vector.h"

#include "input.h"
#include "input_internal.h"

#include "../h/time.h"

#include <stdio.h>

static u32 mouse_button[FSL_MOUSE_BUTTONS_MAX] = {0};
static u32 keyboard_key[FSL_KEYBOARD_KEYS_MAX] = {0};

/* TODO: make function `fsl_key_bind_init()` aware of the type of device (e.g., keyboard or mouse), preferrably internally */
fsl_key_bind fsl_key_bind_init(fsl_keyboard_key key,
        fsl_mod_key shift, fsl_mod_key ctrl, fsl_mod_key alt, fsl_mod_key super)
{
    fsl_key_bind bind = {0};
    str str_key[FSL_ID_CAP] = {0};
    str str_mod[4][FSL_ID_CAP] = {0};

    snprintf(str_key, FSL_ID_CAP, "%u", key);
    if (key >= FSL_KEYBOARD_KEYS_MAX)
    {
        LOGERROR(FSL_ERR_OUT_OF_BOUNDS, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Initialize Key Bind", str_key, "Out of Bounds"));
        return bind;
    }

    snprintf(str_key, FSL_ID_CAP, "%s", fsl_keyboard_key_name[key]);

    bind.key = key;

    switch (shift)
    {
        case FSL_SHIFT_LEFT:
            bind.mod |= FLAG_MOD_KEY_LEFT_SHIFT;
            snprintf(str_mod[0], FSL_ID_CAP, " + %s", fsl_keyboard_key_name[FSL_KEY_LEFT_SHIFT]);
            break;

        case FSL_SHIFT_RIGHT:
            bind.mod |= FLAG_MOD_KEY_RIGHT_SHIFT;
            snprintf(str_mod[0], FSL_ID_CAP, " + %s", fsl_keyboard_key_name[FSL_KEY_RIGHT_SHIFT]);
            break;
    }

    switch (ctrl)
    {
        case FSL_CONTROL_LEFT:
            bind.mod |= FLAG_MOD_KEY_LEFT_CONTROL;
            snprintf(str_mod[1], FSL_ID_CAP, " + %s", fsl_keyboard_key_name[FSL_KEY_LEFT_CONTROL]);
            break;

        case FSL_CONTROL_RIGHT:
            bind.mod |= FLAG_MOD_KEY_RIGHT_CONTROL;
            snprintf(str_mod[1], FSL_ID_CAP, " + %s", fsl_keyboard_key_name[FSL_KEY_RIGHT_CONTROL]);
            break;
    }

    switch (alt)
    {
        case FSL_ALT_LEFT:
            bind.mod |= FLAG_MOD_KEY_LEFT_ALT;
            snprintf(str_mod[2], FSL_ID_CAP, " + %s", fsl_keyboard_key_name[FSL_KEY_LEFT_ALT]);
            break;

        case FSL_ALT_RIGHT:
            bind.mod |= FLAG_MOD_KEY_RIGHT_ALT;
            snprintf(str_mod[2], FSL_ID_CAP, " + %s", fsl_keyboard_key_name[FSL_KEY_RIGHT_ALT]);
            break;
    }

    switch (super)
    {
        case FSL_SUPER_LEFT:
            bind.mod |= FLAG_MOD_KEY_LEFT_SUPER;
            snprintf(str_mod[3], FSL_ID_CAP, " + %s", fsl_keyboard_key_name[FSL_KEY_LEFT_SUPER]);
            break;

        case FSL_SUPER_RIGHT:
            bind.mod |= FLAG_MOD_KEY_RIGHT_SUPER;
            snprintf(str_mod[3], FSL_ID_CAP, " + %s", fsl_keyboard_key_name[FSL_KEY_RIGHT_SUPER]);
            break;
    }

    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_INPUT_KEY_BIND_INIT(str_key, str_mod[0], str_mod[1], str_mod[2], str_mod[3]));

    return bind;
}

fsl_key_bind key_bind_init_internal(fsl_keyboard_key key,
        fsl_mod_key shift, fsl_mod_key ctrl, fsl_mod_key alt, fsl_mod_key super)
{
    fsl_key_bind bind = {0};

    bind.key = key;

    switch (shift)
    {
        case FSL_SHIFT_LEFT:
            bind.mod |= FLAG_MOD_KEY_LEFT_SHIFT;
            break;

        case FSL_SHIFT_RIGHT:
            bind.mod |= FLAG_MOD_KEY_RIGHT_SHIFT;
            break;
    }

    switch (ctrl)
    {
        case FSL_CONTROL_LEFT:
            bind.mod |= FLAG_MOD_KEY_LEFT_CONTROL;
            break;

        case FSL_CONTROL_RIGHT:
            bind.mod |= FLAG_MOD_KEY_RIGHT_CONTROL;
            break;
    }

    switch (alt)
    {
        case FSL_ALT_LEFT:
            bind.mod |= FLAG_MOD_KEY_LEFT_ALT;
            break;

        case FSL_ALT_RIGHT:
            bind.mod |= FLAG_MOD_KEY_RIGHT_ALT;
            break;
    }

    switch (super)
    {
        case FSL_SUPER_LEFT:
            bind.mod |= FLAG_MOD_KEY_LEFT_SUPER;
            break;

        case FSL_SUPER_RIGHT:
            bind.mod |= FLAG_MOD_KEY_RIGHT_SUPER;
            break;
    }

    return bind;
}

b8 fsl_is_mouse_press(fsl_key_bind button)
{
    return mouse_button[button.key] == STATE_INPUT_PRESS;
}

b8 fsl_is_mouse_hold(fsl_key_bind button)
{
    return mouse_button[button.key] == STATE_INPUT_HOLD;
}

b8 fsl_is_mouse_release(fsl_key_bind button)
{
    return mouse_button[button.key] == STATE_INPUT_RELEASE;
}

b8 fsl_is_key_press(fsl_key_bind key)
{
    b8 result = TRUE;

    if (
            keyboard_key[key.key] != STATE_INPUT_PRESS &&
            keyboard_key[key.key] != STATE_INPUT_PRESS_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_LEFT_SHIFT) &&
            keyboard_key[FSL_KEY_LEFT_SHIFT] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_LEFT_SHIFT] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_LEFT_CONTROL) &&
            keyboard_key[FSL_KEY_LEFT_CONTROL] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_LEFT_CONTROL] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_LEFT_ALT) &&
            keyboard_key[FSL_KEY_LEFT_ALT] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_LEFT_ALT] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_LEFT_SUPER) &&
            keyboard_key[FSL_KEY_LEFT_SUPER] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_LEFT_SUPER] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_RIGHT_SHIFT) &&
            keyboard_key[FSL_KEY_RIGHT_SHIFT] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_RIGHT_SHIFT] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_RIGHT_CONTROL) &&
            keyboard_key[FSL_KEY_RIGHT_CONTROL] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_RIGHT_CONTROL] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_RIGHT_ALT) &&
            keyboard_key[FSL_KEY_RIGHT_ALT] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_RIGHT_ALT] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_RIGHT_SUPER) &&
            keyboard_key[FSL_KEY_RIGHT_SUPER] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_RIGHT_SUPER] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    return result;
}

b8 fsl_is_key_press_double(fsl_key_bind key)
{
    return keyboard_key[key.key] == STATE_INPUT_PRESS_DOUBLE;
}

b8 fsl_is_key_hold(fsl_key_bind key)
{
    b8 result = TRUE;

    if (
            keyboard_key[key.key] != STATE_INPUT_HOLD &&
            keyboard_key[key.key] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_LEFT_SHIFT) &&
            keyboard_key[FSL_KEY_LEFT_SHIFT] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_LEFT_SHIFT] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_LEFT_CONTROL) &&
            keyboard_key[FSL_KEY_LEFT_CONTROL] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_LEFT_CONTROL] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_LEFT_ALT) &&
            keyboard_key[FSL_KEY_LEFT_ALT] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_LEFT_ALT] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_LEFT_SUPER) &&
            keyboard_key[FSL_KEY_LEFT_SUPER] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_LEFT_SUPER] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_RIGHT_SHIFT) &&
            keyboard_key[FSL_KEY_RIGHT_SHIFT] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_RIGHT_SHIFT] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_RIGHT_CONTROL) &&
            keyboard_key[FSL_KEY_RIGHT_CONTROL] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_RIGHT_CONTROL] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_RIGHT_ALT) &&
            keyboard_key[FSL_KEY_RIGHT_ALT] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_RIGHT_ALT] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    if (
            (key.mod & FLAG_MOD_KEY_RIGHT_SUPER) &&
            keyboard_key[FSL_KEY_RIGHT_SUPER] != STATE_INPUT_HOLD &&
            keyboard_key[FSL_KEY_RIGHT_SUPER] != STATE_INPUT_HOLD_DOUBLE)
        result = FALSE;

    return result;
}

b8 fsl_is_key_release(fsl_key_bind key)
{
    return
        keyboard_key[key.key] == STATE_INPUT_RELEASE ||
        keyboard_key[key.key] == STATE_INPUT_RELEASE_DOUBLE;
}

void input_mouse_movement_update_internal(void)
{
    static v2f64 mouse_last = {0};

    glfwGetCursorPos(render_internal.window,
            &render_internal.mouse_pos.x, &render_internal.mouse_pos.y);
    render_internal.mouse_delta.x = render_internal.mouse_pos.x - mouse_last.x;
    render_internal.mouse_delta.y = render_internal.mouse_pos.y - mouse_last.y;
    mouse_last = render_internal.mouse_pos;
}

void input_key_states_update_internal(void)
{
    GLFWwindow *window = render_internal.window;
    u64 _time = render_internal.time;
    static u64 double_press_time_interval = (u64)(FSL_DOUBLE_PRESS_TIME_INTERVAL * FSL_SEC2NSEC);
    static u64 key_press_start_time[FSL_KEYBOARD_KEYS_MAX] = {0};
    b8 key_press = FALSE;
    b8 key_release = FALSE;
    b8 mouse_press = FALSE;
    b8 mouse_release = FALSE;
    u32 i = 0;

    /* ---- listen mouse input ---------------------------------------------- */

    for (i = 0; i < FSL_MOUSE_BUTTONS_MAX; ++i)
    {
        mouse_press = glfwGetMouseButton(window, i) == GLFW_PRESS;
        mouse_release = glfwGetMouseButton(window, i) == GLFW_RELEASE;

        if (mouse_press &&
                (mouse_button[i] == STATE_INPUT_IDLE))
        {
            mouse_button[i] = STATE_INPUT_PRESS;
            continue;
        }
        else if (mouse_release &&
                (mouse_button[i] == STATE_INPUT_PRESS || mouse_button[i] == STATE_INPUT_HOLD))
        {
            mouse_button[i] = STATE_INPUT_RELEASE;
            continue;
        }

        if (mouse_button[i] == STATE_INPUT_PRESS)         mouse_button[i] = STATE_INPUT_HOLD;
        else if (mouse_button[i] == STATE_INPUT_RELEASE)  mouse_button[i] = STATE_INPUT_IDLE;
    }

    /* ---- listen keyboard input ------------------------------------------- */

    for (i = 0; i < FSL_KEYBOARD_KEYS_MAX; ++i)
    {
        key_press = glfwGetKey(window, fsl_keyboard_tab[i]) == GLFW_PRESS;
        key_release = glfwGetKey(window, fsl_keyboard_tab[i]) == GLFW_RELEASE;

        if (key_press)
        {
            if (keyboard_key[i] == STATE_INPUT_IDLE)
            {
                keyboard_key[i] = STATE_INPUT_PRESS;
                key_press_start_time[i] = _time;
                continue;
            }
            else if (keyboard_key[i] == STATE_INPUT_LISTEN_DOUBLE)
            {
                if (_time - key_press_start_time[i] <= double_press_time_interval)
                    keyboard_key[i] = STATE_INPUT_PRESS_DOUBLE;
                else
                {
                    keyboard_key[i] = STATE_INPUT_PRESS;
                    key_press_start_time[i] = _time;
                }
                continue;
            }
        }
        else if (key_release)
        {
            if (keyboard_key[i] == STATE_INPUT_PRESS ||
                    keyboard_key[i] == STATE_INPUT_HOLD)
            {
                keyboard_key[i] = STATE_INPUT_RELEASE;
                continue;
            }
            else if (keyboard_key[i] == STATE_INPUT_PRESS_DOUBLE ||
                    keyboard_key[i] == STATE_INPUT_HOLD_DOUBLE)
            {
                keyboard_key[i] = STATE_INPUT_RELEASE_DOUBLE;
                continue;
            }
        }

        if (keyboard_key[i] == STATE_INPUT_PRESS)
            keyboard_key[i] = STATE_INPUT_HOLD;
        else if (keyboard_key[i] == STATE_INPUT_RELEASE)
            keyboard_key[i] = STATE_INPUT_LISTEN_DOUBLE;

        if (keyboard_key[i] == STATE_INPUT_PRESS_DOUBLE)
            keyboard_key[i] = STATE_INPUT_HOLD_DOUBLE;
        else if (keyboard_key[i] == STATE_INPUT_RELEASE_DOUBLE)
            keyboard_key[i] = STATE_INPUT_IDLE;
    }
}
