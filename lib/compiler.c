#include "ast.h"
#include "isa.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t sym_hash(const char* s){
    uint32_t h = 0x811C9DC5;
    while(*s){
        h ^= (unsigned char)*s++;
        h *= 0x01000193;
    }
    return h;
}

static void emit(uint16_t* out, int* ip, int max, int op, int d, int a, int b, int imm){
    if(*ip >= max) return;
    out[(*ip)++] = (op<<11)|(d<<8)|(a<<5)|(b<<2)|(imm&3);
}

static void emit_load_val(uint32_t val, uint16_t* out, int* ip, int max, int reg){
    if(val <= 3){
        emit(out,ip,max,LOAD,reg,0,0,val);
    } else {
        emit(out,ip,max,LOAD,reg,0,0,3);
        if(*ip+1 >= max) return;
        out[(*ip)++] = val & 0xFFFF;
        out[(*ip)++] = (val >> 16) & 0xFFFF;
    }
}

static void compile_to_reg(Node* n, uint16_t* out, int* ip, int max, int dst, int* next_reg){
    if(!n) return;
    if(n->type == ATOM){
        emit_load_val(sym_hash(n->sym), out, ip, max, dst);
        return;
    }
    int r1 = *next_reg; (*next_reg)++;
    int r2 = *next_reg; (*next_reg)++;
    compile_to_reg(n->car, out, ip, max, r1, next_reg);
    compile_to_reg(n->cdr, out, ip, max, r2, next_reg);
    emit(out,ip,max,XOR,dst,r1,r2,0);
    *next_reg = r1;
}

static int is_word_form(Node* n){
    return n && n->type == PAIR && n->car->type == ATOM &&
           strcmp(n->car->sym, "word") == 0;
}

static int compile_word_seq(Node* n, uint16_t* out, int max, int ip){
    while(n && n->type == PAIR){
        if(n->car->type == PAIR && n->car->car->type == ATOM &&
           strcmp(n->car->car->sym, "word") == 0){
            uint32_t val = strtoul(n->car->cdr->sym, NULL, 0);
            if(ip < max) out[ip++] = val & 0xFFFF;
            n = n->cdr;
        } else if(is_word_form(n)){
            uint32_t val = strtoul(n->cdr->sym, NULL, 0);
            if(ip < max) out[ip++] = val & 0xFFFF;
            break;
        } else {
            break;
        }
    }
    return ip;
}

int compile(Node* n, uint16_t* out, int max){
    int ip = 0;
    if(n && n->type == PAIR &&
       ((n->car->type == PAIR && n->car->car->type == ATOM &&
         strcmp(n->car->car->sym, "word") == 0) || is_word_form(n))){
        ip = compile_word_seq(n, out, max, ip);
    } else {
        int next_reg = 1;
        compile_to_reg(n, out, &ip, max, 0, &next_reg);
        if(ip < max) out[ip++] = (HALT<<11);
    }
    return ip;
}
