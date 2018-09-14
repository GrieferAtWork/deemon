/* Copyright (c) 2018 Griefer@Work                                            *
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
#define __GCC_PRIVATE_ARG_PLACEHOLDER_  ,
#define __GCC_PRIVATE_TAKE_SECOND_ARG_IMPL(x,val,...) val
#define __GCC_PRIVATE_TAKE_SECOND_ARG(x) __GCC_PRIVATE_TAKE_SECOND_ARG_IMPL x
#define __GCC_PRIVATE_IS_DEFINED3(x) __GCC_PRIVATE_TAKE_SECOND_ARG((x 1,0))
#define __GCC_PRIVATE_IS_DEFINED2(x) __GCC_PRIVATE_IS_DEFINED3(__GCC_PRIVATE_ARG_PLACEHOLDER_##x)
#define __GCC_PRIVATE_IS_DEFINED(x) __GCC_PRIVATE_IS_DEFINED2(x)

#ifdef __STDC__
#   define __P(x) x
#else
#   define __NO_PROTOTYPES 1
#   define __P(x) ()
#endif

#ifndef __INTEL_VERSION__
#ifdef __INTEL_COMPILER
#if __INTEL_COMPILER == 9999
#   define __INTEL_VERSION__ 1200
#else
#   define __INTEL_VERSION__ __INTEL_COMPILER
#endif
#elif defined(__ICL)
#   define __INTEL_VERSION__ __ICL
#elif defined(__ICC)
#   define __INTEL_VERSION__ __ICC
#elif defined(__ECC)
#   define __INTEL_VERSION__ __ECC
#endif
#endif /* !__INTEL_VERSION__ */


#ifndef __GNUC_MINOR__
#   define __GNUC_MINOR__ 0
#endif
#ifndef __GNUC_PATCH__
#ifdef __GNUC_PATCHLEVEL__
#   define __GNUC_PATCH__ __GNUC_PATCHLEVEL__
#else
#   define __GNUC_PATCH__ 0
#endif
#endif
#define __GCC_VERSION_NUM    (__GNUC__*10000+__GNUC_MINOR__*100+__GNUC_PATCH__)
#define __GCC_VERSION(a,b,c) (__GCC_VERSION_NUM >= ((a)*10000+(b)*100+(c)))


#ifndef __has_attribute
#if (defined(__i386__) || defined(__i386)) && \
    !defined(__x86_64__) && !defined(__x86_64)
#define __GCC_HAS_ATTRIBUTE___fastcall__
#define __GCC_HAS_ATTRIBUTE___stdcall__
#define __GCC_HAS_ATTRIBUTE___cdecl__
#endif
#if defined(__x86_64__) || defined(__x86_64)
#define __GCC_HAS_ATTRIBUTE___ms_abi__
#define __GCC_HAS_ATTRIBUTE___sysv_abi__
#endif
#if !defined(__ELF__) && (defined(__PE__) || defined(_WIN32) || defined(__CYGWIN__))
#define __GCC_HAS_ATTRIBUTE___dllimport__
#define __GCC_HAS_ATTRIBUTE___dllexport__
#endif
#define __GCC_HAS_ATTRIBUTE___warning__
#define __GCC_HAS_ATTRIBUTE___error__
#define __GCC_HAS_ATTRIBUTE___section__
#define __GCC_HAS_ATTRIBUTE___returns_nonnull__
#define __GCC_HAS_ATTRIBUTE___packed__
#define __GCC_HAS_ATTRIBUTE___alias__
#define __GCC_HAS_ATTRIBUTE___aligned__
#define __GCC_HAS_ATTRIBUTE___weak__
#define __GCC_HAS_ATTRIBUTE___returns_twice__
#define __GCC_HAS_ATTRIBUTE___externally_visible__
#define __GCC_HAS_ATTRIBUTE___visibility__
#if __GCC_VERSION(2,0,0) && !defined(__cplusplus)
#define __GCC_HAS_ATTRIBUTE___transparent_union__
#endif
#if __GCC_VERSION(2,3,0)
#define __GCC_HAS_ATTRIBUTE___format__
#endif
#if __GCC_VERSION(2,5,0)
#define __GCC_HAS_ATTRIBUTE___noreturn__
#define __GCC_HAS_ATTRIBUTE___const__
#endif
#if __GCC_VERSION(2,7,0)
#define __GCC_HAS_ATTRIBUTE___unused__
#endif
#if __GCC_VERSION(2,96,0)
#define __GCC_HAS_ATTRIBUTE___pure__
#endif
#if __GCC_VERSION(3,0,0) /* __GCC_VERSION(2,96,0) */
#define __GCC_HAS_ATTRIBUTE___malloc__
#endif
#if __GCC_VERSION(3,1,0)
#define __GCC_HAS_ATTRIBUTE___noinline__
#define __GCC_HAS_ATTRIBUTE___used__
#define __GCC_HAS_ATTRIBUTE___deprecated__ /*  - __GCC_VERSION(3,1,0)
                                            *  - __GCC_VERSION(3,2,0)
                                            *  - __GCC_VERSION(3,5,0)
                                            * The internet isn't unanimous about this one... */

