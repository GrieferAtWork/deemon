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
#include "default-compare.c"
//#define DEFINE_compare
#define DEFINE_compareeq
#endif /* __INTELLISENSE__ */

#include <deemon/int.h>
#include <deemon/thread.h>

#if (defined(DEFINE_compare) + defined(DEFINE_compareeq) != 1)
#error "Must #define exactly one of these macros"
#endif

#ifdef DEFINE_compareeq
#define LOCAL_seq_compare__(x)   seq_compareeq__##x
#define LOCAL_seq_docompare__(x) seq_docompareeq__##x
#else /* DEFINE_compareeq */
#define LOCAL_seq_compare__(x)   seq_compare__##x
#define LOCAL_seq_docompare__(x) seq_docompare__##x
#endif /* !DEFINE_compareeq */

DECL_BEGIN

#ifdef DEFINE_compareeq
#define LOCAL_DeeObject_Compare                   DeeObject_TryCompareEq
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_EQUAL    SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR    SEQ_COMPAREEQ_FOREACH_RESULT_ERROR
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS     SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER  SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE_NE(compare_result) \
	(SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL)
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(compare_result) \
	((compare_result) == 0 ? SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL : SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL)
#else /* DEFINE_compareeq */
#define LOCAL_DeeObject_Compare                   DeeObject_Compare
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_EQUAL    SEQ_COMPARE_FOREACH_RESULT_EQUAL
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR    SEQ_COMPARE_FOREACH_RESULT_ERROR
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS     SEQ_COMPARE_FOREACH_RESULT_LESS
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER  SEQ_COMPARE_FOREACH_RESULT_GREATER
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE_NE(compare_result) \
	((compare_result) < 0 ? SEQ_COMPARE_FOREACH_RESULT_LESS : SEQ_COMPARE_FOREACH_RESULT_GREATER)
#define LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(compare_result) \
	((compare_result) == 0 ? SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL : LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE_NE(compare_result))
#endif /* !DEFINE_compareeq */


