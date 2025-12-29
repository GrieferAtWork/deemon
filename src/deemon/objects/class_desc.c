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
#ifndef GUARD_DEEMON_OBJECTS_CLASS_DESC_C
#define GUARD_DEEMON_OBJECTS_CLASS_DESC_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/instancemethod.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/property.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpy(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/typecore.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
/**/

#include <stddef.h> /* offsetof, size_t */
#include <stdint.h> /* uint16_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

typedef DeeClassDescriptorObject ClassDescriptor;

INTERN struct class_operator empty_class_operators[] = {
	{
		/* .co_name = */ (Dee_operator_t)-1,
		/* .co_addr = */ 0
	}
};

INTERN struct class_attribute empty_class_attributes[] = {
	{
		/* .ca_name = */ NULL,
		/* .ca_hash = */ 0,
		/* .ca_doc  = */ NULL,
		/* .ca_addr = */ 0,
		/* .ca_flag = */ CLASS_ATTRIBUTE_FNORMAL
	}
};

typedef struct {
	/* A mapping-like {(string | int, int)...} object used for mapping
	 * operator names to their respective class instance table slots. */
	PROXY_OBJECT_HEAD_EX(ClassDescriptor, co_desc) /* [1..1][const] The referenced class descriptor. */
} ClassOperatorTable;

typedef struct {
	/* A mapping-like {(string | int, int)...} object used for mapping
	 * operator names to their respective class instance table slots. */
	PROXY_OBJECT_HEAD_EX(ClassDescriptor, co_desc) /* [1..1][const] The referenced class descriptor. */
	DWEAK struct class_operator          *co_iter; /* [1..1][lock(ATOMIC)] Current iterator position. */
	struct class_operator                *co_end;  /* [1..1][const] Iterator end position. */
} ClassOperatorTableIterator;
#define COTI_GETITER(x) atomic_read(&(x)->co_iter)

INTDEF DeeTypeObject ClassOperatorTableIterator_Type;
INTDEF DeeTypeObject ClassOperatorTable_Type;


