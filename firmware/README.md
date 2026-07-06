# OMI-ISA ESP32-S3 Walkie-Talkie Firmware

## Overview

ESP32-S3 firmware that implements the OMI-ISA distributed semantic execution stack over LoRa (SX1262). Bridges USB CDC ACM ↔ LoRa radio, forwarding 64-byte envelopes bidirectionally.

## Pin Mapping (Default)

| Pin | Function |
|-----|----------|
| 10  | SX1262 NSS (CS) |
| 11  | SX1262 MOSI |
| 12  | SX1262 SCK |
| 13  | SX1262 MISO |
| 9   | SX1262 BUSY |
| 8   | SX1262 RESET |
| 7   | SX1262 DIO1 (IRQ) |
| 6   | SX1262 TX_EN (RF switch) |
| 5   | SX1262 RX_EN (RF switch) |

## Build

### PlatformIO (recommended)

```
pio run -e esp32-s3-devkitc-1
pio run -e esp32-s3-devkitc-1 -t upload
pio device monitor
```

### ESP-IDF

```
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

## Architecture

```
USB CDC ACM ──→ UART ──→ StreamParser ──→ auto-dispatch ──→ LoRa TX
                                                                    |
LoRa RX ──→ OMI_Transport ──→ OMI_512_Envelope ──→ UART ──→ USB CDC ACM
```

- USB serial bytes are streamed into `stream_push_byte()` with auto-dispatch
- Completed envelopes are forwarded over LoRa via `omi_transport_send_envelope()`
- Received LoRa envelopes are forwarded to USB serial as raw 64-byte frames
- Probe handshake occurs on peer discovery (capability = 0xFFFFFFFF)

## OMI Component Files

The build includes the core OMI C stack from `lib/` via relative paths in CMakeLists.txt:
- `lib/omienv.c`, `lib/stream.c`, `lib/sector.c`
- `lib/omi_dispatch.c`, `lib/omi_transport.c`
- `lib/gauge_exec.c`, `lib/cpu.c`, `lib/boot.c`
