#include <stdio.h>

#include "rbtree.h"
#include "fibheap.h"

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

struct word {
	int        key;
	const char *str;
};

struct word words1[] = {
	{10, "à"}, {34, "recommencer."}, {35, "Là-bas,"}, {62, " libérée"},
	{24, "elle"}, {31, "avait"}, {30, "elle"}, {79, " aussi,"},
	{18, "pourquoi"}, {14, " semblé\n"}, {72, "droit"}, {60, "s'y"},
	{27, " un"}, {56, "la"}, {55, "de"}, {43, "des"},
	{46, "le"}, {64, " prête"}, {11, "maman."}, {54, "près"},
	{7, "longtemps,"}, {75, "sur"}, {71, "le"}, {23, "vie"},
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
	{49, "comme"}, {3, "première"}, {26, "pris"}, {28, " << fiancé >>,\n"},
	{70, "n'avait"}, {33, "à"}, {69, "personne"}, {41, "asile"},
	{45, "s'eteignaient,"}, {9, "pensé"}, {47, "soir"}, {21, "fin"},
	{42, "où"}, {6, "bien"}, {2, "la"}, {36, " là-bas"},
	{73, "de"}, {25, "avait"}, {76, "elle."}, {37, " aussi,"},
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

/* Compare function for both Red-Black tree and Fibonacci heap nodes. */
static int word_cmp(void *_a, void *_b)
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
		printf("%s ", ((struct word *) x->value)->str);
		__rbtree_inorder_walk(t, x->right);
	}
}

static void rbtree_inorder_walk(struct rbtree *t)
{
	__rbtree_inorder_walk(t, t->root);
}

static void test_rbtree()
{
	struct rbtree *t;
	struct rbtree_node *n;

	t = make_rbtree(word_cmp);

	/* Insert some words. */
	rbtree_insert_words(t, words1, LENGTH(words1));
	rbtree_insert_words(t, words3, LENGTH(words3));

	n = make_rbtree_node(&dummy_word);

	/* Insert a word not belonging to the paragraph. */
	rbtree_insert(t, n);

	/* Now get rid of it! */
	rbtree_delete(t, n);

	/* Dump the contents of the tree. */
	rbtree_inorder_walk(t);

	/* Finally, dump the tree itself. */
	rbtree_destroy(t);
}

static void fibheap_insert_words(struct fibheap *h, struct word *words, int len)
{
	for (int i = 0; i < len; i++)
		fibheap_insert(h, make_fibheap_node(words + i));
}

void test_fibheap()
{
	struct fibheap *h1, *h2;
	struct fibheap_node *n;

	h1 = make_fibheap(word_cmp);
	h2 = make_fibheap(word_cmp);

	/* Insert some words. */
	fibheap_insert_words(h1, words2, LENGTH(words2));
	fibheap_insert_words(h2, words4, LENGTH(words4));

	n = make_fibheap_node(&dummy_word);

	/* Trigger heap consolidation by inserting a word, then deleting it. */
	fibheap_insert(h1, n);
	fibheap_delete(h1, n);

	/* Join the two heaps together. */
	fibheap_union(h1, h2);

	/* Empty the (now merged) heap and print its contents. */
	while (!fibheap_is_empty(h1)) {
		n = fibheap_extract_min(h1);
		printf("%s ", ((struct word *) n->value)->str);
		free(n);
	}
	free(h1);
	free(h2);
}

int main(int argc __attribute__ ((unused)),
	 const char **argv __attribute__ ((unused)))
{
	printf("For those who like Camus:\n\n");

	test_rbtree();
	test_fibheap();

	printf("\n");

	/* Smile, it's good for you. */
	return 0;
}