LOCAL WUNUSED NONNULL((1)) struct class_operator *DCALL
coti_next_ent(ClassOperatorTableIterator *__restrict self) {
	struct class_operator *result, *start;
	for (;;) {
		start  = atomic_read(&self->co_iter);
		result = start;
		for (;; ++result) {
			if (result >= self->co_end)
				return NULL;
			if (result->co_name != (Dee_operator_t)-1)
				break;
		}
		if (atomic_cmpxch_weak_or_write(&self->co_iter, start, result + 1))
			break;
	}
	return result;
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) struct class_operator *DCALL
coti_peek_ent(ClassOperatorTableIterator const *__restrict self) {
	struct class_operator *result, *start;
	start = atomic_read(&self->co_iter);
	for (result = start;; ++result) {
		if (result >= self->co_end)
			return NULL;
		if (result->co_name != (Dee_operator_t)-1)
			break;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
coti_bool(ClassOperatorTableIterator *__restrict self) {
	return coti_peek_ent(self) != NULL ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
coti_nextpair(ClassOperatorTableIterator *__restrict self,
              DREF DeeObject *key_and_value[2]) {
	struct class_operator *ent;
	struct opinfo const *info;
	ent = coti_next_ent(self);
	if (!ent)
		return 1;
	info = DeeTypeType_GetOperatorById(&DeeType_Type, ent->co_name);
	if (info) {
		key_and_value[0] = DeeString_New(info->oi_sname);
	} else {
		key_and_value[0] = DeeInt_NewUInt16(ent->co_name);
	}
	if unlikely(!key_and_value[0])
		goto err;
	key_and_value[1] = DeeInt_NewUInt16(ent->co_addr);
	if unlikely(!key_and_value[1])
		goto err_key_and_value_0;
	return 0;
err_key_and_value_0:
	Dee_Decref(key_and_value[0]);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
coti_nextkey(ClassOperatorTableIterator *__restrict self) {
	struct class_operator *ent;
	struct opinfo const *info;
	ent = coti_next_ent(self);
	if (!ent)
		return ITER_DONE;
	info = DeeTypeType_GetOperatorById(&DeeType_Type, ent->co_name);
	if (info)
		return DeeString_New(info->oi_sname);
	return DeeInt_NewUInt16(ent->co_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
coti_nextvalue(ClassOperatorTableIterator *__restrict self) {
	struct class_operator *ent;
	ent = coti_next_ent(self);
	if (!ent)
		return ITER_DONE;
	return DeeInt_NewUInt16(ent->co_addr);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
coti_advance(ClassOperatorTableIterator *__restrict self, size_t skip) {
	size_t result = 0;
	while (skip-- && coti_next_ent(self))
		++result;
	return result;
}

PRIVATE struct type_iterator coti_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&coti_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&coti_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&coti_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&coti_advance,
};

STATIC_ASSERT(offsetof(ClassOperatorTableIterator, co_desc) == offsetof(ProxyObject, po_obj));
#define coti_fini  generic_proxy__fini
#define coti_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
coti_serialize(ClassOperatorTableIterator *__restrict self,
               DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(ClassOperatorTableIterator, field))
	if (generic_proxy__serialize((ProxyObject *)self, writer, addr))
		goto err;
	if (DeeSerial_PutPointer(writer, ADDROF(co_iter), COTI_GETITER(self)))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(co_end), self->co_end);
#undef ADDROF
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
coti_copy(ClassOperatorTableIterator *__restrict self,
          ClassOperatorTableIterator *__restrict other) {
	self->co_desc = other->co_desc;
	self->co_iter = COTI_GETITER(other);
	self->co_end  = other->co_end;
	Dee_Incref(self->co_desc);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
coti_init(ClassOperatorTableIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	ClassOperatorTable *tab;
	DeeArg_Unpack1(err, argc, argv, "_ClassOperatorTableIterator", &tab);
	if (DeeObject_AssertTypeExact(tab, &ClassOperatorTableIterator_Type))
		goto err;
	self->co_desc = tab->co_desc;
	self->co_iter = tab->co_desc->cd_clsop_list;
	self->co_end = (tab->co_desc->cd_clsop_list +
                    tab->co_desc->cd_clsop_mask + 1);
	Dee_Incref(tab->co_desc);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassOperatorTable *DCALL
coti_getseq(ClassOperatorTableIterator *__restrict self) {
	DREF ClassOperatorTable *result;
	result = DeeObject_MALLOC(ClassOperatorTable);
	if unlikely(!result)
		goto done;
	result->co_desc = self->co_desc;
	Dee_Incref(self->co_desc);
	DeeObject_Init(result, &ClassOperatorTable_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
coti_hash(ClassOperatorTableIterator *self) {
	return Dee_HashPointer(COTI_GETITER(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
coti_compare(ClassOperatorTableIterator *self, ClassOperatorTableIterator *other) {
	if (DeeObject_AssertTypeExact(other, &ClassOperatorTableIterator_Type))
		goto err;
	Dee_return_compareT(struct class_operator *, COTI_GETITER(self),
	                    /*                    */ COTI_GETITER(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp coti_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&coti_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&coti_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_getset tpconst coti_getsets[] = {
	TYPE_GETTER_AB_F(STR_seq, &coti_getseq, METHOD_FNOREFESCAPE,
	                 "->?Ert:ClassOperatorTable"),
	TYPE_GETSET_END
};

STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) ==
              offsetof(ClassOperatorTableIterator, co_desc));
PRIVATE struct type_member tpconst coti_members[] = {
#define cot_members coti_members
	TYPE_MEMBER_FIELD_DOC("__class__", STRUCT_OBJECT,
	                      offsetof(ClassOperatorTableIterator, co_desc),
	                      "->?Ert:ClassDescriptor"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ClassOperatorTableIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassOperatorTableIterator",
	/* .tp_doc      = */ DOC("(tab:?Ert:ClassOperatorTable)\n"
	                         "\n"
	                         "next->?T2?X2?Dstring?Dint?Dint"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ClassOperatorTableIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &coti_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &coti_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &coti_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&coti_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&coti_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&coti_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &coti_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &coti_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ coti_getsets,
	/* .tp_members       = */ coti_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) == offsetof(ProxyObject, po_obj));
#define cot_copy      generic_proxy__copy_alias
#define cot_deep      generic_proxy__copy_alias
#define cot_fini      generic_proxy__fini
#define cot_visit     generic_proxy__visit
#define cot_serialize generic_proxy__serialize

PRIVATE WUNUSED NONNULL((1)) int DCALL
cot_init(ClassOperatorTable *__restrict self,
         size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_ClassOperatorTable", &self->co_desc);
	if (DeeObject_AssertTypeExact(self->co_desc, &DeeClassDescriptor_Type))
		goto err;
	Dee_Incref(self->co_desc);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cot_bool(ClassOperatorTable *__restrict self) {
	return self->co_desc->cd_clsop_list != empty_class_operators;
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassOperatorTableIterator *DCALL
cot_iter(ClassOperatorTable *__restrict self) {
	DREF ClassOperatorTableIterator *result;
	result = DeeObject_MALLOC(ClassOperatorTableIterator);
	if unlikely(!result)
		goto done;
	result->co_desc = self->co_desc;
	result->co_iter = self->co_desc->cd_clsop_list;
	result->co_end = (self->co_desc->cd_clsop_list +
	                  self->co_desc->cd_clsop_mask + 1);
	Dee_Incref(self->co_desc);
	DeeObject_Init(result, &ClassOperatorTableIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cot_size(ClassOperatorTable *__restrict self) {
	Dee_operator_t i;
	size_t result = 0;
	ClassDescriptor *desc = self->co_desc;
	for (i = 0; i <= desc->cd_clsop_mask; ++i) {
		if (desc->cd_clsop_list[i].co_name != (Dee_operator_t)-1)
			++result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cot_trygetitem_byid(ClassOperatorTable *self, Dee_operator_t opname) {
	Dee_operator_t i, perturb;
	ClassDescriptor *desc = self->co_desc;
	i = perturb = opname & desc->cd_clsop_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_operator *op;
		op = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
		if (op->co_name == (Dee_operator_t)-1)
			break;
		if (op->co_name != opname)
			continue;
		return DeeInt_NewUInt16(op->co_addr);
	}
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cot_trygetitem_index(ClassOperatorTable *self, size_t key) {
	if unlikely(key > (size_t)(Dee_operator_t)-1)
		return ITER_DONE;
	return cot_trygetitem_byid(self, (Dee_operator_t)key);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cot_trygetitem_string_hash(ClassOperatorTable *self,
                           char const *key, Dee_hash_t hash) {
	Dee_operator_t opname;
	struct opinfo const *info;
	/* TODO: Check if the table contains a string-operator "key" */
	(void)hash;
	info = DeeTypeType_GetOperatorByName(&DeeType_Type, key, (size_t)-1);
	if (info == NULL)
		goto nope;
	opname = info->oi_id;
	return cot_trygetitem_byid(self, opname);
nope:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cot_trygetitem_string_len_hash(ClassOperatorTable *self, char const *key,
                               size_t keylen, Dee_hash_t hash) {
	Dee_operator_t opname;
	struct opinfo const *info;
	/* TODO: Check if the table contains a string-operator "key" */
	(void)hash;
	info = DeeTypeType_GetOperatorByNameLen(&DeeType_Type, key, keylen, (size_t)-1);
	if (info == NULL)
		goto nope;
	opname = info->oi_id;
	return cot_trygetitem_byid(self, opname);
nope:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cot_foreach_pair(ClassOperatorTable *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	ClassDescriptor *desc = self->co_desc;
	Dee_operator_t i;
	for (i = 0; i <= desc->cd_clsop_mask; ++i) {
		struct opinfo const *info;
		struct class_operator *op;
		DREF DeeObject *name, *addr;
		op = &desc->cd_clsop_list[i];
		if (op->co_name == (Dee_operator_t)-1)
			break;
		addr = DeeInt_NEWU(op->co_addr);
		if unlikely(!addr)
			goto err;
		info = DeeTypeType_GetOperatorById(&DeeType_Type, op->co_name);
		name = info ? DeeString_New(info->oi_sname) : DeeInt_NEWU(op->co_name);
		if unlikely(!name) {
			Dee_Decref(addr);
			goto err;
		}
		temp = (*proc)(arg, name, addr);
		Dee_Decref(addr);
		Dee_Decref(name);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE struct type_seq cot_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cot_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__map_operator_contains__with__map_operator_bounditem),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__trygetitem),
	/* .tp_delitem                    = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&cot_foreach_pair,
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__trygetitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&cot_size,
	/* .tp_size_fast                  = */ NULL, /* Don't assign because it wouldn't be O(1) */
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__trygetitem_index),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__trygetitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cot_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&cot_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__trygetitem_string_hash),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__trygetitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cot_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__trygetitem_string_len_hash),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

PRIVATE struct type_member tpconst cot_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ClassOperatorTableIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cot_print(ClassOperatorTable *__restrict self,
          Dee_formatprinter_t printer, void *arg) {
	DeeStringObject *name = self->co_desc->cd_name;
	if (name == NULL)
		name = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "<operator table for %k>", name);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cot_printrepr(ClassOperatorTable *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	DeeStringObject *name = self->co_desc->cd_name;
	if (name == NULL)
		name = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "%k.__class__.operators", name);
}

INTERN DeeTypeObject ClassOperatorTable_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassOperatorTable",
	/* .tp_doc      = */ DOC("(desc:?Ert:ClassDescriptor)"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ClassOperatorTable,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &cot_copy,
			/* tp_deep_ctor:   */ &cot_deep,
			/* tp_any_ctor:    */ &cot_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &cot_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cot_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&cot_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cot_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cot_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cot_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__2E23147A197C0EE6),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &cot_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ cot_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cot_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};




typedef struct {
	PROXY_OBJECT_HEAD_EX(ClassDescriptor, ca_desc) /* [1..1][const] Class descriptor. */
	struct class_attribute const         *ca_attr; /* [1..1][const] The attribute that was queried. */
} ClassAttribute;

typedef struct {
	PROXY_OBJECT_HEAD_EX(ClassDescriptor, ca_desc) /* [1..1][const] Class descriptor. */
	DWEAK struct class_attribute const   *ca_iter; /* [1..1] Current iterator position. */
	struct class_attribute const         *ca_end;  /* [1..1][const] Iterator end. */
} ClassAttributeTableIterator;

typedef struct {
	PROXY_OBJECT_HEAD_EX(ClassDescriptor, ca_desc)  /* [1..1][const] Class descriptor. */
	struct class_attribute const         *ca_start; /* [1..1][const] Hash-vector starting pointer. */
	size_t                                ca_mask;  /* [const] Mask-vector size mask. */
} ClassAttributeTable;

STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) == offsetof(ClassAttribute, ca_desc));
#define ca_members cot_members
STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) == offsetof(ClassAttributeTable, ca_desc));
#define cat_members cot_members
STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) == offsetof(ClassAttributeTableIterator, ca_desc));
#define cati_members cot_members


INTDEF DeeTypeObject ClassAttribute_Type;
INTDEF DeeTypeObject ClassAttributeTable_Type;
INTDEF DeeTypeObject ClassAttributeTableIterator_Type;

PRIVATE WUNUSED NONNULL((1, 2)) DREF ClassAttribute *DCALL
cattr_new(ClassDescriptor *__restrict desc,
          struct class_attribute const *__restrict attr) {
	DREF ClassAttribute *result;
	result = DeeObject_MALLOC(ClassAttribute);
	if unlikely(!result)
		goto done;
	result->ca_desc = desc;
	result->ca_attr = attr;
	Dee_Incref(desc);
	DeeObject_Init(result, &ClassAttribute_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ca_copy(ClassAttribute *__restrict self,
        ClassAttribute *__restrict other) {
	self->ca_desc = other->ca_desc;
	self->ca_attr = other->ca_attr;
	Dee_Incref(other->ca_desc);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ca_serialize(ClassAttribute *__restrict self,
             DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(ClassAttribute, field))
	if (generic_proxy__serialize((ProxyObject *)self, writer, addr))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(ca_attr), self->ca_attr);
#undef ADDROF
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cat_serialize(ClassAttributeTable *__restrict self,
              DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(ClassAttributeTable, field))
	ClassAttributeTable *out = DeeSerial_Addr2Mem(writer, addr, ClassAttributeTable);
	out->ca_mask = self->ca_mask;
	if (generic_proxy__serialize((ProxyObject *)self, writer, addr))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(ca_start), self->ca_start);
#undef ADDROF
err:
	return -1;
}

STATIC_ASSERT(offsetof(ClassAttribute, ca_desc) == offsetof(ProxyObject, po_obj));
#define ca_fini  generic_proxy__fini
#define ca_visit generic_proxy__visit
STATIC_ASSERT(offsetof(ClassAttributeTable, ca_desc) == offsetof(ProxyObject, po_obj));
#define cat_fini  generic_proxy__fini
#define cat_visit generic_proxy__visit
STATIC_ASSERT(offsetof(ClassAttributeTableIterator, ca_desc) == offsetof(ProxyObject, po_obj));
#define cati_fini  generic_proxy__fini
#define cati_visit generic_proxy__visit

STATIC_ASSERT(offsetof(ClassOperatorTableIterator, co_desc) == offsetof(ClassAttributeTableIterator, ca_desc));
STATIC_ASSERT(offsetof(ClassOperatorTableIterator, co_iter) == offsetof(ClassAttributeTableIterator, ca_iter));
STATIC_ASSERT(offsetof(ClassOperatorTableIterator, co_end) == offsetof(ClassAttributeTableIterator, ca_end));
#define cati_cmp       coti_cmp
#define cati_copy      coti_copy
#define cati_serialize coti_serialize

#define CATI_GETITER(x) atomic_read(&(x)->ca_iter)

PRIVATE WUNUSED NONNULL((1)) int DCALL
cati_iter(ClassAttributeTableIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	ClassAttributeTable *tab;
	DeeArg_Unpack1(err, argc, argv, "_ClassAttributeTableIterator", &tab);
	if (DeeObject_AssertTypeExact(tab, &ClassAttributeTable_Type))
		goto err;
	self->ca_desc = tab->ca_desc;
	self->ca_iter = tab->ca_start;
	self->ca_end  = (tab->ca_start + tab->ca_mask + 1);
	Dee_Incref(tab->ca_desc);
	return 0;
err:
	return -1;
}

LOCAL WUNUSED NONNULL((1)) struct class_attribute const *DCALL
cati_next_ent(ClassAttributeTableIterator *__restrict self) {
	struct class_attribute const *result, *start;
	for (;;) {
		start  = atomic_read(&self->ca_iter);
		result = start;
		for (;; ++result) {
			if (result >= self->ca_end)
				return NULL;
			if (result->ca_name != NULL)
				break;
		}
		if (atomic_cmpxch_weak_or_write(&self->ca_iter, start, result + 1))
			break;
	}
	return result;
}

LOCAL ATTR_PURE WUNUSED NONNULL((1)) struct class_attribute const *DCALL
cati_peek_ent(ClassAttributeTableIterator const *__restrict self) {
	struct class_attribute const *result, *start;
	start = atomic_read(&self->ca_iter);
	for (result = start;; ++result) {
		if (result >= self->ca_end)
			return NULL;
		if (result->ca_name != NULL)
			break;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cati_bool(ClassAttributeTableIterator *__restrict self) {
	return cati_peek_ent(self) != NULL ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cati_nextpair(ClassAttributeTableIterator *__restrict self, DeeObject *key_and_value[2]) {
	DREF ClassAttribute *attr;
	struct class_attribute const *ent;
	ent = cati_next_ent(self);
	if (!ent)
		return 1;
	attr = cattr_new(self->ca_desc, ent);
	if unlikely(!attr)
		goto err;
	Dee_Incref(ent->ca_name);
	key_and_value[0] = Dee_AsObject(ent->ca_name);
	key_and_value[1] = Dee_AsObject(attr); /* Inherit reference */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cati_nextkey(ClassAttributeTableIterator *__restrict self) {
	struct class_attribute const *ent;
	ent = cati_next_ent(self);
	if (!ent)
		return ITER_DONE;
	return_reference_((DeeObject *)ent->ca_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassAttribute *DCALL
cati_nextvalue(ClassAttributeTableIterator *__restrict self) {
	struct class_attribute const *ent;
	ent = cati_next_ent(self);
	if (!ent)
		return (DREF ClassAttribute *)ITER_DONE;
	return cattr_new(self->ca_desc, ent);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cati_advance(ClassAttributeTableIterator *__restrict self, size_t skip) {
	size_t result = 0;
	while (skip-- && cati_next_ent(self))
		++result;
	return result;
}

PRIVATE struct type_iterator cati_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&cati_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cati_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cati_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&cati_advance,
};


PRIVATE WUNUSED NONNULL((1)) DREF ClassAttributeTableIterator *DCALL
cat_iter(ClassAttributeTable *__restrict self) {
	DREF ClassAttributeTableIterator *result;
	result = DeeObject_MALLOC(ClassAttributeTableIterator);
	if unlikely(!result)
		goto done;
	result->ca_desc = self->ca_desc;
	result->ca_iter = self->ca_start;
	result->ca_end  = (self->ca_start + self->ca_mask + 1);
	Dee_Incref(self->ca_desc);
	DeeObject_Init(result, &ClassAttributeTableIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cat_bool(ClassAttributeTable *__restrict self) {
	size_t i;
	for (i = 0; i <= self->ca_mask; ++i) {
		if (self->ca_start[i].ca_name != NULL)
			return 1;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cat_size(ClassAttributeTable *__restrict self) {
	size_t i, result = 0;
	for (i = 0; i <= self->ca_mask; ++i) {
		if (self->ca_desc[i].cd_name != NULL)
			++result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cat_trygetitem_string_len_hash(ClassAttributeTable *self, char const *key,
                               size_t keylen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	i = perturb = hash & self->ca_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_attribute const *at;
		at = &self->ca_start[i & self->ca_mask];
		if (at->ca_name == NULL)
			break;
		if (at->ca_hash != hash)
			continue;
		if (!DeeString_EqualsBuf(at->ca_name, key, keylen))
			continue;
		return Dee_AsObject(cattr_new(self->ca_desc, at));
	}
/*nope:*/
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cat_foreach_pair(ClassAttributeTable *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_hash_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i <= self->ca_mask; ++i) {
		struct class_attribute const *at;
		DREF ClassAttribute *attr;
		at = &self->ca_start[i];
		if (at->ca_name == NULL)
			continue;
		attr = cattr_new(self->ca_desc, at);
		if unlikely(!attr)
			goto err;
		temp = (*proc)(arg, (DeeObject *)at->ca_name, (DeeObject *)attr);
		Dee_Decref(attr);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassAttributeTable *DCALL
cati_getseq(ClassAttributeTableIterator *__restrict self) {
	DREF ClassAttributeTable *result;
	result = DeeObject_MALLOC(ClassAttributeTable);
	if unlikely(!result)
		goto done;
	result->ca_desc = self->ca_desc;
	if (self->ca_end == self->ca_desc->cd_iattr_list + self->ca_desc->cd_iattr_mask + 1) {
		result->ca_start = self->ca_desc->cd_iattr_list;
		result->ca_mask  = self->ca_desc->cd_iattr_mask;
	} else {
		result->ca_start = self->ca_desc->cd_cattr_list;
		result->ca_mask  = self->ca_desc->cd_cattr_mask;
	}
	Dee_Incref(self->ca_desc);
	DeeObject_Init(result, &ClassAttributeTable_Type);
done:
	return result;
}

PRIVATE struct type_getset tpconst cati_getsets[] = {
	TYPE_GETTER_AB_F(STR_seq, &cati_getseq, METHOD_FNOREFESCAPE,
	                 "->?AAttributeTable?Ert:ClassDescriptor"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cat_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ClassAttributeTableIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ca_print(ClassAttribute *__restrict self,
         Dee_formatprinter_t printer, void *arg) {
	DeeStringObject *cnam = self->ca_desc->cd_name;
	if (cnam == NULL)
		cnam = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "<ClassAttribute %k.%k>",
	                        cnam, self->ca_attr->ca_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ca_printrepr(ClassAttribute *__restrict self,
             Dee_formatprinter_t printer, void *arg) {
	ClassDescriptor *desc = self->ca_desc;
	DeeStringObject *cnam = desc->cd_name;
	char field_id = 'c';
	if (self->ca_attr >= desc->cd_iattr_list &&
	    self->ca_attr <= desc->cd_iattr_list + desc->cd_iattr_mask)
		field_id = 'i';
	if (cnam == NULL)
		cnam = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg,
	                        "%k.__class__.%cattr[%r]",
	                        cnam, field_id,
	                        self->ca_attr->ca_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_getname(ClassAttribute *__restrict self) {
	return_reference_(Dee_AsObject(self->ca_attr->ca_name));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_getdoc(ClassAttribute *__restrict self) {
	DeeStringObject *result;
	result = self->ca_attr->ca_doc;
	if (!result)
		result = (DeeStringObject *)Dee_None;
	return_reference_((DeeObject *)result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_getaddr(ClassAttribute *__restrict self) {
	return DeeInt_NewUInt16(self->ca_attr->ca_addr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_isprivate(ClassAttribute *__restrict self) {
	return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_isfinal(ClassAttribute *__restrict self) {
	return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FFINAL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_isreadonly(ClassAttribute *__restrict self) {
	return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_ismethod(ClassAttribute *__restrict self) {
	return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_isproperty(ClassAttribute *__restrict self) {
	return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FGETSET);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_isclassmem(ClassAttribute *__restrict self) {
	return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_getisclassns(ClassAttribute *__restrict self) {
	ClassDescriptor *desc = self->ca_desc;
	return_bool(!(self->ca_attr >= desc->cd_iattr_list &&
	              self->ca_attr <= desc->cd_iattr_list +
	                               desc->cd_iattr_mask));
}


struct attr_flag_entry {
	uint16_t fe_flag;
	char     fe_name[14];
};

#define CLASS_ATTRIBUTE_FLAGS_DB_KNOWN                     \
	(CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL |   \
	 CLASS_ATTRIBUTE_FREADONLY | CLASS_ATTRIBUTE_FMETHOD | \
	 CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)
PRIVATE struct attr_flag_entry const class_attribute_flags_db[] = {
	{ CLASS_ATTRIBUTE_FPRIVATE, "private" },
	{ CLASS_ATTRIBUTE_FFINAL, "final" },
	{ CLASS_ATTRIBUTE_FREADONLY, "readonly" },
	{ CLASS_ATTRIBUTE_FMETHOD, "method" },
	{ CLASS_ATTRIBUTE_FGETSET, "property" },
	{ CLASS_ATTRIBUTE_FCLASSMEM, "classns" },
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_getflags(ClassAttribute *__restrict self) {
	unsigned int i;
	uint16_t flags;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	flags = self->ca_attr->ca_flag;
	for (i = 0; i < COMPILER_LENOF(class_attribute_flags_db); ++i) {
		char const *name;
		if (!(flags & class_attribute_flags_db[i].fe_flag))
			continue;
		if (ASCII_PRINTER_LEN(&printer) != 0 &&
		    ascii_printer_putc(&printer, ','))
			goto err;
		name = class_attribute_flags_db[i].fe_name;
		if (ascii_printer_print(&printer, name, strlen(name)) < 0)
			goto err;
	}
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}


PRIVATE struct type_getset tpconst ca_getsets[] = {
	TYPE_GETTER_AB_F("name", &ca_getname, METHOD_FNOREFESCAPE, "->?Dstring"),
	TYPE_GETTER_AB_F("doc", &ca_getdoc, METHOD_FNOREFESCAPE, "->?X2?Dstring?N"),
	TYPE_GETTER_AB_F("addr", &ca_getaddr, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Index into the class/instance object table, where @this attribute is stored\n"
	                 "When ?#isclassmem or ?#isclassns are ?t, this index and any index offset from it "
	                 /**/ "refer to the class object table. Otherwise, the instance object table is dereferenced\n"
	                 "This is done so-as to allow instance attributes such as member functions to be stored "
	                 /**/ "within the class itself, rather than having to be copied into each and every instance "
	                 /**/ "of the class\n"
	                 "S.a. ?A__ctable__?DType and ?A__itable__?DType"),
	TYPE_GETTER_AB_F("isprivate", &ca_isprivate, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Evaluates to ?t if @this class attribute was declared as $private"),
	TYPE_GETTER_AB_F("isfinal", &ca_isfinal, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Evaluates to ?t if @this class attribute was declared as $final"),
	TYPE_GETTER_AB_F("isreadonly", &ca_isreadonly, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Evaluates to ?t if @this class attribute can only be read from\n"
	                 "When this is case, a property-like attribute can only ever have a getter "
	                 /**/ "associated with itself, while field- or method-like attribute can only be "
	                 /**/ "written once (aka. when not already bound)"),
	TYPE_GETTER_AB_F("ismethod", &ca_ismethod, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Evaluates to ?t if @this class attribute refers to a method\n"
	                 "When set, reading from the attribute will return a an object "
	                 /**/ "${InstanceMethod(obj.MEMBER_TABLE[this.addr], obj)}\n"
	                 "Note however that this is rarely ever required to be done, as method attributes "
	                 /**/ "are usually called directly, in which case a callattr instruction can silently "
	                 /**/ "prepend the this-argument before the passed argument list"),
	TYPE_GETTER_AB_F("isproperty", &ca_isproperty, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Evaluates to ?t if @this class attribute was defined as a property\n"
	                 "When this is the case, a ?#readonly attribute only has a single callback "
	                 /**/ "that may be stored at ?#addr + 0, with that callback being the getter\n"
	                 "Otherwise, up to " PP_STR(CLASS_GETSET_COUNT) " indices within the associated "
	                 /**/ "object table are used by @this attribute, each of which may be left unbound:\n"
	                 "#T{Offset|Callback|Description~"
	                 /**/ "$" PP_STR(CLASS_GETSET_GET) "|$get|The getter callback&"
	                 /**/ "$" PP_STR(CLASS_GETSET_DEL) "|$del|The delete callback&"
	                 /**/ "$" PP_STR(CLASS_GETSET_SET) "|$set|The setter callback"
	                 "}"),
	TYPE_GETTER_AB_F("isclassmem", &ca_isclassmem, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Set if ?#addr is an index into the class object table, rather than into "
	                 /**/ "the instance object table. Note however that when ?#isclassns"),
	TYPE_GETTER_AB_F("isclassns", &ca_getisclassns, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this class attribute is exclusive to the "
	                 /**/ "class-namespace (i.e. was declared as $static)\n"
	                 "During enumeration of attributes, all attributes where this is ?t "
	                 /**/ "are enumated by ?Acattr?Ert:ClassDescriptor, while all for which it isn't "
	                 /**/ "are enumated by ?Aiattr?Ert:ClassDescriptor"),
	TYPE_GETTER_AB_F("flags", &ca_getflags, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "Returns a comma-separated string describing the flags of @this class attribute\n"
	                 "#T{Flag|Property~"
	                 /**/ "$\"private\"|?#isprivate&"
	                 /**/ "$\"final\"|?#isfinal&"
	                 /**/ "$\"readonly\"|?#isreadonly&"
	                 /**/ "$\"method\"|?#ismethod&"
	                 /**/ "$\"property\"|?#isproperty&"
	                 /**/ "$\"classns\"|?#isclassns"
	                 "}"),
	TYPE_GETTER_AB_F("__name__", &ca_getname, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "Alias for ?#name"),
	TYPE_GETTER_AB_F("__doc__", &ca_getdoc, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "Alias for ?#doc"),
	TYPE_GETSET_END
};


PRIVATE struct type_seq cat_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__map_operator_contains__with__map_operator_bounditem),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_string_len_hash),
	/* .tp_delitem                    = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&cat_foreach_pair,
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__bounditem_string_len_hash),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&cat_size,
	/* .tp_size_fast                  = */ NULL, /* Don't assign because it wouldn't be O(1) */
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__bounditem),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))NULL,
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem_string_len_hash),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem_string_len_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cat_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__trygetitem_string_len_hash),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

INTERN DeeTypeObject ClassAttribute_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassAttribute",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ClassAttribute,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ca_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ca_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ca_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ca_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ca_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ca_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__8F384E6A64571883),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ca_getsets,
	/* .tp_members       = */ ca_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject ClassAttributeTableIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassAttributeTableIterator",
	/* .tp_doc      = */ DOC("(tab:?Ert:ClassAttributeTable)\n"
	                         "\n"
	                         "next->?T2?Dstring?AAttribute?Ert:ClassDescriptor"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ClassAttributeTableIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &cati_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &cati_iter,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &cati_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cati_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cati_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cati_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &cati_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &cati_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ cati_getsets,
	/* .tp_members       = */ cati_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cat_print(ClassAttributeTable *__restrict self,
          Dee_formatprinter_t printer, void *arg) {
	ClassDescriptor *desc = self->ca_desc;
	DeeStringObject *name = desc->cd_name;
	if (name == NULL)
		name = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "<%s attribute table for %k>",
	                        self->ca_start == desc->cd_cattr_list ? "class" : "instance",
	                        name);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cat_printrepr(ClassAttributeTable *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	ClassDescriptor *desc = self->ca_desc;
	DeeStringObject *cnam = desc->cd_name;
	char field_id = 'c';
	if (self->ca_start == desc->cd_iattr_list)
		field_id = 'i';
	if (cnam == NULL)
		cnam = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "%k.__class__.%cattr", cnam, field_id);
}

INTERN DeeTypeObject ClassAttributeTable_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassAttributeTable",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ClassAttributeTable,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &cat_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cat_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&cat_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cat_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cat_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cat_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__2E23147A197C0EE6),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &cat_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ cat_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cat_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};








PRIVATE NONNULL((1)) void DCALL
cd_fini(ClassDescriptor *__restrict self) {
	size_t i;
	if (self->cd_cattr_list != empty_class_attributes) {
		for (i = 0; i <= self->cd_cattr_mask; ++i) {
			if (!self->cd_cattr_list[i].ca_name)
				continue;
			Dee_Decref(self->cd_cattr_list[i].ca_name);
			Dee_XDecref(self->cd_cattr_list[i].ca_doc);
		}
		Dee_Free(self->cd_cattr_list);
	}
	if (self->cd_clsop_list != empty_class_operators)
		Dee_Free(self->cd_clsop_list);
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		if (!self->cd_iattr_list[i].ca_name)
			continue;
		Dee_Decref(self->cd_iattr_list[i].ca_name);
		Dee_XDecref(self->cd_iattr_list[i].ca_doc);
	}
	Dee_XDecref(self->cd_name);
	Dee_XDecref(self->cd_doc);
}

PRIVATE NONNULL((1, 2)) void DCALL
cd_visit(ClassDescriptor *__restrict self, Dee_visit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->cd_cattr_mask; ++i) {
		if (!self->cd_cattr_list[i].ca_name)
			continue;
		Dee_Visit(self->cd_cattr_list[i].ca_name);
		Dee_XVisit(self->cd_cattr_list[i].ca_doc);
	}
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		if (!self->cd_iattr_list[i].ca_name)
			continue;
		Dee_Visit(self->cd_iattr_list[i].ca_name);
		Dee_XVisit(self->cd_iattr_list[i].ca_doc);
	}
	Dee_XVisit(self->cd_name);
	Dee_XVisit(self->cd_doc);
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
cd_serialize(ClassDescriptor *__restrict self, DeeSerial *__restrict writer) {
	ClassDescriptor *out;
	Dee_seraddr_t out_addr;
	size_t i, sizeof_self;
	sizeof_self = offsetof(ClassDescriptor, cd_iattr_list) +
	              (self->cd_iattr_mask + 1) *
	              sizeof(struct Dee_class_attribute);
	out_addr = DeeSerial_ObjectMalloc(writer, sizeof_self, self);
	if (!Dee_SERADDR_ISOK(out_addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, ClassDescriptor);
#define ADDROF(field) (out_addr + offsetof(ClassDescriptor, field))
	out->cd_flags      = self->cd_flags;
	out->cd_cmemb_size = self->cd_cmemb_size;
	out->cd_imemb_size = self->cd_imemb_size;
	out->cd_clsop_mask = self->cd_clsop_mask;
	out->cd_cattr_mask = self->cd_cattr_mask;
	out->cd_iattr_mask = self->cd_iattr_mask;
	memcpyc(out->cd_iattr_list, self->cd_iattr_list,
	        self->cd_iattr_mask + 1,
	        sizeof(struct Dee_class_attribute));

	/* Write class name and doc-string */
	if (DeeSerial_XPutObject(writer, ADDROF(cd_name), self->cd_name))
		goto err;
	if (DeeSerial_XPutObject(writer, ADDROF(cd_doc), self->cd_doc))
		goto err;

	/* Write instance attribute table. */
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		Dee_seraddr_t out_addr__cd_iattr_list__i;
		if (!self->cd_iattr_list[i].ca_name)
			continue;
		out_addr__cd_iattr_list__i = ADDROF(cd_iattr_list);
		out_addr__cd_iattr_list__i += i * sizeof(struct Dee_class_attribute);
		if (DeeSerial_PutObject(writer,
		                        out_addr__cd_iattr_list__i +
		                        offsetof(struct Dee_class_attribute, ca_name),
		                        self->cd_iattr_list[i].ca_name))
			goto err;
		if (DeeSerial_XPutObject(writer,
		                         out_addr__cd_iattr_list__i +
		                         offsetof(struct Dee_class_attribute, ca_doc),
		                         self->cd_iattr_list[i].ca_doc))
			goto err;
	}

	/* Write class attribute table. */
	if (self->cd_cattr_list == empty_class_attributes) {
		if (DeeSerial_PutStaticDeemon(writer, ADDROF(cd_cattr_list), empty_class_attributes))
			goto err;
	} else {
		size_t sizeof__cd_cattr_list;
		Dee_seraddr_t addrof_out__cd_cattr_list;
		struct Dee_class_attribute *out__cd_cattr_list;
		struct Dee_class_attribute *in__cd_cattr_list = self->cd_cattr_list;
		sizeof__cd_cattr_list = (self->cd_cattr_mask + 1) * sizeof(struct Dee_class_attribute);
		addrof_out__cd_cattr_list = DeeSerial_Malloc(writer, sizeof__cd_cattr_list, in__cd_cattr_list);
		if (!Dee_SERADDR_ISOK(addrof_out__cd_cattr_list))
			goto err;
		if (DeeSerial_PutAddr(writer, ADDROF(cd_cattr_list), addrof_out__cd_cattr_list))
			goto err;
		out__cd_cattr_list = DeeSerial_Addr2Mem(writer, addrof_out__cd_cattr_list,
		                                        struct Dee_class_attribute);
		memcpy(out__cd_cattr_list, in__cd_cattr_list, sizeof__cd_cattr_list);
		for (i = 0; i <= self->cd_cattr_mask; ++i) {
			Dee_seraddr_t addrof_out__cd_cattr_list__i;
			if (!in__cd_cattr_list[i].ca_name)
				continue;
			addrof_out__cd_cattr_list__i = addrof_out__cd_cattr_list;
			addrof_out__cd_cattr_list__i += i * sizeof(struct Dee_class_attribute);
			if (DeeSerial_PutObject(writer,
			                        addrof_out__cd_cattr_list__i +
			                        offsetof(struct Dee_class_attribute, ca_name),
			                        in__cd_cattr_list[i].ca_name))
				goto err;
			if (DeeSerial_XPutObject(writer,
			                         addrof_out__cd_cattr_list__i +
			                         offsetof(struct Dee_class_attribute, ca_doc),
			                         in__cd_cattr_list[i].ca_doc))
				goto err;
		}
	}

	/* Write class operator table. */
	if (self->cd_clsop_list == empty_class_operators) {
		if (DeeSerial_PutStaticDeemon(writer, ADDROF(cd_clsop_list), empty_class_operators))
			goto err;
	} else {
		size_t sizeof__cd_clsop_list;
		Dee_seraddr_t addrof_out__cd_clsop_list;
		struct Dee_class_operator *out__cd_clsop_list;
		struct Dee_class_operator *in__cd_clsop_list = self->cd_clsop_list;
		sizeof__cd_clsop_list = (self->cd_clsop_mask + 1) * sizeof(struct Dee_class_operator);
		addrof_out__cd_clsop_list = DeeSerial_Malloc(writer, sizeof__cd_clsop_list, in__cd_clsop_list);
		if (!Dee_SERADDR_ISOK(addrof_out__cd_clsop_list))
			goto err;
		if (DeeSerial_PutAddr(writer, ADDROF(cd_clsop_list), addrof_out__cd_clsop_list))
			goto err;
		out__cd_clsop_list = DeeSerial_Addr2Mem(writer, addrof_out__cd_clsop_list,
		                                        struct Dee_class_operator);
		memcpy(out__cd_clsop_list, in__cd_clsop_list, sizeof__cd_clsop_list);
	}
#undef ADDROF
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
}


PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
class_attribute_eq(struct class_attribute *__restrict lhs,
                   struct class_attribute *__restrict rhs) {
	if (!lhs->ca_name)
		return rhs->ca_name == NULL;
	if (!rhs->ca_name)
		goto nope;
	if ((lhs->ca_doc != NULL) != (rhs->ca_doc != NULL))
		goto nope;
	if (lhs->ca_flag != rhs->ca_flag)
		goto nope;
	if (lhs->ca_addr != rhs->ca_addr)
		goto nope;
	if (lhs->ca_hash != rhs->ca_hash)
		goto nope;
	if (!DeeString_EqualsSTR(lhs->ca_name, rhs->ca_name))
		goto nope;
	if (lhs->ca_doc && !DeeString_EqualsSTR(lhs->ca_doc, rhs->ca_doc))
		goto nope;
	return true;
nope:
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cd_compare_eq(ClassDescriptor *lhs, ClassDescriptor *rhs) {
	size_t i;
	if (DeeObject_AssertTypeExact(rhs, &DeeClassDescriptor_Type))
		goto err;
	if (lhs->cd_flags != rhs->cd_flags)
		goto nope;
	if (lhs->cd_cmemb_size != rhs->cd_cmemb_size)
		goto nope;
	if (lhs->cd_imemb_size != rhs->cd_imemb_size)
		goto nope;
	/* FIXME: hash vectors cannot be binary-compared. That leads to false negatives.
	 *        -> s.a. `/util/test/rt-classdescriptor-reprcopy.dee' */
	if (lhs->cd_clsop_mask != rhs->cd_clsop_mask)
		goto nope;
	if (lhs->cd_cattr_mask != rhs->cd_cattr_mask)
		goto nope;
	if (lhs->cd_iattr_mask != rhs->cd_iattr_mask)
		goto nope;
	if (lhs->cd_name) {
		if (!rhs->cd_name)
			goto nope;
		if (!DeeString_EqualsSTR(lhs->cd_name, rhs->cd_name))
			goto nope;
	} else {
		if (rhs->cd_name)
			goto nope;
	}
	if (lhs->cd_doc) {
		if (!rhs->cd_doc)
			goto nope;
		if (!DeeString_EqualsSTR(lhs->cd_doc, rhs->cd_doc))
			goto nope;
	} else {
		if (rhs->cd_doc)
			goto nope;
	}
	if (bcmpc(lhs->cd_clsop_list,
	          rhs->cd_clsop_list,
	          lhs->cd_clsop_mask + 1,
	          sizeof(struct class_operator)) != 0)
		goto nope;
	for (i = 0; i <= lhs->cd_cattr_mask; ++i) {
		if (!class_attribute_eq(&lhs->cd_cattr_list[i],
		                        &rhs->cd_cattr_list[i]))
			goto nope;
	}
	for (i = 0; i <= lhs->cd_iattr_mask; ++i) {
		if (!class_attribute_eq(&lhs->cd_iattr_list[i],
		                        &rhs->cd_iattr_list[i]))
			goto nope;
	}
	return Dee_COMPARE_EQ;
nope:
	return Dee_COMPARE_NE;
err:
	return Dee_COMPARE_ERR;
}


PRIVATE struct type_cmp cd_cmp = {
	/* .tp_hash       = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&cd_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};


PRIVATE WUNUSED NONNULL((1)) DREF ClassOperatorTable *DCALL
cd_operators(ClassDescriptor *__restrict self) {
	DREF ClassOperatorTable *result;
	result = DeeObject_MALLOC(ClassOperatorTable);
	if unlikely(!result)
		goto done;
	result->co_desc = self;
	Dee_Incref(self);
	DeeObject_Init(result, &ClassOperatorTable_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassAttributeTable *DCALL
cd_iattr(ClassDescriptor *__restrict self) {
	DREF ClassAttributeTable *result;
	result = DeeObject_MALLOC(ClassAttributeTable);
	if unlikely(!result)
		goto done;
	result->ca_desc  = self;
	result->ca_start = self->cd_iattr_list;
	result->ca_mask  = self->cd_iattr_mask;
	Dee_Incref(self);
	DeeObject_Init(result, &ClassAttributeTable_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassAttributeTable *DCALL
cd_cattr(ClassDescriptor *__restrict self) {
	DREF ClassAttributeTable *result;
	result = DeeObject_MALLOC(ClassAttributeTable);
	if unlikely(!result)
		goto done;
	result->ca_desc  = self;
	result->ca_start = self->cd_cattr_list;
	result->ca_mask  = self->cd_cattr_mask;
	Dee_Incref(self);
	DeeObject_Init(result, &ClassAttributeTable_Type);
done:
	return result;
}

struct class_flag_entry {
	uint16_t fe_flag;
	char     fe_name[14];
};

#define CLASS_FLAGS_DB_KNOWN                       \
	(TP_FFINAL | TP_FINTERRUPT | TP_FINHERITCTOR | \
	 CLASS_TP_FSUPERKWDS | CLASS_TP_FAUTOINIT |    \
	 TP_FTRUNCATE | TP_FMOVEANY)
PRIVATE struct class_flag_entry const class_flags_db[] = {
	{ TP_FFINAL,           "final" },
	{ TP_FINTERRUPT,       "interrupt" },
	{ TP_FINHERITCTOR,     "superctor" },
	{ CLASS_TP_FSUPERKWDS, "superkwds" },
	{ CLASS_TP_FAUTOINIT,  "autoinit" },
	{ TP_FTRUNCATE,        "inttrunc" },
	{ TP_FMOVEANY,         "moveany" },
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cd_getflags(ClassDescriptor *__restrict self) {
	unsigned int i;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	for (i = 0; i < COMPILER_LENOF(class_flags_db); ++i) {
		char const *name;
		if (!(self->cd_flags & class_flags_db[i].fe_flag))
			continue;
		if (ASCII_PRINTER_LEN(&printer) != 0 &&
		    ascii_printer_putc(&printer, ','))
			goto err;
		name = class_flags_db[i].fe_name;
		if (ascii_printer_print(&printer, name, strlen(name)) < 0)
			goto err;
	}
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cd_sizeof(ClassDescriptor *self) {
	size_t result;
	result = offsetof(ClassDescriptor, cd_iattr_list);
	result += (self->cd_iattr_mask + 1) * sizeof(struct class_attribute);
	if (self->cd_cattr_list != empty_class_attributes)
		result += (self->cd_cattr_mask + 1) * sizeof(struct class_attribute);
	if (self->cd_clsop_list != empty_class_operators)
		result += (self->cd_clsop_mask + 1) * sizeof(struct class_operator);
	return DeeInt_NewSize(result);
}

#if 1
#define cd_methods NULL
#else
PRIVATE struct type_method tpconst cd_methods[] = {
	TYPE_METHOD_END
};
#endif

PRIVATE struct type_getset tpconst cd_getsets[] = {
	TYPE_GETTER_AB_F("flags", &cd_getflags, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "Return a comma-separated string of flags used to describe the combination of "
	                 /**/ "properties described by ?#isfinal, ?#isinterrupt, ?#hassuperconstructor, "
	                 /**/ "?#__hassuperkwds__, ?#__isinttruncated__, and ?#__hasmoveany__\n"
	                 "#T{Flag|Property~"
	                 /**/ "$\"final\"|?#isfinal&"
	                 /**/ "$\"interrupt\"|?#isinterrupt&"
	                 /**/ "$\"superctor\"|?#hassuperconstructor&"
	                 /**/ "$\"superkwds\"|?#__hassuperkwds__&"
	                 /**/ "$\"autoinit\"|?#__hasautoinit__&"
	                 /**/ "$\"inttrunc\"|?#__isinttruncated__&"
	                 /**/ "$\"moveany\"|?#__hasmoveany__"
	                 "}"),
	TYPE_GETTER_AB(STR_operators, &cd_operators,
	               "->?#OperatorTable\n"
	               "Enumerate operators implemented by @this class, as well as their associated "
	               /**/ "class object table indices which are holding the respective implementations\n"
	               "Note that the class object table entry may be left unbound to explicitly "
	               /**/ "define an operator as having been deleted"),
	TYPE_GETTER_AB("iattr", &cd_iattr,
	               "->?#AttributeTable\n"
	               "Enumerate user-defined instance attributes as a mapping-like "
	               /**/ "?Dstring-?#Attribute sequence"),
	TYPE_GETTER_AB("cattr", &cd_cattr,
	               "->?#AttributeTable\n"
	               "Enumerate user-defined class ($static) attributes as a mapping-like "
	               /**/ "?Dstring-?#Attribute sequence"),
	TYPE_GETTER_AB("__sizeof__", &cd_sizeof, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cd_members[] = {
	TYPE_MEMBER_BITFIELD("isfinal", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FFINAL),
	TYPE_MEMBER_BITFIELD_DOC("isinterrupt", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FINTERRUPT,
	                         "Evaluates to ?t if @this class behaves as an interrupt exception when thrown\n"
	                         "An interrupt exception (such as :Interrupt) is not caught by ${catch(...)} "
	                         /**/ "statements, but only by statements marked as ${@[interrupt] catch(...)}\n"
	                         "Certain types exceptions require this in order to prevent catch-all blocks surrounding "
	                         /**/ "optional function calls such as invocations of ?Efs:unlink from accidentally handling "
	                         /**/ "unwanted types of exceptions such as :KeyboardInterrupt, as caused "
	                         /**/ "by the user pressing CTRL+C to terminate the running script, and (normally) not "
	                         /**/ "expecting it to continue running because the error was silently swallowed by an "
	                         /**/ "unrelated catch-all block"),
	TYPE_MEMBER_BITFIELD_DOC("hassuperconstructor", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FINHERITCTOR,
	                         "Evaluates to ?t if @this class inherits its constructor from its base-type\n"
	                         "In user-defined classes, this behavior is encoded as ${this = super;}"),
#ifdef CLASS_TP_FSUPERKWDS
	TYPE_MEMBER_BITFIELD_DOC("__hassuperkwds__", STRUCT_CONST, ClassDescriptor, cd_flags, CLASS_TP_FSUPERKWDS,
	                         "Evaluates to ?t if the super-args operator of @this class returns a tuple (args, kwds) "
	                         /**/ "that should be used to invoke the super-constructor as ${super(args..., **kwds)}\n"
	                         "Otherwise, the super-args operator simply returns $args and the super-constructor "
	                         /**/ "is called as ${super(args...)}"),
#endif /* CLASS_TP_FSUPERKWDS */
#ifdef CLASS_TP_FAUTOINIT
	TYPE_MEMBER_BITFIELD_DOC("__hasautoinit__", STRUCT_CONST, ClassDescriptor, cd_flags, CLASS_TP_FAUTOINIT,
	                         "Evaluates to ?t if @this class provides an automatic initializer and ${operator repr}\n"
	                         "This is used to implement the user-code ${this = default;} constructor definition"),
#endif /* CLASS_TP_FAUTOINIT */
	TYPE_MEMBER_BITFIELD("__isinttruncated__", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FTRUNCATE),
	TYPE_MEMBER_BITFIELD("__hasmoveany__", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FMOVEANY),
	TYPE_MEMBER_FIELD_DOC(STR___name__, STRUCT_OBJECT, offsetof(ClassDescriptor, cd_name), "->?Dstring"),
	TYPE_MEMBER_FIELD_DOC(STR___doc__, STRUCT_OBJECT_OPT, offsetof(ClassDescriptor, cd_doc), "->?X2?Dstring?N"),
	TYPE_MEMBER_FIELD_DOC("__csize__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(ClassDescriptor, cd_cmemb_size), "Size of the class object table"),
	TYPE_MEMBER_FIELD_DOC("__isize__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(ClassDescriptor, cd_imemb_size), "Size of the instance object table"),
	TYPE_MEMBER_END
};

INTDEF DeeTypeObject ObjectTable_Type;
PRIVATE struct type_member tpconst cd_class_members[] = {
	TYPE_MEMBER_CONST(STR_Attribute, &ClassAttribute_Type),
	TYPE_MEMBER_CONST("AttributeTable", &ClassAttributeTable_Type),
	TYPE_MEMBER_CONST("OperatorTable", &ClassOperatorTable_Type),
	TYPE_MEMBER_CONST("ObjectTable", &ObjectTable_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cd_print(ClassDescriptor *__restrict self,
         Dee_formatprinter_t printer, void *arg) {
	DeeStringObject *name = self->cd_name;
	if (name == NULL)
		name = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "<ClassDescriptor for %k>", name);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_ssize_t DCALL
xattr_table_printrepr(struct Dee_class_attribute *table, size_t mask,
                      Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	bool is_first = true;
	for (i = 0; i <= mask; ++i) {
		struct class_attribute *ent = &table[i];
		if (!ent->ca_name)
			continue;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "%r: ", ent->ca_name));
		if (ent->ca_doc || ent->ca_flag) {
			/* Use tuple representation */
			DO(err, DeeFormat_Printf(printer, arg, "(%#" PRFx16 ", ", ent->ca_addr));
			if (ent->ca_flag & ~CLASS_ATTRIBUTE_FLAGS_DB_KNOWN) {
				DO(err, DeeFormat_Printf(printer, arg, "%#" PRFx16, ent->ca_flag));
			} else {
				size_t flag_i;
				bool flag_first = true;
				DO(err, DeeFormat_PRINT(printer, arg, "\""));
				for (flag_i = 0; flag_i < COMPILER_LENOF(class_attribute_flags_db); ++flag_i) {
					if ((ent->ca_flag & class_attribute_flags_db[flag_i].fe_flag) == 0)
						continue;
					if (!flag_first)
						DO(err, DeeFormat_PRINT(printer, arg, ","));
					DO(err, DeeFormat_PrintStr(printer, arg, class_attribute_flags_db[flag_i].fe_name));
					flag_first = false;
				}
				DO(err, DeeFormat_PRINT(printer, arg, "\""));
			}
			if (ent->ca_doc)
				DO(err, DeeFormat_Printf(printer, arg, ", %r", ent->ca_doc));
			DO(err, DeeFormat_PRINT(printer, arg, ")"));
		} else {
			/* Use address representation */
			DO(err, DeeFormat_Printf(printer, arg, "%#" PRFx16, ent->ca_addr));
		}
		is_first = false;
	}
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cd_printrepr(ClassDescriptor *__restrict self,
             Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "rt.ClassDescriptor(");
	if unlikely(result < 0)
		goto done;
	if (self->cd_name)
		DO(err, DeeFormat_Printf(printer, arg, "name: %r, ", self->cd_name));
	if (self->cd_doc)
		DO(err, DeeFormat_Printf(printer, arg, "doc: %r, ", self->cd_doc));
	if (self->cd_flags) {
		if unlikely(self->cd_flags & ~CLASS_FLAGS_DB_KNOWN) {
			/* Special case when unknown flags are present (shouldn't happen). */
			DO(err, DeeFormat_Printf(printer, arg, "flags: %#" PRFx16 ", ", self->cd_flags));
		} else {
			size_t i;
			bool is_first = true;
			DO(err, DeeFormat_PRINT(printer, arg, "flags: \""));
			for (i = 0; i < COMPILER_LENOF(class_flags_db); ++i) {
				if ((class_flags_db[i].fe_flag & self->cd_flags) == 0)
					continue;
				if (!is_first)
					DO(err, DeeFormat_PRINT(printer, arg, ","));
				DO(err, DeeFormat_PrintStr(printer, arg, class_flags_db[i].fe_name));
				is_first = false;
			}
			DO(err, DeeFormat_PRINT(printer, arg, "\", "));
		}
	}
	if (self->cd_clsop_list != empty_class_operators) {
		Dee_operator_t i;
		bool is_first = true;
		DO(err, DeeFormat_PRINT(printer, arg, "operators: { "));
		for (i = 0; i <= self->cd_clsop_mask; ++i) {
			struct opinfo const *info;
			struct class_operator op = self->cd_clsop_list[i];
			if (op.co_name == (Dee_operator_t)-1)
				continue;
			if (!is_first)
				DO(err, DeeFormat_PRINT(printer, arg, ", "));
			info = DeeTypeType_GetOperatorById(&DeeType_Type, op.co_name);
			if (info) {
				DO(err, DeeFormat_Printf(printer, arg, "%q", info->oi_sname));
			} else {
				DO(err, DeeFormat_Printf(printer, arg, "%#" PRFx16, op.co_name));
			}
			DO(err, DeeFormat_Printf(printer, arg, ": %#" PRFx16, op.co_addr));
			is_first = false;
		}
		DO(err, DeeFormat_PRINT(printer, arg, " }, "));
	}
	if (self->cd_iattr_mask > 0 || self->cd_iattr_list[0].ca_name != NULL) {
		DO(err, DeeFormat_PRINT(printer, arg, "iattr: { "));
		DO(err, xattr_table_printrepr(self->cd_iattr_list, self->cd_iattr_mask, printer, arg));
		DO(err, DeeFormat_PRINT(printer, arg, " }, "));
	}
	if (self->cd_cattr_list != empty_class_attributes) {
		DO(err, DeeFormat_PRINT(printer, arg, "cattr: { "));
		DO(err, xattr_table_printrepr(self->cd_cattr_list, self->cd_cattr_mask, printer, arg));
		DO(err, DeeFormat_PRINT(printer, arg, " }, "));
	}
	DO(err, DeeFormat_Printf(printer, arg, "isize: %#" PRFx16 ", csize: %#" PRFx16 ")",
	                         self->cd_imemb_size, self->cd_cmemb_size));
done:
	return result;
err:
	return temp;
}


/* NOTE: This function only initializes `ca_doc', `ca_addr' and `ca_flag' */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
class_attribute_init(struct class_attribute *__restrict self,
                     DeeObject *__restrict data,
                     bool is_class_attribute) {
	DREF DeeObject *addr_flags_doc[3];
	size_t nargs;
	/* ?Dint */
	/* ?T2?Dint?Dstring */
	/* ?T2?Dint?Dint */
	/* ?T3?Dint?Dstring?Dstring */
	/* ?T3?Dint?Dint?Dstring */
	if (DeeInt_Check(data)) {
		if (DeeInt_AsUInt16(data, &self->ca_addr))
			goto err;
		self->ca_doc  = NULL;
		self->ca_flag = CLASS_ATTRIBUTE_FPUBLIC;
		return 0;
	}
	nargs = DeeObject_InvokeMethodHint(seq_unpack_ex, data, 2, 3, addr_flags_doc);
	if unlikely(nargs == (size_t)-1)
		goto err;
#define LOCAL_addr  addr_flags_doc[0]
#define LOCAL_flags addr_flags_doc[1]
#define LOCAL_doc   addr_flags_doc[2]
	if (nargs <= 2) {
		LOCAL_doc = NULL;
	} else {
		if (DeeObject_AssertTypeExact(LOCAL_doc, &DeeString_Type))
			goto err_addr_flags_doc;
	}

	if (DeeObject_AsUInt16(LOCAL_addr, &self->ca_addr))
		goto err_addr_flags_doc;
	if (DeeString_Check(LOCAL_flags)) {
		char const *pos;
		self->ca_flag = 0;
		pos = DeeString_STR(LOCAL_flags);
		if (*pos) {
			for (;;) {
				char const *next;
				size_t flag_len;
				next     = strchr(pos, ',');
				flag_len = next ? (size_t)(next - pos) : strlen(pos);
				if likely(flag_len < COMPILER_LENOF(class_attribute_flags_db[0].fe_name)) {
					unsigned int i;
					for (i = 0; i < COMPILER_LENOF(class_attribute_flags_db); ++i) {
						if (class_attribute_flags_db[i].fe_name[flag_len] != '\0')
							continue;
						if (bcmpc(class_attribute_flags_db[i].fe_name, pos, flag_len, sizeof(char)) != 0)
							continue;
						self->ca_flag |= class_attribute_flags_db[i].fe_flag;
						goto got_flag;
					}
				}
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid flag %$q for %s-attribute %r",
				                flag_len, pos,
				                is_class_attribute ? STR_class : "instance",
				                self->ca_name);
				goto err_addr_flags_doc;
got_flag:
				if (!next)
					break;
				pos = next + 1;
			}
		}
	} else {
		if (DeeObject_AsUInt16(LOCAL_flags, &self->ca_flag))
			goto err_addr_flags_doc;
		if (self->ca_flag & ~CLASS_ATTRIBUTE_FMASK) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid flags for %s-attribute %r (0x%.4I16x)",
			                is_class_attribute ? STR_class : "instance",
			                self->ca_name, self->ca_flag);
			goto err_addr_flags_doc;
		}
	}
	self->ca_doc = (DREF struct string_object *)LOCAL_doc; /* Inherit reference. */
	Dee_Decref(LOCAL_flags);
	Dee_Decref(LOCAL_addr);
	return 0;
err_addr_flags_doc:
	Dee_XDecref(LOCAL_doc);
	Dee_Decref(LOCAL_flags);
	Dee_Decref(LOCAL_addr);
err:
	return -1;
#undef LOCAL_addr
#undef LOCAL_flags
#undef LOCAL_doc
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cd_rehash_cattr(ClassDescriptor *__restrict self) {
	struct class_attribute *new_map;
	size_t new_mask;
	new_mask = (self->cd_cattr_mask << 1) | 1;
	if (new_mask < 7)
		new_mask = 7;
	new_map = (struct class_attribute *)Dee_Callocc(new_mask + 1,
	                                                sizeof(struct class_attribute));
	if unlikely(!new_map)
		goto err;
	if (self->cd_cattr_list != empty_class_attributes) {
		/* Rehash the old table. */
		size_t i, j, perturb;
		for (i = 0; i <= self->cd_cattr_mask; ++i) {
			struct class_attribute *srcent;
			struct class_attribute *dstent;
			srcent = &self->cd_cattr_list[i];
			if (!srcent->ca_name)
				continue;
			j = perturb = srcent->ca_hash & new_mask;
			for (;; DeeClassDescriptor_CATTRNEXT(j, perturb)) {
				dstent = &new_map[j & new_mask];
				if (!dstent->ca_name)
					break;
			}
			memcpy(dstent, srcent, sizeof(struct class_attribute));
		}
		Dee_Free(self->cd_cattr_list);
	}
	self->cd_cattr_mask = new_mask;
	self->cd_cattr_list = new_map;
	return 0;
err:
	return -1;
}


struct cd_alloc_from_iattr_foreach_data {
	ClassDescriptor *cdafiaf_result;
	size_t           cdafiaf_iused;
	size_t           cdafiaf_imask;
	uint16_t         cdafiaf_imemb_size;
	uint16_t         cdafiaf_cmemb_size;
};

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
cd_alloc_from_iattr_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct class_attribute *ent;
	Dee_hash_t hash, j, perturb;
	struct cd_alloc_from_iattr_foreach_data *data;
	data = (struct cd_alloc_from_iattr_foreach_data *)arg;
	if (DeeObject_AssertType(key, &DeeString_Type))
		goto err;
	if (data->cdafiaf_iused >= (data->cdafiaf_imask / 3) * 2) {
		ClassDescriptor *new_result;
		size_t i, new_mask;
		new_mask   = (data->cdafiaf_imask << 1) | 1;
		new_result = (ClassDescriptor *)DeeObject_Callocc(offsetof(ClassDescriptor, cd_iattr_list),
		                                                  new_mask + 1, sizeof(struct class_attribute));
		if unlikely(!new_result)
			goto err;
		/* Rehash the already existing instance attribute table. */
		for (i = 0; i <= data->cdafiaf_imask; ++i) {
			if (!data->cdafiaf_result->cd_iattr_list[i].ca_name)
				continue;
			j = perturb = data->cdafiaf_result->cd_iattr_list[i].ca_hash & new_mask;
			for (;; DeeClassDescriptor_IATTRNEXT(j, perturb)) {
				ent = &new_result->cd_iattr_list[j & new_mask];
				if (ent->ca_name)
					continue;
				*ent = data->cdafiaf_result->cd_iattr_list[i];
				break;
			}
		}
		new_result->cd_imemb_size = data->cdafiaf_result->cd_imemb_size;
		new_result->cd_cmemb_size = data->cdafiaf_result->cd_cmemb_size;
		DeeObject_Free(data->cdafiaf_result);
		data->cdafiaf_result = new_result;
		data->cdafiaf_imask  = new_mask;
	}
	hash = DeeString_Hash(key);
	j = perturb = hash & data->cdafiaf_imask;
	for (;; DeeClassDescriptor_IATTRNEXT(j, perturb)) {
		ent = &data->cdafiaf_result->cd_iattr_list[j & data->cdafiaf_imask];
		if (!ent->ca_name)
			break;
		if (ent->ca_hash != hash)
			continue;
		if (!DeeString_EqualsSTR(ent->ca_name, key))
			continue;
		/* Duplicate attribute. */
		DeeError_Throwf(&DeeError_ValueError,
		                "Duplicate instance attribute %r",
		                key);
		goto err;
	}
	ent->ca_name = (DREF struct string_object *)key;
	ent->ca_hash = hash;
	if (class_attribute_init(ent, value, false))
		goto err_ent;
	Dee_Incref(key);
	++data->cdafiaf_iused;
	{
		uint16_t maxid = ent->ca_addr;
		if ((ent->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)) == CLASS_ATTRIBUTE_FGETSET)
			maxid += CLASS_GETSET_SET;
		if (ent->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			if (data->cdafiaf_cmemb_size != (uint16_t)-1 && maxid >= data->cdafiaf_cmemb_size) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Instance attribute %r uses out-of-bounds class "
				                "object table index %" PRFu16 " (>= %" PRFu16 ")",
				                ent->ca_name, maxid, data->cdafiaf_cmemb_size);
				goto err;
			}
			if (data->cdafiaf_result->cd_cmemb_size <= maxid)
				data->cdafiaf_result->cd_cmemb_size = maxid + 1;
		} else {
			if (data->cdafiaf_imemb_size != (uint16_t)-1 && maxid >= data->cdafiaf_imemb_size) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Instance attribute %r uses out-of-bounds object "
				                "table index %" PRFu16 " (>= %" PRFu16 ")",
				                ent->ca_name, maxid, data->cdafiaf_imemb_size);
				goto err;
			}
			if (data->cdafiaf_result->cd_imemb_size <= maxid)
				data->cdafiaf_result->cd_imemb_size = maxid + 1;
		}
	}
	return 0;
err_ent:
	ent->ca_name = NULL;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) ClassDescriptor *DCALL
cd_alloc_from_iattr(DeeObject *__restrict iattr,
                    uint16_t imemb_size,
                    uint16_t cmemb_size) {
	struct cd_alloc_from_iattr_foreach_data data;
	data.cdafiaf_imask = 7;
	data.cdafiaf_iused = 0;
	data.cdafiaf_imemb_size = imemb_size;
	data.cdafiaf_cmemb_size = cmemb_size;
	data.cdafiaf_result = (ClassDescriptor *)DeeObject_Callocc(offsetof(ClassDescriptor, cd_iattr_list),
	                                                           data.cdafiaf_imask + 1,
	                                                           sizeof(struct class_attribute));
	if unlikely(!data.cdafiaf_result)
		goto err;
	if (DeeObject_ForeachPair(iattr, &cd_alloc_from_iattr_foreach_cb, &data))
		goto err_r;
	data.cdafiaf_result->cd_iattr_mask = data.cdafiaf_imask;
	if (data.cdafiaf_imemb_size != (uint16_t)-1)
		data.cdafiaf_result->cd_imemb_size = data.cdafiaf_imemb_size;
	if (data.cdafiaf_cmemb_size != (uint16_t)-1)
		data.cdafiaf_result->cd_cmemb_size = data.cdafiaf_cmemb_size;
	return data.cdafiaf_result;
err_r:
	do {
		if (data.cdafiaf_result->cd_iattr_list[data.cdafiaf_imask].ca_name) {
			Dee_XDecref(data.cdafiaf_result->cd_iattr_list[data.cdafiaf_imask].ca_doc);
			Dee_Decref(data.cdafiaf_result->cd_iattr_list[data.cdafiaf_imask].ca_name);
		}
	} while (data.cdafiaf_imask--);
	DeeObject_Free(data.cdafiaf_result);
err:
	return NULL;
}

PRIVATE NONNULL((1, 4)) int DCALL
cd_add_operator(ClassDescriptor *__restrict self,
                Dee_operator_t name, uint16_t index,
                Dee_operator_t *__restrict operator_count) {
	Dee_operator_t i, perturb, mask;
	struct class_operator *map, *ent;
	if (name == (Dee_operator_t)-1)
		goto err_invalid_name;
	if (*operator_count >= (self->cd_clsop_mask / 3) * 2) {
		mask = (self->cd_clsop_mask << 1) | 1;
		if (mask < 3)
			mask = 3;
		map = (struct class_operator *)Dee_Mallocc(mask + 1, sizeof(struct class_operator));
		if unlikely(!map)
			goto err;
		memset(map, 0xff, (mask + 1) * sizeof(struct class_operator));
		for (i = 0; i <= self->cd_clsop_mask; ++i) {
			Dee_operator_t dst_i, dst_perturb;
			ent = &self->cd_clsop_list[i];
			if (ent->co_name == (Dee_operator_t)-1)
				continue;
			dst_i = dst_perturb = ent->co_name & mask;
			for (;; DeeClassDescriptor_CLSOPNEXT(dst_i, dst_perturb)) {
				struct class_operator *dst_ent = &map[dst_i & mask];
				if (dst_ent->co_name != (Dee_operator_t)-1)
					continue;
				*dst_ent = *ent;
				break;
			}
		}
		if (self->cd_clsop_list != empty_class_operators)
			Dee_Free(self->cd_clsop_list);
		self->cd_clsop_list = map;
		self->cd_clsop_mask = mask;
	}
	map  = self->cd_clsop_list;
	mask = self->cd_clsop_mask;
	i = perturb = name & mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		ent = &map[i & mask];
		if (ent->co_name == name)
			goto err_duplicate_name;
		if (ent->co_name == (Dee_operator_t)-1)
			break;
	}
	ent->co_name = name;
	ent->co_addr = index;
	++*operator_count;
	return 0;
	{
		struct opinfo const *op;
err_duplicate_name:
		op = DeeTypeType_GetOperatorById(&DeeType_Type, name);
		if (op) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Duplicate operator `%s'",
			                op->oi_sname);
		} else {
			DeeError_Throwf(&DeeError_ValueError,
			                "Duplicate operator `0x%.4I16x'",
			                name);
		}
	}
	goto err;
err_invalid_name:
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid operator name: 0xffff");
err:
	return -1;
}

struct cd_init_kw__cattr_foreach_data {
	ClassDescriptor *cdikcaf_result;
	size_t           cdikcaf_used_attr;
	uint16_t         cdikcaf_csize;
};

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
cd_init_kw__cattr_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct cd_init_kw__cattr_foreach_data *data;
	ClassDescriptor *result;
	struct class_attribute *ent;
	Dee_hash_t hash, i, perturb;
	data   = (struct cd_init_kw__cattr_foreach_data *)arg;
	result = data->cdikcaf_result;
	if (DeeObject_AssertType(key, &DeeString_Type))
		goto err;
	if (data->cdikcaf_used_attr >= (result->cd_cattr_mask / 3) * 2) {
		/* Rehash the class attribute table. */
		if (cd_rehash_cattr(result))
			goto err;
	}
	hash = DeeString_Hash(key);
	i = perturb = hash & result->cd_cattr_mask;
	for (;; DeeClassDescriptor_CATTRNEXT(i, perturb)) {
		ent = &result->cd_cattr_list[i & result->cd_cattr_mask];
		if (!ent->ca_name)
			break;
		if (ent->ca_hash != hash)
			continue;
		if (!DeeString_EqualsSTR(ent->ca_name, key))
			continue;
		DeeError_Throwf(&DeeError_ValueError,
		                "Duplicate class attribute %r",
		                key);
		goto err;
	}
	ent->ca_name = (DREF struct string_object *)key;
	ent->ca_hash = hash;
	if (class_attribute_init(ent, value, true))
		goto err_ent;
	Dee_Incref(key);
	++data->cdikcaf_used_attr;
	{
		uint16_t maxid;
		maxid = ent->ca_addr;
		if ((ent->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)) == CLASS_ATTRIBUTE_FGETSET)
			maxid += CLASS_GETSET_SET;
		if (data->cdikcaf_csize != (uint16_t)-1 && maxid >= data->cdikcaf_csize) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Class attribute %r uses out-of-bounds class "
			                "object table index %" PRFu16 " (>= %" PRFu16 ")",
			                ent->ca_name, maxid, data->cdikcaf_csize);
			goto err;
		}
		if (result->cd_cmemb_size <= maxid)
			result->cd_cmemb_size = maxid + 1;
	}
	return 0;
err_ent:
	ent->ca_name = NULL;
err:
	return -1;
}


struct cd_init_kw__operator_foreach_data {
	ClassDescriptor *cdikopf_result;
	Dee_operator_t   cdikopf_opcount;
	uint16_t         cdikopf_csize;
};

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
cd_init_kw__operator_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct cd_init_kw__operator_foreach_data *data;
	Dee_operator_t name, index;
	data = (struct cd_init_kw__operator_foreach_data *)arg;
	if (DeeString_Check(key)) {
		struct opinfo const *info;
		info = DeeTypeType_GetOperatorByName(&DeeType_Type, DeeString_STR(key), (size_t)-1);
		if (info == NULL) {
			/* TODO: In this case, must store the operator via its name
			 *       (so the name-query can happen in `DeeClass_New()') */
			DeeError_Throwf(&DeeError_ValueError,
			                "Unknown operator %r",
			                key);
			goto err;
		}
		name = info->oi_id;
	} else {
		if (DeeObject_AsUInt16(key, &name))
			goto err;
	}
	if (DeeObject_AsUInt16(value, &index))
		goto err;
	if (data->cdikopf_csize != (uint16_t)-1 && index >= data->cdikopf_csize) {
		struct opinfo const *op = DeeTypeType_GetOperatorById(&DeeType_Type, name);
		if (op) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Operator %s uses out-of-bounds class object "
			                "table index %" PRFu16 " (>= %" PRFu16 ")",
			                op->oi_sname, index, data->cdikopf_csize);
		} else {
			DeeError_Throwf(&DeeError_ValueError,
			                "Operator 0x%.4I16x uses out-of-bounds class "
			                "object table index %" PRFu16 " (>= %" PRFu16 ")",
			                name, index, data->cdikopf_csize);
		}
		goto err;
	}
	if (data->cdikopf_result->cd_cmemb_size <= index)
		data->cdikopf_result->cd_cmemb_size = index + 1;
	if (cd_add_operator(data->cdikopf_result, name, index, &data->cdikopf_opcount))
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED DREF ClassDescriptor *DCALL
cd_init_kw(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ClassDescriptor *result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("_ClassDescriptor", params: "
		DeeStringObject *name;
		DeeStringObject *doc   = (DeeStringObject *)Dee_EmptyString;
		DeeStringObject *flags = (DeeStringObject *)Dee_EmptyString;
		DeeObject *operators   = Dee_EmptyTuple;
		DeeObject *iattr       = Dee_EmptyTuple;
		DeeObject *cattr       = Dee_EmptyTuple;
		uint16_t isize         = (uint16_t)-1;
		uint16_t csize         = (uint16_t)-1;
");]]]*/
	struct {
		DeeStringObject *name;
		DeeStringObject *doc;
		DeeStringObject *flags;
		DeeObject *operators;
		DeeObject *iattr;
		DeeObject *cattr;
		uint16_t isize;
		uint16_t csize;
	} args;
	args.doc = (DeeStringObject *)Dee_EmptyString;
	args.flags = (DeeStringObject *)Dee_EmptyString;
	args.operators = Dee_EmptyTuple;
	args.iattr = Dee_EmptyTuple;
	args.cattr = Dee_EmptyTuple;
	args.isize = (uint16_t)-1;
	args.csize = (uint16_t)-1;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__name_doc_flags_operators_iattr_cattr_isize_csize, "o|ooooo" UNPx16 UNPx16 ":_ClassDescriptor", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertType(args.name, &DeeString_Type))
		goto err;
	if (DeeObject_AssertType(args.doc, &DeeString_Type))
		goto err;
	if (DeeString_IsEmpty(args.doc))
		args.doc = NULL;

	result = cd_alloc_from_iattr(args.iattr, args.isize, args.csize);
	if unlikely(!result)
		goto err;
	result->cd_flags = TP_FNORMAL;
	if (args.flags != (DeeStringObject *)Dee_EmptyString) {
		if (DeeString_Check(args.flags)) {
			char const *pos = DeeString_STR(args.flags);
			if (*pos) {
				for (;;) {
					char const *next;
					size_t flag_len;
					next     = strchr(pos, ',');
					flag_len = next ? (size_t)(next - pos) : strlen(pos);
					if likely(flag_len < COMPILER_LENOF(class_flags_db[0].fe_name)) {
						unsigned int i;
						for (i = 0; i < COMPILER_LENOF(class_flags_db); ++i) {
							if (class_flags_db[i].fe_name[flag_len] != '\0')
								continue;
							if (bcmpc(class_flags_db[i].fe_name, pos, flag_len, sizeof(char)) != 0)
								continue;
							result->cd_flags |= class_flags_db[i].fe_flag;
							goto got_flag;
						}
					}
					DeeError_Throwf(&DeeError_ValueError,
					                "Invalid class flag %$q",
					                flag_len, pos);
					goto err_r_imemb;
got_flag:
					if (!next)
						break;
					pos = next + 1;
				}
			}
		} else {
			if (DeeObject_AsUInt16((DeeObject *)args.flags, &result->cd_flags))
				goto err_r_imemb;
			if (result->cd_flags & ~(TP_FFINAL | TP_FINTERRUPT | TP_FINHERITCTOR |
			                         CLASS_TP_FAUTOINIT | CLASS_TP_FSUPERKWDS |
			                         TP_FTRUNCATE | TP_FMOVEANY)) {
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid set of class flags: 0x%.4I16x",
				                result->cd_flags);
				goto err_r_imemb;
			}
		}
	}
	result->cd_clsop_list = empty_class_operators;
	result->cd_clsop_mask = 0;
	result->cd_cattr_list = empty_class_attributes;
	result->cd_cattr_mask = 0;
	if (args.cattr != Dee_EmptyTuple) {
		struct cd_init_kw__cattr_foreach_data data;
		data.cdikcaf_result    = result;
		data.cdikcaf_used_attr = 0;
		data.cdikcaf_csize     = args.csize;
		if (DeeObject_ForeachPair(args.cattr, &cd_init_kw__cattr_foreach_cb, &data))
			goto err_r_imemb;
	}
	if (args.operators != Dee_EmptyTuple) {
		struct cd_init_kw__operator_foreach_data data;
		data.cdikopf_result  = result;
		data.cdikopf_opcount = 0;
		data.cdikopf_csize   = args.csize;
		if (DeeObject_ForeachPair(args.operators, &cd_init_kw__operator_foreach_cb, &data))
			goto err_r_imemb_cmemb;
	}
	result->cd_name = args.name;
	result->cd_doc  = args.doc;
	Dee_Incref(args.name);
	Dee_XIncref(args.doc);
	DeeObject_Init(result, &DeeClassDescriptor_Type);
	return result;
err_r_imemb_cmemb:
	if (result->cd_clsop_list != empty_class_operators)
		Dee_Free(result->cd_clsop_list);
	if (result->cd_cattr_list != empty_class_attributes) {
		do {
			if (result->cd_cattr_list[result->cd_cattr_mask].ca_name) {
				Dee_XDecref(result->cd_cattr_list[result->cd_cattr_mask].ca_doc);
				Dee_Decref(result->cd_cattr_list[result->cd_cattr_mask].ca_name);
			}
		} while (result->cd_cattr_mask--);
		Dee_Free(result->cd_cattr_list);
	}
err_r_imemb:
	do {
		if (result->cd_iattr_list[result->cd_iattr_mask].ca_name) {
			Dee_XDecref(result->cd_iattr_list[result->cd_iattr_mask].ca_doc);
			Dee_Decref(result->cd_iattr_list[result->cd_iattr_mask].ca_name);
		}
	} while (result->cd_iattr_mask--);
	DeeObject_Free(result);
err:
	return NULL;
}



PUBLIC DeeTypeObject DeeClassDescriptor_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassDescriptor",
	/* .tp_doc      = */ DOC("(name:?Dstring,doc:?Dstring=!P{},flags:?X2?Dstring?Dint=!P{},"
	                         /**/ "operators:?S?T2?X2?Dstring?Dint?Dint=!T0,"
	                         /**/ "iattr:?S?T2?Dstring?X3?Dint?T2?Dint?X2?Dstring?Dint?T3?Dint?X2?Dstring?Dint?Dstring=!T0,"
	                         /**/ "cattr:?S?T2?Dstring?X3?Dint?T2?Dint?X2?Dstring?Dint?T3?Dint?X2?Dstring?Dint?Dstring=!T0,"
	                         /**/ "isize?:?Dint,csize?:?Dint)\n"
	                         "#tValueError{Some operator or attribute was defined multiple times}"
	                         "#tValueError{A specified operator name wasn't recognized (custom operators must be encoded as IDs)}"
	                         "#tValueError{A specified set of flags contains an invalid option}"
	                         "#tValueError{An attribute or operator is bound to an out-of-bounds object table index}"
	                         "#tIntergerOverflow{A used object table index exceeds the hard limit of $0xffff (unsigned 16-bit)}"
	                         "Create a new class descriptor\n"
	                         "The given @flags is a comma-separated string of flags as described in ?#flags\n"
	                         "The given @isize and @csize determine the allocated sizes of the instance class "
	                         /**/ "member tables. - When omitted, these sizes are automatically calculated by "
	                         /**/ "determining the greatest used table indices within @operators, @iattr and @cattr\n"
	                         "Note that both @iattr and @cattr take mappings of attribute names to either "
	                         /**/ "the associated table_index, or a tuple of (table_index, flags[, doc]), where flags is "
	                         /**/ "a comma-separated string of flags as described in ?Aflags?#Attribute\n"
	                         "Hint: Once created, a _ClassDescriptor object can be used "
	                         /**/ "with ?Ert:makeclass to create custom class types at runtime"),
	/* .tp_flags    = */ TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &cd_init_kw,
			/* tp_serialize:   */ &cd_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cd_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cd_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cd_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cd_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &cd_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cd_methods,
	/* .tp_getsets       = */ cd_getsets,
	/* .tp_members       = */ cd_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cd_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



typedef struct {
	PROXY_OBJECT_HEAD(ot_owner)    /* [1..1][const] The associated owner object.
	                                * NOTE: This may be a super-object, in which case the referenced
	                                *       object table refers to the described super-type. */
	struct instance_desc *ot_desc; /* [1..1][valid_if(ot_size != 0)][const] The referenced instance descriptor. */
	uint16_t              ot_size; /* [const] The length of the object table contained within `ot_desc' */
} ObjectTable;

STATIC_ASSERT(offsetof(ObjectTable, ot_owner) == offsetof(ProxyObject, po_obj));
#define ot_fini  generic_proxy__fini
#define ot_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ot_serialize(ObjectTable *__restrict self,
             DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	ObjectTable *out = DeeSerial_Addr2Mem(writer, addr, ObjectTable);
	out->ot_size = self->ot_size;
#define ADDROF(field) (addr + offsetof(ObjectTable, field))
	if (generic_proxy__serialize((ProxyObject *)self, writer, addr))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(ot_desc), self->ot_desc);
#undef ADDROF
err:
	return -1;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeTypeObject *DCALL
ot_type(ObjectTable *__restrict self) {
	DeeTypeObject *result;
	result = DeeType_Check(self->ot_owner)
	         ? (DeeTypeObject *)self->ot_owner
	         : Dee_TYPE(self->ot_owner);
	if (result == &DeeSuper_Type)
		result = DeeSuper_TYPE(self->ot_owner);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ot_print(ObjectTable *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DeeTypeObject *tp = ot_type(self);
	if (DeeType_Check(self->ot_owner))
		return DeeFormat_Printf(printer, arg, "<class object table for %k>", tp);
	return DeeFormat_Printf(printer, arg, "<object table for instance of %k>", tp);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ot_printrepr(ObjectTable *__restrict self, Dee_formatprinter_t printer, void *arg) {
	DeeTypeObject *tp = ot_type(self);
	if (DeeType_Check(self->ot_owner))
		return DeeFormat_Printf(printer, arg, "%r.__ctable__", tp);
	return DeeFormat_Printf(printer, arg, "%r.__itable__", tp);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ot_bool(ObjectTable *__restrict self) {
	return self->ot_size != 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
ot_size(ObjectTable *__restrict self) {
	return self->ot_size;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_getitem_index(ObjectTable *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->ot_size)
		goto err_index;
	Dee_instance_desc_lock_read(self->ot_desc);
	result = self->ot_desc->id_vtab[index];
	if unlikely(!result)
		goto err_unbound;
	Dee_Incref(result);
	Dee_instance_desc_lock_endread(self->ot_desc);
	return result;
err_unbound:
	Dee_instance_desc_lock_endread(self->ot_desc);
	DeeRT_ErrUnboundIndex(self, index);
	return NULL;
err_index:
	DeeRT_ErrIndexOutOfBounds(self, index, self->ot_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_getitem_index_fast(ObjectTable *__restrict self, size_t index) {
	DREF DeeObject *result;
	ASSERT(index < self->ot_size);
	Dee_instance_desc_lock_read(self->ot_desc);
	result = self->ot_desc->id_vtab[index];
	Dee_XIncref(result);
	Dee_instance_desc_lock_endread(self->ot_desc);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ot_delitem_index(ObjectTable *__restrict self, size_t index) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->ot_size)
		goto err_index;
	Dee_instance_desc_lock_write(self->ot_desc);
	oldval = self->ot_desc->id_vtab[index];
	self->ot_desc->id_vtab[index] = NULL;
	Dee_instance_desc_lock_endwrite(self->ot_desc);
	Dee_XDecref(oldval);
	return 0;
err_index:
	return DeeRT_ErrIndexOutOfBounds(self, index, self->ot_size);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
ot_setitem_index(ObjectTable *self, size_t index, DeeObject *value) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->ot_size)
		goto err_index;
	Dee_Incref(value);
	Dee_instance_desc_lock_write(self->ot_desc);
	oldval = self->ot_desc->id_vtab[index];
	self->ot_desc->id_vtab[index] = value;
	Dee_instance_desc_lock_endwrite(self->ot_desc);
	Dee_XDecref(oldval);
	return 0;
err_index:
	return DeeRT_ErrIndexOutOfBounds(self, index, self->ot_size);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ot_bounditem_index(ObjectTable *__restrict self, size_t index) {
	DeeObject *value;
	if unlikely(index >= self->ot_size)
		return Dee_BOUND_MISSING;
	value = Dee_atomic_read_with_atomic_rwlock(&self->ot_desc->id_vtab[index],
	                                           &self->ot_desc->id_lock);
	return Dee_BOUND_FROMBOOL(value);
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
ot_mh_seq_xchitem_index(ObjectTable *self, size_t index, DeeObject *newval) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->ot_size)
		goto err_index;
	Dee_instance_desc_lock_write(self->ot_desc);
	oldval = self->ot_desc->id_vtab[index];
	if unlikely(!oldval)
		goto err_unbound;
	Dee_Incref(newval);
	self->ot_desc->id_vtab[index] = newval;
	Dee_instance_desc_lock_endwrite(self->ot_desc);
	return oldval;
err_unbound:
	Dee_instance_desc_lock_endwrite(self->ot_desc);
	DeeRT_ErrUnboundIndex(self, index);
	goto err;
err_index:
	DeeRT_ErrIndexOutOfBounds(self, index, self->ot_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ot_mh_seq_enumerate_index(ObjectTable *self, Dee_seq_enumerate_index_t proc,
                          void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (end > self->ot_size)
		end = self->ot_size;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
		Dee_instance_desc_lock_read(self->ot_desc);
		elem = self->ot_desc->id_vtab[i];
		if (!elem) {
			Dee_instance_desc_lock_endread(self->ot_desc);
			continue;
		}
		Dee_Incref(elem);
		Dee_instance_desc_lock_endread(self->ot_desc);
		temp = (*proc)(arg, i, elem);
		Dee_Decref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_method tpconst ot_methods[] = {
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst ot_method_hints[] = {
	TYPE_METHOD_HINT(seq_xchitem_index, &ot_mh_seq_xchitem_index),
	TYPE_METHOD_HINT(seq_enumerate_index, &ot_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq ot_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem                    = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__size__and__getitem_index_fast),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&ot_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&ot_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ot_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ot_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ot_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ot_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&ot_bounditem_index,
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__size__and__getitem_index_fast),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
ot_gettype(ObjectTable *__restrict self) {
	DREF DeeTypeObject *result = ot_type(self);
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassDescriptor *DCALL
ot_getclass(ObjectTable *__restrict self) {
	DeeTypeObject *type = ot_type(self);
	ClassDescriptor *result = DeeClass_DESC(type)->cd_desc;
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_isctable(ObjectTable *__restrict self) {
	return_bool(DeeType_Check(self->ot_owner)); /* ot_type(self) == self->ot_owner */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_isitable(ObjectTable *__restrict self) {
	return_bool(!DeeType_Check(self->ot_owner)); /* ot_type(self) != self->ot_owner */
}

PRIVATE struct type_getset tpconst ot_getsets[] = {
	TYPE_GETTER_AB_F(STR___type__, &ot_gettype,
	                 METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?DType\n"
	                 "The type describing @this object table"),
	TYPE_GETTER_AB_F("__class__", &ot_getclass,
	                 METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?Ert:ClassDescriptor\n\n"
	                 "Same as ${this.__type__.__class__}"),
	TYPE_GETTER_AB_F("__isctable__", &ot_isctable,
	                 METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?Dbool\n\n"
	                 "Evaluates to ?t if @this is a class object table"),
	TYPE_GETTER_AB_F("__isitable__", &ot_isitable,
	                 METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?Dbool\n\n"
	                 "Evaluates to ?t if @this is an instance object table"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ot_members[] = {
	TYPE_MEMBER_FIELD_DOC("__owner__", STRUCT_OBJECT, offsetof(ObjectTable, ot_owner),
	                      "The object that owns @this object table"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
ot_init(ObjectTable *__restrict self,
        size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	DeeTypeObject *type = NULL;
	struct class_desc *desc;
	DeeArg_Unpack1Or2(err, argc, argv, "_ObjectTable", &ob, &type);
	if (type) {
		if (DeeObject_AssertImplements(ob, type))
			goto err;
	} else {
		type = Dee_TYPE(ob);
		if (type == &DeeSuper_Type) {
			type = DeeSuper_TYPE(ob);
			ob   = DeeSuper_SELF(ob);
		}
	}
	if (DeeType_IsTypeType(type)) {
		if (!DeeType_IsClass(ob))
			goto err_no_class;
		/* Class member table. */
		desc          = DeeClass_DESC(ob);
		self->ot_desc = class_desc_as_instance(desc);
		self->ot_size = desc->cd_desc->cd_cmemb_size;
	} else {
		if (!DeeType_IsClass(type))
			goto err_no_class;
		/* Instance member table. */
		desc          = DeeClass_DESC(type);
		self->ot_desc = DeeInstance_DESC(desc, ob);
		self->ot_size = desc->cd_desc->cd_imemb_size;
	}
	self->ot_owner = Dee_AsObject(ob);
	Dee_Incref(ob);
	return 0;
err_no_class:
	DeeError_Throwf(&DeeError_TypeError,
	                "Type `%k' isn't a class",
	                type);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ot_copy(ObjectTable *__restrict self, ObjectTable *__restrict other) {
	self->ot_owner = other->ot_owner;
	self->ot_desc  = other->ot_desc;
	self->ot_size  = other->ot_size;
	Dee_Incref(self->ot_owner);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ot_deepcopy(ObjectTable *__restrict self, ObjectTable *__restrict other) {
	ptrdiff_t offset;
	if (DeeType_Check(other->ot_owner))
		return ot_copy(self, other);
	self->ot_owner = DeeObject_DeepCopy(other->ot_owner);
	if unlikely(!self->ot_owner)
		goto err;
	offset = (byte_t *)other->ot_desc - (byte_t *)other->ot_owner;
	self->ot_desc = (struct instance_desc *)((byte_t *)self->ot_owner + offset);
	self->ot_size = other->ot_size;
	return 0;
err:
	return -1;
}


INTERN DeeTypeObject ObjectTable_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ObjectTable",
	/* .tp_doc      = */ DOC("(ob:?X2?O?DType,typ?:?DType)\n"
	                         "#tTypeError{The given @ob isn't a class or class instance}"
	                         "Load the object member table of a class, or class instance"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ ObjectTable,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ot_copy,
			/* tp_deep_ctor:   */ &ot_deepcopy,
			/* tp_any_ctor:    */ &ot_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ot_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ot_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&ot_bool,
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ot_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ot_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ot_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__7EA181D4706D1525),
	/* .tp_seq           = */ &ot_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ot_methods,
	/* .tp_getsets       = */ ot_getsets,
	/* .tp_members       = */ ot_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ ot_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_ctable(DeeTypeObject *__restrict self) {
	DREF ObjectTable *result;
	struct class_desc *desc;
	if (!DeeType_IsClass(self))
		return DeeSeq_NewEmpty();
	desc   = DeeClass_DESC(self);
	result = DeeObject_MALLOC(ObjectTable);
	if unlikely(!result)
		goto done;
	result->ot_owner = Dee_AsObject(self);
	result->ot_desc  = class_desc_as_instance(desc);
	result->ot_size  = desc->cd_desc->cd_cmemb_size;
	Dee_Incref(self);
	DeeObject_Init(result, &ObjectTable_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instance_get_itable(DeeObject *__restrict self) {
	DREF ObjectTable *result;
	struct class_desc *desc;
	DeeObject *real_self = self;
	DeeTypeObject *type  = Dee_TYPE(self);
	if (type == &DeeSuper_Type) {
		type      = DeeSuper_TYPE(self);
		real_self = DeeSuper_SELF(self);
	}
	if (!DeeType_IsClass(type))
		return DeeSeq_NewEmpty();
	desc   = DeeClass_DESC(type);
	result = DeeObject_MALLOC(ObjectTable);
	if unlikely(!result)
		goto done;
	result->ot_owner = Dee_AsObject(self);
	result->ot_desc  = DeeInstance_DESC(desc, real_self);
	result->ot_size  = desc->cd_desc->cd_imemb_size;
	Dee_Incref(self);
	DeeObject_Init(result, &ObjectTable_Type);
done:
	return Dee_AsObject(result);
}





PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get(DeeInstanceMemberObject *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("get", params: """
	DeeObject *thisarg;
""", docStringPrefix: "instancemember");]]]*/
#define instancemember_get_params "thisarg"
	struct {
		DeeObject *thisarg;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__thisarg, "o:get", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeOrAbstract(args.thisarg, self->im_type))
		goto err;
	desc = DeeClass_DESC(self->im_type);
	return DeeInstance_GetAttribute(desc,
	                                DeeInstance_DESC(desc, args.thisarg),
	                                args.thisarg,
	                                self->im_attribute);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_delete(DeeInstanceMemberObject *self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("delete", params: """
	DeeObject *thisarg;
""", docStringPrefix: "instancemember");]]]*/
#define instancemember_delete_params "thisarg"
	struct {
		DeeObject *thisarg;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__thisarg, "o:delete", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeOrAbstract(args.thisarg, self->im_type))
		goto err;
	desc = DeeClass_DESC(self->im_type);
	if (DeeInstance_DelAttribute(desc,
	                             DeeInstance_DESC(desc, args.thisarg),
	                             args.thisarg,
	                             self->im_attribute))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_set(DeeInstanceMemberObject *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("set", params: """
	DeeObject *thisarg;
	DeeObject *value;
""", docStringPrefix: "instancemember");]]]*/
#define instancemember_set_params "thisarg,value"
	struct {
		DeeObject *thisarg;
		DeeObject *value;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__thisarg_value, "oo:set", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeOrAbstract(args.thisarg, self->im_type))
		goto err;
	desc = DeeClass_DESC(self->im_type);
	if (DeeInstance_SetAttribute(desc,
	                             DeeInstance_DESC(desc, args.thisarg),
	                             args.thisarg,
	                             self->im_attribute,
	                             args.value))
		goto err;
	return_none;
err:
	return NULL;
}


STATIC_ASSERT(offsetof(DeeInstanceMemberObject, im_type) == offsetof(ClassAttribute, ca_desc));
STATIC_ASSERT(offsetof(DeeInstanceMemberObject, im_attribute) == offsetof(ClassAttribute, ca_attr));
STATIC_ASSERT(sizeof(DeeInstanceMemberObject) == sizeof(ClassAttribute));
#define instancemember_copy      ca_copy
#define instancemember_serialize ca_serialize

STATIC_ASSERT(offsetof(DeeInstanceMemberObject, im_type) == offsetof(ProxyObject, po_obj));
#define instancemember_fini  generic_proxy__fini
#define instancemember_visit generic_proxy__visit

PRIVATE struct type_method tpconst instancemember_methods[] = {
	TYPE_KWMETHOD_F(STR_get, &instancemember_get, METHOD_FNOREFESCAPE,
	                "(" instancemember_get_params ")->\n"
	                "Return the @thisarg's value of @this member"),
	TYPE_KWMETHOD_F("delete", &instancemember_delete, METHOD_FNOREFESCAPE,
	                "(" instancemember_delete_params ")\n"
	                "Delete @thisarg's value of @this member"),
	TYPE_KWMETHOD_F(STR_set, &instancemember_set, METHOD_FNOREFESCAPE,
	                "(" instancemember_set_params ")\n"
	                "Set @thisarg's value of @this member to @value"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_module(DeeInstanceMemberObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->im_type);
	if (result)
		return result;
	DeeRT_ErrTUnboundAttr(&DeeInstanceMember_Type, self, &str___module__);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_name(DeeInstanceMemberObject *__restrict self) {
	return_reference(self->im_attribute->ca_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_doc(DeeInstanceMemberObject *__restrict self) {
	if (!self->im_attribute->ca_doc)
		return_none;
	return_reference(self->im_attribute->ca_doc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_canget(DeeInstanceMemberObject *__restrict self) {
	if ((self->im_attribute->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) ==
	    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) {
		if (!self->im_type->tp_class->cd_members[self->im_attribute->ca_addr + CLASS_GETSET_GET])
			return_false;
	}
	return_true;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_candel(DeeInstanceMemberObject *__restrict self) {
	if (self->im_attribute->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		return_false;
	if ((self->im_attribute->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) ==
	    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) {
		if (!self->im_type->tp_class->cd_members[self->im_attribute->ca_addr + CLASS_GETSET_DEL])
			return_false;
	}
	return_true;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_canset(DeeInstanceMemberObject *__restrict self) {
	if (self->im_attribute->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		return_false;
	if ((self->im_attribute->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) ==
	    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) {
		if (!self->im_type->tp_class->cd_members[self->im_attribute->ca_addr + CLASS_GETSET_SET])
			return_false;
	}
	return_true;
}

PRIVATE struct type_getset tpconst instancemember_getsets[] = {
	TYPE_GETTER_AB_F("canget", &instancemember_get_canget, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this member can be read from"),
	TYPE_GETTER_AB_F("candel", &instancemember_get_candel, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this member can be deleted"),
	TYPE_GETTER_AB_F("canset", &instancemember_get_canset, METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this member can be written to"),
	TYPE_GETTER_AB_F(STR___name__, &instancemember_get_name, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "The name of @this instance member"),
	TYPE_GETTER_AB_F(STR___doc__, &instancemember_get_doc, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "The documentation string associated with @this instance member"),
	TYPE_GETTER_AB_F(STR___module__, &instancemember_get_module, METHOD_FNOREFESCAPE,
	                 "->?DModule\n"
	                 "#t{UnboundAttribute}"
	                 "Returns the module that is defining @this instance member"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst instancemember_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___type__, STRUCT_OBJECT, offsetof(DeeInstanceMemberObject, im_type), "->?DType"),
	TYPE_MEMBER_END
};

/* NOTE: Must also hash and compare the type because if the class is
 *       created multiple times, then the member descriptor remains
 *       the same and is shared between all instances. */
PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
instancemember_hash(DeeInstanceMemberObject *__restrict self) {
	return Dee_HashCombine(Dee_HashPointer(self->im_type),
	                       Dee_HashPointer(self->im_attribute));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
instancemember_compare_eq(DeeInstanceMemberObject *self,
                          DeeInstanceMemberObject *other) {
	if (DeeObject_AssertType(other, &DeeInstanceMember_Type))
		goto err;
	return (self->im_type == other->im_type &&
	        self->im_attribute == other->im_attribute)
	       ? 0
	       : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
instancemember_trycompare_eq(DeeInstanceMemberObject *self,
                             DeeInstanceMemberObject *other) {
	return (DeeObject_InstanceOf(other, &DeeInstanceMember_Type) &&
	        self->im_type == other->im_type &&
	        self->im_attribute == other->im_attribute)
	       ? 0
	       : 1;
}

PRIVATE struct type_cmp instancemember_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&instancemember_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&instancemember_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&instancemember_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
instancemember_print(DeeInstanceMemberObject *__restrict self,
                     Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<InstanceMember %k.%k>",
	                        self->im_type, self->im_attribute->ca_name);
}

INTDEF WUNUSED NONNULL((1)) bool DCALL
DeeString_IsSymbol(DeeStringObject *__restrict self,
                   size_t start_index,
                   size_t end_index);

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
instancemember_printrepr(DeeInstanceMemberObject *__restrict self,
                         Dee_formatprinter_t printer, void *arg) {
	DeeStringObject *name = (DeeStringObject *)self->im_attribute->ca_name;
	if (DeeString_IsSymbol(name, 0, (size_t)-1)) {
		return DeeFormat_Printf(printer, arg, "%r.%k", self->im_type, name);
	} else {
		return DeeFormat_Printf(printer, arg, "%r.operator . (%r)", self->im_type, name);
	}
}

PRIVATE struct type_callable instancemember_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&instancemember_get,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PUBLIC DeeTypeObject DeeInstanceMember_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_InstanceMember",
	/* .tp_doc      = */ DOC("call(" instancemember_get_params ")->"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeInstanceMemberObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &instancemember_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &instancemember_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&instancemember_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&instancemember_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&instancemember_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&instancemember_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &instancemember_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ instancemember_methods,
	/* .tp_getsets       = */ instancemember_getsets,
	/* .tp_members       = */ instancemember_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&default__call__with__call_kw),
	/* .tp_callable      = */ &instancemember_callable,
};


PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeInstanceMember_New(DeeTypeObject *__restrict class_type,
                      struct class_attribute const *__restrict attr) {
	DREF DeeInstanceMemberObject *result;
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	result = DeeObject_MALLOC(DeeInstanceMemberObject);
	if unlikely(!result)
		goto done;
	result->im_type      = class_type;
	result->im_attribute = attr;
	Dee_Incref(class_type);
	DeeObject_Init(result, &DeeInstanceMember_Type);
done:
	return Dee_AsObject(result);
}


struct class_ciattriter {
	Dee_ATTRITER_HEAD
	DeeTypeObject *cciai_class; /* [1..1][const] Class whose instance attributes to iterate. */
	Dee_hash_t     cciai_index; /* [lock(ATOMIC)] Index into `cciai_desc->cd_iattr_list' (valid if `<= cciai_desc->cd_iattr_mask') */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
class_ciattriter_next(struct class_ciattriter *__restrict self,
                      /*out*/ struct Dee_attrdesc *__restrict desc) {
	struct class_desc *my_class     = DeeClass_DESC(self->cciai_class);
	DeeClassDescriptorObject *cdesc = my_class->cd_desc;
	Dee_hash_t old_index, new_index;
	struct class_attribute *attr;
	uint16_t perm;
	do {
		old_index = atomic_read(&self->cciai_index);
		new_index = old_index;
		for (;;) {
			if (new_index > cdesc->cd_iattr_mask)
				return 1;
			attr = &cdesc->cd_iattr_list[new_index];
			++new_index;
			if (attr->ca_name)
				break;
		}
	} while (!atomic_cmpxch(&self->cciai_index, old_index, new_index));
	perm = (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_WRAPPER |
	        Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_DOCOBJ);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= Dee_ATTRPERM_F_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= Dee_ATTRPERM_F_CANCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= Dee_ATTRPERM_F_PRIVATE;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		Dee_class_desc_lock_read(my_class);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			/* Special case: Figure out what property callbacks have been assigned. */
			perm &= ~Dee_ATTRPERM_F_CANGET;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
				perm |= Dee_ATTRPERM_F_CANGET;
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
					perm |= Dee_ATTRPERM_F_CANDEL;
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
					perm |= Dee_ATTRPERM_F_CANSET;
			}
		} else {
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY))
				perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		}
		Dee_class_desc_lock_endread(my_class);
	} else {
		if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)))
			perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
	}

	Dee_Incref(attr->ca_name);
	desc->ad_name = DeeString_STR(attr->ca_name);
	desc->ad_doc  = NULL;
	if (attr->ca_doc) {
		Dee_Incref(attr->ca_doc);
		desc->ad_doc = DeeString_STR(attr->ca_doc);
	}
	desc->ad_perm = perm;
	desc->ad_info.ai_decl = Dee_AsObject(self->cciai_class);
	desc->ad_info.ai_type = Dee_ATTRINFO_INSTANCE_ATTR;
	desc->ad_info.ai_value.v_instance_attr = attr;
	desc->ad_type = NULL;
	return 0;
}

PRIVATE struct Dee_attriter_type tpconst class_ciattriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&class_ciattriter_next,
};


INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeClass_IterClassInstanceAttributes(DeeTypeObject *__restrict self,
                                     struct Dee_attriter *iterbuf, size_t bufsize) {
	struct class_ciattriter *iter = (struct class_ciattriter *)iterbuf;
	if (bufsize >= sizeof(struct class_ciattriter)) {
		iter->cciai_class = self;
		iter->cciai_index = 0;
		Dee_attriter_init(iter, &class_ciattriter_type);
	}
	return sizeof(struct class_ciattriter);
}




struct class_cattriter {
	Dee_ATTRITER_HEAD
	DeeTypeObject *ccai_class; /* [1..1][const] Class whose class attributes to iterate. */
	Dee_hash_t     ccai_index; /* [lock(ATOMIC)] Index into `ccai_desc->cd_iattr_list' (valid if `<= ccai_desc->cd_iattr_mask') */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
class_cattriter_next(struct class_cattriter *__restrict self,
                     /*out*/ struct Dee_attrdesc *__restrict desc) {
	struct class_desc *my_class     = DeeClass_DESC(self->ccai_class);
	DeeClassDescriptorObject *cdesc = my_class->cd_desc;
	Dee_hash_t old_index, new_index;
	struct class_attribute *attr;
	uint16_t perm;
	do {
		old_index = atomic_read(&self->ccai_index);
		new_index = old_index;
		for (;;) {
			if (new_index > cdesc->cd_cattr_mask)
				return 1;
			attr = &cdesc->cd_cattr_list[new_index];
			++new_index;
			if (attr->ca_name)
				break;
		}
	} while (!atomic_cmpxch(&self->ccai_index, old_index, new_index));

	perm = (Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_CANGET |
	        Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_DOCOBJ);
	/* Figure out which instance descriptor the property is connected to. */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm = (Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_NAMEOBJ);
			/* Actually figure out what callbacks are assigned. */
			Dee_class_desc_lock_read(my_class);
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
				perm |= Dee_ATTRPERM_F_CANGET;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
				perm |= Dee_ATTRPERM_F_CANDEL;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
				perm |= Dee_ATTRPERM_F_CANSET;
			Dee_class_desc_lock_endread(my_class);
		}
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= Dee_ATTRPERM_F_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= Dee_ATTRPERM_F_CANCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= Dee_ATTRPERM_F_PRIVATE;
	Dee_Incref(attr->ca_name);
	desc->ad_name = DeeString_STR(attr->ca_name);
	desc->ad_doc  = NULL;
	if (attr->ca_doc) {
		Dee_Incref(attr->ca_doc);
		desc->ad_doc = DeeString_STR(attr->ca_doc);
	}
	desc->ad_perm = perm;
	desc->ad_info.ai_decl = Dee_AsObject(self->ccai_class);
	desc->ad_info.ai_type = Dee_ATTRINFO_ATTR;
	desc->ad_info.ai_value.v_instance_attr = attr;
	desc->ad_type = NULL;
	return 0;
}

PRIVATE struct Dee_attriter_type tpconst class_cattriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&class_cattriter_next,
};

INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
DeeClass_IterClassAttributes(DeeTypeObject *__restrict self,
                             struct Dee_attriter *iterbuf, size_t bufsize) {
	struct class_cattriter *iter = (struct class_cattriter *)iterbuf;
	if (bufsize >= sizeof(struct class_cattriter)) {
		iter->ccai_class = self;
		iter->ccai_index = 0;
		Dee_attriter_init(iter, &class_cattriter_type);
	}
	return sizeof(struct class_cattriter);
}






struct class_iattriter {
	Dee_ATTRITER_HEAD
	DeeTypeObject *ciai_class; /* [1..1][const] Class whose class attributes to iterate. */
	Dee_hash_t     ciai_index; /* [lock(ATOMIC)] Index into `ciai_desc->cd_iattr_list' (valid if `<= ciai_desc->cd_iattr_mask') */
	DeeObject     *ciai_inst;  /* [0..1][const] Class instance. */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
class_iattriter_next(struct class_iattriter *__restrict self,
                     /*out*/ struct Dee_attrdesc *__restrict desc) {
	struct class_desc *my_class     = DeeClass_DESC(self->ciai_class);
	DeeClassDescriptorObject *cdesc = my_class->cd_desc;
	Dee_hash_t old_index, new_index;
	struct class_attribute *attr;
	struct instance_desc *inst;
	uint16_t perm;
	do {
		old_index = atomic_read(&self->ciai_index);
		new_index = old_index;
		for (;;) {
			if (new_index > cdesc->cd_iattr_mask)
				return 1;
			attr = &cdesc->cd_iattr_list[new_index];
			++new_index;
			if (attr->ca_name)
				break;
		}
	} while (!atomic_cmpxch(&self->ciai_index, old_index, new_index));
	perm = (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET |
	        Dee_ATTRPERM_F_NAMEOBJ | Dee_ATTRPERM_F_DOCOBJ);
	/* Figure out which instance descriptor the property is connected to. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		inst = class_desc_as_instance(my_class);
	} else if (self->ciai_inst) {
		inst = DeeInstance_DESC(my_class, self->ciai_inst);
	} else {
		inst = NULL;
	}
	if (inst)
		Dee_instance_desc_lock_read(inst);
	if (inst && !(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		/* Actually figure out the type of the attr. */
		if (!inst->id_vtab[attr->ca_addr])
			perm &= ~Dee_ATTRPERM_F_CANGET;
	}
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		if (inst && attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm = (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_NAMEOBJ);
			/* Actually figure out what callbacks are assigned. */
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
				perm |= Dee_ATTRPERM_F_CANGET;
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
				perm |= Dee_ATTRPERM_F_CANDEL;
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
				perm |= Dee_ATTRPERM_F_CANSET;
		}
	}
	if (inst)
		Dee_instance_desc_lock_endread(inst);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= Dee_ATTRPERM_F_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= Dee_ATTRPERM_F_CANCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= Dee_ATTRPERM_F_PRIVATE;
	Dee_Incref(attr->ca_name);
	desc->ad_name = DeeString_STR(attr->ca_name);
	desc->ad_doc  = NULL;
	if (attr->ca_doc) {
		Dee_Incref(attr->ca_doc);
		desc->ad_doc = DeeString_STR(attr->ca_doc);
	}
	desc->ad_perm = perm;
	desc->ad_info.ai_decl = Dee_AsObject(self->ciai_class);
	desc->ad_info.ai_type = Dee_ATTRINFO_ATTR;
	desc->ad_info.ai_value.v_instance_attr = attr;
	desc->ad_type = NULL;
	return 0;
}

PRIVATE struct Dee_attriter_type tpconst class_iattriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&class_iattriter_next,
};

INTERN WUNUSED NONNULL((1, 3)) size_t DCALL
DeeClass_IterInstanceAttributes(DeeTypeObject *__restrict self, DeeObject *instance,
                                struct Dee_attriter *iterbuf, size_t bufsize) {
	struct class_iattriter *iter = (struct class_iattriter *)iterbuf;
	if (bufsize >= sizeof(struct class_iattriter)) {
		iter->ciai_class = self;
		iter->ciai_index = 0;
		iter->ciai_inst  = instance;
		Dee_attriter_init(iter, &class_iattriter_type);
	}
	return sizeof(struct class_iattriter);
}

/* Find a specific class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeClass_FindClassAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self,
                            struct Dee_attrspec const *__restrict specs,
                            struct Dee_attrdesc *__restrict result) {
	uint16_t perm;
	struct class_attribute *attr;
	struct class_desc *my_class = DeeClass_DESC(self);
	attr = DeeType_QueryClassAttributeStringHash(tp_invoker, self,
	                                             specs->as_name,
	                                             specs->as_hash);
	if (!attr)
		goto not_found;
	perm = (Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_NAMEOBJ);
	/* Figure out which instance descriptor the property is connected to. */
	Dee_class_desc_lock_read(my_class);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm = (Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_NAMEOBJ);
			/* Actually figure out what callbacks are assigned. */
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
				perm |= Dee_ATTRPERM_F_CANGET;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
				perm |= Dee_ATTRPERM_F_CANDEL;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
				perm |= Dee_ATTRPERM_F_CANSET;
		}
	}
	Dee_class_desc_lock_endread(my_class);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= Dee_ATTRPERM_F_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= Dee_ATTRPERM_F_CANCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= Dee_ATTRPERM_F_PRIVATE;
	if ((perm & specs->as_perm_mask) != specs->as_perm_value) {
not_found:
		return 1;
	}
	result->ad_name = DeeString_STR(attr->ca_name);
	result->ad_doc  = NULL;
	result->ad_perm = perm | Dee_ATTRPERM_F_NAMEOBJ;
	result->ad_info.ai_decl = Dee_AsObject(self);
	result->ad_info.ai_type = Dee_ATTRINFO_ATTR;
	result->ad_info.ai_value.v_attr = attr;
	Dee_Incref(attr->ca_name);
	if (attr->ca_doc) {
		result->ad_doc = DeeString_STR(attr->ca_doc);
		result->ad_perm |= Dee_ATTRPERM_F_DOCOBJ;
		Dee_Incref(attr->ca_doc);
	}
	result->ad_type = NULL;
	return 0;
}


/* Find a specific instance-through-class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeClass_FindClassInstanceAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self,
                                    struct Dee_attrspec const *__restrict specs,
                                    struct Dee_attrdesc *__restrict result) {
	uint16_t perm;
	struct class_attribute *attr;
	struct class_desc *my_class = DeeClass_DESC(self);
	attr = DeeType_QueryInstanceAttributeStringHash(tp_invoker, self,
	                                                specs->as_name,
	                                                specs->as_hash);
	if (!attr)
		goto not_found;
	perm = (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER |
	        Dee_ATTRPERM_F_WRAPPER | Dee_ATTRPERM_F_CANGET |
	        Dee_ATTRPERM_F_NAMEOBJ);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= Dee_ATTRPERM_F_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= Dee_ATTRPERM_F_CANCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= Dee_ATTRPERM_F_PRIVATE;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		Dee_class_desc_lock_read(my_class);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			/* Special case: Figure out what property callbacks have been assigned. */
			perm &= ~Dee_ATTRPERM_F_CANGET;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
				perm |= Dee_ATTRPERM_F_CANGET;
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
					perm |= Dee_ATTRPERM_F_CANDEL;
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
					perm |= Dee_ATTRPERM_F_CANSET;
			}
		} else {
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY))
				perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		}
		Dee_class_desc_lock_endread(my_class);
	} else {
		if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)))
			perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
	}
	if ((perm & specs->as_perm_mask) != specs->as_perm_value) {
not_found:
		return 1;
	}
	result->ad_name = DeeString_STR(attr->ca_name);
	result->ad_doc  = NULL;
	result->ad_perm = perm | Dee_ATTRPERM_F_NAMEOBJ;
	result->ad_info.ai_decl = Dee_AsObject(self);
	result->ad_info.ai_type = Dee_ATTRINFO_INSTANCE_ATTR;
	result->ad_info.ai_value.v_instance_attr = attr;
	Dee_Incref(attr->ca_name);
	if (attr->ca_doc) {
		result->ad_doc = DeeString_STR(attr->ca_doc);
		result->ad_perm |= Dee_ATTRPERM_F_DOCOBJ;
		Dee_Incref(attr->ca_doc);
	}
	result->ad_type = NULL;
	return 0;
}

/* Find a specific instance-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 4, 5)) int DCALL
DeeClass_FindInstanceAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self, DeeObject *instance,
                               struct Dee_attrspec const *__restrict specs,
                               struct Dee_attrdesc *__restrict result) {
	uint16_t perm;
	struct class_attribute *attr;
	struct instance_desc *inst;
	struct class_desc *my_class = DeeClass_DESC(self);
	attr = DeeType_QueryAttributeStringHash(tp_invoker, self,
	                                        specs->as_name,
	                                        specs->as_hash);
	if (!attr)
		goto not_found;
	perm = (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_NAMEOBJ);

	/* Figure out which instance descriptor the property is connected to. */
	inst = NULL;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		inst = class_desc_as_instance(my_class);
	} else if (instance) {
		inst = DeeInstance_DESC(my_class, instance);
	}
	if (inst)
		Dee_instance_desc_lock_read(inst);
	if (inst && !(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		/* Actually figure out the type of the attr. */
		if (!inst->id_vtab[attr->ca_addr])
			perm &= ~Dee_ATTRPERM_F_CANGET;
	}
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		if (inst && attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm = (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_NAMEOBJ);
			/* Actually figure out what callbacks are assigned. */
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
				perm |= Dee_ATTRPERM_F_CANGET;
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
				perm |= Dee_ATTRPERM_F_CANDEL;
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
				perm |= Dee_ATTRPERM_F_CANSET;
		}
	}
	if (inst)
		Dee_instance_desc_lock_endread(inst);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= Dee_ATTRPERM_F_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= Dee_ATTRPERM_F_CANCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= Dee_ATTRPERM_F_PRIVATE;
	if ((perm & specs->as_perm_mask) != specs->as_perm_value) {
not_found:
		return 1;
	}
	result->ad_name = DeeString_STR(attr->ca_name);
	result->ad_doc  = NULL;
	result->ad_perm = perm | Dee_ATTRPERM_F_NAMEOBJ;
	result->ad_info.ai_decl = Dee_AsObject(self);
	result->ad_info.ai_type = Dee_ATTRINFO_ATTR;
	result->ad_info.ai_value.v_attr = attr;
	Dee_Incref(attr->ca_name);
	if (attr->ca_doc) {
		result->ad_doc = DeeString_STR(attr->ca_doc);
		result->ad_perm |= Dee_ATTRPERM_F_DOCOBJ;
		Dee_Incref(attr->ca_doc);
	}
	result->ad_type = NULL;
	return 0;
}






INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_GetInstanceAttribute(DeeTypeObject *__restrict class_type,
                              struct class_attribute const *__restrict attr) {
	DREF DeePropertyObject *result;
	struct class_desc *my_class;

	/* Return an instance-wrapper for instance-members. */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
		return DeeInstanceMember_New(class_type, attr);
	my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		DREF DeeObject *member_value;

		/* Simple case: direct access to unbound class-based attr. */
		Dee_class_desc_lock_read(my_class);
		member_value = my_class->cd_members[attr->ca_addr];
		Dee_XIncref(member_value);
		Dee_class_desc_lock_endread(my_class);
		if unlikely(!member_value)
			goto unbound;
		return member_value;
	}
	result = DeeObject_MALLOC(DeePropertyObject);
	if unlikely(!result)
		goto err;
	result->p_del = NULL;
	result->p_set = NULL;
	Dee_class_desc_lock_read(my_class);
	result->p_get = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
	Dee_XIncref(result->p_get);

	/* Only non-readonly property members have callbacks other than a getter.
	 * In this case, the readonly flag both protects the property from being
	 * overwritten, as well as being invoked using something other than read-access. */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		result->p_del = my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL];
		Dee_XIncref(result->p_del);
		result->p_set = my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET];
		Dee_XIncref(result->p_set);
	}
	Dee_class_desc_lock_endread(my_class);

	/* Make sure that at least a single property callback has been assigned.
	 * If not, raise an unbound-attr error. */
	if unlikely(!result->p_get && !result->p_del && !result->p_set) {
		DeeObject_FREE(result);
		goto unbound;
	}

	/* Finalize initialization of the property wrapper and return it. */
	DeeObject_Init(result, &DeeProperty_Type);
	return Dee_AsObject(result);
unbound:
	DeeRT_ErrUnboundInstanceAttrCA(class_type, attr);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeClass_BoundInstanceAttribute(DeeTypeObject *__restrict class_type,
                                struct class_attribute const *__restrict attr) {
	bool result;
	struct class_desc *my_class;

	/* Return an instance-wrapper for instance-members. */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* instance-members outside of class memory are
		 * accessed through wrappers, which are always bound. */
		return Dee_BOUND_YES;
	}
	my_class = DeeClass_DESC(class_type);

	/* Check if the member is assigned. */
	Dee_class_desc_lock_read(my_class);
	if ((attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)) ==
	    CLASS_ATTRIBUTE_FGETSET) {
		result = ((my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] != NULL) ||
		          (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] != NULL) ||
		          (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] != NULL));
	} else {
		result = my_class->cd_members[attr->ca_addr] != NULL;
	}
	Dee_class_desc_lock_endread(my_class);
	return Dee_BOUND_FROMBOOL(result);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttribute(DeeTypeObject *class_type,
                               struct class_attribute const *__restrict attr,
                               size_t argc, DeeObject *const *argv) {
	DREF DeeObject *callback;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
		if (argc != 1) {
			DeeError_Throwf(&DeeError_TypeError,
			                "instancemember `%k' must be called with exactly 1 argument",
			                attr->ca_name);
			goto err;
		}
		if (DeeObject_AssertTypeOrAbstract(argv[0], class_type))
			goto err;
		return DeeInstance_GetAttribute(my_class,
		                                DeeInstance_DESC(my_class, argv[0]),
		                                argv[0],
		                                attr);
	}
	/* Simple case: direct access to unbound class-based attr. */
#if 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		/* Calling an instance property using the class as base
		 * will simply invoke the getter associated with that property.
		 * Technically, we could assert that `argc == 1' at this point,
		 * as well as that `argv[0] is class_type', but there is no
		 * need for us to do this, as the callback that's going to be
		 * invoked will perform those same check (should that guaranty
		 * become relevant), because it's yet another object over which
		 * the user has full control. */
	}
#endif
	Dee_class_desc_lock_read(my_class);
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
	} else {
		callback = my_class->cd_members[attr->ca_addr];
	}
#else /* CLASS_GETSET_GET != 0 */
	callback = my_class->cd_members[attr->ca_addr];
#endif /* CLASS_GETSET_GET == 0 */
	Dee_XIncref(callback);
	Dee_class_desc_lock_endread(my_class);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	return DeeObject_CallInherited(callback, argc, argv);
unbound:
	DeeRT_ErrUnboundInstanceAttrCA(class_type, attr);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeKw(DeeTypeObject *class_type,
                                 struct class_attribute const *__restrict attr,
                                 size_t argc, DeeObject *const *argv,
                                 DeeObject *kw) {
	DREF DeeObject *callback;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("get", params: """
	DeeObject *thisarg;
""");]]]*/
	struct {
		DeeObject *thisarg;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__thisarg, "o:get", &args))
		goto err;
/*[[[end]]]*/
		if (DeeObject_AssertTypeOrAbstract(args.thisarg, class_type))
			goto err;
		return DeeInstance_GetAttribute(my_class,
		                                DeeInstance_DESC(my_class, args.thisarg),
		                                args.thisarg,
		                                attr);
	}
	/* Simple case: direct access to unbound class-based attr. */
#if 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		/* Calling an instance property using the class as base
		 * will simply invoke the getter associated with that property.
		 * Technically, we could assert that `argc == 1' at this point,
		 * as well as that `argv[0] is class_type', but there is no
		 * need for us to do this, as the callback that's going to be
		 * invoked will perform those same check (should that guaranty
		 * become relevant), because it's yet another object over which
		 * the user has full control. */
	}
#endif
	Dee_class_desc_lock_read(my_class);
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
	} else {
		callback = my_class->cd_members[attr->ca_addr];
	}
#else /* CLASS_GETSET_GET != 0 */
	callback = my_class->cd_members[attr->ca_addr];
#endif /* CLASS_GETSET_GET == 0 */
	Dee_XIncref(callback);
	Dee_class_desc_lock_endread(my_class);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	return DeeObject_CallKwInherited(callback, argc, argv, kw);
unbound:
	DeeRT_ErrUnboundInstanceAttrCA(class_type, attr);
err:
	return NULL;
}

#if defined(CONFIG_CALLTUPLE_OPTIMIZATIONS) || defined(__DEEMON__)
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeTuple(DeeTypeObject *class_type,
                                    struct class_attribute const *__restrict attr,
                                    DeeObject *args) {
	DREF DeeObject *callback;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
		if (DeeTuple_SIZE(args) != 1) {
			DeeError_Throwf(&DeeError_TypeError,
			                "instancemember `%k' must be called with exactly 1 argument",
			                attr->ca_name);
			goto err;
		}
		if (DeeObject_AssertTypeOrAbstract(DeeTuple_GET(args, 0), class_type))
			goto err;
		return DeeInstance_GetAttribute(my_class,
		                                DeeInstance_DESC(my_class, DeeTuple_GET(args, 0)),
		                                DeeTuple_GET(args, 0),
		                                attr);
	}
	/* Simple case: direct access to unbound class-based attr. */
#if 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		/* Calling an instance property using the class as base
		 * will simply invoke the getter associated with that property.
		 * Technically, we could assert that `argc == 1' at this point,
		 * as well as that `DeeTuple_GET(args, 0) is class_type', but there is no
		 * need for us to do this, as the callback that's going to be
		 * invoked will perform those same check (should that guaranty
		 * become relevant), because it's yet another object over which
		 * the user has full control. */
	}
#endif
	Dee_class_desc_lock_read(my_class);
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
	} else {
		callback = my_class->cd_members[attr->ca_addr];
	}
