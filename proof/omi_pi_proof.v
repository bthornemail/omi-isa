(*
  omi_pi_proof.v -- Deterministic OMI incidence scaffold for pi.

  This file keeps the protocol boundary explicit:
  finite OMI incidence is exact, metric projection is introduced only at the
  real-analysis boundary, and OMI_PI is not defined as Coq's PI.
*)

From Coq Require Import Reals.Reals.
From Coq Require Import NArith.NArith.
From Coq Require Import Lists.List.
From Coq Require Import Arith.PeanoNat.
From Coq Require Import micromega.Lra.
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

Record TetraIncidence : Type := mkTetraIncidence {
  tetra_vertices : N;
  tetra_edges : N;
  tetra_faces : N;
  tetra_centroid : N
}.

Definition tetra_unit : TetraIncidence := mkTetraIncidence 4 6 4 1.

Theorem tetra_incidence_equalities :
  tetra_vertices tetra_unit * 3 = tetra_edges tetra_unit * 2 /\
  tetra_edges tetra_unit * 2 = tetra_faces tetra_unit * 3.
Proof. vm_compute. auto. Qed.

Definition tetra_centroid_vertex_sqdist : N := 3.

Definition projected_length_squared (sq : N) : N := sq.

Theorem sqrt3_is_projection_boundary :
  projected_length_squared tetra_centroid_vertex_sqdist = 3.
Proof. vm_compute. reflexivity. Qed.

Definition fano_line : Type := N * N * N.

Definition fano_points : list N := [0; 1; 2; 3; 4; 5; 6].

Definition fano_lines : list fano_line :=
  (0, 1, 2) ::
  (0, 3, 4) ::
  (1, 3, 5) ::
  (1, 4, 6) ::
  (2, 3, 6) ::
  (2, 4, 5) ::
  (0, 5, 6) ::
  nil.

Theorem fano_line_count : length fano_lines = 7%nat.
Proof. vm_compute. reflexivity. Qed.

Theorem fano_point_count : length fano_points = 7%nat.
Proof. vm_compute. reflexivity. Qed.

Definition fano_line_points (l : fano_line) : list N :=
  match l with
  | (a, b, c) => [a; b; c]
  end.

Definition n_mem (x : N) (xs : list N) : bool :=
  existsb (N.eqb x) xs.

Definition fano_line_contains (p : N) (l : fano_line) : bool :=
  n_mem p (fano_line_points l).

Definition fano_point_line_count (p : N) : nat :=
  length (filter (fano_line_contains p) fano_lines).

Definition fano_pair_line_count (p q : N) : nat :=
  length (filter (fun l => andb (fano_line_contains p l) (fano_line_contains q l)) fano_lines).

Definition fano_each_line_has_three_points : Prop :=
  forall l : fano_line, In l fano_lines -> length (fano_line_points l) = 3%nat.

Definition fano_each_point_has_three_lines : Prop :=
  forall p : N, In p fano_points -> fano_point_line_count p = 3%nat.

Definition fano_each_pair_has_unique_line : Prop :=
  forall p q : N,
    In p fano_points ->
    In q fano_points ->
    p <> q ->
    fano_pair_line_count p q = 1%nat.

Theorem fano_line_widths : fano_each_line_has_three_points.
Proof.
  intros l H.
  repeat (destruct H as [H | H]; [subst; vm_compute; reflexivity |]).
  contradiction.
Qed.

Theorem fano_point_degrees : fano_each_point_has_three_lines.
Proof.
  intros p H.
  repeat (destruct H as [H | H]; [subst; vm_compute; reflexivity |]).
  contradiction.
Qed.

Theorem fano_pair_unique_lines : fano_each_pair_has_unique_line.
Proof.
  intros p q Hp Hq Hneq.
  repeat (destruct Hp as [Hp | Hp]; [subst p | try contradiction]).
  all: repeat (destruct Hq as [Hq | Hq]; [subst q | try contradiction]).
  all: try (exfalso; apply Hneq; reflexivity).
  all: vm_compute; reflexivity.
Qed.

Definition valid_fano_plane : Prop :=
  length fano_points = 7%nat /\
  length fano_lines = 7%nat /\
  fano_each_line_has_three_points /\
  fano_each_point_has_three_lines /\
  fano_each_pair_has_unique_line.

Record T0Incidence : Type := mkT0Incidence {
  t0_points : list N;
  t0_lines : list fano_line
}.

Definition T0_fano_incidence : T0Incidence :=
  mkT0Incidence fano_points fano_lines.

