#!/usr/bin/env python3
"""Generate the bootstrap compiler binary for OMI-ISA.

Two-pass assembler producing a .bin file containing raw 16-bit OMI-ISA
instructions for a self-hosting compiler.  The compiled program scans
MEM[0x4000+] for '0x' patterns, parses hex digits, and emits words to
MEM[0x6001+], storing the word count at MEM[0x6000].
"""

import struct
import sys

# ---- opcodes (isa.h enum) ----
NOP, MOV, LOAD, STORE = 0, 1, 2, 3
XOR, ROTL, ROTR = 4, 5, 6
ADD, SUB = 7, 8
CAR, CDR = 9, 10
CMP, JMP, JZ = 11, 12, 13
DELTA, HALT = 14, 15
SYSCALL = 16
LOADM, CALL, RET = 17, 18, 19


class Assembler:
    def __init__(self):
        self.ops = []
        self.labels = {}
        self.words = []

    def label(self, name):
        self.ops.append(('_label', (name,)))

    def _emit(self, name, *args):
        self.ops.append((name, args))

    # ---- low-level instructions ----
    def load(self, reg, imm):
        self._emit('load', reg, imm)

    def load_const(self, reg, val):
        self._emit('load_const', reg, val)

    def loadm(self, dst, base, imm=0):
        self._emit('loadm', dst, base, imm)

    def store(self, src, addr_reg):
        self._emit('store', src, addr_reg)

    def add(self, dst, a, b):
        self._emit('add', dst, a, b)

    def sub(self, dst, a, b):
        self._emit('sub', dst, a, b)

    def xor(self, dst, a, b):
        self._emit('xor', dst, a, b)

    def rotl(self, dst, a, imm):
        self._emit('rotl', dst, a, imm)

    def mov(self, dst, src):
        self._emit('mov', dst, src)

    def halt(self):
        self._emit('halt')

    # ---- branches using R7 as dedicated target register ----
    def jmp_to(self, label):
        self._emit('jmp_r7', label)

    def jz_to(self, test_reg, label):
        self._emit('jz_r7', test_reg, label)

    # ---- sizes (for first pass) ----
    def _size(self, name, args):
        if name == '_label':
            return 0
        if name in ('loadm', 'store', 'add', 'sub', 'xor', 'rotl',
                     'mov', 'halt', 'load'):
            return 1
        if name == 'load_const':
            return 3
        if name in ('jmp_r7', 'jz_r7'):
            return 4  # LOAD R7,3 + 2 data + branch
        raise ValueError(f"unknown op: {name}")

    # ---- first pass: compute label positions ----
    def first_pass(self):
        pc = 0
        for name, args in self.ops:
            if name == '_label':
                self.labels[args[0]] = pc
                continue
            pc += self._size(name, args)

    # ---- encoding ----
    def _enc(self, op, dst=0, a=0, b=0, imm=0):
        return (op << 11) | (dst << 8) | (a << 5) | (b << 2) | (imm & 3)

    # ---- second pass: emit words ----
    def second_pass(self):
        self.words = []
        for name, args in self.ops:
            if name == '_label':
                continue
            elif name == 'load':
                r, imm = args
                self.words.append(self._enc(LOAD, r, 0, 0, imm))
            elif name == 'load_const':
                r, v = args
                self.words.append(self._enc(LOAD, r, 0, 0, 3))
                self.words.append(v & 0xFFFF)
                self.words.append((v >> 16) & 0xFFFF)
            elif name == 'loadm':
                d, b, i = args
                self.words.append(self._enc(LOADM, d, b, 0, i))
            elif name == 'store':
                s, a = args
                self.words.append(self._enc(STORE, s, a))
            elif name == 'add':
                d, a, b = args
                self.words.append(self._enc(ADD, d, a, b))
            elif name == 'sub':
                d, a, b = args
                self.words.append(self._enc(SUB, d, a, b))
            elif name == 'xor':
                d, a, b = args
                self.words.append(self._enc(XOR, d, a, b))
            elif name == 'rotl':
                d, a, i = args
                self.words.append(self._enc(ROTL, d, a, 0, i))
            elif name == 'mov':
                d, s = args
                self.words.append(self._enc(MOV, d, s))
            elif name == 'halt':
                self.words.append(self._enc(HALT))
            elif name == 'jmp_r7':
                lbl = args[0]
                t = self.labels[lbl]
                self.words.append(self._enc(LOAD, 7, 0, 0, 3))
                self.words.append(t & 0xFFFF)
                self.words.append((t >> 16) & 0xFFFF)
                self.words.append(self._enc(JMP, 0, 7))
            elif name == 'jz_r7':
                test_reg, lbl = args
                t = self.labels[lbl]
                self.words.append(self._enc(LOAD, 7, 0, 0, 3))
                self.words.append(t & 0xFFFF)
                self.words.append((t >> 16) & 0xFFFF)
                self.words.append(self._enc(JZ, 0, test_reg, 7))

    def assemble(self):
        self.first_pass()
        self.second_pass()
        return self.words


