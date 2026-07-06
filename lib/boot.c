#include "cpu.h"

#define HEX_TABLE_BASE 0x3000

void init_hex_table(OMI_CPU* cpu){
    for(int i=0; i<256; i++){
        unsigned char c = (unsigned char)i;
        if(c >= '0' && c <= '9')
            cpu->MEM[HEX_TABLE_BASE + i] = c - '0';
        else if(c >= 'A' && c <= 'F')
            cpu->MEM[HEX_TABLE_BASE + i] = c - 'A' + 10;
        else if(c >= 'a' && c <= 'f')
            cpu->MEM[HEX_TABLE_BASE + i] = c - 'a' + 10;
        else
            cpu->MEM[HEX_TABLE_BASE + i] = 0xFF;
    }
}

void boot(OMI_CPU* cpu){
    for(int i=0;i<8;i++) cpu->R[i]=0;
    cpu->delta_acc = 0xA5A5A5A5;
    cpu->R[0] = 0xA5A5A5A5;
    cpu->PC = 0;
    cpu->SP = 0x8000;
    cpu->mode = MODE_KERNEL;
    cpu->FLAGS = SP_FLAG;
    init_hex_table(cpu);
}
