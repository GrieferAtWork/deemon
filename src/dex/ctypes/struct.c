/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_STRUCT_C
#define GUARD_DEX_CTYPES_STRUCT_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>

DECL_BEGIN

PRIVATE DREF DeeStructTypeObject *DCALL
struct_type_rehash(DeeStructTypeObject *__restrict self) {
	DREF DeeStructTypeObject *result;
	size_t i, j, perturb, new_mask;
	new_mask = (self->st_fmsk << 1) | 1;
	result = (DREF DeeStructTypeObject *)DeeGCObject_Malloc(offsetof(DeeStructTypeObject, st_fvec) +
	                                                        (new_mask + 1) * sizeof(struct struct_field));
	if
		unlikely(!result)
	goto err;
	result->st_fmsk = new_mask;
	memcpy(&result->st_base, &self->st_base, sizeof(DeeSTypeObject));
	for (i = 0; i <= self->st_fmsk; ++i) {
		if (!self->st_fvec[i].sf_name)
			continue;
		j = perturb = self->st_fvec[i].sf_hash;
		/* Re-insert this item in the hash-vector of the new struct-type. */
		for (;; j = STRUCT_TYPE_HASHNX(j, perturb), STRUCT_TYPE_HASHPT(perturb)) {
			if (result->st_fvec[j].sf_name)
				continue;
			memcpy(&result->st_fvec[j],
			       &self->st_fvec[i],
			       sizeof(struct struct_field));
			break;
		}
	}
err:
	return NULL;
}

PRIVATE DREF DeeStructTypeObject *DCALL
struct_type_alloc_iterator(DeeObject *__restrict iter,
                           unsigned int flags) {
	DREF DeeStructTypeObject *result;
	size_t field_count = 0;
	DREF DeeObject *elem;
	DREF DeeObject *field_name_and_type[2];
	size_t i, min_align = 1, instance_size = 0;
	result = (DREF DeeStructTypeObject *)DeeGCObject_Calloc(offsetof(DeeStructTypeObject, st_fvec) +
	                                                        (2 * sizeof(struct struct_field)));
	if
		unlikely(!result)
	goto err;
	result->st_fmsk = 1;
	while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
		int temp;
		dhash_t perturb, hash;
		if (field_count >= result->st_fmsk) {
			/* Must allocate more fields. */
			DREF DeeStructTypeObject *new_result;
			new_result = struct_type_rehash(result);
			if
				unlikely(!new_result)
			goto err_r;
			DeeGCObject_Free(result);
			result = new_result;
		}
		ASSERT(field_count < result->st_fmsk);
		temp = DeeObject_Unpack(elem, 2, field_name_and_type);
		Dee_Decref(elem);
		if
			unlikely(temp)
		goto err_r;
		/* Validate that this is a string/struct_type-pair. */
		if (DeeObject_AssertTypeExact(field_name_and_type[0], &DeeString_Type) ||
		    DeeObject_AssertType(field_name_and_type[1], &DeeSType_Type)) {
			Dee_Decref(field_name_and_type[1]);
			Dee_Decref(field_name_and_type[0]);
			goto err_r;
		}
		hash = DeeString_Hash(field_name_and_type[0]);
		i = perturb = STRUCT_TYPE_HASHST(result, hash);
		for (;; i = STRUCT_TYPE_HASHNX(i, perturb), STRUCT_TYPE_HASHPT(perturb)) {
			struct struct_field *field;
			size_t align;
			field = STRUCT_TYPE_HASHIT(result, i);
			if (field->sf_name)
				continue;
			align = DeeSType_Alignof(field_name_and_type[1]);
			if (!(flags & STRUCT_TYPE_FPACKED) && (min_align < align))
				min_align = align;
			if (flags & STRUCT_TYPE_FUNION) {
				field->sf_offset = 0;
				if (instance_size < DeeSType_Sizeof(field_name_and_type[1]))
					instance_size = DeeSType_Sizeof(field_name_and_type[1]);
			} else {
				if (!(flags & STRUCT_TYPE_FPACKED)) {
					instance_size += (align - 1);
					instance_size &= ~(align - 1);
				}
				field->sf_offset = instance_size;
				instance_size += DeeSType_Sizeof(field_name_and_type[1]);
			}
			field->sf_hash = DeeString_Hash(field_name_and_type[0]);
			field->sf_name = (DREF struct string_object *)field_name_and_type[0];       /* Inherit reference. */
			field->sf_type = DeeSType_LValue((DeeSTypeObject *)field_name_and_type[1]); /* Inherit reference. */
			Dee_Decref(field_name_and_type[1]);
			if
				unlikely(!field->sf_type)
			{
				Dee_Decref(field_name_and_type[0]);
				field->sf_name = NULL;
				goto err_r;
			}
			break;
		}
		++field_count;
		if (DeeThread_CheckInterrupt())
			goto err_r;
	}
	if
		unlikely(!elem)
	goto err_r;
	/* Fill in size & alignment info. */
	result->st_base.st_align                                  = min_align;
	result->st_base.st_base.tp_init.tp_alloc.tp_instance_size = (sizeof(DeeObject) + instance_size);
	return result;
