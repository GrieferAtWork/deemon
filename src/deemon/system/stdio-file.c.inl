/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_SYSTEM_STDC_FILE_C_INL
#define GUARD_DEEMON_SYSTEM_STDC_FILE_C_INL 1
#define _DOS_SOURCE 1
#define _LARGEFILE64_SOURCE 1

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
#include <deemon/system-features.h>

#include <hybrid/host.h>
#include <hybrid/minmax.h>
#include <hybrid/typecore.h>
#include <hybrid/wordbits.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */

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

#ifdef CONFIG_HOST_WINDOWS

#ifndef CONFIG_OUTPUTDEBUGSTRINGA_DEFINED
#define CONFIG_OUTPUTDEBUGSTRINGA_DEFINED 1
extern ATTR_DLLIMPORT void ATTR_STDCALL OutputDebugStringA(char const *lpOutputString);
extern ATTR_DLLIMPORT int ATTR_STDCALL IsDebuggerPresent(void);
#endif /* !CONFIG_OUTPUTDEBUGSTRINGA_DEFINED */

PRIVATE dssize_t DCALL
debugfile_write(DeeFileObject *__restrict UNUSED(self),
                void const *__restrict buffer, size_t bufsize,
                dioflag_t UNUSED(flags)) {
	dssize_t result;
	/* Forward all data to stderr. */
	result = DeeFile_Write(DeeFile_DefaultStderr, buffer, bufsize);
	if unlikely(result <= 0)
		goto done;
	if (bufsize > (size_t)result)
		bufsize = (size_t)result;
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
	/* Considering its purpose, always act as though debug_file
	 * is a TTY device, just so automatic buffer interfaces will
	 * act as line-oriented buffers. */
	return_true;
}

PRIVATE struct type_getset debug_file_getsets[] = {
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
		/* .tp_features = */ TF_HASFILEOPS | TF_SINGLETON,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_var = */ {
					/* .tp_ctor      = */ &debugfile_get,
					/* .tp_copy_ctor = */ &DeeObject_NewRef,
					/* .tp_deep_ctor = */ &DeeObject_NewRef,
					/* .tp_any_ctor  = */ NULL
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
#endif /* CONFIG_HOST_WINDOWS */


PUBLIC WUNUSED DREF /*SystemFile*/ DeeObject *DCALL
DeeFile_OpenFd(DeeSysFD fd, /*String*/ DeeObject *filename,
               int UNUSED(oflags), bool inherit_fd) {
	SystemFile *result;
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto done;
	result->sf_handle    = (FILE *)fd;
	result->sf_ownhandle = inherit_fd ? (FILE *)fd : NULL; /* Inherit. */
	result->sf_filename  = filename;
	Dee_XIncref(filename);
	DeeLFileObject_Init(result, &DeeSystemFile_Type);
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL error_file_closed(SystemFile *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_FileClosed, "File was closed");
}

PRIVATE WUNUSED NONNULL((1)) int DCALL error_file_io(SystemFile *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_FSError, "I/O Operation failed");
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_OpenString(char const *__restrict filename,
                   int oflags, int UNUSED(mode)) {
#if defined(CONFIG_HAVE_fopen) || defined(CONFIG_HAVE_fopen64)
	char modbuf[16], *iter = modbuf;
	DREF SystemFile *result;
	FILE *fp;
	ASSERT(filename);
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
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_fopen64
	fp = fopen64(filename, modbuf);
#else /* CONFIG_HAVE_fopen64 */
	fp = fopen(filename, modbuf);
#endif /* !CONFIG_HAVE_fopen64 */
	DBG_ALIGNMENT_ENABLE();
	if (!fp) {
		/* Handle file-not-found / already exists cases. */
		if ((oflags & OPEN_FEXCL) || !(oflags & OPEN_FCREAT))
			return ITER_DONE;
		DeeError_Throwf(&DeeError_FSError,
		                "Failed to open file %q",
		                filename);
		return NULL;
	}
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto err_fp;
	result->sf_handle    = fp;
	result->sf_ownhandle = fp; /* Inherit stream. */
	result->sf_filename  = DeeString_New(filename);
	if unlikely(!result->sf_filename)
		goto err_fp_result;
	DeeLFileObject_Init(result, &DeeFSFile_Type);
	return (DREF DeeObject *)result;
err_unsupported_mode:
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "The given open-mode combination is not supported");
	return NULL;
err_fp_result:
	DeeObject_FREE(result);
err_fp:
#ifdef CONFIG_HAVE_fclose
	fclose(fp);
#endif /* CONFIG_HAVE_fclose */
#else /* CONFIG_HAVE_fopen || CONFIG_HAVE_fopen64 */
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "Unsupported function `fopen()'");
#endif /* !CONFIG_HAVE_fopen && !CONFIG_HAVE_fopen64 */
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_Open(/*String*/ DeeObject *__restrict filename, int oflags, int mode) {
	char *utf8_filename;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		return NULL;
	return DeeFile_OpenString(utf8_filename, oflags, mode);
}


#ifndef CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD
#ifndef CONFIG_CANNOT_STATIC_INITIALIZE_SYSF_STD
/* XXX: When the std file streams are not defined as macros,
 *      we are pretty safe to assume that they are ~real~
 *      external symbols that can be linked against. */
#if !defined(stdin) && !defined(stdout) && !defined(stderr)
#define CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD 1
#endif
#endif /* !CONFIG_CANNOT_STATIC_INITIALIZE_SYSF_STD */
#endif /* !CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD */

#if (!defined(CONFIG_HAVE_stdin) ||  \
     !defined(CONFIG_HAVE_stdout) || \
     !defined(CONFIG_HAVE_stderr))
