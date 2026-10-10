#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
  bool is_ok;
  size_t line_num;
  size_t char_num;
  union {
    anode_t* result;
    const char* err;
  };
} parse_result;

void parser_set_arena(arena_t* a);
bool parser_expect(lexer_t* l, lexem_kind_t type);

parse_result parse_integer(lexer_t* l);
parse_result parse_variable(lexer_t* l);
parse_result parse_bi_op_factor(lexer_t* l);
parse_result parse_bi_op_term(lexer_t* l);

parse_result parse_expr(lexer_t* l);

#endif
