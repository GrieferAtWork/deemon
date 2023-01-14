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
#ifndef GUARD_DEEMON_SYSTEM_GENERIC_FILE_C_INL
#define GUARD_DEEMON_SYSTEM_GENERIC_FILE_C_INL 1
#define _DOS_SOURCE 1
#define _LARGEFILE64_SOURCE 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

PRIVATE char const fs_unsupported_message[] = "A filesystem is not supported";

PRIVATE ATTR_NOINLINE int DCALL fs_unsupported(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       fs_unsupported_message);
}

INTERN DeeSysFD DCALL
DeeSystemFile_Fileno(/*FileSystem*/ DeeObject *__restrict self) {
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
	(void)self;
	return (DeeSysFD)DeeError_Throwf(&DeeError_FileClosed,
	                                 fs_unsupported_message);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSystemFile_Filename(/*SystemFile*/ DeeObject *__restrict self) {
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
	(void)self;
	fs_unsupported();
	return NULL;
}

PUBLIC WUNUSED DREF /*FSFile*/ DeeObject *DCALL
DeeFile_OpenFd(DeeSysFD UNUSED(fd),
               /*String*/ DeeObject *UNUSED(filename),
               int UNUSED(oflags), bool UNUSED(inherit_fd)) {
	fs_unsupported();
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_OpenString(char const *__restrict UNUSED(filename),
                   int UNUSED(oflags), int UNUSED(mode)) {
	fs_unsupported();
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_Open(/*String*/ DeeObject *__restrict UNUSED(filename),
             int UNUSED(oflags), int UNUSED(mode)) {
	fs_unsupported();
	return NULL;
}

PRIVATE DeeFileObject std_file = { FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type) };

/* Return the the default stream for a given STD number. */
PUBLIC ATTR_RETNONNULL DeeObject *DCALL
DeeFile_DefaultStd(unsigned int id) {
	ASSERT(id <= DEE_STDDBG);
	(void)id;
	return (DeeObject *)&std_file;
}


PRIVATE size_t DCALL
sysfile_read(DeeFileObject *__restrict UNUSED(self),
             void *__restrict UNUSED(buffer),
             size_t UNUSED(bufsize),
             dioflag_t UNUSED(flags)) {
	return (size_t)fs_unsupported();
}

PRIVATE size_t DCALL
sysfile_write(DeeFileObject *__restrict UNUSED(self),
              void const *__restrict UNUSED(buffer),
              size_t UNUSED(bufsize),
              dioflag_t UNUSED(flags)) {
	return (size_t)fs_unsupported();
}

PRIVATE dpos_t DCALL
sysfile_seek(DeeFileObject *__restrict UNUSED(self),
             doff_t UNUSED(off), int UNUSED(whence)) {
	return (dpos_t)(doff_t)fs_unsupported();
}

PRIVATE int DCALL
sysfile_sync(DeeFileObject *__restrict UNUSED(self)) {
	return fs_unsupported();
}

PRIVATE int DCALL
sysfile_trunc(DeeFileObject *__restrict UNUSED(self),
              dpos_t UNUSED(size)) {
	return fs_unsupported();
}

PRIVATE int DCALL
sysfile_close(DeeFileObject *__restrict UNUSED(self)) {
	return fs_unsupported();
}

PRIVATE int DCALL
sysfile_getc(DeeFileObject *__restrict UNUSED(self),
             dioflag_t UNUSED(flags)) {
	fs_unsupported();
	return GETC_ERR;
}

PRIVATE int DCALL
sysfile_ungetc(DeeFileObject *__restrict UNUSED(self), int UNUSED(ch)) {
	fs_unsupported();
	return GETC_ERR;
}

PRIVATE int DCALL
sysfile_putc(DeeFileObject *__restrict UNUSED(self), int UNUSED(ch),
             dioflag_t UNUSED(flags)) {
	fs_unsupported();
	return GETC_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_class_sync(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":sync"))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst sysfile_class_methods[] = {
	TYPE_METHOD("sync", &sysfile_class_sync,
	            "()\n"
	            "Synchronize all unwritten data with the host operating system"),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst sysfile_class_members[] = {
	TYPE_MEMBER_CONST("Fs", (DeeTypeObject *)&DeeFSFile_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeFileTypeObject DeeSystemFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_SystemFile",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_HASFILEOPS,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)NULL,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)NULL,
					TYPE_FIXED_ALLOCATOR(DeeFileObject)
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
		/* .tp_class_methods = */ sysfile_class_methods,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ sysfile_class_members
	},
	/* .ft_read   = */ &sysfile_read,
	/* .ft_write  = */ &sysfile_write,
	/* .ft_seek   = */ &sysfile_seek,
	/* .ft_sync   = */ &sysfile_sync,
	/* .ft_trunc  = */ &sysfile_trunc,
	/* .ft_close  = */ &sysfile_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ &sysfile_getc,
	/* .ft_ungetc = */ &sysfile_ungetc,
	/* .ft_putc   = */ &sysfile_putc
};

PUBLIC DeeFileTypeObject DeeFSFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FSFile",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeeSystemFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)NULL,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)NULL,
					TYPE_FIXED_ALLOCATOR(DeeFileObject)
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
	},
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ NULL,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ NULL,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ NULL,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_SYSTEM_GENERIC_FILE_C_INL */
