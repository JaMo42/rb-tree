#include "rb_tree.h"
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
      return node;
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
      return node;
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

