/*!
 *  @file big_num_separator.c
 *  @author Lily Awertnex
 *
 *  big number separator - v4 - Jul 2026
 *
 *  separating numbers with four or more decimal places with commas.
 */

#include "deps/fossil/common/api.h" /* for _GNU_SOURCE */
#include "deps/fossil/common/limits.h"

#include "big_num_separator.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

str *big_num_separator_i64(i64 n)
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
