#ifndef AST_H
#define AST_H

#include "arena.h"
#include <stdbool.h>

struct ast_node;
typedef struct ast_node anode_t;

struct list_node_t;
typedef struct list_node_t list_node_t;

typedef struct {
  list_node_t* first; 
  list_node_t* last;
  size_t len;
} list_t;

typedef enum { 
  //Expr
  INT_LIT, 
  VARIABLE, 
  BI_OP,
  
  //Type system
  TYPE,

  //Statements
  DECLARATION,
  ASSIGNMENT,
  IF_BRANCH,
  WHILE_LOOP,
  DEFER,
  BLOCK,
} ast_type_t;

typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } ast_op_t;
typedef enum { TYPE_INT } ast_native_type_t;

anode_t *ast_new(arena_t *arena, ast_type_t type);

anode_t *ast_new_int(arena_t *arena, int value);
anode_t *ast_new_var(arena_t *arena, const char *value);
anode_t *ast_new_binary(arena_t *arena, ast_op_t op, anode_t *lc, anode_t *rc);
anode_t *ast_new_type(arena_t *arena, ast_native_type_t t, int min, int max, bool bounded);
anode_t *ast_new_declaration(arena_t *arena, const char* name, anode_t* t,anode_t* expr);
anode_t *ast_new_assigment(arena_t *arena, anode_t* lv, anode_t* expr);
anode_t *ast_new_if_branch(arena_t *arena, anode_t* cond, anode_t *then, anode_t *otherwise);
anode_t *ast_new_while_loop(arena_t *arena, anode_t* cond, anode_t *then);
anode_t *ast_new_defer(arena_t *arena, anode_t *stmt);
anode_t *ast_new_block(arena_t *arena, list_t* lst);

ast_type_t ast_get_type(anode_t* ast);

int  ast_get_ival(anode_t* ast);

void ast_set_pos(anode_t* ast, size_t l, size_t c);

const char* ast_get_identifier(anode_t* ast);
anode_t* ast_get_lchild(anode_t* ast);
anode_t* ast_get_rchild(anode_t* ast);

anode_t* ast_simplify_expr(arena_t* arena, anode_t* ast);

void print_ast(anode_t* ast);

// LISTS


list_t* list_new();

void list_append(list_t* l, anode_t* node);
void list_insert_at(list_t* l, anode_t* node, size_t idx);
void list_remove_at(list_t* l, size_t idx);

anode_t* list_get_at(list_t* l, size_t idx);

#endif
