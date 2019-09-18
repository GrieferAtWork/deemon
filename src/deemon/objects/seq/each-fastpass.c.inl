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
#ifdef __INTELLISENSE__
#include "each.c"
#define DEFINE_GETATTR 1
//#define DEFINE_CALLATTR 1
//#define DEFINE_CALLATTRKW 1
#endif

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
#else
#error "No mode defined"
#endif


DECL_BEGIN

#ifdef DEFINE_GETATTR
PRIVATE DREF DeeObject *DCALL
F(getitem_for_inplace)(STRUCT_TYPE *__restrict self,
                       DREF DeeObject **__restrict pbaseelem,
                       size_t index) {
	DREF DeeObject *result, *baseelem;
	baseelem = DeeObject_GetItemIndex(self->se_seq, index);
	if
		unlikely(!baseelem)
	goto err;
#ifdef DEFINE_GETATTR
	result = DeeObject_GetAttr(baseelem,
	                           (DeeObject *)self->sg_attr);
#else /* DEFINE_GETATTR */
#error "Unsupported mode"
#endif /* !DEFINE_GETATTR */
	if
		unlikely(!result)
	goto err_baseelem;
	*pbaseelem = baseelem;
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
	name(STRUCT_TYPE **__restrict pself) {                    \
		size_t i, size;                                       \
		STRUCT_TYPE *seq = *pself;                            \
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

#define DEFINE_SEA_BINARY_INPLACE(name, func, op)                       \
	PRIVATE int DCALL                                                   \
	name(STRUCT_TYPE **__restrict pself, DeeObject *__restrict other) { \
		size_t i, size;                                                 \
		STRUCT_TYPE *seq = *pself;                                      \
		DREF DeeObject *elem, *baseelem;                                \
		size = DeeObject_Size(seq->se_seq);                             \
		if unlikely(size == (size_t)-1)                                 \
			goto err;                                                   \
		for (i = 0; i < size; ++i) {                                    \
			elem = F(getitem_for_inplace)(seq, &baseelem, i);           \
			if unlikely(!elem) {                                        \
				if (DeeError_Catch(&DeeError_UnboundItem))              \
					continue;                                           \
				goto err;                                               \
			}                                                           \
			if (func(&elem, other))                                     \
				goto err_elem;                                          \
			if (F(setitem_for_inplace)(seq, baseelem, elem))            \
				goto err_elem;                                          \
			Dee_Decref(baseelem);                                       \
			Dee_Decref(elem);                                           \
		}                                                               \
		return 0;                                                       \
	err_elem:                                                           \
		Dee_Decref(baseelem);                                           \
		Dee_Decref(elem);                                               \
	err:                                                                \
		return -1;                                                      \
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
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_pow,
	/* .tp_inc         = */ (int(DCALL *)(DeeObject **__restrict))&F(inc),
	/* .tp_dec         = */ (int(DCALL *)(DeeObject **__restrict))&F(dec),
	/* .tp_inplace_add = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_add),
	/* .tp_inplace_sub = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_sub),
	/* .tp_inplace_mul = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_mul),
	/* .tp_inplace_div = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_div),
	/* .tp_inplace_mod = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_mod),
	/* .tp_inplace_shl = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_shl),
	/* .tp_inplace_shr = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_shr),
	/* .tp_inplace_and = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_and),
	/* .tp_inplace_or  = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_or),
	/* .tp_inplace_xor = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_xor),
	/* .tp_inplace_pow = */ (int(DCALL *)(DeeObject **__restrict, DeeObject *__restrict))&F(inplace_pow)
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
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_pow
};
#endif /* !SEW_MATH_DEFINED */

#if defined(DEFINE_CALLATTR)
#define sec_math sew_math
#elif defined(DEFINE_CALLATTRKW)
#define sek_math sew_math
#else
#error "Unsupported mode"
#endif

#endif /* !DEFINE_GETATTR */


PRIVATE DREF SeqEachIterator *DCALL
F(iter)(STRUCT_TYPE *__restrict self) {
	DREF SeqEachIterator *result;
	result = DeeObject_MALLOC(SeqEachIterator);
	if
		unlikely(!result)
	goto done;
	result->ei_each = (DREF SeqEachBase *)self;
	result->ei_iter = DeeObject_IterSelf(((DREF SeqEachBase *)self)->se_seq);
	if
		unlikely(!result->ei_iter)
	goto err_r;
	Dee_Incref(self);
	DeeObject_Init(result, &ITERATOR_TYPE_OBJECT);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

LOCAL DREF DeeObject *DCALL
F(transform)(STRUCT_TYPE *__restrict self,
             /*inherit(always)*/ DREF DeeObject *__restrict elem) {
	DREF DeeObject *result;
#ifdef DEFINE_GETATTR
	result = DeeObject_GetAttr(elem, (DeeObject *)self->sg_attr);
#elif defined(DEFINE_CALLATTR)
	result = DeeObject_CallAttr(elem,
	                            (DeeObject *)self->sg_attr,
	                            self->sg_argc,
	                            self->sg_argv);
#elif defined(DEFINE_CALLATTRKW)
	result = DeeObject_CallAttrKw(elem,
	                              (DeeObject *)self->sg_attr,
	                              self->sg_argc,
	                              self->sg_argv,
	                              self->sg_kw);
#else
#error "Unsupported mode"
#endif
	Dee_Decref(elem);
	return result;
}

PRIVATE DREF DeeObject *DCALL
F(nsi_getitem)(STRUCT_TYPE *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_GetItemIndex(self->se_seq, index);
	if
		likely(result)
	result = F(transform)(self, result);
	return result;
}

PRIVATE DREF DeeObject *DCALL
F(getitem)(STRUCT_TYPE *__restrict self,
           DeeObject *__restrict index) {
	DREF DeeObject *result;
	result = DeeObject_GetItem(self->se_seq, index);
	if
		likely(result)
	result = F(transform)(self, result);
	return result;
}


PRIVATE struct type_nsi F(nsi) = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&sew_nsi_size,
			/* .nsi_getsize_fast = */ (void *)&sew_nsi_fastsize,
			/* .nsi_getitem      = */ (void *)&F(nsi_getitem),
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)NULL,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)NULL,
			/* .nsi_rfind        = */ (void *)NULL,
			/* .nsi_xch          = */ (void *)NULL,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL,
			/* .nsi_removeif     = */ (void *)NULL
		}
	}
};

