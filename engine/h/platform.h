#ifndef ENGINE_PLATFORM_H
#define ENGINE_PLATFORM_H

#include "types.h"

#if defined(__linux__) || defined(__linux)
    #define PLATFORM_LINUX 1
    #define PLATFORM "linux"
    #define _PLATFORM "linux"
    #define ENGINE_NAME_LIB "libfossil.so"
    #define EXE ""
    #define RUNTIME_PATH "$ORIGIN"

    #define SLASH_NATIVE '/'
    #define SLASH_NON_NATIVE '\\'
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #define PLATFORM_WIN 1
    #define PLATFORM "win"
    #define _PLATFORM "windows"
    #define ENGINE_NAME_LIB "fossil.dll"
    #define EXE ".exe"
    #define RUNTIME_PATH "%CD%"

    #define SLASH_NATIVE '\\'
    #define SLASH_NON_NATIVE '/'
#endif /* PLATFORM */

u32 make_dir(const str *path);

/*! @brief change current working directory.
 */
int change_dir(const str *path);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief get real path.
 * 
 *  @param path = relative path,
 *  @param path_real = result/canonical path, ending with slash.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _get_path_absolute(const str *path, str *path_real);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief get current path of binary/executable, assign allocated path string to 'path'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _get_path_bin_root(str *path);

/*! @brief fork child process and execute command, based on 'execvp()'.
 * 
 *  @param cmd = command and args.
 *  @param cmd_name = command name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 exec(Buf *cmd, str *cmd_name);

#endif /* ENGINE_PLATFORM_H */
