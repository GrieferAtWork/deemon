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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_CORE_C
#define GUARD_DEEMON_COMPILER_INTERFACE_CORE_C 1
#define _KOS_SOURCE 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/interface.h>
#include <deemon/compiler/symbol.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/util/cache.h>

#include <hybrid/atomic.h>

DECL_BEGIN

typedef DeeCompilerItemObject CompilerItem;
typedef DeeCompilerWrapperObject CompilerWrapper;

DEFINE_OBJECT_CACHE(compiler_item, CompilerItem, 64)    /* TODO: Use slabs */
DEFINE_OBJECT_CACHE(compiler_wrap, CompilerWrapper, 16) /* TODO: Use slabs */

INTERN void DCALL
DeeCompilerItem_Fini(CompilerItem *__restrict self) {
	DeeCompilerObject *com = self->ci_compiler;
	rwlock_write(&com->cp_items.ci_lock);
	ASSERT(com->cp_items.ci_size);
	if (self->ci_pself) {
		if ((*self->ci_pself = self->ci_next) != NULL)
			self->ci_next->ci_pself = self->ci_pself;
		--com->cp_items.ci_size;
	}
	rwlock_endwrite(&com->cp_items.ci_lock);
#ifdef CONFIG_NO_THREADS
	Dee_Decref(com);
#else /* CONFIG_NO_THREADS */
	if unlikely(!Dee_DecrefIfNotOne(com)) {
		recursive_rwlock_write(&DeeCompiler_Lock);
		Dee_Decref(com);
		recursive_rwlock_endwrite(&DeeCompiler_Lock);
	}
#endif /* !CONFIG_NO_THREADS */
}

INTERN void DCALL
DeeCompilerItem_Visit(CompilerItem *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ci_compiler);
}

INTERN void DCALL
DeeCompilerObjItem_Fini(CompilerItem *__restrict self) {
	DeeCompilerObject *com = self->ci_compiler;
	rwlock_write(&com->cp_items.ci_lock);
	ASSERT(com->cp_items.ci_size);
	ASSERTF(self->ci_pself != NULL,
	        "Compiler object-items must not be deleted externally");
	if ((*self->ci_pself = self->ci_next) != NULL)
		self->ci_next->ci_pself = self->ci_pself;
	--com->cp_items.ci_size;
	rwlock_endwrite(&com->cp_items.ci_lock);
	COMPILER_BEGIN(com);
	ASSERT_OBJECT((DeeObject *)self->ci_value);
	Dee_Decref((DeeObject *)self->ci_value);
	DeeCompiler_End();
	Dee_Decref_unlikely(com);
	recursive_rwlock_endwrite(&DeeCompiler_Lock);
}

INTERN void DCALL
DeeCompilerObjItem_Visit(CompilerItem *__restrict self, dvisit_t proc, void *arg) {
	COMPILER_BEGIN(self->ci_compiler);
	Dee_Visit(self->ci_compiler);
	Dee_Visit((DeeObject *)self->ci_value);
	COMPILER_END();
}

