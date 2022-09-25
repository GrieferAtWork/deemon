/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_SOCKET_ERROR_C
#define GUARD_DEX_SOCKET_ERROR_C 1
#define DEE_SOURCE

#include "libnet.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/file.h>
#include <deemon/object.h>

DECL_BEGIN

#define INIT_ERROR(name, base, children)                       \
	{                                                          \
		OBJECT_HEAD_INIT(&DeeType_Type),                       \
		/* .tp_name     = */ name,                             \
		/* .tp_doc      = */ NULL,                             \
		/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR,     \
		/* .tp_weakrefs = */ 0,                                \
		/* .tp_features = */ TF_NONE,                          \
		/* .tp_base     = */ base,                             \
		/* .tp_init = */ {                                     \
			{                                                  \
				/* .tp_alloc = */ {                            \
					/* .tp_ctor      = */ (dfunptr_t)NULL,     \
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,     \
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,     \
					/* .tp_any_ctor  = */ (dfunptr_t)NULL,     \
					TYPE_FIXED_ALLOCATOR(DeeSystemErrorObject) \
				}                                              \
			},                                                 \
			/* .tp_dtor        = */ NULL,                      \
			/* .tp_assign      = */ NULL,                      \
			/* .tp_move_assign = */ NULL                       \
		},                                                     \
		/* .tp_cast = */ {                                     \
			/* .tp_str  = */ NULL,                             \
			/* .tp_repr = */ NULL,                             \
			/* .tp_bool = */ NULL                              \
		},                                                     \
		/* .tp_call          = */ NULL,                        \
		/* .tp_visit         = */ NULL,                        \
		/* .tp_gc            = */ NULL,                        \
		/* .tp_math          = */ NULL,                        \
		/* .tp_cmp           = */ NULL,                        \
		/* .tp_seq           = */ NULL,                        \
		/* .tp_iter_next     = */ NULL,                        \
		/* .tp_attr          = */ NULL,                        \
		/* .tp_with          = */ NULL,                        \
		/* .tp_buffer        = */ NULL,                        \
		/* .tp_methods       = */ NULL,                        \
		/* .tp_getsets       = */ NULL,                        \
		/* .tp_members       = */ NULL,                        \
		/* .tp_class_methods = */ NULL,                        \
		/* .tp_class_getsets = */ NULL,                        \
		/* .tp_class_members = */ children                     \
	}

PRIVATE struct type_member tpconst neterror_class_members[] = {
	TYPE_MEMBER_CONST("NoSupport", &DeeError_NoSupport),
	TYPE_MEMBER_CONST("NotBound", &DeeError_NotBound),
	TYPE_MEMBER_CONST("NotConnected", &DeeError_NotConnected),
	TYPE_MEMBER_CONST("IsConnected", &DeeError_IsConnected),
	TYPE_MEMBER_CONST("ConnectRefused", &DeeError_ConnectRefused),
	TYPE_MEMBER_CONST("ConnectReset", &DeeError_ConnectReset),
	TYPE_MEMBER_CONST("NetUnreachable", &DeeError_NetUnreachable),
	TYPE_MEMBER_CONST("MessageSize", &DeeError_MessageSize),
	TYPE_MEMBER_CONST("AddrInUse", &DeeError_AddrInUse),
	TYPE_MEMBER_CONST("AddrNotAvail", &DeeError_AddrNotAvail),
	TYPE_MEMBER_CONST("HostNotFound", &DeeError_HostNotFound),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst hostnotfound_class_members[] = {
	TYPE_MEMBER_CONST("NoHostAddress", &DeeError_NoHostAddress),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst netnotbound_class_members[] = {
	TYPE_MEMBER_CONST("NotListening", &DeeError_NotListening),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst netunreachable_class_members[] = {
	TYPE_MEMBER_CONST("HostUnreachable", &DeeError_HostUnreachable),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst connectreset_class_members[] = {
	TYPE_MEMBER_CONST("TimedOut", &DeeError_TimedOut),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeError_NetError = INIT_ERROR("NetError", &DeeError_SystemError, neterror_class_members);

INTERN DeeTypeObject DeeError_NoSupport       = INIT_ERROR("NoSupport", &DeeError_NetError, NULL);
INTERN DeeTypeObject DeeError_NotBound        = INIT_ERROR("NotBound", &DeeError_NetError, netnotbound_class_members);
INTERN DeeTypeObject DeeError_NotListening    = INIT_ERROR("NotListening", &DeeError_NotBound, NULL);
INTERN DeeTypeObject DeeError_NotConnected    = INIT_ERROR("NotConnected", &DeeError_NetError, NULL);
INTERN DeeTypeObject DeeError_IsConnected     = INIT_ERROR("IsConnected", &DeeError_NetError, NULL);
INTERN DeeTypeObject DeeError_ConnectRefused  = INIT_ERROR("ConnectRefused", &DeeError_NetError, NULL);
INTERN DeeTypeObject DeeError_ConnectReset    = INIT_ERROR("ConnectReset", &DeeError_NetError, connectreset_class_members);
INTERN DeeTypeObject DeeError_TimedOut        = INIT_ERROR("TimedOut", &DeeError_ConnectReset, NULL);
INTERN DeeTypeObject DeeError_NetUnreachable  = INIT_ERROR("NetUnreachable", &DeeError_NetError, netunreachable_class_members);
INTERN DeeTypeObject DeeError_HostUnreachable = INIT_ERROR("HostUnreachable", &DeeError_NetUnreachable, NULL);
INTERN DeeTypeObject DeeError_MessageSize     = INIT_ERROR("MessageSize", &DeeError_NetError, NULL);
INTERN DeeTypeObject DeeError_AddrInUse       = INIT_ERROR("AddrInUse", &DeeError_NetError, NULL);
INTERN DeeTypeObject DeeError_AddrNotAvail    = INIT_ERROR("AddrNotAvail", &DeeError_NetError, NULL);
INTERN DeeTypeObject DeeError_HostNotFound    = INIT_ERROR("HostNotFound", &DeeError_NetError, hostnotfound_class_members);
INTERN DeeTypeObject DeeError_NoHostAddress   = INIT_ERROR("NoHostAddress", &DeeError_HostNotFound, NULL);


INTERN NONNULL((2)) int DCALL
err_socket_closed(neterrno_t err, DeeSocketObject *__restrict self) {
	return DeeNet_ThrowErrorf(&DeeError_FileClosed, err,
	                          "Socket %k has been closed", self);
}

DECL_END


#endif /* !GUARD_DEX_SOCKET_ERROR_C */