err_r:
	for (i = 0; i <= result->st_fmsk; ++i) {
		if (!result->st_fvec[i].sf_name)
			continue;
		Dee_Decref(result->st_fvec[i].sf_name);
		Dee_Decref((DeeObject *)result->st_fvec[i].sf_type);
	}
	DeeGCObject_Free(result);
err:
	return NULL;
}


INTERN DREF DeeStructTypeObject *DCALL
DeeStructType_FromSequence(DeeObject *name,
                           DeeObject *__restrict fields,
                           unsigned int flags) {
	DREF DeeStructTypeObject *result;
	size_t i, field_count;
	field_count = DeeFastSeq_GetSize(fields);
	if (field_count != DEE_FASTSEQ_NOTFAST) {
		/* Optimization for fast sequence types. */
		size_t result_mask = 1;
		size_t min_align = 1, instance_size = 0;
		while (result_mask <= field_count)
			result_mask = (result_mask << 1) | 1;
		result = (DREF DeeStructTypeObject *)DeeGCObject_Calloc(offsetof(DeeStructTypeObject, st_fvec) +
		                                                        (result_mask + 1) * sizeof(struct struct_field));
		if
			unlikely(!result)
		goto err;
		result->st_fmsk = result_mask;
		for (i = 0; i < field_count; ++i) {
			DREF DeeObject *elem;
			DREF DeeObject *field_name_and_type[2];
			int temp;
			dhash_t j, perturb, hash;
			elem = DeeFastSeq_GetItem(fields, i);
			if
				unlikely(!elem)
			goto err_r;
			temp = DeeObject_Unpack(elem, 2, field_name_and_type);
			Dee_Decref(elem);
			if
				unlikely(temp)
			goto err_r;
			/* Validate that this is a string/struct_type-pair. */
			if (DeeObject_AssertTypeExact(field_name_and_type[0], &DeeString_Type) ||
			    DeeObject_AssertType(field_name_and_type[1], &DeeSType_Type)) {
				Dee_Decref(field_name_and_type[1]);
				Dee_Decref(field_name_and_type[0]);
				goto err_r;
			}
			hash = DeeString_Hash(field_name_and_type[0]);
			j = perturb = STRUCT_TYPE_HASHST(result, hash);
			for (;; j = STRUCT_TYPE_HASHNX(j, perturb), STRUCT_TYPE_HASHPT(perturb)) {
				struct struct_field *field;
				size_t align;
				field = STRUCT_TYPE_HASHIT(result, j);
				if (field->sf_name)
					continue;
				align = DeeSType_Alignof(field_name_and_type[1]);
				if (!(flags & STRUCT_TYPE_FPACKED) && (min_align < align))
					min_align = align;
				if (flags & STRUCT_TYPE_FUNION) {
					field->sf_offset = 0;
					if (instance_size < DeeSType_Sizeof(field_name_and_type[1]))
						instance_size = DeeSType_Sizeof(field_name_and_type[1]);
				} else {
					if (!(flags & STRUCT_TYPE_FPACKED)) {
						instance_size += (align - 1);
						instance_size &= ~(align - 1);
					}
					field->sf_offset = instance_size;
					instance_size += DeeSType_Sizeof(field_name_and_type[1]);
				}
				field->sf_hash = DeeString_Hash(field_name_and_type[0]);
				field->sf_name = (DREF struct string_object *)field_name_and_type[0];       /* Inherit reference. */
				field->sf_type = DeeSType_LValue((DeeSTypeObject *)field_name_and_type[1]); /* Inherit reference. */
				Dee_Decref(field_name_and_type[1]);
				if
					unlikely(!field->sf_type)
				{
					Dee_Decref(field_name_and_type[0]);
					field->sf_name = NULL;
					goto err_r;
				}
				break;
			}
		}
		/* Fill in size & alignment info. */
		result->st_base.st_align                                  = min_align;
		result->st_base.st_base.tp_init.tp_alloc.tp_instance_size = (sizeof(DeeObject) + instance_size);
	} else {
		/* Use iterators to construct the struct-type. */
		fields = DeeObject_IterSelf(fields);
		if
			unlikely(!fields)
		return NULL;
		result = struct_type_alloc_iterator(fields, flags);
		Dee_Decref(fields);
		if
			unlikely(!result)
		goto err;
	}
	/* Fill in remaining fields and start tracking the new struct type. */
	Dee_Incref((DeeObject *)&DeeStruct_Type);
#ifndef CONFIG_NO_THREADS
	rwlock_cinit(&result->st_base.st_cachelock);
#endif /* !CONFIG_NO_THREADS */
	result->st_base.st_base.tp_base  = (DREF DeeTypeObject *)&DeeStruct_Type;
	result->st_base.st_base.tp_name  = DeeStruct_Type.st_base.st_base.tp_name;
	result->st_base.st_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FHEAP | TP_FMOVEANY;
	if (name) {
		/* Set the name of the new struct-type. */
		result->st_base.st_base.tp_name = DeeString_STR(name);
		result->st_base.st_base.tp_flags |= TP_FNAMEOBJECT;
		Dee_Incref(name);
	}
	DeeObject_Init((DeeObject *)result, &DeeStructType_Type);
	DeeGC_Track((DeeObject *)result);
	return result;
err_r:
	for (i = 0; i <= result->st_fmsk; ++i) {
		if (!result->st_fvec[i].sf_name)
			continue;
		Dee_Decref(result->st_fvec[i].sf_name);
		Dee_Decref((DeeObject *)result->st_fvec[i].sf_type);
	}
	DeeGCObject_Free(result);
err:
	return NULL;
}

