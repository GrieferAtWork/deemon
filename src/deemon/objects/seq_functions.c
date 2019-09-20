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


INTERN WUNUSED NONNULL((1)) size_t DCALL DeeSeq_Size(DeeObject *__restrict self) {
	DREF DeeObject *iter, *elem;
	size_t result = 0;
	/* Count the number of elements, given an iterator. */
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter) {
		if (DeeError_Catch(&DeeError_NotImplemented))
			err_unimplemented_operator(Dee_TYPE(self), OPERATOR_SIZE);
		return (size_t)-1;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		Dee_Decref(elem);
		if unlikely(result == (size_t)-2) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			return (size_t)-1;
		}
		++result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	Dee_Decref(iter);
	if likely(elem)
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
	if (elem)
		err_index_out_of_bounds(sequence, index, current_index);
	return NULL;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_GetItem(DeeObject *__restrict self, size_t index) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if unlikely(tp_self == &DeeSeq_Type) {
		err_index_out_of_bounds(self, index, 0);
		goto err;
	}
	for (;;) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi *nsi;
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
				result = (*seq->tp_get)(self, index_ob);
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
				result = (*seq->tp_range_get)(self, index_ob, index_plus1_ob);
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
			if (seq->tp_iter_self) {
				DREF DeeObject *iterator;
				iterator = (*seq->tp_iter_self)(self);
				if unlikely(!iterator)
					goto err;
				result = iterator_get_nth(iterator, self, index);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeType_Base(tp_self)) == NULL)
			break;
		if (tp_self == &DeeSeq_Type)
			break;
	}
	err_unimplemented_operator3(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_GETRANGE,
	                            OPERATOR_ITERSELF);
err:
	return NULL;
}

PRIVATE ATTR_COLD int DCALL
err_no_generic_sequence(DeeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_NotImplemented,
	                       "Neither `%k.__getitem__' and `%k.__size__', nor `%k.__iter__' are implemented",
	                       Dee_TYPE(self), Dee_TYPE(self), Dee_TYPE(self));
}




