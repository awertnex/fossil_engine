#ifndef ENGINE_BUILD_H
#define ENGINE_BUILD_H

#include "common.h"
#include "types.h"

#define COMPILER        "gcc"EXE
#define CMD_MEMB        64
#define CMD_SIZE        256
#define ARG_MEMB        8
#define ARG_SIZE        256

enum BuildFlag
{
    FLAG_CMD_SHOW =     0x0001,
    FLAG_CMD_RAW =      0x0002,
    FLAG_BUILD_PROJECT = 0x0004,
    FLAG_BUILD_SELF =   0x0008,
    FLAG_BUILD_ENGINE = 0x0010,
    FLAG_BUILD_ALL =    0x001c,
}; /* BuildFlag */

/*! @brief initialize build.
 *
 *  - allocate resources for @ref cmd and other internals.
 *  - parse commands in `argv`, with no particular order:
 *      engine:     build engine, place into `engine/lib/<PLATFORM>` and into directory at `dir_out`.
 *      self:       build the build tool at `build_src_name` into `build_bin_name` if build successful.
 *      noproject:  don't execute `build_function()`.
 *      help:       show help and exit.
 *      show:       show the build command in list format.
 *      raw:        show the build command in raw format.
 *      @ref logger_init() commands:
 *          logfatal:   only output fatal logs (least verbose).
 *          logerror:   only output <= error logs.
 *          logwarn:    only output <= warning logs.
 *          loginfo:    only output <= info logs (default).
 *          logdebug:   only output <= debug logs.
 *          logtrace:   only output <= trace logs (most verbose).
 *  - check if source uses a c-standard other than c99 and re-build with `-std=c99` if true.
 *  - check if source at `build_bin_name` has changed and rebuild if true.
 *
 *  @oaram build_src_name name of the build source file that's using this header.
 *  @oaram build_bin_name name of the build binary file that's using this header,
 *  including extension if needed.
 *
 *  @oaram dir_out name of deployment directory.
 *  @oaram build_function build function to execute, optional.
 *
 *  @remark must be called before anything in the build tool.
 *  @remark can force-terminate process.
 *  @remark @ref engine_err is set accordingly on failure.
 */
void engine_build(int argc, char **argv, const str *build_src_name, const str *build_bin_name,
        const str *dir_out, u32 (*build_function)());

/*! @brief link engine's dependencies with the including software.
 *
 *  @param cmd cmd to push engine's required libs to, if `NULL`, internal cmd is used.
 */
void engine_link_libs(Buf *cmd);

b8 extension_evaluate(const str *file_name);
void extension_strip(const str *file_name, str *dst);

/*! @brief push arguments to the build command.
 *
 *  @param cmd cmd to push to, if `NULL`, internal cmd is used.
 */
void cmd_push(Buf *cmd, const str *string);

/*! @brief finalize build command for execution.
 *
 *  @param cmd cmd to finalize, if `NULL`, internal cmd is used.
 *
 *  @remark must be called after loading `cmd` with all arguments and before @ref exec().
 */
void cmd_ready(Buf *cmd);

void cmd_free(void);

/*! @remark can force-terminate process.
 */
void cmd_fail(void);

#endif /* ENGINE_BUILD_H */
