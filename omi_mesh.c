#include "omi_mesh.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const uint8_t CANONICAL_PREHEADER[8] = {
    0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF
};

#define ROUTE_UPDATE_MARKER 0x01
#define ROUTE_DATA_MARKER   0x02
#define ROUTE_RREQ_MARKER   0x04
#define ROUTE_RREP_MARKER   0x05
#define MESH_STATUS_MARKER  0x03

static OMI_MeshTransport* cast(OMI_Transport* t) {
    return (OMI_MeshTransport*)t;
}

static uint32_t now_ms(OMI_MeshTransport* m) {
    return m->time_ms ? m->time_ms() : 0;
}

static void make_route_update(OMI_MeshTransport* m, OMI_512_Envelope* env) {
    memset(env, 0, sizeof(OMI_512_Envelope));
    memcpy(env->gauge, CANONICAL_PREHEADER, 8);
    env->orientation[0] = ROUTE_UPDATE_MARKER;
    env->target[0] = 0;
    env->target[1] = m->node_id;
    env->target[2] = m->seqno;
    int idx = 0;
    for (int i = 0; i < m->routes.count && idx < 28; i++) {
        if (!m->routes.entries[i].valid) continue;
        env->relation[idx++] = m->routes.entries[i].node_id;
        env->relation[idx++] = m->routes.entries[i].next_hop;
        env->relation[idx++] = m->routes.entries[i].metric;
        env->relation[idx++] = m->routes.entries[i].seqno;
    }
}

static int handle_route_update(OMI_MeshTransport* m, const OMI_512_Envelope* env) {
    uint8_t origin = env->target[1];
    uint8_t seqno = env->target[2];
    if (origin == m->node_id) return 0;

    int found = 0;
    for (int i = 0; i < m->routes.count; i++) {
        if (m->routes.entries[i].valid && m->routes.entries[i].node_id == origin) {
            if ((int8_t)(seqno - m->routes.entries[i].seqno) <= 0) return 0;
            m->routes.entries[i].next_hop = origin;
            m->routes.entries[i].metric = 1;
            m->routes.entries[i].seqno = seqno;
            m->routes.entries[i].timestamp = now_ms(m);
            found = 1;
            break;
        }
    }
    if (!found && m->routes.count < OMI_MESH_MAX_ROUTES) {
        m->routes.entries[m->routes.count].node_id = origin;
        m->routes.entries[m->routes.count].next_hop = origin;
        m->routes.entries[m->routes.count].metric = 1;
        m->routes.entries[m->routes.count].seqno = seqno;
        m->routes.entries[m->routes.count].timestamp = now_ms(m);
        m->routes.entries[m->routes.count].valid = 1;
        m->routes.count++;
        found = 1;
    }

    for (int i = 0; i + 3 < 28 && env->relation[i] != 0; i += 4) {
        uint8_t r_node = env->relation[i];
        uint8_t r_met  = env->relation[i + 2];
        uint8_t r_seq  = env->relation[i + 3];
        if (r_node == m->node_id || r_node == 0) continue;

        int r_found = 0;
        for (int j = 0; j < m->routes.count; j++) {
            if (m->routes.entries[j].valid && m->routes.entries[j].node_id == r_node) {
                if ((int8_t)(r_seq - m->routes.entries[j].seqno) > 0) {
                    m->routes.entries[j].next_hop = origin;
                    m->routes.entries[j].metric = r_met + 1;
                    m->routes.entries[j].seqno = r_seq;
                    m->routes.entries[j].timestamp = now_ms(m);
                }
                r_found = 1;
                break;
            }
        }
        if (!r_found && m->routes.count < OMI_MESH_MAX_ROUTES) {
            m->routes.entries[m->routes.count].node_id = r_node;
            m->routes.entries[m->routes.count].next_hop = origin;
            m->routes.entries[m->routes.count].metric = r_met + 1;
            m->routes.entries[m->routes.count].seqno = r_seq;
            m->routes.entries[m->routes.count].timestamp = now_ms(m);
            m->routes.entries[m->routes.count].valid = 1;
            m->routes.count++;
        }
    }
    return found;
}

static OMI_RouteEntry* find_route(OMI_MeshTransport* m, uint8_t dest_id) {
    for (int i = 0; i < m->routes.count; i++) {
        if (m->routes.entries[i].valid && m->routes.entries[i].node_id == dest_id)
            return &m->routes.entries[i];
    }
    return NULL;
}

static void expire_stale_routes(OMI_MeshTransport* m) {
    uint32_t now = now_ms(m);
    uint32_t threshold = (uint32_t)m->stale_timeout_s * 1000;
    for (int i = 0; i < m->routes.count; i++) {
        if (m->routes.entries[i].valid &&
            (now - m->routes.entries[i].timestamp) > threshold) {
            m->routes.entries[i].valid = 0;
        }
    }
}

