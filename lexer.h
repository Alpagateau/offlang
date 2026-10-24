#ifndef LEXER_H
#define LEXER_H

#include "ast.h"
#include <stdbool.h>
#include <stdio.h>

#define LEXER_BUFFER_SIZE 8

typedef enum {
    LX_ERR = 0,
    // Basic expr
    LX_INT,
    LX_IDENT,
    LX_OP,

    // Types keywords
    LX_TYPE_CHAR,
    LX_TYPE_UCHAR,
    LX_TYPE_SHORT,
    LX_TYPE_USHORT,
    LX_TYPE_INT,
    LX_TYPE_UINT,
    LX_TYPE_LONG,
    LX_TYPE_ULONG,
    LX_TYPE_LLONG,
    LX_TYPE_ULLONG,
    LX_TYPE_FLOAT,
    LX_TYPE_DOUBLE,
    LX_TYPE_LDOUBLE,
    LX_TYPE_BOOL,
    LX_TYPE_VOID,
    // Keywords
    LX_IF,
    LX_ELSE,
    LX_WHILE,
    LX_DEFER,
    LX_RETURN,
    LX_FUNC,

    // Punctuation
    LX_COMMA,
    LX_EQ,
    LX_COLUMN,
    LX_SEMICOLUMN,
    LX_OPAR,
    LX_CPAR,
    LX_OSQR_BRA,
    LX_CSQR_BRA,
    LX_OCRL_BRA,
    LX_CCRL_BRA,
    LX_LT,
    LX_GT,
    LX_CARAT
} lexem_kind_t;

typedef struct {
    lexem_kind_t kind;
    size_t line_num;
    size_t char_num;
    union {
        int ival;
        char identifier[64];
        ast_op_t op;
    };
} lexem_t;

typedef struct {
    FILE* input_file;
    size_t line_num;
    size_t char_num;
    lexem_t next_lexems[LEXER_BUFFER_SIZE];
    size_t next_idx;
    bool in_comment;
} lexer_t;

typedef struct {
    char character;
    lexem_kind_t kind;
} char_lexem_pair_t;

typedef struct {
    const char* keyword;
    lexem_kind_t kind;
} keyword_lexem_pair_t;

void lexer_init (FILE* f, lexer_t* l);

int lexer_peek (lexer_t* l);
int lexer_consume (lexer_t* l);

bool lexer_is_operator (int c);
void lexer_consume_spaces (lexer_t* l);

lexem_t lexer_lex_integer (lexer_t* l);
lexem_t lexer_lex_kw_identifier (lexer_t* l);
lexem_t lexer_lex_operator (lexer_t* l);

lexem_t lexer_next_token (lexer_t* l);
lexem_t lexer_peek_token (lexer_t* l);
lexem_t lexer_peek_at (lexer_t* l, size_t idx);
lexem_t lexer_consume_token (lexer_t* l);
void lexer_throw_till_break (lexer_t* l);

size_t lexer_get_line (lexer_t* l);
size_t lexer_get_char (lexer_t* l);

void lexer_print_lexem (lexem_t lexem);
void lexer_emit_error (const char* err, size_t line_num, size_t char_num);

#endif
