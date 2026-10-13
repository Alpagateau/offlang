#include <stdio.h>
#include <stdlib.h>
#include "arena.h"

struct arena_allocator {
  size_t capacity;
  size_t cursor;
  char*  buffer;
};

arena_t* arena_new(size_t size, arena_flag_t flags)
{
  arena_t* a = (arena_t*)malloc(sizeof(*a));
  a->capacity = size;
  a->cursor = 0;
  a->buffer = (char*)malloc(sizeof(char) * size);
  return a;
}

void* arena_alloc(arena_t* arena, size_t size)
{
  if(arena->cursor + size >= arena->capacity){
    printf("[ARENA ERROR] : No more space, cannot allocate data of size %zu", size);
    return NULL;
  }
  
  char* ptr = arena->buffer + arena->cursor;
  arena->cursor += size;
  return ptr;
}

size_t arena_get_taken(arena_t* arena)
{
  return arena->cursor;
}

void arena_free(arena_t* a)
{
  free(a->buffer);
  free(a);
}
