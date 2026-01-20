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
#ifndef GUARD_DEEMON_RUNTIME_BUILD_VALUE_C
#define GUARD_DEEMON_RUNTIME_BUILD_VALUE_C 1

#include <deemon/api.h>

#include <deemon/bool.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* strnlen() */
#include <deemon/tuple.h>

#include <hybrid/__va_size.h> /* __VA_SIZE */
#include <hybrid/int128.h>
#include <hybrid/typecore.h>  /* __ALIGNOF_DOUBLE__, __ALIGNOF_FLOAT__, __ALIGNOF_INT128__, __ALIGNOF_INTn__, __ALIGNOF_LONG_DOUBLE__, __BYTE_TYPE__, __SIZEOF_INT__, __SIZEOF_LONG_LONG__, __SIZEOF_SIZE_T__ */

#include "runtime_error.h"

#include <stdarg.h> /* va_arg, va_end, va_list, va_start */
#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* intN_t, uintN_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */

#ifdef CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY
#define VALIST_ADDR(x) (&(x))
#else /* CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */
#define VALIST_ADDR(x) (&(x)[0])
#endif /* !CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
count_pack_args(char const *__restrict format) {
	size_t result = 0;
	for (;;) {
		char ch = *format++;
		switch (ch) {

			/* Length modifiers / ignored. */
		case ',':
		case 'h':
		case 'l':
		case 'I':
		case 'L':
		case '.':
		case '*':
		case '?':
		case '$':
			break;

		case 'n':
		case '-': /* none */
		case 'o':
		case 'O': /* Object */
		case 'f':
		case 'D': /* float */
		case 'd':
		case 'i': /* signed int */
		case 'u':
		case 'x': /* unsigned int */
		case 'b': /* bool */
		case 's': /* string */
			++result;
			break;

		case '[':
		case '(':
		case '{':
		case '<': {
			char tagr;
			unsigned int recursion;
			recursion = 1;
			tagr      = ch + 2;
			if (ch == '(')
				--tagr;
			for (; *format; ++format) {
				if (*format == ch) {
					++recursion;
				} else if (*format == tagr) {
					if (!--recursion) {
						++format;
						break;
					}
				} else if (!*format) {
					break;
				}
			}
			++result;
		}	break;

		case '\0':
			goto done;

		default:
			if (ch >= '0' && ch <= '9')
				break;
			goto done;
		}
	}
done:
	return result;
}



