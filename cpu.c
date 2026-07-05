#include "cpu.h"
#include "isa.h"

static inline uint32_t rotl(uint32_t x,int r){return (x<<r)|(x>>(32-r));}
static inline uint32_t rotr(uint32_t x,int r){return (x>>r)|(x<<(32-r));}

static void syscall_handler(OMI_CPU* cpu, uint32_t id){
    switch(id){
        case 0:
            fprintf(cpu->log,"SYSLOG PC=%u R0=%u delta=%u\n",cpu->PC,cpu->R[0],cpu->delta_acc);
            break;
        case 1:
            cpu->delta_acc = 0;
            break;
        case 2:
            cpu->mode = MODE_USER;
            break;
        case 3:
            cpu->mode = MODE_KERNEL;
            cpu->FLAGS |= SP_FLAG;
            break;
    }
}

void init_cpu(OMI_CPU* cpu){
    for(int i=0;i<REG_COUNT;i++) cpu->R[i]=0;
    cpu->delta_acc = 0;
    cpu->PC=0;
    cpu->SP=0;
    cpu->FLAGS=0;
    cpu->mode=MODE_BOOT;
    cpu->log=fopen("omi.log","w");
    cpu->program=0;
    cpu->prog_len=0;
}

void step(OMI_CPU* cpu, uint16_t instr){
    uint32_t op=(instr>>11)&0x1F;
    uint32_t dst=(instr>>8)&7;
    uint32_t a=(instr>>5)&7;
    uint32_t b=(instr>>2)&7;
    uint32_t imm=instr&3;

    if(cpu->mode == MODE_USER && (op == STORE || op == HALT)){
        if(op == STORE && (cpu->R[a] & 0xFFFF) < KERNEL_SIZE){
            cpu->mode = MODE_KERNEL;
            cpu->FLAGS |= SP_FLAG;
            return;
        }
        if(op == HALT){
            cpu->mode = MODE_KERNEL;
            cpu->FLAGS |= SP_FLAG;
            return;
        }
    }

    switch(op){
        case NOP: break;
        case MOV: cpu->R[dst]=cpu->R[a]; break;
        case LOAD:
            if(imm == 3 && cpu->program && cpu->PC+1 < cpu->prog_len){
                uint16_t lo = cpu->program[cpu->PC++];
                uint16_t hi = cpu->program[cpu->PC++];
                cpu->R[dst] = ((uint32_t)hi << 16) | lo;
            } else {
                cpu->R[dst] = imm;
            }
            break;
        case STORE:
            cpu->MEM[cpu->R[a] & 0xFFFF] = cpu->R[dst];
            break;
        case XOR:
            cpu->R[dst] = cpu->R[a] ^ cpu->R[b];
            if(dst != 0) cpu->delta_acc ^= cpu->R[dst];
            break;
        case ROTL: cpu->R[dst]=rotl(cpu->R[a],imm); break;
        case ROTR: cpu->R[dst]=rotr(cpu->R[a],imm); break;
        case ADD: cpu->R[dst]=cpu->R[a]+cpu->R[b]; break;
        case SUB: cpu->R[dst]=cpu->R[a]-cpu->R[b]; break;
        case CAR: cpu->R[dst]=cpu->R[a]&0xFFFF; break;
        case CDR: cpu->R[dst]=cpu->R[a]>>16; break;
        case CMP:
            cpu->FLAGS &= ~3;
            if(cpu->R[a] == cpu->R[b]) cpu->FLAGS |= 1;
            if(cpu->R[a] < cpu->R[b]) cpu->FLAGS |= 2;
            break;
        case JMP: cpu->PC = cpu->R[a]; break;
        case JZ:
            if(cpu->R[a] == 0) cpu->PC = cpu->R[b];
            break;
        case DELTA:
            cpu->R[dst]=rotl(cpu->R[a],1)^rotl(cpu->R[a],3)^rotr(cpu->R[a],2)^0xA5A5A5A5;
            if(dst != 0) cpu->delta_acc ^= cpu->R[dst];
            break;
        case LOADM:
            cpu->R[dst] = cpu->MEM[(cpu->R[a] + imm) & 0xFFFF];
            break;
        case CALL:
            cpu->SP = (cpu->SP - 1) & 0xFFFF;
            cpu->MEM[cpu->SP] = cpu->PC;
            cpu->PC = cpu->R[a];
            break;
        case RET:
            cpu->PC = cpu->MEM[cpu->SP];
            cpu->SP = (cpu->SP + 1) & 0xFFFF;
            break;
        case SYSCALL:
            syscall_handler(cpu, cpu->R[a]);
            break;
        case HALT: cpu->FLAGS |= HALT_FLAG; break;
    }

    fprintf(cpu->log,"PC=%u OP=%u R0=%u delta=%u mode=%d\n",cpu->PC,op,cpu->R[0],cpu->delta_acc,cpu->mode);
}

void run(OMI_CPU* cpu, uint16_t* program, uint32_t len){
    cpu->program = program;
    cpu->prog_len = len;
    while(cpu->PC < len && !(cpu->FLAGS & HALT_FLAG)){
        step(cpu, program[cpu->PC++]);
    }
}
