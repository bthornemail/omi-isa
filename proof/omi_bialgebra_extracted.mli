
type __ = Obj.t

type nat =
| O
| S of nat

type ('a, 'b) prod =
| Pair of 'a * 'b

val snd : ('a1, 'a2) prod -> 'a2

type positive =
| XI of positive
| XO of positive
| XH

type n =
| N0
| Npos of positive

module Pos :
 sig
  val coq_Nsucc_double : n -> n

  val coq_Ndouble : n -> n

  val coq_lxor : positive -> positive -> n
 end

module N :
 sig
  val coq_lxor : n -> n -> n
 end

type word = n

val a : word -> word

val b : word -> word

val delta : word -> word -> word

type state = (word, word) prod

val step : state -> state

type 'a observer = { obs : (state -> 'a); fA : ('a -> 'a) }

val ctrl : state -> n

val ctrl_obs : n observer

type 'carrier alg =
  'carrier -> 'carrier
  (* singleton inductive, whose constructor was Build_Alg *)

type ('carrier, 'obs_type) coAlg = { obs_coalg : ('carrier -> 'obs_type);
                                     step_coalg : ('carrier -> 'carrier) }

type ('carrier, 'obs_type) bialg = { act_bialg : ('carrier -> 'carrier);
                                     obs_bialg : ('carrier -> 'obs_type);
                                     step_bialg : ('carrier -> 'carrier);
                                     fA_bialg : ('obs_type -> 'obs_type) }

type 'x stream = 'x __stream Lazy.t
and 'x __stream =
| Cons of 'x * 'x stream

val get_nth : nat -> 'a1 stream -> 'a1

val bialg_obs : 'a1 observer -> (state, 'a1) bialg

val obs_stream : 'a1 observer -> state -> 'a1 stream

val bialgebra_commutation : __

val bialgebra_coherence : __

val bialg_ctrl : (state, n) bialg
