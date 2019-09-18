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
#ifndef GUARD_DEEMON_OBJECTS_CLASS_C
#define GUARD_DEEMON_OBJECTS_CLASS_C 1
#define _KOS_SOURCE 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/string.h>

#include "../runtime/runtime_error.h"

DECL_BEGIN

/* Class callbacks for inside of `type' */
INTERN void DCALL
class_fini(DeeTypeObject *__restrict self) {
	struct class_desc *my_class;
	DREF DeeObject *buffer[64];
	size_t buflen;
	uint16_t i, size;
	my_class = self->tp_class;
	//my_class = DeeClass_DESC(self); /* This fails, because `self' may no longer be a valid object */
	/* Clear all class members (including cached operators). */
	buflen = 0;
	size   = my_class->cd_desc->cd_cmemb_size;
again:
	rwlock_write(&my_class->cd_lock);
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
			rwlock_endwrite(&my_class->cd_lock);
			Dee_Decref(ob);
			while (buflen) {
				--buflen;
				Dee_Decref(buffer[buflen]);
			}
			rwlock_write(&my_class->cd_lock);
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
				rwlock_endwrite(&my_class->cd_lock);
				Dee_Decref(ob);
				while (buflen) {
					--buflen;
					Dee_Decref(buffer[buflen]);
				}
				rwlock_write(&my_class->cd_lock);
				goto again;
			}
			buffer[buflen++] = ob; /* Inherit reference. */
		}
	}
	rwlock_endwrite(&my_class->cd_lock);
	if (buflen) {
		/* Clear the buffer. */
		while (buflen) {
			--buflen;
			Dee_Decref(buffer[buflen]);
		}
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
	if (!self->tp_base || self->tp_math != self->tp_base->tp_math)
		Dee_Free(self->tp_math);
	if ((self->tp_cmp != &instance_builtin_cmp) &&
	    (!self->tp_base || self->tp_cmp != self->tp_base->tp_cmp))
		Dee_Free(self->tp_cmp);
	if (!self->tp_base || self->tp_seq != self->tp_base->tp_seq)
		Dee_Free(self->tp_seq);
	if (!self->tp_base || self->tp_attr != self->tp_base->tp_attr)
		Dee_Free(self->tp_attr);
	if (!self->tp_base || self->tp_with != self->tp_base->tp_with)
		Dee_Free(self->tp_with);
}

INTERN void DCALL
class_visit(DeeTypeObject *__restrict self, dvisit_t proc, void *arg) {
	struct class_desc *my_class;
	uint16_t i, size;
	my_class = DeeClass_DESC(self);
	size     = my_class->cd_desc->cd_cmemb_size;
	rwlock_read(&my_class->cd_lock);
	for (i = 0; i < size; ++i)
		Dee_XVisit(my_class->cd_members[i]);
	/* Also free all cached operators. */
	for (i = 0; i < CLASS_HEADER_OPC1; ++i) {
		struct class_optable *table;
		uint16_t j;
		table = my_class->cd_ops[i];
		if (!table)
			continue;
		for (j = 0; j < CLASS_HEADER_OPC2; ++j)
			Dee_XVisit(table->co_operators[j]);
	}
	rwlock_endread(&my_class->cd_lock);
	/* Only ever references strings itself, so no point in visiting this one! */
	/*Dee_Visit(my_class->cd_desc);*/
}

INTERN void DCALL
class_clear(DeeTypeObject *__restrict self) {
	struct class_desc *my_class;
	DREF DeeObject *buffer[64];
	size_t buflen;
	uint16_t i, size;
	my_class = DeeClass_DESC(self);
	/* Clear all class members (including cached operators). */
	buflen = 0;
	size   = my_class->cd_desc->cd_cmemb_size;
again:
	rwlock_write(&my_class->cd_lock);
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
			rwlock_endwrite(&my_class->cd_lock);
			Dee_Decref(ob);
			while (buflen) {
				--buflen;
				Dee_Decref(buffer[buflen]);
			}
			rwlock_write(&my_class->cd_lock);
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
				rwlock_endwrite(&my_class->cd_lock);
				Dee_Decref(ob);
				while (buflen) {
					--buflen;
					Dee_Decref(buffer[buflen]);
				}
				rwlock_write(&my_class->cd_lock);
				goto again;
			}
			buffer[buflen++] = ob; /* Inherit reference. */
		}
	}
	rwlock_endwrite(&my_class->cd_lock);
	if (buflen) {
		/* Clear the buffer. */
		while (buflen) {
			--buflen;
			Dee_Decref(buffer[buflen]);
		}
		/* Since custom destructors may have been able to
		 * re-assign new members, we must keep clearing them
		 * all until none are left! */
		goto again;
	}
}

INTERN void DCALL
class_pclear(DeeTypeObject *__restrict self, unsigned int gc_priority) {
	struct class_desc *my_class;
	DREF DeeObject *buffer[64];
	size_t buflen;
	uint16_t i, size;
	my_class = DeeClass_DESC(self);
	/* Clear all class members (including cached operators). */
	buflen = 0;
	size   = my_class->cd_desc->cd_cmemb_size;
again:
	rwlock_write(&my_class->cd_lock);
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
			rwlock_endwrite(&my_class->cd_lock);
			Dee_Decref(ob);
			while (buflen) {
				--buflen;
				Dee_Decref(buffer[buflen]);
			}
			rwlock_write(&my_class->cd_lock);
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
				rwlock_endwrite(&my_class->cd_lock);
				Dee_Decref(ob);
				while (buflen) {
					--buflen;
					Dee_Decref(buffer[buflen]);
				}
				rwlock_write(&my_class->cd_lock);
				goto again;
			}
			buffer[buflen++] = ob; /* Inherit reference. */
		}
	}
	rwlock_endwrite(&my_class->cd_lock);
	if (buflen) {
		/* Clear the buffer. */
		while (buflen) {
			--buflen;
			Dee_Decref(buffer[buflen]);
		}
		/* Since custom destructors may have been able to
		 * re-assign new members, we must keep clearing them
		 * all until none are left! */
		goto again;
	}
}


PRIVATE void DCALL
calls_desc_cache_operator(struct class_desc *__restrict self,
                          uint16_t name, DeeObject *__restrict func) {
	struct class_optable *table;
	uint16_t table_index;
	ASSERT(name < CLASS_OPERATOR_USERCOUNT);
	table_index = name / CLASS_HEADER_OPC2;
again:
	table = ATOMIC_READ(self->cd_ops[table_index]);
	if (!table) {
		table = (struct class_optable *)Dee_TryCalloc(sizeof(struct class_optable));
		if unlikely(!table)
			goto done;
		if (!ATOMIC_CMPXCH(self->cd_ops[table_index], NULL, table)) {
			/* Table was already allocated! */
			Dee_Free(table);
			goto again;
		}
	}
	/* Cache the operator function in the callback table. */
	if (ATOMIC_CMPXCH(table->co_operators[name % CLASS_HEADER_OPC2], NULL, func))
		Dee_Incref(func);
done:;
}

#define OPERATOR_IS_CONSTRUCTOR_INHERITED(x) \
	((x) <= OPERATOR_MOVEASSIGN || (x) == CLASS_OPERATOR_SUPERARGS)

PRIVATE DREF DeeObject *DCALL
class_desc_get_known_operator(DeeTypeObject *__restrict tp_self,
                              struct class_desc *__restrict self,
                              uint16_t name) {
	DREF DeeObject *result;
	DeeClassDescriptorObject *desc;
	uint16_t i, perturb;
	if (name < CLASS_OPERATOR_USERCOUNT) {
		struct class_optable *table;
		table = self->cd_ops[name / CLASS_HEADER_OPC2];
		if likely(table) {
			rwlock_read(&self->cd_lock);
			result = table->co_operators[name % CLASS_HEADER_OPC2];
			if likely(result) {
				Dee_Incref(result);
				rwlock_endread(&self->cd_lock);
				return result;
			}
			rwlock_endread(&self->cd_lock);
		}
	}
	/* Lookup extended, or un-cached operators. */
	desc = self->cd_desc;
	i = perturb = name & desc->cd_clsop_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_operator *entry;
		entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
		ASSERTF(entry->co_name != (uint16_t)-1, "Operator %#I16x not implemented", name);
		if (entry->co_name != name)
			continue;
		/* Found the entry! */
		ASSERT(entry->co_addr < desc->cd_cmemb_size);
		rwlock_read(&self->cd_lock);
		result = self->cd_members[entry->co_addr];
		if unlikely(!result) {
			rwlock_endread(&self->cd_lock);
			err_unimplemented_operator(tp_self, name);
			return NULL;
		}
		Dee_Incref(result);
		rwlock_endread(&self->cd_lock);
		/* Try to cache the associated operator (if possible) */
		if (name < CLASS_OPERATOR_USERCOUNT)
			calls_desc_cache_operator(self, name, result);
		return result;
	}
}

/* Return the nearest operator function for `name',
 * implemented by `self', which must be a class type.
 * If the operator doesn't exist, return NULL and throw
 * a NotImplemented error, or return NULL and don't throw
 * an error when `DeeClass_TryGetOperator()' was used. */
PUBLIC DREF DeeObject *DCALL
DeeClass_GetOperator(DeeTypeObject *__restrict self, uint16_t name) {
	DREF DeeObject *result;
	DeeTypeObject *iter = self;
	ASSERT(DeeType_IsClass(iter));
	do {
		/* Search the descriptor cache of this type. */
		struct class_desc *iter_class;
		DeeClassDescriptorObject *desc;
		uint16_t i, perturb;
		iter_class = DeeClass_DESC(iter);
		if (name < CLASS_OPERATOR_USERCOUNT) {
			struct class_optable *table;
			table = iter_class->cd_ops[name / CLASS_HEADER_OPC2];
			if likely(table) {
				rwlock_read(&iter_class->cd_lock);
				result = table->co_operators[name % CLASS_HEADER_OPC2];
				if likely(result) {
					Dee_Incref(result);
					rwlock_endread(&iter_class->cd_lock);
					/* Inherit the base's operator locally, by caching it. */
					if (iter != self && !OPERATOR_IS_CONSTRUCTOR_INHERITED(name))
						calls_desc_cache_operator(DeeClass_DESC(self), name, result);
					return result;
				}
				rwlock_endread(&iter_class->cd_lock);
			}
		}
		/* Search the operator table of the type. */
		desc = iter_class->cd_desc;
		i = perturb = name & desc->cd_clsop_mask;
		for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
			struct class_operator *entry;
			entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
			if (entry->co_name != name) {
				if (entry->co_name == (uint16_t)-1)
					break; /* Not implemented! */
				continue;
			}
			/* Found the entry! */
			ASSERT(entry->co_addr < desc->cd_cmemb_size);
			rwlock_read(&iter_class->cd_lock);
			result = iter_class->cd_members[entry->co_addr];
			if unlikely(!result) {
				rwlock_endread(&iter_class->cd_lock);
				goto done; /* Deleted operator. */
			}
			Dee_Incref(result);
			rwlock_endread(&iter_class->cd_lock);
			/* Try to cache the associated operator (if possible)
			 * NOTE: Make sure not to accidentally cache inherited constructors! */
			if (name < CLASS_OPERATOR_USERCOUNT) {
				if (iter == self || !OPERATOR_IS_CONSTRUCTOR_INHERITED(name))
					calls_desc_cache_operator(DeeClass_DESC(self), name, result);
			}
			return result;
		}
	} while ((iter = DeeType_Base(iter)) != NULL && DeeType_IsClass(iter));
done:
	err_unimplemented_operator(self, name);
	return NULL;
}

/* Same as `DeeClass_GetOperator()', but don't simply return `NULL'
 * if the operator hasn't been implemented, and `ITER_DONE' when it
 * has been, but wasn't assigned anything. */
PUBLIC DREF DeeObject *DCALL
DeeClass_TryGetOperator(DeeTypeObject *__restrict self, uint16_t name) {
	DREF DeeObject *result;
	DeeTypeObject *iter = self;
	ASSERT(DeeType_IsClass(iter));
	do {
		/* Search the descriptor cache of this type. */
		struct class_desc *iter_class;
		DeeClassDescriptorObject *desc;
		uint16_t i, perturb;
		iter_class = DeeClass_DESC(iter);
		if (name < CLASS_OPERATOR_USERCOUNT) {
			struct class_optable *table;
			table = iter_class->cd_ops[name / CLASS_HEADER_OPC2];
			if likely(table) {
				rwlock_read(&iter_class->cd_lock);
				result = table->co_operators[name % CLASS_HEADER_OPC2];
				if likely(result) {
					Dee_Incref(result);
					rwlock_endread(&iter_class->cd_lock);
					/* Inherit the base's operator locally, by caching it. */
					if (iter != self && !OPERATOR_IS_CONSTRUCTOR_INHERITED(name))
						calls_desc_cache_operator(DeeClass_DESC(self), name, result);
					return result;
				}
				rwlock_endread(&iter_class->cd_lock);
			}
		}
		/* Search the operator table of the type. */
		desc = iter_class->cd_desc;
		i = perturb = name & desc->cd_clsop_mask;
		for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
			struct class_operator *entry;
			entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
			if (entry->co_name != name) {
				if (entry->co_name == (uint16_t)-1)
					break; /* Not implemented! */
				continue;
			}
			/* Found the entry! */
			ASSERT(entry->co_addr < desc->cd_cmemb_size);
			rwlock_read(&iter_class->cd_lock);
			result = iter_class->cd_members[entry->co_addr];
			if unlikely(!result) {
				rwlock_endread(&iter_class->cd_lock);
				return NULL; /* Deleted operator. */
			}
			Dee_Incref(result);
			rwlock_endread(&iter_class->cd_lock);
			/* Try to cache the associated operator (if possible)
			 * NOTE: Make sure not to accidentally cache inherited constructors! */
			if (name < CLASS_OPERATOR_USERCOUNT) {
				if (iter == self || !OPERATOR_IS_CONSTRUCTOR_INHERITED(name))
					calls_desc_cache_operator(DeeClass_DESC(self), name, result);
			}
			return result;
		}
	} while ((iter = DeeType_Base(iter)) != NULL && DeeType_IsClass(iter));
	return NULL;
}

