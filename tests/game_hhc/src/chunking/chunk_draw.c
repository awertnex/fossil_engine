#include "deps/fossil/logger/logger.h"
#include "deps/fossil/memory/memory.h"

#include "chunk_scheduler.h"
#include "chunking.h"
#include "chunking_internal.h"

#include <stdio.h>

fsl_mem_arena memory_arena_chunk_draw_internal = {0};
hhc_chunk_draw chunk_draw[CHUNK_SCHEDULERS_MAX] = {0};

void chunk_draw_push(hhc_chunk_scheduler *sched)
{
}

void chunk_draw_update_internal(hhc_chunk_draw *draw)
{
    hhc_chunk **start = NULL;
    u32 draw_len = draw->len;
    u32 scan = draw->cursor_scan;
    u32 i = 0;
}
