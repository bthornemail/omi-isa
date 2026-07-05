
#include <stdint.h>
#include <stdio.h>

#define OP_LOAD 2
#define OP_XOR  4

uint16_t emit(int op,int a,int b,int c,int imm){
    return (op<<11)|(a<<8)|(b<<5)|(c<<2)|(imm&3);
}

void compile_demo(){
    // (a . b) -> LOAD/LOAD/XOR sequence placeholder
    uint16_t prog[4];

    prog[0] = emit(OP_LOAD,1,0,0,1);
    prog[1] = emit(OP_LOAD,2,0,0,2);
    prog[2] = emit(OP_XOR,0,1,2,0);

    printf("compiled demo program\n");
}
