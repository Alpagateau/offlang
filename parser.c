#include "parser.h"
#include "ast.h"
#include "lexer.h"

arena_t *arena = NULL;

#define PARSE_LOG(msg, ...)                                                    \
  printf("[PARSE DEBUG (%d)]" msg "\n", __LINE__, ##__VA_ARGS__)

void parser_set_arena(arena_t *a) { arena = a; }

bool parser_expect(lexer_t *l, lexem_kind_t type) {
  lexem_t lex = lexer_peek_token(l);
  if (lex.kind != type) {
    PARSE_LOG("Expected (%d) but got (%d)", type, lex.kind);
  }
  return lex.kind == type;
}

parse_result parse_integer(lexer_t *l) {
  parse_result res;
  if (parser_expect(l, LX_INT)) {
    res.is_ok = true;
    lexem_t lex = lexer_consume_token(l);
    res.result = ast_new_int(arena, lex.ival);
    ast_set_pos(res.result, l->line_num, l->char_num);
    PARSE_LOG("parsed int (%d)", lex.ival);
  } else {
    res.is_ok = false;
    res.err = "Could not parse int";
    res.line_num = l->line_num;
    res.char_num = l->char_num;
  }
  return res;
}

parse_result parse_variable(lexer_t *l) {
  parse_result res;
  if (parser_expect(l, LX_IDENT)) {
    res.is_ok = true;
    lexem_t lex = lexer_consume_token(l);
    res.result = ast_new_var(arena, lex.identifier);
    ast_set_pos(res.result, l->line_num, l->char_num);
    PARSE_LOG("ident (%s)", lex.identifier);
  } else {
    res.is_ok = false;
    res.err = "Could not parse variable";
    res.line_num = l->line_num;
    res.char_num = l->char_num;
  }
  return res;
}

parse_result parse_bi_op_factor(lexer_t *l) {
  parse_result res;
  lexem_t lex = lexer_peek_token(l);

  if (lex.kind == LX_IDENT) {
    res = parse_variable(l);
  } else if (lex.kind == LX_INT) {
    res = parse_integer(l);
  } else if (lex.kind == LX_OPAR) {
    lexer_consume_token(l);
    res = parse_expr(l);
    if (!res.is_ok)
      return res;
    if (!parser_expect(l, LX_CPAR)) {
      PARSE_LOG("Parenthesis prblm; got : %d", lexer_peek_token(l).kind);
      res.is_ok = false;
      res.err = "Need to close the parenthesis";
      res.line_num = l->line_num;
      res.char_num = l->char_num;
    } else {
      lexer_consume_token(l);
    }
  } else {
    PARSE_LOG("factor err : got type (%d)", lex.kind);
    lexer_print_lexem(lex);
    res.is_ok = false;
    res.err = "Could not parse factor";
    res.line_num = l->line_num;
    res.char_num = l->char_num;
  }
  return res;
}

parse_result parse_bi_op_term(lexer_t *l) {
  parse_result left = parse_bi_op_factor(l);
  if (!left.is_ok)
    return left;

  while (true) {
    lexem_t op_tok = lexer_peek_token(l);

    if (op_tok.kind != LX_OP || (op_tok.op != OP_DIV && op_tok.op != OP_MUL))
      break;

    lexer_consume_token(l);

    parse_result right = parse_bi_op_factor(l);
    if (!right.is_ok)
      return right;

    left.result = ast_new_binary(arena, op_tok.op, left.result, right.result);
  }

  return left;
}

parse_result parse_expr(lexer_t *l) {
  parse_result left = parse_bi_op_term(l);
  if (!left.is_ok)
    return left;

  while (true) {
    lexem_t op_tok = lexer_peek_token(l);

    if (op_tok.kind != LX_OP || (op_tok.op != OP_ADD && op_tok.op != OP_SUB))
      break;

    lexer_consume_token(l);

    parse_result right = parse_bi_op_term(l);
    if (!right.is_ok)
      return right;

    left.result = ast_new_binary(arena, op_tok.op, left.result, right.result);
  }

  return left;
}

parse_result parse_declaration(lexer_t *l) {
  parse_result res;
  if (!parser_expect(l, LX_TYPE_INT)) {
    res.is_ok = false;
    res.char_num = l->char_num;
    res.line_num = l->line_num;
    res.err = "Need a type to declare a variable";
  }

  lexer_consume_token(l);

  if (!parser_expect(l, LX_IDENT)) {
    res.is_ok = false;
    res.char_num = l->char_num;
    res.line_num = l->line_num;
    res.err = "Need a name to declare a variable";
  }

  lexem_t name = lexer_consume_token(l);

  anode_t *expr_n = NULL;
  if (parser_expect(l, LX_EQ)) {
    lexer_consume_token(l);
    parse_result expr = parse_expr(l);
    if (!expr.is_ok)
      return expr;

    expr_n = expr.result;
  }
  res.is_ok = true;
  res.result = ast_new_declaration(arena, name.identifier, expr_n);

  return res;
}

parse_result parse_assignment(lexer_t *l) {
  parse_result left = parse_expr(l);
  if (!left.is_ok)
    return left;

  if (!parser_expect(l, LX_EQ)) {
    parse_result res = {.is_ok = false,
                        .line_num = l->line_num,
                        .char_num = l->char_num,
                        .err = "Need \'=\' for assignment"};
    return res;
  }

  lexer_consume_token(l);
  parse_result right = parse_expr(l);
  if (!right.is_ok)
    return right;

  parse_result res;
  res.is_ok = true;
  res.char_num = left.char_num;
  res.line_num = left.line_num;
  res.result = ast_new_assigment(arena, left.result, right.result);
  return res;
}

parse_result parse_if(lexer_t *l) {

  parse_result res;
  if (!parser_expect(l, LX_IF)) {
    return (parse_result){
        .is_ok = false,
        .char_num = l->char_num,
        .line_num = l->line_num,
    };
  }

  lexer_consume_token(l);
  parse_result cond = parse_expr(l);
  if (!cond.is_ok)
    return cond;
  //printf("cond (%d): ", ast_get_type(cond.result));
  //print_ast(cond.result);
  //printf("\n");
  parse_result statement = parse_statement(l);
  if (!statement.is_ok)
    return statement;

  res.is_ok = true;

  anode_t *otherwise = NULL;
  if (parser_expect(l, LX_ELSE)) {
    lexer_consume_token(l);
    parse_result stmt = parse_statement(l);
    if (!stmt.is_ok)
      return stmt;
    otherwise = stmt.result;
  }

  res.result =
      ast_new_if_branch(arena, cond.result, statement.result, otherwise);

  return res;
}

parse_result parse_while(lexer_t *l) {

  parse_result res;
  if (!parser_expect(l, LX_WHILE)) {
    return (parse_result){
        .is_ok = false,
        .char_num = l->char_num,
        .line_num = l->line_num,
    };
  }

  lexer_consume_token(l);
  parse_result cond = parse_expr(l);
  if (!cond.is_ok)
    return cond;
  //printf("cond (%d): ", ast_get_type(cond.result));
  //print_ast(cond.result);
  //printf("\n");
  parse_result statement = parse_statement(l);
  if (!statement.is_ok)
    return statement;

  res.is_ok = true;
  res.result = ast_new_while_loop(arena, cond.result, statement.result);
  return res;
}

parse_result parse_defer(lexer_t *l) {
  if (!parser_expect(l, LX_DEFER))
    return (parse_result){.is_ok = false, .err = "No defer"};

  lexem_t defer = lexer_consume_token(l);
  parse_result stmt = parse_statement(l);
  if (!stmt.is_ok)
    return stmt;
  parse_result res;
  res.is_ok = true;
  res.char_num = defer.char_num;
  res.line_num = defer.line_num;
  res.result = ast_new_defer(arena, stmt.result);
  return res;
}

parse_result parse_block(lexer_t* l)
{
  if(!parser_expect(l, LX_OCRL_BRA))
    return (parse_result){.is_ok = false, .err = "No '{'"};

  lexer_consume_token(l);
  parse_result res;
  list_t* list = list_new();

  while(lexer_peek_token(l).kind != LX_CCRL_BRA)
  {
    parse_result stmt = parse_statement(l);
    if(!stmt.is_ok)
      return stmt;
    list_append(list, stmt.result);
  }

  lexer_consume_token(l);
  res.result = ast_new_block(arena, list);
  res.is_ok = true;
  return res;
}

parse_result parse_statement(lexer_t *l) {
  parse_result res;
  lexem_t tok = lexer_peek_token(l);

  if (tok.kind == LX_TYPE_INT) {
    res = parse_declaration(l);
  } else if (tok.kind == LX_IF) {
    res = parse_if(l);
  } else if (tok.kind == LX_WHILE) {
    res = parse_while(l);
  } else if (tok.kind == LX_DEFER) {
    res = parse_defer(l);
  } else if (tok.kind == LX_OCRL_BRA){
    res = parse_block(l);
  } else {
    res = parse_assignment(l);
  }
  return res;
}
