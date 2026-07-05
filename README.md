# OMI-ISA

A 4-stage distributed semantic execution stack: 16-bit reversible-XOR register VM, 512-bit envelope transport, 32-slot dispatch ISA, gauge lambda engine, and LoRa RF transport.

```
OMI-Lisp (.omi) → lexer → parser → AST → compiler → 16-bit bytecode → boot → CPU → log

512-bit envelope → stream parser → dispatch table → handler execution → gauge lambda eval
                                                          ↓
                                                probe handshake ↔ transport layer ↔ RF
```

---

## Architecture

### Delta Law

```
Δ(x) = rotl(x,1) ⊕ rotl(x,3) ⊕ rotr(x,2) ⊕ C
```

XOR preserves information (no bit destruction), rotations spread local influence globally, the constant breaks the zero fixed point. `delta_acc` accumulates all non-R0 XOR results as an invariant trace.

### CPU

- 8 × 32-bit registers (R0–R7)
- 64K × 32-bit memory
- 16-bit fixed-width instructions
- Three privilege modes: BOOT → KERNEL → USER
- SP_FLAG privilege boundary (dot notation is earned at 0x20)
- Deterministic execution with append-only log
- Radio VM state: capability bitmap, peer capability, ISA subset, neighbor table, probe state

### Memory Map

| Range | Region |
|-------|--------|
| 0x0000–0x0FFF | Boot ROM (immutable) |
| 0x1000–0x7FFF | Kernel space |
| 0x8000–0xEFFF | User space |
| 0xF000–0xFFFF | Syscall trap zone |

---

## 512-Bit Envelope

The transport and dispatch unit is a 64-byte envelope:

```
offset  field        size  purpose
------  -----        ----  -------
 0      gauge[8]      8B   gauge codes / pre-header
 8      orientation[8] 8B   capability / sensor pin / mode
16      recovery[8]    8B   frame counter / retry state
24      target[8]      8B   opcode (target[0] & 0x1F) + args
32      relation[32]  32B   OMI-Lisp payload / event data
```

### Canonical Pre-Header

```
FF 00 1C 1D 1E 1F 20 FF
```

Validated by `omi_env_validate()`. Gauge[0] must be >= 0xF0.

### Bitboard

A 64-bit summary register derived from the envelope:

| Bits | Field | Source |
|------|-------|--------|
| 0–6 | gauge code | gauge[0] & 0x7F |
| 7–11 | dplus | orientation diag |
| 12–16 | dminus | orientation diag |
| 17 | bridge_1e flag | gauge[5]=0x1F check |
| 18 | bridge_78 flag | gauge[5]=0x78 check |
| 19 | seal_7f flag | gauge[5]=0x7F check |
| 20 | boot_7c00 flag | orientation check |
| 21 | bridge_aa55 flag | orientation check |
| 22–31 | place | orientation-derived selector |
| 32–47 | result | execution output |
| 48–63 | fold | XOR reduction of envelope bytes |

---

## Gauge Table

128-entry semantic map indexed by gauge code (0–0x7F). Each entry defines:

- `code` — gauge index
- `cls` — CONTROL / PRINTABLE / DEL
- `diag` — diagnostic classification (NONE/PLUS/MINUS/BALANCED)
- `action` — PLACE / REGISTER_INJECT / KERNEL_READ / RECORD_CLOSE / SYSTEM_WITNESS / SEAL / BOOT_PAGE / EXTERNAL_BRIDGE
- `s[8]` — 8 × 16-bit evaluation stack
- `payload`, `mask`, `car`, `cdr`, `bridge` — routing and cascade fields
- `active` — whether entry is enabled
- `is_lambda` — whether entry executes as a lambda
- `car_addr`, `cdr_addr` — car/cdr chain addresses for lambda evaluation
- `eval_depth` — recursion limit

---

## 32-Slot Dispatch ISA

Opcode extracted from `env->target[0] & 0x1F`. Dispatch table maps opcode → handler.

### 0x00–0x13: Core VM Ops (no-op stubs, wired to full handlers when CPU integrated)

