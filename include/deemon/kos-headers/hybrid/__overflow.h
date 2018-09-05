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
#ifndef __GUARD_HYBRID___OVERFLOW_H
#define __GUARD_HYBRID___OVERFLOW_H 1

#include <__stdinc.h>
#include "typecore.h"
#include "limitcore.h"

__DECL_BEGIN

#if defined(__INTELLISENSE__) && defined(__cplusplus) && 0
/* @return: true:  Overflow occurred (unlikely; `*res' is undefined)
 *                 Overflow here means that the finite result stored
 *                 in `*res' doesn't match a value that would have
 *                 been produced when infinite precision was available.
 *                 e.g.: `UINT_MAX + 42u' overflows, but `32u + 42u' or `11 - 19' don't
 * @return: false: `*res' contains the correct result. */
#define __IMPL_HYBRID_DEFINE_OVERFLOW_MATH(opn,n) \
__BOOL __WUNUSED __NONNULL((3)) __ATTR_PURE __NOTHROW(__hybrid_overflow_uadd(__UINT##opn##_TYPE__ x, __UINT##opn##_TYPE__ y, __UINT##n##_TYPE__ *__restrict res)); \
__BOOL __WUNUSED __NONNULL((3)) __ATTR_PURE __NOTHROW(__hybrid_overflow_sadd(__INT##opn##_TYPE__ x, __INT##opn##_TYPE__ y, __INT##n##_TYPE__ *__restrict res)); \
__BOOL __WUNUSED __NONNULL((3)) __ATTR_PURE __NOTHROW(__hybrid_overflow_usub(__UINT##opn##_TYPE__ x, __UINT##opn##_TYPE__ y, __UINT##n##_TYPE__ *__restrict res)); \
__BOOL __WUNUSED __NONNULL((3)) __ATTR_PURE __NOTHROW(__hybrid_overflow_ssub(__INT##opn##_TYPE__ x, __INT##opn##_TYPE__ y, __INT##n##_TYPE__ *__restrict res)); \
__BOOL __WUNUSED __NONNULL((3)) __ATTR_PURE __NOTHROW(__hybrid_overflow_umul(__UINT##opn##_TYPE__ x, __UINT##opn##_TYPE__ y, __UINT##n##_TYPE__ *__restrict res)); \
__BOOL __WUNUSED __NONNULL((3)) __ATTR_PURE __NOTHROW(__hybrid_overflow_smul(__INT##opn##_TYPE__ x, __INT##opn##_TYPE__ y, __INT##n##_TYPE__ *__restrict res)); \
/**/
extern "C++" {
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(8,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(16,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(16,16)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(32,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(32,16)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(32,32)
#ifdef __INT64_TYPE__
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(64,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(64,16)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(64,32)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(64,64)
#ifdef __INT128_TYPE__
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,16)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,32)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,64)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,128)
#endif
#endif
}
#undef __IMPL_HYBRID_DEFINE_OVERFLOW_MATH
#elif (__has_builtin(__builtin_add_overflow) && !defined(__ibmxl__)) || \
      (defined(__GNUC__) && (__GNUC__ >= 5) && !defined(__INTEL_COMPILER))
/* @return: true:  Overflow occurred (unlikely; `*res' is undefined)
 *                 Overflow here means that the finite result stored
 *                 in `*res' doesn't match a value that would have
 *                 been produced when infinite precision was available.
 *                 e.g.: `UINT_MAX + 42u' overflows, but `32u + 42u' or `11 - 19' don't
 * @return: false: `*res' contains the correct result. */
#ifdef __NO_builtin_expect
#define __hybrid_overflow_uadd(x,y,res) __builtin_add_overflow(x,y,res)
#define __hybrid_overflow_sadd(x,y,res) __builtin_add_overflow(x,y,res)
#define __hybrid_overflow_usub(x,y,res) __builtin_sub_overflow(x,y,res)
#define __hybrid_overflow_ssub(x,y,res) __builtin_sub_overflow(x,y,res)
#define __hybrid_overflow_umul(x,y,res) __builtin_mul_overflow(x,y,res)
#define __hybrid_overflow_smul(x,y,res) __builtin_mul_overflow(x,y,res)
#else
#define __hybrid_overflow_uadd(x,y,res) __builtin_expect(__builtin_add_overflow(x,y,res),0)
#define __hybrid_overflow_sadd(x,y,res) __builtin_expect(__builtin_add_overflow(x,y,res),0)
#define __hybrid_overflow_usub(x,y,res) __builtin_expect(__builtin_sub_overflow(x,y,res),0)
#define __hybrid_overflow_ssub(x,y,res) __builtin_expect(__builtin_sub_overflow(x,y,res),0)
#define __hybrid_overflow_umul(x,y,res) __builtin_expect(__builtin_mul_overflow(x,y,res),0)
#define __hybrid_overflow_smul(x,y,res) __builtin_expect(__builtin_mul_overflow(x,y,res),0)
#endif
#else

#ifndef __cplusplus
#define __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,opn,resn,x,y,res) \
        __IMPL_HYBRID_OVERFLOW_NAME(name,opn,resn)((Tpfx##opn##_TYPE__)(x),(Tpfx##opn##_TYPE__)(y),(Tpfx##resn##_TYPE__ *)(res))
#ifdef __NO_builtin_choose_expr
#ifndef __INT64_TYPE__
#define __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 1 && sizeof(y) <= 1) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,8,8,x,y,res) : \
  (sizeof(x) <= 2 && sizeof(y) <= 2) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,8,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,8,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 2 && sizeof(y) <= 2) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,16,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,16,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res) \
        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,32,x,y,res)
#define __IMPL_HYBRID_OVERFLOW_SELECT(name,Tpfx,x,y,res) \
 (sizeof(*(res)) == 1 ? __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) : \
  sizeof(*(res)) == 2 ? __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) : \
                        __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res))
#elif !defined(__INT128_TYPE__) /* With 64-bit integer type */
#define __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 1 && sizeof(y) <= 1) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,8,8,x,y,res) : \
  (sizeof(x) <= 2 && sizeof(y) <= 2) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,8,x,y,res) : \
  (sizeof(x) <= 4 && sizeof(y) <= 4) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,8,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,8,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 2 && sizeof(y) <= 2) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,16,x,y,res) : \
  (sizeof(x) <= 4 && sizeof(y) <= 4) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,16,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,16,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 4 && sizeof(y) <= 4) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,32,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,32,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_64(name,Tpfx,x,y,res) \
        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,64,x,y,res)
#define __IMPL_HYBRID_OVERFLOW_SELECT(name,Tpfx,x,y,res) \
 (sizeof(*(res)) == 1 ? __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) : \
  sizeof(*(res)) == 2 ? __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) : \
  sizeof(*(res)) == 4 ? __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res) : \
                        __IMPL_HYBRID_OVERFLOW_SELECT_64(name,Tpfx,x,y,res))
