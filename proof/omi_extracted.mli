
type __ = Obj.t

type bool =
| True
| False

type nat =
| O
| S of nat

type ('a, 'b) prod =
| Pair of 'a * 'b

val snd : ('a1, 'a2) prod -> 'a2

type 'a list =
| Nil
| Cons of 'a * 'a list

type comparison =
| Eq
| Lt
| Gt

val add : nat -> nat -> nat

type positive =
| XI of positive
| XO of positive
| XH

type n =
| N0
| Npos of positive

type z =
| Z0
| Zpos of positive
| Zneg of positive

module Pos :
 sig
  type mask =
  | IsNul
  | IsPos of positive
  | IsNeg
 end

module Coq_Pos :
 sig
  val succ : positive -> positive

  val add : positive -> positive -> positive

  val add_carry : positive -> positive -> positive

  val pred_double : positive -> positive

  type mask = Pos.mask =
  | IsNul
  | IsPos of positive
  | IsNeg

  val succ_double_mask : mask -> mask

  val double_mask : mask -> mask

  val double_pred_mask : positive -> mask

  val sub_mask : positive -> positive -> mask

  val sub_mask_carry : positive -> positive -> mask

  val mul : positive -> positive -> positive

  val compare_cont : comparison -> positive -> positive -> comparison

  val compare : positive -> positive -> comparison

  val coq_Nsucc_double : n -> n

  val coq_Ndouble : n -> n

  val coq_lxor : positive -> positive -> n
 end

module N :
 sig
  val succ_double : n -> n

  val double : n -> n

  val add : n -> n -> n

  val sub : n -> n -> n

  val mul : n -> n -> n

  val compare : n -> n -> comparison

  val leb : n -> n -> bool

  val pos_div_eucl : positive -> n -> (n, n) prod

  val div_eucl : n -> n -> (n, n) prod

  val modulo : n -> n -> n

  val coq_lxor : n -> n -> n
 end

val map : ('a1 -> 'a2) -> 'a1 list -> 'a2 list

module Z :
 sig
  val double : z -> z

  val succ_double : z -> z

  val pred_double : z -> z

  val pos_sub : positive -> positive -> z

  val add : z -> z -> z

  val mul : z -> z -> z

  val of_N : n -> z
 end

type word = n

val a : word -> word

val b : word -> word

val delta : word -> word -> word

type state = (word, word) prod

val step : state -> state

val trace : nat -> state -> state list

type 'a observer = { obs : (state -> 'a); fA : ('a -> 'a) }

val fano : state -> n

val tetra : state -> n

val phase : state -> n

val bQF : z -> z -> z

val state_energy : state -> z

val atlas_slot : n -> n -> n -> n

val orbit_coord : state -> n

type _UU1d4aa__hom =
  nat
  (* singleton inductive, whose constructor was step_iter *)

val _UU1d4aa__id : state -> _UU1d4aa__hom

val _UU1d4aa__comp :
  state -> state -> state -> _UU1d4aa__hom -> _UU1d4aa__hom -> _UU1d4aa__hom

val functor_theorem : __

val trace_obs : 'a1 observer -> state -> nat -> 'a1 list

val trace_obs_equiv : __
