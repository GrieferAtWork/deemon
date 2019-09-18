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
#ifndef GUARD_DEX_COLLECTIONS_FIXEDLIST_C
#define GUARD_DEX_COLLECTIONS_FIXEDLIST_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libcollections.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/thread.h>
#include <deemon/util/string.h>

/*
#include <deemon/HashSet.h>
#include <deemon/instancemethod.h>
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/callable.h>
#include <deemon/cell.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/dec.h>
#include <deemon/dex.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/notify.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/property.h>
#include <deemon/rodict.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/weakref.h>
*/

DECL_BEGIN

PRIVATE DREF FixedList *DCALL fl_ctor(void) {
	DREF FixedList *result;
	result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem));
	if
		unlikely(!result)
	goto done;
	rwlock_init(&result->fl_lock);
	result->fl_size = 0;
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return result;
}

PRIVATE DREF FixedList *DCALL
fl_copy(FixedList *__restrict self) {
	DREF FixedList *result;
	size_t i;
	result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
	                                              (self->fl_size * sizeof(DREF DeeObject *)));
	if
		unlikely(!result)
	goto done;
	rwlock_init(&result->fl_lock);
	result->fl_size = self->fl_size;
	rwlock_read(&self->fl_lock);
	for (i = 0; i < self->fl_size; ++i) {
		result->fl_elem[i] = self->fl_elem[i];
		Dee_XIncref(result->fl_elem[i]);
	}
	rwlock_endread(&self->fl_lock);
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return result;
}

PRIVATE DREF FixedList *DCALL
fl_init_iterator(DeeObject *__restrict iterator) {
	DREF FixedList *result, *new_result;
	DREF DeeObject *elem;
	DREF DeeObject *next;
	size_t itema, itemc;
	elem = DeeObject_IterNext(iterator);
	if (!ITER_ISOK(elem)) {
		if (elem == ITER_DONE)
			return fl_ctor();
		goto err;
	}
	next = DeeObject_IterNext(iterator);
	if (!ITER_ISOK(next)) {
		if (next == ITER_DONE) {
			result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
			                                              (1 * sizeof(DREF DeeObject *)));
			if
				unlikely(!result)
			goto err;
			itemc              = 1;
			result->fl_elem[0] = elem;
			goto done;
		}
		goto err;
	}
	itemc = 2, itema = 4;
	result = (DREF FixedList *)DeeGCObject_TryMalloc(offsetof(FixedList, fl_elem) +
	                                                 (4 * sizeof(DREF DeeObject *)));
	if
		unlikely(!result)
	{
		itema  = 2;
		result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
		                                              (2 * sizeof(DREF DeeObject *)));
		if
			unlikely(!result)
		goto err;
	}
	result->fl_elem[0] = elem;
	result->fl_elem[1] = next;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		ASSERT(itemc <= itema);
		if (itemc >= itema) {
			itema *= 2;
			new_result = (DREF FixedList *)DeeGCObject_TryRealloc(result,
			                                                      offsetof(FixedList, fl_elem) +
			                                                      (itema * sizeof(DREF DeeObject *)));
			if
				unlikely(!new_result)
			{
				itema      = itemc + 1;
				new_result = (DREF FixedList *)DeeGCObject_Realloc(result,
				                                                   offsetof(FixedList, fl_elem) +
				                                                   (itema * sizeof(DREF DeeObject *)));
				if
					unlikely(!new_result)
				goto err_r;
			}
			result = new_result;
		}
		result->fl_elem[itemc] = elem; /* Inherit reference. */
		++itemc;
		if (DeeThread_CheckInterrupt())
			goto err_r;
	}
	if
		unlikely(!elem)
	goto err_r;
	if (itema > itemc) {
		new_result = (DREF FixedList *)DeeGCObject_TryRealloc(result,
		                                                      offsetof(FixedList, fl_elem) +
		                                                      (itemc * sizeof(DREF DeeObject *)));
		if
			likely(new_result)
		result = new_result;
	}
done:
	rwlock_init(&result->fl_lock);
	result->fl_size = itemc;
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	DeeGC_Track((DeeObject *)result);
	return result;
err_r:
	while (itemc--)
		Dee_Decref(result->fl_elem[itemc]);
	DeeGCObject_Free(result);
