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

void type_free(type_t* t)
{
  switch(t->kind){
    case TYPE_BOUNDED:
      type_free(t->bounded.base);
      break;
    case TYPE_POINTER:
      type_free(t->pointer.base);
      break;
    case TYPE_PRIMITIVE:
    case TYPE_NAMED:
    case TYPE_FUNC:
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
    default:
      printf("SHould not happen yet");
  }
}
