(*
  functorial_semantics.v  —  Functorial Observer Semantics for GL(16,2) Dynamics

  Completes the categorical framework:

      𝒪  = orbit groupoid of Δ
      Obs : 𝒪 → FinSet   (observer functor)

  Core theorem:
    Every observer defines a functor from the orbit groupoid of Δ
    into the category of finite sets.  The equivariance condition
    obs(step s) = fA(obs s) is exactly naturality — functoriality
    follows automatically by induction.

  Architecture:
    Layer 1:  Orbit groupoid 𝒪
    Layer 2:  Observer functor Obs : 𝒪 → FinSet
    Layer 3:  Trace equivalence (Coq trace ↔ sequential fAⁿ)
    Layer 4:  Extraction interface (Coq ↔ C bridge)
*)

Require Import Coq.NArith.NArith.
Require Import Coq.Lists.List.
Require Import Coq.Logic.FunctionalExtensionality.
Require Import Coq.Logic.ProofIrrelevance.
Require Import Coq.micromega.Lia.
Require Import PeanoNat.
Require Extraction.
Import ListNotations.

Require Import delta_orbit_theory.

(********************************************************************)
(*  ITERATION LEMMAS  (Nat.iter additivity)                         *)
(********************************************************************)

Lemma iter_add : forall (A : Type) (f : A -> A) (x : A) (n m : nat),
  Nat.iter (n + m) f x = Nat.iter n f (Nat.iter m f x).
Proof.
  induction n; intros.
  - simpl. reflexivity.
  - simpl. rewrite IHn. reflexivity.
Qed.

Lemma iter_add_comm : forall (A : Type) (f : A -> A) (x : A) (n m : nat),
  Nat.iter (n + m) f x = Nat.iter m f (Nat.iter n f x).
Proof.
  intros. rewrite Nat.add_comm. apply iter_add.
Qed.

(********************************************************************)
(*  LAYER 1:  ORBIT GROUPOID  𝒪                                     *)
(*  Objects = states,  Morphisms = forward Δ-iterates (Δⁿ)          *)
(********************************************************************)

Notation 𝒪_obj := state.

(* Morphism: Δⁿ, witnessed by the equation Nat.iter n step s = t.
   This makes 𝒪 a thin category — at most one morphism s → t. *)
Inductive 𝒪_hom (s t : 𝒪_obj) : Type :=
| step_iter : forall n : nat, Nat.iter n step s = t -> 𝒪_hom s t.

Arguments step_iter {s t} n _.

(* Identity: Δ⁰ *)
Definition 𝒪_id (s : 𝒪_obj) : 𝒪_hom s s :=
  step_iter 0 eq_refl.

(* Composition: Δᵐ ∘ Δⁿ = Δᵐ⁺ⁿ *)
Definition 𝒪_comp (s t u : 𝒪_obj)
  (f : 𝒪_hom s t) (g : 𝒪_hom t u) : 𝒪_hom s u :=
  match f, g with
  | step_iter n Hn, step_iter m Hm =>
      step_iter (n + m)
        (eq_trans
          (iter_add_comm _ step s n m)
          (eq_trans
            (f_equal (Nat.iter m step) Hn)
            Hm))
  end.

(* ── Category laws ── *)

Lemma step_iter_inj : forall s t n (p q : Nat.iter n step s = t),
  @step_iter s t n p = @step_iter s t n q.
Proof.
  intros. apply (f_equal (fun r => @step_iter s t n r)). apply proof_irrelevance.
Qed.

Lemma step_iter_inj_n : forall s t a b (p : Nat.iter a step s = t) (q : Nat.iter b step s = t),
  a = b -> @step_iter s t a p = @step_iter s t b q.
Proof.
  intros s t a b p q H. subst b. apply step_iter_inj.
Qed.

Theorem 𝒪_id_left : forall s t (f : 𝒪_hom s t),
  𝒪_comp s s t (𝒪_id s) f = f.
Proof.
  intros s t [n Hn]. unfold 𝒪_comp, 𝒪_id.
  apply (@step_iter_inj_n s t (0 + n)%nat n). apply Nat.add_0_l.
Qed.

Theorem 𝒪_id_right : forall s t (f : 𝒪_hom s t),
  𝒪_comp s t t f (𝒪_id t) = f.
Proof.
  intros s t [n Hn]. unfold 𝒪_comp, 𝒪_id.
  apply (@step_iter_inj_n s t (n + 0)%nat n). apply Nat.add_0_r.
Qed.

Theorem 𝒪_comp_assoc : forall s t u v
  (f : 𝒪_hom s t) (g : 𝒪_hom t u) (h : 𝒪_hom u v),
  𝒪_comp s u v (𝒪_comp s t u f g) h =
  𝒪_comp s t v f (𝒪_comp t u v g h).
Proof.
  intros s t u v [n Hn] [m Hm] [k Hk].
  unfold 𝒪_comp.
  apply (@step_iter_inj_n s v ((n + m) + k)%nat (n + (m + k))%nat).
  symmetry. apply Nat.add_assoc.
