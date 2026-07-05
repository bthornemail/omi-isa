#ifndef OMI_MESH_H
#define OMI_MESH_H

#include "omienv.h"
#include "omi_transport.h"

#define OMI_MESH_MAX_ROUTES    32
#define OMI_MESH_MAX_QUEUE     64
#define OMI_MESH_MAX_NEIGHBORS 16
#define OMI_MESH_TTL_DEFAULT   15
#define OMI_MESH_FLOOD_INTERVAL 30
#define OMI_MESH_RETRY_INTERVAL 5
#define OMI_MESH_MAX_RETRIES    3
#define OMI_MESH_STALE_TIMEOUT  120

typedef struct {
    uint8_t node_id;
    uint8_t next_hop;
    uint8_t metric;
    uint8_t seqno;
    uint32_t timestamp;
    int valid;
} OMI_RouteEntry;

typedef struct {
    OMI_RouteEntry entries[OMI_MESH_MAX_ROUTES];
    int count;
} OMI_RoutingTable;

typedef struct {
    OMI_512_Envelope envelope;
    uint8_t dest_id;
    uint8_t origin_id;
    uint8_t ttl;
    uint32_t enqueued_at;
    int retries;
    int valid;
} OMI_QueuedEnvelope;

typedef struct {
    OMI_Transport base;
    OMI_Transport* underlying;
    uint8_t node_id;
    uint8_t seqno;
    OMI_RoutingTable routes;
    OMI_QueuedEnvelope queue[OMI_MESH_MAX_QUEUE];
    int queue_count;
    uint32_t last_flood;
    int flood_interval_s;
    int retry_interval_s;
    int max_retries;
    int stale_timeout_s;
    int initialized;
    uint64_t (*time_ms)(void);
} OMI_MeshTransport;

OMI_Transport* omi_mesh_create(OMI_Transport* underlying, uint8_t node_id,
                                uint64_t (*time_ms)(void));
void omi_mesh_destroy(OMI_Transport* t);
int omi_mesh_get_route_count(OMI_Transport* t);
const OMI_RouteEntry* omi_mesh_get_route(OMI_Transport* t, int index);
int omi_mesh_queue_depth(OMI_Transport* t);

#endif
