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
  } 
  return NULL;
}
