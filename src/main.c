#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "hash.h"
#include "rbtree.h"
#include "fibheap.h"
#include "strmatch.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))
#define PRIME  101
#define MASK   0xFF

void hash_insert_words(struct hash_table *, void *);

struct word {
	int        key;
	const char *str;
};

struct word words1[] = {
	{10, "à"}, {34, "recommencer."}, {35, "Là-bas,"}, {62, " libérée"},
	{24, " elle"}, {31, "avait"}, {30, "elle"}, {79, " aussi,"},
	{18, "pourquoi"}, {14, " semblé\n"}, {72, "droit"}, {60, "s'y"},
	{27, " un"}, {56, "la"}, {55, "de"}, {43, "des"},
	{46, "le"}, {64, " prête"}, {11, "maman."}, {54, "près"},
	{7, "longtemps,"}, {75, "sur"}, {71, "le"}, {23, " vie"},
	{32, "joué"}, {8, "j'ai"}, {81, " me\n"}, {57, "morte,"},
	{77, "Et"}, {68, "Personne,"}, {48, "était"}, {40, " cet\n"},
	{53, " Si\n"}, {20, "la"}, {51, "trêve"}, {38, " autour"}
};

struct word words2[] = {
	{34, " la"}, {94, "haine.\n"}, {11, "colère"}, {24, " signes"},
	{44, "moi,"}, {15, " du\n"}, {41, "si"}, {59, "Pour"},
	{29, " pour"}, {32, " fois"}, {76, "qu'il"}, {83, "jour"},
	{68, "sente"}, {71, "il"}, {56, "je"}, {81, " spectateurs\n"},
	{21, " nuit"}, {49, " senti"}, {1, "suis"}, {22, " chargée"},
	{3, "prêt"}, {74, "à"}, {9, "cette"}, {48, " j'ai"},
	{35, " tendre"}, {23, " de"}, {2, "senti"}, {27, " je\n"},
	{16, "mal,"}, {60, "que"}, {18, "d'espoir,"}, {42, "pareil"},
	{73, "restait"}, {47, " enfin,"}, {69, "moins"}, {93, "de"},
	{17, "vidé"}, {88, "qu'ils"}, {78, "ait"}, {14, " purgé"},
	{64, "pour"}, {43, "à"}, {75, "souhaiter"}, {91, "des"}
};

struct word words3[] = {
	{67, " revivre.\n"}, {63, " et"}, {13, " m'a"}, {29, "pourquoi"},
	{52, "mélancolique."}, {61, "sentir"}, {74, "pleurer"}, {58, "maman"},
	{4, "fois"}, {16, "je"}, {12, "Il"}, {39, " de"},
	{65, " à"}, {5, "depuis"}, {19, "à"}, {15, "que"},
	{49, "comme"}, {3, "première"}, {26, " pris"}, {28, " «fiancé»,\n"},
	{70, "n'avait"}, {33, "à"}, {69, "personne"}, {41, "asile"},
	{45, "s'eteignaient,"}, {9, "pensé"}, {47, "soir"}, {21, "fin"},
	{42, "où"}, {6, "bien"}, {2, "la"}, {36, " là-bas"},
	{73, "de"}, {25, " avait"}, {76, "elle."}, {37, " aussi,"},
	{80, " je"}, {50, "une"}, {17, "comprenais"}, {66, " tout"},
	{59, "devait"}, {1, " Pour"}, {78, " moi"}, {22, "d'une"}, {44, "vies"}
};