#endif
#if __GCC_VERSION(3,3,0)
#define __GCC_HAS_ATTRIBUTE___nothrow__
#define __GCC_HAS_ATTRIBUTE___nonnull__
#define __GCC_HAS_ATTRIBUTE___warn_unused_result__ /* __GCC_VERSION(3,3,0) / __GCC_VERSION(3,4,0) */
#endif
#if __GCC_VERSION(3,5,0)
#define __GCC_HAS_ATTRIBUTE___sentinel__
#endif
#if __GCC_VERSION(4,3,0)
#define __GCC_HAS_ATTRIBUTE___alloc_size__
#define __GCC_HAS_ATTRIBUTE___hot__
#define __GCC_HAS_ATTRIBUTE___cold__
#endif
#if __GCC_VERSION(4,4,0)
#define __GCC_HAS_ATTRIBUTE___optimize__
#endif
#if __GCC_VERSION(4,5,0)
#define __GCC_HAS_ATTRIBUTE___noclone__
#endif
#if __GCC_VERSION(4,9,0)
#define __GCC_HAS_ATTRIBUTE___assume_aligned__
#endif
#if __GCC_VERSION(5,4,0)
#define __GCC_HAS_ATTRIBUTE___alloc_align__
#endif
#if __GCC_VERSION(7,0,0)
#define __GCC_HAS_ATTRIBUTE___fallthrough__
#endif
#define __has_attribute(x) __GCC_PRIVATE_IS_DEFINED(__GCC_HAS_ATTRIBUTE_##x)
#endif
#ifndef __has_cpp_attribute
#define __NO_has_cpp_attribute 1
#define __has_cpp_attribute(x) 0
#endif
#ifndef __has_feature
#define __NO_has_feature 1
#define __has_feature(x) 0
#endif
#ifndef __has_builtin
#define __GCC_HAS_BUILTIN___builtin_va_list
#define __GCC_HAS_BUILTIN___builtin_va_start
#define __GCC_HAS_BUILTIN___builtin_va_end
#define __GCC_HAS_BUILTIN___builtin_va_arg
#define __GCC_HAS_BUILTIN___builtin_va_copy
#define __GCC_HAS_BUILTIN___builtin_offsetof
#if __GCC_VERSION(8,1,0)
#define __GCC_HAS_BUILTIN___builtin_alloca_with_align_and_max  /* void *__builtin_alloca_with_align_and_max(size_t size, size_t alignment, size_t max_size) */
#define __GCC_HAS_BUILTIN___builtin_tgmath                     /* type __builtin_tgmath(functions, arguments) */
#define __GCC_HAS_BUILTIN___builtin_extend_pointer             /* Pmode __builtin_extend_pointer(void * x) */
#endif
#if __GCC_VERSION(7,3,0)
#define __GCC_HAS_BUILTIN___builtin_fabsfn
#define __GCC_HAS_BUILTIN___builtin_fabsfnx
#define __GCC_HAS_BUILTIN___builtin_copysignfn
#define __GCC_HAS_BUILTIN___builtin_copysignfnx
#endif
#if __GCC_VERSION(6,4,0)
#define __GCC_HAS_BUILTIN___builtin_clog10
#define __GCC_HAS_BUILTIN___builtin_clog10f
#define __GCC_HAS_BUILTIN___builtin_clog10l
#endif
#if __GCC_VERSION(5,5,0)
#define __GCC_HAS_BUILTIN___builtin_alloca_with_align          /* void *__builtin_alloca_with_align(size_t size, size_t alignment) */
#define __GCC_HAS_BUILTIN___builtin_call_with_static_chain     /* type __builtin_call_with_static_chain(call_exp, pointer_exp) */
#define __GCC_HAS_BUILTIN___builtin___bnd_set_ptr_bounds
#define __GCC_HAS_BUILTIN___builtin___bnd_narrow_ptr_bounds
#define __GCC_HAS_BUILTIN___builtin___bnd_copy_ptr_bounds
#define __GCC_HAS_BUILTIN___builtin___bnd_init_ptr_bounds
#define __GCC_HAS_BUILTIN___builtin___bnd_null_ptr_bounds
#define __GCC_HAS_BUILTIN___builtin___bnd_store_ptr_bounds
#define __GCC_HAS_BUILTIN___builtin___bnd_chk_ptr_lbounds
#define __GCC_HAS_BUILTIN___builtin___bnd_chk_ptr_ubounds
#define __GCC_HAS_BUILTIN___builtin___bnd_chk_ptr_bounds
#define __GCC_HAS_BUILTIN___builtin___bnd_get_ptr_lbound
#define __GCC_HAS_BUILTIN___builtin___bnd_get_ptr_ubound
#endif
#if __GCC_VERSION(4,8,5)
#define __GCC_HAS_BUILTIN___builtin_bswap16                    /* uint16_t __builtin_bswap16(uint16_t x) */
#define __GCC_HAS_BUILTIN___builtin_bswap32                    /* uint32_t __builtin_bswap32(uint32_t x) */
#define __GCC_HAS_BUILTIN___builtin_bswap64                    /* uint64_t __builtin_bswap64(uint64_t x) */
#define __GCC_HAS_BUILTIN___builtin_LINE                       /* int __builtin_LINE */
#define __GCC_HAS_BUILTIN___builtin_FUNCTION                   /* const char * __builtin_FUNCTION */
#define __GCC_HAS_BUILTIN___builtin_FILE                       /* const char * __builtin_FILE */
#endif
#if __GCC_VERSION(4,7,4)
#define __GCC_HAS_BUILTIN___builtin_complex                    /* type __builtin_complex(real, imag) */
#define __GCC_HAS_BUILTIN___builtin_assume_aligned             /* void * __builtin_assume_aligned(const void *exp, size_t align, ...) */
#define __GCC_HAS_BUILTIN___atomic_load_n
#define __GCC_HAS_BUILTIN___atomic_load
#define __GCC_HAS_BUILTIN___atomic_store_n
#define __GCC_HAS_BUILTIN___atomic_store
#define __GCC_HAS_BUILTIN___atomic_exchange_n
#define __GCC_HAS_BUILTIN___atomic_exchange
#define __GCC_HAS_BUILTIN___atomic_compare_exchange_n
#define __GCC_HAS_BUILTIN___atomic_compare_exchange
#define __GCC_HAS_BUILTIN___atomic_add_fetch
#define __GCC_HAS_BUILTIN___atomic_sub_fetch
#define __GCC_HAS_BUILTIN___atomic_and_fetch
#define __GCC_HAS_BUILTIN___atomic_xor_fetch
#define __GCC_HAS_BUILTIN___atomic_or_fetch
#define __GCC_HAS_BUILTIN___atomic_nand_fetch
#define __GCC_HAS_BUILTIN___atomic_fetch_add
#define __GCC_HAS_BUILTIN___atomic_fetch_sub
#define __GCC_HAS_BUILTIN___atomic_fetch_and
#define __GCC_HAS_BUILTIN___atomic_fetch_xor
#define __GCC_HAS_BUILTIN___atomic_fetch_or
#define __GCC_HAS_BUILTIN___atomic_fetch_nand
#define __GCC_HAS_BUILTIN___atomic_test_and_set
#define __GCC_HAS_BUILTIN___atomic_clear
#define __GCC_HAS_BUILTIN___atomic_thread_fence
#define __GCC_HAS_BUILTIN___atomic_signal_fence
#define __GCC_HAS_BUILTIN___atomic_always_lock_free
#define __GCC_HAS_BUILTIN___atomic_is_lock_free
#endif
#if __GCC_VERSION(4,5,4)
#define __GCC_HAS_BUILTIN___builtin_unreachable                /* void __builtin_unreachable(void) */
#endif
#if __GCC_VERSION(4,4,0)
#define __GCC_HAS_BUILTIN___builtin_fpclassify
#define __GCC_HAS_BUILTIN___builtin_isfinite
#define __GCC_HAS_BUILTIN___builtin_isnormal
#define __GCC_HAS_BUILTIN___builtin_isnan
#define __GCC_HAS_BUILTIN___builtin_isinf_sign
#define __GCC_HAS_BUILTIN___builtin_isinf
#endif
#if __GCC_VERSION(4,3,6)
#define __GCC_HAS_BUILTIN___builtin___clear_cache              /* void __builtin___clear_cache(char *begin, char *end) */
#define __GCC_HAS_BUILTIN___builtin_gammaf_r
#define __GCC_HAS_BUILTIN___builtin_gammal_r
#define __GCC_HAS_BUILTIN___builtin_gamma_r
#define __GCC_HAS_BUILTIN___builtin_lgammaf_r
#define __GCC_HAS_BUILTIN___builtin_lgammal_r
#define __GCC_HAS_BUILTIN___builtin_lgamma_r
#define __GCC_HAS_BUILTIN___builtin_signbitd32
#define __GCC_HAS_BUILTIN___builtin_signbitd64
#define __GCC_HAS_BUILTIN___builtin_signbitd128
#define __GCC_HAS_BUILTIN___builtin_memchr
#endif
#if __GCC_VERSION(4,3,0)
#define __GCC_HAS_BUILTIN___builtin_va_arg_pack
#define __GCC_HAS_BUILTIN___builtin_va_arg_pack_len
#endif
#if __GCC_VERSION(4,2,4)
#define __GCC_HAS_BUILTIN___builtin_trap                       /* void __builtin_trap(void) */
#endif
#if __GCC_VERSION(4,1,2)
#define __GCC_HAS_BUILTIN___builtin_stpncpy
#define __GCC_HAS_BUILTIN___builtin_strcasecmp
#define __GCC_HAS_BUILTIN___builtin_strncasecmp
#define __GCC_HAS_BUILTIN___builtin_strndup
#define __GCC_HAS_BUILTIN___builtin_clogf
#define __GCC_HAS_BUILTIN___builtin_clogl
#define __GCC_HAS_BUILTIN___builtin_clog
#define __GCC_HAS_BUILTIN___builtin_object_size
#define __GCC_HAS_BUILTIN___sync_fetch_and_add
#define __GCC_HAS_BUILTIN___sync_fetch_and_sub
#define __GCC_HAS_BUILTIN___sync_fetch_and_or
#define __GCC_HAS_BUILTIN___sync_fetch_and_and
#define __GCC_HAS_BUILTIN___sync_fetch_and_xor
#define __GCC_HAS_BUILTIN___sync_fetch_and_nand
#define __GCC_HAS_BUILTIN___sync_add_and_fetch
#define __GCC_HAS_BUILTIN___sync_sub_and_fetch
#define __GCC_HAS_BUILTIN___sync_or_and_fetch
#define __GCC_HAS_BUILTIN___sync_and_and_fetch
#define __GCC_HAS_BUILTIN___sync_xor_and_fetch
#define __GCC_HAS_BUILTIN___sync_nand_and_fetch
#define __GCC_HAS_BUILTIN___sync_bool_compare_and_swap
#define __GCC_HAS_BUILTIN___sync_val_compare_and_swap
#define __GCC_HAS_BUILTIN___sync_synchronize
#define __GCC_HAS_BUILTIN___sync_lock_test_and_set
#define __GCC_HAS_BUILTIN___sync_lock_release
#endif
#if __GCC_VERSION(4,0,4)
#define __GCC_HAS_BUILTIN___builtin_isascii
#define __GCC_HAS_BUILTIN___builtin_toascii
#define __GCC_HAS_BUILTIN___builtin_isblank
#define __GCC_HAS_BUILTIN___builtin_iswblank
#define __GCC_HAS_BUILTIN___builtin_iswalnum
#define __GCC_HAS_BUILTIN___builtin_iswalpha
#define __GCC_HAS_BUILTIN___builtin_iswcntrl
#define __GCC_HAS_BUILTIN___builtin_iswdigit
#define __GCC_HAS_BUILTIN___builtin_iswgraph
#define __GCC_HAS_BUILTIN___builtin_iswlower
#define __GCC_HAS_BUILTIN___builtin_iswprint
#define __GCC_HAS_BUILTIN___builtin_iswpunct
#define __GCC_HAS_BUILTIN___builtin_iswspace
#define __GCC_HAS_BUILTIN___builtin_iswupper
#define __GCC_HAS_BUILTIN___builtin_iswxdigit
#define __GCC_HAS_BUILTIN___builtin_towlower
#define __GCC_HAS_BUILTIN___builtin_towupper
#define __GCC_HAS_BUILTIN___builtin_isalnum
#define __GCC_HAS_BUILTIN___builtin_isalpha
#define __GCC_HAS_BUILTIN___builtin_iscntrl
#define __GCC_HAS_BUILTIN___builtin_isdigit
#define __GCC_HAS_BUILTIN___builtin_isgraph
#define __GCC_HAS_BUILTIN___builtin_islower
#define __GCC_HAS_BUILTIN___builtin_isprint
#define __GCC_HAS_BUILTIN___builtin_ispunct
#define __GCC_HAS_BUILTIN___builtin_isspace
#define __GCC_HAS_BUILTIN___builtin_isupper
#define __GCC_HAS_BUILTIN___builtin_isxdigit
#define __GCC_HAS_BUILTIN___builtin_tolower
#define __GCC_HAS_BUILTIN___builtin_toupper
#endif
#if __GCC_VERSION(4,0,0)
#define __GCC_HAS_BUILTIN___builtin_signbitf
#define __GCC_HAS_BUILTIN___builtin_signbit
#define __GCC_HAS_BUILTIN___builtin_signbitl
#endif
#if __GCC_VERSION(3,4,6)
#define __GCC_HAS_BUILTIN___builtin_ffsl                       /* int __builtin_ffsl(long) */
#define __GCC_HAS_BUILTIN___builtin_ffsll                      /* int __builtin_ffsll(long long) */
#define __GCC_HAS_BUILTIN___builtin_clz                        /* int __builtin_clz(unsigned int x) */
#define __GCC_HAS_BUILTIN___builtin_clzl                       /* int __builtin_clzl(unsigned long) */
#define __GCC_HAS_BUILTIN___builtin_clzll                      /* int __builtin_clzll(unsigned long long) */
#define __GCC_HAS_BUILTIN___builtin_ctz                        /* int __builtin_ctz(unsigned int x) */
#define __GCC_HAS_BUILTIN___builtin_ctzl                       /* int __builtin_ctzl(unsigned long) */
#define __GCC_HAS_BUILTIN___builtin_ctzll                      /* int __builtin_ctzll(unsigned long long) */
#define __GCC_HAS_BUILTIN___builtin_clrsb                      /* int __builtin_clrsb(int x) */
#define __GCC_HAS_BUILTIN___builtin_clrsbl                     /* int __builtin_clrsbl(long) */
#define __GCC_HAS_BUILTIN___builtin_clrsbll                    /* int __builtin_clrsbll(long long) */
#define __GCC_HAS_BUILTIN___builtin_popcount                   /* int __builtin_popcount(unsigned int x) */
#define __GCC_HAS_BUILTIN___builtin_popcountl                  /* int __builtin_popcountl(unsigned long) */
#define __GCC_HAS_BUILTIN___builtin_popcountll                 /* int __builtin_popcountll(unsigned long long) */
#define __GCC_HAS_BUILTIN___builtin_parity                     /* int __builtin_parity(unsigned int x) */
#define __GCC_HAS_BUILTIN___builtin_parityl                    /* int __builtin_parityl(unsigned long) */
#define __GCC_HAS_BUILTIN___builtin_parityll                   /* int __builtin_parityll(unsigned long long) */
#define __GCC_HAS_BUILTIN___builtin__exit
#define __GCC_HAS_BUILTIN___builtin_dcgettext
#define __GCC_HAS_BUILTIN___builtin_dgettext
#define __GCC_HAS_BUILTIN___builtin_dremf
#define __GCC_HAS_BUILTIN___builtin_dreml
#define __GCC_HAS_BUILTIN___builtin_drem
#define __GCC_HAS_BUILTIN___builtin_exp10f
#define __GCC_HAS_BUILTIN___builtin_exp10l
#define __GCC_HAS_BUILTIN___builtin_exp10
#define __GCC_HAS_BUILTIN___builtin_gammaf
#define __GCC_HAS_BUILTIN___builtin_gammal
#define __GCC_HAS_BUILTIN___builtin_gamma
#define __GCC_HAS_BUILTIN___builtin_gettext
#define __GCC_HAS_BUILTIN___builtin_j0f
#define __GCC_HAS_BUILTIN___builtin_j0l
#define __GCC_HAS_BUILTIN___builtin_j0
#define __GCC_HAS_BUILTIN___builtin_j1f
#define __GCC_HAS_BUILTIN___builtin_j1l
#define __GCC_HAS_BUILTIN___builtin_j1
#define __GCC_HAS_BUILTIN___builtin_jnf
#define __GCC_HAS_BUILTIN___builtin_jnl
#define __GCC_HAS_BUILTIN___builtin_jn
#define __GCC_HAS_BUILTIN___builtin_mempcpy
#define __GCC_HAS_BUILTIN___builtin_pow10f
#define __GCC_HAS_BUILTIN___builtin_pow10l
#define __GCC_HAS_BUILTIN___builtin_pow10
#define __GCC_HAS_BUILTIN___builtin_scalbf
#define __GCC_HAS_BUILTIN___builtin_scalbl
#define __GCC_HAS_BUILTIN___builtin_scalb
#define __GCC_HAS_BUILTIN___builtin_significandf
#define __GCC_HAS_BUILTIN___builtin_significandl
#define __GCC_HAS_BUILTIN___builtin_significand
#define __GCC_HAS_BUILTIN___builtin_sincosf
#define __GCC_HAS_BUILTIN___builtin_sincosl
#define __GCC_HAS_BUILTIN___builtin_sincos
#define __GCC_HAS_BUILTIN___builtin_stpcpy
#define __GCC_HAS_BUILTIN___builtin_strdup
#define __GCC_HAS_BUILTIN___builtin_strfmon
#define __GCC_HAS_BUILTIN___builtin_y0f
#define __GCC_HAS_BUILTIN___builtin_y0l
#define __GCC_HAS_BUILTIN___builtin_y0
#define __GCC_HAS_BUILTIN___builtin_y1f
#define __GCC_HAS_BUILTIN___builtin_y1l
#define __GCC_HAS_BUILTIN___builtin_y1
#define __GCC_HAS_BUILTIN___builtin_ynf
#define __GCC_HAS_BUILTIN___builtin_ynl
#define __GCC_HAS_BUILTIN___builtin_yn 
#define __GCC_HAS_BUILTIN___builtin__Exit
#define __GCC_HAS_BUILTIN___builtin_acoshf
#define __GCC_HAS_BUILTIN___builtin_acoshl
#define __GCC_HAS_BUILTIN___builtin_acosh
#define __GCC_HAS_BUILTIN___builtin_asinhf
#define __GCC_HAS_BUILTIN___builtin_asinhl
#define __GCC_HAS_BUILTIN___builtin_asinh
#define __GCC_HAS_BUILTIN___builtin_atanhf
#define __GCC_HAS_BUILTIN___builtin_atanhl
#define __GCC_HAS_BUILTIN___builtin_atanh
#define __GCC_HAS_BUILTIN___builtin_cabsf
#define __GCC_HAS_BUILTIN___builtin_cabsl
#define __GCC_HAS_BUILTIN___builtin_cabs
#define __GCC_HAS_BUILTIN___builtin_cacosf
#define __GCC_HAS_BUILTIN___builtin_cacoshf
#define __GCC_HAS_BUILTIN___builtin_cacoshl
#define __GCC_HAS_BUILTIN___builtin_cacosh
#define __GCC_HAS_BUILTIN___builtin_cacosl
#define __GCC_HAS_BUILTIN___builtin_cacos
#define __GCC_HAS_BUILTIN___builtin_cargf
#define __GCC_HAS_BUILTIN___builtin_cargl
#define __GCC_HAS_BUILTIN___builtin_carg
#define __GCC_HAS_BUILTIN___builtin_casinf
#define __GCC_HAS_BUILTIN___builtin_casinhf
#define __GCC_HAS_BUILTIN___builtin_casinhl
#define __GCC_HAS_BUILTIN___builtin_casinh
#define __GCC_HAS_BUILTIN___builtin_casinl
#define __GCC_HAS_BUILTIN___builtin_casin
#define __GCC_HAS_BUILTIN___builtin_catanf
#define __GCC_HAS_BUILTIN___builtin_catanhf
#define __GCC_HAS_BUILTIN___builtin_catanhl
#define __GCC_HAS_BUILTIN___builtin_catanh
#define __GCC_HAS_BUILTIN___builtin_catanl
#define __GCC_HAS_BUILTIN___builtin_catan
#define __GCC_HAS_BUILTIN___builtin_cbrtf
#define __GCC_HAS_BUILTIN___builtin_cbrtl
#define __GCC_HAS_BUILTIN___builtin_cbrt
#define __GCC_HAS_BUILTIN___builtin_ccosf
#define __GCC_HAS_BUILTIN___builtin_ccoshf
#define __GCC_HAS_BUILTIN___builtin_ccoshl
#define __GCC_HAS_BUILTIN___builtin_ccosh
#define __GCC_HAS_BUILTIN___builtin_ccosl
#define __GCC_HAS_BUILTIN___builtin_ccos
#define __GCC_HAS_BUILTIN___builtin_cexpf
#define __GCC_HAS_BUILTIN___builtin_cexpl
#define __GCC_HAS_BUILTIN___builtin_cexp
#define __GCC_HAS_BUILTIN___builtin_copysignf
#define __GCC_HAS_BUILTIN___builtin_copysignl
#define __GCC_HAS_BUILTIN___builtin_copysign
#define __GCC_HAS_BUILTIN___builtin_cpowf
#define __GCC_HAS_BUILTIN___builtin_cpowl
#define __GCC_HAS_BUILTIN___builtin_cpow
#define __GCC_HAS_BUILTIN___builtin_cprojf
#define __GCC_HAS_BUILTIN___builtin_cprojl
#define __GCC_HAS_BUILTIN___builtin_cproj
#define __GCC_HAS_BUILTIN___builtin_csinf
#define __GCC_HAS_BUILTIN___builtin_csinhf
#define __GCC_HAS_BUILTIN___builtin_csinhl
#define __GCC_HAS_BUILTIN___builtin_csinh
#define __GCC_HAS_BUILTIN___builtin_csinl
#define __GCC_HAS_BUILTIN___builtin_csin
#define __GCC_HAS_BUILTIN___builtin_csqrtf
#define __GCC_HAS_BUILTIN___builtin_csqrtl
#define __GCC_HAS_BUILTIN___builtin_csqrt
#define __GCC_HAS_BUILTIN___builtin_ctanf
#define __GCC_HAS_BUILTIN___builtin_ctanhf
#define __GCC_HAS_BUILTIN___builtin_ctanhl
#define __GCC_HAS_BUILTIN___builtin_ctanh
#define __GCC_HAS_BUILTIN___builtin_ctanl
#define __GCC_HAS_BUILTIN___builtin_ctan
#define __GCC_HAS_BUILTIN___builtin_erfcf
#define __GCC_HAS_BUILTIN___builtin_erfcl
#define __GCC_HAS_BUILTIN___builtin_erfc
#define __GCC_HAS_BUILTIN___builtin_erff
#define __GCC_HAS_BUILTIN___builtin_erfl
#define __GCC_HAS_BUILTIN___builtin_erf
#define __GCC_HAS_BUILTIN___builtin_exp2f
#define __GCC_HAS_BUILTIN___builtin_exp2l
#define __GCC_HAS_BUILTIN___builtin_exp2
#define __GCC_HAS_BUILTIN___builtin_expm1f
#define __GCC_HAS_BUILTIN___builtin_expm1l
#define __GCC_HAS_BUILTIN___builtin_expm1
#define __GCC_HAS_BUILTIN___builtin_fdimf
#define __GCC_HAS_BUILTIN___builtin_fdiml
#define __GCC_HAS_BUILTIN___builtin_fdim
#define __GCC_HAS_BUILTIN___builtin_fmaf
#define __GCC_HAS_BUILTIN___builtin_fmal
#define __GCC_HAS_BUILTIN___builtin_fmaxf
#define __GCC_HAS_BUILTIN___builtin_fmaxl
#define __GCC_HAS_BUILTIN___builtin_fmax
#define __GCC_HAS_BUILTIN___builtin_fma
#define __GCC_HAS_BUILTIN___builtin_fminf
#define __GCC_HAS_BUILTIN___builtin_fminl
#define __GCC_HAS_BUILTIN___builtin_fmin
#define __GCC_HAS_BUILTIN___builtin_hypotf
#define __GCC_HAS_BUILTIN___builtin_hypotl
#define __GCC_HAS_BUILTIN___builtin_hypot
#define __GCC_HAS_BUILTIN___builtin_ilogbf
#define __GCC_HAS_BUILTIN___builtin_ilogbl
#define __GCC_HAS_BUILTIN___builtin_ilogb
#define __GCC_HAS_BUILTIN___builtin_lgammaf
#define __GCC_HAS_BUILTIN___builtin_lgammal
#define __GCC_HAS_BUILTIN___builtin_lgamma
#define __GCC_HAS_BUILTIN___builtin_llrintf
#define __GCC_HAS_BUILTIN___builtin_llrintl
#define __GCC_HAS_BUILTIN___builtin_llrint
#define __GCC_HAS_BUILTIN___builtin_llroundf
#define __GCC_HAS_BUILTIN___builtin_llroundl
#define __GCC_HAS_BUILTIN___builtin_llround
#define __GCC_HAS_BUILTIN___builtin_log1pf
#define __GCC_HAS_BUILTIN___builtin_log1pl
#define __GCC_HAS_BUILTIN___builtin_log1p
#define __GCC_HAS_BUILTIN___builtin_log2f
#define __GCC_HAS_BUILTIN___builtin_log2l
#define __GCC_HAS_BUILTIN___builtin_log2
#define __GCC_HAS_BUILTIN___builtin_logbf
#define __GCC_HAS_BUILTIN___builtin_logbl
#define __GCC_HAS_BUILTIN___builtin_logb
#define __GCC_HAS_BUILTIN___builtin_lrintf
#define __GCC_HAS_BUILTIN___builtin_lrintl
#define __GCC_HAS_BUILTIN___builtin_lrint
#define __GCC_HAS_BUILTIN___builtin_lroundf
#define __GCC_HAS_BUILTIN___builtin_lroundl
#define __GCC_HAS_BUILTIN___builtin_lround
#define __GCC_HAS_BUILTIN___builtin_nearbyintf
#define __GCC_HAS_BUILTIN___builtin_nearbyintl
#define __GCC_HAS_BUILTIN___builtin_nearbyint
#define __GCC_HAS_BUILTIN___builtin_nextafterf
#define __GCC_HAS_BUILTIN___builtin_nextafterl
#define __GCC_HAS_BUILTIN___builtin_nextafter
#define __GCC_HAS_BUILTIN___builtin_nexttowardf
#define __GCC_HAS_BUILTIN___builtin_nexttowardl
#define __GCC_HAS_BUILTIN___builtin_nexttoward
#define __GCC_HAS_BUILTIN___builtin_remainderf
#define __GCC_HAS_BUILTIN___builtin_remainderl
#define __GCC_HAS_BUILTIN___builtin_remainder
#define __GCC_HAS_BUILTIN___builtin_remquof
#define __GCC_HAS_BUILTIN___builtin_remquol
#define __GCC_HAS_BUILTIN___builtin_remquo
#define __GCC_HAS_BUILTIN___builtin_rintf
#define __GCC_HAS_BUILTIN___builtin_rintl
#define __GCC_HAS_BUILTIN___builtin_rint
#define __GCC_HAS_BUILTIN___builtin_roundf
#define __GCC_HAS_BUILTIN___builtin_roundl
#define __GCC_HAS_BUILTIN___builtin_round
#define __GCC_HAS_BUILTIN___builtin_scalblnf
#define __GCC_HAS_BUILTIN___builtin_scalblnl
#define __GCC_HAS_BUILTIN___builtin_scalbln
#define __GCC_HAS_BUILTIN___builtin_scalbnf
#define __GCC_HAS_BUILTIN___builtin_scalbnl
#define __GCC_HAS_BUILTIN___builtin_scalbn
#define __GCC_HAS_BUILTIN___builtin_tgammaf
#define __GCC_HAS_BUILTIN___builtin_tgammal
#define __GCC_HAS_BUILTIN___builtin_tgamma
#define __GCC_HAS_BUILTIN___builtin_truncf
#define __GCC_HAS_BUILTIN___builtin_truncl
#define __GCC_HAS_BUILTIN___builtin_trunc
#define __GCC_HAS_BUILTIN___builtin_vfscanf
#define __GCC_HAS_BUILTIN___builtin_vscanf
#define __GCC_HAS_BUILTIN___builtin_vsnprintf 
#define __GCC_HAS_BUILTIN___builtin_vsscanf 
#define __GCC_HAS_BUILTIN___builtin_abort
#define __GCC_HAS_BUILTIN___builtin_acos
#define __GCC_HAS_BUILTIN___builtin_asin
#define __GCC_HAS_BUILTIN___builtin_atan2
#define __GCC_HAS_BUILTIN___builtin_atan
#define __GCC_HAS_BUILTIN___builtin_calloc
#define __GCC_HAS_BUILTIN___builtin_ceil
#define __GCC_HAS_BUILTIN___builtin_cosh
#define __GCC_HAS_BUILTIN___builtin_exit
#define __GCC_HAS_BUILTIN___builtin_floor
#define __GCC_HAS_BUILTIN___builtin_fmod
#define __GCC_HAS_BUILTIN___builtin_frexp
#define __GCC_HAS_BUILTIN___builtin_fscanf
#define __GCC_HAS_BUILTIN___builtin_ldexp
#define __GCC_HAS_BUILTIN___builtin_log10
#define __GCC_HAS_BUILTIN___builtin_malloc
#define __GCC_HAS_BUILTIN___builtin_modf
#define __GCC_HAS_BUILTIN___builtin_pow
#define __GCC_HAS_BUILTIN___builtin_sinh
#define __GCC_HAS_BUILTIN___builtin_tanh
#define __GCC_HAS_BUILTIN___builtin_tan
#define __GCC_HAS_BUILTIN___builtin_vfprintf
#endif
#if __GCC_VERSION(3,3,6)
#define __GCC_HAS_BUILTIN___builtin_huge_val                   /* double __builtin_huge_val(void) */
#define __GCC_HAS_BUILTIN___builtin_huge_valf                  /* float __builtin_huge_valf(void) */
#define __GCC_HAS_BUILTIN___builtin_huge_vall                  /* long double __builtin_huge_vall(void) */
#define __GCC_HAS_BUILTIN___builtin_huge_valfn                 /* _Floatn __builtin_huge_valfn(void) */
#define __GCC_HAS_BUILTIN___builtin_huge_valfnx                /* _Floatnx __builtin_huge_valfnx(void) */
#define __GCC_HAS_BUILTIN___builtin_inf                        /* double __builtin_inf(void) */
#define __GCC_HAS_BUILTIN___builtin_infd32                     /* _Decimal32 __builtin_infd32(void) */
#define __GCC_HAS_BUILTIN___builtin_infd64                     /* _Decimal64 __builtin_infd64(void) */
#define __GCC_HAS_BUILTIN___builtin_infd128                    /* _Decimal128 __builtin_infd128(void) */
#define __GCC_HAS_BUILTIN___builtin_inff                       /* float __builtin_inff(void) */
#define __GCC_HAS_BUILTIN___builtin_infl                       /* long double __builtin_infl(void) */
#define __GCC_HAS_BUILTIN___builtin_inffn                      /* _Floatn __builtin_inffn(void) */
#define __GCC_HAS_BUILTIN___builtin_inffnx                     /* _Floatn __builtin_inffnx(void) */
#define __GCC_HAS_BUILTIN___builtin_nan                        /* double __builtin_nan(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nand32                     /* _Decimal32 __builtin_nand32(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nand64                     /* _Decimal64 __builtin_nand64(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nand128                    /* _Decimal128 __builtin_nand128(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nanf                       /* float __builtin_nanf(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nanl                       /* long double __builtin_nanl(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nanfn                      /* _Floatn __builtin_nanfn(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nanfnx                     /* _Floatnx __builtin_nanfnx(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nans                       /* double __builtin_nans(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nansf                      /* float __builtin_nansf(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nansl                      /* long double __builtin_nansl(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nansfn                     /* _Floatn __builtin_nansfn(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_nansfnx                    /* _Floatnx __builtin_nansfnx(const char *str) */
#define __GCC_HAS_BUILTIN___builtin_powi                       /* double __builtin_powi(double, int) */
#define __GCC_HAS_BUILTIN___builtin_powif                      /* float __builtin_powif(float, int) */
#define __GCC_HAS_BUILTIN___builtin_powil                      /* long double __builtin_powil(long double, int) */
#define __GCC_HAS_BUILTIN___builtin_exp
#define __GCC_HAS_BUILTIN___builtin_log
#define __GCC_HAS_BUILTIN___builtin_putchar
#define __GCC_HAS_BUILTIN___builtin_puts
#define __GCC_HAS_BUILTIN___builtin_scanf
#define __GCC_HAS_BUILTIN___builtin_snprintf
#define __GCC_HAS_BUILTIN___builtin_sprintf
#define __GCC_HAS_BUILTIN___builtin_sscanf
#define __GCC_HAS_BUILTIN___builtin_vprintf
#define __GCC_HAS_BUILTIN___builtin_vsprintf
#endif
#if __GCC_VERSION(3,1,1)
#define __GCC_HAS_BUILTIN___builtin_types_compatible_p         /* int __builtin_types_compatible_p(type1, type2) */
#define __GCC_HAS_BUILTIN___builtin_choose_expr                /* type __builtin_choose_expr(const_exp, exp1, exp2) */
#define __GCC_HAS_BUILTIN___builtin_prefetch                   /* void __builtin_prefetch(const void *addr, ...) */
#define __GCC_HAS_BUILTIN___builtin_fputs_unlocked
#define __GCC_HAS_BUILTIN___builtin_printf_unlocked
#define __GCC_HAS_BUILTIN___builtin_fprintf_unlocked 
#endif
#if __GCC_VERSION(3,0,4) /* These may have appeared earlier, but documentation is hard to find... */
#define __GCC_HAS_BUILTIN___builtin_conj
#define __GCC_HAS_BUILTIN___builtin_conjf
#define __GCC_HAS_BUILTIN___builtin_conjl
#define __GCC_HAS_BUILTIN___builtin_creal
#define __GCC_HAS_BUILTIN___builtin_crealf
#define __GCC_HAS_BUILTIN___builtin_creall
#define __GCC_HAS_BUILTIN___builtin_cimag
#define __GCC_HAS_BUILTIN___builtin_cimagf
#define __GCC_HAS_BUILTIN___builtin_cimagl
#define __GCC_HAS_BUILTIN___builtin_llabs
#define __GCC_HAS_BUILTIN___builtin_imaxabs
#endif
#if __GCC_VERSION(3,0,4) /* These may have appeared earlier, but documentation is hard to find... */
#define __GCC_HAS_BUILTIN___builtin_abs
#define __GCC_HAS_BUILTIN___builtin_cos
#define __GCC_HAS_BUILTIN___builtin_fabs
#define __GCC_HAS_BUILTIN___builtin_fprintf
#define __GCC_HAS_BUILTIN___builtin_fputs
#define __GCC_HAS_BUILTIN___builtin_labs
#define __GCC_HAS_BUILTIN___builtin_memcmp
#define __GCC_HAS_BUILTIN___builtin_memcpy
#define __GCC_HAS_BUILTIN___builtin_memset
#define __GCC_HAS_BUILTIN___builtin_printf
#define __GCC_HAS_BUILTIN___builtin_sin
#define __GCC_HAS_BUILTIN___builtin_sqrt
#define __GCC_HAS_BUILTIN___builtin_strcat
#define __GCC_HAS_BUILTIN___builtin_strchr
#define __GCC_HAS_BUILTIN___builtin_strcmp
#define __GCC_HAS_BUILTIN___builtin_strcpy
#define __GCC_HAS_BUILTIN___builtin_strcspn
#define __GCC_HAS_BUILTIN___builtin_strlen
#define __GCC_HAS_BUILTIN___builtin_strncat
#define __GCC_HAS_BUILTIN___builtin_strncmp
#define __GCC_HAS_BUILTIN___builtin_strncpy
#define __GCC_HAS_BUILTIN___builtin_strpbrk
#define __GCC_HAS_BUILTIN___builtin_strrchr
#define __GCC_HAS_BUILTIN___builtin_strspn
#define __GCC_HAS_BUILTIN___builtin_strstr
#endif

