#include <strmatch.h>

#define ALPH_SZ     2048  // Account for all one- and two-byte utf-8 chars.
#define LARGE_PRIME 497   // So that d * q fits within one computer word.

/* Performs modular exponentiation: (base ^ exp) % mod. */
static inline int mod_exp(int base, int exp, int mod)
{
	int ret = 1;

	if (mod == 1)
		return 0;

	base %= mod;

	for (; exp; exp >>= 1) {
		if (exp & 1)
			ret = (ret * base) % mod;

		base = (base * base) % mod;
	}

	return ret;
}

/* Counts the numbers of times _pat_ is found in _txt_. */
static inline int __strmatch_rk(char *txt, const char *pat, int d, int q)
{
	int n = strlen(txt);           // Length of the text.
	int m = strlen(pat);           // Length of the pattern.
	int h = mod_exp(d, m - 1, q);  // Value of the higher order char.
	int p = 0;                     // Hash of pattern.
	int t = 0;                     // Hash of each m-char substring of the text.
	int i, j, matches = 0;

	for (i = 0; i < m; i++) {
		p = (d * p + pat[i]) % q;
		t = (d * t + txt[i]) % q;
	}

	for (i = 0; i <= n - m; i++) {
		if (p == t) {
			for (j = 0; j < m; j++)
				if (txt[i + j] != pat[j])
					break;

			if (j == m)
				matches++;
		}

		if (i < n - m) {
			t = (d * (t - h * txt[i]) + txt[i + m]) % q;

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
