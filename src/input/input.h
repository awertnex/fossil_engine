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
 *  @file input.h
 *
 *  @brief keyboard and mouse input handling.
 */

#ifndef FSL_INPUT_H
#define FSL_INPUT_H

#include "../common/api.h"
#include "../common/types.h"

#include "input_key_codes.h"

#define FSL_MOUSE_BUTTONS_MAX 8
#define FSL_KEYBOARD_KEYS_MAX 120
#define FSL_DOUBLE_PRESS_TIME_INTERVAL 0.25

typedef struct fsl_key_bind fsl_key_bind;

typedef enum fsl_mod_key
{
    FSL_SHIFT_LEFT = 1,
    FSL_SHIFT_RIGHT = 2,
    FSL_CONTROL_LEFT = 1,
    FSL_CONTROL_RIGHT = 2,
    FSL_ALT_LEFT = 1,
    FSL_ALT_RIGHT = 2,
    FSL_SUPER_LEFT = 1,
    FSL_SUPER_RIGHT = 2
} fsl_mod_key;

struct fsl_key_bind
{
    u32 key;
    u32 mod; /* enum @ref fsl_mod_key_flag (internal enum) */
}; /* fsl_key_bind */

/*!
 *  @brief setup a key binding (key combination).
 *
 *  @param key a keyboard key or mouse button.
 *  @param shift left or right shift.
 *  @param ctrl left or right ctrl.
 *  @param alt left or right alt.
 *  @param super left or right super.
 *
 *
 *  @return `fsl_key_bind{0}` on failure and @ref fsl_err is set accordingly.
 */
FSLAPI fsl_key_bind fsl_key_bind_init(fsl_keyboard_key key,
        fsl_mod_key shift, fsl_mod_key ctrl, fsl_mod_key alt, fsl_mod_key super);

FSLAPI b8 fsl_is_mouse_press(fsl_key_bind button);
FSLAPI b8 fsl_is_mouse_hold(fsl_key_bind button);
FSLAPI b8 fsl_is_mouse_release(fsl_key_bind button);
FSLAPI b8 fsl_is_key_press(fsl_key_bind key);
FSLAPI b8 fsl_is_key_press_double(fsl_key_bind key);
FSLAPI b8 fsl_is_key_hold(fsl_key_bind key);
FSLAPI b8 fsl_is_key_release(fsl_key_bind key);

#endif /* FSL_INPUT_H */
