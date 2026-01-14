#ifndef ENGINE_BUILD_H
#define ENGINE_BUILD_H

#include "types.h"
#include "diagnostics.h"
#include "platform.h"

#define COMPILER        "gcc"EXE
#define CMD_MEMB        64
#define CMD_SIZE        512

enum BuildFlag
{
    FLAG_CMD_SHOW = 0x0001,
    FLAG_CMD_RAW =  0x0002,
}; /* BuildFlag */

extern str *str_build_root;
extern Buf cmd;

/*! @brief initialize build.
 *
 *  1. allocate resources for 'cmd' and other internals.
 *  2. parse commands in 'argv', with no particular order:
 *      help:       show help and exit.
 *      show:       show the build command in list format.
 *      raw:        show the build command in raw format.
 *      LOGFATAL:   only output fatal logs (least verbose).
 *      LOGERROR:   only output error logs.
 *      LOGWARN:    only output warning logs.
 *      LOGINFO:    only output info logs.
 *      LOGDEBUG:   only output debug logs.
 *      LOGTRACE:   only output trace logs (most verbose, default).
 *  3. check if source uses a c-standard other than c99 and re-build with c99 if true.
 *  4. check if source at 'build_bin_name' has changed and rebuild if true.
 *
 *  @oaram build_src_name = name of the source file that's using this header.
 *  @oaram build_bin_name = name of the binary file that's using this header
 *  including extension if needed.
 *
 *  @remark must be called before anything in the engine.
 *  @remark can force-terminate process.
 *  @remark 'engine_err' is set accordingly on failure.
 */
void build_init(int argc, char **argv,
        const str *build_src_name, const str *build_bin_name);

/*! @brief build the engine itself into a dynamic/shared library.
 *
 *  @param out_dir = destination directory of the compiled library.
 *
 *  @remark the engine source directory 'engine' must be next to the build tool.
 *  @remark can force-terminate process.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 engine_build(const str *out_dir);

/*! @brief link engine's dependencies with the including software.
 */
void engine_link_libs(void);

b8 extension_evaluate(const str *file_name);
void extension_strip(const str *file_name, str *dest);

/*! @brief push arguments to the build command.
 */
void cmd_push(const str *string);

/*! @brief finalize build command for execution.
 *
 *  @remark must be called after loading 'cmd' with all arguments and before 'exec()'.
 */
void cmd_ready(void);

void cmd_free(void);

/*! @remark can force-terminate process.
 */
void cmd_fail(void);

#endif /* ENGINE_BUILD_H */
