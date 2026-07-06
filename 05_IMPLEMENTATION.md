# 05. Implementation

This layer answers how the system is built and verified.

## Source Layout

- `lib/`: reusable C runtime, compiler, VM, transport, mesh, orbit, sense, and
  receipt modules.
- `test/`: C regression tests.
- `scripts/`: utility scripts, including bootstrap compiler generation.
- `proof/`: Coq proof stack and extraction outputs.
- `web/`: browser UI, Web Serial, Web Audio, and WASM bridge.
- `firmware/`: ESP32-S3 LoRa firmware.
- `programs/`: sample OMI programs.
- `docs/`: supporting reference documents for the five narrative layers.
- `dev-docs/`: development archive and exploratory notes.

## Build Commands

```sh
make                        # build omi_vm and omi_toolchain
make test                   # run C and web regression tests
make -B proof               # rebuild the Coq proof target
make wasm                   # build web/omi_wasm.js and web/omi_wasm.wasm
make bootstrap              # verify the bootstrap compiler loop
make clean                  # remove generated build artifacts
```

Focused test targets remain available:

```sh
make test_orbit
make test_omi_sense
make test_dispatch
make test_mesh
```

## C And WASM

Native builds compile reusable C sources from `lib/` into `build/`.  The WASM
target compiles the same runtime modules with Emscripten and links them through
`web/omi_web_bridge.c`.

## Firmware

The ESP32-S3 firmware in `firmware/` includes the shared C runtime from
`lib/`, then adds board-specific transport code for USB CDC and SX1262 LoRa.

Firmware builds require a complete ESP-IDF or PlatformIO toolchain.

## Coq

The normal proof target currently builds the compiling proof stack, including
the split pi projection modules and the GL(16,2) semantic stack.  The
`verified_execution.v` layer is kept as the intended execution-refinement
target.

Detailed proof status is tracked in
[docs/mathematics/proof-lineage.md](docs/mathematics/proof-lineage.md).
