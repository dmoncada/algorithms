#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "fibheap.h"

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

#define LINE "\n--------------------------------------------------------------------------------"

struct word {
	int        key;
	const char *str;
};

static int word_cmp(struct heap_node *_a, struct heap_node *_b)
{
	struct word *a, *b;

	a = (struct word *) _a->value;
	b = (struct word *) _b->value;

	return a->key < b->key;
}

struct word frames[] = {
	{15, LINE}, {300, "\n"}, {-4, "\n"}, {200, LINE}
};

struct word verbs[] = {
	{27, "ait"}, {1, "\n ... j'ai"}, {10, "l'étais"},
	{5, "été"}, {19, "sente"}, {15, "soit"},
	{16, "consommé,"}, {4, "j'avais"}, {22, "restait"},
	{24, "souhaiter"}, {2, "senti"}, {33, "m'accueillent\n"}
};

struct word words[] = {
	{12, "Pour"}, {8, "que"}, {18, "me"}, {9, "je"},
	{13, "que"}, {11, "encore."}, {14, "tout\n"},
	{29, "spectateurs"}, {30, "le jour de mon"},
	{17, "pour que je"}, {21, "il me"}, {23, "à"},
	{35, "cris de"}, {20, "moins seul,"}, {32, "et qu'ils"},
	{25, "qu'il\n"}, {26, "y"}, {28, "beaucoup de"},
	{34, "avec des"}, {36, "haine."}, {7, "et"},
	{31, "exécution"}, {3, "que"}, {6, "heureux,"}
};

struct word dummy_word = {-10, "dummy"};

static void insert_words(struct heap *h, struct word *words, int len)
{
	for (int i = 0; i < len; i++)
		fib_heap_insert(h, make_heap_node(words + i), word_cmp);
}

int main(int argc __attribute__ ((unused)),
	 const char **argv __attribute__ ((unused)))
{
	struct heap *h1, *h2;
	struct heap_node *n, *n1, *n2;
	struct word *w;

	h1 = make_fib_heap();
	h2 = make_fib_heap();

	insert_words(h1, verbs, LENGTH(verbs));
	insert_words(h2, words, LENGTH(words));

	fib_heap_union(h1, h2, word_cmp);

	n1 = make_heap_node(frames);
	n2 = make_heap_node(frames + 1);

	fib_heap_insert(h1, n1, word_cmp);
	fib_heap_insert(h1, n2, word_cmp);
	fib_heap_insert(h1, make_heap_node(frames + 2), word_cmp);
	fib_heap_insert(h1, make_heap_node(frames + 3), word_cmp);

	/* Dummy insertion and extraction, for triggering heap consolidation. */
	fib_heap_insert(h1, make_heap_node(&dummy_word), word_cmp);
	fib_heap_extract_min(h1, word_cmp);

	frames[0].key = -5;
	frames[1].key = 100;

	fib_heap_decrease(h1, n1, word_cmp); // n1->value points to frames[0].
	fib_heap_decrease(h1, n2, word_cmp); // n2->value points to frames[1].

	printf("For those who like Camus:\n");

	while (!fib_heap_is_empty(h1)) {
		n = fib_heap_extract_min(h1, word_cmp);
		w = n->value;
		printf("%s ", w->str);
		free(n);
	}
	printf("\n");

	/* Smile, it's good for you. */
	return 0;
}

