(*
  OMI_Exports.v -- compatibility exports for the layered OMI proof stack.

  This file contains no new mathematics.  It exists as a stable import surface
  for downstream files that want the public theorem names from the split proof
  layers without depending on the old monolithic pi proof filename.
*)

Require Export DiagonalClosure.
Require Export FiniteIncidence.
Require Export BQFBridge.
Require Export MetricProjection.
Require Export PiProjection.
