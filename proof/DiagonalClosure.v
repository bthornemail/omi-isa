(*
  DiagonalClosure.v -- finite diagonal closure and phase schedule.

  This layer is still finite/discrete: nibbles, D+/D- closure, the 16-bit
  rotate/XOR scaffold, and the alternating diagonal accumulator used by later
  projection layers.
*)

From Coq Require Import Reals.Reals.
From Coq Require Import NArith.NArith.
From Coq Require Import Lists.List.
From Coq Require Import Arith.PeanoNat.
From Coq Require Import micromega.Lia.
Import ListNotations.
Open Scope N_scope.

Definition nibble (x : N) : Prop := x < 16.

Definition poly_xor4 (a b c d : N) : N :=
  N.lxor (N.lxor (N.lxor a b) c) d.

Definition dplus0 : N := 0.
Definition dplus1 : N := 5.
Definition dplus2 : N := 10.
Definition dplus3 : N := 15.

Definition dminus0 : N := 3.
Definition dminus1 : N := 6.
Definition dminus2 : N := 9.
Definition dminus3 : N := 12.

Theorem dplus_xor_zero :
  poly_xor4 dplus0 dplus1 dplus2 dplus3 = 0.
Proof. vm_compute. reflexivity. Qed.

Theorem dminus_xor_zero :
  poly_xor4 dminus0 dminus1 dminus2 dminus3 = 0.
Proof. vm_compute. reflexivity. Qed.

Theorem dplus_sum_1e :
  dplus0 + dplus1 + dplus2 + dplus3 = 30.
Proof. vm_compute. reflexivity. Qed.

Theorem dminus_sum_1e :
  dminus0 + dminus1 + dminus2 + dminus3 = 30.
Proof. vm_compute. reflexivity. Qed.

Record DiagonalClosure : Type := mkDiagonalClosure {
  closure_xor : N;
  closure_sum : N
}.

Definition dplus_closure : DiagonalClosure :=
  mkDiagonalClosure (poly_xor4 dplus0 dplus1 dplus2 dplus3)
                    (dplus0 + dplus1 + dplus2 + dplus3).

Definition dminus_closure : DiagonalClosure :=
  mkDiagonalClosure (poly_xor4 dminus0 dminus1 dminus2 dminus3)
                    (dminus0 + dminus1 + dminus2 + dminus3).

Theorem dplus_closure_valid :
  closure_xor dplus_closure = 0 /\ closure_sum dplus_closure = 30.
Proof. vm_compute. auto. Qed.

Theorem dminus_closure_valid :
  closure_xor dminus_closure = 0 /\ closure_sum dminus_closure = 30.
Proof. vm_compute. auto. Qed.

Theorem diag_sum_3c :
  dplus0 + dplus1 + dplus2 + dplus3 +
  (dminus0 + dminus1 + dminus2 + dminus3) = 60.
Proof. vm_compute. reflexivity. Qed.

Definition is_dplus (x : N) : bool :=
  (x =? dplus0) || (x =? dplus1) || (x =? dplus2) || (x =? dplus3).

Definition is_dminus (x : N) : bool :=
  (x =? dminus0) || (x =? dminus1) || (x =? dminus2) || (x =? dminus3).

Definition complement_sum : N :=
  fold_left
    (fun acc x => if orb (is_dplus x) (is_dminus x) then acc else acc + x)
    [0; 1; 2; 3; 4; 5; 6; 7; 8; 9; 10; 11; 12; 13; 14; 15]
    0.

Theorem complement_sum_3c : complement_sum = 60.
Proof. vm_compute. reflexivity. Qed.

Definition wheel_sum : N :=
  fold_left N.add [0; 1; 2; 3; 4; 5; 6; 7; 8; 9; 10; 11; 12; 13; 14; 15] 0.

Theorem full_wheel_sum_78 : wheel_sum = 120.
Proof. vm_compute. reflexivity. Qed.

Definition mask16 (x : N) : N := x mod 65536.

Definition rotl16 (x k : N) : N :=
  mask16 (N.shiftl (mask16 x) k + N.shiftr (mask16 x) (16 - k)).

Definition rotr16 (x k : N) : N :=
  mask16 (N.shiftr (mask16 x) k + N.shiftl (mask16 x) (16 - k)).

Definition delta16 (x c : N) : N :=
  mask16 (N.lxor (N.lxor (N.lxor (rotl16 x 1) (rotl16 x 3)) (rotr16 x 2)) c).

Theorem mask16_bound : forall x : N, mask16 x < 65536.
Proof.
  intro x.
  unfold mask16.
  apply N.mod_upper_bound.
  discriminate.
Qed.

Theorem delta16_width_preserving : forall x c : N, delta16 x c < 65536.
Proof.
  intros x c.
  unfold delta16.
  apply mask16_bound.
Qed.

Theorem delta16_deterministic : forall x c : N, delta16 x c = delta16 x c.
Proof. reflexivity. Qed.

Open Scope R_scope.

Inductive ChiralPhase : Type :=
| DPlusPhase
| DMinusPhase
| BalancedPhase
| IncompletePhase.

Definition phase_to_sign (p : ChiralPhase) : R :=
  match p with
  | DPlusPhase => 1
  | DMinusPhase => -1
  | BalancedPhase => 0
  | IncompletePhase => 0
  end.

Definition diagonal_phase_schedule : list ChiralPhase :=
  [DPlusPhase; DMinusPhase].

Definition polybius_phase_at (n : nat) : ChiralPhase :=
  nth (n mod 2)%nat diagonal_phase_schedule BalancedPhase.

