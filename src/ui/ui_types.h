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
 *  @file ui_types.h
 *
 *  @brief UI types.
 */

#ifndef FSL_UI_TYPES_H
#define FSL_UI_TYPES_H

#include "../math/vector.h"

typedef struct fsl_ui_quad              fsl_ui_quad;
typedef struct fsl_ui_quad_1_slice      fsl_ui_quad_1_slice;
typedef struct fsl_ui_quad_9_slice      fsl_ui_quad_9_slice;
typedef struct fsl_ui_transform         fsl_ui_transform;
typedef struct fsl_glyph_render_data    fsl_glyph_render_data;

typedef struct fsl_ui_element_header    fsl_ui_element_header;
typedef fsl_ui_element_header           fsl_ui_element;

/**
 *  @brief finalized transforms from @ref fsl_ui_transform, ready for rendering.
 */
struct fsl_ui_quad
{
    /**
     *  @brief baked absolute position in texture, in normalized coordinates.
     */
    v2f32 uv_pos;

    /**
     *  @brief baked absolute size in texture, in normalized coordinates.
     */
    v2f32 uv_size;

    v2f32 pos;              /* baked absolute position on screen, in pixels */
    v2f32 size;             /* baked absolute size on screen, in pixels */
}; /* fsl_ui_quad */

struct fsl_ui_quad_1_slice
{
    fsl_ui_quad quad;
}; /* fsl_ui_quad_1_slice */

struct fsl_ui_quad_9_slice
{
    fsl_ui_quad quad[9];
    i32 slice_size;         /* corner slice diameter, in pixels */
}; /* fsl_ui_quad_9_slice */

struct fsl_ui_transform
{
    v2i32 uv_pos;           /* position in texture, in pixels */
    v2i32 uv_size;          /* size of texture, in pixels */
    v2i32 pos;              /* position on screen, in pixels */
    v2i32 offset;           /* offset on screen from `pos`, in pixels */

    /**
     *  @brief offset on screen from `pos`, in pixels. (scales with `scale`)
     */
    v2i32 offset_scaled;

    v2i32 size;             /* size on screen, in pixels */

    /**
     *  @brief size on screen, in pixels. (scales with `scale`)
     */
    v2i32 size_scaled;

    v2f32 scale;            /* gui scaling, for 'scaled' parameters */

    /**
     *  @brief alignment in respect to `pos`, `size` and `size_scaled`.
     *
     *  <= -1: left-side/top is at `pos`.
     *  == 0: center/center is at `pos`.
     *  >= 1: right-side/bottom is at `pos`.
     */
    v2i32 align;

    v2f32 pos_local;        /* baked local position, in pixels */

    /**
     *  @brief baked screen quad.
     *
     *  not necessarily final, it depends on what else an element needs to be
     *  drawn on screem.
     */
    fsl_ui_quad baked;

}; /* fsl_ui_transform */

struct fsl_glyph_render_data
{
    v2f32 pos;
    u32 char_index;
    u32 color;
}; /* fsl_glyph_render_data */

#endif /* FSL_UI_TYPES_H */
