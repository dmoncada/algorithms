/*
 * hash.h: Implementation of hash tables with collision resolution by chaining.
 *         Items that hash to _key_ are placed in a linked list indexed at _key_
 *         in an array. A good hash function will minimize the occurrence of
 *         collisions, keeping queries fast.
 *
 *         Since this is intented to be a generic implementation, clients are
 *         left to implement their own custom hash and compare functions that
 *         work with the desired data type to store in the table.
 *
 * Summary of operations for hash tables:
 *
 *  - make_hash_table()         Allocs. a table.
 *  - hash_insert()             Inserts an entry in the list of its bucket.
 *  - hash_search()             Searches for an entry in the list of its bucket.
 *  - hash_delete()             Removes an entry from the table.
 */

#ifndef HASH_H_
#define HASH_H_

#include <stdlib.h>             // For malloc().

#include "list.h"               // For linked list struct. and ops.

/* For determining the bucket associated with items. */
typedef unsigned int(*hash_fn)(const void *);

/* For comparing items when performing searches. */
typedef int(*hash_cmp)(struct list_head *, const void *);

struct hash_table {
	struct list_head *table;

	hash_fn          fn;
	hash_cmp         cmp;
};

/* --- API --- */

struct hash_table *make_hash_table(int, hash_fn, hash_cmp);

void hash_insert(struct hash_table *, struct list_head *, const void *);

struct list_head *hash_search(struct hash_table *, const void *);

void hash_delete(struct list_head *);

#endif // HASH_H_
