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
#ifndef GUARD_DEEMON_COMPILER_INTERFACE_CORE_C
#define GUARD_DEEMON_COMPILER_INTERFACE_CORE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/compiler/compiler.h>
#include <deemon/compiler/interface.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/util/atomic.h>

#include <hybrid/sequence/list.h>

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

typedef DeeCompilerItemObject CompilerItem;
typedef DeeCompilerWrapperObject CompilerWrapper;

INTERN NONNULL((1)) void DCALL
DeeCompilerItem_Fini(CompilerItem *__restrict self) {
	DeeCompilerObject *com = self->ci_compiler;
	Dee_compiler_items_lock_write(&com->cp_items);
	ASSERT(com->cp_items.cis_size);
	if (LIST_ISBOUND(self, ci_link)) {
		LIST_REMOVE(self, ci_link);
		--com->cp_items.cis_size;
	}
	Dee_compiler_items_lock_endwrite(&com->cp_items);
#ifdef CONFIG_NO_THREADS
	Dee_Decref(com);
#else /* CONFIG_NO_THREADS */
	if unlikely(!Dee_DecrefIfNotOne(com)) {
		DeeCompiler_LockWriteNoInt();
		Dee_Decref(com);
		DeeCompiler_LockEndWrite();
	}
#endif /* !CONFIG_NO_THREADS */
}

INTERN NONNULL((1, 2)) void DCALL
DeeCompilerItem_Visit(CompilerItem *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->ci_compiler);
}

INTERN NONNULL((1)) void DCALL
DeeCompilerObjItem_Fini(CompilerItem *__restrict self) {
	DeeCompilerObject *com = self->ci_compiler;
	Dee_compiler_items_lock_write(&com->cp_items);
	ASSERT(com->cp_items.cis_size);
	ASSERTF(LIST_ISBOUND(self, ci_link),
	        "Compiler object-items must not be deleted externally");
	LIST_REMOVE(self, ci_link);
	--com->cp_items.cis_size;
	Dee_compiler_items_lock_endwrite(&com->cp_items);
	COMPILER_BEGIN_NOINT(com);
	ASSERT_OBJECT((DeeObject *)(self->ci_value));
	Dee_Decref((DeeObject *)(self->ci_value));
	DeeCompiler_End();
	Dee_Decref_unlikely(com);
	DeeCompiler_LockEndWrite();
}

INTERN NONNULL((1, 2)) void DCALL
DeeCompilerObjItem_Visit(CompilerItem *__restrict self, Dee_visit_t proc, void *arg) {
	COMPILER_BEGIN_NOINT(self->ci_compiler);
	Dee_Visit(self->ci_compiler);
	Dee_Visit((DeeObject *)(self->ci_value));
	COMPILER_END();
}

