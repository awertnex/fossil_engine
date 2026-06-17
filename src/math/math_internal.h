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
 *  @file math_internal.h
 *
 *  @brief initializing math stuff.
 */

#ifndef FSL_MATH_INTERNAL_H
#define FSL_MATH_INTERNAL_H

#include "../common/types.h"

/*!
 *  @internal
 *
 *  @brief initialize and allocate resources for @ref fsl_rand_tab.
 *
 *  allocate resources for @ref fsl_rand_tab and load its look-up from disk if found
 *  and build if not found.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 noise_init_internal(void);

/*!
 *  @internal
 */
void noise_free_internal(void);

#endif /* FSL_MATH_INTERNAL_H */
