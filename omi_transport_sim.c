#include "omi_transport_sim.h"
#include <string.h>

static int sim_send(OMI_Transport* self, const uint8_t* data, size_t len) {
    if (!self || !data) return -1;
    OMI_Transport_Sim* t = (OMI_Transport_Sim*)self;
    if (!t->tx_peer) return -2;
    if (t->tx_peer->count + len > OMI_SIM_BUF_SIZE) return -3;
    memcpy(t->tx_peer->data + t->tx_peer->count, data, len);
    t->tx_peer->count += len;
    return (int)len;
}

static int sim_recv(OMI_Transport* self, uint8_t* data, size_t maxlen, int timeout_ms) {
    (void)timeout_ms;
    if (!self || !data) return -1;
    OMI_Transport_Sim* t = (OMI_Transport_Sim*)self;
    if (t->rx.count == 0) return 0;
    size_t to_copy = t->rx.count < maxlen ? t->rx.count : maxlen;
    memcpy(data, t->rx.data, to_copy);
    if (to_copy < t->rx.count)
        memmove(t->rx.data, t->rx.data + to_copy, t->rx.count - to_copy);
    t->rx.count -= to_copy;
    return (int)to_copy;
}

static int sim_available(OMI_Transport* self) {
    if (!self) return 0;
    OMI_Transport_Sim* t = (OMI_Transport_Sim*)self;
    return (int)t->rx.count;
}

static int sim_flush(OMI_Transport* self) {
    (void)self;
    return 0;
}

void omi_transport_sim_init(OMI_Transport_Sim* t) {
    if (!t) return;
    memset(t, 0, sizeof(*t));
    t->base.send = sim_send;
    t->base.recv = sim_recv;
    t->base.available = sim_available;
    t->base.flush = sim_flush;
    t->base.ctx = NULL;
}

void omi_transport_sim_link(OMI_Transport_Sim* a, OMI_Transport_Sim* b) {
    if (!a || !b) return;
    a->tx_peer = &b->rx;
    b->tx_peer = &a->rx;
}

void omi_transport_sim_unlink(OMI_Transport_Sim* t) {
    if (!t) return;
    t->tx_peer = NULL;
}
