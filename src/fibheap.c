#include "fibheap.h"

#define REMOVE_FROM_LIST(_node)                                                 \
	list_del(&(_node)->list)

#define INSERT_INTO_LIST(_node, _list)                                          \
	list_add(&(_node)->list, &(_list))

#define INSERT_INTO_LIST_TAIL(_node, _list)                                     \
	list_add_tail(&(_node)->list, &(_list))

#define GET_MIN_NODE(_heap)                                                     \
	list_first_entry(&(_heap)->root_list, struct fibheap_node, list)

#define BECOME_MIN_NODE(_node, _heap)                                           \
	list_move(&(_node)->list, &(_heap)->root_list)

unsigned int last_bit = 1 << (sizeof(unsigned int) * CHAR_BIT - 1);

#define GET_DEGREE(_node) ((_node)->degree & ~last_bit)

#define SET_MARK(_node)   ((_node)->degree |= last_bit)

#define RESET_MARK(_node) ((_node)->degree &= ~last_bit)

#define TEST_MARK(_node)  ((_node)->degree & last_bit)

/* Upper bound on the degree of any node of an n-node Fibonacci heap. */
static inline int ubdeg(int n)
{
	double phi = (1 + sqrt(5)) / 2;

	return (int) (floor(log(n) / log(phi)));
}

static inline void link(struct fibheap_node *y, struct fibheap_node *x)
{
	/* Remove y from the root list of H. */
	REMOVE_FROM_LIST(y);

	/* Make y a child of x, incrementing x.degree. */
	INSERT_INTO_LIST(y, x->child);
	y->parent = x;
	x->degree++;

	RESET_MARK(y);
}

static inline void consolidate(struct fibheap *h)
{
	struct fibheap_node **A, *x, *y, *next, *swp, *min;
	int i, d, maxdeg = ubdeg(h->n);

	/* Let A[0..D(H.n)] be a new array. */
	A = malloc((maxdeg + 1) * sizeof(struct fibheap_node *));

	for (i = 0; i <= maxdeg; i++)
		A[i] = NULL;

	list_for_each_entry_safe(x, next, &h->root_list, list) {
		d = GET_DEGREE(x);

		while (A[d]) {
			y = A[d]; // Another node with the same degree as x.

			if (h->cmp(y->value, x->value) < 0) {
				/* Exchange x with y. */
				swp = x;
				x   = y;
				y   = swp;
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
				INSERT_INTO_LIST(A[i], h->root_list);
			} else {
				INSERT_INTO_LIST_TAIL(A[i], h->root_list);

				min = GET_MIN_NODE(h);

				if (h->cmp(A[i]->value, min->value) < 0)
					BECOME_MIN_NODE(A[i], h);
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
	REMOVE_FROM_LIST(x);
	y->degree--;

	/* Add x to the root list of h. */
	INSERT_INTO_LIST_TAIL(x, h->root_list);

	x->parent = NULL;
	RESET_MARK(x);
}

static inline void cascading_cut(struct fibheap *h, struct fibheap_node *y)
{
	struct fibheap_node *z = y->parent;

	if (z) {
		if (TEST_MARK(y)) {
			cut(h, y, z);
			cascading_cut(h, z);
		} else {
			SET_MARK(y);
		}
	}
}

/* --- API --- */

struct fibheap *make_fibheap(fibheap_cmp cmp)
{
	struct fibheap *heap = malloc(sizeof(struct fibheap));

	INIT_LIST_HEAD(&heap->root_list);

	heap->cmp = cmp;
	heap->n   = 0;

	return heap;
}

struct fibheap_node *make_fibheap_node(void *value)
{
	struct fibheap_node *node = malloc(sizeof(struct fibheap_node));

	node->parent = NULL;

	INIT_LIST_HEAD(&node->child);
	INIT_LIST_HEAD(&node->list);

	node->value  = value;
	node->degree = 0;

	return node;
}

int fibheap_is_empty(struct fibheap *h)
{
	return list_empty(&h->root_list);
}

void fibheap_insert(struct fibheap *h, struct fibheap_node *x)
{
	struct fibheap_node *min;

	if (fibheap_is_empty(h)) {
		INSERT_INTO_LIST(x, h->root_list);
	} else {
		INSERT_INTO_LIST_TAIL(x, h->root_list);

		min = GET_MIN_NODE(h);

		if (h->cmp(x->value, min->value) < 0)
			BECOME_MIN_NODE(x, h);
	}
	h->n++;
}

struct fibheap_node *fibheap_minimum(struct fibheap *h)
{
	return GET_MIN_NODE(h);
}

struct fibheap_node *fibheap_extract_min(struct fibheap *h)
{
	struct fibheap_node *x, *z, *next;

	if (fibheap_is_empty(h))
		return NULL;

	z = GET_MIN_NODE(h);

	if (!list_empty(&z->child)) {
		list_for_each_entry_safe(x, next, &z->child, list) {
			/* Add x to the root list of h. */
			INSERT_INTO_LIST_TAIL(x, h->root_list);
			x->parent = NULL;
		}
	}
	/* Remove z from the root list of h. */
	REMOVE_FROM_LIST(z);

	if (!fibheap_is_empty(h))
		consolidate(h);

	h->n--;

	return z;
}

struct fibheap *fibheap_union(struct fibheap *heap1, struct fibheap *heap2)
{
	struct fibheap_node *h1, *h2;

	if (!heap2 || (heap2 && fibheap_is_empty(heap2)))
		return heap1;

	if (!heap1 || (heap1 && fibheap_is_empty(heap1)))
		return heap2;

	h1 = GET_MIN_NODE(heap1);
	h2 = GET_MIN_NODE(heap2);

	list_splice_tail(&heap2->root_list, &heap1->root_list);

	if (heap1->cmp(h2->value, h1->value) < 0)
		BECOME_MIN_NODE(h2, heap1);

	heap1->n += heap2->n;

	/* Should heap2 be destroyed? */

	return heap1;
}

/* Does _not_ decrease the value of the node; clients are responsible for this,
 * as "value" is dependent on usage. */
void fibheap_decrease(struct fibheap *h, struct fibheap_node *x)
{
	struct fibheap_node *y = x->parent, *min;

	if (y && h->cmp(x->value, y->value) < 0) {
		cut(h, x, y);
		cascading_cut(h, y);
	}
	min = GET_MIN_NODE(h);

	if (h->cmp(x->value, min->value) < 0)
		BECOME_MIN_NODE(x, h);
}

void fibheap_delete(struct fibheap *h, struct fibheap_node *x)
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
	BECOME_MIN_NODE(x, h);

	/* Finally, the node is extracted from the heap. */
	fibheap_extract_min(h);
}
