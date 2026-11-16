/**
 *  Copyright 2026 Lily Awertnex
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 *  @file ui_element.h
 *
 *  @brief composing UI elements and baking transforms.
 */

#ifndef FSL_UI_ELEMENT_H
#define FSL_UI_ELEMENT_H

#include "../common/api.h"
#include "../common/types.h"

#include "../assets/asset_types.h"

#include "ui_event.h"
#include "ui_types.h"

/* ---- section: definitions ------------------------------------------------ */

#define FSL_UI_LABEL_TEXT_CAP 512

#define fsl_ui_element_type_set(element, type) \
    fsl_ui_element_type_set_func((fsl_ui_element*)(element), (type))

#define fsl_ui_element_texture_set(element, texture) \
    fsl_ui_element_texture_set_func((fsl_ui_element*)(element), (texture))

#define fsl_ui_element_uv_set(element, pos_x, pos_y, size_x, size_y) \
    fsl_ui_element_uv_set_func((fsl_ui_element*)(element), (pos_x), (pos_y), (size_x), (size_y))

#define fsl_ui_element_position_set(element, pos_x, pos_y, offset_x, offset_y, \
        offset_scaled_x, offset_scaled_y) \
        fsl_ui_element_position_set_func((fsl_ui_element*)(element), \
                (pos_x), (pos_y), (offset_x), (offset_y), \
                (offset_scaled_x), (offset_scaled_y))

#define fsl_ui_element_size_set(element, size_x, size_y, size_scaled_x, size_scaled_y) \
    fsl_ui_element_size_set_func((fsl_ui_element*)(element), \
            (size_x), (size_y), (size_scaled_x), (size_scaled_y))

#define fsl_ui_element_scale_set(element, scale_x, scale_y) \
    fsl_ui_element_scale_set_func((fsl_ui_element*)(element), (scale_x), (scale_y))

#define fsl_ui_element_alignment_set(element, align_x, align_y) \
    fsl_ui_element_alignment_set_func((fsl_ui_element*)(element), (align_x), (align_y))

#define fsl_ui_element_9_slice_set(element, is_9_slice, slice_size) \
    fsl_ui_element_9_slice_set_func((fsl_ui_element*)(element), (is_9_slice), (slice_size))

#define fsl_ui_element_tint_hex_set(element, tint) \
    fsl_ui_element_tint_hex_set_func((fsl_ui_element*)(element), (tint))

#define fsl_ui_element_tint_rgba_set(element, tint_r, tint_g, tint_b, tint_a) \
    fsl_ui_element_tint_rgba_set_func((fsl_ui_element*)(element), \
            (tint_r), (tint_g), (tint_b), (tint_a))

#define fsl_ui_element_callback_set(element, \
        event_type, func, data) \
        fsl_ui_element_callback_set_func((fsl_ui_element*)(element), (event_type), (func), (data))

#define fsl_ui_element_draw_callback_set(element, func, data, fbo) \
    fsl_ui_element_draw_callback_set_func((fsl_ui_element*)(element), (func), (data), (fbo))

#define fsl_ui_element_attach(parent, child) \
    fsl_ui_element_attach_func((fsl_ui_element*)(parent), (fsl_ui_element*)(child))

#define fsl_ui_element_detach(child) \
    fsl_ui_element_detach_func((fsl_ui_element*)(child))

#define fsl_ui_element_bake(element) \
    fsl_ui_element_bake_func((fsl_ui_element*)(element))

#define fsl_ui_element_draw(element) \
    fsl_ui_element_draw_func((fsl_ui_element*)(element))

typedef struct fsl_ui_draw_callback             fsl_ui_draw_callback;
typedef struct fsl_ui_element_container         fsl_ui_element_container;
typedef struct fsl_ui_element_rectangle         fsl_ui_element_rectangle;
typedef struct fsl_ui_element_sprite            fsl_ui_element_sprite;
typedef struct fsl_ui_element_panel_9_slice     fsl_ui_element_panel_9_slice;
typedef struct fsl_ui_element_label             fsl_ui_element_label;
typedef struct fsl_ui_element_button            fsl_ui_element_button;
typedef struct fsl_ui_element_custom            fsl_ui_element_custom;

typedef void (*fsl_ui_draw_func)(fsl_ui_element *element, void *data);

/**
 *  @brief UI element types.
 *
 *  requirements for adding element types:
 *      - element type enum entry.
 *      - element struct definition.
 *      - ui.c: add bake function entry in bake callback table.
 *      - ui.c: add draw function entry in draw callback table.
 *      - ui_element.c: write debug name in internal array.
 *      - ui_element.c: add capabilities in internal array.
 */
