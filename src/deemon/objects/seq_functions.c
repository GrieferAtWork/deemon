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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C
#define GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C 1

#include "seq_functions.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/minmax.h>

#include "../runtime/runtime_error.h"

DECL_BEGIN


/* NOTE: Technically, all of these functions can be used on any type of object,
 *       but all objects derived from `DeeSeq_Type' automatically implement
 *       all of them as member functions.
 *       With that in mind, any type implementing the `tp_seq' interface
 *       with the intention of behaving as an Iterable, should probably
 *       be derived from `DeeSeq_Type' as this allows usercode to query
 *       for a general purpose sequence by writing `x is Sequence from deemon' */
INTERN WUNUSED NONNULL((1)) size_t DCALL DeeSeq_Size(DeeObject *__restrict self) {
	DREF DeeObject *iter, *elem;
	size_t result = 0;
	/* Count the number of elements, given an iterator. */
	iter = DeeObject_Iter(self);
	if unlikely(!iter) {
		if (DeeError_Catch(&DeeError_NotImplemented))
			err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
		goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		Dee_Decref(elem);
		if unlikely(result == (size_t)-2) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			goto err;
		}
		++result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	Dee_Decref(iter);
	if unlikely(!elem)
		goto err;
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
iterator_get_nth(DeeObject *__restrict self,
                 DeeObject *__restrict sequence,
                 size_t index) {
	DREF DeeObject *elem;
	size_t current_index = 0;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		if (current_index == (size_t)index)
			return elem;
		Dee_Decref(elem);
		++current_index;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	err_index_out_of_bounds(sequence, index, current_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_GetItem(DeeObject *__restrict self, size_t index) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if unlikely(tp_self == &DeeSeq_Type) {
		err_index_out_of_bounds(self, index, 0);
		goto err;
	}
	DeeTypeMRO_Init(&mro, tp_self);
	for (;;) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_getitem)
					return (*nsi->nsi_seqlike.nsi_getitem)(self, index);
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					size_t mylen = (*nsi->nsi_common.nsi_getsize)(self);
					if unlikely(mylen == (size_t)-1)
						goto err;
					if unlikely(index >= mylen) {
						err_index_out_of_bounds(self, index, mylen);
						goto err;
					}
					result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, index);
					if unlikely(!result)
						err_unbound_index(self, index);
					return result;
				}
				if (nsi->nsi_seqlike.nsi_getrange) {
					result = (*nsi->nsi_seqlike.nsi_getrange)(self, index, index + 1);
					if unlikely(!result)
						goto err;
					goto return_result_first;
				}
			}
			if (has_noninherited_getitem(tp_self, seq)) {
				DREF DeeObject *index_ob;
				index_ob = DeeInt_NewSize(index);
				if unlikely(!index_ob)
					goto err;
				result = (*seq->tp_getitem)(self, index_ob);
				Dee_Decref(index_ob);
				return result;
			}
			if (has_noninherited_getrange(tp_self, seq)) {
				DREF DeeObject *real_result;
				DREF DeeObject *index_ob, *index_plus1_ob;
				index_ob = DeeInt_NewSize(index);
				if unlikely(!index_ob)
					goto err;
				index_plus1_ob = DeeInt_NewSize(index + 1);
				if unlikely(!index_plus1_ob) {
					Dee_Decref(index_ob);
					goto err;
				}
				result = (*seq->tp_getrange)(self, index_ob, index_plus1_ob);
				Dee_Decref(index_plus1_ob);
				Dee_Decref(index_ob);
				if unlikely(!result)
					goto err;
return_result_first:
				real_result = DeeSeq_Front(result);
				Dee_Decref(result);
				if unlikely(!real_result) {
					/* Translate the empty-sequence error into an index-out-of-bounds */
					if (DeeError_Catch(&DeeError_ValueError)) {
						size_t mylen = DeeObject_Size(self);
						if unlikely(mylen == (size_t)-1)
							goto err;
						err_index_out_of_bounds(self, index, mylen);
					}
				}
				return real_result;
			}
			if (seq->tp_iter) {
				DREF DeeObject *iterator;
				iterator = (*seq->tp_iter)(self);
				if unlikely(!iterator)
					goto err;
				result = iterator_get_nth(iterator, self, index);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
		if (tp_self == &DeeSeq_Type)
			break;
	}
	err_unimplemented_operator3(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_GETRANGE,
	                            OPERATOR_ITER);
err:
	return NULL;
}

INTERN ATTR_COLD int DCALL
err_no_generic_sequence(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Neither `%k.__getitem__' and `%k.__size__', nor `%k.__iter__' are implemented",
	                       Dee_TYPE(self), Dee_TYPE(self), Dee_TYPE(self));
}




INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_NonEmpty(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	DeeTypeMRO mro;
	int result;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if unlikely(tp_self == &DeeSeq_Type)
		return 0;
	DeeTypeMRO_Init(&mro, tp_self);
	for (;;) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			DREF DeeObject *temp;
			if ((nsi = seq->tp_nsi) != NULL &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				size_t length = (*nsi->nsi_common.nsi_getsize)(self);
				if unlikely(length == (size_t)-1)
					goto err;
				return length != 0;
			}
			if (has_noninherited_size(tp_self, seq)) {
				if (has_noninherited_bool(tp_self))
					return (*tp_self->tp_cast.tp_bool)(self);
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				result = DeeObject_Bool(temp);
				Dee_Decref(temp);
				if unlikely(result < 0)
					goto err;
				return result;
			}
			if (seq->tp_iter) {
				DREF DeeObject *elem;
				temp = (*seq->tp_iter)(self);
				if unlikely(!temp)
					goto err;
				elem = DeeObject_IterNext(temp);
				Dee_Decref(temp);
				if (elem == ITER_DONE)
					return 0;
				if unlikely(!elem)
					goto err;
				Dee_Decref(elem);
				return 1;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
		if (tp_self == &DeeSeq_Type)
			break;
	}
	err_unimplemented_operator2(Dee_TYPE(self),
	                            OPERATOR_SIZE,
	                            OPERATOR_ITER);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Front(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			DREF DeeObject *temp;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    nsi->nsi_seqlike.nsi_getitem &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				result = (*nsi->nsi_seqlike.nsi_getitem)(self, 0);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_IndexError))
						goto err_empty;
				}
				return result;
			}
			if (has_noninherited_getitem(tp_self, seq)) {
				result = (*seq->tp_getitem)(self, DeeInt_Zero);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_IndexError))
						goto err_empty;
				}
				return result;
			}
			if (seq->tp_iter) {
				temp = (*seq->tp_iter)(self);
				if unlikely(!temp)
					goto err;
				result = DeeObject_IterNext(temp);
				Dee_Decref(temp);
				if (result == ITER_DONE)
					goto err_empty;
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_unimplemented_operator2(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_ITER);
err:
	return NULL;
err_empty:
	err_empty_sequence(self);
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Back(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	size_t seq_length;
	DREF DeeObject *result, *temp;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_getitem) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if unlikely(!seq_length)
						goto err_empty;
					if (nsi->nsi_seqlike.nsi_getitem_fast)
						return (*nsi->nsi_seqlike.nsi_getitem_fast)(self, seq_length - 1);
					return (*nsi->nsi_seqlike.nsi_getitem)(self, seq_length - 1);
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if unlikely(!seq_length)
						goto err_empty;
					temp = DeeInt_NewSize(seq_length - 1);
					if unlikely(!temp)
						goto err;
					result = (*seq->tp_getitem)(self, temp);
					Dee_Decref(temp);
					return result;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				int size_is_nonzero;
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				size_is_nonzero = DeeObject_Bool(temp);
				if unlikely(size_is_nonzero <= 0) {
					if unlikely(size_is_nonzero < 0)
						goto err_temp;
					Dee_Decref(temp);
					goto err_empty;
				}
				if (DeeObject_Dec(&temp))
					goto err_temp;
				result = (*seq->tp_getitem)(self, temp);
				Dee_Decref(temp);
				return result;
			}
			if (seq->tp_iter) {
				DREF DeeObject *next;
				temp = (*seq->tp_iter)(self);
				if unlikely(!temp)
					goto err;
				result = NULL;
				while (ITER_ISOK(next = DeeObject_IterNext(temp))) {
					Dee_XDecref(result);
					result = next;
					if (DeeThread_CheckInterrupt()) {
						Dee_Decref(result);
						goto err;
					}
				}
				Dee_Decref(temp);
				if unlikely(!next) {
					Dee_XClear(result);
				} else if unlikely(!result) {
					goto err_empty;
				}
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return NULL;
err_empty:
	err_empty_sequence(self);
	return NULL;
err_temp:
	Dee_Decref(temp);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeSeq_Reduce(DeeObject *self, DeeObject *combine, DeeObject *init) {
	DREF DeeObject *iterator, *elem, *merge;
	DREF DeeObject *result = init;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto err;
	Dee_XIncref(result);
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		/* Special handling for the first element. */
		if (!result) {
			result = elem;
		} else {
			/* Invoke the given combination-callback to merge the 2 items. */
			merge = DeeObject_CallPack(combine, 2, result, elem);
			Dee_Decref(elem);
			Dee_Decref(result);
			/* Check for errors. */
			if unlikely(!merge)
				goto err_iter;
			result = merge;
		}
		if (DeeThread_CheckInterrupt())
			goto err_iter_r;
	}
	Dee_Decref(iterator);
	if unlikely(!elem) {
		Dee_XClear(result);
	} else if (!result) {
		/* Must return `none' when the sequence was empty. */
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
	return result;
err_iter_r:
	Dee_Decref(result);
err_iter:
	Dee_Decref(iterator);
err:
	return NULL;
}

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Sum(DeeObject *__restrict self) {
	DREF DeeObject *iterator, *elem, *merge;
	DREF DeeObject *result = NULL;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto err;
	/* Yield the first item to perform type-specific optimizations. */
	result = DeeObject_IterNext(iterator);
	if (!ITER_ISOK(result)) {
		/* Special case: empty sequence. */
		if unlikely(!result)
			goto err_iter;
		Dee_Decref_likely(iterator);
		return_none;
	}
	if (DeeBytes_Check(result)) {
		struct bytes_printer p;
		dssize_t error;
		elem = DeeObject_IterNext(iterator);
		if (!ITER_ISOK(elem)) {
			Dee_Decref(iterator);
			/* Simple case: Nothing to combine. - No need to use a printer. */
			if (elem == ITER_DONE)
				return result;
			Dee_Decref(result);
			goto err;
		}
		/* Use a bytes printer. */
		bytes_printer_init(&p);
		error = bytes_printer_append(&p, DeeBytes_DATA(result), DeeBytes_SIZE(result));
		if (error >= 0)
			error = DeeObject_Print(elem, &bytes_printer_print, &p);
		Dee_Decref(elem);
		Dee_Decref(result);
		if unlikely(error < 0)
			goto err_bytes;
		/* Now print all the rest into the string as well. */
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			error = DeeObject_Print(elem, &bytes_printer_print, &p);
			Dee_Decref(elem);
			if unlikely(error < 0)
				goto err_bytes;
			if (DeeThread_CheckInterrupt())
				goto err_bytes;
		}
		if unlikely(!elem)
			goto err_bytes;
		Dee_Decref(iterator);
		return bytes_printer_pack(&p);
err_bytes:
		bytes_printer_fini(&p);
		goto err_iter;
	}
	if (DeeString_Check(result)) {
		struct unicode_printer p;
		dssize_t error;
		elem = DeeObject_IterNext(iterator);
		if (!ITER_ISOK(elem)) {
			Dee_Decref(iterator);
			/* Simple case: Nothing to combine. - No need to use a printer. */
			if (elem == ITER_DONE)
				return result;
			Dee_Decref(result);
			goto err;
		}
		/* Use a unicode printer. */
		unicode_printer_init(&p);
		error = unicode_printer_printstring(&p, result);
		if (error >= 0)
			error = DeeObject_Print(elem, &unicode_printer_print, &p);
		Dee_Decref(elem);
		Dee_Decref(result);
		if unlikely(error < 0)
			goto err_string;
		/* Now print all the rest into the string as well. */
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			error = DeeObject_Print(elem, &unicode_printer_print, &p);
			Dee_Decref(elem);
			if unlikely(error < 0)
				goto err_string;
			if (DeeThread_CheckInterrupt())
				goto err_string;
		}
		if unlikely(!elem)
			goto err_string;
		Dee_Decref(iterator);
		return unicode_printer_pack(&p);
err_string:
		unicode_printer_fini(&p);
		goto err_iter;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		/* add the given element to the result. */
		merge = DeeObject_Add(result, elem);
		Dee_Decref(elem);
		Dee_Decref(result);
		/* Check for errors. */
		if unlikely(!merge)
			goto err_iter;
		result = merge;
		if (DeeThread_CheckInterrupt())
			goto err_iter_r;
	}
	if unlikely(!elem)
		goto err_iter_r;
	Dee_Decref(iterator);
	return result;
