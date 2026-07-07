# 03. Mathematics

## What Mathematics Does In OMI

Mathematics answers what must follow once boundaries are fixed.

Humans choose constraints.

Mathematics derives consequences.

Theorems do not exist to decorate the project.

They exist to prevent narrative overreach.

If the proof exists, the claim may be stated as proved.

If the proof does not exist, the claim remains motivation, interpretation, or
a target.

This chapter exists to keep those categories separate.

## Dependency Graph

The mathematical stack should be read as a dependency graph:

```text
Finite Sets
  -> Finite Geometry
  -> Incidence
  -> Projection
  -> Runtime Semantics
  -> Categories
  -> Coalgebras
  -> Bialgebras
  -> Bridge Theorems
```

This order matters.

Finite incidence is exact.

Projection is interpretation.

Runtime semantics is executable.

Bridge theorems connect layers that should not be confused.

## Finite Structural Geometry

The finite layer contains exact combinatorics.

It does not contain real-analysis claims.

It does not claim that runtime state emits pi.

It states finite closure, incidence, and selector structure.

The main files are:

- `../omi-axioms/coq/DiagonalClosure.v`
- `../omi-axioms/coq/FiniteIncidence.v`
- `../omi-axioms/coq/BQFBridge.v`

Together they describe diagonal closure, tetrahedral and Fano incidence, BQF
decomposition, selectors, local240, and bridge schedules.

This is the layer where relations organize replay.

## Projection

Projection is the boundary where finite schedules are interpreted in real
analysis.

The main files are:

- `../omi-axioms/coq/MetricProjection.v`
- `../omi-axioms/coq/PiProjection.v`

This is where sqrt3, phi, and pi appear.

The important discipline is that the metric constants are not stored in the
finite layer.

They appear at projection boundaries.

That distinction prevents a common misunderstanding.

The proof that an incidence/projection schedule converges to real pi is not
the same as a proof that the C GF(2^16) orbit engine emits pi.

The former exists.

The latter remains a bridge target.

## Runtime Semantics

Runtime semantics is a separate stack.

The GF(2^16) orbit engine uses a different delta family from the rotate/XOR
kernel lineage.

The semantic files include:

- `../omi-axioms/coq/delta_orbit_theory.v`
- `../omi-axioms/coq/functorial_semantics.v`
- `../omi-axioms/coq/coalgebraic_bisimulation.v`
- `../omi-axioms/coq/OMI_bialgebra.v`
- `../omi-axioms/coq/verified_execution.v`

The C artifact is:

- `lib/omi_orbit.c`

The test artifact is:

- `test/test_orbit.c`

This separation is not a weakness.

It is rigor.

It keeps executable dynamics from being confused with projection scaffolds.

## Observers

Observers are the mathematical reason the architecture can separate transition
from readable surface.

A transition produces state.

An observer projects state.

The projection may be Fano class, tetrahedral class, phase, bitboard field,
receipt slot, or envelope offset.

The observer is not the state itself.

It is a quotient view.

That is why visible change need not replace base identity.

The same base relation can have multiple observer-relative surfaces.

## Categories

The category layer asks whether observer behavior respects structure.

If orbits form a groupoid, an observer should not be just an informal map.

It should preserve the relevant equivalences.

This is the reason for functorial semantics.

The goal is not to make the project sound abstract.

The goal is to state exactly when observation commutes with the dynamics it
claims to observe.

## Coalgebra

Coalgebra enters when state is read as stream.

A stream observer does not merely inspect one value.

It unfolds behavior across time.

Coalgebraic bisimulation is therefore a natural language for saying when two
observed processes are behaviorally equivalent.

This matters for OMI because replay is central.

If two structures replay the same way under the same observation boundary, the
system needs a formal language for that sameness.

## Bialgebra

Bialgebra states coherence between construction and observation.

Algebra builds.

Coalgebra observes.

OMI needs both.

If construction and observation disagree, the receipt is not trustworthy.

The bialgebra layer expresses that the way structures are built and the way
they are observed are compatible.

## Bridges

Bridge files should not introduce new foundations.

They should relate established layers.

Current bridge and export files include:

- `../omi-axioms/coq/omi_pi_bridge.v`
- `../omi-axioms/coq/OMI_Exports.v`
- `../omi-axioms/coq/omi_pi_proof.v`

`OMI_Exports.v` is a neutral export surface.

`omi_pi_proof.v` remains only as a legacy compatibility shim.

`omi_pi_bridge.v` currently connects replay index to the pi incidence schedule.

It does not yet prove that runtime orbit state determines that phase schedule.

## Claim Discipline

OMI uses three claim categories:

1. Proven.
2. Tested as an artifact.
3. Interpreted or targeted but not yet proved.

This is why the proof-lineage document is central.

It prevents the podcast narrative from becoming a theorem by accident.

It prevents a diagram from becoming a proof by enthusiasm.

It preserves credibility.

## From Cosmology To Coq

The movement from theological cosmology to Coq rigor is not a contradiction.

It is a boundary discipline.

Cosmology chooses why relation matters.

Foundations state what is irreducible.

Mathematics proves what follows.

Coq mechanically checks those proofs.

The theorem prover does not certify the motivation.

It certifies formal statements.

That distinction is the bridge from narrative to rigor.

Detailed status: [docs/mathematics/proof-lineage.md](docs/mathematics/proof-lineage.md).

## The Proof Ethic

The proof ethic is conservative.

A theorem proves only what it states.

A passing test exercises only the artifact it runs.

A compelling interpretation remains interpretation until formalized.

This restraint is not a reduction of the project.

It is what lets the project make large claims responsibly.

For example, the repository may say that the incidence projection schedule
converges to real pi when the Coq theorem establishes it.

The repository may not say that the C orbit engine emits pi until a bridge
theorem connects runtime observations to that projection schedule.

The distinction is the difference between whitepaper rigor and mythology.

## Mathematics As Consequence

The mathematical layer begins after constraints.

It does not decide that a packet is 64 bytes.

It studies what follows from a 64-byte packet.

It does not decide that a word is 16 bits.

It studies what follows from 16-bit bounded replay.

It does not decide that an observer sees a quotient.

It states when that quotient preserves the structure it claims to observe.

This is why the proof stack belongs after foundations.

The primitive boundary is chosen first.

The theorem follows.

The artifact then becomes accountable to the theorem.
