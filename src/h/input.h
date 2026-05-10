/*  @file input.h
 *
 *  @brief keyboard and mouse input handling.
 *
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

#ifndef FSL_INPUT_H
#define FSL_INPUT_H

#include "common.h"
#include "types.h"

#define FSL_MOUSE_BUTTONS_MAX 8
#define FSL_KEYBOARD_KEYS_MAX 120
#define FSL_DOUBLE_PRESS_TIME_INTERVAL 0.5f

typedef struct key_bind fsl_key_bind;

enum fsl_keyboard_key
{
    FSL_KEY_SPACE,
    FSL_KEY_APOSTROPHE,
    FSL_KEY_COMMA,
    FSL_KEY_MINUS,
    FSL_KEY_PERIOD,
    FSL_KEY_SLASH,
    FSL_KEY_0,
    FSL_KEY_1,
    FSL_KEY_2,
    FSL_KEY_3,
    FSL_KEY_4,
    FSL_KEY_5,
    FSL_KEY_6,
    FSL_KEY_7,
    FSL_KEY_8,
    FSL_KEY_9,
    FSL_KEY_SEMICOLON,
    FSL_KEY_EQUAL,
    FSL_KEY_A,
    FSL_KEY_B,
    FSL_KEY_C,
    FSL_KEY_D,
    FSL_KEY_E,
    FSL_KEY_F,
    FSL_KEY_G,
    FSL_KEY_H,
    FSL_KEY_I,
    FSL_KEY_J,
    FSL_KEY_K,
    FSL_KEY_L,
    FSL_KEY_M,
    FSL_KEY_N,
    FSL_KEY_O,
    FSL_KEY_P,
    FSL_KEY_Q,
    FSL_KEY_R,
    FSL_KEY_S,
    FSL_KEY_T,
    FSL_KEY_U,
    FSL_KEY_V,
    FSL_KEY_W,
    FSL_KEY_X,
    FSL_KEY_Y,
    FSL_KEY_Z,
    FSL_KEY_LEFT_BRACKET,
    FSL_KEY_BACKSLASH,
    FSL_KEY_RIGHT_BRACKET,
    FSL_KEY_GRAVE_ACCENT,
    FSL_KEY_WORLD_1,
    FSL_KEY_WORLD_2,

    FSL_KEY_ESCAPE,
    FSL_KEY_ENTER,
    FSL_KEY_TAB,
    FSL_KEY_BACKSPACE,
    FSL_KEY_INSERT,
    FSL_KEY_DELETE,
    FSL_KEY_RIGHT,
    FSL_KEY_LEFT,
    FSL_KEY_DOWN,
    FSL_KEY_UP,
    FSL_KEY_PAGE_UP,
    FSL_KEY_PAGE_DOWN,
    FSL_KEY_HOME,
    FSL_KEY_END,
    FSL_KEY_CAPS_LOCK,
    FSL_KEY_SCROLL_LOCK,
    FSL_KEY_NUM_LOCK,
    FSL_KEY_PRINT_SCREEN,
    FSL_KEY_PAUSE,
    FSL_KEY_F1,
    FSL_KEY_F2,
    FSL_KEY_F3,
    FSL_KEY_F4,
    FSL_KEY_F5,
    FSL_KEY_F6,
    FSL_KEY_F7,
    FSL_KEY_F8,
    FSL_KEY_F9,
    FSL_KEY_F10,
    FSL_KEY_F11,
    FSL_KEY_F12,
    FSL_KEY_F13,
    FSL_KEY_F14,
    FSL_KEY_F15,
    FSL_KEY_F16,
    FSL_KEY_F17,
    FSL_KEY_F18,
    FSL_KEY_F19,
    FSL_KEY_F20,
    FSL_KEY_F21,
    FSL_KEY_F22,
    FSL_KEY_F23,
    FSL_KEY_F24,
    FSL_KEY_F25,
    FSL_KEY_KP_0,
    FSL_KEY_KP_1,
    FSL_KEY_KP_2,
    FSL_KEY_KP_3,
    FSL_KEY_KP_4,
    FSL_KEY_KP_5,
    FSL_KEY_KP_6,
    FSL_KEY_KP_7,
    FSL_KEY_KP_8,
    FSL_KEY_KP_9,
    FSL_KEY_KP_DECIMAL,
    FSL_KEY_KP_DIVIDE,
    FSL_KEY_KP_MULTIPLY,
    FSL_KEY_KP_SUBTRACT,
    FSL_KEY_KP_ADD,
    FSL_KEY_KP_ENTER,
    FSL_KEY_KP_EQUAL,
    FSL_KEY_LEFT_SHIFT,
    FSL_KEY_LEFT_CONTROL,
    FSL_KEY_LEFT_ALT,
    FSL_KEY_LEFT_SUPER,
    FSL_KEY_RIGHT_SHIFT,
    FSL_KEY_RIGHT_CONTROL,
    FSL_KEY_RIGHT_ALT,
    FSL_KEY_RIGHT_SUPER,
    FSL_KEY_MENU
}; /* fsl_keyboard_key */

enum fsl_mod_key
{
    FSL_SHIFT_LEFT = 1,
    FSL_SHIFT_RIGHT,
    FSL_CONTROL_LEFT = 1,
    FSL_CONTROL_RIGHT,
    FSL_ALT_LEFT = 1,
    FSL_ALT_RIGHT,
    FSL_SUPER_LEFT = 1,
    FSL_SUPER_RIGHT
}; /* fsl_mod_key */

struct key_bind
{
    u32 key;
    u32 mod; /* enum @ref fsl_mod_key_flag (internal enum) */
}; /* key_bind */

/*! @brief setup a key binding (key combination).
 *
 *  @param key a keyboard key or mouse button.
 */
FSLAPI fsl_key_bind fsl_key_bind_init(enum fsl_keyboard_key key,
        enum fsl_mod_key shift, enum fsl_mod_key ctrl,
        enum fsl_mod_key alt, enum fsl_mod_key super);

FSLAPI b8 fsl_is_mouse_press(const fsl_key_bind button);
FSLAPI b8 fsl_is_mouse_hold(const fsl_key_bind button);
FSLAPI b8 fsl_is_mouse_release(const fsl_key_bind button);
FSLAPI b8 fsl_is_key_press(const fsl_key_bind key);
FSLAPI b8 fsl_is_key_press_double(const fsl_key_bind key);
FSLAPI b8 fsl_is_key_hold(const fsl_key_bind key);
FSLAPI b8 fsl_is_key_release(const fsl_key_bind key);

/*! @brief update mouse movement.
 *
 *  - update parameters at @ref fsl_render.mouse_pos and @ref fsl_render.mouse_delta
 *    of the currently bound `fsl_render`.
 *
 *  @remark called automatically from @ref fsl_engine_running().
 */
FSLAPI void fsl_update_mouse_movement(void);

/*! @brief update internal mouse and key states: press, double-press, hold and release.
 *
 *  @remark called automatically from @ref fsl_engine_running().
 */
FSLAPI void fsl_update_key_states(void);

#endif /* FSL_INPUT_H */
