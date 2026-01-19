/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_ARG_H
#define GUARD_DEEMON_ARG_H 1

#include "api.h"

#include <hybrid/typecore.h>

#include "types.h"

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
#define DeeArg_Unpack0(err, argc, argv, function_name)    \
	do {                                                  \
		if (DeeArg_Unpack(argc, argv, ":" function_name)) \
			goto err;                                     \
	}	__WHILE0
#define DeeArg_Unpack1(err, argc, argv, function_name, p_arg0)           \
	do {                                                                 \
		if (DeeArg_UnpackStruct(argc, argv, "o:" function_name, p_arg0)) \
			goto err;                                                    \
	}	__WHILE0
#define DeeArg_Unpack2(err, argc, argv, function_name, p_arg0, p_arg1)      \
	do {                                                                    \
		if (DeeArg_Unpack(argc, argv, "oo:" function_name, p_arg0, p_arg1)) \
			goto err;                                                       \
	}	__WHILE0
#define DeeArg_Unpack3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2)       \
	do {                                                                             \
		if (DeeArg_Unpack(argc, argv, "ooo:" function_name, p_arg0, p_arg1, p_arg2)) \
			goto err;                                                                \
	}	__WHILE0
#define DeeArg_Unpack0Or1(err, argc, argv, function_name, p_arg0)         \
	do {                                                                  \
		if (DeeArg_UnpackStruct(argc, argv, "|o:" function_name, p_arg0)) \
			goto err;                                                     \
	}	__WHILE0
#define DeeArg_Unpack1Or2(err, argc, argv, function_name, p_arg0, p_arg1)    \
	do {                                                                     \
		if (DeeArg_Unpack(argc, argv, "o|o:" function_name, p_arg0, p_arg1)) \
			goto err;                                                        \
	}	__WHILE0
#define DeeArg_Unpack0Or1Or2(err, argc, argv, function_name, p_arg0, p_arg1) \
	do {                                                                     \
		if (DeeArg_Unpack(argc, argv, "|oo:" function_name, p_arg0, p_arg1)) \
			goto err;                                                        \
	}	__WHILE0
#define DeeArg_Unpack1Or2Or3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2)  \
	do {                                                                              \
		if (DeeArg_Unpack(argc, argv, "o|oo:" function_name, p_arg0, p_arg1, p_arg2)) \
			goto err;                                                                 \
	}	__WHILE0
#define DeeArg_Unpack0Or1Or2Or3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2) \
	do {                                                                                \
		if (DeeArg_Unpack(argc, argv, "|ooo:" function_name, p_arg0, p_arg1, p_arg2))   \
			goto err;                                                                   \
	}	__WHILE0
#define DeeArg_Unpack1X(err, argc, argv, function_name,                      \
                        p_arg0, UNP0, Object_AsUnp0)                         \
	do {                                                                     \
		if (DeeArg_UnpackStruct(argc, argv, UNP0 ":" function_name, p_arg0)) \
			goto err;                                                        \
	}	__WHILE0
#define DeeArg_Unpack2X(err, argc, argv, function_name,            \
                        p_arg0, UNP0, Object_AsUnp0,               \
                        p_arg1, UNP1, Object_AsUnp1)               \
	do {                                                           \
		if (DeeArg_Unpack(argc, argv, UNP0 UNP1 ":" function_name, \
		                  p_arg0, p_arg1))                         \
			goto err;                                              \
	}	__WHILE0
#define DeeArg_Unpack3X(err, argc, argv, function_name,                 \
                        p_arg0, UNP0, Object_AsUnp0,                    \
                        p_arg1, UNP1, Object_AsUnp1,                    \
                        p_arg2, UNP2, Object_AsUnp2)                    \
	do {                                                                \
		if (DeeArg_Unpack(argc, argv, UNP0 UNP1 UNP2 ":" function_name, \
		                  p_arg0, p_arg1, p_arg2))                      \
			goto err;                                                   \
	}	__WHILE0
#define DeeArg_Unpack0Or1X(err, argc, argv, function_name,              \
                           p_arg0, UNP0, Object_AsUnp0)                 \
	do {                                                                \
		if (DeeArg_UnpackStruct(argc, argv, "|" UNP0 ":" function_name, \
		                        p_arg0))                                \
			goto err;                                                   \
	}	__WHILE0
#define DeeArg_Unpack1XOr2X(err, argc, argv, function_name,            \
                            p_arg0, UNP0, Object_AsUnp0,               \
                            p_arg1, UNP1, Object_AsUnp1)               \
	do {                                                               \
		if (DeeArg_Unpack(argc, argv, UNP0 "|" UNP1 ":" function_name, \
		                  p_arg0, p_arg1))                             \
			goto err;                                                  \
	}	__WHILE0
#define DeeArg_Unpack0Or1XOr2X(err, argc, argv, function_name,         \
                               p_arg0, UNP0, Object_AsUnp0,            \
                               p_arg1, UNP1, Object_AsUnp1)            \
	do {                                                               \
		if (DeeArg_Unpack(argc, argv, "|" UNP0 UNP1 ":" function_name, \
		                  p_arg0, p_arg1))                             \
			goto err;                                                  \
	}	__WHILE0
