/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_STRUCT_C
#define GUARD_DEX_CTYPES_STRUCT_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S */
#include <deemon/arg.h>             /* DeeArg_Unpack* */
#include <deemon/bool.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bzero(), ... */
#include <deemon/util/atomic.h>     /* atomic_cmpxch_or_write, atomic_read */
#include <deemon/util/lock.h>       /* Dee_ATOMIC_RWLOCK_INIT, Dee_atomic_rwlock_cinit */

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) DREF DeeStructTypeObject *DCALL
struct_type_rehash(DeeStructTypeObject *__restrict self) {
	DREF DeeStructTypeObject *result;
	Dee_hash_t i, j, perturb, new_mask;
	new_mask = (self->st_fmsk << 1) | 1;
	result = (DREF DeeStructTypeObject *)DeeGCObject_Callocc(offsetof(DeeStructTypeObject, st_fvec),
	                                                         new_mask + 1, sizeof(struct struct_field));
	if unlikely(!result)
		goto err;
	result->st_fmsk = new_mask;
	memcpy(&result->st_base, &self->st_base, sizeof(DeeSTypeObject));
	for (i = 0; i <= self->st_fmsk; ++i) {
		if (!self->st_fvec[i].sf_name)
			continue;
		j = perturb = self->st_fvec[i].sf_hash;
		/* Re-insert this item in the hash-vector of the new struct-type. */
		for (;; STRUCT_TYPE_HASHNX(j, perturb)) {
			Dee_hash_t slot = j & new_mask;
			if (result->st_fvec[slot].sf_name)
				continue;
			memcpy(&result->st_fvec[slot],
			       &self->st_fvec[i],
			       sizeof(struct struct_field));
			break;
		}
	}
	return result;
err:
	return NULL;
}

struct struct_type_alloc_foreach_data {
	DREF DeeStructTypeObject *staf_result;
	size_t                    staf_nfields;
	size_t                    staf_alignof;
	size_t                    staf_sizeof;
	unsigned int              staf_flags;
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
struct_type_alloc_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	Dee_hash_t i, perturb, hash;
	struct struct_type_alloc_foreach_data *data;
	data = (struct struct_type_alloc_foreach_data *)arg;
	if (data->staf_nfields >= data->staf_result->st_fmsk) {
		/* Must allocate more fields. */
		DREF DeeStructTypeObject *new_result;
		new_result = struct_type_rehash(data->staf_result);
		if unlikely(!new_result)
			goto err;
		DeeGCObject_Free(data->staf_result);
		data->staf_result = new_result;
	}
	ASSERT(data->staf_nfields < data->staf_result->st_fmsk);

	/* Validate that this is a string/struct_type-pair. */
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	if (DeeObject_AssertType(value, &DeeSType_Type))
		goto err;

	hash = DeeString_Hash(key);
	i = perturb = STRUCT_TYPE_HASHST(data->staf_result, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct struct_field *field;
		size_t align;
		field = STRUCT_TYPE_HASHIT(data->staf_result, i);
		if (field->sf_name)
			continue;
		align = DeeSType_Alignof(value);
		if (!(data->staf_flags & STRUCT_TYPE_FPACKED) && (data->staf_alignof < align))
			data->staf_alignof = align;
		if (data->staf_flags & STRUCT_TYPE_FUNION) {
			field->sf_offset = 0;
			if (data->staf_sizeof < DeeSType_Sizeof(value))
				data->staf_sizeof = DeeSType_Sizeof(value);
		} else {
			if (!(data->staf_flags & STRUCT_TYPE_FPACKED)) {
				data->staf_sizeof += (align - 1);
				data->staf_sizeof &= ~(align - 1);
			}
			field->sf_offset = data->staf_sizeof;
			data->staf_sizeof += DeeSType_Sizeof(value);
		}
		field->sf_type = DeeSType_LValue((DeeSTypeObject *)value);
		if unlikely(!field->sf_type) {
			field->sf_name = NULL;
			goto err;
		}
		field->sf_hash = hash;
		field->sf_name = (DREF DeeStringObject *)key;
		Dee_Incref(key);
		break;
	}
	++data->staf_nfields;
	return 0;
err:
	return -1;
}