err_iter_r:
	Dee_Decref(result);
err_iter:
	Dee_Decref(iterator);
err:
	return NULL;
}

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSeq_Any(DeeObject *__restrict self) {
	DREF DeeObject *iterator, *elem;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		int temp = DeeObject_Bool(elem);
		Dee_Decref(elem);
		if (temp != 0) {
			Dee_Decref(iterator);
			return temp; /* error or true */
		}
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(iterator);
	if unlikely(!elem)
		goto err;
	return 0;
err:
	return -1;
err_iter:
	Dee_Decref(iterator);
	goto err;
}

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSeq_All(DeeObject *__restrict self) {
	DREF DeeObject *iterator, *elem;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		int temp = DeeObject_Bool(elem);
		Dee_Decref(elem);
		if (temp <= 0) {
			Dee_Decref(iterator);
			return temp; /* error or false */
		}
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(iterator);
	if unlikely(!elem)
		goto err;
	return 1;
err:
	return -1;
err_iter:
	Dee_Decref(iterator);
	goto err;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_Parity(DeeObject *__restrict self) {
	DREF DeeObject *iterator, *elem;
	int result = 0;
	if unlikely((iterator = DeeObject_Iter(self)) == NULL)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		int temp = DeeObject_Bool(elem);
		Dee_Decref(elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err_iter;
			result ^= 1; /* Invert parity. */
		}
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(iterator);
	if unlikely(!elem)
		goto err;
	return result;
err_iter:
	Dee_Decref(iterator);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Min_k(DeeObject *self,
             DeeObject *key) {
	DREF DeeObject *elem, *iterator, *result = NULL;
	DREF DeeObject *key_result = NULL;
	int temp;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!result) {
			result = elem;
		} else {
			DREF DeeObject *key_elem;
			if (!key_result) {
				key_result = DeeObject_Call(key, 1, &result);
				if unlikely(!key_result)
					goto err_r_iter_elem;
			}
			key_elem = DeeObject_Call(key, 1, &elem);
			if unlikely(!key_elem)
				goto err_r_iter_elem;
			temp = DeeObject_CmpLoAsBool(key_result, key_elem);
			if (temp <= 0) {
				if unlikely(temp < 0) {
					Dee_Decref(key_elem);
					goto err_r_iter_elem;
				}
				Dee_Decref(key_result);
				Dee_Decref(result);
				/* Continue working with `elem' after
					 * `result < elem' evaluated to `false' */
				result     = elem;
				key_result = key_elem;
			} else {
				Dee_Decref(key_elem);
				Dee_Decref(elem);
			}
		}
		if (DeeThread_CheckInterrupt())
			goto err_r_iter;
	}
	if unlikely(!elem)
		goto err_r_iter;
	Dee_Decref(iterator);
	/* Return `none' when the sequence was empty. */
	if (!result) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
done:
	Dee_XDecref(key_result);
	return result;
err_r_iter_elem:
	Dee_Decref(elem);
err_r_iter:
	Dee_XDecref(key_result);
	Dee_XDecref(result);
	Dee_Decref(iterator);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Max_k(DeeObject *self,
             DeeObject *key) {
	DREF DeeObject *elem, *iterator, *result = NULL;
	DREF DeeObject *key_result = NULL;
	int temp;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!result) {
			result = elem;
		} else {
			DREF DeeObject *key_elem;
			if (!key_result) {
				key_result = DeeObject_Call(key, 1, &result);
				if unlikely(!key_result)
					goto err_r_iter_elem;
			}
			key_elem = DeeObject_Call(key, 1, &elem);
			if unlikely(!key_elem)
				goto err_r_iter_elem;
			temp = DeeObject_CmpLoAsBool(key_result, key_elem);
			if (temp <= 0) {
				if unlikely(temp < 0) {
					Dee_Decref(key_elem);
					goto err_r_iter_elem;
				}
				Dee_Decref(key_elem);
				Dee_Decref(elem);
			} else {
				Dee_Decref(key_result);
				Dee_Decref(result);
				/* Continue working with `elem' after
				 * `result < elem' evaluated to `false' */
				result     = elem;
				key_result = key_elem;
			}
		}
		if (DeeThread_CheckInterrupt())
			goto err_r_iter;
	}
	if unlikely(!elem)
		goto err_r_iter;
	Dee_Decref(iterator);
	/* Return `none' when the sequence was empty. */
	if (!result) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
done:
	Dee_XDecref(key_result);
	return result;
err_r_iter_elem:
	Dee_Decref(elem);
