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
    res.line_num = lexer_get_line(l);
    res.char_num = lexer_get_line(l);
  }
  return res;
}

parse_result parser_new_error(lexer_t *l, const char *err)
{
  return (parse_result){
    .line_num = lexer_get_line(l),
    .char_num = lexer_get_char(l),
    .err = err
  };
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
    res = parser_new_error(l, "Not an identifier");
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
      res = parser_new_error(l, "Need to close the parenthesis");
    } else {
      lexer_consume_token(l);
    }
  } else {
    PARSE_LOG("factor err : got type (%d)", lex.kind);
    //lexer_print_lexem(lex);
    res = parser_new_error(l, "Could not parse factor");
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
  left.line_num = lexer_get_line(l);
  left.char_num = lexer_get_line(l);

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

parse_result parse_type(lexer_t* l)
{
  if(!parser_expect(l, LX_TYPE_INT))
  {
    return parser_new_error(l, "Need a typename for a type");
  }
  parse_result res;
  lexer_consume_token(l);
  ast_native_type_t nt = TYPE_INT;
  bool is_bounded = false;
  int max = 0;
  int min = 0;
  if(parser_expect(l, LX_LT))
  {
    lexer_consume_token(l);
    is_bounded = true;
    if(!parser_expect(l, LX_INT))
    {
      return parser_new_error(l, "Need to add bounds for a bounded type");
    }
    lexem_t min_token = lexer_consume_token(l); 
    
    if(!parser_expect(l, LX_COMMA))
      return parser_new_error(l, "For bounded types, bounds are separated by commas");

    lexer_consume_token(l);

    if(!parser_expect(l, LX_INT))
      return parser_new_error(l, "Need to add bounds for a bounded type");

    lexem_t max_token = lexer_consume_token(l);

    is_bounded = true;
    min = min_token.ival;
    max = max_token.ival;
    if(!parser_expect(l, LX_GT))
      return parser_new_error(l, "End the boundary by the '>' character");

    lexer_consume_token(l);

  }
  res.is_ok = true;
  res.result = ast_new_type(arena,nt,min,max,is_bounded);
  return res;
}

parse_result parse_declaration(lexer_t *l) {
  parse_result res;
  parse_result t = parse_type(l);
  if (!t.is_ok)
    return t;

  if (!parser_expect(l, LX_IDENT)) 
    return parser_new_error(l, "Need a name to declare a variable");
  
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
  res.result = ast_new_declaration(arena, name.identifier,t.result, expr_n);
  res.char_num = lexer_get_char(l);
  res.line_num = lexer_get_line(l);
  return res;
}

parse_result parse_assignment(lexer_t *l) {
  parse_result left = parse_expr(l);
  if (!left.is_ok)
    return left;

  if (!parser_expect(l, LX_EQ)) 
    return parser_new_error(l, "Need '=' for assignment ");
  
  lexer_consume_token(l);
  parse_result right = parse_expr(l);
  if (!right.is_ok)
    return right;

  parse_result res;
  res.is_ok = true;
  res.char_num = lexer_get_char(l);
  res.line_num = lexer_get_line(l);
  res.result = ast_new_assigment(arena, left.result, right.result);
  return res;
}

parse_result parse_if(lexer_t *l) {

  parse_result res;
  if (!parser_expect(l, LX_IF)) 
    return parser_new_error(l, "No if for an if statement ?!"); 

  lexer_consume_token(l);
  parse_result cond = parse_expr(l);
  if (!cond.is_ok)
    return cond;
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

  res.result   = ast_new_if_branch(arena, cond.result, statement.result, otherwise);
  res.line_num = lexer_get_line(l);
  res.char_num = lexer_get_char(l);

  return res;
}

parse_result parse_while(lexer_t *l) {

  parse_result res;
  if (!parser_expect(l, LX_WHILE))
    return parser_new_error(l, "No while for a WHILE ?!");

  lexer_consume_token(l);
  parse_result cond = parse_expr(l);
  if (!cond.is_ok)
    return cond;
  parse_result statement = parse_statement(l);
  if (!statement.is_ok)
    return statement;

  res.is_ok = true;
  res.result = ast_new_while_loop(arena, cond.result, statement.result);
  res.line_num = lexer_get_line(l);
  res.char_num = lexer_get_char(l);

  return res;
}

parse_result parse_defer(lexer_t *l) {
  if (!parser_expect(l, LX_DEFER))
    return parser_new_error(l,  "No defer");

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
    return parser_new_error(l,  "No '{'");

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
  res.line_num = lexer_get_line(l);
  res.char_num = lexer_get_char(l);
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
