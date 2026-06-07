#include "../../../fossil/deps/fossil/fossil_engine.h"

fsl_key_bind bind_quit = {0};

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, NULL, 1280, 720, 0) != FSL_ERR_SUCCESS)
        goto cleanup;

    bind_quit = fsl_key_bind_init(FSL_KEY_Q, 0, 0, 0, 0, 0);

    while (fsl_engine_running(NULL))
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        if (fsl_is_key_press(bind_quit))
            fsl_request_engine_close();
    }

cleanup:

    fsl_engine_close();
    return 0;
}
