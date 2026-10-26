#include "ast.h"
#include "symbol_table.h"
#include "types.h"
#include <stdio.h>
#include <string.h>

struct ast_node {
  ast_type_t type;
  size_t start_line_num;
  size_t start_char_num;

  size_t end_line_num;
  size_t end_char_num;

  union {
    int ivar;
    sp_t symbol;
    type_t* type_val;

    struct {
      anode_t* lchild;
      anode_t* rchild;
      char op;
    } binary;

    struct {
      anode_t* child;
      char op;
    } unary;

    struct {
      anode_t* expr;
      sp_t symbol;
    } declaration;

    struct {
      anode_t* lval;
      anode_t* expr;
    } assignment;

    struct {
      anode_t* cond;
      anode_t* then;
      anode_t* otherwise;
    } if_branch;

    struct {
      anode_t* cond;
      anode_t* then;
    } while_loop;

    struct {
      anode_t* stmt;
    } defer;

    struct {
      anode_t* expr;
    } ret;

    struct {
      list_t* stmts;
    } block;

    struct {
      sp_t symbol;
      anode_t* stmt;
    } func_def;

    struct {
      anode_t* array;
      anode_t* index;
    } indexed;

    struct {
      anode_t* func;
      list_t* args;
    } fcall;

    struct {
      list_t* top_levels;
    } compile_unit;
  };
};

anode_t* ast_new (arena_t* arena, ast_type_t type) {
  anode_t* a = (anode_t*)arena_alloc (arena, sizeof (anode_t));
  a->type    = type;
  return a;
}

anode_t* ast_new_int (arena_t* arena, int value) {
  anode_t* n = ast_new (arena, INT_LIT);
  n->ivar    = value;
  return n;
}

anode_t* ast_new_var (arena_t* arena, sp_t symbol) {
  anode_t* n = ast_new (arena, VARIABLE);
  n->symbol  = symbol;
  return n;
}

anode_t* ast_new_binary (arena_t* arena, char op, anode_t* lc, anode_t* rc) {
  anode_t* n       = ast_new (arena, BI_OP);
  n->binary.op     = op;
  n->binary.lchild = lc;
  n->binary.rchild = rc;
  return n;
}

anode_t* ast_new_unary (arena_t* arena, char op, anode_t* child) {
  anode_t* n     = ast_new (arena, UN_OP);
  n->unary.op    = op;
  n->unary.child = child;
  return n;
}

anode_t* ast_new_index (arena_t* arena, anode_t* expr, anode_t* idx) {
  anode_t* n       = ast_new (arena, INDEXING);
  n->indexed.array = expr;
  n->indexed.index = idx;
  return n;
}

anode_t* ast_new_fcall (arena_t* arena, anode_t* callee, list_t* args) {
  anode_t* n    = ast_new (arena, CALLING);
  n->fcall.args = args;
  n->fcall.func = callee;
  return n;
}

anode_t* ast_new_declaration (arena_t* arena, sp_t symbol, anode_t* expr) {
  anode_t* n            = ast_new (arena, DECLARATION);
  n->declaration.expr   = expr;
  n->declaration.symbol = symbol;
  return n;
}

anode_t* ast_new_assigment (arena_t* arena, anode_t* lv, anode_t* expr) {
  anode_t* n         = ast_new (arena, ASSIGNMENT);
  n->assignment.lval = lv;
  n->assignment.expr = expr;
  return n;
}

anode_t* ast_new_if_branch (arena_t* arena, anode_t* cond, anode_t* then, anode_t* otherwise) {
  anode_t* n             = ast_new (arena, IF_BRANCH);
  n->if_branch.cond      = cond;
  n->if_branch.then      = then;
  n->if_branch.otherwise = otherwise;
  return n;
}

anode_t* ast_new_while_loop (arena_t* arena, anode_t* cond, anode_t* then) {
  anode_t* n         = ast_new (arena, WHILE_LOOP);
  n->while_loop.cond = cond;
  n->while_loop.then = then;
  return n;
}

anode_t* ast_new_defer (arena_t* arena, anode_t* stmt) {
  anode_t* n    = ast_new (arena, DEFER);
  n->defer.stmt = stmt;
  return n;
}

anode_t* ast_new_return (arena_t* arena, anode_t* ret) {
  anode_t* n  = ast_new (arena, RETURN);
  n->ret.expr = ret;
  return n;
}

anode_t* ast_new_block (arena_t* arena, list_t* lst) {
  anode_t* n     = ast_new (arena, BLOCK);
  n->block.stmts = lst;
  return n;
}

anode_t* ast_new_func_def (arena_t* arena, sp_t sp, anode_t* stmt) {
  anode_t* n         = ast_new (arena, FUNC_DEF);
  n->func_def.stmt   = stmt;
  n->func_def.symbol = sp;
  return n;
}

