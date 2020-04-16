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
#ifndef GUARD_DEEMON_OBJECTS_CLASS_DESC_C
#define GUARD_DEEMON_OBJECTS_CLASS_DESC_C 1

#include <deemon/instancemethod.h>
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/property.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeClassDescriptorObject ClassDescriptor;

INTERN struct class_operator empty_class_operators[] = {
	{
		/* .co_name = */ (uint16_t)-1,
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

INTDEF DeeTypeObject ClassOperatorTableIterator_Type;
INTDEF DeeTypeObject ClassOperatorTable_Type;

#ifdef CONFIG_NO_THREADS
#define COTI_GETITER(x) (x)->co_iter
#else /* CONFIG_NO_THREADS */
#define COTI_GETITER(x) ATOMIC_READ((x)->co_iter)
#endif /* !CONFIG_NO_THREADS */

LOCAL struct class_operator *DCALL
coti_next_ent(ClassOperatorTableIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	struct class_operator *result;
	result = self->co_iter;
	for (;; ++result) {
		if (result >= self->co_end)
			return NULL;
		if (result->co_name != (uint16_t)-1)
			break;
	}
	self->co_iter = result + 1;
#else /* CONFIG_NO_THREADS */
	struct class_operator *result, *start;
	for (;;) {
		result = start = ATOMIC_READ(self->co_iter);
		for (;; ++result) {
			if (result >= self->co_end)
				return NULL;
			if (result->co_name != (uint16_t)-1)
				break;
		}
		if (ATOMIC_CMPXCH_WEAK(self->co_iter, start, result + 1))
			break;
	}
#endif /* !CONFIG_NO_THREADS */
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
coti_next(ClassOperatorTableIterator *__restrict self) {
	struct class_operator *ent;
	struct opinfo *info;
	ent = coti_next_ent(self);
	if (!ent)
		return ITER_DONE;
	info = Dee_OperatorInfo(NULL, ent->co_name);
	if (info) {
		return DeeTuple_Newf("s" DEE_FMT_UINT16,
		                     info->oi_sname,
		                     ent->co_addr);
	}
	return DeeTuple_Newf(DEE_FMT_UINT16
	                     DEE_FMT_UINT16,
	                     ent->co_name,
	                     ent->co_addr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
coti_next_key(ClassOperatorTableIterator *__restrict self) {
	struct class_operator *ent;
	struct opinfo *info;
	ent = coti_next_ent(self);
	if (!ent)
		return ITER_DONE;
	info = Dee_OperatorInfo(NULL, ent->co_name);
	if (info)
		return DeeString_New(info->oi_sname);
	return DeeInt_NewU16(ent->co_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
coti_next_value(ClassOperatorTableIterator *__restrict self) {
	struct class_operator *ent;
	ent = coti_next_ent(self);
	if (!ent)
		return ITER_DONE;
	return DeeInt_NewU16(ent->co_addr);
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

#define DEFINE_COTI_COMPARE(name, op)                                                        \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                                    \
	name(ClassOperatorTableIterator *self,                                                   \
	     ClassOperatorTableIterator *other) {                                                \
		if (DeeObject_AssertTypeExact((DeeObject *)other, &ClassOperatorTableIterator_Type)) \
			goto err;                                                                        \
		return_bool(COTI_GETITER(self) op COTI_GETITER(other));                              \
	err:                                                                                     \
		return NULL;                                                                         \
	}
DEFINE_COTI_COMPARE(coti_eq, ==)
DEFINE_COTI_COMPARE(coti_ne, !=)
DEFINE_COTI_COMPARE(coti_lo, <)
DEFINE_COTI_COMPARE(coti_le, <=)
DEFINE_COTI_COMPARE(coti_gr, >)
DEFINE_COTI_COMPARE(coti_ge, >=)
#undef DEFINE_COTI_COMPARE

PRIVATE struct type_cmp coti_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&coti_ge,
};

PRIVATE struct type_getset coti_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&coti_getseq, NULL, NULL,
	  DOC("->?Aoperators?Ert:ClassDescriptor") },
	{ NULL }
};

PRIVATE struct type_member coti_members[] = {
	TYPE_MEMBER_FIELD_DOC("__class__", STRUCT_OBJECT,
	                      offsetof(ClassOperatorTableIterator, co_desc),
	                      "->?Ert:ClassDescriptor"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject ClassOperatorTableIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassOperatorTableIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ (void *)&coti_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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


STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTable, co_desc) ==
              COMPILER_OFFSETOF(ClassOperatorTableIterator, co_desc));
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
cot_nsi_getsize(ClassOperatorTable *__restrict self) {
	uint16_t i;
	size_t result         = 0;
	ClassDescriptor *desc = self->co_desc;
	for (i = 0; i <= desc->cd_clsop_mask; ++i) {
		if (desc->cd_clsop_list[i].co_name != (uint16_t)-1)
			++result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cot_size(ClassOperatorTable *__restrict self) {
	return DeeInt_NewSize(cot_nsi_getsize(self));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
cot_getitemdef(ClassOperatorTable *__restrict self,
               DeeObject *__restrict key,
               DeeObject *__restrict defl) {
	uint16_t opname, i, perturb;
	ClassDescriptor *desc = self->co_desc;
	if (DeeString_Check(key)) {
		opname = Dee_OperatorFromName(NULL, DeeString_STR(key));
		if (opname == (uint16_t)-1)
			goto nope;
	} else {
		if (DeeObject_AsUInt16(key, &opname))
			goto err;
	}
	i = perturb = opname & desc->cd_clsop_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_operator *op;
		op = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
		if (op->co_name == (uint16_t)-1)
			break;
		if (op->co_name != opname)
			continue;
		return DeeInt_NewU16(op->co_addr);
	}
nope:
	if (defl != ITER_DONE)
		Dee_Incref(defl);
	return defl;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cot_getitem(ClassOperatorTable *self,
            DeeObject *key) {
	DREF DeeObject *result;
	result = cot_getitemdef(self, key, ITER_DONE);
	if (result == ITER_DONE) {
		err_unknown_key((DeeObject *)self, key);
		result = NULL;
	}
	return result;
}

PRIVATE struct type_nsi cot_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (void *)&cot_nsi_getsize,
			/* .nsi_nextkey    = */ (void *)&coti_next_key,
			/* .nsi_nextvalue  = */ (void *)&coti_next_value,
			/* .nsi_getdefault = */ (void *)&cot_getitemdef,
			/* .nsi_setdefault = */ (void *)NULL,
			/* .nsi_updateold  = */ (void *)NULL,
			/* .nsi_insertnew  = */ (void *)NULL
		}
	}
};

PRIVATE struct type_seq cot_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cot_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cot_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cot_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &cot_nsi
};

PRIVATE struct type_member cot_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &ClassOperatorTableIterator_Type),
	TYPE_MEMBER_END
};

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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(ClassOperatorTable)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cot_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cot_bool
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
	DREF ClassDescriptor   *ca_desc; /* [1..1][const] Class descriptor. */
	struct class_attribute *ca_attr; /* [1..1][const] The attribute that was queried. */
} ClassAttribute;

typedef struct {
	OBJECT_HEAD
	DREF ClassDescriptor         *ca_desc; /* [1..1][const] Class descriptor. */
	DWEAK struct class_attribute *ca_iter; /* [1..1] Current iterator position. */
	struct class_attribute       *ca_end;  /* [1..1][const] Iterator end. */
} ClassAttributeTableIterator;

typedef struct {
	OBJECT_HEAD
	DREF ClassDescriptor   *ca_desc;  /* [1..1][const] Class descriptor. */
	struct class_attribute *ca_start; /* [1..1][const] Hash-vector starting pointer. */
	size_t                  ca_mask;  /* [const] Mask-vector size mask. */
} ClassAttributeTable;

INTDEF DeeTypeObject ClassAttribute_Type;
INTDEF DeeTypeObject ClassAttributeTable_Type;
INTDEF DeeTypeObject ClassAttributeTableIterator_Type;

PRIVATE WUNUSED NONNULL((1, 2)) DREF ClassAttribute *DCALL
cattr_new(ClassDescriptor *__restrict desc,
          struct class_attribute *__restrict attr) {
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

STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTable, co_desc) ==
              COMPILER_OFFSETOF(ClassAttribute, ca_desc));
#define ca_fini    cot_fini
#define ca_visit   cot_visit
#define ca_members cot_members
STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTable, co_desc) ==
              COMPILER_OFFSETOF(ClassAttributeTable, ca_desc));
#define cat_fini    cot_fini
#define cat_visit   cot_visit
#define cat_members cot_members
STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTable, co_desc) ==
              COMPILER_OFFSETOF(ClassAttributeTableIterator, ca_desc));
#define cati_fini    cot_fini
#define cati_visit   cot_visit
#define cati_members cot_members

STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTableIterator, co_desc) == COMPILER_OFFSETOF(ClassAttributeTableIterator, ca_desc));
STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTableIterator, co_iter) == COMPILER_OFFSETOF(ClassAttributeTableIterator, ca_iter));
STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTableIterator, co_end) == COMPILER_OFFSETOF(ClassAttributeTableIterator, ca_end));
#define cati_cmp  coti_cmp
#define cati_copy coti_copy

