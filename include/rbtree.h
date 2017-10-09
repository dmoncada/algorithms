/*
 * rbtree.h: Implementation of red-black trees, which are a special kind of
 *           binary tree whose structure is approximately -balanced-. As a
 *           result, its basic operations are guaranteed to take logarithmic
 *           time in the worst case.
 *
 *           See [1] for further details on Red-Black trees and their
 *           operations.
 *
 * Summary of operations for red-black trees:
 *
 *  - make_rbtree()             Allocs. a tree.
 *  - make_rbtree_node()        Allocs. a tree node.
 *  - rbtree_search()           Looks for a node with a specific key.
 *  - rbtree_minimum()          Gets the node with the minimal key.
 *  - rbtree_maximum()          Gets the node with the maximal key.
 *  - rbtree_predecessor()      Gets the preceding node for a specific one.
 *  - rbtree_successor()        Gets the following node for a specific one.
 *  - rbtree_insert()           Inserts a node, then rebalances the tree.
 *  - rbtree_delete()           Deletes a node, then rebalances the tree.
 *  - rbtree_destroy()          Deallocs. the tree and all its nodes.
 *
 * [1] "Introduction to Algorithms", 3rd ed, ch. 13: Red-Black Trees, by CLRS.
 */

#ifndef RBTREE_H_
#define RBTREE_H_

#include <stdlib.h>             // For malloc().

struct rbtree_node;

/* Node colors can be either red or black. */
typedef enum { RED = 0, BLACK } color_t;

/* For comparing any two nodes. Clients have to define this. Returns negative if
 * a has a smaller key than b, positive if the other way around, and zero if
 * they're equal. */
typedef int (*rbtree_cmp)(void *, void *);

/* Simply consists of a pointer to the root node and the number of nodes in the
 * tree. Also, NIL is contained within the tree. */
struct rbtree {
	struct rbtree_node *root;
	struct rbtree_node *nil;

	rbtree_cmp         cmp;
	int                n;
};

/* As with regular binary trees, nodes point up to their parent and down to
 * their child(ren). In case the nodes had no children, they point down to a
 * special "sink" node termed "NIL". The parent of the root node is NIL. */
struct rbtree_node {
	struct rbtree_node *parent;
	struct rbtree_node *left;
	struct rbtree_node *right;

	/* Holds the "value" of the node. */
	void               *value;

	color_t            color;
};

/* --- API --- */

struct rbtree *make_rbtree(rbtree_cmp);

struct rbtree_node *make_rbtree_node(void *);

struct rbtree_node *rbtree_search(struct rbtree *, void *);

struct rbtree_node *rbtree_minimum(struct rbtree *);

struct rbtree_node *rbtree_maximum(struct rbtree *);

struct rbtree_node *rbtree_predecessor(struct rbtree *, struct rbtree_node *);

struct rbtree_node *rbtree_successor(struct rbtree *, struct rbtree_node *);

void rbtree_insert(struct rbtree *, struct rbtree_node *);

void rbtree_delete(struct rbtree *, struct rbtree_node *);

void rbtree_destroy(struct rbtree *);

#endif // RBTREE_H_
