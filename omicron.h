#ifndef OMI_OMICRON_H
#define OMI_OMICRON_H

#include "omienv.h"
#include "omi_dispatch.h"
#include "gauge_exec.h"

#define OMICRON_LAYER_PHYSICAL  8
#define OMICRON_LAYER_ROUTE     9
#define OMICRON_LAYER_DECL      10
#define OMICRON_LAYER_WITNESS   11
#define OMICRON_LAYER_PHASE     12

#define OMICRON_SLIDE_PHASES  4

#define OMICRON_SLIDE_PLACE          0
#define OMICRON_SLIDE_REGISTER_INJECT 1
#define OMICRON_SLIDE_KERNEL_READ    2
#define OMICRON_SLIDE_RECORD_CLOSE   3

#define OMICRON_SLIDE_PLACE_CODE          0x00
#define OMICRON_SLIDE_INJECT_CODE         0x1C
#define OMICRON_SLIDE_KERNEL_CODE         0x1E
#define OMICRON_SLIDE_CLOSE_CODE          0x20

uint32_t omicron_resolve(const OMI_512_Envelope* env, int layer);
int omicron_slide(OMI_DispatchContext* ctx, uint8_t gauge_code);

static inline uint32_t omicron_route(const OMI_512_Envelope* env) {
    return omicron_resolve(env, OMICRON_LAYER_ROUTE);
}
static inline uint32_t omicron_declaration(const OMI_512_Envelope* env) {
    return omicron_resolve(env, OMICRON_LAYER_DECL);
}
static inline uint32_t omicron_witness(const OMI_512_Envelope* env) {
    return omicron_resolve(env, OMICRON_LAYER_WITNESS);
}
static inline uint32_t omicron_phase(const OMI_512_Envelope* env) {
    return omicron_resolve(env, OMICRON_LAYER_PHASE);
}

#endif
