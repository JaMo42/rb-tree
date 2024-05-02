/* github.com/JaMo42/rb-tree */
#ifndef RB_TREE_H
#define RB_TREE_H
#include <stdio.h>
#include <stddef.h>

#define RBT_CONTAINER_OF(ptr, type, member) \
  ((type *)((char *)(ptr) + offsetof (type, member)))

#define RBT_EMPTY (struct rbtree) { NULL, }

#ifdef __cplusplus
extern "C" {
#endif

enum rbt_direction
{
  RBT_LEFT = 0,
  RBT_RIGHT = 1
};

enum rbt_color
{
  RBT_BLACK,
  RBT_RED
};

struct rbt_node
{
  enum rbt_color color;
  struct rbt_node *parent;
  union
  {
    struct rbt_node *child[2];
    struct
    {
      struct rbt_node *left;
      struct rbt_node *right;
    };
  };
};

struct rbtree
{
  struct rbt_node *root;
};

/* Inserts a node into the tree as a child of the given parent. `dir` specifies
   the direction of the child. Rebalances the tree if neccessary. */
void rbt_insert (struct rbtree *self, struct rbt_node *node,
                 struct rbt_node *parent, enum rbt_direction dir);

/* Erases the given node from the tree. Rebalances the free if neccessary. */
void rbt_erase (struct rbtree *self, struct rbt_node *victim);

/* Gets the height of the tree. */
unsigned rbt_height (const struct rbtree *self);

/* Gets the size (number of nodes) of the tree.
   Note that this is O(n) as the tree does not keep track of the size
   and just counts the nodes.  If you need fast access to the size you
   should keep track of it manually. */
unsigned rbt_size (const struct rbtree *self);

/* Returns the first node of the tree. */
struct rbt_node *rbt_first (const struct rbtree *self);

/* Returns the last node of the tree. */
struct rbt_node *rbt_last (const struct rbtree *self);

/* Returns the in-order successor of the given node. */
struct rbt_node *rbt_next (const struct rbt_node *node);

/* Returns the in-order predecessor of the given node. */
struct rbt_node *rbt_prev (const struct rbt_node *node);

/* Should print the nodes value into the given buffer. `width` is the
   `node_width` parameter given to `rbt_print`. */
typedef void (*rbt_print_node_t) (const struct rbt_node *node, unsigned width,
                                  char *buf);

/* Prints the tree into the given stream.
   The `print_node` function should format a given node into the given buffer.
   `node_width` is the width each node takes in the output. */
void rbt_print (const struct rbtree *tree, rbt_print_node_t print_node,
                unsigned node_width, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* RB_TREE_H */

#ifdef RBT_IMPLEMENTATION

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#  include <malloc.h>
#  define alloca _alloca
#else
#  include <alloca.h>
#endif

#define rbt_child_direction(n) \
  ((n) == (n)->parent->left ? RBT_LEFT : RBT_RIGHT)

#ifdef __cplusplus
#define RBT_OPPOSITE(d) ((enum rbt_direction)(1 - (int)(d)))
#else
#define RBT_OPPOSITE(d) (1 - (d))
#endif

#ifdef __cplusplus
extern "C" {
#endif

static struct rbt_node *
rbt_rotate (struct rbtree *self, struct rbt_node *parent,
            enum rbt_direction dir)
{
  struct rbt_node *gparent, *sibling, *close;
  gparent = parent->parent;
  sibling = parent->child[RBT_OPPOSITE (dir)];
  assert (sibling);
  close = sibling->child[dir];

  parent->child[RBT_OPPOSITE (dir)] = close;
  if (close)
    close->parent = parent;

  sibling->child[dir] = parent;
  parent->parent = sibling;

  sibling->parent = gparent;
  if (gparent)
    gparent->child[parent == gparent->right ? RBT_RIGHT : RBT_LEFT] = sibling;
  else
    self->root = sibling;
  return sibling;
}

void
rbt_insert (struct rbtree *self, struct rbt_node *node,
            struct rbt_node *parent, enum rbt_direction dir)
{
  struct rbt_node *gparent, *uncle;

  node->color = RBT_RED;
  node->left = NULL;
  node->right = NULL;
  node->parent = parent;

  if (parent == NULL)
    {
      self->root = node;
      return;
    }

  parent->child[dir] = node;

  do
    {
      if (parent->color == RBT_BLACK)
        /* case 1 */
        return;
      if ((gparent = parent->parent) == NULL)
        {
          /* case 4 */
          parent->color = RBT_BLACK;
          return;
        }
      dir = rbt_child_direction (parent);
      uncle = gparent->child[RBT_OPPOSITE (dir)];
      if (uncle == NULL || uncle->color == RBT_BLACK)
        {
          if (node == parent->child[RBT_OPPOSITE (dir)])
            {
              /* case 5 */
              rbt_rotate (self, parent, dir);
              node = parent;
              parent = gparent->child[dir];
            }
          /* case 6 */
          rbt_rotate (self, gparent, RBT_OPPOSITE (dir));
          parent->color = RBT_BLACK;
          gparent->color = RBT_RED;
          return;
        }
      /* case 2 */
      parent->color = RBT_BLACK;
      uncle->color = RBT_BLACK;
      gparent->color = RBT_RED;
      node = gparent;
    }
  while ((parent = node->parent) != NULL);
  /* case 3 */
}

static void
rbt_swap_nodes (struct rbt_node *a, struct rbt_node *b)
{
  struct rbt_node swap;

  if (a->parent)
    {
      if (a == a->parent->left)
        a->parent->left = b;
      else
        a->parent->right = b;
    }

  if (b->left)
    b->left->parent = a;
  if (b->right)
    b->right->parent = a;

  if (a == b->parent)
    {
      swap = *b;
      if (b == a->left)
        {
           b->right = a->right;
           b->left = a;
        }
      else
        {
          b->left = a->left;
          b->right = a;
        }
      b->parent = a->parent;
      b->color = a->color;

      a->parent = b;
      a->left = swap.left;
      a->right = swap.right;
      a->color = swap.color;
    }
  else
    {
      if (b == b->parent->left)
        b->parent->left = a;
      else
        b->parent->right = a;

      swap = *b;
      *b = *a;
      *a = swap;
    }

  if (b->left)
    b->left->parent = b;
  if (b->right)
    b->right->parent = b;
}


static void rbt_erase_rebalance (struct rbtree *self, struct rbt_node *node);

void
rbt_erase (struct rbtree *self, struct rbt_node *victim)
{
  struct rbt_node *replacement, *parent;
  enum rbt_direction dir;
  if (victim == self->root && victim->left == victim->right)
    {
      self->root = NULL;
      return;
    }
  if (victim->left && victim->right)
    {
      replacement = rbt_prev (victim);
      if (victim == self->root)
        self->root = replacement;
      rbt_swap_nodes (victim, replacement);
    }

  /* `RBT_LEFT` is just a "random" value here, this is not used if the victims
      parent is NULL. */
  dir = victim->parent ? rbt_child_direction (victim) : RBT_LEFT;
  parent = victim->parent;

  if (victim->color == RBT_RED)
    {
      parent->child[dir] = NULL;
      return;
    }

  if (victim->left == victim->right)
    rbt_erase_rebalance (self, victim);
  else
    {
      replacement = victim->left ? victim->left : victim->right;
      replacement->parent = parent;
      replacement->color = RBT_BLACK;
      if (parent)
        parent->child[dir] = replacement;
      else
        self->root = replacement;
    }
}

static void
rbt_erase_rebalance (struct rbtree *self, struct rbt_node *node)
{
  struct rbt_node *parent, *sibling, *close, *distant;
  enum rbt_direction dir;

  parent = node->parent;
  dir = rbt_child_direction (node);

  parent->child[dir] = NULL;

  goto rbt_erase_skip_direction_update;
  do
    {
      dir = rbt_child_direction (node);
rbt_erase_skip_direction_update:
      sibling = parent->child[RBT_OPPOSITE (dir)];
      distant = sibling->child[RBT_OPPOSITE (dir)];
      close = sibling->child[dir];

      if (sibling->color == RBT_RED)
        {
          /* case 3 */
          rbt_rotate (self, parent, dir);
          parent->color = RBT_RED;
          sibling->color = RBT_BLACK;
          sibling = close;
          distant = sibling->child[RBT_OPPOSITE (dir)];
          if (distant && distant->color == RBT_RED)
            goto rbt_delete_1;
          close = sibling->child[dir];
          if (close && close->color == RBT_RED)
            goto rbt_delete_2;
          goto rbt_delete_3;
        }
      if (distant && distant->color == RBT_RED)
        {
rbt_delete_1: /* case 6 */
          rbt_rotate (self, parent, dir);
          sibling->color = parent->color;
          parent->color = RBT_BLACK;
          distant->color = RBT_BLACK;
          return;
        }
      if (close && close->color == RBT_RED)
        {
rbt_delete_2: /* case 5 */
          rbt_rotate (self, sibling, RBT_OPPOSITE (dir));
          sibling->color = RBT_RED;
          close->color = RBT_BLACK;
          distant = sibling;
          sibling = close;
          goto rbt_delete_1;
          __builtin_unreachable ();
        }
      if (parent->color == RBT_RED)
        {
rbt_delete_3: /* case 4 */
          sibling->color = RBT_RED;
          parent->color = RBT_BLACK;
          return;
        }
      /* case 1 */
      sibling->color = RBT_RED;
      node = parent;
    }
  while ((parent = node->parent) != NULL);
  /* case 2 */
}


static unsigned
rbt_height_impl (const struct rbt_node *node)
{
  if (!node)
    return 0;
  const unsigned left = rbt_height_impl (node->left);
  const unsigned right = rbt_height_impl (node->right);
  return 1 + (left > right ? left : right);
}

unsigned
rbt_height (const struct rbtree *tree)
{
  return rbt_height_impl (tree->root);
}


static unsigned
rbt_size_impl (const struct rbt_node *node)
{
  return (node
          ? 1 + rbt_size_impl (node->left) + rbt_size_impl (node->right)
          : 0);
}

unsigned
rbt_size (const struct rbtree *tree)
{
  return rbt_size_impl (tree->root);
}


struct rbt_node *
rbt_next (const struct rbt_node *node)
{
  struct rbt_node *parent;
  if (node->right)
    {
      node = node->right;
      while (node->left)
        node = node->left;
      return (struct rbt_node *)node;
    }
  while ((parent = node->parent) && node == parent->right)
    node = parent;
  return parent;
}


struct rbt_node *
rbt_prev (const struct rbt_node *node)
{
  struct rbt_node *parent;
  if (node->left)
    {
      node = node->left;
      while (node->right)
        node = node->right;
      return (struct rbt_node *)node;
    }
  while ((parent = node->parent) && node == parent->left)
    node = parent;
  return parent;
}


struct rbt_node *
rbt_first (const struct rbtree *self)
{
  struct rbt_node *node = self->root;
  while (node->left)
    node = node->left;
  return node;
}


struct rbt_node *
rbt_last (const struct rbtree *self)
{
  struct rbt_node *node = self->root;
  while (node->right)
    node = node->right;
  return node;
}


static unsigned
rbt_print_impl (char **buf, struct rbt_node *node, rbt_print_node_t print_node,
                bool is_left, unsigned width, unsigned offset, unsigned depth)
{
  char *print_buf;
  unsigned left, right;
  unsigned i;

  if (!node)
    return 0;

  left = rbt_print_impl (buf, node->left, print_node, true, width,
                         offset, depth + 1);
  right = rbt_print_impl (buf, node->right, print_node, false, width,
                          offset + left + width + 1, depth + 1);

  /* node value */
  print_buf = (char *)alloca (width + 1);
  memset (print_buf, ' ', width);
  print_buf[width] = '\0';
  print_node (node, width, print_buf);
  for (i = 0; i < width; ++i)
    buf[depth][offset + left + 1 + i] = print_buf[i];

  /* connection to parent */
  const unsigned width_2 = (width + 2) >> 1;
  if (depth && is_left)
    {
      for (i = 0; i < width + right - 2; ++i)
        buf[depth - 1][offset + left + width_2 + 1 + i] = '-';
      buf[depth - 1][offset + left + width_2] = '.';
    }
  else if (depth)
    {
      for (i = 0; i < left + width - 1; ++i)
        buf[depth - 1][offset - width_2 + 3 + i] = '-';
      buf[depth - 1][offset + left + width_2] = '.';
    }

  /* "box" around node value */
  buf[depth][offset + left] = '(';
  buf[depth][offset + left + width + 1] = ')';

  return left + width + right + 1;
}

void
rbt_print (const struct rbtree *tree, rbt_print_node_t print_node,
           unsigned node_width, FILE *stream)
{
  const unsigned height = rbt_height (tree);
  const unsigned node_count = rbt_size (tree);
  const unsigned width = (node_width + 1) * node_count + 1;
  char **buf;
  unsigned i;

  if (tree->root == NULL)
    return;

  buf = (char **)malloc (height * sizeof (char *));
  for (i = 0; i < height; ++i)
    {
      buf[i] = (char *)malloc (width + 2);
      memset (buf[i], ' ', width);
      buf[i][width] = '\n';
      buf[i][width + 1] = '\0';
    }
  (void)rbt_print_impl (buf, tree->root, print_node, false, node_width, 0, 0);
  for (i = 0; i < height; ++i)
    {
      fputs (buf[i], stream);
      free (buf[i]);
    }
  free (buf);
}

#ifdef __cplusplus
}
#endif
#endif /* RBT_IMPLEMENTATION */

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - 2-Clause BSD License
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
