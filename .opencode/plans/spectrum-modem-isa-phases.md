# Phased Plan: Spectrum Modem ISA — Radio VM ABI + Mesh OS + LoRa Transport

## Overview

Extend omi-isa from a 512-bit envelope parser + fixed ISA into a full **distributed spectrum modem instruction architecture** across three phases:

| Phase | Name | Delivers |
|-------|------|----------|
| 1 | Firmware Refactor | 32-slot ISA dispatch + Transport abstraction + envelope→execute pipeline |
| 2 | OMI-Lisp Spec | Gauge lambda execution, car/cdr chaining, non-printing semantics |
| 3 | LoRa Modem | SX1262 transport driver + probe sync handshake + real RF timing |

---

## Phase 1: Firmware Refactor — Envelope Dispatch + Transport Layer

### 1.1 Extend `isa.h` — 32-slot ISA

Current: 20 opcodes (0x00–0x13).  
Target: 32 opcodes (0x00–0x1F).

**Add 12 new opcodes for mesh/radio/gauge operations:**

```c
enum {
    // 0x00–0x13: existing 20 ops (unchanged)
    NOP=0, MOV, LOAD, STORE, XOR, ROTL, ROTR, ADD, SUB, CAR, CDR,
    CMP, JMP, JZ, DELTA, HALT, SYSCALL, LOADM, CALL, RET,

    // 0x14–0x1F: 12 new radio/mesh/gauge ops
    PROBE       = 20,  // capability discovery
    PROBE_ACK   = 21,  // capability response
    SYNC_COMMIT = 22,  // commit negotiated ISA subset
    SEAL        = 23,  // commit envelope to execution
    ROUTE       = 24,  // forward envelope to neighbor
    BROADCAST   = 25,  // forward to all neighbors
    SEND        = 26,  // addressed send
    RECV        = 27,  // addressed receive
    GAUGE_BIND  = 28,  // bind gauge entry to lambda
    GAUGE_INVOKE= 29,  // invoke gauge lambda
    FOLD        = 30,  // bitboard fold
    VM_ESCAPE   = 31   // escape to host context
};
```

No masking constants change — OP_MASK stays `0xF800`, all 32 opcodes fit in 5 bits.

### 1.2 New module: `omi_dispatch.h` / `omi_dispatch.c`

**Dispatch table of 32 function pointers:**

```c
#ifndef OMI_DISPATCH_H
#define OMI_DISPATCH_H

#include "omienv.h"
#include "stream.h"
#include "cpu.h"

// Dispatch context: what the handler sees
typedef struct {
    OMI_512_Envelope* env;
    uint64_t bitboard;
    OMI_CPU* vm;              // existing VM instance
    uint8_t* tx_buffer;       // output buffer for response envelopes
    size_t* tx_len;
    void* transport_ctx;      // opaque transport context
} OMI_DispatchContext;

// Handler signature: returns 0 on success, <0 on error
typedef int (*OMI_OpcodeHandler)(OMI_DispatchContext* ctx);

// Dispatch table (32 slots)
extern OMI_OpcodeHandler omi_dispatch_table[32];

// Initialize table — wire all 32 handlers
void omi_dispatch_init(void);

// Set a custom handler for a slot
void omi_dispatch_set(uint8_t opcode, OMI_OpcodeHandler handler);

// Invoke dispatch
int omi_dispatch_execute(OMI_DispatchContext* ctx);

#endif
```

**Handler stubs for all 32 ops:**

| Opcode | Handler | Behavior |
|--------|---------|----------|
| NOP | `handle_nop` | no-op |
| MOV–RET | `handle_vm_op` | decode 16-bit instr from envelope → call `cpu_step()` |
| PROBE | `handle_probe` | emit PROBE_ACK with node capability bitmap |
| PROBE_ACK | `handle_probe_ack` | store peer capability, update ISA subset |
| SYNC_COMMIT | `handle_sync_commit` | lock negotiated ISA subset |
| SEAL | `handle_seal` | commit envelope VM state |
| ROUTE | `handle_route` | forward envelope using bitboard PLACE field |
| BROADCAST | `handle_broadcast` | forward to all neighbors |
| SEND | `handle_send` | addressed send via orientation field |
| RECV | `handle_recv` | addressed recv, populate envelope |
| GAUGE_BIND | `handle_gauge_bind` | bind gauge table slot to lambda |
| GAUGE_INVOKE | `handle_gauge_invoke` | execute gauge lambda on envelope |
| FOLD | `handle_fold` | fold bitboard through envelope |
| VM_ESCAPE | `handle_vm_escape` | escape to host (callback) |

