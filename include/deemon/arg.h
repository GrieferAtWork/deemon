/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_ARG_H
#define GUARD_DEEMON_ARG_H 1

#include "api.h"
/**/

#include "types.h"
/**/

#include <hybrid/typecore.h>
/**/

#include <stdarg.h> /* va_list */
#include <stddef.h> /* size_t */

DECL_BEGIN


#ifdef DEE_SOURCE
#define K             Dee_KEYWORD
#define KS            Dee_KEYWORD_STR
#define KEX           Dee_KEYWORD_EX
#define KEND          Dee_KEYWORD_END
#define DEFINE_KWLIST Dee_DEFINE_KWLIST
#endif /* DEE_SOURCE */


/* An extension to `Dee_Unpackf', explicitly for unpacking elements from function arguments.
 * Format language syntax:
 *     using Dee_Unpackf::object;
 *     __main__   ::= [(object  // Process regular objects, writing values to pointers passed through varargs.
 *                    | '|'     // Marker: The remainder of the format is optional.
 *                      )...]
 *                    [':' <function_name>] // Optional, trailing function name (Used in error messages)
 *     ;
 * Example usage:
 * >> // function my_function(int a, int b, int c = 5) -> int;
 * >> // @return: * : The sum of `a', `b' and `c'
 * >> PRIVATE WUNUSED ATTR_INS(2, 1) NONNULL((1)) DREF DeeObject *DCALL
 * >> my_function(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
 * >>     int a, b, c = 5;
 * >>     if (DeeArg_Unpack(argc, argv, "dd|d:my_function", &a, &b, &c))
 * >>         return NULL;
 * >>     return DeeInt_NewInt(a + b + c);
 * >> }
 */
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((3)) int
(DeeArg_Unpack)(size_t argc, /*nonnull_if(argc != 0)*/ DeeObject *const *argv,
                char const *__restrict format, ...);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((3)) int
(DCALL DeeArg_VUnpack)(size_t argc, /*nonnull_if(argc != 0)*/ DeeObject *const *argv,
                       char const *__restrict format, va_list args);

#ifdef __OPTIMIZE_SIZE__
#define _DeeArg_Unpack0(err, argc, argv, function_name)   \
	do {                                                  \
		if (DeeArg_Unpack(argc, argv, ":" function_name)) \
			goto err;                                     \
	}	__WHILE0
#define _DeeArg_Unpack1(err, argc, argv, function_name, p_arg0)    \
	do {                                                           \
		if (DeeArg_Unpack(argc, argv, "o:" function_name, p_arg0)) \
			goto err;                                              \
	}	__WHILE0
#define _DeeArg_Unpack2(err, argc, argv, function_name, p_arg0, p_arg1)     \
	do {                                                                    \
		if (DeeArg_Unpack(argc, argv, "oo:" function_name, p_arg0, p_arg1)) \
			goto err;                                                       \
	}	__WHILE0
#define _DeeArg_Unpack3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2)      \
	do {                                                                             \
		if (DeeArg_Unpack(argc, argv, "ooo:" function_name, p_arg0, p_arg1, p_arg2)) \
			goto err;                                                                \
	}	__WHILE0
#define _DeeArg_Unpack0Or1(err, argc, argv, function_name, p_arg0)  \
	do {                                                            \
		if (DeeArg_Unpack(argc, argv, "|o:" function_name, p_arg0)) \
			goto err;                                               \
	}	__WHILE0
#define _DeeArg_Unpack1Or2(err, argc, argv, function_name, p_arg0, p_arg1)   \
	do {                                                                     \
		if (DeeArg_Unpack(argc, argv, "o|o:" function_name, p_arg0, p_arg1)) \
			goto err;                                                        \
	}	__WHILE0
#define _DeeArg_Unpack0Or1Or2(err, argc, argv, function_name, p_arg0, p_arg1) \
	do {                                                                      \
		if (DeeArg_Unpack(argc, argv, "|oo:" function_name, p_arg0, p_arg1))  \
			goto err;                                                         \
	}	__WHILE0
#define _DeeArg_Unpack1Or2Or3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2) \
	do {                                                                              \
		if (DeeArg_Unpack(argc, argv, "o|oo:" function_name, p_arg0, p_arg1, p_arg2)) \
			goto err;                                                                 \
	}	__WHILE0