PRIVATE DREF DeeStructTypeObject *DCALL struct_type_new_empty(void) {
	Dee_Incref((DeeObject *)&DeeStruct_Type);
	return &DeeStruct_Type;
}

PRIVATE DREF DeeStructTypeObject *DCALL
struct_type_init(size_t argc, DeeObject **__restrict argv) {
	DeeObject *fields_or_name, *fields = NULL;
	unsigned int flags = STRUCT_TYPE_FNORMAL; /* TODO */
	if (DeeArg_Unpack(argc, argv, "o|o:struct_type", &fields_or_name, &fields))
		return NULL;
	return fields
	       ? (DeeObject_AssertTypeExact(fields_or_name, &DeeString_Type)
	          ? NULL
	          : DeeStructType_FromSequence(fields_or_name, fields, flags))
	       : DeeStructType_FromSequence(NULL, fields_or_name, flags);
}

PRIVATE void DCALL
struct_type_fini(DeeStructTypeObject *__restrict self) {
	size_t i;
	for (i = 0; i <= self->st_fmsk; ++i) {
		if (!self->st_fvec[i].sf_name)
			continue;
		Dee_Decref(self->st_fvec[i].sf_name);
		Dee_Decref((DeeObject *)self->st_fvec[i].sf_type);
	}
}

PRIVATE void DCALL
struct_type_visit(DeeStructTypeObject *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->st_fmsk; ++i) {
		if (!self->st_fvec[i].sf_name)
			continue;
		Dee_Visit(self->st_fvec[i].sf_name);
		Dee_Visit((DeeObject *)self->st_fvec[i].sf_type);
	}
}

