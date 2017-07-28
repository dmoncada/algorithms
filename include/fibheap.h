/*
 * fibheap.h: Implementation of Fibonacci heaps, which are data structures
 *	      supporting a set of operations that constitutes what are known
 *	      as "mergeable heaps", most of which run in const. amortized time.
 *
 *	      D'après [1]: "A _Fibonacci heap_ is a collection of rooted trees
 *	      that are _min-heap ordered_. That is, each tree obeys the _min-
 *	      heap property_: the key of a node is greater than or equal to the
 *	      key of its parent."
 *
 *	      The complexity of programming Fibonacci heaps is what makes their
 *	      study compelling mainly from the theoretical perspective. The
 *	      interested reader is encouraged to check [1], pg. 506, under
 *	      "Fibonacci heaps in theory and practice" for a relevant
 *	      discussion.
 *
 * Summary of operations for Fibonacci heaps:
 *
 *  - make_fib_heap()			allocates a heap.
 *  - fib_heap_insert()			inserts a node into the root list.
 *  - fib_heap_minimum()		returns the node with the min. key.
 *  - fib_heap_extract_min()		detaches the min. key, then consolidates the heap.
 *  - fib_heap_union()			concatenates the root lists of two heaps.
 *  - fib_heap_decrease_key()		needs a ref. to the modified node.
 *  - fib_heap_delete()			also needs a ref. to the deleted node.
 *
 * [1] "Introduction to Algorithms", 3rd ed, ch. 19 -- Fibonacci Heaps, by CLRS.
 */

#ifndef FIBHEAP_H_
#define FIBHEAP_H_

#include <stdbool.h>			// For the bool type.
#include <stdlib.h>			// For malloc().
#include <math.h>			// For sqrt(), floor(), and log().

/* CAUTION! Assumes the list doesn't change order. */
#define FIB_HEAP_FOREACH(runner, list)						\
										\
	struct heap_node *runner, *next;					\
	bool first;								\
										\
	for (runner = list, next = runner->right, first = true;			\
	     first || runner != list;						\
	     first = false, runner = next, next = next->right)

/* Nodes are assorted in several circular, doubly linked lists. A node points
 * up to its parent, down to its child(ren), and left and right to its siblings
 * (if any). Nodes in the "root" list have no parent. */
struct heap_node {
	struct heap_node *parent;
	struct heap_node *child;
	struct heap_node *left;
	struct heap_node *right;

	/* Holds the "value" of the node. */
	void             *value;
	int              degree;
	bool             mark;
};

/* Simply consists of a pointer to the node with the minimal key (which is
 * always at the top of the heap) and the number of nodes in the heap. */
struct heap {
	struct heap_node *min;

	int              n;
};

/* For comparing any two nodes. Clients have to define this. Return 1 if a has
 * higher priority, and 0 otherwise. */
typedef int (*fib_heap_cmp_t)(struct heap_node *a, struct heap_node *b);

static inline struct heap_node *make_heap_node(void *value)
{
	struct heap_node *node =
		(struct heap_node *) malloc(sizeof(struct heap_node));

	node->value = value;

	return node;
}

/* Upper bound on the degree of any node of an n-node Fibonacci heap. */
static inline int ubdeg(int n)
{
	double phi = (1 + sqrt(5)) / 2;

	return (int) (floor(log(n) / log(phi)));
}

static inline void list_insert(struct heap_node *u, struct heap_node *v)
{
	u->left->right = v;
	v->right       = u;
	v->left        = u->left;
	u->left        = v;
}

static inline void list_remove(struct heap_node *u)
{
	u->left->right = u->right;
	u->right->left = u->left;
}

/* For circular lists only; otherwise could loop indefinitely. */
static inline int list_get_size(struct heap_node *x)
{
	struct heap_node *runner = x;
	int sz = 0;

	do {
		sz++;
		runner = runner->right;
	} while (runner != x);

	return sz;
}

static inline void link(struct heap_node *y, struct heap_node *x)
{
	/* Remove y from the root list of H. */
	list_remove(y);

	/* Make y a child of x, incrementing x.degree. */
	if (!x->child) {
		x->child = y;
		y->left  = y;
		y->right = y;
	} else {
		list_insert(x->child, y);
	}
	y->parent = x;
	x->degree++;

	y->mark = false;
}

/* Uses the length of the list for knowing when to stop. */
#define FIB_HEAP_FOREACH_NODE_IN_ROOT_LIST(runner, list, index)			\
										\
	struct heap_node *runner, *next;					\
	int n = list_get_size(list);						\
										\
	for (runner = list, next = runner->right, index = 0; index < n;		\
	     runner = next, next = next->right, index++)