/************************************************************************/
/* seq_compareforeach__size_and_getitem_index__data                     */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_foreach__rhs_size_and_getitem_index_fast__cb)(void *arg, DeeObject *lhs_elem) {
	int temp;
	DREF DeeObject *rhs_elem;
	struct seq_compareforeach__size_and_getitem_index__data *data;
	data = (struct seq_compareforeach__size_and_getitem_index__data *)arg;
	if (data->scf_sgi_oindex >= data->scf_sgi_osize)
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	rhs_elem = (*data->scf_sgi_ogetitem_index)(data->scf_sgi_other, data->scf_sgi_oindex++);
	if unlikely(!rhs_elem) {
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	}
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(rhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_foreach__rhs_size_and_trygetitem_index__cb)(void *arg, DeeObject *lhs_elem) {
	int temp;
	DREF DeeObject *rhs_elem;
	struct seq_compareforeach__size_and_getitem_index__data *data;
	data = (struct seq_compareforeach__size_and_getitem_index__data *)arg;
	if (data->scf_sgi_oindex >= data->scf_sgi_osize)
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	rhs_elem = (*data->scf_sgi_ogetitem_index)(data->scf_sgi_other, data->scf_sgi_oindex++);
	if unlikely(!ITER_ISOK(rhs_elem)) {
		if unlikely(!rhs_elem)
			goto err;
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	}
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(rhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_foreach__rhs_size_and_getitem_index__cb)(void *arg, DeeObject *lhs_elem) {
	int temp;
	DREF DeeObject *rhs_elem;
	struct seq_compareforeach__size_and_getitem_index__data *data;
	data = (struct seq_compareforeach__size_and_getitem_index__data *)arg;
	if (data->scf_sgi_oindex >= data->scf_sgi_osize)
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	rhs_elem = (*data->scf_sgi_ogetitem_index)(data->scf_sgi_other, data->scf_sgi_oindex++);
	if unlikely(!rhs_elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem) &&
		    !DeeError_Catch(&DeeError_IndexError))
			goto err;
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	}
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(rhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_size_and_getitem_index_fast__rhs_foreach__cb)(void *arg, DeeObject *rhs_elem) {
	int temp;
	DREF DeeObject *lhs_elem;
	struct seq_compareforeach__size_and_getitem_index__data *data;
	data = (struct seq_compareforeach__size_and_getitem_index__data *)arg;
	if (data->scf_sgi_oindex >= data->scf_sgi_osize)
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	lhs_elem = (*data->scf_sgi_ogetitem_index)(data->scf_sgi_other, data->scf_sgi_oindex++);
	if unlikely(!lhs_elem) {
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	}
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(lhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_size_and_trygetitem_index__rhs_foreach__cb)(void *arg, DeeObject *rhs_elem) {
	int temp;
	DREF DeeObject *lhs_elem;
	struct seq_compareforeach__size_and_getitem_index__data *data;
	data = (struct seq_compareforeach__size_and_getitem_index__data *)arg;
	if (data->scf_sgi_oindex >= data->scf_sgi_osize)
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	lhs_elem = (*data->scf_sgi_ogetitem_index)(data->scf_sgi_other, data->scf_sgi_oindex++);
	if unlikely(!ITER_ISOK(lhs_elem)) {
		if unlikely(!lhs_elem)
			goto err;
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	}
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(lhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_size_and_getitem_index__rhs_foreach__cb)(void *arg, DeeObject *rhs_elem) {
	int temp;
	DREF DeeObject *lhs_elem;
	struct seq_compareforeach__size_and_getitem_index__data *data;
	data = (struct seq_compareforeach__size_and_getitem_index__data *)arg;
	if (data->scf_sgi_oindex >= data->scf_sgi_osize)
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	lhs_elem = (*data->scf_sgi_ogetitem_index)(data->scf_sgi_other, data->scf_sgi_oindex++);
	if unlikely(!lhs_elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem) &&
		    !DeeError_Catch(&DeeError_IndexError))
			goto err;
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	}
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(lhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}






/************************************************************************/
/* seq_compare_foreach__sizeob_and_getitem__data                        */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_foreach__rhs_sizeob_and_getitem__cb)(void *arg, DeeObject *lhs_elem) {
	int temp;
	DREF DeeObject *rhs_elem;
	struct seq_compare_foreach__sizeob_and_getitem__data *data;
	data = (struct seq_compare_foreach__sizeob_and_getitem__data *)arg;
	temp = DeeObject_CmpLoAsBool(data->scf_sg_oindex, data->scf_sg_osize);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	}
	rhs_elem = (*data->scf_sg_ogetitem)(data->scf_sg_other, data->scf_sg_oindex);
	if unlikely(!rhs_elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem) &&
		    !DeeError_Catch(&DeeError_IndexError))
			goto err;
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	}
	if unlikely(DeeObject_Inc(&data->scf_sg_oindex))
		goto err_rhs_elem;
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(rhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err_rhs_elem:
	Dee_Decref(rhs_elem);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_sizeob_and_getitem__rhs_foreach__cb)(void *arg, DeeObject *rhs_elem) {
	int temp;
	DREF DeeObject *lhs_elem;
	struct seq_compare_foreach__sizeob_and_getitem__data *data;
	data = (struct seq_compare_foreach__sizeob_and_getitem__data *)arg;
	temp = DeeObject_CmpLoAsBool(data->scf_sg_oindex, data->scf_sg_osize);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	}
	lhs_elem = (*data->scf_sg_ogetitem)(data->scf_sg_other, data->scf_sg_oindex);
	if unlikely(!lhs_elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem) &&
		    !DeeError_Catch(&DeeError_IndexError))
			goto err;
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	}
	if unlikely(DeeObject_Inc(&data->scf_sg_oindex))
		goto err_lhs_elem;
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(lhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err_lhs_elem:
	Dee_Decref(lhs_elem);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}






