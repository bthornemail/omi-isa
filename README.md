# OMI-ISA

OMI is an experimental deterministic computing architecture that explores
computation through place-value relations, finite geometry, formal
verification, and distributed execution.

The project is organized as a layered story rather than a flat collection of
peer concepts:

1. [Cosmology](01_COSMOLOGY.md): why begin with observer, distinction, place,
   and relation.
2. [Foundations](02_FOUNDATIONS.md): the smallest deterministic computational
   object: mask, rotate, XOR, bounded replay, and delta.
3. [Mathematics](03_MATHEMATICS.md): finite incidence, projection, orbit
   theory, functors, coalgebras, bialgebras, and verification.
4. [Architecture](04_ARCHITECTURE.md): OMI-Lisp, compiler, ISA, VM, envelope,
   dispatch, transport, mesh, and receipts.
5. [Implementation](05_IMPLEMENTATION.md): C, Coq, tests, WASM, browser,
   ESP32/LoRa firmware, and build commands.

The executable artifacts are the authority.  The mathematical proofs justify
those artifacts.  The documentation explains why those artifacts were chosen.

## Quick Start

```sh
make
make test
make -B proof
```

Important directories:

- `lib/`: reusable C runtime and ISA implementation.
- `test/`: C regression tests.
- `proof/`: Coq proof stack.
- `web/`: browser and WASM interface.
- `firmware/`: ESP32-S3 LoRa firmware.
- `docs/`: supporting reference material for the five narrative layers.
- `dev-docs/`: development archive and exploratory notes.

For the shortest technical route, read
[Architecture](04_ARCHITECTURE.md) and then
[Implementation](05_IMPLEMENTATION.md).
