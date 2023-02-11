/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_ATOMIC_H
#define GUARD_DEEMON_UTIL_ATOMIC_H 1

#include "../api.h"
/**/

#ifdef CONFIG_NO_THREADS
#define Dee_ATOMIC_RELAXED 0
#define Dee_ATOMIC_CONSUME 0
#define Dee_ATOMIC_ACQUIRE 0
#define Dee_ATOMIC_RELEASE 0
#define Dee_ATOMIC_ACQ_REL 0
#define Dee_ATOMIC_SEQ_CST 0

#define Dee_atomic_thread_fence(order)             (void)0
#define Dee_atomic_signal_fence(order)             (void)0
#define Dee_atomic_read_explicit(p, order)         (*(p))
#define Dee_atomic_write_explicit(p, value, order) (void)(*(p) = (value))
#define Dee_atomic_cmpxch_explicit(p, old_value, new_value, succ_order, fail_order) \
	(*(p) == (old_value) ? (*(p) = (new_value), 1) : 0)
#define Dee_atomic_cmpxch_val_explicit(p, old_value, new_value, succ_order, fail_order) \
	(*(p) == (old_value) ? (*(p) = (new_value)) : *(p))
#define Dee_atomic_fetchinc_explicit(p, order)         (*(p)++)
#define Dee_atomic_fetchdec_explicit(p, order)         (*(p)--)
#define Dee_atomic_fetchadd_explicit(p, value, order)  ((*(p) += (value)) - (value))
#define Dee_atomic_fetchsub_explicit(p, value, order)  ((*(p) -= (value)) + (value))
#define Dee_atomic_fetchxor_explicit(p, value, order)  ((*(p) ^= (value)) ^ (value))
#define Dee_atomic_incfetch_explicit(p, order)         (++*(p))
#define Dee_atomic_decfetch_explicit(p, order)         (--*(p))
#define Dee_atomic_addfetch_explicit(p, value, order)  (*(p) += (value))
#define Dee_atomic_subfetch_explicit(p, value, order)  (*(p) -= (value))
#define Dee_atomic_andfetch_explicit(p, value, order)  (*(p) &= (value))
#define Dee_atomic_orfetch_explicit(p, value, order)   (*(p) |= (value))
#define Dee_atomic_xorfetch_explicit(p, value, order)  (*(p) ^= (value))
#define Dee_atomic_nandfetch_explicit(p, value, order) (*(p) = ~(*(p) & (value)))
#define Dee_atomic_inc_explicit(p, order)              (void)(++*(p))
#define Dee_atomic_dec_explicit(p, order)              (void)(--*(p))
#define Dee_atomic_add_explicit(p, value, order)       (void)(*(p) += (value))
#define Dee_atomic_sub_explicit(p, value, order)       (void)(*(p) -= (value))
#define Dee_atomic_and_explicit(p, value, order)       (void)(*(p) &= (value))
#define Dee_atomic_or_explicit(p, value, order)        (void)(*(p) |= (value))
#define Dee_atomic_xor_explicit(p, value, order)       (void)(*(p) ^= (value))
#define Dee_atomic_nand_explicit(p, value, order)      (void)(*(p) = ~(*(p) & (value)))

#define Dee_atomic_cmpxch_or_write(p, old_value, new_value)      (Dee_ASSERT(*(p) == (old_value)), *(p) = (new_value), 1)
#define Dee_atomic_cmpxch_weak_or_write(p, old_value, new_value) (Dee_ASSERT(*(p) == (old_value)), *(p) = (new_value), 1)

