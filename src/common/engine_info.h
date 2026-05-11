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
 *  @file engine_info.h
 *
 *  @brief engine info and API definitions.
 */

#ifndef FSL_ENGINE_INFO_H
#define FSL_ENGINE_INFO_H

#define FSL_ENGINE_AUTHOR           "Lily Awertnex"
#define FSL_ENGINE_NAME             "Fossil Engine"
#define FSL_ENGINE_C_STD            "c89"

#define FSL_ENGINE_VERSION_STABLE   ""
#define FSL_ENGINE_VERSION_BETA     "-beta"
#define FSL_ENGINE_VERSION_ALPHA    "-alpha"
#define FSL_ENGINE_VERSION_DEV      "-dev"

#define FSL_ENGINE_VERSION_MAJOR    0
#define FSL_ENGINE_VERSION_MINOR    8
#define FSL_ENGINE_VERSION_PATCH    0
#define FSL_ENGINE_VERSION_BUILD    FSL_ENGINE_VERSION_BETA

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW32__)
#   define FSL_PLATFORM_WIN         1
#   define FSL_PLATFORM             "win"
#   define FSL_FILE_NAME_LIB        "fossil.dll"
#   define FSL_FILE_NAME_PLATFORM   "platform_win.c"
#   define FSL_EXE                  ".exe"
#   define FSL_RUNTIME_PATH         "%CD%"
#   define FSL_SLASH_NATIVE         '\\'
#   define FSL_SLASH_NON_NATIVE     '/'
#   define fsl_mkdir(name)          _mkdir(name)
#   define fsl_chdir(name)          _chdir(name)
#   define fsl_stat(name, st)       _lstat(name, st)
#   define fsl_chmod(name, n)       _chmod(name, n)
#elif defined(__linux__) || defined(__linux)
#   ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#   endif /* _GNU_SOURCE */

#   define FSL_PLATFORM_LINUX       1
#   define FSL_PLATFORM             "linux"
#   define FSL_FILE_NAME_LIB        "libfossil.so"
#   define FSL_FILE_NAME_PLATFORM   "platform_linux.c"
#   define FSL_EXE                  ""
#   define FSL_RUNTIME_PATH         "$ORIGIN"
#   define FSL_SLASH_NATIVE         '/'
#   define FSL_SLASH_NON_NATIVE     '\\'
#   define fsl_mkdir(name)          mkdir(name, 0755)
#   define fsl_chdir(name)          chdir(name)
#   define fsl_stat(name, st)       lstat(name, st)
#   define fsl_chmod(name, n)       lchmod(name, n)
#endif /* FSL_PLATFORM */

#if FSL_PLATFORM_WIN
#   define FSLAPI __declspec(dllexport)
#else
#   define FSLAPI __attribute__((visibility("default")))
#endif /* FSL_PLATFORM */

#ifndef FSLAPI
#   define FSLAPI
#endif /* FSLAPI */

#endif /* FSL_ENGINE_INFO_H */