/* The oldest documentation referring to these builtins is this:
 * >> http://www.jklp.org/~chiefdigger/info/gnu/gcc/gcc_112.html
 * Which was for `__GCC_VERSION(1,1,2)' */
#define __GCC_HAS_BUILTIN___builtin_return_address
#define __GCC_HAS_BUILTIN___builtin_frame_address
#define __GCC_HAS_BUILTIN___builtin_constant_p                 /* int __builtin_constant_p(exp) */
#define __GCC_HAS_BUILTIN___builtin_apply_args
#define __GCC_HAS_BUILTIN___builtin_apply
#define __GCC_HAS_BUILTIN___builtin_return
#if !defined(__clang__) && (!defined(__INTEL_VERSION__) || __INTEL_VERSION__ >= 800)
#define __GCC_HAS_BUILTIN___builtin_expect                     /* long __builtin_expect(long exp, long c) */
#endif
#define __GCC_HAS_BUILTIN___builtin_alloca                     /* void *__builtin_alloca(size_t size) */
#define __GCC_HAS_BUILTIN___builtin_ffs                        /* int __builtin_ffs(int x) */
#define __GCC_HAS_BUILTIN___builtin_bcmp
#define __GCC_HAS_BUILTIN___builtin_bzero
#define __GCC_HAS_BUILTIN___builtin_index
#define __GCC_HAS_BUILTIN___builtin_rindex
#define __GCC_HAS_BUILTIN___builtin_cosf
#define __GCC_HAS_BUILTIN___builtin_cosl
#define __GCC_HAS_BUILTIN___builtin_fabsf
#define __GCC_HAS_BUILTIN___builtin_fabsl
#define __GCC_HAS_BUILTIN___builtin_sinf
#define __GCC_HAS_BUILTIN___builtin_sinl
#define __GCC_HAS_BUILTIN___builtin_sqrtf
#define __GCC_HAS_BUILTIN___builtin_sqrtl
#define __GCC_HAS_BUILTIN___builtin_isunordered
#define __GCC_HAS_BUILTIN___builtin_isgreater
#define __GCC_HAS_BUILTIN___builtin_isgreaterequal
#define __GCC_HAS_BUILTIN___builtin_isless
#define __GCC_HAS_BUILTIN___builtin_islessequal
#define __GCC_HAS_BUILTIN___builtin_islessgreater

