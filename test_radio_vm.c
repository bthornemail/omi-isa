#include "omi_transport_sim.h"
#include "omi_probe.h"
#include "omi_dispatch.h"
#include "stream.h"
#include <string.h>
#include <stdio.h>

#define TEST(name, cond) do { \
    printf("  %s: %s\n", name, (cond) ? "PASS" : "FAIL"); \
    total++; if (!(cond)) failed++; \
} while(0)

static int total = 0, failed = 0;

static void init_cpu_with_cap(OMI_CPU* cpu, uint32_t cap) {
    init_cpu(cpu);
    cpu->capability = cap;
    cpu->peer_capability = 0;
    cpu->isa_subset = 0;
    cpu->probe_state = PROBE_STATE_IDLE;
}

int main(void) {
    printf("=== Radio VM End-to-End Test ===\n");

    omi_dispatch_init();

    printf("\n[Test 1] Transport sim init + link\n");
    {
        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        const char* test_data = "HELLO";
        int r = ta.base.send(&ta.base, (const uint8_t*)test_data, 5);
        TEST("send A->B returns 5", r == 5);
        TEST("B has 5 bytes avail", tb.base.available(&tb.base) == 5);

        uint8_t buf[16];
        r = tb.base.recv(&tb.base, buf, sizeof(buf), 0);
        TEST("recv B returns 5", r == 5);
        TEST("data matches", memcmp(buf, "HELLO", 5) == 0);
        TEST("A->B buffer empty", tb.base.available(&tb.base) == 0);

        r = tb.base.send(&tb.base, (const uint8_t*)"WORLD", 5);
        TEST("send B->A returns 5", r == 5);
        TEST("A has 5 bytes avail", ta.base.available(&ta.base) == 5);

        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    printf("\n[Test 2] Transport envelope send/recv\n");
    {
        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        OMI_512_Envelope env_a;
        memset(&env_a, 0, sizeof(env_a));
        env_a.gauge[0] = 0xFF;
        env_a.target[0] = 42;

        int r = omi_transport_send_envelope(&ta.base, &env_a);
        TEST("send envelope returns 0", r == 0);

        OMI_512_Envelope env_b;
        r = omi_transport_recv_envelope(&tb.base, &env_b);
        TEST("recv envelope returns 0", r == 0);
        TEST("gauge[0] matches", env_b.gauge[0] == 0xFF);
        TEST("target[0] matches", env_b.target[0] == 42);

        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    printf("\n[Test 3] Probe session init\n");
    {
        OMI_CPU cpu;
        init_cpu_with_cap(&cpu, 0xABCD1234);
        OMI_Transport_Sim t;
        omi_transport_sim_init(&t);

        OMI_ProbeSession sess;
        int r = omi_probe_init(&sess, &t.base, &cpu);
        TEST("init returns 0", r == 0);
        TEST("state is IDLE", sess.state == OMI_PROBE_IDLE);
        TEST("vm set", sess.vm == &cpu);
        TEST("transport set", sess.transport == &t.base);
    }

    printf("\n[Test 4] Full probe handshake between two nodes\n");
    {
        OMI_CPU cpu_a, cpu_b;
        init_cpu_with_cap(&cpu_a, 0xFFFF0000);
        init_cpu_with_cap(&cpu_b, 0x00FFFF00);

        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        OMI_ProbeSession sess_a, sess_b;
        omi_probe_init(&sess_a, &ta.base, &cpu_a);
        omi_probe_init(&sess_b, &tb.base, &cpu_b);

        /* Manual orchestration: A initiates, B serves, repeat */
        int r = omi_probe_start(&sess_a);
        TEST("A start returns 0", r == 0);
        TEST("A state = SENT", sess_a.state == OMI_PROBE_SENT);
        TEST("A probe_state = PROBING", cpu_a.probe_state == PROBE_STATE_PROBING);

        /* B serves PROBE → sends PROBE_ACK */
        r = omi_probe_serve(&sess_b);
        TEST("B serve PROBE returns 0", r == 0);

        /* A processes PROBE_ACK */
        r = omi_probe_poll(&sess_a);
        TEST("A poll PROBE_ACK returns 0", r == 0);
        TEST("A state = ACKED", sess_a.state == OMI_PROBE_ACKED);
        TEST("A stored peer capability", sess_a.peer_capability == cpu_b.capability);
        TEST("A computed ISA subset",
             sess_a.isa_subset == (cpu_a.capability & cpu_b.capability));

        /* A sends SYNC_COMMIT */
        r = omi_probe_send_sync(&sess_a);
        TEST("A send sync returns 0", r == 0);

        /* B serves SYNC_COMMIT → sends ack */
        r = omi_probe_serve(&sess_b);
        TEST("B serve SYNC_COMMIT returns 0", r == 0);
        TEST("B probe_state = NEGOTIATED", cpu_b.probe_state == PROBE_STATE_NEGOTIATED);

        /* A processes SYNC_COMMIT ack */
        r = omi_probe_poll(&sess_a);
        if (r == 0 || r == -3) {
            int ok = (sess_a.state == OMI_PROBE_SYNCED);
            if (r == -3) ok = 0;
            TEST("A state = SYNCED", ok);
        }

        /* Verify negotiated state */
        uint32_t expected_subset = cpu_a.capability & cpu_b.capability;
        TEST("B peer_capability stored", cpu_b.peer_capability == cpu_a.capability);
        TEST("B isa_subset computed",
             cpu_b.isa_subset == expected_subset);

        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    printf("\n[Test 5] Transport + stream + dispatch pipeline\n");
    {
        OMI_CPU cpu;
        init_cpu_with_cap(&cpu, 0xFFFFFFFF);
        cpu.probe_state = PROBE_STATE_NEGOTIATED;

        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        StreamParser sp;
        stream_parser_init(&sp);
        sp.vm = &cpu;
        sp.auto_dispatch = 1;

        OMI_512_Envelope probe;
        memset(&probe, 0, sizeof(probe));
        uint8_t pre[8] = {0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF};
        memcpy(probe.gauge, pre, 8);
        probe.target[0] = PROBE;
        probe.orientation[0] = 0xAB;
        probe.orientation[1] = 0xCD;
        probe.orientation[2] = 0x00;
        probe.orientation[3] = 0x00;

        int r = omi_transport_send_envelope(&ta.base, &probe);
        TEST("send probe envelope", r == 0);

        /* Read from B's transport into stream parser */
        uint8_t buf[OMI_ENV_SIZE];
        size_t total_read = 0;
        while (total_read < OMI_ENV_SIZE) {
            r = tb.base.recv(&tb.base, buf + total_read,
                             OMI_ENV_SIZE - total_read, 0);
            if (r > 0) {
                for (size_t i = 0; i < (size_t)r; i++)
                    stream_push_byte(&sp, buf[total_read + i]);
                total_read += (size_t)r;
            } else {
                break;
            }
        }
        TEST("stream completed", sp.state == STREAM_STATE_COMPLETE);
        TEST("dispatch ran", sp.dispatch_result == 0);
        TEST("response generated", sp.response_len > 0);

        StreamEvent evt;
        r = stream_pop_event(&sp, &evt);
        TEST("pop event returns 0", r == 0);
        TEST("event opcode is PROBE", evt.opcode == PROBE);
        TEST("event valid", evt.valid);

        OMI_512_Envelope* resp = (OMI_512_Envelope*)evt.response;
        int resp_op = resp->target[0] & 0x1F;
        TEST("response opcode is PROBE_ACK", resp_op == PROBE_ACK);

        omi_transport_sim_unlink(&ta);
        omi_transport_sim_unlink(&tb);
    }

    printf("\n[Test 6] End-to-end: two full VM nodes\n");
    {
        OMI_CPU cpu_a, cpu_b;
        init_cpu_with_cap(&cpu_a, 0xF0F0F0F0);
        init_cpu_with_cap(&cpu_b, 0x0F0F0F0F);

        OMI_Transport_Sim ta, tb;
        omi_transport_sim_init(&ta);
        omi_transport_sim_init(&tb);
        omi_transport_sim_link(&ta, &tb);

        StreamParser spa, spb;
        stream_parser_init(&spa);
        stream_parser_init(&spb);
        spa.vm = &cpu_a;
        spb.vm = &cpu_b;
        spa.auto_dispatch = 1;
        spb.auto_dispatch = 1;

        /* A initiates probe */
        OMI_512_Envelope probe;
        memset(&probe, 0, sizeof(probe));
        uint8_t pre[8] = {0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF};
        memcpy(probe.gauge, pre, 8);
        probe.target[0] = PROBE;
        probe.orientation[0] = (uint8_t)((cpu_a.capability >> 24) & 0xFF);
        probe.orientation[1] = (uint8_t)((cpu_a.capability >> 16) & 0xFF);
        probe.orientation[2] = (uint8_t)((cpu_a.capability >> 8) & 0xFF);
        probe.orientation[3] = (uint8_t)(cpu_a.capability & 0xFF);

        omi_transport_send_envelope(&ta.base, &probe);

        /* B's transport → stream → dispatch */
        uint8_t buf[OMI_ENV_SIZE];
        while (tb.base.available(&tb.base) > 0) {
            int n = tb.base.recv(&tb.base, buf, OMI_ENV_SIZE, 0);
            for (int i = 0; i < n; i++)
                stream_push_byte(&spb, buf[i]);
        }
        TEST("B completed PROBE", spb.state == STREAM_STATE_COMPLETE);
        TEST("B dispatch ok", spb.dispatch_result == 0);

        /* B's response → transport back to A */
        StreamEvent evt_b;
        stream_pop_event(&spb, &evt_b);
        if (evt_b.response_len > 0) {
            omi_transport_send_envelope(&tb.base,
                (const OMI_512_Envelope*)evt_b.response);
        }

        /* A receives PROBE_ACK */
        while (ta.base.available(&ta.base) > 0) {
            int n = ta.base.recv(&ta.base, buf, OMI_ENV_SIZE, 0);
            for (int i = 0; i < n; i++)
                stream_push_byte(&spa, buf[i]);
        }
        TEST("A completed PROBE_ACK", spa.state == STREAM_STATE_COMPLETE);
        TEST("A dispatch ok", spa.dispatch_result == 0);
        TEST("A stored peer cap", cpu_a.peer_capability == cpu_b.capability);
        uint32_t expected = cpu_a.capability & cpu_b.capability;
        TEST("A isa_subset correct", cpu_a.isa_subset == expected);
    }

    printf("\n=== Results: %d/%d passed ===\n", total - failed, total);
    return failed ? 1 : 0;
}