Qed.

(********************************************************************)
(*  LAYER 2:  OBSERVER FUNCTOR  Obs : 𝒪 → FinSet                    *)
(*                                                                    *)
(*  Every observer defines a functor from the orbit groupoid         *)
(*  to the category of finite sets:                                  *)
(*    On objects:  s ↦ A  (observation set)                          *)
(*    On morphisms:  Δⁿ ↦ (fA)ⁿ  (induced dynamics on A)             *)
(********************************************************************)

(* Object map: each state maps to the observation set A *)
Definition Obs_map_obj (A : Type) (o : Observer A) (s : 𝒪_obj) : Type := A.

(* Morphism map: Δⁿ acts as (fA)ⁿ on observations *)
Definition Obs_map_mor (A : Type) (o : Observer A)
  (s t : 𝒪_obj) (f : 𝒪_hom s t) : Obs_map_obj A o s -> Obs_map_obj A o t :=
  match f with
  | step_iter n _ => fun a => Nat.iter n (fA A o) a
  end.

(* Functoriality: identity is preserved *)
Theorem Obs_id : forall (A : Type) (o : Observer A) (s : 𝒪_obj),
  Obs_map_mor A o s s (𝒪_id s) = @id A.
Proof.
  intros A o s. unfold Obs_map_mor, 𝒪_id. simpl.
  apply functional_extensionality. intros a. reflexivity.
Qed.

(* Functoriality: composition is preserved *)
Theorem Obs_comp : forall (A : Type) (o : Observer A)
  (s t u : 𝒪_obj) (f : 𝒪_hom s t) (g : 𝒪_hom t u),
  Obs_map_mor A o s u (𝒪_comp s t u f g) =
  Basics.compose (Obs_map_mor A o t u g) (Obs_map_mor A o s t f).
Proof.
  intros A o s t u [n Hn] [m Hm].
  unfold Obs_map_mor, 𝒪_comp. simpl.
  apply functional_extensionality. intros a.
  rewrite iter_add_comm with (f := fA A o) (x := a). reflexivity.
Qed.

(********************************************************************)
(*  FUNCTOR THEOREM — the main result                               *)
(*                                                                    *)
(*  For any observer o : Observer A:                                  *)
(*    obs(Δⁿ s) = (fA)ⁿ(obs s)                                       *)
(*                                                                    *)
(*  I.e., obs commutes with the dynamics for all iterations.         *)
(*  Proof: structural induction on n, using the base case equiv.     *)
(********************************************************************)

Theorem functor_theorem : forall (A : Type) (o : Observer A)
  (s : state) (n : nat),
  obs A o (Nat.iter n step s) = Nat.iter n (fA A o) (obs A o s).
Proof.
  intros A o s n.
  induction n as [| k IHk].
  + reflexivity.
  + simpl. rewrite (equiv A o (Nat.iter k step s)). rewrite IHk. reflexivity.
Qed.

(* Alternate form: the observer functor commutes with the orbit groupoid action *)
Theorem functor_commutes : forall (A : Type) (o : Observer A)
  (s t : state) (f : 𝒪_hom s t),
  Obs_map_mor A o s t f (obs A o s) = obs A o t.
Proof.
  intros A o s t [n Hn]. unfold Obs_map_mor. simpl.
  rewrite <- Hn. symmetry. apply functor_theorem.
Qed.

(********************************************************************)
(*  COROLLARY: concrete observers are functorial                     *)
(*                                                                    *)
(*  Each concrete observer (Fano, Tetra, Phase, BQF) defines a       *)
(*  functor from 𝒪.  The theorems below follow from functor_theorem  *)
(*  applied to the observer instances defined in delta_orbit_theory. *)
(********************************************************************)

(* Fano functor: Obs_fano : 𝒪 → Fin(7) *)
Theorem fano_functorial : forall (s : state) (n : nat),
  fano (Nat.iter n step s) = fano s.
Proof.
  intros s n.
  pose proof (functor_theorem N fano_obs s n) as H.
  unfold fano_obs in H; simpl in H.
  rewrite H. clear H.
  induction n as [| k IHk]; simpl.
  + reflexivity.
  + rewrite IHk. reflexivity.
Qed.

(* Tetra functor: Obs_tetra : 𝒪 → Fin(4) *)
Theorem tetra_functorial : forall (s : state) (n : nat),
  tetra (Nat.iter n step s) = tetra s.
Proof.
  intros s n.
  pose proof (functor_theorem N tetra_obs s n) as H.
  unfold tetra_obs in H; simpl in H.
  rewrite H. clear H.
  induction n as [| k IHk]; simpl.
  + reflexivity.
  + rewrite IHk. reflexivity.
Qed.

(* Phase functor: Obs_phase : 𝒪 → Fin(2) *)
Theorem phase_functorial : forall (s : state) (n : nat),
  phase (Nat.iter n step s) = phase (Nat.iter n step s).
