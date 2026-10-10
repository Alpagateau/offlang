#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>
#include "ast.h"

typedef enum {
  LX_ERR = 0,
  LX_INT,
  LX_IDENT,
  LX_OPAR,
  LX_CPAR,
  LX_OP
}lexem_kind_t;

typedef struct
{
  lexem_kind_t kind;
  size_t line_num;
  size_t char_num;
  union {
    int  ival;
    char identifier[64];
    ast_op_t op;
  };
}lexem_t;

typedef struct
{
  FILE*  input_file;
  size_t line_num;
  size_t char_num;
  lexem_t next_lexem;
}lexer_t;

void lexer_init(FILE* f, lexer_t* l);

int lexer_peek(lexer_t* l);
int lexer_consume(lexer_t* l);

bool lexer_is_operator(int c);
void lexer_consume_spaces(lexer_t* l);

lexem_t lexer_lex_integer(lexer_t* l);
lexem_t lexer_lex_identifier(lexer_t* l);
lexem_t lexer_lex_operator(lexer_t* l);

lexem_t lexer_next_token(lexer_t* l);
lexem_t lexer_peek_token(lexer_t* l);
lexem_t lexer_consume_token(lexer_t* l);

void lexer_print_lexem(lexem_t lexem);
void lexer_emit_error(const char* err, size_t line_num, size_t char_num);

#endif
