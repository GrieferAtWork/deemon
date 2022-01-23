/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_FORMAT_C
#define GUARD_DEEMON_RUNTIME_FORMAT_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* strnlen(), memcpyc(), ... */

#include <hybrid/minmax.h>
#include <hybrid/typecore.h>

#include <stdarg.h>
#include <stddef.h>

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


#ifndef __SIZEOF_CHAR__
#define __SIZEOF_CHAR__  1
#endif /* !__SIZEOF_CHAR__ */
#ifndef __SIZEOF_SHORT__
#define __SIZEOF_SHORT__ 2
#endif /* !__SIZEOF_SHORT__ */
#ifndef __SIZEOF_INT__
#define __SIZEOF_INT__   4
#endif /* !__SIZEOF_INT__ */
#ifndef __SIZEOF_LONG__
#ifdef CONFIG_HOST_WINDOWS
#define __SIZEOF_LONG__  4
#else /* CONFIG_HOST_WINDOWS */
#define __SIZEOF_LONG__  __SIZEOF_POINTER__
#endif /* !CONFIG_HOST_WINDOWS */
#endif /* !__SIZEOF_LONG__ */

DECL_BEGIN

#ifndef CONFIG_HAVE_strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */

STATIC_ASSERT(sizeof(int) == __SIZEOF_INT__);
STATIC_ASSERT(sizeof(void *) == __SIZEOF_POINTER__);
STATIC_ASSERT(sizeof(size_t) == __SIZEOF_POINTER__);
STATIC_ASSERT(sizeof(char) == __SIZEOF_CHAR__);
STATIC_ASSERT(sizeof(short) == __SIZEOF_SHORT__);
STATIC_ASSERT(sizeof(long) == __SIZEOF_LONG__);


#define VA_SIZE  __SIZEOF_INT__


#if VA_SIZE == 8
#   define LENGTH_I64  0x40
#   define LENGTH_I32  0x30
#   define LENGTH_I16  0x20
#   define LENGTH_I8   0x10
#elif VA_SIZE == 4
#   define LENGTH_I64  0x41
#   define LENGTH_I32  0x30
#   define LENGTH_I16  0x20
#   define LENGTH_I8   0x10
#elif VA_SIZE == 2
#   define LENGTH_I64  0x42
#   define LENGTH_I32  0x31
#   define LENGTH_I16  0x20
#   define LENGTH_I8   0x10
#elif VA_SIZE == 1
#   define LENGTH_I64  0x43
#   define LENGTH_I32  0x32
#   define LENGTH_I16  0x21
#   define LENGTH_I8   0x10
#else
#   error "Error: Unsupported `VA_SIZE'"
#endif

#define LENGTH_VASIZEOF(x) ((x)&0xf)
#define LENGTH_IXSIZEOF(x) ((x)&0xf0)


#define LENGTH_L    'L'
#define LENGTH_Z    'z'
#define LENGTH_T    't'
#define LENGTH_J    LENGTH_I64 /* intmax_t */
#define LENGTH_SIZE PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_POINTER__))
#define LENGTH_HH   PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_CHAR__))
#define LENGTH_H    PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_SHORT__))
#define LENGTH_l   (PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_LONG__)) | 0x100)
#ifdef __SIZEOF_LONG_LONG__
#define LENGTH_LL   PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_LONG_LONG__))
#else
#define LENGTH_LL   LENGTH_I64
#endif



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

#define print(p, s)                   \
	do {                              \
		temp = (*printer)(arg, p, s); \
		if unlikely(temp < 0)         \
			goto err;                 \
		result += temp;               \
	}	__WHILE0

PRIVATE char const decimals[2][17] = {
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'x' },
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'X' }
};

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
		va_arg(args, size_t);
		goto nextfmt;
#endif /* __SIZEOF_POINTER__ > VA_SIZE */
	case '*':
		va_arg(args, unsigned int);
		goto nextfmt;

	case '.': /* Precision. */
	case ':':
		ch = *format++;
#if __SIZEOF_POINTER__ > VA_SIZE
		if (ch == '*') {
			va_arg(args, unsigned int);
		} else if (ch == '?')
#else /* __SIZEOF_POINTER__ > VA_SIZE */
		if (ch == '*' || ch == '?')