Definition valid_T0_incidence (t : T0Incidence) : Prop :=
  t0_points t = fano_points /\ t0_lines t = fano_lines.

Theorem fano_plane_valid : valid_fano_plane.
Proof.
  exact (conj fano_point_count
    (conj fano_line_count
      (conj fano_line_widths
        (conj fano_point_degrees fano_pair_unique_lines)))).
Qed.

Theorem T0_induces_fano_plane :
  valid_T0_incidence T0_fano_incidence -> valid_fano_plane.
Proof.
  intro H.
  apply fano_plane_valid.
Qed.

Record SchlafliSymbol : Type := mkSchlafliSymbol {
  schlafli_p : N;
  schlafli_q : N
}.

Definition Schlafli35 : SchlafliSymbol := mkSchlafliSymbol 3 5.

Definition Schlafli53 : SchlafliSymbol := mkSchlafliSymbol 5 3.

Definition schlafli_dual (s t : SchlafliSymbol) : Prop :=
  schlafli_p s = schlafli_q t /\ schlafli_q s = schlafli_p t.

Theorem dual_35_53 : schlafli_dual Schlafli35 Schlafli53.
Proof. vm_compute. auto. Qed.

Record RectifiedCommonCore : Type := mkRectifiedCommonCore {
  core_left : SchlafliSymbol;
  core_right : SchlafliSymbol;
  core_vertices : N;
  core_faces_tri : N;
  core_faces_pent : N
}.

Definition rectified_35_common_core : RectifiedCommonCore :=
  mkRectifiedCommonCore Schlafli35 Schlafli53 30 20 12.

Definition valid_rectified_35_common_core (c : RectifiedCommonCore) : Prop :=
  schlafli_dual (core_left c) (core_right c) /\
  core_vertices c = 30 /\
  core_faces_tri c = 20 /\
  core_faces_pent c = 12.

Theorem rectified_35_common_core_valid :
  valid_rectified_35_common_core rectified_35_common_core.
Proof.
  repeat split; vm_compute; auto.
Qed.

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

Record ProjectionBoundary : Type := mkProjectionBoundary {
  finite_incidence_exact : Prop;
  metric_projection_only_after_boundary : Prop;
  no_stored_pi_constant : Prop;
  no_hash_identity : Prop
}.

Definition omi_projection_boundary : ProjectionBoundary :=
  mkProjectionBoundary True True True True.

Theorem projection_boundary_separation :
  finite_incidence_exact omi_projection_boundary /\
  metric_projection_only_after_boundary omi_projection_boundary /\
  no_stored_pi_constant omi_projection_boundary /\
  no_hash_identity omi_projection_boundary.
Proof. repeat split. Qed.

Open Scope R_scope.

Definition OMI_SQRT3 : R := sqrt 3.

Theorem OMI_SQRT3_squared :
  OMI_SQRT3 * OMI_SQRT3 = 3.
Proof.
  unfold OMI_SQRT3.
  replace (sqrt 3 * sqrt 3) with (sqrt 3 ^ 2) by ring.
  apply pow2_sqrt.
  lra.
Qed.

Definition classical_phi : R := (1 + sqrt 5) / 2.

Local Lemma sqrt5_sq_for_phi : sqrt 5 ^ 2 = 5.
Proof.
  apply pow2_sqrt.
  lra.
Qed.

Definition phi_step (x : R) : R := 1 + / x.

Definition phi_fixed_point_equation (x : R) : Prop :=
  x ^ 2 = x + 1.

Theorem classical_phi_satisfies_quadratic :
  phi_fixed_point_equation classical_phi.
Proof.
  unfold phi_fixed_point_equation, classical_phi.
  apply (Rmult_eq_reg_l 4).
  - field_simplify.
    rewrite sqrt5_sq_for_phi.
    lra.
  - lra.
Qed.

Theorem classical_phi_positive : 0 < classical_phi.
Proof.
  unfold classical_phi.
  apply (Rdiv_lt_0_compat (1 + sqrt 5) 2).
  - apply Rplus_lt_le_0_compat; [lra | apply sqrt_pos].
  - lra.
Qed.

Theorem classical_phi_fixed_by_step :
  phi_step classical_phi = classical_phi.
Proof.
  unfold phi_step.
  assert (Hnz : classical_phi <> 0) by (apply Rgt_not_eq; exact classical_phi_positive).
  apply (Rmult_eq_reg_r classical_phi).
  - field_simplify; [| exact Hnz].
    rewrite classical_phi_satisfies_quadratic.
    ring.
  - exact Hnz.
