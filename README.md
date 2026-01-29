# Fossil Engine

**a game engine/library in C99, with minimal dependencies if any.**

## Dependencies (already bundled)

- [buildtool](https://github.com/awertnex/buildtool): build tool used to build the engine
- [glfw v3.4](https://github.com/glfw/glfw/releases): platform-independent windowing (headers modified)
- [glad v0.1.36](https://github.com/dav1dde/glad-web): OpenGL function loader (modified)
    - extension: GL_ARB_bindless_texture
- [stb_truetype.h v1.26](https://github.com/nothings/stb/blob/master/stb_truetype.h): for loading font data (modified)
- [dejavu-fonts v2.37](https://github.com/dejavu-fonts/dejavu-fonts): font of choice (modified)
    - dejavu_sans_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_bold_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_mono_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_mono_bold_ansi.ttf (subset: U+0000-00ff)
- [stb_image.h v2.30](https://github.com/nothings/stb/blob/master/stb_image.h): for loading image data (modified)
- [stb_image_write.h v1.26](https://github.com/nothings/stb/blob/master/stb_image_write.h): for writing images (modified, unused)

### windows-specific:
- [w64devkit v2.4.0](https://github.com/skeeto/w64devkit): not necessary for runtime, just for building from source

## Note About Versioning
**Before version v0.3.4-beta, the repo used to be a directory in [heaven-hell_continuum](htpps://github.com/awertnex/heaven-hell_continuum), so all tags before that are not engine tags and are not aligned with engine version changes.**

## Build From Source

>**NOTES:**
>- for release build, pass argument "release" into the build tool.
>- if build successful, you can place the files inside `fossil/` in your project's deployment directory.

- - -
### for linux x86_64:

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

### additional build commands:

- `./build help`: show help and exit
- `./build show`: show build command in list format
- `./build raw`: show build command in raw format
- `./build self`: re-build build tool
- `./build release`: build as release

- - -
### for windows (using any C compiler, a suggestion is "gcc" from "mingw"):

if you don't already have a C compiler:
- [w64devkit v2.4.0](https://github.com/skeeto/w64devkit) (includes gcc toolchain)

clone and build:

```command
git clone --depth=1 https://github.com/awertnex/fossil_engine.git
cd fossil_engine
./build.exe
```

if you want, bootstrap the build script:

```command
cc.exe build.c -o build.exe
```

### additional build commands:

- `./build.exe help`: show help and exit
- `./build.exe show`: show build command in list format
- `./build.exe raw`: show build command in raw format
- `./build.exe self`: re-build build tool
- `./build.exe release`: build as release

- - -
## Contributing:
currently not accepting contributions since the project is in an early stage, but I will be opening contributions soon (saying this as of today: 24 Oct 2025)
