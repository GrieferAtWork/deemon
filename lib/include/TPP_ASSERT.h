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
#pragma once
#ifndef TPP_ASSERT
#include "__TPP_STDINC.h"

#if defined(__TPP_EVAL)
#define __TPP_PRIVATE_ASSERT_0(s) __pragma(error("Assertion failed: " s))
#define __TPP_PRIVATE_ASSERT_1(s)
#define TPP_ASSERT(expr) __TPP_BASIC_CAT(__TPP_PRIVATE_ASSERT_,__TPP_EVAL(!!(expr)))(#expr)
#elif __has_feature(__tpp_pragma_tpp_exec__)
#define TPP_ASSERT(expr) _Pragma(STR(tpp_exec(\
"#if !" #expr "\n"\
"# error " STR(#expr) "\n"\
"#endif\n"\
)))
#else
#define TPP_ASSERT(expr) /* nothing */
#warning Not enough features enabled for preprocessor assertions
#endif

#endif /* !TPP_ASSERT */
