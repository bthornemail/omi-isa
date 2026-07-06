#include "omi_orbit.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ================================================================== */
/*  Layer 1:  GL(16,2) LINEAR DYNAMICS                                */
/* ================================================================== */

/*
 * GL(16,2) element via GF(2^16) multiplication by primitive element α.
 *
 * Field: GF(2^16) defined by primitive polynomial
 *   x^16 + x^5 + x^3 + x^2 + 1   (0x1002D, canonical: 0x002D)
 *
 * A : x ↦ α·x  (multiplication by α in GF(2^16))
 *     This is an F₂-linear permutation → A ∈ GL(16,2).
 *
 * B : x ↦ x    (identity; can be replaced with any linear map)
 */

static uint16_t A[65536];
static uint16_t B[65536];
static int tables_initialized = 0;

/* GF(2^16) multiply by α (primitive element) */
static inline uint16_t gf16_mul_alpha(uint16_t x) {
    /* left shift; if high bit was set, reduce by polynomial */
    uint16_t result = x << 1;
    if (x & 0x8000)
        result ^= 0x002D;   /* x^16 + x^5 + x^3 + x^2 + 1 (less x^16) */
    return result;
}

/* Precompute A and B tables */
void omi_orbit_init(void) {
    if (tables_initialized) return;

    /* A: multiplication by α  (bijection, GL(16,2)) */
    for (uint32_t i = 0; i < 65536; i++)
        A[i] = gf16_mul_alpha((uint16_t)i);

    /* B: identity (can be replaced with any linear map) */
    for (uint32_t i = 0; i < 65536; i++)
        B[i] = (uint16_t)i;

    tables_initialized = 1;
}

/* Core delta: GL(16,2) linear operator */
uint16_t omi_delta16(uint16_t x, uint16_t c) {
    if (!tables_initialized) omi_orbit_init();
    return A[x] ^ B[c];
}

/* Single step: x' = Δ(x,c), c' = c */
OMI_OrbitState omi_orbit_step(OMI_OrbitState s) {
    OMI_OrbitState ns;
    ns.x = omi_delta16(s.x, s.c);
    ns.c = s.c;
    return ns;
}

/* Trace orbit with visited-array cycle detection.
 * Since c is constant for a given trace, x alone determines trajectory,
 * so a 16-bit visited array (64 KB) correctly detects repeats. */
void omi_orbit_trace(OMI_OrbitState initial, OMI_OrbitTrace* trace) {
    if (!trace) return;

    trace->size        = 0;
    trace->cycle_start = UINT32_MAX;
    trace->c           = initial.c;

    uint16_t* visited = (uint16_t*)calloc(65536, sizeof(uint16_t));
    if (!visited) return;

    OMI_OrbitState s = initial;

    while (trace->size < OMI_TRACE_MAX) {
        uint32_t pos = trace->size;
        trace->states[pos] = s.x;
        trace->size++;

        visited[s.x] = (uint16_t)(pos + 1);
        s = omi_orbit_step(s);

        if (visited[s.x] != 0) {
            trace->cycle_start = visited[s.x] - 1;
            break;
        }
    }

    free(visited);
}

/* ================================================================== */
/*  Layer 2:  EQUIVARIANT OBSERVERS  (quotient maps over GL(16,2))     */
/* ================================================================== */

/* Fano plane: quotient Z_n → Z_7 via mod 7.
 * Under GL(16,2) action, mod-7 classes are permuted as a group. */
uint8_t omi_fano(uint16_t x) {
    return (uint8_t)(x % 7);
}

/* Tetrahedral class: quotient Z_n → Z_4 via mod 4. */
uint8_t omi_tetra(uint16_t x) {
    return (uint8_t)(x % 4);
}

/* Phase: parity (mod 2) — linear functional over F₂. */
uint8_t omi_phase(uint16_t x) {
    return (uint8_t)(x & 1);
}

/* ================================================================== */
/*  Layer 3:  BQF QUADRATIC FORM  (invariant energy)                   */
/* ================================================================== */

int64_t omi_bqf(int64_t x, int64_t y) {
    return 60 * x * x + 16 * x * y + 4 * y * y;
}

/* ================================================================== */
/*  Layer 4:  5040 ORBIT ATLAS  (mixed-radix quotient coordinates)     */
/* ================================================================== */

uint16_t omi_slot5040(uint8_t fano_id, uint8_t tetra_class, uint8_t delta_phase) {
    return (uint16_t)(
        ((uint32_t)fano_id * 720) +
        ((uint32_t)tetra_class * 240) +
        (delta_phase % 240)
    ) % OMI_SLOT5040_SIZE;
}

OMI_Attestation omi_attest_from_state(uint16_t x, uint32_t trace_position) {
    OMI_Attestation a;
    a.fano_id      = omi_fano(x);
    a.tetra_class  = omi_tetra(x);
    a.delta_phase  = (uint8_t)(trace_position & 0xFF);
    a.orbit_id     = omi_slot5040(a.fano_id, a.tetra_class,
                                  (uint8_t)(trace_position % 240));
    return a;
}

void omi_attest_render(OMI_Attestation a, char* out) {
    if (!out) return;
    sprintf(out, "o%04x-o/%02x/%02x@%02x@",
            a.orbit_id, a.fano_id, a.tetra_class, a.delta_phase);
}
