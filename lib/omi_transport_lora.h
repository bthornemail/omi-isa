#ifndef OMI_TRANSPORT_LORA_H
#define OMI_TRANSPORT_LORA_H

#include "omi_transport.h"

typedef struct {
    int spi_cs;
    int spi_mosi;
    int spi_miso;
    int spi_sck;
    int irq_pin;
    int busy_pin;
    int reset_pin;
    uint32_t frequency_hz;
    int spreading_factor;
    int bandwidth_hz;
    int coding_rate;
    int tx_power_dbm;
} OMI_LoraConfig;

OMI_Transport* omi_transport_lora_create(const OMI_LoraConfig* cfg);
void omi_transport_lora_destroy(OMI_Transport* t);

#endif
