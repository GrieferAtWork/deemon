/* Copyright (c) 2018 Griefer@Work                                            *
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
#endif
#ifndef DCALL
#define DCALL   /* nothing */
#endif
#ifndef Treturn
#define Treturn int
#endif
#ifndef T
#define T       char
#endif



PRIVATE Treturn DCALL
wildcompare(T const *string, size_t string_length,
            T const *pattern, size_t pattern_length) {
 T card_post;
 T const *string_end = string+string_length;
 T const *pattern_end = pattern+pattern_length;
 for (;;) {
  if (string == string_end) {
   /* End of string (if the patter is empty, or only contains '*', we have a match) */
   for (;;) {
    if (pattern == pattern_end)
        return 0;
    if (*pattern != '*') break;
    ++pattern;
   }
   return -(Treturn)*pattern;
  }
  if (pattern == pattern_end)
      return (Treturn)*string; /* Pattern end doesn't match */
  if (*pattern == '*') {
   /* Skip starts */
   do {
    ++pattern;
    if (pattern == pattern_end)
        return 0; /* Pattern ends with '*' (matches everything) */
   } while (*pattern == '*');
   card_post = *pattern++;
   if (card_post == '?') goto next; /* Match any --> already found */
#ifdef NOCASE
   card_post = (T)DeeUni_ToLower(card_post);
#endif
   for (;;) {
    T ch = *string++;
#ifdef NOCASE
    if ((T)DeeUni_ToLower(ch) == card_post)
#else
    if (ch == card_post)
#endif
    {
     /* Recursively check if the rest of the string and pattern match */
     if (!wildcompare(string,(size_t)(string_end-string),
                      pattern,(size_t)(pattern_end-pattern)))
          return 0;
    } else if (!ch) {
     return -(Treturn)card_post; /* Wildcard suffix not found */
    }
   }
  }
  if (*pattern == *string ||
      *pattern == '?'
#ifdef NOCASE
      ||
      DeeUni_ToLower(*pattern) == DeeUni_ToLower(*string)
#endif
      )
  {
next:
   ++string;
   ++pattern;
   continue; /* single character match */
  }
  break; /* mismatch */
 }
 return (Treturn)*string - (Treturn)*pattern;
}

#undef NOCASE
#undef Treturn
#undef wildcompare
#undef T