INTERN struct type_member DeeCompilerItem_Members[] = {
	TYPE_MEMBER_FIELD_DOC("compiler", STRUCT_OBJECT, offsetof(CompilerItem, ci_compiler), "->?Ert:Compiler"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeCompilerItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CompilerItem",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_ALLOCATOR(&compiler_item_tp_alloc, &compiler_item_tp_free)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerItem_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&DeeCompilerItem_Visit,
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
	/* .tp_members       = */ DeeCompilerItem_Members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeCompilerObjItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CompilerObjItem",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_ALLOCATOR(&compiler_item_tp_alloc, &compiler_item_tp_free)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerObjItem_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&DeeCompilerObjItem_Visit,
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
	/* .tp_members       = */ DeeCompilerItem_Members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

STATIC_ASSERT(COMPILER_OFFSETOF(CompilerItem, ci_compiler) ==
              COMPILER_OFFSETOF(CompilerWrapper, cw_compiler));
INTERN void DCALL
DeeCompilerWrapper_Fini(CompilerWrapper *__restrict self) {
	Dee_Decref_unlikely(self->cw_compiler);
}

INTERN DeeTypeObject DeeCompilerWrapper_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CompilerWrapper",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_ALLOCATOR(&compiler_wrap_tp_alloc, &compiler_wrap_tp_free)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerWrapper_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&DeeCompilerWrapper_Visit,
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
	/* .tp_members       = */ DeeCompilerWrapper_Members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

/* Construct and return a wrapper for a sub-component of the current compiler. */
INTERN DREF DeeObject *DCALL
DeeCompiler_GetWrapper(DeeCompilerObject *__restrict self,
                       DeeTypeObject *__restrict type) {
	DREF CompilerWrapper *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	ASSERT(!(type->tp_flags & TP_FVARIABLE));
	ASSERT(type->tp_init.tp_alloc.tp_alloc == &compiler_wrap_tp_alloc);
	ASSERT(type->tp_init.tp_alloc.tp_free == &compiler_wrap_tp_free);
	result = compiler_wrap_alloc();
	if unlikely(!result)
		goto done;
	result->cw_compiler = self;
	Dee_Incref(self);
	DeeObject_Init(result, type);
done:
	return (DREF DeeObject *)result;
}



/* Lookup or create a new compiler item for `value' */
LOCAL DREF DeeObject *DCALL
get_compiler_item_impl(DeeTypeObject *__restrict type,
                       void *__restrict value,
                       bool is_an_object) {
	DREF CompilerItem *result, *new_result;
	DeeCompilerObject *self = DeeCompiler_Current;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	ASSERT(!(type->tp_flags & TP_FVARIABLE));
	ASSERT(type->tp_init.tp_alloc.tp_alloc == &compiler_item_tp_alloc);
	ASSERT(type->tp_init.tp_alloc.tp_free == &compiler_item_tp_free);
	ASSERT(value != NULL);
	ASSERT(self);
	ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
again:
	rwlock_read(&self->cp_items.ci_lock);
	if (self->cp_items.ci_list) {
		result = self->cp_items.ci_list[Dee_HashPointer(value) & self->cp_items.ci_mask];
		for (; result; result = result->ci_next) {
			if (result->ci_value != value)
				continue;
			if unlikely(!Dee_IncrefIfNotZero(result))
				continue;
			ASSERT_OBJECT_TYPE_EXACT(result, type);
			rwlock_endread(&self->cp_items.ci_lock);
			return (DREF DeeObject *)result;
		}
	}
	rwlock_endread(&self->cp_items.ci_lock);
	/* Construct a new item. */
	result = compiler_item_alloc();
	if unlikely(!result)
		goto done;
	rwlock_write(&self->cp_items.ci_lock);
	/* Make sure that the item wasn't created in the mean time! */
	if (self->cp_items.ci_list) {
		new_result = self->cp_items.ci_list[Dee_HashPointer(value) & self->cp_items.ci_mask];
		for (; new_result; new_result = new_result->ci_next) {
			if likely(new_result->ci_value != value)
				continue;
			if unlikely(!Dee_IncrefIfNotZero(new_result))
				continue;
			ASSERT_OBJECT_TYPE_EXACT(new_result, type);
			rwlock_endread(&self->cp_items.ci_lock);
			compiler_item_free(result);
			return (DREF DeeObject *)new_result;
		}
	}
	/* Insert the new item into the hash-map. */
	if (self->cp_items.ci_size >= self->cp_items.ci_mask) {
		size_t new_mask = (self->cp_items.ci_mask << 1) | 1;
		DeeCompilerItemObject **new_map;
		if (new_mask == 1)
			new_mask = 16 - 1;
		new_map = (DeeCompilerItemObject **)Dee_TryCalloc((new_mask + 1) *
		                                                  sizeof(DeeCompilerItemObject *));
		if unlikely(!new_map && !self->cp_items.ci_list) {
			rwlock_endwrite(&self->cp_items.ci_lock);
			compiler_item_free(result);
			goto again;
		}
		/* Re-hash the old map. */
		if (self->cp_items.ci_list) {
			DeeCompilerItemObject *iter, *next;
			size_t i;
			for (i = 0; i <= self->cp_items.ci_mask; ++i) {
				iter = self->cp_items.ci_list[i];
				while (iter) {
					next           = iter->ci_next;
					iter->ci_pself = &new_map[Dee_COMPILER_ITEM_HASH(iter) & new_mask];
					if ((iter->ci_next = *iter->ci_pself) != NULL)
						iter->ci_next->ci_pself = &iter->ci_next;
					*iter->ci_pself = iter;
					iter            = next;
				}
			}
			Dee_Free(self->cp_items.ci_list);
		}
		self->cp_items.ci_mask = new_mask;
		self->cp_items.ci_list = new_map;
	}
	/* Insert the new item into the hash-map. */
	result->ci_pself = &self->cp_items.ci_list[Dee_HashPointer(value) & self->cp_items.ci_mask];
	if ((result->ci_next = *result->ci_pself) != NULL)
		result->ci_next->ci_pself = &result->ci_next;
	*result->ci_pself = result;
	/* Keep track of how many items there are. */
	++self->cp_items.ci_size;
	/* Initialize the new item. */
	result->ci_compiler = self;
	result->ci_value    = value;
	DeeObject_Init(result, type);
	Dee_Incref(self);
	if (is_an_object)
		Dee_Incref((DeeObject *)value);
	rwlock_endwrite(&self->cp_items.ci_lock);
done:
	return (DREF DeeObject *)result;
}


INTERN DREF DeeObject *DCALL
DeeCompiler_GetItem(DeeTypeObject *__restrict type,
                    void *__restrict value) {
#ifndef NDEBUG
	DeeTypeObject *tp = type;
	for (;; tp = tp->tp_base) {
		if (!tp->tp_init.tp_dtor)
			continue;
		ASSERTF(tp->tp_init.tp_dtor == (void (DCALL *)(DeeObject * __restrict))&DeeCompilerItem_Fini,
		        "Expected an weakly linked compiler item type");
		break;
	}
#endif /* !NDEBUG */
	return get_compiler_item_impl(type, value, false);
}

INTERN DREF DeeObject *DCALL
DeeCompiler_GetObjItem(DeeTypeObject *__restrict type,
                       DeeObject *__restrict value) {
#ifndef NDEBUG
	DeeTypeObject *tp = type;
	for (;; tp = tp->tp_base) {
		if (!tp->tp_init.tp_dtor)
			continue;
		ASSERTF(tp->tp_init.tp_dtor == (void (DCALL *)(DeeObject * __restrict))&DeeCompilerObjItem_Fini,
		        "Expected an object-like compiler item type");
		break;
	}
#endif /* !NDEBUG */
	return get_compiler_item_impl(type, value, true);
}

/* Delete (clear) the compiler item associated with `value'. */
INTERN bool DCALL DeeCompiler_DelItem(void *value) {
	CompilerItem *item;
	DeeCompilerObject *com = DeeCompiler_Current;
	if (!com)
		return false;
	ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
	rwlock_write(&com->cp_items.ci_lock);
	if unlikely(!com->cp_items.ci_list) {
		rwlock_endwrite(&com->cp_items.ci_lock);
		return false;
	}
	item = com->cp_items.ci_list[Dee_HashPointer(value) & com->cp_items.ci_mask];
	for (; item; item = item->ci_next) {
		if (item->ci_value != value)
			continue;
#ifndef NDEBUG
		{
			DeeTypeObject *tp = Dee_TYPE(item);
			for (;; tp = tp->tp_base) {
				if (!tp->tp_init.tp_dtor)
					continue;
				ASSERTF(tp->tp_init.tp_dtor == (void (DCALL *)(DeeObject * __restrict))&DeeCompilerItem_Fini,
				        "Cannot delete compiler item of type %k, which has a custom, or object-finalizer",
				        tp);
				break;
			}
		}
#endif /* !NDEBUG */
		ASSERT(item->ci_pself != NULL);
		if ((*item->ci_pself = item->ci_next) != NULL)
			item->ci_next->ci_pself = item->ci_pself;
		ASSERT(com->cp_items.ci_size);
		--com->cp_items.ci_size;
		item->ci_pself = NULL;
		break;
	}
	rwlock_endwrite(&com->cp_items.ci_lock);
	return item != NULL;
}

/* Delete (clear) all compiler items matching the given `type'. */
INTERN size_t DCALL
DeeCompiler_DelItemType(DeeTypeObject *__restrict type) {
	size_t i, result = 0;
	CompilerItem *iter, *next;
	DeeCompilerObject *com = DeeCompiler_Current;
#ifndef NDEBUG
	DeeTypeObject *tp = type;
	for (;; tp = tp->tp_base) {
		if (!tp->tp_init.tp_dtor)
			continue;
		ASSERTF(tp->tp_init.tp_dtor == (void (DCALL *)(DeeObject * __restrict))&DeeCompilerItem_Fini,
		        "Cannot delete compiler items of type %k, which has a custom, or object-finalizer",
		        tp);
		break;
	}
#endif /* !NDEBUG */
	ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
	if (!com)
		return false;
	rwlock_write(&com->cp_items.ci_lock);
	if (com->cp_items.ci_size) {
		ASSERT(com->cp_items.ci_list != NULL);
		for (i = 0; i <= com->cp_items.ci_mask; ++i) {
			iter = com->cp_items.ci_list[i];
			while (iter) {
				next = iter->ci_next;
				if (DeeObject_InstanceOfExact(iter, type) &&
				    ATOMIC_READ(iter->ob_refcnt) != 0) {
					ASSERT(iter->ci_pself != NULL);
					if ((*iter->ci_pself = iter->ci_next) != NULL)
						iter->ci_next->ci_pself = iter->ci_pself;
					ASSERT(com->cp_items.ci_size);
					--com->cp_items.ci_size;
					iter->ci_pself = NULL;
					++result;
				}
				iter = next;
			}
		}
	}
	rwlock_endwrite(&com->cp_items.ci_lock);
	return result;
}

INTERN ATTR_COLD int DCALL
err_compiler_item_deleted(DeeCompilerItemObject *__restrict item) {
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "Compiler item of type %k was deleted",
	                       item->ob_type);
}

INTERN void *DCALL
DeeCompilerItem_GetValue(DeeObject *__restrict self) {
	void *result;
	ASSERT(recursive_rwlock_reading(&DeeCompiler_Lock));
	ASSERT(DeeCompiler_Current == ((DeeCompilerItemObject *)self)->ci_compiler);
	result = ((DeeCompilerItemObject *)self)->ci_value;
	if unlikely(!result)
		err_compiler_item_deleted((DeeCompilerItemObject *)self);
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_CORE_C */
