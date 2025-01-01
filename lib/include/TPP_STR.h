/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#pragma once
#include "__TPP_STDINC.h"

#if defined(__TPP_STR_AT) && defined(__TPP_STR_SUBSTR)
#include "TPP_IF.h"
#include "TPP_CTYPE.h"

//////////////////////////////////////////////////////////////////////////
// String TPP_STR_LSTRIP_PRED(String str, bool (*pred)(char c))
// >> Removes all characters from the left of "str", for which "pred" returns true
#define TPP_STR_LSTRIP_PRED(str,pred) TPP_IIF(pred(__TPP_STR_AT(str,0)),TPP_STR_LSTRIP_PRED(__TPP_STR_SUBSTR(str,1,-1),pred),str)

 //////////////////////////////////////////////////////////////////////////
// String TPP_STR_RSTRIP_PRED(String str, bool (*pred)(char c))
// >> Removes all characters from the right of "str", for which "pred" returns true
#define TPP_STR_RSTRIP_PRED(str,pred) TPP_IIF(pred(__TPP_STR_AT(str,-1)),TPP_STR_RSTRIP_PRED(__TPP_STR_SUBSTR(str,0,-1),pred),str)

 //////////////////////////////////////////////////////////////////////////
// String TPP_STR_STRIP_PRED(String str, bool (*pred)(char c))
// >> Removes all characters from the left and right of "str", for which "pred" returns true
#define TPP_STR_STRIP_PRED(str,pred)  TPP_STR_LSTRIP_PRED(TPP_STR_RSTRIP_PRED(str,pred),pred)

//////////////////////////////////////////////////////////////////////////
// String TPP_STR_LSTRIP(String str)
// >> Removes all blank characters from the left of "str"
#define TPP_STR_LSTRIP(str) TPP_STR_LSTRIP_PRED(str,__builtin_tpp_isblank)

//////////////////////////////////////////////////////////////////////////
// String TPP_STR_RSTRIP(String str)
// >> Removes all blank characters from the right of "str"
#define TPP_STR_RSTRIP(str) TPP_STR_RSTRIP_PRED(str,__builtin_tpp_isblank)

//////////////////////////////////////////////////////////////////////////
// String TPP_STR_STRIP(String str)
// >> Removes all blank characters from the left and right of "str"
#define TPP_STR_STRIP(str)  TPP_STR_STRIP_PRED(str,__builtin_tpp_isblank)
#endif


#if defined(__TPP_STR_AT) && defined(__TPP_EVAL) && defined(__TPP_STR_SIZE)
#define __TPP_PRIVATE_STR_FOREACH_ITER_1(i,n,str,m,d) m(__TPP_STR_AT(str,i),d)__TPP_PRIVATE_STR_FOREACH_ITER(__TPP_EVAL(i+1),n,str,m,d)
#define __TPP_PRIVATE_STR_FOREACH_ITER_0(i,n,str,m,d) /* nothing */
#define __TPP_PRIVATE_STR_FOREACH_ITER(i,n,str,m,d) __TPP_BASIC_CAT(__TPP_PRIVATE_STR_FOREACH_ITER_,__TPP_EVAL(i < n))(i,n,str,m,d)

//////////////////////////////////////////////////////////////////////////
// Code ...TPP_STR_FOREACH(String str, Code (*m)(char c, Data d), Data d)
// >> Execute "m" for every character in "str"
#define TPP_STR_FOREACH(str,m,d) __TPP_PRIVATE_STR_FOREACH_ITER(0,__TPP_STR_SIZE(str),str,m,d)

#if defined(__TPP_STR_PACK)
#define __TPP_STR_APPLY_PREDICATE_CALL(d,pred) pred(d)

//////////////////////////////////////////////////////////////////////////
// String TPP_STR_APPLY_PREDICATE(String str, char (*pred)(char c))
// Apply a character modification predicate to every character of "str"
#define TPP_STR_APPLY_PREDICATE(str,pred) __TPP_STR_PACK(TPP_STR_FOREACH(str,__TPP_STR_APPLY_PREDICATE_CALL,pred))

#include "TPP_CTYPE.h"

//////////////////////////////////////////////////////////////////////////
// String TPP_STR_UPPER(String str)
// Returns the uppercase version of "str"
#define TPP_STR_UPPER(str)    TPP_STR_APPLY_PREDICATE(str,__builtin_tpp_toupper)

//////////////////////////////////////////////////////////////////////////
// String TPP_STR_LOWER(String str)
// Returns the lowercase version of "str"
#define TPP_STR_LOWER(str)    TPP_STR_APPLY_PREDICATE(str,__builtin_tpp_tolower)

//////////////////////////////////////////////////////////////////////////
// String TPP_STR_SWAPCASE(String str)
// Returns the swapped-case version of "str"
#define TPP_STR_SWAPCASE(str) TPP_STR_APPLY_PREDICATE(str,__builtin_tpp_swapcase)