#else /* __OPTIMIZE_SIZE__ */
#define _DeeArg_Unpack0(err, argc, argv, function_name) \
	if unlikely((argc) != 0) {                          \
		DeeArg_BadArgc0(function_name, argc);           \
		goto err;                                       \
	} else                                              \
		(void)(argv)
#define _DeeArg_Unpack1(err, argc, argv, function_name, p_arg0) \
	if unlikely((argc) != 1) {                                  \
		DeeArg_BadArgc1(function_name, argc);                   \
		goto err;                                               \
	} else                                                      \
		(void)(*(DeeObject **)(p_arg0) = (argv)[0],             \
		       __builtin_assume(*(p_arg0) != NULL))
#define _DeeArg_Unpack2(err, argc, argv, function_name, p_arg0, p_arg1) \
	if unlikely((argc) != 2) {                                          \
		DeeArg_BadArgc(function_name, argc, 2);                         \
		goto err;                                                       \
	} else                                                              \
		(void)(*(DeeObject **)(p_arg0) = (argv)[0],                     \
		       *(DeeObject **)(p_arg1) = (argv)[1],                     \
		       __builtin_assume(*(p_arg0) != NULL),                     \
		       __builtin_assume(*(p_arg1) != NULL))
#define _DeeArg_Unpack3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2) \
	if unlikely((argc) != 3) {                                                  \
		DeeArg_BadArgc(function_name, argc, 3);                                 \
		goto err;                                                               \
	} else                                                                      \
		(void)(*(DeeObject **)(p_arg0) = (argv)[0],                             \
		       *(DeeObject **)(p_arg1) = (argv)[1],                             \
		       *(DeeObject **)(p_arg2) = (argv)[2],                             \
		       __builtin_assume(*(p_arg0) != NULL),                             \
		       __builtin_assume(*(p_arg1) != NULL),                             \
		       __builtin_assume(*(p_arg2) != NULL))
#define _DeeArg_Unpack0Or1(err, argc, argv, function_name, p_arg0) \
	do {                                                           \
		switch (argc) {                                            \
		case 1:                                                    \
			*(DeeObject **)(p_arg0) = (argv)[0];                   \
			__builtin_assume(*(p_arg0) != NULL);                   \
			break;                                                 \
		case 0:                                                    \
			break;                                                 \
		default:                                                   \
			DeeArg_BadArgcEx(function_name, argc, 0, 1);           \
			goto err;                                              \
		}                                                          \
	}	__WHILE0
#define _DeeArg_Unpack1Or2(err, argc, argv, function_name, p_arg0, p_arg1) \
	do {                                                                   \
		switch (argc) {                                                    \
		case 2:                                                            \
			*(DeeObject **)(p_arg1) = (argv)[1];                           \
			__builtin_assume(*(p_arg1) != NULL);                           \
			ATTR_FALLTHROUGH                                               \
		case 1:                                                            \
			*(DeeObject **)(p_arg0) = (argv)[0];                           \
			__builtin_assume(*(p_arg0) != NULL);                           \
			break;                                                         \
		default:                                                           \
			DeeArg_BadArgcEx(function_name, argc, 1, 2);                   \
			goto err;                                                      \
		}                                                                  \
	}	__WHILE0
#define _DeeArg_Unpack0Or1Or2(err, argc, argv, function_name, p_arg0, p_arg1) \
	do {                                                                      \
		switch (argc) {                                                       \
		case 2:                                                               \
			*(DeeObject **)(p_arg1) = (argv)[1];                              \
			__builtin_assume(*(p_arg1) != NULL);                              \
			ATTR_FALLTHROUGH                                                  \
		case 1:                                                               \
			*(DeeObject **)(p_arg0) = (argv)[0];                              \
			__builtin_assume(*(p_arg0) != NULL);                              \
			break;                                                            \
		case 0:                                                               \
			break;                                                            \
		default:                                                              \
			DeeArg_BadArgcEx(function_name, argc, 0, 2);                      \
			goto err;                                                         \
		}                                                                     \
	}	__WHILE0