INTERN WUNUSED NONNULL((1)) int DCALL
DeeSeq_NonEmpty(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	int result;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	if unlikely(tp_self == &DeeSeq_Type)
		return 0;
	for (;;) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi *nsi;
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
				temp = (*seq->tp_size)(self);
				if unlikely(!temp)
					goto err;
				result = DeeObject_Bool(temp);
				Dee_Decref(temp);
				if unlikely(result < 0)
					goto err;
				return result;
			}
			if (seq->tp_iter_self) {
				DREF DeeObject *elem;
				temp = (*seq->tp_iter_self)(self);
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
		if ((tp_self = DeeType_Base(tp_self)) == NULL)
			break;
		if (tp_self == &DeeSeq_Type)
			break;
	}
	err_unimplemented_operator2(Dee_TYPE(self),
	                            OPERATOR_SIZE,
	                            OPERATOR_ITERSELF);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Front(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi *nsi;
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
				result = (*seq->tp_get)(self, &DeeInt_Zero);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_IndexError))
						goto err_empty;
				}
				return result;
			}
			if (seq->tp_iter_self) {
				temp = (*seq->tp_iter_self)(self);
				if unlikely(!temp)
					goto err;
				result = DeeObject_IterNext(temp);
				Dee_Decref(temp);
				if (result == ITER_DONE)
					goto err_empty;
				return result;
			}
		}
		if ((tp_self = DeeType_Base(tp_self)) == NULL)
			break;
	}
	err_unimplemented_operator2(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_ITERSELF);
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
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi *nsi;
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
					result = (*seq->tp_get)(self, temp);
					Dee_Decref(temp);
					return result;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				int size_is_nonzero;
				temp = (*seq->tp_size)(self);
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
				result = (*seq->tp_get)(self, temp);
				Dee_Decref(temp);
				return result;
			}
			if (seq->tp_iter_self) {
				DREF DeeObject *next;
				temp = (*seq->tp_iter_self)(self);
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
		if ((tp_self = DeeType_Base(tp_self)) == NULL)
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
DeeSeq_Reduce(DeeObject *__restrict self,
              DeeObject *__restrict combine,
              DeeObject *__restrict init) {
	DREF DeeObject *iterator, *elem, *merge;
	DREF DeeObject *result = init;
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		return NULL;
	Dee_XIncref(result);
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		/* Special handling for the first element. */
		if (!result)
			result = elem;
		else {
			/* Invoke the given combination-callback to merge the 2 items. */
			merge = DeeObject_CallPack(combine, 2, result, elem);
			Dee_Decref(elem);
			Dee_Decref(result);
			/* Check for errors. */
			if unlikely(!merge) {
				Dee_Decref(iterator);
				return NULL;
			}
			result = merge;
		}
		if (DeeThread_CheckInterrupt()) {
			Dee_Decref(result);
			return NULL;
		}
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
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Sum(DeeObject *__restrict self) {
	DREF DeeObject *iterator, *elem, *merge;
	DREF DeeObject *result = NULL;
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		return NULL;
	/* Yield the first item to perform type-specific optimizations. */
	result = DeeObject_IterNext(iterator);
	if (!ITER_ISOK(result)) {
		/* Special case: empty sequence. */
		if unlikely(!result)
			goto err_iter;
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
			return NULL;
		}
		/* Use a unicode printer. */
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
			return NULL;
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
	Dee_Decref(iterator);
	if unlikely(!elem)
		Dee_Clear(result);
	return result;
err_iter_r:
	Dee_Decref(result);
err_iter:
	Dee_Decref(iterator);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSeq_Any(DeeObject *__restrict self) {
	DREF DeeObject *iterator, *elem;
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
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
	if likely(elem)
		return 0;
err:
	return -1;
err_iter:
	Dee_Decref(iterator);
	goto err;
}

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSeq_All(DeeObject *__restrict self) {
	DREF DeeObject *iterator, *elem;
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
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
	if likely(elem)
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
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
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
	return (unlikely(!elem))
	       ? -1
	       : result;
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
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!result)
			result = elem;
		else {
			DREF DeeObject *key_elem;
			if (!key_result) {
				key_result = DeeObject_Call(key, 1, &result);
				if unlikely(!key_result)
					goto err_r_iter_elem;
			}
			key_elem = DeeObject_Call(key, 1, &elem);
			if unlikely(!key_elem)
				goto err_r_iter_elem;
			temp = DeeObject_CompareLo(key_result, key_elem);
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
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!result)
			result = elem;
		else {
			DREF DeeObject *key_elem;
			if (!key_result) {
				key_result = DeeObject_Call(key, 1, &result);
				if unlikely(!key_result)
					goto err_r_iter_elem;
			}
			key_elem = DeeObject_Call(key, 1, &elem);
			if unlikely(!key_elem)
				goto err_r_iter_elem;
			temp = DeeObject_CompareLo(key_result, key_elem);
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

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeSeq_Min(DeeObject *__restrict self, DeeObject *key) {
	DREF DeeObject *elem, *iterator, *result = NULL;
	int temp;
	if (key)
		return DeeSeq_Min_k(self, key);
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!result)
			result = elem;
		else {
			temp = DeeObject_CompareLo(result, elem);
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

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeSeq_Max(DeeObject *__restrict self, DeeObject *key) {
	DREF DeeObject *elem, *iterator, *result = NULL;
	int temp;
	if (key)
		return DeeSeq_Max_k(self, key);
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if (!result)
			result = elem;
		else {
			temp = DeeObject_CompareLo(result, elem);
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


INTERN size_t DCALL
DeeSeq_Count(DeeObject *__restrict self,
             DeeObject *__restrict keyed_search_item,
             DeeObject *key) {
	size_t result = 0;
	int temp;
	DREF DeeObject *elem, *iterator;
	/* TODO: NSI Variant + index-based sequence optimizations */
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
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

INTERN WUNUSED DREF DeeObject *DCALL
DeeSeq_Locate(DeeObject *__restrict self,
              DeeObject *__restrict keyed_search_item,
              DeeObject *key) {
	DREF DeeObject *elem, *iterator;
	int temp;
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		return NULL;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
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
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeSeq_RLocate(DeeObject *__restrict self,
               DeeObject *__restrict keyed_search_item,
               DeeObject *key) {
	DREF DeeObject *elem, *iterator, *result = NULL;
	int temp;
	if unlikely((iterator = DeeObject_IterSelf(self)) == NULL)
		return NULL;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err_elem;
			/* Found one! */
			Dee_Decref(iterator);
			Dee_XDecref(result);
			result = elem;
		}
		Dee_Decref(elem);
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
	return NULL;
}

INTERN int DCALL
DeeSeq_Contains(DeeObject *__restrict self,
                DeeObject *__restrict keyed_search_item,
                DeeObject *key) {
	DREF DeeObject *iter, *elem;
	int temp;
	iter = DeeObject_IterSelf(self);
	if unlikely(!iter)
		return -1;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		temp = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
		if (temp != 0) {
			Dee_Decref(iter);
			return temp;
		}
		Dee_Decref(elem);
	}
	Dee_Decref(iter);
	return (likely(elem))
	       ? 0
	       : -1;
}


INTERN int DCALL
DeeSeq_StartsWith(DeeObject *__restrict self,
                  DeeObject *__restrict keyed_search_item,
                  DeeObject *key) {
	DeeTypeObject *tp_self;
	DREF DeeObject *result;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi *nsi;
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
				error = DeeObject_CompareKeyEq(keyed_search_item, result, key);
				Dee_Decref(result);
				return error;
			}
			if (has_noninherited_getitem(tp_self, seq)) {
				result = (*seq->tp_get)(self, &DeeInt_Zero);
				if unlikely(!result) {
					if (DeeError_Catch(&DeeError_IndexError))
						goto err_empty;
					goto err;
				}
				goto check;
			}
			if (seq->tp_iter_self) {
				temp = (*seq->tp_iter_self)(self);
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
		if ((tp_self = DeeType_Base(tp_self)) == NULL)
			break;
	}
	err_unimplemented_operator2(Dee_TYPE(self),
	                            OPERATOR_GETITEM,
	                            OPERATOR_ITERSELF);
err:
	return -1;
err_empty:
	return 0;
}

INTERN int DCALL
DeeSeq_EndsWith(DeeObject *__restrict self,
                DeeObject *__restrict keyed_search_item,
                DeeObject *key) {
	DeeTypeObject *tp_self;
	size_t seq_length;
	DREF DeeObject *result, *temp;
	ASSERT_OBJECT(self);
	tp_self = Dee_TYPE(self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq;
		if ((seq = tp_self->tp_seq) != NULL) {
			struct type_nsi *nsi;
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
					if (nsi->nsi_seqlike.nsi_getitem_fast)
						result = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, seq_length - 1);
					else {
						result = (*nsi->nsi_seqlike.nsi_getitem)(self, seq_length - 1);
					}
					if unlikely(!result)
						goto err;
check:
					error = DeeObject_CompareKeyEq(keyed_search_item, result, key);
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
					result = (*seq->tp_get)(self, temp);
					Dee_Decref(temp);
					if unlikely(!result)
						goto err;
					goto check;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				int size_is_nonzero;
				temp = (*seq->tp_size)(self);
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
				result = (*seq->tp_get)(self, temp);
				Dee_Decref(temp);
				if unlikely(!result)
					goto err;
				goto check;
			}
			if (seq->tp_iter_self) {
				DREF DeeObject *next;
				temp = (*seq->tp_iter_self)(self);
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
		if ((tp_self = DeeType_Base(tp_self)) == NULL)
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

PRIVATE size_t DCALL
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
			return (size_t)-1;
		}
		Dee_Decref(elem);
		--start;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
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
			goto not_found; /* End of search size */
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
not_found:
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
			return (size_t)-1;
		}
		Dee_Decref(elem);
		--start;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_CompareKeyEq(keyed_search_item, elem, key);
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
}


INTERN size_t DCALL
DeeSeq_Find(DeeObject *__restrict self,
            size_t start, size_t end,
            DeeObject *__restrict keyed_search_item,
            DeeObject *key) {
	DREF DeeObject *iterator;
	size_t result;
	DeeTypeObject *tp_self;
	size_t i, seq_length;
	DREF DeeObject *temp;
	int error;
	ASSERT_OBJECT(self);
	if unlikely(start >= end)
		return (size_t)-1;
	tp_self = Dee_TYPE(self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq) {
			struct type_nsi *nsi = seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_find)
					return (*nsi->nsi_seqlike.nsi_find)(self, start, end, keyed_search_item, key);
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start >= seq_length)
						return (size_t)-1;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						temp = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, i);
						if unlikely(!temp)
							continue;
						error = DeeObject_CompareKeyEq(keyed_search_item, temp, key);
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
					return (size_t)-1;
				} else if (nsi->nsi_seqlike.nsi_getitem) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
					if (start >= seq_length)
						return (size_t)-1;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						temp = (*nsi->nsi_seqlike.nsi_getitem)(self, i);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_CompareKeyEq(keyed_search_item, temp, key);
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
					return (size_t)-1;
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
do_lookup_tpget:
					if (start >= seq_length)
						return (size_t)-1;
					if (end > seq_length)
						end = seq_length;
					for (i = start; i < end; ++i) {
						DREF DeeObject *index_ob;
						index_ob = DeeInt_NewSize(i);
						if unlikely(!index_ob)
							goto err;
						temp = (*seq->tp_get)(self, index_ob);
						Dee_Decref(index_ob);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_CompareKeyEq(keyed_search_item, temp, key);
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
					return (size_t)-1;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				temp = (*seq->tp_size)(self);
				if unlikely(!temp)
					goto err;
				if (DeeObject_AsSize(temp, &seq_length))
					goto err_temp;
				Dee_Decref(temp);
				goto do_lookup_tpget;
			}
			if (seq->tp_iter_self) {
				/* Use iterators */
				if unlikely((iterator = (*seq->tp_iter_self)(self)) == NULL)
					goto err;
				result = iterator_find(iterator, keyed_search_item, start, end, key);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeType_Base(tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return (size_t)-2;
err_temp:
	Dee_Decref(temp);
	goto err;
}

INTERN size_t DCALL
DeeSeq_RFind(DeeObject *__restrict self,
             size_t start, size_t end,
             DeeObject *__restrict keyed_search_item,
             DeeObject *key) {
	DREF DeeObject *iterator;
	size_t result;
	DeeTypeObject *tp_self;
	size_t i, seq_length;
	DREF DeeObject *temp;
	int error;
	ASSERT_OBJECT(self);
	if unlikely(start >= end)
		return (size_t)-1;
	tp_self = Dee_TYPE(self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq) {
			struct type_nsi *nsi = seq->tp_nsi;
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
						error = DeeObject_CompareKeyEq(keyed_search_item, temp, key);
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
					return (size_t)-1;
				} else if (nsi->nsi_seqlike.nsi_getitem) {
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
						error = DeeObject_CompareKeyEq(keyed_search_item, temp, key);
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
					return (size_t)-1;
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
						temp = (*seq->tp_get)(self, index_ob);
						Dee_Decref(index_ob);
						if unlikely(!temp) {
							if (DeeError_Catch(&DeeError_UnboundItem))
								continue;
							goto err;
						}
						error = DeeObject_CompareKeyEq(keyed_search_item, temp, key);
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
					return (size_t)-1;
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				temp = (*seq->tp_size)(self);
				if unlikely(!temp)
					goto err;
				if (DeeObject_AsSize(temp, &seq_length))
					goto err_temp;
				Dee_Decref(temp);
				goto do_lookup_tpget;
			}
			if (seq->tp_iter_self) {
				/* Use iterators */
				if unlikely((iterator = (*seq->tp_iter_self)(self)) == NULL)
					goto err;
				result = iterator_rfind(iterator, keyed_search_item, start, end, key);
				Dee_Decref(iterator);
				return result;
			}
		}
		if ((tp_self = DeeType_Base(tp_self)) == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return (size_t)-2;
err_temp:
	Dee_Decref(temp);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Join(DeeObject *self, DeeObject *items);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_Strip(DeeObject *__restrict self, DeeObject *__restrict elem, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_LStrip(DeeObject *__restrict self, DeeObject *__restrict elem, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_RStrip(DeeObject *__restrict self, DeeObject *__restrict elem, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_Split(DeeObject *__restrict self, DeeObject *__restrict sep, DeeObject *key);

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Reversed(DeeObject *__restrict self) {
	DREF DeeObject *result;
	/* TODO: Proxy for sequences implementing index-based item access. */
	result = DeeList_FromSequence(self);
	if likely(result)
		DeeList_Reverse(result);
	return result;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeSeq_Sorted(DeeObject *__restrict self,
              DeeObject *key) {
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


INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_Partition(DeeObject *__restrict self, DeeObject *__restrict elem, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_PartitionSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_RPartition(DeeObject *__restrict self, DeeObject *__restrict elem, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_RPartitionSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN int DCALL DeeSeq_StartsWithSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN int DCALL DeeSeq_EndsWithSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN size_t DCALL DeeSeq_FindSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN size_t DCALL DeeSeq_RFindSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_StripSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_LStripSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_RStripSeq(DeeObject *__restrict self, DeeObject *__restrict seq, DeeObject *key);
INTERN WUNUSED DREF DeeObject *DCALL DeeSeq_SplitSeq(DeeObject *__restrict self, DeeObject *__restrict sep_seq, DeeObject *key);

INTERN size_t DCALL
DeeSeq_CountSeq(DeeObject *__restrict self,
                DeeObject *__restrict seq,
                DeeObject *key) {
	(void)self;
	(void)seq;
	(void)key;
	/*
	 * >> function copy_iterator(seq,iter,i) {
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
	 * >> function iter_same(a,b) {
	 * >>     try {
	 * >>         foreach(local elem_b: b) {
	 * >>             local elem_a = a.operator __next__();
	 * >>             if (!key(elem_a,elem_b))
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
	 * >> foreach(local elem: iter) {
	 * >>     ++i;
	 * >>     if (key(elem,first)) {
	 * >>         if (iter_same(copy_iterator(self,iter,i),
	 * >>                       copy_iterator(seq,head,1)))
	 * >>             ++result;
	 * >>     }
	 * >> }
	 * >> return result;
	 */
	DERROR_NOTIMPLEMENTED();
	return (size_t)-1;
}





/* General-purpose iterator/sequence compare functions.
 * NOTE: The iterator-compare functions compare the _ELEMENTS_
 *       they yield, not the abstract iterator positions. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeIter_Lo(DeeObject *__restrict lhs,
           DeeObject *__restrict rhs) {
	DREF DeeObject *lhs_elem, *rhs_elem;
	int result;
	/* TODO: Recursive sequence objects? */
	ASSERT_OBJECT(lhs);
	ASSERT_OBJECT(rhs);
	do {
		if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs))) {
			if unlikely(!lhs_elem)
				goto err;
			if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs)))
				return (unlikely(!rhs_elem))
				       ? -1
				       : 0; /* size:equal */
			Dee_Decref(rhs_elem);
			return 1; /* size:lower */
		}
		if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs))) {
			Dee_Decref(lhs_elem);
			return (unlikely(!rhs_elem))
			       ? -1
			       : 0; /* size:greater */
		}
		result = DeeObject_CompareLo(lhs_elem, rhs_elem);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhs_elem, lhs_elem);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (DeeThread_CheckInterrupt())
			goto err;
	} while (result == 0);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeIter_Le(DeeObject *__restrict lhs,
           DeeObject *__restrict rhs) {
	DREF DeeObject *lhs_elem, *rhs_elem;
	int result;
	/* TODO: Recursive sequence objects? */
	ASSERT_OBJECT(lhs);
	ASSERT_OBJECT(rhs);
	do {
		if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs)))
			return (unlikely(!lhs_elem))
			       ? -1
			       : 1; /* size:equal_or_lower */
		if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs))) {
			Dee_Decref(lhs_elem);
			return (unlikely(!rhs_elem))
			       ? -1
			       : 0; /* size:greater */
		}
		result = DeeObject_CompareLo(lhs_elem, rhs_elem);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhs_elem, lhs_elem);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (DeeThread_CheckInterrupt())
			goto err;
	} while (result == 0);
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeIter_Eq(DeeObject *__restrict lhs,
           DeeObject *__restrict rhs) {
	DREF DeeObject *lhs_elem, *rhs_elem;
	int result;
	/* TODO: Recursive sequence objects? */
	ASSERT_OBJECT(lhs);
	ASSERT_OBJECT(rhs);
	do {
		if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs))) {
			if unlikely(!lhs_elem)
				goto err;
			if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs)))
				return (unlikely(!rhs_elem))
				       ? -1
				       : 1; /* size:equal */
			Dee_Decref(rhs_elem);
			return 0; /* size:lower */
		}
		if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs))) {
			Dee_Decref(lhs_elem);
			return (unlikely(!rhs_elem))
			       ? -1
			       : 0; /* size:greater */
		}
		result = DeeObject_CompareEq(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (DeeThread_CheckInterrupt())
			goto err;
	} while (result > 0);
	return result;
