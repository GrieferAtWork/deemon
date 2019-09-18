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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_CFORMAT_C
#define GUARD_DEEMON_OBJECTS_UNICODE_CFORMAT_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>

#include <string.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

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

union integral {
	int64_t  int_s64;
	uint64_t int_u64;
#ifdef __INTELLISENSE__
	int32_t  int_s32;
	uint32_t int_u32;
	int16_t  int_s16;
	uint16_t int_u16;
	int8_t   int_s8;
	uint8_t  int_u8;
#else /* __INTELLISENSE__ */
	int32_t  int_s32_v[2];
	uint32_t int_u32_v[2];
	int16_t  int_s16_v[4];
	uint16_t int_u16_v[4];
	int8_t   int_s8_v[8];
	uint8_t  int_u8_v[8];
#ifdef CONFIG_LITTLE_ENDIAN
#define int_s32 int_s32_v[0]
#define int_s16 int_s16_v[0]
#define int_s8  int_s8_v[0]
#define int_u32 int_u32_v[0]
#define int_u16 int_u16_v[0]
#define int_u8  int_u8_v[0]
#else /* CONFIG_LITTLE_ENDIAN */
#define int_s32 int_s32_v[1]
#define int_s16 int_s16_v[3]
#define int_s8  int_s8_v[7]
#define int_u32 int_u32_v[1]
#define int_u16 int_u16_v[3]
#define int_u8  int_u8_v[7]
#endif /* !CONFIG_LITTLE_ENDIAN */
#endif /* !__INTELLISENSE__ */
};


PRIVATE char const decimals[2][17] = {
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'x' },
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'X' },
};

PRIVATE DEFINE_STRING(str_lpnullrp,"(null)");


PRIVATE dssize_t DCALL
print_repr_precision(DeeObject *__restrict self, size_t length,
                     dformatprinter printer,
                     void *arg, unsigned int flags) {
	void *str = DeeString_WSTR(self);
	ASSERT(length <= WSTR_LENGTH(str));
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		return DeeFormat_Quote8(printer, arg, (uint8_t *)str, length, flags);

	CASE_WIDTH_2BYTE:
		return DeeFormat_Quote16(printer, arg, (uint16_t *)str, length, flags);

	CASE_WIDTH_4BYTE:
		return DeeFormat_Quote32(printer, arg, (uint32_t *)str, length, flags);
	}
}


/* Format a given `format' string subject to printf-style formatting rules.
 * NOTE: This is the function called by `operator %' for strings. */
