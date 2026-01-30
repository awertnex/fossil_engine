/*  MIT License
 *
 *  Copyright (c) 2026 Lily Awertnex
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef BUILDTOOL_H
#define BUILDTOOL_H

/* ---- section: changelog -------------------------------------------------- */

/*  v1.8.1-beta (2026 01 30):
 *      - (2026 01 30): Patch dumb error at `copy_file()`: passing
 *                      `strlen(in_file)` instead of `len`, which made things
 *                      explode
 */

/*  v1.8.0-beta (2026 01 30):
 *      - (2026 01 30): Make functions `cmd_show()` and `cmd_raw()` print only
 *                      once per build
 *      - (2026 01 30): Add `show` and `raw` to function `cmd_fail()` to show
 *                      build command even if failure
 *      - (2026 01 30): Move functions `make_dir()` and `change_dir()`
 *                      implementations to file `platform.h`
 *      - (2026 01 30): Add `bt_mkdir()`, `bt_chdir()`, `bt_stat()` and
 *                      `bt_chmod()` for platform abstraction
 *      - (2026 01 30): Treat symlinks as files in function `is_file_exists()`
 *      - (2026 01 30): Use proper error-handling for function `readlink()` in
                        function `get_path_bin_root()`
 *      - (2026 01 30): Truncate string buffer in function `get_path_bin_root()`
 *      - (2026 01 30): Preserve symlinks when using `copy_file()` and
 *                      `copy_dir()`
 *      - (2026 01 29): Add function `get_file_type()`
 *      - (2026 01 29): Add file `TASKS.md`
 */

/*  v1.7.0-beta (2026 Jan 29):
 *      - Change `COMPILER` 'gcc' -> 'cc'
 *      - Add guards around copying `tv_nsec` in `copy_dir()` and `copy_file()`
 *      - Improve windows support, just a little bit, it's still shit
 *      - Initialize all uninitialized values in file 'internal/platform.h'
 */

/*  v1.6.2 (2026 Jan 28):
 *      - `copy_dir()` bug fix and new features:
 *          - Fix parameter `contents_only` not working
 *          - Copy directory permissions
 *          - Copy directory access and modification times
 *      - Add error code `ERR_FILE_STAT_FAIL`
 *      - New `copy_file()` features:
 *          - Copy file permissions
 *          - Copy file access and modification times
 */

/*  v1.6.1 (2026 Jan 24):
 *      - Fix segfault when copying file into destination that's a directory without
 *        specifying the destination file name
 *      - Add function 'get_base_name()' to get base name of a path
 */

/*  v1.6 (2026 Jan 24):
 *      - Remove auto-allocation for '_cmd' in function 'build_init()' so freeing
 *        it isn't mandatory unless 'cmd_push()' is used
 *
 *      - Make function 'cmd_free()' free any cmd, and if NULL, free default cmd
 *        if allocated
 *
 *      - Add example 3: release vs debug builds
 *      - Move license above ifdef guards
 */

/*  v1.5 (2026 Jan 23):
 *      - Make function 'cmd_push()' handle 'NULL' strings safely
 */

/*  v1.4 (2026 Jan 23):
 *      - Remove function 'align_up_u64()', not needed
 *      - Add function 'cmd_init()' that gets called inside 'cmd_push()':
 *          Checks if cmd is initialized, initializes if not
 *      - Fix README.md
 */

/*  v1.3 (2026 Jan 23):
 *      - Change C standard of build tool 'C99' -> 'C89'
 */

/*  v1.2 (2026 Jan 23):
 *      - Add README.md
 *      - Move buildtool files into a bundle directory
 *      - Finalize template file 'build.c'
 */

/*  v1.1 (2026 Jan 23):
 *      - Add intialization for 'cmd' in function 'cmd_push()' if not already intialized
 *      - Add examples:
 *          example1
 *          example2
 *      - Fix function 'mem_alloc_buf()' not checking if 'buf' is already allocated
 */

/*  v1.0 (2026 Jan 23):
 *      - Initial Commit
 */

/* ---- section: examples --------------------------------------------------- */