PUBLIC NONNULL((1)) void DCALL
Dee_VPPackf_Cleanup(char const *__restrict format, va_list args) {
again:
	switch (*format++) {

	case '(':
	case '[':
	case '{':
	case '<': /* Recursion (Can be ignored here) */
	case ',': /* Separator. */
	case 'n': /* `none' */
	case '-':
		goto again;

	case 'o':
		(void)va_arg(args, DeeObject *);
		goto again;

	case 'L':
		ASSERTF(*format == 'D', "Invalid format: `%s'", format);
		(void)va_arg(args, long double);
		goto again;

	case 'f':
	case 'D':
		(void)va_arg(args, double); /* NOTE: C promotes `float' to double in varargs. */
		goto again;

	case '.':
		if (*format == '*') {
			(void)va_arg(args, unsigned int);
			++format;
		} else if (*format == '?') {
		case '$':
			(void)va_arg(args, size_t);
			++format;
		} else {
			while (*format >= '0' && *format <= '9')
				++format;
		}
		ASSERTF(*format == 's', "Invalid format: `%s'", format);
		goto again;

	{
		unsigned int length;
	case 'l':
		length = sizeof(long);
		if (*format == 'l') {
#ifdef __SIZEOF_LONG_LONG__
			length = __SIZEOF_LONG_LONG__;
#else /* __SIZEOF_LONG_LONG__ */
			length = 8;
#endif /* !__SIZEOF_LONG_LONG__ */
			++format;
		}
		++format; /* Consume integer-format-char */
		goto do_int;
	case 'h':
		length = sizeof(short);
		if (*format == 'h') {
			length = sizeof(char);
			++format;
		}
		++format; /* Consume integer-format-char */
		goto do_int;
	case 'I':
		length = sizeof(size_t);
		if (*format == '8') {
			length = 1;
			format += 1;
		} else if (*format == '1') {
			if (format[2] == '2') {
				ASSERTF(format[3] == '8', "Invalid format: `%s'", format);
				length = 16;
				format += 3;
			} else {
				ASSERTF(format[1] == '6', "Invalid format: `%s'", format);
				length = 2;
				format += 2;
			}
		} else if (*format == '3') {
			ASSERTF(format[1] == '2', "Invalid format: `%s'", format);
			length = 4;
			format += 2;
		} else if (*format == '6') {
			ASSERTF(format[1] == '4', "Invalid format: `%s'", format);
			length = 8;
			format += 2;
		}
		++format; /* Consume integer-format-char */
		goto do_int;
	case 'd':
	case 'u':
	case 'i':
	case 'x':
	case 'b':
		length = sizeof(int);
do_int:
		ASSERTF(format[-1] == 'd' || *format == 'u' ||
			    format[-1] == 'i' || *format == 'x' ||
			    format[-1] == 'b',
			    "Invalid format: `%s'", format);
#if __VA_SIZE < 2
		if (length <= 1) {
			(void)va_arg(args, uint8_t);
		} else
#endif /* __VA_SIZE < 2 */
#if __VA_SIZE < 4
		if (length <= 2) {
			(void)va_arg(args, uint16_t);
		} else
#endif /* __VA_SIZE < 4 */
#if __VA_SIZE < 8
		if (length <= 4) {
			(void)va_arg(args, uint32_t);
		} else
#endif /* __VA_SIZE < 8 */
#if __VA_SIZE < 16
		if (length <= 8) {
			(void)va_arg(args, uint64_t);
		} else
#endif /* __VA_SIZE < 16 */
		{
			(void)va_arg(args, Dee_uint128_t);
		}
	}	goto again;

	case 'O': {
		DeeObject *temp;
		/* _Always_ inherit reference to `O' operands. */
		temp = va_arg(args, DeeObject *);
		Dee_XDecref(temp);
		goto again;
	}

	default:
		ASSERTF(!*format, "Invalid format: `%s'", format);
		break;
	}
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_VPPackf(char const **__restrict p_format,
            struct va_list_struct *__restrict p_args) {
	DREF DeeObject *result;
	char const *format = *p_format;
again:
	switch (*format++) {

	case ',':
		goto again;

	case '\0':
		--format;
		ATTR_FALLTHROUGH
	case 'n':
	case '-':
		result = Dee_None;
		Dee_Incref(result);
		break;

	case 'o':
	case 'O':
		result = va_arg(p_args->vl_ap, DeeObject *);
		if (format[-1] == 'o') {
			ASSERTF(result, "NULL-object passed to `o' operand");
			Dee_Incref(result);
		}
		break;

	{
		int length;
		union {
			int32_t i32;
			uint32_t u32;
			int64_t i64;
			uint64_t u64;
		} data;
	case 'h':
		length = sizeof(short);
		if (*format == 'h') {
			++format;
			length = sizeof(char);
		}
		++format; /* Consume integer-format-char */
		goto has_length;

	case 'l':
		length = sizeof(long);
#ifdef __SIZEOF_LONG_LONG__
		if (*format == 'l') {
			++format;
			length = __SIZEOF_LONG_LONG__;
		}
#else /* __SIZEOF_LONG_LONG__ */
		if (*format == 'l') {
			++format;
			length = 8;
		}
#endif /* !__SIZEOF_LONG_LONG__ */
		++format; /* Consume integer-format-char */
		goto has_length;

	case 'I':
		length = sizeof(size_t);
		if (*format == '8') {
			++format;
			length = 1;
		} else if (*format == '1') {
			if (format[2] == '2') {
				union {
					Dee_int128_t  s;
					Dee_uint128_t u;
				} data128;
				ASSERTF(format[3] == '8', "Invalid format: `%s'", format);
				data128.u = va_arg(p_args->vl_ap, Dee_uint128_t);
				format += 3;
				ASSERTF(format[-1] == 'd' || format[-1] == 'u' ||
				        format[-1] == 'i' || format[-1] == 'x' ||
				        format[-1] == 'b',
				        "Invalid format: `%s'", format);
				if (format[-1] == 'b') {
					/* Boolean. */
					result = DeeBool_For(!__hybrid_uint128_iszero(data128.u));
					Dee_Incref(result);
				} else if (format[-1] == 'd' || format[-1] == 'i') {
					/* Signed integer. */
					result = DeeInt_NewInt128(data128.s);
				} else {
					/* Unsigned integer. */
					result = DeeInt_NewUInt128(data128.u);
				}
				break;
			} else {
				ASSERTF(format[1] == '6', "Invalid format: `%s' (`%s')", format, *p_format);
				format += 2;
				length = 2;
			}
		} else if (*format == '3') {
			ASSERTF(format[1] == '2', "Invalid format: `%s' (`%s')", format, *p_format);
			format += 2;
			length = 4;
		} else if (*format == '6') {
			ASSERTF(format[1] == '4', "Invalid format: `%s' (`%s')", format, *p_format);
			format += 2;
			length = 8;
		}
		++format; /* Consume integer-format-char */
		goto has_length;
	case 'd':
	case 'i':
	case 'u':
	case 'x':
	case 'b':
		length = sizeof(int);
has_length:
		ASSERTF(format[-1] == 'd' || format[-1] == 'u' ||
		        format[-1] == 'i' || format[-1] == 'x' ||
		        format[-1] == 'b',
		        "Invalid format: `%s'", format);
#if __VA_SIZE < 2
		if (length <= 1) {
			data.u32 = (uint32_t)va_arg(p_args->vl_ap, uint8_t);
		} else
#endif /* __VA_SIZE < 2 */
#if __VA_SIZE < 4
		if (length <= 2) {
			data.u32 = (uint32_t)va_arg(p_args->vl_ap, uint16_t);
		} else
#endif /* __VA_SIZE < 4 */
#if __VA_SIZE < 8
		if (length <= 4) {
			data.u32 = va_arg(p_args->vl_ap, uint32_t);
		} else
#endif /* __VA_SIZE < 8 */
		{
			data.u64 = va_arg(p_args->vl_ap, uint64_t);
		}
		if (format[-1] == 'b') {
			/* Boolean. */
			result = DeeBool_For(length > 4 ? (data.u64 != 0) : (data.u32 != 0));
			Dee_Incref(result);
		} else if (format[-1] == 'd' || format[-1] == 'i') {
			/* Signed integer. */
			if (length > 4) {
				result = DeeInt_NewInt64(data.i64);
			} else {
				result = DeeInt_NewInt32(data.i32);
			}
		} else {
			/* Unsigned integer. */
			if (length > 4) {
				result = DeeInt_NewUInt64(data.u64);
			} else {
				result = DeeInt_NewUInt32(data.u32);
			}
		}
	}	break;

	{
		size_t string_length;
		char const *string;
	case '$':
		ASSERTF(*format == 's', "Invalid format: `%s' (`%s')", format, *p_format);
		++format;
		string_length = va_arg(p_args->vl_ap, size_t);
		string        = va_arg(p_args->vl_ap, char const *);
		goto do_string;
	case 's':
		string        = va_arg(p_args->vl_ap, char const *);
		string_length = strlen(string);
		goto do_string;
	case '.':
#if (!defined(__SIZEOF_INT__) || !defined(__SIZEOF_SIZE_T__) || \
     (__SIZEOF_INT__ != __SIZEOF_SIZE_T__))
		if (*format == '*') {
			string_length = (size_t)va_arg(p_args->vl_ap, unsigned int);
			goto do_strnlen;
		} else if (*format == '?')
#else /* ... */
		if (*format == '*' || *format == '?')
#endif /* !... */
		{
			string_length = va_arg(p_args->vl_ap, size_t);
do_strnlen:
			string        = va_arg(p_args->vl_ap, char const *);
			string_length = strnlen(string, string_length);
		} else {
			ASSERTF(*format >= '0' && *format <= '9',
				    "Invalid format: `%s' (`%s')", format, *p_format);
			string_length = 0;
			do {
				string_length = (string_length * 10) + (*format - '0');
			} while ((++format, *format >= '0' && *format <= '9'));
			goto do_strnlen;
		}
do_string:
		ASSERTF(format[-1] == 's', "Invalid format: `%s' (`%s')", format, *p_format);
		/* TODO: `%I8s' -- latin-1 string */
		/* TODO: `%I16s' -- 2-byte string */
		/* TODO: `%I32s' -- utf-32 string */
		result = DeeString_NewUtf8(string, string_length, STRING_ERROR_FIGNORE);
	}	break;


	case '(': {
		size_t i, num_args;
		num_args = count_pack_args(format);
		result   = Dee_AsObject(DeeTuple_NewUninitialized(num_args));
		if unlikely(!result)
			break;
		for (i = 0; i < num_args; ++i) {
			DREF DeeObject *elem;
			elem = Dee_VPPackf(&format, p_args);
			if unlikely(!elem) {
				/* Propagate an error. */
				Dee_Decrefv(DeeTuple_ELEM(result), i);
				DeeTuple_FreeUninitialized((DREF DeeTupleObject *)result);
				result = NULL;
				goto end;
			}
			DeeTuple_SET(result, i, elem);
		}
		ASSERTF(*format == ')', "Invalid format: `%s' (`%s')", format, *p_format);
		++format;
	}	break;

		/* TODO: Tuple, Set, Cell */
		/* TODO: float */

	default:
		ASSERTF(0, "Invalid format: `%s' (`%s')", format, *p_format);
		__builtin_unreachable();
		break;
	}
end:
	*p_format = format;
	return result;
}



PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeTuple_VNewf(char const *__restrict format, va_list args) {
	struct va_list_struct *p_args;
	DREF DeeTupleObject *result;
	size_t i, tuple_size = count_pack_args(format);
	p_args = (struct va_list_struct *)VALIST_ADDR(args);
	result = DeeTuple_NewUninitialized(tuple_size);
	if unlikely(!result)
		goto err;
	for (i = 0; i < tuple_size; ++i) {
		DREF DeeObject *elem;
		elem = Dee_VPPackf((char const **)&format, p_args);
		if unlikely(!elem)
			goto err_r;
		DeeTuple_SET(result, i, elem);
	}
	ASSERTF(!*format, "Invalid format: `%s'", format);
	return Dee_AsObject(result);
err_r:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
	Dee_VPPackf_Cleanup(format, p_args->vl_ap);
err:
	return NULL;
}


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_VPackf(char const *__restrict format, va_list args) {
	DREF DeeObject *result;
	result = Dee_VPPackf((char const **)&format, (struct va_list_struct *)VALIST_ADDR(args));
	if unlikely(!result) {
		Dee_VPPackf_Cleanup(format, ((struct va_list_struct *)VALIST_ADDR(args))->vl_ap);
	} else if unlikely(*format != '\0') {
		/* More objects at the end -> cannot pack */
		Dee_VPPackf_Cleanup(format, ((struct va_list_struct *)VALIST_ADDR(args))->vl_ap);
		Dee_Decref(result);
		err_invalid_argc(NULL, 2, 1, 1); /* TODO: 2 is just a guess; it could also be more... */
		result = NULL;
	}
	return result;
}


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
 *     ref_str    ::= [ '.<int>'  // str = va_arg(char const *); DeeString_NewSized(str, MIN(strlen(str), <int>));
 *                    | '.*'      // max = va_arg(unsigned int); str = va_arg(char const *); DeeString_NewSized(str, MIN(strlen(str), max));
 *                    | '.?'      // max = va_arg(size_t); str = va_arg(char const *); DeeString_NewSized(str, MIN(strlen(str), max));
 *                    | '$'       // len = va_arg(size_t); str = va_arg(char const *); DeeString_NewSized(str, len);
 *                    ] 's';      // DeeString_New(va_arg(char const *));
 *
 * Example:
 * >> Dee_Packf("(d<d>Os)", 10, 20, DeeInt_NewInt(42), "foobar");
 * >> // (10, <20>, 42, "foobar")
 */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
