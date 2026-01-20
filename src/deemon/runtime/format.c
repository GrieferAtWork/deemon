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
#ifndef GUARD_DEEMON_RUNTIME_FORMAT_C
#define GUARD_DEEMON_RUNTIME_FORMAT_C 1

#include <deemon/api.h>

#include <deemon/bytes.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/kwds.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* strnlen(), memcpyc(), ... */
#include <deemon/variant.h>

#include <hybrid/int128.h>
#include <hybrid/minmax.h>   /* MIN */
#include <hybrid/typecore.h> /* __BYTE_TYPE__, __SIZEOF_CHAR__, __SIZEOF_INT__, __SIZEOF_LONG_LONG__, __SIZEOF_LONG__, __SIZEOF_POINTER__, __SIZEOF_SHORT__, __SIZEOF_WCHAR_T__ */

#include <stdarg.h>  /* va_arg, va_end, va_list, va_start */
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* UINT16_C, UINT32_C, intN_t, uintN_t */

#ifndef PP_CAT2
#define PP_PRIVATE_CAT2(a, b)    a##b
#define PP_PRIVATE_CAT3(a, b, c) a##b##c
#define PP_CAT2(a, b)            PP_PRIVATE_CAT2(a, b)
#define PP_CAT3(a, b, c)         PP_PRIVATE_CAT3(a, b, c)
#endif /* !PP_CAT2 */

#ifndef PP_MUL8
#define PP_PRIVATE_MUL8_0 0
#define PP_PRIVATE_MUL8_1 8
#define PP_PRIVATE_MUL8_2 16
#define PP_PRIVATE_MUL8_4 32
#define PP_PRIVATE_MUL8_8 64
#define PP_PRIVATE_MUL8(x) PP_PRIVATE_MUL8_##x
#define PP_MUL8(x)         PP_PRIVATE_MUL8(x)
#endif /* !PP_MUL8 */

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) \
	typedef int PP_CAT2(static_assert_, __LINE__)[(expr) ? 1 : -1]
#endif /* !STATIC_ASSERT */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */

STATIC_ASSERT(sizeof(int) == __SIZEOF_INT__);
STATIC_ASSERT(sizeof(void *) == __SIZEOF_POINTER__);
STATIC_ASSERT(sizeof(size_t) == __SIZEOF_POINTER__);
STATIC_ASSERT(sizeof(char) == __SIZEOF_CHAR__);
STATIC_ASSERT(sizeof(short) == __SIZEOF_SHORT__);
STATIC_ASSERT(sizeof(long) == __SIZEOF_LONG__);


#define VA_SIZE __SIZEOF_INT__


#if VA_SIZE >= 8
#define LENGTH_I128 0x51
#define LENGTH_I64  0x40
#define LENGTH_I32  0x30
#define LENGTH_I16  0x20
#define LENGTH_I8   0x10
#elif VA_SIZE >= 4
#define LENGTH_I128 0x52
#define LENGTH_I64  0x41
#define LENGTH_I32  0x30
#define LENGTH_I16  0x20
#define LENGTH_I8   0x10
#elif VA_SIZE >= 2
#define LENGTH_I128 0x53
#define LENGTH_I64  0x42
#define LENGTH_I32  0x31
#define LENGTH_I16  0x20
#define LENGTH_I8   0x10
#elif VA_SIZE >= 1
#define LENGTH_I128 0x54
#define LENGTH_I64  0x43
#define LENGTH_I32  0x32
#define LENGTH_I16  0x21
#define LENGTH_I8   0x10
#else /* VA_SIZE >= ... */
#error "Error: Unsupported `VA_SIZE'"
#endif /* VA_SIZE < ... */

#define LENGTH_VASIZEOF(x) ((x)&0xf)
#define LENGTH_IXSIZEOF(x) ((x)&0xf0)


#define LENGTH_L    'L'
#define LENGTH_Z    'z'
#define LENGTH_T    't'
#define LENGTH_J    LENGTH_I64 /* intmax_t */
#define LENGTH_SIZE PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_POINTER__))
#define LENGTH_HH   PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_CHAR__))
#define LENGTH_H    PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_SHORT__))
#define LENGTH_l    (PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_LONG__)) | 0x100)
#ifdef __SIZEOF_LONG_LONG__
#define LENGTH_LL   PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_LONG_LONG__))
#else /* __SIZEOF_LONG_LONG__ */
#define LENGTH_LL   LENGTH_I64
#endif /* !__SIZEOF_LONG_LONG__ */



#define F_NONE     0x0000
#define F_UPPER    0x0001 /* Print upper-case hex-characters. */
#define F_LJUST    0x0002 /* '%-'. */
#define F_SIGN     0x0004 /* '%+'. */
#define F_SPACE    0x0008 /* '% '. */
#define F_PADZERO  0x0010 /* '%0'. */
#define F_HASWIDTH 0x0020 /* '%123'. */
#define F_HASPREC  0x0040 /* `%.123'. */
#define F_PREFIX   0x0080 /* `%#'. */
#define F_SIGNED   0x0100
#define F_FIXBUF   0x0200

#define DO(expr)                         \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0
#define print(p, s) \
	DO((*printer)(arg, p, s))

