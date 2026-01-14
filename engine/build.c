#include <unistd.h>

#include "h/build.h"
#include "memory.c"
#include "dir.c"
#include "logger.c"
#include "string.c"

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

u32 engine_err = ERR_SUCCESS;
static u32 flag = 0;
str *str_build_root = NULL;
static str str_build_src[CMD_SIZE] = {0};
static str str_build_bin[CMD_SIZE] = {0};
static str str_build_bin_new[CMD_SIZE] = {0};
static str str_build_bin_old[CMD_SIZE] = {0};
Buf cmd = {0};
static u64 cmd_pos = 0;

static u32 is_build_source_changed(void);

/*! @remark can force-terminate process.
 */
static void self_rebuild(char **argv);

/*! -- INTERNAL USE ONLY --;
 */
static void _engine_link_libs(void);

static void cmd_show(void);
static void cmd_raw(void);
static void help(void);

void build_init(int argc, char **argv,
        const str *build_src_name, const str *build_bin_name)
{
    if (find_token("help", argc, argv)) help();
    if (find_token("show", argc, argv)) flag |= FLAG_CMD_SHOW;
    if (find_token("raw", argc, argv)) flag |= FLAG_CMD_RAW;

    logger_init(TRUE, argc, argv);

    if (find_token("LOGFATAL", argc, argv)) log_level_max = LOGLEVEL_FATAL;
    if (find_token("LOGERROR", argc, argv)) log_level_max = LOGLEVEL_ERROR;
    if (find_token("LOGWARN", argc, argv)) log_level_max = LOGLEVEL_WARNING;
    if (find_token("LOGINFO", argc, argv)) log_level_max = LOGLEVEL_INFO;
    if (find_token("LOGDEBUG", argc, argv)) log_level_max = LOGLEVEL_DEBUG;
    if (find_token("LOGTRACE", argc, argv)) log_level_max = LOGLEVEL_TRACE;

    str_build_root = get_path_bin_root();
    if (engine_err != ERR_SUCCESS)
        cmd_fail();

    check_slash(str_build_root);
    normalize_slash(str_build_root);
    change_dir(str_build_root);

    snprintf(str_build_src, CMD_SIZE, "%s", build_src_name);
    normalize_slash(str_build_src);

    snprintf(str_build_bin, CMD_SIZE, "%s", build_bin_name);
    snprintf(str_build_bin_new, CMD_SIZE, "%s_new", build_bin_name);
    snprintf(str_build_bin_old, CMD_SIZE, "%s_old", build_bin_name);

    if (STD != 199901)
    {
        LOGINFO(FALSE, "%s\n", "Rebuilding Self With -std=c99..");
        self_rebuild(argv);
    }

    if (is_build_source_changed() == ERR_SUCCESS)
    {
        LOGINFO(FALSE, "%s\n", "Rebuilding Self..");
        self_rebuild(argv);
    }

    if (mem_alloc_buf(&cmd, CMD_MEMB, CMD_SIZE, "build_init().cmd") != ERR_SUCCESS)
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
        LOGERROR(FALSE, ERR_FILE_NOT_FOUND, "%s\n", "Build Source File Not Found");
        return engine_err;
    }

    if (stat(str_build_bin, &stats) == 0)
        mtime_bin = stats.st_mtime;
    else
    {
        LOGERROR(FALSE, ERR_FILE_NOT_FOUND, "%s\n", "File 'build"EXE"' Not Found");
        return engine_err;
    }

    if (mtime_src && mtime_bin && mtime_src > mtime_bin)
        return ERR_SUCCESS;

    engine_err = ERR_SOURCE_NOT_CHANGE;
    return engine_err;
}

static void self_rebuild(char **argv)
{
    if (mem_alloc_buf(&cmd, 16, CMD_SIZE, "self_rebuild().cmd") != ERR_SUCCESS)
        cmd_fail();

    cmd_push(COMPILER);
    cmd_push(str_build_src);
    cmd_push("-ggdb");
    cmd_push("-std=c99");
    cmd_push("-Wall");
    cmd_push("-Wextra");
    cmd_push("-fno-builtin");
    cmd_push("-Wno-implicit-function-declaration");
    cmd_push("-o");
    cmd_push(str_build_bin_new);
    cmd_ready();

    if (flag & FLAG_CMD_SHOW) cmd_show();
    if (flag & FLAG_CMD_RAW) cmd_raw();

    if (exec(&cmd, "self_rebuild()") == ERR_SUCCESS)
    {
        LOGINFO(FALSE, "%s\n", "Self Rebuild Success");
        rename(str_build_bin, str_build_bin_old);
        rename(str_build_bin_new, str_build_bin);
        remove(str_build_bin_old);

        execvp(argv[0], (str *const *)argv);
        LOGFATAL(FALSE, ERR_EXECVP_FAIL, "%s\n", "'build"EXE"' Failed, Process Aborted");
        cmd_fail();
    }

    LOGFATAL(FALSE, engine_err, "%s\n", "Self-Rebuild Failed, Process Aborted");
    cmd_fail();
}

