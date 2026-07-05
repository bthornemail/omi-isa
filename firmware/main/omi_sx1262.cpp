#include "omi_sx1262.h"
#include <esp_log.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

static const char* TAG = "sx1262";

static spi_device_handle_t spi_handle;

static void wait_on_busy(SX126X_Device* dev) {
    int timeout = 1000;
    while (gpio_get_level((gpio_num_t)dev->cfg.busy_pin) && timeout--) {
        ets_delay_us(1);
    }
    if (timeout <= 0) ESP_LOGW(TAG, "busy timeout");
}

static void spi_write_cmd(SX126X_Device* dev, uint8_t cmd) {
    wait_on_busy(dev);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    spi_device_transmit(spi_handle, &t);
}

static void spi_write_cmd_data(SX126X_Device* dev, uint8_t cmd,
                                const uint8_t* data, int len) {
    wait_on_busy(dev);
    uint8_t buf[256];
    buf[0] = cmd;
    if (len > 0) memcpy(buf + 1, data, len);
    spi_transaction_t t = {
        .length = (8 + len * 8),
        .tx_buffer = buf,
    };
    spi_device_transmit(spi_handle, &t);
}

static void spi_read_cmd_data(SX126X_Device* dev, uint8_t cmd,
                               uint8_t* data, int len) {
    wait_on_busy(dev);
    uint8_t txbuf[4] = {cmd, 0, 0, 0};
    spi_transaction_t t = {
        .length = 32,
        .tx_buffer = txbuf,
        .rx_buffer = txbuf,
    };
    spi_device_transmit(spi_handle, &t);
    if (len > 0 && data) data[0] = txbuf[2];
}

static void spi_write_reg(SX126X_Device* dev, uint16_t addr, uint8_t val) {
    uint8_t data[3] = {(uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF), val};
    spi_write_cmd_data(dev, SX126X_CMD_WRITE_REGISTER, data, 3);
}

static uint8_t spi_read_reg(SX126X_Device* dev, uint16_t addr) {
    uint8_t cmd[3] = {SX126X_CMD_READ_REGISTER, (uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF)};
    wait_on_busy(dev);
    uint8_t buf[5];
    memcpy(buf, cmd, 3);
    buf[3] = 0; buf[4] = 0;
    spi_transaction_t t = {
        .length = 40,
        .tx_buffer = buf,
        .rx_buffer = buf,
    };
    spi_device_transmit(spi_handle, &t);
    return buf[4];
}

static void sx126x_set_dio_irq(SX126X_Device* dev, uint16_t irq_mask) {
    uint8_t data[8] = {
        0x00,                                 // IRQ mask
        (uint8_t)(irq_mask >> 8),
        (uint8_t)(irq_mask & 0xFF),
        0x00, (uint8_t)(irq_mask >> 8),       // DIO1
        (uint8_t)(irq_mask & 0xFF),
        0x00, 0x00,                           // DIO2, DIO3
    };
    spi_write_cmd_data(dev, SX126X_CMD_SET_DIO_IRQ_PARAMS, data, 8);
}

