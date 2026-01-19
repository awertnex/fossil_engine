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
 *  - allocate resources for 'cmd' and other internals.
 *  - parse commands in 'argv', with no particular order:
 *      engine:     build engine, place into 'engine/lib/<PLATFORM>' and into deployment directory at 'dir_out'.
 *      self:       build the build tool at 'build_src_name' into 'build_bin_name' if build successful.
 *      noproject:  don't execute 'build_function()'.
 *      help:       show help and exit.
 *      show:       show the build command in list format.
 *      raw:        show the build command in raw format.
 *      LOGFATAL:   only output fatal logs (least verbose).
 *      LOGERROR:   only output <= error logs.
 *      LOGWARN:    only output <= warning logs.
 *      LOGINFO:    only output <= info logs (default).
 *      LOGDEBUG:   only output <= debug logs.
 *      LOGTRACE:   only output <= trace logs (most verbose).
 *  - check if source uses a c-standard other than c99 and re-build with c99 if true.
 *  - check if source at 'build_bin_name' has changed and rebuild if true.
 *
 *  @oaram build_src_name = name of the source file that's using this header.
 *  @oaram build_bin_name = name of the binary file that's using this header,
 *  including extension if needed.
 *
 *  @oaram dir_out = name of deployment directory.
 *  @oaram build_function = build function to execute, optional.
 *
 *  @remark must be called before anything in the build tool.
 *  @remark can force-terminate process.
 *  @remark 'engine_err' is set accordingly on failure.
 */
void engine_build(int argc, char **argv, const str *build_src_name, const str *build_bin_name,
        const str *dir_out, u32 (*build_function)());

/*! @brief build the engine itself into a dynamic/shared library.
 *
 *  @param out_dir = destination directory of the compiled library.
 *
 *  @remark the engine source directory 'engine' must be next to the build tool.
 *  @remark can force-terminate process.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _engine_build(const str *out_dir);

/*! @brief link engine's dependencies with the including software.
 *
 *  @remark if 'cmd' NULL, internal cmd is used.
 */
void engine_link_libs(Buf *cmd);

b8 extension_evaluate(const str *file_name);
void extension_strip(const str *file_name, str *dest);

/*! @brief push arguments to the build command.
 *
 *  @remark if 'cmd' NULL, internal cmd is used.
 */
void cmd_push(Buf *cmd, const str *string);

/*! @brief finalize build command for execution.
 *
 *  @remark must be called after loading 'cmd' with all arguments and before 'exec()'.
 *  @remark if 'cmd' NULL, internal cmd is used.
 */
void cmd_ready(Buf *cmd);

void cmd_free(void);

/*! @remark can force-terminate process.
 */
void cmd_fail(void);

#endif /* ENGINE_BUILD_H */
