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

#include <hybrid/limits.h>
#include <hybrid/minmax.h>

#include <stdio.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#endif
#if defined(__unix__) || defined(__unix) || defined(unix)
#include <unistd.h>
#endif

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

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
#ifdef PAGESIZE
		/* (ab-)use the fact that the kernel can't keep us from reading
		 *  beyond the end of a buffer so long as that memory location
		 *  is located within the same page as the last byte of said
		 *  buffer (Trust me... I've written by own OS) */
		if ((bufsize <= 1000) && /* There seems to be some kind of limit here... */
		    (((uintptr_t)buffer + bufsize) & ~(uintptr_t)(PAGESIZE - 1)) ==
		    (((uintptr_t)buffer + bufsize - 1) & ~(uintptr_t)(PAGESIZE - 1)) &&
		    (*(char *)((uintptr_t)buffer + bufsize)) == '\0') {
			DBG_ALIGNMENT_DISABLE();
			OutputDebugStringA((char *)buffer);
			DBG_ALIGNMENT_ENABLE();
		} else
#endif /* PAGESIZE */
		{
			char temp[512];
			while (bufsize) {
				size_t part = MIN(bufsize, sizeof(temp) - sizeof(char));
				memcpy(temp, buffer, part);
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

PRIVATE DREF DeeObject *DCALL debugfile_get(void) {
 return_reference(DeeFile_DefaultStddbg);
}

PRIVATE DREF DeeObject *DCALL
debugfile_isatty(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":isatty"))
		return NULL;
	/* Considering its purpose, always act as though debug_file
	 * is a TTY device, just so automatic buffer interfaces will
	 * act as line-oriented buffers. */
	return_true;
}

PRIVATE struct type_method debug_file_methods[] = {
	{ DeeString_STR(&str_isatty),
	  &debugfile_isatty,
	  DOC("->?Dbool") },
	{ NULL }
};

PRIVATE DeeFileTypeObject DebugFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "debug_file",
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
		/* .tp_methods       = */debug_file_methods,
		/* .tp_getsets       = */ NULL,
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
#endif

PUBLIC DREF /*SystemFile*/ DeeObject *DCALL
DeeFile_OpenFd(dsysfd_t fd, /*String*/ DeeObject *filename,
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

PRIVATE int DCALL error_file_closed(SystemFile *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_FileClosed, "File was closed");
}

PRIVATE int DCALL error_file_io(SystemFile *__restrict self) {
	(void)self;
	return DeeError_Throwf(&DeeError_FSError, "I/O Operation failed");
}

PUBLIC DREF DeeObject *DCALL
DeeFile_OpenString(char const *__restrict filename,
                   int oflags, int UNUSED(mode)) {
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
	fp = fopen(filename, modbuf);
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
	fclose(fp);
	return NULL;
}

PUBLIC DREF DeeObject *DCALL
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

#ifdef CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD
PRIVATE SystemFile sysf_std[] = {
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), stdin },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), stdout },
	{ FILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), stderr }
#ifdef CONFIG_HOST_WINDOWS
    ,
	{ FILE_OBJECT_HEAD_INIT(&DebugFile_Type), NULL }
#endif
};
PUBLIC ATTR_RETNONNULL
DeeObject *DCALL DeeFile_DefaultStd(unsigned int id) {
 ASSERT(id <= DEE_STDERR);
 return &sysf_std[id];
}
#else
PRIVATE uint8_t sysf_std_closed = 0;
PRIVATE SystemFile sysf_std[] = {
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL },
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL },
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, NULL, NULL }
#ifdef CONFIG_HOST_WINDOWS
	,
	{ LFILE_OBJECT_HEAD_INIT(&DebugFile_Type), (DeeObject *)-1, NULL, NULL }
#endif /* CONFIG_HOST_WINDOWS */
};

