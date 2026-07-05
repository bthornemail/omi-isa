
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    enum { ATOM, PAIR } type;
    char sym[64];
    struct Node* car;
    struct Node* cdr;
} Node;

Node* new_node(){
    Node* n = malloc(sizeof(Node));
    memset(n,0,sizeof(Node));
    return n;
}