PRIVATE struct type_seq F(seq) = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&F(iter),
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&F(getitem),
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &F(nsi)
};

PRIVATE struct type_member F(members)[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(STRUCT_TYPE, se_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__attr__", STRUCT_OBJECT, offsetof(STRUCT_TYPE, sg_attr), "->?Dstring"),
#ifdef DEFINE_CALLATTRKW
	TYPE_MEMBER_FIELD_DOC("__kw__", STRUCT_OBJECT, offsetof(STRUCT_TYPE, sg_kw), "->?S?T2?Dstring?O"),
#endif /* DEFINE_CALLATTRKW */
	TYPE_MEMBER_END
};

PRIVATE struct type_member F(class_members)[] = {
	TYPE_MEMBER_CONST("Iterator", &ITERATOR_TYPE_OBJECT),
	TYPE_MEMBER_END
};

PRIVATE void DCALL
F(fini)(STRUCT_TYPE *__restrict self) {
	Dee_Decref(self->se_seq);
	Dee_Decref(self->sg_attr);
#ifdef DEFINE_CALLATTRKW
	Dee_Decref(self->sg_kw);
#endif /* DEFINE_CALLATTRKW */
#if defined(DEFINE_CALLATTR) || defined(DEFINE_CALLATTRKW)
	{
		size_t i;
		for (i = 0; i < self->sg_argc; ++i)
			Dee_Decref(self->sg_argv[i]);
	}
#endif /* DEFINE_CALLATTR || DEFINE_CALLATTRKW */
}

PRIVATE void DCALL
F(visit)(STRUCT_TYPE *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->se_seq);
	Dee_Visit(self->sg_attr);
#ifdef DEFINE_CALLATTRKW
	Dee_Visit(self->sg_kw);
#endif /* DEFINE_CALLATTRKW */
#if defined(DEFINE_CALLATTR) || defined(DEFINE_CALLATTRKW)
	{
		size_t i;
		for (i = 0; i < self->sg_argc; ++i)
			Dee_Visit(self->sg_argv[i]);
	}
#endif /* DEFINE_CALLATTR || DEFINE_CALLATTRKW */
}


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
	if
		unlikely(!self->se_seq)
	goto err;
	self->sg_attr = other->sg_attr;
	Dee_Incref(self->sg_attr);
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
F(init)(STRUCT_TYPE *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
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

PRIVATE DREF STRUCT_TYPE *DCALL F(ctor)(void) {
	DREF STRUCT_TYPE *result;
	result = (DREF STRUCT_TYPE *)DeeObject_Malloc(COMPILER_OFFSETOF(STRUCT_TYPE, sg_argv));
	if
		unlikely(!result)
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

PRIVATE DREF STRUCT_TYPE *DCALL
F(copy)(STRUCT_TYPE *__restrict other) {
	DREF STRUCT_TYPE *result;
	size_t i;
	result = (DREF STRUCT_TYPE *)DeeObject_Malloc(COMPILER_OFFSETOF(STRUCT_TYPE, sg_argv) +
	                                              (other->sg_argc * sizeof(DREF DeeObject *)));
	if
		unlikely(!result)
	goto done;
	result->se_seq  = other->se_seq;
	result->sg_attr = other->sg_attr;
	result->sg_argc = other->sg_argc;
	MEMCPY_PTR(result->sg_argv, other->sg_argv, result->sg_argc);
	for (i = 0; i < result->sg_argc; ++i)
		Dee_Incref(result->sg_argv[i]);
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

PRIVATE DREF STRUCT_TYPE *DCALL
F(deep)(STRUCT_TYPE *__restrict other) {
	DREF STRUCT_TYPE *result;
	size_t i;
	result = (DREF STRUCT_TYPE *)DeeObject_Malloc(COMPILER_OFFSETOF(STRUCT_TYPE, sg_argv) +
	                                              (other->sg_argc * sizeof(DREF DeeObject *)));
	if
		unlikely(!result)
	goto done;
	result->se_seq = DeeObject_DeepCopy(other->se_seq);
	if
		unlikely(!result->se_seq)
	goto err_r;
#ifdef DEFINE_CALLATTRKW
	result->sg_kw = DeeObject_DeepCopy(other->sg_kw);
	if
		unlikely(!result->sg_kw)
	goto err_r_seq;
#endif /* DEFINE_CALLATTRKW */
	result->sg_argc = other->sg_argc;
	for (i = 0; i < result->sg_argc; ++i) {
		result->sg_argv[i] = DeeObject_DeepCopy(other->sg_argv[i]);
		if
			unlikely(!result->sg_argv[i])
		goto err_r_argv;
	}
	result->sg_attr = other->sg_attr;
	Dee_Incref(result->sg_attr);
	DeeObject_Init(result, &TYPE_OBJECT);
done:
	return result;
err_r_argv:
	while (i--)
		Dee_Decref(result->sg_argv[i]);
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

PRIVATE DREF STRUCT_TYPE *DCALL
F(init)(size_t argc, DeeObject **__restrict argv) {
	DREF STRUCT_TYPE *result;
	DeeStringObject *attr;
	DeeObject *seq;
	size_t i;
	DeeObject *args = Dee_EmptyTuple;
#ifdef DEFINE_CALLATTR
	if (DeeArg_Unpack(argc, argv, "oo|o:_SeqEachCallAttr", &seq, &attr, &args))
		goto err;
#elif defined(DEFINE_CALLATTRKW)
	DeeObject *kw = Dee_EmptyMapping;
	if (DeeArg_Unpack(argc, argv, "oo|oo:_SeqEachCallAttrKw", &seq, &attr, &args, &kw))
		goto err;
#else
#error "Unsupported mode"
#endif
	if (DeeObject_AssertTypeExact(attr, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	result = (DREF STRUCT_TYPE *)DeeObject_Malloc(COMPILER_OFFSETOF(STRUCT_TYPE, sg_argv) +
	                                              (DeeTuple_SIZE(args) * sizeof(DREF DeeObject *)));
	if
		unlikely(!result)
	goto err;
	result->sg_argc = DeeTuple_SIZE(args);
	MEMCPY_PTR(result->sg_argv, DeeTuple_ELEM(args), DeeTuple_SIZE(args));
	for (i = 0; i < DeeTuple_SIZE(args); ++i)
		Dee_Incref(result->sg_argv[i]);
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

#else
#error "Unsupported mode"
#endif

#ifdef DEFINE_GETATTR
PRIVATE DREF SeqEachCallAttr *DCALL
F(call)(STRUCT_TYPE *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
	return (DREF SeqEachCallAttr *)DeeSeqEach_CallAttr(self->se_seq,
	                                                   (DeeObject *)self->sg_attr,
	                                                   argc, argv);
}

PRIVATE DREF SeqEachCallAttrKw *DCALL
F(call_kw)(STRUCT_TYPE *__restrict self, size_t argc,
           DeeObject **__restrict argv, DeeObject *kw) {
	return (DREF SeqEachCallAttrKw *)DeeSeqEach_CallAttrKw(self->se_seq,
	                                                       (DeeObject *)self->sg_attr,
	                                                       argc, argv, kw);
}
#endif

INTERN DeeTypeObject TYPE_OBJECT = {
	OBJECT_HEAD_INIT(&DeeType_Type),
#ifdef DEFINE_GETATTR
	/* .tp_name     = */ "_SeqEachGetAttr",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,attr:?Dstring)"),
#define TYPE_FLAGS (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY)
	/* .tp_flags    = */TYPE_FLAGS,
#elif defined(DEFINE_CALLATTR)
	/* .tp_name     = */ "_SeqEachCallAttr",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,attr:?Dstring,args=!T0)"),
#define TYPE_FLAGS (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY | TP_FVARIABLE)
	/* .tp_flags    = */TYPE_FLAGS,
#elif defined(DEFINE_CALLATTRKW)
	/* .tp_name     = */ "_SeqEachCallAttrKw",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,attr:?Dstring,args=!T0,kw:?S?T2?Dstring?O=!T0)"),
#define TYPE_FLAGS (TP_FNORMAL | TP_FFINAL | TP_FMOVEANY | TP_FVARIABLE)
	/* .tp_flags    = */TYPE_FLAGS,
#else
#error "Unsupported mode"
#endif
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
#if !(TYPE_FLAGS & TP_FVARIABLE)
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&F(ctor),
				/* .tp_copy_ctor = */ (void *)&F(copy),
				/* .tp_deep_ctor = */ (void *)&F(deep),
				/* .tp_any_ctor  = */ (void *)&F(init),
				TYPE_FIXED_ALLOCATOR(STRUCT_TYPE)
			}
#else
			/* .tp_var = */ {
				/* .tp_ctor      = */ (void *)&F(ctor),
				/* .tp_copy_ctor = */ (void *)&F(copy),
				/* .tp_deep_ctor = */ (void *)&F(deep),
				/* .tp_any_ctor  = */ (void *)&F(init),
				/* .tp_free      = */ (void *)NULL
			}
#endif
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&F(fini),
		/* .tp_assign      = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_assign,
		/* .tp_move_assign = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&sew_moveassign
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&sew_bool
	},
#ifdef DEFINE_GETATTR
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject **__restrict))&F(call),
#else /* DEFINE_GETATTR */
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject **__restrict))&sew_call,
#endif /* !DEFINE_GETATTR */
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&F(visit),
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &F(math),
	/* .tp_cmp           = */ &sew_cmp,
	/* .tp_seq           = */ &F(seq),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_iter_next,
	/* .tp_attr          = */ &sew_attr,
	/* .tp_with          = */ &sew_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL, /* TODO: Access to the arguments vector */
	/* .tp_members       = */ F(members),
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ F(class_members),
#ifdef DEFINE_GETATTR
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject **__restrict, DeeObject *))&F(call_kw),
#else /* DEFINE_GETATTR */
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject **__restrict, DeeObject *))&sew_call_kw,
#endif /* !DEFINE_GETATTR */
};

