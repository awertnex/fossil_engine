#ifndef BUILD_H
#define BUILD_H

#include "internal/common.h"
#include "internal/platform.h"

/* ---- section: definitions ------------------------------------------------ */

#define COMPILER "gcc"EXE
#define CMD_MEMB 64
#define CMD_SIZE 256
#define ARG_MEMB 64
#define ARG_SIZE 256

enum BuildFlag
{
    FLAG_CMD_SHOW =     0x0001,
    FLAG_CMD_RAW =      0x0002,
    FLAG_BUILD_SELF =   0x0004,
}; /* BuildFlag */

/* ---- section: declarations ----------------------------------------------- */

/*! @brief project root directory.
 *  
 *  @remark called from @ref build_init() to change current working dirctory.
 */
static str *DIR_PROC_ROOT = NULL;

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
 *      help:       show help and exit.
 *      show:       show build command in list format.
 *      raw:        show build command in raw format.
 *      self:       build build tool.
 *  - check if source uses a c-standard other than c99 and re-build with `-std=c99` if true.
 *  - check if source at `build_bin_name` has changed and rebuild if true.
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

static b8 extension_evaluate(const str *file_name);
static void extension_strip(const str *file_name, str *dst);

/*! @brief allocate, load, execute and free a command as variadic arguments.
 *
 *  @param n number of arguments passed.
 *  @param ... strings to pass to build command.
 *
 *  @remark return non-zero on failure and @ref build_err is set accordingly.
 */
static u32 cmd_exec(u64 n, ...);

/*! @brief push arguments to the build command.
 *
 *  @param cmd cmd to push to, if `NULL`, @ref _cmd is used.
 */
static void cmd_push(_buf *cmd, const str *string);

/*! @brief finalize build command for execution.
 *
 *  @param cmd cmd to finalize, if `NULL`, @ref _cmd is used.
 *
 *  @remark must be called after loading `cmd` with all arguments and before @ref exec().
 */
static void cmd_ready(_buf *cmd);

static void cmd_free(void);

/*! @remark can force-terminate process.
 */
static void cmd_fail(void);

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

/* ---- section: implementation --------------------------------------------- */

u32 build_init(int argc, char **argv, const str *build_src_name, const str *build_bin_name)
{
    const u64 TOKEN_SELF = 0;
    const u64 TOKEN_SHOW = 1;
    const u64 TOKEN_RAW = 2;
    u64 tokens[3] = {0};

    if (find_token("help", argc, argv)) help();

    if (!DIR_PROC_ROOT)
    {
        DIR_PROC_ROOT = get_path_bin_root();
        if (!DIR_PROC_ROOT)
            return build_err;
        change_dir(DIR_PROC_ROOT);
    }

    if (mem_alloc_buf(&args, ARG_MEMB, ARG_SIZE, "engine_build().args") != ERR_SUCCESS)
        goto cleanup;

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

    if (mem_alloc_buf(&_cmd, CMD_MEMB, CMD_SIZE, "build_init()._cmd") != ERR_SUCCESS)
        goto cleanup;

    if (STD != 199901)
    {
        LOGINFO(FALSE, "%s\n", "Rebuilding Self With -std=c99..");
        self_rebuild((char**)args.i);
    }

    if (flag & FLAG_BUILD_SELF || is_build_source_changed() == ERR_SUCCESS)
    {
        LOGINFO(FALSE, "%s\n", "Rebuilding Self..");
        self_rebuild((char**)args.i);
    }

    build_err = ERR_SUCCESS;
    return build_err;

cleanup:

    cmd_fail();
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
        LOGERROR(FALSE, ERR_FILE_NOT_FOUND, "%s\n", "Build Source File Not Found");
        return build_err;
    }

    if (stat(str_build_bin, &stats) == 0)
        mtime_bin = stats.st_mtime;
    else
    {
        LOGERROR(FALSE, ERR_FILE_NOT_FOUND, "%s\n", "File 'build"EXE"' Not Found");
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
    cmd_push(&_cmd, "-std=c99");
    cmd_push(&_cmd, stringf("-ffile-prefix-map=%s=", DIR_PROC_ROOT));
    cmd_push(&_cmd, "-Wall");
    cmd_push(&_cmd, "-Wextra");
    cmd_push(&_cmd, "-Wformat-truncation=0");
    cmd_push(&_cmd, str_build_src);
    cmd_push(&_cmd, "-o");
    cmd_push(&_cmd, str_build_bin_new);
    cmd_ready(&_cmd);

    if (exec(&_cmd, "self_rebuild()") == ERR_SUCCESS)
    {
        LOGINFO(FALSE, "%s\n", "Self Rebuild Success");
        rename(str_build_bin, str_build_bin_old);
        rename(str_build_bin_new, str_build_bin);
        remove(str_build_bin_old);

        execvp(argv[0], (str *const *)argv);
        LOGFATAL(FALSE, ERR_EXECVP_FAIL, "%s\n", "'build"EXE"' Failed, Process Aborted");
        cmd_fail();
    }

    LOGFATAL(FALSE, build_err, "%s\n", "Self-Rebuild Failed, Process Aborted");
    cmd_fail();
}

u32 cmd_exec(u64 n, ...)
{
    _buf cmd = {0};
    __builtin_va_list va;
    u64 i = 0;
    str temp[CMD_SIZE] = {0};

    if (mem_alloc_buf(&cmd, align_up_u64(n, CMD_MEMB), CMD_SIZE, "cmd_exec().cmd") != ERR_SUCCESS)
        return build_err;

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

    mem_free_buf(&cmd, "cmd_build().cmd");
    cmd_fail();
    return build_err;
}

void cmd_push(_buf *cmd, const str *string)
{
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    if (!string[0])
        return;

    if (_cmdp->cursor >= _cmdp->memb)
    {
        LOGERROR(FALSE, ERR_BUFFER_FULL, "%s\n", "cmd Full");
        return;
    }

    if (strlen(string) >= _cmdp->size)
    {
        LOGERROR(FALSE, ERR_STRING_TOO_LONG,
                "Failed to Push String '%s' to cmd.i[%"PRIu64"], String Too Long\n", string, _cmdp->cursor);
        return;
    }

    LOGTRACE(FALSE, "Pushing String '%s' to cmd.i[%"PRIu64"]..\n", string, _cmdp->cursor);
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

void cmd_free(void)
{
    mem_free((void*)&DIR_PROC_ROOT, CMD_SIZE, "cmd_free().DIR_PROC_ROOT");
    mem_free_buf(&_cmd, "cmd_free()._cmd");
    mem_free_buf(&args, "cmd_free().args");
    _cmd.cursor = 0;
    args.cursor = 0;
}

void cmd_fail(void)
{
    cmd_free();
    _exit(build_err);
}

void cmd_show(_buf *cmd)
{
    _buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

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
            "    self       build build tool\n");
    _exit(ERR_SUCCESS);
}

#endif /* BUILD_H */