/*  example 1: a single command.
 *
 *  - build.c: -----------------------------------------------------------------
 *      #include "buildtool/buildtool.h"
 *
 *      int main(int argc, char **argv)
 *      {
 *          if (build_init(argc, argv, "build.c", "build") != 0)
 *              cmd_fail(NULL); // free resources and return error code
 *
 *          cmd_exec(4, // number of arguments, excluding this number
 *                  "gcc",
 *                  "examples/example1.c",
 *                  "-o",
 *                  "example1");
 *
 *          cmd_free(NULL); // (NULL to free internal resources)
 *          return 0;
 *      }
 *
 *  - shell: -------------------------------------------------------------------
 *      gcc build.c -o build
 *      ./build
 *
 *  - or on windows, via mingw:
 *      gcc.exe build.c -o build.exe
 *      ./build.exe
 *
 *  - output: ------------------------------------------------------------------
 *      > Hello Example 1
 */

/*  example 2: load command now, execute later.
 *
 *  - build.c: -----------------------------------------------------------------
 *      #include "buildtool/buildtool.h"
 *
 *      int main(int argc, char **argv)
 *      {
 *          if (build_init(argc, argv, "build.c", "build") != 0)
 *              cmd_fail(NULL); // free resources and return error code
 *
 *          cmd_push(NULL, // NULL to use internal cmd `_cmd`
 *                  "gcc");
 *          cmd_push(NULL, "examples/example2_main.c");
 *          cmd_push(NULL, "examples/example2_util.c");
 *          cmd_push(NULL, "-o");
 *          cmd_push(NULL, "example2");
 *          cmd_ready(NULL); // finalize command (important for configuring based on platform).
 *
 *          if (exec(&_cmd, "example2_build()._cmd") != 0)
 *              cmd_fail(NULL);
 *
 *          cmd_free(NULL); // (NULL to free internal resources)
 *          return 0;
 *      }
 *
 *  - shell: -------------------------------------------------------------------
 *      gcc build.c -o build
 *      ./build
 *
 *  - or on windows, via mingw:
 *      gcc.exe build.c -o build.exe
 *      ./build.exe
 *
 *  - output: ------------------------------------------------------------------
 *      > Hello Example 2
 */

/*  example 3: command line arguments (release vs debug builds).
 *
 *  - build.c: -----------------------------------------------------------------
 *      #include "buildtool/buildtool.h"
 *
 *      _buf cmd = {0}; // the command to execute
 *
 *      int main(int argc, char **argv)
 *      {
 *          cmd_push(&cmd, COMPILER);
 *
 *          if (find_token("release", argc, argv)) // search 'argv' for the argument
 *              cmd_push(&cmd, "-D_EXAMPLE3_RELEASE_BUILD");
 *
 *          cmd_push(&cmd, "examples/example3.c");
 *          cmd_push(&cmd, "-o");
 *          cmd_push(&cmd, "example3");
 *          cmd_ready(&cmd);
 *
 *          exec(&cmd, "main().cmd");
 *          cmd_free(&cmd);
 *          return 0;
 *      }
 *
 *  - shell: -------------------------------------------------------------------
 *      gcc build.c -o build
 *      ./build
 *
 *      and:
 *
 *      ./build release
 *
 *  - or on windows, via mingw:
 *      gcc.exe build.c -o build.exe
 *      ./build.exe
 *
 *      and:
 *
 *      ./build.exe release
 *
 *  - output (debug): ----------------------------------------------------------
 *      > Example 3:
 *      >     Debug Build
 *      >     Verbose
 *      >     Very verbose
 *      >     Many logs
 *
 *  -  output (release):
 *      > Example 3: Release Build
 */

#include "internal/common.h"
#include "internal/platform.h"

/* ---- section: definitions ------------------------------------------------ */

#define BUILDTOOL_VERSION_STABLE    ""
#define BUILDTOOL_VERSION_BETA      "-beta"
#define BUILDTOOL_VERSION_ALPHA     "-alpha"
#define BUILDTOOL_VERSION_DEV       "-dev"

#define BUILDTOOL_VERSION_MAJOR 1
#define BUILDTOOL_VERSION_MINOR 8
#define BUILDTOOL_VERSION_PATCH 1
#define BUILDTOOL_VERSION_BUILD BUILDTOOL_VERSION_BETA

#define COMPILER "cc"EXE
#define CMD_MEMB 64
#define CMD_SIZE 256
#define ARG_MEMB 64
#define ARG_SIZE 256

enum build_flag
{
    FLAG_CMD_SHOW =     0x0001,
    FLAG_CMD_RAW =      0x0002,
    FLAG_BUILD_SELF =   0x0004,
}; /* build_flag */

/* ---- section: declarations ----------------------------------------------- */

