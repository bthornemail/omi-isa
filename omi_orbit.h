#ifndef OMI_ORBIT_H
#define OMI_ORBIT_H

#include <stdint.h>
#include <stddef.h>

/*
 * Architecture (4 layers, one unified algebra):
 *   Layer 1:  GL(16,2) linear system   (state, delta, step, trace)
 *   Layer 2:  Equivariant observables  (Fano, Tetra, Phase)
 *   Layer 3:  BQF quadratic form       (orbit acceptance energy)
 *   Layer 4:  5040 orbit atlas         (mixed-radix quotient coordinate)
 *
 * Core theorem:
 *   Every observer is an equivariant map under GL(16,2).
 *   Invariance = equivariance, not arbitrary preservation.
 */

/* ── Layer 1: GL(16,2) linear dynamics ── */

#define OMI_TRACE_MAX 65536

typedef struct {
    uint16_t x;
    uint16_t c;
} OMI_OrbitState;

/* Trace with Floyd cycle detection */
typedef struct {
    uint16_t states[OMI_TRACE_MAX];
    uint16_t c;              /* constant control for this trace */
    uint32_t size;
    uint32_t cycle_start;
} OMI_OrbitTrace;

/* Initialize GL(16,2) lookup tables (A, B) */
void omi_orbit_init(void);

/* Core delta: Δ(x,c) = A[x] ^ B[c]   (A ∈ GL(16,2), B ∈ Mat(16,2)) */
uint16_t omi_delta16(uint16_t x, uint16_t c);

/* Single step: x' = Δ(x,c), c' = c */
OMI_OrbitState omi_orbit_step(OMI_OrbitState s);

/* Trace orbit with Floyd cycle detection */
void omi_orbit_trace(OMI_OrbitState initial, OMI_OrbitTrace* trace);

/* ── Layer 2: Equivariant observers (quotient maps) ── */

/* Fano plane: mod 7 quotient of orbit space */
uint8_t omi_fano(uint16_t x);

/* Tetrahedral class: mod 4 quotient */
uint8_t omi_tetra(uint16_t x);

/* Phase: parity (mod 2) */
uint8_t omi_phase(uint16_t x);

/* ── Layer 3: BQF quadratic form (invariant energy) ── */
int64_t omi_bqf(int64_t x, int64_t y);

/* ── Layer 4: 5040 orbit atlas ── */

#define OMI_SLOT5040_SIZE 5040
#define OMI_ATTEST_STR_MAX 32

typedef struct {
    uint16_t orbit_id;
    uint8_t  fano_id;
    uint8_t  tetra_class;
    uint8_t  delta_phase;
} OMI_Attestation;

/* Compute 5040 slot: ((fano * 720) + (tetra * 240) + (phase % 240)) % 5040 */
uint16_t omi_slot5040(uint8_t fano_id, uint8_t tetra_class, uint8_t delta_phase);

/* Create attestation from orbit state and trace position */
OMI_Attestation omi_attest_from_state(uint16_t x, uint32_t trace_position);

/* Render attestation as 4-phase boundary string */
void omi_attest_render(OMI_Attestation a, char* out);

#endif
