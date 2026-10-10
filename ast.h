#ifndef AST_H
#define AST_H

#include "arena.h"

struct ast_node;
typedef struct ast_node anode_t;

typedef enum { INT_LIT, VARIABLE, BI_OP } ast_type_t;

typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } ast_op_t;

anode_t *ast_new(arena_t *arena, ast_type_t type);

anode_t *ast_new_int(arena_t *arena, int value);
anode_t *ast_new_var(arena_t *arena, const char *value);
anode_t *ast_new_binary(arena_t *arena, ast_op_t op, anode_t *lc, anode_t *rc);

ast_type_t ast_get_type(anode_t* ast);

int  ast_get_ival(anode_t* ast);

void ast_set_pos(anode_t* ast, size_t l, size_t c);

const char* ast_get_identifier(anode_t* ast);
anode_t* ast_get_lchild(anode_t* ast);
anode_t* ast_get_rchild(anode_t* ast);

anode_t* ast_simplify_expr(arena_t* arena, anode_t* ast);

void print_ast(anode_t* ast);
#endif
