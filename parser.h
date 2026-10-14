#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

typedef struct {
  bool is_ok;
  size_t line_num;
  size_t char_num;
  union {
    anode_t *result;
    const char *err;
  };
} parse_result;

void parser_set_arena(arena_t *a);
bool parser_expect(lexer_t *l, lexem_kind_t type);

//Error handling
parse_result parser_new_error(lexer_t *l, const char *err);

// Simple expressions
parse_result parse_integer(lexer_t *l);
parse_result parse_variable(lexer_t *l);
parse_result parse_bi_op_factor(lexer_t *l);
parse_result parse_bi_op_term(lexer_t *l);

parse_result parse_expr(lexer_t *l);

// Type system
parse_result parse_type(lexer_t* l);

// Statements
parse_result parse_declaration(lexer_t *l);
parse_result parse_assignment(lexer_t *l);
parse_result parse_if(lexer_t *l);
parse_result parse_while(lexer_t* l);
parse_result parse_defer(lexer_t* l);
parse_result parse_block(lexer_t* l);

parse_result parse_statement(lexer_t *l);

#endif
