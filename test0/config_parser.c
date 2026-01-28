#include <string.h>

#include <src/h/dir.h>
#include <src/h/limits.h>
#include <src/h/memory.h>

#include "h/main.h"

str allowed_chars[126] =
    "          \n  \r                    \"         "
    ",-  0123456789   =                                 _ "
    "abcdefghijklmnopqrstuvwxyz{ }";

enum TokenIndex
{
    TOKEN_MOUSE_SENSITIVITY,
    TOKEN_FOV,
    TOKEN_RENDER_DISTANCE,
    TOKEN_TARGET_FPS,
    TOKEN_COUNT,
};

str tokens[TOKEN_COUNT][NAME_MAX] =
{
    [TOKEN_MOUSE_SENSITIVITY] = "mouse_sensitivity",
    [TOKEN_FOV] = "fov",
    [TOKEN_RENDER_DISTANCE] = "render_distance",
    [TOKEN_TARGET_FPS] = "target_fps",
};

void parse_config(void)
{
    str *file_contents = NULL;
    str string[NAME_MAX] = {0};
    u32 i = 0;
    u64 file_len = fsl_get_file_contents("Heaven-Hell Continuum/config/settings.txt",
            (void*)&file_contents, 1, TRUE);
    if (fsl_err != FSL_ERR_SUCCESS || !file_contents)
        return;

    for (i = 0; i < file_len; ++i)
    {
        if (file_contents[i] > 126 || file_contents[i] != accepted_chars[file_contents[i]])
            continue;

    }

    fsl_mem_free((void*)&file_contents, file_len, "parse_config().file_contents");
}
