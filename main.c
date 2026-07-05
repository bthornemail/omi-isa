
#include "cpu.h"
#include "isa.h"

uint16_t program[] = {
    (LOAD<<11)|(1<<8)|0|0|1,
    (LOAD<<11)|(2<<8)|0|0|2,
    (XOR<<11)|(0<<8)|(1<<5)|(2<<2),
    (HALT<<11)
};

int main(){
    OMI_CPU cpu;
    init_cpu(&cpu);
    boot(&cpu);
    run(&cpu,program,4);
    return 0;
}
