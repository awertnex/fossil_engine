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

#include "../../../common/api.h" /* for _GNU_SOURCE */
#include "../../../common/limits.h"

#include "big_num_separator.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

str *fsl_big_num_separator_i64(i64 n)
{
    str src[FSL_ID_CAP] = {0};
    static char dst[FSL_ID_CAP] = {0};
    str *result = dst;
    i64 i = 0;
    i64 j = 0;
    i64 len = 0;
    i64 commas = 0;

    memset(dst, 0, 256);
    len = snprintf(src, 256, "%ld", n);

    while (n)
    {
        n /= 10;
        ++commas;
    }
    commas /= 3;

    for (i = 1, j = 1; i <= len && j <= len + commas; ++i, ++j)
    {
        dst[(len + commas) - j] = src[len - i];
        if (i % 3 == 0)
        {
            ++j;
            if (i < len)
                dst[(len + commas) - j] = ',';
            else
                ++result;
        }
    }

    return result;
}
