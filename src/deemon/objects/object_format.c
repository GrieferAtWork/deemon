/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_OBJECT_FORMAT_C
#define GUARD_DEEMON_OBJECTS_OBJECT_FORMAT_C 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/super.h>

#include <hybrid/overflow.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
get_generic_attribute(DeeTypeObject *tp_self, DeeObject *self, DeeObject *name);

INTERN WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
object_format_generic(DeeObject *__restrict self,
                      dformatprinter printer, void *arg,
                      /*utf-8*/ char const *__restrict format_str,
                      size_t format_len) {
#define ALIGN_LEFT   0
#define ALIGN_RIGHT  1
#define ALIGN_CENTER 2
#define ALIGN_ZFILL  3
	unsigned int alignment_mode;
	char const *format_end;
	char ch;
	size_t alignment_width;
	char const *filler_str;
	size_t filler_len;
	DREF DeeObject *self_str;
	dssize_t result, temp;
	size_t self_len;
	if (!format_len)
		return DeeObject_Print(self, printer, arg);
	alignment_mode = ALIGN_LEFT;
	format_end     = format_str + format_len;
	filler_str     = " ";
	filler_len     = 1;
	switch (format_str[0]) {

	case '<':
		++format_str;
		break;

	case '>':
		alignment_mode = ALIGN_RIGHT;
		++format_str;
		break;

	case '^':
		alignment_mode = ALIGN_CENTER;
		++format_str;
		break;

	case '=':
		alignment_mode = ALIGN_ZFILL;
		filler_str     = "0";
		++format_str;
		break;

	default: break;
	}
	if unlikely(format_str >= format_end)
		goto err_bad_format_str;
	ch = *format_str++;
	if (!DeeUni_AsDigit(ch, 10, &alignment_width))
		goto err_bad_format_str;
	filler_str      = NULL;
	filler_len      = 0;
	while (format_str < format_end) {
		uint8_t digit;
		ch = *format_str++;
		if (!DeeUni_AsDigit(ch, 10, &digit)) {
			if (ch != ':')
				goto err_bad_format_str;
			filler_str = format_str;
			filler_len = format_end - format_str;
			/* The filler string isn't allowed to be empty. */
			if unlikely(!filler_len)
				goto err_bad_format_str;
			break;
		}
		if (OVERFLOW_UMUL(alignment_width, 10, &alignment_width) ||
		    OVERFLOW_UADD(alignment_width, digit, &alignment_width))
			return err_integer_overflow_i(sizeof(size_t) * 8, true);
	}

	/* Special case: With an alignment width of ZERO(0), we already
	 *               know that we would never have to pad, so we can
	 *               simply print the object regularly! */
	if unlikely(!alignment_width)
		return DeeObject_Print(self, printer, arg);

	/* Generate the string representation of `self' */
	self_str = DeeObject_Str(self);
	if unlikely(!self_str)
		goto err;

	/* Do the alignment! */
	result   = 0;
	self_len = DeeString_WLEN(self_str);
	ASSERT(filler_len != 0);
	switch (alignment_mode) {

	case ALIGN_LEFT:
		if (alignment_width <= self_len)
			goto print_self_raw;
print_ljust:
		temp = DeeString_PrintUtf8(self_str, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		temp = DeeFormat_RepeatUtf8(printer, arg,
		                            filler_str, filler_len,
		                            alignment_width);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		break;

	case ALIGN_RIGHT:
		if (alignment_width > self_len) {
			temp = DeeFormat_RepeatUtf8(printer, arg,
			                            filler_str, filler_len,
			                            alignment_width);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
print_self_raw:
		temp = DeeString_PrintUtf8(self_str, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		break;

	case ALIGN_CENTER: {
		size_t linsert, rinsert;
		if (alignment_width <= self_len)
			goto print_self_raw;
		rinsert = alignment_width - self_len;
		linsert = rinsert / 2;
		rinsert = rinsert - linsert;
		temp = DeeFormat_RepeatUtf8(printer, arg,
		                            filler_str, filler_len,
		                            linsert);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		temp = DeeString_PrintUtf8(self_str, printer, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		temp = DeeFormat_RepeatUtf8(printer, arg,
		                            filler_str, filler_len,
		                            rinsert);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}	break;

	case ALIGN_ZFILL: {
		char *my_str;
		size_t num_signs;
		if (alignment_width <= self_len)
			goto print_self_raw;
		my_str = DeeString_AsUtf8(self_str);
		if unlikely(!my_str) {
			temp = -1;
			goto err_temp;
		}
		num_signs = 0;
		while (my_str[num_signs] == '+' ||
		       my_str[num_signs] == '-')
			++num_signs;
		if (!num_signs)
			goto print_ljust;
		ASSERT(num_signs <= WSTR_LENGTH(my_str));
		temp = (*printer)(arg, my_str, num_signs);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		temp = DeeFormat_RepeatUtf8(printer, arg,
		                            filler_str, filler_len,
		                            alignment_width - self_len);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		temp = (*printer)(arg, my_str + num_signs,
		                  WSTR_LENGTH(my_str) - num_signs);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}	break;

	default:
		__builtin_unreachable();
	}
	Dee_Decref(self_str);
	return result;
err_temp:
	Dee_Decref(self_str);
	return temp;
err_bad_format_str:
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Bad format string %$q",
	                       format_len,
	                       format_end - format_len);
err:
	return -1;
}





PRIVATE WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
object_format_impl(DeeObject *__restrict self,
                   dformatprinter printer, void *arg,
                   /*utf-8*/ char const *__restrict format_str,
                   size_t format_len, DeeObject *format_str_obj) {
	DeeTypeObject *tp_self;
	tp_self = Dee_TYPE(self);
	if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
	}
	/* TODO: Optimizations for known types! */
	do {
		DREF DeeObject *format_function;
		if (tp_self == &DeeObject_Type)
			break;
		if (tp_self->tp_attr) {
			if (!tp_self->tp_attr->tp_getattr)
				break;
			format_function = (*tp_self->tp_attr->tp_getattr)(self, (DeeObject *)&str___format__);
			if unlikely(!format_function) {
check_attribute_error:
				if (DeeError_Catch(&DeeError_AttributeError) ||
				    DeeError_Catch(&DeeError_NotImplemented))
					break; /* Stop searching for a sub-class's __format__ function. */
				goto err;
			}
			goto call_format_function;
		}
		format_function = get_generic_attribute(tp_self, self,
		                                        (DeeObject *)&str___format__);
		if (format_function != ITER_DONE) {
			DREF DeeObject *callback_result;
			dssize_t result;
			if unlikely(!format_function)
				goto check_attribute_error;
call_format_function:
			if (format_str_obj) {
				callback_result = DeeObject_Call(format_function, 1, &format_str_obj);
			} else {
				format_str_obj = DeeString_NewUtf8(format_str, format_len, STRING_ERROR_FIGNORE);
				if unlikely(!format_str_obj)
					callback_result = NULL;
				else {
					callback_result = DeeObject_Call(format_function, 1, &format_str_obj);
					Dee_Decref(format_str_obj);
				}
			}
			Dee_Decref(format_function);
			if unlikely(!callback_result)
				goto err;
			result = DeeObject_Print(callback_result, printer, arg);
			Dee_Decref(callback_result);
			return result;
		}
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
	/* Fallback: Format using `object.__format__' */
	return object_format_generic(self, printer, arg, format_str, format_len);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
DeeObject_PrintFormatString(DeeObject *__restrict self,
                            dformatprinter printer, void *arg,
                            /*utf-8*/ char const *__restrict format_str,
                            size_t format_len) {
	return object_format_impl(self, printer, arg, format_str, format_len, NULL);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
DeeObject_PrintFormat(DeeObject *__restrict self,
                      dformatprinter printer, void *arg,
                      DeeObject *__restrict format_str) {
	char *utf8_format;
	utf8_format = DeeString_AsUtf8(format_str);
	if unlikely(!utf8_format)
		goto err;
	return object_format_impl(self, printer, arg, utf8_format,
	                          WSTR_LENGTH(utf8_format), format_str);
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_OBJECT_FORMAT_C */
