/*!
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

/*!
 *  @file physics.c
 *
 *  @brief physics functions.
 */

#include "../math/math.h"
#include "../math/matrix.h"

#include "physics_types.h"
#include "transform.h"

#include <math.h>

void fsl_position_set(fsl_transform_v3f64 *transform, f64 pos_x, f64 pos_y, f64 pos_z)
{
    transform->pos.x = pos_x;
    transform->pos.y = pos_y;
    transform->pos.z = pos_z;
}

void fsl_position_delta_set(fsl_transform_v3f64 *transform, f64 pos_x, f64 pos_y, f64 pos_z)
{
    transform->pos_delta.x = pos_x;
    transform->pos_delta.y = pos_y;
    transform->pos_delta.z = pos_z;
}

void fsl_rotation_set(fsl_transform_v3f64 *transform, f64 roll, f64 pitch, f64 yaw)
{
    transform->rot.x = roll;
    transform->rot.y = pitch;
    transform->rot.z = yaw;
}

void fsl_rotation_delta_set(fsl_transform_v3f64 *transform, f64 roll, f64 pitch, f64 yaw)
{
    transform->rot_delta.x = roll;
    transform->rot_delta.y = pitch;
    transform->rot_delta.z = yaw;
}

void fsl_scale_set(fsl_transform_v3f64 *transform, f64 scale_x, f64 scale_y, f64 scale_z)
{
    transform->scale.x = scale_x;
    transform->scale.y = scale_y;
    transform->scale.z = scale_z;
}

void fsl_scale_delta_set(fsl_transform_v3f64 *transform, f64 scale_x, f64 scale_y, f64 scale_z)
{
    transform->scale_delta.x = scale_x;
    transform->scale_delta.y = scale_y;
    transform->scale_delta.z = scale_z;
}

void fsl_position_add(fsl_transform_v3f64 *transform, f64 pos_x, f64 pos_y, f64 pos_z)
{
    transform->pos.x += pos_x;
    transform->pos.y += pos_y;
    transform->pos.z += pos_z;
}

void fsl_position_delta_add(fsl_transform_v3f64 *transform, f64 pos_x, f64 pos_y, f64 pos_z)
{
    transform->pos_delta.x += pos_x;
    transform->pos_delta.y += pos_y;
    transform->pos_delta.z += pos_z;
}

void fsl_rotation_add(fsl_transform_v3f64 *transform, f64 roll, f64 pitch, f64 yaw)
{
    transform->rot.x += roll;
    transform->rot.y += pitch;
    transform->rot.z += yaw;
}

void fsl_rotation_delta_add(fsl_transform_v3f64 *transform, f64 roll, f64 pitch, f64 yaw)
{
    transform->rot_delta.x += roll;
    transform->rot_delta.y += pitch;
    transform->rot_delta.z += yaw;
}

void fsl_scale_add(fsl_transform_v3f64 *transform, f64 scale_x, f64 scale_y, f64 scale_z)
{
    transform->scale.x += scale_x;
    transform->scale.y += scale_y;
    transform->scale.z += scale_z;
}

void fsl_scale_delta_add(fsl_transform_v3f64 *transform, f64 scale_x, f64 scale_y, f64 scale_z)
{
    transform->scale_delta.x += scale_x;
    transform->scale_delta.y += scale_y;
    transform->scale_delta.z += scale_z;
}

