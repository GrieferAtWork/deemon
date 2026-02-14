/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "each.c"
//#define DEFINE_SeqEachGetAttr
//#define DEFINE_SeqEachCallAttr
#define DEFINE_SeqEachCallAttrKw
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_*, Dee_Freea, Dee_Mallocac, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, _Dee_MallococBufsize */
#include <deemon/arg.h>                /* DeeArg_Unpack* */
#include <deemon/computed-operators.h>
#include <deemon/format.h>             /* DeeFormat_PrintOperatorRepr, DeeFormat_Printf */
#include <deemon/map.h>                /* DeeMapping_NewEmpty, Dee_EmptyMapping */
#include <deemon/method-hints.h>       /* DeeObject_InvokeMethodHint, Dee_seq_enumerate_index_t, Dee_seq_enumerate_t, TYPE_METHOD_HINT, TYPE_METHOD_HINT_END, type_method_hint */
#include <deemon/mro.h>                /* Dee_attrdesc, Dee_attrhint, Dee_attriter, Dee_attrspec */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_*, Dee_Decref, Dee_Decrefv, Dee_Incref, Dee_Movrefv, Dee_TYPE, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, Dee_visit_t, ITER_ISOK, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_NewEmpty */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString*, WSTR_LENGTH */
#include <deemon/system-features.h>    /* mempcpyc, strlen */
#include <deemon/tuple.h>              /* DeeTuple*, Dee_EmptyTuple */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_Visit, Dee_Visitv, OPERATOR_CALL, STRUCT_OBJECT_AB, TF_NONE, TP_F*, TYPE_MEMBER*, type_* */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "each.h"

#include <stdbool.h> /*  */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

/* clang-format off */
#include <deemon/operator-hints.h> /* default__* (must be included manually) */
/* clang-format on */

#ifdef DEFINE_SeqEachGetAttr
#define LOCAL_ssX(x)               ssa_##x
#define LOCAL_seX(x)               sea_##x
#define LOCAL_seXi(x)              seai_##x
#define LOCAL_SeqEach              SeqEachGetAttr
#define LOCAL_SeqEach_Type_NAME    "SeqEachGetAttr"
#define LOCAL_SeqEach_Type_FLAGS   (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY)
#define LOCAL_SeqEach_Type         SeqEachGetAttr_Type
#define LOCAL_SeqSome_Type         SeqSomeGetAttr_Type
#define LOCAL_SeqEachIterator_Type SeqEachGetAttrIterator_Type
#define LOCAL_SeqSome_Type_NAME    "SeqSomeGetAttr"
#elif defined(DEFINE_SeqEachCallAttr)
#define LOCAL_ssX(x)               ssc_##x
#define LOCAL_seX(x)               sec_##x
#define LOCAL_seXi(x)              seci_##x
#define LOCAL_SeqEach_Type_NAME    "SeqEachCallAttr"
#define LOCAL_SeqEach_Type_FLAGS   (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY | TP_FVARIABLE)
#define LOCAL_SeqEach              SeqEachCallAttr
#define LOCAL_SeqEach_Type         SeqEachCallAttr_Type
#define LOCAL_SeqSome_Type         SeqSomeCallAttr_Type
#define LOCAL_SeqSome_Type_NAME    "SeqSomeCallAttr"
#define LOCAL_SeqEachIterator_Type SeqEachCallAttrIterator_Type
#elif defined(DEFINE_SeqEachCallAttrKw)
#define LOCAL_ssX(x)               ssk_##x
#define LOCAL_seX(x)               sek_##x
#define LOCAL_seXi(x)              seki_##x
#define LOCAL_SeqEach_Type_NAME    "SeqEachCallAttrKw"
#define LOCAL_SeqEach_Type_FLAGS   (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY | TP_FVARIABLE)
#define LOCAL_SeqEach              SeqEachCallAttrKw
#define LOCAL_SeqEach_Type         SeqEachCallAttrKw_Type
#define LOCAL_SeqSome_Type         SeqSomeCallAttrKw_Type
#define LOCAL_SeqSome_Type_NAME    "SeqSomeCallAttrKw"
#define LOCAL_SeqEachIterator_Type SeqEachCallAttrKwIterator_Type
#else /* ... */
#error "No mode defined"
#endif /* !... */


DECL_BEGIN

#ifdef DEFINE_SeqEachGetAttr
struct LOCAL_seX(inplace_foreach_data) {
	DeeStringObject *sifd_attr;
	int (DCALL      *sifd_op)(DREF DeeObject **__restrict p_self, DeeObject *other);
	DeeObject       *sifd_other;
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
LOCAL_seX(inplace_foreach_cb)(void *arg, DeeObject *elem) {
	int result;
	DREF DeeObject *elem_value;
	struct LOCAL_seX(inplace_foreach_data) *data;
	data = (struct LOCAL_seX(inplace_foreach_data) *)arg;
	elem_value = DeeObject_GetAttr(elem, (DeeObject *)data->sifd_attr);
	if unlikely(!elem_value)
		goto err;
	if unlikely((*data->sifd_op)(&elem_value, data->sifd_other))
		goto err_elem_value;
	result = DeeObject_SetAttr(elem, (DeeObject *)data->sifd_attr, elem_value);
	Dee_Decref(elem_value);
	return result;
err_elem_value:
	Dee_Decref(elem_value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(inplace)(LOCAL_SeqEach *self,
                   int (DCALL *op)(DREF DeeObject **__restrict p_self, DeeObject *other),
                   DeeObject *other) {
	struct LOCAL_seX(inplace_foreach_data) data;
	data.sifd_attr  = self->sg_attr;
	data.sifd_op    = op;
	data.sifd_other = other;
	return (int)DeeObject_InvokeMethodHint(seq_operator_foreach, self->se_seq,
	                                       &LOCAL_seX(inplace_foreach_cb), &data);
}

#define DEFINE_SEA_BINARY_INPLACE(name, func)                   \
	PRIVATE NONNULL((1, 2)) int DCALL                           \
	name(LOCAL_SeqEach **__restrict p_self, DeeObject *other) { \
		return LOCAL_seX(inplace)(*p_self, &func, other);       \
	}
PRIVATE NONNULL((1)) int DCALL LOCAL_seX(inc)(LOCAL_SeqEach **__restrict p_self) {
	return LOCAL_seX(inplace)(*p_self, &inplace_inc_wrapper, NULL);
}
PRIVATE NONNULL((1)) int DCALL LOCAL_seX(dec)(LOCAL_SeqEach **__restrict p_self) {
	return LOCAL_seX(inplace)(*p_self, &inplace_dec_wrapper, NULL);
}

DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_add), DeeObject_InplaceAdd)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_sub), DeeObject_InplaceSub)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_mul), DeeObject_InplaceMul)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_div), DeeObject_InplaceDiv)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_mod), DeeObject_InplaceMod)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_shl), DeeObject_InplaceShl)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_shr), DeeObject_InplaceShr)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_and), DeeObject_InplaceAnd)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_or), DeeObject_InplaceOr)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_xor), DeeObject_InplaceXor)
DEFINE_SEA_BINARY_INPLACE(LOCAL_seX(inplace_pow), DeeObject_InplacePow)
#undef DEFINE_SEA_BINARY_INPLACE
#endif /* DEFINE_SeqEachGetAttr */



#ifdef DEFINE_SeqEachGetAttr
PRIVATE struct type_math LOCAL_seX(math) = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_inv,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_pos,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_neg,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_pow,
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&LOCAL_seX(inc),
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&LOCAL_seX(dec),
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_add),
	/* .tp_inplace_sub = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_sub),
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_mul),
	/* .tp_inplace_div = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_div),
	/* .tp_inplace_mod = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_mod),
	/* .tp_inplace_shl = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_shl),
	/* .tp_inplace_shr = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_shr),
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_and),
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_or),
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_xor),
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&LOCAL_seX(inplace_pow)
};

#else /* DEFINE_SeqEachGetAttr */

#ifndef SEW_MATH_DEFINED
#define SEW_MATH_DEFINED 1
PRIVATE struct type_math sew_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_inv,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_pos,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_neg,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_pow
};
#endif /* !SEW_MATH_DEFINED */

#if defined(DEFINE_SeqEachCallAttr)
#define sec_math sew_math
#elif defined(DEFINE_SeqEachCallAttrKw)
#define sek_math sew_math
#else /* ... */
#error "Unsupported mode"
#endif /* !... */

#endif /* !DEFINE_SeqEachGetAttr */