err_r_iter:
	Dee_XDecref(key_result);
	Dee_XDecref(result);
	Dee_Decref(iterator);
	return NULL;
}

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Min(DeeObject *self, DeeObject *key) {
	DREF DeeObject *elem, *iterator, *result = NULL;
	int temp;
	if (key)
		return DeeSeq_Min_k(self, key);
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!result) {
			result = elem;
		} else {
			temp = DeeObject_CmpLoAsBool(result, elem);
			if (temp <= 0) {
				if unlikely(temp < 0)
					goto err_r_iter_elem;
				Dee_Decref(result);
				/* Continue working with `elem' after
				 * `result < elem' evaluated to `false' */
				result = elem;
			} else {
				Dee_Decref(elem);
			}
		}
		if (DeeThread_CheckInterrupt())
			goto err_r_iter;
	}
	if unlikely(!elem)
		goto err_r_iter;
	Dee_Decref(iterator);
	/* Return `none' when the sequence was empty. */
	if (!result) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
done:
	return result;
err_r_iter_elem:
	Dee_Decref(elem);
err_r_iter:
	Dee_XDecref(result);
	Dee_Decref(iterator);
	return NULL;
}

/* Functions used to implement special sequence expressions,
 * such as `x + ...' (as `DeeSeq_Sum'), etc. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Max(DeeObject *self, DeeObject *key) {
	DREF DeeObject *elem, *iterator, *result = NULL;
	int temp;
	if (key)
		return DeeSeq_Max_k(self, key);
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!result) {
			result = elem;
		} else {
			temp = DeeObject_CmpLoAsBool(result, elem);
			if (temp <= 0) {
				if unlikely(temp < 0)
					goto err_r_iter_elem;
				Dee_Decref(elem);
			} else {
				Dee_Decref(result);
				/* Continue working with `elem' after
				 * `result < elem' evaluated to `false' */
				result = elem;
			}
		}
		if (DeeThread_CheckInterrupt())
			goto err_r_iter;
	}
	if unlikely(!elem)
		goto err_r_iter;
	Dee_Decref(iterator);
	/* Return `none' when the sequence was empty. */
	if (!result) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
done:
	return result;
err_r_iter_elem:
	Dee_Decref(elem);
err_r_iter:
	Dee_XDecref(result);
	Dee_Decref(iterator);
	return NULL;
}


INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_Count(DeeObject *self,
             DeeObject *keyed_search_item,
             DeeObject *key) {
	size_t result = 0;
	int temp;
	DREF DeeObject *elem, *iterator;
	/* TODO: NSI Variant + index-based sequence optimizations */
	if unlikely((iterator = DeeObject_Iter(self)) == NULL)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, elem, key);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err_elem;
			++result; /* Found one! */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(iterator);
	if unlikely(!elem)
		goto err;
	return result;
err_elem:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iterator);
err:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Locate(DeeObject *self,
              DeeObject *keyed_search_item,
              DeeObject *key) {
	DREF DeeObject *elem, *iterator;
	int temp;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, elem, key);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err_elem;
			/* Found it! */
			Dee_Decref(iterator);
			return elem;
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(iterator);
	if (elem)
		err_item_not_found(self, keyed_search_item);
	return NULL;
err_elem:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iterator);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_RLocate(DeeObject *self,
               DeeObject *keyed_search_item,
               DeeObject *key) {
	DREF DeeObject *elem, *iterator, *result = NULL;
	int temp;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, elem, key);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err_elem;
			/* Found one! */
			Dee_XDecref(result);
			result = elem;
		} else {
			Dee_Decref(elem);
		}
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	Dee_Decref(iterator);
	if unlikely(!elem) {
		Dee_XClear(result);
	} else if (!result) {
		err_item_not_found(self, keyed_search_item);
	}
	return result;
err_elem:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iterator);
	Dee_XDecref(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_Contains(DeeObject *self,
                DeeObject *keyed_search_item,
                DeeObject *key) {
	DREF DeeObject *iter, *elem;
	int temp;
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, elem, key);
		if (temp != 0) {
			Dee_Decref(iter);
			return temp;
		}
		Dee_Decref(elem);
	}
	Dee_Decref(iter);
	if unlikely(!elem)
		goto err;
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_StartsWith(DeeObject *self,
                  DeeObject *keyed_search_item,
                  DeeObject *key) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			DREF DeeObject *temp;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    nsi->nsi_seqlike.nsi_getitem &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				int error;
				result = (*nsi->nsi_seqlike.nsi_getitem)(self, 0);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_IndexError))
						goto err_empty;
					goto err;
				}
