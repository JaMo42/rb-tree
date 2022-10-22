# rb-tree

Intrusive  [red-black tree](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree) written in C.

Works in C++ too but if compiling with `-pedantic` you will get warnings about anonymous structs.

## Usage

Data nodes are structures containing a `struct rbt_node` member:

```c
struct my_type {
  struct rbt_node rbt_node;
  int key;
};
```

A new tree is initialized to be empty via:

```c
struct rbtree my_tree = RBT_EMPTY;
```

A nodes data is accessed using the `RBT_CONTAINER_OF` macro:

```c
struct rbt_node *node_ptr = ...;
struct my_type *data = RBT_CONTAINER_OF(node_ptr, struct my_type, rbt_node);
```

Note: if the `struct rbt_node` member is the first member of the struct this
boils down to just a cast instead of having to offset the pointer.

### Search

Example:

```c
struct my_type *my_search(struct rbtree *tree, int key) {
  struct rbt_node *node = tree->root;

  while (node) {
    struct my_type *data = RBT_CONTAINER_OF(node, struct my_type, rbt_node);

    if (key < data->key)
      node = node->left;
    else if (key > data->key)
      node = node->right;
    else
      return data;
  }
  return NULL;
}
```

### Insertion

```c
void rbt_insert(struct rbtree *tree, struct rbt_node *node, struct rbt_node *parent, enum rbt_direction direction);
```

Example:

```c
bool my_insert(struct rbtree *tree, struct my_type *data) {
  struct rbt_node *node = tree->root, *parent = NULL;
  enum rbt_direction dir;
  while (node) {
    struct my_type *test = RBT_CONTAINER_OF (node, struct my_type, rbt_node);
    parent = node;
    if (data->key < test->key) {
      node = node->left;
      dir = RBT_LEFT;
    }
    else if (data->key > test->key) {
      node = node->right;
      dir = RBT_RIGHT;
    }
    else
      return false;
  }
  rbt_insert (tree, &data->rbt_node, parent, dir);
  return true;
}
```

### Deletion

```c
void rbt_erase(struct rbtree *tree, struct rbt_node *victim);
```

Example:

```c
struct my_type *data = my_search (tree, 123);
if (data) {
  rbt_erase (tree, &data->node);
  free (data);
}
```

### Traversal

Get the first node:
```c
struct rbt_node *rbt_first (struct rbtree *tree);
```

Get the last node:
```c
struct rbt_node *rbt_last (struct rbtree *tree);
```

Get the predecessor of a node:
```c
struct rbt_node *rbt_prev (struct rbt_node *node);
```

Get the successor of a node:
```c
struct rbt_node *rbt_next (struct rbt_node *node);
```

### Printing

```c
typedef void (*rbt_print_node_t) (struct rbt_node *node, unsigned width, char *buf);
void rbt_print (struct rbtree *tree, rbt_print_node_t print_node, unsigned node_width, FILE *stream);
```

Example:

```c
void my_print_node(struct rbt_node *node, unsigned width, char *buf) {
  struct my_type *self = RBT_CONTAINER_OF (node, struct my_type, rbt_node);
  snprintf (buf, width+1, "%*d", width, data->key);
}

rbt_print (tree, my_print_node, 3, stdout);
```

