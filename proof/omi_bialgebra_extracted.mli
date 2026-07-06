
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

type comparison =
| Eq
| Lt
| Gt

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

  val sub : n -> n -> n

  val compare : n -> n -> comparison

  val leb : n -> n -> bool

  val pos_div_eucl : positive -> n -> (n, n) prod

  val div_eucl : n -> n -> (n, n) prod

  val modulo : n -> n -> n

  val coq_lxor : n -> n -> n
 end

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

type 'a observer = { obs : (state -> 'a); fA : ('a -> 'a) }

val fano : state -> n

val fano_obs : n observer

val tetra : state -> n

val tetra_obs : n observer

val bQF : z -> z -> z

val state_energy : state -> z

val energy_obs : z observer

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

val bialg_fano : (state, n) bialg

val bialg_tetra : (state, n) bialg

val bialg_energy : (state, z) bialg
