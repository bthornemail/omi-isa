(*
  coalgebraic_bisimulation.v  —  Coalgebraic bisimulation and trace equivalence for GL(16,2)

  This file completes the coalgebraic semantics upgrade by:
  1. Defining the output-producing state functor F(X) = X * O
  2. Defining F-coalgebras and coalgebra morphisms (equivariant maps)
  3. Defining coalgebraic bisimulations
  4. Proving the general trace equivalence theorem for bisimilar systems
  5. Specializing these to the GL(16,2) state dynamics to establish C ↔ Coq bridge correctness
*)

Require Import Coq.Lists.List.
Require Import Coq.Logic.FunctionalExtensionality.
Require Import Coq.Logic.ProofIrrelevance.
Import ListNotations.

Require Import delta_orbit_theory.

Section CoalgebraicSemantics.

  (* The observation output type *)
  Variable O : Type.

  (* 1. The Output Functor: F(X) = X * O *)
  Definition F (X : Type) : Type := (X * O)%type.

  Definition fmap {X Y : Type} (f : X -> Y) (xy : F X) : F Y :=
    (f (fst xy), snd xy).

  Lemma fmap_id : forall {X : Type} (xy : F X), fmap (fun x => x) xy = xy.
  Proof. intros X [x o]. reflexivity. Qed.

  Lemma fmap_comp : forall {X Y Z : Type} (g : Y -> Z) (f : X -> Y) (xy : F X),
    fmap (fun x => g (f x)) xy = fmap g (fmap f xy).
  Proof. intros X Y Z g f [x o]. reflexivity. Qed.

  (* 2. F-Coalgebra definition *)
  Record Coalgebra : Type := {
    Carrier : Type;
    step_obs : Carrier -> F Carrier
  }.

  (* Helper projections *)
  Definition step (C : Coalgebra) (x : Carrier C) : Carrier C :=
    fst (step_obs C x).

  Definition obs (C : Coalgebra) (x : Carrier C) : O :=
    snd (step_obs C x).

  (* 3. Coalgebra Morphism *)
  Record Morphism (C1 C2 : Coalgebra) : Type := {
    mor : Carrier C1 -> Carrier C2;
    commute : forall x, step_obs C2 (mor x) = fmap mor (step_obs C1 x)
  }.

  Definition id_morphism (C : Coalgebra) : Morphism C C.
  Proof.
    refine {|
      mor := fun x => x
    |}.
    intros x.
    simpl. unfold fmap.
    destruct (step_obs C x) as [s o].
    reflexivity.
  Defined.

  Definition compose_morphism (C1 C2 C3 : Coalgebra) 
    (m1 : Morphism C1 C2) (m2 : Morphism C2 C3) : Morphism C1 C3.
  Proof.
    refine {|
      mor := fun x => mor C2 C3 m2 (mor C1 C2 m1 x)
    |}.
    intros x.
    pose proof (commute C1 C2 m1 x) as H1.
    pose proof (commute C2 C3 m2 (mor C1 C2 m1 x)) as H2.
    unfold fmap in H1, H2.
    destruct (step_obs C1 x) as [s1 o1] eqn:E1.
    destruct (step_obs C2 (mor C1 C2 m1 x)) as [s2 o2] eqn:E2.
    destruct (step_obs C3 (mor C2 C3 m2 (mor C1 C2 m1 x))) as [s3 o3] eqn:E3.
    simpl in *.
    inversion H1; subst.
    inversion H2; subst.
    reflexivity.
  Defined.

  (* 4. Coalgebraic Bisimulation *)
  Record Bisimulation (C1 C2 : Coalgebra) : Type := {
    rel : Carrier C1 -> Carrier C2 -> Prop;
    bisim_obs : forall x y, rel x y -> obs C1 x = obs C2 y;
    bisim_step : forall x y, rel x y -> rel (step C1 x) (step C2 y)
  }.

  Lemma bisim_iter : forall (C1 C2 : Coalgebra) (B : Bisimulation C1 C2) (n : nat) (x : Carrier C1) (y : Carrier C2),
    rel C1 C2 B x y -> rel C1 C2 B (Nat.iter n (step C1) x) (Nat.iter n (step C2) y).
  Proof.
    intros C1 C2 B n.
    induction n; intros x y Hrel.
    - exact Hrel.
    - simpl. apply bisim_step. apply IHn. exact Hrel.
  Qed.

  (* 5. Coalgebra State and Observation Traces *)
  Fixpoint trace (C : Coalgebra) (n : nat) (x : Carrier C) : list (Carrier C) :=
    match n with
    | 0 => [x]
    | S n' => x :: trace C n' (step C x)
    end.

  Definition trace_obs (C : Coalgebra) (n : nat) (x : Carrier C) : list O :=
    map (obs C) (trace C n x).

  Theorem trace_equivalence : forall (C1 C2 : Coalgebra) (B : Bisimulation C1 C2) (n : nat) (x : Carrier C1) (y : Carrier C2),
    rel C1 C2 B x y -> trace_obs C1 n x = trace_obs C2 n y.
  Proof.
    intros C1 C2 B n.
    unfold trace_obs.
    induction n; intros x y Hrel.
    - simpl. f_equal. apply bisim_obs with (b := B) (x := x) (y := y). exact Hrel.
    - simpl. f_equal.
      + apply bisim_obs with (b := B) (x := x) (y := y). exact Hrel.
      + apply IHn. apply bisim_step with (b := B). exact Hrel.
  Qed.

  (* 7. Morphisms induce Bisimulations *)
  Definition morphism_bisimulation (C1 C2 : Coalgebra) (m : Morphism C1 C2) : Bisimulation C1 C2.
  Proof.
    refine {|
      rel := fun x y => mor C1 C2 m x = y
    |}.
    - intros x y Hrel. subst y.
      unfold obs.
      pose proof (commute C1 C2 m x) as Hcomm.
      rewrite Hcomm.
      unfold fmap. destruct (step_obs C1 x) as [s1 o1]. simpl. reflexivity.
    - intros x y Hrel. subst y.
      unfold step.
      pose proof (commute C1 C2 m x) as Hcomm.
      rewrite Hcomm.
      unfold fmap. destruct (step_obs C1 x) as [s1 o1]. simpl. reflexivity.
  Defined.

  (* 8. Mapping to the GL(16,2) System *)
  Definition state_coalgebra (o : state -> O) : Coalgebra :=
    {| Carrier := state;
       step_obs := fun s => (delta_orbit_theory.step s, o s) |}.

  Definition target_coalgebra (fo : O -> O) : Coalgebra :=
    {| Carrier := O;
       step_obs := fun o => (fo o, o) |}.

  Definition morphism_equiv : forall (o : state -> O) (fo : O -> O)
    (Hequiv : forall s, o (delta_orbit_theory.step s) = fo (o s)),
    Morphism (state_coalgebra o) (target_coalgebra fo).
  Proof.
    intros o fo Hequiv.
    apply (Build_Morphism (state_coalgebra o) (target_coalgebra fo) o).
    intros s.
    simpl. unfold fmap. simpl.
    f_equal.
    symmetry. apply Hequiv.
  Defined.

  Corollary morphism_trace_equivalence : forall (o : state -> O) (fo : O -> O)
    (Hequiv : forall s, o (delta_orbit_theory.step s) = fo (o s)) (n : nat) (s : state),
    trace_obs (state_coalgebra o) n s = trace_obs (target_coalgebra fo) n (o s).
  Proof.
    intros o fo Hequiv n s.
    apply trace_equivalence with (B := morphism_bisimulation _ _ (morphism_equiv o fo Hequiv)).
    simpl. reflexivity.
  Qed.

End CoalgebraicSemantics.
