#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "types.h"
#include <stddef.h>
#include <stdbool.h>

#define SYMBOL_LEN 64

typedef enum {SYM_VARIABLE, SYM_FUNC} symbol_kind_t;

struct symbol
{
  symbol_kind_t kind;
  char identifier[SYMBOL_LEN];
  size_t line_num;
  size_t char_num;

  type_t* type;

  union {
    union {
      int ival;
    } default_value;

    int child_table;
  } info;

};

typedef struct symbol symbol_t;

typedef struct {
  int prev; 
  symbol_t *symbols;
  size_t capacity;
  size_t len;
} symbol_table;

typedef symbol_table st_t;

typedef struct 
{
  size_t capacity;
  size_t len;
  st_t *sts;
}stl_t;

typedef struct {
  int table;
  int idx;
} sp_t;

int symbol_table_new(stl_t* list, int prev);

sp_t symbol_add(stl_t* list, int table, symbol_t s);

sp_t search_symbol(
    stl_t* list, 
    int table, 
    const char* name);

symbol_t symbol_get(stl_t* list, sp_t sym);
const char* symbol_get_name(stl_t* list, sp_t sym);
type_t* symbol_get_type(stl_t* list, sp_t sym);

void symbol_list_free(stl_t stl);
void symbol_table_free(st_t st);

#endif
