#include "omienv.h"
#include <string.h>

static const uint8_t CANONICAL_PREHEADER[8] = {
    0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF
};

static OmiGaugeEntry gauge_table[128];
static int gauge_table_initialized = 0;

uint8_t hex_to_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0xFF;
}

char nibble_to_hex(uint8_t n) {
    const char hex[] = "0123456789ABCDEF";
    return hex[n & 0xF];
}

int omi_env_parse(const uint8_t* data, size_t len, OMI_512_Envelope* env) {
    if (!data || !env || len < OMI_ENV_SIZE) return -1;
    memcpy(env->gauge, data, 8);
    memcpy(env->orientation, data + 8, 8);
    memcpy(env->recovery, data + 16, 8);
    memcpy(env->target, data + 24, 8);
    memcpy(env->relation, data + 32, 32);
    return 0;
}

int omi_env_validate(const OMI_512_Envelope* env) {
    if (!env) return -1;
    if (memcmp(env->gauge, CANONICAL_PREHEADER, 8) != 0) return -2;
    if (env->gauge[0] != env->gauge[7]) return -3;
    if (env->gauge[0] < 0xF0) return -4;
    if (env->gauge[2] != 0x1C || env->gauge[3] != 0x1D ||
        env->gauge[4] != 0x1E || env->gauge[5] != 0x1F) return -5;
    if (env->gauge[6] != 0x20) return -6;
    return 0;
}

int omi_env_to_bitboard(const OMI_512_Envelope* env, uint64_t* board) {
    if (!env || !board) return -1;
    uint64_t b = 0;
    b = omi_bb_set_gauge(b, env->gauge[0]);
    for (int i = 0; i < 8; i++)
        if (env->orientation[i] < 32)
            b = omi_bb_apply(b, env->orientation[i], (uint8_t)(22 + i));
    for (int i = 0; i < 8; i++)
        if (env->recovery[i] < 32)
            b = omi_bb_apply(b, env->recovery[i], (uint8_t)(14 + i));
    for (int i = 0; i < 8; i++)
        if (env->target[i] < 32)
            b = omi_bb_apply(b, env->target[i], (uint8_t)(6 + i));
    for (int i = 0; i < 8 && i < 32; i++)
        if (env->relation[i] < 32)
            b = omi_bb_apply(b, env->relation[i], (uint8_t)i);
    if (env->gauge[4] == 0x1E)
        b = omi_bb_set_flag(b, OMI_BB_BRIDGE_1E_BIT);
    for (int i = 0; i < 32; i++) {
        if (env->relation[i] == 0x78) b = omi_bb_set_flag(b, OMI_BB_BRIDGE_78_BIT);
        if (env->relation[i] == 0x7F) b = omi_bb_set_flag(b, OMI_BB_SEAL_7F_BIT);
    }
    for (int i = 0; i < 7; i++) {
        uint16_t w = (uint16_t)(env->target[i] << 8) | env->target[i+1];
        if (w == 0x7C00) b = omi_bb_set_flag(b, OMI_BB_BOOT_7C00_BIT);
        if (w == 0xAA55) b = omi_bb_set_flag(b, OMI_BB_BRIDGE_AA55_BIT);
    }
    *board = b;
    return 0;
}

int omi_env_to_relation(const OMI_512_Envelope* env, char* out, size_t out_len) {
    if (!env || !out || out_len < 1) return -1;
    size_t pos = 0;
    for (int i = 0; i < 8; i++) {
        if (pos >= out_len) break;
        if (i == 0) { out[pos++] = 'o'; out[pos++] = '-'; }
        else if (i == 4) { out[pos++] = '/'; }
        else { out[pos++] = '-'; }
        uint16_t s = (uint16_t)(env->relation[i*2] << 8) | env->relation[i*2+1];
        if (pos + 4 < out_len) {
            out[pos++] = nibble_to_hex((s >> 12) & 0xF);
            out[pos++] = nibble_to_hex((s >> 8) & 0xF);
            out[pos++] = nibble_to_hex((s >> 4) & 0xF);
            out[pos++] = nibble_to_hex(s & 0xF);
        }
    }
    if (pos + 1 < out_len) {
        out[pos++] = '?';
        for (int i = 0; i < 8 && pos + 1 < out_len; i++) {
            uint8_t b = env->relation[16 + i];
            if (b >= 0x20 && b < 0x7F) out[pos++] = (char)b;
        }
    }
    if (pos + 1 < out_len) {
        out[pos++] = '@';
        for (int i = 0; i < 8 && pos + 1 < out_len; i++) {
            uint8_t b = env->relation[24 + i];
            if (b >= 0x20 && b < 0x7F) out[pos++] = (char)b;
        }
    }
    if (pos < out_len) out[pos] = '\0';
    return (int)pos;
}

uint64_t omi_bb_set_gauge(uint64_t board, uint8_t code) {
    return (board & ~OMI_BB_GAUGE_MASK) | (uint64_t)(code & 0x7F);
}

uint64_t omi_bb_set_flag(uint64_t board, unsigned bit) {
    return board | (1ULL << bit);
}

