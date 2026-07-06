#ifndef OMI_CPU_H
#define OMI_CPU_H

#include <stdint.h>
#include <stdio.h>

#define REG_COUNT 8
#define MEM_SIZE 65536

#define BOOT_ROM_BASE  0x0000
#define BOOT_ROM_SIZE  0x1000
#define KERNEL_BASE    0x1000
#define KERNEL_SIZE    0x7000
#define USER_BASE      0x8000
#define USER_SIZE      0x7000
#define TRAP_BASE      0xF000
#define TRAP_SIZE      0x1000

#define SP_FLAG   0x1
#define HALT_FLAG 0x2

#define PROBE_STATE_IDLE       0
#define PROBE_STATE_PROBING    1
#define PROBE_STATE_NEGOTIATED 2

typedef enum {
    MODE_BOOT,
    MODE_KERNEL,
    MODE_USER
} Mode;

typedef struct {
    uint32_t R[REG_COUNT];
    uint32_t delta_acc;
    uint32_t MEM[MEM_SIZE];
    uint32_t PC;
    uint32_t SP;
    uint32_t FLAGS;
    Mode mode;
    FILE* log;
    uint16_t* program;
    uint32_t prog_len;

    uint32_t capability;
    uint32_t peer_capability;
    uint32_t isa_subset;
    uint8_t neighbor_count;
    uint8_t neighbors[16];
    uint8_t probe_state;
} OMI_CPU;

void init_cpu(OMI_CPU* cpu);
void boot(OMI_CPU* cpu);
void step(OMI_CPU* cpu, uint16_t instr);
void run(OMI_CPU* cpu, uint16_t* program, uint32_t len);

#endif
