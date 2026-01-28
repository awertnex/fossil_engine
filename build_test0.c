#include "deps/buildtool/buildtool.h"
#include "src/h/build.h"

#define DIR_SRC     "test0/"
#define DIR_OUT     "game/"

int main(int argc, char **argv)
{
    /* if error, will fail and exit */
    build_init(argc, argv, "build_test0.c", "build_test0"EXE);

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT);

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
    fsl_engine_link_libs(NULL);
    fsl_engine_set_runtime_path(NULL);
    cmd_push(NULL, "-o");
    cmd_push(NULL, DIR_OUT"hhc");
    cmd_ready(NULL);

    if (exec(&_cmd, "main().cmd") != ERR_SUCCESS)
        cmd_fail();

    if (
            copy_dir("fossil/", DIR_OUT, TRUE) != ERR_SUCCESS ||
            copy_dir("test0/assets/", DIR_OUT"assets/", TRUE) != ERR_SUCCESS)
        cmd_fail();

    build_err = ERR_SUCCESS;
    return build_err;
}
