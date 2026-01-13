#include "engine/build.c"

#define DIR_ROOT        "Heaven-Hell Continuum/"
#define DIR_SRC         "src/"
#define ASSET_COUNT     3
#define NEW_DIR_COUNT   1

str str_out_dir[CMD_SIZE] = {0}; /* bundle directory name */

#if PLATFORM_LINUX
    #define STR_OUT     DIR_ROOT"hhc"
#elif PLATFORM_WIN
    #define STR_OUT     "\""DIR_ROOT"hhc"EXE"\""
#endif /* PLATFORM */

int main(int argc, char **argv)
{
    build_init(argc, argv, "build.c", "build"EXE);

    snprintf(str_out_dir, CMD_SIZE, "%s"DIR_ROOT, str_build_root);

    if (find_token("engine", argc, argv))
        engine_build(
                stringf("%sengine/", str_build_root),
                stringf("%s"DIR_ROOT, str_build_root));

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return engine_err;

    u32 i = 0;
    str temp[CMD_SIZE] = {0};
    cmd_push(COMPILER);
    cmd_push(DIR_SRC"main.c");
    cmd_push(DIR_SRC"assets.c");
    cmd_push(DIR_SRC"chunking.c");
    cmd_push(DIR_SRC"dir.c");
    cmd_push(DIR_SRC"gui.c");
    cmd_push(DIR_SRC"input.c");
    cmd_push(DIR_SRC"player.c");
    cmd_push(DIR_SRC"terrain.c");
    cmd_push(DIR_SRC"world.c");
    cmd_push(stringf("-I%s", str_build_root));
    cmd_push("-std=c99");
    cmd_push("-Ofast");
    cmd_push("-Wall");
    cmd_push("-Wextra");
    cmd_push("-Wno-shift-count-overflow");
    cmd_push("-fno-builtin");
    cmd_push("-ggdb");
    snprintf(temp, CMD_SIZE, "%s", "-Wl,-rpath="RUNTIME_PATH);
    normalize_slash(temp);
    cmd_push(temp);
    engine_link_libs();
    cmd_push("-o");
    cmd_push(STR_OUT);
    cmd_ready();

    str str_mkdir[NEW_DIR_COUNT][CMD_SIZE] = {0};
    str str_from[ASSET_COUNT][CMD_SIZE] = {0};
    str str_to[ASSET_COUNT][CMD_SIZE] = {0};
    snprintf(str_mkdir[0],  CMD_SIZE, "%s", str_out_dir);
    snprintf(str_from[0],   CMD_SIZE, "%sLICENSE", str_build_root);
    snprintf(str_from[1],   CMD_SIZE, "%sengine/lib/"PLATFORM, str_build_root);
    snprintf(str_from[2],   CMD_SIZE, "%sassets/", str_build_root);
    snprintf(str_to[0],     CMD_SIZE, "%sLICENSE", str_out_dir);
    snprintf(str_to[1],     CMD_SIZE, "%s", str_out_dir);
    snprintf(str_to[2],     CMD_SIZE, "%sassets/", str_out_dir);

    for (i = 0; i < NEW_DIR_COUNT; ++i)
    {
        normalize_slash(str_mkdir[i]);
        make_dir(str_mkdir[i]);
    }

    for (i = 0; i < ASSET_COUNT; ++i)
    {
        normalize_slash(str_from[i]);
        normalize_slash(str_to[i]);
        if (is_file(str_from[i]) == ERR_SUCCESS)
            copy_file(str_from[i], str_to[i], "rb", "wb");
        else copy_dir(str_from[i], str_to[i], TRUE, "rb", "wb");

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