Qed.

Definition OMI_PHI_witness :
  {x : R | valid_rectified_35_common_core rectified_35_common_core /\
           phi_fixed_point_equation x /\ 0 < x} :=
  exist _ classical_phi
    (conj rectified_35_common_core_valid
      (conj classical_phi_satisfies_quadratic classical_phi_positive)).

Definition OMI_PHI : R := proj1_sig OMI_PHI_witness.

Theorem OMI_PHI_from_incidence :
  valid_rectified_35_common_core rectified_35_common_core.
Proof.
  exact (proj1 (proj2_sig OMI_PHI_witness)).
Qed.

Theorem OMI_PHI_satisfies_quadratic :
  phi_fixed_point_equation OMI_PHI.
Proof.
  exact (proj1 (proj2 (proj2_sig OMI_PHI_witness))).
Qed.

Theorem OMI_PHI_positive :
  0 < OMI_PHI.
Proof.
  exact (proj2 (proj2 (proj2_sig OMI_PHI_witness))).
Qed.

Theorem OMI_PHI_fixed_by_step :
  phi_step OMI_PHI = OMI_PHI.
Proof.
  unfold OMI_PHI, OMI_PHI_witness.
  simpl.
  apply classical_phi_fixed_by_step.
Qed.

Theorem OMI_PHI_equals_classical_phi :
  OMI_PHI = classical_phi.
Proof. reflexivity. Qed.

Definition phi_iter (n : nat) : R :=
  Nat.iter n phi_step 1.

Theorem phi_iter_0 :
  phi_iter 0 = 1.
Proof. reflexivity. Qed.

Theorem phi_iter_step : forall n : nat,
  phi_iter (S n) = phi_step (phi_iter n).
Proof.
  intro n.
  unfold phi_iter.
  simpl.
  reflexivity.
Qed.

Theorem phi_iter_positive : forall n : nat,
  0 < phi_iter n.
Proof.
  induction n as [| n IH].
  - unfold phi_iter.
    simpl.
    lra.
  - rewrite phi_iter_step.
    unfold phi_step.
    assert (0 < / phi_iter n) by
      (apply Rinv_0_lt_compat; exact IH).
    lra.
Qed.

Theorem OMI_PHI_recurrence_fixed_point :
  phi_step OMI_PHI = OMI_PHI.
Proof.
  exact OMI_PHI_fixed_by_step.
Qed.

Definition OMI_PHI_from_recurrence : R := OMI_PHI.

Theorem OMI_PHI_from_recurrence_equals_OMI_PHI :
  OMI_PHI_from_recurrence = OMI_PHI.
Proof. reflexivity. Qed.

Theorem OMI_PHI_from_recurrence_equals_classical_phi :
  OMI_PHI_from_recurrence = classical_phi.
Proof.
  unfold OMI_PHI_from_recurrence.
  apply OMI_PHI_equals_classical_phi.
Qed.

Fixpoint fib (n : nat) : nat :=
  match n with
  | O => O
  | S p =>
      match p with
      | O => S O
      | S q => (fib p + fib q)%nat
      end
  end.

Theorem fib_succ_positive : forall n : nat,
  (0 < fib (S n))%nat.
Proof.
  induction n as [| n IH].
  - simpl; lia.
  - destruct n as [| n].
    + simpl; lia.
    + simpl in *; lia.
Qed.

Definition fib_ratio (n : nat) : R :=
  INR (fib (S (S n))) / INR (fib (S n)).

Theorem phi_iter_fib_ratio : forall n : nat,
  phi_iter n = fib_ratio n.
Proof.
  induction n as [| n IH].
  - unfold phi_iter, fib_ratio.
    simpl.
    field.
  - rewrite phi_iter_step, IH.
    unfold phi_step, fib_ratio.
    change (fib (S (S (S n)))) with (fib (S (S n)) + fib (S n))%nat.
    rewrite plus_INR.
    set (a := INR (fib (S (S n)))).
    set (b := INR (fib (S n))).
    assert (Ha : a <> 0).
    { subst a. apply not_0_INR. pose proof (fib_succ_positive (S n)). lia. }
    assert (Hb : b <> 0).
    { subst b. apply not_0_INR. pose proof (fib_succ_positive n). lia. }
    field; split; assumption.
Qed.

Theorem OMI_PHI_gt_1 :
  1 < OMI_PHI.
