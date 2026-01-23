#include "deps/buildtool/buildtool.h"
#include "engine/h/common.h"

#define OMIT_LFOSSIL
#include "engine/h/build.h"

#define DIR_SRC     "engine/"
#define DIR_OUT     "Heaven-Hell Continuum/"

#if PLATFORM_LINUX
#   define STR_OUT  DIR_OUT"hhc"
#elif PLATFORM_WIN
#   define STR_OUT  "\""DIR_OUT"hhc"EXE"\""
#endif /* PLATFORM */

static str str_dir[][CMD_SIZE] =
{
    DIR_OUT,
    DIR_OUT"engine",
    DIR_OUT"engine/logs",
};

int main(int argc, char **argv)
{
    u32 i = 0;

    if (build_init(argc, argv, "build_engine.c", "build_engine"EXE) != ERR_SUCCESS)
        return build_err;

    if (is_dir_exists("engine", TRUE) != ERR_SUCCESS)
        return build_err;

    cmd_exec(37,
            COMPILER,
            "-shared",
            "-std=c99",
            "-fPIC",
            "-fvisibility=hidden",
            stringf("-ffile-prefix-map=%s=", DIR_BUILDTOOL_BIN_ROOT),
            "-Ofast",
            "-Wall",
            "-Wextra",
            "-Wformat-truncation=0",
            str_engine_libs[0],
            str_engine_libs[1],
            str_engine_libs[2],
            str_engine_libs[3],
            str_engine_libs[4],
            str_engine_libs[5],
            str_engine_libs[6],
            "-I.",
            "-DGLAD_GLAPI_EXPORT",
            "-DGLAD_GLAPI_EXPORT_BUILD",
            "engine/include/glad/glad.c",
            "engine/assets.c",
            "engine/collision.c",
            "engine/core.c",
            "engine/dir.c",
            "engine/input.c",
            "engine/logger.c",
            "engine/math.c",
            "engine/memory.c",
            "engine/"ENGINE_FILE_NAME_PLATFORM,
            "engine/shaders.c",
            "engine/string.c",
            "engine/text.c",
            "engine/time.c",
            "engine/ui.c",
            "-o",
            "engine/lib/"PLATFORM"/"ENGINE_FILE_NAME_LIB);

    for (i = 0; i < arr_len(str_dir); ++i)
    {
        make_dir(str_dir[i]);
        if (build_err != ERR_SUCCESS && build_err != ERR_DIR_EXISTS)
            cmd_fail();
    }

    if (
            copy_dir(
                stringf("%sengine/assets", DIR_BUILDTOOL_BIN_ROOT),
                DIR_OUT"engine/assets", TRUE) != ERR_SUCCESS ||

            copy_dir("engine/lib/"PLATFORM, DIR_OUT, TRUE) != ERR_SUCCESS)
        cmd_fail();

    return ERR_SUCCESS;
}