err:
	return -1;
}

INTERN int DCALL
DeeSeq_EqVV(DeeObject **__restrict lhsv,
            DeeObject **__restrict rhsv,
            size_t elemc) {
	size_t i;
	int temp;
	for (i = 0; i < elemc; ++i) {
		temp = DeeObject_CompareEq(lhsv[i], rhsv[i]);
		if (temp <= 0)
			goto err;
	}
	return 1;
err:
	return temp;
}

INTERN int DCALL
DeeSeq_EqVF(DeeObject **__restrict lhsv,
            DeeObject *__restrict rhs,
            size_t elemc) {
	size_t i;
	int temp;
	for (i = 0; i < elemc; ++i) {
		DREF DeeObject *rhs_elem;
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem)
			return -1;
		temp = DeeObject_CompareEq(lhsv[i], rhs_elem);
		Dee_Decref(rhs_elem);
		if (temp <= 0)
			goto err;
	}
	return 1;
err:
	return temp;
}

INTERN int DCALL
DeeSeq_EqVI(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject *__restrict rhs) {
	size_t i;
	int temp;
	DREF DeeObject *rhs_elem;
	for (i = 0; i < lhsc; ++i) {
		rhs_elem = DeeObject_IterNext(rhs);
		if (!ITER_ISOK(rhs_elem))
			return rhs_elem ? 0 : -1;
		temp = DeeObject_CompareEq(lhsv[i], rhs_elem);
		Dee_Decref(rhs_elem);
		if (temp <= 0)
			goto err;
	}
	rhs_elem = DeeObject_IterNext(rhs);
	if (rhs_elem != ITER_DONE) {
		if unlikely(!rhs_elem)
			return -1;
		Dee_Decref(rhs_elem);
		return 0;
	}
	return 1;
err:
	return temp;
}

