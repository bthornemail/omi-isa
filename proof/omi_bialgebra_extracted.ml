
type __ = Obj.t
let __ = let rec f _ = Obj.repr f in Obj.repr f

type nat =
| O
| S of nat

type ('a, 'b) prod =
| Pair of 'a * 'b

(** val snd : ('a1, 'a2) prod -> 'a2 **)

let snd = function
| Pair (_, y) -> y

type positive =
| XI of positive
| XO of positive
| XH

type n =
| N0
| Npos of positive

module Pos =
 struct
  (** val coq_Nsucc_double : n -> n **)

  let coq_Nsucc_double = function
  | N0 -> Npos XH
  | Npos p -> Npos (XI p)

  (** val coq_Ndouble : n -> n **)

  let coq_Ndouble = function
  | N0 -> N0
  | Npos p -> Npos (XO p)

  (** val coq_lxor : positive -> positive -> n **)

  let rec coq_lxor p q =
    match p with
    | XI p0 ->
      (match q with
       | XI q0 -> coq_Ndouble (coq_lxor p0 q0)
       | XO q0 -> coq_Nsucc_double (coq_lxor p0 q0)
       | XH -> Npos (XO p0))
    | XO p0 ->
      (match q with
       | XI q0 -> coq_Nsucc_double (coq_lxor p0 q0)
       | XO q0 -> coq_Ndouble (coq_lxor p0 q0)
       | XH -> Npos (XI p0))
    | XH ->
      (match q with
       | XI q0 -> Npos (XO q0)
       | XO q0 -> Npos (XI q0)
       | XH -> N0)
 end

module N =
 struct
  (** val coq_lxor : n -> n -> n **)

  let coq_lxor n0 m =
    match n0 with
    | N0 -> m
    | Npos p -> (match m with
                 | N0 -> n0
                 | Npos q -> Pos.coq_lxor p q)
 end

type word = n

(** val a : word -> word **)

let a = (fun x -> (x lsl 1) land 0xFFFF lxor (if x land 0x8000 <> 0 then 0x002D else 0))

(** val b : word -> word **)

let b = (fun x -> x)

(** val delta : word -> word -> word **)

let delta x c =
  N.coq_lxor (a x) (b c)

type state = (word, word) prod

(** val step : state -> state **)

let step = function
| Pair (x, c) -> Pair ((delta x c), c)

type 'a observer = { obs : (state -> 'a); fA : ('a -> 'a) }

(** val ctrl : state -> n **)

let ctrl =
  snd

(** val ctrl_obs : n observer **)

let ctrl_obs =
  { obs = ctrl; fA = (fun c -> c) }

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

(** val get_nth : nat -> 'a1 stream -> 'a1 **)

let rec get_nth n0 s =
  match n0 with
  | O -> let Cons (h, _) = Lazy.force s in h
  | S n' -> let Cons (_, t) = Lazy.force s in get_nth n' t

(** val bialg_obs : 'a1 observer -> (state, 'a1) bialg **)

let bialg_obs o =
  { act_bialg = step; obs_bialg = o.obs; step_bialg = step; fA_bialg = o.fA }

(** val obs_stream : 'a1 observer -> state -> 'a1 stream **)

let rec obs_stream o s =
  lazy (Cons ((o.obs s), (obs_stream o (step s))))

(** val bialgebra_commutation : __ **)

let bialgebra_commutation =
  __

(** val bialgebra_coherence : __ **)

let bialgebra_coherence =
  __

(** val bialg_ctrl : (state, n) bialg **)

let bialg_ctrl =
  bialg_obs ctrl_obs
