#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include "errors.h"

struct arena_allocator;
typedef struct arena_allocator arena_t;

typedef enum {
  AF_NONE      = 0 << 0,
  AF_RESIZABLE = 1 << 1,
} arena_flag_t;

//allocates a new arena of capicity `size`. flags are currently unused
arena_t* arena_new(size_t size, arena_flag_t flags);

//allocates a region inside of an arena
void* arena_alloc(arena_t* arena, size_t size);
size_t arena_get_taken(arena_t* arena);

//error handling
void arena_new_err(arena_t* arena, error_t err);
error_t* arena_get_errors(arena_t* arena, size_t* len);

//free and destroy the arena
void arena_free(arena_t* a);

#endif