anode_t* ast_new_compile_unit (arena_t* arena, list_t* lst) {
  anode_t* n                 = ast_new (arena, COMPILE_UNIT);
  n->compile_unit.top_levels = lst;
  return n;
}

ast_type_t ast_get_type (anode_t* ast) { return ast->type; }

int ast_get_ival (anode_t* ast) {
  if (ast->type != INT_LIT) return 0;
  return ast->ivar;
}

sp_t ast_get_symbol (anode_t* ast) {
  switch (ast->type) {
  case VARIABLE: return ast->symbol;
  case DECLARATION: return ast->declaration.symbol;
  case FUNC_DEF: return ast->func_def.symbol;
  default: return (sp_t){ -1, -1 };
  }
}

anode_t* ast_get_lchild (anode_t* ast) {
  if (ast->type != BI_OP) return NULL;
  return ast->binary.lchild;
}

anode_t* ast_get_rchild (anode_t* ast) {
  if (ast->type != BI_OP) return NULL;
  return ast->binary.rchild;
}

anode_t* ast_simplify_expr (arena_t* arena, anode_t* ast) {
  switch (ast->type) {
  case INT_LIT: return ast;
  case VARIABLE: return ast;
  case BI_OP: {
    ast->binary.lchild = ast_simplify_expr (arena, ast->binary.lchild);
    ast->binary.rchild = ast_simplify_expr (arena, ast->binary.rchild);

    if (ast->binary.lchild->type == INT_LIT && ast->binary.rchild->type == INT_LIT) {
      int a = ast->binary.lchild->ivar;
      int b = ast->binary.rchild->ivar;

      int v = 0;
      switch (ast->binary.op) {
      case '+': v = a + b; break;
      case '-': v = a - b; break;
      case '*': v = a * b; break;
      case '/': v = a / b; break;
      }
      return ast_new_int (arena, v);
    }
    return ast;
  }
  case UN_OP: {
    ast->unary.child = ast_simplify_expr (arena, ast->unary.child);
    if (ast_get_type (ast->unary.child) == INT_LIT) {
      if (ast->unary.op == '+') {
        return ast->unary.child;
      } else if (ast->unary.op == '-') {
        return ast_new_int (arena, -ast_get_ival (ast->unary.child));
      }
    }
    return ast;
  }
  case INDEXING:
    ast->indexed.array = ast_simplify_expr (arena, ast->indexed.array);
    ast->indexed.index = ast_simplify_expr (arena, ast->indexed.index);
    return ast;
  case CALLING:
    ast->fcall.func = ast_simplify_expr (arena, ast->fcall.func);
    for (int i = 0; i < ast->fcall.args->len; i++) {
      anode_t* a  = list_get_at (ast->fcall.args, i);
      anode_t* na = ast_simplify_expr (arena, a);
      list_remove_at (ast->fcall.args, i);
      list_insert_at (ast->fcall.args, na, i);
    }
    return ast;
  case DECLARATION: {
    if (ast->declaration.expr != NULL)
      ast->declaration.expr = ast_simplify_expr (arena, ast->declaration.expr);
    return ast;
  }
  case ASSIGNMENT: {
    ast->assignment.lval = ast_simplify_expr (arena, ast->assignment.lval);
    ast->assignment.expr = ast_simplify_expr (arena, ast->assignment.expr);
    return ast;
  }
  case IF_BRANCH: {
    ast->if_branch.cond = ast_simplify_expr (arena, ast->if_branch.cond);
    ast->if_branch.then = ast_simplify_expr (arena, ast->if_branch.then);
    if (ast->if_branch.otherwise != NULL) {
      ast->if_branch.otherwise = ast_simplify_expr (arena, ast->if_branch.otherwise);
    }
    return ast;
  }
  case WHILE_LOOP:
    ast->while_loop.cond = ast_simplify_expr (arena, ast->while_loop.cond);
    ast->while_loop.then = ast_simplify_expr (arena, ast->while_loop.then);
    return ast;
  case DEFER:
    ast->defer.stmt = ast_simplify_expr (arena, ast->defer.stmt);
    return ast;
  case RETURN:
    ast->ret.expr = ast_simplify_expr (arena, ast->ret.expr);
    return ast;
  case BLOCK: {
    for (int i = 0; i < ast->block.stmts->len; i++) {
      anode_t* a  = list_get_at (ast->block.stmts, i);
      anode_t* na = ast_simplify_expr (arena, a);
      list_remove_at (ast->block.stmts, i);
      list_insert_at (ast->block.stmts, na, i);
    }
    return ast;
  }
  case FUNC_DEF: return ast;
  case COMPILE_UNIT:
    for (int i = 0; i < ast->compile_unit.top_levels->len; i++) {
      anode_t* a  = list_get_at (ast->compile_unit.top_levels, i);
      anode_t* na = ast_simplify_expr (arena, a);
      list_remove_at (ast->compile_unit.top_levels, i);
      list_insert_at (ast->compile_unit.top_levels, na, i);
    }
  }
  return NULL;
}

