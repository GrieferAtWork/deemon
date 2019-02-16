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
#ifndef TPP_OVERLOAD
#include "__TPP_STDINC.h"
#include "TPP_VA_NARGS.h"


//////////////////////////////////////////////////////////////////////////
// Overload a macro by parameter count:
// >> #define FUNC_0()    __pragma(message(TPP_STR_NOEXPAND(FUNC_0())))
// >> #define FUNC_1(a)   __pragma(message(TPP_STR_NOEXPAND(FUNC_1(a))))
// >> #define FUNC_2(a,b) __pragma(message(TPP_STR_NOEXPAND(FUNC_2(a,b))))
// >> #define FUNC(...) TPP_OVERLOAD(FUNC_,__VA_ARGS__)
// >> FUNC()         // prints: "FUNC_0()"
// >> FUNC(42)       // prints: "FUNC_1(42)"
// >> FUNC(42,10)    // prints: "FUNC_2(42,10)"
#define TPP_OVERLOAD(func,...) __TPP_BASIC_CAT(func,TPP_VA_NARGS(__VA_ARGS__))(__VA_ARGS__)

#endif /* !TPP_OVERLOAD */
