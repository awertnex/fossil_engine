#include "../deps/buildtool/buildtool.h"
#include "../src/h/build.h"

#define DIR_ROOT "../"

#define DIR_PROPER_GAME         "proper_game_hhc/"
#define DIR_SRC_PROPER_GAME     DIR_PROPER_GAME"src/"
#define DIR_OUT_PROPER_GAME     DIR_PROPER_GAME"out/"

#define DIR_TEXT_RENDERING      "text_rendering/"
#define DIR_SRC_TEXT_RENDERING  DIR_TEXT_RENDERING"src/"
#define DIR_OUT_TEXT_RENDERING  DIR_TEXT_RENDERING"out/"

#define DIR_NINE_SLICE          "nine_slice/"
#define DIR_SRC_NINE_SLICE      DIR_NINE_SLICE"src/"
#define DIR_OUT_NINE_SLICE      DIR_NINE_SLICE"out/"

#define DIR_COMPOSABLE_UI       "composable_ui/"
#define DIR_SRC_COMPOSABLE_UI   DIR_COMPOSABLE_UI"src/"
#define DIR_OUT_COMPOSABLE_UI   DIR_COMPOSABLE_UI"out/"

u32 build_proper_game(int argc, char **argv);
u32 build_text_rendering(int argc, char **argv);
u32 build_nine_slice(int argc, char **argv);
u32 build_composable_ui(int argc, char **argv);