struct word words4[] = {
	{80, "de"}, {67, " me\n"}, {57, "l'étais"}, {89, "m'accueillent"},
	{61, "tout"}, {37, " du"}, {12, " m'avait"}, {84, "de"},
	{72, "me"}, {87, "et"}, {30, " la"}, {40, "l'éprouver"},
	{86, "exécution"}, {50, " que"}, {39, " De\n"}, {54, "et"},
	{45, "si"}, {58, "encore."}, {66, " je"}, {38, " monde."},
	{52, " été\n"}, {63, "consommé,"}, {82, "le"},
	{53, "heureux,"}, {55, "que"}, {65, "que"}, {19, "devant"},
	{62, "soit"}, {79, "beaucoup"}, {5, "tout"}, {25, " et"},
	{31, " première"}, {77, "y"}, {33, " à"}, {4, "à"},
	{85, "mon"}, {26, " d'étoiles,"}, {36, " indifférence"}, {20, " cette"},
	{90, "avec"}, {7, "Comme"}, {28, "m'ouvrais"}, {92, "cris"},
	{10, "grande"}, {46, "fraternel"}, {8, "si"}, {70, "seul,"},
	{51, " j'avais"}, {6, "revivre."}
};

struct word dummy_word = {40, "dummy"};

struct hash_table *dict;

char buf[1500]; // For joining individual words.

char tmp[50];   // For temporarily holding stripped words.

struct word_count {
	char             *key;
	int              value;

	struct list_head list;
};

/* Strips src from spaces and punctuation, and places the result in dst. */
static char *strip(const char *src, char *dst)
{
	char c, *d = dst; // d is a runner.

	while ((c = *src++))
		if (!isspace(c) && !ispunct(c))
			*d++ = tolower(c & MASK);

	*d = 0;

	return dst;
}

static struct word_count *make_word_count(const char *word)
{
	struct word_count *wc = malloc(sizeof(struct word_count));
	char *stripped = strip(word, tmp);

	wc->key = malloc((strlen(stripped) + 1) * sizeof(char));
	strcpy(wc->key, stripped);

	wc->value = 1;

	INIT_LIST_HEAD(&wc->list);

	return wc;
}

static int word_cmp(const void *_a, const void *_b)
{
	struct word *a, *b;

	a = (struct word *) _a;
	b = (struct word *) _b;

	return (a->key > b->key) - (a->key < b->key);
}

static void rbtree_insert_words(struct rbtree *t, struct word *words, int len)
{
	for (int i = 0; i < len; i++)
		rbtree_insert(t, make_rbtree_node(words + i));
}

static void __rbtree_inorder_walk(struct rbtree *t, struct rbtree_node *x)
{
	if (x != t->nil) {
		__rbtree_inorder_walk(t, x->left);

		const char *w = ((struct word *) x->value)->str;
		hash_insert_words(dict, make_word_count(w));
		sprintf(buf, "%s%s ", buf, w);

		__rbtree_inorder_walk(t, x->right);
	}
}

static void rbtree_inorder_walk(struct rbtree *t)
{
	__rbtree_inorder_walk(t, t->root);
}

/* Inserts randomly ordered words in a red-black tree. These are then dumped in
 * a temporary buffer (now in the right order) and inserted in a dictionary for
 * registering the number of times they appear. */
static void test_rbtree()
{
	struct rbtree *t = make_rbtree(word_cmp);
	struct rbtree_node *n = make_rbtree_node(&dummy_word);

	/* Insert some words. */
	rbtree_insert_words(t, words1, LEN(words1));
	rbtree_insert_words(t, words3, LEN(words3));

	/* Insert a word not belonging to the paragraph. */
	rbtree_insert(t, n);

	/* Now get rid of it! */
	rbtree_delete(t, n);

	/* Dump the contents of the tree in the buffer and dictionary. */
	rbtree_inorder_walk(t);

	/* Finally, dump the tree itself. */
	rbtree_destroy(t);
}

static void fibheap_insert_words(struct fibheap *h, struct word *words, int len)
{
	for (int i = 0; i < len; i++)
		fibheap_insert(h, make_fibheap_node(words + i));
}

/* Inserts a different set of words (also in a random order) in different
 * fibonacci heaps. Heaps are then merged and their contents emptied in the same
 * buffer and dictionary. */
