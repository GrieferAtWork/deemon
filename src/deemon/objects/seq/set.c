/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SET_C
#define GUARD_DEEMON_OBJECTS_SEQ_SET_C 1

#include "set.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/thread.h>

#include "../../runtime/runtime_error.h"
#include "../gc_inspect.h"

DECL_BEGIN

/* Assert that we can use some common-proxy methods. */
STATIC_ASSERT(sizeof(SetUnion) == sizeof(SetIntersection));
STATIC_ASSERT(sizeof(SetUnion) == sizeof(SetDifference));
STATIC_ASSERT(sizeof(SetUnion) == sizeof(SetSymmetricDifference));
STATIC_ASSERT(offsetof(SetUnion, su_a) == offsetof(SetIntersection, si_a));
STATIC_ASSERT(offsetof(SetUnion, su_b) == offsetof(SetIntersection, si_b));
STATIC_ASSERT(offsetof(SetUnion, su_a) == offsetof(SetDifference, sd_a));
STATIC_ASSERT(offsetof(SetUnion, su_b) == offsetof(SetDifference, sd_b));
STATIC_ASSERT(offsetof(SetUnion, su_a) == offsetof(SetSymmetricDifference, ssd_a));
STATIC_ASSERT(offsetof(SetUnion, su_b) == offsetof(SetSymmetricDifference, ssd_b));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_union) == offsetof(SetIntersectionIterator, sii_intersect));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_union) == offsetof(SetDifferenceIterator, sdi_diff));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_union) == offsetof(SetSymmetricDifferenceIterator, ssd_set));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_iter) == offsetof(SetSymmetricDifferenceIterator, ssd_iter));
STATIC_ASSERT(sizeof(SetUnionIterator) == sizeof(SetSymmetricDifferenceIterator));
STATIC_ASSERT(sizeof(SetIntersectionIterator) == sizeof(SetDifferenceIterator));
STATIC_ASSERT(offsetof(SetIntersectionIterator, sii_iter) == offsetof(SetDifferenceIterator, sdi_iter));
STATIC_ASSERT(offsetof(SetIntersectionIterator, sii_other) == offsetof(SetDifferenceIterator, sdi_other));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(SetUnionIterator, sui_lock) == offsetof(SetSymmetricDifferenceIterator, ssd_lock));
#endif /* !CONFIG_NO_THREADS */
STATIC_ASSERT(offsetof(SetUnionIterator, sui_in2nd) == offsetof(SetSymmetricDifferenceIterator, ssd_in2nd));


/* ================================================================================ */
/*   COMMON PROXY                                                                   */
/* ================================================================================ */
PRIVATE NONNULL((1)) void DCALL
proxy_fini(SetUnion *__restrict self) {
	Dee_Decref(self->su_a);
	Dee_Decref(self->su_b);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_visit(SetUnion *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->su_a);
	Dee_Visit(self->su_b);
}




