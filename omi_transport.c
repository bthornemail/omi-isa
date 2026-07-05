#include "omi_transport.h"
#include <string.h>

int omi_transport_send_envelope(OMI_Transport* t, const OMI_512_Envelope* env) {
    if (!t || !env || !t->send) return -1;
    return t->send(t, (const uint8_t*)env, sizeof(OMI_512_Envelope));
}

int omi_transport_recv_envelope(OMI_Transport* t, OMI_512_Envelope* env) {
    if (!t || !env) return -1;
    if (!t->recv) return -2;
    int n = t->recv(t, (uint8_t*)env, sizeof(OMI_512_Envelope), 0);
    if (n < (int)sizeof(OMI_512_Envelope)) return -3;
    return 0;
}
