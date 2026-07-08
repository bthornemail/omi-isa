#include "loader.h"
#include "ast.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* parse(const char* src);
int compile(Node* n, uint16_t* out, int max);
void print_ast(Node* n);
void free_ast(Node* n);

static int tests_passed = 0;
static int tests_run = 0;

static void TEST(const char* name, int cond) {
    tests_run++;
    if (cond) tests_passed++;
    printf("  %s: %s\n", cond ? "PASS" : "FAIL", name);
}

static char* read_file(const char* path, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) { perror(path); return 0; }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    if (!buf) { fclose(f); return 0; }
    fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);
    if (out_len) *out_len = (size_t)len;
    return buf;
}

static int read_receipt(const char* path, uint32_t* out_r0, uint32_t* out_delta) {
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    int r = fscanf(f, "R0: %u\ndelta: %u\nhalted: yes\n", out_r0, out_delta);
    fclose(f);
    return (r == 2) ? 0 : -1;
}

static int test_fixture(const char* name, const char* source_path,
                        const char* expected_path,
                        const char* expected_car, const char* expected_cdr)
{
    printf("\n[%s]\n", name);

    size_t src_len;
    char* src = read_file(source_path, &src_len);
    if (!src) { TEST("read source", 0); return -1; }
    TEST("read source", 1);

    size_t payload_len;
    const char* payload = loader_strip(src, src_len, &payload_len);
    TEST("strip carrier", payload != NULL);

    Node* ast = parse(payload);
    if (!ast) { TEST("parse", 0); free(src); return -1; }
    TEST("parse", 1);
    TEST("AST is PAIR", ast->type == PAIR);
    TEST("car is ATOM", ast->car && ast->car->type == ATOM);
    TEST("cdr is ATOM", ast->cdr && ast->cdr->type == ATOM);

    if (ast->type == PAIR && ast->car && ast->cdr) {
        int car_ok = ast->car->type == ATOM && strcmp(ast->car->sym, expected_car) == 0;
        int cdr_ok = ast->cdr->type == ATOM && strcmp(ast->cdr->sym, expected_cdr) == 0;
        TEST("car matches expected symbol", car_ok);
        TEST("cdr matches expected symbol", cdr_ok);
    }

    uint16_t prog[65536];
    int len = compile(ast, prog, 65536);
    TEST("compile succeeded", len > 0);

    OMI_CPU cpu;
    init_cpu(&cpu);
    boot(&cpu);
    run(&cpu, prog, len);
    TEST("VM halted", cpu.FLAGS & HALT_FLAG);

    uint32_t exp_r0, exp_delta;
    int read_ok = read_receipt(expected_path, &exp_r0, &exp_delta);
    TEST("read expected receipt", read_ok == 0);

    if (read_ok == 0) {
        char actual[128];
        snprintf(actual, sizeof(actual), "R0: %u\ndelta: %u\nhalted: yes",
                 cpu.R[0], cpu.delta_acc);
        printf("    Actual:   R0=%u delta=%u\n", cpu.R[0], cpu.delta_acc);
        printf("    Expected: R0=%u delta=%u\n", exp_r0, exp_delta);
        TEST("R0 matches expected", cpu.R[0] == exp_r0);
        TEST("delta matches expected", cpu.delta_acc == exp_delta);
    }

    fclose(cpu.log);
    free_ast(ast);
    free(src);
    return 0;
}

int main(void) {
    printf("=== OMI-Lisp Handoff Tests ===\n");
    printf("(local compatibility parser + VM)\n");

    test_fixture("seed.omi -- (NULL . NULL)",
                 "handoff/omi-lisp/source/seed.omi",
                 "handoff/omi-lisp/expected/seed.receipt.txt",
                 "NULL", "NULL");

    test_fixture("pair.omi -- (omi . imo)",
                 "handoff/omi-lisp/source/pair.omi",
                 "handoff/omi-lisp/expected/pair.receipt.txt",
                 "omi", "imo");

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
