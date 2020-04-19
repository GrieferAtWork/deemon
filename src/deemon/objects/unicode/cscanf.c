/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_CSCANF_C
#define GUARD_DEEMON_OBJECTS_UNICODE_CSCANF_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>

#include <hybrid/atomic.h>

#include <string.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

typedef DeeStringObject String;


typedef struct {
	OBJECT_HEAD
	DREF DeeObject *ss_data;   /* [1..1][const] The string data object (either a string, or Bytes object). */
	DREF DeeObject *ss_format; /* [1..1][const] The scanner format object (either a string, or Bytes object). */
} StringScanner;

typedef struct {
	OBJECT_HEAD
	DREF StringScanner *si_scanner; /* [1..1][const] The underlying scanner. */
	char               *si_datend;  /* [1..1][const] End address of the input data (dereferences to a NUL-character). */
	char               *si_fmtend;  /* [1..1][const] End address of the format string (dereferences to a NUL-character). */
#ifndef CONFIG_NO_THREADS
	rwlock_t            si_lock;    /* Lock for modifying the data and format pointers.
	                                 * NOTE: Not required to be held when reading those pointers! */
#endif /* !CONFIG_NO_THREADS */
	char               *si_datiter; /* [1..1][lock(READ(atomic),WRITE(si_lock))] The current data pointer (UTF-8). */
	char               *si_fmtiter; /* [1..1][lock(READ(atomic),WRITE(si_lock))] The current format pointer (UTF-8). */
} StringScanIterator;

#ifdef CONFIG_NO_THREADS
#define GET_FORMAT_POINTER(x)            ((x)->si_fmtiter)
#else /* CONFIG_NO_THREADS */
#define GET_FORMAT_POINTER(x) ATOMIC_READ((x)->si_fmtiter)
#endif /* !CONFIG_NO_THREADS */

INTDEF DeeTypeObject StringScanIterator_Type;
INTDEF DeeTypeObject StringScan_Type;


