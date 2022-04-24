#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "vector.h"

#include "rb_tree.h"

typedef struct
{
  struct rbt_node rbt_node;
  int value;
} Int_Set_Node;

typedef struct Int_Set
{
  struct rbtree tree;
  size_t size;
} Int_Set;

static void
intset_construct (Int_Set *self)
{
  self->tree = RBT_EMPTY;
  self->size = 0;
}

static void
intset_destruct_impl (struct rbt_node *n)
{
  if (n->left)
    intset_destruct_impl (n->left);
  if (n->right)
    intset_destruct_impl (n->right);
  free (RBT_CONTAINER_OF (n, Int_Set_Node, rbt_node));
}

static void
intset_destruct (Int_Set *self)
{
  if (self->tree.root)
    intset_destruct_impl (self->tree.root);
}

static bool
intset_insert (Int_Set *self, int i)
{
  Int_Set_Node *n_self, *new_node;
  struct rbt_node *node = self->tree.root, *parent = NULL;
  enum rbt_direction direction;

  while (node)
    {
      n_self = RBT_CONTAINER_OF (node, Int_Set_Node, rbt_node);
      parent = node;

      if (i == n_self->value)
        return false;
      else if (i < n_self->value)
        {
          node = node->left;
          direction = RBT_LEFT;
        }
      else
        {
          node = node->right;
          direction = RBT_RIGHT;
        }
    }

  new_node = (Int_Set_Node *)malloc (sizeof (Int_Set_Node));
  new_node->value = i;

  rbt_insert (&self->tree, &new_node->rbt_node, parent, direction);
  ++self->size;
  return true;
}

static Int_Set_Node *
intset_search (Int_Set *self, int i)
{
  Int_Set_Node *data;
  struct rbt_node *node = self->tree.root;

  while (node)
    {
      data = RBT_CONTAINER_OF (node, Int_Set_Node, rbt_node);
      if (i < data->value)
        node = node->left;
      else if (i > data->value)
        node = node->right;
      else
        return data;
    }
  return NULL;
}

static bool
intset_remove (Int_Set *self, int i)
{
  Int_Set_Node *victim;
  if ((victim = intset_search (self, i)) != NULL)
    {
      rbt_erase (&self->tree, &victim->rbt_node);
      --self->size;
      free (victim);
      return true;
    }
  return false;
}

static bool
intset_contains (Int_Set *self, int i)
{
  return intset_search (self, i) != NULL;
}

/* for rbt_print */
static void
intset_print_node (struct rbt_node *node, unsigned width, char *buf)
{
  Int_Set_Node *self = RBT_CONTAINER_OF (node, Int_Set_Node, rbt_node);
  snprintf (buf, width+1, "%*d", width, self->value);
}

static void
intset_print_impl (struct rbt_node *node)
{
  if (node->left)
    intset_print_impl (node->left);
  Int_Set_Node *self = RBT_CONTAINER_OF (node, Int_Set_Node, rbt_node);
  printf (" \x1b[3%cm%d\x1b[0m",
          node->color == RBT_BLACK ? '8' : '1', self->value);
  if (node->right)
    intset_print_impl (node->right);
}

static void
intset_print (Int_Set *self)
{
  if (self->size)
    {
      intset_print_impl (self->tree.root);
      putchar ('\n');
    }
}

static bool
verify_order (Int_Set *s)
{
  struct rbt_node *n;
  struct rbt_node *last;
  int v, prev = 0;
  if (s->size == 0 || s->size == 1)
    return true;
  n = rbt_first (&s->tree);
  last = rbt_last (&s->tree);
  do
    {
      v = RBT_CONTAINER_OF (n, Int_Set_Node, rbt_node)->value;
      if (v < prev)
        return false;
      prev = v;
      n = rbt_next (n);
    }
  while (n != last);
  return true;
}

static unsigned my_rand_state = 0;
static unsigned
my_rand ()
{
  unsigned long long tmp;
  unsigned m1, m2;
  my_rand_state += 0xE120FC15;
  tmp = (unsigned long long)my_rand_state * 0x7A39B70D;
  m1 = (tmp >> 32) ^ tmp;
  tmp = (unsigned long long)m1 * 0x12FAD5C9;
  m2 = (tmp >> 32) ^ tmp;
  return m2;
}

void
random_test (unsigned seed, bool verbose)
{
  const int iterations = 1000000;
  const int maxval = 100;
  const int node_width = snprintf (NULL, 0, "%d", maxval-1);
  /* print tree if it fits into 120 columns */
  const size_t max_nodes_for_tree_print = (119 / (node_width + 1));
  Int_Set s;
  int i, j, v;
  int *values = NULL;
  my_rand_state = seed;
  intset_construct (&s);
  printf ("Running with %d iterations...\n", iterations);
  for (i = 0; i < iterations; ++i)
    {
      if (my_rand () % 2)
        {
          /* add */
          v = my_rand () % maxval;
          if (verbose)
            printf ("add:    %2d\n", v);
          vector_push (values, v);
          intset_insert (&s, v);
        }
      else if (s.size)
        {
          /* remove */
          j = my_rand () % vector_size (values);
          v = values[j];
          if (verbose)
            printf ("remove: %2d\n", v);
          vector_remove (values, j);
          intset_remove (&s, v);
        }
    }
  puts ("Final:");
  if (s.size < max_nodes_for_tree_print)
    rbt_print (&s.tree, intset_print_node, node_width, stdout);
  printf ("%zu nodes\n", s.size);
  intset_print (&s);
  if (!verify_order (&s))
    puts ("\x1b[31mOut of order :(\x1b[0m");
  else
    puts ("\x1b[32mIn order :)\x1b[0m");
  intset_destruct (&s);
  vector_free (values);
}

int
main (int argc, const char *const *argv)
{
  const int node_width = 3;
  int i;
  Int_Set my_set;
  for (i = 1; i < argc; ++i)
    {
      if (strcmp (argv[i], "random") == 0)
        {
          random_test (time (NULL), false);
          return 0;
        }
    }

  intset_construct (&my_set);

  for (i = 1; i <= 10; ++i)
    intset_insert (&my_set, i);

  puts ("Full tree:");
  rbt_print (&my_set.tree, intset_print_node, node_width, stdout);
  intset_print (&my_set);

  assert (my_set.size == 10);
  for (i = 1; i <= 10; ++i)
    assert (intset_contains (&my_set, i));

  for (i = 1; i <= 10; ++i)
    intset_insert (&my_set, i);
  assert (my_set.size == 10);

  for (i = 1; i <= 10; i += 2)
    intset_remove (&my_set, i);

  puts ("Odd removed:");
  rbt_print (&my_set.tree, intset_print_node, node_width, stdout);
  intset_print (&my_set);

  assert (my_set.size == 5);
  for (i = 1; i <= 10; ++i)
    assert (intset_contains (&my_set, i) == !(i % 2));

  intset_destruct (&my_set);
}

