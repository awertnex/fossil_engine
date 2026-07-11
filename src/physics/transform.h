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
 *  @file transform.h
 *
 *  @brief transform functions.
 */

#ifndef FSL_PHYSICS_TRANSFORM_H
#define FSL_PHYSICS_TRANSFORM_H

#include "../common/api.h"
#include "../common/types.h"
#include "../math/matrix.h"

#include "physics_types.h"

FSLAPI void fsl_position_set(fsl_transform_v3f64 *transform, f64 pos_x, f64 pos_y, f64 pos_z);
FSLAPI void fsl_position_delta_set(fsl_transform_v3f64 *transform, f64 pos_x, f64 pos_y, f64 pos_z);
FSLAPI void fsl_rotation_set(fsl_transform_v3f64 *transform, f64 roll, f64 pitch, f64 yaw);
FSLAPI void fsl_rotation_delta_set(fsl_transform_v3f64 *transform, f64 roll, f64 pitch, f64 yaw);
FSLAPI void fsl_scale_set(fsl_transform_v3f64 *transform, f64 scale_x, f64 scale_y, f64 scale_z);
FSLAPI void fsl_scale_delta_set(fsl_transform_v3f64 *transform, f64 scale_x, f64 scale_y, f64 scale_z);
FSLAPI void fsl_position_add(fsl_transform_v3f64 *transform, f64 pos_x, f64 pos_y, f64 pos_z);
FSLAPI void fsl_position_delta_add(fsl_transform_v3f64 *transform, f64 pos_x, f64 pos_y, f64 pos_z);
FSLAPI void fsl_rotation_add(fsl_transform_v3f64 *transform, f64 roll, f64 pitch, f64 yaw);
FSLAPI void fsl_rotation_delta_add(fsl_transform_v3f64 *transform, f64 roll, f64 pitch, f64 yaw);
FSLAPI void fsl_scale_add(fsl_transform_v3f64 *transform, f64 scale_x, f64 scale_y, f64 scale_z);
FSLAPI void fsl_scale_delta_add(fsl_transform_v3f64 *transform, f64 scale_x, f64 scale_y, f64 scale_z);
FSLAPI m4f32 fsl_transform_bake(const fsl_transform_v3f64 *transform);

/*!
 *  @brief initialize a physics material.
 */
FSLAPI fsl_physics_material fsl_physics_material_init(f64 friction_x, f64 friction_y, f64 friction_z,
    f64 drag_x, f64 drag_y, f64 drag_z, f64 bounciness);

/*!
 *  @brief apply a force to `kn` as per physics material `mat`.
 *
 *  @param force raw input force, for acceleration.
 *  @param mass_inv inverse of mass to scale final velocity by.
 */
FSLAPI void fsl_kinematics_update_v3f64(fsl_kinematics *kn, fsl_physics_force force,
        const fsl_physics_material *mat, f64 mass_inv, f64 delta_time);

/*!
 *  @brief set mass for `kn` and store its inverse for internal use.
 */
FSLAPI void fsl_kinematics_mass_set(fsl_kinematics *kn, f64 mass);

/*!
 *  @brief get total velocity from all velocities in array `kn`.
 *
 *  @param len length of array `kn`.
 *
 *  @return total velocity.
 */
FSLAPI v3f64 fsl_kinematics_velocity_get(const fsl_kinematics *kn, u64 len);

#endif /* FSL_PHYSICS_TRANSFORM_H */
