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
#ifndef GUARD_DEX_COLLECTIONS_RANGEMAP_C
#define GUARD_DEX_COLLECTIONS_RANGEMAP_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/lock.h>

#include <hybrid/overflow.h>

DECL_BEGIN

#define Dee_EmptyRangeMap (&_Dee_EmptyRangeMap)
PRIVATE DeeObject _Dee_EmptyRangeMap = {
	Dee_OBJECT_HEAD_INIT(&RangeMap_Type)
};

/*[[[deemon
import define_Dee_HashStr from rt.gen.hash;
for (local s: { "first", "last", "get", "pop", "clear", "setdefault",
                "setold", "setnew", "setold_ex", "setnew_ex", }) {
	print define_Dee_HashStr(s);
}
]]]*/
#define Dee_HashStr__first _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)
#define Dee_HashStr__last _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)
#define Dee_HashStr__get _Dee_HashSelectC(0x3b6d35a2, 0x7c8e1568eac4979f)
#define Dee_HashStr__pop _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb)
#define Dee_HashStr__clear _Dee_HashSelectC(0x7857faae, 0x22a34b6f82b3b83c)
#define Dee_HashStr__setdefault _Dee_HashSelectC(0x947d5cce, 0x7cbcb4f64ace9cbc)
#define Dee_HashStr__setold _Dee_HashSelectC(0xb02a28d9, 0xe69353d27a45da0c)
#define Dee_HashStr__setnew _Dee_HashSelectC(0xb6040b2, 0xde8a8697e7aca93d)
#define Dee_HashStr__setold_ex _Dee_HashSelectC(0xf8b4d68b, 0x73d8fdc770be1ae)
#define Dee_HashStr__setnew_ex _Dee_HashSelectC(0x3f694391, 0x104d84a2d9986bc5)
/*[[[end]]]*/


/************************************************************************/
/* PROXY TYPES                                                          */
/************************************************************************/

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *rmp_rmap; /* [1..1][const] Underlying RangeMap. */
} RangeMapProxy;

PRIVATE WUNUSED NONNULL((1, 2)) DREF RangeMapProxy *DCALL
RangeMapProxy_New(DeeObject *rmap, DeeTypeObject *type) {
	DREF RangeMapProxy *result;
	result = DeeObject_MALLOC(RangeMapProxy);
	if likely(result) {
		result->rmp_rmap = rmap;
		Dee_Incref(rmap);
		DeeObject_Init(result, type);
	}
	return result;
}

typedef struct {
	/* For: RangeMapProxyIterator_Type,
	 *      RangeMapNodesIterator_Type */
	OBJECT_HEAD
	DREF DeeObject *rmpi_iter; /* [1..1][const] The iterator for enumerating `rmpi_rmap'. */
	DREF DeeObject *rmpi_rmap; /* [1..1][const] The RangeMap object being iterated. */
} RangeMapProxyIterator;

typedef struct {
	/* For: RangeMapValuesIterator_Type,
	 *      RangeMapItemsIterator_Type,
	 *      RangeMapRangesIterator_Type */
	RangeMapProxyIterator  rmpii_base; /* Underlying iterator */
	union {
		/* For: RangeMapMapItemsIterator_Type,
		 *      RangeMapAsMapIterator_Type */
		DREF DeeObject        *rmpii_value; /* [1..1][lock(:rmpki_lock)] Value linked to `rmpki_prvkey' */
		/* For: ... */
		void *rmpii_UNUSED; /* Unused... (used to be for NSI, but not fully removed because this entire file needs a re-write) */
	};
} RangeMapProxyItemsIterator;

typedef struct {
	/* For: RangeMapKeysIterator_Type,
	 *      RangeMapMapItemsIterator_Type,
	 *      RangeMapAsMapIterator_Type */
	RangeMapProxyItemsIterator rmpki_base;   /* Underlying iterator */
	DREF DeeObject            *rmpki_prvkey; /* [1..1][lock(rmpki_lock)] Previously enumerated key */
	DREF DeeObject            *rmpki_maxkey; /* [1..1][lock(rmpki_lock)] Upper bound for current key range (once `>= rmpki_prvkey', use `rmpki_base.rmpii_base.rmpi_iter' to get the next range) */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t          rmpki_lock;   /* Lock for this iterator */
#endif /* !CONFIG_NO_THREADS */
	bool                       rmpki_first;  /* [lock(rmpki_lock)] Set to true if `rmpki_prvkey' must be copied. */
} RangeMapProxyKeysIterator;

#define RangeMapProxyKeysIterator_LockAvailable(self)  Dee_atomic_lock_available(&(self)->rmpki_lock)
#define RangeMapProxyKeysIterator_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->rmpki_lock)
#define RangeMapProxyKeysIterator_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->rmpki_lock)
#define RangeMapProxyKeysIterator_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->rmpki_lock)
#define RangeMapProxyKeysIterator_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->rmpki_lock)
#define RangeMapProxyKeysIterator_LockRelease(self)    Dee_atomic_lock_release(&(self)->rmpki_lock)




/************************************************************************/
/* CORE ABSTRACT TYPE                                                   */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
rangemap_printrepr(DeeObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DREF DeeObject *iterator;
	DREF DeeObject *elem;
	dssize_t temp, result;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, "{ ");
	if unlikely(result < 0)
		goto done;
	iterator = DeeObject_Iter(self);
	if unlikely(!iterator)
		goto err_m1;
	is_first = true;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		DREF DeeObject *item[3];
		if (DeeSeq_Unpack(elem, 3, item))
			goto err_m1_iterator_elem;
		Dee_Decref(elem);
		temp = DeeFormat_Printf(printer, arg, "%s[%r:%r]: %r",
		                        is_first ? "" : ", ",
		                        item[0], item[1], item[2]);
		Dee_Decrefv(item, 3);
		if unlikely(temp < 0)
			goto err_iterator;
		result += temp;
		is_first = false;
		if (DeeThread_CheckInterrupt())
			goto err_m1_iterator;
	}
	Dee_Decref(iterator);
	if unlikely(!elem)
		goto err_m1;
	if (is_first) {
		temp = DeeFormat_PRINT(printer, arg, "}");
	} else {
		temp = DeeFormat_PRINT(printer, arg, " }");
	}
	if unlikely(temp < 0)
		goto err;
	result += temp;
done:
	return result;
err_m1_iterator_elem:
	temp = -1;
/*err_iterator_elem:*/
	Dee_Decref(elem);
err_iterator:
	Dee_Decref(iterator);
err:
	return temp;
err_m1_iterator:
	temp = -1;
	goto err_iterator;
err_m1:
	temp = -1;
	goto err;
}


	/* RBTree-specific methods */
