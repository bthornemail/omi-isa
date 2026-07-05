#ifndef OMI_SX1262_H
#define OMI_SX1262_H

#include <stdint.h>

#define SX126X_CRYSTAL_FREQ 32000000UL

#define SX126X_CMD_SET_SLEEP               0x84
#define SX126X_CMD_SET_STANDBY             0x80
#define SX126X_CMD_SET_PACKET_TYPE         0x8A
#define SX126X_CMD_SET_RF_FREQUENCY        0x86
#define SX126X_CMD_SET_TX_PARAMS           0x8E
#define SX126X_CMD_SET_BUFFER_BASE_ADDRESS 0x8F
#define SX126X_CMD_SET_MODULATION_PARAMS  0x8B
#define SX126X_CMD_SET_PACKET_PARAMS       0x8C
#define SX126X_CMD_SET_DIO_IRQ_PARAMS      0x08
#define SX126X_CMD_SET_RX_TX_FALLBACK_MODE 0x93
#define SX126X_CMD_TX                      0x83
#define SX126X_CMD_RX                      0x82
#define SX126X_CMD_GET_STATUS              0xC0
#define SX126X_CMD_READ_REGISTER           0x1D
#define SX126X_CMD_WRITE_REGISTER          0x0D
#define SX126X_CMD_READ_BUFFER             0x1E
#define SX126X_CMD_WRITE_BUFFER            0x0E
#define SX126X_CMD_CLEAR_IRQ_STATUS        0x02
#define SX126X_CMD_GET_IRQ_STATUS          0x12
#define SX126X_CMD_SET_CAD                 0xC5
#define SX126X_CMD_SET_CAD_PARAMS          0x88

#define SX126X_CAD_SYMBOLS_1               0x00
#define SX126X_CAD_SYMBOLS_2               0x01
#define SX126X_CAD_SYMBOLS_4               0x02
#define SX126X_CAD_SYMBOLS_8               0x03
#define SX126X_CAD_SYMBOLS_16              0x04
#define SX126X_CAD_EXIT_CONTINUOUS         0x00
#define SX126X_CAD_EXIT_STANDBY            0x01

#define SX126X_REG_LORA_SYNC_WORD          0x0740
#define SX126X_REG_LORA_RX_GAIN            0x08AC

#define SX126X_IRQ_TX_DONE                 0x01
#define SX126X_IRQ_RX_DONE                 0x02
#define SX126X_IRQ_TIMEOUT                 0x04
#define SX126X_IRQ_CAD_DONE                (1 << 6)
#define SX126X_IRQ_CAD_DETECTED            (1 << 7)

#define SX126X_PACKET_TYPE_LORA            0x01

#define SX126X_STANDBY_RC                  0x00

#define SX126X_FALLBACK_STDBY_RC           0x20

#define SX126X_RX_TIMEOUT_NONE             0x000000
#define SX126X_TX_TIMEOUT_NONE             0x000000

typedef enum {
    SX126X_OK = 0,
    SX126X_ERR_SPI,
    SX126X_ERR_BUSY,
    SX126X_ERR_TIMEOUT,
    SX126X_ERR_CRC,
} SX126X_Error;

typedef struct {
    int   spi_host;
    int   cs_pin;
    int   busy_pin;
    int   reset_pin;
    int   dio1_pin;
    int   tx_enable_pin;
    int   rx_enable_pin;
} SX126X_Config;

typedef struct {
    SX126X_Config cfg;
    int initialized;
    int packet_len;
} SX126X_Device;

SX126X_Error sx126x_init(SX126X_Device* dev, const SX126X_Config* cfg);
SX126X_Error sx126x_reset(SX126X_Device* dev);
SX126X_Error sx126x_set_frequency(SX126X_Device* dev, uint32_t freq_hz);
SX126X_Error sx126x_set_tx_power(SX126X_Device* dev, int8_t power_dbm);
SX126X_Error sx126x_set_lora_modem(SX126X_Device* dev, uint8_t sf, uint32_t bw_hz, uint8_t cr);
SX126X_Error sx126x_standby(SX126X_Device* dev);
SX126X_Error sx126x_tx(SX126X_Device* dev, const uint8_t* data, uint8_t len);
SX126X_Error sx126x_rx(SX126X_Device* dev, uint8_t* data, uint8_t* len, uint32_t timeout_ms);
SX126X_Error sx126x_available(SX126X_Device* dev);
SX126X_Error sx126x_get_irq(SX126X_Device* dev, uint16_t* irq);
SX126X_Error sx1262_set_cad_params(SX126X_Device* dev, uint8_t cad_sym_num, uint8_t cad_exit_mode, uint32_t cad_timeout_ms);
SX126X_Error sx1262_cad(SX126X_Device* dev, int* channel_busy);

#endif
