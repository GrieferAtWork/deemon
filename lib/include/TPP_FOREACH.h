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
#ifndef TPP_FOREACH
#include "__TPP_STDINC.h"

#define __TPP_PRIVATE_FOREACH_NEMPTY2(p,r, ...) r
#define __TPP_PRIVATE_FOREACH_0(m,d, ...) /* break */
#define __TPP_PRIVATE_FOREACH_1(m,d,first, ...)\
m(first,d)__TPP_PRIVATE_FOREACH_E(m,d,__VA_ARGS__)
#define __TPP_PRIVATE_FOREACH_E(m,d, ...)\
__TPP_BASIC_CAT(__TPP_PRIVATE_FOREACH_,__TPP_PRIVATE_FOREACH_NEMPTY2(__VA_COMMA__ 1,0))(m,d,__VA_ARGS__)
#define __TPP_PRIVATE_FOREACH_C(m,d, ...)\
__TPP_PRIVATE_FOREACH_E(m,d,__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
// Param tpl: Tuple
// Param m:   Macro(Data e, Data d)
// Param d:   Data // Second argument, when calling "m"
// >> #define ITER(e,d) __pragma(message("ITER: " #e))
// >> TPP_FOREACH((),ITER,~) // prints nothing
// >> TPP_FOREACH((10,20,30,40),ITER,~)
// >> // prints: "ITER: 10", "ITER: 20", "ITER: 30", "ITER: 40"
#define TPP_FOREACH(tpl,m,d) __TPP_PRIVATE_FOREACH_C(m,d,__TPP_FORCE_EXPAND tpl)

#endif /* !TPP_FOREACH */