def build_compiler():
    a = Assembler()

    # Register allocation:
    # R0: scratch (chars, constants, output address)
    # R1: source pointer
    # R2: output word count
    # R3: scratch (hex digit value, address temp)
    # R4: digit countdown (4 -> 0)
    # R5: word accumulator
    # R6: hex table base (0x3000)
    # R7: branch target register (dedicated)

    # ---- init ----
    a.load_const(1, 0x4000)      # R1 = source base
    a.load(2, 0)                 # R2 = 0 (output count)
    a.load_const(6, 0x3000)      # R6 = hex table base

    # ---- main_loop ----
    a.label('main_loop')
    a.loadm(0, 1, 0)             # R0 = MEM[R1] (char)
    a.jz_to(0, 'done')           # if R0 == 0, done

    # Check if R0 == '0' (0x30). Put XOR result in R3 (R7 is branch target reg).
    a.load_const(7, 0x30)        # R7 = '0'
    a.xor(3, 0, 7)               # R3 = R0 ^ '0'
    a.jz_to(3, 'check_x')        # if '0', check for 'x'

    # main_skip: R1 += 1, loop
    a.load(0, 1)                 # R0 = 1
    a.add(1, 1, 0)               # R1 += 1
    a.jmp_to('main_loop')

    # ---- check_x ----
    a.label('check_x')
    # R0 still == '0'. Check MEM[R1+1] == 'x' (0x78)
    a.load(0, 1)                 # R0 = 1
    a.add(3, 1, 0)               # R3 = R1 + 1
    a.loadm(0, 3, 0)             # R0 = MEM[R1+1] (char after '0')

    a.load_const(7, 0x78)        # R7 = 'x'
    a.xor(3, 0, 7)               # R3 = R0 ^ 'x'
    a.jz_to(3, 'parse_hex')      # if 'x', parse hex

    # Not 'x' — skip '0' and continue
    a.load(0, 1)
    a.add(1, 1, 0)               # R1 += 1
    a.jmp_to('main_loop')

    # ---- parse_hex ----
    a.label('parse_hex')
    # R1 still points to '0'. Skip "0x"
    a.load(0, 2)
    a.add(1, 1, 0)               # R1 += 2
    a.load_const(4, 4)           # R4 = 4 (digit countdown)
    a.load(5, 0)                 # R5 = 0 (accumulator)

    # ---- parse_digit_loop ----
    a.label('parse_digit_loop')
    a.loadm(0, 1, 0)             # R0 = MEM[R1] (char)
    a.add(3, 6, 0)               # R3 = 0x3000 + char
    a.loadm(3, 3, 0)             # R3 = hex_table[char] (digit value or 0xFF)

    # Check if hex digit (R3 != 0xFF)
    a.load_const(0, 0xFF)        # R0 = 0xFF
    a.xor(0, 3, 0)               # R0 = R3 ^ 0xFF (= 0 if not hex)
    a.jz_to(0, 'emit_word')      # if not hex, emit accumulated value

    # Accumulate: R5 = R5 * 16 + R3
    a.rotl(5, 5, 3)              # R5 <<= 3
    a.rotl(5, 5, 1)              # R5 <<= 1 (total <<= 4)
    a.add(5, 5, 3)               # R5 += hex digit

    # Advance source pointer
    a.load(0, 1)
    a.add(1, 1, 0)               # R1 += 1

    # Decrement countdown; if zero, emit
    a.load(0, 1)
    a.sub(4, 4, 0)               # R4 -= 1
    a.jz_to(4, 'emit_word')      # if 4 digits done, emit

    a.jmp_to('parse_digit_loop')

    # ---- emit_word ----
    a.label('emit_word')
    # MEM[0x6001 + R2] = R5
    a.load_const(0, 0x6001)      # R0 = output data base
    a.add(0, 0, 2)               # R0 = 0x6001 + R2
    a.store(5, 0)                # MEM[R0] = R5

    # R2 += 1
    a.load(0, 1)
    a.add(2, 2, 0)               # R2 += 1

    a.jmp_to('main_loop')

    # ---- done ----
    a.label('done')
    # MEM[0x6000] = R2
    a.load_const(0, 0x6000)
    a.store(2, 0)
    a.halt()

    return a.assemble()


def main():
    words = build_compiler()
    outpath = sys.argv[1] if len(sys.argv) > 1 else 'bootstrap-compiler.bin'

    with open(outpath, 'wb') as f:
        for w in words:
            f.write(struct.pack('<H', w))

    print(f"wrote {len(words)} words ({len(words)*2} bytes) to {outpath}")

    omipath = outpath.replace('.bin', '.omi')
    with open(omipath, 'w') as f:
        for w in words:
            f.write(f"(word . 0x{w:04X})\n")
    print(f"wrote {len(words)} lines to {omipath}")


if __name__ == '__main__':
    main()
