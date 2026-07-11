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
 *  @file collision.c
 *
 *  @brief collision detection.
 */

#include "../math/math.h"
#include "../math/vector.h"

#include "collision.h"

#include <math.h>

b8 fsl_is_intersect_aabb(fsl_bounding_box a, fsl_bounding_box b)
{
    return !(a.pos.x >= b.pos.x + b.size.x || b.pos.x >= a.pos.x + a.size.x ||
            a.pos.y >= b.pos.y + b.size.y || b.pos.y >= a.pos.y + a.size.y ||
            a.pos.z >= b.pos.z + b.size.z || b.pos.z >= a.pos.z + a.size.z);
}

fsl_collision_info fsl_get_swept_aabb(fsl_bounding_box a, fsl_bounding_box b,
        v3f64 displacement)
{
    fsl_collision_info c = {0};
    v3f64 entry;
    v3f64 exit;
    v3f64 entry_distance;
    v3f64 exit_distance;

    /* ---- entry and exit distance ----------------------------------------- */

    if (displacement.x > 0.0)
    {
        entry_distance.x = b.pos.x - (a.pos.x + a.size.x);
        exit_distance.x = (b.pos.x + b.size.x) - a.pos.x;
    }
    else
    {
        entry_distance.x = (b.pos.x + b.size.x) - a.pos.x;
        exit_distance.x = b.pos.x - (a.pos.x + a.size.x);
    }

    if (displacement.y > 0.0)
    {
        entry_distance.y = b.pos.y - (a.pos.y + a.size.y);
        exit_distance.y = (b.pos.y + b.size.y) - a.pos.y;
    }
    else
    {
        entry_distance.y = (b.pos.y + b.size.y) - a.pos.y;
        exit_distance.y = b.pos.y - (a.pos.y + a.size.y);
    }

    if (displacement.z > 0.0)
    {
        entry_distance.z = b.pos.z - (a.pos.z + a.size.z);
        exit_distance.z = (b.pos.z + b.size.z) - a.pos.z;
    }
    else
    {
        entry_distance.z = (b.pos.z + b.size.z) - a.pos.z;
        exit_distance.z = b.pos.z - (a.pos.z + a.size.z);
    }

    /* ---- entry and exit -------------------------------------------------- */

    if (displacement.x == 0.0)
    {
        entry.x = -INFINITY;
        exit.x = INFINITY;
    }
    else
    {
        entry.x = entry_distance.x / displacement.x;
        exit.x = exit_distance.x / displacement.x;
    }

    if (displacement.y == 0.0)
    {
        entry.y = -INFINITY;
        exit.y = INFINITY;
    }
    else
    {
        entry.y = entry_distance.y / displacement.y;
        exit.y = exit_distance.y / displacement.y;
    }

    if (displacement.z == 0.0)
    {
        entry.z = -INFINITY;
        exit.z = INFINITY;
    }
    else
    {
        entry.z = entry_distance.z / displacement.z;
        exit.z = exit_distance.z / displacement.z;
    }

    c.entry_time = fsl_max_v3f64(entry);
    c.exit_time = fsl_min_v3f64(exit);

    if (c.entry_time > c.exit_time || c.exit_time < 0.0 || c.entry_time > 1.0)
    {
        c.entry_time = 1.0;
        return c;
    }

    /* ---- normals --------------------------------------------------------- */

    switch (fsl_max_axis_v3f64(entry))
    {
        case 1:
            c.normal.x = displacement.x > 0.0 ? -1.0 : 1.0;
            break;

        case 2:
            c.normal.y = displacement.y > 0.0 ? -1.0 : 1.0;
            break;

        case 3:
            c.normal.z = displacement.z > 0.0 ? -1.0 : 1.0;
            break;
    }

    if (fsl_is_in_bounds_f64(c.entry_time, -1.0, 0.0))
    {
        c.hit = TRUE;

        c.dot = fsl_dot_v3f64(displacement, c.normal);
        if (c.dot < 0.0)
            c.slide = TRUE;
    }

    return c;
}