#if !defined(__NO_XBLOCK) && defined(__COMPILER_HAVE_AUTOTYPE)
#define Dee_atomic_xch_explicit(p, value, order)       __XBLOCK({ __auto_type _dafxe_res = *(p); *(p) = (value); __XRETURN _dafxe_res; })
#define Dee_atomic_fetchand_explicit(p, value, order)  __XBLOCK({ __auto_type _dafae_res = *(p); *(p) &= (value); __XRETURN _dafae_res; })
#define Dee_atomic_fetchor_explicit(p, value, order)   __XBLOCK({ __auto_type _dafoe_res = *(p); *(p) |= (value); __XRETURN _dafoe_res; })
#define Dee_atomic_fetchnand_explicit(p, value, order) __XBLOCK({ __auto_type _dafne_res = *(p); *(p) = ~(*(p) & (value)); __XRETURN _dafne_res; })
#elif !defined(__NO_XBLOCK) && defined(__COMPILER_HAVE_TYPEOF)
#define Dee_atomic_xch_explicit(p, value, order)       __XBLOCK({ __typeof__(*(p)) _dafxe_res = *(p); *(p) = (value); __XRETURN _dafxe_res; })
#define Dee_atomic_fetchand_explicit(p, value, order)  __XBLOCK({ __typeof__(*(p)) _dafae_res = *(p); *(p) &= (value); __XRETURN _dafae_res; })
#define Dee_atomic_fetchor_explicit(p, value, order)   __XBLOCK({ __typeof__(*(p)) _dafoe_res = *(p); *(p) |= (value); __XRETURN _dafoe_res; })
#define Dee_atomic_fetchnand_explicit(p, value, order) __XBLOCK({ __typeof__(*(p)) _dafne_res = *(p); *(p) = ~(*(p) & (value)); __XRETURN _dafne_res; })
#elif defined(__cplusplus)
extern "C++" {
template<class T> inline WUNUSED NONNULL((1)) T
_Dee_atomic_xch_no_threads(T *p, T value) {
	T result = *p;
	*p = value;
	return result;
}
template<class T> inline WUNUSED NONNULL((1)) T
_Dee_atomic_fetchand_no_threads(T *p, T value) {
	T result = *p;
	*p &= value;
	return result;
}
template<class T> inline WUNUSED NONNULL((1)) T
_Dee_atomic_fetchor_no_threads(T *p, T value) {
	T result = *p;
	*p |= value;
	return result;
}
template<class T> inline WUNUSED NONNULL((1)) T
_Dee_atomic_fetchnand_no_threads(T *p, T value) {
	T result = *p;
	*p = ~(*p & value);
	return result;
}
} /* extern "C++" */
#define Dee_atomic_xch_explicit(p, value, order)       _Dee_atomic_xch_no_threads(p, value)
#define Dee_atomic_fetchand_explicit(p, value, order)  _Dee_atomic_fetchand_no_threads(p, value)
#define Dee_atomic_fetchor_explicit(p, value, order)   _Dee_atomic_fetchor_no_threads(p, value)
#define Dee_atomic_fetchnand_explicit(p, value, order) _Dee_atomic_fetchnand_no_threads(p, value)
#else /* ... */
#include <hybrid/typecore.h>

#define DEE_DEFINE_ATOMIC_HELPERS(n, T)                   \
	LOCAL WUNUSED NONNULL((1)) T                          \
	_Dee_atomic_xch_no_threads_##n(T *p, T value) {       \
		T result = *p;                                    \
		*p = value;                                       \
		return result;                                    \
	}                                                     \
	LOCAL WUNUSED NONNULL((1)) T                          \
	_Dee_atomic_fetchand_no_threads_##n(T *p, T value) {  \
		T result = *p;                                    \
		*p &= value;                                      \
		return result;                                    \
	}                                                     \
	LOCAL WUNUSED NONNULL((1)) T                          \
	_Dee_atomic_fetchor_no_threads_##n(T *p, T value) {   \
		T result = *p;                                    \
		*p |= value;                                      \
		return result;                                    \
	}                                                     \
	LOCAL WUNUSED NONNULL((1)) T                          \
	_Dee_atomic_fetchnand_no_threads_##n(T *p, T value) { \
		T result = *p;                                    \
		*p = ~(*p & value);                               \
		return result;                                    \
	}
DEE_DEFINE_ATOMIC_HELPERS(8, __UINT8_TYPE__)
DEE_DEFINE_ATOMIC_HELPERS(16, __UINT16_TYPE__)
DEE_DEFINE_ATOMIC_HELPERS(32, __UINT32_TYPE__)
#ifdef __UINT64_TYPE__
DEE_DEFINE_ATOMIC_HELPERS(64, __UINT64_TYPE__)
#endif /* __UINT64_TYPE__ */

