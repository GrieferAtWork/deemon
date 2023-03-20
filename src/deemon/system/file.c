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
#ifndef GUARD_DEEMON_SYSTEM_FILE_C
#define GUARD_DEEMON_SYSTEM_FILE_C 1

#include <deemon/api.h>
#include <deemon/filetypes.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include <hybrid/byteorder.h>
#include <hybrid/host.h>
#include <hybrid/minmax.h>
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h>
#include <hybrid/wordbits.h>

#ifdef CONFIG_HAVE_LIMITS_H
#include <limits.h>
#endif /* CONFIG_HAVE_LIMITS_H */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */

#ifndef UINT32_MAX
#include <hybrid/limitcore.h>
#define UINT32_MAX __UINT32_MAX__
#endif /* !UINT32_MAX */

#ifdef DEESYSTEM_FILE_USE_WINDOWS
#include <Windows.h>
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_STDIO
#include <stdio.h>
#endif /* DEESYSTEM_FILE_USE_STDIO */




/************************************************************************/
/* Auto-configure system features for `DEESYSTEM_FILE_USE_UNIX'-mode    */
/************************************************************************/
#ifdef DEESYSTEM_FILE_USE_UNIX
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif /* !STDIN_FILENO */
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif /* !STDOUT_FILENO */
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif /* !STDERR_FILENO */
#endif /* DEESYSTEM_FILE_USE_UNIX */



/************************************************************************/
/* Auto-configure system features for `DEESYSTEM_FILE_USE_STDIO'-mode   */
/************************************************************************/
#ifdef CONFIG_HAVE_fopen64
#undef fopen
#define fopen(filename, mode) fopen64(filename, mode)
#endif /* CONFIG_HAVE_fopen64 */

#ifndef CONFIG_HAVE_stdin
#undef stdin
#ifdef CONFIG_HAVE_fopen_binary
#define stdin fopen("/dev/stdin", "rb")
#else /* CONFIG_HAVE_fopen_binary */
#define stdin fopen("/dev/stdin", "r")
#endif /* !CONFIG_HAVE_fopen_binary */
#endif /* !CONFIG_HAVE_stdin */

#ifndef CONFIG_HAVE_stdout
#undef stdout
#ifdef CONFIG_HAVE_fopen_binary
#define stdout fopen("/dev/stdout", "rb")
#else /* CONFIG_HAVE_fopen_binary */
#define stdout fopen("/dev/stdout", "r")
#endif /* !CONFIG_HAVE_fopen_binary */
#endif /* !CONFIG_HAVE_stdout */

#ifndef CONFIG_HAVE_stderr
#undef stderr
#ifdef CONFIG_HAVE_stdout
#define stderr stdout
#elif defined(CONFIG_HAVE_fopen_binary)
#define stderr fopen("/dev/stderr", "rb")
#else /* CONFIG_HAVE_fopen_binary */
#define stderr fopen("/dev/stderr", "r")
#endif /* !CONFIG_HAVE_fopen_binary */
#endif /* !CONFIG_HAVE_stderr */

#ifndef EOF
#define EOF (-1)
#endif /* !EOF */

DECL_BEGIN

typedef DeeSystemFileObject SystemFile;




/************************************************************************/
/* Special support for `OutputDebugStringA()'                           */
/************************************************************************/
#ifdef DEE_STDDBG_IS_UNIQUE
#ifndef CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
#define CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
extern ATTR_DLLIMPORT void ATTR_STDCALL OutputDebugStringA(char const *lpOutputString);
extern ATTR_DLLIMPORT int ATTR_STDCALL IsDebuggerPresent(void);
#endif /* !CONFIG_OUTPUTDEBUGSTRINGA_DEFINED */

PRIVATE size_t DCALL
debugfile_write(DeeFileObject *__restrict UNUSED(self),
                void const *__restrict buffer,
                size_t bufsize, dioflag_t UNUSED(flags)) {
	size_t result;
	/* Forward all data to stderr. */
	result = DeeFile_Write(DeeFile_DefaultStderr, buffer, bufsize);
	if unlikely(result == 0 || result == (size_t)-1)
		goto done;
	if (bufsize > result)
		bufsize = result;
	if (IsDebuggerPresent()) {
#ifdef __ARCH_PAGESIZE_MIN
		/* (ab-)use the fact that the kernel can't keep us from reading
		 * beyond the end of a buffer so long as that memory location
		 * is located within the same page as the last byte of said
		 * buffer (Trust me... I've written by own OS) */
		if ((bufsize <= 1000) && /* There seems to be some kind of limit here... */
		    (((uintptr_t)buffer + bufsize) & ~(uintptr_t)(__ARCH_PAGESIZE_MIN - 1)) ==
		    (((uintptr_t)buffer + bufsize - 1) & ~(uintptr_t)(__ARCH_PAGESIZE_MIN - 1)) &&
		    (*(char *)((uintptr_t)buffer + bufsize)) == '\0') {
			DBG_ALIGNMENT_DISABLE();
			OutputDebugStringA((char *)buffer);
			DBG_ALIGNMENT_ENABLE();
		} else
#endif /* __ARCH_PAGESIZE_MIN */
		{
			char temp[512];
			while (bufsize) {
				size_t part = MIN(bufsize, sizeof(temp) - sizeof(char));
				*(char *)mempcpyc(temp, buffer, part, sizeof(char)) = '\0';
				DBG_ALIGNMENT_DISABLE();
				OutputDebugStringA(temp);
				DBG_ALIGNMENT_ENABLE();
				*(uintptr_t *)&buffer += part;
				bufsize -= part;
			}
		}
	}
done:
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL debugfile_get(void) {
	return_reference(DeeFile_DefaultStddbg);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
debugfile_isatty(DeeObject *__restrict UNUSED(self)) {
	/* Considering its purpose, always act as though _DebugFile
	 * is a TTY device, just so automatic buffer interfaces will
	 * act as line-oriented buffers. */
	return_true;
}

PRIVATE struct type_getset tpconst debug_file_getsets[] = {
	TYPE_GETTER(STR_isatty, &debugfile_isatty, "->?Dbool"),
	TYPE_GETSET_END
};


PRIVATE DeeFileTypeObject DebugFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_DebugFile",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_HASFILEOPS,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_var = */ {
					/* .tp_ctor      = */ (dfunptr_t)&debugfile_get,
					/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
					/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
					/* .tp_any_ctor  = */ (dfunptr_t)NULL
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
		/* .tp_getsets       = */ debug_file_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ &debugfile_write,
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
#endif /* DEE_STDDBG_IS_UNIQUE */


#ifdef DEESYSTEM_FILE_USE_STUB
PRIVATE char const fs_unsupported_message[] = "No filesystem supported";
PRIVATE ATTR_NOINLINE int DCALL fs_unsupported(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       fs_unsupported_message);
}
#else /* DEESYSTEM_FILE_USE_STUB */

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_file_closed(SystemFile *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_FileClosed,
	                       "File was closed");
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_file_io(SystemFile *__restrict self) {
	if (self->sf_handle == DeeSysFD_INVALID)
		return err_file_closed(self);
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	return DeeNTSystem_ThrowErrorf(&DeeError_FSError,
	                               GetLastError(),
	                               "I/O Operation failed");
#elif defined(DEESYSTEM_FILE_USE_UNIX)
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError,
	                                 DeeSystem_GetErrno(),
	                                 "I/O Operation failed");
#else /* ... */
	return DeeError_Throwf(&DeeError_FSError, "I/O Operation failed");
#endif /* !... */
}

#endif /* !DEESYSTEM_FILE_USE_STUB */




PUBLIC WUNUSED DREF /*SystemFile*/ DeeObject *DCALL
DeeFile_OpenFd(DeeSysFD fd, /*String*/ DeeObject *filename,
               int oflags, bool inherit_fd) {
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)fd;
	(void)filename;
	(void)oflags;
	(void)inherit_fd;
	fs_unsupported();
	return NULL;
#else /* DEESYSTEM_FILE_USE_STUB */
	SystemFile *result;
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto done;

	/* Os-specific part. */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	(void)oflags;
	result->sf_handle    = fd;
	result->sf_ownhandle = inherit_fd ? fd : INVALID_HANDLE_VALUE; /* Inherit. */
	result->sf_filename  = filename;
	result->sf_filetype  = (uint32_t)FILE_TYPE_UNKNOWN;
	result->sf_pendingc  = 0;
	Dee_XIncref(filename);
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_UNIX
	(void)oflags;
	result->sf_handle    = fd;
	result->sf_ownhandle = inherit_fd ? fd : (DeeSysFD)-1; /* Inherit. */
	result->sf_filename  = filename;
	Dee_XIncref(filename);
#endif /* DEESYSTEM_FILE_USE_UNIX */

#ifdef DEESYSTEM_FILE_USE_STDIO
	(void)oflags;
	result->sf_handle    = (FILE *)fd;
	result->sf_ownhandle = inherit_fd ? (FILE *)fd : NULL; /* Inherit. */
	result->sf_filename  = filename;
	Dee_XIncref(filename);
#endif /* DEESYSTEM_FILE_USE_STDIO */

	DeeObject_Init(result, &DeeSystemFile_Type);
done:
	return (DREF DeeObject *)result;
#endif /* !DEESYSTEM_FILE_USE_STUB */
}


INTERN WUNUSED NONNULL((1)) DeeSysFD DCALL
DeeSystemFile_Fileno(/*FileSystem*/ DeeObject *__restrict self) {
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	return (DeeSysFD)DeeError_Throwf(&DeeError_FileClosed, fs_unsupported_message);
#elif defined(DEESYSTEM_FILE_USE_STDIO) && 1
#define CONFIG_DONT_EXPOSE_FILENO
	/* Due to the unpredictable race condition and the fact that
	 * it's up to the kernel to deal with closed file descriptors,
	 * we could never safely expose the underlying FILE * to the
	 * user... */
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "The host does not implement a safe way of using fileno()");
	return NULL;
#else /* ... */
	DeeSysFD result;
	SystemFile *me = (SystemFile *)self;
	ASSERT_OBJECT_TYPE((DeeObject *)me, (DeeTypeObject *)&DeeSystemFile_Type);
	result = (DeeSysFD)me->sf_handle;
	if (result == DeeSysFD_INVALID)
		err_file_closed(me);
	return result;
#endif /* !... */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSystemFile_Filename(/*SystemFile*/ DeeObject *__restrict self) {
#ifdef DEESYSTEM_FILE_USE_STUB
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
	(void)self;
	fs_unsupported();
	return NULL;
#elif defined(DEESYSTEM_FILE_USE_STDIO)
	SystemFile *me = (SystemFile *)self;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE((DeeObject *)me, (DeeTypeObject *)&DeeSystemFile_Type);
	result = me->sf_filename;
	if unlikely(!result)
		err_cant_access_attribute(Dee_TYPE(me), STR_filename, ATTR_ACCESS_GET);
	return NULL;
#else /* ... */
	SystemFile *me = (SystemFile *)self;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE((DeeObject *)me, (DeeTypeObject *)&DeeSystemFile_Type);
	result = atomic_read(&me->sf_filename);
	if (result) {
		Dee_Incref(result);
	} else {
		DeeSysFD hand = (DeeSysFD)me->sf_handle;
		if unlikely(hand == DeeSysFD_INVALID) {
			err_file_closed(me);
			goto done;
		}
#ifdef DEESYSTEM_FILE_USE_WINDOWS
		result = DeeNTSystem_GetFilenameOfHandle(hand);
#endif /* DEESYSTEM_FILE_USE_WINDOWS */
#ifdef DEESYSTEM_FILE_USE_UNIX
		result = DeeSystem_GetFilenameOfFD(hand);
#endif /* DEESYSTEM_FILE_USE_UNIX */
		if unlikely(!result)
			goto done;

		/* Lazily remember the generated filename. */
		if (!atomic_cmpxch(&me->sf_filename, NULL, result)) {
			Dee_Decref(result);
			result = me->sf_filename;
			Dee_Incref(result);
		}
	}
done:
	return result;
#endif /* !... */
}




/************************************************************************/
/* File open API                                                        */
/************************************************************************/


#ifdef DEESYSTEM_FILE_USE_WINDOWS
PRIVATE DWORD const generic_access[4] = {
	/* [OPEN_FRDONLY] = */ FILE_GENERIC_READ,
	/* [OPEN_FWRONLY] = */ FILE_GENERIC_WRITE,
	/* [OPEN_FRDWR]   = */ FILE_GENERIC_READ | FILE_GENERIC_WRITE,
	/* [0x3]          = */ FILE_GENERIC_READ | FILE_GENERIC_WRITE
};
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_UNIX
/* Fix names of aliasing flags. */
#ifndef CONFIG_HAVE_O_CREAT
#error "Missing system support for `O_CREAT'"
#endif /* !CONFIG_HAVE_O_CREAT */
#ifndef CONFIG_HAVE_O_EXCL
#error "Missing system support for `O_EXCL'"
#endif /* !CONFIG_HAVE_O_EXCL */
#ifndef CONFIG_HAVE_O_TRUNC
#error "Missing system support for `O_TRUNC'"
#endif /* !CONFIG_HAVE_O_TRUNC */
#ifndef CONFIG_HAVE_O_APPEND
#error "Missing system support for `O_APPEND'"
#endif /* !CONFIG_HAVE_O_APPEND */

#define PRIVATE_SHARED_FLAGS_0   0
#define PRIVATE_NOSUPP_FLAGS_0   0
#define PRIVATE_NOSHAR_FLAGS_0   0