LOCAL bool DCALL
match_contains(char *__restrict sel_start,
               char *__restrict sel_end,
               uint32_t ch) {
	while (sel_start < sel_end) {
		uint32_t sel_ch = utf8_readchar((char const **)&sel_start, sel_end);
		/* Deal with character escaping. */
		if (sel_ch == '\\')
			sel_ch = utf8_readchar((char const **)&sel_start, sel_end);
		if (sel_start < sel_end && *sel_start == '-') {
			/* Selection range. */
			uint32_t sel_max;
			++sel_start;
			sel_max = utf8_readchar((char const **)&sel_start, sel_end);
			if (sel_max == '\\')
				sel_max = utf8_readchar((char const **)&sel_start, sel_end);
			/* Check if part of this range. */
			if (ch >= sel_ch && ch <= sel_max)
				return true;
		} else {
			/* Check if this is the one. */
			if (sel_ch == ch)
				return true;
		}
	}
	return false;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ssi_next(StringScanIterator *__restrict self) {
	DREF DeeObject *result;
	uint32_t ch32;
	char *data, *format, ch;
	char *format_end, *data_end;
	int is_bytes = -1;
#ifdef CONFIG_NO_THREADS
	data   = self->si_datiter;
	format = self->si_fmtiter;
#else /* CONFIG_NO_THREADS */
	char *orig_data, *orig_format;
again:
	data = orig_data = ATOMIC_READ(self->si_datiter);
	format = orig_format = ATOMIC_READ(self->si_fmtiter);
#endif /* !CONFIG_NO_THREADS */
	format_end = self->si_fmtend;
	data_end   = self->si_datend;

next_format:
	if (format >= format_end)
		goto out_formatend;
	ch = *format++;
	switch (ch) {

	case '%': {
		bool ignore_data;
		size_t width;
		char *spec_data_start;
		unsigned int radix;
		unsigned int scan_radix;
		/* The heart of it all: Scan a format output argument. */
		ignore_data = false;
		width       = (size_t)-1;
		radix       = 0;
		/* Check: Ignored field. */
		if (format >= format_end)
			goto out_formatend;
		ch32 = utf8_readchar_u((char const **)&format);
		if (ch32 == '*')
			ignore_data = true,
			ch32        = utf8_readchar_u((char const **)&format);
		/* Check: is the max field width given. */
		if (DeeUni_IsDecimal(ch32)) {
			width = DeeUni_AsDigit(ch32);
			for (;;) {
				if (format >= format_end)
					goto out_formatend;
				ch32 = utf8_readchar_u((char const **)&format);
				if (!DeeUni_IsDecimal(ch32))
					break;
				width *= 10;
				width += DeeUni_AsDigit(ch32);
			}
		}
		spec_data_start = data;
next_spec:
		switch (ch32) {

		case '%':
			/* Double-`%' -> match source data against a single `%'-character */
			goto match_ch;

		case 'o': radix = 8; goto do_integer_scan;
		case 'd': radix = 10; goto do_integer_scan;
		case 'x':
		case 'p': radix = 16; goto do_integer_scan;

		case 'i':
		case 'u':
do_integer_scan:
			/* Integer. */
			while (data < data_end &&
			       (*data == '+' || *data == '-') && width)
				++data, --width;
			scan_radix = radix;
			if (is_bytes < 0)
				is_bytes = DeeBytes_Check(self->si_scanner->ss_data);

			if (radix == 0 && data < data_end && width) {
				uint32_t ch32;
				char *prev_data;
				prev_data = data;
				ch32      = is_bytes ? (uint32_t)(uint8_t)*data++ : utf8_readchar((char const **)&data, data_end);
				if (DeeUni_IsDecimalX(ch32, 0)) {
					--width;
					if (width && (*data == 'x' || *data == 'X')) {
						scan_radix = 16;
						--width;
						++data;
					} else if (width && (*data == 'b' || *data == 'B')) {
						scan_radix = 2;
						--width;
						++data;
					} else {
						scan_radix = 8;
					}
				} else {
					data       = prev_data;
					scan_radix = 10;
				}
			}
			while (data < data_end && width) {
				uint32_t ch32;
				char *prev_data;
				struct unitraits *traits;
				if (scan_radix > 10) {
					char data_ch = *data;
					if (((uint8_t)data_ch >= (uint8_t)'a' && (uint8_t)data_ch <= (uint8_t)('a' + (scan_radix - 10))) ||
					    ((uint8_t)data_ch >= (uint8_t)'A' && (uint8_t)data_ch <= (uint8_t)('A' + (scan_radix - 10)))) {
						++data;
						--width;
						continue;
					}
				}
				prev_data = data;
				ch32      = is_bytes ? (uint32_t)(uint8_t)*data++ : utf8_readchar((char const **)&data, data_end);
				traits    = DeeUni_Descriptor(ch32);
				if (!(traits->ut_flags & UNICODE_FDECIMAL) ||
				    traits->ut_digit >= scan_radix) {
					data = prev_data;
					break;
				}
			}
			if (ignore_data)
				goto next_format;
			/* NOTE: Pass the `DEEINT_STRING_FTRY' flag to have `DeeInt_FromString()'
			 *       return ITER_DONE in case of parsing failure, meaning that we in
			 *       turn will stop enumeration. */
			result = DeeInt_FromString(spec_data_start,
			                           (size_t)(data - spec_data_start),
			                           DEEINT_STRING(radix,
			                                         DEEINT_STRING_FNORMAL |
			                                         DEEINT_STRING_FTRY));
			break;

		case 'c':
			if (width == (size_t)-1)
				width = 1;
			if (width > (size_t)(data_end - data)) {
				width = (size_t)(data_end - data);
				if (!width)
					goto out_dataend;
			}
			data += width;
			if (ignore_data)
				goto next_format;
			result = DeeString_NewSized(spec_data_start, width);
			break;

		case 's':
			if (is_bytes < 0)
				is_bytes = DeeBytes_Check(self->si_scanner->ss_data);
			if (is_bytes) {
				while (data < data_end && !DeeUni_IsSpace(*data) && width)
					++data, --width;
			} else {
				while (data < data_end && width) {
					uint32_t data_ch;
					char *prev_data = data;
					data_ch         = utf8_readchar((char const **)&data, data_end);
					if (!DeeUni_IsSpace(data_ch)) {
						data = prev_data;
						break;
					}
					--width;
				}
			}
			if (ignore_data)
				goto next_format;
			result = DeeString_NewSized(spec_data_start,
			                            (size_t)(data - spec_data_start));
			break;


		/* Length modifiers (ignored for backwards compatibility) */
		case 'I':
			if (format >= format_end)
				goto out_formatend;
			if (format[0] == '8') {
				++format;
			} else if (format[0] == '1' && format < format_end - 1 && format[1] == '6') {
				format += 2;
			} else if (format[0] == '3' && format < format_end - 1 && format[1] == '2') {
				format += 2;
			} else if (format[0] == '6' && format < format_end - 1 && format[1] == '4') {
				format += 2;
			}
			ATTR_FALLTHROUGH
		case 'h':
		case 'l':
		case 'j':
		case 'z':
		case 't':
		case 'L':
			if (format >= format_end)
				goto out_formatend;
			ch32 = utf8_readchar_u((char const **)&format);
			goto next_spec;

		case '[': {
			bool inverse_selection;
			char *sel_begin;
			inverse_selection = false;
			if (format >= format_end)
				goto out_formatend;
			ch = *format++;
			if (ch == '^') {
				if (format >= format_end)
					goto out_formatend;
				inverse_selection = true;
				ch                = *format++;
			}
			sel_begin = --format;
			/* Scan for the end of the match-set. */
			while (format < format_end) {
				if (*format == ']' && format[-1] != '\\')
					break;
				++format;
			}
			if (width > (size_t)(data_end - data)) {
				width = (size_t)(data_end - data);
				if (!width)
					goto out_dataend;
			}
			/* Scan until the end of input data matching the given selection. */
			if (is_bytes < 0)
				is_bytes = DeeBytes_Check(self->si_scanner->ss_data);
			if (is_bytes) {
				while (data < data_end && width) {
					if (!(match_contains(sel_begin, format, *data) ^ inverse_selection))
						break;
					--width;
					++data;
				}
			} else {
				while (data < data_end && width) {
					char *prev_data  = data;
					uint32_t data_ch = utf8_readchar((char const **)&data, data_end);
					if (!(match_contains(sel_begin, format, data_ch) ^ inverse_selection)) {
						data = prev_data;
						break;
					}
					--width;
				}
			}
			/* Skip the trailing `]'-character */
			if (format != format_end &&
			    format[0] == ']')
				++format;
			if (ignore_data)
				goto next_format;
			/* Construct a string from all matched data. */
			result = DeeString_NewSized(spec_data_start,
			                            (size_t)(data - spec_data_start));
		}	break;

		case 'n':
			/* Return the number of bytes already consumed. */
			if (is_bytes < 0)
				is_bytes = DeeBytes_Check(self->si_scanner->ss_data);
			if (is_bytes) {
				result = DeeInt_NewSize((size_t)((uint8_t *)data - DeeBytes_DATA(self->si_scanner->ss_data)));
			} else {
				size_t total_consumption;
				char *iter = DeeString_AsUtf8(self->si_scanner->ss_data);
				if unlikely(!iter)
					goto err;
				total_consumption = 0;
				while (iter < data) {
					++total_consumption;
					utf8_readchar((char const **)&iter, data);
				}
				result = DeeInt_NewSize(total_consumption);
			}
			break;

		default:
			/* Throw an error for an invalid format option.
			 * NOTE: the old deemon used to return `none' here, however that was
			 *       a decision I made before finally understanding why you don't
			 *       want to implement spots for potential future expansion as
			 *       no-ops, rather than as exceptions.
			 *       aka.: The difference between ~reserved~ and ~ignored~ */
			DeeError_Throwf(&DeeError_ValueError,
			                "Unknown or unexpected cscanf character %I32C in `%$s', apart of %r",
			                ch32, (size_t)(format - orig_format), orig_format,
			                self->si_scanner->ss_format);
			goto err;
		}
	}	break;

	case ' ':
		/* Skip space characters in `data' */
		if (is_bytes < 0)
			is_bytes = DeeBytes_Check(self->si_scanner->ss_data);
		if (is_bytes) {
			while (data < data_end &&
			       DeeUni_IsSpace(*data))
				++data;
		} else {
			while (data < data_end) {
				char *prev_data  = data;
				uint32_t data_ch = utf8_readchar((char const **)&data, data_end);
				if (!DeeUni_IsSpace(data_ch)) {
					data = prev_data;
					break;
				}
			}
		}
		goto next_format;

	/* Match universal linefeeds in format
	 * against any kind of linefeed in `data'. */

#if 0 /* Don't... (otherwise we'd have to include any kind of unicode line-feed, too) */
	case '\r':
		/* Also allow `\r' and `\r\n' as format input. */
		if (format != format_end &&
		    *format == '\n')
			++format;
		ATTR_FALLTHROUGH
#endif
	case '\n':
		/* Skip some sort of linefeed in `data' */
		if (data >= data_end)
			goto out_dataend;
		if (*data == '\r') {
			if (data + 1 >= data_end)
				goto out_dataend;
			if (*++data == '\n')
				++data;
		} else if (*data == '\n') {
		} else {
			/*char *prev_data;*/
			uint32_t data_ch;
			if (is_bytes < 0)
				is_bytes = DeeBytes_Check(self->si_scanner->ss_data);
			if (is_bytes)
				goto out_missmatch;
			/*prev_data = data;*/
			data_ch = utf8_readchar((char const **)&data, data_end);
			if (!DeeUni_IsLF(data_ch)) { /*data = prev_data;*/
				goto out_missmatch;
			}
		}
		goto next_format;

	default:
		/* Default case: Match the given format-character
		 *               with a character read from data. */
		/* NOTE: deemon<200 used to throw a ValueError here for missmatch / dataend,
		 *       however to better mirror the behavior seen by C's scanf() functions,
		 *       which simply return the number of parsed items, rather than
		 *       returning an error code upon miss-match, the new deemon does
		 *       the same by indicating ITER_DONE.
		 *    -> Despite this, considering the main use of scanf() in
		 *       deemon has always been by code such as this...
		 *       >> local x, y, z;
		 *       >> try {
		 *       >>     x, y, z = data.scanf("x = %d, y = %d, z = %d")...;
		 *       >> } catch (...) {
		 *       >>     print "Bad data:",repr data;
		 *       >>     continue;
		 *       >> }
		 *       >> process_data(x, y, z);
		 *       ... With the intended behavior being that an exception is
		 *       thrown if the number of parsed items doesn't match the
		 *       expected number of items, that behavior still exists due
		 *       to the fact that the unpack operation above will throw an
		 *       UnpackError if less than 3 items were parsed from `data',
		 *       now indicated by the effective scanf()-sequence literally
		 *       not being as long. */
match_ch:
		if (data >= data_end)
			goto out_dataend;
		if (*data != ch)
			goto out_missmatch;
		++data;
		goto next_format;
	}
	/* Check for errors. */
	if unlikely(!result)
		goto err;
done:
#ifdef CONFIG_NO_THREADS
	self->si_datiter = data;
	self->si_fmtiter = format;
#else /* CONFIG_NO_THREADS */
	rwlock_write(&self->si_lock);
	/* Check if another thread extracted a value in the mean time. */
	if unlikely(self->si_datiter != orig_data ||
		         self->si_fmtiter != orig_format)
	{
		/* Race condition! -> Loop back and try to read a value once again. */
		rwlock_endwrite(&self->si_lock);
		if (ITER_ISOK(result))
			Dee_Decref(result);
		goto again;
	}
	/* Save the updated data & format pointers. */
	self->si_datiter = data;
	self->si_fmtiter = format;
	rwlock_endwrite(&self->si_lock);
#endif /* !CONFIG_NO_THREADS */
	return result;
out_dataend:
out_missmatch:
out_formatend:
	--format;
	result = ITER_DONE;
	goto done;
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
ssi_fini(StringScanIterator *__restrict self) {
	/* likely: it was probably only created for iteration,
	 *         in which case it'll be destroyed with us. */
	Dee_Decref_likely(self->si_scanner);
}

PRIVATE NONNULL((1, 2)) void DCALL
ssi_visit(StringScanIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->si_scanner);
}

#define DEFINE_SPLITITER_CMP(name, op)                                               \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                                    \
	name(StringScanIterator *__restrict self,                                        \
	     StringScanIterator *__restrict other) {                                     \
		if (DeeObject_AssertTypeExact((DeeObject *)other, &StringScanIterator_Type)) \
			return NULL;                                                             \
		return_bool(GET_FORMAT_POINTER(self) op GET_FORMAT_POINTER(other));          \
	}
DEFINE_SPLITITER_CMP(ssi_eq, ==)
DEFINE_SPLITITER_CMP(ssi_ne, !=)
DEFINE_SPLITITER_CMP(ssi_lo, <)
DEFINE_SPLITITER_CMP(ssi_le, <=)
DEFINE_SPLITITER_CMP(ssi_gr, >)
DEFINE_SPLITITER_CMP(ssi_ge, >=)
#undef DEFINE_SPLITITER_CMP

PRIVATE struct type_cmp ssi_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssi_ge
};