INTERN int DCALL
DeeSeq_EqVS(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject *__restrict seq) {
	int result;
	size_t fast_size;
	if (DeeTuple_Check(seq)) {
		if (lhsc != DeeTuple_SIZE(seq))
			return 0;
		return DeeSeq_EqVV(lhsv, DeeTuple_ELEM(seq), lhsc);
	}
	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		if (lhsc != fast_size)
			return 0;
		return DeeSeq_EqVF(lhsv, seq, lhsc);
	}
	seq = DeeObject_IterSelf(seq);
	if unlikely(!seq)
		return -1;
	result = DeeSeq_EqVI(lhsv, lhsc, seq);
	Dee_Decref(seq);
	return result;
}

INTERN int DCALL
DeeSeq_LoVV(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject **__restrict rhsv, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int temp = DeeObject_CompareLo(lhsv[i], rhsv[i]);
		if (temp <= 0) { /* *lhs < *rhs : false */
			if unlikely(temp < 0)
				return temp;
			temp = DeeObject_CompareLo(rhsv[i], lhsv[i]);
			if (temp != 0) { /* *rhs < *lhs : true */
				if unlikely(temp < 0)
					return temp;
				return 0; /* return false */
			}
		}
	}
	return lhsc < rhsc;
}

INTERN int DCALL
DeeSeq_LoVF(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject *__restrict rhs, size_t rhsc) {
	int temp;
	DREF DeeObject *rhs_elem;
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem)
			return -1;
		temp = DeeObject_CompareLo(lhsv[i], rhs_elem);
		if (temp <= 0) { /* *lhs < *rhs : false */
			if unlikely(temp < 0)
				goto err_rhs_elem;
			temp = DeeObject_CompareLo(rhs_elem, lhsv[i]);
			if (temp != 0) { /* *rhs < *lhs : true */
				if unlikely(temp < 0)
					goto err_rhs_elem;
				Dee_Decref(rhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
	}
	return lhsc < rhsc;
err_rhs_elem:
	Dee_Decref(rhs_elem);
	return temp;
}

INTERN int DCALL
DeeSeq_LoVI(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject *__restrict rhs) {
	DREF DeeObject *rhs_elem;
	int result;
	size_t i;
	for (i = 0; i < lhsc; ++i) {
		if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs)))
			return (unlikely(!rhs_elem))
			       ? -1
			       : 0; /* size:greater */
		result = DeeObject_CompareLo(lhsv[i], rhs_elem);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhs_elem, lhsv[i]);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		if (result != 0)
			return result;
	}
	if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs)))
		return (unlikely(!rhs_elem))
		       ? -1
		       : 0; /* size:equal */
	Dee_Decref(rhs_elem);
	return 1; /* size:lower */
}

