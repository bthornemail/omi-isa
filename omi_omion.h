#ifndef OMI_OMIOM_H
#define OMI_OMIOM_H

#include "omienv.h"
#include "omicron.h"

#define OMIOM_MAX_PREFIXES  64
#define OMIOM_CASCADE_LAYERS 4

typedef struct {
    uint64_t address;
    uint8_t  prefix_len;
    uint8_t  next_hop;
    uint8_t  layer;
    int      valid;
} OMIOM_PrefixEntry;

typedef struct {
    OMIOM_PrefixEntry entries[OMIOM_MAX_PREFIXES];
    int count;
} OMIOM_Resolver;

void omiom_init(OMIOM_Resolver* r);
int  omiom_add_prefix(OMIOM_Resolver* r, uint64_t address, uint8_t prefix_len, uint8_t next_hop);
int  omiom_remove_prefix(OMIOM_Resolver* r, uint64_t address, uint8_t prefix_len);
int  omiom_resolve(OMIOM_Resolver* r, const OMI_512_Envelope* env, uint8_t* next_hop);
int  omiom_cascade(OMIOM_Resolver* r, const OMI_512_Envelope* env, uint8_t* next_hop);
int  omiom_prefix_count(const OMIOM_Resolver* r);

#endif