#ifdef __COMPILER_HAVE_TYPEOF
#define _Dee_ATOMIC_RECAST(x, y) ((__typeof__(x))(y))
#elif 1
#define _Dee_ATOMIC_RECAST(x, y) (1 ? (y) : (x))
#else /* ... */
#define _Dee_ATOMIC_RECAST(x, y) (y)
#endif /* !... */
#ifdef _MSC_VER
#define _Dee_ATOMIC_DOWNCAST(T) (T)(__UINTPTR_TYPE__)
#else /* _MSC_VER */
#define _Dee_ATOMIC_DOWNCAST(T) (T)
#endif /* !_MSC_VER */
/* clang-format off */
#define Dee_atomic_xch_explicit(p, value, order)                                                                                                 \
	_Dee_ATOMIC_RECAST(*(x), sizeof(x) == 1 ? _Dee_atomic_xch_no_threads_8((__UINT8_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT8_TYPE__)(value)) :    \
	                         sizeof(x) == 2 ? _Dee_atomic_xch_no_threads_16((__UINT16_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT16_TYPE__)(value)) : \
	                         sizeof(x) == 4 ? _Dee_atomic_xch_no_threads_32((__UINT32_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT32_TYPE__)(value)) : \
	                                          _Dee_atomic_xch_no_threads_64((__UINT64_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT64_TYPE__)(value)))
#define Dee_atomic_fetchand_explicit(p, value, order)                                                                                                 \
	_Dee_ATOMIC_RECAST(*(x), sizeof(x) == 1 ? _Dee_atomic_fetchand_no_threads_8((__UINT8_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT8_TYPE__)(value)) :    \
	                         sizeof(x) == 2 ? _Dee_atomic_fetchand_no_threads_16((__UINT16_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT16_TYPE__)(value)) : \
	                         sizeof(x) == 4 ? _Dee_atomic_fetchand_no_threads_32((__UINT32_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT32_TYPE__)(value)) : \
	                                          _Dee_atomic_fetchand_no_threads_64((__UINT64_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT64_TYPE__)(value)))
#define Dee_atomic_fetchor_explicit(p, value, order)                                                                                                 \
	_Dee_ATOMIC_RECAST(*(x), sizeof(x) == 1 ? _Dee_atomic_fetchor_no_threads_8((__UINT8_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT8_TYPE__)(value)) :    \
	                         sizeof(x) == 2 ? _Dee_atomic_fetchor_no_threads_16((__UINT16_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT16_TYPE__)(value)) : \
	                         sizeof(x) == 4 ? _Dee_atomic_fetchor_no_threads_32((__UINT32_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT32_TYPE__)(value)) : \
	                                          _Dee_atomic_fetchor_no_threads_64((__UINT64_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT64_TYPE__)(value)))
#define Dee_atomic_fetchnand_explicit(p, value, order)                                                                                                 \
	_Dee_ATOMIC_RECAST(*(x), sizeof(x) == 1 ? _Dee_atomic_fetchnand_no_threads_8((__UINT8_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT8_TYPE__)(value)) :    \
	                         sizeof(x) == 2 ? _Dee_atomic_fetchnand_no_threads_16((__UINT16_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT16_TYPE__)(value)) : \
	                         sizeof(x) == 4 ? _Dee_atomic_fetchnand_no_threads_32((__UINT32_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT32_TYPE__)(value)) : \
	                                          _Dee_atomic_fetchnand_no_threads_64((__UINT64_TYPE__ *)(p), __ATOMIC_DOWNCAST(__UINT64_TYPE__)(value)))
/* clang-format on */
#endif /* !... */

#else /* CONFIG_NO_THREADS */
#include <hybrid/__atomic.h>

#define Dee_ATOMIC_RELAXED __ATOMIC_RELAXED
#define Dee_ATOMIC_CONSUME __ATOMIC_CONSUME
#define Dee_ATOMIC_ACQUIRE __ATOMIC_ACQUIRE
#define Dee_ATOMIC_RELEASE __ATOMIC_RELEASE
#define Dee_ATOMIC_ACQ_REL __ATOMIC_ACQ_REL
#define Dee_ATOMIC_SEQ_CST __ATOMIC_SEQ_CST

