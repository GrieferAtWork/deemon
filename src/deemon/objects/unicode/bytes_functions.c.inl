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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL
#define GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL 1

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include "string_functions.h"

DECL_BEGIN

typedef DeeBytesObject Bytes;

typedef struct {
	uint8_t *n_data;
	size_t   n_size;
	uint8_t _n_buf[sizeof(size_t)];
} Needle;

#ifndef __NO_builtin_expect
#define get_needle(self, ob) __builtin_expect(get_needle(self, ob), 0)
#endif /* !__NO_builtin_expect */

PRIVATE int (DCALL get_needle)(Needle *__restrict self, DeeObject *__restrict ob) {
	if (DeeString_Check(ob)) {
		self->n_data = DeeString_AsBytes(ob, false);
		if unlikely(!self->n_data)
			goto err;
		self->n_size = WSTR_LENGTH(self->n_data) * sizeof(char);
	} else if (DeeBytes_Check(ob)) {
		self->n_data = DeeBytes_DATA(ob);
		self->n_size = DeeBytes_SIZE(ob);
	} else {
		/* Convert to an integer (to-be used as a single byte). */
		if (DeeObject_AsUInt8(ob, self->_n_buf))
			goto err;
		self->n_data = self->_n_buf;
		self->n_size = 1;
	}
	return 0;
err:
	return -1;
}

INTERN DREF DeeObject *DCALL
bytes_contains(Bytes *__restrict self,
               DeeObject *__restrict find_ob) {
	Needle needle;
	if (get_needle(&needle, find_ob))
		return NULL;
	return_bool(memmemb(DeeBytes_DATA(self),
	                    DeeBytes_SIZE(self),
	                    needle.n_data,
	                    needle.n_size) != NULL);
}


#ifndef PRIVATE_FIND_KWLIST_DEFINED
#define PRIVATE_FIND_KWLIST_DEFINED 1
PRIVATE struct keyword find_kwlist[] = { K(needle), K(start), K(end), KEND };
#endif /* !PRIVATE_FIND_KWLIST_DEFINED */

PRIVATE DREF DeeObject *DCALL
bytes_find(Bytes *__restrict self, size_t argc,
           DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:find", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		result = NULL;
	else {
		result = (uint8_t *)memmemb(DeeBytes_DATA(self) + start,
		                            end - start,
		                            needle.n_data,
		                            needle.n_size);
	}
	if (!result)
		return_reference_(&DeeInt_MinusOne);
	return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_casefind(Bytes *__restrict self, size_t argc,
               DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casefind", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		ptr = NULL;
	else {
		ptr = (uint8_t *)dee_memasciicasemem(DeeBytes_DATA(self) + start,
		                                     end - start,
		                                     needle.n_data,
		                                     needle.n_size);
	}
	if (!ptr)
		goto not_found;
	result = (size_t)(ptr - DeeBytes_DATA(self));
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + needle.n_size);
not_found:
	return_none;
}

PRIVATE DREF DeeObject *DCALL
bytes_rfind(Bytes *__restrict self, size_t argc,
            DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:rfind", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		result = NULL;
	else {
		result = (uint8_t *)memrmemb(DeeBytes_DATA(self) + start,
		                             end - start,
		                             needle.n_data,
		                             needle.n_size);
	}
	if (!result)
		return_reference_(&DeeInt_MinusOne);
	return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_caserfind(Bytes *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caserfind", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		ptr = NULL;
	else {
		ptr = (uint8_t *)dee_memasciicasermem(DeeBytes_DATA(self) + start,
		                                      end - start,
		                                      needle.n_data,
		                                      needle.n_size);
	}
	if (!ptr)
		goto not_found;
	result = (size_t)(ptr - DeeBytes_DATA(self));
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + needle.n_size);
not_found:
	return_none;
}

PRIVATE DREF DeeObject *DCALL
bytes_index(Bytes *__restrict self, size_t argc,
            DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:index", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		result = NULL;
	else {
		result = (uint8_t *)memmemb(DeeBytes_DATA(self) + start,
		                            end - start,
		                            needle.n_data,
		                            needle.n_size);
	}
	if (!result) {
		err_index_not_found((DeeObject *)self, find_ob);
		return NULL;
	}
	return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_caseindex(Bytes *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caseindex", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		ptr = NULL;
	else {
		ptr = (uint8_t *)dee_memasciicasemem(DeeBytes_DATA(self) + start,
		                                     end - start,
		                                     needle.n_data,
		                                     needle.n_size);
	}
	if (!ptr) {
		err_index_not_found((DeeObject *)self, find_ob);
		return NULL;
	}
	result = (size_t)(ptr - DeeBytes_DATA(self));
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + needle.n_size);
}

PRIVATE DREF DeeObject *DCALL
bytes_rindex(Bytes *__restrict self, size_t argc,
             DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:rindex", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		result = NULL;
	else {
		result = (uint8_t *)memrmemb(DeeBytes_DATA(self) + start,
		                             end - start,
		                             needle.n_data,
		                             needle.n_size);
	}
	if (!result) {
		err_index_not_found((DeeObject *)self, find_ob);
		return NULL;
	}
	return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
}

PRIVATE DREF DeeObject *DCALL
bytes_caserindex(Bytes *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caserindex", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		ptr = NULL;
	else {
		ptr = (uint8_t *)dee_memasciicasermem(DeeBytes_DATA(self) + start,
		                                      end - start,
		                                      needle.n_data,
		                                      needle.n_size);
	}
	if (!ptr) {
		err_index_not_found((DeeObject *)self, find_ob);
		return NULL;
	}
	result = (size_t)(ptr - DeeBytes_DATA(self));
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + needle.n_size);
}

