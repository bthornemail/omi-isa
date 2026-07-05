#ifndef OMI_AST_H
#define OMI_AST_H

#include <stdint.h>

typedef struct Node {
    enum { ATOM, PAIR } type;
    char sym[64];
    struct Node* car;
    struct Node* cdr;
} Node;

Node* make_atom(const char* s);
Node* make_pair(Node* a, Node* b);
void free_ast(Node* n);
void print_ast(Node* n);

#endif
