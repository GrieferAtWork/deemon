/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#pragma once
#ifndef TPP_REPEAT
#include "__TPP_STDINC.h"

#define __TPP_PRIVATE_REPEAT_NEXT(b,s,c,m,d) __TPP_BASIC_CAT2(__TPP_PRIVATE_REPEAT_,b)(s,c,m,d)
#define __TPP_PRIVATE_REPEAT_0(s,c,m,d) /* break; */
#define __TPP_PRIVATE_REPEAT_1(s,c,m,d) m(s,d)__TPP_PRIVATE_REPEAT_NEXT(__TPP_EVAL(s+1 < (c)),__TPP_EVAL(s+1),c,m,d)

//////////////////////////////////////////////////////////////////////////
// Repeats a given macro "m" "c" times
// >> Param c: Integer
//    >> Constant expression for the amount of times, to repeat "m"
// >> Param m: macro(Integer i, Data d)
//    >> Macro to be repeated
// >> Param d: Data
//    >> Second parameter for calls to "m"
#define TPP_REPEAT(c,m,d) __TPP_PRIVATE_REPEAT_NEXT(__TPP_EVAL(c > 0),0,c,m,d)

#endif /* !TPP_REPEAT */
