# 02. Foundations

## What Cannot Be Reduced

Foundations answers what survives every successful reduction.

This chapter is not about hardware.

It is not about networking.

It is not about Coq tactics.

It is about the irreducible computational object OMI keeps returning to:

```text
boundary
  -> position
  -> relation
  -> pathway
  -> container
  -> transition
  -> replay
```

The atomic replay kernel is the first executable receipt of that sequence.

## The Reduction Principle

The constitutional question is:

> What structure can be removed while the same canonical replay still results?

The companion question is:

> How long can computation proceed before a symbol becomes necessary?

These questions are not slogans.

They are design tests.

If removing a structure preserves replay, that structure is not fundamental.

If removing a structure changes replay, that structure marks a boundary.

OMI does not begin by asking what names should be assigned.

It asks which places, pathways, and containers must exist before naming is
useful.

## Place Before Symbol

Traditional systems often begin by naming things.

OMI begins by placing things.

In a symbolic expression:

```text
x + y = z
```

the labels appear to carry identity.

In a positional structure:

```text
[0] -> [1] -> [2]
```

the positions exist before names are assigned.

Only later might a projection say:

```text
[0] = CAR
[1] = CDR
[2] = STACK
```

The names are useful.

They are not primitive.

A variable is already an interpretation.

A place is not.

## Pathways And Containers

OMI attempts to postpone symbolic naming for as long as possible.

The primitive objects are:

- positions
- pathways
- boundaries
- containers
- transitions
- observers
- receipts

A pathway is a stable traversal route before it receives a name.

A container is a bounded location before it receives application semantics.

A receipt is a canonical record before it receives narrative explanation.

This is why the project often speaks in offsets, slots, gauges, nibbles,
register positions, orbit positions, and envelope fields.

Those are addresses before they are meanings.

## The Lisp Root

Lisp matters here, but not merely because "code is data."

The more important root is relational:

```text
NULL
  -> Pair
  -> CAR
  -> CDR
```

`NULL` marks the first boundary.

Pair introduces adjacency.

`CAR` gives structural descent.

`CDR` gives continuation.

Every object is located by traversal.

This is closer to OMI's foundation than a conventional variable environment.

## The Atomic Kernel

The first formal receipt is `../omi-axioms/coq/AtomicKernel.v`.

It proves the small things first:

- width is bounded
- masks close over values
- rotations stay inside the mask
- delta is deterministic
- replay is deterministic
- mask idempotence holds

This file is the historical root.

It is not a pi proof.

It is not a geometry proof.

It is not a runtime orbit proof.

It says:

```text
here is the smallest executable object
```

`../omi-axioms/coq/AtomicKernelVNext.v` is the cleaned continuation.

It keeps the original kernel as provenance and adds replay length and bounded
sequence structure.

## Delta

Delta is the deterministic transition boundary.

In the rotate/XOR kernel lineage, delta is expressed through masked bitwise
motion:

```text
rotate
  -> xor
  -> mask
  -> replay
```

The point is not that every later OMI layer uses one identical delta function.

The point is that every later layer must respect the distinction between
transition and projection.

There is a rotate/XOR delta lineage.

There is a GF(2^16) orbit delta lineage.

They are related research tracks, not interchangeable slogans.

## OMI And IMO

The notation:

```text
omi---imo
```

is treated as a reciprocal base relation.

OMI and IMO name an outgoing and returning orientation.

The base relation can remain canonical while observers project readable
surfaces from it.

That is why:

```text
o---o/---/?---?@---@
```

is not a replacement identity.

It is an observer surface.

It is how the base relation becomes inspectable, routable, queryable, and
receivable.

## Idempotence

Idempotence does not mean that the world freezes.

It means that applying the canonicalization boundary again does not create a
new identity.

In the kernel, mask idempotence says that once a value is inside the bounded
width, masking again preserves it.

In the narrative layer, the same idea appears as stable base relation and
observer-relative projection.

The base can remain canonical while the surface changes.

The observer can read change without declaring a new root identity.

## Foundation Vocabulary

The foundational vocabulary is intentionally small:

- `NULL`: observable boundary where relation can begin.
- Pair: first composite adjacency.
- `CAR`: structural descent.
- `CDR`: continuation.
- Place value: position before symbolic meaning.
- Pathway: stable traversal before naming.
- Container: bounded location before payload semantics.
- Mask: explicit width boundary.
- Delta: deterministic transition law.
- Replay: repeated transition under the same boundary.
- Observer: quotient projection of transition into readable surface.
- Receipt: canonical record of what was observed.

## What This Layer Must Not Do

This layer must not pretend that every philosophical term is already a theorem.

It must not collapse rotate/XOR delta into GF(2^16) delta.

It must not claim that pi emerges from runtime state before a bridge theorem
exists.

It must not treat hardware as the origin of truth.

It must define the smallest stable vocabulary from which later layers can be
derived.

## Foundation To Mathematics

Once the foundational boundaries are fixed, mathematics begins.

Finite incidence can organize replay.

Projection can interpret finite schedules.

Orbit theory can describe executable dynamics.

Category theory can state observer functoriality.

Coalgebra can describe streams.

Bialgebra can state coherence between construction and observation.

But all of that comes after the foundational reduction.

The root remains:

```text
place before symbol
relation before object
replay before interpretation
artifact before prose
```

## The No-Variable Instinct

The project does not literally ban variables from every implementation file.

C has variables.

Coq has names.

Markdown has headings.

The point is deeper.

OMI postpones the moment when a name is allowed to carry identity.

Names are permitted when they serve a stabilized pathway.

Names are not permitted to be the foundation.

The foundation is:

```text
position
  -> adjacency
  -> traversal
  -> containment
  -> transition
  -> replay
```

This is why the project keeps returning to registers, offsets, masks, slots,
nibbles, and fields.

Those structures are places.

They can be occupied by changing values without losing their role.

## Boundary Inventory

The foundational boundaries currently visible in the repository include:

- 16-bit words.
- 8 general registers.
- 64-byte envelopes.
- 32 dispatch slots.
- 128 gauge entries.
- 7 Fano classes.
- 4 tetrahedral classes.
- 240 local phase positions.
- 5040 atlas slots.

These numbers should not be treated as mystical proof.

They are boundaries.

Once chosen, they have consequences.

The job of mathematics is to derive those consequences.

The job of implementation is to preserve them.

The job of documentation is to keep the distinction clear.
