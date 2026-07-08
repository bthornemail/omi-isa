# OMI-ISA Document Alignment Pass

Status: working alignment brief  
Scope: `/home/main/omi/omi-isa` document cleanup  
Goal: make the repository read as `omi-isa`, not as the whole OMI universe

## Alignment Doctrine

`omi-isa` is the executable instruction, envelope, dispatch, observer, transport,
mesh, and receipt implementation surface.

It may carry motivation and proof lineage, but it must not present itself as the
entire canon, the entire cosmology, or the extracted `omi-port` route-candidate
library.

The governing sentence for the repo should be:

```text
OMI-ISA implements the bounded executable surface of OMI: fixed-width
instructions, deterministic replay, 512-bit envelopes, dispatch, observers,
transport adapters, mesh routing, and receipts.
```

Everything else is supporting context.

## Authority Order

The root docs should use one consistent authority ladder:

```text
1. Executable artifacts
2. Tests
3. Compiling proofs
4. Architecture/spec documents
5. Foundation and mathematics explanations
6. Cosmological motivation
7. Development archive
```

Short form:

```text
Artifacts define.
Tests guard.
Proofs justify.
Docs explain.
Cosmology motivates.
Archive preserves lineage.
```

## Extracted Boundary

The repository must explicitly name what has moved out of `omi-isa`.

```text
omi-lisp
    declaration and symbolic surface

omi-port
    dormant source-target-gauge route-candidate derivation

proof/canon extraction
    formal and constitutional proof lineage where separated

omi-isa
    executable bounded runtime surface
```

Recommended root wording:

```text
This repository is no longer the whole OMI canon. It is the ISA/runtime
surface. Extracted sibling work such as omi-lisp and omi-port may feed or
consume this surface, but they do not belong to this repository's authority
boundary.
```

## Root Document Roles

| File | Aligned Role | Needed Action |
| --- | --- | --- |
| `README.md` | front door for `omi-isa` only | Rewrite lead around ISA/runtime boundary; add extracted boundary note |
| `01_COSMOLOGY.md` | motivation preamble | Keep, but mark non-normative; remove any claim that cosmology proves runtime behavior |
| `02_FOUNDATIONS.md` | reduction vocabulary | Keep small vocabulary; emphasize fixed boundaries used by ISA |
| `03_MATHEMATICS.md` | proof and consequence map | Keep as lineage/status; distinguish proven/tested/targeted |
| `04_ARCHITECTURE.md` | normative protocol architecture | Make this the main technical doc |
| `05_IMPLEMENTATION.md` | build/test/artifact map | Make current tree and commands authoritative |
| `06-SUMMARY.MD` | likely duplicate summary | Either replace with short index or move to `dev-docs/` |
| `GL(16,2) Orbit Execution Model -- Complete Write-Up.md` | orbit lineage | Move or reference from `docs/mathematics/` unless it is active spec |
| `docs/**` | supporting reference | Align headings to the five root chapters |
| `tree.txt` | snapshot only | Regenerate after cleanup or move to `dev-docs/` |

## README Target Shape

The README should stop opening as universal OMI and start as `omi-isa`.

Proposed opening:

```markdown
# OMI-ISA

OMI-ISA is the bounded executable runtime surface of OMI. It implements the
fixed-width instruction stream, deterministic VM replay, 512-bit envelope
transport, dispatch table, observer projections, transport adapters, mesh
routing, and receipt surfaces.

This repository is not the entire OMI canon. It is the ISA/runtime layer.
Sibling repositories such as `omi-lisp` and `omi-port` provide declaration and
route-candidate surfaces around it. Proof and canon documents justify or
explain the layer, but the executable artifacts and tests define this repo's
current behavior.
```

Then keep:

```text
make
make test
make -B proof
```

But mark `make -B proof` as conditional if the current tree no longer carries
the full proof stack.

## Chapter Alignment

### `01_COSMOLOGY.md`

Keep:

- observer before symbol
- boundary before architecture
- probes are not truth
- cosmology motivates, not proves

Add near top:

```text
Normative status: motivational. This chapter explains why the ISA was shaped
around boundary, place, relation, observer, and receipt. It does not define
opcode behavior, proof status, or runtime compatibility.
```

### `02_FOUNDATIONS.md`

Keep:

- place before symbol
- `NULL`, pair, `CAR`, `CDR`
- mask, delta, replay
- rotate/XOR lineage versus GF(2^16) orbit lineage

Tighten around `omi-isa`:

