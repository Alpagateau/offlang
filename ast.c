#include "ast.h"
#include <stdio.h>
#include <string.h>

struct ast_node {
  ast_type_t type;
  size_t line_num;
  size_t char_num;

  union {
    int ivar;
    char identifier[64];

    struct {
      ast_op_t op;
      anode_t *lchild;
      anode_t *rchild;
    } binary;

    struct {
      ast_native_type_t type;
      int upper_limit;
      int lower_limit;
      bool bounded;
    }type_name;

    struct {
      anode_t* t;
      anode_t* expr;
      char identifier[64];
    } declaration;

    struct {
      anode_t* lval;
      anode_t* expr;
    }assignment;

    struct {
      anode_t* cond;
      anode_t* then;
      anode_t* otherwise;
    }if_branch;

    struct {
      anode_t* cond;
      anode_t* then;
    }while_loop;

    struct {
      anode_t* stmt;
    }defer;

    struct {
      list_t* stmts;
    } block;

  };
};

anode_t *ast_new(arena_t *arena, ast_type_t type) {
  anode_t *a = (anode_t *)arena_alloc(arena, sizeof(anode_t));
  a->type = type;
  return a;
}

anode_t *ast_new_int(arena_t *arena, int value) {
  anode_t *n = ast_new(arena, INT_LIT);
  n->ivar = value;
  return n;
}

anode_t *ast_new_var(arena_t *arena, const char *value) {
  anode_t *n = ast_new(arena, VARIABLE);
  strcpy(n->identifier, value);
  return n;
}

anode_t *ast_new_binary(arena_t *arena, ast_op_t op, anode_t *lc, anode_t *rc) {
  anode_t *n = ast_new(arena, BI_OP);
  n->binary.op = op;
  n->binary.lchild = lc;
  n->binary.rchild = rc;
  return n;
}

anode_t *ast_new_type(arena_t *arena, ast_native_type_t t, 
    int min, int max, bool bounded)
{ 
  anode_t *n = ast_new(arena, TYPE);
  n->type_name.type = t;
  n->type_name.upper_limit = max;
  n->type_name.lower_limit = min;
  n->type_name.bounded = bounded;
  return n;
}

anode_t *ast_new_declaration(arena_t *arena, const char* name,anode_t* t, anode_t* expr)
{
  anode_t *n = ast_new(arena, DECLARATION);
  n->declaration.expr = expr;
  n->declaration.t = t;
  strcpy(n->declaration.identifier, name);
  return n;
}

anode_t *ast_new_assigment(arena_t *arena, anode_t* lv, anode_t* expr)
{
  anode_t *n = ast_new(arena, ASSIGNMENT);
  n->assignment.lval = lv;
  n->assignment.expr = expr;
  return n;
}

anode_t *ast_new_if_branch(arena_t *arena, anode_t* cond, anode_t *then, anode_t *otherwise)
{
  anode_t *n = ast_new(arena, IF_BRANCH);
  n->if_branch.cond = cond;
  n->if_branch.then = then;
  n->if_branch.otherwise = otherwise;
  return n;
}

anode_t *ast_new_while_loop(arena_t *arena, anode_t* cond, anode_t *then)
{
  anode_t *n = ast_new(arena, WHILE_LOOP);
  n->while_loop.cond = cond;
  n->while_loop.then = then;
  return n;
}

anode_t *ast_new_defer(arena_t *arena, anode_t *stmt)
{
  anode_t *n = ast_new(arena, DEFER);
  n->defer.stmt = stmt;
  return n;
}

anode_t *ast_new_block(arena_t *arena, list_t* lst)
{
  anode_t *n = ast_new(arena, BLOCK);
  n->block.stmts = lst;
  return n;
}

ast_type_t ast_get_type(anode_t* ast)
{
  return ast->type;
}

int ast_get_ival(anode_t *ast)
{ 
  if(ast->type == INT_LIT)
    return 0;
  return ast->ivar;
}

const char* ast_get_identifier(anode_t* ast)
{
  if(ast->type == VARIABLE)
    return NULL;
  return ast->identifier;
}

anode_t* ast_get_lchild(anode_t* ast)
{
  if(ast->type == BI_OP)
    return NULL;
  return ast->binary.lchild;
}

anode_t* ast_get_rchild(anode_t* ast)
{
  if(ast->type == BI_OP)
    return NULL;
  return ast->binary.rchild;
}