Dee_Packf(char const *__restrict format, ...) {
	struct va_list_struct args;
	DREF DeeObject *result;
	va_start(args.vl_ap, format);
	result = Dee_VPPackf((char const **)&format, &args);
	va_end(args.vl_ap);
	if unlikely(!result)
		Dee_VPPackf_Cleanup(format, args.vl_ap);
	return result;
}

/* Similar to `Dee_Packf', but parse any number of formated values and
 * put them in a tuple, essentially doing the same as `Dee_Packf' when
 * the entire `format' string was surrounded by `(' and `)'. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
DeeTuple_Newf(char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeTuple_VNewf(format, args);
	va_end(args);
	return result;
}


/* Assert that our alignment constants are correct. If these were to
 * fail, then the `Dee_UnpackStruct()' would produce incorrect results. */
#define ASSERT_ALIGNMENT(T, expected)                      \
	struct _check_alignof_##T { __BYTE_TYPE__ _v; T _x; }; \
	STATIC_ASSERT(offsetof(struct _check_alignof_##T, _x) == (expected))
ASSERT_ALIGNMENT(int8_t, __ALIGNOF_INT8__);
ASSERT_ALIGNMENT(int16_t, __ALIGNOF_INT16__);
ASSERT_ALIGNMENT(int32_t, __ALIGNOF_INT32__);
ASSERT_ALIGNMENT(int64_t, __ALIGNOF_INT64__);
ASSERT_ALIGNMENT(Dee_int128_t, __ALIGNOF_INT128__);
ASSERT_ALIGNMENT(uint8_t, __ALIGNOF_INT8__);
ASSERT_ALIGNMENT(uint16_t, __ALIGNOF_INT16__);
ASSERT_ALIGNMENT(uint32_t, __ALIGNOF_INT32__);
ASSERT_ALIGNMENT(uint64_t, __ALIGNOF_INT64__);
ASSERT_ALIGNMENT(Dee_uint128_t, __ALIGNOF_INT128__);
ASSERT_ALIGNMENT(float, __ALIGNOF_FLOAT__);
ASSERT_ALIGNMENT(double, __ALIGNOF_DOUBLE__);
#ifdef __LONGDOUBLE
ASSERT_ALIGNMENT(__LONGDOUBLE, __ALIGNOF_LONG_DOUBLE__);
#endif /* __LONGDOUBLE */


/* Helper functions for throwing invalid-argc errors */
PUBLIC ATTR_COLD int
(DCALL DeeArg_BadArgc)(char const *function_name, size_t real_argc, size_t want_argc) {
	return err_invalid_argc(function_name, real_argc, want_argc, want_argc);
}

PUBLIC ATTR_COLD int
(DCALL DeeArg_BadArgc0)(char const *function_name, size_t real_argc) {
	return err_invalid_argc(function_name, real_argc, 0, 0);
}

PUBLIC ATTR_COLD int
(DCALL DeeArg_BadArgc1)(char const *function_name, size_t real_argc) {
	return err_invalid_argc(function_name, real_argc, 1, 1);
}

PUBLIC ATTR_COLD int
(DCALL DeeArg_BadArgcEx)(char const *function_name, size_t real_argc, size_t want_argc_min, size_t want_argc_max) {
	return err_invalid_argc(function_name, real_argc, want_argc_min, want_argc_max);
}

DECL_END

#ifndef __INTELLISENSE__
#define DEFINE_DeeArg_Unpack
#include "build-value-unpack.c.inl"
#define DEFINE_DeeArg_UnpackStruct
#include "build-value-unpack.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_RUNTIME_BUILD_VALUE_C */
