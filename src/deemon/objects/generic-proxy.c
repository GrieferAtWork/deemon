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
#ifndef GUARD_DEEMON_OBJECTS_GENERIC_PROXY_C
#define GUARD_DEEMON_OBJECTS_GENERIC_PROXY_C 1

#include <deemon/api.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>

#include "../runtime/runtime_error.h"
#include "seq/default-api.h"

/**/
#include "generic-proxy.h"

DECL_BEGIN

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__copy_alias(ProxyObject *__restrict self,
                         ProxyObject *__restrict other) {
	Dee_Incref(other->po_obj);
	self->po_obj = other->po_obj;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__copy_recursive(ProxyObject *__restrict self,
                             ProxyObject *__restrict other) {
	self->po_obj = DeeObject_Copy(other->po_obj);
	if unlikely(!self->po_obj)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__deepcopy(ProxyObject *__restrict self,
                       ProxyObject *__restrict other) {
	self->po_obj = DeeObject_DeepCopy(other->po_obj);
	if unlikely(!self->po_obj)
		goto err;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__init(ProxyObject *__restrict self,
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
generic_proxy__visit(ProxyObject *__restrict self,
                    dvisit_t proc, void *arg) {
	Dee_Visit(self->po_obj);
}

INTERN NONNULL((1)) void DCALL
generic_proxy__fini(ProxyObject *__restrict self) {
	Dee_Decref(self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2__copy_alias12(ProxyObject2 *__restrict self,
                            ProxyObject2 *__restrict other) {
	Dee_Incref(other->po_obj1);
	self->po_obj1 = other->po_obj1;
	Dee_Incref(other->po_obj2);
	self->po_obj2 = other->po_obj2;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2__copy_recursive1_alias2(ProxyObject2 *__restrict self,
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
generic_proxy2__deepcopy(ProxyObject2 *__restrict self,
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
generic_proxy2__init(ProxyObject2 *__restrict self,
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
generic_proxy2__visit(ProxyObject2 *__restrict self,
                     dvisit_t proc, void *arg) {
	Dee_Visit(self->po_obj1);
	Dee_Visit(self->po_obj2);
}

INTERN NONNULL((1)) void DCALL
generic_proxy2__fini(ProxyObject2 *__restrict self) {
	Dee_Decref(self->po_obj1);
	Dee_Decref(self->po_obj2);
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy3__copy_alias123(ProxyObject3 *__restrict self,
                             ProxyObject3 *__restrict other) {
	Dee_Incref(other->po_obj1);
	self->po_obj1 = other->po_obj1;
	Dee_Incref(other->po_obj2);
	self->po_obj2 = other->po_obj2;
	Dee_Incref(other->po_obj3);
	self->po_obj3 = other->po_obj3;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy3__deepcopy(ProxyObject3 *__restrict self,
                        ProxyObject3 *__restrict other) {
	self->po_obj1 = DeeObject_DeepCopy(other->po_obj1);
	if unlikely(!self->po_obj1)
		goto err;
	self->po_obj2 = DeeObject_DeepCopy(other->po_obj2);
	if unlikely(!self->po_obj2)
		goto err_obj1;
	self->po_obj3 = DeeObject_DeepCopy(other->po_obj3);
	if unlikely(!self->po_obj3)
		goto err_obj1_obj2;
	return 0;
err_obj1_obj2:
	Dee_Decref(self->po_obj2);
err_obj1:
	Dee_Decref(self->po_obj1);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy3__init(ProxyObject3 *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	char const *tp_name;
	if likely(argc == 3) {
		self->po_obj1 = argv[0];
		self->po_obj2 = argv[1];
		self->po_obj3 = argv[2];
		Dee_Incref(self->po_obj1);
		Dee_Incref(self->po_obj2);
		Dee_Incref(self->po_obj3);
		return 0;
	}
	tp_name = Dee_TYPE(self)->tp_name;
	if unlikely(!tp_name)
		tp_name = "<unnamed type>";
	return err_invalid_argc(tp_name, argc, 3, 3);
}

INTERN NONNULL((1, 2)) void DCALL
generic_proxy3__visit(ProxyObject3 *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->po_obj1);
	Dee_Visit(self->po_obj2);
	Dee_Visit(self->po_obj3);
}

INTERN NONNULL((1)) void DCALL
generic_proxy3__fini(ProxyObject3 *__restrict self) {
	Dee_Decref(self->po_obj1);
	Dee_Decref(self->po_obj2);
	Dee_Decref(self->po_obj3);
}



INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__bool(ProxyObject *__restrict self) {
	return DeeObject_Bool(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy__iter_advance(ProxyObject *__restrict self, size_t step) {
	return DeeObject_IterAdvance(self->po_obj, step);
}


INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy__size_fast(ProxyObject *__restrict self) {
	return DeeObject_SizeFast(self->po_obj);
}

#ifndef CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS
INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy__size(ProxyObject *__restrict self) {
	return DeeObject_Size(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__sizeob(ProxyObject *__restrict self) {
	return DeeObject_SizeOb(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__iterkeys(ProxyObject *self) {
	return DeeObject_IterKeys(self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy__getitem(ProxyObject *self, DeeObject *key_or_index) {
	return DeeObject_GetItem(self->po_obj, key_or_index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__delitem(ProxyObject *self, DeeObject *key_or_index) {
	return DeeObject_DelItem(self->po_obj, key_or_index);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_proxy__setitem(ProxyObject *self, DeeObject *key_or_index, DeeObject *value) {
	return DeeObject_SetItem(self->po_obj, key_or_index, value);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_proxy__delrange(ProxyObject *self, DeeObject *start, DeeObject *end) {
	return DeeObject_DelRange(self->po_obj, start, end);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
generic_proxy__setrange(ProxyObject *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	return DeeObject_SetRange(self->po_obj, start, end, values);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__hasitem(ProxyObject *self, DeeObject *key_or_index) {
	return DeeObject_HasItem(self->po_obj, key_or_index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__bounditem(ProxyObject *self, DeeObject *key_or_index) {
	return DeeObject_BoundItem(self->po_obj, key_or_index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__delitem_index(ProxyObject *__restrict self, size_t index) {
	return DeeObject_DelItemIndex(self->po_obj, index);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
generic_proxy__setitem_index(ProxyObject *__restrict self, size_t index, DeeObject *value) {
	return DeeObject_SetItemIndex(self->po_obj, index, value);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__delrange_index(ProxyObject *self, Dee_ssize_t start, Dee_ssize_t end) {
	return DeeObject_DelRangeIndex(self->po_obj, start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__delrange_index_n(ProxyObject *self, Dee_ssize_t start) {
	return DeeObject_DelRangeIndexN(self->po_obj, start);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
generic_proxy__setrange_index(ProxyObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	return DeeObject_SetRangeIndex(self->po_obj, start, end, values);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
generic_proxy__setrange_index_n(ProxyObject *self, Dee_ssize_t start, DeeObject *values) {
	return DeeObject_SetRangeIndexN(self->po_obj, start, values);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__hasitem_index(ProxyObject *__restrict self, size_t index) {
	return DeeObject_HasItemIndex(self->po_obj, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__bounditem_index(ProxyObject *__restrict self, size_t index) {
	return DeeObject_BoundItemIndex(self->po_obj, index);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy__contains(ProxyObject *self, DeeObject *item) {
	return DeeObject_Contains(self->po_obj, item);
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
generic_proxy__getrange(ProxyObject *self, DeeObject *start, DeeObject *end) {
	return DeeObject_GetRange(self->po_obj, start, end);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy__getitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash) {
	return DeeObject_GetItemStringHash(self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__delitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash) {
	return DeeObject_DelItemStringHash(self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
generic_proxy__setitem_string_hash(ProxyObject *self, char const *key,
                                  Dee_hash_t hash, DeeObject *value) {
	return DeeObject_SetItemStringHash(self->po_obj, key, hash, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__hasitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash) {
	return DeeObject_HasItemStringHash(self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__bounditem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash) {
	return DeeObject_BoundItemStringHash(self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy__getitem_string_len_hash(ProxyObject *self, char const *key,
                                      size_t keylen, Dee_hash_t hash) {
	return DeeObject_GetItemStringLenHash(self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__delitem_string_len_hash(ProxyObject *self, char const *key,
                                      size_t keylen, Dee_hash_t hash) {
	return DeeObject_DelItemStringLenHash(self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
generic_proxy__setitem_string_len_hash(ProxyObject *self, char const *key,
                                      size_t keylen, Dee_hash_t hash, DeeObject *value) {
	return DeeObject_SetItemStringLenHash(self->po_obj, key, keylen, hash, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__hasitem_string_len_hash(ProxyObject *self, char const *key,
                                      size_t keylen, Dee_hash_t hash) {
	return DeeObject_HasItemStringLenHash(self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__bounditem_string_len_hash(ProxyObject *self, char const *key,
                                        size_t keylen, Dee_hash_t hash) {
	return DeeObject_BoundItemStringLenHash(self->po_obj, key, keylen, hash);
}
#endif /* !CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */


/*[[[deemon
import * from deemon;
import printProxyObjectMethodHintWrapper from "..method-hints.method-hints";
for (local line: File.open("generic-proxy.h", "rb")) {
	local name;
	try {
		name = line.rescanf(r'\s*printProxyObjectMethodHintWrapper\s*\(\s*"([^"]+)"\)')...;
	} catch (...) {
		continue;
	}
	printProxyObjectMethodHintWrapper(name, impl: true);
	print;
}
]]]*/
INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_operator_bool(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(seq_operator_bool, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy__seq_operator_size(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(seq_operator_size, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__seq_operator_sizeob(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(seq_operator_sizeob, self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__seq_operator_bounditem(ProxyObject *self, DeeObject *index){
	return DeeObject_InvokeMethodHint(seq_operator_bounditem, self->po_obj, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_operator_bounditem_index(ProxyObject *__restrict self, size_t index){
	return DeeObject_InvokeMethodHint(seq_operator_bounditem_index, self->po_obj, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__seq_operator_hasitem(ProxyObject *self, DeeObject *index){
	return DeeObject_InvokeMethodHint(seq_operator_hasitem, self->po_obj, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_operator_hasitem_index(ProxyObject *__restrict self, size_t index){
	return DeeObject_InvokeMethodHint(seq_operator_hasitem_index, self->po_obj, index);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__seq_operator_delitem(ProxyObject *self, DeeObject *index){
	return DeeObject_InvokeMethodHint(seq_operator_delitem, self->po_obj, index);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_proxy__seq_operator_delrange(ProxyObject *self, DeeObject *start, DeeObject *end){
	return DeeObject_InvokeMethodHint(seq_operator_delrange, self->po_obj, start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_operator_delitem_index(ProxyObject *__restrict self, size_t index){
	return DeeObject_InvokeMethodHint(seq_operator_delitem_index, self->po_obj, index);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_operator_delrange_index(ProxyObject *self, Dee_ssize_t start, Dee_ssize_t end){
	return DeeObject_InvokeMethodHint(seq_operator_delrange_index, self->po_obj, start, end);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_operator_delrange_index_n(ProxyObject *self, Dee_ssize_t start){
	return DeeObject_InvokeMethodHint(seq_operator_delrange_index_n, self->po_obj, start);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_proxy__seq_operator_setitem(ProxyObject *self, DeeObject *index, DeeObject *value){
	return DeeObject_InvokeMethodHint(seq_operator_setitem, self->po_obj, index, value);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
generic_proxy__seq_operator_setitem_index(ProxyObject *self, size_t index, DeeObject *value){
	return DeeObject_InvokeMethodHint(seq_operator_setitem_index, self->po_obj, index, value);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
generic_proxy__seq_operator_setrange(ProxyObject *self, DeeObject *start, DeeObject *end, DeeObject *items){
	return DeeObject_InvokeMethodHint(seq_operator_setrange, self->po_obj, start, end, items);
}

INTERN WUNUSED NONNULL((1, 4)) int DCALL
generic_proxy__seq_operator_setrange_index(ProxyObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *items){
	return DeeObject_InvokeMethodHint(seq_operator_setrange_index, self->po_obj, start, end, items);
}

INTERN WUNUSED NONNULL((1, 3)) int DCALL
generic_proxy__seq_operator_setrange_index_n(ProxyObject *self, Dee_ssize_t start, DeeObject *items){
	return DeeObject_InvokeMethodHint(seq_operator_setrange_index_n, self->po_obj, start, items);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy__seq_operator_contains(ProxyObject *self, DeeObject *item){
	return DeeObject_InvokeMethodHint(seq_operator_contains, self->po_obj, item);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy__map_operator_getitem(ProxyObject *self, DeeObject *key){
	return DeeObject_InvokeMethodHint(map_operator_getitem, self->po_obj, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__map_operator_delitem(ProxyObject *self, DeeObject *key){
	return DeeObject_InvokeMethodHint(map_operator_delitem, self->po_obj, key);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
generic_proxy__map_operator_setitem(ProxyObject *self, DeeObject *key, DeeObject *value){
	return DeeObject_InvokeMethodHint(map_operator_setitem, self->po_obj, key, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__map_operator_hasitem(ProxyObject *self, DeeObject *key){
	return DeeObject_InvokeMethodHint(map_operator_hasitem, self->po_obj, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__map_operator_bounditem(ProxyObject *self, DeeObject *key){
	return DeeObject_InvokeMethodHint(map_operator_bounditem, self->po_obj, key);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy__map_operator_getitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash){
	return DeeObject_InvokeMethodHint(map_operator_getitem_string_hash, self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__map_operator_delitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash){
	return DeeObject_InvokeMethodHint(map_operator_delitem_string_hash, self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
generic_proxy__map_operator_setitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash, DeeObject *value){
	return DeeObject_InvokeMethodHint(map_operator_setitem_string_hash, self->po_obj, key, hash, value);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__map_operator_hasitem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash){
	return DeeObject_InvokeMethodHint(map_operator_hasitem_string_hash, self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__map_operator_bounditem_string_hash(ProxyObject *self, char const *key, Dee_hash_t hash){
	return DeeObject_InvokeMethodHint(map_operator_bounditem_string_hash, self->po_obj, key, hash);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__map_operator_getitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash){
	return DeeObject_InvokeMethodHint(map_operator_getitem_string_len_hash, self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__map_operator_delitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash){
	return DeeObject_InvokeMethodHint(map_operator_delitem_string_len_hash, self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1, 5)) int DCALL
generic_proxy__map_operator_setitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value){
	return DeeObject_InvokeMethodHint(map_operator_setitem_string_len_hash, self->po_obj, key, keylen, hash, value);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__map_operator_hasitem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash){
	return DeeObject_InvokeMethodHint(map_operator_hasitem_string_len_hash, self->po_obj, key, keylen, hash);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__map_operator_bounditem_string_len_hash(ProxyObject *self, char const *key, size_t keylen, Dee_hash_t hash){
	return DeeObject_InvokeMethodHint(map_operator_bounditem_string_len_hash, self->po_obj, key, keylen, hash);
}
/*[[[end]]]*/



/*
INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
generic_proxy__hash_id(ProxyObject *self) {
	return DeeObject_HashGeneric(self->po_obj);
}*/


INTERN struct type_cmp generic_proxy__cmp_recursive = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&generic_proxy__hash_recursive,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy__compare_eq_recursive,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy__compare_recursive,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy__trycompare_eq_recursive,
};

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
generic_proxy__hash_recursive(ProxyObject *__restrict self) {
	return DeeObject_Hash(self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__trycompare_eq_recursive(ProxyObject *self, ProxyObject *other) {
	if unlikely(!DeeObject_InstanceOf(other, Dee_TYPE(self)))
		return 1;
	return DeeObject_TryCompareEq(self->po_obj, other->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__compare_eq_recursive(ProxyObject *self, ProxyObject *other) {
	if unlikely(DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return DeeObject_CompareEq(self->po_obj, other->po_obj);
err:
	return Dee_COMPARE_ERR;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__compare_recursive(ProxyObject *self, ProxyObject *other) {
	if unlikely(DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	return DeeObject_Compare(self->po_obj, other->po_obj);
err:
	return Dee_COMPARE_ERR;
}



INTERN struct type_cmp generic_proxy2__cmp_recursive_ordered = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&generic_proxy2__hash_recursive_ordered,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy2__compare_eq_recursive,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *self, DeeObject *))&generic_proxy2__trycompare_eq_recursive,
};

INTERN WUNUSED NONNULL((1)) Dee_hash_t DCALL
generic_proxy2__hash_recursive_ordered(ProxyObject2 *__restrict self) {
	return Dee_HashCombine(DeeObject_Hash(self->po_obj1),
	                       DeeObject_Hash(self->po_obj2));
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2__trycompare_eq_recursive(ProxyObject2 *self,
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
generic_proxy2__compare_eq_recursive(ProxyObject2 *self,
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
