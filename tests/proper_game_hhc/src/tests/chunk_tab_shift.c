#include <stdio.h>
#include <string.h>

#include <engine/h/core.h>
#include <engine/h/logger.h>
#include <engine/h/math.h>
#include <engine/h/memory.h>
#include "../h/main.h"
#include "../h/chunking.h"
#include "../h/dir.h"
#include "../h/gui.h"

#ifdef CHUNK_BUF_RADIUS
    #undef CHUNK_BUF_RADIUS
    #define CHUNK_BUF_RADIUS SETTING_RENDER_DISTANCE_MAX
#endif /* CHUNK_BUF_RADIUS */

#define COLOR_CHUNK_LOADED  0x4c2607ff
#define COLOR_CHUNK_EDGE    0x5e7a0aff
#define COLOR_CHUNK_SHIFTED 0x990f05ff

str path_grandpath[PATH_MAX] = {0};
str GRANDPATH_DIR[][NAME_MAX] =
{
    "lib/",
    "lib/"PLATFORM"/",
    "assets/",
    "assets/fonts/",
    "assets/shaders/",
};

Font font_mono;
Font font_mono_bold;

Render render =
{
    .title = "Test: Chunk Tab Shift | "GAME_NAME": "GAME_VERSION,
    .size = {1280, 720},
};

static struct /* skybox_data */
{
    str *bin_root;
    f64 mouse_sensitivity;
    f32 camera_distance;
    f32 render_distance;

    f32 pitch, yaw;
    Camera camera;
} core =
{
    .mouse_sensitivity  = SETTING_MOUSE_SENSITIVITY_DEFAULT / 256.0f,
    .camera_distance    = 64.0f,
    .render_distance    = SETTING_RENDER_DISTANCE_MAX,
};

Projection projection = {0};
Uniform uniform = {0};

