#include "tmpc.h"
#include "ast.h"
#include "symbol_table.h"

void tmp_print_type_primitive (primitive_type_t t) {
    switch (t) {
    case T_CHAR: printf ("char"); break;
    case T_UCHAR: printf ("unsigned char"); break;
    case T_SHORT: printf ("short"); break;
    case T_USHORT: printf ("unsigned short"); break;
    case T_INT: printf ("int"); break;
    case T_UINT: printf ("unsigned int"); break;
    case T_LONG: printf ("long"); break;
    case T_ULONG: printf ("unsigned long"); break;
    case T_LLONG: printf ("long long"); break;
    case T_ULLONG: printf ("unsigned long long"); break;
    case T_FLOAT: printf ("float"); break;
    case T_DOUBLE: printf ("double"); break;
    case T_LDOUBLE: printf ("long double"); break;
    case T_BOOL: printf ("bool"); break;
    case T_VOID: printf ("void"); break;
    }
}
void tmp_type_print (type_t* t) {
    if (t == NULL) {
        printf ("/*[NULL]*/ void ");
        return;
    }
    switch (t->kind) {
    case TYPE_PRIMITIVE: tmp_print_type_primitive (t->prim_kind); break;
    case TYPE_BOUNDED:
        tmp_type_print (t->bounded.base);
        // printf("<%lld, %lld>", t->bounded.min, t->bounded.max);
        break;
    case TYPE_POINTER:
        tmp_type_print (t->pointer.base);
        printf ("*");
        break;
    default: printf ("SHould not happen yet"); break;
    }
}

void _tmp_print_op (ast_op_t op) {
    switch (op) {
    case OP_ADD: printf ("+"); break;
    case OP_SUB: printf ("-"); break;
    case OP_MUL: printf ("*"); break;
    case OP_DIV: printf ("/"); break;
    }
}

void _fcompile (parser_ctx* ctx, anode_t* ast, int depth) {
    if (ast == NULL) {
        printf ("/*[ NULL ]*/\n");
        return;
    }
    switch (ast->type) {
    case INT_LIT: printf ("%d", ast->ivar); break;
    case VARIABLE:
        printf ("%s", symbol_get_name (ctx->stl, ast->symbol));
        break;
    case BI_OP:
        printf ("(");
        _fcompile (ctx, ast->binary.lchild, depth);
        _tmp_print_op (ast->binary.op);
        _fcompile (ctx, ast->binary.rchild, depth);
        printf (")");
        break;
    case UN_OP:
        _tmp_print_op (ast->unary.op);
        _fcompile (ctx, ast->unary.child, depth);
        break;
    case INDEXING:
        _fcompile (ctx, ast->indexed.array, depth);
        printf ("[");
        _fcompile (ctx, ast->indexed.index, depth);
        printf ("]");
        break;
    case CALLING:
        _fcompile (ctx, ast->fcall.func, depth);
        printf ("(");
        for (int i = 0; i < ast->fcall.args->len; i++) {
            _fcompile (ctx, list_get_at (ast->fcall.args, i), depth);
            if (i < ast->fcall.args->len - 1)
                printf (",");
        }
        printf (")");
        break;
    case DECLARATION: {
        tmp_type_print (symbol_get_type (ctx->stl, ast->declaration.symbol));
        printf (" %s", symbol_get_name (ctx->stl, ast->declaration.symbol));
        if (ast->declaration.expr != NULL) {
            printf (" = ");
            _fcompile (ctx, ast->declaration.expr, depth);
        }
        printf (";");
    } break;
    case ASSIGNMENT:
        _fcompile (ctx, ast->assignment.lval, depth);
        printf (" = ");
        _fcompile (ctx, ast->assignment.expr, depth);
        printf (";");
        break;
    case IF_BRANCH:
        printf ("if (");
        _fcompile (ctx, ast->if_branch.cond, depth);
        printf (") then ");
        _fcompile (ctx, ast->if_branch.then, depth);
        {
            if (ast->if_branch.otherwise != NULL) {
                printf (" else ");
                _fcompile (ctx, ast->if_branch.otherwise, depth);
            }
        }
        break;
    case WHILE_LOOP:
        printf ("while (");
        _fcompile (ctx, ast->while_loop.cond, depth);
        printf (") ");
        _fcompile (ctx, ast->while_loop.then, depth);
        break;
    case DEFER:
        printf ("/*defer */");
        _fcompile (ctx, ast->defer.stmt, depth);
        break;
        printf (";");
    case RETURN:
        printf ("return ");
        _fcompile (ctx, ast->ret.expr, depth);
        printf (";");
        break;
    case BLOCK:
        printf ("{\n");
        for (int i = 0; i < ast->block.stmts->len; i++) {
            for (int j = 0; j < depth; j++) {
                printf ("  ");
            }
            _fcompile (ctx, list_get_at (ast->block.stmts, i), depth + 1);
            printf ("\n");
        }
        for (int j = 0; j < depth - 1; j++) {
            printf ("  ");
        }
        printf ("}");
        break;
    case FUNC_DEF: {
        symbol_t sym = symbol_get (ctx->stl, ast->func_def.symbol);
        tmp_type_print (sym.type->func.ret_type);
        printf (" %s(", sym.identifier);
        for (int i = 0; i < sym.type->func.len; i++) {
            tmp_type_print (ctx->stl->sts[ast->func_def.symbol.table]
            .symbols[ast->func_def.symbol.idx]
            .type->func.args[i]);
            printf (" %s",
            symbol_get_name (ctx->stl, (sp_t){ .table = sym.info.child_table, .idx = i }));
            if (i < sym.type->func.len - 1)
                printf (", ");
        }
        printf ("){");
        _fcompile (ctx, ast->func_def.stmt, depth + 1);
        printf ("}\n");
        break;
    }
    case COMPILE_UNIT:
        for (int i = 0; i < ast->compile_unit.top_levels->len; i++) {
            _fcompile (ctx, list_get_at (ast->compile_unit.top_levels, i), depth);
            printf ("\n");
        }
        break;
    }
}


void fcompile (parser_ctx* ctx, anode_t* node) {
    _fcompile (ctx, node, 0);
}
