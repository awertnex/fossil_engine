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
 *  @file common_values.h
 *
 *  @brief engine's common values, default asset indices and global flag definitions.
 */

#ifndef FSL_COMMON_VALUES_H
#define FSL_COMMON_VALUES_H

enum fsl_flag
{
    FSL_FLAG_RELEASE_BUILD =        0x0001, /* output 'TRACE' and 'DEBUG' logs to console */
    FSL_FLAG_MULTISAMPLE =          0x0002  /* use 'GLFW' multisampling */
}; /* fsl_flag */

enum fsl_text_alignment
{
    FSL_TEXT_ALIGN_LEFT = 0,
    FSL_TEXT_ALIGN_CENTER = 1,
    FSL_TEXT_ALIGN_RIGHT = 2,
    FSL_TEXT_ALIGN_TOP = 0,
    FSL_TEXT_ALIGN_BOTTOM = 2
}; /* fsl_text_alignment */

#endif /* FSL_COMMON_VALUES_H */
