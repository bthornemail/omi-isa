#include "omi_receipt.h"
#include "omicron.h"
#include <string.h>

static const uint8_t CANONICAL_PREHEADER[8] = {
    0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF
};

static uint8_t gauge_action_code(const OMI_512_Envelope* env) {
    if (!env) return 0;
    return env->gauge[0] & 0x1F;
}

int omi_receipt_compute(const OMI_512_Envelope* env, OMI_Receipt* receipt) {
    if (!env || !receipt) return -1;

    memset(receipt, 0, sizeof(OMI_Receipt));

    receipt->layer_values[0] = omicron_resolve(env, 8);
    receipt->layer_values[1] = omicron_resolve(env, 9);
    receipt->layer_values[2] = omicron_resolve(env, 10);
    receipt->layer_values[3] = omicron_resolve(env, 11);
    receipt->layer_values[4] = omicron_resolve(env, 12);

    receipt->gauge_action = gauge_action_code(env);

    int pi = 0;
    for (int i = 0; i < 8; i++) {
        if (pi < OMI_RECEIPT_PATH_MAX) {
            receipt->path[pi++] = env->gauge[i];
        }
    }
    for (int i = 0; i < 8 && pi < OMI_RECEIPT_PATH_MAX; i++) {
        receipt->path[pi++] = env->orientation[i];
    }
    receipt->path_len = (pi > OMI_RECEIPT_PATH_MAX) ? OMI_RECEIPT_PATH_MAX : pi;
    receipt->valid = 1;

    return 0;
}

int omi_receipt_verify(const OMI_512_Envelope* env, const OMI_Receipt* receipt) {
    if (!env || !receipt) return -1;
    if (!receipt->valid) return -2;

    OMI_Receipt computed;
    int ret = omi_receipt_compute(env, &computed);
    if (ret != 0) return -3;

    if (computed.gauge_action != receipt->gauge_action) return -4;

    for (int i = 0; i < OMI_RECEIPT_LAYERS; i++) {
        if (computed.layer_values[i] != receipt->layer_values[i]) return -5;
    }

    if (computed.path_len != receipt->path_len) return -6;
    if (memcmp(computed.path, receipt->path, (size_t)receipt->path_len) != 0) return -7;

    return 0;
}

int omi_receipt_verify_meta64(const OMI_512_Envelope* env) {
    if (!env) return -1;

    if (memcmp(env->gauge, CANONICAL_PREHEADER, 8) != 0) return -2;
    if (env->gauge[0] < 0xF0) return -3;
    if (env->gauge[0] != env->gauge[7]) return -4;

    if (env->gauge[2] != 0x1C || env->gauge[3] != 0x1D ||
        env->gauge[4] != 0x1E || env->gauge[5] != 0x1F) return -5;
    if (env->gauge[6] != 0x20) return -6;

    return 0;
}

int omi_receipt_to_string(const OMI_Receipt* receipt, char* out, size_t out_len) {
    if (!receipt || !out || out_len < 1) return -1;

    size_t pos = 0;

    if (receipt->valid) {
        if (pos + 2 < out_len) { out[pos++] = 'V'; out[pos++] = ':'; out[pos] = '\0'; }
    }

    char buf[16];
    for (int i = 0; i < OMI_RECEIPT_LAYERS && pos < out_len; i++) {
        int n = snprintf(buf, sizeof(buf), "%s%u", (i == 0) ? "" : ",", receipt->layer_values[i]);
        for (int j = 0; j < n && pos + 1 < out_len; j++)
            out[pos++] = buf[j];
    }

    if (pos + 3 < out_len) {
        int n = snprintf(buf, sizeof(buf), ";g%u", receipt->gauge_action);
        for (int j = 0; j < n && pos + 1 < out_len; j++)
            out[pos++] = buf[j];
    }

    if (pos < out_len) out[pos] = '\0';
    return (int)pos;
}
