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
#include "bsearch.c.inl"
#define DEFINE_LOCAL_bfind_with_nsi_getitem_fast
//#define DEFINE_LOCAL_bfind_with_nsi_getitem
//#define DEFINE_LOCAL_bfind_with_getitem
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_LOCAL_bfind_with_nsi_getitem_fast) + \
     defined(DEFINE_LOCAL_bfind_with_nsi_getitem) +      \
     defined(DEFINE_LOCAL_bfind_with_getitem)) != 1
#error "Must #define exactly one of these macros!"
#endif /* DEFINE_LOCAL_bfind_with_... */

DECL_BEGIN

#ifdef DEFINE_LOCAL_bfind_with_nsi_getitem_fast
#define LOCAL_bfind_with_xxx LOCAL_bfind_with_nsi_getitem_fast
#elif defined(DEFINE_LOCAL_bfind_with_nsi_getitem)
#define LOCAL_bfind_with_xxx LOCAL_bfind_with_nsi_getitem
#else /* defined(DEFINE_LOCAL_bfind_with_getitem) */
#define LOCAL_bfind_with_xxx LOCAL_bfind_with_getitem
#endif /* ... */

PRIVATE WUNUSED LOCAL_return_t DCALL
LOCAL_bfind_with_xxx(
#ifdef DEFINE_LOCAL_bfind_with_getitem
                     struct type_seq *seq,
                     size_t seq_length,
#else /* DEFINE_LOCAL_bfind_with_getitem */
                     struct type_nsi const *nsi,
#endif /* !DEFINE_LOCAL_bfind_with_getitem */
                     DeeObject *self, size_t start, size_t end,
                     DeeObject *keyed_search_item, DeeObject *key
                     LOCAL__EXTRA_PARAMS) {
#ifndef DEFINE_LOCAL_bfind_with_getitem
	size_t seq_length;
	seq_length = (*nsi->nsi_seqlike.nsi_getsize)(self);
	if unlikely(seq_length == (size_t)-1)
		goto err;
#endif /* !DEFINE_LOCAL_bfind_with_getitem */

	/* Clamp start/end bounds */
	if (start >= seq_length)
		goto notfound;
	if (end > seq_length)
		end = seq_length;

	/* Do the actual binary search! */
	while (start < end) {
		int diff;
		DREF DeeObject *seq_elem;
		size_t mid = (start + end) / 2;

		/* Load item at `mid' */
#ifdef DEFINE_LOCAL_bfind_with_nsi_getitem_fast
		seq_elem = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, mid);
		if unlikely(!seq_elem)
			goto notfound; /* Unbound items aren't allowed */
#elif defined(DEFINE_LOCAL_bfind_with_nsi_getitem)
		seq_elem = (*nsi->nsi_seqlike.nsi_getitem)(self, mid);
		if unlikely(!seq_elem)
			goto err;
#else /* defined(DEFINE_LOCAL_bfind_with_getitem) */
		{
			DREF DeeObject *index_ob;
			index_ob = DeeInt_NewSize(mid);
			if unlikely(!index_ob)
				goto err;
			seq_elem = (*seq->tp_get)(self, index_ob);
			Dee_Decref(index_ob);
			if unlikely(!seq_elem)
				goto err;
		}
#endif /* !DEFINE_LOCAL_bfind_with_nsi_getitem_fast */

		/* Compare `keyed_search_item' and `seq_elem' */
		diff = DeeObject_CompareKey(keyed_search_item, seq_elem, key);
#ifndef DEFINE_DeeSeq_BLocate
		Dee_Decref(seq_elem);
#endif /* !DEFINE_DeeSeq_BLocate */

		if (diff < 0) {
			if unlikely(diff == -2)
				goto err;
			/* keyed_search_item < self[mid] */
			end = mid;
		} else if (diff > 0) {
			/* keyed_search_item > self[mid] */
			start = mid + 1;
		} else {
			/* Match! */
#ifdef DEFINE_DeeSeq_BFind
			return mid;
#elif defined(DEFINE_DeeSeq_BFindRange)
			size_t result_start = mid;
			size_t result_end   = mid + 1;

			/* Widen the result range's lower bound */
			while (result_start > start) {
				mid = (result_start + start) / 2;
#ifdef DEFINE_LOCAL_bfind_with_nsi_getitem_fast
				seq_elem = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, mid);
				if unlikely(!seq_elem)
					goto notfound; /* Unbound items aren't allowed */
#elif defined(DEFINE_LOCAL_bfind_with_nsi_getitem)
				seq_elem = (*nsi->nsi_seqlike.nsi_getitem)(self, mid);
				if unlikely(!seq_elem)
					goto err;
#else /* defined(DEFINE_LOCAL_bfind_with_getitem) */
				{
					DREF DeeObject *index_ob;
					index_ob = DeeInt_NewSize(mid);
					if unlikely(!index_ob)
						goto err;
					seq_elem = (*seq->tp_get)(self, index_ob);
					Dee_Decref(index_ob);
					if unlikely(!seq_elem)
						goto err;
				}
#endif /* !DEFINE_LOCAL_bfind_with_nsi_getitem_fast */
				diff = DeeObject_CompareKeyEq(keyed_search_item, seq_elem, key);
				Dee_Decref(seq_elem);
				if (diff > 0) {
					/* Still part of returned range! */
					result_start = mid;
				} else {
					if unlikely(diff < 0)
						goto err;
					/* No longer part of returned range! */
					start = mid + 1;
				}
			}

			/* Widen the result range's upper bound */
			while (result_end < end) {
				mid = (result_end + end) / 2;
#ifdef DEFINE_LOCAL_bfind_with_nsi_getitem_fast
				seq_elem = (*nsi->nsi_seqlike.nsi_getitem_fast)(self, mid);
				if unlikely(!seq_elem)
					goto notfound; /* Unbound items aren't allowed */
#elif defined(DEFINE_LOCAL_bfind_with_nsi_getitem)
				seq_elem = (*nsi->nsi_seqlike.nsi_getitem)(self, mid);
				if unlikely(!seq_elem)
					goto err;
#else /* defined(DEFINE_LOCAL_bfind_with_getitem) */
				{
					DREF DeeObject *index_ob;
					index_ob = DeeInt_NewSize(mid);
					if unlikely(!index_ob)
						goto err;
					seq_elem = (*seq->tp_get)(self, index_ob);
					Dee_Decref(index_ob);
					if unlikely(!seq_elem)
						goto err;
				}
#endif /* !DEFINE_LOCAL_bfind_with_nsi_getitem_fast */
				diff = DeeObject_CompareKeyEq(keyed_search_item, seq_elem, key);
				Dee_Decref(seq_elem);
				if (diff > 0) {
					/* Still part of returned range! */
					result_end = mid + 1;
				} else {
					if unlikely(diff < 0)
						goto err;
					/* No longer part of returned range! */
					end = mid;
				}
			}

			/* Write-back the result indices */
			*p_startindex = result_start;
			*p_endindex   = result_end;
			return 0;
#elif defined(DEFINE_DeeSeq_BFindPosition)
			if unlikely(mid == (size_t)-1)
				err_integer_overflow_i(sizeof(size_t) * CHAR_BIT, true);
			return mid;
#else /* defined(DEFINE_DeeSeq_BLocate) */
			return seq_elem;
#endif /* ... */
		}

#ifdef DEFINE_DeeSeq_BLocate
		Dee_Decref(seq_elem);
#endif /* !DEFINE_DeeSeq_BLocate */
	}
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
	/* FALLTHRU to `err' */
#endif /* ... */
err:
	return LOCAL_ERROR_RESULT;
}

#undef LOCAL_bfind_with_xxx

DECL_END

#undef DEFINE_LOCAL_bfind_with_nsi_getitem_fast
#undef DEFINE_LOCAL_bfind_with_nsi_getitem
#undef DEFINE_LOCAL_bfind_with_getitem