typedef enum fsl_ui_element_type
{
    FSL_UI_ELEMENT_TYPE_NONE,
    FSL_UI_ELEMENT_TYPE_CONTAINER,
    FSL_UI_ELEMENT_TYPE_RECTANGLE,
    FSL_UI_ELEMENT_TYPE_SPRITE,
    FSL_UI_ELEMENT_TYPE_PANEL_9_SLICE,
    FSL_UI_ELEMENT_TYPE_LABEL,
    FSL_UI_ELEMENT_TYPE_BUTTON,
    FSL_UI_ELEMENT_TYPE_CUSTOM, /* custom drawables (e.g., debug visuals) */
    FSL_UI_ELEMENT_TYPE_COUNT
} fsl_ui_element_type;

struct fsl_ui_draw_callback
{
    fsl_ui_draw_func func;
    void *data;             /* data to pass into `func` */
    fsl_fbo *fbo;           /* framebuffer for the raw render result */
}; /* fsl_ui_draw_callback */

/**
 *  @brief base UI element requirement.
 *
 *  must be placed at the top of every UI element's struct definition.
 */
struct fsl_ui_element_header
{
    fsl_ui_element_type type;
    u32 flag;                   /* enum @ref fsl_ui_element_flag */
    u32 capabilities;           /* enum @ref fsl_ui_element_capability */
    fsl_ui_transform transform;

    fsl_ui_element *parent;
    fsl_array children_buf;
    fsl_ui_element **children;  /* cached pointer from `children_buf` */

    fsl_ui_event event;
    fsl_ui_event_callback event_func[FSL_UI_EVENT_TYPE_COUNT];

    fsl_texture *texture;       /* -- DEPRECATED IN v0.18.0-dev -- */
}; /* fsl_ui_element_header */

struct fsl_ui_element_container
{
    fsl_ui_element_header header;
    fsl_ui_quad_1_slice quad;
}; /* fsl_ui_element_container */

struct fsl_ui_element_rectangle
{
    fsl_ui_element_header header;
    fsl_ui_quad_1_slice quad;
    fsl_texture *texture;
    v4f32 tint;
}; /* fsl_ui_element_rectangle */

struct fsl_ui_element_sprite
{
    fsl_ui_element_header header;
    fsl_ui_quad_1_slice quad;
    fsl_texture *texture;
}; /* fsl_ui_element_sprite */

struct fsl_ui_element_panel_9_slice
{
    fsl_ui_element_header header;
    fsl_ui_quad_9_slice quad;
    fsl_texture *texture;
}; /* fsl_ui_element_panel_9_slice */

struct fsl_ui_element_button
{
    fsl_ui_element_header header;
    fsl_ui_quad_9_slice quad;
    fsl_texture *texture;
}; /* fsl_ui_element_button */

struct fsl_ui_element_label
{
    fsl_ui_element_header header;
    fsl_ui_quad_1_slice quad;
    str text[FSL_UI_LABEL_TEXT_CAP];
    fsl_glyph_render_data buf[FSL_UI_LABEL_TEXT_CAP];
}; /* fsl_ui_element_label */

struct fsl_ui_element_custom
{
    fsl_ui_element_header header;
    fsl_ui_quad_1_slice quad;
    fsl_ui_draw_callback draw_func;
}; /* fsl_ui_element_custom */

/* ---- section: signatures ------------------------------------------------- */

/**
 *  @brief set element type of `element` so to process accordingly, first step
 *  in initializing a UI element before using.
 */
FSLAPI void fsl_ui_element_type_set_func(fsl_ui_element *element, fsl_ui_element_type type);

/**
 *  @brief set texture for `element` if element type supports it.
 */
FSLAPI void fsl_ui_element_texture_set_func(fsl_ui_element *element, fsl_texture *texture);

/**
 *  @brief set UV coordinates of `element` in its texture.
 *
 *  @param pos_x horizontal position in texture, in pixels.
 *  @param pos_y vertical position in texture, in pixels.
 *  @param size_x width in texture, in pixels.
 *  @param size_y height in texture, in pixels.
 */
FSLAPI void fsl_ui_element_uv_set_func(fsl_ui_element *element,
        i32 pos_x, i32 pos_y, i32 size_x, i32 size_y);

/**
 *  @brief set position of `element` on screen.
 *
 *  @param pos_x horizontal position on screen, in pixels.
 *  @param pos_y vertical position on screen, in pixels.
 *  @param offset_x horizontal offset from position, in pixels.
 *  @param offset_y vertical offset from position, in pixels.
 *
 *  @param offset_scaled_x horizontal offset from position, in pixels. (scales
 *  with `element->transform.scale`)
 *
 *  @param offset_scaled_y vertical offset from position, in pixels. (scales
 *  with `element->transform.scale`)
 */