#else /* CLASS_GETSET_GET != 0 */
	callback = my_class->cd_members[attr->ca_addr];
#endif /* CLASS_GETSET_GET == 0 */
	Dee_XIncref(callback);
	Dee_class_desc_lock_endread(my_class);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	return DeeObject_CallTupleInherited(callback, args);
unbound:
	DeeRT_ErrUnboundInstanceAttrCA(class_type, attr);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeTupleKw(DeeTypeObject *class_type,
                                      struct class_attribute const *__restrict attr,
                                      DeeObject *args, DeeObject *kw) {
	DREF DeeObject *callback, *result;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("get", params: """
	DeeObject *thisarg;
""", argc: "DeeTuple_SIZE(args)", argv: "DeeTuple_ELEM(args)", args: "u_args");]]]*/
	struct {
		DeeObject *thisarg;
	} u_args;
	if (DeeArg_UnpackStructKw(DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw, kwlist__thisarg, "o:get", &u_args))
		goto err;
/*[[[end]]]*/
		if (DeeObject_AssertTypeOrAbstract(u_args.thisarg, class_type))
			goto err;
		return DeeInstance_GetAttribute(my_class,
		                                DeeInstance_DESC(my_class, u_args.thisarg),
		                                u_args.thisarg,
		                                attr);
	}
	/* Simple case: direct access to unbound class-based attr. */