err:
	return NULL;
}

PRIVATE DREF FixedList *DCALL
fl_init_getitem(DREF DeeObject *(DCALL *getitem)(DeeObject *__restrict self,
                                                 DeeObject *__restrict index),
                DeeObject *__restrict sequence, size_t length) {
	DREF FixedList *result;
	size_t i;
	DREF DeeObject *index_ob, *elem;
	result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
	                                              (length * sizeof(DREF DeeObject *)));
	if
		unlikely(!result)
	goto err;
	for (i = 0; i < length; ++i) {
		index_ob = DeeInt_NewSize(i);
		if
			unlikely(!index_ob)
		goto err_r;
		elem = (*getitem)(sequence, index_ob);
		Dee_Decref(index_ob);
		if
			unlikely(!elem)
		{
			if (!DeeError_Catch(&DeeError_UnboundItem))
				goto err_r;
		}
		result->fl_elem[i] = elem;
	}
	rwlock_init(&result->fl_lock);
	result->fl_size = length;
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	DeeGC_Track((DeeObject *)result);
	return result;
err_r:
	while (i--)
		Dee_Decref(result->fl_elem[i]);
	DeeGCObject_Free(result);
err:
	return NULL;
}


PRIVATE DREF FixedList *DCALL
fl_init(size_t argc, DeeObject **__restrict argv) {
	DREF FixedList *result;
	DeeObject *size_ob, *init = NULL;
	size_t size;
	if (DeeArg_Unpack(argc, argv, "o|o:FixedList", &size_ob, &init))
		goto err;
	if (init) {
		if (DeeObject_AsSize(size_ob, &size))
			goto err;
		result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
		                                              (size * sizeof(DREF DeeObject *)));
		if
			unlikely(!result)
		goto err;
		rwlock_init(&result->fl_lock);
		result->fl_size = size;
		Dee_Incref_n(init, size);
		MEMFIL_PTR(result->fl_elem, init, size);
	} else if (DeeInt_Check(size_ob)) {
		if (DeeObject_AsSize(size_ob, &size))
			goto err;
		result = (DREF FixedList *)DeeGCObject_Calloc(offsetof(FixedList, fl_elem) +
		                                              (size * sizeof(DREF DeeObject *)));
		if
			unlikely(!result)
		goto err;
		rwlock_cinit(&result->fl_lock);
		result->fl_size = size;
	} else {
		size_t i;
		if (DeeType_IsInherited(Dee_TYPE(size_ob), &DeeMapping_Type))
			goto init_from_iterator;
		/* Initialize from sequence. */
		size = DeeFastSeq_GetSize(size_ob);
		if (size == (size_t)-1) {
			DeeTypeObject *iter;
			DREF DeeObject *iterator;
			iter = Dee_TYPE(size_ob);
			for (;;) {
				DeeTypeObject *base = DeeType_Base(iter);
				if (iter->tp_seq &&
				    (!base || iter->tp_seq != base->tp_seq)) {
					if (iter->tp_seq->tp_get &&
					    (!base || !base->tp_seq || base->tp_seq->tp_get != iter->tp_seq->tp_get)) {
						size = DeeObject_Size(size_ob);
						if
							unlikely(size == (size_t)-1)
						goto err;
						return fl_init_getitem(iter->tp_seq->tp_get, size_ob, size);
					}
					if (iter->tp_seq->tp_iter_self &&
					    (!base || !base->tp_seq || base->tp_seq->tp_iter_self != iter->tp_seq->tp_iter_self)) {
						iterator = (*iter->tp_seq->tp_iter_self)(size_ob);
						if
							unlikely(!iterator)
						goto err;
						result = fl_init_iterator(iterator);
						Dee_Decref(iterator);
						return result;
					}
				}
				if (!base)
					break;
				iter = base;
			}
init_from_iterator:
			/* Initialize from iterators. */
			iterator = DeeObject_IterSelf(size_ob);
			if
				unlikely(!iterator)
			goto err;
			result = fl_init_iterator(iterator);
			Dee_Decref(iterator);
			return result;
		}
		/* Initialize from a fast sequence */
		result = (DREF FixedList *)DeeGCObject_Malloc(offsetof(FixedList, fl_elem) +
		                                              (size * sizeof(DREF DeeObject *)));
		if
			unlikely(!result)
		goto err;
		rwlock_init(&result->fl_lock);
		result->fl_size = size;
		for (i = 0; i < size; ++i) {
			DREF DeeObject *elem;
			elem = DeeFastSeq_GetItemUnbound(size_ob, i);
			if (elem == ITER_DONE)
				goto err_r;
			result->fl_elem[i] = elem; /* Inherit reference. */
		}
	}
	/*done:*/
	weakref_support_init(result);
	DeeObject_Init(result, &FixedList_Type);
	DeeGC_Track((DeeObject *)result);
	return result;
