# Proof Lineage and Podcast Claim Status

This document maps the podcast verification target onto the current proof files.
It is intentionally conservative: it separates runtime delta models, formal
projection results, and claims that still need bridge theorems.

## Proof Tracks

| Track | File | Status | Role |
| --- | --- | --- | --- |
| Historical rotate/XOR kernel | `proof/AtomicKernel.v` | Historical, currently excluded from `make proof` | First compact kernel attempt. It currently fails at `mask_idempotent` and is kept as provenance. |
| Current rotate/XOR kernel | `proof/AtomicKernelVNext.v` | Compiles | Deterministic replay kernel with masking, rotate/XOR delta, replay determinism, and replay length. This is the clean ancestor of the rotate/XOR delta used by the sense/pi line. |
| Pi/phi incidence projection | `proof/omi_pi_proof.v` | Compiles | Proves diagonal closure facts, Fano/tetra/incidence scaffolding, phi recurrence facts, and an incidence/projection series equal to Coq `PI`. |
| Kernel-to-pi bridge | `proof/omi_pi_bridge.v` | Compiles | Connects `AtomicKernelVNext.replay` to the pi incidence schedule by sharing the replay index as the orbit clock. This is a first bridge, not a proof that runtime state values determine pi phases. |

## Delta Split

There are two different 16-bit delta families in the repository:

- **GF(2^16) orbit delta:** `omi_orbit.c` implements `omi_delta16(x, c) = A[x] ^ B[c]`, where `A` is multiplication by alpha in GF(2^16) and `B` is identity.
- **Rotate/XOR delta:** `AtomicKernelVNext.v`, `omi_pi_proof.v`, and `omi_sense.c` use a rotate/XOR shape: left rotations, right rotation, and a constant/control XOR.

Do not collapse these into one claim. The current proof roadmap treats them as
separate until an explicit theorem relates a chosen runtime observer to the
incidence schedule.

## Podcast Claim Status

| Claim | Current status |
| --- | --- |
| The 16-bit runtime transition is deterministic. | Proven for `AtomicKernelVNext`; tested for the C orbit engine. |
| The incidence/projection schedule converges to real pi. | Proven in `omi_pi_proof.v` via `OMI_PI_from_incidence_equals_PI` and related convergence theorems. |
| Phi is captured by a recurrence/fixed-point layer. | Proven in `omi_pi_proof.v` as a classical phi witness and recurrence/fixed-point result. |
| Pi emerges from the C GF(2^16) orbit engine. | Not yet proven. Needs a theorem connecting `omi_orbit` traces or an observer over them to the incidence phase schedule. |
| Fano/Tetra/Phase/BQF are structural functorial observers of the GF(2^16) model. | Not currently proven. In the main GL(16,2) Coq layer they are analytic projections unless equipped with an `Observer` record and `equiv` proof. |
| DNA/theology/cosmology interpretations are formally verified. | Not proven by the current Coq files. Treat as narrative interpretation unless formal models are added. |

## Next Proof Milestone

Strengthen `proof/omi_pi_bridge.v` so a runtime observer determines the
`diagonal_accumulator_phase` schedule from actual replay or orbit states, rather
than using the replay index as a shared clock.
