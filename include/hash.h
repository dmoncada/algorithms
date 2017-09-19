/*
 * hash.h: Implementation of hash tables. The header acts as a contract with
 *         which client programs must comply. Operations are left for the user
 *         to implement.
 *
 * Summary of operations for hash tables:
 *
 *  - make_hash_table()         Allocs. a table.
 *  - hash_insert()             Interface for the insert operation.
 */

#ifndef HASH_H_
#define HASH_H_

#include <stdlib.h>

#include "list.h"

typedef unsigned int(*hash_func)(void *a);

struct hash_table {

	struct list_head *table;

	hash_func        func;
};

/* --- API --- */

struct hash_table *make_hash_table(int, hash_func);
void hash_insert(struct hash_table *, void *);

#endif // HASH_H_
