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
#ifndef TPP_IF
#include "__TPP_STDINC.h"

#define __TPP_PRIVATE_MIF_0(mt,dt,mf,df) mf(df)
#define __TPP_PRIVATE_MIF_1(mt,dt,mf,df) mt(dt)
#define __TPP_WRITE_COMMA(d) ,

//////////////////////////////////////////////////////////////////////////
// Param c:   Integral    // If-condition
// Param mt:  Macro(Data) // Macro called, with dt, if "c" is true
// Param dt:  Data        // Argument for mt
// Param mf:  Macro(Data) // Macro called, with dt, if "c" is false
// Param df:  Data        // Argument for mf
#define TPP_MIF(c,mt,dt,mf,df)\
 __TPP_BASIC_CAT(__TPP_PRIVATE_MIF_,__TPP_EVAL(!!(c)))(mt,dt,mf,df)

//////////////////////////////////////////////////////////////////////////
// Param c:   Integral // If-condition
// Param tt:  Tuple    // Tuple expanded, if "c" is true
// Param ft:  Tuple    // Tuple expanded, if "c" is false
#define TPP_IF(c,tt,ft) \
 __TPP_BASIC_CAT(__TPP_PRIVATE_MIF_,__TPP_EVAL(!!(c)))(__TPP_BASIC_EXPAND_TUPLE,tt,__TPP_BASIC_EXPAND_TUPLE,ft)

//////////////////////////////////////////////////////////////////////////
// Param c:   Integral // If-condition
// Param tt:  Code     // Code written, if "c" is true
// Param ft:  Code     // Code written, if "c" is false
#define TPP_IIF(c,t,f) \
 __TPP_BASIC_CAT(__TPP_PRIVATE_MIF_,__TPP_EVAL(!!(c)))(__TPP_FORCE_EXPAND,t,__TPP_FORCE_EXPAND,f)


//////////////////////////////////////////////////////////////////////////
// Param c:   Integral // If-condition
// Expands to a comma token [,] if "c" is true
#define TPP_COMMA_IF(c) TPP_MIF(c,__TPP_WRITE_COMMA,~,__TPP_EAT_ARGS,~)

#endif /* !TPP_IF */
