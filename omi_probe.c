#include "omi_probe.h"
#include "omi_dispatch.h"
#include <string.h>

static const uint8_t CANONICAL_PREHEADER[8] = {
    0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF
};

int omi_probe_init(OMI_ProbeSession* sess, OMI_Transport* t, OMI_CPU* vm) {
    if (!sess || !t || !vm) return -1;
    sess->transport = t;
    sess->vm = vm;
    sess->state = OMI_PROBE_IDLE;
    sess->peer_capability = 0;
    sess->isa_subset = 0;
    sess->timeout_ms = OMI_PROBE_TIMEOUT_MS;
    return 0;
}

static int send_probe(OMI_ProbeSession* sess) {
    OMI_512_Envelope env;
    memset(&env, 0, sizeof(env));
    memcpy(env.gauge, CANONICAL_PREHEADER, 8);
    env.target[0] = PROBE;
    env.orientation[0] = (uint8_t)((sess->vm->capability >> 24) & 0xFF);
    env.orientation[1] = (uint8_t)((sess->vm->capability >> 16) & 0xFF);
    env.orientation[2] = (uint8_t)((sess->vm->capability >> 8) & 0xFF);
    env.orientation[3] = (uint8_t)(sess->vm->capability & 0xFF);
    return omi_transport_send_envelope(sess->transport, &env);
}

int omi_probe_send_sync(OMI_ProbeSession* sess) {
    if (!sess) return -1;
    OMI_512_Envelope env;
    memset(&env, 0, sizeof(env));
    memcpy(env.gauge, CANONICAL_PREHEADER, 8);
    env.target[0] = SYNC_COMMIT;
    env.orientation[0] = 1;
    return omi_transport_send_envelope(sess->transport, &env);
}

static int recv_and_dispatch(OMI_ProbeSession* sess) {
    if (!sess) return -1;

    OMI_512_Envelope env;
    int r = omi_transport_recv_envelope(sess->transport, &env);
    if (r != 0) return r;

    uint8_t resp_buf[OMI_ENV_SIZE];
    size_t resp_len = 0;

    OMI_DispatchContext ctx;
    ctx.env = &env;
    ctx.bitboard = 0;
    ctx.vm = sess->vm;
    ctx.tx_buffer = resp_buf;
    ctx.tx_capacity = OMI_ENV_SIZE;
    ctx.tx_len = &resp_len;
    ctx.transport_ctx = NULL;

    int dr = omi_dispatch_execute(&ctx);
    if (dr != 0) return dr;

    if (resp_len > 0) {
        r = omi_transport_send_envelope(sess->transport,
            (const OMI_512_Envelope*)resp_buf);
        if (r != 0) return r;
    }

    int op = omi_env_get_opcode(&env);
    switch (op) {
        case PROBE:
            break;
        case PROBE_ACK:
            sess->state = OMI_PROBE_ACKED;
            sess->peer_capability = sess->vm->peer_capability;
            sess->isa_subset = sess->vm->isa_subset;
            break;
        case SYNC_COMMIT:
            sess->state = OMI_PROBE_SYNCED;
            break;
        default:
            break;
    }
    return 0;
}

int omi_probe_start(OMI_ProbeSession* sess) {
    if (!sess || !sess->transport || !sess->vm) return -1;
    int r = send_probe(sess);
    if (r != 0) return r;
    sess->state = OMI_PROBE_SENT;
    sess->vm->probe_state = PROBE_STATE_PROBING;
    return 0;
}

int omi_probe_poll(OMI_ProbeSession* sess) {
    if (!sess) return -1;
    if (sess->state < OMI_PROBE_SENT) return -2;
    return recv_and_dispatch(sess);
}

int omi_probe_serve(OMI_ProbeSession* sess) {
    if (!sess) return -1;
    return recv_and_dispatch(sess);
}

int omi_probe_handshake(OMI_ProbeSession* sess) {
    if (!sess) return -1;

    int r = omi_probe_start(sess);
    if (r != 0) return r;

    for (int i = 0; i < OMI_PROBE_MAX_ATTEMPTS; i++) {
        r = omi_probe_poll(sess);
        if (r == -3) continue;
        if (r < 0) return r;

        if (sess->state == OMI_PROBE_ACKED) {
            r = omi_probe_send_sync(sess);
            if (r != 0) return r;
            continue;
        }

        if (sess->state == OMI_PROBE_SYNCED) {
            sess->vm->probe_state = PROBE_STATE_NEGOTIATED;
            return 0;
        }
    }

    sess->state = OMI_PROBE_FAILED;
    return -4;
}