PRIVATE NONNULL((1)) void DCALL
format_cleanup(char const *__restrict format, va_list args) {
	int length;
	char ch;
	char const *flush_start;
next:
	ch = *format++;
	if unlikely(!ch)
		goto end;
	if (ch != '%')
		goto next;
	flush_start = format;
	length      = 0;
nextfmt:
	ch = *format++;
	switch (ch) {

	case '\0':
		goto end;

	case '-':
	case '+':
	case ' ':
	case '#':
	case '0':
		goto nextfmt;

	case '?':
#if __SIZEOF_POINTER__ > VA_SIZE
		(void)va_arg(args, size_t);
		goto nextfmt;
#endif /* __SIZEOF_POINTER__ > VA_SIZE */
	case '*':
		(void)va_arg(args, unsigned int);
		goto nextfmt;

	case '.': /* Precision. */
	case ':':
		ch = *format++;
#if __SIZEOF_POINTER__ > VA_SIZE
		if (ch == '*') {
			(void)va_arg(args, unsigned int);
		} else if (ch == '?')
#else /* __SIZEOF_POINTER__ > VA_SIZE */
		if (ch == '*' || ch == '?')
#endif /* __SIZEOF_POINTER__ <= VA_SIZE */
		{
	case '$':
			(void)va_arg(args, size_t);
		} else if (ch >= '0' && ch <= '9') {
			while ((ch = *format, ch >= '0' && ch <= '9'))
				++format;
		} else {
			goto broken_format;
		}
		goto nextfmt;

	case 'h':
		if (*format != 'h') {
			length = LENGTH_VASIZEOF(LENGTH_H);
		} else {
			++format;
			length = LENGTH_VASIZEOF(LENGTH_HH);
		}
		goto nextfmt;

	case 'l':
		if (*format != 'l') {
			length = LENGTH_VASIZEOF(LENGTH_l);
		} else {
			++format;
			length = LENGTH_VASIZEOF(LENGTH_LL);
		}
		goto nextfmt;

	case 'z':
	case 't':
	case 'L':
		length = (unsigned int)ch;
		goto nextfmt;

	case 'I':
		ch = *format++;
		if (ch == '8') {
			length = LENGTH_VASIZEOF(LENGTH_I8);
		} else if (ch == '1' && *format == '6') {
			length = LENGTH_VASIZEOF(LENGTH_I16);
		} else if (ch == '3' && *format == '2') {
			length = LENGTH_VASIZEOF(LENGTH_I32);
		} else if (ch == '6' && *format == '4') {
	case 'j':
			length = LENGTH_VASIZEOF(LENGTH_I64);
		} else {
			--format;
			length = LENGTH_VASIZEOF(LENGTH_SIZE);
		}
		goto nextfmt;

	case 'p':
#if __SIZEOF_POINTER__ > VA_SIZE
		if (!length)
			length = LENGTH_VASIZEOF(LENGTH_SIZE);
		ATTR_FALLTHROUGH
#endif /* __SIZEOF_POINTER__ > VA_SIZE */
	case 'b':
	case 'o':
	case 'u':
	case 'd':
	case 'i':
#if VA_SIZE < 2
		if (length == LENGTH_VASIZEOF(LENGTH_I8)) {
			(void)va_arg(args, uint8_t);
		} else
#endif /* VA_SIZE < 2 */
#if VA_SIZE < 4
		if (length == LENGTH_VASIZEOF(LENGTH_I16)) {
			(void)va_arg(args, uint16_t);
		} else
#endif /* VA_SIZE < 4 */
#if VA_SIZE < 8
		if (length == LENGTH_VASIZEOF(LENGTH_I32)) {
			(void)va_arg(args, uint32_t);
		} else
#endif /* VA_SIZE < 8 */
		{
			(void)va_arg(args, uint64_t);
		}
		goto next;

	case 'c':
		(void)va_arg(args, int);
		goto next;

	case 'q':
	case 's':
	case 'k':
	case 'r':
		(void)va_arg(args, char const *);
		goto next;

	case 'f':
	case 'F':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
#ifdef __COMPILER_HAVE_LONGDOUBLE
		if (length == LENGTH_L) {
			(void)va_arg(args, long double);
		} else
#endif /* __COMPILER_HAVE_LONGDOUBLE */
		{
			(void)va_arg(args, double);
		}
		goto next;


	case 'K':
	case 'R': {
		DeeObject *ob;
		ob = va_arg(args, DeeObject *);
		Dee_XDecref(ob);
	}	goto next;

	default:
		if (ch >= '0' && ch <= '9') {
			while ((ch = *format, ch >= '0' && ch <= '9'))
				++format;
			goto nextfmt;
		}
broken_format:
		format = flush_start;
		goto next;
	}
end:
	ASSERT(!format[-1]);
}


PRIVATE uint8_t const null8[] = { '(', 'n', 'u', 'l', 'l', ')', 0 };
PRIVATE uint16_t const null16[] = { '(', 'n', 'u', 'l', 'l', ')', 0 };
PRIVATE uint32_t const null32[] = { '(', 'n', 'u', 'l', 'l', ')', 0 };
PRIVATE char const dquote[] = { '"' };

LOCAL WUNUSED NONNULL((1)) size_t DCALL
strnlen16(uint16_t const *__restrict str, size_t maxlen) {
	uint16_t const *end = str;
	for (; maxlen && *end; --maxlen, ++end)
		;
	return (size_t)(end - str);
}

LOCAL WUNUSED NONNULL((1)) size_t DCALL
strnlen32(uint32_t const *__restrict str, size_t maxlen) {
	uint32_t const *end = str;
	for (; maxlen && *end; --maxlen, ++end)
		;
	return (size_t)(end - str);
}

typedef union {
	Dee_uint128_t u;
	Dee_int128_t i;
} Xint128_t;

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
print_int128(Dee_formatprinter_t printer, void *arg,
             Xint128_t *value, size_t precision,
             unsigned int flags, unsigned int numsys) {
	Dee_ssize_t result = 0, temp;
	char const *dec;
	bool is_neg;
	char *iter, buffer[131]; /* 128-bit binary, w/prefix + sign */
	dec    = DeeAscii_ItoaDigits(flags & F_UPPER);
	is_neg = false;
	if ((flags & F_SIGNED) && __hybrid_int128_isneg(value->i)) {
		is_neg = true;
		__hybrid_int128_neg(value->i);
	}
	iter = COMPILER_ENDOF(buffer);
	/* Actually translate the given input integer. */
	do {
		Dee_uint128_t digit;
		digit = value->u;
		__hybrid_uint128_mod8(digit, numsys);
		__hybrid_uint128_div8(value->u, numsys);
		*--iter = dec[__hybrid_uint128_get8(digit)];
	} while (!__hybrid_uint128_iszero(value->u));
	if (flags & F_PREFIX && numsys != 10) {
		if (numsys == 16) {
			*--iter = dec[33]; /* X/x */
		} else if (numsys == 2) {
			*--iter = dec[11]; /* B/b */
		}
		*--iter = '0';
	}
	if (is_neg) {
		*--iter = '-';
	} else if (flags & F_SIGN) {
		*--iter = '+';
	}
	for (;;) {
		size_t bufsize = (size_t)(COMPILER_ENDOF(buffer) - iter);
		if ((flags & F_HASPREC) && precision > bufsize) {
			size_t precbufsize = COMPILER_LENOF(buffer) - bufsize;
			precision -= bufsize;
			if (precbufsize > precision)
				precbufsize = precision;
			ASSERT(precbufsize != 0);
			bufsize += precbufsize;
			iter -= precbufsize;
			memset(iter, '0', precbufsize);
			ASSERT(precbufsize <= precision);
			precision -= precbufsize;
		}
		print(iter, bufsize);
		if (precision <= bufsize)
			break;
		ASSERT(flags & F_HASPREC);
		iter = COMPILER_ENDOF(buffer);
	}
	return result;
err:
	return temp;
}


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
 *   - `%k': Taking a `DeeObject *', print `__str__' or `none' when `NULL' was passed.
 *   - `%r': Taking a `DeeObject *', print `__repr__' or `none' when `NULL' was passed.
 *   - `%K': Same as `%k', but decref() the object afterwards.
 *   - `%R': Same as `%r', but decref() the object afterwards.
 * HINT: To guaranty fulfillment of `K' and `R' operands,
 *       the format string is _always_ fully processed.
 * @return: * :  The sum of all return values from calls to `*printer'
 * @return: < 0: The first negative return value of a call to `*printer' */
PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_VPrintf(Dee_formatprinter_t printer, void *arg,
                  char const *__restrict format, va_list args) {
	Dee_ssize_t result = 0, temp;
	char const *flush_start;
	char ch;
	size_t width, precision;
	unsigned int flags;
	unsigned int length;
	flush_start = format;
next:
	ch = *format++;
	if unlikely(!ch)
		goto end;
	if (ch != '%')
		goto next;
	if (format - 1 != flush_start)
		print(flush_start, (size_t)((format - 1) - flush_start));
	flush_start = format;

	/* Format option. */
	flags     = F_NONE;
	length    = 0;
	width     = 0;
	precision = 0;
nextfmt:
	ch = *format++;
	switch (ch) {

	case '%':
		flush_start = format - 1;
		goto next;

	case '\0':
		goto end;

	case '-':
		flags |= F_LJUST;
		goto nextfmt;

	case '+':
		flags |= F_SIGN;
		goto nextfmt;

	case ' ':
		flags |= F_SPACE;
		goto nextfmt;

	case '#':
		flags |= F_PREFIX;
		goto nextfmt;

	case '0':
		flags |= F_PADZERO;
		goto nextfmt;

	case '?':
#if __SIZEOF_POINTER__ > VA_SIZE
		width = va_arg(args, size_t);
		goto nextfmt;
#endif /* __SIZEOF_POINTER__ > VA_SIZE */
	case '*':
		width = (size_t)va_arg(args, unsigned int);
		goto nextfmt;

	case ':':
		flags |= F_FIXBUF;
		ATTR_FALLTHROUGH
	case '.': /* Precision. */
		ch = *format++;
#if __SIZEOF_POINTER__ > VA_SIZE
		if (ch == '*') {
			precision = (size_t)va_arg(args, unsigned int);
		} else if (ch == '?')
#else
		if (ch == '*' || ch == '?')
#endif
		{
			__IF0 {
	case '$':
				flags |= F_FIXBUF;
			}
			precision = va_arg(args, size_t);
		} else if (ch >= '0' && ch <= '9') {
			/* decimal-encoded precision modifier. */
			precision = (size_t)(ch - '0');
			while ((ch = *format, ch >= '0' && ch <= '9')) {
				precision = precision * 10 + (size_t)(ch - '0');
				++format;
			}
		} else {
			goto broken_format;
		}
		flags |= F_HASPREC;
		goto nextfmt;

	case 'h':
		if (*format != 'h') {
			length = LENGTH_H;
		} else {
			++format;
			length = LENGTH_HH;
		}
		goto nextfmt;

	case 'l':
		if (*format != 'l') {
			length = LENGTH_l;
		} else {
			++format;
			length = LENGTH_LL;
		}
		goto nextfmt;

	case 'z':
	case 't':
	case 'L':
		length = (unsigned int)ch;
		goto nextfmt;

	case 'I':
		ch = *format++;
		if (ch == '8') {
			length = LENGTH_I8;
		} else if (ch == '1') {
			if (*format == '6') {
				length = LENGTH_I16;
				++format;
			} else if (*format == '2') {
				++format;
				ASSERT(*format == '8');
				++format;
				length = LENGTH_I128;
			}
		} else if (ch == '3' && *format == '2') {
			length = LENGTH_I32;
			++format;
		} else if (ch == '6' && *format == '4') {
			++format;
			ATTR_FALLTHROUGH
	case 'j':
			length = LENGTH_I64;
		} else {
			--format;
			length = LENGTH_SIZE;
		}
		goto nextfmt;

	{
		unsigned int numsys;
		char const *dec;
		bool is_neg;
		union {
			uint64_t u;
			int64_t i;
		} data;
		char *iter, buffer[67]; /* 64-bit binary, w/prefix + sign */
		__IF0 {
	case 'b':
			numsys = 2;
		}
		__IF0 {
	case 'o':
			numsys = 8;
		}
		__IF0 {
	case 'u':
			numsys = 10;
			if unlikely(length == LENGTH_T)
				flags |= F_SIGNED;
		}
		__IF0 {
	case 'd':
	case 'i':
			numsys = 10;
			if likely(length != LENGTH_Z)
				flags |= F_SIGNED;
		}
		__IF0 {
	case 'p':
			if (!(flags & F_HASPREC)) {
				precision = sizeof(void *) * 2;
				flags |= F_HASPREC;
			}
#if __SIZEOF_POINTER__ > VA_SIZE
			if (!length)
				length = LENGTH_SIZE;
#endif /* __SIZEOF_POINTER__ > VA_SIZE */
	case 'X':
			flags |= F_UPPER;
	case 'x':
			numsys = 16;
			if unlikely(length == LENGTH_T)
				flags |= F_SIGNED;
		}
#if VA_SIZE < 2
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I8)) {
			data.u = (uint64_t)va_arg(args, uint8_t);
			if (flags & F_SIGNED)
				data.i = (int64_t)(int8_t)(uint8_t)data.u;
		} else
#endif /* VA_SIZE < 2 */
#if VA_SIZE < 4
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I16)) {
			data.u = (uint64_t)va_arg(args, uint16_t);
			if (flags & F_SIGNED)
				data.i = (int64_t)(int16_t)(uint16_t)data.u;
		} else
#endif /* VA_SIZE < 4 */
#if VA_SIZE < 8
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I32)) {
			data.u = (uint64_t)va_arg(args, uint32_t);
			if (flags & F_SIGNED)
				data.i = (int64_t)(int32_t)(uint32_t)data.u;
		} else
#endif /* VA_SIZE < 8 */
#if VA_SIZE < 16
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I64)) {
			data.u = va_arg(args, uint64_t);
		} else
#endif /* VA_SIZE < 16 */
		{
			Xint128_t val128;
			val128.i = va_arg(args, Dee_int128_t);
			temp = print_int128(printer, arg, &val128,
			                    precision, flags, numsys);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			break;
		}

		dec    = DeeAscii_ItoaDigits(flags & F_UPPER);
		is_neg = false;
		if ((flags & F_SIGNED) && (data.i < 0)) {
			is_neg = true;
			data.i = -data.i;
		}
		iter = COMPILER_ENDOF(buffer);
		/* Actually translate the given input integer. */
		do {
			*--iter = dec[data.u % numsys];
		} while ((data.u /= numsys) != 0);
		if (flags & F_PREFIX && numsys != 10) {
			if (numsys == 16) {
				*--iter = dec[33]; /* X/x */
			} else if (numsys == 2) {
				*--iter = dec[11]; /* B/b */
			}
			*--iter = '0';
		}
		if (is_neg) {
			*--iter = '-';
		} else if (flags & F_SIGN) {
			*--iter = '+';
		}
		for (;;) {
			size_t bufsize = (size_t)(COMPILER_ENDOF(buffer) - iter);
			if ((flags & F_HASPREC) && precision > bufsize) {
				size_t precbufsize = COMPILER_LENOF(buffer) - bufsize;
				precision -= bufsize;
				if (precbufsize > precision)
					precbufsize = precision;
				ASSERT(precbufsize != 0);
				bufsize += precbufsize;
				iter -= precbufsize;
				memset(iter, '0', precbufsize);
				ASSERT(precbufsize <= precision);
				precision -= precbufsize;
			}
			print(iter, bufsize);
			if (precision <= bufsize)
				break;
			ASSERT(flags & F_HASPREC);
			iter = COMPILER_ENDOF(buffer);
		}
	}	break;

	case 'c': {
		uint32_t print_ch;
#if VA_SIZE < 2
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I8)) {
			print_ch = (uint32_t)va_arg(args, uint8_t);
		} else
