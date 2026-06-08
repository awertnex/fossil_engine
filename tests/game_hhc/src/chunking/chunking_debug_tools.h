#ifndef HHC_CHUNKING_DEBUG_TOOLS_H
#define HHC_CHUNKING_DEBUG_TOOLS_H

#include "deps/fossil/common/common.h"
#include "deps/fossil/common/types.h"
#include "deps/fossil/assets/asset_types.h"

#define CHUNK_GIZMO_COLOR_LOADED fsl_color_v4_to_hex(0.70f, 0.01f, 0.02f, 0.39f)
#define CHUNK_GIZMO_COLOR_VISIBLE fsl_color_v4_to_hex(0.24f, 0.47f, 0.3f, 1.0f)
#define CHUNK_GIZMO_COLOR_FACTOR_INFLUENCE 0.25
#define CHUNK_DEBUG_SCHEDULER_VISUALIZER_OPACITY_DEFAULT 1.0f

void chunk_debug_chunk_gizmo_draw(const fsl_camera *camera);
void chunk_debug_scheduler_visualizer_draw(const fsl_camera *camera, f32 opacity);

#endif /* HHC_CHUNKING_DEBUG_TOOLS_H */

