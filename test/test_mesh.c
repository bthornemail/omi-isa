#include "omi_mesh.h"
#include "omi_transport_sim.h"
#include "omienv.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_passed = 0;
static int tests_run = 0;
static uint64_t fake_time = 0;

static uint64_t fake_time_ms(void) {
    return fake_time;
}

static void TEST(const char* name, int cond) {
    tests_run++;
    if (cond) tests_passed++;
    printf("  %s: %s\n", cond ? "PASS" : "FAIL", name);
}

int main(void) {
    printf("=== OMI Mesh Test ===\n\n");

    /* ---- Test 1: create mesh nodes ---- */
    {
        printf("[Test 1] Mesh node creation\n");
        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        OMI_Transport* mesh_a = omi_mesh_create(&ta.base, 1, fake_time_ms);
        OMI_Transport* mesh_b = omi_mesh_create(&tb.base, 2, fake_time_ms);

        TEST("mesh_a not null", mesh_a != NULL);
        TEST("mesh_b not null", mesh_b != NULL);
        TEST("route count A = 0", omi_mesh_get_route_count(mesh_a) == 0);
        TEST("route count B = 0", omi_mesh_get_route_count(mesh_b) == 0);

        omi_mesh_destroy(mesh_a);
        omi_mesh_destroy(mesh_b);
        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    /* ---- Test 2: route update flood via tick ---- */
    {
        printf("\n[Test 2] Route update flood via tick\n");
        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        OMI_Transport* mesh_a = omi_mesh_create(&ta.base, 1, fake_time_ms);
        OMI_Transport* mesh_b = omi_mesh_create(&tb.base, 2, fake_time_ms);

        fake_time = 31000;
        int avail = mesh_a->available(mesh_a);
        TEST("available triggers tick", avail >= 0);

        OMI_512_Envelope r;
        memset(&r, 0, sizeof(r));
        int ret = mesh_b->recv(mesh_b, (uint8_t*)&r, sizeof(r), 10);
        TEST("mesh_b consumed route update", ret == 0);
        TEST("mesh_b knows node 1", omi_mesh_get_route_count(mesh_b) >= 1);

        const OMI_RouteEntry* r1 = omi_mesh_get_route(mesh_b, 0);
        TEST("node 1 route", r1 != NULL && r1->node_id == 1);
        TEST("metric = 1", r1 != NULL && r1->metric == 1);

        omi_mesh_destroy(mesh_a);
        omi_mesh_destroy(mesh_b);
        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    /* ---- Test 3: data routing ---- */
    {
        printf("\n[Test 3] Data routing through mesh\n");
        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        OMI_Transport* mesh_a = omi_mesh_create(&ta.base, 1, fake_time_ms);
        OMI_Transport* mesh_b = omi_mesh_create(&tb.base, 2, fake_time_ms);

        fake_time = 31000;
        mesh_a->available(mesh_a);
        OMI_512_Envelope r;
        memset(&r, 0, sizeof(r));
        mesh_b->recv(mesh_b, (uint8_t*)&r, sizeof(r), 10);

        // Trigger mesh_b's tick to send its own route update
        mesh_b->available(mesh_b);
        memset(&r, 0, sizeof(r));
        mesh_a->recv(mesh_a, (uint8_t*)&r, sizeof(r), 10);

        OMI_512_Envelope data;
        memset(&data, 0, sizeof(data));
        memcpy(data.gauge, ((uint8_t[]){0xFF,0x00,0x1C,0x1D,0x1E,0x1F,0x20,0xFF}), 8);
        data.target[0] = 2;
        data.target[1] = 0x42;
        data.recovery[1] = 10;
        memcpy(data.relation, "hello from 1", 12);

        memset(&r, 0, sizeof(r));
        int ret = mesh_b->recv(mesh_b, (uint8_t*)&r, sizeof(r), 1);
        TEST("no data before send", ret <= 0);

        mesh_a->send(mesh_a, (const uint8_t*)&data, sizeof(data));

        memset(&r, 0, sizeof(r));
        ret = mesh_b->recv(mesh_b, (uint8_t*)&r, sizeof(r), 10);
        TEST("data envelope received", ret == (int)sizeof(OMI_512_Envelope));
        TEST("data payload intact", ret >= 0 && r.target[1] == 0x42);

        omi_mesh_destroy(mesh_a);
        omi_mesh_destroy(mesh_b);
        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    /* ---- Test 4: store-and-forward ---- */
    {
        printf("\n[Test 4] Store-and-forward\n");
        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        OMI_Transport* mesh_a = omi_mesh_create(&ta.base, 1, fake_time_ms);

        OMI_512_Envelope data;
        memset(&data, 0, sizeof(data));
        memcpy(data.gauge, ((uint8_t[]){0xFF,0x00,0x1C,0x1D,0x1E,0x1F,0x20,0xFF}), 8);
        data.target[0] = 99;
        data.relation[0] = 0xAA;

        TEST("queue depth 0", omi_mesh_queue_depth(mesh_a) == 0);

        mesh_a->send(mesh_a, (const uint8_t*)&data, sizeof(data));
        TEST("queue depth 1 after send to unknown", omi_mesh_queue_depth(mesh_a) == 1);

        fake_time = 6000;
        mesh_a->available(mesh_a);
        TEST("still queued after retry 1", omi_mesh_queue_depth(mesh_a) == 1);

        // retry_interval=5s, max_retries=3
        // After retry 3, retries >= max_retries -> entry dropped
        fake_time = 6000 + 5001;
        mesh_a->available(mesh_a);
        TEST("still queued after retry 2", omi_mesh_queue_depth(mesh_a) == 1);

        fake_time = 6000 + 5001 + 5001;
        mesh_a->available(mesh_a);
        TEST("empty after retry 3 (max exceeded)", omi_mesh_queue_depth(mesh_a) == 0);

        omi_mesh_destroy(mesh_a);
        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    /* ---- Test 5: route update via explicit send ---- */
    {
        printf("\n[Test 5] Route update via explicit send\n");
        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        OMI_Transport* mesh_a = omi_mesh_create(&ta.base, 1, fake_time_ms);
        OMI_Transport* mesh_b = omi_mesh_create(&tb.base, 2, fake_time_ms);

        OMI_512_Envelope update;
        memset(&update, 0, sizeof(update));
        memcpy(update.gauge, ((uint8_t[]){0xFF,0x00,0x1C,0x1D,0x1E,0x1F,0x20,0xFF}), 8);
        update.orientation[0] = 0x01;
        update.target[1] = 1;
        update.target[2] = 1;

        int ret = mesh_a->send(mesh_a, (const uint8_t*)&update, sizeof(update));
        TEST("route update send returns > 0", ret > 0);

        OMI_512_Envelope r;
        memset(&r, 0, sizeof(r));
        ret = mesh_b->recv(mesh_b, (uint8_t*)&r, sizeof(r), 10);
        TEST("mesh_b consumed route update from explicit send", ret == 0);
        TEST("mesh_b knows node 1", omi_mesh_get_route_count(mesh_b) >= 1);

        omi_mesh_destroy(mesh_a);
        omi_mesh_destroy(mesh_b);
        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    /* ---- Test 6: multi-hop metric ---- */
    {
        printf("\n[Test 6] Multi-hop metric counting\n");
        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        OMI_Transport* mesh_b = omi_mesh_create(&tb.base, 2, fake_time_ms);

        // Feed route update directly to underlying transport
        // to simulate receiving from node 1 which knows node 3
        OMI_512_Envelope update;
        memset(&update, 0, sizeof(update));
        memcpy(update.gauge, ((uint8_t[]){0xFF,0x00,0x1C,0x1D,0x1E,0x1F,0x20,0xFF}), 8);
        update.orientation[0] = 0x01;
        update.target[1] = 1;
        update.target[2] = 2;
        update.relation[0] = 3;
        update.relation[1] = 1;
        update.relation[2] = 1;
        update.relation[3] = 1;

        // Write to A's transport so it arrives at B's rx buffer
        OMI_Transport* underlying = &ta.base;
        underlying->send(underlying, (const uint8_t*)&update, sizeof(update));

        OMI_512_Envelope r;
        int ret = mesh_b->recv(mesh_b, (uint8_t*)&r, sizeof(r), 10);
        TEST("route update consumed", ret == 0);
        int rc = omi_mesh_get_route_count(mesh_b);
        TEST("node 1 route exists", rc >= 1);

        const OMI_RouteEntry* r1 = omi_mesh_get_route(mesh_b, 0);
        if (r1) {
            TEST("node 1 metric = 1", r1->metric == 1);
        }

        omi_mesh_destroy(mesh_b);
        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
