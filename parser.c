#include "lexer.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

extern int pos;

Node* parse_expr();

static Token consume(TokenType expected){
    Token t = next_token();
    if(t.type != expected){
        fprintf(stderr,"parse error: expected token type %d got %d\n",expected,t.type);
        exit(1);
    }
    return t;
}

Node* parse_atom(){
    Token t = consume(TOK_SYMBOL);
    return make_atom(t.text);
}

Node* parse_pair(){
    consume(TOK_LPAREN);
    Node* car = parse_expr();
    consume(TOK_DOT);
    Node* cdr = parse_expr();
    consume(TOK_RPAREN);
    return make_pair(car,cdr);
}

Node* parse_expr(){
    Token t = peek_token();
    switch(t.type){
        case TOK_LPAREN: return parse_pair();
        case TOK_SYMBOL: return parse_atom();
        default: return NULL;
    }
}

Node* parse(const char* src_text){
    init_lexer(src_text);
    pos = 0;
    return parse_expr();
}