#if OPEN_FCREAT == O_CREAT
#define PRIVATE_NOSHAR_FLAGS_1   PRIVATE_NOSHAR_FLAGS_0
#define PRIVATE_SHARED_FLAGS_1   PRIVATE_SHARED_FLAGS_0 | OPEN_FCREAT
#else /* OPEN_FCREAT == O_CREAT */
#define PRIVATE_NOSHAR_FLAGS_1   PRIVATE_NOSHAR_FLAGS_0 | OPEN_FCREAT
#define PRIVATE_SHARED_FLAGS_1   PRIVATE_SHARED_FLAGS_0
#endif /* OPEN_FCREAT != O_CREAT */
#if OPEN_FEXCL == O_EXCL
#define PRIVATE_NOSHAR_FLAGS_2   PRIVATE_NOSHAR_FLAGS_1
#define PRIVATE_SHARED_FLAGS_2   PRIVATE_SHARED_FLAGS_1 | OPEN_FEXCL
#else /* OPEN_FEXCL == O_EXCL */
#define PRIVATE_NOSHAR_FLAGS_2   PRIVATE_NOSHAR_FLAGS_1 | OPEN_FEXCL
#define PRIVATE_SHARED_FLAGS_2   PRIVATE_SHARED_FLAGS_1
#endif /* OPEN_FEXCL != O_EXCL */
#if OPEN_FTRUNC == O_TRUNC
#define PRIVATE_NOSHAR_FLAGS_3   PRIVATE_NOSHAR_FLAGS_2
#define PRIVATE_SHARED_FLAGS_3   PRIVATE_SHARED_FLAGS_2 | OPEN_FTRUNC
#else /* OPEN_FTRUNC == O_TRUNC */
#define PRIVATE_NOSHAR_FLAGS_3   PRIVATE_NOSHAR_FLAGS_2 | OPEN_FTRUNC
#define PRIVATE_SHARED_FLAGS_3   PRIVATE_SHARED_FLAGS_2
#endif /* OPEN_FTRUNC != O_TRUNC */
#if OPEN_FAPPEND == O_APPEND
#define PRIVATE_NOSHAR_FLAGS_4   PRIVATE_NOSHAR_FLAGS_3
#define PRIVATE_SHARED_FLAGS_4   PRIVATE_SHARED_FLAGS_3 | OPEN_FAPPEND
#else /* OPEN_FAPPEND == O_APPEND */
#define PRIVATE_NOSHAR_FLAGS_4   PRIVATE_NOSHAR_FLAGS_3 | OPEN_FAPPEND
#define PRIVATE_SHARED_FLAGS_4   PRIVATE_SHARED_FLAGS_3
#endif /* OPEN_FAPPEND != O_APPEND */

/* Optional flags. */
#define PRIVATE_NOSUPP_FLAGS_4   PRIVATE_NOSUPP_FLAGS_0
#ifndef CONFIG_HAVE_O_NONBLOCK
#define PRIVATE_NOSUPP_FLAGS_5   PRIVATE_NOSUPP_FLAGS_4 | OPEN_FNONBLOCK
#define PRIVATE_SHARED_FLAGS_5   PRIVATE_SHARED_FLAGS_4
#define PRIVATE_NOSHAR_FLAGS_5   PRIVATE_NOSHAR_FLAGS_4
#else /* !CONFIG_HAVE_O_NONBLOCK */
#define PRIVATE_NOSUPP_FLAGS_5   PRIVATE_NOSUPP_FLAGS_4
#if OPEN_FNONBLOCK == O_NONBLOCK
#define PRIVATE_NOSHAR_FLAGS_5   PRIVATE_NOSHAR_FLAGS_4
#define PRIVATE_SHARED_FLAGS_5   PRIVATE_SHARED_FLAGS_4 | OPEN_FNONBLOCK
#else /* OPEN_FNONBLOCK == O_NONBLOCK */
#define PRIVATE_NOSHAR_FLAGS_5   PRIVATE_NOSHAR_FLAGS_4 | OPEN_FNONBLOCK
#define PRIVATE_SHARED_FLAGS_5   PRIVATE_SHARED_FLAGS_4
#endif /* OPEN_FNONBLOCK != O_NONBLOCK */
#endif /* O_NONBLOCK */
#ifndef CONFIG_HAVE_O_SYNC
#define PRIVATE_NOSUPP_FLAGS_6   PRIVATE_NOSUPP_FLAGS_5 | OPEN_FSYNC
#define PRIVATE_SHARED_FLAGS_6   PRIVATE_SHARED_FLAGS_5
#define PRIVATE_NOSHAR_FLAGS_6   PRIVATE_NOSHAR_FLAGS_5
#else /* CONFIG_HAVE_O_SYNC */
#define PRIVATE_NOSUPP_FLAGS_6   PRIVATE_NOSUPP_FLAGS_5
#if OPEN_FSYNC == O_SYNC
#define PRIVATE_NOSHAR_FLAGS_6   PRIVATE_NOSHAR_FLAGS_5
#define PRIVATE_SHARED_FLAGS_6   PRIVATE_SHARED_FLAGS_5 | OPEN_FSYNC
#else /* OPEN_FSYNC == O_SYNC */
#define PRIVATE_NOSHAR_FLAGS_6   PRIVATE_NOSHAR_FLAGS_5 | OPEN_FSYNC
#define PRIVATE_SHARED_FLAGS_6   PRIVATE_SHARED_FLAGS_5
#endif /* OPEN_FSYNC != O_SYNC */
#endif /* !CONFIG_HAVE_O_SYNC */
#ifndef CONFIG_HAVE_O_DIRECT
#define PRIVATE_NOSUPP_FLAGS_7   PRIVATE_NOSUPP_FLAGS_6 | OPEN_FDIRECT
#define PRIVATE_SHARED_FLAGS_7   PRIVATE_SHARED_FLAGS_6
#define PRIVATE_NOSHAR_FLAGS_7   PRIVATE_NOSHAR_FLAGS_6
#else /* CONFIG_HAVE_O_DIRECT */
#define PRIVATE_NOSUPP_FLAGS_7   PRIVATE_NOSUPP_FLAGS_6
#if OPEN_FDIRECT == O_DIRECT
#define PRIVATE_NOSHAR_FLAGS_7   PRIVATE_NOSHAR_FLAGS_6
#define PRIVATE_SHARED_FLAGS_7   PRIVATE_SHARED_FLAGS_6 | OPEN_FDIRECT
#else /* OPEN_FDIRECT == O_DIRECT */
#define PRIVATE_NOSHAR_FLAGS_7   PRIVATE_NOSHAR_FLAGS_6 | OPEN_FDIRECT
#define PRIVATE_SHARED_FLAGS_7   PRIVATE_SHARED_FLAGS_6
#endif /* OPEN_FDIRECT != O_DIRECT */
#endif /* !CONFIG_HAVE_O_DIRECT */
#ifndef CONFIG_HAVE_O_NOFOLLOW
#define PRIVATE_NOSUPP_FLAGS_8   PRIVATE_NOSUPP_FLAGS_7 | OPEN_FNOFOLLOW
#define PRIVATE_SHARED_FLAGS_8   PRIVATE_SHARED_FLAGS_7
#define PRIVATE_NOSHAR_FLAGS_8   PRIVATE_NOSHAR_FLAGS_7
#else /* CONFIG_HAVE_O_NOFOLLOW */
#define PRIVATE_NOSUPP_FLAGS_8   PRIVATE_NOSUPP_FLAGS_7
#if OPEN_FNOFOLLOW == O_NOFOLLOW
#define PRIVATE_NOSHAR_FLAGS_8   PRIVATE_NOSHAR_FLAGS_7
#define PRIVATE_SHARED_FLAGS_8   PRIVATE_SHARED_FLAGS_7 | OPEN_FNOFOLLOW
#else /* OPEN_FNOFOLLOW == O_NOFOLLOW */
#define PRIVATE_NOSHAR_FLAGS_8   PRIVATE_NOSHAR_FLAGS_7 | OPEN_FNOFOLLOW
#define PRIVATE_SHARED_FLAGS_8   PRIVATE_SHARED_FLAGS_7
#endif /* OPEN_FNOFOLLOW != O_NOFOLLOW */
#endif /* !CONFIG_HAVE_O_NOFOLLOW */
#ifndef CONFIG_HAVE_O_NOATIME
#define PRIVATE_NOSUPP_FLAGS_9   PRIVATE_NOSUPP_FLAGS_8 | OPEN_FNOATIME
#define PRIVATE_SHARED_FLAGS_9   PRIVATE_SHARED_FLAGS_8
#define PRIVATE_NOSHAR_FLAGS_9   PRIVATE_NOSHAR_FLAGS_8
#else /* CONFIG_HAVE_O_NOATIME */
#define PRIVATE_NOSUPP_FLAGS_9   PRIVATE_NOSUPP_FLAGS_8
#if OPEN_FNOATIME == O_NOATIME
#define PRIVATE_NOSHAR_FLAGS_9   PRIVATE_NOSHAR_FLAGS_8
#define PRIVATE_SHARED_FLAGS_9   PRIVATE_SHARED_FLAGS_8 | OPEN_FNOATIME
#else /* OPEN_FNOATIME == O_NOATIME */
#define PRIVATE_NOSHAR_FLAGS_9   PRIVATE_NOSHAR_FLAGS_8 | OPEN_FNOATIME
#define PRIVATE_SHARED_FLAGS_9   PRIVATE_SHARED_FLAGS_8
#endif /* OPEN_FNOATIME != O_NOATIME */
#endif /* !CONFIG_HAVE_O_NOATIME */

#ifndef CONFIG_HAVE_O_CLOEXEC
#define PRIVATE_NOSUPP_FLAGS_A   PRIVATE_NOSUPP_FLAGS_9 | OPEN_FCLOEXEC
#define PRIVATE_SHARED_FLAGS_A   PRIVATE_SHARED_FLAGS_9
#define PRIVATE_NOSHAR_FLAGS_A   PRIVATE_NOSHAR_FLAGS_9
#else /* CONFIG_HAVE_O_CLOEXEC */
#define PRIVATE_NOSUPP_FLAGS_A   PRIVATE_NOSUPP_FLAGS_9
#if OPEN_FCLOEXEC == O_CLOEXEC
#define PRIVATE_NOSHAR_FLAGS_A   PRIVATE_NOSHAR_FLAGS_9
#define PRIVATE_SHARED_FLAGS_A   PRIVATE_SHARED_FLAGS_9 | OPEN_FCLOEXEC
#else /* OPEN_FCLOEXEC == O_CLOEXEC */
#define PRIVATE_NOSHAR_FLAGS_A   PRIVATE_NOSHAR_FLAGS_9 | OPEN_FCLOEXEC
#define PRIVATE_SHARED_FLAGS_A   PRIVATE_SHARED_FLAGS_9
#endif /* OPEN_FCLOEXEC != O_CLOEXEC */
#endif /* !CONFIG_HAVE_O_CLOEXEC */

#define NOSUPP_FLAGS   (PRIVATE_NOSUPP_FLAGS_A) /* Unsupported options. */
#define SHARED_FLAGS   (PRIVATE_SHARED_FLAGS_A) /* Options with which we share bits. */
#define NOSHAR_FLAGS   (PRIVATE_NOSHAR_FLAGS_A) /* Options with which we don't share bits. */

#ifndef CONFIG_HAVE_O_RDONLY
#undef O_RDONLY
#define O_RDONLY 0
#endif /* !CONFIG_HAVE_O_RDONLY */

#ifndef CONFIG_HAVE_O_WRONLY
#undef O_WRONLY
#define O_WRONLY 0
#endif /* !CONFIG_HAVE_O_WRONLY */

#ifndef CONFIG_HAVE_O_RDWR
#undef O_RDWR
#define O_RDWR 0
#endif /* !CONFIG_HAVE_O_RDWR */
#endif /* DEESYSTEM_FILE_USE_UNIX */




PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_Open(/*String*/ DeeObject *__restrict filename, int oflags, int mode) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	DREF SystemFile *result;
	HANDLE hFile;
	DWORD dwDesiredAccess, dwShareMode;
	DWORD dwCreationDisposition, dwFlagsAndAttributes;
	dwDesiredAccess      = generic_access[oflags & OPEN_FACCMODE];
	dwShareMode          = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
	dwFlagsAndAttributes = (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS);
	/* Apply exclusivity flags. */
	if (oflags & OPEN_FXREAD)
		dwShareMode &= ~(FILE_SHARE_READ);
	if (oflags & OPEN_FXWRITE)
		dwShareMode &= ~(FILE_SHARE_WRITE);
	if (oflags & OPEN_FCREAT) {
		if (oflags & OPEN_FEXCL) {
			dwCreationDisposition = CREATE_NEW;
		} else {
			dwCreationDisposition = ((oflags & OPEN_FTRUNC)
			                         ? CREATE_ALWAYS
			                         : OPEN_ALWAYS);
		}
		if (!(mode & 0444))
			dwFlagsAndAttributes |= FILE_ATTRIBUTE_READONLY;
	} else {
		dwCreationDisposition = ((oflags & OPEN_FTRUNC)
		                         ? TRUNCATE_EXISTING
		                         : OPEN_EXISTING);
	}
	if ((oflags & OPEN_FAPPEND) &&
	    (oflags & OPEN_FACCMODE) != OPEN_FRDONLY) {
#if (FILE_GENERIC_WRITE & FILE_APPEND_DATA) == 0
		dwDesiredAccess |= FILE_APPEND_DATA;
#endif /* (FILE_GENERIC_WRITE & FILE_APPEND_DATA) == 0 */
		dwDesiredAccess &= ~FILE_WRITE_DATA;
	}
	if (oflags & (OPEN_FDIRECT | OPEN_FSYNC))
		dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
	if (oflags & OPEN_FHIDDEN)
		dwFlagsAndAttributes |= FILE_ATTRIBUTE_HIDDEN;
again:
	if (oflags & OPEN_FNOATIME) {
		hFile = DeeNTSystem_CreateFileNoATime(filename, dwDesiredAccess, dwShareMode, NULL,
		                                      dwCreationDisposition, dwFlagsAndAttributes, NULL);
	} else {
		hFile = DeeNTSystem_CreateFile(filename, dwDesiredAccess, dwShareMode, NULL,
		                               dwCreationDisposition, dwFlagsAndAttributes, NULL);
	}
	if unlikely(!hFile)
		goto err;
	if unlikely(hFile == INVALID_HANDLE_VALUE) {
		DWORD error;
		DBG_ALIGNMENT_DISABLE();
		error = GetLastError();
		DBG_ALIGNMENT_ENABLE();

/*check_nt_error:*/
		/* Handle file already-exists. */
		if ((error == ERROR_FILE_EXISTS) &&
		    (oflags & OPEN_FEXCL))
			return ITER_DONE;

		/* Throw the error as an NT error. */
		if (DeeNTSystem_IsBadAllocError(error)) {
			if (Dee_CollectMemory(1))
				goto again;
		} else if (DeeNTSystem_IsNotDir(error) ||
		           DeeNTSystem_IsFileNotFoundError(error)) {
			if (!(oflags & OPEN_FCREAT))
				return ITER_DONE;
			DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, error,
			                        "File %r could not be found",
			                        filename);
		} else if (DeeNTSystem_IsAccessDeniedError(error)) {
			DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, error,
			                        "Access has not been granted for file %r",
			                        filename);
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_FSError, error,
			                        "Failed to obtain a writable handle for %r",
			                        filename);
		}
		goto err;
	}

	/* Simple case: The open was successful. */
	Dee_Incref(filename);
#if 0 /* XXX: Only if `fp' is a pipe */
	{
		DWORD new_mode = oflags & OPEN_FNONBLOCK ? PIPE_NOWAIT : PIPE_WAIT;
		DBG_ALIGNMENT_DISABLE();
		SetNamedPipeHandleState(fp, &new_mode, NULL, NULL);
		DBG_ALIGNMENT_ENABLE();
	}
