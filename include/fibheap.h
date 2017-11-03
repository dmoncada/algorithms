/*
 * fibheap.h: Implementation of Fibonacci heaps, which are data structures
 *            supporting a set of operations that constitutes what are known as
 *            "mergeable heaps", most of which run in const. amortized time.
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
 *  - make_fibheap_node()       Allocs. a heap node.
 *  - fibheap_is_empty()        Asserts if the heap is empty.
 *  - fibheap_insert()          Inserts a node into the heap's root list.
 *  - fibheap_minimum()         Peeks at the top of the heap.
 *  - fibheap_extract_min()     Detaches the min. node, then consolidates the heap.
 *  - fibheap_union()           Concatenates the root lists of two heaps.
 *  - fibheap_decrease()        Moves the decreased node to the heap's root list.
 *  - fibheap_delete()          Deletes a node, then consolidates the heap.
 *
 * [1] "Introduction to Algorithms", 3rd ed, ch. 19: Fibonacci Heaps, by CLRS.
 */

#ifndef FIBHEAP_H_
#define FIBHEAP_H_

#include <limits.h>             // For CHAR_BIT
#include <stdlib.h>             // For malloc().
#include <math.h>               // For sqrt(), floor(), and log().

#include "list.h"               // For linked list struct. and ops.

/* For comparing any two nodes. Clients have to define this. Should return
 * negative if a has higher priority than b. */
typedef int (*fibheap_cmp)(void *, void *);

/* Nodes are accessed by means of the heap's root list, which is implemented as
 * a circular, doubly-linked list whose head node connects to the heap node with
 * the minimal value. For convenience, the comparison function is also embedded
 * within the heap. */
struct fibheap {
	/* Since the head of the list leads to the node with the minimal value,
	 * insertions to the root list should be done at the tail. Only when the
	 * list is empty can one insert node at the head.
	 *
	 * For setting a new min. node, use the BECOME_MIN_NODE() macro. */
	struct list_head root_list;

	fibheap_cmp      cmp;
	int              n;
};

/* Nodes are assorted in several circular, doubly linked lists. Conceptually, a
 * node points up to its parent, down to its child(ren), and left and right to
 * its siblings (if any). Nodes in the root list have no parent. */
struct fibheap_node {
	struct fibheap_node *parent;

	struct list_head    child;  // Connects the node to its children.
	struct list_head    list;   // Connects the node to its siblings.

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

/* --- API --- */

struct fibheap *make_fibheap(fibheap_cmp);

struct fibheap_node *make_fibheap_node(void *);

int fibheap_is_empty(struct fibheap *);

void fibheap_insert(struct fibheap *, struct fibheap_node *);

struct fibheap_node *fibheap_minimum(struct fibheap *);

struct fibheap_node *fibheap_extract_min(struct fibheap *);

struct fibheap *fibheap_union(struct fibheap *, struct fibheap *);

void fibheap_decrease(struct fibheap *, struct fibheap_node *);

void fibheap_delete(struct fibheap *, struct fibheap_node *);

#endif // FIBHEAP_H_