#endif /* VA_SIZE < 2 */
#if VA_SIZE < 4
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I16)) {
			print_ch = (uint32_t)va_arg(args, uint16_t);
		} else
#endif /* VA_SIZE < 4 */
#if VA_SIZE < 8
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I32)) {
			print_ch = va_arg(args, uint32_t);
		} else
#endif /* VA_SIZE < 8 */
		{
			print_ch = (uint32_t)va_arg(args, uint64_t);
		}
		temp = DeeFormat_Putc(printer, arg, print_ch);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}	break;

	case 'C': {
		uint32_t print_ch;
#if VA_SIZE < 2
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I8)) {
			print_ch = (uint32_t)va_arg(args, uint8_t);
		} else
#endif /* VA_SIZE < 2 */
#if VA_SIZE < 4
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I16)) {
			print_ch = (uint32_t)va_arg(args, uint16_t);
		} else
#endif /* VA_SIZE < 4 */
#if VA_SIZE < 8
		if (LENGTH_VASIZEOF(length) == LENGTH_VASIZEOF(LENGTH_I32)) {
			print_ch = va_arg(args, uint32_t);
		} else
#endif /* VA_SIZE < 8 */
		{
			print_ch = (uint32_t)va_arg(args, uint64_t);
		}
		if (!(flags & F_PREFIX))
			print("\'", 1);
		if (print_ch <= 0xff) {
			uint8_t ch8 = (uint8_t)print_ch;
			temp = DeeFormat_Quote8(printer, arg, &ch8, 1);
		} else if (print_ch <= 0xffff) {
			uint16_t ch16 = (uint16_t)print_ch;
			temp = DeeFormat_Quote16(printer, arg, &ch16, 1);
		} else {
			temp = DeeFormat_Quote32(printer, arg, &print_ch, 1);
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
		if (!(flags & F_PREFIX))
			print("\'", 1);
	}	break;

	case 'q':
	case 's': {
		char const *string;
		size_t string_length;
		string = va_arg(args, char const *);
		if (!(flags & F_HASPREC))
			precision = (size_t)-1;
#if __SIZEOF_WCHAR_T__ != __SIZEOF_LONG__
		if (length & 0x100)
			length = PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_WCHAR_T__));
