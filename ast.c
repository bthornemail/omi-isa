#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* make_atom(const char* s){
    Node* n = malloc(sizeof(Node));
    memset(n,0,sizeof(Node));
    n->type = ATOM;
    strncpy(n->sym,s,63);
    return n;
}

Node* make_pair(Node* a, Node* b){
    Node* n = malloc(sizeof(Node));
    memset(n,0,sizeof(Node));
    n->type = PAIR;
    n->car = a;
    n->cdr = b;
    return n;
}

void free_ast(Node* n){
    if(!n) return;
    if(n->type == PAIR){
        free_ast(n->car);
        free_ast(n->cdr);
    }
    free(n);
}

void print_ast(Node* n){
    if(!n){ printf("NULL"); return; }
    if(n->type == ATOM){
        printf("%s", n->sym);
    } else {
        printf("(");
        print_ast(n->car);
        printf(" . ");
        print_ast(n->cdr);
        printf(")");
    }
}
