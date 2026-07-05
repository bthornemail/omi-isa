
#include <stdio.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    TOK_LPAREN, TOK_RPAREN,
    TOK_DOT,
    TOK_SYMBOL,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char text[64];
} Token;

const char* src;
int pos = 0;

char peek(){ return src[pos]; }
char advance(){ return src[pos++]; }

void skip_ws(){
    while(isspace(peek())) advance();
}

Token next_token(){
    skip_ws();

    Token t;
    memset(&t,0,sizeof(t));

    char c = peek();

    if(c=='\0'){ t.type=TOK_EOF; return t; }

    if(c=='('){ advance(); t.type=TOK_LPAREN; return t; }
    if(c==')'){ advance(); t.type=TOK_RPAREN; return t; }
    if(c=='.'){ advance(); t.type=TOK_DOT; return t; }

    int i=0;
    while(isalnum(peek()) || peek()=='_' || peek()=='-'){
        t.text[i++]=advance();
    }

    t.type = TOK_SYMBOL;
    return t;
}
