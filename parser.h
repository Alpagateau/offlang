#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "symbol_table.h"
#include "types.h"

typedef struct {
  bool is_ok;
  size_t line_num;
  size_t char_num;
  union {
    anode_t *result;
    type_t  *type;
    const char *err;
  };
} parse_result;

typedef struct {
  lexer_t* l;
  arena_t* a;
  stl_t* stl;

  int current_table;

}parser_ctx;

bool parser_expect(parser_ctx *ctx, lexem_kind_t type);

bool parser_is_type_token(parser_ctx *ctx);
primitive_type_t parser_token_to_type(lexem_kind_t k);

//Error handling
parse_result parser_new_error(parser_ctx *ctx, const char *err, ...);

// Simple expressions
parse_result parse_integer(parser_ctx *ctx);
parse_result parse_variable(parser_ctx *ctx);
parse_result parse_bi_op_factor(parser_ctx *ctx);
parse_result parse_bi_op_term(parser_ctx *ctx);

parse_result parse_expr(parser_ctx *ctx);

// Type system
parse_result parse_type(parser_ctx *ctx);

// Statements
parse_result parse_declaration(parser_ctx *ctx);
parse_result parse_assignment(parser_ctx *ctx);
parse_result parse_if(parser_ctx *ctx);
parse_result parse_while(parser_ctx *ctx);
parse_result parse_defer(parser_ctx *ctx);
parse_result parse_return(parser_ctx *ctx);
parse_result parse_block(parser_ctx *ctx);

parse_result parse_statement(parser_ctx *ctx);

// Func definition
parse_result parse_func_def(parser_ctx *ctx);

//Compile unit
parse_result parse_compile_unit(parser_ctx *ctx);

#endif
