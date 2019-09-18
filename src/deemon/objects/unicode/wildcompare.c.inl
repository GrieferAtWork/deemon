/* Copyright (c) 2019 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */

#ifndef PRIVATE
#define PRIVATE static
#endif /* !PRIVATE */
#ifndef DCALL
#define DCALL   /* nothing */
#endif /* !DCALL */
#ifndef Treturn
#define Treturn int
#endif /* !Treturn */
#ifndef T
#define T       char
#endif /* !T */

#ifdef CASEFOLD
PRIVATE int DCALL
wildcompare(unicode_foldreader(T) *string,
            unicode_foldreader(T) *pattern) {
	uint32_t pattern_ch, ch;
	for (;;) {
		if (unicode_foldreader_empty(*string)) {
			/* End of string (if the patter is empty, or only contains '*', we have a match) */
			for (;;) {
				if (unicode_foldreader_empty(*pattern))
					return 0;
				ch = unicode_foldreader_getc(*pattern);
				if (ch != '*')
					break;
			}
			return -1;
		}
		if (unicode_foldreader_empty(*pattern))
			return 1; /* Pattern end doesn't match */
		pattern_ch = unicode_foldreader_getc(*pattern);
		if (pattern_ch == '*') {
			/* Skip starts */
			do {
				if (unicode_foldreader_empty(*pattern))
					return 0; /* Pattern ends with '*' (matches everything) */
				pattern_ch = unicode_foldreader_getc(*pattern);
			} while (pattern_ch == '*');
			if (pattern_ch == '?')
				continue; /* Match any --> already found */
			for (;;) {
				ch = unicode_foldreader_getc(*string);
				if (ch == pattern_ch) {
					/* Recursively check if the rest of the string and pattern match */
					unicode_foldreader(T) string_copy  = *string;
					unicode_foldreader(T) pattern_copy = *pattern;
					if (!wildcompare(&string_copy, &pattern_copy))
						return 0;
				} else if (unicode_foldreader_empty(*string)) {
					return -1; /* Wildcard suffix not found */
				}
			}
		}
		ch = unicode_foldreader_getc(*string);
		if (pattern_ch == ch ||
		    pattern_ch == '?')
			continue; /* single character match */
		break;        /* mismatch */
	}
	if (ch < pattern_ch)
		return -1;
	return 1;
}

#else /* CASEFOLD */

PRIVATE Treturn DCALL
wildcompare(T const *string, size_t string_length,
            T const *pattern, size_t pattern_length) {
	T card_post;
	T const *string_end  = string + string_length;
	T const *pattern_end = pattern + pattern_length;
	for (;;) {
		if (string >= string_end) {
			/* End of string (if the patter is empty, or only contains '*', we have a match) */
			for (;;) {
				if (pattern >= pattern_end)
					return 0;
				if (*pattern != '*')
					break;
				++pattern;
			}
			return -(Treturn)*pattern;
		}
		if (pattern >= pattern_end)
			return (Treturn)*string; /* Pattern end doesn't match */
		if (*pattern == '*') {
			/* Skip starts */
			do {
				++pattern;
				if (pattern >= pattern_end)
					return 0; /* Pattern ends with '*' (matches everything) */
			} while (*pattern == '*');
			card_post = *pattern++;
			if (card_post == '?')
				goto next; /* Match any --> already found */
#ifdef NOCASE
			card_post = (T)DeeUni_ToLower(card_post);
#endif /* NOCASE */
			for (;;) {
				T ch = *string++;
#ifdef NOCASE
				if ((T)DeeUni_ToLower(ch) == card_post)
#else /* NOCASE */
				if (ch == card_post)
#endif /* !NOCASE */
				{
					/* Recursively check if the rest of the string and pattern match */
					if (!wildcompare(string, (size_t)(string_end - string),
					                 pattern, (size_t)(pattern_end - pattern)))
						return 0;
				} else if (string >= string_end) {
					return -(Treturn)card_post; /* Wildcard suffix not found */
				}
			}
		}
		if (*pattern == *string ||
		    *pattern == '?'
#ifdef NOCASE
		    ||
		    DeeUni_ToLower(*pattern) == DeeUni_ToLower(*string)
#endif /* !NOCASE */
		) {
		next:
			++string;
			++pattern;
			continue; /* single character match */
		}
		break; /* mismatch */
	}
	return (Treturn)*string - (Treturn)*pattern;
}
#endif /* !CASEFOLD */


#undef CASEFOLD
#undef NOCASE
#undef Treturn
#undef wildcompare
#undef T