PRIVATE WUNUSED NONNULL((1)) DREF SeqEachIterator *DCALL
LOCAL_seX(mh_seq_operator_iter)(LOCAL_SeqEach *__restrict self) {
	DREF SeqEachIterator *result;
	result = DeeObject_MALLOC(SeqEachIterator);
	if unlikely(!result)
		goto done;
	result->ei_each = (DREF SeqEachBase *)self;
	result->ei_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->se_seq);
	if unlikely(!result->ei_iter)
		goto err_r;
	Dee_Incref(self);
	DeeObject_Init(result, &LOCAL_SeqEachIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_seX(transform)(LOCAL_SeqEach *self, DeeObject *elem) {
#ifdef DEFINE_SeqEachGetAttr
	return DeeObject_GetAttr(elem, Dee_AsObject(self->sg_attr));
#elif defined(DEFINE_SeqEachCallAttr)
	return DeeObject_CallAttr(elem, Dee_AsObject(self->sg_attr), self->sg_argc, self->sg_argv);
#elif defined(DEFINE_SeqEachCallAttrKw)
	return DeeObject_CallAttrKw(elem, Dee_AsObject(self->sg_attr), self->sg_argc, self->sg_argv, self->sg_kw);
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_seX(transform_inherited)(LOCAL_SeqEach *self, /*inherit(always)*/ DREF DeeObject *elem) {
	DREF DeeObject *result;
	result = LOCAL_seX(transform)(self, elem);
	Dee_Decref(elem);
	return result;
}

struct LOCAL_seX(foreach_data) {
	LOCAL_SeqEach *seXfd_me;   /* [1..1] The related seq-each operator */
	Dee_foreach_t  seXfd_proc; /* [1..1] User-defined callback */
	void          *seXfd_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seX(foreach_cb)(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	struct LOCAL_seX(foreach_data) *data;
	data = (struct LOCAL_seX(foreach_data) *)arg;
	elem = LOCAL_seX(transform)(data->seXfd_me, elem);
	if unlikely(!elem)
		goto err;
	result = (*data->seXfd_proc)(data->seXfd_arg, elem);
	Dee_Decref(elem);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seX(foreach)(LOCAL_SeqEach *self, Dee_foreach_t proc, void *arg) {
	struct LOCAL_seX(foreach_data) data;
	data.seXfd_me   = self;
	data.seXfd_proc = proc;
	data.seXfd_arg  = arg;
	return DeeObject_Foreach(self->se_seq, &LOCAL_seX(foreach_cb), &data);
}

struct LOCAL_seX(mh_seq_enumerate_data) {
	LOCAL_SeqEach  *seXed_me;   /* [1..1] The related seq-each operator */
	Dee_seq_enumerate_t seXed_proc; /* [1..1] User-defined callback */
	void           *seXed_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seX(mh_seq_enumerate_cb)(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct LOCAL_seX(mh_seq_enumerate_data) *data;
	data = (struct LOCAL_seX(mh_seq_enumerate_data) *)arg;
	if unlikely(!value)
		return (*data->seXed_proc)(data->seXed_arg, index, value);
	value = LOCAL_seX(transform)(data->seXed_me, value);
	if unlikely(!value)
		goto err;
	result = (*data->seXed_proc)(data->seXed_arg, index, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seX(mh_seq_enumerate)(LOCAL_SeqEach *__restrict self, Dee_seq_enumerate_t proc, void *arg) {
	struct LOCAL_seX(mh_seq_enumerate_data) data;
	data.seXed_me   = self;
	data.seXed_proc = proc;
	data.seXed_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate, self->se_seq, &LOCAL_seX(mh_seq_enumerate_cb), &data);
}

struct LOCAL_seX(mh_seq_enumerate_index_data) {
	LOCAL_SeqEach            *seXeid_me;   /* [1..1] The related seq-each operator */
	Dee_seq_enumerate_index_t seXeid_proc; /* [1..1] User-defined callback */
	void                     *seXeid_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
LOCAL_seX(mh_seq_enumerate_index_cb)(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct LOCAL_seX(mh_seq_enumerate_index_data) *data;
	data = (struct LOCAL_seX(mh_seq_enumerate_index_data) *)arg;
	if unlikely(!value)
		return (*data->seXeid_proc)(data->seXeid_arg, index, value);
	value = LOCAL_seX(transform)(data->seXeid_me, value);
	if unlikely(!value)
		goto err;
	result = (*data->seXeid_proc)(data->seXeid_arg, index, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seX(mh_seq_enumerate_index)(LOCAL_SeqEach *__restrict self, Dee_seq_enumerate_index_t proc,
                           void *arg, size_t start, size_t end) {
	struct LOCAL_seX(mh_seq_enumerate_index_data) data;
	data.seXeid_me   = self;
	data.seXeid_proc = proc;
	data.seXeid_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate_index, self->se_seq,
	                                  &LOCAL_seX(mh_seq_enumerate_index_cb),
	                                  &data, start, end);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seX(bool)(LOCAL_SeqEach *__restrict self) {
	Dee_ssize_t result = LOCAL_seX(foreach)(self, &se_foreach_bool_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		return 0;
	if (result == 0)
		return 1;
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(assign)(LOCAL_SeqEach *self, DeeObject *value) {
	return (int)LOCAL_seX(foreach)(self, &se_foreach_assign_cb, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(moveassign)(LOCAL_SeqEach *self, DeeObject *value) {
	return (int)LOCAL_seX(foreach)(self, &se_foreach_moveassign_cb, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(delitem)(LOCAL_SeqEach *self, DeeObject *index) {
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delitem_cb, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seX(delitem_index)(LOCAL_SeqEach *self, size_t index) {
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delitem_index_cb, (void *)(uintptr_t)index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(delitem_string_hash)(LOCAL_SeqEach *self, char const *key, Dee_hash_t hash) {
	struct se_foreach_delitem_string_hash_data data;
	data.sfedish_key  = key;
	data.sfedish_hash = hash;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delitem_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(delitem_string_len_hash)(LOCAL_SeqEach *self, char const *key,
                                   size_t keylen, Dee_hash_t hash) {
	struct se_foreach_delitem_string_len_hash_data data;
	data.sfedislh_key    = key;
	data.sfedislh_keylen = keylen;
	data.sfedislh_hash   = hash;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delitem_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_seX(setitem)(LOCAL_SeqEach *self, DeeObject *index, DeeObject *value) {
	struct se_foreach_setitem_data data;
	data.sfesi_index = index;
	data.sfesi_value = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setitem_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_seX(setitem_index)(LOCAL_SeqEach *self, size_t index, DeeObject *value) {
	struct se_foreach_setitem_index_data data;
	data.sfesii_index = index;
	data.sfesii_value = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setitem_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
LOCAL_seX(setitem_string_hash)(LOCAL_SeqEach *self, char const *key,
                               Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setitem_string_hash_data data;
	data.sfesish_key   = key;
	data.sfesish_hash  = hash;
	data.sfesish_value = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setitem_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
LOCAL_seX(setitem_string_len_hash)(LOCAL_SeqEach *self, char const *key,
                                   size_t keylen, Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setitem_string_len_hash_data data;
	data.sfesislh_key    = key;
	data.sfesislh_keylen = keylen;
	data.sfesislh_hash   = hash;
	data.sfesislh_value  = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setitem_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_seX(delrange)(LOCAL_SeqEach *self, DeeObject *start, DeeObject *end) {
	struct se_foreach_delrange_data data;
	data.sfedr_start = start;
	data.sfedr_end   = end;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delrange_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seX(delrange_index)(LOCAL_SeqEach *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct se_foreach_delrange_index_data data;
	data.sfedri_start = start;
	data.sfedri_end   = end;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delrange_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seX(delrange_index_n)(LOCAL_SeqEach *self, Dee_ssize_t start) {
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delrange_index_n_cb,
	                               (void *)(uintptr_t)(size_t)start);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
LOCAL_seX(setrange)(LOCAL_SeqEach *self, DeeObject *start,
                    DeeObject *end, DeeObject *value) {
	struct se_foreach_setrange_data data;
	data.sfesr_start = start;
	data.sfesr_end   = end;
	data.sfesr_value = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setrange_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
LOCAL_seX(setrange_index)(LOCAL_SeqEach *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value) {
	struct se_foreach_setrange_index_data data;
	data.sfesri_start = start;
	data.sfesri_end   = end;
	data.sfesri_value = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setrange_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
LOCAL_seX(setrange_index_n)(LOCAL_SeqEach *self, Dee_ssize_t start, DeeObject *value) {
	struct se_foreach_setrange_index_n_data data;
	data.sfesrin_start = start;
	data.sfesrin_value = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setrange_index_n_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(delattr)(LOCAL_SeqEach *self, DeeObject *attr) {
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delattr_cb, attr);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(delattr_string_hash)(LOCAL_SeqEach *self, char const *attr, Dee_hash_t hash) {
	struct se_foreach_delattr_string_hash_data data;
	data.sfedsh_attr = attr;
	data.sfedsh_hash = hash;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delattr_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(delattr_string_len_hash)(LOCAL_SeqEach *self, char const *attr,
                                   size_t attrlen, Dee_hash_t hash) {
	struct se_foreach_delattr_string_len_hash_data data;
	data.sfedslh_attr    = attr;
	data.sfedslh_attrlen = attrlen;
	data.sfedslh_hash    = hash;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_delattr_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
LOCAL_seX(setattr)(LOCAL_SeqEach *self, DeeObject *attr, DeeObject *value) {
	struct se_foreach_setattr_data data;
	data.sfes_attr  = attr;
	data.sfes_value = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setattr_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
LOCAL_seX(setattr_string_hash)(LOCAL_SeqEach *self, char const *attr,
                               Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setattr_string_hash_data data;
	data.sfessh_attr  = attr;
	data.sfessh_hash  = hash;
	data.sfessh_value = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setattr_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
LOCAL_seX(setattr_string_len_hash)(LOCAL_SeqEach *self, char const *attr,
                                   size_t attrlen, Dee_hash_t hash,
                                   DeeObject *value) {
	struct se_foreach_setattr_string_len_hash_data data;
	data.sfesslh_attr    = attr;
	data.sfesslh_attrlen = attrlen;
	data.sfesslh_hash    = hash;
	data.sfesslh_value   = value;
	return (int)LOCAL_seX(foreach)(self, &se_foreach_setattr_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(boundattr)(LOCAL_SeqEach *self, DeeObject *attr) {
	return seqeach_map_fe2bound(LOCAL_seX(foreach)(self, &se_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(hasattr)(LOCAL_SeqEach *self, DeeObject *attr) {
	return seqeach_map_fe2has(LOCAL_seX(foreach)(self, &se_hasattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(boundattr_string_hash)(LOCAL_SeqEach *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_fe2bound(LOCAL_seX(foreach)(self, &se_boundattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(hasattr_string_hash)(LOCAL_SeqEach *self, char const *attr, Dee_hash_t hash) {
	struct se_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqeach_map_fe2has(LOCAL_seX(foreach)(self, &se_hasattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(boundattr_string_len_hash)(LOCAL_SeqEach *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_fe2bound(LOCAL_seX(foreach)(self, &se_boundattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(hasattr_string_len_hash)(LOCAL_SeqEach *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct se_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqeach_map_fe2has(LOCAL_seX(foreach)(self, &se_hasattr_string_len_hash_foreach_cb, &data));
}



PRIVATE struct type_attr LOCAL_seX(attr) = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&sew_getattr,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&LOCAL_seX(delattr),
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&LOCAL_seX(setattr),
	/* .tp_iterattr                      = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&sew_iterattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&sew_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_seX(hasattr),
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_seX(boundattr),
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&sew_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&sew_callattr_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr                      = */ NULL,
	/* .tp_callattr_kw                   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattrf                    = */ NULL,
	/* .tp_getattr_string_hash           = */ NULL,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&LOCAL_seX(delattr_string_hash),
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&LOCAL_seX(setattr_string_hash),
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&LOCAL_seX(hasattr_string_hash),
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&LOCAL_seX(boundattr_string_hash),
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&sew_callattr_string_hash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&sew_callattr_string_hash_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_hash          = */ NULL,
	/* .tp_callattr_string_hash_kw       = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattr_string_hashf        = */ NULL,
	/* .tp_getattr_string_len_hash       = */ NULL,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&LOCAL_seX(delattr_string_len_hash),
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&LOCAL_seX(setattr_string_len_hash),
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&LOCAL_seX(hasattr_string_len_hash),
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&LOCAL_seX(boundattr_string_len_hash),
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&sew_callattr_string_len_hash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&sew_callattr_string_len_hash_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_len_hash      = */ NULL,
	/* .tp_callattr_string_len_hash_kw   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_findattr_info_string_len_hash = */ NULL,
};


STATIC_ASSERT(offsetof(LOCAL_SeqEach, se_seq) == offsetof(ProxyObject, po_obj));
#ifdef DEFINE_SeqEachGetAttr
#define sea_mh_seq_operator_size             generic_proxy__seq_operator_size
#define sea_mh_seq_operator_sizeob           generic_proxy__seq_operator_sizeob
#define sea_mh_seq_operator_bounditem        generic_proxy__seq_operator_bounditem
#define sea_mh_seq_operator_bounditem_index  generic_proxy__seq_operator_bounditem_index
#define sea_mh_seq_operator_hasitem          generic_proxy__seq_operator_hasitem
#define sea_mh_seq_operator_hasitem_index    generic_proxy__seq_operator_hasitem_index
#define sea_mh_seq_operator_delitem          generic_proxy__seq_operator_delitem
#define sea_mh_seq_operator_delitem_index    generic_proxy__seq_operator_delitem_index
#define sea_mh_seq_operator_delrange         generic_proxy__seq_operator_delrange
#define sea_mh_seq_operator_delrange_index   generic_proxy__seq_operator_delrange_index
#define sea_mh_seq_operator_delrange_index_n generic_proxy__seq_operator_delrange_index_n
#elif defined(DEFINE_SeqEachCallAttr)
#define sec_mh_seq_operator_size             generic_proxy__seq_operator_size
#define sec_mh_seq_operator_sizeob           generic_proxy__seq_operator_sizeob
#define sec_mh_seq_operator_bounditem        generic_proxy__seq_operator_bounditem
#define sec_mh_seq_operator_bounditem_index  generic_proxy__seq_operator_bounditem_index
#define sec_mh_seq_operator_hasitem          generic_proxy__seq_operator_hasitem
#define sec_mh_seq_operator_hasitem_index    generic_proxy__seq_operator_hasitem_index
#define sec_mh_seq_operator_delitem          generic_proxy__seq_operator_delitem
#define sec_mh_seq_operator_delitem_index    generic_proxy__seq_operator_delitem_index
#define sec_mh_seq_operator_delrange         generic_proxy__seq_operator_delrange
#define sec_mh_seq_operator_delrange_index   generic_proxy__seq_operator_delrange_index
#define sec_mh_seq_operator_delrange_index_n generic_proxy__seq_operator_delrange_index_n
#elif defined(DEFINE_SeqEachCallAttrKw)
#define sek_mh_seq_operator_size             generic_proxy__seq_operator_size
#define sek_mh_seq_operator_sizeob           generic_proxy__seq_operator_sizeob
#define sek_mh_seq_operator_bounditem        generic_proxy__seq_operator_bounditem
#define sek_mh_seq_operator_bounditem_index  generic_proxy__seq_operator_bounditem_index
#define sek_mh_seq_operator_hasitem          generic_proxy__seq_operator_hasitem
#define sek_mh_seq_operator_hasitem_index    generic_proxy__seq_operator_hasitem_index
#define sek_mh_seq_operator_delitem          generic_proxy__seq_operator_delitem
#define sek_mh_seq_operator_delitem_index    generic_proxy__seq_operator_delitem_index
#define sek_mh_seq_operator_delrange         generic_proxy__seq_operator_delrange
#define sek_mh_seq_operator_delrange_index   generic_proxy__seq_operator_delrange_index
#define sek_mh_seq_operator_delrange_index_n generic_proxy__seq_operator_delrange_index_n
#endif /* !... */

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_seX(mh_seq_operator_getitem)(LOCAL_SeqEach *self, DeeObject *index) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(seq_operator_getitem,
	                                                    self->se_seq, index);
	if likely(result)
		result = LOCAL_seX(transform_inherited)(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_seX(mh_seq_operator_getitem_index)(LOCAL_SeqEach *self, size_t index) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(seq_operator_getitem_index,
	                                                    self->se_seq, index);
	if likely(result)
		result = LOCAL_seX(transform_inherited)(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
LOCAL_seX(mh_seq_operator_trygetitem)(LOCAL_SeqEach *self, DeeObject *index) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(seq_operator_trygetitem,
	                                                    self->se_seq, index);
	if likely(ITER_ISOK(result))
		result = LOCAL_seX(transform_inherited)(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_seX(mh_seq_operator_trygetitem_index)(LOCAL_SeqEach *self, size_t index) {
	DREF DeeObject *result = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index,
	                                                    self->se_seq, index);
	if likely(ITER_ISOK(result))
		result = LOCAL_seX(transform_inherited)(self, result);
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF LOCAL_SeqEach *DCALL
LOCAL_seX(wraprange)(LOCAL_SeqEach *self, /*inherit(always)*/ DREF DeeObject *base) {
	DREF LOCAL_SeqEach *result;
#ifdef DEFINE_SeqEachGetAttr
	result = DeeObject_MALLOC(LOCAL_SeqEach);
#else /* DEFINE_SeqEachGetAttr */
	result = (DREF LOCAL_SeqEach *)DeeObject_Mallocc(offsetof(LOCAL_SeqEach, sg_argv),
	                                                 self->sg_argc, sizeof(DREF DeeObject *));
#endif /* !DEFINE_SeqEachGetAttr */
	if unlikely(!result)
		goto err_base;
	result->se_seq  = base; /* Inherit reference. */
	result->sg_attr = self->sg_attr;
	Dee_Incref(self->sg_attr);
#ifdef DEFINE_SeqEachCallAttrKw
	result->sg_kw = self->sg_kw;
	Dee_Incref(self->sg_kw);
#endif /* DEFINE_SeqEachCallAttrKw */
#if defined(DEFINE_SeqEachCallAttr) || defined(DEFINE_SeqEachCallAttrKw)
	result->sg_argc = self->sg_argc;
	Dee_Movrefv(result->sg_argv, self->sg_argv, self->sg_argc);
#endif /* DEFINE_SeqEachCallAttr || DEFINE_SeqEachCallAttrKw */
	DeeObject_Init(result, &LOCAL_SeqEach_Type);
	return result;
err_base:
	Dee_Decref(base);
/*err:*/
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF LOCAL_SeqEach *DCALL
LOCAL_seX(mh_seq_operator_getrange)(LOCAL_SeqEach *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *base = DeeObject_InvokeMethodHint(seq_operator_getrange,
	                                                  self->se_seq, start, end);
	if unlikely(!base)
		goto err;
	return LOCAL_seX(wraprange)(self, base);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF LOCAL_SeqEach *DCALL
LOCAL_seX(mh_seq_operator_getrange_index)(LOCAL_SeqEach *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *base = DeeObject_InvokeMethodHint(seq_operator_getrange_index,
	                                                  self->se_seq, start, end);
	if unlikely(!base)
		goto err;
	return LOCAL_seX(wraprange)(self, base);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF LOCAL_SeqEach *DCALL
LOCAL_seX(mh_seq_operator_getrange_index_n)(LOCAL_SeqEach *self, Dee_ssize_t start) {
	DREF DeeObject *base = DeeObject_InvokeMethodHint(seq_operator_getrange_index_n,
	                                                  self->se_seq, start);
	if unlikely(!base)
		goto err;
	return LOCAL_seX(wraprange)(self, base);
err:
	return NULL;
}


#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_GETITEM
#define LOCAL_seX_operator_size_PTR                       &LOCAL_seX(mh_seq_operator_size)
#define LOCAL_seX_operator_sizeob_PTR                     &LOCAL_seX(mh_seq_operator_sizeob)
#define LOCAL_seX_operator_getitem_PTR                    &LOCAL_seX(mh_seq_operator_getitem)
#define LOCAL_seX_operator_getitem_index_PTR              &LOCAL_seX(mh_seq_operator_getitem_index)
#define LOCAL_seX_operator_trygetitem_PTR                 &LOCAL_seX(mh_seq_operator_trygetitem)
#define LOCAL_seX_operator_trygetitem_index_PTR           &LOCAL_seX(mh_seq_operator_trygetitem_index)
#define LOCAL_seX_operator_bounditem_PTR                  &LOCAL_seX(mh_seq_operator_bounditem)
#define LOCAL_seX_operator_bounditem_index_PTR            &LOCAL_seX(mh_seq_operator_bounditem_index)
#define LOCAL_seX_operator_hasitem_PTR                    &LOCAL_seX(mh_seq_operator_hasitem)
#define LOCAL_seX_operator_hasitem_index_PTR              &LOCAL_seX(mh_seq_operator_hasitem_index)
#define LOCAL_seX_operator_trygetitem_string_hash_PTR     DEFIMPL(&default__trygetitem_string_hash__with__trygetitem)
#define LOCAL_seX_operator_trygetitem_string_len_hash_PTR DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem)
#define LOCAL_seX_operator_getitem_string_hash_PTR        DEFIMPL(&default__getitem_string_hash__with__getitem)
#define LOCAL_seX_operator_getitem_string_len_hash_PTR    DEFIMPL(&default__getitem_string_len_hash__with__getitem)
#define LOCAL_seX_operator_bounditem_string_hash_PTR      DEFIMPL(&default__bounditem_string_hash__with__bounditem)
#define LOCAL_seX_operator_bounditem_string_len_hash_PTR  DEFIMPL(&default__bounditem_string_len_hash__with__bounditem)
#define LOCAL_seX_operator_hasitem_string_hash_PTR        DEFIMPL(&default__hasitem_string_hash__with__hasitem)
#define LOCAL_seX_operator_hasitem_string_len_hash_PTR    DEFIMPL(&default__hasitem_string_len_hash__with__hasitem)
#define LOCAL_seX_operator_getrange_PTR                   &LOCAL_seX(mh_seq_operator_getrange)
#define LOCAL_seX_operator_getrange_index_PTR             &LOCAL_seX(mh_seq_operator_getrange_index)
#define LOCAL_seX_operator_getrange_index_n_PTR           &LOCAL_seX(mh_seq_operator_getrange_index_n)
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_GETITEM */
#define LOCAL_seX_operator_bounditem_PTR &LOCAL_seX(operator_bounditem)
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(operator_bounditem)(LOCAL_SeqEach *self, DeeObject *index) {
	return seqeach_map_fe2bound(LOCAL_seX(foreach)(self, &se_bounditem_foreach_cb, index));
}

#define LOCAL_seX_operator_hasitem_PTR &LOCAL_seX(operator_hasitem)
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(operator_hasitem)(LOCAL_SeqEach *self, DeeObject *index) {
	return seqeach_map_fe2has(LOCAL_seX(foreach)(self, &se_hasitem_foreach_cb, index));
}

#define LOCAL_seX_operator_bounditem_index_PTR &LOCAL_seX(operator_bounditem_index)
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seX(operator_bounditem_index)(LOCAL_SeqEach *self, size_t index) {
	return seqeach_map_fe2bound(LOCAL_seX(foreach)(self, &se_bounditem_index_foreach_cb,
	                                               (void *)(uintptr_t)index));
}

#define LOCAL_seX_operator_hasitem_index_PTR &LOCAL_seX(operator_hasitem_index)
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seX(operator_hasitem_index)(LOCAL_SeqEach *self, size_t index) {
	return seqeach_map_fe2has(LOCAL_seX(foreach)(self, &se_hasitem_index_foreach_cb,
	                                             (void *)(uintptr_t)index));
}

#define LOCAL_seX_operator_bounditem_string_hash_PTR &LOCAL_seX(operator_bounditem_string_hash)
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(operator_bounditem_string_hash)(LOCAL_SeqEach *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_fe2bound(LOCAL_seX(foreach)(self, &se_bounditem_string_hash_foreach_cb, &data));
}

#define LOCAL_seX_operator_hasitem_string_hash_PTR &LOCAL_seX(operator_hasitem_string_hash)
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(operator_hasitem_string_hash)(LOCAL_SeqEach *self, char const *key, Dee_hash_t hash) {
	struct se_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqeach_map_fe2has(LOCAL_seX(foreach)(self, &se_hasitem_string_hash_foreach_cb, &data));
}

#define LOCAL_seX_operator_bounditem_string_len_hash_PTR &LOCAL_seX(operator_bounditem_string_len_hash)
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(operator_bounditem_string_len_hash)(LOCAL_SeqEach *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_fe2bound(LOCAL_seX(foreach)(self, &se_bounditem_string_len_hash_foreach_cb, &data));
}

#define LOCAL_seX_operator_hasitem_string_len_hash_PTR &LOCAL_seX(operator_hasitem_string_len_hash)
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(operator_hasitem_string_len_hash)(LOCAL_SeqEach *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqeach_map_fe2has(LOCAL_seX(foreach)(self, &se_hasitem_string_len_hash_foreach_cb, &data));
}

#define LOCAL_seX_operator_sizeob_PTR                     &sew_sizeob
#define LOCAL_seX_operator_size_PTR                       &default__size__with__sizeob
#define LOCAL_seX_operator_getitem_PTR                    &sew_getitem
#define LOCAL_seX_operator_getitem_index_PTR              &default__getitem_index__with__getitem
#define LOCAL_seX_operator_getitem_string_hash_PTR        &default__getitem_string_hash__with__getitem
#define LOCAL_seX_operator_getitem_string_len_hash_PTR    &default__getitem_string_len_hash__with__getitem
#define LOCAL_seX_operator_trygetitem_PTR                 &sew_getitem
#define LOCAL_seX_operator_trygetitem_index_PTR           &default__getitem_index__with__getitem
#define LOCAL_seX_operator_trygetitem_string_hash_PTR     &default__getitem_string_hash__with__getitem
#define LOCAL_seX_operator_trygetitem_string_len_hash_PTR &default__getitem_string_len_hash__with__getitem
#define LOCAL_seX_operator_getrange_PTR                   &sew_getrange
#define LOCAL_seX_operator_getrange_index_PTR             &default__getrange_index__with__getrange
#define LOCAL_seX_operator_getrange_index_n_PTR           &default__getrange_index_n__with__getrange
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_GETITEM */

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_CONTAINS
#define LOCAL_seX_operator_contains_PTR &default__seq_operator_contains__with__seq_contains
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_CONTAINS */
#define LOCAL_seX_operator_contains_PTR &sew_contains
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_CONTAINS */

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_ITER
#define LOCAL_seX_operator_iter_PTR    &LOCAL_seX(mh_seq_operator_iter)
#define LOCAL_seX_operator_foreach_PTR &LOCAL_seX(foreach)
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_ITER */
#define LOCAL_seX_operator_iter_PTR    &sew_iter
#define LOCAL_seX_operator_foreach_PTR &default__foreach__with__iter
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_ITER */


PRIVATE struct type_seq LOCAL_seX(seq) = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&LOCAL_seX(mh_seq_operator_iter),
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))LOCAL_seX_operator_sizeob_PTR,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))LOCAL_seX_operator_contains_PTR,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))LOCAL_seX_operator_getitem_PTR,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_seX(delitem),
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&LOCAL_seX(setitem),
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))LOCAL_seX_operator_getrange_PTR,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&LOCAL_seX(delrange),
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&LOCAL_seX(setrange),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))LOCAL_seX_operator_foreach_PTR,
	/* .tp_foreach_pair               = */ &default__foreach_pair__with__foreach,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))LOCAL_seX_operator_bounditem_PTR,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))LOCAL_seX_operator_hasitem_PTR,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))LOCAL_seX_operator_size_PTR,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&sew_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))LOCAL_seX_operator_getitem_index_PTR,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&LOCAL_seX(delitem_index),
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&LOCAL_seX(setitem_index),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))LOCAL_seX_operator_bounditem_index_PTR,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))LOCAL_seX_operator_hasitem_index_PTR,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))LOCAL_seX_operator_getrange_index_PTR,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&LOCAL_seX(delrange_index),
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&LOCAL_seX(setrange_index),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))LOCAL_seX_operator_getrange_index_n_PTR,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&LOCAL_seX(delrange_index_n),
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&LOCAL_seX(setrange_index_n),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))LOCAL_seX_operator_trygetitem_PTR,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))LOCAL_seX_operator_trygetitem_index_PTR,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))LOCAL_seX_operator_trygetitem_string_hash_PTR,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))LOCAL_seX_operator_getitem_string_hash_PTR,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&LOCAL_seX(delitem_string_hash),
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&LOCAL_seX(setitem_string_hash),
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))LOCAL_seX_operator_bounditem_string_hash_PTR,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))LOCAL_seX_operator_hasitem_string_hash_PTR,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))LOCAL_seX_operator_trygetitem_string_len_hash_PTR,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))LOCAL_seX_operator_getitem_string_len_hash_PTR,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&LOCAL_seX(delitem_string_len_hash),
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&LOCAL_seX(setitem_string_len_hash),
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))LOCAL_seX_operator_bounditem_string_len_hash_PTR,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))LOCAL_seX_operator_hasitem_string_len_hash_PTR,
};

PRIVATE struct type_method_hint LOCAL_seX(method_hints)[] = {
	TYPE_METHOD_HINT(seq_enumerate, &LOCAL_seX(mh_seq_enumerate)),
	TYPE_METHOD_HINT(seq_enumerate_index, &LOCAL_seX(mh_seq_enumerate_index)),
	TYPE_METHOD_HINT(seq_operator_iter, &LOCAL_seX(mh_seq_operator_iter)),
	TYPE_METHOD_HINT(seq_operator_foreach, &LOCAL_seX(foreach)),
	TYPE_METHOD_HINT(seq_operator_getitem, &LOCAL_seX(mh_seq_operator_getitem)),
	TYPE_METHOD_HINT(seq_operator_getitem_index, &LOCAL_seX(mh_seq_operator_getitem_index)),
	TYPE_METHOD_HINT(seq_operator_trygetitem, &LOCAL_seX(mh_seq_operator_trygetitem)),
	TYPE_METHOD_HINT(seq_operator_trygetitem_index, &LOCAL_seX(mh_seq_operator_trygetitem_index)),
	TYPE_METHOD_HINT(seq_operator_bounditem, &LOCAL_seX(mh_seq_operator_bounditem)),
	TYPE_METHOD_HINT(seq_operator_bounditem_index, &LOCAL_seX(mh_seq_operator_bounditem_index)),
	TYPE_METHOD_HINT(seq_operator_hasitem, &LOCAL_seX(mh_seq_operator_hasitem)),
	TYPE_METHOD_HINT(seq_operator_hasitem_index, &LOCAL_seX(mh_seq_operator_hasitem_index)),
	TYPE_METHOD_HINT(seq_operator_delitem, &LOCAL_seX(mh_seq_operator_delitem)),
	TYPE_METHOD_HINT(seq_operator_delitem_index, &LOCAL_seX(mh_seq_operator_delitem_index)),
	TYPE_METHOD_HINT(seq_operator_setitem_index, &LOCAL_seX(setitem_index)),
	TYPE_METHOD_HINT(seq_operator_getrange, &LOCAL_seX(mh_seq_operator_getrange)),
	TYPE_METHOD_HINT(seq_operator_getrange_index, &LOCAL_seX(mh_seq_operator_getrange_index)),
	TYPE_METHOD_HINT(seq_operator_getrange_index_n, &LOCAL_seX(mh_seq_operator_getrange_index_n)),
	TYPE_METHOD_HINT(seq_operator_delrange, &LOCAL_seX(mh_seq_operator_delrange)),
	TYPE_METHOD_HINT(seq_operator_delrange_index, &LOCAL_seX(mh_seq_operator_delrange_index)),
	TYPE_METHOD_HINT(seq_operator_delrange_index_n, &LOCAL_seX(mh_seq_operator_delrange_index_n)),
	TYPE_METHOD_HINT(seq_operator_size, &LOCAL_seX(mh_seq_operator_size)),
	TYPE_METHOD_HINT(seq_operator_sizeob, &LOCAL_seX(mh_seq_operator_sizeob)),
	TYPE_METHOD_HINT_END
};

#undef LOCAL_seX_operator_iter_PTR
#undef LOCAL_seX_operator_foreach_PTR
#undef LOCAL_seX_operator_sizeob_PTR
#undef LOCAL_seX_operator_contains_PTR
#undef LOCAL_seX_operator_getitem_PTR
#undef LOCAL_seX_operator_getrange_PTR
#undef LOCAL_seX_operator_bounditem_PTR
#undef LOCAL_seX_operator_hasitem_PTR
#undef LOCAL_seX_operator_size_PTR
#undef LOCAL_seX_operator_getitem_index_PTR
#undef LOCAL_seX_operator_bounditem_index_PTR
#undef LOCAL_seX_operator_hasitem_index_PTR
#undef LOCAL_seX_operator_getrange_index_PTR
#undef LOCAL_seX_operator_getrange_index_n_PTR
#undef LOCAL_seX_operator_trygetitem_PTR
#undef LOCAL_seX_operator_trygetitem_index_PTR
#undef LOCAL_seX_operator_trygetitem_string_hash_PTR
#undef LOCAL_seX_operator_getitem_string_hash_PTR
#undef LOCAL_seX_operator_bounditem_string_hash_PTR
#undef LOCAL_seX_operator_hasitem_string_hash_PTR
#undef LOCAL_seX_operator_trygetitem_string_len_hash_PTR
#undef LOCAL_seX_operator_getitem_string_len_hash_PTR
#undef LOCAL_seX_operator_bounditem_string_len_hash_PTR
#undef LOCAL_seX_operator_hasitem_string_len_hash_PTR

PRIVATE struct type_member tpconst LOCAL_seX(members)[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT_AB, offsetof(LOCAL_SeqEach, se_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__attr__", STRUCT_OBJECT_AB, offsetof(LOCAL_SeqEach, sg_attr), "->?Dstring"),
#ifdef DEFINE_SeqEachCallAttrKw
	TYPE_MEMBER_FIELD_DOC("__kw__", STRUCT_OBJECT_AB, offsetof(LOCAL_SeqEach, sg_kw), "->?M?Dstring?O"),
#endif /* DEFINE_SeqEachCallAttrKw */
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst LOCAL_seX(class_members)[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &LOCAL_SeqEachIterator_Type),
	TYPE_MEMBER_END
};

#ifdef DEFINE_SeqEachGetAttr
STATIC_ASSERT(offsetof(LOCAL_SeqEach, se_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(LOCAL_SeqEach, se_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(LOCAL_SeqEach, sg_attr) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(LOCAL_SeqEach, sg_attr) == offsetof(ProxyObject2, po_obj2));
#define sea_fini  generic_proxy2__fini
#define sea_visit generic_proxy2__visit
#else /* DEFINE_SeqEachGetAttr */
PRIVATE NONNULL((1)) void DCALL
LOCAL_seX(fini)(LOCAL_SeqEach *__restrict self) {
	Dee_Decref(self->se_seq);
	Dee_Decref(self->sg_attr);
#ifdef DEFINE_SeqEachCallAttrKw
	Dee_Decref(self->sg_kw);
#endif /* DEFINE_SeqEachCallAttrKw */
#if defined(DEFINE_SeqEachCallAttr) || defined(DEFINE_SeqEachCallAttrKw)
	Dee_Decrefv(self->sg_argv, self->sg_argc);
#endif /* DEFINE_SeqEachCallAttr || DEFINE_SeqEachCallAttrKw */
}

PRIVATE NONNULL((1, 2)) void DCALL
LOCAL_seX(visit)(LOCAL_SeqEach *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->se_seq);
	Dee_Visit(self->sg_attr);
#ifdef DEFINE_SeqEachCallAttrKw
	Dee_Visit(self->sg_kw);
#endif /* DEFINE_SeqEachCallAttrKw */
#if defined(DEFINE_SeqEachCallAttr) || defined(DEFINE_SeqEachCallAttrKw)
	Dee_Visitv(self->sg_argv, self->sg_argc);
#endif /* DEFINE_SeqEachCallAttr || DEFINE_SeqEachCallAttrKw */
}
#endif /* !DEFINE_SeqEachGetAttr */

#ifndef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_seX(printrepr)(LOCAL_SeqEach *__restrict self,
                     Dee_formatprinter_t printer, void *arg) {
	char const *each_suffix = ".each";
	DeeTypeObject *seq_type = Dee_TYPE(self->se_seq);
	if (DeeType_IsSeqEachWrapper(seq_type))
		each_suffix = "";
#ifdef DEFINE_SeqEachGetAttr
	return DeeFormat_Printf(printer, arg, "%r%s.%k",
	                        self->se_seq, each_suffix,
	                        self->sg_attr);
#elif defined(DEFINE_SeqEachCallAttr) || defined(DEFINE_SeqEachCallAttrKw)
	{
#ifdef DEFINE_SeqEachCallAttrKw
		DeeObject *argv[2];
#else /* DEFINE_SeqEachCallAttrKw */
		DeeObject *argv[1];
#endif /* !DEFINE_SeqEachCallAttrKw */
		Dee_ssize_t result = -1;
		size_t full_suffix_len;
		char *full_suffix, *p;
		char const *attr_utf8;
		attr_utf8 = DeeString_AsUtf8(Dee_AsObject(self->sg_attr));
		if unlikely(!attr_utf8)
			return -1;
		full_suffix_len = strlen(each_suffix);
		full_suffix_len += 1; /* "." */
		full_suffix_len += WSTR_LENGTH(attr_utf8);
		full_suffix = (char *)Dee_Mallocac(full_suffix_len, sizeof(char));
		if unlikely(!full_suffix)
			return -1;
		p = (char *)mempcpyc(full_suffix, each_suffix, strlen(each_suffix), sizeof(char));
		*p++ = '.';
		p = (char *)mempcpyc(p, attr_utf8, WSTR_LENGTH(attr_utf8), sizeof(char));
		(void)p;
		ASSERT(full_suffix + full_suffix_len == p);
		argv[0] = DeeTuple_NewVectorSymbolic(self->sg_argc, self->sg_argv);
		if likely(argv[0]) {
#ifdef DEFINE_SeqEachCallAttrKw
			argv[1] = self->sg_kw;
#endif /* !DEFINE_SeqEachCallAttrKw */
			result = DeeFormat_PrintOperatorRepr(printer, arg, self->se_seq,
			                                     OPERATOR_CALL, COMPILER_LENOF(argv), argv,
			                                     NULL, 0, full_suffix, full_suffix_len);
			DeeTuple_DecrefSymbolic(argv[0]);
		}
		Dee_Freea(full_suffix);
		return result;
	}
#else /* ... */
#error "Unsupported each-fastpass mode"
#endif /* !... */
}
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR */


#ifdef DEFINE_SeqEachGetAttr
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seX(ctor)(LOCAL_SeqEach *__restrict self) {
	self->se_seq  = DeeSeq_NewEmpty();
	self->sg_attr = (DREF DeeStringObject *)DeeString_NewEmpty();
	return 0;
}

#if 1
STATIC_ASSERT(offsetof(LOCAL_SeqEach, se_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(LOCAL_SeqEach, se_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(LOCAL_SeqEach, sg_attr) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(LOCAL_SeqEach, sg_attr) == offsetof(ProxyObject2, po_obj2));
#define sea_copy generic_proxy2__copy_alias12
#else
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_seX(copy)(LOCAL_SeqEach *__restrict self,
                LOCAL_SeqEach *__restrict other) {
	self->se_seq  = other->se_seq;
	self->sg_attr = other->sg_attr;
	Dee_Incref(self->se_seq);
	Dee_Incref(self->sg_attr);
	return 0;
}
#endif

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seX(init)(LOCAL_SeqEach *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack2(err, argc, argv, "_" LOCAL_SeqEach_Type_NAME,
	                &self->se_seq, &self->sg_attr);
	if (DeeObject_AssertTypeExact(self->sg_attr, &DeeString_Type))
		goto err;
	Dee_Incref(self->se_seq);
	Dee_Incref(self->sg_attr);
	return 0;
err:
	return -1;
}

#elif defined(DEFINE_SeqEachCallAttr) || defined(DEFINE_SeqEachCallAttrKw)

PRIVATE WUNUSED DREF LOCAL_SeqEach *DCALL LOCAL_seX(ctor)(void) {
	DREF LOCAL_SeqEach *result;
	result = (DREF LOCAL_SeqEach *)DeeObject_Malloc(offsetof(LOCAL_SeqEach, sg_argv));
	if unlikely(!result)
		goto done;
	result->se_seq  = DeeSeq_NewEmpty();
	result->sg_attr = (DREF DeeStringObject *)DeeString_NewEmpty();
	result->sg_argc = 0;
#ifdef DEFINE_SeqEachCallAttrKw
	result->sg_kw   = DeeMapping_NewEmpty();
#endif /* DEFINE_SeqEachCallAttrKw */
	DeeObject_Init(result, &LOCAL_SeqEach_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF LOCAL_SeqEach *DCALL
LOCAL_seX(copy)(LOCAL_SeqEach *__restrict other) {
	DREF LOCAL_SeqEach *result;
	result = (DREF LOCAL_SeqEach *)DeeObject_Mallocc(offsetof(LOCAL_SeqEach, sg_argv),
	                                                 other->sg_argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->se_seq  = other->se_seq;
	result->sg_attr = other->sg_attr;
	result->sg_argc = other->sg_argc;
	Dee_Movrefv(result->sg_argv, other->sg_argv, result->sg_argc);
#ifdef DEFINE_SeqEachCallAttrKw
	result->sg_kw = other->sg_kw;
	Dee_Incref(result->sg_kw);
#endif /* DEFINE_SeqEachCallAttrKw */
	Dee_Incref(result->se_seq);
	Dee_Incref(result->sg_attr);
	DeeObject_Init(result, &LOCAL_SeqEach_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF LOCAL_SeqEach *DCALL
LOCAL_seX(init)(size_t argc, DeeObject *const *argv) {
	DREF LOCAL_SeqEach *result;
	DeeStringObject *attr;
	DeeObject *seq;
	DeeObject *args = Dee_EmptyTuple;
#ifdef DEFINE_SeqEachCallAttrKw
	DeeObject *kw = Dee_EmptyMapping;
#endif /* DEFINE_SeqEachCallAttrKw */
	if (DeeArg_Unpack(argc, argv,
	                  "oo|o"
#ifdef DEFINE_SeqEachCallAttrKw
	                  "o"
#endif /* DEFINE_SeqEachCallAttrKw */
	                  ":_" LOCAL_SeqEach_Type_NAME,
	                  &seq, &attr, &args
#ifdef DEFINE_SeqEachCallAttrKw
	                  , &kw
#endif /* DEFINE_SeqEachCallAttrKw */
	                  ))
		goto err;
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	result = (DREF LOCAL_SeqEach *)DeeObject_Mallocc(offsetof(LOCAL_SeqEach, sg_argv),
	                                                 DeeTuple_SIZE(args),
	                                                 sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto err;
	result->sg_argc = DeeTuple_SIZE(args);
	Dee_Movrefv(result->sg_argv, DeeTuple_ELEM(args), DeeTuple_SIZE(args));
#ifdef DEFINE_SeqEachCallAttrKw
	result->sg_kw = kw;
	Dee_Incref(kw);
#endif /* DEFINE_SeqEachCallAttrKw */
	result->se_seq  = seq;
	result->sg_attr = attr;
	Dee_Incref(seq);
	Dee_Incref(attr);
	DeeObject_Init(result, &LOCAL_SeqEach_Type);
	return result;
err:
	return NULL;
}
#else /* ... */
#error "Unsupported mode"
#endif /* !... */




#ifdef DEFINE_SeqEachGetAttr
STATIC_ASSERT(offsetof(LOCAL_SeqEach, se_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(LOCAL_SeqEach, se_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(LOCAL_SeqEach, sg_attr) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(LOCAL_SeqEach, sg_attr) == offsetof(ProxyObject2, po_obj2));
#define sea_serialize generic_proxy2__serialize
#elif defined(DEFINE_SeqEachCallAttr) || defined(DEFINE_SeqEachCallAttrKw)
PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
LOCAL_seX(serialize)(LOCAL_SeqEach *__restrict self,
                     DeeSerial *__restrict writer) {
	LOCAL_SeqEach *out;
	Dee_seraddr_t out_addr;
	size_t i, sizeof_self;
	sizeof_self = _Dee_MallococBufsize(offsetof(LOCAL_SeqEach, sg_argv),
	                                   self->sg_argc, sizeof(DREF DeeObject *));
	out_addr = DeeSerial_ObjectMalloc(writer, sizeof_self, self);
	if (!Dee_SERADDR_ISOK(out_addr))
		goto err;
#define ADDROF(field) (out_addr + offsetof(LOCAL_SeqEach, field))
	if (DeeSerial_PutObject(writer, ADDROF(se_seq), self->se_seq))
		goto err;
	if (DeeSerial_PutObject(writer, ADDROF(sg_attr), self->sg_attr))
		goto err;
#ifdef DEFINE_SeqEachCallAttrKw
	if (DeeSerial_PutObject(writer, ADDROF(sg_kw), self->sg_kw))
		goto err;
#endif /* DEFINE_SeqEachCallAttrKw */
	out = DeeSerial_Addr2Mem(writer, out_addr, LOCAL_SeqEach);
	out->sg_argc = self->sg_argc;
	for (i = 0; i < out->sg_argc; ++i) {
		Dee_seraddr_t addrof_argv_i = ADDROF(sg_argv) + i * sizeof(DREF DeeObject *);
		if (DeeSerial_PutObject(writer, addrof_argv_i, self->sg_argv[i]))
			goto err;
	}
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
#undef ADDROF
}
#else /* ... */
#error "Unsupported mode"
#endif /* !... */



#ifdef DEFINE_SeqEachGetAttr
PRIVATE WUNUSED DREF SeqEachCallAttr *DCALL
LOCAL_seX(call)(LOCAL_SeqEach *__restrict self, size_t argc, DeeObject *const *argv) {
	return (DREF SeqEachCallAttr *)DeeSeqEach_CallAttr(self->se_seq,
	                                                   Dee_AsObject(self->sg_attr),
	                                                   argc, argv);
}

PRIVATE WUNUSED DREF SeqEachCallAttrKw *DCALL
LOCAL_seX(call_kw)(LOCAL_SeqEach *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return (DREF SeqEachCallAttrKw *)DeeSeqEach_CallAttrKw(self->se_seq,
	                                                       Dee_AsObject(self->sg_attr),
	                                                       argc, argv, kw);
}
#endif /* DEFINE_SeqEachGetAttr */

#ifdef DEFINE_SeqEachGetAttr
PRIVATE struct type_callable LOCAL_seX(callable) = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&LOCAL_seX(call_kw),
};
#endif /* DEFINE_SeqEachGetAttr */

#ifdef CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR
#define LOCAL_seX_operator_printrepr_PTR &default_seq_printrepr
#else /* CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR */
#define LOCAL_seX_operator_printrepr_PTR &seo_printrepr
#endif /* !CONFIG_HAVE_SEQEACHOPERATOR_HAS_SEQLIKE_REPR */

INTERN DeeTypeObject LOCAL_SeqEach_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_" LOCAL_SeqEach_Type_NAME,
#ifdef CONFIG_NO_DOC
	/* .tp_doc      = */ NULL,
#else /* CONFIG_NO_DOC */
	/* .tp_doc      = */ "When deemon was compiled with #CCONFIG_NO_SEQEACH_ATTRIBUTE_OPTIMIZATIONS "
	                     /**/ "(default when #C__OPTIMIZE_SIZE__ was enabled), this type won't exist, "
	                     /**/ "and ?Ert:" LOCAL_SeqEach_Type_NAME "_np will evaluate to "
	                     /**/ "?Ert:SeqEachOperator (hence the #C{*_np} suffix in ?Mrt)\n"
	                     "\n"
	                     "()\n"
#ifdef DEFINE_SeqEachGetAttr
	/* .tp_doc      = */ "(seq:?DSequence,attr:?Dstring)\n"
	                     "Optimized implementation of ${<seq>.each.<attr>}"
#elif defined(DEFINE_SeqEachCallAttr)
	/* .tp_doc      = */ "(seq:?DSequence,attr:?Dstring,args=!T0)\n"
	                     "Optimized implementation of ${<seq>.each.<attr>(<args>...)}"
#elif defined(DEFINE_SeqEachCallAttrKw)
	/* .tp_doc      = */ "(seq:?DSequence,attr:?Dstring,args=!T0,kw:?M?Dstring?O=!T0)\n"
	                     "Optimized implementation of ${<seq>.each.<attr>(<args>..., **<kw>)}"
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
	                     "",
#endif /* !CONFIG_NO_DOC */
	/* .tp_flags    = */ LOCAL_SeqEach_Type_FLAGS,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
#if !(LOCAL_SeqEach_Type_FLAGS & TP_FVARIABLE)
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ LOCAL_SeqEach,
			/* tp_ctor:        */ &LOCAL_seX(ctor),
			/* tp_copy_ctor:   */ &LOCAL_seX(copy),
			/* tp_any_ctor:    */ &LOCAL_seX(init),
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &LOCAL_seX(serialize)
		),
#else /* !(LOCAL_SeqEach_Type_FLAGS & TP_FVARIABLE) */
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &LOCAL_seX(ctor),
			/* tp_copy_ctor:   */ &LOCAL_seX(copy),
			/* tp_any_ctor:    */ &LOCAL_seX(init),
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &LOCAL_seX(serialize),
			/* tp_free:        */ NULL
		),
#endif /* LOCAL_SeqEach_Type_FLAGS & TP_FVARIABLE */
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&LOCAL_seX(fini),
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_seX(assign),
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_seX(moveassign),
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_seX(bool),
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))LOCAL_seX_operator_printrepr_PTR,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&LOCAL_seX(visit),
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &LOCAL_seX(math),
	/* .tp_cmp           = */ &sew_cmp,
	/* .tp_seq           = */ &LOCAL_seX(seq),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_iter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &LOCAL_seX(attr),
	/* .tp_with          = */ &sew_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ sew_methods,
	/* .tp_getsets       = */ NULL, /* TODO: Access to the arguments vector */
	/* .tp_members       = */ LOCAL_seX(members),
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ LOCAL_seX(class_members),
	/* .tp_method_hints  = */ LOCAL_seX(method_hints),
#ifdef DEFINE_SeqEachGetAttr
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&LOCAL_seX(call),
	/* .tp_callable      = */ &LOCAL_seX(callable),
#else /* DEFINE_SeqEachGetAttr */
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sew_call,
	/* .tp_callable      = */ &sew_callable,
#endif /* !DEFINE_SeqEachGetAttr */
};

#undef LOCAL_seX_operator_printrepr_PTR





PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seXi(ctor)(SeqEachIterator *__restrict self) {
	self->ei_each = (DREF SeqEachBase *)DeeObject_NewDefault(&LOCAL_SeqEach_Type);
	if unlikely(!self->ei_each)
		goto err;
	self->ei_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->ei_each->se_seq);
	if unlikely(!self->ei_iter)
		goto err_each;
	return 0;
err_each:
	Dee_Decref(self->ei_each);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_seXi(init)(SeqEachIterator *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_" LOCAL_SeqEach_Type_NAME "Iterator", &self->ei_each);
	if (DeeObject_AssertTypeExact(self->ei_each, &LOCAL_SeqEach_Type))
		goto err;
	self->ei_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->ei_each->se_seq);
	if unlikely(!self->ei_iter)
		goto err;
	Dee_Incref(self->ei_each);
	return 0;
err:
	return -1;
}

#ifndef CONFIG_NO_DOC
PRIVATE struct type_member tpconst LOCAL_seXi(members)[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(SeqEachIterator, ei_each), "->?Ert:" LOCAL_SeqEach_Type_NAME),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT_AB, offsetof(SeqEachIterator, ei_iter), "->?DIterator"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */

PRIVATE WUNUSED DREF DeeObject *DCALL
LOCAL_seXi(next)(SeqEachIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_IterNext(self->ei_iter);
	if likely(ITER_ISOK(result))
		result = LOCAL_seX(transform_inherited)((LOCAL_SeqEach *)self->ei_each, result);
	return result;
}



INTERN DeeTypeObject LOCAL_SeqEachIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_" LOCAL_SeqEach_Type_NAME "Iterator",
	/* .tp_doc      = */ DOC("When deemon was compiled with #CCONFIG_NO_SEQEACH_ATTRIBUTE_OPTIMIZATIONS "
	                         /**/ "(default when #C__OPTIMIZE_SIZE__ was enabled), this type won't exist, "
	                         /**/ "and ?Ert:" LOCAL_SeqEach_Type_NAME "Iterator_np will evaluate to "
	                         /**/ "?Ert:SeqEachOperatorIterator (hence the #C{*_np} suffix in ?Mrt)\n"
	                         "\n"
	                         "(seq?:?Ert:" LOCAL_SeqEach_Type_NAME ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqEachIterator,
			/* tp_ctor:        */ &LOCAL_seXi(ctor),
			/* tp_copy_ctor:   */ &sewi_copy,
			/* tp_any_ctor:    */ &LOCAL_seXi(init),
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sewi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sewi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sewi_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sewi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sewi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&LOCAL_seXi(next),
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
#ifndef CONFIG_NO_DOC
	/* .tp_members       = */ LOCAL_seXi(members),
#else /* !CONFIG_NO_DOC */
	/* .tp_members       = */ seoi_members,
#endif /* CONFIG_NO_DOC */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




#ifdef CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS
PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_ssX(bool)(LOCAL_SeqEach *__restrict self) {
	Dee_ssize_t result = LOCAL_seX(foreach)(self, &ss_foreach_bool_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		return 1;
	return (int)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
LOCAL_ssX(printrepr)(LOCAL_SeqEach *__restrict self,
                     Dee_formatprinter_t printer, void *arg) {
	char const *some_suffix = ".some";
	DeeTypeObject *seq_type = Dee_TYPE(self->se_seq);
	if (DeeType_IsSeqEachWrapper(seq_type))
		some_suffix = "";
#ifdef DEFINE_SeqEachGetAttr
	return DeeFormat_Printf(printer, arg, "%r%s.%k",
	                        self->se_seq, some_suffix,
	                        self->sg_attr);
#elif defined(DEFINE_SeqEachCallAttr) || defined(DEFINE_SeqEachCallAttrKw)
	{
#ifdef DEFINE_SeqEachCallAttrKw
		DeeObject *argv[2];
#else /* DEFINE_SeqEachCallAttrKw */
		DeeObject *argv[1];
#endif /* !DEFINE_SeqEachCallAttrKw */
		Dee_ssize_t result = -1;
		size_t full_suffix_len;
		char *full_suffix, *p;
		char const *attr_utf8;
		attr_utf8 = DeeString_AsUtf8(Dee_AsObject(self->sg_attr));
		if unlikely(!attr_utf8)
			return -1;
		full_suffix_len = strlen(some_suffix);
		full_suffix_len += 1; /* "." */
		full_suffix_len += WSTR_LENGTH(attr_utf8);
		full_suffix = (char *)Dee_Mallocac(full_suffix_len, sizeof(char));
		if unlikely(!full_suffix)
			return -1;
		p = (char *)mempcpyc(full_suffix, some_suffix, strlen(some_suffix), sizeof(char));
		*p++ = '.';
		p = (char *)mempcpyc(p, attr_utf8, WSTR_LENGTH(attr_utf8), sizeof(char));
		(void)p;
		ASSERT(full_suffix + full_suffix_len == p);
		argv[0] = DeeTuple_NewVectorSymbolic(self->sg_argc, self->sg_argv);
		if likely(argv[0]) {
#ifdef DEFINE_SeqEachCallAttrKw
			argv[1] = self->sg_kw;
#endif /* !DEFINE_SeqEachCallAttrKw */
			result = DeeFormat_PrintOperatorRepr(printer, arg, self->se_seq,
			                                     OPERATOR_CALL, COMPILER_LENOF(argv), argv,
			                                     NULL, 0, full_suffix, full_suffix_len);
			DeeTuple_DecrefSymbolic(argv[0]);
		}
		Dee_Freea(full_suffix);
		return result;
	}
#else /* ... */
#error "Unsupported each-fastpass mode"
#endif /* !... */
}

#ifdef DEFINE_SeqEachGetAttr
PRIVATE WUNUSED DREF SeqEachCallAttr *DCALL
LOCAL_ssX(call)(LOCAL_SeqEach *__restrict self, size_t argc, DeeObject *const *argv) {
	return (DREF SeqEachCallAttr *)DeeSeqSome_CallAttr(self->se_seq,
	                                                   Dee_AsObject(self->sg_attr),
	                                                   argc, argv);
}

PRIVATE WUNUSED DREF SeqEachCallAttrKw *DCALL
LOCAL_ssX(call_kw)(LOCAL_SeqEach *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return (DREF SeqEachCallAttrKw *)DeeSeqSome_CallAttrKw(self->se_seq,
	                                                       Dee_AsObject(self->sg_attr),
	                                                       argc, argv, kw);
}
#endif /* DEFINE_SeqEachGetAttr */


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(trycompare_eq)(LOCAL_SeqEach *self, DeeObject *other) {
	Dee_ssize_t result = LOCAL_seX(foreach)(self, &ss_trycompare_eq_cb, other);
	ASSERT(result == SEQSOME_FOREACH_YES ||
	       result == SEQSOME_FOREACH_NO ||
	       result == SEQSOME_FOREACH_ERR);
	switch (result) {
	case SEQSOME_FOREACH_YES:
		return Dee_COMPARE_EQ; /* Some item *does* compare equal */
	case SEQSOME_FOREACH_NO:
		return Dee_COMPARE_NE; /* No items compare equal */
	case SEQSOME_FOREACH_ERR:
		return Dee_COMPARE_ERR;
	default: __builtin_unreachable();
	}
}

PRIVATE struct type_cmp LOCAL_ssX(cmp) = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_ssX(trycompare_eq),
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_ssX(trycompare_eq),
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_ge
};

STATIC_ASSERT(offsetof(SeqEachBase, se_seq) == offsetof(ProxyObject, po_obj));
#define ssw_size_fast generic_proxy__size_fast

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(bounditem)(LOCAL_SeqEach *self, DeeObject *index) {
	return seqsome_map_fe2bound(LOCAL_seX(foreach)(self, &ss_bounditem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(hasitem)(LOCAL_SeqEach *self, DeeObject *index) {
	return seqsome_map_fe2has(LOCAL_seX(foreach)(self, &ss_hasitem_foreach_cb, index));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_ssX(bounditem_index)(LOCAL_SeqEach *self, size_t index) {
	return seqsome_map_fe2bound(LOCAL_seX(foreach)(self, &ss_bounditem_index_foreach_cb,
	                                               (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
LOCAL_ssX(hasitem_index)(LOCAL_SeqEach *self, size_t index) {
	return seqsome_map_fe2has(LOCAL_seX(foreach)(self, &ss_hasitem_index_foreach_cb,
	                                             (void *)(uintptr_t)index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(bounditem_string_hash)(LOCAL_SeqEach *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_fe2bound(LOCAL_seX(foreach)(self, &ss_bounditem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(hasitem_string_hash)(LOCAL_SeqEach *self, char const *key, Dee_hash_t hash) {
	struct ss_bounditem_string_hash_foreach_data data;
	data.ssbishfd_key  = key;
	data.ssbishfd_hash = hash;
	return seqsome_map_fe2has(LOCAL_seX(foreach)(self, &ss_hasitem_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(bounditem_string_len_hash)(LOCAL_SeqEach *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_fe2bound(LOCAL_seX(foreach)(self, &ss_bounditem_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(hasitem_string_len_hash)(LOCAL_SeqEach *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct ss_bounditem_string_len_hash_foreach_data data;
	data.ssbislhfd_key    = key;
	data.ssbislhfd_keylen = keylen;
	data.ssbislhfd_hash   = hash;
	return seqsome_map_fe2has(LOCAL_seX(foreach)(self, &ss_hasitem_string_len_hash_foreach_cb, &data));
}

PRIVATE struct type_seq LOCAL_ssX(seq) = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssw_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ssw_getrange,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&LOCAL_seX(foreach), /* Needed for recursion! */
	/* .tp_foreach_pair               = */ &default__foreach_pair__with__foreach,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_ssX(bounditem),
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_ssX(hasitem),
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&ssw_size_fast,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&LOCAL_ssX(bounditem_index),
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&LOCAL_ssX(hasitem_index),
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&LOCAL_ssX(bounditem_string_hash),
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&LOCAL_ssX(hasitem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&LOCAL_ssX(bounditem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&LOCAL_ssX(hasitem_string_len_hash),
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(boundattr)(LOCAL_SeqEach *self, DeeObject *attr) {
	return seqsome_map_fe2bound(LOCAL_seX(foreach)(self, &ss_boundattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(hasattr)(LOCAL_SeqEach *self, DeeObject *attr) {
	return seqsome_map_fe2has(LOCAL_seX(foreach)(self, &ss_hasattr_foreach_cb, attr));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(boundattr_string_hash)(LOCAL_SeqEach *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_fe2bound(LOCAL_seX(foreach)(self, &ss_boundattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(hasattr_string_hash)(LOCAL_SeqEach *self, char const *attr, Dee_hash_t hash) {
	struct ss_boundattr_string_hash_foreach_data data;
	data.ssbashfd_attr = attr;
	data.ssbashfd_hash = hash;
	return seqsome_map_fe2has(LOCAL_seX(foreach)(self, &ss_hasattr_string_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(boundattr_string_len_hash)(LOCAL_SeqEach *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_fe2bound(LOCAL_seX(foreach)(self, &ss_boundattr_string_len_hash_foreach_cb, &data));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
LOCAL_ssX(hasattr_string_len_hash)(LOCAL_SeqEach *self, char const *attr, size_t attrlen, Dee_hash_t hash) {
	struct ss_boundattr_string_len_hash_foreach_data data;
	data.ssbaslhfd_attr    = attr;
	data.ssbaslhfd_attrlen = attrlen;
	data.ssbaslhfd_hash    = hash;
	return seqsome_map_fe2has(LOCAL_seX(foreach)(self, &ss_hasattr_string_len_hash_foreach_cb, &data));
}

PRIVATE struct type_attr LOCAL_ssX(attr) = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&ssw_getattr,
	/* .tp_delattr                       = */ NULL,
	/* .tp_setattr                       = */ NULL,
	/* .tp_iterattr                      = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&ssw_iterattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&ssw_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_ssX(hasattr),
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&LOCAL_ssX(boundattr),
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&ssw_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&ssw_callattr_kw,
	/* .tp_vcallattrf                    = */ NULL,
	/* .tp_getattr_string_hash           = */ NULL,
	/* .tp_delattr_string_hash           = */ NULL,
	/* .tp_setattr_string_hash           = */ NULL,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&LOCAL_ssX(hasattr_string_hash),
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&LOCAL_ssX(boundattr_string_hash),
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&ssw_callattr_string_hash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&ssw_callattr_string_hash_kw,
	/* .tp_vcallattr_string_hashf        = */ NULL,
	/* .tp_getattr_string_len_hash       = */ NULL,
	/* .tp_delattr_string_len_hash       = */ NULL,
	/* .tp_setattr_string_len_hash       = */ NULL,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&LOCAL_ssX(hasattr_string_len_hash),
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&LOCAL_ssX(boundattr_string_len_hash),
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&ssw_callattr_string_len_hash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&ssw_callattr_string_len_hash_kw,
	/* .tp_findattr_info_string_len_hash = */ NULL,
};



#ifdef DEFINE_SeqEachGetAttr
PRIVATE struct type_callable LOCAL_ssX(callable) = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&LOCAL_ssX(call_kw),
};
#endif /* DEFINE_SeqEachGetAttr */

INTERN DeeTypeObject LOCAL_SeqSome_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_" LOCAL_SeqSome_Type_NAME,
#ifdef CONFIG_NO_DOC
	/* .tp_doc      = */ NULL,
#else /* CONFIG_NO_DOC */
	/* .tp_doc      = */ "When deemon was compiled with #CCONFIG_NO_SEQEACH_ATTRIBUTE_OPTIMIZATIONS "
	                     /**/ "(default when #C__OPTIMIZE_SIZE__ was enabled), this type won't exist, "
	                     /**/ "and ?Ert:" LOCAL_SeqSome_Type_NAME "_np will evaluate to "
	                     /**/ "?Ert:SeqEachOperator (hence the #C{*_np} suffix in ?Mrt)\n"
	                     "\n"
#ifdef DEFINE_SeqEachGetAttr
	                     "()\n"
	                     "(seq:?DSequence,attr:?Dstring)\n"
	                     "Optimized implementation of ${<seq>.some.<attr>}"
#elif defined(DEFINE_SeqEachCallAttr)
	                     "()\n"
	                     "(seq:?DSequence,attr:?Dstring,args=!T0)\n"
	                     "Optimized implementation of ${<seq>.some.<attr>(<args>...)}"
#elif defined(DEFINE_SeqEachCallAttrKw)
	                     "()\n"
	                     "(seq:?DSequence,attr:?Dstring,args=!T0,kw:?M?Dstring?O=!T0)\n"
	                     "Optimized implementation of ${<seq>.some.<attr>(<args>..., **<kw>)}"
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
	                     "",
#endif /* !CONFIG_NO_DOC */
	/* .tp_flags    = */ LOCAL_SeqEach_Type_FLAGS & ~TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
#if !(LOCAL_SeqEach_Type_FLAGS & TP_FVARIABLE)
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ LOCAL_SeqEach,
			/* tp_ctor:        */ &LOCAL_seX(ctor),
			/* tp_copy_ctor:   */ &LOCAL_seX(copy),
			/* tp_any_ctor:    */ &LOCAL_seX(init),
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &LOCAL_seX(serialize)
		),
#else /* !(LOCAL_SeqEach_Type_FLAGS & TP_FVARIABLE) */
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &LOCAL_seX(ctor),
			/* tp_copy_ctor:   */ &LOCAL_seX(copy),
			/* tp_any_ctor:    */ &LOCAL_seX(init),
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &LOCAL_seX(serialize),
			/* tp_free:        */ NULL
		),
#endif /* LOCAL_SeqEach_Type_FLAGS & TP_FVARIABLE */
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&LOCAL_seX(fini),
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&LOCAL_ssX(bool),
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&LOCAL_ssX(printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&LOCAL_seX(visit),
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &ssw_math,
	/* .tp_cmp           = */ &LOCAL_ssX(cmp),
	/* .tp_seq           = */ &LOCAL_ssX(seq),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssw_iter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &LOCAL_ssX(attr),
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL, /* TODO: Access to the arguments vector */
	/* .tp_members       = */ LOCAL_seX(members),
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ LOCAL_seX(class_members),
	/* .tp_method_hints  = */ NULL,
#ifdef DEFINE_SeqEachGetAttr
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&LOCAL_ssX(call),
	/* .tp_callable      = */ &LOCAL_ssX(callable),
#else /* DEFINE_SeqEachGetAttr */
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&ssw_call,
	/* .tp_callable      = */ &ssw_callable,
#endif /* !DEFINE_SeqEachGetAttr */
};
#endif /* CONFIG_HAVE_SEQSOME_ATTRIBUTE_OPTIMIZATIONS */


DECL_END

#undef LOCAL_SeqEachIterator_Type
#undef LOCAL_SeqEach_Type
#undef LOCAL_SeqSome_Type_NAME
#undef LOCAL_SeqSome_Type
#undef LOCAL_SeqEach
#undef LOCAL_SeqEach_Type_FLAGS
#undef LOCAL_SeqEach_Type_NAME
#undef LOCAL_seXi
#undef LOCAL_seX
#undef LOCAL_ssX

#undef DEFINE_SeqEachCallAttrKw
#undef DEFINE_SeqEachCallAttr
#undef DEFINE_SeqEachGetAttr