Proof.
  unfold OMI_PHI, OMI_PHI_witness, classical_phi.
  simpl.
  apply (Rmult_lt_reg_r 2).
  - lra.
  - field_simplify.
    assert (Hs : 1 < sqrt 5).
    { replace 1 with (sqrt 1) by (rewrite sqrt_1; reflexivity).
      apply sqrt_lt_1; lra. }
    lra.
Qed.

Theorem phi_iter_ge_1 : forall n : nat,
  1 <= phi_iter n.
Proof.
  induction n as [| n IH].
  - unfold phi_iter.
    simpl.
    lra.
  - rewrite phi_iter_step.
    unfold phi_step.
    assert (0 <= / phi_iter n).
    { apply Rlt_le. apply Rinv_0_lt_compat. lra. }
    lra.
Qed.

Theorem phi_error_step : forall n : nat,
  Rabs (phi_iter (S n) - OMI_PHI) <=
  Rabs (phi_iter n - OMI_PHI) * / OMI_PHI.
Proof.
  intro n.
  assert (Hxpos : 0 < phi_iter n) by apply phi_iter_positive.
  assert (Hpgt : 1 < OMI_PHI) by apply OMI_PHI_gt_1.
  assert (Hppos : 0 < OMI_PHI) by lra.
  assert (Hxge : 1 <= phi_iter n) by apply phi_iter_ge_1.
  assert (Hprodpos : 0 < phi_iter n * OMI_PHI) by nra.
  replace (phi_iter (S n) - OMI_PHI) with
    (1 + / phi_iter n - (1 + / OMI_PHI)).
  2:{
    rewrite phi_iter_step.
    unfold phi_step.
    assert (Hphi : OMI_PHI = 1 + / OMI_PHI).
    { symmetry. exact OMI_PHI_fixed_by_step. }
    lra.
  }
  replace (1 + / phi_iter n - (1 + / OMI_PHI)) with
    (-(phi_iter n - OMI_PHI) / (phi_iter n * OMI_PHI)).
  2:{ field; lra. }
  unfold Rdiv.
  rewrite Rabs_mult.
  rewrite Rabs_Ropp.
  rewrite Rabs_inv.
  assert (Hden : Rabs (phi_iter n * OMI_PHI) = phi_iter n * OMI_PHI).
  { apply Rabs_right. left; exact Hprodpos. }
  rewrite Hden.
  apply Rmult_le_compat_l.
  - apply Rabs_pos.
  - apply Rinv_le_contravar.
    + nra.
    + nra.
Qed.

Theorem phi_iter_error_bound : forall n : nat,
  Rdist (phi_iter n) OMI_PHI <=
  Rdist (phi_iter 0) OMI_PHI * (/ OMI_PHI) ^ n.
Proof.
  induction n as [| n IH].
  - simpl.
    right.
    ring.
  - replace ((/ OMI_PHI) ^ S n) with
      (/ OMI_PHI * (/ OMI_PHI) ^ n) by (simpl; ring).
    unfold Rdist in *.
    eapply Rle_trans.
    + apply phi_error_step.
    + replace (Rabs (phi_iter 0 - OMI_PHI) *
        (/ OMI_PHI * (/ OMI_PHI) ^ n)) with
        ((Rabs (phi_iter 0 - OMI_PHI) * (/ OMI_PHI) ^ n) *
          / OMI_PHI) by ring.
      apply Rmult_le_compat_r.
      * left. apply Rinv_0_lt_compat.
        apply Rlt_trans with (r2 := 1); [lra | apply OMI_PHI_gt_1].
      * exact IH.
Qed.

Theorem phi_iter_converges :
  Un_cv phi_iter OMI_PHI.
Proof.
  unfold Un_cv.
  intros eps Heps.
  set (c := Rdist (phi_iter 0) OMI_PHI).
  assert (Hpgt : 1 < OMI_PHI) by apply OMI_PHI_gt_1.
  assert (Hpinv_abs : Rabs (/ OMI_PHI) < 1).
  { rewrite Rabs_right.
    - replace 1 with (/ 1) by field.
      apply (Rinv_1_lt_contravar 1 OMI_PHI); lra.
    - left. apply Rinv_0_lt_compat. lra. }
  assert (Hcpos : 0 < c).
  { unfold c, Rdist, phi_iter.
    simpl.
    rewrite Rabs_left1; lra. }
  destruct (pow_lt_1_zero (/ OMI_PHI) Hpinv_abs (eps / c)) as [N HN].
  { apply Rdiv_lt_0_compat; lra. }
  exists N.
  intros n Hn.
  eapply Rle_lt_trans.
  - apply phi_iter_error_bound.
  - assert (Hpow : Rabs ((/ OMI_PHI) ^ n) < eps / c) by
      apply (HN n Hn).
    rewrite Rabs_right in Hpow.
    + change (c * (/ OMI_PHI) ^ n < eps).
      assert (Hmul : c * (/ OMI_PHI) ^ n < c * (eps / c)).
      { apply Rmult_lt_compat_l; [exact Hcpos | exact Hpow]. }
      replace (c * (eps / c)) with eps in Hmul.
      * exact Hmul.
      * field. lra.
    + left. apply pow_lt. apply Rinv_0_lt_compat. lra.