#endif
#if 0 /* Technically we'd need to do this, but then again: \
       * Windows doesn't even have fork (natively...) */
	if (!(oflags & OPEN_FCLOEXEC)) {
		DBG_ALIGNMENT_DISABLE();
		SetHandleInformation(fp, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		DBG_ALIGNMENT_ENABLE();
	}
#endif
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto err_fp;
	DeeObject_Init(result, &DeeFSFile_Type);
	result->sf_handle    = hFile;
	result->sf_ownhandle = hFile;    /* Inherit handle. */
	result->sf_filename  = filename; /* Inherit reference. */
	result->sf_filetype  = (uint32_t)FILE_TYPE_UNKNOWN;
	result->sf_pendingc  = 0;
	return (DREF DeeObject *)result;
err_fp:
	CloseHandle(hFile);
/*err_filename:*/
	Dee_Decref(filename);
err:
	return NULL;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
	DREF SystemFile *result;
	int fd, used_oflags;
	char *utf8_filename;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
#if O_RDONLY == OPEN_FRDONLY && O_WRONLY == OPEN_FWRONLY && O_RDWR == OPEN_FRDWR
	used_oflags = oflags & (SHARED_FLAGS | O_RDONLY | O_WRONLY | O_RDWR);
#else /* O_RDONLY == OPEN_FRDONLY && O_WRONLY == OPEN_FWRONLY && O_RDWR == OPEN_FRDWR */
	{
		PRIVATE int const accmode_flags[4] = {
			/* [OPEN_FRDONLY] = */ O_RDONLY,
			/* [OPEN_FWRONLY] = */ O_WRONLY,
			/* [OPEN_FRDWR  ] = */ O_RDWR,
			/* [OPEN_F???   ] = */ O_RDWR
		};
		used_oflags = accmode_flags[oflags & OPEN_FACCMODE];
#if SHARED_FLAGS != 0
		used_oflags |= oflags & SHARED_FLAGS;
#endif /* SHARED_FLAGS != 0 */
	}
#endif /* O_RDONLY != OPEN_FRDONLY || O_WRONLY != OPEN_FWRONLY || O_RDWR != OPEN_FRDWR */

	/* Copy bits that we don't share. */
#if NOSHAR_FLAGS & OPEN_FCREAT
	if (oflags & OPEN_FCREAT)
		used_oflags |= O_CREAT;
#endif /* NOSHAR_FLAGS & OPEN_FCREAT */
#if NOSHAR_FLAGS & OPEN_FEXCL
	if (oflags & OPEN_FEXCL)
		used_oflags |= O_EXCL;
#endif /* NOSHAR_FLAGS & OPEN_FEXCL */
#if NOSHAR_FLAGS & OPEN_FTRUNC
	if (oflags & OPEN_FTRUNC)
		used_oflags |= O_TRUNC;
#endif /* NOSHAR_FLAGS & OPEN_FTRUNC */
#if NOSHAR_FLAGS & OPEN_FAPPEND
	if (oflags & OPEN_FAPPEND)
		used_oflags |= O_APPEND;
#endif /* NOSHAR_FLAGS & OPEN_FAPPEND */
#ifdef CONFIG_HAVE_O_NONBLOCK
#if NOSHAR_FLAGS & OPEN_FNONBLOCK
	if (oflags & OPEN_FNONBLOCK)
		used_oflags |= O_NONBLOCK;
#endif /* NOSHAR_FLAGS & OPEN_FNONBLOCK */
#endif /* CONFIG_HAVE_O_NONBLOCK */
#ifdef CONFIG_HAVE_O_SYNC
#if NOSHAR_FLAGS & OPEN_FSYNC
	if (oflags & OPEN_FSYNC)
		used_oflags |= O_SYNC;
#endif /* NOSHAR_FLAGS & OPEN_FSYNC */
#endif /* CONFIG_HAVE_O_SYNC */
#ifdef CONFIG_HAVE_O_DIRECT
#if NOSHAR_FLAGS & OPEN_FDIRECT
	if (oflags & OPEN_FDIRECT)
		used_oflags |= O_DIRECT;
#endif /* NOSHAR_FLAGS & OPEN_FDIRECT */
#endif /* CONFIG_HAVE_O_DIRECT */
#ifdef CONFIG_HAVE_O_NOFOLLOW
#if NOSHAR_FLAGS & OPEN_FNOFOLLOW
	if (oflags & OPEN_FNOFOLLOW)
		used_oflags |= O_NOFOLLOW;
#endif /* NOSHAR_FLAGS & OPEN_FNOFOLLOW */
#endif /* CONFIG_HAVE_O_NOFOLLOW */
#ifdef CONFIG_HAVE_O_NOATIME
#if NOSHAR_FLAGS & OPEN_FNOATIME
	if (oflags & OPEN_FNOATIME)
		used_oflags |= O_NOATIME;
#endif /* NOSHAR_FLAGS & OPEN_FNOATIME */
#endif /* CONFIG_HAVE_O_NOATIME */
#ifdef CONFIG_HAVE_O_CLOEXEC
#if NOSHAR_FLAGS & OPEN_FCLOEXEC
	if (oflags & OPEN_FCLOEXEC)
		used_oflags |= O_CLOEXEC;
#endif /* NOSHAR_FLAGS & OPEN_FCLOEXEC */
#endif /* CONFIG_HAVE_O_CLOEXEC */

#ifdef CONFIG_HAVE_O_OBTAIN_DIR
	/* Allow the opening of directories. */
	used_oflags |= O_OBTAIN_DIR;
#endif /* CONFIG_HAVE_O_OBTAIN_DIR */

#ifdef CONFIG_HAVE_O_BINARY
	/* Prevent the system from tinkering with the data. */
	used_oflags |= O_BINARY;
#endif /* CONFIG_HAVE_O_BINARY */

	/* Do the open. */
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		goto err;

	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_open64
	fd = open64(utf8_filename, used_oflags, mode);
#else /* CONFIG_HAVE_open64 */
	fd  = open(utf8_filename, used_oflags, mode);
#endif /* !CONFIG_HAVE_open64 */
	DBG_ALIGNMENT_ENABLE();
#if defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen)
	if (fd < 0) { /* Re-try in wide-character mode if supported by the host. */
		wchar_t *str = (wchar_t *)DeeString_AsWide(filename);
		if unlikely(!str)
			goto err;
		DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_wopen64
		fd = wopen64(str, used_oflags, mode);
#else /* CONFIG_HAVE_wopen64 */
		fd = wopen(str, used_oflags, mode);
#endif /* !CONFIG_HAVE_wopen64 */
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_wopen64 || CONFIG_HAVE_wopen */
	if (fd < 0) {
		int error = DeeSystem_GetErrno();

#ifdef EEXIST /* Handle file-already-exists. */
		if (error == EEXIST && (oflags & OPEN_FEXCL))
			return ITER_DONE;
#endif /* EEXIST */

#ifdef ENOENT /* Handle file-not-found. */
		if (error == ENOENT && !(oflags & OPEN_FCREAT))
			return ITER_DONE;
#endif /* ENOENT */

#ifdef ENOTDIR
		if (error == ENOTDIR)
			return ITER_DONE;
#endif /* ENOTDIR */

#ifdef EACCES
		if (error == EACCES) {
			DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error,
			                          "Failed to access %r", filename);
		} else
#endif /* EACCES */
#if defined(ENXIO) || defined(EROFS) || defined(EISDIR) || defined(ETXTBSY)
		if (0
#ifdef ENXIO
		    || error == ENXIO
#endif /* ENXIO */
#ifdef EROFS
		    || error == EROFS
#endif /* EROFS */
#ifdef EISDIR
		    || error == EISDIR
#endif /* EISDIR */
#ifdef ETXTBSY
		    || error == ETXTBSY
#endif /* ETXTBSY */
		    ) {
			DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, error,
			                          "Cannot open directory %r for writing",
			                          filename);
		} else
#endif /* ENXIO || EROFS || EISDIR || ETXTBSY */
		{
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to open %r",
			                          filename);
		}
		goto err;
	}
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto err_fd;
	DeeObject_Init(result, &DeeFSFile_Type);
	result->sf_handle    = (DeeSysFD)fd;
	result->sf_ownhandle = (DeeSysFD)fd; /* Inherit stream. */
	result->sf_filename  = filename;
	Dee_Incref(filename);
	return (DREF DeeObject *)result;
err_fd:
#ifdef CONFIG_HAVE_close
	(void)close(fd);
#endif /* CONFIG_HAVE_close */
err:
	return NULL;
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
#if defined(CONFIG_HAVE_fopen) || defined(CONFIG_HAVE_fopen64)
	char *utf8_filename;
	char modbuf[16], *iter = modbuf;
	DREF SystemFile *result;
	FILE *fp;
	if (oflags & OPEN_FCREAT) {
		if (!(oflags & OPEN_FAPPEND)) {
			*iter++ = 'a';
		} else if (!(oflags & OPEN_FTRUNC)) {
			goto err_unsupported_mode;
		} else {
			*iter++ = 'w';
		}
		*iter++ = '+';
		if (oflags & OPEN_FEXCL)
			*iter++ = 'x';
	} else if ((oflags & OPEN_FACCMODE) == OPEN_FRDONLY) {
		*iter++ = 'r';
	} else if ((oflags & OPEN_FACCMODE) != OPEN_FWRONLY) {
		*iter++ = 'r';
		*iter++ = '+';
	} else {
		goto err_unsupported_mode;
	}
	*iter = '\0';

	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		goto err;

	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_fopen64
	fp = fopen64(utf8_filename, modbuf);
#else /* CONFIG_HAVE_fopen64 */
	fp = fopen(utf8_filename, modbuf);
#endif /* !CONFIG_HAVE_fopen64 */
	DBG_ALIGNMENT_ENABLE();
	if (!fp) {
		/* Handle file-not-found / already exists cases. */
		if ((oflags & OPEN_FEXCL) || !(oflags & OPEN_FCREAT))
			return ITER_DONE;
		DeeError_Throwf(&DeeError_FSError,
		                "Failed to open file %r",
		                filename);
		return NULL;
	}
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto err_fp;
	result->sf_handle    = fp;
	result->sf_ownhandle = fp; /* Inherit stream. */
	result->sf_filename  = filename;
	Dee_Incref(filename);
	DeeObject_Init(result, &DeeFSFile_Type);
	return (DREF DeeObject *)result;
err_unsupported_mode:
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "The given open-mode combination is not supported");
	return NULL;
err_fp:
#ifdef CONFIG_HAVE_fclose
	(void)fclose(fp);
#endif /* CONFIG_HAVE_fclose */
#else /* CONFIG_HAVE_fopen || CONFIG_HAVE_fopen64 */
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "Unsupported function `fopen()'");
#endif /* !CONFIG_HAVE_fopen && !CONFIG_HAVE_fopen64 */
err:
	return NULL;
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)filename;
	(void)oflags;
	(void)mode;
	fs_unsupported();
	return NULL;
#endif /* DEESYSTEM_FILE_USE_STUB */
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_OpenString(char const *__restrict filename,
                   int oflags, int mode) {
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)filename;
	(void)oflags;
	(void)mode;
	fs_unsupported();
	return NULL;
#else /* DEESYSTEM_FILE_USE_STUB */
	DREF DeeObject *result, *nameob;
	nameob = DeeString_New(filename);
	if unlikely(!nameob)
		goto err;
	result = DeeFile_Open(nameob, oflags, mode);
	Dee_Decref(nameob);
	return result;
err:
	return NULL;
#endif /* !DEESYSTEM_FILE_USE_STUB */
}



#ifdef DEESYSTEM_FILE_USE_STUB
PRIVATE DeeFileObject std_file = { FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type) };

/* Return the the default stream for a given STD number. */
PUBLIC ATTR_RETNONNULL DeeObject *DCALL
DeeFile_DefaultStd(unsigned int id) {
	ASSERT(id <= DEE_STDDBG);
	(void)id;
	return (DeeObject *)&std_file;
}

#else /* DEESYSTEM_FILE_USE_STUB */

#ifdef DEESYSTEM_FILE_USE_WINDOWS
#undef deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD
#elif defined(DEESYSTEM_FILE_USE_UNIX)
#define deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD
#elif defined(DEESYSTEM_FILE_USE_STDIO)
/* When the std file streams are not defined as macros,
 * we are pretty safe to assume that they are ~real~
 * external symbols that can be linked against. */
#if !defined(stdin) && !defined(stdout) && !defined(stderr)
#define deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD
#endif /* ... */
#if (!defined(CONFIG_HAVE_stdin) ||  \
     !defined(CONFIG_HAVE_stdout) || \
     !defined(CONFIG_HAVE_stderr))
#undef deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD
#endif /* ... */
#endif /* ... */

STATIC_ASSERT(DEE_STDIN == 0);
STATIC_ASSERT(DEE_STDOUT == 1);
STATIC_ASSERT(DEE_STDERR == 2);
#ifdef DEE_STDDBG_IS_UNIQUE
STATIC_ASSERT(DEE_STDDBG == 3);
#endif /* DEE_STDDBG_IS_UNIQUE */

PRIVATE SystemFile sysf_std[] = {
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL, FILE_TYPE_UNKNOWN, 0 },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL, FILE_TYPE_UNKNOWN, 0 },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL, FILE_TYPE_UNKNOWN, 0 }
#elif defined(DEESYSTEM_FILE_USE_UNIX)
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, STDIN_FILENO, -1 },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, STDOUT_FILENO, -1 },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, STDERR_FILENO, -1 }
#elif defined(DEESYSTEM_FILE_USE_STDIO)
#ifdef deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, stdin, NULL },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, stdout, NULL },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, stderr, NULL }
#else /* deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD */
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL }
#endif /* !deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD */
#endif /* ... */

#ifdef DEE_STDDBG_IS_UNIQUE
	,
	{ FILE_OBJECT_HEAD_INIT(&DebugFile_Type), (DeeObject *)(void *)-1, DeeSysFD_INVALID, DeeSysFD_INVALID }
#endif /* DEE_STDDBG_IS_UNIQUE */
};

#if !defined(deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD) && defined(DEESYSTEM_FILE_USE_STDIO)
#define HAVE_sysf_std_closed
PRIVATE uint8_t sysf_std_closed = 0;
#endif /* !deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD && DEESYSTEM_FILE_USE_STDIO */


