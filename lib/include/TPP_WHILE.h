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
#ifndef TPP_WHILE
#include "__TPP_STDINC.h"

#define __TPP_PRIVATE_WHILE_0(c,b,s)     s // Final state
#define __TPP_PRIVATE_WHILE_1(c,b,s)\
 __TPP_PRIVATE_WHILE_E(c,b,s,__TPP_FORCE_EXPAND(b(s)))
#define __TPP_PRIVATE_WHILE_E(c,b,s,ns)\
 __TPP_PRIVATE_WHILE_E2(c,b,s,ns)
#define __TPP_PRIVATE_WHILE_E2(c,b,s,ns)\
 __TPP_BASIC_CAT(__TPP_PRIVATE_WHILE_,__TPP_EVAL(!!(c(ns))))(c,b,ns)

//////////////////////////////////////////////////////////////////////////
// State TPP_WHILE(Integral c(State), State b(State), State s)
//  - You can think of it like this:
//    >> while (c(s)) { s = b(s); } return s;
// NOTE: This macro return the last state
// >> #define COND(s)  TPP_TUPLE_AT(s,0) != TPP_TUPLE_AT(s,1)
// >> #define BLOCK(s) (__TPP_EVAL(TPP_TUPLE_AT(s,0)+1),TPP_TUPLE_AT(s,1))
// >> TPP_WHILE(COND,BLOCK,(10,20)) // expands to [(][20][,][20][)]
#define TPP_WHILE(c,b,s) __TPP_BASIC_CAT(__TPP_PRIVATE_WHILE_,__TPP_EVAL(!!(c(s))))(c,b,s)

#endif /* !TPP_WHILE */