#define _DeeArg_Unpack1Or2Or3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2) \
	do {                                                                              \
		switch (argc) {                                                               \
		case 3:                                                                       \
			*(DeeObject **)(p_arg2) = (argv)[2];                                      \
			__builtin_assume(*(p_arg2) != NULL);                                      \
			ATTR_FALLTHROUGH                                                          \
		case 2:                                                                       \
			*(DeeObject **)(p_arg1) = (argv)[1];                                      \
			__builtin_assume(*(p_arg1) != NULL);                                      \
			ATTR_FALLTHROUGH                                                          \
		case 1:                                                                       \
			*(DeeObject **)(p_arg0) = (argv)[0];                                      \
			__builtin_assume(*(p_arg0) != NULL);                                      \
			break;                                                                    \
		default:                                                                      \
			DeeArg_BadArgcEx(function_name, argc, 1, 3);                              \
			goto err;                                                                 \
		}                                                                             \
	}	__WHILE0
#endif /* !__OPTIMIZE_SIZE__ */

/* Helper functions for throwing invalid-argc errors */
DFUNDEF ATTR_COLD int (DCALL DeeArg_BadArgc)(char const *function_name, size_t real_argc, size_t want_argc);
DFUNDEF ATTR_COLD int (DCALL DeeArg_BadArgc0)(char const *function_name, size_t real_argc);
DFUNDEF ATTR_COLD int (DCALL DeeArg_BadArgc1)(char const *function_name, size_t real_argc);
DFUNDEF ATTR_COLD int (DCALL DeeArg_BadArgcEx)(char const *function_name, size_t real_argc, size_t want_argc_min, size_t want_argc_max);
#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define DeeArg_BadArgc(function_name, real_argc, want_argc) \
	Dee_ASSUMED_VALUE((DeeArg_BadArgc)(function_name, real_argc, want_argc), -1)
#define DeeArg_BadArgc0(function_name, real_argc) \
	Dee_ASSUMED_VALUE((DeeArg_BadArgc0)(function_name, real_argc), -1)
#define DeeArg_BadArgc1(function_name, real_argc) \
	Dee_ASSUMED_VALUE((DeeArg_BadArgc1)(function_name, real_argc), -1)
#define DeeArg_BadArgcEx(function_name, real_argc, want_argc_min, want_argc_max) \
	Dee_ASSUMED_VALUE((DeeArg_BadArgcEx)(function_name, real_argc, want_argc_min, want_argc_max), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */


struct Dee_keyword {
	char const *k_name; /* [1..1][SENTINAL(NULL)] Keyword name. */
	Dee_hash_t  k_hash; /* [== Dee_HashStr(ke_name)]
	                     * Hash of this keyword (or (Dee_hash_t)-1 when not yet calculated).
	                     * Filled in the first time the keyword is used. */
};

#define Dee_KEYWORD(x)               { #x, (Dee_hash_t)-1 }
#define Dee_KEYWORD_STR(s)           { s, (Dee_hash_t)-1 }
#define Dee_KEYWORD_EX(s, h32, h64)  { s, _Dee_HashSelectC(h32, h64) }
#define Dee_KEYWORD_END              { NULL }
#define Dee_DEFINE_KWLIST(name, ...) struct Dee_keyword name[] = __VA_ARGS__

/* Same as the regular unpack functions above, however these are enabled to
 * support keyword lists in the event that the calling function has been
 * provided with a keyword object (`kw').
 * -> When `DeeKwds_Check(kw)' is true, keyword argument objects are passed
 *    through the regular argument vector, located within the range
 *   `argc - kw->kw_size .. argc - 1' (if `kw->kw_size > argc', a TypeError is thrown),
 *    using names from `kwlist + NUM_POSITIONAL' to match association.
 * -> Otherwise, positional arguments are also parsed regularly, before
 *    using `DeeObject_GetItemStringHash()' to lookup argument names starting
 *    at `kwlist + NUM_POSITIONAL', counting how may arguments were actually
 *    found (and failing if a non-optional argument wasn't given), before
 *    finally using `DeeObject_Size()' to see how many keyword-arguments
 *    were given by the keyword-list, and throwing an error if more were
 *    given than what was actually used. */
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4, 5)) int
(DeeArg_UnpackKw)(size_t argc, DeeObject *const *argv,
                  DeeObject *kw, struct Dee_keyword *__restrict kwlist,
                  char const *__restrict format, ...);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4, 5)) int