#undef CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD
#endif

#ifdef CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD
PRIVATE SystemFile sysf_std[] = {
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), stdin },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), stdout },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), stderr }
#ifdef CONFIG_HOST_WINDOWS
	,
	{ FILE_OBJECT_HEAD_INIT(&DebugFile_Type), NULL }
#endif /* CONFIG_HOST_WINDOWS */
};
PUBLIC ATTR_RETNONNULL
DeeObject *DCALL DeeFile_DefaultStd(unsigned int id) {
	ASSERT(id <= DEE_STDERR);
	return &sysf_std[id];
}
#else /* CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD */
PRIVATE uint8_t sysf_std_closed = 0;
PRIVATE SystemFile sysf_std[] = {
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL },
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL },
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL }
#ifdef CONFIG_HOST_WINDOWS
	,
	{ LFILE_OBJECT_HEAD_INIT(&DebugFile_Type), (DeeObject *)(uintptr_t)-1, NULL, NULL }
#endif /* CONFIG_HOST_WINDOWS */
};

PUBLIC ATTR_RETNONNULL
DeeObject *DCALL DeeFile_DefaultStd(unsigned int id) {
	SystemFile *result;
	ASSERT(id <= DEE_STDDBG);
	result = &sysf_std[id];
	if unlikely(!result->sf_handle) {
		FILE *new_file;
#ifdef CONFIG_HAVE___iob_func
		new_file = (__iob_func() + id);
#else /* CONFIG_HAVE___iob_func */
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
#endif /* !CONFIG_HAVE___iob_func */
		DeeFile_LockWrite(result);
		/* Make sure not to re-open an std-file that was already closed. */
		if (!(sysf_std_closed & (1 << id))) {
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
		DeeFile_LockEndWrite(result);
	}
	return (DeeObject *)result;
}
#endif /* !CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD */


#ifndef CONFIG_HAVE_fread
#ifdef CONFIG_HAVE_fgetc
#define CONFIG_HAVE_fread 1
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
#define CONFIG_HAVE_fwrite 1
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


PRIVATE dssize_t DCALL
sysfile_read(SystemFile *__restrict self, void *__restrict buffer,
             size_t bufsize, dioflag_t UNUSED(flags)) {
#if defined(CONFIG_HAVE_fread)
	size_t result;
	if unlikely(!self->sf_handle)
		return error_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
	result = (size_t)fread(buffer, 1, bufsize, (FILE *)self->sf_handle);
	if (!result && ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return (dssize_t)result;
#else /* CONFIG_HAVE_fread */
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_READ);
#endif /* !CONFIG_HAVE_fread */
}

PRIVATE dssize_t DCALL
sysfile_write(SystemFile *__restrict self,
              void const *__restrict buffer,
              size_t bufsize, dioflag_t UNUSED(flags)) {
#ifdef CONFIG_HAVE_fwrite
	size_t result;
	if unlikely(!self->sf_handle)
		return error_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
	result = (size_t)fwrite(buffer, 1, bufsize, (FILE *)self->sf_handle);
	if (!result && ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return (dssize_t)result;
#else /* CONFIG_HAVE_fwrite */
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_WRITE);
#endif /* !CONFIG_HAVE_fwrite */
}

PRIVATE doff_t DCALL
sysfile_seek(SystemFile *__restrict self, doff_t off, int whence) {
	doff_t result;
	if unlikely(!self->sf_handle)
		return error_file_closed(self);
#if defined(CONFIG_HAVE_fseeko64) && defined(CONFIG_HAVE_ftello64)
	DBG_ALIGNMENT_DISABLE();
	if (fseeko64((FILE *)self->sf_handle, (__LONGLONG)off, whence)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = ftello64((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result == -1)
		return error_file_io(self);
#elif defined(CONFIG_HAVE_fseeko) && defined(CONFIG_HAVE_ftello)
	DBG_ALIGNMENT_DISABLE();
	if (fseeko((FILE *)self->sf_handle, (off64_t)off, whence)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = (doff_t)ftello((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result == -1)
		return error_file_io(self);
#elif defined(CONFIG_HAVE_fseek) && defined(CONFIG_HAVE_ftell)
	DBG_ALIGNMENT_DISABLE();
	if (fseek((FILE *)self->sf_handle, (long)off, whence)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = (doff_t)ftello((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if ((long)result == -1L)
		return error_file_io(self);
#else /* ... */
	result = err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                    FILE_OPERATOR_SEEK);
#endif /* !... */
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_sync(SystemFile *__restrict self) {
	if unlikely(!self->sf_handle)
		return error_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
#if defined(CONFIG_HAVE_fflush) && defined(CONFIG_HAVE_ferror)
	if (fflush((FILE *)self->sf_handle) &&
	    ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
#elif defined(CONFIG_HAVE_fflush)
	fflush((FILE *)self->sf_handle);
#endif /* ... */
	DBG_ALIGNMENT_ENABLE();
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_trunc(SystemFile *__restrict self, dpos_t size) {
	if unlikely(!self->sf_handle)
		return error_file_closed(self);
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
#elif defined(CONFIG_HAVE_fileno) && \
     (defined(CONFIG_HAVE_ftruncate) || defined(CONFIG_HAVE_ftruncate64))
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
#else
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_TRUNC);
#endif
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_close(SystemFile *__restrict self) {
	if unlikely(!self->sf_handle)
		return error_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_fclose
	if (self->sf_ownhandle &&
	    fclose((FILE *)self->sf_ownhandle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
#endif /* CONFIG_HAVE_fclose */
	DBG_ALIGNMENT_ENABLE();
#ifndef CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD
	if (self >= sysf_std && self < COMPILER_ENDOF(sysf_std)) {
		/* Make sure not to re-open this file later. */
		sysf_std_closed |= 1 << (self - sysf_std);
	}
#endif /* !CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD */
	self->sf_ownhandle = NULL;
	self->sf_handle    = NULL;
	return 0;
}

PRIVATE int DCALL
sysfile_getc(SystemFile *__restrict self, dioflag_t UNUSED(flags)) {
#if defined(CONFIG_HAVE_fgetc) || defined(CONFIG_HAVE_fread)
	int result;
	if unlikely(!self->sf_handle) {
		error_file_closed(self);
		return GETC_ERR;
	}
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_fgetc
	result = fgetc((FILE *)self->sf_handle);
#else /* CONFIG_HAVE_fgetc */
	{
		unsigned char chr;
		if (!fread(&chr, sizeof(chr), 1, (FILE *)self->sf_handle))
			result = EOF;
		else {
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
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_ungetc(SystemFile *__restrict self, int ch) {
#if defined(CONFIG_HAVE_ungetc)
	int result;
	if unlikely(!self->sf_handle) {
		error_file_closed(self);
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
}

PRIVATE int DCALL
sysfile_putc(SystemFile *__restrict self, int ch,
             dioflag_t UNUSED(flags)) {
#if defined(CONFIG_HAVE_fputc) || defined(CONFIG_HAVE_fwrite)
#ifndef CONFIG_HAVE_fputc
	unsigned char chr;
#endif /* !CONFIG_HAVE_fputc */
	if unlikely(!self->sf_handle) {
		error_file_closed(self);
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
}

INTERN DeeSysFD DCALL
DeeSystemFile_Fileno(/*SystemFile*/ DeeObject *__restrict self) {
#if 1
#define CONFIG_DONT_EXPOSE_FILENO 1
	/* Due to the unpredictable race condition and the fact that
	 * it's up to the kernel to deal with closed file descriptors,
	 * we could never safely expose the underlying FILE * to the
	 * user... */
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "The host does not implement a safe way of using fileno()");
	return NULL;
#else
	DeeSysFD result;
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
	result = (DeeSysFD)((SystemFile *)self)->sf_handle;
	if (result == NULL)
		error_file_closed((SystemFile *)self);
	return result;
#endif
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSystemFile_Filename(/*SystemFile*/ DeeObject *__restrict self) {
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
	result = ((SystemFile *)self)->sf_filename;
	if unlikely(!result) {
		err_cant_access_attribute(Dee_TYPE(self), "filename",
		                          ATTR_ACCESS_GET);
	}
	return NULL;
}


#undef sysfile_isatty_USE_ISATTY
#undef sysfile_isatty_USE_ISASTDFILE
#undef sysfile_isatty_USE_RETURN_FALSE
#if defined(CONFIG_HAVE_fileno) && defined(CONFIG_HAVE_isatty)
#define sysfile_isatty_USE_ISATTY 1
#elif defined(CONFIG_HAVE_stdin) ||  defined(CONFIG_HAVE_stdout) || defined(CONFIG_HAVE_stderr)
#define sysfile_isatty_USE_ISASTDFILE 1
#else
#define sysfile_isatty_USE_RETURN_FALSE 1
#endif


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_isatty(SystemFile *self) {
	if unlikely(!self->sf_handle) {
		error_file_closed(self);
		return NULL;
	}
#ifdef sysfile_isatty_USE_ISATTY
	int result;
	DBG_ALIGNMENT_DISABLE();
	result = isatty(fileno((FILE *)self->sf_handle));
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
#endif /* sysfile_isatty_USE_ISATTY */

	/* General-purpose: Assume that the standard streams are connected to a TTY */
#ifdef sysfile_isatty_USE_ISASTDFILE
#ifdef CONFIG_HAVE_stdin
	if (self->sf_handle == stdin)
		goto yes;
#endif /* CONFIG_HAVE_stdin */
#ifdef CONFIG_HAVE_stdout
	if (self->sf_handle == stdout)
		goto yes;
#endif /* CONFIG_HAVE_stdout */
#ifdef CONFIG_HAVE_stderr
	if (self->sf_handle == stderr)
		goto yes;
#endif /* CONFIG_HAVE_stderr */
	return_false;
yes:
	return_true;
#endif /* sysfile_isatty_USE_ISASTDFILE */

#ifdef sysfile_isatty_USE_RETURN_FALSE
	return_false;
#endif /* sysfile_isatty_USE_RETURN_FALSE */
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

#undef FILE_HAVE_SETVBUF
#if defined(CONFIG_HAVE_setvbuf) && \
   (defined(CONFIG_HAVE__IONBF) || defined(CONFIG_HAVE__IOFBF) || defined(CONFIG_HAVE__IOLBF))
#define FILE_HAVE_SETVBUF 1
#endif /* CONFIG_HAVE_setvbuf && (CONFIG_HAVE__IONBF || CONFIG_HAVE__IOFBF || CONFIG_HAVE__IOLBF) */

#ifdef FILE_HAVE_SETVBUF
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

/* CASEEQ(x,'w') --> x == 'w' || x == 'W' */
#define CASEEQ(x, ch) ((x) == (ch) || (x) == (ch) - ('a' - 'A'))
#endif /* FILE_HAVE_SETVBUF */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_setbuf(SystemFile *self, size_t argc, DeeObject *const *argv) {
#ifdef FILE_HAVE_SETVBUF
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
			    (mode_iter[1] == '-' || mode_iter[1] == ','))
				mode_iter += 2;
			else if (CASEEQ(mode_iter[0], 'o') &&
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
	DeeFile_LockWrite(self);
	if unlikely(!self->sf_handle) {
		error_file_closed(self);
		DeeFile_LockEndWrite(self);
		return NULL;
	}
	/* Apply the buffer */
	DBG_ALIGNMENT_DISABLE();
	if (setvbuf((FILE *)self->sf_handle, NULL, mode, size)) {
		DBG_ALIGNMENT_ENABLE();
		DeeFile_LockEndWrite(self);
		if (Dee_CollectMemory(size))
			goto again_setbuf;
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	DeeFile_LockEndWrite(self);
done:
	return_none;
err_invalid_mode:
	DeeError_Throwf(&DeeError_ValueError,
	                "Unrecognized buffer mode `%s'",
	                mode_str);
err:
	return NULL;
#else /* FILE_HAVE_SETVBUF */
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "Unsupported function `setvbuf()'");
	return NULL;
#endif /* !FILE_HAVE_SETVBUF */
}

PRIVATE struct type_method sysfile_methods[] = {
	{ "flush",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sysfile_flush,
	  DOC("()\n"
	      "An alias for #sync used for compatibility with ?ABuffer?DFile") },
	{ "setbuf",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&sysfile_setbuf,
	  DOC("(string mode,size=!0)\n"
	      "Set the buffering mode in a manner that is compatible with ?Asetbuf?ABuffer?DFile") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF SystemFile *DCALL
sysfile_getfile(SystemFile *__restrict self) {
	return_reference_(self);
}

PRIVATE struct type_getset sysfile_getsets[] = {
	{ DeeString_STR(&str_isatty),
	  (DREF DeeObject *(DCALL *)(DeeObject *))&sysfile_isatty, NULL, NULL,
	  DOC("->?Dbool") },
	{ "file",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sysfile_getfile, NULL, NULL,
	  DOC("->?DFile\n"
	      "Returns @this File, indicating the self-buffering "
	      "behavior of system files on this host") },
};

PRIVATE struct type_member sysfile_members[] = {
	TYPE_MEMBER_FIELD("filename", STRUCT_OBJECT, offsetof(SystemFile, sf_filename)),
	TYPE_MEMBER_END
};

PRIVATE NONNULL((1)) void DCALL
sysfile_fini(SystemFile *__restrict self) {
#ifdef CONFIG_HAVE_fclose
	if (self->sf_ownhandle) {
		DBG_ALIGNMENT_DISABLE();
		fclose((FILE *)self->sf_ownhandle);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_fclose */
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
		return NULL;
	/* Flush all streams. */
#ifdef CONFIG_HAVE_fflush
	DBG_ALIGNMENT_DISABLE();
	fflush(NULL);
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HAVE_fflush */
	return_none;
}

PRIVATE struct type_method sysfile_class_methods[] = {
	{ "sync", &sysfile_class_sync,
	  DOC("()\n"
	      "Synchronize all unwritten data with the host operating system") },
	{ NULL }
};

PRIVATE struct type_member sysfile_class_members[] = {
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
					/* .tp_ctor      = */ NULL,
					/* .tp_copy_ctor = */ NULL,
					/* .tp_deep_ctor = */ NULL,
					/* .tp_any_ctor  = */ NULL,
					TYPE_FIXED_ALLOCATOR(SystemFile)
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
		/* .tp_methods       = */ sysfile_methods,
		/* .tp_getsets       = */ sysfile_getsets,
		/* .tp_members       = */ sysfile_members,
		/* .tp_class_methods = */ sysfile_class_methods,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ sysfile_class_members
	},
	/* .ft_read   = */ (dssize_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dioflag_t))&sysfile_read,
	/* .ft_write  = */ (dssize_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, dioflag_t))&sysfile_write,
	/* .ft_seek   = */ (doff_t (DCALL *)(DeeFileObject *__restrict, doff_t, int))&sysfile_seek,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *__restrict))&sysfile_sync,
	/* .ft_trunc  = */ (int (DCALL *)(DeeFileObject *__restrict, dpos_t))&sysfile_trunc,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&sysfile_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ (int (DCALL *)(DeeFileObject *__restrict, dioflag_t))&sysfile_getc,
	/* .ft_ungetc = */ (int (DCALL *)(DeeFileObject *__restrict, int))&sysfile_ungetc,
	/* .ft_putc   = */ (int (DCALL *)(DeeFileObject *__restrict, int, dioflag_t))&sysfile_putc
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
					/* .tp_ctor      = */ NULL,
					/* .tp_copy_ctor = */ NULL,
					/* .tp_deep_ctor = */ NULL,
					/* .tp_any_ctor  = */ NULL,
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

#endif /* !GUARD_DEEMON_SYSTEM_STDC_FILE_C_INL */