PRIVATE DREF DeeObject *DCALL
bytes_count(Bytes *__restrict self, size_t argc,
            DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t result;
	size_t start = 0, end = (size_t)-1;
	uint8_t *iter;
	size_t size;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:count", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	iter = DeeBytes_DATA(self);
	size = DeeBytes_SIZE(self);
	if (end > size)
		end = size;
	result = 0;
	if (start < end) {
		end -= start;
		iter += start;
		while (end >= needle.n_size) {
			if (MEMEQB(iter, needle.n_data, needle.n_size))
				++result;
			--end;
			++iter;
		}
	}
	return DeeInt_NewSize(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_casecount(Bytes *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t result;
	size_t start = 0, end = (size_t)-1;
	uint8_t *iter;
	size_t size;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casecount", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	iter = DeeBytes_DATA(self);
	size = DeeBytes_SIZE(self);
	if (end > size)
		end = size;
	result = 0;
	if (start < end) {
		end -= start;
		iter += start;
		while (end >= needle.n_size) {
			if (dee_memasciicaseeq(iter, needle.n_data, needle.n_size))
				++result;
			--end;
			++iter;
		}
	}
	return DeeInt_NewSize(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_contains_f(Bytes *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:contains", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_false;
	return_bool(memmemb(DeeBytes_DATA(self) + start,
	                    end - start,
	                    needle.n_data,
	                    needle.n_size) != NULL);
}

PRIVATE DREF DeeObject *DCALL
bytes_casecontains_f(Bytes *__restrict self, size_t argc,
                     DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casecontains", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_false;
	return_bool(dee_memasciicasemem(DeeBytes_DATA(self) + start,
	                                end - start,
	                                needle.n_data,
	                                needle.n_size) != NULL);
}

INTDEF dssize_t DCALL
DeeBytes_Format(dformatprinter printer,
                dformatprinter format_printer, void *arg,
                char const *__restrict format,
                size_t format_len, DeeObject *__restrict args);

PRIVATE DREF DeeObject *DCALL
bytes_format(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	DeeObject *args;
	if (DeeArg_Unpack(argc, argv, "o:format", &args))
		goto err;
	{
		struct bytes_printer printer = BYTES_PRINTER_INIT;
		if unlikely(DeeBytes_Format(&bytes_printer_print,
			                         (dformatprinter)&bytes_printer_append,
			                         &printer,
			                         (char *)DeeBytes_DATA(self),
			                         DeeBytes_SIZE(self),
			                         args) < 0)
		goto err_printer;
		return bytes_printer_pack(&printer);
err_printer:
		bytes_printer_fini(&printer);
	}
err:
	return NULL;
}



PRIVATE DREF Bytes *DCALL
bytes_getsubstr(Bytes *__restrict self,
                size_t start, size_t end) {
	if (end >= DeeBytes_SIZE(self)) {
		if (start == 0)
			return_reference_(self);
		end = DeeBytes_SIZE(self);
	}
	if (start >= end)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	return (DREF Bytes *)DeeBytes_NewSubView(self,
	                                         self->b_base + (size_t)start,
	                                         (size_t)(end - start));
}

#ifndef PRIVATE_SUBSTR_KWLIST_DEFINED
#define PRIVATE_SUBSTR_KWLIST_DEFINED 1
PRIVATE struct keyword substr_kwlist[] = { K(start), K(end), KEND };
#endif /* !PRIVATE_SUBSTR_KWLIST_DEFINED */

PRIVATE DREF Bytes *DCALL
bytes_substr(Bytes *__restrict self, size_t argc,
             DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:substr", &start, &end))
		goto err;
	return bytes_getsubstr(self, start, end);
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_resized(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	DREF Bytes *result;
	size_t new_size;
	if (argc == 1) {
		if (DeeObject_AsSize(argv[0], &new_size))
			goto err;
		result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(new_size);
		if unlikely(!result)
			goto err;
		memcpy(result->b_data,
		       DeeBytes_DATA(self),
		       MIN(DeeBytes_SIZE(self), new_size));
	} else {
		uint8_t init;
		if (DeeArg_Unpack(argc, argv, "Iu|I8u:resized", &new_size, &init))
			goto err;
		result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(new_size);
		if unlikely(!result)
			goto err;
		if (new_size <= DeeBytes_SIZE(self)) {
			memcpy(result->b_data, DeeBytes_DATA(self), new_size);
		} else {
			size_t old_size = DeeBytes_SIZE(self);
			memcpy(result->b_data, DeeBytes_DATA(self), old_size);
			memset(result->b_data + old_size, init, new_size - old_size);
		}
	}
	return result;
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_reversed(Bytes *__restrict self, size_t argc,
               DeeObject **__restrict argv, DeeObject *kw) {
	DREF Bytes *result;
	uint8_t *data, *dst;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:reversed", &start, &end))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (end <= start)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	end -= start;
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
	if unlikely(!result)
		goto done;
	data = DeeBytes_DATA(self);
	dst  = DeeBytes_DATA(result);
	do {
		--end;
		*dst++ = ((uint8_t *)data)[end];
	} while (end);
done:
	return result;
}

PRIVATE DREF Bytes *DCALL
bytes_reverse(Bytes *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	uint8_t *data, *dst;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:reverse", &start, &end))
		goto err;
	if unlikely(!DeeBytes_WRITABLE(self)) {
		err_bytes_not_writable((DeeObject *)self);
		goto err;
	}
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (end <= start)
		goto done;
	data = DeeBytes_DATA(self) + start;
	dst  = DeeBytes_DATA(self) + end;
	while (data < dst) {
		uint8_t temp = *data;
		*data++      = *--dst;
		*dst         = temp;
	}
done:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_makereadonly(Bytes *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":makereadonly"))
		goto err;
	if (!DeeBytes_WRITABLE(self))
		return_reference_(self);
	return (DREF Bytes *)DeeBytes_NewSubViewRo(self,
	                                           DeeBytes_DATA(self),
	                                           DeeBytes_SIZE(self));
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_makewritable(Bytes *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":makewritable"))
		goto err;
	if (DeeBytes_WRITABLE(self))
		return_reference_(self);
	/* Return a copy of `self' */
	return (DREF Bytes *)DeeBytes_NewBufferData(DeeBytes_DATA(self),
	                                            DeeBytes_SIZE(self));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_hex(Bytes *__restrict self, size_t argc,
          DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1, i;
	char *dst;
	DREF DeeStringObject *result;
	uint8_t *data;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:hex", &start, &end))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_empty_string;
	end -= start;
	data = DeeBytes_DATA(self);
	data += start;
	result = (DREF DeeStringObject *)DeeString_NewBuffer(end * 2);
	if unlikely(!result)
		goto err;
	dst = DeeString_STR(result);
	for (i = 0; i < end; ++i) {
		uint8_t byte = data[i];
		uint8_t nibble;
#ifndef CONFIG_NO_THREADS
		COMPILER_READ_BARRIER();
#endif /* !CONFIG_NO_THREADS */
		nibble = byte >> 4;
		*dst++ = nibble >= 10 ? (char)('a' + (nibble - 10)) : (char)('0' + nibble);
		byte &= 0xf;
		*dst++ = nibble >= 10 ? (char)('a' + (byte - 10)) : (char)('0' + byte);
	}
	ASSERT(dst == DeeString_END(result));
	return (DREF DeeObject *)result;
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
bytes_ord(Bytes *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	size_t index = 0;
	if (argc) {
		if (DeeArg_Unpack(argc, argv, "Iu:ord", &index))
			goto err;
		if (index >= DeeBytes_SIZE(self)) {
			err_index_out_of_bounds((DeeObject *)self,
			                        index,
			                        DeeBytes_SIZE(self));
			goto err;
		}
	} else if unlikely(DeeBytes_SIZE(self) != 1) {
		err_expected_single_character_string((DeeObject *)self);
		goto err;
	}
	return DeeInt_NewU8(DeeBytes_DATA(self)[index]);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *__restrict self,
                DeeObject *__restrict format);
PRIVATE DREF DeeObject *DCALL
bytes_scanf(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	DeeObject *format;
	if (DeeArg_Unpack(argc, argv, "o:scanf", &format) ||
	    DeeObject_AssertTypeExact(format, &DeeString_Type))
		return NULL;
	return DeeString_Scanf((DeeObject *)self, format);
}


#define DeeBytes_IsPrint(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_FPRINT)
#define DeeBytes_IsAlpha(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_FALPHA)
#define DeeBytes_IsSpace(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_FSPACE)
#define DeeBytes_IsLF(self, start, end)         DeeBytes_TestTrait(self, start, end, UNICODE_FLF)
#define DeeBytes_IsLower(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_FLOWER)
#define DeeBytes_IsUpper(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_FUPPER)
#define DeeBytes_IsCntrl(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_FCNTRL)
#define DeeBytes_IsDigit(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_FDIGIT)
#define DeeBytes_IsDecimal(self, start, end)    DeeBytes_TestTrait(self, start, end, UNICODE_FDECIMAL)
#define DeeBytes_IsSymStrt(self, start, end)    DeeBytes_TestTrait(self, start, end, UNICODE_FSYMSTRT)
#define DeeBytes_IsSymCont(self, start, end)    DeeBytes_TestTrait(self, start, end, UNICODE_FSYMCONT)
#define DeeBytes_IsAlnum(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_FALPHA|UNICODE_FDECIMAL)
#define DeeBytes_IsNumeric(self, start, end)    DeeBytes_TestTrait(self, start, end, UNICODE_FDIGIT|UNICODE_FDECIMAL)
#define DeeBytes_IsAnyPrint(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FPRINT)
#define DeeBytes_IsAnyAlpha(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FALPHA)
#define DeeBytes_IsAnySpace(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FSPACE)
#define DeeBytes_IsAnyLF(self, start, end)      DeeBytes_TestAnyTrait(self, start, end, UNICODE_FLF)
#define DeeBytes_IsAnyLower(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FLOWER)
#define DeeBytes_IsAnyUpper(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FUPPER)
#define DeeBytes_IsAnyTitle(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FTITLE)
#define DeeBytes_IsAnyCntrl(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FCNTRL)
#define DeeBytes_IsAnyDigit(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FDIGIT)
#define DeeBytes_IsAnyDecimal(self, start, end) DeeBytes_TestAnyTrait(self, start, end, UNICODE_FDECIMAL)
#define DeeBytes_IsAnySymStrt(self, start, end) DeeBytes_TestAnyTrait(self, start, end, UNICODE_FSYMSTRT)
#define DeeBytes_IsAnySymCont(self, start, end) DeeBytes_TestAnyTrait(self, start, end, UNICODE_FSYMCONT)
#define DeeBytes_IsAnyAlnum(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_FALPHA|UNICODE_FDECIMAL)
#define DeeBytes_IsAnyNumeric(self, start, end) DeeBytes_TestAnyTrait(self, start, end, UNICODE_FDIGIT|UNICODE_FDECIMAL)

INTERN bool DCALL
DeeBytes_TestTrait(Bytes *__restrict self,
                   size_t start_index,
                   size_t end_index,
                   uniflag_t flags) {
	uint8_t *iter;
	if (start_index > DeeBytes_SIZE(self))
		start_index = DeeBytes_SIZE(self);
	if (end_index > DeeBytes_SIZE(self))
		end_index = DeeBytes_SIZE(self);
	iter = DeeBytes_DATA(self) + start_index;
	while (start_index < end_index) {
		if (!(DeeUni_Flags(*iter) & flags))
			return false;
		++iter, ++start_index;
	}
	return true;
}

INTERN bool DCALL
DeeBytes_IsAscii(Bytes *__restrict self,
                 size_t start_index,
                 size_t end_index) {
	uint8_t *iter;
	if (start_index > DeeBytes_SIZE(self))
		start_index = DeeBytes_SIZE(self);
	if (end_index > DeeBytes_SIZE(self))
		end_index = DeeBytes_SIZE(self);
	iter = DeeBytes_DATA(self) + start_index;
	while (start_index < end_index) {
		if (*iter > 0x7f)
			return false;
		++iter, ++start_index;
	}
	return true;
}

INTERN bool DCALL
DeeBytes_TestAnyTrait(Bytes *__restrict self,
                      size_t start_index,
                      size_t end_index,
                      uniflag_t flags) {
	uint8_t *iter;
	if (end_index > DeeBytes_SIZE(self))
		end_index = DeeBytes_SIZE(self);
	if (start_index < end_index) {
		iter = DeeBytes_DATA(self) + start_index;
		while (start_index < end_index) {
			if (DeeUni_Flags(*iter) & flags)
				return true;
			++iter;
			++start_index;
		}
	}
	return false;
}

INTERN bool DCALL
DeeBytes_IsAnyAscii(Bytes *__restrict self,
                    size_t start_index,
                    size_t end_index) {
	uint8_t *iter;
	if (end_index > DeeBytes_SIZE(self))
		end_index = DeeBytes_SIZE(self);
	if (start_index < end_index) {
		iter = DeeBytes_DATA(self) + start_index;
		while (start_index < end_index) {
			if (*iter <= 0x7f)
				return true;
			++iter;
			++start_index;
		}
	}
	return false;
}


INTERN bool DCALL
DeeBytes_IsTitle(Bytes *__restrict self,
                 size_t start_index,
                 size_t end_index) {
	uniflag_t flags = (UNICODE_FTITLE | UNICODE_FUPPER | UNICODE_FSPACE);
	uint8_t *iter;
	if (end_index > DeeBytes_SIZE(self))
		end_index = DeeBytes_SIZE(self);
	if (start_index < end_index) {
		iter = DeeBytes_DATA(self) + start_index;
		while (start_index < end_index) {
			uniflag_t f = DeeUni_Flags(*iter);
			if (!(f & flags))
				return false;
			flags = (f & UNICODE_FSPACE) ? (UNICODE_FTITLE | UNICODE_FUPPER | UNICODE_FSPACE)
			                             : (UNICODE_FLOWER | UNICODE_FSPACE);
			++iter;
			++start_index;
		}
	}
	return true;
}

INTERN bool DCALL
DeeBytes_IsSymbol(Bytes *__restrict self,
                  size_t start_index,
                  size_t end_index) {
	uniflag_t flags = UNICODE_FSYMSTRT;
	uint8_t *iter;
	if (end_index > DeeBytes_SIZE(self))
		end_index = DeeBytes_SIZE(self);
	if (start_index < end_index) {
		iter = DeeBytes_DATA(self) + start_index;
		while (start_index < end_index) {
			if (!(DeeUni_Flags(*iter) & flags))
				return false;
			flags = UNICODE_FSYMCONT;
			++iter;
			++start_index;
		}
	}
	return true;
}

#define DEFINE_BYTES_TRAIT(name, function, test_ch)                     \
	PRIVATE DREF DeeObject *DCALL                                       \
	bytes_##name(Bytes *__restrict self,                                \
	             size_t argc, DeeObject **__restrict argv) {            \
		size_t start = 0, end = (size_t)-1;                             \
		if (argc == 1) {                                                \
			uint8_t ch;                                                 \
			if (DeeObject_AsSize(argv[0], &start))                      \
				goto err;                                               \
			if unlikely(start >= DeeBytes_SIZE(self)) {                 \
				err_index_out_of_bounds((DeeObject *)self,              \
				                        start,                          \
				                        DeeBytes_SIZE(self));           \
				goto err;                                               \
			}                                                           \
			ch = DeeBytes_DATA(self)[start];                            \
			return_bool(test_ch);                                       \
		} else {                                                        \
			if (DeeArg_Unpack(argc, argv, "|IdId" #name, &start, &end)) \
				goto err;                                               \
			return_bool(function(self, start, end));                    \
		}                                                               \
	err:                                                                \
		return NULL;                                                    \
	}
#define DEFINE_ANY_BYTES_TRAIT(name, function)                                           \
	PRIVATE DREF DeeObject *DCALL                                                        \
	bytes_##name(Bytes *__restrict self, size_t argc,                                    \
	             DeeObject **__restrict argv, DeeObject *kw) {                           \
		size_t start = 0, end = (size_t)-1;                                              \
		if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId" #name, &start, &end)) \
			return NULL;                                                                 \
		return_bool(function(self, start, end));                                         \
	}
DEFINE_BYTES_TRAIT(isprint, DeeBytes_IsPrint, DeeUni_Flags(ch) & UNICODE_FPRINT)
DEFINE_BYTES_TRAIT(isalpha, DeeBytes_IsAlpha, DeeUni_Flags(ch) & UNICODE_FALPHA)
DEFINE_BYTES_TRAIT(isspace, DeeBytes_IsSpace, DeeUni_Flags(ch) & UNICODE_FSPACE)
DEFINE_BYTES_TRAIT(islf, DeeBytes_IsLF, DeeUni_Flags(ch) & UNICODE_FLF)
DEFINE_BYTES_TRAIT(islower, DeeBytes_IsLower, DeeUni_Flags(ch) & UNICODE_FLOWER)
DEFINE_BYTES_TRAIT(isupper, DeeBytes_IsUpper, DeeUni_Flags(ch) & UNICODE_FUPPER)
DEFINE_BYTES_TRAIT(iscntrl, DeeBytes_IsCntrl, DeeUni_Flags(ch) & UNICODE_FCNTRL)
DEFINE_BYTES_TRAIT(isdigit, DeeBytes_IsDigit, DeeUni_Flags(ch) & UNICODE_FDIGIT)
DEFINE_BYTES_TRAIT(isdecimal, DeeBytes_IsDecimal, DeeUni_Flags(ch) & UNICODE_FDECIMAL)
DEFINE_BYTES_TRAIT(issymstrt, DeeBytes_IsSymStrt, DeeUni_Flags(ch) & UNICODE_FSYMSTRT)
DEFINE_BYTES_TRAIT(issymcont, DeeBytes_IsSymStrt, DeeUni_Flags(ch) & UNICODE_FSYMCONT)
DEFINE_BYTES_TRAIT(isalnum, DeeBytes_IsAlnum, DeeUni_Flags(ch) & (UNICODE_FALPHA | UNICODE_FDECIMAL))
DEFINE_BYTES_TRAIT(isnumeric, DeeBytes_IsNumeric, DeeUni_Flags(ch) & (UNICODE_FDECIMAL | UNICODE_FDIGIT))
DEFINE_BYTES_TRAIT(istitle, DeeBytes_IsTitle, DeeUni_Flags(ch) & UNICODE_FTITLE)
DEFINE_BYTES_TRAIT(issymbol, DeeBytes_IsSymbol, DeeUni_Flags(ch) & UNICODE_FSYMSTRT)
DEFINE_BYTES_TRAIT(isascii, DeeBytes_IsAscii, ch <= 0x7f)
DEFINE_ANY_BYTES_TRAIT(isanyprint, DeeBytes_IsAnyPrint)
DEFINE_ANY_BYTES_TRAIT(isanyalpha, DeeBytes_IsAnyAlpha)
DEFINE_ANY_BYTES_TRAIT(isanyspace, DeeBytes_IsAnySpace)
DEFINE_ANY_BYTES_TRAIT(isanylf, DeeBytes_IsAnyLF)
DEFINE_ANY_BYTES_TRAIT(isanylower, DeeBytes_IsAnyLower)
DEFINE_ANY_BYTES_TRAIT(isanyupper, DeeBytes_IsAnyUpper)
DEFINE_ANY_BYTES_TRAIT(isanycntrl, DeeBytes_IsAnyCntrl)
DEFINE_ANY_BYTES_TRAIT(isanydigit, DeeBytes_IsAnyDigit)
DEFINE_ANY_BYTES_TRAIT(isanydecimal, DeeBytes_IsAnyDecimal)
DEFINE_ANY_BYTES_TRAIT(isanysymstrt, DeeBytes_IsAnySymStrt)
DEFINE_ANY_BYTES_TRAIT(isanysymcont, DeeBytes_IsAnySymStrt)
DEFINE_ANY_BYTES_TRAIT(isanyalnum, DeeBytes_IsAnyAlnum)
DEFINE_ANY_BYTES_TRAIT(isanynumeric, DeeBytes_IsAnyNumeric)
DEFINE_ANY_BYTES_TRAIT(isanytitle, DeeBytes_IsAnyTitle)
DEFINE_ANY_BYTES_TRAIT(isanyascii, DeeBytes_IsAnyAscii)
#undef DEFINE_ANY_BYTES_TRAIT
#undef DEFINE_BYTES_TRAIT

PRIVATE DREF DeeObject *DCALL
bytes_asnumber(Bytes *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	uint8_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	if (argc == 0) {
		if unlikely(DeeBytes_SIZE(self) != 1)
			goto err_not_single_char;
		ch = DeeBytes_DATA(self)[0];
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, "Iu|o:asnumber", &index, &defl))
			goto err;
		if unlikely(index >= DeeBytes_SIZE(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeBytes_SIZE(self));
			goto err;
		}
		ch = DeeBytes_DATA(self)[index];
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & (UNICODE_FDIGIT | UNICODE_FDECIMAL)))
		goto err_not_numeric;
	return DeeInt_NewU8(trt->ut_digit);
err_not_numeric:
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I8C is not numeric",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_asdigit(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	uint32_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	if (argc == 0) {
		if unlikely(DeeBytes_SIZE(self) != 1)
			goto err_not_single_char;
		ch = DeeBytes_DATA(self)[0];
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, "Iu|o:asdigit", &index, &defl))
			goto err;
		if unlikely(index >= DeeBytes_SIZE(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeBytes_SIZE(self));
			goto err;
		}
		ch = DeeBytes_DATA(self)[index];
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & UNICODE_FDIGIT))
		goto err_not_numeric;
	return DeeInt_NewU8(trt->ut_digit);
err_not_numeric:
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I8C isn't a digit",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_asdecimal(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	uint32_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	if (argc == 0) {
		if unlikely(DeeBytes_SIZE(self) != 1)
			goto err_not_single_char;
		ch = DeeBytes_DATA(self)[0];
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, "Iu|o:asdecimal", &index, &defl))
			goto err;
		if unlikely(index >= DeeBytes_SIZE(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeBytes_SIZE(self));
			goto err;
		}
		ch = DeeBytes_DATA(self)[index];
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & UNICODE_FDECIMAL))
		goto err_not_numeric;
	return DeeInt_NewU8(trt->ut_digit);
err_not_numeric:
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I8C isn't a decimal",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}


PRIVATE DREF Bytes *DCALL
bytes_lower(Bytes *__restrict self, size_t argc,
            DeeObject **__restrict argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:lower", &start, &end))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	end -= start;
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
	if unlikely(!result)
		goto done;
	for (i = 0; i < end; ++i)
		DeeBytes_DATA(result)[i] = (uint8_t)DeeUni_ToLower(DeeBytes_DATA(self)[start + i]);
done:
	return result;
}

PRIVATE DREF Bytes *DCALL
bytes_upper(Bytes *__restrict self, size_t argc,
            DeeObject **__restrict argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:upper", &start, &end))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	end -= start;
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
	if unlikely(!result)
		goto done;
	for (i = 0; i < end; ++i)
		DeeBytes_DATA(result)[i] = (uint8_t)DeeUni_ToUpper(DeeBytes_DATA(self)[start + i]);
done:
	return result;
}

PRIVATE DREF Bytes *DCALL
bytes_title(Bytes *__restrict self, size_t argc,
            DeeObject **__restrict argv, DeeObject *kw) {
	uintptr_t kind = UNICODE_CONVERT_TITLE;
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:title", &start, &end))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	end -= start;
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
	if unlikely(!result)
		goto done;
	for (i = 0; i < end; ++i) {
		uint8_t ch               = DeeBytes_DATA(self)[start + i];
		DeeBytes_DATA(result)[i] = kind == UNICODE_CONVERT_TITLE
		                           ? (uint8_t)DeeUni_ToTitle(ch)
		                           : (uint8_t)DeeUni_ToLower(ch);
		kind = DeeUni_IsSpace(ch) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
	}
done:
	return result;
}

PRIVATE DREF Bytes *DCALL
bytes_capitalize(Bytes *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:capitalize", &start, &end))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	end -= start;
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
	if unlikely(!result)
		goto done;
	DeeBytes_DATA(result)[0] = (uint8_t)DeeUni_ToUpper(DeeBytes_DATA(self)[start]);
	for (i = 1; i < end; ++i)
		DeeBytes_DATA(result)[i] = (uint8_t)DeeUni_ToLower(DeeBytes_DATA(self)[start + i]);
done:
	return result;
}

PRIVATE DREF Bytes *DCALL
bytes_swapcase(Bytes *__restrict self, size_t argc,
               DeeObject **__restrict argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:swapcase", &start, &end))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_reference_((DREF Bytes *)Dee_EmptyBytes);
	end -= start;
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(end);
	if unlikely(!result)
		goto done;
	for (i = 0; i < end; ++i)
		DeeBytes_DATA(result)[i] = (uint8_t)DeeUni_SwapCase(DeeBytes_DATA(self)[start + i]);
done:
	return result;
}