FSLAPI void fsl_ui_element_position_set_func(fsl_ui_element *element,
        i32 pos_x, i32 pos_y, i32 offset_x, i32 offset_y,
        i32 offset_scaled_x, i32 offset_scaled_y);

/**
 *  @brief set size of `element` on screen.
 *
 *  @param size_x width on screen, in pixels.
 *  @param size_y height on screen, in pixels.
 *
 *  @param size_scaled_x width on screen, in pixels. (scales with
 *  `element->transform.scale`)
 *
 *  @param size_scaled_y height on screen, in pixels. (scales with
 *  `element->transform.scale`)
 */
FSLAPI void fsl_ui_element_size_set_func(fsl_ui_element *element,
        i32 size_x, i32 size_y, i32 size_scaled_x, i32 size_scaled_y);

/**
 *  @brief set scaling of `element` for its 'scaled' parameters.
 *
 *  @param scale_x horizontal gui scaling, for 'scaled' parameters.
 *  @param scale_y vertical gui scaling, for 'scaled' parameters.
 */
FSLAPI void fsl_ui_element_scale_set_func(fsl_ui_element *element,
        f32 scale_x, f32 scale_y);

/**
 *  @brief set alignment of `element`.
 *
 *  @param align_x horizontal alignment in respect to element's position and
 *  size.
 *
 *  @param align_y vertical alignment in respect to element's position and size.
 *
 *  format:
 *      <= -1: left-side/top is at position.
 *      == 0: center is at position.
 *      >= 1: right-side/bottom is at position.
 */
FSLAPI void fsl_ui_element_alignment_set_func(fsl_ui_element *element,
        i32 align_x, i32 align_y);

/**
 *  @brief set 9-slice corner slice diameter for `element`.
 *
 *  @param is_9_slice enable/disable 9-slice. (-- DEPRECATED IN v0.18.0-dev --)
 *
 *  @param slice_size corner slice diameter, middle slices will consume the rest
 *  of the area.
 */
FSLAPI void fsl_ui_element_9_slice_set_func(fsl_ui_element *element,
        b8 is_9_slice, i32 slice_size);

/**
 *  @brief set tint/color for `element`, in HEX format.
 */
FSLAPI void fsl_ui_element_tint_hex_set_func(fsl_ui_element *element, u32 tint);

/**
 *  @brief set tint/color for `element`, in RGBA format.
 *
 *  @remark color range [0.0f, 1.0f].
 */
FSLAPI void fsl_ui_element_tint_rgba_set_func(fsl_ui_element *element,
        f32 tint_r, f32 tint_g, f32 tint_b, f32 tint_a);

/**
 *  @brief set an event action callback for `element`. (e.g., button press,
 *  button hover)
 *
 *  @param element element to enable action callback for.
 *  @param event_type type of event that will trigger `func`.
 *  @param func function to call on `event_type` trigger.
 *  @param data data to pass in to `func` on `event_type` trigger.
 */
FSLAPI void fsl_ui_element_callback_set_func(fsl_ui_element *element,
        fsl_ui_event_type event_type, fsl_ui_event_func func, void *data);

/**
 *  @brief set a draw callback for `element`.
 *
 *  @param element element to enable draw callback for.
 *  @param func function to call inside @ref fsl_ui_element_draw().
 *  @param data data to pass in to `func` when drawing.
 */
FSLAPI void fsl_ui_element_draw_callback_set_func(fsl_ui_element *element,
        fsl_ui_draw_func func, void *data, fsl_fbo *fbo);

/**
 *  @brief attach a UI element to another (e.g., button in a menu).
 *
 *  `child` will draw using `parent` baked transform + its own baked transform.
 *
 *  @param parent element to attach to.
 *  @param child element to attach.
 */
FSLAPI void fsl_ui_element_attach_func(fsl_ui_element *parent, fsl_ui_element *child);

/**
 *  @brief detach a UI element from its parent (e.g., button in a menu).
 *
 *  @param child element to detach from their parent.
 */
FSLAPI void fsl_ui_element_detach_func(fsl_ui_element *child);

/**
 *  @brief bake/refresh a UI element's parameters to prepare for rendering.
 *
 *  @remark called automatically from @ref ui_element_draw() if transform dirty.
 */
FSLAPI void fsl_ui_element_bake_func(fsl_ui_element *element);

/**
 *  @brief draw a UI element on screen.
 *
 *  @remark function @ref fsl_ui_element_bake() must be called to
 *  prepare/refresh parameters for rendering.
 */
FSLAPI void fsl_ui_element_draw_func(fsl_ui_element *element);

#endif /* FSL_UI_ELEMENT_H */
