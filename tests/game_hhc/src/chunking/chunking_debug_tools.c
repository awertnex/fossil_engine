#include "deps/fossil/common/types.h"
#include "deps/fossil/assets/asset_types.h"
#include "deps/fossil/assets/mesh/mesh.h"

#include "deps/fossil/external/glad/glad.h"

#include "../h/main.h"
#include "../h/assets.h"

#include "chunking.h"
#include "chunking_debug_tools.h"

static void chunk_debug_draw_scheduler_visualizer_internal(hhc_chunk_scheduler sched,
        const fsl_mesh *mesh_bounding_box, const fsl_camera *camera,
        f32 color_r, f32 color_g, f32 color_b, f32 color_a);

void chunk_debug_draw_scheduler_visualizer(const fsl_camera *camera, f32 opacity)
{
    fsl_mesh *mesh_p = fsl_mem_handle_get(mesh);

    glClear(GL_DEPTH_BUFFER_BIT);
    chunk_debug_draw_scheduler_visualizer_internal(chunk_sched[2],
            &mesh_p[MESH_CUBE_OF_HAPPINESS], camera,
            0.9f, 0.3f, 0.3f, opacity);
    glClear(GL_DEPTH_BUFFER_BIT);
    chunk_debug_draw_scheduler_visualizer_internal(chunk_sched[1],
            &mesh_p[MESH_CUBE_OF_HAPPINESS], camera,
            0.9f, 0.6f, 0.3f, opacity);
    glClear(GL_DEPTH_BUFFER_BIT);
    chunk_debug_draw_scheduler_visualizer_internal(chunk_sched[0],
            &mesh_p[MESH_CUBE_OF_HAPPINESS], camera,
            0.9f, 0.6f, 0.3f, opacity);
}

static void chunk_debug_draw_scheduler_visualizer_internal(hhc_chunk_scheduler sched,
        const fsl_mesh *mesh_bounding_box, const fsl_camera *camera,
        f32 color_r, f32 color_g, f32 color_b, f32 color_a)
{
    u32 pop = sched.cursor_pop;
    u32 count = sched.count;

    glUniformMatrix4fv(uniform.bounding_box.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&camera->projection.perspective);
    glUniform3f(uniform.bounding_box.size,
            CHUNK_DIAMETER, CHUNK_DIAMETER, CHUNK_DIAMETER);

    while (count--)
    {
        glUniform3f(uniform.bounding_box.position,
                (f32)(sched.p[pop]->pos.x * CHUNK_DIAMETER),
                (f32)(sched.p[pop]->pos.y * CHUNK_DIAMETER),
                (f32)(sched.p[pop]->pos.z * CHUNK_DIAMETER));

        glUniform4f(uniform.bounding_box.color, color_r, color_g, color_b, color_a);
        glBindVertexArray(mesh_bounding_box->vao);
        glDrawElements(GL_LINE_STRIP, 24, GL_UNSIGNED_INT, 0);

        ++pop;
        if (pop == sched.len)
            pop = 0;
    }
}