static inline void consolidate(struct heap *h, fib_heap_cmp_t cmp)
{
	int i, d, maxdeg = ubdeg(h->n);
	struct heap_node **A, *x, *y, *tmp;

	/* Let A[0..D(H.n)] be a new array. */
	A = (struct heap_node **)
		malloc((maxdeg + 1) * sizeof(struct heap_node));

	for (i = 0; i <= maxdeg; i++)
		A[i] = NULL;

	FIB_HEAP_FOREACH_NODE_IN_ROOT_LIST(w, h->min, i) {
		x = w;
		d = x->degree;

		while (A[d]) {
			y = A[d]; // Another node with the same degree as x.

			if (cmp(y, x)) {
				/* Exchange x with y. */
				tmp = x;
				x = y;
				y = tmp;
			}
			link(y, x);
			A[d] = NULL;
			d++;
		}
		A[d] = x;
	}
	h->min = NULL;

	for (i = 0; i <= maxdeg; i++) {
		if (A[i]) {
			if (!h->min) {
				/* Create a root list for h containing just A[i]. */
				h->min      = A[i];
				A[i]->left  = A[i];
				A[i]->right = A[i];
			} else {
				/* Insert A[i] into h's root list. */
				list_insert(h->min, A[i]);

				if (cmp(A[i], h->min))
					h->min = A[i];
			}
			A[i]->parent = NULL;
		}
	}
	free(A);
}

static inline void cut(struct heap *h, struct heap_node *x, struct heap_node *y)
{
	/* Remove x from the child list of y, decrementing y.degree. */
	list_remove(x);
	y->degree--;

	/* Add x to the root list of h. */
	list_insert(h->min, x);
	x->parent = NULL;
	x->mark = false;
}

static inline void cascading_cut(struct heap *h, struct heap_node *y)
{
	struct heap_node *z = y->parent;

	if (z) {
		if (!y->mark) {
			y->mark = true;
		} else {
			cut(h, y, z);
			cascading_cut(h, z);
		}
	}
}

static inline struct heap *make_fib_heap()
{
	struct heap *h = (struct heap *) malloc(sizeof(struct heap));

	h->min = NULL;
	h->n   = 0;

	return h;
}

static inline bool fib_heap_is_empty(struct heap *h)
{
	return h->min == NULL;
}

static inline void fib_heap_insert(struct heap *h, struct heap_node *x,
				   fib_heap_cmp_t cmp)
{
	x->degree = 0;
	x->parent = NULL;
	x->child  = NULL;
	x->mark   = false;

	if (!h->min) {
		/* Create a root list for h containing just x. */
		x->left  = x;
		x->right = x;
		h->min   = x;
	} else {
		/* Insert x into h's root list. */
		list_insert(h->min, x);

		if (cmp(x, h->min))
			h->min = x;
	}
	h->n++;
}

static inline struct heap_node *fib_heap_minimum(struct heap *h)
{
	return h->min;
}

#define FIB_HEAP_FOREACH_CHILD(runner, parent)					\
	FIB_HEAP_FOREACH(runner, parent->child)

static inline struct heap_node *fib_heap_extract_min(struct heap *h,
						     fib_heap_cmp_t cmp)
{
	struct heap_node *z = h->min;

	if (z) {
		if (z->child) {
			FIB_HEAP_FOREACH_CHILD(x, z) {
				/* Add x to the root list of h. */
				list_insert(h->min, x);
				x->parent = NULL;
			}
		}
		/* Remove z from the root list of h. */
		list_remove(z);

		if (z == z->right) {
			h->min = NULL;
		} else {
			h->min = z->right;
			consolidate(h, cmp);
		}
		h->n--;
	}
	return z;
}

static inline struct heap *fib_heap_union(struct heap *heap1,
					  struct heap *heap2,
					  fib_heap_cmp_t cmp)
{
	struct heap_node *h1, *h2, *leftmost, *rightmost;

	if (!heap2 || (heap2 && !heap2->min))
		return heap1;

	if (!heap1 || (heap1 && !heap1->min))
		return heap2;

	h1 = heap1->min;
	h2 = heap2->min;

	/* Concatenate the root list of heap2 with the root list of heap1. */
	leftmost         = h1->right;
	rightmost        = h2->left;

	h1->right->left  = NULL;
	h1->right        = NULL;

	h2->left->right  = NULL;
	h2->left         = NULL;

	h1->right        = h2;
	h2->left         = h1;

	leftmost->left   = rightmost;
	rightmost->right = leftmost;

	if (cmp(h2, h1))
		heap1->min = h2;

	heap1->n += heap2->n;

	/* Should heap2 be destroyed? */
	return heap1;
}

/* FIXME: check if the new value for the node is actually smaller than what it
 *        had before. */
static inline int fib_heap_decrease(struct heap *h, struct heap_node *x,
				    fib_heap_cmp_t cmp)
{
	struct heap_node *y = x->parent;

	if (y && cmp(x, y)) {
		cut(h, x, y);
		cascading_cut(h, y);
	}
	if (cmp(x, h->min))
		h->min = x;

	return 0;
}

/* TODO: properly implement the delete operation. */
//static inline void fib_heap_delete(struct heap *h, struct heap_node *x)
//{
//}

#endif

