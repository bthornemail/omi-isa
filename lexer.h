#ifndef OMI_LEXER_H
#define OMI_LEXER_H

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

void init_lexer(const char* s);
Token next_token();
Token peek_token();

#endif
