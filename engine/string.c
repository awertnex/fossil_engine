#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/string.h"

void swap_strings(str *s1, str *s2)
{
    u16 len = (strlen(s1) > strlen(s2)) ? strlen(s1) : strlen(s2);
    u16 i = 0;
    for (; i <= len; ++i)
        swap_bits(&s1[i], &s2[i]);
}

str *swap_string_char(str *string, char c1, char c2)
{
    u64 len = strlen(string), i;
    if (!len) return string;

    for (i = 0; i < len; ++i)
    {
        if (string[i] == c1)
            string[i] = c2;
    }

    return string;
}

str *stringf(const str* format, ...)
{
    static str str_buf[STRINGF_BUFFERS_MAX][OUT_STRING_MAX] = {0};
    static u64 index = 0, required_bytes;
    str *string = str_buf[index];
    __builtin_va_list args;
    char *trunc_buf = NULL;

    __builtin_va_start(args, format);
    required_bytes = vsnprintf(string, OUT_STRING_MAX, format, args);
    __builtin_va_end(args);

    if (required_bytes >= OUT_STRING_MAX - 1)
    {
        trunc_buf = str_buf[index] + OUT_STRING_MAX - 4;
        snprintf(trunc_buf, 4, "...");
    }

    index = (index + 1) % STRINGF_BUFFERS_MAX;
    return string;
}

u64 find_token(str *arg, int argc, char **argv)
{
    u32 i = 0;
    for (; (int)i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

u32 convert_i32_to_str(str *dest, i32 size, i32 n)
{
    i32 i = 1, j = 0, len = 0, sign = n;

    if (size <= 0)
    {
        LOGERROR(FALSE, ERR_SIZE_TOO_SMALL,
                "%s\n", "Failed to Convert i32 to str, 'size' Too Small");
        return engine_err;
    }

    if (n == 0)
    {
        dest[0] = '0';
        dest[1] = '\0';
        engine_err = ERR_SUCCESS;
        return engine_err;
    }

    (n < 0) ? ++len : 0;
    n = abs(n);
    for (; i < n + 1 && len < size - 1; i *= 10)
        ++len;
  
    i = 0;
    while (n > 0 && i < size - 1)
    {
        dest[i++] = (n % 10) + '0';
      	n /= 10;
    }

    if (sign < 0)
        dest[i++] = '-';
    dest[i] = '\0';

    for (j = i - 1, i = 0; i < j; i++, j--)
        swap_bits(&dest[i], &dest[j]);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 convert_u64_to_str(str *dest, u64 size, u64 n)
{
    u64 i = 1, j = 0, len = 0;

    if (size == 0)
    {
        LOGERROR(FALSE, ERR_SIZE_TOO_SMALL,
                "%s\n", "Failed to Convert u64 to str, 'size' Too Small");
        return engine_err;
    }

    if (n == 0)
    {
        dest[0] = '0';
        dest[1] = '\0';
        engine_err = ERR_SUCCESS;
        return engine_err;
    }

    for (; i < n + 1 && len < size - 1; i *= 10)
        ++len;
  
    i = 0;
    while (n > 0 && i < size - 1)
    {
        dest[i++] = (n % 10) + '0';
      	n /= 10;
    }

    dest[i] = '\0';

    for (j = i - 1, i = 0; i < j; i++, j--)
        swap_bits(&dest[i], &dest[j]);

    engine_err = ERR_SUCCESS;
    return engine_err;
}
