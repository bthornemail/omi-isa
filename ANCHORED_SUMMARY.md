# GL(16,2) Orbit Execution Model — Anchored Summary

## 1. Core State Space

State is a 2-tuple:

    S = (x, c)
    x ∈ GF(2^16)
    c ∈ GF(2^16) (control parameter, constant per orbit)

---

## 2. Dynamics (Δ Operator)

The system evolves via a linear group action:

    x' = A(x) ⊕ B(c)
    c' = c

Where:

- A ∈ GL(16,2) is multiplication by a primitive element in GF(2^16)
- B is the identity map (control injection)
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
      quadratic form over Z (60*x^2 + 16*x*y + 4*y^2)

- Slot5040:
      structured quotient coordinate system:
      (Fano × Tetra × Phase × residue)

---

## 5. Category of Observers

Objects:

    Ob = state → A

Morphisms:

    f : A → B
    commuting with Δ:

    f ∘ obs ∘ Δ = obs' ∘ Δ

This forms a dependent category over the state dynamics.

---

## 6. Orbit Groupoid 𝒪

Objects:
- States $S = (x, c)$

Morphisms:
- Forward $\Delta$-iterates ($\Delta^n$), witnessed by the equation $\text{step}^n(s) = t$

Since there is at most one morphism between any two states, $\mathcal{O}$ is a thin category (preorder category).
- Identity: Zero-step iterate ($\Delta^0$)
- Composition: Sum of steps ($\Delta^m \circ \Delta^n = \Delta^{n+m}$)

---

## 7. Functor Theorem (Obs : 𝒪 → FinSet)

Every equivariant observer $o : S \to A$ with induced dynamics $f_A : A \to A$ defines a functor $Obs$ from the orbit groupoid $\mathcal{O}$ into the category of finite sets ($FinSet$):
- On objects: $s \mapsto A$
- On morphisms: $\Delta^n \mapsto (f_A)^n$

The equivariance condition:
$$\text{obs}(\text{step}(s)) = f_A(\text{obs}(s))$$
implies functoriality via `functor_theorem`:
$$\text{obs}(\text{step}^n(s)) = (f_A)^n(\text{obs}(s))$$
which is proved in Coq by induction on $n$.

---

## 8. Trace Equivalence

The trace of observations of length $n$ matches the sequential application of $f_A$ on the initial observation:
$$\text{trace\_obs}(A, o, s, n) = [\text{obs}(s), f_A(\text{obs}(s)), \dots, (f_A)^n(\text{obs}(s))]$$

Pointwise equivalence: the $k$-th element of the trace (where $k \le n$) corresponds to $(f_A)^k(\text{obs}(s))$, proved via Coq lemma `trace_obs_pointwise`. Full list equality is proven in `trace_obs_equiv` using list extensionality (`nth_ext`) and sequential indices (`seq_nth`).

---

## 9. Coalgebraic Semantics (F-Coalgebra Layer)