#if 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		/* Calling an instance property using the class as base
		 * will simply invoke the getter associated with that property.
		 * Technically, we could assert that `argc == 1' at this point,
		 * as well as that `DeeTuple_GET(args, 0) is class_type', but there is no
		 * need for us to do this, as the callback that's going to be
		 * invoked will perform those same check (should that guaranty
		 * become relevant), because it's yet another object over which
		 * the user has full control. */
	}
#endif
	Dee_class_desc_lock_read(my_class);
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
	} else {
		callback = my_class->cd_members[attr->ca_addr];
	}
#else /* CLASS_GETSET_GET != 0 */
	callback = my_class->cd_members[attr->ca_addr];
#endif /* CLASS_GETSET_GET == 0 */
	Dee_XIncref(callback);
	Dee_class_desc_lock_endread(my_class);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	result = DeeObject_CallTupleKw(callback, args, kw);
	Dee_Decref(callback);
	return result;
unbound:
	DeeRT_ErrUnboundInstanceAttrCA(class_type, attr);
err:
	return NULL;
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeClass_VCallInstanceAttributef(DeeTypeObject *class_type,
                                 struct class_attribute const *__restrict attr,
                                 char const *__restrict format, va_list args) {
	DREF DeeObject *callback, *args_tuple;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
		DeeObject *thisarg;
		DREF DeeObject *result;
		args_tuple = DeeTuple_VNewf(format, args);
		if unlikely(!args_tuple)
			goto err;
		DeeArg_Unpack1(err_args_tuple,
		                DeeTuple_SIZE(args_tuple),
		                DeeTuple_ELEM(args_tuple),
		                "get", &thisarg);
		if (DeeObject_AssertTypeOrAbstract(thisarg, class_type))
			goto err_args_tuple;
		result = DeeInstance_GetAttribute(my_class,
		                                  DeeInstance_DESC(my_class, thisarg),
		                                  thisarg,
		                                  attr);
		Dee_Decref(args_tuple);
		return result;
	}
	/* Simple case: direct access to unbound class-based attr. */
