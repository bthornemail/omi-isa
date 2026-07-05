#include <stdio.h>
#include <string.h>
#include "omienv.h"
#include "stream.h"
#include "sector.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name, expr) do { \
    tests_run++; \
    int ok = (expr); \
    if (ok) tests_passed++; \
    printf("  %s: %s\n", name, ok ? "PASS" : "FAIL"); \
} while(0)

static void print_env(const OMI_512_Envelope* env) {
    printf("    Gauge: ");
    for (int i = 0; i < 8; i++) printf("%02X ", env->gauge[i]);
    printf("\n    Orient: ");
    for (int i = 0; i < 8; i++) printf("%02X ", env->orientation[i]);
    printf("\n    Recov: ");
    for (int i = 0; i < 8; i++) printf("%02X ", env->recovery[i]);
    printf("\n    Target: ");
    for (int i = 0; i < 8; i++) printf("%02X ", env->target[i]);
    printf("\n    Relat: ");
    for (int i = 0; i < 32; i++) printf("%02X ", env->relation[i]);
    printf("\n");
}

int main(void) {
    printf("=== OMI 512-Bit Envelope Tests ===\n\n");

    printf("[Test 1] Parse + validate canonical envelope\n");
    {
        OMI_512_Envelope env;
        uint8_t data[OMI_ENV_SIZE];
        memset(data, 0, sizeof(data));
        data[0] = 0xFF; data[1] = 0x00; data[2] = 0x1C; data[3] = 0x1D;
        data[4] = 0x1E; data[5] = 0x1F; data[6] = 0x20; data[7] = 0xFF;
        const char* rel = "(omi . imo)";
        memcpy(data + 32, rel, strlen(rel) + 1);

        int r = omi_env_parse(data, sizeof(data), &env);
        TEST("parse", r == 0);
        r = omi_env_validate(&env);
        TEST("validate canonical", r == 0);
        print_env(&env);
    }

    printf("\n[Test 2] Reject invalid envelope\n");
    {
        OMI_512_Envelope env;
        uint8_t data[OMI_ENV_SIZE];
        memset(data, 0, sizeof(data));
        data[0] = 0xDE; data[1] = 0xAD;
        int r = omi_env_parse(data, sizeof(data), &env);
        TEST("parse still ok", r == 0);
        r = omi_env_validate(&env);
        TEST("validate rejects bad gauge", r != 0);
    }

    printf("\n[Test 3] Bitboard construction\n");
    {
        OMI_512_Envelope env;
        memset(&env, 0, sizeof(env));
        memcpy(env.gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
        env.orientation[0] = 5;
        env.relation[0] = 0x78;
        memcpy(env.relation + 1, "ABC", 3);

        uint64_t board = 0;
        int r = omi_env_to_bitboard(&env, &board);
        TEST("to_bitboard ok", r == 0);
        TEST("gauge byte set", omi_bb_get_gauge(board) == 0x7F);
        TEST("flag 0x78 set", omi_bb_get_flag(board, OMI_BB_BRIDGE_78_BIT) == 1);
        TEST("place non-zero", omi_bb_get_place(board) != 0);

        uint64_t folded = omi_bb_fold(board);
        TEST("fold produces change", folded != board);
        printf("    Board:  0x%016llX\n", (unsigned long long)board);
        printf("    Folded: 0x%016llX\n", (unsigned long long)folded);
    }

    printf("\n[Test 4] Gauge table\n");
    {
        omi_gauge_init();
        const OmiGaugeEntry* e0 = omi_gauge_lookup(0x00);
        TEST("0x00 is active", e0 && e0->active);
        TEST("0x00 action PLACE", e0 && e0->action == GAUGE_ACTION_PLACE);

        const OmiGaugeEntry* e1e = omi_gauge_lookup(0x1E);
        TEST("0x1E is active", e1e && e1e->active);
        TEST("0x1E action RECORD_CLOSE", e1e && e1e->action == GAUGE_ACTION_RECORD_CLOSE);
        TEST("0x1E diag BALANCED", e1e && e1e->diag == DIAG_BALANCED);

        const OmiGaugeEntry* e78 = omi_gauge_lookup(0x78);
        TEST("0x78 is active", e78 && e78->active);
        TEST("0x78 action SYSTEM_WITNESS", e78 && e78->action == GAUGE_ACTION_SYSTEM_WITNESS);
    }

    printf("\n[Test 5] Streaming parser\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);

        uint8_t data[OMI_ENV_SIZE];
        memset(data, 0, sizeof(data));
        data[0] = 0xFF; data[1] = 0x00; data[2] = 0x1C; data[3] = 0x1D;
        data[4] = 0x1E; data[5] = 0x1F; data[6] = 0x20; data[7] = 0xFF;
        memcpy(data + 32, "test stream parser", 18);

        TEST("no event before push", stream_has_event(&sp) == 0);
        for (size_t i = 0; i < OMI_ENV_SIZE; i++)
            stream_push_byte(&sp, data[i]);
        TEST("event after full push", stream_has_event(&sp) == 1);

        StreamEvent evt;
        int r = stream_pop_event(&sp, &evt);
        TEST("pop event ok", r == 0);
        TEST("event valid", evt.valid == 1);
        TEST("parser reset after pop", stream_has_event(&sp) == 0);
    }

    printf("\n[Test 6] Reject garbage\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);
        const char* garbage = "hello world this is not a valid envelope";
        for (size_t i = 0; i < strlen(garbage); i++)
            stream_push_byte(&sp, (uint8_t)garbage[i]);
        TEST("no event from garbage", stream_has_event(&sp) == 0);
        TEST("state is WAITING or INVALID",
             sp.state == STREAM_STATE_WAITING || sp.state == STREAM_STATE_INVALID);
    }

    printf("\n[Test 7] Partial pre-header reset\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);
        stream_push_byte(&sp, 0xFF);
        stream_push_byte(&sp, 0x00);
        stream_push_byte(&sp, 0xFF);
        TEST("resets on second 0xFF during header",
             sp.state == STREAM_STATE_HEADER && sp.filled == 1);
    }

    printf("\n[Test 8] Binary sensor injection\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);
        int r = stream_inject_sensor(&sp, 12, 1);
        TEST("inject_sensor ok", r == 0);
        TEST("event received", stream_has_event(&sp) == 1);

        StreamEvent evt;
        stream_pop_event(&sp, &evt);
        TEST("sensor event valid", evt.valid == 1);
        char rel[256];
        omi_env_to_relation(&evt.envelope, rel, sizeof(rel));
        printf("    Sensor relation: %s\n", rel);
    }

    printf("\n[Test 9] Binary event injection\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);
        stream_inject_event(&sp, "button", 0x12345678);
        TEST("event received", stream_has_event(&sp) == 1);
        StreamEvent evt;
        stream_pop_event(&sp, &evt);
        TEST("event valid", evt.valid == 1);
    }

    printf("\n[Test 10] Binary hardware injection\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);
        stream_inject_hardware(&sp, "relay", "on");
        TEST("hw event received", stream_has_event(&sp) == 1);
        StreamEvent evt;
        stream_pop_event(&sp, &evt);
        TEST("hw event valid", evt.valid == 1);
    }

    printf("\n[Test 11] Multiple sequential events\n");
    {
        StreamParser sp;
        stream_parser_init(&sp);
        stream_inject_sensor(&sp, 0, 0);
        StreamEvent evt;
        stream_pop_event(&sp, &evt);
        TEST("first event", evt.valid == 1);
        stream_inject_sensor(&sp, 1, 1);
        stream_pop_event(&sp, &evt);
        TEST("second event", evt.valid == 1);
        stream_inject_sensor(&sp, 2, 0);
        stream_pop_event(&sp, &evt);
        TEST("third event", evt.valid == 1);
    }

    printf("\n[Test 12] 512-byte sector iterator\n");
    {
        uint8_t sector[OMI_SECTOR_SIZE];
        memset(sector, 0, sizeof(sector));
        int expected_valid = 5;
        for (int c = 0; c < expected_valid; c++) {
            uint8_t* cell = sector + (c * OMI_ENV_SIZE);
            cell[0] = 0xFF; cell[1] = 0x00; cell[2] = 0x1C; cell[3] = 0x1D;
            cell[4] = 0x1E; cell[5] = 0x1F; cell[6] = 0x20; cell[7] = 0xFF;
            char rel[32];
            snprintf(rel, sizeof(rel), "(cell%d)", c);
            memcpy(cell + 32, rel, strlen(rel) + 1);
        }

        SectorIterator iter;
        int r = sector_iter_init(&iter, sector, sizeof(sector));
        TEST("sector init ok", r == 0);
        TEST("count matches", sector_iter_count(&iter) == expected_valid);

        int count = 0;
        OMI_512_Envelope env;
        uint64_t board;
        while (sector_iter_next(&iter, &env, &board) == 0)
            count++;
        TEST("iterated all valid", count == expected_valid);
        TEST("no more cells", sector_iter_next(&iter, &env, &board) != 0);
    }

    printf("\n[Test 13] Relation string output\n");
    {
        OMI_512_Envelope env;
        memset(&env, 0, sizeof(env));
        memcpy(env.gauge, "\xFF\x00\x1C\x1D\x1E\x1F\x20\xFF", 8);
        memcpy(env.relation, "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456", 32);
        char buf[256];
        int r = omi_env_to_relation(&env, buf, sizeof(buf));
        TEST("relation string produced", r > 0);
        printf("    Relation: %s\n", buf);
    }

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
