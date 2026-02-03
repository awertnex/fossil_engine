/*  @file process.h
 *
 *  @brief identify current process, execute commands, fork.
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
 *  limitations under the License.OFTWARE.
 */

#ifndef FSL_PROCESS_H
#define FSL_PROCESS_H

#include "common.h"
#include "types.h"

/*! @brief get current path of binary/executable, slash (`/`) and null (`\0`) terminated,
 *  assign allocated path string to `dst`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_get_path_bin_root(str **dst);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief get current path of binary/executable and assign to `dst`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 _fsl_get_path_bin_root(str *dst);

/*! @brief execute command in a separate child process (based on @ref execvp()).
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @param cmd command and args to execute.
 *  @param cmd_name command name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_exec(fsl_buf *cmd, str *cmd_name);

#endif /* FSL_PROCESS_H */