PRIVATE DREF Bytes *DCALL
bytes_tolower(Bytes *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:tolower", &start, &end))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(!DeeBytes_WRITABLE(self) && start < end) {
		err_bytes_not_writable((DeeObject *)self);
		goto err;
	}
	for (i = start; i < end; ++i)
		DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_ToLower(DeeBytes_DATA(self)[i]);
	return_reference_(self);
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_toupper(Bytes *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:toupper", &start, &end))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(!DeeBytes_WRITABLE(self) && start < end) {
		err_bytes_not_writable((DeeObject *)self);
		goto err;
	}
	for (i = start; i < end; ++i)
		DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_ToUpper(DeeBytes_DATA(self)[i]);
	return_reference_(self);
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_totitle(Bytes *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	uintptr_t kind = UNICODE_CONVERT_TITLE;
	size_t i, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:totitle", &start, &end))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(!DeeBytes_WRITABLE(self) && start < end) {
		err_bytes_not_writable((DeeObject *)self);
		goto err;
	}
	for (i = start; i < end; ++i) {
		uint8_t ch             = DeeBytes_DATA(self)[i];
		DeeBytes_DATA(self)[i] = kind == UNICODE_CONVERT_TITLE
		                         ? (uint8_t)DeeUni_ToTitle(ch)
		                         : (uint8_t)DeeUni_ToLower(ch);
		kind = DeeUni_IsSpace(ch) ? UNICODE_CONVERT_TITLE : UNICODE_CONVERT_LOWER;
	}
	return_reference_(self);
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_tocapitalize(Bytes *__restrict self, size_t argc,
                   DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:tocapitalize", &start, &end))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start < end) {
		size_t i = start;
		if unlikely(!DeeBytes_WRITABLE(self)) {
			err_bytes_not_writable((DeeObject *)self);
			goto err;
		}
		DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_ToUpper(DeeBytes_DATA(self)[i]);
		++i;
		for (; i < end; ++i)
			DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_ToLower(DeeBytes_DATA(self)[i]);
	}
	return_reference_(self);
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_toswapcase(Bytes *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:toswapcase", &start, &end))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(!DeeBytes_WRITABLE(self) && start < end) {
		err_bytes_not_writable((DeeObject *)self);
		goto err;
	}
	for (i = start; i < end; ++i)
		DeeBytes_DATA(self)[i] = (uint8_t)DeeUni_SwapCase(DeeBytes_DATA(self)[i]);
	return_reference_(self);
err:
	return NULL;
}

#ifndef PRIVATE_REPLACE_KWLIST_DEFINED
#define PRIVATE_REPLACE_KWLIST_DEFINED 1
PRIVATE struct keyword replace_kwlist[] = { K(find), K(replace), K(max), KEND };
#endif /* !PRIVATE_REPLACE_KWLIST_DEFINED */

PRIVATE DREF Bytes *DCALL
bytes_replace(Bytes *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	DREF Bytes *result;
	uint8_t *begin, *end, *block_begin;
	DeeObject *find_ob, *replace_ob;
	size_t max_count = (size_t)-1;
	Needle find_needle, replace_needle;
	struct bytes_printer printer;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|Iu:replace", &find_ob, &replace_ob, &max_count) ||
	    get_needle(&find_needle, find_ob) || get_needle(&replace_needle, replace_ob))
		return NULL;
	/* Handle special cases. */
	if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
		goto return_self;
	if unlikely(!find_needle.n_size) {
		if (DeeBytes_SIZE(self))
			goto return_self;
		result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(replace_needle.n_size);
		if likely(result)
			memcpy(result->b_data, replace_needle.n_data, replace_needle.n_size);
		return result;
	}
	bytes_printer_init(&printer);
	end         = (begin = DeeBytes_DATA(self)) + (DeeBytes_SIZE(self) - (find_needle.n_size - 1));
	block_begin = begin;
	if likely(max_count)
		while (begin <= end) {
		if (MEMEQB(begin, find_needle.n_data, find_needle.n_size)) {
			/* Found one */
			if (unlikely(bytes_printer_append(&printer, block_begin, (size_t)(begin - block_begin)) < 0) ||
			    unlikely(bytes_printer_append(&printer, replace_needle.n_data, replace_needle.n_size) < 0))
				goto err;
			begin += find_needle.n_size;
			block_begin = begin;
			if (begin >= end)
				break;
			if unlikely(!--max_count)
				break;
			continue;
		}
		++begin;
	}
	if unlikely(bytes_printer_append(&printer, block_begin,
		                              (size_t)((end - block_begin) +
		                                       (find_needle.n_size - 1))) < 0)
	goto err;
	/* Pack together a Bytes object. */
	return (DREF Bytes *)bytes_printer_pack(&printer);
err:
	bytes_printer_fini(&printer);
	return NULL;
return_self:
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(DeeBytes_SIZE(self));
	if likely(result)
		memcpy(result->b_data, DeeBytes_DATA(self), DeeBytes_SIZE(self));
	return result;
}

PRIVATE DREF Bytes *DCALL
bytes_casereplace(Bytes *__restrict self, size_t argc,
                  DeeObject **__restrict argv, DeeObject *kw) {
	DREF Bytes *result;
	uint8_t *begin, *end, *block_begin;
	DeeObject *find_ob, *replace_ob;
	size_t max_count = (size_t)-1;
	Needle find_needle, replace_needle;
	struct bytes_printer printer;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|Iu:casereplace", &find_ob, &replace_ob, &max_count) ||
	    get_needle(&find_needle, find_ob) || get_needle(&replace_needle, replace_ob))
		return NULL;
	/* Handle special cases. */
	if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
		goto return_self;
	if unlikely(!find_needle.n_size) {
		if (DeeBytes_SIZE(self))
			goto return_self;
		result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(replace_needle.n_size);
		if likely(result)
			memcpy(result->b_data, replace_needle.n_data, replace_needle.n_size);
		return result;
	}
	bytes_printer_init(&printer);
	end         = (begin = DeeBytes_DATA(self)) + (DeeBytes_SIZE(self) - (find_needle.n_size - 1));
	block_begin = begin;
	if likely(max_count)
		while (begin <= end) {
		if (dee_memasciicaseeq(begin, find_needle.n_data, find_needle.n_size)) {
			/* Found one */
			if (unlikely(bytes_printer_append(&printer, block_begin, (size_t)(begin - block_begin)) < 0) ||
			    unlikely(bytes_printer_append(&printer, replace_needle.n_data, replace_needle.n_size) < 0))
				goto err;
			begin += find_needle.n_size;
			block_begin = begin;
			if (begin >= end)
				break;
			if unlikely(!--max_count)
				break;
			continue;
		}
		++begin;
	}
	if unlikely(bytes_printer_append(&printer, block_begin,
		                              (size_t)((end - block_begin) +
		                                       (find_needle.n_size - 1))) < 0)
	goto err;
	/* Pack together a Bytes object. */
	return (DREF Bytes *)bytes_printer_pack(&printer);
err:
	bytes_printer_fini(&printer);
	return NULL;
return_self:
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(DeeBytes_SIZE(self));
	if likely(result)
		memcpy(result->b_data, DeeBytes_DATA(self), DeeBytes_SIZE(self));
	return result;
}


PRIVATE DREF Bytes *DCALL
bytes_toreplace(Bytes *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob, *replace_ob;
	size_t max_count = (size_t)-1;
	Needle find_needle, replace_needle;
	uint8_t *begin, *end;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|Iu:toreplace", &find_ob, &replace_ob, &max_count) ||
	    get_needle(&find_needle, find_ob) || get_needle(&replace_needle, replace_ob))
		goto err;
	if unlikely(find_needle.n_size != replace_needle.n_size) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Find(%Iu) and replace(%Iu) needles have different sizes",
		                find_needle.n_size, replace_needle.n_size);
		goto err;
	}
	if unlikely(!DeeBytes_WRITABLE(self)) {
		err_bytes_not_writable((DeeObject *)self);
		goto err;
	}

	/* Handle special cases. */
	if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
		goto done;
	if unlikely(!find_needle.n_size)
		goto done;

	end = (begin = DeeBytes_DATA(self)) + (DeeBytes_SIZE(self) - (find_needle.n_size - 1));
	if likely(max_count)
		while (begin <= end) {
		if (MEMEQB(begin, find_needle.n_data, find_needle.n_size)) {
			/* Found one */
			memcpy(begin, replace_needle.n_data, replace_needle.n_size);
			begin += find_needle.n_size;
			if (begin >= end)
				break;
			if unlikely(!--max_count)
				break;
			continue;
		}
		++begin;
	}
done:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE DREF Bytes *DCALL
bytes_tocasereplace(Bytes *__restrict self, size_t argc,
                    DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob, *replace_ob;
	size_t max_count = (size_t)-1;
	Needle find_needle, replace_needle;
	uint8_t *begin, *end;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|Iu:tocasereplace", &find_ob, &replace_ob, &max_count) ||
	    get_needle(&find_needle, find_ob) || get_needle(&replace_needle, replace_ob))
		goto err;
	if unlikely(find_needle.n_size != replace_needle.n_size) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Find(%Iu) and replace(%Iu) needles have different sizes",
		                find_needle.n_size, replace_needle.n_size);
		goto err;
	}
	if unlikely(!DeeBytes_WRITABLE(self)) {
		err_bytes_not_writable((DeeObject *)self);
		goto err;
	}

	/* Handle special cases. */
	if unlikely(find_needle.n_size > DeeBytes_SIZE(self))
		goto done;
	if unlikely(!find_needle.n_size)
		goto done;

	end = (begin = DeeBytes_DATA(self)) + (DeeBytes_SIZE(self) - (find_needle.n_size - 1));
	if likely(max_count)
		while (begin <= end) {
		if (dee_memasciicaseeq(begin, find_needle.n_data, find_needle.n_size)) {
			/* Found one */
			memcpy(begin, replace_needle.n_data, replace_needle.n_size);
			begin += find_needle.n_size;
			if (begin >= end)
				break;
			if unlikely(!--max_count)
				break;
			continue;
		}
		++begin;
	}
done:
	return_reference_(self);
err:
	return NULL;
}


/* The string decode() and encode() member functions also function for `bytes' objects.
 * As a matter of fact: they'd work for any kind of object, however built-in
 *                      codecs only function for bytes and string objects! */
INTDEF DREF DeeObject *DCALL
string_decode(DeeObject *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *DCALL
string_encode(DeeObject *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw);


INTDEF DREF DeeObject *DCALL DeeBytes_SplitByte(Bytes *__restrict self, uint8_t sep);
INTDEF DREF DeeObject *DCALL DeeBytes_Split(Bytes *__restrict self, DeeObject *__restrict sep);
INTDEF DREF DeeObject *DCALL DeeBytes_CaseSplitByte(Bytes *__restrict self, uint8_t sep);
INTDEF DREF DeeObject *DCALL DeeBytes_CaseSplit(Bytes *__restrict self, DeeObject *__restrict sep);
INTDEF DREF DeeObject *DCALL DeeBytes_SplitLines(Bytes *__restrict self, bool keepends);
INTDEF DREF DeeObject *DCALL DeeBytes_FindAll(Bytes *self, DeeObject *other, size_t start, size_t end);
INTDEF DREF DeeObject *DCALL DeeBytes_CaseFindAll(Bytes *self, DeeObject *other, size_t start, size_t end);

PRIVATE DREF DeeObject *DCALL
bytes_findall(Bytes *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:findall", &arg, &start, &end))
		return NULL;
	return DeeBytes_FindAll(self, arg, start, end);
}

PRIVATE DREF DeeObject *DCALL
bytes_casefindall(Bytes *__restrict self, size_t argc,
                  DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casefindall", &arg, &start, &end))
		return NULL;
	return DeeBytes_CaseFindAll(self, arg, start, end);
}

PRIVATE DREF DeeObject *DCALL
bytes_join(Bytes *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:join", &seq))
		goto err;
	{
		struct bytes_printer printer = BYTES_PRINTER_INIT;
		size_t fast_size;
		bool is_first = true;
		DREF DeeObject *iterator, *elem;
		fast_size = DeeFastSeq_GetSize(seq);
		if (fast_size != DEE_FASTSEQ_NOTFAST) {
			/* Fast-sequence optimizations. */
			size_t i;
			for (i = 0; i < fast_size; ++i) {
				/* Print `self' prior to every object, starting with the 2nd one. */
				if unlikely(!is_first &&
					         bytes_printer_append(&printer,
					                              DeeBytes_DATA(self),
					                              DeeBytes_SIZE(self)) < 0)
				goto err_printer;
				elem = DeeFastSeq_GetItem(seq, i);
				if unlikely(!elem)
					goto err_printer;
				/* NOTE: `bytes_printer_printobject()' automatically
				 *        optimizes for other bytes objects being printed. */
				if unlikely(bytes_printer_printobject(&printer, elem) < 0)
					goto err_elem_noiter;
				Dee_Decref(elem);
				is_first = false;
			}
		} else {
			iterator = DeeObject_IterSelf(seq);
			if unlikely(!iterator)
				goto err_printer;
			while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
				if unlikely(!is_first &&
				            bytes_printer_append(&printer,
				                                 DeeBytes_DATA(self),
				                                 DeeBytes_SIZE(self)) < 0)
					goto err_elem;
				/* NOTE: `bytes_printer_printobject()' automatically
				 *        optimizes for other bytes objects being printed. */
				if unlikely(bytes_printer_printobject(&printer, elem) < 0)
					goto err_elem;
				Dee_Decref(elem);
				is_first = false;
				if (DeeThread_CheckInterrupt())
					goto err_iter;
			}
			if unlikely(!elem)
				goto err_iter;
			Dee_Decref(iterator);
		}
		return bytes_printer_pack(&printer);
err_elem_noiter:
		Dee_Decref(elem);
		goto err_printer;
err_elem:
		Dee_Decref(elem);
err_iter:
		Dee_Decref(iterator);
err_printer:
		bytes_printer_fini(&printer);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_split(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	uint8_t sep;
	if (DeeArg_Unpack(argc, argv, "o:split", &other))
		goto err;
	if (DeeString_Check(other) || DeeBytes_Check(other))
		return DeeBytes_Split(self, other);
	if (DeeObject_AsUInt8(other, &sep))
		goto err;
	return DeeBytes_SplitByte(self, sep);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casesplit(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	uint8_t sep;
	if (DeeArg_Unpack(argc, argv, "o:casesplit", &other))
		goto err;
	if (DeeString_Check(other) || DeeBytes_Check(other))
		return DeeBytes_CaseSplit(self, other);
	if (DeeObject_AsUInt8(other, &sep))
		goto err;
	return DeeBytes_CaseSplitByte(self, sep);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_splitlines(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	bool keepends = false;
	if (DeeArg_Unpack(argc, argv, "|b:splitlines", &keepends))
		return NULL;
	return DeeBytes_SplitLines(self, keepends);
}

PRIVATE DREF DeeObject *DCALL
bytes_startswith(Bytes *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	Needle needle;
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:startswith", &arg, &start, &end) ||
	    get_needle(&needle, arg))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start > end ||
	    (end -= start) < needle.n_size)
		return_false;
	return_bool(MEMEQB(DeeBytes_DATA(self) + start,
	                   needle.n_data, needle.n_size));
}

PRIVATE DREF DeeObject *DCALL
bytes_casestartswith(Bytes *__restrict self, size_t argc,
                     DeeObject **__restrict argv, DeeObject *kw) {
	Needle needle;
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casestartswith", &arg, &start, &end) ||
	    get_needle(&needle, arg))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start > end ||
	    (end -= start) < needle.n_size)
		return_false;
	return_bool(dee_memasciicaseeq(DeeBytes_DATA(self) + start,
	                               needle.n_data, needle.n_size));
}

