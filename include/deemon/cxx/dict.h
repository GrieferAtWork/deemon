/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_DICT_H
#define GUARD_DEEMON_CXX_DICT_H 1

#include "api.h"

#include "../dict.h"
#include "mapping.h"
#include "object.h"

DEE_CXX_BEGIN

template<class KeyType = Object, class ValueType = Object> class Dict;

template<class KeyType, class ValueType>
class Dict: public Mapping<KeyType, ValueType> {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeDict_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeDict_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeDict_CheckExact(ob);
	}

public: /* Dict from deemon */
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(Dict, Mapping<KeyType, ValueType>)
	Dict()
	    : Mapping<KeyType, ValueType>(inherit(DeeDict_New())) {}
};



DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_DICT_H */
