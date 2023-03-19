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
#ifndef GUARD_DEX_IPC_PIPE_C_INL
#define GUARD_DEX_IPC_PIPE_C_INL 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/format.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/tuple.h>

#include "libipc.h"

#undef ipc_Pipe_USE_CreatePipe
#undef ipc_Pipe_USE_pipe_OR_pipe2
#undef ipc_Pipe_USE_stub
#ifdef DEESYSTEM_FILE_USE_WINDOWS
#define ipc_Pipe_USE_CreatePipe
#elif (defined(DEESYSTEM_FILE_USE_UNIX) && \
       (defined(CONFIG_HAVE_pipe2) || defined(CONFIG_HAVE_pipe)))
#define ipc_Pipe_USE_pipe_OR_pipe2
#else /* ... */
#define ipc_Pipe_USE_stub
#endif /* !... */

#ifdef ipc_Pipe_USE_CreatePipe
#include <Windows.h>
#endif /* ipc_Pipe_USE_CreatePipe */

#if defined(CONFIG_HAVE_ioctl) && defined(CONFIG_HAVE_FIOCLEX)
#define ipc_unix_set_CLOEXEC(fd) ioctl(fd, FIOCLEX)
#elif defined(CONFIG_HAVE_fcntl) && defined(CONFIG_HAVE_F_SETFD) && defined(CONFIG_HAVE_FD_CLOEXEC)
#ifdef CONFIG_HAVE_F_GETFD
#define ipc_unix_set_CLOEXEC(fd) fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC)
#else /* CONFIG_HAVE_F_GETFD */
#define ipc_unix_set_CLOEXEC(fd) fcntl(fd, F_SETFD, FD_CLOEXEC)
#endif /* CONFIG_HAVE_F_GETFD */
#endif /* !... */


DECL_BEGIN


