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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_EACH_C
#define GUARD_DEEMON_OBJECTS_SEQ_EACH_C 1

#include "each.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/tuple.h>
#include <deemon/util/string.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachOperator *DCALL
seqeach_makeop0(DeeObject *__restrict seq, uint16_t opname) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(0);
	if unlikely(!result)
		goto done;
	result->se_seq    = seq;
	result->so_opname = opname;
	result->so_opargc = 0;
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqEachOperator_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
seqeach_makeop1(DeeObject *__restrict seq, uint16_t opname,
                /*inherit(always)*/ DREF DeeObject *__restrict arg_0) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(1);
	if unlikely(!result)
		goto err;
	result->se_seq       = seq;
	result->so_opname    = opname;
	result->so_opargc    = 1;
	result->so_opargv[0] = arg_0; /* Inherit reference. */
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqEachOperator_Type);
	return result;
err:
	Dee_Decref(arg_0);
	return NULL;
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
seqeach_makeop2(DeeObject *__restrict seq, uint16_t opname,
                /*inherit(always)*/ DREF DeeObject *__restrict arg_0,
                /*inherit(always)*/ DREF DeeObject *__restrict arg_1) {
	DREF SeqEachOperator *result;
	result = SeqEachOperator_MALLOC(2);
	if unlikely(!result)
		goto err;
	result->se_seq       = seq;
	result->so_opname    = opname;
	result->so_opargc    = 2;
	result->so_opargv[0] = arg_0; /* Inherit reference. */
	result->so_opargv[1] = arg_1; /* Inherit reference. */
	Dee_Incref(seq);
	DeeObject_Init(result, &SeqEachOperator_Type);
	return result;
err:
	Dee_Decref(arg_1);
	Dee_Decref(arg_0);
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
se_ctor(SeqEachBase *__restrict self) {
	self->se_seq = Dee_EmptySeq;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_copy(SeqEachBase *__restrict self,
        SeqEachBase *__restrict other) {
	self->se_seq = other->se_seq;
	Dee_Incref(self->se_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_deep(SeqEachBase *__restrict self,
        SeqEachBase *__restrict other) {
	self->se_seq = DeeObject_DeepCopy(other->se_seq);
	if unlikely(!self->se_seq)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_init(SeqEachBase *__restrict self,
        size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqEach", &self->se_seq))
		goto err;
	Dee_Incref(self->se_seq);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
se_fini(SeqEachBase *__restrict self) {
	Dee_Decref(self->se_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
se_visit(SeqEachBase *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->se_seq);
}

#define SEQ_FOREACH_APPLY_EX(seq, size_seq, elem, func)      \
	DREF DeeObject *iter, *elem;                             \
	size_t i, fast_size;                                     \
	fast_size = DeeFastSeq_GetSize(size_seq);                \
	if (fast_size != DEE_FASTSEQ_NOTFAST) {                  \
		for (i = 0; i < fast_size; ++i) {                    \
			elem = DeeFastSeq_GetItemUnbound(seq, i);        \
			if (!ITER_ISOK(elem)) {                          \
				if (elem == NULL)                            \
					continue; /* Unbound item */             \
				goto err;                                    \
			}                                                \
			if unlikely(func)                                \
				goto err_elem_only;                          \
			Dee_Decref(elem);                                \
		}                                                    \
	} else {                                                 \
		iter = DeeObject_IterSelf(seq);                      \
		if unlikely(!iter)                                   \
			goto err;                                        \
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) { \
			if unlikely(func)                                \
				goto err_elem;                               \
			Dee_Decref(elem);                                \
		}                                                    \
		if unlikely(!elem)                                   \
			goto err_iter;                                   \
		Dee_Decref(iter);                                    \
	}                                                        \
	return 0;                                                \
err_elem_only:                                               \
	Dee_Decref(elem);                                        \
	goto err;                                                \
err_elem:                                                    \
	Dee_Decref(elem);                                        \
err_iter:                                                    \
	Dee_Decref(iter);                                        \
err:                                                         \
	return -1;
#define SEQ_FOREACH_APPLY(seq, elem, func) \
	SEQ_FOREACH_APPLY_EX(seq, seq, elem, func)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_assign(SeqEachBase *__restrict self, DeeObject *__restrict value) {
	SEQ_FOREACH_APPLY(self->se_seq, elem, DeeObject_Assign(elem, value));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_moveassign(SeqEachBase *__restrict self, DeeObject *__restrict value) {
	SEQ_FOREACH_APPLY(self->se_seq, elem, DeeObject_MoveAssign(elem, value));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_delitem(SeqEachBase *__restrict self, DeeObject *__restrict index) {
	SEQ_FOREACH_APPLY(self->se_seq, elem, DeeObject_DelItem(elem, index));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
se_setitem(SeqEachBase *__restrict self, DeeObject *__restrict index, DeeObject *__restrict value) {
	SEQ_FOREACH_APPLY(self->se_seq, elem, DeeObject_SetItem(elem, index, value));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
se_delrange(SeqEachBase *__restrict self, DeeObject *__restrict start, DeeObject *__restrict end) {
	SEQ_FOREACH_APPLY(self->se_seq, elem, DeeObject_DelRange(elem, start, end));
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
se_setrange(SeqEachBase *__restrict self, DeeObject *__restrict start, DeeObject *__restrict end, DeeObject *__restrict value) {
	SEQ_FOREACH_APPLY(self->se_seq, elem, DeeObject_SetRange(elem, start, end, value));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
se_delattr(SeqEachBase *__restrict self, DeeObject *__restrict attr) {
	SEQ_FOREACH_APPLY(self->se_seq, elem, DeeObject_DelAttr(elem, attr));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
se_setattr(SeqEachBase *__restrict self, DeeObject *__restrict attr, DeeObject *__restrict value) {
	SEQ_FOREACH_APPLY(self->se_seq, elem, DeeObject_SetAttr(elem, attr, value));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
se_str(SeqEachBase *__restrict self) {
	return DeeString_Newf("<each of %r>", self->se_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
se_repr(SeqEachBase *__restrict self) {
	if (DeeObject_InstanceOf(self->se_seq, &DeeSeq_Type))
		return DeeString_Newf("%r.each", self->se_seq);
	return DeeString_Newf("(%r as sequence).each", self->se_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_bool(SeqEachBase *__restrict self) {
	return DeeObject_Bool(self->se_seq);
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
se_call(SeqEachBase *self, size_t argc, DeeObject **argv) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return seqeach_makeop1(self->se_seq, OPERATOR_CALL, tuple);
err:
	return NULL;
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
se_call_kw(SeqEachBase *__restrict self, size_t argc,
           DeeObject **argv, DeeObject *kw) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return kw
	       ? (Dee_Incref(kw),
	          seqeach_makeop2(self->se_seq, OPERATOR_CALL, tuple, kw))
	       : (seqeach_makeop1(self->se_seq, OPERATOR_CALL, tuple));
err:
	return NULL;
}

#define DEFINE_SEQ_EACH_UNARY(name, op)           \
	PRIVATE WUNUSED DREF SeqEachOperator *DCALL           \
	name(SeqEachBase *__restrict self) {          \
		return seqeach_makeop0(self->se_seq, op); \
	}
#define DEFINE_SEQ_EACH_BINARY(name, op)                              \
	PRIVATE WUNUSED DREF SeqEachOperator *DCALL                               \
	name(SeqEachBase *__restrict self, DeeObject *__restrict other) { \
		Dee_Incref(other);                                            \
		return seqeach_makeop1(self->se_seq, op, other);              \
	}
#define DEFINE_SEQ_EACH_TRINARY(name, op)                                                  \
	PRIVATE WUNUSED DREF SeqEachOperator *DCALL                                                    \
	name(SeqEachBase *__restrict self, DeeObject *__restrict a, DeeObject *__restrict b) { \
		Dee_Incref(a);                                                                     \
		Dee_Incref(b);                                                                     \
		return seqeach_makeop2(self->se_seq, op, a, b);                                    \
	}
DEFINE_SEQ_EACH_UNARY(se_iter_next, OPERATOR_ITERNEXT)
DEFINE_SEQ_EACH_UNARY(se_inv, OPERATOR_INV)
DEFINE_SEQ_EACH_UNARY(se_pos, OPERATOR_POS)
DEFINE_SEQ_EACH_UNARY(se_neg, OPERATOR_NEG)
DEFINE_SEQ_EACH_BINARY(se_add, OPERATOR_ADD)
DEFINE_SEQ_EACH_BINARY(se_sub, OPERATOR_SUB)
DEFINE_SEQ_EACH_BINARY(se_mul, OPERATOR_MUL)
DEFINE_SEQ_EACH_BINARY(se_div, OPERATOR_DIV)
DEFINE_SEQ_EACH_BINARY(se_mod, OPERATOR_MOD)
DEFINE_SEQ_EACH_BINARY(se_shl, OPERATOR_SHL)
DEFINE_SEQ_EACH_BINARY(se_shr, OPERATOR_SHR)
DEFINE_SEQ_EACH_BINARY(se_and, OPERATOR_AND)
DEFINE_SEQ_EACH_BINARY(se_or, OPERATOR_OR)
DEFINE_SEQ_EACH_BINARY(se_xor, OPERATOR_XOR)
DEFINE_SEQ_EACH_BINARY(se_pow, OPERATOR_POW)
DEFINE_SEQ_EACH_BINARY(se_eq, OPERATOR_EQ)
DEFINE_SEQ_EACH_BINARY(se_ne, OPERATOR_NE)
DEFINE_SEQ_EACH_BINARY(se_lo, OPERATOR_LO)
DEFINE_SEQ_EACH_BINARY(se_le, OPERATOR_LE)
DEFINE_SEQ_EACH_BINARY(se_gr, OPERATOR_GR)
DEFINE_SEQ_EACH_BINARY(se_ge, OPERATOR_GE)
DEFINE_SEQ_EACH_UNARY(se_iter, OPERATOR_ITERSELF)
DEFINE_SEQ_EACH_UNARY(se_size, OPERATOR_SIZE)
DEFINE_SEQ_EACH_BINARY(se_contains, OPERATOR_CONTAINS)
DEFINE_SEQ_EACH_BINARY(se_getitem, OPERATOR_GETITEM)
DEFINE_SEQ_EACH_TRINARY(se_getrange, OPERATOR_GETRANGE)

#define DEFINE_SEQ_EACH_UNARY_INPLACE(name, func)          \
	PRIVATE int DCALL                                      \
	name(SeqEachBase **__restrict pself) {                 \
		size_t i, size;                                    \
		DeeObject *seq = (*pself)->se_seq;                 \
		DREF DeeObject *elem;                              \
		size = DeeObject_Size(seq);                        \
		if unlikely(size == (size_t)-1)                    \
			goto err;                                      \
		for (i = 0; i < size; ++i) {                       \
			elem = DeeObject_GetItemIndex(seq, i);         \
			if unlikely(!elem) {                           \
				if (DeeError_Catch(&DeeError_UnboundItem)) \
					continue;                              \
				goto err;                                  \
			}                                              \
			if (func(&elem))                               \
				goto err_elem;                             \
			if (DeeObject_SetItemIndex(seq, i, elem))      \
				goto err_elem;                             \
			Dee_Decref(elem);                              \
		}                                                  \
		return 0;                                          \
	err_elem:                                              \
		Dee_Decref(elem);                                  \
	err:                                                   \
		return -1;                                         \
	}

#define DEFINE_SEQ_EACH_BINARY_INPLACE(name, func)                      \
	PRIVATE int DCALL                                                   \
	name(SeqEachBase **__restrict pself, DeeObject *__restrict other) { \
		size_t i, size;                                                 \
		DeeObject *seq = (*pself)->se_seq;                              \
		DREF DeeObject *elem;                                           \
		size = DeeObject_Size(seq);                                     \
		if unlikely(size == (size_t)-1)                                 \
			goto err;                                                   \
		for (i = 0; i < size; ++i) {                                    \
			elem = DeeObject_GetItemIndex(seq, i);                      \
			if unlikely(!elem) {                                        \
				if (DeeError_Catch(&DeeError_UnboundItem))              \
					continue;                                           \
				goto err;                                               \
			}                                                           \
			if (func(&elem, other))                                     \
				goto err_elem;                                          \
			if (DeeObject_SetItemIndex(seq, i, elem))                   \
				goto err_elem;                                          \
			Dee_Decref(elem);                                           \
		}                                                               \
		return 0;                                                       \
	err_elem:                                                           \
		Dee_Decref(elem);                                               \
	err:                                                                \
		return -1;                                                      \
	}
DEFINE_SEQ_EACH_UNARY_INPLACE(se_inc, DeeObject_Inc)
DEFINE_SEQ_EACH_UNARY_INPLACE(se_dec, DeeObject_Dec)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_add, DeeObject_InplaceAdd)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_sub, DeeObject_InplaceSub)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_mul, DeeObject_InplaceMul)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_div, DeeObject_InplaceDiv)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_mod, DeeObject_InplaceMod)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_shl, DeeObject_InplaceShl)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_shr, DeeObject_InplaceShr)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_and, DeeObject_InplaceAnd)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_or, DeeObject_InplaceOr)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_xor, DeeObject_InplaceXor)
DEFINE_SEQ_EACH_BINARY_INPLACE(se_inplace_pow, DeeObject_InplacePow)



PRIVATE struct type_math se_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_inv,
	/* .tp_pos         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_pos,
	/* .tp_neg         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_neg,
	/* .tp_add         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_add,
	/* .tp_sub         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_sub,
	/* .tp_mul         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_mul,
	/* .tp_div         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_div,
	/* .tp_mod         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_mod,
	/* .tp_shl         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_shl,
	/* .tp_shr         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_shr,
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_xor,
	/* .tp_pow         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_pow,
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&se_inc,
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&se_dec,
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_add,
	/* .tp_inplace_sub = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_sub,
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_mul,
	/* .tp_inplace_div = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_div,
	/* .tp_inplace_mod = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_mod,
	/* .tp_inplace_shl = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_shl,
	/* .tp_inplace_shr = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_shr,
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_and,
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_or,
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_xor,
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&se_inplace_pow
};

PRIVATE struct type_cmp se_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_ge
};

PRIVATE struct type_seq se_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&se_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&se_setitem,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&se_getrange,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&se_delrange,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&se_setrange
};

PRIVATE char const s_unhandled_leave_message[] = "Unhandled exception in `operator leave'";


PRIVATE WUNUSED NONNULL((1)) int DCALL
se_enter(SeqEachBase *__restrict self) {
	DREF DeeObject **elem;
	size_t i, count;
	elem = DeeSeq_AsHeapVector(self->se_seq, &count);
	if unlikely(!elem)
		goto err;
	for (i = 0; i < count; ++i) {
		if (DeeObject_Enter(elem[i]))
			goto err_elem_i;
	}
	while (count--)
		Dee_Decref(elem[count]);
	Dee_Free(elem);
	return 0;
err_elem_i:
	while (i--) {
		if unlikely(DeeObject_Leave(elem[i]))
			DeeError_Print(s_unhandled_leave_message, ERROR_PRINT_DOHANDLE);
	}
/*err_elem:*/
	while (count--)
		Dee_Decref(elem[count]);
	Dee_Free(elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
se_leave(SeqEachBase *__restrict self) {
	DREF DeeObject **elem;
	size_t count;
	elem = DeeSeq_AsHeapVector(self->se_seq, &count);
	if unlikely(!elem)
		goto err;
	while (count--) {
		if unlikely(DeeObject_Leave(elem[count]))
			goto err_elem_count;
		Dee_Decref(elem[count]);
	}
	Dee_Free(elem);
	return 0;
	while (count--) {
		if unlikely(DeeObject_Leave(elem[count]))
			DeeError_Print(s_unhandled_leave_message, ERROR_PRINT_DOHANDLE);
err_elem_count:
		Dee_Decref(elem[count]);
	}
	Dee_Free(elem);
err:
	return -1;
}

PRIVATE struct type_with se_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&se_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&se_leave
};


#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#define se_getattr seqeach_getattr
INTERN WUNUSED NONNULL((1, 2)) DREF SeqEachGetAttr *DCALL
seqeach_getattr(SeqEachBase *__restrict self,
                struct string_object *__restrict attr) {
	DREF SeqEachGetAttr *result;
	result = DeeObject_MALLOC(SeqEachGetAttr);
	if unlikely(!result)
		goto done;
	result->se_seq  = self->se_seq;
	result->sg_attr = attr;
	Dee_Incref(self->se_seq);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqEachGetAttr_Type);
done:
	return result;
}
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
DEFINE_SEQ_EACH_BINARY(se_getattr, OPERATOR_GETATTR)
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */


PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
sew_enumattr(DeeTypeObject *UNUSED(tp_self),
             DeeObject *self, denum_t proc, void *arg) {
	(void)self;
	(void)proc;
	(void)arg;
	/* TODO: Enumerate attributes available via the common
	 *       base class of all elements of `self'. */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
se_enumattr(DeeTypeObject *UNUSED(tp_self),
            SeqEachBase *self, denum_t proc, void *arg) {
	return sew_enumattr(&DeeSeq_Type,
	                    self->se_seq,
	                    proc,
	                    arg);
}

PRIVATE struct type_attr se_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&se_getattr,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&se_delattr,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&se_setattr,
	/* .tp_enumattr = */ (dssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&se_enumattr
};

PRIVATE struct type_member se_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(SeqEachBase, se_seq), "->?DSequence"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqEach_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEach",
	/* .tp_doc      = */ DOC("(seq:?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL|TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&se_ctor,
				/* .tp_copy_ctor = */ (void *)&se_copy,
				/* .tp_deep_ctor = */ (void *)&se_deep,
				/* .tp_any_ctor  = */ (void *)&se_init,
				TYPE_FIXED_ALLOCATOR(SeqEachBase)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&se_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&se_moveassign
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&se_bool
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&se_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&se_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &se_math,
	/* .tp_cmp           = */ &se_cmp,
	/* .tp_seq           = */ &se_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&se_iter_next,
	/* .tp_attr          = */ &se_attr,
	/* .tp_with          = */ &se_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ se_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **__restrict, DeeObject *))&se_call_kw,
};

#undef DEFINE_SEQ_EACH_TRINARY
#undef DEFINE_SEQ_EACH_BINARY
#undef DEFINE_SEQ_EACH_UNARY
#undef DEFINE_SEQ_EACH_BINARY_INPLACE
#undef DEFINE_SEQ_EACH_UNARY_INPLACE



/* Construct an each-wrapper for `self' */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Each(DeeObject *__restrict self) {
	DREF SeqEachBase *result;
	result = DeeObject_MALLOC(SeqEachBase);
	if unlikely(!result)
		goto done;
	result->se_seq = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqEach_Type);
done:
	return (DREF DeeObject *)result;
}





/* SeqEach<WRAPPER> -- Operator */
#define sew_bool   se_bool
PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_ctor(SeqEachOperator *__restrict self) {
	self->se_seq    = Dee_EmptySeq;
	self->so_opname = OPERATOR_BOOL;
	self->so_opargc = 0;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_copy(SeqEachOperator *__restrict self,
         SeqEachOperator *__restrict other) {
	size_t i;
	self->se_seq    = other->se_seq;
	self->so_opname = other->so_opname;
	self->so_opargc = other->so_opargc;
	MEMCPY_PTR(self->so_opargv, other->so_opargv, self->so_opargc);
	for (i = 0; i < self->so_opargc; ++i)
		Dee_Incref(self->so_opargv[i]);
	Dee_Incref(self->se_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
seo_deep(SeqEachOperator *__restrict self,
         SeqEachOperator *__restrict other) {
	size_t i;
	self->se_seq = DeeObject_DeepCopy(other->se_seq);
	if unlikely(!self->se_seq)
		goto err;
	for (i = 0; i < other->so_opargc; ++i) {
		self->so_opargv[i] = DeeObject_DeepCopy(other->so_opargv[i]);
		if unlikely(!self->so_opargv[i])
			goto err_i;
	}
	self->so_opname = other->so_opname;
	self->so_opargc = other->so_opargc;
	return 0;
err_i:
	while (i--)
		Dee_Decref(self->so_opargv[i]);
	Dee_Decref(self->se_seq);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seo_init(SeqEachOperator *__restrict self,
         size_t argc, DeeObject **argv) {
	size_t i;
	DeeObject *name;
	DeeObject *args = Dee_EmptyTuple;
	if (DeeArg_Unpack(argc, argv, "oo|o:_SeqEachOperator",
	                  &self->se_seq, &name, &args))
		goto err;
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if unlikely(DeeTuple_SIZE(args) > COMPILER_LENOF(self->so_opargv)) {
		DeeError_Throwf(&DeeError_UnpackError,
		                "Too many operator arguments (%Iu > %Iu)",
		                (size_t)DeeTuple_SIZE(args),
		                (size_t)COMPILER_LENOF(self->so_opargv));
		goto err;
	}
	if (DeeString_Check(name)) {
		self->so_opname = Dee_OperatorFromName(NULL, DeeString_STR(name));
		if unlikely(self->so_opname == (uint16_t)-1) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Unknown operator %q",
			                DeeString_STR(name));
			goto err;
		}
	} else {
		if (DeeObject_AsUInt16(name, &self->so_opname))
			goto err;
	}
	self->so_opargc = (uint16_t)DeeTuple_SIZE(args);
	MEMCPY_PTR(self->so_opargv, DeeTuple_ELEM(args), self->so_opargc);
	for (i = 0; i < self->so_opargc; ++i)
		Dee_Incref(self->so_opargv[i]);
	Dee_Incref(self->se_seq);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
seo_fini(SeqEachOperator *__restrict self) {
	size_t i;
	Dee_Decref(self->se_seq);
	for (i = 0; i < self->so_opargc; ++i)
		Dee_Decref(self->so_opargv[i]);
}

PRIVATE NONNULL((1, 2)) void DCALL
seo_visit(SeqEachOperator *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	Dee_Visit(self->se_seq);
	for (i = 0; i < self->so_opargc; ++i)
		Dee_Visit(self->so_opargv[i]);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sew_assign(DeeObject *__restrict self, DeeObject *__restrict value) {
	SEQ_FOREACH_APPLY_EX(self, ((SeqEachBase *)self)->se_seq, elem,
	                     DeeObject_Assign(elem, value));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sew_moveassign(DeeObject *__restrict self, DeeObject *__restrict value) {
	SEQ_FOREACH_APPLY_EX(self, ((SeqEachBase *)self)->se_seq, elem,
	                     DeeObject_MoveAssign(elem, value));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sew_delattr(DeeObject *__restrict self, DeeObject *__restrict attr) {
	SEQ_FOREACH_APPLY_EX(self, ((SeqEachBase *)self)->se_seq, elem,
	                     DeeObject_DelAttr(elem, attr));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
sew_setattr(DeeObject *__restrict self, DeeObject *__restrict attr, DeeObject *__restrict value) {
	SEQ_FOREACH_APPLY_EX(self, ((SeqEachBase *)self)->se_seq, elem,
	                     DeeObject_SetAttr(elem, attr, value));
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sew_call(DeeObject *self, size_t argc, DeeObject **argv) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return (DREF DeeObject *)seqeach_makeop1(self, OPERATOR_CALL, tuple);
err:
	return NULL;
}

PRIVATE WUNUSED DREF SeqEachOperator *DCALL
sew_call_kw(DeeObject *__restrict self, size_t argc,
            DeeObject **argv, DeeObject *kw) {
	DREF DeeObject *tuple;
	tuple = DeeTuple_NewVector(argc, argv);
	if unlikely(!tuple)
		goto err;
	return kw
	       ? (Dee_Incref(kw),
	          seqeach_makeop2(self, OPERATOR_CALL, tuple, kw))
	       : (seqeach_makeop1(self, OPERATOR_CALL, tuple));
err:
	return NULL;
}


#define DEFINE_SEW_UNARY(name, op)        \
	PRIVATE WUNUSED DREF SeqEachOperator *DCALL   \
	name(DeeObject *__restrict self) {    \
		return seqeach_makeop0(self, op); \
	}
#define DEFINE_SEW_BINARY(name, op)                                 \
	PRIVATE WUNUSED DREF SeqEachOperator *DCALL                             \
	name(DeeObject *__restrict self, DeeObject *__restrict other) { \
		Dee_Incref(other);                                          \
		return seqeach_makeop1(self, op, other);                    \
	}
#define DEFINE_SEW_TRINARY(name, op)                                                     \
	PRIVATE WUNUSED DREF SeqEachOperator *DCALL                                                  \
	name(DeeObject *__restrict self, DeeObject *__restrict a, DeeObject *__restrict b) { \
		Dee_Incref(a);                                                                   \
		Dee_Incref(b);                                                                   \
		return seqeach_makeop2(self, op, a, b);                                          \
	}
DEFINE_SEW_UNARY(sew_iter_next, OPERATOR_ITERNEXT)
DEFINE_SEW_UNARY(sew_inv, OPERATOR_INV)
DEFINE_SEW_UNARY(sew_pos, OPERATOR_POS)
DEFINE_SEW_UNARY(sew_neg, OPERATOR_NEG)
DEFINE_SEW_BINARY(sew_add, OPERATOR_ADD)
DEFINE_SEW_BINARY(sew_sub, OPERATOR_SUB)
DEFINE_SEW_BINARY(sew_mul, OPERATOR_MUL)
DEFINE_SEW_BINARY(sew_div, OPERATOR_DIV)
DEFINE_SEW_BINARY(sew_mod, OPERATOR_MOD)
DEFINE_SEW_BINARY(sew_shl, OPERATOR_SHL)
DEFINE_SEW_BINARY(sew_shr, OPERATOR_SHR)
DEFINE_SEW_BINARY(sew_and, OPERATOR_AND)
DEFINE_SEW_BINARY(sew_or, OPERATOR_OR)
DEFINE_SEW_BINARY(sew_xor, OPERATOR_XOR)
DEFINE_SEW_BINARY(sew_pow, OPERATOR_POW)
DEFINE_SEW_BINARY(sew_eq, OPERATOR_EQ)
DEFINE_SEW_BINARY(sew_ne, OPERATOR_NE)
DEFINE_SEW_BINARY(sew_lo, OPERATOR_LO)
DEFINE_SEW_BINARY(sew_le, OPERATOR_LE)
DEFINE_SEW_BINARY(sew_gr, OPERATOR_GR)
DEFINE_SEW_BINARY(sew_ge, OPERATOR_GE)


PRIVATE WUNUSED DREF DeeObject *DCALL
seo_getitem_for_inplace(SeqEachOperator *__restrict self,
                        DREF DeeObject **__restrict pbaseelem,
                        size_t index, uint16_t operator_name) {
	DREF DeeObject *result, *baseelem;
	baseelem = DeeObject_GetItemIndex(self->se_seq, index);
	if unlikely(!baseelem)
		goto err;
	switch (self->so_opname) {

		/* Only a select few operators can be used for inplace. */
	case OPERATOR_GETATTR:
		if unlikely(self->so_opargc < 1)
			goto err_noimp;
		if (DeeObject_AssertTypeExact(self->so_opargv[0], &DeeString_Type))
			goto err_baseelem;
		result = DeeObject_GetAttr(baseelem, self->so_opargv[0]);
		break;

	case OPERATOR_GETITEM:
		if unlikely(self->so_opargc < 1)
			goto err_noimp;
		result = DeeObject_GetItem(baseelem, self->so_opargv[0]);
		break;

	case OPERATOR_GETRANGE:
		if unlikely(self->so_opargc < 2)
			goto err_noimp;
		result = DeeObject_GetRange(baseelem,
		                            self->so_opargv[0],
		                            self->so_opargv[1]);
		break;

	default: goto err_noimp;
	}
	if unlikely(!result)
		goto err_baseelem;
	*pbaseelem = baseelem;
	return result;
err_noimp:
	err_unimplemented_operator(&SeqEachOperator_Type, operator_name);
err_baseelem:
	Dee_Decref(baseelem);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
seo_setitem_for_inplace(SeqEachOperator *__restrict self,
                        DeeObject *__restrict baseelem,
                        DeeObject *__restrict value) {
	switch (self->so_opname) {
		/* Only a select few operators can be used for inplace. */

	case OPERATOR_GETATTR:
		ASSERT(self->so_opargc >= 1);
		ASSERT_OBJECT_TYPE_EXACT(self->so_opargv[0], &DeeString_Type);
		return DeeObject_SetAttr(baseelem, self->so_opargv[0], value);

	case OPERATOR_GETITEM:
		ASSERT(self->so_opargc >= 1);
		return DeeObject_SetItem(baseelem, self->so_opargv[0], value);

	case OPERATOR_GETRANGE:
		ASSERT(self->so_opargc >= 2);
		return DeeObject_SetRange(baseelem,
		                          self->so_opargv[0],
		                          self->so_opargv[1],
		                          value);

	default: __builtin_unreachable();
	}
}


#define DEFINE_SEO_UNARY_INPLACE(name, func, op)                   \
	PRIVATE int DCALL                                              \
	name(SeqEachOperator **__restrict pself) {                     \
		size_t i, size;                                            \
		SeqEachOperator *seq = *pself;                             \
		DREF DeeObject *elem, *baseelem;                           \
		size = DeeObject_Size(seq->se_seq);                        \
		if unlikely(size == (size_t)-1)                            \
			goto err;                                              \
		for (i = 0; i < size; ++i) {                               \
			elem = seo_getitem_for_inplace(seq, &baseelem, i, op); \
			if unlikely(!elem) {                                   \
				if (DeeError_Catch(&DeeError_UnboundItem))         \
					continue;                                      \
				goto err;                                          \
			}                                                      \
			if (func(&elem))                                       \
				goto err_elem;                                     \
			if (seo_setitem_for_inplace(seq, baseelem, elem))      \
				goto err_elem;                                     \
			Dee_Decref(baseelem);                                  \
			Dee_Decref(elem);                                      \
		}                                                          \
		return 0;                                                  \
	err_elem:                                                      \
		Dee_Decref(baseelem);                                      \
		Dee_Decref(elem);                                          \
	err:                                                           \
		return -1;                                                 \
	}

#define DEFINE_SEO_BINARY_INPLACE(name, func, op)                           \
	PRIVATE int DCALL                                                       \
	name(SeqEachOperator **__restrict pself, DeeObject *__restrict other) { \
		size_t i, size;                                                     \
		SeqEachOperator *seq = *pself;                                      \
		DREF DeeObject *elem, *baseelem;                                    \
		size = DeeObject_Size(seq->se_seq);                                 \
		if unlikely(size == (size_t)-1)                                     \
			goto err;                                                       \
		for (i = 0; i < size; ++i) {                                        \
			elem = seo_getitem_for_inplace(seq, &baseelem, i, op);          \
			if unlikely(!elem) {                                            \
				if (DeeError_Catch(&DeeError_UnboundItem))                  \
					continue;                                               \
				goto err;                                                   \
			}                                                               \
			if (func(&elem, other))                                         \
				goto err_elem;                                              \
			if (seo_setitem_for_inplace(seq, baseelem, elem))               \
				goto err_elem;                                              \
			Dee_Decref(baseelem);                                           \
			Dee_Decref(elem);                                               \
		}                                                                   \
		return 0;                                                           \
	err_elem:                                                               \
		Dee_Decref(baseelem);                                               \
		Dee_Decref(elem);                                                   \
	err:                                                                    \
		return -1;                                                          \
	}
DEFINE_SEO_UNARY_INPLACE(seo_inc, DeeObject_Inc, OPERATOR_INC)
DEFINE_SEO_UNARY_INPLACE(seo_dec, DeeObject_Dec, OPERATOR_DEC)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_add, DeeObject_InplaceAdd, OPERATOR_INPLACE_ADD)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_sub, DeeObject_InplaceSub, OPERATOR_INPLACE_SUB)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_mul, DeeObject_InplaceMul, OPERATOR_INPLACE_MUL)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_div, DeeObject_InplaceDiv, OPERATOR_INPLACE_DIV)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_mod, DeeObject_InplaceMod, OPERATOR_INPLACE_MOD)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_shl, DeeObject_InplaceShl, OPERATOR_INPLACE_SHL)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_shr, DeeObject_InplaceShr, OPERATOR_INPLACE_SHR)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_and, DeeObject_InplaceAnd, OPERATOR_INPLACE_AND)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_or, DeeObject_InplaceOr, OPERATOR_INPLACE_OR)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_xor, DeeObject_InplaceXor, OPERATOR_INPLACE_XOR)
DEFINE_SEO_BINARY_INPLACE(seo_inplace_pow, DeeObject_InplacePow, OPERATOR_INPLACE_POW)
#undef DEFINE_SEO_UNARY_INPLACE
#undef DEFINE_SEO_BINARY_INPLACE



PRIVATE struct type_math seo_math = {
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
	/* .tp_inc         = */ (int (DCALL *)(DeeObject **__restrict))&seo_inc,
	/* .tp_dec         = */ (int (DCALL *)(DeeObject **__restrict))&seo_dec,
	/* .tp_inplace_add = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_add,
	/* .tp_inplace_sub = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_sub,
	/* .tp_inplace_mul = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_mul,
	/* .tp_inplace_div = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_div,
	/* .tp_inplace_mod = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_mod,
	/* .tp_inplace_shl = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_shl,
	/* .tp_inplace_shr = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_shr,
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_and,
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_or,
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_xor,
	/* .tp_inplace_pow = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))seo_inplace_pow
};

PRIVATE struct type_cmp sew_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sew_ge
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
sew_enter(DeeObject *__restrict self) {
	DREF DeeObject **elem;
	size_t i, count;
	elem = DeeSeq_AsHeapVector(self, &count);
	if unlikely(!elem)
		goto err;
	for (i = 0; i < count; ++i) {
		if (DeeObject_Enter(elem[i]))
			goto err_elem_i;
	}
	while (count--)
		Dee_Decref(elem[count]);
	Dee_Free(elem);
	return 0;
err_elem_i:
	while (i--) {
		if unlikely(DeeObject_Leave(elem[i]))
			DeeError_Print(s_unhandled_leave_message, ERROR_PRINT_DOHANDLE);
	}
/*err_elem:*/
	while (count--)
		Dee_Decref(elem[count]);
	Dee_Free(elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sew_leave(DeeObject *__restrict self) {
	DREF DeeObject **elem;
	size_t count;
	elem = DeeSeq_AsHeapVector(self, &count);
	if unlikely(!elem)
		goto err;
	while (count--) {
		if unlikely(DeeObject_Leave(elem[count]))
			goto err_elem_count;
		Dee_Decref(elem[count]);
	}
	Dee_Free(elem);
	return 0;
	while (count--) {
		if unlikely(DeeObject_Leave(elem[count]))
			DeeError_Print(s_unhandled_leave_message, ERROR_PRINT_DOHANDLE);
err_elem_count:
		Dee_Decref(elem[count]);
	}
	Dee_Free(elem);
err:
	return -1;
}

PRIVATE struct type_with sew_with = {
	/* .tp_enter = */ (int (DCALL *)(DeeObject *__restrict))&sew_enter,
	/* .tp_leave = */ (int (DCALL *)(DeeObject *__restrict))&sew_leave
};


#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#define sew_getattr seqeachw_getattr
INTERN WUNUSED NONNULL((1, 2)) DREF SeqEachGetAttr *DCALL
seqeachw_getattr(DeeObject *__restrict self,
                 struct string_object *__restrict attr) {
	DREF SeqEachGetAttr *result;
	result = DeeObject_MALLOC(SeqEachGetAttr);
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = attr;
	Dee_Incref(self);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqEachGetAttr_Type);
done:
	return result;
}
#else /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
DEFINE_SEW_BINARY(sew_getattr, OPERATOR_GETATTR)
#endif /* !CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */

PRIVATE struct type_attr sew_attr = {
	/* .tp_getattr  = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&sew_getattr,
	/* .tp_delattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&sew_delattr,
	/* .tp_setattr  = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&sew_setattr,
	/* .tp_enumattr = */ (dssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&sew_enumattr
};


PRIVATE WUNUSED NONNULL((1)) DREF SeqEachIterator *DCALL
seo_iter(SeqEachOperator *__restrict self) {
	DREF SeqEachIterator *result;
	result = DeeObject_MALLOC(SeqEachIterator);
	if unlikely(!result)
		goto done;
	result->ei_each = (DREF SeqEachBase *)self;
	result->ei_iter = DeeObject_IterSelf(((DREF SeqEachBase *)self)->se_seq);
	if unlikely(!result->ei_iter)
		goto err_r;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqEachOperatorIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sew_nsi_size(SeqEachBase *__restrict self) {
	return DeeObject_Size(self->se_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sew_nsi_fastsize(SeqEachBase *__restrict self) {
	return DeeFastSeq_GetSize(self->se_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sew_size(SeqEachBase *__restrict self) {
	return DeeObject_SizeObject(self->se_seq);
}

LOCAL WUNUSED DREF DeeObject *DCALL
seo_transform(SeqEachOperator *__restrict self,
              /*inherit(always)*/ DREF DeeObject *__restrict elem) {
	DREF DeeObject *result;
	result = DeeObject_InvokeOperator(elem,
	                                  self->so_opname,
	                                  self->so_opargc,
	                                  self->so_opargv);
	Dee_Decref(elem);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seo_nsi_getitem(SeqEachOperator *__restrict self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_GetItemIndex(self->se_seq, index);
	if likely(result)
		result = seo_transform(self, result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
seo_getitem(SeqEachOperator *self,
            DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_GetItem(self->se_seq, index);
	if likely(result)
		result = seo_transform(self, result);
	return result;
}


PRIVATE struct type_nsi seo_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&sew_nsi_size,
			/* .nsi_getsize_fast = */ (void *)&sew_nsi_fastsize,
			/* .nsi_getitem      = */ (void *)&seo_nsi_getitem,
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

PRIVATE struct type_seq seo_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seo_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&seo_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &seo_nsi
};


#define seo_members   se_members /* TODO: Access to operator name & arguments */

PRIVATE struct type_member seo_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SeqEachOperatorIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqEachOperator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEachOperator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,op:?X2?Dstring?Dint,args=!T0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL|TP_FMOVEANY,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&seo_ctor,
				/* .tp_copy_ctor = */ (void *)&seo_copy,
				/* .tp_deep_ctor = */ (void *)&seo_deep,
				/* .tp_any_ctor  = */ (void *)&seo_init,
				TYPE_SIZED_ALLOCATOR_R(COMPILER_OFFSETOF(SeqEachOperator,so_opargv),
				                       sizeof(SeqEachOperator))
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&seo_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&sew_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&sew_moveassign
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sew_bool
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sew_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&seo_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &seo_math,
	/* .tp_cmp           = */ &sew_cmp,
	/* .tp_seq           = */ &seo_seq,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sew_iter_next,
	/* .tp_attr          = */ &sew_attr,
	/* .tp_with          = */ &sew_with,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ seo_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ seo_class_members,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **__restrict, DeeObject *))&sew_call_kw,
};



/* Iterator support */
PRIVATE WUNUSED NONNULL((1)) int DCALL
seoi_ctor(SeqEachIterator *__restrict self) {
	self->ei_each = (DREF SeqEachBase *)DeeObject_NewDefault(&SeqEachOperator_Type);
	if unlikely(!self->ei_each)
		goto err;
	self->ei_iter = DeeObject_IterSelf(self->ei_each->se_seq);
	if unlikely(!self->ei_iter)
		goto err_each;
	return 0;
err_each:
	Dee_Decref(self->ei_each);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
seoi_init(SeqEachIterator *__restrict self,
          size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqEachOperatorIterator", &self->ei_each))
		goto err;
	if (DeeObject_AssertTypeExact(self->ei_each, &SeqEachOperator_Type))
		goto err;
	self->ei_iter = DeeObject_IterSelf(self->ei_each->se_seq);
	if unlikely(!self->ei_iter)
		goto err;
	Dee_Incref(self->ei_each);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sewi_copy(SeqEachIterator *__restrict self,
          SeqEachIterator *__restrict other) {
	self->ei_iter = DeeObject_Copy(other->ei_iter);
	if unlikely(!self->ei_iter)
		goto err;
	self->ei_each = other->ei_each;
	Dee_Incref(self->ei_each);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sewi_deep(SeqEachIterator *__restrict self,
          SeqEachIterator *__restrict other) {
	self->ei_iter = DeeObject_DeepCopy(other->ei_iter);
	if unlikely(!self->ei_iter)
		goto err;
	self->ei_each = (DREF SeqEachBase *)DeeObject_DeepCopy((DeeObject *)other->ei_each);
	if unlikely(!self->ei_each)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->ei_iter);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
sewi_fini(SeqEachIterator *__restrict self) {
	Dee_Decref(self->ei_iter);
	Dee_Decref(self->ei_each);
}

PRIVATE NONNULL((1, 2)) void DCALL
sewi_visit(SeqEachIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ei_iter);
	Dee_Visit(self->ei_each);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_bool(SeqEachIterator *__restrict self) {
	return DeeObject_Bool(self->ei_iter);
}

#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#define DEFINE_SEWI_COMPARE(name, func)                       \
	PRIVATE WUNUSED DREF DeeObject *DCALL                             \
	name(SeqEachIterator *__restrict self,                    \
	     SeqEachIterator *__restrict other) {                 \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self))) \
			goto err;                                         \
		return func(self->ei_iter, other->ei_iter);           \
	err:                                                      \
		return NULL;                                          \
	}
#else
#define DEFINE_SEWI_COMPARE(name, func)                                      \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                            \
	name(SeqEachIterator *__restrict self,                                   \
	     SeqEachIterator *__restrict other) {                                \
		if (DeeObject_AssertTypeExact(other, &SeqEachOperatorIterator_Type)) \
			goto err;                                                        \
		return func(self->ei_iter, other->ei_iter);                          \
	err:                                                                     \
		return NULL;                                                         \
	}
#endif
DEFINE_SEWI_COMPARE(sewi_eq, DeeObject_CompareEqObject)
DEFINE_SEWI_COMPARE(sewi_ne, DeeObject_CompareNeObject)
DEFINE_SEWI_COMPARE(sewi_lo, DeeObject_CompareLoObject)
DEFINE_SEWI_COMPARE(sewi_le, DeeObject_CompareLeObject)
DEFINE_SEWI_COMPARE(sewi_gr, DeeObject_CompareGrObject)
DEFINE_SEWI_COMPARE(sewi_ge, DeeObject_CompareGeObject)
#undef DEFINE_SEWI_COMPARE

PRIVATE WUNUSED NONNULL((1)) DREF SeqEachBase *DCALL
sewi_nii_getseq(SeqEachIterator *__restrict self) {
	return_reference_(self->ei_each);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sewi_nii_getindex(SeqEachIterator *__restrict self) {
	return DeeIterator_GetIndex(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_setindex(SeqEachIterator *__restrict self, size_t index) {
	return DeeIterator_SetIndex(self->ei_iter, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_rewind(SeqEachIterator *__restrict self) {
	return DeeIterator_Rewind(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_revert(SeqEachIterator *__restrict self, size_t skip) {
	return DeeIterator_Revert(self->ei_iter, skip);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_advance(SeqEachIterator *__restrict self, size_t skip) {
	return DeeIterator_Advance(self->ei_iter, skip);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_prev(SeqEachIterator *__restrict self) {
	return DeeIterator_Prev(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_next(SeqEachIterator *__restrict self) {
	return DeeIterator_Next(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sewi_nii_hasprev(SeqEachIterator *__restrict self) {
	return DeeIterator_HasPrev(self->ei_iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sewi_nii_peek(SeqEachIterator *__restrict self) {
	return DeeIterator_Peek(self->ei_iter);
}

PRIVATE struct type_nii sewi_nii = {
	/* .nii_class = */TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (void *)&sewi_nii_getseq,
			/* .nii_getindex = */ (void *)&sewi_nii_getindex,
			/* .nii_setindex = */ (void *)&sewi_nii_setindex,
			/* .nii_rewind   = */ (void *)&sewi_nii_rewind,
			/* .nii_revert   = */ (void *)&sewi_nii_revert,
			/* .nii_advance  = */ (void *)&sewi_nii_advance,
			/* .nii_prev     = */ (void *)&sewi_nii_prev,
			/* .nii_next     = */ (void *)&sewi_nii_next,
			/* .nii_hasprev  = */ (void *)&sewi_nii_hasprev,
			/* .nii_peek     = */ (void *)&sewi_nii_peek
		}
	}
};

PRIVATE struct type_cmp sewi_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sewi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sewi_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sewi_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sewi_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sewi_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sewi_ge,
	/* .tp_nii  = */ &sewi_nii
};

PRIVATE struct type_member seoi_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SeqEachIterator, ei_each), "->?Ert:SeqEachOperator"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SeqEachIterator, ei_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
seoi_next(SeqEachIterator *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_IterNext(self->ei_iter);
	if likely(ITER_ISOK(result))
		result = seo_transform((SeqEachOperator *)self->ei_each, result);
	return result;
}

INTERN DeeTypeObject SeqEachOperatorIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqEachOperatorIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqEachOperator)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&seoi_ctor,
				/* .tp_copy_ctor = */ (void *)&sewi_copy,
				/* .tp_deep_ctor = */ (void *)&sewi_deep,
				/* .tp_any_ctor  = */ (void *)&seoi_init,
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
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&seoi_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ seoi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
INTERN WUNUSED DREF DeeObject *DCALL
DeeSeqEach_CallAttr(DeeObject *__restrict self,
                    DeeObject *__restrict attr,
                    size_t argc,
                    DeeObject **argv) {
	size_t i;
	DREF SeqEachCallAttr *result;
	result = (DREF SeqEachCallAttr *)DeeObject_Malloc(COMPILER_OFFSETOF(SeqEachCallAttr, sg_argv) +
	                                                  (argc * sizeof(DREF DeeObject *)));
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = (DREF struct string_object *)attr;
	result->sg_argc = argc;
	MEMCPY_PTR(result->sg_argv, argv, argc);
	for (i = 0; i < argc; ++i)
		Dee_Incref(argv[i]);
	Dee_Incref(self);
	Dee_Incref(attr);
	DeeObject_Init(result, &SeqEachCallAttr_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeSeqEach_CallAttrKw(DeeObject *__restrict self,
                      DeeObject *__restrict attr,
                      size_t argc,
                      DeeObject **argv,
                      DeeObject *kw) {
	size_t i;
	DREF SeqEachCallAttrKw *result;
	if (!kw)
		return DeeSeqEach_CallAttr(self, attr, argc, argv);
	result = (DREF SeqEachCallAttrKw *)DeeObject_Malloc(COMPILER_OFFSETOF(SeqEachCallAttrKw, sg_argv) +
	                                                    (argc * sizeof(DREF DeeObject *)));
	if unlikely(!result)
		goto done;
	result->se_seq  = self;
	result->sg_attr = (DREF struct string_object *)attr;
	result->sg_kw   = kw;
	result->sg_argc = argc;
	MEMCPY_PTR(result->sg_argv, argv, argc);
	for (i = 0; i < argc; ++i)
		Dee_Incref(argv[i]);
	Dee_Incref(self);
	Dee_Incref(attr);
	Dee_Incref(kw);
	DeeObject_Init(result, &SeqEachCallAttrKw_Type);
done:
	return (DREF DeeObject *)result;
}


INTERN WUNUSED DREF DeeObject *DCALL
DeeSeqEach_CallAttrString(DeeObject *__restrict self,
                          char const *__restrict attr, dhash_t hash,
                          size_t argc, DeeObject **argv) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewWithHash(attr, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqEach_CallAttr(self,
	                             (DeeObject *)attr_ob,
	                             argc,
	                             argv);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeSeqEach_CallAttrStringLen(DeeObject *__restrict self,
                             char const *__restrict attr, size_t attrlen, dhash_t hash,
                             size_t argc, DeeObject **argv) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqEach_CallAttr(self,
	                             (DeeObject *)attr_ob,
	                             argc,
	                             argv);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeSeqEach_CallAttrStringKw(DeeObject *__restrict self,
                            char const *__restrict attr, dhash_t hash,
                            size_t argc, DeeObject **argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewWithHash(attr, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqEach_CallAttrKw(self,
	                               (DeeObject *)attr_ob,
	                               argc,
	                               argv,
	                               kw);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
DeeSeqEach_CallAttrStringLenKw(DeeObject *__restrict self,
                               char const *__restrict attr, size_t attrlen, dhash_t hash,
                               size_t argc, DeeObject **argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeStringObject *attr_ob;
	attr_ob = (DREF DeeStringObject *)DeeString_NewSizedWithHash(attr, attrlen, hash);
	if unlikely(!attr_ob)
		goto err;
	result = DeeSeqEach_CallAttrKw(self,
	                               (DeeObject *)attr_ob,
	                               argc,
	                               argv,
	                               kw);
	Dee_Decref_unlikely(attr_ob);
	return result;
err:
	return NULL;
}
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */

DECL_END

#ifndef __INTELLISENSE__
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#define DEFINE_GETATTR 1
#include "each-fastpass.c.inl"
#define DEFINE_CALLATTR 1
#include "each-fastpass.c.inl"
#define DEFINE_CALLATTRKW 1
#include "each-fastpass.c.inl"
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
#endif

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_EACH_C */