static void enqueue_dead_letter_raw(OMI_MeshTransport* m, const OMI_512_Envelope* env,
                                     uint8_t dest_id, uint8_t origin_id,
                                     uint8_t ttl, uint8_t reason) {
    if (m->dead_letters.count >= OMI_MESH_DEAD_LETTER_MAX) return;
    OMI_DeadLetter* d = &m->dead_letters.entries[m->dead_letters.count];
    memcpy(&d->envelope, env, sizeof(OMI_512_Envelope));
    d->dest_id = dest_id;
    d->origin_id = origin_id;
    d->ttl = ttl;
    d->dropped_at = now_ms(m);
    d->reason = reason;
    m->dead_letters.count++;
}

static void enqueue_dead_letter(OMI_MeshTransport* m, const OMI_QueuedEnvelope* q,
                                 uint8_t reason) {
    enqueue_dead_letter_raw(m, &q->envelope, q->dest_id, q->origin_id, q->ttl, reason);
}

static int enqueue(OMI_MeshTransport* m, const OMI_512_Envelope* env,
                    uint8_t dest_id, uint8_t ttl) {
    if (m->queue_count >= OMI_MESH_MAX_QUEUE) return -1;
    OMI_QueuedEnvelope* q = &m->queue[m->queue_count];
    memcpy(&q->envelope, env, sizeof(OMI_512_Envelope));
    q->dest_id = dest_id;
    q->origin_id = m->node_id;
    q->ttl = ttl;
    q->enqueued_at = now_ms(m);
    q->retries = 0;
    q->valid = 1;
    m->queue_count++;
    return 0;
}

static int retry_queue(OMI_MeshTransport* m) {
    int sent = 0;
    for (int i = 0; i < m->queue_count; i++) {
        if (!m->queue[i].valid) continue;
        OMI_QueuedEnvelope* q = &m->queue[i];
        uint32_t age = now_ms(m) - q->enqueued_at;
        if ((age / 1000) < (uint32_t)m->retry_interval_s) continue;

        OMI_RouteEntry* r = find_route(m, q->dest_id);
        if (r) {
            q->envelope.target[0] = q->dest_id;
            q->envelope.recovery[0] = q->origin_id;
            q->envelope.recovery[1] = q->ttl;
            m->underlying->send(m->underlying, (const uint8_t*)&q->envelope,
                                sizeof(OMI_512_Envelope));
            q->valid = 0;
            sent++;
        } else {
            q->retries++;
            q->enqueued_at = now_ms(m);
            if (q->retries >= m->max_retries) {
                enqueue_dead_letter(m, q, 2);
                q->valid = 0;
            }
        }
    }
    int wi = 0;
    for (int ri = 0; ri < m->queue_count; ri++) {
        if (m->queue[ri].valid) {
            if (wi != ri) m->queue[wi] = m->queue[ri];
            wi++;
        }
    }
    m->queue_count = wi;
    return sent;
}

static void send_rreq(OMI_MeshTransport* m, uint8_t dest_id) {
    if (m->rreq_pending && m->pending_rreq_dest == dest_id) return;
    OMI_512_Envelope env;
    memset(&env, 0, sizeof(OMI_512_Envelope));
    env.orientation[0] = ROUTE_RREQ_MARKER;
    env.target[0] = dest_id;
    env.target[1] = m->node_id;
    env.target[2] = ++m->rreq_seqno;
    env.recovery[0] = 1;
    env.target[0] = 0xFF;
    m->underlying->send(m->underlying, (const uint8_t*)&env, sizeof(OMI_512_Envelope));
    m->pending_rreq_dest = dest_id;
    m->rreq_sent_at = now_ms(m);
    m->rreq_retries = 0;
    m->rreq_pending = 1;
}

static void handle_rreq(OMI_MeshTransport* m, const OMI_512_Envelope* env) {
    uint8_t dest_id = env->target[0];
    uint8_t origin_id = env->target[1];
    uint8_t seqno = env->target[2];
    uint8_t hop_count = env->recovery[0];

    if (dest_id == m->node_id) {
        OMI_RouteEntry* rt = find_route(m, origin_id);
        if (!rt && m->routes.count < OMI_MESH_MAX_ROUTES) {
            m->routes.entries[m->routes.count].node_id = origin_id;
            m->routes.entries[m->routes.count].next_hop = origin_id;
            m->routes.entries[m->routes.count].metric = hop_count;
            m->routes.entries[m->routes.count].seqno = 0;
            m->routes.entries[m->routes.count].timestamp = now_ms(m);
            m->routes.entries[m->routes.count].valid = 1;
            m->routes.count++;
        }
        OMI_512_Envelope rrep;
        memset(&rrep, 0, sizeof(OMI_512_Envelope));
        rrep.orientation[0] = ROUTE_RREP_MARKER;
        rrep.target[0] = origin_id;
        rrep.target[1] = m->node_id;
        rrep.target[2] = seqno;
        rrep.recovery[0] = hop_count;
        rrep.target[0] = 0xFF;
        m->underlying->send(m->underlying, (const uint8_t*)&rrep, sizeof(OMI_512_Envelope));
    } else {
        OMI_512_Envelope fwd;
        memcpy(&fwd, env, sizeof(OMI_512_Envelope));
        fwd.recovery[0] = hop_count + 1;
        fwd.target[0] = 0xFF;
        m->underlying->send(m->underlying, (const uint8_t*)&fwd, sizeof(OMI_512_Envelope));
    }
}