(DCALL DeeArg_VUnpackKw)(size_t argc, DeeObject *const *argv,
                         DeeObject *kw, struct Dee_keyword *__restrict kwlist,
                         char const *__restrict format, va_list args);

/* Same as the non-*Struct functions, but rather than taking 1 pointer per argument,
 * these take a single pointer to an aligned struct-blob (where each element is always
 * properly aligned, such that "uint32_t a; uint8_t b; uint32_t c;" has 3 padding bytes
 * before the "c" element).
 * Instead of:
 * >> size_t a, b, c;
 * >> if (DeeArg_Unpack(argc, argv, UNPuSIZ UNPuSIZ UNPuSIZ ":foo", &a, &b, &c))
 * >>     goto err;
 * You can do this, which will execute faster at runtime:
 * >> struct {
 * >>     size_t a;
 * >>     size_t b;
 * >>     size_t c;
 * >> } args;
 * >> if (DeeArg_UnpackStruct(argc, argv, UNPuSIZ UNPuSIZ UNPuSIZ ":foo", &args))
 * >>     goto err;
 */
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((3, 4)) int
(DCALL DeeArg_UnpackStruct)(size_t argc, DeeObject *const *argv,
                            char const *__restrict format, void *out);
DFUNDEF WUNUSED ATTR_INS(2, 1) NONNULL((4, 5, 6)) int
(DCALL DeeArg_UnpackStructKw)(size_t argc, DeeObject *const *argv,
                              DeeObject *kw, struct Dee_keyword *__restrict kwlist,
                              char const *__restrict format, void *out);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeArg_Unpack(argc, argv, ...)                             __builtin_expect(DeeArg_Unpack(argc, argv, __VA_ARGS__), 0)
#define DeeArg_UnpackKw(argc, argv, kw, kwlist, ...)               __builtin_expect(DeeArg_UnpackKw(argc, argv, kw, kwlist, __VA_ARGS__), 0)
#define DeeArg_VUnpack(argc, argv, format, args)                   __builtin_expect(DeeArg_VUnpack(argc, argv, format, args), 0)
#define DeeArg_VUnpackKw(argc, argv, kw, kwlist, format, args)     __builtin_expect(DeeArg_VUnpackKw(argc, argv, kw, kwlist, format, args), 0)
#define DeeArg_UnpackStruct(argc, argv, format, out)               __builtin_expect(DeeArg_UnpackStruct(argc, argv, format, out), 0)
#define DeeArg_UnpackStructKw(argc, argv, kw, kwlist, format, out) __builtin_expect(DeeArg_UnpackStructKw(argc, argv, kw, kwlist, format, out), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */



