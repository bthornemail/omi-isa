
type __ = Obj.t
let __ = let rec f _ = Obj.repr f in Obj.repr f

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

module Pos =
 struct
  (** val pred_double : positive -> positive **)

  let rec pred_double = function
  | XI p -> XI (XO p)
  | XO p -> XI (pred_double p)
  | XH -> XH

  (** val pred_N : positive -> n **)

  let pred_N = function
  | XI p -> Npos (XO p)
  | XO p -> Npos (pred_double p)
  | XH -> N0

  (** val iter : ('a1 -> 'a1) -> 'a1 -> positive -> 'a1 **)

  let rec iter f x = function
  | XI n' -> f (iter f (iter f x n') n')
  | XO n' -> iter f (iter f x n') n'
  | XH -> f x

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

  (** val shiftl : positive -> n -> positive **)

  let shiftl p = function
  | N0 -> p
  | Npos n1 -> iter (fun x -> XO x) p n1

  (** val testbit : positive -> n -> bool **)

  let rec testbit p n0 =
    match p with
    | XI p0 -> (match n0 with
                | N0 -> True
                | Npos n1 -> testbit p0 (pred_N n1))
    | XO p0 -> (match n0 with
                | N0 -> False
                | Npos n1 -> testbit p0 (pred_N n1))
    | XH -> (match n0 with
             | N0 -> True
             | Npos _ -> False)
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

  (** val shiftl : n -> n -> n **)

  let shiftl a n0 =
    match a with
    | N0 -> N0
    | Npos a0 -> Npos (Pos.shiftl a0 n0)

  (** val testbit : n -> n -> bool **)

  let testbit a n0 =
    match a with
    | N0 -> False
    | Npos p -> Pos.testbit p n0
 end

type word = n

type state = (word, word) prod

(** val a_concrete : n -> n **)

let a_concrete = (fun x -> (x lsl 1) land 0xFFFF lxor (if x land 0x8000 <> 0 then 0x002D else 0))

(** val b_concrete : n -> n **)

let b_concrete = (fun x -> x)

type cState = { cx : n; cc : n }

(** val repr : cState -> state **)

let repr s =
  Pair (s.cx, s.cc)

(** val c_step : cState -> cState **)

let c_step s =
  { cx = (N.coq_lxor (a_concrete s.cx) (b_concrete s.cc)); cc = s.cc }

(** val c_trace : nat -> cState -> cState list **)

let rec c_trace n0 s =
  match n0 with
  | O -> Cons (s, Nil)
  | S k -> Cons (s, (c_trace k (c_step s)))

(** val step_local : state -> state **)

let step_local = function
| Pair (x, c) -> Pair ((N.coq_lxor (a_concrete x) (b_concrete c)), c)

(** val trace_local : nat -> state -> state list **)

let rec trace_local n0 s =
  match n0 with
  | O -> Cons (s, Nil)
  | S k -> Cons (s, (trace_local k (step_local s)))

(** val delta_refines : __ **)

let delta_refines =
  __

(** val trace_refines : __ **)

let trace_refines =
  __

(** val a_concrete_satisfies_axioms : __ **)

let a_concrete_satisfies_axioms =
  __
