#ifndef HHC_CHUNKING_DEBUG_TOOLS_H
#define HHC_CHUNKING_DEBUG_TOOLS_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/assets/asset_types.h"

#define CHUNK_DEBUG_SCHEDULER_VISUALIZER_OPACITY_DEFAULT 1.0f

void chunk_debug_draw_scheduler_visualizer(const fsl_camera *camera, f32 opacity);

#endif /* HHC_CHUNKING_DEBUG_TOOLS_H */

