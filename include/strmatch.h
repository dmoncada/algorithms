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

/* --- API --- */

int strmatch_rk(char *, const char *);

#endif // STRMATCH_H_