/* Same as `DeeClass_TryGetOperator()', but don't return an operator
 * that has been inherited from a base-class, but return `NULL' instead. */
PUBLIC DREF DeeObject *DCALL
DeeClass_TryGetPrivateOperator(DeeTypeObject *__restrict self, uint16_t name) {
	DREF DeeObject *result;
	DeeClassDescriptorObject *desc;
	uint16_t i, perturb;
	struct class_desc *self_class;
	self_class = DeeClass_DESC(self);
	/* Lookup extended, or un-cached operators. */
	desc = self_class->cd_desc;
	i = perturb = name & desc->cd_clsop_mask;
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		struct class_operator *entry;
		entry = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
		if (entry->co_name != name) {
			if (entry->co_name == (uint16_t)-1)
				break; /* Not implemented! */
			continue;
		}
		/* Found the entry! */
		ASSERT(entry->co_addr < desc->cd_cmemb_size);
		rwlock_read(&self_class->cd_lock);
		result = self_class->cd_members[entry->co_addr];
		if unlikely(!result) {
			rwlock_endread(&self_class->cd_lock);
			break; /* Deleted operator. */
		}
		Dee_Incref(result);
		rwlock_endread(&self_class->cd_lock);
		return result;
	}
	return NULL;
}


INTERN void DCALL
instance_clear_members(struct instance_desc *__restrict self, uint16_t size) {
	DREF DeeObject *buffer[64];
	size_t buflen;
	uint16_t i;
	buflen = 0;
again:
	rwlock_write(&self->id_lock);
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
			rwlock_endwrite(&self->id_lock);
			Dee_Decref(ob);
			while (buflen) {
				--buflen;
				Dee_Decref(buffer[buflen]);
			}
			rwlock_write(&self->id_lock);
			goto again;
		}
		buffer[buflen++] = ob; /* Inherit reference. */
	}
	rwlock_endwrite(&self->id_lock);
	if (buflen) {
		/* Clear the buffer. */
		while (buflen) {
			--buflen;
			Dee_Decref(buffer[buflen]);
		}
		/* Since custom destructors may have been able to
		 * re-assign new members, we must keep clearing them
		 * all until none are left! */
		goto again;
	}
}


INTERN void DCALL
instance_builtin_destructor(DeeObject *__restrict self) {
	struct class_desc *desc;
	desc = DeeClass_DESC(Dee_TYPE(self));
	/* Clear all the members of this instance. */
	instance_clear_members(DeeInstance_DESC(desc, self),
	                       desc->cd_desc->cd_imemb_size);
}

INTERN void DCALL
instance_destructor(DeeObject *__restrict self) {
	DREF DeeObject *callback, *result;
	DeeTypeObject *tp_self  = Dee_TYPE(self);
	struct class_desc *desc = DeeClass_DESC(tp_self);
	callback                = class_desc_get_known_operator(tp_self, desc, OPERATOR_DESTRUCTOR);
	if unlikely(!callback)
		result = NULL;
	else {
		dref_t new_refcnt;
		ATOMIC_FETCHINC(self->ob_refcnt);
		result = DeeObject_ThisCall(callback, self, 0, NULL);
		Dee_Decref(callback);
		/* Check if `self' got revived. - If it did we let the caller
		 * inherit a reference to it to prevent a race condition. */
		for (;;) {
			new_refcnt = ATOMIC_READ(self->ob_refcnt);
			if (new_refcnt > 1) {
				/* Object got revived (let the caller inherit our reference) */
				if likely(result)
					Dee_Decref(result);
				else {
					DeeError_Print("Unhandled error in destructor\n",
					               ERROR_PRINT_DOHANDLE);
				}
				return;
			}
			if (ATOMIC_CMPXCH_WEAK(self->ob_refcnt, new_refcnt, 0))
				break;
		}
	}
	if likely(result)
		Dee_Decref(result);
	else {
		DeeError_Print("Unhandled error in destructor\n",
		               ERROR_PRINT_DOHANDLE);
	}
	/* Clear all the members of this instance. */
	instance_clear_members(DeeInstance_DESC(desc, self),
	                       desc->cd_desc->cd_imemb_size);
}


#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
#define IF_NOBASE(x) x
#else                /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
#define IF_NOBASE(x) /* nothing */
#endif               /* !CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
#ifdef CLASS_TP_FAUTOINIT
#define IF_AUTOINIT(x) x
#else                  /* CLASS_TP_FAUTOINIT */
#define IF_AUTOINIT(x) /* nothing */
#endif                 /* !CLASS_TP_FAUTOINIT */

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


INTERN int DCALL
type_invoke_base_constructor(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self, size_t argc,
                             DeeObject **__restrict argv, DeeObject *kw) {
	ASSERT(!(tp_self->tp_flags & TP_FVARIABLE));
	if (kw) {
		if (tp_self->tp_init.tp_alloc.tp_any_ctor_kw) {
			int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict, DeeObject *);
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
	if (tp_self->tp_init.tp_alloc.tp_any_ctor) {
		int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict);
		func = tp_self->tp_init.tp_alloc.tp_any_ctor;
		return DeeType_INVOKE_ANY_CTOR(func, tp_self, self, argc, argv);
	}
	if (tp_self->tp_init.tp_alloc.tp_ctor && !argc) {
		int(DCALL * func)(DeeObject * __restrict);
		func = tp_self->tp_init.tp_alloc.tp_ctor;
		return DeeType_INVOKE_CTOR(func, tp_self, self);
	}
	if (tp_self->tp_init.tp_alloc.tp_copy_ctor &&
	    (argc == 1 && DeeObject_InstanceOf(argv[0], tp_self))) {
		int(DCALL * func)(DeeObject * __restrict, DeeObject * __restrict);
		func = tp_self->tp_init.tp_alloc.tp_copy_ctor;
		return DeeType_INVOKE_COPY(func, tp_self, self, argv[0]);
	}
	return err_unimplemented_constructor(tp_self, argc, argv);
err_no_keywords:
	return err_keywords_ctor_not_accepted(tp_self, kw);
err:
	return -1;
}



PRIVATE int DCALL
instance_initsuper_as_copy(DeeTypeObject *__restrict tp_super,
                           DeeObject *__restrict self,
                           DeeObject *__restrict other,
                           bool deep_copy) {
	int result;
	/* Handle constructor inheritance */
	while (tp_super->tp_flags & TP_FINHERITCTOR)
		ASSERTF(!(tp_super->tp_flags & TP_FFINAL),
		        "Type derived from final type"),
		ASSERT(DeeType_Base(tp_super)),
		tp_super = DeeType_Base(tp_super);
	ASSERTF(!(tp_super->tp_flags & TP_FVARIABLE), "Type derived from variable type");
	/* Initialize the super-type. */
	if (tp_super->tp_init.tp_alloc.tp_deep_ctor && deep_copy) {
		int(DCALL * func)(DeeObject * __restrict, DeeObject * __restrict);
		func   = tp_super->tp_init.tp_alloc.tp_deep_ctor;
		result = DeeType_INVOKE_DEEPCOPY(func, tp_super, self, other);
	} else if (tp_super->tp_init.tp_alloc.tp_copy_ctor) {
		int(DCALL * func)(DeeObject * __restrict, DeeObject * __restrict);
		func   = tp_super->tp_init.tp_alloc.tp_copy_ctor;
		result = DeeType_INVOKE_COPY(func, tp_super, self, other);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor) {
		int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor;
		result = DeeType_INVOKE_ANY_CTOR(func, tp_super, self, 1, (DeeObject **)&other);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor_kw) {
		int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict, DeeObject *);
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
INTERN int DCALL
instance_tcopy(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               DeeObject *__restrict other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;
	/* Lookup the copy operator function. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_COPY);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_tdeepcopy(DeeTypeObject *__restrict tp_self,
                   DeeObject *__restrict self,
                   DeeObject *__restrict other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;
	/* Lookup the copy operator function. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_DEEPCOPY);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		if (instance_initsuper_as_copy(tp_super, self, other, true))
			goto err_members;
	}
	ASSERT(!tp_self->tp_init.tp_deepload);
	/* Add a deepcopy association for `self' replacing `other' */
	if (Dee_DeepCopyAddAssoc(self, other))
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

INTERN int DCALL
instance_copy(DeeObject *__restrict self,
              DeeObject *__restrict other) {
	return instance_tcopy(Dee_TYPE(self), self, other);
}

INTERN int DCALL
instance_deepcopy(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
	return instance_tdeepcopy(Dee_TYPE(self), self, other);
}


INTERN int DCALL
instance_builtin_tcopy(DeeTypeObject *__restrict tp_self,
                       DeeObject *__restrict self,
                       DeeObject *__restrict other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	struct instance_desc *other_instance;
	DeeTypeObject *tp_super;
	uint16_t i, size;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	/* Initialize the members of this instance as
	 * references to the same also found in `other'. */
	rwlock_init(&instance->id_lock);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	rwlock_read(&other_instance->id_lock);
	MEMCPY_PTR(instance->id_vtab,
	           other_instance->id_vtab, size);
	for (i = 0; i < size; ++i)
		Dee_XIncref(instance->id_vtab[i]);
	rwlock_endread(&other_instance->id_lock);
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
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN int DCALL
instance_builtin_nobase_tcopy(DeeTypeObject *__restrict tp_self,
                              DeeObject *__restrict self,
                              DeeObject *__restrict other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	struct instance_desc *other_instance;
	uint16_t i, size;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	/* Initialize the members of this instance as
	 * references to the same also found in `other'. */
	rwlock_init(&instance->id_lock);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	rwlock_read(&other_instance->id_lock);
	MEMCPY_PTR(instance->id_vtab,
	           other_instance->id_vtab, size);
	for (i = 0; i < size; ++i)
		Dee_XIncref(instance->id_vtab[i]);
	rwlock_endread(&other_instance->id_lock);
	return 0;
}
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
INTERN int DCALL
instance_builtin_tdeepload(DeeTypeObject *__restrict tp_self,
                           DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	uint16_t i, size;
	/* Replace all members with deep copies of themself. */
	size = desc->cd_desc->cd_imemb_size;
	for (i = 0; i < size; ++i) {
		if (DeeObject_XInplaceDeepCopyWithLock(&instance->id_vtab[i],
		                                       &instance->id_lock))
			goto err;
	}
	return 0;
err:
	return -1;
}

INTERN int DCALL
instance_builtin_copy(DeeObject *__restrict self,
                      DeeObject *__restrict other) {
	return instance_builtin_tcopy(Dee_TYPE(self), self, other);
}
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN int DCALL
instance_builtin_nobase_copy(DeeObject *__restrict self,
                             DeeObject *__restrict other) {
	return instance_builtin_nobase_tcopy(Dee_TYPE(self), self, other);
}
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */

INTERN int DCALL
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

#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN int DCALL
instance_builtin_nobase_deepload(DeeObject *__restrict self) {
	return instance_builtin_tdeepload(Dee_TYPE(self), self);
}
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */




INTDEF int DCALL DeeObject_TAssign(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict some_object);
INTDEF int DCALL DeeObject_TMoveAssign(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);

INTERN int DCALL
instance_builtin_tassign(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeObject *__restrict other) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	struct instance_desc *other_instance;
	DeeTypeObject *tp_super;
	uint16_t i, size;
	DREF DeeObject **old_items;
	if unlikely(self == other)
		goto done;
	if (DeeObject_AssertType(other, tp_self))
		goto err;
	tp_super = DeeType_Base(tp_self);
	if (tp_super && DeeObject_TAssign(tp_super, self, other))
		goto err;
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	old_items      = (DREF DeeObject **)Dee_AMalloc(size * sizeof(DREF DeeObject *));
	if unlikely(!old_items)
		goto err;
	/* Load member values from `others' */
	rwlock_read(&other_instance->id_lock);
	MEMCPY_PTR(old_items, other_instance->id_vtab, size);
	for (i = 0; i < size; ++i)
		Dee_XIncref(old_items[i]);
	rwlock_endread(&other_instance->id_lock);
	/* Exchange our own member values with those loaded from `other' */
	rwlock_write(&instance->id_lock);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *temp;
		temp                 = instance->id_vtab[i];
		instance->id_vtab[i] = old_items[i];
		old_items[i]         = temp;
	}
	rwlock_endwrite(&instance->id_lock);
	/* Decref all the old items. */
	for (i = 0; i < size; ++i)
		Dee_XDecref(old_items[i]);
	Dee_AFree(old_items);
done:
	return 0;
err:
	return -1;
}

INTERN int DCALL
instance_builtin_tmoveassign(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self,
                             DeeObject *__restrict other) {
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
	old_items      = (DREF DeeObject **)Dee_AMalloc(size * sizeof(DREF DeeObject *));
	if unlikely(!old_items)
		goto err;
	/* Load member values from `others', while also unbinding all members. */
	rwlock_write(&other_instance->id_lock);
	MEMCPY_PTR(old_items, other_instance->id_vtab, size);
	MEMSET_PTR(other_instance->id_vtab, 0, size);
	rwlock_endread(&other_instance->id_lock);
	/* Exchange our own member values with those loaded from `other' */
	rwlock_write(&instance->id_lock);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *temp;
		temp                 = instance->id_vtab[i];
		instance->id_vtab[i] = old_items[i];
		old_items[i]         = temp;
	}
	rwlock_endwrite(&instance->id_lock);
	/* Decref all the old items. */
	for (i = 0; i < size; ++i)
		Dee_XDecref(old_items[i]);
	Dee_AFree(old_items);
done:
	return 0;
err:
	return -1;
}

INTERN int DCALL
instance_builtin_assign(DeeObject *__restrict self,
                        DeeObject *__restrict other) {
	return instance_builtin_tassign(Dee_TYPE(self), self, other);
}

INTERN int DCALL
instance_builtin_moveassign(DeeObject *__restrict self,
                            DeeObject *__restrict other) {
	return instance_builtin_tmoveassign(Dee_TYPE(self), self, other);
}