#if 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		/* Calling an instance property using the class as base
		 * will simply invoke the getter associated with that property.
		 * Technically, we could assert that `argc == 1' at this point,
		 * as well as that `argv[0] is class_type', but there is no
		 * need for us to do this, as the callback that's going to be
		 * invoked will perform those same check (should that guaranty
		 * become relevant), because it's yet another object over which
		 * the user has full control. */
	}
#endif
	Dee_class_desc_lock_read(my_class);
#if CLASS_GETSET_GET != 0
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
	} else {
		callback = my_class->cd_members[attr->ca_addr];
	}
#else /* CLASS_GETSET_GET != 0 */
	callback = my_class->cd_members[attr->ca_addr];
#endif /* CLASS_GETSET_GET == 0 */
	Dee_XIncref(callback);
	Dee_class_desc_lock_endread(my_class);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	return DeeObject_VCallInheritedf(callback, format, args);
unbound:
	DeeRT_ErrUnboundInstanceAttrCA(class_type, attr);
	goto err;
err_args_tuple:
	Dee_Decref(args_tuple);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeClass_DelInstanceAttribute(DeeTypeObject *__restrict class_type,
                              struct class_attribute const *__restrict attr) {
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if unlikely(!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
		goto err_noaccess;
	/* Make sure not to re-write readonly attributes. */
	if unlikely(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		goto err_noaccess;
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		DREF DeeObject *old_value;
		/* Simple case: directly delete a class-based attr. */
		Dee_class_desc_lock_write(my_class);
		old_value = my_class->cd_members[attr->ca_addr];
		my_class->cd_members[attr->ca_addr] = NULL;
		Dee_class_desc_lock_endwrite(my_class);
		Dee_XDecref(old_value);
	} else {
		/* Property callbacks (delete all bindings, rather than 1) */
		unsigned int i;
		DREF DeeObject *old_value[CLASS_GETSET_COUNT];
		Dee_class_desc_lock_write(my_class);
		for (i = 0; i < CLASS_GETSET_COUNT; ++i) {
			old_value[i] = my_class->cd_members[attr->ca_addr + i];
			my_class->cd_members[attr->ca_addr + i] = NULL;
		}
		Dee_class_desc_lock_endwrite(my_class);
		Dee_XDecref(old_value[2]);
		Dee_XDecref(old_value[1]);
		Dee_XDecref(old_value[0]);
	}
	return 0;
err_noaccess:
	return DeeRT_ErrCRestrictedAttrCA(class_type, attr, DeeRT_ATTRIBUTE_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeClass_SetInstanceAttribute(DeeTypeObject *class_type,
                              struct class_attribute const *__restrict attr,
                              DeeObject *value) {
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if unlikely(!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
		goto err_noaccess;
	/* Make sure not to re-write readonly attributes. */
	if unlikely(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		goto err_noaccess;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *old_values[3];
		if (DeeObject_AssertType(value, &DeeProperty_Type))
			goto err;
		/* Unpack and assign a property wrapper.
		 * NOTE: Because only properties with the read-only flag can get away
		 *       with only a getter VTABLE slot, we can assume that this property
		 *       has 3 slots because we're not allowed to override readonly properties. */
		Dee_XIncref(((DeePropertyObject *)value)->p_get);
		Dee_XIncref(((DeePropertyObject *)value)->p_del);
		Dee_XIncref(((DeePropertyObject *)value)->p_set);
		Dee_class_desc_lock_write(my_class);
		old_values[0] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
		old_values[1] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL];
		old_values[2] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET];
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] = ((DeePropertyObject *)value)->p_get;
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] = ((DeePropertyObject *)value)->p_del;
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] = ((DeePropertyObject *)value)->p_set;
		Dee_class_desc_lock_endwrite(my_class);

		/* Drop references from the old callbacks. */
		Dee_XDecref(old_values[2]);
		Dee_XDecref(old_values[1]);
		Dee_XDecref(old_values[0]);
	} else {
		/* Simple case: direct overwrite an unbound class-based attr. */
		DREF DeeObject *old_value;
		Dee_Incref(value);
		Dee_class_desc_lock_write(my_class);
		old_value = my_class->cd_members[attr->ca_addr];
		my_class->cd_members[attr->ca_addr] = value;
		Dee_class_desc_lock_endwrite(my_class);
		Dee_XDecref(old_value); /* Decref the old value. */
	}
	return 0;
