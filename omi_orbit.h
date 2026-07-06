#ifndef OMI_ORBIT_H
#define OMI_ORBIT_H

#include <stdint.h>
#include <stddef.h>

/* ── Core orbit engine constants ── */
#define OMI_TRACE_MAX 65536
#define OMI_BQF_THRESHOLD 10000000000LL  /* placeholder — needs calibration */

/*
 * Architecture:
 *   delta16 : uint16_t × uint16_t → uint16_t   (endomorphism generator)
 *   orbit_step : OrbitState → OrbitState        (seed-evolved trajectory)
 *   orbit_trace : OrbitState × trace            (cycle detection)
 *
 * Observation layers on top of orbit engine:
 *   BQF         — orbit acceptance metric (energy)
 *   Fano lottery — maps state → Fano index (0..6)
 *   Tetra role   — maps state → tetra class (0..3)
 *   Triple detection — δ-closure + BQF + Fano consistency
 *
 * 31→16 quotient: derived from orbit equivalence classes,
 * NOT from direct state masking.
 */

/* ── Orbit state (the only stateful object) ── */
typedef struct {
    uint16_t x;        /* current state */
    uint16_t seed;     /* evolving seed driving delta16 */
} OMI_OrbitState;

/* ── Orbit trace (visited states with cycle detection) ── */
typedef struct {
    uint16_t states[OMI_TRACE_MAX];
    uint16_t seeds[OMI_TRACE_MAX];
    uint32_t size;
    uint32_t cycle_start;  /* index where cycle begins, or ~0 if no cycle found */
} OMI_OrbitTrace;

/* ── Observed triple (emitted by triple detection, NOT stored structure) ── */
typedef struct {
    uint16_t a, b, c;
    uint16_t orbit_id;
    uint8_t  fano_line;
    uint8_t  tetra_class;
    int32_t  energy;    /* Σ BQF */
} OMI_OrbitTriple;

/* ── Fundamental endomorphism generator ──
 * delta16(x, c) = rotl16(x,1) ^ rotl16(x,3) ^ rotr16(x,2) ^ c
 * This is the ONLY primitive. All structure derives from it. */
uint16_t omi_delta16(uint16_t x, uint16_t c);

/* ── Single orbit step with seed evolution ──
 *   x'     = delta16(x, seed)
 *   seed'  = seed ^ (x' * 31)           */
OMI_OrbitState omi_orbit_step(OMI_OrbitState s);

/* ── Orbit tracing ──
 * Applies orbit_step repeatedly, recording states until a cycle
 * is detected (visited state reappears) or OMI_TRACE_MAX reached. */
void omi_orbit_trace(OMI_OrbitState initial, OMI_OrbitTrace* trace);

/* ── BQF: orbit acceptance metric (energy) ──
 * BQF(x, y) = 60x² + 16xy + 4y²
 * Energy over orbit transitions. Uses int64_t because uint16_t² exceeds INT32_MAX. */
int64_t omi_bqf(int64_t x, int64_t y);

/* ── Fano lottery projection ──
 * Maps state → Fano incidence node (0..6).
 * fano(x) = ((x * 7) ^ (x >> 3) ^ (x * 13)) & 0x07 */
uint8_t omi_fano_lottery(uint16_t x);

/* ── Tetrahedral role ──
 * Maps state → vertex symmetry class (0..3).
 * tetra(x) = ((x ^ (x >> 1) ^ (x >> 2)) & 0x03) */
uint8_t omi_tetra_role(uint16_t x);

/* ── Triple detection ──
 * Checks if (a,b,c) forms an orbit recurrence triangle:
 *   1. δ-closure:   delta16(a,seed) ^ delta16(b,seed) ^ delta16(c,seed) = seed
 *   2. BQF bound:   Σ BQF ≤ OMI_BQF_THRESHOLD
 *   3. Fano line:   fano(a), fano(b), fano(c) form a Fano line (XOR=0)
 *
 * Returns 1 if accepted, 0 otherwise. If accepted, fills *out. */
int omi_is_orbit_triangle(uint16_t a, uint16_t b, uint16_t c,
                          uint16_t seed, OMI_OrbitTriple* out);

/* ── Orbit seed lookup: find seed used at a given trace position ──
 * Returns seed at position idx, or 0 if idx >= trace->size. */
uint16_t omi_orbit_seed_at(const OMI_OrbitTrace* trace, uint32_t idx);

/* ── Quotient projection π : orbit state → nibble index (0..15) ──
 * Collapses orbit state to nibble core via:
 *   π(x) = (x & 0x0F) ^ ((x >= 16) ? (x & 0xF0) >> 4 : 0)
 * This maps the orbit space to the 16-nibble manifold. */
uint8_t omi_project_31_to_16(uint16_t x);

/* ══════════════════════════════════════════════════════════════
 * 5040 Atlas Indexer (orbital query attestation layer)
 * ══════════════════════════════════════════════════════════════
 *
 *  5040 = Fano(7) × tetra(24) × role(30)
 *       = quotient atlas of all admissible δ₁₆ + BQF + Fano projections
 *
 *  Each gauge citation produces one attestation:
 *    o{orbit_id}-o/{fano_id}/{tetra_class}@{delta_phase}@
 * ══════════════════════════════════════════════════════════════ */

#define OMI_SLOT5040_SIZE 5040
#define OMI_ATTEST_STR_MAX 32

/* ── Attestation: 5040-indexed orbit observation ── */
typedef struct {
    uint16_t orbit_id;     /* 0..5039 — slot5040 result (quotient index) */
    uint8_t  fano_id;      /* 0..6     — Fano incidence node */
    uint8_t  tetra_class;  /* 0..3     — tetrahedral vertex class */
    uint8_t  delta_phase;  /* 0..255   — trace position at observation */
} OMI_Attestation;

/* ── Compute 5040 slot from (fano, tetra, delta_phase) ──
 *   slot = ((fano * 720) + (tetra * 240) + (delta_phase % 240)) % 5040 */
uint16_t omi_slot5040(uint8_t fano_id, uint8_t tetra_class, uint8_t delta_phase);

/* ── Create attestation from orbit state and trace position ──
 *   fano_id, tetra_class extracted from state.x;
 *   delta_phase = position % 256;
 *   orbit_id    = omi_slot5040(fano_id, tetra_class, position % 240); */
OMI_Attestation omi_attest_from_state(uint16_t x, uint32_t trace_position);

/* ── Render attestation as 4-phase boundary grammar ──
 *   Format: "o{orbit_id:04x}-o/{fano_id:02x}/{tetra_class:02x}@{delta_phase:02x}@"
 *   out must be at least OMI_ATTEST_STR_MAX bytes. */
void omi_attest_render(OMI_Attestation a, char* out);

#endif