#define Dee_atomic_thread_fence(order)             __hybrid_atomic_thread_fence(order)
#define Dee_atomic_signal_fence(order)             __hybrid_atomic_signal_fence(order)
#define Dee_atomic_read_explicit(p, order)         __hybrid_atomic_load(p, order)
#define Dee_atomic_write_explicit(p, value, order) __hybrid_atomic_store(p, value, order)
#define Dee_atomic_xch_explicit(p, value, order)   __hybrid_atomic_xch(p, value, order)
#define Dee_atomic_cmpxch_explicit(p, old_value, new_value, succ_order, fail_order) \
	__hybrid_atomic_cmpxch(p, old_value, new_value, succ_order, fail_order)
#define Dee_atomic_cmpxch_weak_explicit(p, old_value, new_value, succ_order, fail_order) \
	__hybrid_atomic_cmpxch_weak(p, old_value, new_value, succ_order, fail_order)
#define Dee_atomic_cmpxch_val_explicit(p, old_value, new_value, succ_order, fail_order) \
	__hybrid_atomic_cmpxch_val(p, old_value, new_value, succ_order, fail_order)

#define Dee_atomic_fetchinc_explicit(p, order)         __hybrid_atomic_fetchinc(p, order)
#define Dee_atomic_fetchdec_explicit(p, order)         __hybrid_atomic_fetchdec(p, order)
#define Dee_atomic_fetchadd_explicit(p, value, order)  __hybrid_atomic_fetchadd(p, value, order)
#define Dee_atomic_fetchsub_explicit(p, value, order)  __hybrid_atomic_fetchsub(p, value, order)
#define Dee_atomic_fetchand_explicit(p, value, order)  __hybrid_atomic_fetchand(p, value, order)
#define Dee_atomic_fetchor_explicit(p, value, order)   __hybrid_atomic_fetchor(p, value, order)
#define Dee_atomic_fetchxor_explicit(p, value, order)  __hybrid_atomic_fetchxor(p, value, order)
#define Dee_atomic_fetchnand_explicit(p, value, order) __hybrid_atomic_fetchnand(p, value, order)

#define Dee_atomic_incfetch_explicit(p, order)         __hybrid_atomic_incfetch(p, order)
#define Dee_atomic_decfetch_explicit(p, order)         __hybrid_atomic_decfetch(p, order)
#define Dee_atomic_addfetch_explicit(p, value, order)  __hybrid_atomic_addfetch(p, value, order)
#define Dee_atomic_subfetch_explicit(p, value, order)  __hybrid_atomic_subfetch(p, value, order)
#define Dee_atomic_andfetch_explicit(p, value, order)  __hybrid_atomic_andfetch(p, value, order)
#define Dee_atomic_orfetch_explicit(p, value, order)   __hybrid_atomic_orfetch(p, value, order)
#define Dee_atomic_xorfetch_explicit(p, value, order)  __hybrid_atomic_xorfetch(p, value, order)
#define Dee_atomic_nandfetch_explicit(p, value, order) __hybrid_atomic_nandfetch(p, value, order)

#define Dee_atomic_inc_explicit(p, order)         __hybrid_atomic_inc(p, order)
#define Dee_atomic_dec_explicit(p, order)         __hybrid_atomic_dec(p, order)
#define Dee_atomic_add_explicit(p, value, order)  __hybrid_atomic_add(p, value, order)
#define Dee_atomic_sub_explicit(p, value, order)  __hybrid_atomic_sub(p, value, order)
#define Dee_atomic_and_explicit(p, value, order)  __hybrid_atomic_and(p, value, order)
#define Dee_atomic_or_explicit(p, value, order)   __hybrid_atomic_or(p, value, order)
#define Dee_atomic_xor_explicit(p, value, order)  __hybrid_atomic_xor(p, value, order)
#define Dee_atomic_nand_explicit(p, value, order) __hybrid_atomic_nand(p, value, order)

#endif /* !CONFIG_NO_THREADS */

