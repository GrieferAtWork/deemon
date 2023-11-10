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
#ifndef GUARD_DEEMON_OBJECTS_FILE_C
#define GUARD_DEEMON_OBJECTS_FILE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/mapfile.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/thread.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/host.h>
#include <hybrid/minmax.h>
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#undef byte_t
#define byte_t __BYTE_TYPE__

/* Trace self-optimizing operator inheritance. */
#if 1
#define LOG_INHERIT(base, self, what)                        \
	Dee_DPRINTF("[RT] Inherit `" what "' from %q into %q\n", \
	            (base)->tp_name, (self)->tp_name)
#else
#define LOG_INHERIT(base, self, what) (void)0
#endif

DECL_BEGIN

/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#ifdef CONFIG_HAVE_memcasecmp
#define MEMCASEEQ(a, b, s) (memcasecmp(a, b, s) == 0)
#else /* CONFIG_HAVE_memcasecmp */
#define MEMCASEEQ(a, b, s) dee_memcaseeq((byte_t *)(a), (byte_t *)(b), s)
LOCAL WUNUSED NONNULL((1, 2)) bool
dee_memcaseeq(byte_t const *a, byte_t const *b, size_t s) {
	while (s--) {
		if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
			return false;
		++a;
		++b;
	}
	return true;
}
#endif /* !CONFIG_HAVE_memcasecmp */

#define UNPACK_ARGS_0()        /* nothing */
#define UNPACK_ARGS_1(a)       , a
#define UNPACK_ARGS_2(a, b)    , a, b
#define UNPACK_ARGS_3(a, b, c) , a, b, c
#define UNPACK_ARGS(n, args)   UNPACK_ARGS_##n args

PRIVATE WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL file_read_with_getc(DeeFileObject *__restrict self, void *buffer, size_t bufsize, Dee_ioflag_t flags);
PRIVATE WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL file_write_with_putc(DeeFileObject *__restrict self, void const *buffer, size_t bufsize, Dee_ioflag_t flags);
PRIVATE WUNUSED ATTR_INOUT(1) int DCALL file_getc_with_read(DeeFileObject *__restrict self, Dee_ioflag_t flags);
PRIVATE WUNUSED ATTR_INOUT(1) int DCALL file_putc_with_write(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
PRIVATE WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL file_pread_with_seek_and_read(DeeFileObject *__restrict self, void *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
PRIVATE WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL file_pwrite_with_seek_and_write(DeeFileObject *__restrict self, void const *buffer, size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
PRIVATE WUNUSED ATTR_INOUT(1) int DCALL file_ungetc_with_seek(DeeFileObject *__restrict self, int ch);

PRIVATE WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL
file_read_with_getc(DeeFileObject *__restrict self, void *buffer,
                    size_t bufsize, Dee_ioflag_t flags) {
	size_t result;
	int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
	ft_getc = Dee_TYPE(self)->ft_getc;
	ASSERT(ft_getc != NULL);
	ASSERT(ft_getc != &file_getc_with_read);
	for (result = 0; result < bufsize; ++result) {
		int status = (*ft_getc)(self, flags);
		if unlikely(status == GETC_EOF)
			break;
		if unlikely(status == GETC_ERR)
			goto err;
		((byte_t *)buffer)[result] = (byte_t)(unsigned int)status;
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL
file_write_with_putc(DeeFileObject *__restrict self,
                     void const *buffer,
                     size_t bufsize, Dee_ioflag_t flags) {
	size_t result;
	int (DCALL *ft_putc)(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
	ft_putc = Dee_TYPE(self)->ft_putc;
	ASSERT(ft_putc != NULL);
	ASSERT(ft_putc != &file_putc_with_write);
	for (result = 0; result < bufsize; ++result) {
		int status = (*ft_putc)(self, ((byte_t const *)buffer)[result], flags);
		if unlikely(status == GETC_EOF)
			break;
		if unlikely(status == GETC_ERR)
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED ATTR_INOUT(1) int DCALL
file_getc_with_read(DeeFileObject *__restrict self,
                    Dee_ioflag_t flags) {
	size_t status;
	byte_t result;
	size_t (DCALL *ft_read)(DeeFileObject *__restrict self,
	                        void *buffer, size_t bufsize,
	                        Dee_ioflag_t flags);
	ft_read = Dee_TYPE(self)->ft_read;
	ASSERT(ft_read != NULL);
	ASSERT(ft_read != &file_read_with_getc);
	status = (*ft_read)(self, &result, 1, flags);
	ASSERT(status == 0 || status == 1 || status == (size_t)-1);
	if likely(status > 0)
		return (unsigned int)result;
	if (status == 0)
		return GETC_EOF;
	return GETC_ERR;
}

PRIVATE WUNUSED ATTR_INOUT(1) int DCALL
file_putc_with_write(DeeFileObject *__restrict self,
                     int ch, Dee_ioflag_t flags) {
	size_t status;
	byte_t byte;
	size_t (DCALL *ft_write)(DeeFileObject *__restrict self,
	                         void const *buffer, size_t bufsize,
	                         Dee_ioflag_t flags);
	ft_write = Dee_TYPE(self)->ft_write;
	ASSERT(ft_write != NULL);
	ASSERT(ft_write != &file_write_with_putc);
	byte   = (byte_t)(unsigned int)ch;
	status = (*ft_write)(self, &byte, 1, flags);
	ASSERT(status == 0 || status == 1 || status == (size_t)-1);
	if likely(status > 0)
		return 0;
	if (status == 0)
		return GETC_EOF;
	return GETC_ERR;
}

PRIVATE WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL
file_pread_with_seek_and_read(DeeFileObject *__restrict self,
                              void *buffer, size_t bufsize,
                              Dee_pos_t pos, Dee_ioflag_t flags) {
	size_t result;
	size_t (DCALL *ft_read)(DeeFileObject *__restrict self,
	                        void *buffer, size_t bufsize,
	                        Dee_ioflag_t flags);
	Dee_pos_t (DCALL *ft_seek)(DeeFileObject *__restrict self, Dee_off_t off, int whence);
	ft_seek = Dee_TYPE(self)->ft_seek;
	ASSERT(ft_seek != NULL);
	if unlikely((*ft_seek)(self, (Dee_off_t)pos, SEEK_SET) == (Dee_pos_t)-1)
		goto err;
	ft_read = Dee_TYPE(self)->ft_read;
	ASSERT(ft_read != NULL);
	result = (*ft_read)(self, buffer, bufsize, flags);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL
file_pwrite_with_seek_and_write(DeeFileObject *__restrict self,
                                void const *buffer, size_t bufsize,
                                Dee_pos_t pos, Dee_ioflag_t flags) {
	size_t result;
	size_t (DCALL *ft_write)(DeeFileObject *__restrict self,
	                         void const *buffer, size_t bufsize,
	                         Dee_ioflag_t flags);
	Dee_pos_t (DCALL *ft_seek)(DeeFileObject *__restrict self, Dee_off_t off, int whence);
	ft_seek = Dee_TYPE(self)->ft_seek;
	ASSERT(ft_seek != NULL);
	if unlikely((*ft_seek)(self, (Dee_off_t)pos, SEEK_SET) == (Dee_pos_t)-1)
		goto err;
	ft_write = Dee_TYPE(self)->ft_write;
	ASSERT(ft_write != NULL);
	result = (*ft_write)(self, buffer, bufsize, flags);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED ATTR_INOUT(1) int DCALL
file_ungetc_with_seek(DeeFileObject *__restrict self, int ch) {
	Dee_pos_t result;
	Dee_pos_t (DCALL *ft_seek)(DeeFileObject *__restrict self, Dee_off_t off, int whence);
	ft_seek = Dee_TYPE(self)->ft_seek;
	ASSERT(ft_seek != NULL);
	result = (*ft_seek)(self, (Dee_off_t)-1, SEEK_CUR);
	if unlikely(result == (Dee_pos_t)-1)
		goto err;
	return ch;
err:
	return -1;
}


/* Inherit file operators from bases, and auto-complete missing operators. */
#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field)                                                        \
	INTERN ATTR_INOUT(1) bool DCALL                                                                              \
	name(DeeFileTypeObject *__restrict self) {                                                                   \
		DeeFileTypeObject *base;                                                                                 \
		DeeTypeMRO mro;                                                                                          \
		base = DeeType_AsFileType(DeeTypeMRO_Init(&mro, DeeFileType_AsType(self)));                              \
		while ((base = DeeType_AsFileType(DeeTypeMRO_NextDirectBase(&mro, DeeFileType_AsType(base)))) != NULL) { \
			if (!DeeFileType_Check(DeeFileType_AsType(base)))                                                    \
				continue;                                                                                        \
			if (base->field == NULL) {                                                                           \
				if (!name(base))                                                                                 \
					continue;                                                                                    \
			}                                                                                                    \
			LOG_INHERIT(DeeFileType_AsType(base), DeeFileType_AsType(self), opname);                             \
			self->field = base->field;                                                                           \
			return true;                                                                                         \
		}                                                                                                        \
		return false;                                                                                            \
	}
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_seek, "operator seek", ft_seek)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_sync, "operator sync", ft_sync)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_trunc, "operator trunc", ft_trunc)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_close, "operator close", ft_close)
#undef DEFINE_TYPE_INHERIT_FUNCTION

#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field, alt_condition, altfunc, ...)                           \
	INTERN ATTR_INOUT(1) bool DCALL                                                                              \
	name(DeeFileTypeObject *__restrict self) {                                                                   \
		DeeFileTypeObject *base;                                                                                 \
		DeeTypeMRO mro;                                                                                          \
		if (alt_condition) {                                                                                     \
			self->field = altfunc;                                                                               \
			return true;                                                                                         \
		}                                                                                                        \
		base = DeeType_AsFileType(DeeTypeMRO_Init(&mro, DeeFileType_AsType(self)));                              \
		while ((base = DeeType_AsFileType(DeeTypeMRO_NextDirectBase(&mro, DeeFileType_AsType(base)))) != NULL) { \
			if (!DeeFileType_Check(DeeFileType_AsType(base)))                                                    \
				continue;                                                                                        \
			if (base->field == NULL) {                                                                           \
				if (!name(base))                                                                                 \
					continue;                                                                                    \
			}                                                                                                    \
			LOG_INHERIT(DeeFileType_AsType(base), DeeFileType_AsType(self), opname);                             \
			self->field = base->field;                                                                           \
			__VA_ARGS__;                                                                                         \
			return true;                                                                                         \
		}                                                                                                        \
		return false;                                                                                            \
	}
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_read, "operator read", ft_read,
                             self->ft_getc != NULL, &file_read_with_getc,
                             if (base->ft_getc) self->ft_getc = base->ft_getc)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_write, "operator write", ft_write,
                             self->ft_putc != NULL, &file_write_with_putc,
                             if (base->ft_putc) self->ft_putc = base->ft_putc)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_getc, "operator getc", ft_getc,
                             self->ft_read != NULL, &file_getc_with_read,
                             if (base->ft_read) self->ft_read = base->ft_read)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_putc, "operator putc", ft_putc,
                             self->ft_write != NULL, &file_putc_with_write,
                             if (base->ft_write) self->ft_write = base->ft_write)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_pread, "operator pread", ft_pread,
                             (self->ft_seek != NULL || type_inherit_file_seek(self)) &&
                             (self->ft_read != NULL || type_inherit_file_read(self)),
                             &file_pread_with_seek_and_read,
                             if (base->ft_seek) self->ft_seek = base->ft_seek;
                             if (base->ft_read) self->ft_read = base->ft_read)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_pwrite, "operator pwrite", ft_pwrite,
                             (self->ft_seek != NULL || type_inherit_file_seek(self)) &&
                             (self->ft_write != NULL || type_inherit_file_write(self)),
                             &file_pwrite_with_seek_and_write,
                             if (base->ft_seek) self->ft_seek = base->ft_seek;
                             if (base->ft_write) self->ft_write = base->ft_write)
DEFINE_TYPE_INHERIT_FUNCTION(type_inherit_file_ungetc, "operator ungetc", ft_ungetc,
                             (self->ft_seek != NULL || type_inherit_file_seek(self)),
                             &file_ungetc_with_seek,
                             if (base->ft_seek) self->ft_seek = base->ft_seek)
#undef DEFINE_TYPE_INHERIT_FUNCTION



/************************************************************************/
/* File operator invocation helpers                                     */
/************************************************************************/
PUBLIC WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_Read(DeeObject *__restrict self, void *buffer, size_t bufsize) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_read)(DeeFileObject *__restrict self, void *buffer,
			                        size_t bufsize, Dee_ioflag_t flags);
			ft_read = DeeType_AsFileType(tp_self)->ft_read;
			if likely(ft_read != NULL)
				return (*ft_read)((DeeFileObject *)self, buffer, bufsize, Dee_FILEIO_FNORMAL);
		} while (type_inherit_file_read(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_READ);
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_Readf(DeeObject *__restrict self, void *buffer,
              size_t bufsize, Dee_ioflag_t flags) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_read)(DeeFileObject *__restrict self, void *buffer,
			                        size_t bufsize, Dee_ioflag_t flags);
			ft_read = DeeType_AsFileType(tp_self)->ft_read;
			if likely(ft_read != NULL)
				return (*ft_read)((DeeFileObject *)self, buffer, bufsize, flags);
		} while (type_inherit_file_read(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_READ);
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL
DeeFile_Write(DeeObject *__restrict self, void const *buffer, size_t bufsize) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_write)(DeeFileObject *__restrict self, void const *buffer,
			                         size_t bufsize, Dee_ioflag_t flags);
			ft_write = DeeType_AsFileType(tp_self)->ft_write;
			if likely(ft_write != NULL)
				return (*ft_write)((DeeFileObject *)self, buffer, bufsize, Dee_FILEIO_FNORMAL);
		} while (type_inherit_file_write(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_WRITE);
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL
DeeFile_Writef(DeeObject *__restrict self, void const *buffer,
               size_t bufsize, Dee_ioflag_t flags) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_write)(DeeFileObject *__restrict self, void const *buffer,
			                         size_t bufsize, Dee_ioflag_t flags);
			ft_write = DeeType_AsFileType(tp_self)->ft_write;
			if likely(ft_write != NULL)
				return (*ft_write)((DeeFileObject *)self, buffer, bufsize, flags);
		} while (type_inherit_file_write(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_WRITE);
}

