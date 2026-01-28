# changelog

>**NOTE:**
>- history is mixed with the repo "awertnex/heaven-hell_continuum" since
   this engine originated from the making of that game and later was moved here
   at engine version v0.3.3-beta (2026 Jan 24), which makes v0.3.4-beta the
   first version of the engine as standalone in this repo

- - -
## v<version> (YYYY MMM DD)

#### changes
- `copy_file()` bug fix and new features:
    - Replace clunky usage of `strrchr()` with `fsl_get_base_name()`
    - Copy file permissions
    - Copy file access and modification times
- `copy_dir()` bug fix and new features:
    - Fix parameter `contents_only` not working
    - Copy directory permissions
    - Copy directory access and modification times
- Add error code `FSL_ERR_FILE_STAT_FAIL`

- - -
## v0.3.4-beta (2026 Jan 28)

#### changes
- Move engine from game repo "heaven-hell_continuum" into its own repo
- First version of engine as separate from the game
- Move engine build tool into its own repo and re-introduce as dependency
- Upgrade license from "MIT" license to "Apache 2.0" license
- Change functios:
    - `fsl_get_string()` -> `fsl_get_engine_string()`
    - `fsl_init()` -> `fsl_engine_init()`
    - `fsl_running()` -> `fsl_engine_running()`
    - `fsl_close()` -> `fsl_engine_close()`
- Re-write `README.md` and `TASKS.md`
- Add function `fsl_request_engine_close()` to terminate application
- Add function `fsl_on_time_interval()` to execute code on specific time
  intervals
- Add function `fsl_limit_framerate()` to limit framerate
- Change function `fsl_get_timer()` -> `fsl_is_in_time_window()`
- Change function `fsl_link_libs()` -> `fsl_engine_link_libs()`
- Add function `fsl_engine_set_runtime_path()` to link engine's dependencies
  with the including software for run time

- - -
## v0.3.3-beta (2026 Jan 24)

#### changes
- Make logger easier to use via flags
- Add logger's own little 'check if dir' and 'write file' functions to
  write to its log destinations on disk without modifying global error variable
- Add guards around definitions `TRUE` and `FALSE`

- - -
## v0.3.2-beta (2026 Jan 24)

#### changes
- Add page size alignment for `sys/mman.h` functions:
- Add better memory alignment for memory arena pushes but also tight-packing for
  small pushes:
    I made it so that multiple pushes can use the same memory page as long as
    they're small enough to fit, otherwise the overlapping block will be pushed
    to the next page
- Change macros:
    - `USEC2NSEC` -> `SEC2MSEC`
    - `NSEC2USEC` -> `MSEC2SEC`
- Add ifder wrappers for game release build macro (to use in build tool)
- Improve documentation drastically and organize things
- separate parameter `pos` in function `text_push()` into `pos_x` and `pos_y`
- Put '#include h/common.h' before all includes
- Move build tool out of the engine
- Add defensive check in function `_mem_alloc_buf()`
- Change some logs to the proper log level
- Make logging for debug and release builds toggled at compile time
- Add better 'release' and 'debug' build separation for the engine in the
    build source
- Add function `fsl_get_string()` to get engine-related strings (e.g. title, version)
- Fix very sneaky and annoying segfault:
    - I wrongfully assumed since "if (!p)" protected against `NULL` pointers,
      then it must protect against un-allocated memory... and then I ate dirt
- Advance logger, now logs to console, to screen, to file, and takes flags for
  custom options
- Add function `fsl_shader_free()` to unload individual shaders

- - -
## v0.3.1-beta (2026 Jan 19)

#### changes
- Optimize text rendering loop
- Add arena allocators
- Add function `_mem_remap()` for linux
- Add memory alignment functions:
    `_mem_request_page_size()` (internal use)
    `mem_request_page_size()` (internal use)
    `round_up_u64()`
- Add memory arena allocation functions:
    `_mem_map_arena()`
    `_mem_push_arena()`
    `_mem_unmap_arena()`
- Fix header documentation in `engine/h/memory.h`

- - -
## v0.2.0-beta (2026 Jan 19)

#### changes
- Add engine API exposure definitions in 'h/common.h' for FSLAPI
- Add function 'dir.c/get_base_name()'
- Change 'h/platform.h' -> 'h/process.h'
- Add helpful build commands:
    self: Build build tool
    all: Build build tool and engine
    noproject: don't execute the build function passed to `engine_build()`

- - -
# previous history, from repo: [heaven-hell_continuum](https:/github.com/awertnex/heaven-hell_continuum)

## Heaven-Hell Continuum - v0.4.0-dev

