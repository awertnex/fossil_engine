#include "deps/fossil/common/api.h"

#include "../plugins/big_num_separator/big_num_separator.h"

#include "chunk_work.h"

#include <stdio.h>
#include <math.h>

void chunk_receipt_evaluate(hhc_chunk_receipt *receipt, u32 chunk_cpi)
{
    u32 i = 0;
    f32 distance_multiplier = sqrtf((f32)chunk_cpi);

    receipt->subtotal = 0;

    for (; i < CHUNK_RECEIPT_ITEM_COUNT; ++i)
        receipt->subtotal += receipt->cost[i];

    receipt->priority_rate = (((f64)receipt->subtotal * distance_multiplier) /
            (f64)receipt->subtotal) * 100.0 - 100.0;

    receipt->total = receipt->subtotal * distance_multiplier;
}

void chunk_receipt_print(hhc_chunk_receipt *src, hhc_chunk_receipt_printed *dst)
{
    u32 i = 0;

    for (; i < CHUNK_RECEIPT_ITEM_COUNT; ++i)
        snprintf(dst->cost[i], FSL_ID_CAP, "$%s", big_num_separator_i64(src->cost[i]));

    snprintf(dst->subtotal, FSL_ID_CAP, "$%s", big_num_separator_i64(src->subtotal));
    snprintf(dst->priority_rate, FSL_ID_CAP, "%.2f%%", src->priority_rate);
    snprintf(dst->total, FSL_ID_CAP, "$%s", big_num_separator_i64(src->total));


    snprintf(dst->printed, CHUNK_RECEIPT_CAP,
            "=========  Chunk  Receipt  =========\n"
            "Generate Terrain    %16s\n"
            "Generate Caves      %16s\n"
            "Mesh                %16s\n"
            "Import              %16s\n"
            "Export              %16s\n"
            "------------------------------------\n\n"
            "Subtotal            %16s\n"
            "Priority Rate       %16s\n"
            "------------------------------------\n\n"
            "Total               %16s\n"
            "====================================\n",
            dst->cost[CHUNK_RECEIPT_ITEM_GENERATE_TERRAIN],
            dst->cost[CHUNK_RECEIPT_ITEM_GENERATE_CAVES],
            dst->cost[CHUNK_RECEIPT_ITEM_MESH],
            dst->cost[CHUNK_RECEIPT_ITEM_IMPORT],
            dst->cost[CHUNK_RECEIPT_ITEM_EXPORT],
            dst->subtotal,
            dst->priority_rate,
            dst->total);
}
