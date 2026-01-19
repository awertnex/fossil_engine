#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "h/common.h"

#include "h/build.h"
#include "diagnostics.h"
#include "dir.c"
#include "logger.c"
#include "memory.c"
#include "string.c"
#include "time.c"

#if PLATFORM_LINUX
    #include "platform_linux.c"

    static str str_libs[][CMD_SIZE] =
    {
        "-lm",
        "-lglfw",
    };
#elif PLATFORM_WIN
    #include "platform_windows.c"

    static str str_libs[][CMD_SIZE] =
    {
        "-lgdi32",
        "-lwinmm",
        "-lm",
        "-lglfw3",
    };
#endif /* PLATFORM */

#ifdef __STDC_VERSION__
    #if (__STDC_VERSION__ == 199901)
        #define STD __STDC_VERSION__
    #else
        #define STD 0
    #endif
#else
    #define STD 0
#endif /* STD */

u64 init_time = 0;
str *DIR_PROC_ROOT = NULL;
u32 engine_err = ERR_SUCCESS;
static u32 flag = 0;
static str str_build_src[CMD_SIZE] = {0};
static str str_build_bin[CMD_SIZE] = {0};
static str str_build_bin_new[CMD_SIZE] = {0};
static str str_build_bin_old[CMD_SIZE] = {0};
static Buf _cmd = {0};
static Buf args = {0};

static u32 is_build_source_changed(void);

/*! @remark can force-terminate process.
 */
static void self_rebuild(char **argv);

/*! -- INTERNAL USE ONLY --;
 *
 *  @remark if 'cmd' NULL, internal cmd is used.
 */
static void _engine_link_libs(Buf *cmd);

/*! -- INTERNAL USE ONLY --;
 *
 *  @remark if 'cmd' NULL, internal cmd is used.
 */
static void cmd_show(Buf *cmd);

/*! -- INTERNAL USE ONLY --;
 *
 *  @remark if 'cmd' NULL, internal cmd is used.
 */
static void cmd_raw(Buf *cmd);

static void help(void);

void engine_build(int argc, char **argv, const str *build_src_name, const str *build_bin_name,
        const str *dir_out, u32 (*build_function)())
{
    int i = 0;
    u64 token_self = 0;
    u64 token_all = 0;

    if (logger_init(argc, argv, TRUE, NULL, FALSE) != ERR_SUCCESS)
        cmd_fail();

    if (mem_alloc_buf(&args, ARG_MEMB, ARG_SIZE, "engine_build().args") != ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < argc; ++i)
        cmd_push(&args, argv[i]);

    token_self = find_token("self", argc, (char**)args.i);
    token_all = find_token("all", argc, (char**)args.i);

    if (token_self)
    {
        flag |= FLAG_BUILD_SELF;
        flag &= ~FLAG_BUILD_PROJECT;
        snprintf(args.i[token_self], ARG_SIZE, "%s", "noproject");
    }

    if (token_all)
    {
        flag |= FLAG_BUILD_ALL;
        snprintf(args.i[token_all], ARG_SIZE, "%s", "engine");
        cmd_push(&args, "project");
    }

    if (find_token("help", argc, argv)) help();
    if (find_token("show", argc, argv)) flag |= FLAG_CMD_SHOW;
    if (find_token("raw", argc, argv)) flag |= FLAG_CMD_RAW;
    if (find_token("engine", argc, argv))
    {
        flag |= FLAG_BUILD_ENGINE;

        if (!token_all)
            flag &= ~FLAG_BUILD_PROJECT;
    }
    if (find_token("noproject", argc, argv))
        flag &= ~FLAG_BUILD_PROJECT;
    if (find_token("project", argc, argv))
        flag |= FLAG_BUILD_PROJECT;

    if (build_function)
        flag |= FLAG_BUILD_PROJECT;
    else
        LOGWARNING(FALSE, FALSE, ERR_BUILD_FUNCTION_NOT_FOUND, "%s\n", "Build Function Not Provided");

    make_dir(dir_out);

    if (engine_err != ERR_SUCCESS && engine_err != ERR_DIR_EXISTS)
        goto cleanup;

    snprintf(str_build_src, CMD_SIZE, "%s", build_src_name);
    normalize_slash(str_build_src);

    snprintf(str_build_bin, CMD_SIZE, "%s", build_bin_name);
    snprintf(str_build_bin_new, CMD_SIZE, "%s_new", build_bin_name);
    snprintf(str_build_bin_old, CMD_SIZE, "%s_old", build_bin_name);

    if (mem_alloc_buf(&_cmd, CMD_MEMB, CMD_SIZE, "build_init()._cmd") != ERR_SUCCESS)
        goto cleanup;

    if (STD != 199901)
    {
        LOGINFO(FALSE, FALSE, "%s\n", "Rebuilding Self With -std=c99..");
        self_rebuild((char**)args.i);
    }

    if (flag & FLAG_BUILD_SELF || is_build_source_changed() == ERR_SUCCESS)
    {
        LOGINFO(FALSE, FALSE, "%s\n", "Rebuilding Self..");
        self_rebuild((char**)args.i);
    }

    if (flag & FLAG_BUILD_ENGINE)
    {
        if (_engine_build(dir_out) != ERR_SUCCESS)
            goto cleanup;

        mem_free_buf(&_cmd, "engine_build()._cmd");
    }

    if (flag & FLAG_BUILD_PROJECT)
    {
        if (mem_alloc_buf(&_cmd, CMD_MEMB, CMD_SIZE, "engine_build()._cmd") != ERR_SUCCESS)
            goto cleanup;

        build_function();
        if (exec(&_cmd, "engine_build()") != ERR_SUCCESS)
            goto cleanup;
    }

    return;

cleanup:

    cmd_fail();
}

