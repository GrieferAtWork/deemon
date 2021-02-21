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
#ifndef GUARD_DEEMON_SYSTEM_WIN_FILE_C_INL
#define GUARD_DEEMON_SYSTEM_WIN_FILE_C_INL 1

#include <Windows.h> /* _MUST_ be included first! */
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/system.h>
#include <deemon/thread.h>

#include <hybrid/atomic.h>
#include <hybrid/byteorder.h>
#include <hybrid/host.h>
#include <hybrid/minmax.h>
#include <hybrid/unaligned.h>

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

DECL_BEGIN

typedef DeeSystemFileObject SystemFile;

#ifndef CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
#define CONFIG_OUTPUTDEBUGSTRINGA_DEFINED 1
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
				size_t part;
				part = MIN(bufsize, sizeof(temp) - sizeof(char));
				memcpyc(temp, buffer, part, sizeof(char));
				temp[part] = '\0';
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
	{ DeeString_STR(&str_isatty), &debugfile_isatty, NULL, NULL, DOC("->?Dbool") },
	{ NULL }
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

PUBLIC WUNUSED DREF /*SystemFile*/ DeeObject *DCALL
DeeFile_OpenFd(DeeSysFD fd, /*String*/ DeeObject *filename,
               int UNUSED(oflags), bool inherit_fd) {
	SystemFile *result;
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto done;
	result->sf_handle    = fd;
	result->sf_ownhandle = inherit_fd ? fd : INVALID_HANDLE_VALUE; /* Inherit. */
	result->sf_filename  = filename;
	result->sf_filetype  = (uint32_t)FILE_TYPE_UNKNOWN;
	result->sf_pendingc  = 0;
	Dee_XIncref(filename);
	DeeLFileObject_Init(result, &DeeSystemFile_Type);
done:
	return (DREF DeeObject *)result;
}

PRIVATE ATTR_COLD int DCALL err_file_closed(void) {
	return DeeError_Throwf(&DeeError_FileClosed,
	                       "File was closed");
}

PRIVATE ATTR_COLD int DCALL error_file_io(SystemFile *__restrict self) {
	if (self->sf_handle == INVALID_HANDLE_VALUE)
		return err_file_closed();
	return DeeError_Throwf(&DeeError_FSError,
	                       "I/O Operation failed");
}

INTERN WUNUSED NONNULL((1)) DeeSysFD DCALL
DeeSystemFile_Fileno(/*FileSystem*/ DeeObject *__restrict self) {
	DeeSysFD result;
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
	result = (DeeSysFD)((SystemFile *)self)->sf_handle;
	if (result == INVALID_HANDLE_VALUE)
		error_file_io((SystemFile *)self);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSystemFile_Filename(/*SystemFile*/ DeeObject *__restrict self) {
	SystemFile *me = (SystemFile *)self;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
again:
	result = me->sf_filename;
	if (result)
		Dee_Incref(result);
	else {
		HANDLE hnd = me->sf_handle;
		if unlikely(hnd == INVALID_HANDLE_VALUE) {
			err_file_closed();
			goto done;
		}
		result = DeeNTSystem_GetFilenameOfHandle(hnd);
		if unlikely(!result)
			goto done;
		/* Lazily cache the generated filename. */
		DeeFile_LockWrite(me);
		if (me->sf_filename) {
			DREF DeeObject *new_result;
			new_result = me->sf_filename;
			Dee_Incref(new_result);
			DeeFile_LockEndWrite(me);
			Dee_Decref(result);
			result = new_result;
		} else {
			/* Make sure that the handle is still the same. */
			if likely(me->sf_handle == hnd) {
				Dee_Incref(result);
				me->sf_filename = result;
				DeeFile_LockEndWrite(me);
			} else {
				/* Different handle... */
				DeeFile_LockEndWrite(me);
				Dee_Decref(result);
				goto again;
			}
		}
	}
done:
	return result;
}


PRIVATE DWORD const generic_access[4] = {
	/* [OPEN_FRDONLY] = */ FILE_GENERIC_READ,
	/* [OPEN_FWRONLY] = */ FILE_GENERIC_WRITE,
	/* [OPEN_FRDWR]   = */ FILE_GENERIC_READ | FILE_GENERIC_WRITE,
	/* [0x3]          = */ FILE_GENERIC_READ | FILE_GENERIC_WRITE
};


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_Open(/*String*/ DeeObject *__restrict filename, int oflags, int mode) {
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
		if (oflags & OPEN_FEXCL)
			dwCreationDisposition = CREATE_NEW;
		else {
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
	hFile = DeeNTSystem_CreateFile(filename, dwDesiredAccess, dwShareMode, NULL,
	                               dwCreationDisposition, dwFlagsAndAttributes, NULL);
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
	DeeLFileObject_Init(result, &DeeFSFile_Type);
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
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_OpenString(char const *__restrict filename,
                   int oflags, int mode) {
	DREF DeeObject *result, *nameob;
	/* Due to the whole wide-string mess on windows, this is
	 * just a thin wrapper around the string-object version. */
	nameob = DeeString_New(filename);
	if unlikely(!nameob)
		goto err;
	result = DeeFile_Open(nameob, oflags, mode);
	Dee_Decref(nameob);
	return result;
err:
	return NULL;
}


PRIVATE SystemFile sysf_std[] = {
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL, FILE_TYPE_UNKNOWN, 0 },
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL, FILE_TYPE_UNKNOWN, 0 },
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL, FILE_TYPE_UNKNOWN, 0 },
	{ LFILE_OBJECT_HEAD_INIT(&DebugFile_Type), (DeeObject *)INVALID_HANDLE_VALUE }
};

