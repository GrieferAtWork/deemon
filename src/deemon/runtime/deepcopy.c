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
#ifndef GUARD_DEEMON_RUNTIME_DEEPCOPY_C
#define GUARD_DEEMON_RUNTIME_DEEPCOPY_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/deepcopy.h>
#include <deemon/error-rt.h>
#include <deemon/gc.h>
#include <deemon/object.h>
#include <deemon/serial.h>
#include <deemon/system-features.h> /* memset, memcpy, ... */
#include <deemon/types.h>

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, ptrdiff_t, size_t */
#include <stdint.h>  /* uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

/* "Dee_seraddr_t" is just a regular pointer when it comes to deepcopy */
#define deepcopy_ptr2ser(self, ptr)     ((Dee_seraddr_t)(ptr))
#define deepcopy_ser2ptr(self, addr, T) ((T *)(addr))

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

/* Instruct the deepcopy context to replace pointers/objects
 * in [optr,+=num_bytes) with nptr+OFFSET. Used to implement
 * various operators from `struct Dee_serial_type'.
 *
 * @return: 0 : Success
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
deepcopy_mappointer(DeeDeepCopyContext *__restrict self,
                    void const *optr, void *nptr,
                    size_t num_bytes, bool do_try) {
	size_t lo, hi;
	struct Dee_deepcopy_mapitem *map;
	ASSERT(self->dcc_ptrmapc <= self->dcc_ptrmapa);
	if (self->dcc_ptrmapc >= self->dcc_ptrmapa) {
		size_t new_alloc = self->dcc_ptrmapa * 2;
		struct Dee_deepcopy_mapitem *newmap;
		if (new_alloc < 16)
			new_alloc = 16;
		if (new_alloc <= self->dcc_ptrmapc)
			new_alloc = self->dcc_ptrmapc + 1;
		newmap = (struct Dee_deepcopy_mapitem *)Dee_TryReallocc(self->dcc_ptrmapv, new_alloc,
		                                                        sizeof(struct Dee_deepcopy_mapitem));
		if unlikely(!newmap) {
			new_alloc = self->dcc_ptrmapc + 1;
			newmap = (struct Dee_deepcopy_mapitem *)Dee_TryReallocc(self->dcc_ptrmapv, new_alloc,
			                                                        sizeof(struct Dee_deepcopy_mapitem));
			if unlikely(!newmap) {
				if (do_try)
					goto err;
				newmap = (struct Dee_deepcopy_mapitem *)Dee_Reallocc(self->dcc_ptrmapv, new_alloc,
				                                                     sizeof(struct Dee_deepcopy_mapitem));
				if unlikely(!newmap)
					goto err;
			}
		}
		self->dcc_ptrmapa = new_alloc;
		self->dcc_ptrmapv = newmap;
	}

	map = self->dcc_ptrmapv;
	lo  = 0;
	hi  = self->dcc_ptrmapc;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct Dee_deepcopy_mapitem *ent = &map[mid];
		if ((byte_t *)optr > ent->dcmi_old_maxaddr) {
			lo = mid + 1;
		} else {
			ASSERTF(((byte_t *)optr + num_bytes - 1) < ent->dcmi_old_minaddr,
			        "Cannot map address range %p-%p is supported to %p: "
			        /**/ "range %p-%p is already mapped to %p",
			        (byte_t *)optr, (byte_t *)optr + num_bytes - 1,
			        ent->dcmi_old_minaddr, ent->dcmi_old_maxaddr);
			hi = mid;
		}
	}

	/* Insert at "lo == hi" */
	ASSERT(lo == hi);
	map += lo;
	hi = self->dcc_ptrmapc - hi;
	memmoveupc(map + 1, map, hi, sizeof(struct Dee_deepcopy_mapitem));
	++self->dcc_ptrmapc;
	map->dcmi_old_minaddr = (byte_t *)optr;
	map->dcmi_old_maxaddr = (byte_t *)optr + num_bytes - 1;
	map->dcmi_new         = (byte_t *)nptr;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
