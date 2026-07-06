# GL(16,2) Orbit Execution Model — Anchored Summary

## 1. Core State Space

State is a 2-tuple:

    S = (x, c)
    x ∈ GF(2^16)
    c ∈ GF(2^16) (control parameter, constant per orbit)

---

## 2. Dynamics (Δ Operator)

The system evolves via a linear group action:

    x' = A(x) ⊕ c
    c' = c

Where:

- A ∈ GL(16,2)
- A is multiplication by a primitive element in GF(2^16)
- Operation is linear over GF(2)

Thus:

    Δ : S → S
    Δ(x,c) = (A·x ⊕ c, c)

---

## 3. Orbit Semantics

Each initial state generates a finite orbit:

    orbit(s) = { Δⁿ(s) | n ≥ 0 }

All orbits are finite due to GL(16,2) finiteness.

Cycle detection is well-defined.

---

## 4. Observers (Quotient Maps)

Observers are functions:

    obs : S → A

All invariants are observer-dependent projections:

### Canonical observers

- Fano:
      x mod 7

- Tetra:
      x mod 4

- Phase:
      x mod 2

- BQF:
      quadratic form over GF(2^16)

- Slot5040:
      structured quotient:
      (Fano × Tetra × Phase × residue)

---

## 5. Category of Observers

The observer category is formalized in Coq as a dependent category over
the state dynamics.  Every observer is an equivariant map:

    obs(Δ(s)) = fA(obs(s))

This commuting diagram defines the morphisms of the observer category.
The main theorem (`all_observers_are_equivariant_maps`) states that for any
observer, such an fA exists.

---

## 6. Orbit Groupoid 𝒪

Objects = states.  Morphisms = forward Δ-iterates:

    𝒪_hom(s,t) = { Δⁿ | n : ℕ, Δⁿ(s) = t }

𝒪 is a thin category (at most one morphism between any two objects).
It is the orbit groupoid of the deterministic dynamical system Δ.

Category laws (identity, composition, associativity) are proved in Coq
(`functorial_semantics.v`).

---

## 7. Functor Theorem

**Every observer defines a functor Obs : 𝒪 → FinSet.**

- On objects:  s ↦ A (the observation set)
- On morphisms:  Δⁿ ↦ (fA)ⁿ (the induced dynamics on A)

Functoriality follows from the equivariance condition by induction on n:

    obs(Δⁿ(s)) = (fA)ⁿ(obs(s))

This is the core theorem `functor_theorem` in `functorial_semantics.v`.
Concrete observers (Fano, Tetra, BQF) are corollaries.

---

## 8. Trace Equivalence

The Coq trace of an observer matches sequential application of fA:

    trace_obs A o s n = [obs(s), obs(Δ(s)), ..., obs(Δⁿ(s))]
    nth k (trace_obs ...) = (fA)ᵏ(obs(s))   for k ≤ n

This bridges the Coq formalization to the C execution layer.

---

## 9. Execution Layer

Implemented in C (`omi_orbit.c/h`):

- GL(16,2) multiplication table A[]
- Control injection c
- Deterministic step function Δ
- Visited-array cycle detection
- Trace extraction

---

## 10. Formal Layer (Coq)

Two files in `proof/`:

- `delta_orbit_theory.v`  — GL(16,2) linear dynamics, Observer record, ObsHom
  category, concrete observers (Fano, Tetra, Phase, BQF), 5040 atlas.

- `functorial_semantics.v` — orbit groupoid 𝒪, functor theorem, trace
  equivalence, extraction interface.

---

## 11. Extraction Interface

Coq constants A and B are extracted to OCaml via `Extract Constant`,
matching the C implementation in `omi_orbit.c`:

    A(x) = (x << 1) ⊗ 0x002D   (GF(2^16) multiplication by α)
    B(x) = x                   (identity linear map)

---

## 12. Fundamental Claim

The system is not a program.

It is a finite linear dynamical system:

    Δ ∈ GL(16,2) ⋉ GF(2^16)

and all structures are quotient observers over its orbits.

Nothing exists outside Δ and its induced category of observers.