void _print_ast (anode_t* ast, stl_t* stl, size_t depth) {
  if (ast == NULL) {
    printf ("[ NULL ]");
    return;
  }
  switch (ast->type) {
  case INT_LIT: printf ("%d", ast->ivar); break;
  case VARIABLE:
    printf ("%s", symbol_get_name(stl, ast->symbol));
    break;
  case BI_OP:
    printf ("(");
    _print_ast (ast->binary.lchild, stl, depth);
    printf(" %c ", ast->binary.op);
    _print_ast (ast->binary.rchild, stl, depth);
    printf (")");
    break;
  case UN_OP:
    printf("%c", ast->unary.op);
    _print_ast (ast->unary.child, stl, depth);
    break;
  case INDEXING:
    _print_ast (ast->indexed.array, stl, depth);
    printf ("[");
    _print_ast (ast->indexed.index, stl, depth);
    printf ("]");
    break;
  case CALLING:
    _print_ast (ast->fcall.func, stl, depth);
    printf ("(");
    for (int i = 0; i < ast->fcall.args->len; i++) {
      _print_ast (list_get_at (ast->fcall.args, i), stl, depth);
      if (i < ast->fcall.args->len - 1) printf (",");
    }
    printf (")");
    break;
  case DECLARATION: {
    type_print (symbol_get_type (stl, ast->declaration.symbol));
    printf (" %s", symbol_get_name (stl, ast->declaration.symbol));
    if (ast->declaration.expr != NULL) {
      printf (" = ");
      _print_ast (ast->declaration.expr, stl, depth);
    }
  } break;
  case ASSIGNMENT:
    _print_ast (ast->assignment.lval, stl, depth);
    printf (" = ");
    _print_ast (ast->assignment.expr, stl, depth);
    break;
  case IF_BRANCH:
    printf ("if (");
    _print_ast (ast->if_branch.cond, stl, depth);
    printf (") then ");
    _print_ast (ast->if_branch.then, stl, depth);
    {
      if (ast->if_branch.otherwise != NULL) {
        printf (" else ");
        _print_ast (ast->if_branch.otherwise, stl, depth);
      }
    }
    break;
  case WHILE_LOOP:
    printf ("while (");
    _print_ast (ast->while_loop.cond, stl, depth);
    printf (") do ");
    _print_ast (ast->while_loop.then, stl, depth);
    break;
  case DEFER:
    printf ("defer ");
    _print_ast (ast->defer.stmt, stl, depth);
    break;
  case RETURN:
    printf ("return ");
    _print_ast (ast->ret.expr, stl, depth);
    break;
  case BLOCK:
    printf ("{\n");
    for (int i = 0; i < ast->block.stmts->len; i++) {
      for (int j = 0; j < depth; j++) {
        printf ("  ");
      }
      _print_ast (list_get_at (ast->block.stmts, i), stl, depth + 1);
      printf ("\n");
    }
    for (int j = 0; j < depth - 1; j++) {
      printf ("  ");
    }
    printf ("}");
    break;
  case FUNC_DEF: {
    symbol_t sym = symbol_get (stl, ast->func_def.symbol);
    printf ("func %s(", sym.identifier);
    for (int i = 0; i < sym.type->func.len; i++) {
      type_print (stl->sts[ast->func_def.symbol.table]
      .symbols[ast->func_def.symbol.idx]
      .type->func.args[i]);
      printf (" %s",
      symbol_get_name (stl, (sp_t){ .table = sym.info.child_table, .idx = i }));
      if (i < sym.type->func.len - 1) printf (", ");
    }
    printf (") : ");
    type_print (sym.type->func.ret_type);
    _print_ast (ast->func_def.stmt, stl, depth + 1);
    break;
  }
  case COMPILE_UNIT:
    for (int i = 0; i < ast->compile_unit.top_levels->len; i++) {
      _print_ast (list_get_at (ast->compile_unit.top_levels, i), stl, depth);
      printf ("\n");
    }
    break;
  }
}

void print_ast (anode_t* ast, stl_t* stl) {
  printf ("AST node size : %zu\n", sizeof (anode_t));
  _print_ast (ast, stl, 1);
}

void ast_set_pos (anode_t* ast, size_t sl, size_t sc, size_t el, size_t ec) {
  ast->start_char_num = sc;
  ast->start_line_num = sl;
  ast->end_char_num   = ec;
  ast->end_line_num   = el;
}
