# 02. Foundations

This layer answers what cannot be reduced further.

The foundation of OMI is the atomic replay kernel: a deterministic transition
over bounded machine words.  Before hardware, networking, or real analysis,
the project asks for the smallest executable object that preserves replay.

## Reduction Principle

The central question is:

> What structure can be removed while the same replay still results?

Every time a structure was removed and the replay stayed the same, that
structure was treated as non-fundamental.  The surviving structures are kept
because removing them changes the computation.

## Core Primitives

The foundational vocabulary is intentionally small:

- `NULL`: the observable boundary where relation can begin.
- Pair: the first composite relation.
- `CAR`: structural descent into the head of a relation.
- `CDR`: continuation through the tail of a relation.
- Place value: position before symbolic meaning.
- Bounded width: all replay occurs inside an explicit mask.
- Delta: the deterministic transition law.
- Rotate/XOR: the smallest reversible-looking bitwise vocabulary used by the
  original kernel.

## Atomic Kernel Lineage

The proof lineage begins with `proof/AtomicKernel.v`.  That file is the
historical root: mask, rotate, delta, sequence determinism, and mask
idempotence.

`proof/AtomicKernelVNext.v` is the cleaned replay continuation.  It keeps the
origin proof as provenance and adds bounded replay structure, replay
determinism, and replay length.

Everything after this layer is an interpretation, organization, projection, or
execution of the atomic replay idea.