```text
For this repository, foundations become concrete where they appear as fixed
machine boundaries: 16-bit instruction words, 8 registers, 64-byte envelopes,
32 dispatch slots, bounded replay, and observer projections.
```

### `03_MATHEMATICS.md`

Keep:

- finite incidence
- projection boundary
- runtime semantics separation
- proven/tested/targeted categories

Add:

```text
Mathematics is included here as lineage and compatibility discipline. It is not
a license to overstate what the C runtime proves. Runtime claims are made by
tests and by explicit bridge theorems where those theorems exist.
```

### `04_ARCHITECTURE.md`

Make this the technical center.

Target pipeline:

```text
OMI-Lisp input or bytecode
  -> compiler / loader
  -> 16-bit ISA
  -> VM
  -> observers
  -> 512-bit envelope
  -> dispatch
  -> transport
  -> mesh
  -> receipts
```

Important boundary:

```text
OMI-Port route candidates are not authority inside omi-isa. If consumed later,
they enter as external candidate routes and must pass this repository's runtime,
observer, and receipt boundaries.
```

### `05_IMPLEMENTATION.md`

Make this match the actual tree.

From the provided tree, active implementation surfaces include:

```text
lib/
    compiler, parser, VM, ISA
    envelope, stream, sector
    dispatch, gauge execution
    orbit, projective geometry, sense
    transport, LoRa transport, simulator transport
    mesh, probe, receipt, omicron/omion

test/
    dispatch, env, gauge_exec, mesh, omicron, omion, sense,
    orbit, projective geometry, radio VM, receipt

web/
    browser bridge, serial, audio, viewer, face chain

firmware/
    ESP32-S3 / LoRa transport surface
```

If `proof/` is not present in the current repo, remove `proof/` from the active
source layout and describe it as extracted or external lineage.

## Cleanup Decisions

### Keep In Root

```text
README.md
01_COSMOLOGY.md
02_FOUNDATIONS.md
03_MATHEMATICS.md
04_ARCHITECTURE.md
05_IMPLEMENTATION.md
CONTRIBUTING.md
LICENSE
Makefile
main.c
toolchain_main.c
```

### Move Or Demote

```text
06-SUMMARY.MD
GL(16,2) Orbit Execution Model -- Complete Write-Up.md
tree.txt
```

Recommended destinations:

```text
docs/mathematics/gl16-orbit-execution-model.md
dev-docs/06-SUMMARY.MD
dev-docs/tree-v0-precleanup.txt
```

### Do Not Touch Yet

```text
lib/
test/
web/
firmware/
programs/
scripts/
```

Document alignment comes first. Behavior cleanup comes after the docs say what
the repo is.

## Terminology Lock

Use these terms consistently:

| Term | Use |
| --- | --- |
| ISA | fixed executable instruction boundary |
| VM | deterministic execution artifact |
| envelope | 64-byte bounded observer/transport surface |
| dispatch | 32-slot handler selection |
| observer | quotient projection of state |
| transport | carrier adapter preserving envelope boundary |
| mesh | multi-hop routing over transport |
| receipt | canonical evidence of bounded observation |
| candidate | non-authoritative derived route or declaration |
| authority | validation/receipt/acceptance only |

Avoid using `canon` as if every doc in this repo is normative. Say `lineage`,
`motivation`, `supporting reference`, or `external canon` where appropriate.

## First Patch Order

1. Rewrite `README.md` lead and repository map.
2. Add normative-status blocks to `01_COSMOLOGY.md`, `02_FOUNDATIONS.md`, and
   `03_MATHEMATICS.md`.
3. Tighten `04_ARCHITECTURE.md` around the executable pipeline.
4. Update `05_IMPLEMENTATION.md` to match the actual tree and extracted proof
   boundary.
5. Decide whether `06-SUMMARY.MD` becomes a short index or moves to `dev-docs/`.
6. Move the GL(16,2) write-up into `docs/mathematics/` if it remains active.
7. Regenerate `tree.txt` only after the cleanup.

## Alignment Gate

After document edits:

```sh
rg -n "whole OMI canon|entire canon|proves reality|philosophy proves|podcast transcript proves" .
rg -n "omi-port|omi-lisp|proof/" README.md 01_COSMOLOGY.md 02_FOUNDATIONS.md 03_MATHEMATICS.md 04_ARCHITECTURE.md 05_IMPLEMENTATION.md
make test
```

Expected result:

```text
The docs state that omi-isa is the executable ISA/runtime surface.
Cosmology remains motivation.
Mathematics remains proof/status discipline.
Architecture and implementation carry the technical contract.
Extracted siblings are named as external boundaries.
```

