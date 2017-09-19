#include "hash.h"

/* --- API --- */

struct hash_table *make_hash_table(int sz, hash_func func)
{
	struct hash_table *ht = malloc(sizeof(struct hash_table));

	ht->table = malloc(sz * sizeof(struct list_head));

	for (int i = 0; i < sz; i++)
		INIT_LIST_HEAD(&ht->table[i]);

	ht->func = func;

	return ht;
}
