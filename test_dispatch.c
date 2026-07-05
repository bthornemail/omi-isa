#include <stdio.h>
#include <string.h>
#include "omi_dispatch.h"
#include "omienv.h"
#include "stream.h"
#include "isa.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name, expr) do { \
    tests_run++; \
    int ok = (expr); \
    if (ok) tests_passed++; \
    printf("  %s: %s\n", name, ok ? "PASS" : "FAIL"); \
} while(0)

static void build_test_envelope(OMI_512_Envelope* env, uint8_t opcode) {
    memset(env, 0, sizeof(OMI_512_Envelope));
    memcpy(env->gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
    env->target[0] = opcode;
}

static int custom_handler_stub(OMI_DispatchContext* ctx) {
    (void)ctx;
    return 42;
}

int main(void) {
    printf("=== OMI Dispatch Table Tests ===\n\n");

    printf("[Test 1] Dispatch table init — all 32 slots wired\n");
    {
        omi_dispatch_init();
        int all_nonnull = 1;
        for (int i = 0; i < OMI_DISPATCH_SLOTS; i++) {
            if (!omi_dispatch_table[i]) { all_nonnull = 0; break; }
        }
        TEST("all 32 slots non-null", all_nonnull);
    }

    printf("\n[Test 2] omi_env_get_opcode\n");
    {
        OMI_512_Envelope env;
        build_test_envelope(&env, PROBE);
        int op = omi_env_get_opcode(&env);
        TEST("opcode = PROBE (20)", op == PROBE);
        build_test_envelope(&env, FOLD);
        op = omi_env_get_opcode(&env);
        TEST("opcode = FOLD (30)", op == FOLD);
        build_test_envelope(&env, VM_ESCAPE);
        op = omi_env_get_opcode(&env);
        TEST("opcode = VM_ESCAPE (31)", op == VM_ESCAPE);
    }

    printf("\n[Test 3] NOP dispatch — returns 0\n");
    {
        OMI_512_Envelope env;
        build_test_envelope(&env, NOP);
        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        int r = omi_dispatch_execute(&ctx);
        TEST("nop returns 0", r == 0);
    }

    printf("\n[Test 4] PROBE dispatch — emits PROBE_ACK\n");
    {
        OMI_512_Envelope env;
        build_test_envelope(&env, PROBE);

        OMI_CPU cpu;
        init_cpu(&cpu);
        cpu.capability = 0x00FF00FF;
        cpu.probe_state = PROBE_STATE_IDLE;

        uint8_t tx_buf[OMI_ENV_SIZE];
        size_t tx_len = 0;

        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        ctx.vm = &cpu;
        ctx.tx_buffer = tx_buf;
        ctx.tx_capacity = sizeof(tx_buf);
        ctx.tx_len = &tx_len;

        int r = omi_dispatch_execute(&ctx);
        TEST("probe returns 0", r == 0);
        TEST("tx_len == envelope size", tx_len == OMI_ENV_SIZE);

        OMI_512_Envelope ack;
        memcpy(&ack, tx_buf, tx_len);
        int ack_op = omi_env_get_opcode(&ack);
        TEST("response opcode is PROBE_ACK", ack_op == PROBE_ACK);
        TEST("response valid envelope", omi_env_validate(&ack) == 0);
    }

    printf("\n[Test 5] PROBE_ACK — stores capability\n");
    {
        OMI_512_Envelope env;
        build_test_envelope(&env, PROBE_ACK);
        env.orientation[0] = 0xDE;
        env.orientation[1] = 0xAD;
        env.orientation[2] = 0xBE;
        env.orientation[3] = 0xEF;

        OMI_CPU cpu;
        init_cpu(&cpu);
        cpu.capability = 0xFFFFFFFF;

        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        ctx.vm = &cpu;

        omi_dispatch_execute(&ctx);
        TEST("peer capability stored", cpu.peer_capability == 0xDEADBEEF);
        TEST("isa_subset = intersection", cpu.isa_subset == 0xDEADBEEF);
        TEST("probe_state = NEGOTIATED", cpu.probe_state == PROBE_STATE_NEGOTIATED);
    }

    printf("\n[Test 6] FOLD — folds bitboard\n");
    {
        OMI_512_Envelope env;
        build_test_envelope(&env, FOLD);

        OMI_CPU cpu;
        init_cpu(&cpu);

        uint64_t bb = 0x12345678ABCDEF01ULL;

        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        ctx.bitboard = bb;
        ctx.vm = &cpu;

        int r = omi_dispatch_execute(&ctx);
        TEST("fold returns 0", r == 0);
        uint64_t expected = omi_bb_fold(bb);
        TEST("R0 has folded value", cpu.R[0] == (uint32_t)(expected & 0xFFFFFFFF));
    }

    printf("\n[Test 7] SYNC_COMMIT — stores commit state\n");
    {
        OMI_512_Envelope env;
        build_test_envelope(&env, SYNC_COMMIT);

        OMI_CPU cpu;
        init_cpu(&cpu);
        cpu.probe_state = PROBE_STATE_PROBING;

        uint8_t tx_buf[OMI_ENV_SIZE];
        size_t tx_len = 0;

        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        ctx.vm = &cpu;
        ctx.tx_buffer = tx_buf;
        ctx.tx_capacity = sizeof(tx_buf);
        ctx.tx_len = &tx_len;

        omi_dispatch_execute(&ctx);
        TEST("probe_state = NEGOTIATED", cpu.probe_state == PROBE_STATE_NEGOTIATED);
        TEST("ack sent", tx_len == OMI_ENV_SIZE);
    }

    printf("\n[Test 8] omi_dispatch_set — custom handler\n");
    {
        omi_dispatch_init();

        omi_dispatch_set(VM_ESCAPE, custom_handler_stub);
        TEST("custom set stores handler",
             omi_dispatch_table[VM_ESCAPE] == custom_handler_stub);

        OMI_512_Envelope env;
        build_test_envelope(&env, VM_ESCAPE);
        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        int r = omi_dispatch_execute(&ctx);
        TEST("custom handler returns 42", r == 42);

        /* Reset and verify ROUTE still works */
        omi_dispatch_init();
        build_test_envelope(&env, ROUTE);

        uint8_t tx_buf2[OMI_ENV_SIZE];
        size_t tx_len2 = 0;

        OMI_DispatchContext ctx2;
        memset(&ctx2, 0, sizeof(ctx2));
        ctx2.env = &env;
        ctx2.tx_buffer = tx_buf2;
        ctx2.tx_capacity = sizeof(tx_buf2);
        ctx2.tx_len = &tx_len2;

        int r2 = omi_dispatch_execute(&ctx2);
        TEST("route handler works", r2 == 0);
        TEST("route forwarded", tx_len2 == OMI_ENV_SIZE);
    }

    printf("\n[Test 9] Stream parser integration with dispatch\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);

        OMI_CPU cpu;
        init_cpu(&cpu);

        sp.auto_dispatch = 0;
        sp.vm = &cpu;
        sp.dispatch_result = -99;

        OMI_512_Envelope env;
        build_test_envelope(&env, FOLD);
        env.target[0] = FOLD;

        uint8_t* bytes = (uint8_t*)&env;
        for (size_t i = 0; i < OMI_ENV_SIZE; i++)
            stream_push_byte(&sp, bytes[i]);

        TEST("envelope received", stream_has_event(&sp) == 1);

        /* Now execute */
        stream_execute(&sp);
        TEST("dispatch executed", sp.dispatch_result == 0);

        StreamEvent evt;
        stream_pop_event(&sp, &evt);
        TEST("event valid", evt.valid == 1);
        TEST("opcode extracted", evt.opcode == FOLD);
        TEST("dispatch result propagated", evt.dispatch_result == 0);
    }

    printf("\n[Test 10] Auto-dispatch mode\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);

        OMI_CPU cpu;
        init_cpu(&cpu);
        cpu.capability = 0xCAFEBABE;

        sp.auto_dispatch = 1;
        sp.vm = &cpu;

        OMI_512_Envelope env;
        build_test_envelope(&env, PROBE);

        uint8_t* bytes = (uint8_t*)&env;
        for (size_t i = 0; i < OMI_ENV_SIZE; i++)
            stream_push_byte(&sp, bytes[i]);

        TEST("auto-dispatch ran", sp.dispatch_result == 0);
        TEST("has event", stream_has_event(&sp) == 1);

        StreamEvent evt;
        stream_pop_event(&sp, &evt);
        TEST("event valid", evt.valid == 1);
        TEST("opcode PROBE", evt.opcode == PROBE);
        TEST("response sent", evt.response_len == OMI_ENV_SIZE);

        OMI_512_Envelope ack;
        memcpy(&ack, evt.response, evt.response_len);
        int ack_op = omi_env_get_opcode(&ack);
        TEST("response is PROBE_ACK", ack_op == PROBE_ACK);
    }

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