/************************************************************************/
/* seq_compare_foreach__tsizeob_and_getitem__data                       */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(tlhs_sizeob_and_getitem__rhs_foreach__cb)(void *arg, DeeObject *rhs_elem) {
	int temp;
	DREF DeeObject *lhs_elem;
	struct seq_compare_foreach__tsizeob_and_getitem__data *data;
	data = (struct seq_compare_foreach__tsizeob_and_getitem__data *)arg;
	temp = DeeObject_CmpLoAsBool(data->scf_tsg_oindex, data->scf_tsg_osize);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	}
	lhs_elem = (*data->scf_tsg_otgetitem)(data->scf_tsg_tpother,
	                                      data->scf_tsg_other,
	                                      data->scf_tsg_oindex);
	if unlikely(!lhs_elem) {
		if (!DeeError_Catch(&DeeError_UnboundItem) &&
		    !DeeError_Catch(&DeeError_IndexError))
			goto err;
		/* RHS Unbound item, or premature lhs sequence end */
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	}
	if unlikely(DeeObject_Inc(&data->scf_tsg_oindex))
		goto err_lhs_elem;
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(lhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err_lhs_elem:
	Dee_Decref(lhs_elem);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}






/************************************************************************/
/* @param: arg: [1..1] DeeObject *other_iter;                           */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_foreach__rhs_iter__cb)(void *arg, DeeObject *lhs_elem) {
	int temp;
	DeeObject *rhs_iter = (DeeObject *)arg;
	DREF DeeObject *rhs_elem = DeeObject_IterNext(rhs_iter);
	if (!ITER_ISOK(rhs_elem)) {
		if unlikely(!rhs_elem)
			goto err;
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER; /* lhs > rhs */
	}
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(rhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seq_compare__(lhs_iter__rhs_foreach__cb)(void *arg, DeeObject *rhs_elem) {
	int temp;
	DeeObject *lhs_iter = (DeeObject *)arg;
	DREF DeeObject *lhs_elem = DeeObject_IterNext(lhs_iter);
	if (!ITER_ISOK(lhs_elem)) {
		if unlikely(!lhs_elem)
			goto err;
		return LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS; /* lhs < rhs */
	}
	temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
	Dee_Decref(lhs_elem);
	if unlikely(temp == Dee_COMPARE_ERR)
		goto err;
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE(temp);
err:
	return LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR;
}


#undef LOCAL_SEQ_COMPARE_FOREACH_RESULT_EQUAL
#undef LOCAL_SEQ_COMPARE_FOREACH_RESULT_ERROR
#undef LOCAL_SEQ_COMPARE_FOREACH_RESULT_LESS
#undef LOCAL_SEQ_COMPARE_FOREACH_RESULT_GREATER
#undef LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE
#undef LOCAL_SEQ_COMPARE_FOREACH_RESULT_FROMCOMPARE_NE






/************************************************************************/
/* Implementations when "lhs" should be accessed using size+getitem_index_fast. */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index),
                                                                                        DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		lhs_elem = (*lhs_getitem_index_fast)(lhs, i);
		rhs_elem = (*rhs_getitem_index_fast)(rhs, i);
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
}

INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index),
                                                                                      DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		rhs_elem = (*rhs_trygetitem_index)(rhs, i);
		if unlikely(!rhs_elem)
			goto err;
		lhs_elem = (*lhs_getitem_index_fast)(lhs, i);
		if (!lhs_elem || (rhs_elem == ITER_DONE)) {
			if (!lhs_elem && (rhs_elem == ITER_DONE))
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index),
                                                                                   DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		rhs_elem = (*rhs_getitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		lhs_elem = (*lhs_getitem_index_fast)(lhs, i);
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}


/* @return: (size_t)-1: Error
 * @return: (size_t)-2: Size would overflow */
