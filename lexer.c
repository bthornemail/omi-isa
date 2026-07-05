#include "lexer.h"
#include <ctype.h>
#include <string.h>

const char* src;
int pos = 0;

void init_lexer(const char* s){
    src = s;
    pos = 0;
}

static char peek(){
    return src[pos];
}

static char advance(){
    return src[pos++];
}

static void skip_ws(){
    while(isspace((unsigned char)peek())) advance();
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
    while(isalnum((unsigned char)peek()) || peek()=='_' || peek()=='-' || peek()=='?' || peek()=='@' || peek()=='/'){
        t.text[i++]=advance();
    }
    t.type = TOK_SYMBOL;
    return t;
}

Token peek_token(){
    int saved = pos;
    Token t = next_token();
    pos = saved;
    return t;
}
