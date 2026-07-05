#include "omi_esp_transport.h"
#include <esp_log.h>
#include <string.h>
#include <stdlib.h>

static const char* TAG = "omi_esp_transport";

static int esp_send(OMI_Transport* self, const uint8_t* data, size_t len) {
    OMI_EspTransport* t = (OMI_EspTransport*)self;
    if (!t || !t->initialized) return -1;
    size_t remain = len;
    while (remain > 0) {
        uint8_t chunk_len = (remain > 64) ? 64 : (uint8_t)remain;
        SX126X_Error err = sx126x_tx(&t->sx1262, data + (len - remain), chunk_len);
        if (err != SX126X_OK) {
            ESP_LOGE(TAG, "TX failed: %d", err);
            return -1;
        }
        remain -= chunk_len;
    }
    return (int)len;
}

static int esp_recv(OMI_Transport* self, uint8_t* data, size_t maxlen, int timeout_ms) {
    OMI_EspTransport* t = (OMI_EspTransport*)self;
    if (!t || !t->initialized || !data || maxlen == 0) return -1;

    uint8_t rx_len = 0;
    SX126X_Error err = sx126x_rx(&t->sx1262, data, &rx_len, timeout_ms);
    if (err == SX126X_ERR_TIMEOUT) return 0;
    if (err != SX126X_OK) return -1;

    return (rx_len <= maxlen) ? rx_len : (int)maxlen;
}

static int esp_available(OMI_Transport* self) {
    OMI_EspTransport* t = (OMI_EspTransport*)self;
    if (!t || !t->initialized) return 0;
    SX126X_Error avail = sx126x_available(&t->sx1262);
    return (avail == SX126X_OK) ? 1 : 0;
}

static int esp_flush(OMI_Transport* self) {
    (void)self;
    return 0;
}

OMI_EspTransport* omi_esp_transport_create(const SX126X_Config* lora_cfg,
                                            uint32_t lora_freq_hz,
                                            int cdc_uart_num) {
    if (!lora_cfg) return NULL;
    OMI_EspTransport* t = (OMI_EspTransport*)calloc(1, sizeof(OMI_EspTransport));
    if (!t) return NULL;

    t->base.send      = esp_send;
    t->base.recv      = esp_recv;
    t->base.available = esp_available;
    t->base.flush     = esp_flush;
    t->base.ctx       = NULL;
    t->cdc_uart_num   = cdc_uart_num;

    SX126X_Error err = sx126x_init(&t->sx1262, lora_cfg);
    if (err != SX126X_OK) {
        ESP_LOGE(TAG, "SX1262 init failed: %d", err);
        free(t);
        return NULL;
    }

    sx126x_standby(&t->sx1262);
    sx126x_set_frequency(&t->sx1262, lora_freq_hz);
    sx126x_set_tx_power(&t->sx1262, 14);
    sx126x_set_lora_modem(&t->sx1262, 7, 125000, 5);
    sx126x_standby(&t->sx1262);

    t->initialized = 1;
    ESP_LOGI(TAG, "ESP transport initialized at %u Hz", lora_freq_hz);
    return t;
}

void omi_esp_transport_destroy(OMI_EspTransport* t) {
    if (!t) return;
    if (t->sx1262.initialized) {
        sx126x_standby(&t->sx1262);
    }
    free(t);
}
