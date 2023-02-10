/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_BOOL_H
#define GUARD_DEEMON_CXX_BOOL_H 1

#include "api.h"
/**/

#include "numeric.h"
#include "object.h"
/**/

#include "../format.h"
#include "../bool.h"
/**/

#include <hybrid/typecore.h>

DEE_CXX_BEGIN

class bool_
	: public Numeric
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeBool_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeBool_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeBool_CheckExact(ob);
	}

public:
	static WUNUSED Ref<bool_> false_() {
		return nonnull(Dee_False);
	}
	static WUNUSED Ref<bool_> true_() {
		return nonnull(Dee_True);
	}
	static WUNUSED Ref<bool_> of(bool value) {
		return nonnull(DeeBool_For(value));
	}
public:

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(bool from deemon).printCxxApi();]]]*/

/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_BOOL_H */
