#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>
#include <stdbool.h>

#define SYMBOL_LEN 64

struct symbol
{
  char identifier[SYMBOL_LEN];
  size_t line_num;
  size_t char_num;
  struct {
    bool is_bounded;
    int  min;
    int  max;
  } type;
  union {
    int ival;
  } def_val;
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

void symbol_list_free(stl_t* stl);
void symbol_table_free(st_t st);

#endif