err_r:
	DeeGCObject_Free(result);
err:
	return NULL;
}

PRIVATE void DCALL
fl_fini(FixedList *__restrict self) {
	size_t i;
	weakref_support_fini(self);
	for (i = 0; i < self->fl_size; ++i)
		Dee_XDecref(self->fl_elem[i]);
}

PRIVATE int DCALL
fl_assign(FixedList *__restrict self, DeeObject *__restrict other) {
	DREF DeeObject **items, *temp;
	size_t i;
	items = (DREF DeeObject **)Dee_AMalloc(self->fl_size *
	                                       sizeof(DREF DeeObject *));
	if
		unlikely(!items)
	goto err;
	if (DeeObject_Unpack(other, self->fl_size, items))
		goto err_items;
	rwlock_write(&self->fl_lock);
	/* Exchange all stored items. */
	for (i = 0; i < self->fl_size; ++i) {
		temp             = self->fl_elem[i];
		self->fl_elem[i] = items[i];
		items[i]         = temp;
	}
	rwlock_endwrite(&self->fl_lock);
	/* Drop references to all of the old items. */
	for (i = 0; i < self->fl_size; ++i)
		Dee_XDecref(items[i]);
	Dee_AFree(items);
	return 0;
err_items:
	Dee_AFree(items);
err:
	return -1;
}

PRIVATE int DCALL
fl_moveassign(FixedList *__restrict self,
              FixedList *__restrict other) {
	DREF DeeObject **items;
	size_t i;
	if (self == other)
		return 0;
	if
		unlikely(self->fl_size != other->fl_size)
	{
		return DeeError_Throwf(&DeeError_UnpackError,
		                       "Expected a fixed list containing %Iu object%s when one containing %Iu was given",
		                       self->fl_size, self->fl_size > 1 ? "s" : "", other->fl_size);
	}
	items = (DREF DeeObject **)Dee_AMalloc(self->fl_size *
	                                       sizeof(DREF DeeObject *));
	if
		unlikely(!items)
	goto err;
#ifndef CONFIG_NO_THREADS
write_self_again:
	rwlock_write(&self->fl_lock);
	if (!rwlock_trywrite(&other->fl_lock)) {
		rwlock_endwrite(&self->fl_lock);
		rwlock_write(&other->fl_lock);
		if (!rwlock_trywrite(&self->fl_lock)) {
			rwlock_endwrite(&other->fl_lock);
			goto write_self_again;
		}
	}
#endif /* !CONFIG_NO_THREADS */
	/* Transfer objects. */
	MEMCPY_PTR(items, self->fl_elem, self->fl_size);          /* Backup old objects. */
	MEMCPY_PTR(self->fl_elem, other->fl_elem, self->fl_size); /* Copy new objects. */
	MEMSET_PTR(other->fl_elem, 0, self->fl_size);             /* Steal references. */
	rwlock_endwrite(&other->fl_lock);
	rwlock_endwrite(&self->fl_lock);
	/* Drop references to all of the old items. */
	for (i = 0; i < self->fl_size; ++i)
		Dee_XDecref(items[i]);
	Dee_AFree(items);
	return 0;
/*
err_items:
	Dee_AFree(items);*/
err:
	return -1;
}