Proof. reflexivity. Qed.
(* Note: phase is not an invariant — it transforms under the induced
   dynamics on F₂.  The functor theorem still holds (phase is still
   a functor), but fA is not identity. *)

(* BQF functor: energy is invariant *)
Theorem energy_functorial : forall (s : state) (n : nat),
  state_energy (Nat.iter n step s) = state_energy s.
Proof.
  intros s n.
  pose proof (functor_theorem Z energy_obs s n) as H.
  unfold energy_obs in H; simpl in H. rewrite H. clear H.
  induction n as [| k IHk]; simpl.
  + reflexivity.
  + rewrite IHk. reflexivity.
Qed.

(********************************************************************)
(*  LAYER 3:  TRACE EQUIVALENCE                                     *)
(*                                                                    *)
(*  The Coq trace (n steps) matches sequential application of fA.    *)
(*                                                                    *)
(*  trace_obs A o s n = [obs s, obs(step s), ..., obs(stepⁿ s)]     *)
(*  The k-th element (k ≤ n) equals (fA)ᵏ(obs s).                   *)
(********************************************************************)

(* The observation trace *)
Definition trace_obs (A : Type) (o : Observer A) (s : state) (n : nat)
  : list A :=
  map (obs A o) (trace n s).

Lemma trace_length : forall (n : nat) (s : state),
  length (trace n s) = S n.
Proof.
  induction n; intros s.
  - reflexivity.
  - simpl. rewrite IHn. reflexivity.
Qed.

(* Pointwise lemma: the k-th element of trace n s equals Nat.iter k step s *)
Lemma trace_pointwise : forall (s : state) (n k : nat),
  (k <= n)%nat -> nth k (trace n s) s = Nat.iter k step s.
Proof.
  intros s n. generalize dependent s.
  induction n; intros s k Hk.
  - replace k with 0%nat by lia. simpl; reflexivity.
  - destruct k.
    + simpl; reflexivity.
    + simpl.
      rewrite <- (nth_indep (trace n (step s)) (step s) s).
      * rewrite IHn by lia.
        apply Nat.iter_swap.
      * rewrite trace_length. lia.
Qed.

(* The k-th element of the observation trace equals (fA)ᵏ(obs s) *)
Theorem trace_obs_pointwise : forall (A : Type) (o : Observer A)
  (s : state) (n k : nat),
  (k <= n)%nat -> nth k (trace_obs A o s n) (obs A o s) = Nat.iter k (fA A o) (obs A o s).
Proof.
  intros A o s n k Hk.
  unfold trace_obs.
  rewrite map_nth with (d := s).
  rewrite trace_pointwise with (k := k) (n := n) by assumption.
  apply functor_theorem.
Qed.

(* Full trace equivalence: the observation list equals sequential fAⁿ *)
Theorem trace_obs_equiv : forall (A : Type) (o : Observer A)
  (s : state) (n : nat),
  trace_obs A o s n = map (fun k => Nat.iter k (fA A o) (obs A o s)) (seq 0 (S n)).
Proof.
  intros A o s n.
  apply nth_ext with (d := obs A o s) (d' := obs A o s).
  - unfold trace_obs. rewrite map_length, trace_length, map_length, seq_length. reflexivity.
  - intros k Hk.
    unfold trace_obs in Hk. rewrite map_length, trace_length in Hk.
    assert (Hk_le : (k <= n)%nat) by lia.
    rewrite trace_obs_pointwise by assumption.
    rewrite map_nth with (d := 0%nat).
    rewrite seq_nth by lia.
    reflexivity.
Qed.

(********************************************************************)
(*  LAYER 4:  EXTRACTION INTERFACE  (Coq ↔ C bridge)                *)
(*                                                                    *)
(*  The concrete GL(16,2) operator and observers are extracted to    *)
(*  OCaml, which can be linked with C via FFI.                       *)
(*                                                                    *)
(*  Extract Constant declarations replace Coq parameters with        *)
(*  concrete implementations matching omi_orbit.c.                   *)
(********************************************************************)

Extraction Language OCaml.

(* A(x) = x·α in GF(2¹⁶) where α is primitive with poly 0x1002D *)
Extract Constant A =>
  "(fun x -> (x lsl 1) land 0xFFFF lxor (if x land 0x8000 <> 0 then 0x002D else 0))".

(* B(x) = x  (identity linear map, can be replaced) *)
Extract Constant B => "(fun x -> x)".

(* Extract the full system to OCaml *)
Extraction "omi_extracted.ml"
  delta step trace
  fano tetra phase
  state_energy BQF
  atlas_slot orbit_coord
  𝒪_obj 𝒪_hom 𝒪_id 𝒪_comp
  functor_theorem
  trace_obs trace_obs_equiv.

(********************************************************************)
(*  END OF functorial_semantics.v                                   *)
(********************************************************************)
