#include "../src/external/buildtool/buildtool.h"
#include "../src/h/buildtool_config.h"

#define DIR_ROOT "../"
#define DIR_DEPS "../fossil/"

#define DIR_GAME                "game_hhc/"
#define DIR_SRC_GAME            DIR_GAME"src/"
#define DIR_OUT_GAME            DIR_GAME"out/"

#define DIR_TEXT_RENDERING      "text_rendering/"
#define DIR_SRC_TEXT_RENDERING  DIR_TEXT_RENDERING"src/"
#define DIR_OUT_TEXT_RENDERING  DIR_TEXT_RENDERING"out/"

#define DIR_NINE_SLICE          "nine_slice/"
#define DIR_SRC_NINE_SLICE      DIR_NINE_SLICE"src/"
#define DIR_OUT_NINE_SLICE      DIR_NINE_SLICE"out/"

#define DIR_COMPOSABLE_UI       "composable_ui/"
#define DIR_SRC_COMPOSABLE_UI   DIR_COMPOSABLE_UI"src/"
#define DIR_OUT_COMPOSABLE_UI   DIR_COMPOSABLE_UI"out/"

#define TEST_NAME_WIDTH 32
#define TEST_NAME_WIDTH_FULL 64

typedef struct fsl_test_info
{
    str *name;      /* test name (e.g., composable_ui) */
    str *abbrev;    /* test name abbreviation (e.g., ui) */
    u32 (*build_func)(int argc, char **argv);
} fsl_test_info;

_buf cmd = {0}; /* build cmd */

u32 build_game(int argc, char **argv);
u32 build_text_rendering(int argc, char **argv);
u32 build_nine_slice(int argc, char **argv);
u32 build_composable_ui(int argc, char **argv);

fsl_test_info test_list[] =
{
    /* name             abbreviation    build function */
    {"game_hhc",        "hhc",          &build_game},
    {"text_rendering",  "txt",          &build_text_rendering},
    {"nine_slice",      "9s",           &build_nine_slice},
    {"composable_ui",   "ui",           &build_composable_ui}
};

