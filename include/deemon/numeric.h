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
/*!export **/
/*!export DeeNumeric_**/
#ifndef GUARD_DEEMON_NUMERIC_H
#define GUARD_DEEMON_NUMERIC_H 1 /*!export-*/

#include "api.h"

#include "types.h" /* DeeTypeObject */

DECL_BEGIN

/* `Numeric from deemon' - Base class for `float', `int', and `bool'
 *
 *  The main purpose of this type is to query an object for being
 *  a number object (`float' or `int'), or a user-defined numeric
 *  type, similar to how `Sequence' and `Iterator' are builtin
 *  base classes for certain types of numbers.
 *
 *  This type (might eventually) also provides some helpful member/class
 *  functions/getsets that can be used to query information on the number/type. */
DDATDEF DeeTypeObject DeeNumeric_Type;

DECL_END

#endif /* !GUARD_DEEMON_NUMERIC_H */
