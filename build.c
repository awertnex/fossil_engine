#include "deps/buildtool/buildtool.h"
#include "engine/h/build.h"

#define DIR_SRC     "src/"
#define DIR_OUT     "Heaven-Hell Continuum/"

#if PLATFORM_WIN
#   define STR_OUT  "\""DIR_OUT"hhc"EXE"\""
#else
#   define STR_OUT  DIR_OUT"hhc"
#endif /* PLATFORM */

static str str_cflags[][CMD_SIZE] =
{
    "-Wall",
    "-Wextra",
    "-Wformat-truncation=0",
    "-ggdb",
};

int main(int argc, char **argv)
{
    /* if error, will fail and exit */
    build_init(argc, argv, "build.c", "build"EXE);

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return build_err;

    cmd_push(NULL, COMPILER);

    if (find_token("release", argc, argv))
        cmd_push(NULL, "-DHHC_RELEASE_BUILD");
    else
    {
        cmd_push(NULL, "-Wall");
        cmd_push(NULL, "-Wextra");
        cmd_push(NULL, "-Wformat-truncation=0");
        cmd_push(NULL, "-ggdb");
    }

    cmd_push(NULL, DIR_SRC"main.c");
    cmd_push(NULL, DIR_SRC"assets.c");
    cmd_push(NULL, DIR_SRC"chunking.c");
    cmd_push(NULL, DIR_SRC"common.c");
    cmd_push(NULL, DIR_SRC"dir.c");
    cmd_push(NULL, DIR_SRC"gui.c");
    cmd_push(NULL, DIR_SRC"input.c");
    cmd_push(NULL, DIR_SRC"player.c");
    cmd_push(NULL, DIR_SRC"terrain.c");
    cmd_push(NULL, DIR_SRC"world.c");
    cmd_push(NULL, "-I.");
    cmd_push(NULL, "-std=c99");
    cmd_push(NULL, "-Ofast");
    cmd_push(NULL, "-Wl,-rpath="RUNTIME_PATH);
    fsl_link_libs(NULL);
    cmd_push(NULL, "-o");
    cmd_push(NULL, STR_OUT);
    cmd_ready(NULL);

    if (exec(&_cmd, "main().cmd") != ERR_SUCCESS)
        cmd_fail();

    if (copy_file("LICENSE", DIR_OUT"LICENSE") != ERR_SUCCESS ||
            copy_dir("assets/", DIR_OUT"assets/", TRUE) != ERR_SUCCESS)
        cmd_fail();

    build_err = ERR_SUCCESS;
    return build_err;
}
