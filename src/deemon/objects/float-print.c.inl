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
#ifdef __INTELLISENSE__
#include "float.c"
#define PRINT_LONG_DOUBLE 1
#endif

#include <deemon/format.h>

#include <math.h> /* FIXME: This needs a feature check! */

/* Print a string representation of the given floating point value.
 * @param: flags: Set of `DEEFLOAT_PRINT_F*' */
#ifdef PRINT_LONG_DOUBLE
#define FLOAT_TYPE  long double
PUBLIC WUNUSED NONNULL((2)) dssize_t DCALL
DeeFloat_LPrint(long double value, dformatprinter printer, void *arg,
                size_t width, size_t precision, unsigned int flags)
#else /* PRINT_LONG_DOUBLE */
#define FLOAT_TYPE  double
PUBLIC WUNUSED NONNULL((2)) dssize_t DCALL
DeeFloat_Print(double value, dformatprinter printer, void *arg,
               size_t width, size_t precision, unsigned int flags)
#endif /* !PRINT_LONG_DOUBLE */
{
	dssize_t temp, result = 0;
	PRIVATE FLOAT_TYPE const pow10[10] = {
		1, 10, 100, 1000, 10000, 100000, 1000000,
		10000000, 100000000, 1000000000
	};
	FLOAT_TYPE tmpval, diff;
	char buf[32]; /* Must be able to hold a decimal-encoded UINT64_MAX +1 more character */
	size_t len, total_len;
	bool is_negative;
	unsigned int max_prec, min_prec;
	uintmax_t whole, frac;
#if 1
	if (isinf(value)) {
		buf[1] = 'I';
		buf[2] = 'N';
		buf[3] = 'F';
do_special_float:
		total_len   = 3;
		is_negative = false;
		if (value < 0)
			is_negative = true;
		if (is_negative || (flags & (DEEFLOAT_PRINT_FSIGN | DEEFLOAT_PRINT_FSPACE)))
			++total_len;
		if ((flags & (DEEFLOAT_PRINT_FWIDTH | DEEFLOAT_PRINT_FLJUST)) ==
		    (DEEFLOAT_PRINT_FWIDTH | DEEFLOAT_PRINT_FLJUST) &&
		    (width > total_len)) {
			temp = DeeFormat_Repeat(printer, arg, ' ', width - total_len);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		if (total_len == 4) {
			if (is_negative)
				buf[0] = '-';
			else if (flags & DEEFLOAT_PRINT_FSIGN)
				buf[0] = '+';
			else {
				buf[0] = ' ';
			}
			temp = (*printer)(arg, buf, 4);
		} else {
			temp = (*printer)(arg, buf + 1, 3);
		}
		if unlikely(temp < 0)
			goto err;
		result += temp;
		if ((flags & (DEEFLOAT_PRINT_FWIDTH | DEEFLOAT_PRINT_FLJUST)) ==
		    (DEEFLOAT_PRINT_FWIDTH) &&
		    (width > total_len)) {
			temp = DeeFormat_Repeat(printer, arg, ' ', width - total_len);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		return result;
	} else if (isnan(value)) {
		buf[1] = 'N';
		buf[2] = 'a';
		buf[3] = 'N';
		goto do_special_float;
	}
#endif /* Special floating point values... */
	is_negative = false;
	if (value < 0) {
		is_negative = true;
		value       = -value;
	}
	/* Determine the intended precision. */
	max_prec = (unsigned int)precision;
	min_prec = (unsigned int)precision;
	if (!(flags & DEEFLOAT_PRINT_FPRECISION))
		max_prec = 6, min_prec = 0;
	else if (max_prec > 9)
		max_prec = min_prec = 9;
	/* XXX: This cast can overflow */
	whole  = (uintmax_t)value;
	tmpval = (value - whole) * pow10[max_prec];
	frac   = (uintmax_t)tmpval;
	diff   = tmpval - frac;
	/* Round to the closest fraction. */
	if (diff > 0.5) {
		++frac;
		if (frac > pow10[max_prec]) {
			frac = 0;
			++whole;
		}
	} else if (diff == 0.5 && (frac == 0 || frac & 1)) {
		++frac;
	}
	/* Special case: no fraction wanted. - Round the whole-part. */
	if (max_prec == 0) {
		diff = value - (FLOAT_TYPE)whole;
		if (diff > 0.5) {
			++whole;
		} else if (diff == 0.5 && (whole & 1)) {
			++whole;
		}
	}
	/* Print the whole part. */
	len = COMPILER_LENOF(buf);
	for (;;) {
		buf[--len] = (char)(48 + (whole % 10));
		whole /= 10;
		if (!whole)
			break;
	}
	/* Trim unused fraction digits. (should precision or
	 * width require them, they'll be re-added later) */
	while (frac && (frac % 10) == 0)
		frac /= 10;
	total_len = COMPILER_LENOF(buf) - len;
	if ((flags & (DEEFLOAT_PRINT_FWIDTH | DEEFLOAT_PRINT_FLJUST)) ==
	    (DEEFLOAT_PRINT_FWIDTH | DEEFLOAT_PRINT_FLJUST)) {
		/* Pad with with leading zeroes. */
		if (is_negative || (flags & (DEEFLOAT_PRINT_FSIGN | DEEFLOAT_PRINT_FSPACE)))
			++total_len;
		if (max_prec != 0) {
			unsigned int temp_min = min_prec;
			whole                 = frac;
			++total_len; /* . */
			for (;;) {
				if (temp_min)
					--temp_min;
				++total_len;
				whole /= 10;
				if (!whole)
					break;
			}
			total_len += min_prec;
		}
		if (width <= total_len)
			goto do_float_normal_width;
		if (!(flags & DEEFLOAT_PRINT_FPADZERO)) {
			temp = DeeFormat_Repeat(printer, arg, ' ', width - total_len);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			goto do_float_normal_width;
		}
		if (is_negative || (flags & (DEEFLOAT_PRINT_FSIGN | DEEFLOAT_PRINT_FSPACE))) {
			/* print the sign */
			buf[0] = is_negative ? '-' : (flags & DEEFLOAT_PRINT_FSIGN) ? '+' : ' ';
			temp   = (*printer)(arg, buf, 1);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		/* Insert leading zeroes for padding. */
		temp = DeeFormat_Repeat(printer, arg, '0', width - total_len);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	} else {
do_float_normal_width:
		if (is_negative)
			buf[--len] = '-';
		else if (flags & DEEFLOAT_PRINT_FSIGN)
			buf[--len] = '+';
		else if (flags & DEEFLOAT_PRINT_FSPACE)
			buf[--len] = ' ';
	}
	temp = (*printer)(arg, buf + len, COMPILER_LENOF(buf) - len);
	if unlikely(temp < 0)
		goto err;
	result += temp;

	/* Fractional part. */
	if (max_prec != 0) {
		len = COMPILER_LENOF(buf);
		for (;;) {
			if (min_prec)
				--min_prec;
			buf[--len] = (char)(48 + (frac % 10));
			frac /= 10;
			if (!frac)
				break;
		}
		buf[--len] = '.';
		temp       = (*printer)(arg, buf + len, COMPILER_LENOF(buf) - len);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		total_len += COMPILER_LENOF(buf) - len;
		if (min_prec) {
			temp = DeeFormat_Repeat(printer, arg, '0', min_prec);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			total_len += min_prec;
		}
	}
	if ((flags & (DEEFLOAT_PRINT_FWIDTH | DEEFLOAT_PRINT_FLJUST)) ==
	    (DEEFLOAT_PRINT_FWIDTH) &&
	    (width > total_len)) {
		/* Insert a missing decimal separator. */
		if (flags & DEEFLOAT_PRINT_FPADZERO && max_prec == 0) {
			buf[0] = '.';
			temp   = (*printer)(arg, buf, 1);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			--total_len;
		}
		/* Add trailing zeroes to pad out our length the requested width. */
		temp = DeeFormat_Repeat(printer, arg,
		                        flags & DEEFLOAT_PRINT_FPADZERO ? '0' : ' ',
		                        width - total_len);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
#undef FLOAT_TYPE
	return result;
err:
	return temp;
}


#undef FLOAT_TYPE
#undef PRINT_LONG_DOUBLE