#else /* With 128-bit integer type */
#define __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 1 && sizeof(y) <= 1) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,8,8,x,y,res) : \
  (sizeof(x) <= 2 && sizeof(y) <= 2) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,8,x,y,res) : \
  (sizeof(x) <= 4 && sizeof(y) <= 4) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,8,x,y,res) : \
  (sizeof(x) <= 8 && sizeof(y) <= 8) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,8,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,8,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 2 && sizeof(y) <= 2) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,16,x,y,res) : \
  (sizeof(x) <= 4 && sizeof(y) <= 4) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,16,x,y,res) : \
  (sizeof(x) <= 8 && sizeof(y) <= 8) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,16,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,16,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 4 && sizeof(y) <= 4) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,32,x,y,res) : \
  (sizeof(x) <= 8 && sizeof(y) <= 8) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,32,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,32,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_64(name,Tpfx,x,y,res) \
 ((sizeof(x) <= 8 && sizeof(y) <= 8) ? __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,64,x,y,res) : \
                                       __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,64,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_128(name,Tpfx,x,y,res) \
        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,128,x,y,res)
#define __IMPL_HYBRID_OVERFLOW_SELECT(name,Tpfx,x,y,res) \
 (sizeof(*(res)) == 1 ? __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) : \
  sizeof(*(res)) == 2 ? __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) : \
  sizeof(*(res)) == 4 ? __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res) : \
  sizeof(*(res)) == 8 ? __IMPL_HYBRID_OVERFLOW_SELECT_64(name,Tpfx,x,y,res) : \
                        __IMPL_HYBRID_OVERFLOW_SELECT_128(name,Tpfx,x,y,res))
#endif
#else
#ifndef __INT64_TYPE__
#define __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 1 && sizeof(y) <= 1,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,8,8,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 2 && sizeof(y) <= 2,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,8,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,8,x,y,res)))
#define __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 2 && sizeof(y) <= 2,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,16,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,16,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res) \
        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,32,x,y,res)
