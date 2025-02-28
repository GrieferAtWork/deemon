/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAPS_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAPS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/gc.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/thread.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "default-api.h"
#include "default-sets.h"

/**/
#include "default-maps.h"

#undef SSIZE_MIN
#include <hybrid/limitcore.h>
#define SSIZE_MIN __SSIZE_MIN__

DECL_BEGIN

struct map_foreach_pair_filter_keys {
	Dee_foreach_pair_t mfpfk_cb;   /* [1..1] Inner user-callback */
	void              *mfpfk_arg;  /* [?..?] Cookie for `mfpfk_cb' */
	DeeObject         *mfpfk_keys; /* [1..1] Keys that should be included/excluded (based on callback used) */
};

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL /* Using `DeeSet_OperatorContainsAsBool' */
map_foreach_pair_with_setkeys_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	struct map_foreach_pair_filter_keys *data;
	data = (struct map_foreach_pair_filter_keys *)arg;
	temp = DeeSet_OperatorContainsAsBool(data->mfpfk_keys, key);
	if (temp != 0) {
		if unlikely(temp < 0)
			goto err;
		return (*data->mfpfk_cb)(data->mfpfk_arg, key, value);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL /* Using `DeeSet_OperatorContainsAsBool' */
map_foreach_pair_without_setkeys_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	struct map_foreach_pair_filter_keys *data;
	data = (struct map_foreach_pair_filter_keys *)arg;
	temp = DeeSet_OperatorContainsAsBool(data->mfpfk_keys, key);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return (*data->mfpfk_cb)(data->mfpfk_arg, key, value);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL /* Using `DeeMap_OperatorContainsAsBool' */
map_foreach_pair_without_mapkeys_cb(void *arg, DeeObject *key, DeeObject *value) {
	int temp;
	struct map_foreach_pair_filter_keys *data;
	data = (struct map_foreach_pair_filter_keys *)arg;
	temp = DeeMap_OperatorContainsAsBool(data->mfpfk_keys, key);
	if (temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return (*data->mfpfk_cb)(data->mfpfk_arg, key, value);
	}
	return 0;
err:
	return -1;
}





/* ================================================================================ */
/*   MAP UNION                                                                      */
/* ================================================================================ */

STATIC_ASSERT(offsetof(MapUnion, mu_a) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapUnion, mu_a) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapUnion, mu_b) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapUnion, mu_b) == offsetof(ProxyObject2, po_obj2));
#define mu_copy  generic_proxy2__copy_alias12
#define mu_deep  generic_proxy2__deepcopy
#define mu_init  generic_proxy2__init
#define mu_fini  generic_proxy2__fini
#define mu_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
mu_ctor(MapUnion *__restrict self) {
	self->mu_a = Dee_EmptyMapping;
	self->mu_b = Dee_EmptyMapping;
	Dee_Incref_n(Dee_EmptyMapping, 2);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mu_bool(MapUnion *__restrict self) {
	int result = DeeMap_OperatorBool(self->mu_a);
	if likely(result == 0)
		result = DeeMap_OperatorBool(self->mu_b);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapUnionIterator *DCALL
mu_iter(MapUnion *__restrict self) {
	DREF MapUnionIterator *result = DeeGCObject_MALLOC(MapUnionIterator);
	if unlikely(!result)
		goto err;
	result->mui_iter = DeeMap_OperatorIter(self->mu_a);
	if unlikely(!result->mui_iter)
		goto err_r;
	Dee_Incref(self);
	result->mui_union = self;
	Dee_atomic_rwlock_init(&result->mui_lock);
	result->mui_in2nd = false;
	DeeObject_Init(result, &MapUnionIterator_Type);
	return (DREF MapUnionIterator *)DeeGC_Track((DREF DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mu_contains(MapUnion *__restrict self, DeeObject *key) {
	DREF DeeObject *result;
	int temp;
	result = DeeMap_OperatorContains(self->mu_a, key);
	if unlikely(!result)
		goto done;
	temp = DeeObject_Bool(result);
	if unlikely(temp < 0)
		goto err_r;
	if (temp)
		goto done;
	Dee_Decref_unlikely(result);

	/* Check the second set, and forward the return value. */
	result = DeeMap_OperatorContains(self->mu_b, key);
done:
	return result;
err_r:
	Dee_Clear(result);
	goto done;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mu_getitem(MapUnion *__restrict self, DeeObject *key) {
	DREF DeeObject *result = DeeMap_OperatorTryGetItem(self->mu_a, key);
	if (result == ITER_DONE)
		result = DeeMap_OperatorGetItem(self->mu_b, key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mu_trygetitem(MapUnion *__restrict self, DeeObject *key) {
	DREF DeeObject *result = DeeMap_OperatorTryGetItem(self->mu_a, key);
	if (result == ITER_DONE)
		result = DeeMap_OperatorTryGetItem(self->mu_b, key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mu_hasitem(MapUnion *__restrict self, DeeObject *key) {
	int result = DeeMap_OperatorHasItem(self->mu_a, key);
	if (result == 0)
		result = DeeMap_OperatorHasItem(self->mu_b, key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mu_bounditem(MapUnion *__restrict self, DeeObject *key) {
	int result = DeeMap_OperatorBoundItem(self->mu_a, key);
	if (Dee_BOUND_ISMISSING_OR_UNBOUND(result)) {
		int result2 = DeeMap_OperatorBoundItem(self->mu_b, key);
		if (Dee_BOUND_ISMISSING(result) || !Dee_BOUND_ISMISSING(result2))
			result = result2;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mu_getitem_index(MapUnion *__restrict self, size_t key) {
	DREF DeeObject *result = DeeMap_OperatorTryGetItemIndex(self->mu_a, key);
	if (result == ITER_DONE)
		result = DeeMap_OperatorGetItemIndex(self->mu_b, key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mu_trygetitem_index(MapUnion *__restrict self, size_t key) {
	DREF DeeObject *result = DeeMap_OperatorTryGetItemIndex(self->mu_a, key);
	if (result == ITER_DONE)
		result = DeeMap_OperatorTryGetItemIndex(self->mu_b, key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mu_hasitem_index(MapUnion *__restrict self, size_t key) {
	int result = DeeMap_OperatorHasItemIndex(self->mu_a, key);
	if (result == 0)
		result = DeeMap_OperatorHasItemIndex(self->mu_b, key);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mu_bounditem_index(MapUnion *__restrict self, size_t key) {
	int result = DeeMap_OperatorBoundItemIndex(self->mu_a, key);
	if (Dee_BOUND_ISMISSING_OR_UNBOUND(result)) {
		int result2 = DeeMap_OperatorBoundItemIndex(self->mu_b, key);
		if (Dee_BOUND_ISMISSING(result) || !Dee_BOUND_ISMISSING(result2))
			result = result2;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mu_getitem_string_hash(MapUnion *__restrict self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = DeeMap_OperatorTryGetItemStringHash(self->mu_a, key, hash);
	if (result == ITER_DONE)
		result = DeeMap_OperatorGetItemStringHash(self->mu_b, key, hash);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mu_trygetitem_string_hash(MapUnion *__restrict self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = DeeMap_OperatorTryGetItemStringHash(self->mu_a, key, hash);
	if (result == ITER_DONE)
		result = DeeMap_OperatorTryGetItemStringHash(self->mu_b, key, hash);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mu_hasitem_string_hash(MapUnion *__restrict self, char const *key, Dee_hash_t hash) {
	int result = DeeMap_OperatorHasItemStringHash(self->mu_a, key, hash);
	if (result == 0)
		result = DeeMap_OperatorHasItemStringHash(self->mu_b, key, hash);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mu_bounditem_string_hash(MapUnion *__restrict self, char const *key, Dee_hash_t hash) {
	int result = DeeMap_OperatorBoundItemStringHash(self->mu_a, key, hash);
	if (Dee_BOUND_ISMISSING_OR_UNBOUND(result)) {
		int result2 = DeeMap_OperatorBoundItemStringHash(self->mu_b, key, hash);
		if (Dee_BOUND_ISMISSING(result) || !Dee_BOUND_ISMISSING(result2))
			result = result2;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mu_getitem_string_len_hash(MapUnion *__restrict self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = DeeMap_OperatorTryGetItemStringLenHash(self->mu_a, key, keylen, hash);
	if (result == ITER_DONE)
		result = DeeMap_OperatorGetItemStringLenHash(self->mu_b, key, keylen, hash);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mu_trygetitem_string_len_hash(MapUnion *__restrict self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = DeeMap_OperatorTryGetItemStringLenHash(self->mu_a, key, keylen, hash);
	if (result == ITER_DONE)
		result = DeeMap_OperatorTryGetItemStringLenHash(self->mu_b, key, keylen, hash);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mu_hasitem_string_len_hash(MapUnion *__restrict self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result = DeeMap_OperatorHasItemStringLenHash(self->mu_a, key, keylen, hash);
	if (result == 0)
		result = DeeMap_OperatorHasItemStringLenHash(self->mu_b, key, keylen, hash);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mu_bounditem_string_len_hash(MapUnion *__restrict self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result = DeeMap_OperatorBoundItemStringLenHash(self->mu_a, key, keylen, hash);
	if (Dee_BOUND_ISMISSING_OR_UNBOUND(result)) {
		int result2 = DeeMap_OperatorBoundItemStringLenHash(self->mu_b, key, keylen, hash);
		if (Dee_BOUND_ISMISSING(result) || !Dee_BOUND_ISMISSING(result2))
			result = result2;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mu_foreach_pair(MapUnion *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result = DeeMap_OperatorForeachPair(self->mu_a, cb, arg);
	if likely(result >= 0) {
		Dee_ssize_t temp;
		struct map_foreach_pair_filter_keys data;
		data.mfpfk_cb            = cb;
		data.mfpfk_arg           = arg;
		data.mfpfk_keys = self->mu_a;
		temp = DeeMap_OperatorForeachPair(self->mu_b, &map_foreach_pair_without_mapkeys_cb, &data);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

PRIVATE struct type_seq mu_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mu_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mu_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mu_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&mu_foreach_pair,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&mu_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&mu_hasitem,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&mu_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&mu_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&mu_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mu_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&mu_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&mu_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&mu_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&mu_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&mu_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mu_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mu_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mu_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mu_hasitem_string_len_hash,
};

PRIVATE struct type_member mu_members[] = {
	TYPE_MEMBER_FIELD_DOC("__map_a__", STRUCT_OBJECT, offsetof(MapUnion, mu_a), "->?DMapping"),
	TYPE_MEMBER_FIELD_DOC("__map_b__", STRUCT_OBJECT, offsetof(MapUnion, mu_b), "->?DMapping"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member mu_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &MapUnionIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapUnion_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapUnion",
	/* .tp_doc      = */ DOC("()\n"
	                         "(a:?DMapping,b:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mu_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mu_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&mu_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&mu_init,
				TYPE_FIXED_ALLOCATOR(MapUnion)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mu_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mu_bool,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mu_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &mu_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ mu_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ mu_class_members,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
muiter_ctor(MapUnionIterator *__restrict self) {
	self->mui_union = (DREF MapUnion *)DeeObject_NewDefault(self->ob_type == &MapUnionIterator_Type
	                                                        ? &MapUnion_Type
	                                                        : &MapSymmetricDifference_Type);
	if unlikely(!self->mui_union)
		goto err;
	self->mui_iter = DeeMap_OperatorIter(self->mui_union->mu_a);
	if unlikely(!self->mui_iter)
		goto err_union;
	Dee_atomic_rwlock_init(&self->mui_lock);
	self->mui_in2nd = false;
	return 0;
err_union:
	Dee_Decref_likely(self->mui_union);
err:
	return -1;
}

STATIC_ASSERT(offsetof(MapUnionIterator, mui_union) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapUnionIterator, mui_union) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapUnionIterator, mui_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapUnionIterator, mui_iter) == offsetof(ProxyObject2, po_obj2));
#define muiter_fini generic_proxy2__fini

PRIVATE NONNULL((1)) void DCALL
muiter_clear(MapUnionIterator *__restrict self) {
	DREF DeeObject *iter;
	MapUnionIterator_LockWrite(self);
	iter = self->mui_iter;
	Dee_Incref(Dee_None);
	self->mui_iter = Dee_None;
	MapUnionIterator_LockEndWrite(self);
	Dee_Decref(iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
muiter_get_iter(MapUnionIterator *__restrict self) {
	DREF DeeObject *result;
	MapUnionIterator_LockRead(self);
	result = self->mui_iter;
	Dee_Incref(result);
	MapUnionIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
muiter_set_iter(MapUnionIterator *__restrict self,
                DeeObject *__restrict iter) {
	DREF DeeObject *old_iter;
	Dee_Incref(iter);
	MapUnionIterator_LockRead(self);
	old_iter       = self->mui_iter;
	self->mui_iter = iter;
	MapUnionIterator_LockEndRead(self);
	Dee_Decref(old_iter);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
muiter_get_in2nd(MapUnionIterator *__restrict self) {
	bool result;
	COMPILER_BARRIER();
	result = self->mui_in2nd;
	COMPILER_BARRIER();
	return_bool(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
muiter_set_in2nd(MapUnionIterator *__restrict self,
                 DeeObject *__restrict value) {
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		return newval;
	MapUnionIterator_LockWrite(self);
	self->mui_in2nd = !!newval;
	MapUnionIterator_LockEndWrite(self);
	return 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
muiter_visit(MapUnionIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->mui_union);
	MapUnionIterator_LockRead(self);
	Dee_Visit(self->mui_iter);
	MapUnionIterator_LockEndRead(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
muiter_copy(MapUnionIterator *__restrict self,
            MapUnionIterator *__restrict other) {
	DREF DeeObject *iter;
	MapUnionIterator_LockRead(other);
	iter            = other->mui_iter;
	self->mui_in2nd = other->mui_in2nd;
	Dee_Incref(iter);
	MapUnionIterator_LockEndRead(other);
	self->mui_iter = DeeObject_Copy(iter);
	Dee_Decref(iter);
	if unlikely(!self->mui_iter)
		goto err;
	Dee_atomic_rwlock_init(&self->mui_lock);
	self->mui_union = other->mui_union;
	Dee_Incref(self->mui_union);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
muiter_deep(MapUnionIterator *__restrict self,
            MapUnionIterator *__restrict other) {
	DREF DeeObject *iter;
	MapUnionIterator_LockRead(other);
	iter            = other->mui_iter;
	self->mui_in2nd = other->mui_in2nd;
	Dee_Incref(iter);
	MapUnionIterator_LockEndRead(other);
	self->mui_iter = DeeObject_DeepCopy(iter);
	Dee_Decref(iter);
	if unlikely(!self->mui_iter)
		goto err;
	Dee_atomic_rwlock_init(&self->mui_lock);
	self->mui_union = (DREF MapUnion *)DeeObject_DeepCopy((DeeObject *)other->mui_union);
	if unlikely(!self->mui_union)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->mui_iter);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
muiter_init(MapUnionIterator *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv,
	                  Dee_TYPE(self) == &MapUnionIterator_Type
	                  ? "o:_MapUnionIterator"
	                  : "o:_MapSymmetricDifferenceIterator",
	                  &self->mui_union))
		goto err;
	if (DeeObject_AssertTypeExact(self->mui_union,
	                              Dee_TYPE(self) == &MapUnionIterator_Type
	                              ? &MapUnion_Type
	                              : &MapSymmetricDifference_Type))
		goto err;
	if ((self->mui_iter = DeeMap_OperatorIter(self->mui_union->mu_a)) == NULL)
		goto err;
	Dee_Incref(self->mui_union);
	Dee_atomic_rwlock_init(&self->mui_lock);
	self->mui_in2nd = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
muiter_nextpair(MapUnionIterator *__restrict self,
                /*out*/ DREF DeeObject *key_and_value[2]) {
	bool is_second;
	int result;
	DREF DeeObject *iter;
again:
	MapUnionIterator_LockRead(self);
	iter      = self->mui_iter;
	is_second = self->mui_in2nd;
	Dee_Incref(iter);
	MapUnionIterator_LockEndRead(self);
read_from_iter:
	result = DeeObject_IterNextPair(iter, key_and_value);
	Dee_Decref(iter);
	if (is_second) {
		int key_exists;
		if (result != 0)
			goto done; /* Error or ITER_DONE */
		key_exists = DeeMap_OperatorContainsAsBool(self->mui_union->mu_a,
		                                           key_and_value[0]);
		if likely(key_exists == 0)
			goto done; /* Key hadn't been enumerated, yet */
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		if unlikely(key_exists < 0)
			goto err;
		if unlikely(DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	if (result != 0) {
		DREF DeeObject *iter2;
		if unlikely(result < 0)
			goto done;

		/* End of first iterator -> try to switch to the second iterator. */
		iter2 = DeeMap_OperatorIter(self->mui_union->mu_b);
		if unlikely(!iter2)
			goto err;
		MapUnionIterator_LockWrite(self);
		if unlikely(self->mui_in2nd) {
			MapUnionIterator_LockEndWrite(self);
			Dee_Decref(iter2);
			goto again;
		}
		iter = self->mui_iter;
		self->mui_iter  = iter2;
		self->mui_in2nd = true;
		Dee_Incref(iter2);
		MapUnionIterator_LockEndWrite(self);
		Dee_Decref(iter);
		iter = iter2;
		goto read_from_iter;
	}
done:
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
muiter_nextkey(MapUnionIterator *__restrict self) {
	bool is_second;
	DREF DeeObject *result;
	DREF DeeObject *iter;
again:
	MapUnionIterator_LockRead(self);
	iter      = self->mui_iter;
	is_second = self->mui_in2nd;
	Dee_Incref(iter);
	MapUnionIterator_LockEndRead(self);
read_from_iter:
	result = DeeObject_IterNextKey(iter);
	Dee_Decref(iter);
	if (is_second) {
		int key_exists;
		if (!ITER_ISOK(result))
			goto done; /* Error or ITER_DONE */
		key_exists = DeeMap_OperatorContainsAsBool(self->mui_union->mu_a, result);
		if likely(key_exists == 0)
			goto done; /* Key hadn't been enumerated, yet */
		Dee_Decref(result);
		if unlikely(key_exists < 0)
			goto err;
		if unlikely(DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	if (!ITER_ISOK(result)) {
		DREF DeeObject *iter2;
		if unlikely(!result)
			goto done;

		/* End of first iterator -> try to switch to the second iterator. */
		iter2 = DeeMap_OperatorIter(self->mui_union->mu_b);
		if unlikely(!iter2)
			goto err;
		MapUnionIterator_LockWrite(self);
		if unlikely(self->mui_in2nd) {
			MapUnionIterator_LockEndWrite(self);
			Dee_Decref(iter2);
			goto again;
		}
		iter = self->mui_iter;
		self->mui_iter  = iter2;
		self->mui_in2nd = true;
		Dee_Incref(iter2);
		MapUnionIterator_LockEndWrite(self);
		Dee_Decref(iter);
		iter = iter2;
		goto read_from_iter;
	}
done:
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
muiter_advance(MapUnionIterator *__restrict self, size_t step) {
	size_t result, temp;
	DREF DeeObject *iter;
	MapUnionIterator_LockRead(self);
	if (self->mui_in2nd) {
		MapUnionIterator_LockEndRead(self);
		return DeeObject_DefaultIterAdvanceWithIterNextKey((DeeObject *)self, step);
	}
	iter = self->mui_iter;
	Dee_Incref(iter);
	MapUnionIterator_LockEndRead(self);
	result = DeeObject_IterAdvance(iter, step);
	Dee_Decref(iter);
	if (result >= step /* || unlikely(result == (size_t)-1)*/)
		return result; /* Error, or fully "step" was fully applied */
	/* Must use default for remaining steps. */
	temp = DeeObject_DefaultIterAdvanceWithIterNextKey((DeeObject *)self, step - result);
	if unlikely(temp == (size_t)-1)
		goto err;
	return result + temp;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
muiter_hash(MapUnionIterator *self) {
	Dee_hash_t result;
	bool my_2nd;
	DREF DeeObject *my_iter;
	MapUnionIterator_LockRead(self);
	my_iter = self->mui_iter;
	my_2nd  = self->mui_in2nd;
	MapUnionIterator_LockEndRead(self);
	result = Dee_HashCombine(my_2nd ? 1 : 0, DeeObject_Hash(my_iter));
	Dee_Decref(my_iter);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
muiter_compare(MapUnionIterator *self,
               MapUnionIterator *other) {
	int result;
	bool my_2nd, ot_2nd;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	MapUnionIterator_LockRead(self);
	my_iter = self->mui_iter;
	my_2nd  = self->mui_in2nd;
	MapUnionIterator_LockEndRead(self);
	MapUnionIterator_LockRead(other);
	ot_iter = other->mui_iter;
	ot_2nd  = other->mui_in2nd;
	MapUnionIterator_LockEndRead(other);
	if (my_2nd != ot_2nd) {
		result = Dee_CompareNe(my_2nd, ot_2nd);
	} else {
		result = DeeObject_Compare(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
muiter_compare_eq(MapUnionIterator *self,
                  MapUnionIterator *other) {
	int result;
	bool my_2nd, ot_2nd;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	MapUnionIterator_LockRead(self);
	my_iter = self->mui_iter;
	my_2nd  = self->mui_in2nd;
	MapUnionIterator_LockEndRead(self);
	MapUnionIterator_LockRead(other);
	ot_iter = other->mui_iter;
	ot_2nd  = other->mui_in2nd;
	MapUnionIterator_LockEndRead(other);
	if (my_2nd != ot_2nd) {
		result = 1;
	} else {
		result = DeeObject_CompareEq(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp muiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&muiter_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&muiter_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&muiter_compare,
};

PRIVATE struct type_member tpconst muiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(MapUnionIterator, mui_union), "->?Ert:MapUnion"),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst muiter_getsets[] = {
	TYPE_GETSET_F("__iter__", &muiter_get_iter, NULL, &muiter_set_iter, METHOD_FNOREFESCAPE, "->?DIterator"),
	TYPE_GETSET_F("__in2nd__", &muiter_get_in2nd, NULL, &muiter_set_in2nd, METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETSET_END
};

PRIVATE struct type_iterator muiter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&muiter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&muiter_nextkey,
	/* .tp_nextvalue = */ NULL,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&muiter_advance,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
muiter_bool(MapUnionIterator *__restrict self) {
	bool is_second;
	int result;
	DREF DeeObject *iter;
	MapUnionIterator_LockRead(self);
	iter      = self->mui_iter;
	is_second = self->mui_in2nd;
	Dee_Incref(iter);
	MapUnionIterator_LockEndRead(self);
	result = DeeObject_Bool(iter);
	Dee_Decref(iter);
	if (result != 0)
		return result;
	if (is_second)
		return 0; /* Nothing left to enumerate */
	/* Special case: iterator is now located at end of first mapping
	 * -> it will be able to yield more items if the second mapping
	 *    contains at least 1 key not present in the first mapping. */
	return MapDifferenceMapKeys_NonEmpty(self->mui_union->mu_b,
	                                     self->mui_union->mu_a);
}

PRIVATE struct type_gc muiter_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *))&muiter_clear
};

INTERN DeeTypeObject MapUnionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapUnionIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(mu:?Ert:MapUnion)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&muiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&muiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&muiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&muiter_init,
				TYPE_FIXED_ALLOCATOR_GC(MapUnionIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&muiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&muiter_bool,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&muiter_visit,
	/* .tp_gc            = */ &muiter_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &muiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ muiter_getsets,
	/* .tp_members       = */ muiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};





/* ================================================================================ */
/*   MAP INTERSECTION                                                               */
/* ================================================================================ */

STATIC_ASSERT(offsetof(MapIntersection, mi_map) == offsetof(MapUnion, mu_a));
STATIC_ASSERT(offsetof(MapIntersection, mi_keys) == offsetof(MapUnion, mu_b));
#define mi_ctor mu_ctor

STATIC_ASSERT(offsetof(MapIntersection, mi_map) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapIntersection, mi_map) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapIntersection, mi_keys) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapIntersection, mi_keys) == offsetof(ProxyObject2, po_obj2));
#define mi_copy  generic_proxy2__copy_alias12
#define mi_deep  generic_proxy2__deepcopy
#define mi_init  generic_proxy2__init
#define mi_fini  generic_proxy2__fini
#define mi_visit generic_proxy2__visit

#define MAP_CONTAINSANY_FOREACH__FOUND SSIZE_MIN
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
map_containsany_foreach_cb(void *arg, DeeObject *key) {
	int is_contained = DeeMap_OperatorContainsAsBool((DeeObject *)arg, key);
	if (is_contained != 0) {
		if unlikely(is_contained < 0)
			goto err;
		return MAP_CONTAINSANY_FOREACH__FOUND;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mi_bool(MapIntersection *__restrict self) {
	/* >> mi_map.CONTAINS_ANY(mi_keys) */
	Dee_ssize_t status = DeeSet_OperatorForeach(self->mi_keys, &map_containsany_foreach_cb, self->mi_map);
	if (status == 0)
		return 0;
	if (status == MAP_CONTAINSANY_FOREACH__FOUND)
		return 1;
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapIntersectionIterator *DCALL
mi_iter(MapIntersection *__restrict self) {
	DREF MapIntersectionIterator *result = DeeObject_MALLOC(MapIntersectionIterator);
	if unlikely(!result)
		goto err;
	result->mii_iter = DeeMap_OperatorIter(self->mi_map);
	if unlikely(!result->mii_iter)
		goto err_r;
	Dee_Incref(self);
	result->mii_intersect = self;
	result->mii_keys      = self->mi_keys;
	DeeObject_Init(result, &MapIntersectionIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mi_contains(MapIntersection *__restrict self, DeeObject *key) {
	int contains_a = DeeMap_OperatorContainsAsBool(self->mi_map, key);
	if (contains_a <= 0) {
		if unlikely(contains_a < 0)
			goto err;
		return_false;
	}
	return DeeSet_OperatorContains(self->mi_keys, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mi_getitem(MapIntersection *__restrict self, DeeObject *key) {
	int temp = DeeSet_OperatorContainsAsBool(self->mi_keys, key);
	if unlikely(temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		goto err_key;
	}
	return DeeMap_OperatorGetItem(self->mi_map, key);
err_key:
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mi_trygetitem(MapIntersection *__restrict self, DeeObject *key) {
	int temp = DeeSet_OperatorContainsAsBool(self->mi_keys, key);
	if unlikely(temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return ITER_DONE;
	}
	return DeeMap_OperatorTryGetItem(self->mi_map, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mi_hasitem(MapIntersection *__restrict self, DeeObject *key) {
	int temp = DeeSet_OperatorContainsAsBool(self->mi_keys, key);
	if unlikely(temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return 0;
	}
	return DeeMap_OperatorHasItem(self->mi_map, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mi_bounditem(MapIntersection *__restrict self, DeeObject *key) {
	int temp = DeeSet_OperatorContainsAsBool(self->mi_keys, key);
	if unlikely(temp <= 0) {
		if unlikely(temp < 0)
			goto err;
		return Dee_BOUND_MISSING;
	}
	return DeeMap_OperatorBoundItem(self->mi_map, key);
err:
	return Dee_BOUND_ERR;
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mi_foreach_pair(MapIntersection *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct map_foreach_pair_filter_keys data;
	data.mfpfk_cb   = cb;
	data.mfpfk_arg  = arg;
	data.mfpfk_keys = self->mi_keys;
	return DeeMap_OperatorForeachPair(self->mi_map, &map_foreach_pair_with_setkeys_cb, &data);
}

PRIVATE struct type_seq mi_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mi_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mi_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mi_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&mi_foreach_pair,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&mi_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&mi_hasitem,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mi_trygetitem,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_member tpconst mi_members[] = {
	TYPE_MEMBER_FIELD_DOC("__map__", STRUCT_OBJECT, offsetof(MapIntersection, mi_map), "->?DMapping"),
	TYPE_MEMBER_FIELD_DOC("__keys__", STRUCT_OBJECT, offsetof(MapIntersection, mi_keys), "->?DSet"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst mi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &MapIntersectionIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapIntersection_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapIntersection",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping,keys:?DSet)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&mi_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&mi_init,
				TYPE_FIXED_ALLOCATOR(MapIntersection)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mi_bool,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &mi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ mi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ mi_class_members,
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
miiter_ctor(MapIntersectionIterator *__restrict self) {
	self->mii_intersect = (DREF MapIntersection *)DeeObject_NewDefault(self->ob_type == &MapIntersectionIterator_Type
	                                                                   ? &MapIntersection_Type
	                                                                   : &MapDifference_Type);
	if unlikely(!self->mii_intersect)
		goto err;
	self->mii_iter = DeeMap_OperatorIter(self->mii_intersect->mi_map);
	if unlikely(!self->mii_iter)
		goto err_intersect;
	self->mii_keys = self->mii_intersect->mi_keys;
	return 0;
err_intersect:
	Dee_Decref(self->mii_intersect);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
miiter_copy(MapIntersectionIterator *self, MapIntersectionIterator *other) {
	self->mii_iter = DeeObject_Copy(other->mii_iter);
	if unlikely(!self->mii_iter)
		goto err;
	self->mii_intersect = other->mii_intersect;
	Dee_Incref(self->mii_intersect);
	self->mii_keys = other->mii_keys;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
miiter_deep(MapIntersectionIterator *self, MapIntersectionIterator *other) {
	self->mii_intersect = (DREF MapIntersection *)DeeObject_DeepCopy((DeeObject *)other->mii_intersect);
	if unlikely(!self->mii_intersect)
		goto err;
	self->mii_iter = DeeObject_DeepCopy(other->mii_iter);
	if unlikely(!self->mii_iter)
		goto err_intersect;
	self->mii_keys = self->mii_intersect->mi_keys;
	return 0;
err_intersect:
	Dee_Decref(self->mii_intersect);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
miiter_init(MapIntersectionIterator *self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv,
	                  Dee_TYPE(self) == &MapIntersectionIterator_Type
	                  ? "o:_MapIntersectionIterator"
	                  : "o:_MapDifferenceIterator",
	                  &self->mii_intersect))
		goto err;
	if (DeeObject_AssertType(self->mii_intersect,
	                         Dee_TYPE(self) == &MapIntersectionIterator_Type
	                         ? &MapIntersection_Type
	                         : &MapDifference_Type))
		goto err;
	self->mii_iter = DeeMap_OperatorIter(self->mii_intersect->mi_map);
	if unlikely(!self->mii_iter)
		goto err;
	Dee_Incref(self->mii_intersect);
	self->mii_keys = self->mii_intersect->mi_keys;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(MapIntersectionIterator, mii_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapIntersectionIterator, mii_iter) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapIntersectionIterator, mii_intersect) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapIntersectionIterator, mii_intersect) == offsetof(ProxyObject2, po_obj2));
#define miiter_fini  generic_proxy2__fini
#define miiter_visit generic_proxy2__visit

STATIC_ASSERT(offsetof(MapIntersectionIterator, mii_iter) == offsetof(ProxyObject, po_obj));
#define miiter_hash          generic_proxy__hash_recursive
#define miiter_compare       generic_proxy__compare_recursive
#define miiter_compare_eq    generic_proxy__compare_eq_recursive
#define miiter_trycompare_eq generic_proxy__trycompare_eq_recursive
#define miiter_cmp           generic_proxy__cmp_recursive

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
miiter_nextpair(MapIntersectionIterator *__restrict self,
                /*out*/ DREF DeeObject *key_and_value[2]) {
	int result, temp;
again:
	result = DeeObject_IterNextPair(self->mii_iter, key_and_value);
	if (result != 0)
		return result; /* error, or ITER_DONE */
	temp = DeeSet_OperatorContainsAsBool(self->mii_keys, key_and_value[0]);
	if likely(temp > 0)
		return 0;
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	if unlikely(temp < 0)
		goto err;
	if (DeeThread_CheckInterrupt())
		goto err;
	goto again;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
miiter_nextkey(MapIntersectionIterator *__restrict self) {
	int temp;
	DREF DeeObject *result;
again:
	result = DeeObject_IterNextKey(self->mii_iter);
	if (!ITER_ISOK(result))
		return result; /* error, or ITER_DONE */
	temp = DeeSet_OperatorContainsAsBool(self->mii_keys, result);
	if likely(temp > 0)
		return result;
	Dee_Decref(result);
	if unlikely(temp < 0)
		goto err;
	if (DeeThread_CheckInterrupt())
		goto err;
	goto again;
err:
	return NULL;
}

PRIVATE struct type_iterator miiter_iterator = {
	/* .tp_nextpair = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&miiter_nextpair,
	/* .tp_nextkey  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&miiter_nextkey,
};

PRIVATE struct type_member tpconst miiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(MapIntersectionIterator, mii_intersect), "->?Ert:MapIntersection"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(MapIntersectionIterator, mii_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__keys__", STRUCT_OBJECT, offsetof(MapIntersectionIterator, mii_keys), "->?DSet"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapIntersectionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapIntersectionIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(mi:?Ert:MapIntersection)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&miiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&miiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&miiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&miiter_init,
				TYPE_FIXED_ALLOCATOR(MapIntersectionIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&miiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&miiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &miiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &miiter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ miiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};





/* ================================================================================ */
/*   MAP DIFFERENCE                                                                 */
/* ================================================================================ */

STATIC_ASSERT(offsetof(MapDifference, md_map) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapDifference, md_map) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapDifference, md_keys) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapDifference, md_keys) == offsetof(ProxyObject2, po_obj2));
#define md_copy  generic_proxy2__copy_alias12
#define md_deep  generic_proxy2__deepcopy
#define md_fini  generic_proxy2__fini
#define md_visit generic_proxy2__visit

STATIC_ASSERT(offsetof(MapDifference, md_map) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(MapDifference, md_keys) == offsetof(ProxyObject2, po_obj2));
#define md_init generic_proxy2__init

PRIVATE WUNUSED NONNULL((1)) int DCALL
md_ctor(MapDifference *__restrict self) {
	self->md_map = Dee_EmptyMapping;
	Dee_Incref(Dee_EmptyMapping);
	self->md_keys = Dee_EmptySet;
	Dee_Incref(Dee_EmptySet);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
md_bool(MapDifference *__restrict self) {
	return MapDifference_NonEmpty(self->md_map, self->md_keys);
}

PRIVATE WUNUSED NONNULL((1)) DREF MapDifferenceIterator *DCALL
md_iter(MapDifference *__restrict self) {
	DREF MapDifferenceIterator *result = DeeObject_MALLOC(MapDifferenceIterator);
	if unlikely(!result)
		goto err;
	result->mdi_iter = DeeMap_OperatorIter(self->md_map);
	if unlikely(!result->mdi_iter)
		goto err_r;
	Dee_Incref(self);
	result->mdi_diff = self;
	result->mdi_keys = self->md_keys;
	DeeObject_Init(result, &MapDifferenceIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
md_contains(MapDifference *self, DeeObject *key) {
	int in_keys = DeeSet_OperatorContainsAsBool(self->md_keys, key);
	if (in_keys != 0) {
		if unlikely(in_keys < 0)
			goto err;
		return_false;
	}
	return DeeMap_OperatorContains(self->md_map, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
md_getitem(MapDifference *self, DeeObject *key) {
	int in_keys = DeeSet_OperatorContainsAsBool(self->md_keys, key);
	if unlikely(in_keys != 0) {
		if unlikely(in_keys < 0)
			goto err;
		err_unknown_key((DeeObject *)self, key);
		goto err;
	}
	return DeeMap_OperatorGetItem(self->md_map, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
md_trygetitem(MapDifference *self, DeeObject *key) {
	int in_keys = DeeSet_OperatorContainsAsBool(self->md_keys, key);
	if unlikely(in_keys != 0) {
		if unlikely(in_keys < 0)
			goto err;
		return ITER_DONE;
	}
	return DeeMap_OperatorTryGetItem(self->md_map, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
md_bounditem(MapDifference *self, DeeObject *key) {
	int in_keys = DeeSet_OperatorContainsAsBool(self->md_keys, key);
	if unlikely(in_keys != 0) {
		if unlikely(in_keys < 0)
			goto err;
		return Dee_BOUND_MISSING;
	}
	return DeeMap_OperatorBoundItem(self->md_map, key);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
md_hasitem(MapDifference *self, DeeObject *key) {
	int in_keys = DeeSet_OperatorContainsAsBool(self->md_keys, key);
	if unlikely(in_keys != 0) {
		if unlikely(in_keys < 0)
			goto err;
		return 0;
	}
	return DeeMap_OperatorHasItem(self->md_map, key);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
md_foreach_pair(MapDifference *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	struct map_foreach_pair_filter_keys data;
	data.mfpfk_cb   = cb;
	data.mfpfk_arg  = arg;
	data.mfpfk_keys = self->md_keys;
	return DeeMap_OperatorForeachPair(self->md_map, &map_foreach_pair_without_setkeys_cb, &data);
}

PRIVATE struct type_seq md_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&md_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&md_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&md_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&md_foreach_pair,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&md_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&md_hasitem,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&md_trygetitem,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

#if 1
STATIC_ASSERT(offsetof(MapDifference, md_map) == offsetof(MapIntersection, mi_map));
STATIC_ASSERT(offsetof(MapDifference, md_keys) == offsetof(MapIntersection, mi_keys));
#define md_members mi_members
#else
PRIVATE struct type_member tpconst md_members[] = {
	TYPE_MEMBER_FIELD_DOC("__map__", STRUCT_OBJECT, offsetof(MapDifference, md_map), "->?DMapping"),
	TYPE_MEMBER_FIELD_DOC("__keys__", STRUCT_OBJECT, offsetof(MapDifference, md_keys), "->?DSet"),
	TYPE_MEMBER_END
};
#endif

PRIVATE struct type_member tpconst md_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &MapDifferenceIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapDifference_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapDifference",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping,keys:?DSet)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&md_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&md_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&md_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&md_init,
				TYPE_FIXED_ALLOCATOR(MapDifference)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&md_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&md_bool,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&md_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &md_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ md_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ md_class_members,
};


STATIC_ASSERT(offsetof(MapDifferenceIterator, mdi_iter) == offsetof(MapIntersectionIterator, mii_iter));
STATIC_ASSERT(offsetof(MapDifferenceIterator, mdi_diff) == offsetof(MapIntersectionIterator, mii_intersect));
STATIC_ASSERT(offsetof(MapDifferenceIterator, mdi_keys) == offsetof(MapIntersectionIterator, mii_keys));
#define mditer_ctor miiter_ctor
#define mditer_copy miiter_copy
#define mditer_deep miiter_deep
#define mditer_init miiter_init

STATIC_ASSERT(offsetof(MapDifferenceIterator, mdi_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapDifferenceIterator, mdi_iter) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapDifferenceIterator, mdi_diff) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapDifferenceIterator, mdi_diff) == offsetof(ProxyObject2, po_obj2));
#define mditer_fini  generic_proxy2__fini
#define mditer_visit generic_proxy2__visit

STATIC_ASSERT(offsetof(MapDifferenceIterator, mdi_iter) == offsetof(ProxyObject, po_obj));
#define mditer_hash          generic_proxy__hash_recursive
#define mditer_compare       generic_proxy__compare_recursive
#define mditer_compare_eq    generic_proxy__compare_eq_recursive
#define mditer_trycompare_eq generic_proxy__trycompare_eq_recursive
#define mditer_cmp           generic_proxy__cmp_recursive

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mditer_nextpair(MapDifferenceIterator *__restrict self,
                /*out*/ DREF DeeObject *key_and_value[2]) {
	int result, temp;
again:
	result = DeeObject_IterNextPair(self->mdi_iter, key_and_value);
	if (result != 0)
		return result; /* error, or ITER_DONE */
	temp = DeeSet_OperatorContainsAsBool(self->mdi_keys, key_and_value[0]);
	if likely(temp == 0)
		return 0;
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	if unlikely(temp < 0)
		goto err;
	if (DeeThread_CheckInterrupt())
		goto err;
	goto again;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mditer_nextkey(MapDifferenceIterator *__restrict self) {
	int temp;
	DREF DeeObject *result;
again:
	result = DeeObject_IterNextKey(self->mdi_iter);
	if (!ITER_ISOK(result))
		return result; /* error, or ITER_DONE */
	temp = DeeSet_OperatorContainsAsBool(self->mdi_keys, result);
	if likely(temp == 0)
		return result;
	Dee_Decref(result);
	if unlikely(temp < 0)
		goto err;
	if (DeeThread_CheckInterrupt())
		goto err;
	goto again;
err:
	return NULL;
}

PRIVATE struct type_iterator mditer_iterator = {
	/* .tp_nextpair = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&mditer_nextpair,
	/* .tp_nextkey  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mditer_nextkey,
};

PRIVATE struct type_member tpconst mditer_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(MapDifferenceIterator, mdi_diff), "->?Ert:MapDifference"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(MapDifferenceIterator, mdi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__keys__", STRUCT_OBJECT, offsetof(MapDifferenceIterator, mdi_keys), "->?DSet"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapDifferenceIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapDifferenceIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(md:?Ert:MapDifference)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mditer_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mditer_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&mditer_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&mditer_init,
				TYPE_FIXED_ALLOCATOR(MapDifferenceIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mditer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mditer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &mditer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &mditer_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ mditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};





/* ================================================================================ */
/*   MAP SYMMETRIC DIFFERENCE                                                       */
/* ================================================================================ */

STATIC_ASSERT(offsetof(MapSymmetricDifference, msd_a) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapSymmetricDifference, msd_a) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapSymmetricDifference, msd_b) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapSymmetricDifference, msd_b) == offsetof(ProxyObject2, po_obj2));
#define msd_copy  generic_proxy2__copy_alias12
#define msd_deep  generic_proxy2__deepcopy
#define msd_init  generic_proxy2__init
#define msd_fini  generic_proxy2__fini
#define msd_visit generic_proxy2__visit

STATIC_ASSERT(offsetof(MapSymmetricDifference, msd_a) == offsetof(MapUnion, mu_a) ||
              offsetof(MapSymmetricDifference, msd_a) == offsetof(MapUnion, mu_b));
STATIC_ASSERT(offsetof(MapSymmetricDifference, msd_b) == offsetof(MapUnion, mu_a) ||
              offsetof(MapSymmetricDifference, msd_b) == offsetof(MapUnion, mu_b));
#define msd_ctor mu_ctor

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_bool(MapSymmetricDifference *__restrict self) {
	/* `(a ^ b) != {}'    <=>    `a.keys != b.keys' */
	int result;
	DREF DeeObject *a_keys, *b_keys;
	a_keys = DeeMap_InvokeKeys(self->msd_a);
	if unlikely(!a_keys)
		goto err;
	b_keys = DeeMap_InvokeKeys(self->msd_b);
	if unlikely(!b_keys)
		goto err_a_keys;
	result = DeeSet_OperatorCompareEq(a_keys, b_keys);
	Dee_Decref(b_keys);
	Dee_Decref(a_keys);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return result == 0 ? 0 : 1;
err_a_keys:
	Dee_Decref(a_keys);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapSymmetricDifferenceIterator *DCALL
msd_iter(MapSymmetricDifference *__restrict self) {
	DREF MapSymmetricDifferenceIterator *result = DeeGCObject_MALLOC(MapSymmetricDifferenceIterator);
	if unlikely(!result)
		goto err;
	result->msdi_iter = DeeMap_OperatorIter(self->msd_a);
	if unlikely(!result->msdi_iter)
		goto err_r;
	Dee_Incref(self);
	result->msdi_symdiff = self;
	Dee_atomic_rwlock_init(&result->msdi_lock);
	result->msdi_in2nd = false;
	DeeObject_Init(result, &MapSymmetricDifferenceIterator_Type);
	return (DREF MapSymmetricDifferenceIterator *)DeeGC_Track((DREF DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_contains(MapSymmetricDifference *__restrict self, DeeObject *key) {
	int contains_a, contains_b;
	contains_a = DeeMap_OperatorContainsAsBool(self->msd_a, key);
	if unlikely(contains_a < 0)
		goto err;
	contains_b = DeeMap_OperatorContainsAsBool(self->msd_b, key);
	if unlikely(contains_b < 0)
		goto err;
	return_bool((!!contains_a) != (!!contains_b));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_getitem(MapSymmetricDifference *__restrict self, DeeObject *key) {
	int exists;
	DREF DeeObject *result;
	result = DeeMap_OperatorTryGetItem(self->msd_a, key);
	if (result == ITER_DONE)
		return DeeMap_OperatorGetItem(self->msd_b, key);
	if unlikely(!result)
		goto err;
	exists = DeeMap_OperatorHasItem(self->msd_b, key);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		goto err_key_r;
	}
	return result;
err_key_r:
	Dee_Decref(result);
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_trygetitem(MapSymmetricDifference *__restrict self, DeeObject *key) {
	int exists;
	DREF DeeObject *result;
	result = DeeMap_OperatorTryGetItem(self->msd_a, key);
	if (result == ITER_DONE)
		return DeeMap_OperatorTryGetItem(self->msd_b, key);
	if unlikely(!result)
		goto err;
	exists = DeeMap_OperatorHasItem(self->msd_b, key);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		Dee_Decref(result);
		return ITER_DONE;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_hasitem(MapSymmetricDifference *__restrict self, DeeObject *key) {
	int exists, result;
	result = DeeMap_OperatorHasItem(self->msd_a, key);
	if (result == 0)
		return DeeMap_OperatorHasItem(self->msd_b, key);
	if unlikely(result < 0)
		goto err;
	exists = DeeMap_OperatorHasItem(self->msd_b, key);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		return 0;
	}
	return 1;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_bounditem(MapSymmetricDifference *__restrict self, DeeObject *key) {
	int exists, result;
	result = DeeMap_OperatorBoundItem(self->msd_a, key);
	if (Dee_BOUND_ISMISSING_OR_UNBOUND(result)) {
		int result2 = DeeMap_OperatorBoundItem(self->msd_b, key);
		if (result == result2) {
			result = Dee_BOUND_MISSING;
		} else if (Dee_BOUND_ISMISSING(result) || !Dee_BOUND_ISMISSING(result2)) {
			result = result2;
		}
		return result;
	}
	if unlikely(Dee_BOUND_ISERR(result))
		goto err;
	exists = DeeMap_OperatorBoundItem(self->msd_b, key);
	if unlikely(!Dee_BOUND_ISMISSING_OR_UNBOUND(exists)) {
		if unlikely(Dee_BOUND_ISERR(exists))
			goto err;
		return exists;
	}
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_getitem_index(MapSymmetricDifference *__restrict self, size_t key) {
	int exists;
	DREF DeeObject *result;
	result = DeeMap_OperatorTryGetItemIndex(self->msd_a, key);
	if (result == ITER_DONE)
		return DeeMap_OperatorGetItemIndex(self->msd_b, key);
	if unlikely(!result)
		goto err;
	exists = DeeMap_OperatorHasItemIndex(self->msd_b, key);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		goto err_key_r;
	}
	return result;
err_key_r:
	Dee_Decref(result);
	err_unknown_key_int((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_trygetitem_index(MapSymmetricDifference *__restrict self, size_t key) {
	int exists;
	DREF DeeObject *result;
	result = DeeMap_OperatorTryGetItemIndex(self->msd_a, key);
	if (result == ITER_DONE)
		return DeeMap_OperatorTryGetItemIndex(self->msd_b, key);
	if unlikely(!result)
		goto err;
	exists = DeeMap_OperatorHasItemIndex(self->msd_b, key);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		Dee_Decref(result);
		return ITER_DONE;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_hasitem_index(MapSymmetricDifference *__restrict self, size_t key) {
	int exists, result;
	result = DeeMap_OperatorHasItemIndex(self->msd_a, key);
	if (result == 0)
		return DeeMap_OperatorHasItemIndex(self->msd_b, key);
	if unlikely(result < 0)
		goto err;
	exists = DeeMap_OperatorHasItemIndex(self->msd_b, key);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		return 0;
	}
	return 1;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_bounditem_index(MapSymmetricDifference *__restrict self, size_t key) {
	int exists, result;
	result = DeeMap_OperatorBoundItemIndex(self->msd_a, key);
	if (Dee_BOUND_ISMISSING_OR_UNBOUND(result)) {
		int result2 = DeeMap_OperatorBoundItemIndex(self->msd_b, key);
		if (result == result2) {
			result = Dee_BOUND_MISSING;
		} else if (Dee_BOUND_ISMISSING(result) || !Dee_BOUND_ISMISSING(result2)) {
			result = result2;
		}
		return result;
	}
	if unlikely(Dee_BOUND_ISERR(result))
		goto err;
	exists = DeeMap_OperatorBoundItemIndex(self->msd_b, key);
	if unlikely(!Dee_BOUND_ISMISSING_OR_UNBOUND(exists)) {
		if unlikely(Dee_BOUND_ISERR(exists))
			goto err;
		return exists;
	}
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_getitem_string_hash(MapSymmetricDifference *__restrict self,
                        char const *key, Dee_hash_t hash) {
	int exists;
	DREF DeeObject *result;
	result = DeeMap_OperatorTryGetItemStringHash(self->msd_a, key, hash);
	if (result == ITER_DONE)
		return DeeMap_OperatorGetItemStringHash(self->msd_b, key, hash);
	if unlikely(!result)
		goto err;
	exists = DeeMap_OperatorHasItemStringHash(self->msd_b, key, hash);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		goto err_key_r;
	}
	return result;
err_key_r:
	Dee_Decref(result);
	err_unknown_key_str((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_trygetitem_string_hash(MapSymmetricDifference *__restrict self,
                           char const *key, Dee_hash_t hash) {
	int exists;
	DREF DeeObject *result;
	result = DeeMap_OperatorTryGetItemStringHash(self->msd_a, key, hash);
	if (result == ITER_DONE)
		return DeeMap_OperatorTryGetItemStringHash(self->msd_b, key, hash);
	if unlikely(!result)
		goto err;
	exists = DeeMap_OperatorHasItemStringHash(self->msd_b, key, hash);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		Dee_Decref(result);
		return ITER_DONE;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_hasitem_string_hash(MapSymmetricDifference *__restrict self,
                        char const *key, Dee_hash_t hash) {
	int exists, result;
	result = DeeMap_OperatorHasItemStringHash(self->msd_a, key, hash);
	if (result == 0)
		return DeeMap_OperatorHasItemStringHash(self->msd_b, key, hash);
	if unlikely(result < 0)
		goto err;
	exists = DeeMap_OperatorHasItemStringHash(self->msd_b, key, hash);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		return 0;
	}
	return 1;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_bounditem_string_hash(MapSymmetricDifference *__restrict self,
                          char const *key, Dee_hash_t hash) {
	int exists, result;
	result = DeeMap_OperatorBoundItemStringHash(self->msd_a, key, hash);
	if (Dee_BOUND_ISMISSING_OR_UNBOUND(result)) {
		int result2 = DeeMap_OperatorBoundItemStringHash(self->msd_b, key, hash);
		if (result == result2) {
			result = Dee_BOUND_MISSING;
		} else if (Dee_BOUND_ISMISSING(result) || !Dee_BOUND_ISMISSING(result2)) {
			result = result2;
		}
		return result;
	}
	if unlikely(Dee_BOUND_ISERR(result))
		goto err;
	exists = DeeMap_OperatorBoundItemStringHash(self->msd_b, key, hash);
	if unlikely(!Dee_BOUND_ISMISSING_OR_UNBOUND(exists)) {
		if unlikely(Dee_BOUND_ISERR(exists))
			goto err;
		return exists;
	}
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_getitem_string_len_hash(MapSymmetricDifference *__restrict self,
                            char const *key, size_t keylen, Dee_hash_t hash) {
	int exists;
	DREF DeeObject *result;
	result = DeeMap_OperatorTryGetItemStringLenHash(self->msd_a, key, keylen, hash);
	if (result == ITER_DONE)
		return DeeMap_OperatorGetItemStringLenHash(self->msd_b, key, keylen, hash);
	if unlikely(!result)
		goto err;
	exists = DeeMap_OperatorHasItemStringLenHash(self->msd_b, key, keylen, hash);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		goto err_key_r;
	}
	return result;
err_key_r:
	Dee_Decref(result);
	err_unknown_key_str((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msd_trygetitem_string_len_hash(MapSymmetricDifference *__restrict self,
                               char const *key, size_t keylen, Dee_hash_t hash) {
	int exists;
	DREF DeeObject *result;
	result = DeeMap_OperatorTryGetItemStringLenHash(self->msd_a, key, keylen, hash);
	if (result == ITER_DONE)
		return DeeMap_OperatorTryGetItemStringLenHash(self->msd_b, key, keylen, hash);
	if unlikely(!result)
		goto err;
	exists = DeeMap_OperatorHasItemStringLenHash(self->msd_b, key, keylen, hash);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		Dee_Decref(result);
		return ITER_DONE;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_hasitem_string_len_hash(MapSymmetricDifference *__restrict self,
                            char const *key, size_t keylen, Dee_hash_t hash) {
	int exists, result;
	result = DeeMap_OperatorHasItemStringLenHash(self->msd_a, key, keylen, hash);
	if (result == 0)
		return DeeMap_OperatorHasItemStringLenHash(self->msd_b, key, keylen, hash);
	if unlikely(result < 0)
		goto err;
	exists = DeeMap_OperatorHasItemStringLenHash(self->msd_b, key, keylen, hash);
	if unlikely(exists != 0) {
		if unlikely(exists < 0)
			goto err;
		return 0;
	}
	return 1;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msd_bounditem_string_len_hash(MapSymmetricDifference *__restrict self,
                              char const *key, size_t keylen, Dee_hash_t hash) {
	int exists, result;
	result = DeeMap_OperatorBoundItemStringLenHash(self->msd_a, key, keylen, hash);
	if (Dee_BOUND_ISMISSING_OR_UNBOUND(result)) {
		int result2 = DeeMap_OperatorBoundItemStringLenHash(self->msd_b, key, keylen, hash);
		if (result == result2) {
			result = Dee_BOUND_MISSING;
		} else if (Dee_BOUND_ISMISSING(result) || !Dee_BOUND_ISMISSING(result2)) {
			result = result2;
		}
		return result;
	}
	if unlikely(Dee_BOUND_ISERR(result))
		goto err;
	exists = DeeMap_OperatorBoundItemStringLenHash(self->msd_b, key, keylen, hash);
	if unlikely(!Dee_BOUND_ISMISSING_OR_UNBOUND(exists)) {
		if unlikely(Dee_BOUND_ISERR(exists))
			goto err;
		return exists;
	}
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
msd_foreach_pair(MapSymmetricDifference *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	Dee_ssize_t result, temp;
	struct map_foreach_pair_filter_keys data;
	data.mfpfk_cb   = cb;
	data.mfpfk_arg  = arg;
	data.mfpfk_keys = self->msd_b;
	result = DeeMap_OperatorForeachPair(self->msd_a, &map_foreach_pair_without_mapkeys_cb, &data);
	if unlikely(result < 0)
		return result;
	data.mfpfk_keys = self->msd_a;
	temp = DeeMap_OperatorForeachPair(self->msd_b, &map_foreach_pair_without_mapkeys_cb, &data);
	if unlikely(temp < 0)
		return temp;
	result += temp;
	return result;
}

PRIVATE struct type_seq msd_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&msd_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&msd_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&msd_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&msd_foreach_pair,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&msd_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&msd_hasitem,
	/* .tp_size                       = */ NULL,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&msd_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&msd_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&msd_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&msd_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&msd_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&msd_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&msd_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&msd_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&msd_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&msd_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&msd_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&msd_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&msd_hasitem_string_len_hash,
};

PRIVATE struct type_member tpconst msd_members[] = {
	TYPE_MEMBER_FIELD_DOC("__map_a__", STRUCT_OBJECT, offsetof(MapSymmetricDifference, msd_a), "->?DMapping"),
	TYPE_MEMBER_FIELD_DOC("__map_b__", STRUCT_OBJECT, offsetof(MapSymmetricDifference, msd_b), "->?DMapping"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst msd_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &MapSymmetricDifferenceIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapSymmetricDifference_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapSymmetricDifference",
	/* .tp_doc      = */ DOC("()\n"
	                         "(a:?DMapping,b:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&msd_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&msd_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&msd_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&msd_init,
				TYPE_FIXED_ALLOCATOR(MapSymmetricDifference)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&msd_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&msd_bool,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&msd_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &msd_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ msd_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ msd_class_members,
};

STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_symdiff) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapSymmetricDifferenceIterator, msdi_symdiff) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(MapSymmetricDifferenceIterator, msdi_iter) == offsetof(ProxyObject2, po_obj2));
#define msditer_fini generic_proxy2__fini

#if 1
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_lock) == offsetof(MapUnionIterator, mui_lock));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_iter) == offsetof(MapUnionIterator, mui_iter));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_in2nd) == offsetof(MapUnionIterator, mui_in2nd));
#define msditer_clear     muiter_clear
#define msditer_get_iter  muiter_get_iter
#define msditer_set_iter  muiter_set_iter
#define msditer_get_in2nd muiter_get_in2nd
#define msditer_set_in2nd muiter_set_in2nd
#define msditer_getsets   muiter_getsets
#else
PRIVATE NONNULL((1)) void DCALL
msditer_clear(MapSymmetricDifferenceIterator *__restrict self) {
	DREF DeeObject *iter;
	MapSymmetricDifferenceIterator_LockWrite(self);
	iter = self->msdi_iter;
	Dee_Incref(Dee_None);
	self->msdi_iter = Dee_None;
	MapSymmetricDifferenceIterator_LockEndWrite(self);
	Dee_Decref(iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msditer_get_iter(MapSymmetricDifferenceIterator *__restrict self) {
	DREF DeeObject *result;
	MapSymmetricDifferenceIterator_LockRead(self);
	result = self->msdi_iter;
	Dee_Incref(result);
	MapSymmetricDifferenceIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
msditer_set_iter(MapSymmetricDifferenceIterator *__restrict self,
                DeeObject *__restrict iter) {
	DREF DeeObject *old_iter;
	Dee_Incref(iter);
	MapSymmetricDifferenceIterator_LockRead(self);
	old_iter       = self->msdi_iter;
	self->msdi_iter = iter;
	MapSymmetricDifferenceIterator_LockEndRead(self);
	Dee_Decref(old_iter);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msditer_get_in2nd(MapSymmetricDifferenceIterator *__restrict self) {
	bool result;
	COMPILER_BARRIER();
	result = self->msdi_in2nd;
	COMPILER_BARRIER();
	return_bool(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
msditer_set_in2nd(MapSymmetricDifferenceIterator *__restrict self,
                 DeeObject *__restrict value) {
	int newval = DeeObject_Bool(value);
	if unlikely(newval < 0)
		return newval;
	MapSymmetricDifferenceIterator_LockWrite(self);
	self->msdi_in2nd = !!newval;
	MapSymmetricDifferenceIterator_LockEndWrite(self);
	return 0;
}

PRIVATE struct type_getset tpconst msditer_getsets[] = {
	TYPE_GETSET_F("__iter__", &msditer_get_iter, NULL, &msditer_set_iter, METHOD_FNOREFESCAPE, "->?DIterator"),
	TYPE_GETSET_F("__in2nd__", &msditer_get_in2nd, NULL, &msditer_set_in2nd, METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETSET_END
};
#endif

#if 1
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_lock) == offsetof(MapUnionIterator, mui_lock));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_iter) == offsetof(MapUnionIterator, mui_iter));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_symdiff) == offsetof(MapUnionIterator, mui_union));
#define msditer_visit muiter_visit
#else
PRIVATE NONNULL((1, 2)) void DCALL
msditer_visit(MapSymmetricDifferenceIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->msdi_symdiff);
	MapSymmetricDifferenceIterator_LockRead(self);
	Dee_Visit(self->msdi_iter);
	MapSymmetricDifferenceIterator_LockEndRead(self);
}
#endif

#if 1
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_lock) == offsetof(MapUnionIterator, mui_lock));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_iter) == offsetof(MapUnionIterator, mui_iter));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_symdiff) == offsetof(MapUnionIterator, mui_union));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_in2nd) == offsetof(MapUnionIterator, mui_in2nd));
#define msditer_copy muiter_copy
#define msditer_deep muiter_deep
#define msditer_ctor muiter_ctor
#define msditer_init muiter_init
#else
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
msditer_copy(MapSymmetricDifferenceIterator *__restrict self,
             MapSymmetricDifferenceIterator *__restrict other) {
	DREF DeeObject *iter;
	MapSymmetricDifferenceIterator_LockRead(other);
	iter             = other->msdi_iter;
	self->msdi_in2nd = other->msdi_in2nd;
	Dee_Incref(iter);
	MapSymmetricDifferenceIterator_LockEndRead(other);
	self->msdi_iter = DeeObject_Copy(iter);
	Dee_Decref(iter);
	if unlikely(!self->msdi_iter)
		goto err;
	Dee_atomic_rwlock_init(&self->msdi_lock);
	self->msdi_symdiff = other->msdi_symdiff;
	Dee_Incref(self->msdi_symdiff);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
msditer_deep(MapSymmetricDifferenceIterator *__restrict self,
             MapSymmetricDifferenceIterator *__restrict other) {
	DREF DeeObject *iter;
	MapSymmetricDifferenceIterator_LockRead(other);
	iter            = other->msdi_iter;
	self->msdi_in2nd = other->msdi_in2nd;
	Dee_Incref(iter);
	MapSymmetricDifferenceIterator_LockEndRead(other);
	self->msdi_iter = DeeObject_DeepCopy(iter);
	Dee_Decref(iter);
	if unlikely(!self->msdi_iter)
		goto err;
	Dee_atomic_rwlock_init(&self->msdi_lock);
	self->msdi_symdiff = (DREF MapSymmetricDifference *)DeeObject_DeepCopy((DeeObject *)other->msdi_symdiff);
	if unlikely(!self->msdi_symdiff)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->msdi_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msditer_ctor(MapSymmetricDifferenceIterator *__restrict self) {
	self->msdi_symdiff = (DREF MapSymmetricDifference *)DeeObject_NewDefault(&MapSymmetricDifference_Type);
	if unlikely(!self->msdi_symdiff)
		goto err;
	self->msdi_iter = DeeMap_OperatorIter(self->msdi_symdiff->msd_a);
	if unlikely(!self->msdi_iter)
		goto err_union;
	Dee_atomic_rwlock_init(&self->msdi_lock);
	self->msdi_in2nd = false;
	return 0;
err_union:
	Dee_Decref(self->msdi_symdiff);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
msditer_init(MapSymmetricDifferenceIterator *__restrict self,
             size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_MapSymmetricDifferenceIterator", &self->msdi_symdiff))
		goto err;
	if (DeeObject_AssertTypeExact(self->msdi_symdiff, &MapSymmetricDifference_Type))
		goto err;
	if ((self->msdi_iter = DeeMap_OperatorIter(self->msdi_symdiff->msd_a)) == NULL)
		goto err;
	Dee_Incref(self->msdi_symdiff);
	Dee_atomic_rwlock_init(&self->msdi_lock);
	self->msdi_in2nd = false;
	return 0;
err:
	return -1;
}
#endif

#if 1
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_lock) == offsetof(MapUnionIterator, mui_lock));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_iter) == offsetof(MapUnionIterator, mui_iter));
STATIC_ASSERT(offsetof(MapSymmetricDifferenceIterator, msdi_in2nd) == offsetof(MapUnionIterator, mui_in2nd));
#define msditer_hash       muiter_hash
#define msditer_compare    muiter_compare
#define msditer_compare_eq muiter_compare_eq
#define msditer_cmp        muiter_cmp
#else
PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
msditer_hash(MapSymmetricDifferenceIterator *self) {
	Dee_hash_t result;
	bool my_2nd;
	DREF DeeObject *my_iter;
	MapSymmetricDifferenceIterator_LockRead(self);
	my_iter = self->msdi_iter;
	my_2nd  = self->msdi_in2nd;
	MapSymmetricDifferenceIterator_LockEndRead(self);
	result = Dee_HashCombine(my_2nd ? 1 : 0, DeeObject_Hash(my_iter));
	Dee_Decref(my_iter);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
msditer_compare(MapSymmetricDifferenceIterator *self,
               MapSymmetricDifferenceIterator *other) {
	int result;
	bool my_2nd, ot_2nd;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	MapSymmetricDifferenceIterator_LockRead(self);
	my_iter = self->msdi_iter;
	my_2nd  = self->msdi_in2nd;
	MapSymmetricDifferenceIterator_LockEndRead(self);
	MapSymmetricDifferenceIterator_LockRead(other);
	ot_iter = other->msdi_iter;
	ot_2nd  = other->msdi_in2nd;
	MapSymmetricDifferenceIterator_LockEndRead(other);
	if (my_2nd != ot_2nd) {
		result = Dee_CompareNe(my_2nd, ot_2nd);
	} else {
		result = DeeObject_Compare(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
msditer_compare_eq(MapSymmetricDifferenceIterator *self,
                  MapSymmetricDifferenceIterator *other) {
	int result;
	bool my_2nd, ot_2nd;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	MapSymmetricDifferenceIterator_LockRead(self);
	my_iter = self->msdi_iter;
	my_2nd  = self->msdi_in2nd;
	MapSymmetricDifferenceIterator_LockEndRead(self);
	MapSymmetricDifferenceIterator_LockRead(other);
	ot_iter = other->msdi_iter;
	ot_2nd  = other->msdi_in2nd;
	MapSymmetricDifferenceIterator_LockEndRead(other);
	if (my_2nd != ot_2nd) {
		result = 1;
	} else {
		result = DeeObject_CompareEq(my_iter, ot_iter);
	}
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp msditer_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&msditer_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&msditer_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&msditer_compare,
};
#endif

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
msditer_nextpair(MapSymmetricDifferenceIterator *__restrict self,
                /*out*/ DREF DeeObject *key_and_value[2]) {
	bool is_second;
	int result;
	DREF DeeObject *iter;
again:
	MapSymmetricDifferenceIterator_LockRead(self);
	iter      = self->msdi_iter;
	is_second = self->msdi_in2nd;
	Dee_Incref(iter);
	MapSymmetricDifferenceIterator_LockEndRead(self);
read_from_iter:
	result = DeeObject_IterNextPair(iter, key_and_value);
	Dee_Decref(iter);
	if (result == 0) {
		int key_exists;
		DeeObject *inactive_map;
		inactive_map = is_second ? self->msdi_symdiff->msd_a
		                         : self->msdi_symdiff->msd_b;
		key_exists = DeeMap_OperatorContainsAsBool(inactive_map, key_and_value[0]);
		if likely(key_exists == 0)
			goto done; /* Key hadn't been enumerated, yet */
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		if unlikely(key_exists < 0)
			goto err;
		if unlikely(DeeThread_CheckInterrupt())
			goto err;
		goto again;
	} else if (!is_second) {
		DREF DeeObject *iter2;
		if unlikely(result < 0)
			goto done;

		/* End of first iterator -> try to switch to the second iterator. */
		iter2 = DeeMap_OperatorIter(self->msdi_symdiff->msd_b);
		if unlikely(!iter2)
			goto err;
		MapSymmetricDifferenceIterator_LockWrite(self);
		if unlikely(self->msdi_in2nd) {
			MapSymmetricDifferenceIterator_LockEndWrite(self);
			Dee_Decref(iter2);
			goto again;
		}
		iter = self->msdi_iter;
		self->msdi_iter  = iter2;
		self->msdi_in2nd = true;
		Dee_Incref(iter2);
		MapSymmetricDifferenceIterator_LockEndWrite(self);
		Dee_Decref(iter);
		iter = iter2;
		goto read_from_iter;
	}
done:
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
msditer_nextkey(MapSymmetricDifferenceIterator *__restrict self) {
	bool is_second;
	DREF DeeObject *result;
	DREF DeeObject *iter;
again:
	MapSymmetricDifferenceIterator_LockRead(self);
	iter      = self->msdi_iter;
	is_second = self->msdi_in2nd;
	Dee_Incref(iter);
	MapSymmetricDifferenceIterator_LockEndRead(self);
read_from_iter:
	result = DeeObject_IterNextKey(iter);
	Dee_Decref(iter);
	if (ITER_ISOK(result)) {
		int key_exists;
		DeeObject *inactive_map;
		inactive_map = is_second ? self->msdi_symdiff->msd_a
		                         : self->msdi_symdiff->msd_b;
		key_exists = DeeMap_OperatorContainsAsBool(inactive_map, result);
		if likely(key_exists == 0)
			goto done; /* Key hadn't been enumerated, yet */
		Dee_Decref(result);
		if unlikely(key_exists < 0)
			goto err;
		if unlikely(DeeThread_CheckInterrupt())
			goto err;
		goto again;
	} else if (!is_second) {
		DREF DeeObject *iter2;
		if unlikely(!result)
			goto done;

		/* End of first iterator -> try to switch to the second iterator. */
		iter2 = DeeMap_OperatorIter(self->msdi_symdiff->msd_b);
		if unlikely(!iter2)
			goto err;
		MapSymmetricDifferenceIterator_LockWrite(self);
		if unlikely(self->msdi_in2nd) {
			MapSymmetricDifferenceIterator_LockEndWrite(self);
			Dee_Decref(iter2);
			goto again;
		}
		iter = self->msdi_iter;
		self->msdi_iter  = iter2;
		self->msdi_in2nd = true;
		Dee_Incref(iter2);
		MapSymmetricDifferenceIterator_LockEndWrite(self);
		Dee_Decref(iter);
		iter = iter2;
		goto read_from_iter;
	}
done:
	return result;
err:
	return NULL;
}


PRIVATE struct type_member tpconst msditer_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(MapSymmetricDifferenceIterator, msdi_symdiff), "->?Ert:MapSymmetricDifference"),
	TYPE_MEMBER_END
};

PRIVATE struct type_iterator msditer_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&msditer_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&msditer_nextkey,
};

PRIVATE struct type_gc msditer_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *))&msditer_clear
};

INTERN DeeTypeObject MapSymmetricDifferenceIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapSymmetricDifferenceIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(msd:?Ert:MapSymmetricDifference)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&msditer_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&msditer_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&msditer_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&msditer_init,
				TYPE_FIXED_ALLOCATOR_GC(MapSymmetricDifferenceIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&msditer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&msditer_visit,
	/* .tp_gc            = */ &msditer_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &msditer_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &msditer_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ msditer_getsets,
	/* .tp_members       = */ msditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};


PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
set_containsany_foreach_pair_lhsmap_cb(void *arg, DeeObject *key, DeeObject *value) {
	int is_contained;
	(void)value;
	is_contained = DeeSet_OperatorContainsAsBool((DeeObject *)arg, key);
	if (is_contained != 0) {
		if unlikely(is_contained < 0)
			goto err;
		return MAP_CONTAINSANY_FOREACH__FOUND;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
MapIntersection_NonEmpty(DeeObject *map, DeeObject *keys) {
	if (SetInversion_CheckExact(keys)) {
		SetInversion *xb = (SetInversion *)keys;
		/* `(map & ~keys) != {}'   <=>   `(map - keys) != {}' */
		return MapDifference_NonEmpty(map, xb->si_set);
	} else {
		Dee_ssize_t status;
		size_t size_map, size_keys;
		size_map = DeeMap_OperatorSize(map);
		if unlikely(size_map == (size_t)-1)
			goto err;
		size_keys = DeeSet_OperatorSize(keys);
		if unlikely(size_keys == (size_t)-1)
			goto err;
		if (size_map < size_keys) {
			/* >> for (local key, none: map) if (key in keys) return true;
			 * >> return false; */
			status = DeeMap_OperatorForeachPair(map, &set_containsany_foreach_pair_lhsmap_cb, keys);
		} else {
			/* >> for (local key: keys) if (key in map) return true; */
			/* >> return false; */
			status = DeeSet_OperatorForeach(keys, &map_containsany_foreach_cb, map);
		}
		if (status == MAP_CONTAINSANY_FOREACH__FOUND)
			return 1;
		if (status == 0)
			return 0;
		return -1;
	}
	__builtin_unreachable();
err:
	return -1;
}


#define SET_CONTAINSALL_FOREACH_PAIR_LHSMAP__MISSING SSIZE_MIN
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
set_containsall_foreach_pair_lhsmap_cb(void *arg, DeeObject *key, DeeObject *value) {
	int is_contained;
	(void)value;
	is_contained = DeeSet_OperatorContainsAsBool((DeeObject *)arg, key);
	if (is_contained <= 0) {
		if unlikely(is_contained < 0)
			goto err;
		return SET_CONTAINSALL_FOREACH_PAIR_LHSMAP__MISSING;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
mapkeys_containsall_foreach_pair_lhsmap_cb(void *arg, DeeObject *key, DeeObject *value) {
	int is_contained;
	(void)value;
	is_contained = DeeMap_OperatorContainsAsBool((DeeObject *)arg, key);
	if (is_contained <= 0) {
		if unlikely(is_contained < 0)
			goto err;
		return SET_CONTAINSALL_FOREACH_PAIR_LHSMAP__MISSING;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
MapDifference_NonEmpty(DeeObject *map, DeeObject *keys) {
	if (SetInversion_CheckExact(keys)) {
		/* `(map - ~keys) != {}'   <=>   `(map & keys) != {}' */
		SetInversion *xb = (SetInversion *)keys;
		return MapIntersection_NonEmpty(map, xb->si_set);
	} else {
		/* >> for (local key, none: map) if (key !in keys) return true;
		 * >> return false; */
		Dee_ssize_t status;
		status = DeeMap_OperatorForeachPair(map, &set_containsall_foreach_pair_lhsmap_cb, keys);
		if (status == SET_CONTAINSALL_FOREACH_PAIR_LHSMAP__MISSING)
			return 1;
		if (status == 0)
			return 0;
		return -1;
	}
	__builtin_unreachable();
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
MapDifferenceMapKeys_NonEmpty(DeeObject *map, DeeObject *map2) {
	Dee_ssize_t status;
	status = DeeMap_OperatorForeachPair(map, &mapkeys_containsall_foreach_pair_lhsmap_cb, map2);
	if (status == SET_CONTAINSALL_FOREACH_PAIR_LHSMAP__MISSING)
		return 1;
	if (status == 0)
		return 0;
	return -1;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAPS_C */