/* Emulate `__has_builtin()' for older GCC versions. */
#define __has_builtin(x) __GCC_PRIVATE_IS_DEFINED(__GCC_HAS_BUILTIN_##x)
#endif /* !__has_builtin */

#ifndef __likely
#if __has_builtin(__builtin_expect)
#   define __likely(x)   (__builtin_expect(!!(x),1))
#   define __unlikely(x) (__builtin_expect(!!(x),0))
#else
#   define __builtin_expect(x,y) (x)
#   define __NO_builtin_expect 1
#   define __likely      /* Nothing */
#   define __unlikely    /* Nothing */
#endif
#endif /* !__likely */

#if defined(__clang__) || !defined(__DARWIN_NO_LONG_LONG)
#define __COMPILER_HAVE_LONGLONG 1
#endif
#define __COMPILER_HAVE_LONGDOUBLE 1
#define __COMPILER_HAVE_TRANSPARENT_STRUCT 1
#define __COMPILER_HAVE_TRANSPARENT_UNION 1
#define __COMPILER_HAVE_PRAGMA_PUSHMACRO 1
#if __has_feature(__tpp_pragma_deprecated__)
#define __COMPILER_HAVE_PRAGMA_DEPRECATED 1
#endif
#ifdef __CC__
#define __COMPILER_HAVE_PRAGMA_PACK 1
#endif
#define __COMPILER_HAVE_GCC_ASM 1
#ifdef __cplusplus
#define __COMPILER_ASM_BUFFER(T,s,p) (*(T(*)[s])(p))
#else
#define __COMPILER_ASM_BUFFER(T,s,p) (*(struct { __extension__ T __d[s]; } *)(p))
#endif

