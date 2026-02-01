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
/*!export **/
#ifndef GUARD_DEEMON_UTIL_HASH_IO_H
#define GUARD_DEEMON_UTIL_HASH_IO_H 1 /*!export-*/

#include "../api.h"
#include "../types.h"

#include <hybrid/typecore.h> /* __BYTE_TYPE__, __SHIFT_TYPE__, __SIZEOF_SIZE_T__, __UINT*_C */

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintN_t */

DECL_BEGIN

/* Terminology:
 *
 * TERM       TYPE                 EXAMPLE                  DESCRIPTION
 * VTAB       VTAB_ITEM            DeeDictObject.d_vtab     Value table. Base address can either
 *                                                          be REAL (0-based) or VIRT (1-based)
 * HTAB       union Dee_hash_htab  DeeDictObject.d_htab     Hash table. Vector of `union Dee_hash_htab'.
 *                                                          Element size depends on `Dee_hash_hidxio_t',
 *                                                          which in turn depends on VALLOC
 * VALLOC     Dee_hash_vidx_t      DeeDictObject.d_valloc   Allocated size of VTAB
 * HMASK      Dee_hash_t           DeeDictObject.d_hmask    Hash mask. == size of HTAB (in elements) -1.
 *                                                          +1 is always a power-of-two. Used to &-mask
 *                                                          hash values to convert them into HTAB indices.
 * OPS        Dee_hash_hidxio_ops  DeeDictObject.d_hidxops  Operators for HTAB. Must always be equal to:
 *                                                          >> &Dee_hash_hidxio[Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC)]
 */


/* Index for VTAB (real and virt version) */
#define Dee_SIZEOF_HASH_VIDX_T __SIZEOF_SIZE_T__
typedef size_t Dee_hash_vidx_t;

/* Index for HTAB */
#define Dee_SIZEOF_HASH_HIDX_T Dee_SIZEOF_HASH_T
typedef Dee_hash_t Dee_hash_hidx_t;
#define Dee_hash_hidx_ofhash(hs, HMASK) ((hs) & (HMASK))

union Dee_hash_htab;

typedef WUNUSED_T NONNULL_T((1)) /*virt*/ Dee_hash_vidx_t (DFCALL *Dee_hash_gethidx_t)(union Dee_hash_htab const *__restrict htab, Dee_hash_hidx_t index);
typedef NONNULL_T((1)) void (DFCALL *Dee_hash_sethidx_t)(union Dee_hash_htab *__restrict htab, Dee_hash_hidx_t index, /*virt*/ Dee_hash_vidx_t value);

/* 8-bit HTAB operators (here for static initialization) */
DFUNDEF WUNUSED NONNULL((1)) /*virt*/ Dee_hash_vidx_t DFCALL Dee_hash_gethidx8(union Dee_hash_htab const *__restrict htab, Dee_hash_hidx_t index);
DFUNDEF NONNULL((1)) void DFCALL Dee_hash_sethidx8(union Dee_hash_htab *__restrict htab, Dee_hash_hidx_t index, /*virt*/ Dee_hash_vidx_t value);

DDATDEF __BYTE_TYPE__ const _DeeHash_EmptyTab[];
#define DeeHash_EmptyVTab(VTAB_ITEM) /*virt*/ ((VTAB_ITEM *)_DeeHash_EmptyTab - 1)
#define DeeHash_EmptyHTab            ((union Dee_hash_htab *)_DeeHash_EmptyTab)

#ifdef DEE_SOURCE

/* Helper macros for converting between "virt" and "real" hash VIDX indices. */
#define Dee_hash_vidx_virt2real(/*Dee_hash_vidx_t **/p_self) (void)(--*(p_self))
#define Dee_hash_vidx_real2virt(/*Dee_hash_vidx_t **/p_self) (void)(++*(p_self))
#define Dee_hash_vidx_toreal(/*Dee_hash_vidx_t*/self)        ((self) - 1)
#define Dee_hash_vidx_tovirt(/*Dee_hash_vidx_t*/self)        ((self) + 1)
#if 1
#define Dee_hash_vidx_virt_lt_real(/*virt Dee_hash_vidx_t*/virt_self, /*real Dee_hash_vidx_t*/real_count) ((virt_self) <= (real_count))
#else
#define Dee_hash_vidx_virt_lt_real(/*virt Dee_hash_vidx_t*/virt_self, /*real Dee_hash_vidx_t*/real_count) (Dee_hash_vidx_toreal(virt_self) < (real_count))
#endif

