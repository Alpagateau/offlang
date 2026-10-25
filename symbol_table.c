#include "symbol_table.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int symbol_table_new (stl_t* list, int prev) {
    if (list->len == list->capacity - 1) {
        list->capacity *= 2;
        list->sts = realloc (list->sts, sizeof (st_t) * list->capacity);
    }

    list->sts[list->len++] =
    (st_t){ .len = 0, .capacity = 1, .prev = prev, .symbols = malloc (sizeof (symbol_t)) };
    return list->len - 1;
}

sp_t search_symbol (stl_t* list, int table, const char* name) {
    if (table < 0) return (sp_t){ -1, -1 };

    st_t t = list->sts[table];
    for (int i = 0; i < t.len; i++) {
        if (!strcmp (t.symbols[i].identifier, name)) {
            return (sp_t){ .table = table, .idx = i };
        }
    }
    return search_symbol (list, t.prev, name);
}

sp_t symbol_add (stl_t* list, int table, symbol_t s) {
    st_t* t = &list->sts[table];
    if (t->len >= t->capacity - 1) {
        t->capacity *= 2;
        t->symbols = realloc (t->symbols, t->capacity * sizeof (symbol_t));
    }
    t->symbols[t->len++] = s;
    return (sp_t){ .table = table, .idx = t->len - 1 };
}

symbol_t symbol_get (stl_t* list, sp_t sym) {
    return list->sts[sym.table].symbols[sym.idx];
}

const char* symbol_get_name (stl_t* list, sp_t sym) {
    return list->sts[sym.table].symbols[sym.idx].identifier;
}

type_t* symbol_get_type (stl_t* list, sp_t sym) {
    return list->sts[sym.table].symbols[sym.idx].type;
}

void symbol_list_free (stl_t stl) {
    for (int i = 0; i < stl.len; i++) {
        symbol_table_free (stl.sts[i]);
    }
    free (stl.sts);
    // free(stl);
}

void symbol_table_free (st_t st) {
    for (int i = 0; i < st.len; i++) {
        type_free (st.symbols[i].type);
    }
    free (st.symbols);
}
