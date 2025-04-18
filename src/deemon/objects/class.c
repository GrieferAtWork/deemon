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
#ifndef GUARD_DEEMON_OBJECTS_CLASS_C
#define GUARD_DEEMON_OBJECTS_CLASS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/kwds.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/string.h>
#include <deemon/seq.h>
#include <deemon/super.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/util/objectlist.h>

#include <hybrid/typecore.h>

#include "../runtime/runtime_error.h"
/**/

#include <stdbool.h> /* bool */
#include <stdarg.h>  /* va_start */
#include <stddef.h>  /* size_t */
#include <stdint.h>  /* uint16_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

PRIVATE struct type_cmp instance_builtin_cmp = {
	/* .tp_hash          = */ &usrtype__hash__with__,
	/* .tp_compare_eq    = */ &usrtype__compare_eq__with__,
	/* .tp_compare       = */ &usrtype__compare__with__,
	/* .tp_trycompare_eq = */ &usrtype__trycompare_eq__with__,
	/* .tp_eq            = */ &default__eq__with__compare_eq,
	/* .tp_ne            = */ &default__ne__with__compare_eq,
	/* .tp_lo            = */ &default__lo__with__compare,
	/* .tp_le            = */ &default__le__with__compare,
	/* .tp_gr            = */ &default__gr__with__compare,
	/* .tp_ge            = */ &default__ge__with__compare
};

PRIVATE struct type_callable instance_user_callable = {
	/* .tp_call_kw     = */ &usrtype__call_kw__with__CALL,
	/* .tp_thiscall    = */ &default__thiscall__with__call,
	/* .tp_thiscall_kw = */ &default__thiscall_kw__with__call_kw,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ &default__call_tuple__with__call,
	/* .tp_call_tuple_kw     = */ &default__call_tuple_kw__with__call_kw,
	/* .tp_thiscall_tuple    = */ &default__thiscall_tuple__with__thiscall,
	/* .tp_thiscall_tuple_kw = */ &default__thiscall_tuple_kw__with__thiscall_kw,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};


PRIVATE WUNUSED NONNULL((1)) bool DCALL
is_operator_class_inherited(DeeTypeObject *__restrict type_type,
                            DeeTypeObject *__restrict type,
                            uint16_t oi_class, void *class_table) {
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, type);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		/* In order to have a chance of implementing "oi_class", the base type
		 * needs to be an instance of the type-type that provides "oi_class". */
		if (DeeObject_InstanceOf(base, type_type)) {
			void *base_class_table;
			base_class_table = *(void **)((byte_t *)base + oi_class);
			if (base_class_table == class_table)
				return true; /* Yup: it's inherited! */
		}
	}
	return false;
}

/* Class callbacks for inside of `type' */
INTERN NONNULL((1)) void DCALL
class_fini(DeeTypeObject *__restrict self) {
	struct class_desc *my_class;
	DREF DeeObject *buffer[64];
	size_t buflen;
	uint16_t i, size;
	my_class = self->tp_class;
	//my_class = DeeClass_DESC(self); /* This fails, because `self' may no longer be a valid object */

	/* Clear all class members (including cached operators). */
	size = my_class->cd_desc->cd_cmemb_size;
again:
	buflen = 0;
	Dee_class_desc_lock_write(my_class);
	for (i = 0; i < size; ++i) {
		DeeObject *ob;
		ob = my_class->cd_members[i];
		if (!ob)
			continue;
		my_class->cd_members[i] = NULL;
		if (Dee_DecrefIfNotOne(ob))
			continue;

		/* We're responsible for destroying this member! */
		if (buflen == COMPILER_LENOF(buffer)) {
			Dee_class_desc_lock_endwrite(my_class);
			Dee_Decref(ob);
			Dee_Decrefv(buffer, buflen);
			goto again;
		}
		buffer[buflen++] = ob; /* Inherit reference. */
	}

	/* Also clear all cached operators. */
	for (i = 0; i < CLASS_HEADER_OPC1; ++i) {
		struct class_optable *table;
		uint16_t j;
		table = my_class->cd_ops[i];
		if (!table)
			continue;
		for (j = 0; j < CLASS_HEADER_OPC2; ++j) {
			DeeObject *ob = table->co_operators[j];
			if (!ob)
				continue;
			table->co_operators[j] = NULL;
			if (Dee_DecrefIfNotOne(ob))
				continue;

			/* We're responsible for destroying this member! */
			if (buflen == COMPILER_LENOF(buffer)) {
				Dee_class_desc_lock_endwrite(my_class);
				Dee_Decref(ob);
				Dee_Decrefv(buffer, buflen);
				goto again;
			}
			buffer[buflen++] = ob; /* Inherit reference. */
		}
	}
	Dee_class_desc_lock_endwrite(my_class);
	if (buflen) {
		/* Clear the buffer. */
		Dee_Decrefv(buffer, buflen);

		/* Since custom destructors may have been able to
		 * re-assign new members, we must keep clearing them
		 * all until none are left! */
		goto again;
	}

	/* With all references objects who's destruction could potentially
	 * have side-effects now gone, we can move on to free heap-allocated
	 * data structures. */
	for (i = 0; i < CLASS_HEADER_OPC1; ++i)
		Dee_Free(my_class->cd_ops[i]);
	Dee_Decref(my_class->cd_desc);

	/* Hide pre-defined operator tables from "self"
	 * (so our caller won't try to Dee_Free() them) */
	if (self->tp_cmp == &instance_builtin_cmp)
		self->tp_cmp = NULL;
	if (self->tp_callable == &instance_user_callable)
		self->tp_callable = NULL;

	/* Custom operators. */
	Dee_Free((void *)self->tp_operators);

	/* Free custom operator class tables that weren't inherited from bases. */
	if (Dee_TYPE(self) != &DeeType_Type) {
		DeeTypeObject *type_type = Dee_TYPE(self);
		do {
			size_t opi;
			ASSERT(DeeType_IsTypeType(type_type));
			for (opi = 0; opi < type_type->tp_operators_size; ++opi) {
				struct type_operator const *op = &type_type->tp_operators[opi];
				if likely(type_operator_isdecl(op)) {
					void *class_table;
					uint16_t class_offset = op->to_decl.oi_class;
					if (class_offset == 0)
						continue; /* Inline class (ignore) */
					class_table = *(void **)((byte_t *)self + class_offset);
					if (class_table == NULL)
						continue; /* Class isn't allocated. */
					/* Check if "class_table" has been inherited from a base class. */
					if (!is_operator_class_inherited(type_type, self, class_offset, class_table)) {
						/* Table isn't inherited, meaning it was allocated for use by `self' */
						Dee_Free(class_table);
					}

					/* NULL the table pointer so we don't try to double-free it. */
					*(void **)((byte_t *)self + class_offset) = NULL;
				}
			}
			type_type = DeeType_Base(type_type);
		} while (type_type != &DeeType_Type);
	}
}

INTERN NONNULL((1, 2)) void DCALL
class_visit(DeeTypeObject *__restrict self, dvisit_t proc, void *arg) {
	struct class_desc *my_class;
	uint16_t i, size;
	my_class = DeeClass_DESC(self);
	size     = my_class->cd_desc->cd_cmemb_size;
	Dee_class_desc_lock_read(my_class);
	Dee_XVisitv(my_class->cd_members, size);

	/* Also free all cached operators. */
	for (i = 0; i < CLASS_HEADER_OPC1; ++i) {
		struct class_optable *table;
		table = my_class->cd_ops[i];
		if (!table)
			continue;
		Dee_XVisitv(table->co_operators, CLASS_HEADER_OPC2);
	}
	Dee_class_desc_lock_endread(my_class);
	/* Only ever references strings itself, so no point in visiting this one! */
	/*Dee_Visit(my_class->cd_desc);*/
}

INTERN NONNULL((1)) void DCALL
class_clear(DeeTypeObject *__restrict self) {
	struct class_desc *my_class;
	DREF DeeObject *buffer[64];
	size_t buflen;
	uint16_t i, size;
	my_class = DeeClass_DESC(self);
	/* Clear all class members (including cached operators). */
	size = my_class->cd_desc->cd_cmemb_size;
again:
	buflen = 0;
	Dee_class_desc_lock_write(my_class);
	for (i = 0; i < size; ++i) {
		DeeObject *ob;
		ob = my_class->cd_members[i];
		if (!ob)
			continue;
		my_class->cd_members[i] = NULL;
		if (Dee_DecrefIfNotOne(ob))
			continue;
		/* We're responsible for destroying this member! */
		if (buflen == COMPILER_LENOF(buffer)) {
			Dee_class_desc_lock_endwrite(my_class);
			Dee_Decref(ob);
			Dee_Decrefv(buffer, buflen);
			goto again;
		}
		buffer[buflen++] = ob; /* Inherit reference. */
	}
	/* Also clear all cached operators. */
	for (i = 0; i < CLASS_HEADER_OPC1; ++i) {
		struct class_optable *table;
		uint16_t j;
		table = my_class->cd_ops[i];
		if (!table)
			continue;
		for (j = 0; j < CLASS_HEADER_OPC2; ++j) {
			DeeObject *ob = table->co_operators[j];
			if (!ob)
				continue;
			table->co_operators[j] = NULL;
			if (Dee_DecrefIfNotOne(ob))
				continue;
			/* We're responsible for destroying this member! */
			if (buflen == COMPILER_LENOF(buffer)) {
				Dee_class_desc_lock_endwrite(my_class);
				Dee_Decref(ob);
				Dee_Decrefv(buffer, buflen);
				goto again;
			}
			buffer[buflen++] = ob; /* Inherit reference. */
		}
	}
	Dee_class_desc_lock_endwrite(my_class);
	if (buflen) {
		/* Clear the buffer. */
		Dee_Decrefv(buffer, buflen);
		/* Since custom destructors may have been able to
		 * re-assign new members, we must keep clearing them
		 * all until none are left! */
		goto again;
	}
}

INTERN NONNULL((1)) void DCALL
class_pclear(DeeTypeObject *__restrict self, unsigned int gc_priority) {
	struct class_desc *my_class;
	DREF DeeObject *buffer[64];
	size_t buflen;
	uint16_t i, size;
	my_class = DeeClass_DESC(self);
	/* Clear all class members (including cached operators). */
	size = my_class->cd_desc->cd_cmemb_size;
again:
	buflen = 0;
	Dee_class_desc_lock_write(my_class);
	for (i = 0; i < size; ++i) {
		DeeObject *ob;
		ob = my_class->cd_members[i];
		if (!ob)
			continue;
		if (DeeObject_GCPriority(ob) < gc_priority)
			continue;
		my_class->cd_members[i] = NULL;
		if (Dee_DecrefIfNotOne(ob))
			continue;
		/* We're responsible for destroying this member! */
		if (buflen == COMPILER_LENOF(buffer)) {
			Dee_class_desc_lock_endwrite(my_class);
			Dee_Decref(ob);
			Dee_Decrefv(buffer, buflen);
			goto again;
		}
		buffer[buflen++] = ob; /* Inherit reference. */
	}
	/* Also clear all cached operators. */
	for (i = 0; i < CLASS_HEADER_OPC1; ++i) {
		struct class_optable *table;
		uint16_t j;
		table = my_class->cd_ops[i];
		if (!table)
			continue;
		for (j = 0; j < CLASS_HEADER_OPC2; ++j) {
			DeeObject *ob = table->co_operators[j];
			if (!ob)
				continue;
			if (DeeObject_GCPriority(ob) < gc_priority)
				continue;
			table->co_operators[j] = NULL;
			if (Dee_DecrefIfNotOne(ob))
				continue;
			/* We're responsible for destroying this member! */
			if (buflen == COMPILER_LENOF(buffer)) {
				Dee_class_desc_lock_endwrite(my_class);
				Dee_Decref(ob);
				Dee_Decrefv(buffer, buflen);
				goto again;
			}
			buffer[buflen++] = ob; /* Inherit reference. */
		}
	}
	Dee_class_desc_lock_endwrite(my_class);
	if (buflen) {
		/* Clear the buffer. */
		Dee_Decrefv(buffer, buflen);
		/* Since custom destructors may have been able to
		 * re-assign new members, we must keep clearing them
		 * all until none are left! */
		goto again;
	}
}


PRIVATE NONNULL((1, 3)) void DCALL
class_desc_cache_operator(struct class_desc *__restrict self,
                          Dee_operator_t name, DeeObject *__restrict func) {
	struct class_optable *table;
	Dee_operator_t table_index;
	ASSERT(name < CLASS_OPERATOR_USERCOUNT);
	table_index = name / CLASS_HEADER_OPC2;
again:
	table = atomic_read(&self->cd_ops[table_index]);
	if (!table) {
		table = (struct class_optable *)Dee_TryCalloc(sizeof(struct class_optable));
		if unlikely(!table)
			goto done;
		if unlikely(!atomic_cmpxch_weak(&self->cd_ops[table_index], NULL, table)) {
			/* Table was already allocated! */
			Dee_Free(table);
			goto again;
		}
	}

	/* Cache the operator function in the callback table. */
	Dee_Incref(func);
	if (!atomic_cmpxch(&table->co_operators[name % CLASS_HEADER_OPC2], NULL, func))
		Dee_DecrefNokill(func);
done:
	;
}

#define OPERATOR_IS_CONSTRUCTOR_INHERITED(x) \
	((x) <= OPERATOR_MOVEASSIGN || (x) == CLASS_OPERATOR_SUPERARGS)

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
class_desc_get_known_operator(DeeTypeObject *__restrict tp_self,
                              struct class_desc *__restrict self,
                              Dee_operator_t name) {
	DREF DeeObject *result;
	DeeClassDescriptorObject *desc;
	Dee_operator_t i, perturb;
	if (name < CLASS_OPERATOR_USERCOUNT) {
		struct class_optable *table;
		table = self->cd_ops[name / CLASS_HEADER_OPC2];
		if likely(table) {
			Dee_class_desc_lock_read(self);
			result = table->co_operators[name % CLASS_HEADER_OPC2];
			if likely(result) {
				Dee_Incref(result);
				Dee_class_desc_lock_endread(self);
				return result;
			}
			Dee_class_desc_lock_endread(self);
		}
	}

	/* Lookup extended, or un-cached operators. */
	desc = self->cd_desc;
	i = perturb = name & desc->cd_clsop_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_operator *entry;
		entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
		ASSERTF(entry->co_name != (Dee_operator_t)-1, "Operator %#I16x not implemented", name);
		if (entry->co_name != name)
			continue;

		/* Found the entry! */
		ASSERT(entry->co_addr < desc->cd_cmemb_size);
		Dee_class_desc_lock_read(self);
		result = self->cd_members[entry->co_addr];
		if unlikely(!result) {
			Dee_class_desc_lock_endread(self);
			err_unimplemented_operator(tp_self, name);
			return NULL;
		}
		Dee_Incref(result);
		Dee_class_desc_lock_endread(self);

		/* Try to cache the associated operator (if possible) */
		if (name < CLASS_OPERATOR_USERCOUNT)
			class_desc_cache_operator(self, name, result);
		return result;
	}
}