PRIVATE WUNUSED DREF DeeTupleObject *DCALL
pipe_new_impl(size_t pipe_size) {
#ifdef ipc_Pipe_USE_stub
	(void)pipe_size;
#define WANT_ipc_unimplemented
	ipc_unimplemented();
	return NULL;
#else /* ipc_Pipe_USE_stub */
	DREF DeeSystemFileObject *reader_file;
	DREF DeeSystemFileObject *writer_file;
	DREF DeeTupleObject *result;
	(void)pipe_size;

	/* Allocate file objects and the tuple we want to return later. */
	reader_file = DeeObject_MALLOC(DeeSystemFileObject);
	if unlikely(!reader_file)
		goto err;
	writer_file = DeeObject_MALLOC(DeeSystemFileObject);
	if unlikely(!reader_file)
		goto err_reader_file;
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_reader_file_writer_file;

	/* Allocate the pipe and assign its handles. */
#ifdef ipc_Pipe_USE_CreatePipe
	{
		HANDLE hReader, hWriter;
		DBG_ALIGNMENT_DISABLE();
		if (!CreatePipe(&hReader, &hWriter, NULL, pipe_size)) {
			DWORD dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			DeeNTSystem_ThrowErrorf(NULL, dwError,
			                        "Failed to create pipe (size: %" PRFu32 ")",
			                        pipe_size);
			goto err_reader_file_writer_file_result;
		}
		DBG_ALIGNMENT_ENABLE();
		reader_file->sf_handle    = hReader;
		reader_file->sf_ownhandle = hReader; /* Inherit */
		reader_file->sf_filetype  = FILE_TYPE_PIPE;
		writer_file->sf_handle    = hWriter;
		writer_file->sf_ownhandle = hWriter; /* Inherit */
		writer_file->sf_filetype  = FILE_TYPE_PIPE;
	}
#endif /* ipc_Pipe_USE_CreatePipe */

#ifdef ipc_Pipe_USE_pipe_OR_pipe2
	{
		int pipefds[2];
		DBG_ALIGNMENT_DISABLE();
#if defined(CONFIG_HAVE_pipe2) && defined(CONFIG_HAVE_O_CLOEXEC)
		if (pipe2(pipefds, O_CLOEXEC) != 0)
#elif defined(CONFIG_HAVE_pipe)
#define WANT_TRY_SET_CLOEXEC_ON_PIPES
		if (pipe(pipefds) != 0)
#elif defined(CONFIG_HAVE_pipe2)
#define WANT_TRY_SET_CLOEXEC_ON_PIPES
		if (pipe2(pipefds, 0) != 0)
#else /* ... */
#error "No way of creating pipes"
#endif /* !... */
		{
			int error = DeeSystem_GetErrno();
			DBG_ALIGNMENT_ENABLE();
			DeeUnixSystem_ThrowErrorf(NULL, error,
			                          "Failed to create pipe");
			goto err_reader_file_writer_file_result;
		}
		DBG_ALIGNMENT_ENABLE();

		/* Try to set O_CLOEXEC on pipes */
#ifdef WANT_TRY_SET_CLOEXEC_ON_PIPES
#undef WANT_TRY_SET_CLOEXEC_ON_PIPES
#ifdef ipc_unix_set_CLOEXEC
		(void)ipc_unix_set_CLOEXEC(pipefds[0]);
		(void)ipc_unix_set_CLOEXEC(pipefds[1]);
#endif /* ipc_unix_set_CLOEXEC */
#endif /* WANT_TRY_SET_CLOEXEC_ON_PIPES */

		reader_file->sf_handle    = pipefds[0];
		reader_file->sf_ownhandle = pipefds[0]; /* Inherit */
		writer_file->sf_handle    = pipefds[1];
		writer_file->sf_ownhandle = pipefds[1]; /* Inherit */
	}
#endif /* ipc_Pipe_USE_pipe_OR_pipe2 */

	/* Fill in the result of the to-be returned objects. */
#ifdef DEESYSTEM_FILE_HAVE_sf_filename
	reader_file->sf_filename = NULL;
	writer_file->sf_filename = NULL;
#endif /* DEESYSTEM_FILE_HAVE_sf_filename */
	DeeLFileObject_Init(reader_file, &DeePipeReader_Type);
	DeeLFileObject_Init(writer_file, &DeePipeWriter_Type);
	DeeTuple_SET(result, 0, (DeeObject *)reader_file); /* Inherit reference */
	DeeTuple_SET(result, 1, (DeeObject *)writer_file); /* Inherit reference */
	return result;
err_reader_file_writer_file_result:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err_reader_file_writer_file:
	DeeObject_FREE(writer_file);
err_reader_file:
	DeeObject_FREE(reader_file);
err:
	return NULL;
#endif /* !ipc_Pipe_USE_stub */
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
pipe_class_new(DeeObject *UNUSED(self), size_t argc, DeeObject *const *argv) {
	size_t pipe_size = 0;
	if (DeeArg_Unpack(argc, argv, "|" UNPuSIZ ":new", &pipe_size))
		goto err;
	return pipe_new_impl(pipe_size);
err:
	return NULL;
}


PRIVATE struct type_member tpconst pipe_class_members[] = {
	TYPE_MEMBER_CONST("Reader", &DeePipeReader_Type.ft_base),
	TYPE_MEMBER_CONST("Writer", &DeePipeWriter_Type.ft_base),
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst pipe_class_methods[] = {
	TYPE_METHOD("new", &pipe_class_new,
	            "(size_hint=!0)->?T2?#Reader?#Writer\n"
	            "Creates a new pair of linked pipe files"),
	TYPE_METHOD_END
};

INTERN DeeFileTypeObject DeePipe_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "Pipe",
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
					TYPE_FIXED_ALLOCATOR(DeeSystemFileObject)
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
		/* .tp_class_methods = */ pipe_class_methods,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ pipe_class_members
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

INTERN DeeFileTypeObject DeePipeReader_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "PipeReader",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeePipe_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)NULL,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)NULL,
					TYPE_FIXED_ALLOCATOR(DeeSystemFileObject)
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

INTERN DeeFileTypeObject DeePipeWriter_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "PipeWriter",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeePipe_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)NULL,
					/* .tp_copy_ctor = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor = */ (dfunptr_t)NULL,
					/* .tp_any_ctor  = */ (dfunptr_t)NULL,
					TYPE_FIXED_ALLOCATOR(DeeSystemFileObject)
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

#endif /* !GUARD_DEX_IPC_PIPE_C_INL */
