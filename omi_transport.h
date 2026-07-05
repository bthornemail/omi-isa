#ifndef OMI_TRANSPORT_H
#define OMI_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>
#include "omienv.h"

typedef struct OMI_Transport OMI_Transport;

struct OMI_Transport {
    int (*send)(OMI_Transport* self, const uint8_t* data, size_t len);
    int (*recv)(OMI_Transport* self, uint8_t* data, size_t maxlen, int timeout_ms);
    int (*available)(OMI_Transport* self);
    int (*flush)(OMI_Transport* self);
    void* ctx;
};

int omi_transport_send_envelope(OMI_Transport* t, const OMI_512_Envelope* env);
int omi_transport_recv_envelope(OMI_Transport* t, OMI_512_Envelope* env);

#endif