/* Return the nearest operator function for `name',
 * implemented by `self', which must be a class type.
 * If the operator doesn't exist, return NULL and throw
 * a NotImplemented error, or return NULL and don't throw
 * an error when `DeeClass_TryGetOperator()' was used. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeClass_GetOperator(DeeTypeObject const *__restrict self, Dee_operator_t name) {
	DREF DeeObject *result;
	DeeTypeObject const *iter = self;
	DeeTypeMRO mro;
	ASSERT(DeeType_IsClass(iter));
	DeeTypeMRO_Init(&mro, iter);
	do {
		/* Search the descriptor cache of this type. */
		struct class_desc *iter_class;
		DeeClassDescriptorObject *desc;
		Dee_operator_t i, perturb;
		iter_class = DeeClass_DESC(iter);
		if (name < CLASS_OPERATOR_USERCOUNT) {
			struct class_optable *table;
			table = iter_class->cd_ops[name / CLASS_HEADER_OPC2];
			if likely(table) {
				Dee_class_desc_lock_read(iter_class);
				result = table->co_operators[name % CLASS_HEADER_OPC2];
				if likely(result) {
					Dee_Incref(result);
					Dee_class_desc_lock_endread(iter_class);
					/* Inherit the base's operator locally, by caching it. */
					if (iter != self && !OPERATOR_IS_CONSTRUCTOR_INHERITED(name))
						class_desc_cache_operator(DeeClass_DESC(self), name, result);
					return result;
				}
				Dee_class_desc_lock_endread(iter_class);
			}
		}

		/* Search the operator table of the type. */
		desc = iter_class->cd_desc;
		i = perturb = name & desc->cd_clsop_mask;
		for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
			struct class_operator *entry;
			entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
			if (entry->co_name != name) {
				if (entry->co_name == (Dee_operator_t)-1)
					break; /* Not implemented! */
				continue;
			}

			/* Found the entry! */
			ASSERT(entry->co_addr < desc->cd_cmemb_size);
			Dee_class_desc_lock_read(iter_class);
			result = iter_class->cd_members[entry->co_addr];
			if unlikely(!result) {
				Dee_class_desc_lock_endread(iter_class);
				goto done; /* Deleted operator. */
			}
			Dee_Incref(result);
			Dee_class_desc_lock_endread(iter_class);

			/* Try to cache the associated operator (if possible)
			 * NOTE: Make sure not to accidentally cache inherited constructors! */
			if (name < CLASS_OPERATOR_USERCOUNT) {
				if (iter == self || !OPERATOR_IS_CONSTRUCTOR_INHERITED(name))
					class_desc_cache_operator(DeeClass_DESC(self), name, result);
			}
			return result;
		}
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL && DeeType_IsClass(iter));
done:
	err_unimplemented_operator(self, name);
	return NULL;
}

/* Same as `DeeClass_GetOperator()', but don't simply return `NULL'
 * if the operator hasn't been implemented, and `ITER_DONE' when it
 * has been, but wasn't assigned anything. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeClass_TryGetOperator(DeeTypeObject const *__restrict self, Dee_operator_t name) {
	DREF DeeObject *result;
	DeeTypeObject const *iter = self;
	DeeTypeMRO mro;
	ASSERT(DeeType_IsClass(iter));
	DeeTypeMRO_Init(&mro, iter);
	do {
		/* Search the descriptor cache of this type. */
		struct class_desc *iter_class;
		DeeClassDescriptorObject *desc;
		Dee_operator_t i, perturb;
		iter_class = DeeClass_DESC(iter);
		if (name < CLASS_OPERATOR_USERCOUNT) {
			struct class_optable *table;
			table = iter_class->cd_ops[name / CLASS_HEADER_OPC2];
			if likely(table) {
				Dee_class_desc_lock_read(iter_class);
				result = table->co_operators[name % CLASS_HEADER_OPC2];
				if likely(result) {
					Dee_Incref(result);
					Dee_class_desc_lock_endread(iter_class);
					/* Inherit the base's operator locally, by caching it. */
					if (iter != self && !OPERATOR_IS_CONSTRUCTOR_INHERITED(name))
						class_desc_cache_operator(DeeClass_DESC(self), name, result);
					return result;
				}
				Dee_class_desc_lock_endread(iter_class);
			}
		}

		/* Search the operator table of the type. */
		desc = iter_class->cd_desc;
		i = perturb = name & desc->cd_clsop_mask;
		for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
			struct class_operator *entry;
			entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
			if (entry->co_name != name) {
				if (entry->co_name == (Dee_operator_t)-1)
					break; /* Not implemented! */
				continue;
			}

			/* Found the entry! */
			ASSERT(entry->co_addr < desc->cd_cmemb_size);
			Dee_class_desc_lock_read(iter_class);
			result = iter_class->cd_members[entry->co_addr];
			if unlikely(!result) {
				Dee_class_desc_lock_endread(iter_class);
				return NULL; /* Deleted operator. */
			}
			Dee_Incref(result);
			Dee_class_desc_lock_endread(iter_class);

			/* Try to cache the associated operator (if possible)
			 * NOTE: Make sure not to accidentally cache inherited constructors! */
			if (name < CLASS_OPERATOR_USERCOUNT) {
				if (iter == self || !OPERATOR_IS_CONSTRUCTOR_INHERITED(name))
					class_desc_cache_operator(DeeClass_DESC(self), name, result);
			}
			return result;
		}
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL && DeeType_IsClass(iter));
	return NULL;
}

/* Same as `DeeClass_TryGetOperator()', but don't return an operator
 * that has been inherited from a base-class, but return `NULL' instead. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeClass_TryGetPrivateOperator(DeeTypeObject const *__restrict self, Dee_operator_t name) {
	DeeClassDescriptorObject *desc;
	Dee_operator_t i, perturb;
	struct class_desc *self_class;
	self_class = DeeClass_DESC(self);

	/* Lookup extended, or un-cached operators. */
	desc = self_class->cd_desc;
	i = perturb = name & desc->cd_clsop_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		DREF DeeObject *result;
		struct class_operator *entry;
		entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
		if (entry->co_name != name) {
			if (entry->co_name == (Dee_operator_t)-1)
				break; /* Not implemented! */
			continue;
		}

		/* Found the entry! */
		ASSERT(entry->co_addr < desc->cd_cmemb_size);
		Dee_class_desc_lock_read(self_class);
		result = self_class->cd_members[entry->co_addr];
		if unlikely(!result) {
			Dee_class_desc_lock_endread(self_class);
			break; /* Deleted operator. */
		}
		Dee_Incref(result);
		Dee_class_desc_lock_endread(self_class);
		return result;
	}
	return NULL;
}


