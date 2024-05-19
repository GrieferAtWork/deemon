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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_RANGE_C
#define GUARD_DEEMON_OBJECTS_SEQ_RANGE_C 1

#include "range.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/overflow.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../gc_inspect.h"
#include "repeat.h"

DECL_BEGIN

/* Intended sequence behavior:
>> print repr([0:7] as Sequence);     // { 0, 1, 2, 3, 4, 5, 6 }
>> print repr([0:7, 1] as Sequence);  // { 0, 1, 2, 3, 4, 5, 6 }
>> print repr([0:7, 2] as Sequence);  // { 0, 2, 4, 6 }
>> print repr([0:7, 3] as Sequence);  // { 0, 3, 6 }
>> print repr([0:7, -1] as Sequence); // { }
>> print repr([0:7, -2] as Sequence); // { }
>> print repr([0:7, -3] as Sequence); // { }
>> print repr([7:0] as Sequence);     // { }
>> print repr([7:0, 1] as Sequence);  // { }
>> print repr([7:0, 2] as Sequence);  // { }
>> print repr([7:0, 3] as Sequence);  // { }
>> print repr([7:0, -1] as Sequence); // { 7, 6, 5, 4, 3, 2, 1 }
>> print repr([7:0, -2] as Sequence); // { 7, 5, 3, 1 }
>> print repr([7:0, -3] as Sequence); // { 7, 4, 1 }
Implementation:
function range(start, end, step?) {
	if (step !is bound) {
		while (start < end) {
			yield start;
			++start;
		}
	} else if (step < 0) {
		while (start > end) {
			yield start;
			start += step;
		}
	} else {
		while (start < end) {
			yield start;
			start += step;
		}
	}
}
*/


PRIVATE WUNUSED NONNULL((1)) int DCALL
ri_ctor(RangeIterator *__restrict self) {
	self->ri_range = (DREF Range *)DeeRange_New(Dee_None, Dee_None, NULL);
	if unlikely(!self->ri_range)
		goto err;
	self->ri_index = Dee_None;
	self->ri_end   = Dee_None;
	self->ri_step  = NULL;
	self->ri_first = true;
	Dee_Incref(Dee_None);
	Dee_atomic_rwlock_init(&self->ri_lock);
	return 0;
err:
	return -1;
}

INTDEF DeeTypeObject SeqRange_Type;

