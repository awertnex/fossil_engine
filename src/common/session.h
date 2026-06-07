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
 *  @file session.h
 *
 *  @brief engine's session info.
 */

#ifndef FSL_SESSION_H
#define FSL_SESSION_H

#include "api.h"
#include "types.h"

typedef struct fsl_engine_session fsl_engine_session;

struct fsl_engine_session
{
    /*!
     *  @brief POSIX timestamp of session's start, in millisecond format.
     */
    u64 init_time;

    /*!
     *  @brief running binary/executable's root directory.
     *
     *  @remark used to change current working dirctory to the running binary's.
     */
    str *bin_root;
}; /* fsl_engine_session */

/*!
 *  @remark declared and initialized in @ref fsl_engine_init().
 */
FSLAPI extern fsl_engine_session FSL_SESSION;

#endif /* FSL_SESSION_H */
