#include "arena.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>

struct arena_allocator {
  size_t capacity;
  size_t cursor;
  error_t* errors;
  size_t err_len;
  size_t err_cap;
  char* buffer;
};

arena_t* arena_new (size_t size, arena_flag_t flags) {
  arena_t* a  = (arena_t*)malloc (sizeof (*a));
  a->capacity = size;
  a->cursor   = 0;
  a->buffer   = (char*)malloc (sizeof (char) * size);

  a->err_len = 0;
  a->err_cap = 2;

  a->errors = (error_t*)malloc (sizeof (error_t) * 2);
  return a;
}

void* arena_alloc (arena_t* arena, size_t size) {
  if (arena->cursor + size >= arena->capacity) {
    printf ("[ARENA ERROR] : No more space, cannot allocate data of size %zu", size);
    return NULL;
  }

  char* ptr = arena->buffer + arena->cursor;
  arena->cursor += size;
  return ptr;
}

void* arena_realloc(arena_t* arena, void* src, size_t old_size, size_t new_size)
{
  void* new_mem = arena_alloc(arena, new_size);
  if(!new_mem) return NULL;
  if(!src) return new_mem;

  size_t bytes_to_copy = (old_size > new_size) ? new_size : old_size;
  for(size_t i = 0; i < bytes_to_copy; i++)
  {
    ((char*)new_mem)[i] = ((char*)src)[i];
  }
  return new_mem;
}

size_t arena_get_taken (arena_t* arena) { return arena->cursor; }

void arena_new_err (arena_t* arena, error_t err) {
  if (arena->err_len + 1 >= arena->err_cap) {
    // realocate
    arena->err_cap *= 2;
    arena->errors = realloc (arena->errors, arena->err_cap * sizeof (error_t));
  }
  arena->errors[arena->err_len++] = err;
}

error_t* arena_get_errors (arena_t* arena, size_t* len) {
  if (len != NULL) *len = arena->err_len;
  return arena->errors;
}

void arena_free (arena_t* a) {
  free (a->errors);
  free (a->buffer);
  free (a);
}
