/*
 * rbtree.h: Implementation of red-black trees, which are a special kind of
 *           binary tree whose structure is approximately _balanced_. As a
 *           result, its basic operations are guaranteed to take logarithmic
 *           time in the worst case.
 *
 *           See [1] for further details on Red-Black trees and their
 *           operations.
 *
 * Summary of operations for red-black trees:
 *
 *  - make_rbtree()		allocs. a tree.
 *  - rbtree_search()		looks for a node with a specific key.
 *  - rbtree_minimum()		gets the node with the minimal key.
 *  - rbtree_maximum()		gets the node with the maximal key.
 *  - rbtree_predecessor()	gets the preceding node for a specific one.
 *  - rbtree_successor()	gets the following node for a specific one.
 *  - rbtree_insert()		inserts a node, then rebalances the tree.
 *  - rbtree_delete()		deletes a node, then rebalances the tree.
 *  - rbtree_destroy()		deallocs. the tree and all its nodes.
 *
 * [1] "Introduction to Algorithms", 3rd ed, ch. 13 -- Red-Black Trees, by CLRS.
 */

#ifndef RBTREE_H_
#define RBTREE_H_

#include <stdlib.h>		// For malloc().

/* Node colors can be either red or black. */
typedef enum { RED = 0, BLACK } color_t;

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

/* For comparing any two nodes. Clients have to define this. Returns negative if
 * a has a smaller key than b, positive if the other way around, and zero if
 * they're equal. */
typedef int (*rbtree_cmp_t)(void *a, void *b);

/* Simply consists of a pointer to the root node and the number of nodes in the
 * tree. Also, NIL is contained within the tree. */
struct rbtree {
	struct rbtree_node *root;
	struct rbtree_node *nil;

	/* For embedding the comparison function in the tree. */
	rbtree_cmp_t       cmp;

	int                n;
};

static inline struct rbtree_node *make_rbtree_node(void *value)
{
	struct rbtree_node *node = malloc(sizeof(struct rbtree_node));

	node->parent = NULL;
	node->left   = NULL;
	node->right  = NULL;

	node->value  = value;
	node->color  = RED;

	return node;
}

static inline struct rbtree_node *make_rbtree_sentinel()
{
	struct rbtree_node *sentinel = malloc(sizeof(struct rbtree_node));

	sentinel->parent = NULL;
	sentinel->left   = NULL;
	sentinel->right  = NULL;

	sentinel->value  = NULL;
	sentinel->color  = BLACK;

	return sentinel;
}

#define ROTATE(_rbtree, x, dir, opp)						\
										\
	do {									\
		struct rbtree_node *y;						\
										\
		y        = (x)->opp;						\
		(x)->opp = y->dir;						\
										\
		if (y->dir != (_rbtree)->nil)					\
			y->dir->parent = (x);					\
										\
		y->parent = (x)->parent;					\
										\
		if ((x)->parent == (_rbtree)->nil)				\
			(_rbtree)->root = y;					\
		else if ((x) == (x)->parent->dir)				\
			(x)->parent->dir = y;					\
		else								\
			(x)->parent->opp = y;					\
										\
		y->dir      = (x);						\
		(x)->parent = y;						\
	} while (0)

#define ROTATE_LEFT(_rbtree, x)							\
	ROTATE((_rbtree), (x), left, right)

#define ROTATE_RIGHT(_rbtree, x)						\
	ROTATE((_rbtree), (x), right, left)

static inline void rotate_left(struct rbtree *t, struct rbtree_node *x)
{
	ROTATE_LEFT(t, x);
}

static inline void rotate_right(struct rbtree *t, struct rbtree_node *x)
{
	ROTATE_RIGHT(t, x);
}

#define INSERT_FIXUP(_rbtree, y, z, dir, opp)					\
										\
	do {									\
		(y) = (z)->parent->parent->dir;					\
										\
		if ((y)->color == RED) {					\
			(z)->parent->color         = BLACK;			\
			(y)->color                 = BLACK;			\
			(z)->parent->parent->color = RED;			\
			(z)                        = (z)->parent->parent;	\
		} else {							\
			if ((z) == (z)->parent->dir) {				\
				(z) = (z)->parent;				\
				rotate_ ## opp((_rbtree), (z));			\
			}							\
			(z)->parent->color         = BLACK;			\
			(z)->parent->parent->color = RED;			\
			rotate_ ## dir((_rbtree), (z)->parent->parent);		\
		}								\
	} while (0)

static inline void insert_fixup(struct rbtree *t, struct rbtree_node *z)
{
	struct rbtree_node *y;

	while (z->parent->color == RED) {
		if (z->parent == z->parent->parent->left)
			INSERT_FIXUP(t, y, z, right, left);
		else
			INSERT_FIXUP(t, y, z, left, right);
	}
	t->root->color = BLACK;
}

#define DELETE_FIXUP(_rbtree, w, x, dir, opp)					\
										\
	do {									\
		(w) = (x)->parent->opp;						\
										\
		if ((w)->color == RED) {					\
			(w)->color         = BLACK;				\
			(x)->parent->color = RED;				\
										\
			rotate_ ## dir((_rbtree), (x)->parent);			\
			(w) = (x)->parent->opp;					\
		}								\
		if ((w)->dir->color == BLACK && (w)->opp->color == BLACK) {	\
			(w)->color = RED;					\
			(x)        = (x)->parent;				\
		} else {							\
			if ((w)->opp->color == BLACK) {				\
				(w)->dir->color = BLACK;			\
				(w)->color      = RED;				\
										\
				rotate_ ## opp((_rbtree), (w));			\
				(w) = (x)->parent->opp;				\
			}							\
			(w)->color         = (x)->parent->color;		\
			(x)->parent->color = BLACK;				\
			(w)->opp->color    = BLACK;				\
										\
			rotate_ ## dir((_rbtree), (x)->parent);			\
			(x) = _rbtree->root;					\
		}								\
	} while (0)

