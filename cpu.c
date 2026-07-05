
#include "cpu.h"
#include "isa.h"

static inline uint32_t rotl(uint32_t x,int r){return (x<<r)|(x>>(32-r));}
static inline uint32_t rotr(uint32_t x,int r){return (x>>r)|(x<<(32-r));}

void init_cpu(OMI_CPU* cpu){
    for(int i=0;i<REG_COUNT;i++) cpu->R[i]=0;
    cpu->PC=0;
    cpu->FLAGS=0;
    cpu->mode=MODE_BOOT;
    cpu->log=fopen("omi.log","w");
}

void step(OMI_CPU* cpu,uint16_t instr){
    uint32_t op=(instr>>11)&0x1F;
    uint32_t dst=(instr>>8)&7;
    uint32_t a=(instr>>5)&7;
    uint32_t b=(instr>>2)&7;
    uint32_t imm=instr&3;

    switch(op){
        case MOV: cpu->R[dst]=cpu->R[a]; break;
        case LOAD: cpu->R[dst]=imm; break;
        case XOR: cpu->R[dst]=cpu->R[a]^cpu->R[b]; cpu->R[0]^=cpu->R[dst]; break;
        case ROTL: cpu->R[dst]=rotl(cpu->R[a],imm); break;
        case ROTR: cpu->R[dst]=rotr(cpu->R[a],imm); break;
        case ADD: cpu->R[dst]=cpu->R[a]+cpu->R[b]; break;
        case SUB: cpu->R[dst]=cpu->R[a]-cpu->R[b]; break;
        case CAR: cpu->R[dst]=cpu->R[a]&0xFFFF; break;
        case CDR: cpu->R[dst]=cpu->R[a]>>16; break;
        case DELTA:
            cpu->R[dst]=rotl(cpu->R[a],1)^rotl(cpu->R[a],3)^rotr(cpu->R[a],2)^0xA5A5A5A5;
            cpu->R[0]^=cpu->R[dst];
            break;
        case HALT: cpu->FLAGS=1; break;
    }

    fprintf(cpu->log,"PC=%u OP=%u R0=%u\n",cpu->PC,op,cpu->R[0]);
}

void run(OMI_CPU* cpu,uint16_t* program,uint32_t len){
    while(cpu->PC<len && !(cpu->FLAGS&1)){
        step(cpu,program[cpu->PC++]);
    }
}