#### changes
- fixed engine build tool not copying required libraries to deployment directory
- improved logging of commands:
    - chunk boundary toggling commands
    - chunk queue visualizer toggling commands
    - bounding box toggling commands
    - flashlight toggling commands
- added function 'engine/dir.c/get_base_name()'
- fixed misaligned GUI logger strings
- added engine API exposure definitions in 'engine/h/common.h'
- changed 'engine/h/platform.h' -> 'engine/h/process.h'
- added helpful build commands:
    - self: build build tool
    - all: build build tool, engine and game
    - noproject: don't execute the build function passed to 'engine_build()'
- added memory manamgement stuff:
    - arena allocators:
        - function '_mem_map_arena()'
        - cool function '_mem_push_arena()', grows dynamically and auto-relocates all resident pointers with it
        - function '_mem_unmap_arena()'
        - functions '_mem_request_page_size()' and 'mem_request_page_size()' (internal use)
    - function '_mem_remap()'
- added page size alignment for 'sys/mman.h' functions
- added better memory alignment for memory arena pushes but also tight-packing for small pushes:
    - I made it so that multiple pushes can use the same memory page as long as they're small enough to fit within it,
      otherwise the overlapping block will be pushed to the next page
- removed build tool into its own repository and re-introduced as a dependency
- advanced logger, now logs to console, to screen, to file, and takes flags for custom options
- fixed very sneaky and annoying segfault:
    - I wrongfully assumed since "if (!p)" protected against NULL pointers it protected against un-allocated memory... and then I ate dirt
- added function 'fsl_shader_free()' to unload individual shaders
- added mouse wheel scrolling in GUI logger (press 'Tab' for 'super debug' to scroll)

#### bugs and flaws
- segfault when allocating smaller than 256 bytes for 'size' in function 'mem_alloc_buf()'

## Heaven-Hell Continuum - v0.4.0-beta (18 Jan 2026)

#### changes
- added player air control while not flying
- made movement kinematic (physically-based):
    - acceleration_rate
    - input: vector that takes raw keyboard input * acceleration_rate
    - acceleration: vector that takes 'Player.input'
    - velocity: vector that accumulates 'Player.acceleration'
    - 'Player.pos' accumulates 'Player.velocity'
    - Player.drag: air drag
- added working collision even at high speeds (see 'bugs and fixes')
- fixed double click infinite loop when a release state is recorded between
  a key press and a key hold (when pressing and releasing too quick)
- added nice color variation to chunk gizmo (Alt + G)
- added caves, a sand biome, and better terrain in general
- added multi-block placement (numbers 1..0, or scroll wheel to select)
- added more blocks
- fixed texture colors (semi-transparency used to be fully opaque)
- added camera mode 'stalker':
    - camera anchors itself to a random block within a certain distance from the
      player and stalks the player, changes anchor if distance increases,
      the distance limit is either the closer between max render distance or the
      hard-coded max anchor distance
    - mouse movement and keyboard controls still control the player,
      not the camera
- added screenshot support
- added logger buffer, now it draws logs on screen
- added logger saving to files on disk (see 'bugs and flaws')
- added command logging (prints only the text to logger buffer, unless error, then prints tag and error code)
- added player death and some death messages
- optimized text rendering
- added coloring per string for text rendering
- added ui drawing module for engine (see 'bugs and flaws')

#### bugs and flaws
- extremely high speeds (e.g. 3000 m/s) break collision detection
- high speeds now segfault
- if logger can't find log directory, it saves logs to engine log directory, if not found, it saves to current working directory (it should not save at all if so)
- 'draw_ui_9_slice()' function doesn't work yet

- - -
## Heaven-Hell Continuum - v0.3.0-beta (10 Dec 2025)

#### changes
- nudging the major version up by 1 is an understatement, this is almost
  an entirely different game from the previous version
- changed directory structure completely
- removed instance directories, now the main directory is the only instance
- added baking for lookup tables for faster runtime fetching
- fixed many chunk priority queue bugs
- added bounding boxes for 'player chunk' and 'player collision-check
  bounding-box'
- added more detail to terrain generation
- made post-processing juicier
- added cool textures
- revived old code completely and removed all scrap code
- fixed chunk queue offset miscalculation
- fixed windows support (10 Dec 2025: actually broke it later)
- fixed z-fighting between overlapped quads and lines
- added basic AABB collision
- added proper skybox and stars
- fixed memory leak from forgetting to zero chunk VAOs and VBOs after deleting
- fixed 'chunk_tab' shifting overhead from resetting all chunks when player
  chunk delta is too large, by skipping zeroing chunks when popping and only
  zeroing chunks when pushing to 'chunk_buf'
