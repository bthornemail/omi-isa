#include "cpu.h"
#include "isa.h"
#include "ast.h"
#include "loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* parse(const char* src);
int compile(Node* n, uint16_t* out, int max);

static char* read_file(const char* path, size_t* out_len){
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
    if(out_len) *out_len = (size_t)len;
    return buf;
}

static int bootstrap_main(int argc, char** argv){
    const char* compiler_path = argv[2];
    const char* source_path = argv[3];
    const char* output_path = argc > 4 ? argv[4] : "output.bin";

    // Read compiler binary
    FILE* f = fopen(compiler_path, "rb");
    if(!f){ perror(compiler_path); return 1; }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    int prog_words = fsize / 2;
    uint16_t* prog = malloc(fsize);
    if(!prog){ fclose(f); return 1; }
    fread(prog, 1, fsize, f);
    fclose(f);

    // Read source text
    size_t src_len;
    char* src = read_file(source_path, &src_len);
    if(!src){ free(prog); return 1; }

    // Strip carrier before loading into MEM
    size_t payload_len;
    const char* payload = loader_strip(src, src_len, &payload_len);

    OMI_CPU cpu;
    init_cpu(&cpu);

    // Load compiler binary into MEM[KERNEL_BASE]
    for(int i=0; i<prog_words && (KERNEL_BASE+i) < MEM_SIZE; i++)
        cpu.MEM[KERNEL_BASE + i] = prog[i];

    // Load source text into MEM[0x4000], one byte per word (low byte)
    for(size_t i=0; i<payload_len && (0x4000+i) < MEM_SIZE; i++)
        cpu.MEM[0x4000 + i] = (unsigned char)payload[i];

    boot(&cpu);
    cpu.PC = 0;
    run(&cpu, prog, prog_words);

    // Extract output from MEM[0x6000]
    int out_len = cpu.MEM[0x6000];
    printf("compiler output: %d instructions\n", out_len);

    f = fopen(output_path, "wb");
    if(!f){ perror(output_path); free(prog); free(src); return 1; }
    for(int i=0; i<out_len; i++){
        uint16_t w = cpu.MEM[0x6001 + i] & 0xFFFF;
        fwrite(&w, 2, 1, f);
    }
    fclose(f);
    printf("wrote %s (%d instructions)\n", output_path, out_len);

    free(prog);
    free(src);
    fclose(cpu.log);
    return 0;
}

int main(int argc, char** argv){
    if(argc >= 4 && strcmp(argv[1], "--boot") == 0)
        return bootstrap_main(argc, argv);

    const char* src_path = argc > 1 ? argv[1] : "programs/test.omi";
    uint16_t prog[65536];

    size_t src_len;
    char* src = read_file(src_path, &src_len);
    if(!src){ fprintf(stderr,"failed to read %s\n",src_path); return 1; }

    size_t payload_len;
    const char* payload = loader_strip(src, src_len, &payload_len);

    Node* ast = parse(payload);
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