PUBLIC ATTR_RETNONNULL DeeObject *DCALL
DeeFile_DefaultStd(unsigned int id) {
	SystemFile *result;
	ASSERT(id <= DEE_STDDBG);
	result = &sysf_std[id];
	if unlikely(!result->sf_handle) {
		DWORD std_id;
		HANDLE std_handle;
		switch (id) {
		case DEE_STDIN: std_id = STD_INPUT_HANDLE; break;
		case DEE_STDOUT: std_id = STD_OUTPUT_HANDLE; break;
		default: std_id = STD_ERROR_HANDLE; break;
		}
		DBG_ALIGNMENT_DISABLE();
		std_handle = GetStdHandle(std_id);
		DBG_ALIGNMENT_ENABLE();
		DeeFile_LockWrite(result);
		/* Make sure not to re-open an std-file that was already closed. */
#ifndef CONFIG_NO_THREADS
		COMPILER_READ_BARRIER();
		if (!result->sf_handle)
#endif /* !CONFIG_NO_THREADS */
		{
			result->sf_handle = std_handle;
#if 1 /* Without this, close() on standard descriptors would \
       * leave the file open (which might actually be intended?) */
			result->sf_ownhandle = std_handle;
#endif
		}
		DeeFile_LockEndWrite(result);
	}
	return (DeeObject *)result;
}


/* Determine if the referenced file is a TTY file.
 * @return: * :                One of `FILE_TYPE_*'.
 * @return: FILE_TYPE_UNKNOWN: An error occurred and was thrown. */
PRIVATE DWORD DCALL
nt_sysfile_gettype(SystemFile *__restrict self) {
	if (self->sf_filetype == (uint32_t)FILE_TYPE_UNKNOWN) {
		DWORD type;
		DBG_ALIGNMENT_DISABLE();
		type = GetFileType(self->sf_handle);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(type == FILE_TYPE_UNKNOWN) {
			error_file_io(self);
			return FILE_TYPE_UNKNOWN;
		}
		ATOMIC_CMPXCH(self->sf_filetype,
		              (uint32_t)FILE_TYPE_UNKNOWN,
		              (uint32_t)type);
	}
	return (DWORD)self->sf_filetype;
}

PRIVATE DWORD DCALL
nt_sysfile_trygettype(SystemFile *__restrict self) {
	if (self->sf_filetype == (uint32_t)FILE_TYPE_UNKNOWN) {
		DWORD type;
		DBG_ALIGNMENT_DISABLE();
		type = GetFileType(self->sf_handle);
		DBG_ALIGNMENT_ENABLE();
		if likely(type != FILE_TYPE_UNKNOWN) {
			ATOMIC_CMPXCH(self->sf_filetype,
			              (uint32_t)FILE_TYPE_UNKNOWN,
			              (uint32_t)type);
		}
	}
	return (DWORD)self->sf_filetype;
}



PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
sysfile_read(SystemFile *__restrict self,
             void *__restrict buffer,
             size_t bufsize, dioflag_t flags) {
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
	error_file_io(self);
err:
	return (size_t)-1;
}



FORCELOCAL BOOL DCALL
write_all_console_w(HANDLE hConsoleOutput,
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

FORCELOCAL BOOL DCALL
write_all_file(HANDLE hFileHandle,
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


FORCELOCAL int DCALL
os_write_utf8_to_console(SystemFile *__restrict self,
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
			tempbuf = (LPWSTR)Dee_Malloc(num_widechars * sizeof(WCHAR));
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
			if unlikely(!write_all_console_w(self->sf_handle, tempbuf, num_widechars)) {
				Dee_Free(tempbuf);
				goto fallback;
			}
			Dee_Free(tempbuf);
			return 0;
		}
		DBG_ALIGNMENT_ENABLE();
		goto fallback;
	}
	if (!write_all_console_w(self->sf_handle, stackBuffer, num_widechars))
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
	if (!write_all_file(self->sf_handle, buffer, bufsize)) {
		DBG_ALIGNMENT_DISABLE();
		if (DeeNTSystem_IsIntr(GetLastError())) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		DBG_ALIGNMENT_ENABLE();
		goto err;
	}
	return 0;
err:
	error_file_io(self);
	return -1;
}


/* @return: * : The number UTF-8! bytes processed. */
PRIVATE size_t DCALL
write_utf8_to_console(SystemFile *__restrict self,
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
	if (os_write_utf8_to_console(self, buffer, result))
		result = (size_t)-1;
	return result;
}




PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
append_pending_utf8(SystemFile *__restrict self,
                    void const *__restrict buffer,
                    size_t bufsize) {
	size_t pending_count;
	ASSERTF(bufsize <= COMPILER_LENOF(self->sf_pending),
	        "A non-encodable unicode sequence that is longer than 7 bytes? WTF?");
again:
	pending_count = ATOMIC_READ(self->sf_pendingc);
	if unlikely(pending_count) {
		size_t temp;
		unsigned char with_pending[COMPILER_LENOF(self->sf_pending) * 2];
		ASSERT(pending_count <= COMPILER_LENOF(self->sf_pending));
		memcpyc(with_pending, self->sf_pending, pending_count, sizeof(unsigned char));
		memcpyc(with_pending + pending_count, buffer, bufsize, sizeof(unsigned char));
		if (!ATOMIC_CMPXCH(self->sf_pendingc, pending_count, 0))
			goto again;
		temp = write_utf8_to_console(self, with_pending, pending_count + bufsize);
		if unlikely(temp == (size_t)-1) {
			ATOMIC_CMPXCH(self->sf_pendingc, 0, pending_count);
			goto err;
		}
		pending_count += bufsize;
		ASSERT(temp <= pending_count);
		pending_count -= temp;
		ASSERT(pending_count <= COMPILER_LENOF(self->sf_pending));
		if unlikely(pending_count)
			return append_pending_utf8(self, with_pending + temp, pending_count);
	} else {
		memcpyc(self->sf_pending, buffer, bufsize, sizeof(unsigned char));
		if (!ATOMIC_CMPXCH(self->sf_pendingc, 0, bufsize))
			goto again;
	}
	return 0;
err:
	return -1;
}