PRIVATE DREF DeeObject *DCALL
bytes_endswith(Bytes *__restrict self, size_t argc,
               DeeObject **__restrict argv, DeeObject *kw) {
	Needle needle;
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:endswith", &arg, &start, &end) ||
	    get_needle(&needle, arg))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start > end ||
	    (end - start) < needle.n_size)
		return_false;
	return_bool(MEMEQB(DeeBytes_DATA(self) +
	                   (end - needle.n_size),
	                   needle.n_data, needle.n_size));
}

PRIVATE DREF DeeObject *DCALL
bytes_caseendswith(Bytes *__restrict self, size_t argc,
                   DeeObject **__restrict argv, DeeObject *kw) {
	Needle needle;
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caseendswith", &arg, &start, &end) ||
	    get_needle(&needle, arg))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start > end ||
	    (end - start) < needle.n_size)
		return_false;
	return_bool(dee_memasciicaseeq(DeeBytes_DATA(self) +
	                               (end - needle.n_size),
	                               needle.n_data, needle.n_size));
}

PRIVATE DREF DeeObject *DCALL
bytes_pack_partition(Bytes *__restrict self, uint8_t *find_ptr,
                     uint8_t *__restrict start_ptr, size_t search_size,
                     size_t needle_len) {
	DREF DeeObject *result, *temp;
	if (!find_ptr)
		return DeeTuple_Pack(3, self, Dee_EmptyBytes, Dee_EmptyBytes);
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto done;
	temp = DeeBytes_NewSubView(self, start_ptr,
	                           (size_t)(find_ptr - start_ptr));
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 0, temp); /* Inherit reference. */
	temp = DeeBytes_NewSubView(self, find_ptr, needle_len);
	if unlikely(!temp)
		goto err_r_1;
	DeeTuple_SET(result, 1, temp); /* Inherit reference. */
	find_ptr += needle_len;
	temp = DeeBytes_NewSubView(self, find_ptr,
	                           (start_ptr + search_size) - find_ptr);
	if unlikely(!temp)
		goto err_r_2;
	DeeTuple_SET(result, 2, temp); /* Inherit reference. */
done:
	return result;
err_r_2:
	Dee_Decref(DeeTuple_GET(result, 1));
err_r_1:
	Dee_Decref(DeeTuple_GET(result, 0));
err_r_0:
	DeeTuple_FreeUninitialized(result);
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_parition(Bytes *__restrict self, size_t argc,
               DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:partition", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return DeeTuple_Pack(3, Dee_EmptyBytes, Dee_EmptyBytes, Dee_EmptyBytes);
	end -= start;
	return bytes_pack_partition(self,
	                            memmemb(DeeBytes_DATA(self) + start,
	                                    end,
	                                    needle.n_data,
	                                    needle.n_size),
	                            DeeBytes_DATA(self) + start,
	                            end,
	                            needle.n_size);
}

PRIVATE DREF DeeObject *DCALL
bytes_caseparition(Bytes *__restrict self, size_t argc,
                   DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casepartition", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return DeeTuple_Pack(3, Dee_EmptyBytes, Dee_EmptyBytes, Dee_EmptyBytes);
	end -= start;
	return bytes_pack_partition(self,
	                            dee_memasciicasemem(DeeBytes_DATA(self) + start,
	                                                end,
	                                                needle.n_data,
	                                                needle.n_size),
	                            DeeBytes_DATA(self) + start,
	                            end,
	                            needle.n_size);
}

PRIVATE DREF DeeObject *DCALL
bytes_rparition(Bytes *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:rpartition", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return DeeTuple_Pack(3, Dee_EmptyBytes, Dee_EmptyBytes, Dee_EmptyBytes);
	end -= start;
	return bytes_pack_partition(self,
	                            memrmemb(DeeBytes_DATA(self) + start,
	                                     end,
	                                     needle.n_data,
	                                     needle.n_size),
	                            DeeBytes_DATA(self) + start,
	                            end,
	                            needle.n_size);
}

PRIVATE DREF DeeObject *DCALL
bytes_caserparition(Bytes *__restrict self, size_t argc,
                    DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caserpartition", &find_ob, &start, &end) ||
	    get_needle(&needle, find_ob))
		return NULL;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return DeeTuple_Pack(3, Dee_EmptyBytes, Dee_EmptyBytes, Dee_EmptyBytes);
	end -= start;
	return bytes_pack_partition(self,
	                            dee_memasciicasermem(DeeBytes_DATA(self) + start,
	                                                 end,
	                                                 needle.n_data,
	                                                 needle.n_size),
	                            DeeBytes_DATA(self) + start,
	                            end,
	                            needle.n_size);
}

PRIVATE DREF DeeObject *DCALL
bytes_strip(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin, *end;
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:strip", &mask))
		goto err;
	begin = DeeBytes_DATA(self);
	end   = begin + DeeBytes_SIZE(self);
	if (mask) {
		Needle needle;
		if (get_needle(&needle, mask))
			goto err;
		while (begin < end && memchr(needle.n_data, *begin, needle.n_size))
			++begin;
		while (end > begin && memchr(needle.n_data, end[-1], needle.n_size))
			--end;
	} else {
		while (begin < end && DeeUni_IsSpace(*begin))
			++begin;
		while (end > begin && DeeUni_IsSpace(end[-1]))
			--end;
	}
	if (begin == DeeBytes_DATA(self) &&
	    end == begin + DeeBytes_SIZE(self))
		return_reference_((DeeObject *)self);
	return DeeBytes_NewSubView(self,
	                           begin,
	                           (size_t)(end - begin));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casestrip(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin, *end;
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:casestrip", &mask))
		goto err;
	begin = DeeBytes_DATA(self);
	end   = begin + DeeBytes_SIZE(self);
	if (mask) {
		Needle needle;
		if (get_needle(&needle, mask))
			goto err;
		while (begin < end && dee_memasciicasechr(needle.n_data, *begin, needle.n_size))
			++begin;
		while (end > begin && dee_memasciicasechr(needle.n_data, end[-1], needle.n_size))
			--end;
	} else {
		while (begin < end && DeeUni_IsSpace(*begin))
			++begin;
		while (end > begin && DeeUni_IsSpace(end[-1]))
			--end;
	}
	if (begin == DeeBytes_DATA(self) &&
	    end == begin + DeeBytes_SIZE(self))
		return_reference_((DeeObject *)self);
	return DeeBytes_NewSubView(self,
	                           begin,
	                           (size_t)(end - begin));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_lstrip(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin, *end;
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:lstrip", &mask))
		goto err;
	begin = DeeBytes_DATA(self);
	end   = begin + DeeBytes_SIZE(self);
	if (mask) {
		Needle needle;
		if (get_needle(&needle, mask))
			goto err;
		while (begin < end && memchr(needle.n_data, *begin, needle.n_size))
			++begin;
	} else {
		while (begin < end && DeeUni_IsSpace(*begin))
			++begin;
	}
	if (begin == DeeBytes_DATA(self))
		return_reference_((DeeObject *)self);
	return DeeBytes_NewSubView(self,
	                           begin,
	                           (size_t)(end - begin));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caselstrip(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin, *end;
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:caselstrip", &mask))
		goto err;
	begin = DeeBytes_DATA(self);
	end   = begin + DeeBytes_SIZE(self);
	if (mask) {
		Needle needle;
		if (get_needle(&needle, mask))
			goto err;
		while (begin < end && dee_memasciicasechr(needle.n_data, *begin, needle.n_size))
			++begin;
	} else {
		while (begin < end && DeeUni_IsSpace(*begin))
			++begin;
	}
	if (begin == DeeBytes_DATA(self))
		return_reference_((DeeObject *)self);
	return DeeBytes_NewSubView(self,
	                           begin,
	                           (size_t)(end - begin));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rstrip(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin, *end;
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:rstrip", &mask))
		goto err;
	begin = DeeBytes_DATA(self);
	end   = begin + DeeBytes_SIZE(self);
	if (mask) {
		Needle needle;
		if (get_needle(&needle, mask))
			goto err;
		while (end > begin && memchr(needle.n_data, end[-1], needle.n_size))
			--end;
	} else {
		while (end > begin && DeeUni_IsSpace(end[-1]))
			--end;
	}
	if (end == begin + DeeBytes_SIZE(self))
		return_reference_((DeeObject *)self);
	return DeeBytes_NewSubView(self,
	                           begin,
	                           (size_t)(end - begin));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caserstrip(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin, *end;
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:caserstrip", &mask))
		goto err;
	begin = DeeBytes_DATA(self);
	end   = begin + DeeBytes_SIZE(self);
	if (mask) {
		Needle needle;
		if (get_needle(&needle, mask))
			goto err;
		while (end > begin && dee_memasciicasechr(needle.n_data, end[-1], needle.n_size))
			--end;
	} else {
		while (end > begin && DeeUni_IsSpace(end[-1]))
			--end;
	}
	if (end == begin + DeeBytes_SIZE(self))
		return_reference_((DeeObject *)self);
	return DeeBytes_NewSubView(self,
	                           begin,
	                           (size_t)(end - begin));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_sstrip(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin;
	DeeObject *mask;
	Needle needle;
	size_t size;
	if (DeeArg_Unpack(argc, argv, "o:sstrip", &mask))
		goto err;
	if (get_needle(&needle, mask))
		goto err;
	if (needle.n_size)
		goto retself;
	begin = DeeBytes_DATA(self);
	size  = DeeBytes_SIZE(self);
	while (size >= needle.n_size) {
		if (!MEMEQB(begin, needle.n_data, needle.n_size))
			break;
		begin += needle.n_size;
		size -= needle.n_size;
	}
	while (size >= needle.n_size) {
		if (!MEMEQB(begin + size - needle.n_size, needle.n_data, needle.n_size))
			break;
		size -= needle.n_size;
	}
	if (begin == DeeBytes_DATA(self) &&
	    size == DeeBytes_SIZE(self))
		goto retself;
	return DeeBytes_NewSubView(self,
	                           begin, size);
retself:
	return_reference_((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casesstrip(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin;
	DeeObject *mask;
	Needle needle;
	size_t size;
	if (DeeArg_Unpack(argc, argv, "o:casesstrip", &mask))
		goto err;
	if (get_needle(&needle, mask))
		goto err;
	if (needle.n_size)
		goto retself;
	begin = DeeBytes_DATA(self);
	size  = DeeBytes_SIZE(self);
	while (size >= needle.n_size) {
		if (!dee_memasciicaseeq(begin, needle.n_data, needle.n_size))
			break;
		begin += needle.n_size;
		size -= needle.n_size;
	}
	while (size >= needle.n_size) {
		if (!dee_memasciicaseeq(begin + size - needle.n_size, needle.n_data, needle.n_size))
			break;
		size -= needle.n_size;
	}
	if (begin == DeeBytes_DATA(self) &&
	    size == DeeBytes_SIZE(self))
		goto retself;
	return DeeBytes_NewSubView(self,
	                           begin, size);
retself:
	return_reference_((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_lsstrip(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin;
	DeeObject *mask;
	Needle needle;
	size_t size;
	if (DeeArg_Unpack(argc, argv, "o:lsstrip", &mask))
		goto err;
	if (get_needle(&needle, mask))
		goto err;
	if (needle.n_size)
		goto retself;
	begin = DeeBytes_DATA(self);
	size  = DeeBytes_SIZE(self);
	while (size >= needle.n_size) {
		if (!MEMEQB(begin, needle.n_data, needle.n_size))
			break;
		begin += needle.n_size;
		size -= needle.n_size;
	}
	if (begin == DeeBytes_DATA(self))
		goto retself;
	return DeeBytes_NewSubView(self,
	                           begin, size);
retself:
	return_reference_((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caselsstrip(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin;
	DeeObject *mask;
	Needle needle;
	size_t size;
	if (DeeArg_Unpack(argc, argv, "o:caselsstrip", &mask))
		goto err;
	if (get_needle(&needle, mask))
		goto err;
	if (needle.n_size)
		goto retself;
	begin = DeeBytes_DATA(self);
	size  = DeeBytes_SIZE(self);
	while (size >= needle.n_size) {
		if (!dee_memasciicaseeq(begin, needle.n_data, needle.n_size))
			break;
		begin += needle.n_size;
		size -= needle.n_size;
	}
	if (begin == DeeBytes_DATA(self))
		goto retself;
	return DeeBytes_NewSubView(self,
	                           begin, size);
retself:
	return_reference_((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rsstrip(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin;
	DeeObject *mask;
	Needle needle;
	size_t size;
	if (DeeArg_Unpack(argc, argv, "o:rsstrip", &mask))
		goto err;
	if (get_needle(&needle, mask))
		goto err;
	if (needle.n_size)
		goto retself;
	begin = DeeBytes_DATA(self);
	size  = DeeBytes_SIZE(self);
	while (size >= needle.n_size) {
		if (!MEMEQB(begin + size - needle.n_size, needle.n_data, needle.n_size))
			break;
		size -= needle.n_size;
	}
	if (size == DeeBytes_SIZE(self))
		goto retself;
	return DeeBytes_NewSubView(self,
	                           begin, size);
retself:
	return_reference_((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casersstrip(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	uint8_t *begin;
	DeeObject *mask;
	Needle needle;
	size_t size;
	if (DeeArg_Unpack(argc, argv, "o:casersstrip", &mask))
		goto err;
	if (get_needle(&needle, mask))
		goto err;
	if (needle.n_size)
		goto retself;
	begin = DeeBytes_DATA(self);
	size  = DeeBytes_SIZE(self);
	while (size >= needle.n_size) {
		if (!dee_memasciicaseeq(begin + size - needle.n_size, needle.n_data, needle.n_size))
			break;
		size -= needle.n_size;
	}
	if (size == DeeBytes_SIZE(self))
		goto retself;
	return DeeBytes_NewSubView(self,
	                           begin, size);
retself:
	return_reference_((DeeObject *)self);
err:
	return NULL;
}

struct bcompare_args {
	DeeObject *other; /* [1..1] String or Bytes object. */
	uint8_t *lhs_ptr; /* [0..my_len] Starting pointer of lhs. */
	size_t lhs_len;   /* Number of bytes in lhs. */
	uint8_t *rhs_ptr; /* [0..my_len] Starting pointer of rhs. */
	size_t rhs_len;   /* Number of bytes in rhs. */
};

PRIVATE int DCALL
get_bcompare_args(Bytes *__restrict self,
                  struct bcompare_args *__restrict args,
                  size_t argc, DeeObject **__restrict argv,
                  char const *__restrict funname) {
	DeeObject *other;
	size_t temp, temp2;
	args->lhs_ptr = DeeBytes_DATA(self);
	args->lhs_len = DeeBytes_SIZE(self);
	switch (argc) {
	case 1:
		args->other = other = argv[0];
		if (DeeBytes_Check(other)) {
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
		} else {
			if unlikely(!DeeString_Check(other))
				goto err_type_other;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
		}
		break;
	case 2:
		if (DeeBytes_Check(argv[0])) {
			args->other = other = argv[0];
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&temp))
				goto err;
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		} else if (DeeString_Check(argv[0])) {
			args->other = other = argv[0];
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&temp))
				goto err;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		} else {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&temp))
				goto err;
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			if (DeeBytes_Check(other)) {
				args->rhs_ptr = DeeBytes_DATA(other);
				args->rhs_len = DeeBytes_SIZE(other);
			} else {
				if unlikely(!DeeString_Check(other))
					goto err_type_other;
				args->rhs_ptr = DeeString_AsBytes(other, false);
				if unlikely(!args->rhs_ptr)
					goto err;
				args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			}
		}
		break;
	case 3:
		if (DeeBytes_Check(argv[0])) {
			args->other = other = argv[0];
			args->rhs_ptr       = DeeBytes_DATA(other);
			args->rhs_len       = DeeBytes_SIZE(other);
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&temp))
				goto err;
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&temp2))
				goto err;
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				if (temp2 > args->rhs_len)
					temp2 = args->rhs_len;
				args->rhs_ptr += temp;
				args->rhs_len = temp2 - temp;
			}
		} else if (DeeString_Check(argv[0])) {
			args->other = other = argv[0];
			args->rhs_ptr       = DeeString_AsBytes(other, true);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&temp))
				goto err;
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&temp2))
				goto err;
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				if (temp2 > args->rhs_len)
					temp2 = args->rhs_len;
				args->rhs_ptr += temp;
				args->rhs_len = temp2 - temp;
			}
		} else if (DeeBytes_Check(argv[1])) {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&temp))
				goto err;
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			args->rhs_ptr       = DeeBytes_DATA(other);
			args->rhs_len       = DeeBytes_SIZE(other);
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&temp))
				goto err;
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		} else if (DeeString_Check(argv[1])) {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&temp))
				goto err;
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			args->rhs_ptr       = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&temp))
				goto err;
			if unlikely(temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		} else {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&temp))
				goto err;
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&temp2))
				goto err;
			if (temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				if (temp2 > args->lhs_len)
					temp2 = args->lhs_len;
				args->lhs_ptr += temp;
				args->lhs_len = temp2 - temp;
			}
			args->other = other = argv[2];
			if (DeeBytes_Check(other)) {
				args->rhs_ptr = DeeBytes_DATA(other);
				args->rhs_len = DeeBytes_SIZE(other);
			} else {
				if unlikely(!DeeString_Check(other))
					goto err_type_other;
				args->rhs_ptr = DeeString_AsBytes(other, false);
				if unlikely(!args->rhs_ptr)
					goto err;
				args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			}
		}
		break;
	case 4:
		if (DeeObject_AsSSize(argv[0], (dssize_t *)&temp))
			goto err;
		if (DeeBytes_Check(argv[1])) {
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&temp))
				goto err;
			if (DeeObject_AsSSize(argv[3], (dssize_t *)&temp2))
				goto err;
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
			if (temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				if (temp2 > args->rhs_len)
					temp2 = args->rhs_len;
				args->rhs_ptr += temp;
				args->rhs_len = temp2 - temp;
			}
		} else if (DeeString_Check(argv[1])) {
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				args->lhs_ptr += temp;
				args->lhs_len -= temp;
			}
			args->other = other = argv[1];
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&temp))
				goto err;
			if (DeeObject_AsSSize(argv[3], (dssize_t *)&temp2))
				goto err;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if (temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				if (temp2 > args->rhs_len)
					temp2 = args->rhs_len;
				args->rhs_ptr += temp;
				args->rhs_len = temp2 - temp;
			}
		} else {
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&temp2))
				goto err;
			if unlikely(temp >= args->lhs_len) {
				args->lhs_len = 0;
			} else {
				if (temp2 > args->lhs_len)
					temp2 = args->lhs_len;
				args->lhs_ptr += temp;
				args->lhs_len = temp2 - temp;
			}
			args->other = other = argv[2];
			if unlikely(!DeeString_Check(other))
				goto err_type_other;
			if (DeeObject_AsSSize(argv[3], (dssize_t *)&temp))
				goto err;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
			if (temp >= args->rhs_len) {
				args->rhs_len = 0;
			} else {
				args->rhs_ptr += temp;
				args->rhs_len -= temp;
			}
		}
		break;
	case 5:
		if (DeeObject_AsSSize(argv[0], (dssize_t *)&temp))
			goto err;
		if (DeeObject_AsSSize(argv[1], (dssize_t *)&temp2))
			goto err;
		if (temp >= args->lhs_len) {
			args->lhs_len = 0;
		} else {
			if (temp2 > args->lhs_len)
				temp2 = args->lhs_len;
			args->lhs_ptr += temp;
			args->lhs_len = temp2 - temp;
		}
		args->other = other = argv[2];
		if (DeeBytes_Check(other)) {
			args->rhs_ptr = DeeBytes_DATA(other);
			args->rhs_len = DeeBytes_SIZE(other);
		} else {
			if unlikely(!DeeString_Check(other))
				goto err_type_other;
			args->rhs_ptr = DeeString_AsBytes(other, false);
			if unlikely(!args->rhs_ptr)
				goto err;
			args->rhs_len = WSTR_LENGTH(args->rhs_ptr);
		}
		if (DeeObject_AsSSize(argv[3], (dssize_t *)&temp))
			goto err;
		if (DeeObject_AsSSize(argv[4], (dssize_t *)&temp2))
			goto err;
		if (temp >= args->rhs_len) {
			args->rhs_len = 0;
		} else {
			if (temp2 > args->rhs_len)
				temp2 = args->rhs_len;
			args->rhs_ptr += temp;
			args->rhs_len = temp2 - temp;
		}
		break;
	default:
		err_invalid_argc(funname, argc, 1, 5);
		goto err;
	}
	return 0;
