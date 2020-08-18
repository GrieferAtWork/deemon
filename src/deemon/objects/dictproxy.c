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
#ifndef GUARD_DEEMON_OBJECTS_DICTPROXY_C
#define GUARD_DEEMON_OBJECTS_DICTPROXY_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dict.h>
#include <deemon/gc.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>

#include "../runtime/runtime_error.h"

DECL_BEGIN

#define dummy      (&DeeDict_Dummy)

typedef struct {
	/* HINT: The basic algorithm and idea of iterating
	 *       a Dict is the same as for a set. */
	OBJECT_HEAD
	DeeDictObject    *di_dict; /* [1..1][const] The Dict being iterated. */
	struct dict_item *di_next; /* [?..1][MAYBE(in(di_dict->d_elem))][atomic]
	                            * The first candidate for the next item.
	                            * NOTE: Before being dereferenced, this pointer is checked
	                            *       for being located inside the Dict's element vector.
	                            *       In the event that it is located at its end, `ITER_DONE'
	                            *       is returned, though in the event that it is located
	                            *       outside, an error is thrown (`err_changed_sequence()'). */
} DictIterator;

#ifdef CONFIG_NO_THREADS
#define READ_ITEM(x)            ((x)->di_next)
#else /* CONFIG_NO_THREADS */
#define READ_ITEM(x) ATOMIC_READ((x)->di_next)
#endif /* !CONFIG_NO_THREADS */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dictiterator_next_key(DictIterator *__restrict self) {
	DREF DeeObject *result;
	struct dict_item *item, *end;
	DeeDictObject *Dict = self->di_dict;
	DeeDict_LockRead(Dict);
	end = Dict->d_elem + (Dict->d_mask + 1);
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->di_next);
#else /* CONFIG_NO_THREADS */
		struct dict_item *old_item;
		old_item = item = ATOMIC_READ(self->di_next);
#endif /* !CONFIG_NO_THREADS */
		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < Dict->d_elem)
			goto dict_has_changed;
		/* Search for the next non-empty item. */
		while (item != end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->di_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->di_next, old_item, item))
				continue;
#endif /* !CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->di_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->di_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	result = item->di_key;
	Dee_Incref(result);
	DeeDict_LockEndRead(Dict);
	return result;
dict_has_changed:
	DeeDict_LockEndRead(Dict);
	err_changed_sequence((DeeObject *)Dict);
	return NULL;
iter_exhausted:
	DeeDict_LockEndRead(Dict);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dictiterator_next_item(DictIterator *__restrict self) {
	DREF DeeObject *result, *result_key, *result_item;
	struct dict_item *item, *end;
	DeeDictObject *Dict = self->di_dict;
	DeeDict_LockRead(Dict);
	end = Dict->d_elem + (Dict->d_mask + 1);
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->di_next);
#else /* CONFIG_NO_THREADS */
		struct dict_item *old_item;
		old_item = item = ATOMIC_READ(self->di_next);
#endif /* !CONFIG_NO_THREADS */
		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < Dict->d_elem)
			goto dict_has_changed;
		/* Search for the next non-empty item. */
		while (item != end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->di_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->di_next, old_item, item))
				continue;
#endif /* !CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->di_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->di_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	result_key  = item->di_key;
	result_item = item->di_value;
	Dee_Incref(result_key);
	Dee_Incref(result_item);
	DeeDict_LockEndRead(Dict);
	result = DeeTuple_Pack(2, result_key, result_item);
	Dee_Decref(result_item);
	Dee_Decref(result_key);
	return result;
dict_has_changed:
	DeeDict_LockEndRead(Dict);
	err_changed_sequence((DeeObject *)Dict);
	return NULL;
iter_exhausted:
	DeeDict_LockEndRead(Dict);
	return ITER_DONE;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dictiterator_next_value(DictIterator *__restrict self) {
	DREF DeeObject *result;
	struct dict_item *item, *end;
	DeeDictObject *Dict = self->di_dict;
	DeeDict_LockRead(Dict);
	end = Dict->d_elem + (Dict->d_mask + 1);
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->di_next);
#else /* CONFIG_NO_THREADS */
		struct dict_item *old_item;
		old_item = item = ATOMIC_READ(self->di_next);
#endif /* !CONFIG_NO_THREADS */
		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < Dict->d_elem)
			goto dict_has_changed;
		/* Search for the next non-empty item. */
		while (item != end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->di_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->di_next, old_item, item))
				continue;
#endif /* !CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->di_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->di_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	result = item->di_value;
	Dee_Incref(result);
	DeeDict_LockEndRead(Dict);
	return result;