PRIVATE WUNUSED NONNULL((1)) int DCALL
ri_init(RangeIterator *__restrict self,
        size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqRangeIterator", &self->ri_range))
		goto err;
	if (DeeObject_AssertTypeExact(self->ri_range, &SeqRange_Type))
		goto err;
	self->ri_index = self->ri_range->r_start;
	self->ri_end   = self->ri_range->r_end;
	self->ri_step  = self->ri_range->r_step;
	Dee_Incref(self->ri_index);
	Dee_Incref(self->ri_range);
	self->ri_first = true;
	Dee_atomic_rwlock_init(&self->ri_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ri_copy(RangeIterator *__restrict self,
        RangeIterator *__restrict other) {
	DREF DeeObject *new_index, *old_index;
again:
	RangeIterator_LockRead(other);
	old_index      = other->ri_index;
	self->ri_first = other->ri_first;
	Dee_Incref(old_index);
	RangeIterator_LockEndRead(other);

	/* Create a copy of the index (may not be correct if it already changed) */
	new_index = DeeObject_Copy(old_index);
	Dee_Decref(old_index);
	if unlikely(!new_index)
		goto err;
	COMPILER_READ_BARRIER();
	if (old_index != other->ri_index) {
		Dee_Decref(new_index);
		/* Try to read the old index again.
		 * This can happen if the other iterator
		 * was spun while we were copying its index. */
		goto again;
	}
	Dee_atomic_rwlock_init(&self->ri_lock);

	/* Other members are constant, so we don't
	 * need to bother with synchronizing them. */
	self->ri_range = other->ri_range;
	self->ri_end   = other->ri_end;
	self->ri_step  = other->ri_step;
	Dee_Incref(self->ri_range);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ri_deep(RangeIterator *__restrict self,
        RangeIterator *__restrict other) {
	RangeIterator_LockRead(other);
	self->ri_range = other->ri_range;
	self->ri_index = other->ri_index;
	self->ri_first = other->ri_first;
	Dee_Incref(self->ri_range);
	Dee_Incref(self->ri_index);
	RangeIterator_LockEndRead(other);
	if (DeeObject_InplaceDeepCopy(&self->ri_index))
		goto err_r;
	if (DeeObject_InplaceDeepCopy((DeeObject **)&self->ri_range))
		goto err_r;
	Dee_atomic_rwlock_init(&self->ri_lock);
	self->ri_end  = self->ri_range->r_end;
	self->ri_step = self->ri_range->r_step;
	return 0;
err_r:
	Dee_Decref_unlikely(self->ri_range);
	Dee_Decref(self->ri_index);
/*err:*/
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
ri_fini(RangeIterator *__restrict self) {
	Dee_Decref(self->ri_index);
	Dee_Decref(self->ri_range);
}

PRIVATE NONNULL((1, 2)) void DCALL
ri_visit(RangeIterator *__restrict self,
         dvisit_t proc, void *arg) {
	RangeIterator_LockRead(self);
	Dee_Visit(self->ri_index);
	RangeIterator_LockEndRead(self);
	Dee_Visit(self->ri_range);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ri_next(RangeIterator *__restrict self) {
	DREF DeeObject *new_index;
	DREF DeeObject *old_index;
	int temp;
	bool is_first;
again:
	RangeIterator_LockRead(self);
	new_index = self->ri_index;
	old_index = self->ri_index;
	is_first  = self->ri_first;
	Dee_Incref(new_index);
	RangeIterator_LockEndRead(self);

	/* Skip the index modification on the first loop. */
	if (!is_first) {
		temp = self->ri_step
		       ? DeeObject_InplaceAdd(&new_index, self->ri_step)
		       : DeeObject_Inc(&new_index);
		if unlikely(temp)
			goto err_ni;
	}

	/* Check if the end has been reached */
	temp = likely(!self->ri_range->r_rev)
	       ? DeeObject_CmpLoAsBool(new_index, self->ri_end)
	       : DeeObject_CmpGrAsBool(new_index, self->ri_end);
	if (temp <= 0) {
		/* Error, or done. */
		if unlikely(temp < 0)
			goto err_ni;
		Dee_Decref(new_index);
		return ITER_DONE;
	}

	/* Save the new index object. */
	RangeIterator_LockWrite(self);
	if unlikely(self->ri_index != old_index ||
	            self->ri_first != is_first) {
		RangeIterator_LockEndWrite(self);
		Dee_Decref(new_index);
		goto again;
	}
	old_index = self->ri_index; /* Inherit reference. */
	Dee_Incref(new_index);
	self->ri_index = new_index;
	self->ri_first = false;
	RangeIterator_LockEndWrite(self);
	Dee_Decref(old_index); /* Decref() the old index. */
	return new_index;
err_ni:
	Dee_Decref(new_index);
/*err:*/
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ri_get_next_index(RangeIterator *__restrict self) {
	DREF DeeObject *new_index;
	DREF DeeObject *old_index;
	int temp;
again:
	RangeIterator_LockRead(self);
	new_index = old_index = self->ri_index;
	Dee_Incref(new_index);
	if (!self->ri_first) {
		RangeIterator_LockEndRead(self);
		temp = self->ri_step
		       ? DeeObject_InplaceAdd(&new_index, self->ri_step)
		       : DeeObject_Inc(&new_index);
		if unlikely(temp)
			goto err_r;
		/* Save the new index object. */
		RangeIterator_LockWrite(self);
		if unlikely(self->ri_index != old_index || self->ri_first) {
			RangeIterator_LockEndWrite(self);
			Dee_Decref(new_index);
			goto again;
		}
		old_index = self->ri_index; /* Inherit reference. */
		Dee_Incref(new_index);
		self->ri_index = new_index;
		self->ri_first = true;
		RangeIterator_LockEndWrite(self);
		Dee_Decref(old_index); /* Decref() the old index. */
	} else {
		RangeIterator_LockEndRead(self);
	}
	return new_index;
err_r:
	Dee_Decref(new_index);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ri_bool(RangeIterator *__restrict self) {
	int result;
	DREF DeeObject *ni;
	ni = ri_get_next_index(self);
	/* Check if the end has been reached */
	result = likely(!self->ri_range->r_rev)
	         ? DeeObject_CmpLoAsBool(ni, self->ri_end)
	         : DeeObject_CmpGrAsBool(ni, self->ri_end);
	Dee_Decref(ni);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ri_index_get(RangeIterator *__restrict self) {
	DREF DeeObject *result;
	RangeIterator_LockRead(self);
	result = self->ri_index;
	Dee_Incref(result);
	RangeIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ri_index_del(RangeIterator *__restrict self) {
	DREF DeeObject *old_index;
	RangeIterator_LockWrite(self);
	old_index = self->ri_index;
	/* Assign the original begin-index. */
	self->ri_index = self->ri_range->r_start;
	self->ri_first = true;
	Dee_Incref(self->ri_index);
	RangeIterator_LockEndWrite(self);
	Dee_Decref(old_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ri_index_set(RangeIterator *__restrict self,
             DeeObject *__restrict value) {
	DREF DeeObject *old_index;
	if (DeeGC_ReferredBy(value, (DeeObject *)self))
		return err_reference_loop((DeeObject *)self, value);
	/* XXX: Race condition: What if `value' starts referencing
	 *      us before we acquire the following lock? */
	RangeIterator_LockWrite(self);
	old_index = self->ri_index;
	/* Assign the given value. */
	self->ri_index = value;
	self->ri_first = true;
	Dee_Incref(value);
	RangeIterator_LockEndWrite(self);
	Dee_Decref(old_index);
	return 0;
}

PRIVATE struct type_getset tpconst ri_getsets[] = {
	TYPE_GETSET_F_NODOC(STR_index, &ri_index_get, &ri_index_del, &ri_index_set, METHOD_FNOREFESCAPE),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ri_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RangeIterator, ri_range), "->?Ert:SeqRange"),
	TYPE_MEMBER_FIELD("__end__", STRUCT_OBJECT, offsetof(RangeIterator, ri_end)),
	TYPE_MEMBER_FIELD("__step__", STRUCT_OBJECT, offsetof(RangeIterator, ri_step)),
	/* TODO: `__start__' is writeable when `ri_lock' is held! */
	TYPE_MEMBER_FIELD("__start__", STRUCT_CONST | STRUCT_CBOOL, offsetof(RangeIterator, ri_first)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ri_compare(RangeIterator *self, RangeIterator *other) {
	int result;
	DREF DeeObject *my_index, *ot_index;
	if (DeeObject_AssertTypeExact(other, &SeqRangeIterator_Type))
		goto err;
	my_index = ri_get_next_index(self);
	if unlikely(!my_index)
		goto err;
	ot_index = ri_get_next_index(other);
	if unlikely(!my_index)
		goto err_my_index;
	result = DeeObject_Compare(my_index, ot_index);
	Dee_Decref(ot_index);
	Dee_Decref(my_index);
	return result;
err_my_index:
	Dee_Decref(my_index);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ri_compare_eq(RangeIterator *self, RangeIterator *other) {
	int result;
	DREF DeeObject *my_index, *ot_index;
	if (DeeObject_AssertTypeExact(other, &SeqRangeIterator_Type))
		goto err;
	my_index = ri_get_next_index(self);
	if unlikely(!my_index)
		goto err;
	ot_index = ri_get_next_index(other);
	if unlikely(!my_index)
		goto err_my_index;
	result = DeeObject_CompareEq(my_index, ot_index);
	Dee_Decref(ot_index);
	Dee_Decref(my_index);
	return result;
err_my_index:
	Dee_Decref(my_index);
err:
	return Dee_COMPARE_ERR;
}


PRIVATE struct type_cmp ri_cmp = {
	/* .tp_hash       = */ NULL,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&ri_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ri_compare,
};


INTERN DeeTypeObject SeqRangeIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRangeIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqRange)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&ri_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&ri_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&ri_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&ri_init,
				TYPE_FIXED_ALLOCATOR(RangeIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ri_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&ri_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ri_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: bi-directional iterator support */
	/* .tp_cmp           = */ &ri_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ri_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ri_getsets,
	/* .tp_members       = */ ri_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE NONNULL((1)) void DCALL
range_fini(Range *__restrict self) {
	Dee_Decref(self->r_start);
	Dee_Decref(self->r_end);
	Dee_XDecref(self->r_step);
}

PRIVATE NONNULL((1, 2)) void DCALL
range_visit(Range *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->r_start);
	Dee_Visit(self->r_end);
	Dee_XVisit(self->r_step);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeIterator *DCALL
range_iter(Range *__restrict self) {
	DREF RangeIterator *result;
	result = DeeObject_MALLOC(RangeIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &SeqRangeIterator_Type);
	result->ri_index = self->r_start;
	result->ri_range = self;
	result->ri_end   = self->r_end;
	result->ri_step  = self->r_step;
	Dee_Incref(result->ri_index);
	Dee_Incref(result->ri_range);
	result->ri_first = true;
	Dee_atomic_rwlock_init(&result->ri_lock);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
range_contains(Range *self, DeeObject *index) {
	DREF DeeObject *temp, *temp2, *temp3;
	int error;
	if likely(!self->r_rev) {
		/* >> if (!(self->r_start <= index))
		 * >>     return false; */
		error = DeeObject_CmpLeAsBool(self->r_start, index);
		if (error <= 0) /* if false-or-error */
			goto err_or_false;

		/* >> if (self->r_end <= index)
		 * >>     return false; */
		error = DeeObject_CmpLeAsBool(self->r_end, index);
		if (error != 0) /* if true-or-error */
			goto err_or_false;
		if (self->r_step) {
			/* >> temp = index - self->r_start; */
			if (Dee_TYPE(index) == Dee_TYPE(self->r_start)) {
				temp = DeeObject_Sub(index, self->r_start);
			} else {
				temp = DeeObject_Sub(self->r_start, index);
				if unlikely(!temp)
					goto err;
				temp2 = DeeObject_Neg(temp);
				Dee_Decref(temp);
				temp = temp2;
			}
			if unlikely(!temp)
				goto err;

			/* >> temp = temp % self->r_step; */
			temp2 = DeeObject_Mod(temp, self->r_step);
			Dee_Decref(temp);
			if unlikely(!temp2)
				goto err;
			error = DeeObject_Bool(temp2);
			Dee_Decref(temp2);

			/* >> if ((index - self->r_start) % self->r_step)
			 * >>     return false; */
			if (error != 0) /* if true-or-error */
				goto err_or_false;
		}
	} else {
		ASSERT(self->r_step);

		/* >> if (self->r_start < index)
		 * >>     return false; */
		error = DeeObject_CmpLoAsBool(self->r_start, index);
		if (error != 0) /* if true-or-error */
			goto err_or_false;

		/* >> if (!(self->r_end < index))
		 * >>     return false; */
		error = DeeObject_CmpLoAsBool(self->r_end, index);
		if (error <= 0) /* if false-or-error */
			goto err_or_false;

		/* >> temp = self->r_start - index; */
		temp = DeeObject_Sub(self->r_start, index);
		if unlikely(!temp)
			goto err;
		temp3 = DeeObject_Neg(self->r_step);
		if unlikely(!temp3)
			goto err_temp;

		/* >> temp2 = (self->r_start - index) % -self->r_step; */
		temp2 = DeeObject_Mod(temp, temp3);
		Dee_Decref(temp3);
		Dee_Decref(temp);
		if unlikely(!temp2)
			goto err;
		error = DeeObject_Bool(temp2);
		Dee_Decref(temp2);

		/* >> if ((self->r_start - index) % -self->r_step)
		 * >>     return false; */
		if (error != 0) /* if false-or-error */
			goto err_or_false;
	}
	return_true;
err_or_false:
	if (error >= 0)
		return_false;
err:
	return NULL;
err_temp:
	Dee_Decref(temp);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
range_bool(Range *__restrict self) {
	/* >> if likely(!self->r_rev) {
	 * >>     return self->r_start < self->r_end;
	 * >> } else {
	 * >>     return self->r_start > self->r_end;
	 * >> }
	 */
	return likely(!self->r_rev)
	       ? DeeObject_CmpLoAsBool(self->r_start, self->r_end)
	       : DeeObject_CmpGrAsBool(self->r_start, self->r_end);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
range_size(Range *__restrict self) {
	/* >> local result = self->r_end - self->r_start;
	 * >> if (self->r_step) {
	 * >>     result = result + self->r_step;
	 * >>     if likely(!self->r_rev) {
	 * >>         --result;
	 * >>     } else {
	 * >>         ++result;
	 * >>     }
	 * >>     result = result / self->r_step;
	 * >> }
	 * >> if (result < 0)
	 * >>     result = type(result)();
	 * >> return result;
	 */
	DREF DeeObject *result, *temp;
	int error;
	result = DeeObject_Sub(self->r_end, self->r_start);
	if unlikely(!result)
		goto err;
	if (self->r_step) {
		temp = DeeObject_Add(result, self->r_step);
		if unlikely(!temp)
			goto err_r;
		Dee_Decref(result);
		error = likely(!self->r_rev)
		        ? DeeObject_Dec(&temp)
		        : DeeObject_Inc(&temp);
		if unlikely(error)
			goto err_temp;
		result = DeeObject_Div(temp, self->r_step);
		if unlikely(!result)
			goto err_temp;
		Dee_Decref(temp);
	}
	error = DeeObject_CmpLoAsBool(result, DeeInt_Zero);
	if unlikely(error != 0) {
		if unlikely(error < 0)
			goto err_r;
		temp = DeeObject_NewDefault(Dee_TYPE(result));
		Dee_Decref(result);
		result = temp;
	}
	return result;
err_temp:
	result = temp;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
range_getitem(Range *self, DeeObject *index) {
	/* >> local result;
	 * >> if (!self->r_step) {
	 * >>     result = self->r_start + index;
	 * >>do_compare_positive:
	 * >>     if (result >= self->r_end)
	 * >>         goto oob;
	 * >> } else {
	 * >>     result = self->r_step * index;
	 * >>     result = self->r_start + result;
	 * >>     if likely(!self->r_rev)
	 * >>        goto do_compare_positive;
	 * >>     if (result <= self->r_end)
	 * >>         goto oob;
	 * >> }
	 * >> return result;
	 * >>oob:
	 * >> throw IndexError("..."); */
	DREF DeeObject *result, *temp;
	int error;
	if (!self->r_step) {
		result = DeeObject_Add(self->r_start, index);
		if unlikely(!result)
			goto err;
do_compare_positive:
		error = DeeObject_CmpGeAsBool(result, self->r_end);
	} else {
		temp = DeeObject_Mul(self->r_step, index);
		if unlikely(!temp)
			goto err;
		result = DeeObject_Add(self->r_start, temp);
		Dee_Decref(temp);
		if unlikely(!result)
			goto err;
		if likely(!self->r_rev)
			goto do_compare_positive;
		error = DeeObject_CmpLeAsBool(result, self->r_end);
	}
	if unlikely(error != 0) {
		if unlikely(error < 0)
			goto err;
		goto oob;
	}
	return result;
oob:
	err_index_out_of_bounds_ob((DeeObject *)self, index);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
range_getrange(Range *self,
               DeeObject *start,
               DeeObject *end) {
	int error;
	DREF Range *result;
	DREF DeeObject *new_start, *new_end, *temp;
	DREF DeeObject *mylen;
	if (DeeNone_Check(start)) {
		if (DeeNone_Check(end))
			return_reference_((DeeObject *)self);
		/* if (end < 0) {
		 *     if (self->r_step) {
		 *         new_end = self->r_step * end;
		 *         new_end = self->r_end + new_end;
		 *     } else {
		 *         new_end = self->r_end + end;
		 *     }
		 *     if (self->r_start >= new_end)
		 *         return_reference_(Dee_EmptySeq);
		 * } else {
		 *     mylen = range_size(self);
		 *     if (mylen >= end)
		 *         return_reference_((DeeObject *)self);
		 *     if (self->r_step) {
		 *         new_end = self->r_step * end;
		 *         new_end = self->r_start + new_end;
		 *     } else {
		 *         new_end = self->r_start + end;
		 *     }
		 * }
		 * new_start = self->r_start;
		 * goto got_ns_ne;
		 */
		error = DeeObject_CmpLoAsBool(end, DeeInt_Zero);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			if (self->r_step) {
				temp = DeeObject_Mul(self->r_step, end);
				if unlikely(!temp)
					goto err;
				new_end = DeeObject_Add(self->r_end, temp);
				Dee_Decref(temp);
			} else {
				new_end = DeeObject_Add(self->r_end, end);
			}
		} else {
			mylen = range_size(self);
			if unlikely(!mylen)
				goto err;
			error = DeeObject_CmpGeAsBool(mylen, end);
			if (error != 0) {
				if unlikely(error < 0)
					goto err;
				return_reference_((DeeObject *)self);
			}
			if (self->r_step) {
				temp = DeeObject_Mul(self->r_step, end);
				if unlikely(!temp)
					goto err;
				new_end = DeeObject_Add(self->r_start, temp);
				Dee_Decref(temp);
			} else {
				new_end = DeeObject_Add(self->r_start, end);
			}
		}
		if unlikely(!new_end)
			goto err;
		new_start = self->r_start;
		Dee_Incref(new_start);
		goto got_ns_ne;
	}
	mylen = range_size(self);
	if unlikely(!mylen)
		goto err;
	/* if (start < 0)
	 *     new_start = mylen + start;
	 * else {
	 *     new_start = start;
	 * }
	 */
	error = DeeObject_CmpLoAsBool(start, DeeInt_Zero);
	if (error != 0) {
		if unlikely(error < 0)
			goto err_mylen;
		new_start = DeeObject_Add(mylen, start);
		if unlikely(!new_start)
			goto err_mylen;
	} else {
		new_start = start;
		Dee_Incref(start);
	}
	if (DeeNone_Check(end)) {
reuse_old_end:
		/* if (mylen <= new_start)
		 *     return_reference_(Dee_EmptySeq);
		 */
		error = DeeObject_CmpLeAsBool(mylen, new_start);
		if (error != 0) {
			if unlikely(error < 0)
				goto err_mylen_ns;
return_empty_seq_mylen_ns:
			Dee_Decref(new_start);
			Dee_Decref(mylen);
			return_reference_(Dee_EmptySeq);
		}
		/* Re-use the old end pointer. */
		new_end = self->r_end;
		Dee_Incref(new_end);
	} else {
		/* if (end < 0)
		 *     new_end = mylen + end;
		 * else {
		 *     new_end = end;
		 * }
		 */
		error = DeeObject_CmpLoAsBool(end, DeeInt_Zero);
		if (error != 0) {
			if unlikely(error < 0)
				goto err_mylen_ns;
			new_end = DeeObject_Add(mylen, end);
			if unlikely(!new_end)
				goto err_mylen_ns;
		} else {
			/* if (mylen <= new_end)
			 *     goto reuse_old_end;
			 */
			error = DeeObject_CmpLeAsBool(mylen, end);
			if (error != 0) {
				if unlikely(error < 0)
					goto err_mylen_ns;
				goto reuse_old_end;
			}
			new_end = end;
			Dee_Incref(end);
		}
		/* if (new_start >= new_end)
		 *     return_reference_(Dee_EmptySeq);
		 * new_end = mylen - new_end;
		 * if (self->r_step)
		 *     new_end = new_end * self->r_step;
		 * new_end = self->r_end - new_end;
		 */
		error = new_end == end
		        ? DeeObject_CmpGeAsBool(new_start, new_end)
		        : DeeObject_CmpLeAsBool(new_end, new_start);
		if (error != 0) {
			if unlikely(error < 0)
				goto err_mylen_ns_ne;
			Dee_Decref(new_end);
			goto return_empty_seq_mylen_ns;
		}
		/* new_end = mylen - new_end; */
		temp = DeeObject_Sub(mylen, new_end);
		if unlikely(!temp)
			goto err_mylen_ns_ne;
		Dee_Decref(new_end);
		new_end = temp;
		/* if (self->r_step)
		 *     new_end = new_end * self->r_step;
		 */
		if (self->r_step) {
			temp = DeeObject_Mul(new_end, self->r_step);
			if unlikely(!temp)
				goto err_mylen_ns_ne;
			Dee_Decref(new_end);
			new_end = temp;
		}
		/* new_end = self->r_end - new_end; */
		temp = DeeObject_Sub(self->r_end, new_end);
		if unlikely(!temp)
			goto err_mylen_ns_ne;
		Dee_Decref(new_end);
		new_end = temp;
	}
	Dee_Decref(mylen);
	/* if (self->r_step)
	 *     new_start = self->r_step * new_start;
	 * new_start = self->r_start + new_start;
	 */
	if (self->r_step) {
		temp = DeeObject_Mul(self->r_step, new_start);
		if unlikely(!temp)
			goto err_ns_ne;
		Dee_Decref(new_start);
		new_start = temp;
	}
	/* new_start = self->r_start + new_start; */
	temp = DeeObject_Add(self->r_start, new_start);
	if unlikely(!temp)
		goto err_ns_ne;
	Dee_Decref(new_start);
	new_start = temp;

got_ns_ne:
	/* Pack together the new range object. */
	result = DeeObject_MALLOC(Range);
	if unlikely(!result)
		goto err_ns_ne;
	result->r_start = new_start; /* Inherit reference */
	result->r_end   = new_end;   /* Inherit reference */
	result->r_rev   = self->r_rev;
	result->r_step  = self->r_step;
	Dee_XIncref(result->r_step);
	DeeObject_Init(result, &SeqRange_Type);
	return (DREF DeeObject *)result;
err_ns_ne:
	Dee_Decref(new_end);
	Dee_Decref(new_start);
	goto err;
err_mylen_ns_ne:
	Dee_Decref(new_end);
err_mylen_ns:
	Dee_Decref(new_start);
err_mylen:
	Dee_Decref(mylen);
err:
	return NULL;
}

PRIVATE struct type_seq range_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&range_iter,
	/* .tp_sizeob   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&range_size,
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&range_contains,
	/* .tp_getitem  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&range_getitem,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&range_getrange,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL,
	/* .tp_nsi      = */ NULL,
	/* .tp_foreach  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))NULL, /* TODO */
};

PRIVATE struct type_member tpconst range_members[] = {
	TYPE_MEMBER_FIELD("start", STRUCT_OBJECT, offsetof(Range, r_start)),
	TYPE_MEMBER_FIELD("end", STRUCT_OBJECT, offsetof(Range, r_end)),
	TYPE_MEMBER_FIELD("step", STRUCT_OBJECT, offsetof(Range, r_step)),
	TYPE_MEMBER_FIELD("__rev__", STRUCT_CONST | STRUCT_CBOOL, offsetof(Range, r_rev)),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst range_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst range_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqRangeIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqRange_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
range_printrepr(Range *__restrict self,
                dformatprinter printer, void *arg) {
	return self->r_step
	       ? DeeFormat_Printf(printer, arg, "[%r:%r, %r]", self->r_start, self->r_end, self->r_step)
	       : DeeFormat_Printf(printer, arg, "[%r:%r]", self->r_start, self->r_end);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
range_ctor(Range *__restrict self) {
	self->r_start = Dee_None;
	self->r_end   = Dee_None;
	self->r_step  = NULL;
	self->r_rev   = false;
	Dee_Incref_n(Dee_None, 2);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
range_copy(Range *__restrict self,
           Range *__restrict other) {
	self->r_start = other->r_start;
	self->r_end   = other->r_end;
	self->r_step  = other->r_step;
	self->r_rev   = other->r_rev;
	Dee_Incref(self->r_start);
	Dee_Incref(self->r_end);
	Dee_XIncref(self->r_step);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
range_deep(Range *__restrict self,
           Range *__restrict other) {
	self->r_start = DeeObject_DeepCopy(other->r_start);
	if unlikely(!self->r_start)
		goto err;
	self->r_end = DeeObject_DeepCopy(other->r_end);
	if unlikely(!self->r_end)
		goto err_begin;
	self->r_step = NULL;
	if (other->r_step) {
		int temp;
		self->r_step = DeeObject_DeepCopy(other->r_step);
		if unlikely(!self->r_step)
			goto err_end;
		temp = DeeObject_CmpLoAsBool(self->r_step, DeeInt_Zero);
		if unlikely(temp < 0)
			goto err_step;
		self->r_rev = !!temp;
	}
	return 0;
err_step:
	Dee_Decref(self->r_step);
err_end:
	Dee_Decref(self->r_end);
err_begin:
	Dee_Decref(self->r_start);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
range_init(Range *__restrict self,
           size_t argc, DeeObject *const *argv) {
	self->r_step = NULL;
	if (DeeArg_Unpack(argc, argv, "oo|o:_SeqRange",
	                  &self->r_start,
	                  &self->r_end,
	                  &self->r_step))
		goto err;
	if (self->r_step) {
		int temp;
		temp = DeeObject_CmpLoAsBool(self->r_step, DeeInt_Zero);
		if unlikely(temp < 0)
			goto err;
		self->r_rev = !!temp;
		Dee_Incref(self->r_step);
	}
	Dee_Incref(self->r_end);
	Dee_Incref(self->r_start);
	return 0;
err:
	return -1;
}


INTERN DeeTypeObject SeqRange_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRange",
	/* .tp_doc      = */ DOC("()\n"
	                         "(start,end,step?)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&range_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&range_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&range_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&range_init,
				TYPE_FIXED_ALLOCATOR(Range)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&range_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&range_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&range_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&range_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &range_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ range_getsets,
	/* .tp_members       = */ range_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ range_class_members
};



#define READ_INDEX(x) atomic_read(&(x)->iri_index)

PRIVATE WUNUSED NONNULL((1)) int DCALL
iri_ctor(IntRangeIterator *__restrict self) {
	self->iri_range = DeeObject_MALLOC(IntRange);
	if unlikely(!self->iri_range)
		goto err;
	DeeObject_Init(self->iri_range, &SeqIntRange_Type);
	self->iri_range->ir_start = 0;
	self->iri_range->ir_end   = 0;
	self->iri_range->ir_step  = 0;
	self->iri_index           = 0;
	self->iri_end             = 0;
	self->iri_step            = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
iri_copy(IntRangeIterator *__restrict self,
         IntRangeIterator *__restrict other) {
	self->iri_index = READ_INDEX(other);
	self->iri_end   = other->iri_end;
	self->iri_step  = other->iri_step;
	self->iri_range = other->iri_range;
	Dee_Incref(self->iri_range);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
iri_init(IntRangeIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqIntRangeIterator", &self->iri_range))
		goto err;
	if (DeeObject_AssertTypeExact(self->iri_range, &SeqIntRange_Type))
		goto err;
	self->iri_end   = self->iri_range->ir_end;
	self->iri_step  = self->iri_range->ir_step;
	self->iri_index = self->iri_range->ir_start;
	Dee_Incref(self->iri_range);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
iri_bool(IntRangeIterator *__restrict self) {
	Dee_ssize_t new_index;
	Dee_ssize_t index = READ_INDEX(self);
	return !(OVERFLOW_SADD(index, self->iri_step, &new_index) ||
	         (likely(self->iri_step >= 0) ? index >= self->iri_end
	                                      : index <= self->iri_end));
}

PRIVATE NONNULL((1)) void DCALL
iri_fini(IntRangeIterator *__restrict self) {
	Dee_Decref(self->iri_range);
}

PRIVATE NONNULL((1, 2)) void DCALL
iri_visit(IntRangeIterator *__restrict self,
          dvisit_t proc, void *arg) {
	Dee_Visit(self->iri_range);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iri_next(IntRangeIterator *__restrict self) {
	Dee_ssize_t result_index, new_index;
	do {
		result_index = READ_INDEX(self);
		/* Test for overflow/iteration done. */
		if (OVERFLOW_SADD(result_index, self->iri_step, &new_index) ||
		    (likely(self->iri_step >= 0) ? result_index >= self->iri_end
		                                 : result_index <= self->iri_end))
			return ITER_DONE;
	} while (!atomic_cmpxch_weak_or_write(&self->iri_index, result_index, new_index));

	/* Return a new integer for the resulting index. */
	return DeeInt_NewSSize(result_index);
}

PRIVATE struct type_member tpconst iri_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(IntRangeIterator, iri_range), "->?Ert:SeqIntRange"),
	/* We allow write-access to these members because doing so doesn't
	 * actually harm anything, although fiddling with this stuff may
	 * break some weak expectations but should never crash anything! */
	TYPE_MEMBER_FIELD("__index__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRangeIterator, iri_index)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRangeIterator, iri_end)),
	TYPE_MEMBER_FIELD("__step__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRangeIterator, iri_step)),
	TYPE_MEMBER_END
};



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
iri_compare(IntRangeIterator *lhs, IntRangeIterator *rhs) {
	Dee_ssize_t lhs_index, rhs_index;
	if (DeeObject_AssertTypeExact(rhs, &SeqIntRangeIterator_Type))
		goto err;
	lhs_index = READ_INDEX(lhs);
	rhs_index = READ_INDEX(rhs);
	return Dee_Compare(lhs_index, rhs_index);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp iri_cmp = {
	/* .tp_hash       = */ NULL,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&iri_compare,
};



INTERN DeeTypeObject SeqIntRangeIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqIntRangeIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqIntRange)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&iri_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&iri_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&iri_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&iri_init,
				TYPE_FIXED_ALLOCATOR(IntRangeIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&iri_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&iri_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&iri_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: bi-directional iterator support */
	/* .tp_cmp           = */ &iri_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&iri_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ iri_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF IntRangeIterator *DCALL
intrange_iter(IntRange *__restrict self) {
	DREF IntRangeIterator *result;
	result = DeeObject_MALLOC(IntRangeIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &SeqIntRangeIterator_Type);
	result->iri_range = self;
	result->iri_step  = self->ir_step;
	result->iri_end   = self->ir_end;
	result->iri_index = self->ir_start;
	Dee_Incref(self);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
intrange_contains(IntRange *self,
                  DeeObject *other) {
	Dee_ssize_t index;
	if (DeeObject_AsSSize(other, &index))
		goto err;
	if (self->ir_step >= 0) {
		return_bool(index >= self->ir_start &&
		            index < self->ir_end &&
		            ((index - self->ir_start) % self->ir_step) == 0);
	} else {
		return_bool(index <= self->ir_start &&
		            index > self->ir_end &&
		            ((self->ir_start - index) % -self->ir_step) == 0);
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
intrange_sizeob(IntRange *__restrict self) {
	Dee_ssize_t result;
	ASSERT(self->ir_step != 0);
	result = (self->ir_end - self->ir_start) + self->ir_step;
	if (self->ir_step >= 0) {
		--result;
	} else {
		++result;
	}
	result /= self->ir_step;
	if (result < 0)
		result = 0;
	return DeeInt_NewSize((size_t)result);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
intrange_size(IntRange *__restrict self) {
	Dee_ssize_t result;
	ASSERT(self->ir_step != 0);
	result = (self->ir_end - self->ir_start) + self->ir_step;
	if (self->ir_step >= 0) {
		--result;
	} else {
		++result;
	}
	result /= self->ir_step;
	if (result < 0)
		result = 0;
	return (size_t)result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
intrange_getitem_index(IntRange *__restrict self, size_t index) {
	Dee_ssize_t result;
	/* Check for overflows in this arithmetic */
	if (OVERFLOW_SMUL(self->ir_step, index, &result) ||
	    OVERFLOW_SADD(self->ir_start, result, &result))
		goto oob;
	if (self->ir_step >= 0
	    ? result >= self->ir_end
	    : result <= self->ir_end)
		goto oob;
	return DeeInt_NewSSize(result);
oob:
	err_index_out_of_bounds((DeeObject *)self, index,
	                        intrange_size(self));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
intrange_getitem(IntRange *self,
                 DeeObject *index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return intrange_getitem_index(self, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
intrange_getrange_index(IntRange *__restrict self,
                      Dee_ssize_t begin, Dee_ssize_t end) {
	struct Dee_seq_range range;
	size_t mylen = intrange_size(self);
	DeeSeqRange_Clamp(&range, begin, end, mylen);
	if (range.sr_end <= range.sr_start)
		return_reference_(Dee_EmptySeq);
	return DeeRange_NewInt(self->ir_start + (range.sr_start * self->ir_step),
	                       self->ir_end - ((mylen - range.sr_end) * self->ir_step),
	                       self->ir_step);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
intrange_getrange_index_n(IntRange *__restrict self, Dee_ssize_t begin) {
	size_t start;
	size_t mylen = intrange_size(self);
	start = DeeSeqRange_Clamp_n(begin, mylen);
	if (start >= mylen)
		return_reference_(Dee_EmptySeq);
	return DeeRange_NewInt(self->ir_start + (start * self->ir_step),
	                       self->ir_end,
	                       self->ir_step);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
intrange_getrange(IntRange *__restrict self,
                  DeeObject *__restrict start_ob,
                  DeeObject *__restrict end_ob) {
	Dee_ssize_t start_index, end_index;
	if (DeeObject_AsSSize(start_ob, &start_index))
		goto err;
	if (DeeNone_Check(end_ob))
		return intrange_getrange_index_n(self, start_index);
	if (DeeObject_AsSSize(end_ob, &end_index))
		goto err;
	return intrange_getrange_index(self, start_index, end_index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
intrange_foreach(IntRange *__restrict self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	Dee_ssize_t i = self->ir_start;
	if (self->ir_step >= 0) {
		while (i < self->ir_end) {
			DREF DeeObject *elem;
			elem = DeeInt_NewSSize(i);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
			i += self->ir_step;
		}
	} else {
		while (i > self->ir_end) {
			DREF DeeObject *elem;
			elem = DeeInt_NewSSize(i);
			if unlikely(!elem)
				goto err;
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
			i += self->ir_step;
		}
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}




PRIVATE struct type_nsi tpconst intrange_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&intrange_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&intrange_size,
			/* .nsi_getitem      = */ (dfunptr_t)&intrange_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)&intrange_getrange_index,
			/* .nsi_getrange_n   = */ (dfunptr_t)&intrange_getrange_index_n,
		}
	}
};

PRIVATE struct type_seq intrange_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&intrange_iter,
	/* .tp_sizeob             = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&intrange_sizeob,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&intrange_contains,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&intrange_getitem,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&intrange_getrange,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ &intrange_nsi,
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&intrange_foreach,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ NULL,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&intrange_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&intrange_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&intrange_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&intrange_getrange_index,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&intrange_getrange_index_n,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_trygetitem         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&intrange_getitem,
};


PRIVATE struct type_member tpconst intrange_members[] = {
	TYPE_MEMBER_FIELD("start", STRUCT_ATOMIC | STRUCT_CONST | STRUCT_SSIZE_T, offsetof(IntRange, ir_start)),
	TYPE_MEMBER_FIELD("end", STRUCT_ATOMIC | STRUCT_CONST | STRUCT_SSIZE_T, offsetof(IntRange, ir_end)),
	TYPE_MEMBER_FIELD("step", STRUCT_ATOMIC | STRUCT_CONST | STRUCT_SSIZE_T, offsetof(IntRange, ir_step)),
	TYPE_MEMBER_FIELD("__start__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRange, ir_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRange, ir_end)),
	TYPE_MEMBER_FIELD("__step__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRange, ir_step)),
	TYPE_MEMBER_END
};

#define intrange_getsets range_getsets
PRIVATE struct type_member tpconst intrange_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqIntRangeIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqIntRange_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
intrange_printrepr(IntRange *__restrict self,
                   dformatprinter printer, void *arg) {
	if (self->ir_step != 1) {
		return DeeFormat_Printf(printer, arg,
		                        "[%" PRFdSIZ ":%" PRFdSIZ ", %" PRFdSIZ "]",
		                        self->ir_start, self->ir_end, self->ir_step);
	} else {
		return DeeFormat_Printf(printer, arg,
		                        "[%" PRFdSIZ ":%" PRFdSIZ "]",
		                        self->ir_start, self->ir_end);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
intrange_bool(IntRange *__restrict self) {
	return self->ir_step >= 0
	       ? self->ir_start < self->ir_end
	       : self->ir_start > self->ir_end;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
intrange_ctor(IntRange *__restrict self) {
	self->ir_start = 0;
	self->ir_end   = 0;
	self->ir_step  = 1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
intrange_copy(IntRange *__restrict self,
              IntRange *__restrict other) {
	self->ir_start = other->ir_start;
	self->ir_end   = other->ir_end;
	self->ir_step  = other->ir_step;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
intrange_init(IntRange *__restrict self,
              size_t argc, DeeObject *const *argv) {
	self->ir_step = 1;
	if (DeeArg_Unpack(argc, argv, UNPdSIZ UNPdSIZ "|" UNPdSIZ ":_SeqIntRange",
	                  &self->ir_start,
	                  &self->ir_end,
	                  &self->ir_step))
		goto err;
	if unlikely(self->ir_step == 0) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot used `0' as step for _SeqIntRange");
	}
	return 0;
err:
	return -1;
}


INTERN DeeTypeObject SeqIntRange_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqIntRange",
	/* .tp_doc      = */ DOC("()\n"
	                         "(start:?Dint,end:?Dint,step=!1)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&intrange_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&intrange_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&intrange_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&intrange_init,
				TYPE_FIXED_ALLOCATOR(IntRange)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&intrange_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&intrange_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &intrange_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ intrange_getsets,
	/* .tp_members       = */ intrange_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ intrange_class_members
};














/* Test to force use of object-based ranges. */
#undef ALWAYS_USE_OBJECT_RANGES
/* #define ALWAYS_USE_OBJECT_RANGES 1 */


/* Create new range sequence objects. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeRange_NewInt(Dee_ssize_t begin,
                Dee_ssize_t end,
                Dee_ssize_t step) {
#ifdef ALWAYS_USE_OBJECT_RANGES
	DREF DeeObject *begin_ob, *end_ob, *step_ob, *result;
	ASSERT(step != 0);
	begin_ob = DeeInt_NewSSize(begin);
	if unlikely(!begin_ob)
		goto err;
	end_ob = DeeInt_NewSSize(end);
	if unlikely(!end_ob)
		goto err_begin_ob;
	step_ob = DeeInt_NewSSize(step);
	if unlikely(!step_ob)
		goto err_begin_ob_end_ob;
	result = DeeRange_New(begin_ob, end_ob, step_ob);
	Dee_Decref(step_ob);
	Dee_Decref(end_ob);
	Dee_Decref(begin_ob);
	return result;
err_begin_ob_end_ob:
	Dee_Decref(end_ob);
err_begin_ob:
	Dee_Decref(begin_ob);
err:
	return NULL;
#else /* ALWAYS_USE_OBJECT_RANGES */
	DREF IntRange *result;
	ASSERT(step != 0);

	/* Create the new range. */
	result = DeeObject_MALLOC(IntRange);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &SeqIntRange_Type);
	/* Fill in members of the new range object. */
	result->ir_start = begin;
	result->ir_end   = end;
	result->ir_step  = step;
done:
	return (DREF DeeObject *)result;
#endif /* !ALWAYS_USE_OBJECT_RANGES */
}

/* Create new range sequence objects. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeRange_New(DeeObject *begin,
             DeeObject *end,
             DeeObject *step) {
	DREF Range *result;
	int temp;
	ASSERT_OBJECT(begin);
	ASSERT_OBJECT(end);
	ASSERT_OBJECT_OPT(step);

	/* Check for special optimizations for the likely case of int-only arguments. */
#ifndef ALWAYS_USE_OBJECT_RANGES
	{
		Dee_ssize_t i_begin, i_end, i_step;
		if (DeeInt_Check(begin) && DeeInt_Check(end) &&
		    DeeInt_TryAsSSize(begin, &i_begin) &&
		    DeeInt_TryAsSSize(end, &i_end)) {
			i_step = 1;
			if (step) {
				if (!DeeInt_Check(step) || !DeeInt_TryAsSSize(step, &i_step))
					goto do_object_range;
				if unlikely(!step) {
					if (i_begin >= i_end)
						return_reference_(Dee_EmptySeq);
					return DeeSeq_RepeatItemForever(begin);
				}
			}

			/* Create an integer-based range. */
			return DeeRange_NewInt(i_begin, i_end, i_step);
		}
	}
do_object_range:
#endif /* !ALWAYS_USE_OBJECT_RANGES */
	temp = 0;

	/* Check if `step' is negative (required for proper compare operations of the range iterator). */
	if (step) {
		temp = DeeObject_CmpLoAsBool(step, DeeInt_Zero);
		if unlikely(temp < 0)
			goto err;
	}

	/* Create the new range. */
	result = DeeObject_MALLOC(Range);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &SeqRange_Type);

	/* Fill in members of the new range object. */
	result->r_start = begin;
	result->r_end   = end;
	result->r_step  = step;
	result->r_rev   = !!temp;
	Dee_Incref(begin);
	Dee_Incref(end);
	Dee_XIncref(step);
done:
	return (DREF DeeObject *)result;
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_RANGE_C */
