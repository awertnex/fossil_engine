#ifndef ENGINE_BUILD_H
#define ENGINE_BUILD_H

#include "../../buildtool/build.h"

#ifdef OMIT_LFOSSIL
#   define LFOSSIL ""
#else
#   define LFOSSIL "-lfossil"
#endif /* FOSSIL_ENGINE */

#if PLATFORM_LINUX
    static const str str_engine_libs[][CMD_SIZE] =
    {
        "-Lengine/lib/"PLATFORM,
        "-lm",
        "-lglfw",
        "", /* empty slots for alignment across different platforms */
        "",
        "",
        LFOSSIL,
    };
#elif PLATFORM_WIN
    static const str str_engine_libs[][CMD_SIZE] =
    {
        "-Lengine/lib/"PLATFORM,
        "-lgdi32",
        "-lwinmm",
        "-mwindows",
        "-lm",
        "-lglfw3",
        LFOSSIL,
    };
#endif /* PLATFORM */

/* ---- section: signatures ------------------------------------------------- */

/*! @brief push array of arguments to the build command.
 *
 *  @param cmd cmd to push to, if `NULL`, @ref _cmd is used.
 */
static void engine_link_libs(_buf *cmd);

/* ---- section: implementation --------------------------------------------- */

void engine_link_libs(_buf *cmd)
{
    u32 i = 0;
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    for (; i < arr_len(str_engine_libs); ++i)
        cmd_push(_cmdp, str_engine_libs[i]);
}

#endif /* ENGINE_BUILD_H */