#define DeeArg_Unpack1XOr2XOr3X(err, argc, argv, function_name,             \
                                p_arg0, UNP0, Object_AsUnp0,                \
                                p_arg1, UNP1, Object_AsUnp1,                \
                                p_arg2, UNP2, Object_AsUnp2)                \
	do {                                                                    \
		if (DeeArg_Unpack(argc, argv, UNP0 "|" UNP1 UNP2 ":" function_name, \
		                  p_arg0, p_arg1, p_arg2))                          \
			goto err;                                                       \
	}	__WHILE0
#define DeeArg_Unpack0Or1XOr2XOr3X(err, argc, argv, function_name,          \
                                   p_arg0, UNP0, Object_AsUnp0,             \
                                   p_arg1, UNP1, Object_AsUnp1,             \
                                   p_arg2, UNP2, Object_AsUnp2)             \
	do {                                                                    \
		if (DeeArg_Unpack(argc, argv, "|" UNP0 UNP1 UNP2 ":" function_name, \
		                  p_arg0, p_arg1, p_arg2))                          \
			goto err;                                                       \
	}	__WHILE0
#define DeeArg_UnpackStruct2(err, argc, argv, function_name, p_args, p_arg0, p_arg1) \
	do {                                                                             \
		if (DeeArg_UnpackStruct(argc, argv, "oo:" function_name, p_args))            \
			goto err;                                                                \
	}	__WHILE0
#define DeeArg_UnpackStruct3(err, argc, argv, function_name, p_args, p_arg0, p_arg1, p_arg2) \
	do {                                                                                     \
		if (DeeArg_UnpackStruct(argc, argv, "ooo:" function_name, p_args))                   \
			goto err;                                                                        \
	}	__WHILE0
#define DeeArg_UnpackStruct1Or2(err, argc, argv, function_name, p_args, p_arg0, p_arg1) \
	do {                                                                                \
		if (DeeArg_UnpackStruct(argc, argv, "o|o:" function_name, p_args))              \
			goto err;                                                                   \
	}	__WHILE0
#define DeeArg_UnpackStruct0Or1Or2(err, argc, argv, function_name, p_args, p_arg0, p_arg1) \
	do {                                                                                   \
		if (DeeArg_UnpackStruct(argc, argv, "|oo:" function_name, p_args))                 \
			goto err;                                                                      \
	}	__WHILE0
#define DeeArg_UnpackStruct1Or2Or3(err, argc, argv, function_name, p_args, p_arg0, p_arg1, p_arg2) \
	do {                                                                                           \
		if (DeeArg_UnpackStruct(argc, argv, "o|oo:" function_name, p_args))                        \
			goto err;                                                                              \
	}	__WHILE0
#define DeeArg_UnpackStruct0Or1Or2Or3(err, argc, argv, function_name, p_args, p_arg0, p_arg1, p_arg2) \
	do {                                                                                              \
		if (DeeArg_UnpackStruct(argc, argv, "|ooo:" function_name, p_args))                           \
			goto err;                                                                                 \
	}	__WHILE0
#define DeeArg_UnpackStruct2X(err, argc, argv, function_name, p_args,             \
                              p_arg0, UNP0, Object_AsUnp0,                        \
                              p_arg1, UNP1, Object_AsUnp1)                        \
	do {                                                                          \
		if (DeeArg_UnpackStruct(argc, argv, UNP0 UNP1 ":" function_name, p_args)) \
			goto err;                                                             \
	}	__WHILE0
#define DeeArg_UnpackStruct3X(err, argc, argv, function_name, p_args,                  \
                              p_arg0, UNP0, Object_AsUnp0,                             \
                              p_arg1, UNP1, Object_AsUnp1,                             \
                              p_arg2, UNP2, Object_AsUnp2)                             \
	do {                                                                               \
		if (DeeArg_UnpackStruct(argc, argv, UNP0 UNP1 UNP2 ":" function_name, p_args)) \
			goto err;                                                                  \
	}	__WHILE0
#define DeeArg_UnpackStruct1XOr2X(err, argc, argv, function_name, p_args,             \
                                  p_arg0, UNP0, Object_AsUnp0,                        \
                                  p_arg1, UNP1, Object_AsUnp1)                        \
	do {                                                                              \
		if (DeeArg_UnpackStruct(argc, argv, UNP0 "|" UNP1 ":" function_name, p_args)) \
			goto err;                                                                 \
	}	__WHILE0
#define DeeArg_UnpackStruct0Or1XOr2X(err, argc, argv, function_name, p_args,          \
                                     p_arg0, UNP0, Object_AsUnp0,                     \
                                     p_arg1, UNP1, Object_AsUnp1)                     \
	do {                                                                              \
		if (DeeArg_UnpackStruct(argc, argv, "|" UNP0 UNP1 ":" function_name, p_args)) \
			goto err;                                                                 \
	}	__WHILE0
#define DeeArg_UnpackStruct1XOr2XOr3X(err, argc, argv, function_name, p_args,              \
                                      p_arg0, UNP0, Object_AsUnp0,                         \
                                      p_arg1, UNP1, Object_AsUnp1,                         \
                                      p_arg2, UNP2, Object_AsUnp2)                         \
	do {                                                                                   \
		if (DeeArg_UnpackStruct(argc, argv, UNP0 "|" UNP1 UNP2 ":" function_name, p_args)) \
			goto err;                                                                      \
	}	__WHILE0
