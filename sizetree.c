/* jdupes file size binary search tree
 * This file is part of jdupes; see jdupes.c for license information */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libjodycode.h>

#include "jdupes.h"
#include "likely_unlikely.h"
#include "sizetree.h"


/* Number of slots to add at a time; must be at least 2 */
#define SIZETREE_ALLOC_SLOTS 2

static struct sizetree *sizetree_head = NULL;


/* Allocate a sizetree node */
static struct sizetree *sizetree_alloc(file_t *file)
{
  struct sizetree *node;

  if (file == NULL) jc_nullptr("sizetree_alloc");
  LOUD(fprintf(stderr, "sizetree_alloc('%s' [%ld])\n", file->d_name, file->size);)
  node = (struct sizetree *)calloc(1, sizeof(struct sizetree));
  if (node == NULL) jc_oom("sizetree_alloc");
  node->size = file->size;
  node->list = file;
  return node;
}


/* Return the next file list; reset: 1 = restart, -1 = free resources */
file_t *sizetree_next_list(struct sizetree_state *st)
{
  struct sizetree *cur;

  LOUD(fprintf(stderr, "sizetree_next_list(%p)\n", st);)

  if (st == NULL) jc_nullptr("sizetree_next_list");

  if (unlikely(st->reset == -1)) {
    if (st->stack != NULL) free(st->stack);
    return NULL;
  }

  if (st->reset == 1 || st->stack == NULL) {
    /* Initialize everything and push head of tree as first stack item */
    if (st->stack != NULL) free(st->stack);
    st->reset = 0;
    st->stack = (struct sizetree **)malloc(sizeof(struct sizetree *) * SIZETREE_ALLOC_SLOTS);
    st->stackslots = SIZETREE_ALLOC_SLOTS;
    st->stackcnt = 1;
    st->stack[0] = sizetree_head;
  }
  /* Pop one off the stack */
  if (st->stackcnt < 1) return NULL;
  cur = st->stack[--st->stackcnt];
  if (cur == NULL) return NULL;

  if (st->stackslots - st->stackcnt < 2) {
    void *tempalloc = realloc(st->stack, sizeof(struct sizetree *) * (SIZETREE_ALLOC_SLOTS + st->stackslots));
    if (tempalloc == NULL) jc_oom("sizetree_alloc realloc");
    st->stack = (struct sizetree **)tempalloc;
    st->stackslots += SIZETREE_ALLOC_SLOTS;
  }

  /* Push left/right nodes to stack and return current node's list */
  if (cur->right != NULL) st->stack[st->stackcnt++] = cur->right;
  if (cur->left != NULL) st->stack[st->stackcnt++] = cur->left;
  return cur->list;
}


/* Add a file to the size tree */
void sizetree_add(file_t *file)
{
  struct sizetree *cur;

  if (file == NULL) jc_nullptr("sizetree_add");
  LOUD(fprintf(stderr, "sizetree_add([%p] '%s')\n", file, file->d_name);)

  if (sizetree_head == NULL) {
//    LOUD(fprintf(stderr, "sizetree_add: new sizetree_head size %ld\n", file->size);)
    sizetree_head = sizetree_alloc(file);
    return;
  }

  cur = sizetree_head;
  while (1) {
    if (file->size == cur->size) {
//      LOUD(fprintf(stderr, "sizetree_add: attaching to list [%p] size %ld\n", cur, cur->size);)
      file->next = cur->list;
      cur->list = file;
      return;
    }

    if (file->size < cur->size) {
      if (cur->left == NULL) {
        cur->left = sizetree_alloc(file);
//        LOUD(fprintf(stderr, "sizetree_add: creating new list L [%p] size %ld\n", cur->left, file->size);)
	return;
      }
//      LOUD(fprintf(stderr, "sizetree_add: going left (%ld < %ld)\n", file->size, cur->size);)
      cur = cur->left;
      continue;
    }

    if (file->size > cur->size) {
      if (cur->right == NULL) {
        cur->right = sizetree_alloc(file);
//        LOUD(fprintf(stderr, "sizetree_add: creating new list R [%p] size %ld\n", cur->right, file->size);)
	return;
      }
//      LOUD(fprintf(stderr, "sizetree_add: going right (%ld > %ld)\n", file->size, cur->size);)
      cur = cur->right;
      continue;
    }
  }
}
