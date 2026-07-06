#include "omicron.h"
#include "gauge_exec.h"
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
    printf("=== Omicron Resolution Tests ===\n\n");

    omi_gauge_init();

    OMI_512_Envelope env;
    memset(&env, 0, sizeof(env));
    memcpy(env.gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
    env.gauge[4] = 0x12;
    env.gauge[5] = 0x34;
    env.gauge[6] = 0x56;
    env.gauge[7] = 0x78;
    env.orientation[0] = 0x9A;
    env.orientation[1] = 0xBC;
    env.orientation[2] = 0xDE;
    env.orientation[3] = 0xF0;

    printf("[Test 1] omicron_resolve baseline\n");
    uint32_t r8 = omicron_resolve(&env, 8);
    TEST("layer 8 returns 40320", r8 == 40320);

    uint32_t r9 = omicron_resolve(&env, 9);
    TEST("layer 9 derived from S2 (gauge[4..5])", r9 == (uint32_t)(((uint16_t)0x1234) % 9));

    uint32_t r10 = omicron_resolve(&env, 10);
    TEST("layer 10 derived from S3 (gauge[6..7])", r10 == (uint32_t)(((uint16_t)0x5678) % 10));

    uint32_t r11 = omicron_resolve(&env, 11);
    TEST("layer 11 derived from S4 (orientation[0..1])", r11 == (uint32_t)(((uint16_t)0x9ABC) % 11));

    uint32_t r12 = omicron_resolve(&env, 12);
    TEST("layer 12 derived from S5 (orientation[2..3])", r12 == (uint32_t)(((uint16_t)0xDEF0) % 12));

    printf("\n[Test 2] Convenience wrappers\n");
    TEST("omicron_route == layer 9", omicron_route(&env) == r9);
    TEST("omicron_declaration == layer 10", omicron_declaration(&env) == r10);
    TEST("omicron_witness == layer 11", omicron_witness(&env) == r11);
    TEST("omicron_phase == layer 12", omicron_phase(&env) == r12);

    printf("\n[Test 3] Edge cases\n");
    TEST("null env returns 0", omicron_resolve(NULL, 9) == 0);
    TEST("layer 0 returns 0", omicron_resolve(&env, 0) == 0);
    TEST("layer 13 returns 0", omicron_resolve(&env, 13) == 0);

    printf("\n[Test 4] Lower factorial layers (1-7) from bitboard\n");
    OMI_512_Envelope e2;
    memset(&e2, 0, sizeof(e2));
    memcpy(e2.gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
    e2.relation[0] = 0xAA;
    uint64_t board;
    omi_env_to_bitboard(&e2, &board);
    TEST("layer 1 returns 1", omicron_resolve(&e2, 1) == 1);
    TEST("layer 2 returns 1 or 2", omicron_resolve(&e2, 2) >= 1);
    TEST("layer 3 in 1-6", omicron_resolve(&e2, 3) >= 1 && omicron_resolve(&e2, 3) <= 6);
    TEST("layer 4 in 0-23", omicron_resolve(&e2, 4) < 24);
    TEST("layer 5 in 0-119", omicron_resolve(&e2, 5) < 120);
    TEST("layer 6 in 0-719", omicron_resolve(&e2, 6) < 720);
    TEST("layer 7 in 0-5039", omicron_resolve(&e2, 7) < 5040);

    printf("\n[Test 5] omicron_slide lifecycle\n");
    OMI_CPU cpu;
    init_cpu(&cpu);
    cpu.capability = 0xFFFFFFFF;
    OMI_DispatchContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.env = &env;
    ctx.vm = &cpu;
    omi_dispatch_init();

    int slide_ret = omicron_slide(&ctx, 0x00);
    TEST("omicron_slide returns 0 on valid code", slide_ret == 0);

    int slide_bad = omicron_slide(NULL, 0x00);
    TEST("omicron_slide with NULL ctx returns -1", slide_bad == -1);

    int slide_inactive = omicron_slide(&ctx, 0x40);
    TEST("omicron_slide with inactive code returns -1", slide_inactive == -1);

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
