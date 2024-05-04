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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_COMPARE_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_COMPARE_H 1

#include <deemon/api.h>
#include <deemon/object.h>

DECL_BEGIN

/* Return values for `seq_compareeq__*__cb' callbacks. */
#define SEQ_COMPAREEQ_FOREACH_RESULT_EQUAL    0
#define SEQ_COMPAREEQ_FOREACH_RESULT_ERROR    (-1)
#define SEQ_COMPAREEQ_FOREACH_RESULT_NOTEQUAL (-2)

/* Return values for `seq_compare__*__cb' callbacks. */
#define SEQ_COMPARE_FOREACH_RESULT_EQUAL    0
#define SEQ_COMPARE_FOREACH_RESULT_ERROR    (-1)
#define SEQ_COMPARE_FOREACH_RESULT_LESS     (-2)
#define SEQ_COMPARE_FOREACH_RESULT_GREATER  (-3)


struct seq_compareforeach__size_and_getitem_index__data {
	DeeObject *scf_sgi_other;  /* [1..1] "other" sequence */
	size_t     scf_sgi_osize;  /* Size of the "other" sequence */
	size_t     scf_sgi_oindex; /* Next index to load the element of */
	/* [1..1] Callback for loading elements of "scf_sgi_other" (convention depends on used callback) */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *scf_sgi_ogetitem_index)(DeeObject *self, size_t index);
};
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_foreach__rhs_size_and_getitem_index_fast__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_foreach__rhs_size_and_trygetitem_index__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_foreach__rhs_size_and_getitem_index__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_foreach__rhs_size_and_getitem_index_fast__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_foreach__rhs_size_and_trygetitem_index__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_foreach__rhs_size_and_getitem_index__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_size_and_getitem_index_fast__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_size_and_trygetitem_index__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_size_and_getitem_index__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_size_and_getitem_index_fast__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_size_and_trygetitem_index__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_size_and_getitem_index__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);

struct seq_compare_foreach__sizeob_and_getitem__data {
	DeeObject      *scf_sg_other;  /* [1..1] "other" sequence */
	DeeObject      *scf_sg_osize;  /* [1..1] Size of the "other" sequence */
	DREF DeeObject *scf_sg_oindex; /* [1..1] Next index to load the element of */
	/* [1..1] Callback for loading elements of "scf_sg_other" (convention depends on used callback) */
	WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *scf_sg_ogetitem)(DeeObject *self, DeeObject *index);
};
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_foreach__rhs_sizeob_and_getitem__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_foreach__rhs_sizeob_and_getitem__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_sizeob_and_getitem__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_sizeob_and_getitem__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);

struct seq_compare_foreach__tsizeob_and_getitem__data {
	DeeTypeObject  *scf_tsg_tpother; /* [1..1] Effective type for "scf_tsg_other" */
	DeeObject      *scf_tsg_other;   /* [1..1] "other" sequence */
	DeeObject      *scf_tsg_osize;   /* [1..1] Size of the "other" sequence */
	DREF DeeObject *scf_tsg_oindex;  /* [1..1] Next index to load the element of */
	/* [1..1] Callback for loading elements of "scf_tsg_other" (convention depends on used callback) */
	WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *scf_tsg_otgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);
};
/*INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_foreach__trhs_sizeob_and_getitem__cb(void *arg, DeeObject *lhs_elem);*/
/*INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_foreach__trhs_sizeob_and_getitem__cb(void *arg, DeeObject *lhs_elem);*/
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__tlhs_sizeob_and_getitem__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__tlhs_sizeob_and_getitem__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);

/* @param: arg: [1..1] DeeObject *other_iter; */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_foreach__rhs_iter__cb(void *arg, DeeObject *lhs_elem);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_foreach__rhs_iter__cb(void *arg, DeeObject *lhs_elem);
/*INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compareeq__lhs_iter__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);*/
/*INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL seq_compare__lhs_iter__rhs_foreach__cb(void *arg, DeeObject *rhs_elem);*/


/* Implementations when "lhs" should be accessed using size+getitem_index_fast. */
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL seq_docompareeq__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index_fast(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_trygetitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_getitem_index_fast__rhs_size_and_getitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL seq_docompare__lhs_size_and_getitem_index_fast__rhs_sizeob_and_getitem(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index_fast)(DeeObject *self, size_t index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));