anode_t* ast_simplify_expr(arena_t* arena, anode_t* ast)
{
  switch(ast->type)
  {
    case INT_LIT:
      return ast;
    case VARIABLE:
      return ast;
    case BI_OP:
      {
      ast->binary.lchild = ast_simplify_expr(arena, ast->binary.lchild);
      ast->binary.rchild =ast_simplify_expr(arena, ast->binary.rchild);
  
      if(ast->binary.lchild->type == INT_LIT && ast->binary.rchild->type == INT_LIT)
      {
        int a = ast->binary.lchild->ivar;
        int b = ast->binary.rchild->ivar;

        int v = 0;
        switch(ast->binary.op)
        {
          case OP_ADD:
            v = a+b;
            break;
          case OP_SUB:
            v = a-b;
            break;
          case OP_MUL:
            v = a*b;
            break;
          case OP_DIV:
            v = a/b;
            break;
        } 
        return ast_new_int(arena, v);
      }
      return ast;
      } 
    case TYPE:
      return ast;
    case DECLARATION:
      {
        if(ast->declaration.expr != NULL)
          ast->declaration.expr = ast_simplify_expr(arena, ast->declaration.expr);
        return ast;
      } 
    case ASSIGNMENT:
      {
        ast->assignment.lval = ast_simplify_expr(arena, ast->assignment.lval);
        ast->assignment.expr = ast_simplify_expr(arena, ast->assignment.expr);
        return ast;
      }
    case IF_BRANCH:
      {
        ast->if_branch.cond = ast_simplify_expr(arena, ast->if_branch.cond);
        ast->if_branch.then = ast_simplify_expr(arena, ast->if_branch.then);
        if(ast->if_branch.otherwise != NULL)
        {
          ast->if_branch.otherwise = ast_simplify_expr(arena, ast->if_branch.otherwise);
        }
        return ast;
      }
    case WHILE_LOOP:
      ast->while_loop.cond = ast_simplify_expr(arena, ast->while_loop.cond);
      ast->while_loop.then = ast_simplify_expr(arena, ast->while_loop.then);
      return ast;
    case DEFER:
      ast->defer.stmt = ast_simplify_expr(arena, ast->defer.stmt);
      return ast;
    case BLOCK:
      {
        for(int i = 0; i < ast->block.stmts->len; i++)
        {
          anode_t* a = list_get_at(ast->block.stmts, i);
          anode_t* na = ast_simplify_expr(arena, a);
          list_remove_at(ast->block.stmts, i);
          list_insert_at(ast->block.stmts, na,i);
        }
        return ast;
      }
  }
  return NULL;
}

void _print_op(ast_op_t op)
{
  switch(op)
  {
    case OP_ADD:
      printf("+");
      break;
    case OP_SUB:
      printf("-");
      break;
    case OP_MUL:
      printf("*");
      break;
    case OP_DIV:
      printf("/");
      break;
  }
}

void _print_ast(anode_t* ast, size_t depth)
{
  if(ast == NULL) {
    printf("[ NULL ]");
    return;
  }
  switch(ast->type)
  {
    case INT_LIT:
      printf("%d", ast->ivar);
      break;
    case VARIABLE:
      printf("%s", ast->identifier);
      break;
    case BI_OP:
      printf("(");
      _print_ast(ast->binary.lchild, depth);
      _print_op(ast->binary.op);
      _print_ast(ast->binary.rchild, depth);
      printf(")");
      break;
    case TYPE:
      {
      if(ast->type_name.bounded){
        printf(
            "int <%d, %d>", ast->type_name.lower_limit, 
            ast->type_name.upper_limit);
        }
      else{
        printf("int");
        }
      }
      break;
    case DECLARATION:
      {
      _print_ast(ast->declaration.t, depth);
      printf(" %s", ast->declaration.identifier);
      if(ast->declaration.expr != NULL)
      {
        printf(" = ");
        _print_ast(ast->declaration.expr, depth);
      }
      }
      break;
    case ASSIGNMENT:
      _print_ast(ast->assignment.lval, depth);
      printf(" = ");
      _print_ast(ast->assignment.expr, depth);
      break;
    case IF_BRANCH:
      printf("if (");
      _print_ast(ast->if_branch.cond, depth);
      printf(") then ");
      _print_ast(ast->if_branch.then, depth);
      {
        if(ast->if_branch.otherwise != NULL)
        {
          printf(" else ");
          _print_ast(ast->if_branch.otherwise, depth);
        }
      }
      break;
    case WHILE_LOOP:
      printf("while (");
      _print_ast(ast->while_loop.cond, depth);
      printf(") do ");
      _print_ast(ast->while_loop.then, depth);
      break;
    case DEFER:
      printf("defer ");
      _print_ast(ast->defer.stmt, depth);
      break;
    case BLOCK:
      printf("{\n");
      for(int i = 0; i < ast->block.stmts->len; i++)
      {
        for(int j = 0; j < depth; j++)
        {
          printf("  ");
        }
        _print_ast(list_get_at(ast->block.stmts, i), depth+1);
        printf("\n");
      }
      for(int j = 0; j < depth-1; j++)
      {
        printf("  ");
      }
      printf("}");
      break;
  }
}

void print_ast(anode_t* ast)
{
  printf("AST node size : %zu\n", sizeof(anode_t));
  _print_ast(ast, 1);
}

void ast_set_pos(anode_t* ast, size_t l, size_t c)
{
  ast->char_num  = c;
  ast->line_num  = l;
}
