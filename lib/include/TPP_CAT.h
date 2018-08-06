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
#pragma once
#ifndef TPP_CAT
#include "__TPP_STDINC.h"
#include "TPP_VA_NARGS.h"

#define TPP_CAT_0()                  /* nothing */
#define TPP_CAT_1(a)                 a
#define TPP_CAT_2(a,b)               a ## b
#define TPP_CAT_3(a,b,c)             a ## b ## c
#define TPP_CAT_4(a,b,c,d)           a ## b ## c ## d
#define TPP_CAT_5(a,b,c,d,e)         a ## b ## c ## d ## e
#define TPP_CAT_6(a,b,c,d,e,f)       a ## b ## c ## d ## e ## f
#define TPP_CAT_7(a,b,c,d,e,f,g)     a ## b ## c ## d ## e ## f ## g
#define TPP_CAT_8(a,b,c,d,e,f,g,h)   a ## b ## c ## d ## e ## f ## g ## h
#define TPP_CAT_9(a,b,c,d,e,f,g,h,i) a ## b ## c ## d ## e ## f ## g ## h ## i

#define TPP_CAT(...) __TPP_BASIC_CAT(TPP_CAT_,TPP_VA_NARGS(__VA_ARGS__))(__VA_ARGS__)

#endif /* !TPP_CAT */
