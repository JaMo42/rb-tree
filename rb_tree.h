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

/* Gets the size (number of nodes) of the tree. */
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

