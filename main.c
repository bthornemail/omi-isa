#include "cpu.h"
#include "isa.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* parse(const char* src);
int compile(Node* n, uint16_t* out, int max);

static char* read_file(const char* path){
    FILE* f = fopen(path, "rb");
    if(!f){ perror("read_file"); return 0; }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    if(!buf){ fclose(f); return 0; }
    fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);
    return buf;
}

int main(int argc, char** argv){
    const char* src_path = argc > 1 ? argv[1] : "programs/test.omi";
    uint16_t prog[65536];

    char* src = read_file(src_path);
    if(!src){ fprintf(stderr,"failed to read %s\n",src_path); return 1; }

    Node* ast = parse(src);
    if(!ast){ fprintf(stderr,"no expressions parsed from %s\n",src_path); free(src); return 1; }

    printf("AST: ");
    print_ast(ast);
    printf("\n");

    int len = compile(ast, prog, 65536);
    printf("compiled %d instructions\n",len);

    free_ast(ast);
    free(src);

    OMI_CPU cpu;
    init_cpu(&cpu);
    boot(&cpu);
    run(&cpu, prog, len);

    printf("OMI-VM halted. R0=%u delta=%u log=omi.log\n",cpu.R[0],cpu.delta_acc);
    fclose(cpu.log);
    return 0;
}