static void handle_rrep(OMI_MeshTransport* m, const OMI_512_Envelope* env) {
    uint8_t origin_id = env->target[0];
    uint8_t target_id = env->target[1];
    uint8_t hop_count = env->recovery[0];

    OMI_RouteEntry* rt = find_route(m, target_id);
    if (!rt && m->routes.count < OMI_MESH_MAX_ROUTES) {
        m->routes.entries[m->routes.count].node_id = target_id;
        m->routes.entries[m->routes.count].next_hop = target_id;
        m->routes.entries[m->routes.count].metric = hop_count;
        m->routes.entries[m->routes.count].seqno = 0;
        m->routes.entries[m->routes.count].timestamp = now_ms(m);
        m->routes.entries[m->routes.count].valid = 1;
        m->routes.count++;
    }
    if (origin_id == m->node_id) {
        m->rreq_pending = 0;
    }
}

static int flood_routes(OMI_MeshTransport* m) {
    OMI_512_Envelope update;
    m->seqno++;
    make_route_update(m, &update);
    return m->underlying->send(m->underlying, (const uint8_t*)&update,
                               sizeof(OMI_512_Envelope));
}

static int mesh_send(OMI_Transport* self, const uint8_t* data, size_t len) {
    OMI_MeshTransport* m = cast(self);
    if (!m || !m->initialized) return -1;

    OMI_512_Envelope env;
    if (len > sizeof(OMI_512_Envelope)) len = sizeof(OMI_512_Envelope);
    memcpy(&env, data, len);

    uint8_t marker = env.orientation[0];
    if (marker == ROUTE_UPDATE_MARKER) {
        handle_route_update(m, &env);
        return flood_routes(m);
    }

    uint8_t dest_id = env.target[0];
    uint8_t ttl = env.recovery[1];
    if (ttl == 0) ttl = OMI_MESH_TTL_DEFAULT;
    if (dest_id == 0xFF) {
        return m->underlying->send(m->underlying, data, len);
    }
    if (dest_id == m->node_id) {
        return (int)len;
    }

    OMI_RouteEntry* route = find_route(m, dest_id);
    if (route) {
        env.recovery[0] = m->node_id;
        env.recovery[1] = ttl - 1;
        if (env.recovery[1] == 0) return 0;
        env.target[0] = route->next_hop;
        return m->underlying->send(m->underlying, (const uint8_t*)&env,
                                   sizeof(OMI_512_Envelope));
    }

    send_rreq(m, dest_id);
    return enqueue(m, &env, dest_id, ttl);
}

static int mesh_recv(OMI_Transport* self, uint8_t* data, size_t maxlen, int timeout_ms) {
    OMI_MeshTransport* m = cast(self);
    if (!m || !m->initialized) return -1;
    if (!data || maxlen < sizeof(OMI_512_Envelope)) return -1;

    OMI_512_Envelope env;
    int ret = m->underlying->recv(m->underlying, (uint8_t*)&env,
                                   sizeof(OMI_512_Envelope), timeout_ms);
    if (ret <= 0) return ret;

    uint8_t marker = env.orientation[0];
    if (marker == ROUTE_UPDATE_MARKER) {
        handle_route_update(m, &env);
        return 0;
    }
    if (marker == ROUTE_RREQ_MARKER) {
        handle_rreq(m, &env);
        return 0;
    }
    if (marker == ROUTE_RREP_MARKER) {
        handle_rrep(m, &env);
        return 0;
    }

    uint8_t dest = env.target[0];
    if (dest == m->node_id || dest == 0xFF) {
        memcpy(data, &env, sizeof(OMI_512_Envelope));
        return (int)sizeof(OMI_512_Envelope);
    }

    uint8_t ttl = env.recovery[1];
    if (ttl <= 1) {
        enqueue_dead_letter_raw(m, &env, dest, env.recovery[0], ttl, 1);
        return 0;
    }
    env.recovery[1] = ttl - 1;

    OMI_RouteEntry* route = find_route(m, dest);
    if (route) {
        env.target[0] = route->next_hop;
        m->underlying->send(m->underlying, (const uint8_t*)&env,
                            sizeof(OMI_512_Envelope));
    } else {
        enqueue(m, &env, dest, ttl - 1);
    }
    return 0;
}

