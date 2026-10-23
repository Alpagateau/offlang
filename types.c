#include "types.h"
#include <stdio.h>
#include <stdlib.h>

type_t* type_new_primary(primitive_type_t t)
{
  type_t* data = (type_t*)malloc(sizeof(type_t));
  data->kind = TYPE_PRIMITIVE;
  data->prim_kind = t;
  return data;
}

type_t* type_new_pointer(type_t* base)
{
  type_t* data = (type_t*)malloc(sizeof(type_t));
  data->kind = TYPE_POINTER;
  data->pointer.base = base;
  return data;
}

type_t* type_new_bounded(type_t* base, int min, int max)
{
  type_t* data = (type_t*)malloc(sizeof(type_t));
  data->kind = TYPE_BOUNDED;
  data->bounded.base = base;
  data->bounded.min = min;
  data->bounded.max = max;
  return data;
}

type_t* type_new_func()
{
  type_t* data = (type_t*)malloc(sizeof(type_t));
  data->kind = TYPE_FUNC;
  data->func.len = 0;
  data->func.capacity = 2;
  data->func.args = (type_t**)malloc(sizeof(type_t*) * 2);
  return data;
}
void type_func_add_arg(type_t* f, type_t* t)
{
  if(f->func.len >= f->func.capacity - 1)
  {
    f->func.capacity *= 2;
    f->func.args = (type_t**)realloc(f->func.args, sizeof(size_t*) * f->func.capacity);
  }

  f->func.args[f->func.len++] = t;
}

void type_func_set_ret(type_t* f, type_t* t)
{
  f->func.ret_type = t;
}

type_t* type_copy(type_t* src)
{ 
  type_t* data = (type_t*)malloc(sizeof(type_t));
  data->kind = src->kind;
  switch(data->kind)
  {
    case TYPE_PRIMITIVE:
      data->prim_kind = src->prim_kind;
      break;
    case TYPE_BOUNDED:
      data->bounded.base = type_copy(src->bounded.base);
      data->bounded.max = src->bounded.max;
      data->bounded.min = src->bounded.min;
      break;
    case TYPE_POINTER:
      data->pointer.base = type_copy(src->pointer.base);
      break;
    default:
      printf("[SHOULD NOT HAPPEN]");
      break;
  }
  return data;
}

bool type_eq(type_t* a, type_t* b)
{
  if(a->kind != b->kind) return false;
  switch(a->kind)
  {
    case TYPE_PRIMITIVE:
      return a->prim_kind == b->prim_kind;
    case TYPE_BOUNDED:
      if(!type_eq(a->bounded.base, b->bounded.base))
        return false;
      if(a->bounded.max != b->bounded.max)
        return false;
      if(a->bounded.min != b->bounded.min)
        return false;
    case TYPE_POINTER:
      return type_eq(a->pointer.base, b->pointer.base);
    case TYPE_FUNC:
      if(a->func.ret_type != b->func.ret_type) return false;
      if(a->func.len != b->func.len) return false;
      for(int i = 0; i < a->func.len; i++)
      {
        if(!type_eq(a->func.args[i], b->func.args[i])) 
          return false;
      }
    default:
      return false;
  }
  return true;
}

// To do better
bool type_order(type_t* src, type_t* dst)
{
  if(type_eq(src, dst)) return true;
  if(src->kind == TYPE_POINTER && dst->kind == TYPE_POINTER)
    return type_order(src->pointer.base, dst->pointer.base);
  if(src->kind == TYPE_BOUNDED)
  {
    if(dst->kind == TYPE_PRIMITIVE)
      return type_eq(src->bounded.base, dst);
    if(dst->kind == TYPE_BOUNDED)
    {
      return src->bounded.min >= dst->bounded.min 
        &&   src->bounded.max <= dst->bounded.max;
    }
  }
  if(src->kind == TYPE_FUNC && dst->kind == TYPE_FUNC)
  {
    if(!type_order(src->func.ret_type, dst->func.ret_type))
      return false;

    if(src->func.len != dst->func.len) return false;
    for(int i = 0; i < src->func.len; i++)
    {
      if(!type_order(src->func.args[i], dst->func.args[i]))
        return false;
    }
    return true;
  }

  return false;
}

void type_free(type_t* t)
{
  if(t == NULL) return;
  switch(t->kind){
    case TYPE_BOUNDED:
      type_free(t->bounded.base);
      break;
    case TYPE_POINTER:
      type_free(t->pointer.base);
      break;
    case TYPE_PRIMITIVE:
    case TYPE_NAMED:
      break;
    case TYPE_FUNC:
      for(int i = 0; i < t->func.len; i++)
      {
        fflush(stdout);
        type_free(t->func.args[i]);
      }
      type_free(t->func.ret_type);
      free(t->func.args);
      break;
  }
  free(t);
}

void print_type_primitive(primitive_type_t t)
{
  switch(t)
  {
    case T_CHAR:
      printf("char");
      break;
    case T_UCHAR:
      printf("uchar");
      break;
    case T_SHORT:
      printf("short");
      break;
    case T_USHORT:
      printf("ushort");
      break;
    case T_INT:
      printf("int");
      break;
    case T_UINT:
      printf("uint");
      break;
    case T_LONG:
      printf("long");
      break;
    case T_ULONG:
      printf("ulong");
      break;
    case T_LLONG:
      printf("llong");
      break;
    case T_ULLONG:
      printf("ullong");
      break;
    case T_FLOAT:
      printf("float");
      break;
    case T_DOUBLE:
      printf("double");
      break;
    case T_LDOUBLE:
      printf("ldouble");
      break;
    case T_BOOL:
      printf("bool");
      break;
    case T_VOID:
      printf("void");
      break;
  }
}

void type_print(type_t* t)
{
  if(t == NULL) {
    printf("[NULL]");
    return;
  }
  switch(t->kind)
  {
    case TYPE_PRIMITIVE:
      print_type_primitive(t->prim_kind);
      break; 
    case TYPE_BOUNDED:
      type_print(t->bounded.base);
      printf("<%d, %d>", t->bounded.min, t->bounded.max);
      break;
    case TYPE_POINTER:
      type_print(t->pointer.base);
      printf("^");
      break;
    default:
      printf("SHould not happen yet");
      break;
  }
}
