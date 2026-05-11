#include "deps/buildtool/buildtool.h"
#include "src/h/buildtool_config.h"

#define DIR_SRC "src/"
#define DIR_DEPS "deps/"
#define DIR_DST "fossil/" /* your project name */

_buf cmd = {0};

typedef struct fsl_str_pair fsl_str_pair;
struct fsl_str_pair
{
    str *a;
    str *b;
};

static str str_dir[][CMD_SIZE] =
{
    DIR_DST,
    DIR_DST DIR_DEPS,
    DIR_DST DIR_DEPS DIR_DST,
    DIR_DST DIR_DEPS DIR_DST "logger/",
    DIR_DST DIR_DEPS DIR_DST "memory/",
    DIR_DST DIR_DEPS DIR_DST "shaders/",
    DIR_DST DIR_DST,
    DIR_DST DIR_DST DIR_DST,
    DIR_DST DIR_DST DIR_DST "logs/"
};

static str str_cflags[][CMD_SIZE] =
{
    "-shared",
    "-fPIC",
    "-fvisibility=hidden",
    "-Ofast",
    "-std="FSL_ENGINE_C_STD,
    "-D_GNU_SOURCE",
    "-DGLAD_GLAPI_EXPORT",
    "-DGLAD_GLAPI_EXPORT_BUILD",
    "-I."
};

static str str_cflags_debug[][CMD_SIZE] =
{
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wformat-truncation=0",
    "-ggdb"
};

static str str_files[][CMD_SIZE] =
{
    DIR_DEPS "glad/glad.c",
    DIR_SRC "logger/logger.c",
    DIR_SRC "memory/memory.c",
    DIR_SRC "shaders/shaders.c",
    DIR_SRC "shaders/shader_pre_processor.c",
    DIR_SRC "assets.c",
    DIR_SRC "collision.c",
    DIR_SRC "core.c",
    DIR_SRC "dir.c",
    DIR_SRC "input.c",
    DIR_SRC "math.c",
    DIR_SRC FSL_FILE_NAME_PLATFORM,
    DIR_SRC "string.c",
    DIR_SRC "time.c",
    DIR_SRC "ui.c"
};

static fsl_str_pair copy_targets[] =
{
    {DIR_SRC "logger/logger.h",                 DIR_DST DIR_DEPS DIR_DST "logger/logger.h"},
    {DIR_SRC "logger/logger_macros.h",          DIR_DST DIR_DEPS DIR_DST "logger/logger_macros.h"},
    {DIR_SRC "logger/logger_messages.h",        DIR_DST DIR_DEPS DIR_DST "logger/logger_messages.h"},
    {DIR_SRC "memory/memory.h",                 DIR_DST DIR_DEPS DIR_DST "memory/memory.h"},
    {DIR_SRC "memory/memory_types.h",           DIR_DST DIR_DEPS DIR_DST "memory/memory_types.h"},
    {DIR_SRC "shaders/shader_pre_processor.h",  DIR_DST DIR_DEPS DIR_DST "shaders/shader_pre_processor.h"},
    {DIR_SRC "shaders/shader_types.h",          DIR_DST DIR_DEPS DIR_DST "shaders/shader_types.h"},
    {DIR_SRC "shaders/shaders.h",               DIR_DST DIR_DEPS DIR_DST "shaders/shaders.h"}
};

int main(int argc, char **argv)
{
    u32 i = 0;

    /* if error, will fail and exit */
    build_init(argc, argv, "build.c", "build"EXE);

    /* ---- building directories -------------------------------------------- */

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return build_err;

    for (i = 0; i < arr_len(str_dir); ++i)
    {
        make_dir(str_dir[i]);
        if (build_err != ERR_SUCCESS && build_err != ERR_DIR_EXISTS)
            cmd_fail(&cmd);
    }

    /* ---- building `cmd` -------------------------------------------------- */

    cmd_push(&cmd, COMPILER);

    if (!find_token("release", argc, argv))
    {
        LOGWARNING(0, FALSE, logger_stringf("%s\n", "Building in Debug Mode.."));
        for (i = 0; i < arr_len(str_cflags_debug); ++i)
            cmd_push(&cmd, str_cflags_debug[i]);
    }
    else
    {
        LOGINFO(FALSE, logger_stringf("%s\n", "Building For Release.."));
        cmd_push(&cmd, "-DFOSSIL_RELEASE_BUILD");
    }

    cmd_push(&cmd, stringf("-ffile-prefix-map=%s=", DIR_BUILDTOOL_BIN_ROOT));
    for (i = 0; i < arr_len(str_cflags); ++i)
        cmd_push(&cmd, str_cflags[i]);

    fsl_engine_set_runtime_path(&cmd);
    for (i = 0; i < arr_len(fsl_str_libs_internal); ++i)
        cmd_push(&cmd, fsl_str_libs_internal[i]);

    for (i = 0; i < arr_len(str_files); ++i)
        cmd_push(&cmd, str_files[i]);

    cmd_push(&cmd, "-o");
    cmd_push(&cmd, "lib/" PLATFORM "/" FSL_FILE_NAME_LIB);
    cmd_ready(&cmd);

    if (exec(&cmd, "build().cmd") != ERR_SUCCESS)
        cmd_fail(&cmd);

    if (
            copy_dir(DIR_DEPS,          DIR_DST, FALSE) != ERR_SUCCESS ||
            copy_dir(DIR_SRC "h/",      DIR_DST DIR_DEPS DIR_DST, TRUE) != ERR_SUCCESS ||
            copy_file("LICENSE",        DIR_DST DIR_DEPS DIR_DST) != ERR_SUCCESS ||

            copy_dir("lib/",            DIR_DST, FALSE) != ERR_SUCCESS ||
            copy_dir("lib/" PLATFORM,   DIR_DST DIR_DST, TRUE) != ERR_SUCCESS ||

            copy_dir("assets/",         DIR_DST DIR_DST DIR_DST, FALSE) != ERR_SUCCESS ||
            copy_file("LICENSE",        DIR_DST DIR_DST DIR_DST) != ERR_SUCCESS)
        cmd_fail(&cmd);

    for (i = 0; i < arr_len(copy_targets); ++i)
        if (copy_file(copy_targets[i].a, copy_targets[i].b) != ERR_SUCCESS)
            cmd_fail(&cmd);

    return ERR_SUCCESS;
}
