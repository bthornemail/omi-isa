#include "gauge_exec.h"
#include <string.h>

static OMI_OpcodeHandler gauge_exec_handlers[GAUGE_EXEC_SLOTS];
static int gauge_exec_bound[GAUGE_EXEC_SLOTS];

void gauge_exec_init_handlers(void) {
    for (int i = 0; i < GAUGE_EXEC_SLOTS; i++) {
        gauge_exec_handlers[i] = NULL;
        gauge_exec_bound[i] = 0;
    }
}

int gauge_exec_bind(uint8_t code, OMI_OpcodeHandler handler) {
    if (code >= GAUGE_EXEC_SLOTS) return -1;
    gauge_exec_handlers[code] = handler;
    gauge_exec_bound[code] = 1;
    return 0;
}

int gauge_exec_unbind(uint8_t code) {
    if (code >= GAUGE_EXEC_SLOTS) return -1;
    gauge_exec_handlers[code] = NULL;
    gauge_exec_bound[code] = 0;
    return 0;
}

int gauge_exec_is_bound(uint8_t code) {
    if (code >= GAUGE_EXEC_SLOTS) return 0;
    return gauge_exec_bound[code];
}

int gauge_exec_set_semantics(uint8_t code, const OmiGaugeEntry* entry) {
    if (code >= GAUGE_EXEC_SLOTS || !entry) return -1;
    omi_gauge_init();

    OmiGaugeEntry* target = (OmiGaugeEntry*)omi_gauge_lookup(code);
    if (!target) return -2;

    memcpy(target, entry, sizeof(OmiGaugeEntry));
    target->code = code;
    return 0;
}

int gauge_exec_lambda(OMI_DispatchContext* ctx, uint8_t code) {
    if (!ctx) return -1;
    if (code >= GAUGE_EXEC_SLOTS) return -2;

    const OmiGaugeEntry* entry = omi_gauge_lookup(code);
    if (!entry || !entry->active) return -3;

    OMI_OpcodeHandler handler = gauge_exec_handlers[code];
    if (handler) {
        return handler(ctx);
    }

    if (entry->is_lambda) {
        return gauge_exec_eval(ctx, code, 0);
    }

    return 0;
}

int gauge_exec_eval(OMI_DispatchContext* ctx, uint8_t code, int depth) {
    if (!ctx) return -1;
    if (depth > GAUGE_EVAL_MAX) return -4;

    const OmiGaugeEntry* entry = omi_gauge_lookup(code);
    if (!entry || !entry->active) return -3;

    OMI_OpcodeHandler handler = gauge_exec_handlers[code];
    if (handler) {
        int r = handler(ctx);
        if (r != 0) return r;
    }

    if (!entry->is_lambda) {
        return 0;
    }

    if (entry->car_addr != 0) {
        uint8_t next_code = (uint8_t)(entry->car_addr & 0x7F);
        int r = gauge_exec_eval(ctx, next_code, depth + 1);
        if (r != 0) return r;
    }

    if (entry->cdr_addr != 0) {
        uint8_t next_code = (uint8_t)(entry->cdr_addr & 0x7F);
        return gauge_exec_eval(ctx, next_code, depth + 1);
    }

    return 0;
}
