#ifndef ERRORS_H
#define ERRORS_H

#include <stdarg.h>
#include <stddef.h>

typedef enum { ERR_LEXER, ERR_PARSER } err_type_t;
typedef enum { ERR_LOG, ERR_WAR, ERR_ERR } err_level_t;

typedef struct {
  size_t line_num;
  size_t char_num;
  err_type_t kind;
  err_level_t lvl;
  char* msg;
} error_t;

error_t
new_error (size_t line_num, size_t char_num, err_type_t kind, err_level_t lvl, const char* msg, ...);

error_t
vnew_error (size_t line_num, size_t char_num, err_type_t kind, err_level_t lvl, const char* msg, va_list);

#endif
