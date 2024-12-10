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
#include "each.c"
//#define DEFINE_GETATTR 1
//#define DEFINE_CALLATTR 1
#define DEFINE_CALLATTRKW 1
#endif /* __INTELLISENSE__ */

#include <deemon/map.h>

#ifdef DEFINE_GETATTR
#define F(x)  sea_##x
#define Fi(x) seai_##x
#define STRUCT_TYPE          SeqEachGetAttr
#define TYPE_OBJECT          SeqEachGetAttr_Type
#define ITERATOR_TYPE_OBJECT SeqEachGetAttrIterator_Type
#elif defined(DEFINE_CALLATTR)
#define F(x)  sec_##x
#define Fi(x) seci_##x
#define STRUCT_TYPE          SeqEachCallAttr
#define TYPE_OBJECT          SeqEachCallAttr_Type
#define ITERATOR_TYPE_OBJECT SeqEachCallAttrIterator_Type
#elif defined(DEFINE_CALLATTRKW)
#define F(x)  sek_##x
#define Fi(x) seki_##x
#define STRUCT_TYPE          SeqEachCallAttrKw
#define TYPE_OBJECT          SeqEachCallAttrKw_Type
#define ITERATOR_TYPE_OBJECT SeqEachCallAttrKwIterator_Type
#else /* ... */
#error "No mode defined"
#endif /* !... */


DECL_BEGIN

#ifdef DEFINE_GETATTR
PRIVATE WUNUSED DREF DeeObject *DCALL
F(getitem_for_inplace)(STRUCT_TYPE *__restrict self,
                       DREF DeeObject **__restrict p_baseelem,
                       size_t index) {
	DREF DeeObject *result, *baseelem;
	baseelem = DeeObject_GetItemIndex(self->se_seq, index);
	if unlikely(!baseelem)
		goto err;
#ifdef DEFINE_GETATTR
	result = DeeObject_GetAttr(baseelem,
	                           (DeeObject *)self->sg_attr);
#else /* DEFINE_GETATTR */
#error "Unsupported mode"
#endif /* !DEFINE_GETATTR */
	if unlikely(!result)
		goto err_baseelem;
	*p_baseelem = baseelem;
	return result;
err_baseelem:
	Dee_Decref(baseelem);
err:
	return NULL;
}

#ifdef DEFINE_GETATTR
#define sea_setitem_for_inplace(self, baseelem, value) \
	DeeObject_SetAttr(baseelem, (DeeObject *)(self)->sg_attr, value)
#else /* DEFINE_GETATTR */
#error "Unsupported mode"
#endif /* !DEFINE_GETATTR */


#define DEFINE_SEA_UNARY_INPLACE(name, func, op)              \
	PRIVATE int DCALL                                         \
	name(STRUCT_TYPE **__restrict p_self) {                   \
		size_t i, size;                                       \
		STRUCT_TYPE *seq = *p_self;                           \
		DREF DeeObject *elem, *baseelem;                      \
		size = DeeObject_Size(seq->se_seq);                   \
		if unlikely(size == (size_t)-1)                       \
			goto err;                                         \
		for (i = 0; i < size; ++i) {                          \
			elem = F(getitem_for_inplace)(seq, &baseelem, i); \
			if unlikely(!elem) {                              \
				if (DeeError_Catch(&DeeError_UnboundItem))    \
					continue;                                 \
				goto err;                                     \
			}                                                 \
			if (func(&elem))                                  \
				goto err_elem;                                \
			if (F(setitem_for_inplace)(seq, baseelem, elem))  \
				goto err_elem;                                \
			Dee_Decref(baseelem);                             \
			Dee_Decref(elem);                                 \
		}                                                     \
		return 0;                                             \
	err_elem:                                                 \
		Dee_Decref(baseelem);                                 \
		Dee_Decref(elem);                                     \
	err:                                                      \
		return -1;                                            \
	}

#define DEFINE_SEA_BINARY_INPLACE(name, func, op)             \
	PRIVATE int DCALL                                         \
	name(STRUCT_TYPE **__restrict p_self, DeeObject *other) { \
		size_t i, size;                                       \
		STRUCT_TYPE *seq = *p_self;                           \
		DREF DeeObject *elem, *baseelem;                      \
		size = DeeObject_Size(seq->se_seq);                   \
		if unlikely(size == (size_t)-1)                       \
			goto err;                                         \
		for (i = 0; i < size; ++i) {                          \
			elem = F(getitem_for_inplace)(seq, &baseelem, i); \
			if unlikely(!elem) {                              \
				if (DeeError_Catch(&DeeError_UnboundItem))    \
					continue;                                 \
				goto err;                                     \
			}                                                 \
			if (func(&elem, other))                           \
				goto err_elem;                                \
			if (F(setitem_for_inplace)(seq, baseelem, elem))  \
				goto err_elem;                                \
			Dee_Decref(baseelem);                             \
			Dee_Decref(elem);                                 \
		}                                                     \
		return 0;                                             \
	err_elem:                                                 \
		Dee_Decref(baseelem);                                 \
		Dee_Decref(elem);                                     \
	err:                                                      \
		return -1;                                            \
	}

