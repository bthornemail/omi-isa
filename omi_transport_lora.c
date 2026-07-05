#include "omi_transport_lora.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#endif

typedef struct {
    OMI_Transport base;
    int spi_fd;
    OMI_LoraConfig config;
    int initialized;
} OMI_Transport_Lora;

static uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

static int lora_send(OMI_Transport* self, const uint8_t* data, size_t len) {
    (void)self;
    if (!data || len != OMI_ENV_SIZE) return -1;

    uint8_t wire_buf[OMI_ENV_SIZE + 2];
    memcpy(wire_buf, data, len);
    uint16_t crc = crc16(data, len);
    wire_buf[len]     = (uint8_t)(crc >> 8);
    wire_buf[len + 1] = (uint8_t)(crc & 0xFF);

    (void)wire_buf;
    return -1;
}

static int lora_recv(OMI_Transport* self, uint8_t* data, size_t maxlen, int timeout_ms) {
    (void)self;
    (void)timeout_ms;
    if (!data || maxlen < OMI_ENV_SIZE) return -1;

    uint8_t wire_buf[OMI_ENV_SIZE + 2];
    memset(wire_buf, 0, sizeof(wire_buf));

    uint16_t expected = crc16(wire_buf, OMI_ENV_SIZE);
    uint16_t got = (uint16_t)wire_buf[OMI_ENV_SIZE] << 8 | wire_buf[OMI_ENV_SIZE + 1];
    if (expected != got) return -2;

    memcpy(data, wire_buf, OMI_ENV_SIZE);
    return (int)OMI_ENV_SIZE;
}

static int lora_available(OMI_Transport* self) {
    (void)self;
    return 0;
}

static int lora_flush(OMI_Transport* self) {
    (void)self;
    return 0;
}

OMI_Transport* omi_transport_lora_create(const OMI_LoraConfig* cfg) {
    if (!cfg) return NULL;

    OMI_Transport_Lora* t = (OMI_Transport_Lora*)calloc(1, sizeof(OMI_Transport_Lora));
    if (!t) return NULL;

    t->base.send = lora_send;
    t->base.recv = lora_recv;
    t->base.available = lora_available;
    t->base.flush = lora_flush;
    t->base.ctx = NULL;
    t->config = *cfg;
    t->spi_fd = -1;

#ifdef __linux__
    char spidev[32];
    snprintf(spidev, sizeof(spidev), "/dev/spidev%d.%d", 0, 0);
    t->spi_fd = open(spidev, O_RDWR);
    if (t->spi_fd >= 0) {
        uint8_t mode = 0;
        uint8_t bits = 8;
        uint32_t speed = 8000000;
        ioctl(t->spi_fd, SPI_IOC_WR_MODE, &mode);
        ioctl(t->spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        ioctl(t->spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        t->initialized = 1;
    }
#endif

    return &t->base;
}

void omi_transport_lora_destroy(OMI_Transport* t) {
    if (!t) return;
    OMI_Transport_Lora* lt = (OMI_Transport_Lora*)t;
    if (lt->spi_fd >= 0) {
#ifdef __linux__
        close(lt->spi_fd);
#endif
    }
    free(lt);
}