/* Return the the default stream for a given STD number. */
PUBLIC WUNUSED ATTR_RETNONNULL DeeObject *DCALL
DeeFile_DefaultStd(unsigned int id) {
#ifdef deemon_file_CAN_STATIC_INITIALIZE_SYSF_STD
	ASSERT(id < COMPILER_LENOF(sysf_std));
	return (DeeObject *)&sysf_std[id];
#elif defined(DEESYSTEM_FILE_USE_STDIO)
	SystemFile *result;
	ASSERT(id <= DEE_STDDBG);
	result = &sysf_std[id];
	if unlikely(!result->sf_handle) {
		FILE *new_file;
#ifdef CONFIG_HAVE___iob_func
		new_file = (__iob_func() + id);
#elif defined(CONFIG_HAVE___acrt_iob_func)
		new_file = __acrt_iob_func(id);
#else /* ... */
		switch (id) {

		case DEE_STDIN:
			new_file = stdin;
			break;

		case DEE_STDOUT:
			new_file = stdout;
			break;

		default:
			new_file = stderr;
			break;
		}
#endif /* !... */

		/* Make sure not to re-open an std-file that was already closed. */
#ifdef HAVE_sysf_std_closed
		if (!(sysf_std_closed & (1 << id)))
#endif /* HAVE_sysf_std_closed */
		{
#ifndef CONFIG_NO_THREADS
			/* Make sure not to re-write another file in the event that
			 * the `std*' keywords of the hosting c-library evaluate to
			 * different things when called more than once (this may
			 * sound weird, but some really strange platform might do
			 * this to implement freopen()?) */
			/* NOTE: I know for a fact that the following C libraries don't do this: */
#if !defined(_MSC_VER) && !defined(__linux__) && !defined(__KOS__)
			if (!result->sf_handle)
#endif /* !... */
#endif /* !CONFIG_NO_THREADS */
			{
				result->sf_handle    = new_file;
#if 1 /* Without this, close() on standard descriptors would \
       * leave the file open (which might actually be intended?) */
				result->sf_ownhandle = new_file;
#endif
			}
		}
	}
	return (DeeObject *)result;
#elif defined(DEESYSTEM_FILE_USE_WINDOWS)
	SystemFile *result;
	ASSERT(id < COMPILER_LENOF(sysf_std));
	result = &sysf_std[id];
	if unlikely(result->sf_handle == NULL) {
		DWORD std_id;
		HANDLE std_handle;
		switch (id) {
		case DEE_STDIN:
			std_id = STD_INPUT_HANDLE;
			break;
		case DEE_STDOUT:
			std_id = STD_OUTPUT_HANDLE;
			break;
		default:
			std_id = STD_ERROR_HANDLE;
			break;
		}
		DBG_ALIGNMENT_DISABLE();
		std_handle = GetStdHandle(std_id);
		DBG_ALIGNMENT_ENABLE();
		result->sf_handle    = std_handle;
		result->sf_ownhandle = std_handle;
	}
	return (DeeObject *)result;
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
}
#endif /* !DEESYSTEM_FILE_USE_STUB */


#ifdef HAVE_sysf_std_closed
#define REMEMBER_FILE_CLOSED(self)                                     \
	do {                                                               \
		if ((self) >= sysf_std && (self) < COMPILER_ENDOF(sysf_std)) { \
			/* Make sure not to re-open this file later. */            \
			sysf_std_closed |= 1 << ((self) - sysf_std);               \
		}                                                              \
	}	__WHILE0
#else /* HAVE_sysf_std_closed */
#define REMEMBER_FILE_CLOSED(self) (void)0
#endif /* !HAVE_sysf_std_closed */




/************************************************************************/
/* Helpers used by the WINDOWS implementation                           */
/************************************************************************/
#ifdef DEESYSTEM_FILE_USE_WINDOWS
typedef union {
	struct {
		/* Use our own structure so it gets aligned by 64 bits,
		 * and `Offset' can be assigned directly, without some
		 * sh1tty wrapper code. */
		uintptr_t Internal;
		uintptr_t InternalHigh;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		uint64_t  Offset;
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
		uint32_t  Offset;
		uint32_t  OffsetHigh;
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	}   me;
	OVERLAPPED    ms;
} DEE_FIXED_OVERLAPPED;

PRIVATE NONNULL((1)) void DCALL
nt_sysfile_on_filetype_set(SystemFile *__restrict self, DWORD type) {
	(void)self;
	(void)type;
	if (type == FILE_TYPE_CHAR) {
		DWORD dwConsoleMode;
		DBG_ALIGNMENT_DISABLE();
		if (GetConsoleMode(self->sf_handle, &dwConsoleMode)) {
#ifndef ENABLE_PROCESSED_OUTPUT
#define ENABLE_PROCESSED_OUTPUT 0x1
#endif /* !ENABLE_PROCESSED_OUTPUT */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x4
#endif /* !ENABLE_VIRTUAL_TERMINAL_PROCESSING */
			/* TODO: This is a good beginning, but it'd be better to port KOS's libansitty
			 *       and use it (mainly so we're compatible with older versions of windows). */
			dwConsoleMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			if (!SetConsoleMode(self->sf_handle, dwConsoleMode)) {
				Dee_DPRINTF("[INIT] Failed to get an ansi-terminal for %p (SetConsoleMode returned %lu)\n",
				            self->sf_handle, GetLastError());
			}
		}
		DBG_ALIGNMENT_ENABLE();
	}
}

/* Determine if the referenced file is a TTY file.
 * @return: * :                One of `FILE_TYPE_*'.
 * @return: FILE_TYPE_UNKNOWN: An error occurred and was thrown. */
PRIVATE NONNULL((1)) DWORD DCALL
nt_sysfile_gettype(SystemFile *__restrict self) {
	if (self->sf_filetype == (uint32_t)FILE_TYPE_UNKNOWN) {
		DWORD type;
		DBG_ALIGNMENT_DISABLE();
		type = GetFileType(self->sf_handle);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(type == FILE_TYPE_UNKNOWN) {
			err_file_io(self);
			return FILE_TYPE_UNKNOWN;
		}
		atomic_cmpxch(&self->sf_filetype,
		              (uint32_t)FILE_TYPE_UNKNOWN,
		              (uint32_t)type);
		nt_sysfile_on_filetype_set(self, type);
	}
	return (DWORD)self->sf_filetype;
}

PRIVATE NONNULL((1)) DWORD DCALL
nt_sysfile_trygettype(SystemFile *__restrict self) {
	if (self->sf_filetype == (uint32_t)FILE_TYPE_UNKNOWN) {
		DWORD type;
		DBG_ALIGNMENT_DISABLE();
		type = GetFileType(self->sf_handle);
		DBG_ALIGNMENT_ENABLE();
		if likely(type != FILE_TYPE_UNKNOWN) {
			atomic_cmpxch(&self->sf_filetype,
			              (uint32_t)FILE_TYPE_UNKNOWN,
			              (uint32_t)type);
			nt_sysfile_on_filetype_set(self, type);
		}
	}
	return (DWORD)self->sf_filetype;
}

LOCAL BOOL DCALL
nt_write_all_console_w(HANDLE hConsoleOutput,
                       LPWSTR lpBuffer,
                       DWORD nNumberOfCharsToWrite) {
	DWORD temp;
	for (;;) {
#if __SIZEOF_SIZE_T__ > 4
		DWORD max_write = nNumberOfCharsToWrite > UINT32_MAX ? UINT32_MAX : (DWORD)nNumberOfCharsToWrite;
		DBG_ALIGNMENT_DISABLE();
		if (!WriteConsoleW(hConsoleOutput, lpBuffer, max_write, &temp, NULL))
			goto err;
#else /* __SIZEOF_SIZE_T__ > 4 */
		DBG_ALIGNMENT_DISABLE();
		if (!WriteConsoleW(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, &temp, NULL))
			goto err;
#endif /* __SIZEOF_SIZE_T__ <= 4 */
		if (!temp)
			goto err;
		DBG_ALIGNMENT_ENABLE();
		nNumberOfCharsToWrite -= temp;
		if (!nNumberOfCharsToWrite)
			break;
		lpBuffer += temp;
	}
	return TRUE;
err:
	DBG_ALIGNMENT_ENABLE();
	return FALSE;
}

LOCAL BOOL DCALL
nt_write_all_file(HANDLE hFileHandle,
                  unsigned char const *lpBuffer,
                  size_t nNumberOfBytesToWrite) {
	DWORD temp;
	for (;;) {
#if __SIZEOF_SIZE_T__ > 4
		DWORD max_write = nNumberOfBytesToWrite > UINT32_MAX ? UINT32_MAX : (DWORD)nNumberOfBytesToWrite;
		DBG_ALIGNMENT_DISABLE();
		if (!WriteFile(hFileHandle, lpBuffer, max_write, &temp, NULL))
			goto err;
#else /* __SIZEOF_SIZE_T__ > 4 */
		DBG_ALIGNMENT_DISABLE();
		if (!WriteFile(hFileHandle, lpBuffer, nNumberOfBytesToWrite, &temp, NULL))
			goto err;
#endif /* __SIZEOF_SIZE_T__ <= 4 */
		if (!temp)
			goto err;
		DBG_ALIGNMENT_ENABLE();
		nNumberOfBytesToWrite -= temp;
		if (!nNumberOfBytesToWrite)
			break;
		lpBuffer += temp;
	}
	return TRUE;
err:
	DBG_ALIGNMENT_ENABLE();
	return FALSE;
}


LOCAL NONNULL((1, 2)) int DCALL
nt_os_write_utf8_to_console(SystemFile *__restrict self,
                            unsigned char const *__restrict buffer,
                            size_t bufsize) {
	WCHAR stackBuffer[512];
	DWORD num_widechars;
again:
	DBG_ALIGNMENT_DISABLE();
	num_widechars = (DWORD)MultiByteToWideChar(CP_UTF8,
	                                           0,
	                                           (LPCCH)buffer,
	                                           (int)(DWORD)bufsize,
	                                           stackBuffer,
	                                           (int)(DWORD)COMPILER_LENOF(stackBuffer));
	DBG_ALIGNMENT_ENABLE();
	if (!num_widechars) {
		DBG_ALIGNMENT_DISABLE();
		if (!bufsize)
			goto done;
		if (DeeNTSystem_IsBufferTooSmall(GetLastError())) {
			LPWSTR tempbuf;
			num_widechars = (DWORD)MultiByteToWideChar(CP_UTF8,
			                                           0,
			                                           (LPCCH)buffer,
			                                           (int)(DWORD)bufsize,
			                                           NULL,
			                                           0);
			DBG_ALIGNMENT_ENABLE();
			if unlikely(!num_widechars)
				goto fallback;
			tempbuf = (LPWSTR)Dee_Mallocc(num_widechars, sizeof(WCHAR));
			if unlikely(!tempbuf)
				goto err;
			DBG_ALIGNMENT_DISABLE();
			num_widechars = MultiByteToWideChar(CP_UTF8,
			                                    0,
			                                    (LPCCH)buffer,
			                                    (int)(DWORD)bufsize,
			                                    tempbuf,
			                                    num_widechars);
			DBG_ALIGNMENT_ENABLE();
			if unlikely(!num_widechars) {
				Dee_Free(tempbuf);
				goto fallback;
			}
			if unlikely(!nt_write_all_console_w(self->sf_handle, tempbuf, num_widechars)) {
				Dee_Free(tempbuf);
				goto fallback;
			}
			Dee_Free(tempbuf);
			return 0;
		}
		DBG_ALIGNMENT_ENABLE();
		goto fallback;
	}
	if (!nt_write_all_console_w(self->sf_handle, stackBuffer, num_widechars))
		goto fallback;
done:
	return 0;
fallback:
	DBG_ALIGNMENT_DISABLE();
	if (DeeNTSystem_IsIntr(GetLastError())) {
		DBG_ALIGNMENT_ENABLE();
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	DBG_ALIGNMENT_ENABLE();
	/* Write as ASCII data. */
	if (!nt_write_all_file(self->sf_handle, buffer, bufsize)) {
		DBG_ALIGNMENT_DISABLE();
		if (DeeNTSystem_IsIntr(GetLastError())) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DBG_ALIGNMENT_ENABLE();
		goto err_io;
	}
	return 0;
err_io:
	err_file_io(self);
err:
	return -1;
}


/* @return: * : The number UTF-8! bytes processed. */
PRIVATE NONNULL((1, 2)) size_t DCALL
nt_write_utf8_to_console(SystemFile *__restrict self,
                         unsigned char const *__restrict buffer,
                         size_t bufsize) {
	unsigned char const *iter;
	unsigned char const *end;
	size_t result;
	end = (iter = buffer) + bufsize;
	while (iter < end) {
		uint8_t len, ch = *iter;
		if (ch <= 0xc0) {
			++iter;
			continue;
		}
		len = utf8_sequence_len[ch];
		ASSERT(len != 0);
		if (len > (size_t)(end - iter))
			break;
		iter += len;
	}
	result = (size_t)(iter - buffer);
	if (nt_os_write_utf8_to_console(self, buffer, result))
		result = (size_t)-1;
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
nt_append_pending_utf8(SystemFile *__restrict self,
                       void const *__restrict buffer,
                       size_t bufsize) {
	size_t pending_count;
	ASSERTF(bufsize <= COMPILER_LENOF(self->sf_pending),
	        "A non-encodable unicode sequence that is longer than 7 bytes? WTF?");
again:
	pending_count = atomic_read(&self->sf_pendingc);
	if unlikely(pending_count) {
		size_t temp;
		unsigned char with_pending[COMPILER_LENOF(self->sf_pending) * 2], *p;
		ASSERT(pending_count <= COMPILER_LENOF(self->sf_pending));
		p = (unsigned char *)mempcpyc(with_pending, self->sf_pending,
		                              pending_count, sizeof(unsigned char));
		memcpyc(p, buffer, bufsize, sizeof(unsigned char));
		if unlikely(!atomic_cmpxch_weak_or_write(&self->sf_pendingc, pending_count, 0))
			goto again;
		temp = nt_write_utf8_to_console(self, with_pending, pending_count + bufsize);
		if unlikely(temp == (size_t)-1) {
			atomic_cmpxch(&self->sf_pendingc, 0, pending_count);
			goto err;
		}
		pending_count += bufsize;
		ASSERT(temp <= pending_count);
		pending_count -= temp;
		ASSERT(pending_count <= COMPILER_LENOF(self->sf_pending));
		if unlikely(pending_count)
			return nt_append_pending_utf8(self, with_pending + temp, pending_count);
	} else {
		memcpyc(self->sf_pending, buffer, bufsize, sizeof(unsigned char));
		if unlikely(!atomic_cmpxch_weak_or_write(&self->sf_pendingc, 0, bufsize))
			goto again;
	}
	return 0;
err:
	return -1;
}


LOCAL NONNULL((1, 2)) int DCALL
nt_write_to_console(SystemFile *__restrict self,
                    void const *__restrict buffer,
                    size_t bufsize) {
	uint8_t pending_count;
	size_t num_written;
again:
	if (!bufsize)
		return 0;
	pending_count = atomic_read(&self->sf_pendingc);
	if (pending_count) {
		unsigned char with_pending[64], *p;
		size_t total_length;
		ASSERT(pending_count <= COMPILER_LENOF(self->sf_pending));
		p = (unsigned char *)mempcpyc(with_pending, self->sf_pending, pending_count, sizeof(char));
		total_length = pending_count + bufsize;
		if (total_length > COMPILER_LENOF(with_pending))
			total_length = COMPILER_LENOF(with_pending);
		memcpyc(p, buffer, total_length - pending_count, sizeof(char));
		if unlikely(!atomic_cmpxch_weak_or_write(&self->sf_pendingc, pending_count, 0))
			goto again;
		num_written = nt_write_utf8_to_console(self,
		                                    with_pending,
		                                    total_length);
		if unlikely(num_written == (size_t)-1) {
			atomic_cmpxch(&self->sf_pendingc, 0, pending_count);
			goto err;
		}
		ASSERT(num_written <= total_length);
		if (num_written < total_length) {
			if (nt_append_pending_utf8(self,
			                        with_pending + num_written,
			                        total_length - num_written))
				goto err;
		}
		total_length -= pending_count;
		ASSERT(total_length <= bufsize);
		buffer = (void *)((uint8_t *)buffer + total_length);
		bufsize -= total_length;
		goto again;
	}
	num_written = nt_write_utf8_to_console(self,
	                                    (unsigned char *)buffer,
	                                    bufsize);
	if unlikely(num_written == (size_t)-1)
		goto err;
	ASSERT(num_written <= bufsize);
	if (num_written < bufsize) {
		if (nt_append_pending_utf8(self,
		                        (uint8_t *)buffer + num_written,
		                        bufsize - num_written))
			goto err;
	}
	return 0;
err:
	return -1;
}
#endif /* DEESYSTEM_FILE_USE_WINDOWS */



/************************************************************************/
/* Helpers used by the STDIO implementation                             */
/************************************************************************/
#ifdef DEESYSTEM_FILE_USE_STDIO
#ifndef CONFIG_HAVE_fread
#ifdef CONFIG_HAVE_fgetc
#define CONFIG_HAVE_fread
#undef fread
#define fread dee_fread
PRIVATE size_t DCALL
dee_fread(void *buf, size_t elemsize, size_t elemcount, FILE *stream) {
	size_t i, total;
	total = elemsize * elemcount;
	for (i = 0; i < total; ++i) {
		int ch = fgetc(stream);
		if (ch == EOF)
			break;
		((__BYTE_TYPE__ *)buf)[i] = (__BYTE_TYPE__)(unsigned int)ch;
	}
	return i / elemsize;
}
#endif /* CONFIG_HAVE_fgetc */
#endif /* !CONFIG_HAVE_fread */

#ifndef CONFIG_HAVE_fwrite
#ifdef CONFIG_HAVE_fputc
#define CONFIG_HAVE_fwrite
#undef fwrite
#define fwrite dee_fwrite
PRIVATE size_t DCALL
dee_fwrite(void const *buf, size_t elemsize, size_t elemcount, FILE *stream) {
	size_t i, total;
	total = elemsize * elemcount;
	for (i = 0; i < total; ++i) {
		__BYTE_TYPE__ b;
		b = ((__BYTE_TYPE__ const *)buf)[i];
		if (fputc(b, stream) == EOF)
			break;
	}
	return i / elemsize;
}
#endif /* CONFIG_HAVE_fputc */
#endif /* !CONFIG_HAVE_fwrite */
#endif /* DEESYSTEM_FILE_USE_STDIO */



INTERN WUNUSED NONNULL((1, 2)) size_t DCALL
sysfile_read(SystemFile *__restrict self,
             void *__restrict buffer,
             size_t bufsize, dioflag_t flags) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	DWORD result;
#if __SIZEOF_SIZE_T__ > 4
	if unlikely(bufsize > UINT32_MAX)
		bufsize = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
again:
	if (flags & Dee_FILEIO_FNONBLOCKING) {
		DWORD file_type;
#if __SIZEOF_SIZE_T__ > 4
		if unlikely(bufsize > UINT32_MAX)
			bufsize = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
		file_type = nt_sysfile_gettype(self);
		if (file_type == FILE_TYPE_UNKNOWN)
			goto err;
		if (file_type == FILE_TYPE_PIPE) {
			BYTE temp_buffer[1];
			/* `WaitForSingleObject()' doesn't work on pipes (for some reason...) */
			result = 0;
			DBG_ALIGNMENT_DISABLE();
			if (PeekNamedPipe(self->sf_handle,
			                  temp_buffer,
			                  sizeof(temp_buffer),
			                  &result,
			                  NULL,
			                  NULL) &&
			    result == 0) {
				DBG_ALIGNMENT_ENABLE();
				return 0;
			}
			DBG_ALIGNMENT_ENABLE();
		} else {
			DBG_ALIGNMENT_DISABLE();
			result = WaitForSingleObject(self->sf_handle, 0);
			DBG_ALIGNMENT_ENABLE();
			if (result == WAIT_TIMEOUT)
				return 0;
		}
	}
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!ReadFile(self->sf_handle, buffer, (DWORD)bufsize, &result, NULL)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(error)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		if (error != ERROR_BROKEN_PIPE)
			goto err_io;
		/* Handle pipe-disconnect as EOF (like it should be...). */
		result = 0;
	}
	DBG_ALIGNMENT_ENABLE();
	return result;
err_io:
	err_file_io(self);
err:
	return (size_t)-1;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
#ifdef CONFIG_HAVE_read
	/* TODO: Use `select()' to check if reading will block for `Dee_FILEIO_FNONBLOCKING' */
	/* TODO: Use KOS's readf() function */
	size_t result;
	(void)flags;
	DBG_ALIGNMENT_DISABLE();
	result = (size_t)read((int)self->sf_handle, buffer, bufsize);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result == (size_t)-1)
		err_file_io(self);
	return result;