int omi_bb_get_flag(uint64_t board, unsigned bit) {
    return (board >> bit) & 1;
}

uint8_t omi_bb_get_gauge(uint64_t board) {
    return (uint8_t)(board & OMI_BB_GAUGE_MASK);
}

uint8_t omi_bb_get_place(uint64_t board) {
    return (uint8_t)((board >> OMI_BB_PLACE_SHIFT) & 0x3FF);
}

uint32_t omi_bb_get_result(uint64_t board) {
    return (uint32_t)((board >> OMI_BB_RESULT_SHIFT) & 0xFFFF);
}

uint64_t omi_bb_apply(uint64_t board, uint8_t byte, uint8_t place) {
    if (place > 31) return board;
    uint8_t hi = (byte >> 4) & 0xF;
    uint8_t lo = byte & 0xF;
    int dplus = 0, dminus = 0;
    if (hi == 0 || hi == 5 || hi == 0xA || hi == 0xF) dplus++;
    if (lo == 0 || lo == 5 || lo == 0xA || lo == 0xF) dplus++;
    if (hi == 3 || hi == 6 || hi == 9 || hi == 0xC) dminus++;
    if (lo == 3 || lo == 6 || lo == 9 || lo == 0xC) dminus++;
    uint64_t dp = (board & OMI_BB_DPLUS_MASK) >> OMI_BB_DPLUS_SHIFT;
    uint64_t dm = (board & OMI_BB_DMINUS_MASK) >> OMI_BB_DMINUS_SHIFT;
    dp = (dp + dplus) & 0x1F;
    dm = (dm + dminus) & 0x1F;
    board = (board & ~OMI_BB_DPLUS_MASK) | (dp << OMI_BB_DPLUS_SHIFT);
    board = (board & ~OMI_BB_DMINUS_MASK) | (dm << OMI_BB_DMINUS_SHIFT);
    uint64_t place_bits = (uint64_t)place & 0x3FF;
    board = (board & ~OMI_BB_PLACE_MASK) | (place_bits << OMI_BB_PLACE_SHIFT);
    uint32_t result = omi_bb_get_result(board) ^ (uint32_t)byte;
    board = (board & ~OMI_BB_RESULT_MASK) | ((uint64_t)result << OMI_BB_RESULT_SHIFT);
    return board;
}

uint64_t omi_bb_fold(uint64_t board) {
    uint32_t lo = (uint32_t)(board & 0xFFFFFFFF);
    uint32_t hi = (uint32_t)((board >> 32) & 0xFFFFFFFF);
    uint32_t folded32 = lo ^ hi;
    return (board & ~OMI_BB_FOLD_MASK) | ((uint64_t)folded32 << OMI_BB_FOLD_SHIFT);
}

static void omi_gauge_build_table(void) {
    if (gauge_table_initialized) return;
    for (int i = 0; i < 128; i++) {
        gauge_table[i].code = (uint8_t)i;
        gauge_table[i].cls = GAUGE_CLASS_CONTROL;
        gauge_table[i].diag = DIAG_NONE;
        gauge_table[i].action = GAUGE_ACTION_NONE;
        gauge_table[i].active = 0;
        memset(gauge_table[i].s, 0, sizeof(gauge_table[i].s));
        gauge_table[i].payload = 0;
        gauge_table[i].mask = 0;
        gauge_table[i].car = 0;
        gauge_table[i].cdr = 0;
        gauge_table[i].bridge = 0;
    }
    for (int i = 0; i < 32; i++) {
        gauge_table[i].cls = GAUGE_CLASS_CONTROL;
        gauge_table[i].action = GAUGE_ACTION_PLACE;
        gauge_table[i].active = 1;
    }
    for (int i = 0x20; i <= 0x2F; i++) {
        gauge_table[i].cls = GAUGE_CLASS_PRINTABLE;
        gauge_table[i].action = GAUGE_ACTION_REGISTER_INJECT;
        gauge_table[i].active = 1;
    }
    for (int i = 0x30; i <= 0x3F; i++) {
        gauge_table[i].cls = GAUGE_CLASS_PRINTABLE;
        gauge_table[i].action = GAUGE_ACTION_KERNEL_READ;
        gauge_table[i].active = 1;
    }
    gauge_table[0x1E].diag = DIAG_BALANCED;
    gauge_table[0x1E].action = GAUGE_ACTION_RECORD_CLOSE;
    gauge_table[0x1E].active = 1;
    gauge_table[0x78].action = GAUGE_ACTION_SYSTEM_WITNESS;
    gauge_table[0x78].active = 1;
    gauge_table[0x7F].action = GAUGE_ACTION_SEAL;
    gauge_table[0x7F].active = 1;
    gauge_table_initialized = 1;
}

const OmiGaugeEntry* omi_gauge_lookup(uint8_t code) {
    omi_gauge_build_table();
    if (code > 127) return NULL;
    return &gauge_table[code];
}

void omi_gauge_init(void) {
    omi_gauge_build_table();
}
