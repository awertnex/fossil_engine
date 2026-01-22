#include "engine/build/build.h"
#include "engine/h/build.h"

#define DIR_SRC     "src/"
#define DIR_OUT     "Heaven-Hell Continuum/"

#if PLATFORM_LINUX
#   define STR_OUT  DIR_OUT"hhc"
#elif PLATFORM_WIN
#   define STR_OUT  "\""DIR_OUT"hhc"EXE"\""
#endif /* PLATFORM */

int main(int argc, char **argv)
{
    build_init(argc, argv, "build.c", "build"EXE);

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return build_err;

    cmd_push(NULL, COMPILER);
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
    cmd_push(NULL, "-Wall");
    cmd_push(NULL, "-Wextra");
    cmd_push(NULL, "-Wformat-truncation=0");
    cmd_push(NULL, "-ggdb");
    cmd_push(NULL, "-Wl,-rpath="RUNTIME_PATH);
    engine_link_libs(NULL);
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
