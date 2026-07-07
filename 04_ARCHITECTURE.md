# 04. Architecture

## How Mathematics Becomes Protocol

Architecture answers how mathematical structure becomes a computing system.

This chapter is where narrative begins to harden into protocol.

Cosmology can speak in observers and worlds.

Foundations can speak in boundaries and replay.

Mathematics can speak in incidence, projection, functors, and coalgebras.

Architecture must say what the system does.

It must define surfaces.

It must define boundaries.

It must define fields.

It must define the path from source text to executable receipt.

## Pipeline

The architectural pipeline is:

```text
OMI-Lisp
  -> Compiler
  -> ISA
  -> VM
  -> Observers
  -> 512-bit Envelope
  -> Dispatch
  -> Transport
  -> Mesh
  -> Receipts
```

The important insertion is Observers.

Observers sit between VM state and protocol surface.

They convert computation into readable, routable, attestable structure.

## Protocol Language

Architecture is the layer where RFC-style language becomes appropriate.

The words `MUST`, `SHOULD`, and `MAY` are used in their ordinary protocol
sense:

- `MUST` marks a requirement for compatible artifacts.
- `SHOULD` marks a strong expectation with possible implementation-specific
  exceptions.
- `MAY` marks an allowed extension or implementation choice.

This language does not belong in the cosmology.

It belongs here, where a reader needs to build or validate a compatible
system.

## OMI-Lisp

OMI-Lisp is the symbolic surface.

It is not the foundation.

It appears after place, relation, and replay.

Programs are parsed into AST nodes and compiled into fixed-width instructions.

The language gives humans a handle on a positional architecture.

The implementation entrypoints are:

- `main.c`
- `toolchain_main.c`

The reusable compiler components live in `lib/`.

## Compiler

The compiler translates symbolic input into 16-bit instruction words.

It MUST preserve the instruction encoding expected by the VM.

It SHOULD keep carrier stripping and loader behavior explicit.

It MAY support higher-level syntax, but only if that syntax lowers to the same
canonical instruction behavior.

The compiler is therefore not the authority.

The emitted instruction stream is.

## ISA

The ISA defines the executable instruction boundary.

The opcode table lives in `lib/isa.h`.

The VM executes the instruction behavior in `lib/cpu.c`.

An OMI-compatible ISA implementation MUST preserve opcode meaning.

It MAY change host language or platform.

It MUST NOT change canonical replay while claiming to implement the same ISA.

Supporting reference: [docs/architecture/isa-spec.md](docs/architecture/isa-spec.md).

## VM

The VM provides:

- fixed-width instruction execution
- eight general registers
- explicit privilege modes
- deterministic stepping
- append-only logging
- integration with dispatch and envelope surfaces

The VM is where symbolic code becomes state transition.

But the VM is not where observation ends.

The VM produces state.

Observers project state.

## Observers

Observers convert computation into protocol-visible structure.

They turn VM state, orbit state, or sensor state into quotient views.

Examples include:

- phase
- Fano class
- tetrahedral class
- bitboard fields
- envelope offsets
- receipt slots
- OMI-Sense witnesses

An observer MUST be treated as a projection, not as the full state.

This distinction protects identity.

The base relation:

```text
omi---imo
```

can remain canonical while an observer surface:

```text
o---o/---/?---?@---@
```

changes what is readable.

## OMI-Sense As Probe Architecture

OMI-Sense is not merely a sensor framework.

It is a collection of hardware probes that satisfy predefined observational
interfaces.

The interface comes first.

The hardware comes second.

Examples:

```text
Temperature -> ADC -> 16-bit word -> Delta -> Observer -> Receipt
Light       -> Photodiode -> Voltage -> Sample -> Delta -> Witness
```

The ADC is not truth.

The photodiode is not truth.

They are probes that realize an observer boundary.

## 512-Bit Envelope

The envelope is the fundamental protocol unit.

It is 64 bytes.

It has fixed fields:

- `gauge[8]`
- `orientation[8]`
- `recovery[8]`
- `target[8]`
- `relation[32]`

An implementation MUST preserve this field layout to claim envelope
compatibility.

The envelope is not merely a packet.

It is a bounded observer surface.

It gives state a transportable form without pretending that the transport is
the original state.

Supporting reference:
[docs/architecture/envelope-spec.md](docs/architecture/envelope-spec.md).

## Dispatch

Dispatch uses:

```text
target[0] & 0x1F
```

to select one of 32 handlers.

Dispatch is where envelope structure becomes action.

Handlers MUST preserve envelope validity.

Handlers SHOULD produce receipts or response surfaces when state-changing
operations need to be observed.

Handlers MAY be extended, but extension must not silently rewrite canonical
opcode behavior.

## Transport

Transport is abstracted through `OMI_Transport`.

Different physical or simulated links can satisfy the same interface.

That is the canonical principle in engineering form:

```text
different radio
  -> same observer
  -> same receipt
```

Transport MAY be simulated.

Transport MAY use LoRa.

Transport MAY use browser surfaces.

Transport MUST preserve envelope byte order and envelope boundaries.

## Mesh

Mesh routing builds on transport.

It does not replace the envelope.

It routes bounded surfaces through a network of peers.

Mesh behavior SHOULD preserve route updates, data forwarding, store-and-forward
queue behavior, retry behavior, and stale expiry behavior as tested by the
mesh regression suite.

Supporting reference:
[docs/architecture/mesh-protocol.md](docs/architecture/mesh-protocol.md).

## Receipts

Receipts are canonical records of observation.

They are where execution becomes accountable.

A receipt is not a story about what happened.

It is an artifact recording what the system observed under a defined boundary.

Receipts matter because OMI treats algorithms as authority.

The receipt is where the algorithm leaves evidence.

## Architecture Boundary

Architecture must not overclaim proof.

It can define protocol requirements.

It can define compatibility.

It can define observer surfaces.

It can define packet shape.

It cannot claim that an interpretation is formally verified unless the proof
stack says so.

That is why Architecture points back to Mathematics.

And that is why Implementation must run tests.

## Normative Summary

An OMI-compatible artifact MUST preserve the envelope boundary it claims to
implement.

It MUST preserve opcode behavior for the ISA surface it claims to implement.

It MUST treat observers as projections rather than as replacement identities.

It MUST preserve byte order and field meaning across transport surfaces.

It SHOULD emit or preserve receipts where execution becomes accountable.

It SHOULD keep hardware probes behind observer interfaces.

It SHOULD distinguish symbolic convenience from canonical structure.

It MAY add syntax, transport, visualization, or hardware support.

It MUST NOT use those additions to alter canonical replay while claiming
compatibility.

## Architecture As Translation

Architecture translates between worlds.

It translates cosmology into fields.

It translates foundations into opcodes.

It translates mathematics into observers.

It translates observers into envelopes.

It translates envelopes into transport.

It translates transport into receipts.

The translation is successful only when later layers preserve earlier
boundaries.

That is why architecture is not merely a diagram.

It is the discipline that keeps the doctrine executable.