#endif /* __SIZEOF_WCHAR_T__ != __SIZEOF_LONG__ */
		switch (LENGTH_IXSIZEOF(length)) {

		default: /* UTF-8 */
			if (!string)
				string = (char const *)null8;
			if (flags & F_FIXBUF) {
				string_length = precision;
			} else {
				string_length = strnlen(string, precision);
			}
			if (ch == 'q') {
				if (!(flags & F_PREFIX)) {
					temp = (*printer)(arg, dquote, 1);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				temp = DeeFormat_Quote(printer, arg, string, string_length);
				if (!(flags & F_PREFIX)) {
					if unlikely(temp < 0)
						goto err;
					result += temp;
					temp = (*printer)(arg, dquote, 1);
				}
			} else {
				temp = (*printer)(arg, string, string_length);
			}
			break;

		case LENGTH_IXSIZEOF(LENGTH_I8): /* 1-byte */
			if (!string)
				string = (char const *)null8;
			if (flags & F_FIXBUF) {
				string_length = precision;
			} else {
				string_length = strnlen(string, precision);
			}
			if (ch == 'q') {
				if (!(flags & F_PREFIX)) {
					temp = (*printer)(arg, dquote, 1);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				temp = DeeFormat_Quote8(printer, arg, (uint8_t *)string, string_length);
				if (!(flags & F_PREFIX)) {
					if unlikely(temp < 0)
						goto err;
					result += temp;
					temp = (*printer)(arg, dquote, 1);
				}
			} else {
				temp = DeeFormat_Print8(printer, arg, (uint8_t *)string, string_length);
			}
			break;

		case LENGTH_IXSIZEOF(LENGTH_I16): /* 2-byte */
			if (!string)
				string = (char const *)null16;
			if (flags & F_FIXBUF) {
				string_length = precision;
			} else {
				string_length = strnlen16((uint16_t *)string, precision);
			}
			if (ch == 'q') {
				if (!(flags & F_PREFIX)) {
					temp = (*printer)(arg, dquote, 1);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				temp = DeeFormat_Quote16(printer, arg, (uint16_t *)string, string_length);
				if (!(flags & F_PREFIX)) {
					if unlikely(temp < 0)
						goto err;
					result += temp;
					temp = (*printer)(arg, dquote, 1);
				}
			} else {
				temp = DeeFormat_Print16(printer, arg, (uint16_t *)string, string_length);
			}
			break;

		case LENGTH_IXSIZEOF(LENGTH_I32): /* 4-byte */
			if (!string)
				string = (char const *)null32;
			if (flags & F_FIXBUF) {
				string_length = precision;
			} else {
				string_length = strnlen32((uint32_t *)string, precision);
			}
			if (ch == 'q') {
				if (!(flags & F_PREFIX)) {
					temp = (*printer)(arg, dquote, 1);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				temp = DeeFormat_Quote32(printer, arg, (uint32_t *)string, string_length);
				if (!(flags & F_PREFIX)) {
					if unlikely(temp < 0)
						goto err;
					result += temp;
					temp = (*printer)(arg, dquote, 1);
				}
			} else {
				temp = DeeFormat_Print32(printer, arg, (uint32_t *)string, string_length);
			}
			break;
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}	break;

	case 'k':
	case 'K':
	case 'r':
	case 'R': {
		/* Print an object representation. */
		DeeObject *in_ob;
		in_ob = va_arg(args, DeeObject *);
		/* Allow propagation of errors if input operands were NULL. */
		if unlikely(!in_ob) {
			temp = -1;
			goto err;
		}
		/* Create the string/repr object from the given input. */
		if (ch == 'k' || ch == 'K') {
			temp = DeeObject_Print(in_ob, printer, arg);
			if (ch == 'K')
				Dee_Decref(in_ob);
		} else {
			temp = DeeObject_PrintRepr(in_ob, printer, arg);
			/* Drop a reference from the input object if need be. */
			if (ch == 'R')
				Dee_Decref(in_ob);
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}	break;

	case 'V': {
		struct Dee_variant *v = va_arg(args, struct Dee_variant *);
		ch = *format++;
		ASSERT(ch == 'k' || ch == 'r');
		temp = ch == 'r' ? Dee_variant_printrepr(v, printer, arg)
		                 : Dee_variant_print(v, printer, arg);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}	break;

	case 'f':
	case 'F':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
#ifdef __COMPILER_HAVE_LONGDOUBLE
		if (length == LENGTH_L) {
			long double value;
			value = va_arg(args, long double);
#if (F_LJUST == DEEFLOAT_PRINT_FLJUST &&     \
     F_SIGN == DEEFLOAT_PRINT_FSIGN &&       \
     F_SPACE == DEEFLOAT_PRINT_FSPACE &&     \
     F_PADZERO == DEEFLOAT_PRINT_FPADZERO && \
     F_HASWIDTH == DEEFLOAT_PRINT_FWIDTH &&  \
     F_HASPREC == DEEFLOAT_PRINT_FPRECISION)
			temp = DeeFloat_LPrint(value, printer, arg, width, precision, flags);
#else /* ... */
			{
				unsigned int float_flags = DEEFLOAT_PRINT_FNORMAL;
				if (flags & F_LJUST)
					float_flags |= DEEFLOAT_PRINT_FLJUST;
				if (flags & F_SIGN)
					float_flags |= DEEFLOAT_PRINT_FSIGN;
				if (flags & F_SPACE)
					float_flags |= DEEFLOAT_PRINT_FSPACE;
				if (flags & F_PADZERO)
					float_flags |= DEEFLOAT_PRINT_FPADZERO;
				if (flags & F_HASWIDTH)
					float_flags |= DEEFLOAT_PRINT_FWIDTH;
				if (flags & F_HASPREC)
					float_flags |= DEEFLOAT_PRINT_FPRECISION;
				temp = DeeFloat_LPrint(value, printer, arg, width, precision, float_flags);
			}
#endif /* !... */
		} else
#endif /* __COMPILER_HAVE_LONGDOUBLE */
		{
			double value;
			value = va_arg(args, double);
#if (F_LJUST == DEEFLOAT_PRINT_FLJUST &&     \
     F_SIGN == DEEFLOAT_PRINT_FSIGN &&       \
     F_SPACE == DEEFLOAT_PRINT_FSPACE &&     \
     F_PADZERO == DEEFLOAT_PRINT_FPADZERO && \
     F_HASWIDTH == DEEFLOAT_PRINT_FWIDTH &&  \
     F_HASPREC == DEEFLOAT_PRINT_FPRECISION)
			temp = DeeFloat_Print(value, printer, arg, width, precision, flags);
#else /* ... */
		{
			unsigned int float_flags = DEEFLOAT_PRINT_FNORMAL;
			if (flags & F_LJUST)
				float_flags |= DEEFLOAT_PRINT_FLJUST;
			if (flags & F_SIGN)
				float_flags |= DEEFLOAT_PRINT_FSIGN;
			if (flags & F_SPACE)
				float_flags |= DEEFLOAT_PRINT_FSPACE;
			if (flags & F_PADZERO)
				float_flags |= DEEFLOAT_PRINT_FPADZERO;
			if (flags & F_HASWIDTH)
				float_flags |= DEEFLOAT_PRINT_FWIDTH;
			if (flags & F_HASPREC)
				float_flags |= DEEFLOAT_PRINT_FPRECISION;
			temp = DeeFloat_Print(value, printer, arg, width, precision, float_flags);
		}
#endif /* !... */
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
		break;

	default:
		if (ch >= '0' && ch <= '9') {
			/* decimal-encoded width modifier. */
			width = (size_t)(ch - '0');
			while ((ch = *format, ch >= '0' && ch <= '9')) {
				width = width * 10 + (size_t)(ch - '0');
				++format;
			}
			flags |= F_HASWIDTH;
			goto nextfmt;
		}
broken_format:
		format = flush_start;
		goto next;
	}
	flush_start = format;
	goto next;

end:
	--format;
	ASSERT(!*format);
	if (flush_start != format) {
		temp = (*printer)(arg, flush_start, (size_t)(format - flush_start));
		if unlikely(temp < 0)
			goto err_finish;
		result += temp;
	}
	return result;
err:
	/* Parse the remainder to inherit passed objects. */
	format_cleanup(format, args);
err_finish:
	return temp;
}


PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t
DeeFormat_Printf(Dee_formatprinter_t printer, void *arg,
                 char const *__restrict format, ...) {
	Dee_ssize_t result;
	va_list args;
	va_start(args, format);
	result = DeeFormat_VPrintf(printer, arg, format, args);
	va_end(args);
	return result;
}

#define is_normal_ascii_printable(byte)                  \
	((byte) < 0x80 && DeeUni_IsPrint((uint8_t)(byte)) && \
	 !strchr("\'\"\\", (uint8_t)(byte)))


/* Quote (backslash-escape) the given text, printing the resulting text to `printer'.
 * NOTE: This function always generates pure ASCII, and is therefor safe to be used
 *       when targeting an `ascii_printer'
 * Output:
 * - ASCII+isprint --> keep
 * - ASCII+iscntrl --> \r, \n, \b, ...
 * - ASCII         --> \177
 * - other         --> \uABCD, \U1234ABCD */
PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Quote(Dee_formatprinter_t printer, void *arg,
                /*utf-8*/ char const *__restrict text, size_t textlen) {
	Dee_ssize_t temp, result = 0;
	char const *flush_start, *end;
	flush_start = text;
	end         = text + textlen;
	while (text < end) {
		char escapeseq[COMPILER_STRLEN("\\UFFFFFFFF")], *esc;
		uint32_t ch;
		unsigned char byte = (unsigned char)*text;
		if (is_normal_ascii_printable(byte)) {
			++text;
			continue;
		}
		if (flush_start < text)
			DO((*printer)(arg, flush_start, (size_t)(text - flush_start)));
		ch = unicode_readutf8_n(&text, end);
		esc = escapeseq;
		*esc++ = '\\';
		switch (ch) {
		case '\a': *esc++ = 'a'; break;
		case '\b': *esc++ = 'b'; break;
		case '\f': *esc++ = 'f'; break;
		case '\n': *esc++ = 'n'; break;
		case '\r': *esc++ = 'r'; break;
		case '\t': *esc++ = 't'; break;
		case '\v': *esc++ = 'v'; break;
#if 0 /* Don't include non-standard extensions here... */
		case '\033': *esc++ = 'e'; break;
#endif
		case '\\':
		case '\'':
		case '\"':
			*esc++ = (char)(unsigned char)ch;
			break;

		default:
			if (ch <= 0x7f) {
				/* Encode using octal representation */
				bool can_use_short = text >= end || !(*text >= '0' && *text <= '7');
				if (ch <= 07 && can_use_short) {
					*esc++ = '0' + (uint8_t)ch;
				} else if (ch <= 077 && can_use_short) {
					*esc++ = '0' + (uint8_t)((ch & 0070) >> 3);
					*esc++ = '0' + (uint8_t)((ch & 0007));
				} else {
					*esc++ = '0' + (uint8_t)((ch & 0700) >> 6);
					*esc++ = '0' + (uint8_t)((ch & 0070) >> 3);
					*esc++ = '0' + (uint8_t)((ch & 0007));
				}
			} else if (ch <= 0xffff) {
				/* Encode using "\uABCD" */
				*esc++ = 'u';
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0xf000)) >> 12);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x0f00)) >> 8);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x00f0)) >> 4);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x000f)));
			} else {
				/* Encode using "\U1234ABCD" */
				*esc++ = 'U';
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0xf0000000)) >> 28);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x0f000000)) >> 24);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x00f00000)) >> 20);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x000f0000)) >> 16);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x0000f000)) >> 12);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x00000f00)) >> 8);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x000000f0)) >> 4);
				*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x0000000f)));
			}
			break;
		}
		DO((*printer)(arg, escapeseq, (size_t)(esc - escapeseq)));
		flush_start = text;
	}
	if (flush_start < end)
		DO((*printer)(arg, flush_start, (size_t)(end - flush_start)));
	return result;
