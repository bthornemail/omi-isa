#include "omi_omion.h"
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
    printf("=== OMIOM Prefix-Scoped Cascade Resolver Tests ===\n\n");

    omi_gauge_init();

    OMIOM_Resolver res;
    omiom_init(&res);

    printf("[Test 1] Init and count\n");
    TEST("resolver initialized", omiom_prefix_count(&res) == 0);
    TEST("prefix count 0 after init", omiom_prefix_count(&res) == 0);

    printf("\n[Test 2] Add prefix entries\n");
    int ret = omiom_add_prefix(&res, 0x0A00000000000000ULL, 8, 0x0A);
    TEST("add prefix /8 returns 0", ret == 0);
    ret = omiom_add_prefix(&res, 0x0B0C000000000000ULL, 16, 0x0B);
    TEST("add prefix /16 returns 0", ret == 0);
    ret = omiom_add_prefix(&res, 0x0D0E0F0000000000ULL, 24, 0x0D);
    TEST("add prefix /24 returns 0", ret == 0);
    TEST("prefix count 3 after adds", omiom_prefix_count(&res) == 3);

    printf("\n[Test 3] Duplicate add updates existing\n");
    ret = omiom_add_prefix(&res, 0x0A00000000000000ULL, 8, 0xAA);
    TEST("duplicate add returns 0", ret == 0);
    TEST("count still 3", omiom_prefix_count(&res) == 3);

    printf("\n[Test 4] Remove prefix\n");
    ret = omiom_remove_prefix(&res, 0x0B0C000000000000ULL, 16);
    TEST("remove returns 0", ret == 0);
    TEST("count becomes 2", omiom_prefix_count(&res) == 2);

    printf("\n[Test 5] Remove nonexistent\n");
    ret = omiom_remove_prefix(&res, 0xDEAD000000000000ULL, 8);
    TEST("remove nonexistent returns -2", ret == -2);

    printf("\n[Test 6] omiom_resolve longest prefix match\n");
    OMI_512_Envelope env;
    memset(&env, 0, sizeof(env));
    memcpy(env.gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
    env.target[0] = 0x0A;
    env.target[1] = 0xBB;

    uint8_t next_hop = 0;
    ret = omiom_resolve(&res, &env, &next_hop);
    TEST("resolve /8 match returns 0", ret == 0);
    TEST("next_hop matches /8 entry", next_hop == 0xAA);

    printf("\n[Test 7] Resolve longer prefix wins\n");
    omiom_add_prefix(&res, 0x0A00000000000000ULL, 8, 0x0A);
    omiom_add_prefix(&res, 0x0ABB000000000000ULL, 16, 0xCC);
    memset(&env, 0, sizeof(env));
    memcpy(env.gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
    env.target[0] = 0x0A;
    env.target[1] = 0xBB;
    env.target[2] = 0xCC;

    ret = omiom_resolve(&res, &env, &next_hop);
    TEST("resolve /16 match returns 0", ret == 0);
    TEST("next_hop matches /16 entry", next_hop == 0xCC);

    printf("\n[Test 8] No match returns -2\n");
    memset(&env, 0, sizeof(env));
    env.target[0] = 0xFF;
    env.target[1] = 0xFF;

    ret = omiom_resolve(&res, &env, &next_hop);
    TEST("resolve no match returns -2", ret == -2);

    printf("\n[Test 9] Null safety\n");
    TEST("resolve null resolver", omiom_resolve(NULL, &env, &next_hop) == -1);
    TEST("resolve null env", omiom_resolve(&res, NULL, &next_hop) == -1);
    TEST("resolve null next_hop", omiom_resolve(&res, &env, NULL) == -1);
    TEST("add null resolver", omiom_add_prefix(NULL, 0, 0, 0) == -1);
    TEST("remove null resolver", omiom_remove_prefix(NULL, 0, 0) == -1);
    TEST("prefix count null", omiom_prefix_count(NULL) == 0);

    printf("\n[Test 10] omiom_cascade fallback\n");
    OMIOM_Resolver empty;
    omiom_init(&empty);

    memset(&env, 0, sizeof(env));
    memcpy(env.gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
    env.gauge[4] = 0x01;
    env.gauge[5] = 0x01;
    env.orientation[0] = 0x01;
    env.orientation[1] = 0x01;

    ret = omiom_resolve(&empty, &env, &next_hop);
    TEST("empty resolve returns -2", ret == -2);

    ret = omiom_cascade(&empty, &env, &next_hop);
    TEST("cascade returns 1 on layer fallback", ret == 1);

    printf("\n[Test 11] Resolve with /0 prefix skipped\n");
    omiom_init(&res);
    omiom_add_prefix(&res, 0, 0, 0x01);
    memset(&env, 0, sizeof(env));
    env.target[0] = 0x0A;
    ret = omiom_resolve(&res, &env, &next_hop);
    TEST("/0 prefix skipped, returns -2", ret == -2);

    printf("\n[Test 12] Max prefixes\n");
    omiom_init(&res);
    for (int i = 0; i < OMIOM_MAX_PREFIXES; i++) {
        uint64_t addr = (uint64_t)i << 56;
        ret = omiom_add_prefix(&res, addr, 8, (uint8_t)i);
        if (ret != 0) { TEST("add failed early", 0); break; }
    }
    TEST("max prefixes reachable", omiom_prefix_count(&res) == OMIOM_MAX_PREFIXES);
    ret = omiom_add_prefix(&res, 0xFF00000000000000ULL, 8, 0xFF);
    TEST("add beyond max returns -3", ret == -3);

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
