#include "omi_omion.h"
#include <string.h>

void omiom_init(OMIOM_Resolver* r) {
    if (!r) return;
    memset(r, 0, sizeof(OMIOM_Resolver));
}

static uint64_t env_address(const OMI_512_Envelope* env) {
    if (!env) return 0;
    uint64_t addr = 0;
    for (int i = 0; i < 8; i++) {
        addr = (addr << 8) | env->target[i];
    }
    return addr;
}

int omiom_add_prefix(OMIOM_Resolver* r, uint64_t address, uint8_t prefix_len, uint8_t next_hop) {
    if (!r) return -1;
    if (prefix_len > 64) return -2;
    if (r->count >= OMIOM_MAX_PREFIXES) return -3;

    for (int i = 0; i < r->count; i++) {
        if (r->entries[i].valid &&
            r->entries[i].address == address &&
            r->entries[i].prefix_len == prefix_len) {
            r->entries[i].next_hop = next_hop;
            r->entries[i].layer = 0;
            return 0;
        }
    }

    r->entries[r->count].address    = address;
    r->entries[r->count].prefix_len = prefix_len;
    r->entries[r->count].next_hop   = next_hop;
    r->entries[r->count].layer      = 0;
    r->entries[r->count].valid      = 1;
    r->count++;
    return 0;
}

int omiom_remove_prefix(OMIOM_Resolver* r, uint64_t address, uint8_t prefix_len) {
    if (!r) return -1;
    for (int i = 0; i < r->count; i++) {
        if (r->entries[i].valid &&
            r->entries[i].address == address &&
            r->entries[i].prefix_len == prefix_len) {
            r->entries[i].valid = 0;
            return 0;
        }
    }
    return -2;
}

int omiom_resolve(OMIOM_Resolver* r, const OMI_512_Envelope* env, uint8_t* next_hop) {
    if (!r || !env || !next_hop) return -1;

    uint64_t addr = env_address(env);
    int best_idx = -1;
    int best_len = -1;

    for (int i = 0; i < r->count; i++) {
        if (!r->entries[i].valid) continue;
        uint8_t plen = r->entries[i].prefix_len;
        if (plen == 0) continue;

        uint64_t mask = (plen >= 64) ? ~0ULL : (~0ULL << (64 - plen));
        uint64_t masked_addr = addr & mask;
        uint64_t masked_prefix = r->entries[i].address & mask;

        if (masked_addr == masked_prefix && (int)plen > best_len) {
            best_idx = i;
            best_len = plen;
        }
    }

    if (best_idx >= 0) {
        r->entries[best_idx].layer = 9;
        *next_hop = r->entries[best_idx].next_hop;
        return 0;
    }

    return -2;
}

int omiom_cascade(OMIOM_Resolver* r, const OMI_512_Envelope* env, uint8_t* next_hop) {
    if (!r || !env || !next_hop) return -1;

    int ret = omiom_resolve(r, env, next_hop);
    if (ret == 0) return 0;

    uint32_t layers[OMIOM_CASCADE_LAYERS];
    layers[0] = omicron_resolve(env, 9);
    layers[1] = omicron_resolve(env, 10);
    layers[2] = omicron_resolve(env, 11);
    layers[3] = omicron_resolve(env, 12);

    for (int li = 0; li < OMIOM_CASCADE_LAYERS; li++) {
        if (layers[li] == 0) continue;
        uint8_t candidate_hop = (uint8_t)(layers[li] & 0xFF);
        if (candidate_hop == 0 || candidate_hop == 0xFF) continue;

        for (int i = 0; i < r->count; i++) {
            if (!r->entries[i].valid) continue;
            if ((r->entries[i].next_hop == candidate_hop ||
                 (r->entries[i].address & 0xFF) == candidate_hop)) {
                *next_hop = r->entries[i].next_hop;
                r->entries[i].layer = (uint8_t)(9 + li);
                return 1;
            }
        }

        *next_hop = candidate_hop;
        return 1;
    }

    *next_hop = 0;
    return -2;
}

int omiom_prefix_count(const OMIOM_Resolver* r) {
    if (!r) return 0;
    int c = 0;
    for (int i = 0; i < r->count; i++)
        if (r->entries[i].valid) c++;
    return c;
}
