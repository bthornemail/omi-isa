(*
  omi_pi_bridge.v -- bridge between the rotate/XOR kernel lineage and the
  incidence-to-pi projection layer.

  This file intentionally does not claim that the GF(2^16) orbit engine in
  omi_orbit.c emits pi.  It connects the compiling rotate/XOR replay kernel
  (AtomicKernelVNext) to the incidence/projection schedule in omi_pi_proof by
  using the replay index as the shared orbit clock.
*)

From Coq Require Import Lists.List.
From Coq Require Import NArith.NArith.
From Coq Require Import Reals.Reals.
Import ListNotations.

Require AtomicKernelVNext.
Require omi_pi_proof.

Open Scope R_scope.

Record KernelPiSample : Type := mkKernelPiSample {
  sample_width : N;
  sample_seed : N;
  sample_index : nat;
  sample_state : N;
  sample_phase : omi_pi_proof.ChiralPhase;
  sample_term : R
}.

Definition replay_state_at
    (width seed : N) (index : nat)
    : N :=
  nth index
    (AtomicKernelVNext.replay width seed (S index))
    (AtomicKernelVNext.mask width seed).

Definition kernel_pi_sample
    (width seed : N) (index : nat)
    : KernelPiSample :=
  mkKernelPiSample
    width
    seed
    index
    (replay_state_at width seed index)
    (omi_pi_proof.diagonal_accumulator_phase index)
    (omi_pi_proof.omi_pi_term_from_diagonal_accumulator index).

Theorem kernel_pi_sample_phase_is_accumulator :
  forall width seed index,
    sample_phase (kernel_pi_sample width seed index) =
    omi_pi_proof.diagonal_accumulator_phase index.
Proof. reflexivity. Qed.

Theorem kernel_pi_sample_term_matches_incidence :
  forall width seed index,
    sample_term (kernel_pi_sample width seed index) =
    omi_pi_proof.omi_pi_term_from_incidence index.
Proof.
  intros width seed index.
  unfold kernel_pi_sample.
  simpl.
  rewrite omi_pi_proof.omi_pi_term_from_diagonal_accumulator_matches_race.
  rewrite omi_pi_proof.omi_pi_term_from_diagonal_race_matches_polybius.
  apply omi_pi_proof.omi_pi_term_from_polybius_matches_incidence.
Qed.

Theorem kernel_pi_projection_series_converges :
  forall width seed,
    Un_cv
      (fun n : nat =>
         sum_f_R0
           (fun k : nat => sample_term (kernel_pi_sample width seed k))
           n)
      (omi_pi_proof.OMI_PI / 4).
Proof.
  intros width seed.
  eapply Un_cv_ext with
    (un := fun n : nat =>
       sum_f_R0 omi_pi_proof.omi_pi_term_from_incidence n).
  - intro n.
    apply sum_eq.
    intros k _.
    symmetry.
    apply kernel_pi_sample_term_matches_incidence.
  - apply omi_pi_proof.omi_pi_incidence_projection_series_converges.
Qed.

Theorem kernel_pi_projection_equals_real_pi :
  forall width seed,
    4 *
      proj1_sig
        (exist
          (fun l : R =>
             Un_cv
               (fun n : nat =>
                  sum_f_R0
                    (fun k : nat => sample_term (kernel_pi_sample width seed k))
                    n)
               l)
          (omi_pi_proof.OMI_PI / 4)
          (kernel_pi_projection_series_converges width seed))
    = PI.
Proof.
  intros width seed.
  simpl.
  rewrite <- omi_pi_proof.OMI_PI_Equals_Real_PI.
  field.
Qed.

(*
  Bridge boundary:
  - The replay state is carried so the rotate/XOR trace has a concrete runtime
    clock and state.
  - The pi projection is computed from the diagonal/incidence schedule at that
    same clock index.
  - A future stronger bridge should prove that a chosen runtime state observer
    determines the phase schedule, rather than indexing both by the same n.
*)