#ifndef Dee_atomic_cmpxch_explicit
#define Dee_atomic_cmpxch_explicit(p, old_value, new_value, succ_order, fail_order) \
	(Dee_atomic_cmpxch_val_explicit(p, old_value, new_value, succ_order, fail_order) == (old_value))
#endif /* !Dee_atomic_cmpxch_explicit */
#ifndef Dee_atomic_cmpxch_weak_explicit
#define Dee_atomic_cmpxch_weak_explicit(p, old_value, new_value, succ_order, fail_order) \
	Dee_atomic_cmpxch_explicit(p, old_value, new_value, succ_order, fail_order)
#endif /* !Dee_atomic_cmpxch_weak_explicit */
#ifndef Dee_atomic_write_explicit
#define Dee_atomic_write_explicit(p, value, order) (void)Dee_atomic_xch_explicit(p, value, order)
#endif /* !Dee_atomic_write_explicit */
#ifndef Dee_atomic_inc_explicit
#define Dee_atomic_inc_explicit(p, order) (void)Dee_atomic_fetchinc_explicit(p, order)
#endif /* !Dee_atomic_inc_explicit */
#ifndef Dee_atomic_dec_explicit
#define Dee_atomic_dec_explicit(p, order) (void)Dee_atomic_fetchdec_explicit(p, order)
#endif /* !Dee_atomic_dec_explicit */
#ifndef Dee_atomic_add_explicit
#define Dee_atomic_add_explicit(p, value, order) (void)Dee_atomic_fetchadd_explicit(p, value, order)
#endif /* !Dee_atomic_add_explicit */
#ifndef Dee_atomic_sub_explicit
#define Dee_atomic_sub_explicit(p, value, order) (void)Dee_atomic_fetchsub_explicit(p, value, order)
#endif /* !Dee_atomic_sub_explicit */
#ifndef Dee_atomic_and_explicit
#define Dee_atomic_and_explicit(p, value, order) (void)Dee_atomic_fetchand_explicit(p, value, order)
#endif /* !Dee_atomic_and_explicit */
#ifndef Dee_atomic_or_explicit
#define Dee_atomic_or_explicit(p, value, order) (void)Dee_atomic_fetchor_explicit(p, value, order)
#endif /* !Dee_atomic_or_explicit */
#ifndef Dee_atomic_xor_explicit
#define Dee_atomic_xor_explicit(p, value, order) (void)Dee_atomic_fetchxor_explicit(p, value, order)
#endif /* !Dee_atomic_xor_explicit */
#ifndef Dee_atomic_nand_explicit
#define Dee_atomic_nand_explicit(p, value, order) (void)Dee_atomic_fetchnand_explicit(p, value, order)
#endif /* !Dee_atomic_nand_explicit */