#endif

#endif


#if defined(__TPP_EVAL) && defined(__TPP_STR_SUBSTR) && defined(__TPP_STR_AT) && defined(__TPP_STR_SIZE)
#define __TPP_STRCMP_PRED_ITER_STOP_0(lhs,rhs,n,pred_lo) \
 __TPP_STRCMP_PRED_SIZE_EQ_2_1(__TPP_STR_SUBSTR(lhs,1,-1),__TPP_STR_SUBSTR(rhs,1,-1),__TPP_EVAL(n-1),pred_lo)
#define __TPP_STRCMP_PRED_ITER_STOP_1(lhs,rhs,n,pred_lo) 0
#define __TPP_STRCMP_PRED_ITER_RHS_LO_LHS_0(lhs,rhs,n,pred_lo) \
 __TPP_BASIC_CAT(__TPP_STRCMP_PRED_ITER_STOP_,\
 __TPP_EVAL(n == 1))(lhs,rhs,n,pred_lo)
#define __TPP_STRCMP_PRED_ITER_RHS_LO_LHS_1(lhs,rhs,n,pred_lo) -1
#define __TPP_STRCMP_PRED_ITER_LHS_LO_RHS_0(lhs,rhs,n,pred_lo) \
 __TPP_BASIC_CAT(__TPP_STRCMP_PRED_ITER_RHS_LO_LHS_,\
 __TPP_EVAL(pred_lo(__TPP_STR_AT(rhs,0),__TPP_STR_AT(lhs,0))))(lhs,rhs,n,pred_lo)
#define __TPP_STRCMP_PRED_ITER_LHS_LO_RHS_1(lhs,rhs,n,pred_lo) 1
#define __TPP_STRCMP_PRED_SIZE_EQ_2_1(lhs,rhs,n,pred_lo) \
 __TPP_BASIC_CAT(__TPP_STRCMP_PRED_ITER_LHS_LO_RHS_,\
 __TPP_EVAL(pred_lo(__TPP_STR_AT(lhs,0),__TPP_STR_AT(rhs,0))))(lhs,rhs,n,pred_lo)
#define __TPP_STRCMP_PRED_SIZE_EQ_2_0(lhs,rhs,n,pred_lo) 0
#define __TPP_STRCMP_PRED_SIZE_EQ_1(lhs,rhs,n,pred_lo) \
 __TPP_BASIC_CAT(__TPP_STRCMP_PRED_SIZE_EQ_2_,__TPP_EVAL(n!=0))(lhs,rhs,n,pred_lo)
#define __TPP_STRCMP_PRED_SIZE_EQ_0(lhs,rhs,n,pred_lo) __TPP_EVAL(n < __TPP_STR_SIZE(rhs) ? 1 : -1)
#define __TPP_STRCMP_PRED_SIZE(lhs,rhs,lhs_size,rhs_size,pred_lo) \
 __TPP_BASIC_CAT(__TPP_STRCMP_PRED_SIZE_EQ_,\
 __TPP_EVAL(lhs_size == rhs_size))(lhs,rhs,lhs_size,pred_lo)


//////////////////////////////////////////////////////////////////////////
// int TPP_STRCMP_PRED(String lhs, String rhs, bool (*pred_lo)(char a, char b))
// >> Compares two strings, using a given predicate for comparing two characters
// >> The predicate should returns true, if "a < b"
#define TPP_STRCMP_PRED(lhs,rhs,pred_lo) \
 __TPP_STRCMP_PRED_SIZE(lhs,rhs,__TPP_STR_SIZE(lhs),__TPP_STR_SIZE(rhs),pred_lo)

#define __TPP_STRCMP_DEFAULT_PRED(lhs,rhs)  lhs < rhs
#define __TPP_STRICMP_DEFAULT_PRED(lhs,rhs) __builtin_tpp_tolower(lhs) < __builtin_tpp_tolower(rhs)

//////////////////////////////////////////////////////////////////////////
// int TPP_STRCMP(String lhs, String rhs)
// >> Compares two strings
// Returns: -1 if lhs > rhs
// Returns:  0 if lhs == rhs
// Returns:  1 if lhs < rhs
#define TPP_STRCMP(lhs,rhs)  TPP_STRCMP_PRED(lhs,rhs,__TPP_STRCMP_DEFAULT_PRED)

//////////////////////////////////////////////////////////////////////////
// int TPP_STRICMP(String lhs, String rhs)
// >> Compares two strings (ignores casing)
// Returns: -1 if lhs > rhs
// Returns:  0 if lhs == rhs
// Returns:  1 if lhs < rhs
#define TPP_STRICMP(lhs,rhs) TPP_STRCMP_PRED(lhs,rhs,__TPP_STRICMP_DEFAULT_PRED)

#endif