#ifndef OBJECT_TO_SIZE_WITH_OVERFLOW_HANDLER_DEFINED
#define OBJECT_TO_SIZE_WITH_OVERFLOW_HANDLER_DEFINED
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
object_to_size_with_overflow_handler(DeeObject *sizeob) {
	size_t result;
	if (DeeInt_Check(sizeob)) {
		if (!DeeInt_TryAsSize(sizeob, &result))
			goto overflow;
	} else {
		if (DeeObject_AsSize(sizeob, &result))
			goto err;
	}
	if unlikely(result >= (size_t)-2)
		goto overflow;
	return result;
overflow:
	return (size_t)-2;
err:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		goto overflow;
	return (size_t)-1;
}
#endif /* !OBJECT_TO_SIZE_WITH_OVERFLOW_HANDLER_DEFINED */

INTERN WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index),
                                                                               DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index)) {
	size_t i, common_size = lhs_size;
	size_t rhs_size = object_to_size_with_overflow_handler(rhs_sizeob);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		rhs_elem = (*rhs_getitem)(rhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		lhs_elem = (*lhs_getitem_index_fast)(lhs, i);
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}






/************************************************************************/
/* Implementations when "lhs" should be accessed using size+trygetitem_index. */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index),
                                                                                      DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		lhs_elem = (*lhs_trygetitem_index)(lhs, i);
		if unlikely(!lhs_elem)
			goto err;
		rhs_elem = (*rhs_getitem_index_fast)(rhs, i);
		if ((lhs_elem == ITER_DONE) || !rhs_elem) {
			if ((lhs_elem == ITER_DONE) && !rhs_elem)
				continue; /* Both unbound */
			if (rhs_elem) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index),
                                                                                    DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		lhs_elem = (*lhs_trygetitem_index)(lhs, i);
		if unlikely(!lhs_elem)
			goto err;
		rhs_elem = (*rhs_trygetitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			if (lhs_elem != ITER_DONE)
				Dee_Decref(lhs_elem);
			goto err;
		}
		if ((lhs_elem == ITER_DONE) || (rhs_elem == ITER_DONE)) {
			if ((lhs_elem == ITER_DONE) && (rhs_elem == ITER_DONE))
				continue; /* Both unbound */
			if (rhs_elem != ITER_DONE) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_trygetitem_index__rhs_size_and_getitem_index)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index),
                                                                                 DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		rhs_elem = (*rhs_getitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		lhs_elem = (*lhs_trygetitem_index)(lhs, i);
		if unlikely(!lhs_elem) {
			Dee_Decref(rhs_elem);
			goto err;
		}
		if ((lhs_elem == ITER_DONE) || !rhs_elem) {
			if ((lhs_elem == ITER_DONE) && !rhs_elem)
				continue; /* Both unbound */
			if (rhs_elem) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index),
                                                                             DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index)) {
	size_t i, common_size = lhs_size;
	size_t rhs_size = object_to_size_with_overflow_handler(rhs_sizeob);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		rhs_elem = (*rhs_getitem)(rhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		lhs_elem = (*lhs_trygetitem_index)(lhs, i);
		if unlikely(!lhs_elem) {
			Dee_Decref(rhs_elem);
			goto err;
		}
		if ((lhs_elem == ITER_DONE) || !rhs_elem) {
			if ((lhs_elem == ITER_DONE) && !rhs_elem)
				continue; /* Both unbound */
			if (rhs_elem) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}






/************************************************************************/
/* Implementations when "lhs" should be accessed using size+getitem_index. */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index),
                                                                                   DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		lhs_elem = (*lhs_getitem_index)(lhs, i);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return -1; /* LHS got smaller... -> lhs < rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_getitem_index_fast)(rhs, i);
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (rhs_elem) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_getitem_index__rhs_size_and_trygetitem_index)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index),
                                                                                 DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		lhs_elem = (*lhs_getitem_index)(lhs, i);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return -1; /* LHS got smaller... -> lhs < rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_trygetitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_Decref(lhs_elem);
			goto err;
		}
		if (!lhs_elem || (rhs_elem == ITER_DONE)) {
			if (!lhs_elem && (rhs_elem == ITER_DONE))
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_getitem_index__rhs_size_and_getitem_index)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index),
                                                                              DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = lhs_size;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		lhs_elem = (*lhs_getitem_index)(lhs, i);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return -1; /* LHS got smaller... -> lhs < rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_getitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_XDecref(lhs_elem);
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				Dee_XDecref(lhs_elem);
				goto err;
			}
		}
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (rhs_elem) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL
LOCAL_seq_docompare__(lhs_size_and_getitem_index__rhs_sizeob_and_getitem)(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index),
                                                                          DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index)) {
	size_t i, common_size = lhs_size;
	size_t rhs_size = object_to_size_with_overflow_handler(rhs_sizeob);
	if unlikely(rhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != rhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > rhs_size)
		common_size = rhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		rhs_elem = (*rhs_getitem)(rhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		lhs_elem = (*lhs_getitem_index)(lhs, i);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_XDecref(rhs_elem);
				return -1; /* LHS got smaller... -> lhs < rhs */
			} else {
				Dee_XDecref(rhs_elem);
				goto err;
			}
		}
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (rhs_elem) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}






