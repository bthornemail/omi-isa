#ifndef OMI_DISPATCH_H
#define OMI_DISPATCH_H

#include "omienv.h"
#include "cpu.h"
#include "isa.h"
#include <stddef.h>

#define OMI_DISPATCH_SLOTS 32

typedef struct {
    const OMI_512_Envelope* env;
    uint64_t bitboard;
    OMI_CPU* vm;
    uint8_t* tx_buffer;
    size_t tx_capacity;
    size_t* tx_len;
    void* transport_ctx;
} OMI_DispatchContext;

typedef int (*OMI_OpcodeHandler)(OMI_DispatchContext* ctx);

extern OMI_OpcodeHandler omi_dispatch_table[OMI_DISPATCH_SLOTS];

void omi_dispatch_init(void);
void omi_dispatch_set(uint8_t opcode, OMI_OpcodeHandler handler);
int omi_dispatch_execute(OMI_DispatchContext* ctx);

int omi_env_get_opcode(const OMI_512_Envelope* env);

#endif
