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
#ifdef __INTELLISENSE__
#include "bsearch.c"
//#define DEFINE_DeeSeq_BFind
#define DEFINE_DeeSeq_BFindRange
//#define DEFINE_DeeSeq_BFindPosition
//#define DEFINE_DeeSeq_BLocate
#endif /* __INTELLISENSE__ */

#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>

#include "../../runtime/runtime_error.h"
#include "../seq_functions.h"

#if (defined(DEFINE_DeeSeq_BFind) +         \
     defined(DEFINE_DeeSeq_BFindRange) +    \
     defined(DEFINE_DeeSeq_BFindPosition) + \
     defined(DEFINE_DeeSeq_BLocate)) != 1
#error "Must #define exactly one of these macros!"
#endif /* DEFINE_DeeSeq_... */

DECL_BEGIN

#ifdef DEFINE_DeeSeq_BFind
#define LOCAL_bfind_with_nsi_getitem_fast bfind_with_nsi_getitem_fast
#define LOCAL_bfind_with_nsi_getitem      bfind_with_nsi_getitem
#define LOCAL_bfind_with_getitem          bfind_with_getitem
#define LOCAL_DeeSeq_BFind                DeeSeq_BFind
#define LOCAL_return_t                    size_t
#define LOCAL__EXTRA_PARAMS               /* nothing */
#define LOCAL__EXTRA_ARGS                 /* nothing */
#define LOCAL_ERROR_RESULT                ((size_t)-2)
#elif defined(DEFINE_DeeSeq_BFindRange)
#define LOCAL_bfind_with_nsi_getitem_fast bfindrange_with_nsi_getitem_fast
#define LOCAL_bfind_with_nsi_getitem      bfindrange_with_nsi_getitem
#define LOCAL_bfind_with_getitem          bfindrange_with_getitem
#define LOCAL_DeeSeq_BFind                DeeSeq_BFindRange
#define LOCAL_return_t                    int
#define LOCAL__EXTRA_PARAMS               , size_t *__restrict p_startindex, size_t *__restrict p_endindex
#define LOCAL__EXTRA_ARGS                 , p_startindex, p_endindex
#define LOCAL_ERROR_RESULT                (-1)
#elif defined(DEFINE_DeeSeq_BFindPosition)
#define LOCAL_bfind_with_nsi_getitem_fast bfindposition_with_nsi_getitem_fast
#define LOCAL_bfind_with_nsi_getitem      bfindposition_with_nsi_getitem
#define LOCAL_bfind_with_getitem          bfindposition_with_getitem
#define LOCAL_DeeSeq_BFind                DeeSeq_BFindPosition
#define LOCAL_return_t                    size_t
#define LOCAL__EXTRA_PARAMS               /* nothing */
#define LOCAL__EXTRA_ARGS                 /* nothing */
#define LOCAL_ERROR_RESULT                ((size_t)-1)
#else /* defined(DEFINE_DeeSeq_BLocate) */
#define LOCAL_bfind_with_nsi_getitem_fast blocate_with_nsi_getitem_fast
#define LOCAL_bfind_with_nsi_getitem      blocate_with_nsi_getitem
#define LOCAL_bfind_with_getitem          blocate_with_getitem
#define LOCAL_DeeSeq_BFind                DeeSeq_BLocate
#define LOCAL_return_t                    DREF DeeObject *
#define LOCAL__EXTRA_PARAMS               , DeeObject *defl
#define LOCAL__EXTRA_ARGS                 , defl
#define LOCAL_ERROR_RESULT                NULL
#endif /* ... */

#ifdef __INTELLISENSE__
PRIVATE WUNUSED LOCAL_return_t DCALL
LOCAL_bfind_with_nsi_getitem_fast(struct type_nsi const *nsi,
                                  DeeObject *self, size_t start, size_t end,
                                  DeeObject *keyed_search_item, DeeObject *key
                                  LOCAL__EXTRA_PARAMS);
PRIVATE WUNUSED LOCAL_return_t DCALL
LOCAL_bfind_with_nsi_getitem(struct type_nsi const *nsi,
                             DeeObject *self, size_t start, size_t end,
                             DeeObject *keyed_search_item, DeeObject *key
                             LOCAL__EXTRA_PARAMS);
PRIVATE WUNUSED LOCAL_return_t DCALL
LOCAL_bfind_with_getitem(struct type_seq *seq, size_t seq_length,
                         DeeObject *self, size_t start, size_t end,
                         DeeObject *keyed_search_item, DeeObject *key
                         LOCAL__EXTRA_PARAMS);
#else /* __INTELLISENSE__ */
DECL_END

#define DEFINE_LOCAL_bfind_with_nsi_getitem_fast
#include "bsearch-impl.c.inl"
#define DEFINE_LOCAL_bfind_with_nsi_getitem
#include "bsearch-impl.c.inl"
#define DEFINE_LOCAL_bfind_with_getitem
#include "bsearch-impl.c.inl"

