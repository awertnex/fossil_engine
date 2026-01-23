#ifndef FSL_BUILD_H
#define FSL_BUILD_H

#include "../../deps/buildtool/buildtool.h"

#ifdef FSL_OMIT_LFOSSIL
#   define LFOSSIL ""
#else
#   define LFOSSIL "-lfossil"
#endif /* FSL_OMIT_LFOSSIL */

#if FSL_PLATFORM_WIN
    static const str fsl_str_libs[][CMD_SIZE] =
    {
        "-Lengine/lib/"PLATFORM,
        "-lgdi32",
        "-lwinmm",
        "-mwindows",
        "-lm",
        "-lglfw3",
        LFOSSIL,
    };
#else
    static const str fsl_str_libs[][CMD_SIZE] =
    {
        "-Lengine/lib/"PLATFORM,
        "-lm",
        "-lglfw",
        "", /* empty slots for alignment across different platforms */
        "",
        "",
        LFOSSIL,
    };
#endif /* FSL_PLATFORM */

/* ---- section: signatures ------------------------------------------------- */

/*! @brief push array of arguments to the build command.
 *
 *  @param cmd cmd to push to, if `NULL`, @ref _cmd is used.
 */
static void fsl_link_libs(_buf *cmd);

/* ---- section: implementation --------------------------------------------- */

void fsl_link_libs(_buf *cmd)
{
    u32 i = 0;
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    for (i = 0; i < arr_len(fsl_str_libs); ++i)
        cmd_push(_cmdp, fsl_str_libs[i]);
}

#endif /* FSL_BUILD_H */
