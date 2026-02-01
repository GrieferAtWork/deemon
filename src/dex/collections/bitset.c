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
#ifndef GUARD_DEX_COLLECTIONS_BITSET_C
#define GUARD_DEX_COLLECTIONS_BITSET_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_*, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>             /* DeeArg_BadArgcEx, DeeArg_Unpack*, UNPuSIZ, UNPxSIZ */
#include <deemon/bool.h>            /* DeeBool_Check, DeeBool_IsTrue, return_bool, return_false, return_true */
#include <deemon/bytes.h>           /* DeeBytes* */
#include <deemon/error-rt.h>        /* DeeRT_ErrIndexOutOfBounds, DeeRT_ErrIndexOverflow */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/format.h>          /* DeeFormat_PRINT, DeeFormat_Printf, PRFuSIZ */
#include <deemon/int.h>             /* DeeInt_* */
#include <deemon/none.h>            /* DeeNone_Check, return_none */
#include <deemon/object.h>
#include <deemon/seq.h>             /* DeeIterator_Type, DeeSeqRange_Clamp, DeeSeqRange_Clamp_n, DeeSeq_Type, Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL, Dee_TYPE_ITERX_FNORMAL, Dee_seq_range, type_nii */
#include <deemon/serial.h>          /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/set.h>             /* DeeSet_Type */
#include <deemon/string.h>          /* DeeString_Check, DeeString_STR, WSTR_LENGTH */
#include <deemon/system-features.h> /* memcpy* */
#include <deemon/util/atomic.h>     /* atomic_* */
#include <deemon/util/hash.h>       /* Dee_HASHOF_EMPTY_SEQUENCE, Dee_HashCombine */

#include <hybrid/__atomic.h>  /*  */
#include <hybrid/__bitset.h>  /*  */
#include <hybrid/bitset.h>    /* BITSET_*, _BITSET_*, bitset_* */
#include <hybrid/limitcore.h> /* __SSIZE_MAX__ */
#include <hybrid/overflow.h>  /* OVERFLOW_UADD, OVERFLOW_USUB */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

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






/************************************************************************/

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
	DREF DeeObject *bsv_owner;    /* [1..1][const] Some object that is providing the buffer whose bits are being enumerated. */
	DeeBuffer       bsv_buf;      /* [const] The buffer of raw bytes that are being viewed.
	                               * Unused bits in the first/last byte are undefined. */
	size_t          bsv_startbit; /* [const][<= bsv_endbit] Starting bit number part of the bitset (based at `bsv_buf.bb_base') */
	size_t          bsv_endbit;   /* [const][<= bsv_startbit] End bit number part of the bitset (based at `bsv_buf.bb_base') */
	unsigned int    bsv_bflags;   /* [const] Buffer flags (Set of `Dee_BUFFER_F*'; of relevance is `Dee_BUFFER_FWRITABLE') */
} BitsetView;

#define BitsetView_Check(ob) /* BitsetView is final, so exact check */ \
	DeeObject_InstanceOfExact(ob, &BitsetView_Type)
#define BitsetView_IsWritable(ob) ((ob)->bsv_bflags & Dee_BUFFER_FWRITABLE)
#define BitsetView_GetBitset(ob)  ((bitset_t *)(ob)->bsv_buf.bb_base)
#define BitsetView_GetNBits(ob)   ((ob)->bsv_endbit - (ob)->bsv_startbit)

typedef struct {
	OBJECT_HEAD
	DREF DeeObject *bsi_owner;    /* [1..1][const] Either the Bitset or BitsetView that is being iterated */
	bitset_t       *bsi_bitset;   /* [1..1][const] Base address of the bitset begin iterated */
	size_t          bsi_startbit; /* [const][<= bsi_endbit] Staring bit number */
	size_t          bsi_endbit;   /* [const][>= bsi_startbit] End bit number */
	size_t          bsi_bitno;    /* [<= bsi_nbits][lock(ATOMIC)] Next bitno to iterate. */
} BitsetIterator;

#define BitsetIterator_Check(ob) /* BitsetIterator is final, so exact check */ \
	DeeObject_InstanceOfExact(ob, &BitsetIterator_Type)






/************************************************************************/
PRIVATE ATTR_COLD NONNULL((1)) int DCALL
bs_err_bad_index(Bitset *__restrict self, size_t bitno) {
	return DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), bitno, self->bs_nbits);
}






/************************************************************************/

PRIVATE NONNULL((1, 2)) void DCALL
bitset_ncopy0_and_zero_unused_bits(bitset_t *dst, bitset_t const *src,
                                   size_t src_startbitno, size_t n_bits) {
#if 1
#define bitset_ncopy0_and_maybe_zero_unused_bits bitset_ncopy0_and_zero_unused_bits
	if (src_startbitno > _BITSET_WORD_BMSK) {
		src += src_startbitno >> _BITSET_WORD_SHFT;
		src_startbitno &= _BITSET_WORD_BMSK;
	}
	ASSERT(src_startbitno <= _BITSET_WORD_BMSK);
	if (src_startbitno == 0) {
		size_t whole_words = n_bits >> _BITSET_WORD_SHFT;
		memcpyc(dst, src, whole_words, sizeof(bitset_t));
		if (n_bits & _BITSET_WORD_BMSK) {
			bitset_t mask;
			dst += whole_words;
			src += whole_words;
			mask = _BITSET_LO_MASKIN(n_bits & _BITSET_WORD_BMSK);
			*dst = (*src & mask);
		}
	} else {
		/* Bitsets aren't aligned -> copy 1 bit at-a-time */
		size_t i;
		if (dst <= src) {
			for (i = 0; i < n_bits; ++i) {
				if (bitset_test(src, src_startbitno + i)) {
					bitset_set(dst, i);
				} else {
					bitset_clear(dst, i);
				}
			}
		} else {
			i = n_bits;
			while (i) {
				--i;
				if (bitset_test(src, src_startbitno + i)) {
					bitset_set(dst, i);
				} else {
					bitset_clear(dst, i);
				}
			}
		}
	}
#else
#define bitset_ncopy0_and_maybe_zero_unused_bits bitset_ncopy0
	bitset_ncopy0(dst, src, src_startbitno, n_bits);
	if (n_bits & _BITSET_WORD_BMSK)
		dst[n_bits >> _BITSET_WORD_SHFT] &= _BITSET_LO_MASKIN(n_bits & _BITSET_WORD_BMSK);
#endif
}

PRIVATE NONNULL((1)) void DCALL
bitset_setall_and_zero_unused_bits(bitset_t *self, size_t n_bits) {
	bitset_setall(self, n_bits);
	if (n_bits & _BITSET_WORD_BMSK) {
		/* bitset_setall() may set the unused high bits of the last word.
		 * We don't want that, so fix the last word if that happened */
		self[n_bits >> _BITSET_WORD_SHFT] &= _BITSET_LO_MASKIN(n_bits & _BITSET_WORD_BMSK);
	}
}

PRIVATE NONNULL((1)) void DCALL
bitset_flipall_and_zero_unused_bits(bitset_t *self, size_t n_bits) {
	bitset_flipall(self, n_bits);
	if (n_bits & _BITSET_WORD_BMSK) {
		/* bitset_flipall() may set the unused high bits of the last word.
		 * We don't want that, so fix the last word if that happened */
		self[n_bits >> _BITSET_WORD_SHFT] &= _BITSET_LO_MASKIN(n_bits & _BITSET_WORD_BMSK);
	}
}






/************************************************************************/