deepcopy_unmappointer(DeeDeepCopyContext *__restrict self, void const *optr) {
	size_t lo, hi;
	struct Dee_deepcopy_mapitem *map;
	ASSERT(self->dcc_ptrmapc <= self->dcc_ptrmapa);
	map = self->dcc_ptrmapv;
	lo  = 0;
	hi  = self->dcc_ptrmapc;
	for (;;) {
		size_t mid = (lo + hi) / 2;
		struct Dee_deepcopy_mapitem *ent = &map[mid];
		ASSERTF(lo < hi, "Unable to find mapping for %p", optr);
		if ((byte_t *)optr > ent->dcmi_old_maxaddr) {
			lo = mid + 1;
		} else if ((byte_t *)optr < ent->dcmi_old_minaddr) {
			hi = mid;
		} else {
			/* Remove "ent" at "mid" */
			--self->dcc_ptrmapc;
			mid = self->dcc_ptrmapc - mid;
			memmovedownc(ent + 1, ent, hi, sizeof(struct Dee_deepcopy_mapitem));
			break;
		}
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
deepcopy_addweakref(DeeDeepCopyContext *__restrict self,
                    struct Dee_weakref *__restrict wref) {
	size_t old_alloc;
#ifdef Dee_MallocUsableSize
	old_alloc = Dee_MallocUsableSize(self->dcc_weakrefv) / sizeof(struct Dee_weakref *);
#else /* Dee_MallocUsableSize */
	old_alloc = self->dcc_weakrefc;
#endif /* !Dee_MallocUsableSize */
	ASSERT(old_alloc >= self->dcc_weakrefc);
	if (old_alloc <= self->dcc_weakrefc) {
		size_t new_alloc = self->dcc_weakrefc + 1;
		struct Dee_weakref **vec = (struct Dee_weakref **)Dee_Reallocc(self->dcc_weakrefv, new_alloc,
		                                                               sizeof(struct Dee_weakref *));
		if unlikely(!vec)
			goto err;
		self->dcc_weakrefv = vec;
	}
	self->dcc_weakrefv[self->dcc_weakrefc++] = wref;
	return 0;
err:
	return -1;
}


PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) void *DCALL
deepcopy_addr2mem(DeeDeepCopyContext *__restrict self, Dee_seraddr_t addr) {
	(void)self;
	return deepcopy_ser2ptr(self, addr, void);
}

LOCAL WUNUSED NONNULL((1)) void *DCALL
deepcopy_heap_malloc(Dee_deepcopy_heap_t **__restrict p_heap,
                     size_t num_bytes, bool do_try, bool do_bzero) {
	void *result;
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
	num_bytes += sizeof(void *);
#endif /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
	result = do_bzero ? (do_try ? Dee_TryCalloc(num_bytes) : Dee_Calloc(num_bytes))
	                  : (do_try ? Dee_TryMalloc(num_bytes) : Dee_Malloc(num_bytes));
	if likely(result) {
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
		Dee_deepcopy_heap_setnext(result, *p_heap);
		*p_heap = result;
#else /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
		Dee_deepcopy_heap_t *chunk;
		chunk = (do_try ? Dee_deepcopy_heap_tryallocchunk()
		                : Dee_deepcopy_heap_allocchunk());
		if unlikely(!chunk) {
			Dee_Free(result);
			return NULL;
		}
		chunk->ddch_base = result;
		chunk->ddch_next = *p_heap;
		*p_heap = chunk;
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
	}
	return result;
}

LOCAL NONNULL((1, 2)) void DCALL
deepcopy_heap_free(Dee_deepcopy_heap_t **__restrict p_heap, void *ptr) {
	Dee_deepcopy_heap_t *heap = *p_heap;
	ASSERT(heap);
	ASSERTF(Dee_deepcopy_heap_getbase(heap) == ptr,
	        "Only ever allowed to free the most-recent allocation!");
	*p_heap = Dee_deepcopy_heap_getnext(heap);
	Dee_deepcopy_heap_destroy(heap);
}

LOCAL WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
deepcopy_malloc_impl(DeeDeepCopyContext *__restrict self,
                     size_t num_bytes, void const *ref,
                     bool do_try, bool do_bzero) {
	void *result = deepcopy_heap_malloc(&self->dcc_heap, num_bytes, do_try, do_bzero);
	if unlikely(!result)
		goto err;
	if (ref && unlikely(deepcopy_mappointer(self, ref, result, num_bytes, do_try)))
		goto err_r;
	return deepcopy_ptr2ser(self, result);
err_r:
	deepcopy_heap_free(&self->dcc_heap, result);
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
deepcopy_malloc(DeeDeepCopyContext *__restrict self,
                size_t num_bytes, void const *ref) {
	return deepcopy_malloc_impl(self, num_bytes, ref, false, false);
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
deepcopy_trymalloc(DeeDeepCopyContext *__restrict self,
                   size_t num_bytes, void const *ref) {
	return deepcopy_malloc_impl(self, num_bytes, ref, true, false);
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
deepcopy_calloc(DeeDeepCopyContext *__restrict self,
                size_t num_bytes, void const *ref) {
	return deepcopy_malloc_impl(self, num_bytes, ref, false, true);
}

PRIVATE WUNUSED NONNULL((1)) Dee_seraddr_t DCALL
deepcopy_trycalloc(DeeDeepCopyContext *__restrict self,
                   size_t num_bytes, void const *ref) {
	return deepcopy_malloc_impl(self, num_bytes, ref, true, true);
}


typedef struct {
	OBJECT_HEAD
} GenericObject;

LOCAL WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_object_malloc_impl(DeeDeepCopyContext *__restrict self,
                            size_t num_bytes, DeeObject *__restrict ref,
                            bool do_try, bool do_bzero) {
	GenericObject *result;
	ASSERT_OBJECT(ref);
	ASSERTF(!DeeType_IsGC(Dee_TYPE(ref)), "Use deepcopy_gcobject_malloc_impl()");
	result = (GenericObject *)deepcopy_heap_malloc(&self->dcc_obheap, num_bytes, do_try, do_bzero);
	if unlikely(!result)
		goto err;
	if unlikely(deepcopy_mappointer(self, ref, result, num_bytes, do_try))
		goto err_r;
	DeeObject_Init(result, Dee_TYPE(ref));
	return deepcopy_ptr2ser(self, result);
err_r:
	deepcopy_heap_free(&self->dcc_obheap, result);
err:
	return Dee_SERADDR_INVALID;
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_object_malloc(DeeDeepCopyContext *__restrict self,
                       size_t num_bytes, DeeObject *__restrict ref) {
	return deepcopy_object_malloc_impl(self, num_bytes, ref, false, false);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_object_trymalloc(DeeDeepCopyContext *__restrict self,
                          size_t num_bytes, DeeObject *__restrict ref) {
	return deepcopy_object_malloc_impl(self, num_bytes, ref, true, false);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_object_calloc(DeeDeepCopyContext *__restrict self,
                       size_t num_bytes, DeeObject *__restrict ref) {
	return deepcopy_object_malloc_impl(self, num_bytes, ref, false, true);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_object_trycalloc(DeeDeepCopyContext *__restrict self,
                          size_t num_bytes, DeeObject *__restrict ref) {
	return deepcopy_object_malloc_impl(self, num_bytes, ref, true, true);
}


LOCAL WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_gcobject_malloc_impl(DeeDeepCopyContext *__restrict self,
                              size_t num_bytes, DeeObject *__restrict ref,
                              bool do_try, bool do_bzero) {
	GenericObject *result;
	struct Dee_gc_head *result_head, *next;
	ASSERT_OBJECT(ref);
	ASSERTF(DeeType_IsGC(Dee_TYPE(ref)), "Use deepcopy_object_malloc_impl()");
	num_bytes += DEE_GC_HEAD_SIZE;
	result_head = (struct Dee_gc_head *)deepcopy_heap_malloc(&self->dcc_gcheap, num_bytes, do_try, do_bzero);
	if unlikely(!result_head)
		goto err;
	if unlikely(deepcopy_mappointer(self, DeeGC_Head(ref), result_head, num_bytes, do_try))
		goto err_r;
	result = (GenericObject *)&result_head->gc_object;
	DeeObject_Init(result, Dee_TYPE(ref));

	/* Insert GC head */
	result_head->gc_pself = NULL;
	if ((next = self->dcc_gc_head) != NULL) {
		ASSERT(self->dcc_gc_tail != NULL);
		next->gc_pself = &result_head->gc_next;
	} else {
		ASSERT(self->dcc_gc_tail == NULL);
		self->dcc_gc_tail = result_head;
	}
	result_head->gc_next = next;
	self->dcc_gc_head = result_head;
	return deepcopy_ptr2ser(self, result);
err_r:
	deepcopy_heap_free(&self->dcc_gcheap, result_head);
err:
	return Dee_SERADDR_INVALID;
}


PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_gcobject_malloc(DeeDeepCopyContext *__restrict self,
                         size_t num_bytes, DeeObject *__restrict ref) {
	return deepcopy_gcobject_malloc_impl(self, num_bytes, ref, false, false);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_gcobject_trymalloc(DeeDeepCopyContext *__restrict self,
                            size_t num_bytes, DeeObject *__restrict ref) {
	return deepcopy_gcobject_malloc_impl(self, num_bytes, ref, true, false);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_gcobject_calloc(DeeDeepCopyContext *__restrict self,
                         size_t num_bytes, DeeObject *__restrict ref) {
	return deepcopy_gcobject_malloc_impl(self, num_bytes, ref, false, true);
}

PRIVATE WUNUSED NONNULL((1, 3)) Dee_seraddr_t DCALL
deepcopy_gcobject_trycalloc(DeeDeepCopyContext *__restrict self,
                            size_t num_bytes, DeeObject *__restrict ref) {
	return deepcopy_gcobject_malloc_impl(self, num_bytes, ref, true, true);
}

/* Free a heap pointer
 * CAUTION: Only the most-recent pointer can *actually* be free'd!
 *          If you pass anything else, this function is a no-op! */
PRIVATE NONNULL((1)) void DCALL
deepcopy_free(DeeDeepCopyContext *__restrict self,
              Dee_seraddr_t addr, void const *ref) {
	if (ref)
		deepcopy_unmappointer(self, ref);
	deepcopy_heap_free(&self->dcc_heap, deepcopy_ser2ptr(self, addr, void));
}

PRIVATE NONNULL((1, 3)) void DCALL
deepcopy_object_free(DeeDeepCopyContext *__restrict self,
                     Dee_seraddr_t addr, DeeObject *__restrict ref) {
	DeeObject *out = deepcopy_ser2ptr(self, addr, DeeObject);
	deepcopy_unmappointer(self, ref);
	Dee_Decref_unlikely(out->ob_type);
	deepcopy_heap_free(&self->dcc_obheap, out);
}

PRIVATE NONNULL((1, 3)) void DCALL
deepcopy_gcobject_free(DeeDeepCopyContext *__restrict self,
                       Dee_seraddr_t addr, DeeObject *__restrict ref) {
	struct Dee_gc_head *next;
	struct Dee_gc_head *out = deepcopy_ser2ptr(self, addr, struct Dee_gc_head);
	deepcopy_unmappointer(self, ref);
	ASSERT(self->dcc_gc_head == out);
	if ((next = out->gc_next) == NULL) {
		ASSERT(self->dcc_gc_tail == out);
		self->dcc_gc_head = NULL;
		self->dcc_gc_tail = NULL;
	} else {
		ASSERT(self->dcc_gc_tail != out);
		self->dcc_gc_head = next;
		next->gc_pself    = NULL;
	}
	Dee_Decref_unlikely(out->gc_object.ob_type);
	deepcopy_heap_free(&self->dcc_gcheap, out);
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
deepcopy_putaddr(DeeDeepCopyContext *__restrict self,
                 Dee_seraddr_t addrof_pointer,
                 Dee_seraddr_t addrof_target) {
	(void)self;
	*deepcopy_ser2ptr(self, addrof_pointer, void *) = deepcopy_ser2ptr(self, addrof_target, void);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
deepcopy_putobject_ex(DeeDeepCopyContext *__restrict self,
                      Dee_seraddr_t addrof_object,
                      DeeObject *__restrict obj,
                      ptrdiff_t offset_into_ob) {
	if ((obj = DeeDeepCopy_CopyObject(self, obj)) == NULL)
		goto err;
	*deepcopy_ser2ptr(self, addrof_object, byte_t *) = (byte_t *)obj + offset_into_ob;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
deepcopy_putobject(DeeDeepCopyContext *__restrict self,
                   Dee_seraddr_t addrof_object,
                   DeeObject *__restrict obj) {
	if ((obj = DeeDeepCopy_CopyObject(self, obj)) == NULL)
		goto err;
	*deepcopy_ser2ptr(self, addrof_object, DeeObject *) = obj;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
deepcopy_putpointer(DeeDeepCopyContext *__restrict self,
                    Dee_seraddr_t addrof_pointer,
                    void const *pointer) {
	/* Check if "pointer" falls into an address range that should be replaced */
	size_t lo = 0, hi = self->dcc_ptrmapc;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct Dee_deepcopy_mapitem *ent = &self->dcc_ptrmapv[mid];
		if ((byte_t *)pointer < ent->dcmi_old_minaddr) {
			hi = mid;
		} else if ((byte_t *)pointer > ent->dcmi_old_maxaddr) {
			lo = mid + 1;
		} else {
			/* Object has already been encoded */
			uintptr_t offset = (uintptr_t)((byte_t *)pointer - ent->dcmi_old_minaddr);
			pointer = (byte_t *)(ent->dcmi_new + offset);
			break;
		}
	}

	/* Assign pointer... */
	*deepcopy_ser2ptr(self, addrof_pointer, void const *) = pointer;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
deepcopy_putweakref_ex(DeeDeepCopyContext *__restrict self,
                       Dee_seraddr_t addrof_weakref,
                       DeeObject *__restrict ob,
                       Dee_weakref_callback_t del) {
	struct Dee_weakref *out = deepcopy_ser2ptr(self, addrof_weakref, struct Dee_weakref);
	int result = deepcopy_addweakref(self, out);
	if likely(result == 0) {
		Dee_Incref(ob);
		out->wr_obj = ob;
		out->wr_del = del;
	}
	return 0;
}


PRIVATE struct Dee_serial_type tpconst deepcopy_serial_type = {
	/* .set_addr2mem           = */ (void *(DCALL *)(DeeSerial *__restrict, Dee_seraddr_t))&deepcopy_addr2mem,
	/* .set_malloc             = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&deepcopy_malloc,
	/* .set_calloc             = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&deepcopy_calloc,
	/* .set_trymalloc          = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&deepcopy_trymalloc,
	/* .set_trycalloc          = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, void const *))&deepcopy_trycalloc,
	/* .set_free               = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, void const *))&deepcopy_free,
	/* .set_object_malloc      = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&deepcopy_object_malloc,
	/* .set_object_calloc      = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&deepcopy_object_calloc,
	/* .set_object_trymalloc   = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&deepcopy_object_trymalloc,
	/* .set_object_trycalloc   = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&deepcopy_object_trycalloc,
	/* .set_object_free        = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict))&deepcopy_object_free,
	/* .set_gcobject_malloc    = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&deepcopy_gcobject_malloc,
	/* .set_gcobject_calloc    = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&deepcopy_gcobject_calloc,
	/* .set_gcobject_trymalloc = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&deepcopy_gcobject_trymalloc,
	/* .set_gcobject_trycalloc = */ (Dee_seraddr_t (DCALL *)(DeeSerial *__restrict, size_t, DeeObject *__restrict))&deepcopy_gcobject_trycalloc,
	/* .set_gcobject_free      = */ (void (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict))&deepcopy_gcobject_free,
	/* .set_putaddr            = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, Dee_seraddr_t))&deepcopy_putaddr,
	/* .set_putobject          = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict))&deepcopy_putobject,
	/* .set_putobject_ex       = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict, ptrdiff_t))&deepcopy_putobject_ex,
	/* .set_putpointer         = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, void const *__restrict))&deepcopy_putpointer,
	/* .set_putweakref_ex      = */ (int (DCALL *)(DeeSerial *__restrict, Dee_seraddr_t, DeeObject *__restrict, Dee_weakref_callback_t))&deepcopy_putweakref_ex,
};


/* Initialize/finalize a deepcopy context. */
PUBLIC NONNULL((1)) void DCALL
DeeDeepCopy_Init(DeeDeepCopyContext *__restrict self) {
	self->ser_type       = &deepcopy_serial_type;
	self->dcc_heap       = NULL;
	self->dcc_obheap     = NULL;
	self->dcc_gcheap     = NULL;
	self->dcc_ptrmapv    = NULL;
	self->dcc_ptrmapc    = 0;
	self->dcc_ptrmapa    = 0;
	self->dcc_gc_head    = NULL;
	self->dcc_gc_tail    = NULL;
	self->dcc_immutablev = NULL;
	self->dcc_immutablec = 0;
	self->dcc_weakrefv   = NULL;
	self->dcc_weakrefc   = 0;
}

PRIVATE void DCALL
destroy_heap(Dee_deepcopy_heap_t *heap) {
	while (heap) {
		Dee_deepcopy_heap_t *next;
		next = Dee_deepcopy_heap_getnext(heap);
		Dee_deepcopy_heap_destroy(heap);
		heap = next;
	}
}

PRIVATE void DCALL
destroy_obheap(Dee_deepcopy_heap_t *heap, ptrdiff_t offsetof_object) {
	while (heap) {
		Dee_deepcopy_heap_t *next;
		DeeObject *ob = (DeeObject *)((byte_t *)Dee_deepcopy_heap_getbase(heap) + offsetof_object);
		Dee_Decref_unlikely(ob->ob_type);
		next = Dee_deepcopy_heap_getnext(heap);
		Dee_deepcopy_heap_destroy(heap);
		heap = next;
	}
}

PUBLIC NONNULL((1)) void DCALL
DeeDeepCopy_Fini(DeeDeepCopyContext *__restrict self) {
	size_t i;

	/* Free map tables. */
	Dee_Free(self->dcc_ptrmapv);
	if (self->dcc_immutablec == 1) {
		Dee_Decref_unlikely(self->dcc_immutable1);
	} else {
		Dee_Decrefv_unlikely(self->dcc_immutablev, self->dcc_immutablec);
		Dee_Free(self->dcc_immutablev);
	}
	for (i = 0; i < self->dcc_weakrefc; ++i) {
		struct Dee_weakref *wref = self->dcc_weakrefv[i];
		Dee_Decref_unlikely(wref->wr_obj);
	}
	Dee_Free(self->dcc_weakrefv);

	/* Destroy heaps */
	destroy_heap(self->dcc_heap);
	destroy_obheap(self->dcc_obheap, 0);
	destroy_obheap(self->dcc_gcheap, offsetof(struct Dee_gc_head, gc_object));

	DBG_memset(self, 0xcc, sizeof(*self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeDeepCopy_AddImmutable(DeeDeepCopyContext *__restrict self, DeeObject *ob) {
	if (self->dcc_immutablec == 0) {
		self->dcc_immutable1 = ob; /* Reference created below... */
		self->dcc_immutablec = 1;
	} else if (self->dcc_immutablec == 1) {
		DREF DeeObject **vec = (DREF DeeObject **)Dee_Mallocc(2, sizeof(DREF DeeObject *));
		if unlikely(!vec)
			goto err;
		vec[0] = self->dcc_immutable1; /* Inherit reference */
		vec[1] = ob;                   /* Reference created below... */
		self->dcc_immutablev = vec;
		self->dcc_immutablec = 2;
	} else {
		size_t old_alloc;
#ifdef Dee_MallocUsableSize
		old_alloc = Dee_MallocUsableSize(self->dcc_immutablev) / sizeof(DREF DeeObject *);
#else /* Dee_MallocUsableSize */
		old_alloc = self->dcc_immutablec;
#endif /* !Dee_MallocUsableSize */
		ASSERT(old_alloc >= self->dcc_immutablec);
		if (old_alloc <= self->dcc_immutablec) {
			size_t new_alloc = self->dcc_immutablec + 1;
			DREF DeeObject **vec = (DREF DeeObject **)Dee_Reallocc(self->dcc_immutablev, new_alloc, sizeof(DREF DeeObject *));
			if unlikely(!vec)
				goto err;
			self->dcc_immutablev = vec;
		}
		self->dcc_immutablev[self->dcc_immutablec++] = ob; /* Reference created below... */
	}
	Dee_Incref(ob); /* As stored in `self->dcc_immutable1' / `self->dcc_immutablev' */
	return ob;
err:
	return NULL;
}

/* Serialize "ob" into "self" and return a pointer for where "ob" will
 * eventually be initialized once `DeeDeepCopy_Pack()' is called. But
 * until that has been done, this pointer must **NOT** be exposed to
 * user-code, and must similarly not be interacted with, either!
 *
 * When "ob" has already been copied by "self", remember that an extra
 * incref needs to happen and return that pre-existing copy (any distinct
 * object (as per `DeeObject_Id()') will always be copied at most once)
 *
 * @return: * :   Location where the deep-copy of "ob" will appear
 *                When "ob" is immutable, this function may just re-return "ob"
 *                after storing a special reference to "ob" that is dropped if
 *                `DeeDeepCopy_Fini' is called instead of `DeeDeepCopy_Pack'.
 * @return: NULL: Error */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*after(DeeDeepCopy_Pack)*/ DeeObject *DCALL
DeeDeepCopy_CopyObject(DeeDeepCopyContext *__restrict self,
                       DeeObject *__restrict ob) {
	typedef WUNUSED_T NONNULL_T((1, 2)) Dee_seraddr_t
	(DCALL *Dee_tp_serialize_var_t)(DeeObject *__restrict self,
	                                DeeSerial *__restrict writer);
	typedef WUNUSED_T NONNULL_T((1, 2)) int
	(DCALL *Dee_tp_serialize_obj_t)(DeeObject *__restrict self,
	                                DeeSerial *__restrict writer,
	                                Dee_seraddr_t out_addr);
	DeeTypeObject *tp;
	Dee_funptr_t tp_serialize;
	Dee_seraddr_t out_addr;

	/* Check if "ob" falls into an address range that should be replaced */
	size_t lo = 0, hi = self->dcc_ptrmapc;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		struct Dee_deepcopy_mapitem *ent = &self->dcc_ptrmapv[mid];
		if ((byte_t *)ob < ent->dcmi_old_minaddr) {
			hi = mid;
		} else if ((byte_t *)ob > ent->dcmi_old_maxaddr) {
			lo = mid + 1;
		} else {
			/* Object has already been encoded */
			uintptr_t offset = (uintptr_t)((byte_t *)ob - ent->dcmi_old_minaddr);
			DeeObject *result = (DeeObject *)(ent->dcmi_new + offset);
			++result->ob_refcnt;
			return result;
		}
	}

	/* Check if "ob" is considered "deeply immutable" */
	if (DeeObject_IsDeepImmutable(ob))
		return DeeDeepCopy_AddImmutable(self, ob);

	/* Serialize the object (if possible) */
	tp = Dee_TYPE(ob);
	tp_serialize = DeeType_GetTpSerialize(tp);
	if unlikely(!tp_serialize)
		goto cannot_serialize;
	if (tp->tp_flags & TP_FVARIABLE) {
		out_addr = (*(Dee_tp_serialize_var_t)tp_serialize)(ob, (DeeSerial *)self);
		if unlikely(!Dee_SERADDR_ISOK(out_addr))
			goto err;
	} else {
		int status;
		size_t instance_size = DeeType_GetInstanceSize(tp);
		if unlikely(!instance_size)
			goto cannot_serialize;

		/* Allocate buffer for object. */
		out_addr = tp->tp_flags & TP_FGC
		           ? deepcopy_gcobject_malloc(self, instance_size, ob)
		           : deepcopy_object_malloc(self, instance_size, ob);
		if unlikely(!out_addr)
			goto err;

		/* NOTE: Standard fields have already been initialized by "deepcopy_[gc]object_malloc" */
		status = (*(Dee_tp_serialize_obj_t)tp_serialize)(ob, (DeeSerial *)self, out_addr);
		if unlikely(status)
			goto err;
	}
	return deepcopy_ser2ptr(self, out_addr, DeeObject);
cannot_serialize:
	/* Unable to serialize :( */
	DeeRT_ErrCannotSerialize(ob);
err:
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
gc_trackmany_unlockinfo_cb(struct Dee_unlockinfo *__restrict self) {
	(void)self;
	DeeGC_TrackMany_Unlock();
}


#ifndef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
PRIVATE void DCALL
cleanup_heap(Dee_deepcopy_heap_t *heap) {
	while (heap) {
		Dee_deepcopy_heap_t *next;
		next = Dee_deepcopy_heap_getnext(heap);
		Dee_Free(heap);
		heap = next;
	}
}
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

/* Finalize "self" in the sense of doing a COMMIT.
 *
 * This function behaves similar to `DeeDeepCopy_Fini()', but will also
 * (atomically) initialize the return values of `DeeDeepCopy_CopyObject'
 * to resemble valid references to deemon objects (which must then be
 * inherited by the caller(s) of that function)
 *
 * IMPORTANT: Do **NOT** call `DeeDeepCopy_Fini(self)' after this function.
 *            If you want to re-use "self" as a different context, you must
 *            first re-initialize it using `DeeDeepCopy_Init(self)' */
PUBLIC NONNULL((1)) void DCALL
DeeDeepCopy_Pack(/*inherit(always)*/DeeDeepCopyContext *__restrict self) {
	/* Destroy heap links (if used) */
#ifndef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
	cleanup_heap(self->dcc_heap);
	cleanup_heap(self->dcc_obheap);
	cleanup_heap(self->dcc_gcheap);
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

	/* Free map tables. */
	Dee_Free(self->dcc_ptrmapv);
	if (self->dcc_immutablec != 1)
		Dee_Free(self->dcc_immutablev); /* These references get inherited */
	ASSERT((self->dcc_gc_head != NULL) == (self->dcc_gc_tail != NULL));
	if unlikely(self->dcc_weakrefc && self->dcc_gc_head) {
		DREF DeeObject **objv;
		struct Dee_unlockinfo unlock;
		unlock.dui_unlock = &gc_trackmany_unlockinfo_cb;
		do {
			DeeGC_TrackMany_Lock();
		} while (!Dee_weakref_initmany_trylock(self->dcc_weakrefv, self->dcc_weakrefc, &unlock));
		Dee_weakref_initmany_exec(self->dcc_weakrefv, self->dcc_weakrefc);
		DeeGC_TrackMany_Exec(DeeGC_Object(self->dcc_gc_head), DeeGC_Object(self->dcc_gc_tail));
		DeeGC_TrackMany_Unlock();
		objv = Dee_weakref_initmany_unlock_and_inherit(self->dcc_weakrefv, self->dcc_weakrefc);
		Dee_Decrefv_unlikely(objv, self->dcc_weakrefc);
		ASSERT(objv == (DeeObject **)self->dcc_weakrefv);
	} else if (self->dcc_gc_head) {
		DeeGC_TrackMany_Lock();
		DeeGC_TrackMany_Exec(DeeGC_Object(self->dcc_gc_head), DeeGC_Object(self->dcc_gc_tail));
		DeeGC_TrackMany_Unlock();
	} else if unlikely(self->dcc_weakrefc) {
		DREF DeeObject **objv;
		Dee_weakref_initmany_lock(self->dcc_weakrefv, self->dcc_weakrefc);
		Dee_weakref_initmany_exec(self->dcc_weakrefv, self->dcc_weakrefc);
		objv = Dee_weakref_initmany_unlock_and_inherit(self->dcc_weakrefv, self->dcc_weakrefc);
		Dee_Decrefv_unlikely(objv, self->dcc_weakrefc);
		ASSERT(objv == (DeeObject **)self->dcc_weakrefv);
	} else {
		/* No special registration necessary! */
	}
	Dee_Free(self->dcc_weakrefv);
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_DEEPCOPY_C */
