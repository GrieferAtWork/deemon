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
#ifdef __INTELLISENSE__
#include "float.c"
#define DEFINE_Dee_Strtold
//#define DEFINE_Dee_Strtod
#endif

#include <deemon/api.h>

#include <deemon/system-features.h> /* isdigit */

#if (defined(DEFINE_Dee_Strtold) + \
     defined(DEFINE_Dee_Strtod)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_Dee_Strtold
#define LOCAL_float_t    __LONGDOUBLE
#define LOCAL_Dee_Strtod Dee_Strtold
#else /* DEFINE_Dee_Strtold */
#define LOCAL_float_t    double
#define LOCAL_Dee_Strtod Dee_Strtod
#endif /* !DEFINE_Dee_Strtold */

/* Convert a string into a floating-point number. */
PUBLIC NONNULL((1)) LOCAL_float_t DCALL
LOCAL_Dee_Strtod(char const *str, char **p_endptr) {
	char sign, ch = *str;
	LOCAL_float_t float_extension_mult, fltval = 0.0;
	int numsys, more;
	sign = ch;
	if (ch == '+' || ch == '-')
		ch = *++str;
	if (ch == '0') {
		ch = *++str;
		if (ch == 'x' || ch == 'X') {
			++str;
			numsys = 16;
		} else if (ch == 'b' || ch == 'B') {
			++str;
			numsys = 2;
		} else if (ch == '.') {
			numsys = 10;
		} else if (!isdigit((unsigned char)ch)) {
			goto done;
		} else {
			numsys = 8;
		}
	} else {
#ifdef float_inf_value
		if ((ch == 'i' || ch == 'I') &&
		    (str[1] == 'n' || str[1] == 'N') &&
		    (str[2] == 'f' || str[2] == 'F') &&
		    !isalnum((unsigned char)str[3])) {
			str += 3;
			if (p_endptr)
				*p_endptr = (char *)str;
			return sign == '-' ? -(LOCAL_float_t)float_inf_value
			                   : +(LOCAL_float_t)float_inf_value;
		}
#endif /* float_inf_value */
#ifdef float_nan_value
		if ((ch == 'n' || ch == 'N') &&
		    (str[1] == 'a' || str[1] == 'A') &&
		    (str[2] == 'n' || str[2] == 'N') &&
		    !isalnum((unsigned char)str[3])) {
			str += 3;
			if (p_endptr)
				*p_endptr = (char *)str;
			return sign == '-' ? -(LOCAL_float_t)float_nan_value
			                   : +(LOCAL_float_t)float_nan_value;
		}
#endif /* float_nan_value */
		numsys = 10;
	}
	float_extension_mult = 0;
next:
	for (;;) {
		ch = *str++;
		switch (ch) {

		case 'p':
		case 'P':
			goto flt_ext;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			more = ch - '0';
			break;

		case 'e':
			if (numsys < 15)
				goto flt_ext;
			ATTR_FALLTHROUGH
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'f':
			more = 10 + ch - 'a';
			break;

		case 'E':
			if (numsys < 15)
				goto flt_ext;
			ATTR_FALLTHROUGH
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'F':
			more = 10 + ch - 'A';
			break;

		case '.':
			float_extension_mult = numsys;
			goto next;

		default:
			goto break_main_loop;
		}
		if unlikely(more >= numsys) {
break_main_loop:
			--str;
			goto done;
		}
		if (float_extension_mult != 0) {
			fltval += (LOCAL_float_t)more /
			          (LOCAL_float_t)float_extension_mult;
			float_extension_mult *= numsys;
		} else {
			fltval = fltval * numsys + more;
		}
	}
flt_ext:
	/* Read the Float extension: E[+/-][int] */
#define float_extension_pos numsys
#define float_extension_off more
	float_extension_pos = 1;
	float_extension_off = 0;
	ch = *str++;
	if (ch == '-' || ch == '+') {
		float_extension_pos = (ch == '+');
		ch = *str++;
	}
	while (ch >= '0' && ch <= '9') {
		float_extension_off *= 10;
		float_extension_off += ch - '0';
		ch = *str++;
	}
	--str; /* Point *at* the first character after the digit-sequence */
	float_extension_mult = 1.0;
	while (float_extension_off != 0) {
		float_extension_mult *= 10.0;
		--float_extension_off;
	}
	if (float_extension_pos) {
		fltval *= float_extension_mult;
	} else {
		fltval /= float_extension_mult;
	}
#undef float_extension_pos
#undef float_extension_off
done:
	if (p_endptr)
		*p_endptr = (char *)str;
	return sign == '-' ? -fltval
	                   : +fltval;
}


#undef LOCAL_float_t
#undef LOCAL_Dee_Strtod

DECL_END

#undef DEFINE_Dee_Strtold
#undef DEFINE_Dee_Strtod
