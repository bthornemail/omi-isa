# OMI-ISA

A minimal 16-bit instruction set architecture with a reversible XOR-rotation delta law, register VM, boot sequence, and OMI-Lisp toolchain.

## Architecture

```
OMI-Lisp (.omi) → lexer → parser → AST → compiler → 16-bit OMI-ISA bytecode → boot → CPU → log
```

### Delta Law

The core operator that defines the system:

```
Δ(x) = rotl(x,1) ⊕ rotl(x,3) ⊕ rotr(x,2) ⊕ C
```

XOR preserves information (no bit destruction), rotations spread local influence globally, and the constant breaks the zero fixed point. The `delta_acc` register accumulates all non-R0 XOR results as an invariant trace.

### CPU

- 8 × 32-bit registers (R0–R7)
- 64K × 32-bit memory
- 16-bit fixed-width instructions
- Three privilege modes: BOOT → KERNEL → USER
- SP_FLAG privilege boundary (dot notation is earned)
- Deterministic execution with append-only log

### Instruction Set

| Opcode | Format | Operation |
|--------|--------|-----------|
| NOP | `00000 ddd aaa bbb ii` | no-op |
| MOV | `00001 ddd aaa --- --` | R[d] = R[a] |
| LOAD | `00010 ddd --- --- ii` | R[d] = imm (ii=11 → 32-bit extended) |
| STORE | `00011 ddd aaa --- --` | MEM[R[a]] = R[d] |
| XOR | `00100 ddd aaa bbb --` | R[d] = R[a] ^ R[b]; delta ^= R[d] |
| ROTL | `00101 ddd aaa --- ii` | R[d] = rotl(R[a], imm) |
| ROTR | `00110 ddd aaa --- ii` | R[d] = rotr(R[a], imm) |
| ADD | `00111 ddd aaa bbb --` | R[d] = R[a] + R[b] |
| SUB | `01000 ddd aaa bbb --` | R[d] = R[a] - R[b] |
| CAR | `01001 ddd aaa --- --` | R[d] = R[a] & 0xFFFF (low half) |
| CDR | `01010 ddd aaa --- --` | R[d] = R[a] >> 16 (high half) |
| CMP | `01011 --- aaa bbb --` | FLAGS = cmp(R[a], R[b]) |
| JMP | `01100 --- aaa --- --` | PC = R[a] |
| JZ | `01101 --- aaa bbb --` | if R[a]==0: PC = R[b] |
| DELTA | `01110 ddd aaa --- --` | R[d] = Δ(R[a]); delta ^= R[d] |
| HALT | `01111 ------------` | stop execution |
| SYSCALL | `10000 --- aaa --- --` | syscall(cpu, R[a]) |

Extended immediate: when `ii=11`, the instruction is followed by two 16-bit words forming a 32-bit value.

### Syscalls

| ID | Function |
|----|----------|
| 0 | SYSLOG — log R0 and delta to receipt |
| 1 | reset delta_acc to 0 |
| 2 | enter USER mode |
| 3 | enter KERNEL mode (SP boundary) |

## Build

```
make          # builds omi_vm and omi_toolchain
make omi_vm   # full pipeline (compile + boot + run)
make omi_toolchain  # standalone compiler
make run      # omi_vm programs/test.omi
make run-tc   # omi_toolchain programs/test.omi test.bin
make clean    # remove build artifacts
```

## Usage

### VM (compile and run)

```
./omi_vm programs/test.omi
```

Output:
```
AST: (omi . imo)
compiled 8 instructions
OMI-VM halted. R0=3804767436 delta=2779096485 log=omi.log
```

### Toolchain (compile only)

```
./omi_toolchain input.omi output.bin
```

Writes raw 16-bit OMI-ISA bytecode suitable for loading into the VM.

## File Layout

```
.
├── asm.c              instruction encoder
├── ast.c / ast.h      AST node constructors and utilities
├── boot.c             boot sequence (seed, delta init, kernel entry)
├── compiler.c         OMI-Lisp AST → 16-bit bytecode
├── cpu.c / cpu.h      register VM, step(), run(), mode enforcement
├── isa.h              opcodes, bit masks, encoding shifts
├── lexer.c / lexer.h  OMI-Lisp tokenizer
├── main.c             pipeline: read → parse → compile → boot → run
├── Makefile           build system
├── parser.c           recursive-descent parser (PEG grammar)
├── toolchain_main.c   standalone compiler entry point
├── programs/
│   ├── test.omi       (omi . imo)
│   ├── kernel.omi     (boot . (seed . delta))
│   └── init.omi       (omi--imo . o---o/---/?---?@---@)
└── omi.log            execution receipt trace (generated)
```

## OMI-Lisp Grammar

```peg
OMI    <- SP? Expr
Expr   <- Pair / Atom
Pair   <- '(' Expr '.' Expr ')'
Atom   <- [a-zA-Z_@/?-][a-zA-Z0-9_@/?-]*
```

The SP (0x20) byte marks the boundary between pre-language control stream and readable OMI-Lisp. If present, parsing begins after the first SP.

## Memory Map

| Range | Region |
|-------|--------|
| 0x0000–0x0FFF | Boot ROM (immutable) |
| 0x1000–0x7FFF | Kernel space |
| 0x8000–0xEFFF | User space |
| 0xF000–0xFFFF | Syscall trap zone |
