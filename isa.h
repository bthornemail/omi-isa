
#ifndef OMI_ISA_H
#define OMI_ISA_H

#define OP_MASK   0xF800
#define DST_MASK  0x0700
#define A_MASK    0x00E0
#define B_MASK    0x001C
#define IMM_MASK  0x0003

#define OP_SHIFT  11
#define DST_SHIFT 8
#define A_SHIFT   5
#define B_SHIFT   2
#define IMM_SHIFT 0

enum {
    NOP=0, MOV, LOAD, STORE,
    XOR, ROTL, ROTR,
    ADD, SUB,
    CAR, CDR,
    CMP, JMP, JZ,
    DELTA, HALT
};

#endif
