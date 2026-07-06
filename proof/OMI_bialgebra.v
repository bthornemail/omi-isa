(*
  OMI_bialgebra.v  —  Bialgebra structure for GL(16,2) dynamics

  Formalises (S, Δ, obs) as a family of bialgebras over a distributive law:

      obs ∘ Δ = fA ∘ obs   (the functor theorem / bialgebra_commutation)

  Each observer defines a distinct bialgebra instance.  The bialgebra
  coherence theorem shows that algebraic iteration (Nat.iter n act x)
  and coalgebraic unfolding (obs_stream x) produce the same observation
  sequence — C and Coq are not two implementations to compare, but two
  realisations of the same bialgebra.
*)

Require Import Coq.Lists.List.
Require Coq.Numbers.BinNums.
Import ListNotations.

Require Import delta_orbit_theory.
Require Import functorial_semantics.

(********************************************************************)
(*  1.  ALGEBRA — a carrier with a unary action                     *)
(********************************************************************)

Record Alg (carrier : Type) : Type := {
  act : carrier -> carrier
}.

(********************************************************************)
(*  2.  COALGEBRA — a carrier with observation and dynamics         *)
(********************************************************************)

Record CoAlg (carrier obs_type : Type) : Type := {
  obs_coalg : carrier -> obs_type;
  step_coalg : carrier -> carrier
}.

(********************************************************************)
(*  3.  BIALGEBRA — compatible algebra + coalgebra over             *)
(*      a distributive law                                           *)
(*                                                                    *)
(*  The field distrib_law says that the diagram commutes:            *)
(*                                                                    *)
(*       act                         obs                             *)
(*   carrier ──→ carrier ──→ obs_type                                 *)
(*      │                        │                                   *)
(*      │obs                     │fA                                 *)
(*      v                        v                                   *)
(*   obs_type ────────────────→ obs_type                             *)
(*                                                                    *)
(********************************************************************)

Record Bialg (carrier obs_type : Type) : Type := {
  (* algebra structure *)
  act_bialg : carrier -> carrier;
  (* coalgebra structure *)
  obs_bialg : carrier -> obs_type;
  step_bialg : carrier -> carrier;
  (* induced dynamics on the observation type *)
  fA_bialg : obs_type -> obs_type;
  (* step and act coincide (same smooth dynamics) *)
  step_eq_act : forall x, step_bialg x = act_bialg x;
  (* distributive law: obs commutes with dynamics via fA *)
  distrib_law : forall x, obs_bialg (act_bialg x) = fA_bialg (obs_bialg x)
}.

(********************************************************************)
(*  4.  COINDUCTIVE STREAM — infinite observation trace             *)
(********************************************************************)

CoInductive stream (X : Type) : Type :=
  Cons : X -> stream X -> stream X.

Arguments Cons {X} _ _.

Fixpoint get_nth {X : Type} (n : nat) (s : stream X) : X :=
  match n with
  | O => match s with Cons h _ => h end
  | S n' => match s with Cons _ t => get_nth n' t end
  end.

(********************************************************************)
(*  5.  OBSERVER FAMILY — one bialgebra per Observer A o            *)
(********************************************************************)

