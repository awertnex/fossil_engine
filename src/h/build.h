/*  Copyright 2026 Lily Awertnex
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

/*  build.h - build info to link the engine with external software
 */

#ifndef FSL_BUILD_H
#define FSL_BUILD_H

#include "../../deps/buildtool/buildtool.h"

#define FSL_C_STD "-std=c89"

#ifdef FSL_OMIT_LFOSSIL
#   define LFOSSIL ""
#else
#   define LFOSSIL "-lfossil"
#endif /* FSL_OMIT_LFOSSIL */

#if FSL_PLATFORM_WIN
    static const str fsl_str_libs[][CMD_SIZE] =
    {
        "-Llib/"PLATFORM,
        "-lgdi32",
        "-lwinmm",
        "-mwindows",
        "-lm",
        "-lglfw3",
        LFOSSIL,
    };
#else
    static const str fsl_str_libs[][CMD_SIZE] =
    {
        "-Llib/"PLATFORM,
        "-lm",
        "-lglfw",
        "", /* empty slots for alignment across different platforms */
        "",
        "",
        LFOSSIL,
    };
#endif /* FSL_PLATFORM */

/* ---- section: signatures ------------------------------------------------- */

/*! @brief link engine's dependencies with the including software for compile time.
 *
 *  @param cmd cmd to push engine's required libs to, if `NULL`, internal cmd is used.
 */
static void fsl_engine_link_libs(_buf *cmd);

/*! @brief link engine's dependencies with the including software for run time.
 *
 *  @param cmd cmd to push application's runtime path to, if `NULL`, internal cmd is used.
 */
static void fsl_engine_set_runtime_path(_buf *cmd);

/* ---- section: implementation --------------------------------------------- */

void fsl_engine_link_libs(_buf *cmd)
{
    u32 i = 0;
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    for (i = 0; i < arr_len(fsl_str_libs); ++i)
        cmd_push(_cmdp, fsl_str_libs[i]);
}

void fsl_engine_set_runtime_path(_buf *cmd)
{
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    cmd_push(_cmdp, "-Wl,-rpath="RUNTIME_PATH);
}

#endif /* FSL_BUILD_H */