#else /* CONFIG_HAVE_read */
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)flags;
	return (size_t)err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                          FILE_OPERATOR_READ);
#endif /* !CONFIG_HAVE_read */
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
#ifdef CONFIG_HAVE_fread
	size_t result;
	if unlikely(!self->sf_handle)
		return err_file_closed(self);
	(void)flags;
	DBG_ALIGNMENT_DISABLE();
	result = (size_t)fread(buffer, 1, bufsize, (FILE *)self->sf_handle);
#ifdef CONFIG_HAVE_ferror
	if (!result && ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
#endif /* CONFIG_HAVE_ferror */
	DBG_ALIGNMENT_ENABLE();
	return result;
#else /* CONFIG_HAVE_fread */
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)flags;
	err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                           FILE_OPERATOR_READ);
	return (size_t)-1;
#endif /* !CONFIG_HAVE_fread */
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)flags;
	return (size_t)fs_unsupported();
#endif /* DEESYSTEM_FILE_USE_STUB */
}


PRIVATE size_t DCALL
sysfile_write(SystemFile *__restrict self,
              void const *__restrict buffer,
              size_t bufsize, dioflag_t UNUSED(flags)) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	DWORD bytes_written, file_type;
#if __SIZEOF_SIZE_T__ > 4
	if unlikely(bufsize > UINT32_MAX)
		bufsize = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
	file_type = nt_sysfile_trygettype(self);
	if unlikely(file_type == FILE_TYPE_CHAR) {
		/* Because windows's `WriteFile()' function just doesn't work
		 * for UTF-8 input (even if you do `SetConsoleOutputCP(CP_UTF8)'),
		 * we must manually do the conversion to windows's proprietary
		 * wide-character format, before writing everything using
		 * `WriteConsoleW()'.
		 * -> It seems like this is the only way to get _true_ UTF-8
		 *    output when writing data to the windows console...
		 * Also note that if you were to do `SetConsoleOutputCP(CP_UTF8)',
		 * WriteFile() would still fail to print UTF-8 data, and `WriteConsoleW()'
		 * called with converted UTF-8 data will fail with an error.
		 * This, as well as the fact that I've seen the default console CP
		 * having been changed to `CP_UTF8' in some environments is why in
		 * our main(), we call `SetConsoleOutputCP(GetOEMCP())' to make sure
		 * that the default OEM code page is set (which can be used for the
		 * purposes of converting UTF-8 to Wide-chars, before printing with
		 * full unicode support enabled)
		 * XXX: What about console input? Shouldn't that have the same problem? */
		if unlikely(nt_write_to_console(self, buffer, bufsize))
			goto err;
		return bufsize;
	}
again:
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!WriteFile(self->sf_handle, buffer,
	                       (DWORD)bufsize,
	                       &bytes_written, NULL)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(error)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		return err_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return bytes_written;
err:
	return (size_t)-1;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
#ifdef CONFIG_HAVE_write
	/* TODO: Use `select()' to check if writing will block for `Dee_FILEIO_FNONBLOCKING' */
	/* TODO: Use KOS's writef() function */
	size_t result;
	(void)flags;
	DBG_ALIGNMENT_DISABLE();
	result = (size_t)write((int)self->sf_handle, buffer, bufsize);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result == (size_t)-1)
		err_file_io(self);
	return result;
#else /* CONFIG_HAVE_write */
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)flags;
	return (size_t)err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                          FILE_OPERATOR_WRITE);
#endif /* !CONFIG_HAVE_write */
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
#ifdef CONFIG_HAVE_fwrite
	size_t result;
	if unlikely(!self->sf_handle)
		return err_file_closed(self);
	(void)flags;
	DBG_ALIGNMENT_DISABLE();
	result = (size_t)fwrite(buffer, 1, bufsize, (FILE *)self->sf_handle);
	if (!result && ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return result;
#else /* CONFIG_HAVE_fwrite */
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)flags;
	err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                           FILE_OPERATOR_WRITE);
	return (size_t)-1;
#endif /* !CONFIG_HAVE_fwrite */
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)flags;
	return (size_t)fs_unsupported();
#endif /* DEESYSTEM_FILE_USE_STUB */
}


#define deemon_file_HAVE_sysfile_pread
#define deemon_file_HAVE_sysfile_pwrite
#ifdef DEESYSTEM_FILE_USE_STDIO
/* Under stdio, pread/pwrite must be emulated using seek+read/write
 * We signify this to the high-level operator wrappers by simply not
 * defining the pread/pwrite callbacks. */
#undef deemon_file_HAVE_sysfile_pread
#undef deemon_file_HAVE_sysfile_pwrite
#endif /* DEESYSTEM_FILE_USE_STDIO */



#ifdef deemon_file_HAVE_sysfile_pread
INTERN size_t DCALL
sysfile_pread(SystemFile *__restrict self,
              void *__restrict buffer,
              size_t bufsize, dpos_t pos,
              dioflag_t flags) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	DWORD bytes_read;
	DEE_FIXED_OVERLAPPED overlapped;
	(void)flags;
#if __SIZEOF_SIZE_T__ > 4
	if unlikely(bufsize > UINT32_MAX)
		bufsize = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
again:
	bzero(&overlapped, sizeof(overlapped));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	overlapped.me.Offset = pos;
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
	overlapped.me.Offset = (uint32_t)pos;
	overlapped.me.OffsetHigh = (uint32_t)(pos >> 32);
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__*/
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!ReadFile(self->sf_handle, buffer,
	                      (DWORD)bufsize,
	                      &bytes_read,
	                      (LPOVERLAPPED)&overlapped)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(error)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		return err_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return bytes_read;
err:
	return (size_t)-1;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
#if defined(CONFIG_HAVE_pread64) || defined(CONFIG_HAVE_pread)
	size_t result;
	(void)flags;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_pread64
	result = (size_t)pread64((int)self->sf_handle, buffer, bufsize, pos);
#else /* CONFIG_HAVE_pread64 */
	result = (size_t)pread((int)self->sf_handle, buffer, bufsize, pos);
#endif /* !CONFIG_HAVE_pread64 */
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result == (size_t)-1)
		err_file_io(self);
	return result;
#else /* CONFIG_HAVE_pread[64] */
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)pos;
	(void)flags;
	return (size_t)err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                          FILE_OPERATOR_PREAD);
#endif /* !CONFIG_HAVE_pread[64] */
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)pos;
	(void)flags;
	return (size_t)fs_unsupported();
#endif /* DEESYSTEM_FILE_USE_STUB */
}
#endif /* deemon_file_HAVE_sysfile_pread */

#ifdef deemon_file_HAVE_sysfile_pwrite
PRIVATE size_t DCALL
sysfile_pwrite(SystemFile *__restrict self,
               void const *__restrict buffer,
               size_t bufsize, dpos_t pos,
               dioflag_t flags) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	DWORD bytes_written;
	DEE_FIXED_OVERLAPPED overlapped;
	(void)flags;
#if __SIZEOF_SIZE_T__ > 4
	if unlikely(bufsize > UINT32_MAX)
		bufsize = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
again:
	bzero(&overlapped, sizeof(overlapped));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	overlapped.me.Offset = pos;
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
	overlapped.me.Offset = (uint32_t)pos;
	overlapped.me.OffsetHigh = (uint32_t)(pos >> 32);
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!WriteFile(self->sf_handle, buffer,
	                       (DWORD)bufsize,
	                       &bytes_written,
	                       (LPOVERLAPPED)&overlapped)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(error)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		return err_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return bytes_written;
err:
	return (size_t)-1;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
#if defined(CONFIG_HAVE_pwrite64) || defined(CONFIG_HAVE_pwrite)
	size_t result;
	(void)flags;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_pwrite64
	result = (size_t)pwrite64((int)self->sf_handle, buffer, bufsize, pos);
#else /* CONFIG_HAVE_pwrite64 */
	result = (size_t)pwrite((int)self->sf_handle, buffer, bufsize, pos);
#endif /* !CONFIG_HAVE_pwrite64 */
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result == (size_t)-1)
		err_file_io(self);
	return result;
#else /* CONFIG_HAVE_pwrite[64] */
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)pos;
	(void)flags;
	return (size_t)err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                          FILE_OPERATOR_PWRITE);
#endif /* !CONFIG_HAVE_pwrite[64] */
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)buffer;
	(void)bufsize;
	(void)pos;
	(void)flags;
	return (size_t)fs_unsupported();
#endif /* DEESYSTEM_FILE_USE_STUB */
}
#endif /* deemon_file_HAVE_sysfile_pwrite */


PRIVATE dpos_t DCALL
sysfile_seek(SystemFile *__restrict self, doff_t off, int whence) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	DWORD result;
	LONG high;
again:
	high = (DWORD)((uint64_t)off >> 32);
	DBG_ALIGNMENT_DISABLE();
	result = SetFilePointer(self->sf_handle, (LONG)(off & 0xffffffff), &high, whence);
	if unlikely(result == INVALID_SET_FILE_POINTER) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (error != NO_ERROR) {
			if (DeeNTSystem_IsIntr(error)) {
				if (DeeThread_CheckInterrupt())
					goto err;
				goto again;
			}
			return err_file_io(self);
		}
	}
	DBG_ALIGNMENT_ENABLE();
	return (dpos_t)result | ((dpos_t)high << 32);
