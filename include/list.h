/*
 * list.h: The Linux kernel's implementation of circular, doubly-linked lists.
 *
 *         The key difference between linked lists as one would normally
 *         implement them and Linux's linked lists is that the latter embed list
 *         nodes in the structs. to link. List ops. revolve around this fact.
 *         The struct. itself is fetched by means of the container_of() macro.
 *
 *         Check [1] for more info. on the Linux's linked lists.
 *
 * Summary of operations for doubly-linked lists:
 *
 *  - list_add()                Inserts a node into a list at the head.
 *  - list_add_tail()           Inserts a node into a list at the tail.
 *  - list_del()                Removes a node from a list.
 *  - list_move()               Moves a node to the head of a list.
 *  - list_empty()              Asserts if the list is empty.
 *  - list_splice()             Attaches one list to another at the head.
 *  - list_splice_tail()        Attaches one list to another at the tail.
 *
 *  [1] "Linux Kernel Development", ch. 6: Kernel Data Structures, under "Linked
 *  Lists", by Robert Love.
 */

#ifndef LIST_H_
#define LIST_H_

#include <stddef.h>

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name)                                                         \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr)                                                     \
                                                                                \
	do {                                                                    \
		(ptr)->next = (ptr);                                            \
		(ptr)->prev = (ptr);                                            \
	} while (0)

#define container_of(ptr, type, member) __extension__ ({                        \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);                    \
	(type *)( (char *)__mptr - offsetof(type,member) );})

#ifndef typeof
#define typeof __typeof__
#endif

#define list_entry(ptr, type, member)                                           \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member)                                     \
	list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member)                                            \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each(pos, head)                                                \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head)                                        \
	for (pos = (head)->next, n = pos->next; pos != (head);                  \
	     pos = n; n = pos->next)

#define list_for_each_entry(pos, head, member)                                  \
	for (pos = list_first_entry(head, typeof(*pos), member);                \
	     &pos->member != (head);                                            \
	     pos = list_next_entry(pos, member))

#define list_for_each_entry_safe(pos, n, head, member)                          \
	for (pos = list_first_entry(head, typeof(*pos), member),                \
	     n = list_next_entry(pos, member);                                  \
	     &pos->member != (head);                                            \
	     pos = n, n = list_next_entry(n, member))

/* --- API --- */

void list_add(struct list_head *, struct list_head *);

void list_add_tail(struct list_head *, struct list_head *);

void list_del(struct list_head *);

void list_move(struct list_head *, struct list_head *);

int list_empty(struct list_head *);

void list_splice(struct list_head *, struct list_head *);

void list_splice_tail(struct list_head *, struct list_head *);

#endif // LIST_H_