int main(int argc, char **argv)
{
    u64 i = 0;
    u64 cursor = 0;

    if (find_token("help", argc, argv))
    {
        printf("%s",
                "Usage: ./build [options]...\n"
                "Options:\n"
                "    help       print this help\n"
                "    list       list available tests\n"
                "    show       show build command in list format\n"
                "    raw        show build command in raw format\n"
                "    self       build build source\n");
        _exit(ERR_SUCCESS);
    }

    /* if error, will fail and exit */
    build_init(argc, argv, "build.c", "build"EXE);

    if (find_token("list", argc, argv))
    {
        printf("%s", "available tests:\n");
        cursor = printf("%s", "    id      name");
        for (; cursor < TEST_NAME_WIDTH; ++cursor)
            putchar(' ');
        printf("%s\n", "abbreviation");
        for (cursor = 0; cursor < TEST_NAME_WIDTH_FULL; ++cursor)
            putchar('-');
        putchar('\n');
        for (i = 0; i < arr_len(test_list); ++i)
        {
            cursor = printf("    %04" PRIu64 "    %s", i, test_list[i].name);
            for (; cursor < TEST_NAME_WIDTH; ++cursor)
                putchar(' ');
            printf("%s\n", test_list[i].abbrev);
        }
        _exit(ERR_SUCCESS);
    }

    for (i = 0; i < arr_len(test_list); ++i)
    {
        if (find_token(test_list[i].name, argc, argv) || find_token(test_list[i].abbrev, argc, argv))
            return test_list[i].build_func(argc, argv);
    }

    LOGWARNING(ERR_BUILD_FUNCTION_NOT_FOUND, FALSE,
            logger_stringf("%s\n", "No Test Specified, Pass Test Directory Name As Argument"));
    cmd_fail(NULL);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 build_game(int argc, char **argv)
{
    if (is_dir_exists(DIR_SRC_GAME, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT_GAME);

    cmd_push(&cmd, COMPILER);

    if (find_token("release", argc, argv))
        cmd_push(&cmd, "-DHHC_RELEASE_BUILD");
    else
    {
        cmd_push(&cmd, "-Wall");
        cmd_push(&cmd, "-Wextra");
        cmd_push(&cmd, "-Wformat-truncation=0");
        cmd_push(&cmd, "-ggdb");
    }

    cmd_push(&cmd, DIR_SRC_GAME"main.c");
    cmd_push(&cmd, DIR_SRC_GAME"assets.c");
    cmd_push(&cmd, DIR_SRC_GAME"chunking.c");
    cmd_push(&cmd, DIR_SRC_GAME"common.c");
    cmd_push(&cmd, DIR_SRC_GAME"dir.c");
    cmd_push(&cmd, DIR_SRC_GAME"gui.c");
    cmd_push(&cmd, DIR_SRC_GAME"input.c");
    cmd_push(&cmd, DIR_SRC_GAME"player.c");
    cmd_push(&cmd, DIR_SRC_GAME"terrain.c");
    cmd_push(&cmd, DIR_SRC_GAME"world.c");
    cmd_push(&cmd, "-I"DIR_DEPS);
    cmd_push(&cmd, "-std=c89");
    cmd_push(&cmd, "-Ofast");
    cmd_push(&cmd, "-L"DIR_ROOT"lib/"PLATFORM);
    fsl_engine_link_libs(&cmd);
    fsl_engine_set_runtime_path(&cmd);
    cmd_push(&cmd, "-o");
    cmd_push(&cmd, DIR_OUT_GAME"hhc");
    cmd_ready(&cmd);

    if (exec(&cmd, "build_game().cmd") != ERR_SUCCESS)
        cmd_fail(&cmd);

    if (
            copy_dir(DIR_ROOT"fossil/fossil/", DIR_OUT_GAME, TRUE) != ERR_SUCCESS ||
            copy_dir(DIR_GAME"assets/", DIR_OUT_GAME, FALSE) != ERR_SUCCESS)
        cmd_fail(&cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 build_text_rendering(int argc, char **argv)
{
    if (is_dir_exists(DIR_SRC_TEXT_RENDERING, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT_TEXT_RENDERING);

    cmd_push(&cmd, COMPILER);
    cmd_push(&cmd, "-Wall");
    cmd_push(&cmd, "-Wextra");
    cmd_push(&cmd, "-Wformat-truncation=0");
    cmd_push(&cmd, "-ggdb");
    cmd_push(&cmd, DIR_SRC_TEXT_RENDERING"main.c");
    cmd_push(&cmd, "-I"DIR_DEPS);
    cmd_push(&cmd, "-std=c89");
    cmd_push(&cmd, "-Ofast");
    cmd_push(&cmd, "-L"DIR_ROOT"lib/"PLATFORM);
    fsl_engine_link_libs(&cmd);
    fsl_engine_set_runtime_path(&cmd);
    cmd_push(&cmd, "-o");
    cmd_push(&cmd, DIR_OUT_TEXT_RENDERING"text_rendering");
    cmd_ready(&cmd);

    if (exec(&cmd, "build_text_rendering().cmd") != ERR_SUCCESS)
        cmd_fail(&cmd);

    if (copy_dir(DIR_ROOT"fossil/fossil/", DIR_OUT_TEXT_RENDERING, TRUE) != ERR_SUCCESS)
        cmd_fail(&cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 build_nine_slice(int argc, char **argv)
{
    if (is_dir_exists(DIR_SRC_NINE_SLICE, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT_NINE_SLICE);

    cmd_push(&cmd, COMPILER);
    cmd_push(&cmd, "-Wall");
    cmd_push(&cmd, "-Wextra");
    cmd_push(&cmd, "-Wformat-truncation=0");
    cmd_push(&cmd, "-Wpedantic");
    cmd_push(&cmd, "-ggdb");
    cmd_push(&cmd, DIR_SRC_NINE_SLICE"main.c");
    cmd_push(&cmd, "-I"DIR_DEPS);
    cmd_push(&cmd, "-std=c89");
    cmd_push(&cmd, "-Ofast");
    cmd_push(&cmd, "-L"DIR_ROOT"lib/"PLATFORM);
    fsl_engine_link_libs(&cmd);
    fsl_engine_set_runtime_path(&cmd);
    cmd_push(&cmd, "-o");
    cmd_push(&cmd, DIR_OUT_NINE_SLICE"9s");
    cmd_ready(&cmd);

    if (exec(&cmd, "build_nine_slice().cmd") != ERR_SUCCESS)
        cmd_fail(&cmd);

    if (
            copy_dir(DIR_ROOT"fossil/fossil/", DIR_OUT_NINE_SLICE, TRUE) != ERR_SUCCESS ||
            copy_dir(DIR_NINE_SLICE"shaders/", DIR_OUT_NINE_SLICE, TRUE) != ERR_SUCCESS)
        cmd_fail(&cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 build_composable_ui(int argc, char **argv)
{
    if (is_dir_exists(DIR_SRC_COMPOSABLE_UI, TRUE) != ERR_SUCCESS)
        return build_err;

    make_dir(DIR_OUT_COMPOSABLE_UI);

    cmd_push(&cmd, COMPILER);
    cmd_push(&cmd, "-Wall");
    cmd_push(&cmd, "-Wextra");
    cmd_push(&cmd, "-Wformat-truncation=0");
    cmd_push(&cmd, "-ggdb");
    cmd_push(&cmd, DIR_SRC_COMPOSABLE_UI"main.c");
    cmd_push(&cmd, "-I"DIR_DEPS);
    cmd_push(&cmd, "-std=c89");
    cmd_push(&cmd, "-Ofast");
    cmd_push(&cmd, "-L"DIR_ROOT"lib/"PLATFORM);
    fsl_engine_link_libs(&cmd);
    fsl_engine_set_runtime_path(&cmd);
    cmd_push(&cmd, "-o");
    cmd_push(&cmd, DIR_OUT_COMPOSABLE_UI"ui");
    cmd_ready(&cmd);

    if (exec(&cmd, "build_composable_ui().cmd") != ERR_SUCCESS)
        cmd_fail(&cmd);

    if (copy_dir(DIR_ROOT"fossil/fossil/", DIR_OUT_COMPOSABLE_UI, TRUE) != ERR_SUCCESS)
        cmd_fail(&cmd);

    build_err = ERR_SUCCESS;
    return build_err;
}
