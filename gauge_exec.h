#ifndef GAUGE_EXEC_H
#define GAUGE_EXEC_H

#include "omienv.h"
#include "omi_dispatch.h"

#define GAUGE_EXEC_SLOTS  128
#define GAUGE_EVAL_MAX    16

void gauge_exec_init_handlers(void);
int gauge_exec_bind(uint8_t code, OMI_OpcodeHandler handler);
int gauge_exec_unbind(uint8_t code);
int gauge_exec_lambda(OMI_DispatchContext* ctx, uint8_t code);
int gauge_exec_eval(OMI_DispatchContext* ctx, uint8_t code, int depth);
int gauge_exec_set_semantics(uint8_t code, const OmiGaugeEntry* entry);
int gauge_exec_is_bound(uint8_t code);

#endif
