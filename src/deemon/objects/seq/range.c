/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
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
#include <deemon/util/rwlock.h>

#include <hybrid/atomic.h>
#include <hybrid/overflow.h>

#include "../../runtime/runtime_error.h"
#include "../gc_inspect.h"

DECL_BEGIN

/* Intended sequence behavior:
>> print repr([0:7] as sequence);    // { 0, 1, 2, 3, 4, 5, 6 }
>> print repr([0:7,1] as sequence);  // { 0, 1, 2, 3, 4, 5, 6 }
>> print repr([0:7,2] as sequence);  // { 0, 2, 4, 6 }
>> print repr([0:7,3] as sequence);  // { 0, 3, 6 }
>> print repr([0:7,-1] as sequence); // { }
>> print repr([0:7,-2] as sequence); // { }
>> print repr([0:7,-3] as sequence); // { }
>> print repr([7:0] as sequence);    // { }
>> print repr([7:0,1] as sequence);  // { }
>> print repr([7:0,2] as sequence);  // { }
>> print repr([7:0,3] as sequence);  // { }
>> print repr([7:0,-1] as sequence); // { 7, 6, 5, 4, 3, 2, 1 }
>> print repr([7:0,-2] as sequence); // { 7, 5, 3, 1 }
>> print repr([7:0,-3] as sequence); // { 7, 4, 1 }
Implementation:
function range(start,end,step?) {
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
	rwlock_init(&self->ri_lock);
	return 0;
err:
	return -1;
}

INTDEF DeeTypeObject SeqRange_Type;

PRIVATE WUNUSED NONNULL((1)) int DCALL
ri_init(RangeIterator *__restrict self,
        size_t argc, DeeObject **argv) {
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
	rwlock_init(&self->ri_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ri_copy(RangeIterator *__restrict self,
        RangeIterator *__restrict other) {
	DREF DeeObject *new_index, *old_index;
again:
	rwlock_read(&other->ri_lock);
	old_index      = other->ri_index;
	self->ri_first = other->ri_first;
	Dee_Incref(old_index);
	rwlock_endread(&other->ri_lock);
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
	rwlock_init(&self->ri_lock);
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
	rwlock_read(&other->ri_lock);
	self->ri_range = other->ri_range;
	self->ri_index = other->ri_index;
	self->ri_first = other->ri_first;
	Dee_Incref(self->ri_range);
	Dee_Incref(self->ri_index);
	rwlock_endread(&other->ri_lock);
	if (DeeObject_InplaceDeepCopy(&self->ri_index))
		goto err_r;
	if (DeeObject_InplaceDeepCopy((DeeObject **)&self->ri_range))
		goto err_r;
	rwlock_init(&self->ri_lock);
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
	rwlock_read(&self->ri_lock);
	Dee_Visit(self->ri_index);
	rwlock_endread(&self->ri_lock);
	Dee_Visit(self->ri_range);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ri_next(RangeIterator *__restrict self) {
	DREF DeeObject *new_index;
	DREF DeeObject *old_index;
	int temp;
	bool is_first;
again:
	rwlock_read(&self->ri_lock);
	new_index = old_index = self->ri_index;
	is_first              = self->ri_first;
	Dee_Incref(new_index);
	rwlock_endread(&self->ri_lock);
	/* Skip the index modification on the first loop. */
	if (!is_first) {
		temp = self->ri_step
		       ? DeeObject_InplaceAdd(&new_index, self->ri_step)
		       : DeeObject_Inc(&new_index);
		if unlikely(temp)
			goto err_ni;
	}
	/* Check if the end has been reached */
	temp = (likely(!self->ri_range->r_rev))
	       ? DeeObject_CompareLo(new_index, self->ri_end)
	       : DeeObject_CompareGr(new_index, self->ri_end);
	if (temp <= 0) {
		/* Error, or done. */
		if unlikely(temp < 0)
			goto err_ni;
		Dee_Decref(new_index);
		return ITER_DONE;
	}
	/* Save the new index object. */
	rwlock_write(&self->ri_lock);
	if unlikely(self->ri_index != old_index ||
		         self->ri_first != is_first)
	{
		rwlock_endwrite(&self->ri_lock);
		Dee_Decref(new_index);
		goto again;
	}
	old_index = self->ri_index; /* Inherit reference. */
	Dee_Incref(new_index);
	self->ri_index = new_index;
	self->ri_first = false;
	rwlock_endwrite(&self->ri_lock);
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
	rwlock_read(&self->ri_lock);
	new_index = old_index = self->ri_index;
	Dee_Incref(new_index);
	if (!self->ri_first) {
		rwlock_endread(&self->ri_lock);
		temp = self->ri_step
		       ? DeeObject_InplaceAdd(&new_index, self->ri_step)
		       : DeeObject_Inc(&new_index);
		if unlikely(temp)
			goto err_r;
		/* Save the new index object. */
		rwlock_write(&self->ri_lock);
		if unlikely(self->ri_index != old_index || self->ri_first) {
			rwlock_endwrite(&self->ri_lock);
			Dee_Decref(new_index);
			goto again;
		}
		old_index = self->ri_index; /* Inherit reference. */
		Dee_Incref(new_index);
		self->ri_index = new_index;
		self->ri_first = true;
		rwlock_endwrite(&self->ri_lock);
		Dee_Decref(old_index); /* Decref() the old index. */
	} else {
		rwlock_endread(&self->ri_lock);
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
	result = (likely(!self->ri_range->r_rev))
	         ? DeeObject_CompareLo(ni, self->ri_end)
	         : DeeObject_CompareGr(ni, self->ri_end);
	Dee_Decref(ni);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ri_index_get(RangeIterator *__restrict self) {
	DREF DeeObject *result;
	rwlock_read(&self->ri_lock);
	result = self->ri_index;
	Dee_Incref(result);
	rwlock_endread(&self->ri_lock);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ri_index_del(RangeIterator *__restrict self) {
	DREF DeeObject *old_index;
	rwlock_write(&self->ri_lock);
	old_index = self->ri_index;
	/* Assign the original begin-index. */
	self->ri_index = self->ri_range->r_start;
	self->ri_first = true;
	Dee_Incref(self->ri_index);
	rwlock_endwrite(&self->ri_lock);
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
	rwlock_write(&self->ri_lock);
	old_index = self->ri_index;
	/* Assign the given value. */
	self->ri_index = value;
	self->ri_first = true;
	Dee_Incref(value);
	rwlock_endwrite(&self->ri_lock);
	Dee_Decref(old_index);
	return 0;
}

PRIVATE struct type_getset ri_getsets[] = {
	{ "index",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ri_index_get,
	  (int (DCALL *)(DeeObject *__restrict))&ri_index_del,
	  (int (DCALL *)(DeeObject *, DeeObject *))&ri_index_set },
	{ NULL }
};

PRIVATE struct type_member ri_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(RangeIterator, ri_range), "->?Ert:SeqRange"),
	TYPE_MEMBER_FIELD("__end__", STRUCT_OBJECT, offsetof(RangeIterator, ri_end)),
	TYPE_MEMBER_FIELD("__step__", STRUCT_OBJECT, offsetof(RangeIterator, ri_step)),
	TYPE_MEMBER_FIELD("__first__", STRUCT_ATOMIC | STRUCT_BOOL, offsetof(RangeIterator, ri_first)),
	TYPE_MEMBER_END
};