m4f32 fsl_transform_bake(const fsl_transform_v3f64 *transform)
{
    f32 SROL = 0.0f;
    f32 CROL = 0.0f;
    f32 SPCH = 0.0f;
    f32 CPCH = 0.0f;
    f32 SYAW = 0.0f;
    f32 CYAW = 0.0f;
    m4f32 final = {0};
    m4f32 location = {0};
    m4f32 rotation_roll = {0};
    m4f32 rotation_pitch = {0};
    m4f32 rotation_yaw = {0};
    m4f32 scale = {0};

    location = fsl_identity_m4f32();
    rotation_roll = fsl_identity_m4f32();
    rotation_pitch = fsl_identity_m4f32();
    rotation_yaw = fsl_identity_m4f32();
    scale = fsl_identity_m4f32();

    /* ---- transform base -------------------------------------------------- */

    SROL = sinf(transform->rot.x * FSL_DEG2RAD);
    CROL = sinf(transform->rot.x * FSL_DEG2RAD + FSL_HALF_PI);
    SPCH = sinf(transform->rot.y * FSL_DEG2RAD);
    CPCH = sinf(transform->rot.y * FSL_DEG2RAD + FSL_HALF_PI);
    SYAW = sinf(transform->rot.z * FSL_DEG2RAD);
    CYAW = sinf(transform->rot.z * FSL_DEG2RAD + FSL_HALF_PI);

    location.a41 = transform->pos.x;
    location.a42 = transform->pos.y;
    location.a43 = transform->pos.z;

    rotation_roll.a22 = CROL;
    rotation_roll.a23 = -SROL;
    rotation_roll.a32 = SROL;
    rotation_roll.a33 = CROL;

    rotation_pitch.a11 = CPCH;
    rotation_pitch.a13 = -SPCH;
    rotation_pitch.a31 = SPCH;
    rotation_pitch.a33 = CPCH;

    rotation_yaw.a11 = CYAW;
    rotation_yaw.a12 = -SYAW;
    rotation_yaw.a21 = SYAW;
    rotation_yaw.a22 = CYAW;

    scale.a11 = transform->scale.x;
    scale.a22 = transform->scale.y;
    scale.a33 = transform->scale.z;

    final = fsl_multiply_m4f32(scale, location);
    final = fsl_multiply_m4f32(rotation_pitch, final);
    final = fsl_multiply_m4f32(rotation_yaw, final);

    /* ---- transform delta ------------------------------------------------- */

    SROL = sinf(transform->rot_delta.x * FSL_DEG2RAD);
    CROL = sinf(transform->rot_delta.x * FSL_DEG2RAD + FSL_HALF_PI);
    SPCH = sinf(transform->rot_delta.y * FSL_DEG2RAD);
    CPCH = sinf(transform->rot_delta.y * FSL_DEG2RAD + FSL_HALF_PI);
    SYAW = sinf(transform->rot_delta.z * FSL_DEG2RAD);
    CYAW = sinf(transform->rot_delta.z * FSL_DEG2RAD + FSL_HALF_PI);

    location.a41 = transform->pos_delta.x;
    location.a42 = transform->pos_delta.y;
    location.a43 = transform->pos_delta.z;

    rotation_roll.a22 = CROL;
    rotation_roll.a23 = -SROL;
    rotation_roll.a32 = SROL;
    rotation_roll.a33 = CROL;

    rotation_pitch.a11 = CPCH;
    rotation_pitch.a13 = -SPCH;
    rotation_pitch.a31 = SPCH;
    rotation_pitch.a33 = CPCH;

    rotation_yaw.a11 = CYAW;
    rotation_yaw.a12 = -SYAW;
    rotation_yaw.a21 = SYAW;
    rotation_yaw.a22 = CYAW;

    scale.a11 = transform->scale_delta.x;
    scale.a22 = transform->scale_delta.y;
    scale.a33 = transform->scale_delta.z;

    final = fsl_multiply_m4f32(scale, final);
    final = fsl_multiply_m4f32(location, final);
    final = fsl_multiply_m4f32(rotation_pitch, final);
    final = fsl_multiply_m4f32(rotation_yaw, final);

    return final;
}

fsl_physics_material fsl_physics_material_init(f64 friction_x, f64 friction_y, f64 friction_z,
    f64 drag_x, f64 drag_y, f64 drag_z, f64 bounciness)
{
    fsl_physics_material mat = {0};
    mat.friction.x = friction_x;
    mat.friction.y = friction_y;
    mat.friction.z = friction_z;
    mat.drag.x = drag_x;
    mat.drag.y = drag_y;
    mat.drag.z = drag_z;
    mat.bounciness = bounciness;
    return mat;
}

void fsl_kinematics_update_v3f64(v3f64 *pos, fsl_physics_force force, fsl_kinematics *kn,
        const fsl_physics_material *mat, f64 delta_time)
{
    kn->acceleration.x = force.x * kn->acceleration_rate * (1.0 - mat->friction.x);
    kn->acceleration.y = force.y * kn->acceleration_rate * (1.0 - mat->friction.y);
    kn->acceleration.z = force.z * kn->acceleration_rate * (1.0 - mat->friction.z);

    kn->velocity.x += kn->acceleration.x * delta_time;
    kn->velocity.y += kn->acceleration.y * delta_time;
    kn->velocity.z += kn->acceleration.z * delta_time;

    kn->velocity.x -= mat->drag.x * kn->velocity.x * kn->mass_inv * delta_time;
    kn->velocity.y -= mat->drag.y * kn->velocity.y * kn->mass_inv * delta_time;
    kn->velocity.z -= mat->drag.z * kn->velocity.z * kn->mass_inv * delta_time;
    kn->velocity.x -= mat->friction.x * kn->velocity.x * kn->mass_inv * delta_time;
    kn->velocity.y -= mat->friction.y * kn->velocity.y * kn->mass_inv * delta_time;
    kn->velocity.z -= mat->friction.z * kn->velocity.z * kn->mass_inv * delta_time;
    kn->speed = sqrt(fsl_len_v3f64(kn->velocity));

    pos->x += kn->velocity.x * delta_time;
    pos->y += kn->velocity.y * delta_time;
    pos->z += kn->velocity.z * delta_time;
}

void fsl_kinematics_mass_set(fsl_kinematics *kn, f64 mass)
{
    kn->mass = mass;
    kn->mass_inv = 1.0 / (mass > FSL_EPSILON ? mass : FSL_EPSILON);
}
