/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_FORMAT_H
#define GUARD_DEEMON_FORMAT_H 1

#include "api.h"

#include <hybrid/__va_size.h> /* __VA_SIZE */
#include <hybrid/typecore.h>

#include <stdarg.h>
#include <stddef.h>

#include "object.h"

DECL_BEGIN

/* General-purpose printing of formatted data to the given `printer'.
 * These functions implement c's `printf()' standard (except for the
 * wide-string part) with the following extensions:
 *   - `%I...': Length prefix: sizeof(size_t)
 *   - `%I8...', `%I16...', `%I32...', `%I64...': Absolute length prefix.
 *   - `%q': Print quoted (escaped) string from a UTF-8 source. - Prefix with `%#q' to omit surrounding quotes.
 *   - `%I8q': Print quoted (escaped) string from a 1-byte-per-character source. - Prefix with `%#q' to omit surrounding quotes.
 *   - `%I16q': Print quoted (escaped) string from a 2-byte-per-character source. - Prefix with `%#q' to omit surrounding quotes.
 *   - `%I32q': Print quoted (escaped) string from a 4-byte-per-character source. - Prefix with `%#q' to omit surrounding quotes.
 *   - `%s': Print a UTF-8 string.
 *   - `%I8s': Print a 1-byte-per-character string.
 *   - `%I16s': Print a 2-byte-per-character string.
 *   - `%I32s': Print a 4-byte-per-character string.
 *   - `%I8c': Print a character from the range U+0000 - U+00FF (Same as the regular `%c')
 *   - `%I16c': Print a character from the range U+0000 - U+FFFF
 *   - `%I32c': Print a character from the range U+00000000 - U+FFFFFFFF
 *   - `%$s', `%$q': Take the absolute length of the string as a `size_t' (may also be combined with I* prefixes)
 *   - `%C': Print a character from the range U+0000 - U+00FF in its c-escaped form (using '\'' instead of '\"')
 *   - `%I8C': Print a character from the range U+0000 - U+00FF in its c-escaped form (using '\'' instead of '\"') (Same as the regular `%C')
 *   - `%I16C': Print a character from the range U+0000 - U+FFFF in its c-escaped form (using '\'' instead of '\"')
 *   - `%I32C': Print a character from the range U+00000000 - U+FFFFFFFF in its c-escaped form (using '\'' instead of '\"')
 *   - `%#C': Print a character from the range U+0000 - U+00FF in its c-escaped form (without surrounding '\''-characters)
 *   - `%#I8C': Print a character from the range U+0000 - U+00FF in its c-escaped form (without surrounding '\''-characters) (Same as the regular `%C')
 *   - `%#I16C': Print a character from the range U+0000 - U+FFFF in its c-escaped form (without surrounding '\''-characters)
 *   - `%#I32C': Print a character from the range U+00000000 - U+FFFFFFFF in its c-escaped form (without surrounding '\''-characters)
 *   - `%.?s', `%.?q': Take the max length of the string or precision as a `size_t' (may also be combined with I* prefixes)
 *   - `%:?s', `%:?q': Same as `%$s' / `%$q'
 *   - `%:DIGIT: Similar to `%.DIGIT', but set the absolute length of the string, rather than its maximum length
 *               This is to `%.DIGIT' what `%$s' is to `%.?s'
 *   - `%?...': Take the width of the output text as a `size_t'
 *   - `%b': Integer option: Same as `o', but output as a binary.
 *   - `%k': Taking a `DeeObject *', print `__str__' (return `-1', but don't set an (additional) error if NULL)
 *   - `%r': Taking a `DeeObject *', print `__repr__' (return `-1', but don't set an (additional) error if NULL)
 *   - `%K': Same as `%k', but decref() the object afterwards.
 *   - `%R': Same as `%r', but decref() the object afterwards.
 * HINT: To guaranty fulfillment of `K' and `R' operands,
 *       the format string is _always_ fully processed.
 * @return: * :  The sum of all return values from calls to `*printer'
 * @return: < 0: The first negative return value of a call to `*printer' */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t