| Opcode | Name | Handler |
|--------|------|---------|
| 0x00 | NOP | no-op |
| 0x01 | MOV | move register |
| 0x02 | LOAD | load immediate |
| 0x03 | STORE | store to memory |
| 0x04 | XOR | xor with delta accumulation |
| 0x05 | ROTL | rotate left |
| 0x06 | ROTR | rotate right |
| 0x07 | ADD | add |
| 0x08 | SUB | subtract |
| 0x09 | CAR | low half |
| 0x0A | CDR | high half |
| 0x0B | CMP | compare |
| 0x0C | JMP | jump |
| 0x0D | JZ | jump if zero |
| 0x0E | DELTA | reversible XOR rotation |
| 0x0F | HALT | stop execution |
| 0x10 | SYSCALL | system call |
| 0x11 | LOADM | load from memory |
| 0x12 | CALL | call subroutine |
| 0x13 | RET | return |

### 0x14–0x1F: Radio / Mesh / Gauge Ops

| Opcode | Name | Handler |
|--------|------|---------|
| 0x14 | PROBE | emit PROBE_ACK with node capability |
| 0x15 | PROBE_ACK | store peer capability, compute ISA subset |
| 0x16 | SYNC_COMMIT | lock negotiated ISA subset |
| 0x17 | SEAL | commit envelope to bitboard state |
| 0x18 | ROUTE | forward envelope using place selector |
| 0x19 | BROADCAST | forward to all neighbors |
| 0x1A | SEND | addressed send via orientation |
| 0x1B | RECV | addressed receive, populate register |
| 0x1C | GAUGE_BIND | bind gauge table slot to lambda |
| 0x1D | GAUGE_INVOKE | execute gauge lambda on envelope |
| 0x1E | FOLD | fold bitboard through envelope |
| 0x1F | VM_ESCAPE | escape to host context |

---

## Probe Handshake

Node capability discovery and ISA subset negotiation:

```
Node A                    Node B
  |                         |
  |--- PROBE ------------>|    A sends capability bitmap
  |                         |    B stores A's capability
  |<-- PROBE_ACK ---------|    B responds with its capability
  |                         |    A stores B's capability
  |--- SYNC_COMMIT ------->|    A computes subset = cap(A) & cap(B)
  |                         |    B locks negotiated state
  |<-- SYNC_COMMIT_ACK ---|    Both nodes probe_state = NEGOTIATED
```

Managed by `omi_probe.h/c` — session-based state machine with `omi_probe_handshake()` convenience function.

---

## Streaming Parser

State machine (`stream.h/c`) that reassembles 64-byte envelopes from a byte stream:

```
WAITING → HEADER (match 8-byte pre-header) → PAYLOAD (fill 56 bytes) → COMPLETE
    ↑                                                                       |
    +────────────────────────── pop_event() resets ─────────────────────────+
```

- Auto-dispatch mode: `auto_dispatch=1` triggers `stream_execute()` on completion
- Binary sensor/event/hardware injectors for test
- `StreamEvent` includes opcode, bitboard, dispatch result, and response buffer

---

## Transport Layer

Abstract `OMI_Transport` interface (`omi_transport.h/c`):

| Method | Signature | Purpose |
|--------|-----------|---------|
| send | `(self, data, len)` | blocking send |
| recv | `(self, data, maxlen, timeout_ms)` | blocking/non-blocking recv |
| available | `(self)` | bytes available |
| flush | `(self)` | commit pending |

### Implementations

- **`omi_transport_sim.c`** — linked ring-buffer pair for two-node simulation
- **`omi_transport_lora.c`** — SX1262 SPI driver stub (Linux `/dev/spidev`)

---

## Gauge Lambda Execution

`gauge_exec.h/c` — runtime lambda evaluation engine:

- `gauge_exec_bind(code, handler)` — bind a handler to a gauge slot
- `gauge_exec_unbind(code)` — release binding
- `gauge_exec_lambda(ctx, code)` — evaluate gauge entry as lambda
- `gauge_exec_eval(ctx, code, depth)` — car/cdr chain evaluation (depth-limited to 16)
- `gauge_exec_set_semantics(code, entry)` — non-printing semantics change