struct bitset_fromseq_data {
	DREF Bitset *bsfsd_bitset; /* [1..1] The bitset being created. */
	size_t       bsfsd_abits;  /* Allocated (and 0-initialized) # of bits (max used
	                            * # of bits is stored in `bsfsd_bitset->bs_nbits') */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
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
bs_size(Bitset *__restrict self) {
	return bitset_popcount(self->bs_bitset, self->bs_nbits);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
bs_hash(Bitset *__restrict self) {
	size_t bitno;
	Dee_hash_t result = Dee_HASHOF_EMPTY_SEQUENCE;
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

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
bitset_ref_assign(struct bitset_ref *__restrict dst,
                  struct bitset_ref *__restrict src,
                  DeeObject *dst_obj) {
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
			return DeeRT_ErrIndexOutOfBounds(dst_obj, oob_index, dst_bits);
		}
		/* Trim to the size of our own bitset. */
		src->bsr_endbit = src->bsr_startbit + dst_bits;
		src_bits = dst_bits;
	}

	/* Copy the actual contents of the bitsets. */
	ASSERT(src_bits == bitset_ref_nbits(src));
	ASSERT(dst->bsr_startbit <= _BITSET_WORD_BMSK);
	ASSERT(src->bsr_startbit <= _BITSET_WORD_BMSK);
	bitset_ncopy(dst->bsr_bitset, dst->bsr_startbit,
	             src->bsr_bitset, src->bsr_startbit,
	             src_bits);

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
	self->bsr_startbit = view->bsv_startbit;
	self->bsr_endbit   = view->bsv_endbit;
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
				ASSERT(ref.bsr_startbit <= _BITSET_WORD_BMSK);
				bitset_ncopy0_and_maybe_zero_unused_bits(result->bs_bitset, ref.bsr_bitset,
				                                         ref.bsr_startbit, ref_nbits);
				result->bs_nbits = minbits;
				DeeObject_Init(result, &Bitset_Type);
				return result;
			}
		}
		result = Bitset_Alloc(ref_nbits);
		if unlikely(!result)
			goto err;
		bitset_ncopy0_and_zero_unused_bits(result->bs_bitset, ref.bsr_bitset,
		                                   ref.bsr_startbit, ref_nbits);
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
                       struct bitset_ref *__restrict ref) {
	size_t self_nbits = self->bs_nbits;
	if (self_nbits != bitset_ref_nbits(ref)) {
		/* Trim trailing 0-bits from "other" */
		size_t ot_nbits;
		ot_nbits = bitset_nfls(ref->bsr_bitset,
		                       ref->bsr_startbit,
		                       ref->bsr_endbit);
		if (ot_nbits >= ref->bsr_endbit) {
			/* "other" is empty -> check if "self" is empty, too */
			return !bitset_anyset(self->bs_bitset, self->bs_nbits);
		}
		ref->bsr_endbit = ot_nbits + 1;
		if (self_nbits != bitset_ref_nbits(ref)) {
			/* Trim trailing 0-bits from "self" */
			size_t lastset = bitset_fls(self->bs_bitset, self_nbits);
			if (lastset >= self_nbits)
				goto nope; /* "self" is empty */
			self_nbits = lastset + 1;
			if (self_nbits != bitset_ref_nbits(ref))
				goto nope; /* "self" has a different greatest-set-bit */
		}
	}

	ASSERT(ref->bsr_startbit <= _BITSET_WORD_BMSK);
	return bitset_ncmpeq0(self->bs_bitset, ref->bsr_bitset,
	                      ref->bsr_startbit, self_nbits);
nope:
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_eq(Bitset *self, DeeObject *other) {
	struct bitset_ref ref;
	if (!DeeObject_AsBitset(other, &ref))
		return (*DeeSeq_Type.tp_cmp->tp_eq)(Dee_AsObject(self), other);
	return_bool(bs_cmp_eqne_bitset_ref(self, &ref));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_ne(Bitset *self, DeeObject *other) {
	struct bitset_ref ref;
	if (!DeeObject_AsBitset(other, &ref))
		return (*DeeSeq_Type.tp_cmp->tp_ne)(Dee_AsObject(self), other);
	return_bool(!bs_cmp_eqne_bitset_ref(self, &ref));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_le(Bitset *self, DeeObject *other) {
	size_t n_bits;
	struct bitset_ref ref;
	if (!DeeObject_AsBitset(other, &ref))
		return (*DeeSeq_Type.tp_cmp->tp_le)(Dee_AsObject(self), other);
	/* All bits from "self" must also be set in "other" */
	n_bits = self->bs_nbits;
	if (n_bits > bitset_ref_nbits(&ref)) {
		if (bitset_nanyset(self->bs_bitset, bitset_ref_nbits(&ref), n_bits))
			goto nope;
		n_bits = bitset_ref_nbits(&ref);
	}
	ASSERT(ref.bsr_startbit <= _BITSET_WORD_BMSK);
	if (!bitset_ncmple0(self->bs_bitset, ref.bsr_bitset, ref.bsr_startbit, n_bits))
		goto nope;
	return_true;
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_ge(Bitset *self, DeeObject *other) {
	size_t n_bits;
	struct bitset_ref ref;
	if (!DeeObject_AsBitset(other, &ref))
		return (*DeeSeq_Type.tp_cmp->tp_ge)(Dee_AsObject(self), other);
	/* All bits from "other" must also be set in "self" */
	n_bits = bitset_ref_nbits(&ref);
	if (n_bits > self->bs_nbits) {
		if (bitset_ref_nanyset(&ref, self->bs_nbits, n_bits))
			goto nope;
		n_bits = self->bs_nbits;
	}
	ASSERT(ref.bsr_startbit <= _BITSET_WORD_BMSK);
	if (!bitset_ncmpge0(self->bs_bitset, ref.bsr_bitset, ref.bsr_startbit, n_bits))
		goto nope;
	return_true;
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_gr(Bitset *self, DeeObject *other) {
	size_t n_bits;
	struct bitset_ref ref;
	if (!DeeObject_AsBitset(other, &ref))
		return (*DeeSeq_Type.tp_cmp->tp_gr)(Dee_AsObject(self), other);
	/* not(All bits from "self" must also be set in "other") */
	n_bits = self->bs_nbits;
	if (n_bits > bitset_ref_nbits(&ref)) {
		if (!bitset_nanyset(self->bs_bitset, bitset_ref_nbits(&ref), n_bits))
			goto nope;
		n_bits = bitset_ref_nbits(&ref);
	}
	ASSERT(ref.bsr_startbit <= _BITSET_WORD_BMSK);
	if (!bitset_ncmpgr0(self->bs_bitset, ref.bsr_bitset, ref.bsr_startbit, n_bits))
		goto nope;
	return_true;
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bs_lo(Bitset *self, DeeObject *other) {
	size_t n_bits;
	struct bitset_ref ref;
	if (!DeeObject_AsBitset(other, &ref))
		return (*DeeSeq_Type.tp_cmp->tp_lo)(Dee_AsObject(self), other);
	/* not(All bits from "other" must also be set in "self") */
	n_bits = bitset_ref_nbits(&ref);
	if (n_bits > self->bs_nbits) {
		if (!bitset_ref_nanyset(&ref, self->bs_nbits, n_bits))
			goto nope;
		n_bits = self->bs_nbits;
	}
	ASSERT(ref.bsr_startbit <= _BITSET_WORD_BMSK);
	if (!bitset_ncmplo0(self->bs_bitset, ref.bsr_bitset, ref.bsr_startbit, n_bits))
		goto nope;
	return_true;
nope:
	return_false;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetIterator *DCALL
bs_iter(Bitset *__restrict self) {
	DREF BitsetIterator *result;
	result = DeeObject_MALLOC(BitsetIterator);
	if unlikely(!result)
		goto err;
	result->bsi_owner = Dee_AsObject(self);
	Dee_Incref(self);
	result->bsi_bitset   = self->bs_bitset;
	result->bsi_startbit = 0;
	result->bsi_endbit   = self->bs_nbits;
	result->bsi_bitno    = 0;
	DeeObject_Init(result, &BitsetIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_contains(Bitset *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	return_bool(bitno < self->bs_nbits &&
	            bitset_test(self->bs_bitset, bitno));
err:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		return_false;
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_getitem_index(Bitset *self, size_t bitno) {
	if unlikely(bitno >= self->bs_nbits)
		goto err_too_large;
	return_bool(bitset_test(self->bs_bitset, bitno));
err_too_large:
	bs_err_bad_index(self, bitno);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_trygetitem_index(Bitset *self, size_t bitno) {
	if unlikely(bitno >= self->bs_nbits)
		return ITER_DONE;
	return_bool(bitset_test(self->bs_bitset, bitno));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_delitem_index(Bitset *self, size_t bitno) {
	if unlikely(bitno >= self->bs_nbits)
		goto err_too_large;
	bitset_atomic_clear(self->bs_bitset, bitno);
	return 0;
err_too_large:
	return bs_err_bad_index(self, bitno);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
bs_setitem_index(Bitset *self, size_t bitno, DeeObject *value) {
	int temp;
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
bs_getrange_index(Bitset *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, self->bs_nbits);
	result->bsv_owner = Dee_AsObject(self);
	Dee_Incref(self);
	result->bsv_buf.bb_base = self->bs_bitset;
	result->bsv_buf.bb_size = BITSET_SIZEOF(self->bs_nbits);
	result->bsv_startbit    = range.sr_start;
	result->bsv_endbit      = range.sr_end;
	result->bsv_bflags      = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
bs_getrange_index_n(Bitset *self, Dee_ssize_t start) {
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	result->bsv_owner = Dee_AsObject(self);
	Dee_Incref(self);
	result->bsv_buf.bb_base = self->bs_bitset;
	result->bsv_buf.bb_size = BITSET_SIZEOF(self->bs_nbits);
	result->bsv_startbit    = DeeSeqRange_Clamp_n(start, self->bs_nbits);
	result->bsv_endbit      = self->bs_nbits;
	result->bsv_bflags      = Dee_BUFFER_FWRITABLE;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_delrange_index(Bitset *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DeeSeqRange_Clamp(&range, start, end, self->bs_nbits);
	bitset_nclear(self->bs_bitset, range.sr_start, range.sr_end);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_delrange_index_n(Bitset *self, Dee_ssize_t start) {
	size_t used_start = DeeSeqRange_Clamp_n(start, self->bs_nbits);
	bitset_nclear(self->bs_bitset, used_start, self->bs_nbits);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
bs_setrange_index(Bitset *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *value) {
	struct Dee_seq_range range;
	DeeSeqRange_Clamp(&range, start, end, self->bs_nbits);
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
			return bitset_ref_assign(&dst, &src, Dee_AsObject(self));
		value_bitset = bs_init_fromseq(value, NULL);
		if unlikely(!value_bitset)
			goto err;
		src.bsr_bitset   = value_bitset->bs_bitset;
		src.bsr_startbit = 0;
		src.bsr_endbit   = value_bitset->bs_nbits;
		result = bitset_ref_assign(&dst, &src, Dee_AsObject(self));
		Dee_DecrefDokill(value_bitset);
		return result;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
bs_setrange_index_n(Bitset *self, Dee_ssize_t start, DeeObject *value) {
	return bs_setrange_index(self, start, SSIZE_MAX, value);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_flip(Bitset *__restrict self, size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 0:
		bitset_flipall_and_zero_unused_bits(self->bs_bitset, self->bs_nbits);
		break;

	case 1: {
		size_t index;
		bool old_state;
		if (DeeObject_AsSize(argv[0], &index))
			goto err_maybe_overflow;
		if unlikely(index >= self->bs_nbits) {
			bs_err_bad_index(self, index);
			goto err;
		}
		old_state = bitset_atomic_fetchflip(self->bs_bitset, index);
		return_bool(old_state);
	}	break;

	case 2: {
		size_t start, end;
		if (DeeObject_AsSize(argv[0], &start))
			goto err;
		if (DeeObject_AsSizeM1(argv[1], &end))
			goto err;
		if (end > self->bs_nbits)
			end = self->bs_nbits;
		if (start > end)
			start = end;
		bitset_nflip(self->bs_bitset, start, end);
	}	break;

	default:
		DeeArg_BadArgcEx("flip", argc, 0, 2);
		goto err;
	}
	return_none;
err_maybe_overflow:
	DeeRT_ErrIndexOverflow(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_set(Bitset *__restrict self, size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 0:
		bitset_setall_and_zero_unused_bits(self->bs_bitset, self->bs_nbits);
		break;

	case 1: {
		size_t index;
		bool old_state;
		if (DeeObject_AsSize(argv[0], &index))
			goto err_maybe_overflow;
		if unlikely(index >= self->bs_nbits) {
			bs_err_bad_index(self, index);
			goto err;
		}
		old_state = bitset_atomic_fetchset(self->bs_bitset, index);
		return_bool(old_state);
	}	break;

	case 2: {
		size_t start, end;
		if (DeeObject_AsSize(argv[0], &start))
			goto err;
		if (DeeObject_AsSizeM1(argv[1], &end))
			goto err;
		if (end > self->bs_nbits)
			end = self->bs_nbits;
		if (start > end)
			start = end;
		bitset_nset(self->bs_bitset, start, end);
	}	break;

	default:
		DeeArg_BadArgcEx("set", argc, 0, 2);
		goto err;
	}
	return_none;
err_maybe_overflow:
	DeeRT_ErrIndexOverflow(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_clear(Bitset *__restrict self, size_t argc, DeeObject *const *argv) {
	switch (argc) {
	case 0:
		bitset_clearall(self->bs_bitset, self->bs_nbits);
		break;

	case 1: {
		size_t index;
		bool old_state;
		if (DeeObject_AsSize(argv[0], &index))
			goto err_maybe_overflow;
		if unlikely(index >= self->bs_nbits) {
			bs_err_bad_index(self, index);
			goto err;
		}
		old_state = bitset_atomic_fetchclear(self->bs_bitset, index);
		return_bool(old_state);
	}	break;

	case 2: {
		size_t start, end;
		if (DeeObject_AsSize(argv[0], &start))
			goto err;
		if (DeeObject_AsSizeM1(argv[1], &end))
			goto err;
		if (end > self->bs_nbits)
			end = self->bs_nbits;
		if (start > end)
			start = end;
		bitset_nclear(self->bs_bitset, start, end);
	}	break;

	default:
		DeeArg_BadArgcEx("clear", argc, 0, 2);
		goto err;
	}
	return_none;
err_maybe_overflow:
	DeeRT_ErrIndexOverflow(self);
err:
	return NULL;
}

PRIVATE DEFINE_KWLIST(kwlist_start_end, { K(start), K(end), KEND });

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_popcount(Bitset *__restrict self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":popcount",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_npopcount(self->bs_bitset, start, end);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_any(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":any",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	return_bool(bitset_nanyset(self->bs_bitset, start, end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_all(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":all",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	return_bool(bitset_nallset(self->bs_bitset, start, end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_ffs(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":ffs",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_nffs(self->bs_bitset, start, end);
	if (result >= end)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_ffc(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":ffc",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_nffc(self->bs_bitset, start, end);
	if (result >= end)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_fls(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":fls",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_nfls(self->bs_bitset, start, end);
	if (result >= end)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_flc(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":flc",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_nflc(self->bs_bitset, start, end);
	if (result >= end)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_clz(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":clz",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_nfls(self->bs_bitset, start, end);
	if (result >= end)
		result = (start - 1);
	return DeeInt_NewSize((end - 1) - result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_ctz(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":ctz",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_nffs(self->bs_bitset, start, end);
	if (result > end)
		result = end;
	return DeeInt_NewSize(result - start);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_clo(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":clo",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_nflc(self->bs_bitset, start, end);
	if (result >= end)
		result = (start - 1);
	return DeeInt_NewSize((end - 1) - result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bs_cto(Bitset *__restrict self, size_t argc,
       DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":cto",
	                    &start, &end))
		goto err;
	if (end > self->bs_nbits)
		end = self->bs_nbits;
	if (start > end)
		start = end;
	result = bitset_nffc(self->bs_bitset, start, end);
	if (result > end)
		result = end;
	return DeeInt_NewSize(result - start);
err:
	return NULL;
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
	DeeArg_Unpack0(err, argc, argv, "bytes");
	return DeeBytes_NewView(Dee_AsObject(self),
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
bs_serialize(Bitset *__restrict self,
             DeeSerial *__restrict writer) {
	Bitset *out;
	size_t sizeof_bits = BITSET_SIZEOF(self->bs_nbits);
	size_t sizeof_self = offsetof(Bitset, bs_bitset) + sizeof_bits;
	Dee_seraddr_t out_addr = DeeSerial_ObjectMalloc(writer, sizeof_self, self);
#define ADDROF(field) (out_addr + offsetof(Bitset, field))
	if unlikely(!Dee_SERADDR_ISOK(out_addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, Bitset);
	out->bs_nbits = self->bs_nbits;
	memcpy(out->bs_bitset, self->bs_bitset, sizeof_bits);
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
#undef ADDROF
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
	DeeArg_Unpack1Or2(err, argc, argv, "Bitset", &seq_or_nbits, &init_or_minbits);
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
		bitset_setall_and_zero_unused_bits(result->bs_bitset, nbits);
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
bs_assign_bitset(Bitset *self, struct bitset_ref *__restrict ref) {
	size_t copy_bits = bitset_ref_nbits(ref);
	if (copy_bits > self->bs_nbits) {
		/* Ensure that no bit is set that can't be represented in "self" */
		size_t oob_index;
		oob_index = bitset_nffs(ref->bsr_bitset,
		                        ref->bsr_startbit + self->bs_nbits,
		                        ref->bsr_endbit);
		if (oob_index < ref->bsr_endbit) {
			oob_index -= ref->bsr_startbit;
			ASSERT(oob_index >= self->bs_nbits);
			return bs_err_bad_index(self, oob_index);
		}
		/* Trim to the size of our own bitset. */
		ref->bsr_endbit = ref->bsr_startbit + self->bs_nbits;
		copy_bits = self->bs_nbits;
	}

	ASSERT(copy_bits == bitset_ref_nbits(ref));
	ASSERT(ref->bsr_startbit <= _BITSET_WORD_BMSK);
	bitset_ncopy0_and_maybe_zero_unused_bits(self->bs_bitset, ref->bsr_bitset, ref->bsr_startbit, copy_bits);

	/* Clear out all bits that weren't copied from "other" */
	if (copy_bits < self->bs_nbits)
		bitset_nclear(self->bs_bitset, copy_bits, self->bs_nbits);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_inplaceop_bitset(Bitset *self, struct bitset_ref *__restrict ref, unsigned int op) {
	size_t copy_bits = bitset_ref_nbits(ref);
	if (copy_bits > self->bs_nbits) {
		if (op != BITSET_OP_AND) {
			/* Ensure that no bit is set that can't be represented in "self" */
			size_t oob_index;
			oob_index = bitset_nffs(ref->bsr_bitset,
			                        ref->bsr_startbit + self->bs_nbits,
			                        ref->bsr_endbit);
			if (oob_index < ref->bsr_endbit) {
				oob_index -= ref->bsr_startbit;
				ASSERT(oob_index >= self->bs_nbits);
				return bs_err_bad_index(self, oob_index);
			}
		}

		/* Trim to the size of our own bitset. */
		ref->bsr_endbit = ref->bsr_startbit + self->bs_nbits;
		copy_bits = self->bs_nbits;
	}

	ASSERT(copy_bits == bitset_ref_nbits(ref));
	ASSERT(ref->bsr_startbit <= _BITSET_WORD_BMSK);
	bitset_nbitop(self->bs_bitset, 0, ref->bsr_bitset, ref->bsr_startbit, copy_bits, op);
	if (copy_bits < self->bs_nbits) {
		if (op == BITSET_OP_AND)
			bitset_nclear(self->bs_bitset, copy_bits, self->bs_nbits);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_assign(Bitset *self, DeeObject *other) {
	int result;
	DREF Bitset *temp;
	struct bitset_ref ref;
	if (DeeObject_AsBitset(other, &ref))
		return bs_assign_bitset(self, &ref);

	/* Must create a temp bitset from "other" and then assign that one.
	 * We can't directly assign from "other" in case "other" somehow
	 * re-uses the state of "self" (e.g.: is a yield function) */
	temp = bs_init_fromseq(other, NULL);
	if unlikely(!temp)
		goto err;
	ref.bsr_bitset   = temp->bs_bitset;
	ref.bsr_startbit = 0;
	ref.bsr_endbit   = temp->bs_nbits;
	result = bs_assign_bitset(self, &ref);
	Dee_DecrefDokill(temp);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_inplaceop(DREF Bitset **p_self, DeeObject *other, unsigned int op) {
	Bitset *self = *p_self;
	int result;
	DREF Bitset *temp;
	struct bitset_ref ref;
	if (DeeObject_AsBitset(other, &ref))
		return bs_inplaceop_bitset(self, &ref, op);

	/* Must create a temp bitset from "other" and then assign that one.
	 * We can't directly assign from "other" in case "other" somehow
	 * re-uses the state of "self" (e.g.: is a yield function) */
	temp = bs_init_fromseq(other, NULL);
	if unlikely(!temp)
		goto err;
	ref.bsr_bitset   = temp->bs_bitset;
	ref.bsr_startbit = 0;
	ref.bsr_endbit   = temp->bs_nbits;
	result = bs_inplaceop_bitset(self, &ref, op);
	Dee_DecrefDokill(temp);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_inplace_and(DREF Bitset **p_self, DeeObject *other) {
	return bs_inplaceop(p_self, other, BITSET_OP_AND);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_inplace_or(DREF Bitset **p_self, DeeObject *other) {
	return bs_inplaceop(p_self, other, BITSET_OP_OR);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bs_inplace_xor(DREF Bitset **p_self, DeeObject *other) {
	return bs_inplaceop(p_self, other, BITSET_OP_XOR);
}


PRIVATE WUNUSED NONNULL((1)) DREF Bitset *DCALL
bs_inv(Bitset *__restrict self) {
	DREF Bitset *result = Bitset_Alloc(self->bs_nbits);
	if unlikely(!result)
		goto err;
	memcpy(result->bs_bitset, self->bs_bitset,
	       BITSET_SIZEOF(self->bs_nbits));
	bitset_flipall_and_zero_unused_bits(result->bs_bitset,
	                                    self->bs_nbits);
	result->bs_nbits = self->bs_nbits;
	DeeObject_Init(result, &RoBitset_Type); /* Yes: this returns a read-only bitset! */
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bs_bitop_bitset(Bitset *self, struct bitset_ref *__restrict ref, unsigned int op) {
	DREF Bitset *result;
	size_t lhs_bits = self->bs_nbits;
	size_t rhs_bits = bitset_ref_nbits(ref);
	size_t com_bits = lhs_bits < rhs_bits ? lhs_bits : rhs_bits;
	size_t res_bits = lhs_bits > rhs_bits ? lhs_bits : rhs_bits;
	result = Bitset_Calloc(res_bits);
	if unlikely(!result)
		goto err;
	ASSERT(ref->bsr_startbit <= _BITSET_WORD_BMSK);
	memcpy(result->bs_bitset, self->bs_bitset, BITSET_SIZEOF(lhs_bits));
	bitset_nbitop(result->bs_bitset, 0, ref->bsr_bitset, ref->bsr_startbit, com_bits, op);
	if (com_bits < res_bits) {
		if (op == BITSET_OP_AND) {
			/* This part was already done by the `Bitset_Calloc()' above. */
			/*bitset_nclear(result->bs_bitset, com_bits, res_bits);*/
		} else {
			/* Insert bits from the larger operand */
			if (lhs_bits < rhs_bits) {
				ASSERT(com_bits == lhs_bits);
				ASSERT(res_bits == rhs_bits);
				bitset_ncopy(result->bs_bitset, lhs_bits,
				             ref->bsr_bitset, ref->bsr_startbit + lhs_bits,
				             rhs_bits - lhs_bits);
			} else {
				ASSERT(lhs_bits > rhs_bits);
				ASSERT(com_bits == rhs_bits);
				ASSERT(res_bits == lhs_bits);
				bitset_ncopy(result->bs_bitset, rhs_bits,
				             self->bs_bitset, rhs_bits,
				             lhs_bits - rhs_bits);
			}
		}
	}
	result->bs_nbits = res_bits;
	DeeObject_Init(result, &RoBitset_Type); /* Yes: this returns a read-only bitset! */
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bs_bitop(Bitset *self, DeeObject *other, unsigned int op) {
	DREF Bitset *result, *temp;
	struct bitset_ref ref;
	if (DeeObject_AsBitset(other, &ref))
		return bs_bitop_bitset(self, &ref, op);

	/* Must create a temp bitset from "other" and then assign that one.
	 * We can't directly assign from "other" in case "other" somehow
	 * re-uses the state of "self" (e.g.: is a yield function) */
	temp = bs_init_fromseq(other, NULL);
	if unlikely(!temp)
		goto err;
	ref.bsr_bitset   = temp->bs_bitset;
	ref.bsr_startbit = 0;
	ref.bsr_endbit   = temp->bs_nbits;
	result = bs_bitop_bitset(self, &ref, op);
	Dee_DecrefDokill(temp);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bs_and(Bitset *self, DeeObject *other) {
	return bs_bitop(self, other, BITSET_OP_AND);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bs_or(Bitset *self, DeeObject *other) {
	return bs_bitop(self, other, BITSET_OP_OR);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bs_xor(Bitset *self, DeeObject *other) {
	return bs_bitop(self, other, BITSET_OP_XOR);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bs_bool(Bitset *__restrict self) {
	return bitset_anyset(self->bs_bitset, self->bs_nbits) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
bs_printrepr(Bitset *__restrict self, Dee_formatprinter_t printer, void *arg) {
	bool is_first;
	size_t bitno;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "Bitset({");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	bitset_foreach (bitno, self->bs_bitset, self->bs_nbits) {
		DO(err_temp, DeeFormat_Printf(printer, arg,
		                              "%s%" PRFuSIZ,
		                              is_first ? " " : ", ",
		                              bitno));
		is_first = false;
	}
	DO(err_temp, is_first ? DeeFormat_PRINT(printer, arg, "})")
	                      : DeeFormat_PRINT(printer, arg, " })"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
bs_foreach(Bitset *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t bitno;
	bitset_foreach (bitno, self->bs_bitset, self->bs_nbits) {
		DREF DeeObject *id;
		id = DeeInt_NewSize(bitno);
		if unlikely(!id)
			goto err;
		temp = (*proc)(arg, id);
		Dee_Decref(id);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}


PRIVATE struct type_math bs_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *))&bs_inv,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ NULL,
	/* .tp_sub         = */ NULL,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL, /* TODO */
	/* .tp_shr         = */ NULL, /* TODO */
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_xor,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ NULL,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL, /* TODO */
	/* .tp_inplace_shr = */ NULL, /* TODO */
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&bs_inplace_and,
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&bs_inplace_or,
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&bs_inplace_xor,
	/* .tp_inplace_pow = */ NULL
};

/* Compare operators with optimizations when the operand is another `Bitset' or `BitsetView' */
PRIVATE struct type_cmp bs_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&bs_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_ge,
};

PRIVATE struct type_seq bs_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bs_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bs_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&bs_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&bs_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&bs_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&bs_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&bs_setitem_index,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&bs_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&bs_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&bs_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&bs_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&bs_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&bs_setrange_index_n,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&bs_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_buffer bs_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&bs_getbuf,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE struct type_method tpconst bs_methods[] = {
	TYPE_METHOD_F("flip", &bs_flip, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Flip all bits\n"
	              "\n"

	              "(bitno:?Dint)->?DBool\n"
	              "#tIntegerOverflow{@bitno is negative or too large}"
	              "#tValueError{@bitno is greater than or equal to ?#nbits}"
	              "Flip the state of @bitno, returning its old state. This is an atomic operation\n"
	              "\n"

	              "(start:?Dint,end:?Dint)\n"
	              "#tIntegerOverflow{@start or @end is negative or too large}"
	              "Flip the state of all bits in #C{[start,end)}. "
	              /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	              /**/ "If @start is greater than @end (after clamping), the call is a no-op"),

	TYPE_METHOD_F("set", &bs_set, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Set all bits\n"
	              "\n"

	              "(bitno:?Dint)->?DBool\n"
	              "#tIntegerOverflow{@bitno is negative or too large}"
	              "#tValueError{@bitno is greater than or equal to ?#nbits}"
	              "Set @bitno, returning its old state. This is an atomic operation. "
	              /**/ "Same as ${!this.insert(bitno)}.\n"
	              "\n"

	              "(start:?Dint,end:?Dint)\n"
	              "#tIntegerOverflow{@start or @end is negative or too large}"
	              "Set all bits in #C{[start,end)}. "
	              /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	              /**/ "If @start is greater than @end (after clamping), the call is a no-op"),

	TYPE_METHOD_F("clear", &bs_clear, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Clear all bits\n"
	              "\n"

	              "(bitno:?Dint)->?DBool\n"
	              "#tIntegerOverflow{@bitno is negative or too large}"
	              "#tValueError{@bitno is greater than or equal to ?#nbits}"
	              "Clear @bitno, returning its old state. This is an atomic operation. "
	              /**/ "Same as ${this.remove(bitno)}.\n"
	              "\n"

	              "(start:?Dint,end:?Dint)\n"
	              "#tIntegerOverflow{@start or @end is negative or too large}"
	              "Clear all bits in #C{[start,end)}. "
	              /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	              /**/ "If @start is greater than @end (after clamping), the call is a no-op"),

	TYPE_KWMETHOD_F("popcount", &bs_popcount, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Return the # of 1-bits in #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("any", &bs_any, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dbool\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Returns !t if there any bits in #C{[start,end)} are on. Same as ${this.popcount(start, end) > 0}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("all", &bs_all, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dbool\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Returns !t if all bits in #C{[start,end)} are on. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ffs", &bs_ffs, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindFirstSet: find the index of the lowest 1-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ffc", &bs_ffc, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindFirstClear: find the index of the lowest 0-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("fls", &bs_fls, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindLastSet: find the index of the greatest 1-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("flc", &bs_flc, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindLastClear: find the index of the greatest 0-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("clz", &bs_clz, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountLeadingZeroes: Return number of consecutive 0-bits that exist, starting at the high end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ctz", &bs_ctz, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountTrailingZeroes: Return number of consecutive 0-bits that exist, starting at the low end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("clo", &bs_clo, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountLeadingOnes: Return number of consecutive 1-bits that exist, starting at the high end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("cto", &bs_cto, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountTrailingOnes: Return number of consecutive 1-bits that exist, starting at the low end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
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
	TYPE_GETTER_AB_F("frozen", &bs_frozen, METHOD_FNOREFESCAPE, "->?#Frozen"),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &bs_sizeof, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dint"),
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
#define bsv_class_members (bs_class_members + 1)
	TYPE_MEMBER_CONST("Iterator", &BitsetIterator_Type),
	TYPE_MEMBER_CONST("ItemType", &DeeInt_Type),
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
	                         /**/ "!t or !f to the bit's index (${this[bitno] = true}), or by using the "
	                         /**/ "?#set, ?#clear and ?#flip methods. Additionally, the state of a simple "
	                         /**/ "bit can be queried as either ${bitno in this} or ${this[bitno]}\n"
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
	                         "Same as ?#{op:copy}\n"
	                         "\n"

	                         ":=(other:?S?Dint)->\n"
	                         "#tIntegerOverflow{One of the elements of @other is negative or too large}"
	                         "#tIndexError{One of the elements of @other is greater that or equal to ?#nbits}"
	                         "Turn off all bits, except for those whose indices appear in @other\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of 1-bits (that is: the population count) of this ?.. "
	                         /**/ "Same as ${this.popcount(0, this.nbits)}\n"
	                         "\n"

	                         "move:=->\n"
	                         "Same as ?#{op:assign}\n"
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

	                         "~->?#Frozen\n"
	                         "Return a frozen ?. with the same ?#nbits as @this, but with the state "
	                         /**/ "of all bits inverted.\n"
	                         "\n"

	                         "&(other:?X4?S?Dint?.?GBitsetView?#Frozen)->?#Frozen\n"
	                         "|(other:?X4?S?Dint?.?GBitsetView?#Frozen)->?#Frozen\n"
	                         "^(other:?X4?S?Dint?.?GBitsetView?#Frozen)->?#Frozen\n"
	                         "Combine this ?. with another ?. or bitset-like object by performing "
	                         /**/ "the specified bit-wise operator.\n"
	                         "\n"

	                         "&=(other:?X4?S?Dint?.?GBitsetView?#Frozen)\n"
	                         "|=(other:?X4?S?Dint?.?GBitsetView?#Frozen)\n"
	                         "^=(other:?X4?S?Dint?.?GBitsetView?#Frozen)\n"
	                         "Inplace-combine this ?. with another ?. or bitset-like object by performing "
	                         /**/ "the specified bit-wise operator, and assigning the result back to @this.\n"
	                         "\n"

	                         "[]->?Dbool\n"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Returns !t or !f indicative of he the state of the @index'th bit\n"
	                         "\n"

	                         "[]=(index:?Dint,value:?Dbool)\n"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Atomically set the state of the @index'th bit to @valuen"
	                         "\n"

	                         "del[]->\n"
	                         "#tIntegerOverflow{@index is negative or too large}"
	                         "#tIndexError{@index is greater that or equal to ?#nbits}"
	                         "Same as ${this[index] = false}\n"
	                         "\n"

	                         "contains(index:?Dint)->?Dbool\n"
	                         "Alias for ${this[index]}\n"
	                         "\n"

	                         "[:](start:?X2?N?Dint,end:?X2?N?Dint)->?GBitsetView\n"
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
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &bs_ctor,
			/* tp_copy_ctor:   */ &bs_copy,
			/* tp_deep_ctor:   */ &bs_copy,
			/* tp_any_ctor:    */ &bs_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bs_serialize,
			/* tp_free:        */ NULL
		),
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
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&bs_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &bs_math,
	/* .tp_cmp           = */ &bs_cmp,
	/* .tp_seq           = */ &bs_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ &bs_buffer,
	/* .tp_methods       = */ bs_methods,
	/* .tp_getsets       = */ bs_getsets,
	/* .tp_members       = */ bs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bs_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ bs_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(bs_operators)
};








#define robs_nsi_getsize bs_size
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
		result = Bitset_Alloc(ref_nbits);
		if unlikely(!result)
			goto err;
		bitset_ncopy0_and_zero_unused_bits(result->bs_bitset, ref.bsr_bitset,
		                                   ref.bsr_startbit, ref_nbits);
		result->bs_nbits = ref_nbits;
		DeeObject_Init(result, &RoBitset_Type);
		return result;
	}
	return robs_init_fromseq(seq);
err:
	return NULL;
}


#define robs_inv bs_inv
#define robs_and bs_and
#define robs_or  bs_or
#define robs_xor bs_xor

#define robs_eq               bs_eq
#define robs_ne               bs_ne
#define robs_le               bs_le
#define robs_ge               bs_ge
#define robs_gr               bs_gr
#define robs_lo               bs_lo
#define robs_iter             bs_iter
#define robs_size             bs_size
#define robs_contains         bs_contains
#define robs_foreach          bs_foreach
#define robs_getitem_index    bs_getitem_index
#define robs_trygetitem_index bs_trygetitem_index

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
robs_getrange_index(Bitset *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, self->bs_nbits);
	result->bsv_owner = Dee_AsObject(self);
	Dee_Incref(self);
	result->bsv_buf.bb_base = self->bs_bitset;
	result->bsv_buf.bb_size = BITSET_SIZEOF(self->bs_nbits);
	result->bsv_startbit    = range.sr_start;
	result->bsv_endbit      = range.sr_end;
	result->bsv_bflags      = Dee_BUFFER_FREADONLY;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
robs_getrange_index_n(Bitset *self, Dee_ssize_t start) {
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	result->bsv_owner = Dee_AsObject(self);
	Dee_Incref(self);
	result->bsv_buf.bb_base = self->bs_bitset;
	result->bsv_buf.bb_size = BITSET_SIZEOF(self->bs_nbits);
	result->bsv_startbit    = DeeSeqRange_Clamp_n(start, self->bs_nbits);
	result->bsv_endbit      = self->bs_nbits;
	result->bsv_bflags      = Dee_BUFFER_FREADONLY;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

#define robs_getbuf bs_getbuf

#define robs_popcount bs_popcount
#define robs_any      bs_any
#define robs_all      bs_all
#define robs_ffs      bs_ffs
#define robs_ffc      bs_ffc
#define robs_fls      bs_fls
#define robs_flc      bs_flc
#define robs_clz      bs_clz
#define robs_ctz      bs_ctz
#define robs_clo      bs_clo
#define robs_cto      bs_cto

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
robs_bytes(Bitset *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "bytes");
	return DeeBytes_NewView(Dee_AsObject(self),
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
	DeeArg_Unpack1(err, argc, argv, "Bitset.Frozen", &seq);
	return robs_init_fromseq_or_bitset(seq);
err:
	return NULL;
}

#define robs_serialize bs_serialize
#define robs_bool      bs_bool

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
robs_printrepr(Bitset *__restrict self, Dee_formatprinter_t printer, void *arg) {
	bool is_first;
	size_t bitno;
	Dee_ssize_t temp, result;
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


PRIVATE struct type_math robs_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *))&robs_inv,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ NULL,
	/* .tp_sub         = */ NULL,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL, /* TODO */
	/* .tp_shr         = */ NULL, /* TODO */
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_xor,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ NULL,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL,
	/* .tp_inplace_shr = */ NULL,
	/* .tp_inplace_and = */ NULL,
	/* .tp_inplace_or  = */ NULL,
	/* .tp_inplace_xor = */ NULL,
	/* .tp_inplace_pow = */ NULL
};

/* Compare operators with optimizations when the operand is another `Bitset' or `BitsetView' */
PRIVATE struct type_cmp robs_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&robs_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_ge,
};

PRIVATE struct type_seq robs_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&robs_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&robs_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&robs_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&robs_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&robs_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&robs_getrange_index,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&robs_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&robs_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_buffer robs_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&robs_getbuf,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FREADONLY
};

PRIVATE struct type_method tpconst robs_methods[] = {
	TYPE_KWMETHOD_F("popcount", &robs_popcount, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Return the # of 1-bits in #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("any", &robs_any, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dbool\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Returns !t if there any bits in #C{[start,end)} are on. Same as ${this.popcount(start, end) > 0}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("all", &robs_all, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dbool\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Returns !t if all bits in #C{[start,end)} are on. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ffs", &robs_ffs, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindFirstSet: find the index of the lowest 1-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ffc", &robs_ffc, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindFirstClear: find the index of the lowest 0-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("fls", &robs_fls, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindLastSet: find the index of the greatest 1-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("flc", &robs_flc, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindLastClear: find the index of the greatest 0-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("clz", &robs_clz, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountLeadingZeroes: Return number of consecutive 0-bits that exist, starting at the high end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ctz", &robs_ctz, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountTrailingZeroes: Return number of consecutive 0-bits that exist, starting at the low end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("clo", &robs_clo, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountLeadingOnes: Return number of consecutive 1-bits that exist, starting at the high end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("cto", &robs_cto, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountTrailingOnes: Return number of consecutive 1-bits that exist, starting at the low end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_METHOD("bytes", &robs_bytes,
	            "->?DBytes\n"
	            "Returns a view for the underlying bytes of ?."),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst robs_getsets[] = {
	TYPE_GETTER_AB_F("frozen", &DeeObject_NewRef, METHOD_FCONSTCALL, "->?Dint"),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &robs_sizeof, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

#define robs_members       bs_members
#define robs_class_members bs_class_members

PRIVATE struct type_operator const robs_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
#ifdef CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_SERIALIZE, METHOD_FCONSTCALL | METHOD_FNOTHROW),
#else /* CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR */
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
#endif /* !CONFIG_EXPERIMENTAL_SERIALIZE_OPERATOR */
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
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
	                         "Returns the number of 1-bits (that is: the population count) of this ?.. "
	                         /**/ "Same as ${this.popcount(0, this.nbits)}\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if at least one bit has been turned on\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the bit indices that are turned on\n"
	                         "\n"

	                         "repr->\n"
	                         "Print the representation of @this in the form of ${Bitset.Frozen({ 0, 1, 2, ... })} "
	                         /**/ "(listing all bit indices where the associated bit is on)\n"
	                         "\n"

	                         "~->?.\n"
	                         "Return a frozen ?GBitset with the same ?#nbits as @this, but with the state "
	                         /**/ "of all bits inverted. The implementation is allowed to return a sequence "
	                         /**/ "proxy instead.\n"
	                         "\n"

	                         "&(other:?X4?S?Dint?GBitset?GBitsetView?.)->?.\n"
	                         "|(other:?X4?S?Dint?GBitset?GBitsetView?.)->?.\n"
	                         "^(other:?X4?S?Dint?GBitset?GBitsetView?.)->?.\n"
	                         "Combine this ?. with another ?GBitset or bitset-like object by performing "
	                         /**/ "the specified bit-wise operator. If @other is another ?., or ?GBitsetView "
	                         /**/ "that cannot be modified, the implementation of this function is allowed "
	                         /**/ "to return a sequence proxy instead or a new ?..\n"
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
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &robs_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ &robs_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &robs_serialize,
			/* tp_free:        */ NULL
		),
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
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&robs_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &robs_math,
	/* .tp_cmp           = */ &robs_cmp,
	/* .tp_seq           = */ &robs_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ &robs_buffer,
	/* .tp_methods       = */ robs_methods,
	/* .tp_getsets       = */ robs_getsets,
	/* .tp_members       = */ robs_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ robs_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ robs_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(robs_operators)
};






PRIVATE ATTR_COLD NONNULL((1)) int DCALL
bsv_err_bad_index(BitsetView *__restrict self, size_t bitno) {
	return DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), bitno, BitsetView_GetNBits(self));
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
bsv_err_readonly(BitsetView *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_BufferError,
	                       "The BitsetView object is not writable");
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
bsv_size(BitsetView *__restrict self) {
	return bitset_npopcount(BitsetView_GetBitset(self),
	                        self->bsv_startbit,
	                        self->bsv_endbit);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
bsv_hash(BitsetView *__restrict self) {
	size_t bitno;
	Dee_hash_t result = Dee_HASHOF_EMPTY_SEQUENCE;
	for (bitno = self->bsv_startbit;
	     bitno < self->bsv_endbit; ++bitno) {
		if (bitset_test(BitsetView_GetBitset(self), bitno)) {
			size_t index = bitno - self->bsv_startbit;
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
	}
	ASSERT(a->bsr_startbit <= _BITSET_WORD_BMSK);
	ASSERT(b->bsr_startbit <= _BITSET_WORD_BMSK);
	return bitset_ncmpeq(a->bsr_bitset, a->bsr_startbit,
	                     b->bsr_bitset, b->bsr_startbit,
	                     a_nbits);
nope:
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
bitset_ref_cmp_le(struct bitset_ref const *__restrict a,
                  struct bitset_ref const *__restrict b) {
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
	ASSERT(a->bsr_startbit <= _BITSET_WORD_BMSK);
	ASSERT(b->bsr_startbit <= _BITSET_WORD_BMSK);
	return bitset_ncmple(a->bsr_bitset, a->bsr_startbit,
	                     b->bsr_bitset, b->bsr_startbit,
	                     a_nbits);
nope:
	return false;
}

#define bitset_ref_cmp_ne(a, b) (!bitset_ref_cmp_eq(a, b))
#define bitset_ref_cmp_ge(a, b) bitset_ref_cmp_le(b, a)
#define bitset_ref_cmp_lo(a, b) (!bitset_ref_cmp_ge(a, b))
#define bitset_ref_cmp_gr(a, b) (!bitset_ref_cmp_le(a, b))


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_eq(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_eq)(Dee_AsObject(self), other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_eq(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_ne(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_eq)(Dee_AsObject(self), other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_ne(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_le(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_le)(Dee_AsObject(self), other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_le(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_ge(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_ge)(Dee_AsObject(self), other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_ge(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_gr(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_gr)(Dee_AsObject(self), other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_gr(&a, &b));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bsv_lo(BitsetView *self, DeeObject *other) {
	struct bitset_ref a, b;
	if (!DeeObject_AsBitset(other, &b))
		return (*DeeSeq_Type.tp_cmp->tp_lo)(Dee_AsObject(self), other);
	bitset_ref_fromview(&a, self);
	return_bool(bitset_ref_cmp_lo(&a, &b));
}


PRIVATE WUNUSED NONNULL((1)) DREF BitsetIterator *DCALL
bsv_iter(BitsetView *__restrict self) {
	DREF BitsetIterator *result;
	result = DeeObject_MALLOC(BitsetIterator);
	if unlikely(!result)
		goto err;
	result->bsi_owner = Dee_AsObject(self);
	Dee_Incref(self);
	result->bsi_bitset   = BitsetView_GetBitset(self);
	result->bsi_startbit = self->bsv_startbit;
	result->bsi_endbit   = self->bsv_endbit;
	result->bsi_bitno    = self->bsv_startbit;
	DeeObject_Init(result, &BitsetIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_contains(BitsetView *self, DeeObject *key) {
	size_t bitno;
	if (DeeObject_AsSize(key, &bitno))
		goto err;
	return_bool(bitno < BitsetView_GetNBits(self) &&
	            bitset_test(BitsetView_GetBitset(self),
	                        bitno + self->bsv_startbit));
err:
	if (DeeError_Catch(&DeeError_IntegerOverflow))
		return_false;
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_getitem_index(BitsetView *self, size_t bitno) {
	if unlikely(bitno >= BitsetView_GetNBits(self))
		goto err_too_large;
	return_bool(bitset_test(BitsetView_GetBitset(self),
	                        bitno + self->bsv_startbit));
err_too_large:
	bsv_err_bad_index(self, bitno);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_trygetitem_index(BitsetView *self, size_t bitno) {
	if unlikely(bitno >= BitsetView_GetNBits(self))
		return ITER_DONE;
	return_bool(bitset_test(BitsetView_GetBitset(self),
	                        bitno + self->bsv_startbit));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_delitem_index(BitsetView *self, size_t bitno) {
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
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_setitem_index(BitsetView *self, size_t bitno, DeeObject *value) {
	int temp;
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
bsv_getrange_index(BitsetView *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	DeeSeqRange_Clamp(&range, start, end, BitsetView_GetNBits(self));
	result->bsv_owner = self->bsv_owner;
	Dee_Incref(result->bsv_owner);
	result->bsv_buf.bb_base = self->bsv_buf.bb_base;
	result->bsv_buf.bb_size = self->bsv_buf.bb_size;
	result->bsv_startbit = range.sr_start + self->bsv_startbit;
	result->bsv_endbit   = range.sr_end + self->bsv_startbit;
	result->bsv_bflags   = self->bsv_bflags;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF BitsetView *DCALL
bsv_getrange_index_n(BitsetView *self, Dee_ssize_t start) {
	DREF BitsetView *result;
	result = DeeObject_MALLOC(BitsetView);
	if unlikely(!result)
		goto err;
	result->bsv_owner = self->bsv_owner;
	Dee_Incref(result->bsv_owner);
	result->bsv_buf.bb_base = self->bsv_buf.bb_base;
	result->bsv_buf.bb_size = self->bsv_buf.bb_size;
	result->bsv_startbit = DeeSeqRange_Clamp_n(start, BitsetView_GetNBits(self)) + self->bsv_startbit;
	result->bsv_endbit   = self->bsv_endbit;
	result->bsv_bflags   = self->bsv_bflags;
	DeeObject_Init(result, &BitsetView_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_delrange_index(BitsetView *self, Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	DeeSeqRange_Clamp(&range, start, end, BitsetView_GetNBits(self));
	bitset_nclear(BitsetView_GetBitset(self),
	              self->bsv_startbit + range.sr_start,
	              self->bsv_startbit + range.sr_end);
	return 0;
err_readonly:
	return bsv_err_readonly(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_delrange_index_n(BitsetView *self, Dee_ssize_t start) {
	size_t used_start;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	used_start = DeeSeqRange_Clamp_n(start, BitsetView_GetNBits(self));
	bitset_nclear(BitsetView_GetBitset(self),
	              self->bsv_startbit + used_start,
	              self->bsv_endbit);
	return 0;
err_readonly:
	return bsv_err_readonly(self);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
bsv_setrange_index(BitsetView *self, Dee_ssize_t start,
                   Dee_ssize_t end, DeeObject *value) {
	struct Dee_seq_range range;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	DeeSeqRange_Clamp(&range, start, end, BitsetView_GetNBits(self));
	if (DeeBool_Check(value)) {
		if (DeeBool_IsTrue(value)) {
			bitset_nset(BitsetView_GetBitset(self),
			            self->bsv_startbit + range.sr_start,
			            self->bsv_startbit + range.sr_end);
		} else {
			bitset_nclear(BitsetView_GetBitset(self),
			              self->bsv_startbit + range.sr_start,
			              self->bsv_startbit + range.sr_end);
		}
	} else if (DeeNone_Check(value)) {
		bitset_nclear(BitsetView_GetBitset(self),
		              self->bsv_startbit + range.sr_start,
		              self->bsv_startbit + range.sr_end);
	} else {
		int result;
		DREF Bitset *value_bitset;
		struct bitset_ref dst, src;
		dst.bsr_bitset   = BitsetView_GetBitset(self);
		dst.bsr_startbit = self->bsv_startbit + range.sr_start;
		dst.bsr_endbit   = self->bsv_startbit + range.sr_end;
		bitset_ref_fix(&dst);
		if (DeeObject_AsBitset(value, &src))
			return bitset_ref_assign(&dst, &src, Dee_AsObject(self));
		value_bitset = bs_init_fromseq(value, NULL);
		if unlikely(!value_bitset)
			goto err;
		src.bsr_bitset   = value_bitset->bs_bitset;
		src.bsr_startbit = 0;
		src.bsr_endbit   = value_bitset->bs_nbits;
		result = bitset_ref_assign(&dst, &src, Dee_AsObject(self));
		Dee_DecrefDokill(value_bitset);
		return result;
	}
	return 0;
err_readonly:
	return bsv_err_readonly(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
bsv_setrange_index_n(BitsetView *self, Dee_ssize_t start, DeeObject *value) {
	return bsv_setrange_index(self, start, SSIZE_MAX, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_getbuf(BitsetView *__restrict self,
           DeeBuffer *__restrict info,
           unsigned int flags) {
	struct bitset_ref ref;
	if ((flags & Dee_BUFFER_FWRITABLE) && !(self->bsv_bflags & Dee_BUFFER_FWRITABLE))
		goto err_readonly;
	bitset_ref_fromview(&ref, self);
	if (ref.bsr_startbit || (ref.bsr_endbit & _BITSET_WORD_BMSK)) {
		/* Bitset view isn't byte-aligned :( */
		return DeeError_Throwf(&DeeError_BufferError,
		                       "Cannot access bytes of BitsetView: underlying memory isn't byte-aligned "
		                       "(unaligned bits are: %" PRFuSIZ " leading, %" PRFuSIZ " trailing)",
		                       (size_t)(ref.bsr_startbit),
		                       (size_t)(ref.bsr_endbit & _BITSET_WORD_BMSK));
	}
	info->bb_base = ref.bsr_bitset;
	info->bb_size = ref.bsr_endbit >> _BITSET_WORD_SHFT;
	return 0;
err_readonly:
	return bsv_err_readonly(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_flip(BitsetView *__restrict self, size_t argc, DeeObject *const *argv) {
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	switch (argc) {
	case 0:
		bitset_nflip(BitsetView_GetBitset(self),
		             self->bsv_startbit,
		             self->bsv_endbit);
		break;

	case 1: {
		size_t index;
		bool old_state;
		if (DeeObject_AsSize(argv[0], &index))
			goto err_maybe_overflow;
		if unlikely(index >= BitsetView_GetNBits(self)) {
			bsv_err_bad_index(self, index);
			goto err;
		}
		old_state = bitset_atomic_fetchflip(BitsetView_GetBitset(self),
		                                    self->bsv_startbit + index);
		return_bool(old_state);
	}	break;

	case 2: {
		size_t start, end, nbits;
		if (DeeObject_AsSize(argv[0], &start))
			goto err;
		if (DeeObject_AsSizeM1(argv[1], &end))
			goto err;
		nbits = BitsetView_GetNBits(self);
		if (end > nbits)
			end = nbits;
		if (start > end)
			start = end;
		bitset_nflip(BitsetView_GetBitset(self),
		             self->bsv_startbit + start,
		             self->bsv_startbit + end);
	}	break;

	default:
		DeeArg_BadArgcEx("flip", argc, 0, 2);
		goto err;
	}
	return_none;
err_readonly:
	bsv_err_readonly(self);
err:
	return NULL;
err_maybe_overflow:
	DeeRT_ErrIndexOverflow(self);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_set(BitsetView *__restrict self, size_t argc, DeeObject *const *argv) {
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	switch (argc) {
	case 0:
		bitset_nset(BitsetView_GetBitset(self),
		            self->bsv_startbit, self->bsv_endbit);
		break;

	case 1: {
		size_t index;
		bool old_state;
		if (DeeObject_AsSize(argv[0], &index))
			goto err_maybe_overflow;
		if unlikely(index >= BitsetView_GetNBits(self)) {
			bsv_err_bad_index(self, index);
			goto err;
		}
		old_state = bitset_atomic_fetchset(BitsetView_GetBitset(self),
		                                   self->bsv_startbit + index);
		return_bool(old_state);
	}	break;

	case 2: {
		size_t start, end, nbits;
		if (DeeObject_AsSize(argv[0], &start))
			goto err;
		if (DeeObject_AsSizeM1(argv[1], &end))
			goto err;
		nbits = BitsetView_GetNBits(self);
		if (end > nbits)
			end = nbits;
		if (start > end)
			start = end;
		bitset_nset(BitsetView_GetBitset(self),
		            self->bsv_startbit + start,
		            self->bsv_startbit + end);
	}	break;

	default:
		DeeArg_BadArgcEx("set", argc, 0, 2);
		goto err;
	}
	return_none;
err_readonly:
	bsv_err_readonly(self);
err:
	return NULL;
err_maybe_overflow:
	DeeRT_ErrIndexOverflow(self);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_clear(BitsetView *__restrict self, size_t argc, DeeObject *const *argv) {
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	switch (argc) {
	case 0:
		bitset_nclear(BitsetView_GetBitset(self),
		              self->bsv_startbit, self->bsv_endbit);
		break;

	case 1: {
		size_t index;
		bool old_state;
		if (DeeObject_AsSize(argv[0], &index))
			goto err_maybe_overflow;
		if unlikely(index >= BitsetView_GetNBits(self)) {
			bsv_err_bad_index(self, index);
			goto err;
		}
		old_state = bitset_atomic_fetchclear(BitsetView_GetBitset(self),
		                                     self->bsv_startbit + index);
		return_bool(old_state);
	}	break;

	case 2: {
		size_t start, end, nbits;
		if (DeeObject_AsSize(argv[0], &start))
			goto err;
		if (DeeObject_AsSizeM1(argv[1], &end))
			goto err;
		nbits = BitsetView_GetNBits(self);
		if (end > nbits)
			end = nbits;
		if (start > end)
			start = end;
		bitset_nclear(BitsetView_GetBitset(self),
		              self->bsv_startbit + start,
		              self->bsv_startbit + end);
	}	break;

	default:
		DeeArg_BadArgcEx("clear", argc, 0, 2);
		goto err;
	}
	return_none;
err_readonly:
	bsv_err_readonly(self);
err:
	return NULL;
err_maybe_overflow:
	DeeRT_ErrIndexOverflow(self);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_popcount(BitsetView *__restrict self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":popcount",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	result = bitset_npopcount(BitsetView_GetBitset(self),
	                          self->bsv_startbit + start,
	                          self->bsv_startbit + end);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_any(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":any",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	return_bool(bitset_nanyset(BitsetView_GetBitset(self),
	                           self->bsv_startbit + start,
	                           self->bsv_startbit + end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_all(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":all",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	return_bool(bitset_nallset(BitsetView_GetBitset(self),
	                           self->bsv_startbit + start,
	                           self->bsv_startbit + end));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_ffs(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":ffs",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	start += self->bsv_startbit;
	end += self->bsv_startbit;
	result = bitset_nffs(BitsetView_GetBitset(self), start, end);
	if (result >= end)
		return_none;
	return DeeInt_NewSize(result - self->bsv_startbit);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_ffc(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":ffc",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	start += self->bsv_startbit;
	end += self->bsv_startbit;
	result = bitset_nffc(BitsetView_GetBitset(self), start, end);
	if (result >= end)
		return_none;
	return DeeInt_NewSize(result - self->bsv_startbit);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_fls(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":fls",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	start += self->bsv_startbit;
	end += self->bsv_startbit;
	result = bitset_nfls(BitsetView_GetBitset(self), start, end);
	if (result >= end)
		return_none;
	return DeeInt_NewSize(result - self->bsv_startbit);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_flc(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":flc",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	start += self->bsv_startbit;
	end += self->bsv_startbit;
	result = bitset_nflc(BitsetView_GetBitset(self), start, end);
	if (result >= end)
		return_none;
	return DeeInt_NewSize(result - self->bsv_startbit);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_clz(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":clz",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	start += self->bsv_startbit;
	end += self->bsv_startbit;
	result = bitset_nfls(BitsetView_GetBitset(self), start, end);
	if (result >= end)
		result = (start - 1);
	return DeeInt_NewSize((end - 1) - result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_ctz(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":ctz",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	start += self->bsv_startbit;
	end += self->bsv_startbit;
	result = bitset_nffs(BitsetView_GetBitset(self), start, end);
	if (result > end)
		result = end;
	return DeeInt_NewSize(result - start);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_clo(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":clo",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	start += self->bsv_startbit;
	end += self->bsv_startbit;
	result = bitset_nflc(BitsetView_GetBitset(self), start, end);
	if (result >= end)
		result = (start - 1);
	return DeeInt_NewSize((end - 1) - result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_cto(BitsetView *__restrict self, size_t argc,
        DeeObject *const *argv, DeeObject *kw) {
	size_t result;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist_start_end,
	                    "|" UNPuSIZ UNPxSIZ ":cto",
	                    &start, &end))
		goto err;
	if (end > BitsetView_GetNBits(self))
		end = BitsetView_GetNBits(self);
	if (start > end)
		start = end;
	start += self->bsv_startbit;
	end += self->bsv_startbit;
	result = bitset_nffc(BitsetView_GetBitset(self), start, end);
	if (result > end)
		result = end;
	return DeeInt_NewSize(result - start);
err:
	return NULL;
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
	                                    self->bsv_startbit + bitno));
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
	                                     self->bsv_startbit + bitno));
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
	DeeArg_Unpack0(err, argc, argv, "bytes");
	bitset_ref_fromview(&ref, self);
	return DeeBytes_NewView(self->bsv_owner, ref.bsr_bitset,
	                        (ref.bsr_endbit + _BITSET_WORD_BMSK) >> _BITSET_WORD_SHFT,
	                        self->bsv_bflags);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsv_frozen(BitsetView *__restrict self) {
	DREF Bitset *result;
	struct bitset_ref ref;
	size_t ref_nbits;
	if (!(self->bsv_bflags & Dee_BUFFER_FWRITABLE)) {
		DeeTypeObject *tp_owner = Dee_TYPE(self->bsv_owner);
		while (tp_owner && !tp_owner->tp_buffer)
			tp_owner = DeeType_Base(tp_owner);
		if (tp_owner && tp_owner->tp_buffer &&
		    (tp_owner->tp_buffer->tp_buffer_flags & Dee_BUFFER_TYPE_FREADONLY)) {
			/* Underlying buffer is always read-only -> can re-return "self" */
			return_reference_(Dee_AsObject(self));
		}
	}

	/* Must create a new frozen bitset from "self". */
	bitset_ref_fromview(&ref, self);
	ref_nbits = bitset_ref_nbits(&ref);
	result = Bitset_Alloc(ref_nbits);
	if unlikely(!result)
		goto err;
	bitset_ncopy0_and_zero_unused_bits(result->bs_bitset, ref.bsr_bitset,
	                                   ref.bsr_startbit, ref_nbits);
	result->bs_nbits = ref_nbits;
	DeeObject_Init(result, &RoBitset_Type);
	return Dee_AsObject(result);
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
	self->bsv_owner       = DeeBytes_NewEmpty();
	self->bsv_buf.bb_base = DeeBytes_DATA(self->bsv_owner);
	self->bsv_buf.bb_size = 0;
	self->bsv_startbit    = 0;
	self->bsv_endbit      = 0;
	self->bsv_bflags      = Dee_BUFFER_FREADONLY;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_copy(BitsetView *__restrict self,
         BitsetView *__restrict other) {
	self->bsv_owner = other->bsv_owner;
	Dee_Incref(self->bsv_owner);
	self->bsv_buf.bb_base = other->bsv_buf.bb_base;
	self->bsv_buf.bb_size = other->bsv_buf.bb_size;
	self->bsv_startbit = other->bsv_startbit;
	self->bsv_endbit   = other->bsv_endbit;
	self->bsv_bflags   = other->bsv_bflags;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_serialize(BitsetView *__restrict self,
              DeeSerial *__restrict writer,
              Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(BitsetView, field))
	BitsetView *out = DeeSerial_Addr2Mem(writer, addr, BitsetView);
	out->bsv_startbit = self->bsv_startbit;
	out->bsv_endbit   = self->bsv_endbit;
	out->bsv_bflags   = self->bsv_bflags;
	out->bsv_buf.bb_size = self->bsv_buf.bb_size;
	if (DeeSerial_PutObject(writer, ADDROF(bsv_owner), self->bsv_owner))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(bsv_buf.bb_base), self->bsv_buf.bb_base);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_init(BitsetView *__restrict self, size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	unsigned int flags = Dee_BUFFER_FREADONLY;
	size_t start = 0, end = (size_t)-1, nbits;
	if (argc >= 2) {
		ob = argv[0];
		if (DeeString_Check(argv[1])) {
			char const *str = DeeString_STR(argv[1]);
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
				if (DeeObject_AsSSize(argv[2], (Dee_ssize_t *)&start))
					goto err;
				if (argc >= 4) {
					if unlikely(argc > 4)
						goto err_args;
					if (DeeObject_AsSSize(argv[3], (Dee_ssize_t *)&end))
						goto err;
				}
			}
		} else {
			if (DeeObject_AsSSize(argv[1], (Dee_ssize_t *)&start))
				goto err;
			if (argc >= 3) {
				if unlikely(argc > 3)
					goto err_args;
				if (DeeObject_AsSSize(argv[2], (Dee_ssize_t *)&end))
					goto err;
			}
		}
	} else {
		if (argc != 1)
			goto err_args;
		ob = argv[0];
	}

	/* Special casing for when "ob" is bitset-like.
	 *
	 * This is needed so we're able to load other bitset views,
	 * even when those views contain unaligned bits (since the
	 * normal `DeeObject_GetBuf()' interface throws an error if
	 * there are unaligned bits) */
	if (Bitset_Check(ob)) {
		Bitset *o;
handle_bitset_ob:
		o = (Bitset *)ob;
		self->bsv_buf.bb_base = o->bs_bitset;
		self->bsv_buf.bb_size = BITSET_SIZEOF(o->bs_nbits);
		nbits = o->bs_nbits;
	} else if (RoBitset_Check(ob)) {
		if (flags & Dee_BUFFER_FWRITABLE)
			goto err_readonly;
		goto handle_bitset_ob;
	} else if (BitsetView_Check(ob)) {
		BitsetView *o = (BitsetView *)ob;
		self->bsv_buf.bb_base = o->bsv_buf.bb_base;
		self->bsv_buf.bb_size = o->bsv_buf.bb_size;
		nbits = BitsetView_GetNBits(o);
		start += o->bsv_startbit;
		end += o->bsv_startbit;
		ob = o->bsv_owner; /* Inline owner */
	} else {
		/* Fallback: load byte-aligned buffer from "ob" */
		if (DeeObject_GetBuf(ob, &self->bsv_buf, flags))
			goto err;
		nbits = self->bsv_buf.bb_size << _BITSET_WORD_SHFT;
	}

	if (end > nbits)
		end = nbits;
	if (start > end)
		start = end;
	self->bsv_owner = ob;
	Dee_Incref(ob);
	self->bsv_startbit = start;
	self->bsv_endbit   = end;
	self->bsv_bflags   = flags;
	return 0;
err_readonly:
	DeeError_Throwf(&DeeError_BufferError,
	                "Cannot write to read-only buffer of type %k",
	                Dee_TYPE(ob));
	goto err;
err_args:
	DeeArg_BadArgcEx("BitsetView", argc, 1, 4);
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
	DeeBuffer_Fini(&self->bsv_buf);
	Dee_Decref(self->bsv_owner);
}

PRIVATE NONNULL((1)) void DCALL
bsv_visit(BitsetView *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->bsv_owner);
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
		return bitset_ref_assign(&dst, &src, Dee_AsObject(self));
	value_bitset = bs_init_fromseq(value, NULL);
	if unlikely(!value_bitset)
		goto err;
	src.bsr_bitset   = value_bitset->bs_bitset;
	src.bsr_startbit = 0;
	src.bsr_endbit   = value_bitset->bs_nbits;
	result = bitset_ref_assign(&dst, &src, Dee_AsObject(self));
	Dee_DecrefDokill(value_bitset);
	return result;
err_readonly:
	return bsv_err_readonly(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_inplaceop_bitset(BitsetView *self, struct bitset_ref *__restrict ref, unsigned int op) {
	size_t self_bits = BitsetView_GetNBits(self);
	size_t copy_bits = bitset_ref_nbits(ref);
	if (copy_bits > self_bits) {
		if (op != BITSET_OP_AND) {
			/* Ensure that no bit is set that can't be represented in "self" */
			size_t oob_index;
			oob_index = bitset_nffs(ref->bsr_bitset,
			                        ref->bsr_startbit + self_bits,
			                        ref->bsr_endbit);
			if (oob_index < ref->bsr_endbit) {
				oob_index -= ref->bsr_startbit;
				ASSERT(oob_index >= self_bits);
				return bsv_err_bad_index(self, oob_index);
			}
		}

		/* Trim to the size of our own bitset. */
		ref->bsr_endbit = ref->bsr_startbit + self_bits;
		copy_bits = self_bits;
	}

	ASSERT(copy_bits == bitset_ref_nbits(ref));
	ASSERT(ref->bsr_startbit <= _BITSET_WORD_BMSK);
	bitset_nbitop(BitsetView_GetBitset(self), self->bsv_startbit,
	              ref->bsr_bitset, ref->bsr_startbit, copy_bits, op);
	if (copy_bits < self_bits) {
		if (op == BITSET_OP_AND)
			bitset_nclear(BitsetView_GetBitset(self), copy_bits, self_bits);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_inplaceop(DREF BitsetView **p_self, DeeObject *other, unsigned int op) {
	int result;
	BitsetView *self = *p_self;
	DREF Bitset *temp;
	struct bitset_ref ref;
	if unlikely(!BitsetView_IsWritable(self))
		goto err_readonly;
	if (DeeObject_AsBitset(other, &ref))
		return bsv_inplaceop_bitset(self, &ref, op);

	/* Must create a temp bitset from "other" and then assign that one.
	 * We can't directly assign from "other" in case "other" somehow
	 * re-uses the state of "self" (e.g.: is a yield function) */
	temp = bs_init_fromseq(other, NULL);
	if unlikely(!temp)
		goto err;
	ref.bsr_bitset   = temp->bs_bitset;
	ref.bsr_startbit = 0;
	ref.bsr_endbit   = temp->bs_nbits;
	result = bsv_inplaceop_bitset(self, &ref, op);
	Dee_DecrefDokill(temp);
	return result;
err_readonly:
	return bsv_err_readonly(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_inplace_or(DREF BitsetView **p_self, DeeObject *other) {
	return bsv_inplaceop(p_self, other, BITSET_OP_OR);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_inplace_and(DREF BitsetView **p_self, DeeObject *other) {
	return bsv_inplaceop(p_self, other, BITSET_OP_AND);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsv_inplace_xor(DREF BitsetView **p_self, DeeObject *other) {
	return bsv_inplaceop(p_self, other, BITSET_OP_XOR);
}

PRIVATE WUNUSED NONNULL((1)) DREF Bitset *DCALL
bsv_inv(BitsetView *__restrict self) {
	size_t self_bits = BitsetView_GetNBits(self);
	DREF Bitset *result = Bitset_Alloc(self_bits);
	if unlikely(!result)
		goto err;
	bitset_ncopy0_and_maybe_zero_unused_bits(result->bs_bitset,
	                                         BitsetView_GetBitset(self),
	                                         self->bsv_startbit, self_bits);
	bitset_flipall_and_zero_unused_bits(result->bs_bitset, self_bits);
	result->bs_nbits = self_bits;
	DeeObject_Init(result, &RoBitset_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bitset_ref_bitop(struct bitset_ref const *__restrict lhs,
                 struct bitset_ref const *__restrict rhs,
                 unsigned int op) {
	DREF Bitset *result;
	size_t lhs_bits = bitset_ref_nbits(lhs);
	size_t rhs_bits = bitset_ref_nbits(rhs);
	size_t com_bits = lhs_bits < rhs_bits ? lhs_bits : rhs_bits;
	size_t res_bits = lhs_bits > rhs_bits ? lhs_bits : rhs_bits;
	result = Bitset_Calloc(res_bits);
	if unlikely(!result)
		goto err;
	ASSERT(rhs->bsr_startbit <= _BITSET_WORD_BMSK);
	bitset_ncopy0_and_zero_unused_bits(result->bs_bitset, lhs->bsr_bitset,
	                                   lhs->bsr_startbit, lhs_bits);
	bitset_nbitop(result->bs_bitset, 0, rhs->bsr_bitset,
	              rhs->bsr_startbit, com_bits, op);
	if (com_bits < res_bits) {
		if (op == BITSET_OP_AND) {
			/* This part was already done by the `Bitset_Calloc()' above. */
			/*bitset_nclear(result->bs_bitset, com_bits, res_bits);*/
		} else {
			/* Insert bits from the larger operand */
			if (lhs_bits < rhs_bits) {
				ASSERT(com_bits == lhs_bits);
				ASSERT(res_bits == rhs_bits);
				bitset_ncopy(result->bs_bitset, lhs_bits,
				             rhs->bsr_bitset, rhs->bsr_startbit + lhs_bits,
				             rhs_bits - lhs_bits);
			} else {
				ASSERT(lhs_bits > rhs_bits);
				ASSERT(com_bits == rhs_bits);
				ASSERT(res_bits == lhs_bits);
				bitset_ncopy(result->bs_bitset, rhs_bits,
				             lhs->bsr_bitset, lhs->bsr_startbit + rhs_bits,
				             lhs_bits - rhs_bits);
			}
		}
	}
	result->bs_nbits = res_bits;
	DeeObject_Init(result, &RoBitset_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bsv_bitop_bitset(BitsetView *self, struct bitset_ref *__restrict ref,
                 unsigned int op) {
	struct bitset_ref lhs;
	bitset_ref_fromview(&lhs, self);
	return bitset_ref_bitop(&lhs, ref, op);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bsv_bitop(BitsetView *self, DeeObject *other, unsigned int op) {
	DREF Bitset *result, *temp;
	struct bitset_ref ref;
	if (DeeObject_AsBitset(other, &ref))
		return bsv_bitop_bitset(self, &ref, op);

	/* Must create a temp bitset from "other" and then assign that one.
	 * We can't directly assign from "other" in case "other" somehow
	 * re-uses the state of "self" (e.g.: is a yield function) */
	temp = bs_init_fromseq(other, NULL);
	if unlikely(!temp)
		goto err;
	ref.bsr_bitset   = temp->bs_bitset;
	ref.bsr_startbit = 0;
	ref.bsr_endbit   = temp->bs_nbits;
	result = bsv_bitop_bitset(self, &ref, op);
	Dee_DecrefDokill(temp);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bsv_and(BitsetView *self, DeeObject *other) {
	return bsv_bitop(self, other, BITSET_OP_AND);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bsv_or(BitsetView *self, DeeObject *other) {
	return bsv_bitop(self, other, BITSET_OP_OR);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Bitset *DCALL
bsv_xor(BitsetView *self, DeeObject *other) {
	return bsv_bitop(self, other, BITSET_OP_XOR);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsv_bool(BitsetView *self) {
	return bitset_nanyset(BitsetView_GetBitset(self), self->bsv_startbit, self->bsv_endbit) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
bsv_printrepr(BitsetView *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	DeeObject *owner = self->bsv_owner;
	result = DeeFormat_Printf(printer, arg, "BitsetView(%r", owner);
	if unlikely(result < 0)
		goto done;
	if (self->bsv_bflags & Dee_BUFFER_FWRITABLE)
		DO(err, DeeFormat_PRINT(printer, arg, ", \"w\""));
	if (self->bsv_startbit != 0) {
do_print_start_and_end:
		DO(err, DeeFormat_Printf(printer, arg, ", %" PRFuSIZ ", %" PRFuSIZ,
		                         self->bsv_startbit, self->bsv_endbit));
	} else {
		size_t max_endbit = self->bsv_buf.bb_size << _BITSET_WORD_SHFT;
		if (Bitset_Check(owner))
			max_endbit = ((Bitset *)owner)->bs_nbits;
		if (self->bsv_endbit != max_endbit)
			goto do_print_start_and_end;
	}
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
bsv_foreach(BitsetView *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t bitno;
	bitset_nforeach (bitno, BitsetView_GetBitset(self),
	                 self->bsv_startbit, self->bsv_endbit) {
		DREF DeeObject *id;
		id = DeeInt_NewSize(bitno - self->bsv_startbit);
		if unlikely(!id)
			goto err;
		temp = (*proc)(arg, id);
		Dee_Decref(id);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE struct type_math bsv_math = {
	/* .tp_int32       = */ NULL,
	/* .tp_int64       = */ NULL,
	/* .tp_double      = */ NULL,
	/* .tp_int         = */ NULL,
	/* .tp_inv         = */ (DREF DeeObject *(DCALL *)(DeeObject *))&bsv_inv,
	/* .tp_pos         = */ NULL,
	/* .tp_neg         = */ NULL,
	/* .tp_add         = */ NULL,
	/* .tp_sub         = */ NULL,
	/* .tp_mul         = */ NULL,
	/* .tp_div         = */ NULL,
	/* .tp_mod         = */ NULL,
	/* .tp_shl         = */ NULL, /* TODO */
	/* .tp_shr         = */ NULL, /* TODO */
	/* .tp_and         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_and,
	/* .tp_or          = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_or,
	/* .tp_xor         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_xor,
	/* .tp_pow         = */ NULL,
	/* .tp_inc         = */ NULL,
	/* .tp_dec         = */ NULL,
	/* .tp_inplace_add = */ NULL,
	/* .tp_inplace_sub = */ NULL,
	/* .tp_inplace_mul = */ NULL,
	/* .tp_inplace_div = */ NULL,
	/* .tp_inplace_mod = */ NULL,
	/* .tp_inplace_shl = */ NULL, /* TODO */
	/* .tp_inplace_shr = */ NULL, /* TODO */
	/* .tp_inplace_and = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&bsv_inplace_and,
	/* .tp_inplace_or  = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&bsv_inplace_or,
	/* .tp_inplace_xor = */ (int (DCALL *)(DeeObject **__restrict, DeeObject *))&bsv_inplace_xor,
	/* .tp_inplace_pow = */ NULL
};

/* Compare operators with optimizations when the operand is another `Bitset' or `BitsetView' */
PRIVATE struct type_cmp bsv_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&bsv_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_ge,
};

PRIVATE struct type_seq bsv_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsv_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&bsv_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&bsv_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&bsv_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&bsv_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&bsv_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&bsv_setitem_index,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&bsv_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&bsv_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&bsv_setrange_index,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&bsv_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&bsv_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&bsv_setrange_index_n,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&bsv_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_buffer bsv_buffer = {
	/* .tp_getbuf       = */ (int (DCALL *)(DeeObject *__restrict, DeeBuffer *__restrict, unsigned int))&bsv_getbuf,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};

PRIVATE struct type_method tpconst bsv_methods[] = {
	TYPE_METHOD_F("flip", &bsv_flip, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Flip all bits\n"
	              "\n"

	              "(bitno:?Dint)->?DBool\n"
	              "#tIntegerOverflow{@bitno is negative or too large}"
	              "#tValueError{@bitno is greater than or equal to ?#nbits}"
	              "Flip the state of @bitno, returning its old state. This is an atomic operation\n"
	              "\n"

	              "(start:?Dint,end:?Dint)\n"
	              "#tIntegerOverflow{@start or @end is negative or too large}"
	              "Flip the state of all bits in #C{[start,end)}. "
	              /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	              /**/ "If @start is greater than @end (after clamping), the call is a no-op"),

	TYPE_METHOD_F("set", &bsv_set, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Set all bits\n"
	              "\n"

	              "(bitno:?Dint)->?DBool\n"
	              "#tIntegerOverflow{@bitno is negative or too large}"
	              "#tValueError{@bitno is greater than or equal to ?#nbits}"
	              "Set @bitno, returning its old state. This is an atomic operation. "
	              /**/ "Same as ${!this.insert(bitno)}.\n"
	              "\n"

	              "(start:?Dint,end:?Dint)\n"
	              "#tIntegerOverflow{@start or @end is negative or too large}"
	              "Set all bits in #C{[start,end)}. "
	              /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	              /**/ "If @start is greater than @end (after clamping), the call is a no-op"),

	TYPE_METHOD_F("clear", &bsv_clear, METHOD_FNOREFESCAPE,
	              "()\n"
	              "Clear all bits\n"
	              "\n"

	              "(bitno:?Dint)->?DBool\n"
	              "#tIntegerOverflow{@bitno is negative or too large}"
	              "#tValueError{@bitno is greater than or equal to ?#nbits}"
	              "Clear @bitno, returning its old state. This is an atomic operation. "
	              /**/ "Same as ${this.remove(bitno)}.\n"
	              "\n"

	              "(start:?Dint,end:?Dint)\n"
	              "#tIntegerOverflow{@start or @end is negative or too large}"
	              "Clear all bits in #C{[start,end)}. "
	              /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	              /**/ "If @start is greater than @end (after clamping), the call is a no-op"),

	TYPE_KWMETHOD_F("popcount", &bsv_popcount, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Return the # of 1-bits in #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("any", &bsv_any, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dbool\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Returns !t if there any bits in #C{[start,end)} are on. Same as ${this.popcount(start, end) > 0}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("all", &bsv_all, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dbool\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "Returns !t if all bits in #C{[start,end)} are on. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ffs", &bsv_ffs, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindFirstSet: find the index of the lowest 1-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ffc", &bsv_ffc, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindFirstClear: find the index of the lowest 0-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("fls", &bsv_fls, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindLastSet: find the index of the greatest 1-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("flc", &bsv_flc, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?X2?Dint?N\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "FindLastClear: find the index of the greatest 0-bit that within #C{[start,end)}. "
	                /**/ "If no such bit exists, return ?N instead. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("clz", &bsv_clz, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountLeadingZeroes: Return number of consecutive 0-bits that exist, starting at the high end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("ctz", &bsv_ctz, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountTrailingZeroes: Return number of consecutive 0-bits that exist, starting at the low end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("clo", &bsv_clo, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountLeadingOnes: Return number of consecutive 1-bits that exist, starting at the high end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),
	TYPE_KWMETHOD_F("cto", &bsv_cto, METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?Dint\n"
	                "#tIntegerOverflow{@start or @end is negative or too large}"
	                "CountTrailingOnes: Return number of consecutive 1-bits that exist, starting at the low end of #C{[start,end)}. "
	                /**/ "If @end is greater than or equal to ?#nbits, it is automatically clamped. "
	                /**/ "If @start is greater than @end (after clamping), the call is a no-op"),

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
	TYPE_GETTER_AB_F("frozen", &bsv_frozen, METHOD_FNORMAL,
	                 "->?X2?.?AFrozen?GBitset\n"
	                 "Returns a frozen copy of @this ?., which is either @this when the view is "
	                 /**/ "read-only, and the underlying object is also read-only, or a @this view "
	                 /**/ "wrapped as a ?AFrozen?GBitset when the underlying bits may be modified "
	                 /**/ "by something"),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("nbits", &bsv_nbits, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?Dint\n"
	                 "The # of bits stored in this bitset. Attempting to alter the state of "
	                 /**/ "a bit greater than or equal to this value result in an :IndexError"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst bsv_members[] = {
	TYPE_MEMBER_BITFIELD_DOC("iswritable", STRUCT_CONST, BitsetView, bsv_bflags, Dee_BUFFER_FWRITABLE,
	                         "Evaluates to ?t if @this ?. object not be written to (the inverse of ?#isreadonly)"),
	TYPE_MEMBER_BITFIELD_DOC("ismutable", STRUCT_CONST, BitsetView, bsv_bflags, Dee_BUFFER_FWRITABLE,
	                         "Alias for ?#iswritable, overriding ?Aismutable?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__owner__", STRUCT_OBJECT, offsetof(BitsetView, bsv_owner), "->?X2?GBitset?O"),
	TYPE_MEMBER_FIELD_DOC("__startbit__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BitsetView, bsv_startbit),
	                      "Starting bit number in the buffer of ?#__owner__"),
	TYPE_MEMBER_FIELD_DOC("__endbit__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BitsetView, bsv_endbit),
	                      "Ending bit number in the buffer of ?#__owner__"),
	TYPE_MEMBER_FIELD("__flags__", STRUCT_CONST | STRUCT_UINT, offsetof(BitsetView, bsv_bflags)),
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
	                         "Returns the number of 1-bits (that is: the population count) of this ?.. "
	                         /**/ "Same as ${this.popcount(0, this.nbits)}\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if at least one bit has been turned on\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating the bit indices that are turned on\n"
	                         "\n"

	                         "~->?AFrozen?GBitset\n"
	                         "Return a frozen ?GBitset with the same ?#nbits as @this, but with the state "
	                         /**/ "of all bits inverted. When the buffer of @this bitset can't be modified, "
	                         /**/ "the implementation is allowed to return a sequence proxy instead.\n"
	                         "\n"

	                         "&(other:?X4?S?Dint?GBitset?.?AFrozen?GBitset)->?AFrozen?GBitset\n"
	                         "|(other:?X4?S?Dint?GBitset?.?AFrozen?GBitset)->?AFrozen?GBitset\n"
	                         "^(other:?X4?S?Dint?GBitset?.?AFrozen?GBitset)->?AFrozen?GBitset\n"
	                         "Combine this ?. with another ?GBitset or bitset-like object by performing "
	                         /**/ "the specified bit-wise operator. When the buffer of @this bitset can't "
	                         /**/ "be modified, and @other is a ?AFrozen?GBitset or another ?. where the "
	                         /**/ "underlying buffer can't be modified, the implementation is allowed to "
	                         /**/ "return a sequence proxy instead.\n"
	                         "\n"

	                         "&=(other:?X4?S?Dint?GBitset?.?AFrozen?GBitset)\n"
	                         "|=(other:?X4?S?Dint?GBitset?.?AFrozen?GBitset)\n"
	                         "^=(other:?X4?S?Dint?GBitset?.?AFrozen?GBitset)\n"
	                         "#tBufferError{@this ?. object is not writable}"
	                         "Inplace-combine this ?. with another ?GBitset or bitset-like object by performing "
	                         /**/ "the specified bit-wise operator, and assigning the result back to @this.\n"
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BitsetView,
			/* tp_ctor:        */ &bsv_ctor,
			/* tp_copy_ctor:   */ &bsv_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &bsv_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bsv_serialize
		),
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
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&bsv_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bsv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &bsv_math,
	/* .tp_cmp           = */ &bsv_cmp,
	/* .tp_seq           = */ &bsv_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ &bsv_buffer,
	/* .tp_methods       = */ bsv_methods,
	/* .tp_getsets       = */ bsv_getsets,
	/* .tp_members       = */ bsv_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bsv_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ NULL,
	/* .tp_callable      = */ NULL,
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
	size_t bitno = atomic_read(&self->bsi_bitno);
	if (bitno >= self->bsi_startbit &&
	    bitno <= self->bsi_endbit) /* NOTE: `<= endbit' due to final, exhausted position */
		return bitno - self->bsi_startbit;
	return (size_t)-2; /* Indeterminate */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_setindex(BitsetIterator *__restrict self, size_t index) {
	size_t nbits = self->bsi_endbit - self->bsi_startbit;
	if (index > nbits)
		index = nbits;
	atomic_write(&self->bsi_bitno, self->bsi_startbit + index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_rewind(BitsetIterator *__restrict self) {
	size_t startbit = self->bsi_startbit;
	atomic_write(&self->bsi_bitno, startbit);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_revert(BitsetIterator *__restrict self, size_t step) {
	size_t old_bitno, new_bitno;
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno <= self->bsi_startbit)
			return 1; /* Already at starting position */
		if (OVERFLOW_USUB(old_bitno, step, &new_bitno) || new_bitno < self->bsi_startbit)
			new_bitno = self->bsi_startbit;
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno));
	if (new_bitno <= self->bsi_startbit)
		return 1; /* Now at starting position */
	return 2;     /* Iterator isn't at its starting position */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_advance(BitsetIterator *__restrict self, size_t step) {
	size_t old_bitno, new_bitno;
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno >= self->bsi_endbit)
			return 1; /* Already at end position */
		if (OVERFLOW_UADD(old_bitno, step, &new_bitno) || new_bitno >= self->bsi_endbit)
			new_bitno = self->bsi_endbit;
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno));
	if (new_bitno >= self->bsi_endbit)
		return 1; /* Now at end position */
	return 2;     /* Iterator isn't at its end position */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_prev(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno;
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno <= self->bsi_startbit)
			return 1; /* Already at starting position */
		new_bitno = old_bitno - 1;
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_next(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno;
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno >= self->bsi_endbit)
			return 1; /* Already at end position */
		new_bitno = old_bitno + 1;
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno));
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_nii_hasprev(BitsetIterator *__restrict self) {
	size_t startbit = self->bsi_startbit;
	size_t bitno    = atomic_read(&self->bsi_bitno);
	return bitno > startbit ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsiter_nii_peek(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno;
	old_bitno = atomic_read(&self->bsi_bitno);
	if (old_bitno >= self->bsi_endbit)
		return ITER_DONE; /* Already at end position */
	new_bitno = bitset_nffs(self->bsi_bitset, old_bitno, self->bsi_endbit);
	if (new_bitno >= self->bsi_endbit)
		return ITER_DONE; /* End position reached */
	return DeeInt_NewSize(new_bitno - self->bsi_startbit);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bsiter_next(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno;
	do {
		old_bitno = atomic_read(&self->bsi_bitno);
		if (old_bitno >= self->bsi_endbit)
			return ITER_DONE; /* Already at end position */
		new_bitno = bitset_nffs(self->bsi_bitset, old_bitno, self->bsi_endbit);
		if (new_bitno >= self->bsi_endbit) {
#ifndef __OPTIMIZE_SIZE__
			atomic_cmpxch_weak_or_write(&self->bsi_bitno, old_bitno, new_bitno);
#endif /* !__OPTIMIZE_SIZE__ */
			return ITER_DONE; /* End position reached */
		}
	} while (!atomic_cmpxch_or_write(&self->bsi_bitno, old_bitno, new_bitno + 1));
	return DeeInt_NewSize(new_bitno - self->bsi_startbit);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_bool(BitsetIterator *__restrict self) {
	size_t old_bitno, new_bitno, end_bitno;
	end_bitno = self->bsi_endbit;
	old_bitno = atomic_read(&self->bsi_bitno);
	if (old_bitno >= end_bitno)
		return 0; /* Already at end position */
	new_bitno = bitset_nffs(self->bsi_bitset, old_bitno, end_bitno);
	if (new_bitno >= end_bitno)
		return 0; /* End position reached */
	return 1;
}


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
bsiter_hash(BitsetIterator *self) {
	return atomic_read(&self->bsi_bitno);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsiter_compare(BitsetIterator *self, BitsetIterator *other) {
	if (DeeObject_AssertTypeExact(other, &BitsetIterator_Type))
		goto err;
	Dee_return_compareT(size_t, atomic_read(&self->bsi_bitno),
	                    /*   */ atomic_read(&other->bsi_bitno));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_ctor(BitsetIterator *__restrict self) {
	self->bsi_owner = (DREF DeeObject *)bs_ctor();
	if unlikely(!self->bsi_owner)
		goto err;
	self->bsi_bitset   = ((Bitset *)self->bsi_owner)->bs_bitset;
	self->bsi_startbit = 0;
	self->bsi_endbit   = 0;
	self->bsi_bitno    = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsiter_copy(BitsetIterator *__restrict self,
            BitsetIterator *__restrict other) {
	self->bsi_owner = other->bsi_owner;
	Dee_Incref(self->bsi_owner);
	self->bsi_bitset   = other->bsi_bitset;
	self->bsi_startbit = other->bsi_startbit;
	self->bsi_endbit   = other->bsi_endbit;
	self->bsi_bitno    = atomic_read(&other->bsi_bitno);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bsiter_serialize(BitsetIterator *__restrict self,
                 DeeSerial *__restrict writer,
                 Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(BitsetIterator, field))
	BitsetIterator *out = DeeSerial_Addr2Mem(writer, addr, BitsetIterator);
	out->bsi_startbit = self->bsi_startbit;
	out->bsi_endbit   = self->bsi_endbit;
	out->bsi_bitno    = atomic_read(&self->bsi_bitno);
	if (DeeSerial_PutObject(writer, ADDROF(bsi_owner), self->bsi_owner))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(bsi_bitset), self->bsi_bitset);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bsiter_init(BitsetIterator *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "BitsetIterator", &self->bsi_owner);
	if (BitsetView_Check(self->bsi_owner)) {
		BitsetView *o = (BitsetView *)self->bsi_owner;
		self->bsi_bitset = BitsetView_GetBitset(o);
		self->bsi_startbit = o->bsv_startbit;
		self->bsi_endbit   = o->bsv_endbit;
		self->bsi_bitno    = o->bsv_startbit;
	} else {
		Bitset *o = (Bitset *)self->bsi_owner;
		if (!RoBitset_Check(o)) {
			if (DeeObject_AssertTypeExact(o, &Bitset_Type))
				goto err;
		}
		self->bsi_bitset   = o->bs_bitset;
		self->bsi_startbit = 0;
		self->bsi_endbit   = o->bs_nbits;
		self->bsi_bitno    = 0;
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
bsiter_visit(BitsetIterator *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->bsi_owner);
}


PRIVATE struct type_nii tpconst bsiter_nii = {
	/* .nii_class = */ Dee_TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ Dee_TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (Dee_funptr_t)&bsiter_nii_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&bsiter_nii_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&bsiter_nii_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&bsiter_nii_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)&bsiter_nii_revert,
			/* .nii_advance  = */ (Dee_funptr_t)&bsiter_nii_advance,
			/* .nii_prev     = */ (Dee_funptr_t)&bsiter_nii_prev,
			/* .nii_next     = */ (Dee_funptr_t)&bsiter_nii_next,
			/* .nii_hasprev  = */ (Dee_funptr_t)&bsiter_nii_hasprev,
			/* .nii_peek     = */ (Dee_funptr_t)&bsiter_nii_peek
		}
	}
};

PRIVATE struct type_cmp bsiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&bsiter_hash,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&bsiter_compare,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ NULL,
	/* .tp_ne            = */ NULL,
	/* .tp_lo            = */ NULL,
	/* .tp_le            = */ NULL,
	/* .tp_gr            = */ NULL,
	/* .tp_ge            = */ NULL,
	/* .tp_nii           = */ &bsiter_nii
};

PRIVATE struct type_member tpconst bsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(BitsetIterator, bsi_owner),
	                      "->?X2?GBitset?AView?GBitset\n"
	                      "The ?GBitset or ?AView?GBitset that is being iterated"),
	TYPE_MEMBER_FIELD_DOC("__startbit__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BitsetIterator, bsi_startbit),
	                      "Starting bit number in the buffer of ?#__owner__"),
	TYPE_MEMBER_FIELD_DOC("__endbit__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(BitsetIterator, bsi_endbit),
	                      "Ending bit number in the buffer of ?#__owner__"),
	TYPE_MEMBER_FIELD_DOC("__bitno__", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(BitsetIterator, bsi_bitno),
	                      "The next bit index to iterate (skipping bits that aren't turned on)"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject BitsetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "BitsetIterator",
	/* .tp_doc      = */ DOC("(seq?:?GBitset)\n"
	                         "\n"
	                         "next->?Dint"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ BitsetIterator,
			/* tp_ctor:        */ &bsiter_ctor,
			/* tp_copy_ctor:   */ &bsiter_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &bsiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &bsiter_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&bsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &bsiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&bsiter_next,
	/* .tp_iterator      = */ NULL,
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