DeeFormat_Printf(Dee_formatprinter_t printer, void *arg,
                 char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_VPrintf(Dee_formatprinter_t printer, void *arg,
                  char const *__restrict format, va_list args);

/* Optimized format sequences for:
 * - DeeFormat_Printf()
 * - DeeString_Newf()
 * - ... */
#define _DEE_PRF8 "" /* Due to integer promotions, we can assume that any 8-bit integral alwas
                      * gets promoted to an integer, because we can assume that `sizeof(int) >= 1' */
#if __VA_SIZE >= 2
#define _DEE_PRF16 ""
#elif __SIZEOF_SHORT__ == 2
#define _DEE_PRF16 "h"
#elif __SIZEOF_LONG__ == 2
#define _DEE_PRF16 "l"
#elif __SIZEOF_POINTER__ == 2
#define _DEE_PRF16 "I"
#elif __SIZEOF_CHAR__ == 2
#define _DEE_PRF16 "hh"
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 2
#define _DEE_PRF16 "ll"
#else /* ... == 2 */
#define _DEE_PRF16 "I16"
#endif /* ... != 2 */
#if __VA_SIZE >= 4
#define _DEE_PRF32 ""
#elif __SIZEOF_LONG__ == 4
#define _DEE_PRF32 "l"
#elif __SIZEOF_SHORT__ == 4
#define _DEE_PRF32 "h"
#elif __SIZEOF_POINTER__ == 4
#define _DEE_PRF32 "I"
#elif __SIZEOF_CHAR__ == 4
#define _DEE_PRF32 "hh"
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 4
#define _DEE_PRF32 "ll"
#else /* ... == 4 */
#define _DEE_PRF32 "I32"
#endif /* ... != 4 */
#if __VA_SIZE >= 8
#define _DEE_PRF64 ""
#elif __SIZEOF_LONG__ == 8
#define _DEE_PRF64 "l"
#elif __SIZEOF_POINTER__ == 8
#define _DEE_PRF64 "I"
#elif __SIZEOF_SHORT__ == 8
#define _DEE_PRF64 "h"
#elif __SIZEOF_CHAR__ == 8
#define _DEE_PRF64 "hh"
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 8
#define _DEE_PRF64 "ll"
#else /* ... == 8 */
#define _DEE_PRF64 "I64"
#endif /* ... != 8 */
#define _DEE_PRF128 "I128"


#define DEE_PRFu8   _DEE_PRF8 "u"
#define DEE_PRFd8   _DEE_PRF8 "d"
#define DEE_PRFx8   _DEE_PRF8 "x"
#define DEE_PRFX8   _DEE_PRF8 "X"
#define DEE_PRFu16  _DEE_PRF16 "u"
#define DEE_PRFd16  _DEE_PRF16 "d"
#define DEE_PRFx16  _DEE_PRF16 "x"
#define DEE_PRFX16  _DEE_PRF16 "X"
#define DEE_PRFu32  _DEE_PRF32 "u"
#define DEE_PRFd32  _DEE_PRF32 "d"
#define DEE_PRFx32  _DEE_PRF32 "x"
#define DEE_PRFX32  _DEE_PRF32 "X"
#define DEE_PRFu64  _DEE_PRF64 "u"
#define DEE_PRFd64  _DEE_PRF64 "d"
#define DEE_PRFx64  _DEE_PRF64 "x"
#define DEE_PRFX64  _DEE_PRF64 "X"
#define DEE_PRFu128 _DEE_PRF128 "u"
#define DEE_PRFd128 _DEE_PRF128 "d"
#define DEE_PRFx128 _DEE_PRF128 "x"
#define DEE_PRFX128 _DEE_PRF128 "X"

#define DEE_PRIVATE_PRFu1         DEE_PRFu8
#define DEE_PRIVATE_PRFd1         DEE_PRFd8
#define DEE_PRIVATE_PRFx1         DEE_PRFx8
#define DEE_PRIVATE_PRFX1         DEE_PRFX8
#define DEE_PRIVATE_PRFu2         DEE_PRFu16
#define DEE_PRIVATE_PRFd2         DEE_PRFd16
#define DEE_PRIVATE_PRFx2         DEE_PRFx16
#define DEE_PRIVATE_PRFX2         DEE_PRFX16
#define DEE_PRIVATE_PRFu4         DEE_PRFu32
#define DEE_PRIVATE_PRFd4         DEE_PRFd32
#define DEE_PRIVATE_PRFx4         DEE_PRFx32
#define DEE_PRIVATE_PRFX4         DEE_PRFX32
#define DEE_PRIVATE_PRFu8         DEE_PRFu64
#define DEE_PRIVATE_PRFd8         DEE_PRFd64
#define DEE_PRIVATE_PRFx8         DEE_PRFx64
#define DEE_PRIVATE_PRFX8         DEE_PRFX64
#define DEE_PRIVATE_PRFu16        DEE_PRFu128
#define DEE_PRIVATE_PRFd16        DEE_PRFd128
#define DEE_PRIVATE_PRFx16        DEE_PRFx128
#define DEE_PRIVATE_PRFX16        DEE_PRFX128
#define DEE_PRIVATE_PRFuN(sizeof) DEE_PRIVATE_PRFu##sizeof
#define DEE_PRIVATE_PRFdN(sizeof) DEE_PRIVATE_PRFd##sizeof
#define DEE_PRIVATE_PRFxN(sizeof) DEE_PRIVATE_PRFx##sizeof
#define DEE_PRIVATE_PRFXN(sizeof) DEE_PRIVATE_PRFX##sizeof
#define DEE_PRFuN(sizeof)         DEE_PRIVATE_PRFuN(sizeof)
#define DEE_PRFdN(sizeof)         DEE_PRIVATE_PRFdN(sizeof)
#define DEE_PRFxN(sizeof)         DEE_PRIVATE_PRFxN(sizeof)
#define DEE_PRFXN(sizeof)         DEE_PRIVATE_PRFXN(sizeof)

/* Helpful aliases */
#define DEE_PRFuSIZ DEE_PRFuN(__SIZEOF_SIZE_T__)
#define DEE_PRFdSIZ DEE_PRFdN(__SIZEOF_SIZE_T__)
#define DEE_PRFxSIZ DEE_PRFxN(__SIZEOF_SIZE_T__)
#define DEE_PRFXSIZ DEE_PRFXN(__SIZEOF_SIZE_T__)
#define DEE_PRFuPTR DEE_PRFuN(__SIZEOF_POINTER__)
#define DEE_PRFdPTR DEE_PRFdN(__SIZEOF_POINTER__)
#define DEE_PRFxPTR DEE_PRFxN(__SIZEOF_POINTER__)
#define DEE_PRFXPTR DEE_PRFXN(__SIZEOF_POINTER__)

/* Unescaped names. */
#ifdef DEE_SOURCE
#define PRFu8   DEE_PRFu8
#define PRFd8   DEE_PRFd8
#define PRFx8   DEE_PRFx8
#define PRFX8   DEE_PRFX8
#define PRFu16  DEE_PRFu16
#define PRFd16  DEE_PRFd16
#define PRFx16  DEE_PRFx16
#define PRFX16  DEE_PRFX16
#define PRFu32  DEE_PRFu32
#define PRFd32  DEE_PRFd32
#define PRFx32  DEE_PRFx32
#define PRFX32  DEE_PRFX32
#define PRFu64  DEE_PRFu64
#define PRFd64  DEE_PRFd64
#define PRFx64  DEE_PRFx64
#define PRFX64  DEE_PRFX64
#define PRFu128 DEE_PRFu128
#define PRFd128 DEE_PRFd128
#define PRFx128 DEE_PRFx128
#define PRFX128 DEE_PRFX128
#define PRFuN   DEE_PRFuN
#define PRFdN   DEE_PRFdN
#define PRFxN   DEE_PRFxN
#define PRFXN   DEE_PRFXN
#define PRFuSIZ DEE_PRFuSIZ
#define PRFdSIZ DEE_PRFdSIZ
#define PRFxSIZ DEE_PRFxSIZ
#define PRFXSIZ DEE_PRFXSIZ
#define PRFuPTR DEE_PRFuPTR
#define PRFdPTR DEE_PRFdPTR
#define PRFxPTR DEE_PRFxPTR
#define PRFXPTR DEE_PRFXPTR
#endif /* DEE_SOURCE */





#define DeeFormat_Print(printer, arg, str, len)     (*printer)(arg, str, len)
#define DeeFormat_PrintStr(printer, arg, str)       (*printer)(arg, str, strlen(str))
#define DeeFormat_PRINT(printer, arg, str)          (*printer)(arg, str, COMPILER_STRLEN(str))
#define DeeFormat_PrintObject(printer, arg, ob)     DeeObject_Print(ob, printer, arg)
#define DeeFormat_PrintObjectRepr(printer, arg, ob) DeeObject_PrintRepr(ob, printer, arg)

/* Quote (backslash-escape) the given text, printing the resulting text to `printer'.
 * NOTE: This function always generates pure ASCII, and is therefor safe to be used
 *       when targeting an `ascii_printer'
 * Output:
 * - ASCII+isprint --> keep
 * - ASCII+iscntrl --> \r, \n, \b, ...
 * - ASCII         --> \177
 * - other         --> \uABCD, \U1234ABCD */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Quote(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                /*utf-8*/ char const *__restrict text, size_t textlen);

/* Format-quote raw bytes. Output:
 * - ASCII+isprint --> keep
 * - ASCII+iscntrl --> \r, \n, \b, ...
 * - ASCII+[0-7]   --> \0, \1, ... \7
 * - other         --> \xAB */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_QuoteBytes(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                     /*bytes*/ void const *__restrict data, size_t num_bytes);


/* Format-quote 8-bit, 16-bit, and 32-bit unicode text: */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Quote8(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                 /*latin-1*/ uint8_t const *__restrict text, size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Quote16(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                  /*utf-16-without-surrogates*/ uint16_t const *__restrict text, size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Quote32(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                  /*utf-32*/ uint32_t const *__restrict text, size_t textlen);

/* Repeat the given `ch' a total of `count' times. */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeFormat_Repeat(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                 /*ascii*/ char ch, size_t count);

/* Repeat `str...+=length' such that a total of `total_characters'
 * characters (not bytes, but characters) are printed. */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_RepeatUtf8(/*utf-8*/ Dee_formatprinter_t printer, void *arg,
                     /*utf-8*/ char const *__restrict str,
                     size_t length, size_t total_characters);

/* Print a unicode character `ch', encoded as UTF-8 into `printer' */
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeFormat_Putc(/*utf-8*/ Dee_formatprinter_t printer, void *arg, uint32_t ch);

/* Convert an 8, 16, or 32-bit character array to UTF-8 and write it to `printer'
 * NOTE: 8-bit here refers to the unicode range U+0000 - U+00FF */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Print8(/*utf-8*/ Dee_formatprinter_t printer, void *arg,
                 /*latin-1*/ uint8_t const *__restrict text,
                 size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Print16(/*utf-8*/ Dee_formatprinter_t printer, void *arg,
                  /*utf-16-without-surrogates*/ uint16_t const *__restrict text,
                  size_t textlen);
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Print32(/*utf-8*/ Dee_formatprinter_t printer, void *arg,
                  /*utf-32*/ uint32_t const *__restrict text,
                  size_t textlen);


struct va_list_struct { va_list vl_ap; };

/* Pack a new value, given a special format string `string'.
 * Format language syntax:
 *     __main__ ::= object;
 *     object ::= ('n' | '-')         // `none'
 *              | ref_object          // `Object' <-- `va_arg(DeeObject *)'
 *              | ref_int             // `int'    <-- `va_arg(...)'
 *              | ref_float           // `float'  <-- `va_arg(...)'
 *              | ref_bool            // `bool'   <-- `va_arg(...)'
 *              | ref_str             // `string' <-- `va_arg(...)'
 *              | '[' [objects] ']'   // `List'
 *              | '(' [objects] ')'   // `Tuple'
 *              | '{' [objects] '}'   // `Set'
 *              | '<' [object] '>'    // `Cell' (When `object')
 *     ;
 *     objects ::= (object | ',')...  // `,' is simply ignored, but can be used to prevent ambiguity
 *     
 *     ref_object ::= 'o' | 'O'; // `DeeObject *' (Uppercase `O' inherits a reference from `va_arg' and causes `Dee_Packf' to propagate an error when `NULL')
 *     ref_int    ::= ref_intlen ('d' | 'u' | 'i' | 'x'); // `u' and `x' create unsigned integers
 *     ref_intlen ::= 'I' ['8' | '16' | '32' | '64'] // Fixed-length / sizeof(size_t)
 *                  | 'hh' // char
 *                  | 'h'  // short
 *                  | ''   // int (Default when nothing else was given)
 *                  | 'l'  // long
 *                  | 'll' // long long (__(U)LONGLONG)
 *     ;
 *     ref_float  ::= 'f'  // float / double
 *                  | 'LD' // long double
 *     ;
 *     ref_bool   ::= ref_intlen 'b';
 *     ref_str    ::= [ '.<int>'  // str = va_arg(char *); DeeString_NewSized(str, MIN(strlen(str), <int>));
 *                    | '.*'      // max = va_arg(unsigned int); str = va_arg(char *); DeeString_NewSized(str, MIN(strlen(str), max));
 *                    | '.?'      // max = va_arg(size_t); str = va_arg(char *); DeeString_NewSized(str, MIN(strlen(str), max));
 *                    | '$'       // len = va_arg(size_t); str = va_arg(char *); DeeString_NewSized(str, len);
 *                    ] 's';      // DeeString_New(va_arg(char *));
 *
 * Example:
 * >> Dee_Packf("(d<d>Os)", 10, 20, DeeInt_NewInt(42), "foobar");
 * >> // (10, <20>, 42, "foobar")
 */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *
Dee_Packf(char const *__restrict format, ...);

DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_VPackf(char const *__restrict format, va_list args);

DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_VPPackf(char const **__restrict p_format,
            struct va_list_struct *__restrict p_args);

DFUNDEF NONNULL((1)) void DCALL
Dee_VPPackf_Cleanup(char const *__restrict format, va_list args);


/* Optimized format sequences for:
 * - Dee_Packf()
 * - DeeTuple_Newf()
 * - DeeObject_CallAttrStringf()
 * - ... */
#define DEE_PCKu8 "u" /* Due to integer promotions, we can assume that any 8-bit integral always
                       * gets promoted to an integer, because we can assume that `sizeof(int) >= 1' */
#define DEE_PCKd8 "d" /* *ditto* */
#if __VA_SIZE >= 2
#define DEE_PCKu16 "u"
#define DEE_PCKd16 "d"
#elif __SIZEOF_SHORT__ == 2
#define DEE_PCKu16 "hu"
#define DEE_PCKd16 "hd"
#elif __SIZEOF_LONG__ == 2
#define DEE_PCKu16 "lu"
#define DEE_PCKd16 "ld"
#elif __SIZEOF_POINTER__ == 2
#define DEE_PCKu16 "Iu"
#define DEE_PCKd16 "Id"
#elif __SIZEOF_CHAR__ == 2
#define DEE_PCKu16 "hhu"
#define DEE_PCKd16 "hhd"
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 2
#define DEE_PCKu16 "llu"
#define DEE_PCKd16 "lld"
#else /* ... == 2 */
#define DEE_PCKu16 "I16u"
#define DEE_PCKd16 "I16d"
#endif /* ... != 2 */
#if __VA_SIZE >= 4
#define DEE_PCKu32 "u"
#define DEE_PCKd32 "d"
#elif __SIZEOF_LONG__ == 4
#define DEE_PCKu32 "lu"
#define DEE_PCKd32 "ld"
#elif __SIZEOF_SHORT__ == 4
#define DEE_PCKu32 "hu"
#define DEE_PCKd32 "hd"
#elif __SIZEOF_POINTER__ == 4
#define DEE_PCKu32 "Iu"
#define DEE_PCKd32 "Id"
#elif __SIZEOF_CHAR__ == 4
#define DEE_PCKu32 "hhu"
#define DEE_PCKd32 "hhd"
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 4
#define DEE_PCKu32 "llu"
#define DEE_PCKd32 "lld"
#else /* ... == 4 */
#define DEE_PCKu32 "I32u"
#define DEE_PCKd32 "I32d"
#endif /* ... != 4 */
#if __VA_SIZE >= 8
#define DEE_PCKu64 "u"
#define DEE_PCKd64 "d"
#elif __SIZEOF_LONG__ == 8
#define DEE_PCKu64 "lu"
#define DEE_PCKd64 "ld"
#elif __SIZEOF_POINTER__ == 8
#define DEE_PCKu64 "Iu"
#define DEE_PCKd64 "Id"
#elif __SIZEOF_SHORT__ == 8
#define DEE_PCKu64 "hu"
#define DEE_PCKd64 "hd"
#elif __SIZEOF_CHAR__ == 8
#define DEE_PCKu64 "hhu"
#define DEE_PCKd64 "hhd"
#elif defined(__SIZEOF_LONG_LONG__) && __SIZEOF_LONG_LONG__ == 8
#define DEE_PCKu64 "llu"
#define DEE_PCKd64 "lld"
#else /* ... == 8 */
#define DEE_PCKu64 "I64u"
#define DEE_PCKd64 "I64d"
#endif /* ... != 8 */
#define DEE_PCKu128 "I128u"
#define DEE_PCKd128 "I128d"

#define DEE_PRIVATE_PCKu1         DEE_PCKu8
#define DEE_PRIVATE_PCKd1         DEE_PCKd8
#define DEE_PRIVATE_PCKu2         DEE_PCKu16
#define DEE_PRIVATE_PCKd2         DEE_PCKd16
#define DEE_PRIVATE_PCKu4         DEE_PCKu32
#define DEE_PRIVATE_PCKd4         DEE_PCKd32
#define DEE_PRIVATE_PCKu8         DEE_PCKu64
#define DEE_PRIVATE_PCKd8         DEE_PCKd64
#define DEE_PRIVATE_PCKu16        DEE_PCKu128
#define DEE_PRIVATE_PCKd16        DEE_PCKd128
#define DEE_PRIVATE_PCKuN(sizeof) DEE_PRIVATE_PCKu##sizeof
#define DEE_PRIVATE_PCKdN(sizeof) DEE_PRIVATE_PCKd##sizeof
#define DEE_PCKuN(sizeof)         DEE_PRIVATE_PCKuN(sizeof)
#define DEE_PCKdN(sizeof)         DEE_PRIVATE_PCKdN(sizeof)

/* Helpful aliases */
#define DEE_PCKuSIZ DEE_PCKuN(__SIZEOF_SIZE_T__)
#define DEE_PCKdSIZ DEE_PCKdN(__SIZEOF_SIZE_T__)
#define DEE_PCKuPTR DEE_PCKuN(__SIZEOF_POINTER__)
#define DEE_PCKdPTR DEE_PCKdN(__SIZEOF_POINTER__)

/* Unescaped names. */
#ifdef DEE_SOURCE
#define PCKu8   DEE_PCKu8
#define PCKd8   DEE_PCKd8
#define PCKu16  DEE_PCKu16
#define PCKd16  DEE_PCKd16
#define PCKu32  DEE_PCKu32
#define PCKd32  DEE_PCKd32
#define PCKu64  DEE_PCKu64
#define PCKd64  DEE_PCKd64
#define PCKu128 DEE_PCKu128
#define PCKd128 DEE_PCKd128
#define PCKuN   DEE_PCKuN
#define PCKdN   DEE_PCKdN
#define PCKuSIZ DEE_PCKuSIZ
#define PCKdSIZ DEE_PCKdSIZ
#define PCKuPTR DEE_PCKuPTR
#define PCKdPTR DEE_PCKdPTR
#endif /* DEE_SOURCE */



/* Unpack values from an object.
 * Format language syntax:
 *     __main__   ::= object;
 *     object     ::= ('n' | '-')     // Ignore / skip this object. (Do not advance `va_arg')
 *                  | ref_object      // `va_arg(DeeObject **)'
 *                  | ref_int         //
 *                  | ref_float       //
 *                  | ref_bool        //
 *                  | ref_str         // `char **'
 *                  | '(' objects ')' // Enumerate elements of a sequence
 *     ;
 *     objects    ::= [(object   // Parse some object from the sequence.
 *                    | ','      // `,' is simply ignored, but can be used to prevent ambiguity.
 *                    | '|'      // Any following objects are optional (va_arg() is still invoked, but non-present elements are skipped)
 *                    )...];
 *     ref_object ::= 'o'; // `va_arg(DeeObject **)'
 *     ref_int    ::= [ref_intlen] ('d' | 'u' | 'i' | 'x'); // `u' and `x' read unsigned integers
 *     ref_str    ::= ['$']      // `va_arg(size_t *)' (Store the length of the string)
 *                  | 'l' 's'    // `va_arg(wchar_t **)'
 *                  | 'U16' 's'  // `va_arg(uint16_t **)'
 *                  | 'U32' 's'  // `va_arg(uint32_t **)'
 *                  | 's'        // `va_arg(char **)'
 *                  ;
 *     ref_intlen ::= 'I' ['8' | '16' | '32' | '64'] // Fixed-length / sizeof(size_t)
 *                  | 'hh' // `va_arg(char *)'
 *                  | 'h'  // `va_arg(short *)'
 *                  | ''   // `va_arg(int *)' (Default when nothing else was given)
 *                  | 'l'  // `va_arg(long *)'
 *                  | 'll' // `va_arg(long long *)' (__(U)LONGLONG *)
 *     ;
 *     ref_float  ::= 'f'  // `va_arg(float *)'
 *                  | 'D'  // `va_arg(double *)'
 *                  | 'LD' // `va_arg(long double *)'
 *     ;
 *     ref_bool   ::= 'b'; // `va_arg(bool)'
 */
DFUNDEF WUNUSED NONNULL((1, 2)) int
(Dee_Unpackf)(DeeObject *__restrict self, char const *__restrict format, ...);
DFUNDEF WUNUSED NONNULL((1, 2)) int
(DCALL Dee_VUnpackf)(DeeObject *__restrict self, char const *__restrict format, va_list args);
DFUNDEF WUNUSED NONNULL((1, 2, 3)) int
(DCALL Dee_VPUnpackf)(DeeObject *__restrict self,
                      char const **__restrict p_format,
                      struct va_list_struct *__restrict p_args);


/* Both of these functions return a pointer to the target address where
 * printing has/would have stopped (excluding a terminating \0-character).
 * In the event that the buffer provided to `Dee_snprintf' is insufficient,
 * at most `bufsize' characters will have been written, and the exact required
 * size can be determined by `((return - buffer) + 1) * sizeof(char)'.
 * In the event of an error (only possible when `format' contains
 * something like `%k' or `%r'), `NULL' will be returned. */
DFUNDEF NONNULL((1, 2)) char *
Dee_sprintf(char *__restrict buffer,
            char const *__restrict format, ...);
DFUNDEF NONNULL((3)) char *
Dee_snprintf(char *__restrict buffer, size_t bufsize,
             char const *__restrict format, ...);
DFUNDEF NONNULL((1, 2)) char *DCALL
Dee_vsprintf(char *__restrict buffer,
             char const *__restrict format, va_list args);
DFUNDEF NONNULL((3)) char *DCALL
Dee_vsnprintf(char *__restrict buffer, size_t bufsize,
              char const *__restrict format, va_list args);

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define Dee_Unpackf(self, ...)                __builtin_expect(Dee_Unpackf(self, __VA_ARGS__), 0)
#define Dee_VUnpackf(self, format, args)      __builtin_expect(Dee_VUnpackf(self, format, args), 0)
#define Dee_VPUnpackf(self, p_format, p_args) __builtin_expect(Dee_VPUnpackf(self, p_format, p_args), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */


/* ==========? Extensible formating functions ?========== */

/* Print the object types passed by the given argument list.
 * If given, also include keyword names & types from `kw'
 * >> foo(10, 1.0, "bar", enabled: true);
 * Printed: "int, float, string, enabled: bool" */
#define DeeFormat_PrintArgumentTypes(printer, arg, argc, argv) \
	DeeFormat_PrintArgumentTypesKw(printer, arg, argc, argv, NULL)
DFUNDEF WUNUSED NONNULL((1)) Dee_ssize_t
(DCALL DeeFormat_PrintArgumentTypesKw)(Dee_formatprinter_t printer, void *arg,
                                       size_t argc, DeeObject *const *argv,
                                       DeeObject *kw);

/* Print a representation of invoking operator `name' on `self' with the given arguments.
 * This function is used to generate the representation of the expression in the default
 * assertion failure handler.
 * NOTE: This function also accepts "fake" operators (`FAKE_OPERATOR_*') for `name' */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_PrintOperatorRepr(Dee_formatprinter_t printer, void *arg,
                            DeeObject *self, uint16_t name,
                            size_t argc, DeeObject *const *argv,
                            char const *self_prefix, size_t self_prefix_len,
                            char const *self_suffix, size_t self_suffix_len);


DECL_END

#endif /* !GUARD_DEEMON_FORMAT_H */
