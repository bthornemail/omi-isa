# 01. Cosmology

## Why This Chapter Exists

Cosmology answers why OMI exists.

Not how it compiles.

Not how packets are routed.

Not which theorem proves which lemma.

Those questions come later.

This chapter names the motivation that comes before architecture.

OMI is framed as a computational cosmology: not a scientific model of the
physical universe, but a disciplined inquiry into how an observable world can
arise from minimal relational structure.

The point is not to prove reality.

The point is to choose a starting boundary and then let the artifacts speak.

## The Human Boundary

Humans do not create truth in this model.

Humans define admissible boundaries.

They say:

```text
this word is 16 bits
this packet is 64 bytes
this register file has eight positions
this protocol has fixed fields
this topology has four faces
this observer can see one projection
```

Those are design constraints.

Once the constraints are fixed, mathematics is no longer a matter of opinion.

Orbit lengths follow.

Invariants follow.

Kernels follow.

Fixed points follow.

Quotients follow.

Receipts follow.

Engineering then asks what hardware or software can realize those structures.

The resulting cycle is:

```text
Observation
  -> Boundary
  -> Constraint
  -> Mathematics
  -> Algorithm
  -> Implementation
  -> Measurement
  -> Observation
```

This cycle is the cosmological frame of OMI.

## The First Distinction

The system does not begin with numbers.

It does not begin with logic.

It does not begin with syntax.

It does not begin with variables.

It begins with distinction.

An observer appears only as the boundary where distinction becomes possible.

The observer has no coordinates.

No memory.

No type.

No language.

No symbol table.

Only the possibility that one side of a boundary is not the other.

From there:

```text
Observer
  -> Distinction
  -> Place
  -> Relation
  -> Structure
  -> Replay
  -> Projection
  -> Execution
  -> History
  -> World
```

Execution appears late in this sequence.

That matters.

OMI is not first a machine that later gains meaning.

It is first a discipline of boundary and relation that later becomes
executable.

## Empty List, Void, And Relation

The empty list is not treated as mere nothing.

It is the first observable boundary between absence and relation.

In Lisp, people often say that code is data.

For OMI, the more important observation is that the list is relational.

The empty list marks a boundary.

The pair introduces adjacency.

`CAR` and `CDR` introduce traversable pathways.

Names can arrive later.

The pathway exists before the name.

This is why OMI does not begin with a variable.

A variable already interprets a place.

OMI wants the place first.

## Theological Motivation And Formal Boundary

This work is motivated by the idea that reality is fundamentally relational
rather than merely symbolic.

The author's theological language, including the phrase "God is the Word and
the Word is God," belongs to motivation.

It explains why relation, word, observation, and boundary matter to the
project.

It is not a theorem.

It is not an assumption inside Coq.

It does not replace the executable artifact.

The mathematics in this repository does not attempt to prove that
philosophical motivation.

It investigates what computational structures arise when the world is
approached through deterministic relations rather than arbitrary symbols.

That distinction is essential.

OMI can be philosophically motivated without being formally overclaimed.

## Canonical Does Not Mean Conventional

Canonical does not mean "standard because everyone says so."

In OMI, canonical means:

```text
the representation remaining after arbitrary implementation choices
have been removed
```

Different ESP32 boards can implement the same observer.

Different radios can preserve the same receipt.

Different programming languages can realize the same transition function.

Different browser surfaces can expose the same protocol.

What survives those changes is canonical.

The canonical form is not the decoration.

It is the invariant representative.

## Tetrahedron As Quotient

Because human knowledge is necessarily partial, every observation is treated as
a quotient of a larger structure.

The tetrahedron is adopted as the smallest three-dimensional object whose four
faces provide multiple local observations while remaining globally
reconstructible through incidence relations.

Each face is a local view.

No face is the whole object.

The object is reconstructed through the relations between faces.

This is why finite incidence appears before metric geometry.

The finite layer records exact structural relations.

Metric constants appear only later, at projection boundaries.

## Observer Surfaces

The Observer model says that OMI does not treat every visible change as a new
identity.

Identity remains anchored in the base reciprocal relation:

```text
omi---imo
```

Changes are interpreted as observer-relative projections across the extended
surface:

```text
o---o/---/?---?@---@
```

One way to read this surface is:

```text
o---o   entry or object boundary
/---/   path or flow
?---?   query, observation, or incidence
@---@   address, closure, or receipt
```

The base relation is stable.

The surface is readable.

This is related to idempotence, but it does not mean that nothing ever changes.

It means the same canonical relation can be inspected through different
observers without replacing its identity.

## Probes Are Not Truth

Most engineering starts with a sensor.

OMI starts with a mathematical observer.

The hardware becomes a probe that satisfies the observer interface.

For example:

```text
Temperature
  -> ADC
  -> 16-bit word
  -> Delta
  -> Observer
  -> Receipt
```

And:

```text
Light
  -> Photodiode
  -> Voltage
  -> Sample
  -> Delta
  -> Witness
```

The ADC is not truth.

The photodiode is not truth.

They are physical probes that satisfy a predefined observational boundary.

This is the role of OMI-Sense.

It is not merely a sensor framework.

It is a collection of hardware probes designed to realize predefined observer
interfaces.

The interface comes first.

The hardware comes second.

## The Layer Contract

Cosmology motivates which boundaries are chosen.

Foundations state those boundaries precisely.

Mathematics derives what must follow from them.

Architecture organizes those consequences into a protocol.

Implementation realizes the protocol in software and hardware.

Proofs verify that implementation preserves mathematical structure.

This chapter is therefore not a proof.

It is the constitutional preamble.

It tells the reader why the system has the shape it has.

Supporting material: [docs/cosmology/vision.md](docs/cosmology/vision.md).

## Cosmological Commitments

This chapter commits to a beginning, not to a completed metaphysics.

It begins from observation.

It treats observation as partial.

It treats partial observation as quotient.

It treats quotient as a disciplined boundary rather than a loss of meaning.

It treats projection as late.

It treats symbols as useful, but not primitive.

These commitments explain why the repository is shaped the way it is.

The proof files are not isolated mathematical papers.

They are attempts to make parts of this cosmology accountable.

The C runtime is not an accidental implementation.

It is an attempt to discover what this cosmology looks like when it must fit
inside machine boundaries.

The firmware is not a decoration.

It is an attempt to ask whether the same observer surfaces survive physical
probes.

## Idempotent Identity And Readable Change

The phrase "idempotent base relation" can sound like a denial of change.

That is not the intention.

The intention is that identity is anchored before projection.

If the base relation is canonical, then repeated canonicalization does not
create a new root.

Observers can still expose differences.

Receipts can still record changes.

Queries can still distinguish surfaces.

The difference is that visible variation is not immediately treated as a new
identity.

That is the function of:

```text
omi---imo
```

as base relation, and:

```text
o---o/---/?---?@---@
```

as observer surface.

The base holds.

The surface reads.

The receipt records.

The proof layer must then say which parts of that story are formally
established.