/* Optimized format sequences for `Dee_Unpackf()' and `DeeArg_Unpack[Kw]()' */
#define DEE_UNPu8   "I8u"
#define DEE_UNPd8   "I8d"
#define DEE_UNPu16  "I16u"
#define DEE_UNPd16  "I16d"
#define DEE_UNPu32  "I32u"
#define DEE_UNPd32  "I32d"
#define DEE_UNPu64  "I64u"
#define DEE_UNPd64  "I64d"
#define DEE_UNPu128 "I128u"
#define DEE_UNPd128 "I128d"
#ifdef __SIZEOF_LONG_LONG__
#if __SIZEOF_LONG_LONG__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "llu"
#define DEE_UNPd64 "lld"
#elif __SIZEOF_LONG_LONG__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "llu"
#define DEE_UNPd32 "lld"
#elif __SIZEOF_LONG_LONG__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "llu"
#define DEE_UNPd16 "lld"
#endif /* __SIZEOF_LONG_LONG__ == ... */
#endif /* __SIZEOF_LONG_LONG__ */
#if __SIZEOF_CHAR__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "hhu"
#define DEE_UNPd64 "hhd"
#elif __SIZEOF_CHAR__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "hhu"
#define DEE_UNPd32 "hhd"
#elif __SIZEOF_CHAR__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "hhu"
#define DEE_UNPd16 "hhd"
#endif /* __SIZEOF_CHAR__ == ... */
#if __SIZEOF_LONG__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "lu"
#define DEE_UNPd64 "ld"
#elif __SIZEOF_LONG__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "lu"
#define DEE_UNPd32 "ld"
#elif __SIZEOF_LONG__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "lu"
#define DEE_UNPd16 "ld"
#elif __SIZEOF_LONG__ == 1
#undef DEE_UNPu8
#undef DEE_UNPd8
#define DEE_UNPu8 "lu"
#define DEE_UNPd8 "ld"
#endif /* __SIZEOF_LONG__ == ... */
#if __SIZEOF_SHORT__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "hu"
#define DEE_UNPd64 "hd"
#elif __SIZEOF_SHORT__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "hu"
#define DEE_UNPd32 "hd"
#elif __SIZEOF_SHORT__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "hu"
#define DEE_UNPd16 "hd"
#elif __SIZEOF_SHORT__ == 1
#undef DEE_UNPu8
#undef DEE_UNPd8
#define DEE_UNPu8 "hu"
#define DEE_UNPd8 "hd"
#endif /* __SIZEOF_SHORT__ == ... */
#if __SIZEOF_INT__ == 8
#undef DEE_UNPu64
#undef DEE_UNPd64
#define DEE_UNPu64 "u"
#define DEE_UNPd64 "d"
#elif __SIZEOF_INT__ == 4
#undef DEE_UNPu32
#undef DEE_UNPd32
#define DEE_UNPu32 "u"
#define DEE_UNPd32 "d"
#elif __SIZEOF_INT__ == 2
#undef DEE_UNPu16
#undef DEE_UNPd16
#define DEE_UNPu16 "u"
#define DEE_UNPd16 "d"
#elif __SIZEOF_INT__ == 1
#undef DEE_UNPu8
#undef DEE_UNPd8
#define DEE_UNPu8 "u"
#define DEE_UNPd8 "d"
#endif /* __SIZEOF_INT__ == ... */
#define DEE_PRIVATE_UNPu1         DEE_UNPu8
#define DEE_PRIVATE_UNPd1         DEE_UNPd8
#define DEE_PRIVATE_UNPu2         DEE_UNPu16
#define DEE_PRIVATE_UNPd2         DEE_UNPd16
#define DEE_PRIVATE_UNPu4         DEE_UNPu32
#define DEE_PRIVATE_UNPd4         DEE_UNPd32
#define DEE_PRIVATE_UNPu8         DEE_UNPu64
#define DEE_PRIVATE_UNPd8         DEE_UNPd64
#define DEE_PRIVATE_UNPuN(sizeof) DEE_PRIVATE_UNPu##sizeof
#define DEE_PRIVATE_UNPdN(sizeof) DEE_PRIVATE_UNPd##sizeof
#define DEE_UNPuN(sizeof)         DEE_PRIVATE_UNPuN(sizeof)
#define DEE_UNPdN(sizeof)         DEE_PRIVATE_UNPdN(sizeof)

/* Helpful aliases */
#define DEE_UNPdSIZ DEE_UNPdN(__SIZEOF_SIZE_T__)
#define DEE_UNPuSIZ DEE_UNPuN(__SIZEOF_SIZE_T__)
#define DEE_UNPdPTR DEE_UNPdN(__SIZEOF_POINTER__)
#define DEE_UNPuPTR DEE_UNPuN(__SIZEOF_POINTER__)
#define DEE_UNPdB   DEE_UNPd8
#define DEE_UNPuB   DEE_UNPu8

/* Unescaped names. */
#ifdef DEE_SOURCE
#define UNPdB   DEE_UNPdB
#define UNPuB   DEE_UNPuB
#define UNPd8   DEE_UNPd8
#define UNPu8   DEE_UNPu8
#define UNPd16  DEE_UNPd16
#define UNPu16  DEE_UNPu16
#define UNPd32  DEE_UNPd32
#define UNPu32  DEE_UNPu32
#define UNPd64  DEE_UNPd64
#define UNPu64  DEE_UNPu64
#define UNPd128 DEE_UNPd128
#define UNPu128 DEE_UNPu128
#define UNPdN   DEE_UNPdN
#define UNPuN   DEE_UNPuN
#define UNPdSIZ DEE_UNPdSIZ
#define UNPuSIZ DEE_UNPuSIZ
#define UNPdPTR DEE_UNPdPTR
#define UNPuPTR DEE_UNPuPTR
#endif /* DEE_SOURCE */

DECL_END

#endif /* !GUARD_DEEMON_ARG_H */
