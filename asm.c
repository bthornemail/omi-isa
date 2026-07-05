
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "isa.h"

uint16_t encode(int op,int d,int a,int b,int imm){
    return (op<<11)|(d<<8)|(a<<5)|(b<<2)|(imm&3);
}
