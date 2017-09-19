#include <strmatch.h>

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

/* Finds the numbers of times pattern _pat_ is found in text _txt_. */
static inline int __strmatch_rk(char *txt, const char *pat, int d, int q)
{
	int n = strlen(txt);          // Length of the text.
	int m = strlen(pat);          // Length of the pattern.
	int h = mod_exp(d, m - 1, q); // Value of the higher order char.
	int p = 0;                    // Hash of pattern.
	int t = 0;                    // Hash of each m-char substring of the text.
	int i, s;

	for (i = 0; i < m; i++) {
		p = (d * p + pat[i]) % q;
		t = (d * t + txt[i]) % q;
	}
	int matches = 0;

	for (s = 0; s <= n - m; s++) {
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

/* --- API --- */

int strmatch_rk(char *txt, const char *pat)
{
	int d = ALPH_SZ;
	int p = LARGE_PRIME;

	return __strmatch_rk(txt, pat, d, p);
}