- added OpenGL extension: GL_ARB_bindless_texture
- added texturing per block face
- added more textures and assets
- added inventory slots and block selection
- optimized chunking in general quite hard
- improved readability and documentation
- fixed wrong indexing caused by my terrible idea to shift chunks only one axis
  per frame, instead prioritized shifting all required axes before allowing
  anything else to interact with chunks
- added zooming
- added smooth movement
- improved physics a lot
- added player overflow logic:
    - if player crosses 1 chunk boundary beyond world edge, they're teleported
      to 1 chunk boundary before opposite world edge
    - if player enters region of certain distance of chunks near world edge,
      their overflow flag for that axis is triggered, for future, very cool
      logic
- un-spaghettified a lot of code, notably:
    - function 'logic.c/player_state_update()', when handling player movement
      flags, now instead of checking 'if player is flying' with every other
      if-statement that needs it, everything is under one 'if player is
      flying' statement
    - in 'chunking.c', combined functions 'chunk_tab_shift()',
      'chunking_update()' and chain of 'chunk_queue_update()' calls into one
      function 'chunking_update()' (actually makes readability more difficult
      but since it's internal it's better to just call that one function in
      'main.c' and let it handle things related to chunking
- made build tool build engine separately into a dynamic shared library
- simplified build tool by removing some unnecessary commands

- - -
## Heaven-Hell Continuum - v0.2.3-beta (16 Oct 2025)

#### changes
- added image loading with stb_image.h
- added UI parsing shaders
- fixed patchy chunk generation
- fixed player place block and break block
- switched from malloc to mmap for faster and more efficient memory management
- switched tables from stack to heap (beyond a certain render distance,
  nothing allocated, and just froze there)
- baked chunk order indices to a table on disk to avoid runtime re-calculation

- - -
## Heaven-Hell Continuum - v0.2.2-beta (12 Oct 2025)

#### changes
- optimized chunk rendering:
    - added queues for processing chunks at a reasonable rate per frame
    - changed block data from 'u64' -> 'u32' to save memory, and sacrifice
      some performance by calculating position data at meshing time
    - removed render flags from invisible inside chunks
    - meshed each chunk and loaded only the blocks with faces into an array
      to send to the gpu
    - added 'vbo_len' member to struct 'Chunk' to determine draw-call length
      (major performance gain)
    - changed some loop iteration to linear pointer iteration
- added silly fog
- better chunk rendering:
    - added buffer that sorts chunks based on distance from player:
        - helped push dirty chunks to meshing queue based on distance
        - leveraged buffer advantage for depth sorting as well
        - leveraged buffer for breaking the draw loop short
- better chunk generation performance:
    - added chunk queue buffer to process dirty chunks:
        - added 'rate_chunk' to limit number of chunks processed per frame
        - added 'rate_block' to limit number of blocks processed per chunk

- - -
## Heaven-Hell Continuum - v0.2.0-beta (05 Oct 2025)

#### changes
- added skybox colors for day/night cycle
- fixed menu buttons not disappearing after leaving menu (forgot to clear background)
- added windows support for building launcher and tests
- removed 'draw_text_centered()', added arguments 'alignX' and 'alignY' to 'draw_text()'
- added text alignment to left, center, right, top, center and bottom
- added window resize poll events
- fixed bug centering cursor to center of screen instead of center of viewport
- added texture alignment to left, center, right, top, center and bottom
- added functions kill_player() and respawn_player()
- changed chunk drawing into entire chunkBuf drawing
- added block texturing
- fixed player movement
- fixed player bounding box slidy
- fixed window close bugs
- fixed menu input events
- added <shift> + <F3> combination for special debug (e.g. draw bounding boxes)
- fixed bug mouse movement not registering with intended precision
- added secure malloc, free and zero memory
- added secure loading and unloading chunks
- added more definitions for world size and chunk size for more precision
- added get/set block data macros
- added player-position-relative chunk-loading
- added render-distance-based chunk loading
- added block_parse_limit optimization to limit block parsing to last non-air block in chunk block array
- added chunk_tab shifting to correct player targeting
- added world directory structure creation
- added basic terrain generation
- changed chunking system from 2d to 3d chunks (took me like 20 minutes)
    - removed block_parse_limit optimization, because 3d chunks
- added basic 3d renderer in opengl
    - added skybox colors for day/night cycle in renderer
- simplified build tool
- localized dependencies' headers and shared libraries
- changed project name "minecraft.c" -> "heaven-hell_continuum"
- added self-rebuild logic for build tool
- made dynamically-linked libraries local, still dynamic
- removed raylib
- improved engine directory handling (src/engine/dir.c)
- improved memory safety (src/engine/memory.c)
- improved platform abstraction:
    - (src/engine/platform_<platform>.c)
    - (src/engine/h/platform.h)
    - (src/platform_<platform>.c)
    - (src/h/platform.h)
- broke windows support, will fix in next version
- made asset-fetching independent from user's current working directory:
    - fetch '/proc/self/exe' on linux (the executed process' current directory)
- improved logging:
    - added log colors
    - added LOGTRACE for extremely verbose logging
- added font loading and font atlas baking
- added image reading/writing (stb_image_write.h)
- optimized and expanded input handling
- fixed gravity's framerate-dependence
- added easy motion
- added text newline wrapping
- added text alignment support for all 6 modes
- added anti-aliasing
- recovered chunk drawing and chunk generation
- added chunk gizmo
- added debug info for opengl version and other general info

#### bugs
- chunk rendering draws chunk boundary faces for the furthest chunks at chunk-shift direction
- chunk gizmo size is screen size dependent
- gizmo size is screen size dependent
- geometry shaders' precision errors show up sometimes in DEBUG_MORE mode (semi-transparent blocks)
- setting render distance to 0 segfaults (ofc, but I'm not gonna patch that)

- - -
## Heaven-Hell Continuum - v0.1.4-alpha (08 Apr 2025)

#### changes
- added windows support for build tool
- created minecraft.c logo 'resources/logo/'
- added platform layer C files 'linux_minecraft.c' and 'windows_minecraft.c'
- compiled and ran on windows successfully
- added game ticking
- added day/night cycle
- added compiled release_build for windows

- - -
## Heaven-Hell Continuum - v0.1.3 (03 Apr 2025)

#### changes
- fixed segfault while placing or breaking blocks in non-allocated chunk area 
- switched to C99 standard
- changed original resources to avoid copyright
- made build system more difficult to read but easier to use (not tested on windows)

- - -
## Heaven-Hell Continuum - v0.1.2 (01 Apr 2025)

#### changes
- can break and place blocks
- added menu navigation
- fixed breaking and placing blocks in other chunks than 'xy: 0, 0'
- fixed segfault while wandering into unloaded chunks (I returned 0 while I was supposed to return NULL...)
- fixed chunk states shifting by one block into a specific direction with each chunk in the positive direction and shift into the opposite direction with each chunk in the negative direction
    now I know why, I was doing 'at player target xyz', apparently I forgot to do 'at player target xyz % CHUNK_SIZE'.
    I abandoned this project for over 3 months because I couldn't figure out... somehow.
    so the chunk states weren't shifting by one block, they were shifting by a familiar value, if I printed the index number I was targeting I would have gotten somewhere maybe, but doesn't matter, I fixed it
- created instance directory structure
- created info/ directory to populate assets and array info
- wrote a proper logger
- instance directory structure creation successful
- added game launcher
- changed build system from Bash to C
- made 'defines.h' a local file

- - -
## Heaven-Hell Continuum - v0.1.1 (27 Mar 2025)

#### changes
- added many button names for menus and containers enum
- added all menu names enum
- added all container names enum
- implemented more menus (just 1 more, now I have title screen and game menu working well, next I'll do Options)
- got title screen working fine
- organized game flow a little better
- clicking 'Singleplayer' loads the world
- clicking 'Save and Quit to Title' goes back to title screen, but doesn't save, just keeps the game state saved for the session
- fixed fullscreen (it's actually borderless windowed mode)
- made the codebase a lot more readable
- added delta time

- - -
## Heaven-Hell Continuum - v0.1.0 (2025 Mar 24)

**first version, as I have started this project long before I version-controlled it with git**

#### features
- basic gui functionality
- centering text vertically and horizontally
- basic controls:
    - player movement: sprinting, crouching and jumping
    - build, break, look around
    - 5 camera modes:
        - 1st person
        - 3rd person: back
        - 3rd person: front
        - cinematic (anchors to random blocks and targets player)
        - spectator (like the original, but without dragging the player character along)
    - open inventory gui
    - switch active inventory slot
    - show/hide hud
    - show/hide debug info (F3)
- pause screen, back to game and quit game
- minecraft instance directory creation or opening if exists (instance name = argv 1)
- basic chunking, rigid, buggy, but there.
- basic debugger interface, doesn't do anything, just pops up and shows bounding boxes
- my favorite: segfaulting while wandering into unloaded chunks
- my 2nd favorite: chunk states shift by one block into a specific direction with each chunk in the positive direction and shift into the opposite direction with each chunk in the negative direction, don't know why, I've never encountered such goofy behavior and I'm stuck here