### 1.3 Extend `stream.h` / `stream.c` — add `stream_execute()`

Add to `StreamParser`:

```c
// In StreamEvent:
typedef struct {
    uint64_t bitboard;
    OMI_512_Envelope envelope;
    int valid;
    int cell_index;
    int opcode;              // extracted from envelope->gauge[0]
    int dispatch_result;     // result of executing the handler
    uint8_t response[OMI_ENV_SIZE];  // optional response envelope
    size_t response_len;
} StreamEvent;
```

Add function:

```c
// Execute the dispatched handler for a completed envelope.
// Returns 0 on success, <0 on error.
int stream_execute(StreamParser* sp, OMI_CPU* vm);
```

Wire in `stream_push_byte` — when state reaches COMPLETE, optionally auto-dispatch:

```c
if (sp->state == STREAM_STATE_COMPLETE && sp->auto_dispatch) {
    sp->dispatch_result = stream_execute(sp, sp->vm);
}
```

### 1.4 New module: `omi_transport.h` / `omi_transport.c`

**Abstract transport interface:**

```c
#ifndef OMI_TRANSPORT_H
#define OMI_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>

typedef struct OMI_Transport OMI_Transport;

struct OMI_Transport {
    // Send bytes (blocking)
    int (*send)(OMI_Transport* self, const uint8_t* data, size_t len);
    // Receive bytes (blocking, with timeout ms; 0 = non-blocking)
    int (*recv)(OMI_Transport* self, uint8_t* data, size_t maxlen, int timeout_ms);
    // Number of bytes available (non-blocking)
    int (*available)(OMI_Transport* self);
    // Flush / commit
    int (*flush)(OMI_Transport* self);
    // Opaque backend state
    void* ctx;
};

// Fill an envelope-sized buffer from transport, returns number of bytes read
int omi_transport_recv_envelope(OMI_Transport* t, uint8_t* buf, size_t bufsize);

// Send an envelope
int omi_transport_send_envelope(OMI_Transport* t, const OMI_512_Envelope* env);

#endif
```

Backend implementations:
- `omi_transport_null.c` — null/no-op (for testing)
- `omi_transport_file.c` — file read/write (for simulation)
- `omi_transport_lora.c` — SX1262 (Phase 3)

### 1.5 Extend `cpu.h` — Radio VM state

Add to `OMI_CPU`:

```c
typedef struct {
    // existing fields...
    
    // Radio VM state (Phase 1)
    uint32_t capability;           // this node's capability bitmap
    uint32_t peer_capability;      // last negotiated peer capability
    uint32_t isa_subset;           // agreed ISA subset
    uint8_t neighbor_count;
    uint8_t neighbors[16];         // simple neighbor table
    uint8_t probe_state;           // 0=idle, 1=probing, 2=negotiated
} OMI_CPU;
```

### 1.6 Files to create/modify — Phase 1

| File | Action | Purpose |
|------|--------|---------|
| `isa.h` | **Modify** | Add 12 new opcodes (PROBE through VM_ESCAPE) |
| `omi_dispatch.h` | **Create** | Dispatch table, context struct, 32 handler slots |
| `omi_dispatch.c` | **Create** | Default handlers + table init |
| `stream.h` | **Modify** | Add `stream_execute()` + `auto_dispatch` flag + `StreamEvent` response field |
| `stream.c` | **Modify** | Implement `stream_execute()`, wire auto-dispatch |
| `omi_transport.h` | **Create** | Abstract `OMI_Transport` struct + envelope helpers |
| `omi_transport.c` | **Create** | `omi_transport_recv_envelope()`, `omi_transport_send_envelope()` |
| `cpu.h` | **Modify** | Add radio VM state fields |
| `Makefile` | **Modify** | Add `omi_dispatch.o`, `omi_transport.o` |
| `test_dispatch.c` | **Create** | Test dispatch table wiring + handler invocation |

### 1.7 Verification — Phase 1

```
make clean && make
gcc -o test_dispatch test_dispatch.c omienv.o stream.o omi_dispatch.o cpu.o
./test_dispatch
```

Expected: All 32 dispatch slots wired, handlers execute without crash, PROBE emits acknowledgment envelope.

---

## Phase 2: OMI-Lisp Spec — Gauge Lambda Execution

### 2.1 Extend `OmiGaugeEntry` — lambda fields