int main(int argc, char **argv)
{
    /* if error, will fail and exit */
    build_init(argc, argv, "build.c", "build"EXE);

    if (find_token("proper_game_hhc", argc, argv) || find_token("hhc", argc, argv))
        return build_proper_game(argc, argv);
    else if (find_token("text_rendering", argc, argv))
        return build_text_rendering(argc, argv);
    else if (find_token("nine_slice", argc, argv) || find_token("9s", argc, argv))
        return build_nine_slice(argc, argv);
    else if (find_token("composable_ui", argc, argv) || find_token("ui", argc, argv))
        return build_composable_ui(argc, argv);
    else
    {
        LOGWARNING(ERR_BUILD_FUNCTION_NOT_FOUND, FALSE,
                "%s\n", "No Test Specified, Pass Test Directory Name As Argument");
        cmd_fail(NULL);
    }

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 build_proper_game(int argc, char **argv)
{
    if (is_dir_exists(DIR_SRC_PROPER_GAME, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT_PROPER_GAME);

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

    cmd_push(NULL, DIR_SRC_PROPER_GAME"main.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"assets.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"chunking.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"common.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"dir.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"gui.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"input.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"player.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"terrain.c");
    cmd_push(NULL, DIR_SRC_PROPER_GAME"world.c");
    cmd_push(NULL, "-I"DIR_ROOT);
    cmd_push(NULL, "-std=c99");
    cmd_push(NULL, "-Ofast");
    cmd_push(NULL, "-L"DIR_ROOT"lib/"PLATFORM);
    fsl_engine_link_libs(NULL);
    fsl_engine_set_runtime_path(NULL);
    cmd_push(NULL, "-o");
    cmd_push(NULL, DIR_OUT_PROPER_GAME"hhc");
    cmd_ready(NULL);

    if (exec(&_cmd, "build_proper_game().cmd") != ERR_SUCCESS)
        cmd_fail(&_cmd);

    if (
            copy_dir(DIR_ROOT"fossil/fossil/", DIR_OUT_PROPER_GAME, TRUE) != ERR_SUCCESS ||
            copy_dir(DIR_PROPER_GAME"assets/", DIR_OUT_PROPER_GAME, FALSE) != ERR_SUCCESS)
        cmd_fail(&_cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 build_text_rendering(int argc, char **argv)
{
    if (is_dir_exists(DIR_SRC_TEXT_RENDERING, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT_TEXT_RENDERING);

    cmd_push(NULL, COMPILER);
    cmd_push(NULL, "-Wall");
    cmd_push(NULL, "-Wextra");
    cmd_push(NULL, "-Wformat-truncation=0");
    cmd_push(NULL, "-ggdb");
    cmd_push(NULL, DIR_SRC_TEXT_RENDERING"main.c");
    cmd_push(NULL, "-I"DIR_ROOT);
    cmd_push(NULL, "-std=c89");
    cmd_push(NULL, "-Ofast");
    cmd_push(NULL, "-L"DIR_ROOT"lib/"PLATFORM);
    fsl_engine_link_libs(NULL);
    fsl_engine_set_runtime_path(NULL);
    cmd_push(NULL, "-o");
    cmd_push(NULL, DIR_OUT_TEXT_RENDERING"text_rendering");
    cmd_ready(NULL);

    if (exec(&_cmd, "build_text_rendering().cmd") != ERR_SUCCESS)
        cmd_fail(&_cmd);

    if (copy_dir(DIR_ROOT"fossil/fossil/", DIR_OUT_TEXT_RENDERING, TRUE) != ERR_SUCCESS)
        cmd_fail(&_cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 build_nine_slice(int argc, char **argv)
{
    if (is_dir_exists(DIR_SRC_NINE_SLICE, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT_NINE_SLICE);

    cmd_push(NULL, COMPILER);
    cmd_push(NULL, "-Wall");
    cmd_push(NULL, "-Wextra");
    cmd_push(NULL, "-Wformat-truncation=0");
    cmd_push(NULL, "-ggdb");
    cmd_push(NULL, DIR_SRC_NINE_SLICE"main.c");
    cmd_push(NULL, "-I"DIR_ROOT);
    cmd_push(NULL, "-std=c89");
    cmd_push(NULL, "-Ofast");
    cmd_push(NULL, "-L"DIR_ROOT"lib/"PLATFORM);
    fsl_engine_link_libs(NULL);
    fsl_engine_set_runtime_path(NULL);
    cmd_push(NULL, "-o");
    cmd_push(NULL, DIR_OUT_NINE_SLICE"9s");
    cmd_ready(NULL);

    if (exec(&_cmd, "build_nine_slice().cmd") != ERR_SUCCESS)
        cmd_fail(&_cmd);

    if (
            copy_dir(DIR_ROOT"fossil/fossil/", DIR_OUT_NINE_SLICE, TRUE) != ERR_SUCCESS ||
            copy_dir(DIR_NINE_SLICE"shaders/", DIR_OUT_NINE_SLICE, TRUE) != ERR_SUCCESS)
        cmd_fail(&_cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 build_composable_ui(int argc, char **argv)
{
    if (is_dir_exists(DIR_SRC_COMPOSABLE_UI, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT_COMPOSABLE_UI);

    cmd_push(NULL, COMPILER);
    cmd_push(NULL, "-Wall");
    cmd_push(NULL, "-Wextra");
    cmd_push(NULL, "-Wformat-truncation=0");
    cmd_push(NULL, "-ggdb");
    cmd_push(NULL, DIR_SRC_COMPOSABLE_UI"main.c");
    cmd_push(NULL, "-I"DIR_ROOT);
    cmd_push(NULL, "-std=c89");
    cmd_push(NULL, "-Ofast");
    cmd_push(NULL, "-L"DIR_ROOT"lib/"PLATFORM);
    fsl_engine_link_libs(NULL);
    fsl_engine_set_runtime_path(NULL);
    cmd_push(NULL, "-o");
    cmd_push(NULL, DIR_OUT_COMPOSABLE_UI"ui");
    cmd_ready(NULL);

    if (exec(&_cmd, "build_composable_ui().cmd") != ERR_SUCCESS)
        cmd_fail(&_cmd);

    if (copy_dir(DIR_ROOT"fossil/fossil/", DIR_OUT_COMPOSABLE_UI, TRUE) != ERR_SUCCESS)
        cmd_fail(&_cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}