typedef __SHIFT_TYPE__ Dee_hash_hidxio_t;

struct Dee_hash_hidxio_ops {
	Dee_hash_gethidx_t hxio_get; /* Getter */
	Dee_hash_sethidx_t hxio_set; /* Setter */

	/* bzero */
	NONNULL_T((1)) void
	(DCALL *hxio_zro)(union Dee_hash_htab *dst,
	                  Dee_hash_hidx_t n_words);

	/* memcpy */
	NONNULL_T((1, 2)) void
	(DCALL *hxio_cpy)(union Dee_hash_htab *dst,
	                  union Dee_hash_htab const *src,
	                  Dee_hash_hidx_t n_words);

	/* memmove */
	NONNULL_T((1, 2)) void
	(DCALL *hxio_mov)(union Dee_hash_htab *dst,
	                  union Dee_hash_htab const *src,
	                  Dee_hash_hidx_t n_words);
#define hxio_movup     hxio_mov  /* memmoveup */   /*!export-*/
#define hxio_movdown   hxio_mov  /* memmovedown */ /*!export-*/

	/* Upsize ("dst" is Dee_hash_hidxio_t+1; assume that "dst >= src") */
	NONNULL_T((1)) void
	(DCALL *hxio_upr)(union Dee_hash_htab *dst,
	                  union Dee_hash_htab const *src,
	                  Dee_hash_hidx_t n_words);

	/* Downsize ("dst" is Dee_hash_hidxio_t-1; assume that "dst <= src") */
	NONNULL_T((1)) void
	(DCALL *hxio_lwr)(union Dee_hash_htab *dst,
	                  union Dee_hash_htab const *src,
	                  Dee_hash_hidx_t n_words);

	/* Insert `it_vidx' (whose associated object has a hash of `it_hash')
	 * into "HTAB", and return the "htab_idx" where the item was inserted:
	 * >> Dee_hash_t hs, perturb;
	 * >> for (_DeeHash_HashIdxInit(&hs, &perturb, it_hash, hmask);;
	 * >>      _DeeHash_HashIdxNext(&hs, &perturb, it_hash, hmask)) {
	 * >>     Dee_hash_hidx_t htab_idx = Dee_hash_hidx_ofhash(hs, hmask);
	 * >>     Dee_hash_vidx_t vtab_idx = (*hxio_get)(htab, htab_idx);
	 * >>     if likely(vtab_idx == Dee_HASH_HTAB_EOF) {
	 * >>         (*hxio_set)(htab, htab_idx, it_vidx);
	 * >>         return htab_idx;
	 * >>     }
	 * >> }
	 *
	 * Caller is responsible to ensure that "htab" isn't fully in-use,
	 * meaning it has at least 2 "Dee_HASH_HTAB_EOF" entries. */
	NONNULL_T((1)) Dee_hash_hidx_t
	(DCALL *hxio_insert)(union Dee_hash_htab *htab, Dee_hash_t hmask,
	                     Dee_hash_t it_hash, /*virt*/Dee_hash_vidx_t it_vidx);

	/* Decrement all HTAB elements `>= vtab_threshold':
	 * >> Dee_hash_t i;
	 * >> for (i = 0; i <= hmask; ++i) {
	 * >>     Dee_hash_vidx_t vtab_index = (*hxio_get)(htab, i);
	 * >>     if (vtab_index >= vtab_threshold)
	 * >>         (*hxio_set)(htab, i, vtab_index - 1);
	 * >> } */
	NONNULL_T((1)) void
	(DCALL *hxio_decafter)(union Dee_hash_htab *htab, Dee_hash_t hmask,
	                       /*virt*/ Dee_hash_vidx_t vtab_threshold);