#ifdef __CPROTO__
#include "cproto.h"
#endif

#if 1
/* XXX: When was this added in C? */
#   define __COMPILER_HAVE_AUTOTYPE 1
#elif __has_feature(cxx_auto_type) || \
     (defined(__cplusplus) && __GCC_VERSION(4,4,0))
#     define __auto_type              auto
#     define __COMPILER_HAVE_AUTOTYPE 1
#endif

#if __has_feature(cxx_static_assert) || \
   (__GCC_VERSION(4,3,0) && (defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L))
#   define __STATIC_ASSERT(expr)         static_assert(expr,#expr)
#   define __STATIC_ASSERT_MSG(expr,msg) static_assert(expr,msg)
#elif defined(_Static_assert) || __has_feature(c_static_assert) || \
     (!defined(__cplusplus) && ( \
     (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 201112L) || \
     (__GCC_VERSION(4,6,0) && !defined(__STRICT_ANSI__))))
#   define __STATIC_ASSERT(expr)         _Static_assert(expr,#expr)
#   define __STATIC_ASSERT_MSG(expr,msg) _Static_assert(expr,msg)
#elif defined(__TPP_COUNTER)
#   define __STATIC_ASSERT(expr)         extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__TPP_COUNTER(__static_assert))[(expr)?1:-1]
#   define __STATIC_ASSERT_MSG(expr,msg) extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__TPP_COUNTER(__static_assert))[(expr)?1:-1]
#elif defined(__COUNTER__)
#   define __STATIC_ASSERT(expr)         extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__COUNTER__)[(expr)?1:-1]
#   define __STATIC_ASSERT_MSG(expr,msg) extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__COUNTER__)[(expr)?1:-1]
#else
#   define __STATIC_ASSERT(expr)         extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__LINE__)[(expr)?1:-1]
#   define __STATIC_ASSERT_MSG(expr,msg) extern __ATTR_UNUSED int __PP_CAT2(__static_assert_,__LINE__)[(expr)?1:-1]
#endif
#ifdef __INTELLISENSE__
#   define __ASMNAME(x)   /* Nothing */
#else
#   define __ASMNAME(x)   __asm__(x)
#endif
//#define __NO_ASMNAME 1 /* TO-DO: Remove me */
#if !__GCC_VERSION(2,7,0)
#ifndef __attribute__
#   define __attribute__(x) /* Nothing */
#endif /* !__attribute__ */
#endif
#if !__GCC_VERSION(2,8,0)
#ifndef __extension__
#   define __extension__
#endif /* !__extension__ */
#endif
#define __COMPILER_HAVE_TYPEOF 1
#if __GCC_VERSION(3,1,0)
#   define __ATTR_NOINLINE         __attribute__((__noinline__))
#else
#   define __NO_ATTR_NOINLINE      1
#   define __ATTR_NOINLINE         /* Nothing */
#endif
#if __GCC_VERSION(2,5,0)
#   define __ATTR_NORETURN         __attribute__((__noreturn__))
#else
#   define __NO_ATTR_NORETURN      1
#   define __ATTR_NORETURN         /* Nothing */
#endif
#if defined(__cplusplus) && __has_cpp_attribute(fallthrough)
#define __ATTR_FALLTHROUGH         [[fallthrough]];
#elif __has_attribute(fallthrough)
#define __ATTR_FALLTHROUGH         __attribute__((__fallthrough__));
#else
#define __NO_ATTR_FALLTHROUGH      1
#define __ATTR_FALLTHROUGH         /* Nothing */
#endif
#define __NO_ATTR_W64              1
#define __ATTR_W64                 /* Nothing */
#if (defined(__i386__) || defined(__i386)) && \
    !defined(__x86_64__) && !defined(__x86_64)