static u32 is_build_source_changed(void)
{
    unsigned long mtime_src = 0;
    unsigned long mtime_bin = 0;

    struct stat stats;

    if (stat(str_build_src, &stats) == 0)
        mtime_src = stats.st_mtime;
    else
    {
        LOGERROR(FALSE, FALSE, ERR_FILE_NOT_FOUND, "%s\n", "Build Source File Not Found");
        return engine_err;
    }

    if (stat(str_build_bin, &stats) == 0)
        mtime_bin = stats.st_mtime;
    else
    {
        LOGERROR(FALSE, FALSE, ERR_FILE_NOT_FOUND, "%s\n", "File 'build"EXE"' Not Found");
        return engine_err;
    }

    if (mtime_src && mtime_bin && mtime_src > mtime_bin)
        return ERR_SUCCESS;

    engine_err = ERR_SOURCE_NOT_CHANGE;
    return engine_err;
}

static void self_rebuild(char **argv)
{
    flag &= ~FLAG_BUILD_SELF;

    cmd_push(&_cmd, COMPILER);
    cmd_push(&_cmd, str_build_src);
#if PLATFORM_LINUX
    cmd_push(&_cmd, "-D_GNU_SOURCE");
#endif /* PLATFORM_LINUX */
    cmd_push(&_cmd, "-std=c99");
    cmd_push(&_cmd, "-fvisibility=hidden");
    cmd_push(&_cmd, stringf("-ffile-prefix-map=%s=", DIR_PROC_ROOT));
    cmd_push(&_cmd, "-Wall");
    cmd_push(&_cmd, "-Wextra");
    cmd_push(&_cmd, "-Wformat-truncation=0");
    cmd_push(&_cmd, "-o");
    cmd_push(&_cmd, str_build_bin_new);
    cmd_ready(&_cmd);

    if (flag & FLAG_CMD_SHOW) cmd_show(&_cmd);
    if (flag & FLAG_CMD_RAW) cmd_raw(&_cmd);

    if (exec(&_cmd, "self_rebuild()") == ERR_SUCCESS)
    {
        LOGINFO(FALSE, FALSE, "%s\n", "Self Rebuild Success");
        rename(str_build_bin, str_build_bin_old);
        rename(str_build_bin_new, str_build_bin);
        remove(str_build_bin_old);

        execvp(argv[0], (str *const *)argv);
        LOGFATAL(FALSE, FALSE, ERR_EXECVP_FAIL, "%s\n", "'build"EXE"' Failed, Process Aborted");
        cmd_fail();
    }

    LOGFATAL(FALSE, FALSE, engine_err, "%s\n", "Self-Rebuild Failed, Process Aborted");
    cmd_fail();
}