SX126X_Error sx126x_reset(SX126X_Device* dev) {
    gpio_set_level((gpio_num_t)dev->cfg.reset_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level((gpio_num_t)dev->cfg.reset_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    return SX126X_OK;
}

SX126X_Error sx126x_init(SX126X_Device* dev, const SX126X_Config* cfg) {
    if (!dev || !cfg) return SX126X_ERR_SPI;
    memcpy(&dev->cfg, cfg, sizeof(SX126X_Config));
    dev->initialized = 0;
    dev->packet_len = 0;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << cfg->cs_pin) | (1ULL << cfg->busy_pin) |
                        (1ULL << cfg->reset_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << cfg->busy_pin) | (1ULL << cfg->dio1_pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_set_level((gpio_num_t)cfg->cs_pin, 1);

    if (cfg->tx_enable_pin >= 0) {
        gpio_set_level((gpio_num_t)cfg->tx_enable_pin, 0);
        gpio_reset_pin((gpio_num_t)cfg->tx_enable_pin);
        gpio_set_direction((gpio_num_t)cfg->tx_enable_pin, GPIO_MODE_OUTPUT);
    }
    if (cfg->rx_enable_pin >= 0) {
        gpio_set_level((gpio_num_t)cfg->rx_enable_pin, 0);
        gpio_reset_pin((gpio_num_t)cfg->rx_enable_pin);
        gpio_set_direction((gpio_num_t)cfg->rx_enable_pin, GPIO_MODE_OUTPUT);
    }

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = 11,
        .miso_io_num = 13,
        .sclk_io_num = 12,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 256,
    };
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 8000000,
        .mode = 0,
        .spics_io_num = cfg->cs_pin,
        .queue_size = 4,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &spi_handle));

    sx126x_reset(dev);
    vTaskDelay(pdMS_TO_TICKS(15));

    sx126x_standby(dev);
    vTaskDelay(pdMS_TO_TICKS(1));

    spi_write_cmd_data(dev, SX126X_CMD_SET_PACKET_TYPE,
                       (uint8_t[]){SX126X_PACKET_TYPE_LORA}, 1);

    spi_write_reg(dev, SX126X_REG_LORA_SYNC_WORD, 0x34);
    ESP_LOGI(TAG, "SX1262 initialized");
    dev->initialized = 1;
    return SX126X_OK;
}

SX126X_Error sx126x_standby(SX126X_Device* dev) {
    spi_write_cmd_data(dev, SX126X_CMD_SET_STANDBY,
                       (uint8_t[]){SX126X_STANDBY_RC}, 1);
    return SX126X_OK;
}

SX126X_Error sx126x_set_frequency(SX126X_Device* dev, uint32_t freq_hz) {
    uint32_t rf_freq = (uint32_t)((uint64_t)freq_hz * (1 << 25) / SX126X_CRYSTAL_FREQ);
    uint8_t data[4] = {
        (uint8_t)(rf_freq >> 16),
        (uint8_t)(rf_freq >> 8),
        (uint8_t)(rf_freq),
        0,
    };
    spi_write_cmd_data(dev, SX126X_CMD_SET_RF_FREQUENCY, data, 4);
    ESP_LOGI(TAG, "frequency set to %u Hz", freq_hz);
    return SX126X_OK;
}

SX126X_Error sx126x_set_tx_power(SX126X_Device* dev, int8_t power_dbm) {
    if (power_dbm > 22) power_dbm = 22;
    if (power_dbm < -9) power_dbm = -9;
    uint8_t params[2] = {
        (uint8_t)(power_dbm + 9),  // OCP = dBm + 9
        0x00,                      // PA ramp time
    };
    spi_write_cmd_data(dev, SX126X_CMD_SET_TX_PARAMS, params, 2);
    return SX126X_OK;
}

SX126X_Error sx126x_set_lora_modem(SX126X_Device* dev, uint8_t sf,
                                    uint32_t bw_hz, uint8_t cr) {
    uint8_t bw;
    if (bw_hz >= 500000)       bw = 0x08;
    else if (bw_hz >= 250000)  bw = 0x06;
    else if (bw_hz >= 125000)  bw = 0x05;
    else if (bw_hz >= 62500)   bw = 0x04;
    else if (bw_hz >= 31250)   bw = 0x03;
    else if (bw_hz >= 15625)   bw = 0x02;
    else if (bw_hz >= 10417)   bw = 0x01;
    else                        bw = 0x00;

    uint8_t mod_params[4] = {
        sf,                       // SpreadingFactor
        bw,                       // Bandwidth
        cr,                       // CodingRate
        0x01,                     // LowDataRateOptimize (auto)
    };
    spi_write_cmd_data(dev, SX126X_CMD_SET_MODULATION_PARAMS, mod_params, 4);
    ESP_LOGI(TAG, "LoRa modem: SF=%d BW=%u CR=4/%d", sf, bw_hz, cr);
    return SX126X_OK;
}

