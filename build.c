#include "deps/buildtool/buildtool.h"
#include "src/h/common.h"

#define FSL_OMIT_LFOSSIL
#include "src/h/build.h"

#define DIR_SRC "src/"
#define DIR_DEPS "deps/"
#define DIR_OUT "fossil/" /* your project name */

static str str_dir[][CMD_SIZE] =
{
    DIR_OUT,
    DIR_OUT"engine/",
    DIR_OUT"engine/logs/",
    DIR_OUT"deps/",
    DIR_OUT"deps/fossil/",
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
    build_init(argc, argv, "build.c", "build"EXE);

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return build_err;

    if (find_token("release", argc, argv))
    {
        token_release = 1;
        str_cflags[0][0] = 0;
        str_cflags[1][0] = 0;
        str_cflags[2][0] = 0;
        str_cflags[3][0] = 0;
    }

    for (i = 0; i < arr_len(str_dir); ++i)
    {
        make_dir(str_dir[i]);
        if (build_err != ERR_SUCCESS && build_err != ERR_DIR_EXISTS)
            cmd_fail();
    }

    cmd_exec(39,
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
            str_cflags[3],
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
            DIR_DEPS"glad/glad.c",
            stringf("%s", token_release ? "-DFOSSIL_RELEASE_BUILD" : ""),
            DIR_SRC"assets.c",
            DIR_SRC"collision.c",
            DIR_SRC"core.c",
            DIR_SRC"dir.c",
            DIR_SRC"input.c",
            DIR_SRC"logger.c",
            DIR_SRC"math.c",
            DIR_SRC"memory.c",
            DIR_SRC FSL_FILE_NAME_PLATFORM,
            DIR_SRC"shaders.c",
            DIR_SRC"string.c",
            DIR_SRC"text.c",
            DIR_SRC"time.c",
            DIR_SRC"ui.c",
            "-o",
            "lib/"PLATFORM"/"FSL_FILE_NAME_LIB);

    if (
            copy_dir(DIR_DEPS, DIR_OUT, FALSE) != ERR_SUCCESS ||
            copy_dir(DIR_SRC"h/", DIR_OUT"deps/fossil/", TRUE) != ERR_SUCCESS ||
            copy_dir("lib/"PLATFORM, DIR_OUT, TRUE) != ERR_SUCCESS ||
            copy_dir("assets/", DIR_OUT"engine/assets", TRUE) != ERR_SUCCESS ||
            copy_file("LICENSE", DIR_OUT"engine/") != ERR_SUCCESS ||
            copy_file("LICENSE", DIR_OUT"deps/fossil/") != ERR_SUCCESS)
        cmd_fail();

    return ERR_SUCCESS;
}
