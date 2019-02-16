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
#pragma once
#ifndef TPP_FOR
#include "__TPP_STDINC.h"

#define __TPP_PRIVATE_FOR_0(s,p,n,m) /*break;*/
#define __TPP_PRIVATE_FOR_1(s,p,n,m)\
 m(s)__TPP_PRIVATE_FOR_E(s,p,n,m,n(s))
#define __TPP_PRIVATE_FOR_E(s,p,n,m,ns)\
 __TPP_PRIVATE_FOR_E2(s,p,n,m,ns)
#define __TPP_PRIVATE_FOR_E2(s,p,n,m,ns)\
 __TPP_BASIC_CAT(__TPP_PRIVATE_FOR_,__TPP_EVAL(!!(p(ns))))(ns,p,n,m)

//////////////////////////////////////////////////////////////////////////
// PP_FOR(State start, Bool pred(State s), State next(State s), Code macro(State s))
// Example:
// >> #define PRED(s) TPP_TUPLE_AT(s,0) != TPP_TUPLE_AT(s,1)
// >> #define NEXT(s) (__TPP_EVAL(TPP_TUPLE_AT(s,0)-1),TPP_TUPLE_AT(s,1))
// >> #define ITER(s) __pragma(message("for(" TPP_STR(TPP_TUPLE_AT(s,0)) ")"))
// >> // Prints "for(2)" "for(1)" "for(0)" "for(-1)"
// >> TPP_FOR((2,-2),PRED,NEXT,ITER)
#define TPP_FOR(s,p,n,m) __TPP_BASIC_CAT(__TPP_PRIVATE_FOR_,__TPP_EVAL(!!(p(s))))(s,p,n,m)

#endif /* !TPP_FOR */
