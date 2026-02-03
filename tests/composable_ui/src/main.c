#include "src/h/fossil_engine.h"

int main(int argc, char **argv)
{
    if (
            fsl_engine_init(argc, argv, NULL, NULL, 1280, 720, NULL,
                FSL_FLAG_LOAD_DEFAULT_SHADERS) != FSL_ERR_SUCCESS ||
            fsl_ui_init(FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;

    while (fsl_engine_running(NULL))
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        if (fsl_is_key_press(FSL_KEY_Q))
            fsl_request_engine_close();
    }

cleanup:

    fsl_engine_close();
    return 0;
}