INTERN int DCALL
DeeSeq_LoVS(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject *__restrict seq) {
	int result;
	size_t fast_size;
	if (DeeTuple_Check(seq))
		return DeeSeq_LoVV(lhsv, lhsc, DeeTuple_ELEM(seq), DeeTuple_SIZE(seq));
	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST)
		return DeeSeq_LoVF(lhsv, lhsc, seq, fast_size);
	seq = DeeObject_IterSelf(seq);
	if unlikely(!seq)
		return -1;
	result = DeeSeq_LoVI(lhsv, lhsc, seq);
	Dee_Decref(seq);
	return result;
}

INTERN int DCALL
DeeSeq_LeVV(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject **__restrict rhsv, size_t rhsc) {
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		int temp = DeeObject_CompareLo(lhsv[i], rhsv[i]);
		if (temp <= 0) { /* *lhs < *rhs : false */
			if unlikely(temp < 0)
				return temp;
			temp = DeeObject_CompareLo(rhsv[i], lhsv[i]);
			if (temp != 0) { /* *rhs < *lhs : true */
				if unlikely(temp < 0)
					return temp;
				return 0; /* return false */
			}
		}
	}
	return lhsc <= rhsc;
}

INTERN int DCALL
DeeSeq_LeVF(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject *__restrict rhs, size_t rhsc) {
	int temp;
	DREF DeeObject *rhs_elem;
	size_t i, common = MIN(lhsc, rhsc);
	for (i = 0; i < common; ++i) {
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem)
			return -1;
		temp = DeeObject_CompareLo(lhsv[i], rhs_elem);
		if (temp <= 0) { /* *lhs < *rhs : false */
			if unlikely(temp < 0)
				goto err_rhs_elem;
			temp = DeeObject_CompareLo(rhs_elem, lhsv[i]);
			if (temp != 0) { /* *rhs < *lhs : true */
				if unlikely(temp < 0)
					goto err_rhs_elem;
				Dee_Decref(rhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
	}
	return lhsc <= rhsc;
err_rhs_elem:
	Dee_Decref(rhs_elem);
	return temp;
}

INTERN int DCALL
DeeSeq_LeVI(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject *__restrict rhs) {
	DREF DeeObject *rhs_elem;
	int result;
	size_t i;
	for (i = 0; i < lhsc; ++i) {
		if (!ITER_ISOK(rhs_elem = DeeObject_IterNext(rhs)))
			return (unlikely(!rhs_elem))
			       ? -1
			       : 0; /* size:greater */
		result = DeeObject_CompareLo(lhsv[i], rhs_elem);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhs_elem, lhsv[i]);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		if (result != 0)
			return result;
	}
	return 1; /* size:equal_or_lower */
}

INTERN int DCALL
DeeSeq_LeVS(DeeObject **__restrict lhsv, size_t lhsc,
            DeeObject *__restrict seq) {
	int result;
	size_t fast_size;
	if (DeeTuple_Check(seq))
		return DeeSeq_LeVV(lhsv, lhsc, DeeTuple_ELEM(seq), DeeTuple_SIZE(seq));
	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST)
		return DeeSeq_LeVF(lhsv, lhsc, seq, fast_size);
	seq = DeeObject_IterSelf(seq);
	if unlikely(!seq)
		return -1;
	result = DeeSeq_LeVI(lhsv, lhsc, seq);
	Dee_Decref(seq);
	return result;
}


