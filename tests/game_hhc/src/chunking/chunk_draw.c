#include "deps/fossil/logger/logger.h"
#include "deps/fossil/memory/memory.h"

#include "chunking.h"
#include "chunking_internal.h"

fsl_mem_arena memory_arena_chunk_draw_internal = {0};
hhc_chunk_draw chunk_draw = {0};

void chunk_draw_push(void)
{
}

void chunk_draw_update_internal(void)
{
}
