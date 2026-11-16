/**
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

/**
 *  @file big_num_separator.c
 *
 *  @author Lily Awertnex
 *
 *  @brief Big Num Separator plug-in implementation, for 'Fossil Engine'.
 *
 *  big number separator - v4 - Jul 2026: separate numbers with four or more
 *  decimal places with commas.
 *
 *  this is a small utility I made in 2021 and called "big_num_separator"
 *  because I had no better name, and I made it just for fun.
 */

#ifndef FSL_BIG_NUM_SEPARATOR_H
#define FSL_BIG_NUM_SEPARATOR_H

#include "../../../common/api.h"
#include "../../../common/types.h"

FSLAPI str *fsl_big_num_separator_i64(i64 n);

#endif /* FSL_BIG_NUM_SEPARATOR_H */
