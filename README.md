# OMI-ISA

## The OMI Declaration

Modern computing begins with symbols.

Variables.

Identifiers.

References.

Objects.

Types.

Messages.

Protocols.

OMI explores the opposite direction.

It asks whether computation can begin before symbolic naming, from distinction,
position, relation, and deterministic replay alone.

In this view, positions precede variables.

Relations precede objects.

Pathways precede names.

Containers precede payloads.

Algorithms, not prose, are the primary authority.

Proofs justify those algorithms.

Documentation explains them.

Philosophical interpretation motivates why they were chosen.

None of those interpretations replace the behavior of the executable system
itself.

## The Constitutional Question

Every engineering discipline eventually asks what cannot be removed.

OMI asks that question of computation:

> What structure can be removed while the same canonical replay still results?

That question is the center of the repository.

Everything else is an answer.

The C runtime answers it operationally.

The Coq proofs answer it formally.

The architecture answers it as a protocol.

The firmware answers it as hardware.

The documentation answers it as a narrative path from motivation to rigor.

## What OMI Is

OMI is an experimental deterministic computing architecture built around
place-value relations, finite geometry, formal verification, and distributed
execution.

Concretely, the repository implements a 7-phase distributed semantic execution
stack:

```text
16-bit register VM
  -> 512-bit envelope transport
  -> 32-slot dispatch ISA
  -> gauge lambda engine
  -> LoRa RF transport
  -> Web Serial + WASM bridge
  -> mesh networking
```

The operating path is:

```text
OMI-Lisp (.omi)
  -> lexer
  -> parser
  -> AST
  -> compiler
  -> 16-bit bytecode
  -> boot
  -> CPU
  -> log

512-bit envelope
  -> stream parser
  -> dispatch table
  -> handler execution
  -> gauge lambda eval
  -> probe handshake
  -> transport layer
  -> RF / mesh / browser surface
```

It is not presented as a claim that philosophy proves physics.

It is not presented as a claim that a podcast transcript proves mathematics.

It is not presented as a claim that an analogy is a theorem.

It is presented as a research program with executable artifacts.

The artifacts define the system.

The proofs state what has been mechanically justified.

The documentation marks the boundary between motivation, theorem, protocol,
and implementation.

## The Narrative Arc

The repository is organized as five chapters.

Each chapter answers one question.

1. [Cosmology](01_COSMOLOGY.md)
   answers why OMI begins with observer, boundary, distinction, place, and
   relation.

2. [Foundations](02_FOUNDATIONS.md)
   answers what survives reduction: `NULL`, pair, `CAR`, `CDR`, place value,
   bounded width, rotate/XOR delta, replay, and determinism.

3. [Mathematics](03_MATHEMATICS.md)
   answers what follows once those primitives are fixed: finite incidence,
   projection, orbit dynamics, functorial semantics, coalgebra,
   bialgebra, and theorem-prover boundaries.

4. [Architecture](04_ARCHITECTURE.md)
   answers how the mathematics becomes a protocol: OMI-Lisp, compiler, ISA,
   VM, observers, envelopes, dispatch, transport, mesh, and receipts.

5. [Implementation](05_IMPLEMENTATION.md)
   answers how the protocol is built: C, Coq, tests, scripts, WebAssembly,
   browser surfaces, ESP32/LoRa firmware, and extraction targets.

Read in order, these documents move from theological and philosophical
motivation toward RFC 2119-style protocol discipline and Coq theorem-prover
rigor.

That movement is intentional.

The repository needs both ends.

Without the cosmology, the engineering looks arbitrary.

Without the engineering, the cosmology is only language.

Without the proofs, the bridge between them is too easy to overstate.

## The Boundary Principle

Humans define admissible boundaries and constraints.

Mathematics determines the consequences of those constraints.

Engineering searches for computational or physical probes that realize those
mathematical structures.

The resulting algorithms constitute the authoritative specification.

This principle keeps the project honest.

It permits theological and philosophical motivation.

It permits geometric and symbolic interpretation.

It permits hardware experiments.

But it does not allow any of those to replace the artifact.

The packet is 64 bytes because the implementation makes it so.

The word is 16 bits because the implementation makes it so.

The register file has eight positions because the implementation makes it so.

The proof either compiles or it does not.

The test either passes or it does not.

The receipt either verifies or it does not.

## The Observer Model

OMI separates transition from observation.

A transition changes state.

An observer projects that state onto a readable surface.

This distinction is why visible change does not necessarily create a new
identity.

The base relation can remain canonical while observers expose different
surfaces.

In the project language:

```text
omi---imo
```

is the reciprocal base relation.

```text
o---o/---/?---?@---@
```

is an extended observer surface where entry, path, query, address, and receipt
become readable.

The base relation anchors identity.

The observer surface makes changes legible.

This is why the proof stack keeps returning to observer language: Coq is
forcing the distinction between state transition and projection.

## Executable Architecture Snapshot

The C runtime lives under `lib/`.

The orbit engine is `lib/omi_orbit.c` / `lib/omi_orbit.h`.

It implements a finite linear dynamical system over GF(2^16):

```text
Delta(x, c) = A(x) xor c
```

where `A` is multiplication by a primitive element in GF(2^16), and `c` is a
control parameter held constant across an orbit trace.

The observer functions project orbit state into quotient views:

```text
Fano  = x mod 7
Tetra = x mod 4
Phase = x & 1
```

The point is not that these observers are the whole state.

They are readable surfaces over state.

That same architectural pattern repeats throughout the system.

