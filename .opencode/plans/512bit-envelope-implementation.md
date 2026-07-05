# Plan: 512-Bit Envelope + Bitboard + Streaming + Sector Reference Implementation

## Goal
Extend omi-isa with a complete carrier/bitboard layer supporting 512-bit envelopes, uniform bitboard witness, streaming parser for GPIO/sensors, and 512-byte sector iteration.

## Files to Create

### 1. `omienv.h` — Header
- `OMI_512_Envelope` struct (gauge[8], orientation[8], recovery[8], target[8], relation[32])
- 64-bit bitboard constants/macros (D+/D- counts, flags, place, result, fold)
- `OmiGaugeEntry` struct + 128-entry gauge table API
- Hex helper declarations: `hex_to_nibble`, `nibble_to_hex`

### 2. `omienv.c` — Implementation
- `omi_env_parse()` — memcpy 64 bytes into struct
- `omi_env_validate()` — gauge match, FS/GS/RS/US cascade, SP boundary
- `omi_env_to_bitboard()` — projects envelope rails into 64-bit witness
- `omi_env_to_relation()` — formats relation frame as `o-S0-S1-S2-S3/S4/S5-S6-S7?payload@car@cdr`
- Bitboard apply: Polybius gauge classification, D+/D- accumulation, place set, XOR accumulation
- Bitboard fold: XOR high/low 32 bits into fold field
- Gauge table: 128-entry with action mappings (0x00-0x1F = control/place, 0x20-0x2F = register inject, 0x30-0x3F = kernel read, 0x1E/0x78/0x7F = special)

### 3. `stream.h` — Streaming Parser Header
- `StreamState` enum: WAITING, HEADER, PAYLOAD, COMPLETE, INVALID
- `StreamParser` struct: 64-byte buffer, state, filled count, gauge_match_count, envelope, bitboard, validation_result
- `StreamEvent` struct: parsed envelope + bitboard + validity
- `stream_parser_init()`, `stream_push_byte()`, `stream_has_event()`, `stream_pop_event()`
- Sensor injection: `stream_inject_sensor()`, `stream_inject_event()`, `stream_inject_hardware()`
  - **Now binary envelopes**: builds full 64-byte binary envelope with canonical pre-header
  - Sensor data packed into orientation[0..1] and relation[0..3] (value as big-endian uint32)

### 4. `stream.c` — Streaming Parser Implementation
- State machine: WAITING→HEADER→PAYLOAD→COMPLETE, resets on mismatch
- `build_binary_envelope()` helper: creates canonical envelope with metadata in orientation + relation
- Each `stream_inject_*` builds a 64-byte envelope and pushes all bytes through the stream parser

### 5. `sector.h` — Sector Iterator Header
- `OMI_SECTOR_SIZE` = 512, `OMI_CELLS_PER_SECTOR` = 8
- `SectorIterator` struct: data pointer, cell_index, 8 parsed cells + validity flags + bitboards
- `sector_iter_init()`, `sector_iter_next()`, `sector_iter_count()`

### 6. `sector.c` — Sector Iterator Implementation
- Parses all 8 × 64-byte cells at init time
- `sector_iter_next()` skips invalid cells, returns next valid one
- Returns -2 when no more cells

## Files to Modify

### 7. `Makefile`
- Add `ENV_OBJ=omienv.o stream.o sector.o`
- Add to `VM_OBJ` and `TC_OBJ`
- Add dependency rules for new `.o` files
- Add `test_env` target: `gcc -o test_env test_env.c omienv.c stream.c sector.c`

## Test File

### 8. `test_env.c` — Validation Tests
- Build canonical envelope → validate → should PASS
- Build invalid envelope (wrong gauge) → validate → should FAIL
- Build envelope → convert to bitboard → verify gauge, D+/D-, flags, place, result
- Fold bitboard → verify fold field
- Stream parser: push valid 64-byte envelope → verify event received and valid
- Stream parser: push garbage → verify no event
- Stream parser: test reset on partial match
- Sensor injection: inject binary envelope → verify parser accepts it
- Sector iterator: build 512-byte sector with 8 cells → verify all 8 parsed, iterate all
- Sector iterator: sector with only first 4 cells valid → verify count = 4

## Key Design Decisions
1. `nibble_to_hex`/`hex_to_nibble` declared in `omienv.h` so usable by `env_loader.c` if needed
2. Sensor injection uses **binary envelopes** (64 bytes) not text — each injector builds a full canonical envelope
3. CANONICAL_PREHEADER defined as `static const` in each `.c` file that needs it (simple, no header export needed)
4. C99 compatible (`-std=c99`), no VLAs, no inline functions in .c files

## Build & Verify
```sh
make clean && make
gcc -Wall -Wextra -std=c99 -g -o test_env test_env.c omienv.c stream.c sector.c
./test_env
```
