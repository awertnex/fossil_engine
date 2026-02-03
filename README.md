# Fossil Engine

**a game engine written in C89, with minimal dependencies.**

## Dependencies (already bundled)

- [buildtool v1.8.1-beta](https://github.com/awertnex/buildtool): build tool used to build the engine
- [glfw v3.4](https://github.com/glfw/glfw): platform-independent windowing (headers modified)
    - modifications: remove `__cplusplus` support guard, just to keep it completely C
- [glad v0.1.36](https://github.com/dav1dde/glad-web): OpenGL function loader (modified)
    - extensions: GL_ARB_bindless_texture
    - modifications: remove `__cplusplus` support guard, just to keep it completely C
- [stb_truetype.h v1.26](https://github.com/nothings/stb/blob/master/stb_truetype.h): loading font data (modified)
    - modifications: change all // comments to /* */ block comments to support C89 standard
- [dejavu-fonts v2.37](https://github.com/dejavu-fonts/dejavu-fonts): font of choice (modified)
    - modifications:
        - (subset: U+0000-00ff): dejavu_sans_ansi.ttf
        - (subset: U+0000-00ff): dejavu_sans_bold_ansi.ttf
        - (subset: U+0000-00ff): dejavu_sans_mono_ansi.ttf
        - (subset: U+0000-00ff): dejavu_sans_mono_bold_ansi.ttf
- [stb_image.h v2.30](https://github.com/nothings/stb/blob/master/stb_image.h): loading image data (modified)
    - modifications: change all // comments to /* */ block comments to support C89 standard
- [stb_image_write.h v1.26](https://github.com/nothings/stb/blob/master/stb_image_write.h): writing images (modified, unused)
    - modifications: change all // comments to /* */ block comments to support C89 standard

windows-specific:
- [w64devkit v2.4.0](https://github.com/skeeto/w64devkit): not necessary for runtime, just for building from source

>**NOTE ABOUT VERSIONING:** before version v0.3.4-beta, the repo used to be a directory in [heaven-hell_continuum](htpps://github.com/awertnex/heaven-hell_continuum), so all tags before that are not engine tags and are not aligned with engine version changes.

## Build From Source

>**NOTES:**
>- for release build, pass argument "release" into the build tool.
>- if build successful, you can place the files inside `fossil/` in your project's deployment directory.
>- files:
>   - `deps/`: required dependencies for development
>   - `fossil/`: required dependencies and libraries for deployment
>   - `lib/`: required libraries for link time

**linux x86_64:**

clone and build:

```bash
git clone --depth=1 https://github.com/awertnex/fossil_engine.git
cd fossil_engine/
./build
```

if you want, bootstrap the build script:

```bash
cc build.c -o build
```

additional build commands:

- `./build help`: show help and exit
- `./build show`: show build command in list format
- `./build raw`: show build command in raw format
- `./build self`: re-build build tool
- `./build release`: build as release


**windows (using any C compiler):**

>**BIG NOTE:** windows not yet supported well

clone and build:

```command
git.exe clone --depth=1 https://github.com/awertnex/fossil_engine.git
cd.exe fossil_engine
./build.exe
```

if you want, bootstrap the build script:

```command
cc.exe build.c -o build.exe
```

additional build commands:

- `./build.exe help`: show help and exit
- `./build.exe show`: show build command in list format
- `./build.exe raw`: show build command in raw format
- `./build.exe self`: re-build build tool
- `./build.exe release`: build as release

## Contributing:
currently not accepting contributions since the project is in an early stage, but I will be opening contributions soon (saying this as of today: 24 Oct 2025)
