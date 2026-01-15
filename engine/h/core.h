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

#include <engine/include/glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <engine/include/glfw3.h>
#include <engine/include/stb_truetype.h>
#include <engine/include/stb_image.h>

#include "common.h"
#include "limits.h"
#include "platform.h"
#include "types.h"

enum EngineFlag
{
    FLAG_ENGINE_GLFW_INITIALIZED,
}; /* EngineFlag */

typedef struct Render
{
    GLFWwindow *window;
    char title[128];
    v2i32 size;
    GLFWimage icon;
    v2f64 mouse_pos;
    v2f64 mouse_delta;
    u64 time;
    u64 time_delta;
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

typedef struct Shader
{
    str *file_name;
    GLuint id;          /* used by 'glCreateShader()' */
    GLuint type;        /* 'GL_<x>_SHADER' */
    GLchar *source;     /* shader file source code */
    GLint loaded;       /* used by 'glGetShaderiv()' */
} Shader;

typedef struct ShaderProgram
{
    str *name;          /* for stress-free debugging */
    GLuint id;          /* used by 'lCreateProgram()' */
    GLint loaded;       /* used by 'glGetProgramiv()' */
    Shader vertex;
    Shader geometry;
    Shader fragment;
} ShaderProgram;

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

typedef struct Glyph
{
    v2i32 scale;
    v2i32 bearing;
    i32 advance;
    i32 x0, y0, x1, y1;
    v2f32 texture_sample;
    b8 loaded;
} Glyph;

typedef struct Glyphf
{
    v2f32 scale;
    v2f32 bearing;
    f32 advance;
    f32 x0, y0, x1, y1;
    v2f32 texture_sample;
    b8 loaded;
} Glyphf;

typedef struct Font
{
    /*! @brief font file name,
     *
     *  initialized in 'font_init()'.
     */
    str path[PATH_MAX];

    u32 resolution;         /* glyph bitmap diameter in bytes */
    f32 char_size;          /* for font atlas sampling */

    /*! @brief glyphs highest points' deviation from baseline.
     */
    i32 ascent;

    /*! @brief glyphs lowest points' deviation from baseline.
     */
    i32 descent;

    i32 line_gap;
    i32 line_height;
    f32 size;               /* global font size for text uniformity */
    v2i32 scale;            /* biggest glyph bounding box in pixels */

    /*! @brief used by 'stbtt_InitFont()'.
     */
    stbtt_fontinfo info;

    /*! @brief font file contents.
     *
     *  used by 'stbtt_InitFont()'.
     */
    u8 *buf;

    u64 buf_len;            /* 'buf' size in bytes */
    u8 *bitmap;             /* memory block for all font glyph bitmaps */

    GLuint id;              /* used by opengl's glGenTextures() */
    Glyph glyph[GLYPH_MAX];
} Font;

enum TextAlignment
{
    TEXT_ALIGN_LEFT = 0,
    TEXT_ALIGN_CENTER = 1,
    TEXT_ALIGN_RIGHT = 2,
    TEXT_ALIGN_TOP = 0,
    TEXT_ALIGN_BOTTOM = 2,
}; /* TextAlignment */

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief default render.
 *
 *  @remark declared internally.
 */
extern Render *render;

/*! @brief default shaders.
 *
 *  @remark declared internally.
 */
extern ShaderProgram engine_shader[ENGINE_SHADER_COUNT];

/*! @brief default textures.
 *
 *  @remark declared internally.
 */
extern Texture engine_texture[ENGINE_TEXTURE_COUNT];

extern Mesh engine_mesh_unit;
extern Font engine_font[ENGINE_FONT_COUNT];

/*! @brief initialize engine stuff.
 *
 *  set GLFW error callback.
 *  initialize logger.
 *  call 'change_dir()' to change working directory to the running applications'.
 *  calls 'glfw_init()', 'window_init()' and 'glad_init()'.
 *
 *  @param argc, argv = used for logger log level if args provided.
 *  @param _log_dir = directory to write log files into for the lifetime of the process,
 *  if NULL, logs won't be written to disk.
 *
 *  @param _render = render to use for engine,
 *  if NULL, the global variable 'render' is defined as default and declared internally.
 *
 *  @param shaders = initialize default shaders (like 'text' and 'ui').
 *  @param release_build = if TRUE, TRACE and DEBUG logs will be disabled.
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
        i32 size_x, i32 size_y, Render *_render, b8 shaders, b8 multisample, b8 release_build);

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

/*! @remark called automatically from 'engine_init()'.
 *
 *  @param title = window/application title, if NULL, default title is used.
 *  @param size_x, size_y = window size, if either is 0, default size is used.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 window_init(const str *title, i32 size_x, i32 size_y);

/*! @remark called automatically from 'engine_init()'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 glad_init(void);

/*! @remark switch engine's current bound render to '_render'.
 */
u32 change_render(Render *_render);

/*! @brief initialize single shader.
 *
 *  calls 'shader_pre_process()' on 'shader->file_name' before compiling
 *  shader, then compiles shader.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 shader_init(const str *shaders_dir, Shader *shader);

/*! @brief initialize shader program.
 *
 *  calls 'shader_init()' on all shaders in 'program' if 'shader->type' is set.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 shader_program_init(const str *shaders_dir, ShaderProgram *program);

void shader_program_free(ShaderProgram *program);

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
 *  @param bindless = use opengl extension 'GL_ARB_bindless_texture'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 texture_generate(Texture *texture, b8 bindless);

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

/*! @brief load font from file at font_path.
 *
 *  1. allocate memory for 'font.buf' and load file contents into it in binary format.
 *  2. allocate memory for 'font.bitmap' and render glyphs onto it.
 *  3. generate square texture of diameter (size * 16) and bake bitmap onto it.
 *
 *  @param resolution = font size (font atlas cell diameter).
 *  @param file_name = font file name.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 font_init(Font *font, u32 resolution, const str *file_name);

void font_free(Font *font);

/*! -- IMPLEMENTATION: text.c --;
 *
 *  @brief init text rendering settings.
 *
 *  @param multisample = turn on multisampling.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 text_init(b8 multisample);

/*! -- IMPLEMENTATION: text.c --;
 *
 *  @brief start text rendering batch.
 *
 *  @param size = font height in pixels.
 *
 *  @param fbo = fbo to draw text to, if NULL, internal fbo is used
 *  (must then call 'text_fbo_blit()' to blit result onto desired fbo).
 *
 *  @param length = pre-allocate buffer for string (if 0, STRING_MAX is allocated).
 *  @param clear = clear the framebuffer before rendering.
 *
 *  @remark disables 'GL_DEPTH_TEST', 'text_stop()' re-enables it.
 *  @remark can re-allocate 'fbo' with 'multisample' setting used in 'text_init()'.
 */
void text_start(Font *font, f32 size, u64 length, FBO *fbo, b8 clear);

/*! -- IMPLEMENTATION: text.c --;
 *
 *  @brief push string's glyph metrics, position and alignment to render buffer.
 *
 *  @param align_x = TEXT_ALIGN_RIGHT, TEXT_ALIGN_CENTER, TEXT_ALIGN_LEFT.
 *  @param align_y = TEXT_ALIGN_TOP, TEXT_ALIGN_CENTER, TEXT_ALIGN_BOTTOM.
 *
 *  @remark can be called multiple times within a text rendering batch,
 *  chained with 'text_render()'.
 *
 *  @remark default alignment top left (0, 0), enum: TextAlignment.
 */
void text_push(const str *text, v2f32 pos, i8 align_x, i8 align_y);

/*! -- IMPLEMENTATION: text.c --;
 *  @brief render text to framebuffer.
 *
 *  @param color = hex format: 0xrrggbbaa.
 *
 *  @remark can be called multiple times within a text rendering batch,
 *  chained with 'text_push()'.
 */
void text_render(u32 color, b8 shadow);

/*! -- IMPLEMENTATION: text.c --;
 *
 *  @brief stop text rendering batch.
 *
 *  @remark enables 'GL_DEPTH_TEST'.
 */
void text_stop(void);

/*! -- IMPLEMENTATION: text.c --;
 *
 *  @brief blit rendered text onto 'fbo'.
 */
void text_fbo_blit(GLuint fbo);

/*! -- IMPLEMENTATION: text.c --;
 */
void text_free(void);

#endif /* ENGINE_CORE_H */
