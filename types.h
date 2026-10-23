#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
  bool positive;
  long long smax;
  long long smin;
  unsigned long long umax;
  unsigned long long umin;
} range_t;

typedef enum {
  TYPE_PRIMITIVE,
  TYPE_BOUNDED,
  TYPE_POINTER,
  TYPE_NAMED,
  TYPE_FUNC
} type_kind_t;

typedef enum {
  // Integers
  T_CHAR,
  T_UCHAR,
  T_SHORT,
  T_USHORT,
  T_INT,
  T_UINT,
  T_LONG,
  T_ULONG,
  T_LLONG,
  T_ULLONG,

  // Floating point
  T_FLOAT,
  T_DOUBLE,
  T_LDOUBLE,

  // Other
  T_BOOL,
  T_VOID
} primitive_type_t;

typedef struct type_t {
  type_kind_t kind;

  union {
    primitive_type_t prim_kind;
    union {
      struct {
        struct type_t* base;
        range_t range;
      } bounded;

      struct {
        struct type_t* base;
      }pointer;

      struct {
        struct type_t* ret_type;
        struct type_t** args;
        size_t len;
        size_t capacity;
      }func;

    };
  };
} type_t;


type_t* type_new_primary(primitive_type_t t);
type_t* type_new_pointer(type_t* base);
type_t* type_new_bounded(type_t* base, long long min, long long max);
type_t* type_new_func();
void type_func_add_arg(type_t* f, type_t* t);
void type_func_set_ret(type_t* f, type_t* t);

type_t* type_copy(type_t* src);
bool type_eq(type_t* a, type_t* b);

// returns true if src is a subtype of dst
range_t type_to_range(type_t* t);
bool type_is_sub(type_t* src, type_t* dst);
type_t* type_combine(type_t* a, type_t* b);

void type_free(type_t* t);

void type_print(type_t* t);

#endif