INTERN dssize_t DCALL
DeeString_CFormat(dformatprinter printer,
                  dformatprinter format_printer, void *arg,
                  /*utf-8*/ char const *__restrict format, size_t format_len,
                  size_t argc, DeeObject **__restrict argv) {
	char *iter, *end, *flush_start;
	DeeObject *in_arg;
	dssize_t temp, result = 0;
#define print(p, s)                                     \
	do {                                                \
		if unlikely((temp = (*printer)(arg, p, s)) < 0) \
			goto err;                                   \
		result += temp;                                 \
	} __WHILE0
#define print_format(p, s)                                     \
	do {                                                       \
		if unlikely((temp = (*format_printer)(arg, p, s)) < 0) \
			goto err;                                          \
		result += temp;                                        \
	} __WHILE0
#define printob(ob)                                                 \
	do {                                                            \
		if unlikely((temp = DeeObject_Print(ob, printer, arg)) < 0) \
			goto err;                                               \
		result += temp;                                             \
	} __WHILE0
#define printf(...)                                                           \
	do {                                                                      \
		if unlikely((temp = DeeFormat_Printf(printer, arg, __VA_ARGS__)) < 0) \
			goto err;                                                         \
		result += temp;                                                       \
	} __WHILE0
#define GETARG()                   \
	do {                           \
		if unlikely(!argc)         \
			goto missing_argument; \
		in_arg = *argv++;          \
		--argc;                    \
	} __WHILE0
	ASSERT(!argc || argv);
	end = (iter = flush_start = (char *)format) + format_len;
	while (iter != end) {
		size_t width, precision;
		uint16_t flags;
		char *format_start;
		char ch = *iter++;
		if (ch != '%')
			continue;
		/* Flush everything up to this point. */
		print_format(flush_start, (size_t)((iter - 1) - flush_start));
		format_start = iter;
		flags        = 0;
		width        = 0;
		precision    = 0;
	next_spec:
		ch = *iter++;
		switch (ch) {
		case '%': /* %% --> % */
			flush_start = iter - 1;
			continue;

			/* Flag characters. */
		case '-':
			if (flags & F_LJUST)
				goto invalid_format;
			flags |= F_LJUST;
			goto next_spec;

		case '+':
			if (flags & F_SIGN)
				goto invalid_format;
			flags |= F_SIGN;
			goto next_spec;

		case ' ':
			if (flags & F_SPACE)
				goto invalid_format;
			flags |= F_SPACE;
			goto next_spec;

		case '0':
			if (flags & F_PADZERO)
				goto invalid_format;
			flags |= F_PADZERO;
			goto next_spec;

		case '#':
			if (flags & F_PREFIX)
				goto invalid_format;
			flags |= F_PREFIX;
			goto next_spec;

		case '$':
			/* Fixed-width precision. */
			if (flags & (F_HASPREC | F_FIXBUF))
				goto invalid_format;
			flags |= (F_HASPREC | F_FIXBUF);
			GETARG();
			if (DeeObject_AsSize(in_arg, &precision))
				goto err_m1;
			goto next_spec;

		case ':':
			if (flags & (F_HASPREC | F_FIXBUF))
				goto invalid_format;
			flags |= (F_HASPREC | F_FIXBUF);
			goto do_prec_spec;

		case '.':
			/* Precision. */
			if (flags & F_HASPREC)
				goto invalid_format;
			flags |= F_HASPREC;
		do_prec_spec:
			ch = *iter++;
			if (ch == '*') {
				unsigned int temp;
				GETARG();
				if (DeeObject_AsUInt(in_arg, &temp))
					goto err_m1;
				precision = (size_t)temp;
			} else if (ch == '?') {
				GETARG();
				if (DeeObject_AsSize(in_arg, &precision))
					goto err_m1;
			} else if (ch >= '0' && ch <= '9') {
				precision = ch - '0';
				while ((ch = *iter, ch >= '0' && ch <= '9'))
					precision *= 10, precision += ch - '0', ++iter;
			} else {
				goto invalid_format;
			}
			goto next_spec;

		case '*': {
			unsigned int temp;
			/* Width. */
			if (flags & F_HASWIDTH)
				goto invalid_format;
			flags |= F_HASWIDTH;
			GETARG();
			if (DeeObject_AsUInt(in_arg, &temp))
				goto err_m1;
			width = (size_t)temp;
		}	goto next_spec;

			/* Width. */
		case '?':
			if (flags & F_HASWIDTH)
				goto invalid_format;
			flags |= F_HASWIDTH;
			GETARG();
			if (DeeObject_AsSize(in_arg, &width))
				goto err_m1;
			goto next_spec;

		case 'L':
			goto next_spec; /* Ignored (for now...) */


		{
			unsigned int length, radix, addend; /* integer. */
			union integral intval;
			size_t printsize;
			char const *dec;
			bool is_neg;
			char *bufiter, buffer[67]; /* 64-bit binary, w/prefix + sign */

		/* Integer without length specific. */
		case 'b':
		case 'o':
		case 'u':
		case 'd':
		case 'i':
		case 'x':
		case 'X':
			length = 4;
			--iter;
			goto do_length_integer;

		case 'p':
			length = sizeof(void *);
			--iter;
			goto do_length_integer;

		/* Various integer length specifiers. */
		case 'h':
			length = 2;
			if (*iter == 'h')
				++iter, length = 1;
			goto do_length_integer;

		case 'l':
			length = 4;
			if (*iter == 'l')
				++iter, length = 8;
			goto do_length_integer;

		case 'I':
			length = sizeof(size_t);
			if (*iter == '8')
				++iter, length = 1;
			else if (*iter == '1') {
				if (iter[1] != '6')
					goto invalid_format;
				iter += 2, length = 2;
			} else if (*iter == '3') {
				if (iter[1] != '2')
					goto invalid_format;
				iter += 2, length = 4;
			} else if (*iter == '6') {
				if (iter[1] != '4')
					goto invalid_format;
				iter += 2, length = 8;
			}
			goto do_length_integer;

		case 't':
			flags |= F_SIGNED;
			ATTR_FALLTHROUGH
		case 'z':
			length = sizeof(size_t);
		do_length_integer:
			switch (*iter) {

			case 'b':
				radix = 2;
				break;

			case 'o':
				radix = 8;
				break;

			case 'u':
				radix = 10;
				break;

			case 'd':
			case 'i':
				radix = 10;
				if (ch != 'z')
					flags |= F_SIGNED;
				break;

			case 'p':
				if (!(flags & F_HASPREC)) {
					precision = sizeof(void *) * 2;
					flags |= F_HASPREC;
				}
				ATTR_FALLTHROUGH
			case 'X':
				flags |= F_UPPER;
				ATTR_FALLTHROUGH
			case 'x':
				radix = 16;
				break;

			default:
				ch = *iter;
				goto invalid_format;
			}
			++iter;
			GETARG();
			/* Clear out all bits that won't be used. */
			intval.int_u64 = 0;
			/* Load the argument value. */
			if (length == 4) {
				if (flags & F_SIGNED) {
					if (DeeObject_AsInt32(in_arg, &intval.int_s32))
						goto err_m1;
					intval.int_s64 = (int64_t)intval.int_s32; /* sign-extend */
				} else {
					if (DeeObject_AsUInt32(in_arg, &intval.int_u32))
						goto err_m1;
				}
			} else if (length == 8) {
				if (flags & F_SIGNED ? DeeObject_AsInt64(in_arg, &intval.int_s64)
				                     : DeeObject_AsUInt64(in_arg, &intval.int_u64))
					goto err_m1;
			} else if (length == 2) {
				if (flags & F_SIGNED) {
					if (DeeObject_AsInt16(in_arg, &intval.int_s16))
						goto err_m1;
					intval.int_s64 = (int64_t)intval.int_s16; /* sign-extend */
				} else {
					if (DeeObject_AsUInt16(in_arg, &intval.int_u16))
						goto err_m1;
				}
			} else {
				if (flags & F_SIGNED) {
					if (DeeObject_AsInt8(in_arg, &intval.int_s8))
						goto err_m1;
					intval.int_s64 = (int64_t)intval.int_s8; /* sign-extend */
				} else {
					if (DeeObject_AsUInt8(in_arg, &intval.int_u8))
						goto err_m1;
				}
			}
			/* Now we just need to print this integer. */
#if F_UPPER == 1
			dec = decimals[flags & F_UPPER];
#else /* F_UPPER == 1 */
			dec = decimals[!!(flags & F_UPPER)];
#endif /* F_UPPER != 1 */
			bufiter = COMPILER_ENDOF(buffer);
			is_neg  = false;
			if (flags & F_SIGNED && intval.int_s64 < 0) {
				is_neg         = true;
				intval.int_s64 = -intval.int_s64;
			}
			/* Actually translate the given input integer. */
			do {
				*--bufiter = dec[intval.int_u64 % radix];
			} while ((intval.int_u64 /= radix) != 0);
			addend = 0;
			if (flags & F_PREFIX && radix != 10) {
				if (radix == 16) {
					++addend;
					*--bufiter = dec[16]; /* X/x */
				} else if (radix == 2) {
					++addend;
					*--bufiter = dec[11]; /* B/b */
				}
				++addend, *--bufiter = '0';
			}
			if (is_neg) {
				++addend;
				*--bufiter = '-';
			} else if (flags & F_SIGN) {
				++addend;
				*--bufiter = '+';
			}
			printsize = (size_t)(COMPILER_ENDOF(buffer) - bufiter);
			if (printsize < precision + addend)
				printsize = precision + addend;
			if (width > printsize && !(flags & F_LJUST)) {
				temp = DeeFormat_Repeat(format_printer, arg, ' ', width - printsize);
				if
					unlikely(temp < 0)
				goto err;
				result += temp;
			}
			printsize = (size_t)(COMPILER_ENDOF(buffer) - bufiter) - addend;
			if (precision > printsize) {
				/* Print the leading addend. */
				print_format(bufiter, addend);
				bufiter += addend;
				temp = DeeFormat_Repeat(format_printer, arg, '0',
				                        precision - printsize);
				if
					unlikely(temp < 0)
				goto err;
				result += temp;
				print_format(bufiter, printsize);
			} else {
				printsize += addend;
				print_format(bufiter, printsize);
			}
			if (width > printsize && (flags & F_LJUST)) {
				temp = DeeFormat_Repeat(format_printer, arg, ' ', width - printsize);
				if
					unlikely(temp < 0)
				goto err;
				result += temp;
			}
		}	break;

		case 's':
		case 'q': {
			size_t str_length;
			GETARG();
			if (DeeNone_Check(in_arg))
				in_arg = (DeeObject *)&str_lpnullrp;
			else if (DeeObject_AssertTypeExact(in_arg, &DeeString_Type)) {
				goto err_m1;
			}
			str_length = DeeString_WLEN(in_arg);
			if (flags & F_HASPREC) {
				if (str_length > precision)
					str_length = precision;
				else if (flags & F_FIXBUF) {
					DeeError_Throwf(&DeeError_ValueError,
					                "String argument is too short (%Iu characters) "
					                "for fixed buffer length %Iu",
					                precision);
					goto err_m1;
				}
			}

			if (width <= str_length || ch == 's' || (flags & F_LJUST)) {
				if (width > str_length && !(flags & F_LJUST)) {
					temp = DeeFormat_Repeat(format_printer, arg, ' ',
					                        width - str_length);
					if
						unlikely(temp < 0)
					goto err;
					result += temp;
				}
				if (ch == 'q') {
					temp = print_repr_precision(in_arg,
					                            str_length,
					                            printer,
					                            arg,
#if F_PREFIX == FORMAT_QUOTE_FPRINTRAW
					                            flags & F_PREFIX
#else /* F_PREFIX == FORMAT_QUOTE_FPRINTRAW */
					                            flags & F_PREFIX
					                            ? FORMAT_QUOTE_FPRINTRAW
					                            : FORMAT_QUOTE_FNORMAL
#endif /* F_PREFIX != FORMAT_QUOTE_FPRINTRAW */
					                            );
					if
						unlikely(temp < 0)
					goto err;
					result += temp;
					str_length = (size_t)temp; /* Fir ljust width-strings. */
				} else {
					temp = DeeString_PrintUtf8(in_arg, printer, arg);
					if
						unlikely(temp < 0)
					goto err;
					result += temp;
				}
				if (width > str_length && (flags & F_LJUST)) {
					temp = DeeFormat_Repeat(format_printer, arg, ' ', width - str_length);
					if
						unlikely(temp < 0)
					goto err;
					result += temp;
				}
			} else {
				/* Special case: rjust-quote with width.
				 * For this case, we must pre-generate the quoted
				 * string so we can know it's exact length. */
				struct unicode_printer subprinter = UNICODE_PRINTER_INIT;
				temp = print_repr_precision(in_arg,
				                            str_length,
				                            &unicode_printer_print,
				                            &subprinter,
#if F_PREFIX == FORMAT_QUOTE_FPRINTRAW
				                            flags & F_PREFIX
#else /* F_PREFIX == FORMAT_QUOTE_FPRINTRAW */
				                            flags & F_PREFIX
				                            ? FORMAT_QUOTE_FPRINTRAW
				                            : FORMAT_QUOTE_FNORMAL
#endif /* F_PREFIX != FORMAT_QUOTE_FPRINTRAW */
				                            );
				if
					unlikely(temp < 0)
				{
				err_subprinter:
					unicode_printer_fini(&subprinter);
					goto err;
				}
				/* Print leading spaces. */
				temp = DeeFormat_Repeat(format_printer, arg, ' ',
				                        width -
				                        UNICODE_PRINTER_LENGTH(&subprinter));
				if
					unlikely(temp < 0)
				goto err_subprinter;
				result += temp;
				/* Print the substring. */
				temp = unicode_printer_printinto(&subprinter, printer, arg);
				if
					unlikely(temp < 0)
				goto err_subprinter;
				result += temp;
				unicode_printer_fini(&subprinter);
			}
		} break;

		case 'c': {
			uint32_t uch;
			GETARG();
			if (DeeString_Check(in_arg)) {
				if (DeeString_WLEN(in_arg) != 1) {
					err_expected_single_character_string(in_arg);
					goto err_m1;
				}
				uch = DeeString_GetChar(in_arg, 0);
			} else {
				if (DeeObject_AsUInt32(in_arg, &uch))
					goto err_m1;
			}
			/* Print the user-character. */
			if (width > 1 && !(flags & F_LJUST)) {
				temp = DeeFormat_Repeat(format_printer, arg, ' ', width - 1);
				if
					unlikely(temp < 0)
				goto err;
				result += temp;
			}
			temp = DeeFormat_Putc(printer, arg, uch);
			if
				unlikely(temp < 0)
			goto err;
			result += temp;
			if (width > 1 && (flags & F_LJUST)) {
				temp = DeeFormat_Repeat(format_printer, arg, ' ', width - 1);
				if
					unlikely(temp < 0)
				goto err;
				result += temp;
			}
		} break;

		/* Print native objects. */
		case 'R': ch = 'r'; ATTR_FALLTHROUGH
		case 'k':
		case 'K':
		case 'O':
		case 'r':
			GETARG();
			if (width) {
				/* Must pre-print the object, so we can know its length and can properly insert spacings. */
				struct unicode_printer preprinter = UNICODE_PRINTER_INIT;
				size_t preprinter_length;
				temp = ch == 'r'
				       ? DeeObject_PrintRepr(in_arg,
				                             &unicode_printer_print,
				                             &preprinter)
				       : DeeObject_Print(in_arg,
				                         &unicode_printer_print,
				                         &preprinter);
				if
					unlikely(temp < 0)
				{
				err_preprinter:
					unicode_printer_fini(&preprinter);
					goto err;
				}
				preprinter_length = UNICODE_PRINTER_LENGTH(&preprinter);
				if (width > preprinter_length && !(flags & F_LJUST)) {
					temp = DeeFormat_Repeat(format_printer, arg, ' ', width - preprinter_length);
					if
						unlikely(temp < 0)
					goto err_preprinter;
					result += temp;
				}
				temp = unicode_printer_printinto(&preprinter, printer, arg);
				unicode_printer_fini(&preprinter);
				if
					unlikely(temp < 0)
				goto err_preprinter;
				result += temp;
				if (width > preprinter_length && (flags & F_LJUST)) {
					temp = DeeFormat_Repeat(format_printer, arg, ' ', width - preprinter_length);
					if
						unlikely(temp < 0)
					goto err_preprinter;
					result += temp;
				}
			} else if (ch == 'r') {
				temp = DeeObject_PrintRepr(in_arg, printer, arg);
				if
					unlikely(temp < 0)
				goto err;
				result += temp;
			} else {
				temp = DeeObject_Print(in_arg, printer, arg);
				if
					unlikely(temp < 0)
				goto err;
				result += temp;
			}
			break;

		case 'f':
		case 'F':
		case 'e':
		case 'E':
		case 'g':
		case 'G': {
			double value;
			GETARG();
			if (DeeObject_AsDouble(in_arg, &value))
				goto err;
#if F_LJUST == DEEFLOAT_PRINT_FLJUST && \
F_SIGN == DEEFLOAT_PRINT_FSIGN &&       \
F_SPACE == DEEFLOAT_PRINT_FSPACE &&     \
F_PADZERO == DEEFLOAT_PRINT_FPADZERO && \
F_HASWIDTH == DEEFLOAT_PRINT_FWIDTH &&  \
F_HASPREC == DEEFLOAT_PRINT_FPRECISION
			temp = DeeFloat_Print(value, printer, arg, width, precision, flags);
#else
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
#endif
			if
				unlikely(temp < 0)
			goto err;
			result += temp;
		} break;

		default:
			if ((ch >= '0' && ch <= '9') &&
			    !(flags & F_HASWIDTH)) {
				width = ch - '0';
				while ((ch = *iter, ch >= '0' && ch <= '9'))
					width *= 10, width += ch - '0', ++iter;
				flags |= F_HASWIDTH;
				goto next_spec;
			}

		invalid_format:
			DeeError_Throwf(&DeeError_ValueError,
			                "Unknown or unexpected cformat character `%c' in `%$s', apart of %r",
			                ch, (size_t)(iter - format_start), format_start, format);
			goto err_m1;
		missing_argument:
			DeeError_Throwf(&DeeError_ValueError,
			                "Missing argument for `%$s'",
			                (size_t)(iter - format_start), format_start);
			goto err_m1;
		}
		flush_start = iter;
	}
	/* Error if arguments remained unused. */
	if
		unlikely(argc)
	{
		DeeError_Throwf(&DeeError_ValueError,
		                "%Iu arguments were unused", argc);
		goto err_m1;
	}
	/* Flush the remainder. */
	print_format(flush_start, (size_t)(iter - flush_start));
	return result;
#undef printf
#undef print
err_m1:
	temp = -1;
err:
	return temp;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_CFORMAT_C */