#define DeeArg_UnpackStruct0Or1XOr2XOr3X(err, argc, argv, function_name, p_args,           \
                                         p_arg0, UNP0, Object_AsUnp0,                      \
                                         p_arg1, UNP1, Object_AsUnp1,                      \
                                         p_arg2, UNP2, Object_AsUnp2)                      \
	do {                                                                                   \
		if (DeeArg_UnpackStruct(argc, argv, "|" UNP0 UNP1 UNP2 ":" function_name, p_args)) \
			goto err;                                                                      \
	}	__WHILE0
#else /* __OPTIMIZE_SIZE__ */
#ifdef __COMPILER_HAVE_TYPEOF
#define __DeeArg_ASSIGN(p_arg, value) (*(p_arg) = (__typeof__(*(p_arg)))(value))
#else /* __COMPILER_HAVE_TYPEOF */
/* !!! This version here technically breaks "strict-aliasing rules", but without
 * !!! compiler support for "typeof", there's nothing we can do about that. Luckily,
 * !!! MSVC (being the only compiler not to support typeof) doesn't care about
 * !!! strict aliasing rules. */
#define __DeeArg_ASSIGN(p_arg, value) (*(DeeObject **)(p_arg) = (value))
#endif /* !__COMPILER_HAVE_TYPEOF */

/* Dummy converter function used to encode "o" (object) operands in `DeeArg_Unpack<N>X' */
#define _DeeArg_AsObject(self, p_result) (__DeeArg_ASSIGN(p_result, self), 0)

/* Cat with empty string to assert that "function_name" is a compile-time
 * string literal (if it isn't, then the __OPTIMIZE_SIZE__ variant above
 * would fail to compile) */
#define _DeeArg_FuncName(function_name) "" function_name
#define DeeArg_Unpack0(err, argc, argv, function_name)          \
	if unlikely ((argc) != 0) {                                 \
		DeeArg_BadArgc0(_DeeArg_FuncName(function_name), argc); \
		goto err;                                               \
	} else                                                      \
		(void)(argv)
#define DeeArg_Unpack1(err, argc, argv, function_name, p_arg0)  \
	if unlikely ((argc) != 1) {                                 \
		DeeArg_BadArgc1(_DeeArg_FuncName(function_name), argc); \
		goto err;                                               \
	} else                                                      \
		(void)(__DeeArg_ASSIGN(p_arg0, (argv)[0]),              \
		       __builtin_assume(*(p_arg0)))
#define DeeArg_Unpack2(err, argc, argv, function_name, p_arg0, p_arg1) \
	if unlikely ((argc) != 2) {                                        \
		DeeArg_BadArgc(_DeeArg_FuncName(function_name), argc, 2);      \
		goto err;                                                      \
	} else                                                             \
		(void)(__DeeArg_ASSIGN(p_arg0, (argv)[0]),                     \
		       __DeeArg_ASSIGN(p_arg1, (argv)[1]),                     \
		       __builtin_assume(*(p_arg0)),                            \
		       __builtin_assume(*(p_arg1)))
#define DeeArg_Unpack3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2) \
	if unlikely ((argc) != 3) {                                                \
		DeeArg_BadArgc(_DeeArg_FuncName(function_name), argc, 3);              \
		goto err;                                                              \
	} else                                                                     \
		(void)(__DeeArg_ASSIGN(p_arg0, (argv)[0]),                             \
		       __DeeArg_ASSIGN(p_arg1, (argv)[1]),                             \
		       __DeeArg_ASSIGN(p_arg2, (argv)[2]),                             \
		       __builtin_assume(*(p_arg0)),                                    \
		       __builtin_assume(*(p_arg1)),                                    \
		       __builtin_assume(*(p_arg2)))
#define DeeArg_Unpack0Or1(err, argc, argv, function_name, p_arg0)          \
	do {                                                                   \
		switch (argc) {                                                    \
		case 1:                                                            \
			__DeeArg_ASSIGN(p_arg0, (argv)[0]);                            \
			__builtin_assume(*(p_arg0));                                   \
			break;                                                         \
		case 0:                                                            \
			break;                                                         \
		default:                                                           \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 0, 1); \
			goto err;                                                      \
		}                                                                  \
	}	__WHILE0
#define DeeArg_Unpack1Or2(err, argc, argv, function_name, p_arg0, p_arg1)  \
	do {                                                                   \
		switch (argc) {                                                    \
		case 2:                                                            \
			__DeeArg_ASSIGN(p_arg1, (argv)[1]);                            \
			__builtin_assume(*(p_arg1));                                   \
			ATTR_FALLTHROUGH                                               \
		case 1:                                                            \
			__DeeArg_ASSIGN(p_arg0, (argv)[0]);                            \
			__builtin_assume(*(p_arg0));                                   \
			break;                                                         \
		default:                                                           \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 1, 2); \
			goto err;                                                      \
		}                                                                  \
	}	__WHILE0
#define DeeArg_Unpack0Or1Or2(err, argc, argv, function_name, p_arg0, p_arg1) \
	do {                                                                     \
		switch (argc) {                                                      \
		case 2:                                                              \
			__DeeArg_ASSIGN(p_arg1, (argv)[1]);                              \
			__builtin_assume(*(p_arg1));                                     \
			ATTR_FALLTHROUGH                                                 \
		case 1:                                                              \
			__DeeArg_ASSIGN(p_arg0, (argv)[0]);                              \
			__builtin_assume(*(p_arg0));                                     \
			break;                                                           \
		case 0:                                                              \
			break;                                                           \
		default:                                                             \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 0, 2);   \
			goto err;                                                        \
		}                                                                    \
	}	__WHILE0
