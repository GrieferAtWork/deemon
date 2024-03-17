/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_COLLECTIONS_BITSET_C
#define GUARD_DEX_COLLECTIONS_BITSET_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/system-features.h>
#include <deemon/util/atomic.h>

#include <hybrid/bitset.h>
#include <hybrid/overflow.h>

#include <stddef.h>

/* Use <deemon/util/atomic.h> to implement atomic bitset ops (so they're faster under CONFIG_NO_THREADS) */
#undef __hybrid_bitset_atomic_set
#undef __hybrid_bitset_atomic_clear
#undef __hybrid_bitset_atomic_flip
#undef __hybrid_bitset_atomic_fetchset
#undef __hybrid_bitset_atomic_fetchclear
#undef __hybrid_bitset_atomic_fetchflip
#undef bitset_atomic_set
#undef bitset_atomic_clear
#undef bitset_atomic_flip
#undef bitset_atomic_fetchset
#undef bitset_atomic_fetchclear
#undef bitset_atomic_fetchflip
#define __hybrid_bitset_atomic_set(self, bitno)        atomic_or(&(self)[_BITSET_WORD(bitno)], _BITSET_MASK(bitno))
#define __hybrid_bitset_atomic_clear(self, bitno)      atomic_and(&(self)[_BITSET_WORD(bitno)], ~_BITSET_MASK(bitno))
#define __hybrid_bitset_atomic_flip(self, bitno)       atomic_xor(&(self)[_BITSET_WORD(bitno)], _BITSET_MASK(bitno))
#define __hybrid_bitset_atomic_fetchset(self, bitno)   (atomic_fetchor(&(self)[_BITSET_WORD(bitno)], _BITSET_MASK(bitno)) & _BITSET_MASK(bitno))
#define __hybrid_bitset_atomic_fetchclear(self, bitno) (atomic_fetchand(&(self)[_BITSET_WORD(bitno)], ~_BITSET_MASK(bitno)) & _BITSET_MASK(bitno))
#define __hybrid_bitset_atomic_fetchflip(self, bitno)  (atomic_fetchxor(&(self)[_BITSET_WORD(bitno)], _BITSET_MASK(bitno)) & _BITSET_MASK(bitno))
#define bitset_atomic_set(self, bitno)                 __hybrid_bitset_atomic_set(self, bitno)
#define bitset_atomic_clear(self, bitno)               __hybrid_bitset_atomic_clear(self, bitno)
#define bitset_atomic_flip(self, bitno)                __hybrid_bitset_atomic_flip(self, bitno)
#define bitset_atomic_fetchset(self, bitno)            __hybrid_bitset_atomic_fetchset(self, bitno)
#define bitset_atomic_fetchclear(self, bitno)          __hybrid_bitset_atomic_fetchclear(self, bitno)
#define bitset_atomic_fetchflip(self, bitno)           __hybrid_bitset_atomic_fetchflip(self, bitno)

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

typedef struct {
	OBJECT_HEAD
	size_t                            bs_nbits;   /* [const] # of bits in this bitset */
	COMPILER_FLEXIBLE_ARRAY(bitset_t, bs_bitset); /* [0..bs_nbits] The actual bitset (writable).
	                                               * Unused bits within the last byte should be 0. */
} Bitset;

#define Bitset_Calloc(n_bits) \
	((DREF Bitset *)DeeObject_Calloc(offsetof(Bitset, bs_bitset) + BITSET_SIZEOF(n_bits)))
#define Bitset_Realloc(self, n_bits) \
	((DREF Bitset *)DeeObject_Realloc(self, offsetof(Bitset, bs_bitset) + BITSET_SIZEOF(n_bits)))
#define Bitset_TryRealloc(self, n_bits) \
	((DREF Bitset *)DeeObject_TryRealloc(self, offsetof(Bitset, bs_bitset) + BITSET_SIZEOF(n_bits)))
#define Bitset_Alloc(n_bits) \
	((DREF Bitset *)DeeObject_Malloc(offsetof(Bitset, bs_bitset) + BITSET_SIZEOF(n_bits)))

#define Bitset_Check(ob) /* Bitset is final, so exact check */ \
	DeeObject_InstanceOfExact(ob, &Bitset_Type)
#define RoBitset_Check(ob) /* RoBitset is final, so exact check */ \
	DeeObject_InstanceOfExact(ob, &RoBitset_Type)

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *bsi_owner;    /* [1..1][const] Some object that is providing the buffer whose bits are being enumerated. */
	DeeBuffer       bsi_buf;      /* [const] The buffer of raw bytes that are being viewed.
	                               * Unused bits in the first/last byte are undefined. */
	size_t          bsi_startbit; /* [const] Starting bit number part of the bitset (based at `bsi_buf.bb_base') */
	size_t          bsi_endbit;   /* [const] End bit number part of the bitset (based at `bsi_buf.bb_base') */
	unsigned int    bsi_bflags;   /* [const] Buffer flags (Set of `Dee_BUFFER_F*'; of relevance is `Dee_BUFFER_FWRITABLE') */
} BitsetView;

#define BitsetView_Check(ob) /* BitsetView is final, so exact check */ \
	DeeObject_InstanceOfExact(ob, &BitsetView_Type)
#define BitsetView_IsWritable(ob)  ((ob)->bsi_bflags & Dee_BUFFER_FWRITABLE)
#define BitsetView_GetBitset(ob)   ((bitset_t *)(ob)->bsi_buf.bb_base)
#define BitsetView_GetNBits(ob)    ((ob)->bsi_endbit - (ob)->bsi_startbit)
#define BitsetView_GetMinBitno(ob) ((ob)->bsi_startbit)
#define BitsetView_GetMaxBitno(ob) ((ob)->bsi_endbit - 1)
#define BitsetView_GetEndBitno(ob) ((ob)->bsi_endbit)

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *bsi_owner;  /* [1..1][const] Either the Bitset or BitsetView that is being iterated */
	bitset_t       *bsi_bitset; /* [1..1][const] Base address of the bitset begin iterated */
	size_t          bsi_nbits;  /* [const] # of bits at which iteration should be stopped. */
	size_t          bsi_bitno;  /* [<= bsi_nbits][lock(ATOMIC)] Next bitno to iterate. */
} BitsetIterator;

#define BitsetIterator_Check(ob) /* BitsetIterator is final, so exact check */ \
	DeeObject_InstanceOfExact(ob, &BitsetIterator_Type)

/* Return the bit number where iteration started (and
 * where it gets reset to when the iterator is rewound) */
#define BitsetIterator_GetStartBit(self)                \
	(BitsetView_Check((self)->bsi_owner)               \
	 ? ((BitsetView *)(self)->bsi_owner)->bsi_startbit \
	 : 0)

/* Return the bit number where iteration will end */
#define BitsetIterator_GetEndBit(self) \
	((self)->bsi_nbits)