#   define __ATTR_FASTCALL         __attribute__((__fastcall__))
#   define __ATTR_STDCALL          __attribute__((__stdcall__))
#if !defined(__INTELLISENSE__)
#   define __ATTR_CDECL            /* [default] */
#else
#   define __ATTR_CDECL            __attribute__((__cdecl__))
#endif
#else
#   define __NO_ATTR_FASTCALL      1
#   define __ATTR_FASTCALL         /* Nothing */
#   define __NO_ATTR_STDCALL       1
#   define __ATTR_STDCALL          /* Nothing */
#   define __NO_ATTR_CDECL         1
#   define __ATTR_CDECL            /* Nothing */
#endif
#if defined(__x86_64__) || defined(__x86_64)
#   define __VA_LIST_IS_ARRAY      1
#   define __ATTR_MSABI            __attribute__((__ms_abi__))
#   define __ATTR_SYSVABI          __attribute__((__sysv_abi__))
#else
#   define __NO_ATTR_MSABI         1
#   define __ATTR_MSABI            /* Nothing */
#   define __NO_ATTR_SYSVABI       1
#   define __ATTR_SYSVABI          /* Nothing */
#endif
#if __has_attribute(__pure__)
#   define __ATTR_PURE             __attribute__((__pure__))
#else
#   define __NO_ATTR_PURE          1
#   define __ATTR_PURE             /* Nothing */
#endif
#if __has_attribute(__const__)
#   define __ATTR_CONST            __attribute__ ((__const__))
#else
#   define __NO_ATTR_CONST         1
#   define __ATTR_CONST            /* Nothing */
#endif
#if __has_attribute(__malloc__)
#   define __ATTR_MALLOC           __attribute__((__malloc__))
#else
#   define __NO_ATTR_MALLOC        1
#   define __ATTR_MALLOC           /* Nothing */
#endif
#if __has_attribute(__alloc_size__)
#   define __ATTR_ALLOC_SIZE(ppars) __attribute__((__alloc_size__ ppars))
#else
#   define __NO_ATTR_ALLOC_SIZE     1
#   define __ATTR_ALLOC_SIZE(ppars) /* Nothing */
#endif
#if __has_attribute(__unused__)
#   define __ATTR_UNUSED           __attribute__((__unused__))
#else
#   define __NO_ATTR_UNUSED        1
#   define __ATTR_UNUSED           /* Nothing */
#endif
#if __has_attribute(__used__)
#   define __ATTR_USED             __attribute__((__used__))
#else
#   define __NO_ATTR_USED          1
#   define __ATTR_USED             /* Nothing */
#endif
#ifdef __INTELLISENSE__
#   define __ATTR_DEPRECATED_      __declspec(deprecated)
#   define __ATTR_DEPRECATED(text) __declspec(deprecated(text))
#elif __has_attribute(__deprecated__)
#   define __ATTR_DEPRECATED_      __attribute__((__deprecated__))
#if __GCC_VERSION(4,5,0)
#   define __ATTR_DEPRECATED(text) __attribute__((__deprecated__(text)))
#else
#   define __ATTR_DEPRECATED(text) __attribute__((__deprecated__))
#endif
#else
#   define __NO_ATTR_DEPRECATED    1
#   define __ATTR_DEPRECATED_      /* Nothing */
#   define __ATTR_DEPRECATED(text) /* Nothing */
#endif
#if __has_attribute(__sentinel__)
#   define __ATTR_SENTINEL         __attribute__((__sentinel__))
#ifdef __INTELLISENSE__
#   define __ATTR_SENTINEL_O(x)    __attribute__((__sentinel__))
#else
#   define __ATTR_SENTINEL_O(x)    __attribute__((__sentinel__(x)))
#endif
#else
#   define __NO_ATTR_SENTINEL      1
#   define __NO_ATTR_SENTINEL_O    1
#   define __ATTR_SENTINEL         /* Nothing */
#   define __ATTR_SENTINEL_O(x)    /* Nothing */
#endif
#if __has_attribute(__hot__)
#   define __ATTR_HOT              __attribute__((__hot__))
#else
#   define __NO_ATTR_HOT           1
#   define __ATTR_HOT              /* Nothing */
#endif
#if __has_attribute(__cold__)
#   define __ATTR_COLD             __attribute__((__cold__))
#else
#   define __NO_ATTR_COLD          1
#   define __ATTR_COLD             /* Nothing */
#endif
#if __has_attribute(__noclone__)
#   define __ATTR_NOCLONE          __attribute__((__noclone__))
#else
#   define __NO_ATTR_NOCLONE       1
#   define __ATTR_NOCLONE          /* Nothing */
#endif
#if __GCC_VERSION(4,8,0)
#   define __ATTR_THREAD           __thread
#else
#   define __NO_ATTR_THREAD        1
#   define __ATTR_THREAD           /* Nothing */
#endif
#if __has_attribute(__assume_aligned__)
#   define __ATTR_ASSUME_ALIGNED(n) __attribute__((__assume_aligned__(n)))
#else
#   define __NO_ATTR_ASSUME_ALIGNED 1
#   define __ATTR_ASSUME_ALIGNED(n) /* Nothing */
#endif
#if __has_attribute(__alloc_align__)
#   define __ATTR_ALLOC_ALIGN(pari) __attribute__((__alloc_align__(pari)))
#else
#   define __NO_ATTR_ALLOC_ALIGN   1
#   define __ATTR_ALLOC_ALIGN(pari) /* Nothing */
#endif
#if __has_attribute(__nothrow__)
#   define __ATTR_NOTHROW        __attribute__((__nothrow__))
#else
#   define __NO_ATTR_NOTHROW     1
#   define __ATTR_NOTHROW        /* Nothing */
#endif
#if __has_attribute(__optimize__)
#   define __ATTR_OPTIMIZE(opt)  __attribute__((__optimize__(opt)))
#else
#   define __NO_ATTR_OPTIMIZE    1
#   define __ATTR_OPTIMIZE(opt)  /* Nothing */
#endif
#if __has_attribute(__transparent_union__) && !defined(__cplusplus)
#   define __ATTR_TRANSPARENT_UNION __attribute__((__transparent_union__))
#else
#   define __NO_ATTR_TRANSPARENT_UNION 1
#   define __ATTR_TRANSPARENT_UNION    /* Nothing */
#endif
/* format-printer attributes. */
#if __has_attribute(__format__)
#   define __ATTR_FORMAT_PRINTF(fmt,args) __attribute__((__format__(__printf__,fmt,args)))
#else
#   define __NO_ATTR_FORMAT_PRINTF        1
#   define __ATTR_FORMAT_PRINTF(fmt,args) /* Nothing */
#endif
#if !defined(__NO_ATTR_FORMAT_PRINTF) /* TODO: There were added later. - But when exactly? */
#   define __ATTR_FORMAT_SCANF(fmt,args)    __attribute__((__format__(__scanf__,fmt,args)))
#   define __ATTR_FORMAT_STRFMON(fmt,args)  __attribute__((__format__(__strfmon__,fmt,args)))
#   define __ATTR_FORMAT_STRFTIME(fmt,args) __attribute__((__format__(__strftime__,fmt,0)))
#else
#   define __NO_ATTR_FORMAT_SCANF           1
#   define __NO_ATTR_FORMAT_STRFMON         1
#   define __NO_ATTR_FORMAT_STRFTIME        1
#   define __ATTR_FORMAT_SCANF(fmt,args)    /* Nothing */
#   define __ATTR_FORMAT_STRFMON(fmt,args)  /* Nothing */
#   define __ATTR_FORMAT_STRFTIME(fmt,args) /* Nothing */
#endif
#if !defined(__ELF__) && \
    (defined(__PE__) || defined(_WIN32) || defined(__CYGWIN__))