PUBLIC ATTR_RETNONNULL
DeeObject *DCALL DeeFile_DefaultStd(unsigned int id) {
	SystemFile *result;
	ASSERT(id <= DEE_STDDBG);
	result = &sysf_std[id];
	if unlikely(!result->sf_handle) {
		FILE *new_file;
#if defined(_MSC_VER) || defined(__CRT_DOS)
		new_file = (__iob_func() + id);
#else
		switch (id) {
		case DEE_STDIN: new_file = stdin; break;
		case DEE_STDOUT: new_file = stdout; break;
		default: new_file = stderr; break;
		}
#endif
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
#endif
#endif
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
#endif





PRIVATE dssize_t DCALL
sysfile_read(SystemFile *__restrict self, void *__restrict buffer,
             size_t bufsize, dioflag_t UNUSED(flags)) {
	size_t result;
	if (!self->sf_handle)
		return error_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
	result = (size_t)fread(buffer, 1, bufsize, (FILE *)self->sf_handle);
	if (!result && ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return (dssize_t)result;
}

PRIVATE dssize_t DCALL
sysfile_write(SystemFile *__restrict self,
              void const *__restrict buffer,
              size_t bufsize, dioflag_t UNUSED(flags)) {
	size_t result;
	if (!self->sf_handle)
		return error_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
	result = (size_t)fwrite(buffer, 1, bufsize, (FILE *)self->sf_handle);
	if (!result && ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return (dssize_t)result;
}

PRIVATE doff_t DCALL
sysfile_seek(SystemFile *__restrict self, doff_t off, int whence) {
	doff_t result;
	if (!self->sf_handle)
		return error_file_closed(self);
#ifdef _MSC_VER
	DBG_ALIGNMENT_DISABLE();
	if (_fseeki64((FILE *)self->sf_handle, (long long)off, whence)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = _ftelli64((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result == -1)
		return error_file_io(self);
#elif defined(__USE_LARGEFILE64) && !defined(__CYGWIN__)
	DBG_ALIGNMENT_DISABLE();
	if (fseeko64((FILE *)self->sf_handle, (off64_t)off, whence)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = (doff_t)ftello64((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result == -1)
		return error_file_io(self);
#elif defined(__USE_LARGEFILE) || defined(__USE_XOPEN2K)
	DBG_ALIGNMENT_DISABLE();
	if (fseeko((FILE *)self->sf_handle, (off64_t)off, whence)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = (doff_t)ftello((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result == -1)
		return error_file_io(self);
#else
	DBG_ALIGNMENT_DISABLE();
	if (fseek((FILE *)self->sf_handle, (long)off, whence)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	result = (doff_t)ftello((FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if ((long)result == -1L)
		return error_file_io(self);
#endif
	return result;
}

PRIVATE int DCALL sysfile_sync(SystemFile *__restrict self) {
	if (!self->sf_handle)
		return error_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
	if (fflush((FILE *)self->sf_handle) &&
	    ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
}

PRIVATE int DCALL
sysfile_trunc(SystemFile *__restrict self, dpos_t size) {
	if (!self->sf_handle)
		return error_file_closed(self);
#ifdef _MSC_VER
	DBG_ALIGNMENT_DISABLE();
	if (_chsize_s(fileno((FILE *)self->sf_handle), (long long)size)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
#elif defined(__unix__) && defined(__USE_LARGEFILE64) && !defined(__CYGWIN__)
	DBG_ALIGNMENT_DISABLE();
	if (ftruncate64(fileno((FILE *)self->sf_handle), (off64_t)size)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
#elif defined(__unix__)
	DBG_ALIGNMENT_DISABLE();
	if (ftruncate(fileno((FILE *)self->sf_handle), (off_t)size)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
#else
	err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                           OPERATOR_TRUNC);
	return -1;
#endif
}

PRIVATE int DCALL
sysfile_close(SystemFile *__restrict self) {
	if (!self->sf_handle)
		return error_file_closed(self);
	DBG_ALIGNMENT_DISABLE();
	if (self->sf_ownhandle &&
	    fclose((FILE *)self->sf_ownhandle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
#ifndef CONFIG_CAN_STATIC_INITIALIZE_SYSF_STD
	if (self >= sysf_std && self < COMPILER_ENDOF(sysf_std)) {
		/* Make sure not to re-open this file later. */
		sysf_std_closed |= 1 << (self - sysf_std);
	}
#endif
	self->sf_ownhandle = NULL;
	self->sf_handle    = NULL;
	return 0;
}

PRIVATE int DCALL
sysfile_getc(SystemFile *__restrict self, dioflag_t UNUSED(flags)) {
	int result;
	if (!self->sf_handle) {
		error_file_closed(self);
		return GETC_ERR;
	}
	DBG_ALIGNMENT_DISABLE();
	result = getc((FILE *)self->sf_handle);
	if (result == EOF) {
		if (ferror((FILE *)self->sf_handle)) {
			DBG_ALIGNMENT_ENABLE();
			error_file_io(self);
			return GETC_ERR;
		}
		DBG_ALIGNMENT_ENABLE();
		return GETC_EOF;
	}
	DBG_ALIGNMENT_ENABLE();
	return result;
}

PRIVATE int DCALL
sysfile_ungetc(SystemFile *__restrict self, int ch) {
	int result;
	if (!self->sf_handle) {
		error_file_closed(self);
		return GETC_ERR;
	}
#if EOF == GETC_EOF
	DBG_ALIGNMENT_DISABLE();
	result = ungetc(ch, (FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result != EOF)
		result = 0;
	return result;
#else
	DBG_ALIGNMENT_DISABLE();
	result = ungetc(ch, (FILE *)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	return result == EOF ? GETC_EOF : 0;
#endif
}

PRIVATE int DCALL
sysfile_putc(SystemFile *__restrict self, int ch,
             dioflag_t UNUSED(flags)) {
	if (!self->sf_handle) {
		error_file_closed(self);
		return GETC_ERR;
	}
	DBG_ALIGNMENT_DISABLE();
	if (putc(ch, (FILE *)self->sf_handle) == EOF &&
	    ferror((FILE *)self->sf_handle)) {
		DBG_ALIGNMENT_ENABLE();
		error_file_io(self);
		return GETC_ERR;
	}
	DBG_ALIGNMENT_ENABLE();
	return 0;
}

INTERN dsysfd_t DCALL
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
	dsysfd_t result;
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
	result = (dsysfd_t)((SystemFile *)self)->sf_handle;
	if (result == NULL)
		error_file_closed((SystemFile *)self);
	return result;
#endif
}

INTERN DREF DeeObject *DCALL
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

PRIVATE DREF DeeObject *DCALL
sysfile_fileno(SystemFile *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
#ifdef CONFIG_DONT_EXPOSE_FILENO
	if (!DeeArg_Unpack(argc, argv, ":fileno"))
		DeeSystemFile_Fileno((DeeObject *)self);
	return NULL;
#else
	dsysfd_t result;
	if (DeeArg_Unpack(argc, argv, ":fileno"))
		return NULL;
	result = DeeSystemFile_Fileno((DeeObject *)self);
	if unlikely(!result)
		return NULL;
	return DeeInt_NewUIntptr((uintptr_t)result);
#endif
}

PRIVATE DREF DeeObject *DCALL
sysfile_isatty(SystemFile *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":isatty"))
		return NULL;
	if (!self->sf_handle) {
		error_file_closed(self);
		return NULL;
	}
	/* General-purpose: Assume that the standard streams are connected to a TTY */
	return_bool(self->sf_handle == stdin ||
	            self->sf_handle == stdout ||
	            self->sf_handle == stderr);
}

PRIVATE DREF DeeObject *DCALL
sysfile_flush(SystemFile *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":flush") ||
	    sysfile_sync(self))
		return NULL;
	return_none;
}

struct mode_name {
	union {
		char     name[4]; /* Mode name. */
		uint32_t nameid;
	};
	int          flag;    /* Mode flags. */
};

PRIVATE struct mode_name const mode_names[] = {
	{ { { 'n', 'o', 'n', 'e' } }, _IONBF },
	{ { { 'f', 'u', 'l', 'l' } }, _IOFBF },
	{ { { 'l', 'i', 'n', 'e' } }, _IOLBF },
	{ { { 'a', 'u', 't', 'o' } }, _IOLBF } /* There is no such thing in STD-C */
};

#ifdef CONFIG_LITTLE_ENDIAN
#define ENCODE4(a, b, c, d) ((d) << 24 | (c) << 16 | (b) << 8 | (a))
#else /* CONFIG_LITTLE_ENDIAN */
#define ENCODE4(a, b, c, d) ((d) | (c) << 8 | (b) << 16 | (a) << 24)
#endif /* !CONFIG_LITTLE_ENDIAN */

/* CASEEQ(x,'w') --> x == 'w' || x == 'W' */
#define CASEEQ(x, ch) ((x) == (ch) || (x) == (ch) - ('a' - 'A'))

PRIVATE DREF DeeObject *DCALL
sysfile_setbuf(SystemFile *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
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
	if (buf.id == ENCODE4('k', 'e', 'e', 'p'))
		goto done;

	/* Parse the main mode name. */
	for (i = 0;; ++i) {
		if (i == COMPILER_LENOF(mode_names))
			goto err_invalid_mode;
		if (mode_names[i].nameid != buf.id)
			continue;
		mode = mode_names[i].flag; /* Found it! */
		break;
	}
again_setbuf:
	DeeFile_LockWrite(self);
	if (!self->sf_handle) {
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
}

PRIVATE struct type_method sysfile_methods[] = {
	{ STR_FILENO,
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sysfile_fileno,
	  DOC("->?Dint") },
	{ DeeString_STR(&str_isatty),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sysfile_isatty,
	  DOC("->?Dbool") },
	{ "flush",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sysfile_flush,
	  DOC("()\n"
	      "An alias for #sync used for compatibility with :File.Buffer") },
	{ "setbuf",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&sysfile_setbuf,
	  DOC("(string mode,size=!0)\n"
	      "Set the buffering mode in a manner that is compatible with :File.Buffer.setbuf") },
	{ NULL }
};

PRIVATE DREF SystemFile *DCALL
sysfile_getfile(SystemFile *__restrict self) {
	return_reference_(self);
}

PRIVATE struct type_getset sysfile_getsets[] = {
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

PRIVATE void DCALL
sysfile_fini(SystemFile *__restrict self) {
	if (self->sf_ownhandle) {
		DBG_ALIGNMENT_DISABLE();
		fclose((FILE *)self->sf_ownhandle);
		DBG_ALIGNMENT_ENABLE();
	}
	Dee_XDecref(self->sf_filename);
}

PRIVATE DREF DeeObject *DCALL
sysfile_class_sync(DeeObject *__restrict UNUSED(self),
                   size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":sync"))
		return NULL;
	/* Flush all streams. */
	DBG_ALIGNMENT_DISABLE();
	fflush(NULL);
	DBG_ALIGNMENT_ENABLE();
	return_none;
}

PRIVATE struct type_method sysfile_class_methods[] = {
	{ "sync", &sysfile_class_sync,
	  DOC("()\n"
	      "Synchronize all unwritten data with the host operating system") },
	{ NULL }
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
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */sysfile_methods,
		/* .tp_getsets       = */sysfile_getsets,
		/* .tp_members       = */sysfile_members,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
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
		/* .tp_class_methods = */sysfile_class_methods,
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