#ifndef Dee_atomic_cmpxch_or_write
#define Dee_atomic_cmpxch_or_write Dee_atomic_cmpxch
#endif /* !Dee_atomic_cmpxch_or_write */
#ifndef Dee_atomic_cmpxch_weak_or_write
#define Dee_atomic_cmpxch_weak_or_write Dee_atomic_cmpxch_weak
#endif /* !Dee_atomic_cmpxch_weak_or_write */
#define Dee_atomic_read(p)                              Dee_atomic_read_explicit(p, Dee_ATOMIC_ACQUIRE)
#define Dee_atomic_write(p, value)                      Dee_atomic_write_explicit(p, value, Dee_ATOMIC_RELEASE)
#define Dee_atomic_xch(p, value)                        Dee_atomic_xch_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_cmpxch(p, old_value, new_value)      Dee_atomic_cmpxch_explicit(p, old_value, new_value, Dee_ATOMIC_SEQ_CST, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_cmpxch_weak(p, old_value, new_value) Dee_atomic_cmpxch_weak_explicit(p, old_value, new_value, Dee_ATOMIC_SEQ_CST, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_cmpxch_val(p, old_value, new_value)  Dee_atomic_cmpxch_val_explicit(p, old_value, new_value, Dee_ATOMIC_SEQ_CST, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_fetchinc(p)                          Dee_atomic_fetchinc_explicit(p, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_fetchdec(p)                          Dee_atomic_fetchdec_explicit(p, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_fetchadd(p, value)                   Dee_atomic_fetchadd_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_fetchsub(p, value)                   Dee_atomic_fetchsub_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_fetchand(p, value)                   Dee_atomic_fetchand_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_fetchor(p, value)                    Dee_atomic_fetchor_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_fetchxor(p, value)                   Dee_atomic_fetchxor_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_fetchnand(p, value)                  Dee_atomic_fetchnand_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_incfetch(p)                          Dee_atomic_incfetch_explicit(p, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_decfetch(p)                          Dee_atomic_decfetch_explicit(p, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_addfetch(p, value)                   Dee_atomic_addfetch_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_subfetch(p, value)                   Dee_atomic_subfetch_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_andfetch(p, value)                   Dee_atomic_andfetch_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_orfetch(p, value)                    Dee_atomic_orfetch_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_xorfetch(p, value)                   Dee_atomic_xorfetch_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_nandfetch(p, value)                  Dee_atomic_nandfetch_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_inc(p)                               Dee_atomic_inc_explicit(p, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_dec(p)                               Dee_atomic_dec_explicit(p, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_add(p, value)                        Dee_atomic_add_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_sub(p, value)                        Dee_atomic_sub_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_and(p, value)                        Dee_atomic_and_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_or(p, value)                         Dee_atomic_or_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_xor(p, value)                        Dee_atomic_xor_explicit(p, value, Dee_ATOMIC_SEQ_CST)
#define Dee_atomic_nand(p, value)                       Dee_atomic_nand_explicit(p, value, Dee_ATOMIC_SEQ_CST)

#ifdef DEE_SOURCE
#undef atomic_read
#undef atomic_write
#undef atomic_xch
#undef atomic_cmpxch
#undef atomic_cmpxch_weak
#undef atomic_cmpxch_val
#undef atomic_fetchinc
#undef atomic_fetchdec
#undef atomic_fetchadd
#undef atomic_fetchsub
#undef atomic_fetchand
#undef atomic_fetchor
#undef atomic_fetchxor
#undef atomic_fetchnand
#undef atomic_incfetch
#undef atomic_decfetch
#undef atomic_addfetch
#undef atomic_subfetch
#undef atomic_andfetch
#undef atomic_orfetch
#undef atomic_xorfetch
#undef atomic_nandfetch
#undef atomic_inc
#undef atomic_dec
#undef atomic_add
#undef atomic_sub
#undef atomic_and
#undef atomic_or
#undef atomic_xor
#undef atomic_nand
#undef atomic_thread_fence
#undef atomic_signal_fence
#undef atomic_read_explicit
#undef atomic_write_explicit
#undef atomic_xch_explicit
#undef atomic_cmpxch_explicit
#undef atomic_cmpxch_weak_explicit
#undef atomic_cmpxch_val_explicit
#undef atomic_fetchinc_explicit
#undef atomic_fetchdec_explicit
#undef atomic_fetchadd_explicit
#undef atomic_fetchsub_explicit
#undef atomic_fetchand_explicit
#undef atomic_fetchor_explicit
#undef atomic_fetchxor_explicit
#undef atomic_fetchnand_explicit
#undef atomic_incfetch_explicit
#undef atomic_decfetch_explicit
#undef atomic_addfetch_explicit
#undef atomic_subfetch_explicit
#undef atomic_andfetch_explicit
#undef atomic_orfetch_explicit
#undef atomic_xorfetch_explicit
#undef atomic_nandfetch_explicit
#undef atomic_inc_explicit
#undef atomic_dec_explicit
#undef atomic_add_explicit
#undef atomic_sub_explicit
#undef atomic_and_explicit
#undef atomic_or_explicit
#undef atomic_xor_explicit
#undef atomic_nand_explicit