u32 _engine_build(const str *out_dir)
{
    if (
            is_dir_exists("engine", TRUE) != ERR_SUCCESS ||
            is_dir_exists(out_dir, TRUE) != ERR_SUCCESS)
        return engine_err;

    cmd_push(&_cmd, COMPILER);
    cmd_push(&_cmd, "engine/assets.c");
    cmd_push(&_cmd, "engine/collision.c");
    cmd_push(&_cmd, "engine/core.c");
    cmd_push(&_cmd, "engine/dir.c");
    cmd_push(&_cmd, "engine/input.c");
    cmd_push(&_cmd, "engine/logger.c");
    cmd_push(&_cmd, "engine/math.c");
    cmd_push(&_cmd, "engine/memory.c");
    cmd_push(&_cmd, "engine/"ENGINE_FILE_NAME_PLATFORM);
    cmd_push(&_cmd, "engine/shaders.c");
    cmd_push(&_cmd, "engine/string.c");
    cmd_push(&_cmd, "engine/text.c");
    cmd_push(&_cmd, "engine/time.c");
    cmd_push(&_cmd, "engine/ui.c");
    cmd_push(&_cmd, "engine/include/glad/glad.c");
    cmd_push(&_cmd, "-I.");
#if PLATFORM_LINUX
    cmd_push(&_cmd, "-D_GNU_SOURCE");
#endif /* PLATFORM_LINUX */
    cmd_push(&_cmd, "-DGLAD_GLAPI_EXPORT");
    cmd_push(&_cmd, "-DGLAD_GLAPI_EXPORT_BUILD");
    cmd_push(&_cmd, "-shared");
    cmd_push(&_cmd, "-std=c99");
    cmd_push(&_cmd, "-fPIC");
    cmd_push(&_cmd, "-fvisibility=hidden");
    cmd_push(&_cmd, stringf("-ffile-prefix-map=%s=", DIR_PROC_ROOT));
    cmd_push(&_cmd, "-Ofast");
    cmd_push(&_cmd, "-Wall");
    cmd_push(&_cmd, "-Wextra");
    cmd_push(&_cmd, "-Wformat-truncation=0");
    _engine_link_libs(&_cmd);
    cmd_push(&_cmd, "-o");
    cmd_push(&_cmd, "engine/lib/"PLATFORM"/"ENGINE_FILE_NAME_LIB);
    cmd_ready(&_cmd);

    if (flag & FLAG_CMD_SHOW) cmd_show(&_cmd);
    if (flag & FLAG_CMD_RAW) cmd_raw(&_cmd);

    make_dir(stringf("%sengine", out_dir));
    make_dir(stringf("%sengine/logs", out_dir));
    if (engine_err != ERR_SUCCESS && engine_err != ERR_DIR_EXISTS)
        cmd_fail();

    if (copy_dir(
                stringf("%sengine/assets", DIR_PROC_ROOT),
                stringf("%sengine/assets", out_dir), TRUE) != ERR_SUCCESS)
        cmd_fail();

    if (exec(&_cmd, "_engine_build()") != ERR_SUCCESS)
        cmd_fail();

    if (copy_dir("engine/lib/"PLATFORM, out_dir, TRUE) != ERR_SUCCESS)
        cmd_fail();

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void engine_link_libs(Buf *cmd)
{
    Buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    _engine_link_libs(_cmdp);
    cmd_push(_cmdp, "-lfossil");
}

static void _engine_link_libs(Buf *cmd)
{
    u32 i = 0;
    Buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    str temp[CMD_SIZE] = {0};
    snprintf(temp, CMD_SIZE, "%s", "-Lengine/lib/"PLATFORM);
    normalize_slash(temp);
    cmd_push(_cmdp, temp);
    for (;i < arr_len(str_libs); ++i)
        cmd_push(_cmdp, str_libs[i]);
}

static void cmd_show(Buf *cmd)
{
    Buf *_cmdp = cmd;
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

static void cmd_raw(Buf *cmd)
{
    Buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    printf("\nRAW:\n");
    u32 i = 0;
    for (; i < CMD_MEMB; ++i)
    {
        if (!_cmdp->i[i]) break;
        printf("%s ", (str*)_cmdp->i[i]);
    }

    printf("%s", "\n\n");
}

void cmd_push(Buf *cmd, const str *string)
{
    Buf *_cmdp = cmd;
    if (!cmd)
        _cmdp = &_cmd;

    if (_cmdp->cursor >= _cmdp->memb)
    {
        LOGERROR(FALSE, FALSE, ERR_BUFFER_FULL, "%s\n", "cmd Full");
        return;
    }

    if (strlen(string) >= _cmdp->size)
    {
        LOGERROR(FALSE, FALSE, ERR_STRING_TOO_LONG,
                "Failed to Push String '%s' to cmd.i[%"PRIu64"], String Too Long\n", string, _cmdp->cursor);
        return;
    }

    LOGTRACE(FALSE, FALSE, "Pushing String '%s' to cmd.i[%"PRIu64"]..\n", string, _cmdp->cursor);
    strncpy(_cmdp->i[_cmdp->cursor++], string, CMD_SIZE);
}

void cmd_ready(Buf *cmd)
{
    Buf *_cmdp = cmd;
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
    logger_close();
    mem_free((void*)&DIR_PROC_ROOT, CMD_SIZE, "cmd_free().DIR_PROC_ROOT");
    mem_free_buf(&_cmd, "cmd_free()._cmd");
    mem_free_buf(&args, "cmd_free().args");
    _cmd.cursor = 0;
    args.cursor = 0;
}

void cmd_fail(void)
{
    cmd_free();
    _exit(engine_err);
}

static void help(void)
{
    printf("%s",
            "Usage: ./build [options]...\n"
            "Options:\n"
            "    help       print this help\n"
            "    show       show build command\n"
            "    raw        show build command, raw\n");
    _exit(ERR_SUCCESS);
}