	/* Increment all HTAB elements `>= vtab_threshold':
	 * >> Dee_hash_t i;
	 * >> for (i = 0; i <= hmask; ++i) {
	 * >>     Dee_hash_vidx_t vtab_index = (*hxio_get)(htab, i);
	 * >>     if (vtab_index >= vtab_threshold)
	 * >>         (*hxio_set)(htab, i, vtab_index + 1);
	 * >> } */
	NONNULL_T((1)) void
	(DCALL *hxio_incafter)(union Dee_hash_htab *htab, Dee_hash_t hmask,
	                       /*virt*/ Dee_hash_vidx_t vtab_threshold);

	/* Decrement all HTAB elements `>= vtab_min && <= vtab_max':
	 * >> Dee_hash_t i;
	 * >> for (i = 0; i <= hmask; ++i) {
	 * >>     Dee_hash_vidx_t vtab_index = (*hxio_get)(htab, i);
	 * >>     if (vtab_index >= vtab_min && vtab_index <= vtab_max)
	 * >>         (*hxio_set)(htab, i, vtab_index - 1);
	 * >> } */
	NONNULL_T((1)) void
	(DCALL *hxio_decrange)(union Dee_hash_htab *htab, Dee_hash_t hmask,
	                       /*virt*/ Dee_hash_vidx_t vtab_min,
	                       /*virt*/ Dee_hash_vidx_t vtab_max);

	/* Increment all HTAB elements `>= vtab_min && <= vtab_max':
	 * >> Dee_hash_t i;
	 * >> for (i = 0; i <= hmask; ++i) {
	 * >>     Dee_hash_vidx_t vtab_index = (*hxio_get)(htab, i);
	 * >>     if (vtab_index >= vtab_min && vtab_index <= vtab_max)
	 * >>         (*hxio_set)(htab, i, vtab_index + 1);
	 * >> } */
	NONNULL_T((1)) void
	(DCALL *hxio_incrange)(union Dee_hash_htab *htab, Dee_hash_t hmask,
	                       /*virt*/ Dee_hash_vidx_t vtab_min,
	                       /*virt*/ Dee_hash_vidx_t vtab_max);

	/* Reverse the order of all references to VTAB elements in [vtab_min,vtab_max],
	 * such that a reference to 'vtab_min' becomes one to 'vtab_max', as well as the
	 * inverse:
	 * >> Dee_hash_hidx_t i;
	 * >> Dee_hash_vidx_t ceiling = vmin + vmax;
	 * >> for (i = 0; i <= hmask; ++i) {
	 * >>     Dee_hash_vidx_t vtab_index = (*hxio_get)(htab, i);
	 * >>     if (vtab_index >= vtab_min && vtab_index <= vtab_max) {
	 * >>         vtab_index = ceiling - vtab_index;
	 * >>         (*hxio_set)(htab, i, vtab_index);
	 * >>     }
	 * >> } */
	NONNULL_T((1)) void
	(DCALL *hxio_revrange)(union Dee_hash_htab *htab, Dee_hash_t hmask,
	                       /*virt*/ Dee_hash_vidx_t vtab_min,
	                       /*virt*/ Dee_hash_vidx_t vtab_max);
};

/* NOTE: HIDXIO indices can also used as <<shifts to multiply some value by the size of an index:
 * >> Dee_hash_hidx_t htab_size = (HMASK + 1) << Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC); */
#if __SIZEOF_SIZE_T__ >= 8
#define Dee_HASH_HIDXIO_COUNT 4
#define Dee_HASH_HIDXIO_IS8(VALLOC)  likely((VALLOC) <= __UINT8_C(0xff))
#define Dee_HASH_HIDXIO_IS16(VALLOC) likely((VALLOC) <= __UINT16_C(0xffff))
#define Dee_HASH_HIDXIO_IS32(VALLOC) likely((VALLOC) <= __UINT32_C(0xffffffff))
#define Dee_HASH_HIDXIO_IS64(VALLOC) 1
#if 1
#define Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC) \
	(((VALLOC) > __UINT8_C(0xff)) + ((VALLOC) > __UINT16_C(0xffff)) + ((VALLOC) > __UINT32_C(0xffffffff)))
