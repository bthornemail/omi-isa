#include "cpu.h"

void boot(OMI_CPU* cpu){
    for(int i=0;i<8;i++) cpu->R[i]=0;
    cpu->delta_acc = 0xA5A5A5A5;
    cpu->R[0] = 0xA5A5A5A5;
    cpu->PC = 0;
    cpu->mode = MODE_KERNEL;
    cpu->FLAGS = SP_FLAG;
}
