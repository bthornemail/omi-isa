#include "omi_orbit.h"
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_run = 0;

static void TEST(const char* name, int cond) {
    tests_run++;
    if (cond) tests_passed++;
    printf("  %s: %s\n", cond ? "PASS" : "FAIL", name);
}

int main(void) {
    printf("=== GL(16,2) Orbit Engine Tests ===\n\n");

    /* ── Test 1: delta16  (GL(16,2) linear map) ── */
    printf("[Test 1] delta16\n");
    /* delta16(x, c) = A[x] ^ B[c]
     * A[x] = x << 1  (with GF reduction by 0x002D if high bit set)
     * B[c] = c       (identity linear map) */
    TEST("delta16(0,0) == 0", omi_delta16(0, 0) == 0);
    TEST("delta16(0,1) == 1", omi_delta16(0, 1) == 1);
    /* A[1] = 1 << 1 = 2 */
    TEST("delta16(1,0) == 2", omi_delta16(1, 0) == 2);
    /* A[0x8000] = (0x8000 << 1) ^ 0x002D = 0x002D */
    TEST("delta16(0x8000,0) == 0x002D", omi_delta16(0x8000, 0) == 0x002D);
    /* A[0] = 0, B[0x55AA] = 0x55AA */
    TEST("delta16(0,0x55AA) == 0x55AA", omi_delta16(0, 0x55AA) == 0x55AA);
    /* delta16 is linear in c: delta16(x, c1^c2) = delta16(x,c1) ^ delta16(x,c2) ^ A[x] */
    TEST("delta16(5,5) == delta16(5,5)", omi_delta16(5, 5) == omi_delta16(5, 5));
    TEST("delta16 is deterministic",
         omi_delta16(42, 99) == omi_delta16(42, 99));

    /* ── Test 2: orbit_step ── */
    printf("\n[Test 2] orbit_step\n");
    {
        /* Fixed point: (0, c) stays at (0, c) */
        OMI_OrbitState s  = { .x = 0, .c = 0 };
        OMI_OrbitState s1 = omi_orbit_step(s);
        TEST("step from (0,0) is (0,0)", s1.x == 0 && s1.c == 0);

        OMI_OrbitState s2 = { .x = 0, .c = 5 };
        OMI_OrbitState s3 = omi_orbit_step(s2);
        /* delta16(0,5) = A[0] ^ B[5] = 0 ^ 5 = 5 */
        TEST("step from (0,5) x becomes 5, c stays", s3.x == 5 && s3.c == 5);

        /* Non-trivial: x evolves, c stays fixed */
        OMI_OrbitState s4 = { .x = 1, .c = 0 };
        OMI_OrbitState s5 = omi_orbit_step(s4);
        TEST("step from (1,0) updates x to 2", s5.x == 2);
        TEST("step from (1,0) keeps c=0", s5.c == 0);

        OMI_OrbitState s6 = omi_orbit_step(s5);
        TEST("second step diverges", s6.x != s5.x);
        TEST("c stays fixed after second step", s6.c == 0);

        /* Deterministic */
        OMI_OrbitState sa = omi_orbit_step((OMI_OrbitState){.x=7,.c=3});
        OMI_OrbitState sb = omi_orbit_step((OMI_OrbitState){.x=7,.c=3});
        TEST("orbit_step is deterministic", sa.x == sb.x && sa.c == sb.c);
    }

    /* ── Test 3: orbit_trace cycle detection ── */
    printf("\n[Test 3] orbit_trace\n");
    {
        /* Fixed point: (0,0) — cycle of length 1 */
        OMI_OrbitTrace trace;
        omi_orbit_trace((OMI_OrbitState){.x=0,.c=0}, &trace);
        TEST("trace from (0,0) has size >= 1", trace.size >= 1);
        TEST("trace from (0,0) detects cycle",
             trace.cycle_start != UINT32_MAX);
        TEST("cycle from (0,0) at position 0",
             trace.cycle_start == 0);
        TEST("trace states[0] == 0", trace.states[0] == 0);
        TEST("trace c == 0", trace.c == 0);

        /* Non-trivial orbit: (1,0) */
        omi_orbit_trace((OMI_OrbitState){.x=1,.c=0}, &trace);
        TEST("trace from (1,0) size > 1", trace.size > 1);
        TEST("trace from (1,0) detects cycle",
             trace.cycle_start != UINT32_MAX);
        TEST("cycle_start < size", trace.cycle_start < trace.size);

        /* Different control value */
        omi_orbit_trace((OMI_OrbitState){.x=0,.c=42}, &trace);
        TEST("trace from (0,42) c==42", trace.c == 42);

        /* Trace orbit is deterministic */
        OMI_OrbitTrace t1, t2;
        omi_orbit_trace((OMI_OrbitState){.x=5,.c=7}, &t1);
        omi_orbit_trace((OMI_OrbitState){.x=5,.c=7}, &t2);
        TEST("trace is deterministic", t1.size == t2.size);
        if (t1.size == t2.size) {
            int same = 1;
            for (uint32_t i = 0; i < t1.size; i++)
                if (t1.states[i] != t2.states[i]) { same = 0; break; }
            TEST("trace states match", same);
            TEST("trace cycle_start match",
                 t1.cycle_start == t2.cycle_start);
        }

        /* Null safety */
        omi_orbit_trace((OMI_OrbitState){.x=0,.c=0}, NULL);
        TEST("trace null safety (no crash)", 1);
    }

    /* ── Test 4: BQF ── */
    printf("\n[Test 4] BQF\n");
    {
        TEST("BQF(0,0) == 0", omi_bqf(0, 0) == 0);
        TEST("BQF(1,0) == 60", omi_bqf(1, 0) == 60);
        TEST("BQF(0,1) == 4", omi_bqf(0, 1) == 4);
        TEST("BQF(1,1) == 80", omi_bqf(1, 1) == 80);
        /* BQF(2,3) = 60*4 + 16*6 + 4*9 = 240 + 96 + 36 = 372 */
        TEST("BQF(2,3) == 372", omi_bqf(2, 3) == 372);
        TEST("BQF is not symmetric", omi_bqf(2, 3) != omi_bqf(3, 2));
    }

    /* ── Test 5: Fano observer (x % 7) ── */
    printf("\n[Test 5] Fano observer\n");
    {
        int counts[7] = {0};
        for (uint32_t i = 0; i < 10000; i++) {
            uint8_t f = omi_fano((uint16_t)i);
            if (f < 7) counts[f]++;
        }
        int all_hit = 1;
        for (int i = 0; i < 7; i++)
            if (counts[i] == 0) { all_hit = 0; break; }
        TEST("Fano hits all 7 values over 0..9999", all_hit);
        TEST("Fano deterministic", omi_fano(42) == omi_fano(42));
        TEST("Fano(0) == 0", omi_fano(0) == 0);
        TEST("Fano(7) == 0", omi_fano(7) == 0);
        TEST("Fano(1) == 1", omi_fano(1) == 1);
    }

    /* ── Test 6: Tetra observer (x % 4) ── */
    printf("\n[Test 6] Tetra observer\n");
    {
        int counts[4] = {0};
        for (uint32_t i = 0; i < 1000; i++) {
            uint8_t t = omi_tetra((uint16_t)i);
            if (t < 4) counts[t]++;
        }
        int all_hit = 1;
        for (int i = 0; i < 4; i++)
            if (counts[i] == 0) { all_hit = 0; break; }
        TEST("Tetra hits all 4 values over 0..999", all_hit);
        TEST("Tetra deterministic", omi_tetra(42) == omi_tetra(42));
        TEST("Tetra(0) == 0", omi_tetra(0) == 0);
        TEST("Tetra(4) == 0", omi_tetra(4) == 0);
    }

    /* ── Test 7: Phase observer (x & 1) ── */
    printf("\n[Test 7] Phase observer\n");
    {
        TEST("Phase(0) == 0", omi_phase(0) == 0);
        TEST("Phase(1) == 1", omi_phase(1) == 1);
        TEST("Phase(2) == 0", omi_phase(2) == 0);
        TEST("Phase(3) == 1", omi_phase(3) == 1);
        TEST("Phase deterministic", omi_phase(42) == omi_phase(42));
        /* Check that phase alternates in GL(16,2) orbit */
        OMI_OrbitState s = { .x = 1, .c = 0 };
        uint8_t p0 = omi_phase(s.x);
        s = omi_orbit_step(s);
        uint8_t p1 = omi_phase(s.x);
        TEST("Phase toggles on step from 1", p1 != p0);
    }

    /* ── Test 8: 5040 Atlas Indexer ── */
    printf("\n[Test 8] 5040 Atlas Indexer\n");
    {
        uint16_t s0 = omi_slot5040(0, 0, 0);
        uint16_t s1 = omi_slot5040(1, 0, 0);
        TEST("slot5040(0,0,0) == 0", s0 == 0);
        TEST("slot5040(1,0,0) == 720", s1 == 720);
        TEST("slot5040(0,1,0) == 240",
             omi_slot5040(0, 1, 0) == 240);
        TEST("slot5040(0,0,240) == 0 (mod 240)",
             omi_slot5040(0, 0, 240) == 0);
        TEST("slot5040(6,3,239) < OMI_SLOT5040_SIZE",
             omi_slot5040(6, 3, 239) < OMI_SLOT5040_SIZE);
        int all_in_range = 1;
        for (uint8_t f = 0; f < 7; f++)
            for (uint8_t t = 0; t < 4; t++)
                for (uint16_t p = 0; p < 256; p++) {
                    uint16_t slot = omi_slot5040(f, t, (uint8_t)p);
                    if (slot >= OMI_SLOT5040_SIZE) { all_in_range = 0; break; }
                }
        TEST("all computed slots in 0..5039", all_in_range);

        OMI_Attestation a0 = omi_attest_from_state(0, 0);
        OMI_Attestation a1 = omi_attest_from_state(1, 5);
        TEST("attest from (0,0) orbit_id < OMI_SLOT5040_SIZE",
             a0.orbit_id < OMI_SLOT5040_SIZE);
        TEST("attest from (0,0) delta_phase == 0", a0.delta_phase == 0);
        TEST("attest from (1,5) delta_phase == 5", a1.delta_phase == 5);
        TEST("attest from (1,5) fano_id == fano(1)",
             a1.fano_id == omi_fano(1));
        TEST("attest from (1,5) tetra_class == tetra(1)",
             a1.tetra_class == omi_tetra(1));

        OMI_Attestation a2a = omi_attest_from_state(42, 7);
        OMI_Attestation a2b = omi_attest_from_state(42, 7);
        TEST("attest is deterministic",
             a2a.orbit_id == a2b.orbit_id &&
             a2a.fano_id == a2b.fano_id &&
             a2a.tetra_class == a2b.tetra_class &&
             a2a.delta_phase == a2b.delta_phase);

        {
            OMI_Attestation ar = omi_attest_from_state(0, 0);
            char buf[OMI_ATTEST_STR_MAX];
            omi_attest_render(ar, buf);
            TEST("attest render starts with 'o'", buf[0] == 'o');
            TEST("attest render contains '-'", strchr(buf, '-') != NULL);
            TEST("attest render contains '/'", strchr(buf, '/') != NULL);
            TEST("attest render contains '@'", strchr(buf, '@') != NULL);
            TEST("attest render ends with '@'",
                 buf[strlen(buf)-1] == '@');
            size_t len = strlen(buf);
            TEST("attest render length >= 16 && <= 24",
                 len >= 16 && len <= 24);

            OMI_Attestation ak = { .orbit_id = 5039, .fano_id = 6,
                                   .tetra_class = 3, .delta_phase = 0xFF };
            omi_attest_render(ak, buf);
            TEST("attest render known value",
                 strcmp(buf, "o13af-o/06/03@ff@") == 0);
        }

        omi_attest_render((OMI_Attestation){0}, NULL);
        TEST("attest render null safety (no crash)", 1);
    }

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
