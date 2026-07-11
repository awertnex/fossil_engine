TASKS: [x] = done, [!] = cancelled, 'X' = unknown
DONE DATE: (YYYY MM DD)

STATUS  DONE DATE       TASK
- [x]   (2025 03 25):   figure out delta time
- [x]   (2026 02 01):   rewrite entire gui
- [x]   (2026 01 10):   limit framerate
- [x]   (2025 12 19):   add camera look-at
- [x]   (2025 11 XX):   make `mem_map()`, `mem_commit()` and `mem_unmap()` for
                        windows
- [!]   (          ):   compile glfw into the binary directly
- [x]   (2026 05 10):   move all allocations to one big memory arena (including
                        stack arrays in struct definitions)
- [x]   (2026 05 17):   change deployment package directory structure to make
                        engine headers' relative includes not break in other
                        repos
- [x]   (2026 07 09):   make physics module
- [x]   (2026 05 27):   make camera module
- [x]   (2026 05 24):   do mesh loading and unloading from disk
- [ ]   (          ):   do json parsing

quirks:
- [x]   (2025 09 19):   fix weird framebuffer scaling on window scaling
                        (possibly reallocate framebuffers)
- [ ]   (          ):   fix segfault when allocating smaller than 256 bytes for
                        `size` parameter of function `fsl_mem_alloc_buf()`
- [ ]   (          ):   fix double press for windows
