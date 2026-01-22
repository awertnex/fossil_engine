#ifndef ENGINE_BUILD_H
#define ENGINE_BUILD_H

#include "../build/build.h"

#if PLATFORM_LINUX
    static str str_libs[][CMD_SIZE] =
    {
        "-lm",
        "-lglfw",
        "",
        "",
    };
#elif PLATFORM_WIN
    static str str_libs[][CMD_SIZE] =
    {
        "-lgdi32",
        "-lwinmm",
        "-lm",
        "-lglfw3",
    };
#endif /* PLATFORM */

/* ---- section: signatures ------------------------------------------------- */

/*! @brief link engine's dependencies with the including software.
 *
 *  @param cmd cmd to push engine's required libs to, if `NULL`, @ref _cmd is used.
 */
static void engine_link_libs(_buf *cmd);

/*! @brief link engine's dependencies with the engine binary.
 *
 *  @param cmd cmd to push engine's required libs to, if `NULL`, @ref _cmd is used.
 */
static void _engine_link_libs(_buf *cmd);

/* ---- section: implementation --------------------------------------------- */

void engine_link_libs(_buf *cmd)
{
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    _engine_link_libs(_cmdp);
    cmd_push(_cmdp, "-lfossil");
}

void _engine_link_libs(_buf *cmd)
{
    u32 i = 0;
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    cmd_push(_cmdp, "-Lengine/lib/"PLATFORM);
    for (;i < arr_len(str_libs); ++i)
        cmd_push(_cmdp, str_libs[i]);
}

#endif /* ENGINE_BUILD_H */
