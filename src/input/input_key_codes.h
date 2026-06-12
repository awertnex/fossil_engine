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
 *  @file input_key_codes.h
 *
 *  @brief keyboard and mouse key codes.
 */

#ifndef FSL_INPUT_KEY_CODES_H
#define FSL_INPUT_KEY_CODES_H

#define GLFW_INCLUDE_NONE
#include "../external/glfw3.h"

typedef enum fsl_keyboard_key
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
} fsl_keyboard_key;

typedef enum fsl_mouse_button
{
    FSL_MOUSE_BUTTON_1 = GLFW_MOUSE_BUTTON_1,
    FSL_MOUSE_BUTTON_2 = GLFW_MOUSE_BUTTON_2,
    FSL_MOUSE_BUTTON_3 = GLFW_MOUSE_BUTTON_3,
    FSL_MOUSE_BUTTON_4 = GLFW_MOUSE_BUTTON_4,
    FSL_MOUSE_BUTTON_5 = GLFW_MOUSE_BUTTON_5,
    FSL_MOUSE_BUTTON_6 = GLFW_MOUSE_BUTTON_6,
    FSL_MOUSE_BUTTON_7 = GLFW_MOUSE_BUTTON_7,
    FSL_MOUSE_BUTTON_8 = GLFW_MOUSE_BUTTON_8,
    FSL_MOUSE_BUTTON_LEFT = GLFW_MOUSE_BUTTON_LEFT,
    FSL_MOUSE_BUTTON_RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
    FSL_MOUSE_BUTTON_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE
} fsl_mouse_button;

#endif /* FSL_INPUT_KEY_CODES_H */