err_noaccess:
	DeeRT_ErrCRestrictedAttrCA(class_type, attr, DeeRT_ATTRIBUTE_ACCESS_SET);
err:
	return -1;
}




PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_GetAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct class_attribute const *__restrict attr) {
	DREF DeeObject *result;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto illegal;

		/* Invoke the getter. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
			return DeeObject_ThisCallInherited(getter, this_arg, 0, NULL);
		return DeeObject_CallInherited(getter, 0, NULL);
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		/* Construct a thiscall function. */
		DREF DeeObject *callback;
		Dee_instance_desc_lock_read(self);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!callback)
			goto unbound;
		Dee_Incref(this_arg);
		return DeeInstanceMethod_NewInherited(callback, this_arg);
	} else {
		/* Simply return the attribute as-is. */
		Dee_instance_desc_lock_read(self);
		result = self->id_vtab[attr->ca_addr];
		Dee_XIncref(result);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!result)
			goto unbound;
	}
	return result;
unbound:
	return DeeRT_ErrCUnboundAttrCA(this_arg, attr);
illegal:
	DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_GET);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_BoundAttribute(struct class_desc *__restrict desc,
                           struct instance_desc *__restrict self,
                           DeeObject *__restrict this_arg,
                           struct class_attribute const *__restrict attr) {
	DREF DeeObject *result;
	DREF DeeObject *value;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto unbound;

		/* Invoke the getter. */
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCallInherited(getter, this_arg, 0, NULL)
		         : DeeObject_CallInherited(getter, 0, NULL);
		if likely(result) {
			Dee_Decref(result);
			return Dee_BOUND_YES;
		}
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return Dee_BOUND_NO;
		return Dee_BOUND_ERR;
	} else {
		/* Simply return the attribute as-is. */
		value = Dee_atomic_read_with_atomic_rwlock(&self->id_vtab[attr->ca_addr],
		                                           &self->id_lock);
		return value != NULL;
	}
