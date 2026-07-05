#include "omi_pg.h"
#include <string.h>

/* ── Bitwise helpers ── */

static uint16_t rotl16(uint16_t x, unsigned n) {
    n &= 15u;
    return (uint16_t)((x << n) | (x >> (16u - n)));
}

static uint16_t rotr16(uint16_t x, unsigned n) {
    n &= 15u;
    return (uint16_t)((x >> n) | (x << (16u - n)));
}

static uint32_t rotl32(uint32_t x, unsigned n) {
    n &= 31u;
    return (x << n) | (x >> (32u - n));
}

/* Δ(x, C) — matches omi_sense_delta16 exactly */
static uint16_t delta16(uint16_t x, uint16_t C) {
    return (uint16_t)(rotl16(x, 1) ^ rotl16(x, 3) ^ rotr16(x, 2) ^ C);
}

static int popcount5(uint8_t x) {
    x &= 0x1F;
    return (int)__builtin_popcount((unsigned int)x);
}

/* ── PG(4,2) constants ── */

#define PG_SEED_PRIME 0x9E3779B9u

/* ── Point encoding ── */

uint32_t omi_pg_point_seed(uint8_t point) {
    if (point == 0 || point > OMI_PG_POINTS) return 0;
    return (uint32_t)point * PG_SEED_PRIME;
}

void omi_pg_point_encode(uint8_t point, uint32_t out[4]) {
    memset(out, 0, (size_t)OMI_PG_KERNEL_WORDS * sizeof(uint32_t));
    if (point == 0 || point > OMI_PG_POINTS) return;

    uint32_t seed = omi_pg_point_seed(point);
    int wt = popcount5(point);
    uint32_t rotated = rotl32(seed, (unsigned)wt);

    int word_idx = ((int)point >> 3) & 3;
    uint32_t basis_bit = 1u << (uint32_t)(point & 31);
    out[word_idx] = rotated ^ basis_bit;
}

/* ── Kernel computation ── */

void omi_pg_kernel_from_gauge(const uint8_t gauge[8], uint32_t kernel[4]) {
    if (!gauge || !kernel) return;
    memset(kernel, 0, (size_t)OMI_PG_KERNEL_WORDS * sizeof(uint32_t));

    /* Collect active points from gauge */
    uint8_t active[OMI_PG_POINTS + 1];
    memset(active, 0, sizeof(active));
    for (int i = 0; i < 8; i++) {
        uint8_t p = gauge[i] & 0x1F;
        if (p != 0) active[p] = 1;
    }

    /* Count active points for early exit */
    int count = 0;
    for (int p = 1; p <= OMI_PG_POINTS; p++)
        if (active[p]) count++;
    if (count < 3) return;  /* no lines possible with < 3 points */

    /* Precompute point encodings */
    uint32_t enc[OMI_PG_POINTS + 1][OMI_PG_KERNEL_WORDS];
    for (int p = 1; p <= OMI_PG_POINTS; p++)
        omi_pg_point_encode((uint8_t)p, enc[p]);

    /* XOR all lines ℓ = {p,q,p⊕q} fully contained in active set.
     * Each line is counted C(3,2)=3 times by iterating all pairs,
     * but XOR in GF(2) makes 3× same as 1×. */
    for (int p = 1; p <= OMI_PG_POINTS; p++) {
        if (!active[p]) continue;
        for (int q = p + 1; q <= OMI_PG_POINTS; q++) {
            if (!active[q]) continue;
            uint8_t r = (uint8_t)(p ^ q);
            if (r == 0 || !active[r]) continue;
            for (int i = 0; i < OMI_PG_KERNEL_WORDS; i++)
                kernel[i] ^= enc[p][i] ^ enc[q][i] ^ enc[r][i];
        }
    }
}

void omi_pg_kernel(const OMI_512_Envelope* env, uint32_t kernel[4]) {
    if (!env || !kernel) return;
    omi_pg_kernel_from_gauge(env->gauge, kernel);
}

void omi_pg_kernel_to_env(const uint32_t kernel[4], OMI_512_Envelope* env) {
    if (!kernel || !env) return;
    for (int i = 0; i < OMI_PG_KERNEL_WORDS; i++) {
        int off = OMI_PG_CROSS_REF_OFFSET + i * 4;
        env->relation[off + 0] = (uint8_t)(kernel[i] & 0xFF);
        env->relation[off + 1] = (uint8_t)((kernel[i] >> 8) & 0xFF);
        env->relation[off + 2] = (uint8_t)((kernel[i] >> 16) & 0xFF);
        env->relation[off + 3] = (uint8_t)((kernel[i] >> 24) & 0xFF);
    }
}