PRIVATE struct type_member ssi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT,
	                      offsetof(StringScanIterator, si_scanner),
	                      "->?Ert:StringScan"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ssi_copy(StringScanIterator *__restrict self,
         StringScanIterator *__restrict other) {
	rwlock_read(&other->si_lock);
	self->si_datiter = other->si_datiter;
	self->si_fmtiter = other->si_fmtiter;
	rwlock_endread(&other->si_lock);
	rwlock_init(&self->si_lock);
	self->si_scanner = other->si_scanner;
	Dee_Incref(self->si_scanner);
	self->si_datend = other->si_datend;
	self->si_fmtend = other->si_fmtend;
	return 0;
}


INTERN DeeTypeObject StringScanIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringScanIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)NULL, /* TODO */
				/* .tp_copy_ctor = */ (void *)&ssi_copy,
				/* .tp_deep_ctor = */ (void *)NULL,
				/* .tp_any_ctor  = */ (void *)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(StringScanIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ssi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ssi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &ssi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ssi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};






PRIVATE NONNULL((1)) void DCALL
ss_fini(StringScanner *__restrict self) {
	Dee_Decref(self->ss_data);
	/* unlikely: it's probably a string constant */
	Dee_Decref_unlikely(self->ss_format);
}

PRIVATE NONNULL((1, 2)) void DCALL
ss_visit(StringScanner *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ss_data);
	Dee_Visit(self->ss_format);
}