unbound:
	return Dee_BOUND_NO;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_CallAttribute(struct class_desc *__restrict desc,
                          struct instance_desc *__restrict self,
                          DeeObject *this_arg,
                          struct class_attribute const *__restrict attr,
                          size_t argc, DeeObject *const *argv) {
	DREF DeeObject *callback;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto illegal;

		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCallInherited(getter, this_arg, 0, NULL)
		           : DeeObject_CallInherited(getter, 0, NULL);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		return DeeObject_CallInherited(callback, argc, argv);
	}

	/* Call the attr as-is. */
	Dee_instance_desc_lock_read(self);
	callback = self->id_vtab[attr->ca_addr];
	Dee_XIncref(callback);
	Dee_instance_desc_lock_endread(self);
	if unlikely(!callback)
		goto unbound;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		return DeeObject_ThisCallInherited(callback, this_arg, argc, argv);
	return DeeObject_CallInherited(callback, argc, argv);
unbound:
	return DeeRT_ErrCUnboundAttrCA(this_arg, attr);
illegal:
	DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
DeeInstance_VCallAttributef(struct class_desc *__restrict desc,
                            struct instance_desc *__restrict self,
                            DeeObject *this_arg,
                            struct class_attribute const *__restrict attr,
                            char const *__restrict format, va_list args) {
	DREF DeeObject *callback;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto illegal;

		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCallInherited(getter, this_arg, 0, NULL)
		           : DeeObject_CallInherited(getter, 0, NULL);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		return DeeObject_VCallInheritedf(callback, format, args);
	}

	/* Call the attr as-is. */
	Dee_instance_desc_lock_read(self);
	callback = self->id_vtab[attr->ca_addr];
	Dee_XIncref(callback);
	Dee_instance_desc_lock_endread(self);
	if unlikely(!callback)
		goto unbound;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		return DeeObject_VThisCallInheritedf(callback, this_arg, format, args);
	return DeeObject_VCallInheritedf(callback, format, args);
unbound:
	return DeeRT_ErrCUnboundAttrCA(this_arg, attr);
illegal:
	DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_CallAttributeKw(struct class_desc *__restrict desc,
                            struct instance_desc *__restrict self,
                            DeeObject *this_arg,
                            struct class_attribute const *__restrict attr,
                            size_t argc, DeeObject *const *argv,
                            DeeObject *kw) {
	DREF DeeObject *callback;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto err_illegal;

		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCallInherited(getter, this_arg, 0, NULL)
		           : DeeObject_CallInherited(getter, 0, NULL);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		return DeeObject_CallKwInherited(callback, argc, argv, kw);
	}

	/* Call the attr as-is. */
	Dee_instance_desc_lock_read(self);
	callback = self->id_vtab[attr->ca_addr];
	Dee_XIncref(callback);
	Dee_instance_desc_lock_endread(self);
	if unlikely(!callback)
		goto err_unbound;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		return DeeObject_ThisCallKwInherited(callback, this_arg, argc, argv, kw);
	return DeeObject_CallKwInherited(callback, argc, argv, kw);
err_unbound:
	return DeeRT_ErrCUnboundAttrCA(this_arg, attr);
err_illegal:
	DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *
(DCALL DeeInstance_CallAttributeTuple)(struct class_desc *__restrict desc,
                                       struct instance_desc *__restrict self,
                                       DeeObject *this_arg,
                                       struct class_attribute const *__restrict attr,
                                       DeeObject *args) {
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	DREF DeeObject *callback;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto illegal;

		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCallInherited(getter, this_arg, 0, NULL)
		           : DeeObject_CallInherited(getter, 0, NULL);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		return DeeObject_CallTupleInherited(callback, args);
	}

	/* Call the attr as-is. */
	Dee_instance_desc_lock_read(self);
	callback = self->id_vtab[attr->ca_addr];
	Dee_XIncref(callback);
	Dee_instance_desc_lock_endread(self);
	if unlikely(!callback)
		goto unbound;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		return DeeObject_ThisCallTupleInherited(callback, this_arg, args);
	return DeeObject_CallTupleInherited(callback, args);
