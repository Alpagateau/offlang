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
      char identifier[64];
      anode_t* expr;
    } declaration;

    struct {
      anode_t* lval;
      anode_t* expr;
    }assignment;
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

anode_t *ast_new_declaration(arena_t *arena, const char* name, anode_t* expr)
{
  anode_t *n = ast_new(arena, DECLARATION);
  n->declaration.expr = expr;
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
    case DECLARATION:
      {
      printf("int %s", ast->declaration.identifier);
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
  }
}

void print_ast(anode_t* ast)
{
  _print_ast(ast, 0);
}

void ast_set_pos(anode_t* ast, size_t l, size_t c)
{
  ast->char_num  = c;
  ast->line_num  = l;
}
