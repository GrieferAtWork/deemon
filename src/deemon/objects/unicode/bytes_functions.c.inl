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
#include <deemon/regex.h>
#include <deemon/seq.h>
#include <deemon/system-features.h> /* memcpy(), memset(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include "regroups.h"
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_contains(Bytes *self, DeeObject *find_ob) {
	Needle needle;
	if (get_needle(&needle, find_ob))
		goto err;
	return_bool(memmemb(DeeBytes_DATA(self),
	                    DeeBytes_SIZE(self),
	                    needle.n_data,
	                    needle.n_size) != NULL);
err:
	return NULL;
}


#ifndef PRIVATE_FIND_KWLIST_DEFINED
#define PRIVATE_FIND_KWLIST_DEFINED 1
PRIVATE struct keyword find_kwlist[] = { K(needle), K(start), K(end), KEND };
#endif /* !PRIVATE_FIND_KWLIST_DEFINED */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_find(Bytes *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":find",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefind(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casefind",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		ptr = NULL;
	else {
		ptr = (uint8_t *)memasciicasemem(DeeBytes_DATA(self) + start,
		                                 end - start,
		                                 needle.n_data,
		                                 needle.n_size);
	}
	if (!ptr)
		goto not_found;
	result = (size_t)(ptr - DeeBytes_DATA(self));
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result,
	                     result + needle.n_size);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rfind(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rfind",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end) {
		result = NULL;
	} else {
		result = (uint8_t *)memrmemb(DeeBytes_DATA(self) + start,
		                             end - start,
		                             needle.n_data,
		                             needle.n_size);
	}
	if (!result)
		return_reference_(&DeeInt_MinusOne);
	return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserfind(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserfind",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end) {
		ptr = NULL;
	} else {
		ptr = (uint8_t *)memasciicasermem(DeeBytes_DATA(self) + start,
		                                  end - start,
		                                  needle.n_data,
		                                  needle.n_size);
	}
	if (!ptr)
		goto not_found;
	result = (size_t)(ptr - DeeBytes_DATA(self));
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result,
	                     result + needle.n_size);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_index(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":index",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end) {
		result = NULL;
	} else {
		result = (uint8_t *)memmemb(DeeBytes_DATA(self) + start,
		                            end - start,
		                            needle.n_data,
		                            needle.n_size);
	}
	if (!result) {
		err_index_not_found((DeeObject *)self, find_ob);
		goto err;
	}
	return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseindex(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caseindex",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end) {
		ptr = NULL;
	} else {
		ptr = (uint8_t *)memasciicasemem(DeeBytes_DATA(self) + start,
		                                 end - start,
		                                 needle.n_data,
		                                 needle.n_size);
	}
	if (!ptr) {
		err_index_not_found((DeeObject *)self, find_ob);
		goto err;
	}
	result = (size_t)(ptr - DeeBytes_DATA(self));
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result,
	                     result + needle.n_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rindex(Bytes *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rindex",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end) {
		result = NULL;
	} else {
		result = (uint8_t *)memrmemb(DeeBytes_DATA(self) + start,
		                             end - start,
		                             needle.n_data,
		                             needle.n_size);
	}
	if (!result) {
		err_index_not_found((DeeObject *)self, find_ob);
		goto err;
	}
	return DeeInt_NewSize((size_t)(result - DeeBytes_DATA(self)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserindex(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr;
	size_t result;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserindex",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end) {
		ptr = NULL;
	} else {
		ptr = (uint8_t *)memasciicasermem(DeeBytes_DATA(self) + start,
		                                  end - start,
		                                  needle.n_data,
		                                  needle.n_size);
	}
	if (!ptr) {
		err_index_not_found((DeeObject *)self, find_ob);
		goto err;
	}
	result = (size_t)(ptr - DeeBytes_DATA(self));
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result,
	                     result + needle.n_size);
err:
	return NULL;
}


LOCAL NONNULL((1, 2, 5)) int DCALL
bytes_find_specific_needle(Bytes *self, DeeObject *find_ob,
                           size_t start, size_t mylen,
                           size_t *__restrict p_end) {
	Needle needle;
	size_t end;
	uint8_t *result;
	if (get_needle(&needle, find_ob))
		goto err;
	end = *p_end + needle.n_size;
	if (end > mylen)
		end = mylen;
	result = (uint8_t *)memmemb(DeeBytes_DATA(self) + start,
	                            end - start,
	                            needle.n_data,
	                            needle.n_size);
	if (result != NULL)
		*p_end = (size_t)(result - DeeBytes_DATA(self));
	return 0;
err:
	return -1;
}

LOCAL NONNULL((1, 2, 3, 5)) int DCALL
bytes_rfind_specific_needle(Bytes *self, DeeObject *find_ob,
                            size_t *__restrict p_start, size_t end,
                            bool *__restrict p_did_find_any) {
	Needle needle;
	size_t start;
	uint8_t *result;
	if (get_needle(&needle, find_ob))
		goto err;
	start = *p_start;
	result = (uint8_t *)memmemb(DeeBytes_DATA(self) + start,
	                            end - start,
	                            needle.n_data,
	                            needle.n_size);
	if (result != NULL) {
		*p_start        = (size_t)(result - DeeBytes_DATA(self));
		*p_did_find_any = true;
	}
	return 0;
err:
	return -1;
}

LOCAL NONNULL((1, 2, 5, 6)) int DCALL
bytes_casefind_specific_needle(Bytes *self, DeeObject *find_ob,
                               size_t start, size_t mylen,
                               size_t *__restrict p_end,
                               size_t *__restrict p_match_length) {
	Needle needle;
	size_t end;
	uint8_t *result;
	if (get_needle(&needle, find_ob))
		goto err;
	end = *p_end + needle.n_size;
	if (end > mylen)
		end = mylen;
	result = (uint8_t *)memasciicasemem(DeeBytes_DATA(self) + start,
	                                    end - start,
	                                    needle.n_data,
	                                    needle.n_size);
	if (result != NULL) {
		*p_end = (size_t)(result - DeeBytes_DATA(self));
		*p_match_length = needle.n_size;
	}
	return 0;
err:
	return -1;
}

