#include "ast.h"
#include "arena.h"
#include <stdio.h>

struct list_node_t {
  anode_t *elem;
  list_node_t *nxt;
};

list_node_t *new_list_node(anode_t *a, arena_t* arena) {
  list_node_t *n = (list_node_t *)arena_alloc(arena, sizeof(*n));
  n->elem = a;
  n->nxt = NULL;
  return n;
}

list_t *list_new(arena_t* arena) {
  list_t *l = (list_t *)arena_alloc(arena, sizeof(*l));
  l->first = NULL;
  l->last = NULL;
  l->len = 0;
  l->arena = arena;
  return l;
}

void list_append(list_t *l, anode_t *node) {
  if (l == NULL)
    return;
  list_node_t *n = new_list_node(node, l->arena);
  if (l->len == 0) {
    l->first = n;
    l->last = n;
  } else {
    l->last->nxt = n;
    l->last = n;
  }
  l->len++;
}

void list_insert_at(list_t *l, anode_t *node, size_t idx) {
  if (idx == 0) {
    list_node_t *n = new_list_node(node, l->arena);
    n->nxt = l->first;
    l->first = n;
    l->len++;
  } else if (idx == l->len) {
    list_append(l, node);
  } else if (idx < l->len) {
    list_node_t *n = new_list_node(node, l->arena);
    list_node_t *e = l->first;
    for (size_t i = 0; i < idx - 1; i++) {
      e = e->nxt;
    }
    //if(n == NULL) return;
    //if(e == NULL) return;
    n->nxt = e->nxt;
    e->nxt = n;
    l->len++;
  } else {
    return;
  }
}

void list_remove_at(list_t *l, size_t idx) {
  if (idx == 0) {
    l->first = l->first->nxt;
    if(l->first == NULL)
    {
      l->last = NULL;
    }
    l->len--;
  }
  if (idx < l->len && idx > 0) {
    list_node_t *n = l->first;
    for (size_t i = 0; i < idx - 1; i++) {
      n = n->nxt;
    }
    //if(n == NULL) return;
    if (l->last == n->nxt) {
      l->last = n;
    }
    n->nxt = n->nxt->nxt;
    l->len--;
  }
}

anode_t *list_get_at(list_t *l, size_t idx) {
  if(idx >= l->len)
    return NULL;
  list_node_t *n = l->first;

  //printf("[LIST LOG] l of len %zu. First : %p; Last : %p\n", l->len, l->first, l->last);

  size_t i = 0;
  while(i != idx){
    //printf("[LIST LOG] going from %p to %p (idx = %zu)\n", n, n->nxt, i);
    n = n->nxt;
    i++;
  }
  if(n == NULL)
  {
    //printf("[LIST ERR] idx %zu in l of len %zu is null\n", idx, l->len);
    return NULL;
  }
  return n->elem;
}