err_type_other:
	DeeObject_TypeAssertFailed(other, &DeeBytes_Type);
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
bytes_compare(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "compare"))
		return NULL;
	if (args.lhs_len < args.rhs_len) {
		result = memcmp(args.lhs_ptr, args.rhs_ptr, args.lhs_len);
		if (result == 0)
			return_reference_(&DeeInt_MinusOne);
	} else if (args.lhs_len > args.rhs_len) {
		result = memcmp(args.lhs_ptr, args.rhs_ptr, args.rhs_len);
		if (result == 0)
			return_reference_(&DeeInt_One);
	} else {
		result = memcmp(args.lhs_ptr, args.rhs_ptr, args.lhs_len);
	}
	return DeeInt_NewInt(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_vercompare(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	int8_t result;
	if (get_bcompare_args(self, &args, argc, argv, "vercompare"))
		return NULL;
	result = dee_strverscmpb(args.lhs_ptr, args.lhs_len,
	                         args.rhs_ptr, args.lhs_len);
	return DeeInt_NewS8(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_wildcompare(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "wildcompare"))
		return NULL;
	result = wildcompareb(args.lhs_ptr, args.lhs_len,
	                      args.rhs_ptr, args.lhs_len);
	return DeeInt_NewInt(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_fuzzycompare(Bytes *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	dssize_t result;
	if (get_bcompare_args(self, &args, argc, argv, "fuzzycompare"))
		goto err;
	result = fuzzy_compareb(args.lhs_ptr, args.lhs_len,
	                        args.rhs_ptr, args.lhs_len);
	if unlikely(result == (dssize_t)-1)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_wmatch(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "wmatch"))
		return NULL;
	result = wildcompareb(args.lhs_ptr, args.lhs_len,
	                      args.rhs_ptr, args.lhs_len);
	return_bool_(result == 0);
}

PRIVATE DREF DeeObject *DCALL
bytes_casecompare(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "casecompare"))
		return NULL;
	if (args.lhs_len < args.rhs_len) {
		result = memasciicasecmp(args.lhs_ptr, args.rhs_ptr, args.lhs_len);
		if (result == 0)
			return_reference_(&DeeInt_MinusOne);
	} else if (args.lhs_len > args.rhs_len) {
		result = memasciicasecmp(args.lhs_ptr, args.rhs_ptr, args.rhs_len);
		if (result == 0)
			return_reference_(&DeeInt_One);
	} else {
		result = memasciicasecmp(args.lhs_ptr, args.rhs_ptr, args.lhs_len);
	}
	return DeeInt_NewInt(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_casevercompare(Bytes *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	int8_t result;
	if (get_bcompare_args(self, &args, argc, argv, "casevercompare"))
		return NULL;
	result = dee_strcaseverscmpb(args.lhs_ptr, args.lhs_len, /* TODO: ASCII variant */
	                             args.rhs_ptr, args.lhs_len);
	return DeeInt_NewS8(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_casewildcompare(Bytes *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "casewildcompare"))
		return NULL;
	result = dee_wildasccicasecompareb(args.lhs_ptr, args.lhs_len,
	                                   args.rhs_ptr, args.lhs_len);
	return DeeInt_NewInt(result);
}

PRIVATE DREF DeeObject *DCALL
bytes_casefuzzycompare(Bytes *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	dssize_t result;
	if (get_bcompare_args(self, &args, argc, argv, "casefuzzycompare"))
		goto err;
	result = fuzzy_asciicasecompareb(args.lhs_ptr, args.lhs_len,
	                                 args.rhs_ptr, args.lhs_len);
	if unlikely(result == (dssize_t)-1)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casewmatch(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "casewmatch"))
		return NULL;
	result = dee_wildasccicasecompareb(args.lhs_ptr, args.lhs_len,
	                                   args.rhs_ptr, args.lhs_len);
	return_bool_(result == 0);
}

PRIVATE DREF DeeObject *DCALL
bytes_center(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *result;
	size_t width;
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, "Iu|o:center", &width, &filler_ob))
		goto err;
	if (filler_ob) {
		if (get_needle(&filler, filler_ob))
			goto err;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = 0x20; /* ' ' */
	}
	if (width <= DeeBytes_SIZE(self)) {
		result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
		                                DeeBytes_SIZE(self));
	} else {
		size_t fill_front, fill_back;
		result = DeeBytes_NewBufferUninitialized(width);
		if unlikely(!result)
			goto err;
		fill_front = (width - DeeBytes_SIZE(self));
		fill_back  = fill_front / 2;
		fill_front -= fill_back;
		memfilb(DeeBytes_DATA(result) + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(DeeBytes_DATA(result) + fill_front,
		        DeeBytes_DATA(self), DeeBytes_SIZE(self));
		memfilb(DeeBytes_DATA(result) + fill_front + DeeBytes_SIZE(self),
		        fill_back, filler.n_data, filler.n_size);
	}
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_ljust(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *result;
	size_t width;
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, "Iu|o:ljust", &width, &filler_ob))
		goto err;
	if (filler_ob) {
		if (get_needle(&filler, filler_ob))
			goto err;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = 0x20; /* ' ' */
	}
	if (width <= DeeBytes_SIZE(self)) {
		result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
		                                DeeBytes_SIZE(self));
	} else {
		size_t fill_back;
		result = DeeBytes_NewBufferUninitialized(width);
		if unlikely(!result)
			goto err;
		fill_back = (width - DeeBytes_SIZE(self));
		memcpyb(DeeBytes_DATA(result) + 0,
		        DeeBytes_DATA(self), DeeBytes_SIZE(self));
		memfilb(DeeBytes_DATA(result) + DeeBytes_SIZE(self),
		        fill_back, filler.n_data, filler.n_size);
	}
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rjust(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *result;
	size_t width;
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, "Iu|o:rjust", &width, &filler_ob))
		goto err;
	if (filler_ob) {
		if (get_needle(&filler, filler_ob))
			goto err;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = 0x20; /* ' ' */
	}
	if (width <= DeeBytes_SIZE(self)) {
		result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
		                                DeeBytes_SIZE(self));
	} else {
		size_t fill_front;
		result = DeeBytes_NewBufferUninitialized(width);
		if unlikely(!result)
			goto err;
		fill_front = (width - DeeBytes_SIZE(self));
		memfilb(DeeBytes_DATA(result) + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(DeeBytes_DATA(result) + fill_front,
		        DeeBytes_DATA(self), DeeBytes_SIZE(self));
	}
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_zfill(Bytes *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *result;
	size_t width;
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, "Iu|o:zfill", &width, &filler_ob))
		goto err;
	if (filler_ob) {
		if (get_needle(&filler, filler_ob))
			goto err;
	} else {
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = 0x30; /* '0' */
	}
	if (width <= DeeBytes_SIZE(self)) {
		result = DeeBytes_NewBufferData(DeeBytes_DATA(self),
		                                DeeBytes_SIZE(self));
	} else {
		size_t fill_front, src_len;
		uint8_t *dst, *src;
		result = DeeBytes_NewBufferUninitialized(width);
		if unlikely(!result)
			goto err;
		dst        = DeeBytes_DATA(result);
		src        = DeeBytes_DATA(self);
		src_len    = DeeBytes_SIZE(self);
		fill_front = (width - src_len);
		while (src_len && DeeUni_IsSign(src[0])) {
			*dst++ = *src++;
			--src_len;
		}
		memfilb(dst + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(dst + fill_front, src, src_len);
	}
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_expandtabs(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	size_t tab_width = 8;
	if (DeeArg_Unpack(argc, argv, "|Iu:expandtabs", &tab_width))
		goto err;
	{
		struct bytes_printer printer = BYTES_PRINTER_INIT;
		uint8_t *iter, *end, *flush_start;
		size_t line_inset = 0;
		iter              = DeeBytes_DATA(self);
		end               = iter + DeeBytes_SIZE(self);
		flush_start       = iter;
		for (; iter < end; ++iter) {
			uint8_t ch = *iter;
			if (!DeeUni_IsTab(ch)) {
				++line_inset;
				if (DeeUni_IsLF(ch))
					line_inset = 0; /* Reset insets at line starts. */
				continue;
			}
			if (bytes_printer_append(&printer, flush_start,
			                         (size_t)(iter - flush_start)) < 0)
				goto err_printer;
			/* Replace with white-space. */
			if likely(tab_width) {
				line_inset = tab_width - (line_inset % tab_width);
				if (bytes_printer_repeat(&printer, ASCII_SPACE, line_inset) < 0)
					goto err_printer;
				line_inset = 0;
			}
			flush_start = iter + 1;
		}
		if (!BYTES_PRINTER_SIZE(&printer))
			goto retself;
		if (bytes_printer_append(&printer, flush_start,
		                         (size_t)(iter - flush_start)) < 0)
			goto err_printer;
		return bytes_printer_pack(&printer);
retself:
		bytes_printer_fini(&printer);
		return_reference_((DeeObject *)self);
err_printer:
		bytes_printer_fini(&printer);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_unifylines(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	DeeObject *replace_ob = NULL;
	Needle replace;
	if (DeeArg_Unpack(argc, argv, "|o:unifylines", &replace_ob))
		goto err;
	if (replace_ob) {
		if (get_needle(&replace, replace_ob))
			goto err;
	} else {
		replace.n_data    = replace._n_buf;
		replace.n_size    = 1;
		replace._n_buf[0] = '\n';
	}
	{
		struct bytes_printer printer = BYTES_PRINTER_INIT;
		uint8_t *iter, *end, *flush_start;
		iter        = DeeBytes_DATA(self);
		end         = iter + DeeBytes_SIZE(self);
		flush_start = iter;
		for (; iter < end; ++iter) {
			uint8_t ch = *iter;
			if (ch != ASCII_CR && ch != ASCII_LF)
				continue; /* Not a line-feed character */
			if (replace.n_size == 1 && ch == replace.n_data[0]) {
				if (ch != ASCII_CR)
					continue; /* No-op replacement. */
				if (iter + 1 >= end)
					continue; /* Cannot be CRLF */
				if (iter[1] != ASCII_LF)
					continue; /* Isn't CRLF */
			}
			if (bytes_printer_append(&printer, flush_start,
			                         (size_t)(iter - flush_start)) < 0)
				goto err_printer;
			if (bytes_printer_append(&printer,
			                         replace.n_data,
			                         replace.n_size) < 0)
				goto err_printer;
			if (ch == ASCII_CR && iter + 1 < end && iter[1] == ASCII_LF)
				++iter;
			flush_start = iter + 1;
		}
		if (!BYTES_PRINTER_SIZE(&printer))
			goto retself;
		if (bytes_printer_append(&printer, flush_start,
		                         (size_t)(iter - flush_start)) < 0)
			goto err_printer;
		return bytes_printer_pack(&printer);
retself:
		bytes_printer_fini(&printer);
		return_reference_((DeeObject *)self);
err_printer:
		bytes_printer_fini(&printer);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_indent(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, "|o:indent", &filler_ob))
		goto err;
	if (filler_ob) {
		if (get_needle(&filler, filler_ob))
			goto err;
		if unlikely(!filler.n_size || DeeBytes_IsEmpty(self))
			goto retself;
	} else {
		if unlikely(DeeBytes_IsEmpty(self))
			goto retself;
		filler.n_data    = filler._n_buf;
		filler.n_size    = 1;
		filler._n_buf[0] = ASCII_TAB;
	}
	{
		struct bytes_printer printer = BYTES_PRINTER_INIT;
		uint8_t *flush_start, *iter, *end;
		/* Start by inserting the initial, unconditional indentation at the start. */
		if (bytes_printer_append(&printer, filler.n_data, filler.n_size) < 0)
			goto err_printer;
		iter        = DeeBytes_DATA(self);
		end         = iter + DeeBytes_SIZE(self);
		flush_start = iter;
		while (iter < end) {
			uint8_t ch = *iter;
			if (DeeUni_IsLF(ch)) {
				++iter;
				/* Deal with windows-style linefeeds. */
				if (ch == ASCII_CR && *iter == ASCII_LF)
					++iter;
				/* Flush all unwritten data up to this point. */
				if (bytes_printer_append(&printer, flush_start,
				                         (size_t)(iter - flush_start)) < 0)
					goto err_printer;
				flush_start = iter;
				/* Insert the filler just before the linefeed. */
				if (bytes_printer_append(&printer, filler.n_data, filler.n_size) < 0)
					goto err_printer;
				continue;
			}
			++iter;
		}
		if (iter == flush_start) {
			/* Either the string is empty or ends with a line-feed.
			 * In either case, we must remove `filler' from its end,
			 * because we're not supposed to have the resulting
			 * string include it as trailing memory. */
			ASSERT(BYTES_PRINTER_SIZE(&printer) >= filler.n_size);
			bytes_printer_release(&printer, filler.n_size);
		} else {
			/* Flush the remainder. */
			if (bytes_printer_append(&printer, flush_start,
			                         (size_t)(iter - flush_start)) < 0)
				goto err_printer;
		}
		return bytes_printer_pack(&printer);
err_printer:
		bytes_printer_fini(&printer);
	}
err:
	return NULL;
retself:
	return_reference_((DeeObject *)self);
}

PRIVATE DREF DeeObject *DCALL
bytes_dedent(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	size_t max_chars   = 1;
	DeeObject *mask_ob = NULL;
	if (DeeArg_Unpack(argc, argv, "|Iuo:dedent", &max_chars, &mask_ob))
		goto err;
	if unlikely(!max_chars)
		goto retself;
	{
		struct bytes_printer printer = BYTES_PRINTER_INIT;
		uint8_t *flush_start, *iter, *end;
		size_t i;
		iter = DeeBytes_DATA(self);
		end  = iter + DeeBytes_SIZE(self);
		if (mask_ob) {
			Needle mask;
			if (get_needle(&mask, mask_ob))
				goto err_printer;

			/* Remove leading characters. */
			for (i = 0; i < max_chars && memchr(mask.n_data, *iter, mask.n_size); ++i)
				++iter;
			flush_start = iter;
			while (iter < end) {
				uint8_t ch = *iter;
				if (DeeUni_IsLF(ch)) {
					++iter;
					if (ch == UNICODE_CR && *iter == UNICODE_LF)
						++iter;
					/* Flush all unwritten data up to this point. */
					if (bytes_printer_append(&printer, flush_start,
					                         (size_t)(iter - flush_start)) < 0)
						goto err;
					/* Skip up to `max_chars' characters after a linefeed. */
					for (i = 0; i < max_chars && memchr(mask.n_data, *iter, mask.n_size); ++i)
						++iter;
					flush_start = iter;
					continue;
				}
				++iter;
			}
			/* Flush the remainder. */
			if (bytes_printer_append(&printer, flush_start,
			                         (size_t)(iter - flush_start)) < 0)
				goto err;
		} else {
			/* Remove leading characters. */
			for (i = 0; i < max_chars && DeeUni_IsSpace(*iter); ++i)
				++iter;
			flush_start = iter;
			while (iter < end) {
				uint8_t ch = *iter;
				if (DeeUni_IsLF(ch)) {
					++iter;
					if (ch == ASCII_CR && *iter == ASCII_LF)
						++iter;
					/* Flush all unwritten data up to this point. */
					if (bytes_printer_append(&printer, flush_start,
					                         (size_t)(iter - flush_start)) < 0)
						goto err;
					/* Skip up to `max_chars' characters after a linefeed. */
					for (i = 0; i < max_chars && DeeUni_IsSpace(*iter); ++i)
						++iter;
					flush_start = iter;
					continue;
				}
				++iter;
			}
			/* Flush the remainder. */
			if (bytes_printer_append(&printer, flush_start,
			                         (size_t)(iter - flush_start)) < 0)
				goto err;
		}
		return bytes_printer_pack(&printer);
err_printer:
		bytes_printer_fini(&printer);
	}
err:
	return NULL;
retself:
	return_reference_((DeeObject *)self);
}

PRIVATE DREF DeeObject *DCALL
bytes_common(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	size_t result = 0;
	if (get_bcompare_args(self, &args, argc, argv, "common"))
		goto err;
	if (args.lhs_len > args.rhs_len)
		args.lhs_len = args.rhs_len;
	while (result < args.lhs_len) {
		if (args.lhs_ptr[result] != args.rhs_ptr[result])
			break;
		++result;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rcommon(Bytes *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	size_t result = 0;
	if (get_bcompare_args(self, &args, argc, argv, "rcommon"))
		goto err;
	args.lhs_ptr += args.lhs_len;
	args.rhs_ptr += args.rhs_len;
	if (args.lhs_len > args.rhs_len)
		args.lhs_len = args.rhs_len;
	while (result < args.lhs_len) {
		if (args.lhs_ptr[-1] != args.rhs_ptr[-1])
			break;
		++result;
		--args.lhs_ptr;
		--args.rhs_ptr;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casecommon(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	size_t result = 0;
	if (get_bcompare_args(self, &args, argc, argv, "casecommon"))
		goto err;
	if (args.lhs_len > args.rhs_len)
		args.lhs_len = args.rhs_len;
	while (result < args.lhs_len) {
		uint8_t a = args.lhs_ptr[result];
		uint8_t b = args.rhs_ptr[result];
		if (a != b) {
			a = (uint8_t)DeeUni_IsLower(a);
			b = (uint8_t)DeeUni_IsLower(b);
			if (a != b)
				break;
		}
		++result;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casercommon(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	struct bcompare_args args;
	size_t result = 0;
	if (get_bcompare_args(self, &args, argc, argv, "casercommon"))
		goto err;
	args.lhs_ptr += args.lhs_len;
	args.rhs_ptr += args.rhs_len;
	if (args.lhs_len > args.rhs_len)
		args.lhs_len = args.rhs_len;
	while (result < args.lhs_len) {
		uint8_t a = args.lhs_ptr[-1];
		uint8_t b = args.rhs_ptr[-1];
		if (a != b) {
			a = (uint8_t)DeeUni_IsLower(a);
			b = (uint8_t)DeeUni_IsLower(b);
			if (a != b)
				break;
		}
		++result;
		--args.lhs_ptr;
		--args.rhs_ptr;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
bytes_findmatch(Bytes *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:findmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto err_not_found; /* Empty search area. */
	scan_len = end - start;
	scan_str = DeeBytes_DATA(self);
	ptr = find_matchb(scan_str + start, scan_len,
	                  s_open.n_data, s_open.n_size,
	                  s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto err_not_found;
	return DeeInt_NewSize((size_t)(ptr - scan_str));
err_not_found:
	return_reference_(&DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rfindmatch(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:rfindmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto err_not_found; /* Empty search area. */
	scan_len = end - start;
	scan_str = DeeBytes_DATA(self);
	ptr = rfind_matchb(scan_str + start, scan_len,
	                   s_open.n_data, s_open.n_size,
	                   s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto err_not_found;
	return DeeInt_NewSize((size_t)(ptr - scan_str));
err_not_found:
	return_reference_(&DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_indexmatch(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:indexmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto err_not_found; /* Empty search area. */
	scan_len = end - start;
	scan_str = DeeBytes_DATA(self);
	ptr = find_matchb(scan_str + start, scan_len,
	                  s_open.n_data, s_open.n_size,
	                  s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto err_not_found;
	return DeeInt_NewSize((size_t)(ptr - scan_str));
err_not_found:
	err_index_not_found((DeeObject *)self, s_clos_ob);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rindexmatch(Bytes *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:rindexmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto err_not_found; /* Empty search area. */
	scan_len = end - start;
	scan_str = DeeBytes_DATA(self);
	ptr = rfind_matchb(scan_str + start, scan_len,
	                   s_open.n_data, s_open.n_size,
	                   s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto err_not_found;
	return DeeInt_NewSize((size_t)(ptr - scan_str));
err_not_found:
	err_index_not_found((DeeObject *)self, s_open_ob);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casefindmatch(Bytes *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:casefindmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto err_not_found; /* Empty search area. */
	scan_len = end - start;
	scan_str = DeeBytes_DATA(self);
	ptr = find_asciicasematchb(scan_str + start, scan_len,
	                           s_open.n_data, s_open.n_size,
	                           s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto err_not_found;
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     (size_t)(ptr - scan_str),
	                     s_clos.n_size);
err_not_found:
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caserfindmatch(Bytes *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:caserfindmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto err_not_found; /* Empty search area. */
	scan_len = end - start;
	scan_str = DeeBytes_DATA(self);
	ptr = rfind_asciicasematchb(scan_str + start, scan_len,
	                            s_open.n_data, s_open.n_size,
	                            s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto err_not_found;
	result = (size_t)(ptr - scan_str);
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + s_open.n_size);
err_not_found:
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caseindexmatch(Bytes *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:caseindexmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto err_not_found; /* Empty search area. */
	scan_len = end - start;
	scan_str = DeeBytes_DATA(self);
	ptr = find_asciicasematchb(scan_str + start, scan_len,
	                           s_open.n_data, s_open.n_size,
	                           s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto err_not_found;
	result = (size_t)(ptr - scan_str);
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + s_clos.n_size);
err_not_found:
	err_index_not_found((DeeObject *)self, s_clos_ob);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caserindexmatch(Bytes *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:caserindexmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto err_not_found; /* Empty search area. */
	scan_len = end - start;
	scan_str = DeeBytes_DATA(self);
	ptr = rfind_asciicasematchb(scan_str + start, scan_len,
	                            s_open.n_data, s_open.n_size,
	                            s_clos.n_data, s_clos.n_size);
	if unlikely(!ptr)
		goto err_not_found;
	result = (size_t)(ptr - scan_str);
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + s_open.n_size);
err_not_found:
	err_index_not_found((DeeObject *)self, s_open_ob);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_partitionmatch(Bytes *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
	DREF DeeTupleObject *result;
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:partitionmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
#define SET_BYTES(a, b, c)                                       \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	} __WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_str = DeeBytes_DATA(self);
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto match_not_found; /* Empty search area. */
	scan_len    = end - start;
	match_start = memmemb(scan_str + start, scan_len,
	                      s_open.n_data, s_open.n_size);
	if unlikely(!match_start)
		goto match_not_found;
	match_end = find_matchb(match_start + s_open.n_size, scan_len - (match_start - (scan_str + start)),
	                        s_open.n_data, s_open.n_size,
	                        s_clos.n_data, s_clos.n_size);
	if unlikely(!match_end)
		goto match_not_found;
	SET_BYTES(DeeBytes_NewSubView(self, scan_str,
	                              (size_t)(match_start - scan_str)),
	          DeeBytes_NewSubView(self, match_start,
	                              (match_end + s_clos.n_size) -
	                              match_start),
	          DeeBytes_NewSubView(self, match_end + s_clos.n_size,
	                              (size_t)(scan_str + end) -
	                              (size_t)(match_end + s_clos.n_size)));
#undef SET_BYTES
done:
	return (DREF DeeObject *)result;
match_not_found:
	result->t_elem[0] = (DREF DeeObject *)bytes_getsubstr(self, start, end);
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyBytes;
	result->t_elem[2] = Dee_EmptyBytes;
	Dee_Incref_n(Dee_EmptyBytes, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_rpartitionmatch(Bytes *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	DREF DeeTupleObject *result;
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:rpartitionmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
#define SET_BYTES(a, b, c)                                       \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	} __WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_str = DeeBytes_DATA(self);
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto match_not_found; /* Empty search area. */
	scan_len  = end - start;
	match_end = memrmemb(scan_str + start, scan_len,
	                     s_clos.n_data, s_clos.n_size);
	if unlikely(!match_end)
		goto match_not_found;
	match_start = rfind_matchb(scan_str + start,
	                           (size_t)(match_end - (scan_str + start)),
	                           s_open.n_data, s_open.n_size,
	                           s_clos.n_data, s_clos.n_size);
	if unlikely(!match_start)
		goto match_not_found;
	SET_BYTES(DeeBytes_NewSubView(self, scan_str,
	                              (size_t)(match_start - scan_str)),
	          DeeBytes_NewSubView(self, match_start,
	                              (size_t)((match_end + s_clos.n_size) -
	                                       match_start)),
	          DeeBytes_NewSubView(self, match_end + s_clos.n_size,
	                              (size_t)(scan_str + end) -
	                              (size_t)(match_end + s_clos.n_size)));
#undef SET_BYTES
done:
	return (DREF DeeObject *)result;
match_not_found:
	result->t_elem[0] = (DREF DeeObject *)bytes_getsubstr(self, start, end);
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyString;
	result->t_elem[2] = Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_casepartitionmatch(Bytes *__restrict self,
                         size_t argc, DeeObject **__restrict argv) {
	DREF DeeTupleObject *result;
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:casepartitionmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
#define SET_BYTES(a, b, c)                                       \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	} __WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_str = DeeBytes_DATA(self);
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto match_not_found; /* Empty search area. */
	scan_len    = end - start;
	match_start = dee_memasciicasemem(scan_str + start, scan_len,
	                                  s_open.n_data, s_open.n_size);
	if unlikely(!match_start)
		goto match_not_found;
	match_end = find_asciicasematchb(match_start + s_open.n_size, scan_len - (match_start - (scan_str + start)),
	                                 s_open.n_data, s_open.n_size,
	                                 s_clos.n_data, s_clos.n_size);
	if unlikely(!match_end)
		goto match_not_found;
	SET_BYTES(DeeBytes_NewSubView(self, scan_str,
	                              (size_t)(match_start - scan_str)),
	          DeeBytes_NewSubView(self, match_start,
	                              (match_end + s_clos.n_size) -
	                              match_start),
	          DeeBytes_NewSubView(self, match_end + s_clos.n_size,
	                              (size_t)(scan_str + end) -
	                              (size_t)(match_end + s_clos.n_size)));
#undef SET_BYTES
done:
	return (DREF DeeObject *)result;
match_not_found:
	result->t_elem[0] = (DREF DeeObject *)bytes_getsubstr(self, start, end);
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyBytes;
	result->t_elem[2] = Dee_EmptyBytes;
	Dee_Incref_n(Dee_EmptyBytes, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_caserpartitionmatch(Bytes *__restrict self,
                          size_t argc, DeeObject **__restrict argv) {
	DREF DeeTupleObject *result;
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:caserpartitionmatch", &s_open_ob, &s_clos_ob, &start, &end) ||
	    get_needle(&s_open, s_open_ob) || get_needle(&s_clos, s_clos_ob))
		goto err;
#define SET_BYTES(a, b, c)                                       \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	} __WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_str = DeeBytes_DATA(self);
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto match_not_found; /* Empty search area. */
	scan_len  = end - start;
	match_end = dee_memasciicasermem(scan_str + start, scan_len,
	                                 s_clos.n_data, s_clos.n_size);
	if unlikely(!match_end)
		goto match_not_found;
	match_start = rfind_asciicasematchb(scan_str + start,
	                                    (size_t)(match_end - (scan_str + start)),
	                                    s_open.n_data, s_open.n_size,
	                                    s_clos.n_data, s_clos.n_size);
	if unlikely(!match_start)
		goto match_not_found;
	SET_BYTES(DeeBytes_NewSubView(self, scan_str,
	                              (size_t)(match_start - scan_str)),
	          DeeBytes_NewSubView(self, match_start,
	                              (size_t)((match_end + s_clos.n_size) -
	                                       match_start)),
	          DeeBytes_NewSubView(self, match_end + s_clos.n_size,
	                              (size_t)(scan_str + end) -
	                              (size_t)(match_end + s_clos.n_size)));
#undef SET_BYTES
done:
	return (DREF DeeObject *)result;
match_not_found:
	result->t_elem[0] = (DREF DeeObject *)bytes_getsubstr(self, start, end);
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyString;
	result->t_elem[2] = Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
DeeBytes_Segments(DeeBytesObject *__restrict self, size_t substring_length);

PRIVATE DREF DeeObject *DCALL
bytes_segments(Bytes *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	size_t substring_length;
	if (DeeArg_Unpack(argc, argv, "Iu:segments", &substring_length))
		goto err;
	if unlikely(!substring_length) {
		err_invalid_segment_size(substring_length);
		goto err;
	}
	return DeeBytes_Segments(self, substring_length);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
bytes_distribute(Bytes *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	size_t substring_count;
	size_t substring_length;
	if (DeeArg_Unpack(argc, argv, "Iu:distribute", &substring_count))
		goto err;
	if unlikely(!substring_count) {
		err_invalid_distribution_count(substring_count);
		goto err;
	}
	substring_length = DeeBytes_SIZE(self);
	substring_length += substring_count - 1;
	substring_length /= substring_count;
	if unlikely(!substring_length)
		return_empty_seq;
	return DeeBytes_Segments(self, substring_length);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
bytes_sizeof(Bytes *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	size_t result;
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	result = offsetof(Bytes, b_data);
	if (self->b_buffer.bb_base == self->b_data)
		result += self->b_buffer.bb_size;
	return DeeInt_NewSize(result);
err:
	return NULL;
}



INTERN struct type_method bytes_methods[] = {
	{ "decode",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&string_decode,
	  DOC("(codec:?Dstring,errors=!Pstrict)->?X2?Dstring?O\n"
	      "@throw ValueError The given @codec or @errors wasn't recognized\n"
	      "@throw UnicodeDecodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
	      "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
	      "Same as :string.decode, but instead use the data of @this Bytes object as characters to decode"),
	  TYPE_METHOD_FKWDS },
	{ "encode",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&string_encode,
	  DOC("(codec:?Dstring,errors=!Pstrict)->?X3?.?Dstring?O\n"
	      "@throw ValueError The given @codec or @errors wasn't recognized\n"
	      "@throw UnicodeEncodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
	      "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
	      "Same as :string.encode, but instead use the data of @this Bytes object as characters to decode"),
	  TYPE_METHOD_FKWDS },
	{ "bytes",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_substr,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Same as #substr (here for ABI compatibility with :string.bytes)"),
	  TYPE_METHOD_FKWDS },
	{ "ord",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_ord,
	  DOC("->?Dint\n"
	      "@throw ValueError The length of @this Bytes object is not equal to $1\n"
	      "Same as ${this[0]}\n"
	      "\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw IntegerOverflow The given @index is lower than $0\n"
	      "@throw IndexError The given @index is greater than ${#this}\n"
	      "Same as ${this[index]}") },

	/* Bytes-specific functions. */
	{ "resized",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_resized,
	  DOC("(new_size:?Dint,filler?:?Dint)->?.\n"
	      "Return a new writable Bytes object with a length of @new_size, and its "
	      "first ${(#this,new_size) < ...} bytes initialized from ${this.substr(0,new_size)}, "
	      "with the remainder then either left uninitialized, or initialized to @filler\n"
	      "Note that because a Bytes object cannot be resized in-line, code using this function "
	      "must make use of the returned Bytes object:\n"
	      ">local x = \"foobar\";\n"
	      ">local y = x.bytes();\n"
	      ">print repr y; /* \"foobar\" */\n"
	      ">y = y.resized(16,\"?\".ord());\n"
	      ">print repr y; /* \"foobar??"
	      "??"
	      "??"
	      "??"
	      "??\" */") },
	{ "reverse",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_reverse,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Same as #reversed, but modifications are performed "
	      "in-line, before @this Bytes object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "makereadonly",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_makereadonly,
	  DOC("->?.\n"
	      "The inverse of #makewritable, either re-returning @this Bytes object if it "
	      "already is read-only, or construct a view for the data contained within @this "
	      "Bytes object, but making that view read-only") },
	{ "makewritable",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_makewritable,
	  DOC("->?.\n"
	      "Either re-return @this Bytes object is it already #iswritable, or create a "
	      "copy (s.a. #op:copy) and return it:\n"
	      ">function makewritable() {\n"
	      "> if (this.iswritable)\n"
	      ">  return this;\n"
	      "> return copy this;\n"
	      ">}") },
	{ "hex",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_hex,
	  DOC("(start=!0,end=!-1)->?Dstring\n"
	      "Returns a hex-encoded string for the bytes contained within "
	      "${this.substr(start,end)}, that is a string containing 2 characters "
	      "for each encoded byte, both of which are lower-case hexadecimal digits\n"
	      "The returned string is formatted such that #fromhex can be used to decode "
	      "it into another Bytes object"),
	  TYPE_METHOD_FKWDS },

	/* Bytes formatting / scanning. */
	{ "format",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_format,
	  DOC("(args:?S?O)->?.") },
	{ "scanf",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_scanf,
	  DOC("(format:?Dstring)->?S?O") },

	/* String/Character traits */
	{ "isprint",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isprint,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all "
	      "characters in ${this.substr(start,end)} are printable") },
	{ "isalpha",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isalpha,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are alphabetical") },
	{ "isspace",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isspace,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are space-characters") },
	{ "islf",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_islf,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are line-feeds") },
	{ "islower",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_islower,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are lower-case") },
	{ "isupper",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isupper,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are upper-case") },
	{ "iscntrl",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_iscntrl,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are control characters") },
	{ "isdigit",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isdigit,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are digits") },
	{ "isdecimal",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isdecimal,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are dicimal characters") },
	{ "issymstrt",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_issymstrt,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} can be used to start a symbol name") },
	{ "issymcont",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_issymcont,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} can be used to continue a symbol name") },
	{ "isalnum",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isalnum,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} are alpha-numerical") },
	{ "isnumeric",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isnumeric,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all bytes (when interpreted as ASCII characters) "
	      "in ${this.substr(start,end)} qualify as digit or decimal characters") },
	{ "istitle",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_istitle,
	  DOC("(index:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if the character at ${this[index]} has title-casing\n"
	      "\n"
	      "->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "Returns :true if $this, or the sub-string ${this.substr(start,end)} "
	      "follows title-casing, meaning that space is followed by upper-case") },
	{ "issymbol",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_issymbol,
	  DOC("(index:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if the character at ${this[index]} can be used "
	      "to start a symbol name. Same as ${this.issymstrt(index)}\n"
	      "\n"
	      "->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "Returns :true if $this, or the sub-string ${this.substr(start,end)} "
	      "is a valid symbol name") },
	{ "isascii",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isascii,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters in ${this.substr(start,end)} "
	      "are ascii-characters, that is have an ordinal value ${<= 0x7f}") },
	{ "asnumber",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_asnumber,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Return the numeric value of the @index'th or only character of @this string, "
	      "or throw a :ValueError or return @defl if that character isn't #isnumeric") },
	{ "asdigit",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_asdigit,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Same as #asnumber, but only succeed if the selected character matches #isdigit, rather than #isnumeric") },
	{ "asdecimal",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_asdecimal,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Same as #asnumber, but only succeed if the selected character matches #isdecimal, rather than #isnumeric") },

	{ "isanyprint",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanyprint,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is printable"),
	  TYPE_METHOD_FKWDS },
	{ "isanyalpha",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanyalpha,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is alphabetical"),
	  TYPE_METHOD_FKWDS },
	{ "isanyspace",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanyspace,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a space character"),
	  TYPE_METHOD_FKWDS },
	{ "isanylf",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanylf,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a line-feeds"),
	  TYPE_METHOD_FKWDS },
	{ "isanylower",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanylower,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is lower-case"),
	  TYPE_METHOD_FKWDS },
	{ "isanyupper",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanyupper,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is upper-case"),
	  TYPE_METHOD_FKWDS },
	{ "isanycntrl",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanycntrl,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a control character"),
	  TYPE_METHOD_FKWDS },
	{ "isanydigit",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanydigit,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a digit"),
	  TYPE_METHOD_FKWDS },
	{ "isanydecimal",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanydecimal,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a dicimal character"),
	  TYPE_METHOD_FKWDS },
	{ "isanysymstrt",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanysymstrt,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} can be used to start a symbol name"),
	  TYPE_METHOD_FKWDS },
	{ "isanysymcont",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanysymcont,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} can be used to continue a symbol name"),
	  TYPE_METHOD_FKWDS },
	{ "isanyalnum",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanyalnum,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is alpha-numerical"),
	  TYPE_METHOD_FKWDS },
	{ "isanynumeric",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanynumeric,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} qualifies as digit or decimal characters"),
	  TYPE_METHOD_FKWDS },
	{ "isanytitle",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanytitle,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} has title-casing"),
	  TYPE_METHOD_FKWDS },
	{ "isanyascii",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_isanyascii,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in ${this.substr(start,end)} are "
	      "ascii-characters, that is have an ordinal value ${<= 0x7f}"),
	  TYPE_METHOD_FKWDS },

	/* Bytes conversion functions */
	{ "lower",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_lower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this Bytes object converted to lower-case (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "upper",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_upper,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this Bytes object converted to upper-case (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "title",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_title,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this Bytes object converted to title-casing (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "capitalize",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_capitalize,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this Bytes object with each word capitalized (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "swapcase",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_swapcase,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this Bytes object with the casing of each "
	      "character that has two different casings swapped (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "casefold",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_lower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Alias for #{lower}. This function exists to match :string.casefold in "
	      "order to improve binary compatibility between :Bytes and :string objects"),
	  TYPE_METHOD_FKWDS },

	/* Inplace variants of bytes conversion functions */
	{ "tolower",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_tolower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Same as #lower, but character modifications are performed in-place, and @this Bytes object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "toupper",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_toupper,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Same as #upper, but character modifications are performed in-place, and @this Bytes object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "totitle",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_totitle,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Same as #title, but character modifications are performed in-place, and @this Bytes object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "tocapitalize",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_tocapitalize,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Same as #capitalize, but character modifications are performed in-place, and @this Bytes object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "toswapcase",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_toswapcase,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Same as #swapcase, but character modifications are performed in-place, and @this Bytes object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "tocasefold",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_tolower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Alias for #tolower, here to coincide with #casefold existing as an alias for #lower"),
	  TYPE_METHOD_FKWDS },

	/* Case-sensitive query functions */
	{ DeeString_STR(&str_replace),
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_replace,
	  DOC("(find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@throw ValueError The given @find or @replace is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @find or @replace is an integer lower than $0, or greater than $0xff\n"
	      "Find up to @max occurrances of @find and replace each with @replace, then return the resulting data as a writable Bytes object"),
	  TYPE_METHOD_FKWDS },
	{ "toreplace",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_toreplace,
	  DOC("(find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@throw ValueError The given @find or @replace is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @find or @replace is an integer lower than $0, or greater than $0xff\n"
	      "@throw ValueError The number of bytes specified by @find and @replace are not identical\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Same as #replace, but the Bytes object is modified in-place, and @this is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "find",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_find,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Find the first instance of @needle that exists within ${this.substr(start,end)}, "
	      "and return its starting index, or ${-1} if no such position exists"),
	  TYPE_METHOD_FKWDS },
	{ "rfind",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rfind,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Find the first instance of @needle that exists within ${this.substr(start,end)}, "
	      "and return its starting index, or ${-1} if no such position exists"),
	  TYPE_METHOD_FKWDS },
	{ "index",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_index,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
	      "Find the first instance of @needle that exists within ${this.substr(start,end)}, "
	      "and return its starting index"),
	  TYPE_METHOD_FKWDS },
	{ "rindex",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rindex,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
	      "Find the last instance of @needle that exists within ${this.substr(start,end)}, "
	      "and return its starting index"),
	  TYPE_METHOD_FKWDS },
	{ "findall",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_findall,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?S?Dint\n"
	      "Find all instances of @needle within ${this.substr(start,end)}, "
	      "and return their starting indeces as a sequence"),
	  TYPE_METHOD_FKWDS },
	{ "count",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_count,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "Count the number of instances of @needle that exists within ${this.substr(start,end)}, "
	      "and return now many were found"),
	  TYPE_METHOD_FKWDS },
	{ "contains",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_contains_f,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Check if @needle can be found within ${this.substr(start,end)}, and return a boolean indicative of that"),
	  TYPE_METHOD_FKWDS },
	{ "substr",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_substr,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Similar to ${this[start:end]}, and semantically equialent to :string.substr\n"
	      "This function can be used to view a sub-set of bytes from @this Bytes object\n"
	      "Modifications then made to the returned Bytes object will affect the same memory already described by @this Bytes object"),
	  TYPE_METHOD_FKWDS },
	{ "strip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_strip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all leading and trailing whitespace-characters, or "
	      "characters apart of @mask, and return a sub-view of @this Bytes object") },
	{ "lstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_lstrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all leading whitespace-characters, or characters "
	      "apart of @mask, and return a sub-view of @this Bytes object") },
	{ "rstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rstrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all trailing whitespace-characters, or characters "
	      "apart of @mask, and return a sub-view of @this Bytes object") },
	{ "sstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_sstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all leading and trailing instances of @other from @this string\n"
	      ">local result = this;\n"
	      ">while (result.startswith(other))\n"
	      "> result = result[#other:];\n"
	      ">while (result.endswith(other))\n"
	      "> result = result[:#result-#other];") },
	{ "lsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_lsstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all leading instances of @other from @this string\n"
	      ">local result = this;\n"
	      ">while (result.startswith(other))\n"
	      "> result = result[#other:];") },
	{ "rsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rsstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all trailing instances of @other from @this string\n"
	      ">local result = this;\n"
	      ">while (result.endswith(other))\n"
	      "> result = result[:#result-#other];") },
	{ "startswith",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_startswith,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Return :true if the sub-string ${this.substr(start,end)} starts with @other"),
	  TYPE_METHOD_FKWDS },
	{ "endswith",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_endswith,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Return :true if the sub-string ${this.substr(start,end)} ends with @other"),
	  TYPE_METHOD_FKWDS },
	{ "partition",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_parition,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Search for the first instance of @needle within ${this.substr(start,end)} and "
	      "return a 3-element sequence of byte objects ${(this[:pos],needle,this[pos+#needle:])}.\n"
	      "If @needle could not be found, ${(this,\"\".bytes(),\"\".bytes())} is returned"),
	  TYPE_METHOD_FKWDS },
	{ "rpartition",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rparition,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Search for the last instance of @needle within ${this.substr(start,end)} and "
	      "return a 3-element sequence of strings ${(this[:pos],needle,this[pos+#needle:])}.\n"
	      "If @needle could not be found, ${(this,\"\".bytes(),\"\".bytes())} is returned"),
	  TYPE_METHOD_FKWDS },
	{ "compare",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_compare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Compare the sub-string ${left = this.substr(my_start,my_end)} with ${right = other.substr(other_start,other_end)}, "
	      "returning ${< 0} if ${left < right}, ${> 0} if ${left > right}, or ${== 0} if they are equal") },
	{ "vercompare",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_vercompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Performs a version-string comparison. This is similar to #compare, but rather than "
	      "performing a strict lexicographical comparison, the numbers found in the strings "
	      "being compared are comparsed as a whole, solving the common problem seen in applications "
	      "such as file navigators showing a file order of `foo1.txt', `foo10.txt', `foo11.txt', `foo2.txt', etc...\n"
	      "This function is a portable implementation of the GNU function "
	      "%{link https://linux.die.net/man/3/strverscmp strverscmp}, "
	      "for which you may follow the link for further details") },
	{ "wildcompare",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_wildcompare,
	  DOC("(pattern:?X2?.?Dstring,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "(my_start:?Dint,pattern:?X2?.?Dstring,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?X2?.?Dstring,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "Perform a wild-character-enabled comparising of the sub-string ${left = this.substr(my_start,my_end)} "
	      "with ${right = pattern.substr(pattern_start,pattern_end)}, returning ${< 0} if ${left < right}, ${> 0} "
	      "if ${left > right}, or ${== 0} if they are equal\n"
	      "Wild-compare characters are only parsed from @pattern, allowing $\"?\" to "
	      "be matched with any single character from @this, and $\"*\" to be matched to "
	      "any number of characters") },
	{ "fuzzycompare",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_fuzzycompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Perform a fuzzy string comparison between ${this.substr(my_start,my_end)} and ${other.substr(other_start,other_end)}\n"
	      "The return value is a similarty-factor that can be used to score how close the two strings look alike.\n"
	      "How exactly the scoring is done is implementation-specific, however a score of $0 is reserved for two "
	      "strings that are perfectly identical, any two differing strings always have a score ${> 0}, and the closer "
	      "the score is to $0, the more alike they are\n"
	      "The intended use of this function is for auto-completion, as well as warning "
	      "messages and recommendations in the sense of I-dont-know-foo-but-did-you-mean-bar\n"
	      "Note that there is another version #casefuzzycompare that also ignores casing") },
	{ "wmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_wmatch,
	  DOC("(pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "Same as #wildcompare, returning :true where #wildcompare would return $0, and :false in all pattern cases") },

	/* Case-insensitive query functions */
	{ "casereplace",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casereplace,
	  DOC("(find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@throw ValueError The given @find or @replace is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @find or @replace is an integer lower than $0, or greater than $0xff\n"
	      "Same as #replace, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "tocasereplace",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_tocasereplace,
	  DOC("(find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@throw ValueError The given @find or @replace is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @find or @replace is an integer lower than $0, or greater than $0xff\n"
	      "@throw ValueError The number of bytes specified by @find and @replace are not identical\n"
	      "@throw BufferError @this Bytes object is not writable\n"
	      "Same as #toreplace, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casefind",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casefind,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Same as #find, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #needle}\n"
	      "If no match if found, :none is returned"),
	  TYPE_METHOD_FKWDS },
	{ "caserfind",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caserfind,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Same as #rfind, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #needle}\n"
	      "If no match if found, :none is returned"),
	  TYPE_METHOD_FKWDS },
	{ "caseindex",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caseindex,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
	      "Same as #index, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #needle}"),
	  TYPE_METHOD_FKWDS },
	{ "caserindex",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caserindex,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
	      "Same as #rindex, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #needle}"),
	  TYPE_METHOD_FKWDS },
	{ "casefindall",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casefindall,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?S?T2?Dint?Dint\n"
	      "Same as #findall, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #needle}"),
	  TYPE_METHOD_FKWDS },
	{ "casecount",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casecount,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "Same as #count, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casecontains",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casecontains_f,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Same as #contains, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casestrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casestrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as #strip, however ascii-casing is ignored during character comparisons") },
	{ "caselstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caselstrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as #lstrip, however ascii-casing is ignored during character comparisons") },
	{ "caserstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caserstrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as #rstrip, however ascii-casing is ignored during character comparisons") },
	{ "casesstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casesstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Same as #sstrip, however ascii-casing is ignored during character comparisons") },
	{ "caselsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caselsstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Same as #lsstrip, however ascii-casing is ignored during character comparisons") },
	{ "casersstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casersstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Same as #rsstrip, however ascii-casing is ignored during character comparisons") },
	{ "casestartswith",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casestartswith,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Same as #startswith, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "caseendswith",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caseendswith,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Same as #endswith, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casepartition",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caseparition,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as #partition, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "caserpartition",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caserparition,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as #rpartition, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casecompare",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casecompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #compare, however ascii-casing is ignored during character comparisons") },
	{ "casevercompare",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casevercompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #vercompare, however ascii-casing is ignored during character comparisons") },
	{ "casewildcompare",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casewildcompare,
	  DOC("(pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #wildcompare, however ascii-casing is ignored during character comparisons") },
	{ "casefuzzycompare",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casefuzzycompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #fuzzycompare, however ascii-casing is ignored during character comparisons") },
	{ "casewmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casewmatch,
	  DOC("(pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "Same as #casewmatch, however ascii-casing is ignored during character comparisons") },

	/* Bytes alignment functions. */
	{ "center",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_center,
	  DOC("(width:?Dint,filler:?X3?.?Dstring?Dint=!P{ })->?.\n"
	      "Use a writable copy of @this Bytes object as result, then evenly "
	      "insert @filler at the front and back to pad its length to @width bytes") },
	{ "ljust",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_ljust,
	  DOC("(width:?Dint,filler=!P{ })->?.\n"
	      "Use a writable copy of @this Bytes object as result, then "
	      "insert @filler at the back to pad its length to @width bytes") },
	{ "rjust",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rjust,
	  DOC("(width:?Dint,filler=!P{ })->?.\n"
	      "Use a writable copy of @this Bytes object as result, then "
	      "insert @filler at the front to pad its length to @width bytes") },
	{ "zfill",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_zfill,
	  DOC("(width:?Dint,filler=!P{0})->?.\n"
	      "Skip leading ${\'+\'} and ${\'-\'} ascii-characters, then insert @filler "
	      "to pad the resulting string to a length of @width bytes") },
	{ "reversed",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_reversed,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Return a copy of the sub-string ${this.substr(start,end)} with its byte order reversed"),
	  TYPE_METHOD_FKWDS },
	{ "expandtabs",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_expandtabs,
	  DOC("(tabwidth=!8)->?.\n"
	      "Expand tab characters with whitespace offset from the "
	      "start of their respective line at multiples of @tabwidth\n"
	      "Note that in the event of no tabs being found, @this Bytes object may be re-returned") },
	{ "unifylines",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_unifylines,
	  DOC("(replacement:?X3?.?Dstring?Dint=!P{\\\n})->?.\n"
	      "Unify all ascii-linefeed character sequences ($\"\\n\", $\"\\r\" and $\"\\r\\n\") "
	      "found in @this Bytes object to make exclusive use of @replacement\n"
	      "Note that in the event of no line-feeds differing from @replacement being found, "
	      "@this Bytes object may be re-returned") },

	/* Bytes splitter functions. */
	{ "join",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_join,
	  DOC("(seq:?S?O)->?.\n"
	      "Iterate @seq and convert all items into string, inserting @this "
	      "Bytes object before each string's :string.bytes representation element, "
	      "starting only with the second. :Bytes objects contained in @seq are not "
	      "converted to and from strings, but inserted directly") },
	{ "split",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_split,
	  DOC("(needle:?X3?.?Dstring?Dint)->?S?.\n"
	      "Split @this Bytes object at each instance of @sep, "
	      "returning a sequence of the resulting parts\n"
	      "The returned bytes objects are views of @this byte object, meaning they "
	      "have the same #iswritable characteristics as @this, and refer to the same "
	      "memory") },
	{ "casesplit",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casesplit,
	  DOC("(needle:?X3?.?Dstring?Dint)->?S?.\n"
	      "Same as #split, however ascii-casing is ignored during character comparisons\n"
	      "The returned bytes objects are views of @this byte object, meaning they "
	      "have the same #iswritable characteristics as @this, and refer to the same "
	      "memory") },
	{ "splitlines",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_splitlines,
	  DOC("(keepends=!f)->?S?.\n"
	      "Split @this Bytes object at each linefeed, returning a sequence of all contained lines\n"
	      "When @keepends is :false, this is identical to ${this.unifylines().split(\"\\n\")}\n"
	      "When @keepends is :true, items found in the returned sequence will still have their "
	      "original, trailing line-feed appended\n"
	      "This function recognizes $\"\\n\", $\"\\r\" and $\"\\r\\n\" as linefeed sequences\n"
	      "The returned bytes objects are views of @this byte object, meaning they "
	      "have the same #iswritable characteristics as @this, and refer to the same "
	      "memory") },

	/* String indentation. */
	{ "indent",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_indent,
	  DOC("(filler:?X3?.?Dstring?Dint=!P{\t})->?.\n"
	      "Using @this Bytes object as result, insert @filler at the front, as well as after "
	      "every ascii-linefeed with the exception of one that may be located at its end\n"
	      "The inteded use is for generating strings from structured data, such as HTML:\n"
	      ">text = \"<html>\n{}\n</html>\".format({ get_html_bytes().strip().indent() });") },
	{ "dedent",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_dedent,
	  DOC("(max_chars=!1,mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Using @this string as result, remove up to @max_chars whitespace "
	      "(s.a. #isspace) characters, or if given: characters apart of @mask "
	      "from the front, as well as following any linefeed") },

	/* Common-character search functions. */
	{ "common",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_common,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Returns the number of common leading bytes shared between @this and @other, "
	      "or in other words: the lowest index $i for which ${this[i] != other.bytes()[i]} is true") },
	{ "rcommon",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rcommon,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Returns the number of common trailing bytes shared between @this and @other") },
	{ "casecommon",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casecommon,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #common, however ascii-casing is ignored during character comparisons") },
	{ "casercommon",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casercommon,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #rcommon, however ascii-casing is ignored during character comparisons") },

	/* Find match character sequences */
	{ "findmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_findmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "Similar to #find, but do a recursive search for the "
	      "first @close that doesn't have a match @{open}\n"
	      "For more information, see :string.findmatch") },
	{ "indexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_indexmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @close without a match @open exists within ${this.substr(start,end)}\n"
	      "Same as #findmatch, but throw an :IndexError instead of "
	      "returning ${-1} if no @close without a match @open exists") },
	{ "casefindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casefindmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "Same as #findmatch, however casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #close}\n"
	      "If no match if found, :none is returned") },
	{ "caseindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caseindexmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @close without a match @open exists within ${this.substr(start,end)}\n"
	      "Same as #indexmatch, however casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #close}") },
	{ "rfindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rfindmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "Similar to #findmatch, but operate in a mirrored fashion, searching for the "
	      "last instance of @open that has no match @close within ${this.substr(start,end)}:\n"
	      ">s = \"get_string().foo(bar(),baz(42),7).length\";\n"
	      ">lcol = s.find(\")\");\n"
	      ">print lcol; /* 19 */\n"
	      ">mtch = s.rfindmatch(\"(\",\")\",0,lcol);\n"
	      ">print repr s[mtch:lcol+1]; /* \"(bar(),baz(42),7)\" */\n"
	      "If no @open without a match @close exists, ${-1} is returned") },
	{ "rindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rindexmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @open without a match @close exists within ${this.substr(start,end)}\n"
	      "Same as #rfindmatch, but throw an :IndexError instead of returning ${-1} if no @open without a match @close exists") },
	{ "caserfindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caserfindmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "Same as #rfindmatch, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #open}\n"
	      "If no match if found, :none is returned") },
	{ "caserindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caserindexmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @open without a match @close exists within ${this.substr(start,end)}\n"
	      "Same as #rindexmatch, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + #open}") },

	/* Using the find-match functionality, also provide a partitioning version */
	{ "partitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_partitionmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "A hybrid between #find, #findmatch and #partition that returns the strings surrounding "
	      "the matched string portion, the first being the substring prior to the match, "
	      "the second being the matched string itself (including the @open and @close strings), "
	      "and the third being the substring after the match\n"
	      "For more information see :string.partitionmatch") },
	{ "rpartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_rpartitionmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "A hybrid between #rfind, #rfindmatch and #rpartition that returns the strings surrounding "
	      "the matched string portion, the first being the substring prior to the match, "
	      "the second being the matched string itself (including the @open and @close strings), "
	      "and the third being the substring after the match.\n"
	      "For more information see :string.rpartitionmatch") },
	{ "casepartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_casepartitionmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as #partitionmatch, however casing is ignored during character comparisons") },
	{ "caserpartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_caserpartitionmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as #rpartitionmatch, however casing is ignored during character comparisons") },

	{ "segments",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_segments,
	  DOC("(substring_length:?Dint)->?S?.\n"
	      "Split @this Bytes object into segments, each exactly @substring_length characters long, with the "
	      "last segment containing the remaining characters and having a length of between "
	      "$1 and @substring_length characters.\n"
	      "This function is similar to #distribute, but instead of being given the "
	      "length of sub-strings and figuring out their amount, this function takes "
	      "the amount of sub-strings and figures out their lengths") },
	{ "distribute",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_distribute,
	  DOC("(substring_count:?Dint)->?S?.\n"
	      "Split @this Bytes object into @substring_count similarly sized sub-strings, each with a "
	      "length of ${(#this + (substring_count - 1)) / substring_count}, followed by a last, optional "
	      "sub-string containing all remaining characters.\n"
	      "This function is similar to #segments, but instead of being given the "
	      "amount of sub-strings and figuring out their lengths, this function takes "
	      "the length of sub-strings and figures out their amount") },

	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&bytes_sizeof,
	  DOC("->?Dint") },

	{ NULL }
};


DECL_END

#ifndef __INTELLISENSE__
#include "bytes_split.c.inl"
#include "bytes_segments.c.inl"
#include "bytes_finder.c.inl"
#endif

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL */
