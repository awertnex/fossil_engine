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
 *  @file string_internal.h
 *
 *  @brief engine's internal string functions and definitions.
 */

#ifndef FSL_STRING_INTERNAL_H
#define FSL_STRING_INTERNAL_H

#include "../common/types.h"

/*!
 *  @internal
 *
 *  @brief write temporary formatted string.
 *
 *  @note the use of @ref stringf_internal more than once in a single expression is not advised.
 *
 *  @remark use temporary static buffers internally.
 *  @remark inspired by Raylib: `github.com/raysan5/raylib`: `raylib/src/rtext.c/TextFormat()`.
 *
 *  @return static formatted string.
 */
str *stringf_internal(const str *format, ...);

#endif /* FSL_STRING_INTERNAL_H */
