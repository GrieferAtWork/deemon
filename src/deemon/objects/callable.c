/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_CALLABLE_C
#define GUARD_DEEMON_OBJECTS_CALLABLE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/callable.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include "../runtime/strings.h"

DECL_BEGIN

INTDEF int DCALL none_i1(void *UNUSED(a));
INTDEF int DCALL none_i2(void *UNUSED(a), void *UNUSED(b));

/* `Callable from deemon'
 *
 * Base class for callable wrapper types, such as ObjMethod, CMethod,
 * InstanceMethod or just a plain old function. There is no particular
 * reason why this exists, other than to allow user-code to query
 * for a type for that particular set of objects type simply writing
 * `x is Callable from deemon' */
PUBLIC DeeTypeObject DeeCallable_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Callable),
	/* .tp_doc      = */ DOC("Base class for callable types such as ?DFunction or ?DInstanceMethod, "
	                         /**/ "as well as any implementation-specific, wrapper object type"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_i1,
				/* .tp_copy_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CALLABLE_C */