### car/cdr Evaluation

```
car = structural descent (head instruction)
cdr = continuation chain (next gauge to evaluate)
```

When a gauge entry has `is_lambda=1`, executing it evaluates the car/cdr chain depth-first.

---

## Sector Iterator

`sector.h/c` — 512-byte sector divided into 8 × 64-byte cells for storage iteration.

---

## Build

```
make                        # builds omi_vm and omi_toolchain
make omi_vm                 # full VM pipeline
make omi_toolchain          # standalone compiler
make test_env               # 39 envelope/stream/bitboard tests
make test_dispatch          # 31 dispatch table tests
make test_gauge_exec        # 21 gauge lambda execution tests
make test_radio_vm          # 43 radio VM end-to-end tests
make run                    # ./omi_vm programs/test.omi
make run-tc                 # toolchain compile only
make clean                  # remove build artifacts
```

All test targets auto-run after compilation. Zero warnings expected.

### Test Summary

| Suite | Tests | What it covers |
|-------|-------|----------------|
| test_env | 39 | Envelope parse/validate, bitboard fold, gauge table, stream parser, binary injectors, sector iterator |
| test_dispatch | 31 | 32-slot dispatch table, PROBE handshake, FOLD, SYNC_COMMIT, custom handlers, stream dispatch integration |
| test_gauge_exec | 21 | Gauge bind/unbind, lambda eval, car/cdr chain, depth limit, non-printing semantics |
| test_radio_vm | 43 | Sim transport, envelope send/recv, probe session, two-node handshake, transport→stream→dispatch pipeline |

---

## Files

```
Phase 0 — Core VM + Envelope
├── isa.h                 opcodes, bit masks, encoding shifts
├── cpu.c / cpu.h         16-bit register VM, step(), run(), mode enforcement
├── boot.c                boot sequence (seed, delta init, kernel entry)
├── asm.c                 instruction encoder
├── ast.c / ast.h         AST node constructors
├── compiler.c            OMI-Lisp AST → 16-bit bytecode
├── lexer.c / lexer.h     OMI-Lisp tokenizer
├── parser.c              recursive-descent parser
├── loader.c / loader.h   binary loader
├── main.c                pipeline entry point
├── toolchain_main.c      standalone compiler entry
├── omienv.c / omienv.h   512-bit envelope, bitboard, gauge table
├── stream.c / stream.h   streaming parser, auto-dispatch
├── sector.c / sector.h   512-byte sector iterator

Phase 1 — Dispatch + Transport
├── omi_dispatch.c / .h   32-slot dispatch table, handlers
├── omi_transport.c / .h  abstract transport interface

Phase 2 — Gauge Lambda
├── gauge_exec.c / .h     gauge lambda evaluation engine

Phase 3 — Radio VM
├── omi_transport_sim.c / .h  simulated paired transport
├── omi_transport_lora.c / .h SX1262 LoRa driver stub
├── omi_probe.c / .h      probe handshake state machine

Tests
├── test_env.c            39 envelope/stream/sector tests
├── test_dispatch.c       31 dispatch table tests
├── test_gauge_exec.c     21 gauge lambda tests
├── test_radio_vm.c       43 radio VM end-to-end tests

Programs
├── programs/test.omi     (omi . imo)
├── programs/kernel.omi   (boot . (seed . delta))
└── programs/init.omi     (omi--imo . o---o/---/?---?@---@)
```

---

## OMI-Lisp Grammar

```peg
OMI    <- SP? Expr
Expr   <- Pair / Atom
Pair   <- '(' Expr '.' Expr ')'
Atom   <- [a-zA-Z_@/?-][a-zA-Z0-9_@/?-]*
```

SP (0x20) marks the boundary between pre-language control stream and readable OMI-Lisp. Parsing begins after the first SP.

---

## Syscalls

| ID | Function |
|----|----------|
| 0 | SYSLOG — log R0 and delta to receipt |
| 1 | reset delta_acc to 0 |
| 2 | enter USER mode |
| 3 | enter KERNEL mode (SP boundary) |
