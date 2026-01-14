#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#include "h/types.h"
#include "h/diagnostics.h"
#include "h/limits.h"
#include "h/logger.h"

u32 log_level_max = LOGLEVEL_TRACE;
str *logger_buf = NULL;

static str log_tag[][16] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "TRACE",
};

str esc_code_color[][16] =
{
    "\033[31m",
    "\033[91m",
    "\033[95m",
    "\033[0m",
    "\033[0m",
    "\033[33m",
};

str esc_code_nocolor[] = "\033[0m";

static str in_message[IN_STRING_MAX] = {0};
static str out_message[OUT_STRING_MAX] = {0};

u32 logger_init(b8 release_build, int argc, char **argv)
{
    if (release_build)
        log_level_max = LOGLEVEL_INFO;

    if (argc && argv &&
            argc > 2 && !strncmp(argv[1], "LOGLEVEL", 8ul))
        {
            if (!strncmp(argv[2], "FATAL", 5ul))        log_level_max = LOGLEVEL_FATAL;
            else if (!strncmp(argv[2], "ERROR", 5ul))   log_level_max = LOGLEVEL_ERROR;
            else if (!strncmp(argv[2], "WARN", 4ul))    log_level_max = LOGLEVEL_WARNING;
            else if (!strncmp(argv[2], "INFO", 4ul))    log_level_max = LOGLEVEL_INFO;
            else if (!strncmp(argv[2], "DEBUG", 5ul))   log_level_max = LOGLEVEL_DEBUG;
            else if (!strncmp(argv[2], "TRACE", 5ul))   log_level_max = LOGLEVEL_TRACE;
        }

    if (mem_map((void*)&logger_buf, LOGGER_LINES_MAX * STRING_MAX,
                "logger_init().logger_buf") != ERR_SUCCESS)
    {
        LOGFATAL(FALSE, ERR_LOGGER_INIT_FAIL,
                "%s\n", "Failed to Initialize Logger, Process Aborted");
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void logger_close(void)
{
    mem_unmap((void*)&logger_buf, LOGGER_LINES_MAX * STRING_MAX, "logger_close().logger_buf");
}

void _log_output(b8 verbose, const str *file, u64 line,
        u8 level, u32 error_code, const str* format, ...)
{
    __builtin_va_list args;

    if (level > log_level_max) return;

    va_start(args, format);
    vsnprintf(in_message, IN_STRING_MAX, format, args);
    va_end(args);

    if (level <= LOGLEVEL_WARNING)
    {
        if (verbose)
            snprintf(out_message, OUT_STRING_MAX,
                    "%s%s%s:%s%"PRIu64"%s:%s%s[%"PRIu32"]: %s%s",
                    esc_code_color[level], file, esc_code_nocolor,
                    esc_code_color[level], line, esc_code_nocolor,
                    esc_code_color[level],
                    log_tag[level], error_code, in_message, esc_code_nocolor);
        else snprintf(out_message, OUT_STRING_MAX,
                    "%s%s[%"PRIu32"]: %s%s",
                    esc_code_color[level],
                    log_tag[level], error_code, in_message, esc_code_nocolor);
    }
    else
    {
        if (verbose)
            snprintf(out_message, OUT_STRING_MAX,
                    "%s%s%s:%s%"PRIu64"%s:%s%s: %s%s",
                    esc_code_color[level], file, esc_code_nocolor,
                    esc_code_color[level], line, esc_code_nocolor,
                    esc_code_color[level],
                    log_tag[level], in_message, esc_code_nocolor);
        else snprintf(out_message, OUT_STRING_MAX,
                    "%s%s: %s%s",
                    esc_code_color[level],
                    log_tag[level], in_message, esc_code_nocolor);
    }

    fprintf(stderr, "%s", out_message);
}
