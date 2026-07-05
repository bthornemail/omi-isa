#include "omi_transport_lora.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

static int lora_send(OMI_Transport* self, const uint8_t* data, size_t len) {
    (void)self;
    (void)data;
    (void)len;
    return -1;
}

static int lora_recv(OMI_Transport* self, uint8_t* data, size_t maxlen, int timeout_ms) {
    (void)self;
    (void)data;
    (void)maxlen;
    (void)timeout_ms;
    return -1;
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