PRIVATE WUNUSED NONNULL((1)) DREF StringScanIterator *DCALL
ss_iter(StringScanner *__restrict self) {
	DREF StringScanIterator *result;
	result = DeeObject_MALLOC(StringScanIterator);
	if unlikely(!result)
		goto done;
	/* Universally use UTF-8 for string scanning to prevent
	 * any problems related to unicode text processing. */
	if (DeeString_Check(self->ss_data)) {
		result->si_datiter = DeeString_AsUtf8((DeeObject *)self->ss_data);
		if unlikely(!result->si_datiter)
			goto err_r;
		result->si_datend = result->si_datiter + WSTR_LENGTH(result->si_datiter);
	} else {
		ASSERT(DeeBytes_Check(self->ss_data));
		result->si_datiter = (char *)DeeBytes_DATA(self->ss_data);
		result->si_datend  = result->si_datiter + DeeBytes_SIZE(self->ss_data);
	}
	if (DeeString_Check(self->ss_format)) {
		result->si_fmtiter = DeeString_AsUtf8((DeeObject *)self->ss_format);
		if unlikely(!result->si_fmtiter)
			goto err_r;
		result->si_fmtend = result->si_fmtiter + WSTR_LENGTH(result->si_fmtiter);
	} else {
		ASSERT(DeeBytes_Check(self->ss_format));
		result->si_fmtiter = (char *)DeeBytes_DATA(self->ss_format);
		result->si_fmtend  = result->si_fmtiter + DeeBytes_SIZE(self->ss_format);
	}
	rwlock_init(&result->si_lock);
	result->si_scanner = self;
	Dee_Incref(self);
	DeeObject_Init(result, &StringScanIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE struct type_seq ss_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ss_iter
};

PRIVATE struct type_member ss_members[] = {
	TYPE_MEMBER_FIELD_DOC("__str__", STRUCT_OBJECT, offsetof(StringScanner, ss_data), "->?X2?Dstring?DBytes"),
	TYPE_MEMBER_FIELD_DOC("__format__", STRUCT_OBJECT, offsetof(StringScanner, ss_format), "->?X2?Dstring?DBytes"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member ss_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &StringScanIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject StringScan_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringScan",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(StringScanner)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ss_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ss_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ss_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ss_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ss_class_members
};





/* Implement c-style string scanning, using a scanf()-style format string.
 * This functions then returns a sequence of all scanned objects, that is
 * the usually used in an expand expression:
 * >> for (local line: File.stdin) {
 * >>     local a, b, c;
 * >>     try {
 * >>         a, b, c = line.scanf("%s %s %s")...;
 * >>     } catch (...) { // Unpack errors.
 * >>         continue;
 * >>     }
 * >>     print "a:", a;
 * >>     print "b:", b;
 * >>     print "c:", c;
 * >> }
 */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *self,
                DeeObject *format) {
	DREF StringScanner *result;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT(format);
	ASSERT(DeeString_Check(self) || DeeBytes_Check(self));
	ASSERT(DeeString_Check(format));
	/* Deemon 200+ uses a generator to implement scanf()
	 * as efficiently as possible, without the need of an
	 * intermediate sequence type, as well as optimizing
	 * to only generate output values which are actually
	 * being used. */
	result = DeeObject_MALLOC(StringScanner);
	if unlikely(!result)
		goto done;
	result->ss_data   = self;
	result->ss_format = format;
	Dee_Incref(self);
	Dee_Incref(format);
	DeeObject_Init(result, &StringScan_Type);
done:
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_CSCANF_C */