/* Construct a new struct-type from `fields', which is a `{(string, StructuredType)...}' */
INTERN WUNUSED NONNULL((2)) DREF DeeStructTypeObject *DCALL
DeeStructType_FromSequence(DeeObject *name,
                           DeeObject *__restrict fields,
                           unsigned int flags) {
	struct struct_type_alloc_foreach_data data;
	Dee_hash_t i;
	data.staf_nfields = 0;
	data.staf_flags   = flags;
	data.staf_sizeof  = 0;
	data.staf_alignof = 1;
	data.staf_result = (DREF DeeStructTypeObject *)DeeGCObject_Callocc(offsetof(DeeStructTypeObject, st_fvec),
	                                                                   2, sizeof(struct struct_field));
	if unlikely(!data.staf_result)
		goto err;
	data.staf_result->st_fmsk = 1;

	/* Enumerate key/value pairs of "fields" */
	if unlikely(DeeObject_ForeachPair(fields, &struct_type_alloc_foreach_cb, &data) < 0)
		goto err_r;

	/* Fill in size & alignment info. */
	data.staf_result->st_base.st_sizeof = data.staf_sizeof;
	data.staf_result->st_base.st_align  = data.staf_alignof;
	data.staf_result->st_base.st_base.tp_init.tp_alloc.tp_instance_size = sizeof(DeeObject) + data.staf_sizeof;

	/* Fill in remaining fields and start tracking the new struct type. */
	Dee_Incref(DeeStructType_AsType(&DeeStruct_Type));
	Dee_atomic_rwlock_cinit(&data.staf_result->st_base.st_cachelock);
	data.staf_result->st_base.st_base.tp_base  = (DREF DeeTypeObject *)&DeeStruct_Type;
	data.staf_result->st_base.st_base.tp_name  = DeeStruct_Type.st_base.st_base.tp_name;
	data.staf_result->st_base.st_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FHEAP | TP_FMOVEANY;

	/* If given, set the name of the new struct-type. */
	if (name) {
		data.staf_result->st_base.st_base.tp_name = DeeString_STR(name);
		data.staf_result->st_base.st_base.tp_flags |= TP_FNAMEOBJECT;
		Dee_Incref(name);
	}
	DeeObject_Init(&data.staf_result->st_base.st_base, &DeeStructType_Type);
	return DeeType_AsStructType(DeeGC_TRACK(DeeTypeObject, DeeStructType_AsType(data.staf_result)));
err_r:
	for (i = 0; i <= data.staf_result->st_fmsk; ++i) {
		if (!data.staf_result->st_fvec[i].sf_name)
			continue;
		Dee_Decref(data.staf_result->st_fvec[i].sf_name);
		Dee_Decref(DeeLValueType_AsType(data.staf_result->st_fvec[i].sf_type));
	}
	DeeGCObject_Free(data.staf_result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeStructTypeObject *DCALL struct_type_new_empty(void) {
	Dee_Incref(DeeStructType_AsType(&DeeStruct_Type));
	return (DREF DeeStructTypeObject *)&DeeStruct_Type;
}

PRIVATE WUNUSED DREF DeeStructTypeObject *DCALL
struct_type_init(size_t argc, DeeObject *const *argv) {
	DeeObject *fields_or_name, *fields = NULL;
	unsigned int flags = STRUCT_TYPE_FNORMAL; /* TODO */
	DeeArg_Unpack1Or2(err, argc, argv, "StructType", &fields_or_name, &fields);
	if (!fields)
		return DeeStructType_FromSequence(NULL, fields_or_name, flags);
	if (DeeObject_AssertTypeExact(fields_or_name, &DeeString_Type))
		goto err;
	return DeeStructType_FromSequence(fields_or_name, fields, flags);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
struct_type_fini(DeeStructTypeObject *__restrict self) {
	Dee_hash_t i;
	for (i = 0; i <= self->st_fmsk; ++i) {
		if (!self->st_fvec[i].sf_name)
			continue;
		Dee_Decref(self->st_fvec[i].sf_name);
		Dee_Decref(DeeLValueType_AsType(self->st_fvec[i].sf_type));
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
struct_type_visit(DeeStructTypeObject *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_hash_t i;
	for (i = 0; i <= self->st_fmsk; ++i) {
		if (!self->st_fvec[i].sf_name)
			continue;
		Dee_Visit(self->st_fvec[i].sf_name);
		Dee_Visit(DeeLValueType_AsType(self->st_fvec[i].sf_type));
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_type_offsetof(DeeStructTypeObject *self,
                     size_t argc, DeeObject *const *argv) {
	Dee_hash_t i, perturb, hash;
	DeeObject *name;
	DeeArg_Unpack1(err, argc, argv, "offsetof", &name);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(self, i);
		if unlikely(!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_EqualsSTR(field->sf_name, name))
			return DeeInt_NewSize(field->sf_offset);
	}
	DeeError_Throwf(&DeeError_AttributeError,
	                "Cannot get unknown attribute `%k.%k'",
	                self, name);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_type_offsetafter(DeeStructTypeObject *self,
                        size_t argc, DeeObject *const *argv) {
	Dee_hash_t i, perturb, hash;
	DeeObject *name;
	DeeArg_Unpack1(err, argc, argv, "offsetafter", &name);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(self, i);
		if unlikely(!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_EqualsSTR(field->sf_name, name)) {
			return DeeInt_NewSize(field->sf_offset +
			                      DeeSType_Sizeof(field->sf_type->lt_orig));
		}
	}
	DeeError_Throwf(&DeeError_AttributeError,
	                "Cannot get unknown attribute `%k.%k'",
	                self, name);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_type_typeof(DeeStructTypeObject *self,
                   size_t argc, DeeObject *const *argv) {
	Dee_hash_t i, perturb, hash;
	DeeObject *name;
	DeeArg_Unpack1(err, argc, argv, "typeof", &name);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(self, i);
		if unlikely(!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_EqualsSTR(field->sf_name, name))
			return_reference(DeeSType_AsObject(field->sf_type->lt_orig));
	}
	DeeError_Throwf(&DeeError_AttributeError,
	                "Cannot get unknown attribute `%k.%k'",
	                self, name);
err:
	return NULL;
}



PRIVATE struct type_method tpconst struct_type_methods[] = {
	TYPE_METHOD_F("offsetof", &struct_type_offsetof, METHOD_FNOREFESCAPE,
	              "(field:?Dstring)->?Dint\n"
	              "#tAttributeError{No field with the name @field exists}"
	              "Returns the offset of a given @field"),
	TYPE_METHOD_F("offsetafter", &struct_type_offsetafter, METHOD_FNOREFESCAPE,
	              "(field:?Dstring)->?Dint\n"
	              "#tAttributeError{No field with the name @field exists}"
	              "Returns the offset after a given @field"),
	/* TODO: containerof(pointer p, string field) -> lvalue
	 *       Where type(p) === this.typeof(field).pointer,
	 *       and type(return) == this.lvalue */
	TYPE_METHOD_F("typeof", &struct_type_typeof, METHOD_FNOREFESCAPE,
	              "(field:?Dstring)->?GStructuredType\n"
	              "#tAttributeError{No field with the name @field exists}"
	              "Returns the typing of given @field"),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst struct_type_members[] = {
	TYPE_MEMBER_CONST_DOC("isstruct", Dee_True, "Returns ?t if @this ?GStructuredType is a ?GStructType"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeStructType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "StructType",
	/* .tp_doc      = */ DOC("(fields:?X2?S?T2?Dstring?GStructuredType?M?Dstring?GStructuredType)\n"
	                         "(name:?Dstring,fields:?X2?S?T2?Dstring?GStructuredType?M?Dstring?GStructuredType)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSType_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &struct_type_new_empty,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ &struct_type_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&struct_type_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&struct_type_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ struct_type_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ struct_type_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE ATTR_COLD NONNULL((1, 3)) int DCALL
ctypes_err_unknown_attr(DeeSTypeObject *tp, void *self,
                        DeeObject *attr, unsigned int access) {
	DREF DeeObject *lv = DeeLValue_NewFor(tp, self);
	if unlikely(!lv)
		goto err;
	DeeRT_ErrUnknownAttr(lv, attr, access);
	Dee_Decref(lv);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 3)) DREF struct lvalue_object *DCALL
struct_getattr(DeeStructTypeObject *tp_self,
               void *self, DeeObject *name) {
	DREF struct lvalue_object *result;
	Dee_hash_t i, perturb, hash;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(tp_self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(tp_self, i);
		if (!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (!DeeString_EqualsSTR(field->sf_name, name))
			continue;

		/* Found it! (return an l-value to the field in question) */
		result = DeeObject_MALLOC(struct lvalue_object);
		if unlikely(!result)
			goto err;
		DeeObject_Init(result, DeeLValueType_AsType(field->sf_type));
		result->l_ptr.ptr = (byte_t *)self + field->sf_offset;
		return result;
	}
	ctypes_err_unknown_attr(DeeStructType_AsSType(tp_self), self,
	                        name, DeeRT_ATTRIBUTE_ACCESS_GET);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
struct_delattr(DeeStructTypeObject *tp_self,
               void *self, DeeObject *name) {
	Dee_hash_t i, perturb, hash;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(tp_self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct struct_field *field;
		byte_t *addr;
		size_t size;
		field = STRUCT_TYPE_HASHIT(tp_self, i);
		if (!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (!DeeString_EqualsSTR(field->sf_name, name))
			continue;

		/* Found it! (clear out the memory of this object) */
		addr = (byte_t *)self + field->sf_offset;
		size = DeeSType_Sizeof(field->sf_type->lt_orig);
		CTYPES_FAULTPROTECT(bzero(addr, size), return -1);
		return 0;
	}
	return ctypes_err_unknown_attr(DeeStructType_AsSType(tp_self), self,
	                               name, DeeRT_ATTRIBUTE_ACCESS_DEL);
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
struct_setattr(DeeStructTypeObject *tp_self,
               void *self, DeeObject *name,
               DeeObject *value) {
	Dee_hash_t i, perturb, hash;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(tp_self, hash);
	for (;; STRUCT_TYPE_HASHNX(i, perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(tp_self, i);
		if (!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (!DeeString_EqualsSTR(field->sf_name, name))
			continue;
		/* Found it! (Assign the value to this field) */
		return DeeStruct_Assign(field->sf_type->lt_orig,
		                        (byte_t *)self + field->sf_offset,
		                        value);
	}
	return ctypes_err_unknown_attr(DeeStructType_AsSType(tp_self), self,
	                               name, DeeRT_ATTRIBUTE_ACCESS_SET);
}

struct ctypes_struct_attriter {
	Dee_ATTRITER_HEAD
	DeeStructTypeObject *casi_struct; /* [1..1][const] The struct being enumerated */
	Dee_hash_t           csai_hidx;   /* [lock(ATOMIC)] Next hash-table index to yield. */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ctypes_struct_attriter_next(struct ctypes_struct_attriter *__restrict self,
                            /*out*/ struct Dee_attrdesc *__restrict desc) {
	DeeStructTypeObject *strct = self->casi_struct;
	struct struct_field *field;
	Dee_hash_t old_hidx;
	Dee_hash_t new_hidx;
	do {
		old_hidx = atomic_read(&self->csai_hidx);
		new_hidx = old_hidx;
		for (;;) {
			if (new_hidx > strct->st_fmsk)
				return 1;
			field = &strct->st_fvec[new_hidx];
			++new_hidx;
			if (field->sf_name)
				break;
		}
	} while (!atomic_cmpxch_or_write(&self->csai_hidx, old_hidx, new_hidx));

	/* Fill in attribute descriptor based on "field" */
	Dee_Incref(field->sf_name);
	desc->ad_name = DeeString_STR(field->sf_name);
	desc->ad_doc  = NULL;
	desc->ad_perm = Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET |
	                Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_NAMEOBJ;
	desc->ad_info.ai_decl = DeeStructType_AsObject(strct);
	desc->ad_info.ai_type = Dee_ATTRINFO_CUSTOM;
	desc->ad_info.ai_value.v_custom = DeeStructured_Type.st_base.tp_attr;
	desc->ad_type = DeeLValueType_AsType(field->sf_type);
	return 0;
}

PRIVATE struct Dee_attriter_type tpconst ctypes_struct_attriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&ctypes_struct_attriter_next,
};

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
struct_iterattr(DeeStructTypeObject *__restrict self,
                struct Dee_attriter *iterbuf, size_t bufsize,
                struct Dee_attrhint const *__restrict hint) {
	struct ctypes_struct_attriter *iter = (struct ctypes_struct_attriter *)iterbuf;
	if (bufsize >= sizeof(struct ctypes_struct_attriter)) {
		iter->casi_struct = self;
		iter->csai_hidx   = 0;
		Dee_attriter_init(iter, &ctypes_struct_attriter_type);
	}
	(void)hint;
	return sizeof(struct ctypes_struct_attriter);
}


PRIVATE struct stype_attr struct_attr = {
	/* .st_getattr  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&struct_getattr,
	/* .st_delattr  = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&struct_delattr,
	/* .st_setattr  = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *, DeeObject *))&struct_setattr,
	/* .st_iterattr = */ (size_t (DCALL *)(DeeSTypeObject *__restrict, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&struct_iterattr,
};


struct struct_assign_foreach_data {
	DeeStructTypeObject *safd_tp_self; /* [1..1] Struct type of `safd_self' */
	void                *safd_self;    /* [?..?] Instance pointer. */
};

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
struct_assign_foreach_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct struct_assign_foreach_data *data;
	data = (struct struct_assign_foreach_data *)arg;
	return struct_setattr(data->safd_tp_self, data->safd_self, key, value);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
struct_assign(DeeStructTypeObject *tp_self,
              void *self, DeeObject *value) {
	struct struct_assign_foreach_data data;
	if (DeeObject_InstanceOfExact(value, DeeStructType_AsType(tp_self))) {
		/* Copy-assign. */
		byte_t *dst = (byte_t *)self;
		byte_t *src = (byte_t *)DeeStruct_Data(value);
		size_t size = DeeSType_Sizeof(tp_self);
		CTYPES_FAULTPROTECT(memcpy(dst, src, size), return -1);
		return 0;
	}
	if (DeeNone_Check(value)) {
		/* Clear memory. */
		byte_t *dst = (byte_t *)self;
		size_t size = DeeSType_Sizeof(tp_self);
		CTYPES_FAULTPROTECT(bzero(dst, size), return -1);
		return 0;
	}

	/* Fallback: assign a sequence:
	 * >> struct_type point = {
	 * >>     ("x", int),
	 * >>     ("y", int),
	 * >> };
	 * >> point p = {
	 * >>     .x = 10,
	 * >>     .y = 20,
	 * >> };
	 * >> print repr p;
	 */
	data.safd_tp_self = tp_self;
	data.safd_self    = self;
	return (int)DeeObject_ForeachPair(value, &struct_assign_foreach_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
struct_init(DeeStructTypeObject *tp_self, void *self,
            size_t argc, DeeObject *const *argv) {
	DeeObject *value = Dee_None;
	DeeArg_Unpack0Or1(err, argc, argv, "struct", &value);

	/* Do an initial assignment using the initializer. */
	CTYPES_FAULTPROTECT(bzero(self, tp_self->st_base.st_sizeof), goto err);
	return struct_assign(tp_self, self, value);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
struct_repr(DeeStructTypeObject *tp_self, void *self) {
	Dee_hash_t i;
	bool is_first                = true;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if (ascii_printer_printf(&printer, "%s { ", tp_self->st_base.st_base.tp_name) < 0)
		goto err;
	/* TODO: Print fields in order of offset ascending */
	for (i = 0; i <= tp_self->st_fmsk; ++i) {
		byte_t *field_addr;
		DeeSTypeObject *field_type;
		DeeStringObject *field_name;
		field_name = tp_self->st_fvec[i].sf_name;
		if (!field_name)
			continue;
		if (!is_first && ASCII_PRINTER_PRINT(&printer, ", ") < 0)
			goto err;
		is_first = false;
		field_addr = (byte_t *)self + tp_self->st_fvec[i].sf_offset;
		field_type = tp_self->st_fvec[i].sf_type->lt_orig;
		if (ascii_printer_printf(&printer, ".%k = %k(%K)",
		                         field_name, field_type,
		                         DeeStruct_Repr(field_type, field_addr)) < 0)
			goto err;
	}
	if ((is_first ? ascii_printer_putc(&printer, '}')
	              : ASCII_PRINTER_PRINT(&printer, " }")) < 0)
		goto err;
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}


INTERN struct empty_struct_type_object DeeStruct_Type = {
	/* .st_base = */ {
		/* .st_base = */ {
			OBJECT_HEAD_INIT(&DeeStructType_Type),
			/* .tp_name     = */ "Struct",
			/* .tp_doc      = */ NULL,
			/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ DeeSType_AsType(&DeeStructured_Type),
			/* .tp_init = */ {
				Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
					/* T:              */ DeeObject,
					/* tp_ctor:        */ NULL,
					/* tp_copy_ctor:   */ NULL,
					/* tp_deep_ctor:   */ NULL,
					/* tp_any_ctor:    */ NULL,
					/* tp_any_ctor_kw: */ NULL,
					/* tp_serialize:   */ NULL /* TODO */
				),
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
			/* .tp_seq           = */ NULL,
			/* .tp_iter_next     = */ NULL,
			/* .tp_iterator      = */ NULL,
			/* .tp_attr          = */ NULL,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ NULL,
			/* .tp_methods       = */ NULL,
			/* .tp_getsets       = */ NULL,
			/* .tp_members       = */ NULL,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL
		},
#ifndef CONFIG_NO_THREADS
		/* .st_cachelock = */ Dee_ATOMIC_RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
		/* .st_pointer  = */ &DeePointer_Type,
		/* .st_lvalue   = */ &DeeLValue_Type,
		/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
		/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
		/* .st_ffitype  = */ &ffi_type_void,
#endif /* !CONFIG_NO_CFUNCTION */
		/* .st_sizeof   = */ 0,
		/* .st_align    = */ CONFIG_CTYPES_ALIGNOF_POINTER,
		/* .st_init     = */ (int (DCALL *)(DeeSTypeObject *, void *, size_t, DeeObject *const *))&struct_init,
		/* .st_assign   = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&struct_assign,
		/* .st_cast     = */ {
			/* .st_str  = */ NULL,
			/* .st_repr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&struct_repr,
			/* .st_bool = */ NULL
		},
		/* .st_call     = */ NULL,
		/* .st_math     = */ NULL,
		/* .st_cmp      = */ NULL,
		/* .st_seq      = */ NULL,
		/* .st_attr     = */ &struct_attr
	},
	/* .st_fmsk = */ 0,
	/* .st_fvec = */ { { NULL, 0, 0, NULL } }
};

DECL_END

#endif /* !GUARD_DEX_CTYPES_STRUCT_C */
