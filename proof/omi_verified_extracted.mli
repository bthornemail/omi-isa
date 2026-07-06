
type __ = Obj.t

type bool =
| True
| False

type nat =
| O
| S of nat

type ('a, 'b) prod =
| Pair of 'a * 'b

type 'a list =
| Nil
| Cons of 'a * 'a list

type positive =
| XI of positive
| XO of positive
| XH

type n =
| N0
| Npos of positive

module Pos :
 sig
  val pred_double : positive -> positive

  val pred_N : positive -> n

  val iter : ('a1 -> 'a1) -> 'a1 -> positive -> 'a1

  val coq_Nsucc_double : n -> n

  val coq_Ndouble : n -> n

  val coq_lxor : positive -> positive -> n

  val shiftl : positive -> n -> positive

  val testbit : positive -> n -> bool
 end

module N :
 sig
  val coq_lxor : n -> n -> n

  val shiftl : n -> n -> n

  val testbit : n -> n -> bool
 end

type word = n

type state = (word, word) prod

val a_concrete : n -> n

val b_concrete : n -> n

type cState = { cx : n; cc : n }

val repr : cState -> state

val c_step : cState -> cState

val c_trace : nat -> cState -> cState list

val step_local : state -> state

val trace_local : nat -> state -> state list

val delta_refines : __

val trace_refines : __

val a_concrete_satisfies_axioms : __