dict_has_changed:
	DeeDict_LockEndRead(Dict);
	err_changed_sequence((DeeObject *)Dict);
	return NULL;
iter_exhausted:
	DeeDict_LockEndRead(Dict);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dictiterator_bool(DictIterator *__restrict self) {
	struct dict_item *item = READ_ITEM(self);
	DeeDictObject *Dict    = self->di_dict;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Dict since we're not dereferencing anything. */
	return (item >= Dict->d_elem &&
	        item < Dict->d_elem + (Dict->d_mask + 1));
}

INTERN WUNUSED NONNULL((1)) int DCALL
dictiterator_init(DictIterator *__restrict self,
                  size_t argc, DeeObject *const *argv) {
	DeeDictObject *Dict;
	if (DeeArg_Unpack(argc, argv, "o:_DictIterator", &Dict) ||
	    DeeObject_AssertType((DeeObject *)Dict, &DeeDict_Type))
		return -1;
	self->di_dict = Dict;
	Dee_Incref(Dict);
#ifdef CONFIG_NO_THREADS
	self->di_next = Dict->d_elem;
#else /* CONFIG_NO_THREADS */
	self->di_next = ATOMIC_READ(Dict->d_elem);
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
dictiterator_ctor(DictIterator *__restrict self) {
	self->di_dict = (DeeDictObject *)DeeDict_New();
	if unlikely(!self->di_dict)
		return -1;
	self->di_next = self->di_dict->d_elem;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
dictiterator_copy(DictIterator *__restrict self,
                  DictIterator *__restrict other) {
	self->di_dict = other->di_dict;
	Dee_Incref(self->di_dict);
	self->di_next = READ_ITEM(other);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
dictiterator_fini(DictIterator *__restrict self) {
	Dee_Decref(self->di_dict);
}

PRIVATE NONNULL((1, 2)) void DCALL
dictiterator_visit(DictIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->di_dict);
}

INTDEF DeeTypeObject DictIterator_Type;
#define DEFINE_ITERATOR_COMPARE(name, op)                                 \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                 \
	name(DictIterator *self, DictIterator *other) {                       \
		if (DeeObject_AssertType((DeeObject *)other, &DictIterator_Type)) \
			goto err;                                                     \
		return_bool(READ_ITEM(self) op READ_ITEM(other));                 \
	err:                                                                  \
		return NULL;                                                      \
	}
DEFINE_ITERATOR_COMPARE(dictiterator_eq, ==)
DEFINE_ITERATOR_COMPARE(dictiterator_ne, !=)
DEFINE_ITERATOR_COMPARE(dictiterator_lo, <)
DEFINE_ITERATOR_COMPARE(dictiterator_le, <=)
DEFINE_ITERATOR_COMPARE(dictiterator_gr, >)
DEFINE_ITERATOR_COMPARE(dictiterator_ge, >=)
#undef DEFINE_ITERATOR_COMPARE

PRIVATE struct type_cmp dictiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dictiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dictiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dictiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dictiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dictiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dictiterator_ge
};


PRIVATE struct type_member dict_iterator_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(DictIterator,di_dict)),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&dictiterator_ctor,
				/* .tp_copy_ctor = */ (void *)&dictiterator_copy,
				/* .tp_deep_ctor = */ (void *)NULL, /* TODO */
				/* .tp_any_ctor  = */ (void *)&dictiterator_init,
				TYPE_FIXED_ALLOCATOR(DictIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&dictiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: bi-directional iterator support */
	/* .tp_cmp           = */ &dictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dictiterator_next_item,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dict_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_iter(DeeDictObject *__restrict self) {
	DREF DictIterator *result;
	result = DeeObject_MALLOC(DictIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DictIterator_Type);
	result->di_dict = self;
	Dee_Incref(self);
#ifdef CONFIG_NO_THREADS
	result->di_next = self->d_elem;
#else
	result->di_next = ATOMIC_READ(self->d_elem);
#endif
done:
	return (DREF DeeObject *)result;
}

typedef struct {
	OBJECT_HEAD
	DREF DeeDictObject *dp_dict; /* [1..1][const] Referenced Dict object. */
} DictProxy;

typedef struct {
	DictIterator    dpi_base;  /* The underlying Dict iterator. */
	DREF DictProxy *dpi_proxy; /* [1..1][const] The proxy that spawned this iterator. */
} DictProxyIterator;

PRIVATE struct type_member proxy_iterator_members[] = {
	TYPE_MEMBER_FIELD("seq", STRUCT_OBJECT, offsetof(DictProxyIterator, dpi_proxy)),
	TYPE_MEMBER_END
};

PRIVATE NONNULL((1)) void DCALL
dictproxyiterator_fini(DictProxyIterator *__restrict self) {
	Dee_Decref(self->dpi_proxy);
}

PRIVATE NONNULL((1, 2)) void DCALL
dictproxyiterator_visit(DictProxyIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->dpi_proxy);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
dictproxyiterator_copy(DictProxyIterator *__restrict self,
                       DictProxyIterator *__restrict other) {
	self->dpi_base.di_dict = other->dpi_base.di_dict;
	self->dpi_proxy        = other->dpi_proxy;
	Dee_Incref(self->dpi_base.di_dict);
	Dee_Incref(self->dpi_proxy);
#ifdef CONFIG_NO_THREADS
	self->dpi_base.di_next = other->dpi_base.di_next;
#else /* CONFIG_NO_THREADS */
	self->dpi_base.di_next = ATOMIC_READ(other->dpi_base.di_next);
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

PRIVATE DeeTypeObject DictProxyIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictProxyIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DictIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)NULL,
				/* .tp_copy_ctor = */ (void *)&dictproxyiterator_copy,
				/* .tp_deep_ctor = */ (void *)NULL,
				/* .tp_any_ctor  = */ (void *)NULL,
				TYPE_FIXED_ALLOCATOR(DictProxyIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dictproxyiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dictproxyiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ proxy_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTDEF DeeTypeObject DictKeysIterator_Type;
INTDEF DeeTypeObject DictItemsIterator_Type;
INTDEF DeeTypeObject DictValuesIterator_Type;

INTERN WUNUSED NONNULL((1)) int DCALL
dictproxyiterator_ctor(DictProxyIterator *__restrict self) {
	DeeTypeObject *proxy_type;
	proxy_type             = (DeeObject_InstanceOf((DeeObject *)self, &DictKeysIterator_Type) ? &DeeDictKeys_Type : DeeObject_InstanceOf((DeeObject *)self, &DictItemsIterator_Type) ? &DeeDictItems_Type : &DeeDictValues_Type);
	self->dpi_base.di_dict = (DeeDictObject *)DeeDict_New();
	if unlikely(!self->dpi_base.di_dict)
		goto err;
	self->dpi_proxy = DeeObject_MALLOC(DictProxy);
	if unlikely(!self->dpi_proxy)
		goto err_dict;
	DeeObject_Init(self->dpi_proxy, proxy_type);
	self->dpi_proxy->dp_dict = self->dpi_base.di_dict;
	Dee_Incref(self->dpi_base.di_dict);
	return 0;
err_dict:
	Dee_Decref(self->dpi_base.di_dict);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
dictproxyiterator_init(DictProxyIterator *__restrict self,
                       size_t argc, DeeObject *const *argv) {
	DictProxy *proxy;
	if (DeeArg_Unpack(argc, argv, "o:_DictIterator", &proxy))
		goto err;
	if (DeeObject_AssertType((DeeObject *)proxy,
	                         DeeObject_InstanceOf((DeeObject *)self, &DictKeysIterator_Type)
	                         ? &DeeDictKeys_Type
	                         : DeeObject_InstanceOf((DeeObject *)self, &DictItemsIterator_Type)
	                           ? &DeeDictItems_Type
	                           : &DeeDictValues_Type))
		goto err;
	self->dpi_proxy        = proxy;
	self->dpi_base.di_dict = proxy->dp_dict;
	Dee_Incref(proxy);
	Dee_Incref(proxy->dp_dict);
#ifdef CONFIG_NO_THREADS
	self->dpi_base.di_next = proxy->dp_dict->d_elem;
#else /* CONFIG_NO_THREADS */
	self->dpi_base.di_next = ATOMIC_READ(proxy->dp_dict->d_elem);
#endif /* !CONFIG_NO_THREADS */
	return 0;
err:
	return -1;
}

#define INIT_PROXY_ITERATOR_TYPE(tp_name, tp_iter_next)                                           \
	{                                                                                             \
		OBJECT_HEAD_INIT(&DeeType_Type),                                                          \
		/* .tp_name     = */tp_name,                                                              \
		/* .tp_doc      = */ NULL,                                                                \
		/* .tp_flags    = */ TP_FNORMAL,                                                          \
		/* .tp_weakrefs = */ 0,                                                                   \
		/* .tp_features = */ TF_NONE,                                                             \
		/* .tp_base     = */ &DictProxyIterator_Type,                                             \
		/* .tp_init = */ {                                                                        \
			{                                                                                     \
				/* .tp_alloc = */ {                                                               \
					/* .tp_ctor      = */ (void *)&dictproxyiterator_ctor,                        \
					/* .tp_copy_ctor = */ (void *)&dictproxyiterator_copy,                        \
					/* .tp_deep_ctor = */ (void *)NULL, /* TODO */                                \
					/* .tp_any_ctor  = */ (void *)&dictproxyiterator_init,                        \
					TYPE_FIXED_ALLOCATOR(DictProxyIterator)                                       \
				}                                                                                 \
			},                                                                                    \
			/* .tp_dtor        = */ NULL, /* INHERITED */                                         \
			/* .tp_assign      = */ NULL,                                                         \
			/* .tp_move_assign = */ NULL                                                          \
		},                                                                                        \
		/* .tp_cast = */ {                                                                        \
			/* .tp_str  = */ NULL,                                                                \
			/* .tp_repr = */ NULL,                                                                \
			/* .tp_bool = */ NULL                                                                 \
		},                                                                                        \
		/* .tp_call          = */ NULL,                                                           \
		/* .tp_visit         = */ NULL, /* INHERITED */                                           \
		/* .tp_gc            = */ NULL,                                                           \
		/* .tp_math          = */ NULL,                                                           \
		/* .tp_cmp           = */ NULL,                                                           \
		/* .tp_seq           = */ NULL,                                                           \
		/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))tp_iter_next, \
		/* .tp_attr          = */ NULL,                                                           \
		/* .tp_with          = */ NULL,                                                           \
		/* .tp_buffer        = */ NULL,                                                           \
		/* .tp_methods       = */ NULL,                                                           \
		/* .tp_getsets       = */ NULL,                                                           \
		/* .tp_members       = */ NULL,                                                           \
		/* .tp_class_methods = */ NULL,                                                           \
		/* .tp_class_getsets = */ NULL,                                                           \
		/* .tp_class_members = */ NULL                                                            \
	}

INTERN DeeTypeObject DictKeysIterator_Type   = INIT_PROXY_ITERATOR_TYPE("_DictKeysIterator", &dictiterator_next_key);
INTERN DeeTypeObject DictItemsIterator_Type  = INIT_PROXY_ITERATOR_TYPE("_DictItemsIterator", &dictiterator_next_item);
INTERN DeeTypeObject DictValuesIterator_Type = INIT_PROXY_ITERATOR_TYPE("_DictValuesIterator", &dictiterator_next_value);
#undef INIT_PROXY_ITERATOR_TYPE

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_newproxy_iterator(DictProxy *self,
                       DeeTypeObject *proxy_iterator_type) {
	DREF DictProxyIterator *result;
	result = DeeObject_MALLOC(DictProxyIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init((DeeObject *)result, proxy_iterator_type);
	result->dpi_base.di_dict = self->dp_dict;
	result->dpi_proxy        = self;
	Dee_Incref(self->dp_dict);
	Dee_Incref(self);
#ifdef CONFIG_NO_THREADS
	result->dpi_base.di_next = self->dp_dict->d_elem;
#else /* CONFIG_NO_THREADS */
	result->dpi_base.di_next = ATOMIC_READ(self->dp_dict->d_elem);
#endif /* !CONFIG_NO_THREADS */
done:
	return (DREF DeeObject *)result;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_newproxy(DeeDictObject *self,
              DeeTypeObject *proxy_type) {
	DREF DictProxy *result;
	ASSERT_OBJECT_TYPE(self, &DeeDict_Type);
	result = DeeObject_MALLOC(DictProxy);
	if unlikely(!result)
		return NULL;
	DeeObject_Init(result, proxy_type);
	result->dp_dict = self;
	Dee_Incref(self);
	return (DREF DeeObject *)result;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_ctor(DictProxy *__restrict self) {
	self->dp_dict = (DREF DeeDictObject *)DeeDict_New();
	if unlikely(!self->dp_dict)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_init(DictProxy *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeObject *Dict;
	if (DeeArg_Unpack(argc, argv, "o:_DictProxy", &Dict))
		goto err;
	if (DeeObject_AssertType(Dict, &DeeDict_Type))
		goto err;
	self->dp_dict = (DREF DeeDictObject *)Dict;
	Dee_Incref(Dict);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_copy(DictProxy *__restrict self,
           DictProxy *__restrict other) {
	self->dp_dict = other->dp_dict;
	Dee_Incref(self->dp_dict);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_deep(DictProxy *__restrict self,
           DictProxy *__restrict other) {
	self->dp_dict = (DREF DeeDictObject *)DeeObject_DeepCopy((DeeObject *)other->dp_dict);
	return self->dp_dict ? 0 : -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_fini(DictProxy *__restrict self) {
	Dee_Decref(self->dp_dict);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_bool(DictProxy *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->dp_dict->d_used != 0;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->dp_dict->d_used) != 0;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_visit(DictProxy *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->dp_dict);
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_size(DeeDictObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_contains(DeeDictObject *self, DeeObject *key);


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
proxy_size(DictProxy *__restrict self) {
	return dict_size(self->dp_dict);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
proxy_contains_key(DictProxy *self, DeeObject *key) {
	return dict_contains(self->dp_dict, key);
}

PRIVATE struct type_seq proxy_seq = {
	/* .tp_iter_self = */ NULL,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE struct type_member proxy_members[] = {
	TYPE_MEMBER_FIELD("__dict__", STRUCT_OBJECT, offsetof(DictProxy, dp_dict)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member proxy_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DictProxyIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member dict_keys_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DictKeysIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member dict_items_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DictItemsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member dict_values_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DictValuesIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_keys_iter(DictProxy *__restrict self) {
	return dict_newproxy_iterator(self, &DictKeysIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_items_iter(DictProxy *__restrict self) {
	return dict_newproxy_iterator(self, &DictItemsIterator_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_values_iter(DictProxy *__restrict self) {
	return dict_newproxy_iterator(self, &DictValuesIterator_Type);
}

PRIVATE struct type_seq dict_keys_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_keys_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&proxy_contains_key,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE struct type_seq dict_items_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_items_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE struct type_seq dict_values_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_values_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};


INTDEF struct keyword seq_byhash_kwlist[];

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_keys_byhash(DictProxy *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_byhash_kwlist, "o:byhash", &template_))
		goto err;
	return DeeDict_ByHash((DeeObject *)self->dp_dict,
	                      DeeObject_Hash(template_),
	                      true);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_items_byhash(DictProxy *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_byhash_kwlist, "o:byhash", &template_))
		goto err;
	return DeeDict_ByHash((DeeObject *)self->dp_dict,
	                      DeeObject_Hash(template_),
	                      false);
err:
	return NULL;
}


DOC_REF(map_byhash_doc);

PRIVATE struct type_method dict_keys_methods[] = {
	{ "byhash",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_keys_byhash,
	  DOC_GET(map_byhash_doc),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};

PRIVATE struct type_method dict_items_methods[] = {
	{ "byhash",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_items_byhash,
	  DOC_GET(map_byhash_doc),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};



PUBLIC DeeTypeObject DeeDictProxy_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictProxy",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&proxy_ctor,
				/* .tp_copy_ctor = */ (void *)&proxy_copy,
				/* .tp_deep_ctor = */ (void *)&proxy_deep,
				/* .tp_any_ctor  = */ (void *)&proxy_init,
				TYPE_FIXED_ALLOCATOR(DictProxy)
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &proxy_seq,
	/* .tp_iter_next     = */ NULL,
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

PUBLIC DeeTypeObject DeeDictKeys_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictKeys",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeDictProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&proxy_ctor,
				/* .tp_copy_ctor = */ (void *)&proxy_copy,
				/* .tp_deep_ctor = */ (void *)&proxy_deep,
				/* .tp_any_ctor  = */ (void *)&proxy_init,
				TYPE_FIXED_ALLOCATOR(DictProxy)
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dict_keys_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ dict_keys_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dict_keys_class_members
};

PUBLIC DeeTypeObject DeeDictItems_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictItems",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeDictProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&proxy_ctor,
				/* .tp_copy_ctor = */ (void *)&proxy_copy,
				/* .tp_deep_ctor = */ (void *)&proxy_deep,
				/* .tp_any_ctor  = */ (void *)&proxy_init,
				TYPE_FIXED_ALLOCATOR(DictProxy)
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dict_items_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ dict_items_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dict_items_class_members
};

PUBLIC DeeTypeObject DeeDictValues_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictValues",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeDictProxy_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&proxy_ctor,
				/* .tp_copy_ctor = */ (void *)&proxy_copy,
				/* .tp_deep_ctor = */ (void *)&proxy_deep,
				/* .tp_any_ctor  = */ (void *)&proxy_init,
				TYPE_FIXED_ALLOCATOR(DictProxy)
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
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dict_values_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dict_values_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_DICTPROXY_C */