/*! @brief project root directory.
 *  
 *  @remark called from @ref build_init() to change current working dirctory.
 */
static str *DIR_BUILDTOOL_BIN_ROOT = NULL;

u32 log_level_max = LOGLEVEL_INFO;
u32 build_err = ERR_SUCCESS;
static u32 flag = 0;
static str str_build_src[CMD_SIZE] = {0};
static str str_build_bin[CMD_SIZE] = {0};
static str str_build_bin_new[CMD_SIZE] = {0};
static str str_build_bin_old[CMD_SIZE] = {0};
static _buf _cmd = {0};
static _buf args = {0};

/* ---- section: signatures ------------------------------------------------- */

/*! @brief initialize build.
 *
 *  - allocate resources for @ref _cmd and other internals.
 *  - parse commands in `argv`, with no particular order:
 *      -v, --version   show version and exit.
 *      help            show help and exit.
 *      show            show build command in list format.
 *      raw             show build command in raw format.
 *      self            build build tool.
 *  - check if source uses a c-standard other than c89 and re-build with `-std=c89` if true.
 *  - check if source at `build_bin_name` has changed and rebuild if true.
 *
 *  @param argc number of arguments in `argv` if `argv` provided.
 *  @param argv command line arguments, optional.
 *
 *  @oaram build_src_name name of the build source file that's using this header.
 *  @oaram build_bin_name name of the build binary file that's using this header,
 *  including extension if needed.
 *
 *  @remark must be called before anything in the build tool.
 *  @remark can force-terminate process.
 *  @remark return non-zero on failure and @ref build_err is set accordingly.
 */
static u32 build_init(int argc, char **argv, const str *build_src_name, const str *build_bin_name);

static u32 is_build_source_changed(void);

/*! @remark can force-terminate process.
 */
static void self_rebuild(char **argv);

/*! @brief allocate, load, execute and free a command as variadic arguments.
 *
 *  @param n number of arguments passed.
 *  @param ... strings to pass into build command.
 *
 *  @remark return non-zero on failure and @ref build_err is set accordingly.
 */
static u32 cmd_exec(u64 n, ...);

/*! @brief allocate resources for `cmd` if not initialized.
 *
 *  @param cmd cmd to initialize, if `NULL`, @ref _cmd is used.
 */
static void cmd_init(_buf *cmd);

/*! @brief push arguments to the build command.
 *
 *  @param cmd cmd to push to, if `NULL`, @ref _cmd is used.
 *
 *  @remark `cmd` will be initialized if it isn't.
 */
static void cmd_push(_buf *cmd, const str *string);

/*! @brief finalize build command for execution.
 *
 *  @param cmd cmd to finalize, if `NULL`, @ref _cmd is used.
 *
 *  @remark must be called after loading `cmd` with all arguments and before @ref exec().
 */
static void cmd_ready(_buf *cmd);

static void cmd_free(_buf *cmd);

/*! @remark can force-terminate process.
 *
 *  @param cmd cmd to show, if `NULL`, @ref _cmd is used.
 */
static void cmd_fail(_buf *cmd);

/*! @brief show build command in list format.
 *
 *  @param cmd cmd to show, if `NULL`, @ref _cmd is used.
 */
static void cmd_show(_buf *cmd);

/*! @brief show build command in raw format.
 *
 *  @param cmd cmd to show, if `NULL`, @ref _cmd is used.
 */
static void cmd_raw(_buf *cmd);

static void help(void);
static void print_version(void);

/* ---- section: implementation --------------------------------------------- */