INTERN int DCALL
DeeSeq_EqIV(DeeObject *__restrict lhs,
            DeeObject **__restrict rhsv,
            size_t rhsc) {
	size_t i;
	int temp;
	DREF DeeObject *lhs_elem;
	for (i = 0; i < rhsc; ++i) {
		lhs_elem = DeeObject_IterNext(lhs);
		if (!ITER_ISOK(lhs_elem))
			return lhs_elem ? 0 : -1;
		temp = DeeObject_CompareEq(lhs_elem, rhsv[i]);
		Dee_Decref(lhs_elem);
		if (temp <= 0)
			goto err;
	}
	lhs_elem = DeeObject_IterNext(lhs);
	if (lhs_elem != ITER_DONE) {
		if unlikely(!lhs_elem)
			return -1;
		Dee_Decref(lhs_elem);
		return 0;
	}
	return 1;
err:
	return temp;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_EqIF(DeeObject *__restrict lhs,
            DeeObject *__restrict rhs,
            size_t rhsc) {
	size_t i;
	int temp;
	DREF DeeObject *lhs_elem, *rhs_elem;
	for (i = 0; i < rhsc; ++i) {
		lhs_elem = DeeObject_IterNext(lhs);
		if (!ITER_ISOK(lhs_elem))
			return lhs_elem ? 0 : -1;
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_Decref(lhs_elem);
			return -1;
		}
		temp = DeeObject_CompareEq(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp <= 0)
			goto err;
		if (DeeThread_CheckInterrupt())
			goto err_m1;
	}
	lhs_elem = DeeObject_IterNext(lhs);
	if (lhs_elem != ITER_DONE) {
		if unlikely(!lhs_elem)
			return -1;
		Dee_Decref(lhs_elem);
		return 0;
	}
	return 1;
err_m1:
	temp = -1;
err:
	return temp;
}

