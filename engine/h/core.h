#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include "common.h"
#include "limits.h"
#include "types.h"

#include <engine/include/glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <engine/include/glfw3.h>

#include <engine/include/stb_image.h>
#include <engine/include/stb_image_write.h>

typedef struct Render
{
    GLFWwindow *window;
    char title[128];
    v2i32 size;

    /*! @brief conversion from world-space to screen-space.
     *
     *  @remark read-only, updated internally in @ref engine_update().
     */
    v2f32 ndc_scale;

    GLFWimage icon;
    v2f64 mouse_pos;
    v2f64 mouse_delta;
    u64 time;
    u64 time_delta;

    /*! @brief for reading screen pixels back to RAM (e.g. screenshots).
     */
    u8 *screen_buf;
} Render;

typedef struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint vbo_len;
    GLuint ebo_len;
    GLfloat *vbo_data;
    GLfloat *ebo_data;
} Mesh;

typedef struct FBO
{
    GLuint fbo;
    GLuint color_buf;
    GLuint rbo;
} FBO;

typedef struct Texture
{
    v2i32 size;
    u64 data_len;
    GLuint id;              /* used by @ref glGenTextures() */

    /*! @brief used by `OpenGL` extension @ref GL_ARB_bindless_texture.
     */
    u64 handle;

    GLint format;           /* used by @ref glTexImage2D() */
    GLint format_internal;  /* used by @ref glTexImage2D() */
    GLint filter;           /* used by @ref glTexParameteri() */

    /*! @brief number of color channels, used by @ref stbi_load().
     */
    int channels;

    b8 grayscale;
    u8 *buf;
} Texture;

typedef struct Camera
{
    v3f64 pos;
    f64 roll, pitch, yaw;
    f64 sin_roll, sin_pitch, sin_yaw;
    f64 cos_roll, cos_pitch, cos_yaw;
    f32 fovy;
    f32 fovy_smooth;
    f32 ratio;
    f32 far;
    f32 near;
    f32 zoom;
} Camera;

typedef struct Projection
{
    m4f32 target;
    m4f32 translation;
    m4f32 rotation;
    m4f32 orientation;
    m4f32 view;
    m4f32 projection;
    m4f32 perspective;
} Projection;

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief default render.
 *
 *  @remark declared internally.
 */
FSLAPI extern Render *render;

/*! @brief default textures.
 *
 *  @remark declared internally.
 */
FSLAPI extern Texture engine_texture[ENGINE_TEXTURE_COUNT];

FSLAPI extern Mesh engine_mesh_unit;

/*! @brief initialize engine stuff.
 *
 *  - set 'GLFW' error callback.
 *  - call @ref change_dir() to change working directory to the running process'.
 *  - call @ref logger_init(), @ref glfw_init(), @ref window_init() and @ref glad_init().
 *  - initialize default shaders if requested.
 *
 *  @param argc number of arguments in `argv` if `argv` provided.
 *  @param argv used for logger log level if args provided.
 *
 *  @param _log_dir directory to write log files into for the lifetime of the process,
 *  if `NULL`, logs won't be written to disk.
 *
 *  @param _render `Render` to use for engine,
 *  if `NULL`, @ref render is defined as default, declared and used internally.
 *
 *  @param flags enum: @ref EngineFlag.
 *
 *  @param title = window/application title, if `NULL`, default title is used
 *  (@ref window_init() parameter).
 *
 *  @param size_x window width, if 0, @ref RENDER_WIDTH_DEFAULT is used
 *  (@ref window_init() parameter).
 *
 *  @param size_y window height, if 0, @ref RENDER_HEIGHT_DEFAULT is used
 *  (@ref window_init() parameter).
 *
 *  @remark release_build can be overridden with these args in `argv`:
 *      logfatal:   only output fatal logs (least verbose).
 *      logerror:   only output <= error logs.
 *      logwarn:    only output <= warning logs.
 *      loginfo:    only output <= info logs (default).
 *      logdebug:   only output <= debug logs.
 *      logtrace:   only output <= trace logs (most verbose).
 *
 *  @remark on error, 'engine_close()' must be called to free allocated resources.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 engine_init(int argc, char **argv, const str *_log_dir, const str *title,
        i32 size_x, i32 size_y, Render *_render, u64 flags);

/*! @brief engine main loop check.
 *
 *  - update @ref Render.time and @ref Render.time_delta of the currently bound `Render`.
 *
 *  @return `TRUE` unless @ref glfwWindowShouldClose() returns `FALSE` or engine inactive.
 */
FSLAPI b8 engine_running(void);

/*! @brief update render settings (like render size).
 *
 *  - update @ref Render.ndc_scale of the currently bound `Render`.
 *
 *  @remark usually passed into @ref glfwSetFramebufferSizeCallback().
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 engine_update_render_settings(i32 size_x, i32 size_y);

/*! @brief free engine resources.
 *
 *  free logger, destroy window (if not `NULL`) and terminate 'GLFW'.
 */
FSLAPI void engine_close(void);

