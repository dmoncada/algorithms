/*
 * strmatch.h: Implementation of string-matching algorithms.
 *
 * Summary of string-matching algorithms:
 *
 *  - strmatch_rk()             Algorithm due to Rabin and Karp, see [1].
 *
 * Modular exponentiation is performed by means of an efficient method that runs
 * in the number of bits of the exponent (O(log exp)), which is useful when
 * matching long strings. The method is due to Brune Schneier, see [2] for
 * details.
 *
 * When matching utf-8 code points using the Rabin-Karp string matcher,
 * multi-byte characters are hashed as normal numerical values would, thus
 * providing support for non-ASCII character strings lookup. For a more
 * comprehensive description of the utf-8 encoding see [3].
 *
 * [1] "Introduction to Algorithms", 3rd ed, ch. 32: String Matching, by CLRS.
 * [2] https://en.wikipedia.org/wiki/Modular_exponentiation.
 * [3] https://en.wikipedia.org/wiki/UTF-8.
 */

#ifndef STRMATCH_H_
#define STRMATCH_H_

#include <string.h>

#define ALPH_SZ     2048 // Account for all one- and two-byte utf-8 chars.
#define LARGE_PRIME 497  // So that d * q fits within one computer word.

/* Performs modular exponentiation: (base ^ exp) % mod */
static inline int mod_exp(int base, int exp, int mod)
{
	if (mod == 1)
		return 0;

	int ret = 1;
	base %= mod;

	while (exp > 0) {
		if (exp % 2 == 1)
			ret = (ret * base) % mod;
		base = (base * base) % mod;
		exp >>= 1;
	}
	return ret;
}

/* Find the numbers of times pattern _pat_ is found in text _txt_ */
static inline int __strmatch_rk(char *txt, const char *pat, int d, int q)
{
	int n = strlen(txt);          // Length of the text.
	int m = strlen(pat);          // Length of the pattern.
	int h = mod_exp(d, m - 1, q); // Value of the higher order char.
	int p = 0;                    // Hash of pattern.
	int t = 0;                    // Hash of each m-char substring of the text.
	int i;

	for (int i = 0; i < m; i++) {
		p = (d * p + pat[i]) % q;
		t = (d * t + txt[i]) % q;
	}
	int matches = 0;

	for (int s = 0; s <= n - m; s++) {
		if (p == t) {
			for (i = 0; i < m; i++)
				if (txt[s + i] != pat[i])
					break;

			if (i == m)
				matches++;
		}
		if (s < n - m) {
			t = (d * (t - (txt[s]) * h) + txt[s + m]) % q;

			if (t < 0)
				t += q;
		}
	}
	return matches;
}

static inline int strmatch_rk(char *txt, const char *pat)
{
	int d = ALPH_SZ;
	int p = LARGE_PRIME;

	return __strmatch_rk(txt, pat, d, p);
}

#endif // STRMATCH_H_