Definition diagonal_race_phase (n : nat) : ChiralPhase :=
  match polybius_phase_at n with
  | DPlusPhase =>
      if andb (closure_xor dplus_closure =? 0) (closure_sum dplus_closure =? 30)
      then DPlusPhase else IncompletePhase
  | DMinusPhase =>
      if andb (closure_xor dminus_closure =? 0) (closure_sum dminus_closure =? 30)
      then DMinusPhase else IncompletePhase
  | BalancedPhase => BalancedPhase
  | IncompletePhase => IncompletePhase
  end.

Theorem diagonal_phase_schedule_even : forall n : nat,
  polybius_phase_at n = if Nat.even n then DPlusPhase else DMinusPhase.
Proof.
  intro n.
  unfold polybius_phase_at, diagonal_phase_schedule.
  destruct (Nat.even n) eqn:He.
  - destruct (n mod 2)%nat eqn:Hm.
    + reflexivity.
    + assert (n0 = 0%nat) by
        (assert (S n0 < 2)%nat by (rewrite <- Hm; apply Nat.mod_upper_bound; lia); lia).
      subst n0.
      exfalso.
      apply Nat.even_spec in He.
      destruct He as [k Hk].
      assert ((n mod 2) = 0)%nat.
        subst n.
        rewrite Nat.mul_comm.
        apply Nat.Div0.mod_mul.
      lia.
  - destruct (n mod 2)%nat eqn:Hm.
    + exfalso.
      assert (Nat.Even n).
        apply Nat.Lcm0.mod_divide in Hm.
        destruct Hm as [k Hk].
        exists k.
        rewrite Nat.mul_comm.
        exact Hk.
      apply Nat.even_spec in H.
      rewrite H in He.
      discriminate.
    + assert (n0 = 0%nat) by
        (assert (S n0 < 2)%nat by (rewrite <- Hm; apply Nat.mod_upper_bound; lia); lia).
      subst n0.
      reflexivity.
Qed.

Theorem polybius_diagonal_race_forces_phase_schedule : forall n : nat,
  diagonal_race_phase n = polybius_phase_at n.
Proof.
  intro n.
  unfold diagonal_race_phase.
  destruct (polybius_phase_at n); vm_compute; reflexivity.
Qed.

Record DiagonalAccumulator : Type := mkDiagonalAccumulator {
  accumulator_phase : ChiralPhase
}.

Definition diagonal_closure_ready (c : DiagonalClosure) : bool :=
  andb (N.eqb (closure_xor c) 0) (N.eqb (closure_sum c) 30).

Definition diagonal_accumulator_step (a : DiagonalAccumulator) : DiagonalAccumulator :=
  match accumulator_phase a with
  | DPlusPhase =>
      if diagonal_closure_ready dplus_closure
      then mkDiagonalAccumulator DMinusPhase
      else mkDiagonalAccumulator IncompletePhase
  | DMinusPhase =>
      if diagonal_closure_ready dminus_closure
      then mkDiagonalAccumulator DPlusPhase
      else mkDiagonalAccumulator IncompletePhase
  | BalancedPhase => mkDiagonalAccumulator BalancedPhase
  | IncompletePhase => mkDiagonalAccumulator IncompletePhase
  end.

Definition diagonal_accumulator_at (n : nat) : DiagonalAccumulator :=
  Nat.iter n diagonal_accumulator_step (mkDiagonalAccumulator DPlusPhase).

Definition diagonal_accumulator_phase (n : nat) : ChiralPhase :=
  accumulator_phase (diagonal_accumulator_at n).

Lemma nat_induction_by_two :
  forall P : nat -> Prop,
    P 0%nat ->
    P 1%nat ->
    (forall n : nat, P n -> P (S (S n))) ->
    forall n : nat, P n.
Proof.
  intros P H0 H1 Hstep n.
  assert (P n /\ P (S n)) as [H _].
  - induction n as [| n [IHn IHSn]].
    + split; assumption.
    + split; [exact IHSn | apply Hstep; exact IHn].
  - exact H.
Qed.

Theorem diagonal_accumulator_forces_phase_schedule : forall n : nat,
  diagonal_accumulator_phase n = polybius_phase_at n.
Proof.
  intro n.
  induction n using nat_induction_by_two.
  - vm_compute. reflexivity.
  - vm_compute. reflexivity.
  - unfold diagonal_accumulator_phase, diagonal_accumulator_at in *.
    change (Nat.iter (S (S n)) diagonal_accumulator_step (mkDiagonalAccumulator DPlusPhase))
      with (diagonal_accumulator_step
              (diagonal_accumulator_step
                (Nat.iter n diagonal_accumulator_step (mkDiagonalAccumulator DPlusPhase)))).
    destruct (Nat.iter n diagonal_accumulator_step (mkDiagonalAccumulator DPlusPhase)) as [p].
    simpl in IHn.
    rewrite IHn.
    rewrite diagonal_phase_schedule_even.
    rewrite diagonal_phase_schedule_even.
    rewrite Nat.even_succ_succ.
    destruct (Nat.even n); vm_compute; reflexivity.
Qed.

Theorem diagonal_accumulator_phase_matches_race : forall n : nat,
  diagonal_accumulator_phase n = diagonal_race_phase n.
Proof.
  intro n.
  rewrite diagonal_accumulator_forces_phase_schedule.
  rewrite polybius_diagonal_race_forces_phase_schedule.
  reflexivity.
Qed.

Theorem dplus_phase_sign : phase_to_sign DPlusPhase = 1.
Proof. reflexivity. Qed.

Theorem dminus_phase_sign : phase_to_sign DMinusPhase = -1.
Proof. reflexivity. Qed.
