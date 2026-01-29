#include <engine/core.c>
#include <engine/logger.c>
#include <engine/dir.c>
#include <engine/math.c>

#define DIR_SHADERS "src/shaders/"
#define SETTING_DAY_TICKS_MAX   24000 /*temp*/

/* ---- declarations -------------------------------------------------------- */

Render render =
{
    .size = {854, 480},
    .title = "heaven-hell continuum",
};

u16 state = 0;
f64 game_start_time = 0.0f;
u64 game_tick = 0;
u64 game_days = 0;
Camera camera = {0};

ShaderProgram shader_fbo =
{
    .name = "fbo",
    .vertex =
    {
        .file_name = DIR_SHADERS"fbo.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = DIR_SHADERS"fbo.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

ShaderProgram shader_default =
{
    .name = "default",
    .vertex =
    {
        .file_name = DIR_SHADERS"default.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = DIR_SHADERS"default.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

ShaderProgram shader_skybox =
{
    .name = "skybox",
    .vertex =
    {
        .file_name = DIR_SHADERS"skybox.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = DIR_SHADERS"skybox.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

ShaderProgram shader_gizmo =
{
    .name = "gizmo",
    .vertex =
    {
        .file_name = DIR_SHADERS"gizmo.vert",
        .type = GL_VERTEX_SHADER,
    },

    .fragment =
    {
        .file_name = DIR_SHADERS"gizmo.frag",
        .type = GL_FRAGMENT_SHADER,
    },
};

Projection projection = {0};

struct /* skybox_data */
{
    f32 time;
    v3f32 sun_rotation;
    v3f32 color;
} skybox_data;


struct /* uniform_default */
{
    int mat_perspective;
    int camera_position;
    int sun_rotation;
    int sky_color;
} uniform_default;

struct /* uniform_skybox */
{
    int camera_position;
    int mat_rotation;
    int mat_orientation;
    int mat_projection;
    int time;
    int sun_rotation;
    int sky_color;
} uniform_skybox;

struct /* uniform_gizmo */
{
    int render_ratio;
    int mat_target;
    int mat_rotation;
    int mat_orientation;
    int mat_projection;
} uniform_gizmo;

GLuint fbo_skybox;
GLuint color_buf_skybox;
GLuint rbo_skybox;

GLuint fbo_world;
GLuint color_buf_world;
GLuint rbo_world;

GLuint fbo_hud;
GLuint color_buf_hud;
GLuint rbo_hud;

Mesh mesh_fbo = {0};

#define VBO_LEN_SKYBOX  24
#define EBO_LEN_SKYBOX  36
Mesh mesh_skybox = {0};

#define VBO_LEN_COH     24
#define EBO_LEN_COH     36
Mesh mesh_cube_of_happiness = {0};

#define VBO_LEN_GIZMO   51
#define EBO_LEN_GIZMO   90
Mesh mesh_gizmo = {0};

/* ---- callbacks ----------------------------------------------------------- */

void error_callback(int error, const char* message)
{
    LOGERROR("%s\n", message);
}
static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height);
static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
static void gl_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

/* ---- signatures ---------------------------------------------------------- */

void update_input(GLFWwindow *window, Camera *camera);
void generate_standard_meshes();
void bind_shader_uniforms();
void update_world();
void draw_skybox();
void draw_world();
void draw_hud();
void draw_everything();

int main(void)
{
    glfwSetErrorCallback(error_callback);
    /*temp*/ render.size = (v2i32){1080, 820};

    if (0
            || init_glfw() != 0
            || init_window(&render) != 0
            || init_glew() != 0
       ) return -1;

    game_start_time = glfwGetTime();

    glfwSetWindowSizeLimits(render.window, 100, 70, 1920, 1080);
    /*temp*/ glfwSetWindowPos(render.window, 1920 - render.size.x, 0);

    /* ---- set mouse input ------------------------------------------------- */
    glfwSetInputMode(render.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(render.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        LOGINFO("%s\n", "Raw Mouse Motion Enabled");
    }
    else LOGERROR("%s\n", "Raw Mouse Motion Not Supported");
    glfwGetCursorPos(render.window, &render.mouse_position.x, &render.mouse_position.y);

    /* ---- set callbacks --------------------------------------------------- */
    glfwSetFramebufferSizeCallback(render.window, gl_frame_buffer_size_callback);
    gl_frame_buffer_size_callback(render.window, render.size.x, render.size.y);

    glfwSetCursorPosCallback(render.window, gl_cursor_pos_callback);
    gl_cursor_pos_callback(render.window, render.mouse_position.x, render.mouse_position.y);

    glfwSetKeyCallback(render.window, gl_key_callback);
    gl_key_callback(render.window, 0, 0, 0, 0);

    /* ---- graphics -------------------------------------------------------- */
    if (0
            || init_shader_program(&shader_skybox) != 0
            || init_shader_program(&shader_default) != 0
            || init_shader_program(&shader_gizmo) != 0
            || init_shader_program(&shader_fbo) != 0
            || init_fbo(&render, &fbo_skybox, &color_buf_skybox, &rbo_skybox, &mesh_fbo) != 0
            || init_fbo(&render, &fbo_world, &color_buf_world, &rbo_world, &mesh_fbo) != 0
            || init_fbo(&render, &fbo_hud, &color_buf_hud, &rbo_hud, &mesh_fbo) != 0
       ) goto cleanup;

    glfwSwapInterval(1); /* vsync */
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    camera =
        (Camera){
            .pos = (v3f32){-0.5f, -0.5f, 0.5f},
            .rot = (v3f32){0.0f, 0.0f, 315.0f},
            .fov = 70.0f,
            .ratio = (f32)render.size.x / (f32)render.size.y,
            .far = GL_CLIP_DISTANCE0,
            .near = 0.05f,
        };

    bind_shader_uniforms();
    generate_standard_meshes();

    /* ---- main loop ------------------------------------------------------- */
    while (!glfwWindowShouldClose(render.window))
    {
        get_mouse_position(&render, &render.mouse_position);
        get_mouse_movement(render.mouse_last, &render.mouse_delta);

        LOGDEBUG("%s\n", "-------------------------------------]");
        LOGDEBUG("        mouse[%7.2lf %7.2lf        ]\n", render.mouse_position.x, render.mouse_position.y);
        LOGDEBUG("        delta[%7.2lf %7.2lf        ]\n", render.mouse_delta.x, render.mouse_delta.y);
        LOGDEBUG("   camera xyz[%7.2f %7.2f %7.2f]\n", camera.pos.x, camera.pos.y, camera.pos.z);
        LOGDEBUG("camera yawpch[        %7.2f %7.2f]\n", camera.rot.y, camera.rot.z);
        LOGDEBUG("        ratio[%7.2f                ]\n\n", camera.ratio);

        update_world();
        draw_everything();

        glfwSwapBuffers(render.window);
        glfwPollEvents();
        update_input(render.window, &camera);
    }

cleanup: /* ----------------------------------------------------------------- */
    delete_mesh(&mesh_skybox);
    delete_mesh(&mesh_cube_of_happiness);
    delete_mesh(&mesh_gizmo);
    glDeleteFramebuffers(1, &fbo_skybox);
    glDeleteFramebuffers(1, &fbo_world);
    glDeleteFramebuffers(1, &fbo_hud);
    glDeleteProgram(shader_default.id);
    glDeleteProgram(shader_skybox.id);
    glDeleteProgram(shader_gizmo.id);
    glfwDestroyWindow(render.window);
    glfwTerminate();
    return 0;
}

static void gl_frame_buffer_size_callback(GLFWwindow* window, int width, int height)
{
    render.size = (v2i32){width, height};
    camera.ratio = (f32)width / (f32)height;
    glViewport(0, 0, width, height);
}

static void gl_cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    render.mouse_delta = (v2f64){xpos - render.mouse_position.x, ypos - render.mouse_position.y};
    render.mouse_position = (v2f64){xpos, ypos};

    camera.rot.y += render.mouse_delta.y;
    camera.rot.z += render.mouse_delta.x;
}

static void gl_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_PAUSE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT))
        init_shader_program(&shader_default);

    if (key == GLFW_KEY_C && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

f64 movement_speed = 0.08f;
void update_input(GLFWwindow *window, Camera *camera)
{
    /* ---- movement -------------------------------------------------------- */
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera->pos.x += (movement_speed * camera->sin_yaw);
        camera->pos.y += (movement_speed * camera->cos_yaw);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera->pos.x -= (movement_speed * camera->sin_yaw);
        camera->pos.y -= (movement_speed * camera->cos_yaw);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera->pos.x -= (movement_speed * camera->cos_yaw);
        camera->pos.y += (movement_speed * camera->sin_yaw);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera->pos.x += (movement_speed * camera->cos_yaw);
        camera->pos.y -= (movement_speed * camera->sin_yaw);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera->pos.z += movement_speed;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        camera->pos.z -= movement_speed;
    }
}

void generate_standard_meshes()
{
    GLfloat vbo_data_skybox[] =
    {
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    GLuint ebo_data_skybox[] =
    {
        0, 1, 5, 5, 4, 0,
        1, 3, 7, 7, 5, 1,
        3, 2, 6, 6, 7, 3,
        2, 0, 4, 4, 6, 2,
        4, 5, 7, 7, 6, 4,
        0, 2, 3, 3, 1, 0,
    };

    GLfloat vbo_data_coh[] =
    {
        1.0f, 1.0f, 1.0f,
        2.0f, 1.0f, 1.0f,
        1.0f, 2.0f, 1.0f,
        2.0f, 2.0f, 1.0f,
        1.0f, 1.0f, 2.0f,
        2.0f, 1.0f, 2.0f,
        1.0f, 2.0f, 2.0f,
        2.0f, 2.0f, 2.0f,
    };

    GLuint ebo_data_coh[] =
    {
        0, 4, 5, 5, 1, 0,
        1, 5, 7, 7, 3, 1,
        3, 7, 6, 6, 2, 3,
        2, 6, 4, 4, 0, 2,
        4, 6, 7, 7, 5, 4,
        0, 1, 3, 3, 2, 0,
    };
    
    GLfloat THIC = 0.05f;
    GLfloat vbo_data_gizmo[] =
    {
        0.0f, 0.0f, 0.0f,
        THIC, THIC, 0.0f,
        THIC, 0.0f, THIC,
        0.0f, THIC, THIC,
        1.0f, 0.0f, 0.0f,
        1.0f, THIC, 0.0f,
        1.0f, 0.0f, THIC,
        0.0f, 1.0f, 0.0f,
        THIC, 1.0f, 0.0f,
        0.0f, 1.0f, THIC,
        0.0f, 0.0f, 1.0f,
        THIC, 0.0f, 1.0f,
        0.0f, THIC, 1.0f,
        THIC, THIC, THIC,
        1.0f, THIC, THIC,
        THIC, 1.0f, THIC,
        THIC, THIC, 1.0f,
    };

    GLuint ebo_data_gizmo[] =
    {
        0, 2, 6, 6, 4, 0,
        0, 4, 5, 5, 1, 0,
        0, 1, 8, 8, 7, 0,
        0, 7, 9, 9, 3, 0,
        0, 3, 12, 12, 10, 0,
        0, 10, 11, 11, 2, 0,
        2, 13, 14, 14, 6, 2,
        3, 9, 15, 15, 13, 3,
        2, 11, 16, 16, 13, 2,
        13, 16, 12, 12, 3, 13,
        4, 6, 14, 14, 5, 4,
        7, 8, 15, 15, 9, 7,
        10, 12, 16, 16, 11, 10,
        1, 5, 14, 14, 13, 1,
        1, 13, 15, 15, 8, 1,
    };

    if (generate_mesh(&mesh_skybox, GL_STATIC_DRAW, VBO_LEN_SKYBOX, EBO_LEN_SKYBOX,
            vbo_data_skybox, ebo_data_skybox) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Skybox' Mesh Generated");

    if (generate_mesh(&mesh_cube_of_happiness, GL_STATIC_DRAW, VBO_LEN_COH, EBO_LEN_COH,
            vbo_data_coh, ebo_data_coh) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Cube of Happiness' Mesh Generated");

    if (generate_mesh(&mesh_gizmo, GL_STATIC_DRAW, VBO_LEN_GIZMO, EBO_LEN_GIZMO,
            vbo_data_gizmo, ebo_data_gizmo) != 0)
        goto cleanup;
    LOGINFO("%s\n", "'Gizmo' Mesh Generated");

    return;

cleanup:
    LOGERROR("%s\n", "Mesh Generation Failed");
}

void bind_shader_uniforms()
{
    uniform_skybox.camera_position = glGetUniformLocation(shader_skybox.id, "camera_position");
    uniform_skybox.mat_rotation = glGetUniformLocation(shader_skybox.id, "mat_rotation");
    uniform_skybox.mat_orientation = glGetUniformLocation(shader_skybox.id, "mat_orientation");
    uniform_skybox.mat_projection = glGetUniformLocation(shader_skybox.id, "mat_projection");
    uniform_skybox.sun_rotation = glGetUniformLocation(shader_skybox.id, "sun_rotation");
    uniform_skybox.sky_color = glGetUniformLocation(shader_skybox.id, "sky_color");

    uniform_default.mat_perspective = glGetUniformLocation(shader_default.id, "mat_perspective");
    uniform_default.camera_position = glGetUniformLocation(shader_default.id, "camera_position");
    uniform_default.sun_rotation = glGetUniformLocation(shader_default.id, "sun_rotation");
    uniform_default.sky_color = glGetUniformLocation(shader_default.id, "sky_color");

    uniform_gizmo.render_ratio = glGetUniformLocation(shader_gizmo.id, "ratio");
    uniform_gizmo.mat_target = glGetUniformLocation(shader_gizmo.id, "mat_translation");
    uniform_gizmo.mat_rotation = glGetUniformLocation(shader_gizmo.id, "mat_rotation");
    uniform_gizmo.mat_orientation = glGetUniformLocation(shader_gizmo.id, "mat_orientation");
    uniform_gizmo.mat_projection = glGetUniformLocation(shader_gizmo.id, "mat_projection");
}

void update_world()
{
    game_tick = (floor((glfwGetTime() - game_start_time) * 1500)) - (SETTING_DAY_TICKS_MAX * game_days);
    if (game_tick >= SETTING_DAY_TICKS_MAX)
        ++game_days;

    update_camera_movement(&camera);
    update_camera_perspective(&camera, &projection);
}

void draw_skybox()
{
    skybox_data.time = (f32)game_tick / (f32)SETTING_DAY_TICKS_MAX;
    skybox_data.sun_rotation = (v3f32){-cos((skybox_data.time * 360.0f) * DEG2RAD) + 1.0f, -cos((skybox_data.time * 360.0f) * DEG2RAD) + 1.0f, 12.0f};

    f32 intensity =     0.0039f;
    f32 mid_day =       fabsf(sinf(1.5f * sinf(skybox_data.time * PI)));
    f32 pre_burn =      fabsf(sinf(powf(sinf((skybox_data.time + 0.33f) * PI * 1.2f), 16.0f)));
    f32 burn =          fabsf(sinf(1.5f * powf(sinf((skybox_data.time + 0.124f) * PI * 1.6f), 32.0f)));
    f32 burn_boost =    fabsf(powf(sinf((skybox_data.time + 0.212f) * PI * 1.4f), 64.0f));
    f32 mid_night =     fabsf(sinf(powf(2.0f * cosf(skybox_data.time * PI), 3.0f)));

    skybox_data.color =
    (v3f32){
        (mid_day * 171.0f) + (burn * 85.0f) + (mid_night * 1.0f) + (pre_burn * 13.0f) + (burn_boost * 76.0f),
        (mid_day * 229.0f) + (burn * 42.0f) + (mid_night * 4.0f) + (pre_burn * 7.0f) + (burn_boost * 34.0f),
        (mid_day * 255.0f) + (burn * 19.0f) + (mid_night * 14.0f) + (pre_burn * 20.0f),
    };

    skybox_data.color =
        (v3f32){
            clamp_f32(skybox_data.color.x * intensity, 0.0f, 1.0f),
            clamp_f32(skybox_data.color.y * intensity, 0.0f, 1.0f),
            clamp_f32(skybox_data.color.z * intensity, 0.0f, 1.0f),
        };

    glUseProgram(shader_skybox.id);
    glUniform3fv(uniform_skybox.camera_position, 1, (GLfloat*)&camera.pos);
    glUniformMatrix4fv(uniform_skybox.mat_rotation, 1, GL_FALSE, (GLfloat*)&projection.rotation);
    glUniformMatrix4fv(uniform_skybox.mat_orientation, 1, GL_FALSE, (GLfloat*)&projection.orientation);
    glUniformMatrix4fv(uniform_skybox.mat_projection, 1, GL_FALSE, (GLfloat*)&projection.projection);
    glUniform3fv(uniform_skybox.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform_skybox.sky_color, 1, (GLfloat*)&skybox_data.color);
    draw_mesh(&mesh_skybox);

    LOGDEBUG("        ticks[%5d                  ]\n", game_tick);
    LOGDEBUG("  skybox time[%7.2f                ]\n", skybox_data.time);
    LOGDEBUG(" sun rotation[%7.2f %7.2f %7.2f]\n", skybox_data.sun_rotation.x, skybox_data.sun_rotation.y, skybox_data.sun_rotation.z);
    LOGDEBUG("        color[%7.2f %7.2f %7.2f]\n\n\n", skybox_data.color.x, skybox_data.color.y, skybox_data.color.z);
}

void draw_world()
{
    glUseProgram(shader_default.id);
    glUniformMatrix4fv(uniform_default.mat_perspective, 1, GL_FALSE, (GLfloat*)&projection.perspective);
    glUniform3fv(uniform_default.camera_position, 1, (GLfloat*)&camera.pos);
    glUniform3fv(uniform_default.sun_rotation, 1, (GLfloat*)&skybox_data.sun_rotation);
    glUniform3fv(uniform_default.sky_color, 1, (GLfloat*)&skybox_data.color);
    draw_mesh(&mesh_cube_of_happiness);
}

void draw_hud()
{
    glUseProgram(shader_gizmo.id);
    glUniform1f(uniform_gizmo.render_ratio, (GLfloat)camera.ratio);
    glUniformMatrix4fv(uniform_gizmo.mat_target, 1, GL_FALSE, (GLfloat*)&projection.target);
    glUniformMatrix4fv(uniform_gizmo.mat_rotation, 1, GL_FALSE, (GLfloat*)&projection.rotation);
    glUniformMatrix4fv(uniform_gizmo.mat_orientation, 1, GL_FALSE, (GLfloat*)&projection.orientation);
    glUniformMatrix4fv(uniform_gizmo.mat_projection, 1, GL_FALSE, (GLfloat*)&projection.projection);
    draw_mesh(&mesh_gizmo);
}

void draw_everything()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_skybox);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    draw_skybox();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_world);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_world();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_hud);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw_hud();

    /* ---- draw everything ------------------------------------------------- */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(shader_fbo.id);
    glBindVertexArray(mesh_fbo.vao);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, color_buf_skybox);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, color_buf_world);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, color_buf_hud);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