err:
	return (dpos_t)-1;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
#if defined(CONFIG_HAVE_lseek) || defined(CONFIG_HAVE_lseek64)
	dpos_t result;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_lseek64
	result = (dpos_t)lseek64((int)self->sf_handle, (int64_t)off, whence);
#else /* CONFIG_HAVE_lseek64 */
	result = (dpos_t)lseek((int)self->sf_handle, (off_t)off, whence);
#endif /* !CONFIG_HAVE_lseek64 */
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result == (dpos_t)-1)
		err_file_io(self);
	return result;
#else /* CONFIG_HAVE_lseek || CONFIG_HAVE_lseek64 */
	(void)self;
	(void)off;
	(void)whence;
	return (dpos_t)err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                          FILE_OPERATOR_SEEK);
#endif /* !CONFIG_HAVE_lseek && !CONFIG_HAVE_lseek64 */
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
	dpos_t result;
	if unlikely(!self->sf_handle)
		return err_file_closed(self);
#if defined(CONFIG_HAVE_fseeko64) && defined(CONFIG_HAVE_ftello64)
	DBG_ALIGNMENT_DISABLE();
	if unlikely(fseeko64((FILE *)self->sf_handle, (off64_t)off, whence) != 0) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = (dpos_t)ftello64((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result == (dpos_t)-1)
		error_file_io(self);
#elif defined(CONFIG_HAVE_fseeko) && defined(CONFIG_HAVE_ftello)
	DBG_ALIGNMENT_DISABLE();
	if unlikely(fseeko((FILE *)self->sf_handle, (off_t)off, whence) != 0) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = (dpos_t)ftello((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result == (dpos_t)-1)
		error_file_io(self);
#elif defined(CONFIG_HAVE_fseek) && defined(CONFIG_HAVE_ftell)
	DBG_ALIGNMENT_DISABLE();
	if unlikely(fseek((FILE *)self->sf_handle, (long)off, whence) != 0) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = (dpos_t)ftello((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if unlikely((unsigned long)result == (unsigned long)-1L)
		return error_file_io(self);
#else /* ... */
	(void)self;
	(void)off;
	(void)whence;
	err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                           FILE_OPERATOR_SEEK);
	result = (dpos_t)-1;
#endif /* !... */
	return result;
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)off;
	(void)whence;
	return (dpos_t)(doff_t)fs_unsupported();
#endif /* DEESYSTEM_FILE_USE_STUB */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_sync(SystemFile *__restrict self) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	/* Attempting to flush a console handle bickers about the handle being invalid... */
	if (self->sf_filetype == (uint32_t)FILE_TYPE_CHAR)
		goto done;
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!FlushFileBuffers(self->sf_handle)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		/* If the error is ERROR_INVALID_HANDLE but our handle isn't
		 * set to be invalid, this can be because our handle is that
		 * of a console:
		 * http://winapi.freetechsecrets.com/win32/WIN32FlushFileBuffers.htm */
		if (error == ERROR_INVALID_HANDLE &&
		    self->sf_handle != INVALID_HANDLE_VALUE) {
			DWORD temp = nt_sysfile_gettype(self);
			if unlikely(temp == FILE_TYPE_UNKNOWN)
				goto err;
			if (temp == FILE_TYPE_CHAR)
				goto done;
			DBG_ALIGNMENT_DISABLE();
			SetLastError(error);
			DBG_ALIGNMENT_ENABLE();
		}
		return err_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
done:
	return 0;
err:
	return -1;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
#if defined(CONFIG_HAVE_fdatasync)
	DBG_ALIGNMENT_DISABLE();
	if unlikely(fdatasync((int)self->sf_handle) < 0) {
		DBG_ALIGNMENT_ENABLE();
		return err_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
#elif defined(CONFIG_HAVE_fsync)
	DBG_ALIGNMENT_DISABLE();
	if unlikely(fsync((int)self->sf_handle) < 0) {
		DBG_ALIGNMENT_ENABLE();
		return err_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
#else /* ... */
	(void)self;
#endif /* !... */
	return 0;
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
	if unlikely(!self->sf_handle)
		return err_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
#if defined(CONFIG_HAVE_fflush) && defined(CONFIG_HAVE_ferror)
	if (fflush((FILE *)self->sf_handle) &&
	    ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
#elif defined(CONFIG_HAVE_fflush)
	(void)fflush((FILE *)self->sf_handle);
#endif /* ... */
	DBG_ALIGNMENT_ENABLE();
	return 0;
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	return fs_unsupported();
#endif /* DEESYSTEM_FILE_USE_STUB */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_trunc(SystemFile *__restrict self, dpos_t size) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	dpos_t old_pos = sysfile_seek(self, 0, SEEK_CUR);
	if unlikely(old_pos == (dpos_t)-1)
		goto err;
	if ((dpos_t)old_pos != size) {
		if unlikely(sysfile_seek(self, (doff_t)size, SEEK_SET) == (dpos_t)-1)
			goto err;
	}
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!SetEndOfFile(self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return err_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	if ((dpos_t)old_pos != size) {
		if unlikely(sysfile_seek(self, old_pos, SEEK_SET) == (dpos_t)-1)
			goto err;
	}
	return 0;
err:
	return -1;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
	int result;
#if defined(CONFIG_HAVE_ftruncate) || defined(CONFIG_HAVE_ftruncate64)
	/* Use ftruncate() */
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_ftruncate64
	result = ftruncate64((int)self->sf_handle, (int64_t)size);
#else /* CONFIG_HAVE_ftruncate64 */
	result = ftruncate((int)self->sf_handle, (off_t)size);
#endif /* !CONFIG_HAVE_ftruncate64 */
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result != 0)
		err_file_io(self);
#elif defined(CONFIG_HAVE_truncate) || defined(CONFIG_HAVE_truncate64)
	/* Use truncate() */
	DREF DeeObject *filename;
	char *utf8_filename;
	filename = DeeSystemFile_Filename((DeeObject *)self);
	if unlikely(!filename)
		goto err;
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename) {
		Dee_Decref(filename);
err:
		return -1;
	}
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_truncate64
	result = truncate64(utf8_filename, (off64_t)size);
#else /* CONFIG_HAVE_truncate64 */
	result = truncate(utf8_filename, (off64_t)size);
#endif /* !CONFIG_HAVE_truncate64 */
	DBG_ALIGNMENT_ENABLE();
	Dee_Decref(filename);
	if unlikely(result != 0)
		err_file_io(self);
#else /* ... */
	result = err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                    FILE_OPERATOR_TRUNC);
#endif /* !... */
	return result;
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
	if unlikely(!self->sf_handle)
		return err_file_closed(self);
#ifdef CONFIG_HAVE_fftruncate64
	DBG_ALIGNMENT_DISABLE();
	if (fftruncate64((FILE *)self->sf_handle, size)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
#elif defined(CONFIG_HAVE_fftruncate)
	DBG_ALIGNMENT_DISABLE();
	if (fftruncate64((FILE *)self->sf_handle, (uint32_t)size)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
#elif (defined(CONFIG_HAVE_fileno) && \
       (defined(CONFIG_HAVE_ftruncate) || defined(CONFIG_HAVE_ftruncate64)))
	int fd;
	DBG_ALIGNMENT_DISABLE();
	fd = fileno((FILE *)self->sf_handle);
#ifdef CONFIG_HAVE_ftruncate64
	if (ftruncate64(fd, size))
#else /* CONFIG_HAVE_ftruncate64 */
	if (ftruncate(fd, size))
#endif /* !CONFIG_HAVE_ftruncate64 */
	{
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
#else /* ... */
	(void)self;
	(void)size;
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_TRUNC);
#endif /* !... */
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)size;
	return fs_unsupported();
#endif /* DEESYSTEM_FILE_USE_STUB */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_close(SystemFile *__restrict self) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	{
		HANDLE hHandle;
		atomic_write(&self->sf_handle, INVALID_HANDLE_VALUE);
		hHandle = atomic_xch(&self->sf_ownhandle, INVALID_HANDLE_VALUE);
		if (hHandle != INVALID_HANDLE_VALUE) {
			DBG_ALIGNMENT_DISABLE();
			if unlikely(!CloseHandle(hHandle)) {
				DBG_ALIGNMENT_ENABLE();
				return err_file_io(self);
			}
			DBG_ALIGNMENT_ENABLE();
		}
		REMEMBER_FILE_CLOSED(self);
		return 0;
	}
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

	/* Unix implementation */
#ifdef DEESYSTEM_FILE_USE_UNIX
	{
		int fd;
		atomic_write(&self->sf_handle, -1);
		fd = atomic_xch(&self->sf_ownhandle, -1);
#ifdef CONFIG_HAVE_close
		if (fd != -1) {
			DBG_ALIGNMENT_DISABLE();
			if unlikely(close(fd) != 0) {
				DBG_ALIGNMENT_ENABLE();
				return err_file_io(self);
			}
			DBG_ALIGNMENT_ENABLE();
		}
#endif /* CONFIG_HAVE_close */
		REMEMBER_FILE_CLOSED(self);
		return 0;
	}
#endif /* DEESYSTEM_FILE_USE_UNIX */

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
	{
		FILE *fp;
		atomic_write(&self->sf_handle, NULL);
		fp = (FILE *)atomic_xch(&self->sf_ownhandle, NULL);
		if (fp != NULL) {
			DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_fclose
			if (fclose((FILE *)fp) != 0) {
				DBG_ALIGNMENT_ENABLE();
				return error_file_io(self);
			}
#endif /* CONFIG_HAVE_fclose */
			DBG_ALIGNMENT_ENABLE();
		}
		REMEMBER_FILE_CLOSED(self);
		return 0;
	}
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	return fs_unsupported();
#endif /* DEESYSTEM_FILE_USE_STUB */
}



#undef deemon_file_HAVE_sysfile_getc
#undef deemon_file_HAVE_sysfile_ungetc
#undef deemon_file_HAVE_sysfile_putc
#ifdef DEESYSTEM_FILE_USE_STDIO
#define deemon_file_HAVE_sysfile_getc
#define deemon_file_HAVE_sysfile_ungetc
#define deemon_file_HAVE_sysfile_putc
#endif /* DEESYSTEM_FILE_USE_STDIO */
#ifdef DEESYSTEM_FILE_USE_STUB
#define deemon_file_HAVE_sysfile_getc
#define deemon_file_HAVE_sysfile_ungetc
#define deemon_file_HAVE_sysfile_putc
#endif /* DEESYSTEM_FILE_USE_STUB */

#ifdef deemon_file_HAVE_sysfile_getc
PRIVATE NONNULL((1)) int DCALL
sysfile_getc(SystemFile *__restrict self, dioflag_t flags) {

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
#if defined(CONFIG_HAVE_fgetc) || defined(CONFIG_HAVE_fread)
	int result;
	(void)flags;
	if unlikely(!self->sf_handle) {
		err_file_closed(self);
		return GETC_ERR;
	}
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_fgetc
	result = fgetc((FILE *)self->sf_handle);
#else /* CONFIG_HAVE_fgetc */
	{
		unsigned char chr;
		if (!fread(&chr, sizeof(chr), 1, (FILE *)self->sf_handle)) {
			result = EOF;
		} else {
			result = (int)(unsigned int)chr;
		}
	}
#endif /* !CONFIG_HAVE_fgetc */
#if defined(CONFIG_HAVE_ferror) || EOF != GETC_EOF
	if (result == EOF) {
#ifdef CONFIG_HAVE_ferror
		if (ferror((FILE *)self->sf_handle)) {
			DBG_ALIGNMENT_ENABLE();
			error_file_io(self);
			return GETC_ERR;
		}
#endif /* CONFIG_HAVE_ferror */
#if EOF != GETC_EOF
		DBG_ALIGNMENT_ENABLE();
		return GETC_EOF;
#endif
	}
#endif /* CONFIG_HAVE_ferror || EOF != GETC_EOF */
	DBG_ALIGNMENT_ENABLE();
	return result;
#else /* CONFIG_HAVE_fgetc || CONFIG_HAVE_fread */
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_GETC);
#endif /* !CONFIG_HAVE_fgetc && !CONFIG_HAVE_fread */
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)flags;
	fs_unsupported();
	return GETC_ERR;
#endif /* DEESYSTEM_FILE_USE_STUB */
}
#endif /* deemon_file_HAVE_sysfile_getc */

#ifdef deemon_file_HAVE_sysfile_ungetc
PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_ungetc(SystemFile *__restrict self, int ch) {

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
#if defined(CONFIG_HAVE_ungetc)
	int result;
	if unlikely(!self->sf_handle) {
		err_file_closed(self);
		return GETC_ERR;
	}
	DBG_ALIGNMENT_DISABLE();
	result = ungetc(ch, (FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	return result == EOF ? GETC_EOF : 0;
#else /* CONFIG_HAVE_ungetc */
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_UNGETC);
#endif /* !CONFIG_HAVE_ungetc */
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)ch;
	fs_unsupported();
	return GETC_ERR;
#endif /* DEESYSTEM_FILE_USE_STUB */
}
#endif /* deemon_file_HAVE_sysfile_ungetc */

#ifdef deemon_file_HAVE_sysfile_putc
PRIVATE int DCALL
sysfile_putc(SystemFile *__restrict self, int ch, dioflag_t flags) {

	/* Stdio implementation */
#ifdef DEESYSTEM_FILE_USE_STDIO
#if defined(CONFIG_HAVE_fputc) || defined(CONFIG_HAVE_fwrite)
#ifndef CONFIG_HAVE_fputc
	unsigned char chr;
#endif /* !CONFIG_HAVE_fputc */
	(void)flags;
	if unlikely(!self->sf_handle) {
		err_file_closed(self);
		return GETC_ERR;
	}
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_fputc
	if (fputc(ch, (FILE *)self->sf_handle) == EOF)
#else /* CONFIG_HAVE_fputc */
	chr = (unsigned char)(unsigned int)ch;
	if (fwrite(&chr, sizeof(chr), 1, (FILE *)self->sf_handle) == 0)
#endif /* !CONFIG_HAVE_fputc */
	{
#ifdef CONFIG_HAVE_ferror
		if (ferror((FILE *)self->sf_handle)) {
			DBG_ALIGNMENT_ENABLE();
			error_file_io(self);
			return GETC_ERR;
		}
#endif /* CONFIG_HAVE_ferror */
		return GETC_EOF;
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
#else /* CONFIG_HAVE_fputc || CONFIG_HAVE_fwrite */
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_PUTC);
#endif /* !CONFIG_HAVE_fputc && !CONFIG_HAVE_fwrite */
#endif /* DEESYSTEM_FILE_USE_STDIO */

	/* Stub implementation */
#ifdef DEESYSTEM_FILE_USE_STUB
	(void)self;
	(void)ch;
	(void)flags;
	fs_unsupported();
	return GETC_ERR;
#endif /* DEESYSTEM_FILE_USE_STUB */
}
#endif /* deemon_file_HAVE_sysfile_putc */


