/*
 * patmatch.h: Implementation of pattern matching algorithms.
 */

#ifndef PATMATCH_H_
#define PATMATCH_H_

#include <string.h>

#define ALPH_SZ     52    // A-Z, a-z.
#define LARGE_PRIME 10007 // So that d * q fits within a computer word.

/* Performs modular exponentiation: (base ^ exp) % mod */
static inline int mod_exp(int base, int exp, int mod)
{
	int ret = 1;

	for (int i = 0; i < exp; i++)
		ret = (ret * base) % mod;

	return ret;
}

/* Find the numbers of times pattern _pat_ is found in text _txt_ */
static inline unsigned int __rk_matcher(char *txt, const char *pat, int d, int q)
{
	int n = strlen(txt);          // length of the text.
	int m = strlen(pat);          // length of the pattern.
	int h = mod_exp(d, m - 1, q); // value of the higher order char.
	int p = 0;                    // hash of pat.
	int t = 0;                    // hash of the first m-char substring of txt.

	for (int i = 0; i < m; i++) {
		p = (d * p + pat[i] - 'A' + 1) % q;
		t = (d * t + txt[i] - 'A' + 1) % q;
	}
	unsigned int matches = 0;

	for (int s = 0; s <= (n - m); s++) {
		if (p == t) {
			int i, j;

			for (i = s, j = 0; pat[j] != '\0' && txt[i] == pat[j];
			     i++, j++);

			if (j > 0 && pat[j] == '\0')
				matches++;
		}
		if (s < (n - m))
			t = (d * (t - (txt[s] - 'A' + 1) * h) +
			     txt[s + m] - 'A' + 1) % q;
	}
	return matches;
}

static inline int rk_matcher(char *txt, const char *pat)
{
	int d = ALPH_SZ;
	int p = LARGE_PRIME;

	return __rk_matcher(txt, pat, d, p);
}

#endif // PATMATCH_H_

