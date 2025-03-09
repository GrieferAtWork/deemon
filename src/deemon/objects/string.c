/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_STRING_C
#define GUARD_DEEMON_OBJECTS_STRING_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none-operator.h> /* _DeeNone_reti1_3 */
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memmem() */
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include <hybrid/limitcore.h>
#include <hybrid/minmax.h>
#include <hybrid/typecore.h>

#include "../runtime/method-hint-defaults.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
/**/

#include <stdarg.h> /* va_list */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__
#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeStringObject String;

#ifndef CONFIG_HAVE_memmem
#define memmem dee_memmem
DeeSystem_DEFINE_memmem(dee_memmem)
#endif /* !CONFIG_HAVE_memmem */

/* Release exactly `datalen' bytes from the printer to be
 * re-used in subsequent calls, or be truncated eventually. */
PUBLIC NONNULL((1)) void
(DCALL Dee_ascii_printer_release)(struct ascii_printer *__restrict self, size_t datalen) {
	ASSERT(self->ap_length >= datalen);
	/* This's actually all that needs to be
	 * done with the current implementation. */
	self->ap_length -= datalen;
}

/* Allocate space for `datalen' bytes at the end of `self',
 * then return a pointer to the start of this new buffer. */
PUBLIC WUNUSED NONNULL((1)) char *
(DCALL Dee_ascii_printer_alloc)(struct ascii_printer *__restrict self, size_t datalen) {
	String *string;
	size_t alloc_size;
	char *result;
	if ((string = self->ap_string) == NULL) {
		/* Make sure not to allocate a string when the used length remains ZERO.
		 * >> Must be done to assure the expectation of `if(ap_length == 0) ap_string == NULL' */
		if unlikely(!datalen)
			return 0;
		/* Allocate the initial string. */
		alloc_size = 8;
		while (alloc_size < datalen)
			alloc_size *= 2;
alloc_again:
		string = (String *)DeeObject_TryMallocc(offsetof(String, s_str),
		                                        alloc_size + 1, sizeof(char));
		if unlikely(!string) {
			if (alloc_size != datalen) {
				alloc_size = datalen;
				goto alloc_again;
			}
			if (Dee_CollectMemoryoc(offsetof(String, s_str),
			                        alloc_size + 1, sizeof(char)))
				goto alloc_again;
			return NULL;
		}
		self->ap_string = string;
		string->s_len   = alloc_size;
		self->ap_length = datalen;
		return string->s_str;
	}
	alloc_size = string->s_len;
	ASSERT(alloc_size >= self->ap_length);
	alloc_size -= self->ap_length;
	if unlikely(alloc_size < datalen) {
		size_t min_alloc = self->ap_length + datalen;
		alloc_size       = (min_alloc + 63) & ~63;
realloc_again:
		string = (String *)DeeObject_TryReallocc(string, offsetof(String, s_str),
		                                         alloc_size + 1, sizeof(char));
		if unlikely(!string) {
			string = self->ap_string;
			if (alloc_size != min_alloc) {
				alloc_size = min_alloc;
				goto realloc_again;
			}
			if (Dee_CollectMemoryoc(offsetof(String, s_str), alloc_size + 1, sizeof(char)))
				goto realloc_again;
			return NULL;
		}
		self->ap_string = string;
		string->s_len   = alloc_size;
	}
	/* Append text at the end. */
	result = string->s_str + self->ap_length;
	self->ap_length += datalen;
	return result;
}

/* Print a single character, returning -1 on error or 0 on success. */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL Dee_ascii_printer_putc)(struct ascii_printer *__restrict self, char ch) {
	/* Quick check: Can we print to an existing buffer. */
	if (self->ap_string &&
	    self->ap_length < self->ap_string->s_len) {
		self->ap_string->s_str[self->ap_length++] = ch;
		goto done;
	}
	/* Fallback: go the long route. */
	if (ascii_printer_print(self, &ch, 1) < 0)
		goto err;
done:
	return 0;
err:
	return -1;
}

/* Append the given data to a string printer. (HINT: Use this one as a `Dee_formatprinter_t') */
PUBLIC WUNUSED NONNULL((1)) Dee_ssize_t
(DPRINTER_CC Dee_ascii_printer_print)(void *__restrict self,
                                      char const *__restrict data,
                                      size_t datalen) {
	struct ascii_printer *me;
	String *string;
	size_t alloc_size;
	me = (struct ascii_printer *)self;
	ASSERT(data || !datalen);
	if ((string = me->ap_string) == NULL) {
		/* Make sure not to allocate a string when the used length remains ZERO.
		 * >> Must be done to assure the expectation of `if(ap_length == 0) ap_string == NULL' */
		if unlikely(!datalen)
			return 0;
		/* Allocate the initial string. */
		alloc_size = 8;
		while (alloc_size < datalen)
			alloc_size *= 2;
alloc_again:
		string = (String *)DeeObject_TryMallocc(offsetof(String, s_str),
		                                        alloc_size + 1, sizeof(char));
		if unlikely(!string) {
			if (alloc_size != datalen) {
				alloc_size = datalen;
				goto alloc_again;
			}
			if (Dee_CollectMemoryoc(offsetof(String, s_str),
			                        alloc_size + 1, sizeof(char)))
				goto alloc_again;
			return -1;
		}
		me->ap_string = string;
		string->s_len = alloc_size;
		memcpyc(string->s_str, data,
		        datalen, sizeof(char));
		me->ap_length = datalen;
		goto done;
	}
	alloc_size = string->s_len;
	ASSERT(alloc_size >= me->ap_length);
	alloc_size -= me->ap_length;
	if unlikely(alloc_size < datalen) {
		size_t min_alloc = me->ap_length + datalen;
		alloc_size       = (min_alloc + 63) & ~63;
realloc_again:
		string = (String *)DeeObject_TryReallocc(string, offsetof(String, s_str),
		                                         alloc_size + 1, sizeof(char));
		if unlikely(!string) {
			string = me->ap_string;
			if (alloc_size != min_alloc) {
				alloc_size = min_alloc;
				goto realloc_again;
			}
			if (Dee_CollectMemoryoc(offsetof(String, s_str),
			                        alloc_size + 1, sizeof(char)))
				goto realloc_again;
			return -1;
		}
		me->ap_string = string;
		string->s_len = alloc_size;
	}
	/* Copy text into the dynamic string. */
	/*Dee_DPRINTF("PRINT: %" PRFXSIZ " - `%.*s'\n", datalen, (int)datalen, data);*/
	memcpyc(string->s_str + me->ap_length,
	        data, datalen, sizeof(char));
	me->ap_length += datalen;
done:
	return (Dee_ssize_t)datalen;
}

/* Pack together data from a string printer and return the generated contained string.
 * Upon success, as well as upon failure, the state of `self' is undefined upon return. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL Dee_ascii_printer_pack)(struct ascii_printer *__restrict self) {
	DREF String *result = self->ap_string;
	if unlikely(!result)
		return_reference_(Dee_EmptyString);
	/* Deallocate unused memory. */
	if likely(self->ap_length != result->s_len) {
		DREF String *reloc;
		reloc = (DREF String *)DeeObject_TryReallocc(result, offsetof(String, s_str),
		                                             self->ap_length + 1, sizeof(char));
		if likely(reloc)
			result = reloc;
		result->s_len = self->ap_length;
	}

	/* Make sure to terminate the c-string representation. */
	result->s_str[self->ap_length] = '\0';

	/* Do final object initialization. */
	DeeObject_Init(result, &DeeString_Type);
	result->s_hash = (Dee_hash_t)-1;
	result->s_data = NULL;
	DBG_memset(self, 0xcc, sizeof(*self));
	return (DREF DeeObject *)result;
}

/* Search the buffer that has already been created for an existing instance
 * of `str...+=length' and if found, return a pointer to its location.
 * Otherwise, append the given string and return a pointer to that location.
 * Upon error (append failed to allocate more memory), NULL is returned.
 * HINT: This function is very useful when creating
 *       string tables for NUL-terminated strings:
 *       >> ascii_printer_allocstr("foobar\0"); // Table is now `foobar\0'
 *       >> ascii_printer_allocstr("foo\0");    // Table is now `foobar\0foo\0'
 *       >> ascii_printer_allocstr("bar\0");    // Table is still `foobar\0foo\0' - `bar\0' points into `foobar\0'
 * @return: * :   A pointer to a volitile memory location within the already printed string
 *                (the caller should calculate the offset to `ASCII_PRINTER_STR(self)'
 *                to ensure consistency if the function is called multiple times)
 * @return: NULL: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) char *
(DCALL Dee_ascii_printer_allocstr)(struct ascii_printer *__restrict self,
                                   char const *__restrict str, size_t length) {
	char *result;
	Dee_ssize_t error;
	if (self->ap_string) {
		result = (char *)memmem(self->ap_string->s_str,
		                        self->ap_length, str, length);
		if (result)
			return result;
	}

	/* Append a new string. */
	error = ascii_printer_print(self, str, length);
	if unlikely(error < 0)
		goto err;
	ASSERT(self->ap_string || (!self->ap_length && !length));
	ASSERT(self->ap_length >= length);
	return self->ap_string->s_str + (self->ap_length - length);
err:
	return NULL;
}

STATIC_ASSERT(STRING_WIDTH_1BYTE < 1);

