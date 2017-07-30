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
 *  - make_fibheap()		allocs. a heap.
 *  - fibheap_is_empty()	asserts if the heap is empty.
 *  - fibheap_insert()		inserts a node into the root list.
 *  - fibheap_minimum()		gets the node with the min. key.
 *  - fibheap_extract_min()	detaches the min. node, then consolidates the heap.
 *  - fibheap_union()		concats. the root lists of two heaps.
 *  - fibheap_decrease()	moves the decreased node to the root list.
 *  - fibheap_delete()		deletes a node, then consolidates the heap.
 *
 * [1] "Introduction to Algorithms", 3rd ed, ch. 19 -- Fibonacci Heaps, by CLRS.
 */

#ifndef FIBHEAP_H_
#define FIBHEAP_H_

#include <stdbool.h>		// For the bool type.
#include <stdlib.h>		// For malloc().
#include <math.h>		// For sqrt(), floor(), and log().

/* CAUTION! Assumes the list doesn't change order. */
#define FIBHEAP_FOREACH(runner, list, nodes, index)				\
										\
	struct fibheap_node *(runner), *next;					\
	int n = (nodes);							\
										\
	for ((runner) = (list), next = (runner)->right, (index) = 0;		\
	     (index) < n; (runner) = next, next = next->right, (index)++)

/* Nodes are assorted in several circular, doubly linked lists. A node points
 * up to its parent, down to its child(ren), and left and right to its siblings
 * (if any). Nodes in the "root" list have no parent. */
struct fibheap_node {
	struct fibheap_node *parent;
	struct fibheap_node *child;
	struct fibheap_node *left;
	struct fibheap_node *right;

	/* Holds the "value" of the node. */
	void                *value;
	int                 degree;
	bool                mark;
};

/* For comparing any two nodes. Clients have to define this. Returns negative if
 * a has higher priority than b. */
typedef int (*fibheap_cmp_t)(void *a, void *b);

/* Simply consists of a pointer to the node with the minimal value (which is
 * always at the top of the heap) and the number of nodes in the heap. */
struct fibheap {
	struct fibheap_node *min;

	/* For embedding the comparison function in the tree. */
	fibheap_cmp_t       cmp;

	int                 n;
};

static inline struct fibheap_node *make_fibheap_node(void *value)
{
	struct fibheap_node *node = malloc(sizeof(struct fibheap_node));

	node->value = value;

	return node;
}

/* Upper bound on the degree of any node of an n-node Fibonacci heap. */
static inline int ubdeg(int n)
{
	double phi = (1 + sqrt(5)) / 2;

	return (int) (floor(log(n) / log(phi)));
}

static inline void list_insert(struct fibheap_node *u, struct fibheap_node *v)
{
	u->left->right = v;
	v->right       = u;
	v->left        = u->left;
	u->left        = v;
}

static inline void list_remove(struct fibheap_node *u)
{
	u->left->right = u->right;
	u->right->left = u->left;
}

/* For circular lists only; otherwise could loop indefinitely. */
static inline int list_get_size(struct fibheap_node *x)
{
	struct fibheap_node *runner = x;
	int sz = 0;

	do {
		sz++;
		runner = runner->right;
	} while (runner != x);

	return sz;
}

static inline void link(struct fibheap_node *y, struct fibheap_node *x)
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

#define FIBHEAP_FOREACH_NODE_IN_ROOT_LIST(w, fibheap, index)			\
	FIBHEAP_FOREACH((w), (fibheap)->min, list_get_size((fibheap)->min),	\
			(index))

static inline void consolidate(struct fibheap *h)
{
	struct fibheap_node **A, *x, *y, *tmp;
	int i, d, maxdeg = ubdeg(h->n);

	/* Let A[0..D(H.n)] be a new array. */
	A = malloc((maxdeg + 1) * sizeof(struct fibheap_node *));

	for (i = 0; i <= maxdeg; i++)
		A[i] = NULL;

	FIBHEAP_FOREACH_NODE_IN_ROOT_LIST(w, h, i) {
		x = w;
		d = x->degree;

		while (A[d]) {
			y = A[d]; // Another node with the same degree as x.

			if (h->cmp(y->value, x->value) < 0) {
				/* Exchange x with y. */
				tmp = x;
				x   = y;
				y   = tmp;
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

				if (h->cmp(A[i]->value, h->min->value) < 0)
					h->min = A[i];
			}
			A[i]->parent = NULL;
		}
	}
	free(A);
}

