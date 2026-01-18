#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#define ENGINE_VERSION_STABLE   "-stable"
#define ENGINE_VERSION_BETA     "-beta"
#define ENGINE_VERSION_ALPHA    "-alpha"
#define ENGINE_VERSION_DEV      "-dev"

#define ENGINE_AUTHOR           "Lily Awertnex"
#define ENGINE_NAME             "Fossil Engine"
#define ENGINE_VERSION          "0.1.3"ENGINE_VERSION_STABLE
#define ENGINE_TITLE            ENGINE_NAME": "ENGINE_VERSION

#include "common.h"
#include "limits.h"
#include "platform.h"
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
     *  @remark read-only, updated internally in 'engine_update()'.
     */
    v2f32 ndc_scale;

    GLFWimage icon;
    v2f64 mouse_pos;
    v2f64 mouse_delta;
    u64 time;
    u64 time_delta;

    /*! @brief for reading screen pixels back to RAM.
     */
    u8 *screen_buf;
} Render;

typedef struct Mesh
{
    GLuint vao;
    GLuint vbo; GLuint ebo;
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
    GLuint id;              /* used by 'glGenTextures() */

    /*! @brief used by opengl extension 'GL_ARB_bindless_texture'.
     */
    u64 handle;

    GLint format;           /* used by 'glTexImage2D()' */
    GLint format_internal;  /* used by 'glTexImage2D()' */
    GLint filter;           /* used by 'glTexParameteri()' */

    /*! @brief number of color channels, used by 'stbi_load()'.
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
extern Render *render;

/*! @brief default textures.
 *
 *  @remark declared internally.
 */
extern Texture engine_texture[ENGINE_TEXTURE_COUNT];

extern Mesh engine_mesh_unit;

/*! @brief initialize engine stuff.
 *
 *  - set GLFW error callback.
 *  - call 'change_dir()' to change working directory to the running applications'.
 *  - call 'logger_init()', 'glfw_init()', 'window_init()' and 'glad_init()'.
 *  - initialize default shaders if requested.
 *
 *  @param argc, argv = used for logger log level if args provided.
 *  @param _log_dir = directory to write log files into for the lifetime of the process,
 *  if NULL, logs won't be written to disk.
 *
 *  @param _render = render to use for engine,
 *  if NULL, the global variable 'render' is defined as default and declared internally.
 *
 *  @param flags = enum 'common.h/EngineFlag'.
 *
 *  @param title = window/application title, if NULL, default title is used
 *  ('window_init()' parameter).
 *
 *  @param size_x, size_y = window size, if either is 0, default size is used
 *  ('window_init()' parameter).
 *
 *  @remark release_build can be overridden with these args in 'argv':
 *      LOGLEVEL FATAL = log only fatal errors.
 *      LOGLEVEL ERROR = log errors and above.
 *      LOGLEVEL WARNING = log warnings and above.
 *      LOGLEVEL INFO = log info and above.
 *      LOGLEVEL DEBUG = log debug and above.
 *      LOGLEVEL TRACE = log everything, default.
 *
 *  @remark on error, 'engine_close()' must be called to free allocated resources.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 engine_init(int argc, char **argv, const str *_log_dir, const str *title,
        i32 size_x, i32 size_y, Render *_render, u64 flags);

/*! @brief free engine resources.
 *
 *  free logger, destroy window (if not NULL) and terminate glfw.
 */
void engine_close(void);

/*! @param multisample = turn on multisampling.
 *
 *  @remark called automatically from 'engine_init()'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 glfw_init(b8 multisample);

/*! @param title = window/application title, if NULL, default title is used.
 *  @param size_x, size_y = window size, if either is 0, default size is used.
 *
 *  @remark called automatically from 'engine_init()'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 window_init(const str *title, i32 size_x, i32 size_y);

/*! @remark called automatically from 'engine_init()'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 glad_init(void);

/*! @brief engine main loop check.
 *
 *  @return TRUE unless 'glfw/glfwWindowShouldClose()' returns FALSE or engine inactive.
 */
b8 engine_update(void);

/*! @brief update render settings like frame size, used mainly in
 *  'glfw/glfwSetFramebufferSizeCallback()'.
 *
 *  - update uniform 'ndc_scale' of engine shaders.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 engine_update_render_settings(i32 size_x, i32 size_y);

/*! @brief switch engine's current bound render to '_render'.
 */
u32 change_render(Render *_render);

/*! @remark send screenshot request.
 *
 *  can be called from anywhere, then 'process_screenshot_request()' can be called
 *  after the render loop has finished to take the screenshot.
 */
void request_screenshot(void);

/*! @remark take screenshot if requested by 'request_screenshot()' and save into dir at '_screenshot_dir'.
 *  
 *  the code for taking a screenshot is separated into its own function to reduce
 *  function call overhead since this function is meant to be called in a render loop
 *  and the code allocates a sizable block of memory every call.
 *
 *  @param special_text = string appended to file name before extension.
 *
 *  @remark if directory not found, screenshot is still saved at 'render->screen_buf'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 process_screenshot_request(const str *_screenshot_dir, const str *special_text);

/*! @remark take screenshot and save into dir at '_screenshot_dir'.
 *  
 *  save pixel data as 'RGB' into 'render->screen_buf'.
 *
 *  @param special_text = string appended to file name before extension.
 *
 *  @remark if directory not found, screenshot is still saved at 'render->screen_buf'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 take_screenshot(const str *_screenshot_dir, const str *special_text);

/*! @brief set a vec3 attribute array for a vao.
 */
void attrib_vec3(void);

/*! @brief set a vec3 and a vec2 attribute arrays for a vao.
 */
void attrib_vec3_vec2(void);

/*! @brief set a vec3 and a vec3 attribute arrays for a vao.
 */
void attrib_vec3_vec3(void);

/*! @brief set a vec3 and a vec4 attribute arrays for a vao.
 */
void attrib_vec3_vec4(void);

/*! @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 fbo_init(FBO *fbo, Mesh *mesh_fbo, b8 multisample, u32 samples);

/*! @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 fbo_realloc(FBO *fbo, b8 multisample, u32 samples);

void fbo_free(FBO *fbo);

/*! @brief load image data from disk into 'texture->buf' and set texture info.
 *
 *  @return non-zero on failure and engine_err is set accordingly.
 */
u32 texture_init(Texture *texture, v2i32 size, const GLint format_internal, const GLint format,
        GLint filter, int channels, b8 grayscale, const str *file_name);

/*! @brief generate texture for opengl from image loaded by 'texture_init()'.
 *
 *  @param bindless = use opengl extension 'GL_ARB_bindless_texture'
 *  (handle is in 'texture.handle'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 texture_generate(Texture *texture, b8 bindless);

/*! @brief generate texture for opengl from 'buf'.
 *
 *  @brief automatically called from 'texture_generate()' if texture data is already
 *  loaded into a texture by calling 'texture_init()'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 _texture_generate(GLuint *id, const GLint format_internal,  const GLint format,
        GLint filter, u32 width, u32 height, void *buf, b8 grayscale);

void texture_free(Texture *texture);

/*! @param attrib = pointer to a function to set attribute arrays for 'mesh->vao'
 *  (e.g. &attrib_vec3, set a single vec3 attribute array),
 *  @param usage = 'GL_<x>_DRAW'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 mesh_generate(Mesh *mesh, void (*attrib)(), GLenum usage,
        GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data);

void mesh_free(Mesh *mesh);

/*! @brief apply sin() and cos() to camera's roll, pitch and yaw from camera rotation.
 *
 *  @param roll = if TRUE, roll rotation will be applied from 'camera.rot.x',
 *  and set to 0 if FALSE.
 *
 *  @remark rotation limits:
 *      roll: [0, 360].
 *      pitch: [-90, 90].
 *      yaw: [0, 360].
 */
void update_camera_movement(Camera *camera, b8 roll);

/*! @brief make perspective projection matrices from camera parameters.
 *
 *  setup camera matrices for Z-up, right-handed coordinates and vertical fov (fovy).
 *  setup roll, pitch and yaw rotations from 'camera.rot'.
 *
 *  @param roll = if TRUE, roll rotation will be applied from 'camera.rot.x'.
 */
void update_projection_perspective(Camera camera, Projection *projection, b8 roll);

/*! @brief get camera look-at angles from camera position and target position.
 *  
 *  assign vertical angle to 'pitch' and horizontal angle to 'yaw'.
 */
void get_camera_lookat_angles(v3f64 camera_pos, v3f64 target, f64 *pitch, f64 *yaw);

#endif /* ENGINE_CORE_H */