/************************************************************************/
/* Implementations when "lhs" should be accessed using sizeob+getitem.  */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast)(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index),
                                                                               DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index)) {
	size_t i, common_size = rhs_size;
	size_t lhs_size = object_to_size_with_overflow_handler(lhs_sizeob);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != lhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > lhs_size)
		common_size = lhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		lhs_elem = (*lhs_getitem)(lhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_getitem_index_fast)(rhs, i);
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (rhs_elem) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index)(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index),
                                                                             DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = rhs_size;
	size_t lhs_size = object_to_size_with_overflow_handler(lhs_sizeob);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != lhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > lhs_size)
		common_size = lhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		lhs_elem = (*lhs_getitem)(lhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_trygetitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_XDecref(lhs_elem);
			goto err;
		}
		if (!lhs_elem || (rhs_elem == ITER_DONE)) {
			if (!lhs_elem && (rhs_elem == ITER_DONE))
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL
LOCAL_seq_docompare__(lhs_sizeob_and_getitem__rhs_size_and_getitem_index)(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index),
                                                                          DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = rhs_size;
	size_t lhs_size = object_to_size_with_overflow_handler(lhs_sizeob);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != lhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > lhs_size)
		common_size = lhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		lhs_elem = (*lhs_getitem)(lhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_getitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_XDecref(lhs_elem);
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				Dee_XDecref(lhs_elem);
				goto err;
			}
		}
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5, 6)) int DCALL
LOCAL_seq_docompare__(lhs_sizeob_and_getitem__rhs_sizeob_and_getitem)(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index),
                                                                      DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index)) {
	int temp;
#ifndef DEFINE_compareeq
	bool common_sizeob__is__lhs_sizeob = true;
#endif /* !DEFINE_compareeq */
	DREF DeeObject *index_ob;
	DeeObject *common_sizeob = lhs_sizeob;
#ifdef DEFINE_compareeq
	temp = DeeObject_TryCompareEq(common_sizeob, rhs_sizeob);
	if (temp != 0)
		return temp; /* Not-equal or error */
#else /* DEFINE_compareeq */
	temp = DeeObject_CmpLoAsBool(common_sizeob, rhs_sizeob);
	if unlikely(temp < 0)
		goto err;
	if (!temp) {
		common_sizeob__is__lhs_sizeob = false;
		common_sizeob = rhs_sizeob;
	}
#endif /* !DEFINE_compareeq */

	index_ob = DeeObject_NewDefault(Dee_TYPE(common_sizeob));
	if unlikely(!index_ob)
		goto err;
	for (;;) {
		DREF DeeObject *lhs_elem, *rhs_elem;
		temp = DeeObject_CmpLoAsBool(index_ob, common_sizeob);
		if unlikely(temp < 0)
			goto err_index_ob;
		if (!temp)
			break;
		lhs_elem = (*lhs_getitem)(lhs, index_ob);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_Decref(index_ob);
				return -1; /* LHS got smaller... -> lhs < rhs */
			} else {
				goto err_index_ob;
			}
		}
		rhs_elem = (*rhs_getitem)(rhs, index_ob);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_Decref(index_ob);
				Dee_XDecref(lhs_elem);
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				Dee_XDecref(lhs_elem);
				goto err_index_ob;
			}
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0) {
			Dee_Decref(index_ob);
			return temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err_index_ob;
		if (DeeObject_Inc(&index_ob))
			goto err_index_ob;
	}
	Dee_Decref(index_ob);
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	if (!common_sizeob__is__lhs_sizeob)
		return 1; /* lhs_size > rhs_size */
	temp = DeeObject_TryCompareEq(lhs_sizeob, rhs_sizeob);
	if (temp > 0)
		temp = -1;
	ASSERT(temp == Dee_COMPARE_ERR || temp == -1 || temp == 0);
	return temp;
