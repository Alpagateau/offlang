#include "types.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

type_t* type_new_primary (primitive_type_t t) {
  type_t* data    = (type_t*)malloc (sizeof (type_t));
  data->kind      = TYPE_PRIMITIVE;
  data->prim_kind = t;
  return data;
}

type_t* type_new_pointer (type_t* base) {
  type_t* data       = (type_t*)malloc (sizeof (type_t));
  data->kind         = TYPE_POINTER;
  data->pointer.base = base;
  return data;
}

type_t* type_new_bounded (type_t* base, long long min, long long max) {
  type_t* data             = (type_t*)malloc (sizeof (type_t));
  data->kind               = TYPE_BOUNDED;
  data->bounded.base       = base;
  data->bounded.range.smin = min;
  data->bounded.range.smax = max;
  return data;
}

type_t* type_new_func () {
  type_t* data        = (type_t*)malloc (sizeof (type_t));
  data->kind          = TYPE_FUNC;
  data->func.len      = 0;
  data->func.capacity = 2;
  data->func.args     = (type_t**)malloc (sizeof (type_t*) * 2);
  return data;
}
void type_func_add_arg (type_t* f, type_t* t) {
  if (f->func.len >= f->func.capacity - 1) {
    f->func.capacity *= 2;
    f->func.args = (type_t**)realloc (f->func.args, sizeof (size_t*) * f->func.capacity);
  }

  f->func.args[f->func.len++] = t;
}

void type_func_set_ret (type_t* f, type_t* t) { f->func.ret_type = t; }

type_t* type_copy (type_t* src) {
  type_t* data = (type_t*)malloc (sizeof (type_t));
  data->kind   = src->kind;
  switch (data->kind) {
  case TYPE_PRIMITIVE: data->prim_kind = src->prim_kind; break;
  case TYPE_BOUNDED:
    data->bounded.base       = type_copy (src->bounded.base);
    data->bounded.range.smax = src->bounded.range.smax;
    data->bounded.range.smin = src->bounded.range.smin;
    break;
  case TYPE_POINTER: data->pointer.base = type_copy (src->pointer.base); break;
  case TYPE_FUNC:
    data->func.len      = src->func.len;
    data->func.capacity = src->func.len;
    data->func.args     = malloc (sizeof (type_t*) * src->func.len);
    for (int i = 0; i < data->func.len; i++) {
      data->func.args[i] = type_copy (src->func.args[i]);
    }
  default:
    free (data);
    printf ("[SHOULD NOT HAPPEN]");
    break;
    return NULL;
  }
  return data;
}

bool type_eq (type_t* a, type_t* b) {
  if (a->kind != b->kind) return false;
  switch (a->kind) {
  case TYPE_PRIMITIVE: return a->prim_kind == b->prim_kind;
  case TYPE_BOUNDED:
    if (!type_eq (a->bounded.base, b->bounded.base)) return false;
    if (a->bounded.range.smax != b->bounded.range.smax) return false;
    if (a->bounded.range.smin != b->bounded.range.smin) return false;
    break;
  case TYPE_POINTER: return type_eq (a->pointer.base, b->pointer.base);
  case TYPE_FUNC:
    if (!type_eq (a->func.ret_type, b->func.ret_type)) return false;
    if (a->func.len != b->func.len) return false;
    for (int i = 0; i < a->func.len; i++) {
      if (!type_eq (a->func.args[i], b->func.args[i])) return false;
    }
    break;
  default: return false;
  }
  return true;
}

bool type_is_valid (type_t* t) {
  switch (t->kind) {
  case TYPE_PRIMITIVE: return true;
  case TYPE_POINTER: return type_is_valid (t->pointer.base);
  case TYPE_BOUNDED: {
    // TODO : Need to check against base type
    if (t->bounded.range.positive) {
      return t->bounded.range.umax >= t->bounded.range.umin;
    } else {
      return t->bounded.range.smax >= t->bounded.range.smin;
    }
  }
  case TYPE_NAMED: return false;
  case TYPE_FUNC: {
    if (!type_is_valid (t->func.ret_type)) return false;
    for (int i = 0; i < t->func.len; i++) {
      if (!type_is_valid (t->func.args[i])) return false;
    }
    return true;
  }
  }
  return false;
}