/* Resize a single-byte string to have a length of `num_bytes' bytes.
 * You may pass `NULL' for `self', or a reference to `Dee_EmptyString'
 * in order to allocate and return a new buffer. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeString_ResizeBuffer(DREF DeeObject *self, size_t num_bytes) {
	DREF String *result;
	if unlikely(self == Dee_EmptyString)
		self = NULL;
	ASSERTF(!self || DeeString_Check(self), "Not a string buffer");
	ASSERTF(!self || !DeeObject_IsShared(self), "String buffers cannot be shared");

	/* Special case: Resize-to-zero. */
	if unlikely(!num_bytes) {
		if (self)
			Dee_DecrefDokill(self);
		Dee_Incref(Dee_EmptyString);
		return Dee_EmptyString;
	}

	/* Re-allocate the buffer. */
	result = (DREF String *)DeeObject_Reallocc(self, offsetof(String, s_str),
	                                           num_bytes + 1, sizeof(char));
	if likely(result) {
		if (!self) {
			/* Do the initial init when `self' was `NULL'. */
			DeeObject_Init(result, &DeeString_Type);
			result->s_data = NULL;
			result->s_hash = (Dee_hash_t)-1;
		}
		result->s_len = num_bytes;
		result->s_str[num_bytes] = '\0';
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeString_TryResizeBuffer(DREF DeeObject *self, size_t num_bytes) {
	DREF String *result;
	if unlikely(self == Dee_EmptyString)
		self = NULL;
	ASSERTF(!self || DeeString_Check(self), "Not a string buffer");
	ASSERTF(!self || !DeeObject_IsShared(self), "String buffers cannot be shared");

	/* Special case: Resize-to-zero. */
	if unlikely(!num_bytes) {
		if (self)
			Dee_DecrefDokill(self);
		Dee_Incref(Dee_EmptyString);
		return Dee_EmptyString;
	}

	/* Re-allocate the buffer. */
	result = (DREF String *)DeeObject_TryReallocc(self, offsetof(String, s_str),
	                                              num_bytes + 1, sizeof(char));
	if likely(result) {
		if (!self) {
			/* Do the initial init when `self' was `NULL'. */
			DeeObject_Init(result, &DeeString_Type);
			result->s_data = NULL;
			result->s_hash = (Dee_hash_t)-1;
		}
		result->s_len            = num_bytes;
		result->s_str[num_bytes] = '\0';
	}
	return (DREF DeeObject *)result;
}

/* Construct an uninitialized single-byte string,
 * capable of representing up to `num_bytes' bytes of text. */
#ifdef NDEBUG
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_NewBuffer)(size_t num_bytes)
#else /* NDEBUG */
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeDbgString_NewBuffer)(size_t num_bytes, char const *file, int line)
#endif /* !NDEBUG */
{
	DREF String *result;
	if unlikely(!num_bytes) {
		Dee_Incref(Dee_EmptyString);
		return Dee_EmptyString;
	}
#ifdef NDEBUG
	result = (DREF String *)DeeObject_Mallocc(offsetof(String, s_str),
	                                          num_bytes + 1, sizeof(char));
#else /* NDEBUG */
	result = (DREF String *)DeeDbgObject_Mallocc(offsetof(String, s_str),
	                                             num_bytes + 1, sizeof(char),
	                                             file, line);
#endif /* !NDEBUG */
	if likely(result) {
		DeeObject_Init(result, &DeeString_Type);
		result->s_data           = NULL;
		result->s_hash           = (Dee_hash_t)-1;
		result->s_len            = num_bytes;
		result->s_str[num_bytes] = '\0';
	}
	return (DREF DeeObject *)result;
}

#ifdef NDEBUG
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeDbgString_NewBuffer)(size_t num_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return DeeString_NewBuffer(num_bytes);
}
#else /* NDEBUG */
PUBLIC WUNUSED DREF DeeObject *
(DCALL DeeString_NewBuffer)(size_t num_bytes) {
	return DeeDbgString_NewBuffer(num_bytes, NULL, 0);
}
#endif /* !NDEBUG */



PUBLIC NONNULL((1)) void DCALL
DeeString_FreeWidth(DeeObject *__restrict self) {
	DeeStringObject *me;
	struct string_utf *utf;
	me = (DeeStringObject *)self;
	ASSERTF(DeeString_Check(me), "Not a string buffer");
	ASSERTF(!DeeObject_IsShared(me), "String buffers cannot be shared");
	if ((utf = me->s_data) == NULL)
		return;
	ASSERTF(utf->u_width == STRING_WIDTH_1BYTE, "This isn't a 1-byte string");
	Dee_string_utf_fini(utf, me);
	me->s_data = NULL;
	Dee_string_utf_free(utf);
}

/* Construct strings with basic width-data. */
#ifdef NDEBUG
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeDbgString_NewSized)(/*unsigned latin-1*/ char const *__restrict str,
                              size_t length, char const *file, int line) {
	(void)file;
	(void)line;
	return DeeString_NewSized(str, length);
}
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_NewSized)(/*unsigned latin-1*/ char const *__restrict str, size_t length)
#else /* NDEBUG */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_NewSized)(/*unsigned latin-1*/ char const *__restrict str, size_t length) {
	return DeeDbgString_NewSized(str, length, NULL, 0);
}
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeDbgString_NewSized)(/*unsigned latin-1*/ char const *__restrict str,
                              size_t length, char const *file, int line)
#endif /* !NDEBUG */
{
	DREF DeeObject *result;
	/* Optimization: use pre-allocated latin1 strings
	 *               for single-character sequences. */
	switch (length) {

	case 0:
		return_empty_string;

	case 1:
		return DeeString_Chr((uint8_t)str[0]);

	default:
		break;
	}
#ifdef NDEBUG
	result = DeeString_NewBuffer(length);
#else /* NDEBUG */
	result = DeeDbgString_NewBuffer(length, file, line);
#endif /* !NDEBUG */
	if likely(result) {
		memcpyc(DeeString_STR(result), str,
		        length, sizeof(char));
	}
	return result;
}

/* Construct a new, non-decoded single-byte-per-character string `str'.
 * The string itself may contain characters above 127, which are then
 * interpreted as part of the unicode character-range U+0080...U+00FF. */
#ifdef NDEBUG
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_New)(/*unsigned*/ char const *__restrict str) {
	return DeeString_NewSized(str, strlen(str));
}
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeDbgString_New)(/*unsigned*/ char const *__restrict str,
                         char const *file, int line) {
	(void)file;
	(void)line;
	return DeeString_New(str);
}
#else /* NDEBUG */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_New)(/*unsigned*/ char const *__restrict str) {
	return DeeDbgString_New(str, NULL, 0);
}
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeDbgString_New)(/*unsigned*/ char const *__restrict str,
                         char const *file, int line) {
	return DeeDbgString_NewSized(str, strlen(str), file, line);
}
#endif /* !NDEBUG */