#endif /* !DEFINE_compareeq */
err_index_ob:
	Dee_Decref(index_ob);
err:
	return Dee_COMPARE_ERR;
}






/************************************************************************/
/* Implementations when "lhs" should be accessed using typed sizeob+getitem. */
/************************************************************************/
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL
LOCAL_seq_docompare__(lhs_tsizeob_and_getitem__rhs_size_and_getitem_index_fast)(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index),
                                                                                DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index)) {
	size_t i, common_size = rhs_size;
	size_t lhs_size = object_to_size_with_overflow_handler(lhs_sizeob);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != lhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > lhs_size)
		common_size = lhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		lhs_elem = (*lhs_tgetitem)(tp_lhs, lhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_getitem_index_fast)(rhs, i);
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (rhs_elem) {
				Dee_Decref(rhs_elem);
			} else {
				Dee_Decref(lhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL
LOCAL_seq_docompare__(lhs_tsizeob_and_getitem__rhs_size_and_trygetitem_index)(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index),
                                                                              DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = rhs_size;
	size_t lhs_size = object_to_size_with_overflow_handler(lhs_sizeob);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != lhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > lhs_size)
		common_size = lhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		lhs_elem = (*lhs_tgetitem)(tp_lhs, lhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_trygetitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			Dee_XDecref(lhs_elem);
			goto err;
		}
		if (!lhs_elem || (rhs_elem == ITER_DONE)) {
			if (!lhs_elem && (rhs_elem == ITER_DONE))
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL
LOCAL_seq_docompare__(lhs_tsizeob_and_getitem__rhs_size_and_getitem_index)(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index),
                                                                           DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index)) {
	size_t i, common_size = rhs_size;
	size_t lhs_size = object_to_size_with_overflow_handler(lhs_sizeob);
	if unlikely(lhs_size == (size_t)-1)
		goto err;
#ifdef DEFINE_compareeq
	if (common_size != lhs_size)
		return 1; /* not-equal */
#else /* DEFINE_compareeq */
	if (common_size > lhs_size)
		common_size = lhs_size;
#endif /* DEFINE_compareeq */
	for (i = 0; i < common_size; ++i) {
		int temp;
		DREF DeeObject *lhs_elem, *rhs_elem;
		DREF DeeObject *index_ob;
		index_ob = DeeInt_NewSize(i);
		if unlikely(!index_ob)
			goto err;
		lhs_elem = (*lhs_tgetitem)(tp_lhs, lhs, index_ob);
		Dee_Decref(index_ob);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				goto err;
			}
		}
		rhs_elem = (*rhs_getitem_index)(rhs, i);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_XDecref(lhs_elem);
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				Dee_XDecref(lhs_elem);
				goto err;
			}
		}
		if (!lhs_elem || !rhs_elem) {
			if (!lhs_elem && !rhs_elem)
				continue; /* Both unbound */
			if (lhs_elem) {
				Dee_Decref(lhs_elem);
			} else {
				Dee_Decref(rhs_elem);
			}
#ifdef DEFINE_compareeq
			return 1; /* Not-equal */
#else /* DEFINE_compareeq */
			return lhs_elem ? 1 : -1;
#endif /* !DEFINE_compareeq */
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0)
			return temp;
		if (DeeThread_CheckInterrupt())
			goto err;
	}
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	return Dee_Compare(lhs_size, lhs_size);
#endif /* !DEFINE_compareeq */
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5, 6, 7)) int DCALL
LOCAL_seq_docompare__(lhs_tsizeob_and_getitem__rhs_sizeob_and_getitem)(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index),
                                                                       DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index)) {
	int temp;
#ifndef DEFINE_compareeq
	bool common_sizeob__is__lhs_sizeob = true;
#endif /* !DEFINE_compareeq */
	DREF DeeObject *index_ob;
	DeeObject *common_sizeob = lhs_sizeob;
#ifdef DEFINE_compareeq
	temp = DeeObject_TryCompareEq(common_sizeob, rhs_sizeob);
	if (temp != 0)
		return temp; /* Not-equal or error */
#else /* DEFINE_compareeq */
	temp = DeeObject_CmpLoAsBool(common_sizeob, rhs_sizeob);
	if unlikely(temp < 0)
		goto err;
	if (!temp) {
		common_sizeob__is__lhs_sizeob = false;
		common_sizeob = rhs_sizeob;
	}
#endif /* !DEFINE_compareeq */

	index_ob = DeeObject_NewDefault(Dee_TYPE(common_sizeob));
	if unlikely(!index_ob)
		goto err;
	for (;;) {
		DREF DeeObject *lhs_elem, *rhs_elem;
		temp = DeeObject_CmpLoAsBool(index_ob, common_sizeob);
		if unlikely(temp < 0)
			goto err_index_ob;
		if (!temp)
			break;
		lhs_elem = (*lhs_tgetitem)(tp_lhs, lhs, index_ob);
		if unlikely(!lhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_Decref(index_ob);
				return -1; /* LHS got smaller... -> lhs < rhs */
			} else {
				goto err_index_ob;
			}
		}
		rhs_elem = (*rhs_getitem)(rhs, index_ob);
		if unlikely(!rhs_elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				/* Unbound... */
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				Dee_Decref(index_ob);
				Dee_XDecref(lhs_elem);
				return 1; /* RHS got smaller... -> lhs > rhs */
			} else {
				Dee_XDecref(lhs_elem);
				goto err_index_ob;
			}
		}
		temp = LOCAL_DeeObject_Compare(lhs_elem, rhs_elem);
		Dee_Decref(rhs_elem);
		Dee_Decref(lhs_elem);
		if (temp != 0) {
			Dee_Decref(index_ob);
			return temp;
		}
		if (DeeThread_CheckInterrupt())
			goto err_index_ob;
		if (DeeObject_Inc(&index_ob))
			goto err_index_ob;
	}
	Dee_Decref(index_ob);
#ifdef DEFINE_compareeq
	return 0; /* Equal */
#else /* DEFINE_compareeq */
	if (!common_sizeob__is__lhs_sizeob)
		return 1; /* lhs_size > rhs_size */
	temp = DeeObject_TryCompareEq(lhs_sizeob, rhs_sizeob);
	if (temp > 0)
		temp = -1;
	ASSERT(temp == Dee_COMPARE_ERR || temp == -1 || temp == 0);
	return temp;
#endif /* !DEFINE_compareeq */
err_index_ob:
	Dee_Decref(index_ob);
err:
	return Dee_COMPARE_ERR;
}


#undef LOCAL_DeeObject_Compare

DECL_END

#undef LOCAL_seq_compare__
#undef LOCAL_seq_docompare__
#undef DEFINE_compareeq
#undef DEFINE_compare