PRIVATE ATTR_COLD int DCALL
bitset_err_bad_index(size_t bitno, size_t nbits) {
	return DeeError_Throwf(&DeeError_IndexError,
	                       "Index `%" PRFuSIZ "' lies outside the valid bounds "
	                       "[0,%" PRFuSIZ ") of sequence of type `Bitset'",
	                       bitno, nbits);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
bs_err_bad_index(Bitset *__restrict self, size_t bitno) {
	return bitset_err_bad_index(bitno, self->bs_nbits);
}

struct bitset_fromseq_data {
	DREF Bitset *bsfsd_bitset; /* [1..1] The bitset being created. */
	size_t       bsfsd_abits;  /* Allocated (and 0-initialized) # of bits (max used
	                            * # of bits is stored in `bsfsd_bitset->bs_nbits') */
};

PRIVATE WUNUSED NONNULL((2)) dssize_t DCALL
bitset_fromseq_cb(void *cookie, DeeObject *item) {
	struct bitset_fromseq_data *me = (struct bitset_fromseq_data *)cookie;
	DREF Bitset *bitset;
	size_t bitno;
	if (DeeObject_AsSize(item, &bitno))
		goto err;
	if (bitno >= me->bsfsd_abits) {
		DREF Bitset *new_bitset;
		size_t new_abits = me->bsfsd_abits;
		do {
			new_abits <<= 1;
		} while (bitno >= new_abits && new_abits);
		if (bitno >= new_abits) {
			new_abits = bitno + 1;
			if (bitno >= new_abits)
				goto err_overflow;
		}
		new_bitset = Bitset_TryRealloc(me->bsfsd_bitset, new_abits);
		if unlikely(!new_bitset) {
			new_abits = bitno + 1;
			new_bitset = Bitset_Realloc(me->bsfsd_bitset, new_abits);
			if unlikely(!new_bitset)
				goto err;
		}
		bitset_nclear(new_bitset->bs_bitset, new_bitset->bs_nbits, new_abits);
		me->bsfsd_bitset = new_bitset;
		me->bsfsd_abits  = new_abits;
	}
	bitset = me->bsfsd_bitset;
	if (bitset->bs_nbits <= bitno)
		bitset->bs_nbits = bitno + 1;
	bitset_set(bitset->bs_bitset, bitno);
	return 0;
err_overflow:
	return DeeError_Throwf(&DeeError_IntegerOverflow,
	                       "positive integer overflow in %k",
	                       item);
err:
	return -1;
}

/* Initialize from sequence of integers. */
PRIVATE WUNUSED NONNULL((1)) DREF Bitset *DCALL
bs_init_fromseq(DeeObject *seq, DeeObject *minbits_ob) {
	DREF Bitset *result;
	struct bitset_fromseq_data data;
	size_t minbits   = 0;
	data.bsfsd_abits = 128;
	if (minbits_ob) {
		if (DeeInt_AsSize(minbits_ob, &minbits))
			goto err;
		if (data.bsfsd_abits < minbits) {
			data.bsfsd_abits = minbits;
			data.bsfsd_abits += _BITSET_WORD_BMSK;
			data.bsfsd_abits &= ~_BITSET_WORD_BMSK;
		}
	}

	/* Fallback: allocate a full Bitset object and enumerate given "seq" */
	data.bsfsd_bitset = Bitset_Calloc(data.bsfsd_abits);
	if unlikely(!data.bsfsd_bitset)
		goto err;
	data.bsfsd_bitset->bs_nbits = minbits;
	if unlikely(DeeObject_Foreach(seq, &bitset_fromseq_cb, &data) < 0) {
		DeeObject_Free(data.bsfsd_bitset);
		goto err;
	}
	result = data.bsfsd_bitset;
	ASSERT(data.bsfsd_abits >= result->bs_nbits);
	if (data.bsfsd_abits > result->bs_nbits) {
		DREF Bitset *new_result;
		new_result = Bitset_TryRealloc(result, result->bs_nbits);
		if likely(new_result)
			result = new_result;
	}
	DeeObject_Init(result, &Bitset_Type);
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bs_nsi_getsize(Bitset *__restrict self) {
	return bitset_popcount(self->bs_bitset, self->bs_nbits);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_nsi_insert(Bitset *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	if unlikely(bitno >= self->bs_nbits)
		goto err_too_large;
	return bitset_atomic_fetchset(self->bs_bitset, bitno) ? 0 : 1;
err_too_large:
	return bs_err_bad_index(self, bitno);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_nsi_remove(Bitset *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	if unlikely(bitno >= self->bs_nbits)
		goto err_too_large;
	return bitset_atomic_fetchclear(self->bs_bitset, bitno) ? 1 : 0;
err_too_large:
	return bs_err_bad_index(self, bitno);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
bs_hash(Bitset *__restrict self) {
	size_t bitno;
	dhash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	bitset_foreach (bitno, self->bs_bitset, self->bs_nbits) {
		result = Dee_HashCombine(result, bitno);
	}
	return result;
}

struct bitset_ref {
	bitset_t *bsr_bitset;   /* [1..1] Referenced bitset. */
	size_t    bsr_startbit; /* Starting bit number part of the bitset (based at `bsr_bitset') */
	size_t    bsr_endbit;   /* End bit number part of the bitset (based at `bsr_bitset') */
};

#define bitset_ref_test(self, bitno) bitset_test((self)->bsr_bitset, (self)->bsr_startbit + (bitno))
#define bitset_ref_nbits(self)       ((self)->bsr_endbit - (self)->bsr_startbit)
#define bitset_ref_nanyset(self, startbitno, endbitno) \
	bitset_nanyset((self)->bsr_bitset, (self)->bsr_startbit + (startbitno), (self)->bsr_startbit + (endbitno))

PRIVATE NONNULL((1)) void DCALL
bitset_ref_fix(struct bitset_ref *__restrict self) {
	if (self->bsr_startbit > _BITSET_WORD_BMSK) {
		self->bsr_bitset += (self->bsr_startbit >> _BITSET_WORD_SHFT);
		self->bsr_endbit -= (self->bsr_startbit & ~_BITSET_WORD_BMSK);
		self->bsr_startbit &= _BITSET_WORD_BMSK;
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bitset_ref_assign(struct bitset_ref *__restrict dst,
                  struct bitset_ref *__restrict src) {
	size_t dst_bits = bitset_ref_nbits(dst);
	size_t src_bits = bitset_ref_nbits(src);
	if (src_bits > dst_bits) {
		/* Ensure that no bit is set that can't be represented in "dst" */
		size_t oob_index;
		oob_index = bitset_nffs(src->bsr_bitset,
		                        src->bsr_startbit + dst_bits,
		                        src->bsr_endbit);
		if (oob_index < src->bsr_endbit) {
			oob_index -= src->bsr_startbit;
			ASSERT(oob_index >= dst_bits);
			return bitset_err_bad_index(oob_index, dst_bits);
		}
		/* Trim to the size of our own bitset. */
		src->bsr_endbit = src->bsr_startbit + dst_bits;
		src_bits = dst_bits;
	}

	ASSERT(src_bits == bitset_ref_nbits(src));
	if (dst->bsr_startbit == src->bsr_startbit) {
		size_t minbitno = src->bsr_startbit;
		size_t maxbitno = src->bsr_endbit - 1;
		size_t minword = (size_t)_BITSET_WORD(minbitno);
		size_t maxword = (size_t)_BITSET_WORD(maxbitno);
		if (minword >= maxword) {
			bitset_t mask = ~((_BITSET_WORD_BMAX >> (_BITSET_WORD_BITS - (minbitno & _BITSET_WORD_BMSK))) |
			                  (_BITSET_WORD_BMAX << ((maxbitno & _BITSET_WORD_BMSK) + 1)));
			dst->bsr_bitset[maxword] = (dst->bsr_bitset[maxword] & ~mask) |
			                           (src->bsr_bitset[maxword] & mask);
		} else if (dst->bsr_bitset < src->bsr_bitset) {
			size_t n_words;
			bitset_t mask;
			mask = ~(_BITSET_WORD_BMAX >> (_BITSET_WORD_BITS - (minbitno & _BITSET_WORD_BMSK)));
			dst->bsr_bitset[minword] = (dst->bsr_bitset[minword] & ~mask) |
			                           (src->bsr_bitset[minword] & mask);
			n_words = maxword - (minword + 1);
			memmovedown(&dst->bsr_bitset[minword + 1],
			            &src->bsr_bitset[minword + 1],
			            n_words * sizeof(bitset_t));
			mask = ~(_BITSET_WORD_BMAX << ((maxbitno & _BITSET_WORD_BMSK) + 1));
			dst->bsr_bitset[maxword] = (dst->bsr_bitset[maxword] & ~mask) |
			                           (src->bsr_bitset[maxword] & mask);
		} else {
			size_t n_words;
			bitset_t mask;
			mask = ~(_BITSET_WORD_BMAX << ((maxbitno & _BITSET_WORD_BMSK) + 1));
			dst->bsr_bitset[maxword] = (dst->bsr_bitset[maxword] & ~mask) |
			                           (src->bsr_bitset[maxword] & mask);
			n_words = maxword - (minword + 1);
			memmoveup(&dst->bsr_bitset[minword + 1],
			          &src->bsr_bitset[minword + 1],
			          n_words * sizeof(bitset_t));
			mask = ~(_BITSET_WORD_BMAX >> (_BITSET_WORD_BITS - (minbitno & _BITSET_WORD_BMSK)));
			dst->bsr_bitset[minword] = (dst->bsr_bitset[minword] & ~mask) |
			                           (src->bsr_bitset[minword] & mask);
		}
	} else if (dst->bsr_bitset < src->bsr_bitset) {
		size_t bitno;
		for (bitno = 0; bitno < src_bits; ++bitno) {
			if (bitset_ref_test(src, bitno)) {
				bitset_set(dst->bsr_bitset, dst->bsr_startbit + bitno);
			} else {
				bitset_clear(dst->bsr_bitset, dst->bsr_startbit + bitno);
			}
		}
	} else {
		size_t bitno = src_bits;
		while (bitno) {
			--bitno;
			if (bitset_ref_test(src, bitno)) {
				bitset_set(dst->bsr_bitset, dst->bsr_startbit + bitno);
			} else {
				bitset_clear(dst->bsr_bitset, dst->bsr_startbit + bitno);
			}
		}
	}

	/* Clear out all bits that weren't copied from "src" */
	if (src_bits < dst_bits) {
		bitset_nclear(dst->bsr_bitset,
		              dst->bsr_startbit + src_bits,
		              dst->bsr_endbit);
	}
	return 0;
}



PRIVATE ATTR_IN(2) ATTR_OUT(1) void DCALL
bitset_ref_fromview(struct bitset_ref *__restrict self,
                    BitsetView const *__restrict view) {
	self->bsr_bitset   = BitsetView_GetBitset(view);
	self->bsr_startbit = view->bsi_startbit;
	self->bsr_endbit   = view->bsi_endbit;
	bitset_ref_fix(self);
}

PRIVATE WUNUSED ATTR_IN(1) ATTR_OUT(2) bool DCALL
DeeObject_AsBitset(DeeObject const *__restrict self,
                   struct bitset_ref *__restrict result) {
	if (Bitset_Check(self) || RoBitset_Check(self)) {
		Bitset *me = (Bitset *)self;
		result->bsr_bitset   = me->bs_bitset;
		result->bsr_startbit = 0;
		result->bsr_endbit   = me->bs_nbits;
		return true;
	} else if (BitsetView_Check(self)) {
		bitset_ref_fromview(result, (BitsetView *)self);
		return true;
	} else {
		return false;
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
copy_aligned_bits_to_0(bitset_t *__restrict dst,
                       bitset_t const *__restrict src,
                       size_t nbits) {
	memcpy(dst, src, BITSET_SIZEOF(nbits));
	if (nbits & _BITSET_WORD_BMSK) {
		/* Ensure that unused bits of the last byte are all zero */
		size_t maxword = _BITSET_WORD(nbits);
		bitset_t mask = __HYBRID_BITSET_LO_MASKIN(nbits & _BITSET_WORD_BMSK);
		dst[maxword] &= mask;
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF Bitset *DCALL
bs_init_fromseq_or_bitset(DeeObject *seq, DeeObject *minbits_ob) {
	/* Check for special case: is `seq' a bitset-like object? */
	struct bitset_ref ref;
	if (DeeObject_AsBitset(seq, &ref)) {
		DREF Bitset *result;
		size_t ref_nbits = bitset_ref_nbits(&ref);
		if (minbits_ob) {
			size_t minbits;
			if (DeeInt_AsSize(minbits_ob, &minbits))
				goto err;
			if (ref_nbits < minbits) {
				result = Bitset_Calloc(minbits);
				if unlikely(!result)
					goto err;
				if (ref.bsr_startbit == 0) {
					copy_aligned_bits_to_0(result->bs_bitset, ref.bsr_bitset, ref_nbits);
				} else {
					/* Slow case: bits don't align, so we must copy them one-at-a-time */
					size_t i;
					for (i = 0; i < ref_nbits; ++i) {
						if (bitset_ref_test(&ref, i))
							bitset_set(result->bs_bitset, i);
					}
				}
				result->bs_nbits = minbits;
				DeeObject_Init(result, &Bitset_Type);
				return result;
			}
		}
		if (ref.bsr_startbit == 0) {
			result = Bitset_Alloc(ref_nbits);
			if unlikely(!result)
				goto err;
			copy_aligned_bits_to_0(result->bs_bitset, ref.bsr_bitset, ref_nbits);
		} else {
			/* Slow case: bits don't align, so we must copy them one-at-a-time */
			size_t i;
			result = Bitset_Calloc(ref_nbits);
			if unlikely(!result)
				goto err;
			for (i = 0; i < ref_nbits; ++i) {
				if (bitset_ref_test(&ref, i))
					bitset_set(result->bs_bitset, i);
			}
		}
		result->bs_nbits = ref_nbits;
		DeeObject_Init(result, &Bitset_Type);
		return result;
	}
	return bs_init_fromseq(seq, minbits_ob);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
bs_cmp_eqne_bitset_ref(Bitset *__restrict self,
                       struct bitset_ref *__restrict other) {
	size_t self_nbits = self->bs_nbits;
	if (self_nbits != bitset_ref_nbits(other)) {
		/* Trim trailing 0-bits from "other" */
		size_t ot_nbits;
		ot_nbits = bitset_nfls(other->bsr_bitset,
		                       other->bsr_startbit,
		                       other->bsr_endbit);
		if (ot_nbits >= other->bsr_endbit) {
			/* "other" is empty -> check if "self" is empty, too */
			return !bitset_anyset(self->bs_bitset, self->bs_nbits);
		}
		other->bsr_endbit = ot_nbits + 1;
		if (self_nbits != bitset_ref_nbits(other)) {
			/* Trim trailing 0-bits from "self" */
			size_t lastset = bitset_fls(self->bs_bitset, self_nbits);
			if (lastset >= self_nbits)
				goto nope; /* "self" is empty */
			self_nbits = lastset + 1;
			if (self_nbits != bitset_ref_nbits(other))
				goto nope; /* "self" has a different greatest-set-bit */
		}
	}
	if (other->bsr_startbit == 0) {
		size_t num_bytes = self_nbits >> _BITSET_WORD_SHFT;
		if (memcmp(self->bs_bitset, other->bsr_bitset, num_bytes) != 0)
			goto nope;
		if (self_nbits & _BITSET_WORD_BMSK) {
			bitset_t mask = ((bitset_t)1 << (self_nbits & _BITSET_WORD_BMSK)) - 1;
			bitset_t my_last = self->bs_bitset[num_bytes] & mask;
			bitset_t ot_last = other->bsr_bitset[num_bytes] & mask;
			if (my_last != ot_last)
				goto nope;
		}
	} else {
		size_t bitno;
		for (bitno = 0; bitno < self_nbits; ++bitno) {
			if ((!!bitset_test(self->bs_bitset, bitno)) !=
			    (!!bitset_ref_test(other, bitno)))
				goto nope;
		}
	}
	return true;
nope:
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_eq(Bitset *self, DeeObject *other) {
	struct bitset_ref bs_other;
	if (!DeeObject_AsBitset(other, &bs_other))
		return (*DeeSeq_Type.tp_cmp->tp_eq)((DeeObject *)self, other);
	return_bool(bs_cmp_eqne_bitset_ref(self, &bs_other));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_ne(Bitset *self, DeeObject *other) {
	struct bitset_ref bs_other;
	if (!DeeObject_AsBitset(other, &bs_other))
		return (*DeeSeq_Type.tp_cmp->tp_ne)((DeeObject *)self, other);
	return_bool(!bs_cmp_eqne_bitset_ref(self, &bs_other));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_le(Bitset *self, DeeObject *other) {
	size_t bitno, nbits;
	struct bitset_ref bs_other;
	if (!DeeObject_AsBitset(other, &bs_other))
		return (*DeeSeq_Type.tp_cmp->tp_le)((DeeObject *)self, other);
	/* All bits from "self" must also be set in "other" */
	nbits = self->bs_nbits;
	if (nbits > bitset_ref_nbits(&bs_other)) {
		if (bitset_nanyset(self->bs_bitset, bitset_ref_nbits(&bs_other), nbits))
			goto nope;
		nbits = bitset_ref_nbits(&bs_other);
	}
	for (bitno = 0; bitno < nbits; ++bitno) {
		if (bitset_test(self->bs_bitset, bitno) &&
		    !bitset_ref_test(&bs_other, bitno))
			goto nope;
	}
	return_true;
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_ge(Bitset *self, DeeObject *other) {
	size_t bitno, nbits;
	struct bitset_ref bs_other;
	if (!DeeObject_AsBitset(other, &bs_other))
		return (*DeeSeq_Type.tp_cmp->tp_ge)((DeeObject *)self, other);
	/* All bits from "other" must also be set in "self" */
	nbits = bitset_ref_nbits(&bs_other);
	if (nbits > self->bs_nbits) {
		if (bitset_ref_nanyset(&bs_other, self->bs_nbits, nbits))
			goto nope;
		nbits = self->bs_nbits;
	}
	for (bitno = 0; bitno < nbits; ++bitno) {
		if (bitset_ref_test(&bs_other, bitno) &&
		    !bitset_test(self->bs_bitset, bitno))
			goto nope;
	}
	return_true;
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_gr(Bitset *self, DeeObject *other) {
	size_t bitno, nbits;
	struct bitset_ref bs_other;
	if (!DeeObject_AsBitset(other, &bs_other))
		return (*DeeSeq_Type.tp_cmp->tp_gr)((DeeObject *)self, other);
	/* not(All bits from "self" must also be set in "other") */
	nbits = self->bs_nbits;
	if (nbits > bitset_ref_nbits(&bs_other)) {
		if (bitset_nanyset(self->bs_bitset, bitset_ref_nbits(&bs_other), nbits))
			goto nope;
		nbits = bitset_ref_nbits(&bs_other);
	}
	for (bitno = 0; bitno < nbits; ++bitno) {
		if (bitset_test(self->bs_bitset, bitno) &&
		    !bitset_ref_test(&bs_other, bitno))
			goto nope;
	}
	return_false;
nope:
	return_true;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_lo(Bitset *self, DeeObject *other) {
	size_t bitno, nbits;
	struct bitset_ref bs_other;
	if (!DeeObject_AsBitset(other, &bs_other))
		return (*DeeSeq_Type.tp_cmp->tp_lo)((DeeObject *)self, other);
	/* not(All bits from "other" must also be set in "self") */
	nbits = bitset_ref_nbits(&bs_other);
	if (nbits > self->bs_nbits) {
		if (bitset_ref_nanyset(&bs_other, self->bs_nbits, nbits))
			goto nope;
		nbits = self->bs_nbits;
	}
	for (bitno = 0; bitno < nbits; ++bitno) {
		if (bitset_ref_test(&bs_other, bitno) &&
		    !bitset_test(self->bs_bitset, bitno))
			goto nope;
	}
	return_false;
nope:
	return_true;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetIterator *DCALL
bs_iter(Bitset *__restrict self) {
	DREF BitsetIterator *result;
	result = DeeObject_MALLOC(BitsetIterator);
	if unlikely(!result)
		goto err;
	result->bsi_owner = (DREF DeeObject *)self;
	Dee_Incref(self);
	result->bsi_bitset = self->bs_bitset;
	result->bsi_nbits  = self->bs_nbits;
	result->bsi_bitno  = 0;
	DeeObject_Init(result, &BitsetIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_size(Bitset *__restrict self) {
	return DeeInt_NewSize(bs_nsi_getsize(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_contains(Bitset *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	return_bool(bitno < self->bs_nbits &&
	            bitset_test(self->bs_bitset, bitno));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_delitem(Bitset *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	if unlikely(bitno >= self->bs_nbits)
		goto err_too_large;
	bitset_atomic_clear(self->bs_bitset, bitno);
	return 0;
err_too_large:
	return bs_err_bad_index(self, bitno);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_setitem(Bitset *self, DeeObject *key, DeeObject *value) {
	int temp;
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	if unlikely(bitno >= self->bs_nbits)
		goto err_too_large;
	temp = DeeObject_Bool(value);
	if unlikely(temp < 0)
		goto err;
	if (temp) {
		bitset_atomic_set(self->bs_bitset, bitno);
	} else {
		bitset_atomic_clear(self->bs_bitset, bitno);
	}
	return 0;
err_too_large:
	return bs_err_bad_index(self, bitno);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
bs_getrange_i(Bitset *self, dssize_t start, dssize_t end) {
	struct Dee_seq_range range;
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, self->bs_nbits);
	result->bsi_owner = (DREF DeeObject *)self;
	Dee_Incref(self);
	result->bsi_buf.bb_base = self->bs_bitset;
	result->bsi_buf.bb_size = BITSET_SIZEOF(self->bs_nbits);
#ifndef __INTELLISENSE__
	result->bsi_buf.bb_put = NULL;
#endif /* !__INTELLISENSE__ */
	result->bsi_startbit = range.sr_start;
	result->bsi_endbit   = range.sr_end;
	result->bsi_bflags   = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
bs_getrange(Bitset *self, DeeObject *start, DeeObject *end) {
	Dee_ssize_t start_i, end_i = self->bs_nbits;
	if (DeeObject_AsSSize(start, &start_i))
		goto err;
	if (!DeeNone_Check(end)) {
		if (DeeObject_AsSSize(end, &end_i))
			goto err;
	}
	return bs_getrange_i(self, start_i, end_i);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_delrange(Bitset *self, DeeObject *start, DeeObject *end) {
	struct Dee_seq_range range;
	Dee_ssize_t start_i, end_i = self->bs_nbits;
	if (DeeObject_AsSSize(start, &start_i))
		goto err;
	if (!DeeNone_Check(end)) {
		if (DeeObject_AsSSize(end, &end_i))
			goto err;
	}
	DeeSeqRange_Clamp(&range, start_i, end_i, self->bs_nbits);
	bitset_nclear(self->bs_bitset, range.sr_start, range.sr_end);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
bs_setrange(Bitset *self, DeeObject *start,
            DeeObject *end, DeeObject *value) {
	Dee_ssize_t start_i, end_i = self->bs_nbits;
	struct Dee_seq_range range;
	if (DeeObject_AsSSize(start, &start_i))
		goto err;
	if (!DeeNone_Check(end)) {
		if (DeeObject_AsSSize(end, &end_i))
			goto err;
	}
	DeeSeqRange_Clamp(&range, start_i, end_i, self->bs_nbits);
	if (DeeBool_Check(value)) {
		if (DeeBool_IsTrue(value)) {
			bitset_nset(self->bs_bitset, range.sr_start, range.sr_end);
		} else {
			bitset_nclear(self->bs_bitset, range.sr_start, range.sr_end);
		}
	} else if (DeeNone_Check(value)) {
		bitset_nclear(self->bs_bitset, range.sr_start, range.sr_end);
	} else {
		int result;
		DREF Bitset *value_bitset;
		struct bitset_ref dst, src;
		dst.bsr_bitset   = self->bs_bitset;
		dst.bsr_startbit = range.sr_start;
		dst.bsr_endbit   = range.sr_end;
		bitset_ref_fix(&dst);
		if (DeeObject_AsBitset(value, &src))
			return bitset_ref_assign(&dst, &src);
		value_bitset = bs_init_fromseq(value, NULL);
		if unlikely(!value_bitset)
			goto err;
		src.bsr_bitset   = value_bitset->bs_bitset;
		src.bsr_startbit = 0;
		src.bsr_endbit   = value_bitset->bs_nbits;
		result = bitset_ref_assign(&dst, &src);
		Dee_DecrefDokill(value_bitset);
		return result;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_getbuf(Bitset *__restrict self,
          DeeBuffer *__restrict info,
          unsigned int flags) {
	(void)flags;
	info->bb_base = self->bs_bitset;
	info->bb_size = BITSET_SIZEOF(self->bs_nbits);
	return 0;
}

PRIVATE DEFINE_KWLIST(kwlist_bitno, { K(bitno), KEND });

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_insert(Bitset *__restrict self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	size_t bitno;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_bitno, UNPuSIZ ":insert", &bitno))
		goto err;
	if unlikely(bitno >= self->bs_nbits)
		goto err_too_large;
	return_bool(!bitset_atomic_fetchset(self->bs_bitset, bitno));
err_too_large:
	bs_err_bad_index(self, bitno);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_remove(Bitset *__restrict self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	size_t bitno;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_bitno, UNPuSIZ ":remove", &bitno))
		goto err;
	if unlikely(bitno >= self->bs_nbits)
		goto err_too_large;
	return_bool(bitset_atomic_fetchclear(self->bs_bitset, bitno));
err_too_large:
	bs_err_bad_index(self, bitno);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_bytes(Bitset *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":bytes"))
		goto err;
	return DeeBytes_NewView((DeeObject *)self,
	                        self->bs_bitset,
	                        BITSET_SIZEOF(self->bs_nbits),
	                        Dee_BUFFER_FWRITABLE);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_sizeof(Bitset *__restrict self) {
	size_t result = offsetof(Bitset, bs_bitset) + BITSET_SIZEOF(self->bs_nbits);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED DREF Bitset *DCALL bs_ctor(void) {
	DREF Bitset *result = Bitset_Alloc(0);
	if unlikely(!result)
		goto err;
	result->bs_nbits = 0;
	DeeObject_Init(result, &Bitset_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bitset *DCALL
bs_copy(Bitset *__restrict self) {
	DREF Bitset *result = Bitset_Alloc(self->bs_nbits);
	if unlikely(!result)
		goto err;
	result->bs_nbits = self->bs_nbits;
	memcpy(result->bs_bitset, self->bs_bitset,
	       BITSET_SIZEOF(self->bs_nbits));
	DeeObject_Init(result, &Bitset_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bitset *DCALL
bs_frozen(Bitset *__restrict self) {
	DREF Bitset *result = Bitset_Alloc(self->bs_nbits);
	if unlikely(!result)
		goto err;
	result->bs_nbits = self->bs_nbits;
	memcpy(result->bs_bitset, self->bs_bitset,
	       BITSET_SIZEOF(self->bs_nbits));
	DeeObject_Init(result, &RoBitset_Type);
	return result;
err:
	return NULL;
}


/* (nbits:?Dint,init=!f)
 * (seq:?S?Dint,minbits=!0) */
PRIVATE WUNUSED DREF Bitset *DCALL
bs_init(size_t argc, DeeObject *const *argv) {
	size_t nbits;
	DREF Bitset *result;
	DeeObject *seq_or_nbits;
	DeeObject *init_or_minbits = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:Bitset", &seq_or_nbits, &init_or_minbits))
		goto err;
	if (!DeeInt_Check(seq_or_nbits))
		return bs_init_fromseq_or_bitset(seq_or_nbits, init_or_minbits);
	/* Initialize fixed-length bitset. */
	if (DeeInt_AsSize(seq_or_nbits, &nbits))
		goto err;
	if (init_or_minbits) {
		int temp = DeeObject_Bool(init_or_minbits);
		if unlikely(temp < 0)
			goto err;
		if (!temp)
			goto init_fixed_length_as_empty;
		result = Bitset_Alloc(nbits);
		if unlikely(!result)
			goto err;
		bitset_setall(result->bs_bitset, nbits);
	} else {
init_fixed_length_as_empty:
		result = Bitset_Calloc(nbits);
		if unlikely(!result)
			goto err;
	}
	result->bs_nbits = nbits;
	DeeObject_Init(result, &Bitset_Type);
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_assign_bitset(Bitset *self, struct bitset_ref *__restrict other) {
	size_t ot_bits = bitset_ref_nbits(other);
	if (ot_bits > self->bs_nbits) {
		/* Ensure that no bit is set that can't be represented in "self" */
		size_t oob_index;
		oob_index = bitset_nffs(other->bsr_bitset,
		                        other->bsr_startbit + self->bs_nbits,
		                        other->bsr_endbit);
		if (oob_index < other->bsr_endbit) {
			oob_index -= other->bsr_startbit;
			ASSERT(oob_index >= self->bs_nbits);
			return bs_err_bad_index(self, oob_index);
		}
		/* Trim to the size of our own bitset. */
		other->bsr_endbit = other->bsr_startbit + self->bs_nbits;
		ot_bits = self->bs_nbits;
	}

	ASSERT(ot_bits == bitset_ref_nbits(other));
	if (other->bsr_startbit == 0) {
		/* Optimization for when direct word-copy is possible.
		 * NOTE: Still use "memmove" in case "other" is a view of "self" */
		memmove(self->bs_bitset, other->bsr_bitset, BITSET_SIZEOF(ot_bits));
	} else {
		/* Slow case: bits don't align, so we must copy them one-at-a-time */
		size_t i;
		for (i = 0; i < ot_bits; ++i) {
			if (bitset_test(other->bsr_bitset, other->bsr_startbit + i)) {
				bitset_set(self->bs_bitset, i);
			} else {
				bitset_clear(self->bs_bitset, i);
			}
		}
	}

	/* Clear out all bits that weren't copied from "other" */
	if (ot_bits < self->bs_nbits)
		bitset_nclear(self->bs_bitset, ot_bits, self->bs_nbits);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_assign(Bitset *self, DeeObject *other) {
	int result;
	DREF Bitset *temp;
	struct bitset_ref bs_other;
	if (DeeObject_AsBitset(other, &bs_other))
		return bs_assign_bitset(self, &bs_other);

	/* Must create a temp bitset from "other" and then assign that one.
	 * We can't directly assign from "other" in case "other" somehow
	 * re-uses the state of "self" (e.g.: is a yield function) */
	temp = bs_init_fromseq(other, NULL);
	if unlikely(!temp)
		goto err;
	bs_other.bsr_bitset   = temp->bs_bitset;
	bs_other.bsr_startbit = 0;
	bs_other.bsr_endbit   = temp->bs_nbits;
	result = bs_assign_bitset(self, &bs_other);
	Dee_DecrefDokill(temp);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_bool(Bitset *__restrict self) {
	return bitset_anyset(self->bs_bitset, self->bs_nbits) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) dssize_t DCALL
bs_printrepr(Bitset *__restrict self, dformatprinter printer, void *arg) {
	bool is_first;
	size_t bitno;
	dssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "Bitset({");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	bitset_foreach (bitno, self->bs_bitset, self->bs_nbits) {
		DO(err, DeeFormat_Printf(printer, arg,
		                         "%s%" PRFuSIZ,
		                         is_first ? " " : ", ",
		                         bitno));
		is_first = false;
	}
	DO(err, is_first ? DeeFormat_PRINT(printer, arg, "})")
	                 : DeeFormat_PRINT(printer, arg, " })"));
done:
	return result;
err:
	return temp;
}


PRIVATE struct type_nsi tpconst bs_nsi = {
	/* .nsi_class = */ TYPE_SEQX_CLASS_SET,
	/* .nsi_flags = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_setlike = */ {
			/* .nsi_getsize = */ (dfunptr_t)&bs_nsi_getsize,
			/* .nsi_insert  = */ (dfunptr_t)&bs_nsi_insert,
			/* .nsi_remove  = */ (dfunptr_t)&bs_nsi_remove,
		}
	}
};

/* Compare operators with optimizations when the operand is another `Bitset' or `BitsetView' */
PRIVATE struct type_cmp bs_cmp = {
	/* .tp_hash = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&bs_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_ge,
};

PRIVATE struct type_seq bs_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bs_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bs_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_contains,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&bs_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bs_setitem,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bs_getrange,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bs_delrange,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&bs_setrange,
	/* .tp_nsi       = */ &bs_nsi
};

PRIVATE struct type_buffer bs_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&bs_getbuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE struct type_method tpconst bs_methods[] = {
	/* TODO: flip()  (flip all) */
	/* TODO: flip(bitno:?Dint) */
	/* TODO: flip(start:?Dint,end:?Dint) */

	/* TODO: set()  (set all) */
	/* TODO: set(bitno:?Dint) */
	/* TODO: set(start:?Dint,end:?Dint) */

	/* TODO: clear()  (clear all) */
	/* TODO: clear(bitno:?Dint) */
	/* TODO: clear(start:?Dint,end:?Dint) */

	/* TODO: any(start=0,end=-1) */
	/* TODO: all(start=0,end=-1) */
	/* TODO: ffs() ffc() fls() flc() */
	TYPE_KWMETHOD_F("insert", &bs_insert, METHOD_FNOREFESCAPE,
	                "(bitno:?Dint)->?Dbool\n"
	                "#tIntegerOverflow{@bitno is negative or too large}"
	                "#tValueError{@bitno is greater than or equal to ?#nbits}"
	                "Turn on @bitno, returning !t if it was turned on, or !f if it was already on"),
	TYPE_KWMETHOD_F("remove", &bs_remove, METHOD_FNOREFESCAPE,
	                "(bitno:?Dint)->?Dbool\n"
	                "#tIntegerOverflow{@bitno is negative or too large}"
	                "#tValueError{@bitno is greater than or equal to ?#nbits}"
	                "Turn off @bitno, returning !t if it was turned off, or !f if it was already off"),
	TYPE_METHOD("bytes", &bs_bytes,
	            "->?DBytes\n"
	            "Returns a view for the underlying bytes of ?."),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst bs_getsets[] = {
	TYPE_GETTER_F("frozen", &bs_frozen, METHOD_FNOREFESCAPE, "->?#Frozen"),
	TYPE_GETTER_F("__sizeof__", &bs_sizeof, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst bs_members[] = {
	TYPE_MEMBER_FIELD_DOC("nbits", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Bitset, bs_nbits),
	                      "The # of bits stored in this bitset. Attempting to alter the state of "
	                      /**/ "a bit greater than or equal to this value result in an :IndexError"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bs_class_members[] = {
	TYPE_MEMBER_CONST("Frozen", &RoBitset_Type),
	TYPE_MEMBER_CONST("View", &BitsetView_Type),
#define bsv_class_members (bs_class_members + 2)
	TYPE_MEMBER_CONST("Iterator", &BitsetIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_operator const bs_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FNOTHROW),
};

INTERN DeeTypeObject Bitset_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Bitset",
	/* .tp_doc      = */ DOC("Implements a bitset, that is a tightly packed array of bits, where "
	                         /**/ "each bit can either be on or off. The # of bits stored cannot be "
	                         /**/ "changed, but their state can using ?DSet operators, or by assigning "
	                         /**/ "!t or !f to the bit's index. Additionally, the state of a bit can "
	                         /**/ "be queried as either ${bitno in this} or ${this[bitno]}\n"
	                         "\n"

	                         "()\n"
	                         "Construct an empty ?.\n"
	                         "\n"

	                         "(nbits:?Dint,init=!f)\n"
	                         "Construct a new Bitset that consists of @nbits bits all pre-initialized to @init\n"
	                         "\n"

	                         "(seq:?S?Dint,minbits=!0)\n"
	                         "#tIntegerOverflow{One of the elements of @seq is negative, or greater than ?ASIZE_MAX?Dint}"
	                         "Construct a Bitset, turning on all the bits from @seq. The resulting "
	                         /**/ "?.'s ?#nbits is ${{ (seq.each + 1)..., minbits } > ...}\n"
	                         "\n"

	                         "copy->\n"
	                         "Returns a copy @this ?.\n"
	                         "\n"

	                         "deepcopy->\n"
	                         "Same as ?#op:copy\n"
	                         "\n"

	                         ":=(other:?S?Dint)->\n"
	                         "#tIntegerOverflow{One of the elements of @other is negative or too large}"
	                         "#tIndexError{One of the elements of @other is greater that or equal to ?#nbits}"
	                         "Turn off all bits, except for those whose indices appear in @other\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of 1-bits (that is: the population count) of this ?.\n"
	                         "\n"

	                         "move:=->\n"
	                         "Same as ?#op:assign\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if at least one bit has been turned on\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the bit indices that are turned on\n"
	                         "\n"

	                         "repr->\n"
	                         "Print the representation of @this in the form of ${Bitset({ 0, 1, 2, ... })} "
	                         /**/ "(listing all bit indices where the associated bit is on)\n"
	                         "\n"

	                         "[]->?Dbool\n"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Returns !t or !f indicative of he the state of the @index'th bit\n"
	                         "\n"

	                         "[]=(index:?Dint,value:?Dbool)\n"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Set the state of the @index'th bit to @value\n"
	                         "\n"

	                         "del[]->\n"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Same as ${this[index] = false}\n"
	                         "\n"

	                         "contains(index:?Dint)->?Dbool\n"
	                         "Alias for ${this[index]}\n"
	                         "\n"

	                         "[:](start:?X2?N?Dint,end:?X2?N?Dint)->?#View\n"
	                         "Returns a proxy-view for reading/writing the bit-range #C{[start,end)}\n"
	                         "\n"

	                         "del[:]->\n"
	                         "Set all bits within the range #C{[start,end)} to !f (alias for ${this[start, end] = false})\n"
	                         "\n"

	                         "[:]=(start:?Dint,end:?Dint,value:?Dbool)\n"
	                         "Turn on/off all bits within the range #C{[start,end)}, as per @value\n"
	                         "\n"

	                         "[:]=(start:?Dint,end:?Dint,values:?S?Dint)\n"
	                         "Assign @values to the range #C{[start,end)}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bs_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bs_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&bs_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&bs_init,
			}
		},
		/* .tp_dtor        = */ NULL, /* No destructor needed! */
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&bs_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&bs_assign,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&bs_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&bs_printrepr,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: "|=" "&=" "^=" */
	/* .tp_cmp           = */ &bs_cmp,
	/* .tp_seq           = */ &bs_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ &bs_buffer,
	/* .tp_methods       = */ bs_methods,
	/* .tp_getsets       = */ bs_getsets,
	/* .tp_members       = */ bs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bs_class_members,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ bs_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(bs_operators)
};








#define robs_nsi_getsize bs_nsi_getsize
#define robs_hash        bs_hash

PRIVATE WUNUSED NONNULL((1)) DREF Bitset *DCALL
robs_init_fromseq(DeeObject *seq) {
	DREF Bitset *result;
	struct bitset_fromseq_data data;

	/* Fallback: allocate a full Bitset object and enumerate given "seq" */
	data.bsfsd_abits  = 128;
	data.bsfsd_bitset = Bitset_Calloc(data.bsfsd_abits);
	if unlikely(!data.bsfsd_bitset)
		goto err;
	data.bsfsd_bitset->bs_nbits = 0;
	if unlikely(DeeObject_Foreach(seq, &bitset_fromseq_cb, &data) < 0) {
		DeeObject_Free(data.bsfsd_bitset);
		goto err;
	}
	result = data.bsfsd_bitset;
	ASSERT(data.bsfsd_abits >= result->bs_nbits);
	if (data.bsfsd_abits > result->bs_nbits) {
		DREF Bitset *new_result;
		new_result = Bitset_TryRealloc(result, result->bs_nbits);
		if likely(new_result)
			result = new_result;
	}
	DeeObject_Init(result, &RoBitset_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Bitset *DCALL
robs_init_fromseq_or_bitset(DeeObject *seq) {
	/* Check for special case: is `seq' a bitset-like object? */
	struct bitset_ref ref;
	if (DeeObject_AsBitset(seq, &ref)) {
		DREF Bitset *result;
		size_t ref_nbits = bitset_ref_nbits(&ref);
		if (ref.bsr_startbit == 0) {
			result = Bitset_Alloc(ref_nbits);
			if unlikely(!result)
				goto err;
			copy_aligned_bits_to_0(result->bs_bitset, ref.bsr_bitset, ref_nbits);
		} else {
			/* Slow case: bits don't align, so we must copy them one-at-a-time */
			size_t i;
			result = Bitset_Calloc(ref_nbits);
			if unlikely(!result)
				goto err;
			for (i = 0; i < ref_nbits; ++i) {
				if (bitset_ref_test(&ref, i))
					bitset_set(result->bs_bitset, i);
			}
		}
		result->bs_nbits = ref_nbits;
		DeeObject_Init(result, &RoBitset_Type);
		return result;
	}
	return robs_init_fromseq(seq);
err:
	return NULL;
}

#define robs_eq       bs_eq
#define robs_ne       bs_ne
#define robs_le       bs_le
#define robs_ge       bs_ge
#define robs_gr       bs_gr
#define robs_lo       bs_lo
#define robs_iter     bs_iter
#define robs_size     bs_size
#define robs_contains bs_contains

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
robs_getrange_i(Bitset *self, dssize_t start, dssize_t end) {
	struct Dee_seq_range range;
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, self->bs_nbits);
	result->bsi_owner = (DREF DeeObject *)self;
	Dee_Incref(self);
	result->bsi_buf.bb_base = self->bs_bitset;
	result->bsi_buf.bb_size = BITSET_SIZEOF(self->bs_nbits);
#ifndef __INTELLISENSE__
	result->bsi_buf.bb_put = NULL;
#endif /* !__INTELLISENSE__ */
	result->bsi_startbit = range.sr_start;
	result->bsi_endbit   = range.sr_end;
	result->bsi_bflags   = Dee_BUFFER_FREADONLY;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
robs_getrange(Bitset *self, DeeObject *start, DeeObject *end) {
	Dee_ssize_t start_i, end_i = self->bs_nbits;
	if (DeeObject_AsSSize(start, &start_i))
		goto err;
	if (!DeeNone_Check(end)) {
		if (DeeObject_AsSSize(end, &end_i))
			goto err;
	}
	return robs_getrange_i(self, start_i, end_i);
err:
	return NULL;
}

#define robs_getbuf bs_getbuf

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
robs_bytes(Bitset *__restrict self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":bytes"))
		goto err;
	return DeeBytes_NewView((DeeObject *)self,
	                        self->bs_bitset,
	                        BITSET_SIZEOF(self->bs_nbits),
	                        Dee_BUFFER_FREADONLY);
err:
	return NULL;
}

#define robs_sizeof bs_sizeof
PRIVATE WUNUSED DREF Bitset *DCALL robs_ctor(void) {
	DREF Bitset *result = Bitset_Alloc(0);
	if unlikely(!result)
		goto err;
	result->bs_nbits = 0;
	DeeObject_Init(result, &RoBitset_Type);
	return result;
err:
	return NULL;
}

/* (seq:?S?Dint) */
PRIVATE WUNUSED DREF Bitset *DCALL
robs_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:Bitset.Frozen", &seq))
		goto err;
	return robs_init_fromseq_or_bitset(seq);
err:
	return NULL;
}

#define robs_bool bs_bool

PRIVATE WUNUSED NONNULL((1)) dssize_t DCALL
robs_printrepr(Bitset *__restrict self, dformatprinter printer, void *arg) {
	bool is_first;
	size_t bitno;
	dssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "Bitset.Frozen({");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	bitset_foreach (bitno, self->bs_bitset, self->bs_nbits) {
		DO(err, DeeFormat_Printf(printer, arg,
		                         "%s%" PRFuSIZ,
		                         is_first ? " " : ", ",
		                         bitno));
		is_first = false;
	}
	DO(err, is_first ? DeeFormat_PRINT(printer, arg, "})")
	                 : DeeFormat_PRINT(printer, arg, " })"));
done:
	return result;
err:
	return temp;
}


PRIVATE struct type_nsi tpconst robs_nsi = {
	/* .nsi_class = */ TYPE_SEQX_CLASS_SET,
	/* .nsi_flags = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_setlike = */ {
			/* .nsi_getsize = */ (dfunptr_t)&robs_nsi_getsize,
			/* .nsi_insert  = */ NULL,
			/* .nsi_remove  = */ NULL,
		}
	}
};

/* Compare operators with optimizations when the operand is another `Bitset' or `BitsetView' */
PRIVATE struct type_cmp robs_cmp = {
	/* .tp_hash = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&robs_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_ge,
};

PRIVATE struct type_seq robs_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&robs_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&robs_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_contains,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&robs_getrange,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &robs_nsi
};

PRIVATE struct type_buffer robs_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&robs_getbuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE struct type_method tpconst robs_methods[] = {
	/* TODO: any(start=0,end=-1) */
	/* TODO: all(start=0,end=-1) */
	/* TODO: ffs() ffc() fls() flc() */
	TYPE_METHOD("bytes", &robs_bytes,
	            "->?DBytes\n"
	            "Returns a view for the underlying bytes of ?."),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst robs_getsets[] = {
	TYPE_GETTER_F("frozen", &DeeObject_NewRef, METHOD_FCONSTCALL, "->?Dint"),
	TYPE_GETTER_F("__sizeof__", &robs_sizeof, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

#define robs_members       bs_members
#define robs_class_members bs_class_members

PRIVATE struct type_operator const robs_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCOMPARE | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCOMPARE | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCOMPARE | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCOMPARE | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCOMPARE | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCOMPARE | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITERSELF, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0035_GETRANGE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_8003_GETBUF, METHOD_FCONSTCALL),
};

INTERN DeeTypeObject RoBitset_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RoBitset",
	/* .tp_doc      = */ DOC("Frozen (read-only) variant of ?GBitset\n"
	                         "\n"

	                         "()\n"
	                         "Construct an empty ?.\n"
	                         "\n"

	                         "(seq:?S?Dint)\n"
	                         "#tIntegerOverflow{One of the elements of @seq is negative, or greater than ?ASIZE_MAX?Dint}"
	                         "Construct a Bitset, turning on all the bits from @seq. The resulting "
	                         /**/ "?.'s ?#nbits is ${(seq.each + 1) > ...}\n"
	                         "\n"

	                         "copy->\n"
	                         "Always re-returns @this\n"
	                         "\n"

	                         "deepcopy->\n"
	                         "Always re-returns @this\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of 1-bits (that is: the population count) of this ?.\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if at least one bit has been turned on\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the bit indices that are turned on\n"
	                         "\n"

	                         "repr->\n"
	                         "Print the representation of @this in the form of ${Bitset({ 0, 1, 2, ... })} "
	                         /**/ "(listing all bit indices where the associated bit is on)\n"
	                         "\n"

	                         "[]->?Dbool\n"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Returns !t or !f indicative of he the state of the @index'th bit\n"
	                         "\n"

	                         "contains(index:?Dint)->?Dbool\n"
	                         "Alias for ${this[index]}\n"
	                         "\n"

	                         "[:](start:?X2?N?Dint,end:?X2?N?Dint)->?#View\n"
	                         "Returns a proxy-view for reading the bit-range #C{[start,end)}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&robs_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)&robs_init,
			}
		},
		/* .tp_dtor        = */ NULL, /* No destructor needed! */
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&robs_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&robs_printrepr,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &robs_cmp,
	/* .tp_seq           = */ &robs_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ &robs_buffer,
	/* .tp_methods       = */ robs_methods,
	/* .tp_getsets       = */ robs_getsets,
	/* .tp_members       = */ robs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ robs_class_members,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ robs_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(robs_operators)
};






PRIVATE ATTR_COLD NONNULL((1)) int DCALL
bsv_err_bad_index(BitsetView *__restrict self, size_t bitno) {
	return bitset_err_bad_index(bitno, BitsetView_GetNBits(self));
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
bsv_err_readonly(BitsetView *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_BufferError,
	                       "The BitsetView object is not writable");
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bsv_nsi_getsize(BitsetView *__restrict self) {
	return BitsetView_GetNBits(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_nsi_insert(BitsetView *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	if unlikely(bitno >= BitsetView_GetNBits(self))
		goto err_too_large;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	return bitset_atomic_fetchset(BitsetView_GetBitset(self),
	                              self->bsi_startbit + bitno)
	       ? 0
	       : 1;
err_readonly:
	return bsv_err_readonly(self);
err_too_large:
	return bsv_err_bad_index(self, bitno);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_nsi_remove(BitsetView *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	if unlikely(bitno >= BitsetView_GetNBits(self))
		goto err_too_large;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	return bitset_atomic_fetchclear(BitsetView_GetBitset(self),
	                                self->bsi_startbit + bitno)
	       ? 1
	       : 0;
err_readonly:
	return bsv_err_readonly(self);
err_too_large:
	return bsv_err_bad_index(self, bitno);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
bsv_hash(BitsetView *__restrict self) {
	size_t bitno;
	dhash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	for (bitno = BitsetView_GetMinBitno(self);
	     bitno < BitsetView_GetEndBitno(self); ++bitno) {
		if (bitset_test(BitsetView_GetBitset(self), bitno)) {
			size_t index = bitno - BitsetView_GetMinBitno(self);
			result = Dee_HashCombine(result, index);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
bitset_ref_cmp_eq(struct bitset_ref *__restrict a,
                  struct bitset_ref *__restrict b) {
	size_t a_nbits = bitset_ref_nbits(a);
	size_t b_nbits = bitset_ref_nbits(b);
	ASSERT(a->bsr_startbit <= _BITSET_WORD_BMSK);
	ASSERT(b->bsr_startbit <= _BITSET_WORD_BMSK);
	if (a_nbits != b_nbits) {
		/* Trim trailing 0-bits from "b" */
		b_nbits = bitset_nfls(b->bsr_bitset, b->bsr_startbit, b->bsr_endbit);
		if (b_nbits >= b->bsr_endbit) {
			/* "b" is empty -> check if "a" is empty, too */
			return !bitset_nanyset(a->bsr_bitset, a->bsr_startbit, a->bsr_endbit);
		}
		b->bsr_endbit = b_nbits + 1;
		b_nbits = bitset_ref_nbits(b);
		if (a_nbits != b_nbits) {
			/* Trim trailing 0-bits from "a" */
			size_t lastset = bitset_nfls(a->bsr_bitset, a->bsr_startbit, a->bsr_endbit);
			if (lastset >= a->bsr_endbit)
				goto nope; /* "a" is empty */
			a->bsr_endbit = lastset + 1;
			a_nbits = bitset_ref_nbits(a);
			if (a_nbits != b_nbits)
				goto nope; /* "a" has a different greatest-set-bit */
		}
		goto nope;
	}
	if (a->bsr_startbit == b->bsr_startbit) {
		size_t minbitno = a->bsr_startbit;
		size_t maxbitno = a->bsr_endbit - 1;
		size_t minword = (size_t)_BITSET_WORD(minbitno);
		size_t maxword = (size_t)_BITSET_WORD(maxbitno);
		if (minword >= maxword) {
			bitset_t mask = ~((_BITSET_WORD_BMAX >> (_BITSET_WORD_BITS - (minbitno & _BITSET_WORD_BMSK))) |
			                  (_BITSET_WORD_BMAX << ((maxbitno & _BITSET_WORD_BMSK) + 1)));
			if ((a->bsr_bitset[maxword] & mask) != (b->bsr_bitset[maxword] & mask))
				goto nope;
		} else {
			size_t n_words;
			bitset_t mask;
			mask = ~(_BITSET_WORD_BMAX >> (_BITSET_WORD_BITS - (minbitno & _BITSET_WORD_BMSK)));
			if ((a->bsr_bitset[minword] & mask) != (b->bsr_bitset[minword] & mask))
				goto nope;
			n_words = maxword - (minword + 1);
			if (memcmp(&a->bsr_bitset[minword + 1],
			           &b->bsr_bitset[minword + 1],
			           n_words * sizeof(bitset_t)) != 0)
				goto nope;
			mask = ~(_BITSET_WORD_BMAX << ((maxbitno & _BITSET_WORD_BMSK) + 1));
			if ((a->bsr_bitset[maxword] & mask) != (b->bsr_bitset[maxword] & mask))
				goto nope;
		}
	} else {
		size_t bitno;
		for (bitno = 0; bitno < a_nbits; ++bitno) {
			if ((!!bitset_ref_test(a, bitno)) !=
			    (!!bitset_ref_test(b, bitno)))
				goto nope;
		}
	}
	return true;
nope:
	return false;
}

#define bitset_ref_cmp_ne(a, b) (!bitset_ref_cmp_eq(a, b))
#define bitset_ref_cmp_lo(a, b) (!bitset_ref_cmp_ge(a, b))
#define bitset_ref_cmp_gr(a, b) (!bitset_ref_cmp_le(a, b))
#define bitset_ref_cmp_ge(a, b) bitset_ref_cmp_le(b, a)
PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
bitset_ref_cmp_le(struct bitset_ref const *__restrict a,
                  struct bitset_ref const *__restrict b) {
	size_t bitno;
	size_t a_nbits = bitset_ref_nbits(a);
	size_t b_nbits = bitset_ref_nbits(b);

	/* All bits from "a" must also be set in "b" */
	if (a_nbits > b_nbits) {
		if (bitset_nanyset(a->bsr_bitset,
		                   a->bsr_startbit + b_nbits,
		                   a->bsr_endbit))
			goto nope;
		a_nbits = b_nbits;
	}
	for (bitno = 0; bitno < a_nbits; ++bitno) {
		if (bitset_ref_test(a, bitno) && !bitset_ref_test(b, bitno))
			goto nope;
	}
	return true;
nope:
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_eq(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_eq)((DeeObject *)self, other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_eq(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_ne(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_eq)((DeeObject *)self, other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_ne(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_le(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_le)((DeeObject *)self, other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_le(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_ge(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_ge)((DeeObject *)self, other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_ge(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_gr(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_gr)((DeeObject *)self, other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_gr(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_lo(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_lo)((DeeObject *)self, other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_lo(&a, &b));
}


PRIVATE WUNUSED NONNULL((1)) DREF BitsetIterator *DCALL
bsv_iter(BitsetView *__restrict self) {
	DREF BitsetIterator *result;
	result = DeeObject_MALLOC(BitsetIterator);
	if unlikely(!result)
		goto err;
	result->bsi_owner = (DREF DeeObject *)self;
	Dee_Incref(self);
	result->bsi_bitset = BitsetView_GetBitset(self);
	result->bsi_nbits  = self->bsi_endbit;
	result->bsi_bitno  = self->bsi_startbit;
	DeeObject_Init(result, &BitsetIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_size(BitsetView *__restrict self) {
	return DeeInt_NewSize(bsv_nsi_getsize(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_contains(BitsetView *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	return_bool(bitno < BitsetView_GetNBits(self) &&
	            bitset_test(BitsetView_GetBitset(self),
	                        bitno + self->bsi_startbit));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_delitem(BitsetView *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	if unlikely(bitno >= BitsetView_GetNBits(self))
		goto err_too_large;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	bitset_atomic_clear(BitsetView_GetBitset(self), bitno);
	return 0;
err_readonly:
	return bsv_err_readonly(self);
err_too_large:
	return bsv_err_bad_index(self, bitno);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_setitem(BitsetView *self, DeeObject *key, DeeObject *value) {
	int temp;
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	if unlikely(bitno >= BitsetView_GetNBits(self))
		goto err_too_large;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	temp = DeeObject_Bool(value);
	if unlikely(temp < 0)
		goto err;
	if (temp) {
		bitset_atomic_set(BitsetView_GetBitset(self), bitno);
	} else {
		bitset_atomic_clear(BitsetView_GetBitset(self), bitno);
	}
	return 0;
err_readonly:
	return bsv_err_readonly(self);
err_too_large:
	return bsv_err_bad_index(self, bitno);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
bsv_getrange_i(BitsetView *self, dssize_t start, dssize_t end) {
	struct Dee_seq_range range;
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, BitsetView_GetNBits(self));
	result->bsi_owner = (DREF DeeObject *)self;
	result->bsi_buf.bb_base = self->bsi_buf.bb_base;
	result->bsi_buf.bb_size = self->bsi_buf.bb_size;
#ifndef __INTELLISENSE__
	result->bsi_buf.bb_put = self->bsi_buf.bb_put;
	if (!result->bsi_buf.bb_put)
		result->bsi_owner = self->bsi_owner;
#endif /* !__INTELLISENSE__ */
	Dee_Incref(result->bsi_owner);
	result->bsi_startbit = range.sr_start + self->bsi_startbit;
	result->bsi_endbit   = range.sr_end + self->bsi_startbit;
	result->bsi_bflags   = self->bsi_bflags;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
bsv_getrange(BitsetView *self, DeeObject *start, DeeObject *end) {
	Dee_ssize_t start_i, end_i = BitsetView_GetNBits(self);
	if (DeeObject_AsSSize(start, &start_i))
		goto err;
	if (!DeeNone_Check(end)) {
		if (DeeObject_AsSSize(end, &end_i))
			goto err;
	}
	return bsv_getrange_i(self, start_i, end_i);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_delrange(BitsetView *self, DeeObject *start, DeeObject *end) {
	struct Dee_seq_range range;
	Dee_ssize_t start_i, end_i = BitsetView_GetNBits(self);
	if (DeeObject_AsSSize(start, &start_i))
		goto err;
	if (!DeeNone_Check(end)) {
		if (DeeObject_AsSSize(end, &end_i))
			goto err;
	}
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	DeeSeqRange_Clamp(&range, start_i, end_i, BitsetView_GetNBits(self));
	bitset_nclear(BitsetView_GetBitset(self),
	              self->bsi_startbit + range.sr_start,
	              self->bsi_startbit + range.sr_end);
	return 0;
err_readonly:
	return bsv_err_readonly(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
bsv_setrange(BitsetView *self, DeeObject *start,
             DeeObject *end, DeeObject *value) {
	Dee_ssize_t start_i, end_i = BitsetView_GetNBits(self);
	struct Dee_seq_range range;
	if (DeeObject_AsSSize(start, &start_i))
		goto err;
	if (!DeeNone_Check(end)) {
		if (DeeObject_AsSSize(end, &end_i))
			goto err;
	}
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	DeeSeqRange_Clamp(&range, start_i, end_i, BitsetView_GetNBits(self));
	if (DeeBool_Check(value)) {
		if (DeeBool_IsTrue(value)) {
			bitset_nset(BitsetView_GetBitset(self),
			            self->bsi_startbit + range.sr_start,
			            self->bsi_startbit + range.sr_end);
		} else {
			bitset_nclear(BitsetView_GetBitset(self),
			              self->bsi_startbit + range.sr_start,
			              self->bsi_startbit + range.sr_end);
		}
	} else if (DeeNone_Check(value)) {
		bitset_nclear(BitsetView_GetBitset(self),
		              self->bsi_startbit + range.sr_start,
		              self->bsi_startbit + range.sr_end);
	} else {
		int result;
		DREF Bitset *value_bitset;
		struct bitset_ref dst, src;
		dst.bsr_bitset   = BitsetView_GetBitset(self);
		dst.bsr_startbit = self->bsi_startbit + range.sr_start;
		dst.bsr_endbit   = self->bsi_startbit + range.sr_end;
		bitset_ref_fix(&dst);
		if (DeeObject_AsBitset(value, &src))
			return bitset_ref_assign(&dst, &src);
		value_bitset = bs_init_fromseq(value, NULL);
		if unlikely(!value_bitset)
			goto err;
		src.bsr_bitset   = value_bitset->bs_bitset;
		src.bsr_startbit = 0;
		src.bsr_endbit   = value_bitset->bs_nbits;
		result = bitset_ref_assign(&dst, &src);
		Dee_DecrefDokill(value_bitset);
		return result;
	}
	return 0;
err_readonly:
	return bsv_err_readonly(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_getbuf(BitsetView *__restrict self,
           DeeBuffer *__restrict info,
           unsigned int flags) {
	struct bitset_ref ref;
	if ((flags & Dee_BUFFER_FWRITABLE) && !(self->bsi_bflags & Dee_BUFFER_FWRITABLE))
		goto err_readonly;
	bitset_ref_fromview(&ref, self);
	info->bb_base = ref.bsr_bitset;
	info->bb_size = (ref.bsr_endbit + _BITSET_WORD_BMSK) >> _BITSET_WORD_SHFT;
	return 0;
err_readonly:
	return bsv_err_readonly(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_insert(BitsetView *__restrict self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	size_t bitno;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_bitno, UNPuSIZ ":insert", &bitno))
		goto err;
	if unlikely(bitno >= BitsetView_GetNBits(self))
		goto err_too_large;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	return_bool(!bitset_atomic_fetchset(BitsetView_GetBitset(self),
	                                    self->bsi_startbit + bitno));
err_readonly:
	bsv_err_readonly(self);
	goto err;
err_too_large:
	bsv_err_bad_index(self, bitno);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_remove(BitsetView *__restrict self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	size_t bitno;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_bitno, UNPuSIZ ":remove", &bitno))
		goto err;
	if unlikely(bitno >= BitsetView_GetNBits(self))
		goto err_too_large;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	return_bool(bitset_atomic_fetchclear(BitsetView_GetBitset(self),
	                                     self->bsi_startbit + bitno));
err_readonly:
	bsv_err_readonly(self);
	goto err;
err_too_large:
	bsv_err_bad_index(self, bitno);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_bytes(BitsetView *__restrict self, size_t argc, DeeObject *const *argv) {
	struct bitset_ref ref;
	DeeObject *owner;
	if (DeeArg_Unpack(argc, argv, ":bytes"))
		goto err;
	bitset_ref_fromview(&ref, self);
	owner = (DeeObject *)self;
#ifndef __INTELLISENSE__
	if (!self->bsi_buf.bb_put)
		owner = self->bsi_owner;
#endif /* !__INTELLISENSE__ */
	return DeeBytes_NewView(owner, ref.bsr_bitset,
	                        (ref.bsr_endbit + _BITSET_WORD_BMSK) >> _BITSET_WORD_SHFT,
	                        self->bsi_bflags);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_frozen(BitsetView *__restrict self) {
	DREF Bitset *result;
	struct bitset_ref ref;
	size_t ref_nbits;
	if (!(self->bsi_bflags & Dee_BUFFER_FWRITABLE)) {
		DeeTypeObject *tp_owner = Dee_TYPE(self->bsi_owner);
		while (tp_owner && !tp_owner->tp_buffer)
			tp_owner = DeeType_Base(tp_owner);
		if (tp_owner && tp_owner->tp_buffer &&
		    (tp_owner->tp_buffer->tp_buffer_flags & Dee_BUFFER_TYPE_FREADONLY)) {
			/* Underlying buffer is always read-only -> can re-return "self" */
			return_reference_((DeeObject *)self);
		}
	}

	/* Must create a new frozen bitset from "self". */
	bitset_ref_fromview(&ref, self);
	ref_nbits = bitset_ref_nbits(&ref);
	if (ref.bsr_startbit == 0) {
		result = Bitset_Alloc(ref_nbits);
		if unlikely(!result)
			goto err;
		copy_aligned_bits_to_0(result->bs_bitset, ref.bsr_bitset, ref_nbits);
	} else {
		/* Slow case: bits don't align, so we must copy them one-at-a-time */
		size_t i;
		result = Bitset_Calloc(ref_nbits);
		if unlikely(!result)
			goto err;
		for (i = 0; i < ref_nbits; ++i) {
			if (bitset_ref_test(&ref, i))
				bitset_set(result->bs_bitset, i);
		}
	}
	result->bs_nbits = ref_nbits;
	DeeObject_Init(result, &RoBitset_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_nbits(BitsetView *__restrict self) {
	size_t result = BitsetView_GetNBits(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_ctor(BitsetView *__restrict self) {
	Dee_Incref(Dee_EmptyBytes);
	self->bsi_owner       = Dee_EmptyBytes;
	self->bsi_buf.bb_base = DeeBytes_DATA(Dee_EmptyBytes);
	self->bsi_buf.bb_size = 0;
#ifndef __INTELLISENSE__
	self->bsi_buf.bb_put = NULL;
#endif /* !__INTELLISENSE__ */
	self->bsi_startbit = 0;
	self->bsi_endbit   = 0;
	self->bsi_bflags   = Dee_BUFFER_FREADONLY;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_copy(BitsetView *__restrict self,
         BitsetView *__restrict other) {
	self->bsi_owner = other->bsi_owner;
	self->bsi_buf.bb_base = other->bsi_buf.bb_base;
	self->bsi_buf.bb_size = other->bsi_buf.bb_size;
#ifndef __INTELLISENSE__
	self->bsi_buf.bb_put = NULL;
	if (other->bsi_buf.bb_put)
		self->bsi_owner = (DREF DeeObject *)other;
#endif /* !__INTELLISENSE__ */
	Dee_Incref(self->bsi_owner);
	self->bsi_startbit = other->bsi_startbit;
	self->bsi_endbit   = other->bsi_startbit;
	self->bsi_bflags   = other->bsi_startbit;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_init(BitsetView *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	unsigned int flags = Dee_BUFFER_FREADONLY;
	size_t start = 0, end = (size_t)-1, nbits;
	if (argc >= 2) {
		ob = argv[0];
		if (DeeString_Check(argv[1])) {
			char *str = DeeString_STR(argv[1]);
			if (WSTR_LENGTH(str) != 1)
				goto err_invalid_mode;
			if (str[0] == 'r') {
				/* ... */
			} else if (str[0] == 'w') {
				flags = Dee_BUFFER_FWRITABLE;
			} else {
				goto err_invalid_mode;
			}
			if (argc >= 3) {
				if (DeeObject_AsSSize(argv[2], (dssize_t *)&start))
					goto err;
				if (argc >= 4) {
					if unlikely(argc > 4)
						goto err_args;
					if (DeeObject_AsSSize(argv[3], (dssize_t *)&end))
						goto err;
				}
			}
		} else {
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&start))
				goto err;
			if (argc >= 3) {
				if unlikely(argc > 3)
					goto err_args;
				if (DeeObject_AsSSize(argv[2], (dssize_t *)&end))
					goto err;
			}
		}
	} else {
		if (argc != 1)
			goto err_args;
		ob = argv[0];
	}
	if (DeeObject_GetBuf(ob, &self->bsi_buf, flags))
		goto err;
	nbits = self->bsi_buf.bb_size << _BITSET_WORD_SHFT;
	if (start > nbits)
		start = nbits;
	if (end > nbits)
		end = nbits;
	self->bsi_owner = ob;
	Dee_Incref(ob);
	self->bsi_startbit = start;
	self->bsi_endbit   = end;
	self->bsi_bflags   = flags;
	return 0;
err_args:
	err_invalid_argc("BitsetView", argc, 1, 4);
	goto err;
err_invalid_mode:
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid buffer mode %r",
	                argv[1]);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
bsv_fini(BitsetView *__restrict self) {
	DeeObject_PutBuf(self->bsi_owner, &self->bsi_buf, self->bsi_bflags);
	Dee_Decref(self->bsi_owner);
}

PRIVATE NONNULL((1)) void DCALL
bsv_visit(BitsetView *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bsi_owner);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_assign(BitsetView *self, DeeObject *value) {
	int result;
	DREF Bitset *value_bitset;
	struct bitset_ref dst, src;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	bitset_ref_fromview(&dst, self);
	if (DeeObject_AsBitset(value, &src))
		return bitset_ref_assign(&dst, &src);
	value_bitset = bs_init_fromseq(value, NULL);
	if unlikely(!value_bitset)
		goto err;
	src.bsr_bitset   = value_bitset->bs_bitset;
	src.bsr_startbit = 0;
	src.bsr_endbit   = value_bitset->bs_nbits;
	result = bitset_ref_assign(&dst, &src);
	Dee_DecrefDokill(value_bitset);
	return result;
err_readonly:
	return bsv_err_readonly(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_bool(BitsetView *self) {
	return bitset_nanyset(BitsetView_GetBitset(self),
	                      BitsetView_GetMinBitno(self),
	                      BitsetView_GetMaxBitno(self))
	       ? 1
	       : 0;
}

PRIVATE WUNUSED NONNULL((1)) dssize_t DCALL
bsv_printrepr(BitsetView *__restrict self, dformatprinter printer, void *arg) {
	dssize_t temp, result;
	DeeObject *owner = self->bsi_owner;
	result = DeeFormat_Printf(printer, arg, "BitsetView(%r", owner);
	if unlikely(result < 0)
		goto done;
	if (self->bsi_bflags & Dee_BUFFER_FWRITABLE)
		DO(err, DeeFormat_PRINT(printer, arg, ", \"w\""));
	if (self->bsi_startbit != 0) {
do_print_start_and_end:
		DO(err, DeeFormat_Printf(printer, arg, ", %" PRFuSIZ ", %" PRFuSIZ,
		                         self->bsi_startbit, self->bsi_endbit));
	} else {
		size_t max_endbit = self->bsi_buf.bb_size << _BITSET_WORD_SHFT;
		if (Bitset_Check(owner))
			max_endbit = ((Bitset *)owner)->bs_nbits;
		if (self->bsi_endbit != max_endbit)
			goto do_print_start_and_end;
	}
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err:
	return temp;
}

PRIVATE struct type_nsi tpconst bsv_nsi = {
	/* .nsi_class = */ TYPE_SEQX_CLASS_SET,
	/* .nsi_flags = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_setlike = */ {
			/* .nsi_getsize = */ (dfunptr_t)&bsv_nsi_getsize,
			/* .nsi_insert  = */ (dfunptr_t)&bsv_nsi_insert,
			/* .nsi_remove  = */ (dfunptr_t)&bsv_nsi_remove,
		}
	}
};

/* Compare operators with optimizations when the operand is another `Bitset' or `BitsetView' */
PRIVATE struct type_cmp bsv_cmp = {
	/* .tp_hash = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&bsv_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_ge,
};

PRIVATE struct type_seq bsv_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsv_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsv_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_contains,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&bsv_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bsv_setitem,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bsv_getrange,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&bsv_delrange,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&bsv_setrange,
	/* .tp_nsi       = */ &bsv_nsi
};

PRIVATE struct type_buffer bsv_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&bsv_getbuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE struct type_method tpconst bsv_methods[] = {
	/* TODO: flip()  (flip all) */
	/* TODO: flip(bitno:?Dint) */
	/* TODO: flip(start:?Dint,end:?Dint) */

	/* TODO: set()  (set all) */
	/* TODO: set(bitno:?Dint) */
	/* TODO: set(start:?Dint,end:?Dint) */

	/* TODO: clear()  (clear all) */
	/* TODO: clear(bitno:?Dint) */
	/* TODO: clear(start:?Dint,end:?Dint) */

	/* TODO: any(start=0,end=-1) */
	/* TODO: all(start=0,end=-1) */
	/* TODO: ffs() ffc() fls() flc() */
	TYPE_KWMETHOD_F("insert", &bsv_insert, METHOD_FNOREFESCAPE,
	                "(bitno:?Dint)->?Dbool\n"
	                "#tValueError{@bitno is negative or too large}"
	                "Turn on @bitno, returning !t if it was turned on, or !f if it was already on"),
	TYPE_KWMETHOD_F("remove", &bsv_remove, METHOD_FNOREFESCAPE,
	                "(bitno:?Dint)->?Dbool\n"
	                "#tValueError{@bitno is negative or too large}"
	                "Turn off @bitno, returning !t if it was turned off, or !f if it was already off"),
	TYPE_METHOD_F("bytes", &bsv_bytes, METHOD_FNOREFESCAPE,
	              "->?DBytes\n"
	              "Returns a view for the underlying bytes of the viewed memory range. "
	              /**/ "Note that the bytes of the returned ?DBytes buffer may hold some "
	              /**/ "extra bits at the start/end when ?#__startbit__ and ?#__endbit__ "
	              /**/ "aren't cleanly divisible by #CNBBY"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst bsv_getsets[] = {
	TYPE_GETTER_F("frozen", &bsv_frozen, METHOD_FNORMAL,
	              "->?X2?.?AFrozen?GBitset\n"
	              "Returns a frozen copy of @this ?., which is either @this when the view is "
	              /**/ "read-only, and the underlying object is also read-only, or a @this view "
	              /**/ "wrapped as a ?AFrozen?GBitset when the underlying bits may be modified "
	              /**/ "by something"),
	TYPE_GETTER_F("nbits", &bsv_nbits, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	              "->?Dint\n"
	              "The # of bits stored in this bitset. Attempting to alter the state of "
	              /**/ "a bit greater than or equal to this value result in an :IndexError"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst bsv_members[] = {
	TYPE_MEMBER_BITFIELD_DOC("iswritable", STRUCT_CONST, BitsetView, bsi_bflags, Dee_BUFFER_FWRITABLE,
	                         "Evaluates to ?t if @this ?. object not be written to (the inverse of ?#isreadonly)"),
	TYPE_MEMBER_BITFIELD_DOC("ismutable", STRUCT_CONST, BitsetView, bsi_bflags, Dee_BUFFER_FWRITABLE,
	                         "Alias for ?#iswritable, overriding ?Aismutable?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__owner__", STRUCT_OBJECT, offsetof(BitsetView, bsi_owner), "->?X2?GBitset?O"),
	TYPE_MEMBER_FIELD_DOC("__startbit__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BitsetView, bsi_startbit),
	                      "Starting bit number in the buffer of ?#__owner__"),
	TYPE_MEMBER_FIELD_DOC("__endbit__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BitsetView, bsi_endbit),
	                      "Ending bit number in the buffer of ?#__owner__"),
	TYPE_MEMBER_FIELD("__flags__", STRUCT_CONST | STRUCT_UINT, offsetof(BitsetView, bsi_bflags)),
	TYPE_MEMBER_END
};

PRIVATE struct type_operator const bsv_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FNOTHROW),
};

INTERN DeeTypeObject BitsetView_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "BitsetView",
	/* .tp_doc      = */ DOC("Similar to ?GBitset, a ?. is a specialized bitset that can be used to view "
	                         /**/ "(and if applicable) modify the bits of objects that implement the #CBuffer "
	                         /**/ "operator group. This allows you to view and modify the individual bits of \n"
	                         /**/ "generic buffers.\n"
	                         "\n"

	                         "()\n"
	                         "Construct an empty ?.\n"
	                         "\n"

	                         "(ob:?O,start=!0,end=!-1)\n"
	                         "(ob:?O,mode=!Pr,start=!0,end=!-1)\n"
	                         "#pmode{Either $\"r\" or $\"w\", specifying the read/write-mode to use}"
	                         "#tNotImplemented{The given @ob does not implement the buffer protocol}"
	                         "Create a ?. object to view the bit-range ${[start,end)} of @ob. "
	                         /**/ "See also ?Aop:constructor?DBytes\n"
	                         "\n"

	                         "copy->\n"
	                         "Returns a copy @this ?.\n"
	                         "\n"

	                         ":=(other:?S?Dint)->\n"
	                         "#tIntegerOverflow{One of the elements of @other is negative or too large}"
	                         "#tIndexError{One of the elements of @other is greater that or equal to ?#nbits}"
	                         "Turn off all bits, except for those whose indices appear in @other\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of 1-bits (that is: the population count) of this ?.\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if at least one bit has been turned on\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the bit indices that are turned on\n"
	                         "\n"

	                         "[]->?Dbool\n"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Returns !t or !f indicative of he the state of the @index'th bit\n"
	                         "\n"

	                         "[]=(index:?Dint,value:?Dbool)\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Set the state of the @index'th bit to @value\n"
	                         "\n"

	                         "del[]->\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Same as ${this[index] = false}\n"
	                         "\n"

	                         "contains(index:?Dint)->?Dbool\n"
	                         "Alias for ${this[index]}\n"
	                         "\n"

	                         "[:](start:?X2?N?Dint,end:?X2?N?Dint)->?.\n"
	                         "Returns a proxy-view for reading/writing the bit-range #C{[start,end)}\n"
	                         "\n"

	                         "del[:]->\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "Set all bits within the range #C{[start,end)} to !f (alias for ${this[start, end] = false})\n"
	                         "\n"

	                         "[:]=(start:?Dint,end:?Dint,value:?Dbool)\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "Turn on/off all bits within the range #C{[start,end)}, as per @value\n"
	                         "\n"

	                         "[:]=(start:?Dint,end:?Dint,values:?S?Dint)\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "Assign @values to the range #C{[start,end)}"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bsv_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bsv_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&bsv_init,
				TYPE_FIXED_ALLOCATOR_GC(BitsetView)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bsv_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&bsv_assign,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&bsv_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&bsv_printrepr,
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bsv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL, /* TODO: "|=" "&=" "^=" */
	/* .tp_cmp           = */ &bsv_cmp,
	/* .tp_seq           = */ &bsv_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ &bsv_buffer,
	/* .tp_methods       = */ bsv_methods,
	/* .tp_getsets       = */ bsv_getsets,
	/* .tp_members       = */ bsv_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bsv_class_members,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ bsv_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(bsv_operators)
};





PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsiter_nii_getseq(BitsetIterator *__restrict self) {
	return_reference_(self->bsi_owner);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bsiter_nii_getindex(BitsetIterator *__restrict self) {
	size_t startbit = BitsetIterator_GetStartBit(self);
	size_t endbit   = BitsetIterator_GetEndBit(self);
	size_t bitno = atomic_read(&self->bsi_bitno);
	if (bitno >= startbit && bitno <= endbit) /* NOTE: `<= endbit' due to final, exhausted position */
		return bitno - startbit;
	return (size_t)-2; /* Indeterminate */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_setindex(BitsetIterator *__restrict self, size_t index) {
	size_t startbit = BitsetIterator_GetStartBit(self);
	size_t endbit   = BitsetIterator_GetEndBit(self);
	size_t nbits    = endbit - startbit;
	if (index > nbits)
		index = nbits;
	atomic_write(&self->bsi_bitno, startbit + index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_rewind(BitsetIterator *__restrict self) {
	size_t startbit = BitsetIterator_GetStartBit(self);
	atomic_write(&self->bsi_bitno, startbit);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_revert(BitsetIterator *__restrict self, size_t step) {
	size_t old_bitno, new_bitno, min_bitno;
	min_bitno = BitsetIterator_GetStartBit(self);
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno <= min_bitno)
			return 1; /* Already at starting position */
		if (OVERFLOW_USUB(old_bitno, step, &new_bitno) || new_bitno < min_bitno)
			new_bitno = min_bitno;
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno));
	if (new_bitno <= min_bitno)
		return 1; /* Now at starting position */
	return 2;     /* Iterator isn't at its starting position */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_advance(BitsetIterator *__restrict self, size_t step) {
	size_t old_bitno, new_bitno, end_bitno;
	end_bitno = BitsetIterator_GetEndBit(self);
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno >= end_bitno)
			return 1; /* Already at end position */
		if (OVERFLOW_UADD(old_bitno, step, &new_bitno) || new_bitno >= end_bitno)
			new_bitno = end_bitno;
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno));
	if (new_bitno >= end_bitno)
		return 1; /* Now at end position */
	return 2;     /* Iterator isn't at its end position */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_prev(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno, min_bitno;
	min_bitno = BitsetIterator_GetStartBit(self);
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno <= min_bitno)
			return 1; /* Already at starting position */
		new_bitno = old_bitno - 1;
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_next(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno, end_bitno;
	end_bitno = BitsetIterator_GetEndBit(self);
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno >= end_bitno)
			return 1; /* Already at end position */
		new_bitno = old_bitno + 1;
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno));
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_hasprev(BitsetIterator *__restrict self) {
	size_t startbit = BitsetIterator_GetStartBit(self);
	size_t bitno    = atomic_read(&self->bsi_bitno);
	return bitno > startbit ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsiter_nii_peek(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno, end_bitno;
	end_bitno = BitsetIterator_GetEndBit(self);
	old_bitno = atomic_read(&self->bsi_bitno);
	if (old_bitno >= end_bitno)
		return ITER_DONE; /* Already at end position */
	new_bitno = bitset_nffs(self->bsi_bitset, old_bitno, end_bitno);
	if (new_bitno >= end_bitno)
		return ITER_DONE; /* End position reached */
	return DeeInt_NewSize(new_bitno);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsiter_next(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno, end_bitno;
	end_bitno = BitsetIterator_GetEndBit(self);
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno >= end_bitno)
			return ITER_DONE; /* Already at end position */
		new_bitno = bitset_nffs(self->bsi_bitset, old_bitno, end_bitno);
		if (new_bitno >= end_bitno) {
#ifndef __OPTIMIZE_SIZE__
			atomic_cmpxch_weak_or_write(&self->bsi_bitno, old_bitno, new_bitno);
#endif /* !__OPTIMIZE_SIZE__ */
			return ITER_DONE; /* End position reached */
		}
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno + 1));
	return DeeInt_NewSize(new_bitno);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_bool(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno, end_bitno;
	end_bitno = BitsetIterator_GetEndBit(self);
	old_bitno = atomic_read(&self->bsi_bitno);
	if (old_bitno >= end_bitno)
		return 0; /* Already at end position */
	new_bitno = bitset_nffs(self->bsi_bitset, old_bitno, end_bitno);
	if (new_bitno >= end_bitno)
		return 0; /* End position reached */
	return 1;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsiter_eq(BitsetIterator *self, BitsetIterator *other) {
	if (DeeObject_AssertTypeExact(other, &BitsetIterator_Type))
		goto err;
	return_bool(self->bsi_bitset == other->bsi_bitset &&
	            self->bsi_nbits == other->bsi_nbits &&
	            atomic_read(&self->bsi_bitno) == atomic_read(&other->bsi_bitno));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsiter_ne(BitsetIterator *self, BitsetIterator *other) {
	if (DeeObject_AssertTypeExact(other, &BitsetIterator_Type))
		goto err;
	return_bool(self->bsi_bitset != other->bsi_bitset ||
	            self->bsi_nbits != other->bsi_nbits ||
	            atomic_read(&self->bsi_bitno) != atomic_read(&other->bsi_bitno));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsiter_lo(BitsetIterator *self, BitsetIterator *other) {
	if (DeeObject_AssertTypeExact(other, &BitsetIterator_Type))
		goto err;
	return_bool((self->bsi_bitset == other->bsi_bitset && self->bsi_nbits == other->bsi_nbits)
	            ? (atomic_read(&self->bsi_bitno) < atomic_read(&other->bsi_bitno))
	            : (self->bsi_bitset < other->bsi_bitset ||
	               (self->bsi_bitset == other->bsi_bitset && self->bsi_nbits < other->bsi_nbits)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsiter_le(BitsetIterator *self, BitsetIterator *other) {
	if (DeeObject_AssertTypeExact(other, &BitsetIterator_Type))
		goto err;
	return_bool((self->bsi_bitset == other->bsi_bitset && self->bsi_nbits == other->bsi_nbits)
	            ? (atomic_read(&self->bsi_bitno) <= atomic_read(&other->bsi_bitno))
	            : (self->bsi_bitset < other->bsi_bitset ||
	               (self->bsi_bitset == other->bsi_bitset && self->bsi_nbits < other->bsi_nbits)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsiter_gr(BitsetIterator *self, BitsetIterator *other) {
	if (DeeObject_AssertTypeExact(other, &BitsetIterator_Type))
		goto err;
	return_bool((self->bsi_bitset == other->bsi_bitset && self->bsi_nbits == other->bsi_nbits)
	            ? (atomic_read(&self->bsi_bitno) > atomic_read(&other->bsi_bitno))
	            : (self->bsi_bitset > other->bsi_bitset ||
	               (self->bsi_bitset == other->bsi_bitset && self->bsi_nbits > other->bsi_nbits)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsiter_ge(BitsetIterator *self, BitsetIterator *other) {
	if (DeeObject_AssertTypeExact(other, &BitsetIterator_Type))
		goto err;
	return_bool((self->bsi_bitset == other->bsi_bitset && self->bsi_nbits == other->bsi_nbits)
	            ? (atomic_read(&self->bsi_bitno) >= atomic_read(&other->bsi_bitno))
	            : (self->bsi_bitset > other->bsi_bitset ||
	               (self->bsi_bitset == other->bsi_bitset && self->bsi_nbits > other->bsi_nbits)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_ctor(BitsetIterator *__restrict self) {
	self->bsi_owner = (DREF DeeObject *)bs_ctor();
	if unlikely(!self->bsi_owner)
		goto err;
	self->bsi_bitset = ((Bitset *)self->bsi_owner)->bs_bitset;
	self->bsi_nbits  = 0;
	self->bsi_bitno  = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsiter_copy(BitsetIterator *__restrict self,
            BitsetIterator *__restrict other) {
	self->bsi_owner = other->bsi_owner;
	Dee_Incref(self->bsi_owner);
	self->bsi_bitset = other->bsi_bitset;
	self->bsi_nbits  = other->bsi_nbits;
	self->bsi_bitno  = atomic_read(&other->bsi_bitno);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_init(BitsetIterator *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:BitsetIterator", &self->bsi_owner))
		goto err;
	if (BitsetView_Check(self->bsi_owner)) {
		BitsetView *o = (BitsetView *)self->bsi_owner;
		self->bsi_bitset = BitsetView_GetBitset(o);
		self->bsi_nbits  = o->bsi_endbit;
		self->bsi_bitno  = o->bsi_startbit;
	} else {
		Bitset *o = (Bitset *)self->bsi_owner;
		if (DeeObject_AssertTypeExact(self->bsi_owner, &Bitset_Type))
			goto err;
		self->bsi_bitset = o->bs_bitset;
		self->bsi_nbits  = o->bs_nbits;
		self->bsi_bitno  = 0;
	}
	Dee_Incref(self->bsi_owner);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
bsiter_fini(BitsetIterator *__restrict self) {
	Dee_Decref(self->bsi_owner);
}

PRIVATE NONNULL((1)) void DCALL
bsiter_visit(BitsetIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->bsi_owner);
}


PRIVATE struct type_nii tpconst bsiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&bsiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&bsiter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&bsiter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&bsiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)&bsiter_nii_revert,
			/* .nii_advance  = */ (dfunptr_t)&bsiter_nii_advance,
			/* .nii_prev     = */ (dfunptr_t)&bsiter_nii_prev,
			/* .nii_next     = */ (dfunptr_t)&bsiter_nii_next,
			/* .nii_hasprev  = */ (dfunptr_t)&bsiter_nii_hasprev,
			/* .nii_peek     = */ (dfunptr_t)&bsiter_nii_peek
		}
	}
};

PRIVATE struct type_cmp bsiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsiter_ge,
	/* .tp_nii  = */ &bsiter_nii
};

PRIVATE struct type_member tpconst bsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BitsetIterator, bsi_owner),
	                      "->?X2?GBitset?AView?GBitset\n"
	                      "The ?GBitset or ?AView?GBitset that is being iterated"),
	TYPE_MEMBER_FIELD_DOC("__nbits__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BitsetIterator, bsi_nbits),
	                      "The bit index at which iteration will stop"),
	TYPE_MEMBER_FIELD_DOC("__bitno__", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(BitsetIterator, bsi_bitno),
	                      "The next bit index to iterate (skipping bits that aren't turned on)"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BitsetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "BitsetIterator",
	/* .tp_doc      = */ DOC("(seq?:?GBitset)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bsiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&bsiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&bsiter_init,
				TYPE_FIXED_ALLOCATOR(BitsetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&bsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&bsiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&bsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &bsiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END


#endif /* !GUARD_DEX_COLLECTIONS_BITSET_C */