/* Implementations when "lhs" should be accessed using size+trygetitem_index. */
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL seq_docompareeq__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index_fast(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_trygetitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_trygetitem_index__rhs_size_and_getitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL seq_docompare__lhs_size_and_trygetitem_index__rhs_sizeob_and_getitem(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_trygetitem_index)(DeeObject *self, size_t index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));

/* Implementations when "lhs" should be accessed using size+getitem_index. */
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompareeq__lhs_size_and_getitem_index__rhs_size_and_getitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL seq_docompareeq__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index_fast(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_getitem_index__rhs_size_and_trygetitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 6)) int DCALL seq_docompare__lhs_size_and_getitem_index__rhs_size_and_getitem_index(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 3, 4, 5, 6)) int DCALL seq_docompare__lhs_size_and_getitem_index__rhs_sizeob_and_getitem(DeeObject *lhs, size_t lhs_size, DREF DeeObject *(DCALL *lhs_getitem_index)(DeeObject *self, size_t index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));

/* Implementations when "lhs" should be accessed using sizeob+getitem. */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL seq_docompareeq__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 6)) int DCALL seq_docompareeq__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index_fast(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_trygetitem_index(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int DCALL seq_docompare__lhs_sizeob_and_getitem__rhs_size_and_getitem_index(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 6)) int DCALL seq_docompare__lhs_sizeob_and_getitem__rhs_sizeob_and_getitem(DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_getitem)(DeeObject *self, DeeObject *index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));

/* Implementations when "lhs" should be accessed using typed sizeob+getitem. */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL seq_docompareeq__lhs_tsizeob_and_getitem__rhs_size_and_getitem_index_fast(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL seq_docompareeq__lhs_tsizeob_and_getitem__rhs_size_and_trygetitem_index(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL seq_docompareeq__lhs_tsizeob_and_getitem__rhs_size_and_getitem_index(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 6, 7)) int DCALL seq_docompareeq__lhs_tsizeob_and_getitem__rhs_sizeob_and_getitem(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL seq_docompare__lhs_tsizeob_and_getitem__rhs_size_and_getitem_index_fast(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index_fast)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL seq_docompare__lhs_tsizeob_and_getitem__rhs_size_and_trygetitem_index(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_trygetitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL seq_docompare__lhs_tsizeob_and_getitem__rhs_size_and_getitem_index(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index), DeeObject *rhs, size_t rhs_size, DREF DeeObject *(DCALL *rhs_getitem_index)(DeeObject *self, size_t index));
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 6, 7)) int DCALL seq_docompare__lhs_tsizeob_and_getitem__rhs_sizeob_and_getitem(DeeTypeObject *tp_lhs, DeeObject *lhs, DeeObject *lhs_sizeob, DREF DeeObject *(DCALL *lhs_tgetitem)(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index), DeeObject *rhs, DeeObject *rhs_sizeob, DREF DeeObject *(DCALL *rhs_getitem)(DeeObject *self, DeeObject *index));



struct set_compare__lhs_foreach__rhs__data {
	DeeObject *sc_lfr_rhs; /* [1..1] The right-hand-side set */
	/* [1..1] The tp_contains-operator for "sc_lfr_rhs" */
	DREF DeeObject *(DCALL *sc_lfr_rcontains)(DeeObject *self, DeeObject *item);
};
/* @return:  1: "sc_lfr_rhs" does contain "lhs_elem"
 * @return: -2: "sc_lfr_rhs" does not contain "lhs_elem"
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL set_compare__lhs_foreach__rhs__cb(void *arg, DeeObject *lhs_elem);


struct map_compare__lhs_foreach__rhs__data {
	DeeObject *mc_lfr_rhs; /* [1..1] The right-hand-side set */
	/* [1..1] The tp_trygetitem-operator for "mc_lfr_rhs" */
	DREF DeeObject *(DCALL *mc_lfr_rtrygetitem)(DeeObject *self, DeeObject *key);
};
/* @return:  1: "mc_lfr_rhs" does contain "lhs_key" with the same value
 * @return: -2: "mc_lfr_rhs" does not contain "lhs_key", or has a different value for it
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL map_compare__lhs_foreach__rhs__cb(void *arg, DeeObject *lhs_key, DeeObject *lhs_value);



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_COMPARE_H */
