/*
 * fibheap.h: Implementation of Fibonacci heaps, which are data structures
 *            supporting a set of operations that constitutes what are known
 *            as "mergeable heaps", most of which run in const. amortized time.
 *
 *            D'après [1]: "A _Fibonacci heap_ is a collection of rooted trees
 *            that are _min-heap ordered_. That is, each tree obeys the _min-
 *            heap property_: the key of a node is greater than or equal to the
 *            key of its parent."
 *
 *            The complexity of programming Fibonacci heaps is what makes their
 *            study compelling mainly from the theoretical perspective. The
 *            interested reader is encouraged to check [1], pg. 506, under
 *            "Fibonacci heaps in theory and practice" for a relevant
 *            discussion.
 *
 * Summary of operations for Fibonacci heaps:
 *
 *  - make_fibheap()            Allocs. a heap.
 *  - fibheap_is_empty()        Asserts if the heap is empty.
 *  - fibheap_insert()          Inserts a node into the root list.
 *  - fibheap_minimum()         Gets the node with the min. key.
 *  - fibheap_extract_min()     Detaches the min. node, then consolidates the heap.
 *  - fibheap_union()           Concats. the root lists of two heaps.
 *  - fibheap_decrease()        Moves the decreased node to the root list.
 *  - fibheap_delete()          deletes a node, then consolidates the heap.
 *
 * [1] "Introduction to Algorithms", 3rd ed, ch. 19 -- Fibonacci Heaps, by CLRS.
 */

#ifndef FIBHEAP_H_
#define FIBHEAP_H_

#include <limits.h>             // For CHAR_BIT
#include <stdlib.h>             // For malloc().
#include <math.h>               // For sqrt(), floor(), and log().

#include "list.h"               // For linked list struct. and ops.

/* Wrappers around linked list ops. */
#define FIBHEAP_REMOVE_FROM_LIST(_node)                                         \
	list_del(&(_node)->list)

#define FIBHEAP_INSERT_INTO_LIST(_node, _list)                                  \
	list_add(&(_node)->list, &(_list))

#define FIBHEAP_INSERT_INTO_LIST_TAIL(_node, _list)                             \
	list_add_tail(&(_node)->list, &(_list))

#define FIBHEAP_GET_MIN_NODE(_heap)                                             \
	list_first_entry(&(_heap)->root_list, struct fibheap_node, list)

#define FIBHEAP_BECOME_MIN_NODE(_node, _heap)                                   \
	list_move(&(_node)->list, &(_heap)->root_list)

/* For manipulating nodes' mark and degree. */
#define last_bit (sizeof(unsigned int) * CHAR_BIT - 1)

#define FIBHEAP_GET_DEGREE(_node) ((_node)->degree & ~(1 << last_bit))

#define FIBHEAP_SET_MARK(_node)   ((_node)->degree |= 1 << last_bit)

#define FIBHEAP_RESET_MARK(_node) ((_node)->degree &= ~(1 << last_bit))

#define FIBHEAP_TEST_MARK(_node)  ((_node)->degree & 1 << last_bit)

/* For comparing any two nodes. Clients have to define this. Should return
 * negative if a has higher priority than b. */
typedef int (*fibheap_cmp_t)(void *a, void *b);

/* Nodes are accessed by means of the heap's root list, which is implemented as
 * a circular, doubly-linked list whose head node connects to the heap node with
 * the minimal value. For convenience, the comparison function is also embedded
 * within the heap. */
struct fibheap {
	/* Since the head of the list leads to the node with the minimal value,
	 * insertions to the root list should be done at the tail. Only when the
	 * list is empty can one insert node at the head.
	 *
	 * For setting a new min. node, use FIBHEAP_BECOME_MIN_NODE(). */
	struct list_head root_list;

	fibheap_cmp_t    cmp;

	/* The number of nodes in the heap. */
	int              n;
};

/* Nodes are assorted in several circular, doubly linked lists. Conceptually, a
 * node points up to its parent, down to its child(ren), and left and right to
 * its siblings (if any). Nodes in the "root" list have no parent. */
struct fibheap_node {
	struct fibheap_node *parent;

	struct list_head    child; // Connects the node to its children.
	struct list_head    list;  // Connects the node to its siblings.

	/* A single bit (the leftmost one) of this member is used as the boolean
	 * mark needed for the cascading cut op. Thus, some space per node is
	 * saved, at the expense of limiting the range of possible degrees to
	 * whatever number you can represent with an integer shortened by one
	 * bit.
	 *
	 * The degree can be increased/decreased in the usual way, just be
	 * careful not to overflow to the mark bit! */
	unsigned int        degree;

	/* Holds the "value" of the node. */
	void                *value;
};

static inline struct fibheap_node *make_fibheap_node(void *value)
{
	struct fibheap_node *node = malloc(sizeof(struct fibheap_node));

	node->parent = NULL;

	INIT_LIST_HEAD(&node->child);
	INIT_LIST_HEAD(&node->list);

	node->value  = value;
	node->degree = 0;

	return node;
}

/* Upper bound on the degree of any node of an n-node Fibonacci heap. */
static inline int ubdeg(int n)
{
	double phi = (1 + sqrt(5)) / 2;

	return (int) (floor(log(n) / log(phi)));
}

static inline void link(struct fibheap_node *y, struct fibheap_node *x)
{
	/* Remove y from the root list of H. */
	FIBHEAP_REMOVE_FROM_LIST(y);

	/* Make y a child of x, incrementing x.degree. */
	FIBHEAP_INSERT_INTO_LIST(y, x->child);
	y->parent = x;
	x->degree++;

	FIBHEAP_RESET_MARK(y);
}

