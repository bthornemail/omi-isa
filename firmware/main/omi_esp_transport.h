#ifndef OMI_ESP_TRANSPORT_H
#define OMI_ESP_TRANSPORT_H

#include "omi_transport.h"
#include "omi_sx1262.h"

typedef struct {
    OMI_Transport base;
    SX126X_Device sx1262;
    int  cdc_uart_num;
    int  initialized;
} OMI_EspTransport;

OMI_EspTransport* omi_esp_transport_create(const SX126X_Config* lora_cfg,
                                            uint32_t lora_freq_hz,
                                            int cdc_uart_num);
void omi_esp_transport_destroy(OMI_EspTransport* t);

#endif
