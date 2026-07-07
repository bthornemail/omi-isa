# 05. Implementation

## How The System Becomes Artifact

Implementation answers how the protocol is built, tested, and verified.

This is the layer where narrative must become executable.

If the code does not run, the architecture is only a proposal.

If the tests do not pass, the behavior is not stable.

If the proof does not compile, the theorem is not established.

If the firmware cannot be built, the hardware claim remains aspirational.

Implementation is where OMI pays its debts.

## Source Layout

The repository is organized around artifact boundaries:

- `lib/`: reusable C runtime, compiler, VM, transport, mesh, orbit, sense, and
  receipt modules.
- `test/`: C regression tests.
- `scripts/`: utility scripts, including bootstrap compiler generation.
- `web/`: browser UI, Web Serial, Web Audio, and WASM bridge.
- `firmware/`: ESP32-S3 LoRa firmware.
- `programs/`: sample OMI programs.
- `docs/`: supporting reference documents for the five narrative layers.
- `dev-docs/`: development archive and exploratory notes.

The Coq proof stack lives in the sibling `../omi-axioms` repository.

This layout is not cosmetic.

It reflects the story:

```text
shared runtime
  -> tests
  -> delegated proofs
  -> browser surface
  -> firmware surface
  -> archived exploration
```

## Build Commands

The basic build path is:

```sh
make
make test
make -B proof
```

The `proof` target delegates to `../omi-axioms`.

Additional targets:

```sh
make wasm
make bootstrap
make clean
```

Focused test targets:

```sh
make test_orbit
make test_omi_sense
make test_dispatch
make test_mesh
```

These commands are part of the documentation.

They are how a reader moves from manifesto to artifact.

## C Runtime

The reusable C runtime lives in `lib/`.

The root C files are only entrypoints:

- `main.c`
- `toolchain_main.c`

The runtime includes:

- compiler and parser components
- VM and ISA components
- envelope and stream components
- dispatch and gauge components
- transport and mesh components
- orbit and observer components
- sense, receipt, and projective geometry components

The implementation MUST keep reusable runtime logic in `lib/` so that native,
WASM, test, and firmware builds share the same behavioral core.

## Orbit Engine

The orbit engine in `lib/omi_orbit.c` mirrors the mathematical hierarchy:

```text
Layer 1: GL(16,2) linear dynamics
Layer 2: quotient observers
Layer 3: BQF
Layer 4: 5040 atlas
```

This is an important implementation success.

The source file is not only code.

It is a map from mathematical layers to executable layers.

The corresponding test artifact is:

```text
test/test_orbit.c
```

## OMI-Sense

OMI-Sense implements the probe idea.

It accepts sensor-like input only through bounded observer interfaces.

The hardware does not define truth.

The observer boundary defines what can be accepted.

The implementation then converts physical measurement into canonical witness
or receipt surfaces.

This is where the cosmology becomes engineering:

```text
Boundary
  -> Interface
  -> Probe
  -> Sample
  -> Delta
  -> Observer
  -> Receipt
```

## Tests

Tests are executable documentation.

They state what the implementation currently preserves.

The major test artifacts include:

- `test/test_env.c`
- `test/test_dispatch.c`
- `test/test_gauge_exec.c`
- `test/test_radio_vm.c`
- `test/test_mesh.c`
- `test/test_orbit.c`
- `test/test_omi_sense.c`
- `test/test_pg.c`
- `test/test_omicron.c`
- `test/test_omion.c`
- `test/test_receipt.c`

Tests do not replace proofs.

Proofs do not replace tests.

They answer different questions.

Tests ask whether this artifact behaves as expected here.

Proofs ask whether a formal statement follows from formal assumptions.

## Coq

The proof stack lives in the sibling `../omi-axioms` repository.

The local `make proof` target delegates there and builds the proof modules in
dependency order.

The proof stack currently includes:

- atomic kernel lineage
- diagonal closure
- finite incidence
- BQF bridge
- metric projection
- pi projection
- export and compatibility shims
- kernel-to-pi bridge
- GL(16,2) semantic stack

The intended execution-refinement layer is represented by
`../omi-axioms/coq/verified_execution.v`.

Detailed proof status is tracked in:

[docs/mathematics/proof-lineage.md](docs/mathematics/proof-lineage.md).

## WebAssembly And Browser

The WASM build compiles shared C runtime modules from `lib/` and links them
through:

```text
web/omi_web_bridge.c
```

The browser surface is not a separate truth.

It is another probe surface.

It must preserve the same envelope, dispatch, and observer boundaries as the
native runtime.

## Firmware

The firmware in `firmware/` targets ESP32-S3 LoRa execution.

It includes shared C runtime sources from `lib/`.

It adds board-specific transport and radio code.

Firmware builds require a complete ESP-IDF or PlatformIO toolchain.

The firmware is where the project tests a strong form of canonicality:

```text
same observer
same receipt
different physical device
```

## Extraction

Coq extraction files are generated from `../omi-axioms/coq/`.

They are implementation-facing artifacts produced from formal definitions and
are not tracked in `omi-isa`.

Extraction matters because it narrows the distance between theorem and
runtime.

It does not magically prove every C behavior.

It provides a controlled path from formal semantics toward executable
artifacts.

## Implementation Discipline

Implementation MUST preserve artifact boundaries.

Runtime code SHOULD remain in `lib/`.

Tests SHOULD remain in `test/`.

Utility scripts SHOULD remain in `scripts/`.

Web-specific assets SHOULD remain in `web/`.

Firmware-specific assets SHOULD remain in `firmware/`.

Proof modules SHOULD remain in `../omi-axioms/coq/`.

Development transcripts and exploratory material SHOULD remain in `dev-docs/`.

This discipline lets a reader distinguish source of truth from explanation,
experiment, archive, and projection.

## Final Contract

Implementation is the last layer, but it is not secondary.

It is where the project becomes accountable.

The cosmology motivates.

The foundations reduce.

The mathematics proves.

The architecture specifies.

The implementation runs.

When in doubt, run the artifact.

## Definition Of Done

A documentation claim is done when it points to the artifact or proof that
supports it.

A C behavior is done when it builds and has a regression test.

A proof claim is done when `coqc` accepts the theorem in the intended build
order.

A protocol surface is done when its field boundaries are documented and
preserved by implementation.

A hardware probe is done when it satisfies an observer interface rather than
merely producing raw samples.

A bridge claim is done only when it relates two previously separate layers
without weakening either one.

## Implementation As Memory

The implementation is the repository's memory.

Narrative can drift.

Diagrams can seduce.

Terminology can inflate.

The artifact remembers what actually runs.

This is why generated receipts, tests, compiled proofs, and executable
programs matter.

They are not afterthoughts.

They are the mechanism by which the project resists becoming only metaphor.

The implementation is where the doctrine becomes accountable.