err:
	return temp;
}

/* Format-quote raw bytes. Output:
 * - ASCII+isprint --> keep
 * - ASCII+iscntrl --> \r, \n, \b, ...
 * - ASCII+[0-7]   --> \0, \1, ... \7
 * - other         --> \xAB */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_QuoteBytes(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                     /*bytes*/ void const *__restrict data, size_t num_bytes) {
	Dee_ssize_t temp, result = 0;
	byte_t const *text, *flush_start, *end;
	text        = (unsigned char const *)data;
	flush_start = text;
	end         = text + num_bytes;
	while (text < end) {
		char escapeseq[COMPILER_STRLEN("\\xFF")], *esc;
		byte_t byte = *text++;
		if (is_normal_ascii_printable(byte))
			continue;
		if (flush_start < (text - 1))
			DO((*printer)(arg, (char const *)flush_start, (size_t)((text - 1) - flush_start)));
		esc = escapeseq;
		*esc++ = '\\';
		switch (byte) {
		case '\a': *esc++ = 'a'; break;
		case '\b': *esc++ = 'b'; break;
		case '\f': *esc++ = 'f'; break;
		case '\n': *esc++ = 'n'; break;
		case '\r': *esc++ = 'r'; break;
		case '\t': *esc++ = 't'; break;
		case '\v': *esc++ = 'v'; break;
#if 0 /* Don't include non-standard extensions here... */
		case '\033': *esc++ = 'e'; break;
#endif
		case '\\':
		case '\'':
		case '\"':
			*esc++ = (char)(unsigned char)byte;
			break;

		default:
			if (byte <= 07 && (text >= end || !(*text >= '0' && *text <= '7'))) {
				/* Encode using octal representation */
				*esc++ = '0' + (uint8_t)byte;
			} else if (text < end && DeeUni_AsDigitVal(*text) < 16) {
				/* Cannot use \x-encoding because the next character would become part of the number
				 * -> Instead, use octal (which is limited to a maximum of 3 consecutive digits). */
				*esc++ = '0' + (uint8_t)((byte & 0700) >> 6);
				*esc++ = '0' + (uint8_t)((byte & 0070) >> 3);
				*esc++ = '0' + (uint8_t)((byte & 0007));
			} else {
				/* Encode using "\xFF" */
				*esc++ = 'x';
				*esc++ = DeeAscii_ItoaUpperDigit((byte & 0xf0) >> 4);
				*esc++ = DeeAscii_ItoaUpperDigit((byte & 0x0f));
			}
			break;
		}
		DO((*printer)(arg, escapeseq, (size_t)(esc - escapeseq)));
		flush_start = text;
	}
	if (flush_start < end)
		DO((*printer)(arg, (char const *)flush_start, (size_t)(end - flush_start)));
	return result;
err:
	return temp;
}



/* Format-quote 8-bit, 16-bit, and 32-bit unicode text: */
PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Quote8(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                 /*latin-1*/ uint8_t const *__restrict text, size_t textlen) {
	Dee_ssize_t temp, result = 0;
	uint8_t const *flush_start, *end;
	flush_start = text;
	end         = text + textlen;
	while (text < end) {
		char escapeseq[COMPILER_STRLEN("\\u00FF")], *esc;
		uint8_t byte = *text++;
		if (is_normal_ascii_printable(byte))
			continue;
		if (flush_start < (text - 1))
			DO((*printer)(arg, (char const *)flush_start, (size_t)((text - 1) - flush_start)));
		esc = escapeseq;
		*esc++ = '\\';
		switch (byte) {
		case '\a': *esc++ = 'a'; break;
		case '\b': *esc++ = 'b'; break;
		case '\f': *esc++ = 'f'; break;
		case '\n': *esc++ = 'n'; break;
		case '\r': *esc++ = 'r'; break;
		case '\t': *esc++ = 't'; break;
		case '\v': *esc++ = 'v'; break;
#if 0 /* Don't include non-standard extensions here... */
		case '\033': *esc++ = 'e'; break;
#endif
		case '\\':
		case '\'':
		case '\"':
			*esc++ = (char)byte;
			break;

		default:
			if (byte <= 0x7f) {
				/* Encode using octal representation */
				bool can_use_short = text >= end || !(*text >= '0' && *text <= '7');
				if (byte <= 07 && can_use_short) {
					*esc++ = '0' + (uint8_t)byte;
				} else if (byte <= 077 && can_use_short) {
					*esc++ = '0' + (uint8_t)((byte & 0070) >> 3);
					*esc++ = '0' + (uint8_t)((byte & 0007));
				} else {
					*esc++ = '0' + (uint8_t)((byte & 0700) >> 6);
					*esc++ = '0' + (uint8_t)((byte & 0070) >> 3);
					*esc++ = '0' + (uint8_t)((byte & 0007));
				}
			} else {
				/* Encode using "\uABCD" */
				*esc++ = 'u';
				*esc++ = '0';
				*esc++ = '0';
				*esc++ = DeeAscii_ItoaUpperDigit((byte & 0xf0) >> 4);
				*esc++ = DeeAscii_ItoaUpperDigit((byte & 0x0f));
			}
			break;
		}
		DO((*printer)(arg, escapeseq, (size_t)(esc - escapeseq)));
		flush_start = text;
	}
	if (flush_start < end)
		DO((*printer)(arg, (char const *)flush_start, (size_t)(end - flush_start)));
	return result;
err:
	return temp;
}

PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Quote16(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                  /*utf-16-without-surrogates*/ uint16_t const *__restrict text, size_t textlen) {
	Dee_ssize_t temp, result = 0;
	uint16_t const *end = text + textlen;
	while (text < end) {
		char escapeseq[COMPILER_STRLEN("\\uFFFF")], *esc;
		uint16_t ch = *text++;
		if (is_normal_ascii_printable(ch)) {
			escapeseq[0] = (char)(unsigned char)(uint8_t)ch;
			temp = (*printer)(arg, escapeseq, 1);
		} else {
			esc = escapeseq;
			*esc++ = '\\';
			switch (ch) {
			case '\a': *esc++ = 'a'; break;
			case '\b': *esc++ = 'b'; break;
			case '\f': *esc++ = 'f'; break;
			case '\n': *esc++ = 'n'; break;
			case '\r': *esc++ = 'r'; break;
			case '\t': *esc++ = 't'; break;
			case '\v': *esc++ = 'v'; break;
#if 0 /* Don't include non-standard extensions here... */
			case '\033': *esc++ = 'e'; break;
#endif
			case '\\':
			case '\'':
			case '\"':
				*esc++ = (char)(unsigned char)(uint8_t)ch;
				break;
	
			default:
				if (ch <= 0x7f) {
					/* Encode using octal representation */
					bool can_use_short = text >= end || !(*text >= '0' && *text <= '7');
					if (ch <= 07 && can_use_short) {
						*esc++ = '0' + (uint8_t)ch;
					} else if (ch <= 077 && can_use_short) {
						*esc++ = '0' + (uint8_t)((ch & 0070) >> 3);
						*esc++ = '0' + (uint8_t)((ch & 0007));
					} else {
						*esc++ = '0' + (uint8_t)((ch & 0700) >> 6);
						*esc++ = '0' + (uint8_t)((ch & 0070) >> 3);
						*esc++ = '0' + (uint8_t)((ch & 0007));
					}
				} else {
					/* Encode using "\uABCD" */
					*esc++ = 'u';
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0xf000)) >> 12);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x0f00)) >> 8);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x00f0)) >> 4);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x000f)));
				}
				break;
			}
			temp = (*printer)(arg, escapeseq, (size_t)(esc - escapeseq));
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}

PUBLIC WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_Quote32(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                  /*utf-32*/ uint32_t const *__restrict text, size_t textlen) {
	Dee_ssize_t temp, result = 0;
	uint32_t const *end = text + textlen;
	while (text < end) {
		char escapeseq[COMPILER_STRLEN("\\UFFFFFFFF")], *esc;
		uint32_t ch = *text++;
		if (is_normal_ascii_printable(ch)) {
			escapeseq[0] = (char)(unsigned char)(uint8_t)ch;
			temp = (*printer)(arg, escapeseq, 1);
		} else {
			esc = escapeseq;
			*esc++ = '\\';
			switch (ch) {
			case '\a': *esc++ = 'a'; break;
			case '\b': *esc++ = 'b'; break;
			case '\f': *esc++ = 'f'; break;
			case '\n': *esc++ = 'n'; break;
			case '\r': *esc++ = 'r'; break;
			case '\t': *esc++ = 't'; break;
			case '\v': *esc++ = 'v'; break;
#if 0 /* Don't include non-standard extensions here... */
			case '\033': *esc++ = 'e'; break;
#endif
			case '\\':
			case '\'':
			case '\"':
				*esc++ = (char)(unsigned char)(uint8_t)ch;
				break;
	
			default:
				if (ch <= 0x7f) {
					/* Encode using octal representation */
					bool can_use_short = text >= end || !(*text >= '0' && *text <= '7');
					if (ch <= 07 && can_use_short) {
						*esc++ = '0' + (uint8_t)ch;
					} else if (ch <= 077 && can_use_short) {
						*esc++ = '0' + (uint8_t)((ch & 0070) >> 3);
						*esc++ = '0' + (uint8_t)((ch & 0007));
					} else {
						*esc++ = '0' + (uint8_t)((ch & 0700) >> 6);
						*esc++ = '0' + (uint8_t)((ch & 0070) >> 3);
						*esc++ = '0' + (uint8_t)((ch & 0007));
					}
				} else if (ch <= 0xffff) {
					/* Encode using "\uABCD" */
					*esc++ = 'u';
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0xf000)) >> 12);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x0f00)) >> 8);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x00f0)) >> 4);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT16_C(0x000f)));
				} else {
					/* Encode using "\U1234ABCD" */
					*esc++ = 'U';
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0xf0000000)) >> 28);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x0f000000)) >> 24);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x00f00000)) >> 20);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x000f0000)) >> 16);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x0000f000)) >> 12);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x00000f00)) >> 8);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x000000f0)) >> 4);
					*esc++ = DeeAscii_ItoaUpperDigit((ch & UINT32_C(0x0000000f)));
				}
				break;
			}
			temp = (*printer)(arg, escapeseq, (size_t)(esc - escapeseq));
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}


/* Repeat the given `ch' a total of `count' times. */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t DCALL
DeeFormat_Repeat(/*ascii*/ Dee_formatprinter_t printer, void *arg,
                 /*ascii*/ char ch, size_t count) {
	char buffer[128];
	Dee_ssize_t temp, result;
	if (COMPILER_LENOF(buffer) >= count) {
		memset(buffer, ch, count);
		return (*printer)(arg, buffer, count);
	}

	/* Fast-paths for certain printers. */
	if (printer == &unicode_printer_print) {
		return unicode_printer_repeatascii((struct unicode_printer *)arg, ch, count);
	} else if (printer == &ascii_printer_print) {
		char *buf = ascii_printer_alloc((struct ascii_printer *)arg, count);
		if unlikely(!buf)
			return -1;
		memset(buf, ch, count);
		return (Dee_ssize_t)count;
	} else if (printer == &bytes_printer_print) {
		unsigned char *buf = bytes_printer_alloc((struct bytes_printer *)arg, count);
		if unlikely(!buf)
			return -1;
		memset(buf, ch, count);
		return (Dee_ssize_t)count;
	}

	result = 0;
	memset(buffer, ch, sizeof(buffer));
	while (count) {
		size_t part;
		part = MIN(count, COMPILER_LENOF(buffer));
		temp = (*printer)(arg, buffer, part);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		count -= part;
	}
	return result;
err:
	return temp;
}



/* Repeat `str...+=length' such that a total of `total_characters'
 * characters (not bytes, but characters) are printed. */