PUBLIC WUNUSED ATTR_INOUT(1) Dee_pos_t DCALL
DeeFile_Seek(DeeObject *__restrict self, Dee_off_t off, int whence) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			Dee_pos_t (DCALL *ft_seek)(DeeFileObject *__restrict self, Dee_off_t off, int whence);
			ft_seek = DeeType_AsFileType(tp_self)->ft_seek;
			if likely(ft_seek != NULL)
				return (*ft_seek)((DeeFileObject *)self, off, whence);
		} while (type_inherit_file_seek(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (Dee_pos_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_SEEK);
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_Sync(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_sync)(DeeFileObject *__restrict self);
			ft_sync = DeeType_AsFileType(tp_self)->ft_sync;
			if likely(ft_sync != NULL)
				return (*ft_sync)((DeeFileObject *)self);
		} while (type_inherit_file_sync(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_SYNC);
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_Trunc(DeeObject *__restrict self, Dee_pos_t size) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_trunc)(DeeFileObject *__restrict self, Dee_pos_t size);
			ft_trunc = DeeType_AsFileType(tp_self)->ft_trunc;
			if likely(ft_trunc != NULL)
				return (*ft_trunc)((DeeFileObject *)self, size);
		} while (type_inherit_file_trunc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_TRUNC);
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_Close(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_close)(DeeFileObject *__restrict self);
			ft_close = DeeType_AsFileType(tp_self)->ft_close;
			if likely(ft_close != NULL)
				return (*ft_close)((DeeFileObject *)self);
		} while (type_inherit_file_close(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_CLOSE);
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_Ungetc(DeeObject *__restrict self, int ch) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_ungetc)(DeeFileObject *__restrict self, int ch);
			ft_ungetc = DeeType_AsFileType(tp_self)->ft_ungetc;
			if likely(ft_ungetc != NULL)
				return (*ft_ungetc)((DeeFileObject *)self, ch);
		} while (type_inherit_file_ungetc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return GETC_EOF;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
#if GETC_ERR == -1
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_UNGETC);
#else /* GETC_ERR == -1 */
	err_unimplemented_operator(tp_self, FILE_OPERATOR_UNGETC);
	return GETC_ERR;
#endif /* GETC_ERR != -1 */
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_Getc(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
			ft_getc = DeeType_AsFileType(tp_self)->ft_getc;
			if likely(ft_getc != NULL)
				return (*ft_getc)((DeeFileObject *)self, Dee_FILEIO_FNORMAL);
		} while (type_inherit_file_getc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return GETC_EOF;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
#if GETC_ERR == -1
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
#else /* GETC_ERR == -1 */
	err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
	return GETC_ERR;
#endif /* GETC_ERR != -1 */
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_Getcf(DeeObject *__restrict self, Dee_ioflag_t flags) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
			ft_getc = DeeType_AsFileType(tp_self)->ft_getc;
			if likely(ft_getc != NULL)
				return (*ft_getc)((DeeFileObject *)self, flags);
		} while (type_inherit_file_getc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return GETC_EOF;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
#if GETC_ERR == -1
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
#else /* GETC_ERR == -1 */
	err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
	return GETC_ERR;
#endif /* GETC_ERR != -1 */
}

PRIVATE WUNUSED ATTR_INOUT(1) int
(DCALL DeeFile_unicode_unget)(DeeFileObject *__restrict self, byte_t byte0,
                              byte_t const *bytev, size_t bytec) {
	int result;
	while (bytec) {
		--bytec;
		result = DeeFile_Ungetc((DeeObject *)self, bytev[bytec]);
		if (result != 0)
			goto done;
	}
	result = DeeFile_Ungetc((DeeObject *)self, byte0);
	if (result == 0)
		result = GETC_EOF;
done:
	return result;
}

#if GETC_ERR < 0 && GETC_EOF < 0
#define IS_ERR_OR_EOF(x) ((x) < 0)
#else /* GETC_ERR < 0 && GETC_EOF < 0 */
#define IS_ERR_OR_EOF(x) ((x) == GETC_EOF || (x) == GETC_ERR)
#endif /* GETC_ERR >= 0 || GETC_EOF >= 0 */

PRIVATE WUNUSED ATTR_INOUT(1) uint32_t
(DCALL DeeFile_unicode_readutf8)(DeeFileObject *__restrict self,
                                 int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags),
                                 byte_t byte0, Dee_ioflag_t flags) {
	int b;
	byte_t buf[6];
	uint32_t result = (uint32_t)byte0;
	switch (unicode_utf8seqlen[result]) {

	case 0:
	case 1:
		break;

	case 2:
		b = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_0;
		result = (result & 0x1f) << 6;
		result |= ((byte_t)b & 0x3f);
		break;

	case 3:
		b = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_0;
		buf[0] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_1;
		result = (result & 0x0f) << 12;
		result |= (buf[0] & 0x3f) << 6;
		result |= ((byte_t)b & 0x3f);
		break;

	case 4:
		b = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_0;
		buf[0] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_1;
		buf[1] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_2;
		result = (result & 0x07) << 18;
		result |= (buf[0] & 0x3f) << 12;
		result |= (buf[1] & 0x3f) << 6;
		result |= ((byte_t)b & 0x3f);
		break;

	case 5:
		b = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_0;
		buf[0] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_1;
		buf[1] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_2;
		buf[2] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_3;
		result = (result & 0x03) << 24;
		result |= (buf[0] & 0x3f) << 18;
		result |= (buf[1] & 0x3f) << 12;
		result |= (buf[2] & 0x3f) << 6;
		result |= ((byte_t)b & 0x3f);
		break;

	case 6:
		b = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_0;
		buf[0] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_1;
		buf[1] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_2;
		buf[2] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_3;
		buf[3] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_4;
		result = (result & 0x01) << 30;
		result |= (buf[0] & 0x3f) << 24;
		result |= (buf[1] & 0x3f) << 18;
		result |= (buf[2] & 0x3f) << 12;
		result |= (buf[3] & 0x3f) << 6;
		result |= ((byte_t)b & 0x3f);
		break;

	case 7:
		b = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_0;
		buf[0] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_1;
		buf[1] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_2;
		buf[2] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_3;
		buf[3] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_4;
		buf[4] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_5;
		result  = (buf[0] & 0x03/*0x3f*/) << 30;
		result |= (buf[1] & 0x3f) << 24;
		result |= (buf[2] & 0x3f) << 18;
		result |= (buf[3] & 0x3f) << 12;
		result |= (buf[4] & 0x3f) << 6;
		result |= ((byte_t)b & 0x3f);
check_uni_char:
		if unlikely(result == (uint32_t)GETC_EOF ||
		            result == (uint32_t)GETC_ERR) {
			DeeError_Throwf(&DeeError_UnicodeDecodeError,
			                "Invalid unicode character: U+%.8" PRFx32,
			                result);
			result = (uint32_t)GETC_ERR;
		}
		break;

	case 8:
		b = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_0;
		buf[0] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_1;
		buf[1] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_2;
		buf[2] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_3;
		buf[3] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_4;
		buf[4] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_5;
		buf[5] = (byte_t)b;
		b      = (*ft_getc)(self, flags);
		if (IS_ERR_OR_EOF(b))
			goto err_6;
		/*result = (buf[0] & 0x3f) << 36;*/
		result  = (buf[1] & 0x03/*0x3f*/) << 30;
		result |= (buf[2] & 0x3f) << 24;
		result |= (buf[3] & 0x3f) << 18;
		result |= (buf[4] & 0x3f) << 12;
		result |= (buf[5] & 0x3f) << 6;
		result |= ((byte_t)b & 0x3f);
		goto check_uni_char;

	default:
		__builtin_unreachable();
	}
	return result;
	{
		size_t n;
		__IF0 { err_6: n = 6; }
		__IF0 { err_5: n = 5; }
		__IF0 { err_4: n = 4; }
		__IF0 { err_3: n = 3; }
		__IF0 { err_2: n = 2; }
		__IF0 { err_1: n = 1; }
		__IF0 { err_0: n = 0; }
		if (b == GETC_EOF)
			b = DeeFile_unicode_unget(self, byte0, buf, n);
		return (uint32_t)b;
	}
}

PUBLIC WUNUSED ATTR_INOUT(1) uint32_t DCALL
DeeFile_GetUtf8(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
			ft_getc = DeeType_AsFileType(tp_self)->ft_getc;
			if likely(ft_getc != NULL) {
				int result = (*ft_getc)((DeeFileObject *)self, Dee_FILEIO_FNORMAL);
				if unlikely(result >= 0xc0) {
					result = DeeFile_unicode_readutf8((DeeFileObject *)self, ft_getc,
					                                  (byte_t)result, Dee_FILEIO_FNORMAL);
				}
				return (uint32_t)result;
			}
		} while (type_inherit_file_getc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return (uint32_t)GETC_EOF;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
#if GETC_ERR == -1
	return (uint32_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
#else /* GETC_ERR == -1 */
	err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
	return (uint32_t)GETC_ERR;
#endif /* GETC_ERR != -1 */
}

PUBLIC WUNUSED ATTR_INOUT(1) uint32_t DCALL
DeeFile_GetUtf8f(DeeObject *__restrict self, Dee_ioflag_t flags) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
			ft_getc = DeeType_AsFileType(tp_self)->ft_getc;
			if likely(ft_getc != NULL) {
				int result = (*ft_getc)((DeeFileObject *)self, flags);
				if unlikely(result >= 0xc0) {
					result = DeeFile_unicode_readutf8((DeeFileObject *)self, ft_getc,
					                                  (byte_t)result, flags);
				}
				return (uint32_t)result;
			}
		} while (type_inherit_file_getc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return (uint32_t)GETC_EOF;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
#if GETC_ERR == -1
	return (uint32_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
#else /* GETC_ERR == -1 */
	err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
	return (uint32_t)GETC_ERR;
#endif /* GETC_ERR != -1 */
}

/* @return: Dee_GETC_ERR: Error */
PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_UngetUtf8(DeeObject *__restrict self, uint32_t ch) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_ungetc)(DeeFileObject *__restrict self, int ch);
			ft_ungetc = DeeType_AsFileType(tp_self)->ft_ungetc;
			if likely(ft_ungetc != NULL) {
				int result;
				char buf[UNICODE_UTF8_CURLEN], *endp;
				endp = unicode_writeutf8(buf, ch);
				do {
					--endp;
					result = (*ft_ungetc)((DeeFileObject *)self, (int)(unsigned int)(unsigned char)*endp);
					if unlikely(result != 0)
						break;
				} while (endp > buf);
				return result;
			}
		} while (type_inherit_file_ungetc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return GETC_EOF;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
#if GETC_ERR == -1
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_UNGETC);
#else /* GETC_ERR == -1 */
	err_unimplemented_operator(tp_self, FILE_OPERATOR_UNGETC);
	return GETC_ERR;
#endif /* GETC_ERR != -1 */
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_Putc(DeeObject *__restrict self, int ch) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_putc)(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
			ft_putc = DeeType_AsFileType(tp_self)->ft_putc;
			if likely(ft_putc != NULL)
				return (*ft_putc)((DeeFileObject *)self, ch, Dee_FILEIO_FNORMAL);
		} while (type_inherit_file_putc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return GETC_EOF;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
#if GETC_ERR == -1
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_PUTC);
#else /* GETC_ERR == -1 */
	err_unimplemented_operator(tp_self, FILE_OPERATOR_PUTC);
	return GETC_ERR;
#endif /* GETC_ERR != -1 */
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_Putcf(DeeObject *__restrict self, int ch, Dee_ioflag_t flags) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_putc)(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
			ft_putc = DeeType_AsFileType(tp_self)->ft_putc;
			if likely(ft_putc != NULL)
				return (*ft_putc)((DeeFileObject *)self, ch, flags);
		} while (type_inherit_file_putc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return GETC_EOF;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
#if GETC_ERR == -1
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_PUTC);
#else /* GETC_ERR == -1 */
	err_unimplemented_operator(tp_self, FILE_OPERATOR_PUTC);
	return GETC_ERR;
