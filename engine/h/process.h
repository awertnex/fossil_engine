#ifndef ENGINE_PROCESS_H
#define ENGINE_PROCESS_H

#include "common.h"
#include "types.h"

/*! @brief get calloc'd string of executable's path, slash and NULL terminated.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
FSLAPI str *get_path_bin_root(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief get current path of binary/executable, assign allocated path string to 'path'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _get_path_bin_root(str *path);

/*! @brief fork child process and execute command, based on 'execvp()'.
 * 
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @param cmd = command and args.
 *  @param cmd_name = command name (for logging).
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 exec(Buf *cmd, str *cmd_name);

#endif /* ENGINE_PROCESS_H */
