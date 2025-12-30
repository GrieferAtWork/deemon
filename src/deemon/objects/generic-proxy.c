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

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/format.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/set.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>

#include "../runtime/runtime_error.h"

/**/
#include "generic-proxy.h"
/**/

#include <stddef.h> /* size_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

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
	tp_name = DeeType_GetName(Dee_TYPE(self));
	return err_invalid_argc(tp_name, argc, 1, 1);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__serialize(ProxyObject *__restrict self,
                         DeeSerial *__restrict writer,
                         Dee_seraddr_t addr) {
	return DeeSerial_PutObject(writer, addr + offsetof(ProxyObject, po_obj), self->po_obj);
}

INTERN NONNULL((1, 2)) void DCALL
generic_proxy__visit(ProxyObject *__restrict self,
                     Dee_visit_t proc, void *arg) {
	Dee_Visit(self->po_obj);
}

INTERN NONNULL((1)) void DCALL
generic_proxy__fini(ProxyObject *__restrict self) {
	Dee_Decref(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__getobj(ProxyObject *__restrict self) {
	return_reference(self->po_obj);
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
	tp_name = DeeType_GetName(Dee_TYPE(self));
	return err_invalid_argc(tp_name, argc, 2, 2);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2__serialize(ProxyObject2 *__restrict self,
                          DeeSerial *__restrict writer,
                          Dee_seraddr_t addr) {
	int result = DeeSerial_PutObject(writer, addr + offsetof(ProxyObject2, po_obj1), self->po_obj1);
	if likely(result == 0)
		result = DeeSerial_PutObject(writer, addr + offsetof(ProxyObject2, po_obj2), self->po_obj2);
	return result;
}


INTERN NONNULL((1, 2)) void DCALL
generic_proxy2__visit(ProxyObject2 *__restrict self,
                      Dee_visit_t proc, void *arg) {
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
	tp_name = DeeType_GetName(Dee_TYPE(self));
	return err_invalid_argc(tp_name, argc, 3, 3);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy3__serialize(ProxyObject3 *__restrict self,
                          DeeSerial *__restrict writer,
                          Dee_seraddr_t addr) {
	int result = DeeSerial_PutObject(writer, addr + offsetof(ProxyObject3, po_obj1), self->po_obj1);
	if likely(result == 0) {
		result = DeeSerial_PutObject(writer, addr + offsetof(ProxyObject3, po_obj2), self->po_obj2);
		if likely(result == 0)
			result = DeeSerial_PutObject(writer, addr + offsetof(ProxyObject3, po_obj3), self->po_obj3);
	}
	return result;
}

INTERN NONNULL((1, 2)) void DCALL
generic_proxy3__visit(ProxyObject3 *__restrict self, Dee_visit_t proc, void *arg) {
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



PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
serialize_copy_after_getsize(DeeObject *__restrict self) {
	size_t instance_size;
	DeeTypeObject *tp_self = Dee_TYPE(self);
	void (DCALL *tp_free)(void *);
	ASSERT(!(tp_self->tp_flags & TP_FVARIABLE));
	if ((tp_free = tp_self->tp_init.tp_alloc.tp_free) != NULL) {
#ifdef CONFIG_NO_OBJECT_SLABS
		Dee_Fatalf("Unable to determine tp_instance_size for %r", tp_self);
		return 0;
#else /* CONFIG_NO_OBJECT_SLABS */
		/* Figure out the slab size used by the base-class. */
		if (tp_self->tp_flags & TP_FGC) {
#define CHECK_ALLOCATOR(index, size)                      \
			if (tp_free == &DeeGCObject_SlabFree##size) { \
				instance_size = size * sizeof(void *);    \
			} else
			DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
			{
				Dee_Fatalf("Unable to determine tp_instance_size for %r", tp_self);
				return 0;
			}
		} else {
#define CHECK_ALLOCATOR(index, size)                    \
			if (tp_free == &DeeObject_SlabFree##size) { \
				instance_size = size * sizeof(void *);  \
			} else
			DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
			{
				Dee_Fatalf("Unable to determine tp_instance_size for %r", tp_self);
				return 0;
			}
		}
#endif /* !CONFIG_NO_OBJECT_SLABS */
	} else {
		instance_size = tp_self->tp_init.tp_alloc.tp_instance_size;
	}
	return instance_size;
}

PRIVATE NONNULL((1, 2)) void DCALL
serialize_copy_after(DeeObject *__restrict self,
                     struct Dee_serial *__restrict writer,
                     Dee_seraddr_t addr, size_t basic_size) {
	DeeObject *out;
	size_t instance_size = serialize_copy_after_getsize(self);
	ASSERTF(instance_size >= basic_size,
	        "Instance size %" PRFuSIZ " of %r is lower than "
	        /**/ "basic size %" PRFuSIZ " of proxy object",
	        instance_size, Dee_TYPE(self), basic_size);
	out = DeeSerial_Addr2Mem(writer, addr, DeeObject);
	memcpy((byte_t *)out + basic_size,
	       (byte_t const *)self + basic_size,
	       instance_size - basic_size);
}


/* Same as `generic_proxy__serialize()', but look at "tp_instance_size" (or
 * "tp_alloc") and memcpy all memory that is located after "ProxyObject". */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__serialize_and_copy(ProxyObject *__restrict self,
                                  struct Dee_serial *__restrict writer,
                                  Dee_seraddr_t addr) {
	serialize_copy_after(Dee_AsObject(self), writer, addr, sizeof(*self));
	return generic_proxy__serialize(self, writer, addr);
}

#ifndef CONFIG_NO_THREADS
INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__serialize_and_copy_atomic16(ProxyObject *__restrict self,
                                           struct Dee_serial *__restrict writer,
                                           Dee_seraddr_t addr) {
	uint16_t *out;
	uint16_t const *in;
	size_t instance_size = serialize_copy_after_getsize((DeeObject *)self);
	ASSERTF(instance_size >= sizeof(*self),
	        "Instance size %" PRFuSIZ " of %r is lower than "
	        /**/ "basic size %" PRFuSIZ " of proxy object",
	        instance_size, Dee_TYPE(self), sizeof(*self));
	instance_size -= sizeof(*self);
	ASSERTF((instance_size & 1) == 0,
	        "Remaining instance size %" PRFuSIZ " of %r is not a multiple of 2",
	        instance_size, Dee_TYPE(self));
	ASSERTF(instance_size != 0,
	        "Remaining instance size is 0; why is %r not using 'generic_proxy__serialize()'?",
	        Dee_TYPE(self));
	instance_size >>= 1;
	in = (uint16_t const *)(self + 1);
	out = DeeSerial_Addr2Mem(writer, addr + sizeof(*self), uint16_t);
	do {
		*out = atomic_read(in);
		++in;
		++out;
	} while (--instance_size);
	return generic_proxy__serialize(self, writer, addr);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__serialize_and_copy_atomic32(ProxyObject *__restrict self,
                                           struct Dee_serial *__restrict writer,
                                           Dee_seraddr_t addr) {
	uint32_t *out;
	uint32_t const *in;
	size_t instance_size = serialize_copy_after_getsize((DeeObject *)self);
	ASSERTF(instance_size >= sizeof(*self),
	        "Instance size %" PRFuSIZ " of %r is lower than "
	        /**/ "basic size %" PRFuSIZ " of proxy object",
	        instance_size, Dee_TYPE(self), sizeof(*self));
	instance_size -= sizeof(*self);
	ASSERTF((instance_size & 3) == 0,
	        "Remaining instance size %" PRFuSIZ " of %r is not a multiple of 4",
	        instance_size, Dee_TYPE(self));
	ASSERTF(instance_size != 0,
	        "Remaining instance size is 0; why is %r not using 'generic_proxy__serialize()'?",
	        Dee_TYPE(self));
	instance_size >>= 2;
	in = (uint32_t const *)(self + 1);
	out = DeeSerial_Addr2Mem(writer, addr + sizeof(*self), uint32_t);
	do {
		*out = atomic_read(in);
		++in;
		++out;
	} while (--instance_size);
	return generic_proxy__serialize(self, writer, addr);
}

#if __SIZEOF_POINTER__ >= 8
INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__serialize_and_copy_atomic64(ProxyObject *__restrict self,
                                           struct Dee_serial *__restrict writer,
                                           Dee_seraddr_t addr) {
	uint64_t *out;
	uint64_t const *in;
	size_t instance_size = serialize_copy_after_getsize((DeeObject *)self);
	ASSERTF(instance_size >= sizeof(*self),
	        "Instance size %" PRFuSIZ " of %r is lower than "
	        /**/ "basic size %" PRFuSIZ " of proxy object",
	        instance_size, Dee_TYPE(self), sizeof(*self));
	instance_size -= sizeof(*self);
	ASSERTF((instance_size & 7) == 0,
	        "Remaining instance size %" PRFuSIZ " of %r is not a multiple of 8",
	        instance_size, Dee_TYPE(self));
	ASSERTF(instance_size != 0,
	        "Remaining instance size is 0; why is %r not using 'generic_proxy__serialize()'?",
	        Dee_TYPE(self));
	instance_size >>= 3;
	in = (uint64_t const *)(self + 1);
	out = DeeSerial_Addr2Mem(writer, addr + sizeof(*self), uint64_t);
	do {
		*out = atomic_read(in);
		++in;
		++out;
	} while (--instance_size);
	return generic_proxy__serialize(self, writer, addr);
}
#endif /* __SIZEOF_POINTER__ >= 8 */
#endif /* !CONFIG_NO_THREADS */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__serialize_and_copy_xptr_atomic(ProxyObjectWithPointer *__restrict self,
                                              struct Dee_serial *__restrict writer,
                                              Dee_seraddr_t addr) {
	int result = generic_proxy__serialize((ProxyObject *)self, writer, addr);
	if likely(result == 0) {
		void *pointer = atomic_read(&self->po_ptr);
		result = DeeSerial_XPutPointer(writer, addr + offsetof(ProxyObjectWithPointer, po_ptr), pointer);
	}
	return result;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy2__serialize_and_copy(ProxyObject2 *__restrict self,
                                   struct Dee_serial *__restrict writer,
                                   Dee_seraddr_t addr) {
	serialize_copy_after(Dee_AsObject(self), writer, addr, sizeof(*self));
	return generic_proxy2__serialize(self, writer, addr);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy3__serialize_and_copy(ProxyObject3 *__restrict self,
                                   struct Dee_serial *__restrict writer,
                                   Dee_seraddr_t addr) {
	serialize_copy_after(Dee_AsObject(self), writer, addr, sizeof(*self));
	return generic_proxy3__serialize(self, writer, addr);
}



INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__bool(ProxyObject *__restrict self) {
	return DeeObject_Bool(self->po_obj);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy__iter_advance(ProxyObject *__restrict self, size_t step) {
	return DeeObject_IterAdvance(self->po_obj, step);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__iter_next(ProxyObject *__restrict self) {
	return DeeObject_IterNext(self->po_obj);
}


INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy__size_fast(ProxyObject *__restrict self) {
	return DeeObject_SizeFast(self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__hasattr_string_hash(ProxyObject *self, char const *attr, Dee_hash_t hash) {
	return DeeObject_HasAttrStringHash(self->po_obj, attr, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__boundattr_string_hash(ProxyObject *self, char const *attr, Dee_hash_t hash) {
	return DeeObject_BoundAttrStringHash(self->po_obj, attr, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__hasattr_string_len_hash(ProxyObject *self, char const *attr,
                                       size_t attrlen, Dee_hash_t hash) {
	return DeeObject_HasAttrStringLenHash(self->po_obj, attr, attrlen, hash);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__boundattr_string_len_hash(ProxyObject *self, char const *attr,
                                         size_t attrlen, Dee_hash_t hash) {
	return DeeObject_BoundAttrStringLenHash(self->po_obj, attr, attrlen, hash);
}



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

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_clear(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(seq_clear, self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__seq_contains(ProxyObject *self, DeeObject *item){
	return DeeObject_InvokeMethodHint(seq_contains, self->po_obj, item);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_boundfirst(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(seq_boundfirst, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_delfirst(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(seq_delfirst, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_boundlast(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(seq_boundlast, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) int DCALL
generic_proxy__seq_dellast(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(seq_dellast, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__set_operator_iter(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(set_operator_iter, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy__set_operator_size(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(set_operator_size, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__set_operator_sizeob(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(set_operator_sizeob, self->po_obj);
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

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_proxy__map_operator_contains(ProxyObject *self, DeeObject *key){
	return DeeObject_InvokeMethodHint(map_operator_contains, self->po_obj, key);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL
generic_proxy__map_operator_size(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(map_operator_size, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__map_operator_sizeob(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(map_operator_sizeob, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__map_iterkeys(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(map_iterkeys, self->po_obj);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_proxy__map_itervalues(ProxyObject *__restrict self){
	return DeeObject_InvokeMethodHint(map_itervalues, self->po_obj);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__map_remove(ProxyObject *self, DeeObject *key){
	return DeeObject_InvokeMethodHint(map_remove, self->po_obj, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
generic_proxy__map_removekeys(ProxyObject *self, DeeObject *keys){
	return DeeObject_InvokeMethodHint(map_removekeys, self->po_obj, keys);
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
		return Dee_COMPARE_NE;
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
		return Dee_COMPARE_NE;
	result = DeeObject_TryCompareEq(self->po_obj1, other->po_obj1);
	if (result == Dee_COMPARE_EQ)
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


/* Wrap the given object "self" as a Sequence, Set, or Mapping */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_obj__asseq(DeeObject *__restrict self) {
	return DeeSuper_New(&DeeSeq_Type, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_obj__asset(DeeObject *__restrict self) {
	return DeeSuper_New(&DeeSet_Type, self);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
generic_obj__asmap(DeeObject *__restrict self) {
	return DeeSuper_New(&DeeMapping_Type, self);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_GENERIC_PROXY_C */