#define DEFINE_RANGEITERATOR_COMPARE(name, compare_object)                         \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                                  \
	name(RangeIterator *__restrict self,                                           \
	     RangeIterator *__restrict other) {                                        \
		DREF DeeObject *my_index, *ot_index, *result;                              \
		if (DeeObject_AssertTypeExact((DeeObject *)other, &SeqRangeIterator_Type)) \
			goto err;                                                              \
		my_index = ri_get_next_index(self);                                        \
		if unlikely(!my_index)                                                     \
			goto err;                                                              \
		ot_index = ri_get_next_index(other);                                       \
		if unlikely(!my_index)                                                     \
			goto err_myi;                                                          \
		result = compare_object(my_index, ot_index);                               \
		Dee_Decref(ot_index);                                                      \
		Dee_Decref(my_index);                                                      \
		return result;                                                             \
	err_myi:                                                                       \
		Dee_Decref(my_index);                                                      \
	err:                                                                           \
		return NULL;                                                               \
	}
#define DEFINE_RANGEITERATOR_COMPARE_R(name, compare_object)                       \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                                  \
	name(RangeIterator *__restrict self,                                           \
	     RangeIterator *__restrict other) {                                        \
		DREF DeeObject *my_index, *ot_index, *result;                              \
		if (DeeObject_AssertTypeExact((DeeObject *)other, &SeqRangeIterator_Type)) \
			goto err;                                                              \
		my_index = ri_get_next_index(self);                                        \
		if unlikely(!my_index)                                                     \
			goto err;                                                              \
		ot_index = ri_get_next_index(other);                                       \
		if unlikely(!my_index)                                                     \
			goto err_myi;                                                          \
		result = (unlikely(self->ri_range->r_rev))                                 \
		         ? compare_object(my_index, ot_index)                              \
		         : compare_object(ot_index, my_index);                             \
		Dee_Decref(ot_index);                                                      \
		Dee_Decref(my_index);                                                      \
		return result;                                                             \
	err_myi:                                                                       \
		Dee_Decref(my_index);                                                      \
	err:                                                                           \
		return NULL;                                                               \
	}
