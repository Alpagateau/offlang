#include "parser.h"
#include "arena.h"
#include "ast.h"
#include "errors.h"
#include "lexer.h"
#include "symbol_table.h"
#include "types.h"
#include <stdarg.h>
#include <string.h>

#define PARSE_LOG(msg, ...)                                                    \
  printf("[PARSE DEBUG (%d)]" msg "\n", __LINE__, ##__VA_ARGS__)

bool parser_expect(parser_ctx *ctx, lexem_kind_t type) {
  lexem_t lex = lexer_peek_token(ctx->l);
  if (lex.kind != type) {
    return false;
  }
  return lex.kind == type;
}

bool parser_is_type_token(parser_ctx *ctx) {
  lexem_t lex = lexer_peek_token(ctx->l);

  return lex.kind == LX_TYPE_CHAR    || lex.kind == LX_TYPE_UCHAR  ||
         lex.kind == LX_TYPE_SHORT   || lex.kind == LX_TYPE_USHORT ||
         lex.kind == LX_TYPE_INT     || lex.kind == LX_TYPE_UINT   ||
         lex.kind == LX_TYPE_LONG    || lex.kind == LX_TYPE_ULONG  ||
         lex.kind == LX_TYPE_LLONG   || lex.kind == LX_TYPE_ULLONG ||
         lex.kind == LX_TYPE_FLOAT   || lex.kind == LX_TYPE_DOUBLE ||
         lex.kind == LX_TYPE_LDOUBLE ||
         lex.kind == LX_TYPE_BOOL    || lex.kind == LX_TYPE_VOID;
}

primitive_type_t parser_token_to_type(lexem_kind_t k)
{
  switch(k)
  {
    case LX_TYPE_CHAR: return T_CHAR;
    case LX_TYPE_UCHAR: return T_UCHAR;
    case LX_TYPE_SHORT: return T_SHORT;
    case LX_TYPE_USHORT: return T_USHORT;
    case LX_TYPE_INT: return T_INT;
    case LX_TYPE_UINT: return T_UINT;
    case LX_TYPE_LONG: return T_LONG;
    case LX_TYPE_ULONG: return T_ULONG;
    case LX_TYPE_LLONG: return T_LLONG;
    case LX_TYPE_ULLONG: return T_ULLONG;
    case LX_TYPE_FLOAT: return T_FLOAT;
    case LX_TYPE_DOUBLE: return T_DOUBLE;
    case LX_TYPE_LDOUBLE: return T_LDOUBLE;
    case LX_TYPE_BOOL: return T_BOOL;
    case LX_TYPE_VOID: return T_VOID;
    default:
    {
      printf("[SHOULD NOT HAPPEN]\n");
      return T_INT;
    }
  }
}

parse_result parse_integer(parser_ctx *ctx) {
  parse_result res;
  if (parser_expect(ctx, LX_INT)) {
    res.is_ok = true;
    lexem_t lex = lexer_consume_token(ctx->l);
    res.result = ast_new_int(ctx->a, lex.ival);
    ast_set_pos(res.result, lexer_get_line(ctx->l), lexer_get_char(ctx->l));
  } else {
    res = parser_new_error(ctx, "Could not parse int, got (%d) instead of (%d)",
                           lexer_peek_token(ctx->l).kind, LX_INT);
  }
  return res;
}

parse_result parser_new_error(parser_ctx *ctx, const char *err, ...) {
  va_list args;
  va_start(args, err);
  error_t error = vnew_error(lexer_get_line(ctx->l), lexer_get_char(ctx->l),
                             ERR_PARSER, ERR_ERR, err, args);
  arena_new_err(ctx->a, error);
  return (parse_result){.line_num = lexer_get_line(ctx->l),
                        .char_num = lexer_get_char(ctx->l),
                        .err = err};
}

parse_result parse_variable(parser_ctx *ctx) {
  parse_result res;
  if (parser_expect(ctx, LX_IDENT)) {
    res.is_ok = true;
    lexem_t lex = lexer_consume_token(ctx->l);

    sp_t symbol = search_symbol(ctx->stl, ctx->current_table, lex.identifier);

    if (symbol.table < 0 || symbol.idx < 0) {
      parser_new_error(ctx, "variable %s not declared", lex.identifier);
    }

    res.result = ast_new_var(ctx->a, symbol);
    ast_set_pos(res.result, lexer_get_line(ctx->l), lexer_get_char(ctx->l));
  } else {
    res = parser_new_error(ctx, "Not an identifier (%d)",
                           lexer_peek_token(ctx->l).kind);
  }
  return res;
}

parse_result parse_bi_op_factor(parser_ctx *ctx) {
  parse_result res;
  lexem_t lex = lexer_peek_token(ctx->l);

  if (lex.kind == LX_IDENT) {
    res = parse_variable(ctx);
  } else if (lex.kind == LX_INT) {
    res = parse_integer(ctx);
  } else if (lex.kind == LX_OPAR) {
    lexer_consume_token(ctx->l);
    res = parse_expr(ctx);
    if (!res.is_ok)
      return res;
    if (!parser_expect(ctx, LX_CPAR)) {
      res = parser_new_error(ctx, "Need to close the parenthesis");
    } else {
      lexer_consume_token(ctx->l);
    }
  } else if (lex.kind == LX_OP) {
    lexem_t tok = lexer_consume_token(ctx->l);
    parse_result sub = parse_bi_op_factor(ctx);
    if (!sub.is_ok)
      return sub;
    res.is_ok = true;
    res.result = ast_new_unary(ctx->a, tok.op, sub.result);
  } else {
    res = parser_new_error(ctx, "Could not parse factor");
  }

  while(parser_expect(ctx, LX_OPAR) || parser_expect(ctx, LX_OSQR_BRA))
  {
    if(parser_expect(ctx, LX_OSQR_BRA))
    {
      // indexing
      lexer_consume_token(ctx->l);
      parse_result index = parse_expr(ctx);
      if(!index.is_ok)
        return index;
      res.result = ast_new_index(ctx->a, res.result, index.result);
      if(!parser_expect(ctx, LX_CSQR_BRA))
        return parser_new_error(ctx, "Need a ']' to close an index");
      lexer_consume_token(ctx->l);
      continue;
    }
    if(parser_expect(ctx, LX_OPAR))
    {
      lexer_consume_token(ctx->l);
      list_t* lst = list_new(ctx->a);
      
      while(!parser_expect(ctx, LX_CPAR))
      {
        parse_result expr = parse_expr(ctx);
        if(!expr.is_ok)
          return expr;
        list_append(lst, expr.result);
        if(!parser_expect(ctx, LX_CPAR))
        {
          if(!parser_expect(ctx, LX_COMMA))
            return parser_new_error(ctx, "Missing comma");
          lexer_consume_token(ctx->l);
        }
        else
        {
          break;
        }
      }
      lexer_consume_token(ctx->l);
      res.result = ast_new_fcall(ctx->a, res.result, lst);
    }
  }

  return res;
}

parse_result parse_bi_op_term(parser_ctx *ctx) {
  parse_result left = parse_bi_op_factor(ctx);
  if (!left.is_ok)
    return left;

  while (true) {
    lexem_t op_tok = lexer_peek_token(ctx->l);

    if (op_tok.kind != LX_OP || (op_tok.op != OP_DIV && op_tok.op != OP_MUL))
      break;

    lexer_consume_token(ctx->l);

    parse_result right = parse_bi_op_factor(ctx);
    if (!right.is_ok)
      return right;

    left.result = ast_new_binary(ctx->a, op_tok.op, left.result, right.result);
  }
  left.line_num = lexer_get_line(ctx->l);
  left.char_num = lexer_get_char(ctx->l);

  return left;
}

parse_result parse_expr(parser_ctx *ctx) {
  parse_result left = parse_bi_op_term(ctx);
  if (!left.is_ok)
    return left;

  while (true) {
    lexem_t op_tok = lexer_peek_token(ctx->l);

    if (op_tok.kind != LX_OP || (op_tok.op != OP_ADD && op_tok.op != OP_SUB))
      break;

    lexer_consume_token(ctx->l);

    parse_result right = parse_bi_op_term(ctx);
    if (!right.is_ok)
      return right;

    left.result = ast_new_binary(ctx->a, op_tok.op, left.result, right.result);
  }

  return left;
}

parse_result parse_type(parser_ctx *ctx) {
  if (!parser_is_type_token(ctx)) {
    return parser_new_error(ctx, "Need a typename for a type");
  }
  parse_result res;
  lexem_t type_token = lexer_consume_token(ctx->l);
  type_t *t = type_new_primary(parser_token_to_type(type_token.kind));
  if (parser_expect(ctx, LX_LT)) {
    lexer_consume_token(ctx->l);
    parse_result bmin = parse_expr(ctx);
    if (!bmin.is_ok)
      return bmin;

    if (!parser_expect(ctx, LX_COMMA))
      return parser_new_error(
          ctx, "For bounded types, bounds are separated by commas");
    lexer_consume_token(ctx->l);

    parse_result bmax = parse_expr(ctx);

    if (!parser_expect(ctx, LX_GT))
      return parser_new_error(ctx, "End the boundary by the '>' character");
    lexer_consume_token(ctx->l);

    bmin.result = ast_simplify_expr(ctx->a, bmin.result);
    bmax.result = ast_simplify_expr(ctx->a, bmax.result);

    if (ast_get_type(bmin.result) != INT_LIT ||
        ast_get_type(bmax.result) != INT_LIT)
      return parser_new_error(ctx, "Boudaries need to be integer expressions");
    int min = ast_get_ival(bmin.result);
    int max = ast_get_ival(bmax.result);
    t = type_new_bounded(t, min, max);
  }

  while (parser_expect(ctx, LX_CARAT)) {
    t = type_new_pointer(t);
    lexer_consume_token(ctx->l);
  }

  res.is_ok = true;
  res.type = t;
  return res;
}

parse_result parse_declaration(parser_ctx *ctx) {
  parse_result res;
  parse_result t = parse_type(ctx);
  if (!t.is_ok)
    return t;

  if (!parser_expect(ctx, LX_IDENT))
    return parser_new_error(ctx, "Need a name to declare a variable");

  lexem_t name = lexer_consume_token(ctx->l);

  anode_t *expr_n = NULL;
  if (parser_expect(ctx, LX_EQ)) {
    lexer_consume_token(ctx->l);
    parse_result expr = parse_expr(ctx);
    if (!expr.is_ok)
      return expr;

    expr_n = expr.result;
  }

  if (!parser_expect(ctx, LX_SEMICOLUMN))
    return parser_new_error(ctx, "[decl] Missing semicolumn");
  else
    lexer_consume_token(ctx->l);

  symbol_t s = (symbol_t){.line_num = lexer_get_line(ctx->l),
                          .char_num = lexer_get_char(ctx->l),
                          .type = t.type};
  strcpy(s.identifier, name.identifier);
  sp_t symbol = symbol_add(ctx->stl, ctx->current_table, s);
  res.is_ok = true;
  res.result = ast_new_declaration(ctx->a, symbol, expr_n);
  res.char_num = lexer_get_char(ctx->l);
  res.line_num = lexer_get_line(ctx->l);
  return res;
}

parse_result parse_assignment(parser_ctx *ctx) {
  parse_result left = parse_expr(ctx);
  if (!left.is_ok)
    return left;

  if (!parser_expect(ctx, LX_EQ))
    return parser_new_error(ctx, "Need '=' for assignment ");

  lexer_consume_token(ctx->l);
  parse_result right = parse_expr(ctx);
  if (!right.is_ok)
    return right;

  if (!parser_expect(ctx, LX_SEMICOLUMN))
    return parser_new_error(ctx, "[assi] Missing semicolumn");
  else
    lexer_consume_token(ctx->l);

  parse_result res;
  res.is_ok = true;
  res.char_num = lexer_get_char(ctx->l);
  res.line_num = lexer_get_line(ctx->l);
  res.result = ast_new_assigment(ctx->a, left.result, right.result);
  return res;
}

parse_result parse_if(parser_ctx *ctx) {
  parse_result res;
  if (!parser_expect(ctx, LX_IF))
    return parser_new_error(ctx, "No if for an if statement ?!");

  lexer_consume_token(ctx->l);
  parse_result cond = parse_expr(ctx);
  if (!cond.is_ok)
    return cond;
  parse_result statement = parse_statement(ctx);
  if (!statement.is_ok)
    return statement;

  res.is_ok = true;

  anode_t *otherwise = NULL;
  if (parser_expect(ctx, LX_ELSE)) {
    lexer_consume_token(ctx->l);
    parse_result stmt = parse_statement(ctx);
    if (!stmt.is_ok)
      return stmt;
    otherwise = stmt.result;
  }

  res.result =
      ast_new_if_branch(ctx->a, cond.result, statement.result, otherwise);
  res.line_num = lexer_get_line(ctx->l);
  res.char_num = lexer_get_char(ctx->l);

  return res;
}

parse_result parse_while(parser_ctx *ctx) {

  parse_result res;
  if (!parser_expect(ctx, LX_WHILE))
    return parser_new_error(ctx, "No while for a WHILE ?!");

  lexer_consume_token(ctx->l);
  parse_result cond = parse_expr(ctx);
  if (!cond.is_ok)
    return cond;
  parse_result statement = parse_statement(ctx);
  if (!statement.is_ok)
    return statement;

  res.is_ok = true;
  res.result = ast_new_while_loop(ctx->a, cond.result, statement.result);
  res.line_num = lexer_get_line(ctx->l);
  res.char_num = lexer_get_char(ctx->l);

  return res;
}

parse_result parse_defer(parser_ctx *ctx) {
  if (!parser_expect(ctx, LX_DEFER))
    return parser_new_error(ctx, "No defer");

  lexem_t defer = lexer_consume_token(ctx->l);
  parse_result stmt = parse_statement(ctx);
  if (!stmt.is_ok)
    return stmt;
  parse_result res;
  res.is_ok = true;
  res.char_num = defer.char_num;
  res.line_num = defer.line_num;
  res.result = ast_new_defer(ctx->a, stmt.result);
  return res;
}

parse_result parse_return(parser_ctx *ctx) {
  if (!parser_expect(ctx, LX_RETURN))
    return parser_new_error(ctx, "No return");
  lexer_consume_token(ctx->l);
  parse_result expr = parse_expr(ctx);
  if (!expr.is_ok)
    return expr;

  if (!parser_expect(ctx, LX_SEMICOLUMN))
    return parser_new_error(ctx, "[assi] Missing semicolumn");
  else
    lexer_consume_token(ctx->l);

  parse_result res;
  res.is_ok = true;
  res.char_num = expr.char_num;
  res.line_num = expr.line_num;
  res.result = ast_new_return(ctx->a, expr.result);
  return res;
}

parse_result parse_block(parser_ctx *ctx) {

  int old_table = ctx->current_table;

  if (!parser_expect(ctx, LX_OCRL_BRA))
    return parser_new_error(ctx, "No '{'");

  lexer_consume_token(ctx->l);
  parse_result res;
  list_t *list = list_new(ctx->a);

  ctx->current_table = symbol_table_new(ctx->stl, ctx->current_table);

  while (lexer_peek_token(ctx->l).kind != LX_CCRL_BRA) {
    parse_result stmt = parse_statement(ctx);
    if (!stmt.is_ok) {
      printf("[PARSER ERR <%zu:%zu>] %s\n", stmt.line_num, stmt.char_num,
             stmt.err);
      lexer_throw_till_break(ctx->l);
      continue;
    }
    list_append(list, stmt.result);
  }

  lexer_consume_token(ctx->l);
  res.result = ast_new_block(ctx->a, list);
  res.is_ok = true;
  res.line_num = lexer_get_line(ctx->l);
  res.char_num = lexer_get_char(ctx->l);

  ctx->current_table = old_table;
  return res;
}

parse_result parse_statement(parser_ctx *ctx) {
  parse_result res;
  lexem_t tok = lexer_peek_token(ctx->l);

  if (parser_is_type_token(ctx)) {
    res = parse_declaration(ctx);
  } else if (tok.kind == LX_IF) {
    res = parse_if(ctx);
  } else if (tok.kind == LX_WHILE) {
    res = parse_while(ctx);
  } else if (tok.kind == LX_DEFER) {
    res = parse_defer(ctx);
  } else if (tok.kind == LX_OCRL_BRA) {
    res = parse_block(ctx);
  } else if (tok.kind == LX_RETURN) {
    res = parse_return(ctx);
  } else {
    res = parse_assignment(ctx);
  }
  return res;
}

parse_result parse_func_def(parser_ctx *ctx) {
  if (!parser_expect(ctx, LX_FUNC))
    return parser_new_error(ctx, "Need 'func' or 'fn' to define a function");
  lexer_consume_token(ctx->l);

  if (!parser_expect(ctx, LX_IDENT))
    return parser_new_error(ctx, "Need identifier after 'fn' or 'func'");
  lexem_t name = lexer_consume_token(ctx->l);

  int old_table = ctx->current_table;
  ctx->current_table = symbol_table_new(ctx->stl, ctx->current_table);
  symbol_t sym = (symbol_t){.kind = SYM_FUNC,
                            .line_num = lexer_get_line(ctx->l),
                            .char_num = lexer_get_char(ctx->l),
                            .info.child_table = ctx->current_table
                           };

  strcpy(sym.identifier, name.identifier);

  if (!parser_expect(ctx, LX_OPAR))
    return parser_new_error(ctx, "Need parenthesis to define a function");
  lexer_consume_token(ctx->l);


  type_t *t = type_new_func();

  while (!parser_expect(ctx, LX_CPAR)) {
    if (t->func.len > 0) {
      if (!parser_expect(ctx, LX_COMMA))
        return parser_new_error(ctx, "Need a comma after each argument");

      lexer_consume_token(ctx->l);
    }

    parse_result type_arg = parse_type(ctx);
    if (!type_arg.is_ok) {
      return type_arg;
    }

    if (!parser_expect(ctx, LX_IDENT))
      return parser_new_error(ctx, "Need an identifier");

    lexem_t ident = lexer_consume_token(ctx->l);

    // Add the type to the func type
    type_func_add_arg(t, type_arg.type);

    // Create the symbol for the func args
    symbol_t arg_sym = (symbol_t){.kind = SYM_VARIABLE,
                                  .type = type_copy(type_arg.type),
                                  .line_num = lexer_get_line(ctx->l),
                                  .char_num = lexer_get_char(ctx->l)};
    strcpy(arg_sym.identifier, ident.identifier);
    symbol_add(ctx->stl, ctx->current_table, arg_sym);
  }
  lexer_consume_token(ctx->l);

  if (parser_expect(ctx, LX_COLUMN)) {
    lexer_consume_token(ctx->l);
    parse_result p = parse_type(ctx);
    if (!p.is_ok)
      return p;
    type_func_set_ret(t, p.type);
  } else {
    type_func_set_ret(t, type_new_primary(T_VOID));
  }

  sym.type = t;
  sp_t s = symbol_add(ctx->stl, ctx->current_table, sym);

  parse_result stmt = parse_statement(ctx);
  if (!stmt.is_ok)
    return stmt;

  ctx->current_table = old_table;

  parse_result res;
  res.is_ok = true;
  res.result = ast_new_func_def(ctx->a, s, stmt.result);
  return res;
}

parse_result parse_compile_unit(parser_ctx *ctx)
{
  list_t *lst = list_new(ctx->a);
  while(parser_expect(ctx, LX_FUNC))
  {
    parse_result f = parse_func_def(ctx);
    if(!f.is_ok)
      return f;
    list_append(lst, f.result);
  }
  parse_result res;
  res.is_ok = true;
  res.result = ast_new_compile_unit(ctx->a, lst);
  return res;
}
