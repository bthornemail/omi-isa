# 04. Architecture

This layer answers how the mathematics becomes a computer.

```text
OMI-Lisp
  -> Compiler
  -> ISA
  -> VM
  -> 512-bit Envelope
  -> Dispatch
  -> Transport
  -> Mesh
  -> Receipts
```

## Language And Compiler

OMI-Lisp programs are parsed into AST nodes and compiled into 16-bit
instructions.  The root entrypoints are `main.c` for the VM pipeline and
`toolchain_main.c` for standalone compilation.  The reusable compiler,
parser, lexer, AST, loader, and assembler live in `lib/`.

## ISA And VM

The VM uses fixed-width 16-bit instructions, eight general registers, explicit
privilege modes, and deterministic execution with an append-only log.  The ISA
is defined in `lib/isa.h` and executed by `lib/cpu.c`.

Supporting reference: [docs/architecture/isa-spec.md](docs/architecture/isa-spec.md).

## Envelope And Dispatch

The fundamental transport unit is a 512-bit envelope:

- `gauge[8]`: canonical pre-header and gauge codes.
- `orientation[8]`: packet type, capability, sensor, and mode fields.
- `recovery[8]`: frame counter and retry state.
- `target[8]`: opcode and arguments.
- `relation[32]`: payload, event data, or OMI-Lisp relation text.

Dispatch uses `target[0] & 0x1F` to select one of 32 handlers.

Supporting reference: [docs/architecture/envelope-spec.md](docs/architecture/envelope-spec.md).

## Transport, Mesh, And Receipts

Transport is abstracted through `OMI_Transport`, with simulated and LoRa
implementations.  Mesh routing builds on envelopes and transport.  Receipts
record canonical execution evidence over the computed state.

Supporting reference: [docs/architecture/mesh-protocol.md](docs/architecture/mesh-protocol.md).