#else
#define Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC) \
	(Dee_HASH_HIDXIO_IS8(VALLOC) ? 0 : Dee_HASH_HIDXIO_IS16(VALLOC) ? 1 : Dee_HASH_HIDXIO_IS32(VALLOC) ? 2 : 3)
#endif
#elif __SIZEOF_SIZE_T__ >= 4
#define Dee_HASH_HIDXIO_COUNT 3
#define Dee_HASH_HIDXIO_IS8(VALLOC)  likely((VALLOC) <= __UINT8_C(0xff))
#define Dee_HASH_HIDXIO_IS16(VALLOC) likely((VALLOC) <= __UINT16_C(0xffff))
#define Dee_HASH_HIDXIO_IS32(VALLOC) 1
#if 1
#define Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC) \
	(((VALLOC) > __UINT8_C(0xff)) + ((VALLOC) > __UINT16_C(0xffff)))
#else
#define Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC) \
	(Dee_HASH_HIDXIO_IS8(VALLOC) ? 0 : Dee_HASH_HIDXIO_IS16(VALLOC) ? 1 : 2)
#endif
#elif __SIZEOF_SIZE_T__ >= 2
#define Dee_HASH_HIDXIO_COUNT 2
#define Dee_HASH_HIDXIO_IS8(VALLOC)  likely((VALLOC) <= __UINT8_C(0xff))
#define Dee_HASH_HIDXIO_IS16(VALLOC) 1
#if 1
#define Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC) \
	((VALLOC) > __UINT8_C(0xff))
#else
#define Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC) \
	(Dee_HASH_HIDXIO_IS8(VALLOC) ? 0 : 1)
#endif
#else /* __SIZEOF_SIZE_T__ >= 1 */
#define Dee_HASH_HIDXIO_COUNT 1
#define Dee_HASH_HIDXIO_IS8(VALLOC) 1
#define Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC) 0
#endif /* __SIZEOF_SIZE_T__ < 1 */

/* This is the typing of HTAB elements */
union Dee_hash_htab {
#ifdef Dee_HASH_HIDXIO_IS8
	uint8_t  ht_8[4096];
#endif /* Dee_HASH_HIDXIO_IS8 */
#ifdef Dee_HASH_HIDXIO_IS16
	uint16_t ht_16[4096];
#endif /* Dee_HASH_HIDXIO_IS16 */
#ifdef Dee_HASH_HIDXIO_IS32
	uint32_t ht_32[4096];
#endif /* Dee_HASH_HIDXIO_IS32 */
#ifdef Dee_HASH_HIDXIO_IS64
	uint64_t ht_64[4096];
#endif /* Dee_HASH_HIDXIO_IS64 */
};



/* Dynamic hash I/O functions:
 * >> vtab = &Dee_hash_hidxio[Dee_HASH_HIDXIO_FROM_VALLOC(VALLOC)]; */
DDATDEF struct Dee_hash_hidxio_ops Dee_tpconst Dee_hash_hidxio[Dee_HASH_HIDXIO_COUNT];

/* Index value found in the HTAB table when end-of-chain is encountered. */
#define Dee_HASH_HTAB_EOF 0

/* Get/set VTAB in both its:
 * - virt[ual] (index starts at 1), and
 * - real (index starts at 0) form
 *
 * VIRT:
 * - Accepts indices in range "[Dee_hash_vidx_tovirt(0),Dee_hash_vidx_tovirt(ht_vsize)-1)"  (aka: "[1,ht_vsize]")
 * - These sort of indices are what is stored in HTAB. Indices
 *   start at 1, because an index=0 appearing in HTAB has the
 *   special meaning of `Dee_HASH_HTAB_EOF'
 *
 * REAL:
 * - Accepts indices in range "[0,ht_vsize)"
 * - Actual, regular, 0-based indices
 * - _DeeHash_GetRealVTab() also represents the actual base
 *   of the heap-block holding the hash tables's tables. */
