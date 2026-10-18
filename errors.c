#include "errors.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

error_t vnew_error(
    size_t line_num, size_t char_num, 
    err_type_t kind, err_level_t lvl, 
    const char* msg, va_list args)
{
  error_t err = {
    .line_num = line_num,
    .char_num = char_num,
    .kind = kind,
    .lvl = lvl,
    .msg = NULL
  };
  va_list args2;
  va_copy(args2, args);
  size_t len = vsnprintf(NULL, 0, msg, args2);
  va_end(args2);
  if(len < 0)
  {
    va_end(args);
    return err;
  }
  char* message = (char*)malloc(sizeof(char) * len);
  if(!message){
    va_end(args);
    return err;
  }

  vsprintf(message, msg, args);
  va_end(args);

  err.msg = message;

  return err;
}

error_t new_error(
    size_t line_num, size_t char_num, 
    err_type_t kind, err_level_t lvl,
    const char* msg, ...)
{
  va_list args;
  va_start(args, msg);
  return vnew_error(line_num,char_num,kind,lvl, msg, args); 
}
