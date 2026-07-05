
#include "parser.c"

Node* make_atom(char* s){
    Node* n = new_node();
    n->type = ATOM;
    strcpy(n->sym,s);
    return n;
}

Node* make_pair(Node* a, Node* b){
    Node* n = new_node();
    n->type = PAIR;
    n->car = a;
    n->cdr = b;
    return n;
}
