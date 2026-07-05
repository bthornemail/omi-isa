#ifndef OMI_PROBE_H
#define OMI_PROBE_H

#include "omi_transport.h"
#include "cpu.h"
#include "omienv.h"

#define OMI_PROBE_TIMEOUT_MS 1000
#define OMI_PROBE_MAX_ATTEMPTS 100

typedef enum {
    OMI_PROBE_IDLE,
    OMI_PROBE_SENT,
    OMI_PROBE_ACKED,
    OMI_PROBE_SYNCED,
    OMI_PROBE_FAILED
} OMI_ProbeState;

typedef struct {
    OMI_Transport* transport;
    OMI_CPU* vm;
    OMI_ProbeState state;
    uint32_t peer_capability;
    uint32_t isa_subset;
    int timeout_ms;
} OMI_ProbeSession;

int omi_probe_init(OMI_ProbeSession* sess, OMI_Transport* t, OMI_CPU* vm);
int omi_probe_start(OMI_ProbeSession* sess);
int omi_probe_send_sync(OMI_ProbeSession* sess);
int omi_probe_poll(OMI_ProbeSession* sess);
int omi_probe_serve(OMI_ProbeSession* sess);
int omi_probe_handshake(OMI_ProbeSession* sess);

#endif
