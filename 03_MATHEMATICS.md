# 03. Mathematics

This layer answers what structures emerge after the atomic kernel is fixed.

The mathematical stack should be read as a progression:

```text
Finite Sets
  -> Finite Geometry
  -> Incidence
  -> Projection
  -> Group Actions
  -> Orbit Theory
  -> Categories
  -> Coalgebras
  -> Bialgebras
  -> Verification
```

## Finite Structural Geometry

The finite layer contains exact combinatorics only:

- `proof/DiagonalClosure.v`: D+/D- nibble closure, wheel sums, and the
  alternating diagonal accumulator.
- `proof/FiniteIncidence.v`: tetrahedral and Fano incidence.
- `proof/BQFBridge.v`: BQF decomposition, selectors, local240, and bridge
  schedules.

No real-analysis claim belongs in this layer.

## Projection

Metric constants appear only at projection boundaries:

- `proof/MetricProjection.v`: sqrt3, phi, fixed points, and recurrence limits.
- `proof/PiProjection.v`: Leibniz/Alt_PI terms, convergence, and
  `OMI_PI = PI`.

This proves the projection schedules, not that the C GF(2^16) orbit engine
itself emits pi.

## Dynamics And Semantics

The executable semantic stack is separate from the rotate/XOR projection stack:

- `proof/delta_orbit_theory.v`: GL(16,2) dynamics and observer definitions.
- `proof/functorial_semantics.v`: orbit groupoid and observer functor.
- `proof/coalgebraic_bisimulation.v`: stream observation and bisimulation.
- `proof/OMI_bialgebra.v`: algebra/coalgebra coherence.
- `proof/verified_execution.v`: intended C-to-Coq refinement layer.

## Bridges

Bridge files should state equivalences or compatibility surfaces, not introduce
new foundational mathematics:

- `proof/omi_pi_bridge.v`: connects replay index to the pi incidence schedule.
- `proof/OMI_Exports.v`: neutral export surface for split proof layers.
- `proof/omi_pi_proof.v`: legacy compatibility shim.

Detailed status: [docs/mathematics/proof-lineage.md](docs/mathematics/proof-lineage.md).
