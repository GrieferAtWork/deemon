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
#ifndef GUARD_DEEMON_RUNTIME_SIMPLE_HASHSET_C
#define GUARD_DEEMON_RUNTIME_SIMPLE_HASHSET_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>               /* Dee_Callocc, Dee_CollectMemoryc, Dee_Free, Dee_TryCallocc */
#include <deemon/object.h>              /* DREF, DeeObject, DeeObject_Hash, DeeObject_TryCompareEq, Dee_COMPARE_ISEQ_NO_ERR, Dee_COMPARE_ISERR, Dee_DecrefNokill, Dee_Decref_unlikely, Dee_Incref, Dee_XDecref, Dee_XIncref, Dee_hash_t */
#include <deemon/serial.h>              /* DeeSerial*, Dee_SERADDR_ISOK, Dee_seraddr_t, Dee_serial */
#include <deemon/system-features.h>     /* memcpyc */
#include <deemon/util/lock.h>           /* Dee_atomic_lock_init */
#include <deemon/util/simple-hashset.h> /* Dee_simple_hashset, Dee_simple_hashset_* */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

/* Try to increase the hash-mask by 1 bit */
PRIVATE NONNULL((1)) bool DCALL
Dee_simple_hashset_tryrehash(struct Dee_simple_hashset *__restrict self) {
	Dee_hash_t i, j, perturb;
	struct Dee_simple_hashset_item *new_elem;
	size_t new_mask = self->shs_mask << 1;
	new_elem = (struct Dee_simple_hashset_item *)Dee_TryCallocc(new_mask + 1,
	                                                            sizeof(struct Dee_simple_hashset_item));
	if unlikely(!new_elem)
		return false;
	ASSERT(self->shs_elem);
	for (i = 0; i <= self->shs_mask; ++i) {
		struct Dee_simple_hashset_item *dst_item;
		struct Dee_simple_hashset_item *src_item = &self->shs_elem[i];
		perturb = j = src_item->shsi_hash & new_mask;
		for (;; Dee_simple_hashset_hashnx(j, perturb)) {
			dst_item = &new_elem[j & new_mask];
			if likely(!dst_item->shsi_key)
				break;
		}
		dst_item->shsi_hash = src_item->shsi_hash;
		dst_item->shsi_key  = src_item->shsi_key;  /* Inherit reference */
	}
	Dee_Free(self->shs_elem);
	self->shs_elem = new_elem;
	self->shs_mask = new_mask;
	return true;
}

/* Increase the hash-mask by 1 bit */
PRIVATE WUNUSED NONNULL((1)) int DCALL
Dee_simple_hashset_rehash(struct Dee_simple_hashset *__restrict self) {
	while unlikely(!Dee_simple_hashset_tryrehash(self)) {
		if (!Dee_CollectMemoryc((self->shs_mask << 1) + 1,
		                        sizeof(struct Dee_simple_hashset_item)))
			return -1;
	}
	return 0;
}

/* Insert `item' into `self' (if it wasn't present already)
 * @return:  1: Successfully inserted the object.
 * @return:  0: An identical object already exists.
 * @return: -1: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_simple_hashset_insert(struct Dee_simple_hashset *__restrict self,
                          DeeObject *item) {
	struct Dee_simple_hashset_item *slot;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(item);
	if unlikely(!self->shs_elem) {
		/* Special case: first insert */
		self->shs_mask = 7;
		self->shs_elem = (struct Dee_simple_hashset_item *)Dee_TryCallocc(8, sizeof(struct Dee_simple_hashset_item));
		if unlikely(!self->shs_elem) {
			self->shs_mask = 1;
			self->shs_elem = (struct Dee_simple_hashset_item *)Dee_Callocc(2, sizeof(struct Dee_simple_hashset_item));
			if unlikely(!self->shs_elem)
				goto err;
		}
		i = Dee_simple_hashset_hashst(self, hash);
		slot = &self->shs_elem[i];
		Dee_Incref(item);
		slot->shsi_key  = item;
		slot->shsi_hash = hash;
		self->shs_size  = 1;
		return 1;
	}
	perturb = i = Dee_simple_hashset_hashst(self, hash);
	for (;; Dee_simple_hashset_hashnx(i, perturb)) {
		int cmp;
		slot = Dee_simple_hashset_hashit(self, i);
		if (slot->shsi_key == NULL)
			break; /* Found an empty slot! */
		if (slot->shsi_hash != hash)
			continue;
		cmp = DeeObject_TryCompareEq(item, slot->shsi_key);
		if (Dee_COMPARE_ISEQ_NO_ERR(cmp))
			return 0; /* Identical item exists */
		if (Dee_COMPARE_ISERR(cmp))
			goto err;
	}

	/* Check for special case: a re-hash is mandatory right now */
	if unlikely(self->shs_size >= self->shs_mask) {
		if unlikely(Dee_simple_hashset_rehash(self))
			goto err;
		perturb = i = Dee_simple_hashset_hashst(self, hash);
		for (;; Dee_simple_hashset_hashnx(i, perturb)) {
			slot = Dee_simple_hashset_hashit(self, i);
			if (slot->shsi_key == NULL)
				break; /* Found an empty slot! */
		}
	}

	/* Populate "slot" with the item */
	Dee_Incref(item);
	slot->shsi_key  = item;
	slot->shsi_hash = hash;
	++self->shs_size;

	/* Try to keep the buffer large enough to avoid most hash collisions. */
	if (self->shs_size * 2 > self->shs_mask)
		Dee_simple_hashset_tryrehash(self);
	return 1;