//	TYPE_METHOD("locate", &rangemap_locate,
//	            "(key)->?X2?T3?O?O?O?N\n"
//	            "#r{The ${minkey,maxkey,value} triple where @key is located within $minkey and $maxkey, "
//	            /**/ "or ?N when no node contains @key}"),
//	TYPE_METHOD("rlocate", &rangemap_rlocate,
//	            "(minkey,maxkey)->?S?T3?O?O?O\n"
//	            "#r{A list of ${minkey,maxkey,value} triples describing nodes which overlap with the given key-range}"),
//	TYPE_METHOD("itlocate", &rangemap_itlocate,
//	            "(key)->?X2?#Iterator?N\n"
//	            "#r{An ?#Iterator bound to the node containing @key, or ?N when no node contains @key}"),
//	TYPE_METHOD("remove", &rangemap_remove,
//	            "(key)->?X2?T3?O?O?O?N\n"
//	            "#r{The ${minkey,maxkey,value} triple where @key was located within $minkey and $maxkey, "
//	            /**/ "or ?N when no node contained @key}"
//	            "Like ?#locate, but also remove the node"),
//	TYPE_METHOD("rremove", &rangemap_rremove,
//	            "(minkey,maxkey)->?S?T3?O?O?O\n"
//	            "#r{A list of ${minkey,maxkey,value} triples describing nodes which overlapped with the given key-range}"
//	            "Like ?#rlocate, but also remove the nodes"),
//	TYPE_METHOD("prevnode", &rangemap_prevnode,
//	            "(key)->?X2?T3?O?O?O?N\n"
//	            "#r{The ${minkey,maxkey,value} triple of the node that comes before "
//	            /**/ "the one containing @key, or ?N when no such node exists}"),
//	TYPE_METHOD("nextnode", &rangemap_nextnode,
//	            "(key)->?X2?T3?O?O?O?N\n"
//	            "#r{The ${minkey,maxkey,value} triple of the node that comes after "
//	            /**/ "the one containing @key, or ?N when no such node exists}"),
//	TYPE_METHOD("itprevnode", &rangemap_itprevnode,
//	            "(key)->?X2?#Iterator?N\n"
//	            "#r{An ?#Iterator bound to the node that comes before "
//	            /**/ "the one containing @key, or ?N when no such node exists}"),
//	TYPE_METHOD("itnextnode", &rangemap_itnextnode,
//	            "(key)->?X2?#Iterator?N\n"
//	            "#r{An ?#Iterator bound to the node that comes after "
//	            /**/ "the one containing @key, or ?N when no such node exists}"),
//
//	TYPE_METHOD("optimize", &rangemap_optimize,
//	            "()\n"
//	            "Search for adjacent nodes that can be merged due to having identical (as per ${===}) "
//	            /**/ "values, as well as consecutive key-ranges. By default, this sort of optimization "
//	            /**/ "is only done when using ?Dint as key type, though if custom key types are used, "
//	            /**/ "then this function may be used to perform that same optimization."),
//
//	/* Generic methods */
//	TYPE_METHOD("clear", &rangemap_doclear,
//	            "()\n"
//	            "Clear all values from @this ?."),
//	TYPE_METHOD("pop", &rangemap_pop,
//	            "(key)->\n"
//	            "(key,def)->\n"
//	            DOC_ERROR_NotImplemented_CANNOT_SPLIT
//	            "#tKeyError{No @def was given and @key was not found}"
//	            "Delete @key from @this and return its previously assigned "
//	            /**/ "value or @def when @key had no item associated"),

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rangemap_popfront(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *front_key;
	_DeeArg_Unpack0(err, argc, argv, "popfront");
	result = DeeObject_GetAttrStringHash(self, "first", Dee_HashStr__first);
	if unlikely(!result)
		goto err;
	front_key = DeeObject_GetItemIndex(result, 0);
	if unlikely(!front_key)
		goto err_result;
	if unlikely(DeeObject_DelItem(self, front_key))
		goto err_result_front_key;
	Dee_Decref(front_key);
	return result;
err_result_front_key:
	Dee_Decref(front_key);
err_result:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rangemap_popback(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *front_key;
	_DeeArg_Unpack0(err, argc, argv, "popback");
	result = DeeObject_GetAttrStringHash(self, "last", Dee_HashStr__last);
	if unlikely(!result)
		goto err;
	front_key = DeeObject_GetItemIndex(result, 0);
	if unlikely(!front_key)
		goto err_result;
	if unlikely(DeeObject_DelItem(self, front_key))
		goto err_result_front_key;
	Dee_Decref(front_key);
	return result;
err_result_front_key:
	Dee_Decref(front_key);
err_result:
	Dee_Decref(result);
err:
	return NULL;
}

#define rangemap_popitem rangemap_popfront

PRIVATE WUNUSED NONNULL((2)) dssize_t DCALL
rangemap_insert_range(void *arg, DeeObject *elem) {
	int result;
	DREF DeeObject *item[3];
	if (DeeSeq_Unpack(elem, 3, item))
		goto err;
	result = DeeObject_SetRange((DeeObject *)arg, item[0], item[1], item[2]);
	Dee_Decrefv(item, 3);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rangemap_clear(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack0(err, argc, argv, "clear");
	for (;;) {
		int temp;
		DREF DeeObject *elem, *iter;
		DREF DeeObject *item[3];
		iter = DeeObject_Iter(self);
		if unlikely(!iter)
			goto err;
		elem = DeeObject_IterNext(iter);
		Dee_Decref_likely(iter);
		if unlikely(!elem)
			goto err;
		if (elem == ITER_DONE)
			break;
		temp = DeeSeq_Unpack(elem, 3, item);
		Dee_Decref(elem);
		if unlikely(temp)
			goto err;
		Dee_Decref(item[2]);
		temp = DeeObject_DelRange(self, item[0], item[1]);
		Dee_Decref(item[1]);
		Dee_Decref(item[0]);
		if unlikely(temp)
			goto err;
	}
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rangemap_update(DeeObject *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	_DeeArg_Unpack1(err, argc, argv, "update", &items);
	if unlikely(DeeObject_Foreach(items, &rangemap_insert_range, self) < 0)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rangemap_get_first(DeeObject *__restrict self) {
	return DeeObject_CallAttrStringHash((DeeObject *)&DeeSeq_Type, "first",
	                                    Dee_HashStr__first, 1, (DeeObject *const *)&self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rangemap_get_last(DeeObject *__restrict self) {
	return DeeObject_CallAttrStringHash((DeeObject *)&DeeSeq_Type, "last",
	                                    Dee_HashStr__last, 1, (DeeObject *const *)&self);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
rangemap_keys(DeeObject *__restrict self) {
	return RangeMapProxy_New(self, &RangeMapKeys_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
rangemap_values(DeeObject *__restrict self) {
	return RangeMapProxy_New(self, &RangeMapValues_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
rangemap_items(DeeObject *__restrict self) {
	return RangeMapProxy_New(self, &RangeMapItems_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
rangemap_nodes(DeeObject *__restrict self) {
	return RangeMapProxy_New(self, &RangeMapNodes_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
rangemap_ranges(DeeObject *__restrict self) {
	return RangeMapProxy_New(self, &RangeMapRanges_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
rangemap_mapitems(DeeObject *__restrict self) {
	return RangeMapProxy_New(self, &RangeMapMapItems_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
rangemap_asmap(DeeObject *__restrict self) {
	return RangeMapProxy_New(self, &RangeMapAsMap_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
rangemap_iterator_get(DeeTypeObject *__restrict self) {
	if (self == &RangeMap_Type)
		return_reference_(&DeeIterator_Type);
	DeeRT_ErrUnknownAttrStr(self, "Iterator", DeeRT_ATTRIBUTE_ACCESS_GET);
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rangemap_iterself(DeeObject *__restrict self) {
	if unlikely(Dee_TYPE(self) == &DeeMapping_Type) {
		/* Special case: Create an empty iterator.
		 * >> This can happen when someone tries to iterate a symbolic empty-mapping object. */
		return DeeIterator_NewEmpty();
	}
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_ITER);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rangemap_size(DeeObject *__restrict self) {
	size_t result = 0;
	DREF DeeObject *iter, *item;
	DeeTypeObject *tp_self = Dee_TYPE(self);

	/* Check if a sub-class is overriding `operator iter'. If
	 * not, then the mapping is empty for all we're concerned */
	if (tp_self->tp_seq->tp_iter == &rangemap_iterself)
		goto done;

	/* Very inefficient: iterate the mapping and count items. */
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		Dee_Decref(item);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
		++result;
	}
	if unlikely(!item)
		goto err_iter;
	Dee_Decref(iter);
done:
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rangemap_getitem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *iter, *item;
	DREF DeeObject *item_data[3];
	DeeTypeObject *tp_self = Dee_TYPE(self);

	/* Check if a sub-class is overriding `operator iter'. If
	 * not, then the mapping is empty for all we're concerned */
	if (tp_self->tp_seq->tp_iter == &rangemap_iterself) {
		DeeRT_ErrUnknownKey(self, key);
		goto err;
	}

	/* Very inefficient: iterate the mapping to search for a matching key-item pair. */
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		int unpack_error, temp;
		unpack_error = DeeSeq_Unpack(item, 3, item_data);
		Dee_Decref(item);
		if unlikely(unpack_error)
			goto err_iter;

		/* Check if this is the key we're looking for. */
		temp = DeeObject_CmpLoAsBool(key, item_data[0]);
		if (temp == 0)
			temp = DeeObject_CmpGrAsBool(key, item_data[1]);
		Dee_Decref(item_data[0]);
		Dee_Decref(item_data[1]);
		if (temp <= 0) {
			if unlikely(temp < 0)
				Dee_Clear(item_data[2]);

			/* Found it! */
			Dee_Decref(iter);
			return item_data[2];
		}
		Dee_Decref(item_data[2]);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	if (item == ITER_DONE)
		DeeRT_ErrUnknownKey(self, key);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rangemap_trygetitem(DeeObject *self, DeeObject *key) {
	DREF DeeObject *iter, *item;
	DREF DeeObject *item_data[3];
	DeeTypeObject *tp_self = Dee_TYPE(self);

	/* Check if a sub-class is overriding `operator iter'. If
	 * not, then the mapping is empty for all we're concerned */
	if (tp_self->tp_seq->tp_iter == &rangemap_iterself)
		goto return_defl;

	/* Very inefficient: iterate the mapping to search for a matching key-item pair. */
	iter = DeeObject_Iter(self);
	if unlikely(!iter)
		goto err;
	while (ITER_ISOK(item = DeeObject_IterNext(iter))) {
		int unpack_error, temp;
		unpack_error = DeeSeq_Unpack(item, 3, item_data);
		Dee_Decref(item);
		if unlikely(unpack_error)
			goto err_iter;

		/* Check if this is the key we're looking for. */
		temp = DeeObject_CmpLoAsBool(key, item_data[0]);
		if (temp == 0)
			temp = DeeObject_CmpGrAsBool(key, item_data[1]);
		Dee_Decref(item_data[0]);
		Dee_Decref(item_data[1]);
		if (temp <= 0) {
			if unlikely(temp < 0)
				Dee_Clear(item_data[2]);

			/* Found it! */
			Dee_Decref(iter);
			return item_data[2];
		}
		Dee_Decref(item_data[2]);
		if (DeeThread_CheckInterrupt())
			goto err_iter;
	}
	if (!item)
		goto err_iter;
	Dee_Decref(iter);
return_defl:
	return ITER_DONE;
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rangemap_contains(DeeObject *self, DeeObject *key) {
	DREF DeeObject *value;
	value = DeeObject_TryGetItem(self, key);
	if (!ITER_ISOK(value)) {
		if (value == ITER_DONE)
			return_false;
		return NULL;
	}
	Dee_Decref(value);
	return_true;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rangemap_delitem(DeeObject *self, DeeObject *key) {
	return DeeObject_DelRange(self, key, key);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
rangemap_setitem(DeeObject *self, DeeObject *key, DeeObject *value) {
	return DeeObject_SetRange(self, key, key, value);
}


PRIVATE struct type_cmp rangemap_cmp = {
	/* .tp_hash          = */ NULL, /* TODO: &rangemap_hash, */
	/* .tp_compare_eq    = */ NULL, /* TODO: &rangemap_compare_eq, */
};

PRIVATE struct type_seq rangemap_seq = {
	/* .tp_iter                       = */ &rangemap_iterself,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ &rangemap_contains,
	/* .tp_getitem                    = */ &rangemap_getitem,
	/* .tp_delitem                    = */ &rangemap_delitem,
	/* .tp_setitem                    = */ &rangemap_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL, // TODO: &rangemap_foreach,
	/* .tp_foreach_pair               = */ NULL, // TODO: &rangemap_foreach_pair,
	/* .tp_bounditem                  = */ NULL, // TODO: &rangemap_bounditem,
	/* .tp_hasitem                    = */ NULL, // TODO: &rangemap_hasitem,
	/* .tp_size                       = */ &rangemap_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL, // TODO: &rangemap_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL, // TODO: &rangemap_getitem_index_fast,
	/* .tp_delitem_index              = */ NULL, // TODO: &rangemap_delitem_index,
	/* .tp_setitem_index              = */ NULL, // TODO: &rangemap_setitem_index,
	/* .tp_bounditem_index            = */ NULL, // TODO: &rangemap_bounditem_index,
	/* .tp_hasitem_index              = */ NULL, // TODO: &rangemap_hasitem_index,
	/* .tp_getrange_index             = */ NULL, // TODO: &rangemap_getrange_index,
	/* .tp_delrange_index             = */ NULL, // TODO: &rangemap_delrange_index,
	/* .tp_setrange_index             = */ NULL, // TODO: &rangemap_setrange_index,
	/* .tp_getrange_index_n           = */ NULL, // TODO: &rangemap_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL, // TODO: &rangemap_delrange_index_n,
	/* .tp_setrange_index_n           = */ NULL, // TODO: &rangemap_setrange_index_n,
	/* .tp_trygetitem                 = */ &rangemap_trygetitem,
	/* .tp_trygetitem_index           = */ NULL, // TODO: &rangemap_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ NULL, // TODO: &rangemap_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ NULL, // TODO: &rangemap_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL, // TODO: &rangemap_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ NULL, // TODO: &rangemap_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ NULL, // TODO: &rangemap_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ NULL, // TODO: &rangemap_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ NULL, // TODO: &rangemap_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ NULL, // TODO: &rangemap_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL, // TODO: &rangemap_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ NULL, // TODO: &rangemap_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ NULL, // TODO: &rangemap_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ NULL, // TODO: &rangemap_hasitem_string_len_hash,
};

PRIVATE struct type_method tpconst rangemap_methods[] = {
	/* RBTree-specific methods */
	//TODO: TYPE_METHOD("locate", &rangemap_locate,
	//TODO:             "(key)->?X2?T3?O?O?O?N\n"
	//TODO:             "#r{The ${minkey,maxkey,value} triple where @key is located within $minkey and $maxkey, "
	//TODO:             /**/ "or ?N when no node contains @key}"),
	//TODO: TYPE_METHOD("rlocate", &rangemap_rlocate,
	//TODO:             "(minkey,maxkey)->?S?T3?O?O?O\n"
	//TODO:             "#r{A list of ${minkey,maxkey,value} triples describing nodes which overlap with the given key-range}"),
	//TODO: TYPE_METHOD("itlocate", &rangemap_itlocate,
	//TODO:             "(key)->?X2?#Iterator?N\n"
	//TODO:             "#r{An ?#Iterator bound to the node containing @key, or ?N when no node contains @key}"),
	//TODO: TYPE_METHOD("remove", &rangemap_remove,
	//TODO:             "(key)->?X2?T3?O?O?O?N\n"
	//TODO:             "#r{The ${minkey,maxkey,value} triple where @key was located within $minkey and $maxkey, "
	//TODO:             /**/ "or ?N when no node contained @key}"
	//TODO:             "Like ?#locate, but also remove the node"),
	//TODO: TYPE_METHOD("rremove", &rangemap_rremove,
	//TODO:             "(minkey,maxkey)->?S?T3?O?O?O\n"
	//TODO:             "#r{A list of ${minkey,maxkey,value} triples describing nodes which overlapped with the given key-range}"
	//TODO:             "Like ?#rlocate, but also remove the nodes"),
	//TODO: TYPE_METHOD("prevnode", &rangemap_prevnode,
	//TODO:             "(key)->?X2?T3?O?O?O?N\n"
	//TODO:             "#r{The ${minkey,maxkey,value} triple of the node that comes before "
	//TODO:             /**/ "the one containing @key, or ?N when no such node exists}"),
	//TODO: TYPE_METHOD("nextnode", &rangemap_nextnode,
	//TODO:             "(key)->?X2?T3?O?O?O?N\n"
	//TODO:             "#r{The ${minkey,maxkey,value} triple of the node that comes after "
	//TODO:             /**/ "the one containing @key, or ?N when no such node exists}"),
	//TODO: TYPE_METHOD("itprevnode", &rangemap_itprevnode,
	//TODO:             "(key)->?X2?#Iterator?N\n"
	//TODO:             "#r{An ?#Iterator bound to the node that comes before "
	//TODO:             /**/ "the one containing @key, or ?N when no such node exists}"),
	//TODO: TYPE_METHOD("itnextnode", &rangemap_itnextnode,
	//TODO:             "(key)->?X2?#Iterator?N\n"
	//TODO:             "#r{An ?#Iterator bound to the node that comes after "
	//TODO:             /**/ "the one containing @key, or ?N when no such node exists}"),

	TYPE_METHOD("popfront", &rangemap_popfront,
	            "->?T3?O?O?O\n"
	            DOC_ERROR_ValueError_EMPTY_SEQUENCE
	            "#r{the lowest element that used to be stored in @this ?. (s.a. ?#first)}"),
	TYPE_METHOD("popback", &rangemap_popback,
	            "->?T3?O?O?O\n"
	            DOC_ERROR_ValueError_EMPTY_SEQUENCE
	            "#r{the greatest element that used to be stored in @this ?. (s.a. ?#last)}"),
	//TODO: TYPE_METHOD("optimize", &rangemap_optimize,
	//TODO:             "()\n"
	//TODO:             "Search for adjacent nodes that can be merged due to having identical (as per ${===}) "
	//TODO:             /**/ "values, as well as consecutive key-ranges. By default, this sort of optimization "
	//TODO:             /**/ "is only done when using ?Dint as key type, though if custom key types are used, "
	//TODO:             /**/ "then this function may be used to perform that same optimization."),

	/* Generic methods */
	TYPE_METHOD("clear", &rangemap_clear,
	            "()\n"
	            "Clear all values from @this ?."),
	TYPE_METHOD("popitem", &rangemap_popitem,
	            "->?T3?O?O?O\n"
	            DOC_ERROR_ValueError_EMPTY_SEQUENCE
	            "#r{A random pair minkey-maxkey-value pair that has been removed}"),
	TYPE_METHOD("update", &rangemap_update,
	            "(items:?S?T3?O?O?O)\n"
	            DOC_ERROR_NotImplemented_CANNOT_SPLIT
	            "Iterate @items and unpack each element into 3 others, "
	            /**/ "using them as #C{minkey,maxkey,value} to insert into @this ?."),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst rangemap_getsets[] = {
	TYPE_GETTER("first", &rangemap_get_first,
	            "->?T3?O?O?O\n"
	            DOC_ERROR_ValueError_EMPTY_SEQUENCE
	            "Return the triple #C{minkey,maxkey,value} for the lowest range in @this ?."),
	TYPE_GETTER("last", &rangemap_get_last,
	            "->?T3?O?O?O\n"
	            DOC_ERROR_ValueError_EMPTY_SEQUENCE
	            "Return the triple #C{minkey,maxkey,value} for the greatest range in @this ?."),
	TYPE_GETTER("keys", &rangemap_keys,
	            "->?#Keys\n"
	            "Return a ?DSet-compatible proxy-view for ${{Key...}}, meaning "
	            /**/ "it contains all keys that are apart of any of the nodes of "
	            /**/ "@this ?.. This ?DSet can only be enumerated if all used "
	            /**/ "keys implement ${operator copy} and ${operator ++}. "
	            /**/ "Otherwise, it is still possible to check for membership "
	            /**/ "using ${operator contains} (which behaves the same as ?#{op:contains})"),
	TYPE_GETTER("values", &rangemap_values,
	            "->?#Values\n"
	            "Return a ?DSequence-proxy for ${{Values...}}, enumerating the value of each distinct node"),
	TYPE_GETTER("items", &rangemap_items,
	            "->?#Items\n"
	            "Return a ?DSequence-proxy for ${{MinKey, MaxKey, Value...}}, "
	            /**/ "enumerating the bounds and value of each node"),
	TYPE_GETTER("nodes", &rangemap_nodes,
	            "->?#Nodes\n"
	            "Return a ?DSequence-proxy for ${{Iterator...}}, "
	            /**/ "enumerating iterators for every node of @this ?."),
	TYPE_GETTER("ranges", &rangemap_ranges,
	            "->?#Ranges\n"
	            "Return a ?DSequence-proxy for ${{(MinKey, MaxKey)...}}, "
	            /**/ "enumerating the bounds of the ranges of @this ?."),
	TYPE_GETTER("mapitems", &rangemap_mapitems,
	            "->?#MapItems\n"
	            "Return a ?DSequence-proxy for ${{(Key, Value)...}}, "
	            /**/ "enumerating all keys contained in nodes, as well as "
	            /**/ "repeating the value of the relevant node for each key.\n"
	            "Same as ${this.asmap.items}"),
	TYPE_GETTER("asmap", &rangemap_asmap,
	            "->?#AsMap\n"
	            "Return a ?DMapping-proxy for @this ?., which allows @this "
	            /**/ "to be used as though it were a regular ?DMapping."),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst rangemap_class_getsets[] = {
	TYPE_GETTER("Iterator", &rangemap_iterator_get,
	            "->?DType\n"
	            "Returns the iterator class used by instances of @this ?. type\n"
	            "This member must be overwritten by sub-classes of ?."),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rangemap_class_members[] = {
	TYPE_MEMBER_CONST("Keys", &RangeMapKeys_Type),
	TYPE_MEMBER_CONST("Values", &RangeMapValues_Type),
	TYPE_MEMBER_CONST("Items", &RangeMapItems_Type),
	TYPE_MEMBER_CONST("Nodes", &RangeMapNodes_Type),
	TYPE_MEMBER_CONST("Ranges", &RangeMapRanges_Type),
	TYPE_MEMBER_CONST("Proxy", &RangeMapProxy_Type),
	TYPE_MEMBER_CONST("MapItems", &RangeMapMapItems_Type),
	TYPE_MEMBER_CONST("AsMap", &RangeMapAsMap_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RangeMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RangeMap",
	/* TODO: Document the minimal feature-set required by sub-classes
	 *       (and what needs to be implemented for optimal performance) */
	/* .tp_doc      = */ DOC("Base-class for range-based mappings"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&DeeNone_OperatorCtor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeNone_OperatorCopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL, /* Inherited function already does the correct thing */
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rangemap_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rangemap_cmp,
	/* .tp_seq           = */ &rangemap_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rangemap_methods,
	/* .tp_getsets       = */ rangemap_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ rangemap_class_getsets,
	/* .tp_class_members = */ rangemap_class_members
};







/************************************************************************/
/* PROXY TYPES                                                          */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_ctor(RangeMapProxy *__restrict self) {
	self->rmp_rmap = Dee_EmptyRangeMap;
	Dee_Incref(self->rmp_rmap);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_init(RangeMapProxy *__restrict self, size_t argc,
           DeeObject *const *argv) {
	self->rmp_rmap = Dee_EmptyRangeMap;
	_DeeArg_Unpack0Or1(err, argc, argv, "_RangeMapProxy", &self->rmp_rmap);
	Dee_Incref(self->rmp_rmap);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_copy(RangeMapProxy *__restrict self,
           RangeMapProxy *__restrict other) {
	self->rmp_rmap = other->rmp_rmap;
	Dee_Incref(self->rmp_rmap);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_deepcopy(RangeMapProxy *__restrict self,
               RangeMapProxy *__restrict other) {
	self->rmp_rmap = DeeObject_DeepCopy(other->rmp_rmap);
	if unlikely(!self->rmp_rmap)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_fini(RangeMapProxy *__restrict self) {
	Dee_Decref(self->rmp_rmap);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_visit(RangeMapProxy *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rmp_rmap);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
proxy_size(RangeMapProxy *__restrict self) {
	return DeeObject_Size(self->rmp_rmap);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_sizeob(RangeMapProxy *__restrict self) {
	return DeeObject_SizeOb(self->rmp_rmap);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_bool(RangeMapProxy *__restrict self) {
	return DeeObject_Bool(self->rmp_rmap);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
proxy_keys_contains(RangeMapProxy *self, DeeObject *key) {
	return DeeObject_Contains(self->rmp_rmap, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
proxy_asmap_getitem(RangeMapProxy *self, DeeObject *key) {
	return DeeObject_GetItem(self->rmp_rmap, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
proxy_asmap_trygetitem(RangeMapProxy *self, DeeObject *key) {
	return DeeObject_TryGetItem(self->rmp_rmap, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_asmap_delitem(RangeMapProxy *self, DeeObject *key) {
	return DeeObject_DelItem(self->rmp_rmap, key);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
proxy_asmap_setitem(RangeMapProxy *self, DeeObject *key, DeeObject *value) {
	return DeeObject_SetItem(self->rmp_rmap, key, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF RangeMapProxyItemsIterator *DCALL
proxy_iterself(RangeMapProxy *__restrict self, DeeTypeObject *__restrict result_type) {
	DREF RangeMapProxyItemsIterator *result;
	result = DeeObject_MALLOC(RangeMapProxyItemsIterator);
	if unlikely(!result)
		goto done;
	result->rmpii_base.rmpi_iter = DeeObject_Iter(self->rmp_rmap);
	if unlikely(!result->rmpii_base.rmpi_iter)
		goto err_r;
	result->rmpii_base.rmpi_rmap = self->rmp_rmap;
	Dee_Incref(result->rmpii_base.rmpi_rmap);
	DeeObject_Init(&result->rmpii_base, result_type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxyKeysIterator *DCALL
proxy_iterself_keys(RangeMapProxy *__restrict self) {
	DREF RangeMapProxyKeysIterator *result;
	result = DeeObject_MALLOC(RangeMapProxyKeysIterator);
	if unlikely(!result)
		goto done;
	result->rmpki_base.rmpii_base.rmpi_iter = DeeObject_Iter(self->rmp_rmap);
	if unlikely(!result->rmpki_base.rmpii_base.rmpi_iter)
		goto err_r;
	result->rmpki_base.rmpii_base.rmpi_rmap = self->rmp_rmap;
	Dee_Incref(result->rmpki_base.rmpii_base.rmpi_rmap);
	Dee_Incref_n(_DeeInt_Zero, 2);
	result->rmpki_prvkey = (DeeObject *)_DeeInt_Zero;
	result->rmpki_maxkey = (DeeObject *)_DeeInt_Zero;
	Dee_atomic_lock_init(&result->rmpki_lock);
	result->rmpki_first = false;
	DeeObject_Init(&result->rmpki_base.rmpii_base, &RangeMapKeysIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxyItemsIterator *DCALL
proxy_iterself_values(RangeMapProxy *__restrict self) {
	return proxy_iterself(self, &RangeMapValuesIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxyItemsIterator *DCALL
proxy_iterself_items(RangeMapProxy *__restrict self) {
	return proxy_iterself(self, &RangeMapItemsIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxyIterator *DCALL
proxy_iterself_nodes(RangeMapProxy *__restrict self) {
	DREF RangeMapProxyIterator *result;
	result = DeeObject_MALLOC(RangeMapProxyIterator);
	if unlikely(!result)
		goto done;
	result->rmpi_iter = DeeObject_Iter(self->rmp_rmap);
	if unlikely(!result->rmpi_iter)
		goto err_r;
	result->rmpi_rmap = self->rmp_rmap;
	Dee_Incref(result->rmpi_rmap);
	DeeObject_Init(result, &RangeMapNodesIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxyItemsIterator *DCALL
proxy_iterself_ranges(RangeMapProxy *__restrict self) {
	return proxy_iterself(self, &RangeMapRangesIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxyKeysIterator *DCALL
make_RangeMapProxyMapItemsIterator(RangeMapProxy *self, DeeTypeObject *type) {
	DREF RangeMapProxyKeysIterator *result;
	result = DeeObject_MALLOC(RangeMapProxyKeysIterator);
	if unlikely(!result)
		goto done;
	result->rmpki_base.rmpii_base.rmpi_iter = DeeObject_Iter(self->rmp_rmap);
	if unlikely(!result->rmpki_base.rmpii_base.rmpi_iter)
		goto err_r;
	result->rmpki_base.rmpii_base.rmpi_rmap = self->rmp_rmap;
	Dee_Incref(result->rmpki_base.rmpii_base.rmpi_rmap);
	Dee_Incref_n(_DeeInt_Zero, 3);
	result->rmpki_base.rmpii_value = (DeeObject *)_DeeInt_Zero;
	result->rmpki_prvkey           = (DeeObject *)_DeeInt_Zero;
	result->rmpki_maxkey           = (DeeObject *)_DeeInt_Zero;
	Dee_atomic_lock_init(&result->rmpki_lock);
	result->rmpki_first = false;
	DeeObject_Init(&result->rmpki_base.rmpii_base, type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxyKeysIterator *DCALL
proxy_iterself_mapitems(RangeMapProxy *__restrict self) {
	return make_RangeMapProxyMapItemsIterator(self, &RangeMapMapItemsIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxyKeysIterator *DCALL
proxy_iterself_asmap(RangeMapProxy *__restrict self) {
	return make_RangeMapProxyMapItemsIterator(self, &RangeMapAsMapIterator_Type);
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
rangemap_keys_sizeof_range(DeeObject *minkey, DeeObject *maxkey) {
	size_t result;
	DREF DeeObject *temp;
	if (minkey == maxkey)
		return 1;
	temp = DeeObject_Sub(maxkey, minkey);
	if unlikely(!temp)
		goto err;
	if unlikely(DeeObject_AsSize(temp, &result))
		goto err_temp;
	Dee_Decref(temp);
	if unlikely(result >= (size_t)-2) {
		DeeError_Throwf(&DeeError_IntegerOverflow,
		                "Range size is too great");
		goto err;
	}
	return result + 1;
err_temp:
	Dee_Decref(temp);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((2)) dssize_t DCALL
rangemap_keys_getsize_foreach(void *arg, DeeObject *elem) {
	size_t temp, *p_result = (size_t *)arg;
	DREF DeeObject *item[3];
	if unlikely(DeeSeq_Unpack(elem, 3, item))
		goto err;
	Dee_Decref(item[2]);
	temp = rangemap_keys_sizeof_range(item[0], item[1]);
	Dee_Decref(item[1]);
	Dee_Decref(item[0]);
	if unlikely(temp == (size_t)-1)
		goto err;
	if (OVERFLOW_UADD(*p_result, temp, p_result))
		goto err_overflow;
	return 0;
err_overflow:
	DeeError_Throwf(&DeeError_IntegerOverflow,
	                "Sum of range sizes is too great");
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rangemap_keys_getsize(DeeObject *__restrict self) {
	size_t result = 0;
	if unlikely(DeeObject_Foreach(self, &rangemap_keys_getsize_foreach, &result) < 0)
		goto err;
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
proxy_keys_getsize(RangeMapProxy *__restrict self) {
	return rangemap_keys_getsize(self->rmp_rmap);
}

PRIVATE struct type_seq proxy_keys_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_keys,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&proxy_keys_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_keys_getsize,
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
	/* .tp_trygetitem                 = */ NULL,
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

PRIVATE struct type_seq proxy_values_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_values,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
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
	/* .tp_trygetitem                 = */ NULL,
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

PRIVATE struct type_seq proxy_items_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_items,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
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
	/* .tp_trygetitem                 = */ NULL,
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

PRIVATE struct type_seq proxy_nodes_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_nodes,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
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
	/* .tp_trygetitem                 = */ NULL,
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

PRIVATE struct type_seq proxy_ranges_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_ranges,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
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
	/* .tp_trygetitem                 = */ NULL,
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

PRIVATE struct type_seq proxy_mapitems_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_mapitems,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_keys_getsize,
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
	/* .tp_trygetitem                 = */ NULL,
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

PRIVATE struct type_seq proxy_asmap_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_iterself_asmap,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&proxy_keys_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&proxy_asmap_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_asmap_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&proxy_asmap_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_keys_getsize,
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
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&proxy_asmap_trygetitem,
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

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_asmap_keys(RangeMapProxy *__restrict self) {
	return RangeMapProxy_New(self->rmp_rmap, &RangeMapKeys_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_asmap_items(RangeMapProxy *__restrict self) {
	return RangeMapProxy_New(self->rmp_rmap, &RangeMapMapItems_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_setdefault(RangeMapProxy *__restrict self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrStringHash(self->rmp_rmap, "setdefault", Dee_HashStr__setdefault, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_pop(RangeMapProxy *__restrict self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrStringHash(self->rmp_rmap, "pop", Dee_HashStr__pop, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_clear(RangeMapProxy *__restrict self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrStringHash(self->rmp_rmap, "clear", Dee_HashStr__clear, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_setold(RangeMapProxy *__restrict self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrStringHash(self->rmp_rmap, "setold", Dee_HashStr__setold, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_setold_ex(RangeMapProxy *__restrict self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrStringHash(self->rmp_rmap, "setold_ex", Dee_HashStr__setold_ex, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_setnew(RangeMapProxy *__restrict self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrStringHash(self->rmp_rmap, "setnew", Dee_HashStr__setnew, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_setnew_ex(RangeMapProxy *__restrict self, size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrStringHash(self->rmp_rmap, "setnew_ex", Dee_HashStr__setnew_ex, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_get_first(RangeMapProxy *__restrict self) {
	int error;
	DREF DeeObject *triple, *result;
	DREF DeeObject *item[3];
	triple = DeeObject_GetAttrStringHash(self->rmp_rmap, "first", Dee_HashStr__first);
	if unlikely(!triple)
		goto err;
	error = DeeSeq_Unpack(triple, 3, item);
	Dee_Decref(triple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[1]);
	result = DeeTuple_PackSymbolic(2, item[0], item[2]);
	if unlikely(!result)
		goto err_item_0_2;
	return result;
err_item_0_2:
	Dee_Decref(item[2]);
	Dee_Decref(item[0]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_asmap_get_last(RangeMapProxy *__restrict self) {
	int error;
	DREF DeeObject *triple, *result;
	DREF DeeObject *item[3];
	triple = DeeObject_GetAttrStringHash(self->rmp_rmap, "last", Dee_HashStr__last);
	if unlikely(!triple)
		goto err;
	error = DeeSeq_Unpack(triple, 3, item);
	Dee_Decref(triple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[0]);
	result = DeeTuple_NewVectorSymbolic(2, item + 1);
	if unlikely(!result)
		goto err_item_1_2;
	return result;
err_item_1_2:
	Dee_Decref(item[2]);
	Dee_Decref(item[1]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_keys_get_first(RangeMapProxy *__restrict self) {
	int error;
	DREF DeeObject *triple;
	DREF DeeObject *item[3];
	triple = DeeObject_GetAttrStringHash(self->rmp_rmap, "first", Dee_HashStr__first);
	if unlikely(!triple)
		goto err;
	error = DeeSeq_Unpack(triple, 3, item);
	Dee_Decref(triple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[2]);
	Dee_Decref(item[1]);
	return item[0];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_keys_get_last(RangeMapProxy *__restrict self) {
	int error;
	DREF DeeObject *triple;
	DREF DeeObject *item[3];
	triple = DeeObject_GetAttrStringHash(self->rmp_rmap, "last", Dee_HashStr__last);
	if unlikely(!triple)
		goto err;
	error = DeeSeq_Unpack(triple, 3, item);
	Dee_Decref(triple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[2]);
	Dee_Decref(item[0]);
	return item[1];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_values_get_first(RangeMapProxy *__restrict self) {
	int error;
	DREF DeeObject *triple;
	DREF DeeObject *item[3];
	triple = DeeObject_GetAttrStringHash(self->rmp_rmap, "first", Dee_HashStr__first);
	if unlikely(!triple)
		goto err;
	error = DeeSeq_Unpack(triple, 3, item);
	Dee_Decref(triple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[1]);
	Dee_Decref(item[0]);
	return item[2];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_values_get_last(RangeMapProxy *__restrict self) {
	int error;
	DREF DeeObject *triple;
	DREF DeeObject *item[3];
	triple = DeeObject_GetAttrStringHash(self->rmp_rmap, "last", Dee_HashStr__last);
	if unlikely(!triple)
		goto err;
	error = DeeSeq_Unpack(triple, 3, item);
	Dee_Decref(triple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[1]);
	Dee_Decref(item[0]);
	return item[2];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_items_get_first(RangeMapProxy *__restrict self) {
	return DeeObject_GetAttrStringHash(self->rmp_rmap, "first", Dee_HashStr__first);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_items_get_last(RangeMapProxy *__restrict self) {
	return DeeObject_GetAttrStringHash(self->rmp_rmap, "last", Dee_HashStr__last);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_ranges_get_first(RangeMapProxy *__restrict self) {
	int error;
	DREF DeeObject *triple, *result;
	DREF DeeObject *item[3];
	triple = DeeObject_GetAttrStringHash(self->rmp_rmap, "first", Dee_HashStr__first);
	if unlikely(!triple)
		goto err;
	error = DeeSeq_Unpack(triple, 3, item);
	Dee_Decref(triple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[2]);
	result = DeeTuple_NewVectorSymbolic(2, item);
	if unlikely(!result)
		goto err_item_0_1;
	return result;
err_item_0_1:
	Dee_Decref(item[1]);
	Dee_Decref(item[0]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_ranges_get_last(RangeMapProxy *__restrict self) {
	int error;
	DREF DeeObject *triple, *result;
	DREF DeeObject *item[3];
	triple = DeeObject_GetAttrStringHash(self->rmp_rmap, "last", Dee_HashStr__last);
	if unlikely(!triple)
		goto err;
	error = DeeSeq_Unpack(triple, 3, item);
	Dee_Decref(triple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[2]);
	result = DeeTuple_NewVectorSymbolic(2, item);
	if unlikely(!result)
		goto err_item_0_1;
	return result;
err_item_0_1:
	Dee_Decref(item[1]);
	Dee_Decref(item[0]);
err:
	return NULL;
}

PRIVATE struct type_method tpconst proxy_asmap_methods[] = {
	TYPE_METHOD_F("setdefault", &proxy_asmap_setdefault, METHOD_FNOREFESCAPE,
	              "(key,def=!N)->\n"
	              "#r{The object currently assigned to @key}"
	              "Lookup @key in @this ?. and return its value if found. "
	              /**/ "Otherwise, assign @def to @key and return it instead"),
	TYPE_METHOD_F("pop", &proxy_asmap_pop, METHOD_FNOREFESCAPE,
	              "(key)->\n"
	              "(key,def)->\n"
	              "#tKeyError{No @def was given and @key was not found}"
	              "Delete @key from @this ?. and return its previously assigned "
	              /**/ "value or @def when @key had no item associated"),
	TYPE_METHOD_F("clear", &proxy_asmap_clear, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Clear all values from @this ?."),
	TYPE_METHOD_F("setold", &proxy_asmap_setold, METHOD_FNOREFESCAPE,
	              "(key,value)->?Dbool\n"
	              "#r{Indicative of @value having been assigned to @key}"
	              "Assign @value to @key, only succeeding when @key already existed to begin with"),
	TYPE_METHOD_F("setnew", &proxy_asmap_setnew, METHOD_FNOREFESCAPE,
	              "(key,value)->?Dbool\n"
	              "#r{Indicative of @value having been assigned to @key}"
	              "Assign @value to @key, only succeeding when @key didn't exist before"),
	TYPE_METHOD_F("setold_ex", &proxy_asmap_setold_ex, METHOD_FNOREFESCAPE,
	              "(key,value)->?T2?Dbool?O\n"
	              "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
	              "Same as ?#setold but also return the previously assigned value"),
	TYPE_METHOD_F("setnew_ex", &proxy_asmap_setnew_ex, METHOD_FNOREFESCAPE,
	              "(key,value)->?T2?Dbool?O\n"
	              "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
	              "Same as ?#setnew but return the previously assigned value on failure"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst proxy_asmap_getsets[] = {
	TYPE_GETTER_F("first", &proxy_asmap_get_first, METHOD_FNOREFESCAPE, "->?T2?O?O"),
	TYPE_GETTER_F("last", &proxy_asmap_get_last, METHOD_FNOREFESCAPE, "->?T2?O?O"),
	TYPE_GETTER_F("keys", &proxy_asmap_keys, METHOD_FNOREFESCAPE, "->?#Keys"),
	/* NOT overwritten (our version wouldn't repeat values for every key in a range)
	 * As such, simply make use of the default Mapping.Values type. */
	/*TYPE_GETTER("values", &proxy_asmap_values, "->?#Values"),*/
	TYPE_GETTER_F("items", &proxy_asmap_items, METHOD_FNOREFESCAPE, "->?#Items"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_keys_getsets[] = {
	TYPE_GETTER_F_NODOC("first", &proxy_keys_get_first, METHOD_FNOREFESCAPE),
	TYPE_GETTER_F_NODOC("last", &proxy_keys_get_last, METHOD_FNOREFESCAPE),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_values_getsets[] = {
	TYPE_GETTER_F_NODOC("first", &proxy_values_get_first, METHOD_FNOREFESCAPE),
	TYPE_GETTER_F_NODOC("last", &proxy_values_get_last, METHOD_FNOREFESCAPE),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_items_getsets[] = {
	TYPE_GETTER_F("first", &proxy_items_get_first, METHOD_FNOREFESCAPE, "->?T3?O?O?O"),
	TYPE_GETTER_F("last", &proxy_items_get_last, METHOD_FNOREFESCAPE, "->?T3?O?O?O"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_ranges_getsets[] = {
	TYPE_GETTER_F("first", &proxy_ranges_get_first, METHOD_FNOREFESCAPE, "->?T2?O?O"),
	TYPE_GETTER_F("last", &proxy_ranges_get_last, METHOD_FNOREFESCAPE, "->?T2?O?O"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_mapitems_getsets[] = {
	TYPE_GETTER_F("first", &proxy_asmap_get_first, METHOD_FNOREFESCAPE, "->?T2?O?O"),
	TYPE_GETTER_F("last", &proxy_asmap_get_last, METHOD_FNOREFESCAPE, "->?T2?O?O"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst proxy_asmap_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RangeMapAsMapIterator_Type),
	TYPE_MEMBER_CONST("Keys", &RangeMapKeys_Type),
	/*TYPE_MEMBER_CONST("Values", &RangeMapValues_Type),*/
	TYPE_MEMBER_CONST("Items", &RangeMapMapItems_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RangeMapProxyIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_keys_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RangeMapKeysIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_values_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RangeMapValuesIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_items_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RangeMapItemsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_nodes_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RangeMapNodesIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_ranges_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RangeMapRangesIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_mapitems_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RangeMapMapItemsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst proxy_members[] = {
	TYPE_MEMBER_FIELD_DOC("__rangemap__", STRUCT_OBJECT, offsetof(RangeMapProxy, rmp_rmap), "->?GRangeMap"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject RangeMapProxy_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapProxy",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?GRangeMap)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxy)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_items_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_class_members
};

PRIVATE DeeTypeObject *tpconst rmapping_keys_mro[] = {
	&RangeMapProxy_Type,
	&DeeSet_Type,
	&DeeSeq_Type,
	&DeeObject_Type,
	NULL
};

INTERN DeeTypeObject RangeMapKeys_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapKeys",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?GRangeMap)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_keys_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_keys_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_keys_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
	/* .tp_mro           = */ rmapping_keys_mro
};

INTERN DeeTypeObject RangeMapValues_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapValues",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?GRangeMap)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_values_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_values_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_values_class_members
};

INTERN DeeTypeObject RangeMapItems_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapItems",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?GRangeMap)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_items_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_items_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_items_class_members
};

INTERN DeeTypeObject RangeMapNodes_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapNodes",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?GRangeMap)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_nodes_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL, /* XXX: first/last */
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_nodes_class_members
};

INTERN DeeTypeObject RangeMapRanges_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapRanges",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?GRangeMap)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_ranges_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_ranges_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_ranges_class_members
};

INTERN DeeTypeObject RangeMapMapItems_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapMapItems",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?GRangeMap)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_mapitems_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_mapitems_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_mapitems_class_members
};

PRIVATE DeeTypeObject *tpconst rmapping_asmap_mro[] = {
	&DeeMapping_Type,
	&RangeMapProxy_Type,
	&DeeSeq_Type,
	&DeeObject_Type,
	NULL
};

INTERN DeeTypeObject RangeMapAsMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapAsMap",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?GRangeMap)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxy)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_asmap_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ proxy_asmap_methods,
	/* .tp_getsets       = */ proxy_asmap_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ proxy_asmap_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
	/* .tp_mro           = */ rmapping_asmap_mro
};



PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_iterator_get_keys(RangeMapProxyKeysIterator *__restrict self) {
	return RangeMapProxy_New(self->rmpki_base.rmpii_base.rmpi_rmap, &RangeMapKeys_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_iterator_get_values(RangeMapProxyItemsIterator *__restrict self) {
	return RangeMapProxy_New(self->rmpii_base.rmpi_rmap, &RangeMapValues_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_iterator_get_items(RangeMapProxyItemsIterator *__restrict self) {
	return RangeMapProxy_New(self->rmpii_base.rmpi_rmap, &RangeMapItems_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_iterator_get_nodes(RangeMapProxyIterator *__restrict self) {
	return RangeMapProxy_New(self->rmpi_rmap, &RangeMapNodes_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_iterator_get_ranges(RangeMapProxyItemsIterator *__restrict self) {
	return RangeMapProxy_New(self->rmpii_base.rmpi_rmap, &RangeMapRanges_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_iterator_get_mapitems(RangeMapProxyKeysIterator *__restrict self) {
	return RangeMapProxy_New(self->rmpki_base.rmpii_base.rmpi_rmap, &RangeMapMapItems_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF RangeMapProxy *DCALL
proxy_iterator_get_asmap(RangeMapProxyKeysIterator *__restrict self) {
	return RangeMapProxy_New(self->rmpki_base.rmpii_base.rmpi_rmap, &RangeMapAsMap_Type);
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_iterator_ctor(RangeMapProxyIterator *__restrict self) {
	self->rmpi_iter = DeeObject_Iter(Dee_EmptyRangeMap);
	if unlikely(!self->rmpi_iter)
		goto err;
	self->rmpi_rmap = Dee_EmptyRangeMap;
	Dee_Incref(self->rmpi_rmap);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_iterator_init(RangeMapProxyIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_RangeMapProxyIterator", &self->rmpi_rmap);
	self->rmpi_iter = DeeObject_Iter(self->rmpi_rmap);
	if unlikely(!self->rmpi_iter)
		goto err;
	Dee_Incref(self->rmpi_rmap);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_iterator_copy(RangeMapProxyIterator *__restrict self,
                    RangeMapProxyIterator *__restrict other) {
	self->rmpi_iter = DeeObject_Copy(other->rmpi_iter);
	if unlikely(!self->rmpi_iter)
		goto err;
	self->rmpi_rmap = other->rmpi_rmap;
	Dee_Incref(self->rmpi_rmap);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_iterator_deepcopy(RangeMapProxyIterator *__restrict self,
                        RangeMapProxyIterator *__restrict other) {
	self->rmpi_iter = DeeObject_DeepCopy(other->rmpi_iter);
	if unlikely(!self->rmpi_iter)
		goto err;
	self->rmpi_rmap = DeeObject_DeepCopy(other->rmpi_rmap);
	if unlikely(!self->rmpi_iter)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->rmpi_iter);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_iterator_fini(RangeMapProxyIterator *__restrict self) {
	Dee_Decref(self->rmpi_iter);
	Dee_Decref(self->rmpi_rmap);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_iterator_visit(RangeMapProxyIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rmpi_iter);
	Dee_Visit(self->rmpi_rmap);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_iterator_bool(RangeMapProxyIterator *__restrict self) {
	return DeeObject_Bool(self->rmpi_iter);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_items_iterator_ctor(RangeMapProxyItemsIterator *__restrict self) {
	self->rmpii_base.rmpi_iter = DeeObject_Iter(Dee_EmptyRangeMap);
	if unlikely(!self->rmpii_base.rmpi_iter)
		goto err;
	self->rmpii_base.rmpi_rmap = Dee_EmptyRangeMap;
	Dee_Incref(self->rmpii_base.rmpi_rmap);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_items_iterator_init(RangeMapProxyItemsIterator *__restrict self,
                          size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_RangeMapItemsIterator", &self->rmpii_base.rmpi_rmap);
	self->rmpii_base.rmpi_iter = DeeObject_Iter(self->rmpii_base.rmpi_rmap);
	if unlikely(!self->rmpii_base.rmpi_iter)
		goto err;
	Dee_Incref(self->rmpii_base.rmpi_rmap);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_items_iterator_copy(RangeMapProxyItemsIterator *__restrict self,
                          RangeMapProxyItemsIterator *__restrict other) {
	self->rmpii_base.rmpi_iter = DeeObject_Copy(other->rmpii_base.rmpi_iter);
	if unlikely(!self->rmpii_base.rmpi_iter)
		goto err;
	self->rmpii_base.rmpi_rmap = other->rmpii_base.rmpi_rmap;
	Dee_Incref(self->rmpii_base.rmpi_rmap);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_items_iterator_deepcopy(RangeMapProxyItemsIterator *__restrict self,
                              RangeMapProxyItemsIterator *__restrict other) {
	self->rmpii_base.rmpi_iter = DeeObject_DeepCopy(other->rmpii_base.rmpi_iter);
	if unlikely(!self->rmpii_base.rmpi_iter)
		goto err;
	self->rmpii_base.rmpi_rmap = DeeObject_DeepCopy(other->rmpii_base.rmpi_rmap);
	if unlikely(!self->rmpii_base.rmpi_iter)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->rmpii_base.rmpi_iter);
err:
	return -1;
}

PRIVATE struct type_member tpconst proxy_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("__rangemap__", STRUCT_OBJECT, offsetof(RangeMapProxyIterator, rmpi_rmap), "->?GRangeMap"),
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(RangeMapProxyIterator, rmpi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_items_iterator_next_range(RangeMapProxyItemsIterator *__restrict self) {
	int error;
	DREF DeeObject *tuple, *result;
	DREF DeeObject *item[3];
	tuple = DeeObject_IterNext(self->rmpii_base.rmpi_iter);
	if (!ITER_ISOK(tuple))
		return tuple;
	error = DeeSeq_Unpack(tuple, 3, item);
	Dee_Decref(tuple);
	if unlikely(error)
		goto err;
	result = DeeTuple_NewVectorSymbolic(2, item);
	if unlikely(!result)
		goto err_item;
	Dee_Decref(item[2]);
	return result;
err_item:
	Dee_Decrefv(item, 3);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_items_iterator_next_value(RangeMapProxyItemsIterator *__restrict self) {
	int error;
	DREF DeeObject *tuple;
	DREF DeeObject *item[3];
	tuple = DeeObject_IterNext(self->rmpii_base.rmpi_iter);
	if (!ITER_ISOK(tuple))
		return tuple;
	error = DeeSeq_Unpack(tuple, 3, item);
	Dee_Decref(tuple);
	if unlikely(error)
		goto err;
	Dee_Decref(item[1]);
	Dee_Decref(item[0]);
	return item[2];
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_nodes_iterator_next(RangeMapProxyIterator *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_Copy(self->rmpi_iter);
	if likely(result) {
		DREF DeeObject *temp;
		temp = DeeObject_IterNext(self->rmpi_iter);
		if (ITER_ISOK(temp)) {
			Dee_Decref(temp);
		} else {
			Dee_Decref(result);
			result = temp;
		}
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_items_iterator_next_item(RangeMapProxyItemsIterator *__restrict self) {
	return DeeObject_IterNext(self->rmpii_base.rmpi_iter);
}

PRIVATE struct type_getset tpconst proxy_iterator_keys_getsets[] = {
	TYPE_GETTER_F("seq", &proxy_iterator_get_keys, METHOD_FNOREFESCAPE, "->?AKeys?GRangeMap"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_iterator_values_getsets[] = {
	TYPE_GETTER_F("seq", &proxy_iterator_get_values, METHOD_FNOREFESCAPE, "->?AValues?GRangeMap"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_iterator_items_getsets[] = {
	TYPE_GETTER_F("seq", &proxy_iterator_get_items, METHOD_FNOREFESCAPE, "->?AItems?GRangeMap"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_iterator_nodes_getsets[] = {
	TYPE_GETTER_F("seq", &proxy_iterator_get_nodes, METHOD_FNOREFESCAPE, "->?ANodes?GRangeMap"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_iterator_ranges_getsets[] = {
	TYPE_GETTER_F("seq", &proxy_iterator_get_ranges, METHOD_FNOREFESCAPE, "->?ARanges?GRangeMap"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_iterator_mapitems_getsets[] = {
	TYPE_GETTER_F("seq", &proxy_iterator_get_mapitems, METHOD_FNOREFESCAPE, "->?AMapItems?GRangeMap"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst proxy_iterator_asmap_getsets[] = {
	TYPE_GETTER_F("seq", &proxy_iterator_get_asmap, METHOD_FNOREFESCAPE, "->?AAsMap?GRangeMap"),
	TYPE_GETSET_END
};



INTERN DeeTypeObject RangeMapProxyIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapProxyIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxyIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_iterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_iterator_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_iterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL, /* TODO: `minkey', `maxkey', `value' */
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject RangeMapValuesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapValuesIterator",
	/* .tp_doc      = */ NULL, /* Value */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_items_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_items_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_items_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_items_iterator_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxyItemsIterator)
			}
		},
		/* .tp_dtor        = */ NULL, /* Inherited... */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL /* Inherited... */
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_items_iterator_next_value,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_iterator_values_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject RangeMapItemsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapItemsIterator",
	/* .tp_doc      = */ DOC("next->?T3?O?O?O"), /* (MinKey, MaxKey, Value) */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_items_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_items_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_items_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_items_iterator_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxyItemsIterator)
			}
		},
		/* .tp_dtor        = */ NULL, /* Inherited... */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL /* Inherited... */
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_items_iterator_next_item,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_iterator_items_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject RangeMapRangesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapRangesIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"), /* (MinKey, MaxKey) */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_items_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_items_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_items_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_items_iterator_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxyItemsIterator)
			}
		},
		/* .tp_dtor        = */ NULL, /* Inherited... */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL /* Inherited... */
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_items_iterator_next_range,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_iterator_ranges_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject RangeMapNodesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapNodesIterator",
	/* .tp_doc      = */ DOC("next->?AIterator?GRangeMap"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_iterator_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxyIterator)
			}
		},
		/* .tp_dtor        = */ NULL, /* Inherited... */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL /* Inherited... */
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_nodes_iterator_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_iterator_nodes_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_keys_iterator_ctor(RangeMapProxyKeysIterator *__restrict self) {
	self->rmpki_base.rmpii_base.rmpi_iter = DeeObject_Iter(Dee_EmptyRangeMap);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_iter)
		goto err;
	self->rmpki_base.rmpii_base.rmpi_rmap = Dee_EmptyRangeMap;
	Dee_Incref(self->rmpki_base.rmpii_base.rmpi_rmap);
	Dee_Incref_n(_DeeInt_Zero, 2);
	self->rmpki_prvkey = (DeeObject *)_DeeInt_Zero;
	self->rmpki_maxkey = (DeeObject *)_DeeInt_Zero;
	Dee_atomic_lock_init(&self->rmpki_lock);
	self->rmpki_first = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_keys_iterator_init(RangeMapProxyKeysIterator *__restrict self,
                         size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_RangeMapKeysIterator",
	                  &self->rmpki_base.rmpii_base.rmpi_rmap);
	self->rmpki_base.rmpii_base.rmpi_iter = DeeObject_Iter(self->rmpki_base.rmpii_base.rmpi_rmap);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_iter)
		goto err;
	Dee_Incref(self->rmpki_base.rmpii_base.rmpi_rmap);
	Dee_Incref_n(_DeeInt_Zero, 2);
	self->rmpki_prvkey = (DeeObject *)_DeeInt_Zero;
	self->rmpki_maxkey = (DeeObject *)_DeeInt_Zero;
	Dee_atomic_lock_init(&self->rmpki_lock);
	self->rmpki_first = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_keys_iterator_copy(RangeMapProxyKeysIterator *__restrict self,
                         RangeMapProxyKeysIterator *__restrict other) {
	self->rmpki_base.rmpii_base.rmpi_iter = DeeObject_Copy(other->rmpki_base.rmpii_base.rmpi_iter);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_iter)
		goto err;
	RangeMapProxyKeysIterator_LockAcquire(other);
	self->rmpki_prvkey = other->rmpki_prvkey;
	self->rmpki_maxkey = other->rmpki_maxkey;
	Dee_Incref(self->rmpki_prvkey);
	Dee_Incref(self->rmpki_maxkey);
	RangeMapProxyKeysIterator_LockRelease(other);
	self->rmpki_base.rmpii_base.rmpi_rmap = other->rmpki_base.rmpii_base.rmpi_rmap;
	Dee_Incref(self->rmpki_base.rmpii_base.rmpi_rmap);
	Dee_atomic_lock_init(&self->rmpki_lock);
	self->rmpki_first = true;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_keys_iterator_deepcopy(RangeMapProxyKeysIterator *__restrict self,
                             RangeMapProxyKeysIterator *__restrict other) {
	self->rmpki_base.rmpii_base.rmpi_rmap = DeeObject_DeepCopy(other->rmpki_base.rmpii_base.rmpi_rmap);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_rmap)
		goto err;
	self->rmpki_base.rmpii_base.rmpi_iter = DeeObject_DeepCopy(other->rmpki_base.rmpii_base.rmpi_iter);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_iter)
		goto err_rmap;
	RangeMapProxyKeysIterator_LockAcquire(other);
	self->rmpki_prvkey = other->rmpki_prvkey;
	self->rmpki_maxkey = other->rmpki_maxkey;
	Dee_Incref(self->rmpki_prvkey);
	Dee_Incref(self->rmpki_maxkey);
	RangeMapProxyKeysIterator_LockRelease(other);
	if unlikely(DeeObject_InplaceDeepCopy(&self->rmpki_prvkey))
		goto err_rmap_iter_prv_max;
	if unlikely(DeeObject_InplaceDeepCopy(&self->rmpki_maxkey))
		goto err_rmap_iter_prv_max;
	Dee_atomic_lock_init(&self->rmpki_lock);
	self->rmpki_first = false;
	return 0;
err_rmap_iter_prv_max:
	Dee_Decref(self->rmpki_maxkey);
	Dee_Decref(self->rmpki_prvkey);
	Dee_Decref(self->rmpki_base.rmpii_base.rmpi_iter);
err_rmap:
	Dee_Decref(self->rmpki_base.rmpii_base.rmpi_rmap);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_keys_iterator_fini(RangeMapProxyKeysIterator *__restrict self) {
	Dee_Decref(self->rmpki_prvkey);
	Dee_Decref(self->rmpki_maxkey);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_keys_iterator_visit(RangeMapProxyKeysIterator *__restrict self, dvisit_t proc, void *arg) {
	RangeMapProxyKeysIterator_LockAcquire(self);
	Dee_Visit(self->rmpki_prvkey);
	Dee_Visit(self->rmpki_maxkey);
	RangeMapProxyKeysIterator_LockRelease(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_mapitems_iterator_ctor(RangeMapProxyKeysIterator *__restrict self) {
	self->rmpki_base.rmpii_base.rmpi_iter = DeeObject_Iter(Dee_EmptyRangeMap);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_iter)
		goto err;
	self->rmpki_base.rmpii_base.rmpi_rmap = Dee_EmptyRangeMap;
	Dee_Incref(self->rmpki_base.rmpii_base.rmpi_rmap);
	Dee_Incref_n(_DeeInt_Zero, 3);
	self->rmpki_base.rmpii_value = (DeeObject *)_DeeInt_Zero;
	self->rmpki_prvkey           = (DeeObject *)_DeeInt_Zero;
	self->rmpki_maxkey           = (DeeObject *)_DeeInt_Zero;
	Dee_atomic_lock_init(&self->rmpki_lock);
	self->rmpki_first = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_mapitems_iterator_init(RangeMapProxyKeysIterator *__restrict self,
                         size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_RangeMapMapItemsIterator",
	                  &self->rmpki_base.rmpii_base.rmpi_rmap);
	self->rmpki_base.rmpii_base.rmpi_iter = DeeObject_Iter(self->rmpki_base.rmpii_base.rmpi_rmap);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_iter)
		goto err;
	Dee_Incref(self->rmpki_base.rmpii_base.rmpi_rmap);
	Dee_Incref_n(_DeeInt_Zero, 3);
	self->rmpki_base.rmpii_value = (DeeObject *)_DeeInt_Zero;
	self->rmpki_prvkey           = (DeeObject *)_DeeInt_Zero;
	self->rmpki_maxkey           = (DeeObject *)_DeeInt_Zero;
	Dee_atomic_lock_init(&self->rmpki_lock);
	self->rmpki_first = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_mapitems_iterator_copy(RangeMapProxyKeysIterator *__restrict self,
                         RangeMapProxyKeysIterator *__restrict other) {
	self->rmpki_base.rmpii_base.rmpi_iter = DeeObject_Copy(other->rmpki_base.rmpii_base.rmpi_iter);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_iter)
		goto err;
	RangeMapProxyKeysIterator_LockAcquire(other);
	self->rmpki_prvkey           = other->rmpki_prvkey;
	self->rmpki_maxkey           = other->rmpki_maxkey;
	self->rmpki_base.rmpii_value = other->rmpki_base.rmpii_value;
	Dee_Incref(self->rmpki_prvkey);
	Dee_Incref(self->rmpki_maxkey);
	Dee_Incref(self->rmpki_base.rmpii_value);
	RangeMapProxyKeysIterator_LockRelease(other);
	self->rmpki_base.rmpii_base.rmpi_rmap = other->rmpki_base.rmpii_base.rmpi_rmap;
	Dee_Incref(self->rmpki_base.rmpii_base.rmpi_rmap);
	Dee_atomic_lock_init(&self->rmpki_lock);
	self->rmpki_first = true;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_mapitems_iterator_deepcopy(RangeMapProxyKeysIterator *__restrict self,
                                 RangeMapProxyKeysIterator *__restrict other) {
	self->rmpki_base.rmpii_base.rmpi_rmap = DeeObject_DeepCopy(other->rmpki_base.rmpii_base.rmpi_rmap);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_rmap)
		goto err;
	self->rmpki_base.rmpii_base.rmpi_iter = DeeObject_DeepCopy(other->rmpki_base.rmpii_base.rmpi_iter);
	if unlikely(!self->rmpki_base.rmpii_base.rmpi_iter)
		goto err_rmap;
	RangeMapProxyKeysIterator_LockAcquire(other);
	self->rmpki_prvkey           = other->rmpki_prvkey;
	self->rmpki_maxkey           = other->rmpki_maxkey;
	self->rmpki_base.rmpii_value = other->rmpki_base.rmpii_value;
	Dee_Incref(self->rmpki_prvkey);
	Dee_Incref(self->rmpki_maxkey);
	Dee_Incref(self->rmpki_base.rmpii_value);
	RangeMapProxyKeysIterator_LockRelease(other);
	if unlikely(DeeObject_InplaceDeepCopy(&self->rmpki_prvkey))
		goto err_rmap_iter_prv_max_val;
	if unlikely(DeeObject_InplaceDeepCopy(&self->rmpki_maxkey))
		goto err_rmap_iter_prv_max_val;
	if unlikely(DeeObject_InplaceDeepCopy(&self->rmpki_base.rmpii_value))
		goto err_rmap_iter_prv_max_val;
	Dee_atomic_lock_init(&self->rmpki_lock);
	self->rmpki_first = false;
	return 0;
err_rmap_iter_prv_max_val:
	Dee_Decref(self->rmpki_base.rmpii_value);
	Dee_Decref(self->rmpki_maxkey);
	Dee_Decref(self->rmpki_prvkey);
	Dee_Decref(self->rmpki_base.rmpii_base.rmpi_iter);
err_rmap:
	Dee_Decref(self->rmpki_base.rmpii_base.rmpi_rmap);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_mapitems_iterator_fini(RangeMapProxyKeysIterator *__restrict self) {
	Dee_Decref(self->rmpki_base.rmpii_value);
	Dee_Decref(self->rmpki_prvkey);
	Dee_Decref(self->rmpki_maxkey);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_mapitems_iterator_visit(RangeMapProxyKeysIterator *__restrict self, dvisit_t proc, void *arg) {
	RangeMapProxyKeysIterator_LockAcquire(self);
	Dee_Visit(self->rmpki_base.rmpii_value);
	Dee_Visit(self->rmpki_prvkey);
	Dee_Visit(self->rmpki_maxkey);
	RangeMapProxyKeysIterator_LockRelease(self);
}

#define DeeObject_CompareLo_S(a, b) \
	((a) == (b) ? 0 : DeeObject_CmpLoAsBool(a, b))

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_keys_iterator_bool(RangeMapProxyKeysIterator *__restrict self) {
	int result;
	DREF DeeObject *prvkey, *maxkey;
	RangeMapProxyKeysIterator_LockAcquire(self);
	prvkey = self->rmpki_prvkey;
	maxkey = self->rmpki_maxkey;
	Dee_Incref(prvkey);
	Dee_Incref(maxkey);
	RangeMapProxyKeysIterator_LockRelease(self);
	result = DeeObject_CompareLo_S(prvkey, maxkey);
	Dee_Decref(maxkey);
	Dee_Decref(prvkey);
	if (result == 0)
		result = DeeObject_Bool(self->rmpki_base.rmpii_base.rmpi_iter);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_keys_iterator_next(RangeMapProxyKeysIterator *__restrict self) {
	int error;
	bool first;
	DREF DeeObject *prvkey, *maxkey;
again:
	RangeMapProxyKeysIterator_LockAcquire(self);
	prvkey = self->rmpki_prvkey;
	maxkey = self->rmpki_maxkey;
	first  = self->rmpki_first;
	Dee_Incref(prvkey);
	Dee_Incref(maxkey);
	RangeMapProxyKeysIterator_LockRelease(self);
	error = DeeObject_CompareLo_S(prvkey, maxkey);
	Dee_Decref(maxkey);
	if unlikely(error < 0)
		goto err_prvkey;
	if (error) {
		maxkey = prvkey;
		if (first) {
			DREF DeeObject *copy;
			copy = DeeObject_Copy(prvkey);
			Dee_Decref(prvkey);
			if unlikely(!copy)
				goto err;
			prvkey = copy;
		}
		if unlikely(DeeObject_Inc(&prvkey))
			goto err_prvkey;
		RangeMapProxyKeysIterator_LockAcquire(self);
		if unlikely(self->rmpki_prvkey != maxkey) {
			RangeMapProxyKeysIterator_LockRelease(self);
			Dee_Decref(prvkey);
			goto again;
		}
		maxkey = self->rmpki_prvkey;
		self->rmpki_prvkey = prvkey;
		self->rmpki_first  = false;
		Dee_Incref(prvkey);
		RangeMapProxyKeysIterator_LockRelease(self);
		Dee_Decref(maxkey);
	} else {
		/* Move on to the next range. */
		DREF DeeObject *item[3];
		DREF DeeObject *tuple;
		Dee_Decref(prvkey);
		tuple = DeeObject_IterNext(self->rmpki_base.rmpii_base.rmpi_iter);
		if (!ITER_ISOK(tuple))
			return tuple;
		error = DeeSeq_Unpack(tuple, 3, item);
		Dee_Decref(tuple);
		if unlikely(error)
			goto err;
		Dee_Decref(item[2]);
		Dee_Incref(item[0]);
		RangeMapProxyKeysIterator_LockAcquire(self);
		prvkey = self->rmpki_prvkey;  /* Inherit reference */
		maxkey = self->rmpki_maxkey;  /* Inherit reference */
		self->rmpki_prvkey = item[0]; /* Inherit reference */
		self->rmpki_maxkey = item[1]; /* Inherit reference */
		self->rmpki_first  = true;
		RangeMapProxyKeysIterator_LockRelease(self);
		Dee_Decref(prvkey);
		Dee_Decref(maxkey);
		prvkey = item[0];
	}
	return prvkey;
err_prvkey:
	Dee_Decref(prvkey);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_mapitems_iterator_nextpair(RangeMapProxyKeysIterator *__restrict self,
                                 DREF DeeObject *key_and_value[2]) {
	int error;
	bool first;
	DREF DeeObject *prvkey, *maxkey, *value;
again:
	RangeMapProxyKeysIterator_LockAcquire(self);
	prvkey = self->rmpki_prvkey;
	maxkey = self->rmpki_maxkey;
	first  = self->rmpki_first;
	Dee_Incref(prvkey);
	Dee_Incref(maxkey);
	RangeMapProxyKeysIterator_LockRelease(self);
	error = DeeObject_CompareLo_S(prvkey, maxkey);
	Dee_Decref(maxkey);
	if unlikely(error < 0)
		goto err_prvkey;
	if (error) {
		maxkey = prvkey;
		if (first) {
			DREF DeeObject *copy;
			copy = DeeObject_Copy(prvkey);
			Dee_Decref(prvkey);
			if unlikely(!copy)
				goto err;
			prvkey = copy;
		}
		if unlikely(DeeObject_Inc(&prvkey))
			goto err_prvkey;
		RangeMapProxyKeysIterator_LockAcquire(self);
		if unlikely(self->rmpki_prvkey != maxkey) {
			RangeMapProxyKeysIterator_LockRelease(self);
			Dee_Decref(prvkey);
			goto again;
		}
		maxkey = self->rmpki_prvkey;
		self->rmpki_prvkey = prvkey;
		Dee_Incref(prvkey);
		value = self->rmpki_base.rmpii_value;
		Dee_Incref(value);
		RangeMapProxyKeysIterator_LockRelease(self);
		Dee_Decref(maxkey);
	} else {
		/* Move on to the next range. */
		DREF DeeObject *item[3];
		DREF DeeObject *tuple;
		Dee_Decref(prvkey);
		tuple = DeeObject_IterNext(self->rmpki_base.rmpii_base.rmpi_iter);
		if (!ITER_ISOK(tuple))
			return tuple ? 1 : -1;
		error = DeeSeq_Unpack(tuple, 3, item);
		Dee_Decref(tuple);
		if unlikely(error)
			goto err;
		Dee_Incref(item[0]);
		Dee_Incref(item[2]);
		RangeMapProxyKeysIterator_LockAcquire(self);
		prvkey = self->rmpki_prvkey;            /* Inherit reference */
		maxkey = self->rmpki_maxkey;            /* Inherit reference */
		value  = self->rmpki_base.rmpii_value;  /* Inherit reference */
		self->rmpki_prvkey           = item[0]; /* Inherit reference */
		self->rmpki_maxkey           = item[1]; /* Inherit reference */
		self->rmpki_base.rmpii_value = item[2]; /* Inherit reference */
		self->rmpki_first            = true;
		RangeMapProxyKeysIterator_LockRelease(self);
		Dee_Decref(value);
		Dee_Decref(prvkey);
		Dee_Decref(maxkey);
		prvkey = item[0];
		value  = item[2];
	}

	/* Pack the key and its value into the result. */
	key_and_value[0] = prvkey; /* Inherit reference */
	key_and_value[1] = value;  /* Inherit reference */
	return 0;
err_prvkey:
	Dee_Decref(prvkey);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_mapitems_iterator_nextkey(RangeMapProxyKeysIterator *__restrict self) {
	int error;
	bool first;
	DREF DeeObject *prvkey, *maxkey, *value;
again:
	RangeMapProxyKeysIterator_LockAcquire(self);
	prvkey = self->rmpki_prvkey;
	maxkey = self->rmpki_maxkey;
	first  = self->rmpki_first;
	Dee_Incref(prvkey);
	Dee_Incref(maxkey);
	RangeMapProxyKeysIterator_LockRelease(self);
	error = DeeObject_CompareLo_S(prvkey, maxkey);
	Dee_Decref(maxkey);
	if unlikely(error < 0)
		goto err_prvkey;
	if (error) {
		maxkey = prvkey;
		if (first) {
			DREF DeeObject *copy;
			copy = DeeObject_Copy(prvkey);
			Dee_Decref(prvkey);
			if unlikely(!copy)
				goto err;
			prvkey = copy;
		}
		if unlikely(DeeObject_Inc(&prvkey))
			goto err_prvkey;
		RangeMapProxyKeysIterator_LockAcquire(self);
		if unlikely(self->rmpki_prvkey != maxkey) {
			RangeMapProxyKeysIterator_LockRelease(self);
			Dee_Decref(prvkey);
			goto again;
		}
		maxkey = self->rmpki_prvkey;
		self->rmpki_prvkey = prvkey;
		Dee_Incref(prvkey);
		value = self->rmpki_base.rmpii_value;
		Dee_Incref(value);
		RangeMapProxyKeysIterator_LockRelease(self);
		Dee_Decref(maxkey);
	} else {
		/* Move on to the next range. */
		DREF DeeObject *item[3];
		DREF DeeObject *tuple;
		Dee_Decref(prvkey);
		tuple = DeeObject_IterNext(self->rmpki_base.rmpii_base.rmpi_iter);
		if (!ITER_ISOK(tuple))
			return tuple;
		error = DeeSeq_Unpack(tuple, 3, item);
		Dee_Decref(tuple);
		if unlikely(error)
			goto err;
		Dee_Incref(item[0]);
		Dee_Incref(item[2]);
		RangeMapProxyKeysIterator_LockAcquire(self);
		prvkey = self->rmpki_prvkey;            /* Inherit reference */
		maxkey = self->rmpki_maxkey;            /* Inherit reference */
		value  = self->rmpki_base.rmpii_value;  /* Inherit reference */
		self->rmpki_prvkey           = item[0]; /* Inherit reference */
		self->rmpki_maxkey           = item[1]; /* Inherit reference */
		self->rmpki_base.rmpii_value = item[2]; /* Inherit reference */
		self->rmpki_first            = true;
		RangeMapProxyKeysIterator_LockRelease(self);
		Dee_Decref(value);
		Dee_Decref(prvkey);
		Dee_Decref(maxkey);
		prvkey = item[0];
		value  = item[2];
	}

	Dee_Decref(value);
	return prvkey;
err_prvkey:
	Dee_Decref(prvkey);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_mapitems_iterator_nextvalue(RangeMapProxyKeysIterator *__restrict self) {
	int error;
	bool first;
	DREF DeeObject *prvkey, *maxkey, *value;
again:
	RangeMapProxyKeysIterator_LockAcquire(self);
	prvkey = self->rmpki_prvkey;
	maxkey = self->rmpki_maxkey;
	first  = self->rmpki_first;
	Dee_Incref(prvkey);
	Dee_Incref(maxkey);
	RangeMapProxyKeysIterator_LockRelease(self);
	error = DeeObject_CompareLo_S(prvkey, maxkey);
	Dee_Decref(maxkey);
	if unlikely(error < 0)
		goto err_prvkey;
	if (error) {
		maxkey = prvkey;
		if (first) {
			DREF DeeObject *copy;
			copy = DeeObject_Copy(prvkey);
			Dee_Decref(prvkey);
			if unlikely(!copy)
				goto err;
			prvkey = copy;
		}
		if unlikely(DeeObject_Inc(&prvkey))
			goto err_prvkey;
		RangeMapProxyKeysIterator_LockAcquire(self);
		if unlikely(self->rmpki_prvkey != maxkey) {
			RangeMapProxyKeysIterator_LockRelease(self);
			Dee_Decref(prvkey);
			goto again;
		}
		maxkey = self->rmpki_prvkey;
		self->rmpki_prvkey = prvkey;
		Dee_Incref(prvkey);
		value = self->rmpki_base.rmpii_value;
		Dee_Incref(value);
		RangeMapProxyKeysIterator_LockRelease(self);
		Dee_Decref(maxkey);
	} else {
		/* Move on to the next range. */
		DREF DeeObject *item[3];
		DREF DeeObject *tuple;
		Dee_Decref(prvkey);
		tuple = DeeObject_IterNext(self->rmpki_base.rmpii_base.rmpi_iter);
		if (!ITER_ISOK(tuple))
			return tuple;
		error = DeeSeq_Unpack(tuple, 3, item);
		Dee_Decref(tuple);
		if unlikely(error)
			goto err;
		Dee_Incref(item[0]);
		Dee_Incref(item[2]);
		RangeMapProxyKeysIterator_LockAcquire(self);
		prvkey = self->rmpki_prvkey;            /* Inherit reference */
		maxkey = self->rmpki_maxkey;            /* Inherit reference */
		value  = self->rmpki_base.rmpii_value;  /* Inherit reference */
		self->rmpki_prvkey           = item[0]; /* Inherit reference */
		self->rmpki_maxkey           = item[1]; /* Inherit reference */
		self->rmpki_base.rmpii_value = item[2]; /* Inherit reference */
		self->rmpki_first            = true;
		RangeMapProxyKeysIterator_LockRelease(self);
		Dee_Decref(value);
		Dee_Decref(prvkey);
		Dee_Decref(maxkey);
		prvkey = item[0];
		value  = item[2];
	}

	Dee_Decref(prvkey);
	return value;
err_prvkey:
	Dee_Decref(prvkey);
err:
	return NULL;
}


PRIVATE struct type_iterator proxy_mapitems_iterator_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&proxy_mapitems_iterator_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_mapitems_iterator_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_mapitems_iterator_nextvalue,
	/* .tp_advance   = */ NULL, // TODO: (size_t (DCALL *)(DeeObject *__restrict, size_t))&proxy_mapitems_iterator_advance,
};

INTERN DeeTypeObject RangeMapKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapKeysIterator",
	/* .tp_doc      = */ NULL, /* Key... */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_keys_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_keys_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_keys_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_keys_iterator_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxyKeysIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_keys_iterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_keys_iterator_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_keys_iterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_keys_iterator_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_iterator_keys_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject RangeMapMapItemsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapMapItemsIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"), /* (Key, Value) */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapProxyIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_mapitems_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_mapitems_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_mapitems_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_mapitems_iterator_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxyKeysIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_mapitems_iterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_keys_iterator_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_mapitems_iterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &proxy_mapitems_iterator_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_iterator_mapitems_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject RangeMapAsMapIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RangeMapAsMapIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"), /* (Key, Value) */
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &RangeMapMapItemsIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_mapitems_iterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_mapitems_iterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_mapitems_iterator_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_mapitems_iterator_init,
				TYPE_FIXED_ALLOCATOR(RangeMapProxyKeysIterator)
			}
		},
		/* .tp_dtor        = */ NULL, /* Inherited... */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL /* Inherited... */
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &proxy_mapitems_iterator_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_iterator_asmap_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_RANGEMAP_C */