DEFINE_SEA_UNARY_INPLACE(F(inc), DeeObject_Inc, OPERATOR_INC)
DEFINE_SEA_UNARY_INPLACE(F(dec), DeeObject_Dec, OPERATOR_DEC)
DEFINE_SEA_BINARY_INPLACE(F(inplace_add), DeeObject_InplaceAdd, OPERATOR_INPLACE_ADD)
DEFINE_SEA_BINARY_INPLACE(F(inplace_sub), DeeObject_InplaceSub, OPERATOR_INPLACE_SUB)
DEFINE_SEA_BINARY_INPLACE(F(inplace_mul), DeeObject_InplaceMul, OPERATOR_INPLACE_MUL)
DEFINE_SEA_BINARY_INPLACE(F(inplace_div), DeeObject_InplaceDiv, OPERATOR_INPLACE_DIV)
DEFINE_SEA_BINARY_INPLACE(F(inplace_mod), DeeObject_InplaceMod, OPERATOR_INPLACE_MOD)
DEFINE_SEA_BINARY_INPLACE(F(inplace_shl), DeeObject_InplaceShl, OPERATOR_INPLACE_SHL)
DEFINE_SEA_BINARY_INPLACE(F(inplace_shr), DeeObject_InplaceShr, OPERATOR_INPLACE_SHR)
DEFINE_SEA_BINARY_INPLACE(F(inplace_and), DeeObject_InplaceAnd, OPERATOR_INPLACE_AND)
DEFINE_SEA_BINARY_INPLACE(F(inplace_or), DeeObject_InplaceOr, OPERATOR_INPLACE_OR)
DEFINE_SEA_BINARY_INPLACE(F(inplace_xor), DeeObject_InplaceXor, OPERATOR_INPLACE_XOR)
DEFINE_SEA_BINARY_INPLACE(F(inplace_pow), DeeObject_InplacePow, OPERATOR_INPLACE_POW)
#undef DEFINE_SEA_UNARY_INPLACE
#undef DEFINE_SEA_BINARY_INPLACE
#endif /* DEFINE_GETATTR */



#ifdef DEFINE_GETATTR
PRIVATE struct type_math F(math) = {
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
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&F(inc),
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&F(dec),
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_add),
	/* .tp_inplace_sub = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_sub),
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_mul),
	/* .tp_inplace_div = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_div),
	/* .tp_inplace_mod = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_mod),
	/* .tp_inplace_shl = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_shl),
	/* .tp_inplace_shr = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_shr),
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_and),
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_or),
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_xor),
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&F(inplace_pow)
};

#else /* DEFINE_GETATTR */

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

#if defined(DEFINE_CALLATTR)
#define sec_math sew_math
#elif defined(DEFINE_CALLATTRKW)
#define sek_math sew_math
#else /* ... */
#error "Unsupported mode"
#endif /* !... */

#endif /* !DEFINE_GETATTR */