LOCAL int DCALL
instance_initsuper_as_ctor(DeeTypeObject *__restrict tp_super,
                           DeeObject *__restrict self) {
	int result;
	/* Handle constructor inheritance */
	while (tp_super->tp_flags & TP_FINHERITCTOR)
		ASSERTF(!(tp_super->tp_flags & TP_FFINAL),
		        "Type derived from final type"),
		ASSERT(DeeType_Base(tp_super)),
		tp_super = DeeType_Base(tp_super);
	ASSERTF(!(tp_super->tp_flags & TP_FVARIABLE), "Type derived from variable type");
	/* Initialize the super-type. */
	if (tp_super->tp_init.tp_alloc.tp_ctor) {
		int(DCALL * func)(DeeObject * __restrict);
		func   = tp_super->tp_init.tp_alloc.tp_ctor;
		result = DeeType_INVOKE_CTOR(func, tp_super, self);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor) {
		int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor;
		result = DeeType_INVOKE_ANY_CTOR(func, tp_super, self, 0, NULL);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor_kw) {
		int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict, DeeObject *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor_kw;
		result = DeeType_INVOKE_ANY_CTOR_KW(func, tp_super, self, 0, NULL, NULL);
	} else {
		result = err_unimplemented_constructor(tp_super, 0, NULL);
	}
	return result;
}

LOCAL int DCALL
instance_initsuper_as_init(DeeTypeObject *__restrict tp_super,
                           DeeObject *__restrict self, size_t argc,
                           DeeObject **__restrict argv) {
	int result;
	/* Handle constructor inheritance */
	while (tp_super->tp_flags & TP_FINHERITCTOR)
		ASSERTF(!(tp_super->tp_flags & TP_FFINAL),
		        "Type derived from final type"),
		ASSERT(DeeType_Base(tp_super)),
		tp_super = DeeType_Base(tp_super);
	ASSERTF(!(tp_super->tp_flags & TP_FVARIABLE), "Type derived from variable type");
	/* Initialize the super-type. */
	if (tp_super->tp_init.tp_alloc.tp_any_ctor) {
		int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor;
		result = DeeType_INVOKE_ANY_CTOR(func, tp_super, self, argc, argv);
	} else if (tp_super->tp_init.tp_alloc.tp_any_ctor_kw) {
		int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict, DeeObject *);
		func   = tp_super->tp_init.tp_alloc.tp_any_ctor_kw;
		result = DeeType_INVOKE_ANY_CTOR_KW(func, tp_super, self, argc, argv, NULL);
	} else if (tp_super->tp_init.tp_alloc.tp_ctor && !argc) {
		int(DCALL * func)(DeeObject * __restrict);
		func   = tp_super->tp_init.tp_alloc.tp_ctor;
		result = DeeType_INVOKE_CTOR(func, tp_super, self);
	} else {
		result = err_unimplemented_constructor(tp_super, argc, argv);
	}
	return result;
}

LOCAL int DCALL
instance_initsuper_as_initkw(DeeTypeObject *__restrict tp_super,
                             DeeObject *__restrict self, size_t argc,
                             DeeObject **__restrict argv,
                             DeeObject *kw) {
	int result;
	/* Handle constructor inheritance */
	while (tp_super->tp_flags & TP_FINHERITCTOR)
		ASSERTF(!(tp_super->tp_flags & TP_FFINAL),
		        "Type derived from final type"),
		ASSERT(DeeType_Base(tp_super)),
		tp_super = DeeType_Base(tp_super);
	ASSERTF(!(tp_super->tp_flags & TP_FVARIABLE), "Type derived from variable type");
	/* Initialize the super-type. */
	if (tp_super->tp_init.tp_alloc.tp_any_ctor_kw) {
		int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict, DeeObject *);
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
			int(DCALL * func)(DeeObject * __restrict, size_t, DeeObject * *__restrict);
			func   = tp_super->tp_init.tp_alloc.tp_any_ctor;
			result = DeeType_INVOKE_ANY_CTOR(func, tp_super, self, argc, argv);
		} else if (tp_super->tp_init.tp_alloc.tp_ctor && !argc) {
			int(DCALL * func)(DeeObject * __restrict);
			func   = tp_super->tp_init.tp_alloc.tp_ctor;
			result = DeeType_INVOKE_CTOR(func, tp_super, self);
		} else {
			result = err_unimplemented_constructor(tp_super, argc, argv);
		}
	}
	return result;
err_no_keywords:
	return err_keywords_ctor_not_accepted(tp_super, kw);
err:
	return -1;
}

