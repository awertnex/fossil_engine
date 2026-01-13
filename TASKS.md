TASKS:

- [x] display Chunk coordinates
- [x] detect targeted block
- [x] place blocks
- [x] break blocks
- [x] figure out delta time (25 Mar 2025)
- [x] make the player target a ray-caster (01 Jan 2026)
- [x] detect new chunk, allocate memory and spawn accordingly (09 May 2025)
- [x] fix segfault when player target enters non-allocated chunk area
      (29 Mar 2025)
- [x] fix segfault while placing or breaking blocks in non-allocated chunk area
      (02 Apr 2025)
- [x] fix funky chunk states shifting away by 1 unit each chunk (29 Mar 2025)
- [x] change chunk_buff allocation from stack to heap (14 Apr 2025)
- [ ] navigate menus with arrow keys
- [ ] compress chunk block state files
- [x] figure out why the button "Back to Game" is inactive (26 Mar 2025)
- [ ] rewrite entire gui
- [ ] make gameTick stop when game is paused, and save state on quit-to-title
- [x] fix fullscreen, still not going back to original size on toggle off
      (10 May 2025)
- [x] fix fullscreen affecting mouse delta (10 May 2025)
- [x] fix hollow states at top of chunk (22 May 2025)
- [x] fix remove_block block_parse_limit math (21 May 2025)

quirks:
- [x] fix haloing (black traces when drawing an RGB texture onto an RGBA
      framebuffer, more visible when aspect ratio isn't 1:1) (27 Sep 2025)
- [x] fix weird framebuffer scaling on window scaling (possibly reallocate
      framebuffers) (10 Sep 2025)
- [ ] bake skybox into a look-up table, or HDR (retain old code, just because
      it was fun to write)
- [x] limit framerate (10 Jan 2026)
- [x] add camera look-at (19 Dec 2025)
- [ ] fix double press for windows
- [x] fix chunk queue sort (13 Oct 2025)
- [x] make the chunk generation count per frame (09 Oct 2025)
- [x] make mem_map() for windows (DD Nov 2025)
- [x] make mem_commit() for windows (DD Nov 2025)
- [x] make mem_unmap() for windows (DD Nov 2025)
- [x] fix overhead from function 'chunk_tab_shift()' (09 Dec 2025)
- [ ] possibly fix overhead from bubble-sorting in function 'chunking_init()'
- [ ] put blocks in a contiguous array and bind it all to an SSBO, and bind
      chunk pointers to their addresses