DECL_BEGIN
#endif /* !__INTELLISENSE__ */



/* Binary search for `keyed_search_item' in `self' */
INTERN WUNUSED LOCAL_return_t DCALL
LOCAL_DeeSeq_BFind(DeeObject *self, size_t start, size_t end,
                   DeeObject *keyed_search_item, DeeObject *key
                   LOCAL__EXTRA_PARAMS) {
	DeeTypeObject *tp_self;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	if unlikely(start >= end)
		goto notfound;
	tp_self = Dee_TYPE(self);
	DeeTypeMRO_Init(&mro, tp_self);
	while (tp_self != &DeeSeq_Type) {
		struct type_seq *seq = tp_self->tp_seq;
		if (seq) {
			size_t seq_length;
			struct type_nsi const *nsi = seq->tp_nsi;
			if (nsi && nsi->nsi_class == TYPE_SEQX_CLASS_SEQ &&
			    is_noninherited_nsi(tp_self, seq, nsi)) {
				if (nsi->nsi_seqlike.nsi_getitem_fast) {
					return LOCAL_bfind_with_nsi_getitem_fast(nsi, self, start, end,
					                                         keyed_search_item, key
					                                         LOCAL__EXTRA_ARGS);
				}
				if (nsi->nsi_seqlike.nsi_getitem) {
					return LOCAL_bfind_with_nsi_getitem(nsi, self, start, end,
					                                    keyed_search_item, key
					                                    LOCAL__EXTRA_ARGS);
				}
				if (has_noninherited_getitem(tp_self, seq)) {
					seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
					if unlikely(seq_length == (size_t)-1)
						goto err;
do_lookup_tpget:
					return LOCAL_bfind_with_getitem(seq, seq_length, self, start, end,
					                                keyed_search_item, key
					                                LOCAL__EXTRA_ARGS);
				}
			}
			if (has_noninherited_getitem(tp_self, seq) &&
			    has_noninherited_size(tp_self, seq)) {
				DREF DeeObject *size_ob;
				int error;
				size_ob = (*seq->tp_size)(self);
				if unlikely(!size_ob)
					goto err;
				error = DeeObject_AsSize(size_ob, &seq_length);
				Dee_Decref(size_ob);
				if unlikely(error)
					goto err;
				goto do_lookup_tpget;
			}
			if (seq->tp_iter_self) {
				/* Use iterators to convert to a tuple, then do the lookup on said tuple. */
				LOCAL_return_t result;
				DREF DeeObject *tuple;
				DREF DeeObject *iterator;
				if unlikely((iterator = (*seq->tp_iter_self)(self)) == NULL)
					goto err;
				tuple = DeeTuple_FromIterator(iterator);
				Dee_Decref(iterator);
				if unlikely(!tuple)
					goto err;
				result = LOCAL_bfind_with_nsi_getitem_fast(DeeTuple_Type.tp_seq->tp_nsi, tuple,
				                                           start, end, keyed_search_item, key
				                                           LOCAL__EXTRA_ARGS);
				Dee_Decref(tuple);
				return result;
			}
		}
		tp_self = DeeTypeMRO_Next(&mro, tp_self);
		if (tp_self == NULL)
			break;
	}
	err_no_generic_sequence(self);
err:
	return LOCAL_ERROR_RESULT;
notfound:
#ifdef DEFINE_DeeSeq_BFind
	return (size_t)-1;
#elif defined(DEFINE_DeeSeq_BFindRange)
	*p_startindex = start;
	*p_endindex   = start;
	return 0;
#elif defined(DEFINE_DeeSeq_BFindPosition)
	if unlikely(start == (size_t)-1)
		err_integer_overflow_i(sizeof(size_t) * CHAR_BIT, true);
	return start;
#else /* defined(DEFINE_DeeSeq_BLocate) */
	if (defl)
		return_reference_(defl);
	err_item_not_found(self, keyed_search_item);
	return NULL;
#endif /* ... */
}

DECL_END

#ifndef __INTELLISENSE__
#undef LOCAL_bfind_with_nsi_getitem_fast
#undef LOCAL_bfind_with_nsi_getitem
#undef LOCAL_bfind_with_getitem
#undef LOCAL_DeeSeq_BFind
#undef LOCAL_return_t
#undef LOCAL__EXTRA_PARAMS
#undef LOCAL__EXTRA_ARGS
#undef LOCAL_ERROR_RESULT

#undef DEFINE_DeeSeq_BFind
#undef DEFINE_DeeSeq_BFindRange
#undef DEFINE_DeeSeq_BFindPosition
#undef DEFINE_DeeSeq_BLocate
#endif /* !__INTELLISENSE__ */