LOCAL NONNULL((1, 2, 3, 5, 6)) int DCALL
bytes_caserfind_specific_needle(Bytes *self, DeeObject *find_ob,
                                size_t *__restrict p_start, size_t end,
                                bool *__restrict p_did_find_any,
                                size_t *__restrict p_match_length) {
	Needle needle;
	size_t start;
	uint8_t *result;
	if (get_needle(&needle, find_ob))
		goto err;
	start = *p_start;
	result = (uint8_t *)memasciicasermem(DeeBytes_DATA(self) + start,
	                                     end - start,
	                                     needle.n_data,
	                                     needle.n_size);
	if (result != NULL) {
		*p_start        = (size_t)(result - DeeBytes_DATA(self));
		*p_did_find_any = true;
		*p_match_length = needle.n_size;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_findany(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *iter, *elem;
	size_t fastsize, mylen, orig_end;
	DeeObject *needles;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":findany",
	                    &needles, &start, &end))
		goto err;
	mylen = DeeBytes_SIZE(self);
	if unlikely(start > mylen)
		start = mylen;
	if likely(end > mylen)
		end = mylen;
	if unlikely(end <= start)
		goto not_found;
	orig_end = end;
	fastsize = DeeFastSeq_GetSize(needles);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(needles, i);
			if unlikely(bytes_find_specific_needle(self, elem, start, mylen, &end))
				goto err_elem;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
	} else {
		/* Must use an iterator. */
		iter = DeeObject_IterSelf(needles);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			if unlikely(bytes_find_specific_needle(self, elem, start, mylen, &end))
				goto err_elem_iter;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	if (end < orig_end)
		return DeeInt_NewSize(end);
not_found:
	return_none;
err_elem_iter:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefindany(Bytes *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *iter, *elem;
	size_t fastsize, mylen, orig_end, match_length;
	DeeObject *needles;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casefindany",
	                    &needles, &start, &end))
		goto err;
	mylen = DeeBytes_SIZE(self);
	if unlikely(start > mylen)
		start = mylen;
	if likely(end > mylen)
		end = mylen;
	if unlikely(end <= start)
		goto not_found;
	orig_end     = end;
	match_length = 0; /* Prevent compiler warnings... */
	fastsize     = DeeFastSeq_GetSize(needles);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(needles, i);
			if unlikely(bytes_casefind_specific_needle(self, elem, start, mylen, &end, &match_length))
				goto err_elem;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
	} else {
		/* Must use an iterator. */
		iter = DeeObject_IterSelf(needles);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			if unlikely(bytes_casefind_specific_needle(self, elem, start, mylen, &end, &match_length))
				goto err_elem_iter;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	if (end < orig_end) {
		return DeeTuple_Newf(PCKuSIZ
		                     PCKuSIZ,
		                     end,
		                     end + match_length);
	}
not_found:
	return_none;
err_elem_iter:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rfindany(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DeeObject *needles;
	DREF DeeObject *iter, *elem;
	size_t fastsize, mylen;
	size_t start = 0, end = (size_t)-1;
	bool did_find_any;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rfindany",
	                    &needles, &start, &end))
		goto err;
	did_find_any = false;
	mylen = DeeBytes_SIZE(self);
	if unlikely(start > mylen)
		start = mylen;
	if likely(end > mylen)
		end = mylen;
	if unlikely(end <= start)
		goto not_found;
	fastsize = DeeFastSeq_GetSize(needles);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(needles, i);
			if unlikely(bytes_rfind_specific_needle(self, elem, &start, mylen, &did_find_any))
				goto err_elem;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
	} else {
		/* Must use an iterator. */
		iter = DeeObject_IterSelf(needles);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			if unlikely(bytes_rfind_specific_needle(self, elem, &start, mylen, &did_find_any))
				goto err_elem_iter;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	if (did_find_any)
		return DeeInt_NewSize(start);
not_found:
	return_none;
err_elem_iter:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserfindany(Bytes *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	DeeObject *needles;
	DREF DeeObject *iter, *elem;
	size_t fastsize, mylen, match_length;
	size_t start = 0, end = (size_t)-1;
	bool did_find_any;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserfindany",
	                    &needles, &start, &end))
		goto err;
	did_find_any = false;
	mylen = DeeBytes_SIZE(self);
	if unlikely(start > mylen)
		start = mylen;
	if likely(end > mylen)
		end = mylen;
	if unlikely(end <= start)
		goto not_found;
	match_length = 0; /* Prevent compiler warnings... */
	fastsize     = DeeFastSeq_GetSize(needles);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(needles, i);
			if unlikely(bytes_caserfind_specific_needle(self, elem, &start, mylen, &did_find_any, &match_length))
				goto err_elem;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
	} else {
		/* Must use an iterator. */
		iter = DeeObject_IterSelf(needles);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			if unlikely(bytes_caserfind_specific_needle(self, elem, &start, mylen, &did_find_any, &match_length))
				goto err_elem_iter;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	if (did_find_any) {
		return DeeTuple_Newf(PCKuSIZ
		                     PCKuSIZ,
		                     start,
		                     start + match_length);
	}
not_found:
	return_none;
err_elem_iter:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_indexany(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *iter, *elem;
	size_t fastsize, mylen, orig_end;
	DeeObject *needles;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":indexany",
	                    &needles, &start, &end))
		goto err;
	mylen = DeeBytes_SIZE(self);
	if unlikely(start > mylen)
		start = mylen;
	if likely(end > mylen)
		end = mylen;
	if unlikely(end <= start)
		goto not_found;
	orig_end = end;
	fastsize = DeeFastSeq_GetSize(needles);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(needles, i);
			if unlikely(bytes_find_specific_needle(self, elem, start, mylen, &end))
				goto err_elem;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
	} else {
		/* Must use an iterator. */
		iter = DeeObject_IterSelf(needles);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			if unlikely(bytes_find_specific_needle(self, elem, start, mylen, &end))
				goto err_elem_iter;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	if (end < orig_end)
		return DeeInt_NewSize(end);
not_found:
	err_index_not_found((DeeObject *)self, needles);
	goto err;
err_elem_iter:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseindexany(Bytes *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *iter, *elem;
	size_t fastsize, mylen, orig_end, match_length;
	DeeObject *needles;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caseindexany",
	                    &needles, &start, &end))
		goto err;
	mylen = DeeBytes_SIZE(self);
	if unlikely(start > mylen)
		start = mylen;
	if likely(end > mylen)
		end = mylen;
	if unlikely(end <= start)
		goto not_found;
	orig_end     = end;
	match_length = 0; /* Prevent compiler warnings... */
	fastsize     = DeeFastSeq_GetSize(needles);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(needles, i);
			if unlikely(bytes_casefind_specific_needle(self, elem, start, mylen, &end, &match_length))
				goto err_elem;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
	} else {
		/* Must use an iterator. */
		iter = DeeObject_IterSelf(needles);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			if unlikely(bytes_casefind_specific_needle(self, elem, start, mylen, &end, &match_length))
				goto err_elem_iter;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	if (end < orig_end) {
		return DeeTuple_Newf(PCKuSIZ
		                     PCKuSIZ,
		                     end,
		                     end + match_length);
	}
not_found:
	err_index_not_found((DeeObject *)self, needles);
	goto err;
err_elem_iter:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rindexany(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *needles;
	DREF DeeObject *iter, *elem;
	size_t fastsize, mylen;
	size_t start = 0, end = (size_t)-1;
	bool did_find_any;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rindexany",
	                    &needles, &start, &end))
		goto err;
	did_find_any = false;
	mylen = DeeBytes_SIZE(self);
	if unlikely(start > mylen)
		start = mylen;
	if likely(end > mylen)
		end = mylen;
	if unlikely(end <= start)
		goto not_found;
	fastsize = DeeFastSeq_GetSize(needles);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(needles, i);
			if unlikely(bytes_rfind_specific_needle(self, elem, &start, mylen, &did_find_any))
				goto err_elem;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
	} else {
		/* Must use an iterator. */
		iter = DeeObject_IterSelf(needles);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			if unlikely(bytes_rfind_specific_needle(self, elem, &start, mylen, &did_find_any))
				goto err_elem_iter;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	if (did_find_any)
		return DeeInt_NewSize(start);
not_found:
	err_index_not_found((DeeObject *)self, needles);
	goto err;
err_elem_iter:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserindexany(Bytes *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	DeeObject *needles;
	DREF DeeObject *iter, *elem;
	size_t fastsize, mylen, match_length;
	size_t start = 0, end = (size_t)-1;
	bool did_find_any;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserindexany",
	                    &needles, &start, &end))
		goto err;
	did_find_any = false;
	mylen = DeeBytes_SIZE(self);
	if unlikely(start > mylen)
		start = mylen;
	if likely(end > mylen)
		end = mylen;
	if unlikely(end <= start)
		goto not_found;
	match_length = 0; /* Prevent compiler warnings... */
	fastsize     = DeeFastSeq_GetSize(needles);
	if (fastsize != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fastsize; ++i) {
			elem = DeeFastSeq_GetItem(needles, i);
			if unlikely(bytes_caserfind_specific_needle(self, elem, &start, mylen, &did_find_any, &match_length))
				goto err_elem;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
	} else {
		/* Must use an iterator. */
		iter = DeeObject_IterSelf(needles);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			if unlikely(bytes_caserfind_specific_needle(self, elem, &start, mylen, &did_find_any, &match_length))
				goto err_elem_iter;
			Dee_Decref(elem);
			if unlikely(end <= start)
				break;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	if (did_find_any) {
		return DeeTuple_Newf(PCKuSIZ
		                     PCKuSIZ,
		                     start,
		                     start + match_length);
	}
not_found:
	err_index_not_found((DeeObject *)self, needles);
	goto err;
err_elem_iter:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
	goto err;
err_elem:
	Dee_Decref(elem);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_count(Bytes *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t result;
	size_t start = 0, end = (size_t)-1;
	uint8_t *iter;
	size_t size;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":count",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casecount(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t result;
	size_t start = 0, end = (size_t)-1;
	uint8_t *iter;
	size_t size;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casecount",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	iter = DeeBytes_DATA(self);
	size = DeeBytes_SIZE(self);
	if (end > size)
		end = size;
	result = 0;
	if (start < end) {
		end -= start;
		iter += start;
		while (end >= needle.n_size) {
			if (memasciicaseeq(iter, needle.n_data, needle.n_size))
				++result;
			--end;
			++iter;
		}
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_contains_f(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":contains",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_false;
	return_bool(memmemb(DeeBytes_DATA(self) + start,
	                    end - start,
	                    needle.n_data,
	                    needle.n_size) != NULL);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casecontains_f(Bytes *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casecontains",
	                    &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return_false;
	return_bool(memasciicasemem(DeeBytes_DATA(self) + start,
	                            end - start,
	                            needle.n_data,
	                            needle.n_size) != NULL);
err:
	return NULL;
}

INTDEF dssize_t DCALL
DeeBytes_Format(dformatprinter printer,
                dformatprinter format_printer, void *arg,
                char const *__restrict format,
                size_t format_len, DeeObject *__restrict args);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_format(Bytes *self, size_t argc, DeeObject *const *argv) {
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



PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
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

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_substr(Bytes *__restrict self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":substr", &start, &end))
		goto err;
	return bytes_getsubstr(self, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_resized(Bytes *self,
              size_t argc, DeeObject *const *argv) {
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
		if (DeeArg_Unpack(argc, argv, UNPuSIZ "|" UNPu8 ":resized", &new_size, &init))
			goto err;
		result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(new_size);
		if unlikely(!result)
			goto err;
		if (new_size <= DeeBytes_SIZE(self)) {
			memcpy(result->b_data, DeeBytes_DATA(self), new_size);
		} else {
			void *endptr;
			size_t old_size = DeeBytes_SIZE(self);
			endptr = mempcpy(result->b_data, DeeBytes_DATA(self), old_size);
			memset(endptr, init, new_size - old_size);
		}
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_reversed(Bytes *__restrict self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DREF Bytes *result;
	uint8_t *data, *dst;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":reversed", &start, &end))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_reverse(Bytes *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	uint8_t *data, *dst;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":reverse", &start, &end))
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

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_makereadonly(Bytes *self,
                   size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_makewritable(Bytes *self,
                   size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_hex(Bytes *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1, i;
	char *dst;
	DREF DeeStringObject *result;
	uint8_t *data;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":hex", &start, &end))
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
#ifndef CONFIG_NO_THREADS
		COMPILER_READ_BARRIER();
#endif /* !CONFIG_NO_THREADS */
		*dst++ = DeeAscii_ItoaLowerDigit(byte >> 4);
		*dst++ = DeeAscii_ItoaLowerDigit(byte & 0xf);
	}
	ASSERT(dst == DeeString_END(result));
	return (DREF DeeObject *)result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_ord(Bytes *self, size_t argc, DeeObject *const *argv) {
	size_t index = 0;
	if (argc) {
		if (DeeArg_Unpack(argc, argv, UNPuSIZ ":ord", &index))
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

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *self,
                DeeObject *format);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_scanf(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *format;
	if (DeeArg_Unpack(argc, argv, "o:scanf", &format))
		goto err;
	if (DeeObject_AssertTypeExact(format, &DeeString_Type))
		goto err;
	return DeeString_Scanf((DeeObject *)self, format);
err:
	return NULL;
}


#define DeeBytes_IsCntrl(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISCNTRL)
#define DeeBytes_IsTab(self, start, end)     DeeBytes_TestTrait(self, start, end, UNICODE_ISTAB)
#define DeeBytes_IsCempty(self, start, end)  DeeBytes_TestTrait(self, start, end, UNICODE_ISEMPTY)
#define DeeBytes_IsWhite(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISWHITE)
#define DeeBytes_IsLF(self, start, end)      DeeBytes_TestTrait(self, start, end, UNICODE_ISLF)
#define DeeBytes_IsSpace(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISSPACE)
#define DeeBytes_IsLower(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISLOWER)
#define DeeBytes_IsUpper(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISUPPER)
#define DeeBytes_IsAlpha(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISALPHA)
#define DeeBytes_IsDigit(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISDIGIT)
#define DeeBytes_IsHex(self, start, end)     DeeBytes_TestTrait(self, start, end, UNICODE_ISHEX)
#define DeeBytes_IsXdigit(self, start, end)  DeeBytes_TestTrait(self, start, end, UNICODE_ISXDIGIT)
#define DeeBytes_IsAlnum(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISALNUM)
#define DeeBytes_IsPunct(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISPUNCT)
#define DeeBytes_IsGraph(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISGRAPH)
#define DeeBytes_IsPrint(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISPRINT)
#define DeeBytes_IsBlank(self, start, end)   DeeBytes_TestTrait(self, start, end, UNICODE_ISBLANK)
#define DeeBytes_IsNumeric(self, start, end) DeeBytes_TestTrait(self, start, end, UNICODE_ISNUMERIC)
#define DeeBytes_IsSymStrt(self, start, end) DeeBytes_TestTrait(self, start, end, UNICODE_ISSYMSTRT)
#define DeeBytes_IsSymCont(self, start, end) DeeBytes_TestTrait(self, start, end, UNICODE_ISSYMCONT)

#define DeeBytes_IsAnyCntrl(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISCNTRL)
#define DeeBytes_IsAnyTab(self, start, end)     DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISTAB)
#define DeeBytes_IsAnyCempty(self, start, end)  DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISEMPTY)
#define DeeBytes_IsAnyWhite(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISWHITE)
#define DeeBytes_IsAnyLF(self, start, end)      DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISLF)
#define DeeBytes_IsAnySpace(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISSPACE)
#define DeeBytes_IsAnyLower(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISLOWER)
#define DeeBytes_IsAnyUpper(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISUPPER)
#define DeeBytes_IsAnyAlpha(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISALPHA)
#define DeeBytes_IsAnyDigit(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISDIGIT)
#define DeeBytes_IsAnyHex(self, start, end)     DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISHEX)
#define DeeBytes_IsAnyXdigit(self, start, end)  DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISXDIGIT)
#define DeeBytes_IsAnyAlnum(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISALNUM)
#define DeeBytes_IsAnyPunct(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISPUNCT)
#define DeeBytes_IsAnyGraph(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISGRAPH)
#define DeeBytes_IsAnyPrint(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISPRINT)
#define DeeBytes_IsAnyBlank(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISBLANK)
#define DeeBytes_IsAnyTitle(self, start, end)   DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISTITLE)
#define DeeBytes_IsAnyNumeric(self, start, end) DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISNUMERIC)
#define DeeBytes_IsAnySymStrt(self, start, end) DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISSYMSTRT)
#define DeeBytes_IsAnySymCont(self, start, end) DeeBytes_TestAnyTrait(self, start, end, UNICODE_ISSYMCONT)

PRIVATE WUNUSED NONNULL((1)) bool DCALL
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

PRIVATE WUNUSED NONNULL((1)) bool DCALL
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

PRIVATE WUNUSED NONNULL((1)) bool DCALL
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

PRIVATE WUNUSED NONNULL((1)) bool DCALL
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


PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeBytes_IsTitle(Bytes *__restrict self,
                 size_t start_index,
                 size_t end_index) {
	uniflag_t flags = (UNICODE_ISTITLE | UNICODE_ISUPPER | UNICODE_ISSPACE);
	uint8_t *iter;
	if (end_index > DeeBytes_SIZE(self))
		end_index = DeeBytes_SIZE(self);
	if (start_index < end_index) {
		iter = DeeBytes_DATA(self) + start_index;
		while (start_index < end_index) {
			uniflag_t f = DeeUni_Flags(*iter);
			if (!(f & flags))
				return false;
			flags = (f & UNICODE_ISSPACE) ? (UNICODE_ISTITLE | UNICODE_ISUPPER | UNICODE_ISSPACE)
			                             : (UNICODE_ISLOWER | UNICODE_ISSPACE);
			++iter;
			++start_index;
		}
	}
	return true;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeBytes_IsSymbol(Bytes *__restrict self,
                  size_t start_index,
                  size_t end_index) {
	uniflag_t flags = UNICODE_ISSYMSTRT;
	uint8_t *iter;
	if (end_index > DeeBytes_SIZE(self))
		end_index = DeeBytes_SIZE(self);
	if (start_index < end_index) {
		iter = DeeBytes_DATA(self) + start_index;
		while (start_index < end_index) {
			if (!(DeeUni_Flags(*iter) & flags))
				return false;
			flags = UNICODE_ISSYMCONT;
			++iter;
			++start_index;
		}
	}
	return true;
}

#define DEFINE_BYTES_TRAIT(name, function, test_ch)                     \
	PRIVATE WUNUSED DREF DeeObject *DCALL                               \
	bytes_##name(Bytes *self, size_t argc, DeeObject *const *argv) {    \
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
			if (DeeArg_Unpack(argc, argv,                               \
			                  "|" UNPdSIZ UNPdSIZ ":" #name,            \
			                  &start, &end))                            \
				goto err;                                               \
			return_bool(function(self, start, end));                    \
		}                                                               \
	err:                                                                \
		return NULL;                                                    \
	}
#define DEFINE_ANY_BYTES_TRAIT(name, function)                                      \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                           \
	bytes_##name(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) { \
		size_t start = 0, end = (size_t)-1;                                         \
		if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,                          \
		                    "|" UNPdSIZ UNPdSIZ ":" #name, &start, &end))           \
			goto err;                                                               \
		return_bool(function(self, start, end));                                    \
	err:                                                                            \
		return NULL;                                                                \
	}
DEFINE_BYTES_TRAIT(iscntrl, DeeBytes_IsCntrl, DeeUni_IsCntrl(ch))
DEFINE_BYTES_TRAIT(istab, DeeBytes_IsTab, DeeUni_IsTab(ch))
DEFINE_BYTES_TRAIT(iscempty, DeeBytes_IsCempty, DeeUni_IsEmpty(ch))
DEFINE_BYTES_TRAIT(iswhite, DeeBytes_IsWhite, DeeUni_IsWhite(ch))
DEFINE_BYTES_TRAIT(islf, DeeBytes_IsLF, DeeUni_IsLF(ch))
DEFINE_BYTES_TRAIT(isspace, DeeBytes_IsSpace, DeeUni_IsSpace(ch))
DEFINE_BYTES_TRAIT(islower, DeeBytes_IsLower, DeeUni_IsLower(ch))
DEFINE_BYTES_TRAIT(isupper, DeeBytes_IsUpper, DeeUni_IsUpper(ch))
DEFINE_BYTES_TRAIT(isalpha, DeeBytes_IsAlpha, DeeUni_IsAlpha(ch))
DEFINE_BYTES_TRAIT(isdigit, DeeBytes_IsDigit, DeeUni_IsDigit(ch))
DEFINE_BYTES_TRAIT(ishex, DeeBytes_IsHex, DeeUni_IsHex(ch))
DEFINE_BYTES_TRAIT(isxdigit, DeeBytes_IsXdigit, DeeUni_IsXdigit(ch))
DEFINE_BYTES_TRAIT(isalnum, DeeBytes_IsAlnum, DeeUni_IsAlnum(ch))
DEFINE_BYTES_TRAIT(ispunct, DeeBytes_IsPunct, DeeUni_IsPunct(ch))
DEFINE_BYTES_TRAIT(isgraph, DeeBytes_IsGraph, DeeUni_IsGraph(ch))
DEFINE_BYTES_TRAIT(isprint, DeeBytes_IsPrint, DeeUni_IsPrint(ch))
DEFINE_BYTES_TRAIT(isblank, DeeBytes_IsBlank, DeeUni_IsBlank(ch))
DEFINE_BYTES_TRAIT(istitle, DeeBytes_IsTitle, DeeUni_IsTitle(ch))
DEFINE_BYTES_TRAIT(isnumeric, DeeBytes_IsNumeric, DeeUni_IsNumeric(ch))
DEFINE_BYTES_TRAIT(issymstrt, DeeBytes_IsSymStrt, DeeUni_IsSymStrt(ch))
DEFINE_BYTES_TRAIT(issymcont, DeeBytes_IsSymCont, DeeUni_IsSymCont(ch))
DEFINE_BYTES_TRAIT(issymbol, DeeBytes_IsSymbol, DeeUni_IsSymStrt(ch))
DEFINE_BYTES_TRAIT(isascii, DeeBytes_IsAscii, ch <= 0x7f)
DEFINE_ANY_BYTES_TRAIT(isanycntrl, DeeBytes_IsAnyCntrl)
DEFINE_ANY_BYTES_TRAIT(isanytab, DeeBytes_IsAnyTab)
DEFINE_ANY_BYTES_TRAIT(isanycempty, DeeBytes_IsAnyCempty)
DEFINE_ANY_BYTES_TRAIT(isanywhite, DeeBytes_IsAnyWhite)
DEFINE_ANY_BYTES_TRAIT(isanylf, DeeBytes_IsAnyLF)
DEFINE_ANY_BYTES_TRAIT(isanyspace, DeeBytes_IsAnySpace)
DEFINE_ANY_BYTES_TRAIT(isanylower, DeeBytes_IsAnyLower)
DEFINE_ANY_BYTES_TRAIT(isanyupper, DeeBytes_IsAnyUpper)
DEFINE_ANY_BYTES_TRAIT(isanyalpha, DeeBytes_IsAnyAlpha)
DEFINE_ANY_BYTES_TRAIT(isanydigit, DeeBytes_IsAnyDigit)
DEFINE_ANY_BYTES_TRAIT(isanyhex, DeeBytes_IsAnyHex)
DEFINE_ANY_BYTES_TRAIT(isanyxdigit, DeeBytes_IsAnyXdigit)
DEFINE_ANY_BYTES_TRAIT(isanyalnum, DeeBytes_IsAnyAlnum)
DEFINE_ANY_BYTES_TRAIT(isanypunct, DeeBytes_IsAnyPunct)
DEFINE_ANY_BYTES_TRAIT(isanygraph, DeeBytes_IsAnyGraph)
DEFINE_ANY_BYTES_TRAIT(isanyprint, DeeBytes_IsAnyPrint)
DEFINE_ANY_BYTES_TRAIT(isanyblank, DeeBytes_IsAnyBlank)
DEFINE_ANY_BYTES_TRAIT(isanytitle, DeeBytes_IsAnyTitle)
DEFINE_ANY_BYTES_TRAIT(isanynumeric, DeeBytes_IsAnyNumeric)
DEFINE_ANY_BYTES_TRAIT(isanysymstrt, DeeBytes_IsAnySymStrt)
DEFINE_ANY_BYTES_TRAIT(isanysymcont, DeeBytes_IsAnySymCont)
DEFINE_ANY_BYTES_TRAIT(isanyascii, DeeBytes_IsAnyAscii)
#undef DEFINE_ANY_BYTES_TRAIT
#undef DEFINE_BYTES_TRAIT

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_asdigit(Bytes *self, size_t argc, DeeObject *const *argv) {
	uint8_t ch, digit;
	DeeObject *defl = NULL;
	if (argc == 0) {
		if unlikely(DeeBytes_SIZE(self) != 1)
			goto err_not_single_char;
		ch = DeeBytes_DATA(self)[0];
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:asdigit", &index, &defl))
			goto err;
		if unlikely(index >= DeeBytes_SIZE(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeBytes_SIZE(self));
			goto err;
		}
		ch = DeeBytes_DATA(self)[index];
	}
	digit = DeeUni_AsDigitVal(ch);
	if likely(digit < 10)
		return DeeInt_NewU8(digit);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_asxdigit(Bytes *self, size_t argc, DeeObject *const *argv) {
	uint8_t ch, digit;
	DeeObject *defl = NULL;
	if (argc == 0) {
		if unlikely(DeeBytes_SIZE(self) != 1)
			goto err_not_single_char;
		ch = DeeBytes_DATA(self)[0];
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:asxdigit", &index, &defl))
			goto err;
		if unlikely(index >= DeeBytes_SIZE(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeBytes_SIZE(self));
			goto err;
		}
		ch = DeeBytes_DATA(self)[index];
	}
	digit = DeeUni_AsDigitVal(ch);
	if likely(digit != 0xff)
		return DeeInt_NewU8(digit);
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I8C isn't a hex-digit",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_lower(Bytes *__restrict self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":lower", &start, &end))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_upper(Bytes *__restrict self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":upper", &start, &end))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_title(Bytes *__restrict self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	uintptr_t kind = UNICODE_CONVERT_TITLE;
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":title", &start, &end))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_capitalize(Bytes *__restrict self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":capitalize", &start, &end))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_swapcase(Bytes *__restrict self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	DREF Bytes *result;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":swapcase", &start, &end))
		goto err;
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
err:
	return NULL;
}



PRIVATE WUNUSED DREF Bytes *DCALL
bytes_tolower(Bytes *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":tolower", &start, &end))
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

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_toupper(Bytes *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":toupper", &start, &end))
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

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_totitle(Bytes *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	uintptr_t kind = UNICODE_CONVERT_TITLE;
	size_t i, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":totitle", &start, &end))
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

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_tocapitalize(Bytes *__restrict self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":tocapitalize", &start, &end))
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

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_toswapcase(Bytes *__restrict self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	size_t i, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":toswapcase", &start, &end))
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

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_replace(Bytes *__restrict self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DREF Bytes *result;
	uint8_t *begin, *end, *block_begin;
	DeeObject *find_ob, *replace_ob;
	size_t max_count = (size_t)-1;
	Needle find_needle, replace_needle;
	struct bytes_printer printer;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|" UNPuSIZ ":replace", &find_ob, &replace_ob, &max_count))
		goto err;
	if (get_needle(&find_needle, find_ob))
		goto err;
	if (get_needle(&replace_needle, replace_ob))
		goto err;
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
	if likely(max_count) {
		while (begin < end) {
			if (MEMEQB(begin, find_needle.n_data, find_needle.n_size)) {
				/* Found one */
				if unlikely(bytes_printer_append(&printer, block_begin, (size_t)(begin - block_begin)) < 0)
					goto err_printer;
				if unlikely(bytes_printer_append(&printer, replace_needle.n_data, replace_needle.n_size) < 0)
					goto err_printer;
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
	}
	end += (find_needle.n_size - 1);
	ASSERT(block_begin <= end);
	if unlikely(bytes_printer_append(&printer, block_begin,
	                                 (size_t)(end - block_begin)) < 0)
		goto err_printer;
	/* Pack together a Bytes object. */
	return (DREF Bytes *)bytes_printer_pack(&printer);
err_printer:
	bytes_printer_fini(&printer);
err:
	return NULL;
return_self:
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(DeeBytes_SIZE(self));
	if likely(result)
		memcpy(result->b_data, DeeBytes_DATA(self), DeeBytes_SIZE(self));
	return result;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_casereplace(Bytes *__restrict self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DREF Bytes *result;
	uint8_t *begin, *end, *block_begin;
	DeeObject *find_ob, *replace_ob;
	size_t max_count = (size_t)-1;
	Needle find_needle, replace_needle;
	struct bytes_printer printer;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|" UNPuSIZ ":casereplace", &find_ob, &replace_ob, &max_count))
		goto err;
	if (get_needle(&find_needle, find_ob))
		goto err;
	if (get_needle(&replace_needle, replace_ob))
		goto err;
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
	if likely(max_count) {
		while (begin < end) {
			if (memasciicaseeq(begin, find_needle.n_data, find_needle.n_size)) {
				/* Found one */
				if unlikely(bytes_printer_append(&printer, block_begin, (size_t)(begin - block_begin)) < 0)
					goto err_printer;
				if unlikely(bytes_printer_append(&printer, replace_needle.n_data, replace_needle.n_size) < 0)
					goto err_printer;
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
	}
	end += (find_needle.n_size - 1);
	ASSERT(block_begin <= end);
	if unlikely(bytes_printer_append(&printer, block_begin,
	                                 (size_t)(end - block_begin)) < 0)
		goto err_printer;
	/* Pack together a Bytes object. */
	return (DREF Bytes *)bytes_printer_pack(&printer);
err_printer:
	bytes_printer_fini(&printer);
err:
	return NULL;
return_self:
	result = (DREF Bytes *)DeeBytes_NewBufferUninitialized(DeeBytes_SIZE(self));
	if likely(result)
		memcpy(result->b_data, DeeBytes_DATA(self), DeeBytes_SIZE(self));
	return result;
}


PRIVATE WUNUSED DREF Bytes *DCALL
bytes_toreplace(Bytes *__restrict self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob, *replace_ob;
	size_t max_count = (size_t)-1;
	Needle find_needle, replace_needle;
	uint8_t *begin, *end;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|" UNPuSIZ ":toreplace", &find_ob, &replace_ob, &max_count))
		goto err;
	if (get_needle(&find_needle, find_ob))
		goto err;
	if (get_needle(&replace_needle, replace_ob))
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
	if likely(max_count) {
		while (begin < end) {
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
	}
done:
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_tocasereplace(Bytes *__restrict self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob, *replace_ob;
	size_t max_count = (size_t)-1;
	Needle find_needle, replace_needle;
	uint8_t *begin, *end;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|" UNPuSIZ ":tocasereplace", &find_ob, &replace_ob, &max_count))
		goto err;
	if (get_needle(&find_needle, find_ob))
		goto err;
	if (get_needle(&replace_needle, replace_ob))
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
	if likely(max_count) {
		while (begin < end) {
			if (memasciicaseeq(begin, find_needle.n_data, find_needle.n_size)) {
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
	}
done:
	return_reference_(self);
err:
	return NULL;
}


/* The string decode() and encode() member functions also function for `Bytes' objects.
 * As a matter of fact: they'd work for any kind of object, however built-in
 *                      codecs only function for bytes and string objects! */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_decode(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_encode(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw);


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeBytes_SplitByte(Bytes *__restrict self, uint8_t sep);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeBytes_Split(Bytes *self, DeeObject *sep);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeBytes_CaseSplitByte(Bytes *__restrict self, uint8_t sep);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeBytes_CaseSplit(Bytes *self, DeeObject *sep);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeBytes_SplitLines(Bytes *__restrict self, bool keepends);
INTDEF WUNUSED DREF DeeObject *DCALL DeeBytes_FindAll(Bytes *self, DeeObject *other, size_t start, size_t end);
INTDEF WUNUSED DREF DeeObject *DCALL DeeBytes_CaseFindAll(Bytes *self, DeeObject *other, size_t start, size_t end);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_findall(Bytes *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":findall", &arg, &start, &end))
		goto err;
	return DeeBytes_FindAll(self, arg, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefindall(Bytes *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":casefindall", &arg, &start, &end))
		goto err;
	return DeeBytes_CaseFindAll(self, arg, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_join(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_split(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casesplit(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_splitlines(Bytes *self, size_t argc, DeeObject *const *argv) {
	bool keepends = false;
	if (DeeArg_Unpack(argc, argv, "|b:splitlines", &keepends))
		goto err;
	return DeeBytes_SplitLines(self, keepends);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_startswith(Bytes *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	Needle needle;
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":startswith", &arg, &start, &end))
		goto err;
	if (get_needle(&needle, arg))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start > end || (end - start) < needle.n_size)
		return_false;
	return_bool(MEMEQB(DeeBytes_DATA(self) + start,
	                   needle.n_data, needle.n_size));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casestartswith(Bytes *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	Needle needle;
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":casestartswith", &arg, &start, &end))
		goto err;
	if (get_needle(&needle, arg))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start > end || (end - start) < needle.n_size)
		return_false;
	return_bool(memasciicaseeq(DeeBytes_DATA(self) + start,
	                           needle.n_data, needle.n_size));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_endswith(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	Needle needle;
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":endswith", &arg, &start, &end))
		goto err;
	if (get_needle(&needle, arg))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start > end || (end - start) < needle.n_size)
		return_false;
	return_bool(MEMEQB(DeeBytes_DATA(self) +
	                   (end - needle.n_size),
	                   needle.n_data, needle.n_size));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseendswith(Bytes *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	Needle needle;
	DeeObject *arg;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":caseendswith", &arg, &start, &end))
		goto err;
	if (get_needle(&needle, arg))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start > end || (end - start) < needle.n_size)
		return_false;
	return_bool(memasciicaseeq(DeeBytes_DATA(self) +
	                           (end - needle.n_size),
	                           needle.n_data, needle.n_size));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
bytes_pack_partition(Bytes *self, uint8_t *find_ptr,
                     uint8_t *start_ptr, size_t search_size,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_parition(Bytes *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":partition", &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseparition(Bytes *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":casepartition", &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return DeeTuple_Pack(3, Dee_EmptyBytes, Dee_EmptyBytes, Dee_EmptyBytes);
	end -= start;
	return bytes_pack_partition(self,
	                            memasciicasemem(DeeBytes_DATA(self) + start,
	                                            end,
	                                            needle.n_data,
	                                            needle.n_size),
	                            DeeBytes_DATA(self) + start,
	                            end,
	                            needle.n_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rparition(Bytes *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":rpartition", &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserparition(Bytes *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	DeeObject *find_ob;
	Needle needle;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|" UNPdSIZ UNPdSIZ ":caserpartition", &find_ob, &start, &end))
		goto err;
	if (get_needle(&needle, find_ob))
		goto err;
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if (start >= end)
		return DeeTuple_Pack(3, Dee_EmptyBytes, Dee_EmptyBytes, Dee_EmptyBytes);
	end -= start;
	return bytes_pack_partition(self,
	                            memasciicasermem(DeeBytes_DATA(self) + start,
	                                             end,
	                                             needle.n_data,
	                                             needle.n_size),
	                            DeeBytes_DATA(self) + start,
	                            end,
	                            needle.n_size);
err:
	return NULL;
}

#ifdef __INTELLISENSE__
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_strip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_lstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_rstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_sstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_lsstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_rsstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casestrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caselstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caserstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casesstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caselsstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casersstrip(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_striplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_lstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_rstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_sstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_lsstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_rsstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casestriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caselstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caserstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casesstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_caselsstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL bytes_casersstriplines(Bytes *self, size_t argc, DeeObject *const *argv);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_bytes_strip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_lstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_rstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_sstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_lsstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_rsstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casestrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caselstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caserstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casesstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caselsstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casersstrip
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_striplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_lstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_rstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_sstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_lsstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_rsstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casestriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caselstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caserstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casesstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_caselsstriplines
#include "bytes_functions-strip.c.inl"
#define DEFINE_bytes_casersstriplines
#include "bytes_functions-strip.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */



struct bcompare_args {
	DeeObject *other;   /* [1..1] String or Bytes object. */
	uint8_t   *lhs_ptr; /* [0..my_len] Starting pointer of lhs. */
	size_t     lhs_len; /* # of bytes in lhs. */
	uint8_t   *rhs_ptr; /* [0..my_len] Starting pointer of rhs. */
	size_t     rhs_len; /* # of bytes in rhs. */
};

PRIVATE int DCALL
get_bcompare_args(Bytes *__restrict self,
                  struct bcompare_args *__restrict args,
                  size_t argc, DeeObject *const *argv,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_compare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "compare"))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_vercompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int8_t result;
	if (get_bcompare_args(self, &args, argc, argv, "vercompare"))
		goto err;
	result = strverscmpb(args.lhs_ptr, args.lhs_len,
	                     args.rhs_ptr, args.lhs_len);
	return DeeInt_NewS8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_wildcompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "wildcompare"))
		goto err;
	result = wildcompareb(args.lhs_ptr, args.lhs_len,
	                      args.rhs_ptr, args.lhs_len);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_fuzzycompare(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_wmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "wmatch"))
		goto err;
	result = wildcompareb(args.lhs_ptr, args.lhs_len,
	                      args.rhs_ptr, args.lhs_len);
	return_bool_(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casecompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "casecompare"))
		goto err;
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
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casevercompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int8_t result;
	if (get_bcompare_args(self, &args, argc, argv, "casevercompare"))
		goto err;
	result = strcaseverscmpb(args.lhs_ptr, args.lhs_len, /* TODO: ASCII variant */
	                         args.rhs_ptr, args.lhs_len);
	return DeeInt_NewS8(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casewildcompare(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "casewildcompare"))
		goto err;
	result = dee_wildasccicasecompareb(args.lhs_ptr, args.lhs_len,
	                                   args.rhs_ptr, args.lhs_len);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefuzzycompare(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casewmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	struct bcompare_args args;
	int result;
	if (get_bcompare_args(self, &args, argc, argv, "casewmatch"))
		goto err;
	result = dee_wildasccicasecompareb(args.lhs_ptr, args.lhs_len,
	                                   args.rhs_ptr, args.lhs_len);
	return_bool_(result == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_center(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	size_t width;
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:center", &width, &filler_ob))
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
		mempfilb(DeeBytes_DATA(result) + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(DeeBytes_DATA(result) + fill_front,
		        DeeBytes_DATA(self), DeeBytes_SIZE(self));
		mempfilb(DeeBytes_DATA(result) + fill_front + DeeBytes_SIZE(self),
		        fill_back, filler.n_data, filler.n_size);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_ljust(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	size_t width;
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:ljust", &width, &filler_ob))
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
		mempfilb(DeeBytes_DATA(result) + DeeBytes_SIZE(self),
		        fill_back, filler.n_data, filler.n_size);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rjust(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	size_t width;
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:rjust", &width, &filler_ob))
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
		mempfilb(DeeBytes_DATA(result) + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(DeeBytes_DATA(result) + fill_front,
		        DeeBytes_DATA(self), DeeBytes_SIZE(self));
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_zfill(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	size_t width;
	DeeObject *filler_ob = NULL;
	Needle filler;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:zfill", &width, &filler_ob))
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
		mempfilb(dst + 0, fill_front, filler.n_data, filler.n_size);
		memcpyb(dst + fill_front, src, src_len);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_expandtabs(Bytes *self, size_t argc, DeeObject *const *argv) {
	size_t tab_width = 8;
	if (DeeArg_Unpack(argc, argv, "|" UNPuSIZ ":expandtabs", &tab_width))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_unifylines(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_indent(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_dedent(Bytes *self, size_t argc, DeeObject *const *argv) {
	size_t max_chars   = 1;
	DeeObject *mask_ob = NULL;
	if (DeeArg_Unpack(argc, argv, "|" UNPuSIZ "o:dedent", &max_chars, &mask_ob))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_common(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rcommon(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casecommon(Bytes *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casercommon(Bytes *self, size_t argc, DeeObject *const *argv) {
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


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_findmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":findmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rfindmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":rfindmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_indexmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":indexmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rindexmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":rindexmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casefindmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":casefindmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
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
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     (size_t)(ptr - scan_str),
	                     s_clos.n_size);
err_not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserfindmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":caserfindmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
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
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result,
	                     result + s_open.n_size);
err_not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caseindexmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":caseindexmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
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
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result,
	                     result + s_clos.n_size);
err_not_found:
	err_index_not_found((DeeObject *)self, s_clos_ob);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserindexmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *ptr, *scan_str;
	size_t scan_len, result;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":caserindexmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
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
	return DeeTuple_Newf(PCKuSIZ
	                     PCKuSIZ,
	                     result,
	                     result + s_open.n_size);
err_not_found:
	err_index_not_found((DeeObject *)self, s_open_ob);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_partitionmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":partitionmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
		goto err;
#define SET_BYTES(a, b, c)                                       \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	}	__WHILE0
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rpartitionmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":rpartitionmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
		goto err;
#define SET_BYTES(a, b, c)                                       \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	}	__WHILE0
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_casepartitionmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":casepartitionmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
		goto err;
#define SET_BYTES(a, b, c)                                       \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	}	__WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_str = DeeBytes_DATA(self);
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto match_not_found; /* Empty search area. */
	scan_len    = end - start;
	match_start = memasciicasemem(scan_str + start, scan_len,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_caserpartitionmatch(Bytes *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	DeeObject *s_open_ob, *s_clos_ob;
	size_t start = 0, end = (size_t)-1;
	uint8_t *scan_str, *match_start, *match_end;
	size_t scan_len;
	Needle s_open, s_clos;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":caserpartitionmatch", &s_open_ob, &s_clos_ob, &start, &end))
		goto err;
	if (get_needle(&s_open, s_open_ob))
		goto err;
	if (get_needle(&s_clos, s_clos_ob))
		goto err;
#define SET_BYTES(a, b, c)                                       \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	}	__WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	scan_str = DeeBytes_DATA(self);
	if (end > DeeBytes_SIZE(self))
		end = DeeBytes_SIZE(self);
	if unlikely(end <= start)
		goto match_not_found; /* Empty search area. */
	scan_len  = end - start;
	match_end = memasciicasermem(scan_str + start, scan_len,
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

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeBytes_Segments(DeeBytesObject *__restrict self, size_t substring_length);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_segments(Bytes *self, size_t argc, DeeObject *const *argv) {
	size_t substring_length;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":segments", &substring_length))
		goto err;
	if unlikely(!substring_length) {
		err_invalid_segment_size(substring_length);
		goto err;
	}
	return DeeBytes_Segments(self, substring_length);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_distribute(Bytes *self, size_t argc, DeeObject *const *argv) {
	size_t substring_count;
	size_t substring_length;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":distribute", &substring_count))
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


/************************************************************************/
/* Regex functions                                                      */
/************************************************************************/
#ifdef __INTELLISENSE__ /* Stuff we share with "./string_functions.c" */
PRIVATE struct keyword string_rereplace_kwlist[];
PRIVATE struct keyword generic_regex_kwlist[];
PRIVATE struct keyword search_regex_kwlist[];
struct DeeRegexExecWithRange {
	struct DeeRegexExec rewr_exec;  /* Normal exec args */
	size_t              rewr_range; /* Max # of search attempts to perform (in bytes) */
};

struct DeeRegexBaseExec {
	DREF String               *rx_pattern;  /* [1..1] Pattern string (only a reference within objects in "./reproxy.c.inl") */
	struct DeeRegexCode const *rx_code;     /* [1..1] Regex code */
	void const                *rx_inbase;   /* [0..rx_insize][valid_if(rx_startoff < rx_endoff)] Input data to scan
	                                         * When `rx_code' was compiled with `DEE_REGEX_COMPILE_NOUTF8', this data
	                                         * is treated as raw bytes; otherwise, it is treated as a utf-8 string.
	                                         * In either case, `rx_insize' is the # of bytes within this buffer. */
	size_t                     rx_insize;   /* Total # of bytes starting at `rx_inbase' */
	size_t                     rx_startoff; /* Starting byte offset into `rx_inbase' of data to match. */
	size_t                     rx_endoff;   /* Ending byte offset into `rx_inbase' of data to match. */
	unsigned int               rx_eflags;   /* Execution-flags (set of `DEE_RE_EXEC_*') */
};

/* Functions from "./reproxy.c.inl" */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_findall(DeeBytesObject *__restrict self,
                 struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_reg_findall(DeeBytesObject *__restrict self,
                 struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_locateall(DeeBytesObject *__restrict self,
                   struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_re_split(DeeBytesObject *__restrict self,
               struct DeeRegexBaseExec const *__restrict exec);
#endif /* __INTELLISENSE__ */
#define bytes_rereplace_kwlist     string_rereplace_kwlist
#define bytes_generic_regex_kwlist generic_regex_kwlist
#define bytes_search_regex_kwlist  search_regex_kwlist

#define BYTES_GENERIC_REGEX_GETARGS_FMT(name) "o|" UNPdSIZ UNPdSIZ "o:" name
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
bytes_generic_regex_getargs(Bytes *self, size_t argc, DeeObject *const *argv,
                            DeeObject *kw, char const *__restrict fmt,
                            struct DeeRegexExec *__restrict result) {
	DeeObject *pattern, *rules = NULL;
	result->rx_startoff = 0;
	result->rx_endoff   = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, bytes_generic_regex_kwlist, fmt,
	                    &pattern, &result->rx_startoff, &result->rx_endoff,
	                    &rules))
		goto err;
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	result->rx_code = DeeString_GetRegex(pattern, DEE_REGEX_COMPILE_NOUTF8, rules);
	if unlikely(!result->rx_code)
		goto err;
	result->rx_nmatch = 0;
	result->rx_pmatch = NULL;
	result->rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rx_inbase = DeeBytes_DATA(self);
	result->rx_insize = DeeBytes_SIZE(self);
	if (result->rx_endoff > result->rx_insize)
		result->rx_endoff = result->rx_insize;
	return 0;
err:
	return -1;
}

#define BYTES_SEARCH_REGEX_GETARGS_FMT(name) "o|" UNPdSIZ UNPdSIZ UNPdSIZ "o:" name
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
bytes_search_regex_getargs(Bytes *self, size_t argc, DeeObject *const *argv,
                           DeeObject *kw, char const *__restrict fmt,
                           struct DeeRegexExecWithRange *__restrict result) {
	DeeObject *pattern, *rules = NULL;
	result->rewr_exec.rx_startoff = 0;
	result->rewr_exec.rx_endoff   = (size_t)-1;
	result->rewr_range            = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, bytes_search_regex_kwlist, fmt,
	                    &pattern, &result->rewr_exec.rx_startoff,
	                    &result->rewr_exec.rx_endoff,
	                    &rules))
		goto err;
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	result->rewr_exec.rx_code = DeeString_GetRegex(pattern, DEE_REGEX_COMPILE_NOUTF8, rules);
	if unlikely(!result->rewr_exec.rx_code)
		goto err;
	result->rewr_exec.rx_nmatch = 0;
	result->rewr_exec.rx_pmatch = NULL;
	result->rewr_exec.rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rewr_exec.rx_inbase = DeeBytes_DATA(self);
	result->rewr_exec.rx_insize = DeeBytes_SIZE(self);
	if (result->rewr_exec.rx_endoff > result->rewr_exec.rx_insize)
		result->rewr_exec.rx_endoff = result->rewr_exec.rx_insize;
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rematch(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("rematch"),
	                                        &exec))
		goto err;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regmatch(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("regmatch"),
	                                        &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rx_nmatch = exec.rx_code->rc_ngrps;
	exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH) {
		ReGroups_Free(groups);
		return_empty_seq;
	}
	groups->rg_groups[0].rm_so = 0;
	groups->rg_groups[0].rm_eo = (size_t)result;
	ReGroups_Init(groups, 1 + exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rematches(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("rematches"),
	                                        &exec))
		goto err;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_false;
	ASSERT((size_t)result <= exec.rx_endoff);
	return_bool((size_t)result >= exec.rx_endoff);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_refind(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("refind"),
	                                       &exec))
		goto err;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	match_size += (size_t)result;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, (size_t)result, (size_t)match_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rerfind(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("rerfind"),
	                                       &exec))
		goto err;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	match_size += (size_t)result;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, (size_t)result, (size_t)match_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regfind(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regfind"),
	                                       &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH) {
		ReGroups_Free(groups);
		return_empty_seq;
	}
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = (size_t)result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regrfind(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regrfind"),
	                                       &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH) {
		ReGroups_Free(groups);
		return_empty_seq;
	}
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = (size_t)result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_reindex(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("reindex"),
	                                       &exec))
		goto err;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		goto err_not_found;
	match_size += (size_t)result;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, (size_t)result, (size_t)match_size);
err_not_found:
	err_regex_index_not_found((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rerindex(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("rerindex"),
	                                       &exec))
		goto err;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		goto err_not_found;
	match_size += (size_t)result;
	return DeeTuple_Newf(PCKuSIZ PCKuSIZ, (size_t)result, (size_t)match_size);
err_not_found:
	err_regex_index_not_found((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regindex(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regindex"),
	                                       &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH)
		goto err_not_found;
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = (size_t)result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_not_found:
	err_regex_index_not_found((DeeObject *)self);
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regrindex(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("regrindex"),
	                                       &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH)
		goto err_not_found;
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = (size_t)result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_not_found:
	err_regex_index_not_found((DeeObject *)self);
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
bytes_relocate(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("relocate"),
	                                       &exec))
		goto err;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	return DeeBytes_NewSubView(self,
	                           (void *)((char const *)exec.rewr_exec.rx_inbase +
	                                    (size_t)result),
	                           match_size);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
bytes_rerlocate(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("rerlocate"),
	                                       &exec))
		goto err;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	return DeeBytes_NewSubView(self,
	                           (void *)((char const *)exec.rewr_exec.rx_inbase + (size_t)result),
	                           match_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_pack_partition_not_found(Bytes *__restrict self,
                               char const *__restrict bytes_base,
                               size_t startoff, size_t endoff) {
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto done;
	if (startoff < endoff) {
		DREF DeeObject *str0;
		str0 = DeeBytes_NewSubView(self,
		                           (void *)(bytes_base + startoff),
		                           endoff - startoff);
		if unlikely(!str0)
			goto err_r;
		DeeTuple_SET(result, 0, str0);
	} else {
		DeeTuple_SET(result, 0, Dee_EmptyString);
		Dee_Incref(Dee_EmptyString);
	}
	DeeTuple_SET(result, 1, Dee_EmptyString);
	DeeTuple_SET(result, 2, Dee_EmptyString);
	Dee_Incref_n(Dee_EmptyString, 2);
	return result;
done:
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bytes_pack_partition_found(Bytes *__restrict self,
                           char const *__restrict bytes_base,
                           size_t match_startoff,
                           size_t match_endoff,
                           size_t str_endoff) {
	DREF DeeObject *result;
	DREF DeeObject *str;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto done;
	str = DeeBytes_NewSubView(self, (void *)bytes_base, match_startoff);
	if unlikely(!str)
		goto err_r_0;
	DeeTuple_SET(result, 0, str);
	str = DeeBytes_NewSubView(self, (void *)(bytes_base + match_startoff),
	                          match_endoff - match_startoff);
	if unlikely(!str)
		goto err_r_1;
	DeeTuple_SET(result, 1, str);
	str = DeeBytes_NewSubView(self, (void *)(bytes_base + match_endoff),
	                          str_endoff - match_endoff);
	if unlikely(!str)
		goto err_r_2;
	DeeTuple_SET(result, 2, str);
	return result;
done:
	return result;
err_r_2:
	Dee_Decref_likely(DeeTuple_GET(result, 1));
err_r_1:
	Dee_Decref_likely(DeeTuple_GET(result, 0));
err_r_0:
	DeeTuple_FreeUninitialized(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_repartition(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("repartition"),
	                                       &exec))
		goto err;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH) {
		return bytes_pack_partition_not_found(self,
		                                      (char const *)exec.rewr_exec.rx_inbase,
		                                      exec.rewr_exec.rx_startoff,
		                                      exec.rewr_exec.rx_endoff);
	}
	result -= exec.rewr_exec.rx_startoff;
	exec.rewr_exec.rx_endoff -= exec.rewr_exec.rx_startoff;
	exec.rewr_exec.rx_inbase = (char const *)exec.rewr_exec.rx_inbase + exec.rewr_exec.rx_startoff;
	return bytes_pack_partition_found(self,
	                                  (char const *)exec.rewr_exec.rx_inbase,
	                                  (size_t)result,
	                                  (size_t)result + match_size,
	                                  exec.rewr_exec.rx_endoff);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_rerpartition(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(bytes_search_regex_getargs(self, argc, argv, kw,
	                                       BYTES_SEARCH_REGEX_GETARGS_FMT("rerpartition"),
	                                       &exec))
		goto err;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH) {
		return bytes_pack_partition_not_found(self,
		                                      (char const *)exec.rewr_exec.rx_inbase,
		                                      exec.rewr_exec.rx_startoff,
		                                      exec.rewr_exec.rx_endoff);
	}
	result -= exec.rewr_exec.rx_startoff;
	exec.rewr_exec.rx_endoff -= exec.rewr_exec.rx_startoff;
	exec.rewr_exec.rx_inbase = (char const *)exec.rewr_exec.rx_inbase + exec.rewr_exec.rx_startoff;
	return bytes_pack_partition_found(self,
	                                  (char const *)exec.rewr_exec.rx_inbase,
	                                  (size_t)result,
	                                  (size_t)result + match_size,
	                                  exec.rewr_exec.rx_endoff);
err:
	return NULL;
}


#ifndef CONFIG_HAVE_memsetp
#define CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */

PRIVATE WUNUSED NONNULL((1)) DREF Bytes *DCALL
bytes_rereplace(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct bytes_printer printer = BYTES_PRINTER_INIT;
	DeeObject *pattern, *replace, *rules = NULL;
	struct DeeRegexMatch groups[9];
	size_t maxreplace = (size_t)-1;
	char const *replace_start, *replace_end;
	struct DeeRegexExec exec;
	if (DeeArg_UnpackKw(argc, argv, kw, bytes_rereplace_kwlist,
	                    "oo|" UNPuSIZ "o:rereplace",
	                    &pattern, &replace, &maxreplace, &rules))
		goto err;
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(replace, &DeeString_Type))
		goto err;
	replace_start = DeeString_AsUtf8(replace);
	if unlikely(!replace_start)
		goto err;
	replace_end = replace_start + WSTR_LENGTH(replace_start);
	exec.rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	exec.rx_code = DeeString_GetRegex(pattern, DEE_REGEX_COMPILE_NOUTF8, rules);
	if unlikely(!exec.rx_code)
		goto err;
	exec.rx_nmatch = COMPILER_LENOF(groups);
	exec.rx_pmatch = groups;
	exec.rx_inbase = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!exec.rx_inbase)
		goto err;
	exec.rx_insize   = WSTR_LENGTH(exec.rx_inbase);
	exec.rx_startoff = 0;
	exec.rx_endoff   = exec.rx_insize;
	while (exec.rx_startoff < exec.rx_endoff && maxreplace) {
		char const *replace_iter, *replace_flush;
		Dee_ssize_t match_offset;
		size_t match_size;
		memsetp(groups, (void *)(uintptr_t)(size_t)-1, 2 * 9);
		match_offset = DeeRegex_Search(&exec, (size_t)-1, &match_size);
		if unlikely(match_offset == DEE_RE_STATUS_ERROR)
			goto err;
		if (match_offset == DEE_RE_STATUS_NOMATCH)
			break;
		if unlikely(match_size == 0)
			break; /* Prevent infinite loop when epsilon was matched. */
		/* Flush until start-of-match. */
		if unlikely(bytes_printer_print(&printer,
		                                (char const *)exec.rx_inbase + exec.rx_startoff,
		                                (size_t)match_offset - exec.rx_startoff) < 0)
			goto err;

		/* Parse and print the replacement bytes. */
		for (replace_iter = replace_flush = replace_start;
		     replace_iter < replace_end;) {
			struct DeeRegexMatch match;
			char ch = *replace_iter;
			switch (ch) {

			case '&':
				/* Insert full match. */
				match.rm_so = (size_t)match_offset;
				match.rm_eo = (size_t)match_offset + match_size;
do_insert_match:
				if unlikely(bytes_printer_print(&printer, replace_flush,
				                                (size_t)(replace_iter - replace_flush)) < 0)
					goto err;
				if unlikely(bytes_printer_print(&printer,
				                                (char const *)exec.rx_inbase + match.rm_so,
				                                (size_t)(match.rm_eo - match.rm_so)) < 0)
					goto err;
				++replace_iter;
				if (ch == '\\')
					++replace_iter;
				replace_flush = replace_iter;
				break;

			case '\\':
				ch = replace_iter[1];
				if (ch == '&' || ch == '\\') {
					/* Insert literal '&' or '\' */
					if unlikely(bytes_printer_print(&printer, replace_flush,
					                                (size_t)(replace_iter - replace_flush)) < 0)
						goto err;
					++replace_iter;
					replace_flush = replace_iter;
					++replace_iter;
				} else if (ch >= '1' && ch <= '9') {
					/* Insert matched group N.
					 * NOTE: When the group was never matched, both of its offsets will be equal
					 *       here, meaning that the code above will simply print an empty bytes! */
					match = groups[ch - '1'];
					goto do_insert_match;
				} else {
					++replace_iter;
				}
				break;

			default:
				++replace_iter;
				break;
			}
		}

		/* Flush remainder of replacement bytes. */
		if (replace_flush < replace_end) {
			if unlikely(bytes_printer_print(&printer, replace_flush,
			                                (size_t)(replace_end - replace_flush)) < 0)
				goto err;
		}

		/* Keep on scanning after the matched area. */
		exec.rx_startoff = (size_t)match_offset + match_size;
		--maxreplace;
	}

	/* Flush remainder */
#ifndef __OPTIMIZE_SIZE__
	if unlikely(exec.rx_startoff == 0) {
		bytes_printer_fini(&printer);
		return_reference_(self);
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(bytes_printer_print(&printer,
	                                (char const *)exec.rx_inbase + exec.rx_startoff,
	                                exec.rx_endoff - exec.rx_startoff) < 0)
		goto err;
	return (DREF Bytes *)bytes_printer_pack(&printer);
err:
	bytes_printer_fini(&printer);
	return NULL;
}

#define BYTES_BASE_REGEX_GETARGS_FMT BYTES_GENERIC_REGEX_GETARGS_FMT
#define bytes_base_regex_kwlist      bytes_generic_regex_kwlist
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
bytes_base_regex_getargs(Bytes *self, size_t argc, DeeObject *const *argv,
                         DeeObject *kw, char const *__restrict fmt,
                         struct DeeRegexBaseExec *__restrict result) {
	DeeObject *rules = NULL;
	result->rx_startoff = 0;
	result->rx_endoff   = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, bytes_base_regex_kwlist, fmt,
	                    &result->rx_pattern,
	                    &result->rx_startoff,
	                    &result->rx_endoff,
	                    &rules))
		goto err;
	if (DeeObject_AssertTypeExact(result->rx_pattern, &DeeString_Type))
		goto err;
	result->rx_code = DeeString_GetRegex((DeeObject *)result->rx_pattern,
	                                     DEE_REGEX_COMPILE_NOUTF8, rules);
	if unlikely(!result->rx_code)
		goto err;
	result->rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rx_inbase = DeeBytes_DATA(self);
	result->rx_insize = DeeBytes_SIZE(self);
	if (result->rx_endoff > result->rx_insize)
		result->rx_endoff = result->rx_insize;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_refindall(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("refindall"),
	                                     &exec))
		goto err;
	return bytes_re_findall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_regfindall(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("regfindall"),
	                                     &exec))
		goto err;
	return bytes_reg_findall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_relocateall(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("relocateall"),
	                                     &exec))
		goto err;
	return bytes_re_locateall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_resplit(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(bytes_base_regex_getargs(self, argc, argv, kw,
	                                     BYTES_BASE_REGEX_GETARGS_FMT("resplit"),
	                                     &exec))
		goto err;
	return bytes_re_split(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_restartswith(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("restartswith"),
	                                        &exec))
		goto err;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	return_bool(result != DEE_RE_STATUS_NOMATCH);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_reendswith(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("reendswith"),
	                                        &exec))
		goto err;
	result = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if unlikely(result == DEE_RE_STATUS_NOMATCH)
		return_false;
	ASSERT((size_t)result + match_size <= exec.rx_endoff);
	return_bool((size_t)result + match_size >= exec.rx_endoff);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_relstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("relstrip"),
	                                        &exec))
		goto err;
	for (;;) {
		Dee_ssize_t result;
		result = DeeRegex_Match(&exec);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if (result == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		exec.rx_startoff = (size_t)result;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	return (DREF Bytes *)DeeBytes_NewSubView(self,
	                                         (void *)((char const *)exec.rx_inbase + exec.rx_startoff),
	                                         exec.rx_endoff - exec.rx_startoff);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_rerstrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("rerstrip"),
	                                        &exec))
		goto err;
	for (;;) {
		size_t match_size;
		Dee_ssize_t result;
		result = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if (match_size == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		ASSERT((size_t)result + match_size <= exec.rx_endoff);
		if ((size_t)result + match_size < exec.rx_endoff)
			break;
		exec.rx_endoff = (size_t)result;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	return (DREF Bytes *)DeeBytes_NewSubView(self,
	                                         (void *)((char const *)exec.rx_inbase + exec.rx_startoff),
	                                         exec.rx_endoff - exec.rx_startoff);
err:
	return NULL;
}

PRIVATE WUNUSED DREF Bytes *DCALL
bytes_restrip(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("restrip"),
	                                        &exec))
		goto err;
	/* lstrip */
	for (;;) {
		Dee_ssize_t result;
		result = DeeRegex_Match(&exec);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if (result == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		exec.rx_startoff = (size_t)result;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* rstrip */
	for (;;) {
		size_t match_size;
		Dee_ssize_t result;
		result = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if (match_size == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		ASSERT((size_t)result + match_size <= exec.rx_endoff);
		if ((size_t)result + match_size < exec.rx_endoff)
			break;
		exec.rx_endoff = (size_t)result;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	return (DREF Bytes *)DeeBytes_NewSubView(self,
	                                         (void *)((char const *)exec.rx_inbase + exec.rx_startoff),
	                                         exec.rx_endoff - exec.rx_startoff);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_recontains(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("recontains"),
	                                        &exec))
		goto err;
	result = DeeRegex_Search(&exec, (size_t)-1, NULL);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	return_bool(result != DEE_RE_STATUS_NOMATCH);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bytes_recount(Bytes *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t count;
	struct DeeRegexExec exec;
	if unlikely(bytes_generic_regex_getargs(self, argc, argv, kw,
	                                        BYTES_GENERIC_REGEX_GETARGS_FMT("recount"),
	                                        &exec))
		goto err;
	count = 0;
	for (;;) {
		Dee_ssize_t result;
		size_t match_size;
		result = DeeRegex_Search(&exec, (size_t)-1, &match_size);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if unlikely(match_size == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		++count;
		exec.rx_startoff = (size_t)result + match_size;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}
	return DeeInt_NewSize(count);
err:
	return NULL;
}




INTDEF struct type_method tpconst bytes_methods[];
INTERN struct type_method tpconst bytes_methods[] = {
	{ "decode",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&string_decode,
	  DOC("(codec:?Dstring,errors=!Pstrict)->?X2?Dstring?O\n"
	      "@throw ValueError The given @codec or @errors wasn't recognized\n"
	      "@throw UnicodeDecodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
	      "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
	      "Same as ?Adecode?Dstring, but instead use the data of @this ?. object as characters to decode"),
	  TYPE_METHOD_FKWDS },
	{ "encode",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&string_encode,
	  DOC("(codec:?Dstring,errors=!Pstrict)->?X3?.?Dstring?O\n"
	      "@throw ValueError The given @codec or @errors wasn't recognized\n"
	      "@throw UnicodeEncodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
	      "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
	      "Same as ?Aencode?Dstring, but instead use the data of @this ?. object as characters to decode"),
	  TYPE_METHOD_FKWDS },
	{ "bytes",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_substr,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Same as ?#substr (here for ABI compatibility with ?Abytes?Dstring)"),
	  TYPE_METHOD_FKWDS },
	{ "ord",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_ord,
	  DOC("->?Dint\n"
	      "@throw ValueError The length of @this ?. object is not equal to $1\n"
	      "Same as ${this[0]}\n"

	      "\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw IntegerOverflow The given @index is lower than $0\n"
	      "@throw IndexError The given @index is greater than ${##this}\n"
	      "Same as ${this[index]}") },

	/* Bytes-specific functions. */
	{ "resized",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_resized,
	  DOC("(new_size:?Dint,filler?:?Dint)->?.\n"
	      "Return a new writable ?. object with a length of @new_size, and its "
	      /**/ "first ${(##this, new_size) < ...} bytes initialized from ${this.substr(0, new_size)}, "
	      /**/ "with the remainder then either left uninitialized, or initialized to @filler\n"
	      "Note that because a ?. object cannot be resized in-line, code using this function "
	      /**/ "must make use of the returned ?. object:\n"
	      "${"
	      "local x = \"foobar\";\n"
	      "local y = x.bytes();\n"
	      "print repr y; /* \"foobar\" */\n"
	      "y = y.resized(16, \"?\".ord());\n"
	      "print repr y; /* \"foobar??" "??" "??" "??" "??\" */"
	      "}") },
	{ "reverse",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_reverse,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Same as ?#reversed, but modifications are performed "
	      /**/ "in-line, before @this ?. object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "makereadonly",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_makereadonly,
	  DOC("->?.\n"
	      "The inverse of ?#makewritable, either re-returning @this ?. object if it "
	      /**/ "already is read-only, or construct a view for the data contained within @this "
	      /**/ "?. object, but making that view read-only") },
	{ "makewritable",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_makewritable,
	  DOC("->?.\n"
	      "Either re-return @this ?. object is it already ?#iswritable, or create a "
	      /**/ "copy (s.a. ?#{op:copy}) and return it:\n"
	      "${"
	      "function makewritable() {\n"
	      "	if (this.iswritable)\n"
	      "		return this;\n"
	      "	return copy this;\n"
	      "}}") },
	{ "hex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_hex,
	  DOC("(start=!0,end=!-1)->?Dstring\n"
	      "Returns a hex-encoded string for the bytes contained within "
	      /**/ "${this.substr(start, end)}, that is a string containing 2 characters "
	      /**/ "for each encoded byte, both of which are lower-case hexadecimal digits\n"
	      "The returned string is formatted such that ?#fromhex can be used to decode "
	      /**/ "it into another ?. object"),
	  TYPE_METHOD_FKWDS },

	/* Bytes formatting / scanning. */
	{ DeeString_STR(&str_format),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_format,
	  DOC("(args:?S?O)->?.") },
	{ "scanf",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_scanf,
	  DOC("(format:?Dstring)->?S?O") },

	/* String/Character traits */
#define DEFINE_BYTES_TRAIT_EX(name, func, doc)                                  \
	{ name,                                                                      \
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&func, \
	  DOC("->?Dbool\n"                                                           \
	      "(index:?Dint)->?Dbool\n"                                              \
	      "(start:?Dint,end:?Dint)->?Dbool\n"                                    \
	      "@throw IndexError The given @index is larger than ${##this}\n"        \
	      "@throw IntegerOverflow The given @index is negative or too large\n"   \
	      doc) }
#define DEFINE_BYTES_TRAIT(name, func, are_xxx) \
	DEFINE_BYTES_TRAIT_EX(name, func, "Returns ?t if $this, ${this[index]}, or all characters in ${this.substr(start, end)} " are_xxx)
	DEFINE_BYTES_TRAIT("iscntrl", bytes_iscntrl, "are control characters"),
	DEFINE_BYTES_TRAIT("istab", bytes_istab, "are tabulator characters (#C{U+0009}, #C{U+000B}, #C{U+000C}, ...)"),
	DEFINE_BYTES_TRAIT("iscempty", bytes_iscempty, "are tabulator (?#istab) or white-space (?#iswhite) characters (alias for ?#isspacexlf)"),
	DEFINE_BYTES_TRAIT("iswhite", bytes_iswhite, "are white-space characters (#C{U+0020}, ...)"),
	DEFINE_BYTES_TRAIT("islf", bytes_islf, "are line-feeds"),
	DEFINE_BYTES_TRAIT("isspace", bytes_isspace, "are space-characters"),
	DEFINE_BYTES_TRAIT("islower", bytes_islower, "are lower-case"),
	DEFINE_BYTES_TRAIT("isupper", bytes_isupper, "are upper-case"),
	DEFINE_BYTES_TRAIT("isalpha", bytes_isalpha, "are alphabetical"),
	DEFINE_BYTES_TRAIT("isdigit", bytes_isdigit, "are digits"),
	DEFINE_BYTES_TRAIT("ishex", bytes_ishex, "are alphabetical hex-characters (#C{U+0041-U+0046}, #C{U+0061-U+0066})"),
	DEFINE_BYTES_TRAIT("isxdigit", bytes_isxdigit, "are digit (?#isdigit) or alphabetical hex-characters (?#ishex)"),
	DEFINE_BYTES_TRAIT("isalnum", bytes_isalnum, "are alpha-numerical"),
	DEFINE_BYTES_TRAIT("ispunct", bytes_ispunct, "are punctuational characters"),
	DEFINE_BYTES_TRAIT("isgraph", bytes_isgraph, "are graphical characters"),
	DEFINE_BYTES_TRAIT("isprint", bytes_isprint, "are printable"),
	DEFINE_BYTES_TRAIT("isblank", bytes_isblank, "are blank"),
	DEFINE_BYTES_TRAIT("isnumeric", bytes_isnumeric, "qualify as digits or otherwise numeric characters"),
	DEFINE_BYTES_TRAIT("issymstrt", bytes_issymstrt, "can be used to start a symbol name"),
	DEFINE_BYTES_TRAIT("issymcont", bytes_issymcont, "can be used to continue a symbol name"),
	DEFINE_BYTES_TRAIT("isspacexlf", bytes_iscempty, "are space-characters, where linefeeds are not considered as spaces (IsSpaceeXcludingLineFeed) (alias for ?#iscempty)"),
	DEFINE_BYTES_TRAIT("isascii", bytes_isascii, "are ascii-characters, that is have an ordinal value ${<= 0x7f}"),
	{ "istitle",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_istitle,
	  DOC("(index:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${?#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns ?t if the character at ${this[index]} has title-casing\n"

	      "\n"
	      "->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "Returns ?t if $this, or the sub-bytes ${this.substr(start, end)} "
	      /**/ "follows title-casing, meaning that space is followed by title-case, "
	      /**/ "with all remaining characters not being title-case") },
	{ "issymbol",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_issymbol,
	  DOC("(index:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${?#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns ?t if the character at ${this[index]} can be used "
	      /**/ "to start a symbol name. Same as ${this.issymstrt(index)}\n"

	      "\n"
	      "->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "Returns ?t if $this, or the sub-bytes ${this.substr(start, end)} "
	      /**/ "is a valid symbol name") },
#undef DEFINE_BYTES_TRAIT

#define DEFINE_ANY_BYTES_TRAIT_EX(name, func, doc)                               \
	{ name,                                                                      \
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&func, \
	  DOC("(start=!0,end=!-1)->?Dbool\n"                                         \
	      doc),                                                                  \
	  TYPE_METHOD_FKWDS }
#define DEFINE_ANY_BYTES_TRAIT(name, func, is_xxx) \
	DEFINE_ANY_BYTES_TRAIT_EX(name, func, "Returns ?t if any character in ${this.substr(start, end)} " is_xxx)
	DEFINE_ANY_BYTES_TRAIT("isanycntrl", bytes_isanycntrl, "is a control character"),
	DEFINE_ANY_BYTES_TRAIT("isanytab", bytes_isanytab, "is a tabulator character (#C{U+0009}, #C{U+000B}, #C{U+000C}, ...)"),
	DEFINE_ANY_BYTES_TRAIT("isanycempty", bytes_isanycempty, "is a tabulator (?#istab) or white-space (?#iswhite) character (alias for ?#isanyspacexlf)"),
	DEFINE_ANY_BYTES_TRAIT("isanywhite", bytes_isanywhite, "is a white-space character (#C{U+0020}, ...)"),
	DEFINE_ANY_BYTES_TRAIT("isanylf", bytes_isanylf, "is a line-feed"),
	DEFINE_ANY_BYTES_TRAIT("isanyspace", bytes_isanyspace, "is a space character"),
	DEFINE_ANY_BYTES_TRAIT("isanylower", bytes_isanylower, "is lower-case"),
	DEFINE_ANY_BYTES_TRAIT("isanyupper", bytes_isanyupper, "is upper-case"),
	DEFINE_ANY_BYTES_TRAIT("isanyalpha", bytes_isanyalpha, "is alphabetical"),
	DEFINE_ANY_BYTES_TRAIT("isanydigit", bytes_isanydigit, "is a digit"),
	DEFINE_ANY_BYTES_TRAIT("isanyhex", bytes_isanyhex, "is an alphabetical hex-character (#C{U+0041-U+0046}, #C{U+0061-U+0066})"),
	DEFINE_ANY_BYTES_TRAIT("isanyxdigit", bytes_isanyxdigit, "is a digit (?#isdigit) or an alphabetical hex-character (?#ishex)"),
	DEFINE_ANY_BYTES_TRAIT("isanyalnum", bytes_isanyalnum, "is alpha-numerical"),
	DEFINE_ANY_BYTES_TRAIT("isanypunct", bytes_isanypunct, "is a punctuational character"),
	DEFINE_ANY_BYTES_TRAIT("isanygraph", bytes_isanygraph, "is a graphical character"),
	DEFINE_ANY_BYTES_TRAIT("isanyprint", bytes_isanyprint, "is printable"),
	DEFINE_ANY_BYTES_TRAIT("isanyblank", bytes_isanyblank, "is blank"),
	DEFINE_ANY_BYTES_TRAIT("isanytitle", bytes_isanytitle, "has title-casing"),
	DEFINE_ANY_BYTES_TRAIT("isanynumeric", bytes_isanynumeric, "qualifies as digit or some other numeric character"),
	DEFINE_ANY_BYTES_TRAIT("isanysymstrt", bytes_isanysymstrt, "can be used to start a symbol name"),
	DEFINE_ANY_BYTES_TRAIT("isanysymcont", bytes_isanysymcont, "can be used to continue a symbol name"),
	DEFINE_ANY_BYTES_TRAIT("isanyspacexlf", bytes_isanycempty, "is a space character, where linefeeds are not considered as spaces (IsSpaceeXcludingLineFeed) (alias for ?#isanycempty)"),
	{ "isanyascii",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_isanyascii,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns ?t if any character in ${this.substr(start, end)} is "
	      /**/ "an ascii character, that is has an ordinal value ${<= 0x7f}"),
	  TYPE_METHOD_FKWDS },
#undef DEFINE_ANY_BYTES_TRAIT
#undef DEFINE_ANY_BYTES_TRAIT_EX

	{ "asnumeric",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_asdigit,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Alias for ?#asdigit") },
	{ "asdigit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_asdigit,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Returns the digit value of the byte at the specific index") },
	{ "asxdigit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_asxdigit,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Same as ?#asdigit, but also accepts #C{a-f} and #C{A-F}") },

	/* Bytes conversion functions */
	{ "lower",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_lower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this ?. object converted to lower-case (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "upper",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_upper,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this ?. object converted to upper-case (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "title",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_title,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this ?. object converted to title-casing (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "capitalize",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_capitalize,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this ?. object with each word capitalized (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "swapcase",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_swapcase,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns a writable copy of @this ?. object with the casing of each "
	      /**/ "character that has two different casings swapped (when interpreted as ASCII)"),
	  TYPE_METHOD_FKWDS },
	{ "casefold",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_lower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Alias for ?#{lower}. This function exists to match ?Acasefold?Dstring in "
	      /**/ "order to improve binary compatibility between ?. and ?Dstring objects"),
	  TYPE_METHOD_FKWDS },

	/* Inplace variants of bytes conversion functions */
	{ "tolower",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_tolower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Same as ?#lower, but character modifications are performed in-place, and @this ?. object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "toupper",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_toupper,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Same as ?#upper, but character modifications are performed in-place, and @this ?. object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "totitle",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_totitle,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Same as ?#title, but character modifications are performed in-place, and @this ?. object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "tocapitalize",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_tocapitalize,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Same as ?#capitalize, but character modifications are performed in-place, and @this ?. object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "toswapcase",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_toswapcase,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Same as ?#swapcase, but character modifications are performed in-place, and @this ?. object is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "tocasefold",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_tolower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Alias for ?#tolower, here to coincide with ?#casefold existing as an alias for ?#lower"),
	  TYPE_METHOD_FKWDS },

	/* Case-sensitive query functions */
	{ DeeString_STR(&str_replace),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_replace,
	  DOC("(find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@throw ValueError The given @find or @replace is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @find or @replace is an integer lower than $0, or greater than $0xff\n"
	      "Find up to @max occurrences of @find and replace each with @replace, then return the resulting data as a writable ?. object"),
	  TYPE_METHOD_FKWDS },
	{ "toreplace",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_toreplace,
	  DOC("(find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@throw ValueError The given @find or @replace is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @find or @replace is an integer lower than $0, or greater than $0xff\n"
	      "@throw ValueError The number of bytes specified by @find and @replace are not identical\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Same as ?#replace, but the ?. object is modified in-place, and @this is re-returned"),
	  TYPE_METHOD_FKWDS },
	{ "find",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_find,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return its starting index, or ${-1} if no such position exists"),
	  TYPE_METHOD_FKWDS },
	{ "rfind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rfind,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return its starting index, or ${-1} if no such position exists"),
	  TYPE_METHOD_FKWDS },
	{ "index",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_index,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start, end)}\n"
	      "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return its starting index"),
	  TYPE_METHOD_FKWDS },
	{ "rindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rindex,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start, end)}\n"
	      "Find the last instance of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return its starting index"),
	  TYPE_METHOD_FKWDS },
	{ "findany",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_findany,
	  DOC("(needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?Dint?N\n"
	      "@throw ValueError One of the given @needles is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return its starting index, or ${-1} if no such position exists"),
	  TYPE_METHOD_FKWDS },
	{ "rfindany",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rfindany,
	  DOC("(needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?Dint?N\n"
	      "@throw ValueError One of the given @needles is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return its starting index, or ${-1} if no such position exists"),
	  TYPE_METHOD_FKWDS },
	{ "indexany",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_indexany,
	  DOC("(needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?Dint?N\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start, end)}\n"
	      "Find the first instance of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return its starting index"),
	  TYPE_METHOD_FKWDS },
	{ "rindexany",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rindexany,
	  DOC("(needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?Dint?N\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start, end)}\n"
	      "Find the last instance of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return its starting index"),
	  TYPE_METHOD_FKWDS },
	{ "findall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_findall,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?S?Dint\n"
	      "Find all instances of @needle within ${this.substr(start, end)}, "
	      /**/ "and return their starting indices as a sequence"),
	  TYPE_METHOD_FKWDS },
	{ "count",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_count,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "Count the number of instances of @needle that exists within ${this.substr(start, end)}, "
	      /**/ "and return now many were found"),
	  TYPE_METHOD_FKWDS },
	{ "contains",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_contains_f,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Check if @needle can be found within ${this.substr(start, end)}, and return a boolean indicative of that"),
	  TYPE_METHOD_FKWDS },
	{ "substr",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_substr,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Similar to ${this[start:end]}, and semantically equialent to ?Asubstr?Dstring\n"
	      "This function can be used to view a sub-set of bytes from @this ?. object\n"
	      "Modifications then made to the returned ?. object will affect the same memory already described by @this ?. object"),
	  TYPE_METHOD_FKWDS },
	{ "strip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_strip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all leading and trailing whitespace-characters, or "
	      /**/ "characters apart of @mask, and return a sub-view of @this ?. object") },
	{ "lstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_lstrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all leading whitespace-characters, or characters "
	      /**/ "apart of @mask, and return a sub-view of @this ?. object") },
	{ "rstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rstrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all trailing whitespace-characters, or characters "
	      /**/ "apart of @mask, and return a sub-view of @this ?. object") },
	{ "sstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_sstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all leading and trailing instances of @other from @this string\n"
	      "${"
	      "local result = this;\n"
	      "while (result.startswith(other))\n"
	      "	result = result[##other:];\n"
	      "while (result.endswith(other))\n"
	      "	result = result[:##result - ##other];"
	      "}") },
	{ "lsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_lsstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all leading instances of @other from @this string\n"
	      "${"
	      "local result = this;\n"
	      "while (result.startswith(other))\n"
	      "	result = result[##other:];"
	      "}") },
	{ "rsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rsstrip,
	  DOC("(other:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all trailing instances of @other from @this string\n"
	      "${"
	      "local result = this;\n"
	      "while (result.endswith(other))\n"
	      "	result = result[:##result - ##other];"
	      "}") },
	{ "striplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_striplines,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all whitspace, or @mask characters at the start, end, and before/after linefeeds\n"
	      "Note that for this purpose, linefeed characters don't count as whitespace\n"
	      "aka: strip all leading and trailing whitespace\n"
	      "Similar to ${\"\".bytes().join(this.splitlines(true).each.strip())}") },
	{ "lstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_lstriplines,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all whitspace, or @mask characters at the start, and after linefeeds\n"
	      "Note that for this purpose, linefeed characters don't count as whitespace\n"
	      "aka: strip all leading whitespace\n"
	      "Similar to ${\"\".bytes().join(this.splitlines(true).each.lstrip())}") },
	{ "rstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rstriplines,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Strip all whitspace, or @mask characters at the end, and before linefeeds\n"
	      "Note that for this purpose, linefeed characters don't count as whitespace\n"
	      "aka: strip all trailing whitespace\n"
	      "Similar to ${\"\".bytes().join(this.splitlines(true).each.rstrip())}") },
	{ "sstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_sstriplines,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#striplines, but sequence for complete sequences of #needle, rather "
	      /**/ "than bytes apart of its $mask character.") },
	{ "lsstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_lsstriplines,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#lstriplines, but sequence for complete sequences of #needle, rather "
	      /**/ "than bytes apart of its $mask character.") },
	{ "rsstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rsstriplines,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#rstriplines, but sequence for complete sequences of #needle, rather "
	      /**/ "than bytes apart of its $mask character.") },
	{ "startswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_startswith,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Return ?t if the sub-string ${this.substr(start, end)} starts with @other"),
	  TYPE_METHOD_FKWDS },
	{ "endswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_endswith,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Return ?t if the sub-string ${this.substr(start, end)} ends with @other"),
	  TYPE_METHOD_FKWDS },
	{ "partition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_parition,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Search for the first instance of @needle within ${this.substr(start, end)} and "
	      /**/ "return a 3-element sequence of byte objects ${(this[:pos], needle, this[pos + ##needle:])}.\n"
	      "If @needle could not be found, ${(this, \"\".bytes(), \"\".bytes())} is returned"),
	  TYPE_METHOD_FKWDS },
	{ "rpartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rparition,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Search for the last instance of @needle within ${this.substr(start, end)} and "
	      "return a 3-element sequence of strings ${(this[:pos], needle, this[pos + ##needle:])}.\n"
	      "If @needle could not be found, ${(this, \"\".bytes(), \"\".bytes())} is returned"),
	  TYPE_METHOD_FKWDS },
	{ "compare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_compare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Compare the sub-string ${left = this.substr(my_start, my_end)} with ${right = other.substr(other_start, other_end)}, "
	      /**/ "returning ${< 0} if ${left < right}, ${> 0} if ${left > right}, or ${== 0} if they are equal") },
	{ "vercompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_vercompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Performs a version-string comparison. This is similar to ?#compare, but rather than "
	      /**/ "performing a strict lexicographical comparison, the numbers found in the strings "
	      /**/ "being compared are compared as a whole, solving the common problem seen in applications "
	      /**/ "such as file navigators showing a file order of $\"foo1.txt\", $\"foo10.txt\", "
	      /**/ "$\"foo11.txt\", $\"foo2.txt\", etc...\n"
	      "This function is a portable implementation of the GNU function "
	      /**/ "#A{strverscmp|https://linux.die.net/man/3/strverscmp}, "
	      /**/ "for which you may follow the link for further details") },
	{ "wildcompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_wildcompare,
	  DOC("(pattern:?X2?.?Dstring,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "(my_start:?Dint,pattern:?X2?.?Dstring,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?X2?.?Dstring,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "Perform a wild-character-enabled comparising of the sub-string ${left = this.substr(my_start,my_end)} "
	      /**/ "with ${right = pattern.substr(pattern_start, pattern_end)}, returning ${< 0} if ${left < right}, ${> 0} "
	      /**/ "if ${left > right}, or ${== 0} if they are equal\n"
	      "Wild-compare characters are only parsed from @pattern, allowing $\"?\" to "
	      /**/ "be matched with any single character from @this, and $\"*\" to be matched to "
	      /**/ "any number of characters") },
	{ "fuzzycompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_fuzzycompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Perform a fuzzy string comparison between ${this.substr(my_start,my_end)} and ${other.substr(other_start,other_end)}\n"
	      "The return value is a similarty-factor that can be used to score how close the two strings look alike.\n"
	      "How exactly the scoring is done is implementation-specific, however a score of $0 is reserved for two "
	      /**/ "strings that are perfectly identical, any two differing strings always have a score ${> 0}, and the closer "
	      /**/ "the score is to $0, the more alike they are\n"
	      "The intended use of this function is for auto-completion, as well as warning "
	      /**/ "messages and recommendations in the sense of I-dont-know-foo-but-did-you-mean-bar\n"
	      "Note that there is another version ?#casefuzzycompare that also ignores casing") },
	{ "wmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_wmatch,
	  DOC("(pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "Same as ?#wildcompare, returning ?t where ?#wildcompare would return $0, and ?f in all pattern cases") },

	/* Case-insensitive query functions */
	{ "casereplace",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casereplace,
	  DOC("(find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@throw ValueError The given @find or @replace is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @find or @replace is an integer lower than $0, or greater than $0xff\n"
	      "Same as ?#replace, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "tocasereplace",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_tocasereplace,
	  DOC("(find:?X3?.?Dstring?Dint,replace:?X3?.?Dstring?Dint,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@throw ValueError The given @find or @replace is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @find or @replace is an integer lower than $0, or greater than $0xff\n"
	      "@throw ValueError The number of bytes specified by @find and @replace are not identical\n"
	      "@throw BufferError @this ?. object is not writable\n"
	      "Same as ?#toreplace, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casefind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casefind,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Same as ?#find, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}\n"
	      "If no match if found, ?N is returned"),
	  TYPE_METHOD_FKWDS },
	{ "caserfind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserfind,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Same as ?#rfind, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}\n"
	      "If no match if found, ?N is returned"),
	  TYPE_METHOD_FKWDS },
	{ "caseindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caseindex,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start, end)}\n"
	      "Same as ?#index, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	  TYPE_METHOD_FKWDS },
	{ "caserindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserindex,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start, end)}\n"
	      "Same as ?#rindex, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	  TYPE_METHOD_FKWDS },
	{ "casefindany",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casefindany,
	  DOC("(needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Same as ?#findany, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}\n"
	      "If no match if found, ?N is returned"),
	  TYPE_METHOD_FKWDS },
	{ "caserfindany",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserfindany,
	  DOC("(needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "@throw ValueError The given @needle is a string containing characters ${> 0xff}\n"
	      "@throw IntegerOverflow The given @needle is an integer lower than $0, or greater than $0xff\n"
	      "Same as ?#rfindany, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}\n"
	      "If no match if found, ?N is returned"),
	  TYPE_METHOD_FKWDS },
	{ "caseindexany",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caseindexany,
	  DOC("(needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start, end)}\n"
	      "Same as ?#indexany, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	  TYPE_METHOD_FKWDS },
	{ "caserindexany",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserindexany,
	  DOC("(needles:?S?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start, end)}\n"
	      "Same as ?#rindexany, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	  TYPE_METHOD_FKWDS },
	{ "casefindall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casefindall,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?S?T2?Dint?Dint\n"
	      "Same as ?#findall, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ##needle}"),
	  TYPE_METHOD_FKWDS },
	{ "casecount",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casecount,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "Same as ?#count, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casecontains",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casecontains_f,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Same as ?#contains, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casestrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casestrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#strip, however ascii-casing is ignored during character comparisons") },
	{ "caselstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caselstrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#lstrip, however ascii-casing is ignored during character comparisons") },
	{ "caserstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserstrip,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#rstrip, however ascii-casing is ignored during character comparisons") },
	{ "casesstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casesstrip,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#sstrip, however ascii-casing is ignored during character comparisons") },
	{ "caselsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caselsstrip,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#lsstrip, however ascii-casing is ignored during character comparisons") },
	{ "casersstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casersstrip,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#rsstrip, however ascii-casing is ignored during character comparisons") },
	{ "casestriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casestriplines,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#striplines, however ascii-casing is ignored during character comparisons") },
	{ "caselstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caselstriplines,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#lstriplines, however ascii-casing is ignored during character comparisons") },
	{ "caserstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserstriplines,
	  DOC("(mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#rstriplines, however ascii-casing is ignored during character comparisons") },
	{ "casesstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casesstriplines,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#sstriplines, however ascii-casing is ignored during character comparisons") },
	{ "caselsstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caselsstriplines,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#lsstriplines, however ascii-casing is ignored during character comparisons") },
	{ "casersstriplines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casersstriplines,
	  DOC("(needle:?X3?.?Dstring?Dint)->?.\n"
	      "Same as ?#rsstriplines, however ascii-casing is ignored during character comparisons") },
	{ "casestartswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casestartswith,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Same as ?#startswith, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "caseendswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caseendswith,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dbool\n"
	      "Same as ?#endswith, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casepartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caseparition,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as ?#partition, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "caserpartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserparition,
	  DOC("(needle:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as ?#rpartition, however ascii-casing is ignored during character comparisons"),
	  TYPE_METHOD_FKWDS },
	{ "casecompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casecompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as ?#compare, however ascii-casing is ignored during character comparisons") },
	{ "casevercompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casevercompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as ?#vercompare, however ascii-casing is ignored during character comparisons") },
	{ "casewildcompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casewildcompare,
	  DOC("(pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as ?#wildcompare, however ascii-casing is ignored during character comparisons") },
	{ "casefuzzycompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casefuzzycompare,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as ?#fuzzycompare, however ascii-casing is ignored during character comparisons") },
	{ "casewmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casewmatch,
	  DOC("(pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dbool\n"
	      "Same as ?#casewmatch, however ascii-casing is ignored during character comparisons") },

	/* Bytes alignment functions. */
	{ "center",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_center,
	  DOC("(width:?Dint,filler:?X3?.?Dstring?Dint=!P{ })->?.\n"
	      "Use a writable copy of @this ?. object as result, then evenly "
	      /**/ "insert @filler at the front and back to pad its length to @width bytes") },
	{ "ljust",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_ljust,
	  DOC("(width:?Dint,filler=!P{ })->?.\n"
	      "Use a writable copy of @this ?. object as result, then "
	      /**/ "insert @filler at the back to pad its length to @width bytes") },
	{ "rjust",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rjust,
	  DOC("(width:?Dint,filler=!P{ })->?.\n"
	      "Use a writable copy of @this ?. object as result, then "
	      /**/ "insert @filler at the front to pad its length to @width bytes") },
	{ "zfill",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_zfill,
	  DOC("(width:?Dint,filler=!P{0})->?.\n"
	      "Skip leading ${\'+\'} and ${\'-\'} ascii-characters, then insert @filler "
	      /**/ "to pad the resulting string to a length of @width bytes") },
	{ "reversed",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_reversed,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Return a copy of the sub-string ${this.substr(start, end)} with its byte order reversed"),
	  TYPE_METHOD_FKWDS },
	{ "expandtabs",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_expandtabs,
	  DOC("(tabwidth=!8)->?.\n"
	      "Expand tab characters with whitespace offset from the "
	      /**/ "start of their respective line at multiples of @tabwidth\n"
	      "Note that in the event of no tabs being found, @this ?. object may be re-returned") },
	{ "unifylines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_unifylines,
	  DOC("(replacement:?X3?.?Dstring?Dint=!P{\\\n})->?.\n"
	      "Unify all ascii-linefeed character sequences ($\"\\n\", $\"\\r\" and $\"\\r\\n\") "
	      "found in @this ?. object to make exclusive use of @replacement\n"
	      "Note that in the event of no line-feeds differing from @replacement being found, "
	      /**/ "@this ?. object may be re-returned") },

	/* Bytes splitter functions. */
	{ "join",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_join,
	  DOC("(seq:?S?O)->?.\n"
	      "Iterate @seq and convert all items into string, inserting @this "
	      /**/ "?. object before each string's ?Abytes?Dstring representation element, "
	      /**/ "starting only with the second. ?. objects contained in @seq are not "
	      /**/ "converted to and from strings, but inserted directly") },
	{ "split",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_split,
	  DOC("(needle:?X3?.?Dstring?Dint)->?S?.\n"
	      "Split @this ?. object at each instance of @sep, "
	      /**/ "returning a sequence of the resulting parts\n"
	      "The returned bytes objects are views of @this byte object, meaning they "
	      /**/ "have the same ?#iswritable characteristics as @this, and refer to the same "
	      /**/ "memory") },
	{ "casesplit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casesplit,
	  DOC("(needle:?X3?.?Dstring?Dint)->?S?.\n"
	      "Same as ?#split, however ascii-casing is ignored during character comparisons\n"
	      "The returned bytes objects are views of @this byte object, meaning they "
	      /**/ "have the same ?#iswritable characteristics as @this, and refer to the same "
	      /**/ "memory") },
	{ "splitlines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_splitlines,
	  DOC("(keepends=!f)->?S?.\n"
	      "Split @this ?. object at each linefeed, returning a sequence of all contained lines\n"
	      "When @keepends is ?f, this is identical to ${this.unifylines().split(\"\\n\")}\n"
	      "When @keepends is ?t, items found in the returned sequence will still have their "
	      /**/ "original, trailing line-feed appended\n"
	      "This function recognizes $\"\\n\", $\"\\r\" and $\"\\r\\n\" as linefeed sequences\n"
	      "The returned bytes objects are views of @this byte object, meaning they "
	      /**/ "have the same ?#iswritable characteristics as @this, and refer to the same "
	      /**/ "memory") },

	/* String indentation. */
	{ "indent",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_indent,
	  DOC("(filler:?X3?.?Dstring?Dint=!P{\t})->?.\n"
	      "Using @this ?. object as result, insert @filler at the front, as well as after "
	      /**/ "every ascii-linefeed with the exception of one that may be located at its end\n"
	      "The intended use is for generating strings from structured data, such as HTML:\n"
	      "${"
	      "text = \"<html>\n{}\n</html>\".format({\n"
	      "	get_html_bytes().strip().indent()\n"
	      "});"
	      "}") },
	{ "dedent",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_dedent,
	  DOC("(max_chars=!1,mask?:?X3?.?Dstring?Dint)->?.\n"
	      "Using @this string as result, remove up to @max_chars whitespace "
	      /**/ "(s.a. ?#isspace) characters, or if given characters apart of @mask "
	      /**/ "from the front, as well as following any linefeed") },

	/* Common-character search functions. */
	{ "common",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_common,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Returns the number of common leading bytes shared between @this and @other, "
	      /**/ "or in other words: the lowest index $i for which ${this[i] != other.bytes()[i]} is true") },
	{ "rcommon",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rcommon,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Returns the number of common trailing bytes shared between @this and @other") },
	{ "casecommon",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casecommon,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as ?#common, however ascii-casing is ignored during character comparisons") },
	{ "casercommon",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casercommon,
	  DOC("(other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?X2?.?Dstring,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as ?#rcommon, however ascii-casing is ignored during character comparisons") },

	/* Find match character sequences */
	{ "findmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_findmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "Similar to ?#find, but do a recursive search for the "
	      /**/ "first @close that doesn't have a match @{open}\n"
	      "For more information, see :string.findmatch") },
	{ "indexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_indexmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @close without a matching @open exists within ${this.substr(start, end)}\n"
	      "Same as ?#findmatch, but throw an :IndexError instead of "
	      /**/ "returning ${-1} if no @close without a matching @open exists") },
	{ "casefindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casefindmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "Same as ?#findmatch, however casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ?#close}\n"
	      "If no match if found, ?N is returned") },
	{ "caseindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caseindexmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @close without a matching @open exists within ${this.substr(start, end)}\n"
	      "Same as ?#indexmatch, however casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ?#close}") },
	{ "rfindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rfindmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "Similar to ?#findmatch, but operate in a mirrored fashion, searching for the "
	      /**/ "last instance of @open that has no match @close within ${this.substr(start, end)}:\n"
	      "${"
	      "s = \"get_string().foo(bar(), baz(42), 7).length\";\n"
	      "lcol = s.find(\")\");\n"
	      "print lcol; /* 19 */\n"
	      "mtch = s.rfindmatch(\"(\", \")\", 0, lcol);\n"
	      "print repr s[mtch:lcol+1]; /* \"(bar(), baz(42), 7)\" */"
	      "}\n"
	      "If no @open without a matching @close exists, ${-1} is returned") },
	{ "rindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rindexmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @open without a matching @close exists within ${this.substr(start, end)}\n"
	      "Same as ?#rfindmatch, but throw an :IndexError instead of returning ${-1} if no @open without a matching @close exists") },
	{ "caserfindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserfindmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "Same as ?#rfindmatch, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ?#open}\n"
	      "If no match if found, ?N is returned") },
	{ "caserindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserindexmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @open without a matching @close exists within ${this.substr(start, end)}\n"
	      "Same as ?#rindexmatch, however ascii-casing is ignored during character comparisons\n"
	      "Upon success, the second returned integer is equal to ${return[0] + ?#open}") },

	/* Using the find-match functionality, also provide a partitioning version */
	{ "partitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_partitionmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "A hybrid between ?#find, ?#findmatch and ?#partition that returns the strings surrounding "
	      /**/ "the matched string portion, the first being the substring prior to the match, "
	      /**/ "the second being the matched string itself (including the @open and @close strings), "
	      /**/ "and the third being the substring after the match\n"
	      "For more information see ?Apartitionmatch?Dstring") },
	{ "rpartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rpartitionmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "A hybrid between ?#rfind, ?#rfindmatch and ?#rpartition that returns the strings surrounding "
	      /**/ "the matched string portion, the first being the substring prior to the match, "
	      /**/ "the second being the matched string itself (including the @open and @close strings), "
	      /**/ "and the third being the substring after the match.\n"
	      "For more information see ?Arpartitionmatch?Dstring") },
	{ "casepartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_casepartitionmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as ?#partitionmatch, however casing is ignored during character comparisons") },
	{ "caserpartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_caserpartitionmatch,
	  DOC("(open:?X3?.?Dstring?Dint,close:?X3?.?Dstring?Dint,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as ?#rpartitionmatch, however casing is ignored during character comparisons") },

	{ "segments",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_segments,
	  DOC("(substring_length:?Dint)->?S?.\n"
	      "Split @this ?. object into segments, each exactly @substring_length characters long, with the "
	      /**/ "last segment containing the remaining characters and having a length of between "
	      /**/ "$1 and @substring_length characters.\n"
	      "This function is similar to ?#distribute, but instead of being given the "
	      /**/ "length of sub-strings and figuring out their amount, this function takes "
	      /**/ "the amount of sub-strings and figures out their lengths") },
	{ "distribute",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_distribute,
	  DOC("(substring_count:?Dint)->?S?.\n"
	      "Split @this ?. object into @substring_count similarly sized sub-strings, each with a "
	      /**/ "length of ${(##this + (substring_count - 1)) / substring_count}, followed by a last, optional "
	      /**/ "sub-string containing all remaining characters.\n"
	      "This function is similar to ?#segments, but instead of being given the "
	      /**/ "amount of sub-strings and figuring out their lengths, this function takes "
	      /**/ "the length of sub-strings and figures out their amount") },

	/* Regex functions. */
	{ "rematch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rematch,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?X2?Dint?N\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "@return The number of leading bytes in ${this.substr(start, end)} "
	      /*    */ "matched by @pattern, or ?N if @pattern doesn't match\n"
	      "Check if ${this.substr(start, end)} matches the given regular expression @pattern\n"
	      "This function behaves similar to ${this.encode(\"ascii\").rematch(pattern)}, "
	      "except that the given pattern may match non-ASCII bytes with #C{\\xAB} or #C{\\0377} "
	      "escape sequences. Furthermore, unicode character escape sequences cannot be used in "
	      "@pattern. For more information, see ?Arematch?Dstring"),
	  TYPE_METHOD_FKWDS },
	{ "rematches",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rematches,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Check if @pattern matches the entirety of the specified range of @this ?.\n"
	      "This function behaves identical to ${this.rematch(...) == ?#this}"),
	  TYPE_METHOD_FKWDS },
	{ "refind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_refind,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?X2?T2?Dint?Dint?N\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Find the first sub-string matched by @pattern, and return its start/end indices, or ?N if no match exists\n"
	      "Note that using ?N in an expand expression will expand to the all ?N-values"),
	  TYPE_METHOD_FKWDS },
	{ "rerfind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rerfind,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?X2?T2?Dint?Dint?N\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Find the last sub-string matched by @pattern, and return its start/end indices, "
	      /**/ "or ?N if no match exists (s.a. #refind)"),
	  TYPE_METHOD_FKWDS },
	{ "reindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_reindex,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?T2?Dint?Dint\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "@throw IndexError No substring matching the given @pattern could be found\n"
	      "Same as ?#refind, but throw an :IndexError when no match can be found"),
	  TYPE_METHOD_FKWDS },
	{ "rerindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rerindex,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?T2?Dint?Dint\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "@throw IndexError No substring matching the given @pattern could be found\n"
	      "Same as ?#rerfind, but throw an :IndexError when no match can be found"),
	  TYPE_METHOD_FKWDS },
	{ "relocate",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_relocate,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?X2?.?N\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Same as ${this.substr(this.refind(pattern, start, end, rules)...)}\n"
	      "In other words: return the first sub-string matched by the "
	      /**/ "given regular expression, or ?N if not found\n"
	      "This function has nothing to do with relocations! - it's pronounced R.E. locate"),
	  TYPE_METHOD_FKWDS },
	{ "rerlocate",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rerlocate,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?X2?.?N\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Same as ${this.substr(this.rerfind(pattern, start, end, rules)...)}\n"
	      "In other words: return the last sub-string matched by the "
	      /**/ "given regular expression, or ?N if not found"),
	  TYPE_METHOD_FKWDS },
	{ "repartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_repartition,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?T3?.?.?.\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "A hybrid between ?#refind and ?#partition\n${"
	      "function repartition(pattern: string, start: int, end: int, rules: string) {\n"
	      "	local start, end = this.refind(pattern, start, end, rules)...;\n"
	      "	if (start is none)\n"
	      "		return (this, \"\".bytes(), \"\".bytes());\n"
	      "	return (\n"
	      "		this.substr(0, start),\n"
	      "		this.substr(start, end),\n"
	      "		this.substr(end, -1)\n"
	      "	);\n"
	      "}}"),
	  TYPE_METHOD_FKWDS },
	{ "rerpartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rerpartition,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?T3?.?.?.\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "A hybrid between ?#rerfind and ?#rpartition\n${"
	      "function rerpartition(pattern: string, start: int, end: int, rules: string) {\n"
	      "	local start, end = this.rerfind(pattern, start, end, rules)...;\n"
	      "	if (start is none)\n"
	      "		return (this, \"\".bytes(), \"\".bytes());\n"
	      "	return (\n"
	      "		this.substr(0, start),\n"
	      "		this.substr(start, end), \n"
	      "		this.substr(end, -1)\n"
	      "	);\n"
	      "}}"),
	  TYPE_METHOD_FKWDS },
	{ "rereplace",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rereplace,
	  DOC("(pattern:?Dstring,replace:?.,max:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?.\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Similar to ?#replace, however the ?. to search for is implemented as a regular expression "
	      "pattern, with the sub-string matched by it then getting replaced by @replace\n"
	      "Additionally, @replace may contain sed-like match sequences:\n"
	      "#T{Expression|Description~"
	      /**/ "#C{&}|Replaced with the entire sub-string matched by @pattern&"
	      /**/ "#C{\\n}|Where $n is a digit ${1-9} specifying the n'th (1-based) group in "
	      /**/ /*   */ "@pattern (groups are determined by parenthesis in regex patterns)&"
	      /**/ "#C{\\\\}|Outputs a literal $r\"\\\" into the returned ?.&"
	      /**/ "#C{\\&}|Outputs a literal $r\"&\" into the returned ?."
	      "}"),
	  TYPE_METHOD_FKWDS },
	{ "refindall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_refindall,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?T2?Dint?Dint\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Similar to ?#refind, but return a sequence of all matches found within ${this.substr(start, end)}\n"
	      "Note that the matches returned are ordered ascendingly"),
	  TYPE_METHOD_FKWDS },
	{ "relocateall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_relocateall,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?.\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Similar to ?#relocate, but return a sequence of all matched "
	      "sub-strings found within ${this.substr(start, end)}\n"
	      "Note that the matches returned are ordered ascendingly\n"
	      "This function has nothing to do with relocations! - it's pronounced R.E. locate all"),
	  TYPE_METHOD_FKWDS },
	{ "resplit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_resplit,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?.\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Similar to ?#split, but use a regular expression in order to "
	      "express the sections of the ?. around which to perform the split\n"

	      "${"
	      /**/ "local data = \"10 , 20,30 40, 50\".bytes();\n"
	      /**/ "for (local x: data.resplit(r\"[[:space:],]+\"))\n"
	      /**/ "	print x; /* `10' `20' `30' `40' `50' */"
	      "}\n"

	      "If you wish to do the inverse and enumerate matches, rather than the "
	      "strings between matches, use ?#relocateall instead, which also behaves "
	      "as a sequence"),
	  TYPE_METHOD_FKWDS },
	{ "restartswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_restartswith,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Check if @this ?. starts with a regular expression described by @pattern (s.a. ?#startswith)\n"
	      "${"
	      /**/ "function restartswith(pattern: string) {\n"
	      /**/ "	return this.rematch(pattern) !is none;\n"
	      /**/ "}"
	      "}"),
	  TYPE_METHOD_FKWDS },
	{ "reendswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_reendswith,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Check if @this ?. ends with a regular expression described by @pattern (s.a. ?#endswith)\n"
	      "${"
	      /**/ "function restartswith(pattern: string) {\n"
	      /**/ "	local rpos = this.rerfind(pattern);\n"
	      /**/ "	return rpos !is none && rpos[1] == ##this;\n"
	      /**/ "}"
	      "}"),
	  TYPE_METHOD_FKWDS },
	{ "restrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_restrip,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?.\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Strip all leading and trailing matches for @pattern from @this ?. and return the result (s.a. ?#strip)"),
	  TYPE_METHOD_FKWDS },
	{ "relstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_relstrip,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?.\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Strip all leading matches for @pattern from @this ?. and return the result (s.a. ?#lstrip)"),
	  TYPE_METHOD_FKWDS },
	{ "rerstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_rerstrip,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?.\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Strip all trailing matches for @pattern from @this ?. and return the result (s.a. ?#lstrip)"),
	  TYPE_METHOD_FKWDS },
	{ "recount",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_recount,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dint\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Count the number of matches of a given regular expression @pattern (s.a. ?#count)\n"
	      "Hint: This is the same as ${##this.refindall(pattern)} or ${##this.relocateall(pattern)}\n"
	      "If the pattern starts matching epsilon, counting is stopped"),
	  TYPE_METHOD_FKWDS },
	{ "recontains",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_recontains,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Check if @this contains a match for the given regular expression @pattern (s.a. ?#contains)\n"
	      "Hint: This is the same as ${!!this.refindall(pattern)} or ${!!this.relocateall(pattern)}"),
	  TYPE_METHOD_FKWDS },

	/* Regex functions that return the start-/end-offsets of all groups (rather than only the whole match) */
	{ "regmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_regmatch,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	      "Similar to ?#rematch, but rather than only return the number of characters that were "
	      /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	      /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	      /**/ "start-/end-offsets of each individual group referenced by @pattern. Groups "
	      /**/ "that didn't get matched (because they might be optional) appear as ?N in the "
	      /**/ "returned sequence.\n"
	      "When nothing was matched, an empty sequence is returned.\n"
	      "Example:\n"
	      "${"
	      /**/ "local groups = \"foo bar foobar\".bytes().regmatch(r\"fo(o) (b(x)r|bar) fo(o?bar)\");\n"
	      /**/ "assert groups == {\n"
	      /**/ "	{0, 14},  /* Whole match */\n"
	      /**/ "	{2, 3},   /* \"o\" */\n"
	      /**/ "	{4, 7},   /* \"bar\" */\n"
	      /**/ "	none,     /* never matched: \"x\" */\n"
	      /**/ "	{10, 14}, /* \"obar\" */\n"
	      /**/ "};"
	      "}\n"
	      "Note that (if something was matched), this function still only matches at the "
	      "start of @this ?.. If you want to search for @pattern and get the offsets of "
	      "all of the matched groups, you should use ?#regfind instead."),
	  TYPE_METHOD_FKWDS },
	{ "regfind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_regfind,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	      "Similar to ?#refind, but rather than only return the character-range "
	      /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	      /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	      /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	      "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	  TYPE_METHOD_FKWDS },
	{ "regrfind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_regrfind,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	      "Similar to ?#rerfind, but rather than only return the character-range "
	      /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	      /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	      /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	      "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	  TYPE_METHOD_FKWDS },
	{ "regfindall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_regfindall,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?S?X2?T2?Dint?Dint?N\n"
	      "Similar to ?#refindall, but rather than only return the character-ranges "
	      /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	      /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	      /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	      "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	  TYPE_METHOD_FKWDS },
	{ "regindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_regindex,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "@throw IndexError No substring matching the given @pattern could be found\n"
	      "Same as ?#regfind, but throw an :IndexError when no match can be found"),
	  TYPE_METHOD_FKWDS },
	{ "regrindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&bytes_regrindex,
	  DOC("(pattern:?Dstring,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	      "@param pattern The regular expression pattern (s.a. ?#rematch)\n"
	      "@param range The max number of search attempts to perform\n"
	      "@param rules The regular expression rules (s.a. ?#rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "@throw IndexError No substring matching the given @pattern could be found\n"
	      "Same as ?#regrfind, but throw an :IndexError when no match can be found"),
	  TYPE_METHOD_FKWDS },

	{ NULL }
};


DECL_END

#ifndef __INTELLISENSE__
#include "bytes_split.c.inl"
#include "bytes_segments.c.inl"
#include "bytes_finder.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_BYTES_FUNCTIONS_C_INL */