#ifdef CONFIG_NO_THREADS
#define CATI_GETITER(x) (x)->ca_iter
#else /* CONFIG_NO_THREADS */
#define CATI_GETITER(x) ATOMIC_READ((x)->ca_iter)
#endif /* !CONFIG_NO_THREADS */

LOCAL struct class_attribute *DCALL
cati_next_ent(ClassAttributeTableIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	struct class_attribute *result;
	result = self->ca_iter;
	for (;; ++result) {
		if (result >= self->ca_end)
			return NULL;
		if (result->co_name != NULL)
			break;
	}
	self->co_iter = result + 1;
#else /* CONFIG_NO_THREADS */
	struct class_attribute *result, *start;
	for (;;) {
		result = start = ATOMIC_READ(self->ca_iter);
		for (;; ++result) {
			if (result >= self->ca_end)
				return NULL;
			if (result->ca_name != NULL)
				break;
		}
		if (ATOMIC_CMPXCH_WEAK(self->ca_iter, start, result + 1))
			break;
	}
#endif /* !CONFIG_NO_THREADS */
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cati_next(ClassAttributeTableIterator *__restrict self) {
	DREF DeeObject *result;
	DREF ClassAttribute *attr;
	struct class_attribute *ent;
	ent = cati_next_ent(self);
	if (!ent)
		return ITER_DONE;
	attr = cattr_new(self->ca_desc, ent);
	if unlikely(!attr)
		return NULL;
	result = DeeTuple_Pack(2, ent->ca_name, attr);
	Dee_Decref(attr);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cati_next_key(ClassAttributeTableIterator *__restrict self) {
	struct class_attribute *ent;
	ent = cati_next_ent(self);
	if (!ent)
		return ITER_DONE;
	return_reference_((DeeObject *)ent->ca_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF ClassAttribute *DCALL
cati_next_value(ClassAttributeTableIterator *__restrict self) {
	struct class_attribute *ent;
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
cat_nsi_getsize(ClassAttributeTable *__restrict self) {
	size_t i, result = 0;
	for (i = 0; i <= self->ca_mask; ++i) {
		if (self->ca_desc[i].cd_name != NULL)
			++result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cat_size(ClassAttributeTable *__restrict self) {
	return DeeInt_NewSize(cat_nsi_getsize(self));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
cat_getitemdef(ClassAttributeTable *__restrict self,
               DeeObject *__restrict key,
               DeeObject *__restrict defl) {
	dhash_t hash, i, perturb;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	hash = DeeString_Hash(key);
	i = perturb = hash & self->ca_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_attribute *at;
		at = &self->ca_start[i & self->ca_mask];
		if (at->ca_name == NULL)
			break;
		if (at->ca_hash != hash)
			continue;
		if (DeeString_SIZE(at->ca_name) != DeeString_SIZE(key))
			continue;
		if (memcmp(DeeString_STR(at->ca_name), DeeString_STR(key),
		           DeeString_SIZE(key) * sizeof(char)) != 0)
			continue;
		return (DREF DeeObject *)cattr_new(self->ca_desc, at);
	}
/*nope:*/
	if (defl != ITER_DONE)
		Dee_Incref(defl);
	return defl;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cat_getitem(ClassAttributeTable *self,
            DeeObject *key) {
	DREF DeeObject *result;
	result = cat_getitemdef(self, key, ITER_DONE);
	if (result == ITER_DONE) {
		err_unknown_key((DeeObject *)self, key);
		result = NULL;
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

PRIVATE struct type_getset cati_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cati_getseq, NULL, NULL,
	  DOC("->?Aattributes?Ert:ClassDescriptor") },
	{ NULL }
};

PRIVATE struct type_member cat_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &ClassAttributeTableIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_str(ClassAttribute *__restrict self) {
	DeeStringObject *cnam = self->ca_desc->cd_name;
	if (cnam)
		return DeeString_Newf("%k.%k", cnam, self->ca_attr->ca_name);
	return_reference_((DeeObject *)self->ca_attr->ca_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ca_repr(ClassAttribute *__restrict self) {
	ClassDescriptor *desc = self->ca_desc;
	DeeStringObject *cnam = desc->cd_name;
	char field_id         = 'c';
	if (self->ca_attr >= desc->cd_iattr_list &&
	    self->ca_attr <= desc->cd_iattr_list + desc->cd_iattr_mask)
		field_id = 'i';
	if (cnam)
		return DeeString_Newf("%k.__class__.%cattr[%r]", cnam, field_id, self->ca_attr->ca_name);
	return DeeString_Newf("<anonymous>.__class__.%cattr[%r]", field_id, self->ca_attr->ca_name);
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
	return DeeInt_NewU16(self->ca_attr->ca_addr);
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


PRIVATE struct type_getset ca_getsets[] = {
	{ "name", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getname, NULL, NULL, DOC("->?Dstring") },
	{ "doc", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getdoc, NULL, NULL, DOC("->?X2?Dstring?N") },
	{ "addr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getaddr, NULL, NULL,
	  DOC("->?Dint\n"
	      "Index into the class/instance object table, where @this attribute is stored\n"
	      "When #isclassmem or #isclassns are :true, this index and any index offset from it "
	      "refer to the class object table. Otherwise, the instance object table is dereferenced\n"
	      "This is done so-as to allow instance attributes such as member functions to be stored "
	      "within the class itself, rather than having to be copied into each and every instance "
	      "of the class\n"
	      "S.a. :Type.__ctable__ and :Type.__itable__") },
	{ "isprivate", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isprivate, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Evaluates to :true if @this class attribute was declared as $private") },
	{ "isfinal", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isfinal, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Evaluates to :true if @this class attribute was declared as $final") },
	{ "isreadonly", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isreadonly, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Evaluates to :true if @this class attribute can only be read from\n"
	      "When this is case, a property-like attribute can only ever have a getter "
	      "associated with itself, while field- or method-like attribute can only be "
	      "written once (aka. when not already bound)") },
	{ "ismethod", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_ismethod, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Evaluates to :true if @this class attribute referrs to a method\n"
	      "When set, reading from the attribute will return a an object "
	      "${InstanceMethod(obj.MEMBER_TABLE[this.addr],obj)}\n"
	      "Note however that this is rarely ever required to be done, as method attributes "
	      "are usually called directly, in which case a callattr instruction can silently "
	      "prepend the this-argument before the passed argument list") },
	{ "isproperty", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isproperty, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Evaluates to :true if @this class attribute was defined as a property\n"
	      "When this is the case, a #readonly attribute only has a single callback "
	      "that may be stored at #addr + 0, with that callback being the getter\n"
	      "Otherwise, up to 3 indices within the associated object table are used by "
	      "@this attribute, each of which may be left unbound:\n"
	      "%{table Offset|Callback\n"
	      "$" PP_STR(CLASS_GETSET_GET) "|The getter callback ($get)\n"
	                                   "$" PP_STR(CLASS_GETSET_DEL) "|The delete callback ($del)\n"
	                                                                "$" PP_STR(CLASS_GETSET_SET) "|The setter callback ($set)}") },
	{ "isclassmem", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isclassmem, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Set if #addr is an index into the class object table, rather than into the "
	      "instance object table. Note however that when #isclassns") },
	{ "isclassns", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getisclassns, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this class attribute is exclusive to the "
	      "class-namespace (i.e. was declared as $static)\n"
	      "During enumeration of attributes, all attributes where this is :true "
	      "are enumated by :ClassDescriptor.cattr, while all for which it isn't "
	      "are enumated by :ClassDescriptor.iattr") },
	{ "flags", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getflags, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns a comma-separated string describing the flags of @this class attribute\n"
	      "%{table Flag|Property\n"
	      "$\"private\"|#isprivate\n"
	      "$\"final\"|#isfinal\n"
	      "$\"readonly\"|#isreadonly\n"
	      "$\"method\"|#ismethod\n"
	      "$\"property\"|#isproperty\n"
	      "$\"classns\"|#isclassns}\n") },
	{ NULL }
};


PRIVATE struct type_nsi cat_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (void *)&cat_nsi_getsize,
			/* .nsi_nextkey    = */ (void *)&cati_next_key,
			/* .nsi_nextvalue  = */ (void *)&cati_next_value,
			/* .nsi_getdefault = */ (void *)&cat_getitemdef,
			/* .nsi_setdefault = */ (void *)NULL,
			/* .nsi_updateold  = */ (void *)NULL,
			/* .nsi_insertnew  = */ (void *)NULL
		}
	}
};

PRIVATE struct type_seq cat_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cat_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &cat_nsi
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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(ClassAttribute)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ca_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_repr,
		/* .tp_bool = */ NULL
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
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ (void *)&cati_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(ClassAttributeTable)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cat_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cat_bool
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
	if (DeeString_SIZE(lhs->ca_name) !=
	    DeeString_SIZE(rhs->ca_name))
		goto nope;
	if (memcmp(DeeString_STR(lhs->ca_name),
	           DeeString_STR(rhs->ca_name),
	           DeeString_SIZE(lhs->ca_name) * sizeof(char)) != 0)
		goto nope;
	if (lhs->ca_doc) {
		if (DeeString_SIZE(lhs->ca_doc) !=
		    DeeString_SIZE(rhs->ca_doc))
			goto nope;
		if (memcmp(DeeString_STR(lhs->ca_doc),
		           DeeString_STR(rhs->ca_doc),
		           DeeString_SIZE(lhs->ca_doc) * sizeof(char)) != 0)
			goto nope;
	}
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
	if (self->cd_clsop_mask != other->cd_clsop_mask)
		goto nope;
	if (self->cd_cattr_mask != other->cd_cattr_mask)
		goto nope;
	if (self->cd_iattr_mask != other->cd_iattr_mask)
		goto nope;
	if (self->cd_name) {
		if (!other->cd_name)
			goto nope;
		if (DeeString_SIZE(self->cd_name) != DeeString_SIZE(other->cd_name))
			goto nope;
		if (memcmp(DeeString_STR(self->cd_name),
		           DeeString_STR(other->cd_name),
		           DeeString_SIZE(other->cd_name) * sizeof(char)) != 0)
			goto nope;
	} else {
		if (other->cd_name)
			goto nope;
	}
	if (self->cd_doc) {
		if (!other->cd_doc)
			goto nope;
		if (DeeString_SIZE(self->cd_doc) != DeeString_SIZE(other->cd_doc))
			goto nope;
		if (memcmp(DeeString_STR(self->cd_doc),
		           DeeString_STR(other->cd_doc),
		           DeeString_SIZE(other->cd_doc) * sizeof(char)) != 0)
			goto nope;
	} else {
		if (other->cd_doc)
			goto nope;
	}
	if (memcmp(self->cd_clsop_list,
	           other->cd_clsop_list,
	           (self->cd_clsop_mask + 1) *
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
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cd_eq
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
cd_sizeof(ClassDescriptor *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	return DeeInt_NewSize(offsetof(ClassDescriptor, cd_iattr_list) +
	                      ((self->cd_iattr_mask + 1) * sizeof(struct class_attribute)) +
	                      (self->cd_cattr_list == empty_class_attributes ? 0 : (self->cd_cattr_mask + 1) * sizeof(struct class_attribute)) +
	                      (self->cd_clsop_list == empty_class_operators ? 0 : (self->cd_clsop_mask + 1) * sizeof(struct class_operator)));
err:
 return NULL;
}

PRIVATE struct type_method cd_methods[] = {
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&cd_sizeof,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_getset cd_getsets[] = {
	{ "flags",
	  (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_getflags, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Return a comma-separated string of flags used to describe the combination of properties described by "
	      "#isfinal, #isinterrupt, #hassuperconstructor, #__hassuperkwds__, #__isinttruncated__, and #__hasmoveany__\n"
	      "%{table Flag|Property\n"
	      "$\"final\"|#isfinal\n"
	      "$\"interrupt\"|#isinterrupt\n"
	      "$\"superctor\"|#hassuperconstructor\n"
	      "$\"superkwds\"|#__hassuperkwds__\n"
	      "$\"autoinit\"|#__hasautoinit__\n"
	      "$\"inttrunc\"|#__isinttruncated__\n"
	      "$\"moveany\"|#__hasmoveany__\n}") },
	{ DeeString_STR(&str_operators),
	  (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_operators, NULL, NULL,
	  DOC("->?#OperatorTable\n"
	      "Enumerate operators implemented by @this class, as well as their associated "
	      "class object table indices which are holding the respective implementations\n"
	      "Note that the class object table entry may be left unbound to explicitly "
	      "define an operator as having been deleted") },
	{ "iattr",
	  (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_iattr, NULL, NULL,
	  DOC("->?#AttributeTable\n"
	      "Enumerate user-defined instance attributes as a mapping-like :deemon:string-#Attribute sequence") },
	{ "cattr",
	  (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_cattr, NULL, NULL,
	  DOC("->?#AttributeTable\n"
	      "Enumerate user-defined class ($static) attributes as a mapping-like :deemon:string-#Attribute sequence") },
	{ NULL }
};

PRIVATE struct type_member cd_members[] = {
	TYPE_MEMBER_BITFIELD("isfinal", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FFINAL),
	TYPE_MEMBER_BITFIELD_DOC("isinterrupt", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FINTERRUPT,
	                         "Evaluates to :true if @this class behaves as an interrupt exception when thrown\n"
	                         "An interrupt exception (such as :Signal.Interrupt) is not caught by ${catch(...)} "
	                         "statements, but only by statements marked as ${@[interrupt] catch(...)}\n"
	                         "Certain types exceptions require this in order to prevent catch-all blocks surrounding "
	                         "optional function calls such as invocations of :fs:unlink from accidentally handling "
	                         "unwanted types of exceptions such as :Signal.Interrupt.KeyboardInterrupt, as caused "
	                         "by the user pressing CTRL+C to terminate the running script, and (normally) not "
	                         "expecting it to continue running because the error was silently swallowed by an "
	                         "unrelated catch-all block"),
	TYPE_MEMBER_BITFIELD_DOC("hassuperconstructor", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FINHERITCTOR,
	                         "Evaluates to :true if @this class inherits its constructor from its base-type\n"
	                         "In user-defined classes, this behavior is encoded as ${this = super;}"),
#ifdef CLASS_TP_FSUPERKWDS
	TYPE_MEMBER_BITFIELD_DOC("__hassuperkwds__", STRUCT_CONST, ClassDescriptor, cd_flags, CLASS_TP_FSUPERKWDS,
	                         "Evaluates to :true if the super-args operator of @this class returns a tuple (args,kwds) "
	                         "that should be used to invoke the super-constructor as ${super(args...,**kwds)}\n"
	                         "Otherwise, the super-args operator simply returns args and the super-constructor "
	                         "is called as ${super(args...)}"),
#endif /* CLASS_TP_FSUPERKWDS */
#ifdef CLASS_TP_FAUTOINIT
	TYPE_MEMBER_BITFIELD_DOC("__hasautoinit__", STRUCT_CONST, ClassDescriptor, cd_flags, CLASS_TP_FAUTOINIT,
	                         "Evaluates to :true if @this class provides an automatic initializer and ${operator repr}\n"
	                         "This is used to implement the user-code ${this = default;} constructor definition"),
#endif /* CLASS_TP_FAUTOINIT */
	TYPE_MEMBER_BITFIELD("__isinttruncated__", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FTRUNCATE),
	TYPE_MEMBER_BITFIELD("__hasmoveany__", STRUCT_CONST, ClassDescriptor, cd_flags, TP_FMOVEANY),
	TYPE_MEMBER_FIELD_DOC("__name__", STRUCT_OBJECT_OPT, offsetof(ClassDescriptor, cd_name), "->?X2?Dstring?N"),
	TYPE_MEMBER_FIELD_DOC("__doc__", STRUCT_OBJECT_OPT, offsetof(ClassDescriptor, cd_doc), "->?X2?Dstring?N"),
	TYPE_MEMBER_FIELD_DOC("__csize__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(ClassDescriptor, cd_cmemb_size), "Size of the class object table"),
	TYPE_MEMBER_FIELD_DOC("__isize__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(ClassDescriptor, cd_imemb_size), "Size of the instance object table"),
	TYPE_MEMBER_END
};

INTDEF DeeTypeObject ObjectTable_Type;
PRIVATE struct type_member cd_class_members[] = {
	TYPE_MEMBER_CONST("Attribute", &ClassAttribute_Type),
	TYPE_MEMBER_CONST("AttributeTable", &ClassAttributeTable_Type),
	TYPE_MEMBER_CONST("OperatorTable", &ClassOperatorTable_Type),
	TYPE_MEMBER_CONST("ObjectTable", &ObjectTable_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cd_str(ClassDescriptor *__restrict self) {
	if (self->cd_name)
		return_reference_((DeeObject *)self->cd_name);
	return DeeString_New("<anonymous>");
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
		if (DeeInt_AsU16(data, &self->ca_addr))
			goto err;
		self->ca_doc  = NULL;
		self->ca_flag = CLASS_ATTRIBUTE_FPUBLIC;
		return 0;
	}
	doc       = NULL;
	fast_size = DeeFastSeq_GetSize(data);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		if (fast_size != 2 && fast_size != 3) {
			err_invalid_unpack_size_minmax(data, 2, 3, fast_size);
			goto err;
		}
		addr = DeeFastSeq_GetItem(data, 0);
		if unlikely(!addr)
			goto err;
		flags = DeeFastSeq_GetItem(data, 1);
		if unlikely(!addr)
			goto err_addr;
		if (fast_size >= 3) {
			doc = DeeFastSeq_GetItem(data, 2);
			if unlikely(!addr)
				goto err_addr_flags;
		}
	} else {
		iter = DeeObject_IterSelf(data);
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
		pos           = DeeString_STR(flags);
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
					if (memcmp(class_attribute_flags_db[i].fe_name, pos, flag_len * sizeof(char)) != 0)
						continue;
					self->ca_flag |= class_attribute_flags_db[i].fe_flag;
					goto got_flag;
				}
			}
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid flag %$q for %s-attribute %r",
			                flag_len, pos,
			                is_class_attribute ? "class" : "instance",
			                self->ca_name);
			goto err_addr_flags_doc;
got_flag:
			if (!next)
				break;
			pos = next + 1;
		}
	} else {
		if (DeeObject_AsUInt16(addr, &self->ca_flag))
			goto err_addr_flags_doc;
		if (self->ca_flag & ~CLASS_ATTRIBUTE_FMASK) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Invalid flags for %s-attribute %r (0x%.4I16x)",
			                is_class_attribute ? "class" : "instance",
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
	new_map = (struct class_attribute *)Dee_Calloc((new_mask + 1) *
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
	iterator = DeeObject_IterSelf(iattr);
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
			if (DeeString_SIZE(ent->ca_name) != DeeString_SIZE(data[0]))
				continue;
			if (memcmp(DeeString_STR(ent->ca_name),
			           DeeString_STR(data[0]),
			           DeeString_SIZE(ent->ca_name)) != 0)
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
					                "Instance attribute %r uses out-of-bounds class object table index %I16u (>= %I16u)",
					                ent->ca_name, maxid, cmemb_size);
					goto err_iter_r;
				}
				if (result->cd_cmemb_size <= maxid)
					result->cd_cmemb_size = maxid + 1;
			} else {
				if (imemb_size != (uint16_t)-1 && maxid >= imemb_size) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Instance attribute %r uses out-of-bounds object table index %I16u (>= %I16u)",
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

PRIVATE int DCALL
cd_add_operator(ClassDescriptor *__restrict self,
                uint16_t name, uint16_t index,
                uint16_t *__restrict operator_count) {
	uint16_t i, perturb, mask;
	struct class_operator *map, *ent;
	if (name == (uint16_t)-1)
		goto err_invalid_name;
	if (*operator_count >= (self->cd_clsop_mask / 3) * 2) {
		mask = (self->cd_clsop_mask << 1) | 1;
		if (mask <= 1)
			mask = 3;
		map = (struct class_operator *)Dee_Malloc((mask + 1) *
		                                          sizeof(struct class_operator));
		if unlikely(!map)
			goto err;
		memset(map, 0xff, (mask + 1) * sizeof(struct class_operator));
		for (i = 0; i <= self->cd_clsop_mask; ++i) {
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
		if (ent->co_name == (uint16_t)-1)
			break;
	}
	ent->co_name = name;
	ent->co_addr = index;
	return 0;
err_duplicate_name: {
	struct opinfo *op = Dee_OperatorInfo(NULL, name);
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
	PRIVATE DEFINE_KWLIST(kwlist, { K(name), K(doc), K(flags), K(operators), K(iattr), K(cattr), K(isize), K(csize), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|oooooI16uI16u",
	                    &class_name, &class_doc,
	                    &class_flags, &class_operators,
	                    &class_iattr, &class_cattr,
	                    &class_isize, &class_csize))
		goto err;
	if (DeeObject_AssertType((DeeObject *)class_name, &DeeString_Type))
		goto err;
	if (class_doc &&
	    DeeObject_AssertType((DeeObject *)class_doc, &DeeString_Type))
		goto err;

	result = cd_alloc_from_iattr(class_iattr, class_isize, class_csize);
	if unlikely(!result)
		goto err;
	result->cd_flags = TP_FNORMAL;
	if (class_flags != (DeeStringObject *)Dee_EmptyString) {
		if (DeeString_Check(class_flags)) {
			char *pos;
			pos = DeeString_STR(class_flags);
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
						if (memcmp(class_flags_db[i].fe_name, pos, flag_len * sizeof(char)) != 0)
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
		iterator         = DeeObject_IterSelf(class_cattr);
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
				if (DeeString_SIZE(ent->ca_name) != DeeString_SIZE(data[0]))
					continue;
				if (memcmp(DeeString_STR(ent->ca_name), DeeString_STR(data[0]), DeeString_SIZE(data[0])) != 0)
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
					                "Class attribute %r uses out-of-bounds class object table index %I16u (>= %I16u)",
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
		uint16_t operator_count = 0;
		iterator                = DeeObject_IterSelf(class_operators);
		if unlikely(!iterator)
			goto err_r_imemb_cmemb;
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			uint16_t name, index;
			if (DeeObject_Unpack(elem, 2, data))
				goto err_r_imemb_iter_elem;
			Dee_Decref(elem);
			if (DeeObject_AsUInt16(data[1], &index))
				goto err_r_imemb_iter_data;
			if (DeeString_Check(data[0])) {
				name = Dee_OperatorFromName(NULL, DeeString_STR(data[0]));
				if (name == (uint16_t)-1) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Unknown operator %r",
					                data[0]);
					goto err_r_imemb_iter_data;
				}
			} else {
				if (DeeObject_AsUInt16(data[0], &name))
					goto err_r_imemb_iter_data;
			}
			Dee_Decref(data[1]);
			Dee_Decref(data[0]);
			if (class_csize != (uint16_t)-1 && index >= class_csize) {
				struct opinfo *op = Dee_OperatorInfo(NULL, name);
				if (op) {
					DeeError_Throwf(&DeeError_ValueError,
					                "Operator %s uses out-of-bounds class object table index %I16u (>= %I16u)",
					                op->oi_sname, index, class_csize);
				} else {
					DeeError_Throwf(&DeeError_ValueError,
					                "Operator 0x%.4I16x uses out-of-bounds class object table index %I16u (>= %I16u)",
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
	                         "operators:?S?T2?X2?Dstring?Dint?Dint=!T0,"
	                         "iattr:?S?T2?Dstring?X3?Dint?T2?Dint?X2?Dstring?Dint?T3?Dint?X2?Dstring?Dint?Dstring=!T0,"
	                         "cattr:?S?T2?Dstring?X3?Dint?T2?Dint?X2?Dstring?Dint?T3?Dint?X2?Dstring?Dint?Dstring=!T0,"
	                         "isize?:?Dint,csize?:?Dint)\n"
	                         "@throw ValueError Some operator or attribute was defined multiple times\n"
	                         "@throw ValueError A specified operator name wasn't recognized (custom operators must be encoded as IDs)\n"
	                         "@throw ValueError A specified set of flags contains an invalid option\n"
	                         "@throw ValueError An attribute or operator is bound to an out-of-bounds object table index\n"
	                         "@throw IntergerOverflow A used object table index exceeds the hard limit of $0xffff (unsigned 16-bit)\n"
	                         "Create a new class descriptor\n"
	                         "The given @flags is a comma-separated string of flags as described in #flags\n"
	                         "The given @isize and @csize determine the allocated sizes of the instance class "
	                         "member tables. - When omitted, these sizes are automatically calculated by "
	                         "determining the greatest used table indices within @operators, @iattr and @cattr\n"
	                         "Note that both @iattr and @cattr take mappings of attribute names to one either "
	                         "the associated table_index, or a tuple of (table_index,flags[,doc]), where flags is "
	                         "a comma-separated string of flags as described in #Attribute.flags\n"
	                         "Hint: Once created, a _ClassDescriptor object can be used "
	                         "with :rt:makeclass to create custom class types at runtime"),
	/* .tp_flags    = */ TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor        = */ NULL,
				/* .tp_copy_ctor   = */ &DeeObject_NewRef,
				/* .tp_deep_ctor   = */ &DeeObject_NewRef,
				/* .tp_any_ctor    = */ NULL,
				/* .tp_free        = */ NULL,
				/* .tp_pad         = */ { 0 },
				/* .tp_any_ctor_kw = */ &cd_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cd_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_str,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
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

STATIC_ASSERT(COMPILER_OFFSETOF(ObjectTable, ot_owner) ==
              COMPILER_OFFSETOF(ClassAttributeTable, ca_desc));
#define ot_fini    cot_fini

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_str(ObjectTable *__restrict self) {
	DeeTypeObject *tp = DeeObject_Class(self->ot_owner);
	if (DeeType_IsTypeType(tp))
		return DeeString_Newf("<class object table for %k>", tp);
	return DeeString_Newf("<object table for instance of %k>", tp);
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
ot_nsi_getsize(ObjectTable *__restrict self) {
	return self->ot_size;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_size(ObjectTable *__restrict self) {
	return DeeInt_NewU16(self->ot_size);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_nsi_getitem(ObjectTable *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->ot_size)
		goto err_index;
	rwlock_read(&self->ot_desc->id_lock);
	result = self->ot_desc->id_vtab[index];
	if unlikely(!result)
		goto err_unbound;
	Dee_Incref(result);
	rwlock_endread(&self->ot_desc->id_lock);
	return result;
err_unbound:
	rwlock_endread(&self->ot_desc->id_lock);
	err_unbound_index((DeeObject *)self, index);
	return NULL;
err_index:
	err_index_out_of_bounds((DeeObject *)self, index, self->ot_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ot_nsi_getitem_fast(ObjectTable *__restrict self, size_t index) {
	DREF DeeObject *result;
	ASSERT(index < self->ot_size);
	rwlock_read(&self->ot_desc->id_lock);
	result = self->ot_desc->id_vtab[index];
	Dee_XIncref(result);
	rwlock_endread(&self->ot_desc->id_lock);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ot_nsi_delitem(ObjectTable *__restrict self, size_t index) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->ot_size)
		goto err_index;
	rwlock_write(&self->ot_desc->id_lock);
	oldval = self->ot_desc->id_vtab[index];
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	if unlikely(!oldval)
		goto err_unbound;
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	self->ot_desc->id_vtab[index] = NULL;
	rwlock_endwrite(&self->ot_desc->id_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	Dee_Decref(oldval);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
	Dee_XDecref(oldval);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
	return 0;
#ifdef CONFIG_ERROR_DELETE_UNBOUND
err_unbound:
	rwlock_endwrite(&self->ot_desc->id_lock);
	return err_unbound_index((DeeObject *)self, index);
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
err_index:
	return err_index_out_of_bounds((DeeObject *)self, index, self->ot_size);
}

PRIVATE int DCALL
ot_nsi_setitem(ObjectTable *__restrict self, size_t index,
               DeeObject *__restrict value) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->ot_size)
		goto err_index;
	Dee_Incref(value);
	rwlock_write(&self->ot_desc->id_lock);
	oldval                        = self->ot_desc->id_vtab[index];
	self->ot_desc->id_vtab[index] = value;
	rwlock_endwrite(&self->ot_desc->id_lock);
	Dee_XDecref(oldval);
	return 0;
err_index:
	return err_index_out_of_bounds((DeeObject *)self, index, self->ot_size);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
ot_nsi_xchitem(ObjectTable *__restrict self, size_t index,
               DeeObject *__restrict newval) {
	DREF DeeObject *oldval;
	if unlikely(index >= self->ot_size)
		goto err_index;
	rwlock_write(&self->ot_desc->id_lock);
	oldval = self->ot_desc->id_vtab[index];
	if unlikely(!oldval)
		goto err_unbound;
	Dee_Incref(newval);
	self->ot_desc->id_vtab[index] = newval;
	rwlock_endwrite(&self->ot_desc->id_lock);
	return oldval;
err_unbound:
	rwlock_endwrite(&self->ot_desc->id_lock);
	err_unbound_index((DeeObject *)self, index);
	goto err;
err_index:
	err_index_out_of_bounds((DeeObject *)self, index, self->ot_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ot_getitem(ObjectTable *self,
           DeeObject *index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return ot_nsi_getitem(self, i);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ot_delitem(ObjectTable *__restrict self,
           DeeObject *__restrict index) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return ot_nsi_delitem(self, i);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
ot_setitem(ObjectTable *__restrict self,
           DeeObject *__restrict index,
           DeeObject *__restrict value) {
	size_t i;
	if (DeeObject_AsSize(index, &i))
		goto err;
	return ot_nsi_setitem(self, i, value);
err:
	return -1;
}



PRIVATE struct type_nsi ot_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&ot_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&ot_nsi_getsize,
			/* .nsi_getitem      = */ (void *)&ot_nsi_getitem,
			/* .nsi_delitem      = */ (void *)&ot_nsi_delitem,
			/* .nsi_setitem      = */ (void *)&ot_nsi_setitem,
			/* .nsi_getitem_fast = */ (void *)&ot_nsi_getitem_fast,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)NULL,
			/* .nsi_rfind        = */ (void *)NULL,
			/* .nsi_xch          = */ (void *)&ot_nsi_xchitem,
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

PRIVATE struct type_seq ot_seq = {
	/* .tp_iter_self = */ NULL, /* WARNING: If you assign a dedicated iterator here, `librt' will
	                             * not longer be able to reverse-engineer `DeeGenericIterator_Type', as in
	                             * order to do that, it does `DeeObject_GetAttr(&ObjectTable_Type, "Iterator")' */
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ot_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ot_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&ot_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ot_setitem,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &ot_nsi
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

PRIVATE struct type_getset ot_getsets[] = {
	{ DeeString_STR(&str___type__),
	  (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_gettype, NULL, NULL,
	  DOC("->?DType\nThe type describing @this object table") },
	{ "__class__", (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_getclass, NULL, NULL,
	  DOC("->?Ert:ClassDescriptor\nSame as ${this.__type__.__class__}") },
	{ "__isctable__", (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_isctable, NULL, NULL,
	  DOC("->?Dbool\nEvaluates to :true if @this is a class object table") },
	{ "__isitable__", (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_isitable, NULL, NULL,
	  DOC("->?Dbool\nEvaluates to :true if @this is an instance object table") },
	{ NULL }
};

PRIVATE struct type_member ot_members[] = {
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
		if (DeeObject_AssertType(ob, type))
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
	                         "@throw TypeError The given @ob isn't a class or class instance\n"
	                         "Load the object member table of a class, or class instance"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&ot_init,
				TYPE_FIXED_ALLOCATOR(ObjectTable)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ot_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_str,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&ot_bool
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
	DeeTypeObject *type;
	DeeObject *real_self = self;
	type                 = Dee_TYPE(self);
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
	if (DeeObject_AssertType(thisarg, self->im_type))
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
	if (DeeObject_AssertType(thisarg, self->im_type))
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
	if (DeeObject_AssertType(thisarg, self->im_type))
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


PRIVATE struct type_method instancemember_methods[] = {
	{ DeeString_STR(&str_get),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&instancemember_get,
	  DOC("(thisarg)->\n"
	      "Return the @thisarg's value of @this member"),
	  TYPE_METHOD_FKWDS },
	{ "delete",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&instancemember_delete,
	  DOC("(thisarg)\n"
	      "Delete @thisarg's value of @this member"),
	  TYPE_METHOD_FKWDS },
	{ DeeString_STR(&str_set),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&instancemember_set,
	  DOC("(thisarg,value)\n"
	      "Set @thisarg's value of @this member to @value"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_module(DeeInstanceMemberObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->im_type);
	if (!result)
		return_none;
	return result;
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
	ASSERT(self);
	if ((self->im_attribute->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) ==
	    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) {
		if (!self->im_type->tp_class->cd_members[self->im_attribute->ca_addr + CLASS_GETSET_GET])
			return_false;
	}
	return_true;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemember_get_candel(DeeInstanceMemberObject *__restrict self) {
	ASSERT(self);
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
	ASSERT(self);
	if (self->im_attribute->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		return_false;
	if ((self->im_attribute->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) ==
	    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) {
		if (!self->im_type->tp_class->cd_members[self->im_attribute->ca_addr + CLASS_GETSET_SET])
			return_false;
	}
	return_true;
}

PRIVATE struct type_getset instancemember_getsets[] = {
	{ "canget",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_canget, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this member can be read from") },
	{ "candel",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_candel, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this member can be deleted") },
	{ "canset",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_canset, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this member can be written to") },
	{ DeeString_STR(&str___name__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_name, NULL, NULL,
	  DOC("->?Dstring\n"
	      "The name of @this instance member") },
	{ DeeString_STR(&str___doc__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_doc, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "The documentation string associated with @this instance member") },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_module, NULL, NULL,
	  DOC("->?X2?DModule?N\n"
	      "Returns the module that is defining @this instance "
	      "member, or :none if that module could not be defined") },
	{ NULL }
};

PRIVATE struct type_member instancemember_members[] = {
	TYPE_MEMBER_FIELD("__type__", STRUCT_OBJECT, offsetof(DeeInstanceMemberObject, im_type)),
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
	if (DeeObject_AssertType((DeeObject *)other, &DeeInstanceMember_Type))
		return NULL;
	return_bool_(self->im_type == other->im_type &&
	             self->im_attribute == other->im_attribute);
}

PRIVATE struct type_cmp instancemember_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&instancemember_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&instancemember_eq
};

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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */instancemember_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeInstanceMemberObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&instancemember_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
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
                      struct class_attribute *__restrict attr) {
	DREF DeeInstanceMemberObject *result;
	ASSERT_OBJECT_TYPE(class_type, &DeeType_Type);
	ASSERT(DeeType_IsClass(class_type));
	ASSERT(attr);
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
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
			perm |= ATTR_PROPERTY;
		else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
			perm |= ATTR_PERMCALL;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
			perm |= ATTR_PRIVATE;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			rwlock_read(&my_class->cd_lock);
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
			rwlock_endread(&my_class->cd_lock);
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
		rwlock_read(&my_class->cd_lock);
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
		rwlock_endread(&my_class->cd_lock);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
			perm |= ATTR_PROPERTY;
		else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
			perm |= ATTR_PERMCALL;
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
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
			inst = class_desc_as_instance(my_class);
		else if (instance)
			inst = DeeInstance_DESC(my_class, instance);
		if (inst)
			rwlock_read(&inst->id_lock);
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
			rwlock_endread(&inst->id_lock);
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
			perm |= ATTR_PROPERTY;
		else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
			perm |= ATTR_PERMCALL;
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
	attr = DeeType_QueryClassAttributeStringWithHash(tp_invoker, self,
	                                                 rules->alr_name,
	                                                 rules->alr_hash);
	if (!attr)
		goto not_found;
	attr_type = NULL;
	perm      = (ATTR_CMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
	/* Figure out which instance descriptor the property is connected to. */
	rwlock_read(&my_class->cd_lock);
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
	rwlock_endread(&my_class->cd_lock);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
		perm |= ATTR_PROPERTY;
	else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		perm |= ATTR_PERMCALL;
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
	attr = DeeType_QueryInstanceAttributeStringWithHash(tp_invoker, self,
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
		rwlock_read(&my_class->cd_lock);
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
		rwlock_endread(&my_class->cd_lock);
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
	attr = DeeType_QueryAttributeStringWithHash(tp_invoker, self,
	                                            rules->alr_name,
	                                            rules->alr_hash);
	if (!attr)
		goto not_found;
	attr_type = NULL, inst = NULL;
	perm = (ATTR_IMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
	/* Figure out which instance descriptor the property is connected to. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		inst = class_desc_as_instance(my_class);
	else if (instance)
		inst = DeeInstance_DESC(my_class, instance);
	if (inst)
		rwlock_read(&inst->id_lock);
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
		rwlock_endread(&inst->id_lock);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
		perm |= ATTR_PROPERTY;
	else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		perm |= ATTR_PERMCALL;
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
                              struct class_attribute *__restrict attr) {
	DREF DeePropertyObject *result;
	struct class_desc *my_class;
	/* Return an instance-wrapper for instance-members. */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
		return DeeInstanceMember_New(class_type, attr);
	my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		DREF DeeObject *member_value;
		/* Simple case: direct access to unbound class-based attr. */
		rwlock_read(&my_class->cd_lock);
		member_value = my_class->cd_members[attr->ca_addr];
		Dee_XIncref(member_value);
		rwlock_endread(&my_class->cd_lock);
		if unlikely(!member_value)
			goto unbound;
		return member_value;
	}
	result = DeeObject_MALLOC(DeePropertyObject);
	if unlikely(!result)
		goto err;
	result->p_del = NULL;
	result->p_set = NULL;
	rwlock_read(&my_class->cd_lock);
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
	rwlock_endread(&my_class->cd_lock);
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
	err_unbound_attribute(class_type,
	                      DeeString_STR(attr->ca_name));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeClass_BoundInstanceAttribute(DeeTypeObject *__restrict class_type,
                                struct class_attribute *__restrict attr) {
	int result;
	struct class_desc *my_class;
	/* Return an instance-wrapper for instance-members. */
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
		return 1; /* instance-members outside of class memory are
		           * accessed through wrappers, which are always bound. */
	my_class = DeeClass_DESC(class_type);
	/* Check if the member is assigned. */
	rwlock_read(&my_class->cd_lock);
	if ((attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)) ==
	    CLASS_ATTRIBUTE_FGETSET) {
		result = ((my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] != NULL) ||
		          (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] != NULL) ||
		          (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] != NULL));
	} else {
		result = my_class->cd_members[attr->ca_addr] != NULL;
	}
	rwlock_endread(&my_class->cd_lock);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttribute(DeeTypeObject *class_type,
                               struct class_attribute *__restrict attr,
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
		if (DeeObject_AssertType(argv[0], class_type))
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
	rwlock_read(&my_class->cd_lock);
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
	rwlock_endread(&my_class->cd_lock);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	result = DeeObject_Call(callback, argc, argv);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute(class_type,
	                      DeeString_STR(attr->ca_name));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeKw(DeeTypeObject *class_type,
                                 struct class_attribute *__restrict attr,
                                 size_t argc, DeeObject *const *argv,
                                 DeeObject *kw) {
	DREF DeeObject *callback, *result;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
		DeeObject *thisarg;
		if (DeeArg_UnpackKw(argc, argv, kw, thisarg_kwlist, "o:get", &thisarg) ||
		    DeeObject_AssertType(thisarg, class_type))
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
	rwlock_read(&my_class->cd_lock);
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
	rwlock_endread(&my_class->cd_lock);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	result = DeeObject_CallKw(callback, argc, argv, kw);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute(class_type,
	                      DeeString_STR(attr->ca_name));
err:
	return NULL;
}

#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeTuple(DeeTypeObject *class_type,
                                    struct class_attribute *__restrict attr,
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
		if (DeeObject_AssertType(DeeTuple_GET(args, 0), class_type))
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
		 * as well as that `DeeTuple_GET(args,0) is class_type', but there is no
		 * need for us to do this, as the callback that's going to be
		 * invoked will perform those same check (should that guaranty
		 * become relevant), because it's yet another object over which
		 * the user has full control. */
	}
#endif
	rwlock_read(&my_class->cd_lock);
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
	rwlock_endread(&my_class->cd_lock);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	result = DeeObject_CallTuple(callback, args);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute(class_type,
	                      DeeString_STR(attr->ca_name));
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeTupleKw(DeeTypeObject *class_type,
                                      struct class_attribute *__restrict attr,
                                      DeeObject *args, DeeObject *kw) {
	DREF DeeObject *callback, *result;
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		/* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
		DeeObject *thisarg;
		if (DeeArg_UnpackKw(DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw, thisarg_kwlist, "o:get", &thisarg) ||
		    DeeObject_AssertType(thisarg, class_type))
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
		 * as well as that `DeeTuple_GET(args,0) is class_type', but there is no
		 * need for us to do this, as the callback that's going to be
		 * invoked will perform those same check (should that guaranty
		 * become relevant), because it's yet another object over which
		 * the user has full control. */
	}
#endif
	rwlock_read(&my_class->cd_lock);
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
	rwlock_endread(&my_class->cd_lock);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	result = DeeObject_CallTupleKw(callback, args, kw);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute(class_type,
	                      DeeString_STR(attr->ca_name));
err:
	return NULL;
}
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeClass_VCallInstanceAttributef(DeeTypeObject *class_type,
                                 struct class_attribute *__restrict attr,
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
		if (DeeObject_AssertType(thisarg, class_type))
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
	rwlock_read(&my_class->cd_lock);
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
	rwlock_endread(&my_class->cd_lock);
	if unlikely(!callback)
		goto unbound;
	/* Invoke the callback. */
	result = DeeObject_VCallf(callback, format, args);
	Dee_Decref(callback);
	return result;
unbound:
	err_unbound_attribute(class_type,
	                      DeeString_STR(attr->ca_name));
	goto err;
err_args_tuple:
	Dee_Decref(args_tuple);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeClass_DelInstanceAttribute(DeeTypeObject *__restrict class_type,
                              struct class_attribute *__restrict attr) {
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if unlikely(!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
		goto err_noaccess;
	/* Make sure not to re-write readonly attributes. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
		return err_cant_access_attribute(class_type,
		                                 DeeString_STR(attr->ca_name),
		                                 ATTR_ACCESS_DEL);
	}
	if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
		DREF DeeObject *old_value;
		/* Simple case: directly delete a class-based attr. */
		rwlock_write(&my_class->cd_lock);
		old_value                           = my_class->cd_members[attr->ca_addr];
		my_class->cd_members[attr->ca_addr] = NULL;
		rwlock_endwrite(&my_class->cd_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
		if unlikely(!old_value)
			goto unbound;
		Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
		Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
	} else {
		/* Property callbacks (delete 3 bindings, rather than 1) */
		DREF DeeObject *old_value[3];
		rwlock_write(&my_class->cd_lock);
		old_value[0]                                           = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
		old_value[1]                                           = my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL];
		old_value[2]                                           = my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET];
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] = NULL;
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] = NULL;
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] = NULL;
		rwlock_endwrite(&my_class->cd_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
		/* Only thrown an unbound-error when none of the callbacks were assigned. */
		if unlikely(!old_value[0] &&
			         !old_value[1] &&
			         !old_value[2])
		goto unbound;
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
		Dee_XDecref(old_value[2]);
		Dee_XDecref(old_value[1]);
		Dee_XDecref(old_value[0]);
	}
	return 0;
#ifdef CONFIG_ERROR_DELETE_UNBOUND
unbound:
	return err_unbound_attribute(class_type, DeeString_STR(attr->ca_name));
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
err_noaccess:
	return err_cant_access_attribute(class_type,
	                                 DeeString_STR(attr->ca_name),
	                                 ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeClass_SetInstanceAttribute(DeeTypeObject *class_type,
                              struct class_attribute *__restrict attr,
                              DeeObject *value) {
	struct class_desc *my_class = DeeClass_DESC(class_type);
	if unlikely(!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
		goto err_noaccess;
	/* Make sure not to re-write readonly attributes. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
		return err_cant_access_attribute(class_type,
		                                 DeeString_STR(attr->ca_name),
		                                 ATTR_ACCESS_SET);
	}
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *old_value[3];
		if (DeeObject_AssertType(value, &DeeProperty_Type))
			return -1;
		/* Unpack and assign a property wrapper.
		 * NOTE: Because only properties with the read-only flag can get away
		 *       with only a getter VTABLE slot, we can assume that this property
		 *       has 3 slots because we're not allowed to override readonly properties. */
		Dee_XIncref(((DeePropertyObject *)value)->p_get);
		Dee_XIncref(((DeePropertyObject *)value)->p_del);
		Dee_XIncref(((DeePropertyObject *)value)->p_set);
		rwlock_write(&my_class->cd_lock);
		old_value[0]                                           = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
		old_value[1]                                           = my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL];
		old_value[2]                                           = my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET];
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] = ((DeePropertyObject *)value)->p_get;
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] = ((DeePropertyObject *)value)->p_del;
		my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] = ((DeePropertyObject *)value)->p_set;
		rwlock_endwrite(&my_class->cd_lock);
		/* Drop references from the old callbacks. */
		Dee_XDecref(old_value[2]);
		Dee_XDecref(old_value[1]);
		Dee_XDecref(old_value[0]);
	} else {
		/* Simple case: direct overwrite an unbound class-based attr. */
		DREF DeeObject *old_value;
		Dee_Incref(value);
		rwlock_write(&my_class->cd_lock);
		old_value                           = my_class->cd_members[attr->ca_addr];
		my_class->cd_members[attr->ca_addr] = value;
		rwlock_endwrite(&my_class->cd_lock);
		Dee_XDecref(old_value); /* Decref the old value. */
	}
	return 0;
err_noaccess:
	return err_cant_access_attribute(class_type,
	                                 DeeString_STR(attr->ca_name),
	                                 ATTR_ACCESS_SET);
}




INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_GetAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct class_attribute *__restrict attr) {
	DREF DeeObject *result;
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		rwlock_endread(&self->id_lock);
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
		rwlock_read(&self->id_lock);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		rwlock_endread(&self->id_lock);
		if unlikely(!callback)
			goto unbound;
		result = DeeInstanceMethod_New(callback, this_arg);
		Dee_Decref(callback);
	} else {
		/* Simply return the attribute as-is. */
		rwlock_read(&self->id_lock);
		result = self->id_vtab[attr->ca_addr];
		Dee_XIncref(result);
		rwlock_endread(&self->id_lock);
		if unlikely(!result)
			goto unbound;
	}
	return result;
unbound:
	err_unbound_attribute_c(desc, DeeString_STR(attr->ca_name));
	return NULL;
illegal:
	err_cant_access_attribute_c(desc,
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_GET);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_BoundAttribute(struct class_desc *__restrict desc,
                           struct instance_desc *__restrict self,
                           DeeObject *__restrict this_arg,
                           struct class_attribute *__restrict attr) {
	DREF DeeObject *result;
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		rwlock_endread(&self->id_lock);
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
#ifdef CONFIG_NO_THREADS
		return self->id_vtab[attr->ca_addr] != NULL;
#else /* CONFIG_NO_THREADS */
		return ATOMIC_READ(self->id_vtab[attr->ca_addr]) != NULL;
#endif /* !CONFIG_NO_THREADS */
	}
unbound:
	return 0;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_CallAttribute(struct class_desc *__restrict desc,
                          struct instance_desc *__restrict self,
                          DeeObject *this_arg,
                          struct class_attribute *__restrict attr,
                          size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result, *callback;
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		rwlock_endread(&self->id_lock);
		if unlikely(!getter)
			goto illegal;
		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			return NULL;
		result = DeeObject_Call(callback, argc, argv);
		Dee_Decref(callback);
	} else {
		/* Call the attr as-is. */
		rwlock_read(&self->id_lock);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		rwlock_endread(&self->id_lock);
		if unlikely(!callback)
			goto unbound;
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCall(callback, this_arg, argc, argv)
		         : DeeObject_Call(callback, argc, argv);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_c(desc,
	                        DeeString_STR(attr->ca_name));
	return NULL;
illegal:
	err_cant_access_attribute_c(desc,
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_GET);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
DeeInstance_VCallAttributef(struct class_desc *__restrict desc,
                            struct instance_desc *__restrict self,
                            DeeObject *this_arg,
                            struct class_attribute *__restrict attr,
                            char const *__restrict format, va_list args) {
	DREF DeeObject *result, *callback;
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		rwlock_endread(&self->id_lock);
		if unlikely(!getter)
			goto illegal;
		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			return NULL;
		result = DeeObject_VCallf(callback, format, args);
		Dee_Decref(callback);
	} else {
		/* Call the attr as-is. */
		rwlock_read(&self->id_lock);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		rwlock_endread(&self->id_lock);
		if unlikely(!callback)
			goto unbound;
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_VThisCallf(callback, this_arg, format, args)
		         : DeeObject_VCallf(callback, format, args);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_c(desc,
	                        DeeString_STR(attr->ca_name));
	return NULL;
illegal:
	err_cant_access_attribute_c(desc,
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_GET);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
DeeInstance_CallAttributeKw(struct class_desc *__restrict desc,
                            struct instance_desc *__restrict self,
                            DeeObject *__restrict this_arg,
                            struct class_attribute *__restrict attr,
                            size_t argc, DeeObject *const *argv,
                            DeeObject *kw) {
	DREF DeeObject *result, *callback;
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		rwlock_endread(&self->id_lock);
		if unlikely(!getter)
			goto illegal;
		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			return NULL;
		result = DeeObject_CallKw(callback, argc, argv, kw);
		Dee_Decref(callback);
	} else {
		/* Call the attr as-is. */
		rwlock_read(&self->id_lock);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		rwlock_endread(&self->id_lock);
		if unlikely(!callback)
			goto unbound;
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCallKw(callback, this_arg, argc, argv, kw)
		         : DeeObject_CallKw(callback, argc, argv, kw);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_c(desc,
	                        DeeString_STR(attr->ca_name));
	return NULL;
illegal:
	err_cant_access_attribute_c(desc,
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_GET);
	return NULL;
}

#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
DeeInstance_CallAttributeTuple(struct class_desc *__restrict desc,
                               struct instance_desc *__restrict self,
                               DeeObject *this_arg,
                               struct class_attribute *__restrict attr,
                               DeeObject *args) {
	DREF DeeObject *result, *callback;
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		rwlock_endread(&self->id_lock);
		if unlikely(!getter)
			goto illegal;
		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			return NULL;
		result = DeeObject_CallTuple(callback, args);
		Dee_Decref(callback);
	} else {
		/* Call the attr as-is. */
		rwlock_read(&self->id_lock);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		rwlock_endread(&self->id_lock);
		if unlikely(!callback)
			goto unbound;
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCallTuple(callback, this_arg, args)
		         : DeeObject_CallTuple(callback, args);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_c(desc,
	                        DeeString_STR(attr->ca_name));
	return NULL;
illegal:
	err_cant_access_attribute_c(desc,
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_GET);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL
DeeInstance_CallAttributeTupleKw(struct class_desc *__restrict desc,
                                 struct instance_desc *__restrict self,
                                 DeeObject *this_arg,
                                 struct class_attribute *__restrict attr,
                                 DeeObject *args, DeeObject *kw) {
	DREF DeeObject *result, *callback;
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		rwlock_endread(&self->id_lock);
		if unlikely(!getter)
			goto illegal;
		/* Invoke the getter. */
		callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		           ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		           : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
		/* Invoke the return value of the getter. */
		if unlikely(!callback)
			return NULL;
		result = DeeObject_CallTupleKw(callback, args, kw);
		Dee_Decref(callback);
	} else {
		/* Call the attr as-is. */
		rwlock_read(&self->id_lock);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		rwlock_endread(&self->id_lock);
		if unlikely(!callback)
			goto unbound;
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCallTupleKw(callback, this_arg, args, kw)
		         : DeeObject_CallTupleKw(callback, args, kw);
		Dee_Decref(callback);
	}
	return result;
unbound:
	err_unbound_attribute_c(desc,
	                        DeeString_STR(attr->ca_name));
	return NULL;
illegal:
	err_cant_access_attribute_c(desc,
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_GET);
	return NULL;
}
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */


INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_DelAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct class_attribute *__restrict attr) {
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	/* Make sure that the access is allowed. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		goto illegal;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *delfun, *temp;
		rwlock_read(&self->id_lock);
		delfun = self->id_vtab[attr->ca_addr + CLASS_GETSET_DEL];
		Dee_XIncref(delfun);
		rwlock_endread(&self->id_lock);
		if unlikely(!delfun)
			goto illegal;
		/* Invoke the getter. */
		temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		       ? DeeObject_ThisCall(delfun, this_arg, 0, NULL)
		       : DeeObject_Call(delfun, 0, NULL);
		Dee_Decref(delfun);
		if unlikely(!temp)
			return -1;
		Dee_Decref(temp);
	} else {
		DREF DeeObject *old_value;
		/* Simply unbind the field in the attr table. */
		rwlock_write(&self->id_lock);
		old_value                    = self->id_vtab[attr->ca_addr];
		self->id_vtab[attr->ca_addr] = NULL;
		rwlock_endwrite(&self->id_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
		if unlikely(!old_value)
			goto unbound;
		Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
		Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
	}
	return 0;
#ifdef CONFIG_ERROR_DELETE_UNBOUND
unbound:
	return err_unbound_attribute_c(desc,
	                               DeeString_STR(attr->ca_name));
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
illegal:
	return err_cant_access_attribute_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_DEL);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
DeeInstance_SetAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *this_arg,
                         struct class_attribute *__restrict attr,
                         DeeObject *value) {
	ASSERT(self);
	ASSERT(attr);
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *setter, *temp;
		/* Make sure that the access is allowed. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			goto illegal;
		rwlock_read(&self->id_lock);
		setter = self->id_vtab[attr->ca_addr + CLASS_GETSET_SET];
		Dee_XIncref(setter);
		rwlock_endread(&self->id_lock);
		if unlikely(!setter)
			goto illegal;
		/* Invoke the getter. */
		temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		       ? DeeObject_ThisCall(setter, this_arg, 1, (DeeObject **)&value)
		       : DeeObject_Call(setter, 1, (DeeObject **)&value);
		Dee_Decref(setter);
		if unlikely(!temp)
			return -1;
		Dee_Decref(temp);
	} else {
		DREF DeeObject *old_value;
		/* Simply override the field in the attr table. */
		rwlock_write(&self->id_lock);
		old_value = self->id_vtab[attr->ca_addr];
		if (old_value && (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
			rwlock_endwrite(&self->id_lock);
			goto illegal; /* readonly fields can only be set once. */
		} else {
			Dee_Incref(value);
			self->id_vtab[attr->ca_addr] = value;
		}
		rwlock_endwrite(&self->id_lock);
		/* Drop a reference from the old value. */
		Dee_XDecref(old_value);
	}
	return 0;
illegal:
	return err_cant_access_attribute_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_SET);
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int
(DCALL DeeInstance_SetBasicAttribute_)(struct class_desc *__restrict desc,
                                       struct instance_desc *__restrict self,
                                       struct class_attribute *__restrict attr,
                                       DeeObject *__restrict value) {
	DREF DeeObject *old_value;
	ASSERT(self);
	ASSERT(attr);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance(desc);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
		return 2; /* Not a basic attribute. */
	/* Simply override the field in the attr table. */
	rwlock_write(&self->id_lock);
	old_value = self->id_vtab[attr->ca_addr];
	if (old_value && (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
		rwlock_endwrite(&self->id_lock);
		goto illegal; /* readonly fields can only be set once. */
	} else {
		Dee_Incref(value);
		self->id_vtab[attr->ca_addr] = value;
	}
	rwlock_endwrite(&self->id_lock);
	/* Drop a reference from the old value. */
	Dee_XDecref(old_value);
	return 0;
illegal:
	return err_cant_access_attribute_c(desc,
	                                   DeeString_STR(attr->ca_name),
	                                   ATTR_ACCESS_SET);
}



INTERN WUNUSED NONNULL((1, 2)) bool DCALL
class_attribute_mayaccess_impl(struct class_attribute *__restrict self,
                               DeeTypeObject *__restrict impl_class) {
	struct code_frame *caller_frame;
	ASSERT_OBJECT_TYPE(impl_class, &DeeType_Type);
	ASSERT(DeeType_IsClass(impl_class));
	ASSERT(self);
	ASSERT(self->ca_flag & CLASS_ATTRIBUTE_FPRIVATE);
	/* Only allow access if the calling code-frame originates from
	 * a this-call who's this-argument derives from `class_type'. */
	caller_frame = DeeThread_Self()->t_exec;
	if (!caller_frame ||
	    !(caller_frame->cf_func->fo_code->co_flags & CODE_FTHISCALL))
		return false;
	return DeeType_IsInherited(DeeObject_Class(caller_frame->cf_this),
	                           impl_class);
}


PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryClassAttributeWithHash(DeeClassDescriptorObject *self,
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
		if (DeeString_SIZE(result->ca_name) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(result->ca_name), DeeString_STR(name),
		           DeeString_SIZE(name) * sizeof(char)) != 0)
			continue;
		return result;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryClassAttributeStringWithHash(DeeClassDescriptorObject *__restrict self,
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
DeeClassDescriptor_QueryClassAttributeStringLenWithHash(DeeClassDescriptorObject *__restrict self,
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
		if (DeeString_SIZE(result->ca_name) != attrlen)
			continue;
		if (memcmp(DeeString_STR(result->ca_name), name, attrlen * sizeof(char)) != 0)
			continue;
		return result;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryInstanceAttributeWithHash(DeeClassDescriptorObject *self,
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
		if (DeeString_SIZE(result->ca_name) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(result->ca_name), DeeString_STR(name),
		           DeeString_SIZE(name) * sizeof(char)) != 0)
			continue;
		return result;
	}
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
DeeClassDescriptor_QueryInstanceAttributeStringWithHash(DeeClassDescriptorObject *__restrict self,
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
DeeClassDescriptor_QueryInstanceAttributeStringLenWithHash(DeeClassDescriptorObject *__restrict self,
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
		if (DeeString_SIZE(result->ca_name) != attrlen)
			continue;
		if (memcmp(DeeString_STR(result->ca_name), name, attrlen * sizeof(char)) != 0)
			continue;
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
	char const *name = "??"
	                   "?";
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
	return err_unbound_attribute(class_type, name);
}

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_unbound_member(/*Class*/ DeeTypeObject *__restrict class_type,
                   struct class_desc *__restrict desc,
                   uint16_t addr) {
	/* Check if we can find the proper member so we can pass its name. */
	size_t i;
	char const *name = "??"
	                   "?";
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
	return err_unbound_attribute(class_type, name);
}


/* Instance member access (by addr) */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
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
	ASSERT(addr <= desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);
	/* Lock and extract the member. */
	rwlock_read(&inst->id_lock);
	result = inst->id_vtab[addr];
	Dee_XIncref(result);
	rwlock_endread(&inst->id_lock);
	if (!result)
		err_unbound_member(tp_self, desc, addr);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) bool
(DCALL DeeInstance_BoundMember)(/*Class*/ DeeTypeObject *__restrict tp_self,
                                /*Instance*/ DeeObject *__restrict self,
                                uint16_t addr) {
	struct class_desc *desc;
	struct instance_desc *inst;
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
	ASSERT(DeeType_IsClass(tp_self));
	ASSERT_OBJECT_TYPE_A(self, tp_self);
	desc = DeeClass_DESC(tp_self);
	ASSERT(addr <= desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);
#ifdef CONFIG_NO_THREADS
	return inst->id_vtab[addr] != NULL;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(inst->id_vtab[addr]) != NULL;
#endif /* !CONFIG_NO_THREADS */
}

INTERN WUNUSED NONNULL((1, 2)) int
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
	ASSERT(addr <= desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);
	/* Lock and extract the member. */
	rwlock_write(&inst->id_lock);
	old_value           = inst->id_vtab[addr];
	inst->id_vtab[addr] = NULL;
	rwlock_endwrite(&inst->id_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	if unlikely(!old_value)
		return err_unbound_member(tp_self, desc, addr);
	Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
	Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
	return 0;
}

INTERN NONNULL((1, 2, 4)) void
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
	ASSERT(addr <= desc->cd_desc->cd_imemb_size);
	inst = DeeInstance_DESC(desc, self);
	/* Lock and extract the member. */
	Dee_Incref(value);
	rwlock_write(&inst->id_lock);
	old_value           = inst->id_vtab[addr];
	inst->id_vtab[addr] = value;
	rwlock_endwrite(&inst->id_lock);
	Dee_XDecref(old_value);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeInstance_GetMemberSafe)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  uint16_t addr) {
	if (DeeObject_AssertType((DeeObject *)tp_self, &DeeType_Type))
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

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeInstance_BoundMemberSafe)(DeeTypeObject *__restrict tp_self,
                                    DeeObject *__restrict self,
                                    uint16_t addr) {
	if (DeeObject_AssertType((DeeObject *)tp_self, &DeeType_Type))
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

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeInstance_DelMemberSafe)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  uint16_t addr) {
	if (DeeObject_AssertType((DeeObject *)tp_self, &DeeType_Type))
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

INTERN WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeInstance_SetMemberSafe)(DeeTypeObject *tp_self,
                                  DeeObject *self,
                                  uint16_t addr, DeeObject *value) {
	if (DeeObject_AssertType((DeeObject *)tp_self, &DeeType_Type))
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

/* Class member access (by addr) */
INTERN NONNULL((1, 3)) void
(DCALL DeeClass_SetMember)(DeeTypeObject *self,
                           uint16_t addr, DeeObject *value) {
	struct class_desc *desc;
	DREF DeeObject *old_value;
	ASSERT_OBJECT_TYPE(self, &DeeType_Type);
	ASSERT(DeeType_IsClass(self));
	desc = DeeClass_DESC(self);
	ASSERT(addr <= desc->cd_desc->cd_cmemb_size);
	/* Lock and extract the member. */
	Dee_Incref(value);
	rwlock_write(&desc->cd_lock);
	old_value              = desc->cd_members[addr];
	desc->cd_members[addr] = value;
	rwlock_endwrite(&desc->cd_lock);
	Dee_XDecref(old_value);
}

INTERN WUNUSED NONNULL((1, 3)) int
(DCALL DeeClass_SetMemberSafe)(DeeTypeObject *__restrict self,
                               uint16_t addr, DeeObject *__restrict value) {
	if (DeeObject_AssertType((DeeObject *)self, &DeeType_Type))
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

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeClass_GetMember)(DeeTypeObject *__restrict self, uint16_t addr) {
	struct class_desc *desc;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, &DeeType_Type);
	ASSERT(DeeType_IsClass(self));
	desc = DeeClass_DESC(self);
	ASSERT(addr <= desc->cd_desc->cd_cmemb_size);
	/* Lock and extract the member. */
	rwlock_write(&desc->cd_lock);
	result = desc->cd_members[addr];
	Dee_XIncref(result);
	rwlock_endwrite(&desc->cd_lock);
	if unlikely(!result)
		err_unbound_class_member(self, desc, addr);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeClass_GetMemberSafe)(DeeTypeObject *__restrict self, uint16_t addr) {
	if (DeeObject_AssertType((DeeObject *)self, &DeeType_Type))
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


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CLASS_DESC_C */