static ShaderProgram shader_fbo =
{
    .name = "fbo",
    .vertex.file_name = "fbo.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .fragment.file_name = "fbo.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_text =
{
    .name = "text",
    .vertex.file_name = "text.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .geometry.file_name = "text.geom",
    .geometry.type = GL_GEOMETRY_SHADER,
    .fragment.file_name = "text.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static ShaderProgram shader_voxel =
{
    .name = "voxel",
    .vertex.file_name = "voxel.vert",
    .vertex.type = GL_VERTEX_SHADER,
    .geometry.file_name = "voxel.geom",
    .geometry.type = GL_GEOMETRY_SHADER,
    .fragment.file_name = "voxel.frag",
    .fragment.type = GL_FRAGMENT_SHADER,
};

static FBO fbo_world = {0};
static FBO fbo_world_msaa = {0};
static FBO fbo_text = {0};
static FBO fbo_text_msaa = {0};
static Mesh mesh_fbo = {0};
static u32 test_tab[CHUNK_BUF_VOLUME] = {0};

/* ---- callbacks ----------------------------------------------------------- */
static void callback_error(int error, const char* message)
{
    LOGERROR("GLFW: %s\n", message);
}
static void callback_framebuffer_size(GLFWwindow* window, int width, int height)
{
    render.size = (v2i32){width, height};
    core.camera.ratio = (f32)width / (f32)height;
    glViewport(0, 0, render.size.x, render.size.y);

    realloc_fbo(&render, &fbo_world, FALSE, 4);
    realloc_fbo(&render, &fbo_world_msaa, TRUE, 4);
    realloc_fbo(&render, &fbo_text, FALSE, 4);
    realloc_fbo(&render, &fbo_text_msaa, TRUE, 4);
}

/* ---- signatures ---------------------------------------------------------- */
u8 init_everything(void);
void update_input(void);
void mark_test_tab(u8 direction);
void shift_test_tab(u8 direction);
void draw_world(void);
void draw_text(void);
void draw_everything(void);

int init_paths(void)
{
    core.bin_root = get_path_bin_root();
    LOGINFO("Main Directory Path '%s'\n", core.bin_root);

    str string[PATH_MAX] = {0};
    u64 len = arr_len(GRANDPATH_DIR);
    for (u8 i = 0; i < 255 && i < len; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", path_grandpath, GRANDPATH_DIR[i]);
        check_slash(string);
        normalize_slash(string);
        snprintf(GRANDPATH_DIR[i], PATH_MAX, "%s", string);

        if (!is_dir_exists(string))
            make_dir(string);
    }
    return 0;
}

b8 init_gui(void)
{
    str font_path[PATH_MAX] = {0};

    snprintf(font_path, PATH_MAX, "%s%s", GRANDPATH_DIR[DIR_ROOT_FONTS],
            "dejavu-fonts-ttf-2.37/dejavu_sans_mono_ansi.ttf");
    normalize_slash(font_path);
    if (!init_font(&font_mono, FONT_RESOLUTION_DEFAULT, font_path))
        goto cleanup;

    snprintf(font_path, PATH_MAX, "%s%s", GRANDPATH_DIR[DIR_ROOT_FONTS],
            "dejavu-fonts-ttf-2.37/dejavu_sans_mono_bold_ansi.ttf");
    normalize_slash(font_path);
    if (!init_font(&font_mono_bold, FONT_RESOLUTION_DEFAULT, font_path))
        goto cleanup;
    return 0;

cleanup:
    free_font(&font_mono);
    free_font(&font_mono_bold);
    return -1;
}

u8 init_everything(void)
{
    glfwSetErrorCallback(callback_error);

    if (init_glfw(FALSE) != 0 ||
            init_window(&render) != 0 ||
            init_glad() != 0)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetWindowPos(render.window,
            (1920 / 2) - (render.size.x / 2),
            (1080 / 2) - (render.size.y / 2));
    glfwSetWindowSizeLimits(render.window, 200, 200, 1920, 1080);

    glfwSetFramebufferSizeCallback(render.window, callback_framebuffer_size);
    callback_framebuffer_size(render.window, render.size.x, render.size.y);

    /* ---- graphics -------------------------------------------------------- */
    if (
            init_shader_program(
                GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_fbo, "r") != 0 ||
            init_shader_program(
                GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_text, "r") != 0 ||
            init_shader_program(
                GRANDPATH_DIR[DIR_ROOT_SHADERS], &shader_voxel, "r") != 0 ||

            init_fbo(&render, &fbo_world, &mesh_fbo, FALSE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_world_msaa,  NULL, TRUE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_text,        NULL, FALSE, 4, FALSE) != 0 ||
            init_fbo(&render, &fbo_text_msaa,   NULL, TRUE, 4, FALSE) != 0)
        return -1;

    glfwSwapInterval(0); /* vsync off */
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    if (
            init_gui() != 0 ||
            init_text() != 0)
        return -1;

    core.camera =
        (Camera){
            .fovy = SETTING_FOV_DEFAULT,
            .ratio = (f32)render.size.x / (f32)render.size.y,
            .far = GL_CLIP_DISTANCE0,
            .near = 0.05f,
        };

    font_mono.uniform.char_size =
        glGetUniformLocation(shader_text.id, "char_size");
    font_mono.uniform.font_size =
        glGetUniformLocation(shader_text.id, "font_size");
    font_mono.uniform.text_color =
        glGetUniformLocation(shader_text.id, "text_color");

    font_mono_bold.uniform.char_size =
        glGetUniformLocation(shader_text.id, "char_size");
    font_mono_bold.uniform.font_size =
        glGetUniformLocation(shader_text.id, "font_size");
    font_mono_bold.uniform.text_color =
        glGetUniformLocation(shader_text.id, "text_color");

    uniform.voxel.mat_perspective =
        glGetUniformLocation(shader_voxel.id, "mat_perspective");
    uniform.voxel.camera_position =
        glGetUniformLocation(shader_voxel.id, "camera_position");
    uniform.voxel.sky_color =
        glGetUniformLocation(shader_voxel.id, "sky_color");
    uniform.voxel.color =
        glGetUniformLocation(shader_voxel.id, "voxel_color");
}

void update_input(void)
{
    static u8 direction = 0;
    if (is_key_press(KEY_X))
    {
        if (is_key_hold(KEY_LEFT_SHIFT) || is_key_hold(KEY_RIGHT_SHIFT))
            direction = 2;
        else direction = 1;
        mark_test_tab(direction);
    }
    else if (is_key_press(KEY_Y))
    {
        if (is_key_hold(KEY_LEFT_SHIFT) || is_key_hold(KEY_RIGHT_SHIFT))
            direction = 4;
        else direction = 3;
        mark_test_tab(direction);
    }
    else if (is_key_press(KEY_Z))
    {
        if (is_key_hold(KEY_LEFT_SHIFT) || is_key_hold(KEY_RIGHT_SHIFT))
            direction = 6;
        else direction = 5;
        mark_test_tab(direction);
    }

    if (is_key_press(KEY_BACKSPACE) || is_key_press(KEY_ESCAPE))
    {
        direction = 0;
        for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
        {
            if (!test_tab[i]) continue;
            test_tab[i] = COLOR_CHUNK_LOADED;
        }
    }

    if (is_key_press(KEY_ENTER))
        shift_test_tab(direction);
}

void mark_test_tab(u8 direction)
{
    const u8 AXIS = (direction + 1) / 2;
    const i8 INCREMENT = (direction % 2 == 1) - (direction % 2 == 0);

    u32 mirror_index = 0;
    u8 is_on_edge = 0;

    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
        if (test_tab[i] == COLOR_CHUNK_EDGE)
            test_tab[i] = COLOR_CHUNK_LOADED;

    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        if (!test_tab[i]) continue;
        v3u32 coordinates = index_to_coordinates_v3u32(i, CHUNK_BUF_DIAMETER);
        v3u32 _mirror_index =
        {
            i + CHUNK_BUF_DIAMETER - 1 - (coordinates.x * 2),

            (coordinates.z * CHUNK_BUF_LAYER) +
                ((CHUNK_BUF_DIAMETER - 1 - coordinates.y) *
                 CHUNK_BUF_DIAMETER) + coordinates.x,

            ((CHUNK_BUF_DIAMETER - 1 - coordinates.z) * CHUNK_BUF_LAYER) +
                (coordinates.y * CHUNK_BUF_DIAMETER) + coordinates.x,
        };

        v3u8 _is_on_edge = {0};

        switch (INCREMENT)
        {
            case -1:
                _is_on_edge =
                    (v3u8){
                        (coordinates.x == CHUNK_BUF_DIAMETER - 1) ||
                            (test_tab[i + 1] == 0),

                        (coordinates.y == CHUNK_BUF_DIAMETER - 1) ||
                            (test_tab[i + CHUNK_BUF_DIAMETER] == 0),

                        (coordinates.z == CHUNK_BUF_DIAMETER - 1) ||
                            (test_tab[i + CHUNK_BUF_LAYER] == 0),
                    };
                break;

            case 1:
                _is_on_edge =
                    (v3u8){
                        (coordinates.x == 0) || (test_tab[i - 1] == 0),

                        (coordinates.y == 0) ||
                            (test_tab[i - CHUNK_BUF_DIAMETER] == 0),

                        (coordinates.z == 0) ||
                            (test_tab[i - CHUNK_BUF_LAYER] == 0),
                    };
                break;
        }

        switch (AXIS)
        {
            case SHIFT_PX:
            case SHIFT_NX:
                mirror_index = _mirror_index.x;
                is_on_edge = _is_on_edge.x;
                break;

            case SHIFT_PY:
            case SHIFT_NY:
                mirror_index = _mirror_index.y;
                is_on_edge = _is_on_edge.y;
                break;

            case SHIFT_PZ:
            case SHIFT_NZ:
                mirror_index = _mirror_index.z;
                is_on_edge = _is_on_edge.z;
                break;
        }

        if (is_on_edge)
        {
            test_tab[i] = COLOR_CHUNK_SHIFTED;
            if (test_tab[mirror_index])
                test_tab[mirror_index] = COLOR_CHUNK_RENDER;
        }
    }
}

void shift_test_tab(u8 direction)
{
    i8 INCREMENT = (direction % 2 == 1) - (direction % 2 == 0);
    u8 AXIS = (direction + 1) / 2;

    u32 target_index = 0;
    u8 is_on_edge = 0;

    for (u32 i = (INCREMENT == 1) ? 0 : CHUNK_BUF_VOLUME - 1;
            i < CHUNK_BUF_VOLUME; i += INCREMENT)
    {
        if (!test_tab[i]) continue;
        v3u32 coordinates = index_to_coordinates_v3u32(i, CHUNK_BUF_DIAMETER);
        v3u32 _target_index = {0};

        switch (INCREMENT)
        {
            case -1:
                _target_index = (v3u32){
                        (coordinates.x == 0) ? i : i - 1,
                        (coordinates.y == 0) ? i : i - CHUNK_BUF_DIAMETER,
                        (coordinates.z == 0) ? i : i - CHUNK_BUF_LAYER};
                break;

            case 1:
                _target_index = (v3u32){
                        (coordinates.x == CHUNK_BUF_DIAMETER - 1) ? i : i + 1,

                        (coordinates.y == CHUNK_BUF_DIAMETER - 1) ?
                            i : i + CHUNK_BUF_DIAMETER,

                        (coordinates.z == CHUNK_BUF_DIAMETER - 1) ?
                            i : i + CHUNK_BUF_LAYER};
                break;
        }

        switch (AXIS)
        {
            case SHIFT_PX:
            case SHIFT_NX:
                target_index = _target_index.x;
                break;

            case SHIFT_PY:
            case SHIFT_NY:
                target_index = _target_index.y;
                break;

            case SHIFT_PZ:
            case SHIFT_NZ:
                target_index = _target_index.z;
                break;
        }

        test_tab[i] = test_tab[target_index];
        if (test_tab[i] && test_tab[i] == COLOR_CHUNK_RENDER)
        {
            test_tab[target_index] = 0;
        }
    }
}

void update_camera_movement_test(void)
{
    if (glfwGetMouseButton(render.window,
                GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        core.camera_distance += (f32)render.mouse_delta.y * 0.3f;
        core.camera_distance =
            clamp_f32(core.camera_distance, 10.0f, 100.0f);
    }
    else core.pitch +=
        (f32)render.mouse_delta.y * core.mouse_sensitivity;
    core.yaw += (f32)render.mouse_delta.x * core.mouse_sensitivity;

    core.yaw = fmodf(core.yaw, CAMERA_RANGE_MAX);
    if (core.yaw < 0.0f)
        core.yaw += CAMERA_RANGE_MAX;

    core.pitch =
        clamp_f32(core.pitch, -CAMERA_ANGLE_MAX, CAMERA_ANGLE_MAX);

    core.camera.sin_pitch = sin(core.pitch * DEG2RAD);
    core.camera.cos_pitch = cos(core.pitch * DEG2RAD);
    core.camera.sin_yaw =   sin(core.yaw * DEG2RAD);
    core.camera.cos_yaw =   cos(core.yaw * DEG2RAD);
    const f32 SPCH = core.camera.sin_pitch;
    const f32 CPCH = core.camera.cos_pitch;
    const f32 SYAW = core.camera.sin_yaw;
    const f32 CYAW = core.camera.cos_yaw;

    core.camera.pos =
        (v3f32){
            ((CYAW * CPCH) * core.camera_distance),
            -((SYAW * CPCH) * core.camera_distance),
            (SPCH * core.camera_distance),
        };
}

void draw_world(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_world_msaa.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_voxel.id);

    glUniformMatrix4fv(uniform.voxel.mat_perspective, 1, GL_FALSE,
            (GLfloat*)&projection.perspective);

    glUniform3fv(uniform.voxel.camera_position, 1,
            (GLfloat*)&core.camera.pos);

    glUniform3fv(uniform.voxel.sky_color, 1,
            (GLfloat*)(f32[4]){0.3f, 0.5f, 0.9f, 1.0f});

    v3f32 offset_cursor =
    {
        -(CHUNK_BUF_RADIUS + 0.5f),
        -(CHUNK_BUF_RADIUS + 0.5f),
        -(CHUNK_BUF_RADIUS + 0.5f),
    };

    static f32 pulse;
    static v4f32 voxel_color = {0};
    for (u32 i = 0; i < CHUNK_BUF_VOLUME; ++i)
    {
        if (!test_tab[i]) continue;

        offset_cursor = index_to_coordinates_v3f32(i, CHUNK_BUF_DIAMETER);
        glUniform3fv(uniform.voxel.offset_cursor, 1,
                (GLfloat*)&offset_cursor);

        pulse = (sinf((offset_cursor.z * 0.3f) -
                    (render.frame_start * 5.0f)) * 0.1f) + 0.9f;
        glUniform1f(uniform.voxel.size, pulse);

        voxel_color =
            (v4f32){
                (f32)((test_tab[i] >> 24) & 0xff) / 0xff,
                (f32)((test_tab[i] >> 16) & 0xff) / 0xff,
                (f32)((test_tab[i] >> 8) & 0xff) / 0xff,
                (f32)((test_tab[i] & 0xff)) / 0xff,
            };
        glUniform4fv(uniform.voxel.color, 1, (GLfloat*)&voxel_color);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_world_msaa.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_world.fbo);
    glBlitFramebuffer(0, 0, render.size.x, render.size.y, 0, 0,
            render.size.x, render.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void draw_text(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_text_msaa.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    start_text(0, FONT_SIZE_DEFAULT, &font_mono_bold,
            &render, &shader_text, &fbo_text, 1);

    push_text(stringf("FPS[%d]", (u32)(1.0f / render.frame_delta)),
            (v2f32){MARGIN, MARGIN}, 0, 0);
    render_text(0x6f9f3fff);

    push_text(stringf("PITCH[%.2f] YAW[%.2f]\n",
                core.pitch, core.yaw),
            (v2f32){MARGIN, MARGIN + FONT_SIZE_DEFAULT}, 0, 0);
    render_text(0xffffffff);

    push_text(stringf(
                "MOUSE XY: %.2f %.2f\n" "DELTA XY: %.2f %.2f\n",
                render.mouse_position.x, render.mouse_position.y,
                render.mouse_delta.x, render.mouse_delta.y),
            (v2f32){MARGIN, MARGIN +
            (FONT_SIZE_DEFAULT * 2)}, 0, 0);
    render_text(0x3f6f9fff);

    push_text(stringf(
                "CAMERA DISTANCE: %.2f\n"
                "RENDER DISTANCE: %d\n",
                core.camera_distance,
                core.render_distance),
            (v2f32){MARGIN, MARGIN + (FONT_SIZE_DEFAULT * 4)}, 0, 0);
    render_text(0x3f9f3fff);

    push_text(
            "              :     Look Around\n"
            "Hold    :           Zoom and Pan\n"
            " :                  Mark Shift Direction PX\n"
            " :                  Mark Shift Direction PY\n"
            " :                  Mark Shift Direction PZ\n"
            "     + :            Mark Shift Direction NX\n"
            "     + :            Mark Shift Direction NY\n"
            "     + :            Mark Shift Direction NZ\n"
            "     :              Shift Into Marked Direction\n"
            "    /          :    Clear Marking\n"
            " :                  Toggle Cursor\n",
            (v2f32){MARGIN, render.size.y - MARGIN -
            (FONT_SIZE_DEFAULT * 10)}, 0, 0);
    render_text(0x845c5cff);

    push_text(
            "MOUSE MOVEMENT\n"
            "     LMB\n"
            "X\nY\nZ\n"
            "SHIFT X\nSHIFT Y\nSHIFT Z\n"
            "ENTER\n"
            "ESC   BACKSPACE\n"
            "C\n",
            (v2f32){MARGIN, render.size.y - MARGIN -
            (FONT_SIZE_DEFAULT * 10)}, 0, 0);
    render_text(0x9f3f3fff);
    stop_text();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_text_msaa.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_text.fbo);
    glBlitFramebuffer(0, 0, render.size.x, render.size.y, 0, 0,
            render.size.x, render.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void draw_everything(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    draw_world();
    draw_text();

    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader_fbo.id);
    glBindVertexArray(mesh_fbo.vao);
    glBindTexture(GL_TEXTURE_2D, fbo_world.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindTexture(GL_TEXTURE_2D, fbo_text.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

int main(int argc, char **argv)
{
    if ((argc > 2) && !strncmp(argv[1], "LOGLEVEL", 8))
    {
        if (!strncmp(argv[2], "FATAL", 5))
            log_level = LOGLEVEL_FATAL;
        else if (!strncmp(argv[2], "ERROR", 5))
            log_level = LOGLEVEL_ERROR;
        else if (!strncmp(argv[2], "WARN", 4))
            log_level = LOGLEVEL_WARNING;
        else if (!strncmp(argv[2], "INFO", 4))
            log_level = LOGLEVEL_INFO;
        else if (!strncmp(argv[2], "DEBUG", 5))
            log_level = LOGLEVEL_DEBUG;
        else if (!strncmp(argv[2], "TRACE", 5))
            log_level = LOGLEVEL_TRACE;
    }

    init_everything();

    while (!glfwWindowShouldClose(render.window))
    {
        render.frame_start = glfwGetTime();
        render.frame_delta = render.frame_start - render.frame_last;
        render.frame_delta_square = pow(render.frame_delta, 2.0f);
        render.frame_last = render.frame_start;
        glfwPollEvents();
        update_key_states(&render);
        update_input();

        update_mouse_movement(&render);
        update_camera_movement_test();
        update_camera_perspective(&core.camera, &projection);
        draw_everything();

        glfwSwapBuffers(render.window);
    }

cleanup:
    free_font(&font_mono);
    free_font(&font_mono_bold);
    free_mesh(&mesh_fbo);
    free_fbo(&fbo_world);
    free_fbo(&fbo_world_msaa);
    free_fbo(&fbo_text);
    free_fbo(&fbo_text_msaa);
    free_text();
    mem_free((void*)&core.bin_root, strlen(core.bin_root), "core.bin_root");
    free_shader_program(&shader_fbo);
    free_shader_program(&shader_text);
    free_shader_program(&shader_voxel);
    glfwDestroyWindow(render.window);
    glfwTerminate();
    return 0;
}
