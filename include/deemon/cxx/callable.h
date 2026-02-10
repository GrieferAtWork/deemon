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
#ifndef GUARD_DEEMON_CXX_CALLABLE_H
#define GUARD_DEEMON_CXX_CALLABLE_H 1

#include "../api.h"
#include "api.h"

#include "../callable.h" /* DeeCallable_Type */
#include "../object.h"   /* DeeObject_CallAttrStringHash */
#include "../types.h"    /* DeeObject, DeeObject_InstanceOf, DeeObject_InstanceOfExact, _Dee_HashSelectC */
#include "object.h"

#include <stdbool.h> /* bool */

DEE_CXX_BEGIN

class Type;

class Callable
	: public Object
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeCallable_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeCallable_Type);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeCallable_Type);
	}

public:

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Callable from deemon).printCxxApi();]]]*/
	WUNUSED NONNULL_CXX((1)) Ref<Object> (compose)(DeeObject *before) {
		DeeObject *args[1];
		args[0] = before;
		return inherit(DeeObject_CallAttrStringHash(this, "compose", _Dee_HashSelectC(0xcf4f7642, 0xdf765e13ceb0315e), 1, args));
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_CALLABLE_H */
