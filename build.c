#include "src/common/engine_info.h"

#include "src/external/buildtool/buildtool.h"
#include "src/buildtool_config.h"

#define DIR_SRC "src/"
#define DIR_DEPS "deps/"
#define DIR_DST "fossil/" /* your project name */

bt_buf cmd = {0};

static str str_cflags[][CMD_SIZE] =
{
    "-shared",
    "-fPIC",
    "-fvisibility=hidden",
    "-std="FSL_ENGINE_C_STD,
    "-D_GNU_SOURCE",
    "-DGLAD_GLAPI_EXPORT",
    "-DGLAD_GLAPI_EXPORT_BUILD",
    "-Ofast"
};

static str str_cflags_debug[][CMD_SIZE] =
{
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wformat-truncation=0",
    "-ggdb"
};

static str str_files[][CMD_SIZE] =
{
    DIR_SRC"external/glad/glad.c",
    DIR_SRC"assets/assets.c",
    DIR_SRC"assets/mesh/mesh.c",
    DIR_SRC"assets/mesh/mesh_loader_fmesh.c",
    DIR_SRC"assets/mesh/mesh_loader_gltf.c",
    DIR_SRC"assets/mesh/mesh_loader_obj.c",
    DIR_SRC"engine/engine.c",
    DIR_SRC"engine/engine_assets.c",
    DIR_SRC"input/input.c",
    DIR_SRC"logger/logger.c",
    DIR_SRC"math/math.c",
    DIR_SRC"math/perlin_noise.c",
    DIR_SRC"memory/memory.c",
    DIR_SRC"physics/collision.c",
    DIR_SRC"shaders/shaders.c",
    DIR_SRC"shaders/shader_pre_processor.c",
    DIR_SRC"string/string.c",
    DIR_SRC"ui/ui.c",
    DIR_SRC"ui/ui_element.c",
    DIR_SRC"ui/ui_event.c",
    DIR_SRC"dir.c",
    DIR_SRC FSL_FILE_NAME_PLATFORM,
    DIR_SRC"time.c"
};

static str str_make_dir[][CMD_SIZE] =
{
    DIR_DST,
    DIR_DST DIR_DEPS,
    DIR_DST DIR_DEPS DIR_DST,
    DIR_DST DIR_DEPS DIR_DST"assets/",
    DIR_DST DIR_DEPS DIR_DST"assets/mesh/",
    DIR_DST DIR_DEPS DIR_DST"common/",
    DIR_DST DIR_DEPS DIR_DST"engine/",
    DIR_DST DIR_DEPS DIR_DST"input/",
    DIR_DST DIR_DEPS DIR_DST"external/",
    DIR_DST DIR_DEPS DIR_DST"external/glad/",
    DIR_DST DIR_DEPS DIR_DST"logger/",
    DIR_DST DIR_DEPS DIR_DST"math/",
    DIR_DST DIR_DEPS DIR_DST"memory/",
    DIR_DST DIR_DEPS DIR_DST"physics/",
    DIR_DST DIR_DEPS DIR_DST"shaders/",
    DIR_DST DIR_DEPS DIR_DST"string/",
    DIR_DST DIR_DEPS DIR_DST"ui/",

    DIR_DST DIR_DST,
    DIR_DST DIR_DST DIR_DST,
    DIR_DST DIR_DST DIR_DST"logs/",

    DIR_DST "lib/",
    DIR_DST "lib/"PLATFORM
};

static str *copy_targets[][48] =
{
    {DIR_SRC"external/glfw3.h",             DIR_DST DIR_DEPS DIR_DST"external/"},
    {DIR_SRC"external/glad/glad.h",         DIR_DST DIR_DEPS DIR_DST"external/glad/"},
    {DIR_SRC"external/glad/khrplatform.h",  DIR_DST DIR_DEPS DIR_DST"external/glad/"},

    {DIR_SRC"buildtool_config.h",           DIR_DST DIR_DEPS DIR_DST},
    {DIR_SRC"fossil_engine.h",              DIR_DST DIR_DEPS DIR_DST},
    {DIR_SRC"assets/asset_types.h",         DIR_DST DIR_DEPS DIR_DST"assets/"},
    {DIR_SRC"assets/assets.h",              DIR_DST DIR_DEPS DIR_DST"assets/"},
    {DIR_SRC"assets/mesh/mesh.h",           DIR_DST DIR_DEPS DIR_DST"assets/mesh/"},
    {DIR_SRC"engine/engine.h",              DIR_DST DIR_DEPS DIR_DST"engine/"},
    {DIR_SRC"engine/engine_assets.h",       DIR_DST DIR_DEPS DIR_DST"engine/"},
    {DIR_SRC"input/input.h",                DIR_DST DIR_DEPS DIR_DST"input/"},
    {DIR_SRC"input/input_key_codes.h",      DIR_DST DIR_DEPS DIR_DST"input/"},
    {DIR_SRC"logger/logger.h",              DIR_DST DIR_DEPS DIR_DST"logger/"},
    {DIR_SRC"math/math.h",                  DIR_DST DIR_DEPS DIR_DST"math/"},
    {DIR_SRC"math/matrix.h",                DIR_DST DIR_DEPS DIR_DST"math/"},
    {DIR_SRC"math/noise.h",                 DIR_DST DIR_DEPS DIR_DST"math/"},
    {DIR_SRC"math/trigonometry.h",          DIR_DST DIR_DEPS DIR_DST"math/"},
    {DIR_SRC"math/vector.h",                DIR_DST DIR_DEPS DIR_DST"math/"},
    {DIR_SRC"memory/memory.h",              DIR_DST DIR_DEPS DIR_DST"memory/"},
    {DIR_SRC"memory/memory_types.h",        DIR_DST DIR_DEPS DIR_DST"memory/"},
    {DIR_SRC"physics/collision.h",          DIR_DST DIR_DEPS DIR_DST"physics/"},
    {DIR_SRC"physics/physics_types.h",      DIR_DST DIR_DEPS DIR_DST"physics/"},
    {DIR_SRC"shaders/shader_types.h",       DIR_DST DIR_DEPS DIR_DST"shaders/"},
    {DIR_SRC"shaders/shaders.h",            DIR_DST DIR_DEPS DIR_DST"shaders/"},
    {DIR_SRC"string/string.h",              DIR_DST DIR_DEPS DIR_DST"string/"},
    {DIR_SRC"ui/ui.h",                      DIR_DST DIR_DEPS DIR_DST"ui/"},
    {DIR_SRC"ui/ui_element.h",              DIR_DST DIR_DEPS DIR_DST"ui/"},
    {DIR_SRC"ui/ui_types.h",                DIR_DST DIR_DEPS DIR_DST"ui/"}
};