INTERN int DCALL
DeeSeq_LoIV(DeeObject *__restrict lhs,
            DeeObject **__restrict rhsv,
            size_t rhsc) {
	DREF DeeObject *lhs_elem;
	int result;
	size_t i;
	ASSERT_OBJECT(lhs);
	for (i = 0; i < rhsc; ++i) {
		if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs))) {
			if unlikely(!lhs_elem)
				return -1;
			return 1; /* size:lower */
		}
		result = DeeObject_CompareLo(lhs_elem, rhsv[i]);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhsv[i], lhs_elem);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(lhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(lhs_elem);
		if (result != 0)
			return result;
	}
	return 0; /* size:greater */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_LoIF(DeeObject *__restrict lhs,
            DeeObject *__restrict rhs,
            size_t rhsc) {
	DREF DeeObject *lhs_elem, *rhs_elem;
	int result;
	size_t i;
	ASSERT_OBJECT(lhs);
	for (i = 0; i < rhsc; ++i) {
		if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs))) {
			if unlikely(!lhs_elem)
				goto err;
			return 1; /* size:lower */
		}
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_Decref(lhs_elem);
			goto err;
		}
		result = DeeObject_CompareLo(lhs_elem, rhs_elem);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhs_elem, lhs_elem);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (result != 0)
			return result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return 0; /* size:greater */
err:
	return -1;
}