/* ================================================================================ */
/*   SET UNION                                                                      */
/* ================================================================================ */
STATIC_ASSERT(offsetof(SetUnionIterator, sui_union) == offsetof(SetUnion, su_a));
STATIC_ASSERT(offsetof(SetUnionIterator, sui_iter) == offsetof(SetUnion, su_b));
#define suiter_fini proxy_fini
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
suiter_get_iter(SetUnionIterator *__restrict self) {
	DREF DeeObject *result;
	rwlock_read(&self->sui_lock);
	result = self->sui_iter;
	Dee_Incref(result);
	rwlock_endread(&self->sui_lock);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_set_iter(SetUnionIterator *__restrict self,
                DeeObject *__restrict iter) {
	DREF DeeObject *old_iter;
	if (DeeGC_ReferredBy(iter, (DeeObject *)self))
		return err_reference_loop((DeeObject *)self, iter);
	Dee_Incref(iter);
	rwlock_read(&self->sui_lock);
	old_iter       = self->sui_iter;
	self->sui_iter = iter;
	rwlock_endread(&self->sui_lock);
	Dee_Decref(old_iter);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
suiter_get_in2nd(SetUnionIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return_bool(self->sui_in2nd);
#else /* CONFIG_NO_THREADS */
	bool result;
	COMPILER_BARRIER();
	result = self->sui_in2nd;
	COMPILER_BARRIER();
	return_bool(result);
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_set_in2nd(SetUnionIterator *__restrict self,
                 DeeObject *__restrict value) {
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		return newval;
	rwlock_write(&self->sui_lock);
	self->sui_in2nd = !!newval;
	rwlock_endwrite(&self->sui_lock);
	return 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
suiter_visit(SetUnionIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->sui_union);
	rwlock_read(&self->sui_lock);
	Dee_Visit(self->sui_iter);
	rwlock_endread(&self->sui_lock);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_copy(SetUnionIterator *__restrict self,
            SetUnionIterator *__restrict other) {
	DREF DeeObject *iter;
	rwlock_read(&other->sui_lock);
	iter            = other->sui_iter;
	self->sui_in2nd = other->sui_in2nd;
	Dee_Incref(iter);
	rwlock_endread(&other->sui_lock);
	self->sui_iter = DeeObject_Copy(iter);
	Dee_Decref(iter);
	if unlikely(!self->sui_iter)
		goto err;
	rwlock_init(&self->sui_lock);
	self->sui_union = other->sui_union;
	Dee_Incref(self->sui_union);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
suiter_deep(SetUnionIterator *__restrict self,
            SetUnionIterator *__restrict other) {
	DREF DeeObject *iter;
	rwlock_read(&other->sui_lock);
	iter            = other->sui_iter;
	self->sui_in2nd = other->sui_in2nd;
	Dee_Incref(iter);
	rwlock_endread(&other->sui_lock);
	self->sui_iter = DeeObject_DeepCopy(iter);
	Dee_Decref(iter);
	if unlikely(!self->sui_iter)
		goto err;
	rwlock_init(&self->sui_lock);
	self->sui_union = (DREF SetUnion *)DeeObject_DeepCopy((DeeObject *)other->sui_union);
	if unlikely(!self->sui_union)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->sui_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
suiter_ctor(SetUnionIterator *__restrict self) {
	self->sui_union = (DREF SetUnion *)DeeObject_NewDefault(self->ob_type == &SetUnionIterator_Type
	                                                        ? &SetUnion_Type
	                                                        : &SetSymmetricDifference_Type);
	if unlikely(!self->sui_union)
		goto err;
	self->sui_iter = DeeObject_IterSelf(self->sui_union->su_a);
	if unlikely(!self->sui_iter)
		goto err_union;
	rwlock_init(&self->sui_lock);
	self->sui_in2nd = false;
	return 0;
err_union:
	Dee_Decref(self->sui_union);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
suiter_init(SetUnionIterator *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SetUnionIterator", &self->sui_union))
		goto err;
	if (DeeObject_AssertTypeExact(self->sui_union, &SetUnion_Type))
		goto err;
	if ((self->sui_iter = DeeObject_IterSelf(self->sui_union->su_a)) == NULL)
		goto err;
	Dee_Incref(self->sui_union);
	rwlock_init(&self->sui_lock);
	self->sui_in2nd = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
suiter_next(SetUnionIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *iter;
	bool is_second;
again:
	rwlock_read(&self->sui_lock);
	iter      = self->sui_iter;
	is_second = self->sui_in2nd;
	Dee_Incref(iter);
	rwlock_endread(&self->sui_lock);
read_from_iter:
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (is_second)
		goto done;
	if unlikely(result != ITER_DONE) {
		if (result) {
			int temp;
			/* Check if the found item is also part of the second set.
			 * If it is, don't yield it now, but yield it later, as
			 * part of the enumeration of the second set. */
			temp = DeeObject_Contains(self->sui_union->su_b, result);
			if (temp != 0) {
				/* Error, or apart of second set. */
				Dee_Decref(result);
				if unlikely(temp < 0) {
					result = NULL;
					goto done;
				}
				goto again;
			}
		}
		goto done;
	}
	/* Try to switch to the next iterator. */
	if (self->sui_iter != iter)
		goto again;
	if (self->sui_in2nd)
		goto done;
#ifndef CONFIG_NO_THREADS
	COMPILER_READ_BARRIER();
	if (self->sui_iter != iter)
		goto again; /* Test again to prevent race conditions. */
#endif              /* !CONFIG_NO_THREADS */
	/* Create the level #2 iterator. */
	result = DeeObject_IterSelf(self->sui_union->su_b);
	if unlikely(!result)
		goto done;
	rwlock_write(&self->sui_lock);
	/* Check for another race condition. */
	if unlikely(self->sui_iter != iter) {
		rwlock_endwrite(&self->sui_lock);
		Dee_Decref(result);
		goto again;
	}
	ASSERT(!self->sui_in2nd);
	self->sui_iter  = result; /* Inherit reference (x2) */
	self->sui_in2nd = true;
	Dee_Incref(result); /* Reference stored in `sui_iter' */
	rwlock_endwrite(&self->sui_lock);
	Dee_Decref(iter); /* Reference inherited from `sui_iter' */
	iter      = result;
	is_second = true;
	if (DeeThread_CheckInterrupt()) {
		Dee_Decref(iter);
		return NULL;
	}
	goto read_from_iter;
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
suiter_eq(SetUnionIterator *self,
          SetUnionIterator *other) {
	DREF DeeObject *my_iter, *ot_iter, *result;
	bool my_2nd, ot_2nd;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	rwlock_read(&self->sui_lock);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	rwlock_endread(&self->sui_lock);
	rwlock_read(&other->sui_lock);
	ot_iter = other->sui_iter;
	ot_2nd  = other->sui_in2nd;
	rwlock_endread(&other->sui_lock);
	if (my_2nd != ot_2nd) {
		result = Dee_False;
		Dee_Incref(result);
	} else {
		result = DeeObject_CompareEqObject(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
suiter_ne(SetUnionIterator *self,
          SetUnionIterator *other) {
	DREF DeeObject *my_iter, *ot_iter, *result;
	bool my_2nd, ot_2nd;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	rwlock_read(&self->sui_lock);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	rwlock_endread(&self->sui_lock);
	rwlock_read(&other->sui_lock);
	ot_iter = other->sui_iter;
	ot_2nd  = other->sui_in2nd;
	rwlock_endread(&other->sui_lock);
	if (my_2nd != ot_2nd) {
		result = Dee_True;
		Dee_Incref(result);
	} else {
		result = DeeObject_CompareNeObject(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
suiter_lo(SetUnionIterator *self,
          SetUnionIterator *other) {
	DREF DeeObject *my_iter, *ot_iter, *result;
	bool my_2nd, ot_2nd;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	rwlock_read(&self->sui_lock);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	rwlock_endread(&self->sui_lock);
	rwlock_read(&other->sui_lock);
	ot_iter = other->sui_iter;
	ot_2nd  = other->sui_in2nd;
	rwlock_endread(&other->sui_lock);
	if (my_2nd != ot_2nd) {
		result = my_2nd ? Dee_False : Dee_True;
		Dee_Incref(result);
	} else {
		result = DeeObject_CompareLoObject(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
suiter_le(SetUnionIterator *self,
          SetUnionIterator *other) {
	DREF DeeObject *my_iter, *ot_iter, *result;
	bool my_2nd, ot_2nd;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	rwlock_read(&self->sui_lock);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	rwlock_endread(&self->sui_lock);
	rwlock_read(&other->sui_lock);
	ot_iter = other->sui_iter;
	ot_2nd  = other->sui_in2nd;
	rwlock_endread(&other->sui_lock);
	if (my_2nd != ot_2nd) {
		result = my_2nd ? Dee_False : Dee_True;
		Dee_Incref(result);
	} else {
		result = DeeObject_CompareLeObject(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
suiter_gr(SetUnionIterator *self,
          SetUnionIterator *other) {
	DREF DeeObject *my_iter, *ot_iter, *result;
	bool my_2nd, ot_2nd;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	rwlock_read(&self->sui_lock);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	rwlock_endread(&self->sui_lock);
	rwlock_read(&other->sui_lock);
	ot_iter = other->sui_iter;
	ot_2nd  = other->sui_in2nd;
	rwlock_endread(&other->sui_lock);
	if (my_2nd != ot_2nd) {
		result = my_2nd ? Dee_True : Dee_False;
		Dee_Incref(result);
	} else {
		result = DeeObject_CompareGrObject(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
suiter_ge(SetUnionIterator *self,
          SetUnionIterator *other) {
	DREF DeeObject *my_iter, *ot_iter, *result;
	bool my_2nd, ot_2nd;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	rwlock_read(&self->sui_lock);
	my_iter = self->sui_iter;
	my_2nd  = self->sui_in2nd;
	rwlock_endread(&self->sui_lock);
	rwlock_read(&other->sui_lock);
	ot_iter = other->sui_iter;
	ot_2nd  = other->sui_in2nd;
	rwlock_endread(&other->sui_lock);
	if (my_2nd != ot_2nd) {
		result = my_2nd ? Dee_True : Dee_False;
		Dee_Incref(result);
	} else {
		result = DeeObject_CompareGeObject(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return NULL;
}

PRIVATE struct type_cmp suiter_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&suiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&suiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&suiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&suiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&suiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&suiter_ge
};

PRIVATE struct type_member tpconst suiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SetUnionIterator, sui_union), "->?Ert:SetUnion"),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst suiter_getsets[] = {
	{ "__iter__",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&suiter_get_iter, NULL,
	  (int (DCALL *)(DeeObject *, DeeObject *))&suiter_set_iter,
	  DOC("->?DIterator") },
	{ "__in2nd__",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&suiter_get_in2nd, NULL,
	  (int (DCALL *)(DeeObject *, DeeObject *))&suiter_set_in2nd,
	  DOC("->?Dbool") },
	{ NULL }
};

INTERN DeeTypeObject SetUnionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetUnionIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SetUnion)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&suiter_ctor,
				/* .tp_copy_ctor = */ (void *)&suiter_copy,
				/* .tp_deep_ctor = */ (void *)&suiter_deep,
				/* .tp_any_ctor  = */ (void *)&suiter_init,
				TYPE_FIXED_ALLOCATOR(SetUnionIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&suiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&suiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &suiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&suiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ suiter_getsets,
	/* .tp_members       = */ suiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
su_ctor(SetUnion *__restrict self) {
	self->su_a = Dee_EmptySet;
	self->su_b = Dee_EmptySet;
	Dee_Incref(Dee_EmptySet);
	Dee_Incref(Dee_EmptySet);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
su_copy(SetUnion *__restrict self,
        SetUnion *__restrict other) {
	self->su_a = other->su_a;
	Dee_Incref(self->su_a);
	self->su_b = other->su_b;
	Dee_Incref(self->su_b);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
su_deep(SetUnion *__restrict self,
        SetUnion *__restrict other) {
	self->su_a = DeeObject_DeepCopy(other->su_a);
	if unlikely(!self->su_a)
		goto err;
	self->su_b = DeeObject_DeepCopy(other->su_b);
	if unlikely(!self->su_a)
		goto err_a;
	return 0;
err_a:
	Dee_Decref(self->su_a);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
su_init(SetUnion *__restrict self,
        size_t argc, DeeObject *const *argv) {
	self->su_a = Dee_EmptySet;
	self->su_b = Dee_EmptySet;
	if (DeeArg_Unpack(argc, argv, "|oo:_SetUnion", &self->su_a, &self->su_b))
		goto err;
	Dee_Incref(self->su_a);
	Dee_Incref(self->su_b);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF SetUnionIterator *DCALL
su_iter(SetUnion *__restrict self) {
	DREF SetUnionIterator *result;
	result = DeeObject_MALLOC(SetUnionIterator);
	if unlikely(!result)
		goto done;
	result->sui_iter = DeeObject_IterSelf(self->su_a);
	if unlikely(!result->sui_iter)
		goto err_r;
	rwlock_init(&result->sui_lock);
	result->sui_in2nd = false;
	result->sui_union = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SetUnionIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
su_contains(SetUnion *self, DeeObject *item) {
	DREF DeeObject *result;
	int temp;
	result = DeeObject_ContainsObject(self->su_a, item);
	if unlikely(!result)
		goto done;
	temp = DeeObject_Bool(result);
	if unlikely(temp < 0)
		goto err_r;
	if (temp)
		goto done;
	Dee_Decref(result);
	/* Check the second set, and forward the return value. */
	result = DeeObject_ContainsObject(self->su_b, item);
done:
	return result;
err_r:
	Dee_Clear(result);
	goto done;
}

PRIVATE struct type_seq su_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&su_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&su_contains,
};

PRIVATE struct type_member tpconst su_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SetUnionIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetUnion_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetUnion",
	/* .tp_doc      = */ DOC("(a:?Dset=!S0,b:?Dset=!S0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&su_ctor,
				/* .tp_copy_ctor = */ (void *)&su_copy,
				/* .tp_deep_ctor = */ (void *)&su_deep,
				/* .tp_any_ctor  = */ (void *)&su_init,
				TYPE_FIXED_ALLOCATOR(SetUnion)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &su_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ su_class_members
};




/* ================================================================================ */
/*   SET SYMMETRIC DIFFERENCE                                                       */
/* ================================================================================ */
#define ssditer_ctor    suiter_ctor
#define ssditer_copy    suiter_copy
#define ssditer_deep    suiter_deep
#define ssditer_init    suiter_init
#define ssditer_fini    suiter_fini
#define ssditer_visit   suiter_visit
#define ssditer_cmp     suiter_cmp
#define ssditer_getsets suiter_getsets
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ssditer_next(SetSymmetricDifferenceIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *iter;
	bool is_second;
again:
	rwlock_read(&self->ssd_lock);
	iter      = self->ssd_iter;
	is_second = self->ssd_in2nd;
	Dee_Incref(iter);
	rwlock_endread(&self->ssd_lock);
read_from_iter:
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (is_second)
		goto done;
	if unlikely(result != ITER_DONE) {
		if (result) {
			int temp;
			/* Only yield the item if it's not contained in the other set. */
			temp = DeeObject_Contains(is_second
			                          ? self->ssd_set->ssd_b
			                          : self->ssd_set->ssd_a,
			                          result);
			if (temp != 0) {
				/* Error, or apart of second set. */
				Dee_Decref(result);
				if unlikely(temp < 0) {
					result = NULL;
					goto done;
				}
				goto again;
			}
		}
		goto done;
	}
	/* Try to switch to the next iterator. */
	if (self->ssd_iter != iter)
		goto again;
	if (self->ssd_in2nd)
		goto done;
#ifndef CONFIG_NO_THREADS
	COMPILER_READ_BARRIER();
	if (self->ssd_iter != iter)
		goto again; /* Test again to prevent race conditions. */
#endif              /* !CONFIG_NO_THREADS */
	/* Create the level #2 iterator. */
	result = DeeObject_IterSelf(self->ssd_set->ssd_b);
	if unlikely(!result)
		goto done;
	rwlock_write(&self->ssd_lock);
	/* Check for another race condition. */
	if unlikely(self->ssd_iter != iter) {
		rwlock_endwrite(&self->ssd_lock);
		Dee_Decref(result);
		goto again;
	}
	ASSERT(!self->ssd_in2nd);
	self->ssd_iter  = result; /* Inherit reference (x2) */
	self->ssd_in2nd = true;
	Dee_Incref(result); /* Reference stored in `ssd_iter' */
	rwlock_endwrite(&self->ssd_lock);
	Dee_Decref(iter); /* Reference inherited from `ssd_iter' */
	iter      = result;
	is_second = true;
	if (DeeThread_CheckInterrupt()) {
		Dee_Decref(iter);
		return NULL;
	}
	goto read_from_iter;
done:
	return result;
}

PRIVATE struct type_member tpconst ssditer_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT,
	                      offsetof(SetSymmetricDifferenceIterator, ssd_set),
	                      "->?Ert:SetSymmetricDifference"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetSymmetricDifferenceIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetSymmetricDifferenceIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SetSymmetricDifference)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&ssditer_ctor,
				/* .tp_copy_ctor = */ (void *)&ssditer_copy,
				/* .tp_deep_ctor = */ (void *)&ssditer_deep,
				/* .tp_any_ctor  = */ (void *)&ssditer_init,
				TYPE_FIXED_ALLOCATOR(SetSymmetricDifferenceIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ssditer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ssditer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &ssditer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssditer_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ssditer_getsets,
	/* .tp_members       = */ ssditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

#define ssd_ctor su_ctor
#define ssd_copy su_copy
#define ssd_deep su_deep
PRIVATE WUNUSED NONNULL((1)) int DCALL
ssd_init(SetSymmetricDifference *__restrict self,
         size_t argc, DeeObject *const *argv) {
	self->ssd_a = Dee_EmptySet;
	self->ssd_b = Dee_EmptySet;
	if (DeeArg_Unpack(argc, argv, "|oo:_SetSymmetricDifference", &self->ssd_a, &self->ssd_b))
		return -1;
	Dee_Incref(self->ssd_a);
	Dee_Incref(self->ssd_b);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF SetSymmetricDifferenceIterator *DCALL
ssd_iter(SetSymmetricDifference *__restrict self) {
	DREF SetSymmetricDifferenceIterator *result;
	result = DeeObject_MALLOC(SetSymmetricDifferenceIterator);
	if unlikely(!result)
		goto done;
	result->ssd_iter = DeeObject_IterSelf(self->ssd_a);
	if unlikely(!result->ssd_iter)
		goto err_r;
	rwlock_init(&result->ssd_lock);
	result->ssd_in2nd = false;
	result->ssd_set   = self;
	Dee_Incref(self);
	DeeObject_Init(result, &SetSymmetricDifferenceIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ssd_contains(SetSymmetricDifference *self, DeeObject *item) {
	DREF DeeObject *result;
	int cona, conb;
	cona = DeeObject_Contains(self->ssd_a, item);
	if unlikely(cona < 0)
		goto err;
	conb = DeeObject_Contains(self->ssd_b, item);
	if unlikely(conb < 0)
		goto err;
	result = DeeBool_For(!!cona ^ !!conb);
	Dee_Incref(result);
done:
	return result;
err:
	result = NULL;
	goto done;
}

PRIVATE struct type_seq ssd_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ssd_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ssd_contains,
};

PRIVATE struct type_member tpconst ssd_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SetSymmetricDifferenceIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetSymmetricDifference_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetSymmetricDifference",
	/* .tp_doc      = */ DOC("(a:?Dset=!S0,b:?Dset=!S0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&ssd_ctor,
				/* .tp_copy_ctor = */ (void *)&ssd_copy,
				/* .tp_deep_ctor = */ (void *)&ssd_deep,
				/* .tp_any_ctor  = */ (void *)&ssd_init,
				TYPE_FIXED_ALLOCATOR(SetSymmetricDifference)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ssd_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ssd_class_members
};






/* ================================================================================ */
/*   SET INTERSECTION                                                               */
/* ================================================================================ */
PRIVATE NONNULL((1)) void DCALL
siiter_fini(SetIntersectionIterator *__restrict self) {
	Dee_Decref_unlikely(self->sii_other);
	Dee_Decref(self->sii_intersect);
	Dee_Decref(self->sii_iter);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
siiter_ctor(SetIntersectionIterator *__restrict self) {
	self->sii_intersect = (DREF SetIntersection *)DeeObject_NewDefault(self->ob_type == &SetDifferenceIterator_Type
	                                                                   ? &SetIntersection_Type
	                                                                   : &SetDifference_Type);
	if unlikely(!self->sii_intersect)
		goto err;
	self->sii_iter = DeeObject_IterSelf(self->sii_intersect->si_a);
	if unlikely(!self->sii_iter)
		goto err_isec;
	self->sii_other = self->sii_intersect->si_b;
	Dee_Incref(self->sii_other);
	return 0;
err_isec:
	Dee_Decref(self->sii_intersect);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
siiter_copy(SetIntersectionIterator *__restrict self,
            SetIntersectionIterator *__restrict other) {
	self->sii_iter = DeeObject_Copy(other->sii_iter);
	if unlikely(!self->sii_iter)
		goto err;
	self->sii_intersect = other->sii_intersect;
	self->sii_other     = other->sii_other;
	Dee_Incref(self->sii_intersect);
	Dee_Incref(self->sii_other);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
siiter_deep(SetIntersectionIterator *__restrict self,
            SetIntersectionIterator *__restrict other) {
	self->sii_iter = DeeObject_DeepCopy(other->sii_iter);
	if unlikely(!self->sii_iter)
		goto err;
	self->sii_intersect = (DREF SetIntersection *)DeeObject_DeepCopy((DeeObject *)other->sii_intersect);
	if unlikely(!self->sii_intersect)
		goto err_iter;
	self->sii_other = self->sii_intersect->si_b;
	Dee_Incref(self->sii_other);
	return 0;
err_iter:
	Dee_Decref(self->sii_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
siiter_init(SetIntersectionIterator *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SetIntersectionIterator", &self->sii_intersect))
		goto err;
	if (DeeObject_AssertTypeExact(self->sii_intersect, &SetIntersection_Type))
		goto err;
	self->sii_iter = DeeObject_IterSelf(self->sii_intersect->si_a);
	if unlikely(!self)
		goto err;
	Dee_Incref(self->sii_intersect);
	self->sii_other = self->sii_intersect->si_b;
	Dee_Incref(self->sii_other);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
siiter_visit(SetIntersectionIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->sii_intersect);
	Dee_Visit(self->sii_iter);
	Dee_Visit(self->sii_other);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
siiter_next(SetIntersectionIterator *__restrict self) {
	DREF DeeObject *result;
	int temp;
again:
	result = DeeObject_IterNext(self->sii_iter);
	if (!ITER_ISOK(result))
		goto done;
	/* Check if contained in the second set. */
	temp = DeeObject_Contains(self->sii_other, result);
	if (temp <= 0) {
		Dee_Decref(result);
		if (!temp) {
			if (!DeeThread_CheckInterrupt())
				goto again; /* If the object isn't contained within, read the next one. */
		}
		result = NULL; /* Error... */
	}
done:
	return result;
}

#define DEFINE_SIITER_COMPARE(name, func)                                  \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                  \
	name(SetIntersectionIterator *self, SetIntersectionIterator *other) {  \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self))) \
			goto err;                                                      \
		return func(self->sii_iter, other->sii_iter);                      \
	err:                                                                   \
		return NULL;                                                       \
	}
DEFINE_SIITER_COMPARE(siiter_eq, DeeObject_CompareEqObject)
DEFINE_SIITER_COMPARE(siiter_ne, DeeObject_CompareNeObject)
DEFINE_SIITER_COMPARE(siiter_lo, DeeObject_CompareLoObject)
DEFINE_SIITER_COMPARE(siiter_le, DeeObject_CompareLeObject)
DEFINE_SIITER_COMPARE(siiter_gr, DeeObject_CompareGrObject)
DEFINE_SIITER_COMPARE(siiter_ge, DeeObject_CompareGeObject)
#undef DEFINE_SIITER_COMPARE

PRIVATE struct type_cmp siiter_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&siiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&siiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&siiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&siiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&siiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&siiter_ge
};

PRIVATE struct type_member tpconst siiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SetIntersectionIterator, sii_intersect), "->?Ert:SetIntersection"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SetIntersectionIterator, sii_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__other__", STRUCT_OBJECT, offsetof(SetIntersectionIterator, sii_other), "->?Dset"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetIntersectionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetIntersectionIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SetIntersection)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&siiter_ctor,
				/* .tp_copy_ctor = */ (void *)&siiter_copy,
				/* .tp_deep_ctor = */ (void *)&siiter_deep,
				/* .tp_any_ctor  = */ (void *)&siiter_init,
				TYPE_FIXED_ALLOCATOR(SetIntersectionIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&siiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&siiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &siiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&siiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ siiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

#define si_ctor su_ctor
#define si_copy su_copy
#define si_deep su_deep
PRIVATE WUNUSED NONNULL((1)) int DCALL
si_init(SetIntersection *__restrict self,
        size_t argc, DeeObject *const *argv) {
	self->si_a = Dee_EmptySet;
	self->si_b = Dee_EmptySet;
	if (DeeArg_Unpack(argc, argv, "|oo:_SetIntersection", &self->si_a, &self->si_b))
		goto err;
	Dee_Incref(self->si_a);
	Dee_Incref(self->si_b);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF SetIntersectionIterator *DCALL
si_iter(SetIntersection *__restrict self) {
	DREF SetIntersectionIterator *result;
	result = DeeObject_MALLOC(SetIntersectionIterator);
	if unlikely(!result)
		goto done;
	result->sii_iter = DeeObject_IterSelf(self->si_a);
	if unlikely(!result->sii_iter)
		goto err_r;
	result->sii_intersect = self;
	result->sii_other     = self->si_b;
	Dee_Incref(self);
	Dee_Incref(self->si_b);
	DeeObject_Init(result, &SetIntersectionIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
si_contains(SetIntersection *self, DeeObject *item) {
	DREF DeeObject *result;
	int temp;
	result = DeeObject_ContainsObject(self->si_a, item);
	if unlikely(!result)
		goto done;
	temp = DeeObject_Bool(result);
	if unlikely(temp < 0)
		goto err_r;
	if (!temp)
		goto done;
	Dee_Decref(result);
	/* Check the second set, and forward the return value. */
	result = DeeObject_ContainsObject(self->si_b, item);
done:
	return result;
err_r:
	Dee_Clear(result);
	goto done;
}

PRIVATE struct type_seq si_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&si_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&si_contains,
};

PRIVATE struct type_member tpconst si_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SetIntersectionIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetIntersection_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetIntersection",
	/* .tp_doc      = */ DOC("(a:?Dset=!S0,b:?Dset=!S0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&si_ctor,
				/* .tp_copy_ctor = */ (void *)&si_copy,
				/* .tp_deep_ctor = */ (void *)&si_deep,
				/* .tp_any_ctor  = */ (void *)&si_init,
				TYPE_FIXED_ALLOCATOR(SetIntersection)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &si_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ si_class_members
};





/* ================================================================================ */
/*   SET DIFFERENCE                                                                 */
/* ================================================================================ */
#define sditer_ctor    siiter_ctor
#define sditer_copy    siiter_copy
#define sditer_deep    siiter_deep
#define sditer_init    siiter_init
#define sditer_fini    siiter_fini
#define sditer_visit   siiter_visit
#define sditer_cmp     siiter_cmp
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sditer_next(SetDifferenceIterator *__restrict self) {
	DREF DeeObject *result;
	int temp;
again:
	result = DeeObject_IterNext(self->sdi_iter);
	if (!ITER_ISOK(result))
		goto done;
	/* Check if contained in the second set. */
	temp = DeeObject_Contains(self->sdi_other, result);
	if (temp != 0) {
		Dee_Decref(result);
		if (temp) {
			if (!DeeThread_CheckInterrupt())
				goto again; /* If the object is contained within, read the next one. */
		}
		result = NULL; /* Error... */
	}
done:
	return result;
}

PRIVATE struct type_member tpconst sditer_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SetDifferenceIterator, sdi_diff), "->?Ert:SetDifference"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SetDifferenceIterator, sdi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__other__", STRUCT_OBJECT, offsetof(SetDifferenceIterator, sdi_other), "->?Dset"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetDifferenceIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetDifferenceIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SetDifference)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&sditer_ctor,
				/* .tp_copy_ctor = */ (void *)&sditer_copy,
				/* .tp_deep_ctor = */ (void *)&sditer_deep,
				/* .tp_any_ctor  = */ (void *)&sditer_init,
				TYPE_FIXED_ALLOCATOR(SetDifferenceIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sditer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sditer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sditer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sditer_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

#define sd_ctor su_ctor
#define sd_copy su_copy
#define sd_deep su_deep
PRIVATE WUNUSED NONNULL((1)) int DCALL
sd_init(SetDifference *__restrict self,
        size_t argc, DeeObject *const *argv) {
	self->sd_a = Dee_EmptySet;
	self->sd_b = Dee_EmptySet;
	if (DeeArg_Unpack(argc, argv, "|oo:_SetDifference", &self->sd_a, &self->sd_b))
		goto err;
	Dee_Incref(self->sd_a);
	Dee_Incref(self->sd_b);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF SetDifferenceIterator *DCALL
sd_iter(SetDifference *__restrict self) {
	DREF SetDifferenceIterator *result;
	result = DeeObject_MALLOC(SetDifferenceIterator);
	if unlikely(!result)
		goto done;
	result->sdi_iter = DeeObject_IterSelf(self->sd_a);
	if unlikely(!result->sdi_iter)
		goto err_r;
	result->sdi_diff  = self;
	result->sdi_other = self->sd_b;
	Dee_Incref(self);
	Dee_Incref(self->sd_b);
	DeeObject_Init(result, &SetDifferenceIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sd_contains(SetDifference *self, DeeObject *item) {
	int temp;
	/* Check the primary set for the object. */
	temp = DeeObject_Contains(self->sd_a, item);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return_false;
	}
	/* The object is apart of the primary set.
	 * -> Return true if it's not apart of the secondary set.
	 * -> Return false otherwise. */
	temp = DeeObject_Contains(self->sd_b, item);
	if unlikely(temp < 0)
		goto err;
	return_bool_(!temp);
err:
	return NULL;
}

PRIVATE struct type_seq sd_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sd_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sd_contains,
};

PRIVATE struct type_member tpconst sd_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SetDifferenceIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SetDifference_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SetDifference",
	/* .tp_doc      = */ DOC("(a:?Dset=!S0,b:?Dset=!S0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&sd_ctor,
				/* .tp_copy_ctor = */ (void *)&sd_copy,
				/* .tp_deep_ctor = */ (void *)&sd_deep,
				/* .tp_any_ctor  = */ (void *)&sd_init,
				TYPE_FIXED_ALLOCATOR(SetDifference)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &sd_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sd_class_members
};








INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_Union(DeeObject *lhs,
             DeeObject *rhs) {
	DREF DeeObject *result, *temp;
	if (DeeInverseSet_CheckExact(lhs)) {
		if (!DeeInverseSet_CheckExact(rhs)) {
			/* Special case: `~a | b' --> `~(a & ~b)' */
			if unlikely((rhs = DeeSet_Invert(rhs)) == NULL)
				goto err;
			temp = DeeSet_Intersection(DeeInverseSet_SET(lhs), rhs);
			Dee_Decref(rhs);
			if unlikely(!temp)
				goto err;
			result = DeeSet_Invert(temp);
			Dee_Decref(temp);
			goto done;
		}
		/* Special case: `~a | ~b' --> `~(a & b)' */
		temp = DeeSet_Intersection(DeeInverseSet_SET(lhs),
		                           DeeInverseSet_SET(rhs));
		if unlikely(!temp)
			goto err;
		result = DeeSet_Invert(temp);
		Dee_Decref(temp);
		goto done;
	}
	if (DeeInverseSet_CheckExact(rhs)) {
		/* Special case: `a | ~b' --> `~(b & ~a)' */
		if unlikely((lhs = DeeSet_Invert(lhs)) == NULL)
			goto err;
		temp = DeeSet_Intersection(DeeInverseSet_SET(rhs), lhs);
		Dee_Decref(lhs);
		if unlikely(!temp)
			goto err;
		result = DeeSet_Invert(temp);
		Dee_Decref(temp);
		goto done;
	}
	if (DeeSet_CheckEmpty(lhs))
		return_reference_(rhs);
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(lhs);
	/* Construct a set-union wrapper. */
	result = (DREF DeeObject *)DeeObject_MALLOC(SetUnion);
	if unlikely(!result)
		goto done;
	((SetUnion *)result)->su_a = lhs;
	((SetUnion *)result)->su_b = rhs;
	Dee_Incref(lhs);
	Dee_Incref(rhs);
	DeeObject_Init(result, &SetUnion_Type);
done:
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_Intersection(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result, *temp;
	if (DeeInverseSet_CheckExact(lhs)) {
		if (DeeInverseSet_CheckExact(rhs)) {
			/* Special case: `~a & ~b' -> `~(a | b)' */
			temp = DeeSet_Intersection(DeeInverseSet_SET(lhs),
			                           DeeInverseSet_SET(rhs));
			if unlikely(!temp)
				goto err;
			result = DeeSet_Invert(temp);
			Dee_Decref(temp);
			goto done;
		}
		/* Special case: `~a & b' -> `b & ~a' */
		temp = lhs;
		lhs  = rhs;
		rhs  = temp;
	}
	if (DeeSet_CheckEmpty(lhs) || DeeSet_CheckEmpty(rhs))
		return_reference_(Dee_EmptySet);
	/* Construct a set-intersection wrapper. */
	result = (DREF DeeObject *)DeeObject_MALLOC(SetIntersection);
	if unlikely(!result)
		goto done;
	ASSERT(!DeeInverseSet_CheckExact(lhs));
	((SetIntersection *)result)->si_a = lhs;
	((SetIntersection *)result)->si_b = rhs;
	Dee_Incref(lhs);
	Dee_Incref(rhs);
	DeeObject_Init(result, &SetIntersection_Type);
done:
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_Difference(DeeObject *lhs, DeeObject *rhs) {
	DREF DeeObject *result, *temp;
	if (DeeInverseSet_CheckExact(lhs)) {
		/* Special case: `~a - b' -> `~(a | b)' */
		temp = DeeSet_Union(DeeInverseSet_SET(lhs), rhs);
		if unlikely(!temp)
			goto err;
		result = DeeSet_Invert(temp);
		Dee_Decref(temp);
		goto done;
	}
	if (DeeSet_CheckEmpty(lhs))
		return_reference_(Dee_EmptySet);
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(lhs);
	/* Construct a set-difference wrapper. */
	result = (DREF DeeObject *)DeeObject_MALLOC(SetDifference);
	if unlikely(!result)
		goto done;
	ASSERT(!DeeInverseSet_CheckExact(lhs));
	((SetDifference *)result)->sd_a = lhs;
	((SetDifference *)result)->sd_b = rhs;
	Dee_Incref(lhs);
	Dee_Incref(rhs);
	DeeObject_Init(result, &SetDifference_Type);
done:
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSet_SymmetricDifference(DeeObject *lhs, DeeObject *rhs) {
	DREF SetSymmetricDifference *result;
	if (DeeSet_CheckEmpty(lhs))
		return_reference_(rhs);
	if (DeeSet_CheckEmpty(rhs))
		return_reference_(lhs);
	/* Construct a set-symmetric-difference wrapper. */
	result = DeeObject_MALLOC(SetSymmetricDifference);
	if unlikely(!result)
		goto done;
	ASSERT(!DeeInverseSet_CheckExact(lhs));
	result->ssd_a = lhs;
	result->ssd_b = rhs;
	Dee_Incref(lhs);
	Dee_Incref(rhs);
	DeeObject_Init(result, &SetSymmetricDifference_Type);
done:
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SET_C */