```c
typedef struct {
    // existing fields...
    
    // Gauge lambda execution (Phase 2)
    OMI_OpcodeHandler exec;          // function pointer for this gauge entry
    uint32_t car_addr;               // car: head instruction (literal or env offset)
    uint32_t cdr_addr;               // cdr: continuation (next gauge index or env offset)
    uint8_t eval_depth;              // evaluation recursion limit
    uint8_t is_lambda;               // 1 = this entry is a lambda, not a static action
} OmiGaugeEntry;
```

### 2.2 New module: `gauge_exec.h` / `gauge_exec.c`

**Gauge lambda evaluation engine:**

```c
#ifndef GAUGE_EXEC_H
#define GAUGE_EXEC_H

#include "omienv.h"
#include "omi_dispatch.h"

// Evaluate a gauge entry as a lambda over an envelope
// Returns 0 on success, <0 on error
int gauge_exec_lambda(const OmiGaugeEntry* entry, OMI_DispatchContext* ctx);

// Bind a gauge table slot to a custom handler
int gauge_exec_bind(uint8_t code, OMI_OpcodeHandler handler);

// Evaluate car/cdr chain:
//   car = head instruction
//   cdr = continuation (next gauge to evaluate)
//   payload = immediate data
int gauge_exec_eval(uint8_t code, OMI_DispatchContext* ctx);

// Non-printing semantics: change gauge behavior without changing packet format
// This is the key idea — the gauge table IS the program
void gauge_exec_set_semantics(uint8_t code, const OmiGaugeEntry* new_entry);

#endif
```

### 2.3 Evaluation rule

```
(gauge opcode) → lambda execution over envelope fields

car        = head instruction (what to execute now)
cdr        = continuation   (what to execute next)
payload    = immediate data
s[8]       = evaluation stack
```

### 2.4 Non-printing semantics

The gauge table can be modified at runtime. Behavior changes without modifying the 64-byte envelope format. This is the key invention:

```
Before: gauge[0x1E] = { action: RECORD_CLOSE, ... }
After:  gauge[0x1E] = { is_lambda: 1, exec: custom_handler, ... }

Same envelope bytes → different execution behavior
```

### 2.5 Files to create/modify — Phase 2

| File | Action | Purpose |
|------|--------|---------|
| `omienv.h` | **Modify** | Add lambda fields to `OmiGaugeEntry` (car_addr, cdr_addr, exec, is_lambda) |
| `omienv.c` | **Modify** | Initialize new fields in gauge table build |
| `gauge_exec.h` | **Create** | Gauge lambda evaluation API |
| `gauge_exec.c` | **Create** | car/cdr chain evaluation, non-printing rebinding |
| `omi_dispatch.c` | **Modify** | `handle_gauge_bind`/`handle_gauge_invoke` call gauge_exec |
| `Makefile` | **Modify** | Add `gauge_exec.o` |
| `test_gauge_exec.c` | **Create** | Test lambda binding, car/cdr chain, non-printing rebind |

### 2.6 Verification — Phase 2

```
gcc -o test_gauge_exec test_gauge_exec.c omienv.o stream.o omi_dispatch.o gauge_exec.o cpu.o
./test_gauge_exec
```

Expected: Gauge entries executable as lambdas, car/cdr chains evaluate in sequence, rebinding changes behavior without envelope format change.

---

## Phase 3: LoRa Modem — SX1262 Transport

### 3.1 New module: `omi_transport_lora.h` / `omi_transport_lora.c`

**SX1262 driver implementation of `OMI_Transport`:**

```c
#ifndef OMI_TRANSPORT_LORA_H
#define OMI_TRANSPORT_LORA_H

#include "omi_transport.h"

// SX1262 configuration
typedef struct {
    int spi_cs;
    int spi_mosi;
    int spi_miso;
    int spi_sck;
    int irq_pin;
    int busy_pin;
    int reset_pin;
    uint32_t frequency_hz;     // e.g. 868000000
    int spreading_factor;      // e.g. 7 (SF7)
    int bandwidth_hz;          // e.g. 125000
    int coding_rate;           // e.g. 5 (4/5)
    int tx_power_dbm;          // e.g. 14
} OMI_LoraConfig;

// Create an SX1262 transport instance
// Returns NULL on failure
OMI_Transport* omi_transport_lora_create(const OMI_LoraConfig* cfg);

// Destroy the instance
void omi_transport_lora_destroy(OMI_Transport* t);

#endif
```

### 3.2 Probe sync handshake