#define atomic_read                 Dee_atomic_read
#define atomic_write                Dee_atomic_write
#define atomic_xch                  Dee_atomic_xch
#define atomic_cmpxch               Dee_atomic_cmpxch
#define atomic_cmpxch_or_write      Dee_atomic_cmpxch_or_write
#define atomic_cmpxch_weak          Dee_atomic_cmpxch_weak
#define atomic_cmpxch_weak_or_write Dee_atomic_cmpxch_weak_or_write
#define atomic_cmpxch_val           Dee_atomic_cmpxch_val
#define atomic_fetchinc             Dee_atomic_fetchinc
#define atomic_fetchdec             Dee_atomic_fetchdec
#define atomic_fetchadd             Dee_atomic_fetchadd
#define atomic_fetchsub             Dee_atomic_fetchsub
#define atomic_fetchand             Dee_atomic_fetchand
#define atomic_fetchor              Dee_atomic_fetchor
#define atomic_fetchxor             Dee_atomic_fetchxor
#define atomic_fetchnand            Dee_atomic_fetchnand
#define atomic_incfetch             Dee_atomic_incfetch
#define atomic_decfetch             Dee_atomic_decfetch
#define atomic_addfetch             Dee_atomic_addfetch
#define atomic_subfetch             Dee_atomic_subfetch
#define atomic_andfetch             Dee_atomic_andfetch
#define atomic_orfetch              Dee_atomic_orfetch
#define atomic_xorfetch             Dee_atomic_xorfetch
#define atomic_nandfetch            Dee_atomic_nandfetch
#define atomic_inc                  Dee_atomic_inc
#define atomic_dec                  Dee_atomic_dec
#define atomic_add                  Dee_atomic_add
#define atomic_sub                  Dee_atomic_sub
#define atomic_and                  Dee_atomic_and
#define atomic_or                   Dee_atomic_or
#define atomic_xor                  Dee_atomic_xor
#define atomic_nand                 Dee_atomic_nand
#define atomic_thread_fence         Dee_atomic_thread_fence
#define atomic_signal_fence         Dee_atomic_signal_fence
#define atomic_read_explicit        Dee_atomic_read_explicit
#define atomic_write_explicit       Dee_atomic_write_explicit
#define atomic_xch_explicit         Dee_atomic_xch_explicit
#define atomic_cmpxch_explicit      Dee_atomic_cmpxch_explicit
#define atomic_cmpxch_weak_explicit Dee_atomic_cmpxch_weak_explicit
#define atomic_cmpxch_val_explicit  Dee_atomic_cmpxch_val_explicit
#define atomic_fetchinc_explicit    Dee_atomic_fetchinc_explicit
#define atomic_fetchdec_explicit    Dee_atomic_fetchdec_explicit
#define atomic_fetchadd_explicit    Dee_atomic_fetchadd_explicit
#define atomic_fetchsub_explicit    Dee_atomic_fetchsub_explicit
#define atomic_fetchand_explicit    Dee_atomic_fetchand_explicit
#define atomic_fetchor_explicit     Dee_atomic_fetchor_explicit
#define atomic_fetchxor_explicit    Dee_atomic_fetchxor_explicit
#define atomic_fetchnand_explicit   Dee_atomic_fetchnand_explicit
#define atomic_incfetch_explicit    Dee_atomic_incfetch_explicit
#define atomic_decfetch_explicit    Dee_atomic_decfetch_explicit
#define atomic_addfetch_explicit    Dee_atomic_addfetch_explicit
#define atomic_subfetch_explicit    Dee_atomic_subfetch_explicit
#define atomic_andfetch_explicit    Dee_atomic_andfetch_explicit
#define atomic_orfetch_explicit     Dee_atomic_orfetch_explicit
#define atomic_xorfetch_explicit    Dee_atomic_xorfetch_explicit
#define atomic_nandfetch_explicit   Dee_atomic_nandfetch_explicit
#define atomic_inc_explicit         Dee_atomic_inc_explicit
#define atomic_dec_explicit         Dee_atomic_dec_explicit
#define atomic_add_explicit         Dee_atomic_add_explicit
#define atomic_sub_explicit         Dee_atomic_sub_explicit
#define atomic_and_explicit         Dee_atomic_and_explicit
#define atomic_or_explicit          Dee_atomic_or_explicit
#define atomic_xor_explicit         Dee_atomic_xor_explicit
#define atomic_nand_explicit        Dee_atomic_nand_explicit
#endif /* DEE_SOURCE */

#endif /* !GUARD_DEEMON_UTIL_ATOMIC_H */