PRIVATE WUNUSED DREF DeeObject *DCALL
string_new_empty(void) {
	return_empty_string;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
string_new(size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	if (DeeArg_Unpack(argc, argv, "o:string", &ob))
		goto err;
	return DeeObject_Str(ob);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_VNewf(/*utf-8*/ char const *__restrict format, va_list args) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(unicode_printer_vprintf(&printer, format, args) < 0)
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

/* Construct a new string using printf-like (and deemon-enhanced) format-arguments. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
DeeString_Newf(/*utf-8*/ char const *__restrict format, ...) {
	va_list args;
	DREF DeeObject *result;
	va_start(args, format);
	result = DeeString_VNewf(format, args);
	va_end(args);
	return result;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
string_bool(String *__restrict self) {
	return !DeeString_IsEmpty(self);
}

#ifdef Dee_BOUND_PRESENT_MAYALIAS_BOOL
#define string_bool_asbound string_bool
#else /* Dee_BOUND_PRESENT_MAYALIAS_BOOL */
PRIVATE WUNUSED NONNULL((1)) int DCALL
string_bool_asbound(String *__restrict self) {
	return Dee_BOUND_FROMBOOL(!DeeString_IsEmpty(self));
}
#endif /* !Dee_BOUND_PRESENT_MAYALIAS_BOOL */

INTERN ATTR_PURE WUNUSED NONNULL((1)) Dee_hash_t DCALL
DeeString_Hash(DeeObject *__restrict self) {
	Dee_hash_t result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	result = DeeString_HASH(self);
	if (result == (Dee_hash_t)-1) {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			result = Dee_HashPtr(DeeString_STR(self),
			                     DeeString_SIZE(self));
			break;

		CASE_WIDTH_2BYTE: {
			uint16_t *str;
			str    = DeeString_Get2Byte(self);
			result = Dee_Hash2Byte(str, WSTR_LENGTH(str));
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t *str;
			str    = DeeString_Get4Byte(self);
			result = Dee_Hash4Byte(str, WSTR_LENGTH(str));
		}	break;

		}
		DeeString_HASH(self) = result;
	}
	return result;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) Dee_hash_t DCALL
DeeString_HashCase(DeeObject *__restrict self) {
	Dee_hash_t result;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		result = Dee_HashCasePtr(DeeString_STR(self),
		                         DeeString_SIZE(self));
		break;

	CASE_WIDTH_2BYTE: {
		uint16_t *str;
		str    = DeeString_Get2Byte(self);
		result = Dee_HashCase2Byte(str, WSTR_LENGTH(str));
	}	break;

	CASE_WIDTH_4BYTE: {
		uint32_t *str;
		str    = DeeString_Get4Byte(self);
		result = Dee_HashCase4Byte(str, WSTR_LENGTH(str));
	}	break;

	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_repr(DeeObject *__restrict self) {
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if unlikely(DeeString_PrintRepr(self, &ascii_printer_print, &printer) < 0)
		goto err;
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}


/* Destroy the regex cache associated with `self'.
 * Called from `DeeString_Type.tp_fini' when `STRING_UTF_FREGEX' was set. */
INTDEF NONNULL((1)) void DCALL /* From "./unicode/regex.c" */
DeeString_DestroyRegex(String *__restrict self);

PRIVATE NONNULL((1)) void DCALL
string_fini(String *__restrict self) {
	struct string_utf *utf;
	/* Clean up UTF data. */
	if ((utf = self->s_data) != NULL) {
		/* If present */
		if unlikely(utf->u_flags & STRING_UTF_FREGEX)
			DeeString_DestroyRegex(self);

		Dee_string_utf_fini(utf, self);
		Dee_string_utf_free(utf);
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
compare_string_bytes(String *__restrict lhs,
                     DeeBytesObject *__restrict rhs) {
	size_t lhs_len;
	size_t rhs_len;
	if (!lhs->s_data ||
	    lhs->s_data->u_width == STRING_WIDTH_1BYTE) {
		uint8_t const *lhs_str;
		int result;
		/* Compare against single-byte string. */
		lhs_str = (uint8_t const *)lhs->s_str;
		lhs_len = lhs->s_len;
		rhs_len = DeeBytes_SIZE(rhs);
		/* Most simple case: compare ascii/single-byte strings. */
		result = memcmp(lhs_str, DeeBytes_DATA(rhs), MIN(lhs_len, rhs_len));
		if (result != 0)
			return Dee_CompareFromDiff(result);
	} else {
		byte_t const *rhs_str;
		struct string_utf *lhs_utf;
		/* Compare against single-byte string. */
		rhs_str = DeeBytes_DATA(rhs);
		rhs_len = DeeBytes_SIZE(rhs);
		lhs_utf = lhs->s_data;
		switch (lhs_utf->u_width) {

		CASE_WIDTH_2BYTE: {
			uint16_t *lhs_str;
			size_t i, common_len;
			lhs_str    = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len    = WSTR_LENGTH(lhs_str);
			common_len = MIN(rhs_len, lhs_len);
			for (i = 0; i < common_len; ++i) {
				if (lhs_str[i] == (uint16_t)(rhs_str[i]))
					continue;
				return lhs_str[i] < (uint16_t)(rhs_str[i]) ? -1 : 1;
			}
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t *lhs_str;
			size_t i, common_len;
			lhs_str    = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len    = WSTR_LENGTH(lhs_str);
			common_len = MIN(rhs_len, lhs_len);
			for (i = 0; i < common_len; ++i) {
				if (lhs_str[i] == (uint32_t)(rhs_str[i]))
					continue;
				return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
			}
		}	break;

		default: __builtin_unreachable();
		}
	}

	/* If string contents are identical, leave off by comparing their lengths. */
	if (lhs_len == rhs_len)
		return 0;
	if (lhs_len < rhs_len)
		return -1;
	return 1;
}

#ifndef CONFIG_HAVE_memcmpw
#define CONFIG_HAVE_memcmpw
#define memcmpw dee_memcmpw
DeeSystem_DEFINE_memcmpw(dee_memcmpw)
#endif /* !CONFIG_HAVE_memcmpw */

#ifndef CONFIG_HAVE_memcmpl
#define CONFIG_HAVE_memcmpl
#define memcmpl dee_memcmpl
DeeSystem_DEFINE_memcmpl(dee_memcmpl)
#endif /* !CONFIG_HAVE_memcmpl */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
compare_strings(String *__restrict lhs,
                String *__restrict rhs) {
	size_t lhs_len;
	size_t rhs_len;
	if (!lhs->s_data ||
	    lhs->s_data->u_width == STRING_WIDTH_1BYTE) {
		uint8_t const *lhs_str;
		/* Compare against single-byte string. */
		lhs_str = (uint8_t const *)lhs->s_str;
		lhs_len = lhs->s_len;
		if (!rhs->s_data ||
		    rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
			int result;
			rhs_len = rhs->s_len;
			/* Most simple case: compare ascii/single-byte strings. */
			result = memcmp(lhs_str, rhs->s_str, MIN(lhs_len, rhs_len));
			if (result != 0)
				return Dee_CompareFromDiff(result);
		} else {
			struct string_utf *rhs_utf = rhs->s_data;
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				uint16_t *rhs_str;
				size_t i, common_len;
				rhs_str    = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len    = WSTR_LENGTH(rhs_str);
				common_len = MIN(lhs_len, rhs_len);
				for (i = 0; i < common_len; ++i) {
					if ((uint16_t)(lhs_str[i]) == rhs_str[i])
						continue;
					return (uint16_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
				}
			}	break;

			CASE_WIDTH_4BYTE: {
				uint32_t *rhs_str;
				size_t i, common_len;
				rhs_str    = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len    = WSTR_LENGTH(rhs_str);
				common_len = MIN(lhs_len, rhs_len);
				for (i = 0; i < common_len; ++i) {
					if ((uint32_t)(lhs_str[i]) == rhs_str[i])
						continue;
					return (uint32_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
				}
			}	break;

			default: __builtin_unreachable();
			}
		}
	} else if (!rhs->s_data ||
	           rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
		uint8_t const *rhs_str;
		struct string_utf *lhs_utf;
		/* Compare against single-byte string. */
		rhs_str = (uint8_t const *)rhs->s_str;
		rhs_len = rhs->s_len;
		lhs_utf = lhs->s_data;
		switch (lhs_utf->u_width) {

		CASE_WIDTH_2BYTE: {
			uint16_t const *lhs_str;
			size_t i, common_len;
			lhs_str    = (uint16_t const *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len    = WSTR_LENGTH(lhs_str);
			common_len = MIN(rhs_len, lhs_len);
			for (i = 0; i < common_len; ++i) {
				if (lhs_str[i] == (uint16_t)(rhs_str[i]))
					continue;
				return lhs_str[i] < (uint16_t)(rhs_str[i]) ? -1 : 1;
			}
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t const *lhs_str;
			size_t i, common_len;
			lhs_str    = (uint32_t const *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len    = WSTR_LENGTH(lhs_str);
			common_len = MIN(rhs_len, lhs_len);
			for (i = 0; i < common_len; ++i) {
				if (lhs_str[i] == (uint32_t)(rhs_str[i]))
					continue;
				return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
			}
		}	break;

		default: __builtin_unreachable();
		}
	} else {
		struct string_utf const *lhs_utf;
		struct string_utf const *rhs_utf;
		lhs_utf = lhs->s_data;
		rhs_utf = rhs->s_data;
		ASSERT(lhs_utf != NULL);
		ASSERT(rhs_utf != NULL);
		ASSERT(lhs_utf->u_width != STRING_WIDTH_1BYTE);
		ASSERT(rhs_utf->u_width != STRING_WIDTH_1BYTE);
		/* Complex string comparison. */
		switch (lhs_utf->u_width) {

		CASE_WIDTH_2BYTE: {
			uint16_t const *lhs_str;
			lhs_str = (uint16_t const *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				int result;
				uint16_t const *rhs_str;
				size_t common_len;
				rhs_str    = (uint16_t const *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len    = WSTR_LENGTH(rhs_str);
				common_len = MIN(lhs_len, rhs_len);
				result     = memcmpw(lhs_str, rhs_str, common_len);
				if (result != 0)
					return Dee_CompareFromDiff(result);
			}	break;

			CASE_WIDTH_4BYTE: {
				uint32_t const *rhs_str;
				size_t i, common_len;
				rhs_str    = (uint32_t const *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len    = WSTR_LENGTH(rhs_str);
				common_len = MIN(lhs_len, rhs_len);
				for (i = 0; i < common_len; ++i) {
					if ((uint32_t)(lhs_str[i]) == rhs_str[i])
						continue;
					return (uint32_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
				}
			}	break;

			default: __builtin_unreachable();
			}
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t const *lhs_str;
			lhs_str = (uint32_t const *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				uint16_t const *rhs_str;
				size_t i, common_len;
				rhs_str    = (uint16_t const *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len    = WSTR_LENGTH(rhs_str);
				common_len = MIN(lhs_len, rhs_len);
				for (i = 0; i < common_len; ++i) {
					if (lhs_str[i] == (uint32_t)(rhs_str[i]))
						continue;
					return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
				}
			}	break;

			CASE_WIDTH_4BYTE: {
				int result;
				uint32_t const *rhs_str;
				size_t common_len;
				rhs_str    = (uint32_t const *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len    = WSTR_LENGTH(rhs_str);
				common_len = MIN(lhs_len, rhs_len);
				result     = memcmpl(lhs_str, rhs_str, common_len);
				if (result != 0)
					return Dee_CompareFromDiff(result);
			}	break;

			default: __builtin_unreachable();
			}
		}	break;

		default: __builtin_unreachable();
		}
	}

	/* If string contents are identical, leave off by comparing their lengths. */
	if (lhs_len == rhs_len)
		return 0;
	if (lhs_len < rhs_len)
		return -1;
	return 1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_compare(String *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if likely(tp_rhs == &DeeString_Type)
		return compare_strings(lhs, (String *)rhs);
	if likely(tp_rhs == &DeeBytes_Type)
		return compare_string_bytes(lhs, (DeeBytesObject *)rhs);
	DeeObject_TypeAssertFailed2(rhs, &DeeString_Type, &DeeBytes_Type);
	return Dee_COMPARE_ERR;
}

INTDEF WUNUSED NONNULL((1, 2)) bool DCALL
string_eq_bytes(String *__restrict self,
                DeeBytesObject *__restrict other);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_compare_eq(String *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if likely(tp_rhs == &DeeString_Type) {
		if (lhs == (String *)rhs)
			return 0;
		if (DeeString_Hash((DeeObject *)lhs) !=
		    DeeString_Hash((DeeObject *)rhs))
			return 1;
		return compare_strings(lhs, (String *)rhs);
	}
	if likely(tp_rhs == &DeeBytes_Type)
		return !string_eq_bytes(lhs, (DeeBytesObject *)rhs);
	DeeObject_TypeAssertFailed2(rhs, &DeeString_Type, &DeeBytes_Type);
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_trycompare_eq(String *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if likely(tp_rhs == &DeeString_Type) {
		if (lhs == (String *)rhs)
			return 0;
		if (DeeString_Hash((DeeObject *)lhs) !=
		    DeeString_Hash((DeeObject *)rhs))
			return 1;
		return compare_strings(lhs, (String *)rhs);
	}
	if likely(tp_rhs == &DeeBytes_Type)
		return !string_eq_bytes(lhs, (DeeBytesObject *)rhs);
	return 1;
}


struct string_compare_seq_data {
	size_t               *scsd_wstr;  /* [1..1] The LHS width-string */
	size_t                scsd_index; /* Index to next character */
	__UINTPTR_HALF_TYPE__ scsd_width; /* String switch (one of `Dee_STRING_WIDTH_*BYTE') */
};

/* @return: 0 : lhs == rhs (for now...)
 * @return: -1: Error
 * @return: -2: lhs < rhs
 * @return: -3: lhs > rhs */
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
string_compare_seq_cb(void *arg, DeeObject *rhs_elem) {
	uint32_t lhs_ord, rhs_ord;
	struct string_compare_seq_data *data;
	data = (struct string_compare_seq_data *)arg;
	if (data->scsd_index >= WSTR_LENGTH(data->scsd_wstr))
		return -2; /* lhs < rhs */
	if (DeeString_Check(rhs_elem)) {
		if (DeeString_WLEN(rhs_elem) != 1)
			goto err_wrong_length;
		rhs_ord = DeeString_GetChar(rhs_elem, 0);
	} else if (DeeBytes_Check(rhs_elem)) {
		if (DeeBytes_SIZE(rhs_elem) != 1)
			goto err_wrong_length;
		rhs_ord = DeeBytes_DATA(rhs_elem)[0];
	} else {
		DeeObject_TypeAssertFailed(rhs_elem, &DeeString_Type);
		goto err;
	}
	SWITCH_SIZEOF_WIDTH(data->scsd_width) {
	CASE_WIDTH_1BYTE:
		lhs_ord = ((uint8_t const *)data->scsd_wstr)[data->scsd_index];
		break;
	CASE_WIDTH_2BYTE:
		lhs_ord = ((uint16_t const *)data->scsd_wstr)[data->scsd_index];
		break;
	CASE_WIDTH_4BYTE:
		lhs_ord = ((uint32_t const *)data->scsd_wstr)[data->scsd_index];
		break;
	}
	++data->scsd_index;
	if (lhs_ord == rhs_ord)
		return 0;
	if (lhs_ord < rhs_ord)
		return -2; /* lhs < rhs */
	return -3;     /* lhs > rhs */
err_wrong_length:
	err_expected_single_character_string(rhs_elem);
err:
	return -1;
}


/* Also accept "rhs" complaying with `{(string | Bytes)...}'
 * - string: Must be a single character
 * - Bytes:  Must be a single byte
 * @return: -1: lhs < rhs
 * @return:  0: Equal
 * @return:  1: lhs > rhs
 * @return: Dee_COMPARE_ERR: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_compare_seq(String *lhs, DeeObject *rhs) {
	Dee_ssize_t foreach_status;
	struct string_compare_seq_data data;
	data.scsd_wstr  = DeeString_WSTR(lhs);
	data.scsd_index = 0;
	data.scsd_width = DeeString_WIDTH(lhs);
	foreach_status = DeeObject_Foreach(rhs, &string_compare_seq_cb, &data);
	ASSERT(foreach_status == 0 || foreach_status == -1 ||
	       foreach_status == -2 || foreach_status == -3);
	if unlikely(foreach_status == -1)
		goto err;
	if unlikely(foreach_status == -2)
		return -1; /* lhs < rhs */
	if unlikely(foreach_status == -3)
		return 1; /* lhs > rhs */
	if (data.scsd_index < WSTR_LENGTH(data.scsd_wstr))
		return 1; /* lhs > rhs */
	return 0;
err:
	return Dee_COMPARE_ERR;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_mh_seq_compare(String *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if likely(tp_rhs == &DeeString_Type)
		return compare_strings(lhs, (String *)rhs);
	if likely(tp_rhs == &DeeBytes_Type)
		return compare_string_bytes(lhs, (DeeBytesObject *)rhs);
	return string_compare_seq(lhs, rhs);
}

INTDEF WUNUSED NONNULL((1, 2)) bool DCALL
string_eq_bytes(String *__restrict self,
                DeeBytesObject *__restrict other);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_mh_seq_compare_eq(String *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if likely(tp_rhs == &DeeString_Type) {
		if (lhs == (String *)rhs)
			return 0;
		if (DeeString_Hash((DeeObject *)lhs) !=
		    DeeString_Hash((DeeObject *)rhs))
			return 1;
		return compare_strings(lhs, (String *)rhs);
	}
	if likely(tp_rhs == &DeeBytes_Type)
		return !string_eq_bytes(lhs, (DeeBytesObject *)rhs);
	return string_compare_seq(lhs, rhs);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_mh_seq_trycompare_eq(String *lhs, DeeObject *rhs) {
	DeeTypeObject *tp_rhs = Dee_TYPE(rhs);
	if likely(tp_rhs == &DeeString_Type) {
		if (lhs == (String *)rhs)
			return 0;
		if (DeeString_Hash((DeeObject *)lhs) !=
		    DeeString_Hash((DeeObject *)rhs))
			return 1;
		return compare_strings(lhs, (String *)rhs);
	}
	if likely(tp_rhs == &DeeBytes_Type)
		return !string_eq_bytes(lhs, (DeeBytesObject *)rhs);
	if (!DeeType_HasNativeOperator(tp_rhs, foreach))
		return 1;
	return string_compare_seq(lhs, rhs);
}

typedef struct {
	PROXY_OBJECT_HEAD_EX(String, si_string); /* [1..1][const] The string that is being iterated. */
	union dcharptr               si_iter;    /* [1..1][weak] The current iterator position. */
	union dcharptr               si_end;     /* [1..1][const] The string end pointer. */
	unsigned int                 si_width;   /* [const] The stirng width used during iteration (One of `STRING_WIDTH_*'). */
} StringIterator;
#define READ_ITER_PTR(x) atomic_read(&(x)->si_iter.ptr)

STATIC_ASSERT(offsetof(StringIterator, si_string) == offsetof(ProxyObject, po_obj));
#define stringiter_fini  generic_proxy__fini
#define stringiter_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stringiter_next(StringIterator *__restrict self) {
	DREF DeeObject *result;
	union dcharptr pos, new_pos;

	/* Consume one character (atomically) */
	do {
		pos.ptr = atomic_read(&self->si_iter.ptr);
		if (pos.ptr >= self->si_end.ptr)
			return ITER_DONE;
		new_pos.cp8 = pos.cp8 + STRING_SIZEOF_WIDTH(self->si_width);
	} while (!atomic_cmpxch_weak_or_write(&self->si_iter.ptr, pos.ptr, new_pos.ptr));

	/* Create the single-character string. */
	SWITCH_SIZEOF_WIDTH(self->si_width) {

	CASE_WIDTH_1BYTE:
		result = DeeString_Chr(*pos.cp8);
		break;

	CASE_WIDTH_2BYTE:
		result = DeeString_Chr(*pos.cp16);
		break;

	CASE_WIDTH_4BYTE:
		result = DeeString_Chr(*pos.cp32);
		break;

	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringiter_bool(StringIterator *__restrict self) {
	return READ_ITER_PTR(self) < self->si_end.ptr;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringiter_ctor(StringIterator *__restrict self) {
	self->si_string   = (DREF String *)Dee_EmptyString;
	self->si_iter.ptr = DeeString_STR(Dee_EmptyString);
	self->si_end.ptr  = DeeString_STR(Dee_EmptyString);
	self->si_width    = STRING_WIDTH_1BYTE;
	Dee_Incref(Dee_EmptyString);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
stringiter_copy(StringIterator *__restrict self,
                StringIterator *__restrict other) {
	self->si_string   = other->si_string;
	self->si_iter.ptr = READ_ITER_PTR(other);
	self->si_end      = other->si_end;
	self->si_width    = other->si_width;
	Dee_Incref(self->si_string);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringiter_init(StringIterator *__restrict self,
                size_t argc, DeeObject *const *argv) {
	String *str;
	if (DeeArg_Unpack(argc, argv, "o:string.Iterator", &str))
		goto err;
	if (DeeObject_AssertTypeExact(str, &DeeString_Type))
		goto err;
	self->si_string = str;
	Dee_Incref(str);
	self->si_width    = DeeString_WIDTH(str);
	self->si_iter.ptr = DeeString_WSTR(str);
	self->si_end.ptr = (void *)((uintptr_t)self->si_iter.ptr +
	                            WSTR_LENGTH(self->si_iter.ptr) *
	                            STRING_SIZEOF_WIDTH(self->si_width));
	return 0;
err:
	return -1;
}

PRIVATE struct type_member tpconst stringiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(StringIterator, si_string), "->?Dstring"),
	TYPE_MEMBER_FIELD("__width__", STRUCT_CONST | STRUCT_INT, offsetof(StringIterator, si_width)),
	TYPE_MEMBER_END
};

INTDEF DeeTypeObject StringIterator_Type;

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
stringiter_hash(StringIterator *self) {
	return Dee_HashPointer(READ_ITER_PTR(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
stringiter_compare(StringIterator *self, StringIterator *other) {
	if (DeeObject_AssertTypeExact(other, &StringIterator_Type))
		goto err;
	Dee_return_compareT(void *, READ_ITER_PTR(self),
	                    /*   */ READ_ITER_PTR(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
stringiter_nii_getseq(StringIterator *__restrict self) {
	return_reference_(self->si_string);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringiter_nii_hasprev(StringIterator *__restrict self) {
	return READ_ITER_PTR(self) > (void *)DeeString_WSTR(self->si_string);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
stringiter_nii_getindex(StringIterator *__restrict self) {
	union dcharptr pos;
	pos.ptr = READ_ITER_PTR(self);
	return (size_t)(pos.cp8 - (byte_t const *)DeeString_WSTR(self->si_string)) /
	       STRING_SIZEOF_WIDTH(self->si_width);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringiter_nii_setindex(StringIterator *__restrict self, size_t index) {
	if (index > DeeString_WLEN(self->si_string))
		index = DeeString_WLEN(self->si_string);
	self->si_iter.cp8 = (byte_t *)DeeString_WSTR(self->si_string) +
	                    (index * STRING_SIZEOF_WIDTH(self->si_width));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringiter_nii_rewind(StringIterator *__restrict self) {
	atomic_write(&self->si_iter.ptr,
	             DeeString_WSTR(self->si_string));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringiter_nii_prev(StringIterator *__restrict self) {
	union dcharptr pos, new_pos;
	do {
		pos.ptr = atomic_read(&self->si_iter.ptr);
		if (pos.ptr <= (void *)DeeString_WSTR(self->si_string))
			return 1;
		new_pos.cp8 = pos.cp8 - STRING_SIZEOF_WIDTH(self->si_width);
	} while (!atomic_cmpxch_weak_or_write(&self->si_iter.ptr, pos.ptr, new_pos.ptr));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
stringiter_nii_next(StringIterator *__restrict self) {
	union dcharptr pos, new_pos;

	/* Consume one character (atomically) */
	do {
		pos.ptr = atomic_read(&self->si_iter.ptr);
		if (pos.ptr >= self->si_end.ptr)
			return 1;
		new_pos.cp8 = pos.cp8 + STRING_SIZEOF_WIDTH(self->si_width);
	} while (!atomic_cmpxch_weak_or_write(&self->si_iter.ptr, pos.ptr, new_pos.ptr));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stringiter_nii_peek(StringIterator *__restrict self) {
	DREF DeeObject *result;
	union dcharptr pos;
	pos.ptr = self->si_iter.ptr;
	if (pos.ptr >= self->si_end.ptr)
		return ITER_DONE;

	/* Create the single-character string. */
	SWITCH_SIZEOF_WIDTH(self->si_width) {

	CASE_WIDTH_1BYTE:
		result = DeeString_Chr(*pos.cp8);
		break;

	CASE_WIDTH_2BYTE:
		result = DeeString_Chr(*pos.cp16);
		break;

	CASE_WIDTH_4BYTE:
		result = DeeString_Chr(*pos.cp32);
		break;

	}
	return result;
}

PRIVATE struct type_nii tpconst stringiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&stringiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&stringiter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&stringiter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&stringiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL, //TODO:&stringiter_nii_revert,
			/* .nii_advance  = */ (dfunptr_t)NULL, //TODO:&stringiter_nii_advance,
			/* .nii_prev     = */ (dfunptr_t)&stringiter_nii_prev,
			/* .nii_next     = */ (dfunptr_t)&stringiter_nii_next,
			/* .nii_hasprev  = */ (dfunptr_t)&stringiter_nii_hasprev,
			/* .nii_peek     = */ (dfunptr_t)&stringiter_nii_peek
		}
	}
};


PRIVATE struct type_cmp stringiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&stringiter_hash,
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&stringiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &stringiter_nii,
};

INTERN DeeTypeObject StringIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StringIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(s:?Dstring)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&stringiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&stringiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&stringiter_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&stringiter_init,
				TYPE_FIXED_ALLOCATOR(StringIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&stringiter_fini,
		/* .tp_assign      = */ NULL, /* TODO */
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&stringiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&stringiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &stringiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stringiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ stringiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__E31EBEB26CC72F83),
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_iter(String *__restrict self) {
	DREF StringIterator *result;
	result = DeeObject_MALLOC(StringIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &StringIterator_Type);
	result->si_string = self;
	Dee_Incref(self);
	result->si_width    = DeeString_WIDTH(self);
	result->si_iter.ptr = DeeString_WSTR(self);
	result->si_end.ptr = (void *)((uintptr_t)result->si_iter.ptr +
	                              WSTR_LENGTH(result->si_iter.ptr) *
	                              STRING_SIZEOF_WIDTH(result->si_width));
done:
	return (DREF DeeObject *)result;
}

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_contains(String *self, DeeObject *some_object);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_sizeob(String *__restrict self) {
	size_t result = DeeString_WLEN(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
string_size(String *__restrict self) {
	ASSERT(DeeString_WLEN(self) != (size_t)-1);
	return DeeString_WLEN(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_getitem_index(String *__restrict self, size_t index) {
	int width = DeeString_WIDTH(self);
	union dcharptr str;
	size_t len;
	str.ptr = DeeString_WSTR(self);
	len     = WSTR_LENGTH(str.ptr);
	if unlikely(index >= len)
		goto err_oob;
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		return DeeString_Chr(str.cp8[index]);

	CASE_WIDTH_2BYTE:
		return DeeString_Chr(str.cp16[index]);

	CASE_WIDTH_4BYTE:
		return DeeString_Chr(str.cp32[index]);

	}
err_oob:
	err_index_out_of_bounds((DeeObject *)self, index, len);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_trygetitem_index(String *__restrict self, size_t index) {
	int width = DeeString_WIDTH(self);
	union dcharptr str;
	size_t len;
	str.ptr = DeeString_WSTR(self);
	len     = WSTR_LENGTH(str.ptr);
	if unlikely(index >= len)
		goto err_oob;
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		return DeeString_Chr(str.cp8[index]);

	CASE_WIDTH_2BYTE:
		return DeeString_Chr(str.cp16[index]);

	CASE_WIDTH_4BYTE:
		return DeeString_Chr(str.cp32[index]);

	}
err_oob:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_getrange_index(String *__restrict self,
                      Dee_ssize_t begin, Dee_ssize_t end) {
	struct Dee_seq_range range;
	size_t range_size;
	void *str  = DeeString_WSTR(self);
	int width  = DeeString_WIDTH(self);
	size_t len = WSTR_LENGTH(str);
	DeeSeqRange_Clamp(&range, begin, end, len);
	range_size = range.sr_end - range.sr_start;
	if unlikely(range_size <= 0)
		return_empty_string;
	return DeeString_NewWithWidth((byte_t *)str +
	                              (range.sr_start * STRING_SIZEOF_WIDTH(width)),
	                              range_size, width);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_getrange_index_n(String *__restrict self, Dee_ssize_t begin) {
#ifdef __OPTIMIZE_SIZE__
	return string_getrange_index(self, begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	size_t range_size, start;
	void *str  = DeeString_WSTR(self);
	int width  = DeeString_WIDTH(self);
	size_t len = WSTR_LENGTH(str);
	start = DeeSeqRange_Clamp_n(begin, len);
	range_size = len - start;
	if unlikely(range_size <= 0)
		return_empty_string;
	return DeeString_NewWithWidth((byte_t *)str +
	                              (start * STRING_SIZEOF_WIDTH(width)),
	                              range_size, width);
#endif /* !__OPTIMIZE_SIZE__ */
}


PRIVATE struct type_cmp string_cmp = {
	/* .tp_hash          = */ &DeeString_Hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&string_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&string_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&string_trycompare_eq,
	/* .tp_eq            = */ &default__eq__with__compare_eq,
	/* .tp_ne            = */ &default__ne__with__compare_eq,
	/* .tp_lo            = */ &default__lo__with__compare,
	/* .tp_le            = */ &default__le__with__compare,
	/* .tp_gr            = */ &default__gr__with__compare,
	/* .tp_ge            = */ &default__ge__with__compare,
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
string_foreach(String *self, Dee_foreach_t proc, void *arg) {
	union dcharptr ptr, end;
	Dee_ssize_t temp, result = 0;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		ptr.cp8 = DeeString_Get1Byte((DeeObject *)self);
		end.cp8 = ptr.cp8 + WSTR_LENGTH(ptr.cp8);
		while (ptr.cp8 < end.cp8) {
#ifdef CONFIG_STRING_LATIN1_STATIC
			temp = (*proc)(arg, (DeeObject *)&DeeString_Latin1[*ptr.cp8]);
#else /* CONFIG_STRING_LATIN1_STATIC */
			DREF DeeObject *elem;
			elem = DeeString_Chr(*ptr.cp8);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
#endif /* !CONFIG_STRING_LATIN1_STATIC */
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
			++ptr.cp8;
		}
		break;

	CASE_WIDTH_2BYTE:
		ptr.cp16 = DeeString_Get2Byte((DeeObject *)self);
		end.cp16 = ptr.cp16 + WSTR_LENGTH(ptr.cp16);
		while (ptr.cp16 < end.cp16) {
			DREF DeeObject *elem;
			elem = DeeString_Chr(*ptr.cp16);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
			++ptr.cp16;
		}
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = DeeString_Get4Byte((DeeObject *)self);
		end.cp32 = ptr.cp32 + WSTR_LENGTH(ptr.cp32);
		while (ptr.cp32 < end.cp32) {
			DREF DeeObject *elem;
			elem = DeeString_Chr(*ptr.cp32);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
			++ptr.cp32;
		}
		break;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
string_foreach_reverse(String *self, Dee_foreach_t proc, void *arg) {
	union dcharptr ptr, end;
	Dee_ssize_t temp, result = 0;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		ptr.cp8 = DeeString_Get1Byte((DeeObject *)self);
		end.cp8 = ptr.cp8 + WSTR_LENGTH(ptr.cp8);
		while (ptr.cp8 < end.cp8) {
			--end.cp8;
#ifdef CONFIG_STRING_LATIN1_STATIC
			temp = (*proc)(arg, (DeeObject *)&DeeString_Latin1[*end.cp8]);
#else /* CONFIG_STRING_LATIN1_STATIC */
			DREF DeeObject *elem;
			elem = DeeString_Chr(*end.cp8);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
#endif /* !CONFIG_STRING_LATIN1_STATIC */
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		break;

	CASE_WIDTH_2BYTE:
		ptr.cp16 = DeeString_Get2Byte((DeeObject *)self);
		end.cp16 = ptr.cp16 + WSTR_LENGTH(ptr.cp16);
		while (ptr.cp16 < end.cp16) {
			DREF DeeObject *elem;
			--end.cp16;
			elem = DeeString_Chr(*end.cp16);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = DeeString_Get4Byte((DeeObject *)self);
		end.cp32 = ptr.cp32 + WSTR_LENGTH(ptr.cp32);
		while (ptr.cp32 < end.cp32) {
			DREF DeeObject *elem;
			--end.cp32;
			elem = DeeString_Chr(*end.cp32);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
		break;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
string_mh_seq_enumerate_index(String *self, Dee_seq_enumerate_index_t cb,
                              void *arg, size_t start, size_t end) {
	union dcharptr ptr;
	Dee_ssize_t temp, result = 0;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE: {
		size_t i;
		ptr.cp8 = DeeString_Get1Byte((DeeObject *)self);
		if (end > WSTR_LENGTH(ptr.cp8))
			end = WSTR_LENGTH(ptr.cp8);
		for (i = start; i < end; ++i) {
			uint8_t ch = ptr.cp8[i];
#ifdef CONFIG_STRING_LATIN1_STATIC
			temp = (*cb)(arg, i, (DeeObject *)&DeeString_Latin1[ch]);
#else /* CONFIG_STRING_LATIN1_STATIC */
			DREF DeeObject *elem;
			elem = DeeString_Chr(ch);
			if unlikely(!elem)
				goto err;
			temp = (*cb)(arg, i, elem);
			Dee_Decref(elem);
#endif /* !CONFIG_STRING_LATIN1_STATIC */
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}	break;

	CASE_WIDTH_2BYTE: {
		size_t i;
		ptr.cp16 = DeeString_Get2Byte((DeeObject *)self);
		if (end > WSTR_LENGTH(ptr.cp16))
			end = WSTR_LENGTH(ptr.cp16);
		for (i = start; i < end; ++i) {
			uint16_t ch = ptr.cp16[i];
			DREF DeeObject *elem;
			elem = DeeString_Chr(ch);
			if unlikely(!elem)
				goto err;
			temp = (*cb)(arg, i, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}	break;

	CASE_WIDTH_4BYTE: {
		size_t i;
		ptr.cp32 = DeeString_Get4Byte((DeeObject *)self);
		if (end > WSTR_LENGTH(ptr.cp32))
			end = WSTR_LENGTH(ptr.cp32);
		for (i = start; i < end; ++i) {
			uint32_t ch = ptr.cp32[i];
			DREF DeeObject *elem;
			elem = DeeString_Chr(ch);
			if unlikely(!elem)
				goto err;
			temp = (*cb)(arg, i, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}	break;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
string_mh_seq_enumerate_index_reverse(String *self, Dee_seq_enumerate_index_t cb,
                                      void *arg, size_t start, size_t end) {
	union dcharptr ptr;
	Dee_ssize_t temp, result = 0;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE: {
		ptr.cp8 = DeeString_Get1Byte((DeeObject *)self);
		if (end > WSTR_LENGTH(ptr.cp8))
			end = WSTR_LENGTH(ptr.cp8);
		while (end > start) {
			uint8_t ch = ptr.cp8[--end];
#ifdef CONFIG_STRING_LATIN1_STATIC
			temp = (*cb)(arg, end, (DeeObject *)&DeeString_Latin1[ch]);
#else /* CONFIG_STRING_LATIN1_STATIC */
			DREF DeeObject *elem;
			elem = DeeString_Chr(ch);
			if unlikely(!elem)
				goto err;
			temp = (*cb)(arg, end, elem);
			Dee_Decref(elem);
#endif /* !CONFIG_STRING_LATIN1_STATIC */
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}	break;

	CASE_WIDTH_2BYTE: {
		ptr.cp16 = DeeString_Get2Byte((DeeObject *)self);
		if (end > WSTR_LENGTH(ptr.cp16))
			end = WSTR_LENGTH(ptr.cp16);
		while (end > start) {
			uint16_t ch = ptr.cp16[--end];
			DREF DeeObject *elem;
			elem = DeeString_Chr(ch);
			if unlikely(!elem)
				goto err;
			temp = (*cb)(arg, end, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}	break;

	CASE_WIDTH_4BYTE: {
		ptr.cp32 = DeeString_Get4Byte((DeeObject *)self);
		if (end > WSTR_LENGTH(ptr.cp32))
			end = WSTR_LENGTH(ptr.cp32);
		while (end > start) {
			uint32_t ch = ptr.cp32[--end];
			DREF DeeObject *elem;
			elem = DeeString_Chr(ch);
			if unlikely(!elem)
				goto err;
			temp = (*cb)(arg, end, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		}
	}	break;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
string_asvector(String *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t result;
	union dcharptr ptr, end;
	DREF DeeObject **dst_iter = dst;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		ptr.cp8 = DeeString_Get1Byte((DeeObject *)self);
		result  = WSTR_LENGTH(ptr.cp8);
		if unlikely(dst_length < result)
			break;
		end.cp8 = ptr.cp8 + result;
		for (; ptr.cp8 < end.cp8; ++ptr.cp8) {
			DREF DeeObject *chr;
#ifdef CONFIG_STRING_LATIN1_STATIC
			chr = (DeeObject *)&DeeString_Latin1[*ptr.cp8];
			Dee_Incref(chr);
#else /* CONFIG_STRING_LATIN1_STATIC */
			chr = DeeString_Chr(*ptr.cp8);
			if unlikely(!chr)
				goto err_dst_iter;
#endif /* !CONFIG_STRING_LATIN1_STATIC */
			*dst_iter++ = chr;
		}
		break;

	CASE_WIDTH_2BYTE:
		ptr.cp16 = DeeString_Get2Byte((DeeObject *)self);
		result   = WSTR_LENGTH(ptr.cp16);
		if unlikely(dst_length < result)
			break;
		end.cp16 = ptr.cp16 + result;
		for (; ptr.cp16 < end.cp16; ++ptr.cp16) {
			DREF DeeObject *chr;
			chr = DeeString_Chr(*ptr.cp16);
			if unlikely(!chr)
				goto err_dst_iter;
			*dst_iter++ = chr;
		}
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = DeeString_Get4Byte((DeeObject *)self);
		result   = WSTR_LENGTH(ptr.cp32);
		if unlikely(dst_length < result)
			break;
		end.cp32 = ptr.cp32 + result;
		for (; ptr.cp32 < end.cp32; ++ptr.cp32) {
			DREF DeeObject *chr;
			chr = DeeString_Chr(*ptr.cp32);
			if unlikely(!chr)
				goto err_dst_iter;
			*dst_iter++ = chr;
		}
		break;
	}
	return result;
err_dst_iter:
	Dee_Decrefv(dst, (size_t)(dst_iter - dst));
	return (size_t)-1;
}


#ifdef __OPTIMIZE_SIZE__
#define string_getitem  default__getitem__with__getitem_index
#define string_getrange default__getrange__with__getrange_index__and__getrange_index_n
#else /* __OPTIMIZE_SIZE__ */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_getitem(String *self, DeeObject *index) {
	int width = DeeString_WIDTH(self);
	union dcharptr str;
	size_t i, len;
	str.ptr = DeeString_WSTR(self);
	len     = WSTR_LENGTH(str.ptr);
	if (DeeObject_AsSize(index, &i))
		goto err;
	if unlikely(i >= len)
		goto err_oob;
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		return DeeString_Chr(str.cp8[i]);

	CASE_WIDTH_2BYTE:
		return DeeString_Chr(str.cp16[i]);

	CASE_WIDTH_4BYTE:
		return DeeString_Chr(str.cp32[i]);

	}
err_oob:
	err_index_out_of_bounds((DeeObject *)self, i, len);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
string_getrange(String *self, DeeObject *begin, DeeObject *end) {
	Dee_ssize_t i_begin, i_end;
	if (DeeObject_AsSSize(begin, &i_begin))
		goto err;
	if (DeeNone_Check(end))
		return string_getrange_index_n(self, i_begin);
	if (DeeObject_AsSSize(end, &i_end))
		goto err;
	return string_getrange_index(self, i_begin, i_end);
err:
	return NULL;
}
#endif /* !__OPTIMIZE_SIZE__ */


PRIVATE struct type_seq string_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&string_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&string_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&string_getitem,
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&string_getrange,
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&string_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem                    = */ &default__hasitem__with__hasitem_index,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&string_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&string_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&string_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index              = */ &default__seq_operator_hasitem_index__with__seq_operator_size,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&string_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&string_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ &default__trygetitem__with__trygetitem_index,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&string_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&string_asvector,
	/* .tp_asvector_nothrow           = */ NULL,
};

PRIVATE struct type_member tpconst string_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &StringIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_class_chr(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	uint32_t ch;
	if (DeeArg_Unpack(argc, argv, UNPu32 ":chr", &ch))
		goto err;
	return DeeString_Chr(ch);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_class_fromseq(DeeObject *UNUSED(self),
                     size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:fromseq", &seq))
		goto err;
	/* XXX: Fast-sequence optimizations? */
	{
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		DREF DeeObject *iter, *elem;
		iter = DeeObject_Iter(seq);
		if unlikely(!iter)
			goto err_printer;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			uint32_t chr;
			int temp;
			temp = DeeObject_AsUInt32(elem, &chr);
			Dee_Decref(elem);
			if unlikely(temp)
				goto err_iter;
			if unlikely(unicode_printer_putc(&printer, chr))
				goto err_iter;
			if (DeeThread_CheckInterrupt())
				goto err_iter;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
		return unicode_printer_pack(&printer);
err_iter:
		Dee_Decref(iter);
err_printer:
		unicode_printer_fini(&printer);
	}
err:
	return NULL;
}

PRIVATE struct type_method tpconst string_class_methods[] = {
	TYPE_METHOD_F("chr", &string_class_chr,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(ch:?Dint)->?.\n"
	              "#tIntegerOverflow{@ch is negative or greater than the greatest unicode-character}"
	              "#r{A single-character string matching the unicode-character @ch}"),
	TYPE_METHOD_F("fromseq", &string_class_fromseq,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(ordinals:?S?Dint)->?.\n"
	              "#tIntegerOverflow{One of the ordinals is negative, or greater than $0xffffffff}"
	              "Construct a new string object from a sequence of ordinal values"),
	TYPE_METHOD_END
};

INTDEF struct type_method tpconst string_methods[];
INTDEF struct type_math string_math;

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_Ordinals(DeeObject *__restrict self);

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_hashed(String *__restrict self) {
	return_bool(self->s_hash != DEE_STRING_HASH_UNSET);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_hasutf(String *__restrict self) {
	return_bool(atomic_read(&self->s_data) != NULL);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_hasregex(String *__restrict self) {
	struct string_utf *utf;
	utf = atomic_read(&self->s_data);
	if (utf == NULL)
		return_false;
	return_bool((atomic_read(&utf->u_flags) & STRING_UTF_FREGEX) != 0);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_getfirst(String *__restrict self) {
	void *str = DeeString_WSTR(self);
	int width = DeeString_WIDTH(self);
	if unlikely(!WSTR_LENGTH(str))
		goto err_empty;
	return DeeString_Chr(STRING_WIDTH_GETCHAR(width, str, 0));
err_empty:
	err_unbound_attribute_string(&DeeString_Type, STR_first);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_getlast(String *__restrict self) {
	void *str     = DeeString_WSTR(self);
	int width     = DeeString_WIDTH(self);
	size_t length = WSTR_LENGTH(str);
	if unlikely(!length)
		goto err_empty;
	return DeeString_Chr(STRING_WIDTH_GETCHAR(width, str, length - 1));
err_empty:
	err_unbound_attribute_string(&DeeString_Type, STR_last);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_trygetfirst(String *__restrict self) {
	void *str = DeeString_WSTR(self);
	int width = DeeString_WIDTH(self);
	if unlikely(!WSTR_LENGTH(str))
		goto err_empty;
	return DeeString_Chr(STRING_WIDTH_GETCHAR(width, str, 0));
err_empty:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_trygetlast(String *__restrict self) {
	void *str     = DeeString_WSTR(self);
	int width     = DeeString_WIDTH(self);
	size_t length = WSTR_LENGTH(str);
	if unlikely(!length)
		goto err_empty;
	return DeeString_Chr(STRING_WIDTH_GETCHAR(width, str, length - 1));
err_empty:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_sizeof(String *__restrict self) {
	size_t result;
	struct string_utf *utf;
	result = offsetof(String, s_str);
	result += (self->s_len + 1) * sizeof(char);
	utf = self->s_data;
	if (utf) {
		unsigned int i;
		result += sizeof(struct string_utf);
		for (i = 1; i < STRING_WIDTH_COUNT; ++i) {
			if (!utf->u_data[i])
				continue;
			result += STRING_MUL_SIZEOF_WIDTH(WSTR_LENGTH(utf->u_data[i]), i);
		}
		if (utf->u_data[STRING_WIDTH_1BYTE] &&
		    utf->u_data[STRING_WIDTH_1BYTE] != (size_t *)DeeString_STR(self))
			result += WSTR_LENGTH(utf->u_data[STRING_WIDTH_1BYTE]) * 1;
		if (utf->u_utf8 && utf->u_utf8 != (char *)DeeString_STR(self) &&
		    utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE])
			result += WSTR_LENGTH(utf->u_utf8) * 1;
		if (utf->u_utf16 && (uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
			result += WSTR_LENGTH(utf->u_utf16) * 2;
	}
	return DeeInt_NewSize(result);
}


/* Expose auditing internals for `deemon.string' */
#undef CONFIG_HAVE_STRING_AUDITING_INTERNALS
#if 1
#define CONFIG_HAVE_STRING_AUDITING_INTERNALS
#endif

#ifdef CONFIG_HAVE_STRING_AUDITING_INTERNALS
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_str_bytes(String *__restrict self) {
	return DeeBytes_NewView((DeeObject *)self,
	                        DeeString_STR(self),
	                        DeeString_SIZE(self) * sizeof(char),
	                        Dee_BUFFER_FREADONLY);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_str_bytes_isutf8(String *__restrict self) {
	return_bool(DeeString_STR_ISUTF8(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_str_bytes_islatin1(String *__restrict self) {
	return_bool(DeeString_STR_ISLATIN1(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_str_width(String *__restrict self) {
	int width   = DeeString_WIDTH(self);
	size_t size = STRING_SIZEOF_WIDTH(width);
	return DeeInt_NewSize(size);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_wstr_bytes(String *__restrict self) {
	return DeeBytes_NewView((DeeObject *)self,
	                        DeeString_WSTR(self),
	                        DeeString_WSIZ(self),
	                        Dee_BUFFER_FREADONLY);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_utf8_bytes(String *__restrict self) {
	char *utf8 = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!utf8)
		goto err;
	return DeeBytes_NewView((DeeObject *)self,
	                        utf8, WSTR_LENGTH(utf8),
	                        Dee_BUFFER_FREADONLY);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_utf16_bytes(String *__restrict self) {
	uint16_t *utf16 = DeeString_AsUtf16((DeeObject *)self, STRING_ERROR_FSTRICT);
	if unlikely(!utf16)
		goto err;
	return DeeBytes_NewView((DeeObject *)self,
	                        utf16, WSTR_LENGTH(utf16) << 1,
	                        Dee_BUFFER_FREADONLY);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_utf32_bytes(String *__restrict self) {
	uint32_t *utf32 = DeeString_AsUtf32((DeeObject *)self);
	if unlikely(!utf32)
		goto err;
	return DeeBytes_NewView((DeeObject *)self,
	                        utf32, WSTR_LENGTH(utf32) << 2,
	                        Dee_BUFFER_FREADONLY);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_1byte_bytes(String *__restrict self) {
	uint8_t *b1;
	if (self->s_data && self->s_data->u_width != Dee_STRING_WIDTH_1BYTE)
		goto err_too_large;
	b1 = DeeString_As1Byte((DeeObject *)self);
	return DeeBytes_NewView((DeeObject *)self,
	                        b1, WSTR_LENGTH(b1),
	                        Dee_BUFFER_FREADONLY);
err_too_large:
	DeeError_Throwf(&DeeError_UnicodeEncodeError,
	                "String %r contains ordinals greater than U+00FF",
	                self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_audit_2byte_bytes(String *__restrict self) {
	uint16_t *b2;
	if (self->s_data && self->s_data->u_width > Dee_STRING_WIDTH_2BYTE)
		goto err_too_large;
	b2 = DeeString_As2Byte((DeeObject *)self);
	return DeeBytes_NewView((DeeObject *)self,
	                        b2, WSTR_LENGTH(b2) << 1,
	                        Dee_BUFFER_FREADONLY);
err_too_large:
	DeeError_Throwf(&DeeError_UnicodeEncodeError,
	                "String %r contains ordinals greater than U+FFFF",
	                self);
	return NULL;
}

#define string_audit_4byte_bytes string_audit_utf32_bytes
#endif /* CONFIG_HAVE_STRING_AUDITING_INTERNALS */

INTERN WUNUSED NONNULL((1)) DREF String *DCALL
string_getsubstr(String *__restrict self,
                 size_t start, size_t end);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_mh_seq_sum(String *__restrict self) {
	/* Must handle special case: `({} + ...)' is "none", so
	 * if the string is empty, we must return "none" here! */
	if (DeeString_IsEmpty(self))
		return_none;
	return_reference_((DeeObject *)self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_mh_seq_sum_with_range(String *__restrict self,
                             size_t start, size_t end) {
	if (end > DeeString_WLEN(self))
		end = DeeString_WLEN(self);
	if (start >= end)
		return_none;
	return (DREF DeeObject *)string_getsubstr(self, start, end);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
string_mh_seq_any_with_range(String *self, size_t start, size_t end) {
	size_t length;
	if (start <= end)
		return 0;
	if (start == 0)
		return !DeeString_IsEmpty(self);
	length = DeeString_WLEN(self);
	return start < length;
}


PRIVATE struct type_method_hint tpconst string_method_hints[] = {
	/* Helper hints. */
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &string_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &string_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &string_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &string_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetlast, &string_trygetlast, METHOD_FNOREFESCAPE),

	/* This stuff here is needed so that `("foo" as Sequence) <=> ["f", "o", "o"]' works. */
	TYPE_METHOD_HINT_F(seq_operator_compare_eq, &string_mh_seq_compare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare, &string_mh_seq_compare, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trycompare_eq, &string_mh_seq_trycompare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_eq, &default__seq_operator_eq__with__seq_operator_compare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_ne, &default__seq_operator_ne__with__seq_operator_compare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_lo, &default__seq_operator_lo__with__seq_operator_compare, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_le, &default__seq_operator_le__with__seq_operator_compare, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_gr, &default__seq_operator_gr__with__seq_operator_compare, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_ge, &default__seq_operator_ge__with__seq_operator_compare, METHOD_FNOREFESCAPE),

	/* Optimized  */
	TYPE_METHOD_HINT(seq_sum, &string_mh_seq_sum),
	TYPE_METHOD_HINT(seq_sum_with_range, &string_mh_seq_sum_with_range),

	/* seq.all() is true if no sequence element evaluations to false.
	 * Since the elements of strings are all 1-char strings, there can
	 * never be an empty string (meaning all elements are always true) */
	TYPE_METHOD_HINT_F(seq_all, (int (DCALL *)(DeeObject *))&_DeeNone_reti1_1, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_key, &default__seq_all_with_key__with__seq_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range, (int (DCALL *)(DeeObject *, size_t, size_t))&_DeeNone_reti1_3, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range_and_key, &default__seq_all_with_range_and_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),

	/* seq.any() is true if there is at least 1 true element in the
	 * sequence. Since all elements of strings are 1-char strings,
	 * which are always true, seq.any() is true if the string is
	 * non-empty. */
	TYPE_METHOD_HINT_F(seq_any, &string_bool, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_key, &default__seq_any_with_key__with__seq_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range, &string_mh_seq_any_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range_and_key, &default__seq_any_with_range_and_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),

	/* These are here because string defines its own "find", "rfind", etc.
	 * functions that work differently from those defined by Sequence. Since
	 * we still want "(string as Sequence).find" to behave like the version
	 * from "Sequence", we have to re-inject the original behavior here, as
	 * well as link it as explicit method references in "string_functions.c" */
	TYPE_METHOD_HINT_F(seq_count, &default__seq_count__with__seq_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_key, &default__seq_count_with_key__with__seq_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range, &default__seq_count_with_range__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range_and_key, &default__seq_count_with_range_and_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains, &default__seq_contains__with__seq_operator_contains, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_key, &default__seq_contains_with_key__with__seq_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range, &default__seq_contains_with_range__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &default__seq_contains_with_range_and_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith, &default__seq_startswith__with__seq_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_key, &default__seq_startswith_with_key__with__seq_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_range, &default__seq_startswith_with_range__with__seq_operator_trygetitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_startswith_with_range_and_key, &default__seq_startswith_with_range_and_key__with__seq_operator_trygetitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith, &default__seq_endswith__with__seq_trygetlast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_key, &default__seq_endswith_with_key__with__seq_trygetlast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_range, &default__seq_endswith_with_range__with__seq_operator_size__and__operator_trygetitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_endswith_with_range_and_key, &default__seq_endswith_with_range_and_key__with__seq_operator_size__and__operator_trygetitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find, &default__seq_find__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &default__seq_find_with_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind, &default__seq_rfind__with__seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind_with_key, &default__seq_rfind_with_key__with__seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),

	TYPE_METHOD_HINT_END
};

PRIVATE struct type_getset tpconst string_getsets[] = {
	TYPE_GETTER_F("ordinals", &DeeString_Ordinals, METHOD_FCONSTCALL,
	              "->?S?Dint\n"
	              "Returns a proxy view for the characters of @this ?. as a sequence of "
	              /**/ "integers referring to the ordinal values of each character (s.a. ?#ord)"),
	TYPE_GETTER_F("__hashed__", &string_hashed,
	              METHOD_FNOREFESCAPE, /* Not CONSTCALL, because can change to true */
	              "->?Dbool\n"
	              "Evaluates to ?t if @this ?. has been hashed"),
	TYPE_GETTER_F("__hasutf__", &string_hasutf,
	              METHOD_FNOREFESCAPE, /* Not CONSTCALL, because can change to true */
	              "->?Dbool\n"
	              "Evaluates to ?t if @this ?. owns a UTF container"),
	TYPE_GETTER_F("__hasregex__", &string_hasregex,
	              METHOD_FNOREFESCAPE, /* Not CONSTCALL, because can change to true */
	              "->?Dbool\n"
	              "Evaluates to ?t if @this ?. has been compiled as a regex pattern in the past"),
	TYPE_GETTER_BOUND_F(STR_first, &string_getfirst, &string_bool_asbound,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?.\n"
	                    "#tUnboundAttribute{@this ?. is empty}"
	                    "Returns the first character of @this ?."),
	TYPE_GETTER_BOUND_F(STR_last, &string_getlast, &string_bool_asbound,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?.\n"
	                    "#tUnboundAttribute{@this ?. is empty}"
	                    "Returns the last character of @this ?."),
	TYPE_GETTER_F("__sizeof__", &string_sizeof,
	              METHOD_FNOREFESCAPE, /* Not CONSTCALL, because can change when UTF data is allocated */
	              "->?Dint"),

#ifdef CONFIG_HAVE_STRING_AUDITING_INTERNALS
	TYPE_GETTER_F("__str_bytes__", &string_audit_str_bytes, METHOD_FCONSTCALL,
	              "->?DBytes\n"
	              "Internal function to view the bytes of ${DeeString_STR()}"),
	TYPE_GETTER_F("__str_bytes_isutf8__", &string_audit_str_bytes_isutf8, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Value of ${DeeString_STR_ISUTF8()}"),
	TYPE_GETTER_F("__str_bytes_islatin1__", &string_audit_str_bytes_islatin1, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Value of ${DeeString_STR_ISLATIN1()}"),
	TYPE_GETTER_F("__str_width__", &string_audit_str_width, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Returns $1, $2 or $4 based on ${DeeString_WIDTH()}"),
	TYPE_GETTER_F("__wstr_bytes__", &string_audit_wstr_bytes, METHOD_FCONSTCALL,
	              "->?DBytes\n"
	              "Internal function to view the bytes of ${DeeString_WSTR()}"),
	TYPE_GETTER_F("__utf8_bytes__", &string_audit_utf8_bytes, METHOD_FCONSTCALL,
	              "->?DBytes\n"
	              "Internal function to view the bytes of ${DeeString_AsUtf8()}"),
	TYPE_GETTER_F("__utf16_bytes__", &string_audit_utf16_bytes, METHOD_FCONSTCALL,
	              "->?DBytes\n"
	              "#tUnicodeEncodeError{@this ?Dstring contains ordinals that can't be encoded as utf-16}"
	              "Internal function to view the bytes of ${DeeString_AsUtf16()}"),
	TYPE_GETTER_F("__utf32_bytes__", &string_audit_utf32_bytes, METHOD_FCONSTCALL,
	              "->?DBytes\n"
	              "Internal function to view the bytes of ${DeeString_AsUtf32()}"),
	TYPE_GETTER_F("__1byte_bytes__", &string_audit_1byte_bytes, METHOD_FCONSTCALL,
	              "->?DBytes\n"
	              "#tUnicodeEncodeError{@this ?Dstring contains ordinals greater than $0xff}"
	              "Internal function to view the bytes of ${DeeString_As1Byte()}"),
	TYPE_GETTER_F("__2byte_bytes__", &string_audit_2byte_bytes, METHOD_FCONSTCALL,
	              "->?DBytes\n"
	              "#tUnicodeEncodeError{@this ?Dstring contains ordinals greater than $0xffff}"
	              "Internal function to view the bytes of ${DeeString_As2Byte()}"),
	TYPE_GETTER_F("__4byte_bytes__", &string_audit_4byte_bytes, METHOD_FCONSTCALL,
	              "->?DBytes\n"
	              "Internal function to view the bytes of ${DeeString_As4Byte()}"),
#endif /* CONFIG_HAVE_STRING_AUDITING_INTERNALS */
	TYPE_GETSET_END
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
string_getbuf(String *__restrict self,
              DeeBuffer *__restrict info,
              unsigned int flags) {
	(void)flags;
	info->bb_base = DeeString_AsBytes((DeeObject *)self, false);
	if unlikely(!info->bb_base)
		goto err;
	info->bb_size = WSTR_LENGTH(info->bb_base);
	return 0;
err:
	return -1;
}

PRIVATE struct type_buffer string_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&string_getbuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FREADONLY
};

PRIVATE struct type_operator const string_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES),
	TYPE_OPERATOR_FLAGS(OPERATOR_0012_MUL, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_8003_GETBUF, METHOD_FCONSTCALL),
};

/* `string from deemon' */
PUBLIC DeeTypeObject DeeString_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_string),
	/* .tp_doc      = */ DOC("An encoding-neutral, immutable sequence of characters\n"
	                         "\n"

	                         "()\n"
	                         "Returns an empty string $\"\"\n"
	                         "\n"

	                         "(ob)\n"
	                         "Same as ${str ob}, returning the string representation of @ob\n"
	                         "\n"

	                         "str->\n"
	                         "Simply re-return @this ?.\n"
	                         "\n"

	                         "repr->\n"
	                         "Returns @this ?. as a C-style escaped string\n"
	                         "${"
	                         /**/ "operator repr() {\n"
	                         /**/ "	return \"\\\"{}\\\"\".format({ this.encode(\"c-escape\") });\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this ?. is non-empty\n"
	                         "${"
	                         /**/ "operator bool() {\n"
	                         /**/ "	return ##this != 0;\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "+(other:?X3?.?DBytes?O)->\n"
	                         "Return a new string that is the concatenation of @this and ${str other}\n"
	                         "${"
	                         /**/ "operator + (other) {\n"
	                         /**/ "	return \"{}{}\".format({ this, other });\n"
	                         /**/ "}"
	                         "}\n"
	                         "\n"

	                         "*(times:?Dint)->\n"
	                         "#tIntegerOverflow{@times is negative, or too large}"
	                         "Returns @this ?. repeated @times number of times\n"
	                         "\n"

	                         "%(args:?DTuple)->\n"
	                         "%(arg:?O)->\n"
	                         "Using @this ?. as a printf-style format string, use a tuple found "
	                         /**/ "in @args to format it into a new string, which is then returned\n"
	                         "If @arg isn't a tuple, it is packed into one and the call is identical "
	                         /**/ "to ${this.operator % (pack(arg))}\n"
	                         "${"
	                         /**/ "local x = 42;\n"
	                         /**/ "print \"x = %d\" % x; /* \"x = 42\" */"
	                         "}\n"
	                         "\n"

	                         "<(other:?X2?.?DBytes)->\n"
	                         "<=(other:?X2?.?DBytes)->\n"
	                         "==(other:?X2?.?DBytes)->\n"
	                         "!=(other:?X2?.?DBytes)->\n"
	                         ">(other:?X2?.?DBytes)->\n"
	                         ">=(other:?X2?.?DBytes)->\n"
	                         "Perform a lexicographical comparison between @this ?. "
	                         /**/ "and @other, and return the result\n"
	                         "\n"

	                         "iter->\n"
	                         "Return a string iterator that can be used to enumerate each of "
	                         /**/ "the string's characters individually\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the length of @this ?. in characters\n"
	                         "\n"

	                         "contains(substr:?X2?.?DBytes)->\n"
	                         "Returns ?t if @substr is apart of @this ?.\n"
	                         "${"
	                         /**/ "print \"foo\" in \"bar\";    /* false */\n"
	                         /**/ "print \"foo\" in \"foobar\"; /* true */"
	                         "}\n"
	                         "\n"

	                         "[]->?.\n"
	                         "#tIntegerOverflow{@index is negative}"
	                         "#tIndexError{@index is greater than ${##this}}"
	                         "Returns the @{index}th character of @this ?.\n"
	                         "${"
	                         /**/ "print \"foo\"[0]; /* \"f\" */\n"
	                         /**/ "print \"foo\"[1]; /* \"o\" */"
	                         "}\n"
	                         "\n"

	                         "[:]->?.\n"
	                         "Return a sub-string of @this, that starts at @start and ends at @end\n"
	                         "If @end is greater than ${##this}, it is truncated to that value\n"
	                         "If @start is greater than, or equal to @end, an empty string is returned\n"
	                         "If either @start or @end is negative, ${##this} is added before "
	                         /**/ "further index transformations are performed\n"
	                         "As per convention, ?N may be passed for @end as an alias for ${##this}\n"
	                         "${"
	                         /**/ "print \"foo\"[:-1];      /* \"fo\" */\n"
	                         /**/ "print \"bar\"[1:];       /* \"ar\" */\n"
	                         /**/ "print \"foobar\"[3:123]; /* \"bar\" */\n"
	                         /**/ "print \"bizbuz\"[5:4];   /* \"\" */"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FNAMEOBJECT | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&string_new_empty,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)&string_new
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&string_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ &DeeObject_NewRef,
		/* .tp_repr      = */ &string_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&string_bool,
		/* .tp_print     = */ &DeeString_PrintUtf8,
		/* .tp_printrepr = */ &DeeString_PrintRepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &string_math,
	/* .tp_cmp           = */ &string_cmp,
	/* .tp_seq           = */ &string_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ &string_buffer,
	/* .tp_methods       = */ string_methods,
	/* .tp_getsets       = */ string_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ string_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ string_class_members,
	/* .tp_method_hints  = */ string_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__5B2E0F4105586532),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ string_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(string_operators),
};

PRIVATE struct {
	size_t len;
	uint32_t zero;
} empty_utf = { 0, 0 };

PRIVATE struct string_utf empty_string_utf = {
	/* .u_width = */ STRING_WIDTH_1BYTE, /* Every character fits into a single byte (because there are no characters) */
	/* .u_flags = */ STRING_UTF_FASCII,
	/* .u_data  = */ {
		/* [STRING_WIDTH_1BYTE] = */ (size_t *)&DeeString_Empty.s_zero,
		/* [STRING_WIDTH_2BYTE] = */ (size_t *)&empty_utf.zero,
		/* [STRING_WIDTH_4BYTE] = */ (size_t *)&empty_utf.zero
	},
	/* .u_utf8  = */ &DeeString_Empty.s_zero
};

PUBLIC struct Dee_empty_string_struct DeeString_Empty = {
	OBJECT_HEAD_INIT(&DeeString_Type),
	/* .s_data = */ &empty_string_utf,
	/* .s_hash = */ /*[[[deemon print (_Dee_HashSelect from rt.gen.hash)("");]]]*/_Dee_HashSelectC(0x0, 0x0)/*[[[end]]]*/,
	/* .s_len  = */ 0,
	/* .s_zero = */ '\0'
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_STRING_C */
