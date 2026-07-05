
#ifndef OMI_CPU_H
#define OMI_CPU_H

#include <stdint.h>
#include <stdio.h>

#define REG_COUNT 8
#define MEM_SIZE 65536

typedef enum {
    MODE_BOOT,
    MODE_KERNEL,
    MODE_USER
} Mode;

typedef struct {
    uint32_t R[REG_COUNT];
    uint32_t MEM[MEM_SIZE];
    uint32_t PC;
    uint32_t FLAGS;
    Mode mode;
    FILE* log;
} OMI_CPU;

void init_cpu(OMI_CPU* cpu);
void run(OMI_CPU* cpu, uint16_t* program, uint32_t len);

#endif
