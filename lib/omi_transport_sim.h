#ifndef OMI_TRANSPORT_SIM_H
#define OMI_TRANSPORT_SIM_H

#include "omi_transport.h"

#define OMI_SIM_BUF_SIZE 8192

typedef struct {
    uint8_t data[OMI_SIM_BUF_SIZE];
    size_t count;
} OMI_SimBuf;

typedef struct {
    OMI_Transport base;
    OMI_SimBuf rx;
    OMI_SimBuf* tx_peer;
} OMI_Transport_Sim;

void omi_transport_sim_init(OMI_Transport_Sim* t);
void omi_transport_sim_link(OMI_Transport_Sim* a, OMI_Transport_Sim* b);
void omi_transport_sim_unlink(OMI_Transport_Sim* t);

#endif
