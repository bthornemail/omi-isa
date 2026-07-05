#include <stdio.h>
#include <string.h>
#include "gauge_exec.h"
#include "omienv.h"
#include "omi_dispatch.h"
#include "isa.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name, expr) do { \
    tests_run++; \
    int ok = (expr); \
    if (ok) tests_passed++; \
    printf("  %s: %s\n", name, ok ? "PASS" : "FAIL"); \
} while(0)

static void build_test_env(OMI_512_Envelope* env, uint8_t opcode) {
    memset(env, 0, sizeof(OMI_512_Envelope));
    memcpy(env->gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
    env->target[0] = opcode;
}

int main(void) {
    printf("=== Gauge Lambda Execution Tests ===\n\n");

    omi_dispatch_init();
    omi_gauge_init();

    printf("[Test 1] Gauge exec init — all slots NULL\n");
    {
        int all_null = 1;
        for (int i = 0; i < 128; i++) {
            if (gauge_exec_is_bound(i)) { all_null = 0; break; }
        }
        TEST("all 128 slots unbound", all_null == 1);
    }

    printf("\n[Test 2] Gauge exec bind / unbind\n");
    {
        int r = gauge_exec_bind(42, NULL);
        TEST("bind returns 0", r == 0);
        TEST("is_bound true after bind", gauge_exec_is_bound(42) == 1);

        r = gauge_exec_unbind(42);
        TEST("unbind returns 0", r == 0);
        TEST("is_bound false after unbind", gauge_exec_is_bound(42) == 0);
    }

    printf("\n[Test 3] Lambda execution via eval chain\n");
    {
        OmiGaugeEntry entry;
        memset(&entry, 0, sizeof(entry));
        entry.code = 0x50;
        entry.active = 1;
        entry.is_lambda = 1;
        entry.car_addr = 0x51;
        entry.cdr_addr = 0x52;

        OmiGaugeEntry car_entry;
        memset(&car_entry, 0, sizeof(car_entry));
        car_entry.code = 0x51;
        car_entry.active = 1;
        car_entry.is_lambda = 1;
        car_entry.car_addr = 0;
        car_entry.cdr_addr = 0;

        OmiGaugeEntry cdr_entry;
        memset(&cdr_entry, 0, sizeof(cdr_entry));
        cdr_entry.code = 0x52;
        cdr_entry.active = 1;
        cdr_entry.is_lambda = 1;
        cdr_entry.car_addr = 0;
        cdr_entry.cdr_addr = 0;

        gauge_exec_set_semantics(0x50, &entry);
        gauge_exec_set_semantics(0x51, &car_entry);
        gauge_exec_set_semantics(0x52, &cdr_entry);

        OMI_512_Envelope env;
        build_test_env(&env, GAUGE_INVOKE);
        env.target[1] = 0x50;

        OMI_CPU cpu;
        init_cpu(&cpu);

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
        TEST("gauge_invoke returns 0", r == 0);
    }

    printf("\n[Test 4] Eval depth limit\n");
    {
        OmiGaugeEntry loop;
        memset(&loop, 0, sizeof(loop));
        loop.code = 0x60;
        loop.active = 1;
        loop.is_lambda = 1;
        loop.car_addr = 0x60;
        loop.cdr_addr = 0;

        gauge_exec_set_semantics(0x60, &loop);

        OMI_512_Envelope env;
        build_test_env(&env, GAUGE_INVOKE);
        env.target[1] = 0x60;

        OMI_CPU cpu;
        init_cpu(&cpu);

        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        ctx.vm = &cpu;

        int r = gauge_exec_lambda(&ctx, 0x60);
        TEST("self-loop hits depth limit", r == -4);
    }

    printf("\n[Test 5] Non-printing semantics change\n");
    {
        const OmiGaugeEntry* before = omi_gauge_lookup(0x1E);
        TEST("0x1E initially RECORD_CLOSE",
             before && before->action == GAUGE_ACTION_RECORD_CLOSE);
        TEST("0x1E initially not lambda",
             before && before->is_lambda == 0);

        OmiGaugeEntry modified;
        memcpy(&modified, before, sizeof(modified));
        modified.is_lambda = 1;
        modified.action = GAUGE_ACTION_NONE;

        int r = gauge_exec_set_semantics(0x1E, &modified);
        TEST("set_semantics returns 0", r == 0);

        const OmiGaugeEntry* after = omi_gauge_lookup(0x1E);
        TEST("0x1E now is_lambda", after && after->is_lambda == 1);
        TEST("0x1E action changed", after && after->action == GAUGE_ACTION_NONE);

        /* Restore */
        memcpy(&modified, before, sizeof(modified));
        gauge_exec_set_semantics(0x1E, &modified);
    }

    printf("\n[Test 6] GAUGE_BIND via dispatch\n");
    {
        OMI_512_Envelope env;
        build_test_env(&env, GAUGE_BIND);
        env.target[1] = 0x70;

        OMI_CPU cpu;
        init_cpu(&cpu);

        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        ctx.vm = &cpu;

        int r = omi_dispatch_execute(&ctx);
        TEST("GAUGE_BIND dispatch ok", r == 0);
    }

    printf("\n[Test 7] Custom handler via gauge_exec_bind\n");
    {
        int custom_count = 0;
        OMI_OpcodeHandler count_handler = (OMI_OpcodeHandler)(void*)&custom_count;
        (void)count_handler;

        gauge_exec_bind(0x7F, NULL);
        TEST("bind NULL handler to 0x7F", gauge_exec_is_bound(0x7F) == 1);

        gauge_exec_unbind(0x7F);
        TEST("unbind 0x7F", gauge_exec_is_bound(0x7F) == 0);
    }

    printf("\n[Test 8] Invoke inactive gauge returns error\n");
    {
        OMI_512_Envelope env;
        build_test_env(&env, GAUGE_INVOKE);
        env.target[1] = 0x40;

        OMI_CPU cpu;
        init_cpu(&cpu);

        OMI_DispatchContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.env = &env;
        ctx.vm = &cpu;

        int r = omi_dispatch_execute(&ctx);
        TEST("invoke inactive gauge fails", r == -3);
    }

    printf("\n[Test 9] Set semantics restructures gauge table\n");
    {
        OmiGaugeEntry custom;
        memset(&custom, 0, sizeof(custom));
        custom.code = 0x25;
        custom.cls = GAUGE_CLASS_PRINTABLE;
        custom.action = GAUGE_ACTION_REGISTER_INJECT;
        custom.active = 1;
        custom.car_addr = 0x1E;
        custom.cdr_addr = 0x78;
        custom.is_lambda = 1;

        int r = gauge_exec_set_semantics(0x25, &custom);
        TEST("set_semantics returns 0", r == 0);

        const OmiGaugeEntry* retrieved = omi_gauge_lookup(0x25);
        TEST("code preserved", retrieved && retrieved->code == 0x25);
        TEST("is_lambda set", retrieved && retrieved->is_lambda == 1);
        TEST("car_addr set", retrieved && retrieved->car_addr == 0x1E);
        TEST("cdr_addr set", retrieved && retrieved->cdr_addr == 0x78);
    }

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
