#ifndef HHC_CONTAINER_H
#define HHC_CONTAINER_H

#include "deps/fossil/common/types.h"

#define CONTAINER_HOTBAR_SLOTS_MAX 10
#define CONTAINER_INVENTORY_SLOTS_MAX (CONTAINER_HOTBAR_SLOTS_MAX * 4)

typedef struct hhc_container_slot
{
    u32 id;
    u32 count;
} hhc_container_slot;

#endif /* HHC_CONTAINER_H */