#define DeeArg_Unpack1Or2Or3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2) \
	do {                                                                             \
		switch (argc) {                                                              \
		case 3:                                                                      \
			__DeeArg_ASSIGN(p_arg2, (argv)[2]);                                      \
			__builtin_assume(*(p_arg2));                                             \
			ATTR_FALLTHROUGH                                                         \
		case 2:                                                                      \
			__DeeArg_ASSIGN(p_arg1, (argv)[1]);                                      \
			__builtin_assume(*(p_arg1));                                             \
			ATTR_FALLTHROUGH                                                         \
		case 1:                                                                      \
			__DeeArg_ASSIGN(p_arg0, (argv)[0]);                                      \
			__builtin_assume(*(p_arg0));                                             \
			break;                                                                   \
		default:                                                                     \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 1, 3);           \
			goto err;                                                                \
		}                                                                            \
	}	__WHILE0
#define DeeArg_Unpack0Or1Or2Or3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2) \
	do {                                                                                \
		switch (argc) {                                                                 \
		case 3:                                                                         \
			__DeeArg_ASSIGN(p_arg2, (argv)[2]);                                         \
			__builtin_assume(*(p_arg2));                                                \
			ATTR_FALLTHROUGH                                                            \
		case 2:                                                                         \
			__DeeArg_ASSIGN(p_arg1, (argv)[1]);                                         \
			__builtin_assume(*(p_arg1));                                                \
			ATTR_FALLTHROUGH                                                            \
		case 1:                                                                         \
			__DeeArg_ASSIGN(p_arg0, (argv)[0]);                                         \
			__builtin_assume(*(p_arg0));                                                \
			break;                                                                      \
		case 0:                                                                         \
			break;                                                                      \
		default:                                                                        \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 0, 3);              \
			goto err;                                                                   \
		}                                                                               \
	}	__WHILE0
#define DeeArg_Unpack1X(err, argc, argv, function_name,             \
                        p_arg0, UNP0, Object_AsUnp0)                \
	do {                                                            \
		if unlikely ((argc) != 1) {                                 \
			DeeArg_BadArgc1(_DeeArg_FuncName(function_name), argc); \
			goto err;                                               \
		}                                                           \
		if unlikely (Object_AsUnp0((argv)[0], p_arg0))              \
			goto err;                                               \
	}	__WHILE0
#define DeeArg_Unpack2X(err, argc, argv, function_name,               \
                        p_arg0, UNP0, Object_AsUnp0,                  \
                        p_arg1, UNP1, Object_AsUnp1)                  \
	do {                                                              \
		if unlikely ((argc) != 2) {                                   \
			DeeArg_BadArgc(_DeeArg_FuncName(function_name), argc, 2); \
			goto err;                                                 \
		}                                                             \
		if unlikely (Object_AsUnp0((argv)[0], p_arg0))                \
			goto err;                                                 \
		if unlikely (Object_AsUnp1((argv)[1], p_arg1))                \
			goto err;                                                 \
	}	__WHILE0
#define DeeArg_Unpack3X(err, argc, argv, function_name,               \
                        p_arg0, UNP0, Object_AsUnp0,                  \
                        p_arg1, UNP1, Object_AsUnp1,                  \
                        p_arg2, UNP2, Object_AsUnp2)                  \
	do {                                                              \
		if unlikely ((argc) != 3) {                                   \
			DeeArg_BadArgc(_DeeArg_FuncName(function_name), argc, 3); \
			goto err;                                                 \
		}                                                             \
		if unlikely (Object_AsUnp0((argv)[0], p_arg0))                \
			goto err;                                                 \
		if unlikely (Object_AsUnp1((argv)[1], p_arg1))                \
			goto err;                                                 \
		if unlikely (Object_AsUnp2((argv)[2], p_arg2))                \
			goto err;                                                 \
	}	__WHILE0
#define DeeArg_Unpack0Or1X(err, argc, argv, function_name,                 \
                           p_arg0, UNP0, Object_AsUnp0)                    \
	do {                                                                   \
		switch (argc) {                                                    \
		case 1:                                                            \
			if unlikely (Object_AsUnp0((argv)[0], p_arg0))                 \
				goto err;                                                  \
			break;                                                         \
		case 0:                                                            \
			break;                                                         \
		default:                                                           \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 0, 1); \
			goto err;                                                      \
		}                                                                  \
	}	__WHILE0
#define DeeArg_Unpack1XOr2X(err, argc, argv, function_name,                \
                            p_arg0, UNP0, Object_AsUnp0,                   \
                            p_arg1, UNP1, Object_AsUnp1)                   \
	do {                                                                   \
		switch (argc) {                                                    \
		case 2:                                                            \
			if unlikely (Object_AsUnp1((argv)[1], p_arg1))                 \
				goto err;                                                  \
			ATTR_FALLTHROUGH                                               \
		case 1:                                                            \
			if unlikely (Object_AsUnp0((argv)[0], p_arg0))                 \
				goto err;                                                  \
			break;                                                         \
		default:                                                           \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 1, 2); \
			goto err;                                                      \
		}                                                                  \
	}	__WHILE0