#define __IMPL_HYBRID_OVERFLOW_SELECT(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(*(res)) == 1,__IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res), \
 __builtin_choose_expr(sizeof(*(res)) == 2,__IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res), \
                                           __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res)))
#elif !defined(__INT128_TYPE__) /* With 64-bit integer type */
#define __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 1 && sizeof(y) <= 1,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,8,8,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 2 && sizeof(y) <= 2,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,8,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 4 && sizeof(y) <= 4,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,8,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,8,x,y,res))))
#define __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 2 && sizeof(y) <= 2,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,16,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 4 && sizeof(y) <= 4,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,16,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,16,x,y,res)))
#define __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 4 && sizeof(y) <= 4,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,32,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,32,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT_64(name,Tpfx,x,y,res) \
        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,64,x,y,res)
#define __IMPL_HYBRID_OVERFLOW_SELECT(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(*(res)) == 1,__IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res), \
 __builtin_choose_expr(sizeof(*(res)) == 2,__IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res), \
 __builtin_choose_expr(sizeof(*(res)) == 4,__IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res), \
                                           __IMPL_HYBRID_OVERFLOW_SELECT_64(name,Tpfx,x,y,res))))
#else /* With 128-bit integer type */
#define __IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 1 && sizeof(y) <= 1,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,8,8,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 2 && sizeof(y) <= 2,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,8,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 4 && sizeof(y) <= 4,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,8,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 8 && sizeof(y) <= 8,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,8,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,8,x,y,res)))))
#define __IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 2 && sizeof(y) <= 2,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,16,16,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 4 && sizeof(y) <= 4,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,16,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 8 && sizeof(y) <= 8,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,16,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,16,x,y,res))))
#define __IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 4 && sizeof(y) <= 4,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,32,32,x,y,res), \
 __builtin_choose_expr(sizeof(x) <= 8 && sizeof(y) <= 8,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,32,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,32,x,y,res)))
#define __IMPL_HYBRID_OVERFLOW_SELECT_64(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(x) <= 8 && sizeof(y) <= 8,__IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,64,64,x,y,res), \
                                                        __IMPL_HYBRID_OVERFLOW_INVOKE(name,Tpfx,128,64,x,y,res))
#define __IMPL_HYBRID_OVERFLOW_SELECT(name,Tpfx,x,y,res) \
 __builtin_choose_expr(sizeof(*(res)) == 1,__IMPL_HYBRID_OVERFLOW_SELECT_8(name,Tpfx,x,y,res), \
 __builtin_choose_expr(sizeof(*(res)) == 2,__IMPL_HYBRID_OVERFLOW_SELECT_16(name,Tpfx,x,y,res), \
 __builtin_choose_expr(sizeof(*(res)) == 4,__IMPL_HYBRID_OVERFLOW_SELECT_32(name,Tpfx,x,y,res), \
 __builtin_choose_expr(sizeof(*(res)) == 8,__IMPL_HYBRID_OVERFLOW_SELECT_64(name,Tpfx,x,y,res), \
                                           __IMPL_HYBRID_OVERFLOW_SELECT_128(name,Tpfx,x,y,res)))))
