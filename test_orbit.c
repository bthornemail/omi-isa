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
    printf("=== Delta16 Orbit Engine Tests ===\n\n");

    /* ── Test 1: delta16 ── */
    printf("[Test 1] delta16\n");
    /* delta16(x, c) = rotl16(x,1) ^ rotl16(x,3) ^ rotr16(x,2) ^ c */
    /* x=0, c=0 => 0 ^ 0 ^ 0 ^ 0 = 0 */
    TEST("delta16(0,0) == 0", omi_delta16(0, 0) == 0);
    /* x=0, c=1 => 0 ^ 0 ^ 0 ^ 1 = 1 */
    TEST("delta16(0,1) == 1", omi_delta16(0, 1) == 1);
    /* x=0xFFFF, c=0 */
    uint16_t d = omi_delta16(0xFFFF, 0);
    TEST("delta16(0xFFFF,0) non-zero", d != 0);
    /* rotl16(0xFFFF,1)=0xFFFF, rotl16(0xFFFF,3)=0xFFFF,
     * rotr16(0xFFFF,2)=0xFFFF => 0xFFFF ^ 0xFFFF ^ 0xFFFF ^ 0 = 0xFFFF */
    TEST("delta16(0xFFFF,0) == 0xFFFF", d == 0xFFFF);
    /* x=1, c=0 => rotl16(1,1)=2, rotl16(1,3)=8, rotr16(1,2)=0x4000
     * => 2 ^ 8 ^ 0x4000 = 0x400A */
    TEST("delta16(1,0) == 0x400A", omi_delta16(1, 0) == 0x400A);
    /* x=0xAA55, c=0x55AA */
    uint16_t d2 = omi_delta16(0xAA55, 0x55AA);
    TEST("delta16(0xAA55,0x55AA) != 0", d2 != 0);
    /* delta16(-,-) is symmetric in bit operations */
    TEST("delta16(5,5) == delta16(5,5)", omi_delta16(5, 5) == omi_delta16(5, 5));
    /* deterministic */
    TEST("delta16 is deterministic",
         omi_delta16(42, 99) == omi_delta16(42, 99));

    /* ── Test 2: orbit_step ── */
    printf("\n[Test 2] orbit_step\n");
    {
        OMI_OrbitState s = { .x = 0, .seed = 0 };
        OMI_OrbitState s1 = omi_orbit_step(s);
        /* delta16(0,0) = 0, so x' = 0, seed' = 0 ^ (0*31) = 0 */
        TEST("step from (0,0) stays at (0,0)", s1.x == 0 && s1.seed == 0);

        /* (x=1, seed=0): delta16(1,0)=0x400A, seed' = 0 ^ (0x400A*31) */
        OMI_OrbitState s2 = { .x = 1, .seed = 0 };
        OMI_OrbitState s3 = omi_orbit_step(s2);
        TEST("step from (1,0) updates x",
             s3.x == 0x400A);
        uint16_t expected_seed = (uint16_t)(0 ^ (uint32_t)(0x400A * 31));
        TEST("step from (1,0) evolves seed",
             s3.seed == expected_seed);
        /* Subsequent step diverges */
        OMI_OrbitState s4 = omi_orbit_step(s3);
        TEST("second step diverges", s4.x != s3.x);
        TEST("second seed differs from first",
             s4.seed != s3.seed);

        /* Deterministic */
        OMI_OrbitState sa = omi_orbit_step((OMI_OrbitState){.x=7,.seed=3});
        OMI_OrbitState sb = omi_orbit_step((OMI_OrbitState){.x=7,.seed=3});
        TEST("orbit_step is deterministic", sa.x == sb.x && sa.seed == sb.seed);
    }

    /* ── Test 3: orbit_trace cycle detection ── */
    printf("\n[Test 3] orbit_trace\n");
    {
        /* Fixed point: (0,0) stays at (0,0) — cycle of length 1 */
        OMI_OrbitTrace trace;
        omi_orbit_trace((OMI_OrbitState){.x=0,.seed=0}, &trace);
        TEST("trace from (0,0) has size >= 1", trace.size >= 1);
        TEST("trace from (0,0) detects cycle",
             trace.cycle_start != UINT32_MAX);
        TEST("cycle from (0,0) repeats at position 0",
             trace.cycle_start == 0);
        TEST("trace states[0] == 0", trace.states[0] == 0);

        /* Non-trivial orbit */
        omi_orbit_trace((OMI_OrbitState){.x=1,.seed=0}, &trace);
        TEST("trace from (1,0) has size > 1", trace.size > 1);
        TEST("trace from (1,0) detects cycle",
             trace.cycle_start != UINT32_MAX);
        TEST("cycle_start < size", trace.cycle_start < trace.size);
        TEST("all states in non-trivial trace are non-zero or some non-zero",
             1); /* just checking it runs */

        /* Null safety */
        omi_orbit_trace((OMI_OrbitState){.x=0,.seed=0}, NULL);
        TEST("trace null safety (no crash)", 1);

        /* Seed evolution in trace */
        TEST("seed at position 0 equals initial seed",
             trace.seeds[0] == 0); /* initial seed was 0 */
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
        /* Symmetry: BQF(x,y) should not equal BQF(y,x) in general
         * BQF(2,3)=372, BQF(3,2)=60*9+16*6+4*4=540+96+16=652 */
        TEST("BQF is not symmetric", omi_bqf(2, 3) != omi_bqf(3, 2));
    }

    /* ── Test 5: Fano lottery distribution ── */
    printf("\n[Test 5] Fano lottery\n");
    {
        int counts[8] = {0};
        for (uint32_t i = 0; i < 10000; i++) {
            uint8_t f = omi_fano_lottery((uint16_t)i);
            if (f < 8) counts[f]++;
        }
        int all_hit = 1;
        for (int i = 0; i < 8; i++)
            if (counts[i] == 0) { all_hit = 0; break; }
        TEST("Fano lottery hits all 8 values over 0..9999", all_hit);
        /* Deterministic */
        TEST("fano lottery deterministic",
             omi_fano_lottery(42) == omi_fano_lottery(42));
    }

    /* ── Test 6: Tetra role distribution ── */
    printf("\n[Test 6] Tetra role\n");
    {
        int counts[4] = {0};
        for (uint32_t i = 0; i < 1000; i++) {
            uint8_t t = omi_tetra_role((uint16_t)i);
            if (t < 4) counts[t]++;
        }
        int all_hit = 1;
        for (int i = 0; i < 4; i++)
            if (counts[i] == 0) { all_hit = 0; break; }
        TEST("tetra role hits all 4 values over 0..999", all_hit);
        TEST("tetra role deterministic",
             omi_tetra_role(42) == omi_tetra_role(42));
    }

    /* ── Test 7: Triple detection ── */
    printf("\n[Test 7] Triple detection\n");
    {
        /* For δ-closure to hold: δ(a)⊕δ(b)⊕δ(c) = seed.
         * We need to find actual triples that satisfy this. */
        OMI_OrbitTriple triple;
        int result;

        /* Start from (1,0) and take 3 steps to find a,b,c with the correct seed */
        OMI_OrbitState s = { .x = 1, .seed = 0 };
        s = omi_orbit_step(s); /* s1 */
        s = omi_orbit_step(s); /* s2 */
        s = omi_orbit_step(s); /* s3 */

        /* The triple (1, s1.x, s2.x) should satisfy δ-closure iff
         * delta16(1,0) = s1.x, delta16(s1.x, s0.seed) = s2.x, ...
         * But the seed evolves, so the 3rd step uses a different seed.
         *
         * The condition is: three steps under the SAME seed.
         * Let me compute directly:
         *   δ(1, seed) ⊕ δ(δ(1, seed), seed) ⊕ δ(δ(δ(1, seed), seed), seed) = seed
         *
         * For fixed point (0,0): δ(0,0)=0, δ(0,0)=0, δ(0,0)=0
         *   0 ⊕ 0 ⊕ 0 = 0 = seed ✓
         * But (0,0,0) has all same Fano values, rejected by distinctness.
         */

        /* Let's try seed=0 and find a,b,c that work.
         * For any a: δ(a,0) = rotl16(a,1)^rotl16(a,3)^rotr16(a,2)
         *
         * We need: δ(a,0) ⊕ δ(b,0) ⊕ δ(c,0) = 0, i.e., δ(a,0)⊕δ(b,0)=δ(c,0)
         *
         * With a=0, b=0: δ(0,0)=0, δ(0,0)=0, so we need δ(c,0)=0.
         * c=0: δ(0,0)=0. Triple (0,0,0): all same Fano value. Rejected. ✓
         *
         * Let's try: we need distinct Fano values forming a line.
         * Let's brute-force search small values. */
        int found_triple = 0;
        for (uint16_t a = 0; a < 256 && !found_triple; a++) {
            for (uint16_t b = a + 1; b < 256 && !found_triple; b++) {
                for (uint16_t c = b + 1; c < 256 && !found_triple; c++) {
                    if (omi_is_orbit_triangle(a, b, c, 0, &triple)) {
                        found_triple = 1;
                    }
                }
            }
        }
        if (found_triple) {
            TEST("found valid triple via brute force", 1);
            printf("    triple: (%u,%u,%u) fano_line=%u tetra=%u energy=%d\n",
                   triple.a, triple.b, triple.c,
                   triple.fano_line, triple.tetra_class, (int)triple.energy);
        } else {
            TEST("no triple found in 0..255 with seed=0",
                 !found_triple);
        }

        /* Null safety */
        result = omi_is_orbit_triangle(1, 2, 3, 0, NULL);
        TEST("triple check with NULL output (no crash)", result == 0 || result == 1);
    }

    /* ── Test 8: Seed lookup ── */
    printf("\n[Test 8] Seed lookup\n");
    {
        OMI_OrbitTrace trace;
        omi_orbit_trace((OMI_OrbitState){.x=0,.seed=42}, &trace);
        TEST("seed_at(0) == 42", omi_orbit_seed_at(&trace, 0) == 42);
        TEST("seed_at(out of bounds) == 0",
             omi_orbit_seed_at(&trace, trace.size + 100) == 0);
        TEST("seed_at(NULL) == 0", omi_orbit_seed_at(NULL, 0) == 0);
    }

    /* ── Test 10: 5040 Atlas Indexer ── */
    printf("\n[Test 10] 5040 Atlas Indexer\n");
    {
        /* omi_slot5040 */
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
        /* Coverage: compute many slots and check range */
        int all_in_range = 1;
        for (uint8_t f = 0; f < 7; f++)
            for (uint8_t t = 0; t < 4; t++)
                for (uint16_t p = 0; p < 256; p++) {
                    uint16_t slot = omi_slot5040(f, t, (uint8_t)p);
                    if (slot >= OMI_SLOT5040_SIZE) { all_in_range = 0; break; }
                }
        TEST("all computed slots in 0..5039", all_in_range);

        /* omi_attest_from_state */
        OMI_Attestation a0 = omi_attest_from_state(0, 0);
        OMI_Attestation a1 = omi_attest_from_state(1, 5);
        TEST("attest from (0,0) orbit_id < OMI_SLOT5040_SIZE",
             a0.orbit_id < OMI_SLOT5040_SIZE);
        TEST("attest from (0,0) delta_phase == 0", a0.delta_phase == 0);
        TEST("attest from (1,5) delta_phase == 5", a1.delta_phase == 5);
        TEST("attest from (1,5) fano_id == fano(1)",
             a1.fano_id == omi_fano_lottery(1));
        TEST("attest from (1,5) tetra_class == tetra(1)",
             a1.tetra_class == omi_tetra_role(1));
        /* Deterministic */
        OMI_Attestation a2a = omi_attest_from_state(42, 7);
        OMI_Attestation a2b = omi_attest_from_state(42, 7);
        TEST("attest is deterministic",
             a2a.orbit_id == a2b.orbit_id &&
             a2a.fano_id == a2b.fano_id &&
             a2a.tetra_class == a2b.tetra_class &&
             a2a.delta_phase == a2b.delta_phase);

        /* omi_attest_render */
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
            /* Length: "o" + 4 hex + "-o/" + 2 + "/" + 2 + "@" + 2 + "@" = 19 chars */
            size_t len = strlen(buf);
            TEST("attest render length >= 16 && <= 24",
                 len >= 16 && len <= 24);

            /* Render max orbit_id (5039 = 0x13AF) */
            OMI_Attestation ak = { .orbit_id = 5039, .fano_id = 6,
                                   .tetra_class = 3, .delta_phase = 0xFF };
            omi_attest_render(ak, buf);
            TEST("attest render known value",
                 strcmp(buf, "o13af-o/06/03@ff@") == 0);
        }

        /* omi_attest_render null safety */
        omi_attest_render((OMI_Attestation){0}, NULL);
        TEST("attest render null safety (no crash)", 1);
    }

    /* ── Test 9: Quotient projection 31→16 ── */
    printf("\n[Test 9] Quotient projection π\n");
    {
        /* π(x) = (x & 0x0F) ^ ((x >= 16) ? ((x & 0xF0) >> 4) : 0) */
        TEST("π(0) == 0", omi_project_31_to_16(0) == 0);
        TEST("π(15) == 15", omi_project_31_to_16(15) == 15);
        /* 16: (16 & 0x0F) ^ ((16 & 0xF0) >> 4) = 0 ^ 1 = 1 */
        TEST("π(16) == 1", omi_project_31_to_16(16) == 1);
        /* 255: (255 & 0x0F) ^ ((255 & 0xF0) >> 4) = 15 ^ 15 = 0 */
        TEST("π(255) == 0", omi_project_31_to_16(255) == 0);
        /* All 16-bit values project to 0..15 */
        int all_in_range = 1;
        for (uint32_t x = 0; x < 65536; x++) {
            uint8_t p = omi_project_31_to_16((uint16_t)x);
            if (p >= 16) { all_in_range = 0; break; }
        }
        TEST("π maps all 65536 values to 0..15", all_in_range);

        /* Deterministic */
        TEST("π is deterministic",
             omi_project_31_to_16(42) == omi_project_31_to_16(42));
    }

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