#endif /* __SIZEOF_POINTER__ <= VA_SIZE */
		{
	case '$':
			va_arg(args, size_t);
		} else if (ch >= '0' && ch <= '9') {
			while ((ch = *format, ch >= '0' && ch <= '9'))
				++format;
		} else {
			goto broken_format;
		}
		goto nextfmt;

	case 'h':
		if (*format != 'h')
			length = LENGTH_VASIZEOF(LENGTH_H);
		else {
			++format;
			length = LENGTH_VASIZEOF(LENGTH_HH);
		}
		goto nextfmt;

	case 'l':
		if (*format != 'l')
			length = LENGTH_VASIZEOF(LENGTH_l);
		else {
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
		if (length == LENGTH_VASIZEOF(LENGTH_I8))
			va_arg(args, uint8_t);
		else
#endif /* VA_SIZE < 2 */
#if VA_SIZE < 4
		if (length == LENGTH_VASIZEOF(LENGTH_I16))
			va_arg(args, uint16_t);
		else
#endif /* VA_SIZE < 4 */
#if VA_SIZE < 8
		if (length == LENGTH_VASIZEOF(LENGTH_I32))
			va_arg(args, uint32_t);
		else
#endif /* VA_SIZE < 8 */
		{
			va_arg(args, uint64_t);
		}
		goto next;

	case 'c':
		va_arg(args, int);
		goto next;

	case 'q':
	case 's':
	case 'k':
	case 'r':
		va_arg(args, char *);
		goto next;

	case 'f':
	case 'F':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
#ifdef __COMPILER_HAVE_LONGDOUBLE
		if (length == LENGTH_L) {
			va_arg(args, long double);
		} else
#endif /* __COMPILER_HAVE_LONGDOUBLE */
		{
			va_arg(args, double);
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


PRIVATE uint8_t const null8[]   = { '(', 'n', 'u', 'l', 'l', ')', 0 };
PRIVATE uint16_t const null16[] = { '(', 'n', 'u', 'l', 'l', ')', 0 };
PRIVATE uint32_t const null32[] = { '(', 'n', 'u', 'l', 'l', ')', 0 };

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
PUBLIC WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeFormat_VPrintf(dformatprinter printer, void *arg,
                  char const *__restrict format, va_list args) {
	dssize_t result = 0, temp;
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
		if (*format != 'h')
			length = LENGTH_H;
		else {
			++format;
			length = LENGTH_HH;
		}
		goto nextfmt;

	case 'l':
		if (*format != 'l')
			length = LENGTH_l;
		else {
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
		} else if (ch == '1' && *format == '6') {
			length = LENGTH_I16;
		} else if (ch == '3' && *format == '2') {
			length = LENGTH_I32;
		} else if (ch == '6' && *format == '4') {
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
		{
			data.u = va_arg(args, uint64_t);
		}

#if F_UPPER == 1
		dec = decimals[flags & F_UPPER];
#else /* F_UPPER == 1 */
		dec = decimals[!!(flags & F_UPPER)];
#endif /* F_UPPER != 1 */
		is_neg = false;
		if (flags & F_SIGNED && data.i < 0) {
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
				*--iter = dec[16]; /* X/x */
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
			temp        = DeeFormat_Quote8(printer, arg, &ch8, 1, FORMAT_QUOTE_FPRINTRAW);
		} else if (print_ch <= 0xffff) {
			uint16_t ch16 = (uint16_t)print_ch;
			temp          = DeeFormat_Quote16(printer, arg, &ch16, 1, FORMAT_QUOTE_FPRINTRAW);
		} else {
			temp = DeeFormat_Quote32(printer, arg, &print_ch, 1, FORMAT_QUOTE_FPRINTRAW);
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
		if (!(flags & F_PREFIX))
			print("\'", 1);
	}	break;

	case 'q':
	case 's': {
		char *string;
		size_t string_length;
		string = va_arg(args, char *);
		if (!(flags & F_HASPREC))
			precision = (size_t)-1;
#if __SIZEOF_WCHAR_T__ != __SIZEOF_LONG__
		if (length & 0x100)
			length = PP_CAT2(LENGTH_I, PP_MUL8(__SIZEOF_WCHAR_T__));
#endif /* __SIZEOF_WCHAR_T__ != __SIZEOF_LONG__ */
		switch (LENGTH_IXSIZEOF(length)) {

		default: /* UTF-8 */
			if (!string)
				string = (char *)null8;
			if (flags & F_FIXBUF)
				string_length = precision;
			else {
				string_length = strnlen(string, precision);
			}
			if (ch == 'q') {
				temp = DeeFormat_Quote(printer, arg, string, string_length,
#if F_PREFIX == FORMAT_QUOTE_FPRINTRAW
				                       flags & F_PREFIX
#else /* F_PREFIX == FORMAT_QUOTE_FPRINTRAW */
				                       flags & F_PREFIX
				                       ? FORMAT_QUOTE_FPRINTRAW
				                       : FORMAT_QUOTE_FNORMAL
#endif /* F_PREFIX != FORMAT_QUOTE_FPRINTRAW */
				);
			} else {
				temp = (*printer)(arg, string, string_length);
			}
			break;

		case LENGTH_IXSIZEOF(LENGTH_I8): /* 1-byte */
			if (!string)
				string = (char *)null8;
			if (flags & F_FIXBUF)
				string_length = precision;
			else {
				string_length = strnlen(string, precision);
			}
			if (ch == 'q') {
				temp = DeeFormat_Quote8(printer, arg,
				                        (uint8_t *)string, string_length,
#if F_PREFIX == FORMAT_QUOTE_FPRINTRAW
				                        flags & F_PREFIX
#else /* F_PREFIX == FORMAT_QUOTE_FPRINTRAW */
				                        flags & F_PREFIX
				                        ? FORMAT_QUOTE_FPRINTRAW
				                        : FORMAT_QUOTE_FNORMAL
#endif /* F_PREFIX != FORMAT_QUOTE_FPRINTRAW */
				                        );
			} else {
				temp = DeeFormat_Print8(printer, arg, (uint8_t *)string, string_length);
			}
			break;

		case LENGTH_IXSIZEOF(LENGTH_I16): /* 2-byte */
			if (!string)
				string = (char *)null16;
			if (flags & F_FIXBUF)
				string_length = precision;
			else {
				string_length = strnlen16((uint16_t *)string, precision);
			}
			if (ch == 'q') {
				temp = DeeFormat_Quote16(printer, arg,
				                         (uint16_t *)string, string_length,
#if F_PREFIX == FORMAT_QUOTE_FPRINTRAW
				                         flags & F_PREFIX
#else /* F_PREFIX == FORMAT_QUOTE_FPRINTRAW */
				                         flags & F_PREFIX
				                         ? FORMAT_QUOTE_FPRINTRAW
				                         : FORMAT_QUOTE_FNORMAL
#endif /* F_PREFIX != FORMAT_QUOTE_FPRINTRAW */
				                         );
			} else {
				temp = DeeFormat_Print16(printer, arg, (uint16_t *)string, string_length);
			}
			break;

		case LENGTH_IXSIZEOF(LENGTH_I32): /* 4-byte */
			if (!string)
				string = (char *)null32;
			if (flags & F_FIXBUF)
				string_length = precision;
			else {
				string_length = strnlen32((uint32_t *)string, precision);
			}
			if (ch == 'q') {
				temp = DeeFormat_Quote32(printer, arg,
				                         (uint32_t *)string, string_length,
#if F_PREFIX == FORMAT_QUOTE_FPRINTRAW
				                         flags & F_PREFIX
#else /* F_PREFIX == FORMAT_QUOTE_FPRINTRAW */
				                         flags & F_PREFIX
				                         ? FORMAT_QUOTE_FPRINTRAW
				                         : FORMAT_QUOTE_FNORMAL
#endif /* F_PREFIX != FORMAT_QUOTE_FPRINTRAW */
				                         );
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


PUBLIC WUNUSED NONNULL((1, 3)) dssize_t
DeeFormat_Printf(dformatprinter printer, void *arg,
                 char const *__restrict format, ...) {
	dssize_t result;
	va_list args;
	va_start(args, format);
	result = DeeFormat_VPrintf(printer, arg, format, args);
	va_end(args);
	return result;
}



#define tooct(c) ('0'+(char)(unsigned char)(c))
PUBLIC WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeFormat_Quote8(dformatprinter printer, void *arg,
                 uint8_t const *__restrict text, size_t textlen,
                 unsigned int flags) {
	char encoded_text[6];
	size_t encoded_text_size;
	dssize_t result = 0, temp;
	uint8_t ch;
	uint8_t const *iter, *end, *flush_start;
	char const *c_hex;
	end             = (iter = flush_start = text) + textlen;
	c_hex           = decimals[!(flags & FORMAT_QUOTE_FUPPERHEX)];
	encoded_text[0] = '\\';
	if (!(flags & FORMAT_QUOTE_FPRINTRAW))
		print("\"", 1);
	while (iter != end) {
		ch = *iter;
		if (ch < 32 || ch >= 127 || ch == '\'' ||
		    ch == '\"' || ch == '\\' ||
		    (flags & FORMAT_QUOTE_FNOASCII)) {
			/* Character requires special encoding. */
#if 0
			if (!ch && !(flags & FORMAT_QUOTE_FQUOTEALL))
				goto done;
#endif
			/* Flush unprinted text. */
			if (iter != flush_start)
				print((char *)flush_start, (size_t)(iter - flush_start));
			flush_start = iter + 1;
			if (ch < 32) {
#if 0
				goto encode_hex;
#endif
				/* Control character. */
				if (flags & FORMAT_QUOTE_FNOCTRL) {
default_ctrl:
					if (flags & FORMAT_QUOTE_FFORCEHEX)
						goto encode_hex;
encode_oct:
					if (iter + 1 < end) {
						struct unitraits *desc = DeeUni_Descriptor(iter[1]);
						if ((desc->ut_flags & UNICODE_FDECIMAL) &&
						    (desc->ut_digit < 8))
							goto encode_hex;
					}
					if (ch <= 0x07) {
						encoded_text[1]   = tooct((ch & 0x07));
						encoded_text_size = 2;
					} else if (ch <= 0x3f) {
						encoded_text[1]   = tooct((ch & 0x38) >> 3);
						encoded_text[2]   = tooct((ch & 0x07));
						encoded_text_size = 3;
					} else {
						encoded_text[1]   = tooct((ch & 0xC0) >> 6);
						encoded_text[2]   = tooct((ch & 0x38) >> 3);
						encoded_text[3]   = tooct((ch & 0x07));
						encoded_text_size = 4;
					}
					goto print_encoded;
				}
special_control:
				switch (ch) {
				case '\a': ch = 'a'; break;
				case '\b': ch = 'b'; break;
				case '\f': ch = 'f'; break;
				case '\n': ch = 'n'; break;
				case '\r': ch = 'r'; break;
				case '\t': ch = 't'; break;
				case '\v': ch = 'v'; break;
				case '\033': ch = 'e'; break;
				case '\\':
				case '\'':
				case '\"': break;
				default: goto default_ctrl;
				}
				encoded_text[1]   = (char)ch;
				encoded_text_size = 2;
				goto print_encoded;
			} else if ((ch == '\\' || ch == '\'' || ch == '\"') &&
			           !(flags & FORMAT_QUOTE_FNOCTRL)) {
				goto special_control;
			} else {
				/* Non-ascii character. */
/*default_nonascii:*/
				if (flags & FORMAT_QUOTE_FFORCEOCT)
					goto encode_oct;
encode_hex:
				if (iter + 1 < end) {
					uint8_t next_ch = iter[1];
					/* Prevent ambiguity in something like "\x12ff" supposed to be "\x12" "ff" by using "\u0012ff" */
					/* FIXME: This whole way of escaping is broken!
					 *        Doing "\xFF" isn't the same as "\u00FF"!
					 *        The later encode the unicode character U+00FF (possibly as utf-8;
					 *        iow: \xC3\xBF), while the former encodes a single byte 0xFF / 255 */
					if ((next_ch >= 'a' && next_ch <= 'f') ||
					    (next_ch >= 'A' && next_ch <= 'F')) {
encode_uni:
						encoded_text[1]   = 'u';
						encoded_text[2]   = '0';
						encoded_text[3]   = '0';
						encoded_text[4]   = c_hex[(ch & 0xf0) >> 4];
						encoded_text[5]   = c_hex[ch & 0xf];
						encoded_text_size = 6;
						goto print_encoded;
					} else {
						struct unitraits *desc;
						desc = DeeUni_Descriptor(next_ch);
						if ((desc->ut_flags & UNICODE_FDECIMAL) &&
						    (desc->ut_digit < 10))
							goto encode_uni;
					}
				}
				encoded_text[1] = 'x';
				if (ch <= 0xf) {
					encoded_text[2]   = c_hex[ch];
					encoded_text_size = 3;
				} else {
					encoded_text[2]   = c_hex[(ch & 0xf0) >> 4];
					encoded_text[3]   = c_hex[ch & 0xf];
					encoded_text_size = 4;
				}
print_encoded:
				print(encoded_text, encoded_text_size);
				goto next;
			}
		}
next:
		++iter;
	}
/*done:*/
	if (iter != flush_start)
		print((char *)flush_start, (size_t)(iter - flush_start));
	if (!(flags & FORMAT_QUOTE_FPRINTRAW))
		print("\"", 1);
	return result;
err:
	return temp;
}


/* Format-quote 16-bit, and 32-bit text. */
PUBLIC WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeFormat_Quote16(dformatprinter printer, void *arg,
                  uint16_t const *__restrict text, size_t textlen,
                  unsigned int flags) {
	char encoded_text[7];
	size_t encoded_text_size;
	dssize_t result = 0, temp;
	char const *c_hex;
	size_t i;
	c_hex           = decimals[!(flags & FORMAT_QUOTE_FUPPERHEX)];
	encoded_text[0] = '\\';
	if (!(flags & FORMAT_QUOTE_FPRINTRAW))
		print("\"", 1);
	for (i = 0; i < textlen; ++i) {
		uint16_t ch = text[i];
		if (ch < 32 || ch >= 127 || ch == '\'' ||
		    ch == '\"' || ch == '\\' ||
		    (flags & FORMAT_QUOTE_FNOASCII)) {
			/* Character requires special encoding. */
			if (ch < 32) {
				/* Control character. */
				if (flags & FORMAT_QUOTE_FNOCTRL) {
default_ctrl:
					if (flags & FORMAT_QUOTE_FFORCEHEX)
						goto encode_hex;
encode_oct:
					if (i + 1 < textlen) {
						struct unitraits *desc = DeeUni_Descriptor(text[i + 1]);
						if ((desc->ut_flags & UNICODE_FDECIMAL) &&
						    (desc->ut_digit < 8))
							goto encode_hex;
					}
					if (ch <= 0x07) {
						encoded_text[1]   = tooct((ch & 0x0007));
						encoded_text_size = 2;
					} else if (ch <= 0x3f) {
						encoded_text[1]   = tooct((ch & 0x0038) >> 3);
						encoded_text[2]   = tooct((ch & 0x0007));
						encoded_text_size = 3;
					} else if (ch <= 0x1ff) {
						encoded_text[1]   = tooct((ch & 0x00c0) >> 6);
						encoded_text[2]   = tooct((ch & 0x0038) >> 3);
						encoded_text[3]   = tooct((ch & 0x0007));
						encoded_text_size = 4;
					} else if (ch <= 0xfff) {
						encoded_text[1]   = tooct((ch & 0x0e00) >> 9);
						encoded_text[2]   = tooct((ch & 0x00c0) >> 6);
						encoded_text[3]   = tooct((ch & 0x0038) >> 3);
						encoded_text[4]   = tooct((ch & 0x0007));
						encoded_text_size = 5;
					} else if (ch <= 0x7fff) {
						encoded_text[1]   = tooct((ch & 0x7000) >> 12);
						encoded_text[2]   = tooct((ch & 0x0e00) >> 9);
						encoded_text[3]   = tooct((ch & 0x00c0) >> 6);
						encoded_text[4]   = tooct((ch & 0x0038) >> 3);
						encoded_text[5]   = tooct((ch & 0x0007));
						encoded_text_size = 6;
					} else {
						encoded_text[1]   = tooct((ch & 0x8000) >> 15);
						encoded_text[2]   = tooct((ch & 0x7000) >> 12);
						encoded_text[3]   = tooct((ch & 0x0e00) >> 9);
						encoded_text[4]   = tooct((ch & 0x00c0) >> 6);
						encoded_text[5]   = tooct((ch & 0x0038) >> 3);
						encoded_text[6]   = tooct((ch & 0x0007));
						encoded_text_size = 7;
					}
					goto print_encoded;
				}
special_control:
				switch (ch) {
				case '\a': ch = 'a'; break;
				case '\b': ch = 'b'; break;
				case '\f': ch = 'f'; break;
				case '\n': ch = 'n'; break;
				case '\r': ch = 'r'; break;
				case '\t': ch = 't'; break;
				case '\v': ch = 'v'; break;
				case '\033': ch = 'e'; break;
				case '\\':
				case '\'':
				case '\"': break;
				default: goto default_ctrl;
				}
				encoded_text[1]   = (char)ch;
				encoded_text_size = 2;
				goto print_encoded;
			} else if ((ch == '\\' || ch == '\'' || ch == '\"') &&
			           !(flags & FORMAT_QUOTE_FNOCTRL)) {
				goto special_control;
			} else {
				/* Non-ascii character. */
/*default_nonascii:*/
				if (flags & FORMAT_QUOTE_FFORCEOCT)
					goto encode_oct;
encode_hex:
				if (i + 1 < textlen) {
					struct unitraits *desc;
					uint16_t next_ch = text[i + 1];
					if ((next_ch >= 'a' && next_ch <= 'f') ||
					    (next_ch >= 'A' && next_ch <= 'F'))
						goto encode_uni;
					desc = DeeUni_Descriptor(next_ch);
					if ((desc->ut_flags & UNICODE_FDECIMAL) &&
					    (desc->ut_digit < 10))
						goto encode_uni;
				}
				if (ch <= 0xf) {
					encoded_text[1]   = 'x';
					encoded_text[2]   = c_hex[ch];
					encoded_text_size = 3;
				} else if (ch <= 0xff) {
					encoded_text[1]   = 'x';
					encoded_text[2]   = c_hex[(ch & 0x00f0) >> 4];
					encoded_text[3]   = c_hex[ch & 0x000f];
					encoded_text_size = 4;
				} else {
encode_uni:
					encoded_text[1]   = 'u';
					encoded_text[2]   = c_hex[(ch & 0xf000) >> 12];
					encoded_text[3]   = c_hex[(ch & 0x0f00) >> 8];
					encoded_text[4]   = c_hex[(ch & 0x00f0) >> 4];
					encoded_text[5]   = c_hex[ch & 0x000f];
					encoded_text_size = 6;
				}
print_encoded:
				print(encoded_text, encoded_text_size);
				goto next;
			}
		} else {
			unsigned char print_ch[1];
			print_ch[0] = (unsigned char)ch;
			print((char *)print_ch, 1);
		}
next:
		;
	}
/*done:*/
	if (!(flags & FORMAT_QUOTE_FPRINTRAW))
		print("\"", 1);
	return result;
err:
	return temp;
}

PUBLIC WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeFormat_Quote32(dformatprinter printer, void *arg,
                  uint32_t const *__restrict text, size_t textlen,
                  unsigned int flags) {
	char encoded_text[12];
	size_t encoded_text_size;
	dssize_t result = 0, temp;
	char const *c_hex;
	size_t i;
	c_hex           = decimals[!(flags & FORMAT_QUOTE_FUPPERHEX)];
	encoded_text[0] = '\\';
	if (!(flags & FORMAT_QUOTE_FPRINTRAW))
		print("\"", 1);
	for (i = 0; i < textlen; ++i) {
		uint32_t ch = text[i];
		if (ch < 32 || ch >= 127 || ch == '\'' ||
		    ch == '\"' || ch == '\\' ||
		    (flags & FORMAT_QUOTE_FNOASCII)) {
			/* Character requires special encoding. */
			if (ch < 32) {
				/* Control character. */
				if (flags & FORMAT_QUOTE_FNOCTRL) {
default_ctrl:
					if (flags & FORMAT_QUOTE_FFORCEHEX)
						goto encode_hex;
encode_oct:
					if (i + 1 < textlen) {
						struct unitraits *desc = DeeUni_Descriptor(text[i + 1]);
						if ((desc->ut_flags & UNICODE_FDECIMAL) &&
						    (desc->ut_digit < 8))
							goto encode_hex;
					}
					if (ch <= 0x07) {
						encoded_text[1]   = tooct((ch & 0x00000007));
						encoded_text_size = 2;
					} else if (ch <= 0x3f) {
						encoded_text[1]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[2]   = tooct((ch & 0x00000007));
						encoded_text_size = 3;
					} else if (ch <= 0x1ff) {
						encoded_text[1]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[2]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[3]   = tooct((ch & 0x00000007));
						encoded_text_size = 4;
					} else if (ch <= 0xfff) {
						encoded_text[1]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[2]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[3]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[4]   = tooct((ch & 0x00000007));
						encoded_text_size = 5;
					} else if (ch <= 0x7fff) {
						encoded_text[1]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[2]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[3]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[4]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[5]   = tooct((ch & 0x00000007));
						encoded_text_size = 6;
					} else if (ch <= 0x3ffff) {
						encoded_text[1]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[2]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[3]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[4]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[5]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[6]   = tooct((ch & 0x00000007));
						encoded_text_size = 7;
					} else if (ch <= 0x1fffff) {
						encoded_text[1]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[2]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[3]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[4]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[5]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[6]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[7]   = tooct((ch & 0x00000007));
						encoded_text_size = 8;
					} else if (ch <= 0xffffff) {
						encoded_text[1]   = tooct((ch & 0x00e00000) >> 21);
						encoded_text[2]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[3]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[4]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[5]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[6]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[7]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[8]   = tooct((ch & 0x00000007));
						encoded_text_size = 9;
					} else if (ch <= 0x7ffffff) {
						encoded_text[1]   = tooct((ch & 0x07000000) >> 24);
						encoded_text[2]   = tooct((ch & 0x00e00000) >> 21);
						encoded_text[3]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[4]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[5]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[6]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[7]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[8]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[9]   = tooct((ch & 0x00000007));
						encoded_text_size = 10;
					} else if (ch <= 0x3fffffff) {
						encoded_text[1]   = tooct((ch & 0x38000000) >> 27);
						encoded_text[2]   = tooct((ch & 0x07000000) >> 24);
						encoded_text[3]   = tooct((ch & 0x00e00000) >> 21);
						encoded_text[4]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[5]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[6]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[7]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[8]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[9]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[10]  = tooct((ch & 0x00000007));
						encoded_text_size = 11;
					} else {
						encoded_text[1]   = tooct((ch & 0xc0000000) >> 30);
						encoded_text[2]   = tooct((ch & 0x38000000) >> 27);
						encoded_text[3]   = tooct((ch & 0x07000000) >> 24);
						encoded_text[4]   = tooct((ch & 0x00e00000) >> 21);
						encoded_text[5]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[6]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[7]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[8]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[9]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[10]  = tooct((ch & 0x00000038) >> 3);
						encoded_text[11]  = tooct((ch & 0x00000007));
						encoded_text_size = 12;
					}
					goto print_encoded;
				}
special_control:
				switch (ch) {
				case '\a': ch = 'a'; break;
				case '\b': ch = 'b'; break;
				case '\f': ch = 'f'; break;
				case '\n': ch = 'n'; break;
				case '\r': ch = 'r'; break;
				case '\t': ch = 't'; break;
				case '\v': ch = 'v'; break;
				case '\033': ch = 'e'; break;
				case '\\':
				case '\'':
				case '\"': break;
				default: goto default_ctrl;
				}
				encoded_text[1]   = (char)ch;
				encoded_text_size = 2;
				goto print_encoded;
			} else if ((ch == '\\' || ch == '\'' || ch == '\"') &&
			           !(flags & FORMAT_QUOTE_FNOCTRL)) {
				goto special_control;
			} else {
				/* Non-ascii character. */
/*default_nonascii:*/
				if (flags & FORMAT_QUOTE_FFORCEOCT)
					goto encode_oct;
encode_hex:
				if (i + 1 < textlen) {
					struct unitraits *desc;
					uint32_t next_ch = text[i + 1];
					if ((next_ch >= 'a' && next_ch <= 'f') ||
					    (next_ch >= 'A' && next_ch <= 'F'))
						goto encode_uni;
					desc = DeeUni_Descriptor(next_ch);
					if ((desc->ut_flags & UNICODE_FDECIMAL) &&
					    (desc->ut_digit < 10))
						goto encode_uni;
				}
				if (ch <= 0xf) {
					encoded_text[1]   = 'x';
					encoded_text[2]   = c_hex[ch];
					encoded_text_size = 3;
				} else if (ch <= 0xff) {
					encoded_text[1]   = 'x';
					encoded_text[2]   = c_hex[(ch & 0x000000f0) >> 4];
					encoded_text[3]   = c_hex[ch & 0x0000000f];
					encoded_text_size = 4;
				} else {
encode_uni:
					if (ch <= 0xffff) {
						encoded_text[1]   = 'u';
						encoded_text[2]   = c_hex[(ch & 0x0000f000) >> 12];
						encoded_text[3]   = c_hex[(ch & 0x00000f00) >> 8];
						encoded_text[4]   = c_hex[(ch & 0x000000f0) >> 4];
						encoded_text[5]   = c_hex[ch & 0x0000000f];
						encoded_text_size = 6;
					} else {
						encoded_text[1]   = 'U';
						encoded_text[2]   = c_hex[(ch & 0xf0000000) >> 28];
						encoded_text[3]   = c_hex[(ch & 0x0f000000) >> 24];
						encoded_text[4]   = c_hex[(ch & 0x00f00000) >> 20];
						encoded_text[5]   = c_hex[(ch & 0x000f0000) >> 16];
						encoded_text[6]   = c_hex[(ch & 0x0000f000) >> 12];
						encoded_text[7]   = c_hex[(ch & 0x00000f00) >> 8];
						encoded_text[8]   = c_hex[(ch & 0x000000f0) >> 4];
						encoded_text[9]   = c_hex[ch & 0x0000000f];
						encoded_text_size = 10;
					}
				}
print_encoded:
				print(encoded_text, encoded_text_size);
				goto next;
			}
		} else {
			unsigned char print_ch[1];
			print_ch[0] = (unsigned char)ch;
			print((char *)print_ch, 1);
		}
next:
		;
	}
/*done:*/
	if (!(flags & FORMAT_QUOTE_FPRINTRAW))
		print("\"", 1);
	return result;
err:
	return temp;
}


/* Quote (backslash-escape) the given text, printing the resulting text to `printer'.
 * NOTE: This function always generates pure ASCII, and is therefor safe to be used
 *       when targeting an `ascii_printer' */
PUBLIC WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeFormat_Quote(dformatprinter printer, void *arg,
                /*utf-8*/ char const *__restrict text, size_t textlen,
                unsigned int flags) {
	char encoded_text[12];
	size_t encoded_text_size;
	dssize_t result = 0, temp;
	char const *c_hex;
	char const *textend = text + textlen;
	c_hex               = decimals[!(flags & FORMAT_QUOTE_FUPPERHEX)];
	encoded_text[0]     = '\\';
	if (!(flags & FORMAT_QUOTE_FPRINTRAW))
		print("\"", 1);
	while (text < textend) {
		uint32_t ch = utf8_readchar((char const **)&text, textend);
		if (ch < 32 || ch >= 127 || ch == '\'' ||
		    ch == '\"' || ch == '\\' ||
		    (flags & FORMAT_QUOTE_FNOASCII)) {
			/* Character requires special encoding. */
			if (ch < 32) {
				/* Control character. */
				if (flags & FORMAT_QUOTE_FNOCTRL) {
default_ctrl:
					if (flags & FORMAT_QUOTE_FFORCEHEX)
						goto encode_hex;
encode_oct:
					if (text < textend) {
						char const *new_text   = text;
						uint32_t next_ch       = utf8_readchar((char const **)&new_text, textend);
						struct unitraits *desc = DeeUni_Descriptor(next_ch);
						if ((desc->ut_flags & UNICODE_FDECIMAL) &&
						    (desc->ut_digit < 8))
							goto encode_hex;
					}
					if (ch <= 0x07) {
						encoded_text[1]   = tooct((ch & 0x00000007));
						encoded_text_size = 2;
					} else if (ch <= 0x3f) {
						encoded_text[1]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[2]   = tooct((ch & 0x00000007));
						encoded_text_size = 3;
					} else if (ch <= 0x1ff) {
						encoded_text[1]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[2]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[3]   = tooct((ch & 0x00000007));
						encoded_text_size = 4;
					} else if (ch <= 0xfff) {
						encoded_text[1]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[2]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[3]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[4]   = tooct((ch & 0x00000007));
						encoded_text_size = 5;
					} else if (ch <= 0x7fff) {
						encoded_text[1]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[2]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[3]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[4]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[5]   = tooct((ch & 0x00000007));
						encoded_text_size = 6;
					} else if (ch <= 0x3ffff) {
						encoded_text[1]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[2]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[3]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[4]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[5]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[6]   = tooct((ch & 0x00000007));
						encoded_text_size = 7;
					} else if (ch <= 0x1fffff) {
						encoded_text[1]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[2]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[3]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[4]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[5]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[6]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[7]   = tooct((ch & 0x00000007));
						encoded_text_size = 8;
					} else if (ch <= 0xffffff) {
						encoded_text[1]   = tooct((ch & 0x00e00000) >> 21);
						encoded_text[2]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[3]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[4]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[5]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[6]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[7]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[8]   = tooct((ch & 0x00000007));
						encoded_text_size = 9;
					} else if (ch <= 0x7ffffff) {
						encoded_text[1]   = tooct((ch & 0x07000000) >> 24);
						encoded_text[2]   = tooct((ch & 0x00e00000) >> 21);
						encoded_text[3]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[4]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[5]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[6]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[7]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[8]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[9]   = tooct((ch & 0x00000007));
						encoded_text_size = 10;
					} else if (ch <= 0x3fffffff) {
						encoded_text[1]   = tooct((ch & 0x38000000) >> 27);
						encoded_text[2]   = tooct((ch & 0x07000000) >> 24);
						encoded_text[3]   = tooct((ch & 0x00e00000) >> 21);
						encoded_text[4]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[5]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[6]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[7]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[8]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[9]   = tooct((ch & 0x00000038) >> 3);
						encoded_text[10]  = tooct((ch & 0x00000007));
						encoded_text_size = 11;
					} else {
						encoded_text[1]   = tooct((ch & 0xc0000000) >> 30);
						encoded_text[2]   = tooct((ch & 0x38000000) >> 27);
						encoded_text[3]   = tooct((ch & 0x07000000) >> 24);
						encoded_text[4]   = tooct((ch & 0x00e00000) >> 21);
						encoded_text[5]   = tooct((ch & 0x001c0000) >> 18);
						encoded_text[6]   = tooct((ch & 0x00038000) >> 15);
						encoded_text[7]   = tooct((ch & 0x00007000) >> 12);
						encoded_text[8]   = tooct((ch & 0x00000e00) >> 9);
						encoded_text[9]   = tooct((ch & 0x000000c0) >> 6);
						encoded_text[10]  = tooct((ch & 0x00000038) >> 3);
						encoded_text[11]  = tooct((ch & 0x00000007));
						encoded_text_size = 12;
					}
					goto print_encoded;
				}
special_control:
				switch (ch) {
				case '\a': ch = 'a'; break;
				case '\b': ch = 'b'; break;
				case '\f': ch = 'f'; break;
				case '\n': ch = 'n'; break;
				case '\r': ch = 'r'; break;
				case '\t': ch = 't'; break;
				case '\v': ch = 'v'; break;
				case '\033': ch = 'e'; break;
				case '\\':
				case '\'':
				case '\"': break;
				default: goto default_ctrl;
				}
				encoded_text[1]   = (char)ch;
				encoded_text_size = 2;
				goto print_encoded;
			} else if ((ch == '\\' || ch == '\'' || ch == '\"') &&
			           !(flags & FORMAT_QUOTE_FNOCTRL)) {
				goto special_control;
			} else {
				/* Non-ascii character. */
/*default_nonascii:*/
				if (flags & FORMAT_QUOTE_FFORCEOCT)
					goto encode_oct;
encode_hex:
				if (text < textend) {
					char const *new_text = text;
					uint32_t next_ch     = utf8_readchar((char const **)&new_text, textend);
					struct unitraits *desc;
					if ((next_ch >= 'a' && next_ch <= 'f') ||
					    (next_ch >= 'A' && next_ch <= 'F'))
						goto encode_uni;
					desc = DeeUni_Descriptor(next_ch);
					if ((desc->ut_flags & UNICODE_FDECIMAL) &&
					    (desc->ut_digit < 10))
						goto encode_uni;
				}
				if (ch <= 0xf) {
					encoded_text[1]   = 'x';
					encoded_text[2]   = c_hex[ch];
					encoded_text_size = 3;
				} else if (ch <= 0xff) {
					encoded_text[1]   = 'x';
					encoded_text[2]   = c_hex[(ch & 0x000000f0) >> 4];
					encoded_text[3]   = c_hex[ch & 0x0000000f];
					encoded_text_size = 4;
				} else {
encode_uni:
					if (ch <= 0xffff) {
						encoded_text[1]   = 'u';
						encoded_text[2]   = c_hex[(ch & 0x0000f000) >> 12];
						encoded_text[3]   = c_hex[(ch & 0x00000f00) >> 8];
						encoded_text[4]   = c_hex[(ch & 0x000000f0) >> 4];
						encoded_text[5]   = c_hex[ch & 0x0000000f];
						encoded_text_size = 6;
					} else {
						encoded_text[1]   = 'U';
						encoded_text[2]   = c_hex[(ch & 0xf0000000) >> 28];
						encoded_text[3]   = c_hex[(ch & 0x0f000000) >> 24];
						encoded_text[4]   = c_hex[(ch & 0x00f00000) >> 20];
						encoded_text[5]   = c_hex[(ch & 0x000f0000) >> 16];
						encoded_text[6]   = c_hex[(ch & 0x0000f000) >> 12];
						encoded_text[7]   = c_hex[(ch & 0x00000f00) >> 8];
						encoded_text[8]   = c_hex[(ch & 0x000000f0) >> 4];
						encoded_text[9]   = c_hex[ch & 0x0000000f];
						encoded_text_size = 10;
					}
				}
print_encoded:
				print(encoded_text, encoded_text_size);
				goto next;
			}
		} else {
			unsigned char print_ch[1];
			print_ch[0] = (unsigned char)ch;
			print((char *)print_ch, 1);
		}
next:
		;
	}
/*done:*/
	if (!(flags & FORMAT_QUOTE_FPRINTRAW))
		print("\"", 1);
	return result;
err:
	return temp;
}


/* Repeat the given `ch' a total of `count' times. */
PUBLIC WUNUSED NONNULL((1)) dssize_t DCALL
DeeFormat_Repeat(/*ascii*/ dformatprinter printer, void *arg,
                 /*ascii*/ char ch, size_t count) {
	char buffer[128];
	dssize_t temp, result;
	if (count <= sizeof(buffer)) {
		memset(buffer, ch, count);
		return (*printer)(arg, buffer, count);
	}
	result = 0;
	memset(buffer, ch, sizeof(buffer));
	while (count) {
		size_t part = MIN(count, sizeof(buffer));
		temp        = (*printer)(arg, buffer, part);
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
DFUNDEF WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeFormat_RepeatUtf8(dformatprinter printer, void *arg,
                     /*utf-8*/ char const *__restrict str,
                     size_t length, size_t total_characters) {
	size_t utf8_length, i;
	dssize_t temp, result;
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
		ch = utf8_sequence_len[ch];
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
			uint8_t len;
			len = utf8_sequence_len[(uint8_t)str[i]];
			if unlikely(!len)
				len = 1;
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




PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DPRINTER_CC
sprintf_callback(char **__restrict pbuffer,
                 char const *__restrict data, size_t datalen) {
	memcpyc(*pbuffer, data, datalen, sizeof(char));
	*pbuffer += datalen;
	return (dssize_t)datalen;
}

struct snprintf_data {
	char  *buf;
	size_t siz;
};

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DPRINTER_CC
snprintf_callback(struct snprintf_data *__restrict arg,
                  char const *__restrict data, size_t datalen) {
	if (arg->siz) {
		size_t copy_size;
		copy_size = arg->siz < datalen ? arg->siz : datalen;
		memcpyc(arg->buf, data, copy_size, sizeof(char));
		arg->siz -= copy_size;
	}
	arg->buf += datalen;
	return (dssize_t)datalen;
}

PUBLIC NONNULL((1, 2)) char *DCALL
Dee_vsprintf(char *__restrict buffer,
             char const *__restrict format, va_list args) {
	if unlikely(DeeFormat_VPrintf((dformatprinter)&sprintf_callback,
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
	if unlikely(DeeFormat_VPrintf((dformatprinter)&snprintf_callback,
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

/* Print the object types passed by the given argument list.
 * If given, also include keyword names & types from `kw'
 * >> foo(10, 1.0, "bar", enabled: true);
 * Printed: "int, float, string, enabled: bool" */
PUBLIC WUNUSED NONNULL((1)) dssize_t
(DCALL DeeFormat_PrintArgumentTypesKw)(dformatprinter printer, void *arg,
                                       size_t argc, DeeObject *const *argv,
                                       DeeObject *kw) {
	size_t i;
	char const *str;
	dssize_t temp, result = 0;
	if (kw && DeeKwds_Check(kw)) {
		size_t kwargc = DeeKwds_SIZE(kw);
		struct kwds_entry *entry;
		if unlikely(kwargc > argc) {
			kwargc = argc;
		} else {
			for (i = 0; i < argc - kwargc; ++i) {
				if (i != 0) {
					temp = (*printer)(arg, ", ", 2);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				str = Dee_TYPE(argv[i])->tp_name;
				if unlikely(!str)
					str = "<anonymous_type>";
				temp = (*printer)(arg, str, strlen(str));
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
		}
		for (i = 0; i < kwargc; ++i) {
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
			str = Dee_TYPE(argv[argc - kwargc + i])->tp_name;
			if unlikely(!str)
				str = "<anonymous_type>";
			temp = (*printer)(arg, str, strlen(str));
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		goto done;
	}
	for (i = 0; i < argc; ++i) {
		if (i != 0) {
			temp = (*printer)(arg, ", ", 2);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		str = Dee_TYPE(argv[i])->tp_name;
		if unlikely(!str)
			str = "<anonymous_type>";
		temp = (*printer)(arg, str, strlen(str));
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	if (kw) {
		/* Print keyword passed via a generic mappings. */
		DREF DeeObject *iter, *elem, *pair[2];
		iter = DeeObject_IterSelf(kw);
		if unlikely(!iter)
			goto err_m1;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			int error;
			if (argc != 0) {
				temp = (*printer)(arg, ", ", 2);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			argc  = 1;
			error = DeeObject_Unpack(elem, 2, pair);
			Dee_Decref(elem);
			if unlikely(error) {
err_iter:
				Dee_Decref(iter);
				goto err_m1;
			}
			temp = DeeObject_Print(pair[0], printer, arg);
			Dee_Decref(pair[0]);
			if unlikely(temp < 0) {
err_pair1_iter_temp:
				Dee_Decref(pair[1]);
err_iter_temp:
				Dee_Decref(iter);
				goto err;
			}
			result += temp;
			temp = (*printer)(arg, ": ", 2);
			if unlikely(temp < 0)
				goto err_pair1_iter_temp;
			result += temp;
			str = Dee_TYPE(pair[1])->tp_name;
			if unlikely(!str)
				str = "<anonymous_type>";
			temp = (*printer)(arg, str, strlen(str));
			Dee_Decref(pair[1]);
			if unlikely(temp < 0)
				goto err_iter_temp;
			result += temp;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
done:
	return result;
err:
	return temp;
err_m1:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_FORMAT_C */
