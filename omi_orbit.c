#include "omi_orbit.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ── 16-bit rotations ── */
static inline uint16_t rotl16(uint16_t x, unsigned n) {
    n &= 15;
    return (uint16_t)((x << n) | (x >> (16 - n)));
}

static inline uint16_t rotr16(uint16_t x, unsigned n) {
    n &= 15;
    return (uint16_t)((x >> n) | (x << (16 - n)));
}

/* ── delta16: fundamental endomorphism generator ── */
uint16_t omi_delta16(uint16_t x, uint16_t c) {
    return rotl16(x, 1) ^ rotl16(x, 3) ^ rotr16(x, 2) ^ c;
}

/* ── Single orbit step with seed evolution ── */
OMI_OrbitState omi_orbit_step(OMI_OrbitState s) {
    uint16_t d = omi_delta16(s.x, s.seed);
    OMI_OrbitState ns;
    ns.x    = d;
    ns.seed = (uint16_t)(s.seed ^ (uint32_t)(d * 31));
    return ns;
}

/* ── Orbit tracing with cycle detection ── */
void omi_orbit_trace(OMI_OrbitState initial, OMI_OrbitTrace* trace) {
    if (!trace) return;

    trace->size        = 0;
    trace->cycle_start = UINT32_MAX;

    /* Visited array: visited[state] = trace_position + 1 (0 = not visited) */
    uint16_t* visited = (uint16_t*)calloc(65536, sizeof(uint16_t));
    if (!visited) return;

    OMI_OrbitState s = initial;

    while (trace->size < OMI_TRACE_MAX) {
        uint32_t pos = trace->size;
        trace->states[pos] = s.x;
        trace->seeds[pos]  = s.seed;
        trace->size++;

        /* Mark visited (1-indexed position + 1 so visited[0] = 0 means unvisited) */
        visited[s.x] = (uint16_t)(pos + 1);

        /* Step forward */
        s = omi_orbit_step(s);

        /* Check if we've seen this state before */
        if (visited[s.x] != 0) {
            trace->cycle_start = visited[s.x] - 1;
            break;
        }
    }

    free(visited);
}

/* ── BQF: orbit acceptance metric ── */
int64_t omi_bqf(int64_t x, int64_t y) {
    return 60 * x * x + 16 * x * y + 4 * y * y;
}

/* ── Fano lottery projection ── */
uint8_t omi_fano_lottery(uint16_t x) {
    uint32_t t = x;
    return (uint8_t)((t * 7) ^ (t >> 3) ^ (t * 13)) & 0x07;
}

/* ── Tetrahedral role ── */
uint8_t omi_tetra_role(uint16_t x) {
    return (uint8_t)((x ^ (x >> 1) ^ (x >> 2)) & 0x03);
}

/* ── Triple detection ── */
int omi_is_orbit_triangle(uint16_t a, uint16_t b, uint16_t c,
                          uint16_t seed, OMI_OrbitTriple* out) {
    /* 1. δ-closure: δ(a) ⊕ δ(b) ⊕ δ(c) = seed */
    uint16_t da = omi_delta16(a, seed);
    uint16_t db = omi_delta16(b, seed);
    uint16_t dc = omi_delta16(c, seed);
    if ((da ^ db ^ dc) != seed) return 0;

    /* 2. BQF conservation bound */
    int64_t energy = omi_bqf(a, b) + omi_bqf(b, c) + omi_bqf(c, a);
    if (energy > OMI_BQF_THRESHOLD) return 0;

    /* 3. Fano line consistency: all three map to distinct Fano points
     *    whose XOR is 0 (forming a Fano line) */
    uint8_t fa = omi_fano_lottery(a);
    uint8_t fb = omi_fano_lottery(b);
    uint8_t fc = omi_fano_lottery(c);
    if (fa == fb || fb == fc || fc == fa) return 0;
    if ((fa ^ fb ^ fc) != 0) return 0;

    if (out) {
        out->a = a;
        out->b = b;
        out->c = c;
        out->orbit_id    = 0;
        out->fano_line   = fa ^ fb;  /* = fc since fa^fb^fc=0 */
        out->tetra_class = omi_tetra_role(a);
        out->energy      = (int32_t)energy;
    }
    return 1;
}

/* ── Seed lookup in trace ── */
uint16_t omi_orbit_seed_at(const OMI_OrbitTrace* trace, uint32_t idx) {
    if (!trace || idx >= trace->size) return 0;
    return trace->seeds[idx];
}

/* ── Quotient projection π: orbit state → nibble index (0..15) ── */
uint8_t omi_project_31_to_16(uint16_t x) {
    uint8_t lo = (uint8_t)(x & 0x0F);
    if (x < 16) return lo;
    return (uint8_t)(lo ^ ((x & 0xF0) >> 4));
}

/* ── 5040 slot computation ── */
uint16_t omi_slot5040(uint8_t fano_id, uint8_t tetra_class, uint8_t delta_phase) {
    return (uint16_t)(
        ((uint32_t)fano_id * 720) +
        ((uint32_t)tetra_class * 240) +
        (delta_phase % 240)
    ) % OMI_SLOT5040_SIZE;
}

/* ── Create attestation from orbit state + trace position ── */
OMI_Attestation omi_attest_from_state(uint16_t x, uint32_t trace_position) {
    OMI_Attestation a;
    a.fano_id      = omi_fano_lottery(x);
    a.tetra_class  = omi_tetra_role(x);
    a.delta_phase  = (uint8_t)(trace_position & 0xFF);
    a.orbit_id     = omi_slot5040(a.fano_id, a.tetra_class, (uint8_t)(trace_position % 240));
    return a;
}

/* ── Render attestation as 4-phase boundary string ── */
void omi_attest_render(OMI_Attestation a, char* out) {
    if (!out) return;
    sprintf(out, "o%04x-o/%02x/%02x@%02x@",
            a.orbit_id, a.fano_id, a.tetra_class, a.delta_phase);
}