#ifdef DEESYSTEM_FILE_USE_WINDOWS
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_osfhandle_np(SystemFile *__restrict self) {
	DeeSysFD result;
	result = DeeSystemFile_Fileno((DeeObject *)self);
	if unlikely(!result)
		goto err;
	return DeeInt_NewUIntptr((uintptr_t)result);
err:
	return NULL;
}
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_UNIX
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_fileno_np(SystemFile *__restrict self) {
	DeeSysFD result;
	result = DeeSystemFile_Fileno((DeeObject *)self);
	if unlikely(result == (DeeSysFD)-1)
		goto err;
	return DeeInt_NewInt((int)result);
err:
	return NULL;
}
#endif /* DEESYSTEM_FILE_USE_UNIX */


#undef DeeSysFD_AsUnixFd
#ifdef DEESYSTEM_FILE_USE_UNIX
#define DeeSysFD_AsUnixFd(sysfd) (sysfd)
#elif defined(CONFIG_HAVE_fileno) && defined(DEESYSTEM_FILE_USE_STDIO)
#define DeeSysFD_AsUnixFd(sysfd) fileno(sysfd)
#endif /* ... */

/* Figure out how to implement `isatty()' */
#undef sysfile_isatty_USE_nt_sysfile_gettype
#undef sysfile_isatty_USE_fisatty
#undef sysfile_isatty_USE_isatty
#undef sysfile_isatty_USE_isastdfile_stdio
#undef sysfile_isatty_USE_isastdfile
#undef sysfile_isatty_USE_return_false
#ifdef DEESYSTEM_FILE_USE_WINDOWS
#define sysfile_isatty_USE_nt_sysfile_gettype
#elif defined(DEESYSTEM_FILE_USE_STDIO) && defined(CONFIG_HAVE_fisatty)
#define sysfile_isatty_USE_fisatty
#elif defined(DeeSysFD_AsUnixFd) && defined(CONFIG_HAVE_isatty)
#define sysfile_isatty_USE_isatty
#elif defined(DEESYSTEM_FILE_USE_STDIO)
#define sysfile_isatty_USE_isastdfile_stdio
#elif defined(DeeSysFD_AsUnixFd) && (defined(STDIN_FILENO) || defined(STDOUT_FILENO) || defined(STDERR_FILENO))
#define sysfile_isatty_USE_isastdfile
#else /* ... */
#define sysfile_isatty_USE_return_false
#endif /* !... */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_isatty(SystemFile *__restrict self) {
#ifdef sysfile_isatty_USE_nt_sysfile_gettype
	DWORD result;
	result = nt_sysfile_gettype(self);
	if unlikely(result == FILE_TYPE_UNKNOWN)
		goto err;
	return_bool_(result == FILE_TYPE_CHAR);
err:
	return NULL;
#endif /* sysfile_isatty_USE_nt_sysfile_gettype */

#ifdef sysfile_isatty_USE_fisatty
	int result;
	DBG_ALIGNMENT_DISABLE();
	result = fisatty((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result)
		return_true;
#if defined(EINVAL) || defined(ENOTTY)
	/* Check our whitelist of errors that indicate not-a-tty. */
	DeeSystem_IF_E2(DeeSystem_GetErrno(),
	                EINVAL,
	                ENOTTY,
	                return_false);
	error_file_io(self);
	return NULL;
#else /* EINVAL || ENOTTY */
	return_false;
#endif /* !EINVAL && !ENOTTY */
#endif /* sysfile_isatty_USE_fisatty */

#ifdef sysfile_isatty_USE_isatty
	int result;
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
	/* BUG BUG BUG: Cygwin doesn't set errno when isatty()
	 *              returns `0' because file isn't a tty */
	DeeSystem_SetErrno(ENOTTY);
#endif /* __CYGWIN__ || __CYGWIN32__ */
	DBG_ALIGNMENT_DISABLE();
	result = isatty(DeeSysFD_AsUnixFd(self->sf_handle));
	DBG_ALIGNMENT_ENABLE();
	if (result)
		return_true;
#if defined(EINVAL) || defined(ENOTTY)
	/* Check our whitelist of errors that indicate not-a-tty. */
	DeeSystem_IF_E2(DeeSystem_GetErrno(), ENOTTY, EINVAL, return_false);
	err_file_io(self);
	return NULL;
#else /* EINVAL || ENOTTY */
	return_false;
#endif /* !EINVAL && !ENOTTY */
#endif /* sysfile_isatty_USE_isatty */

#ifdef sysfile_isatty_USE_isastdfile_stdio
	if ((FILE *)self->sf_handle == stdin)
		goto is_an_std_file;
	if ((FILE *)self->sf_handle == stdout)
		goto is_an_std_file;
	if ((FILE *)self->sf_handle == stderr)
		goto is_an_std_file;
	return_false;
is_an_std_file:
	return_true;
#endif /* sysfile_isatty_USE_isastdfile_stdio */

#ifdef sysfile_isatty_USE_isastdfile
#ifdef STDIN_FILENO
	if (DeeSysFD_AsUnixFd(self->sf_handle) == STDIN_FILENO)
		goto is_an_std_file;
#endif /* STDIN_FILENO */
#ifdef STDOUT_FILENO
	if (DeeSysFD_AsUnixFd(self->sf_handle) == STDOUT_FILENO)
		goto is_an_std_file;
#endif /* STDOUT_FILENO */
#ifdef STDERR_FILENO
	if (DeeSysFD_AsUnixFd(self->sf_handle) == STDERR_FILENO)
		goto is_an_std_file;
#endif /* STDERR_FILENO */
	return_false;
is_an_std_file:
	return_true;
#endif /* sysfile_isatty_USE_isastdfile */

#ifdef sysfile_isatty_USE_return_false
	(void)self;
	return_false;
#endif /* sysfile_isatty_USE_return_false */
}



#if defined(DeeSysFD_GETSET) && defined(DeeSysFD_IS_HANDLE)
#define STR_osfhandle_np DeeString_STR(&str_getsysfd)
#else /* DeeSysFD_GETSET && DeeSysFD_IS_HANDLE */
#define STR_osfhandle_np DeeSysFD_HANDLE_GETSET
#endif /* !DeeSysFD_GETSET || !DeeSysFD_IS_HANDLE */

#if defined(DeeSysFD_GETSET) && defined(DeeSysFD_IS_FILE)
#define STR_fileno_np DeeString_STR(&str_getsysfd)
#else /* DeeSysFD_GETSET && DeeSysFD_IS_FILE */
#define STR_fileno_np DeeSysFD_INT_GETSET
#endif /* !DeeSysFD_GETSET || !DeeSysFD_IS_FILE */


PRIVATE struct type_getset tpconst sysfile_getsets[] = {
	TYPE_GETTER(STR_isatty, &sysfile_isatty, "->?Dbool"),
#ifndef DEESYSTEM_FILE_USE_STDIO /* In the stdio-backend, "filename" is a member instead of a getset! */
	TYPE_GETTER(STR_filename, &DeeSystemFile_Filename, "->?Dstring"),
#endif /* !DEESYSTEM_FILE_USE_STDIO */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	TYPE_GETTER(STR_osfhandle_np, &sysfile_osfhandle_np, "->?Dint"),
#endif /* DEESYSTEM_FILE_USE_WINDOWS */
#ifdef DEESYSTEM_FILE_USE_UNIX
	TYPE_GETTER(STR_fileno_np, &sysfile_fileno_np, "->?Dint"),
#endif /* DEESYSTEM_FILE_USE_UNIX */
#ifdef DEESYSTEM_FILE_USE_STDIO
	TYPE_GETTER("file", &DeeObject_NewRef,
	            "->?DFile\n"
	            "Returns @this File, indicating the self-buffering "
	            "behavior of system files on this host"),
#endif /* DEESYSTEM_FILE_USE_STDIO */
	TYPE_GETSET_END
};


#undef deemon_file_HAVE_sysfile_init_kw
#ifdef DEESYSTEM_FILE_USE_WINDOWS
#define deemon_file_HAVE_sysfile_init_kw
#endif /* DEESYSTEM_FILE_USE_WINDOWS */
#ifdef DEESYSTEM_FILE_USE_UNIX
#define deemon_file_HAVE_sysfile_init_kw
#endif /* DEESYSTEM_FILE_USE_UNIX */

#ifdef deemon_file_HAVE_sysfile_init_kw
PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_init_kw(SystemFile *__restrict self,
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {

	/* Windows implementation */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	PRIVATE DEFINE_KWLIST(kwlist, { K(fd), K(inherit), K(duplicate), KEND });
	bool inherit = false;
	bool duplicate = false;
	HANDLE hHandle;
	DeeObject *fd_obj;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|bb:_SystemFile",
	                    &fd_obj, &inherit, &duplicate))
		goto err;
	hHandle = (HANDLE)DeeNTSystem_GetHandle(fd_obj);
	if unlikely(hHandle == INVALID_HANDLE_VALUE)
		goto err;
	if (duplicate) {
		HANDLE hDuplicatedHandle;
		HANDLE hMyProcess = GetCurrentProcess();
again_duplicate:
		DBG_ALIGNMENT_DISABLE();
		if (!DuplicateHandle(hMyProcess, hHandle,
		                     hMyProcess, &hDuplicatedHandle,
		                     0, TRUE, DUPLICATE_SAME_ACCESS)) {
			DWORD dwError;
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err;
				goto again_duplicate;
			}
			DeeNTSystem_ThrowErrorf(NULL, dwError,
			                        "Failed to duplicate handle %p",
			                        hHandle);
		}
		DBG_ALIGNMENT_ENABLE();
		self->sf_handle    = (void *)hDuplicatedHandle;
		self->sf_ownhandle = (void *)hDuplicatedHandle;
	} else {
		self->sf_handle    = (void *)hHandle;
		self->sf_ownhandle = INVALID_HANDLE_VALUE;
		if (inherit)
			self->sf_ownhandle = (void *)hHandle;
	}
	self->sf_filename = NULL;
	self->sf_filetype = FILE_TYPE_UNKNOWN;
	self->sf_pendingc = 0;
	return 0;
err:
	return -1;
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_UNIX
	PRIVATE DEFINE_KWLIST(kwlist, { K(fd), K(inherit), K(duplicate), KEND });
	bool inherit = false;
	bool duplicate = false;
	int fd;
	DeeObject *fdobj;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|bb:_SystemFile",
	                    &fdobj, &inherit, &duplicate))
		goto err;
	fd = DeeUnixSystem_GetFD(fdobj);
	if unlikely(fd == -1)
		goto err;
	if (duplicate) {
#ifdef CONFIG_HAVE_dup
		int dupfd;
#ifdef EINTR
again_dup:
#endif /* EINTR */
		DBG_ALIGNMENT_DISABLE();
		dupfd = dup(fd);
		if unlikely(dupfd < 0) {
			int error = DeeSystem_GetErrno();
			DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
			if (error == EINTR)
				goto again_dup;
#endif /* EINTR */
#ifdef ENOSYS
			if (error == ENOSYS) {
				DeeError_Throwf(&DeeError_UnsupportedAPI,
				                "Unsupported function `dup'");
				goto err;
			}
#endif /* ENOSYS */
#ifdef EBADF
			if (error == EBADF) {
				DeeError_Throwf(&DeeError_FileClosed,
				                "Invalid handle %d", fd);
			}
#endif /* EBADF */
			DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
			                          "Failed to dup %d", fd);
			goto err;
		}
		DBG_ALIGNMENT_ENABLE();
#else /* CONFIG_HAVE_dup */
		DeeError_Throwf(&DeeError_UnsupportedAPI,
		                "Unsupported function `dup'");
		goto err;
#endif /* !CONFIG_HAVE_dup */
	} else {
		self->sf_handle    = (DeeSysFD)fd;
		self->sf_ownhandle = (DeeSysFD)-1;
		if (inherit)
			self->sf_ownhandle = (DeeSysFD)fd;
	}
	self->sf_filename = NULL;
	return 0;
err:
	return -1;
#endif /* DEESYSTEM_FILE_USE_UNIX */
}
#endif /* deemon_file_HAVE_sysfile_init_kw */


#undef deemon_file_HAVE_sysfile_fini
#ifdef DEESYSTEM_FILE_USE_WINDOWS
#define deemon_file_HAVE_sysfile_fini
#endif /* DEESYSTEM_FILE_USE_WINDOWS */
#ifdef DEESYSTEM_FILE_USE_UNIX
#define deemon_file_HAVE_sysfile_fini
#endif /* DEESYSTEM_FILE_USE_UNIX */
#ifdef DEESYSTEM_FILE_USE_STDIO
#define deemon_file_HAVE_sysfile_fini
#endif /* DEESYSTEM_FILE_USE_STDIO */
#ifdef DEESYSTEM_FILE_HAVE_sf_filename
#define deemon_file_HAVE_sysfile_fini
#endif /* DEESYSTEM_FILE_HAVE_sf_filename */