A third Coq file, [coalgebraic_bisimulation.v](file:///home/main/omi/omi-isa/proof/coalgebraic_bisimulation.v), provides a general coalgebraic semantics that unifies the observer and orbit frameworks:

### Output Functor

$$F(X) = X \times O$$

where $O$ is the observation type. This is the standard "stream" functor for state machines with output.

### F-Coalgebras

An $F$-coalgebra is a pair $(C, \langle \text{step}, \text{obs} \rangle : C \to F(C))$, represented by the `Coalgebra` record:

| Field       | Type         | Meaning                  |
|-------------|-------------|--------------------------|
| `Carrier`   | `Type`      | State type               |
| `step_obs`  | `C → C × O` | One-step transition + observation |

### Coalgebra Morphisms

A morphism $m : C_1 \to C_2$ is an equivariant map commuting with both step and observation:

$$m(\text{step}_{C_1}(x)) = \text{step}_{C_2}(m(x))$$
$$\text{obs}_{C_1}(x) = \text{obs}_{C_2}(m(x))$$

Proved: **every morphism induces a bisimulation** (`morphism_bisimulation`).

### Key Theorems

1. **`trace_equivalence`**: For any bisimulation $B$ relating $x \in C_1$ and $y \in C_2$, the observation traces of length $n$ are identical — $C_1$ and $C_2$ are observationally indistinguishable.

2. **`morphism_equiv`**: The equivariance condition $\text{obs}(\text{step}(s)) = f(\text{obs}(s))$ defines a coalgebra morphism from the state coalgebra to the target (observation-only) coalgebra.

3. **`morphism_trace_equivalence`**: Combines the above two: given equivariant $o : S \to A$ with $f_A : A \to A$, the trace of $o$ over $\Delta$ equals the trace of $f_A$ starting from $o(s)$:

   $$\text{trace\_obs}(\text{state\_coalgebra}(o), n, s) = \text{trace\_obs}(\text{target\_coalgebra}(f_A), n, o(s))$$

This bridges the orbit groupoid semantics (functor $Obs : \mathcal{O} \to \text{FinSet}$) with the coalgebraic stream semantics (final coalgebra of $F$), providing a third, independent formal proof that observation traces are determined entirely by the initial observation and the induced dynamics $f_A$.

### 4. Bialgebra Structure (Algebra ⊗ Coalgebra)

A fourth Coq file, [OMI_bialgebra.v](file:///home/main/omi/omi-isa/proof/OMI_bialgebra.v), unifies the algebraic structure (GL(16,2) action) with the coalgebraic structure (observation unfolding) into a single bialgebra record:

| Record     | Fields                          | Meaning                                        |
|------------|---------------------------------|------------------------------------------------|
| `Alg`      | `carrier`, `act`                | Algebra: carrier set $X$ with $A$-action $\cdot : X \to X$ |
| `CoAlg`    | `carrier`, `obs_coalg`, `step_coalg` | Coalgebra: carrier set $X$ with step $\Delta : X \to X$ and observation $\text{obs} : X \to O$ |
| `Bialg`    | `act_bialg`, `obs_bialg`, `step_bialg`, `fA_bialg`, `step_eq_act`, `distrib_law` | Bialgebra: algebraic $A$-action and coalgebraic unfolding satisfying the distributive law $\text{obs} \circ \text{step} = f_A \circ \text{obs}$ and $\text{step} = \text{act}$ |

Key components:

- **Coinductive stream**: `CoInductive stream (A : Type) : Type := Cons : A -> stream A -> stream A.` with `get_nth` for indexing — used as the final coalgebra intuition.
- **Section ObserverBialgebras**: Parameterized by an `Observer A o`, constructs:
  - `alg_state` — algebra $(S, A)$ where $A(s) = \text{step}(s)$
  - `coalg_obs` — coalgebra $(S, \text{step}, o)$
  - `bialg_obs` — the unifying bialgebra
  - `obs_stream` — the cofixpoint infinite observation stream $\text{Cons}(o(s), \text{obs\_stream}(\text{step}(s)))$

- **`bialgebra_commutation`**: $\text{obs}(\Delta^n(s)) = (f_A)^n(\text{obs}(s))$ — directly follows from the functor theorem and the bialgebra distributive law.
- **`bialgebra_coherence`**: $\text{get\_nth}(n)(\text{obs\_stream}(s)) = (f_A)^n(\text{obs}(s))$ — the core structural identity equating the $n$-th observation in the infinite coinductive stream with $n$-fold application of $f_A$ on the initial observation. This is the bialgebra analogue of `trace_obs_pointwise`.

**Concrete instances** (one per observer):
- `bialg_fano` — bialgebra with `N` observation type and Fano modulus 7
- `bialg_tetra` — bialgebra with `N` observation type and Tetra modulus 4
- `bialg_energy` — bialgebra with `Z` observation type and BQF energy

Each is extracted to OCaml via `Extraction "omi_bialgebra_extracted.ml"`.

---

## 10. Execution Layer

Implemented in C (`omi_orbit.c` / `omi_orbit.h`):

- $\text{GL}(16,2)$ multiplication by primitive element $\alpha$ modulo polynomial `0x1002D`
- Precomputed lookup tables for operators $A$ and $B$ (identity)
- Step function `omi_orbit_step`
- Fast cycle detection and trace extraction using a 16-bit visited-array cache
- Quotient maps: Fano, Tetra, Phase, and Slot5040

---

## 11. Formal Layer (Coq)

The formalization is split into four Coq files:

- [delta_orbit_theory.v](file:///home/main/omi/omi-isa/proof/delta_orbit_theory.v): Encodes the $\text{GL}(16,2)$ linear operators, state type, step function, trace, Observer and ObsHom category records, modulus observers, and 5040 orbit atlas.
- [functorial_semantics.v](file:///home/main/omi/omi-isa/proof/functorial_semantics.v): Encodes the orbit groupoid $\mathcal{O}$ category laws, the observer functor map, the functor theorem, trace equivalence, and the Extraction directives.
- [coalgebraic_bisimulation.v](file:///home/main/omi/omi-isa/proof/coalgebraic_bisimulation.v): Encodes the $F(X) = X \times O$ output functor, $F$-coalgebras, coalgebra morphisms, bisimulations, and the `trace_equivalence` / `morphism_trace_equivalence` theorems that bridge the coalgebraic and orbit semantics.
- [OMI_bialgebra.v](file:///home/main/omi/omi-isa/proof/OMI_bialgebra.v): Encodes the unified bialgebra structure: `Alg`, `CoAlg`, `Bialg` records, coinductive stream, `obs_stream` cofixpoint, `bialgebra_commutation`/`bialgebra_coherence`, concrete observer bialgebras, and extraction.

---

## 12. Extraction Boundary

Coq's extraction engine compiles the verified formalization into OCaml:
- `delta`, `step`, and `trace` functions are extracted.
- Observers and functorial proofs are verified at compilation.
- The abstract parameters $A$ and $B$ are bound to native OCaml implementations mimicking the C engine's linear dynamics.

---

## 13. Fundamental Claim

The system is not a program.

It is a finite linear dynamical system:

    Δ ∈ GL(16,2) ⋉ GF(2^16)

where $c$ is fixed per orbit, and all structures are quotient observers over its orbits.

Nothing exists outside Δ and its induced category of observers.