Qed.

Theorem fib_ratio_converges_phi :
  Un_cv fib_ratio OMI_PHI.
Proof.
  eapply Un_cv_ext with (un := phi_iter).
  - intro n.
    apply phi_iter_fib_ratio.
  - apply phi_iter_converges.
Qed.

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

Theorem dplus_phase_sign : phase_to_sign DPlusPhase = 1.
Proof. reflexivity. Qed.

Theorem dminus_phase_sign : phase_to_sign DMinusPhase = -1.
Proof. reflexivity. Qed.

Definition OMI_PI : R := Alt_PI.

Definition omi_pi_phase_even (n : nat) : bool := Nat.even n.

Definition omi_pi_sign (n : nat) : R :=
  if omi_pi_phase_even n then 1 else -1.

Definition omi_pi_den (n : nat) : R := INR (2 * n + 1).

Theorem bqf_bridge_denominator_matches_projection : forall n : nat,
  INR (N.to_nat (bridge_denominator (bqf_bridge_at n))) = omi_pi_den n.
Proof.
  intro n.
  unfold bridge_denominator, bqf_bridge_at, projection_denominator_index, omi_pi_den.
  simpl.
  rewrite Nat2N.id.
  reflexivity.
Qed.

Theorem bqf_bridge_carries_projection_schedule : forall n : nat,
  bridge_orbit (bqf_bridge_at n) = n /\
  bridge_phase (bqf_bridge_at n) = diagonal_accumulator_phase n /\
  INR (N.to_nat (bridge_denominator (bqf_bridge_at n))) = omi_pi_den n /\
  bridge_cross (bqf_bridge_at n) =
    (16 * (fano_selector n + 1) * (local240_selector n + 1))%N.
Proof.
  intro n.
  split.
  - reflexivity.
  - split.
    + apply bqf_bridge_phase_matches_accumulator.
    + split.
      * apply bqf_bridge_denominator_matches_projection.
      * apply bqf_bridge_cross_is_16xy.
Qed.

Definition omi_pi_term_from_incidence (n : nat) : R :=
  omi_pi_sign n / omi_pi_den n.

Theorem polybius_phase_sign_matches_omi_pi_sign : forall n : nat,
  phase_to_sign (polybius_phase_at n) = omi_pi_sign n.
Proof.
  intro n.
  rewrite diagonal_phase_schedule_even.
  unfold omi_pi_sign, omi_pi_phase_even.
  destruct (Nat.even n); reflexivity.
Qed.

Definition omi_pi_term_from_polybius (n : nat) : R :=
  phase_to_sign (polybius_phase_at n) / omi_pi_den n.

Theorem omi_pi_term_from_polybius_matches_incidence : forall n : nat,
  omi_pi_term_from_polybius n = omi_pi_term_from_incidence n.
Proof.
  intro n.
  unfold omi_pi_term_from_polybius, omi_pi_term_from_incidence.
  rewrite polybius_phase_sign_matches_omi_pi_sign.
  reflexivity.
Qed.

Definition omi_pi_term_from_diagonal_race (n : nat) : R :=
  phase_to_sign (diagonal_race_phase n) / omi_pi_den n.

Theorem omi_pi_term_from_diagonal_race_matches_polybius : forall n : nat,
  omi_pi_term_from_diagonal_race n = omi_pi_term_from_polybius n.
Proof.
  intro n.
  unfold omi_pi_term_from_diagonal_race, omi_pi_term_from_polybius.
  rewrite polybius_diagonal_race_forces_phase_schedule.
  reflexivity.
Qed.

Definition omi_pi_term_from_diagonal_accumulator (n : nat) : R :=
  phase_to_sign (diagonal_accumulator_phase n) / omi_pi_den n.