DEFINE_RANGEITERATOR_COMPARE(ri_eq, DeeObject_CompareEqObject)
DEFINE_RANGEITERATOR_COMPARE(ri_ne, DeeObject_CompareNeObject)
DEFINE_RANGEITERATOR_COMPARE_R(ri_lo, DeeObject_CompareLoObject)
DEFINE_RANGEITERATOR_COMPARE_R(ri_le, DeeObject_CompareLeObject)
DEFINE_RANGEITERATOR_COMPARE_R(ri_gr, DeeObject_CompareGrObject)
DEFINE_RANGEITERATOR_COMPARE_R(ri_ge, DeeObject_CompareGeObject)
#undef DEFINE_RANGEITERATOR_COMPARE_R
#undef DEFINE_RANGEITERATOR_COMPARE

PRIVATE struct type_cmp ri_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ri_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ri_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ri_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ri_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ri_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ri_ge,
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
				/* .tp_ctor      = */ (void *)&ri_ctor,
				/* .tp_copy_ctor = */ (void *)&ri_copy,
				/* .tp_deep_ctor = */ (void *)&ri_deep,
				/* .tp_any_ctor  = */ (void *)&ri_init,
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
	rwlock_init(&result->ri_lock);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
range_contains(Range *self,
               DeeObject *index) {
	DREF DeeObject *temp, *temp2, *temp3;
	int error;
	if likely(!self->r_rev) {
		/* if (!(self->r_start <= index)) return false; */
		error = DeeObject_CompareLe(self->r_start, index);
		if (error <= 0) /* if false-or-error */
			goto err_or_false;
		/* if (self->r_end <= index) return false; */
		error = DeeObject_CompareLe(self->r_end, index);
		if (error != 0) /* if true-or-error */
			goto err_or_false;
		if (self->r_step) {
			/* temp = index - self->r_start; */
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
			/* temp = temp % self->r_step; */
			temp2 = DeeObject_Mod(temp, self->r_step);
			Dee_Decref(temp);
			if unlikely(!temp2)
				goto err;
			error = DeeObject_Bool(temp2);
			Dee_Decref(temp2);
			/* if ((index - self->r_start) % self->r_step) return false; */
			if (error != 0) /* if true-or-error */
				goto err_or_false;
		}
	} else {
		ASSERT(self->r_step);
		/* if (self->r_start < index) return false; */
		error = DeeObject_CompareLo(self->r_start, index);
		if (error != 0) /* if true-or-error */
			goto err_or_false;
		/* if (!(self->r_end < index)) return false; */
		error = DeeObject_CompareLo(self->r_end, index);
		if (error <= 0) /* if false-or-error */
			goto err_or_false;
		/* temp = self->r_start - index; */
		temp = DeeObject_Sub(self->r_start, index);
		if unlikely(!temp)
			goto err;
		temp3 = DeeObject_Neg(self->r_step);
		if unlikely(!temp3)
			goto err_temp;
		/* temp2 = (self->r_start - index) % -self->r_step; */
		temp2 = DeeObject_Mod(temp, temp3);
		Dee_Decref(temp3);
		Dee_Decref(temp);
		if unlikely(!temp2)
			goto err;
		error = DeeObject_Bool(temp2);
		Dee_Decref(temp2);
		/* if ((self->r_start - index) % -self->r_step) return false; */
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
	return (likely(!self->r_rev))
	       ? DeeObject_CompareLo(self->r_start, self->r_end)
	       : DeeObject_CompareGr(self->r_start, self->r_end);
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
		error = (likely(!self->r_rev))
		        ? DeeObject_Dec(&temp)
		        : DeeObject_Inc(&temp);
		if unlikely(error)
			goto err_temp;
		result = DeeObject_Div(temp, self->r_step);
		if unlikely(!result)
			goto err_temp;
		Dee_Decref(temp);
	}
	error = DeeObject_CompareLo(result, &DeeInt_Zero);
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
		error = DeeObject_CompareGe(result, self->r_end);
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
		error = DeeObject_CompareLe(result, self->r_end);
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
range_getrange(Range *__restrict self,
               DeeObject *__restrict start,
               DeeObject *__restrict end) {
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
		error = DeeObject_CompareLo(end, &DeeInt_Zero);
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
			error = DeeObject_CompareGe(mylen, end);
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
	error = DeeObject_CompareLo(start, &DeeInt_Zero);
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
		error = DeeObject_CompareLe(mylen, new_start);
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
		error = DeeObject_CompareLo(end, &DeeInt_Zero);
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
			error = DeeObject_CompareLe(mylen, end);
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
		        ? DeeObject_CompareGe(new_start, new_end)
		        : DeeObject_CompareLe(new_end, new_start);
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
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&range_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&range_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&range_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&range_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&range_getrange,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ NULL /* TODO */
};


PRIVATE struct type_member range_members[] = {
	TYPE_MEMBER_FIELD("start", STRUCT_OBJECT, offsetof(Range, r_start)),
	TYPE_MEMBER_FIELD("end", STRUCT_OBJECT, offsetof(Range, r_end)),
	TYPE_MEMBER_FIELD("step", STRUCT_OBJECT, offsetof(Range, r_step)),
	TYPE_MEMBER_FIELD("__rev__", STRUCT_CONST | STRUCT_BOOL, offsetof(Range, r_rev)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member range_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SeqRangeIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
range_repr(Range *__restrict self) {
	return self->r_step
	       ? DeeString_Newf("[%r:%r,%r]", self->r_start, self->r_end, self->r_step)
	       : DeeString_Newf("[%r:%r]", self->r_start, self->r_end);
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
		temp = DeeObject_CompareLo(self->r_step, &DeeInt_Zero);
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
           size_t argc, DeeObject **argv) {
	self->r_step = NULL;
	if (DeeArg_Unpack(argc, argv, "oo|o:_Range",
	                  &self->r_start,
	                  &self->r_end,
	                  &self->r_step))
		goto err;
	if (self->r_step) {
		int temp;
		temp = DeeObject_CompareLo(self->r_step, &DeeInt_Zero);
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
				/* .tp_ctor      = */ (void *)&range_ctor,
				/* .tp_copy_ctor = */ (void *)&range_copy,
				/* .tp_deep_ctor = */ (void *)&range_deep,
				/* .tp_any_ctor  = */ (void *)&range_init,
				TYPE_FIXED_ALLOCATOR(Range)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&range_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&range_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&range_bool
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
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ range_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ range_class_members
};






#ifdef CONFIG_NO_THREADS
#define READ_INDEX(x)             (x)->iri_index
#else /* CONFIG_NO_THREADS */
#define READ_INDEX(x) ATOMIC_READ((x)->iri_index)
#endif /* !CONFIG_NO_THREADS */


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
         size_t argc, DeeObject **argv) {
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
	dssize_t new_index;
	dssize_t index = READ_INDEX(self);
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
	dssize_t result_index, new_index;
#ifndef CONFIG_NO_THREADS
	do
#endif /* !CONFIG_NO_THREADS */
	{
		result_index = READ_INDEX(self);
		/* Test for overflow/iteration done. */
		if (OVERFLOW_SADD(result_index, self->iri_step, &new_index) ||
		    (likely(self->iri_step >= 0) ? result_index >= self->iri_end
		                                 : result_index <= self->iri_end))
			return ITER_DONE;
#ifdef CONFIG_NO_THREADS
		self->iri_index = new_index;
#endif /* CONFIG_NO_THREADS */
	}
#ifndef CONFIG_NO_THREADS
	while (!ATOMIC_CMPXCH(self->iri_index, result_index, new_index));
#endif /* !CONFIG_NO_THREADS */
	/* Return a new integer for the resulting index. */
	return DeeInt_NewSSize(result_index);
}

PRIVATE struct type_member iri_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(IntRangeIterator, iri_range), "->?Ert:SeqIntRange"),
	/* We allow write-access to these members because doing so doesn't
	 * actually harm anything, although fiddling with this stuff may
	 * break some weak expectations but should never crash anything! */
	TYPE_MEMBER_FIELD("__index__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRangeIterator, iri_index)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRangeIterator, iri_end)),
	TYPE_MEMBER_FIELD("__step__", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRangeIterator, iri_step)),
	TYPE_MEMBER_END
};

#define DEFINE_IRI_COMPARE(name, op)                                     \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                        \
	name(IntRangeIterator *__restrict self,                              \
	     IntRangeIterator *__restrict other) {                           \
		if (DeeObject_AssertTypeExact(other, &SeqIntRangeIterator_Type)) \
			goto err;                                                    \
		return_bool(READ_INDEX(self) op READ_INDEX(other));              \
	err:                                                                 \
		return NULL;                                                     \
	}
#define DEFINE_IRI_COMPARE_R(name, op)                                   \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                        \
	name(IntRangeIterator *__restrict self,                              \
	     IntRangeIterator *__restrict other) {                           \
		if (DeeObject_AssertTypeExact(other, &SeqIntRangeIterator_Type)) \
			goto err;                                                    \
		return_bool(self->iri_step >= 0                                  \
		            ? READ_INDEX(self) op READ_INDEX(other)              \
		            : READ_INDEX(other) op READ_INDEX(self));            \
	err:                                                                 \
		return NULL;                                                     \
	}
DEFINE_IRI_COMPARE(iri_eq, ==)
DEFINE_IRI_COMPARE(iri_ne, !=)
DEFINE_IRI_COMPARE_R(iri_lo, <)
DEFINE_IRI_COMPARE_R(iri_le, <=)
DEFINE_IRI_COMPARE_R(iri_gr, >)
DEFINE_IRI_COMPARE_R(iri_ge, >=)
#undef DEFINE_IRI_COMPARE_R
#undef DEFINE_IRI_COMPARE

PRIVATE struct type_cmp iri_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iri_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iri_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iri_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iri_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iri_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iri_ge
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
				/* .tp_ctor      = */ (void *)&iri_ctor,
				/* .tp_copy_ctor = */ (void *)&iri_copy,
				/* .tp_deep_ctor = */ (void *)&iri_copy,
				/* .tp_any_ctor  = */ (void *)&iri_init,
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
	dssize_t index;
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
intrange_size(IntRange *__restrict self) {
	dssize_t result;
	ASSERT(self->ir_step != 0);
	result = (self->ir_end - self->ir_start) + self->ir_step;
	if (self->ir_step >= 0)
		--result;
	else {
		++result;
	}
	result /= self->ir_step;
	if (result < 0)
		result = 0;
	return DeeInt_NewSize((size_t)result);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
intrange_nsi_getsize(IntRange *__restrict self) {
	dssize_t result;
	ASSERT(self->ir_step != 0);
	result = (self->ir_end - self->ir_start) + self->ir_step;
	if (self->ir_step >= 0)
		--result;
	else {
		++result;
	}
	result /= self->ir_step;
	if (result < 0)
		result = 0;
	return (size_t)result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
intrange_nsi_getitem(IntRange *__restrict self, size_t index) {
	dssize_t result;
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
	                        intrange_nsi_getsize(self));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
intrange_getitem(IntRange *self,
                 DeeObject *index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return intrange_nsi_getitem(self, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
intrange_nsi_getrange(IntRange *__restrict self,
                      dssize_t start, dssize_t end) {
	size_t mylen = intrange_nsi_getsize(self);
	if (start < 0)
		start += mylen;
	if (end < 0)
		end += mylen;
	if ((size_t)end > mylen)
		end = (dssize_t)mylen;
	if ((size_t)start >= (size_t)end)
		return_reference_(Dee_EmptySeq);
	return DeeRange_NewInt(self->ir_start + ((size_t)start * self->ir_step),
	                       self->ir_end - ((mylen - (size_t)end) * self->ir_step),
	                       self->ir_step);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
intrange_nsi_getrange_n(IntRange *__restrict self,
                        dssize_t start) {
	size_t mylen = intrange_nsi_getsize(self);
	if (start < 0)
		start += mylen;
	if ((size_t)start >= mylen)
		return_reference_(Dee_EmptySeq);
	return DeeRange_NewInt(self->ir_start + ((size_t)start * self->ir_step),
	                       self->ir_end,
	                       self->ir_step);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
intrange_getrange(IntRange *__restrict self,
                  DeeObject *__restrict start_ob,
                  DeeObject *__restrict end_ob) {
	dssize_t start_index, end_index;
	if (DeeObject_AsSSize(start_ob, &start_index))
		goto err;
	if (DeeNone_Check(end_ob))
		return intrange_nsi_getrange_n(self, start_index);
	if (DeeObject_AsSSize(end_ob, &end_index))
		goto err;
	return intrange_nsi_getrange(self, start_index, end_index);
err:
	return NULL;
}




PRIVATE struct type_nsi intrange_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&intrange_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&intrange_nsi_getsize,
			/* .nsi_getitem      = */ (void *)&intrange_nsi_getitem,
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)NULL,
			/* .nsi_getrange     = */ (void *)&intrange_nsi_getrange,
			/* .nsi_getrange_n   = */ (void *)&intrange_nsi_getrange_n,
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

PRIVATE struct type_seq intrange_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&intrange_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&intrange_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&intrange_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&intrange_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&intrange_getrange,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &intrange_nsi
};


PRIVATE struct type_member intrange_members[] = {
	TYPE_MEMBER_FIELD("start", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRange, ir_start)),
	TYPE_MEMBER_FIELD("end", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRange, ir_end)),
	TYPE_MEMBER_FIELD("step", STRUCT_ATOMIC | STRUCT_SSIZE_T, offsetof(IntRange, ir_step)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member intrange_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SeqIntRangeIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
intrange_repr(IntRange *__restrict self) {
	return self->ir_step != 1
	       ? DeeString_Newf("[%Id:%Id,%Id]", self->ir_start, self->ir_end, self->ir_step)
	       : DeeString_Newf("[%Id:%Id]", self->ir_start, self->ir_end);
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
              size_t argc, DeeObject **argv) {
	self->ir_step = 1;
	if (DeeArg_Unpack(argc, argv, "IdId|Id:_SeqIntRange",
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
				/* .tp_ctor      = */ (void *)&intrange_ctor,
				/* .tp_copy_ctor = */ (void *)&intrange_copy,
				/* .tp_deep_ctor = */ (void *)&intrange_copy,
				/* .tp_any_ctor  = */ (void *)&intrange_init,
				TYPE_FIXED_ALLOCATOR(IntRange)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&intrange_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&intrange_bool
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
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ intrange_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ intrange_class_members
};














/* Test to force use of object-based ranges. */
#undef ALWAYS_USE_OBJECT_RANGES
/* #define ALWAYS_USE_OBJECT_RANGES 1 */

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeRange_NewInt(dssize_t begin,
                dssize_t end,
                dssize_t step) {
#ifdef ALWAYS_USE_OBJECT_RANGES
	DREF DeeObject *begin_ob, *end_ob, *step_ob, *result;
	ASSERT(step != 0);
	if ((begin_ob = DeeInt_NewSSize(begin)) == NULL)
		return NULL;
	if ((end_ob = DeeInt_NewSSize(end)) == NULL) {
		Dee_Decref(begin_ob);
		return NULL;
	}
	if ((step_ob = DeeInt_NewSSize(step)) == NULL) {
		Dee_Decref(end_ob);
		Dee_Decref(begin_ob);
		return NULL;
	}
	result = DeeRange_New(begin_ob, end_ob, step_ob);
	Dee_Decref(step_ob);
	Dee_Decref(end_ob);
	Dee_Decref(begin_ob);
	return result;
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

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeRange_New(DeeObject *begin,
             DeeObject *end,
             DeeObject *step) {
	DREF Range *result;
	int temp;
	ASSERT_OBJECT(begin);
	ASSERT_OBJECT(end);
	ASSERT_OBJECT_OPT(step);
#ifndef ALWAYS_USE_OBJECT_RANGES
	/* Check for special optimizations for the likely case of int-only arguments. */
	{
		dssize_t i_begin, i_end, i_step;
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
		temp = DeeObject_CompareLo(step, &DeeInt_Zero);
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
