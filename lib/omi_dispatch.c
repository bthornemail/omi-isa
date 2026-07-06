#include "omi_dispatch.h"
#include "gauge_exec.h"
#include <string.h>

OMI_OpcodeHandler omi_dispatch_table[OMI_DISPATCH_SLOTS];

static const uint8_t CANONICAL_PREHEADER[8] = {
    0xFF, 0x00, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0xFF
};

int omi_env_get_opcode(const OMI_512_Envelope* env) {
    if (!env) return -1;
    return env->target[0] & 0x1F;
}

/* ---- handler stubs for 0x00..0x13 (existing VM ops) ---- */
static int handle_nop(OMI_DispatchContext* ctx) {
    (void)ctx;
    return 0;
}
static int handle_mov(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_load(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_store(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_xor(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_rotl(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_rotr(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_add(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_sub(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_car(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_cdr(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_cmp(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_jmp(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_jz(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_delta(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_halt(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_syscall(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_loadm(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_call(OMI_DispatchContext* ctx) { (void)ctx; return 0; }
static int handle_ret(OMI_DispatchContext* ctx) { (void)ctx; return 0; }

/* ---- 0x14: PROBE ---- */
static int handle_probe(OMI_DispatchContext* ctx) {
    if (!ctx || !ctx->tx_buffer || !ctx->tx_len) return -1;
    if (ctx->vm && ctx->env) {
        uint32_t cap = ((uint32_t)ctx->env->orientation[0] << 24)
                     | ((uint32_t)ctx->env->orientation[1] << 16)
                     | ((uint32_t)ctx->env->orientation[2] << 8)
                     | (uint32_t)ctx->env->orientation[3];
        ctx->vm->peer_capability = cap;
        ctx->vm->isa_subset = ctx->vm->capability & cap;
    }
    OMI_512_Envelope ack;
    memset(&ack, 0, sizeof(ack));
    memcpy(ack.gauge, CANONICAL_PREHEADER, 8);
    ack.target[0] = PROBE_ACK;
    if (ctx->vm) {
        ack.orientation[0] = (ctx->vm->capability >> 24) & 0xFF;
        ack.orientation[1] = (ctx->vm->capability >> 16) & 0xFF;
        ack.orientation[2] = (ctx->vm->capability >> 8) & 0xFF;
        ack.orientation[3] = ctx->vm->capability & 0xFF;
    }
    size_t copy = sizeof(ack) < ctx->tx_capacity ? sizeof(ack) : ctx->tx_capacity;
    memcpy(ctx->tx_buffer, &ack, copy);
    *ctx->tx_len = copy;
    return 0;
}

/* ---- 0x15: PROBE_ACK ---- */
static int handle_probe_ack(OMI_DispatchContext* ctx) {
    if (!ctx || !ctx->vm) return -1;
    uint32_t cap = ((uint32_t)ctx->env->orientation[0] << 24)
                 | ((uint32_t)ctx->env->orientation[1] << 16)
                 | ((uint32_t)ctx->env->orientation[2] << 8)
                 | (uint32_t)ctx->env->orientation[3];
    ctx->vm->peer_capability = cap;
    ctx->vm->isa_subset = ctx->vm->capability & cap;
    ctx->vm->probe_state = PROBE_STATE_NEGOTIATED;
    return 0;
}

/* ---- 0x16: SYNC_COMMIT ---- */
static int handle_sync_commit(OMI_DispatchContext* ctx) {
    if (!ctx || !ctx->vm) return -1;
    if (ctx->vm->probe_state == PROBE_STATE_NEGOTIATED) return 0;
    ctx->vm->probe_state = PROBE_STATE_NEGOTIATED;
    if (ctx->tx_buffer && ctx->tx_len) {
        OMI_512_Envelope ack;
        memset(&ack, 0, sizeof(ack));
        memcpy(ack.gauge, CANONICAL_PREHEADER, 8);
        ack.target[0] = SYNC_COMMIT;
        ack.orientation[0] = 1;
        size_t copy = sizeof(ack) < ctx->tx_capacity ? sizeof(ack) : ctx->tx_capacity;
        memcpy(ctx->tx_buffer, &ack, copy);
        *ctx->tx_len = copy;
    }
    return 0;
}

/* ---- 0x17: SEAL ---- */
static int handle_seal(OMI_DispatchContext* ctx) {
    if (!ctx || !ctx->vm) return -1;
    if (ctx->tx_buffer && ctx->tx_len) {
        OMI_512_Envelope res;
        memset(&res, 0, sizeof(res));
        memcpy(res.gauge, CANONICAL_PREHEADER, 8);
        res.target[0] = 0;
        res.relation[0] = (ctx->bitboard >> 56) & 0xFF;
        res.relation[1] = (ctx->bitboard >> 48) & 0xFF;
        res.relation[2] = (ctx->bitboard >> 40) & 0xFF;
        res.relation[3] = (ctx->bitboard >> 32) & 0xFF;
        res.relation[4] = (ctx->bitboard >> 24) & 0xFF;
        res.relation[5] = (ctx->bitboard >> 16) & 0xFF;
        res.relation[6] = (ctx->bitboard >> 8) & 0xFF;
        res.relation[7] = ctx->bitboard & 0xFF;
        size_t copy = sizeof(res) < ctx->tx_capacity ? sizeof(res) : ctx->tx_capacity;
        memcpy(ctx->tx_buffer, &res, copy);
        *ctx->tx_len = copy;
    }
    return 0;
}

/* ---- 0x18: ROUTE ---- */
static int handle_route(OMI_DispatchContext* ctx) {
    if (!ctx || !ctx->tx_buffer || !ctx->tx_len) return -1;
    uint8_t place = omi_bb_get_place(ctx->bitboard);
    size_t copy = sizeof(OMI_512_Envelope) < ctx->tx_capacity
                  ? sizeof(OMI_512_Envelope) : ctx->tx_capacity;
    memcpy(ctx->tx_buffer, ctx->env, copy);
    ctx->tx_buffer[8] = place;
    *ctx->tx_len = copy;
    return 0;
}

/* ---- 0x19: BROADCAST ---- */
static int handle_broadcast(OMI_DispatchContext* ctx) {
    if (!ctx || !ctx->tx_buffer || !ctx->tx_len) return -1;
    size_t copy = sizeof(OMI_512_Envelope) < ctx->tx_capacity
                  ? sizeof(OMI_512_Envelope) : ctx->tx_capacity;
    memcpy(ctx->tx_buffer, ctx->env, copy);
    ctx->tx_buffer[8] = 0xFF;
    *ctx->tx_len = copy;
    return 0;
}

/* ---- 0x1A: SEND ---- */
static int handle_send(OMI_DispatchContext* ctx) {
    if (!ctx || !ctx->tx_buffer || !ctx->tx_len) return -1;
    size_t copy = sizeof(OMI_512_Envelope) < ctx->tx_capacity
                  ? sizeof(OMI_512_Envelope) : ctx->tx_capacity;
    memcpy(ctx->tx_buffer, ctx->env, copy);
    *ctx->tx_len = copy;
    return 0;
}

/* ---- 0x1B: RECV ---- */
static int handle_recv(OMI_DispatchContext* ctx) {
    if (!ctx) return -1;
    if (ctx->vm) {
        uint8_t place = omi_bb_get_place(ctx->bitboard);
        ctx->vm->R[0] = place;
    }
    return 0;
}

/* ---- 0x1C: GAUGE_BIND ---- */
static int handle_gauge_bind(OMI_DispatchContext* ctx) {
    if (!ctx) return -1;
    uint8_t code = ctx->env->target[1];
    if (code > 127) return -2;

    const OmiGaugeEntry* entry = omi_gauge_lookup(code);
    if (!entry) return -3;

    int r = gauge_exec_bind(code, NULL);
    return r;
}

/* ---- 0x1D: GAUGE_INVOKE ---- */
static int handle_gauge_invoke(OMI_DispatchContext* ctx) {
    if (!ctx) return -1;
    uint8_t code = ctx->env->target[1];
    if (code > 127) return -2;

    int r = gauge_exec_lambda(ctx, code);
    if (r != 0) return r;

    if (ctx->vm) {
        ctx->vm->R[0] = code;
    }
    return 0;
}

/* ---- 0x1E: FOLD ---- */
static int handle_fold(OMI_DispatchContext* ctx) {
    if (!ctx) return -1;
    uint64_t folded = omi_bb_fold(ctx->bitboard);
    if (ctx->vm) {
        ctx->vm->R[0] = (uint32_t)(folded & 0xFFFFFFFF);
    }
    return 0;
}

/* ---- 0x1F: VM_ESCAPE ---- */
static int handle_vm_escape(OMI_DispatchContext* ctx) {
    if (!ctx) return -1;
    if (ctx->vm) {
        ctx->vm->FLAGS |= HALT_FLAG;
    }
    return 0;
}

void omi_dispatch_init(void) {
    gauge_exec_init_handlers();

    for (int i = 0; i < OMI_DISPATCH_SLOTS; i++)
        omi_dispatch_table[i] = handle_nop;

    omi_dispatch_table[NOP]    = handle_nop;
    omi_dispatch_table[MOV]    = handle_mov;
    omi_dispatch_table[LOAD]   = handle_load;
    omi_dispatch_table[STORE]  = handle_store;
    omi_dispatch_table[XOR]    = handle_xor;
    omi_dispatch_table[ROTL]   = handle_rotl;
    omi_dispatch_table[ROTR]   = handle_rotr;
    omi_dispatch_table[ADD]    = handle_add;
    omi_dispatch_table[SUB]    = handle_sub;
    omi_dispatch_table[CAR]    = handle_car;
    omi_dispatch_table[CDR]    = handle_cdr;
    omi_dispatch_table[CMP]    = handle_cmp;
    omi_dispatch_table[JMP]    = handle_jmp;
    omi_dispatch_table[JZ]     = handle_jz;
    omi_dispatch_table[DELTA]  = handle_delta;
    omi_dispatch_table[HALT]   = handle_halt;
    omi_dispatch_table[SYSCALL]= handle_syscall;
    omi_dispatch_table[LOADM]  = handle_loadm;
    omi_dispatch_table[CALL]   = handle_call;
    omi_dispatch_table[RET]    = handle_ret;
    omi_dispatch_table[PROBE]       = handle_probe;
    omi_dispatch_table[PROBE_ACK]   = handle_probe_ack;
    omi_dispatch_table[SYNC_COMMIT] = handle_sync_commit;
    omi_dispatch_table[SEAL]        = handle_seal;
    omi_dispatch_table[ROUTE]       = handle_route;
    omi_dispatch_table[BROADCAST]   = handle_broadcast;
    omi_dispatch_table[SEND]        = handle_send;
    omi_dispatch_table[RECV]        = handle_recv;
    omi_dispatch_table[GAUGE_BIND]  = handle_gauge_bind;
    omi_dispatch_table[GAUGE_INVOKE]= handle_gauge_invoke;
    omi_dispatch_table[FOLD]        = handle_fold;
    omi_dispatch_table[VM_ESCAPE]   = handle_vm_escape;
}

void omi_dispatch_set(uint8_t opcode, OMI_OpcodeHandler handler) {
    if (opcode < OMI_DISPATCH_SLOTS)
        omi_dispatch_table[opcode] = handler;
}

int omi_dispatch_execute(OMI_DispatchContext* ctx) {
    if (!ctx || !ctx->env) return -1;
    int opcode = omi_env_get_opcode(ctx->env);
    if (opcode < 0 || opcode >= OMI_DISPATCH_SLOTS) return -2;
    OMI_OpcodeHandler handler = omi_dispatch_table[opcode];
    if (!handler) return -3;
    return handler(ctx);
}