#   define __ATTR_DLLIMPORT      __attribute__((__dllimport__))
#   define __ATTR_DLLEXPORT      __attribute__((__dllexport__))
#else
#   define __NO_ATTR_DLLIMPORT   1
#   define __ATTR_DLLIMPORT      /* Nothing */
#   define __NO_ATTR_DLLEXPORT   1
#   define __ATTR_DLLEXPORT      /* Nothing */
#endif
#if __has_attribute(__nonnull__)
#   define __NONNULL(ppars)      __attribute__((__nonnull__ ppars))
#else
#   define __NO_NONNULL          1
#   define __NONNULL(ppars)      /* Nothing */
#endif
#if __has_attribute(__warn_unused_result__)
#   define __WUNUSED             __attribute__((__warn_unused_result__))
#else
#   define __NO_WUNUSED          1
#   define __WUNUSED             /* Nothing */
#endif

#define __ATTR_WARNING(text)     __attribute__((__warning__(text)))
#define __ATTR_ERROR(text)       __attribute__((__error__(text)))
#define __ATTR_SECTION(name)     __attribute__((__section__(name)))
#define __ATTR_RETNONNULL        __attribute__((__returns_nonnull__))
#define __ATTR_PACKED            __attribute__((__packed__))
#define __ATTR_ALIAS(name)       __attribute__((__alias__(name)))
#define __ATTR_ALIGNED(n)        __attribute__((__aligned__(n)))
#define __ATTR_WEAK              __attribute__((__weak__))
#define __ATTR_RETURNS_TWICE     __attribute__((__returns_twice__))
#define __ATTR_EXTERNALLY_VISIBLE __attribute__((__externally_visible__))
#define __ATTR_VISIBILITY(vis)   __attribute__((__visibility__(vis)))