static void mesh_tick(OMI_MeshTransport* m) {
    if (!m || !m->initialized) return;
    uint32_t now = now_ms(m);

    expire_stale_routes(m);

    if (m->rreq_pending &&
        (now - m->rreq_sent_at) > OMI_MESH_RREQ_RETRY_INTERVAL &&
        m->rreq_retries < OMI_MESH_RREQ_MAX_RETRIES) {
        m->rreq_retries++;
        OMI_512_Envelope env;
        memset(&env, 0, sizeof(OMI_512_Envelope));
        env.orientation[0] = ROUTE_RREQ_MARKER;
        env.target[0] = m->pending_rreq_dest;
        env.target[1] = m->node_id;
        env.target[2] = m->rreq_seqno;
        env.recovery[0] = 1;
        env.target[0] = 0xFF;
        m->underlying->send(m->underlying, (const uint8_t*)&env, sizeof(OMI_512_Envelope));
        m->rreq_sent_at = now;
    }

    if (now - m->last_flood > (uint32_t)m->flood_interval_s * 1000) {
        flood_routes(m);
        m->last_flood = now;
    }

    retry_queue(m);
}

static int mesh_available(OMI_Transport* self) {
    OMI_MeshTransport* m = cast(self);
    if (!m || !m->initialized) return 0;
    mesh_tick(m);
    return m->underlying->available(m->underlying);
}

static int mesh_flush(OMI_Transport* self) {
    OMI_MeshTransport* m = cast(self);
    if (!m) return -1;
    return m->underlying->flush(m->underlying);
}

OMI_Transport* omi_mesh_create(OMI_Transport* underlying, uint8_t node_id,
                                uint64_t (*time_ms)(void)) {
    if (!underlying) return NULL;
    OMI_MeshTransport* m = (OMI_MeshTransport*)calloc(1, sizeof(OMI_MeshTransport));
    if (!m) return NULL;

    m->base.send      = mesh_send;
    m->base.recv      = mesh_recv;
    m->base.available = mesh_available;
    m->base.flush     = mesh_flush;
    m->base.ctx       = NULL;
    m->underlying   = underlying;
    m->node_id      = node_id;
    m->seqno        = 0;
    m->last_flood   = 0;
    m->flood_interval_s  = OMI_MESH_FLOOD_INTERVAL;
    m->retry_interval_s  = OMI_MESH_RETRY_INTERVAL;
    m->max_retries      = OMI_MESH_MAX_RETRIES;
    m->stale_timeout_s  = OMI_MESH_STALE_TIMEOUT;
    m->time_ms      = time_ms;
    m->initialized  = 1;

    m->routes.count = 0;
    m->queue_count  = 0;
    m->dead_letters.count = 0;
    m->rreq_pending = 0;
    m->pending_rreq_dest = 0;
    m->rreq_seqno = 0;
    m->rreq_sent_at = 0;
    m->rreq_retries = 0;

    return &m->base;
}

void omi_mesh_destroy(OMI_Transport* t) {
    if (!t) return;
    OMI_MeshTransport* m = cast(t);
    free(m);
}

int omi_mesh_get_route_count(OMI_Transport* t) {
    OMI_MeshTransport* m = cast(t);
    if (!m) return 0;
    expire_stale_routes(m);
    int c = 0;
    for (int i = 0; i < m->routes.count; i++)
        if (m->routes.entries[i].valid) c++;
    return c;
}

const OMI_RouteEntry* omi_mesh_get_route(OMI_Transport* t, int index) {
    OMI_MeshTransport* m = cast(t);
    if (!m) return NULL;
    int c = 0;
    for (int i = 0; i < m->routes.count; i++) {
        if (m->routes.entries[i].valid) {
            if (c == index) return &m->routes.entries[i];
            c++;
        }
    }
    return NULL;
}

int omi_mesh_queue_depth(OMI_Transport* t) {
    OMI_MeshTransport* m = cast(t);
    if (!m) return 0;
    return m->queue_count;
}

int omi_mesh_dead_letter_count(OMI_Transport* t) {
    OMI_MeshTransport* m = cast(t);
    if (!m) return 0;
    return m->dead_letters.count;
}

const OMI_DeadLetter* omi_mesh_get_dead_letter(OMI_Transport* t, int index) {
    OMI_MeshTransport* m = cast(t);
    if (!m || index < 0 || index >= m->dead_letters.count) return NULL;
    return &m->dead_letters.entries[index];
}