#undef TYPE_FLAGS




PRIVATE int DCALL
Fi(ctor)(SeqEachIterator *__restrict self) {
	self->ei_each = (DREF SeqEachBase *)DeeObject_NewDefault(&TYPE_OBJECT);
	if
		unlikely(!self->ei_each)
	goto err;
	self->ei_iter = DeeObject_IterSelf(self->ei_each->se_seq);
	if
		unlikely(!self->ei_iter)
	goto err_each;
	return 0;
err_each:
	Dee_Decref(self->ei_each);
err:
	return -1;
}

PRIVATE int DCALL
Fi(init)(SeqEachIterator *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
#ifdef DEFINE_GETATTR
	if (DeeArg_Unpack(argc, argv, "o:_SeqEachGetAttrIterator", &self->ei_each))
		goto err;
#elif defined(DEFINE_CALLATTR)
	if (DeeArg_Unpack(argc, argv, "o:_SeqEachCallAttrIterator", &self->ei_each))
		goto err;
#elif defined(DEFINE_CALLATTRKW)
	if (DeeArg_Unpack(argc, argv, "o:_SeqEachCallAttrIteratorKw", &self->ei_each))
		goto err;
#else
#error "Unsupported mode"
#endif
	if (DeeObject_AssertTypeExact(self->ei_each, &TYPE_OBJECT))
		goto err;
	self->ei_iter = DeeObject_IterSelf(self->ei_each->se_seq);
	if
		unlikely(!self->ei_iter)
	goto err;
	Dee_Incref(self->ei_each);
	return 0;
err:
	return -1;
}