static inline int fibheap_is_empty(struct fibheap *);

static inline void consolidate(struct fibheap *h)
{
	struct fibheap_node **A, *x, *y, *next, *tmp, *min;
	int i, d, maxdeg = ubdeg(h->n);

	/* Let A[0..D(H.n)] be a new array. */
	A = malloc((maxdeg + 1) * sizeof(struct fibheap_node *));

	for (i = 0; i <= maxdeg; i++)
		A[i] = NULL;

	list_for_each_entry_safe(x, next, &h->root_list, list) {
		d = FIBHEAP_GET_DEGREE(x);

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
	INIT_LIST_HEAD(&h->root_list);

	for (i = 0; i <= maxdeg; i++) {
		if (A[i]) {
			if (fibheap_is_empty(h)) {
				FIBHEAP_INSERT_INTO_LIST(A[i], h->root_list);
			} else {
				FIBHEAP_INSERT_INTO_LIST_TAIL(A[i],
							      h->root_list);

				min = FIBHEAP_GET_MIN_NODE(h);

				if (h->cmp(A[i]->value, min->value) < 0)
					FIBHEAP_BECOME_MIN_NODE(A[i], h);
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
	FIBHEAP_REMOVE_FROM_LIST(x);
	y->degree--;

	/* Add x to the root list of h. */
	FIBHEAP_INSERT_INTO_LIST_TAIL(x, h->root_list);

	x->parent = NULL;
	FIBHEAP_RESET_MARK(x);
}

static inline void cascading_cut(struct fibheap *h, struct fibheap_node *y)
{
	struct fibheap_node *z = y->parent;

	if (z) {
		if (FIBHEAP_TEST_MARK(y)) {
			cut(h, y, z);
			cascading_cut(h, z);
		} else {
			FIBHEAP_SET_MARK(y);
		}
	}
}

/* --- API --- */

static inline struct fibheap *make_fibheap(fibheap_cmp_t cmp)
{
	struct fibheap *heap = malloc(sizeof(struct fibheap));

	INIT_LIST_HEAD(&heap->root_list);

	heap->cmp = cmp;
	heap->n   = 0;

	return heap;
}

static inline int fibheap_is_empty(struct fibheap *h)
{
	return list_empty(&h->root_list);
}

static inline void fibheap_insert(struct fibheap *h, struct fibheap_node *x)
{
	struct fibheap_node *min;

	if (fibheap_is_empty(h)) {
		FIBHEAP_INSERT_INTO_LIST(x, h->root_list);
	} else {
		FIBHEAP_INSERT_INTO_LIST_TAIL(x, h->root_list);

		min = FIBHEAP_GET_MIN_NODE(h);

		if (h->cmp(x->value, min->value) < 0)
			FIBHEAP_BECOME_MIN_NODE(x, h);
	}
	h->n++;
}

static inline struct fibheap_node *fibheap_minimum(struct fibheap *h)
{
	return FIBHEAP_GET_MIN_NODE(h);
}

static inline struct fibheap_node *fibheap_extract_min(struct fibheap *h)
{
	struct fibheap_node *x, *z, *next;

	if (fibheap_is_empty(h))
		return NULL;

	z = FIBHEAP_GET_MIN_NODE(h);

	if (!list_empty(&z->child)) {
		list_for_each_entry_safe(x, next, &z->child, list) {
			/* Add x to the root list of h. */
			FIBHEAP_INSERT_INTO_LIST_TAIL(x, h->root_list);
			x->parent = NULL;
		}
	}
	/* Remove z from the root list of h. */
	FIBHEAP_REMOVE_FROM_LIST(z);

	if (!fibheap_is_empty(h))
		consolidate(h);

	h->n--;

	return z;
}

static inline struct fibheap *fibheap_union(struct fibheap *heap1,
					    struct fibheap *heap2)
{
	struct fibheap_node *h1, *h2;

	if (!heap2 || (heap2 && fibheap_is_empty(heap2)))
		return heap1;

	if (!heap1 || (heap1 && fibheap_is_empty(heap1)))
		return heap2;

	h1 = FIBHEAP_GET_MIN_NODE(heap1);
	h2 = FIBHEAP_GET_MIN_NODE(heap2);

	list_splice_tail(&heap2->root_list, &heap1->root_list);

	if (heap1->cmp(h2->value, h1->value) < 0)
		FIBHEAP_BECOME_MIN_NODE(h2, heap1);

	heap1->n += heap2->n;

	/* Should heap2 be destroyed? */

	return heap1;
}

/* Does _not_ "decrease" the value of the node; clients are responsible for
 * this, as "value" is dependent on usage. */
static inline void fibheap_decrease(struct fibheap *h, struct fibheap_node *x)
{
	struct fibheap_node *y = x->parent, *min;

	if (y && h->cmp(x->value, y->value) < 0) {
		cut(h, x, y);
		cascading_cut(h, y);
	}
	min = FIBHEAP_GET_MIN_NODE(h);

	if (h->cmp(x->value, min->value) < 0)
		FIBHEAP_BECOME_MIN_NODE(x, h);
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
	FIBHEAP_BECOME_MIN_NODE(x, h);

	/* Finally, the node is extracted from the heap. */
	fibheap_extract_min(h);
}

#endif // FIBHEAP_H_

