#include "deps/buildtool/buildtool.h"
#include "engine/h/common.h"

#define FSL_OMIT_LFOSSIL
#include "engine/h/build.h"

#define DIR_SRC "engine/"
#define DIR_OUT "Heaven-Hell Continuum/"

#if PLATFORM_WIN
#   define STR_OUT  "\""DIR_OUT"hhc"EXE"\""
#else
#   define STR_OUT  DIR_OUT"hhc"
#endif /* PLATFORM */

static str str_dir[][CMD_SIZE] =
{
    DIR_OUT,
    DIR_OUT"engine",
    DIR_OUT"engine/logs",
};

static str str_cflags[][CMD_SIZE] =
{
    "-Wall",
    "-Wextra",
    "-Wformat-truncation=0",
    "-ggdb",
};

int main(int argc, char **argv)
{
    u32 i = 0, token_release = 0;

    /* if error, will fail and exit */
    build_init(argc, argv, "build_engine.c", "build_engine"EXE);

    if (is_dir_exists("engine", TRUE) != ERR_SUCCESS)
        return build_err;

    if (find_token("release", argc, argv))
    {
        token_release = 1;
        str_cflags[0][0] = 0;
        str_cflags[1][0] = 0;
        str_cflags[2][0] = 0;
    }

    cmd_exec(38,
            COMPILER,
            "-shared",
            "-std=c99",
            "-fPIC",
            "-fvisibility=hidden",
            stringf("-ffile-prefix-map=%s=", DIR_BUILDTOOL_BIN_ROOT),
            "-Ofast",
            str_cflags[0],
            str_cflags[1],
            str_cflags[2],
            fsl_str_libs[0],
            fsl_str_libs[1],
            fsl_str_libs[2],
            fsl_str_libs[3],
            fsl_str_libs[4],
            fsl_str_libs[5],
            fsl_str_libs[6],
            "-I.",
            "-DGLAD_GLAPI_EXPORT",
            "-DGLAD_GLAPI_EXPORT_BUILD",
            "engine/include/glad/glad.c",
            stringf("%s", token_release ? "-DFOSSIL_RELEASE_BUILD" : ""),
            "engine/assets.c",
            "engine/collision.c",
            "engine/core.c",
            "engine/dir.c",
            "engine/input.c",
            "engine/logger.c",
            "engine/math.c",
            "engine/memory.c",
            "engine/"FSL_FILE_NAME_PLATFORM,
            "engine/shaders.c",
            "engine/string.c",
            "engine/text.c",
            "engine/time.c",
            "engine/ui.c",
            "-o",
            "engine/lib/"FSL_PLATFORM"/"FSL_FILE_NAME_LIB);

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