INTERN int DCALL
DeeSeq_LeIV(DeeObject *__restrict lhs,
            DeeObject **__restrict rhsv,
            size_t rhsc) {
	DREF DeeObject *lhs_elem;
	int result;
	size_t i;
	ASSERT_OBJECT(lhs);
	for (i = 0; i < rhsc; ++i) {
		if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs))) {
			if unlikely(!lhs_elem)
				goto err;
			return 1; /* size:lower */
		}
		result = DeeObject_CompareLo(lhs_elem, rhsv[i]);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhsv[i], lhs_elem);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(lhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(lhs_elem);
		if (result != 0)
			return result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs))) {
		if unlikely(!lhs_elem)
			goto err;
		return 1; /* size:equal */
	}
	Dee_Decref(lhs_elem);
	return 0; /* size:greater */
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_LeIF(DeeObject *__restrict lhs,
            DeeObject *__restrict rhs,
            size_t rhsc) {
	DREF DeeObject *lhs_elem, *rhs_elem;
	int result;
	size_t i;
	ASSERT_OBJECT(lhs);
	for (i = 0; i < rhsc; ++i) {
		if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs))) {
			if unlikely(!lhs_elem)
				goto err;
			return 1; /* size:lower */
		}
		rhs_elem = DeeFastSeq_GetItem(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_Decref(lhs_elem);
			goto err;
		}
		result = DeeObject_CompareLo(lhs_elem, rhs_elem);
		if (result == 0) { /* *lhs < *rhs : false */
			result = DeeObject_CompareLo(rhs_elem, lhs_elem);
			if (result > 0) { /* *rhs < *lhs : true */
				Dee_Decref(rhs_elem);
				Dee_Decref(lhs_elem);
				return 0; /* return false */
			}
		}
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (result != 0)
			return result;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if (!ITER_ISOK(lhs_elem = DeeObject_IterNext(lhs))) {
		if unlikely(!lhs_elem)
			goto err;
		return 1; /* size:equal */
	}
	Dee_Decref(lhs_elem);
	return 0; /* size:greater */
err:
	return -1;
}




/* Perform a generic sequence comparison. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_Eq(DeeObject *__restrict self,
          DeeObject *__restrict some_object) {
	DeeObject *iter_self, *iter_rhs;
	int result;
	size_t other_size;
	iter_self = DeeObject_IterSelf(self);
	if unlikely(!iter_self)
		return -1;
	if (DeeTuple_Check(some_object)) {
		result = DeeSeq_EqIV(iter_self,
		                     DeeTuple_ELEM(some_object),
		                     DeeTuple_SIZE(some_object));
	} else if ((other_size = DeeFastSeq_GetSize(some_object)) != DEE_FASTSEQ_NOTFAST) {
		result = DeeSeq_EqIF(iter_self, some_object, other_size);
	} else {
		iter_rhs = DeeObject_IterSelf(some_object);
		if unlikely(!iter_rhs) {
			result = -1;
			goto err1;
		}
		result = DeeIter_Eq(iter_self, iter_rhs);
		Dee_Decref(iter_rhs);
	}
err1:
	Dee_Decref(iter_self);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_Lo(DeeObject *__restrict self,
          DeeObject *__restrict some_object) {
	DeeObject *iter_self, *iter_rhs;
	int result;
	size_t other_size;
	iter_self = DeeObject_IterSelf(self);
	if unlikely(!iter_self)
		return -1;
	if (DeeTuple_Check(some_object)) {
		result = DeeSeq_LoIV(iter_self,
		                     DeeTuple_ELEM(some_object),
		                     DeeTuple_SIZE(some_object));
	} else if ((other_size = DeeFastSeq_GetSize(some_object)) != DEE_FASTSEQ_NOTFAST) {
		result = DeeSeq_LoIF(iter_self, some_object, other_size);
	} else {
		iter_rhs = DeeObject_IterSelf(some_object);
		if unlikely(!iter_rhs) {
			result = -1;
			goto err1;
		}
		result = DeeIter_Lo(iter_self, iter_rhs);
		Dee_Decref(iter_rhs);
	}
err1:
	Dee_Decref(iter_self);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeSeq_Le(DeeObject *__restrict self,
          DeeObject *__restrict some_object) {
	DeeObject *iter_self, *iter_rhs;
	int result;
	size_t other_size;
	iter_self = DeeObject_IterSelf(self);
	if unlikely(!iter_self)
		return -1;
	if (DeeTuple_Check(some_object)) {
		result = DeeSeq_LeIV(iter_self,
		                     DeeTuple_ELEM(some_object),
		                     DeeTuple_SIZE(some_object));
	} else if ((other_size = DeeFastSeq_GetSize(some_object)) != DEE_FASTSEQ_NOTFAST) {
		result = DeeSeq_LeIF(iter_self, some_object, other_size);
	} else {
		iter_rhs = DeeObject_IterSelf(some_object);
		if unlikely(!iter_rhs) {
			result = -1;
			goto err1;
		}
		result = DeeIter_Le(iter_self, iter_rhs);
		Dee_Decref(iter_rhs);
	}
err1:
	Dee_Decref(iter_self);
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FUNCTIONS_C */