SX126X_Error sx126x_tx(SX126X_Device* dev, const uint8_t* data, uint8_t len) {
    if (!dev->initialized) return SX126X_ERR_SPI;
    if (len > 64) len = 64;
    dev->packet_len = len;

    uint8_t pkt_params[6] = {
        0x00,                     // PreambleLength MSB
        0x0C,                     // PreambleLength LSB (12)
        len,                      // PayloadLength
        0x01,                     // CRC (enabled)
        0x00,                     // InvertIQ (standard)
        0x00,                     // HeaderType (variable)
    };
    spi_write_cmd_data(dev, SX126X_CMD_SET_PACKET_PARAMS, pkt_params, 6);

    sx126x_set_dio_irq(dev, SX126X_IRQ_TX_DONE);

    uint8_t buf[1 + len];
    buf[0] = 0x00;
    memcpy(buf + 1, data, len);
    wait_on_busy(dev);
    spi_write_cmd_data(dev, SX126X_CMD_WRITE_BUFFER, buf, 1 + len);

    if (dev->cfg.tx_enable_pin >= 0)
        gpio_set_level((gpio_num_t)dev->cfg.tx_enable_pin, 1);

    wait_on_busy(dev);
    spi_write_cmd(dev, SX126X_CMD_TX);

    uint16_t irq = 0;
    int timeout = 5000;
    while (!(irq & SX126X_IRQ_TX_DONE) && timeout--) {
        sx126x_get_irq(dev, &irq);
        ets_delay_us(100);
    }

    if (dev->cfg.tx_enable_pin >= 0)
        gpio_set_level((gpio_num_t)dev->cfg.tx_enable_pin, 0);

    if (!timeout) {
        ESP_LOGW(TAG, "TX timeout");
        return SX126X_ERR_TIMEOUT;
    }

    uint8_t clear_cmd[3] = {SX126X_CMD_CLEAR_IRQ_STATUS, 0xFF, 0xFF};
    spi_write_cmd_data(dev, SX126X_CMD_CLEAR_IRQ_STATUS, clear_cmd + 1, 2);
    ESP_LOGI(TAG, "TX %d bytes", len);
    return SX126X_OK;
}

SX126X_Error sx126x_rx(SX126X_Device* dev, uint8_t* data,
                        uint8_t* len, uint32_t timeout_ms) {
    if (!dev->initialized || !data || !len) return SX126X_ERR_SPI;
    *len = 0;

    uint8_t pkt_params[6] = {
        0x00, 0x0C, 0xFF, 0x01, 0x00, 0x00
    };
    spi_write_cmd_data(dev, SX126X_CMD_SET_PACKET_PARAMS, pkt_params, 6);

    sx126x_set_dio_irq(dev, SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT);

    if (dev->cfg.rx_enable_pin >= 0)
        gpio_set_level((gpio_num_t)dev->cfg.rx_enable_pin, 1);

    wait_on_busy(dev);
    uint8_t rx_cmd[3] = {SX126X_CMD_RX, 0x00, 0x00,};
    spi_write_cmd_data(dev, SX126X_CMD_RX, rx_cmd + 1, 2);

    uint16_t irq = 0;
    int timeout = (timeout_ms > 0) ? (timeout_ms * 10) : 10000;
    while (!(irq & (SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT)) && timeout--) {
        sx126x_get_irq(dev, &irq);
        ets_delay_us(100);
    }

    if (dev->cfg.rx_enable_pin >= 0)
        gpio_set_level((gpio_num_t)dev->cfg.rx_enable_pin, 0);

    if (irq & SX126X_IRQ_TIMEOUT || !timeout) {
        uint8_t clear[2] = {0xFF, 0xFF};
        spi_write_cmd_data(dev, SX126X_CMD_CLEAR_IRQ_STATUS, clear, 2);
        return SX126X_ERR_TIMEOUT;
    }

    uint8_t rxlen = spi_read_reg(dev, 0x01AC); // RXRXBYTECNT
    if (rxlen > 64) rxlen = 64;

    wait_on_busy(dev);
    uint8_t buf[2] = {SX126X_CMD_READ_BUFFER, 0x00};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = buf,
        .rx_buffer = buf,
    };
    uint8_t rx_data[128];
    t.length = (2 + rxlen) * 8;
    t.tx_buffer = buf;
    t.rx_buffer = rx_data;
    wait_on_busy(dev);
    spi_device_transmit(spi_handle, &t);

    memcpy(data, rx_data + 2, rxlen);
    *len = rxlen;

    uint8_t clear[2] = {0xFF, 0xFF};
    spi_write_cmd_data(dev, SX126X_CMD_CLEAR_IRQ_STATUS, clear, 2);
    ESP_LOGI(TAG, "RX %d bytes", rxlen);
    return SX126X_OK;
}