#ifdef deemon_file_HAVE_sysfile_fini
PRIVATE NONNULL((1)) void DCALL
sysfile_fini(SystemFile *__restrict self) {

#ifdef DEESYSTEM_FILE_USE_WINDOWS
	if (self->sf_ownhandle &&
	    self->sf_ownhandle != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		CloseHandle(self->sf_ownhandle);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_UNIX
#ifdef CONFIG_HAVE_close
	if (self->sf_ownhandle != -1) {
		DBG_ALIGNMENT_DISABLE();
		(void)close((int)self->sf_ownhandle);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_close */
#endif /* DEESYSTEM_FILE_USE_UNIX */

#ifdef DEESYSTEM_FILE_USE_STDIO
#ifdef CONFIG_HAVE_fclose
	if (self->sf_ownhandle) {
		DBG_ALIGNMENT_DISABLE();
		fclose((FILE *)self->sf_ownhandle);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_fclose */
#endif /* DEESYSTEM_FILE_USE_STDIO */

#ifdef DEESYSTEM_FILE_HAVE_sf_filename
	Dee_XDecref(self->sf_filename);
#endif /* DEESYSTEM_FILE_HAVE_sf_filename */
}
#endif /* deemon_file_HAVE_sysfile_fini */

PRIVATE NONNULL((1, 2)) void DCALL
sysfile_visit(SystemFile *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->sf_filename);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_class_sync(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":sync"))
		goto err;
#ifdef CONFIG_HAVE_sync
	DBG_ALIGNMENT_DISABLE();
	(void)sync();
	DBG_ALIGNMENT_ENABLE();
#elif defined(DEESYSTEM_FILE_USE_STDIO) && defined(CONFIG_HAVE_fflush)
	DBG_ALIGNMENT_DISABLE();
	(void)fflush(NULL);
	DBG_ALIGNMENT_ENABLE();
#elif defined(DEESYSTEM_FILE_USE_WINDOWS)
	/* TODO:
	 * >> for (filename: "\\.\<DRIVELETTER>:")
	 * >>     with (local x = File.open(filename, "r"))
	 * >>         x.sync(); // FlushFileBuffers
	 * XXX: This method requires admin privileges...
	 *      How do we handle this? Should we ignore it?
	 *      Considering how the linux alternative doesn't
	 *      require any special rights, should we simply
	 *      ignore failure?
	 *      How does cygwin implement `sync()'?
	 */
#endif /* DEESYSTEM_FILE_USE_WINDOWS */
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


/* Extra methods to emulate the `File.Buffer' API that
 * are only available when the STDIO backend is used. */
#ifdef DEESYSTEM_FILE_USE_STDIO
#undef HAVE_USABLE_setvbuf
#if (defined(CONFIG_HAVE_setvbuf) && \
     (defined(CONFIG_HAVE__IONBF) || defined(CONFIG_HAVE__IOFBF) || defined(CONFIG_HAVE__IOLBF)))
#define HAVE_USABLE_setvbuf
#endif /* CONFIG_HAVE_setvbuf && (CONFIG_HAVE__IONBF || CONFIG_HAVE__IOFBF || CONFIG_HAVE__IOLBF) */

#ifdef HAVE_USABLE_setvbuf
struct mode_name {
	union {
		char     mn_name[4]; /* Mode name. */
		uint32_t mn_nameid;
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define mn_name   _dee_aunion.mn_name
#define mn_nameid _dee_aunion.mn_nameid
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	int          mn_flag;    /* Mode flags. */
};

PRIVATE struct mode_name const mode_names[] = {
#ifdef CONFIG_HAVE__IONBF
	{ { { 'n', 'o', 'n', 'e' } }, _IONBF },
#endif /* CONFIG_HAVE__IONBF */
#ifdef CONFIG_HAVE__IOFBF
	{ { { 'f', 'u', 'l', 'l' } }, _IOFBF },
#endif /* CONFIG_HAVE__IOFBF */
#ifdef CONFIG_HAVE__IOLBF
	{ { { 'l', 'i', 'n', 'e' } }, _IOLBF },
	{ { { 'a', 'u', 't', 'o' } }, _IOLBF } /* There is no such thing in STD-C */
#endif /* CONFIG_HAVE__IOLBF */
};

/* CASEEQ(x, 'w') --> x == 'w' || x == 'W' */
#define CASEEQ(x, ch) ((x) == (ch) || (x) == (ch) - ('a' - 'A'))
#endif /* HAVE_USABLE_setvbuf */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_setbuf(SystemFile *self, size_t argc, DeeObject *const *argv) {
#ifdef HAVE_USABLE_setvbuf
	int mode;
	char const *mode_iter;
	DeeObject *file;
	char const *mode_str;
	size_t size = 0;
	unsigned int i;
	union {
		char chrs[4];
		uint32_t id;
	} buf;
	if (DeeArg_Unpack(argc, argv, "s|d:setbuf", &file, &mode_str, &size))
		goto err;
	mode_iter = mode_str;
	/* Interpret the given mode string. */
	for (;;) {
		if (CASEEQ(*mode_iter, 'n')) {
			++mode_iter;
			if (CASEEQ(mode_iter[0], 's') &&
			    (mode_iter[1] == '-' || mode_iter[1] == ',')) {
				mode_iter += 2;
			} else if (CASEEQ(mode_iter[0], 'o') &&
			           CASEEQ(mode_iter[1], 's') &&
			           CASEEQ(mode_iter[2], 'y') &&
			           CASEEQ(mode_iter[3], 'n') &&
			           CASEEQ(mode_iter[4], 'c') &&
			           mode_iter[5] == ',') {
				mode_iter += 6;
			}
			continue;
		}
		break;
	}
	/* Ensure that the string is at least 4 characters long. */
	if ((buf.chrs[0] = (char)DeeUni_ToLower(mode_iter[0])) == '\0' ||
	    (buf.chrs[1] = (char)DeeUni_ToLower(mode_iter[1])) == '\0' ||
	    (buf.chrs[2] = (char)DeeUni_ToLower(mode_iter[2])) == '\0' ||
	    (buf.chrs[3] = (char)DeeUni_ToLower(mode_iter[3])) == '\0')
		goto err_invalid_mode;
	if (mode_iter[4])
		goto err_invalid_mode;
	if (buf.id == ENCODE_INT32('k', 'e', 'e', 'p'))
		goto done;

	/* Parse the main mode name. */
	for (i = 0;; ++i) {
		if (i == COMPILER_LENOF(mode_names))
			goto err_invalid_mode;
		if (mode_names[i].mn_nameid != buf.id)
			continue;
		mode = mode_names[i].mn_flag; /* Found it! */
		break;
	}
again_setbuf:
	if unlikely(!self->sf_handle) {
		err_file_closed(self);
		return NULL;
	}
	/* Apply the buffer */
	DBG_ALIGNMENT_DISABLE();
	if (setvbuf((FILE *)self->sf_handle, NULL, mode, size)) {
		DBG_ALIGNMENT_ENABLE();
		if (Dee_CollectMemory(size))
			goto again_setbuf;
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
done:
	return_none;
err_invalid_mode:
	DeeError_Throwf(&DeeError_ValueError,
	                "Unrecognized buffer mode `%s'",
	                mode_str);
err:
	return NULL;
#else /* HAVE_USABLE_setvbuf */
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "Unsupported function `setvbuf()'");
	return NULL;
#endif /* !HAVE_USABLE_setvbuf */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_flush(SystemFile *self, size_t argc, DeeObject *const *argv) {
	if unlikely(DeeArg_Unpack(argc, argv, ":flush"))
		goto err;
	if unlikely(sysfile_sync(self))
		goto err;
	return_none;
err:
	return NULL;
}

#define deemon_file_HAVE_sysfile_methods
PRIVATE struct type_method tpconst sysfile_methods[] = {
	TYPE_METHOD("flush", &sysfile_flush,
	            "()\n"
	            "An alias for #sync used for compatibility with ?ABuffer?DFile"),
	TYPE_METHOD("setbuf", &sysfile_setbuf,
	            "(string mode,size=!0)\n"
	            "Set the buffering mode in a manner that is compatible with ?Asetbuf?ABuffer?DFile"),
	TYPE_METHOD_END
};

#define deemon_file_HAVE_sysfile_members
PRIVATE struct type_member tpconst sysfile_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_filename, STRUCT_OBJECT, offsetof(SystemFile, sf_filename), "->?Dstring"),
	TYPE_MEMBER_END
};
#endif /* DEESYSTEM_FILE_USE_STDIO */


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
sysfile_print(SystemFile *__restrict self, dformatprinter printer, void * arg) {
#ifdef DEESYSTEM_FILE_HAVE_sf_filename
	if (self->sf_filename != NULL)
		return DeeFormat_Printf(printer, arg, "<File %r>", self->sf_filename);
#endif /* DEESYSTEM_FILE_HAVE_sf_filename */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	return DeeFormat_Printf(printer, arg, "<File (handle %p)>", self->sf_handle);
#elif defined(DEESYSTEM_FILE_USE_UNIX)
	return DeeFormat_Printf(printer, arg, "<File (fd %d)>", self->sf_handle);
#elif defined(DEESYSTEM_FILE_USE_STDIO) && defined(CONFIG_HAVE_fileno)
	return DeeFormat_Printf(printer, arg, "<File (fd %d)>", fileno(self->sf_handle));
#else /* ... */
	(void)self;
	return DeeFormat_PRINT(printer, arg, "<File>");
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
sysfile_printrepr(SystemFile *__restrict self, dformatprinter printer, void * arg) {
#ifdef DEESYSTEM_FILE_HAVE_sf_filename
	if (self->sf_filename != NULL)
		return DeeFormat_Printf(printer, arg, "File.open(%r)", self->sf_filename);
#endif /* DEESYSTEM_FILE_HAVE_sf_filename */
#ifdef DEESYSTEM_FILE_USE_WINDOWS
	{
		char const *name = self->ob_type->ft_base.tp_name;
		if (name == NULL)
			name = DeeSystemFile_Type.ft_base.tp_name;
		return DeeFormat_Printf(printer, arg, "%s(fd: HANDLE(%#" PRFxPTR "), inherit: %s)",
		                        name, self->sf_handle,
		                        self->sf_ownhandle == self->sf_handle ? "true" : "false");
	}
#elif defined(DEESYSTEM_FILE_USE_UNIX)
	{
		char const *name = self->ob_type->ft_base.tp_name;
		if (name == NULL)
			name = DeeSystemFile_Type.ft_base.tp_name;
		return DeeFormat_Printf(printer, arg, "%s(fd: %d, inherit: %s)",
		                        name, self->sf_handle,
		                        self->sf_ownhandle == self->sf_handle ? "true" : "false");
	}
#else /* ... */
	return sysfile_print(self, printer, arg);
#endif /* !... */
}




PUBLIC DeeFileTypeObject DeeSystemFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_SystemFile",
#ifdef CONFIG_NO_DOC
		/* .tp_doc      = */ NULL,
#else /* CONFIG_NO_DOC */
		/* .tp_doc      = */ "Base-class for os-specific file I/O\n"
		                     "\n"
#ifdef deemon_file_HAVE_sysfile_init_kw
		                     "(fd:"
#ifdef DEESYSTEM_FILE_USE_WINDOWS
		                     "?X3?Dint?DFile?Ewin32:HANDLE"
#else /* DEESYSTEM_FILE_USE_WINDOWS */
		                     "?X2?Dint?DFile"
#endif /* !DEESYSTEM_FILE_USE_WINDOWS */
		                     ",inherit=!f,duplicate=!f)\n"
		                     "Construct a new SystemFile wrapper for @handle. When @inherit is "
		                     /**/ "?t, the given @handle is inherited (and automatically closed "
		                     /**/ "once the returned :File is destroyed or ?#{close}ed. When @duplicate "
		                     /**/ "is ?t, the given @handle is duplicated, and the duplicated copy "
		                     /**/ "will be stored inside (in this case, @inherit is ignored)"
#endif /* deemon_file_HAVE_sysfile_init_kw */
		                     "",
#endif /* !CONFIG_NO_DOC */
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_HASFILEOPS | TF_NONLOOPING,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor        = */ (dfunptr_t)NULL,
					/* .tp_copy_ctor   = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor   = */ (dfunptr_t)NULL,
					/* .tp_any_ctor    = */ (dfunptr_t)NULL,
					TYPE_FIXED_ALLOCATOR(SystemFile),
#ifdef deemon_file_HAVE_sysfile_init_kw
					/* .tp_any_ctor_kw = */ (dfunptr_t)&sysfile_init_kw
#else /* deemon_file_HAVE_sysfile_init_kw */
					/* .tp_any_ctor_kw = */ (dfunptr_t)NULL
#endif /* !deemon_file_HAVE_sysfile_init_kw */
				}
			},
#ifdef deemon_file_HAVE_sysfile_fini
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sysfile_fini,
#else /* deemon_file_HAVE_sysfile_fini */
			/* .tp_dtor        = */ NULL,
#endif /* !deemon_file_HAVE_sysfile_fini */
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&sysfile_print,
			/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&sysfile_printrepr
		},
		/* .tp_call          = */ NULL,
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sysfile_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
#ifdef deemon_file_HAVE_sysfile_methods
		/* .tp_methods       = */ sysfile_methods,
#else /* deemon_file_HAVE_sysfile_methods */
		/* .tp_methods       = */ NULL,
#endif /* !deemon_file_HAVE_sysfile_methods */
		/* .tp_getsets       = */ sysfile_getsets,
#ifdef deemon_file_HAVE_sysfile_members
		/* .tp_members       = */ sysfile_members,
#else /* deemon_file_HAVE_sysfile_members */
		/* .tp_members       = */ NULL,
#endif /* !deemon_file_HAVE_sysfile_members */
		/* .tp_class_methods = */ sysfile_class_methods,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ sysfile_class_members
	},
	/* .ft_read   = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dioflag_t))&sysfile_read,
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, dioflag_t))&sysfile_write,
	/* .ft_seek   = */ (dpos_t (DCALL *)(DeeFileObject *__restrict, doff_t, int))&sysfile_seek,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *__restrict))&sysfile_sync,
	/* .ft_trunc  = */ (int (DCALL *)(DeeFileObject *__restrict, dpos_t))&sysfile_trunc,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&sysfile_close,
#ifdef deemon_file_HAVE_sysfile_pread
	/* .ft_pread  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dpos_t, dioflag_t))&sysfile_pread,
#else /* deemon_file_HAVE_sysfile_pread */
	/* .ft_pread  = */ NULL,
#endif /* !deemon_file_HAVE_sysfile_pread */
#ifdef deemon_file_HAVE_sysfile_pwrite
	/* .ft_pwrite = */ (size_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, dpos_t, dioflag_t))&sysfile_pwrite,
#else /* deemon_file_HAVE_sysfile_pwrite */
	/* .ft_pwrite = */ NULL,
#endif /* !deemon_file_HAVE_sysfile_pwrite */
#ifdef deemon_file_HAVE_sysfile_getc
	/* .ft_getc   = */ (int (DCALL *)(DeeFileObject *__restrict, dioflag_t))&sysfile_getc,
#else /* deemon_file_HAVE_sysfile_getc */
	/* .ft_getc   = */ NULL,
#endif /* !deemon_file_HAVE_sysfile_getc */
#ifdef deemon_file_HAVE_sysfile_ungetc
	/* .ft_ungetc = */ (int (DCALL *)(DeeFileObject *__restrict, int))&sysfile_ungetc,
#else /* deemon_file_HAVE_sysfile_ungetc */
	/* .ft_ungetc = */ NULL,
#endif /* !deemon_file_HAVE_sysfile_ungetc */
#ifdef deemon_file_HAVE_sysfile_putc
	/* .ft_putc   = */ (int (DCALL *)(DeeFileObject *__restrict, int, dioflag_t))&sysfile_putc
#else /* deemon_file_HAVE_sysfile_putc */
	/* .ft_putc   = */ NULL
#endif /* !deemon_file_HAVE_sysfile_putc */
};

PUBLIC DeeFileTypeObject DeeFSFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FSFile",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR,
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
					TYPE_FIXED_ALLOCATOR(SystemFile)
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

#endif /* !GUARD_DEEMON_SYSTEM_FILE_C */