u32 engine_build(const str *out_dir)
{
    if (
            is_dir_exists("engine", TRUE) != ERR_SUCCESS ||
            is_dir_exists(out_dir, TRUE) != ERR_SUCCESS)
        return engine_err;

    cmd_push(COMPILER);
    cmd_push("engine/collision.c");
    cmd_push("engine/core.c");
    cmd_push("engine/dir.c");
    cmd_push("engine/input.c");
    cmd_push("engine/logger.c");
    cmd_push("engine/math.c");
    cmd_push("engine/memory.c");
    cmd_push("engine/platform_"_PLATFORM".c");
    cmd_push("engine/string.c");
    cmd_push("engine/text.c");
    cmd_push("engine/time.c");
    cmd_push("engine/ui.c");
    cmd_push("engine/include/glad/glad.c");
    cmd_push("-I.");
    cmd_push("-shared");
    cmd_push("-std=c99");
#if PLATFORM_LINUX
    cmd_push("-D_GNU_SOURCE");
#endif /* PLATFORM_LINUX */
    cmd_push("-fPIC");
    cmd_push("-Ofast");
    cmd_push("-Wall");
    cmd_push("-Wextra");
    cmd_push(stringf("-ffile-prefix-map=%s=", str_build_root));
    _engine_link_libs();
    cmd_push("-o");
    cmd_push(stringf("%s"ENGINE_NAME_LIB, out_dir));
    cmd_ready();

    if (flag & FLAG_CMD_SHOW) cmd_show();
    if (flag & FLAG_CMD_RAW) cmd_raw();

    make_dir(stringf("%sengine", out_dir));
    if (engine_err != ERR_SUCCESS && engine_err != ERR_DIR_EXISTS)
        cmd_fail();

    if (
            copy_dir(
                stringf("%sengine/shaders", str_build_root),
                stringf("%sengine/shaders", out_dir), TRUE,
                "rb", "wb") != ERR_SUCCESS ||
            copy_dir(
                stringf("%sengine/assets", str_build_root),
                stringf("%sengine/assets", out_dir), TRUE,
                "rb", "wb") != ERR_SUCCESS)
        cmd_fail();

    if (exec(&cmd, "engine_build()") != ERR_SUCCESS)
        cmd_fail();

    if (copy_file(stringf("%s"ENGINE_NAME_LIB, out_dir),
                "engine/lib/"PLATFORM"/"ENGINE_NAME_LIB,
                "rb", "wb") != ERR_SUCCESS)
        cmd_fail();

    cmd_free();
    engine_err = ERR_SUCCESS;
    _exit(engine_err);
    return engine_err;
}

void engine_link_libs(void)
{
    _engine_link_libs();
    cmd_push("-lfossil");
}

static void _engine_link_libs(void)
{
    u32 i = 0;
    str temp[CMD_SIZE] = {0};
    snprintf(temp, CMD_SIZE, "-L%sengine/lib/"PLATFORM, str_build_root);
    normalize_slash(temp);
    cmd_push(temp);
    for (;i < arr_len(str_libs); ++i)
        cmd_push(str_libs[i]);
}

static void cmd_show(void)
{
    printf("\nCMD:\n");
    u32 i = 0;
    for (; i < CMD_MEMB; ++i)
    {
        if (!cmd.i[i]) break;
        printf("    %.3d: %s\n", i, (str*)cmd.i[i]);
    }

    if (!(flag & FLAG_CMD_RAW))
        putchar('\n');
}

static void cmd_raw(void)
{
    printf("\nRAW:\n");
    u32 i = 0;
    for (; i < CMD_MEMB; ++i)
    {
        if (!cmd.i[i]) break;
        printf("%s ", (str*)cmd.i[i]);
    }

    printf("%s", "\n\n");
}

void cmd_push(const str *string)
{
    if (cmd_pos >= CMD_MEMB - 1)
    {
        LOGERROR(FALSE, ERR_BUFFER_FULL, "%s\n", "cmd Full");
        return;
    }

    if (strlen(string) >= CMD_SIZE - 1)
    {
        LOGERROR(FALSE, ERR_STRING_TOO_LONG, "string '%s' Too Long\n", string);
        return;
    }

    LOGTRACE(FALSE, "Pushing String '%s' to cmd.i[%"PRIu64"]..\n", string, cmd_pos);
    strncpy(cmd.i[cmd_pos++], string, CMD_SIZE);
}

void cmd_ready(void)
{
#if PLATFORM_LINUX
    cmd.i[cmd_pos] = NULL;
#endif

    if (flag & FLAG_CMD_SHOW) cmd_show();
    if (flag & FLAG_CMD_RAW) cmd_raw();
}

void cmd_free(void)
{
    logger_close();
    mem_free((void*)&str_build_root, CMD_SIZE, "cmd_free().str_build_root");
    mem_free_buf(&cmd, "cmd_free().cmd");
    cmd_pos = 0;
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
