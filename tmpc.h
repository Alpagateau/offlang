#ifndef TMPC_H
#define TMPC_H

// This is a temporary AST to C program. 
// It shall not stay long. 
// Only used for testing purposes

#include "ast.h"
#include "parser.h"

void fcompile(parser_ctx *ctx, anode_t *node);

#endif
