TASKS: [x] = done, [!] = cancelled

- [x] figure out delta time (25 Mar 2025)
- [ ] rewrite entire gui

quirks:
- [x] fix weird framebuffer scaling on window scaling (possibly reallocate
      framebuffers) (10 Sep 2025)

- [x] limit framerate (10 Jan 2026)
- [x] add camera look-at (19 Dec 2025)
- [ ] fix double press for windows
- [x] make `mem_map()` for windows (XX Nov 2025)
- [x] make `mem_commit()` for windows (XX Nov 2025)
- [x] make `mem_unmap()` for windows (XX Nov 2025)
- [ ] fix segfault when allocating smaller than 256 bytes for `size` parameter
      of function `mem_alloc_buf()`
