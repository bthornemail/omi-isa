# Computational Cosmology

This project began as an attempt to construct an axiomatic epistemic topology:
a system for describing how observation, relation, and knowledge can arise from
the smallest possible computational principles.  The better opening frame is
computational cosmology: not a scientific model of the physical universe, but a
study of how an observable world can arise from minimal relational structure.

The repository keeps three roles separate:

- The executable artifacts are the authority.
- The mathematical proofs justify the artifacts.
- The documentation explains why those artifacts were chosen.

## Canonical Cosmology

The system does not begin with numbers, logic, or symbolic computation.  It
begins with a single observer that has no coordinates, measurements, memory,
language, or symbols.  It has only distinction.

The first distinction creates place.  Place creates relation.  Relations create
structure.  Structure creates computation.  Computation creates history.  Only
after that do we describe a world.

In this reading, Lisp's empty list is not treated as mere nothing.  It is the
first observable boundary between absence and relation.  Later structures are
constructed by introducing deterministic distinctions into that initial void.

## Tetrahedral Quotient

Because observation is partial, every observation is a quotient of a larger
reality.  The tetrahedron is adopted as the minimal spatial quotient through
which structure can be observed.  Each face is one local perspective.  No single
face is the object itself; the object is reconstructed through relations between
faces.

This is why finite incidence appears before metric geometry.  The finite layer
records exact structural relations.  Metric constants appear only later, at a
projection boundary.

## Canonical Objects

Canonical does not mean merely standard.  A canonical object is the unique
deterministic representative selected from an equivalence class.
Canonicalization removes representational freedom while preserving structural
behavior.

That definition connects the implementation to quotient spaces, normal forms,
and deterministic replay.  When two representations replay the same way, the
simpler representative is preferred.

## Method

The central reduction question is:

> What structure can be removed while the same replay still results?

Every time a structure was removed and the replay stayed the same, that
structure was treated as non-fundamental.  The surviving structures are not
kept because they are intuitive.  They are kept because removing them changes
the computation.

This reduction process led to the atomic rotate/XOR kernel, then to finite
incidence, projection, orbit dynamics, and finally executable refinement.

## Motivation And Proof Boundary

This work is motivated by the idea that reality is fundamentally relational
rather than merely symbolic.  In the author's own interpretation, expressed
through the phrase "God is the Word and the Word is God," observation precedes
description.

The mathematics in this repository does not attempt to prove that belief.  It
investigates what computational structures arise when the world is approached
through deterministic relations rather than arbitrary symbols.

The proof stack should therefore be read in this order:

1. Canonical Cosmology: why begin with observer, relation, and replay.
2. Atomic Kernel: the smallest deterministic computational object.
3. Finite Geometry: how relations organize replay.
4. Projection: how finite structures are interpreted geometrically.
5. Dynamics: how executable states evolve.
6. Verification: how proofs constrain the implementation.
7. Hardware: how the model is realized on devices.