PRIVATE int DCALL
fl_deepload(FixedList *__restrict self) {
	size_t i;
	for (i = 0; i < self->fl_size; ++i) {
		if (DeeObject_XInplaceDeepCopyWithLock(&self->fl_elem[i],
		                                       &self->fl_lock))
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE int DCALL fl_bool(FixedList *__restrict self) {
	return self->fl_size != 0;
}

PRIVATE void DCALL
fl_visit(FixedList *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	rwlock_read(&self->fl_lock);
	for (i = 0; i < self->fl_size; ++i)
		Dee_XVisit(self->fl_elem[i]);
	rwlock_endread(&self->fl_lock);
}

PRIVATE void DCALL
fl_clear(FixedList *__restrict self) {
	size_t i, buflen = 0;
	DREF DeeObject *buf[16];
again:
	rwlock_write(&self->fl_lock);
	for (i = 0; i < self->fl_size; ++i) {
		DREF DeeObject *ob;
		ob = self->fl_elem[i];
		if (!ob)
			continue;
		self->fl_elem[i] = NULL;
		if (buflen >= COMPILER_LENOF(buf)) {
			rwlock_endwrite(&self->fl_lock);
			Dee_Decref(ob);
			while (buflen--)
				Dee_Decref(buf[buflen]);
			goto again;
		}
		buf[buflen++] = ob; /* Inherit reference. */
	}
	rwlock_endwrite(&self->fl_lock);
	/* Drop all of the references we took. */
	while (buflen--)
		Dee_Decref(buf[buflen]);
}

PRIVATE void DCALL
fl_pclear(FixedList *__restrict self, unsigned int gc_priority) {
	size_t i, buflen = 0;
	DREF DeeObject *buf[16];
again:
	rwlock_write(&self->fl_lock);
	for (i = 0; i < self->fl_size; ++i) {
		DREF DeeObject *ob;
		ob = self->fl_elem[i];
		if (!ob)
			continue;
		if (DeeObject_GCPriority(ob) < gc_priority)
			continue; /* Ignore this object (for now) */
		self->fl_elem[i] = NULL;
		if (buflen >= COMPILER_LENOF(buf)) {
			rwlock_endwrite(&self->fl_lock);
			Dee_Decref(ob);
			while (buflen--)
				Dee_Decref(buf[buflen]);
			goto again;
		}
		buf[buflen++] = ob; /* Inherit reference. */
	}
	rwlock_endwrite(&self->fl_lock);
	/* Drop all of the references we took. */
	while (buflen--)
		Dee_Decref(buf[buflen]);
}


PRIVATE struct type_gc fl_gc = {
	/* .tp_clear  = */ (void(DCALL *)(DeeObject *__restrict))&fl_clear,
	/* .tp_pclear = */ (void(DCALL *)(DeeObject *__restrict, unsigned int))&fl_pclear
};


PRIVATE size_t DCALL
fl_nsi_size(FixedList *__restrict self) {
	return self->fl_size;
}

PRIVATE DREF DeeObject *DCALL
fl_size(FixedList *__restrict self) {
	return DeeInt_NewSize(self->fl_size);
}

PRIVATE DREF DeeObject *DCALL
fl_nsi_getitem(FixedList *__restrict self, size_t index) {
	DREF DeeObject *result;
	if
		unlikely(index >= self->fl_size)
	{
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	rwlock_read(&self->fl_lock);
	result = self->fl_elem[index];
	if
		unlikely(!result)
	{
		rwlock_endread(&self->fl_lock);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(result);
	rwlock_endread(&self->fl_lock);
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
fl_nsi_getitem_fast(FixedList *__restrict self, size_t index) {
	DREF DeeObject *result;
	ASSERT(index < self->fl_size);
	rwlock_read(&self->fl_lock);
	result = self->fl_elem[index];
	Dee_XIncref(result);
	rwlock_endread(&self->fl_lock);
	return result;
}

PRIVATE int DCALL
fl_nsi_delitem(FixedList *__restrict self, size_t index) {
	DREF DeeObject *oldval;
	if
		unlikely(index >= self->fl_size)
	{
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	rwlock_read(&self->fl_lock);
	oldval = self->fl_elem[index];
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	if
		unlikely(!oldval)
	{
		rwlock_endread(&self->fl_lock);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	self->fl_elem[index] = NULL;
	rwlock_endread(&self->fl_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	Dee_Decref(oldval);
#else  /* CONFIG_ERROR_DELETE_UNBOUND */
	Dee_XDecref(oldval);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
fl_nsi_setitem(FixedList *__restrict self, size_t index,
               DeeObject *__restrict value) {
	DREF DeeObject *oldval;
	if
		unlikely(index >= self->fl_size)
	{
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	Dee_Incref(value);
	rwlock_read(&self->fl_lock);
	oldval               = self->fl_elem[index];
	self->fl_elem[index] = value;
	rwlock_endread(&self->fl_lock);
	Dee_XDecref(oldval);
	return 0;
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
fl_nsi_xchitem(FixedList *__restrict self, size_t index,
               DeeObject *__restrict value) {
	DREF DeeObject *result;
	if
		unlikely(index >= self->fl_size)
	{
		err_index_out_of_bounds((DeeObject *)self, index, self->fl_size);
		goto err;
	}
	rwlock_read(&self->fl_lock);
	result = self->fl_elem[index];
	if
		unlikely(!result)
	{
		rwlock_endread(&self->fl_lock);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(value);
	self->fl_elem[index] = value;
	rwlock_endread(&self->fl_lock);
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
fl_getitem(FixedList *__restrict self,
           DeeObject *__restrict index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return fl_nsi_getitem(self, index);
err:
	return NULL;
}

PRIVATE int DCALL
fl_delitem(FixedList *__restrict self,
           DeeObject *__restrict index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return fl_nsi_delitem(self, index);
err:
	return -1;
}

PRIVATE int DCALL
fl_setitem(FixedList *__restrict self,
           DeeObject *__restrict index_ob,
           DeeObject *__restrict value) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return fl_nsi_setitem(self, index, value);
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
fl_contains(FixedList *__restrict self,
            DeeObject *__restrict value) {
	size_t i;
	for (i = 0; i < self->fl_size; ++i) {
		DREF DeeObject *elem;
		int temp;
		rwlock_read(&self->fl_lock);
		elem = self->fl_elem[i];
		Dee_Incref(elem);
		rwlock_endread(&self->fl_lock);
		temp = DeeObject_CompareEq(value, elem);
		Dee_Decref(elem);
		if (temp != 0) {
			if
				unlikely(temp < 0)
			goto err;
			return_true;
		}
	}
	return_false;
err:
	return NULL;
}

PRIVATE DREF FixedListIterator *DCALL
fl_iter(FixedList *__restrict self) {
	DREF FixedListIterator *result;
	result = DeeObject_MALLOC(FixedListIterator);
	if
		unlikely(!result)
	goto done;
	result->li_list = self;
	result->li_iter = 0;
	Dee_Incref(self);
	DeeObject_Init(result, &FixedListIterator_Type);
done:
	return result;
}


PRIVATE struct type_nsi fl_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&fl_nsi_size,
			/* .nsi_getsize_fast = */ (void *)&fl_nsi_size,
			/* .nsi_getitem      = */ (void *)&fl_nsi_getitem,
			/* .nsi_delitem      = */ (void *)&fl_nsi_delitem,
			/* .nsi_setitem      = */ (void *)&fl_nsi_setitem,
			/* .nsi_getitem_fast = */ (void *)&fl_nsi_getitem_fast,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL, /* TODO */
			/* .nsi_setrange_n   = */ (void *)NULL, /* TODO */
			/* .nsi_find         = */ (void *)NULL, /* TODO */
			/* .nsi_rfind        = */ (void *)NULL, /* TODO */
			/* .nsi_xch          = */ (void *)&fl_nsi_xchitem,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL, /* TODO */
			/* .nsi_erase        = */ (void *)NULL, /* TODO */
			/* .nsi_remove       = */ (void *)NULL, /* TODO */
			/* .nsi_rremove      = */ (void *)NULL, /* TODO */
			/* .nsi_removeall    = */ (void *)NULL, /* TODO */
			/* .nsi_removeif     = */ (void *)NULL  /* TODO */
		}
	}
};

PRIVATE struct type_seq fl_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&fl_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&fl_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fl_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fl_getitem,
	/* .tp_del       = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fl_delitem,
	/* .tp_set       = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict))&fl_setitem,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict))NULL,
	/* .tp_range_del = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict))NULL,
	/* .tp_range_set = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict))NULL,
	/* .tp_nsi       = */ &fl_nsi
};

PRIVATE DREF DeeObject *DCALL
fl_clear_meth(FixedList *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	fl_clear(self);
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
fl_sizeof(FixedList *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	return DeeInt_NewSize(offsetof(FixedList, fl_elem) +
	                      (self->fl_size * sizeof(DREF DeeObject *)));
err:
	return NULL;
}

PRIVATE struct type_method fl_methods[] = {
	{ "clear",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & fl_clear_meth,
	  DOC("()") },
	{ "__sizeof__",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & fl_sizeof,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_member fl_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &FixedListIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject FixedList_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "FixedList",
	/* .tp_doc      = */ DOC("A mutable, but fixed-length sequence type, functioning as "
	                         "a sort-of hybrid between :deemon.List and :deemon.tuple\n"
	                         "\n"
	                         "()\n"
	                         "Construct a fixed list that is empty\n"
	                         "\n"
	                         "(size:?Dint,init?)\n"
	                         "Create an pre-sized list of @size elements, all initialized "
	                         "as @init, or as unbound when no @init is given\n"
	                         "\n"
	                         "(seq:?S?O)\n"
	                         "Construct a FixedList from items taken from the given @seq\n"
	                         "\n"
	                         "copy->\n"
	                         "Returns a shallow copy of @this FixedList\n"
	                         "\n"
	                         "deepcopy->\n"
	                         "Returns a deep copy of @this FixedList\n"
	                         "\n"
	                         ":=(other:?S?O)->\n"
	                         "@throw UnpackError @other has a different length than @this\n"
	                         "Assign all the elements from @other to @this FixedList\n"
	                         "\n"
	                         "move:=->\n"
	                         "@throw UnpackError @other has a different length than @this\n"
	                         "Move all the elements from @other into @this FixedList, changing all of them to unbound in @other\n"
	                         "\n"
	                         "bool->\n"
	                         "Returns :true if @this FixedList is non-empty. :false otherwise\n"
	                         "\n"
	                         "iter->\n"
	                         "Returns an iterator for enumerating the elements of @this FixedList in ascending order\n"
	                         "\n"
	                         "[]->\n"
	                         "@throw IndexError @index is greater that the length of @this FixedList\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Returns the @index'th item of @this FixedList\n"
	                         "\n"
	                         "[]=->\n"
	                         "@throw IndexError @index is greater that the length of @this FixedList\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Set the @index'th item of @this FixedList to @item\n"
	                         "\n"
	                         "del[]->\n"
	                         "@throw IndexError @index is greater that the length of @this FixedList\n"
	                         "@throw IntegerOverflow @index is negative or too large\n"
	                         "Mark the @index'th element of @this FixedList as unbound\n"
	                         "\n"
	                         "contains->\n"
	                         "Returns :true if @item is apart of @this FixedList, :false otherwise"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC|TP_FVARIABLE,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(FixedList),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (void *)&fl_ctor,
				/* .tp_copy_ctor = */ (void *)&fl_copy,
				/* .tp_deep_ctor = */ (void *)&fl_copy,
				/* .tp_any_ctor  = */ (void *)&fl_init,
				/* .tp_free      = */ NULL
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&fl_fini,
		/* .tp_assign      = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fl_assign,
		/* .tp_move_assign = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fl_moveassign,
		/* .tp_deepload    = */ (int(DCALL *)(DeeObject *__restrict))&fl_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int(DCALL *)(DeeObject *__restrict))&fl_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&fl_visit,
	/* .tp_gc            = */ &fl_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &fl_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ fl_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ fl_class_members
};




#ifdef CONFIG_NO_THREADS
#define FLI_GETITER(x)             ((x)->li_iter)
#else /* CONFIG_NO_THREADS */
#define FLI_GETITER(x)  ATOMIC_READ((x)->li_iter)
#endif /* !CONFIG_NO_THREADS */

PRIVATE int DCALL
fli_ctor(FixedListIterator *__restrict self) {
	self->li_iter = 0;
	self->li_list = fl_ctor();
	return likely(self->li_list)
	? 0 : -1;
}

PRIVATE int DCALL
fli_copy(FixedListIterator *__restrict self,
         FixedListIterator *__restrict other) {
	self->li_list = other->li_list;
	self->li_iter = FLI_GETITER(other);
	Dee_Incref(self->li_list);
	return 0;
}

PRIVATE int DCALL
fli_deep(FixedListIterator *__restrict self,
         FixedListIterator *__restrict other) {
	self->li_iter = FLI_GETITER(other);
	self->li_list = (DREF FixedList *)DeeObject_DeepCopy((DeeObject *)other->li_list);
	if
		unlikely(!self->li_list)
	goto err;
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
fli_init(FixedListIterator *__restrict self,
         size_t argc, DeeObject **__restrict argv) {
	self->li_iter = 0;
	if (DeeArg_Unpack(argc, argv, "o|Iu:FixedListIterator", &self->li_list, &self->li_iter))
		goto err;
	if (DeeObject_AssertTypeExact(self->li_list, &FixedList_Type))
		goto err;
	Dee_Incref(self->li_list);
	return 0;
err:
	return -1;
}

PRIVATE void DCALL
fli_fini(FixedListIterator *__restrict self) {
	Dee_Decref(self->li_list);
}

PRIVATE void DCALL
fli_visit(FixedListIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->li_list);
}

#define DEFINE_FLI_COMPARE(name, op)                                                \
	PRIVATE DREF DeeObject *DCALL                                                   \
	name(FixedListIterator *__restrict self,                                        \
	     FixedListIterator *__restrict other) {                                     \
		if (DeeObject_AssertTypeExact((DeeObject *)other, &FixedListIterator_Type)) \
			return NULL;                                                            \
		return_bool(FLI_GETITER(self) op FLI_GETITER(other));                       \
	}
DEFINE_FLI_COMPARE(fli_eq, ==)
DEFINE_FLI_COMPARE(fli_ne, !=)
DEFINE_FLI_COMPARE(fli_lo, <)
DEFINE_FLI_COMPARE(fli_le, <=)
DEFINE_FLI_COMPARE(fli_gr, >)
DEFINE_FLI_COMPARE(fli_ge, >=)
#undef DEFINE_FLI_COMPARE

PRIVATE struct type_cmp fli_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fli_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fli_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fli_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fli_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fli_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&fli_ge,
	/* TODO: NII Support */
};


PRIVATE struct type_member fli_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(FixedListIterator, li_list), "->?GFixedList"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_OBJECT, offsetof(FixedListIterator, li_iter)),
	TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
fli_next(FixedListIterator *__restrict self) {
	DREF DeeObject *result;
	size_t iter, newiter;
	FixedList *list = self->li_list;
#ifndef CONFIG_NO_THREADS
again:
#endif /* !CONFIG_NO_THREADS */
	iter = FLI_GETITER(self);
	if (iter >= list->fl_size)
		return ITER_DONE;
	newiter = iter;
	rwlock_read(&list->fl_lock);
	for (;; ++newiter) {
		if (newiter >= list->fl_size) {
			rwlock_endread(&list->fl_lock);
			return ITER_DONE;
		}
		result = list->fl_elem[newiter];
		if (result)
			break;
	}
	Dee_Incref(result);
	rwlock_endread(&list->fl_lock);
#ifdef CONFIG_NO_THREADS
	self->li_iter = newiter + 1;
#else /* CONFIG_NO_THREADS */
	if (!ATOMIC_CMPXCH(self->li_iter, iter, newiter + 1)) {
		Dee_Decref(result);
		goto again;
	}
#endif /* !CONFIG_NO_THREADS */
	return result;
}


INTERN DeeTypeObject FixedListIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "FixedListIterator",
	/* .tp_doc      = */ DOC("(seq?:?GFixedList)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&fli_ctor,
				/* .tp_copy_ctor = */ (void *)&fli_copy,
				/* .tp_deep_ctor = */ (void *)&fli_deep,
				/* .tp_any_ctor  = */ (void *)&fli_init,
				TYPE_FIXED_ALLOCATOR(FixedListIterator)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&fli_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&fli_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: bi-directional iterator support */
	/* .tp_cmp           = */ &fli_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&fli_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ fli_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_FIXEDLIST_C */
