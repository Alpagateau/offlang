#ifndef AST_H
#define AST_H

#include "arena.h"
#include "symbol_table.h"
#include <stdbool.h>

struct ast_node;
typedef struct ast_node anode_t;

struct list_node_t;
typedef struct list_node_t list_node_t;

typedef struct {
  list_node_t* first; 
  list_node_t* last;
  size_t len;
  arena_t* arena;
} list_t;

typedef enum { 
  //Expr
  INT_LIT, 
  VARIABLE, 
  BI_OP,
  UN_OP,
  
  //Type system
  TYPE,

  //Statements
  DECLARATION,
  ASSIGNMENT,
  IF_BRANCH,
  WHILE_LOOP,
  DEFER,
  RETURN,
  BLOCK,
  
  //Top Level
  FUNC_DEF,
} ast_type_t;

typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } ast_op_t;
typedef enum { TYPE_INT } ast_native_type_t;

anode_t *ast_new(arena_t *arena, ast_type_t type);

anode_t *ast_new_int(arena_t *arena, int value);
anode_t *ast_new_var(arena_t *arena, sp_t symbol);
anode_t *ast_new_binary(arena_t *arena, ast_op_t op, anode_t *lc, anode_t *rc);
anode_t *ast_new_unary(arena_t *arena, ast_op_t op, anode_t *child);
anode_t *ast_new_type(arena_t *arena, ast_native_type_t t,int min, int max, bool bounded);
anode_t *ast_new_declaration(arena_t *arena, sp_t symbol, anode_t* t,anode_t* expr);
anode_t *ast_new_assigment(arena_t *arena, anode_t* lv, anode_t* expr);
anode_t *ast_new_if_branch(arena_t *arena, anode_t* cond, anode_t *then, anode_t *otherwise);
anode_t *ast_new_while_loop(arena_t *arena, anode_t* cond, anode_t *then);
anode_t *ast_new_defer(arena_t *arena, anode_t *stmt);
anode_t *ast_new_return(arena_t *arena, anode_t *ret);
anode_t *ast_new_block(arena_t *arena, list_t* lst);
anode_t *ast_new_func_def(arena_t *arena, anode_t *t, 
    list_t *args, anode_t *stmt, const char* name);

ast_type_t ast_get_type(anode_t* ast);

int  ast_get_ival(anode_t* ast);

void ast_set_pos(anode_t* ast, size_t l, size_t c);

//const char* ast_get_identifier(anode_t* ast);
anode_t* ast_get_lchild(anode_t* ast);
anode_t* ast_get_rchild(anode_t* ast);

anode_t* ast_simplify_expr(arena_t* arena, anode_t* ast);

void print_ast(anode_t* ast);

// LISTS
list_t* list_new(arena_t* arena);

void list_append(list_t* l, anode_t* node);
void list_insert_at(list_t* l, anode_t* node, size_t idx);
void list_remove_at(list_t* l, size_t idx);

anode_t* list_get_at(list_t* l, size_t idx);

#endif