#define DeeArg_Unpack0Or1XOr2X(err, argc, argv, function_name,             \
                               p_arg0, UNP0, Object_AsUnp0,                \
                               p_arg1, UNP1, Object_AsUnp1)                \
	do {                                                                   \
		switch (argc) {                                                    \
		case 2:                                                            \
			if unlikely (Object_AsUnp1((argv)[1], p_arg1))                 \
				goto err;                                                  \
			ATTR_FALLTHROUGH                                               \
		case 1:                                                            \
			if unlikely (Object_AsUnp0((argv)[0], p_arg0))                 \
				goto err;                                                  \
			break;                                                         \
		case 0:                                                            \
			break;                                                         \
		default:                                                           \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 0, 2); \
			goto err;                                                      \
		}                                                                  \
	}	__WHILE0
#define DeeArg_Unpack1XOr2XOr3X(err, argc, argv, function_name,            \
                                p_arg0, UNP0, Object_AsUnp0,               \
                                p_arg1, UNP1, Object_AsUnp1,               \
                                p_arg2, UNP2, Object_AsUnp2)               \
	do {                                                                   \
		switch (argc) {                                                    \
		case 3:                                                            \
			if unlikely (Object_AsUnp2((argv)[2], p_arg2))                 \
				goto err;                                                  \
			ATTR_FALLTHROUGH                                               \
		case 2:                                                            \
			if unlikely (Object_AsUnp1((argv)[1], p_arg1))                 \
				goto err;                                                  \
			ATTR_FALLTHROUGH                                               \
		case 1:                                                            \
			if unlikely (Object_AsUnp0((argv)[0], p_arg0))                 \
				goto err;                                                  \
			break;                                                         \
		default:                                                           \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 1, 3); \
			goto err;                                                      \
		}                                                                  \
	}	__WHILE0
#define DeeArg_Unpack0Or1XOr2XOr3X(err, argc, argv, function_name,         \
                                   p_arg0, UNP0, Object_AsUnp0,            \
                                   p_arg1, UNP1, Object_AsUnp1,            \
                                   p_arg2, UNP2, Object_AsUnp2)            \
	do {                                                                   \
		switch (argc) {                                                    \
		case 3:                                                            \
			if unlikely (Object_AsUnp2((argv)[2], p_arg2))                 \
				goto err;                                                  \
			ATTR_FALLTHROUGH                                               \
		case 2:                                                            \
			if unlikely (Object_AsUnp1((argv)[1], p_arg1))                 \
				goto err;                                                  \
			ATTR_FALLTHROUGH                                               \
		case 1:                                                            \
			if unlikely (Object_AsUnp0((argv)[0], p_arg0))                 \
				goto err;                                                  \
			break;                                                         \
		case 0:                                                            \
			break;                                                         \
		default:                                                           \
			DeeArg_BadArgcEx(_DeeArg_FuncName(function_name), argc, 0, 3); \
			goto err;                                                      \
		}                                                                  \
	}	__WHILE0
#define DeeArg_UnpackStruct2(err, argc, argv, function_name, p_args, p_arg0, p_arg1) \
	DeeArg_Unpack2(err, argc, argv, function_name, p_arg0, p_arg1)
#define DeeArg_UnpackStruct3(err, argc, argv, function_name, p_args, p_arg0, p_arg1, p_arg2) \
	DeeArg_Unpack3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2)
#define DeeArg_UnpackStruct1Or2(err, argc, argv, function_name, p_args, p_arg0, p_arg1) \
	DeeArg_Unpack1Or2(err, argc, argv, function_name, p_arg0, p_arg1)
#define DeeArg_UnpackStruct0Or1Or2(err, argc, argv, function_name, p_args, p_arg0, p_arg1) \
	DeeArg_Unpack0Or1Or2(err, argc, argv, function_name, p_arg0, p_arg1)
#define DeeArg_UnpackStruct1Or2Or3(err, argc, argv, function_name, p_args, p_arg0, p_arg1, p_arg2) \
	DeeArg_Unpack1Or2Or3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2)
#define DeeArg_UnpackStruct0Or1Or2Or3(err, argc, argv, function_name, p_args, p_arg0, p_arg1, p_arg2) \
	DeeArg_Unpack0Or1Or2Or3(err, argc, argv, function_name, p_arg0, p_arg1, p_arg2)
#define DeeArg_UnpackStruct2X(err, argc, argv, function_name, p_args, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1) \
	DeeArg_Unpack2X(err, argc, argv, function_name, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1)
#define DeeArg_UnpackStruct3X(err, argc, argv, function_name, p_args, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1, p_arg2, UNP2, Object_AsUnp2) \
	DeeArg_Unpack3X(err, argc, argv, function_name, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1, p_arg2, UNP2, Object_AsUnp2)
#define DeeArg_UnpackStruct1XOr2X(err, argc, argv, function_name, p_args, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1) \
	DeeArg_Unpack1XOr2X(err, argc, argv, function_name, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1)
#define DeeArg_UnpackStruct0Or1XOr2X(err, argc, argv, function_name, p_args, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1) \
	DeeArg_Unpack0Or1XOr2X(err, argc, argv, function_name, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1)