#endif
#endif
#define __IMPL_HYBRID_OVERFLOW_NAME(name,opn,resn)  name##opn##_##resn
#else /* !__cplusplus */
#define __IMPL_HYBRID_OVERFLOW_SELECT(name,Tpfx,x,y,res) name(x,y,res)
#define __IMPL_HYBRID_OVERFLOW_NAME(name,opn,resn)  name
extern "C++" {
#endif /* __cplusplus */

#define __IMPL_HYBRID_DEFINE_PROMOTION_MATH(opn,n,opn_x2) \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_uadd,opn,n) \
(__UINT##opn##_TYPE__ __x, __UINT##opn##_TYPE__ __y, __UINT##n##_TYPE__ *__restrict __res)) { \
 __UINT##opn_x2##_TYPE__ __true_res = (__UINT##opn_x2##_TYPE__)((__UINT##opn_x2##_TYPE__)__x + (__UINT##opn_x2##_TYPE__)__y); \
 __UINT##n##_TYPE__ __false_res = *__res = (__UINT##n##_TYPE__)__true_res; \
 return (__UINT##opn_x2##_TYPE__)__false_res != __true_res; \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_sadd,opn,n) \
(__INT##opn##_TYPE__ __x, __INT##opn##_TYPE__ __y, __INT##n##_TYPE__ *__restrict __res)) { \
 __INT##opn_x2##_TYPE__ __true_res = (__INT##opn_x2##_TYPE__)((__INT##opn_x2##_TYPE__)__x + (__INT##opn_x2##_TYPE__)__y); \
 __INT##n##_TYPE__ __false_res = *__res = (__INT##n##_TYPE__)__true_res; \
 return (__INT##opn_x2##_TYPE__)__false_res != __true_res; \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_usub,opn,n) \
(__UINT##opn##_TYPE__ __x, __UINT##opn##_TYPE__ __y, __UINT##n##_TYPE__ *__restrict __res)) { \
 *__res = (__UINT##n##_TYPE__)(__x - __y); \
 return !(__x >= __y); \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_ssub,opn,n) \
(__INT##opn##_TYPE__ __x, __INT##opn##_TYPE__ __y, __INT##n##_TYPE__ *__restrict __res)) { \
 __INT##opn_x2##_TYPE__ __true_res = (__INT##opn_x2##_TYPE__)((__INT##opn_x2##_TYPE__)__x - (__INT##opn_x2##_TYPE__)__y); \
 __INT##n##_TYPE__ __false_res = *__res = (__INT##n##_TYPE__)__true_res; \
 return (__INT##opn_x2##_TYPE__)__false_res != __true_res; \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_umul,opn,n) \
(__UINT##opn##_TYPE__ __x, __UINT##opn##_TYPE__ __y, __UINT##n##_TYPE__ *__restrict __res)) { \
 __UINT##opn_x2##_TYPE__ __true_res = (__UINT##opn_x2##_TYPE__)((__UINT##opn_x2##_TYPE__)__x * (__UINT##opn_x2##_TYPE__)__y); \
 __UINT##n##_TYPE__ __false_res = *__res = (__UINT##n##_TYPE__)__true_res; \
 return (__UINT##opn_x2##_TYPE__)__false_res != __true_res; \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_smul,opn,n) \
(__INT##opn##_TYPE__ __x, __INT##opn##_TYPE__ __y, __INT##n##_TYPE__ *__restrict __res)) { \
 __INT##opn_x2##_TYPE__ __true_res = (__INT##opn_x2##_TYPE__)((__INT##opn_x2##_TYPE__)__x * (__INT##opn_x2##_TYPE__)__y); \
 __INT##n##_TYPE__ __false_res = *__res = (__INT##n##_TYPE__)__true_res; \
 return (__INT##opn_x2##_TYPE__)__false_res != __true_res; \
} \
/**/

#define __IMPL_HYBRID_DEFINE_OVERFLOW_MATH(opn,n) \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_uadd,opn,n) \
(__UINT##opn##_TYPE__ __x, __UINT##opn##_TYPE__ __y, __UINT##n##_TYPE__ *__restrict __res)) { \
 *__res = (__UINT##n##_TYPE__)(__x + __y); \
 return __x > (__UINT##opn##_MAX__ - __y); \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_sadd,opn,n) \
(__INT##opn##_TYPE__ __x, __INT##opn##_TYPE__ __y, __INT##n##_TYPE__ *__restrict __res)) { \
 *__res = (__INT##n##_TYPE__)(__x + __y); \
 return (__y > 0 && __x > (__INT##opn##_MAX__ - __y)) || \
        (__y < 0 && __x < (__INT##opn##_MIN__ - __y)); \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_usub,opn,n) \
(__UINT##opn##_TYPE__ __x, __UINT##opn##_TYPE__ __y, __UINT##n##_TYPE__ *__restrict __res)) { \
 *__res = (__UINT##n##_TYPE__)(__x - __y); \
 return !(__x >= __y); \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_ssub,opn,n) \
(__INT##opn##_TYPE__ __x, __INT##opn##_TYPE__ __y, __INT##n##_TYPE__ *__restrict __res)) { \
 *__res = (__INT##n##_TYPE__)(__x + __y); \
 return (__y > 0 && __x < (__INT##opn##_MIN__ + __y)) || \
        (__y < 0 && __x > (__INT##opn##_MAX__ + __y)); \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_umul,opn,n) \
(__UINT##opn##_TYPE__ __x, __UINT##opn##_TYPE__ __y, __UINT##n##_TYPE__ *__restrict __res)) { \
 *__res = (__UINT##n##_TYPE__)(__x * __y); \
 return __x && __y && (__x > (__UINT##opn##_MAX__ / __y)); \
} \
__LOCAL __WUNUSED __NONNULL((3)) __ATTR_PURE __BOOL __NOTHROW(__IMPL_HYBRID_OVERFLOW_NAME(__impl_hybrid_overflow_smul,opn,n) \
(__INT##opn##_TYPE__ __x, __INT##opn##_TYPE__ __y, __INT##n##_TYPE__ *__restrict __res)) { \
 __BOOL __did_overflow = 0;  \
 *__res = (__INT##n##_TYPE__)(__x * __y); \
 if (__x > 0) { \
  if (__y > 0) { \
   /* +x * +y */ \
   if (__x > (__INT##opn##_MAX__ / __y)) \
       __did_overflow = 1; \
  } else { \
   /* +x * -y */ \
   if (__y < (__INT##opn##_MIN__ / __x)) \
       __did_overflow = 1; \
  } \
 } else if (__y > 0) { \
  /* -x * +y */ \
  if (__x < (__INT##opn##_MIN__ / __y)) \
      __did_overflow = 1; \
 } else { \
  /* -x * -y */ \
  if (__x && (__y < (__INT##opn##_MAX__ / __x))) \
      __did_overflow = 1; \
 } \
 return __did_overflow; \
} \
/**/


#if __SIZEOF_REGISTER__ >= 2
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(8,8,16)
#else
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(8,8)
#endif
#if __SIZEOF_REGISTER__ >= 4
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(16,8,32)
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(16,16,32)
#else
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(16,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(16,16)
#endif
#if __SIZEOF_REGISTER__ >= 8 && defined(__INT64_TYPE__)
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(32,8,64)
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(32,16,64)
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(32,32,64)
#else
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(32,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(32,16)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(32,32)
#endif
#ifdef __INT64_TYPE__
#if __SIZEOF_REGISTER__ >= 16 && defined(__INT128_TYPE__)
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(64,8,128)
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(64,16,128)
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(64,32,128)
__IMPL_HYBRID_DEFINE_PROMOTION_MATH(64,64,128)
#else
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(64,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(64,16)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(64,32)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(64,64)
#endif
#ifdef __INT128_TYPE__
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,8)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,16)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,32)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,64)
__IMPL_HYBRID_DEFINE_OVERFLOW_MATH(128,128)
#endif
#endif
#undef __IMPL_HYBRID_DEFINE_OVERFLOW_MATH
#undef __IMPL_HYBRID_DEFINE_PROMOTION_MATH

#undef __IMPL_HYBRID_OVERFLOW_NAME
#ifdef __cplusplus
}
#endif /* __cplusplus */


/* @return: true:  Overflow occurred (unlikely; `*res' is undefined)
 *                 Overflow here means that the finite result stored
 *                 in `*res' doesn't match a value that would have
 *                 been produced when infinite precision was available.
 *                 e.g.: `UINT_MAX + 42u' overflows, but `32u + 42u' or `11 - 19' don't
 * @return: false: `*res' contains the correct result. */
#ifdef __NO_builtin_expect
#define __hybrid_overflow_uadd(x,y,res) __IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_uadd,__UINT,x,y,res)
#define __hybrid_overflow_sadd(x,y,res) __IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_sadd,__INT,x,y,res)
#define __hybrid_overflow_usub(x,y,res) __IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_usub,__UINT,x,y,res)
#define __hybrid_overflow_ssub(x,y,res) __IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_ssub,__INT,x,y,res)
#define __hybrid_overflow_umul(x,y,res) __IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_umul,__UINT,x,y,res)
#define __hybrid_overflow_smul(x,y,res) __IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_smul,__INT,x,y,res)
#else
#define __hybrid_overflow_uadd(x,y,res) __builtin_expect(__IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_uadd,__UINT,x,y,res),0)
#define __hybrid_overflow_sadd(x,y,res) __builtin_expect(__IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_sadd,__INT,x,y,res),0)
#define __hybrid_overflow_usub(x,y,res) __builtin_expect(__IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_usub,__UINT,x,y,res),0)
#define __hybrid_overflow_ssub(x,y,res) __builtin_expect(__IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_ssub,__INT,x,y,res),0)
#define __hybrid_overflow_umul(x,y,res) __builtin_expect(__IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_umul,__UINT,x,y,res),0)
#define __hybrid_overflow_smul(x,y,res) __builtin_expect(__IMPL_HYBRID_OVERFLOW_SELECT(__impl_hybrid_overflow_smul,__INT,x,y,res),0)
#endif
#endif


__DECL_END

#endif /* !__GUARD_HYBRID___OVERFLOW_H */