int main(int argc, char **argv)
{
    u32 i = 0;

    /* if error, will fail and exit */
    build_init(argc, argv, "build.c", "build"EXE);

    /* ---- building directories -------------------------------------------- */

    if (is_dir_exists(DIR_SRC, TRUE) != ERR_SUCCESS)
        return build_err;

    for (i = 0; i < arr_len(str_make_dir); ++i)
    {
        make_dir(str_make_dir[i]);
        if (build_err != ERR_SUCCESS && build_err != ERR_DIR_EXISTS)
            cmd_fail(&cmd);
    }

    /* ---- building `cmd` -------------------------------------------------- */

    cmd_push(&cmd, COMPILER);

    if (find_token("release", argc, argv))
    {
        LOGINFO(FALSE, "Building For Release..\n");
        cmd_push(&cmd, "-DFOSSIL_RELEASE_BUILD");
    }
    else
    {
        LOGWARNING(0, FALSE, "Building in Debug Mode..\n");
        for (i = 0; i < arr_len(str_cflags_debug); ++i)
            cmd_push(&cmd, str_cflags_debug[i]);
    }

    cmd_push(&cmd, stringf("-ffile-prefix-map=%s=", DIR_BUILDTOOL_BIN_ROOT));
    for (i = 0; i < arr_len(str_cflags); ++i)
        cmd_push(&cmd, str_cflags[i]);

    fsl_engine_set_runtime_path(&cmd);
    for (i = 0; i < arr_len(fsl_str_libs_internal); ++i)
        cmd_push(&cmd, fsl_str_libs_internal[i]);

    for (i = 0; i < arr_len(str_files); ++i)
        cmd_push(&cmd, str_files[i]);

    cmd_push(&cmd, "-o");
    cmd_push(&cmd, "lib/"PLATFORM"/"FSL_FILE_NAME_LIB);
    cmd_ready(&cmd);

    if (exec(&cmd, "build().cmd") != ERR_SUCCESS)
        cmd_fail(&cmd);

    if (
            copy_dir(DIR_SRC"common/",      DIR_DST DIR_DEPS DIR_DST, FALSE) != ERR_SUCCESS ||
            copy_dir(DIR_SRC"external/buildtool/", DIR_DST DIR_DEPS DIR_DST"external/", FALSE) != ERR_SUCCESS ||
            copy_dir(DIR_SRC"h/",           DIR_DST DIR_DEPS DIR_DST, FALSE) != ERR_SUCCESS ||
            copy_file("LICENSE",            DIR_DST DIR_DEPS DIR_DST) != ERR_SUCCESS ||
            copy_file("version.txt",        DIR_DST DIR_DEPS DIR_DST) != ERR_SUCCESS ||

            copy_dir("assets/",             DIR_DST DIR_DST DIR_DST, FALSE) != ERR_SUCCESS ||
            copy_file("LICENSE",            DIR_DST DIR_DST DIR_DST) != ERR_SUCCESS ||
            copy_file("version.txt",        DIR_DST DIR_DST DIR_DST) != ERR_SUCCESS ||

            copy_dir("lib/"PLATFORM,        DIR_DST "lib/"PLATFORM, TRUE) != ERR_SUCCESS ||
            copy_dir("lib/"PLATFORM,        DIR_DST DIR_DST, TRUE) != ERR_SUCCESS)
        cmd_fail(&cmd);

    for (i = 0; i < arr_len(copy_targets); ++i)
        if (copy_file(copy_targets[i][0], copy_targets[i][1]) != ERR_SUCCESS)
            cmd_fail(&cmd);

    return ERR_SUCCESS;
}