static inline void delete_fixup(struct rbtree *t, struct rbtree_node *x)
{
	struct rbtree_node *w;

	while (x != t->root && x->color == BLACK) {
		if (x == x->parent->left)
			DELETE_FIXUP(t, w, x, left, right);
		else
			DELETE_FIXUP(t, w, x, right, left);
	}
	x->color = BLACK;
}

static inline void transplant(struct rbtree *t, struct rbtree_node *u,
			      struct rbtree_node *v)
{
	if (u->parent == t->nil)
		t->root = v;
	else if (u == u->parent->left)
		u->parent->left = v;
	else
		u->parent->right = v;

	v->parent = u->parent;
}

static inline struct rbtree_node *__rbtree_search(struct rbtree *t,
						  struct rbtree_node *x,
						  void *value)
{
	while (x != t->nil && t->cmp(value, x->value)) {
		if (t->cmp(value, x->value) < 0)
			x = x->left;
		else
			x = x->right;
	}
	return x;
}

/* __min and __max are so short there's no point in turning them into macros. */
static inline struct rbtree_node *__rbtree_minimum(struct rbtree *t,
						   struct rbtree_node *x)
{
	while (x->left != t->nil)
		x = x->left;

	return x;
}

static inline struct rbtree_node *__rbtree_maximum(struct rbtree *t,
						   struct rbtree_node *x)
{
	while (x->right != t->nil)
		x = x->right;

	return x;
}

static inline void __rbtree_destroy(struct rbtree *t, struct rbtree_node *x)
{
	if (x != t->nil) {
		__rbtree_destroy(t, x->left);
		__rbtree_destroy(t, x->right);
		free(x);
	}
}

static inline struct rbtree *make_rbtree(rbtree_cmp_t cmp)
{
	struct rbtree *tree = malloc(sizeof(struct rbtree));

	tree->nil  = make_rbtree_sentinel();
	tree->root = tree->nil;
	tree->cmp  = cmp;
	tree->n    = 0;

	return tree;
}

/* Iterative search. Employs the custom comparison function for determining if a
 * match was found. There probably are different ways to implement this, but
 * this should be enough for now. */
static inline struct rbtree_node *rbtree_search(struct rbtree *t,
						void *value)
{
	return __rbtree_search(t, t->root, value);
}

/* Iterative lookup. A different way to implement this would be caching the
 * node with the minimum value, for allowing fast retrieval (as done in the
 * Linux kernel.) */
static inline struct rbtree_node *rbtree_minimum(struct rbtree *t)
{
	return __rbtree_minimum(t, t->root);
}

static inline struct rbtree_node *rbtree_maximum(struct rbtree *t)
{
	return __rbtree_maximum(t, t->root);
}

#define XXXCESSOR(_rbtree, x, dir, func)					\
										\
	struct rbtree_node *y;							\
										\
	if ((x)->dir != (_rbtree)->nil)						\
		return __rbtree_ ## func ## imum((_rbtree), (x)->dir);		\
										\
	y = (x)->parent;							\
										\
	while (y != (_rbtree)->nil && (x) == y->dir) {				\
		(x) = y;							\
		y   = y->parent;						\
	}									\
	return y

#define PREDECESSOR(_rbtree, x)							\
	XXXCESSOR((_rbtree), (x), left, max)

#define SUCCESSOR(_rbtree, x)							\
	XXXCESSOR((_rbtree), (x), right, min)

static inline struct rbtree_node *rbtree_predecessor(struct rbtree *t,
						     struct rbtree_node *x)
{
	PREDECESSOR(t, x);
}

static inline struct rbtree_node *rbtree_successor(struct rbtree *t,
						   struct rbtree_node *x)
{
	SUCCESSOR(t, x);
}

static inline void rbtree_insert(struct rbtree *t, struct rbtree_node *z)
{
	struct rbtree_node *x, *y;

	y = t->nil;
	x = t->root;

	while (x != t->nil) {
		y = x;

		if (t->cmp(z->value, x->value) < 0)
			x = x->left;
		else
			x = x->right;
	}
	z->parent = y;

	if (y == t->nil)
		t->root = z;
	else if (t->cmp(z->value, y->value) < 0)
		y->left = z;
	else
		y->right = z;

	z->left  = t->nil;
	z->right = t->nil;
	z->color = RED;

	insert_fixup(t, z);

	t->n++;
}

static inline void rbtree_delete(struct rbtree *t, struct rbtree_node *z)
{
	struct rbtree_node *x, *y;
	color_t y_color;

	y       = z;
	y_color = y->color;

	if (z->left == t->nil) {
		x = z->right;
		transplant(t, z, z->right);
	} else if (z->right == t->nil) {
		x = z->left;
		transplant(t, z, z->left);
	} else {
		y       = __rbtree_minimum(t, z->right);
		y_color = y->color;
		x       = y->right;

		if (y->parent == z) {
			x->parent = y;
		} else {
			transplant(t, y, y->right);
			y->right         = z->right;
			y->right->parent = y;
		}
		transplant(t, z, y);
		y->left         = z->left;
		y->left->parent = y;
		y->color        = z->color;
	}
	if (y_color == BLACK)
		delete_fixup(t, x);

	t->n--;
}

/* Recursive destruction. */
static inline void rbtree_destroy(struct rbtree *t)
{
	__rbtree_destroy(t, t->root);
	free(t->nil);
	free(t);
}

#endif // RBTREE_H_