#define _DeeHash_VIRT_GetVirtVTab(VTAB)    (VTAB)                   /* Assuming that VTAB is stored as virtual */
#define _DeeHash_VIRT_SetVirtVTab(VTAB, v) (void)((VTAB) = (v))     /* Assuming that VTAB is stored as virtual */
#define _DeeHash_VIRT_GetRealVTab(VTAB)    ((VTAB) + 1)             /* Assuming that VTAB is stored as virtual */
#define _DeeHash_VIRT_SetRealVTab(VTAB, v) (void)((VTAB) = (v) - 1) /* Assuming that VTAB is stored as virtual */
#define _DeeHash_REAL_GetVirtVTab(VTAB)    ((VTAB) - 1)             /* Assuming that VTAB is stored as real */
#define _DeeHash_REAL_SetVirtVTab(VTAB, v) (void)((VTAB) = (v) - 1) /* Assuming that VTAB is stored as real */
#define _DeeHash_REAL_GetRealVTab(VTAB)    (VTAB)                   /* Assuming that VTAB is stored as real */
#define _DeeHash_REAL_SetRealVTab(VTAB, v) (void)((VTAB) = (v))     /* Assuming that VTAB is stored as real */

/* Hash-index iteration functions:
 * >> Dee_hash_t hash = DeeObject_Hash(key);
 * >> Dee_hash_t hs, perturb;
 * >> for (_DeeHash_HashIdxInit(&hs, &perturb, hash, self->HMASK);;
 * >>      _DeeHash_HashIdxNext(&hs, &perturb, hash, self->HMASK)) {
 * >>     int cmp;
 * >>     VTAB_ITEM *item;
 * >>     Dee_hash_hidx_t hidx = Dee_hash_hidx_ofhash(hs, self->HMASK);
 * >>     Dee_hash_vidx_t vidx = (*OPS->hxio_get)(self->HTAB, hidx);
 * >>     if (vidx == Dee_HASH_HTAB_EOF) {
 * >>         ...   Key not found
 * >>     }
 * >>     item = _DeeHash_[VIRT|REAL]_GetVirtVTab(self->VTAB)[vidx];
 * >>     cmp = DeeObject_TryCompareEq(item->vti_key, key);
 * >>     if (Dee_COMPARE_ISERR(cmp))
 * >>         goto err;
 * >>     if (Dee_COMPARE_ISEQ(cmp)) {
 * >>         ...   Key found
 * >>     }
 * >> }
 */
#define _DeeHash_HashIdxInit(p_hs, p_perturb, hash, HMASK) \
	(void)(*(p_hs) = (*(p_perturb) = (hash)) & (HMASK))
#define _DeeHash_HashIdxNext(p_hs, p_perturb, hash, HMASK) \
	(void)(*(p_hs) = (*(p_hs) << 2) + *(p_hs) + *(p_perturb) + 1, *(p_perturb) >>= 5)



#if Dee_HASH_HIDXIO_COUNT >= 2
#define IF_Dee_HASH_HIDXIO_COUNT_GE_2(...) __VA_ARGS__
#else /* Dee_HASH_HIDXIO_COUNT >= 2 */
#define IF_Dee_HASH_HIDXIO_COUNT_GE_2(...) /* nothing */
#endif /* Dee_HASH_HIDXIO_COUNT < 2 */
#if Dee_HASH_HIDXIO_COUNT >= 3
#define IF_Dee_HASH_HIDXIO_COUNT_GE_3(...) __VA_ARGS__
#else /* Dee_HASH_HIDXIO_COUNT >= 3 */
#define IF_Dee_HASH_HIDXIO_COUNT_GE_3(...) /* nothing */
#endif /* Dee_HASH_HIDXIO_COUNT < 3 */
#if Dee_HASH_HIDXIO_COUNT >= 4
#define IF_Dee_HASH_HIDXIO_COUNT_GE_4(...) __VA_ARGS__
#else /* Dee_HASH_HIDXIO_COUNT >= 4 */
#define IF_Dee_HASH_HIDXIO_COUNT_GE_4(...) /* nothing */
#endif /* Dee_HASH_HIDXIO_COUNT < 4 */

#endif /* DEE_SOURCE */

DECL_END

#endif /* !GUARD_DEEMON_UTIL_HASH_IO_H */
