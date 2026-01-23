#ifndef FSL_PROCESS_H
#define FSL_PROCESS_H

#include "common.h"
#include "types.h"

/*! @brief get calloc'd string of executable's path, slash (`/`) and null (`\0`) terminated.
 *
 *  @return `NULL` on failure and @ref fsl_err is set accordingly.
 */
FSLAPI str *fsl_get_path_bin_root(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @brief get current path of binary/executable, assign allocated path string to `path`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 _fsl_get_path_bin_root(str *path);

/*! @brief execute command in a separate child process (based on @ref execvp()).
 * 
 *  -- IMPLEMENTATION: platform_<PLATFORM>.c --;
 *
 *  @param cmd command and args to execute.
 *  @param cmd_name command name (for logging).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_exec(fsl_buf *cmd, str *cmd_name);

#endif /* FSL_PROCESS_H */
