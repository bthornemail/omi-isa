(*
  BQFBridge.v -- BQF decomposition and finite bridge selectors.

  This layer connects finite incidence coordinates to the later projection
  schedule without introducing real-analysis limits.
*)

From Coq Require Import NArith.NArith.
From Coq Require Import Arith.PeanoNat.
From Coq Require Import micromega.Lia.
Open Scope N_scope.

Require Import DiagonalClosure.
Require Import FiniteIncidence.

Definition bqf_high_shell (x : N) : N := 60 * x * x.

Definition bqf_chiral_bridge (x y : N) : N := 16 * x * y.

Definition bqf_local_seed (y : N) : N := 4 * y * y.

Definition bqf (x y : N) : N :=
  bqf_high_shell x + bqf_chiral_bridge x y + bqf_local_seed y.

Theorem bqf_decompose : forall x y : N,
  bqf x y = 4 * (15 * x * x + 4 * x * y + y * y).
Proof.
  intros x y.
  unfold bqf, bqf_high_shell, bqf_chiral_bridge, bqf_local_seed.
  nia.
Qed.

Theorem bqf_chiral_bridge_is_16xy : forall x y : N,
  bqf_chiral_bridge x y = 16 * x * y.
Proof. reflexivity. Qed.

Theorem bqf_layer_sum : forall x y : N,
  bqf x y = bqf_high_shell x + bqf_chiral_bridge x y + bqf_local_seed y.
Proof. reflexivity. Qed.


Definition five_factorial_resolution : N := 120%N.

Definition local240_resolution : N := (2 * five_factorial_resolution)%N.

Theorem local240_is_two_5factorial :
  local240_resolution = 240%N.
Proof. vm_compute. reflexivity. Qed.

Definition fano_selector (n : nat) : N := N.of_nat (n mod 7)%nat.

Definition local240_selector (n : nat) : N := N.of_nat (n mod 240)%nat.

Definition projection_denominator_index (n : nat) : N := N.of_nat (2 * n + 1).

Record BQFBridge : Type := mkBQFBridge {
  bridge_orbit : nat;
  bridge_fano7 : N;
  bridge_local240 : N;
  bridge_cross : N
}.

Definition bridge_phase (b : BQFBridge) : ChiralPhase :=
  diagonal_accumulator_phase (bridge_orbit b).

Definition bridge_denominator (b : BQFBridge) : N :=
  projection_denominator_index (bridge_orbit b).

Definition bqf_bridge_at (n : nat) : BQFBridge :=
  let x := (fano_selector n + 1)%N in
  let y := (local240_selector n + 1)%N in
  mkBQFBridge
    n
    (fano_selector n)
    (local240_selector n)
    (bqf_chiral_bridge x y).

Theorem fano_selector_bound : forall n : nat,
  (fano_selector n < 7)%N.
Proof.
  intro n.
  unfold fano_selector.
  apply N.compare_lt_iff.
  change ((N.of_nat (n mod 7) ?= N.of_nat 7)%N = Lt).
  rewrite <- Nat2N.inj_compare.
  apply Nat.compare_lt_iff.
  apply Nat.mod_upper_bound.
  lia.
Qed.

Theorem local240_selector_bound : forall n : nat,
  (local240_selector n < local240_resolution)%N.
Proof.
  intro n.
  rewrite local240_is_two_5factorial.
  unfold local240_selector.
  apply N.compare_lt_iff.
  change ((N.of_nat (n mod 240) ?= N.of_nat 240)%N = Lt).
  rewrite <- Nat2N.inj_compare.
  apply Nat.compare_lt_iff.
  apply Nat.mod_upper_bound.
  lia.
Qed.

Theorem bqf_bridge_phase_matches_accumulator : forall n : nat,
  bridge_phase (bqf_bridge_at n) = diagonal_accumulator_phase n.
Proof. reflexivity. Qed.

Theorem bqf_bridge_phase_computed_from_orbit : forall b : BQFBridge,
  bridge_phase b = diagonal_accumulator_phase (bridge_orbit b).
Proof. reflexivity. Qed.

Theorem bqf_bridge_denominator_computed_from_orbit : forall b : BQFBridge,
  bridge_denominator b = projection_denominator_index (bridge_orbit b).
Proof. reflexivity. Qed.

Theorem bqf_bridge_cross_is_16xy : forall n : nat,
  bridge_cross (bqf_bridge_at n) =
    (16 * (fano_selector n + 1) * (local240_selector n + 1))%N.
Proof. reflexivity. Qed.