The VM provides bounded execution:

- 8 general registers.
- 16-bit fixed-width instructions.
- explicit BOOT, KERNEL, and USER modes.
- deterministic stepping.
- append-only log behavior.

The envelope provides bounded transport:

```text
offset  field            size
0       gauge[8]          8 bytes
8       orientation[8]    8 bytes
16      recovery[8]       8 bytes
24      target[8]         8 bytes
32      relation[32]     32 bytes
```

The canonical pre-header is:

```text
FF 00 1C 1D 1E 1F 20 FF
```

The dispatch table extracts:

```text
target[0] & 0x1F
```

and routes it to one of 32 handlers.

The transport layer abstracts send, receive, availability, and flush behavior
so simulated links, LoRa links, browser surfaces, and firmware surfaces can
share the same envelope discipline.

The gauge lambda layer gives the envelope a programmable evaluation surface:

- bind and unbind gauge handlers.
- evaluate lambda-like gauge entries.
- walk `CAR` / `CDR` continuation chains.
- preserve depth limits and non-printing semantics.

The mesh layer routes the same bounded envelope through multi-hop transport.

Receipts record canonical evidence that a bounded observation occurred.

## The Reduction Path

The project did not begin with an instruction set.

It did not begin with GF(2^16).

It did not begin with Coq.

It began with repeated attempts to remove assumptions.

Names were treated as late.

Variables were treated as interpretations.

Objects were treated as projections.

What remained first was boundary.

Then position.

Then pathway.

Then container.

Then deterministic transition.

Only after those structures stabilized were symbols allowed to return as
protocol names, opcodes, theorem identifiers, file paths, and implementation
surfaces.

This is the sense in which OMI is about place before bits.

Bits are the executable medium.

Place is the organizing primitive.

## Repository Map

- `lib/`: reusable C runtime and ISA implementation.
- `test/`: C regression tests.
- `../omi-axioms/coq/`: Coq proof stack in the sibling proof-only repo.
- `web/`: browser and WebAssembly interface.
- `firmware/`: ESP32-S3 LoRa firmware.
- `scripts/`: generated artifact helpers.
- `programs/`: sample OMI programs.
- `docs/`: supporting material for the five root chapters.
- `dev-docs/`: development archive and exploratory notes.

## Quick Start

```sh
make
make test
make -B proof
```

The quick start is deliberately small.

The `proof` target delegates to the sibling `omi-axioms` repository.

OMI is not verified by reading this README.

It is verified by running the artifacts.

Common focused targets:

```sh
make omi_vm
make omi_toolchain
make test_orbit
make test_omi_sense
make test_dispatch
make test_mesh
make wasm
make bootstrap
```

The delegated proof target builds the current compiling Coq stack:

```text
AtomicKernel
  -> AtomicKernelVNext
  -> DiagonalClosure
  -> FiniteIncidence
  -> BQFBridge
  -> MetricProjection
  -> PiProjection
  -> OMI_Exports
  -> omi_pi_proof
  -> omi_pi_bridge
  -> delta_orbit_theory
  -> functorial_semantics
  -> coalgebraic_bisimulation
  -> OMI_bialgebra
  -> verified_execution
```

## Reading Paths

For the philosophical route, read:

```text
README
  -> 01_COSMOLOGY
  -> 02_FOUNDATIONS
  -> 03_MATHEMATICS
```

For the protocol route, read:

```text
README
  -> 04_ARCHITECTURE
  -> docs/architecture
  -> 05_IMPLEMENTATION
```

For the verification route, read:

```text
README
  -> 03_MATHEMATICS
  -> docs/mathematics/proof-lineage.md
  -> ../omi-axioms/
```

For the engineering route, read:

```text
README
  -> 05_IMPLEMENTATION
  -> Makefile
  -> lib/
  -> test/
```

## What This Repository Does Not Claim

It does not claim that theological language is a theorem.

It does not claim that every projected analogy is formally verified.

It does not claim that pi currently emerges from the C GF(2^16) orbit engine.

It does not claim that hardware probes are truth.

It does not claim that names are useless.

It claims something narrower and more testable:

computation can be explored from positional relations and deterministic replay,
and the resulting artifacts can be implemented, tested, and partially proved.

That is the research program.

## Doctrinal Clauses

We hold that deterministic computation need not begin with symbols.

We hold that names are projections of positional relations, not their
foundation.

We hold that place is prior to variable.

We hold that relation is prior to object.

We hold that replay is prior to interpretation.

We hold that canonical form is what remains after arbitrary implementation
choice has been removed.

We hold that observers project state without replacing identity.

We hold that hardware probes approximate mathematical observers.

We hold that algorithms are the primary authority.

We hold that proofs justify algorithms, tests exercise artifacts, and documents
explain the boundary between them.

## From Cosmology To RFC

OMI deliberately spans two modes of language.

At the top of the repository, it speaks as cosmology because it must explain
why the primitive is relation rather than symbol.

In the architecture, it speaks as protocol because compatible implementations
need bounded fields, stable surfaces, and normative requirements.

In the proof stack, it speaks as Coq because claims about determinism,
convergence, and semantic preservation must be mechanically checked.

The movement is:

```text
motivation
  -> boundary
  -> primitive
  -> theorem
  -> protocol
  -> artifact
```

The theological language belongs to motivation.

The RFC-style language belongs to architecture.

The theorem statements belong to Coq.

The executable behavior belongs to the artifacts.

The reader should not collapse these registers.

The repository exists to keep them aligned without pretending they are the same
thing.