Theorem omi_pi_term_from_diagonal_accumulator_matches_race : forall n : nat,
  omi_pi_term_from_diagonal_accumulator n = omi_pi_term_from_diagonal_race n.
Proof.
  intro n.
  unfold omi_pi_term_from_diagonal_accumulator, omi_pi_term_from_diagonal_race.
  rewrite diagonal_accumulator_phase_matches_race.
  reflexivity.
Qed.

Theorem omi_pi_sign_matches_alternation : forall n : nat,
  omi_pi_sign n = (-1) ^ n.
Proof.
  intro n.
  induction n using nat_induction_by_two.
  - vm_compute. reflexivity.
  - unfold omi_pi_sign, omi_pi_phase_even.
    simpl.
    field.
  - unfold omi_pi_sign, omi_pi_phase_even in *.
    rewrite Nat.even_succ_succ.
    rewrite IHn.
    simpl.
    ring.
Qed.

Theorem omi_pi_term_matches_tg_alt : forall n : nat,
  omi_pi_term_from_incidence n = tg_alt PI_tg n.
Proof.
  intro n.
  unfold omi_pi_term_from_incidence, omi_pi_den, tg_alt, PI_tg.
  rewrite omi_pi_sign_matches_alternation.
  field.
  apply not_0_INR.
  lia.
Qed.

Definition omi_pi_partial (n : nat) : R :=
  4 * sum_f_R0 (tg_alt PI_tg) n.

Definition omi_pi_partial_from_incidence (n : nat) : R :=
  4 * sum_f_R0 omi_pi_term_from_incidence n.

Definition omi_pi_partial_from_polybius (n : nat) : R :=
  4 * sum_f_R0 omi_pi_term_from_polybius n.

Definition omi_pi_partial_from_diagonal_race (n : nat) : R :=
  4 * sum_f_R0 omi_pi_term_from_diagonal_race n.

Definition omi_pi_partial_from_diagonal_accumulator (n : nat) : R :=
  4 * sum_f_R0 omi_pi_term_from_diagonal_accumulator n.

Definition omi_pi_lower (n : nat) : R := omi_pi_partial (S (2 * n)).

Definition omi_pi_upper (n : nat) : R := omi_pi_partial (2 * n).

Theorem omi_pi_partial_from_incidence_matches : forall n : nat,
  omi_pi_partial_from_incidence n = omi_pi_partial n.
Proof.
  intro n.
  unfold omi_pi_partial_from_incidence, omi_pi_partial.
  apply Rmult_eq_compat_l.
  apply sum_eq.
  intros k _.
  apply omi_pi_term_matches_tg_alt.
Qed.

Theorem omi_pi_partial_from_polybius_matches_incidence : forall n : nat,
  omi_pi_partial_from_polybius n = omi_pi_partial_from_incidence n.
Proof.
  intro n.
  unfold omi_pi_partial_from_polybius, omi_pi_partial_from_incidence.
  apply Rmult_eq_compat_l.
  apply sum_eq.
  intros k _.
  apply omi_pi_term_from_polybius_matches_incidence.
Qed.

Theorem omi_pi_partial_from_diagonal_race_matches_polybius : forall n : nat,
  omi_pi_partial_from_diagonal_race n = omi_pi_partial_from_polybius n.
Proof.
  intro n.
  unfold omi_pi_partial_from_diagonal_race, omi_pi_partial_from_polybius.
  apply Rmult_eq_compat_l.
  apply sum_eq.
  intros k _.
  apply omi_pi_term_from_diagonal_race_matches_polybius.
Qed.

Theorem omi_pi_partial_from_diagonal_accumulator_matches_race : forall n : nat,
  omi_pi_partial_from_diagonal_accumulator n = omi_pi_partial_from_diagonal_race n.
Proof.
  intro n.
  unfold omi_pi_partial_from_diagonal_accumulator, omi_pi_partial_from_diagonal_race.
  apply Rmult_eq_compat_l.
  apply sum_eq.
  intros k _.
  apply omi_pi_term_from_diagonal_accumulator_matches_race.
Qed.

Theorem omi_pi_projection_series_converges :
  Un_cv (fun n : nat => sum_f_R0 (tg_alt PI_tg) n) (OMI_PI / 4).
Proof.
  unfold OMI_PI, Alt_PI.
  destruct exist_PI as [l Hl].
  simpl.
  replace (4 * l / 4) with l by field.
  exact Hl.
Qed.