unbound:
	return DeeRT_ErrCUnboundAttrCA(this_arg, attr);
illegal:
	DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	/* For binary compatibility */
	return DeeInstance_CallAttribute(desc, self, this_arg, attr,
	                                 DeeTuple_SIZE(args),
	                                 DeeTuple_ELEM(args));
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *
(DCALL DeeInstance_CallAttributeTupleKw)(struct class_desc *__restrict desc,
                                         struct instance_desc *__restrict self,
                                         DeeObject *this_arg,
                                         struct class_attribute const *__restrict attr,
                                         DeeObject *args, DeeObject *kw) {
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	DREF DeeObject *result, *callback;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto illegal;

		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCallInherited(getter, this_arg, 0, NULL)
		           : DeeObject_CallInherited(getter, 0, NULL);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		result = DeeObject_CallTupleKw(callback, args, kw);
		Dee_Decref(callback);
	} else {
		/* Call the attr as-is. */
		Dee_instance_desc_lock_read(self);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!callback)
			goto unbound;
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCallTupleKw(callback, this_arg, args, kw)
		         : DeeObject_CallTupleKw(callback, args, kw);
		Dee_Decref(callback);
	}
	return result;
unbound:
	return DeeRT_ErrCUnboundAttrCA(this_arg, attr);
illegal:
	DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	/* For binary compatibility */
	return DeeInstance_CallAttributeKw(desc, self, this_arg, attr,
	                                   DeeTuple_SIZE(args),
	                                   DeeTuple_ELEM(args),
	                                   kw);
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */
}


PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_DelAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct class_attribute const *__restrict attr) {
	ASSERT_OBJECT(this_arg);

	/* Make sure that the access is allowed. */
	if unlikely(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		goto illegal;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *delfun, *temp;
		Dee_instance_desc_lock_read(self);
		delfun = self->id_vtab[attr->ca_addr + CLASS_GETSET_DEL];
		Dee_XIncref(delfun);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!delfun)
			goto illegal;

		/* Invoke the getter. */
		temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		       ? DeeObject_ThisCallInherited(delfun, this_arg, 0, NULL)
		       : DeeObject_CallInherited(delfun, 0, NULL);
		if unlikely(!temp)
			goto err;
		Dee_Decref_unlikely(temp); /* *_unlikely because it's probably "none" */
	} else {
		DREF DeeObject *old_value;

		/* Simply unbind the field in the attr table. */
		Dee_instance_desc_lock_write(self);
		old_value = self->id_vtab[attr->ca_addr];
		self->id_vtab[attr->ca_addr] = NULL;
		Dee_instance_desc_lock_endwrite(self);
		Dee_XDecref(old_value);
	}
	return 0;
illegal:
	return DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_DEL);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
DeeInstance_SetAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *this_arg,
                         struct class_attribute const *__restrict attr,
                         DeeObject *value) {
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *setter, *temp;

		/* Make sure that the access is allowed. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			goto illegal;
		Dee_instance_desc_lock_read(self);
		setter = self->id_vtab[attr->ca_addr + CLASS_GETSET_SET];
		Dee_XIncref(setter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!setter)
			goto illegal;

		/* Invoke the getter. */
		temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		       ? DeeObject_ThisCallInherited(setter, this_arg, 1, (DeeObject **)&value)
		       : DeeObject_CallInherited(setter, 1, (DeeObject **)&value);
		if unlikely(!temp)
			goto err;
		Dee_Decref_unlikely(temp); /* *_unlikely because it's probably "none" */
	} else {
		DREF DeeObject *old_value;

		/* Simply override the field in the attr table. */
		Dee_instance_desc_lock_write(self);
		old_value = self->id_vtab[attr->ca_addr];
		if (old_value && (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
			Dee_instance_desc_lock_endwrite(self);
			goto illegal; /* readonly fields can only be set once. */
		} else {
			Dee_Incref(value);
			self->id_vtab[attr->ca_addr] = value;
		}
		Dee_instance_desc_lock_endwrite(self);

		/* Drop a reference from the old value. */
		Dee_XDecref(old_value);
	}
	return 0;
illegal:
	return DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_SET);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL DeeInstance_SetBasicAttribute)(struct class_desc *__restrict desc,
                                      struct instance_desc *__restrict self,
                                      struct class_attribute const *__restrict attr,
                                      DeeObject *__restrict value) {
	DREF DeeObject *old_value;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
		goto illegal; /* Not a basic attribute. */

	/* Simply override the field in the attr table. */
	Dee_Incref(value);
	Dee_instance_desc_lock_write(self);
	old_value = self->id_vtab[attr->ca_addr];
#if 0 /* Special case: `this = default' constructors are allowed to override default initializers of "final" members! */
	if (old_value && (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		Dee_instance_desc_lock_endwrite(self);
		goto illegal;
	}
#endif
	self->id_vtab[attr->ca_addr] = value;
	Dee_instance_desc_lock_endwrite(self);
	Dee_XDecref(old_value);
	return 0;
	{
		DeeObject *instance;
illegal:
		instance = (DeeObject *)((byte_t *)self - desc->cd_offset);
		return DeeRT_ErrRestrictedAttrCADuringInitialization(Dee_TYPE(instance), attr);
	}
}



/* Check if the current execution context allows access to `self',
 * which is either an instance or class method of `impl_class' */
INTERN WUNUSED NONNULL((1, 2)) bool DCALL
class_attribute_mayaccess_impl(struct class_attribute *__restrict self,
                               DeeTypeObject *__restrict impl_class) {
	struct code_frame *caller_frame;
	ASSERT_OBJECT_TYPE(impl_class, &DeeType_Type);
	ASSERT(DeeType_IsClass(impl_class));
	ASSERT(self->ca_flag & CLASS_ATTRIBUTE_FPRIVATE);

	/* Only allow access if the calling code-frame originates from
	 * a this-call who's this-argument derives from `class_type'. */
	caller_frame = DeeThread_Self()->t_exec;
	if (!caller_frame ||
	    !(caller_frame->cf_func->fo_code->co_flags & CODE_FTHISCALL))
		return false;
	return DeeType_Extends(DeeObject_Class(caller_frame->cf_this),
	                       impl_class);
}


/* Lookup class / instance attributes within the given class descriptor.
 * @return: * :   A pointer to attribute that was found.
 * @return: NULL: Attribute could not be found (no error is thrown) */
PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryClassAttributeHash(DeeClassDescriptorObject *self,
                                           /*String*/ DeeObject *name, Dee_hash_t hash) {
	struct class_attribute *result;
	Dee_hash_t i, perturb;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	i = perturb = hash & self->cd_cattr_mask;
	for (;; DeeClassDescriptor_CATTRNEXT(i, perturb)) {
		result = &self->cd_cattr_list[i & self->cd_cattr_mask];
		if (!result->ca_name)
			break;
		if (result->ca_hash != hash)
			continue;
		if (DeeString_EqualsSTR(result->ca_name, name))
			return result;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryClassAttributeStringHash(DeeClassDescriptorObject *__restrict self,
                                                 char const *__restrict name, Dee_hash_t hash) {
	struct class_attribute *result;
	Dee_hash_t i, perturb;
	i = perturb = hash & self->cd_cattr_mask;
	for (;; DeeClassDescriptor_CATTRNEXT(i, perturb)) {
		result = &self->cd_cattr_list[i & self->cd_cattr_mask];
		if (!result->ca_name)
			break;
		if (result->ca_hash != hash)
			continue;
		if (strcmp(DeeString_STR(result->ca_name), name) != 0)
			continue;
		return result;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryClassAttributeStringLenHash(DeeClassDescriptorObject *__restrict self,
                                                    char const *__restrict name,
                                                    size_t attrlen,
                                                    Dee_hash_t hash) {
	struct class_attribute *result;
	Dee_hash_t i, perturb;
	i = perturb = hash & self->cd_cattr_mask;
	for (;; DeeClassDescriptor_CATTRNEXT(i, perturb)) {
		result = &self->cd_cattr_list[i & self->cd_cattr_mask];
		if (!result->ca_name)
			break;
		if (result->ca_hash != hash)
			continue;
		if (DeeString_EqualsBuf(result->ca_name, name, attrlen))
			return result;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryInstanceAttributeHash(DeeClassDescriptorObject *self,
                                              /*String*/ DeeObject *name, Dee_hash_t hash) {
	struct class_attribute *result;
	Dee_hash_t i, perturb;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	i = perturb = hash & self->cd_iattr_mask;
	for (;; DeeClassDescriptor_IATTRNEXT(i, perturb)) {
		result = &self->cd_iattr_list[i & self->cd_iattr_mask];
		if (!result->ca_name)
			break;
		if (result->ca_hash != hash)
			continue;
		if (DeeString_EqualsSTR(result->ca_name, name))
			return result;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryInstanceAttributeStringHash(DeeClassDescriptorObject *__restrict self,
                                                    char const *__restrict name, Dee_hash_t hash) {
	struct class_attribute *result;
	Dee_hash_t i, perturb;
	i = perturb = hash & self->cd_iattr_mask;
	for (;; DeeClassDescriptor_IATTRNEXT(i, perturb)) {
		result = &self->cd_iattr_list[i & self->cd_iattr_mask];
		if (!result->ca_name)
			break;
		if (result->ca_hash != hash)
			continue;
		if (strcmp(DeeString_STR(result->ca_name), name) != 0)
			continue;
		return result;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryInstanceAttributeStringLenHash(DeeClassDescriptorObject *__restrict self,
                                                       char const *__restrict name,
                                                       size_t attrlen, Dee_hash_t hash) {
	struct class_attribute *result;
	Dee_hash_t i, perturb;
	i = perturb = hash & self->cd_iattr_mask;
	for (;; DeeClassDescriptor_IATTRNEXT(i, perturb)) {
		result = &self->cd_iattr_list[i & self->cd_iattr_mask];
		if (!result->ca_name)
			break;
		if (result->ca_hash != hash)
			continue;
		if (DeeString_EqualsBuf(result->ca_name, name, attrlen))
			return result;
	}
	return NULL;
}

/* Instance member access (by addr) */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeInstance_GetMember)(/*Class*/ DeeTypeObject *__restrict tp_self,
                              /*Instance*/ DeeObject *__restrict self,
                              uint16_t addr) {
	struct class_desc *desc;
	struct instance_desc *inst;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
	ASSERT(DeeType_IsClass(tp_self));
	ASSERT_OBJECT_TYPE_A(self, tp_self);
	desc = DeeClass_DESC(tp_self);
	ASSERT(addr < desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);
	/* Lock and extract the member. */
	Dee_instance_desc_lock_read(inst);
	result = inst->id_vtab[addr];
	Dee_XIncref(result);
	Dee_instance_desc_lock_endread(inst);
	if unlikely(result == NULL)
		result = DeeRT_ErrCUnboundInstanceMember(tp_self, self, addr);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInstance_BoundMember)(/*Class*/ DeeTypeObject *__restrict tp_self,
                                /*Instance*/ DeeObject *__restrict self,
                                uint16_t addr) {
	DeeObject *value;
	struct class_desc *desc;
	struct instance_desc *inst;
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
	ASSERT(DeeType_IsClass(tp_self));
	ASSERT_OBJECT_TYPE_A(self, tp_self);
	desc = DeeClass_DESC(tp_self);
	ASSERT(addr < desc->cd_desc->cd_imemb_size);
	inst  = DeeInstance_DESC(desc, self);
	value = Dee_atomic_read_with_atomic_rwlock(&inst->id_vtab[addr],
	                                           &inst->id_lock);
	return value != NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) int /* TODO: Change to return "void" */
(DCALL DeeInstance_DelMember)(/*Class*/ DeeTypeObject *__restrict tp_self,
                              /*Instance*/ DeeObject *__restrict self,
                              uint16_t addr) {
	struct class_desc *desc;
	struct instance_desc *inst;
	DREF DeeObject *old_value;
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
	ASSERT(DeeType_IsClass(tp_self));
	ASSERT_OBJECT_TYPE_A(self, tp_self);
	desc = DeeClass_DESC(tp_self);
	ASSERT(addr < desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);

	/* Lock and extract the member. */
	Dee_instance_desc_lock_write(inst);
	old_value = inst->id_vtab[addr];
	inst->id_vtab[addr] = NULL;
	Dee_instance_desc_lock_endwrite(inst);
	Dee_XDecref(old_value);
	return 0;
}

PUBLIC NONNULL((1, 2, 4)) void
(DCALL DeeInstance_SetMember)(/*Class*/ DeeTypeObject *tp_self,
                              /*Instance*/ DeeObject *self,
                              uint16_t addr, DeeObject *value) {
	struct class_desc *desc;
	struct instance_desc *inst;
	DREF DeeObject *old_value;
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
	ASSERT(DeeType_IsClass(tp_self));
	ASSERT_OBJECT_TYPE_A(self, tp_self);
	desc = DeeClass_DESC(tp_self);
	ASSERT(addr < desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);

	/* Lock and extract the member. */
	Dee_Incref(value);
	Dee_instance_desc_lock_write(inst);
	old_value = inst->id_vtab[addr];
	inst->id_vtab[addr] = value;
	Dee_instance_desc_lock_endwrite(inst);
	Dee_XDecref(old_value);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeInstance_SetMemberInitial)(/*Class*/ DeeTypeObject *tp_self,
                                     /*Instance*/ DeeObject *self,
                                     uint16_t addr, DeeObject *value) {
	struct class_desc *desc;
	struct instance_desc *inst;
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
	ASSERT(DeeType_IsClass(tp_self));
	ASSERT_OBJECT_TYPE_A(self, tp_self);
	desc = DeeClass_DESC(tp_self);
	ASSERT(addr < desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);

	/* Lock and extract the member. */
	Dee_Incref(value);
	Dee_instance_desc_lock_write(inst);
	if unlikely(inst->id_vtab[addr]) {
		Dee_instance_desc_lock_endwrite(inst);
		Dee_DecrefNokill(value);
		return DeeRT_ErrCAlreadyBoundInstanceMember(tp_self, self, addr);
	}
	inst->id_vtab[addr] = value;
	Dee_instance_desc_lock_endwrite(inst);
	return 0;
}


PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeInstance_GetMemberSafe)(DeeTypeObject *tp_self,
                                  DeeObject *__restrict self,
                                  uint16_t addr) {
	if (DeeObject_AssertType(tp_self, &DeeType_Type))
		goto err;
	if (DeeObject_AssertType(self, tp_self))
		goto err;
	if (!DeeType_IsClass(tp_self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
		goto err_bad_index;
	return DeeInstance_GetMember(tp_self, self, addr);
err_bad_index:
	err_invalid_instance_addr(tp_self, self, addr);
	goto err;
err_req_class:
	err_requires_class(tp_self);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInstance_BoundMemberSafe)(DeeTypeObject *tp_self,
                                    DeeObject *__restrict self,
                                    uint16_t addr) {
	if (DeeObject_AssertType(tp_self, &DeeType_Type))
		goto err;
	if (DeeObject_AssertType(self, tp_self))
		goto err;
	if (!DeeType_IsClass(tp_self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
		goto err_bad_index;
	return Dee_BOUND_FROMBOOL(DeeInstance_BoundMember(tp_self, self, addr));
err_bad_index:
	return err_invalid_instance_addr(tp_self, self, addr);
err_req_class:
	return err_requires_class(tp_self);
err:
	return Dee_BOUND_ERR;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeInstance_DelMemberSafe)(DeeTypeObject *tp_self,
                                  DeeObject *__restrict self,
                                  uint16_t addr) {
	if (DeeObject_AssertType(tp_self, &DeeType_Type))
		goto err;
	if (DeeObject_AssertType(self, tp_self))
		goto err;
	if (!DeeType_IsClass(tp_self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
		goto err_bad_index;
	return DeeInstance_DelMember(tp_self, self, addr);
err_bad_index:
	return err_invalid_instance_addr(tp_self, self, addr);
err_req_class:
	return err_requires_class(tp_self);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeInstance_SetMemberSafe)(DeeTypeObject *tp_self,
                                  DeeObject *self,
                                  uint16_t addr, DeeObject *value) {
	if (DeeObject_AssertType(tp_self, &DeeType_Type))
		goto err;
	if (DeeObject_AssertType(self, tp_self))
		goto err;
	if (!DeeType_IsClass(tp_self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
		goto err_bad_index;
	DeeInstance_SetMember(tp_self, self, addr, value);
	return 0;
err_bad_index:
	return err_invalid_instance_addr(tp_self, self, addr);
err_req_class:
	return err_requires_class(tp_self);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeInstance_SetMemberInitialSafe)(DeeTypeObject *tp_self,
                                         DeeObject *self,
                                         uint16_t addr, DeeObject *value) {
	if (DeeObject_AssertType(tp_self, &DeeType_Type))
		goto err;
	if (DeeObject_AssertType(self, tp_self))
		goto err;
	if (!DeeType_IsClass(tp_self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
		goto err_bad_index;
	return DeeInstance_SetMemberInitial(tp_self, self, addr, value);
err_bad_index:
	return err_invalid_instance_addr(tp_self, self, addr);
err_req_class:
	return err_requires_class(tp_self);
err:
	return -1;
}

/* Class member access (by addr) */
PUBLIC NONNULL((1, 3)) void
(DCALL DeeClass_SetMember)(DeeTypeObject *self,
                           uint16_t addr, DeeObject *value) {
	struct class_desc *desc;
	DREF DeeObject *old_value;
	ASSERT_OBJECT_TYPE(self, &DeeType_Type);
	ASSERT(DeeType_IsClass(self));
	desc = DeeClass_DESC(self);
	ASSERT(addr < desc->cd_desc->cd_cmemb_size);
	/* Lock and extract the member. */
	Dee_Incref(value);
	Dee_class_desc_lock_write(desc);
	old_value = desc->cd_members[addr];
	desc->cd_members[addr] = value;
	Dee_class_desc_lock_endwrite(desc);
	Dee_XDecref(old_value);
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeClass_SetMemberSafe)(DeeTypeObject *self,
                               uint16_t addr, DeeObject *value) {
	if (DeeObject_AssertType(self, &DeeType_Type))
		goto err;
	if (!DeeType_IsClass(self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(self)->cd_desc->cd_cmemb_size)
		goto err_bad_index;
	DeeClass_SetMember(self, addr, value);
	return 0;
err_bad_index:
	return err_invalid_class_addr(self, addr);
err_req_class:
	return err_requires_class(self);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeClass_GetMember)(DeeTypeObject *__restrict self, uint16_t addr) {
	struct class_desc *desc;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, &DeeType_Type);
	ASSERT(DeeType_IsClass(self));
	desc = DeeClass_DESC(self);
	ASSERT(addr < desc->cd_desc->cd_cmemb_size);

	/* Lock and extract the member. */
	Dee_class_desc_lock_read(desc);
	result = desc->cd_members[addr];
	if unlikely(!result)
		goto err_unlock_unbound;
	Dee_Incref(result);
	Dee_class_desc_lock_endread(desc);
	return result;
err_unlock_unbound:
	Dee_class_desc_lock_endread(desc);
	return DeeRT_ErrCUnboundClassMember(self, addr);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeClass_GetMemberSafe)(DeeTypeObject *__restrict self, uint16_t addr) {
	if (DeeObject_AssertType(self, &DeeType_Type))
		goto err;
	if (!DeeType_IsClass(self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(self)->cd_desc->cd_cmemb_size)
		goto err_bad_index;
	return DeeClass_GetMember(self, addr);
err_bad_index:
	err_invalid_class_addr(self, addr);
	goto err;
err_req_class:
	err_requires_class(self);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeClass_BoundMemberSafe)(DeeTypeObject *__restrict self, uint16_t addr) {
	bool bound;
	if (DeeObject_AssertType(self, &DeeType_Type))
		goto err;
	if (!DeeType_IsClass(self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(self)->cd_desc->cd_cmemb_size)
		goto err_bad_index;
	bound = DeeClass_BoundMember(self, addr);
	return Dee_BOUND_FROMBOOL(bound);
err_bad_index:
	return err_invalid_class_addr(self, addr);
err_req_class:
	return err_requires_class(self);
err:
	return Dee_BOUND_ERR;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CLASS_DESC_C */
