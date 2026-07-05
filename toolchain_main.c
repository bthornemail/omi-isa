#include "ast.h"
#include "isa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* parse(const char* src);
int compile(Node* n, uint16_t* out, int max);
void print_ast(Node* n);

static char* read_file(const char* path){
    FILE* f = fopen(path, "rb");
    if(!f){ perror("read_file"); return 0; }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);
    return buf;
}

int main(int argc, char** argv){
    if(argc < 3){
        fprintf(stderr,"usage: omi_toolchain input.omi output.bin\n");
        return 1;
    }
    char* src = read_file(argv[1]);
    if(!src) return 1;
    Node* ast = parse(src);
    if(!ast){ fprintf(stderr,"parse failed\n"); free(src); return 1; }

    printf("AST: ");
    print_ast(ast);
    printf("\n");
    free(src);

    uint16_t prog[65536];
    int len = compile(ast, prog, 65536);
    printf("compiled %d instructions\n",len);
    free_ast(ast);

    FILE* f = fopen(argv[2], "wb");
    if(!f){ perror("output"); return 1; }
    fwrite(prog, 2, len, f);
    fclose(f);
    printf("wrote %s\n",argv[2]);
    return 0;
}