check:
				error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, result, key);
				Dee_Decref(result);
				return error;
			}
			if (has_noninherited_getitem(tp_self, seq)) {
				result = (*seq->tp_getitem)(self, DeeInt_Zero);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_IndexError))
						goto err_empty;
					goto err;
				}
				goto check;
			}
			if (seq->tp_iter) {
				temp = (*seq->tp_iter)(self);
				if unlikely(!temp)
					goto err;
				result = DeeObject_IterNext(temp);
				Dee_Decref(temp);
				if (result == ITER_DONE)
					goto err_empty;
				if unlikely(!result)
					goto err;
				goto check;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_unimplemented_operator2(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_ITER);
err:
	return -1;
err_empty:
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EndsWith(DeeObject *self,
                DeeObject *keyed_search_item,
                DeeObject *key) {
	DeeTypeObject *tp_self;
	size_t seq_length;
	DREF DeeObject *result, *temp;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi const *nsi;
			if ((nsi = seq->tp_nsi) != NULL &&
			    nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_getitem) {
					int error;
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if unlikely(!seq_length)
						goto err_empty;
					if (nsi->nsi_seqlike.nsi_getitem_fast) {
						result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, seq_length - 1);
					} else {
						result = (*nsi->nsi_seqlike.nsi_getitem)(self, seq_length - 1);
					}
					if unlikely(!result)
						goto err;
check:
					error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, result, key);
					Dee_Decref(result);
					return error;
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if unlikely(!seq_length)
						goto err_empty;
					temp = DeeInt_NewSize(seq_length - 1);
					if unlikely(!temp)
						goto err;
					result = (*seq->tp_getitem)(self, temp);
					Dee_Decref(temp);
					if unlikely(!result)
						goto err;
					goto check;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				int size_is_nonzero;
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				size_is_nonzero = DeeObject_Bool(temp);
				if unlikely(size_is_nonzero <= 0) {
					if unlikely(size_is_nonzero < 0)
						goto err_temp;
					Dee_Decref(temp);
					goto err_empty;
				}
				if (DeeObject_Dec(&temp))
					goto err_temp;
				result = (*seq->tp_getitem)(self, temp);
				Dee_Decref(temp);
				if unlikely(!result)
					goto err;
				goto check;
			}
			if (seq->tp_iter) {
				DREF DeeObject *next;
				temp = (*seq->tp_iter)(self);
				if unlikely(!temp)
					goto err;
				result = NULL;
				while (ITER_ISOK(next = DeeObject_IterNext(temp))) {
					Dee_XDecref(result);
					result = next;
					if (DeeThread_CheckInterrupt())
						goto err_r;
				}
				Dee_Decref(temp);
				if unlikely(!next) {
err_r:
					Dee_XDecref(result);
					goto err;
				}
				if unlikely(!result)
					goto err_empty;
				goto check;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return -1;
err_empty:
	return 0;
err_temp:
	Dee_Decref(temp);
	goto err;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
iterator_find(DeeObject *__restrict iterator,
              DeeObject *__restrict keyed_search_item,
              size_t start, size_t end,
              DeeObject *key) {
	DREF DeeObject *elem;
	size_t index = 0;
	int temp;
	size_t search_size = end - start;
	while (start) {
		elem = DeeObject_IterNext(iterator);
		if (!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			goto notfound;
		}
		Dee_Decref(elem);
		--start;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, elem, key);
		Dee_Decref(elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			/* Found it! */
			return index;
		}
		++index;
		if unlikely(index == (size_t)-2) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			goto err;
		}
		if (!--search_size)
			goto notfound; /* End of search size */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
notfound:
	return (size_t)-1; /* Not found. */
err:
	return (size_t)-2;
}

PRIVATE size_t DCALL
iterator_rfind(DeeObject *__restrict iterator,
               DeeObject *__restrict keyed_search_item,
               size_t start, size_t end,
               DeeObject *key) {
	DREF DeeObject *elem;
	size_t index = 0;
	int temp;
	size_t search_size = end - start;
	size_t result      = (size_t)-1;
	while (start) {
		elem = DeeObject_IterNext(iterator);
		if (!ITER_ISOK(elem)) {
			if unlikely(!elem)
				goto err;
			goto notfound;
		}
		Dee_Decref(elem);
		--start;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, elem, key);
		Dee_Decref(elem);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			/* Found it! */
			result = index;
		}
		++index;
		if unlikely(index == (size_t)-2) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			goto err;
		}
		if (!--search_size)
			return result; /* End of search size */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	return result;
err:
	return (size_t)-2;
notfound:
	return (size_t)-1;
}


INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
DeeSeq_Find(DeeObject *self,
            size_t start, size_t end,
            DeeObject *keyed_search_item,
            DeeObject *key) {
	DREF DeeObject *iterator;
	size_t result;
	DeeTypeObject *tp_self;
	size_t i, seq_length;
	DREF DeeObject *temp;
	DeeTypeMRO mro;
	int error;
	ASSERT_OBJECT(self);
	if unlikely(start >= end)
		goto notfound;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq) {
			struct type_nsi const *nsi = seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_find)
					return (*nsi->nsi_seqlike.nsi_find)(self, start, end, keyed_search_item, key);
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start >= seq_length)
						goto notfound;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						temp = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, i);
						if unlikely(!temp)
							continue;
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					}
					goto notfound;
				}
				if (nsi->nsi_seqlike.nsi_getitem) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start >= seq_length)
						goto notfound;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						temp = (*nsi->nsi_seqlike.nsi_getitem)(self, i);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					}
					goto notfound;
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
do_lookup_tpget:
					if (start >= seq_length)
						goto notfound;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						DREF DeeObject *index_ob;
						index_ob = DeeInt_NewSize(i);
						if unlikely(!index_ob)
							goto err;
						temp = (*seq->tp_getitem)(self, index_ob);
						Dee_Decref(index_ob);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					}
					goto notfound;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				if (DeeObject_AsSize(temp, &seq_length))
					goto err_temp;
				Dee_Decref(temp);
				goto do_lookup_tpget;
			}
			if (seq->tp_iter) {
				/* Use iterators */
				if unlikely((iterator = (*seq->tp_iter)(self)) == NULL)
					goto err;
				result = iterator_find(iterator, keyed_search_item, start, end, key);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return (size_t)-2;
err_temp:
	Dee_Decref(temp);
	goto err;
notfound:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 4)) size_t DCALL
DeeSeq_RFind(DeeObject *self,
             size_t start, size_t end,
             DeeObject *keyed_search_item,
             DeeObject *key) {
	DREF DeeObject *iterator;
	size_t result;
	DeeTypeObject *tp_self;
	size_t i, seq_length;
	DREF DeeObject *temp;
	DeeTypeMRO mro;
	int error;
	ASSERT_OBJECT(self);
	if unlikely(start >= end)
		goto notfound;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq) {
			struct type_nsi const *nsi = seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_rfind)
					return (*nsi->nsi_seqlike.nsi_rfind)(self, start, end, keyed_search_item, key);
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start > seq_length)
						start = seq_length;
					if (end > seq_length)
						end = seq_length;
					i = end - 1;
					do {
						temp = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, i);
						if unlikely(!temp)
							continue;
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					} while (i-- > start);
					goto notfound;
				}
				if (nsi->nsi_seqlike.nsi_getitem) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start > seq_length)
						start = seq_length;
					if (end > seq_length)
						end = seq_length;
					i = end - 1;
					do {
						temp = (*nsi->nsi_seqlike.nsi_getitem)(self, i);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					} while (i-- > start);
					goto notfound;
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
do_lookup_tpget:
					if (start > seq_length)
						start = seq_length;
					if (end > seq_length)
						end = seq_length;
					i = end - 1;
					do {
						DREF DeeObject *index_ob;
						index_ob = DeeInt_NewSize(i);
						if unlikely(!index_ob)
							goto err;
						temp = (*seq->tp_getitem)(self, index_ob);
						Dee_Decref(index_ob);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_TryCmpKeyEqAsBool(keyed_search_item, temp, key);
						Dee_Decref(temp);
						if (error != 0) {
							if unlikely(error < 0)
								goto err;
							if unlikely(i == (size_t)-2 || i == (size_t)-1) {
								err_integer_overflow_i(sizeof(size_t) * 8, true);
								goto err;
							}
							return i;
						}
					} while (i-- > start);
					goto notfound;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				temp = (*seq->tp_sizeob)(self);
				if unlikely(!temp)
					goto err;
				if (DeeObject_AsSize(temp, &seq_length))
					goto err_temp;
				Dee_Decref(temp);
				goto do_lookup_tpget;
			}
			if (seq->tp_iter) {
				/* Use iterators */
				if unlikely((iterator = (*seq->tp_iter)(self)) == NULL)
					goto err;
				result = iterator_rfind(iterator, keyed_search_item, start, end, key);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return (size_t)-2;
err_temp:
	Dee_Decref(temp);
	goto err;
notfound:
	return (size_t)-1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Join(DeeObject *self, DeeObject *items);
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Strip(DeeObject *self, DeeObject *elem, DeeObject *key);
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_LStrip(DeeObject *self, DeeObject *elem, DeeObject *key);
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RStrip(DeeObject *self, DeeObject *elem, DeeObject *key);
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Split(DeeObject *self, DeeObject *sep, DeeObject *key);

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Reversed(DeeObject *__restrict self) {
	DREF DeeObject *result;
	/* TODO: Proxy for sequences implementing index-based item access. */
	result = DeeList_FromSequence(self);
	if likely(result)
		DeeList_Reverse(result);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Sorted(DeeObject *self, DeeObject *key) {
	DREF DeeObject *result;
	/* TODO: Using lists for this is less than optimal... */
	result = DeeList_FromSequence(self);
	if unlikely(!result)
		goto done;
	if unlikely(DeeList_Sort(result, key))
		Dee_Clear(result);
done:
	return result;
}


/* Sequence functions. */
INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeSeq_CountSeq(DeeObject *self,
                DeeObject *seq,
                DeeObject *key) {
	(void)self;
	(void)seq;
	(void)key;
	/*
	 * >> function copyIterator(seq, iter, i) {
	 * >>     try {
	 * >>         return copy iter;
	 * >>     } catch (Error.RuntimeError.NotImplemented |
	 * >>              Error.ValueError) {
	 * >>         // Re-create the iterator.
	 * >>         local result = seq.operator __iter__();
	 * >>         while (i--) result.operator __next__();
	 * >>         return result;
	 * >>     }
	 * >> }
	 * >> function iterSame(a: Iterator, b: Iterator) {
	 * >>     try {
	 * >>         foreach (local elem_b: b) {
	 * >>             local elem_a = a.operator __next__();
	 * >>             if (key(elem_a) != key(elem_b))
	 * >>                 return false;
	 * >>         }
	 * >>     } catch (Signal.StopIteration) {
	 * >>         return false;
	 * >>     }
	 * >>     return true;
	 * >> }
	 * >> 
	 * >> local iter = self.operator __iter__();
	 * >> local head = seq.operator __iter__();
	 * >> local first;
	 * >> try {
	 * >>     first = head.operator __next__();
	 * >> } catch (Signal.StopIteration) {
	 * >>     return 0;
	 * >> }
	 * >> local result = 0;
	 * >> local i = 0;
	 * >> first = key(first);
	 * >> foreach (local elem: iter) {
	 * >>     ++i;
	 * >>     if (first != key(elem)) {
	 * >>         local aIter = copyIterator(self, iter, i);
	 * >>         local bIter = copyIterator(seq, head, 1);
	 * >>         if (iterSame(aIter, bIter))
	 * >>             ++result;
	 * >>     }
	 * >> }
	 * >> return result;
	 */
	DERROR_NOTIMPLEMENTED();
	return (size_t)-1;
}

/* TODO: */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_CountSeq(DeeObject *self, DeeObject *seq, DeeObject *key); /* @return: -1: Error. */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_ContainsSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Partition(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RPartition(DeeObject *self, DeeObject *keyed_search_item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_PartitionSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RPartitionSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_StartsWithSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_EndsWithSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_FindSeq(DeeObject *self, DeeObject *seq, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_RFindSeq(DeeObject *self, DeeObject *seq, DeeObject *key); /* @return: -1: Not found. @return: -2: Error. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_StripSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_LStripSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_RStripSeq(DeeObject *self, DeeObject *seq, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_SplitSeq(DeeObject *self, DeeObject *sep_seq, DeeObject *key);





/************************************************************************/
/* Sequence compare                                                     */
/************************************************************************/

/* @return: == Dee_COMPARE_ERR: An error occurred.
 * @return: == -1: `self < some_object'
 * @return: == 0:  Objects compare as equal
 * @return: == 1:  `self > some_object' */
INTERN WUNUSED int DCALL
DeeSeq_CompareVV(DeeObject *const *lhsv, size_t lhsc,
                 DeeObject *const *rhsv, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int diff;
		diff = DeeObject_Compare(lhsv[i], rhsv[i]);
		if (diff != 0)
			return diff;
	}
	return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_CompareVF(DeeObject *const *lhsv, size_t lhsc,
                 DeeObject *rhs, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int diff;
		DREF DeeObject *rhs_item;
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item)
			return Dee_COMPARE_ERR;
		diff = DeeObject_Compare(lhsv[i], rhs_item);
		Dee_Decref(rhs_item);
		if (diff != 0)
			return diff;
	}
	return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_CompareVI(DeeObject *const *lhsv, size_t lhsc,
                 DeeObject *rhs) {
	size_t i;
	DREF DeeObject *rhs_item;
	for (i = 0; i < lhsc; ++i) {
		int diff;
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			if (rhs_item == ITER_DONE)
				return 1; /* COUNT(LHS) > COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhsv[i], rhs_item);
		Dee_Decref(rhs_item);
		if (diff != 0)
			return diff;
	}
	rhs_item = DeeObject_IterNext(rhs);
	if (!ITER_ISOK(rhs_item)) {
		if (rhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(rhs_item);
	return -1; /* COUNT(LHS) < COUNT(RHS) */
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_CompareVS(DeeObject *const *lhsv, size_t lhsc,
                 DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_CompareVV(lhsv, lhsc, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result = DeeSeq_CompareVF(lhsv, lhsc, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			return Dee_COMPARE_ERR;
		result = DeeSeq_CompareVI(lhsv, lhsc, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_CompareFV(DeeObject *lhs, size_t lhsc,
                 DeeObject *const *rhsv, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int diff;
		DREF DeeObject *lhs_item;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return Dee_COMPARE_ERR;
		diff = DeeObject_Compare(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_CompareFF(DeeObject *lhs, size_t lhsc, DeeObject *rhs, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return Dee_COMPARE_ERR;
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item) {
			Dee_Decref(lhs_item);
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	return lhsc < rhsc ? -1 : lhsc > rhsc ? 1 : 0;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_CompareFI(DeeObject *lhs, size_t lhsc, DeeObject *rhs) {
	size_t i;
	DREF DeeObject *rhs_item;
	for (i = 0; i < lhsc; ++i) {
		DREF DeeObject *lhs_item;
		int diff;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return Dee_COMPARE_ERR;
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if (rhs_item == ITER_DONE)
				return 1; /* COUNT(LHS) > COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	rhs_item = DeeObject_IterNext(rhs);
	if (!ITER_ISOK(rhs_item)) {
		if (rhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(rhs_item);
	return -1; /* COUNT(LHS) < COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_CompareFS(DeeObject *lhs, size_t lhsc, DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_CompareFV(lhs, lhsc, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result = DeeSeq_CompareFF(lhs, lhsc, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			return Dee_COMPARE_ERR;
		result = DeeSeq_CompareFI(lhs, lhsc, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_CompareIV(DeeObject *lhs,
                 DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return -1; /* COUNT(LHS) < COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(lhs_item);
	return 1; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_CompareIF(DeeObject *lhs,
                 DeeObject *rhs, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return -1; /* COUNT(LHS) < COUNT(RHS) */
			return Dee_COMPARE_ERR;
		}
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item) {
			Dee_Decref(lhs_item);
			return Dee_COMPARE_ERR;
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 0; /* COUNT(LHS) == COUNT(RHS) */
		return Dee_COMPARE_ERR;
	}
	Dee_Decref(lhs_item);
	return 1; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_CompareII(DeeObject *lhs,
                 DeeObject *rhs) {
	for (;;) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if unlikely(lhs_item != ITER_DONE)
				return Dee_COMPARE_ERR;
			rhs_item = DeeObject_IterNext(rhs);
			if (!ITER_ISOK(rhs_item)) {
				if unlikely(rhs_item != ITER_DONE)
					return Dee_COMPARE_ERR;
				return 0; /* COUNT(LHS) == COUNT(RHS) */
			}
			Dee_Decref(rhs_item);
			return -1; /* COUNT(LHS) < COUNT(RHS) */
		}
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if unlikely(rhs_item != ITER_DONE)
				return Dee_COMPARE_ERR;
			return 1; /* COUNT(LHS) > COUNT(RHS) */
		}
		diff = DeeObject_Compare(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff != 0)
			return diff;
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_CompareIS(DeeObject *lhs,
                 DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_CompareIV(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result = DeeSeq_CompareIF(lhs, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter)
			return Dee_COMPARE_ERR;
		result = DeeSeq_CompareII(lhs, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}



/* @return: == -1: An error occurred.
 * @return: == 0:  Sequences differ
 * @return: == 1:  Sequences are equal */
INTERN WUNUSED int DCALL
DeeSeq_EqVV(DeeObject *const *lhsv,
            DeeObject *const *rhsv, size_t elemc) {
	size_t i;
	for (i = 0; i < elemc; ++i) {
		int diff;
		diff = DeeObject_TryCmpEqAsBool(lhsv[i], rhsv[i]);
		if (diff <= 0)
			return diff;
	}
	return 1;
}

INTERN WUNUSED NONNULL((2)) int DCALL
DeeSeq_EqVF(DeeObject *const *lhsv,
            DeeObject *rhs, size_t elemc) {
	size_t i;
	for (i = 0; i < elemc; ++i) {
		int diff;
		DREF DeeObject *rhs_item;
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item)
			return -1;
		diff = DeeObject_TryCmpEqAsBool(lhsv[i], rhs_item);
		Dee_Decref(rhs_item);
		if (diff <= 0)
			return diff;
	}
	return 1;
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_EqVI(DeeObject *const *lhsv, size_t lhsc,
            DeeObject *rhs) {
	size_t i;
	DREF DeeObject *rhs_item;
	for (i = 0; i < lhsc; ++i) {
		int diff;
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			if (rhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) > COUNT(RHS) */
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhsv[i], rhs_item);
		Dee_Decref(rhs_item);
		if (diff <= 0)
			return diff;
	}
	rhs_item = DeeObject_IterNext(rhs);
	if (!ITER_ISOK(rhs_item)) {
		if (rhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(rhs_item);
	return 0; /* COUNT(LHS) < COUNT(RHS) */
}

INTERN WUNUSED NONNULL((3)) int DCALL
DeeSeq_EqVS(DeeObject *const *lhsv, size_t lhsc,
            DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		if (lhsc != DeeTuple_SIZE(rhs))
			return 0;
		result = DeeSeq_EqVV(lhsv, DeeTuple_ELEM(rhs), lhsc);
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (lhsc != rhs_size)
			return 0;
		result = DeeSeq_EqVF(lhsv, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter) {
			if (DeeError_Catch(&DeeError_NotImplemented))
				return 0;
			return -1;
		}
		result = DeeSeq_EqVI(lhsv, lhsc, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_EqFV(DeeObject *lhs,
            DeeObject *const *rhsv, size_t elemc) {
	size_t i;
	for (i = 0; i < elemc; ++i) {
		int diff;
		DREF DeeObject *lhs_item;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return -1;
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqFF(DeeObject *lhs, DeeObject *rhs, size_t elemc) {
	size_t i;
	for (i = 0; i < elemc; ++i) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return -1;
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item) {
			Dee_Decref(lhs_item);
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_EqFI(DeeObject *lhs, size_t lhsc, DeeObject *rhs) {
	size_t i;
	DREF DeeObject *rhs_item;
	for (i = 0; i < lhsc; ++i) {
		DREF DeeObject *lhs_item;
		int diff;
		lhs_item = DeeFastSeq_GetItem_deprecated(lhs, i);
		if unlikely(!lhs_item)
			return -1;
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if (rhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) > COUNT(RHS) */
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	rhs_item = DeeObject_IterNext(rhs);
	if (!ITER_ISOK(rhs_item)) {
		if (rhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(rhs_item);
	return 0; /* COUNT(LHS) < COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
DeeSeq_EqFS(DeeObject *lhs, size_t lhsc, DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		if (DeeTuple_SIZE(rhs) != lhsc)
			return 0;
		result = DeeSeq_EqFV(lhs, DeeTuple_ELEM(rhs), lhsc);
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (rhs_size != lhsc)
			return 0;
		result = DeeSeq_EqFF(lhs, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter) {
			if (DeeError_Catch(&DeeError_NotImplemented))
				return 0;
			return -1;
		}
		result = DeeSeq_EqFI(lhs, lhsc, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}


INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_EqIV(DeeObject *lhs,
            DeeObject *const *rhsv, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) < COUNT(RHS) */
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhsv[i]);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(lhs_item);
	return 0; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqIF(DeeObject *lhs,
            DeeObject *rhs, size_t rhsc) {
	size_t i;
	DREF DeeObject *lhs_item;
	for (i = 0; i < rhsc; ++i) {
		int diff;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if (lhs_item == ITER_DONE)
				return 0; /* COUNT(LHS) < COUNT(RHS) */
			return -1;
		}
		rhs_item = DeeFastSeq_GetItem_deprecated(rhs, i);
		if unlikely(!rhs_item) {
			Dee_Decref(lhs_item);
			return -1;
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
	lhs_item = DeeObject_IterNext(lhs);
	if (!ITER_ISOK(lhs_item)) {
		if (lhs_item == ITER_DONE)
			return 1; /* COUNT(LHS) == COUNT(RHS) */
		return -1;
	}
	Dee_Decref(lhs_item);
	return 0; /* COUNT(LHS) > COUNT(RHS) */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqII(DeeObject *lhs, DeeObject *rhs) {
	for (;;) {
		int diff;
		DREF DeeObject *lhs_item;
		DREF DeeObject *rhs_item;
		lhs_item = DeeObject_IterNext(lhs);
		if unlikely(!ITER_ISOK(lhs_item)) {
			if unlikely(lhs_item != ITER_DONE)
				return -1;
			rhs_item = DeeObject_IterNext(rhs);
			if (!ITER_ISOK(rhs_item)) {
				if unlikely(rhs_item != ITER_DONE)
					return -1;
				return 1; /* COUNT(LHS) == COUNT(RHS) */
			}
			Dee_Decref(rhs_item);
			return 0; /* COUNT(LHS) < COUNT(RHS) */
		}
		rhs_item = DeeObject_IterNext(rhs);
		if unlikely(!ITER_ISOK(rhs_item)) {
			Dee_Decref(lhs_item);
			if unlikely(rhs_item != ITER_DONE)
				return -1;
			return 0; /* COUNT(LHS) > COUNT(RHS) */
		}
		diff = DeeObject_TryCmpEqAsBool(lhs_item, rhs_item);
		Dee_Decref(rhs_item);
		Dee_Decref(lhs_item);
		if (diff <= 0)
			return diff;
	}
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqIS(DeeObject *lhs, DeeObject *rhs) {
	int result;
	size_t rhs_size;
	if (DeeTuple_Check(rhs)) {
		result = DeeSeq_EqIV(lhs, DeeTuple_ELEM(rhs), DeeTuple_SIZE(rhs));
	} else if ((rhs_size = DeeFastSeq_GetSize_deprecated(rhs)) != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		result = DeeSeq_EqIF(lhs, rhs, rhs_size);
	} else {
		DREF DeeObject *rhs_iter;
		rhs_iter = DeeObject_Iter(rhs);
		if unlikely(!rhs_iter) {
			if (DeeError_Catch(&DeeError_NotImplemented))
				return 0;
			return -1;
		}
		result = DeeSeq_EqII(lhs, rhs_iter);
		Dee_Decref(rhs_iter);
	}
	return result;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C */
