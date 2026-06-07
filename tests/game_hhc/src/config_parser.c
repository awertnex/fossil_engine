#include "deps/fossil/common/limits.h"
#include "deps/fossil/memory/memory.h"

#include "deps/fossil/h/dir.h"

#include "h/main.h"

#include <stdio.h>

str allowed_chars[126] =
    "          \n  \r                    \"         "
    ",-  0123456789   =                                 _ "
    "abcdefghijklmnopqrstuvwxyz{ }";

enum hhc_token_index
{
    HHC_TOKEN_MOUSE_SENSITIVITY,
    HHC_TOKEN_FOV,
    HHC_TOKEN_RENDER_DISTANCE,
    HHC_TOKEN_TARGET_FPS,
    HHC_TOKEN_COUNT
}; /* hhc_token_index */

str tokens[HHC_TOKEN_COUNT][FSL_ID_CAP] = {0};

void parse_config(void)
{
    str *file_contents = NULL;
    str string[FSL_ID_CAP] = {0};
    u32 i = 0;
    u64 file_len = fsl_get_file_contents("Heaven-Hell Continuum/config/settings.txt",
            (void*)&file_contents, TRUE);

    snprintf(tokens[HHC_TOKEN_MOUSE_SENSITIVITY], FSL_ID_CAP, "%s", "mouse_sensitivity");
    snprintf(tokens[HHC_TOKEN_FOV], FSL_ID_CAP, "%s", "fov");
    snprintf(tokens[HHC_TOKEN_RENDER_DISTANCE], FSL_ID_CAP, "%s", "render_distance");
    snprintf(tokens[HHC_TOKEN_TARGET_FPS], FSL_ID_CAP, "%s", "target_fps");

    if (!file_contents)
        return;

    while (i++ < file_len && file_contents[i] < 127 &&
            file_contents[i] == accepted_chars[file_contents[i]])
    {
    }

    if (file_len)
        fsl_mem_free((void*)&file_contents, file_len, "parse_config().file_contents");
}