DFUNDEF WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
DeeFormat_RepeatUtf8(/*utf-8*/ Dee_formatprinter_t printer, void *arg,
                     /*utf-8*/ char const *__restrict str,
                     size_t length, size_t total_characters) {
	size_t utf8_length, i;
	Dee_ssize_t temp, result;
	ASSERT(length != 0);
	/* Optimization for single-ascii-character repetitions.
	 * NOTE: Because the input should be UTF-8, str[0] should
	 *       also be an ASCII character, because anything else
	 *       would require at least 2 bytes.
	 *       If it isn't, then it's the caller's fault for not
	 *       passing a valid UTF-8 string, so us not operating
	 *       correctly is on them, too. */
	if (length == 1)
		return DeeFormat_Repeat(printer, arg, str[0], total_characters);
	utf8_length = length;
	for (i = 0; i < length; ++i) {
		uint8_t ch = (uint8_t)str[i];
		if (ch < 0xc0)
			continue;
		ch = unicode_utf8seqlen[ch];
		ASSERT(ch != 0);
		utf8_length -= ch - 1;
		i += ch - 1;
	}
	result = 0;
	while (total_characters >= utf8_length) {
		temp = (*printer)(arg, str, length);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		total_characters -= utf8_length;
	}
	if (total_characters) {
		/* Print the remainder as a portion of the given repetition-string. */
		i = 0;
		while (i < length && total_characters--) {
			uint8_t chr = (uint8_t)str[i];
			uint8_t len = unicode_utf8seqlen_safe[chr];
			i += len;
		}
		temp = (*printer)(arg, str, i);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}




PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DPRINTER_CC
sprintf_callback(char **__restrict pbuffer,
                 char const *__restrict data, size_t datalen) {
	*pbuffer = (char *)mempcpyc(*pbuffer, data, datalen, sizeof(char));
	return (Dee_ssize_t)datalen;
}

struct snprintf_data {
	char  *buf;
	size_t siz;
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DPRINTER_CC
snprintf_callback(struct snprintf_data *__restrict arg,
                  char const *__restrict data, size_t datalen) {
	if (arg->siz) {
		size_t copy_size;
		copy_size = arg->siz < datalen ? arg->siz : datalen;
		memcpyc(arg->buf, data, copy_size, sizeof(char));
		arg->siz -= copy_size;
	}
	arg->buf += datalen;
	return (Dee_ssize_t)datalen;
}

PUBLIC NONNULL((1, 2)) char *DCALL
Dee_vsprintf(char *__restrict buffer,
             char const *__restrict format, va_list args) {
	if unlikely(DeeFormat_VPrintf((Dee_formatprinter_t)&sprintf_callback,
	                              (void *)&buffer, format, args) < 0)
		goto err;
	*buffer = '\0';
	return buffer;
err:
	return NULL;
}

PUBLIC NONNULL((3)) char *DCALL
Dee_vsnprintf(char *__restrict buffer, size_t bufsize,
              char const *__restrict format, va_list args) {
	struct snprintf_data data;
	data.buf = buffer;
	data.siz = bufsize;
	if unlikely(DeeFormat_VPrintf((Dee_formatprinter_t)&snprintf_callback,
	                              &data, format, args) < 0)
		goto err;
	if (data.siz)
		*data.buf = '\0';
	return data.buf;
err:
	return NULL;
}

/* Both of these functions return a pointer to the target address where
 * printing has/would have stopped (excluding a terminating \0-character).
 * In the event that the buffer provided to `Dee_snprintf' is insufficient,
 * at most `bufsize' characters will have been written, and the exact required
 * size can be determined by `((return - buffer) + 1) * sizeof(char)'.
 * In the event of an error (only possible when `format' contains
 * something like `%k' or `%r'), `NULL' will be returned. */
PUBLIC NONNULL((1, 2)) char *
Dee_sprintf(char *__restrict buffer,
            char const *__restrict format, ...) {
	char *result;
	va_list args;
	va_start(args, format);
	result = Dee_vsprintf(buffer, format, args);
	va_end(args);
	return result;
}

PUBLIC NONNULL((3)) char *
Dee_snprintf(char *__restrict buffer, size_t bufsize,
             char const *__restrict format, ...) {
	char *result;
	va_list args;
	va_start(args, format);
	result = Dee_vsnprintf(buffer, bufsize, format, args);
	va_end(args);
	return result;
}




/************************************************************************/
/* Extensible formating functions                                       */
/************************************************************************/

LOCAL WUNUSED NONNULL((1)) struct kwds_entry *DCALL
kwds_find_entry(DeeKwdsObject *__restrict self, size_t index) {
	size_t i;
	for (i = 0; i <= self->kw_mask; ++i) {
		if (!self->kw_map[i].ke_name)
			continue;
		if (self->kw_map[i].ke_index != index)
			continue;
		return &self->kw_map[i];
	}
	return NULL;
}

struct print_keyword_argument_type_data {
	Dee_formatprinter_t pkatd_printer;
	void          *pkatd_arg;
	size_t         pkatd_argc;
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
print_keyword_argument_type_foreach(void *arg, DeeObject *key, DeeObject *value) {
	char const *str;
	Dee_ssize_t temp, result = 0;
	struct print_keyword_argument_type_data *me = (struct print_keyword_argument_type_data *)arg;
	if (me->pkatd_argc != 0) {
		temp = (*me->pkatd_printer)(me->pkatd_arg, ", ", 2);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	me->pkatd_argc = 1;
	temp = DeeObject_Print(key, me->pkatd_printer, me->pkatd_arg);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	temp = (*me->pkatd_printer)(me->pkatd_arg, ": ", 2);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	str = DeeType_GetName(Dee_TYPE(value));
	temp = DeeFormat_PrintStr(me->pkatd_printer, me->pkatd_arg, str);
	if unlikely(temp < 0)
		goto err;
	result += temp;
	return result;
err:
	return temp;
}

/* Print the object types passed by the given argument list.
 * If given, also include keyword names & types from `kw'
 * >> foo(10, 1.0, "bar", enabled: true);
 * Printed: "int, float, string, enabled: bool" */
PUBLIC WUNUSED ATTR_INS(4, 3) NONNULL((1)) Dee_ssize_t
(DCALL DeeFormat_PrintArgumentTypesKw)(Dee_formatprinter_t printer, void *arg,
                                       size_t argc, DeeObject *const *argv,
                                       DeeObject *kw) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (kw && DeeKwds_Check(kw)) {
		size_t kwargc = DeeKwds_SIZE(kw);
		struct kwds_entry *entry;
		if unlikely(kwargc > argc) {
			kwargc = argc;
		} else {
			for (i = 0; i < argc - kwargc; ++i) {
				char const *str;
				if (i != 0) {
					temp = (*printer)(arg, ", ", 2);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				str  = DeeType_GetName(Dee_TYPE(argv[i]));
				temp = DeeFormat_PrintStr(printer, arg, str);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
		}
		for (i = 0; i < kwargc; ++i) {
			char const *str;
			if (i != 0 || argc > kwargc) {
				temp = (*printer)(arg, ", ", 2);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			entry = kwds_find_entry((DeeKwdsObject *)kw, i);
			if (entry) {
				temp = DeeString_PrintUtf8((DeeObject *)entry->ke_name, printer, arg);
				if unlikely(temp < 0)
					goto err;
				result += temp;
				temp = (*printer)(arg, ": ", 2);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			str  = DeeType_GetName(Dee_TYPE(argv[argc - kwargc + i]));
			temp = DeeFormat_PrintStr(printer, arg, str);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		goto done;
	}
	for (i = 0; i < argc; ++i) {
		char const *str;
		if (i != 0) {
			temp = (*printer)(arg, ", ", 2);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		str  = DeeType_GetName(Dee_TYPE(argv[i]));
		temp = DeeFormat_PrintStr(printer, arg, str);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	if (kw) {
		/* Print keyword passed via a generic mappings. */
		struct print_keyword_argument_type_data data;
		data.pkatd_printer = printer;
		data.pkatd_arg     = arg;
		data.pkatd_argc    = argc;
		temp = DeeObject_ForeachPair(kw, &print_keyword_argument_type_foreach, &data);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
done:
	return result;
err:
	return temp;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_FORMAT_C */
