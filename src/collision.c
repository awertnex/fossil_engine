/*  Copyright 2026 Lily Awertnex
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
 *  limitations under the License.OFTWARE.
 */

/*
 *	collision.c - collision detection
 */

#include "h/collision.h"
#include "h/types.h"
#include "h/math.h"

b8 fsl_is_intersect_aabb(fsl_bounding_box a, fsl_bounding_box b)
{
    return !(a.pos.x >= b.pos.x + b.size.x || b.pos.x >= a.pos.x + a.size.x ||
            a.pos.y >= b.pos.y + b.size.y || b.pos.y >= a.pos.y + a.size.y ||
            a.pos.z >= b.pos.z + b.size.z || b.pos.z >= a.pos.z + a.size.z);
}

f32 fsl_get_swept_aabb(fsl_bounding_box a, fsl_bounding_box b, v3f32 displacement, v3f32 *normal)
{
    v3f32 entry, exit, entry_distance, exit_distance;
    f32 entry_time, exit_time;

    /* ---- entry and exit distance ----------------------------------------- */

    if (displacement.x > 0.0f)
    {
        entry_distance.x = b.pos.x - (a.pos.x + a.size.x);
        exit_distance.x = (b.pos.x + b.size.x) - a.pos.x;
    }
    else
    {
        entry_distance.x = (b.pos.x + b.size.x) - a.pos.x;
        exit_distance.x = b.pos.x - (a.pos.x + a.size.x);
    }

    if (displacement.y > 0.0f)
    {
        entry_distance.y = b.pos.y - (a.pos.y + a.size.y);
        exit_distance.y = (b.pos.y + b.size.y) - a.pos.y;
    }
    else
    {
        entry_distance.y = (b.pos.y + b.size.y) - a.pos.y;
        exit_distance.y = b.pos.y - (a.pos.y + a.size.y);
    }

    if (displacement.z > 0.0f)
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

    if (displacement.x == 0.0f)
    {
        entry.x = -INFINITY;
        exit.x = INFINITY;
    }
    else
    {
        entry.x = entry_distance.x / displacement.x;
        exit.x = exit_distance.x / displacement.x;
    }

    if (displacement.y == 0.0f)
    {
        entry.y = -INFINITY;
        exit.y = INFINITY;
    }
    else
    {
        entry.y = entry_distance.y / displacement.y;
        exit.y = exit_distance.y / displacement.y;
    }

    if (displacement.z == 0.0f)
    {
        entry.z = -INFINITY;
        exit.z = INFINITY;
    }
    else
    {
        entry.z = entry_distance.z / displacement.z;
        exit.z = exit_distance.z / displacement.z;
    }

    entry_time = fsl_max_v3f32(entry);
    exit_time = fsl_min_v3f32(exit);

    if (entry_time > exit_time || exit_time < 0.0f || entry_time > 1.0f)
        goto cleanup;

    /* ---- normals --------------------------------------------------------- */

    *normal = (v3f32){0};

    switch (fsl_max_axis_v3f32(entry))
    {
        case 1:
            normal->x = displacement.x > 0.0f ? -1.0f : 1.0f;
            break;

        case 2:
            normal->y = displacement.y > 0.0f ? -1.0f : 1.0f;
            break;

        case 3:
            normal->z = displacement.z > 0.0f ? -1.0f : 1.0f;
            break;
    }

    return entry_time;

cleanup:

    *normal = (v3f32){0};
    return 1.0f;
}
