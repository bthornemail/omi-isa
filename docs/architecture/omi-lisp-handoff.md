# OMI-Lisp Handoff Contract

## Purpose

Define the boundary between `omi-lisp` (declaration surface) and `omi-isa`
(execution surface). The two repositories remain separate. They connect
through handoff fixture files --- never through imports.

## Seam

```text
omi-lisp                      omi-isa
    |                             |
    | owns declaration syntax     | owns ISA, VM, dispatch, receipts
    |                             |
    | --- handoff fixture ---->   |
    |      .omi source            | parse + compile + execute
    |      lowered .omibin        | verify deterministic receipt
    |      expected receipt       |
    |                             |
```

## Boundary Rules

- **omi-lisp** owns declaration syntax and normalization. It produces
  handoff fixtures.
- **omi-isa** owns loader, ISA, VM, envelope, dispatch, and receipts.
  It consumes handoff fixtures.
- **No repository imports the other's source code.**
- The handoff artifact is the boundary.
- Receipts record what omi-isa observed.

## Fixture Directory Layout

```text
handoff/
  omi-lisp/
    source/        .omi files (omi-lisp-shaped declarations)
    lowered/       .omibin files (bytecode, future omi-lisp output)
    expected/      .receipt.txt files (deterministic expected results)
```

## Current Fixtures

| File | Content | Purpose |
|------|---------|---------|
| `source/seed.omi` | `(NULL . NULL)` | Minimal dotted pair; proves syntactic handoff |
| `source/pair.omi` | `(omi . imo)` | OMI/IMO boundary language; proves deterministic execution |

## Receipt Format

Expected receipt files record the deterministic VM final state:

```text
R0: <value>
delta: <value>
halted: yes
```

## What The Handoff Proves

- omi-isa parser accepts all omi-lisp syntactic forms (dotted pairs, atoms)
- Loader/parser/compiler/VM pipeline processes fixtures without error
- Execution is deterministic: same input always produces same R0 and delta
- No omi-lisp source code was imported to prove this

## Local Parser Status

The OMI-Lisp parser inside `omi-isa` (`lib/parser.c`, `lib/lexer.c`) is:

> **Legacy compatibility parser.**
> It exists only to test the ISA lowering path from omi-lisp-shaped fixtures.
> The canonical declaration language lives in the sibling `omi-lisp`
> repository. This parser does not make omi-isa the canonical omi-lisp
> implementation.

## NULL Treatment

`NULL` is parsed as an ordinary atom. No sentinel semantics are assigned
in v0.1. This pass proves syntactic handoff and deterministic execution
only.

## Verification

```sh
make clean && make test_lisp_handoff
```

## Seam Ownership Summary

| Layer | Owner | Artifact |
|-------|-------|----------|
| Declaration syntax | omi-lisp | `.omi` source files |
| Normalized form | omi-lisp | canonical fixture |
| Bytecode lowering | omi-lisp (future) | `.omibin` files |
| Execution | omi-isa | VM state |
| Observation | omi-isa | Receipt |
| Expected results | omi-isa | `.receipt.txt` |