PRIVATE DREF DeeObject *DCALL
struct_type_offsetof(DeeStructTypeObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
	dhash_t i, perturb, hash;
	DeeObject *name;
	if (DeeArg_Unpack(argc, argv, "o:offsetof", &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; i = STRUCT_TYPE_HASHNX(i, perturb), STRUCT_TYPE_HASHPT(perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(self, i);
		if
			unlikely(!field->sf_name)
		break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_SIZE(field->sf_name) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(field->sf_name),
		           DeeString_STR(name),
		           DeeString_SIZE(name) *
		           sizeof(char)) != 0)
			continue;
		return DeeInt_NewSize(field->sf_offset);
	}
	DeeError_Throwf(&DeeError_AttributeError,
	                "Cannot get unknown attribute `%k.%k'",
	                self, name);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
struct_type_offsetafter(DeeStructTypeObject *__restrict self,
                        size_t argc, DeeObject **__restrict argv) {
	dhash_t i, perturb, hash;
	DeeObject *name;
	if (DeeArg_Unpack(argc, argv, "o:offsetafter", &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; i = STRUCT_TYPE_HASHNX(i, perturb), STRUCT_TYPE_HASHPT(perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(self, i);
		if
			unlikely(!field->sf_name)
		break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_SIZE(field->sf_name) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(field->sf_name),
		           DeeString_STR(name),
		           DeeString_SIZE(name) *
		           sizeof(char)) != 0)
			continue;
		return DeeInt_NewSize(field->sf_offset +
		                      DeeSType_Sizeof(field->sf_type->lt_orig));
	}
	DeeError_Throwf(&DeeError_AttributeError,
	                "Cannot get unknown attribute `%k.%k'",
	                self, name);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
struct_type_typeof(DeeStructTypeObject *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	dhash_t i, perturb, hash;
	DeeObject *name;
	if (DeeArg_Unpack(argc, argv, "o:typeof", &name) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		return NULL;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(self, hash);
	for (;; i = STRUCT_TYPE_HASHNX(i, perturb), STRUCT_TYPE_HASHPT(perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(self, i);
		if
			unlikely(!field->sf_name)
		break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_SIZE(field->sf_name) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(field->sf_name),
		           DeeString_STR(name),
		           DeeString_SIZE(name) *
		           sizeof(char)) != 0)
			continue;
		return_reference((DeeObject *)field->sf_type->lt_orig);
	}
	DeeError_Throwf(&DeeError_AttributeError,
	                "Cannot get unknown attribute `%k.%k'",
	                self, name);
	return NULL;
}



PRIVATE struct type_method struct_type_methods[] = {
	{ "offsetof", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & struct_type_offsetof,
	  DOC("(field:?Dstring)->?Dint\n"
	      "@throw AttributeError No field with the name @field exists\n"
	      "Returns the offset of a given @field") },
	{ "offsetafter", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & struct_type_offsetafter,
	  DOC("(field:?Dstring)->?Dint\n"
	      "@throw AttributeError No field with the name @field exists\n"
	      "Returns the offset after a given @field") },
	/* TODO: containerof(pointer p, string field) -> lvalue
	 *       Where type(p) === this.typeof(field).pointer,
	 *       and type(return) == this.lvalue */
	{ "typeof", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & struct_type_typeof,
	  DOC("(field:?Dstring)->structured_type\n"
	      "@throw AttributeError No field with the name @field exists\n"
	      "Returns the typing of given @field") },
	{ NULL }
};

PRIVATE struct type_member struct_type_members[] = {
	TYPE_MEMBER_CONST_DOC("isstruct", Dee_True, "Returns :true if @this :structured_type is a :struct_type"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeStructType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "StructType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL|TP_FGC|TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSType_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (void *)&struct_type_new_empty,
				/* .tp_copy_ctor = */ (void *)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (void *)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (void *)&struct_type_init,
				/* .tp_free      = */ NULL
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&struct_type_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&struct_type_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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



PRIVATE ATTR_COLD int DCALL
err_unknown_attribute(DeeTypeObject *__restrict tp,
                      DeeObject *__restrict name,
                      char const *__restrict reason) {
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Cannot %s unknown attribute `%k.%k'",
	                       reason, tp, name);
}


PRIVATE DREF struct lvalue_object *DCALL
struct_getattr(DeeStructTypeObject *__restrict tp_self,
               void *self, DeeObject *__restrict name) {
	DREF struct lvalue_object *result;
	dhash_t i, perturb, hash;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(tp_self, hash);
	for (;; i = STRUCT_TYPE_HASHNX(i, perturb), STRUCT_TYPE_HASHPT(perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(tp_self, i);
		if (!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_SIZE(field->sf_name) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(field->sf_name),
		           DeeString_STR(name),
		           DeeString_SIZE(name) * sizeof(char)) != 0)
			continue;
		/* Found it! (return an l-value to the field in question) */
		result = DeeObject_MALLOC(struct lvalue_object);
		if
			unlikely(!result)
		goto err;
		DeeObject_Init(result, (DeeTypeObject *)field->sf_type);
		result->l_ptr.uint = (uintptr_t)self + field->sf_offset;
		return result;
	}
	err_unknown_attribute((DeeTypeObject *)tp_self, name, "get");
err:
	return NULL;
}

PRIVATE int DCALL
struct_delattr(DeeStructTypeObject *__restrict tp_self,
               void *self, DeeObject *__restrict name) {
	dhash_t i, perturb, hash;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(tp_self, hash);
	for (;; i = STRUCT_TYPE_HASHNX(i, perturb), STRUCT_TYPE_HASHPT(perturb)) {
		struct struct_field *field;
		uint8_t *dst;
		size_t size;
		field = STRUCT_TYPE_HASHIT(tp_self, i);
		if (!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_SIZE(field->sf_name) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(field->sf_name),
		           DeeString_STR(name),
		           DeeString_SIZE(name) * sizeof(char)) != 0)
			continue;
		/* Found it! (clear out the memory of this object) */
		dst  = (uint8_t *)((uintptr_t)self + field->sf_offset);
		size = DeeSType_Sizeof(field->sf_type->lt_orig);
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
		CTYPES_FAULTPROTECT(memset(dst, 0, size), return -1);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
		CTYPES_FAULTPROTECT({
			while (size--)
				*dst++ = 0;
		},
		                    return -1);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
		memset(dst, 0, size);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
		return 0;
	}
	return err_unknown_attribute((DeeTypeObject *)tp_self, name, "delete");
}

PRIVATE int DCALL
struct_setattr(DeeStructTypeObject *__restrict tp_self,
               void *self, DeeObject *__restrict name,
               DeeObject *__restrict value) {
	dhash_t i, perturb, hash;
	hash = DeeString_Hash(name);
	i = perturb = STRUCT_TYPE_HASHST(tp_self, hash);
	for (;; i = STRUCT_TYPE_HASHNX(i, perturb), STRUCT_TYPE_HASHPT(perturb)) {
		struct struct_field *field;
		field = STRUCT_TYPE_HASHIT(tp_self, i);
		if (!field->sf_name)
			break;
		if (field->sf_hash != hash)
			continue;
		if (DeeString_SIZE(field->sf_name) != DeeString_SIZE(name))
			continue;
		if (memcmp(DeeString_STR(field->sf_name),
		           DeeString_STR(name),
		           DeeString_SIZE(name) * sizeof(char)) != 0)
			continue;
		/* Found it! (Assign the value to this field) */
		return DeeStruct_Assign(field->sf_type->lt_orig,
		                        (void *)((uintptr_t)self + field->sf_offset),
		                        value);
	}
	return err_unknown_attribute((DeeTypeObject *)tp_self, name, "set");
}

PRIVATE dssize_t DCALL
struct_enumattr(DeeStructTypeObject *__restrict self, denum_t proc, void *arg) {
	size_t i;
	dssize_t temp, result = 0;
	for (i = 0; i < self->st_fmsk; ++i) {
		if (!self->st_fvec[i].sf_name)
			continue;
		temp = (*proc)((DeeObject *)self,
		               DeeString_STR(self->st_fvec[i].sf_name),
		               NULL,
		               ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET,
		               (DeeTypeObject *)self->st_fvec[i].sf_type,
		               arg);
		if
			unlikely(temp < 0)
		goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}


PRIVATE struct stype_attr struct_attr = {
	/* .st_getattr  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&struct_getattr,
	/* .st_delattr  = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&struct_delattr,
	/* .st_setattr  = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict, DeeObject *__restrict))&struct_setattr,
	/* .st_enumattr = */ (dssize_t(DCALL *)(DeeSTypeObject *__restrict, denum_t, void *))&struct_enumattr
};


PRIVATE int DCALL
struct_setpair(DeeStructTypeObject *__restrict tp_self,
               void *self, DeeObject *__restrict pair) {
	DREF DeeObject *key_and_value[2];
	int result;
	if
		unlikely(DeeObject_Unpack(pair, 2, key_and_value))
	goto err;
	result = struct_setattr(tp_self, self,
	                        key_and_value[0],
	                        key_and_value[1]);
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	return result;
err:
	return -1;
}

PRIVATE int DCALL
struct_assign(DeeStructTypeObject *__restrict tp_self,
              void *self, DeeObject *__restrict value) {
	size_t fast_size;
	DREF DeeObject *elem;
	if (DeeObject_InstanceOfExact(value, (DeeTypeObject *)tp_self)) {
		uint8_t *dst, *src;
		size_t size; /* Copy-assign. */
		dst  = (uint8_t *)self;
		src  = (uint8_t *)DeeStruct_Data(value);
		size = DeeSType_Sizeof(tp_self);
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
		CTYPES_FAULTPROTECT(memcpy(dst, src, size), return -1);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
		CTYPES_FAULTPROTECT({
			while (size--)
				*dst++ = *src++;
		},
		return -1);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
		memcpy(dst, src, size);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
		return 0;
	}
	if (DeeNone_Check(value)) {
		uint8_t *dst;
		size_t size; /* Clear memory. */
		dst  = (uint8_t *)self;
		size = DeeSType_Sizeof(tp_self);
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
		CTYPES_FAULTPROTECT(memset(dst, 0, size), return -1);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
		CTYPES_FAULTPROTECT({
			while (size--)
				*dst++ = 0;
		},
		return -1);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
		memset(dst, 0, size);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
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
	fast_size = DeeFastSeq_GetSize(value);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fast_size; ++i) {
			int temp;
			elem = DeeFastSeq_GetItem(value, i);
			if
				unlikely(!elem)
			goto err;
			temp = struct_setpair(tp_self, self, elem);
			Dee_Decref(elem);
			if
				unlikely(temp)
			goto err;
		}
		return 0;
	}

	/* Use iterators. */
	value = DeeObject_IterSelf(value);
	if
		unlikely(!value)
	goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(value))) {
		int temp;
		temp = struct_setpair(tp_self, self, elem);
		Dee_Decref(elem);
		if
			unlikely(temp)
		goto err_value;
		if (DeeThread_CheckInterrupt())
			goto err_value;
	}
	if
		unlikely(!elem)
	goto err_value;
	Dee_Decref(value);
	return 0;
err_value:
	Dee_Decref(value);
err:
	return -1;
}

PRIVATE int DCALL
struct_init(DeeStructTypeObject *__restrict tp_self,
            void *self, size_t argc, DeeObject **__restrict argv) {
	DeeObject *value = Dee_None;
	if (DeeArg_Unpack(argc, argv, "|o:struct", &value))
		return -1;
	/* Do an initial assignment using the initializer. */
	return struct_assign(tp_self, self, value);
}


PRIVATE DREF DeeObject *DCALL
struct_repr(DeeStructTypeObject *__restrict tp_self, void *self) {
	size_t i;
	bool is_first                = true;
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	if (ascii_printer_printf(&printer, "%s { ", tp_self->st_base.st_base.tp_name) < 0)
		goto err;
	for (i = 0; i <= tp_self->st_fmsk; ++i) {
		if (!tp_self->st_fvec[i].sf_name)
			continue;
		if (!is_first && ASCII_PRINTER_PRINT(&printer, ", ") < 0)
			goto err;
		is_first = false;
		if (ascii_printer_printf(&printer, ".%k = %k(%K)",
		                         tp_self->st_fvec[i].sf_name,
		                         tp_self->st_fvec[i].sf_type->lt_orig,
		                         DeeStruct_Repr(tp_self->st_fvec[i].sf_type->lt_orig,
		                                        (void *)((uint8_t *)self +
		                                                 tp_self->st_fvec[i].sf_offset))) < 0)
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


INTERN DeeStructTypeObject DeeStruct_Type = {
	/* .st_base = */ {
		/* .st_base = */ {
			OBJECT_HEAD_INIT((DeeTypeObject *)&DeeStructType_Type),
			/* .tp_name     = */ "Struct",
			/* .tp_doc      = */ NULL,
			/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ (DeeTypeObject *)&DeeStructured_Type,
			/* .tp_init = */ {
				{
					/* .tp_alloc = */ {
						/* .tp_ctor      = */ NULL,
						/* .tp_copy_ctor = */ NULL,
						/* .tp_deep_ctor = */ NULL,
						/* .tp_any_ctor  = */ NULL,
						TYPE_FIXED_ALLOCATOR_S(DeeObject)
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
			/* .tp_seq           = */ NULL,
			/* .tp_iter_next     = */ NULL,
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
		/* .st_cachelock = */ RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
		/* .st_pointer  = */ &DeePointer_Type,
		/* .st_lvalue   = */ &DeeLValue_Type,
		/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
		/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
		/* .st_ffitype  = */ &ffi_type_void,
#endif /* !CONFIG_NO_CFUNCTION */
		/* .st_align    = */ CONFIG_CTYPES_ALIGNOF_POINTER,
		/* .st_init     = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, size_t, DeeObject **__restrict))&struct_init,
		/* .st_assign   = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&struct_assign,
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
};


DECL_END

#endif /* !GUARD_DEX_CTYPES_STRUCT_C */