/* User-defined constructor invocation. */
/* `OPERATOR_CONSTRUCTOR' + `CLASS_OPERATOR_SUPERARGS' */
INTERN int DCALL
instance_super_tctor(DeeTypeObject *__restrict tp_self,
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_kwsuper_tctor(DeeTypeObject *__restrict tp_self,
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_super_tinit(DeeTypeObject *__restrict tp_self,
                     DeeObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_kwsuper_tinit(DeeTypeObject *__restrict tp_self,
                       DeeObject *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_super_tinitkw(DeeTypeObject *__restrict tp_self,
                       DeeObject *__restrict self, size_t argc,
                       DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args, *result;
	DeeTypeObject *tp_super;
	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_CallKw(func, argc, argv, kw);
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_kwsuper_tinitkw(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self, size_t argc,
                         DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args, *result;
	DeeTypeObject *tp_super;
	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_CallKw(func, argc, argv, kw);
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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
INTERN int DCALL
instance_builtin_super_tctor(DeeTypeObject *__restrict tp_self,
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_kwsuper_tctor(DeeTypeObject *__restrict tp_self,
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_super_tinit(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self,
                             size_t argc, DeeObject **__restrict argv) {
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_kwsuper_tinit(DeeTypeObject *__restrict tp_self,
                               DeeObject *__restrict self,
                               size_t argc, DeeObject **__restrict argv) {
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_super_tinitkw(DeeTypeObject *__restrict tp_self,
                               DeeObject *__restrict self, size_t argc,
                               DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args;
	DeeTypeObject *tp_super;
	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_CallKw(func, argc, argv, kw);
	Dee_Decref(func);
	if unlikely(!args)
		goto err;
	/* Make sure that the super-arguments object is a tuple. */
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err_args;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_kwsuper_tinitkw(DeeTypeObject *__restrict tp_self,
                                 DeeObject *__restrict self, size_t argc,
                                 DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *args;
	DeeTypeObject *tp_super;
	/* Figure out the arguments to-be passed to the super-constructor. */
	func = class_desc_get_known_operator(tp_self, desc, CLASS_OPERATOR_SUPERARGS);
	if unlikely(!func)
		goto err;
	args = DeeObject_CallKw(func, argc, argv, kw);
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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
INTERN int DCALL
instance_tctor(DeeTypeObject *__restrict tp_self,
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_tinit(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_tinitkw(DeeTypeObject *__restrict tp_self,
                 DeeObject *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* `OPERATOR_CONSTRUCTOR' (but the type doesn't have a base) */
INTERN int DCALL
instance_nobase_tctor(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_nobase_tinit(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_nobase_tinitkw(DeeTypeObject *__restrict tp_self,
                        DeeObject *__restrict self, size_t argc,
                        DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* `OPERATOR_CONSTRUCTOR', with the `TP_FINHERITCTOR' flag set.
 * NOTE: These functions always invoke the user-defined constructor without any arguments! */
INTERN int DCALL
instance_inherited_tinit(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_inherited_tinitkw(DeeTypeObject *__restrict tp_self,
                           DeeObject *__restrict self, size_t argc,
                           DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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
INTERN int DCALL
instance_builtin_tctor(DeeTypeObject *__restrict tp_self,
                       DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_tinit(DeeTypeObject *__restrict tp_self,
                       DeeObject *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	if unlikely(argc != 0) {
		err_unimplemented_constructor(tp_self, argc, argv);
		goto err;
	}
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_tinitkw(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self, size_t argc,
                         DeeObject **__restrict argv, DeeObject *kw) {
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* No predefined construction operators. (but the type doesn't have a base) */
INTERN int DCALL
instance_builtin_nobase_tctor(DeeTypeObject *__restrict tp_self,
                              DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	return 0;
}

INTERN int DCALL
instance_builtin_nobase_tinit(DeeTypeObject *__restrict tp_self,
                              DeeObject *__restrict self,
                              size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	if unlikely(argc != 0)
		return err_unimplemented_constructor(tp_self, argc, argv);
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	return 0;
}

INTERN int DCALL
instance_builtin_nobase_tinitkw(DeeTypeObject *__restrict tp_self,
                                DeeObject *__restrict self, size_t argc,
                                DeeObject **__restrict argv, DeeObject *kw) {
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	return 0;
err:
	return -1;
}
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */

/* No predefined construction operators, but `TP_FINHERITCTOR' is set. */
INTERN int DCALL
instance_builtin_inherited_tctor(DeeTypeObject *__restrict tp_self,
                                 DeeObject *__restrict self) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_inherited_tinit(DeeTypeObject *__restrict tp_self,
                                 DeeObject *__restrict self,
                                 size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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

INTERN int DCALL
instance_builtin_inherited_tinitkw(DeeTypeObject *__restrict tp_self,
                                   DeeObject *__restrict self, size_t argc,
                                   DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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


INTERN int DCALL
instance_super_ctor(DeeObject *__restrict self) {
	return instance_super_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_kwsuper_ctor(DeeObject *__restrict self) {
	return instance_kwsuper_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_super_init(DeeObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
	return instance_super_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_kwsuper_init(DeeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	return instance_kwsuper_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_super_initkw(DeeObject *__restrict self, size_t argc,
                      DeeObject **__restrict argv, DeeObject *kw) {
	return instance_super_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN int DCALL
instance_kwsuper_initkw(DeeObject *__restrict self, size_t argc,
                        DeeObject **__restrict argv, DeeObject *kw) {
	return instance_kwsuper_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN int DCALL
instance_builtin_super_ctor(DeeObject *__restrict self) {
	return instance_builtin_super_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_builtin_kwsuper_ctor(DeeObject *__restrict self) {
	return instance_builtin_kwsuper_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_builtin_super_init(DeeObject *__restrict self,
                            size_t argc, DeeObject **__restrict argv) {
	return instance_builtin_super_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_builtin_kwsuper_init(DeeObject *__restrict self,
                              size_t argc, DeeObject **__restrict argv) {
	return instance_builtin_kwsuper_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_builtin_super_initkw(DeeObject *__restrict self, size_t argc,
                              DeeObject **__restrict argv, DeeObject *kw) {
	return instance_builtin_super_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN int DCALL
instance_builtin_kwsuper_initkw(DeeObject *__restrict self, size_t argc,
                                DeeObject **__restrict argv, DeeObject *kw) {
	return instance_builtin_kwsuper_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN int DCALL
instance_ctor(DeeObject *__restrict self) {
	return instance_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_init(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	return instance_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_initkw(DeeObject *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	return instance_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN int DCALL
instance_nobase_ctor(DeeObject *__restrict self) {
	return instance_nobase_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_nobase_init(DeeObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
	return instance_nobase_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_nobase_initkw(DeeObject *__restrict self, size_t argc,
                       DeeObject **__restrict argv, DeeObject *kw) {
	return instance_nobase_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
INTERN int DCALL
instance_inherited_init(DeeObject *__restrict self,
                        size_t argc, DeeObject **__restrict argv) {
	return instance_inherited_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_inherited_initkw(DeeObject *__restrict self, size_t argc,
                          DeeObject **__restrict argv, DeeObject *kw) {
	return instance_inherited_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN int DCALL
instance_builtin_ctor(DeeObject *__restrict self) {
	return instance_builtin_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_builtin_init(DeeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	return instance_builtin_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_builtin_initkw(DeeObject *__restrict self, size_t argc,
                        DeeObject **__restrict argv, DeeObject *kw) {
	return instance_builtin_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
INTERN int DCALL
instance_builtin_nobase_ctor(DeeObject *__restrict self) {
	return instance_builtin_nobase_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_builtin_nobase_init(DeeObject *__restrict self,
                             size_t argc, DeeObject **__restrict argv) {
	return instance_builtin_nobase_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_builtin_nobase_initkw(DeeObject *__restrict self, size_t argc,
                               DeeObject **__restrict argv, DeeObject *kw) {
	return instance_builtin_nobase_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
INTERN int DCALL
instance_builtin_inherited_ctor(DeeObject *__restrict self) {
	return instance_builtin_inherited_tctor(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_builtin_inherited_init(DeeObject *__restrict self,
                                size_t argc, DeeObject **__restrict argv) {
	return instance_builtin_inherited_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_builtin_inherited_initkw(DeeObject *__restrict self, size_t argc,
                                  DeeObject **__restrict argv, DeeObject *kw) {
	return instance_builtin_inherited_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}



#ifdef CLASS_TP_FAUTOINIT
LOCAL struct class_attribute *DCALL
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

LOCAL int DCALL
instance_autoload_members(DeeTypeObject *__restrict tp_self,
                          struct class_desc *__restrict desc,
                          struct instance_desc *__restrict instance,
                          DeeObject *__restrict self,
                          size_t argc, DeeObject **__restrict argv) {
	size_t i;
	uint16_t next_table_index = 0;
	for (i = 0; i < argc; ++i) {
		struct class_attribute *at;
		at = find_next_attribute(desc->cd_desc, &next_table_index);
		if unlikely(!at)
			goto err_argc;
		if unlikely(DeeInstance_SetAttribute(desc,
			                                  instance,
			                                  self,
			                                  at,
			                                  argv[i]))
		goto err;
	}
	return 0;
err_argc:
	err_invalid_argc(tp_self->tp_name, argc, 0, i);
err:
	return -1;
}

LOCAL int DCALL
instance_autoload_members_kw(DeeTypeObject *__restrict tp_self,
                             struct class_desc *__restrict desc,
                             struct instance_desc *__restrict instance,
                             DeeObject *__restrict self, size_t argc,
                             DeeObject **__restrict argv,
                             DeeKwdsObject *kw) {
	uint16_t next_table_index;
	DREF DeeObject *iterator, *elem, *data[2];
	if (!kw)
		return instance_autoload_members(tp_self, desc, instance, self, argc, argv);
	next_table_index = 0;
	if (DeeKwds_Check(kw)) {
		size_t i, positional_argc;
		DeeObject **kw_argv;
		if unlikely(DeeKwds_SIZE(kw) > argc)
			return err_keywords_bad_for_argc(argc, DeeKwds_SIZE(kw));
		positional_argc = argc - DeeKwds_SIZE(kw);
		kw_argv         = argv + positional_argc;
		/* Load positional arguments into the first positional_argc instance members. */
		for (i = 0; i < positional_argc; ++i) {
			struct class_attribute *at;
			at = find_next_attribute(desc->cd_desc, &next_table_index);
			if unlikely(!at) {
				err_invalid_argc(tp_self->tp_name, argc, 0, i);
				goto err;
			}
			if unlikely(DeeInstance_SetAttribute(desc,
				                                  instance,
				                                  self,
				                                  at,
				                                  argv[i]))
			goto err;
		}
		for (i = 0; i <= kw->kw_mask; ++i) {
			struct class_attribute *at;
			if (!kw->kw_map[i].ke_name)
				continue;
			at = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,
			                                                       DeeString_STR(kw->kw_map[i].ke_name),
			                                                       kw->kw_map[i].ke_hash);
			if unlikely(!at || !CLASS_ATTRIBUTE_ALLOW_AUTOINIT(at)) {
				err_unknown_attribute(tp_self,
				                      DeeString_STR(kw->kw_map[i].ke_name),
				                      ATTR_ACCESS_SET);
				goto err;
			}
			if unlikely(at->ca_addr < next_table_index) {
				/* Member had already been initialized via a positional argument! */
				err_keywords_shadows_positional(DeeString_STR(kw->kw_map[i].ke_name));
				goto err;
			}
			if unlikely(DeeInstance_SetAttribute(desc,
				                                  instance,
				                                  self,
				                                  at,
				                                  kw_argv[kw->kw_map[i].ke_index]))
			goto err;
		}
	} else {
		size_t i;
		/* Load positional arguments into the first positional_argc instance members. */
		for (i = 0; i < argc; ++i) {
			struct class_attribute *at;
			at = find_next_attribute(desc->cd_desc, &next_table_index);
			if unlikely(!at) {
				err_invalid_argc(tp_self->tp_name, argc, 0, i);
				goto err;
			}
			if unlikely(DeeInstance_SetAttribute(desc,
				                                  instance,
				                                  self,
				                                  at,
				                                  argv[i]))
			goto err;
		}
		iterator = DeeObject_IterSelf((DeeObject *)kw);
		if unlikely(!iterator)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
			struct class_attribute *at;
			if unlikely(DeeObject_Unpack(elem, 2, data))
				goto err_iter_elem;
			Dee_Decref(elem);
			if (DeeObject_AssertTypeExact(data[0], &DeeString_Type))
				goto err_iter_data;
			at = DeeClassDesc_QueryInstanceAttribute(desc, data[0]);
			if unlikely(!at || !CLASS_ATTRIBUTE_ALLOW_AUTOINIT(at)) {
				err_unknown_attribute(tp_self,
				                      DeeString_STR(data[0]),
				                      ATTR_ACCESS_SET);
				goto err_iter_data;
			}
			if unlikely(at->ca_addr < next_table_index) {
				/* Member had already been initialized via a positional argument! */
				err_keywords_shadows_positional(DeeString_STR(data[0]));
				goto err_iter_data;
			}
			if unlikely(DeeInstance_SetAttribute(desc,
				                                  instance,
				                                  self,
				                                  at,
				                                  data[1]))
			goto err_iter_data;
			Dee_Decref(data[1]);
			Dee_Decref(data[0]);
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iterator);
	}
	return 0;
err_iter_data:
	Dee_Decref(data[1]);
	Dee_Decref(data[0]);
	goto err_iter;
err_iter_elem:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iterator);
err:
	return -1;
}

INTERN DREF DeeObject *DCALL
instance_builtin_auto_trepr(DeeTypeObject *__restrict tp_self,
                            DeeObject *__restrict self) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	uint16_t i, count;
	DREF DeeObject *ob;
	bool is_first = true;
	count         = desc->cd_desc->cd_imemb_size;
	if (unicode_printer_print(&printer,
	                          tp_self->tp_name,
	                          strlen(tp_self->tp_name)) < 0)
		goto err;
	if (unicode_printer_putc(&printer, '('))
		goto err;
	rwlock_read(&instance->id_lock);
	for (i = 0; i < count; ++i) {
		size_t attr_index;
		ob = instance->id_vtab[i];
		if (!ob)
			continue;
		Dee_Incref(ob);
		rwlock_endread(&instance->id_lock);
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
			if (!is_first) {
				if (UNICODE_PRINTER_PRINT(&printer, ", ") < 0)
					goto err_ob;
			}
			is_first = false;
			if (unicode_printer_printstring(&printer, (DeeObject *)at->ca_name) < 0)
				goto err_ob;
			if (UNICODE_PRINTER_PRINT(&printer, ": ") < 0)
				goto err_ob;
			if (unicode_printer_printobjectrepr(&printer, ob) < 0)
				goto err_ob;
			break;
		}
		Dee_Decref(ob);
		rwlock_read(&instance->id_lock);
	}
	rwlock_endread(&instance->id_lock);
	if (unicode_printer_putc(&printer, ')'))
		goto err;
	return unicode_printer_pack(&printer);
err_ob:
	Dee_Decref(ob);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_builtin_auto_repr(DeeObject *__restrict self) {
	return instance_builtin_auto_trepr(Dee_TYPE(self), self);
}

/* No predefined construction operators (with `CLASS_TP_FAUTOINIT'). */
INTERN int DCALL
instance_auto_tinit(DeeTypeObject *__restrict tp_self,
                    DeeObject *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	DeeTypeObject *tp_super;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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
	if unlikely(instance_autoload_members(tp_self,
		                                   desc,
		                                   instance,
		                                   self,
		                                   argc,
		                                   argv))
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

INTERN int DCALL
instance_auto_tinitkw(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self, size_t argc,
                      DeeObject **__restrict argv, DeeObject *kw) {
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
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
	if unlikely(instance_autoload_members_kw(tp_self,
		                                      desc,
		                                      instance,
		                                      self,
		                                      argc,
		                                      argv,
		                                      (DeeKwdsObject *)kw))
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

INTERN int DCALL
instance_builtin_auto_tinit(DeeTypeObject *__restrict tp_self,
                            DeeObject *__restrict self,
                            size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}
	/* Auto-initialize members. */
	if unlikely(instance_autoload_members(tp_self,
		                                   desc,
		                                   instance,
		                                   self,
		                                   argc,
		                                   argv))
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

INTERN int DCALL
instance_builtin_auto_tinitkw(DeeTypeObject *__restrict tp_self,
                              DeeObject *__restrict self, size_t argc,
                              DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DeeTypeObject *tp_super;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	/* Initialize the super-classes. */
	tp_super = DeeType_Base(tp_self);
	if (tp_super && tp_super != &DeeObject_Type) {
		/* XXX: Keyword arguments in super-constructor calls? */
		if (instance_initsuper_as_ctor(tp_super, self))
			goto err_members;
	}
	/* Auto-initialize members. */
	if unlikely(instance_autoload_members_kw(tp_self,
		                                      desc,
		                                      instance,
		                                      self,
		                                      argc,
		                                      argv,
		                                      (DeeKwdsObject *)kw))
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

INTERN int DCALL
instance_auto_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
	return instance_auto_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_auto_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	return instance_auto_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN int DCALL
instance_builtin_auto_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
	return instance_builtin_auto_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_builtin_auto_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	return instance_builtin_auto_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}


#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
/* No predefined construction operators (with `CLASS_TP_FAUTOINIT'). */
INTERN int DCALL
instance_auto_nobase_tinit(DeeTypeObject *__restrict tp_self,
                           DeeObject *__restrict self,
                           size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	DREF DeeObject *func, *result;
	/* Lookup the user-defined constructor for this class. */
	func = class_desc_get_known_operator(tp_self, desc, OPERATOR_CONSTRUCTOR);
	if unlikely(!func)
		goto err;
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_members;
	Dee_Decref(result);
	/* Auto-initialize members. */
	if unlikely(instance_autoload_members(tp_self,
		                                   desc,
		                                   instance,
		                                   self,
		                                   argc,
		                                   argv))
	goto err_members;
	Dee_Decref(func);
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN int DCALL
instance_auto_nobase_tinitkw(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self, size_t argc,
                             DeeObject **__restrict argv, DeeObject *kw) {
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
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	/* Invoke the user-defined class constructor. */
	result = DeeObject_ThisCall(func, self, 0, NULL);
	if unlikely(!result)
		goto err_members;
	Dee_Decref(result);
	/* Auto-initialize members. */
	if unlikely(instance_autoload_members_kw(tp_self,
	                                         desc,
	                                         instance,
	                                         self,
	                                         argc,
	                                         argv,
	                                         (DeeKwdsObject *)kw))
		goto err_members;
	Dee_Decref(func);
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
	Dee_Decref(func);
err:
	return -1;
}

INTERN int DCALL
instance_builtin_auto_nobase_tinit(DeeTypeObject *__restrict tp_self,
                                   DeeObject *__restrict self,
                                   size_t argc, DeeObject **__restrict argv) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	/* Auto-initialize members. */
	if unlikely(instance_autoload_members(tp_self,
	                                      desc,
	                                      instance,
	                                      self,
	                                      argc,
	                                      argv))
		goto err_members;
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
/*err:*/
	return -1;
}

INTERN int DCALL
instance_builtin_auto_nobase_tinitkw(DeeTypeObject *__restrict tp_self,
                                     DeeObject *__restrict self, size_t argc,
                                     DeeObject **__restrict argv, DeeObject *kw) {
	struct class_desc *desc        = DeeClass_DESC(tp_self);
	struct instance_desc *instance = DeeInstance_DESC(desc, self);
	/* Default-initialize the members of this instance. */
	rwlock_init(&instance->id_lock);
	MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
	/* Auto-initialize members. */
	if unlikely(instance_autoload_members_kw(tp_self,
		                                      desc,
		                                      instance,
		                                      self,
		                                      argc,
		                                      argv,
		                                      (DeeKwdsObject *)kw))
	goto err_members;
	return 0;
err_members:
	instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
/*err:*/
	return -1;
}

INTERN int DCALL
instance_auto_nobase_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
	return instance_auto_nobase_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_auto_nobase_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	return instance_auto_nobase_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN int DCALL
instance_builtin_auto_nobase_init(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv) {
	return instance_builtin_auto_nobase_tinit(Dee_TYPE(self), self, argc, argv);
}

INTERN int DCALL
instance_builtin_auto_nobase_initkw(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	return instance_builtin_auto_nobase_tinitkw(Dee_TYPE(self), self, argc, argv, kw);
}
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
#endif /* CLASS_TP_FAUTOINIT */








/* Builtin hash & comparison support. */
INTERN dhash_t DCALL
instance_builtin_thash(DeeTypeObject *__restrict tp_self,
                       DeeObject *__restrict self) {
	struct class_desc *desc;
	uint16_t i;
	struct instance_desc *instance;
	DREF DeeObject *member;
	dhash_t result = 0;
	desc           = DeeClass_DESC(tp_self);
	instance       = DeeInstance_DESC(desc, self);
	rwlock_read(&instance->id_lock);
	for (i = 0; i < desc->cd_desc->cd_imemb_size; ++i) {
		member = instance->id_vtab[i];
		if (!member)
			continue;
		Dee_Incref(member);
		rwlock_endread(&instance->id_lock);
		result ^= DeeObject_Hash(member);
		Dee_Decref(member);
		rwlock_read(&instance->id_lock);
	}
	rwlock_endread(&instance->id_lock);
	return result;
}




INTERN int DCALL
impl_instance_builtin_eq(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeObject *__restrict other) {
	struct instance_desc *instance, *other_instance;
	struct class_desc *desc;
	uint16_t i, size;
	int temp;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	desc           = DeeClass_DESC(tp_self);
	instance       = DeeInstance_DESC(desc, self);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	rwlock_read(&instance->id_lock);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *lhs_val;
		DREF DeeObject *rhs_val;
		lhs_val = instance->id_vtab[i];
		rhs_val = other_instance->id_vtab[i];
		if (lhs_val != rhs_val) {
			if (!lhs_val || !rhs_val) {
				rwlock_endread(&instance->id_lock);
				return 0; /* Different NULL values. */
			}
			Dee_Incref(lhs_val);
			Dee_Incref(rhs_val);
			rwlock_endread(&instance->id_lock);
			/* Compare the two members. */
			temp = DeeObject_CompareEq(lhs_val, rhs_val);
			Dee_Decref(rhs_val);
			Dee_Decref(lhs_val);
			if (temp <= 0)
				return temp; /* Error, or non-equal */
		}
	}
	rwlock_endread(&instance->id_lock);
	return 1; /* All elements are equal */
}

INTERN int DCALL
impl_instance_builtin_lo(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeObject *__restrict other) {
	struct instance_desc *instance, *other_instance;
	struct class_desc *desc;
	uint16_t i, size;
	int temp;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	desc           = DeeClass_DESC(tp_self);
	instance       = DeeInstance_DESC(desc, self);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	rwlock_read(&instance->id_lock);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *lhs_val;
		DREF DeeObject *rhs_val;
		lhs_val = instance->id_vtab[i];
		rhs_val = other_instance->id_vtab[i];
		if (lhs_val != rhs_val) {
			if (!lhs_val || !rhs_val) {
				rwlock_endread(&instance->id_lock);
				/* Different NULL values. */
				return lhs_val ? 0 : /* NON_NULL < *    --> false */
				       rhs_val ? 1 : /* NULL < NON_NULL --> true */
				       0;            /* NULL < NULL     --> false */
			}
			Dee_Incref(lhs_val);
			Dee_Incref(rhs_val);
			rwlock_endread(&instance->id_lock);
			/* Compare the two members. */
			temp = DeeObject_CompareLo(lhs_val, rhs_val);
			if (temp != 0) {
				Dee_Decref(rhs_val);
				Dee_Decref(lhs_val);
				return temp; /* Error, or lower */
			}
			temp = DeeObject_CompareEq(lhs_val, rhs_val);
			Dee_Decref(rhs_val);
			Dee_Decref(lhs_val);
			if (temp <= 0)
				return temp; /* Error, or non-qual */
		}
	}
	rwlock_endread(&instance->id_lock);
	return 0; /* All elements are equal */
}

INTERN int DCALL
impl_instance_builtin_le(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeObject *__restrict other) {
	struct instance_desc *instance, *other_instance;
	struct class_desc *desc;
	uint16_t i, size;
	int temp;
	ASSERT(DeeObject_InstanceOf(other, tp_self));
	desc           = DeeClass_DESC(tp_self);
	instance       = DeeInstance_DESC(desc, self);
	other_instance = DeeInstance_DESC(desc, other);
	size           = desc->cd_desc->cd_imemb_size;
	rwlock_read(&instance->id_lock);
	for (i = 0; i < size; ++i) {
		DREF DeeObject *lhs_val;
		DREF DeeObject *rhs_val;
		lhs_val = instance->id_vtab[i];
		rhs_val = other_instance->id_vtab[i];
		if (lhs_val != rhs_val) {
			size_t j;
			if (!lhs_val || !rhs_val) {
				rwlock_endread(&instance->id_lock);
				/* Different NULL values. */
				return !lhs_val ? 1 : /* NULL <= *        --> true */
				       0;             /* NON_NULL <= NULL --> false */
			}
			Dee_Incref(lhs_val);
			Dee_Incref(rhs_val);
			/* Check if this is the last member. */
			for (j = i; j < size; ++j) {
				if (instance->id_vtab[j] ||
				    other_instance->id_vtab[j])
					goto non_last_member;
			}
			/* Last member! */
			rwlock_endread(&instance->id_lock);
			temp = DeeObject_CompareLe(lhs_val, rhs_val);
			Dee_Decref(rhs_val);
			Dee_Decref(lhs_val);
			return temp;
non_last_member:
			rwlock_endread(&instance->id_lock);
			/* Compare the two members. */
			temp = DeeObject_CompareLo(lhs_val, rhs_val);
			if (temp != 0) {
				Dee_Decref(rhs_val);
				Dee_Decref(lhs_val);
				return temp; /* Error, or lower */
			}
			temp = DeeObject_CompareEq(lhs_val, rhs_val);
			Dee_Decref(rhs_val);
			Dee_Decref(lhs_val);
			if (temp <= 0)
				return temp; /* Error, or non-qual */
		}
	}
	rwlock_endread(&instance->id_lock);
	return 1; /* All elements are equal */
}

INTERN int DCALL
impl_instance_builtin_ne(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeObject *__restrict other) {
	int result = impl_instance_builtin_eq(tp_self, self, other);
	if (result >= 0)
		result = !result;
	return result;
}

INTERN int DCALL
impl_instance_builtin_gr(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeObject *__restrict other) {
	int result = impl_instance_builtin_le(tp_self, self, other);
	if (result >= 0)
		result = !result;
	return result;
}

INTERN int DCALL
impl_instance_builtin_ge(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeObject *__restrict other) {
	int result = impl_instance_builtin_lo(tp_self, self, other);
	if (result >= 0)
		result = !result;
	return result;
}


INTDEF dhash_t DCALL DeeObject_THash(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);
INTDEF DREF DeeObject *DCALL DeeObject_TCompareEqObject(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeObject_TCompareNeObject(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeObject_TCompareLoObject(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeObject_TCompareLeObject(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeObject_TCompareGrObject(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF DREF DeeObject *DCALL DeeObject_TCompareGeObject(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);

INTERN DREF DeeObject *DCALL
instance_builtin_teq(DeeTypeObject *__restrict tp_self,
                     DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	DREF DeeObject *result;
	int temp;
	/* Make sure that `other' is an instance of `self' */
	if (DeeObject_AssertType(other, tp_self))
		goto err;
	/* Compare the underlying objects. */
	if (DeeType_Base(tp_self)) {
		result = DeeObject_TCompareEqObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			goto nope;
	}
	temp = impl_instance_builtin_eq(tp_self, self, other);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return_true;
nope:
	return_false;
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_builtin_tne(DeeTypeObject *__restrict tp_self,
                     DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	DREF DeeObject *result;
	int temp;
	/* Make sure that `other' is an instance of `self' */
	if (DeeObject_AssertType(other, tp_self))
		goto err;
	/* Compare the underlying objects. */
	if (DeeType_Base(tp_self)) {
		result = DeeObject_TCompareNeObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			goto nope;
	}
	temp = impl_instance_builtin_ne(tp_self, self, other);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return_true;
nope:
	return_false;
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_builtin_tlo(DeeTypeObject *__restrict tp_self,
                     DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	DREF DeeObject *result;
	int temp;
	if (DeeObject_AssertType(other, tp_self))
		goto err;
	/* BASE < OTHER || (BASE == OTHER && SELF < OTHER) */
	if (DeeType_Base(tp_self)) {
		/* Compare the underlying objects. */
		result = DeeObject_TCompareLoObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (temp)
			return_true;
		result = DeeObject_TCompareEqObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			goto nope;
	}
	temp = impl_instance_builtin_lo(tp_self, self, other);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return_true;
nope:
	return_false;
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_builtin_tle(DeeTypeObject *__restrict tp_self,
                     DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	DREF DeeObject *result;
	int temp;
	if (DeeObject_AssertType(other, tp_self))
		goto err;
	/* BASE < OTHER || (BASE == OTHER && SELF <= OTHER) */
	if (DeeType_Base(tp_self)) {
		/* Compare the underlying objects. */
		result = DeeObject_TCompareLoObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (temp)
			return_true;
		result = DeeObject_TCompareEqObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			goto nope;
	}
	temp = impl_instance_builtin_le(tp_self, self, other);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return_true;
nope:
	return_false;
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_builtin_tgr(DeeTypeObject *__restrict tp_self,
                     DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	DREF DeeObject *result;
	int temp;
	if (DeeObject_AssertType(other, tp_self))
		goto err;
	/* BASE > OTHER || (BASE == OTHER && SELF > OTHER) */
	if (DeeType_Base(tp_self)) {
		/* Compare the underlying objects. */
		result = DeeObject_TCompareGrObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (temp)
			return_true;
		result = DeeObject_TCompareEqObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			goto nope;
	}
	temp = impl_instance_builtin_gr(tp_self, self, other);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return_true;
nope:
	return_false;
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_builtin_tge(DeeTypeObject *__restrict tp_self,
                     DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	DREF DeeObject *result;
	int temp;
	if (DeeObject_AssertType(other, tp_self))
		goto err;
	/* BASE > OTHER || (BASE == OTHER && SELF >= OTHER) */
	if (DeeType_Base(tp_self)) {
		/* Compare the underlying objects. */
		result = DeeObject_TCompareGrObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (temp)
			return_true;
		result = DeeObject_TCompareEqObject(DeeType_Base(tp_self), self, other);
		if unlikely(!result)
			goto err;
		temp = DeeObject_Bool(result);
		Dee_Decref(result);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			goto nope;
	}
	temp = impl_instance_builtin_ge(tp_self, self, other);
	if unlikely(temp < 0)
		goto err;
	if (temp)
		return_true;
nope:
	return_false;
err:
	return NULL;
}

INTERN dhash_t DCALL
instance_builtin_hash(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	dhash_t result         = 0;
	do {
		result ^= instance_builtin_thash(tp_self, self);
	} while ((tp_self = DeeType_Base(tp_self)) != NULL &&
	         DeeType_IsClass(tp_self));
	if (tp_self)
		result ^= DeeObject_THash(tp_self, self);
	return result;
}

INTERN DREF DeeObject *DCALL
instance_builtin_eq(DeeObject *__restrict self, DeeObject *__restrict other) {
	return instance_builtin_teq(Dee_TYPE(self), self, other);
}

INTERN DREF DeeObject *DCALL
instance_builtin_ne(DeeObject *__restrict self, DeeObject *__restrict other) {
	return instance_builtin_tne(Dee_TYPE(self), self, other);
}

INTERN DREF DeeObject *DCALL
instance_builtin_lo(DeeObject *__restrict self, DeeObject *__restrict other) {
	return instance_builtin_tlo(Dee_TYPE(self), self, other);
}

INTERN DREF DeeObject *DCALL
instance_builtin_le(DeeObject *__restrict self, DeeObject *__restrict other) {
	return instance_builtin_tle(Dee_TYPE(self), self, other);
}

INTERN DREF DeeObject *DCALL
instance_builtin_gr(DeeObject *__restrict self, DeeObject *__restrict other) {
	return instance_builtin_tgr(Dee_TYPE(self), self, other);
}

INTERN DREF DeeObject *DCALL
instance_builtin_ge(DeeObject *__restrict self, DeeObject *__restrict other) {
	return instance_builtin_tge(Dee_TYPE(self), self, other);
}

INTERN struct type_cmp instance_builtin_cmp = {
	/* .tp_hash = */ &instance_builtin_hash,
	/* .tp_eq   = */ &instance_builtin_eq,
	/* .tp_ne   = */ &instance_builtin_ne,
	/* .tp_lo   = */ &instance_builtin_lo,
	/* .tp_le   = */ &instance_builtin_le,
	/* .tp_gr   = */ &instance_builtin_gr,
	/* .tp_ge   = */ &instance_builtin_ge
};



INTERN DREF DeeObject *DCALL
instance_tcall(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, OPERATOR_CALL);
	if unlikely(!func)
		return NULL;
	result = DeeObject_ThisCall(func, self, argc, argv);
	Dee_Decref(func);
	return result;
}

INTERN DREF DeeObject *DCALL
instance_tcallkw(DeeTypeObject *__restrict tp_self,
                 DeeObject *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, OPERATOR_CALL);
	if unlikely(!func)
		return NULL;
	result = DeeObject_ThisCallKw(func, self, argc, argv, kw);
	Dee_Decref(func);
	return result;
}

INTERN DREF DeeObject *DCALL
instance_call(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	return instance_tcall(Dee_TYPE(self), self, argc, argv);
}

INTERN DREF DeeObject *DCALL
instance_callkw(DeeObject *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	return instance_tcallkw(Dee_TYPE(self), self, argc, argv, kw);
}

INTERN dssize_t DCALL
instance_enumattr(DeeTypeObject *__restrict tp_self,
                  DeeObject *__restrict self,
                  denum_t proc, void *arg) {
	/* Hook function for user-defined enumattr() callbacks! */
	(void)tp_self;
	(void)self;
	(void)proc;
	(void)arg;
	DERROR_NOTIMPLEMENTED();
	return -1;
}

#define DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_txxx, instance_xxx, op) \
	INTERN DREF DeeObject *DCALL                                                \
	instance_txxx(DeeTypeObject *__restrict tp_self,                            \
	              DeeObject *__restrict self) {                                 \
		DREF DeeObject *func, *result;                                          \
		func = DeeClass_GetOperator(tp_self, op);                               \
		if                                                                      \
			unlikely(!func)                                                     \
		return NULL;                                                            \
		result = DeeObject_ThisCall(func, self, 0, NULL);                       \
		Dee_Decref(func);                                                       \
		return result;                                                          \
	}                                                                           \
	INTERN DREF DeeObject *DCALL                                                \
	instance_xxx(DeeObject *__restrict self) {                                  \
		return instance_txxx(Dee_TYPE(self), self);                             \
	}
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_tinv, instance_inv, OPERATOR_INV)
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_tpos, instance_pos, OPERATOR_POS)
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_tneg, instance_neg, OPERATOR_NEG)
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_titer, instance_iter, OPERATOR_ITERSELF)
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION(instance_tsize, instance_size, OPERATOR_SIZE)
#undef DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION

INTERN DREF DeeObject *DCALL
instance_tnext(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, OPERATOR_ITERNEXT);
	if unlikely(!func)
		return NULL;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
	if (!result && DeeError_Catch(&DeeError_StopIteration))
		result = ITER_DONE;
	return result;
}

INTERN DREF DeeObject *DCALL
instance_next(DeeObject *__restrict self) {
	return instance_tnext(Dee_TYPE(self), self);
}


#define DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_txxx, instance_xxx, op) \
	INTERN DREF DeeObject *DCALL                                                 \
	instance_txxx(DeeTypeObject *__restrict tp_self,                             \
	              DeeObject *__restrict self,                                    \
	              DeeObject *__restrict other) {                                 \
		DREF DeeObject *func, *result;                                           \
		func = DeeClass_GetOperator(tp_self, op);                                \
		if                                                                       \
			unlikely(!func)                                                      \
		goto err;                                                                \
		result = DeeObject_ThisCall(func, self, 1, (DeeObject **)&other);        \
		Dee_Decref(func);                                                        \
		return result;                                                           \
	err:                                                                         \
		return NULL;                                                             \
	}                                                                            \
	INTERN DREF DeeObject *DCALL                                                 \
	instance_xxx(DeeObject *__restrict self, DeeObject *__restrict other) {      \
		return instance_txxx(Dee_TYPE(self), self, other);                       \
	}
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tadd, instance_add, OPERATOR_ADD)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tsub, instance_sub, OPERATOR_SUB)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tmul, instance_mul, OPERATOR_MUL)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tdiv, instance_div, OPERATOR_DIV)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tmod, instance_mod, OPERATOR_MOD)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tshl, instance_shl, OPERATOR_SHL)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tshr, instance_shr, OPERATOR_SHR)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tand, instance_and, OPERATOR_AND)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tor, instance_or, OPERATOR_OR)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_txor, instance_xor, OPERATOR_XOR)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tpow, instance_pow, OPERATOR_POW)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_teq, instance_eq, OPERATOR_EQ)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tne, instance_ne, OPERATOR_NE)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tlo, instance_lo, OPERATOR_LO)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tle, instance_le, OPERATOR_LE)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tgr, instance_gr, OPERATOR_GR)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tge, instance_ge, OPERATOR_GE)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tcontains, instance_contains, OPERATOR_CONTAINS)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tgetitem, instance_getitem, OPERATOR_GETITEM)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION(instance_tgetattr, instance_getattr, OPERATOR_GETATTR)
#undef DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION

#define DEFINE_TRINARY_INSTANCE_WRAPPER_FUNCTION(instance_txxx, instance_xxx, op)                         \
	INTERN DREF DeeObject *DCALL                                                                          \
	instance_txxx(DeeTypeObject *__restrict tp_self,                                                      \
	              DeeObject *__restrict self,                                                             \
	              DeeObject *__restrict other,                                                            \
	              DeeObject *__restrict other2) {                                                         \
		DREF DeeObject *func, *result;                                                                    \
		DeeObject *argv[2];                                                                               \
		func = DeeClass_GetOperator(tp_self, op);                                                         \
		if                                                                                                \
			unlikely(!func)                                                                               \
		return NULL;                                                                                      \
		argv[0] = other;                                                                                  \
		argv[1] = other2;                                                                                 \
		result  = DeeObject_ThisCall(func, self, 2, argv);                                                \
		Dee_Decref(func);                                                                                 \
		return result;                                                                                    \
	}                                                                                                     \
	INTERN DREF DeeObject *DCALL                                                                          \
	instance_xxx(DeeObject *__restrict self, DeeObject *__restrict other, DeeObject *__restrict other2) { \
		return instance_txxx(Dee_TYPE(self), self, other, other2);                                        \
	}
DEFINE_TRINARY_INSTANCE_WRAPPER_FUNCTION(instance_tgetrange, instance_getrange, OPERATOR_GETRANGE)
#undef DEFINE_TRINARY_INSTANCE_WRAPPER_FUNCTION

#define DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_txxx, instance_xxx, op) \
	INTERN int DCALL                                                                \
	instance_txxx(DeeTypeObject *__restrict tp_self,                                \
	              DeeObject *__restrict self) {                                     \
		DeeObject *func, *result;                                                   \
		func = DeeClass_GetOperator(tp_self, op);                                   \
		if                                                                          \
			unlikely(!func)                                                         \
		goto err;                                                                   \
		result = DeeObject_ThisCall(func, self, 0, NULL);                           \
		Dee_Decref(func);                                                           \
		if                                                                          \
			unlikely(!result)                                                       \
		goto err;                                                                   \
		Dee_Decref(result);                                                         \
		return 0;                                                                   \
	err:                                                                            \
		return -1;                                                                  \
	}                                                                               \
	INTERN int DCALL                                                                \
	instance_xxx(DeeObject *__restrict self) {                                      \
		return instance_txxx(Dee_TYPE(self), self);                                 \
	}
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tenter, instance_enter, OPERATOR_ENTER)
DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tleave, instance_leave, OPERATOR_LEAVE)
#undef DEFINE_UNARY_INSTANCE_WRAPPER_FUNCTION_INT

#define DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_txxx, instance_xxx, op) \
	INTERN int DCALL                                                                 \
	instance_txxx(DeeTypeObject *__restrict tp_self,                                 \
	              DeeObject *__restrict self,                                        \
	              DeeObject *__restrict other) {                                     \
		DeeObject *func, *result;                                                    \
		func = DeeClass_GetOperator(tp_self, op);                                    \
		if                                                                           \
			unlikely(!func)                                                          \
		goto err;                                                                    \
		result = DeeObject_ThisCall(func, self, 1, (DeeObject **)&other);            \
		Dee_Decref(func);                                                            \
		if                                                                           \
			unlikely(!result)                                                        \
		goto err;                                                                    \
		Dee_Decref(result);                                                          \
		return 0;                                                                    \
	err:                                                                             \
		return -1;                                                                   \
	}                                                                                \
	INTERN int DCALL                                                                 \
	instance_xxx(DeeObject *__restrict self,                                         \
	             DeeObject *__restrict other) {                                      \
		return instance_txxx(Dee_TYPE(self), self, other);                           \
	}
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tassign, instance_assign, OPERATOR_ASSIGN)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tmoveassign, instance_moveassign, OPERATOR_MOVEASSIGN)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tdelitem, instance_delitem, OPERATOR_DELITEM)
DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tdelattr, instance_delattr, OPERATOR_DELATTR)
#undef DEFINE_BINARY_INSTANCE_WRAPPER_FUNCTION_INT

#define DEFINE_TRINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_txxx, instance_xxx, op)                     \
	INTERN int DCALL                                                                                      \
	instance_txxx(DeeTypeObject *__restrict tp_self,                                                      \
	              DeeObject *__restrict self,                                                             \
	              DeeObject *__restrict other,                                                            \
	              DeeObject *__restrict other2) {                                                         \
		DREF DeeObject *func, *result;                                                                    \
		DeeObject *argv[2];                                                                               \
		func = DeeClass_GetOperator(tp_self, op);                                                         \
		if                                                                                                \
			unlikely(!func)                                                                               \
		goto err;                                                                                         \
		argv[0] = other;                                                                                  \
		argv[1] = other2;                                                                                 \
		result  = DeeObject_ThisCall(func, self, 2, argv);                                                \
		Dee_Decref(func);                                                                                 \
		if                                                                                                \
			unlikely(!result)                                                                             \
		goto err;                                                                                         \
		Dee_Decref(result);                                                                               \
		return 0;                                                                                         \
	err:                                                                                                  \
		return -1;                                                                                        \
	}                                                                                                     \
	INTERN int DCALL                                                                                      \
	instance_xxx(DeeObject *__restrict self, DeeObject *__restrict other, DeeObject *__restrict other2) { \
		return instance_txxx(Dee_TYPE(self), self, other, other2);                                        \
	}
DEFINE_TRINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tsetitem, instance_setitem, OPERATOR_SETITEM)
DEFINE_TRINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tdelrange, instance_delrange, OPERATOR_DELRANGE)
DEFINE_TRINARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tsetattr, instance_setattr, OPERATOR_SETATTR)
#undef DEFINE_TRINARY_INSTANCE_WRAPPER_FUNCTION_INT

#define DEFINE_QUADARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_txxx, instance_xxx, op) \
	INTERN int DCALL                                                                  \
	instance_txxx(DeeTypeObject *__restrict tp_self,                                  \
	              DeeObject *__restrict self,                                         \
	              DeeObject *__restrict other,                                        \
	              DeeObject *__restrict other2,                                       \
	              DeeObject *__restrict other3) {                                     \
		DREF DeeObject *func, *result;                                                \
		DeeObject *argv[3];                                                           \
		func = DeeClass_GetOperator(tp_self, op);                                     \
		if                                                                            \
			unlikely(!func)                                                           \
		goto err;                                                                     \
		argv[0] = other;                                                              \
		argv[1] = other2;                                                             \
		argv[2] = other3;                                                             \
		result  = DeeObject_ThisCall(func, self, 3, argv);                            \
		Dee_Decref(func);                                                             \
		if                                                                            \
			unlikely(!result)                                                         \
		goto err;                                                                     \
		Dee_Decref(result);                                                           \
		return 0;                                                                     \
	err:                                                                              \
		return -1;                                                                    \
	}                                                                                 \
	INTERN int DCALL                                                                  \
	instance_xxx(DeeObject *__restrict self,                                          \
	             DeeObject *__restrict other,                                         \
	             DeeObject *__restrict other2,                                        \
	             DeeObject *__restrict other3) {                                      \
		return instance_txxx(Dee_TYPE(self), self, other, other2, other3);            \
	}
DEFINE_QUADARY_INSTANCE_WRAPPER_FUNCTION_INT(instance_tsetrange, instance_setrange, OPERATOR_SETRANGE)
#undef DEFINE_QUADARY_INSTANCE_WRAPPER_FUNCTION_INT

#define DEFINE_UNARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_txxx, instance_xxx, op) \
	INTERN int DCALL                                                                    \
	instance_txxx(DeeTypeObject *__restrict tp_self,                                    \
	              DeeObject **__restrict pself) {                                       \
		DREF DeeObject *func, *result;                                                  \
		func = DeeClass_GetOperator(tp_self, op);                                       \
		if                                                                              \
			unlikely(!func)                                                             \
		goto err;                                                                       \
		result = DeeObject_ThisCall(func, *pself, 0, NULL);                             \
		Dee_Decref(func);                                                               \
		if                                                                              \
			unlikely(!result)                                                           \
		goto err;                                                                       \
		Dee_Decref(*pself);                                                             \
		*pself = result;                                                                \
		return 0;                                                                       \
	err:                                                                                \
		return -1;                                                                      \
	}                                                                                   \
	INTERN int DCALL                                                                    \
	instance_xxx(DeeObject **__restrict pself) {                                        \
		return instance_txxx(Dee_TYPE(*pself), pself);                                  \
	}
DEFINE_UNARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tinc, instance_inc, OPERATOR_INC)
DEFINE_UNARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tdec, instance_dec, OPERATOR_DEC)
#undef DEFINE_UNARY_INPLACE_INSTANCE_WRAPPER_FUNCTION

#define DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_txxx, instance_xxx, op) \
	INTERN int DCALL                                                                     \
	instance_txxx(DeeTypeObject *__restrict tp_self,                                     \
	              DeeObject **__restrict pself,                                          \
	              DeeObject *__restrict other) {                                         \
		DREF DeeObject *func, *result;                                                   \
		func = DeeClass_GetOperator(tp_self, op);                                        \
		if                                                                               \
			unlikely(!func)                                                              \
		goto err;                                                                        \
		result = DeeObject_ThisCall(func, *pself, 1, (DeeObject **)&other);              \
		Dee_Decref(func);                                                                \
		if                                                                               \
			unlikely(!result)                                                            \
		goto err;                                                                        \
		Dee_Decref(*pself);                                                              \
		*pself = result;                                                                 \
		return 0;                                                                        \
	err:                                                                                 \
		return -1;                                                                       \
	}                                                                                    \
	INTERN int DCALL                                                                     \
	instance_xxx(DeeObject **__restrict pself, DeeObject *__restrict other) {            \
		return instance_txxx(Dee_TYPE(*pself), pself, other);                            \
	}
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tiadd, instance_iadd, OPERATOR_ADD)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tisub, instance_isub, OPERATOR_SUB)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_timul, instance_imul, OPERATOR_MUL)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tidiv, instance_idiv, OPERATOR_DIV)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_timod, instance_imod, OPERATOR_MOD)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tishl, instance_ishl, OPERATOR_SHL)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tishr, instance_ishr, OPERATOR_SHR)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tiand, instance_iand, OPERATOR_AND)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tior, instance_ior, OPERATOR_OR)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tixor, instance_ixor, OPERATOR_XOR)
DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION(instance_tipow, instance_ipow, OPERATOR_POW)
#undef DEFINE_BINARY_INPLACE_INSTANCE_WRAPPER_FUNCTION


INTERN DREF DeeObject *DCALL
instance_tstr(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, OPERATOR_STR);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
	if (likely(result) &&
	    DeeObject_AssertTypeExact(result, &DeeString_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_str(DeeObject *__restrict self) {
	return instance_tstr(Dee_TYPE(self), self);
}

INTERN DREF DeeObject *DCALL
instance_trepr(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, OPERATOR_REPR);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
	if (likely(result) &&
	    DeeObject_AssertTypeExact(result, &DeeString_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_repr(DeeObject *__restrict self) {
	return instance_trepr(Dee_TYPE(self), self);
}

INTERN DREF DeeObject *DCALL
instance_tint(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self) {
	DREF DeeObject *func, *result;
	func = DeeClass_GetOperator(tp_self, OPERATOR_INT);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
	if (likely(result) && DeeObject_AssertTypeExact(result, &DeeInt_Type))
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
instance_int(DeeObject *__restrict self) {
	return instance_tint(Dee_TYPE(self), self);
}

INTERN int DCALL
instance_tbool(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self) {
	DREF DeeObject *func, *result;
	int retval;
	func = DeeClass_GetOperator(tp_self, OPERATOR_BOOL);
	if unlikely(!func)
		goto err;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
	if unlikely(!result)
		goto err;
	if (DeeObject_AssertTypeExact(result, &DeeBool_Type))
		goto err;
	retval = DeeBool_IsTrue(result);
	Dee_Decref(result);
	return retval;
err:
	return -1;
}

INTERN int DCALL
instance_bool(DeeObject *__restrict self) {
	return instance_tbool(Dee_TYPE(self), self);
}


INTERN int DCALL
instance_tint32(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self,
                int32_t *__restrict result) {
	DREF DeeObject *intval;
	int error;
	intval = instance_tint(tp_self, self);
	if unlikely(!intval)
		return -1;
	error = DeeInt_As32(intval, result);
	Dee_Decref(intval);
	return error;
}

INTERN int DCALL
instance_tint64(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self,
                int64_t *__restrict result) {
	DREF DeeObject *intval;
	int error;
	intval = instance_tint(tp_self, self);
	if unlikely(!intval)
		return -1;
	error = DeeInt_As64(intval, result);
	Dee_Decref(intval);
	return error;
}

INTERN int DCALL
instance_tdouble(DeeTypeObject *__restrict tp_self,
                 DeeObject *__restrict self,
                 double *__restrict result) {
	DREF DeeObject *func, *value;
	func = DeeClass_GetOperator(tp_self, OPERATOR_FLOAT);
	if unlikely(!func)
		goto err;
	value = DeeObject_ThisCall(func, self, 0, NULL);
	if (likely(value) && DeeObject_AssertTypeExact(value, &DeeFloat_Type))
		goto err_r;
	*result = DeeFloat_VALUE(value);
	Dee_Decref(value);
	return 0;
err_r:
	Dee_Decref(value);
err:
	return -1;
}

INTERN int DCALL
instance_int32(DeeObject *__restrict self,
               int32_t *__restrict result) {
	return instance_tint32(Dee_TYPE(self), self, result);
}

INTERN int DCALL
instance_int64(DeeObject *__restrict self,
               int64_t *__restrict result) {
	return instance_tint64(Dee_TYPE(self), self, result);
}

INTERN int DCALL
instance_double(DeeObject *__restrict self,
                double *__restrict result) {
	return instance_tdouble(Dee_TYPE(self), self, result);
}

INTERN dhash_t DCALL
instance_thash(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self) {
	DREF DeeObject *func, *result;
	dhash_t result_value;
	int temp;
	func = DeeClass_TryGetOperator(tp_self, OPERATOR_HASH);
	if unlikely(!func)
		goto fallback;
	result = DeeObject_ThisCall(func, self, 0, NULL);
	Dee_Decref(func);
	if unlikely(!result)
		goto fallback_handled;
	temp = DeeObject_AsUIntptr(result, &result_value);
	Dee_Decref(result);
	if unlikely(temp)
		goto fallback_handled;
	return result_value;
fallback_handled:
	DeeError_Print("Unhandled error in `operator hash'\n",
	               ERROR_PRINT_DOHANDLE);
fallback:
	return DeeObject_HashGeneric(self);
}

INTERN dhash_t DCALL
instance_hash(DeeObject *__restrict self) {
	return instance_thash(Dee_TYPE(self), self);
}



/* GC support for class objects. */
INTERN void DCALL
instance_tvisit(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self,
                dvisit_t proc, void *arg) {
	struct class_desc *desc;
	uint16_t i;
	struct instance_desc *instance;
	desc     = DeeClass_DESC(tp_self);
	instance = DeeInstance_DESC(desc, self);
	rwlock_read(&instance->id_lock);
	for (i = 0; i < desc->cd_desc->cd_imemb_size; ++i)
		Dee_XVisit(instance->id_vtab[i]);
	rwlock_endread(&instance->id_lock);
}

INTERN void DCALL
instance_tclear(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self) {
	struct class_desc *desc;
	uint16_t i;
	struct instance_desc *instance;
	DREF DeeObject *buffer[64];
	size_t buflen;
	desc     = DeeClass_DESC(tp_self);
	instance = DeeInstance_DESC(desc, self);
	buflen   = 0;
	rwlock_write(&instance->id_lock);
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
			rwlock_endwrite(&instance->id_lock);
			while (buflen) {
				--buflen;
				Dee_Decref(buffer[buflen]);
			}
			rwlock_write(&instance->id_lock);
			goto again_i;
		}
		buffer[buflen++]     = instance->id_vtab[i]; /* Steal reference. */
		instance->id_vtab[i] = NULL;
	}
	rwlock_endwrite(&instance->id_lock);
	while (buflen) {
		--buflen;
		Dee_Decref(buffer[buflen]);
	}
}

INTERN void DCALL
instance_tpclear(DeeTypeObject *__restrict tp_self,
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
	rwlock_write(&instance->id_lock);
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
			rwlock_endwrite(&instance->id_lock);
			while (buflen) {
				--buflen;
				Dee_Decref(buffer[buflen]);
			}
			rwlock_write(&instance->id_lock);
			goto again_i;
		}
		buffer[buflen++]     = instance->id_vtab[i]; /* Steal this reference. */
		instance->id_vtab[i] = NULL;
	}
	rwlock_endwrite(&instance->id_lock);
	while (buflen) {
		--buflen;
		Dee_Decref(buffer[buflen]);
	}
}

INTERN void DCALL
instance_visit(DeeObject *__restrict self,
               dvisit_t proc, void *arg) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		instance_tvisit(tp_self, self, proc, arg);
	} while ((tp_self = DeeType_Base(tp_self)) != NULL &&
	         DeeType_IsClass(tp_self));
}

INTERN void DCALL
instance_clear(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		instance_tclear(tp_self, self);
	} while ((tp_self = DeeType_Base(tp_self)) != NULL &&
	         DeeType_IsClass(tp_self));
}

INTERN void DCALL
instance_pclear(DeeObject *__restrict self,
                unsigned int gc_priority) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		instance_tpclear(tp_self, self, gc_priority);
	} while ((tp_self = DeeType_Base(tp_self)) != NULL &&
	         DeeType_IsClass(tp_self));
}

INTERN struct type_gc instance_gc = {
	/* .tp_clear  = */ &instance_clear,
	/* .tp_pclear = */ &instance_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_INSTANCE
};



/* Binding descriptors for standard operators provided by DeeType_Type */
struct opwrapper {
	uintptr_t offset;  /* Offset from the containing operator table to where `wrapper' must be written. */
	void     *wrapper; /* [0..1] The C wrapper function that should be assigned to
	                    *         the operator to have it invoke user-callbacks.
	                    *   NOTE: A value of NULL in this field acts as a sentinel. */
};

/* Operator wrapper descriptor tables. */
PRIVATE struct opwrapper type_wrappers[] = {
	/* [OPERATOR_COPY - OPERATOR_COPY]        = */ { offsetof(DeeTypeObject, tp_init.tp_alloc.tp_copy_ctor), (void *)&instance_copy },
	/* [OPERATOR_DEEPCOPY - OPERATOR_COPY]    = */ { offsetof(DeeTypeObject, tp_init.tp_alloc.tp_deep_ctor), (void *)&instance_deepcopy },
	/* [OPERATOR_DESTRUCTOR - OPERATOR_COPY]  = */ { offsetof(DeeTypeObject, tp_init.tp_dtor), &instance_destructor },
	/* [OPERATOR_ASSIGN - OPERATOR_COPY]      = */ { 0, NULL /*offsetof(DeeTypeObject,tp_init.tp_assign), (void *)&instance_assign*/ },
	/* [OPERATOR_MOVEASSIGN - OPERATOR_COPY]  = */ { 0, NULL /*offsetof(DeeTypeObject,tp_init.tp_move_assign), (void *)&instance_moveassign*/ },
	/* [OPERATOR_STR - OPERATOR_COPY]         = */ { offsetof(DeeTypeObject, tp_cast.tp_str), (void *)&instance_str },
	/* [OPERATOR_REPR - OPERATOR_COPY]        = */ { offsetof(DeeTypeObject, tp_cast.tp_repr), (void *)&instance_repr },
	/* [OPERATOR_BOOL - OPERATOR_COPY]        = */ { offsetof(DeeTypeObject, tp_cast.tp_bool), (void *)&instance_bool },
	/* [OPERATOR_ITERNEXT - OPERATOR_COPY]    = */ { offsetof(DeeTypeObject, tp_iter_next), (void *)&instance_next }
};

PRIVATE struct opwrapper math_wrappers[] = {
	/* [OPERATOR_FLOAT - OPERATOR_FLOAT]       = */ { offsetof(struct type_math, tp_double), (void *)&instance_double },
	/* [OPERATOR_INV - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_inv), (void *)&instance_inv },
	/* [OPERATOR_POS - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_pos), (void *)&instance_pos },
	/* [OPERATOR_NEG - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_neg), (void *)&instance_neg },
	/* [OPERATOR_ADD - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_add), (void *)&instance_add },
	/* [OPERATOR_SUB - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_sub), (void *)&instance_sub },
	/* [OPERATOR_MUL - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_mul), (void *)&instance_mul },
	/* [OPERATOR_DIV - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_div), (void *)&instance_div },
	/* [OPERATOR_MOD - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_mod), (void *)&instance_mod },
	/* [OPERATOR_SHL - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_shl), (void *)&instance_shl },
	/* [OPERATOR_SHR - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_shr), (void *)&instance_shr },
	/* [OPERATOR_AND - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_and), (void *)&instance_and },
	/* [OPERATOR_OR - OPERATOR_FLOAT]          = */ { offsetof(struct type_math, tp_or), (void *)&instance_or },
	/* [OPERATOR_XOR - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_xor), (void *)&instance_xor },
	/* [OPERATOR_POW - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_pow), (void *)&instance_pow },
	/* [OPERATOR_INC - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_inc), (void *)&instance_inc },
	/* [OPERATOR_DEC - OPERATOR_FLOAT]         = */ { offsetof(struct type_math, tp_dec), (void *)&instance_dec },
	/* [OPERATOR_INPLACE_ADD - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_add), (void *)&instance_iadd },
	/* [OPERATOR_INPLACE_SUB - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_sub), (void *)&instance_isub },
	/* [OPERATOR_INPLACE_MUL - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_mul), (void *)&instance_imul },
	/* [OPERATOR_INPLACE_DIV - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_div), (void *)&instance_idiv },
	/* [OPERATOR_INPLACE_MOD - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_mod), (void *)&instance_imod },
	/* [OPERATOR_INPLACE_SHL - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_shl), (void *)&instance_ishl },
	/* [OPERATOR_INPLACE_SHR - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_shr), (void *)&instance_ishr },
	/* [OPERATOR_INPLACE_AND - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_and), (void *)&instance_iand },
	/* [OPERATOR_INPLACE_OR - OPERATOR_FLOAT]  = */ { offsetof(struct type_math, tp_inplace_or), (void *)&instance_ior },
	/* [OPERATOR_INPLACE_XOR - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_xor), (void *)&instance_ixor },
	/* [OPERATOR_INPLACE_POW - OPERATOR_FLOAT] = */ { offsetof(struct type_math, tp_inplace_pow), (void *)&instance_ipow }
};

PRIVATE struct opwrapper cmp_wrappers[] = {
	/* [OPERATOR_HASH - OPERATOR_CMPMIN] = */ { offsetof(struct type_cmp, tp_hash), (void *)&instance_hash },
	/* [OPERATOR_EQ - OPERATOR_CMPMIN]   = */ { offsetof(struct type_cmp, tp_eq), (void *)&instance_eq },
	/* [OPERATOR_NE - OPERATOR_CMPMIN]   = */ { offsetof(struct type_cmp, tp_ne), (void *)&instance_ne },
	/* [OPERATOR_LO - OPERATOR_CMPMIN]   = */ { offsetof(struct type_cmp, tp_lo), (void *)&instance_lo },
	/* [OPERATOR_LE - OPERATOR_CMPMIN]   = */ { offsetof(struct type_cmp, tp_le), (void *)&instance_le },
	/* [OPERATOR_GR - OPERATOR_CMPMIN]   = */ { offsetof(struct type_cmp, tp_gr), (void *)&instance_gr },
	/* [OPERATOR_GE - OPERATOR_CMPMIN]   = */ { offsetof(struct type_cmp, tp_ge), (void *)&instance_ge }
};

PRIVATE struct opwrapper seq_wrappers[] = {
	/* [OPERATOR_ITERSELF - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_iter_self), (void *)&instance_iter },
	/* [OPERATOR_SIZE     - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_size), (void *)&instance_size },
	/* [OPERATOR_CONTAINS - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_contains), (void *)&instance_contains },
	/* [OPERATOR_GETITEM  - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_get), (void *)&instance_getitem },
	/* [OPERATOR_DELITEM  - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_del), (void *)&instance_delitem },
	/* [OPERATOR_SETITEM  - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_set), (void *)&instance_setitem },
	/* [OPERATOR_GETRANGE - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_range_get), (void *)&instance_getrange },
	/* [OPERATOR_DELRANGE - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_range_del), (void *)&instance_delrange },
	/* [OPERATOR_SETRANGE - OPERATOR_SEQMIN] = */ { offsetof(struct type_seq, tp_range_set), (void *)&instance_setrange }
};

PRIVATE struct opwrapper attr_wrappers[] = {
	/* [OPERATOR_GETATTR  - OPERATOR_ATTRMIN] = */ { offsetof(struct type_attr, tp_getattr), (void *)&instance_getattr },
	/* [OPERATOR_DELATTR  - OPERATOR_ATTRMIN] = */ { offsetof(struct type_attr, tp_delattr), (void *)&instance_delattr },
	/* [OPERATOR_SETATTR  - OPERATOR_ATTRMIN] = */ { offsetof(struct type_attr, tp_setattr), (void *)&instance_setattr },
	/* [OPERATOR_ENUMATTR - OPERATOR_ATTRMIN] = */ { offsetof(struct type_attr, tp_enumattr), (void *)&instance_enumattr }
};

PRIVATE struct opwrapper with_wrappers[] = {
	/* [OPERATOR_ENTER - OPERATOR_WITHMIN] = */ { offsetof(struct type_with, tp_enter), (void *)&instance_enter },
	/* [OPERATOR_LEAVE - OPERATOR_WITHMIN] = */ { offsetof(struct type_with, tp_leave), (void *)&instance_leave },
};


PRIVATE int DCALL
lazy_allocate(void **ptable, size_t table_size) {
	void *new_table;
	if (*ptable)
		return 0;
	new_table = Dee_Calloc(table_size);
	if unlikely(!new_table)
		return -1;
#ifdef CONFIG_NO_THREADS
	if unlikely(*ptable)
		Dee_Free(new_table);
	else *ptable = new_table;
#else  /* CONFIG_NO_THREADS */
	if unlikely(!ATOMIC_CMPXCH(*ptable, NULL, new_table))
		Dee_Free(new_table);
#endif /* !CONFIG_NO_THREADS */
	return 0;
}

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define lazy_allocate(ptable, table_size) \
	__builtin_expect(lazy_allocate(ptable, table_size), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

#define LAZY_ALLOCATE(p) lazy_allocate((void **)&(p), sizeof(*(p)))


/* Bind the C-wrapper function(s) for `operator_name' in `class_type',
 * with `type_type' being responsible for providing said operator. */
PRIVATE int DCALL
bind_class_operator(DeeTypeObject *__restrict type_type,
                    DeeTypeObject *__restrict class_type,
                    uint16_t operator_name) {
	void *wrapper, **target;
	/* Assign operator wrapper. */
	if (operator_name <= OPERATOR_TYPEMAX) {
		/* type operator. */
		ASSERT(operator_name >= OPERATOR_COPY);
		wrapper = type_wrappers[operator_name - OPERATOR_COPY].wrapper;
		target  = (void **)((uintptr_t)class_type + type_wrappers[operator_name - OPERATOR_COPY].offset);
	} else if (operator_name <= OPERATOR_MATHMAX) {
		if (LAZY_ALLOCATE(class_type->tp_math))
			goto err;
		/* math operator. */
		ASSERT(operator_name >= OPERATOR_FLOAT);
		wrapper = math_wrappers[operator_name - OPERATOR_FLOAT].wrapper;
		target  = (void **)((uintptr_t)class_type->tp_math + math_wrappers[operator_name - OPERATOR_FLOAT].offset);
	} else if (operator_name <= OPERATOR_CMPMAX) {
		if (LAZY_ALLOCATE(class_type->tp_cmp))
			goto err;
		/* compare operator. */
		ASSERT(operator_name >= OPERATOR_CMPMIN);
		wrapper = cmp_wrappers[operator_name - OPERATOR_CMPMIN].wrapper;
		target  = (void **)((uintptr_t)class_type->tp_cmp + cmp_wrappers[operator_name - OPERATOR_CMPMIN].offset);
	} else if (operator_name <= OPERATOR_SEQMAX) {
		if (LAZY_ALLOCATE(class_type->tp_seq))
			goto err;
		/* compare operator. */
		ASSERT(operator_name >= OPERATOR_SEQMIN);
		wrapper = seq_wrappers[operator_name - OPERATOR_SEQMIN].wrapper;
		target  = (void **)((uintptr_t)class_type->tp_seq + seq_wrappers[operator_name - OPERATOR_SEQMIN].offset);
	} else if (operator_name <= OPERATOR_ATTRMAX) {
		if (LAZY_ALLOCATE(class_type->tp_attr))
			goto err;
		/* attribute operator. */
		ASSERT(operator_name >= OPERATOR_ATTRMIN);
		wrapper = attr_wrappers[operator_name - OPERATOR_ATTRMIN].wrapper;
		target  = (void **)((uintptr_t)class_type->tp_attr + attr_wrappers[operator_name - OPERATOR_ATTRMIN].offset);
	} else if (operator_name <= OPERATOR_WITHMAX) {
		if (LAZY_ALLOCATE(class_type->tp_with))
			goto err;
		/* with operators. */
		ASSERT(operator_name >= OPERATOR_WITHMIN);
		wrapper = with_wrappers[operator_name - OPERATOR_WITHMIN].wrapper;
		target  = (void **)((uintptr_t)class_type->tp_with + with_wrappers[operator_name - OPERATOR_WITHMIN].offset);
	} else {
		return DeeError_Throwf(&DeeError_TypeError,
		                       "Type %q does not define an operator %#I16x",
		                       type_type->tp_name, operator_name);
	}
	/* Assign the proper wrapper to the target. */
	*target = wrapper;
	return 0;
err:
	return -1;
}


PUBLIC DREF DeeTypeObject *DCALL
DeeClass_New(DeeTypeObject *__restrict base,
             DeeObject *__restrict descriptor) {
	DeeClassDescriptorObject *desc;
	DREF DeeTypeObject *result;
	DeeTypeObject *result_type_type;
	struct class_desc *result_class;
	size_t result_class_offset;
	ASSERT_OBJECT_TYPE_EXACT(descriptor, &DeeClassDescriptor_Type);
	desc             = (DeeClassDescriptorObject *)descriptor;
	result_type_type = Dee_TYPE(base);
	if (result_type_type == &DeeNone_Type) {
		result_type_type = &DeeType_Type; /* No base class. */
	} else {
		/* Make sure that the given base-object is actually a type. */
		if unlikely(!DeeType_IsInherited(&DeeType_Type, (DeeTypeObject *)result_type_type)) {
			DeeObject_TypeAssertFailed((DeeObject *)base, &DeeType_Type);
			goto err;
		}
		ASSERTF(!(result_type_type->tp_flags & TP_FVARIABLE),
		        "type-type objects must not have the variable-size flag, but %s has it set!",
		        result_type_type->tp_name);
		if (base->tp_flags & (TP_FFINAL | TP_FVARIABLE)) {
			DeeError_Throwf(&DeeError_TypeError,
			                "Cannot use final, or variable type `%s' as class base",
			                base->tp_name);
			goto err;
		}
	}
	result_class_offset = result_type_type->tp_init.tp_alloc.tp_instance_size;
	if (result_type_type->tp_init.tp_alloc.tp_free) {
err_custom_allocator:
		DeeError_Throwf(&DeeError_TypeError,
		                "Cannot use `%s' with custom allocator as class base",
		                base->tp_name);
		goto err;
	}
	result_class_offset += (sizeof(void *) - 1);
	result_class_offset &= ~(sizeof(void *) - 1);
	/* Allocate the resulting class object. */
	result = (DREF DeeTypeObject *)DeeGCObject_Calloc(result_class_offset +
	                                                  COMPILER_OFFSETOF(struct class_desc, cd_members) +
	                                                  (desc->cd_cmemb_size * sizeof(DREF DeeObject *)));
	if unlikely(!result)
		goto err;
	/* Figure out where the class descriptor starts. */
	result_class     = (struct class_desc *)((uintptr_t)result + result_class_offset);
	result->tp_class = result_class;
	/* Setup flags for the resulting type. */
	result->tp_flags = ((TP_FHEAP | TP_FGC) |
	                    (desc->cd_flags & (TP_FFINAL | TP_FTRUNCATE |
	                                       TP_FINTERRUPT | TP_FMOVEANY)));
	if (DeeNone_Check(base)) {
		/*result->tp_base = NULL;*/
		result_class->cd_offset = sizeof(DeeObject);
	} else {
		/* Calculate the offset of instance descriptors. */
		result_class->cd_offset = base->tp_init.tp_alloc.tp_instance_size;
		if (base->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_NO_OBJECT_SLABS
			void(DCALL * tp_free)(void *__restrict ob);
			size_t base_size;
			tp_free = base->tp_init.tp_alloc.tp_free;
			/* Figure out the slab size used by the base-class. */
			if (base->tp_flags & TP_FGC) {
#define CHECK_ALLOCATOR(index, size)                        \
				if (tp_free == &DeeGCObject_SlabFree##size) \
					base_size = size * sizeof(void *);      \
				else
				DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
				{
					DeeGCObject_Free(result);
					goto err_custom_allocator;
				}
			} else {
#define CHECK_ALLOCATOR(index, size)                      \
				if (tp_free == &DeeObject_SlabFree##size) \
					base_size = size * sizeof(void *);    \
				else
				DeeSlab_ENUMERATE(CHECK_ALLOCATOR)
#undef CHECK_ALLOCATOR
				{
					DeeGCObject_Free(result);
					goto err_custom_allocator;
				}
			}
			result_class->cd_offset = base_size;
#else  /* !CONFIG_NO_OBJECT_SLABS */
			DeeGCObject_Free(result);
			goto err_custom_allocator;
#endif /* CONFIG_NO_OBJECT_SLABS */
		}
		result_class->cd_offset += (sizeof(void *) - 1);
		result_class->cd_offset &= ~(sizeof(void *) - 1);
		result->tp_base = base;
		result->tp_flags |= base->tp_flags & TP_FINTERHITABLE;
		result->tp_weakrefs = base->tp_weakrefs;
		Dee_Incref(base);
	}
	result_class->cd_desc = desc;
	Dee_Incref(desc);
	rwlock_cinit(&result_class->cd_lock);

	if likely(desc->cd_name) {
		result->tp_name = DeeString_STR(desc->cd_name);
		result->tp_flags |= TP_FNAMEOBJECT;
		Dee_Incref(desc->cd_name);
	}
	if likely(desc->cd_doc) {
		result->tp_doc = DeeString_STR(desc->cd_doc);
		result->tp_flags |= TP_FDOCOBJECT;
		Dee_Incref(desc->cd_doc);
	}

	/* Determine the memory data size of instances for this class. */
	result->tp_init.tp_alloc.tp_instance_size = result_class->cd_offset;                         /* Memory used by base-classes. */
	result->tp_init.tp_alloc.tp_instance_size += offsetof(struct instance_desc, id_vtab);        /* Instance descriptor header. */
	result->tp_init.tp_alloc.tp_instance_size += desc->cd_imemb_size * sizeof(DREF DeeObject *); /* Instance member objects. */

	/* Assign default / mandatory operators. */
	result->tp_init.tp_alloc.tp_copy_ctor = &instance_builtin_copy;
	result->tp_init.tp_assign             = &instance_builtin_assign;
	result->tp_init.tp_move_assign        = &instance_builtin_moveassign;
	result->tp_init.tp_deepload           = &instance_builtin_deepload;
	result->tp_init.tp_dtor               = &instance_builtin_destructor;
	result->tp_visit                      = &instance_visit;
	result->tp_gc                         = &instance_gc;
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
	if (DeeNone_Check(base) || base == &DeeObject_Type) {
		result->tp_init.tp_alloc.tp_copy_ctor = &instance_builtin_nobase_copy;
		result->tp_init.tp_deepload           = &instance_builtin_nobase_deepload;
	}
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */


	{
#define FEATURE_CONSTRUCTOR 0x0001 /* A constructor is provided */
#define FEATURE_SUPERARGS   0x0002 /* A super-arguments generator is provided */
		uint16_t constructor_features = 0;
		uint16_t i                    = 0;
		do {
			struct class_operator *op;
			op = &desc->cd_clsop_list[i];
			if (op->co_name == (uint16_t)-1)
				continue;
			switch (op->co_name) {

			case OPERATOR_CONSTRUCTOR:
				constructor_features |= FEATURE_CONSTRUCTOR;
				break;

				/* Defining either assign, or move_assign will get rid
				 * of automatically generated operators for the other. */
			case OPERATOR_ASSIGN:
				result->tp_init.tp_assign = &instance_assign;
				if (result->tp_init.tp_move_assign == &instance_builtin_moveassign)
					result->tp_init.tp_move_assign = NULL;
				break;

			case OPERATOR_MOVEASSIGN:
				result->tp_init.tp_move_assign = &instance_moveassign;
				if (result->tp_init.tp_assign == &instance_builtin_assign)
					result->tp_init.tp_assign = NULL;
				break;

			case CLASS_OPERATOR_SUPERARGS:
				constructor_features |= FEATURE_SUPERARGS;
				break;

			case OPERATOR_CALL:
				result->tp_call    = &instance_call;
				result->tp_call_kw = &instance_callkw;
				break;

			case OPERATOR_INT:
				if (LAZY_ALLOCATE(result->tp_math))
					goto err_r_base;
				result->tp_math->tp_int32 = &instance_int32;
				result->tp_math->tp_int64 = &instance_int64;
				result->tp_math->tp_int   = &instance_int;
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
					goto err_r_base;
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
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
				if (DeeNone_Check(base) || base == &DeeObject_Type) {
					result->tp_init.tp_alloc.tp_ctor        = &instance_auto_nobase_ctor;
					result->tp_init.tp_alloc.tp_any_ctor    = &instance_auto_nobase_init;
					result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_auto_nobase_initkw;
				} else
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
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
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
			} else if (DeeNone_Check(base) || base == &DeeObject_Type) {
				result->tp_init.tp_alloc.tp_ctor        = &instance_nobase_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_nobase_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_nobase_initkw;
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
			} else {
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
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
				if (DeeNone_Check(base) || base == &DeeObject_Type) {
					result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_auto_nobase_ctor;
					result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_auto_nobase_init;
					result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_auto_nobase_initkw;
				} else
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
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
#ifdef CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS
			} else if (DeeNone_Check(base) || base == &DeeObject_Type) {
				result->tp_init.tp_alloc.tp_ctor        = &instance_builtin_nobase_ctor;
				result->tp_init.tp_alloc.tp_any_ctor    = &instance_builtin_nobase_init;
				result->tp_init.tp_alloc.tp_any_ctor_kw = &instance_builtin_nobase_initkw;
#endif /* CONFIG_HAVE_NOBASE_OPTIMIZED_CLASS_OPERATORS */
			} else {
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
	if (!result->tp_cast.tp_repr && (desc->cd_flags & CLASS_TP_FAUTOINIT))
		result->tp_cast.tp_repr = &instance_builtin_auto_repr;
#endif /* CLASS_TP_FAUTOINIT */
	if (!result->tp_cmp)
		result->tp_cmp = &instance_builtin_cmp;
	/* Make sure to disallow MOVE-ANY when the builtin move-assign operator is used. */
	if (result->tp_init.tp_move_assign == &instance_builtin_moveassign)
		result->tp_flags &= ~TP_FMOVEANY;
	if (result_type_type != &DeeType_Type) {
		/* Initialize custom fields of the underlying type. */
		int error = 0;
		if (result_type_type->tp_init.tp_alloc.tp_ctor) {
			error = (*result_type_type->tp_init.tp_alloc.tp_ctor)((DeeObject *)result);
		} else if (result_type_type->tp_init.tp_alloc.tp_any_ctor) {
			error = (*result_type_type->tp_init.tp_alloc.tp_any_ctor)((DeeObject *)result, 0, NULL);
		} else if (result_type_type->tp_init.tp_alloc.tp_any_ctor_kw) {
			error = (*result_type_type->tp_init.tp_alloc.tp_any_ctor_kw)((DeeObject *)result, 0, NULL, NULL);
		}
		if unlikely(error)
			goto err_r_base;
	}

	/* Initialize the resulting object, and start tracking it. */
	DeeObject_Init(result, result_type_type);
	return (DeeTypeObject *)DeeGC_Track((DeeObject *)result);
err_r_base:
	Dee_Free(result->tp_math);
	Dee_Free(result->tp_cmp);
	Dee_Free(result->tp_seq);
	Dee_Free(result->tp_attr);
	Dee_Free(result->tp_with);
	Dee_XDecref_unlikely(result->tp_base);
	Dee_XDecref_unlikely(desc->cd_name);
	Dee_XDecref_unlikely(desc->cd_doc);
	Dee_Decref_unlikely(desc);
/*err_r:*/
	DeeGCObject_Free(result);
err:
	return NULL;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CLASS_C */
