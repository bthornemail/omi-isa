
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

(** val snd : ('a1, 'a2) prod -> 'a2 **)

let snd = function
| Pair (_, y) -> y

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

module Pos =
 struct
  type mask =
  | IsNul
  | IsPos of positive
  | IsNeg
 end

module Coq_Pos =
 struct
  (** val succ : positive -> positive **)

  let rec succ = function
  | XI p -> XO (succ p)
  | XO p -> XI p
  | XH -> XO XH

  (** val add : positive -> positive -> positive **)

  let rec add x y =
    match x with
    | XI p ->
      (match y with
       | XI q -> XO (add_carry p q)
       | XO q -> XI (add p q)
       | XH -> XO (succ p))
    | XO p ->
      (match y with
       | XI q -> XI (add p q)
       | XO q -> XO (add p q)
       | XH -> XI p)
    | XH -> (match y with
             | XI q -> XO (succ q)
             | XO q -> XI q
             | XH -> XO XH)

  (** val add_carry : positive -> positive -> positive **)

  and add_carry x y =
    match x with
    | XI p ->
      (match y with
       | XI q -> XI (add_carry p q)
       | XO q -> XO (add_carry p q)
       | XH -> XI (succ p))
    | XO p ->
      (match y with
       | XI q -> XO (add_carry p q)
       | XO q -> XI (add p q)
       | XH -> XO (succ p))
    | XH ->
      (match y with
       | XI q -> XI (succ q)
       | XO q -> XO (succ q)
       | XH -> XI XH)

  (** val pred_double : positive -> positive **)

  let rec pred_double = function
  | XI p -> XI (XO p)
  | XO p -> XI (pred_double p)
  | XH -> XH

  type mask = Pos.mask =
  | IsNul
  | IsPos of positive
  | IsNeg

  (** val succ_double_mask : mask -> mask **)

  let succ_double_mask = function
  | IsNul -> IsPos XH
  | IsPos p -> IsPos (XI p)
  | IsNeg -> IsNeg

  (** val double_mask : mask -> mask **)

  let double_mask = function
  | IsPos p -> IsPos (XO p)
  | x0 -> x0

  (** val double_pred_mask : positive -> mask **)

  let double_pred_mask = function
  | XI p -> IsPos (XO (XO p))
  | XO p -> IsPos (XO (pred_double p))
  | XH -> IsNul

  (** val sub_mask : positive -> positive -> mask **)

  let rec sub_mask x y =
    match x with
    | XI p ->
      (match y with
       | XI q -> double_mask (sub_mask p q)
       | XO q -> succ_double_mask (sub_mask p q)
       | XH -> IsPos (XO p))
    | XO p ->
      (match y with
       | XI q -> succ_double_mask (sub_mask_carry p q)
       | XO q -> double_mask (sub_mask p q)
       | XH -> IsPos (pred_double p))
    | XH -> (match y with
             | XH -> IsNul
             | _ -> IsNeg)

  (** val sub_mask_carry : positive -> positive -> mask **)

  and sub_mask_carry x y =
    match x with
    | XI p ->
      (match y with
       | XI q -> succ_double_mask (sub_mask_carry p q)
       | XO q -> double_mask (sub_mask p q)
       | XH -> IsPos (pred_double p))
    | XO p ->
      (match y with
       | XI q -> double_mask (sub_mask_carry p q)
       | XO q -> succ_double_mask (sub_mask_carry p q)
       | XH -> double_pred_mask p)
    | XH -> IsNeg

  (** val mul : positive -> positive -> positive **)

  let rec mul x y =
    match x with
    | XI p -> add y (XO (mul p y))
    | XO p -> XO (mul p y)
    | XH -> y

  (** val compare_cont : comparison -> positive -> positive -> comparison **)

  let rec compare_cont r x y =
    match x with
    | XI p ->
      (match y with
       | XI q -> compare_cont r p q
       | XO q -> compare_cont Gt p q
       | XH -> Gt)
    | XO p ->
      (match y with
       | XI q -> compare_cont Lt p q
       | XO q -> compare_cont r p q
       | XH -> Gt)
    | XH -> (match y with
             | XH -> r
             | _ -> Lt)

  (** val compare : positive -> positive -> comparison **)

  let compare =
    compare_cont Eq

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
  (** val succ_double : n -> n **)

  let succ_double = function
  | N0 -> Npos XH
  | Npos p -> Npos (XI p)

  (** val double : n -> n **)

  let double = function
  | N0 -> N0
  | Npos p -> Npos (XO p)

  (** val sub : n -> n -> n **)

  let sub n0 m =
    match n0 with
    | N0 -> N0
    | Npos n' ->
      (match m with
       | N0 -> n0
       | Npos m' ->
         (match Coq_Pos.sub_mask n' m' with
          | Coq_Pos.IsPos p -> Npos p
          | _ -> N0))

  (** val compare : n -> n -> comparison **)

  let compare n0 m =
    match n0 with
    | N0 -> (match m with
             | N0 -> Eq
             | Npos _ -> Lt)
    | Npos n' -> (match m with
                  | N0 -> Gt
                  | Npos m' -> Coq_Pos.compare n' m')

  (** val leb : n -> n -> bool **)

  let leb x y =
    match compare x y with
    | Gt -> False
    | _ -> True

  (** val pos_div_eucl : positive -> n -> (n, n) prod **)

  let rec pos_div_eucl a0 b0 =
    match a0 with
    | XI a' ->
      let Pair (q, r) = pos_div_eucl a' b0 in
      let r' = succ_double r in
      (match leb b0 r' with
       | True -> Pair ((succ_double q), (sub r' b0))
       | False -> Pair ((double q), r'))
    | XO a' ->
      let Pair (q, r) = pos_div_eucl a' b0 in
      let r' = double r in
      (match leb b0 r' with
       | True -> Pair ((succ_double q), (sub r' b0))
       | False -> Pair ((double q), r'))
    | XH ->
      (match b0 with
       | N0 -> Pair (N0, (Npos XH))
       | Npos p ->
         (match p with
          | XH -> Pair ((Npos XH), N0)
          | _ -> Pair (N0, (Npos XH))))

  (** val div_eucl : n -> n -> (n, n) prod **)

  let div_eucl a0 b0 =
    match a0 with
    | N0 -> Pair (N0, N0)
    | Npos na ->
      (match b0 with
       | N0 -> Pair (N0, a0)
       | Npos _ -> pos_div_eucl na b0)

  (** val modulo : n -> n -> n **)

  let modulo a0 b0 =
    snd (div_eucl a0 b0)

  (** val coq_lxor : n -> n -> n **)

  let coq_lxor n0 m =
    match n0 with
    | N0 -> m
    | Npos p -> (match m with
                 | N0 -> n0
                 | Npos q -> Coq_Pos.coq_lxor p q)
 end

module Z =
 struct
  (** val double : z -> z **)

  let double = function
  | Z0 -> Z0
  | Zpos p -> Zpos (XO p)
  | Zneg p -> Zneg (XO p)

  (** val succ_double : z -> z **)

  let succ_double = function
  | Z0 -> Zpos XH
  | Zpos p -> Zpos (XI p)
  | Zneg p -> Zneg (Coq_Pos.pred_double p)

  (** val pred_double : z -> z **)

  let pred_double = function
  | Z0 -> Zneg XH
  | Zpos p -> Zpos (Coq_Pos.pred_double p)
  | Zneg p -> Zneg (XI p)

  (** val pos_sub : positive -> positive -> z **)

  let rec pos_sub x y =
    match x with
    | XI p ->
      (match y with
       | XI q -> double (pos_sub p q)
       | XO q -> succ_double (pos_sub p q)
       | XH -> Zpos (XO p))
    | XO p ->
      (match y with
       | XI q -> pred_double (pos_sub p q)
       | XO q -> double (pos_sub p q)
       | XH -> Zpos (Coq_Pos.pred_double p))
    | XH ->
      (match y with
       | XI q -> Zneg (XO q)
       | XO q -> Zneg (Coq_Pos.pred_double q)
       | XH -> Z0)

  (** val add : z -> z -> z **)

  let add x y =
    match x with
    | Z0 -> y
    | Zpos x' ->
      (match y with
       | Z0 -> x
       | Zpos y' -> Zpos (Coq_Pos.add x' y')
       | Zneg y' -> pos_sub x' y')
    | Zneg x' ->
      (match y with
       | Z0 -> x
       | Zpos y' -> pos_sub y' x'
       | Zneg y' -> Zneg (Coq_Pos.add x' y'))

  (** val mul : z -> z -> z **)

  let mul x y =
    match x with
    | Z0 -> Z0
    | Zpos x' ->
      (match y with
       | Z0 -> Z0
       | Zpos y' -> Zpos (Coq_Pos.mul x' y')
       | Zneg y' -> Zneg (Coq_Pos.mul x' y'))
    | Zneg x' ->
      (match y with
       | Z0 -> Z0
       | Zpos y' -> Zneg (Coq_Pos.mul x' y')
       | Zneg y' -> Zpos (Coq_Pos.mul x' y'))

  (** val of_N : n -> z **)

  let of_N = function
  | N0 -> Z0
  | Npos p -> Zpos p
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

(** val fano : state -> n **)

let fano = function
| Pair (x, _) -> N.modulo x (Npos (XI (XI XH)))

(** val fano_obs : n observer **)

let fano_obs =
  { obs = fano; fA = (fun f -> f) }

(** val tetra : state -> n **)

let tetra = function
| Pair (x, _) -> N.modulo x (Npos (XO (XO XH)))

(** val tetra_obs : n observer **)

let tetra_obs =
  { obs = tetra; fA = (fun t -> t) }

(** val bQF : z -> z -> z **)

let bQF x y =
  Z.add
    (Z.add (Z.mul (Z.mul (Zpos (XO (XO (XI (XI (XI XH)))))) x) x)
      (Z.mul (Z.mul (Zpos (XO (XO (XO (XO XH))))) x) y))
    (Z.mul (Z.mul (Zpos (XO (XO XH))) y) y)

(** val state_energy : state -> z **)

let state_energy = function
| Pair (x, c) -> bQF (Z.of_N x) (Z.of_N c)

(** val energy_obs : z observer **)

let energy_obs =
  { obs = state_energy; fA = (fun e -> e) }

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

(** val bialg_fano : (state, n) bialg **)

let bialg_fano =
  bialg_obs fano_obs

(** val bialg_tetra : (state, n) bialg **)

let bialg_tetra =
  bialg_obs tetra_obs

(** val bialg_energy : (state, z) bialg **)

let bialg_energy =
  bialg_obs energy_obs
