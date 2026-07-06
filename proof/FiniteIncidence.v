(*
  FiniteIncidence.v -- finite tetrahedral, Fano, and rectified-core incidence.

  This layer deliberately stays on exact finite combinatorics.  Metric meaning
  is introduced later by MetricProjection.v.
*)

From Coq Require Import NArith.NArith.
From Coq Require Import Lists.List.
Import ListNotations.
Open Scope N_scope.

Require Import DiagonalClosure.

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
