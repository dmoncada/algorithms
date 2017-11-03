#include "hash.h"

/* --- API --- */

struct hash_table *make_hash_table(int sz, hash_fn fn, hash_cmp cmp)
{
	if (!fn || !cmp)
		return NULL;

	struct hash_table *ht = malloc(sizeof(struct hash_table));

	ht->table = malloc(sz * sizeof(struct list_head));

	for (int i = 0; i < sz; i++)
		INIT_LIST_HEAD(&ht->table[i]);

	ht->fn  = fn;
	ht->cmp = cmp;

	return ht;
}

void hash_insert(struct hash_table *ht, struct list_head *new, const void *key)
{
	list_add(new, &ht->table[ht->fn(key)]);
}

struct list_head *hash_search(struct hash_table *ht, const void *key)
{
	int idx = ht->fn(key);
	struct list_head *runner;

	list_for_each(runner, &ht->table[idx]) {
		if (ht->cmp(runner, key))
			return runner;
	}
	return NULL;
}

void hash_delete(struct list_head *entry)
{
	list_del(entry);
}
