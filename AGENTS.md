# AGENTS

This repository is the OMI ISA workspace.

Treat `omi-isa` as the ISA definition and firmware layer. Do not turn it into an authority layer for notation or protocol.

## Ground Rules

- Keep this repository scoped to ISA definition, firmware, and toolchain work unless explicitly told otherwise.
- Do not modify other omi repos from this repo task.
- Preserve existing Makefile targets and test structure.

## Canonical OMI Notation

The canonical notation surface is `omi---imo?O_o`.

This notation is not arbitrary. It is derived from the byte table structure
and carries its own derivation history.

### Derivation chain

Byte table:
  0x1F US = hinge point (last hidden unit separator)
  0x2F /  = first printable separator
  0x3F ?  = witness/query separator
  0x4F O  = upper/norm Omicron
  0x5F _  = carrier/floor separator
  0x6F o  = lower/local omicron

The F-column projects tangentially from 0x1F into the printable branch.
?O_o is the readable branch of hidden US.

### 8-tuple (pedagogical scaffold)

The 8-tuple is a pedagogical automata-style scaffold for explaining
how OMI notation may be read as a bounded relation frame. It is not
canonical structure, not an authority layer, and not a proof object.

Q Σ L R δ s t r

Q = state space / identity context
Σ = alphabet / allowed symbols
L = left / source / beginning
R = right / target / ending
δ = transition / transformation
s = start value
t = accept / success / terminal output
r = reject / failure / alternative

Address cascade (explanatory overlay, not canonical identity):
  S0 may be explained as Q
  S1 may be explained as Σ
  S2 may be explained as L
  S3 may be explained as R
  S4 may be explained as δ
  S5 may be explained as s
  S6 may be explained as t
  S7 may be explained as r

Register compression (pedagogical pairings):
  CAR      ≈ Q / Σ
  CDR      ≈ L / R
  PAYLOAD  ≈ δ / s
  MASK     ≈ t / r

### Null Ring

(NULL . NULL) is the folded null seed.

Byte ring:
  0x00 ^ 0x20 = 0x20
  0x20 ^ 0x7F = 0x5F
  0x7F ^ 0xFF = 0x80
  0xFF ^ 0x00 = 0xFF

Full witness closure: 0x20 ^ 0x5F ^ 0x80 ^ 0xFF = 0x00

### Agent rules

- Use omi---imo?O_o as the canonical notation surface
- Do not infer notation from older nearby files
- Do not promote #/ from carrier compatibility into OMI-native syntax
- Do not treat the 8-tuple as 8×16-bit unless discussing later lowering
- Do not treat the four dotted pairs as 4×32-bit integer widths
- Do not treat the 8-tuple as canonical structure, authority, or proof object
- Do not treat geometry drawings as authority
- Do not let projection, route, notation, or gauge become authority

### Authority order

Omnicron frames.
Tetragrammatron validates.
Metatron scribes.
Receipt records.

### Canonical source documents

The full derivation lives in omi-canon:
- `dev-docs/The 8-Tuple Basis of \`omi---imo?O_o\`.md`
- `dev-docs/OMI Gauge Table, F-Column Surface, and.md`
- `docs/The 8-Tuple Basis of omi---imo?O_o.md`
