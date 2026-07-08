# OMI-ISA Agent Contract

## Repository Role

`omi-isa` is the bounded executable runtime surface of OMI:
fixed-width instructions, deterministic replay, 512-bit envelopes,
dispatch, observers, transport adapters, mesh routing, and receipts.

## Non-Negotiable Boundaries

Agents must not:

- Expand the ISA/runtime boundary into omi-lisp, omi-port, or
  omi-axioms territory
- Import authority from omi-port into omi-isa
- Add transport or protocol behavior beyond the existing surface
- Treat extracted proofs as still belonging to this repository
- Re-introduce files that were extracted (proof/, canon/, dev-docs/
  beyond the one alignment doc)
- Import sibling omi-lisp or omi-port source code as a dependency
- Add new code features before handoff fixture alignment is committed
- Treat handoff/ directory as local program territory

## Extracted Siblings

| Repository | Role |
|------------|------|
| `omi-lisp` | Declaration surface |
| `omi-port` | Dormant source-target-gauge route candidates |
| `omi-axioms` | Coq proof lineage and constitutional law |

These repos are external. Agents may reference them but must not
create code dependencies.

## Handoff Boundary

The `handoff/` directory is the seam between repositories:

```text
handoff/omi-lisp/
  source/     .omi fixture files from omi-lisp territory
  lowered/    future .omibin bytecode from omi-lisp compiler
  expected/   .receipt.txt deterministic expected outputs
```

Rules:
- `handoff/` is for external seams only
- `programs/` is for native local examples
- No sibling source code may be imported
- The local OMI-Lisp parser (`lib/parser.c`) is a compatibility
  parser for testing the lowering path, not canonical omi-lisp
- Handoff tests must verify deterministic observable output
  against committed expected receipt fixtures

## Build Layout

```text
lib/     source code (authority)
build/   object files, intermediates (disposable)
bin/     runnable binaries (disposable)
```

No generated executable may live at repo root.
No object file may live in `lib/`.
Object files go to `build/`.
Runnable outputs go to `bin/`.

This layout must not regress. Run the layout gate to verify:

```sh
test -d lib && test -d build && test -d bin
find . -maxdepth 1 -type f -perm -111 ! -name '*.sh' ! -name 'configure' -print
find . -maxdepth 1 -name '*.o' -print
```

Expected: no root binaries, no root `.o` files.

## Alignment Priority

1. Document alignment (identity, authority, architecture)
2. Behavior preservation (no changes without tests)
3. Repo hygiene (gitignore, stale files)

## Verification Commands

```sh
make clean && make test
git status --short
rg -n "whole OMI canon|entire canon|proves reality|philosophy proves" .
```

## Handoff Format

When finishing a task, report:
- Summary
- Files changed
- Boundary checks (no sibling imports, no authority expansion)
- Verification results
- Next recommended step
