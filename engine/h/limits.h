#ifndef ENGINE_LIMITS_H
#define ENGINE_LIMITS_H

#include "common.h"

#if PLATFORM_LINUX
#   include <linux/limits.h>
#elif PLATFORM_WIN
#   define NAME_MAX 255
#   include <limits.h>
#endif /* PLATFORM */

#define U8_MAX 255
#define I8_MAX 127
#define I8_MIN (-I8_MAX - 1)

#define U16_MAX 65535
#define I16_MAX 32767
#define I16_MIN (-I16_MAX - 1)

#define U32_MAX 4294967295
#define I32_MAX 2147483647
#define I32_MIN (-I32_MAX - 1)

#define U64_MAX 18446744073709551615
#define I64_MAX 9223372036854775807
#define I64_MIN (-I64_MAX - 1)

#define STRING_MAX              2048
#define STRING_TOKEN_MAX        128
#define STRINGF_BUFFERS_MAX     4
#define TIME_STRING_MAX         128
#define OUT_STRING_MAX          (STRING_MAX + 256)
#define LOGGER_STRING_MAX       OUT_STRING_MAX
#define LOGGER_HISTORY_MAX      2048
#define GLYPH_MAX               256
#define INCLUDE_RECURSION_MAX   512
#define SCREENSHOT_RATE_MAX     128

#endif /* ENGINE_LIMITS_H */
