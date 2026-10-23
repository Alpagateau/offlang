#include "semantic.h"
#include "ast.h"
#include "symbol_table.h"
#include "types.h"

type_t *analyse_node(parser_ctx *ctx, anode_t *node)
{
  if(node == NULL) return NULL;
  
  switch(ast_get_type(node))
  {
    case INT_LIT:
      return type_new_bounded(
          type_new_primary(T_INT), 
          ast_get_ival(node), 
          ast_get_ival(node)
          );
    case VARIABLE:
    {
      sp_t sym = ast_get_symbol(node);
      return symbol_get_type(ctx->stl, sym);
    }
    case BI_OP:{
      type_t* left = analyse_node(ctx, node->binary.lchild);
      type_t* right = analyse_node(ctx, node->binary.rchild);
      if(type_is_sub(right, left))
        return left;
      }
      break;
    case UN_OP:{
        type_t* child = analyse_node(ctx, node->unary.child);
        if(!type_to_range(child).positive)
        {
        }
      }
  } 
  return NULL;
}