/*! @brief get engine-specific string no longer than @ref NAME_MAX bytes.
 *
 *  @param dst pointer to buffer to store string.
 *  
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 fsl_get_string(str *dst, enum FossilStringIndex type);

/*! @brief initialize 'GLFW'.
 *
 *  @param multisample = turn on multisampling.
 *
 *  @remark called automatically from @ref engine_init().
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 glfw_init(b8 multisample);

/*! @brief initialize a new 'GLFW' window for the currently bound `Render`.
 *
 *  @param title = window/application title, if `NULL`, default title is used.
 *  @param size_x window width, if 0, @ref RENDER_WIDTH_DEFAULT is used.
 *  @param size_y window height, if 0, @ref RENDER_HEIGHT_DEFAULT is used.
 *
 *  @remark called automatically from @ref engine_init().
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 window_init(const str *title, i32 size_x, i32 size_y);

/*! @brief initialize 'OpenGL' function loader 'GLAD'.
 *
 *  @remark called automatically from @ref engine_init().
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 glad_init(void);

/*! @brief switch engine's current bound `Render` to `_render`.
 *
 *  @remark only if you know what you're doing.
 */
FSLAPI u32 change_render(Render *_render);

/*! @remark send screenshot request to then be processed by @ref process_screenshot_request().
 *
 *  can be called from anywhere, then @ref process_screenshot_request() can be called
 *  to take the screenshot at the end of the render loop.
 */
FSLAPI void request_screenshot(void);

/*! @remark take screenshot requested by @ref request_screenshot() and save into dir at `dir_screenshots`.
 *  
 *  @param special_text string appended to file name before extension.
 *
 *  @remark if directory not found, screenshot is still saved at @def Render.screen_buf
 *  of the currently bound `Render`.
 *
 *  @remark the internals for taking a screenshot are separated into their own function
 *  internally so to reduce function call overhead since this function is meant to be called
 *  in a render loop and the code for taking a screenshot allocates a sizable block of memory when called.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
FSLAPI u32 process_screenshot_request(const str *dir_screenshots, const str *special_text);

/*! @brief set a `vec3` attribute array for a `vao`.
 */
FSLAPI void attrib_vec3(void);

/*! @brief set a `vec3` and a `vec2` attribute arrays for a `vao`.
 */
FSLAPI void attrib_vec3_vec2(void);

/*! @brief set a `vec3` and a `vec3` attribute arrays for a `vao`.
 */
FSLAPI void attrib_vec3_vec3(void);

/*! @brief set a `vec3` and a `vec4` attribute arrays for a `vao`.
 */
FSLAPI void attrib_vec3_vec4(void);

/*! @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 fbo_init(FBO *fbo, Mesh *mesh_fbo, b8 multisample, u32 samples);

/*! @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 fbo_realloc(FBO *fbo, b8 multisample, u32 samples);

FSLAPI void fbo_free(FBO *fbo);

/*! @brief load image data from disk into `texture->buf` and set texture info.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 texture_init(Texture *texture, v2i32 size, const GLint format_internal, const GLint format,
        GLint filter, int channels, b8 grayscale, const str *file_name);

/*! @brief generate texture for `OpenGL` from image loaded by 'texture_init()'.
 *
 *  @param bindless use `OpenGL` extension `GL_ARB_bindless_texture`
 *  (handle is in `texture->handle`).
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 texture_generate(Texture *texture, b8 bindless);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief generate texture for `OpenGL` and upload to `GPU` memory.
 *
 *  @param id where to store texture ID.
 *  @param buf texture data to upload to `gpu` memory.
 *
 *  @remark called automatically from @ref texture_generate() if texture data is
 *  already loaded into a texture by calling @ref texture_init().
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
u32 _texture_generate(GLuint *id, const GLint format_internal,  const GLint format,
        GLint filter, u32 width, u32 height, void *buf, b8 grayscale);

FSLAPI void texture_free(Texture *texture);

/*! @param attrib pointer to a function to set attribute arrays for `mesh->vao`
 *  (e.g. &attrib_vec3, set a single vec3 attribute array).
 *
 *  @param usage `GL_<x>_DRAW`.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 mesh_generate(Mesh *mesh, void (*attrib)(), GLenum usage,
        GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data);

FSLAPI void mesh_free(Mesh *mesh);

/*! @brief update `sine` and `cosine` of camera roll, pitch and yaw.
 *
 *  @param roll enable/disable roll rotation.
 *
 *  @remark rotation limits:
 *      roll:  [  0, 360].
 *      pitch: [-90,  90].
 *      yaw:   [  0, 360].
 */
FSLAPI void update_camera_movement(Camera *camera, b8 roll);

/*! @brief make perspective projection matrices from camera parameters.
 *
 *  - setup camera matrices for Z-up, right-handed coordinates and vertical fov (fovy).
 *
 *  @param roll enable/disable roll rotation.
 */
FSLAPI void update_projection_perspective(Camera camera, Projection *projection, b8 roll);

/*! @brief get camera look-at angles from camera position and target position.
 *  
 *  assign vertical angle to `pitch` and horizontal angle to `yaw`.
 */
FSLAPI void get_camera_lookat_angles(v3f64 camera_pos, v3f64 target, f64 *pitch, f64 *yaw);

#endif /* ENGINE_CORE_H */
