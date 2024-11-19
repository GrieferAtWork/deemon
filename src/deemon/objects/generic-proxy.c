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
#ifndef GUARD_DEEMON_OBJECTS_GENERIC_PROXY_C
#define GUARD_DEEMON_OBJECTS_GENERIC_PROXY_C 1

#include <deemon/api.h>
#include <deemon/object.h>

#include "../runtime/runtime_error.h"

/**/
#include "generic-proxy.h"

DECL_BEGIN

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_copy_alias(ProxyObject *__restrict self,
                         ProxyObject *__restrict other) {
	Dee_Incref(other->po_obj);
	self->po_obj = other->po_obj;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_copy_recursive(ProxyObject *__restrict self,
                             ProxyObject *__restrict other) {
	self->po_obj = DeeObject_Copy(other->po_obj);
	if unlikely(!self->po_obj)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_deepcopy(ProxyObject *__restrict self,
                       ProxyObject *__restrict other) {
	self->po_obj = DeeObject_DeepCopy(other->po_obj);
	if unlikely(!self->po_obj)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy_init(ProxyObject *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	char const *tp_name;
	if likely(argc == 1) {
		self->po_obj = argv[0];
		Dee_Incref(self->po_obj);
		return 0;
	}
	tp_name = Dee_TYPE(self)->tp_name;
	if unlikely(!tp_name)
		tp_name = "<unnamed type>";
	return err_invalid_argc(tp_name, argc, 1, 1);
}

INTERN NONNULL((1, 2)) void DCALL
generic_proxy_visit(ProxyObject *__restrict self,
                    dvisit_t proc, void *arg) {
	Dee_Visit(self->po_obj);
}

INTERN NONNULL((1)) void DCALL
generic_proxy_fini(ProxyObject *__restrict self) {
	Dee_Decref(self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2_copy_alias12(ProxyObject2 *__restrict self,
                            ProxyObject2 *__restrict other) {
	Dee_Incref(other->po_obj1);
	self->po_obj1 = other->po_obj1;
	Dee_Incref(other->po_obj2);
	self->po_obj2 = other->po_obj2;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2_copy_recursive1_alias2(ProxyObject2 *__restrict self,
                                      ProxyObject2 *__restrict other) {
	self->po_obj1 = DeeObject_Copy(other->po_obj1);
	if unlikely(!self->po_obj1)
		goto err;
	Dee_Incref(other->po_obj2);
	self->po_obj2 = other->po_obj2;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2_deepcopy(ProxyObject2 *__restrict self,
                        ProxyObject2 *__restrict other) {
	self->po_obj1 = DeeObject_DeepCopy(other->po_obj1);
	if unlikely(!self->po_obj1)
		goto err;
	self->po_obj2 = DeeObject_DeepCopy(other->po_obj2);
	if unlikely(!self->po_obj2)
		goto err_obj1;
	return 0;
err_obj1:
	Dee_Decref(self->po_obj1);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy2_init(ProxyObject2 *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	char const *tp_name;
	if likely(argc == 2) {
		self->po_obj1 = argv[0];
		self->po_obj2 = argv[1];
		Dee_Incref(self->po_obj1);
		Dee_Incref(self->po_obj2);
		return 0;
	}
	tp_name = Dee_TYPE(self)->tp_name;
	if unlikely(!tp_name)
		tp_name = "<unnamed type>";
	return err_invalid_argc(tp_name, argc, 2, 2);
}

INTERN NONNULL((1, 2)) void DCALL
generic_proxy2_visit(ProxyObject2 *__restrict self,
                     dvisit_t proc, void *arg) {
	Dee_Visit(self->po_obj1);
	Dee_Visit(self->po_obj2);
}

INTERN NONNULL((1)) void DCALL
generic_proxy2_fini(ProxyObject2 *__restrict self) {
	Dee_Decref(self->po_obj1);
	Dee_Decref(self->po_obj2);
}



INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy_bool(ProxyObject *__restrict self) {
	return DeeObject_Bool(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy_iter_advance(ProxyObject *__restrict self, size_t step) {
	return DeeObject_IterAdvance(self->po_obj, step);
}


INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy_size(ProxyObject *__restrict self) {
	return DeeObject_Size(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy_size_fast(ProxyObject *__restrict self) {
	return DeeObject_SizeFast(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy_sizeob(ProxyObject *__restrict self) {
	return DeeObject_SizeOb(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy_iterkeys(ProxyObject *self) {
	return DeeObject_IterKeys(self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy_getitem(ProxyObject *self, DeeObject *key_or_index) {
	return DeeObject_GetItem(self->po_obj, key_or_index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_delitem(ProxyObject *self, DeeObject *key_or_index) {
	return DeeObject_DelItem(self->po_obj, key_or_index);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_proxy_setitem(ProxyObject *self, DeeObject *key_or_index, DeeObject *value) {
	return DeeObject_SetItem(self->po_obj, key_or_index, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_hasitem(ProxyObject *self, DeeObject *key_or_index) {
	return DeeObject_HasItem(self->po_obj, key_or_index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_bounditem(ProxyObject *self, DeeObject *key_or_index) {
	return DeeObject_BoundItem(self->po_obj, key_or_index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy_hasitem_index(ProxyObject *__restrict self, size_t index) {
	return DeeObject_HasItemIndex(self->po_obj, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy_bounditem_index(ProxyObject *__restrict self, size_t index) {
	return DeeObject_BoundItemIndex(self->po_obj, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy_contains(ProxyObject *self, DeeObject *item) {
	return DeeObject_Contains(self->po_obj, item);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy_getitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash) {
	return DeeObject_GetItemStringHash(self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_delitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash) {
	return DeeObject_DelItemStringHash(self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
generic_proxy_setitem_string_hash(ProxyObject *self, char const *key,
                           Dee_hash_t hash, DeeObject *value) {
	return DeeObject_SetItemStringHash(self->po_obj, key, hash, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_hasitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash) {
	return DeeObject_HasItemStringHash(self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_bounditem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash) {
	return DeeObject_BoundItemStringHash(self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy_getitem_string_len_hash(ProxyObject *self, char const *key,
                                      size_t keylen, Dee_hash_t hash) {
	return DeeObject_GetItemStringLenHash(self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_delitem_string_len_hash(ProxyObject *self, char const *key,
                                      size_t keylen, Dee_hash_t hash) {
	return DeeObject_DelItemStringLenHash(self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
generic_proxy_setitem_string_len_hash(ProxyObject *self, char const *key,
                                      size_t keylen, Dee_hash_t hash, DeeObject *value) {
	return DeeObject_SetItemStringLenHash(self->po_obj, key, keylen, hash, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_hasitem_string_len_hash(ProxyObject *self, char const *key,
                                      size_t keylen, Dee_hash_t hash) {
	return DeeObject_HasItemStringLenHash(self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_bounditem_string_len_hash(ProxyObject *self, char const *key,
                                        size_t keylen, Dee_hash_t hash) {
	return DeeObject_BoundItemStringLenHash(self->po_obj, key, keylen, hash);
}

/*
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
generic_proxy_hash_id(ProxyObject *self) {
	return DeeObject_HashGeneric(self->po_obj);
}*/


INTERN struct type_cmp generic_proxy_cmp_recursive= {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&generic_proxy_hash_recursive,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy_compare_eq_recursive,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy_compare_recursive,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy_trycompare_eq_recursive,
};

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
generic_proxy_hash_recursive(ProxyObject *__restrict self) {
	return DeeObject_Hash(self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_trycompare_eq_recursive(ProxyObject *self, ProxyObject *other) {
	if unlikely(!DeeObject_InstanceOf(other, Dee_TYPE(self)))
		return 1;
	return DeeObject_TryCompareEq(self->po_obj, other->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_compare_eq_recursive(ProxyObject *self, ProxyObject *other) {
	if unlikely(DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return DeeObject_CompareEq(self->po_obj, other->po_obj);
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy_compare_recursive(ProxyObject *self, ProxyObject *other) {
	if unlikely(DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return DeeObject_Compare(self->po_obj, other->po_obj);
err:
	return Dee_COMPARE_ERR;
}



INTERN struct type_cmp generic_proxy2_cmp_recursive_ordered = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&generic_proxy2_hash_recursive_ordered,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy2_compare_eq_recursive,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy2_trycompare_eq_recursive,
};

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
generic_proxy2_hash_recursive_ordered(ProxyObject2 *__restrict self) {
	return Dee_HashCombine(DeeObject_Hash(self->po_obj1),
	                       DeeObject_Hash(self->po_obj2));
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2_trycompare_eq_recursive(ProxyObject2 *self,
                                       ProxyObject2 *other) {
	int result;
	if (!DeeObject_InstanceOf(other, Dee_TYPE(self)))
		return 1;
	result = DeeObject_TryCompareEq(self->po_obj1, other->po_obj1);
	if (result == 0)
		result = DeeObject_TryCompareEq(self->po_obj2, other->po_obj2);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2_compare_eq_recursive(ProxyObject2 *self,
                                    ProxyObject2 *other) {
	int result;
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	result = DeeObject_CompareEq(self->po_obj1, other->po_obj1);
	if (result == 0)
		result = DeeObject_CompareEq(self->po_obj2, other->po_obj2);
	return result;
err:
	return Dee_COMPARE_ERR;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_GENERIC_PROXY_C */