PUBLIC WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeClass_CallOperator(DeeTypeObject const *__restrict tp_self, DeeObject *self,
                      Dee_operator_t name, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *func;
	func = DeeClass_GetOperator(tp_self, name);
	if unlikely(!func)
		goto err;
	return DeeObject_ThisCallInherited(func, self, argc, argv);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *
DeeClass_CallOperatorf(DeeTypeObject const *__restrict tp_self, DeeObject *self,
                       Dee_operator_t name, char const *format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeClass_VCallOperatorf(tp_self, self, name, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeClass_VCallOperatorf(DeeTypeObject const *__restrict tp_self, DeeObject *self,
                        Dee_operator_t name, char const *format, va_list args) {
	DREF DeeObject *func = DeeClass_GetOperator(tp_self, name);
	if unlikely(!func)
		goto err;
	return DeeObject_VThisCallInheritedf(func, self, format, args);
err:
	return NULL;
}



/* Same as `DeeClass_TryGetPrivateOperator()', but don't return a reference */
INTERN ATTR_PURE WUNUSED NONNULL((1)) DeeObject *DCALL
DeeClass_TryGetPrivateOperatorPtr(DeeTypeObject const *__restrict self, Dee_operator_t name) {
	DeeClassDescriptorObject *desc;
	Dee_operator_t i, perturb;
	struct class_desc *self_class;
	self_class = DeeClass_DESC(self);

	/* Lookup extended, or un-cached operators. */
	desc = self_class->cd_desc;
	i = perturb = name & desc->cd_clsop_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_operator *entry;
		entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
		if (entry->co_name != name) {
			if (entry->co_name == (Dee_operator_t)-1)
				break; /* Not implemented! */
			continue;
		}

		/* Found the entry! */
		ASSERT(entry->co_addr < desc->cd_cmemb_size);
		return atomic_read(&self_class->cd_members[entry->co_addr]);
	}
	return NULL;
}


INTERN NONNULL((1)) void DCALL
instance_clear_members(struct instance_desc *__restrict self, uint16_t size) {
	DREF DeeObject *buffer[64];
	size_t buflen;
	uint16_t i;
again:
	buflen = 0;
	Dee_instance_desc_lock_write(self);
	for (i = 0; i < size; ++i) {
		DeeObject *ob;
		ob = self->id_vtab[i];
		if (!ob)
			continue;
		self->id_vtab[i] = NULL;
		if (Dee_DecrefIfNotOne(ob))
			continue;

		/* We're responsible for destroying this member! */
		if (buflen == COMPILER_LENOF(buffer)) {
			Dee_instance_desc_lock_endwrite(self);
			Dee_Decref(ob);
			Dee_Decrefv(buffer, buflen);
			goto again;
		}
		buffer[buflen++] = ob; /* Inherit reference. */
	}
	Dee_instance_desc_lock_endwrite(self);
	if (buflen) {
		/* Clear the buffer. */
		Dee_Decrefv(buffer, buflen);

		/* Since custom destructors may have been able to
		 * re-assign new members, we must keep clearing them
		 * all until none are left! */
		goto again;
	}
}


INTERN NONNULL((1)) void DCALL
instance_builtin_destructor(DeeObject *__restrict self) {
	struct class_desc *desc;
	desc = DeeClass_DESC(Dee_TYPE(self));
	/* Clear all the members of this instance. */
	instance_clear_members(DeeInstance_DESC(desc, self),
	                       desc->cd_desc->cd_imemb_size);
}

INTERN NONNULL((1)) void DCALL
instance_destructor(DeeObject *__restrict self) {
	DeeTypeObject *tp_self  = Dee_TYPE(self);
	struct class_desc *desc = DeeClass_DESC(tp_self);
	DREF DeeObject *callback, *result;
	callback = class_desc_get_known_operator(tp_self, desc, OPERATOR_DESTRUCTOR);
	if unlikely(!callback) {
		result = NULL;
	} else {
		Dee_refcnt_t new_refcnt;
		atomic_inc(&self->ob_refcnt);
		result = DeeObject_ThisCallInherited(callback, self, 0, NULL);

		/* Check if `self' got revived. - If it did we let the caller
		 * inherit a reference to it to prevent a race condition. */
		for (;;) {
			new_refcnt = atomic_read(&self->ob_refcnt);
			if (new_refcnt > 1) {
				/* Object got revived (let the caller inherit our reference) */
				if likely(result) {
					Dee_Decref(result);
				} else {
					DeeError_Print("Unhandled error in destructor\n",
					               ERROR_PRINT_DOHANDLE);
				}
				return;
			}
			if (atomic_cmpxch_weak(&self->ob_refcnt, new_refcnt, 0))
				break;
		}
	}
	if likely(result) {
		Dee_Decref(result);
	} else {
		DeeError_Print("Unhandled error in destructor\n",
		               ERROR_PRINT_DOHANDLE);
	}

	/* Clear all the members of this instance. */
	instance_clear_members(DeeInstance_DESC(desc, self),
	                       desc->cd_desc->cd_imemb_size);
}


#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
#define IF_NOBASE(x) x
#else /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
#define IF_NOBASE(x) /* nothing */
#endif /* !CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
#ifdef CLASS_TP_FAUTOINIT
#define IF_AUTOINIT(x) x
#else /* CLASS_TP_FAUTOINIT */
#define IF_AUTOINIT(x) /* nothing */
#endif /* !CLASS_TP_FAUTOINIT */

#define IF_AUTOINITNB(x) IF_NOBASE(IF_AUTOINIT(x))




#define DeeType_INVOKE_COPY(func, tp_self, self, other)                                                              \
	               ((func) == &instance_builtin_copy ? instance_builtin_tcopy(tp_self, self, other) :                \
	      IF_NOBASE((func) == &instance_builtin_nobase_copy ? instance_builtin_nobase_tcopy(tp_self, self, other) :) \
	                (func) == &instance_copy ? instance_tcopy(tp_self, self, other) :                                \
	              (*(func))(self, other))
#define DeeType_INVOKE_DEEPCOPY(func, tp_self, self, other)                                   \
	               ((func) == &instance_deepcopy ? instance_tdeepcopy(tp_self, self, other) : \
	              (*(func))(self, other))
#define DeeType_INVOKE_CTOR(func, tp_self, self)                                                                          \
	               ((func) == &instance_builtin_ctor ? instance_builtin_tctor(tp_self, self) :                            \
	      IF_NOBASE((func) == &instance_builtin_nobase_ctor ? instance_builtin_nobase_tctor(tp_self, self) :)             \
	                (func) == &instance_builtin_inherited_ctor ? instance_builtin_inherited_tctor(tp_self, self) :        \
	                (func) == &instance_builtin_super_ctor ? instance_builtin_super_tctor(tp_self, self) :                \
	                (func) == &instance_builtin_kwsuper_ctor ? instance_builtin_kwsuper_tctor(tp_self, self) :            \
	                (func) == &instance_ctor ? instance_tctor(tp_self, self) :                                            \
	      IF_NOBASE((func) == &instance_nobase_ctor ? instance_nobase_tctor(tp_self, self) :)                             \
	              /*(func) == &instance_inherited_ctor ? instance_inherited_tctor(tp_self, self) :*/                      \
	                (func) == &instance_super_ctor ? instance_super_tctor(tp_self, self) :                                \
	                (func) == &instance_kwsuper_ctor ? instance_kwsuper_tctor(tp_self, self) :                            \
	  /*IF_AUTOINIT((func) == &instance_auto_ctor ? instance_auto_tctor(tp_self, self) :)*/                               \
	  /*IF_AUTOINIT((func) == &instance_builtin_auto_ctor ? instance_builtin_auto_tctor(tp_self, self) :)*/               \
	/*IF_AUTOINITNB((func) == &instance_auto_nobase_ctor ? instance_auto_nobase_tctor(tp_self, self) :)*/                 \
	/*IF_AUTOINITNB((func) == &instance_builtin_auto_nobase_ctor ? instance_builtin_auto_nobase_tctor(tp_self, self) :)*/ \
	              (*(func))(self))
#define DeeType_INVOKE_ANY_CTOR(func, tp_self, self, argc, argv)                                                                    \
	               ((func) == &instance_builtin_init ? instance_builtin_tinit(tp_self, self, argc, argv) :                          \
	      IF_NOBASE((func) == &instance_builtin_nobase_init ? instance_builtin_nobase_tinit(tp_self, self, argc, argv) :)           \
	                (func) == &instance_builtin_inherited_init ? instance_builtin_inherited_tinit(tp_self, self, argc, argv) :      \
	                (func) == &instance_builtin_super_init ? instance_builtin_super_tinit(tp_self, self, argc, argv) :              \
	                (func) == &instance_builtin_kwsuper_init ? instance_builtin_kwsuper_tinit(tp_self, self, argc, argv) :          \
	                (func) == &instance_init ? instance_tinit(tp_self, self, argc, argv) :                                          \
	      IF_NOBASE((func) == &instance_nobase_init ? instance_nobase_tinit(tp_self, self, argc, argv) :)                           \
	                (func) == &instance_inherited_init ? instance_inherited_tinit(tp_self, self, argc, argv) :                      \
	                (func) == &instance_super_init ? instance_super_tinit(tp_self, self, argc, argv) :                              \
	                (func) == &instance_kwsuper_init ? instance_kwsuper_tinit(tp_self, self, argc, argv) :                          \
	    IF_AUTOINIT((func) == &instance_auto_init ? instance_auto_tinit(tp_self, self, argc, argv) :)                               \
	    IF_AUTOINIT((func) == &instance_builtin_auto_init ? instance_builtin_auto_tinit(tp_self, self, argc, argv) :)               \
	  IF_AUTOINITNB((func) == &instance_auto_nobase_init ? instance_auto_nobase_tinit(tp_self, self, argc, argv) :)                 \
	  IF_AUTOINITNB((func) == &instance_builtin_auto_nobase_init ? instance_builtin_auto_nobase_tinit(tp_self, self, argc, argv) :) \
	              (*(func))(self, argc, argv))
#define DeeType_INVOKE_ANY_CTOR_KW(func, tp_self, self, argc, argv, kw)                                                                     \
	               ((func) == &instance_builtin_initkw ? instance_builtin_tinitkw(tp_self, self, argc, argv, kw) :                          \
	      IF_NOBASE((func) == &instance_builtin_nobase_initkw ? instance_builtin_nobase_tinitkw(tp_self, self, argc, argv, kw) :)           \
	                (func) == &instance_builtin_inherited_initkw ? instance_builtin_inherited_tinitkw(tp_self, self, argc, argv, kw) :      \
	                (func) == &instance_builtin_super_initkw ? instance_builtin_super_tinitkw(tp_self, self, argc, argv, kw) :              \
	                (func) == &instance_builtin_kwsuper_initkw ? instance_builtin_kwsuper_tinitkw(tp_self, self, argc, argv, kw) :          \
	                (func) == &instance_initkw ? instance_tinitkw(tp_self, self, argc, argv, kw) :                                          \
	      IF_NOBASE((func) == &instance_nobase_initkw ? instance_nobase_tinitkw(tp_self, self, argc, argv, kw) :)                           \
	                (func) == &instance_inherited_initkw ? instance_inherited_tinitkw(tp_self, self, argc, argv, kw) :                      \
	                (func) == &instance_super_initkw ? instance_super_tinitkw(tp_self, self, argc, argv, kw) :                              \
	                (func) == &instance_kwsuper_initkw ? instance_kwsuper_tinitkw(tp_self, self, argc, argv, kw) :                          \
	    IF_AUTOINIT((func) == &instance_auto_initkw ? instance_auto_tinitkw(tp_self, self, argc, argv, kw) :)                               \
	    IF_AUTOINIT((func) == &instance_builtin_auto_initkw ? instance_builtin_auto_tinitkw(tp_self, self, argc, argv, kw) :)               \
	  IF_AUTOINITNB((func) == &instance_auto_nobase_initkw ? instance_auto_nobase_tinitkw(tp_self, self, argc, argv, kw) :)                 \
	  IF_AUTOINITNB((func) == &instance_builtin_auto_nobase_initkw ? instance_builtin_auto_nobase_tinitkw(tp_self, self, argc, argv, kw) :) \
	              (*(func))(self, argc, argv, kw))


INTERN WUNUSED NONNULL((1, 2)) int DCALL
type_invoke_base_constructor(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self, size_t argc,
                             DeeObject *const *argv, DeeObject *kw) {
	ASSERT(!(tp_self->tp_flags & TP_FVARIABLE));
	if (kw) {
		if (tp_self->tp_init.tp_alloc.tp_any_ctor_kw) {
			int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *, DeeObject *);
			func = tp_self->tp_init.tp_alloc.tp_any_ctor_kw;
			return DeeType_INVOKE_ANY_CTOR_KW(func, tp_self, self, argc, argv, kw);
		}
		if (DeeKwds_Check(kw)) {
			if (DeeKwds_SIZE(kw) != 0)
				goto err_no_keywords;
		} else {
			size_t kw_size = DeeObject_Size(kw);
			if unlikely(kw_size == (size_t)-1)
				goto err;
			if (kw_size != 0)
				goto err_no_keywords;
		}
	}
	if (tp_self->tp_init.tp_alloc.tp_ctor && !argc) {
		int (DCALL *func)(DeeObject *__restrict);
		func = tp_self->tp_init.tp_alloc.tp_ctor;
		return DeeType_INVOKE_CTOR(func, tp_self, self);
	}
	if (tp_self->tp_init.tp_alloc.tp_any_ctor) {
		int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *);
		func = tp_self->tp_init.tp_alloc.tp_any_ctor;
		return DeeType_INVOKE_ANY_CTOR(func, tp_self, self, argc, argv);
	}
	if (tp_self->tp_init.tp_alloc.tp_copy_ctor &&
	    (argc == 1 && DeeObject_InstanceOf(argv[0], tp_self))) {
		int (DCALL *func)(DeeObject *__restrict, DeeObject *__restrict);
		func = tp_self->tp_init.tp_alloc.tp_copy_ctor;
		return DeeType_INVOKE_COPY(func, tp_self, self, argv[0]);
	}
	return err_unimplemented_constructor(tp_self, argc, argv);
err_no_keywords:
	return err_keywords_ctor_not_accepted(tp_self, kw);
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_initsuper_as_copy(DeeTypeObject *tp_super,
                           DeeObject *__restrict self,
                           DeeObject *other,
                           bool deep_copy) {
	int result;

	/* Handle constructor inheritance */
	while (tp_super->tp_flags & TP_FINHERITCTOR) {
		ASSERTF(!(tp_super->tp_flags & TP_FFINAL),
		        "Type derived from final type");
		ASSERT(DeeType_Base(tp_super));
		tp_super = DeeType_Base(tp_super);
	}
	ASSERTF(!(tp_super->tp_flags & TP_FVARIABLE), "Type derived from variable type");

	/* Initialize the super-type. */
	if (tp_super->tp_init.tp_alloc.tp_deep_ctor && deep_copy) {
		int (DCALL *func)(DeeObject *__restrict, DeeObject *__restrict);
		func   = tp_super->tp_init.tp_alloc.tp_deep_ctor;
		result = DeeType_INVOKE_DEEPCOPY(func, tp_super, self, other);
	} else if (tp_super->tp_init.tp_alloc.tp_copy_ctor) {
		int (DCALL *func)(DeeObject *__restrict, DeeObject *__restrict);
		func   = tp_super->tp_init.tp_alloc.tp_copy_ctor;
		result = DeeType_INVOKE_COPY(func, tp_super, self, other);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor) {
		int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor;
		result = DeeType_INVOKE_ANY_CTOR(func, tp_super, self, 1, (DeeObject **)&other);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor_kw) {
		int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *, DeeObject *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor_kw;
		result = DeeType_INVOKE_ANY_CTOR_KW(func, tp_super, self, 1, (DeeObject **)&other, NULL);
	} else {
		result = err_unimplemented_operator(tp_super,
		                                    deep_copy ? OPERATOR_COPY
		                                              : OPERATOR_DEEPCOPY);
	}
	return result;
}



PRIVATE char const str_shared_ctor_failed[] = "Constructor of shared object failed\n";
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_tcopy(DeeTypeObject *tp_self,
               DeeObject *__restrict self,
               DeeObject *other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;

	/* Lookup the copy operator function. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_COPY);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_copy(tp_super, self, other, false))
			goto err_members;
	}
	result = DeeObject_ThisCall(func, self, 1, (DeeObject **)&other);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_tdeepcopy(DeeTypeObject *tp_self,
                   DeeObject *__restrict self,
                   DeeObject *other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;

	/* Lookup the copy operator function. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_DEEPCOPY);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_copy(tp_super, self, other, true))
			goto err_members;
	}
	ASSERT(!tp_self->tp_init.tp_deepload);

	/* Add a deepcopy association for `self' replacing `other' */
	if unlikely(!Dee_DeepCopyAddAssoc(self, other))
		goto err_super;
	result = DeeObject_ThisCall(func, self, 1, (DeeObject **)&other);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_copy(DeeObject *__restrict self,
              DeeObject *__restrict other) {
	return instance_tcopy(Dee_TYPE(self), self, other);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_deepcopy(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
	return instance_tdeepcopy(Dee_TYPE(self), self, other);
}


INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_builtin_tcopy(DeeTypeObject *tp_self,
                       DeeObject *__restrict self,
                       DeeObject *other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	struct instance_desc *other_instance;
	DeeTypeObject *tp_super;
	uint16_t size;
	ASSERT(DeeObject_InstanceOf(other, tp_self));

	/* Initialize the members of this instance as
	 * references to the same also found in `other'. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	Dee_instance_desc_lock_read(other_instance);
	Dee_XMovrefv(instance->id_vtab, other_instance->id_vtab, size);
	Dee_instance_desc_lock_endread(other_instance);

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_copy(tp_super, self, other, false))
			goto err_members;
	}
	return 0;
err_members:
	instance_clear_members(instance, size);
	return -1;
}

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_builtin_nobase_tcopy(DeeTypeObject *tp_self,
                              DeeObject *__restrict self,
                              DeeObject *other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	struct instance_desc *other_instance;
	uint16_t size;
	ASSERT(DeeObject_InstanceOf(other, tp_self));

	/* Initialize the members of this instance as
	 * references to the same also found in `other'. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	Dee_instance_desc_lock_read(other_instance);
	Dee_XMovrefv(instance->id_vtab, other_instance->id_vtab, size);
	Dee_instance_desc_lock_endread(other_instance);
	return 0;
}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_tdeepload(DeeTypeObject *tp_self,
                           DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	uint16_t i, size;

	/* Replace all members with deep copies of themself. */
	size = desc->cd_desc->cd_imemb_size;
	for (i = 0; i < size; ++i) {
		if (DeeObject_XInplaceDeepCopyWithRWLock(&instance->id_vtab[i],
		                                         &instance->id_lock))
			goto err;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_copy(DeeObject *__restrict self,
                      DeeObject *__restrict other) {
	return instance_builtin_tcopy(Dee_TYPE(self), self, other);
}

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_nobase_copy(DeeObject *__restrict self,
                             DeeObject *__restrict other) {
	return instance_builtin_nobase_tcopy(Dee_TYPE(self), self, other);
}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_deepload(DeeObject *__restrict self) {
	DeeTypeObject *tp_self;
	tp_self = Dee_TYPE(self);
	do {
		if unlikely(instance_builtin_tdeepload(tp_self, self))
			goto err;
	} while ((tp_self = DeeType_Base(tp_self)) != NULL &&
	         DeeType_IsClass(tp_self));

	/* Invoke deepload for all non-user defined base classes. */
	for (; tp_self; tp_self = DeeType_Base(tp_self)) {
		if (!tp_self->tp_init.tp_deepload)
			continue;
		if unlikely((*tp_self->tp_init.tp_deepload)(self))
			goto err;
	}
	return 0;
err:
	return -1;
}

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_nobase_deepload(DeeObject *__restrict self) {
	return instance_builtin_tdeepload(Dee_TYPE(self), self);
}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */




INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_builtin_tassign(DeeTypeObject *tp_self,
                         DeeObject *self,
                         DeeObject *other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	struct instance_desc *other_instance;
	DeeTypeObject *tp_super;
	uint16_t i, size;
	DREF DeeObject **old_items;
	if unlikely(self == other)
		goto done;
	if (DeeObject_AssertImplements(other, tp_self))
		goto err;
	tp_super = DeeType_Base(tp_self);
	if (tp_super && DeeObject_TAssign(tp_super, self, other))
		goto err;
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	old_items      = (DREF DeeObject **)Dee_Mallocac(size, sizeof(DREF DeeObject *));
	if unlikely(!old_items)
		goto err;

	/* Load member values from `others' */
	Dee_instance_desc_lock_read(other_instance);
	Dee_XMovrefv(old_items, other_instance->id_vtab, size);
	Dee_instance_desc_lock_endread(other_instance);

	/* Exchange our own member values with those loaded from `other' */
	Dee_instance_desc_lock_write(instance);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *temp;
		temp                 = instance->id_vtab[i];
		instance->id_vtab[i] = old_items[i];
		old_items[i]         = temp;
	}
	Dee_instance_desc_lock_endwrite(instance);

	/* Decref all the old items. */
	Dee_XDecrefv(old_items, size);
	Dee_Freea(old_items);
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_builtin_tmoveassign(DeeTypeObject *tp_self,
                             DeeObject *self,
                             DeeObject *other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	struct instance_desc *other_instance;
	DeeTypeObject *tp_super;
	uint16_t i, size;
	DREF DeeObject **old_items;
	if unlikely(self == other)
		goto done;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	tp_super = DeeType_Base(tp_self);
	if (tp_super && DeeObject_TMoveAssign(tp_super, self, other))
		goto err;
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	old_items      = (DREF DeeObject **)Dee_Mallocac(size, sizeof(DREF DeeObject *));
	if unlikely(!old_items)
		goto err;

	/* Load member values from `others', while also unbinding all members. */
	Dee_instance_desc_lock_read(other_instance);
	memcpyc(old_items, other_instance->id_vtab,
	        size, sizeof(DREF DeeObject *));
	bzeroc(other_instance->id_vtab,
	       size,
	       sizeof(DREF DeeObject *));
	Dee_instance_desc_lock_endread(other_instance);

	/* Exchange our own member values with those loaded from `other' */
	Dee_instance_desc_lock_write(instance);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *temp;
		temp                 = instance->id_vtab[i];
		instance->id_vtab[i] = old_items[i];
		old_items[i]         = temp;
	}
	Dee_instance_desc_lock_endwrite(instance);

	/* Decref all the old items. */
	Dee_XDecrefv(old_items, size);
	Dee_Freea(old_items);
done:
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_assign(DeeObject *self, DeeObject *other) {
	return instance_builtin_tassign(Dee_TYPE(self), self, other);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_moveassign(DeeObject *self, DeeObject *other) {
	return instance_builtin_tmoveassign(Dee_TYPE(self), self, other);
}



LOCAL WUNUSED NONNULL((1, 2)) int DCALL
instance_initsuper_as_ctor(DeeTypeObject *__restrict tp_super,
                           DeeObject *__restrict self) {
	int result;

	/* Handle constructor inheritance */
	while (tp_super->tp_flags & TP_FINHERITCTOR) {
		ASSERTF(!(tp_super->tp_flags & TP_FFINAL),
		        "Type derived from final type");
		ASSERT(DeeType_Base(tp_super));
		tp_super = DeeType_Base(tp_super);
	}
	ASSERTF(!(tp_super->tp_flags & TP_FVARIABLE), "Type derived from variable type");

	/* Initialize the super-type. */
	if (tp_super->tp_init.tp_alloc.tp_ctor) {
		int (DCALL *func)(DeeObject *__restrict);
		func   = tp_super->tp_init.tp_alloc.tp_ctor;
		result = DeeType_INVOKE_CTOR(func, tp_super, self);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor) {
		int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor;
		result = DeeType_INVOKE_ANY_CTOR(func, tp_super, self, 0, NULL);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor_kw) {
		int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *, DeeObject *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor_kw;
		result = DeeType_INVOKE_ANY_CTOR_KW(func, tp_super, self, 0, NULL, NULL);
	} else {
		result = err_unimplemented_constructor(tp_super, 0, NULL);
	}
	return result;
}

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
instance_initsuper_as_init(DeeTypeObject *tp_super,
                           DeeObject *self, size_t argc,
                           DeeObject *const *argv) {
	int result;

	/* Handle constructor inheritance */
	while (tp_super->tp_flags & TP_FINHERITCTOR) {
		ASSERTF(!(tp_super->tp_flags & TP_FFINAL),
		        "Type derived from final type");
		ASSERT(DeeType_Base(tp_super));
		tp_super = DeeType_Base(tp_super);
	}
	ASSERTF(!(tp_super->tp_flags & TP_FVARIABLE), "Type derived from variable type");

	/* Initialize the super-type. */
	if (tp_super->tp_init.tp_alloc.tp_ctor && !argc) {
		int (DCALL *func)(DeeObject *__restrict);
		func   = tp_super->tp_init.tp_alloc.tp_ctor;
		result = DeeType_INVOKE_CTOR(func, tp_super, self);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor) {
		int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor;
		result = DeeType_INVOKE_ANY_CTOR(func, tp_super, self, argc, argv);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor_kw) {
		int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *, DeeObject *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor_kw;
		result = DeeType_INVOKE_ANY_CTOR_KW(func, tp_super, self, argc, argv, NULL);
	} else {
		result = err_unimplemented_constructor(tp_super, argc, argv);
	}
	return result;
}

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
instance_initsuper_as_initkw(DeeTypeObject *tp_super,
                             DeeObject *self, size_t argc,
                             DeeObject *const *argv, DeeObject *kw) {
	int result;

	/* Handle constructor inheritance */
	while (tp_super->tp_flags & TP_FINHERITCTOR) {
		ASSERTF(!(tp_super->tp_flags & TP_FFINAL),
		        "Type derived from final type");
		ASSERT(DeeType_Base(tp_super));
		tp_super = DeeType_Base(tp_super);
	}
	ASSERTF(!(tp_super->tp_flags & TP_FVARIABLE), "Type derived from variable type");

	/* Initialize the super-type. */
	if (tp_super->tp_init.tp_alloc.tp_any_ctor_kw) {
		int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *, DeeObject *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor_kw;
		result = DeeType_INVOKE_ANY_CTOR_KW(func, tp_super, self, argc, argv, kw);
	} else {
		if (kw) {
			if (DeeKwds_Check(kw)) {
				if (DeeKwds_SIZE(kw) != 0)
					goto err_no_keywords;
			} else {
				size_t kw_size = DeeObject_Size(kw);
				if unlikely(kw_size == (size_t)-1)
					goto err;
				if (kw_size != 0)
					goto err_no_keywords;
			}
		}
		if (tp_super->tp_init.tp_alloc.tp_any_ctor) {
			int (DCALL *func)(DeeObject *__restrict, size_t, DeeObject *const *);
			func   = tp_super->tp_init.tp_alloc.tp_any_ctor;
			result = DeeType_INVOKE_ANY_CTOR(func, tp_super, self, argc, argv);
		} else if (tp_super->tp_init.tp_alloc.tp_ctor && !argc) {
			int (DCALL *func)(DeeObject *__restrict);
			func   = tp_super->tp_init.tp_alloc.tp_ctor;
			result = DeeType_INVOKE_CTOR(func, tp_super, self);
		} else {
			result = err_unimplemented_constructor(tp_super, argc, argv);
		}
	}
	return result;
err_no_keywords:
	err_keywords_ctor_not_accepted(tp_super, kw);
err:
	return -1;
}

/* User-defined constructor invocation. */
/* `OPERATOR_CONSTRUCTOR' + `CLASS_OPERATOR_SUPERARGS' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_super_tctor(DeeTypeObject *tp_self,
                     DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args, *result;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_Call(func, 0, NULL);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args_only;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err_args_only;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_init(tp_super, self,
		                               DeeTuple_SIZE(args),
		                               DeeTuple_ELEM(args)))
			goto err_members;
	} else if (DeeTuple_SIZE(args) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor(&DeeObject_Type,
		                              DeeTuple_SIZE(args),
		                              DeeTuple_ELEM(args));
		goto err_args;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(args);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(args);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
	Dee_Decref(func);
err:
	return -1;
err_args_only:
	Dee_Decref(args);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_kwsuper_tctor(DeeTypeObject *tp_self,
                       DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args, *result;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_Call(func, 0, NULL);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args_only;
	if (DeeTuple_SIZE(args) != 2)
		goto err_invalid_init_size;
	if (DeeObject_AssertTypeExact(DeeTuple_GET(args, 0), &DeeTuple_Type))
		goto err_args_only;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err_args_only;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_initkw(tp_super, self,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1)))
			goto err_members;
	} else if (DeeTuple_SIZE(DeeTuple_GET(args, 0)) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor_kw(&DeeObject_Type,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1));
		goto err_args;
	} else {
		if (check_empty_keywords_obj(DeeTuple_GET(args, 1)))
			goto err_args;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(args);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(args);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
	Dee_Decref(func);
err:
	return -1;
err_invalid_init_size:
	err_invalid_unpack_size(args, 2, DeeTuple_SIZE(args));
err_args_only:
	Dee_Decref(args);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_super_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                     size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args, *result;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_Call(func, argc, argv);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args_only;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err_args_only;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_init(tp_super, self,
		                               DeeTuple_SIZE(args),
		                               DeeTuple_ELEM(args)))
			goto err_members;
	} else if (DeeTuple_SIZE(args) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor(&DeeObject_Type,
		                              DeeTuple_SIZE(args),
		                              DeeTuple_ELEM(args));
		goto err_args;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, argc, argv);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(args);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(args);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
	Dee_Decref(func);
err:
	return -1;
err_args_only:
	Dee_Decref(args);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_kwsuper_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                       size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args, *result;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_Call(func, argc, argv);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args_only;
	if (DeeTuple_SIZE(args) != 2)
		goto err_invalid_init_size;
	if (DeeObject_AssertTypeExact(DeeTuple_GET(args, 0), &DeeTuple_Type))
		goto err_args_only;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err_args_only;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_initkw(tp_super, self,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1)))
			goto err_members;
	} else if (DeeTuple_SIZE(DeeTuple_GET(args, 0)) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor_kw(&DeeObject_Type,
		                                 DeeTuple_SIZE(args),
		                                 DeeTuple_ELEM(args),
		                                 DeeTuple_GET(args, 1));
		goto err_args;
	} else {
		if (check_empty_keywords_obj(DeeTuple_GET(args, 1)))
			goto err_args;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, argc, argv);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(args);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(args);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
	Dee_Decref(func);
err:
	return -1;
err_invalid_init_size:
	err_invalid_unpack_size(args, 2, DeeTuple_SIZE(args));
err_args_only:
	Dee_Decref(args);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_super_tinitkw(DeeTypeObject *tp_self,
                       DeeObject *__restrict self, size_t argc,
                       DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args, *result;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_CallKwInherited(func, argc, argv, kw);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args_only;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err_args_only;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_init(tp_super, self,
		                               DeeTuple_SIZE(args),
		                               DeeTuple_ELEM(args)))
			goto err_members;
	} else if (DeeTuple_SIZE(args) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor(&DeeObject_Type,
		                              DeeTuple_SIZE(args),
		                              DeeTuple_ELEM(args));
		goto err_args;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(args);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(args);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
	Dee_Decref(func);
err:
	return -1;
err_args_only:
	Dee_Decref(args);
	goto err;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_kwsuper_tinitkw(DeeTypeObject *tp_self,
                         DeeObject *__restrict self, size_t argc,
                         DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args, *result;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_CallKwInherited(func, argc, argv, kw);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args_only;
	if (DeeTuple_SIZE(args) != 2)
		goto err_invalid_init_size;
	if (DeeObject_AssertTypeExact(DeeTuple_GET(args, 0), &DeeTuple_Type))
		goto err_args_only;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err_args_only;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_initkw(tp_super, self,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1)))
			goto err_members;
	} else if (DeeTuple_SIZE(DeeTuple_GET(args, 0)) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor_kw(&DeeObject_Type,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1));
		goto err_args;
	} else {
		if (check_empty_keywords_obj(DeeTuple_GET(args, 1)))
			goto err_args;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(args);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(args);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
	Dee_Decref(func);
err:
	return -1;
err_invalid_init_size:
	err_invalid_unpack_size(args, 2, DeeTuple_SIZE(args));
err_args_only:
	Dee_Decref(args);
	goto err;
}

/* `CLASS_OPERATOR_SUPERARGS' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_super_tctor(DeeTypeObject *tp_self,
                             DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_Call(func, 0, NULL);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_init(tp_super, self,
		                               DeeTuple_SIZE(args),
		                               DeeTuple_ELEM(args)))
			goto err_members;
	} else if (DeeTuple_SIZE(args) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor(&DeeObject_Type,
		                              DeeTuple_SIZE(args),
		                              DeeTuple_ELEM(args));
		goto err_args;
	}
	Dee_Decref(args);
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_kwsuper_tctor(DeeTypeObject *tp_self,
                               DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_Call(func, 0, NULL);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args;
	if (DeeTuple_SIZE(args) != 2)
		goto err_invalid_init_size;
	if (DeeObject_AssertTypeExact(DeeTuple_GET(args, 0), &DeeTuple_Type))
		goto err_args;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_initkw(tp_super, self,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1)))
			goto err_members;
	} else if (DeeTuple_SIZE(DeeTuple_GET(args, 0)) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor_kw(&DeeObject_Type,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1));
		goto err_args;
	} else {
		if (check_empty_keywords_obj(DeeTuple_GET(args, 1)))
			goto err_args;
	}
	Dee_Decref(args);
	return 0;
err_invalid_init_size:
	err_invalid_unpack_size(args, 2, DeeTuple_SIZE(args));
	goto err_args;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_super_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                             size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_Call(func, argc, argv);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_init(tp_super, self,
		                               DeeTuple_SIZE(args),
		                               DeeTuple_ELEM(args)))
			goto err_members;
	} else if (DeeTuple_SIZE(args) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor(&DeeObject_Type,
		                              DeeTuple_SIZE(args),
		                              DeeTuple_ELEM(args));
		goto err_args;
	}
	Dee_Decref(args);
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_kwsuper_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                               size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_Call(func, argc, argv);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args;
	if (DeeTuple_SIZE(args) != 2)
		goto err_invalid_init_size;
	if (DeeObject_AssertTypeExact(DeeTuple_GET(args, 0), &DeeTuple_Type))
		goto err_args;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_initkw(tp_super, self,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1)))
			goto err_members;
	} else if (DeeTuple_SIZE(DeeTuple_GET(args, 0)) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor_kw(&DeeObject_Type,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1));
		goto err_args;
	} else {
		if (check_empty_keywords_obj(DeeTuple_GET(args, 1)))
			goto err_args;
	}
	Dee_Decref(args);
	return 0;
err_invalid_init_size:
	err_invalid_unpack_size(args, 2, DeeTuple_SIZE(args));
	goto err_args;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_super_tinitkw(DeeTypeObject *tp_self,
                               DeeObject *__restrict self, size_t argc,
                               DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_CallKwInherited(func, argc, argv, kw);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_init(tp_super, self,
		                               DeeTuple_SIZE(args),
		                               DeeTuple_ELEM(args)))
			goto err_members;
	} else if (DeeTuple_SIZE(args) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor(&DeeObject_Type,
		                              DeeTuple_SIZE(args),
		                              DeeTuple_ELEM(args));
		goto err_args;
	}
	Dee_Decref(args);
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_kwsuper_tinitkw(DeeTypeObject *tp_self,
                                 DeeObject *__restrict self, size_t argc,
                                 DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args;
	DeeTypeObject *tp_super;

	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_CallKwInherited(func, argc, argv, kw);
	if unlikely(!args)
		goto err;

	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args;
	if (DeeTuple_SIZE(args) != 2)
		goto err_invalid_init_size;
	if (DeeObject_AssertTypeExact(DeeTuple_GET(args, 0), &DeeTuple_Type))
		goto err_args;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_initkw(tp_super, self,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1)))
			goto err_members;
	} else if (DeeTuple_SIZE(DeeTuple_GET(args, 0)) != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor_kw(&DeeObject_Type,
		                                 DeeTuple_SIZE(DeeTuple_GET(args, 0)),
		                                 DeeTuple_ELEM(DeeTuple_GET(args, 0)),
		                                 DeeTuple_GET(args, 1));
		goto err_args;
	} else {
		if (check_empty_keywords_obj(DeeTuple_GET(args, 1)))
			goto err_args;
	}
	Dee_Decref(args);
	return 0;
err_invalid_init_size:
	err_invalid_unpack_size(args, 2, DeeTuple_SIZE(args));
	goto err_args;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err_args:
	Dee_Decref(args);
err:
	return -1;
}

/* `OPERATOR_CONSTRUCTOR' */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_tctor(DeeTypeObject *tp_self,
               DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
               size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, argc, argv);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_tinitkw(DeeTypeObject *tp_self,
                 DeeObject *__restrict self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* `OPERATOR_CONSTRUCTOR' (but the type doesn't have a base) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_nobase_tctor(DeeTypeObject *tp_self,
                      DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	Dee_Decref(func);
	if (!DeeObject_UndoConstructionNoBase(self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return 0;
	}
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_nobase_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                      size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, argc, argv);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	Dee_Decref(func);
	if (!DeeObject_UndoConstructionNoBase(self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return 0;
	}
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_nobase_tinitkw(DeeTypeObject *tp_self,
                        DeeObject *__restrict self, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	Dee_Decref(func);
	if (!DeeObject_UndoConstructionNoBase(self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return 0;
	}
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err:
	return -1;
}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* `OPERATOR_CONSTRUCTOR', with the `TP_FINHERITCTOR' flag set.
 * NOTE: These functions always invoke the user-defined constructor without any arguments! */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_inherited_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                         size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_init(tp_super, self, argc, argv))
			goto err_members;
	}

	/* Invoke the user-defined class constructor. (without any arguments) */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_inherited_tinitkw(DeeTypeObject *tp_self,
                           DeeObject *__restrict self, size_t argc,
                           DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_initkw(tp_super, self, argc, argv, kw))
			goto err_members;
	}

	/* Invoke the user-defined class constructor. (without any arguments) */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

/* No predefined construction operators. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_tctor(DeeTypeObject *tp_self,
                       DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                       size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	if unlikely(argc != 0) {
		err_unimplemented_constructor(tp_self, argc, argv);
		goto err;
	}

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_tinitkw(DeeTypeObject *tp_self,
                         DeeObject *__restrict self, size_t argc,
                         DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	if unlikely(argc != 0) {
		err_unimplemented_constructor(tp_self, argc, argv);
		goto err;
	}
	if unlikely(kw && !DeeKwds_Check(kw)) {
		size_t keyword_count;
		keyword_count = DeeObject_Size(kw);
		if unlikely(keyword_count == (size_t)-1)
			goto err;
		if unlikely(keyword_count != 0) {
			err_keywords_ctor_not_accepted(tp_self, kw);
			goto err;
		}
	}

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err:
	return -1;
}

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* No predefined construction operators. (but the type doesn't have a base) */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_nobase_tctor(DeeTypeObject *tp_self,
                              DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_nobase_tinit(DeeTypeObject *tp_self,
                              DeeObject *__restrict self,
                              size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	if unlikely(argc != 0)
		return err_unimplemented_constructor(tp_self, argc, argv);

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_nobase_tinitkw(DeeTypeObject *tp_self,
                                DeeObject *__restrict self, size_t argc,
                                DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	if unlikely(argc != 0) {
		err_unimplemented_constructor(tp_self, argc, argv);
		goto err;
	}
	if unlikely(kw && !DeeKwds_Check(kw)) {
		size_t keyword_count;
		keyword_count = DeeObject_Size(kw);
		if unlikely(keyword_count == (size_t)-1)
			goto err;
		if unlikely(keyword_count != 0) {
			err_keywords_ctor_not_accepted(tp_self, kw);
			goto err;
		}
	}

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));
	return 0;
err:
	return -1;
}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* No predefined construction operators, but `TP_FINHERITCTOR' is set. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_inherited_tctor(DeeTypeObject *tp_self,
                                 DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_inherited_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                                 size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_init(tp_super, self, argc, argv))
			goto err_members;
	} else if (argc != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor(&DeeObject_Type, argc, argv);
		goto err;
	}
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_inherited_tinitkw(DeeTypeObject *tp_self,
                                   DeeObject *__restrict self, size_t argc,
                                   DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_initkw(tp_super, self, argc, argv, kw))
			goto err_members;
	} else if (argc != 0) {
		/* Without a custom base class, the constructor requires _no_ arguments! */
		err_unimplemented_constructor(&DeeObject_Type, argc, argv);
		goto err;
	} else if (kw && !DeeKwds_Check(kw)) {
		size_t keyword_count;
		keyword_count = DeeObject_Size(kw);
		if unlikely(keyword_count == (size_t)-1)
			goto err;
		if (keyword_count != 0) {
			err_keywords_ctor_not_accepted(&DeeObject_Type, kw);
			goto err;
		}
	}
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) int DCALL
instance_super_ctor(DeeObject *__restrict self) {
	return instance_super_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_kwsuper_ctor(DeeObject *__restrict self) {
	return instance_kwsuper_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_super_init(DeeObject *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	return instance_super_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_kwsuper_init(DeeObject *__restrict self,
                      size_t argc, DeeObject *const *argv) {
	return instance_kwsuper_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_super_initkw(DeeObject *__restrict self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	return instance_super_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_kwsuper_initkw(DeeObject *__restrict self, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	return instance_kwsuper_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_super_ctor(DeeObject *__restrict self) {
	return instance_builtin_super_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_kwsuper_ctor(DeeObject *__restrict self) {
	return instance_builtin_kwsuper_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_super_init(DeeObject *__restrict self,
                            size_t argc, DeeObject *const *argv) {
	return instance_builtin_super_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_kwsuper_init(DeeObject *__restrict self,
                              size_t argc, DeeObject *const *argv) {
	return instance_builtin_kwsuper_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_super_initkw(DeeObject *__restrict self, size_t argc,
                              DeeObject *const *argv, DeeObject *kw) {
	return instance_builtin_super_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_kwsuper_initkw(DeeObject *__restrict self, size_t argc,
                                DeeObject *const *argv, DeeObject *kw) {
	return instance_builtin_kwsuper_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_ctor(DeeObject *__restrict self) {
	return instance_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_init(DeeObject *__restrict self,
              size_t argc, DeeObject *const *argv) {
	return instance_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_initkw(DeeObject *__restrict self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	return instance_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN WUNUSED NONNULL((1)) int DCALL
instance_nobase_ctor(DeeObject *__restrict self) {
	return instance_nobase_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_nobase_init(DeeObject *__restrict self,
                     size_t argc, DeeObject *const *argv) {
	return instance_nobase_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_nobase_initkw(DeeObject *__restrict self, size_t argc,
                       DeeObject *const *argv, DeeObject *kw) {
	return instance_nobase_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

INTERN WUNUSED NONNULL((1)) int DCALL
instance_inherited_init(DeeObject *__restrict self,
                        size_t argc, DeeObject *const *argv) {
	return instance_inherited_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_inherited_initkw(DeeObject *__restrict self, size_t argc,
                          DeeObject *const *argv, DeeObject *kw) {
	return instance_inherited_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_ctor(DeeObject *__restrict self) {
	return instance_builtin_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_init(DeeObject *__restrict self,
                      size_t argc, DeeObject *const *argv) {
	return instance_builtin_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_initkw(DeeObject *__restrict self, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	return instance_builtin_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_nobase_ctor(DeeObject *__restrict self) {
	return instance_builtin_nobase_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_nobase_init(DeeObject *__restrict self,
                             size_t argc, DeeObject *const *argv) {
	return instance_builtin_nobase_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_nobase_initkw(DeeObject *__restrict self, size_t argc,
                               DeeObject *const *argv, DeeObject *kw) {
	return instance_builtin_nobase_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_inherited_ctor(DeeObject *__restrict self) {
	return instance_builtin_inherited_tctor(Dee_TYPE(self), self);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_inherited_init(DeeObject *__restrict self, size_t argc,
                                DeeObject *const *argv) {
	return instance_builtin_inherited_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_inherited_initkw(DeeObject *__restrict self, size_t argc,
                                  DeeObject *const *argv, DeeObject *kw) {
	return instance_builtin_inherited_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}



#ifdef CLASS_TP_FAUTOINIT
LOCAL WUNUSED NONNULL((1, 2)) struct class_attribute *DCALL
find_next_attribute(DeeClassDescriptorObject *__restrict self,
                    uint16_t *__restrict pnext_table_index) {
	size_t i;
	struct class_attribute *result = NULL, *entry;
	uint16_t next_table_index      = *pnext_table_index;
	uint16_t nearest_table_index   = (uint16_t)-1;
	if unlikely(next_table_index >= self->cd_imemb_size)
		goto done;
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		entry = &self->cd_iattr_list[i];
		if (entry->ca_addr < next_table_index)
			continue;
		if (entry->ca_addr >= nearest_table_index)
			continue;
		if (!CLASS_ATTRIBUTE_ALLOW_AUTOINIT(entry))
			continue;
		result              = entry;
		nearest_table_index = entry->ca_addr;
	}
	*pnext_table_index = nearest_table_index + 1;
done:
	return result;
}

LOCAL WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_autoload_members(DeeTypeObject *tp_self,
                          struct class_desc *__restrict desc,
                          struct instance_desc *__restrict instance,
                          size_t argc, DeeObject *const *argv) {
	size_t i;
	uint16_t next_table_index = 0;
	for (i = 0; i < argc; ++i) {
		struct class_attribute *at;
		at = find_next_attribute(desc->cd_desc, &next_table_index);
		if unlikely(!at)
			goto err_argc;
		if unlikely(DeeInstance_SetBasicAttribute(desc, instance, at, argv[i]))
		goto err;
	}
	return 0;
err_argc:
	err_invalid_argc(tp_self->tp_name, argc, 0, i);
err:
	return -1;
}

struct instance_autoload_foreach_data {
	DeeTypeObject        *ialf_tp_self;
	struct class_desc    *ialf_desc;
	struct instance_desc *ialf_instance;
	uint16_t              ialf_next_table_index;
};

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
instance_autoload_foreach_pair_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct class_attribute *at;
	struct instance_autoload_foreach_data *data;
	data = (struct instance_autoload_foreach_data *)arg;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	at = DeeClassDesc_QueryInstanceAttribute(data->ialf_desc, key);
	if unlikely(!at || !CLASS_ATTRIBUTE_ALLOW_AUTOINIT(at)) {
		err_unknown_attribute_string(data->ialf_tp_self,
		                             DeeString_STR(key),
		                             ATTR_ACCESS_SET);
		goto err;
	}
	if unlikely(at->ca_addr < data->ialf_next_table_index) {
		/* Member had already been initialized via a positional argument! */
		err_keywords_shadows_positional(DeeString_STR(key));
		goto err;
	}
	if unlikely(DeeInstance_SetBasicAttribute(data->ialf_desc,
	                                          data->ialf_instance,
	                                          at, value))
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
instance_autoload_members_kw(DeeTypeObject *tp_self,
                             struct class_desc *__restrict desc,
                             struct instance_desc *__restrict instance,
                             size_t argc, DeeObject *const *argv, DeeObject *kw) {
	uint16_t next_table_index;
	if (!kw)
		return instance_autoload_members(tp_self, desc, instance, argc, argv);
	next_table_index = 0;
	if (DeeKwds_Check(kw)) {
		DeeKwdsObject *kwds = (DeeKwdsObject *)kw;
		size_t i, positional_argc;
		DeeObject *const *kw_argv;
		if unlikely(DeeKwds_SIZE(kwds) > argc)
			return err_keywords_bad_for_argc(argc, DeeKwds_SIZE(kwds));
		positional_argc = argc - DeeKwds_SIZE(kwds);
		kw_argv         = argv + positional_argc;
		/* Load positional arguments into the first positional_argc instance members. */
		for (i = 0; i < positional_argc; ++i) {
			struct class_attribute *at;
			at = find_next_attribute(desc->cd_desc, &next_table_index);
			if unlikely(!at) {
				err_invalid_argc(tp_self->tp_name, argc, 0, i);
				goto err;
			}
			if unlikely(DeeInstance_SetBasicAttribute(desc, instance, at, argv[i]))
				goto err;
		}
		for (i = 0; i <= kwds->kw_mask; ++i) {
			struct class_attribute *at;
			if (!kwds->kw_map[i].ke_name)
				continue;
			at = DeeClassDesc_QueryInstanceAttributeHash(desc,
			                                             (DeeObject *)kwds->kw_map[i].ke_name,
			                                             kwds->kw_map[i].ke_hash);
			if unlikely(!at || !CLASS_ATTRIBUTE_ALLOW_AUTOINIT(at)) {
				err_unknown_attribute_string(tp_self,
				                             DeeString_STR(kwds->kw_map[i].ke_name),
				                             ATTR_ACCESS_SET);
				goto err;
			}
			if unlikely(at->ca_addr < next_table_index) {
				/* Member had already been initialized via a positional argument! */
				err_keywords_shadows_positional(DeeString_STR(kwds->kw_map[i].ke_name));
				goto err;
			}
			if unlikely(DeeInstance_SetBasicAttribute(desc, instance, at,
			                                          kw_argv[kwds->kw_map[i].ke_index]))
				goto err;
		}
	} else {
		size_t i;
		struct instance_autoload_foreach_data data;
		/* Load positional arguments into the first positional_argc instance members. */
		for (i = 0; i < argc; ++i) {
			struct class_attribute *at;
			at = find_next_attribute(desc->cd_desc, &next_table_index);
			if unlikely(!at) {
				err_invalid_argc(tp_self->tp_name, argc, 0, i);
				goto err;
			}
			if unlikely(DeeInstance_SetBasicAttribute(desc, instance, at, argv[i]))
				goto err;
		}
		data.ialf_tp_self          = tp_self;
		data.ialf_desc             = desc;
		data.ialf_instance         = instance;
		data.ialf_next_table_index = next_table_index;
		if (DeeObject_ForeachPair(kw, &instance_autoload_foreach_pair_cb, &data))
			goto err;
	}
	return 0;
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
instance_builtin_auto_tprintrepr(DeeTypeObject *tp_self,
                                 DeeObject *__restrict self,
                                 Dee_formatprinter_t printer, void *arg) {
#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0
	Dee_ssize_t temp, result;
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	uint16_t i, count = desc->cd_desc->cd_imemb_size;
	DREF DeeObject *ob;
	bool is_first = true;
	result = DeeFormat_Printf(printer, arg, "%s(", tp_self->tp_name);
	if unlikely(result < 0)
		goto done;
	Dee_instance_desc_lock_read(instance);
	for (i = 0; i < count; ++i) {
		size_t attr_index;
		ob = instance->id_vtab[i];
		if (!ob)
			continue;
		Dee_Incref(ob);
		Dee_instance_desc_lock_endread(instance);
		for (attr_index = 0;
		     attr_index <= desc->cd_desc->cd_iattr_mask; ++attr_index) {
			struct class_attribute *at;
			at = &desc->cd_desc->cd_iattr_list[attr_index];
			if (!at->ca_name)
				continue;
			if (at->ca_addr != i)
				continue;
			if (!CLASS_ATTRIBUTE_ALLOW_AUTOINIT(at))
				continue;
			if (!is_first)
				DO(err_ob, DeeFormat_PRINT(printer, arg, ", "));
			DO(err_ob, DeeString_PrintUtf8((DeeObject *)at->ca_name, printer, arg));
			DO(err_ob, DeeFormat_Printf(printer, arg, ": %r", ob));
			is_first = false;
			break;
		}
		Dee_Decref(ob);
		Dee_instance_desc_lock_read(instance);
	}
	Dee_instance_desc_lock_endread(instance);
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_ob:
	Dee_Decref(ob);
err:
	return temp;
#undef DO
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
instance_builtin_auto_printrepr(DeeObject *__restrict self,
                                Dee_formatprinter_t printer, void *arg) {
	return instance_builtin_auto_tprintrepr(Dee_TYPE(self), self, printer, arg);
}

/* No predefined construction operators (with `CLASS_TP_FAUTOINIT'). */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_auto_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);

	/* Auto-initialize members. */
	if unlikely(instance_autoload_members(tp_self, desc, instance, argc, argv))
		goto err_super;
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_auto_tinitkw(DeeTypeObject *tp_self,
                      DeeObject *__restrict self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc;
	struct instance_desc *instance;
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;
	desc     = DeeClass_DESC(tp_self);
	instance = DeeInstance_DESC(desc, self);

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_super;
	Dee_Decref(result);

	/* Auto-initialize members. */
	if unlikely(instance_autoload_members_kw(tp_self, desc, instance, argc, argv, kw))
		goto err_super;
	Dee_Decref(func);
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		Dee_Decref(func);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_auto_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                            size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}

	/* Auto-initialize members. */
	if unlikely(instance_autoload_members(tp_self, desc, instance, argc, argv))
		goto err_super;
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_auto_tinitkw(DeeTypeObject *tp_self,
                              DeeObject *__restrict self, size_t argc,
                              DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}

	/* Auto-initialize members. */
	if unlikely(instance_autoload_members_kw(tp_self, desc, instance, argc, argv, kw))
		goto err_super;
	return 0;
err_super:
	if (!DeeObject_UndoConstruction(tp_super, self)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return 0;
	}
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_auto_init(DeeObject *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	return instance_auto_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_auto_initkw(DeeObject *__restrict self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	return instance_auto_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_auto_init(DeeObject *__restrict self,
                           size_t argc, DeeObject *const *argv) {
	return instance_builtin_auto_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_auto_initkw(DeeObject *__restrict self, size_t argc,
                             DeeObject *const *argv, DeeObject *kw) {
	return instance_builtin_auto_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}


#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* No predefined construction operators (with `CLASS_TP_FAUTOINIT'). */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_auto_nobase_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                           size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_members;
	Dee_Decref(result);

	/* Auto-initialize members. */
	if unlikely(instance_autoload_members(tp_self, desc, instance, argc, argv))
		goto err_members;
	Dee_Decref(func);
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_auto_nobase_tinitkw(DeeTypeObject *tp_self,
                             DeeObject *__restrict self, size_t argc,
                             DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc;
	struct instance_desc *instance;
	DREF DeeObject *func, *result;
	desc     = DeeClass_DESC(tp_self);
	instance = DeeInstance_DESC(desc, self);

	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_members;
	Dee_Decref(result);

	/* Auto-initialize members. */
	if unlikely(instance_autoload_members_kw(tp_self, desc, instance, argc, argv, kw))
		goto err_members;
	Dee_Decref(func);
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_auto_nobase_tinit(DeeTypeObject *tp_self, DeeObject *__restrict self,
                                   size_t argc, DeeObject *const *argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Auto-initialize members. */
	if unlikely(instance_autoload_members(tp_self, desc, instance, argc, argv))
		goto err_members;
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
instance_builtin_auto_nobase_tinitkw(DeeTypeObject *tp_self,
                                     DeeObject *__restrict self, size_t argc,
                                     DeeObject *const *argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);

	/* Default-initialize the members of this instance. */
	Dee_atomic_rwlock_init(&instance->id_lock);
	bzeroc(instance->id_vtab,
	       desc->cd_desc->cd_imemb_size,
	       sizeof(DREF DeeObject *));

	/* Auto-initialize members. */
	if unlikely(instance_autoload_members_kw(tp_self, desc, instance, argc, argv, kw))
		goto err_members;
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
/*err:*/
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_auto_nobase_init(DeeObject *__restrict self,
                          size_t argc, DeeObject *const *argv) {
	return instance_auto_nobase_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_auto_nobase_initkw(DeeObject *__restrict self, size_t argc,
                            DeeObject *const *argv, DeeObject *kw) {
	return instance_auto_nobase_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_auto_nobase_init(DeeObject *__restrict self,
                                  size_t argc, DeeObject *const *argv) {
	return instance_builtin_auto_nobase_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN WUNUSED NONNULL((1)) int DCALL
instance_builtin_auto_nobase_initkw(DeeObject *__restrict self, size_t argc,
                                    DeeObject *const *argv, DeeObject *kw) {
	return instance_builtin_auto_nobase_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
#endif /* CLASS_TP_FAUTOINIT */








/* Builtin hash & comparison support. */
INTERN WUNUSED NONNULL((1, 2, 5)) size_t DCALL
instance_iterattr(DeeTypeObject *tp_self, DeeObject *__restrict self,
                  struct Dee_attriter *iterbuf, size_t bufsize,
                  struct Dee_attrhint const *__restrict hint) {
	/* Hook function for user-defined enumattr() callbacks!
	 * User-code should be allowed to yield either:
	 *  - deemon.Attribute   (gets forwarded as-is)
	 *  - deemon.string      (gets wrapped as `Attribute(tp_self, <value>)') */
	(void)tp_self;
	(void)self;
	(void)iterbuf;
	(void)bufsize;
	(void)hint;
	/* TODO */
	return (size_t)DeeError_NOTIMPLEMENTED();
}


/* GC support for class objects. */
INTERN NONNULL((1, 2, 3)) void DCALL
instance_tvisit(DeeTypeObject *tp_self,
                DeeObject *__restrict self,
                dvisit_t proc, void *arg) {
	struct class_desc *desc;
	struct instance_desc *instance;
	desc     = DeeClass_DESC(tp_self);
	instance = DeeInstance_DESC(desc, self);
	Dee_instance_desc_lock_read(instance);
	Dee_XVisitv(instance->id_vtab, desc->cd_desc->cd_imemb_size);
	Dee_instance_desc_lock_endread(instance);
}

INTERN NONNULL((1, 2)) void DCALL
instance_tclear(DeeTypeObject *tp_self,
                DeeObject *__restrict self) {
	struct class_desc *desc;
	uint16_t i;
	struct instance_desc *instance;
	DREF DeeObject *buffer[64];
	size_t buflen;
	desc     = DeeClass_DESC(tp_self);
	instance = DeeInstance_DESC(desc, self);
	buflen   = 0;
	Dee_instance_desc_lock_write(instance);
	for (i = 0; i < desc->cd_desc->cd_imemb_size; ++i) {
again_i:
		if (!instance->id_vtab[i])
			continue;
		if (Dee_DecrefIfNotOne(instance->id_vtab[i])) {
			/* Clear was possible without side-effects */
			instance->id_vtab[i] = NULL;
			continue;
		}
		if (buflen == COMPILER_LENOF(buffer)) {
			Dee_instance_desc_lock_endwrite(instance);
			Dee_Decrefv(buffer, buflen);
			buflen = 0;
			Dee_instance_desc_lock_write(instance);
			goto again_i;
		}
		buffer[buflen++]     = instance->id_vtab[i]; /* Steal reference. */
		instance->id_vtab[i] = NULL;
	}
	Dee_instance_desc_lock_endwrite(instance);
	Dee_Decrefv(buffer, buflen);
}

INTERN NONNULL((1, 2)) void DCALL
instance_tpclear(DeeTypeObject *tp_self,
                 DeeObject *__restrict self,
                 unsigned int gc_priority) {
	struct class_desc *desc;
	uint16_t i;
	struct instance_desc *instance;
	DREF DeeObject *buffer[64];
	size_t buflen;
	desc     = DeeClass_DESC(tp_self);
	instance = DeeInstance_DESC(desc, self);
	buflen   = 0;
	Dee_instance_desc_lock_write(instance);
	for (i = 0; i < desc->cd_desc->cd_imemb_size; ++i) {
again_i:
		if (!instance->id_vtab[i])
			continue; /* Unbound member slot. */
		if (DeeObject_GCPriority(instance->id_vtab[i]) < gc_priority)
			continue; /* Object isn't of interest. */
		if (Dee_DecrefIfNotOne(instance->id_vtab[i])) {
			/* Clear was possible without side-effects */
			instance->id_vtab[i] = NULL;
			continue;
		}
		if (buflen == COMPILER_LENOF(buffer)) {
			Dee_instance_desc_lock_endwrite(instance);
			Dee_Decrefv(buffer, buflen);
			buflen = 0;
			Dee_instance_desc_lock_write(instance);
			goto again_i;
		}
		buffer[buflen++]     = instance->id_vtab[i]; /* Steal this reference. */
		instance->id_vtab[i] = NULL;
	}
	Dee_instance_desc_lock_endwrite(instance);
	Dee_Decrefv(buffer, buflen);
}

INTERN NONNULL((1, 2)) void DCALL
instance_visit(DeeObject *__restrict self,
               dvisit_t proc, void *arg) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		instance_tvisit(tp_self, self, proc, arg);
	} while ((tp_self = DeeType_Base(tp_self)) != NULL &&
	         DeeType_IsClass(tp_self));
}

INTERN NONNULL((1)) void DCALL
instance_clear(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		instance_tclear(tp_self, self);
	} while ((tp_self = DeeType_Base(tp_self)) != NULL &&
	         DeeType_IsClass(tp_self));
}

INTERN NONNULL((1)) void DCALL
instance_pclear(DeeObject *__restrict self,
                unsigned int gc_priority) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		instance_tpclear(tp_self, self, gc_priority);
	} while ((tp_self = DeeType_Base(tp_self)) != NULL &&
	         DeeType_IsClass(tp_self));
}

INTERN struct type_gc tpconst instance_gc = {
	/* .tp_clear  = */ &instance_clear,
	/* .tp_pclear = */ &instance_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_INSTANCE
};


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
instance_generic_operator_invoke_cb(DeeTypeObject *tp_self, DeeObject *self,
                                    /*0..1*/ DREF DeeObject **p_self, size_t argc,
                                    DeeObject *const *argv, Dee_operator_t opname) {
	DREF DeeObject *result;
	result = DeeClass_CallOperator(tp_self, self, opname, argc, argv);
	if (p_self && result) {
		ASSERT(*p_self == self);
		Dee_Incref(result);
		Dee_Decref_unlikely(self);
		*p_self = result;
	}
	return result;
}




PRIVATE WUNUSED NONNULL((1)) size_t DCALL
get_operator_class_table_size(DeeTypeObject *__restrict type_type, uint16_t oi_class) {
	size_t result;
	ASSERTF(oi_class != 0, "Not for inline operators!");
#ifndef __OPTIMIZE_SIZE__
	/* Fast-pass for known operator classes */
	if (oi_class == offsetof(DeeTypeObject, tp_math))
		return sizeof(struct type_math);
	if (oi_class == offsetof(DeeTypeObject, tp_cmp))
		return sizeof(struct type_cmp);
	if (oi_class == offsetof(DeeTypeObject, tp_seq))
		return sizeof(struct type_seq);
	if (oi_class == offsetof(DeeTypeObject, tp_attr))
		return sizeof(struct type_attr);
	if (oi_class == offsetof(DeeTypeObject, tp_with))
		return sizeof(struct type_with);
#endif /* !__OPTIMIZE_SIZE__ */
	result = sizeof(Dee_funptr_t);
	while (type_type != &DeeType_Type) {
		size_t i;
		ASSERT(DeeType_IsTypeType(type_type));
		for (i = 0; i < type_type->tp_operators_size; ++i) {
			struct type_operator const *op = &type_type->tp_operators[i];
			if likely(type_operator_isdecl(op)) {
				if (op->to_decl.oi_class == oi_class) {
					size_t class_size = op->to_decl.oi_offset + sizeof(Dee_funptr_t);
					if (result < class_size)
						result = class_size;
				}
			}
		}
		type_type = DeeType_Base(type_type);
	}
	return result;
}

/* Bind the C-wrapper function(s) for `operator_name' in `class_type',
 * with `type_type' being responsible for providing said operator. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bind_class_operator(DeeTypeObject *__restrict type_type,
                    DeeTypeObject *__restrict class_type,
                    Dee_operator_t operator_name) {
	struct opinfo const *info;
	info = DeeTypeType_GetOperatorById(type_type, operator_name);
	if likely(info) {
		/* Dedicated operator with C-wrapper. */
		void *class_table;
		ASSERT(info->oi_invoke);
		if unlikely(!info->oi_invoke->opi_classhook) {
			/* Special case: operator cannot be overwritten by user-code. */
			DeeError_Throwf(&DeeError_TypeError,
			                "Operator `%s' of type-type %q cannot be implemented",
			                info->oi_uname, type_type->tp_name);
			goto err;
		}
		class_table = (void *)class_type;
		if (info->oi_class != 0) {
			void **p_class_table;
			p_class_table = (void **)((byte_t *)class_table + info->oi_class);
			class_table = *p_class_table;
			if (!class_table) {
				/* Must allocate class table. */
				size_t tabsize = get_operator_class_table_size(type_type, info->oi_class);
				class_table = Dee_Calloc(tabsize);
				if unlikely(!class_table)
					goto err;
				*p_class_table = class_table;
			}
		}

		/* Store the C-wrapper for the operator in its designated location. */
		*(void **)((byte_t *)class_table + info->oi_offset) = info->oi_invoke->opi_classhook;
	} else {
		/* Operator has nowhere to go natively
		 * -> add a Dee_TYPE_OPERATOR_CUSTOM-entry for it in `class_type->tp_operators',
		 *    using `instance_generic_operator_invoke_cb()'. */
		struct type_operator *new_table;
		size_t new_size;
		size_t lo, hi;

		/* Figure out where in the table "operator_name" needs to go. */
		lo = 0;
		hi = class_type->tp_operators_size;
		while (lo < hi) {
			size_t mid = (lo + hi) / 2;
			Dee_operator_t mid_name = class_type->tp_operators[mid].to_id;
			if (operator_name < mid_name) {
				hi = mid;
			} else if (operator_name > mid_name) {
				lo = mid + 1;
			} else {
				/* Operator had already been defined before? -> ignore */
				return 0;
			}
		}

		new_size  = class_type->tp_operators_size + 1;
		new_table = (struct type_operator *)Dee_Reallocc((void *)class_type->tp_operators,
		                                                 new_size, sizeof(struct type_operator));
		if unlikely(!new_table)
			goto err;
		class_type->tp_operators = new_table;
		ASSERT(lo == hi);
		memmoveupc(&new_table[lo + 1], &new_table[lo],
		           class_type->tp_operators_size - lo,
		           sizeof(struct type_operator));
		++class_type->tp_operators_size;

		/* Fill in the operator slot. */
		bzero(&new_table[lo], sizeof(struct type_operator));
		new_table[lo].to_custom._s_pad_id = operator_name;
		new_table[lo].to_custom._s_class  = OPCLASS_CUSTOM;
		/*new_table[lo].to_custom._s_offset = 0;*/
#if OPCC_SPECIAL != 0
		new_table[lo].to_custom._s_cc = OPCC_SPECIAL;
#endif /* OPCC_SPECIAL != 0 */
#if Dee_METHOD_FNORMAL != 0
		new_table[lo].to_custom.s_flags = Dee_METHOD_FNORMAL;
#endif /* Dee_METHOD_FNORMAL != 0 */
		new_table[lo].to_custom.s_invoke = &instance_generic_operator_invoke_cb;
	}
	return 0;
err:
	return -1;
}



struct class_bases {
	DREF DeeTypeObject  *cb_base; /* [0..1] Primary class base. */
	DREF DeeTypeObject **cb_mro;  /* [0..1][0..n][owned] Class MRO override (or NULL if not needed) */
};

/* Finalize the given class-bases descriptor. */
PRIVATE NONNULL((1)) void DCALL
class_bases_fini(struct class_bases *__restrict self) {
	if (self->cb_mro != NULL) {
		size_t i;
		for (i = 0; self->cb_mro[i] != NULL; ++i)
			Dee_Decref_unlikely(self->cb_mro[i]);
		Dee_Free(self->cb_mro);
	}
	Dee_XDecref_unlikely(self->cb_base);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_cannot_use_final_type_as_base(DeeTypeObject *__restrict base) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot use final, or variable type `%s' as class base",
	                       base->tp_name);
}

/* Return the first "relevant" base of `self' (or `self' itself if it is "relevant").
 * A type is relevant if (at least one):
 * - It isn't a user-defined class type
 * - It has a non-inherited constructor
 * - It has a destructor
 * - It has instance members */
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) DeeTypeObject *DCALL
DeeType_GetFirstRelevantBase(DeeTypeObject *__restrict self) {
	for (;;) {
		if (!DeeType_IsClass(self))
			break;
		if (self->tp_class->cd_desc->cd_imemb_size > 0)
			break;
		if (DeeClass_TryGetPrivateOperatorPtr(self, OPERATOR_CONSTRUCTOR))
			break;
		if (DeeClass_TryGetPrivateOperatorPtr(self, OPERATOR_DESTRUCTOR))
			break;
		self = DeeType_Base(self);
		ASSERT(self);
	}
	return self;
}

/* Load class bases from `bases'. */
PRIVATE NONNULL((1, 2)) int DCALL
class_bases_init(struct class_bases *__restrict self,
                 DeeObject *__restrict bases) {
	struct objectlist bases_list;
	if (DeeNone_Check(bases)) {
		/* Special case: no base */
no_base:
		self->cb_base = NULL;
		self->cb_mro  = NULL;
	} else if (DeeType_Check(bases)) {
		/* Single base. */
		self->cb_base = (DREF DeeTypeObject *)bases;
		Dee_Incref(bases);
		self->cb_mro = NULL;
	} else {
		/* MRO support: Types are appended to the MRO the first time they're
		 *              encountered, following a breath-first, left-to-right
		 *              search:
		 * >> local result = [bases...];
		 * >> local scanMe = copy result;
		 * >> while (scanMe) {
		 * >>     local newScanMe = [];
		 * >>     for (local base: scanMe.__bases__) {
		 * >>         for (local baseBase: base.__bases__) {
		 * >>             if (baseBase !in result) {
		 * >>                 result.append(baseBase);
		 * >>                 newScanMe.append(baseBase);
		 * >>             }
		 * >>         }
		 * >>     }
		 * >>     scanMe = newScanMe;
		 * >> } */
		size_t direct_base_count;
		size_t mro_i, mro_end;
		if (objectlist_initseq(&bases_list, bases))
			goto err;

		/* Check for special case: no bases */
		direct_base_count = bases_list.ol_elemc;
		if unlikely(direct_base_count == 0) {
			objectlist_fini(&bases_list);
			goto no_base;
		}

		/* Check for special case: single base */
		if unlikely(direct_base_count == 1) {
			self->cb_base = (DREF DeeTypeObject *)bases_list.ol_elemv[0];
			Dee_Free(bases_list.ol_elemv);
			if (DeeObject_AssertType((DeeObject *)self->cb_base, &DeeType_Type)) {
				Dee_Decref_unlikely(self->cb_base);
				goto err;
			}
			self->cb_mro = NULL;
			goto done;
		}

		/* Determine the primary base. */
		self->cb_base = NULL;
		for (mro_i = 0; mro_i < direct_base_count; ++mro_i) {
			DeeTypeObject *base = (DeeTypeObject *)bases_list.ol_elemv[mro_i];
			if (DeeObject_AssertType((DeeObject *)base, &DeeType_Type))
				goto err_bases_list;
			if (base->tp_flags & (TP_FFINAL | TP_FVARIABLE)) {
				err_cannot_use_final_type_as_base(base);
				goto err_bases_list;
			}
			if (!DeeType_IsAbstract(base)) {
				/* Non-abstract types must appear as base, but
				 * there can only be 1 such base per sub-class. */
				if (self->cb_base != NULL) {
					DeeTypeObject *old_relevant_base;
					DeeTypeObject *new_relevant_base;
					/* Special case: only check the relationship of "relevant" bases.
					 *
					 * >> class List1: List { function a() { print "a"; } };
					 * >> class List2: List { function b() { print "b"; } };
					 * >> class List3: List1, List2 {}; // This should be allowed
					 */
					old_relevant_base = DeeType_GetFirstRelevantBase(self->cb_base);
					new_relevant_base = DeeType_GetFirstRelevantBase(base);

					/* if "old_relevant_base.extends(new_relevant_base)"
					 * -> ignore ("base" is already present in MRO) */
					if (DeeType_Extends(old_relevant_base, new_relevant_base))
						continue;

					/* if "new_relevant_base.extends(old_relevant_base)"
					 * -> Continue working with "self->cb_base = base" */
					if (DeeType_Extends(new_relevant_base, old_relevant_base))
						goto use_this_base_as_primary_base;

					DeeError_Throwf(&DeeError_TypeError,
					                "Cannot construct type with at least 2 non-abstract bases %r and %r",
					                self->cb_base, base);
					goto err_bases_list;
				}
use_this_base_as_primary_base:
				self->cb_base = base;
			}
		}

		/* Fallback: just use the first abstract base as primary base. */
		if (self->cb_base == NULL)
			self->cb_base = (DeeTypeObject *)bases_list.ol_elemv[0];

		/* Recursively scan for more bases. */
		mro_i   = 0;
		mro_end = direct_base_count;
		for (;;) {
			for (; mro_i < mro_end; ++mro_i) {
				/* Enumerate the direct bases of this base, and append all that
				 * we didn't already come across to the MRO vector that we are
				 * currently constructing. */
				DeeTypeObject *base = (DeeTypeObject *)bases_list.ol_elemv[mro_i];
				DeeTypeMRO base_mro;
				base = DeeTypeMRO_Init(&base_mro, base);
				do {
					if (!objectlist_contains_byid(&bases_list, (DeeObject *)base)) {
						if (Dee_objectlist_append(&bases_list, (DeeObject *)base))
							goto err_bases_list;
					}
				} while ((base = DeeTypeMRO_NextDirectBase(&base_mro, base)) != NULL);
			}
			ASSERT(mro_i == mro_end);
			if (mro_end == bases_list.ol_elemc)
				break;
			mro_end = bases_list.ol_elemc;
		}

		/* Finalize the MRO vector. */
		if (objectlist_setallocated(&bases_list, bases_list.ol_elemc + 1))
			goto err_bases_list;
		bases_list.ol_elemv[bases_list.ol_elemc] = NULL;
		self->cb_mro = (DREF DeeTypeObject **)bases_list.ol_elemv; /* Inherit vector + references */

		/* Create a reference for the primary base. */
		Dee_Incref(self->cb_base);
	}

done:
	return 0;
err_bases_list:
	objectlist_fini(&bases_list);
err:
	return -1;
}


/* Create a new class type derived from `bases',
 * featuring traits from `descriptor'.
 * @param: bases: The base(s) of the resulting class.
 *                You may pass `Dee_None' to have the resulting
 *                class not be derived from anything (be base-less).
 *                You may also pass a sequence of types, in which
 *                case this sequence (and its order) describe the
 *                class's top-level MRO (thus becoming its __bases__).
 * @param: descriptor: A `DeeClassDescriptor_Type'-object, detailing the class's prototype.
 * @param: declaring_module: When non-NULL, the module that gets stored in `tp_module'
 *                           NOTE: Passing NULL here must be allowed for situations where
 *                                 code is executing without having a module-context (as
 *                                 is the case for code running in a JIT-context)
 * @throw: TypeError: The given `base' is neither `none', nor a type-object.
 * @throw: TypeError: The given `base' is a final or variable type. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeTypeObject *DCALL
DeeClass_New(DeeObject *bases, DeeObject *descriptor,
             struct Dee_module_object *declaring_module) {
	struct class_bases cbases;
	DeeClassDescriptorObject *desc;
	DREF DeeTypeObject *result;
	DeeTypeObject *result_type_type;
	struct class_desc *result_class;
	size_t result_class_offset;
	ASSERT_OBJECT_TYPE_EXACT(descriptor, &DeeClassDescriptor_Type);
	ASSERT_OBJECT_TYPE_OPT(declaring_module, &DeeModule_Type);
	desc = (DeeClassDescriptorObject *)descriptor;

	/* Load class bases. */
	if unlikely(class_bases_init(&cbases, bases))
		goto err;
	if (cbases.cb_base == NULL) {
		result_type_type = &DeeType_Type; /* No base class. */
	} else {
		/* Make sure that the given base-object is actually a type. */
		result_type_type = Dee_TYPE(cbases.cb_base);
		ASSERTF(DeeType_IsTypeType(result_type_type),
		        "The type of type object '%r' isn't actually a type-type",
		        cbases.cb_base);
		ASSERTF(!(result_type_type->tp_flags & TP_FVARIABLE),
		        "type-type objects must not have the variable-size flag, but %s has it set!",
		        result_type_type->tp_name);
		if (cbases.cb_base->tp_flags & (TP_FFINAL | TP_FVARIABLE)) {
			err_cannot_use_final_type_as_base(cbases.cb_base);
			goto err_cbases;
		}
	}
	result_class_offset = result_type_type->tp_init.tp_alloc.tp_instance_size;
	if (result_type_type->tp_init.tp_alloc.tp_free) {
err_custom_allocator:
		DeeError_Throwf(&DeeError_TypeError,
		                "Cannot use `%s' with custom allocator as class base",
		                cbases.cb_base->tp_name);
		goto err_cbases;
	}
	result_class_offset += (sizeof(void *) - 1);
	result_class_offset &= ~(sizeof(void *) - 1);

	/* Allocate the resulting class object. */
	result = (DREF DeeTypeObject *)DeeGCObject_Callocc(result_class_offset +
	                                                   offsetof(struct class_desc, cd_members),
	                                                   desc->cd_cmemb_size, sizeof(DREF DeeObject *));
	if unlikely(!result)
		goto err_cbases;

	/* Figure out where the class descriptor starts. */
	result_class     = (struct class_desc *)((uintptr_t)result + result_class_offset);
	result->tp_class = result_class;

	/* Setup flags for the resulting type. */
	result->tp_flags = ((TP_FHEAP | TP_FGC) |
	                    (desc->cd_flags & (TP_FFINAL | TP_FTRUNCATE |
	                                       TP_FINTERRUPT | TP_FMOVEANY)));
	result->tp_mro = cbases.cb_mro; /* Inherit reference */
	if (cbases.cb_base == NULL) {
		/*result->tp_base = NULL;*/
		result_class->cd_offset = sizeof(DeeObject);
	} else {
		/* Calculate the offset of instance descriptors. */
		result_class->cd_offset = cbases.cb_base->tp_init.tp_alloc.tp_instance_size;
		if (cbases.cb_base->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_NO_OBJECT_SLABS
			void (DCALL *tp_free)(void *__restrict ob);
			size_t base_size;
			tp_free = cbases.cb_base->tp_init.tp_alloc.tp_free;
			/* Figure out the slab size used by the base-class. */
			if (cbases.cb_base->tp_flags & TP_FGC) {
#define CHECK_ALLOCATOR(index, size)                          \
				if (tp_free == &DeeGCObject_SlabFree##size) { \
					base_size = size * sizeof(void *);        \
				} else
				DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
				{
					DeeGCObject_Free(result);
					goto err_custom_allocator;
				}
			} else {
#define CHECK_ALLOCATOR(index, size)                        \
				if (tp_free == &DeeObject_SlabFree##size) { \
					base_size = size * sizeof(void *);      \
				} else
				DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
				{
					DeeGCObject_Free(result);
					goto err_custom_allocator;
				}
			}
			result_class->cd_offset = base_size;
#else /* !CONFIG_NO_OBJECT_SLABS */
			DeeGCObject_Free(result);
			goto err_custom_allocator;
#endif /* CONFIG_NO_OBJECT_SLABS */
		}
		result_class->cd_offset += (sizeof(void *) - 1);
		result_class->cd_offset &= ~(sizeof(void *) - 1);
		result->tp_base = cbases.cb_base; /* Inherit reference */
		result->tp_flags |= cbases.cb_base->tp_flags & TP_FINTERHITABLE;
		result->tp_weakrefs = cbases.cb_base->tp_weakrefs;
	}
	result_class->cd_desc = desc;
	Dee_Incref(desc);
	Dee_atomic_rwlock_cinit(&result_class->cd_lock);

	if likely(desc->cd_name) {
		result->tp_name = DeeString_STR(desc->cd_name);
		result->tp_flags |= TP_FNAMEOBJECT;
		Dee_Incref(desc->cd_name);
	}
	if (desc->cd_doc) {
		result->tp_doc = DeeString_STR(desc->cd_doc);
		result->tp_flags |= TP_FDOCOBJECT;
		Dee_Incref(desc->cd_doc);
	}

	/* Determine the memory data size of instances for this class. */
	result->tp_init.tp_alloc.tp_instance_size = result_class->cd_offset;                         /* Memory used by base-classes. */
	result->tp_init.tp_alloc.tp_instance_size += offsetof(struct instance_desc, id_vtab);        /* Instance descriptor header. */
	result->tp_init.tp_alloc.tp_instance_size += desc->cd_imemb_size * sizeof(DREF DeeObject *); /* Instance member objects. */

	/* When the type doesn't have any instance members, it's an abstract type... */
	if (desc->cd_imemb_size == 0) {
		/* ... but only if the underlying type is abstract */
		if ((cbases.cb_base == NULL) ||
#if 0 /* `DeeObject_Type' has TP_FABSTRACT set, so no extra check needed */
		    (cbases.cb_base == &DeeObject_Type) ||
#endif
		    (cbases.cb_base->tp_flags & TP_FABSTRACT)) {
			result->tp_flags |= TP_FABSTRACT;
		}

		/* Try to re-use the instance descriptor of the base class. */
		if (cbases.cb_base && DeeType_IsClass(cbases.cb_base)) {
			result_class->cd_offset                   = cbases.cb_base->tp_class->cd_offset;
			result->tp_init.tp_alloc.tp_instance_size = cbases.cb_base->tp_init.tp_alloc.tp_instance_size;
		}
	}


	/* Assign default / mandatory operators. */
	result->tp_init.tp_alloc.tp_copy_ctor = &instance_builtin_copy;
	result->tp_init.tp_assign             = &instance_builtin_assign;
	result->tp_init.tp_move_assign        = &instance_builtin_moveassign;
	result->tp_init.tp_deepload           = &instance_builtin_deepload;
	result->tp_init.tp_dtor               = &instance_builtin_destructor;
	result->tp_visit                      = &instance_visit;
	result->tp_gc                         = &instance_gc;
#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
	if (cbases.cb_base == NULL || cbases.cb_base == &DeeObject_Type) {
		result->tp_init.tp_alloc.tp_copy_ctor = &instance_builtin_nobase_copy;
		result->tp_init.tp_deepload           = &instance_builtin_nobase_deepload;
	}
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */

	{
#define FEATURE_CONSTRUCTOR 0x0001 /* A constructor is provided */
#define FEATURE_SUPERARGS   0x0002 /* A super-arguments generator is provided */
		uint16_t constructor_features = 0;
		uint16_t i = 0;
		do {
			struct class_operator *op;
			op = &desc->cd_clsop_list[i];
			if (op->co_name == (Dee_operator_t)-1)
				continue;
			switch (op->co_name) {

			case OPERATOR_CONSTRUCTOR:
				constructor_features |= FEATURE_CONSTRUCTOR;
				break;

				/* Defining either assign, or move_assign will get rid
				 * of automatically generated operators for the other. */
			case OPERATOR_ASSIGN:
				result->tp_init.tp_assign = &usrtype__assign__with__ASSIGN;
				if (result->tp_init.tp_move_assign == &instance_builtin_moveassign)
					result->tp_init.tp_move_assign = NULL;
				break;

			case OPERATOR_MOVEASSIGN:
				result->tp_init.tp_move_assign = &usrtype__move_assign__with__MOVEASSIGN;
				if (result->tp_init.tp_assign == &instance_builtin_assign)
					result->tp_init.tp_assign = NULL;
				break;

			case CLASS_OPERATOR_SUPERARGS:
				constructor_features |= FEATURE_SUPERARGS;
				break;

			case OPERATOR_CALL:
				result->tp_call     = &usrtype__call__with__CALL;
				result->tp_callable = &instance_user_callable;
				break;

			case OPERATOR_STR:
				result->tp_cast.tp_str = &usrtype__str__with__STR;
				if (result->tp_cast.tp_print == NULL)
					result->tp_cast.tp_print = &usrtype__print__with__STR;
				break;

			case CLASS_OPERATOR_PRINT:
				result->tp_cast.tp_print = &usrtype__print__with__PRINT;
				if (result->tp_cast.tp_str == NULL)
					result->tp_cast.tp_str = &usrtype__str__with__PRINT;
				break;

			case OPERATOR_REPR:
				result->tp_cast.tp_repr = &usrtype__repr__with__REPR;
				if (result->tp_cast.tp_printrepr == NULL)
					result->tp_cast.tp_printrepr = &usrtype__printrepr__with__REPR;
				break;

			case CLASS_OPERATOR_PRINTREPR:
				result->tp_cast.tp_printrepr = &usrtype__printrepr__with__PRINTREPR;
				if (result->tp_cast.tp_repr == NULL)
					result->tp_cast.tp_repr = &usrtype__repr__with__PRINTREPR;
				break;

			case OPERATOR_DEEPCOPY:
				/* A user-defined deepcopy callback must not invoke the deepload
				 * operator, which would normally replace all members with deep
				 * copies of themself. */
				result->tp_init.tp_deepload = NULL;
				ATTR_FALLTHROUGH
			default:
				/* Bind the C-wrapper-function for this operator. */
				if (bind_class_operator(result_type_type, result, op->co_name))
					goto err_r_base_cbases;
				break;
			}
		} while (++i <= desc->cd_clsop_mask);
		switch (constructor_features) {

		case FEATURE_CONSTRUCTOR | FEATURE_SUPERARGS:
			/* TODO: SUPERARGS + CLASS_TP_FAUTOINIT */
			if (desc->cd_flags & CLASS_TP_FSUPERKWDS) {
				result->tp_init.tp_alloc.tp_ctor        = &instance_kwsuper_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_kwsuper_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_kwsuper_initkw;
			} else {
				result->tp_init.tp_alloc.tp_ctor        = &instance_super_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_super_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_super_initkw;
			}
			break;

		case FEATURE_CONSTRUCTOR:
#ifdef CLASS_TP_FAUTOINIT
			if (desc->cd_flags & CLASS_TP_FAUTOINIT) {
#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
				if (cbases.cb_base == NULL || cbases.cb_base == &DeeObject_Type) {
					result->tp_init.tp_alloc.tp_ctor        = &instance_auto_nobase_ctor;
					result->tp_init.tp_alloc.tp_any_ctor    = &instance_auto_nobase_init;
					result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_auto_nobase_initkw;
				} else
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
				{
					result->tp_init.tp_alloc.tp_ctor        = &instance_auto_ctor;
					result->tp_init.tp_alloc.tp_any_ctor    = &instance_auto_init;
					result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_auto_initkw;
				}
			} else
#endif /* CLASS_TP_FAUTOINIT */
			if (desc->cd_flags & TP_FINHERITCTOR) {
				result->tp_init.tp_alloc.tp_ctor        = &instance_inherited_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_inherited_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_inherited_initkw;
			} else
#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
			if (cbases.cb_base == NULL || cbases.cb_base == &DeeObject_Type) {
				result->tp_init.tp_alloc.tp_ctor        = &instance_nobase_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_nobase_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_nobase_initkw;
			} else
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
			{
				result->tp_init.tp_alloc.tp_ctor        = &instance_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_initkw;
			}
			break;

		case FEATURE_SUPERARGS:
			/* TODO: SUPERARGS + CLASS_TP_FAUTOINIT */
			if (desc->cd_flags & CLASS_TP_FSUPERKWDS) {
				result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_kwsuper_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_kwsuper_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_kwsuper_initkw;
			} else {
				result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_super_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_super_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_super_initkw;
			}
			break;

		default:
#ifdef CLASS_TP_FAUTOINIT
			if (desc->cd_flags & CLASS_TP_FAUTOINIT) {
#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
				if (cbases.cb_base == NULL || cbases.cb_base == &DeeObject_Type) {
					result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_auto_nobase_ctor;
					result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_auto_nobase_init;
					result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_auto_nobase_initkw;
				} else
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
				{
					result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_auto_ctor;
					result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_auto_init;
					result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_auto_initkw;
				}
			} else
#endif /* CLASS_TP_FAUTOINIT */
			if (desc->cd_flags & TP_FINHERITCTOR) {
				/* this = super */
				result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_inherited_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_inherited_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_inherited_initkw;
			} else
#ifdef CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS
			if (cbases.cb_base == NULL || cbases.cb_base == &DeeObject_Type) {
				result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_nobase_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_nobase_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_nobase_initkw;
			} else
#endif /* CONFIG_NOBASE_OPTIMIZED_CLASS_OPERATORS */
			{
				/* Undefined constructor... */
				result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_initkw;
			}
			break;
		}
	}

	/* Provide builtin support for comparison, if not already defined by the type itself! */
#ifdef CLASS_TP_FAUTOINIT
	if ((!result->tp_cast.tp_repr && !result->tp_cast.tp_printrepr) && (desc->cd_flags & CLASS_TP_FAUTOINIT))
		result->tp_cast.tp_printrepr = &instance_builtin_auto_printrepr;
#endif /* CLASS_TP_FAUTOINIT */
	if (!result->tp_cmp && !(desc->cd_flags & CLASS_TP_FNOBUILTIN)
#if 1 /* TODO: Get rid of this once the compiler is able to produce "CLASS_TP_FNOBUILTIN"
       * right now, this is here for the "class MyCell: Cell" test (asserting that cross-
       * dependent operators are inherited as sets) */
	    && desc->cd_imemb_size > 0
#endif
	    )
		result->tp_cmp = &instance_builtin_cmp;

	/* Make sure to disallow MOVE-ANY when the builtin move-assign operator is used. */
	if (result->tp_init.tp_move_assign == &instance_builtin_moveassign)
		result->tp_flags &= ~TP_FMOVEANY;

	/* If the class has a custom destructor, it *may* be able to revive itself during destruction.
	 * XXX: Technically only needed if the dtor function doesn't support "METHOD_FNOREFESCAPE",
	 *      but we don't keep track of attributes like that for user-functions (yet?) */
	if (result->tp_init.tp_dtor == &instance_destructor)
		result->tp_flags |= TP_FMAYREVIVE;

	/* Assign the declaring module (if given) */
	if likely(declaring_module != NULL)
		Dee_weakref_init(&result->tp_module, (DeeObject *)declaring_module, NULL);

	/* Initialize custom fields of the underlying type. */
	if (result_type_type != &DeeType_Type) {
		int error = 0;
		if (result_type_type->tp_init.tp_alloc.tp_ctor) {
			error = (*result_type_type->tp_init.tp_alloc.tp_ctor)((DeeObject *)result);
		} else if (result_type_type->tp_init.tp_alloc.tp_any_ctor) {
			error = (*result_type_type->tp_init.tp_alloc.tp_any_ctor)((DeeObject *)result, 0, NULL);
		} else if (result_type_type->tp_init.tp_alloc.tp_any_ctor_kw) {
			error = (*result_type_type->tp_init.tp_alloc.tp_any_ctor_kw)((DeeObject *)result, 0, NULL, NULL);
		}
		if unlikely(error)
			goto err_r_base_cbases;
	}

	/* Initialize the resulting object, and start tracking it. */
	DeeObject_Init(result, result_type_type);
	return (DeeTypeObject *)DeeGC_Track((DeeObject *)result);
err_r_base_cbases:
	Dee_weakref_fini(&result->tp_module);
	Dee_Free(result->tp_math);
	Dee_Free(result->tp_cmp);
	Dee_Free(result->tp_seq);
	Dee_Free((void *)result->tp_attr);
	Dee_Free(result->tp_with);
	Dee_Free((void *)result->tp_operators);
	if (result_type_type != &DeeType_Type) {
		/* Free extra operator class tables. */
		do {
			size_t i;
			ASSERT(DeeType_IsTypeType(result_type_type));
			for (i = 0; i < result_type_type->tp_operators_size; ++i) {
				struct type_operator const *op = &result_type_type->tp_operators[i];
				if (Dee_type_operator_isdecl(op)) {
					uint16_t oi_class = op->to_decl.oi_class;
					if (oi_class != 0) {
						void *class_table;
						class_table = *(void **)((byte_t *)result + oi_class);
						if (class_table) {
							*(void **)((byte_t *)result + oi_class) = NULL;
							Dee_Free(class_table);
						}
					}
				}
			}
			result_type_type = DeeType_Base(result_type_type);
		} while (result_type_type != &DeeType_Type);
	}
	Dee_XDecref_unlikely(result->tp_base);
	Dee_XDecref_unlikely(desc->cd_name);
	Dee_XDecref_unlikely(desc->cd_doc);
	Dee_Decref_unlikely(desc);
/*err_r:*/
	DeeGCObject_Free(result);
err_cbases:
	class_bases_fini(&cbases);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CLASS_C */
