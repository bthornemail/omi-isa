#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_efuse.h>
#include <nvs_flash.h>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" {
#include "omienv.h"
#include "stream.h"
#include "omi_dispatch.h"
#include "cpu.h"
#include "gauge_exec.h"
#include "omi_transport.h"
#include "omi_mesh.h"
}

#include "omi_sx1262.h"
#include "omi_esp_transport.h"

static const char* TAG = "omi_esp32";

#define CDC_UART_NUM  UART_NUM_0
#define CDC_BUF_SIZE  1024

static StreamParser stream_parser;
static OMI_CPU cpu;
static OMI_EspTransport* lora_transport;
static OMI_Transport* mesh_transport;

static void init_omi_stack(void) {
    omi_gauge_init();
    init_cpu(&cpu);
    cpu.capability = 0xFFFFFFFF;
    stream_parser_init(&stream_parser);
    stream_parser.auto_dispatch = 1;
    stream_parser.vm = &cpu;
    omi_dispatch_init();
    ESP_LOGI(TAG, "OMI stack initialized");
}

static void init_serial(void) {
    uart_config_t uart_cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(CDC_UART_NUM, &uart_cfg);
    uart_set_pin(CDC_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(CDC_UART_NUM, CDC_BUF_SIZE, 0, 0, NULL, 0);
    ESP_LOGI(TAG, "CDC UART initialized");
}

static uint64_t esp_time_ms(void) {
    return esp_timer_get_time() / 1000;
}

static void process_serial_to_mesh(void) {
    uint8_t buf[64];
    int len = uart_read_bytes(CDC_UART_NUM, buf, sizeof(buf), 0);
    if (len <= 0) return;

    for (int i = 0; i < len; i++) {
        stream_push_byte(&stream_parser, buf[i]);
        if (stream_parser.state == STREAM_STATE_COMPLETE) {
            OMI_512_Envelope* env = &stream_parser.envelope;
            if (mesh_transport) {
                mesh_transport->send(mesh_transport, (const uint8_t*)env, OMI_ENV_SIZE);
            }
            StreamEvent evt;
            stream_pop_event(&stream_parser, &evt);
        }
    }
}

static void process_mesh_to_serial(void) {
    if (!mesh_transport) return;

    OMI_512_Envelope env;
    int ret = mesh_transport->recv(mesh_transport, (uint8_t*)&env, OMI_ENV_SIZE, 0);
    if (ret != OMI_ENV_SIZE) return;

    uart_write_bytes(CDC_UART_NUM, (const char*)&env, OMI_ENV_SIZE);
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "OMI-ISA ESP32-S3 Walkie-Talkie starting");

    esp_err_t nvs = nvs_flash_init();
    if (nvs == ESP_ERR_NVS_NO_FREE_PAGES || nvs == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    init_omi_stack();
    init_serial();

    SX126X_Config lora_cfg = {
        .spi_host      = SPI2_HOST,
        .cs_pin        = 10,
        .busy_pin      = 9,
        .reset_pin     = 8,
        .dio1_pin      = 7,
        .tx_enable_pin = 6,
        .rx_enable_pin = 5,
    };

    lora_transport = omi_esp_transport_create(&lora_cfg, 868000000, CDC_UART_NUM);
    if (!lora_transport) {
        ESP_LOGE(TAG, "Failed to create LoRa transport");
        return;
    }
    ESP_LOGI(TAG, "LoRa transport ready at 868 MHz");

    uint8_t mac[6] = {0};
    esp_efuse_mac_get_default(mac);
    uint8_t node_id = mac[5];
    mesh_transport = omi_mesh_create(&lora_transport->base, node_id, esp_time_ms);
    if (!mesh_transport) {
        ESP_LOGE(TAG, "Failed to create mesh transport");
        return;
    }
    ESP_LOGI(TAG, "Mesh transport ready, node_id=%d", node_id);

    while (1) {
        process_serial_to_mesh();
        process_mesh_to_serial();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