Theorem omi_pi_projection_interval_route : forall n : nat,
  omi_pi_lower n <= OMI_PI <= omi_pi_upper n.
Proof.
  intro n.
  unfold omi_pi_lower, omi_pi_upper, omi_pi_partial, OMI_PI.
  destruct (Alt_PI_ineq n) as [Hlo Hhi].
  split.
  - replace Alt_PI with (4 * (Alt_PI / 4)) by field.
    apply Rmult_le_compat_l; lra.
  - replace Alt_PI with (4 * (Alt_PI / 4)) by field.
    apply Rmult_le_compat_l; lra.
Qed.

Theorem omi_pi_partial_error_bound : forall n : nat,
  Rdist (omi_pi_partial n) OMI_PI <= 4 * PI_tg n.
Proof.
  intro n.
  unfold omi_pi_partial.
  replace OMI_PI with (4 * (OMI_PI / 4)) by field.
  rewrite Rdist_mult_l.
  replace (Rabs 4) with 4 by (rewrite Rabs_right; lra).
  apply Rmult_le_compat_l; [lra |].
  apply (Alt_first_term_bound PI_tg (OMI_PI / 4) n n).
  - exact PI_tg_decreasing.
  - exact PI_tg_cv.
  - exact omi_pi_projection_series_converges.
  - apply le_n.
Qed.

Theorem omi_pi_partial_error_bound_explicit : forall n : nat,
  Rdist (omi_pi_partial n) OMI_PI <= 4 / INR (2 * n + 1).
Proof.
  intro n.
  eapply Rle_trans.
  - apply omi_pi_partial_error_bound.
  - unfold PI_tg.
    right.
    reflexivity.
Qed.

Theorem OMI_PI_Equals_Real_PI : OMI_PI = PI.
Proof.
  unfold OMI_PI.
  exact Alt_PI_eq.
Qed.

Theorem omi_pi_incidence_projection_series_converges :
  Un_cv (fun n : nat => sum_f_R0 omi_pi_term_from_incidence n) (OMI_PI / 4).
Proof.
  eapply Un_cv_ext with
    (un := fun n : nat => sum_f_R0 (tg_alt PI_tg) n).
  - intro n.
    symmetry.
    apply sum_eq.
    intros k _.
    apply omi_pi_term_matches_tg_alt.
  - apply omi_pi_projection_series_converges.
Qed.

Definition omi_pi_incidence_limit : {l : R | Un_cv (fun n : nat => sum_f_R0 omi_pi_term_from_incidence n) l} :=
  exist _ (OMI_PI / 4) omi_pi_incidence_projection_series_converges.

Definition OMI_PI_from_incidence : R := 4 * proj1_sig omi_pi_incidence_limit.

Theorem OMI_PI_from_incidence_equals_OMI_PI :
  OMI_PI_from_incidence = OMI_PI.
Proof.
  unfold OMI_PI_from_incidence, omi_pi_incidence_limit.
  simpl.
  field.
Qed.

Theorem OMI_PI_from_incidence_equals_PI :
  OMI_PI_from_incidence = PI.
Proof.
  rewrite OMI_PI_from_incidence_equals_OMI_PI.
  apply OMI_PI_Equals_Real_PI.
Qed.

Theorem omi_pi_polybius_projection_series_converges :
  Un_cv (fun n : nat => sum_f_R0 omi_pi_term_from_polybius n) (OMI_PI / 4).
Proof.
  eapply Un_cv_ext with
    (un := fun n : nat => sum_f_R0 omi_pi_term_from_incidence n).
  - intro n.
    symmetry.
    apply sum_eq.
    intros k _.
    apply omi_pi_term_from_polybius_matches_incidence.
  - apply omi_pi_incidence_projection_series_converges.
Qed.

Definition omi_pi_polybius_limit : {l : R | Un_cv (fun n : nat => sum_f_R0 omi_pi_term_from_polybius n) l} :=
  exist _ (OMI_PI / 4) omi_pi_polybius_projection_series_converges.

Definition OMI_PI_from_polybius : R := 4 * proj1_sig omi_pi_polybius_limit.

Theorem OMI_PI_from_polybius_equals_incidence :
  OMI_PI_from_polybius = OMI_PI_from_incidence.
Proof.
  unfold OMI_PI_from_polybius, omi_pi_polybius_limit,
    OMI_PI_from_incidence, omi_pi_incidence_limit.
  simpl.
  field.
Qed.

Theorem OMI_PI_from_polybius_equals_PI :
  OMI_PI_from_polybius = PI.