void omi_pg_kernel_from_env(const OMI_512_Envelope* env, uint32_t kernel[4]) {
    if (!env || !kernel) return;
    for (int i = 0; i < OMI_PG_KERNEL_WORDS; i++) {
        int off = OMI_PG_CROSS_REF_OFFSET + i * 4;
        kernel[i] = (uint32_t)env->relation[off]
                  | ((uint32_t)env->relation[off + 1] << 8)
                  | ((uint32_t)env->relation[off + 2] << 16)
                  | ((uint32_t)env->relation[off + 3] << 24);
    }
}

/* ── Δ non-zero-divisor filter ──
 *
 * F(x) = Δ(x, C) ⊕ x
 * x safe iff F(x) ≠ 0 AND F(F(x)) ≠ x
 *
 * Uses C = 0x55AA as the delta rotation constant.
 */

#define PG_DELTA_C 0x55AAu

static uint16_t F(uint16_t x) {
    return delta16(x, PG_DELTA_C) ^ x;
}

int omi_pg_subblock_safe(uint16_t x) {
    uint16_t fx = F(x);
    if (fx == 0) return 0;   /* fixed point (period 1) */
    if (F(fx) == x) return 0; /* 2-cycle collapse */
    return 1;
}

int omi_pg_kernel_safe(const uint32_t kernel[4]) {
    if (!kernel) return 0;
    for (int i = 0; i < OMI_PG_KERNEL_WORDS; i++) {
        uint16_t lo = (uint16_t)(kernel[i] & 0xFFFF);
        uint16_t hi = (uint16_t)((kernel[i] >> 16) & 0xFFFF);
        if (!omi_pg_subblock_safe(lo)) return 0;
        if (!omi_pg_subblock_safe(hi)) return 0;
    }
    return 1;  /* all 8 subblocks safe */
}

/* ── CAR/CDR forbidden transition check ──
 *
 * Forbidden if any 16-bit subblock pair forms a Δ 2-cycle:
 *   Δ(car_i) = cdr_i AND Δ(cdr_i) = car_i
 */

int omi_pg_transition_allowed(uint32_t car, uint32_t cdr) {
    uint16_t car_lo = (uint16_t)(car & 0xFFFF);
    uint16_t car_hi = (uint16_t)((car >> 16) & 0xFFFF);
    uint16_t cdr_lo = (uint16_t)(cdr & 0xFFFF);
    uint16_t cdr_hi = (uint16_t)((cdr >> 16) & 0xFFFF);

    /* Check upper 16-bit half */
    if (delta16(car_hi, PG_DELTA_C) == cdr_hi &&
        delta16(cdr_hi, PG_DELTA_C) == car_hi)
        return -1; /* forbidden 2-cycle in upper half */

    /* Check lower 16-bit half */
    if (delta16(car_lo, PG_DELTA_C) == cdr_lo &&
        delta16(cdr_lo, PG_DELTA_C) == car_lo)
        return -2; /* forbidden 2-cycle in lower half */

    return 0; /* transition allowed */
}

/* ── Full envelope validation ── */

static uint32_t read_le32_env(const uint8_t* base) {
    return (uint32_t)base[0]
         | ((uint32_t)base[1] << 8)
         | ((uint32_t)base[2] << 16)
         | ((uint32_t)base[3] << 24);
}

int omi_pg_validate_envelope(const OMI_512_Envelope* env) {
    if (!env) return -1;

    uint32_t kernel[OMI_PG_KERNEL_WORDS];
    omi_pg_kernel(env, kernel);

    /* 1. Kernel must be Δ-safe */
    if (!omi_pg_kernel_safe(kernel)) return -2;

    /* 2. Kernel must match stored cross-reference tail */
    uint32_t stored[OMI_PG_KERNEL_WORDS];
    omi_pg_kernel_from_env(env, stored);
    for (int i = 0; i < OMI_PG_KERNEL_WORDS; i++)
        if (kernel[i] != stored[i]) return -3;

    /* 3. CAR → CDR transition must not be forbidden */
    uint32_t car = read_le32_env(env->target + 4);
    uint32_t cdr = read_le32_env(env->relation);
    if (omi_pg_transition_allowed(car, cdr) != 0) return -4;

    return 0; /* envelope is projectively consistent */
}