SX126X_Error sx126x_available(SX126X_Device* dev) {
    if (!dev || !dev->initialized) return 0;
    uint16_t irq = 0;
    sx126x_get_irq(dev, &irq);
    return (irq & SX126X_IRQ_RX_DONE) ? 1 : 0;
}

SX126X_Error sx126x_get_irq(SX126X_Device* dev, uint16_t* irq) {
    if (!irq) return SX126X_ERR_SPI;
    wait_on_busy(dev);
    uint8_t buf[4] = {SX126X_CMD_GET_IRQ_STATUS, 0, 0, 0};
    spi_transaction_t t = {
        .length = 32,
        .tx_buffer = buf,
        .rx_buffer = buf,
    };
    spi_device_transmit(spi_handle, &t);
    *irq = ((uint16_t)buf[2] << 8) | buf[3];
    return SX126X_OK;
}

SX126X_Error sx1262_set_cad_params(SX126X_Device* dev, uint8_t cad_sym_num, uint8_t cad_exit_mode, uint32_t cad_timeout_ms) {
    uint8_t params[7] = {
        cad_sym_num,
        cad_exit_mode,
        (uint8_t)(cad_timeout_ms >> 16),
        (uint8_t)(cad_timeout_ms >> 8),
        (uint8_t)(cad_timeout_ms),
        0x00,
        0x00
    };
    spi_write_cmd_data(dev, SX126X_CMD_SET_CAD_PARAMS, params, 7);
    sx126x_set_dio_irq(dev, SX126X_IRQ_CAD_DONE | SX126X_IRQ_CAD_DETECTED);
    return SX126X_OK;
}

SX126X_Error sx1262_cad(SX126X_Device* dev, int* channel_busy) {
    if (!dev || !channel_busy) return SX126X_ERR_SPI;
    *channel_busy = 0;

    wait_on_busy(dev);
    spi_write_cmd(dev, SX126X_CMD_SET_CAD);

    uint16_t irq = 0;
    int timeout = 5000;
    while (!(irq & SX126X_IRQ_CAD_DONE) && timeout--) {
        sx126x_get_irq(dev, &irq);
        ets_delay_us(100);
    }

    if (!timeout) {
        ESP_LOGW(TAG, "CAD timeout");
        uint8_t clear[2] = {0xFF, 0xFF};
        spi_write_cmd_data(dev, SX126X_CMD_CLEAR_IRQ_STATUS, clear, 2);
        return SX126X_ERR_TIMEOUT;
    }

    *channel_busy = (irq & SX126X_IRQ_CAD_DETECTED) ? 1 : 0;

    uint8_t clear[2] = {0xFF, 0xFF};
    spi_write_cmd_data(dev, SX126X_CMD_CLEAR_IRQ_STATUS, clear, 2);
    return SX126X_OK;
}
