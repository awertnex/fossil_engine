# Buildtool

**A build tool written in C89, built in C89, builds C.**

repo: [buildtool](https://github.com/awertnex/buildtool.git)

## tiny walkthrough

- copy `buildtool/` and `build.c` to your repository.
- `build.c` is the build source file and comes with a template.
- start using the functions in `buildtool/buildtool.h` (examples are at the top of that file).
- bootstrap and build project with:

```bash
gcc build.c -o build
./build
```

>**NOTES:**
>- changelog can be found at the top in 'buildtool/buildtool.h'
>- the function 'build_init()' is important for:
>   - setting the current directory to the build binary's directory (only for the process).
>   - rebuilding build source if either the file has changed or it wasn't built in C89, or if you pass the argument 'self'.
>   - parsing command line arguments:
>       - 'help': show help and exit
>       - 'show': show build command in list format
>       - 'raw': show build command in raw format
>       - 'self': re-build build source