static void test_fibheap()
{
	struct fibheap *h1, *h2;
	struct fibheap_node *n = make_fibheap_node(&dummy_word);
	const char *w;

	h1 = make_fibheap(word_cmp);
	h2 = make_fibheap(word_cmp);

	/* Insert some words. */
	fibheap_insert_words(h1, words2, LEN(words2));
	fibheap_insert_words(h2, words4, LEN(words4));

	/* Trigger heap consolidation by inserting a word, then deleting it. */
	fibheap_insert(h1, n);
	fibheap_delete(h1, n);

	/* Merge the two heaps together. */
	fibheap_union(h1, h2);

	/* Empty the (now merged) heap and add its contents to the buffer. */
	while (!fibheap_is_empty(h1)) {
		n = fibheap_extract_min(h1);
		w = ((struct word *) n->value)->str;
		hash_insert_words(dict, make_word_count(w));
		sprintf(buf, "%s%s ", buf, w);
		free(n);
	}

	free(h1);
	free(h2);
}

/* Prints the number of times a few patterns are found in the buffer. */
static void test_patmatch()
{
	const char *pat, *pats[] = { "que", "première", "coiffeur" };
	int i, len = LEN(pats), occur;

	for (i = 0; i < len; i++) {
		pat = pats[i];

		/* Match the pattern using the Rabin-Karp algorithm. */
		occur = strmatch_rk(buf, pat);

		printf("The pattern \"%s\" ", pat);

		if (occur)
			printf("occurs %i time(s) ", occur);
		else
			printf("does not occur ");

		printf("in the paragraph.\n");
	}

	printf("\n");
}

static unsigned int word_count_hash_fn(const void *word)
{
	unsigned int c, ret = 1;
	char *w = (char *) word;

	while ((c = *w++))
		ret = (ret * c) % PRIME;

	return ret;
}

void hash_insert_words(struct hash_table *ht, void *entry)
{
	struct word_count *wc, *_wc = (struct word_count *) entry;
	char *stripped = strip(_wc->key, tmp);
	int idx = ht->fn(stripped);

	/* If the word is already in the dictionary, increase its count. */
	list_for_each_entry(wc, &ht->table[idx], list) {
		if (!strcmp(stripped, wc->key)) {
			wc->value++;
			free(_wc->key);
			free(_wc);
			return;
		}
	}

	hash_insert(ht, &_wc->list, stripped);
}

static int word_count_fibheap_cmp(const void *_a, const void *_b)
{
	struct word_count *a, *b;

	a = (struct word_count *) _a;
	b = (struct word_count *) _b;

	return (b->value > a->value) - (b->value < a->value);
}

static int word_count_hash_cmp(struct list_head *left, const void *right)
{
	return word_count_fibheap_cmp(
		container_of(left, struct word_count, list), right);
}

/* Finds the most repeated words in the buffer. */
static void test_hash()
{
	struct word_count *wc, *next;
	struct fibheap *h = make_fibheap(word_count_fibheap_cmp);
	struct fibheap_node *hn;
	int i, n = 10, sz = PRIME;

	printf("Here are the %i most repeated words in the paragraph:\n\n", n);

	/* Insert the counts in a heap; higher counts mean higher priority. */
	for (i = 0; i < sz; i++) {
		list_for_each_entry_safe(wc, next, &dict->table[i], list) {
			fibheap_insert(h, make_fibheap_node(wc));
			list_del(&wc->list);
		}
	}

	i = 0;

	/* Dump the top n elements of the heap. */
	while (!fibheap_is_empty(h)) {
		hn = fibheap_extract_min(h);
		wc = (struct word_count *) hn->value;

		if (i++ < n)
			printf(" Word: \"%s\", frequency: %i\n",
			       wc->key, wc->value);

		free(wc->key);
		free(wc);
		free(hn);
	}

	free(h);
}

int main(int argc __attribute__ ((unused)),
	 const char **argv __attribute__ ((unused)))
{
	/* A small test case for the implemented algorithms and data structures.
	 * A set of words are sorted and printed such that together make sense. */

	int sz = PRIME;

	dict = make_hash_table(sz, word_count_hash_fn, word_count_hash_cmp);

	printf("For those who like Camus:\n\n");

	test_rbtree();
	test_fibheap();

	printf("%s\n", buf);

	test_patmatch();
	test_hash();

	printf("\n");

	/* Smile, it's good for you. */
	return 0;
}