range_t type_to_range (type_t* t) {
  if (t->kind == TYPE_BOUNDED) {
    return t->bounded.range;
  }
  if (t->kind == TYPE_PRIMITIVE) {
    switch (t->prim_kind) {
    case T_CHAR:
      return (range_t){ .positive = false, .smax = CHAR_MAX, .smin = CHAR_MIN };
    case T_UCHAR:
      return (range_t){ .positive = true, .umax = UCHAR_MAX, .umin = 0 };
    case T_SHORT:
      return (range_t){ .positive = false, .smax = SHRT_MAX, .smin = SHRT_MIN };
    case T_USHORT:
      return (range_t){ .positive = true, .umax = USHRT_MAX, .umin = 0 };
    case T_INT:
      return (range_t){ .positive = false, .smax = INT_MAX, .smin = INT_MIN };
    case T_UINT:
      return (range_t){ .positive = true, .umax = UINT_MAX, .umin = 0 };
    case T_LONG:
      return (range_t){ .positive = false, .smax = LONG_MAX, .smin = LONG_MIN };
    case T_ULONG:
      return (range_t){ .positive = true, .umax = ULONG_MAX, .umin = 0 };
    case T_LLONG:
      return (range_t){ .positive = false, .smax = LONG_MAX, .smin = LONG_MIN };
    case T_ULLONG:
      return (range_t){ .positive = true, .umax = ULONG_MAX, .umin = 0 };
    case T_FLOAT:
      return (range_t){ .positive = false, .smax = LONG_MAX, .smin = LONG_MIN };
    case T_DOUBLE:
      return (range_t){ .positive = false, .smax = LONG_MAX, .smin = LONG_MIN };
    case T_LDOUBLE:
      return (range_t){ .positive = false, .smax = LONG_MAX, .smin = LONG_MIN };
    case T_BOOL: return (range_t){ .positive = true, .umin = 0, .umax = 1 };
    case T_VOID:
      return (range_t){ .positive = true, .umin = 0, .umax = 0 };
      break;
    }
  }
  return (range_t){ true, 0, 0 };
}

// To do better
bool type_is_sub (type_t* src, type_t* dst) {
  if (type_eq (src, dst)) return true;

  if ((src->kind == TYPE_PRIMITIVE || src->kind == TYPE_BOUNDED) &&
      (dst->kind == TYPE_PRIMITIVE || dst->kind == TYPE_BOUNDED)) {
    range_t r_src = type_to_range (src);
    range_t r_dst = type_to_range (dst);
    return r_src.smin >= r_dst.smin && r_src.smax <= r_dst.smax;
  }
  if (src->kind == TYPE_POINTER && dst->kind == TYPE_POINTER) {
    return type_is_sub (src->pointer.base, dst->pointer.base);
  }
  if (src->kind == TYPE_FUNC && dst->kind == TYPE_FUNC) {
    if (src->func.len != dst->func.len) return false;
    if (!type_is_sub (src->func.ret_type, dst->func.ret_type)) return false;
    for (int i = 0; i < src->func.len; i++) {
      if (!type_is_sub (src->func.args[i], dst->func.args[i])) return false;
    }
    return true;
  }
  return false;
}


// type_t *common_super_type(type_t* a, type_t* b)
//{
//   if(type_is_sub(a, b)) return b;
//   if(type_is_sub(b, a)) return a;
//
//   //if(type_t)
// }

void type_free (type_t* t) {
  if (t == NULL) return;
  switch (t->kind) {
  case TYPE_BOUNDED: type_free (t->bounded.base); break;
  case TYPE_POINTER: type_free (t->pointer.base); break;
  case TYPE_PRIMITIVE:
  case TYPE_NAMED: break;
  case TYPE_FUNC:
    for (int i = 0; i < t->func.len; i++) {
      fflush (stdout);
      type_free (t->func.args[i]);
    }
    type_free (t->func.ret_type);
    free (t->func.args);
    break;
  }
  free (t);
}

void print_type_primitive (primitive_type_t t) {
  switch (t) {
  case T_CHAR: printf ("char"); break;
  case T_UCHAR: printf ("uchar"); break;
  case T_SHORT: printf ("short"); break;
  case T_USHORT: printf ("ushort"); break;
  case T_INT: printf ("int"); break;
  case T_UINT: printf ("uint"); break;
  case T_LONG: printf ("long"); break;
  case T_ULONG: printf ("ulong"); break;
  case T_LLONG: printf ("llong"); break;
  case T_ULLONG: printf ("ullong"); break;
  case T_FLOAT: printf ("float"); break;
  case T_DOUBLE: printf ("double"); break;
  case T_LDOUBLE: printf ("ldouble"); break;
  case T_BOOL: printf ("bool"); break;
  case T_VOID: printf ("void"); break;
  }
}

void type_print (type_t* t) {
  if (t == NULL) {
    printf ("[NULL]");
    return;
  }
  switch (t->kind) {
  case TYPE_PRIMITIVE: print_type_primitive (t->prim_kind); break;
  case TYPE_BOUNDED:
    type_print (t->bounded.base);
    printf ("<%lld, %lld>", t->bounded.range.smin, t->bounded.range.smax);
    break;
  case TYPE_POINTER:
    type_print (t->pointer.base);
    printf ("^");
    break;
  default: printf ("SHould not happen yet"); break;
  }
}
