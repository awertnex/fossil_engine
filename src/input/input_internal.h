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
 *  @file input_internal.h
 *
 *  @brief input module internal definitions, so to keep verbose internals away from API code.
 */

#ifndef FSL_INPUT_INTERNAL_H
#define FSL_INPUT_INTERNAL_H

#include "../common/types.h"

#include "input.h"

#include "../external/glfw3.h"

enum keyboard_key_state
{
    STATE_KEY_IDLE,
    STATE_KEY_PRESS,
    STATE_KEY_HOLD,
    STATE_KEY_RELEASE,
    STATE_KEY_LISTEN_DOUBLE,
    STATE_KEY_PRESS_DOUBLE,
    STATE_KEY_HOLD_DOUBLE,
    STATE_KEY_RELEASE_DOUBLE
}; /* keyboard_key_state */

enum mod_key_flag
{
    FLAG_MOD_KEY_LEFT_SHIFT = 0x01,
    FLAG_MOD_KEY_LEFT_CONTROL = 0x02,
    FLAG_MOD_KEY_LEFT_ALT = 0x04,
    FLAG_MOD_KEY_LEFT_SUPER = 0x08,
    FLAG_MOD_KEY_RIGHT_SHIFT = 0x10,
    FLAG_MOD_KEY_RIGHT_CONTROL = 0x20,
    FLAG_MOD_KEY_RIGHT_ALT = 0x40,
    FLAG_MOD_KEY_RIGHT_SUPER = 0x80
}; /* mod_key_flag */

/*!
 *  @brief translation table to communicate engine conventions to GLFW.
 */
static u32 fsl_keyboard_tab[FSL_KEYBOARD_KEYS_MAX] =
{
    GLFW_KEY_SPACE,
    GLFW_KEY_APOSTROPHE,
    GLFW_KEY_COMMA,
    GLFW_KEY_MINUS,
    GLFW_KEY_PERIOD,
    GLFW_KEY_SLASH,
    GLFW_KEY_0,
    GLFW_KEY_1,
    GLFW_KEY_2,
    GLFW_KEY_3,
    GLFW_KEY_4,
    GLFW_KEY_5,
    GLFW_KEY_6,
    GLFW_KEY_7,
    GLFW_KEY_8,
    GLFW_KEY_9,
    GLFW_KEY_SEMICOLON,
    GLFW_KEY_EQUAL,
    GLFW_KEY_A,
    GLFW_KEY_B,
    GLFW_KEY_C,
    GLFW_KEY_D,
    GLFW_KEY_E,
    GLFW_KEY_F,
    GLFW_KEY_G,
    GLFW_KEY_H,
    GLFW_KEY_I,
    GLFW_KEY_J,
    GLFW_KEY_K,
    GLFW_KEY_L,
    GLFW_KEY_M,
    GLFW_KEY_N,
    GLFW_KEY_O,
    GLFW_KEY_P,
    GLFW_KEY_Q,
    GLFW_KEY_R,
    GLFW_KEY_S,
    GLFW_KEY_T,
    GLFW_KEY_U,
    GLFW_KEY_V,
    GLFW_KEY_W,
    GLFW_KEY_X,
    GLFW_KEY_Y,
    GLFW_KEY_Z,
    GLFW_KEY_LEFT_BRACKET,
    GLFW_KEY_BACKSLASH,
    GLFW_KEY_RIGHT_BRACKET,
    GLFW_KEY_GRAVE_ACCENT,
    GLFW_KEY_WORLD_1,
    GLFW_KEY_WORLD_2,

    GLFW_KEY_ESCAPE,
    GLFW_KEY_ENTER,
    GLFW_KEY_TAB,
    GLFW_KEY_BACKSPACE,
    GLFW_KEY_INSERT,
    GLFW_KEY_DELETE,
    GLFW_KEY_RIGHT,
    GLFW_KEY_LEFT,
    GLFW_KEY_DOWN,
    GLFW_KEY_UP,
    GLFW_KEY_PAGE_UP,
    GLFW_KEY_PAGE_DOWN,
    GLFW_KEY_HOME,
    GLFW_KEY_END,
    GLFW_KEY_CAPS_LOCK,
    GLFW_KEY_SCROLL_LOCK,
    GLFW_KEY_NUM_LOCK,
    GLFW_KEY_PRINT_SCREEN,
    GLFW_KEY_PAUSE,
    GLFW_KEY_F1,
    GLFW_KEY_F2,
    GLFW_KEY_F3,
    GLFW_KEY_F4,
    GLFW_KEY_F5,
    GLFW_KEY_F6,
    GLFW_KEY_F7,
    GLFW_KEY_F8,
    GLFW_KEY_F9,
    GLFW_KEY_F10,
    GLFW_KEY_F11,
    GLFW_KEY_F12,
    GLFW_KEY_F13,
    GLFW_KEY_F14,
    GLFW_KEY_F15,
    GLFW_KEY_F16,
    GLFW_KEY_F17,
    GLFW_KEY_F18,
    GLFW_KEY_F19,
    GLFW_KEY_F20,
    GLFW_KEY_F21,
    GLFW_KEY_F22,
    GLFW_KEY_F23,
    GLFW_KEY_F24,
    GLFW_KEY_F25,
    GLFW_KEY_KP_0,
    GLFW_KEY_KP_1,
    GLFW_KEY_KP_2,
    GLFW_KEY_KP_3,
    GLFW_KEY_KP_4,
    GLFW_KEY_KP_5,
    GLFW_KEY_KP_6,
    GLFW_KEY_KP_7,
    GLFW_KEY_KP_8,
    GLFW_KEY_KP_9,
    GLFW_KEY_KP_DECIMAL,
    GLFW_KEY_KP_DIVIDE,
    GLFW_KEY_KP_MULTIPLY,
    GLFW_KEY_KP_SUBTRACT,
    GLFW_KEY_KP_ADD,
    GLFW_KEY_KP_ENTER,
    GLFW_KEY_KP_EQUAL,
    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_LEFT_ALT,
    GLFW_KEY_LEFT_SUPER,
    GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_RIGHT_CONTROL,
    GLFW_KEY_RIGHT_ALT,
    GLFW_KEY_RIGHT_SUPER,
    GLFW_KEY_MENU
}; /* fsl_keyboard_tab */