PRIVATE WUNUSED NONNULL((1)) DREF SeqEachIterator *DCALL
F(iter)(STRUCT_TYPE *__restrict self) {
	DREF SeqEachIterator *result;
	result = DeeObject_MALLOC(SeqEachIterator);
	if unlikely(!result)
		goto done;
	result->ei_each = (DREF SeqEachBase *)self;
	result->ei_iter = DeeObject_Iter(((DREF SeqEachBase *)self)->se_seq);
	if unlikely(!result->ei_iter)
		goto err_r;
	Dee_Incref(self);
	DeeObject_Init(result, &ITERATOR_TYPE_OBJECT);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
F(transform)(STRUCT_TYPE *self, DeeObject *elem) {
#ifdef DEFINE_GETATTR
	return DeeObject_GetAttr(elem, (DeeObject *)self->sg_attr);
#elif defined(DEFINE_CALLATTR)
	return DeeObject_CallAttr(elem,
	                          (DeeObject *)self->sg_attr,
	                          self->sg_argc,
	                          self->sg_argv);
#elif defined(DEFINE_CALLATTRKW)
	return DeeObject_CallAttrKw(elem,
	                            (DeeObject *)self->sg_attr,
	                            self->sg_argc,
	                            self->sg_argv,
	                            self->sg_kw);
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
}

LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
F(transform_inherit)(STRUCT_TYPE *self,
                     /*inherit(always)*/ DREF DeeObject *elem) {
	DREF DeeObject *result;
	result = F(transform)(self, elem);
	Dee_Decref(elem);
	return result;
}

PRIVATE NONNULL((1, 2)) WUNUSED DREF DeeObject *DCALL
F(getitem)(STRUCT_TYPE *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_GetItem(self->se_seq, index);
	if likely(result)
		result = F(transform_inherit)(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
F(getitem_index)(STRUCT_TYPE *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_GetItemIndex(self->se_seq, index);
	if likely(result)
		result = F(transform_inherit)(self, result);
	return result;
}

PRIVATE NONNULL((1, 2)) WUNUSED DREF DeeObject *DCALL
F(trygetitem)(STRUCT_TYPE *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItem(self->se_seq, index);
	if likely(ITER_ISOK(result))
		result = F(transform_inherit)(self, result);
	return result;
}

PRIVATE NONNULL((1)) WUNUSED DREF DeeObject *DCALL
F(trygetitem_index)(STRUCT_TYPE *self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemIndex(self->se_seq, index);
	if likely(ITER_ISOK(result))
		result = F(transform_inherit)(self, result);
	return result;
}

PRIVATE NONNULL((1, 2)) WUNUSED DREF DeeObject *DCALL
F(getitem_string_hash)(STRUCT_TYPE *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItemStringHash(self->se_seq, key, hash);
	if likely(result)
		result = F(transform_inherit)(self, result);
	return result;
}

PRIVATE NONNULL((1, 2)) WUNUSED DREF DeeObject *DCALL
F(trygetitem_string_hash)(STRUCT_TYPE *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemStringHash(self->se_seq, key, hash);
	if likely(ITER_ISOK(result))
		result = F(transform_inherit)(self, result);
	return result;
}

PRIVATE NONNULL((1, 2)) WUNUSED DREF DeeObject *DCALL
F(getitem_string_len_hash)(STRUCT_TYPE *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItemStringLenHash(self->se_seq, key, keylen, hash);
	if likely(result)
		result = F(transform_inherit)(self, result);
	return result;
}

PRIVATE NONNULL((1, 2)) WUNUSED DREF DeeObject *DCALL
F(trygetitem_string_len_hash)(STRUCT_TYPE *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemStringLenHash(self->se_seq, key, keylen, hash);
	if likely(ITER_ISOK(result))
		result = F(transform_inherit)(self, result);
	return result;
}


struct F(foreach_data) {
	STRUCT_TYPE   *seXfd_me;   /* [1..1] The related seq-each operator */
	Dee_foreach_t  seXfd_proc; /* [1..1] User-defined callback */
	void          *seXfd_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
F(foreach_cb)(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	struct F(foreach_data) *data;
	data = (struct F(foreach_data) *)arg;
	elem = F(transform)(data->seXfd_me, elem);
	if unlikely(!elem)
		goto err;
	result = (*data->seXfd_proc)(data->seXfd_arg, elem);
	Dee_Decref(elem);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
F(foreach)(STRUCT_TYPE *self, Dee_foreach_t proc, void *arg) {
	struct F(foreach_data) data;
	data.seXfd_me   = self;
	data.seXfd_proc = proc;
	data.seXfd_arg  = arg;
	return DeeObject_Foreach(self->se_seq, &F(foreach_cb), &data);
}

struct F(enumerate_data) {
	STRUCT_TYPE     *seXed_me;   /* [1..1] The related seq-each operator */
	Dee_enumerate_t  seXed_proc; /* [1..1] User-defined callback */
	void            *seXed_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
F(enumerate_cb)(void *arg, DeeObject *index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct F(enumerate_data) *data;
	data = (struct F(enumerate_data) *)arg;
	if unlikely(!value)
		return (*data->seXed_proc)(data->seXed_arg, index, value);
	value = F(transform)(data->seXed_me, value);
	if unlikely(!value)
		goto err;
	result = (*data->seXed_proc)(data->seXed_arg, index, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
F(enumerate)(STRUCT_TYPE *__restrict self, Dee_enumerate_t proc, void *arg) {
	struct F(enumerate_data) data;
	data.seXed_me   = self;
	data.seXed_proc = proc;
	data.seXed_arg  = arg;
	return DeeObject_Enumerate(self->se_seq, &F(enumerate_cb), &data);
}

struct F(enumerate_index_data) {
	STRUCT_TYPE          *seXeid_me;   /* [1..1] The related seq-each operator */
	Dee_enumerate_index_t seXeid_proc; /* [1..1] User-defined callback */
	void                 *seXeid_arg;  /* [?..?] User-defined cookie */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
F(enumerate_index_cb)(void *arg, size_t index, /*nullable*/ DeeObject *value) {
	Dee_ssize_t result;
	struct F(enumerate_index_data) *data;
	data = (struct F(enumerate_index_data) *)arg;
	if unlikely(!value)
		return (*data->seXeid_proc)(data->seXeid_arg, index, value);
	value = F(transform)(data->seXeid_me, value);
	if unlikely(!value)
		goto err;
	result = (*data->seXeid_proc)(data->seXeid_arg, index, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
F(enumerate_index)(STRUCT_TYPE *__restrict self, Dee_enumerate_index_t proc,
                   void *arg, size_t start, size_t end) {
	struct F(enumerate_index_data) data;
	data.seXeid_me   = self;
	data.seXeid_proc = proc;
	data.seXeid_arg  = arg;
	return DeeObject_EnumerateIndex(self->se_seq, &F(enumerate_index_cb),
	                                &data, start, end);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(assign)(STRUCT_TYPE *self, DeeObject *value) {
	return (int)F(foreach)(self, &se_foreach_assign_cb, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(moveassign)(STRUCT_TYPE *self, DeeObject *value) {
	return (int)F(foreach)(self, &se_foreach_moveassign_cb, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(delitem)(STRUCT_TYPE *self, DeeObject *index) {
	return (int)F(foreach)(self, &se_foreach_delitem_cb, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(delitem_index)(STRUCT_TYPE *self, size_t index) {
	return (int)F(foreach)(self, &se_foreach_delitem_index_cb, (void *)(uintptr_t)index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(delitem_string_hash)(STRUCT_TYPE *self, char const *key, Dee_hash_t hash) {
	struct se_foreach_delitem_string_hash_data data;
	data.sfedish_key  = key;
	data.sfedish_hash = hash;
	return (int)F(foreach)(self, &se_foreach_delitem_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(delitem_string_len_hash)(STRUCT_TYPE *self, char const *key, size_t keylen, Dee_hash_t hash) {
	struct se_foreach_delitem_string_len_hash_data data;
	data.sfedislh_key    = key;
	data.sfedislh_keylen = keylen;
	data.sfedislh_hash   = hash;
	return (int)F(foreach)(self, &se_foreach_delitem_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
F(setitem)(STRUCT_TYPE *self, DeeObject *index, DeeObject *value) {
	struct se_foreach_setitem_data data;
	data.sfesi_index = index;
	data.sfesi_value = value;
	return (int)F(foreach)(self, &se_foreach_setitem_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(setitem_index)(STRUCT_TYPE *self, size_t index, DeeObject *value) {
	struct se_foreach_setitem_index_data data;
	data.sfesii_index = index;
	data.sfesii_value = value;
	return (int)F(foreach)(self, &se_foreach_setitem_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
F(setitem_string_hash)(STRUCT_TYPE *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setitem_string_hash_data data;
	data.sfesish_key   = key;
	data.sfesish_hash  = hash;
	data.sfesish_value = value;
	return (int)F(foreach)(self, &se_foreach_setitem_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
F(setitem_string_len_hash)(STRUCT_TYPE *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setitem_string_len_hash_data data;
	data.sfesislh_key    = key;
	data.sfesislh_keylen = keylen;
	data.sfesislh_hash   = hash;
	data.sfesislh_value  = value;
	return (int)F(foreach)(self, &se_foreach_setitem_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
F(delrange)(STRUCT_TYPE *self, DeeObject *start, DeeObject *end) {
	struct se_foreach_delrange_data data;
	data.sfedr_start = start;
	data.sfedr_end   = end;
	return (int)F(foreach)(self, &se_foreach_delrange_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(delrange_index)(STRUCT_TYPE *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct se_foreach_delrange_index_data data;
	data.sfedri_start = start;
	data.sfedri_end   = end;
	return (int)F(foreach)(self, &se_foreach_delrange_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(delrange_index_n)(STRUCT_TYPE *self, Dee_ssize_t start) {
	return (int)F(foreach)(self, &se_foreach_delrange_index_n_cb,
	                       (void *)(uintptr_t)(size_t)start);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
F(setrange)(STRUCT_TYPE *self, DeeObject *start,
            DeeObject *end, DeeObject *value) {
	struct se_foreach_setrange_data data;
	data.sfesr_start = start;
	data.sfesr_end   = end;
	data.sfesr_value = value;
	return (int)F(foreach)(self, &se_foreach_setrange_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
F(setrange_index)(STRUCT_TYPE *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value) {
	struct se_foreach_setrange_index_data data;
	data.sfesri_start = start;
	data.sfesri_end   = end;
	data.sfesri_value = value;
	return (int)F(foreach)(self, &se_foreach_setrange_index_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(setrange_index_n)(STRUCT_TYPE *self, Dee_ssize_t start, DeeObject *value) {
	struct se_foreach_setrange_index_n_data data;
	data.sfesrin_start = start;
	data.sfesrin_value = value;
	return (int)F(foreach)(self, &se_foreach_setrange_index_n_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(delattr)(STRUCT_TYPE *self, DeeObject *attr) {
	return (int)F(foreach)(self, &se_foreach_delattr_cb, attr);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(delattr_string_hash)(STRUCT_TYPE *self, char const *attr, Dee_hash_t hash) {
	struct se_foreach_delattr_string_hash_data data;
	data.sfedsh_attr = attr;
	data.sfedsh_hash = hash;
	return (int)F(foreach)(self, &se_foreach_delattr_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(delattr_string_len_hash)(STRUCT_TYPE *self, char const *attr,
                           size_t attrlen, Dee_hash_t hash) {
	struct se_foreach_delattr_string_len_hash_data data;
	data.sfedslh_attr    = attr;
	data.sfedslh_attrlen = attrlen;
	data.sfedslh_hash    = hash;
	return (int)F(foreach)(self, &se_foreach_delattr_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
F(setattr)(STRUCT_TYPE *self, DeeObject *attr, DeeObject *value) {
	struct se_foreach_setattr_data data;
	data.sfes_attr  = attr;
	data.sfes_value = value;
	return (int)F(foreach)(self, &se_foreach_setattr_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
F(setattr_string_hash)(STRUCT_TYPE *self, char const *attr,
                       Dee_hash_t hash, DeeObject *value) {
	struct se_foreach_setattr_string_hash_data data;
	data.sfessh_attr  = attr;
	data.sfessh_hash  = hash;
	data.sfessh_value = value;
	return (int)F(foreach)(self, &se_foreach_setattr_string_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
F(setattr_string_len_hash)(STRUCT_TYPE *self, char const *attr,
                           size_t attrlen, Dee_hash_t hash,
                           DeeObject *value) {
	struct se_foreach_setattr_string_len_hash_data data;
	data.sfesslh_attr    = attr;
	data.sfesslh_attrlen = attrlen;
	data.sfesslh_hash    = hash;
	data.sfesslh_value   = value;
	return (int)F(foreach)(self, &se_foreach_setattr_string_len_hash_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(hasattr)(STRUCT_TYPE *self, DeeObject *attr) {
	Dee_ssize_t status = F(foreach)(self, &se_foreach_hasattr_cb, attr);
	ASSERT(status <= 0);
	if (status == 0)
		return 1; /* All elements have the attribute */
	if (status == -2)
		return 0; /* Attribute doesn't exist for some element */
	ASSERT(status == -1);
	return (int)status; /* Error (-1) */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(hasattr_string_hash)(STRUCT_TYPE *self,
                       char const *attr, dhash_t hash) {
	Dee_ssize_t status;
	struct se_hasattr_string_hash_data data;
	data.shashd_attr = attr;
	data.shashd_hash = hash;
	status           = F(foreach)(self, &se_foreach_hasattr_string_hash_cb, &data);
	ASSERT(status <= 0);
	if (status == 0)
		return 1; /* All elements have the attribute */
	if (status == -2)
		return 0; /* Attribute doesn't exist for some element */
	ASSERT(status == -1);
	return (int)status; /* Error (-1) */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(hasattr_string_len_hash)(STRUCT_TYPE *self,
                           char const *attr,
                           size_t attrlen, dhash_t hash) {
	Dee_ssize_t status;
	struct se_hasattr_string_len_hash_data data;
	data.shaslhd_attr    = attr;
	data.shaslhd_attrlen = attrlen;
	data.shaslhd_hash    = hash;
	status               = F(foreach)(self, &se_foreach_hasattr_string_len_hash_cb, &data);
	ASSERT(status <= 0);
	if (status == 0)
		return 1; /* All elements have the attribute */
	if (status == -2)
		return 0; /* Attribute doesn't exist for some element */
	ASSERT(status == -1);
	return (int)status; /* Error (-1) */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(boundattr)(STRUCT_TYPE *self, DeeObject *attr) {
	Dee_ssize_t status = F(foreach)(self, &se_foreach_boundattr_cb, attr);
	ASSERT(status <= 0);
	if (status == 0)
		return 1; /* All elements have the attribute */
	if (status == -4)
		return 0; /* Attribute isn't bound for some element */
	if (status == -3)
		status = -2; /* A user-defined getattr operator threw an error indicating that the attribute doesn't exists. */
	ASSERT(status == -1 || status == -2);
	return (int)status; /* Error (-1), or attribute doesn't exist (-2) */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(boundattr_string_hash)(STRUCT_TYPE *self,
                         char const *attr, dhash_t hash) {
	Dee_ssize_t status;
	struct se_hasattr_string_hash_data data;
	data.shashd_attr = attr;
	data.shashd_hash = hash;
	status           = F(foreach)(self, &se_foreach_boundattr_string_hash_cb, &data);
	ASSERT(status <= 0);
	if (status == 0)
		return 1; /* All elements have the attribute */
	if (status == -4)
		return 0; /* Attribute isn't bound for some element */
	if (status == -3)
		status = -2; /* A user-defined getattr operator threw an error indicating that the attribute doesn't exists. */
	ASSERT(status == -1 || status == -2);
	return (int)status; /* Error (-1), or attribute doesn't exist (-2) */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
F(boundattr_string_len_hash)(STRUCT_TYPE *self,
                             char const *attr,
                             size_t attrlen, dhash_t hash) {
	Dee_ssize_t status;
	struct se_hasattr_string_len_hash_data data;
	data.shaslhd_attr    = attr;
	data.shaslhd_attrlen = attrlen;
	data.shaslhd_hash    = hash;
	status = F(foreach)(self, &se_foreach_boundattr_string_len_hash_cb, &data);
	ASSERT(status <= 0);
	if (status == 0)
		return 1; /* All elements have the attribute */
	if (status == -4)
		return 0; /* Attribute isn't bound for some element */
	if (status == -3)
		status = -2; /* A user-defined getattr operator threw an error indicating that the attribute doesn't exists. */
	ASSERT(status == -1 || status == -2);
	return (int)status; /* Error (-1), or attribute doesn't exist (-2) */
}

PRIVATE struct type_seq F(seq) = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&F(iter),
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_sizeob,
	/* .tp_contains                   = */ &DeeSeq_DefaultContainsWithForeachDefault,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&F(getitem),
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&F(delitem),
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&F(setitem),
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&sew_getrange,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&F(delrange),
	/* .tp_setrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&F(setrange),
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&F(foreach),
	/* .tp_foreach_pair               = */ NULL, /* &DeeObject_DefaultForeachPairWithForeachs */
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_t, void *))&F(enumerate),
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&F(enumerate_index),
	/* .tp_iterkeys                   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_iterkeys,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&sew_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&sew_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&sew_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&sew_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&F(getitem_index),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&F(delitem_index),
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&F(setitem_index),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&sew_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&sew_hasitem_index,
	/* .tp_getrange_index             = */ NULL, /* &sew_getrange_index */
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&F(delrange_index),
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&F(setrange_index),
	/* .tp_getrange_index_n           = */ NULL, /* &sew_getrange_index_n */
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&F(delrange_index_n),
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&F(setrange_index_n),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&F(trygetitem),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&F(trygetitem_index),
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&F(trygetitem_string_hash),
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&F(getitem_string_hash),
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&F(delitem_string_hash),
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&F(setitem_string_hash),
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&sew_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&sew_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&F(trygetitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&F(getitem_string_len_hash),
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&F(delitem_string_len_hash),
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&F(setitem_string_len_hash),
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&sew_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&sew_hasitem_string_len_hash,
};

PRIVATE struct type_member tpconst F(members)[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(STRUCT_TYPE, se_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__attr__", STRUCT_OBJECT, offsetof(STRUCT_TYPE, sg_attr), "->?Dstring"),
#ifdef DEFINE_CALLATTRKW
	TYPE_MEMBER_FIELD_DOC("__kw__", STRUCT_OBJECT, offsetof(STRUCT_TYPE, sg_kw), "->?M?Dstring?O"),
#endif /* DEFINE_CALLATTRKW */
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst F(class_members)[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ITERATOR_TYPE_OBJECT),
	TYPE_MEMBER_END
};

PRIVATE NONNULL((1)) void DCALL
F(fini)(STRUCT_TYPE *__restrict self) {
	Dee_Decref(self->se_seq);
	Dee_Decref(self->sg_attr);
#ifdef DEFINE_CALLATTRKW
	Dee_Decref(self->sg_kw);
#endif /* DEFINE_CALLATTRKW */
#if defined(DEFINE_CALLATTR) || defined(DEFINE_CALLATTRKW)
	Dee_Decrefv(self->sg_argv, self->sg_argc);
#endif /* DEFINE_CALLATTR || DEFINE_CALLATTRKW */
}

PRIVATE NONNULL((1, 2)) void DCALL
F(visit)(STRUCT_TYPE *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->se_seq);
	Dee_Visit(self->sg_attr);
#ifdef DEFINE_CALLATTRKW
	Dee_Visit(self->sg_kw);
#endif /* DEFINE_CALLATTRKW */
#if defined(DEFINE_CALLATTR) || defined(DEFINE_CALLATTRKW)
	Dee_Visitv(self->sg_argv, self->sg_argc);
#endif /* DEFINE_CALLATTR || DEFINE_CALLATTRKW */
}

#ifdef CONFIG_HAVE_SEQEACH_OPERATOR_REPR
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
F(printrepr)(STRUCT_TYPE *__restrict self,
             dformatprinter printer, void *arg) {
	char const *each_suffix = ".each";
	DeeTypeObject *seq_type = Dee_TYPE(self->se_seq);
	if (DeeType_IsSeqEachWrapper(seq_type))
		each_suffix = "";
#ifdef DEFINE_GETATTR
	return DeeFormat_Printf(printer, arg, "%r%s.%k",
	                        self->se_seq, each_suffix,
	                        self->sg_attr);
#elif defined(DEFINE_CALLATTR) || defined(DEFINE_CALLATTRKW)
	{
#ifdef DEFINE_CALLATTRKW
		DeeObject *argv[2];
#else /* DEFINE_CALLATTRKW */
		DeeObject *argv[1];
#endif /* !DEFINE_CALLATTRKW */
		Dee_ssize_t result = -1;
		size_t full_suffix_len;
		char *full_suffix, *attr_utf8, *p;
		attr_utf8 = DeeString_AsUtf8((DeeObject *)self->sg_attr);
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
#ifdef DEFINE_CALLATTRKW
			argv[1] = self->sg_kw;
#endif /* !DEFINE_CALLATTRKW */
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
#endif /* CONFIG_HAVE_SEQEACH_OPERATOR_REPR */


#ifdef DEFINE_GETATTR
PRIVATE int DCALL
F(ctor)(STRUCT_TYPE *__restrict self) {
	self->se_seq  = Dee_EmptySeq;
	self->sg_attr = (DREF DeeStringObject *)Dee_EmptyString;
	Dee_Incref(Dee_EmptySeq);
	Dee_Incref(Dee_EmptyString);
	return 0;
}

PRIVATE int DCALL
F(copy)(STRUCT_TYPE *__restrict self,
        STRUCT_TYPE *__restrict other) {
	self->se_seq  = other->se_seq;
	self->sg_attr = other->sg_attr;
	Dee_Incref(self->se_seq);
	Dee_Incref(self->sg_attr);
	return 0;
}

PRIVATE int DCALL
F(deep)(STRUCT_TYPE *__restrict self,
        STRUCT_TYPE *__restrict other) {
	self->se_seq = DeeObject_DeepCopy(other->se_seq);
	if unlikely(!self->se_seq)
		goto err;
	self->sg_attr = other->sg_attr;
	Dee_Incref(self->sg_attr);
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
F(init)(STRUCT_TYPE *__restrict self,
        size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oo:_SeqEachGetAttr",
	                  &self->se_seq,
	                  &self->sg_attr))
		goto err;
	if (DeeObject_AssertTypeExact(self->sg_attr, &DeeString_Type))
		goto err;
	Dee_Incref(self->se_seq);
	Dee_Incref(self->sg_attr);
	return 0;
err:
	return -1;
}

#elif defined(DEFINE_CALLATTR) || defined(DEFINE_CALLATTRKW)

PRIVATE WUNUSED DREF STRUCT_TYPE *DCALL F(ctor)(void) {
	DREF STRUCT_TYPE *result;
	result = (DREF STRUCT_TYPE *)DeeObject_Malloc(offsetof(STRUCT_TYPE, sg_argv));
	if unlikely(!result)
		goto done;
	result->se_seq  = Dee_EmptySeq;
	result->sg_attr = (DREF DeeStringObject *)Dee_EmptyString;
	result->sg_argc = 0;
#ifdef DEFINE_CALLATTRKW
	result->sg_kw   = Dee_EmptyMapping;
	Dee_Incref(Dee_EmptyMapping);
#endif /* DEFINE_CALLATTRKW */
	Dee_Incref(Dee_EmptySeq);
	Dee_Incref(Dee_EmptyString);
	DeeObject_Init(result, &TYPE_OBJECT);
done:
	return result;
}

PRIVATE WUNUSED DREF STRUCT_TYPE *DCALL
F(copy)(STRUCT_TYPE *__restrict other) {
	DREF STRUCT_TYPE *result;
	result = (DREF STRUCT_TYPE *)DeeObject_Mallocc(offsetof(STRUCT_TYPE, sg_argv),
	                                               other->sg_argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->se_seq  = other->se_seq;
	result->sg_attr = other->sg_attr;
	result->sg_argc = other->sg_argc;
	Dee_Movrefv(result->sg_argv, other->sg_argv, result->sg_argc);
#ifdef DEFINE_CALLATTRKW
	result->sg_kw = other->sg_kw;
	Dee_Incref(result->sg_kw);
#endif /* DEFINE_CALLATTRKW */
	Dee_Incref(result->se_seq);
	Dee_Incref(result->sg_attr);
	DeeObject_Init(result, &TYPE_OBJECT);
done:
	return result;
}

PRIVATE WUNUSED DREF STRUCT_TYPE *DCALL
F(deep)(STRUCT_TYPE *__restrict other) {
	DREF STRUCT_TYPE *result;
	size_t i;
	result = (DREF STRUCT_TYPE *)DeeObject_Mallocc(offsetof(STRUCT_TYPE, sg_argv),
	                                               other->sg_argc, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto done;
	result->se_seq = DeeObject_DeepCopy(other->se_seq);
	if unlikely(!result->se_seq)
		goto err_r;
#ifdef DEFINE_CALLATTRKW
	result->sg_kw = DeeObject_DeepCopy(other->sg_kw);
	if unlikely(!result->sg_kw)
		goto err_r_seq;
#endif /* DEFINE_CALLATTRKW */
	result->sg_argc = other->sg_argc;
	for (i = 0; i < result->sg_argc; ++i) {
		result->sg_argv[i] = DeeObject_DeepCopy(other->sg_argv[i]);
		if unlikely(!result->sg_argv[i])
			goto err_r_argv;
	}
	result->sg_attr = other->sg_attr;
	Dee_Incref(result->sg_attr);
	DeeObject_Init(result, &TYPE_OBJECT);
done:
	return result;
err_r_argv:
	Dee_Decrefv(result->sg_argv, i);
/*err_r_kw:*/
#ifdef DEFINE_CALLATTRKW
	Dee_Decref(result->sg_kw);
err_r_seq:
#endif /* DEFINE_CALLATTRKW */
	Dee_Decref(result->se_seq);
err_r:
	DeeObject_Free(result);
	return NULL;
}

PRIVATE WUNUSED DREF STRUCT_TYPE *DCALL
F(init)(size_t argc, DeeObject *const *argv) {
	DREF STRUCT_TYPE *result;
	DeeStringObject *attr;
	DeeObject *seq;
	DeeObject *args = Dee_EmptyTuple;
#ifdef DEFINE_CALLATTR
	if (DeeArg_Unpack(argc, argv, "oo|o:_SeqEachCallAttr", &seq, &attr, &args))
		goto err;
#elif defined(DEFINE_CALLATTRKW)
	DeeObject *kw = Dee_EmptyMapping;
	if (DeeArg_Unpack(argc, argv, "oo|oo:_SeqEachCallAttrKw", &seq, &attr, &args, &kw))
		goto err;
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	result = (DREF STRUCT_TYPE *)DeeObject_Mallocc(offsetof(STRUCT_TYPE, sg_argv),
	                                               DeeTuple_SIZE(args), sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto err;
	result->sg_argc = DeeTuple_SIZE(args);
	Dee_Movrefv(result->sg_argv, DeeTuple_ELEM(args), DeeTuple_SIZE(args));
#ifdef DEFINE_CALLATTRKW
	result->sg_kw = kw;
	Dee_Incref(kw);
#endif /* DEFINE_CALLATTRKW */
	result->se_seq  = seq;
	result->sg_attr = attr;
	Dee_Incref(seq);
	Dee_Incref(attr);
	DeeObject_Init(result, &TYPE_OBJECT);
	return result;
err:
	return NULL;
}

#else /* ... */
#error "Unsupported mode"
#endif /* !... */

#ifdef DEFINE_GETATTR
PRIVATE WUNUSED DREF SeqEachCallAttr *DCALL
F(call)(STRUCT_TYPE *__restrict self,
        size_t argc, DeeObject *const *argv) {
	return (DREF SeqEachCallAttr *)DeeSeqEach_CallAttr(self->se_seq,
	                                                   (DeeObject *)self->sg_attr,
	                                                   argc, argv);
}

PRIVATE WUNUSED DREF SeqEachCallAttrKw *DCALL
F(call_kw)(STRUCT_TYPE *__restrict self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	return (DREF SeqEachCallAttrKw *)DeeSeqEach_CallAttrKw(self->se_seq,
	                                                       (DeeObject *)self->sg_attr,
	                                                       argc, argv, kw);
}
#endif /* DEFINE_GETATTR */

PRIVATE struct type_attr F(attr) = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&sew_getattr,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&F(delattr),
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&F(setattr),
	/* .tp_enumattr                      = */ (Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&sew_enumattr,
	/* .tp_findattr                      = */ NULL,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&F(hasattr),
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&F(boundattr),
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&sew_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&sew_callattr_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr                      = */ NULL,
	/* .tp_callattr_kw                   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattrf                    = */ NULL,
	/* .tp_getattr_string_hash           = */ NULL,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&F(delattr_string_hash),
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&F(setattr_string_hash),
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&F(hasattr_string_hash),
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&F(boundattr_string_hash),
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&sew_callattr_string_hash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&sew_callattr_string_hash_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_hash          = */ NULL,
	/* .tp_callattr_string_hash_kw       = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_vcallattr_string_hashf        = */ NULL,
	/* .tp_getattr_string_len_hash       = */ NULL,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&F(delattr_string_len_hash),
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&F(setattr_string_len_hash),
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&F(hasattr_string_len_hash),
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&F(boundattr_string_len_hash),
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&sew_callattr_string_len_hash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&sew_callattr_string_len_hash_kw,
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_callattr_string_len_hash      = */ NULL,
	/* .tp_callattr_string_len_hash_kw   = */ NULL,
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
	/* .tp_findattr_info_string_len_hash = */ NULL,
};


INTERN DeeTypeObject TYPE_OBJECT = {
	OBJECT_HEAD_INIT(&DeeType_Type),
#ifdef DEFINE_GETATTR
	/* .tp_name     = */ "_SeqEachGetAttr",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,attr:?Dstring)"),
#define TYPE_FLAGS (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY)
	/* .tp_flags    = */ TYPE_FLAGS,
#elif defined(DEFINE_CALLATTR)
	/* .tp_name     = */ "_SeqEachCallAttr",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,attr:?Dstring,args=!T0)"),
#define TYPE_FLAGS (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY | TP_FVARIABLE)
	/* .tp_flags    = */ TYPE_FLAGS,
#elif defined(DEFINE_CALLATTRKW)
	/* .tp_name     = */ "_SeqEachCallAttrKw",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,attr:?Dstring,args=!T0,kw:?M?Dstring?O=!T0)"),
#define TYPE_FLAGS (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY | TP_FVARIABLE)
	/* .tp_flags    = */ TYPE_FLAGS,
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* Not a sequence type! (can't have stuff like "find()", etc.) */
	/* .tp_init = */ {
		{
#if !(TYPE_FLAGS & TP_FVARIABLE)
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&F(ctor),
				/* .tp_copy_ctor = */ (dfunptr_t)&F(copy),
				/* .tp_deep_ctor = */ (dfunptr_t)&F(deep),
				/* .tp_any_ctor  = */ (dfunptr_t)&F(init),
				TYPE_FIXED_ALLOCATOR(STRUCT_TYPE)
			}
#else /* !(TYPE_FLAGS & TP_FVARIABLE) */
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&F(ctor),
				/* .tp_copy_ctor = */ (dfunptr_t)&F(copy),
				/* .tp_deep_ctor = */ (dfunptr_t)&F(deep),
				/* .tp_any_ctor  = */ (dfunptr_t)&F(init),
				/* .tp_free      = */ (dfunptr_t)NULL
			}
#endif /* TYPE_FLAGS & TP_FVARIABLE */
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&F(fini),
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&F(assign),
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&F(moveassign),
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&sew_bool,
		/* .tp_print     = */ NULL,
#ifdef CONFIG_HAVE_SEQEACH_OPERATOR_REPR
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&F(printrepr),
#else /* CONFIG_HAVE_SEQEACH_OPERATOR_REPR */
		/* .tp_printrepr = */ &default_seq_printrepr,
#endif /* !CONFIG_HAVE_SEQEACH_OPERATOR_REPR */
	},
#ifdef DEFINE_GETATTR
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&F(call),
#else /* DEFINE_GETATTR */
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sew_call,
#endif /* !DEFINE_GETATTR */
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&F(visit),
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &F(math),
	/* .tp_cmp           = */ &sew_cmp,
	/* .tp_seq           = */ &F(seq),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_iter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &F(attr),
	/* .tp_with          = */ &sew_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL, /* TODO: Access to the arguments vector */
	/* .tp_members       = */ F(members),
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ F(class_members),
	/* .tp_method_hints  = */ NULL,
#ifdef DEFINE_GETATTR
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&F(call_kw),
#else /* DEFINE_GETATTR */
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&sew_call_kw,
#endif /* !DEFINE_GETATTR */
};

#undef TYPE_FLAGS




PRIVATE WUNUSED NONNULL((1)) int DCALL
Fi(ctor)(SeqEachIterator *__restrict self) {
	self->ei_each = (DREF SeqEachBase *)DeeObject_NewDefault(&TYPE_OBJECT);
	if unlikely(!self->ei_each)
		goto err;
	self->ei_iter = DeeObject_Iter(self->ei_each->se_seq);
	if unlikely(!self->ei_iter)
		goto err_each;
	return 0;
err_each:
	Dee_Decref(self->ei_each);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
Fi(init)(SeqEachIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
#ifdef DEFINE_GETATTR
	if (DeeArg_Unpack(argc, argv, "o:_SeqEachGetAttrIterator", &self->ei_each))
		goto err;
#elif defined(DEFINE_CALLATTR)
	if (DeeArg_Unpack(argc, argv, "o:_SeqEachCallAttrIterator", &self->ei_each))
		goto err;
#elif defined(DEFINE_CALLATTRKW)
	if (DeeArg_Unpack(argc, argv, "o:_SeqEachCallAttrKwIterator", &self->ei_each))
		goto err;
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
	if (DeeObject_AssertTypeExact(self->ei_each, &TYPE_OBJECT))
		goto err;
	self->ei_iter = DeeObject_Iter(self->ei_each->se_seq);
	if unlikely(!self->ei_iter)
		goto err;
	Dee_Incref(self->ei_each);
	return 0;
err:
	return -1;
}

#ifndef CONFIG_NO_DOC
PRIVATE struct type_member tpconst Fi(members)[] = {
#ifdef DEFINE_GETATTR
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SeqEachIterator, ei_each), "->?Ert:SeqEachGetAttr"),
#elif defined(DEFINE_CALLATTR)
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SeqEachIterator, ei_each), "->?Ert:SeqEachCallAttr"),
#elif defined(DEFINE_CALLATTRKW)
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SeqEachIterator, ei_each), "->?Ert:SeqEachCallAttrKw"),
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SeqEachIterator, ei_iter), "->?DIterator"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */

PRIVATE WUNUSED DREF DeeObject *DCALL
Fi(next)(SeqEachIterator *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_IterNext(self->ei_iter);
	if likely(ITER_ISOK(result))
		result = F(transform_inherit)((STRUCT_TYPE *)self->ei_each, result);
	return result;
}



INTERN DeeTypeObject ITERATOR_TYPE_OBJECT = {
	OBJECT_HEAD_INIT(&DeeType_Type),
#ifdef DEFINE_GETATTR
	/* .tp_name     = */ "_SeqEachGetAttrIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqEachGetAttr)"),
#elif defined(DEFINE_CALLATTR)
	/* .tp_name     = */ "_SeqEachCallAttrIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqEachCallAttr)"),
#elif defined(DEFINE_CALLATTRKW)
	/* .tp_name     = */ "_SeqEachCallAttrKwIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqEachCallAttrKw)"),
#else /* ... */
#error "Unsupported mode"
#endif /* !... */
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&Fi(ctor),
				/* .tp_copy_ctor = */ (dfunptr_t)&sewi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&sewi_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&Fi(init),
				TYPE_FIXED_ALLOCATOR(SeqEachIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sewi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sewi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sewi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sewi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&Fi(next),
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
#ifndef CONFIG_NO_DOC
	/* .tp_members       = */ Fi(members),
#else /* !CONFIG_NO_DOC */
	/* .tp_members       = */ seoi_members,
#endif /* CONFIG_NO_DOC */
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#undef ITERATOR_TYPE_OBJECT
#undef TYPE_OBJECT
#undef STRUCT_TYPE
#undef Fi
#undef F
#undef DEFINE_CALLATTRKW
#undef DEFINE_CALLATTR
#undef DEFINE_GETATTR