u32 build_init(int argc, char **argv, const str *build_src_name, const str *build_bin_name)
{
    const u64 TOKEN_SELF = 0;
    const u64 TOKEN_SHOW = 1;
    const u64 TOKEN_RAW = 2;
    u64 tokens[3] = {0};

    if (find_token("help", argc, argv)) help();

    if (    
            find_token("-v", argc, argv) ||
            find_token("--version", argc, argv))
        print_version();

    if (!DIR_BUILDTOOL_BIN_ROOT)
    {
        DIR_BUILDTOOL_BIN_ROOT = get_path_bin_root();
        if (!DIR_BUILDTOOL_BIN_ROOT)
            return build_err;
        change_dir(DIR_BUILDTOOL_BIN_ROOT);
    }

    cmd_push(&args, argv[0]);

    tokens[TOKEN_SELF] = find_token("self", argc, argv);
    tokens[TOKEN_SHOW] = find_token("show", argc, argv);
    tokens[TOKEN_RAW] = find_token("raw", argc, argv);

    if (tokens[TOKEN_SELF])
        flag |= FLAG_BUILD_SELF;
    if (tokens[TOKEN_SHOW]) flag |= FLAG_CMD_SHOW;
    if (tokens[TOKEN_RAW]) flag |= FLAG_CMD_RAW;

    snprintf(str_build_src, CMD_SIZE, "%s", build_src_name);
    posix_slash(str_build_src);

    snprintf(str_build_bin, CMD_SIZE, "%s", build_bin_name);
    snprintf(str_build_bin_new, CMD_SIZE, "%s_new", build_bin_name);
    snprintf(str_build_bin_old, CMD_SIZE, "%s_old", build_bin_name);

    if (STD != 89)
    {
        LOGINFO(FALSE,
                "%s\n", "Rebuilding Self With -std=c89..");
        self_rebuild((char**)args.i);
    }

    if (flag & FLAG_BUILD_SELF || is_build_source_changed() == ERR_SUCCESS)
    {
        LOGINFO(FALSE,
                "%s\n", "Rebuilding Self..");
        self_rebuild((char**)args.i);
    }

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 is_build_source_changed(void)
{
    unsigned long mtime_src = 0;
    unsigned long mtime_bin = 0;

    struct stat stats;

    if (stat(str_build_src, &stats) == 0)
        mtime_src = stats.st_mtime;
    else
    {
        LOGERROR(ERR_FILE_NOT_FOUND, FALSE,
                "%s\n", "Build Source File Not Found");
        return build_err;
    }

    if (stat(str_build_bin, &stats) == 0)
        mtime_bin = stats.st_mtime;
    else
    {
        LOGERROR(ERR_FILE_NOT_FOUND, FALSE,
                "%s\n", "File 'build"EXE"' Not Found");
        return build_err;
    }

    if (mtime_src && mtime_bin && mtime_src > mtime_bin)
        return ERR_SUCCESS;

    build_err = ERR_SOURCE_NOT_CHANGE;
    return build_err;
}

void self_rebuild(char **argv)
{
    flag &= ~FLAG_BUILD_SELF;

    cmd_push(&_cmd, COMPILER);
    cmd_push(&_cmd, "-std=c89");
    cmd_push(&_cmd, stringf("-ffile-prefix-map=%s=", DIR_BUILDTOOL_BIN_ROOT));
    cmd_push(&_cmd, "-Wall");
    cmd_push(&_cmd, "-Wextra");
    cmd_push(&_cmd, "-Wformat-truncation=0");
    cmd_push(&_cmd, str_build_src);
    cmd_push(&_cmd, "-o");
    cmd_push(&_cmd, str_build_bin_new);
    cmd_ready(&_cmd);

    if (exec(&_cmd, "self_rebuild()") == ERR_SUCCESS)
    {
        LOGINFO(FALSE,
                "%s\n", "Self Rebuild Success");
        rename(str_build_bin, str_build_bin_old);
        rename(str_build_bin_new, str_build_bin);
        remove(str_build_bin_old);

        execvp(argv[0], (str *const *)argv);
        LOGFATAL(ERR_EXECVP_FAIL, FALSE,
                "%s\n", "'build"EXE"' Failed, Process Aborted");
        cmd_fail(&_cmd);
    }

    LOGFATAL(build_err, FALSE,
            "%s\n", "Self-Rebuild Failed, Process Aborted");
    cmd_fail(&_cmd);
}

u32 cmd_exec(u64 n, ...)
{
    _buf cmd = {0};
    __builtin_va_list va;
    u64 i = 0;
    str temp[CMD_SIZE] = {0};

    cmd_init(&cmd);

    va_start(va, n);
    for (i = 0; i < n; ++i)
    {
        vsnprintf(temp, CMD_SIZE, "%s", va);
        cmd_push(&cmd, temp);
    }
    va_end(va);

    cmd_ready(&cmd);

    if (exec(&cmd, "cmd_exec().cmd") != ERR_SUCCESS)
        goto cleanup;

    mem_free_buf(&cmd, "cmd_exec().cmd");

    build_err = ERR_SUCCESS;
    return build_err;

cleanup:

    mem_free_buf(&cmd, "cmd_exec().cmd");
    cmd_fail(&cmd);
    return build_err;
}

void cmd_init(_buf *cmd)
{
    if (!cmd)
    {
        LOGFATAL(ERR_POINTER_NULL, TRUE,
                "%s\n", "Failed to Initialize 'cmd', Pointer NULL, Process Aborted\n");
        cmd_fail(cmd);
    }

    if (cmd->loaded)
        return;

    if (mem_alloc_buf(cmd, CMD_MEMB, CMD_SIZE, "cmd_init().cmd") != ERR_SUCCESS)
        cmd_fail(cmd);
}

void cmd_push(_buf *cmd, const str *string)
{
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    cmd_init(_cmdp);

    if (!string || !string[0])
        return;

    if (!_cmdp->loaded && mem_alloc_buf(_cmdp, CMD_MEMB, CMD_SIZE, "cmd_push()._cmdp") != ERR_SUCCESS)
        cmd_fail(_cmdp);

    if (_cmdp->cursor >= _cmdp->memb)
    {
        LOGERROR(ERR_BUFFER_FULL, FALSE,
                "%s\n", "cmd Full");
        return;
    }

    if (strlen(string) >= _cmdp->size)
    {
        LOGERROR(ERR_STRING_TOO_LONG, FALSE,
                "Failed to Push String '%s' to cmd.i[%"PRIu64"], String Too Long\n", string, _cmdp->cursor);
        return;
    }

    LOGTRACE(FALSE,
            "Pushing String '%s' to cmd.i[%"PRIu64"]..\n", string, _cmdp->cursor);
    strncpy(_cmdp->i[_cmdp->cursor++], string, CMD_SIZE);
}

void cmd_ready(_buf *cmd)
{
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

#if PLATFORM_LINUX
    _cmdp->i[_cmdp->cursor] = NULL;
#endif

    if (flag & FLAG_CMD_SHOW) cmd_show(_cmdp);
    if (flag & FLAG_CMD_RAW) cmd_raw(_cmdp);
}

void cmd_free(_buf *cmd)
{
    if (!cmd)
    {
        mem_free_buf(&_cmd, "cmd_free()._cmd");
        mem_free_buf(&args, "cmd_free().args");
        mem_free((void*)&DIR_BUILDTOOL_BIN_ROOT, CMD_SIZE, "cmd_free().DIR_BUILDTOOL_BIN_ROOT");
        _cmd.cursor = 0;
        args.cursor = 0;
    }
    else
    {
        mem_free_buf(cmd, "cmd_free().cmd");
        cmd->cursor = 0;
    }
}

void cmd_fail(_buf *cmd)
{
    _buf *_cmdp = cmd;
    if (!_cmdp)
        _cmdp = &_cmd;

    if (_cmdp)
    {
        if (flag & FLAG_CMD_SHOW) cmd_show(_cmdp);
        if (flag & FLAG_CMD_RAW) cmd_raw(_cmdp);
    }

    cmd_free(_cmdp);
    _exit(build_err);
}

void cmd_show(_buf *cmd)
{
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    flag &= ~FLAG_CMD_SHOW;

    printf("\nCMD:\n");
    u32 i = 0;
    for (; i < CMD_MEMB; ++i)
    {
        if (!_cmdp->i[i]) break;
        printf("    %.3d: %s\n", i, (str*)_cmdp->i[i]);
    }

    if (!(flag & FLAG_CMD_RAW))
        putchar('\n');
}

void cmd_raw(_buf *cmd)
{
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    flag &= ~FLAG_CMD_RAW;

    printf("\nCMD RAW:\n");
    u32 i = 0;
    for (; i < CMD_MEMB; ++i)
    {
        if (!_cmdp->i[i]) break;
        printf("%s ", (str*)_cmdp->i[i]);
    }

    printf("%s", "\n\n");
}

void help(void)
{
    printf("%s",
            "Usage: ./build [options]...\n"
            "Options:\n"
            "    help       print this help\n"
            "    show       show build command in list format\n"
            "    raw        show build command in raw format\n"
            "    self       build build source\n");
    _exit(ERR_SUCCESS);
}

void print_version(void)
{
    printf("buildtool - v%u.%u.%u%s\n",
            BUILDTOOL_VERSION_MAJOR,
            BUILDTOOL_VERSION_MINOR,
            BUILDTOOL_VERSION_PATCH,
            BUILDTOOL_VERSION_BUILD);
    _exit(ERR_SUCCESS);
}

#endif /* BUILDTOOL_H */