#define DeeArg_UnpackStruct1XOr2XOr3X(err, argc, argv, function_name, p_args, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1, p_arg2, UNP2, Object_AsUnp2) \
	DeeArg_Unpack1XOr2XOr3X(err, argc, argv, function_name, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1, p_arg2, UNP2, Object_AsUnp2)
#define DeeArg_UnpackStruct0Or1XOr2XOr3X(err, argc, argv, function_name, p_args, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1, p_arg2, UNP2, Object_AsUnp2) \
	DeeArg_Unpack0Or1XOr2XOr3X(err, argc, argv, function_name, p_arg0, UNP0, Object_AsUnp0, p_arg1, UNP1, Object_AsUnp1, p_arg2, UNP2, Object_AsUnp2)
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
#define Dee_UNPd8   "I8d"   /* DeeObject_AsInt8(...) */
#define Dee_UNPu8   "I8u"   /* DeeObject_AsUInt8(...) */
#define Dee_UNPx8   "I8x"   /* DeeObject_AsUInt8M1(...) */
#define Dee_UNPd16  "I16d"  /* DeeObject_AsInt16(...) */
#define Dee_UNPu16  "I16u"  /* DeeObject_AsUInt16(...) */
#define Dee_UNPx16  "I16x"  /* DeeObject_AsUInt16M1(...) */
#define Dee_UNPd32  "I32d"  /* DeeObject_AsInt32(...) */
#define Dee_UNPu32  "I32u"  /* DeeObject_AsUInt32(...) */
#define Dee_UNPx32  "I32x"  /* DeeObject_AsUInt32M1(...) */
#define Dee_UNPd64  "I64d"  /* DeeObject_AsInt64(...) */
#define Dee_UNPu64  "I64u"  /* DeeObject_AsUInt64(...) */
#define Dee_UNPx64  "I64x"  /* DeeObject_AsUInt64M1(...) */
#define Dee_UNPd128 "I128d" /* DeeObject_AsInt128(...) */
#define Dee_UNPu128 "I128u" /* DeeObject_AsUInt128(...) */
#define Dee_UNPx128 "I128x" /* DeeObject_AsUInt128M1(...) */
#ifdef __SIZEOF_LONG_LONG__
#if __SIZEOF_LONG_LONG__ == 8
#undef Dee_UNPd64
#undef Dee_UNPu64
#undef Dee_UNPx64
#define Dee_UNPd64 "lld" /* DeeObject_AsInt64(...) */
#define Dee_UNPu64 "llu" /* DeeObject_AsUInt64(...) */
#define Dee_UNPx64 "llx" /* DeeObject_AsUInt64M1(...) */
#elif __SIZEOF_LONG_LONG__ == 4
#undef Dee_UNPd32
#undef Dee_UNPu32
#undef Dee_UNPx32
#define Dee_UNPd32 "lld" /* DeeObject_AsInt32(...) */
#define Dee_UNPu32 "llu" /* DeeObject_AsUInt32(...) */
#define Dee_UNPx32 "llx" /* DeeObject_AsUInt32M1(...) */
#elif __SIZEOF_LONG_LONG__ == 2
#undef Dee_UNPd16
#undef Dee_UNPu16
#undef Dee_UNPx16
#define Dee_UNPd16 "lld" /* DeeObject_AsInt16(...) */
#define Dee_UNPu16 "llu" /* DeeObject_AsUInt16(...) */
#define Dee_UNPx16 "llx" /* DeeObject_AsUInt16M1(...) */
#endif /* __SIZEOF_LONG_LONG__ == ... */
#endif /* __SIZEOF_LONG_LONG__ */
#if __SIZEOF_CHAR__ == 8
#undef Dee_UNPd64
#undef Dee_UNPu64
#undef Dee_UNPx64
#define Dee_UNPd64 "hhd" /* DeeObject_AsInt64(...) */
#define Dee_UNPu64 "hhu" /* DeeObject_AsUInt64(...) */
#define Dee_UNPx64 "hhx" /* DeeObject_AsUInt64M1(...) */
#elif __SIZEOF_CHAR__ == 4
#undef Dee_UNPd32
#undef Dee_UNPu32
#undef Dee_UNPx32
#define Dee_UNPd32 "hhd" /* DeeObject_AsInt32(...) */
#define Dee_UNPu32 "hhu" /* DeeObject_AsUInt32(...) */
#define Dee_UNPx32 "hhx" /* DeeObject_AsUInt32M1(...) */
#elif __SIZEOF_CHAR__ == 2
#undef Dee_UNPd16
#undef Dee_UNPu16
#undef Dee_UNPx16
#define Dee_UNPd16 "hhd" /* DeeObject_AsInt16(...) */
#define Dee_UNPu16 "hhu" /* DeeObject_AsUInt16(...) */
#define Dee_UNPx16 "hhx" /* DeeObject_AsUInt16M1(...) */
#endif /* __SIZEOF_CHAR__ == ... */
#if __SIZEOF_LONG__ == 8
#undef Dee_UNPd64
#undef Dee_UNPu64
#undef Dee_UNPx64
#define Dee_UNPd64 "ld" /* DeeObject_AsInt64(...) */
#define Dee_UNPu64 "lu" /* DeeObject_AsUInt64(...) */
#define Dee_UNPx64 "lx" /* DeeObject_AsUInt64M1(...) */
#elif __SIZEOF_LONG__ == 4
#undef Dee_UNPd32
#undef Dee_UNPu32
#undef Dee_UNPx32
#define Dee_UNPd32 "ld" /* DeeObject_AsInt32(...) */
#define Dee_UNPu32 "lu" /* DeeObject_AsUInt32(...) */
#define Dee_UNPx32 "lx" /* DeeObject_AsUInt32M1(...) */
#elif __SIZEOF_LONG__ == 2
#undef Dee_UNPd16
#undef Dee_UNPu16
#undef Dee_UNPx16
#define Dee_UNPd16 "ld" /* DeeObject_AsInt16(...) */
#define Dee_UNPu16 "lu" /* DeeObject_AsUInt16(...) */
#define Dee_UNPx16 "lx" /* DeeObject_AsUInt16M1(...) */
#elif __SIZEOF_LONG__ == 1
#undef Dee_UNPd8
#undef Dee_UNPu8
#undef Dee_UNPx8
#define Dee_UNPd8 "ld" /* DeeObject_AsInt8(...) */
#define Dee_UNPu8 "lu" /* DeeObject_AsUInt8(...) */
#define Dee_UNPx8 "lx" /* DeeObject_AsUInt8M1(...) */
#endif /* __SIZEOF_LONG__ == ... */
#if __SIZEOF_SHORT__ == 8
#undef Dee_UNPd64
#undef Dee_UNPu64
#undef Dee_UNPx64
#define Dee_UNPd64 "hd" /* DeeObject_AsInt64(...) */
#define Dee_UNPu64 "hu" /* DeeObject_AsUInt64(...) */
#define Dee_UNPx64 "hx" /* DeeObject_AsUInt64M1(...) */
#elif __SIZEOF_SHORT__ == 4
#undef Dee_UNPd32
#undef Dee_UNPu32
#undef Dee_UNPx32
#define Dee_UNPd32 "hd" /* DeeObject_AsInt32(...) */
#define Dee_UNPu32 "hu" /* DeeObject_AsUInt32(...) */
#define Dee_UNPx32 "hx" /* DeeObject_AsUInt32M1(...) */
#elif __SIZEOF_SHORT__ == 2
#undef Dee_UNPd16
#undef Dee_UNPu16
#undef Dee_UNPx16
#define Dee_UNPd16 "hd" /* DeeObject_AsInt16(...) */
#define Dee_UNPu16 "hu" /* DeeObject_AsUInt16(...) */
#define Dee_UNPx16 "hx" /* DeeObject_AsUInt16M1(...) */
#elif __SIZEOF_SHORT__ == 1
#undef Dee_UNPd8
#undef Dee_UNPu8
#undef Dee_UNPx8
#define Dee_UNPd8 "hd" /* DeeObject_AsInt8(...) */
#define Dee_UNPu8 "hu" /* DeeObject_AsUInt8(...) */
#define Dee_UNPx8 "hx" /* DeeObject_AsUInt8M1(...) */
#endif /* __SIZEOF_SHORT__ == ... */
#if __SIZEOF_INT__ == 8
#undef Dee_UNPd64
#undef Dee_UNPu64
#undef Dee_UNPx64
#define Dee_UNPd64 "d" /* DeeObject_AsInt64(...) */
#define Dee_UNPu64 "u" /* DeeObject_AsUInt64(...) */
#define Dee_UNPx64 "x" /* DeeObject_AsUInt64M1(...) */
#elif __SIZEOF_INT__ == 4
#undef Dee_UNPd32
#undef Dee_UNPu32
#undef Dee_UNPx32
#define Dee_UNPd32 "d" /* DeeObject_AsInt32(...) */
#define Dee_UNPu32 "u" /* DeeObject_AsUInt32(...) */
#define Dee_UNPx32 "x" /* DeeObject_AsUInt32M1(...) */
#elif __SIZEOF_INT__ == 2
#undef Dee_UNPd16
#undef Dee_UNPu16
#undef Dee_UNPx16
#define Dee_UNPd16 "d" /* DeeObject_AsInt16(...) */
#define Dee_UNPu16 "u" /* DeeObject_AsUInt16(...) */
#define Dee_UNPx16 "x" /* DeeObject_AsUInt16M1(...) */
#elif __SIZEOF_INT__ == 1
#undef Dee_UNPd8
#undef Dee_UNPu8
#undef Dee_UNPx8
#define Dee_UNPd8 "d" /* DeeObject_AsInt8(...) */
#define Dee_UNPu8 "u" /* DeeObject_AsUInt8(...) */
#define Dee_UNPx8 "x" /* DeeObject_AsUInt8M1(...) */
#endif /* __SIZEOF_INT__ == ... */
#define DEE_PRIVATE_UNPd1         Dee_UNPd8
#define DEE_PRIVATE_UNPu1         Dee_UNPu8
#define DEE_PRIVATE_UNPx1         Dee_UNPx8
#define DEE_PRIVATE_UNPd2         Dee_UNPd16
#define DEE_PRIVATE_UNPu2         Dee_UNPu16
#define DEE_PRIVATE_UNPx2         Dee_UNPx16
#define DEE_PRIVATE_UNPd4         Dee_UNPd32
#define DEE_PRIVATE_UNPu4         Dee_UNPu32
#define DEE_PRIVATE_UNPx4         Dee_UNPx32
#define DEE_PRIVATE_UNPd8         Dee_UNPd64
#define DEE_PRIVATE_UNPu8         Dee_UNPu64
#define DEE_PRIVATE_UNPx8         Dee_UNPx64
#define DEE_PRIVATE_UNPd16        Dee_UNPd128
#define DEE_PRIVATE_UNPu16        Dee_UNPu128
#define DEE_PRIVATE_UNPx16        Dee_UNPx128
#define DEE_PRIVATE_UNPdN(sizeof) DEE_PRIVATE_UNPd##sizeof
#define DEE_PRIVATE_UNPuN(sizeof) DEE_PRIVATE_UNPu##sizeof
#define DEE_PRIVATE_UNPxN(sizeof) DEE_PRIVATE_UNPx##sizeof
#define Dee_UNPdN(sizeof)         DEE_PRIVATE_UNPdN(sizeof) /* DeeObject_AsXInt(sizeof, ...) */
#define Dee_UNPuN(sizeof)         DEE_PRIVATE_UNPuN(sizeof) /* DeeObject_AsXUInt(sizeof, ...) */
#define Dee_UNPxN(sizeof)         DEE_PRIVATE_UNPxN(sizeof) /* DeeObject_AsXUIntM1(sizeof, ...) */