```
Node A                    Node B
  |                         |
  |--- PROBE envelope ---->|
  |                         |--- verify gauge, extract capability
  |<-- PROBE_ACK envelope --|
  |                         |--- compute ISA subset = intersect(A, B)
  |--- SYNC_COMMIT ------->|
  |                         |--- lock subset, begin execution
  |<-- SYNC_COMMIT_ACK ----|
  |                         |
  |=== active VM loop ====>|
```

Implementation in `omi_dispatch.c` handlers:
- `handle_probe()` — sends `PROBE_ACK` with node capability in relation field
- `handle_probe_ack()` — stores peer capability, emits `SYNC_COMMIT`
- `handle_sync_commit()` — locks ISA subset, transitions to active mode

### 3.3 Real RF timing

Each 64-byte envelope at common LoRa settings:

| SF | BW | CR | Preamble | Time per 64B | Range |
|----|-----|-----|----------|-------------|-------|
| 7 | 125kHz | 4/5 | 8 | ~150ms | ~2km |
| 9 | 125kHz | 4/5 | 8 | ~400ms | ~5km |
| 12 | 125kHz | 4/5 | 8 | ~1400ms | ~10km+ |

Transport layer handles fragmentation: larger payloads split across multiple `omi_transport_send/recv` calls.

### 3.4 Files to create — Phase 3

| File | Action | Purpose |
|------|--------|---------|
| `omi_transport_lora.h` | **Create** | SX1262 transport config and factory |
| `omi_transport_lora.c` | **Create** | SX1262 SPI driver, send/recv/available implementation |
| `omi_transport_sim.c` | **Create** | Simulated transport for testing (writes to file) |
| `omi_probe.h` | **Create** | Probe sync handshake state machine |
| `omi_probe.c` | **Create** | Probe/ProbeAck/SyncCommit logic |
| `Makefile` | **Modify** | Add `omi_transport_lora.o`, `omi_transport_sim.o`, `omi_probe.o` |
| `test_radio_vm.c` | **Create** | End-to-end: transport → stream → dispatch → execute → response |

### 3.5 Verification — Phase 3

```
# Simulation mode (no RF hardware needed)
gcc -o test_radio_vm test_radio_vm.c \
    omienv.o stream.o omi_dispatch.o omi_transport.o omi_transport_sim.o omi_probe.o cpu.o
./test_radio_vm

# Expected: two simulated nodes exchange PROBE/PROBE_ACK/SYNC_COMMIT,
# negotiate ISA subset, then execute a test program via envelopes
```

---

## Full Build Integration

### Makefile additions (all phases)

```makefile
DISP_OBJ=omi_dispatch.o
TRANS_OBJ=omi_transport.o
GAUGE_EXEC_OBJ=gauge_exec.o
LORA_OBJ=omi_transport_lora.o omi_transport_sim.o omi_probe.o

VM_OBJ += $(DISP_OBJ) $(TRANS_OBJ) $(GAUGE_EXEC_OBJ)
TC_OBJ += $(DISP_OBJ) $(TRANS_OBJ) $(GAUGE_EXEC_OBJ)

omi_dispatch.o: omi_dispatch.c omi_dispatch.h omienv.h cpu.h isa.h
omi_transport.o: omi_transport.c omi_transport.h
gauge_exec.o: gauge_exec.c gauge_exec.h omienv.h omi_dispatch.h
omi_probe.o: omi_probe.c omi_probe.h omi_transport.h omi_dispatch.h
omi_transport_lora.o: omi_transport_lora.c omi_transport_lora.h omi_transport.h
omi_transport_sim.o: omi_transport_sim.c omi_transport.h

test_dispatch: test_dispatch.c $(DISP_OBJ) $(TRANS_OBJ) $(GAUGE_EXEC_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
	./$@

test_radio_vm: test_radio_vm.c $(DISP_OBJ) $(TRANS_OBJ) $(GAUGE_EXEC_OBJ) $(LORA_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
	./$@
```

---

## Summary: File Inventory

| Phase | New Files | Modified Files |
|-------|-----------|----------------|
| **P1** | `omi_dispatch.h/c`, `omi_transport.h/c`, `test_dispatch.c` | `isa.h`, `stream.h/c`, `cpu.h`, `Makefile` |
| **P2** | `gauge_exec.h/c`, `test_gauge_exec.c` | `omienv.h/c`, `omi_dispatch.c`, `Makefile` |
| **P3** | `omi_transport_lora.h/c`, `omi_transport_sim.c`, `omi_probe.h/c`, `test_radio_vm.c` | `Makefile` |

Total: ~14 new files, ~7 modified files, ~0 deleted files.