FORCELOCAL int DCALL
write_to_console(SystemFile *__restrict self,
                 void const *__restrict buffer,
                 size_t bufsize) {
	uint8_t pending_count;
	size_t num_written;
again:
	if (!bufsize)
		return 0;
	pending_count = ATOMIC_READ(self->sf_pendingc);
	if (pending_count) {
		unsigned char with_pending[64];
		size_t total_length;
		ASSERT(pending_count <= COMPILER_LENOF(self->sf_pending));
		memcpyc(with_pending, self->sf_pending, pending_count, sizeof(char));
		total_length = pending_count + bufsize;
		if (total_length > COMPILER_LENOF(with_pending))
			total_length = COMPILER_LENOF(with_pending);
		memcpyc(with_pending + pending_count, buffer,
		        total_length - pending_count, sizeof(char));
		if (!ATOMIC_CMPXCH(self->sf_pendingc, pending_count, 0))
			goto again;
		num_written = write_utf8_to_console(self,
		                                    with_pending,
		                                    total_length);
		if unlikely(num_written == (size_t)-1) {
			ATOMIC_CMPXCH(self->sf_pendingc, 0, pending_count);
			goto err;
		}
		ASSERT(num_written <= total_length);
		if (num_written < total_length) {
			if (append_pending_utf8(self,
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
	num_written = write_utf8_to_console(self,
	                                    (unsigned char *)buffer,
	                                    bufsize);
	if unlikely(num_written == (size_t)-1)
		goto err;
	ASSERT(num_written <= bufsize);
	if (num_written < bufsize) {
		if (append_pending_utf8(self,
		                        (uint8_t *)buffer + num_written,
		                        bufsize - num_written))
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE size_t DCALL
sysfile_write(SystemFile *__restrict self,
              void const *__restrict buffer,
              size_t bufsize, dioflag_t UNUSED(flags)) {
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
		if unlikely(write_to_console(self, buffer, bufsize))
			goto err;
		/* TODO: Support for-, and emulation of `ENABLE_VIRTUAL_TERMINAL_PROCESSING' */
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
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return bytes_written;
err:
	return (size_t)-1;
}


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
} my_OVERLAPPED;

PRIVATE size_t DCALL
sysfile_pread(SystemFile *__restrict self,
              void *__restrict buffer,
              size_t bufsize, dpos_t pos,
              dioflag_t UNUSED(flags)) {
	DWORD bytes_written;
	my_OVERLAPPED overlapped;
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
	                      &bytes_written,
	                      (LPOVERLAPPED)&overlapped)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(error)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return bytes_written;
err:
	return (size_t)-1;
}

PRIVATE size_t DCALL
sysfile_pwrite(SystemFile *__restrict self,
               void const *__restrict buffer,
               size_t bufsize, dpos_t pos,
               dioflag_t UNUSED(flags)) {
	DWORD bytes_written;
	my_OVERLAPPED overlapped;
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
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return bytes_written;
err:
	return (size_t)-1;
}

PRIVATE dpos_t DCALL
sysfile_seek(SystemFile *__restrict self, doff_t off, int whence) {
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
			return error_file_io(self);
		}
	}
	DBG_ALIGNMENT_ENABLE();
	return (dpos_t)result | ((dpos_t)high << 32);
err:
	return (dpos_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL sysfile_sync(SystemFile *__restrict self) {
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
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
done:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_trunc(SystemFile *__restrict self, dpos_t size) {
	dpos_t old_pos = sysfile_seek(self, 0, SEEK_CUR);
	if unlikely(old_pos == (dpos_t)-1)
		goto err;
	if ((dpos_t)old_pos != size) {
		if unlikely(sysfile_seek(self, (doff_t)size, SEEK_SET) < 0)
			goto err;
	}
	DBG_ALIGNMENT_DISABLE();
	if unlikely(!SetEndOfFile(self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	if ((dpos_t)old_pos != size) {
		if unlikely(sysfile_seek(self, old_pos, SEEK_SET) == (dpos_t)-1)
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_close(SystemFile *__restrict self) {
	if (self->sf_ownhandle != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		if unlikely(!CloseHandle(self->sf_ownhandle)) {
			DBG_ALIGNMENT_ENABLE();
			return error_file_io(self);
		}
		DBG_ALIGNMENT_ENABLE();
	}
	self->sf_handle    = INVALID_HANDLE_VALUE;
	self->sf_ownhandle = INVALID_HANDLE_VALUE;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_osfhandle(SystemFile *__restrict self) {
	DeeSysFD result;
	result = DeeSystemFile_Fileno((DeeObject *)self);
	if unlikely(!result)
		goto err;
	return DeeInt_NewUIntptr((uintptr_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_isatty(SystemFile *__restrict self) {
	DWORD result;
	result = nt_sysfile_gettype(self);
	if unlikely(result == FILE_TYPE_UNKNOWN)
		goto err;
	return_bool_(result == FILE_TYPE_CHAR);
err:
	return NULL;
}

#if defined(DeeSysFD_GETSET) && defined(DeeSysFS_IS_HANDLE)
#define STR_osfhandle DeeString_STR(&str_getsysfd)
#else /* DeeSysFD_GETSET && DeeSysFS_IS_HANDLE */
#define STR_osfhandle DeeSysFD_HANDLE_GETSET
#endif /* !DeeSysFD_GETSET || !DeeSysFS_IS_HANDLE */

PRIVATE struct type_getset tpconst sysfile_getsets[] = {
	{ STR_osfhandle, (DREF DeeObject * (DCALL *)(DeeObject *))&sysfile_osfhandle, NULL, NULL, DOC("->?Dint") },
	{ DeeString_STR(&str_isatty), (DREF DeeObject *(DCALL *)(DeeObject *))&sysfile_isatty, NULL, NULL, DOC("->?Dbool") },
	{ "filename", &DeeSystemFile_Filename, NULL, NULL, DOC("->?Dstring") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_init_kw(SystemFile *__restrict self,
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {
	PRIVATE DEFINE_KWLIST(kwlist, { K(handle), K(inherit), K(duplicate), KEND });
	bool inherit = false;
	bool duplicate = false;
	HANDLE hHandle;
	DeeObject *hHandleObj;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|bb:_SystemFile",
	                    &hHandleObj, &inherit, &duplicate))
		goto err;
	hHandle = (HANDLE)DeeNTSystem_GetHandle(hHandleObj);
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
	rwlock_init(&self->fo_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
sysfile_fini(SystemFile *__restrict self) {
	if (self->sf_ownhandle &&
	    self->sf_ownhandle != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		CloseHandle(self->sf_ownhandle);
		DBG_ALIGNMENT_ENABLE();
	}
	Dee_XDecref(self->sf_filename);
}

PRIVATE NONNULL((1, 2)) void DCALL
sysfile_visit(SystemFile *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->sf_filename);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_class_sync(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":sync"))
		goto err;
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
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst sysfile_class_methods[] = {
	{ "sync", &sysfile_class_sync,
	  DOC("()\n"
	      "Synchronize all unwritten data with the host operating system") },
	{ NULL }
};

PRIVATE struct type_member tpconst sysfile_class_members[] = {
	TYPE_MEMBER_CONST("Fs", (DeeTypeObject *)&DeeFSFile_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeFileTypeObject DeeSystemFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_SystemFile",
		/* .tp_doc      = */ DOC("(handle:?X3?Dint?DFile?Ewin32:HANDLE,inherit=!f,duplicate=!f)\n"
		                         "Construct a new SystemFile wrapper for @handle. When @inherit is "
		                         "?t, the given @handle is inherited (and automatically closed "
		                         "once the returned :File is destroyed or ?#{close}ed. When @duplicate "
		                         "is ?t, the given @handle is duplicated, and the duplicated copy "
		                         "will be stored inside (in this case, @inherit is ignored)"),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_HASFILEOPS,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor        = */ (dfunptr_t)NULL,
					/* .tp_copy_ctor   = */ (dfunptr_t)NULL,
					/* .tp_deep_ctor   = */ (dfunptr_t)NULL,
					/* .tp_any_ctor    = */ (dfunptr_t)NULL,
					TYPE_FIXED_ALLOCATOR(SystemFile),
					/* .tp_any_ctor_kw = */ (dfunptr_t)&sysfile_init_kw
				}
			},
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sysfile_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
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
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ sysfile_getsets,
		/* .tp_members       = */ NULL,
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
	/* .ft_pread  = */ (size_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dpos_t, dioflag_t))&sysfile_pread,
	/* .ft_pwrite = */ (size_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, dpos_t, dioflag_t))&sysfile_pwrite,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
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

#endif /* !GUARD_DEEMON_SYSTEM_WIN_FILE_C_INL */
