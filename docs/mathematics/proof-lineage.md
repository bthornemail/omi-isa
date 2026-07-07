# Proof Lineage and Podcast Claim Status

This document maps the podcast verification target onto the current proof files
in the sibling `omi-axioms` repository.
It is intentionally conservative: it separates runtime delta models, formal
projection results, and claims that still need bridge theorems.

For the discovery narrative, start with [Computational Cosmology](../../01_COSMOLOGY.md).
For the supporting motivation note, see [vision.md](../cosmology/vision.md).
Those documents explain motivation.  This document tracks what the proof files
actually establish.

## Repository Roles

- Discovery explains why observer, place, relation, and replay are the chosen
  starting points.
- Mathematics states what is formally true.
- Implementation shows how the artifacts execute on C, WASM, and firmware.

The executable artifacts are the authority.  The mathematical proofs justify
the artifacts.  The documentation explains why those artifacts were chosen.
Algorithms are the source of truth; prose, diagrams, and philosophical
interpretations are projections of those algorithms.

## Volume Map

| Volume | Layer | Files | Role |
| --- | --- | --- | --- |
| I | Atomic Kernel | `AtomicKernel.v`, `AtomicKernelVNext.v` | Historical root and clean replay continuation. No pi, geometry, or runtime orbit claims live here. |
| II | Finite Structural Geometry | `DiagonalClosure.v`, `FiniteIncidence.v`, `BQFBridge.v` | Exact finite combinatorics: diagonals, incidence, selectors, and bridge schedules. |
| III | Projection | `MetricProjection.v`, `PiProjection.v` | Real-analysis boundary: sqrt3, phi, Leibniz/Alt_PI, and convergence claims. |
| IV | Dynamics | `delta_orbit_theory.v`, `functorial_semantics.v`, `coalgebraic_bisimulation.v`, `OMI_bialgebra.v`, `verified_execution.v` | Executable semantic stack and eventual refinement to runtime behavior. |
| V | Bridges and Exports | `omi_pi_bridge.v`, `OMI_Exports.v`, `omi_pi_proof.v` | Equivalences and compatibility surfaces. These files should not introduce new foundational mathematics. |

## Proof Stack

| Track | File | Status | Role |
| --- | --- | --- | --- |
| Atomic replay root | `../omi-axioms/coq/AtomicKernel.v` | Compiles | First compact kernel receipt: mask, rotate, delta, sequence determinism, and mask idempotence. This is the narrative root. |
| Clean replay continuation | `../omi-axioms/coq/AtomicKernelVNext.v` | Compiles | Keeps `AtomicKernel` as provenance and adds bounded replay structure, replay determinism, and replay length. |
| Diagonal closure | `../omi-axioms/coq/DiagonalClosure.v` | Compiles | D+/D- nibble closure, wheel sums, rotate/XOR scaffold, and the alternating diagonal phase accumulator. |
| Finite incidence | `../omi-axioms/coq/FiniteIncidence.v` | Compiles | Tetra/Fano/Schlafli finite incidence and rectified common core. No real-analysis limit claims live here. |
| BQF bridge | `../omi-axioms/coq/BQFBridge.v` | Compiles | BQF decomposition, local240/fano selectors, and bridge schedule fields. |
| Metric projection | `../omi-axioms/coq/MetricProjection.v` | Compiles | Projection boundary, sqrt3, phi fixed point, Fibonacci-ratio convergence. |
| Pi projection | `../omi-axioms/coq/PiProjection.v` | Compiles | Leibniz/Alt_PI bridge, incidence terms, convergence, and `OMI_PI = PI`. |
| Exports surface | `../omi-axioms/coq/OMI_Exports.v` | Compiles | Neutral re-export surface for the layered pi stack. Contains no new mathematics. |
| Legacy compatibility shim | `../omi-axioms/coq/omi_pi_proof.v` | Compiles | Re-exports `OMI_Exports` so old imports continue to work. New code should import `OMI_Exports` or a specific layer. |
| Kernel-to-pi bridge | `../omi-axioms/coq/omi_pi_bridge.v` | Compiles | Connects `AtomicKernelVNext.replay` to the pi incidence schedule by sharing the replay index as the orbit clock. This is a first bridge, not a proof that runtime state values determine pi phases. |
| GL(16,2) semantics | `../omi-axioms/coq/delta_orbit_theory.v` and dependents | Compiles | Separate executable semantic stack: orbit theory, functorial semantics, coalgebra, bialgebra, and execution refinement. |

## Delta Split

There are two different 16-bit delta families in the repository:

- **GF(2^16) orbit delta:** `lib/omi_orbit.c` implements `omi_delta16(x, c) = A[x] ^ B[c]`, where `A` is multiplication by alpha in GF(2^16) and `B` is identity.
- **Rotate/XOR delta:** `AtomicKernel.v`, `AtomicKernelVNext.v`, `DiagonalClosure.v`, and `lib/omi_sense.c` use a rotate/XOR shape: left rotations, right rotation, and a constant/control XOR.

Do not collapse these into one claim. The current proof roadmap treats them as
separate until an explicit theorem relates a chosen runtime observer to the
incidence schedule.

## Podcast Claim Status

| Claim | Proof | Artifact |
| --- | --- | --- |
| The rotate/XOR replay kernel is deterministic. | Proven first in `AtomicKernel.v`, extended in `AtomicKernelVNext.v`. | `../omi-axioms/coq/AtomicKernel.v`, `../omi-axioms/coq/AtomicKernelVNext.v` |
| The C GF(2^16) orbit transition is deterministic. | Tested for the C orbit engine; semantic stack compiles separately. | `lib/omi_orbit.c`, `test/test_orbit.c`, `../omi-axioms/coq/delta_orbit_theory.v` |
| The incidence/projection schedule converges to real pi. | Proven in `PiProjection.v` via `OMI_PI_from_incidence_equals_PI` and related convergence theorems. | `../omi-axioms/coq/PiProjection.v` |
| Phi is captured by a recurrence/fixed-point layer. | Proven in `MetricProjection.v` as a classical phi witness and recurrence/fixed-point result. | `../omi-axioms/coq/MetricProjection.v` |
| Pi emerges from the C GF(2^16) orbit engine. | Not yet proven. Needs a theorem connecting `omi_orbit` traces or an observer over them to the incidence phase schedule. | Future bridge over `lib/omi_orbit.c` traces or `lib/omi_sense.c` observations. |
| Fano/Tetra/Phase/BQF are structural functorial observers of the GF(2^16) model. | Not currently proven as functorial observers. In the main GL(16,2) Coq layer they are analytic projections unless equipped with an `Observer` record and `equiv` proof. | `lib/omi_orbit.c`, `../omi-axioms/coq/delta_orbit_theory.v` |
| DNA/theology/cosmology interpretations are formally verified. | Not proven by the current Coq files. Treat as narrative interpretation unless formal models are added. | `01_COSMOLOGY.md`, `docs/cosmology/vision.md` |

## Next Proof Milestone

Strengthen `../omi-axioms/coq/omi_pi_bridge.v` so a runtime observer determines the
`diagonal_accumulator_phase` schedule from actual replay or orbit states, rather
than using the replay index as a shared clock.