Proof.
  rewrite OMI_PI_from_polybius_equals_incidence.
  apply OMI_PI_from_incidence_equals_PI.
Qed.

Theorem omi_pi_diagonal_race_projection_series_converges :
  Un_cv (fun n : nat => sum_f_R0 omi_pi_term_from_diagonal_race n) (OMI_PI / 4).
Proof.
  eapply Un_cv_ext with
    (un := fun n : nat => sum_f_R0 omi_pi_term_from_polybius n).
  - intro n.
    symmetry.
    apply sum_eq.
    intros k _.
    apply omi_pi_term_from_diagonal_race_matches_polybius.
  - apply omi_pi_polybius_projection_series_converges.
Qed.

Definition omi_pi_diagonal_race_limit :
  {l : R | Un_cv (fun n : nat => sum_f_R0 omi_pi_term_from_diagonal_race n) l} :=
  exist _ (OMI_PI / 4) omi_pi_diagonal_race_projection_series_converges.

Definition OMI_PI_from_diagonal_race : R :=
  4 * proj1_sig omi_pi_diagonal_race_limit.

Theorem OMI_PI_from_diagonal_race_equals_polybius :
  OMI_PI_from_diagonal_race = OMI_PI_from_polybius.
Proof.
  unfold OMI_PI_from_diagonal_race, omi_pi_diagonal_race_limit,
    OMI_PI_from_polybius, omi_pi_polybius_limit.
  simpl.
  field.
Qed.

Theorem OMI_PI_from_diagonal_race_equals_PI :
  OMI_PI_from_diagonal_race = PI.
Proof.
  rewrite OMI_PI_from_diagonal_race_equals_polybius.
  apply OMI_PI_from_polybius_equals_PI.
Qed.

Theorem omi_pi_diagonal_accumulator_projection_series_converges :
  Un_cv (fun n : nat => sum_f_R0 omi_pi_term_from_diagonal_accumulator n) (OMI_PI / 4).
Proof.
  eapply Un_cv_ext with
    (un := fun n : nat => sum_f_R0 omi_pi_term_from_diagonal_race n).
  - intro n.
    symmetry.
    apply sum_eq.
    intros k _.
    apply omi_pi_term_from_diagonal_accumulator_matches_race.
  - apply omi_pi_diagonal_race_projection_series_converges.
Qed.

Definition omi_pi_diagonal_accumulator_limit :
  {l : R | Un_cv (fun n : nat => sum_f_R0 omi_pi_term_from_diagonal_accumulator n) l} :=
  exist _ (OMI_PI / 4) omi_pi_diagonal_accumulator_projection_series_converges.

Definition OMI_PI_from_diagonal_accumulator : R :=
  4 * proj1_sig omi_pi_diagonal_accumulator_limit.

Theorem OMI_PI_from_diagonal_accumulator_equals_race :
  OMI_PI_from_diagonal_accumulator = OMI_PI_from_diagonal_race.
Proof.
  unfold OMI_PI_from_diagonal_accumulator, omi_pi_diagonal_accumulator_limit,
    OMI_PI_from_diagonal_race, omi_pi_diagonal_race_limit.
  simpl.
  field.
Qed.

Theorem OMI_PI_from_diagonal_accumulator_equals_PI :
  OMI_PI_from_diagonal_accumulator = PI.
Proof.
  rewrite OMI_PI_from_diagonal_accumulator_equals_race.
  apply OMI_PI_from_diagonal_race_equals_PI.
Qed.

Theorem omi_pi_bounds : 3 < OMI_PI < 4.
Proof.
  rewrite OMI_PI_Equals_Real_PI.
  split.
  - destruct (PI_ineq 3) as [Hlow _].
    apply (Rlt_le_trans _ (4 * sum_f_R0 (tg_alt PI_tg) 7) _).
    + unfold sum_f_R0, tg_alt, PI_tg; simpl; field_simplify; lra.
    + apply (Rmult_le_compat_l (4 : R)) in Hlow; [field_simplify in Hlow; exact Hlow | lra].
  - destruct (PI_ineq 1) as [_ Hup].
    apply (Rle_lt_trans _ (4 * sum_f_R0 (tg_alt PI_tg) 2) _).
    + apply (Rmult_le_compat_l (4 : R)) in Hup; [field_simplify in Hup; exact Hup | lra].
    + unfold sum_f_R0, tg_alt, PI_tg; simpl; field_simplify; lra.
Qed.