INTDEF struct type_member tpconst DeeCompilerItem_Members[];
INTERN_CONST struct type_member tpconst DeeCompilerItem_Members[] = {
	TYPE_MEMBER_FIELD_DOC("compiler", STRUCT_OBJECT,
	                      offsetof(CompilerItem, ci_compiler),
	                      "->" DR_Compiler),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ CompilerItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerItem_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&DeeCompilerItem_Visit,
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ CompilerItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerObjItem_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&DeeCompilerObjItem_Visit,
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
	/* .tp_members       = */ DeeCompilerItem_Members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

STATIC_ASSERT(offsetof(CompilerItem, ci_compiler) ==
              offsetof(CompilerWrapper, cw_compiler));
INTERN NONNULL((1)) void DCALL
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ CompilerWrapper,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&DeeCompilerWrapper_Fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&DeeCompilerWrapper_Visit,
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
	/* .tp_members       = */ DeeCompilerWrapper_Members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

/* Construct and return a wrapper for a sub-component of the current compiler. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCompiler_GetWrapper(DeeCompilerObject *__restrict self,
                       DeeTypeObject *__restrict type) {
	DREF CompilerWrapper *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	ASSERT(!(type->tp_flags & TP_FVARIABLE));
	result = DeeObject_MALLOC(CompilerWrapper);
	if unlikely(!result)
		goto done;
	result->cw_compiler = self;
	Dee_Incref(self);
	DeeObject_Init(result, type);
done:
	return Dee_AsObject(result);
}



/* Lookup or create a new compiler item for `value' */
LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
get_compiler_item_impl(DeeTypeObject *__restrict type,
                       void *__restrict value,
                       bool is_an_object) {
	DREF CompilerItem *result;
	DeeCompilerObject *self = DeeCompiler_Current;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	ASSERT(!(type->tp_flags & TP_FVARIABLE));
	ASSERT(DeeCompiler_LockReading());
again:
	Dee_compiler_items_lock_read(&self->cp_items);
	if (self->cp_items.cis_list) {
		struct compiler_item_object_list *list;
		list = &self->cp_items.cis_list[Dee_HashPointer(value) & self->cp_items.cis_mask];
		LIST_FOREACH(result, list, ci_link) {
			if (result->ci_value != value)
				continue;
			if unlikely(!Dee_IncrefIfNotZero(result))
				continue;
			ASSERT_OBJECT_TYPE_EXACT(result, type);
			Dee_compiler_items_lock_endread(&self->cp_items);
			return Dee_AsObject(result);
		}
	}
	Dee_compiler_items_lock_endread(&self->cp_items);

	/* Construct a new item. */
	result = DeeObject_MALLOC(CompilerItem);
	if unlikely(!result)
		goto done;

	/* Make sure that the item wasn't created in the mean time! */
	Dee_compiler_items_lock_write(&self->cp_items);
	if (self->cp_items.cis_list) {
		DREF CompilerItem *new_result;
		struct compiler_item_object_list *list;
		list = &self->cp_items.cis_list[Dee_HashPointer(value) & self->cp_items.cis_mask];
		LIST_FOREACH (new_result, list, ci_link) {
			if likely(new_result->ci_value != value)
				continue;
			if unlikely(!Dee_IncrefIfNotZero(new_result))
				continue;
			ASSERT_OBJECT_TYPE_EXACT(new_result, type);
			Dee_compiler_items_lock_endread(&self->cp_items);
			DeeObject_FREE(result);
			return Dee_AsObject(new_result);
		}
	}

	/* Insert the new item into the hash-map. */
	if (self->cp_items.cis_size >= self->cp_items.cis_mask) {
		size_t new_mask = (self->cp_items.cis_mask << 1) | 1;
		struct compiler_item_object_list *new_map;
		if (new_mask == 1)
			new_mask = 16 - 1;
		new_map = (struct compiler_item_object_list *)Dee_TryCallocc(new_mask + 1,
		                                                             sizeof(struct compiler_item_object_list));
		if unlikely(!new_map && !self->cp_items.cis_list) {
			Dee_compiler_items_lock_endwrite(&self->cp_items);
			DeeObject_FREE(result);
			goto again;
		}

		/* Re-hash the old map. */
		if (self->cp_items.cis_list) {
			size_t i;
			for (i = 0; i <= self->cp_items.cis_mask; ++i) {
				DeeCompilerItemObject *iter, *next;
				LIST_FOREACH_SAFE (iter, &self->cp_items.cis_list[i], ci_link, next) {
					struct compiler_item_object_list *newlist;
					newlist = &new_map[Dee_COMPILER_ITEM_HASH(iter) & new_mask];
					LIST_REMOVE(iter, ci_link);
					LIST_INSERT_HEAD(newlist, iter, ci_link);
				}
			}
			Dee_Free(self->cp_items.cis_list);
		}
		self->cp_items.cis_mask = new_mask;
		self->cp_items.cis_list = new_map;
	}

	/* Insert the new item into the hash-map. */
	{
		struct compiler_item_object_list *list;
		list = &self->cp_items.cis_list[Dee_HashPointer(value) & self->cp_items.cis_mask];
		LIST_INSERT_HEAD(list, result, ci_link);
	}

	/* Keep track of how many items there are. */
	++self->cp_items.cis_size;

	/* Initialize the new item. */
	result->ci_compiler = self;
	result->ci_value    = value;
	DeeObject_Init(result, type);
	Dee_Incref(self);
	if (is_an_object)
		Dee_Incref((DeeObject *)value);
	Dee_compiler_items_lock_endwrite(&self->cp_items);
done:
	return Dee_AsObject(result);
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCompiler_GetItem(DeeTypeObject *__restrict type,
                    void *__restrict value) {
#ifndef NDEBUG
	DeeTypeObject *tp = type;
	for (;; tp = DeeType_Base(tp)) {
		if (!tp->tp_init.tp_dtor)
			continue;
		ASSERTF(tp->tp_init.tp_dtor == (void (DCALL *)(DeeObject *__restrict))&DeeCompilerItem_Fini,
		        "Expected an weakly linked compiler item type");
		break;
	}
#endif /* !NDEBUG */
	return get_compiler_item_impl(type, value, false);
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCompiler_GetObjItem(DeeTypeObject *__restrict type,
                       DeeObject *__restrict value) {
#ifndef NDEBUG
	DeeTypeObject *tp = type;
	for (;; tp = DeeType_Base(tp)) {
		if (!tp->tp_init.tp_dtor)
			continue;
		ASSERTF(tp->tp_init.tp_dtor == (void (DCALL *)(DeeObject *__restrict))&DeeCompilerObjItem_Fini,
		        "Expected an object-like compiler item type");
		break;
	}
#endif /* !NDEBUG */
	return get_compiler_item_impl(type, value, true);
}

/* Delete (clear) the compiler item associated with `value'. */
INTERN bool DCALL DeeCompiler_DelItem(void *value) {
	CompilerItem *item;
	struct compiler_item_object_list *list;
	DeeCompilerObject *com = DeeCompiler_Current;
	if (!com)
		return false;
	ASSERT(DeeCompiler_LockReading());
	Dee_compiler_items_lock_write(&com->cp_items);
	if unlikely(!com->cp_items.cis_list) {
		Dee_compiler_items_lock_endwrite(&com->cp_items);
		return false;
	}
	list = &com->cp_items.cis_list[Dee_HashPointer(value) & com->cp_items.cis_mask];
	LIST_FOREACH (item, list, ci_link) {
		if (item->ci_value != value)
			continue;
#ifndef NDEBUG
		{
			DeeTypeObject *tp = Dee_TYPE(item);
			for (;; tp = DeeType_Base(tp)) {
				if (!tp->tp_init.tp_dtor)
					continue;
				ASSERTF(tp->tp_init.tp_dtor == (void (DCALL *)(DeeObject *__restrict))&DeeCompilerItem_Fini,
				        "Cannot delete compiler item of type %k, which has a custom, or object-finalizer",
				        tp);
				break;
			}
		}
#endif /* !NDEBUG */
		ASSERT(LIST_ISBOUND(item, ci_link));
		LIST_UNBIND(item, ci_link);
		ASSERT(com->cp_items.cis_size);
		--com->cp_items.cis_size;
		break;
	}
	Dee_compiler_items_lock_endwrite(&com->cp_items);
	return item != NULL;
}

/* Delete (clear) all compiler items matching the given `type'. */
INTERN NONNULL((1)) size_t DCALL
DeeCompiler_DelItemType(DeeTypeObject *__restrict type) {
	size_t i, result = 0;
	DeeCompilerObject *com = DeeCompiler_Current;
#ifndef NDEBUG
	DeeTypeObject *tp = type;
	for (;; tp = DeeType_Base(tp)) {
		if (!tp->tp_init.tp_dtor)
			continue;
		ASSERTF(tp->tp_init.tp_dtor == (void (DCALL *)(DeeObject *__restrict))&DeeCompilerItem_Fini,
		        "Cannot delete compiler items of type %k, which has a custom, or object-finalizer",
		        tp);
		break;
	}
#endif /* !NDEBUG */
	ASSERT(DeeCompiler_LockReading());
	if (!com)
		return false;
	Dee_compiler_items_lock_write(&com->cp_items);
	if (com->cp_items.cis_size) {
		ASSERT(com->cp_items.cis_list != NULL);
		for (i = 0; i <= com->cp_items.cis_mask; ++i) {
			CompilerItem *iter, *next;
			struct compiler_item_object_list *list;
			list = &com->cp_items.cis_list[i];
			LIST_FOREACH_SAFE (iter, list, ci_link, next) {
				if (DeeObject_InstanceOfExact(iter, type) &&
				    atomic_read(&iter->ob_refcnt) != 0) {
					ASSERT(LIST_ISBOUND(iter, ci_link));
					LIST_UNBIND(iter, ci_link);
					ASSERT(com->cp_items.cis_size);
					--com->cp_items.cis_size;
					++result;
				}
			}
		}
	}
	Dee_compiler_items_lock_endwrite(&com->cp_items);
	return result;
}

INTERN ATTR_COLD NONNULL((1)) int
(DCALL err_compiler_item_deleted)(DeeCompilerItemObject *__restrict item) {
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "Compiler item of type %k was deleted",
	                       item->ob_type);
}

INTERN WUNUSED NONNULL((1)) void *DCALL
DeeCompilerItem_GetValue(DeeObject *__restrict self) {
	void *result;
	ASSERT(DeeCompiler_LockReading());
	ASSERT(DeeCompiler_Current == ((DeeCompilerItemObject *)self)->ci_compiler);
	result = ((DeeCompilerItemObject *)self)->ci_value;
	if unlikely(!result)
		err_compiler_item_deleted((DeeCompilerItemObject *)self);
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_INTERFACE_CORE_C */
