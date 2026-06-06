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

/*!
 *  @brief POSIX timestamp of the main process' start, in millisecond format.
 */
extern u64 fsl_init_time;

/*!
 *  @brief project root directory.
 *
 *  @remark called from @ref fsl_engine_init() to change current working dirctory to the main process'.
 *  @remark declared and initialized internally.
 */
FSLAPI extern str *FSL_DIR_PROC_ROOT;

#endif /* FSL_SESSION_H */
