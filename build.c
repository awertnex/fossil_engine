#include "engine/build.c"

#define DIR_OUT         "Heaven-Hell Continuum/"
#define DIR_SRC         "src/"
#define ASSET_COUNT     2

#if PLATFORM_LINUX
    #define STR_OUT     DIR_OUT"hhc"
#elif PLATFORM_WIN
    #define STR_OUT     "\""DIR_OUT"hhc"EXE"\""
#endif /* PLATFORM */

int main(int argc, char **argv)
{
    u32 i = 0;

    build_init(argc, argv, "build.c", "build"EXE);
    make_dir(DIR_OUT);

    if (find_token("engine", argc, argv))
        engine_build(DIR_OUT);

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return engine_err;

    cmd_push(COMPILER);
    cmd_push(DIR_SRC"main.c");
    cmd_push(DIR_SRC"assets.c");
    cmd_push(DIR_SRC"chunking.c");
    cmd_push(DIR_SRC"common.c");
    cmd_push(DIR_SRC"dir.c");
    cmd_push(DIR_SRC"gui.c");
    cmd_push(DIR_SRC"input.c");
    cmd_push(DIR_SRC"player.c");
    cmd_push(DIR_SRC"terrain.c");
    cmd_push(DIR_SRC"world.c");
    cmd_push("-I.");
    cmd_push("-std=c99");
    cmd_push("-Ofast");
    cmd_push("-Wall");
    cmd_push("-Wextra");
    cmd_push("-Wformat-truncation=0");
    cmd_push("-ggdb");
    cmd_push("-Wl,-rpath="RUNTIME_PATH);
    engine_link_libs();
    cmd_push("-o");
    cmd_push(STR_OUT);
    cmd_ready();

    str str_from[ASSET_COUNT][CMD_SIZE] = {0};
    str str_to[ASSET_COUNT][CMD_SIZE] = {0};
    snprintf(str_from[0],   CMD_SIZE, "%sLICENSE", DIR_PROC_ROOT);
    snprintf(str_from[1],   CMD_SIZE, "%sassets/", DIR_PROC_ROOT);
    snprintf(str_to[0],     CMD_SIZE, "%sLICENSE", DIR_OUT);
    snprintf(str_to[1],     CMD_SIZE, "%sassets/", DIR_OUT);

    for (i = 0; i < ASSET_COUNT; ++i)
    {
        if (is_file(str_from[i]) == ERR_SUCCESS)
            copy_file(str_from[i], str_to[i]);
        else copy_dir(str_from[i], str_to[i], TRUE);

        if (engine_err != ERR_SUCCESS)
            goto cleanup;
    }

    if (exec(&cmd, "build") != ERR_SUCCESS)
        goto cleanup;

    cmd_free();
    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:

    cmd_fail();
    return engine_err;
}