#ifndef CONFIG_NO_DOC
PRIVATE struct type_member Fi(members)[] = {
#ifdef DEFINE_GETATTR
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SeqEachIterator, ei_each), "->?Ert:SeqEachGetAttr"),
#elif defined(DEFINE_CALLATTR)
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SeqEachIterator, ei_each), "->?Ert:SeqEachCallAttr"),
#elif defined(DEFINE_CALLATTRKW)
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SeqEachIterator, ei_each), "->?Ert:SeqEachCallAttrKw"),
#else
#error "Unsupported mode"
#endif
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SeqEachIterator, ei_iter), "->?DIterator"),
	TYPE_MEMBER_END
};
#endif /* !CONFIG_NO_DOC */

PRIVATE DREF DeeObject *DCALL
Fi(next)(SeqEachIterator *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_IterNext(self->ei_iter);
	if
		likely(ITER_ISOK(result))
	result = F(transform)((STRUCT_TYPE *)self->ei_each, result);
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
	/* .tp_name     = */ "_SeqEachCallAttrIteratorKw",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqEachCallAttrKw)"),
#else
#error "Unsupported mode"
#endif
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&Fi(ctor),
				/* .tp_copy_ctor = */ (void *)&sewi_copy,
				/* .tp_deep_ctor = */ (void *)&sewi_deep,
				/* .tp_any_ctor  = */ (void *)&Fi(init),
				TYPE_FIXED_ALLOCATOR(SeqEachIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&sewi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&sewi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sewi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sewi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&Fi(next),
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
