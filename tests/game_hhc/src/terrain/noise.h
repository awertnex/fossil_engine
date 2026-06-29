#ifndef HHC_TERRAIN_NOISE_H
#define HHC_TERRAIN_NOISE_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/math/vector.h"

#include "../chunking/chunk_work.h"

typedef struct hhc_noise_sample
{
    f64 v[3];   /* position */
    i64 a[3];   /* `v` floor */
    i64 b[3];   /* `v` ceil */
    f64 dv[3];  /* `v` delta */
    f64 dw[3];  /* `v` weight */
    f64 da[3];  /* `a` delta */
    f64 db[3];  /* `b` delta */
} hhc_noise_sample;

typedef f64 (*hhc_noise_sample_lerp_func)(const f64 *n, const f64 *t);

f64 nolerp_f64(const f64 *n, const f64 *t);
f64 lerp_f64(const f64 *n, const f64 *t);
f64 bilerp_f64(const f64 *n, const f64 *t);
f64 trilerp_f64(const f64 *n, const f64 *t);
v2f64 gradient_2d(i32 x, i32 y, u64 seed);
v3f64 gradient_3d(i32 x, i32 y, i32 z, u64 seed);

chunk_work_cost noise_sample_axis_init(hhc_noise_sample *s, u8 axis, f64 pos, f64 frequency);
chunk_work_cost noise_sample_make_2d(const hhc_noise_sample *s, f64 *dst, f64 amplitude, u64 seed);
chunk_work_cost noise_sample_make_3d(const hhc_noise_sample *s, f64 *dst, f64 amplitude, u64 seed);

#endif /* HHC_TERRAIN_NOISE_H */
