#ifndef OMI_PG_H
#define OMI_PG_H

#include "omienv.h"
#include <stdint.h>

/* ── PG(4,2) Incidence Kernel ──
 *
 * PG(4,2) = projective geometry over GF(2)^5
 * 31 points = GF(2)^5 \ {0}
 * 155 lines  = 3-point incidence classes
 *
 * The incidence kernel π(G) is a 128-bit value stored in
 * the envelope's relation[16..31] (the cross-reference tail).
 * It is computed from active gauge points and their line closure.
 *
 * Δ : 16-bit rotation-XOR endomorphism (from omi_sense delta16)
 * F(x) = Δ(x, C) ⊕ x   — non-zero-divisor filter
 * T    = CAR/CDR forbidden 2-cycle detection
 */

#define OMI_PG_POINTS          31
#define OMI_PG_KERNEL_WORDS    4
#define OMI_PG_CROSS_REF_OFFSET 16
#define OMI_PG_CROSS_REF_BYTES  (OMI_PG_KERNEL_WORDS * 4)

/* ── Point/line encoding ── */

/* Encode a single PG(4,2) point (1..31) into its 32-bit seed word.
 * point=0 returns 0 (zero vector, not a PG point). */
uint32_t omi_pg_point_seed(uint8_t point);

/* Compute the 128-bit line contribution for point p:
 *   contribution = rotl(seed(p), wt(p)) ⊕ basis_mask(p)
 * Returns the word selected by bits [4..3] of p.
 * All 4 output words are initially 0; only the selected word is set. */
void omi_pg_point_encode(uint8_t point, uint32_t out[4]);

/* ── Kernel computation ──
 *
 * The kernel is the XOR of encode(ℓ) for all lines ℓ fully
 * contained in the active point set S.
 *
 * A line ℓ = {p, q, p⊕q} for p,q ∈ S, p≠q.
 * encode(ℓ) = encode(p) ⊕ encode(q) ⊕ encode(p⊕q).
 *
 * The gauge[0..7] bytes define S: gauge[i] & 0x1F selects a point
 * (1..31). A value of 0 means inactive. */

/* Compute π(S): 128-bit incidence kernel from gauge state.
 * Stores result into kernel[4]. */
void omi_pg_kernel_from_gauge(const uint8_t gauge[8], uint32_t kernel[4]);

/* Compute kernel from full envelope (reads gauge).
 * Shorthand for omi_pg_kernel_from_gauge(env->gauge, kernel). */
void omi_pg_kernel(const OMI_512_Envelope* env, uint32_t kernel[4]);

/* Read/write kernel to/from envelope's cross-reference tail. */
void omi_pg_kernel_to_env(const uint32_t kernel[4], OMI_512_Envelope* env);
void omi_pg_kernel_from_env(const OMI_512_Envelope* env, uint32_t kernel[4]);

/* ── Δ non-zero-divisor filter ──
 *
 * F(x) = Δ(x, C) ⊕ x   (using omi_sense_delta16 on each 16-bit subblock)
 *
 * A 16-bit value is safe if:
 *   F(x) ≠ 0  AND  F(F(x)) ≠ x
 * i.e. it is not a fixed point (period 1) nor a 2-cycle of Δ.
 *
 * A 128-bit kernel is safe if NO 16-bit subblock is unsafe. */

int omi_pg_subblock_safe(uint16_t x);
int omi_pg_kernel_safe(const uint32_t kernel[4]);

/* ── CAR/CDR forbidden transition check ──
 *
 * A transition CAR → CDR is invalid (forbidden) if:
 *   For any 16-bit subblock pair (car_i, cdr_i):
 *     Δ(car_i) = cdr_i AND Δ(cdr_i) = car_i
 *   This is a 2-cycle collapse condition.
 *
 * Returns 0 if allowed, <0 if forbidden. */

int omi_pg_transition_allowed(uint32_t car, uint32_t cdr);

/* ── Full envelope validation ──
 *
 * Checks:
 *   1. Kernel in relation[16..31] is Δ-safe
 *   2. CAR → CDR transition is not forbidden
 *   3. Kernel is consistent with gauge state
 *
 * CAR  = env->target[4..7]  (read as LE32)
 * CDR  = env->relation[0..3] (read as LE32)
 * gauge[0..7] = env->gauge
 * kernel stored in env->relation[16..31] */

int omi_pg_validate_envelope(const OMI_512_Envelope* env);

#endif