#endif /* GETC_ERR != -1 */
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_PRead(DeeObject *__restrict self, void *buffer,
              size_t bufsize, Dee_pos_t pos) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pread)(DeeFileObject *__restrict self, void *buffer,
			                         size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pread = DeeType_AsFileType(tp_self)->ft_pread;
			if likely(ft_pread != NULL)
				return (*ft_pread)((DeeFileObject *)self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL);
		} while (type_inherit_file_pread(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_PREAD);
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_PReadf(DeeObject *__restrict self, void *buffer,
               size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pread)(DeeFileObject *__restrict self, void *buffer,
			                         size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pread = DeeType_AsFileType(tp_self)->ft_pread;
			if likely(ft_pread != NULL)
				return (*ft_pread)((DeeFileObject *)self, buffer, bufsize, pos, flags);
		} while (type_inherit_file_pread(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_PREAD);
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL
DeeFile_PWrite(DeeObject *__restrict self, void const *buffer,
               size_t bufsize, Dee_pos_t pos) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pwrite)(DeeFileObject *__restrict self, void const *buffer,
			                          size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pwrite = DeeType_AsFileType(tp_self)->ft_pwrite;
			if likely(ft_pwrite != NULL)
				return (*ft_pwrite)((DeeFileObject *)self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL);
		} while (type_inherit_file_pwrite(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_PWRITE);
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL
DeeFile_PWritef(DeeObject *__restrict self, void const *buffer,
                size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pwrite)(DeeFileObject *__restrict self, void const *buffer,
			                          size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pwrite = DeeType_AsFileType(tp_self)->ft_pwrite;
			if likely(ft_pwrite != NULL)
				return (*ft_pwrite)((DeeFileObject *)self, buffer, bufsize, pos, flags);
		} while (type_inherit_file_pwrite(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_PWRITE);
}


PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_TruncHere(DeeObject *__restrict self, dpos_t *p_size) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_trunc)(DeeFileObject *__restrict self, Dee_pos_t size);
			Dee_pos_t (DCALL *ft_seek)(DeeFileObject *__restrict self, Dee_off_t off, int whence);
			ft_trunc = DeeType_AsFileType(tp_self)->ft_trunc;
			ft_seek  = DeeType_AsFileType(tp_self)->ft_seek;
			if likely(ft_trunc != NULL && ft_seek != NULL) {
				int result;
				dpos_t trunc_pos;
				/* Determine the current position and truncate the file there. */
				trunc_pos = (*ft_seek)((DeeFileObject *)self, 0, SEEK_CUR);
				if unlikely(trunc_pos == (dpos_t)-1) {
					result = -1;
				} else {
					result = (*ft_trunc)((DeeFileObject *)self, trunc_pos);
				}
				if (p_size != NULL)
					*p_size = trunc_pos;
				return result;
			}
		} while (type_inherit_file_trunc(DeeType_AsFileType(tp_self)) ||
		         type_inherit_file_seek(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return err_unimplemented_operator(tp_self, FILE_OPERATOR_TRUNC);
}



PUBLIC WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_ReadAll(DeeObject *__restrict self,
                void *buffer, size_t bufsize) {
	size_t result = 0, temp;
	for (;;) {
		temp = DeeFile_Read(self, buffer, bufsize);
		if unlikely(temp == (size_t)-1)
			return temp;
		if (!temp)
			break;
		result += temp;
		if (temp >= bufsize)
			break;
		bufsize -= temp;
		buffer = (void *)((uintptr_t)buffer + temp);
	}
	return result;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DPRINTER_CC
DeeFile_WriteAll(DeeObject *__restrict self,
                 void const *buffer,
                 size_t bufsize) {
	size_t result = 0, temp;
	for (;;) {
		temp = DeeFile_Write(self, buffer, bufsize);
		if unlikely(temp == (size_t)-1)
			return temp;
		if (!temp)
			break;
		result += temp;
		if (temp >= bufsize)
			break;
		bufsize -= temp;
		buffer = (void *)((uintptr_t)buffer + temp);
	}
	return result;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_PReadAll(DeeObject *__restrict self,
                 void *buffer,
                 size_t bufsize, dpos_t pos) {
	size_t result = 0, temp;
	for (;;) {
		temp = DeeFile_PRead(self, buffer, bufsize, pos);
		if unlikely(temp == (size_t)-1)
			return temp;
		if (!temp)
			break;
		result += temp;
		if (temp >= bufsize)
			break;
		pos += temp;
		bufsize -= temp;
		buffer = (void *)((uintptr_t)buffer + temp);
	}
	return result;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INS(2, 3) size_t DCALL
DeeFile_PWriteAll(DeeObject *__restrict self,
                  void const *buffer,
                  size_t bufsize, dpos_t pos) {
	size_t result = 0, temp;
	for (;;) {
		temp = DeeFile_PWrite(self, buffer, bufsize, pos);
		if unlikely(temp == (size_t)-1)
			return temp;
		if (!temp)
			break;
		result += temp;
		if (temp >= bufsize)
			break;
		pos += temp;
		bufsize -= temp;
		buffer = (void *)((uintptr_t)buffer + temp);
	}
	return result;
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_IsAtty(DeeObject *__restrict self) {
	DREF DeeObject *result_ob;
	int result;

	/* Very simply: Just lookup the `isatty' property. */
	result_ob = DeeObject_GetAttr(self, (DeeObject *)&str_isatty);
	if unlikely(!result_ob)
		goto err_call;
	result = DeeObject_Bool(result_ob);
	Dee_Decref(result_ob);
	return result;
err_call:
	/* Check if we can handle attribute/not-implement errors that
	 * could be interpreted as indicative of this not being a tty.
	 * Fun fact: The way that `isatty' is implemented on linux (and KOS ;) ),
	 *           is by invoking an fcntl() that is only allowed for TTY file
	 *           descriptors, then checking if errno was set, meaning that
	 *           even linux does something similar to this, just on a
	 *           different level. */
	if (DeeError_Catch(&DeeError_AttributeError) ||
	    DeeError_Catch(&DeeError_NotImplemented))
		return 0;
	return -1;
}

/* Return the system file descriptor of the given file, or throw
 * an error and return `Dee_fd_INVALID' if the file was closed,
 * or doesn't refer to a file carrying a descriptor.
 * Note that this function queries the `Dee_fd_GETSET' attribute
 * of the given object, and always fails if `Dee_fd_GETSET' isn't
 * defined for the configuration used when deemon was built.
 * NOTE: This function doesn't require that `self' actually be
 *       derived from a `deemon.File'!
 * @return: * :               The used system fD. (either a `HANDLE', `fd_t' or `FILE *')
 * @return: Dee_fd_INVALID: An error occurred. */
PUBLIC WUNUSED ATTR_INOUT(1) Dee_fd_t DCALL
DeeFile_GetSysFD(DeeObject *__restrict self) {
#ifdef Dee_fd_GETSET
	DREF DeeObject *result_ob;
	Dee_fd_t result;

	/* Special case: If the file is a system-file,  */
	if (DeeObject_InstanceOf(self, (DeeTypeObject *)&DeeSystemFile_Type))
		return DeeSystemFile_Fileno(self);

	/* General case: look for a `Dee_fd_GETSET' attribute */
	result_ob = DeeObject_GetAttr(self, (DeeObject *)&str_getsysfd);
	if unlikely(!result_ob) {
#if defined(Dee_fd_t_IS_HANDLE) && defined(CONFIG_HAVE_get_osfhandle)
		/* TODO: Also check for an attribute `Dee_fd_fileno_GETSET' */
#endif /* Dee_fd_t_IS_HANDLE && CONFIG_HAVE_get_osfhandle */
		goto err;
	}

	/* Cast the member function's return value to an integer. */
	if (DeeObject_AsFd(result_ob, &result))
		goto err_result_ob;
	Dee_Decref(result_ob);
	return result;
err_result_ob:
	Dee_Decref(result_ob);
err:
	return Dee_fd_INVALID;
#else /* Dee_fd_GETSET */
	(void)self;
	DeeError_Throwf(&DeeError_UnsupportedAPI,
	                "System file descriptors cannot be bound to objects");
	return Dee_fd_INVALID;
#endif /* Dee_fd_GETSET */
}

/* Retrieve and return the filename used to open the given file.
 * NOTE: This function automatically asserts that `self'
 *       is a `File', throwing a TypeError if it isn't.
 * For this purpose, `DeeSystemFile_Filename()' is invoked if `self'
 * is a system file, however if it isn't, `self.filename' will be
 * retrieved (using `operator getattr()') and after asserting the
 * result to be a string object, its value will be returned instead.
 * This function should be used by library functions that wish to
 * operate on a path, thus allowing them to accept file objects just
 * as well as strings for operations:
 * >> if (!DeeString_Check(arg)) {
 * >>     arg = DeeFile_Filename(arg);
 * >>     if unlikely(!arg)
 * >>         goto err;
 * >> } else {
 * >>     Dee_Incref(arg);
 * >> }
 * >> ... // Operate on a filename string `arg'
 * >> Dee_Decref(arg); */
PUBLIC WUNUSED ATTR_INOUT(1) DREF /*String*/ DeeObject *DCALL
DeeFile_Filename(DeeObject *__restrict self) {
	DREF DeeObject *result;
	/* Special case: If the file is a system-file,  */
	if (DeeObject_InstanceOf(self, (DeeTypeObject *)&DeeSystemFile_Type))
		return DeeSystemFile_Filename(self);
#if 0 /* There might be non-file types that implement a `filename' member. */
	/* Check that it's a file at all. */
	if (DeeObject_AssertType(self, (DeeTypeObject *)&DeeFile_Type))
		goto err;
#endif
	result = DeeObject_GetAttr(self, (DeeObject *)&str_filename);
	/* Validate that `filename' is actually a string. */
	if (result && DeeObject_AssertTypeExact(result, &DeeString_Type))
		Dee_Clear(result);
	return result;
/*err:*/
	/* return NULL;*/
}

/* Read text from a file, a line or block at a time.
 * @param: readall: When true, keep trying to read data until `DeeFile_Read()'
 *                  actually returns ZERO(0), rather than stopping once it returns
 *                  something other than the then effective read buffer size.
 * @return: ITER_DONE: [DeeFile_ReadLine] The file has ended. */
PUBLIC WUNUSED ATTR_INOUT(1) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_ReadLine(DeeObject *__restrict self,
                 size_t maxbytes, bool keep_lf) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
			int (DCALL *ft_ungetc)(DeeFileObject *__restrict self, int ch);
			ft_getc   = DeeType_AsFileType(tp_self)->ft_getc;
			ft_ungetc = DeeType_AsFileType(tp_self)->ft_ungetc;
			if likely(ft_getc != NULL && ft_ungetc != NULL) {
				struct bytes_printer printer = BYTES_PRINTER_INIT;

				/* Keep on reading characters until a linefeed is encountered. */
				while (BYTES_PRINTER_SIZE(&printer) < maxbytes) {
					int ch;
					ch = (*ft_getc)((DeeFileObject *)self, Dee_FILEIO_FNORMAL);
					if (ch == '\r') {
						/* If the next character is '\n', then we must consume it as well. */
						ch = (*ft_getc)((DeeFileObject *)self, Dee_FILEIO_FNORMAL);
						if (ch >= 0 && ch != '\n')
							ch = (*ft_ungetc)((DeeFileObject *)self, ch);
						if (ch == GETC_ERR)
							goto err_printer;

						/* Found a \r\n or \r-linefeed. */
						if (keep_lf) {
							if (bytes_printer_putb(&printer, '\r'))
								goto err_printer;
							if (ch == '\n' && BYTES_PRINTER_SIZE(&printer) < maxbytes &&
							    bytes_printer_putb(&printer, '\n'))
								goto err_printer;
						}
						goto done_printer;
					}
					if (ch == GETC_ERR)
						goto err_printer;
					if (ch == '\n') {
						/* Found a \n-linefeed */
						if (keep_lf && bytes_printer_putb(&printer, '\n'))
							goto err_printer;
						goto done_printer;
					}
					if (ch == GETC_EOF) {
						/* Stop on EOF */
						if (!BYTES_PRINTER_SIZE(&printer)) {
							/* Nothing was read -> return ITER_DONE */
							bytes_printer_fini(&printer);
							return ITER_DONE;
						}
						goto done_printer;
					}

					/* Print the character. */
					if (bytes_printer_putb(&printer, (byte_t)ch))
						goto err_printer;
				}
done_printer:
				return bytes_printer_pack(&printer);
err_printer:
				bytes_printer_fini(&printer);
				goto err;
			}
		} while (type_inherit_file_getc(DeeType_AsFileType(tp_self)) ||
		         type_inherit_file_ungetc(DeeType_AsFileType(tp_self)));
		if (DeeType_AsFileType(tp_self)->ft_getc != NULL) {
			/* If getc() is implemented, but ungetc() isn't, indicate the correct missing operator. */
			err_unimplemented_operator(tp_self, FILE_OPERATOR_UNGETC);
			goto err;
		}
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return ITER_DONE;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	err_unimplemented_operator(tp_self, FILE_OPERATOR_GETC);
err:
	return NULL;
}


#ifdef DeeSystemFile_GetHandle
#undef HAVE_file_read_trymap

#ifdef DEESYSTEM_FILE_USE_WINDOWS
#define HAVE_file_read_trymap
#endif /* DEESYSTEM_FILE_USE_WINDOWS */

#ifdef DEESYSTEM_FILE_USE_UNIX
#define HAVE_file_read_trymap
#endif /* DEESYSTEM_FILE_USE_UNIX */

#ifdef HAVE_file_read_trymap
#ifndef FILE_READ_MMAP_THRESHOLD
#ifdef __ARCH_PAGESIZE
#define FILE_READ_MMAP_THRESHOLD (__ARCH_PAGESIZE * 2)
#elif defined(PAGESIZE)
#define FILE_READ_MMAP_THRESHOLD (PAGESIZE * 2)
#elif defined(PAGE_SIZE)
#define FILE_READ_MMAP_THRESHOLD (PAGE_SIZE * 2)
#else /* ... */
#define FILE_READ_MMAP_THRESHOLD (4096 * 2)
#endif /* !... */
#endif /* !FILE_READ_MMAP_THRESHOLD */

INTDEF WUNUSED ATTR_INOUT((1)) ATTR_OUTS(2, 3) size_t DCALL
sysfile_read(DeeSystemFileObject *__restrict self,
             void *buffer, size_t bufsize,
             dioflag_t flags);
INTERN WUNUSED ATTR_INOUT((1)) ATTR_INS(2, 3) size_t DCALL
sysfile_pread(DeeSystemFileObject *__restrict self,
              void *buffer, size_t bufsize,
              dpos_t pos, dioflag_t flags);

PRIVATE WUNUSED DREF /*Bytes*/ DeeObject *DCALL
file_read_trymap(Dee_fd_t fd, size_t maxbytes,
                 dpos_t pos, bool readall) {
	int error;
	struct DeeMapFile map;
	DREF DeeObject *result;
	DREF DeeMapFileObject *mapob;

	error = DeeMapFile_InitSysFd(&map, fd, pos, 0, maxbytes, 0,
	                             readall ? (DEE_MAPFILE_F_MUSTMMAP | DEE_MAPFILE_F_TRYMMAP | DEE_MAPFILE_F_READALL)
	                                     : (DEE_MAPFILE_F_MUSTMMAP | DEE_MAPFILE_F_TRYMMAP));
	if (error != 0) {
		if unlikely(error < 0)
			return NULL;
		return ITER_DONE; /* Unsupported (instruct caller to use fallback read/pread) */
	}

	/* Success! -> Wrap the mapfile as `DeeMapFileObject -> DeeBytesObject' */
	mapob = DeeObject_MALLOC(DREF DeeMapFileObject);
	if unlikely(!mapob)
		goto err_map;
	DeeObject_Init(mapob, &DeeMapFile_Type);
	DeeMapFile_Move(&mapob->mf_map, &map);
	mapob->mf_rsize = DeeMapFile_GetSize(&mapob->mf_map);

	/* Now create the bytes view of the map. */
	result = DeeBytes_NewView((DeeObject *)mapob,
	                          (void *)DeeMapFile_GetBase(&mapob->mf_map),
	                          mapob->mf_rsize, Dee_BUFFER_FREADONLY);
	Dee_Decref_unlikely(mapob);
	return result;
err_map:
	DeeMapFile_Fini(&map);
	return NULL;
}
#endif /* HAVE_file_read_trymap */
#endif /* DeeSystemFile_GetHandle */


#define READTEXT_INITIAL_BUFSIZE 1024

PUBLIC WUNUSED ATTR_INOUT(1) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_ReadBytes(DeeObject *__restrict self,
                  size_t maxbytes, bool readall) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_read)(DeeFileObject *__restrict self, void *buffer,
			                        size_t bufsize, Dee_ioflag_t flags);
			ft_read = DeeType_AsFileType(tp_self)->ft_read;
			if likely(ft_read != NULL) {
#ifdef HAVE_file_read_trymap
				/* if `ft_read' belongs to `DeeSystemFile_Type', and `maxbytes' is larger
				 * than some threshold (>= 2*PAGESIZE), then try to create a file view using
				 * `DeeMapFile_InitSysFd()', which is then wrapped by file view holder object,
				 * which can then be wrapped by a regular `Bytes' object.
				 * -> That way, we can provide the user with O(1) reads from large files! */
				if ((maxbytes >= FILE_READ_MMAP_THRESHOLD) &&
				    (ft_read == (size_t (DCALL *)(DeeFileObject *__restrict self, void *__restrict, size_t, dioflag_t flags))&sysfile_read)) {
					DREF /*Bytes*/ DeeObject *result;
					Dee_fd_t os_fd = DeeSystemFile_Fileno(self);
					if unlikely(os_fd == Dee_fd_INVALID)
						goto err;
					result = file_read_trymap(os_fd, maxbytes, (dpos_t)-1, readall);
					if (result != ITER_DONE)
						return result;
				}
				/* TODO: if `ft_read' is for a FileBuffer that is currently empty, also try to mmap!
				 *       In this case we can also use the FileBuffer's position to (possibly) skip
				 *       the initial seek done during file mapping! */
#endif /* HAVE_file_read_trymap */
			
				{
					struct bytes_printer printer = BYTES_PRINTER_INIT;
					size_t readtext_bufsize      = READTEXT_INITIAL_BUFSIZE;
					while (maxbytes) {
						void *buffer;
						size_t read_size;
						size_t bufsize = MIN(maxbytes, readtext_bufsize);

						/* Allocate more buffer memory. */
						buffer = bytes_printer_alloc(&printer, bufsize);
						if unlikely(!buffer)
							goto err_printer;

						/* Read more data. */
						read_size = (*ft_read)((DeeFileObject *)self, buffer, bufsize, Dee_FILEIO_FNORMAL);
						if unlikely(read_size == (size_t)-1)
							goto err_printer;
						ASSERT(read_size <= bufsize);
						bytes_printer_release(&printer, bufsize - read_size);
						if (!read_size ||
						    (!readall && read_size < bufsize))
							break; /* EOF */
						maxbytes -= read_size;
						readtext_bufsize *= 2;
					}
/*done_printer:*/
					return bytes_printer_pack(&printer);
err_printer:
					bytes_printer_fini(&printer);
				} /* Scope... */
				goto err;
			}
		} while (type_inherit_file_read(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return_empty_bytes;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	err_unimplemented_operator(tp_self, FILE_OPERATOR_READ);
err:
	return NULL;
}

PUBLIC WUNUSED ATTR_INOUT(1) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_PReadBytes(DeeObject *__restrict self,
                   size_t maxbytes, dpos_t pos,
                   bool readall) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pread)(DeeFileObject *__restrict self, void *buffer,
			                         size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pread = DeeType_AsFileType(tp_self)->ft_pread;
			if likely(ft_pread != NULL) {
#ifdef HAVE_file_read_trymap
				/* if `ft_pread' belongs to `DeeSystemFile_Type', and `maxbytes' is larger
				 * than some threshold (>= 2*PAGESIZE), then try to create a file view using
				 * `DeeMapFile_InitSysFd()', which is then wrapped by file view holder object,
				 * which can then be wrapped by a regular `Bytes' object.
				 * -> That way, we can provide the user with O(1) reads from large files! */
				if ((maxbytes >= FILE_READ_MMAP_THRESHOLD) &&
				    (ft_pread == (size_t (DCALL *)(DeeFileObject *__restrict self, void *__restrict, size_t, dpos_t, dioflag_t flags))&sysfile_pread)) {
					DREF /*Bytes*/ DeeObject *result;
					result = file_read_trymap(DeeSystemFile_GetHandle(self),
					                          maxbytes, pos, readall);
					if (result != ITER_DONE)
						return result;
				}
				/* TODO: if `ft_read' is for a FileBuffer that is currently empty, also try to mmap! */
#endif /* HAVE_file_read_trymap */
			
				{
					struct bytes_printer printer = BYTES_PRINTER_INIT;
					size_t readtext_bufsize      = READTEXT_INITIAL_BUFSIZE;
					while (maxbytes) {
						void *buffer;
						size_t read_size;
						size_t bufsize = MIN(maxbytes, readtext_bufsize);

						/* Allocate more buffer memory. */
						buffer = bytes_printer_alloc(&printer, bufsize);
						if unlikely(!buffer)
							goto err_printer;

						/* Read more data. */
						read_size = (*ft_pread)((DeeFileObject *)self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL);
						if unlikely(read_size == (size_t)-1)
							goto err_printer;
						ASSERT(read_size <= bufsize);
						bytes_printer_release(&printer, bufsize - read_size);
						if (!read_size || (!readall && read_size < bufsize))
							break; /* EOF */
						maxbytes -= read_size;
						pos += read_size;
						readtext_bufsize *= 2;
					}
/*done_printer:*/
					return bytes_printer_pack(&printer);
err_printer:
					bytes_printer_fini(&printer);
				} /* Scope... */
				goto err;
			}
		} while (type_inherit_file_pread(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (tp_self == &DeeNone_Type) {
		return_empty_bytes;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	err_unimplemented_operator(tp_self, FILE_OPERATOR_PREAD);
err:
	return NULL;
}



PRIVATE WUNUSED ATTR_INOUT(1) int DCALL
print_sp(DeeObject *__restrict self) {
	size_t result = DeeFile_WriteAll(self, " ", sizeof(char));
	return unlikely(result == (size_t)-1) ? -1 : 0;
}

PUBLIC WUNUSED ATTR_INOUT(1) int DCALL
DeeFile_PrintNl(DeeObject *__restrict self) {
	size_t result = DeeFile_WriteAll(self, "\n", sizeof(char));
	return unlikely(result == (size_t)-1) ? -1 : 0;
}

#define print_ob_str(self, ob) \
	DeeObject_Print(ob, (dformatprinter)&DeeFile_WriteAll, self)
#define print_ob_repr(self, ob) \
	DeeObject_PrintRepr(ob, (dformatprinter)&DeeFile_WriteAll, self)


PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintObject(DeeObject *self,
                    DeeObject *ob) {
	if unlikely(print_ob_str(self, ob) < 0)
		goto err;
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintObjectSp(DeeObject *self,
                      DeeObject *ob) {
	if unlikely(print_ob_str(self, ob) < 0)
		goto err;
	return print_sp(self);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintObjectNl(DeeObject *self,
                      DeeObject *ob) {
	if unlikely(print_ob_str(self, ob) < 0)
		goto err;
	return DeeFile_PrintNl(self);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintObjectRepr(DeeObject *self,
                        DeeObject *ob) {
	if unlikely(print_ob_repr(self, ob) < 0)
		goto err;
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintObjectReprSp(DeeObject *self,
                          DeeObject *ob) {
	if unlikely(print_ob_repr(self, ob) < 0)
		goto err;
	return print_sp(self);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintObjectReprNl(DeeObject *self,
                          DeeObject *ob) {
	if unlikely(print_ob_repr(self, ob) < 0)
		goto err;
	return DeeFile_PrintNl(self);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintAll(DeeObject *self,
                 DeeObject *ob) {
	int result;
	DREF DeeObject *elem;
	bool is_first    = true;
	size_t fast_size = DeeFastSeq_GetSize(ob);
	/* Optimization for fast-sequence objects. */
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fast_size; ++i) {
			if (i != 0) {
				result = print_sp(self);
				if unlikely(result)
					goto err;
			}
			elem = DeeFastSeq_GetItem(ob, i);
			if unlikely(!elem)
				goto err_m1;
			result = DeeFile_PrintObject(self, elem);
			Dee_Decref(elem);
			if unlikely(result)
				goto err;
		}
		return 0;
	}
	if unlikely((ob = DeeObject_IterSelf(ob)) == NULL)
		goto err_m1;
	while (ITER_ISOK(elem = DeeObject_IterNext(ob))) {
		if (!is_first) {
			result = print_sp(self);
			if unlikely(result)
				goto err_elem;
		}
		result = DeeFile_PrintObject(self, elem);
		if unlikely(result)
			goto err_elem;
		Dee_Decref(elem);
		is_first = false;
		if (DeeThread_CheckInterrupt())
			goto err_ob;
	}
	Dee_Decref(ob);
	if unlikely(!elem)
		goto err_m1;
	return 0;
err_elem:
	Dee_Decref(elem);
err_ob:
	Dee_Decref(ob);
err:
	return result;
err_m1:
	result = -1;
	goto err;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintAllSp(DeeObject *self,
                   DeeObject *ob) {
	int result;
	DREF DeeObject *elem;
	size_t fast_size = DeeFastSeq_GetSize(ob);
	/* Optimization for fast-sequence objects. */
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		size_t i;
		for (i = 0; i < fast_size; ++i) {
			elem = DeeFastSeq_GetItem(ob, i);
			if unlikely(!elem)
				goto err;
			result = DeeFile_PrintObjectSp(self, elem);
			Dee_Decref(elem);
			if unlikely(result)
				goto err;
		}
		return 0;
	}
	if unlikely((ob = DeeObject_IterSelf(ob)) == NULL)
		goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(ob))) {
		result = DeeFile_PrintObjectSp(self, elem);
		Dee_Decref(elem);
		if unlikely(result) {
			Dee_Decref(ob);
			return result;
		}
		if (DeeThread_CheckInterrupt())
			goto err_ob;
	}
	if unlikely(!elem)
		goto err_ob;
	Dee_Decref(ob);
	return 0;
err_ob:
	Dee_Decref(ob);
err:
	return -1;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
DeeFile_PrintAllNl(DeeObject *self,
                   DeeObject *ob) {
	if unlikely(DeeFile_PrintAll(self, ob))
		goto err;
	return DeeFile_PrintNl(self);
err:
	return -1;
}




/* HINT: `DeeFile_Printf' is literally implemented as
 *       `DeeFormat_Printf(&DeeFile_WriteAll, self, format, ...)'
 * @return: -1: Error */
PUBLIC WUNUSED ATTR_INOUT(1) ATTR_IN(2) dssize_t
DeeFile_Printf(DeeObject *__restrict self,
               char const *__restrict format, ...) {
	va_list args;
	dssize_t result;
	va_start(args, format);
	result = DeeFile_VPrintf(self, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED ATTR_INOUT(1) ATTR_IN(2) dssize_t DCALL
DeeFile_VPrintf(DeeObject *__restrict self,
                char const *__restrict format, va_list args) {
	return DeeFormat_VPrintf((dformatprinter)&DeeFile_WriteAll, self, format, args);
}


INTDEF WUNUSED NONNULL((1)) int DCALL
type_ctor(DeeTypeObject *__restrict self);

PRIVATE WUNUSED NONNULL((1)) int DCALL
filetype_ctor(DeeFileTypeObject *__restrict self) {
	self->ft_read   = NULL;
	self->ft_write  = NULL;
	self->ft_seek   = NULL;
	self->ft_sync   = NULL;
	self->ft_trunc  = NULL;
	self->ft_close  = NULL;
	self->ft_pread  = NULL;
	self->ft_pwrite = NULL;
	self->ft_getc   = NULL;
	self->ft_ungetc = NULL;
	self->ft_putc   = NULL;
	return type_ctor(&self->ft_base);
}

PUBLIC DeeTypeObject DeeFileType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_FileType",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeType_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&filetype_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_GC(DeeFileTypeObject)
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






PRIVATE WUNUSED NONNULL((1)) int DCALL
file_init(DeeFileObject *__restrict UNUSED(self)) {
	return 0;
}


struct open_option {
	char         name[11]; /* Name. */
#define OPEN_EXFLAG_FNORMAL 0x00
#define OPEN_EXFLAG_FTEXT   0x01 /* Wrap the file in a text-file wrapper that
	                              * automatically converts its encoding to UTF-8. */
#define OPEN_EXFLAG_FNOBUF  0x02 /* Open the file without wrapping it inside a buffer. */
	uint8_t      exflg;   /* Extended flags (Set of `OPEN_EXFLAG_F*'). */
	unsigned int mask;    /* Mask of flags which, when already set, causes the format to become invalid. */
	unsigned int flag;    /* Flags. (or-ed with the flags after `mask' is checked) */
};

/* Open options are parsed from a comma-separated
 * string passed as second argument to file.open:
 * >> file.open("foo.txt", "w+");                    // STD-C mode name.
 * >> file.open("foo.txt", "text,RW,T,C");           // Extended form.
 * >> file.open("foo.txt", "text,rdwr,trunc,creat"); // Long form.
 */
#define BASEMODE_MASK (OPEN_FACCMODE | OPEN_FCREAT | OPEN_FEXCL | OPEN_FTRUNC | OPEN_FAPPEND)
PRIVATE struct open_option const open_options[] = {
#if 0
	/* STD-C compatible open modes. */
	{ "r",    OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDONLY },
	{ "r+",   OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDWR },
	{ "a",    OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FWRONLY | OPEN_FAPPEND | OPEN_FCREAT },
	{ "a+",   OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDWR | OPEN_FAPPEND | OPEN_FCREAT },
	{ "rb",   OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDONLY },
	{ "rb+",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR },
	{ "r+b",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR },
	{ "ab",   OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FWRONLY | OPEN_FAPPEND | OPEN_FCREAT },
	{ "ab+",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR | OPEN_FAPPEND | OPEN_FCREAT },
	{ "a+b",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR | OPEN_FAPPEND | OPEN_FCREAT },
	{ "w",    OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FWRONLY | OPEN_FTRUNC | OPEN_FCREAT },
	{ "w+",   OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDWR | OPEN_FTRUNC | OPEN_FCREAT },
	{ "wb",   OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FWRONLY | OPEN_FTRUNC | OPEN_FCREAT },
	{ "wb+",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR | OPEN_FTRUNC | OPEN_FCREAT },
	{ "w+b",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR | OPEN_FTRUNC | OPEN_FCREAT },
	{ "wx",   OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FWRONLY | OPEN_FTRUNC | OPEN_FCREAT | OPEN_FEXCL },
	{ "w+x",  OPEN_EXFLAG_FTEXT, BASEMODE_MASK, OPEN_FRDWR | OPEN_FTRUNC | OPEN_FCREAT | OPEN_FEXCL },
	{ "wbx",  OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FWRONLY | OPEN_FTRUNC | OPEN_FCREAT | OPEN_FEXCL },
	{ "wb+x", OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR | OPEN_FTRUNC | OPEN_FCREAT | OPEN_FEXCL },
	{ "w+bx", OPEN_EXFLAG_FNORMAL, BASEMODE_MASK, OPEN_FRDWR | OPEN_FTRUNC | OPEN_FCREAT | OPEN_FEXCL },
#endif
	/* Short flag names. */
	{ "R", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FRDONLY },
	{ "W", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FWRONLY },
	{ "RW", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FRDWR },
	{ "C", OPEN_EXFLAG_FNORMAL, OPEN_FCREAT, OPEN_FCREAT },
	{ "X", OPEN_EXFLAG_FNORMAL, OPEN_FEXCL, OPEN_FEXCL },
	{ "T", OPEN_EXFLAG_FNORMAL, OPEN_FTRUNC, OPEN_FTRUNC },
	{ "A", OPEN_EXFLAG_FNORMAL, OPEN_FAPPEND, OPEN_FAPPEND },
	{ "NB", OPEN_EXFLAG_FNORMAL, OPEN_FNONBLOCK, OPEN_FNONBLOCK },
	{ "S", OPEN_EXFLAG_FNORMAL, OPEN_FSYNC, OPEN_FSYNC },
	{ "D", OPEN_EXFLAG_FNOBUF, OPEN_FDIRECT, OPEN_FDIRECT },
	{ "NF", OPEN_EXFLAG_FNORMAL, OPEN_FNOFOLLOW, OPEN_FNOFOLLOW },
	{ "NA", OPEN_EXFLAG_FNORMAL, OPEN_FNOATIME, OPEN_FNOATIME },
	{ "CE", OPEN_EXFLAG_FNORMAL, OPEN_FCLOEXEC, OPEN_FCLOEXEC },
	{ "XR", OPEN_EXFLAG_FNORMAL, OPEN_FXREAD, OPEN_FXREAD },
	{ "XW", OPEN_EXFLAG_FNORMAL, OPEN_FXWRITE, OPEN_FXWRITE },
	{ "H", OPEN_EXFLAG_FNORMAL, OPEN_FHIDDEN, OPEN_FHIDDEN },
	/* Flags by name. */
	{ "rdonly", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FRDONLY },
	{ "wronly", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FWRONLY },
	{ "rdwr", OPEN_EXFLAG_FNORMAL, OPEN_FACCMODE, OPEN_FRDWR },
	{ "creat", OPEN_EXFLAG_FNORMAL, OPEN_FCREAT, OPEN_FCREAT },
	{ "excl", OPEN_EXFLAG_FNORMAL, OPEN_FEXCL, OPEN_FEXCL },
	{ "trunc", OPEN_EXFLAG_FNORMAL, OPEN_FTRUNC, OPEN_FTRUNC },
	{ "append", OPEN_EXFLAG_FNORMAL, OPEN_FAPPEND, OPEN_FAPPEND },
	{ "nonblock", OPEN_EXFLAG_FNORMAL, OPEN_FNONBLOCK, OPEN_FNONBLOCK },
	{ "sync", OPEN_EXFLAG_FNORMAL, OPEN_FSYNC, OPEN_FSYNC },
	{ "direct", OPEN_EXFLAG_FNOBUF, OPEN_FDIRECT, OPEN_FDIRECT },
	{ "nofollow", OPEN_EXFLAG_FNORMAL, OPEN_FNOFOLLOW, OPEN_FNOFOLLOW },
	{ "noatime", OPEN_EXFLAG_FNORMAL, OPEN_FNOATIME, OPEN_FNOATIME },
	{ "cloexec", OPEN_EXFLAG_FNORMAL, OPEN_FCLOEXEC, OPEN_FCLOEXEC },
	{ "xread", OPEN_EXFLAG_FNORMAL, OPEN_FXREAD, OPEN_FXREAD },
	{ "xwrite", OPEN_EXFLAG_FNORMAL, OPEN_FXWRITE, OPEN_FXWRITE },
	{ "hidden", OPEN_EXFLAG_FNORMAL, OPEN_FHIDDEN, OPEN_FHIDDEN },
	/* Extended flag names. */
	{ "binary", OPEN_EXFLAG_FNORMAL, 0, 0 },
	{ "text", OPEN_EXFLAG_FTEXT, 0, 0 },
	{ "nobuf", OPEN_EXFLAG_FNOBUF, 0, 0 },
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_class_open(DeeObject *UNUSED(self),
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {
	DeeObject *path;
	DREF DeeObject *result, *new_result;
	uint8_t flags;
	int oflags, mode = 0644;
	DeeObject *oflags_ob = NULL;
	PRIVATE DEFINE_KWLIST(kwlist, { K(path), K(oflags), K(mode), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|od:open", &path, &oflags_ob, &mode))
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
	if (!oflags_ob) {
		/* Default to `r' */
		flags  = OPEN_EXFLAG_FTEXT;
		oflags = OPEN_FRDONLY;
	} else if (!DeeString_Check(oflags_ob)) {
		flags = OPEN_EXFLAG_FNORMAL;
		if (DeeObject_AsInt(oflags_ob, &oflags))
			goto err;
	} else {
		char *iter;
		iter   = DeeString_STR(oflags_ob);
		flags  = OPEN_EXFLAG_FNORMAL;
		oflags = 0;
		for (;;) {
			bool open_binary;
			unsigned int i;
			size_t optlen;
			char *next = strchr(iter, ',');
			if (!next)
				next = iter + strlen(iter);
			optlen = (size_t)(next - iter);
			if (optlen < COMPILER_LENOF(open_options[0].name)) {
				for (i = 0; i < COMPILER_LENOF(open_options); ++i) {
					if (open_options[i].name[optlen])
						continue;
					if (bcmpc(open_options[i].name, iter,
					          optlen, sizeof(char)) != 0)
						continue;
					if (oflags & open_options[i].mask)
						goto err_invalid_oflags; /* Check illegal old flags. */
					/* Apply new flags. */
					flags |= open_options[i].exflg;
					oflags |= open_options[i].flag;
					goto found_option;
				}
			}
			/* Check for an STD-C conforming open mode. */
			if (!optlen)
				goto err_invalid_oflags;
			if (oflags & BASEMODE_MASK)
				goto err_invalid_oflags;
			open_binary = false;
			if (*iter == 'r') {
				oflags |= OPEN_FRDONLY;
			} else if (*iter == 'w') {
				oflags |= OPEN_FWRONLY | OPEN_FTRUNC | OPEN_FCREAT;
			} else if (*iter == 'a') {
				oflags |= OPEN_FWRONLY | OPEN_FAPPEND | OPEN_FCREAT;
			} else {
				goto err_invalid_oflags;
			}
			if (*++iter == 'b')
				++iter, open_binary = true;
			if (*iter == '+')
				++iter, oflags &= ~OPEN_FACCMODE, oflags |= OPEN_FRDWR;
			if (*iter == 'b' && !open_binary)
				++iter, open_binary = true;
			if (*iter == 'x' && (oflags & (OPEN_FTRUNC | OPEN_FCREAT)) == (OPEN_FTRUNC | OPEN_FCREAT))
				++iter, oflags |= OPEN_FEXCL;
			if (*iter == 't' && !open_binary)
				++iter; /* Accept a trailing `t', as suggested by STD-C */
			if (iter != next)
				goto err_invalid_oflags;
			if (!open_binary)
				flags |= OPEN_EXFLAG_FTEXT;
found_option:
			if (!*next)
				break;
			iter = next + 1;
		}
	}

	/* Actually open the file. */
	result = DeeFile_Open(path, oflags, mode);
	if unlikely(!ITER_ISOK(result)) {
		if unlikely(!result)
			goto err;
		/* Handle file-not-found / file-already-exists errors. */
		if ((oflags & (OPEN_FCREAT | OPEN_FEXCL)) == (OPEN_FCREAT | OPEN_FEXCL)) {
			DeeError_Throwf(&DeeError_FileExists,
			                "File %r already exists",
			                path);
		} else {
			DeeError_Throwf(&DeeError_FileNotFound,
			                "File %r could not be found",
			                path);
		}
		goto err;
	}
	if (flags & OPEN_EXFLAG_FTEXT) {
		/* TODO: Wrap the file in a text-decoder. */
	}
	if (!(flags & OPEN_EXFLAG_FNOBUF)) {
		/* Wrap the file in a buffer. */
		new_result = DeeFileBuffer_New(result,
		                               (oflags & OPEN_FACCMODE) == OPEN_FRDONLY
		                               ? (FILE_BUFFER_MODE_AUTO | FILE_BUFFER_FREADONLY)
		                               : (FILE_BUFFER_MODE_AUTO),
		                               0);
		Dee_Decref(result);
		if unlikely(!new_result)
			goto err;
		result = new_result;
	}
	return result;
err_invalid_oflags:
	DeeError_Throwf(&DeeError_ValueError,
	                "Invalid open mode %r",
	                oflags_ob);
err:
	return NULL;
}


PRIVATE struct type_method tpconst file_class_methods[] = {
	TYPE_KWMETHOD("open", &file_class_open,
	              "(path:?Dstring,oflags=!Pr,mode=!0644)->?.\n"
	              "(path:?Dstring,oflags:?Dint,mode=!0644)->?.\n"
	              "#t{:Interrupt}"
	              "#tFileExists{The passed @oflags contains both $\"creat\" and "
	              /*             */ "$\"excl\", but the given @path already existed}"
	              "#tFileNotFound{The given @path could not be found}"
	              "#tFileAccessError{The current user does not have permissions to "
	              /*                  */ "access the given @path in the requested manner}"
	              "#tReadOnlyFile{Write-access, or create-file was requested, but the "
	              /*               */ "filesystem hosting @path is mounted as read-only}"
	              "#tUnsupportedAPI{Filesystem access has been disabled, or $\"creat\" was passed and the "
	              /*                 */ "filesystem hosting @path does not support the creation of new files}"
	              "#tFSError{Failed to open the given @path for some reason}"
	              "#pmode{The unix-like permissions to be set for newly created files}"

	              "Consults the filesystem to open or create a given @path, using the given @oflags\n"

	              "Mode is implemented as a comma-separated list of open options that include "
	              /**/ "those described by the C standard, allowing the basic $\"r\", $\"w\", $\"a\" "
	              /**/ "as well as each followed by an additional $\"+\", also supporting the "
	              /**/ "$\"b\" and $\"x\" modifiers, as well as the suggested $\"t\" flag\n"

	              "In addition to this, any of the comma-separated options can be one of the following "
	              /**/ "strings to better fine-tune the exact open-behavior, if supported by the host:\n"

	              "#T{Flag String|Description~"
	              /**/ "$\"rdonly\", $\"R\"|Open for read-only (default)&"
	              /**/ "$\"wronly\", $\"W\"|Open for write access only&"
	              /**/ "$\"rdwr\", $\"RW\"|Open for both reading and writing&"
	              /**/ "$\"creat\", $\"C\"|Create the file if it doesn't exist already&"
	              /**/ "$\"excl\", $\"X\"|When used with $\"creat\", fail if the file already exists&"
	              /**/ "$\"trunc\", $\"T\"|Truncate an existing file to a length of $0 before opening it&"
	              /**/ "$\"append\", $\"A\"|Write operations always append data to the end of the file"
	              "}\n"

	              "Additionally, the following flags are accepted, but ignored if the host doesn't support them:\n"
	              "#T{Flag String|Description~"
	              /**/ "$\"nonblock\", $\"NB\"|Don't block when attempting to read/write&"
	              /**/ "$\"sync\", $\"S\"|Write operations block until all data has been written to disk&"
	              /**/ "$\"direct\", $\"D\"|Bypass system buffers and directly pass data to the kernel when writing&"
	              /**/ "$\"nofollow\", $\"NF\"|Do not follow symbolic links&"
	              /**/ "$\"noatime\", $\"NA\"|Do not update access times&"
	              /**/ "$\"cloexec\", $\"CE\"|Do not inherit the file in child processes&"
	              /**/ "$\"xread\", $\"XR\"|Request exclusive read access&"
	              /**/ "$\"xwrite\", $\"XW\"|Request exclusive write access&"
	              /**/ "$\"hidden\", $\"H\"|Set a host-specific hidden-file flag when creating a new file (if the host uses a flag to track this attribute)"
	              "}\n"

	              "The following flags may be passed to modify buffering behavior:\n"
	              "#T{Flag String|Description~"
	              /**/ "$\"binary\"|Open the file in binary mode (default, unless an STD-C modifier is used, in which case it that must contain the $\"b\" flag)&"
	              /**/ "$\"text\"|Open the file in text mode (default if an STD-C modifier was used)&"
	              /**/ "$\"nobuf\"|Do not wrap the returned file in a buffer (Also implied when $\"direct\" is passed)"
	              "}\n"

	              "Not that unlike in many other places, case is NOT ignored for these options\n"

	              "In addition to the string-based options for @oflags, an integer bit-set may be passed "
	              /**/ "consisting of #C{Dee_OPEN_F*} flags that can be found in deemon's system headers. "
	              /**/ "Note that these flags may not necessarily equal the #C{O_*} flags from ?Mposix"),
	TYPE_METHOD_END
};


#ifdef DEE_STDDBG_IS_UNIQUE
#define DEE_STDCNT 4
PRIVATE DREF DeeObject *dee_std[DEE_STDCNT] = { ITER_DONE, ITER_DONE, ITER_DONE, ITER_DONE };
#else /* DEE_STDDBG_IS_UNIQUE */
#define DEE_STDCNT 3
PRIVATE DREF DeeObject *dee_std[DEE_STDCNT] = { ITER_DONE, ITER_DONE, ITER_DONE };
#endif /* !DEE_STDDBG_IS_UNIQUE */

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t dee_std_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define dee_std_lock_reading()    Dee_atomic_rwlock_reading(&dee_std_lock)
#define dee_std_lock_writing()    Dee_atomic_rwlock_writing(&dee_std_lock)
#define dee_std_lock_tryread()    Dee_atomic_rwlock_tryread(&dee_std_lock)
#define dee_std_lock_trywrite()   Dee_atomic_rwlock_trywrite(&dee_std_lock)
#define dee_std_lock_canread()    Dee_atomic_rwlock_canread(&dee_std_lock)
#define dee_std_lock_canwrite()   Dee_atomic_rwlock_canwrite(&dee_std_lock)
#define dee_std_lock_waitread()   Dee_atomic_rwlock_waitread(&dee_std_lock)
#define dee_std_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&dee_std_lock)
#define dee_std_lock_read()       Dee_atomic_rwlock_read(&dee_std_lock)
#define dee_std_lock_write()      Dee_atomic_rwlock_write(&dee_std_lock)
#define dee_std_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&dee_std_lock)
#define dee_std_lock_upgrade()    Dee_atomic_rwlock_upgrade(&dee_std_lock)
#define dee_std_lock_downgrade()  Dee_atomic_rwlock_downgrade(&dee_std_lock)
#define dee_std_lock_endwrite()   Dee_atomic_rwlock_endwrite(&dee_std_lock)
#define dee_std_lock_endread()    Dee_atomic_rwlock_endread(&dee_std_lock)
#define dee_std_lock_end()        Dee_atomic_rwlock_end(&dee_std_lock)

PRIVATE uint16_t const std_buffer_modes[DEE_STDCNT] = {
	/* [DEE_STDIN ] = */ FILE_BUFFER_MODE_AUTO | FILE_BUFFER_FREADONLY,
	/* [DEE_STDOUT] = */ FILE_BUFFER_MODE_AUTO,
	/* [DEE_STDERR] = */ FILE_BUFFER_MODE_AUTO
#ifdef DEE_STDDBG_IS_UNIQUE
	,
	/* [DEE_STDDBG] = */ FILE_BUFFER_MODE_AUTO
#endif /* DEE_STDDBG_IS_UNIQUE */
};

PRIVATE WUNUSED DREF DeeObject *DCALL
create_std_buffer(unsigned int id) {
	DREF DeeObject *result, *new_result;
	ASSERT(id < DEE_STDCNT);
	/* Create a buffer for the standard stream. */
#ifdef CONFIG_NATIVE_STD_FILES_ARE_BUFFERED
	/* If the native STD files are already buffered, there'd
	 * be no point in us adding our own buffer into the mix. */
	result = DeeFile_DefaultStd(id);
	Dee_Incref(result);
#else /* CONFIG_NATIVE_STD_FILES_ARE_BUFFERED */
	result = DeeFileBuffer_New(DeeFile_DefaultStd(id),
	                           std_buffer_modes[id], 0);
	if unlikely(!result)
		goto done;
#endif /* !CONFIG_NATIVE_STD_FILES_ARE_BUFFERED */
	dee_std_lock_write();
	/* Save the newly created buffer in the standard stream vector. */
	new_result = dee_std[id];
	if unlikely(new_result != ITER_DONE) {
		Dee_XIncref(new_result);
		dee_std_lock_endwrite();
		Dee_Decref(result);
		result = new_result;
		if (!result) {
			DeeError_Throwf(&DeeError_UnboundAttribute,
			                "Unbound standard stream");
		}
		goto done;
	}
	Dee_Incref(result);
	dee_std[id] = result;
	dee_std_lock_endwrite();
done:
	return result;
}


/* Return a file stream for a standard file number `id'.
 * @param: id:   One of `DEE_STD*' (Except `DEE_STDDBG')
 * @param: file: The file to use, or `NULL' to unbind that stream.
 * `DeeFile_GetStd()' will throw an `UnboundLocal' error if the stream isn't assigned. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeFile_GetStd(unsigned int id) {
	DREF DeeObject *result;
	ASSERT(id < DEE_STDCNT);
	dee_std_lock_read();
	result = dee_std[id];
	if unlikely(!ITER_ISOK(result)) {
		dee_std_lock_endread();
		/* When the stream is `ITER_DONE', lazily create the STD stream. */
		if (result == ITER_DONE)
			return create_std_buffer(id);
		DeeError_Throwf(&DeeError_UnboundAttribute,
		                "Unbound standard stream");
		goto done;
	}
	Dee_Incref(result);
	dee_std_lock_endread();
done:
	return result;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeFile_TryGetStd(unsigned int id) {
	DREF DeeObject *result;
	ASSERT(id < DEE_STDCNT);
	dee_std_lock_read();
	result = dee_std[id];
	if unlikely(!ITER_ISOK(result)) {
		dee_std_lock_endread();
		/* When the stream is `ITER_DONE', lazily create the STD stream. */
		if (result == ITER_DONE) {
			result = create_std_buffer(id);
			if unlikely(!result)
				DeeError_Handled(ERROR_HANDLED_RESTORE);
		}
		goto done;
	}
	Dee_Incref(result);
	dee_std_lock_endread();
done:
	return result;
}

/* Returns the old stream, `NULL' when none was assigned,
 * or `ITER_DONE' when it hadn't been allocated yet */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeFile_SetStd(unsigned int id, DeeObject *file) {
	DREF DeeObject *old_stream;
	ASSERT(id < DEE_STDCNT);
	if (ITER_ISOK(file)) {
		ASSERT_OBJECT(file);
		Dee_Incref(file);
	}
	dee_std_lock_write();
	/* Set the given stream. */
	old_stream  = dee_std[id];
	dee_std[id] = file;
	dee_std_lock_endwrite();
	return old_stream;
}

/* [0..1][lock(WRITE_ONCE)] The `files' module. */
PRIVATE DREF DeeObject *files_module = NULL;
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t files_module_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define files_module_lock_reading()    Dee_atomic_rwlock_reading(&files_module_lock)
#define files_module_lock_writing()    Dee_atomic_rwlock_writing(&files_module_lock)
#define files_module_lock_tryread()    Dee_atomic_rwlock_tryread(&files_module_lock)
#define files_module_lock_trywrite()   Dee_atomic_rwlock_trywrite(&files_module_lock)
#define files_module_lock_canread()    Dee_atomic_rwlock_canread(&files_module_lock)
#define files_module_lock_canwrite()   Dee_atomic_rwlock_canwrite(&files_module_lock)
#define files_module_lock_waitread()   Dee_atomic_rwlock_waitread(&files_module_lock)
#define files_module_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&files_module_lock)
#define files_module_lock_read()       Dee_atomic_rwlock_read(&files_module_lock)
#define files_module_lock_write()      Dee_atomic_rwlock_write(&files_module_lock)
#define files_module_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&files_module_lock)
#define files_module_lock_upgrade()    Dee_atomic_rwlock_upgrade(&files_module_lock)
#define files_module_lock_downgrade()  Dee_atomic_rwlock_downgrade(&files_module_lock)
#define files_module_lock_endwrite()   Dee_atomic_rwlock_endwrite(&files_module_lock)
#define files_module_lock_endread()    Dee_atomic_rwlock_endread(&files_module_lock)
#define files_module_lock_end()        Dee_atomic_rwlock_end(&files_module_lock)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
get_files_object(DeeObject *__restrict name) {
	DREF DeeObject *result, *mod;
again:
	files_module_lock_read();
	mod = files_module;
	if unlikely(!mod) {
		files_module_lock_endread();
		mod = DeeModule_OpenGlobal((DeeObject *)&str_files, NULL, true);
		if unlikely(!mod)
			goto err;
		if unlikely(DeeModule_RunInit(mod) < 0)
			goto err_mod;
		files_module_lock_write();
		if unlikely(atomic_read(&files_module)) {
			files_module_lock_endwrite();
			Dee_Decref(mod);
			goto again;
		}
		Dee_Incref(mod);
		files_module = mod;
		files_module_lock_endwrite();
	} else {
		Dee_Incref(mod);
		files_module_lock_endread();
	}
	result = DeeObject_GetAttr(mod, name);
	Dee_Decref(mod);
	return result;
err_mod:
	Dee_Decref(mod);
err:
	return NULL;
}

PRIVATE bool DCALL clear_files_module(void) {
	DREF DeeObject *mod;
	files_module_lock_write();
	mod          = files_module;
	files_module = NULL;
	files_module_lock_endwrite();
	Dee_XDecref(mod);
	return mod != NULL;
}

/* Reset all standard stream (called during the cleanup phase prior to shutdown)
 * @return: true:  A non-default file had been assigned to at
 *                 least one of the known standard streams.
 * @return: false: All streams had already been reset. */
PUBLIC bool DCALL DeeFile_ResetStd(void) {
	bool result     = clear_files_module();
	unsigned int id = 0;
	/* Set the default stream for all standard streams. */
	do {
		DREF DeeObject *old_stream, *default_stream;
		default_stream = DeeFile_DefaultStd(id);
		old_stream     = DeeFile_SetStd(id, default_stream);
		if (ITER_ISOK(old_stream))
			Dee_Decref(old_stream);
		if (old_stream != default_stream)
			result = true;
		++id;
	} while (id != DEE_STDCNT);
	return result;
}

#define DEFINE_FILE_CLASS_STD_FUNCTIONS(stdxxx, DEE_STDXXX)       \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL            \
	file_class_get_##stdxxx(DeeObject *__restrict UNUSED(self)) { \
		return DeeFile_GetStd(DEE_STDXXX);                        \
	}                                                             \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                        \
	file_class_del_##stdxxx(DeeObject *__restrict self) {         \
		DREF DeeObject *old_stream;                               \
		old_stream = DeeFile_SetStd(DEE_STDXXX, NULL);            \
		if unlikely(!old_stream)                                  \
			goto err_unbound;                                     \
		return 0;                                                 \
	err_unbound:                                                  \
		err_unbound_attribute_string(Dee_TYPE(self), #stdxxx);    \
		return -1;                                                \
	}                                                             \
	PRIVATE WUNUSED NONNULL((1, 2)) int DCALL                     \
	file_class_set_##stdxxx(DeeObject *UNUSED(self),              \
	                        DeeObject *value) {                   \
		DREF DeeObject *old_stream;                               \
		old_stream = DeeFile_SetStd(DEE_STDXXX, value);           \
		if (ITER_ISOK(old_stream))                                \
			Dee_Decref(old_stream);                               \
		return 0;                                                 \
	}
DEFINE_FILE_CLASS_STD_FUNCTIONS(stdin, DEE_STDIN)
DEFINE_FILE_CLASS_STD_FUNCTIONS(stdout, DEE_STDOUT)
DEFINE_FILE_CLASS_STD_FUNCTIONS(stderr, DEE_STDERR)
#undef DEFINE_FILE_CLASS_STD_FUNCTIONS

#if DEE_STDDBG != DEE_STDERR
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_class_stddbg(DeeObject *__restrict UNUSED(self)) {
	return_reference(DeeFile_DefaultStd(DEE_STDDBG));
}
#endif /* DEE_STDDBG != DEE_STDERR */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_class_default_stdin(DeeObject *__restrict UNUSED(self)) {
	return_reference(DeeFile_DefaultStd(DEE_STDIN));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_class_default_stdout(DeeObject *__restrict UNUSED(self)) {
	return_reference(DeeFile_DefaultStd(DEE_STDOUT));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_class_default_stderr(DeeObject *__restrict UNUSED(self)) {
	return_reference(DeeFile_DefaultStd(DEE_STDERR));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_class_getjoined(DeeObject *__restrict UNUSED(self)) {
	return get_files_object((DeeObject *)&str_Joined);
}

PRIVATE struct type_getset tpconst file_class_getsets[] = {
	TYPE_GETSET("stdin",
	            &file_class_get_stdin,
	            &file_class_del_stdin,
	            &file_class_set_stdin,
	            "->?DFile\n"
	            "The standard input stream"),
	TYPE_GETSET("stdout",
	            &file_class_get_stdout,
	            &file_class_del_stdout,
	            &file_class_set_stdout,
	            "->?DFile\n"
	            "The standard output stream\n"
	            "This is also what $print statements will write to when used "
	            /**/ "without an explicit file target:\n"
	            "${"
	            /**/ "print \"foo\";\n"
	            /**/ "// Same as:\n"
	            /**/ "import File from deemon;\n"
	            /**/ "print File.stdout: \"foo\";"
	            "}"),
	TYPE_GETSET("stderr",
	            &file_class_get_stderr,
	            &file_class_del_stderr,
	            &file_class_set_stderr,
	            "->?DFile\n"
	            "The standard error stream"),
	TYPE_GETTER("default_stdin",
	            &file_class_default_stdin,
	            "->?DFile\n"
	            "The default standard input stream"),
	TYPE_GETTER("default_stdout",
	            &file_class_default_stdout,
	            "->?DFile\n"
	            "The default standard output stream"),
	TYPE_GETTER("default_stderr",
	            &file_class_default_stderr,
	            "->?DFile\n"
	            "The default standard error stream"),
#if DEE_STDDBG == DEE_STDERR
#define LINKED_P_file_class_stddbg &file_class_default_stderr
#else /* DEE_STDDBG == DEE_STDERR */
#define LINKED_P_file_class_stddbg &file_class_stddbg
#endif /* DEE_STDDBG != DEE_STDERR */
	TYPE_GETTER("stddbg",
	            LINKED_P_file_class_stddbg,
	            "->?DFile\n"
	            "A standard stream that usually simply aliases the "
	            /**/ "default #stderr, but should be used for debug-output\n"

	            "Note that unlike the other streams, this one can't be redirected"),
	TYPE_GETTER(STR_Joined, &file_class_getjoined,
	            "->?DType\n"
	            "Deprecated alias for ?Efiles:Joined"),
	TYPE_GETSET_END
};


#if SEEK_SET == 0
#define file_SEEK_SET (*DeeInt_Zero)
#else /* SEEK_SET == 0 */
PRIVATE DEFINE_UINT32(file_SEEK_SET, SEEK_SET);
#endif /* SEEK_SET != 0 */
#if SEEK_CUR == 1
#define file_SEEK_CUR (*DeeInt_One)
#else /* SEEK_CUR == 1 */
PRIVATE DEFINE_UINT32(file_SEEK_CUR, SEEK_CUR);
#endif /* SEEK_CUR != 1 */
#if SEEK_END <= ((1 << 15) - 1)
PRIVATE DEFINE_UINT15(file_SEEK_END, SEEK_END);
#else /* SEEK_END <= ((1 << 15) - 1) */
PRIVATE DEFINE_UINT32(file_SEEK_END, SEEK_END);
#endif /* SEEK_END > ((1 << 15) - 1) */


PRIVATE struct type_member tpconst file_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, (DeeObject *)&DeeFile_Type),
	TYPE_MEMBER_CONST("Reader", (DeeObject *)&DeeFileReader_Type),
	TYPE_MEMBER_CONST("Writer", (DeeObject *)&DeeFileWriter_Type),
	TYPE_MEMBER_CONST("Buffer", (DeeObject *)&DeeFileBuffer_Type),
	TYPE_MEMBER_CONST("System", (DeeObject *)&DeeSystemFile_Type),
	TYPE_MEMBER_CONST_DOC("io", (DeeObject *)&DeeFile_Type,
	                      "Deprecated alias for backwards-compatible access to "
	                      /**/ "std-streams that used to be located in ${File.io.stdxxx}\n"
	                      /**/ "Starting with deemon v200, these streams can now be found "
	                      /**/ "under ${File.stdxxx} and the ${File.io} type has been "
	                      /**/ "renamed to ?#System\n"
	                      /**/ "With that in mind, this field is now simply an alias for ?DFile"),
	TYPE_MEMBER_CONST_DOC("SEEK_SET", (DeeObject *)&file_SEEK_SET,
	                      "Deprecated argument for ?#seek (Use the string $\"set\" instead)"),
	TYPE_MEMBER_CONST_DOC("SEEK_CUR", (DeeObject *)&file_SEEK_CUR,
	                      "Deprecated argument for ?#seek (Use the string $\"cur\" instead)"),
	TYPE_MEMBER_CONST_DOC("SEEK_END", (DeeObject *)&file_SEEK_END,
	                      "Deprecated argument for ?#seek (Use the string $\"end\" instead)"),
	TYPE_MEMBER_END
};

/* Returns the total size of a given file stream.
 * If the file doesn't support retrieval of its
 * size, a NotImplemented error is thrown.
 * NOTE: This function is equivalent to calling a member function `size()',
 *       which file objects default-implement by temporarily seeking to the
 *       end of the file and determining where that position is located at.
 * @return: * : The size of the given file `self' in bytes.
 * @return: -1: An error occurred. */
PUBLIC WUNUSED NONNULL((1)) dpos_t DCALL
DeeFile_GetSize(DeeObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_size, 0, NULL);
	if likely(result) {
		dpos_t resval;
		int error;
		error = DeeObject_AsUInt64(result, &resval);
		Dee_Decref(result);
		if unlikely(error)
			goto err;

		/* Ensure that the file isn't too large. */
		if unlikely(resval == (dpos_t)-1) {
			DeeError_Throwf(&DeeError_ValueError,
			                "Failed %k is too large (%" PRFu64 " is bigger than 2^63 bytes)",
			                self, resval);
			goto err;
		}
		return resval;
	}

	/* Failed to call the size() member function. */
	if (DeeFileType_CheckExact(Dee_TYPE(self)) &&
	    DeeError_Catch(&DeeError_AttributeError)) {
		/* Translate missing size() attributes to doesnt-implement-seek errors. */
		err_unimplemented_operator(Dee_TYPE(self),
		                           FILE_OPERATOR_SEEK);
	}
err:
	return (dpos_t)-1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_read(DeeObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	size_t maxbytes = (size_t)-1;
	bool readall    = false;
	PRIVATE DEFINE_KWLIST(kwlist, { K(maxbytes), K(readall), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|" UNPdSIZ "b:read", &maxbytes, &readall))
		goto err;
	return DeeFile_ReadBytes(self, maxbytes, readall);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_readinto(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeBuffer buffer;
	size_t result;
	DeeObject *dst;
	bool readall = false;
	PRIVATE DEFINE_KWLIST(kwlist, { K(dst), K(readall), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|b:readinto", &dst, &readall))
		goto err;
	if (DeeObject_GetBuf(dst, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
	result = readall
	         ? DeeFile_ReadAll(self, buffer.bb_base, buffer.bb_size)
	         : DeeFile_Read(self, buffer.bb_base, buffer.bb_size);
	DeeObject_PutBuf(dst, &buffer, Dee_BUFFER_FWRITABLE);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_write(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	DeeBuffer buffer;
	DeeObject *data;
	bool writeall = true;
	size_t result;
	PRIVATE DEFINE_KWLIST(kwlist, { K(data), K(writeall), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|b:write", &data, &writeall))
		goto err;
	if (DeeObject_GetBuf(data, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = writeall
	         ? DeeFile_WriteAll(self, buffer.bb_base, buffer.bb_size)
	         : DeeFile_Write(self, buffer.bb_base, buffer.bb_size);
	DeeObject_PutBuf(data, &buffer, Dee_BUFFER_FREADONLY);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_pread(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	dpos_t file_pos;
	size_t maxbytes = (size_t)-1;
	bool readall      = false;
	PRIVATE DEFINE_KWLIST(kwlist, { K(pos), K(maxbytes), K(readall), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    UNPuN(DEE_SIZEOF_DEE_POS_T) "|" UNPdSIZ "b:pread",
	                    &file_pos, &maxbytes, &readall))
		goto err;
	return DeeFile_PReadBytes(self, maxbytes, file_pos, readall);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_preadinto(DeeObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DeeBuffer buffer;
	size_t result;
	DeeObject *data;
	dpos_t file_pos;
	bool readall = false;
	PRIVATE DEFINE_KWLIST(kwlist, { K(data), K(pos), K(readall), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    "o" UNPuN(DEE_SIZEOF_DEE_POS_T) "|b:preadinto",
	                    &data, &file_pos, &readall))
		goto err;
	if (DeeObject_GetBuf(data, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
	result = readall ? DeeFile_PReadAll(self, buffer.bb_base, buffer.bb_size, file_pos)
	                 : DeeFile_PRead(self, buffer.bb_base, buffer.bb_size, file_pos);
	DeeObject_PutBuf(data, &buffer, Dee_BUFFER_FWRITABLE);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_pwrite(DeeObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeBuffer buffer;
	DeeObject *data;
	dpos_t file_pos;
	bool writeall = true;
	size_t result;
	PRIVATE DEFINE_KWLIST(kwlist, { K(data), K(pos), K(writeall), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    "o" UNPuN(DEE_SIZEOF_DEE_POS_T) "|b:pwrite",
	                    &data, &file_pos, &writeall))
		goto err;
	if (DeeObject_GetBuf(data, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = writeall ? DeeFile_PWriteAll(self, buffer.bb_base, buffer.bb_size, file_pos)
	                  : DeeFile_PWrite(self, buffer.bb_base, buffer.bb_size, file_pos);
	DeeObject_PutBuf(data, &buffer, Dee_BUFFER_FREADONLY);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

struct whence_name {
	char name[4];
	int id;
};

PRIVATE struct whence_name whence_names[] = {
	{ "set", SEEK_SET },
	{ "cur", SEEK_CUR },
	{ "end", SEEK_END },
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_seek(DeeObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	DeeObject *whence_ob = NULL;
	doff_t seek_off;
	dpos_t result;
	int whence = SEEK_SET;
	PRIVATE DEFINE_KWLIST(kwlist, { K(off), K(whence), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    UNPdN(DEE_SIZEOF_DEE_POS_T) "|o:seek",
	                    &seek_off, &whence_ob))
		goto err;
	if (whence_ob) {
		if (DeeString_Check(whence_ob)) {
			char const *name = DeeString_STR(whence_ob);
			size_t length    = DeeString_SIZE(whence_ob);
			if (length >= 5 && MEMCASEEQ(name, "SEEK_", 5 * sizeof(char))) {
				name += 5;
				length -= 5;
			}
			if (length == 3) {
				char buf[4];
				/* Convert the given mode name to lower-case. */
				buf[0] = (char)DeeUni_ToLower(name[0]);
				buf[1] = (char)DeeUni_ToLower(name[1]);
				buf[2] = (char)DeeUni_ToLower(name[2]);
				buf[3] = '\0';
				for (whence = 0; (unsigned int)whence < COMPILER_LENOF(whence_names); ++whence) {
					if (UNALIGNED_GET32(whence_names[(unsigned int)whence].name) != UNALIGNED_GET32(buf))
						continue;
					whence = whence_names[(unsigned int)whence].id;
					goto got_whence;
				}
			}
			DeeError_Throwf(&DeeError_ValueError,
			                "Unknown whence mode %r for seek",
			                whence_ob);
			goto err;
		}

		/* Fallback: Convert the whence-object to an integer. */
		if (DeeObject_AsInt(whence_ob, &whence))
			goto err;
	}
got_whence:
	result = DeeFile_Seek(self, seek_off, whence);
	if unlikely(result == (dpos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_tell(DeeObject *self, size_t argc, DeeObject *const *argv) {
	dpos_t result;
	if (DeeArg_Unpack(argc, argv, ":tell"))
		goto err;
	result = DeeFile_Tell(self);
	if unlikely(result == (dpos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_rewind(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":rewind"))
		goto err;
	if (DeeFile_Rewind(self) == (dpos_t)-1)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_trunc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	dpos_t trunc_pos;
	if (argc == 0) {
		/* Truncate at the current position. */
		if (DeeFile_TruncHere(self, &trunc_pos))
			goto err;
	} else {
		/* Truncate at the current position. */
		if (DeeArg_Unpack(argc, argv, "|" UNPuN(DEE_SIZEOF_DEE_POS_T) ":trunc", &trunc_pos))
			goto err;
		if (DeeFile_Trunc(self, trunc_pos))
			goto err;
	}

	/* Return the position where we've truncated the file. */
	return DeeInt_NewUInt64(trunc_pos);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_sync(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":sync"))
		goto err;
	if (DeeFile_Sync(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_close(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":close"))
		goto err;
	if (DeeFile_Close(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":getc"))
		goto err;
	result = DeeFile_Getc(self);
	if unlikely(result == GETC_ERR)
		goto err;
#if GETC_EOF != -1
	if (result == GETC_EOF)
		result = -1;
#endif /* GETC_EOF != -1 */
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_ungetc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, "d:ungetc", &result))
		goto err;
	result = DeeFile_Ungetc(self, result);
	if unlikely(result == GETC_ERR)
		goto err;
	return_bool_(result != GETC_EOF);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_putc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	byte_t byte;
	if (DeeArg_Unpack(argc, argv, UNPuB ":putc", &byte))
		goto err;
	result = DeeFile_Putc(self, (int)(unsigned int)byte);
	if unlikely(result == GETC_ERR)
		goto err;
	return_bool_(result != GETC_EOF);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getutf8(DeeObject *self, size_t argc, DeeObject *const *argv) {
	uint32_t result;
	if (DeeArg_Unpack(argc, argv, ":getutf8"))
		goto err;
	result = DeeFile_GetUtf8(self);
	if unlikely(result == (uint32_t)GETC_ERR)
		goto err;
	if (result == (uint32_t)GETC_EOF)
		return_empty_string;
	return DeeString_Chr(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_ungetutf8(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	uint32_t ch;
	if (DeeArg_Unpack(argc, argv, UNPu32 ":ungetutf8", &ch))
		goto err;
	result = DeeFile_UngetUtf8(self, ch);
	if unlikely(result == GETC_ERR)
		goto err;
	return_bool_(result != GETC_EOF);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_pututf8(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *str;
	char const *utf8;
	size_t written;
	if (DeeArg_Unpack(argc, argv, "o:pututf8", &str))
		goto err;
	if (DeeObject_AssertTypeExact(str, &DeeString_Type))
		goto err;
	utf8 = DeeString_AsUtf8(str);
	if unlikely(!utf8)
		goto err;
	written = DeeFile_Write(self, utf8, WSTR_LENGTH(utf8));
	if unlikely(written == (size_t)-1)
		goto err;
	ASSERT(written <= WSTR_LENGTH(utf8));
	return_bool_(written >= WSTR_LENGTH(utf8));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_size(DeeObject *self, size_t argc, DeeObject *const *argv) {
	dpos_t old_pos, result;
	if (DeeArg_Unpack(argc, argv, ":size"))
		goto err;
	old_pos = DeeFile_Seek(self, 0, SEEK_CUR);
	if unlikely(old_pos == (dpos_t)-1)
		goto err;
	result = DeeFile_Seek(self, 0, SEEK_END);
	if unlikely(result == (dpos_t)-1)
		goto err;
	if (DeeFile_Seek(self, old_pos, SEEK_SET) == (Dee_pos_t)-1)
		goto err;

	/* Return the size of the file. */
	return DeeInt_NewUInt64((uint64_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_readline(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	size_t maxbytes = (size_t)-1;
	bool keeplf     = true;
	if (argc == 1 && DeeBool_Check(argv[0])) {
		keeplf = DeeBool_IsTrue(argv[0]);
	} else {
		if (DeeArg_Unpack(argc, argv, "|" UNPdSIZ "b:readline", &maxbytes, &keeplf))
			goto err;
	}
	result = DeeFile_ReadLine(self, maxbytes, keeplf);
	if (result == ITER_DONE) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_readall(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t maxbytes = (size_t)-1;
	if (DeeArg_Unpack(argc, argv, "|" UNPdSIZ ":readall", &maxbytes))
		goto err;
	return DeeFile_ReadBytes(self, maxbytes, true);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_readallat(DeeObject *self, size_t argc, DeeObject *const *argv) {
	dpos_t file_pos;
	size_t maxbytes = (size_t)-1;
	if (DeeArg_Unpack(argc, argv,
	                  UNPuN(DEE_SIZEOF_DEE_POS_T) "|" UNPdSIZ ":readallat",
	                  &file_pos, &maxbytes))
		goto err;
	return DeeFile_PReadBytes(self, maxbytes, file_pos, true);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_mmap(DeeObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeMapFileObject *mapob;
	size_t minbytes = (size_t)0;
	size_t maxbytes = (size_t)-1;
	dpos_t offset   = (dpos_t)-1;
	size_t nulbytes = 0;
	bool readall    = false;
	bool mustmmap   = false;
	bool mapshared  = false;
	unsigned int mapflags;
	PRIVATE DEFINE_KWLIST(kwlist, { K(minbytes), K(maxbytes), K(offset), K(nulbytes), K(readall), K(mustmmap), K(mapshared), KEND });
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist,
	                    "|" UNPdSIZ UNPdSIZ UNPuN(DEE_SIZEOF_DEE_POS_T) UNPdSIZ "bbb:mapfile",
	                    &minbytes, &maxbytes, &offset, &nulbytes, &readall, &mustmmap, &mapshared))
		goto err;
	mapflags = 0;

	/* Package flags */
	if (readall)
		mapflags |= DEE_MAPFILE_F_READALL;
	if (mustmmap)
		mapflags |= DEE_MAPFILE_F_MUSTMMAP;
	if (mapshared)
		mapflags |= DEE_MAPFILE_F_MUSTMMAP | DEE_MAPFILE_F_MAPSHARED;
	mapob = DeeObject_MALLOC(DREF DeeMapFileObject);
	if unlikely(!mapob)
		goto err;

	/* Create the actual file mapping */
	if unlikely(DeeMapFile_InitFile(&mapob->mf_map, self, offset,
	                                minbytes, maxbytes, nulbytes,
	                                mapflags))
		goto err_r;
	mapob->mf_rsize = DeeMapFile_GetSize(&mapob->mf_map) + nulbytes;
	DeeObject_Init(mapob, &DeeMapFile_Type);
	result = DeeBytes_NewView((DeeObject *)mapob,
	                          (void *)DeeMapFile_GetBase(&mapob->mf_map),
	                          mapob->mf_rsize, Dee_BUFFER_FWRITABLE);
	Dee_Decref_unlikely(mapob);
	return result;
err_r:
	DeeObject_FREE(mapob);
err:
	return NULL;
}



PRIVATE struct type_method tpconst file_methods[] = {
	TYPE_KWMETHOD("read", &file_read,
	              "(maxbytes=!-1,readall=!f)->?DBytes\n"
	              "Read and return at most @maxbytes of data from the file stream. "
	              /**/ "When @readall is ?t, keep on reading data until the buffer is full, or the "
	              /**/ "read-callback returns $0, rather than until it returns something other than "
	              /**/ "the internal buffer size used when reading data."),
	TYPE_KWMETHOD("readinto", &file_readinto,
	              "(dst:?DBytes,readall=!f)->?Dint\n"
	              "Read data into the given buffer @dst and return the number of bytes read. "
	              /**/ "When @readall is ?t, keep on reading data until the buffer is full, or the "
	              /**/ "read-callback returns $0, rather than until it returns something other than "
	              /**/ "the requested read size."),
	TYPE_KWMETHOD("write", &file_write,
	              "(data:?DBytes,writeall=!t)->?Dint\n"
	              "Write @data to the file stream and return the actual number of bytes written. "
	              /**/ "When @writeall is ?t, keep writing data until the write-callback "
	              /**/ "returns $0 or until all data has been written, rather than invoke "
	              /**/ "the write-callback only a single time."),
	TYPE_KWMETHOD("pread", &file_pread,
	              "(pos:?Dint,maxbytes=!-1,readall=!f)->?DBytes\n"
	              "Similar to ?#read, but read data from a given file-offset "
	              /**/ "@pos, rather than from the current file position"),
	TYPE_KWMETHOD("preadinto", &file_preadinto,
	              "(dst:?DBytes,pos:?Dint,readall=!f)->?DBytes\n"
	              "Similar to ?#readinto, but read data from a given file-offset "
	              /**/ "@pos, rather than from the current file position"),
	TYPE_KWMETHOD("pwrite", &file_pwrite,
	              "(data:?DBytes,pos:?Dint,writeall=!t)->?Dint\n"
	              "Similar to ?#write, but write data to a given file-offset "
	              /**/ "@pos, rather than at the current file position"),
	TYPE_KWMETHOD("seek", &file_seek,
	              "(off:?Dint,whence=!PSET)->?Dint\n"
	              "(off:?Dint,whence:?Dint)->?Dint\n"
	              "#tValueError{The given string passed as seek mode @whence was not recognized}"

	              "Change the current file pointer according to @off and @whence "
	              /**/ "before returning its absolute offset within the file.\n"

	              "When a string is given for @whence, it may be one of the following "
	              /**/ "case-insensitive values, optionally prefixed with $\"SEEK_\".\n"

	              "#T{Whence-name|Description~"
	              /**/ "$\"SET\"|Set the file pointer to an absolute in-file position&"
	              /**/ "$\"CUR\"|Adjust the file pointer relative to its previous position&"
	              /**/ "$\"END\"|Set the file pointer relative to the end of the stream}"),
	TYPE_METHOD("tell", &file_tell,
	            "->?Dint\n"
	            "Same as calling ?#seek as ${this.seek(0, \"CUR\")}"),
	TYPE_METHOD(STR_rewind, &file_rewind,
	            "()\n"
	            "Same as calling ?#seek as ${this.seek(0, \"SET\")}"),
	TYPE_METHOD("trunc", &file_trunc,
	            "->?Dint\n"
	            "(size:?Dint)->?Dint\n"
	            "Truncate the file to a new length of @size bytes. "
	            /**/ "When no argument is given, the file's length is truncated "
	            /**/ "to its current position (?#tell), rather than the one given."),
	TYPE_METHOD("sync", &file_sync,
	            "()\n"
	            "Flush buffers and synchronize disk activity of the file."),
	TYPE_METHOD("close", &file_close,
	            "()\n"
	            "Close the file"),
	TYPE_METHOD("getc", &file_getc,
	            "->?Dint\n"
	            "Read and return a single character (byte) from then file, "
	            /**/ "or return ${-1} if the file's end has been reached."),
	TYPE_METHOD("ungetc", &file_ungetc,
	            "(ch:?Dint)->?Dbool\n"
	            "Unget a given character @ch to be re-read the next time ?#getc or ?#read is called. "
	            /**/ "If the file's start has already been reached, ?f is returned and the character "
	            /**/ "will not be re-read from this file."),
	TYPE_METHOD("putc", &file_putc,
	            "(byte:?Dint)->?Dbool\n"
	            "Append a single @byte at the end of @this File, returning ?t on "
	            /**/ "success, or ?f if the file has entered an end-of-file state."),

	/* Unicode (utf-8) read/write functions */
	TYPE_METHOD("getutf8", &file_getutf8,
	            "->?Dstring\n"
	            "Read and return a single unicode character (utf-8) from then "
	            /**/ "file, or return $\"\" if the file's end has been reached."),
	TYPE_METHOD("ungetutf8", &file_ungetutf8,
	            "(ch:?Dstring)->?Dbool\n"
	            "Unget a given unicode character @ch (as utf-8) to be re-read the next time ?#getuni "
	            /**/ "or ?#read is called. If the file's start has already been reached, ?f is returned "
	            /**/ "and the character will not be re-read from this file."),
	TYPE_METHOD("pututf8", &file_pututf8,
	            "(data:?Dstring)->?Dbool\n"
	            "Append a unicode string @data (as utf-8) at the end of @this File, returning "
	            /**/ "?t on success, or ?f if the file has entered an end-of-file state."),

	TYPE_METHOD(STR_size, &file_size,
	            "->?Dint\n"
	            "Returns the size (in bytes) of the file stream."),
	TYPE_METHOD("readline", &file_readline,
	            "(keeplf:?Dbool)->?X2?DBytes?N\n"
	            "(maxbytes=!-1,keeplf=!t)->?X2?DBytes?N\n"
	            "Read one line from the file stream, but read at most @maxbytes bytes.\n"
	            "When @keeplf is ?f, strip the trailing linefeed from the returned ?DBytes object.\n"
	            "Once EOF is reached, return ?N instead."),

	/* mmap support */
	TYPE_KWMETHOD("mmap", &file_mmap,
	              "(minbytes=!0,maxbytes=!-1,offset=!-1,nulbytes=!0,readall=!f,mustmmap=!f,mapshared=!f)->?DBytes\n"
	              "#pminbytes{The min number of bytes (excluding @nulbytes) that should be mapped "
	              /*      */ "starting at @offset. If the file is smaller than this, or indicates EOF before "
	              /*      */ "this number of bytes has been reached, nul bytes are mapped for its remainder.}"
	              "#pmaxbytes{The max number of bytes (excluding @nulbytes) that should be mapped starting "
	              /*      */ "at @offset. If the file is smaller than this, or indicates EOF before this "
	              /*      */ "number of bytes has been reached, simply stop there.}"
	              "#poffset{Starting offset of mapping (absolute), or ${-1} to map the entire file}"
	              "#pnulbytes{When non-zero, append this many trailing ${0x00}-bytes at the end of the map}"
	              "#preadall{When ?t, use ?#readall, rather than ?#read}"
	              "#pmustmmap{When ?t, throw an :UnsupportedAPI exception if @this file doesn't support $mmap}"
	              "#pmapshared{When ?t, use $MAP_SHARED instead of $MAP_PRIVATE (also implies @mustmmap)}"
	              "Map the contents of the file into memory. If #Cmmap isn't supported by the file, and "
	              /**/ "@mustmmap is ?f, allow the use of ?#read and ?#readall for loading file data.\n"
	              "The returned ?DBytes object is always writable, though changes are only reflected within "
	              /**/ "files when @mapshared is ?t.\n"
	              "Calls to ?#read and ?#pread (without a caller-provided buffer) automatically make use of this "
	              /**/ "function during large I/O requests in order to off-load disk I/O until the actual point of use.\n"
	              "Be careful when using this function, and don't use it as a catch-all method of loading a file "
	              /**/ "from disk, since this function will fail for files that are larger than the maximum possible "
	              /**/ "address space (since every byte of the file needs to be given a distinct memory address for "
	              /**/ "this function to succeed, as opposed to something like #read when given $maxbytes)"),

	/* Deprecated functions. */
	TYPE_METHOD("readall", &file_readall,
	            "(maxbytes=!-1)->?DBytes\n"
	            "Deprecated alias for ${this.read(maxbytes, true)}"),
	TYPE_KWMETHOD("readat", &file_pread,
	              "(pos:?Dint,maxbytes=!-1,readall=!f)->?DBytes\n"
	              "Deprecated alias for ?#pread"),
	TYPE_KWMETHOD("writeat", &file_pwrite,
	              "(data:?DBytes,pos:?Dint,writeall=!t)->?Dint\n"
	              "Deprecated alias for ?#pwrite"),
	TYPE_METHOD("readallat", &file_readallat,
	            "(pos:?Dint,maxbytes=!-1)->?DBytes\n"
	            "Deprecated alias for ${this.pread(pos, maxbytes, true)}"),
	TYPE_KWMETHOD("setpos", &file_seek,
	              "(pos:?Dint)->?Dint\n"
	              "Deprecated alias for ?#seek"),
	TYPE_METHOD("flush", &file_sync,
	            "()\n"
	            "Deprecated alias for ?#sync"),
	TYPE_KWMETHOD("puts", &file_write,
	              "(data:?DBytes)->?Dint\n"
	              "Deprecated alias for ?#write"),

	TYPE_METHOD_END
};

PRIVATE struct type_with file_with = {
	/* Implement with-control for files to close the file upon exit. */
	/* .tp_enter = */ NULL,
	/* .tp_leave = */ &DeeFile_Close
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_next(DeeFileObject *__restrict self) {
	return DeeFile_ReadLine((DeeObject *)self, (size_t)-1, true);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_pos_get(DeeFileObject *__restrict self) {
	dpos_t result;
	result = DeeFile_Tell((DeeObject *)self);
	if unlikely(result == (dpos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_pos_del(DeeFileObject *__restrict self) {
	if unlikely(DeeFile_Rewind((DeeObject *)self) == (dpos_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_pos_set(DeeFileObject *self, DeeObject *value) {
	dpos_t newpos;
	if (DeeObject_AsUInt64(value, &newpos))
		goto err;
	if unlikely(DeeFile_SetPos((DeeObject *)self, newpos) == (dpos_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE struct type_getset tpconst file_getsets[] = {
	TYPE_GETSET("pos", &file_pos_get, &file_pos_del, &file_pos_set,
	            "->?Dint\n"
	            "Control the current file position"),

	/* Maintain at least a tiny bit of compatibility to the iterator interface... */
	TYPE_GETTER(STR_seq, &DeeObject_NewRef, "->?DFile"),

	TYPE_GETSET_END
};


PRIVATE struct type_seq file_seq = {
	/* .tp_iter_self = */ &DeeObject_NewRef
};


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
file_shl(DeeObject *self, DeeObject *some_object) {
	if (DeeFile_PrintObject(self, some_object))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
file_shr(DeeObject *self, DeeObject *some_object) {
	DeeBuffer buffer;
	size_t result;
	if (DeeObject_GetBuf(some_object, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
	result = DeeFile_ReadAll(self, buffer.bb_base, buffer.bb_size);
	DeeObject_PutBuf(some_object, &buffer, Dee_BUFFER_FWRITABLE);
	if unlikely(result == (size_t)-1)
		goto err;
	if unlikely(result < buffer.bb_size) {
		DeeError_Throwf(&DeeError_FSError,
		                "Failed to fill the entire buffer of %" PRFuSIZ " "
		                "bytes when only %" PRFuSIZ " were read",
		                buffer.bb_size, result);
		goto err;
	}
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
file_repr(DeeObject *__restrict self) {
	if (Dee_TYPE(self) == (DeeTypeObject *)&DeeFile_Type)
		return (DREF DeeStringObject *)DeeString_New("File()");
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_REPR);
	return NULL;
}


PRIVATE struct type_math file_math = {
	/* .tp_int32  = */ NULL,
	/* .tp_int64  = */ NULL,
	/* .tp_double = */ NULL,
	/* .tp_int    = */ NULL,
	/* .tp_inv    = */ NULL,
	/* .tp_pos    = */ NULL,
	/* .tp_neg    = */ NULL,
	/* .tp_add    = */ NULL,
	/* .tp_sub    = */ NULL,
	/* .tp_mul    = */ NULL,
	/* .tp_div    = */ NULL,
	/* .tp_mod    = */ NULL,
	/* .tp_shl    = */ &file_shl,
	/* .tp_shr    = */ &file_shr,
	/* .tp_and    = */ NULL,
	/* .tp_or     = */ NULL,
	/* .tp_xor    = */ NULL,
	/* .tp_pow    = */ NULL
};

PUBLIC DeeFileTypeObject DeeFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ DeeString_STR(&str_File),
		/* .tp_doc      = */ DOC("The base class for all file types\n"

		                         "Types derived from this are able to implement a variety of "
		                         /**/ "new operators related to file operations, which are then "
		                         /**/ "used by the member functions and other operators provided "
		                         /**/ "by this type, or can also be used directly:\n"

		                         "#T{Operator prototype|Description~"
		                         /**/ "${operator read(size: int): Bytes}|"
		                         /**/ "Reads up to size bytes and returns a buffer (usually a ?Dstring) containing read data."
		                         "&"
		                         /**/ "${operator write(buf: Bytes): int}|"
		                         /**/ "Writes data from buf into the file and returns the number of bytes written."
		                         "&"
		                         /**/ "${operator seek(off: int, whence: int): int}|"
		                         /**/ "Moves the file pointer relative to whence, "
		                         /**/ /**/ "which is one of ?#SEEK_SET, ?#SEEK_CUR or ?#SEEK_END. "
		                         /**/ /**/ "The return value of this operator is the new, "
		                         /**/ /**/ "absolute file position within the stream."
		                         "&"
		                         /**/ "${operator sync(): none}|"
		                         /**/ "Synchronize unwritten data with lower-level components."
		                         "&"
		                         /**/ "${operator trunc(int newsize): none}|"
		                         /**/ "Truncate, or pre-allocate file memory to match a length of newsize."
		                         "&"
		                         /**/ "${operator close(): none}|"
		                         /**/ "Close the file. This operator is invoked during destruction, but "
		                         /**/ /**/ "can also be invoked before then using the ?#close member function."
		                         "&"
		                         /**/ "${operator pread(size: int, pos: int): Bytes}|"
		                         /**/ "Similar to ${operator read}, but data is read from the given "
		                         /**/ /**/ "absolute file position pos."
		                         "&"
		                         /**/ "${operator pwrite(buf: Bytes, pos: int): int}|"
		                         /**/ "Similar to ${operator write}, but data is written to the given "
		                         /**/ /**/ "absolute file position pos."
		                         "&"
		                         /**/ "${operator getc(): int}|"
		                         /**/ "Reads, and returns a single byte from the stream (Usually the same "
		                         /**/ /**/ "as ${operator read(1)}). If EOF has been reached, a negative value "
		                         /**/ /**/ "is returned."
		                         "&"
		                         /**/ "${operator ungetc(ch: int): bool}|"
		                         /**/ "Returns a previously read character to the stream, allowing it to be "
		                         /**/ /**/ "read once again. This functionality isn't provided by many file types, "
		                         /**/ /**/ "but ?#buffer supports it, thereby allowing you to wrap practically any "
		                         /**/ /**/ "file into a buffer to enable support for ungetc, which is a dependency "
		                         /**/ /**/ "for ?#readline. If the character could be returned, ?t is returned. "
		                         /**/ /**/ "Otherwise ?f is is returned."
		                         "&"
		                         /**/ "${operator putc(ch: int): bool}|"
		                         /**/ "Write a single byte to the stream (Usually the same as ${operator write"
		                         /**/ /**/ "(Bytes({ ch }))}). Returns ?t if the byte was successfully "
		                         /**/ /**/ "written, or ?f if EOF was reached."
		                         "}\n"
		                         "\n"

		                         "()\n"
		                         "Default-construct the ?. base-class\n"
		                         "\n"

		                         "iter->?.\n"
		                         "Returns an iterator that allows for line-wise processing of "
		                         /**/ "file data, making use of the the ?#readline member function.\n"
		                         "The returned lines have their trailing line-feeds preserved.\n"
		                         "Note that because a ?. cannot be iterated multiple times "
		                         /**/ "without additional work being done, as well as the fact that "
		                         /**/ "this type of iteration isn't thread-save, ?. isn't derived "
		                         /**/ "from ?DSequence, meaning that abstract ?DSequence functions are "
		                         /**/ "not implicitly provided, but would have to be invoked like "
		                         /**/ "${Sequence.find(File.open(\"foo.txt\"), \"find this line\")}.\n"
		                         "Note that because ?. isn't derived from ?DSequence, the returned "
		                         /**/ "iterator also isn't required to be derived from ?DIterator.\n"
		                         "\n"

		                         "next->?DBytes\n"
		                         "Alias for ?#readline, allowing for line-wise reading of lines.\n"
		                         "Note that the trailing linefeed is always included in this.\n"
		                         "\n"

		                         "<<(ob)->\n"
		                         "#r{Always re-returns @this ?.}"
		                         "Same as ${print this: ob,;}\n"
		                         "\n"

		                         ">>(buf:?DBytes)->\n"
		                         "#tFSError{Failed to fill the entirety of @buf}"
		                         "#r{Always re-returns @this ?.}"
		                         "Same as ${this.readinto(buf, true)}\n"
		                         "\n"

		                         "leave->\n"
		                         "Invokes ${this.operator close()}\n"
		                         "Note that due to this operators presence, an "
		                         /**/ "implicit enter-operator exists, which is a no-op."),
		/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
#if 0 /* Even though `File' is technically a sequence, don't spam its
       * members with all the additional functions provided by `DeeSeq_Type'.
       * Especially considering that `File' does its own handling for
       * operations such as `a|b' (creating a multi-targeted file), whereas
       * `sequence' already implements that operator as a union (in the case
       * of file: of all lines in either file).
       * Besides: A lot of sequence functions expect to be able to re-run
       *          iterators multiple times, yet file iterators expect the
       *          user to rewind the file before they can be iterated again,
       *          meaning that in that respect, `File' isn't 100% compliant
       *          to the `sequence' interface, meaning we're kind-of not
       *          even allowed to consider ourselves a sequence.
       *          But as already said: that's a good thing, because
       *          we don't even want to be considered one. */
		/* .tp_base     = */ &DeeSeq_Type,
#else
		/* .tp_base     = */ &DeeObject_Type,
#endif
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ (dfunptr_t)&file_init,
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
			/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_repr,
			/* .tp_bool = */ NULL
		},
		/* .tp_call          = */ NULL,
		/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ &file_math,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ &file_seq,
		/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_next,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ &file_with,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ file_methods,
		/* .tp_getsets       = */ file_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ file_class_methods,
		/* .tp_class_getsets = */ file_class_getsets,
		/* .tp_class_members = */ file_class_members
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

#endif /* !GUARD_DEEMON_OBJECTS_FILE_C */
