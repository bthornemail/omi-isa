#include "omi_receipt.h"
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
    printf("=== META64 Receipt Verification Tests ===\n\n");

    omi_gauge_init();

    OMI_512_Envelope env_canon;
    memset(&env_canon, 0, sizeof(env_canon));
    memcpy(env_canon.gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);

    printf("[Test 1] META64 validation\n");
    int ret = omi_receipt_verify_meta64(&env_canon);
    TEST("canonical envelope passes META64 check", ret == 0);

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
    env.target[0] = 0x0A;
    env.target[1] = 0xBB;
    env.relation[0] = 0x01;
    env.relation[1] = 0x02;
    env.relation[2] = 0x03;

    printf("\n[Test 2] Invalid META64 (bad gauge[0])\n");
    OMI_512_Envelope bad;
    memcpy(&bad, &env_canon, sizeof(env_canon));
    bad.gauge[0] = 0x0F;
    ret = omi_receipt_verify_meta64(&bad);
    TEST("bad gauge[0] rejected (memcmp fails)", ret < 0);

    printf("\n[Test 3] Invalid META64 (bad gauge[7])\n");
    memcpy(&bad, &env_canon, sizeof(env_canon));
    bad.gauge[7] = 0x00;
    ret = omi_receipt_verify_meta64(&bad);
    TEST("bad gauge[7] rejected (memcmp fails)", ret < 0);

    printf("\n[Test 4] Invalid META64 (bad control bytes)\n");
    memcpy(&bad, &env_canon, sizeof(env_canon));
    bad.gauge[2] = 0x00;
    ret = omi_receipt_verify_meta64(&bad);
    TEST("bad gauge[2] rejected (memcmp fails)", ret < 0);

    memcpy(&bad, &env_canon, sizeof(env_canon));
    bad.gauge[6] = 0x00;
    ret = omi_receipt_verify_meta64(&bad);
    TEST("bad gauge[6] rejected (memcmp fails)", ret < 0);

    printf("\n[Test 5] Receipt compute\n");
    OMI_Receipt receipt;
    ret = omi_receipt_compute(&env, &receipt);
    TEST("compute returns 0", ret == 0);
    TEST("receipt is marked valid", receipt.valid == 1);
    TEST("gauge action matches", receipt.gauge_action == (env.gauge[0] & 0x1F));
    TEST("layer 8 == 40320", receipt.layer_values[0] == 40320);
    TEST("layer 9 matches gauge[4..5] % 9",
         receipt.layer_values[1] == (uint32_t)(((uint16_t)0x1234) % 9));
    TEST("layer 10 matches gauge[6..7] % 10",
         receipt.layer_values[2] == (uint32_t)(((uint16_t)0x5678) % 10));
    TEST("layer 11 matches orientation[0..1] % 11",
         receipt.layer_values[3] == (uint32_t)(((uint16_t)0x9ABC) % 11));
    TEST("layer 12 matches orientation[2..3] % 12",
         receipt.layer_values[4] == (uint32_t)(((uint16_t)0xDEF0) % 12));

    printf("\n[Test 6] Receipt verify (self-consistent)\n");
    ret = omi_receipt_verify(&env, &receipt);
    TEST("self-verify returns 0", ret == 0);

    printf("\n[Test 7] Receipt verify (tampered envelope)\n");
    OMI_512_Envelope tampered;
    memcpy(&tampered, &env, sizeof(env));
    tampered.gauge[4] = 0xFF;

    OMI_Receipt bad_receipt;
    omi_receipt_compute(&tampered, &bad_receipt);
    ret = omi_receipt_verify(&tampered, &receipt);
    TEST("tampered envelope fails verify", ret != 0);

    printf("\n[Test 8] Receipt to string\n");
    char buf[128];
    ret = omi_receipt_to_string(&receipt, buf, sizeof(buf));
    TEST("to_string returns positive", ret > 0);
    TEST("to_string non-empty", buf[0] != '\0');

    printf("\n[Test 9] Null safety\n");
    TEST("compute null env", omi_receipt_compute(NULL, &receipt) == -1);
    TEST("compute null receipt", omi_receipt_compute(&env, NULL) == -1);
    TEST("verify null env", omi_receipt_verify(NULL, &receipt) == -1);
    TEST("verify null receipt", omi_receipt_verify(&env, NULL) == -1);
    TEST("verify_meta64 null env", omi_receipt_verify_meta64(NULL) == -1);
    TEST("to_string null receipt", omi_receipt_to_string(NULL, buf, sizeof(buf)) == -1);
    TEST("to_string null out", omi_receipt_to_string(&receipt, NULL, sizeof(buf)) == -1);
    TEST("to_string zero len", omi_receipt_to_string(&receipt, buf, 0) == -1);

    printf("\n[Test 10] Invalid receipt flag\n");
    OMI_Receipt invalid_rcpt;
    memset(&invalid_rcpt, 0, sizeof(invalid_rcpt));
    invalid_rcpt.valid = 0;
    ret = omi_receipt_verify(&env, &invalid_rcpt);
    TEST("invalid receipt flag returns -2", ret == -2);

    printf("\n[Test 11] Path length mismatch\n");
    memcpy(&invalid_rcpt, &receipt, sizeof(receipt));
    invalid_rcpt.path_len = 0;
    ret = omi_receipt_verify(&env, &invalid_rcpt);
    TEST("path_len mismatch fails verify", ret < 0);

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
