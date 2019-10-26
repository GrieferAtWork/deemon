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
#ifndef GUARD_DEX_FS_GENERIC_USER_C_INL
#define GUARD_DEX_FS_GENERIC_USER_C_INL 1

#ifdef __INTELLISENSE__
#include "generic.c.inl"
#endif

#include "_res.h"

INTERN WUNUSED DREF DeeObject *DCALL
fs_gethome(bool try_get) {
	if (!try_get)
		fs_unsupported();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
fs_getuser(bool try_get) {
	if (!try_get)
		fs_unsupported();
	return NULL;
}

INTERN int DCALL
fs_printhome(struct unicode_printer *__restrict UNUSED(printer), bool try_get) {
	if (try_get)
		return 1;
	return fs_unsupported();
}

INTERN int DCALL
fs_printuser(struct unicode_printer *__restrict UNUSED(printer), bool try_get) {
	if (try_get)
		return 1;
	return fs_unsupported();
}

INTERN DeeTypeObject DeeUser_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_User_tp_name,
	/* .tp_doc      = */ S_User_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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

#endif /* !GUARD_DEX_FS_GENERIC_USER_C_INL */