/*!
 *  @brief keyboard key names.
 */
static str *fsl_mouse_button_name[FSL_MOUSE_BUTTONS_MAX] =
{
    "<lmb>",
    "<rmb>",
    "<mmb>",
    "<mb 4>",
    "<mb 5>",
    "<mb 6>",
    "<mb 7>",
    "<mb 8>"
};

/*!
 *  @brief keyboard key names.
 */
static str *fsl_keyboard_key_name[FSL_KEYBOARD_KEYS_MAX] =
{
    "<space>",
    "'",
    ",",
    "-",
    ".",
    "/",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    ";",
    "=",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "[",
    "\\",
    "]",
    "`",
    "<world 1>",
    "<world 2>",

    "<esc>",
    "<enter>",
    "<tab>",
    "<backspace>",
    "<ins>",
    "<del>",
    "<right>",
    "<left>",
    "<down>",
    "<up>",
    "<pgup>",
    "<pgdn>",
    "<home>",
    "<end>",
    "<caps>",
    "<scrlk>",
    "<num>",
    "<prtsc>",
    "<pause>",
    "<f1>",
    "<f2>",
    "<f3>",
    "<f4>",
    "<f5>",
    "<f6>",
    "<f7>",
    "<f8>",
    "<f9>",
    "<f10>",
    "<f11>",
    "<f12>",
    "<f13>",
    "<f14>",
    "<f15>",
    "<f16>",
    "<f17>",
    "<f18>",
    "<f19>",
    "<f20>",
    "<f21>",
    "<f22>",
    "<f23>",
    "<f24>",
    "<f25>",
    "<kp 0>",
    "<kp 1>",
    "<kp 2>",
    "<kp 3>",
    "<kp 4>",
    "<kp 5>",
    "<kp 6>",
    "<kp 7>",
    "<kp 8>",
    "<kp 9>",
    "<kp .>",
    "<kp />",
    "<kp *>",
    "<kp ->",
    "<kp +>",
    "<kp enter>",
    "<kp =>",
    "<lshift>",
    "<lctrl>",
    "<lalt>",
    "<lsuper>",
    "<rshift>",
    "<rctrl>",
    "<ralt>",
    "<rsuper>",
    "<menu>"
}; /* fsl_keyboard_tab */
#endif /* FSL_INPUT_INTERNAL_H */