static inline void cut(struct fibheap *h, struct fibheap_node *x,
		       struct fibheap_node *y)
{
	/* Remove x from the child list of y, decrementing y.degree. */
	list_remove(x);
	y->degree--;

	/* Add x to the root list of h. */
	list_insert(h->min, x);

	x->parent = NULL;
	x->mark   = false;
}

static inline void cascading_cut(struct fibheap *h, struct fibheap_node *y)
{
	struct fibheap_node *z = y->parent;

	if (z) {
		if (!y->mark) {
			y->mark = true;
		} else {
			cut(h, y, z);
			cascading_cut(h, z);
		}
	}
}

static inline struct fibheap *make_fibheap(fibheap_cmp_t cmp)
{
	struct fibheap *heap = malloc(sizeof(struct fibheap));

	heap->min = NULL;
	heap->cmp = cmp;
	heap->n   = 0;

	return heap;
}

static inline bool fibheap_is_empty(struct fibheap *h)
{
	return h->min == NULL;
}

static inline void fibheap_insert(struct fibheap *h, struct fibheap_node *x)
{
	x->parent = NULL;
	x->child  = NULL;
	x->degree = 0;
	x->mark   = false;

	if (!h->min) {
		/* Create a root list for h containing just x. */
		x->left  = x;
		x->right = x;
		h->min   = x;
	} else {
		/* Insert x into h's root list. */
		list_insert(h->min, x);

		if (h->cmp(x->value, h->min->value) < 0)
			h->min = x;
	}
	h->n++;
}

static inline struct fibheap_node *fibheap_minimum(struct fibheap *h)
{
	return h->min;
}

#define FIBHEAP_FOREACH_CHILD(runner, parent, index)				\
	FIBHEAP_FOREACH((runner), (parent)->child, (parent)->degree, (index))

static inline struct fibheap_node *fibheap_extract_min(struct fibheap *h)
{
	struct fibheap_node *z = h->min;
	int i;

	if (z) {
		if (z->child) {
			FIBHEAP_FOREACH_CHILD(x, z, i) {
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
			consolidate(h);
		}
		h->n--;
	}
	return z;
}

static inline struct fibheap *fibheap_union(struct fibheap *heap1,
					    struct fibheap *heap2)
{
	struct fibheap_node *h1, *h2, *leftmost, *rightmost;

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

	if (heap1->cmp(h2->value, h1->value) < 0)
		heap1->min = h2;

	heap1->n += heap2->n;

	/* Should heap2 be destroyed? */
	return heap1;
}

/* Does _not_ "decrease" the value of the node; clients are responsible for
 * this, as "value" is dependent on usage. */
static inline int fibheap_decrease(struct fibheap *h, struct fibheap_node *x)
{
	struct fibheap_node *y = x->parent;

	if (y && h->cmp(x->value, y->value) < 0) {
		cut(h, x, y);
		cascading_cut(h, y);
	}
	if (h->cmp(x->value, h->min->value) < 0)
		h->min = x;

	return 0;
}

static inline void fibheap_delete(struct fibheap *h, struct fibheap_node *x)
{
	struct fibheap_node *y = x->parent;

	/* "Decreasing" the value of the deleted node to the minimum possible
	 * value is equivalent to cutting the node from its parent and cascade
	 * cutting the parent unconditionally. */
	if (y) {
		cut(h, x, y);
		cascading_cut(h, y);
	}
	/* Also, the node is marked as the one with the minimum value. */
	h->min = x;

	/* Finally, the node is extracted from the heap. */
	fibheap_extract_min(h);
}

#endif // FIBHEAP_H_

