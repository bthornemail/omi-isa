#include "omicron.h"
#include <string.h>

uint32_t omicron_resolve(const OMI_512_Envelope* env, int layer) {
    if (!env) return 0;

    if (layer < 1 || layer > 12) return 0;

    if (layer <= 7) {
        uint64_t board = 0;
        omi_env_to_bitboard(env, &board);
        switch (layer) {
            case 1: return 1;
            case 2: return (board & 1) ? 2 : 1;
            case 3: return (board & 0xF) % 6 + 1;
            case 4: return (board & 0xFF) % 24;
            case 5: return (board >> 8) % 120;
            case 6: return (board >> 16) % 720;
            case 7: return (board >> 24) % 5040;
        }
    }

    uint16_t seg;
    switch (layer) {
        case 8:
            return 40320;
        case 9:
            seg = (uint16_t)env->gauge[4] << 8 | env->gauge[5];
            return (uint32_t)(seg % 9);
        case 10:
            seg = (uint16_t)env->gauge[6] << 8 | env->gauge[7];
            return (uint32_t)(seg % 10);
        case 11:
            seg = (uint16_t)env->orientation[0] << 8 | env->orientation[1];
            return (uint32_t)(seg % 11);
        case 12:
            seg = (uint16_t)env->orientation[2] << 8 | env->orientation[3];
            return (uint32_t)(seg % 12);
        default:
            return 0;
    }
}

int omicron_slide(OMI_DispatchContext* ctx, uint8_t gauge_code) {
    if (!ctx) return -1;

    const OmiGaugeEntry* entry = omi_gauge_lookup(gauge_code);
    if (!entry || !entry->active) return -1;

    int ret;

    ret = gauge_exec_eval(ctx, OMICRON_SLIDE_PLACE_CODE, 0);
    if (ret < 0) return -1;

    ret = gauge_exec_eval(ctx, OMICRON_SLIDE_INJECT_CODE, 0);
    if (ret < 0) return -2;

    ret = gauge_exec_eval(ctx, OMICRON_SLIDE_KERNEL_CODE, 0);
    if (ret < 0) return -3;

    ret = gauge_exec_eval(ctx, OMICRON_SLIDE_CLOSE_CODE, 0);
    if (ret < 0) return -4;

    return 0;
}
