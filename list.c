#include "ast.h"
#include "arena.h"

extern arena_t *arena;

struct list_node_t {
  anode_t *elem;
  list_node_t *nxt;
};

list_node_t *new_list_node(anode_t *a) {
  list_node_t *n = (list_node_t *)arena_alloc(arena, sizeof(*n));
  n->elem = a;
  n->nxt = NULL;
  return n;
}

list_t *list_new() {
  list_t *l = (list_t *)arena_alloc(arena, sizeof(*l));
  l->first = NULL;
  l->last = NULL;
  l->len = 0;
  return l;
}

void list_append(list_t *l, anode_t *node) {
  if (l == NULL)
    return;
  list_node_t *n = new_list_node(node);
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
    list_node_t *n = new_list_node(node);
    n->nxt = l->first;
    l->first = n;
  } else if (idx == l->len) {
    list_append(l, node);
  } else if (idx < l->len) {
    list_node_t *n = new_list_node(node);
    list_node_t *e = l->first;
    for (size_t i = 0; i < idx - 1; i++) {
      e = e->nxt;
    }
    if(n == NULL) return;
    if(e == NULL) return;
    n->nxt = e->nxt;
    e->nxt = n;
  } else {
    return;
  }
  l->len++;
}

void list_remove_at(list_t *l, size_t idx) {
  if (idx == 0) {
    l->first = l->first->nxt;
    l->len--;
  }
  if (idx < l->len && idx > 0) {
    list_node_t *n = l->first;
    for (size_t i = 0; i < idx - 1; i++) {
      n = n->nxt;
    }
    if(n == NULL) return;
    n->nxt = n->nxt->nxt;
    if (l->last == n->nxt) {
      l->last = n;
    }
    l->len--;
  }
}

anode_t *list_get_at(list_t *l, size_t idx) {
  if(idx >= l->len)
    return NULL;
  list_node_t *n = l->first;
  for (size_t i = 0; i < idx; i++) {
    if(n == NULL) return NULL;
    n = n->nxt;
  }
  if(n == NULL) return NULL;
  return n->elem;
}
