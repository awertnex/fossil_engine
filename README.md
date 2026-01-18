# Heaven/Hell Continuum

<h1 align="center">
  <img src="assets/textures/logo/128x128.png" alt="Heaven-Hell Continuum">
</h1>

**a voxel game/engine in C99, with minimal dependencies if any.**


## Download (unzip and launch)

\# | Linux x86_64 (latest) | Windows (latest)
--- | --- | ---
tar | [v0.3.0-beta-linux-x86_64.tar.gz](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.3.0-beta/heaven-hell_continuum-v0.3.0-beta-linux-x86_64.tar.gz)
zip | [v0.3.0-beta-linux-x86_64.zip](https://github.com/awertnex/heaven-hell_continuum/releases/download/v0.3.0-beta/heaven-hell_continuum-v0.3.0-beta-linux-x86_64.zip) | not yet available
[All Release Builds](https://github.com/awertnex/heaven-hell_continuum/blob/main/RELEASE_BUILDS.md)


## Dependencies (already bundled)

- [glfw v3.4](https://github.com/glfw/glfw/releases): platform-independent windowing (headers modified)
- [glad v0.1.36](https://github.com/dav1dde/glad-web): OpenGL function loader (modified)
    - extension: GL_ARB_bindless_texture
- [stb_image.h v2.30](https://github.com/nothings/stb/blob/master/stb_image.h): for loading image data (modified)
- [stb_image_write.h v1.26](https://github.com/nothings/stb/blob/master/stb_image_write.h): for writing images (modified, unused)
- [stb_truetype.h v1.26](https://github.com/nothings/stb/blob/master/stb_truetype.h): for loading font data (modified)
- [dejavu-fonts v2.37](https://github.com/dejavu-fonts/dejavu-fonts): font of choice (modified)
    - dejavu_sans_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_bold_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_mono_ansi.ttf (subset: U+0000-00ff)
    - dejavu_sans_mono_bold_ansi.ttf (subset: U+0000-00ff)

### windows-specific:
- [w64devkit v2.4.0](https://github.com/skeeto/w64devkit): not necessary for runtime, just for building from source


## Note About Versioning
**Before v0.4.0-beta, v0.4.0-dev used to be "development of v0.4.0-beta", after that v0.4.0-dev is "development after v0.4.0-beta".**

## Build From Source

>**NOTES:**
>- for development build, turn off `GAME_RELEASE_BUILD` in `src/h/main.h` (set as `0`), enables TRACE and DEBUG logging.
>- if build successful, you can place the built directory 'Heaven-Hell Continuum' anywhere you wish, that's the entire bundle.
>- the build tool will rebuild itself before building the project if you either have modified its source or didn't build it in C99.


- - -
### for linux x86_64:

1. clone and build:

```bash
git clone --depth=1 https://github.com/awertnex/heaven-hell_continuum.git
cd heaven-hell_continuum/
./build

# optionally, but has to be called before './build'
./build engine
```

2. run:

```bash
./Heaven-Hell\ Continuum/hhc
```

### additional build commands:

- `./build help`: show help
- `./build engine`: build engine; it places it into './lib/linux', run `./build` again to copy it to 'Heaven-Hell Continuum/lib/linux'
- `./build show`: show build command
- `./build raw`: show build command, raw


- - -
### for windows (using any C compiler, a suggestion is "gcc" from "mingw"): (it is very broken)

if you don't already have a C compiler:
- [w64devkit v2.4.0](https://github.com/skeeto/w64devkit) (includes gcc toolchain)

1. clone and build:

```command
git clone --depth=1 https://github.com/awertnex/heaven-hell_continuum.git
cd heaven-hell_continuum
./build.exe

# optionally, but has to be called before './build.exe'
./build.exe engine
```

2. run:

```command
`./Heaven-Hell\ Continuum/hhc.exe`.
```

### additional build commands:

- `./build.exe help`: show help
- `./build.exe engine`: build engine; it places it into './lib/win', run `./build` again to copy it to 'Heaven-Hell Continuum/lib/win'
- `./build.exe show`: show build command
- `./build.exe raw`: show build command, raw


- - -
## Build The Build Tool:

```command
gcc.exe build.c -o build.exe
```

## Contributing:
currently not accepting contributions since the project is in an early stage, but I will be opening contributions soon (saying this as today is 24-Oct-2025)

## Contributors:
- [sofia](https://github.com/EdgySofia666): alpha-testing on windows