#ifdef __INTELLISENSE__
#   define __XBLOCK(...)      (([&]__VA_ARGS__)())
#   define __XRETURN             return
#   define __builtin_assume(x)   __assume(x)
#else
#if __GCC_VERSION(4,4,0) || defined(__TPP_VERSION__)
#   define __PRIVATE_PRAGMA(...) _Pragma(#__VA_ARGS__)
#   define __pragma(...) __PRIVATE_PRAGMA(__VA_ARGS__)
#else
#   define __NO_pragma   1
#   define __pragma(...) /* Nothing */
#endif
#   define __XBLOCK              __extension__
#   define __XRETURN             /* Nothing */
#if !__has_builtin(__builtin_assume)
#   define __NO_builtin_assume   1
#   define __builtin_assume(x)  (void)0
#endif
#endif
#if __GCC_VERSION(4,3,0) && (!defined(__GCCXML__) && \
   !defined(__clang__) && !defined(unix) && \
   !defined(__unix__)) || defined(__LP64__)
#   define __COMPILER_ALIGNOF    __alignof__
#elif defined(__clang__)
#   define __COMPILER_ALIGNOF    __alignof
#elif defined(__cplusplus)
extern "C++" { template<class T> struct __compiler_alignof { char __x; T __y; }; }
#   define __COMPILER_ALIGNOF(T) (sizeof(__compiler_alignof< T >)-sizeof(T))
#else
#   define __COMPILER_ALIGNOF(T) ((__SIZE_TYPE__)&((struct{ char __x; T __y; } *)0)->__y)
#endif
#if defined(__NO_INLINE__) && 0
#   define __NO_ATTR_INLINE 1
#   define __ATTR_INLINE    /* Nothing */
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ > 199901L
#   define __ATTR_INLINE    inline
#elif __GCC_VERSION(2,7,0)
#   define __ATTR_INLINE    __inline__
#else
#   define __NO_ATTR_INLINE 1
#   define __ATTR_INLINE    /* Nothing */
#endif
#if __GCC_VERSION(3,0,0)
#   define __ATTR_FORCEINLINE __inline__ __attribute__((__always_inline__))
#elif __GCC_VERSION(2,7,0)
#   define __NO_ATTR_FORCEINLINE 1
#   define __ATTR_FORCEINLINE __inline__
#else
#   define __NO_ATTR_FORCEINLINE 1
#   define __ATTR_FORCEINLINE /* Nothing */
#endif
#define __LOCAL       static __ATTR_INLINE
#define __FORCELOCAL  static __ATTR_FORCEINLINE
#ifndef __LONGLONG
#ifdef __CC__
__extension__ typedef long long __longlong_t;
__extension__ typedef unsigned long long __ulonglong_t;
#define __LONGLONG   __longlong_t
#define __ULONGLONG  __ulonglong_t
#endif
#endif /* !__LONGLONG */

#if !__GCC_VERSION(2,92,0) /* !__GCC_VERSION(2,95,0) */
#ifndef __restrict
#if defined(restrict) || \
   (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 199901L)
#   define __restrict restrict
#else
#   define __restrict /* Nothing */
#endif
#endif /* !__restrict */
#ifndef __restrict__
#define __restrict__   __restrict
#endif /* !__restrict__ */
#endif

#if __GCC_VERSION(3,1,0) && !defined(__GNUG__)
#   define __restrict_arr __restrict
#else
#   define __restrict_arr /* Nothing */
#endif
#if 1
#   define __empty_arr(T,x) __extension__ T x[0]
#else
#   define __empty_arr(T,x) T x[1]
#endif

#define __STATIC_IF(x)   if(x)
#define __STATIC_ELSE(x) if(!(x))
#ifdef __cplusplus
#define __IF0     if(false)
#define __IF1     if(true)
#define __WHILE0  while(false)
#define __WHILE1  while(true)
#else
#define __IF0     if(0)
#define __IF1     if(1)
#define __WHILE0  while(0)
#define __WHILE1  while(1)
#endif

#ifdef __cplusplus
#if !defined(__INTEL_VERSION__) || __INTEL_VERSION__ >= 600 || \
    (_WCHAR_T_DEFINED+0 != 0) || (_WCHAR_T+0 != 0)
#define __native_wchar_t_defined 1
#define __wchar_t_defined 1
#endif
#endif

#ifndef __INTELLISENSE__
#define __FUNCTION__   __extension__ __FUNCTION__
#endif

#if !__has_builtin(__builtin_LINE)
#define __builtin_LINE()     __LINE__
#define __builtin_FUNCTION() __FUNCTION__
#define __builtin_FILE()     __FILE__
#endif

#if !__has_builtin(__builtin_unreachable)
#define __builtin_unreachable() __XBLOCK({ for (;;); (void)0; })
#endif

#if !__has_builtin(__builtin_object_size)
#define __builtin_object_size(ptr,type) ((type) < 2 ? (__SIZE_TYPE__)-1 : 0)
#endif


#if __GCC_VERSION(4,7,0)
#   define __COMPILER_BARRIER()       __atomic_signal_fence(__ATOMIC_ACQ_REL)
#   define __COMPILER_READ_BARRIER()  __atomic_signal_fence(__ATOMIC_ACQUIRE)
#   define __COMPILER_WRITE_BARRIER() __atomic_signal_fence(__ATOMIC_RELEASE)
#elif defined(__COMPILER_HAVE_GCC_ASM)
#   define __COMPILER_BARRIERS_ALL_IDENTICAL 1
#   define __COMPILER_BARRIER()       __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#   define __COMPILER_READ_BARRIER()  __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#   define __COMPILER_WRITE_BARRIER() __XBLOCK({ __asm__ __volatile__("" : : : "memory"); (void)0; })
#else
#   define __COMPILER_BARRIERS_ALL_IDENTICAL 1
#   define __COMPILER_BARRIER()       __sync_synchronize()
#   define __COMPILER_READ_BARRIER()  __sync_synchronize()
#   define __COMPILER_WRITE_BARRIER() __sync_synchronize()
#endif

#if 1
#define __COMPILER_IGNORE_UNINITIALIZED(var) var=var
#endif

#ifdef __cplusplus
#ifdef __INTELLISENSE__
#   define __NULLPTR    nullptr
#else
#   define __NULLPTR          0
#endif
#else
#   define __NULLPTR ((void *)0)
#endif

#ifndef __INTELLISENSE__
#define __COMPILER_DEPRECATED_EXPR(val) \
   ({ __pragma(message("Warning: Deprecated value")) (val); })
#define __COMPILER_DEPRECATED_EXPR_(msg,val) \
   ({ __pragma(message("Warning: " msg)) (val); })
#endif



#ifdef __cplusplus
/* `__builtin_choose_expr()' is only available in C, but not in C++ */
#undef __builtin_choose_expr
#define __NO_builtin_choose_expr 1
#define __builtin_choose_expr(c,tt,ff) ((c)?(tt):(ff))

/* g++ 6.4.0 on windows had a bug that essentially broke
 * `__builtin_types_compatible_p()' from working. - Instead, you'd get an
 * error >'__builtin_types_compatible_p' was not declared in this scope< */
#if defined(_WIN32) && __GNUC__ == 6 && __GNUC_MINOR__ == 4
namespace __int {
template<class __T> struct __fixed_types_compatible_remcv {typedef __T __type; };
template<class __T> struct __fixed_types_compatible_remcv<__T const> {typedef __T __type; };
template<class __T> struct __fixed_types_compatible_remcv<__T volatile> {typedef __T __type; };
template<class __T> struct __fixed_types_compatible_remcv<__T const volatile> {typedef __T __type; };
template<class __T1, class __T2> struct __fixed_types_compatible_impl {enum{__val=false};};
template<class __T1> struct __fixed_types_compatible_impl<__T1,__T1> {enum{__val=true};};
template<class __T1, class __T2> struct __fixed_types_compatible:
 __fixed_types_compatible_impl<typename __fixed_types_compatible_remcv<__T1>::__type,
                               typename __fixed_types_compatible_remcv<__T2>::__type>{};
}
#undef __builtin_types_compatible_p
#define __builtin_types_compatible_p(...) (::__int::__fixed_types_compatible< __VA_ARGS__ >::__val)
#endif
#endif /* __cplusplus */


