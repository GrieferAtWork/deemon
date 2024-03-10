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
#ifdef __INTELLISENSE__
#include "float.c"
#define DEFINE_DeeFloat_LPrint
//#define DEFINE_DeeFloat_Print
#endif

#if (defined(DEFINE_DeeFloat_LPrint) + \
     defined(DEFINE_DeeFloat_Print)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#include <deemon/format.h>
#include <deemon/system-features.h>

#ifdef CONFIG_HAVE_MATH_H
#include <math.h>
#endif /* CONFIG_HAVE_MATH_H */

/* Print a string representation of the given floating point value.
 * @param: flags: Set of `DEEFLOAT_PRINT_F*' */
#ifdef DEFINE_DeeFloat_LPrint
#define LOCAL_float_t __LONGDOUBLE
PUBLIC WUNUSED NONNULL((2)) dssize_t DCALL
DeeFloat_LPrint(LOCAL_float_t value, dformatprinter printer, void *arg,
                size_t width, size_t precision, unsigned int flags)
#else /* DEFINE_DeeFloat_LPrint */
#define LOCAL_float_t double
PUBLIC WUNUSED NONNULL((2)) dssize_t DCALL
DeeFloat_Print(LOCAL_float_t value, dformatprinter printer, void *arg,
               size_t width, size_t precision, unsigned int flags)
#endif /* !DEFINE_DeeFloat_LPrint */
{
	dssize_t temp, result = 0;
	PRIVATE LOCAL_float_t const pow10[10] = {
		1, 10, 100, 1000, 10000, 100000, 1000000,
		10000000, 100000000, 1000000000
	};
	LOCAL_float_t tmpval, diff;
	char buf[32]; /* Must be able to hold a decimal-encoded UINT64_MAX +1 more character */
	size_t len, total_len;
	bool is_negative;
	unsigned int max_prec, min_prec;
	uintmax_t whole, frac;

	/* Check for INF */
#ifdef CONFIG_HAVE_isinf
	if (isinf(value)) {
		buf[1] = 'I';
		buf[2] = 'N';
		buf[3] = 'F';
		goto do_special_float;
#define WANT_do_special_float
	}
#endif /* CONFIG_HAVE_isinf */

	/* Check for NAN */
#ifdef CONFIG_HAVE_isnan
	if (isnan(value)) {
		buf[1] = 'N';
		buf[2] = 'a';
		buf[3] = 'N';
		goto do_special_float;
#define WANT_do_special_float
	}
#endif /* CONFIG_HAVE_isnan */

	is_negative = false;
	if (value < 0) {
		is_negative = true;
		value       = -value;
	}

	/* Determine the intended precision. */
	max_prec = (unsigned int)precision;
	min_prec = (unsigned int)precision;
	if (!(flags & DEEFLOAT_PRINT_FPRECISION)) {
		max_prec = 6;
		min_prec = 0;
	} else if (max_prec > 9) {
		max_prec = min_prec = 9;
	}

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
		diff = value - (LOCAL_float_t)whole;
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
	    /*    */ (DEEFLOAT_PRINT_FWIDTH | DEEFLOAT_PRINT_FLJUST)) {
		/* Pad with with leading zeroes. */
		if (is_negative || (flags & (DEEFLOAT_PRINT_FSIGN | DEEFLOAT_PRINT_FSPACE)))
			++total_len;
		if (max_prec != 0) {
			unsigned int temp_min;
			temp_min = min_prec;
			whole    = frac;
			++total_len; /* . */
			for (;;) {
				if (temp_min)
					--temp_min;
				++total_len;
				whole /= 10;
				if (!whole)
					break;
			}
			total_len += temp_min;
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
		if (is_negative) {
			buf[--len] = '-';
		} else if (flags & DEEFLOAT_PRINT_FSIGN) {
			buf[--len] = '+';
		} else if (flags & DEEFLOAT_PRINT_FSPACE) {
			buf[--len] = ' ';
		}
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
	    /*    */ (DEEFLOAT_PRINT_FWIDTH) &&
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
	return result;
err:
	return temp;
#ifdef WANT_do_special_float
#undef WANT_do_special_float
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
		if (is_negative) {
			buf[0] = '-';
		} else if (flags & DEEFLOAT_PRINT_FSIGN) {
			buf[0] = '+';
		} else {
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
#endif /* WANT_do_special_float */
}


#undef LOCAL_float_t
#undef DEFINE_DeeFloat_LPrint
#undef DEFINE_DeeFloat_Print