Section ObserverBialgebras.

  Context {A : Type} (o : Observer A).

  (* ── Algebra instance: GL(16,2) action ── *)

  Definition alg_state : Alg state :=
    {| act := step |}.

  (* ── Coalgebra instance: observer unfolding ── *)

  Definition coalg_obs : CoAlg state A :=
    {| obs_coalg := obs A o;
       step_coalg := step |}.

  (* ── Bialgebra instance: structured compatibility ── *)

  Definition bialg_obs : Bialg state A :=
    {| act_bialg := step;
       obs_bialg := obs A o;
       step_bialg := step;
       fA_bialg := fA A o;
       step_eq_act := fun x => eq_refl;
       distrib_law := equiv A o
    |}.

  (* ── Coinductive infinite observation stream ── *)

  CoFixpoint obs_stream (s : state) : stream A :=
    Cons (obs A o s) (obs_stream (step s)).

  (* ── Helper: iter swap lemma ── *)

  Lemma iter_swap (f : A -> A) (x : A) (n : nat) :
    Nat.iter n f (f x) = f (Nat.iter n f x).
  Proof.
    induction n; simpl; auto.
    rewrite IHn. reflexivity.
  Qed.

  (*****************************************************************)
  (*  THEOREM 1: Bialgebra commutation                              *)
  (*                                                               *)
  (*  The iterated distributive law: obs(actⁿ s) = (fA)ⁿ(obs s)    *)
  (*****************************************************************)

  Theorem bialgebra_commutation (s : state) (n : nat) :
    obs A o (Nat.iter n step s) = Nat.iter n (fA A o) (obs A o s).
  Proof.
    exact (functor_theorem A o s n).
  Qed.

  (*****************************************************************)
  (*  THEOREM 2: Bialgebra coherence                               *)
  (*                                                               *)
  (*  Algebraic iteration and coalgebraic unfolding agree:         *)
  (*    get_nth n (obs_stream s) = (fA)ⁿ(obs s)                    *)
  (*                                                               *)
  (*  This is the core structural identity that makes C and Coq    *)
  (*  two realisations of the same bialgebra, not two systems      *)
  (*  that merely agree on test cases.                             *)
  (*****************************************************************)

  Lemma get_nth_obs_stream_succ (s : state) (k : nat) :
    get_nth (S k) (obs_stream s) = get_nth k (obs_stream (step s)).
  Proof.
    lazy iota delta [obs_stream get_nth]. reflexivity.
  Qed.

  Theorem bialgebra_coherence (s : state) (n : nat) :
    get_nth n (obs_stream s) = Nat.iter n (fA A o) (obs A o s).
  Proof.
    revert s.
    induction n as [| k IHk]; intros s.
    - reflexivity.
    - rewrite get_nth_obs_stream_succ.
      rewrite (IHk (step s)).
      rewrite (equiv A o s).
      simpl.
      apply iter_swap.
  Qed.

  (*****************************************************************)
  (*  COROLLARY: finite trace equivalence from bialgebra coherence *)
  (*****************************************************************)

  Corollary bialgebra_trace_obs_pointwise (s : state) (n k : nat)
    (Hk : k <= n) :
    nth k (map (obs A o) (trace n s)) (obs A o s) = Nat.iter k (fA A o) (obs A o s).
  Proof.
    change (map (obs A o) (trace n s)) with (trace_obs A o s n).
    rewrite (trace_obs_pointwise A o s n k Hk).
    rewrite <- bialgebra_coherence.
    reflexivity.
  Qed.

End ObserverBialgebras.

(********************************************************************)
(*  6.  CONCRETE BIALGEBRA INSTANCES                                *)
(*                                                                    *)
(*  Each concrete observer defines a bialgebra.  The instances       *)
(*  below make the categorical structure concrete.                    *)
(********************************************************************)

Definition bialg_fano : Bialg state Coq.Numbers.BinNums.N := bialg_obs fano_obs.
Definition bialg_tetra : Bialg state Coq.Numbers.BinNums.N := bialg_obs tetra_obs.
Definition bialg_energy : Bialg state Coq.Numbers.BinNums.Z := bialg_obs energy_obs.
(* phase_obs not defined in delta_orbit_theory.v — add when observer is fully specified *)

(********************************************************************)
(*  7.  EXTRACTION  —  OMI bialgebra to OCaml                       *)
(********************************************************************)

Extraction Language OCaml.

Extract Constant A =>
  "(fun x -> (x lsl 1) land 0xFFFF lxor (if x land 0x8000 <> 0 then 0x002D else 0))".
Extract Constant B => "(fun x -> x)".

Extraction "omi_bialgebra_extracted.ml"
  Alg CoAlg Bialg
  stream get_nth
  bialg_obs bialg_fano bialg_tetra bialg_energy
  obs_stream bialgebra_commutation bialgebra_coherence.

(********************************************************************)
(*  END OF OMI_bialgebra.v                                          *)
(********************************************************************)
