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
#ifndef GUARD_DEEMON_OBJECTS_CLASS_DESC_C
#define GUARD_DEEMON_OBJECTS_CLASS_DESC_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/instancemethod.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/property.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* memcpy(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

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
	OBJECT_HEAD
	DREF ClassDescriptor *co_desc; /* [1..1][const] The referenced class descriptor. */
} ClassOperatorTable;

typedef struct {
	/* A mapping-like {(string | int, int)...} object used for mapping
	 * operator names to their respective class instance table slots. */
	OBJECT_HEAD
	DREF ClassDescriptor        *co_desc; /* [1..1][const] The referenced class descriptor. */
	DWEAK struct class_operator *co_iter; /* [1..1] Current iterator position. */
	struct class_operator       *co_end;  /* [1..1][const] Iterator end position. */
} ClassOperatorTableIterator;
#define COTI_GETITER(x) atomic_read(&(x)->co_iter)

INTDEF DeeTypeObject ClassOperatorTableIterator_Type;
INTDEF DeeTypeObject ClassOperatorTable_Type;


LOCAL struct class_operator *DCALL
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
coti_next(ClassOperatorTableIterator *__restrict self) {
	struct class_operator *ent;
	struct opinfo const *info;
	ent = coti_next_ent(self);
	if (!ent)
		return ITER_DONE;
	info = DeeTypeType_GetOperatorById(&DeeType_Type, ent->co_name);
	if (info) {
		return DeeTuple_Newf("s" PCKu16,
		                     info->oi_sname,
		                     ent->co_addr);
	}
	return DeeTuple_Newf(PCKu16 PCKu16,
	                     ent->co_name,
	                     ent->co_addr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
coti_next_key(ClassOperatorTableIterator *__restrict self) {
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
coti_next_value(ClassOperatorTableIterator *__restrict self) {
	struct class_operator *ent;
	ent = coti_next_ent(self);
	if (!ent)
		return ITER_DONE;
	return DeeInt_NewUInt16(ent->co_addr);
}

PRIVATE NONNULL((1)) void DCALL
coti_fini(ClassOperatorTableIterator *__restrict self) {
	Dee_Decref(self->co_desc);
}

PRIVATE NONNULL((1, 2)) void DCALL
coti_visit(ClassOperatorTableIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->co_desc);
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

#define DEFINE_CLASSOPERATORTABLEITERATOR_COMPARE(name, op)                     \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                       \
	name(ClassOperatorTableIterator *self,                                      \
	     ClassOperatorTableIterator *other) {                                   \
		if (DeeObject_AssertTypeExact(other, &ClassOperatorTableIterator_Type)) \
			goto err;                                                           \
		return_bool(COTI_GETITER(self) op COTI_GETITER(other));                 \
	err:                                                                        \
		return NULL;                                                            \
	}
DEFINE_CLASSOPERATORTABLEITERATOR_COMPARE(coti_eq, ==)
DEFINE_CLASSOPERATORTABLEITERATOR_COMPARE(coti_ne, !=)
DEFINE_CLASSOPERATORTABLEITERATOR_COMPARE(coti_lo, <)
DEFINE_CLASSOPERATORTABLEITERATOR_COMPARE(coti_le, <=)
DEFINE_CLASSOPERATORTABLEITERATOR_COMPARE(coti_gr, >)
DEFINE_CLASSOPERATORTABLEITERATOR_COMPARE(coti_ge, >=)
#undef DEFINE_CLASSOPERATORTABLEITERATOR_COMPARE

PRIVATE struct type_cmp coti_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_ge,
};

PRIVATE struct type_getset tpconst coti_getsets[] = {
	TYPE_GETTER_F(STR_seq, &coti_getseq, METHOD_FNOREFESCAPE, "->?Ert:ClassOperatorTable"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst coti_members[] = {
	TYPE_MEMBER_FIELD_DOC("__class__", STRUCT_OBJECT,
	                      offsetof(ClassOperatorTableIterator, co_desc),
	                      "->?Ert:ClassDescriptor"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ClassOperatorTableIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassOperatorTableIterator",
	/* .tp_doc      = */ DOC("next->?T2?X2?Dstring?Dint?Dint"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&coti_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ClassOperatorTableIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&coti_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&coti_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &coti_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&coti_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ coti_getsets,
	/* .tp_members       = */ coti_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) ==
              offsetof(ClassOperatorTableIterator, co_desc));
#define cot_fini    coti_fini
#define cot_visit   coti_visit
#define cot_members coti_members

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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cot_trygetitem(ClassOperatorTable *self, DeeObject *key) {
	Dee_operator_t opname;
	if (DeeString_Check(key)) {
		struct opinfo const *info;
		/* TODO: Check if the table contains a string-operator "key" */
		info = DeeTypeType_GetOperatorByName(&DeeType_Type, DeeString_STR(key), (size_t)-1);
		if (info == NULL)
			goto nope;
		opname = info->oi_id;
	} else {
		if (DeeObject_AsUInt16(key, &opname))
			goto err;
	}
	return cot_trygetitem_byid(self, opname);
nope:
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cot_trygetitem_string_hash(ClassOperatorTable *self, char const *key, Dee_hash_t hash) {
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

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
cot_getitemdef(ClassOperatorTable *self,
               DeeObject *key, DeeObject *defl) {
	DREF DeeObject *result = cot_trygetitem(self, key);
	if (result == ITER_DONE) {
		result = defl;
		if (result != ITER_DONE)
			Dee_Incref(result);
	}
	return result;
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

PRIVATE struct type_nsi tpconst cot_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&cot_size,
			/* .nsi_nextkey    = */ (dfunptr_t)&coti_next_key,
			/* .nsi_nextvalue  = */ (dfunptr_t)&coti_next_value,
			/* .nsi_getdefault = */ (dfunptr_t)&cot_getitemdef,
			/* .nsi_setdefault = */ (dfunptr_t)NULL,
			/* .nsi_updateold  = */ (dfunptr_t)NULL,
			/* .nsi_insertnew  = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq cot_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cot_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &cot_nsi,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&cot_foreach_pair,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&cot_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&cot_size,
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
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cot_trygetitem,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&cot_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cot_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_member tpconst cot_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ClassOperatorTableIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cot_print(ClassOperatorTable *__restrict self,
          dformatprinter printer, void *arg) {
	DeeStringObject *name = self->co_desc->cd_name;
	if (name == NULL)
		name = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "<operator table for %k>", name);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cot_printrepr(ClassOperatorTable *__restrict self,
              dformatprinter printer, void *arg) {
	DeeStringObject *name = self->co_desc->cd_name;
	if (name == NULL)
		name = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "%k.__class__.operators", name);
}

INTERN DeeTypeObject ClassOperatorTable_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassOperatorTable",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ClassOperatorTable)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cot_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&cot_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cot_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cot_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cot_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &cot_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ cot_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cot_class_members
};




typedef struct {
	OBJECT_HEAD
	DREF ClassDescriptor         *ca_desc; /* [1..1][const] Class descriptor. */
	struct class_attribute const *ca_attr; /* [1..1][const] The attribute that was queried. */
} ClassAttribute;

typedef struct {
	OBJECT_HEAD
	DREF ClassDescriptor               *ca_desc; /* [1..1][const] Class descriptor. */
	DWEAK struct class_attribute const *ca_iter; /* [1..1] Current iterator position. */
	struct class_attribute const       *ca_end;  /* [1..1][const] Iterator end. */
} ClassAttributeTableIterator;

typedef struct {
	OBJECT_HEAD
	DREF ClassDescriptor         *ca_desc;  /* [1..1][const] Class descriptor. */
	struct class_attribute const *ca_start; /* [1..1][const] Hash-vector starting pointer. */
	size_t                        ca_mask;  /* [const] Mask-vector size mask. */
} ClassAttributeTable;

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

STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) ==
              offsetof(ClassAttribute, ca_desc));
#define ca_fini    cot_fini
#define ca_visit   cot_visit
#define ca_members cot_members
STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) ==
              offsetof(ClassAttributeTable, ca_desc));
#define cat_fini    cot_fini
#define cat_visit   cot_visit
#define cat_members cot_members
STATIC_ASSERT(offsetof(ClassOperatorTable, co_desc) ==
              offsetof(ClassAttributeTableIterator, ca_desc));
#define cati_fini    cot_fini
#define cati_visit   cot_visit
#define cati_members cot_members

STATIC_ASSERT(offsetof(ClassOperatorTableIterator, co_desc) == offsetof(ClassAttributeTableIterator, ca_desc));
STATIC_ASSERT(offsetof(ClassOperatorTableIterator, co_iter) == offsetof(ClassAttributeTableIterator, ca_iter));
STATIC_ASSERT(offsetof(ClassOperatorTableIterator, co_end) == offsetof(ClassAttributeTableIterator, ca_end));
#define cati_cmp  coti_cmp
#define cati_copy coti_copy

#define CATI_GETITER(x) atomic_read(&(x)->ca_iter)

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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cati_next(ClassAttributeTableIterator *__restrict self) {
	DREF DeeObject *result;
	DREF ClassAttribute *attr;
	struct class_attribute const *ent;
	ent = cati_next_ent(self);
	if (!ent)
		return ITER_DONE;
	attr = cattr_new(self->ca_desc, ent);
	if unlikely(!attr)
		goto err;
	result = DeeTuple_Pack(2, ent->ca_name, attr);
	Dee_Decref(attr);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cati_next_key(ClassAttributeTableIterator *__restrict self) {
	struct class_attribute const *ent;
	ent = cati_next_ent(self);
	if (!ent)
		return ITER_DONE;
	return_reference_((DeeObject *)ent->ca_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassAttribute *DCALL
cati_next_value(ClassAttributeTableIterator *__restrict self) {
	struct class_attribute const *ent;
	ent = cati_next_ent(self);
	if (!ent)
		return (DREF ClassAttribute *)ITER_DONE;
	return cattr_new(self->ca_desc, ent);
}

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
cat_trygetitem(ClassAttributeTable *self, DeeObject *key) {
	dhash_t hash, i, perturb;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	hash = DeeString_Hash(key);
	i = perturb = hash & self->ca_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_attribute const *at;
		at = &self->ca_start[i & self->ca_mask];
		if (at->ca_name == NULL)
			break;
		if (at->ca_hash != hash)
			continue;
		if (DeeString_SIZE(at->ca_name) != DeeString_SIZE(key))
			continue;
		if (bcmpc(DeeString_STR(at->ca_name), DeeString_STR(key),
		          DeeString_SIZE(key), sizeof(char)) != 0)
			continue;
		return (DREF DeeObject *)cattr_new(self->ca_desc, at);
	}
/*nope:*/
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cat_trygetitem_string_hash(ClassAttributeTable *self,
                           char const *key, Dee_hash_t hash) {
	dhash_t i, perturb;
	i = perturb = hash & self->ca_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_attribute const *at;
		at = &self->ca_start[i & self->ca_mask];
		if (at->ca_name == NULL)
			break;
		if (at->ca_hash != hash)
			continue;
		if (strcmp(DeeString_STR(at->ca_name), key) != 0)
			continue;
		return (DREF DeeObject *)cattr_new(self->ca_desc, at);
	}
/*nope:*/
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cat_trygetitem_string_len_hash(ClassAttributeTable *self, char const *key,
                               size_t keylen, Dee_hash_t hash) {
	dhash_t i, perturb;
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
		return (DREF DeeObject *)cattr_new(self->ca_desc, at);
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

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
cat_getitemdef(ClassAttributeTable *self, DeeObject *key, DeeObject *defl) {
	DREF DeeObject *result = cat_trygetitem(self, key);
	if (result == ITER_DONE) {
		result = defl;
		if (result != ITER_DONE)
			Dee_Incref(result);
	}
	return result;
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
	TYPE_GETTER_F(STR_seq, &cati_getseq, METHOD_FNOREFESCAPE,
	              "->?AAttributeTable?Ert:ClassDescriptor"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cat_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &ClassAttributeTableIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
ca_print(ClassAttribute *__restrict self,
         dformatprinter printer, void *arg) {
	DeeStringObject *cnam = self->ca_desc->cd_name;
	if (cnam == NULL)
		cnam = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "<ClassAttribute %k.%k>",
	                        cnam, self->ca_attr->ca_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
ca_printrepr(ClassAttribute *__restrict self,
             dformatprinter printer, void *arg) {
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
	return_reference_((DeeObject *)self->ca_attr->ca_name);
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
	TYPE_GETTER_F("name", &ca_getname, METHOD_FNOREFESCAPE, "->?Dstring"),
	TYPE_GETTER_F("doc", &ca_getdoc, METHOD_FNOREFESCAPE, "->?X2?Dstring?N"),
	TYPE_GETTER_F("addr", &ca_getaddr, METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Index into the class/instance object table, where @this attribute is stored\n"
	              "When ?#isclassmem or ?#isclassns are ?t, this index and any index offset from it "
	              /**/ "refer to the class object table. Otherwise, the instance object table is dereferenced\n"
	              "This is done so-as to allow instance attributes such as member functions to be stored "
	              /**/ "within the class itself, rather than having to be copied into each and every instance "
	              /**/ "of the class\n"
	              "S.a. ?A__ctable__?DType and ?A__itable__?DType"),
	TYPE_GETTER_F("isprivate", &ca_isprivate, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Evaluates to ?t if @this class attribute was declared as $private"),
	TYPE_GETTER_F("isfinal", &ca_isfinal, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Evaluates to ?t if @this class attribute was declared as $final"),
	TYPE_GETTER_F("isreadonly", &ca_isreadonly, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Evaluates to ?t if @this class attribute can only be read from\n"
	              "When this is case, a property-like attribute can only ever have a getter "
	              /**/ "associated with itself, while field- or method-like attribute can only be "
	              /**/ "written once (aka. when not already bound)"),
	TYPE_GETTER_F("ismethod", &ca_ismethod, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Evaluates to ?t if @this class attribute refers to a method\n"
	              "When set, reading from the attribute will return a an object "
	              /**/ "${InstanceMethod(obj.MEMBER_TABLE[this.addr], obj)}\n"
	              "Note however that this is rarely ever required to be done, as method attributes "
	              /**/ "are usually called directly, in which case a callattr instruction can silently "
	              /**/ "prepend the this-argument before the passed argument list"),
	TYPE_GETTER_F("isproperty", &ca_isproperty, METHOD_FNOREFESCAPE,
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
	TYPE_GETTER_F("isclassmem", &ca_isclassmem, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Set if ?#addr is an index into the class object table, rather than into "
	              /**/ "the instance object table. Note however that when ?#isclassns"),
	TYPE_GETTER_F("isclassns", &ca_getisclassns, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this class attribute is exclusive to the "
	              /**/ "class-namespace (i.e. was declared as $static)\n"
	              "During enumeration of attributes, all attributes where this is ?t "
	              /**/ "are enumated by ?Acattr?Ert:ClassDescriptor, while all for which it isn't "
	              /**/ "are enumated by ?Aiattr?Ert:ClassDescriptor"),
	TYPE_GETTER_F("flags", &ca_getflags, METHOD_FNOREFESCAPE,
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
	TYPE_GETTER_F("__name__", &ca_getname, METHOD_FNOREFESCAPE, "->?Dstring\nAlias for ?#name"),
	TYPE_GETTER_F("__doc__", &ca_getdoc, METHOD_FNOREFESCAPE, "->?X2?Dstring?N\nAlias for ?#doc"),
	TYPE_GETSET_END
};


PRIVATE struct type_nsi tpconst cat_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&cat_size,
			/* .nsi_nextkey    = */ (dfunptr_t)&cati_next_key,
			/* .nsi_nextvalue  = */ (dfunptr_t)&cati_next_value,
			/* .nsi_getdefault = */ (dfunptr_t)&cat_getitemdef,
			/* .nsi_setdefault = */ (dfunptr_t)NULL,
			/* .nsi_updateold  = */ (dfunptr_t)NULL,
			/* .nsi_insertnew  = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq cat_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &cat_nsi,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&cat_foreach_pair,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&cat_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&cat_size,
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
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cat_trygetitem,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&cat_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cat_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ClassAttribute)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ca_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&ca_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&ca_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ca_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ca_getsets,
	/* .tp_members       = */ ca_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject ClassAttributeTableIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassAttributeTableIterator",
	/* .tp_doc      = */ DOC("next->?T2?Dstring?AAttribute?Ert:ClassDescriptor"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&cati_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ClassAttributeTableIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cati_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cati_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &cati_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cati_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ cati_getsets,
	/* .tp_members       = */ cati_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cat_print(ClassAttributeTable *__restrict self,
          dformatprinter printer, void *arg) {
	ClassDescriptor *desc = self->ca_desc;
	DeeStringObject *name = desc->cd_name;
	if (name == NULL)
		name = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "<%s attribute table for %k>",
	                        self->ca_start == desc->cd_cattr_list ? "class" : "instance",
	                        name);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cat_printrepr(ClassAttributeTable *__restrict self,
              dformatprinter printer, void *arg) {
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(ClassAttributeTable)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cat_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&cat_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cat_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cat_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cat_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &cat_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ cat_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cat_class_members
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
cd_visit(ClassDescriptor *__restrict self, dvisit_t proc, void *arg) {
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cd_eq(ClassDescriptor *self,
      ClassDescriptor *other) {
	size_t i;
	if (!DeeClassDescriptor_Check(other))
		goto nope;
	if (self->cd_flags != other->cd_flags)
		goto nope;
	if (self->cd_cmemb_size != other->cd_cmemb_size)
		goto nope;
	if (self->cd_imemb_size != other->cd_imemb_size)
		goto nope;
	/* FIXME: hash vectors cannot be binary-compared. That leads to false negatives.
	 *        -> s.a. `/util/test/rt-classdescriptor-reprcopy.dee' */
	if (self->cd_clsop_mask != other->cd_clsop_mask)
		goto nope;
	if (self->cd_cattr_mask != other->cd_cattr_mask)
		goto nope;
	if (self->cd_iattr_mask != other->cd_iattr_mask)
		goto nope;
	if (self->cd_name) {
		if (!other->cd_name)
			goto nope;
		if (!DeeString_EqualsSTR(self->cd_name, other->cd_name))
			goto nope;
	} else {
		if (other->cd_name)
			goto nope;
	}
	if (self->cd_doc) {
		if (!other->cd_doc)
			goto nope;
		if (!DeeString_EqualsSTR(self->cd_doc, other->cd_doc))
			goto nope;
	} else {
		if (other->cd_doc)
			goto nope;
	}
	if (bcmpc(self->cd_clsop_list,
	          other->cd_clsop_list,
	          self->cd_clsop_mask + 1,
	          sizeof(struct class_operator)) != 0)
		goto nope;
	for (i = 0; i <= self->cd_cattr_mask; ++i) {
		if (!class_attribute_eq(&self->cd_cattr_list[i],
		                        &other->cd_cattr_list[i]))
			goto nope;
	}
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		if (!class_attribute_eq(&self->cd_iattr_list[i],
		                        &other->cd_iattr_list[i]))
			goto nope;
	}
	return_true;
nope:
	return_false;
}


PRIVATE struct type_cmp cd_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cd_eq,
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
	char fe_name[14];
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
	TYPE_GETTER_F("flags", &cd_getflags, METHOD_FNOREFESCAPE,
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
	TYPE_GETTER(STR_operators, &cd_operators,
	            "->?#OperatorTable\n"
	            "Enumerate operators implemented by @this class, as well as their associated "
	            /**/ "class object table indices which are holding the respective implementations\n"
	            "Note that the class object table entry may be left unbound to explicitly "
	            /**/ "define an operator as having been deleted"),
	TYPE_GETTER("iattr", &cd_iattr,
	            "->?#AttributeTable\n"
	            "Enumerate user-defined instance attributes as a mapping-like ?Dstring-?#Attribute sequence"),
	TYPE_GETTER("cattr", &cd_cattr,
	            "->?#AttributeTable\n"
	            "Enumerate user-defined class ($static) attributes as a mapping-like ?Dstring-?#Attribute sequence"),
	TYPE_GETTER("__sizeof__", &cd_sizeof, "->?Dint"),
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
	                         "Otherwise, the super-args operator simply returns args and the super-constructor "
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

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cd_print(ClassDescriptor *__restrict self,
         dformatprinter printer, void *arg) {
	DeeStringObject *name = self->cd_name;
	if (name == NULL)
		name = &str_lt_anonymous_gr;
	return DeeFormat_Printf(printer, arg, "<ClassDescriptor for %k>", name);
}

PRIVATE WUNUSED NONNULL((1, 3)) dssize_t DCALL
xattr_table_printrepr(struct Dee_class_attribute *table, size_t mask,
                      dformatprinter printer, void *arg) {
	dssize_t temp, result = 0;
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

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
cd_printrepr(ClassDescriptor *__restrict self,
             dformatprinter printer, void *arg) {
	dssize_t temp, result;
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
LOCAL int DCALL
class_attribute_init(struct class_attribute *__restrict self,
                     DeeObject *__restrict data,
                     bool is_class_attribute) {
	DREF DeeObject *iter;
	DREF DeeObject *addr, *flags, *doc;
	size_t fast_size;
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
	doc       = NULL;
	fast_size = DeeFastSeq_GetSize_deprecated(data);
	if (fast_size != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (fast_size != 2 && fast_size != 3) {
			err_invalid_unpack_size_minmax(data, 2, 3, fast_size);
			goto err;
		}
		addr = DeeFastSeq_GetItem_deprecated(data, 0);
		if unlikely(!addr)
			goto err;
		flags = DeeFastSeq_GetItem_deprecated(data, 1);
		if unlikely(!addr)
			goto err_addr;
		if (fast_size >= 3) {
			doc = DeeFastSeq_GetItem_deprecated(data, 2);
			if unlikely(!addr)
				goto err_addr_flags;
		}
	} else {
		iter = DeeObject_Iter(data);
		if unlikely(!iter)
			goto err;
		addr = DeeObject_IterNext(iter);
		if unlikely(!ITER_ISOK(addr)) {
			if (addr == ITER_DONE)
				err_invalid_unpack_size_minmax(data, 2, 3, 0);
			goto err_iter;
		}
		flags = DeeObject_IterNext(iter);
		if unlikely(!ITER_ISOK(flags)) {
			if (flags == ITER_DONE)
				err_invalid_unpack_size_minmax(data, 2, 3, 1);
			goto err_iter_addr;
		}
		doc = DeeObject_IterNext(iter);
		if (doc == ITER_DONE) {
			doc = NULL;
		} else {
			DREF DeeObject *tail;
			if unlikely(!doc)
				goto err_iter_addr_flags;
			tail = DeeObject_IterNext(iter);
			if unlikely(tail != ITER_DONE) {
				if (tail) {
					Dee_Decref(tail);
					err_invalid_unpack_iter_size_minmax(data, iter, 2, 3);
				}
				goto err_iter_addr_flags_doc;
			}
		}
		Dee_Decref(iter);
	}
	if (doc && DeeObject_AssertTypeExact(doc, &DeeString_Type))
		goto err_addr_flags_doc;
	if (DeeObject_AsUInt16(addr, &self->ca_addr))
		goto err_addr_flags_doc;
	if (DeeString_Check(flags)) {
		char *pos;
		self->ca_flag = 0;
		pos = DeeString_STR(flags);
		if (*pos) {
			for (;;) {
				char *next;
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
		if (DeeObject_AsUInt16(addr, &self->ca_flag))
			goto err_addr_flags_doc;
		if (self->ca_flag & ~CLASS_ATTRIBUTE_FMASK) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid flags for %s-attribute %r (0x%.4I16x)",
			                is_class_attribute ? STR_class : "instance",
			                self->ca_name, self->ca_flag);
			goto err_addr_flags_doc;
		}
	}
	self->ca_doc = (DREF struct string_object *)doc; /* Inherit reference. */
	Dee_Decref(flags);
	Dee_Decref(addr);
	return 0;
err_iter_addr_flags_doc:
	Dee_Decref(doc);
err_iter_addr_flags:
	Dee_Decref(flags);
err_iter_addr:
	Dee_Decref(addr);
err_iter:
	Dee_Decref(iter);
	goto err;
err_addr_flags_doc:
	Dee_XDecref(doc);
err_addr_flags:
	Dee_Decref(flags);
err_addr:
	Dee_Decref(addr);
err:
	return -1;
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


PRIVATE WUNUSED NONNULL((1)) ClassDescriptor *DCALL
cd_alloc_from_iattr(DeeObject *__restrict iattr,
                    uint16_t imemb_size,
                    uint16_t cmemb_size) {
	size_t iattr_used = 0, imask = 7;
	DREF DeeObject *iterator, *elem;
	DREF DeeObject *data[2];
	ClassDescriptor *result;
	iterator = DeeObject_Iter(iattr);
	if unlikely(!iterator)
		goto err;
	result = (ClassDescriptor *)DeeObject_Calloc(offsetof(ClassDescriptor, cd_iattr_list) +
	                                             (8 * sizeof(struct class_attribute)));
	if unlikely(!result)
		goto err_iter;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		struct class_attribute *ent;
		dhash_t hash, j, perturb;
		if (iattr_used >= (imask / 3) * 2) {
			ClassDescriptor *new_result;
			size_t i, new_mask;
			new_mask   = (imask << 1) | 1;
			new_result = (ClassDescriptor *)DeeObject_Calloc(offsetof(ClassDescriptor, cd_iattr_list) +
			                                                 ((new_mask + 1) * sizeof(struct class_attribute)));
			if unlikely(!new_result)
				goto err_iter_r_elem;
			/* Rehash the already existing instance attribute table. */
			for (i = 0; i <= imask; ++i) {
				if (!result->cd_iattr_list[i].ca_name)
					continue;
				j = perturb = result->cd_iattr_list[i].ca_hash & new_mask;
				for (;; DeeClassDescriptor_IATTRNEXT(j, perturb)) {
					ent = &new_result->cd_iattr_list[j & new_mask];
					if (ent->ca_name)
						continue;
					*ent = result->cd_iattr_list[i];
					break;
				}
			}
			new_result->cd_imemb_size = result->cd_imemb_size;
			new_result->cd_cmemb_size = result->cd_cmemb_size;
			DeeObject_Free(result);
			result = new_result;
			imask  = new_mask;
		}
		if (DeeObject_Unpack(elem, 2, data))
			goto err_iter_r_elem;
		Dee_Decref(elem);
		if (DeeObject_AssertType(data[0], &DeeString_Type))
			goto err_iter_r_data;
		hash = DeeString_Hash(data[0]);
		j = perturb = hash & imask;
		for (;; DeeClassDescriptor_IATTRNEXT(j, perturb)) {
			ent = &result->cd_iattr_list[j & imask];
			if (!ent->ca_name)
				break;
			if (ent->ca_hash != hash)
				continue;
			if (!DeeString_EqualsSTR(ent->ca_name, data[0]))
				continue;
			/* Duplicate attribute. */
			DeeError_Throwf(&DeeError_ValueError,
			                "Duplicate instance attribute %r",
			                data[0]);
			goto err_iter_r_data;
		}
		ent->ca_name = (DREF struct string_object *)data[0]; /* Inherit reference (on success). */
		ent->ca_hash = hash;
		if (class_attribute_init(ent, data[1], false))
			goto err_iter_r_data;
		++iattr_used;
		Dee_Decref(data[1]);
		{
			uint16_t maxid;
			maxid = ent->ca_addr;
			if ((ent->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)) == CLASS_ATTRIBUTE_FGETSET)
				maxid += CLASS_GETSET_SET;
			if (ent->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
				if (cmemb_size != (uint16_t)-1 && maxid >= cmemb_size) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Instance attribute %r uses out-of-bounds class "
					                "object table index %" PRFu16 " (>= %" PRFu16 ")",
					                ent->ca_name, maxid, cmemb_size);
					goto err_iter_r;
				}
				if (result->cd_cmemb_size <= maxid)
					result->cd_cmemb_size = maxid + 1;
			} else {
				if (imemb_size != (uint16_t)-1 && maxid >= imemb_size) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Instance attribute %r uses out-of-bounds object "
					                "table index %" PRFu16 " (>= %" PRFu16 ")",
					                ent->ca_name, maxid, imemb_size);
					goto err_iter_r;
				}
				if (result->cd_imemb_size <= maxid)
					result->cd_imemb_size = maxid + 1;
			}
		}
	}
	if (!elem)
		goto err_iter_r;
	result->cd_iattr_mask = imask;
	if (imemb_size != (uint16_t)-1)
		result->cd_imemb_size = imemb_size;
	if (cmemb_size != (uint16_t)-1)
		result->cd_cmemb_size = cmemb_size;
	Dee_Decref(iterator);
	return result;
err_iter_r_data:
	Dee_Decref(data[1]);
	Dee_Decref(data[0]);
	goto err_iter_r;
err_iter_r_elem:
	Dee_Decref(elem);
err_iter_r:
	do {
		if (result->cd_iattr_list[imask].ca_name) {
			Dee_XDecref(result->cd_iattr_list[imask].ca_doc);
			Dee_Decref(result->cd_iattr_list[imask].ca_name);
		}
	} while (imask--);
	DeeObject_Free(result);
err_iter:
	Dee_Decref(iterator);
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


PRIVATE WUNUSED DREF ClassDescriptor *DCALL
cd_init_kw(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ClassDescriptor *result;
	DREF DeeObject *iterator, *elem, *data[2];
	DeeStringObject *class_name;
	DeeStringObject *class_doc   = (DeeStringObject *)Dee_EmptyString;
	DeeStringObject *class_flags = (DeeStringObject *)Dee_EmptyString;
	DeeObject *class_operators   = Dee_EmptyTuple;
	DeeObject *class_iattr       = Dee_EmptyTuple;
	DeeObject *class_cattr       = Dee_EmptyTuple;
	uint16_t class_isize         = (uint16_t)-1;
	uint16_t class_csize         = (uint16_t)-1;
	PRIVATE DEFINE_KWLIST(kwlist, { K(name), K(doc), K(flags), K(operators),
	                                K(iattr), K(cattr), K(isize), K(csize),
	                                KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|ooooo" UNPu16 UNPu16 ":_ClassDescriptor",
	                    &class_name, &class_doc,
	                    &class_flags, &class_operators,
	                    &class_iattr, &class_cattr,
	                    &class_isize, &class_csize))
		goto err;
	if (DeeObject_AssertType(class_name, &DeeString_Type))
		goto err;
	if (class_doc &&
	    DeeObject_AssertType(class_doc, &DeeString_Type))
		goto err;

	result = cd_alloc_from_iattr(class_iattr, class_isize, class_csize);
	if unlikely(!result)
		goto err;
	result->cd_flags = TP_FNORMAL;
	if (class_flags != (DeeStringObject *)Dee_EmptyString) {
		if (DeeString_Check(class_flags)) {
			char *pos;
			pos = DeeString_STR(class_flags);
			if (*pos) {
				for (;;) {
					char *next;
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
			if (DeeObject_AsUInt16((DeeObject *)class_flags, &result->cd_flags))
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
	if (class_cattr != Dee_EmptyTuple) {
		size_t used_attr = 0;
		iterator         = DeeObject_Iter(class_cattr);
		if unlikely(!iterator)
			goto err_r_imemb;
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			struct class_attribute *ent;
			dhash_t hash, i, perturb;
			if (DeeObject_Unpack(elem, 2, data))
				goto err_r_imemb_iter_elem;
			Dee_Decref(elem);
			if (DeeObject_AssertType(data[0], &DeeString_Type))
				goto err_r_imemb_iter_data;
			if (used_attr >= (result->cd_cattr_mask / 3) * 2) {
				/* Rehash the class attribute table. */
				if (cd_rehash_cattr(result))
					goto err_r_imemb_iter_data;
			}
			hash = DeeString_Hash(data[0]);
			i = perturb = hash & result->cd_cattr_mask;
			for (;; DeeClassDescriptor_CATTRNEXT(i, perturb)) {
				ent = &result->cd_cattr_list[i & result->cd_cattr_mask];
				if (!ent->ca_name)
					break;
				if (ent->ca_hash != hash)
					continue;
				if (!DeeString_EqualsSTR(ent->ca_name, data[0]))
					continue;
				DeeError_Throwf(&DeeError_ValueError,
				                "Duplicate class attribute %r",
				                data[0]);
				goto err_r_imemb_iter_data;
			}
			ent->ca_name = (DREF struct string_object *)data[0]; /* Inherit reference (on success) */
			ent->ca_hash = hash;
			if (class_attribute_init(ent, data[1], true))
				goto err_r_imemb_iter_data;
			++used_attr;
			Dee_Decref(data[1]);
			{
				uint16_t maxid;
				maxid = ent->ca_addr;
				if ((ent->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)) == CLASS_ATTRIBUTE_FGETSET)
					maxid += CLASS_GETSET_SET;
				if (class_csize != (uint16_t)-1 && maxid >= class_csize) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Class attribute %r uses out-of-bounds class "
					                "object table index %" PRFu16 " (>= %" PRFu16 ")",
					                ent->ca_name, maxid, class_csize);
					goto err_r_imemb_iter;
				}
				if (result->cd_cmemb_size <= maxid)
					result->cd_cmemb_size = maxid + 1;
			}
		}
		if (!elem)
			goto err_r_imemb_iter;
		Dee_Decref(iterator);
	}
	if (class_operators != Dee_EmptyTuple) {
		Dee_operator_t operator_count = 0;
		iterator = DeeObject_Iter(class_operators);
		if unlikely(!iterator)
			goto err_r_imemb_cmemb;
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			Dee_operator_t name, index;
			if (DeeObject_Unpack(elem, 2, data))
				goto err_r_imemb_iter_elem;
			Dee_Decref(elem);
			if (DeeObject_AsUInt16(data[1], &index))
				goto err_r_imemb_iter_data;
			if (DeeString_Check(data[0])) {
				struct opinfo const *info;
				info = DeeTypeType_GetOperatorByName(&DeeType_Type, DeeString_STR(data[0]), (size_t)-1);
				if (info == NULL) {
					/* TODO: In this case, must store the operator via its name
					 *       (so the name-query can happen in `DeeClass_New()') */
					DeeError_Throwf(&DeeError_ValueError,
					                "Unknown operator %r",
					                data[0]);
					goto err_r_imemb_iter_data;
				}
				name = info->oi_id;
			} else {
				if (DeeObject_AsUInt16(data[0], &name))
					goto err_r_imemb_iter_data;
			}
			Dee_Decref(data[1]);
			Dee_Decref(data[0]);
			if (class_csize != (uint16_t)-1 && index >= class_csize) {
				struct opinfo const *op = DeeTypeType_GetOperatorById(&DeeType_Type, name);
				if (op) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Operator %s uses out-of-bounds class object "
					                "table index %" PRFu16 " (>= %" PRFu16 ")",
					                op->oi_sname, index, class_csize);
				} else {
					DeeError_Throwf(&DeeError_ValueError,
					                "Operator 0x%.4I16x uses out-of-bounds class "
					                "object table index %" PRFu16 " (>= %" PRFu16 ")",
					                name, index, class_csize);
				}
				goto err_r_imemb_iter;
			}
			if (result->cd_cmemb_size <= index)
				result->cd_cmemb_size = index + 1;
			if (cd_add_operator(result, name, index, &operator_count))
				goto err_r_imemb_iter;
		}
		if (!elem)
			goto err_r_imemb_iter;
		Dee_Decref(iterator);
	}
	result->cd_name = class_name;
	result->cd_doc  = class_doc;
	Dee_Incref(class_name);
	Dee_XIncref(class_doc);
	DeeObject_Init(result, &DeeClassDescriptor_Type);
	return result;
err_r_imemb_iter_data:
	Dee_Decref(data[1]);
	Dee_Decref(data[0]);
	goto err_r_imemb_iter;
err_r_imemb_iter_elem:
	Dee_Decref(elem);
err_r_imemb_iter:
	Dee_Decref(iterator);
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
		{
			/* .tp_var = */ {
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor   = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				/* .tp_free        = */ (dfunptr_t)NULL,
				/* .tp_pad         = */ { (dfunptr_t)NULL },
				/* .tp_any_ctor_kw = */ (dfunptr_t)&cd_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cd_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cd_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&cd_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cd_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &cd_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ cd_methods,
	/* .tp_getsets       = */ cd_getsets,
	/* .tp_members       = */ cd_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cd_class_members
};



typedef struct {
	OBJECT_HEAD
	DREF DeeObject       *ot_owner; /* [1..1][const] The associated owner object.
	                                 * NOTE: This may be a super-object, in which case the referenced
	                                 *       object table refers to the described super-type. */
	struct instance_desc *ot_desc;  /* [1..1][valid_if(ot_size != 0)][const] The referenced instance descriptor. */
	uint16_t              ot_size;  /* [const] The length of the object table contained within `ot_desc' */
} ObjectTable;

STATIC_ASSERT(offsetof(ObjectTable, ot_owner) ==
              offsetof(ClassAttributeTable, ca_desc));
#define ot_fini cot_fini

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
ot_print(ObjectTable *__restrict self, dformatprinter printer, void *arg) {
	DeeTypeObject *tp = DeeObject_Class(self->ot_owner);
	if (DeeType_IsTypeType(tp))
		return DeeFormat_Printf(printer, arg, "<class object table for %k>", tp);
	return DeeFormat_Printf(printer, arg, "<object table for instance of %k>", tp);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
ot_printrepr(ObjectTable *__restrict self, dformatprinter printer, void *arg) {
	DeeTypeObject *tp = DeeObject_Class(self->ot_owner);
	if (DeeType_IsTypeType(tp))
		return DeeFormat_Printf(printer, arg, "%r.__ctable__", tp);
	return DeeFormat_Printf(printer, arg, "%r.__itable__", self->ot_owner);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ot_bool(ObjectTable *__restrict self) {
	return self->ot_size != 0;
}

PRIVATE NONNULL((1, 2)) void DCALL
ot_visit(ObjectTable *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ot_owner);
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
	err_unbound_index((DeeObject *)self, index);
	return NULL;
err_index:
	err_index_out_of_bounds((DeeObject *)self, index, self->ot_size);
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
	return err_index_out_of_bounds((DeeObject *)self, index, self->ot_size);
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
	return err_index_out_of_bounds((DeeObject *)self, index, self->ot_size);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ot_bounditem_index(ObjectTable *__restrict self, size_t index) {
	if unlikely(index >= self->ot_size)
		return -2;
	return atomic_read(&self->ot_desc->id_vtab[index]) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
ot_nsi_xchitem(ObjectTable *self, size_t index, DeeObject *newval) {
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
	err_unbound_index((DeeObject *)self, index);
	goto err;
err_index:
	err_index_out_of_bounds((DeeObject *)self, index, self->ot_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ot_foreach(ObjectTable *self, Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < self->ot_size; ++i) {
		DREF DeeObject *elem;
		Dee_instance_desc_lock_read(self->ot_desc);
		elem = self->ot_desc->id_vtab[i];
		if (!elem) {
			Dee_instance_desc_lock_endread(self->ot_desc);
			continue;
		}
		Dee_Incref(elem);
		Dee_instance_desc_lock_endread(self->ot_desc);
		temp = (*proc)(arg, elem);
		Dee_Decref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_nsi tpconst ot_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&ot_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&ot_size,
			/* .nsi_getitem      = */ (dfunptr_t)&ot_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)&ot_delitem_index,
			/* .nsi_setitem      = */ (dfunptr_t)&ot_setitem_index,
			/* .nsi_getitem_fast = */ (dfunptr_t)&ot_getitem_index_fast,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL,
			/* .nsi_rfind        = */ (dfunptr_t)NULL,
			/* .nsi_xch          = */ (dfunptr_t)&ot_nsi_xchitem,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq ot_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &ot_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ot_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&ot_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&ot_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ot_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ot_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ot_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ot_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&ot_bounditem_index,
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


PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
ot_gettype(ObjectTable *__restrict self) {
	DREF DeeTypeObject *result;
	result = DeeType_Check(self->ot_owner)
	         ? (DeeTypeObject *)self->ot_owner
	         : Dee_TYPE(self->ot_owner);
	if (result == &DeeSuper_Type)
		result = DeeSuper_TYPE(self->ot_owner);
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassDescriptor *DCALL
ot_getclass(ObjectTable *__restrict self) {
	DREF ClassDescriptor *result;
	DREF DeeTypeObject *type;
	type = DeeType_Check(self->ot_owner)
	       ? (DeeTypeObject *)self->ot_owner
	       : Dee_TYPE(self->ot_owner);
	if (type == &DeeSuper_Type)
		type = DeeSuper_TYPE(self->ot_owner);
	result = DeeClass_DESC(type)->cd_desc;
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_isctable(ObjectTable *__restrict self) {
	return_bool(DeeType_Check(self->ot_owner));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_isitable(ObjectTable *__restrict self) {
	return_bool(!DeeType_Check(self->ot_owner));
}

PRIVATE struct type_getset tpconst ot_getsets[] = {
	TYPE_GETTER_F(STR___type__, &ot_gettype,
	              METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	              "->?DType\n"
	              "The type describing @this object table"),
	TYPE_GETTER_F("__class__", &ot_getclass,
	              METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	              "->?Ert:ClassDescriptor\n\n"
	              "Same as ${this.__type__.__class__}"),
	TYPE_GETTER_F("__isctable__", &ot_isctable,
	              METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	              "->?Dbool\n\n"
	              "Evaluates to ?t if @this is a class object table"),
	TYPE_GETTER_F("__isitable__", &ot_isitable,
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
	if (DeeArg_Unpack(argc, argv, "o|o:_ObjectTable", &ob, &type))
		goto err;
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
	if (type == &DeeType_Type) {
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
	self->ot_owner = (DREF DeeObject *)ob;
	Dee_Incref(ob);
	return 0;
err_no_class:
	DeeError_Throwf(&DeeError_TypeError,
	                "Type `%k' isn't a class",
	                type);
err:
	return -1;
}


INTERN DeeTypeObject ObjectTable_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ObjectTable",
	/* .tp_doc      = */ DOC("(ob:?X2?O?DType)\n"
	                         "#tTypeError{The given @ob isn't a class or class instance}"
	                         "Load the object member table of a class, or class instance"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&ot_init,
				TYPE_FIXED_ALLOCATOR(ObjectTable)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ot_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&ot_bool,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&ot_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&ot_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ot_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ot_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ot_getsets,
	/* .tp_members       = */ ot_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_ctable(DeeTypeObject *__restrict self) {
	DREF ObjectTable *result;
	struct class_desc *desc;
	if (!DeeType_IsClass(self))
		return_empty_seq;
	desc   = DeeClass_DESC(self);
	result = DeeObject_MALLOC(ObjectTable);
	if unlikely(!result)
		goto done;
	result->ot_owner = (DREF DeeObject *)self;
	result->ot_desc  = class_desc_as_instance(desc);
	result->ot_size  = desc->cd_desc->cd_cmemb_size;
	Dee_Incref(self);
	DeeObject_Init(result, &ObjectTable_Type);
done:
	return (DREF DeeObject *)result;
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
		return_empty_seq;
	desc   = DeeClass_DESC(type);
	result = DeeObject_MALLOC(ObjectTable);
	if unlikely(!result)
		goto done;
	result->ot_owner = (DREF DeeObject *)self;
	result->ot_desc  = DeeInstance_DESC(desc, real_self);
	result->ot_size  = desc->cd_desc->cd_imemb_size;
	Dee_Incref(self);
	DeeObject_Init(result, &ObjectTable_Type);
done:
	return (DREF DeeObject *)result;
}





PRIVATE struct keyword thisarg_kwlist[] = { K(thisarg), KEND };

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get(DeeInstanceMemberObject *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg;
	struct class_desc *desc;
	if (DeeArg_UnpackKw(argc, argv, kw, thisarg_kwlist, "o:get", &thisarg))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->im_type))
		goto err;
	desc = DeeClass_DESC(self->im_type);
	return DeeInstance_GetAttribute(desc,
	                                DeeInstance_DESC(desc,
	                                                 thisarg),
	                                thisarg,
	                                self->im_attribute);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_delete(DeeInstanceMemberObject *self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg;
	struct class_desc *desc;
	if (DeeArg_UnpackKw(argc, argv, kw, thisarg_kwlist, "o:delete", &thisarg))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->im_type))
		goto err;
	desc = DeeClass_DESC(self->im_type);
	if (DeeInstance_DelAttribute(desc,
	                             DeeInstance_DESC(desc,
	                                              thisarg),
	                             thisarg,
	                             self->im_attribute))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_set(DeeInstanceMemberObject *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg, *value;
	struct class_desc *desc;
	PRIVATE struct keyword kwlist[] = { K(thisarg), K(value), KEND };
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "oo:set", &thisarg, &value))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->im_type))
		goto err;
	desc = DeeClass_DESC(self->im_type);
	if (DeeInstance_SetAttribute(desc,
	                             DeeInstance_DESC(desc,
	                                              thisarg),
	                             thisarg,
	                             self->im_attribute,
	                             value))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
instancemember_copy(DeeInstanceMemberObject *__restrict self,
                    DeeInstanceMemberObject *__restrict other) {
	self->im_type      = other->im_type;
	self->im_attribute = other->im_attribute;
	Dee_Incref(other->im_type);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
instancemember_fini(DeeInstanceMemberObject *__restrict self) {
	Dee_Decref(self->im_type);
}

PRIVATE NONNULL((1, 2)) void DCALL
instancemember_visit(DeeInstanceMemberObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->im_type);
}


PRIVATE struct type_method tpconst instancemember_methods[] = {
	TYPE_KWMETHOD_F(STR_get, &instancemember_get, METHOD_FNOREFESCAPE,
	                "(thisarg)->\n"
	                "Return the @thisarg's value of @this member"),
	TYPE_KWMETHOD_F("delete", &instancemember_delete, METHOD_FNOREFESCAPE,
	                "(thisarg)\n"
	                "Delete @thisarg's value of @this member"),
	TYPE_KWMETHOD_F(STR_set, &instancemember_set, METHOD_FNOREFESCAPE,
	                "(thisarg,value)\n"
	                "Set @thisarg's value of @this member to @value"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_module(DeeInstanceMemberObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->im_type);
	if (result)
		return result;
	err_unbound_attribute_string(&DeeInstanceMember_Type, STR___module__);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_name(DeeInstanceMemberObject *__restrict self) {
	return_reference_((DREF DeeObject *)self->im_attribute->ca_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_doc(DeeInstanceMemberObject *__restrict self) {
	if (!self->im_attribute->ca_doc)
		return_none;
	return_reference_((DREF DeeObject *)self->im_attribute->ca_doc);
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
	TYPE_GETTER_F("canget", &instancemember_get_canget, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this member can be read from"),
	TYPE_GETTER_F("candel", &instancemember_get_candel, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this member can be deleted"),
	TYPE_GETTER_F("canset", &instancemember_get_canset, METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this member can be written to"),
	TYPE_GETTER_F(STR___name__, &instancemember_get_name, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "The name of @this instance member"),
	TYPE_GETTER_F(STR___doc__, &instancemember_get_doc, METHOD_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "The documentation string associated with @this instance member"),
	TYPE_GETTER_F(STR___module__, &instancemember_get_module, METHOD_FNOREFESCAPE,
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
PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
instancemember_hash(DeeInstanceMemberObject *__restrict self) {
	return (Dee_HashPointer(self->im_type) ^
	        Dee_HashPointer(self->im_attribute));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
instancemember_eq(DeeInstanceMemberObject *self,
                  DeeInstanceMemberObject *other) {
	if (DeeObject_AssertType(other, &DeeInstanceMember_Type))
		goto err;
	return_bool(self->im_type == other->im_type &&
	            self->im_attribute == other->im_attribute);
err:
	return NULL;
}

PRIVATE struct type_cmp instancemember_cmp = {
	/* .tp_hash          = */ (dhash_t (DCALL *)(DeeObject *__restrict))&instancemember_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&instancemember_eq,
};

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
instancemember_print(DeeInstanceMemberObject *__restrict self,
                     dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<InstanceMember %k.%k>",
	                        self->im_type, self->im_attribute->ca_name);
}

INTDEF WUNUSED NONNULL((1)) bool DCALL
DeeString_IsSymbol(DeeStringObject *__restrict self,
                   size_t start_index,
                   size_t end_index);

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
instancemember_printrepr(DeeInstanceMemberObject *__restrict self,
                         dformatprinter printer, void *arg) {
	DeeStringObject *name = (DeeStringObject *)self->im_attribute->ca_name;
	if (DeeString_IsSymbol(name, 0, (size_t)-1)) {
		return DeeFormat_Printf(printer, arg, "%r.%k", self->im_type, name);
	} else {
		return DeeFormat_Printf(printer, arg, "%r.operator . (%r)", self->im_type, name);
	}
}

PUBLIC DeeTypeObject DeeInstanceMember_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_InstanceMember",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&instancemember_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeInstanceMemberObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&instancemember_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&instancemember_print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&instancemember_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&instancemember_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &instancemember_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ instancemember_methods,
	/* .tp_getsets       = */ instancemember_getsets,
	/* .tp_members       = */ instancemember_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&instancemember_get
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
	return (DREF DeeObject *)result;
}


INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeClass_EnumClassInstanceAttributes(DeeTypeObject *__restrict self,
                                     denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	size_t i;
	struct class_desc *my_class    = DeeClass_DESC(self);
	DeeClassDescriptorObject *desc = my_class->cd_desc;
	for (i = 0; i <= desc->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		DREF DeeTypeObject *attr_type;
		uint16_t perm;
		attr = &desc->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		attr_type = NULL;
		perm = (ATTR_IMEMBER | ATTR_CMEMBER | ATTR_WRAPPER |
		        ATTR_PERMGET | ATTR_NAMEOBJ);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm |= ATTR_PROPERTY;
		} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
			perm |= ATTR_PERMCALL;
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
			perm |= ATTR_PRIVATE;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			Dee_class_desc_lock_read(my_class);
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				/* Special case: Figure out what property callbacks have been assigned. */
				perm &= ~ATTR_PERMGET;
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
					perm |= ATTR_PERMGET;
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
					if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
						perm |= ATTR_PERMDEL;
					if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
						perm |= ATTR_PERMSET;
				}
			} else {
				DeeObject *obj;
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY))
					perm |= (ATTR_PERMDEL | ATTR_PERMSET);
				obj = my_class->cd_members[attr->ca_addr];
				if (obj) {
					attr_type = Dee_TYPE(obj);
					Dee_Incref(attr_type);
				}
			}
			Dee_class_desc_lock_endread(my_class);
		} else {
			if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)))
				perm |= (ATTR_PERMDEL | ATTR_PERMSET);
		}
		if (attr->ca_doc)
			perm |= ATTR_DOCOBJ;
		temp = (*proc)((DeeObject *)self, DeeString_STR(attr->ca_name),
		               attr->ca_doc ? DeeString_STR(attr->ca_doc) : NULL,
		               perm, attr_type, arg);
		Dee_XDecref(attr_type);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeClass_EnumClassAttributes(DeeTypeObject *__restrict self,
                             denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	size_t i;
	struct class_desc *my_class    = DeeClass_DESC(self);
	DeeClassDescriptorObject *desc = my_class->cd_desc;
	for (i = 0; i <= desc->cd_cattr_mask; ++i) {
		struct class_attribute *attr;
		uint16_t perm;
		DREF DeeTypeObject *attr_type;
		attr = &desc->cd_cattr_list[i];
		if (!attr->ca_name)
			continue;
		attr_type = NULL;
		perm      = (ATTR_CMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
		/* Figure out which instance descriptor the property is connected to. */
		Dee_class_desc_lock_read(my_class);
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
			/* Actually figure out the type of the attr. */
			attr_type = (DREF DeeTypeObject *)my_class->cd_members[attr->ca_addr];
			if (attr_type) {
				attr_type = Dee_TYPE(attr_type);
				Dee_Incref(attr_type);
			}
		}
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
			perm |= (ATTR_PERMDEL | ATTR_PERMSET);
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				perm = (ATTR_CMEMBER | ATTR_NAMEOBJ);
				/* Actually figure out what callbacks are assigned. */
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
					perm |= ATTR_PERMGET;
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
					perm |= ATTR_PERMDEL;
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
					perm |= ATTR_PERMSET;
			}
		}
		Dee_class_desc_lock_endread(my_class);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm |= ATTR_PROPERTY;
		} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
			perm |= ATTR_PERMCALL;
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
			perm |= ATTR_PRIVATE;
		if (attr->ca_doc)
			perm |= ATTR_DOCOBJ;
		temp = (*proc)((DeeObject *)self, DeeString_STR(attr->ca_name),
		               attr->ca_doc ? DeeString_STR(attr->ca_doc) : NULL,
		               perm, attr_type, arg);
		Dee_XDecref(attr_type);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

INTERN WUNUSED NONNULL((1, 3)) dssize_t DCALL
DeeClass_EnumInstanceAttributes(DeeTypeObject *__restrict self,
                                DeeObject *instance,
                                denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	size_t i;
	struct class_desc *my_class    = DeeClass_DESC(self);
	DeeClassDescriptorObject *desc = my_class->cd_desc;
	for (i = 0; i <= desc->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		uint16_t perm;
		DREF DeeTypeObject *attr_type;
		struct instance_desc *inst;
		attr = &desc->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		inst = NULL, attr_type = NULL;
		perm = (ATTR_IMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
		/* Figure out which instance descriptor the property is connected to. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			inst = class_desc_as_instance(my_class);
		} else if (instance) {
			inst = DeeInstance_DESC(my_class, instance);
		}
		if (inst)
			Dee_instance_desc_lock_read(inst);
		if (inst && !(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
			/* Actually figure out the type of the attr. */
			attr_type = (DREF DeeTypeObject *)inst->id_vtab[attr->ca_addr];
			if (attr_type) {
				attr_type = Dee_TYPE(attr_type);
				Dee_Incref(attr_type);
			} else {
				perm &= ~ATTR_PERMGET;
			}
		}
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
			perm |= (ATTR_PERMDEL | ATTR_PERMSET);
			if (inst && attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				perm = (ATTR_IMEMBER | ATTR_NAMEOBJ);
				/* Actually figure out what callbacks are assigned. */
				if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
					perm |= ATTR_PERMGET;
				if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
					perm |= ATTR_PERMDEL;
				if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
					perm |= ATTR_PERMSET;
			}
		}
		if (inst)
			Dee_instance_desc_lock_endread(inst);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm |= ATTR_PROPERTY;
		} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
			perm |= ATTR_PERMCALL;
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
			perm |= ATTR_PRIVATE;
		if (attr->ca_doc)
			perm |= ATTR_DOCOBJ;
		temp = (*proc)((DeeObject *)self, DeeString_STR(attr->ca_name),
		               attr->ca_doc ? DeeString_STR(attr->ca_doc) : NULL,
		               perm, attr_type, arg);
		Dee_XDecref(attr_type);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}


/* Find a specific class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeClass_FindClassAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self,
                            struct attribute_info *__restrict result,
                            struct attribute_lookup_rules const *__restrict rules) {
	struct class_attribute *attr;
	struct class_desc *my_class = DeeClass_DESC(self);
	uint16_t perm;
	DREF DeeTypeObject *attr_type;
	attr = DeeType_QueryClassAttributeStringHash(tp_invoker, self,
	                                             rules->alr_name,
	                                             rules->alr_hash);
	if (!attr)
		goto not_found;
	attr_type = NULL;
	perm      = (ATTR_CMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
	/* Figure out which instance descriptor the property is connected to. */
	Dee_class_desc_lock_read(my_class);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		/* Actually figure out the type of the attr. */
		attr_type = (DREF DeeTypeObject *)my_class->cd_members[attr->ca_addr];
		if (attr_type) {
			attr_type = Dee_TYPE(attr_type);
			Dee_Incref(attr_type);
		}
	}
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		perm |= (ATTR_PERMDEL | ATTR_PERMSET);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm = (ATTR_CMEMBER | ATTR_NAMEOBJ);
			/* Actually figure out what callbacks are assigned. */
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
				perm |= ATTR_PERMGET;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
				perm |= ATTR_PERMDEL;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
				perm |= ATTR_PERMSET;
		}
	}
	Dee_class_desc_lock_endread(my_class);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= ATTR_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= ATTR_PERMCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= ATTR_PRIVATE;
	if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
		Dee_XDecref(attr_type);
not_found:
		return 1;
	}
	result->a_doc      = NULL;
	result->a_decl     = (DREF DeeObject *)self;
	result->a_attrtype = attr_type; /* Inherit reference. */
	if (attr->ca_doc) {
		result->a_doc = DeeString_STR(attr->ca_doc);
		perm |= ATTR_DOCOBJ;
		Dee_Incref(attr->ca_doc);
	}
	result->a_perm = perm;
	Dee_Incref(result->a_decl);
	return 0;
}


/* Find a specific instance-through-class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeClass_FindClassInstanceAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self,
                                    struct attribute_info *__restrict result,
                                    struct attribute_lookup_rules const *__restrict rules) {
	struct class_attribute *attr;
	struct class_desc *my_class = DeeClass_DESC(self);
	uint16_t perm;
	DREF DeeTypeObject *attr_type;
	attr = DeeType_QueryInstanceAttributeStringHash(tp_invoker, self,
	                                                rules->alr_name,
	                                                rules->alr_hash);
	if (!attr)
		goto not_found;
	attr_type = NULL;
	perm = (ATTR_IMEMBER | ATTR_CMEMBER |
	        ATTR_WRAPPER | ATTR_PERMGET |
	        ATTR_NAMEOBJ);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= ATTR_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= ATTR_PERMCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= ATTR_PRIVATE;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		Dee_class_desc_lock_read(my_class);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			/* Special case: Figure out what property callbacks have been assigned. */
			perm &= ~ATTR_PERMGET;
			if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
				perm |= ATTR_PERMGET;
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
					perm |= ATTR_PERMDEL;
				if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
					perm |= ATTR_PERMSET;
			}
		} else {
			DeeObject *obj;
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY))
				perm |= (ATTR_PERMDEL | ATTR_PERMSET);
			obj = my_class->cd_members[attr->ca_addr];
			if (obj) {
				attr_type = Dee_TYPE(obj);
				Dee_Incref(attr_type);
			}
		}
		Dee_class_desc_lock_endread(my_class);
	} else {
		if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)))
			perm |= (ATTR_PERMDEL | ATTR_PERMSET);
	}
	if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
		Dee_XDecref(attr_type);
not_found:
		return 1;
	}
	result->a_decl     = (DREF DeeObject *)self;
	result->a_attrtype = attr_type; /* Inherit reference. */
	result->a_doc      = NULL;
	if (attr->ca_doc) {
		result->a_doc = DeeString_STR(attr->ca_doc);
		perm |= ATTR_DOCOBJ;
		Dee_Incref(attr->ca_doc);
	}
	result->a_perm = perm;
	Dee_Incref(result->a_decl);
	return 0;
}

/* Find a specific instance-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 4, 5)) int DCALL
DeeClass_FindInstanceAttribute(DeeTypeObject *tp_invoker, DeeTypeObject *self, DeeObject *instance,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules) {
	struct class_attribute *attr;
	struct instance_desc *inst;
	struct class_desc *my_class = DeeClass_DESC(self);
	uint16_t perm;
	DREF DeeTypeObject *attr_type;
	attr = DeeType_QueryAttributeStringHash(tp_invoker, self,
	                                        rules->alr_name,
	                                        rules->alr_hash);
	if (!attr)
		goto not_found;
	attr_type = NULL, inst = NULL;
	perm = (ATTR_IMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);

	/* Figure out which instance descriptor the property is connected to. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
		inst = class_desc_as_instance(my_class);
	} else if (instance) {
		inst = DeeInstance_DESC(my_class, instance);
	}
	if (inst)
		Dee_instance_desc_lock_read(inst);
	if (inst && !(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		/* Actually figure out the type of the attr. */
		attr_type = (DREF DeeTypeObject *)inst->id_vtab[attr->ca_addr];
		if (attr_type) {
			attr_type = Dee_TYPE(attr_type);
			Dee_Incref(attr_type);
		} else {
			perm &= ~ATTR_PERMGET;
		}
	}
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		perm |= (ATTR_PERMDEL | ATTR_PERMSET);
		if (inst && attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			perm = (ATTR_IMEMBER | ATTR_NAMEOBJ);
			/* Actually figure out what callbacks are assigned. */
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
				perm |= ATTR_PERMGET;
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
				perm |= ATTR_PERMDEL;
			if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
				perm |= ATTR_PERMSET;
		}
	}
	if (inst)
		Dee_instance_desc_lock_endread(inst);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		perm |= ATTR_PROPERTY;
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		perm |= ATTR_PERMCALL;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
		perm |= ATTR_PRIVATE;
	if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
		Dee_XDecref(attr_type);
not_found:
		return 1;
	}
	result->a_decl     = (DREF DeeObject *)self;
	result->a_attrtype = attr_type; /* Inherit reference. */
	result->a_doc      = NULL;
	if (attr->ca_doc) {
		result->a_doc = DeeString_STR(attr->ca_doc);
		perm |= ATTR_DOCOBJ;
		Dee_Incref(attr->ca_doc);
	}
	result->a_perm = perm;
	Dee_Incref(result->a_decl);
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
	if (!result->p_get && !result->p_del && !result->p_set) {
		DeeObject_FREE(result);
		goto unbound;
	}

	/* Finalize initialization of the property wrapper and return it. */
	DeeObject_Init(result, &DeeProperty_Type);
	return (DREF DeeObject *)result;
unbound:
	err_unbound_attribute_string(class_type, DeeString_STR(attr->ca_name));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeClass_BoundInstanceAttribute(DeeTypeObject *__restrict class_type,
                                struct class_attribute const *__restrict attr) {
	int result;
	struct class_desc *my_class;

	/* Return an instance-wrapper for instance-members. */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		return 1; /* instance-members outside of class memory are
		           * accessed through wrappers, which are always bound. */
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
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttribute(DeeTypeObject *class_type,
                               struct class_attribute const *__restrict attr,
                               size_t argc, DeeObject *const *argv) {
	DREF DeeObject *callback, *result;
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
	result = DeeObject_Call(callback, argc, argv);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute_string(class_type, DeeString_STR(attr->ca_name));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeKw(DeeTypeObject *class_type,
                                 struct class_attribute const *__restrict attr,
                                 size_t argc, DeeObject *const *argv,
                                 DeeObject *kw) {
	DREF DeeObject *callback, *result;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
		DeeObject *thisarg;
		if (DeeArg_UnpackKw(argc, argv, kw, thisarg_kwlist, "o:get", &thisarg))
			goto err;
		if (DeeObject_AssertTypeOrAbstract(thisarg, class_type))
			goto err;
		return DeeInstance_GetAttribute(my_class,
		                                DeeInstance_DESC(my_class, thisarg),
		                                thisarg,
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
	result = DeeObject_CallKw(callback, argc, argv, kw);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute_string(class_type, DeeString_STR(attr->ca_name));
err:
	return NULL;
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeTuple(DeeTypeObject *class_type,
                                    struct class_attribute const *__restrict attr,
                                    DeeObject *args) {
	DREF DeeObject *callback, *result;
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
	result = DeeObject_CallTuple(callback, args);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute_string(class_type, DeeString_STR(attr->ca_name));
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
		DeeObject *thisarg;
		if (DeeArg_UnpackKw(DeeTuple_SIZE(args), DeeTuple_ELEM(args),
		                    kw, thisarg_kwlist, "o:get", &thisarg))
			goto err;
		if (DeeObject_AssertTypeOrAbstract(thisarg, class_type))
			goto err;
		return DeeInstance_GetAttribute(my_class,
		                                DeeInstance_DESC(my_class, thisarg),
		                                thisarg,
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
	err_unbound_attribute_string(class_type, DeeString_STR(attr->ca_name));
err:
	return NULL;
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeClass_VCallInstanceAttributef(DeeTypeObject *class_type,
                                 struct class_attribute const *__restrict attr,
                                 char const *__restrict format, va_list args) {
	DREF DeeObject *callback, *result, *args_tuple;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
		DeeObject *thisarg;
		args_tuple = DeeTuple_VNewf(format, args);
		if unlikely(!args_tuple)
			goto err;
		if (DeeArg_Unpack(DeeTuple_SIZE(args_tuple),
		                  DeeTuple_ELEM(args_tuple),
		                  "o:get", &thisarg))
			goto err_args_tuple;
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
	result = DeeObject_VCallf(callback, format, args);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute_string(class_type, DeeString_STR(attr->ca_name));
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
	if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
		return err_cant_access_attribute_string(class_type,
		                                        DeeString_STR(attr->ca_name),
		                                        ATTR_ACCESS_DEL);
	}
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		DREF DeeObject *old_value;
		/* Simple case: directly delete a class-based attr. */
		Dee_class_desc_lock_write(my_class);
		old_value                           = my_class->cd_members[attr->ca_addr];
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
	return err_cant_access_attribute_string(class_type,
	                                        DeeString_STR(attr->ca_name),
	                                        ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeClass_SetInstanceAttribute(DeeTypeObject *class_type,
                              struct class_attribute const *__restrict attr,
                              DeeObject *value) {
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if unlikely(!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
		goto err_noaccess;

	/* Make sure not to re-write readonly attributes. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
		err_cant_access_attribute_string(class_type,
		                                 DeeString_STR(attr->ca_name),
		                                 ATTR_ACCESS_SET);
		goto err;
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *old_value[3];
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
		old_value[0] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
		old_value[1] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL];
		old_value[2] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET];
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] = ((DeePropertyObject *)value)->p_get;
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] = ((DeePropertyObject *)value)->p_del;
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] = ((DeePropertyObject *)value)->p_set;
		Dee_class_desc_lock_endwrite(my_class);

		/* Drop references from the old callbacks. */
		Dee_XDecref(old_value[2]);
		Dee_XDecref(old_value[1]);
		Dee_XDecref(old_value[0]);
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
	err_cant_access_attribute_string(class_type,
	                                 DeeString_STR(attr->ca_name),
	                                 ATTR_ACCESS_SET);
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
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		         : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		/* Construct a thiscall function. */
		DREF DeeObject *callback;
		Dee_instance_desc_lock_read(self);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!callback)
			goto unbound;
		result = DeeInstanceMethod_New(callback, this_arg);
		Dee_Decref(callback);
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
	err_unbound_attribute_string_c(desc, DeeString_STR(attr->ca_name));
	return NULL;
illegal:
	err_cant_access_attribute_string_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_GET);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_BoundAttribute(struct class_desc *__restrict desc,
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
			goto unbound;

		/* Invoke the getter. */
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		         : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
		if likely(result) {
			Dee_Decref(result);
			return 1;
		}
		if (CATCH_ATTRIBUTE_ERROR())
			return -3;
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return 0;
		return -1;
	} else {
		/* Simply return the attribute as-is. */
		return atomic_read(&self->id_vtab[attr->ca_addr]) != NULL;
	}
unbound:
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_CallAttribute(struct class_desc *__restrict desc,
                          struct instance_desc *__restrict self,
                          DeeObject *this_arg,
                          struct class_attribute const *__restrict attr,
                          size_t argc, DeeObject *const *argv) {
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
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		result = DeeObject_Call(callback, argc, argv);
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
		         ? DeeObject_ThisCall(callback, this_arg, argc, argv)
		         : DeeObject_Call(callback, argc, argv);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_string_c(desc, DeeString_STR(attr->ca_name));
	goto err;
illegal:
	err_cant_access_attribute_string_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_GET);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
DeeInstance_VCallAttributef(struct class_desc *__restrict desc,
                            struct instance_desc *__restrict self,
                            DeeObject *this_arg,
                            struct class_attribute const *__restrict attr,
                            char const *__restrict format, va_list args) {
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
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		result = DeeObject_VCallf(callback, format, args);
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
		         ? DeeObject_VThisCallf(callback, this_arg, format, args)
		         : DeeObject_VCallf(callback, format, args);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_string_c(desc, DeeString_STR(attr->ca_name));
	goto err;
illegal:
	err_cant_access_attribute_string_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_GET);
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
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		result = DeeObject_CallKw(callback, argc, argv, kw);
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
		         ? DeeObject_ThisCallKw(callback, this_arg, argc, argv, kw)
		         : DeeObject_CallKw(callback, argc, argv, kw);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_string_c(desc, DeeString_STR(attr->ca_name));
	goto err;
illegal:
	err_cant_access_attribute_string_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_GET);
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
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);

		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			goto err;
		result = DeeObject_CallTuple(callback, args);
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
		         ? DeeObject_ThisCallTuple(callback, this_arg, args)
		         : DeeObject_CallTuple(callback, args);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_string_c(desc, DeeString_STR(attr->ca_name));
	goto err;
illegal:
	err_cant_access_attribute_string_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_GET);
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
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);

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
	err_unbound_attribute_string_c(desc, DeeString_STR(attr->ca_name));
	goto err;
illegal:
	err_cant_access_attribute_string_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_GET);
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
	if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
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
		       ? DeeObject_ThisCall(delfun, this_arg, 0, NULL)
		       : DeeObject_Call(delfun, 0, NULL);
		Dee_Decref(delfun);
		if unlikely(!temp)
			goto err;
		Dee_Decref(temp);
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
	err_cant_access_attribute_string_c(desc,
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_DEL);
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
		       ? DeeObject_ThisCall(setter, this_arg, 1, (DeeObject **)&value)
		       : DeeObject_Call(setter, 1, (DeeObject **)&value);
		Dee_Decref(setter);
		if unlikely(!temp)
			goto err;
		Dee_Decref(temp);
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
	err_cant_access_attribute_string_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_SET);
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
	if unlikely(old_value) {
		Dee_instance_desc_lock_endwrite(self);
		Dee_Decref_unlikely(old_value);
		return 0;
	}
	Dee_instance_desc_lock_endwrite(self);
	return 0;
illegal:
	return err_cant_access_attribute_string_c(desc,
	                                          DeeString_STR(attr->ca_name),
	                                          ATTR_ACCESS_SET);
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
                                           /*String*/ DeeObject *name, dhash_t hash) {
	struct class_attribute *result;
	dhash_t i, perturb;
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
                                                 char const *__restrict name, dhash_t hash) {
	struct class_attribute *result;
	dhash_t i, perturb;
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
                                                    dhash_t hash) {
	struct class_attribute *result;
	dhash_t i, perturb;
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
                                              /*String*/ DeeObject *name, dhash_t hash) {
	struct class_attribute *result;
	dhash_t i, perturb;
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
                                                    char const *__restrict name, dhash_t hash) {
	struct class_attribute *result;
	dhash_t i, perturb;
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
                                                       size_t attrlen, dhash_t hash) {
	struct class_attribute *result;
	dhash_t i, perturb;
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

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_class_member(/*Class*/ DeeTypeObject *__restrict class_type,
                         struct class_desc *__restrict desc,
                         uint16_t addr) {
	/* Check if we can find the proper member so we can pass its name. */
	size_t i;
	char const *name = "??" "?";
	for (i = 0; i <= desc->cd_desc->cd_cattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &desc->cd_desc->cd_cattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		name = DeeString_STR(attr->ca_name);
		goto got_it;
	}
	for (i = 0; i <= desc->cd_desc->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &desc->cd_desc->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
			continue;
		name = DeeString_STR(attr->ca_name);
		goto got_it;
	}
	/* Throw the error. */
got_it:
	return err_unbound_attribute_string(class_type, name);
}

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_member(/*Class*/ DeeTypeObject *__restrict class_type,
                   struct class_desc *__restrict desc,
                   uint16_t addr) {
	/* Check if we can find the proper member so we can pass its name. */
	size_t i;
	char const *name = "??" "?";
	for (i = 0; i <= desc->cd_desc->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &desc->cd_desc->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
			continue;
		name = DeeString_STR(attr->ca_name);
		break;
	}

	/* Throw the error. */
	return err_unbound_attribute_string(class_type, name);
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
	if (!result)
		err_unbound_member(tp_self, desc, addr);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInstance_BoundMember)(/*Class*/ DeeTypeObject *__restrict tp_self,
                                /*Instance*/ DeeObject *__restrict self,
                                uint16_t addr) {
	struct class_desc *desc;
	struct instance_desc *inst;
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
	ASSERT(DeeType_IsClass(tp_self));
	ASSERT_OBJECT_TYPE_A(self, tp_self);
	desc = DeeClass_DESC(tp_self);
	ASSERT(addr < desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);
	return atomic_read(&inst->id_vtab[addr]) != NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
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
	old_value           = inst->id_vtab[addr];
	inst->id_vtab[addr] = value;
	Dee_instance_desc_lock_endwrite(inst);
	Dee_XDecref(old_value);
}

PRIVATE ATTR_COLD NONNULL((1)) int
(DCALL err_readonly_member_already_bound)(struct class_desc *__restrict desc,
                                          uint16_t addr) {
	/* Check if we can find the proper member so we can pass its name. */
	size_t i;
	char const *name = "??" "?";
	for (i = 0; i <= desc->cd_desc->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &desc->cd_desc->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (addr < attr->ca_addr)
			continue;
		if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1)))
			continue;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
			continue;
		name = DeeString_STR(attr->ca_name);
		break;
	}

	/* Throw the error. */
	return err_cant_access_attribute_string_c(desc, name, ATTR_ACCESS_SET);
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
		return err_readonly_member_already_bound(desc, addr);
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
	return DeeInstance_BoundMember(tp_self, self, addr);
err_bad_index:
	return err_invalid_instance_addr(tp_self, self, addr);
err_req_class:
	return err_requires_class(tp_self);
err:
	return -1;
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
	Dee_XIncref(result);
	Dee_class_desc_lock_endread(desc);
	if unlikely(!result)
		err_unbound_class_member(self, desc, addr);
	return result;
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
	if (DeeObject_AssertType(self, &DeeType_Type))
		goto err;
	if (!DeeType_IsClass(self))
		goto err_req_class;
	if (addr >= DeeClass_DESC(self)->cd_desc->cd_cmemb_size)
		goto err_bad_index;
	return DeeClass_BoundMember(self, addr);
err_bad_index:
	return err_invalid_class_addr(self, addr);
err_req_class:
	return err_requires_class(self);
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CLASS_DESC_C */
