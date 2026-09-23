# Fossil Engine - v0.16.0-dev

**a headless game engine written in C89, with minimal dependencies.**

Goals of this project:
- to make 2D and/or 3D games, and general software (such as image editors, 3D
  editors, or GUI-based text editors and such)
- to provide simple, general-purpose (mostly) implementations so to give the
  user finer control (which makes the engine verbose), although some areas of
  the engine are less versatile still
- to compile using only a C compiler

- - -

## Engine Start
- engine uses OpenGL as its primary graphics API
- all source code for the engine is in the directory `src/`
- quick examples using the engine are in the directory `examples/`

## Quick Examples
- a simple window:
```c
#include "fossil/fossil_engine.h" /* or wherever it's located */

fsl_key_bind bind_quit = {0}; /* to exit */

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, NULL, 800, 800, 0) != FSL_ERR_SUCCESS)
        goto cleanup;

    bind_quit = fsl_key_bind_init(FSL_KEY_Q, 0, 0, 0, 0);

    while (fsl_engine_running(NULL)) /* main loop function */
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        if (fsl_is_key_press(bind_quit))
            fsl_request_engine_close();
    }

cleanup:

    fsl_engine_close(); /* engine cleanup */
    return 0;
}
```

## Engine Modules
- `src/external/`: engine's dependencies.
- `src/common/`: most important module, has common definitions, base data types, error codes and API control.
- `src/string/`: string handling, token-searching and basic hashing algorithms.
- `src/math/`: math functions, perlin noise functions and some RNG stuff.
- `src/logger/`: engine's logging system, prints to the command line and an internal history buffer.
- `src/assets/`: asset handling and pipeline, handles fonts, textures, meshes, camera and projection and graphics API buffers.
- `src/engine/`: engine's entry point, main functions and engine's own assets.
- `src/h/`: old global directory for headers, all headers used to be here.
- `src/input/`: keyboard and mouse input handling.
- `src/memory/`: memory handling, memory arenas, with some security checks.
- `src/physics/`: physics-handling, collisions and forces.
- `src/plugins/`: any external plug-ins that use the engine (right now plug-ins are compiled directly into the engine binary).
- `src/shaders/`: pre-processing, hot-reloading and fall-back for the graphics API's shaders.
- `src/ui/`: UI library, panels, buttons and interactivity.
- `src/buildtool_config.h`: definitions and functions that the linking software can use to build with the engine.
- `src/fossil_engine.h`: header that includes the entire engine.

## Supported Platforms
- Linux x86_64
- Windows (broken right now)

## Dependencies (already bundled)

- [buildtool v1.8.7](https://github.com/awertnex/buildtool): build tool used to build the engine
- [glfw v3.4](https://github.com/glfw/glfw): platform-independent windowing (headers modified)
    - modifications: remove `__cplusplus` guard, since the engine is C-only
- [glad v0.1.36](https://github.com/dav1dde/glad-web): OpenGL function loader (modified)
    - extensions: GL_ARB_bindless_texture
    - modifications: remove `__cplusplus` guard, since the engine is C-only
- [stb_truetype.h v1.26](https://github.com/nothings/stb/blob/master/stb_truetype.h): loading font data (modified)
    - modifications: change all // comments to /* */ block comments to support C89 standard
- [stb_image.h v2.30](https://github.com/nothings/stb/blob/master/stb_image.h): loading image data (modified)
    - modifications: change all // comments to /* */ block comments to support C89 standard
- [stb_image_write.h v1.26](https://github.com/nothings/stb/blob/master/stb_image_write.h): writing images (modified, unused)
    - modifications: change all // comments to /* */ block comments to support C89 standard
- [dejavu-fonts v2.37](https://github.com/dejavu-fonts/dejavu-fonts): fonts of choice (modified)
    - modifications:
        - (subset: U+0000-00ff): dejavu_sans_ansi.ttf
        - (subset: U+0000-00ff): dejavu_sans_bold_ansi.ttf
        - (subset: U+0000-00ff): dejavu_sans_mono_ansi.ttf
        - (subset: U+0000-00ff): dejavu_sans_mono_bold_ansi.ttf

windows-specific:
- [w64devkit v2.4.0](https://github.com/skeeto/w64devkit): not necessary for runtime, just for building from source

>**NOTE ABOUT VERSIONING:** before version v0.3.4-beta, the repo used to be a directory in [heaven-hell_continuum](https://github.com/awertnex/heaven-hell_continuum), so all tags before that are not engine tags and are not aligned with engine version changes.

## Build From Source

>**NOTES:**
>- for release build, run `./build release`.
>- if build successful, you can copy/move the files in `fossil/` into your project's root.
>- files:
>   - `deps/`: required headers for development
>   - `fossil/`: required libraries and files for runtime
>   - `lib/`: required libraries for linking

**linux x86_64:**

clone and build:

```bash
git clone --depth=1 https://github.com/awertnex/fossil_engine.git
cd fossil_engine/
./build
```

bootstrap buildtool (optional):

```bash
cc build.c -o build
```

additional build commands:

- `./build help`: show help and exit
- `./build show`: show build command in list format
- `./build raw`: show build command in raw format
- `./build self`: re-build buildtool
- `./build release`: build as release
- `./build btdebug`: show debug info for buildtool


**windows (using any C compiler):**

>**BIG NOTE:** windows not yet supported well

clone and build:

```command
git.exe clone --depth=1 https://github.com/awertnex/fossil_engine.git
cd.exe fossil_engine
./build.exe
```

bootstrap buildtool (optional):

```command
cc.exe build.c -o build
```

additional build commands:

- `./build.exe help`: show help and exit
- `./build.exe show`: show build command in list format
- `./build.exe raw`: show build command in raw format
- `./build.exe self`: re-build buildtool
- `./build.exe release`: build as release
- `./build.exe btdebug`: show debug info for buildtool

## Contributing:
currently not accepting contributions since the project is in an early stage, but I will be opening contributions soon (saying this as of today: 24 Oct 2025)