/* Helpful aliases */
#define Dee_UNPdSIZ Dee_UNPdN(__SIZEOF_SIZE_T__)  /* DeeObject_AsSSize(...) */
#define Dee_UNPuSIZ Dee_UNPuN(__SIZEOF_SIZE_T__)  /* DeeObject_AsSize(...) */
#define Dee_UNPxSIZ Dee_UNPxN(__SIZEOF_SIZE_T__)  /* DeeObject_AsSizeM1(...) */
#define Dee_UNPdPTR Dee_UNPdN(__SIZEOF_POINTER__) /* DeeObject_AsIntptr(...) */
#define Dee_UNPuPTR Dee_UNPuN(__SIZEOF_POINTER__) /* DeeObject_AsUIntptr(...) */
#define Dee_UNPxPTR Dee_UNPxN(__SIZEOF_POINTER__) /* DeeObject_AsUIntptrM1(...) */
#define Dee_UNPdB   Dee_UNPd8                     /* DeeObject_AsSByte(...) */
#define Dee_UNPuB   Dee_UNPu8                     /* DeeObject_AsByte(...) */
#define Dee_UNPxB   Dee_UNPx8                     /* DeeObject_AsByteM1(...) */

/* Unescaped names. */
#ifdef DEE_SOURCE
#define UNPdB   Dee_UNPdB   /* DeeObject_AsSByte(...) */
#define UNPuB   Dee_UNPuB   /* DeeObject_AsByte(...) */
#define UNPxB   Dee_UNPxB   /* DeeObject_AsByteM1(...) */
#define UNPd8   Dee_UNPd8   /* DeeObject_AsInt8(...) */
#define UNPu8   Dee_UNPu8   /* DeeObject_AsUInt8(...) */
#define UNPx8   Dee_UNPx8   /* DeeObject_AsUInt8M1(...) */
#define UNPd16  Dee_UNPd16  /* DeeObject_AsInt16(...) */
#define UNPu16  Dee_UNPu16  /* DeeObject_AsUInt16(...) */
#define UNPx16  Dee_UNPx16  /* DeeObject_AsUInt16M1(...) */
#define UNPd32  Dee_UNPd32  /* DeeObject_AsInt32(...) */
#define UNPu32  Dee_UNPu32  /* DeeObject_AsUInt32(...) */
#define UNPx32  Dee_UNPx32  /* DeeObject_AsUInt32M1(...) */
#define UNPd64  Dee_UNPd64  /* DeeObject_AsInt64(...) */
#define UNPu64  Dee_UNPu64  /* DeeObject_AsUInt64(...) */
#define UNPx64  Dee_UNPx64  /* DeeObject_AsUInt64M1(...) */
#define UNPd128 Dee_UNPd128 /* DeeObject_AsInt128(...) */
#define UNPu128 Dee_UNPu128 /* DeeObject_AsUInt128(...) */
#define UNPx128 Dee_UNPx128 /* DeeObject_AsUInt128M1(...) */
#define UNPdN   Dee_UNPdN   /* DeeObject_AsXInt(sizeof, ...) */
#define UNPuN   Dee_UNPuN   /* DeeObject_AsXUInt(sizeof, ...) */
#define UNPxN   Dee_UNPxN   /* DeeObject_AsXUIntM1(sizeof, ...) */
#define UNPdSIZ Dee_UNPdSIZ /* DeeObject_AsSSize(...) */
#define UNPuSIZ Dee_UNPuSIZ /* DeeObject_AsSize(...) */
#define UNPxSIZ Dee_UNPxSIZ /* DeeObject_AsSizeM1(...) */
#define UNPdPTR Dee_UNPdPTR /* DeeObject_AsIntptr(...) */
#define UNPuPTR Dee_UNPuPTR /* DeeObject_AsUIntptr(...) */
#define UNPxPTR Dee_UNPxPTR /* DeeObject_AsUIntptrM1(...) */
#endif /* DEE_SOURCE */

DECL_END

#endif /* !GUARD_DEEMON_ARG_H */