err:
	return -1;
}


#ifndef CONFIG_NO_THREADS
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_simple_hashset_with_lock_insert(struct Dee_simple_hashset_with_lock *__restrict self,
                                    DeeObject *item) {
	size_t mask;
	struct Dee_simple_hashset_item *slot, *elem;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(item);
again:
	Dee_simple_hashset_with_lock_acquire(self);
again_locked:
	if unlikely(!self->shswl_set.shs_elem) {
		/* Special case: first insert */
		mask = 7;
		Dee_simple_hashset_with_lock_release(self);
		elem = (struct Dee_simple_hashset_item *)Dee_TryCallocc(8, sizeof(struct Dee_simple_hashset_item));
		if unlikely(!elem) {
			mask = 1;
			elem = (struct Dee_simple_hashset_item *)Dee_Callocc(2, sizeof(struct Dee_simple_hashset_item));
			if unlikely(!elem)
				goto err;
		}
		slot = &elem[hash & mask];
		Dee_Incref(item);
		slot->shsi_key  = item;
		slot->shsi_hash = hash;

		Dee_simple_hashset_with_lock_acquire(self);
		if likely(!self->shswl_set.shs_elem) {
			self->shswl_set.shs_elem = elem;
			self->shswl_set.shs_mask = mask;
			self->shswl_set.shs_size = 1;
			Dee_simple_hashset_with_lock_release(self);
			return 1;
		}
		Dee_simple_hashset_with_lock_release(self);
		Dee_DecrefNokill(item);
		Dee_Free(elem);
		goto again;
	}

	elem    = self->shswl_set.shs_elem;
	mask    = self->shswl_set.shs_mask;
	perturb = i = hash & mask;
	for (;; Dee_simple_hashset_hashnx(i, perturb)) {
		int cmp;
		DREF DeeObject *slot_key;
		slot = &elem[i & mask];
		if (slot->shsi_key == NULL)
			break; /* Found an empty slot! */
		if (slot->shsi_hash != hash)
			continue;

		slot_key = slot->shsi_key;
		Dee_Incref(slot_key);
		Dee_simple_hashset_with_lock_release(self);
		cmp = DeeObject_TryCompareEq(item, slot_key);
		Dee_Decref_unlikely(slot_key);
		if (Dee_COMPARE_ISEQ_NO_ERR(cmp))
			return 0; /* Identical item exists */
		if (Dee_COMPARE_ISERR(cmp))
			goto err;
		Dee_simple_hashset_with_lock_acquire(self);
		if unlikely(slot->shsi_key != slot_key)
			goto again_locked;
		if unlikely(elem != self->shswl_set.shs_elem)
			goto again_locked;
		if unlikely(mask != self->shswl_set.shs_mask)
			goto again_locked;
	}

	/* Check for special case: a re-hash is mandatory right now */
	if unlikely(self->shswl_set.shs_size >= self->shswl_set.shs_mask) {
		while unlikely(!Dee_simple_hashset_tryrehash(&self->shswl_set)) {
			size_t req = (self->shswl_set.shs_mask << 1) + 1;
			Dee_simple_hashset_with_lock_release(self);
			if (!Dee_CollectMemoryc(req, sizeof(struct Dee_simple_hashset_item)))
				goto err;
			Dee_simple_hashset_with_lock_acquire(self);
			if unlikely(elem != self->shswl_set.shs_elem)
				goto again_locked;
			if unlikely(mask != self->shswl_set.shs_mask)
				goto again_locked;
			if unlikely(slot->shsi_key)
				goto again_locked;
		}
		perturb = i = Dee_simple_hashset_hashst(&self->shswl_set, hash);
		for (;; Dee_simple_hashset_hashnx(i, perturb)) {
			slot = Dee_simple_hashset_hashit(&self->shswl_set, i);
			if (slot->shsi_key == NULL)
				break; /* Found an empty slot! */
		}
	}

	/* Populate "slot" with the item */
	Dee_Incref(item);
	slot->shsi_key  = item;
	slot->shsi_hash = hash;
	++self->shswl_set.shs_size;

	/* Try to keep the buffer large enough to avoid most hash collisions. */
	if (self->shswl_set.shs_size * 2 > self->shswl_set.shs_mask)
		Dee_simple_hashset_tryrehash(&self->shswl_set);
	Dee_simple_hashset_with_lock_release(self);
	return 1;
err:
	return -1;
}
#endif /* !CONFIG_NO_THREADS */


PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_simple_hashset_with_lock_serialize(struct Dee_simple_hashset_with_lock *__restrict self,
                                       struct Dee_serial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(struct Dee_simple_hashset_with_lock, field))
	size_t i, self__shs_mask;
	size_t self__shs_size;
	Dee_seraddr_t out__shs_elem;
	struct Dee_simple_hashset_with_lock *out;
	struct Dee_simple_hashset_item *ou__shs_elem;
	struct Dee_simple_hashset_item *in__shs_elem;
again:
	Dee_simple_hashset_with_lock_acquire(self);
	self__shs_mask = self->shswl_set.shs_mask;
	self__shs_size = self->shswl_set.shs_size;
	out__shs_elem  = DeeSerial_TryMalloc(writer, (self__shs_mask + 1) * sizeof(struct Dee_simple_hashset_item), NULL);
	if (!Dee_SERADDR_ISOK(out__shs_elem)) {
		Dee_simple_hashset_with_lock_release(self);
		out__shs_elem = DeeSerial_Malloc(writer, (self__shs_mask + 1) * sizeof(struct Dee_simple_hashset_item), NULL);
		if (!Dee_SERADDR_ISOK(out__shs_elem))
			goto err;
		Dee_simple_hashset_with_lock_acquire(self);
		if unlikely(self__shs_mask != self->shswl_set.shs_mask ||
		            self__shs_size != self->shswl_set.shs_size) {
			Dee_simple_hashset_with_lock_release(self);
			DeeSerial_Free(writer, out__shs_elem, NULL);
			goto again;
		}
	}
	ou__shs_elem = DeeSerial_Addr2Mem(writer, out__shs_elem, struct Dee_simple_hashset_item);
	in__shs_elem = self->shswl_set.shs_elem;
	memcpyc(ou__shs_elem, in__shs_elem, self__shs_mask + 1, sizeof(struct Dee_simple_hashset_item));
	for (i = 0; i <= self__shs_mask; ++i)
		Dee_XIncref(ou__shs_elem[i].shsi_key);
	Dee_simple_hashset_with_lock_release(self);
	for (i = 0; i <= self__shs_mask; ++i) {
		Dee_seraddr_t addrof_elem = out__shs_elem + i * sizeof(struct Dee_simple_hashset_item);
		if (DeeSerial_InplacePutObject(writer, addrof_elem + offsetof(struct Dee_simple_hashset_item, shsi_key))) {
			ou__shs_elem = DeeSerial_Addr2Mem(writer, out__shs_elem, struct Dee_simple_hashset_item);
			for (++i; i <= self__shs_mask; ++i)
				Dee_XDecref(ou__shs_elem[i].shsi_key);
			goto err;
		}
	}
	out = DeeSerial_Addr2Mem(writer, addr, struct Dee_simple_hashset_with_lock);
	out->shswl_set.shs_mask = self__shs_mask;
	out->shswl_set.shs_size = self__shs_size;
	Dee_atomic_lock_init(&out->shswl_lock);
	return DeeSerial_PutAddr(writer, ADDROF(shswl_set.shs_elem), out__shs_elem);
err:
	return -1;
#undef ADDROF
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_SIMPLE_HASHSET_C */
