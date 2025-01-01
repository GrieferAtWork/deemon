/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/mapfile.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/stringutils.h>
#include <deemon/super.h>
/**/

#include "file-type-operators.h"
/**/

#include "../runtime/runtime_error.h"

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

#ifndef FILE_READ_WRITE_WITH_GETC_PUTC_WRAPPERS_DEFINED
#define FILE_READ_WRITE_WITH_GETC_PUTC_WRAPPERS_DEFINED

INTERN WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_DefaultReadWithGetc(DeeFileObject *__restrict self, void *buffer,
                            size_t bufsize, Dee_ioflag_t flags) {
	size_t result;
	int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
	ft_getc = Dee_TYPE(self)->ft_getc;
	ASSERT(ft_getc != NULL);
	ASSERT(ft_getc != &DeeFile_DefaultGetcWithRead);
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

INTERN WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
DeeFile_DefaultWriteWithPutc(DeeFileObject *__restrict self,
                             void const *buffer,
                             size_t bufsize, Dee_ioflag_t flags) {
	size_t result;
	int (DCALL *ft_putc)(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
	ft_putc = Dee_TYPE(self)->ft_putc;
	ASSERT(ft_putc != NULL);
	ASSERT(ft_putc != &DeeFile_DefaultPutcWithWrite);
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

INTERN WUNUSED NONNULL((1)) int DCALL
DeeFile_DefaultGetcWithRead(DeeFileObject *__restrict self,
                            Dee_ioflag_t flags) {
	size_t status;
	byte_t result;
	size_t (DCALL *ft_read)(DeeFileObject *__restrict self,
	                        void *buffer, size_t bufsize,
	                        Dee_ioflag_t flags);
	ft_read = Dee_TYPE(self)->ft_read;
	ASSERT(ft_read != NULL);
	ASSERT(ft_read != &DeeFile_DefaultReadWithGetc);
	status = (*ft_read)(self, &result, 1, flags);
	ASSERT(status == 0 || status == 1 || status == (size_t)-1);
	if likely(status > 0)
		return (unsigned int)result;
	if (status == 0)
		return GETC_EOF;
	return GETC_ERR;
}

INTERN WUNUSED NONNULL((1)) int DCALL
DeeFile_DefaultPutcWithWrite(DeeFileObject *__restrict self,
                             int ch, Dee_ioflag_t flags) {
	size_t status;
	byte_t byte;
	size_t (DCALL *ft_write)(DeeFileObject *__restrict self,
	                         void const *buffer, size_t bufsize,
	                         Dee_ioflag_t flags);
	ft_write = Dee_TYPE(self)->ft_write;
	ASSERT(ft_write != NULL);
	ASSERT(ft_write != &DeeFile_DefaultWriteWithPutc);
	byte   = (byte_t)(unsigned int)ch;
	status = (*ft_write)(self, &byte, 1, flags);
	ASSERT(status == 0 || status == 1 || status == (size_t)-1);
	if likely(status > 0)
		return 0;
	if (status == 0)
		return GETC_EOF;
	return GETC_ERR;
}

INTERN WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_DefaultPreadWithSeekAndRead(DeeFileObject *__restrict self,
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

INTERN WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
DeeFile_DefaultPwriteWithSeekAndWrite(DeeFileObject *__restrict self,
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

INTERN WUNUSED NONNULL((1)) int DCALL
DeeFile_DefaultUngetcWithSeek(DeeFileObject *__restrict self, int ch) {
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

#endif /* !FILE_READ_WRITE_WITH_GETC_PUTC_WRAPPERS_DEFINED */


/* Inherit file operators from bases, and auto-complete missing operators. */
#ifndef FILE_TYPE_INHERIT_OPERATOR_DEFINED
#define FILE_TYPE_INHERIT_OPERATOR_DEFINED
#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field)                                                        \
	INTERN NONNULL((1)) bool DCALL                                                                               \
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
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritSeek, "operator seek", ft_seek)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritSync, "operator sync", ft_sync)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritTrunc, "operator trunc", ft_trunc)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritClose, "operator close", ft_close)
#undef DEFINE_TYPE_INHERIT_FUNCTION

#define DEFINE_TYPE_INHERIT_FUNCTION(name, opname, field, alt_condition, altfunc, ...)                           \
	INTERN NONNULL((1)) bool DCALL                                                                               \
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
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritRead, "operator read", ft_read,
                             self->ft_getc != NULL, &DeeFile_DefaultReadWithGetc,
                             if (base->ft_getc) self->ft_getc = base->ft_getc)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritWrite, "operator write", ft_write,
                             self->ft_putc != NULL, &DeeFile_DefaultWriteWithPutc,
                             if (base->ft_putc) self->ft_putc = base->ft_putc)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritGetc, "operator getc", ft_getc,
                             self->ft_read != NULL, &DeeFile_DefaultGetcWithRead,
                             if (base->ft_read) self->ft_read = base->ft_read)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritPutc, "operator putc", ft_putc,
                             self->ft_write != NULL, &DeeFile_DefaultPutcWithWrite,
                             if (base->ft_write) self->ft_write = base->ft_write)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritPRead, "operator pread", ft_pread,
                             (self->ft_seek != NULL || DeeFileType_InheritSeek(self)) &&
                             (self->ft_read != NULL || DeeFileType_InheritRead(self)),
                             &DeeFile_DefaultPreadWithSeekAndRead,
                             if (base->ft_seek) self->ft_seek = base->ft_seek;
                             if (base->ft_read) self->ft_read = base->ft_read)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritPWrite, "operator pwrite", ft_pwrite,
                             (self->ft_seek != NULL || DeeFileType_InheritSeek(self)) &&
                             (self->ft_write != NULL || DeeFileType_InheritWrite(self)),
                             &DeeFile_DefaultPwriteWithSeekAndWrite,
                             if (base->ft_seek) self->ft_seek = base->ft_seek;
                             if (base->ft_write) self->ft_write = base->ft_write)
DEFINE_TYPE_INHERIT_FUNCTION(DeeFileType_InheritUngetc, "operator ungetc", ft_ungetc,
                             (self->ft_seek != NULL || DeeFileType_InheritSeek(self)),
                             &DeeFile_DefaultUngetcWithSeek,
                             if (base->ft_seek) self->ft_seek = base->ft_seek)
#undef DEFINE_TYPE_INHERIT_FUNCTION
#endif /* !FILE_TYPE_INHERIT_OPERATOR_DEFINED */


#ifdef DEFINE_TYPED_OPERATORS
#define LOCAL_DeeFileType_invoke_ft_read(tp_self, ft_read, self, buffer, bufsize, flags)          DeeFileType_invoke_ft_read(tp_self, ft_read, self, buffer, bufsize, flags)
#define LOCAL_DeeFileType_invoke_ft_write(tp_self, ft_write, self, buffer, bufsize, flags)        DeeFileType_invoke_ft_write(tp_self, ft_write, self, buffer, bufsize, flags)
#define LOCAL_DeeFileType_invoke_ft_seek(tp_self, ft_seek, self, off, whence)                     DeeFileType_invoke_ft_seek(tp_self, ft_seek, self, off, whence)
#define LOCAL_DeeFileType_invoke_ft_sync(tp_self, ft_sync, self)                                  DeeFileType_invoke_ft_sync(tp_self, ft_sync, self)
#define LOCAL_DeeFileType_invoke_ft_trunc(tp_self, ft_trunc, self, size)                          DeeFileType_invoke_ft_trunc(tp_self, ft_trunc, self, size)
#define LOCAL_DeeFileType_invoke_ft_close(tp_self, ft_close, self)                                DeeFileType_invoke_ft_close(tp_self, ft_close, self)
#define LOCAL_DeeFileType_invoke_ft_pread(tp_self, ft_pread, self, buffer, bufsize, pos, flags)   DeeFileType_invoke_ft_pread(tp_self, ft_pread, self, buffer, bufsize, pos, flags)
#define LOCAL_DeeFileType_invoke_ft_pwrite(tp_self, ft_pwrite, self, buffer, bufsize, pos, flags) DeeFileType_invoke_ft_pwrite(tp_self, ft_pwrite, self, buffer, bufsize, pos, flags)
#define LOCAL_DeeFileType_invoke_ft_getc(tp_self, ft_getc, self, flags)                           DeeFileType_invoke_ft_getc(tp_self, ft_getc, self, flags)
#define LOCAL_DeeFileType_invoke_ft_ungetc(tp_self, ft_ungetc, self, ch)                          DeeFileType_invoke_ft_ungetc(tp_self, ft_ungetc, self, ch)
#define LOCAL_DeeFileType_invoke_ft_putc(tp_self, ft_putc, self, ch, flags)                       DeeFileType_invoke_ft_putc(tp_self, ft_putc, self, ch, flags)
#else /* DEFINE_TYPED_OPERATORS */
#define LOCAL_DeeFileType_invoke_ft_read(tp_self, ft_read, self, buffer, bufsize, flags)          (*ft_read)((DeeFileObject *)(self), buffer, bufsize, flags)
#define LOCAL_DeeFileType_invoke_ft_write(tp_self, ft_write, self, buffer, bufsize, flags)        (*ft_write)((DeeFileObject *)(self), buffer, bufsize, flags)
#define LOCAL_DeeFileType_invoke_ft_seek(tp_self, ft_seek, self, off, whence)                     (*ft_seek)((DeeFileObject *)(self), off, whence)
#define LOCAL_DeeFileType_invoke_ft_sync(tp_self, ft_sync, self)                                  (*ft_sync)((DeeFileObject *)(self))
#define LOCAL_DeeFileType_invoke_ft_trunc(tp_self, ft_trunc, self, size)                          (*ft_trunc)((DeeFileObject *)(self), size)
#define LOCAL_DeeFileType_invoke_ft_close(tp_self, ft_close, self)                                (*ft_close)((DeeFileObject *)(self))
#define LOCAL_DeeFileType_invoke_ft_pread(tp_self, ft_pread, self, buffer, bufsize, pos, flags)   (*ft_pread)((DeeFileObject *)(self), buffer, bufsize, pos, flags)
#define LOCAL_DeeFileType_invoke_ft_pwrite(tp_self, ft_pwrite, self, buffer, bufsize, pos, flags) (*ft_pwrite)((DeeFileObject *)(self), buffer, bufsize, pos, flags)
#define LOCAL_DeeFileType_invoke_ft_getc(tp_self, ft_getc, self, flags)                           (*ft_getc)((DeeFileObject *)(self), flags)
#define LOCAL_DeeFileType_invoke_ft_ungetc(tp_self, ft_ungetc, self, ch)                          (*ft_ungetc)((DeeFileObject *)(self), ch)
#define LOCAL_DeeFileType_invoke_ft_putc(tp_self, ft_putc, self, ch, flags)                       (*ft_putc)((DeeFileObject *)(self), ch, flags)
#endif /* !DEFINE_TYPED_OPERATORS */


#ifdef DEFINE_TYPED_OPERATORS
#define LOCAL_DeeObject_InvokeOperator(name, argc, argv)   DeeObject_TInvokeOperator(tp_self, self, name, argc, argv)
#define LOCAL_DeeObject_InvokeOperatorf(name, format, ...) DeeObject_TInvokeOperatorf(tp_self, self, name, format, __VA_ARGS__)
#else /* DEFINE_TYPED_OPERATORS */
#define LOCAL_DeeObject_InvokeOperator(name, argc, argv)   DeeObject_InvokeOperator(self, name, argc, argv)
#define LOCAL_DeeObject_InvokeOperatorf(name, format, ...) DeeObject_InvokeOperatorf(self, name, format, __VA_ARGS__)
#endif /* !DEFINE_TYPED_OPERATORS */


#ifdef DEFINE_TYPED_OPERATORS
#define LOAD_TP_SELF   /* nothing */
#define IF_TYPED(...)                       __VA_ARGS__
#define IF_TYPED_OR_ELSE(if_typed, or_else) if_typed
#define SUPER_PRIVATE_EXPANDARGS(...) (DeeTypeObject *tp_self, __VA_ARGS__)
#define DEFINE_FILE_OPERATOR(return, name, args) \
	INTERN return DCALL DeeFile_T##name SUPER_PRIVATE_EXPANDARGS args
#else /* DEFINE_TYPED_OPERATORS */
#define LOAD_TP_SELF   DeeTypeObject *tp_self = Dee_TYPE(self)
#define IF_TYPED(...)                       /* nothing */
#define IF_TYPED_OR_ELSE(if_typed, or_else) or_else
#define DEFINE_FILE_OPERATOR(return, name, args) \
	PUBLIC return (DCALL DeeFile_##name)args
#endif /* !DEFINE_TYPED_OPERATORS */


/************************************************************************/
/* File operator invocation helpers                                     */
/************************************************************************/
#ifndef DEFINE_TYPED_OPERATORS
DEFINE_FILE_OPERATOR(size_t, Read, (DeeObject *self, void *buffer, size_t bufsize)) {
	LOAD_TP_SELF;
IF_TYPED(IF_TYPED(again:))
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_read)(DeeFileObject *__restrict self, void *buffer,
			                        size_t bufsize, Dee_ioflag_t flags);
			ft_read = DeeType_AsFileType(tp_self)->ft_read;
			if likely(ft_read != NULL)
				return LOCAL_DeeFileType_invoke_ft_read(tp_self, ft_read, self, buffer, bufsize, Dee_FILEIO_FNORMAL);
		} while (DeeFileType_InheritRead(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TReadf(tp_self, self, buffer, bufsize, Dee_FILEIO_FNORMAL));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return IF_TYPED_OR_ELSE(DeeFile_TRead(tp_self, self, buffer, bufsize),
	                        DeeFile_Readf(self, buffer, bufsize, Dee_FILEIO_FNORMAL));
}

DEFINE_FILE_OPERATOR(size_t, Write, (DeeObject *self, void const *buffer, size_t bufsize)) {
	LOAD_TP_SELF;
IF_TYPED(IF_TYPED(again:))
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_write)(DeeFileObject *__restrict self, void const *buffer,
			                         size_t bufsize, Dee_ioflag_t flags);
			ft_write = DeeType_AsFileType(tp_self)->ft_write;
			if likely(ft_write != NULL)
				return LOCAL_DeeFileType_invoke_ft_write(tp_self, ft_write, self, buffer, bufsize, Dee_FILEIO_FNORMAL);
		} while (DeeFileType_InheritWrite(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TWritef(tp_self, self, buffer, bufsize, Dee_FILEIO_FNORMAL));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return IF_TYPED_OR_ELSE(DeeFile_TWrite(tp_self, self, buffer, bufsize),
	                        DeeFile_Writef(self, buffer, bufsize, Dee_FILEIO_FNORMAL));
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_FILE_OPERATOR(size_t, Readf, (DeeObject *self, void *buffer,
                                     size_t bufsize, Dee_ioflag_t flags)) {
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_read)(DeeFileObject *__restrict self, void *buffer,
			                        size_t bufsize, Dee_ioflag_t flags);
			ft_read = DeeType_AsFileType(tp_self)->ft_read;
			if likely(ft_read != NULL)
				return LOCAL_DeeFileType_invoke_ft_read(tp_self, ft_read, self, buffer, bufsize, flags);
		} while (DeeFileType_InheritRead(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TReadf(tp_self, self, buffer, bufsize, flags));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	}
	/* TODO: Need a way to construct temporary Bytes objects: `DeeBytes_NewTempView() + DeeBytes_ReleaseTempView()',
	 *       where `DeeBytes_ReleaseTempView()' makes it so the bytes object can't be accessed anymore.
	 *       (and yes: I realize that means adding a lock to `DeeBytesObject') */
	/* TODO: Make a call to DeeObject_InvokeOperatorf() */
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_READ);
}

DEFINE_FILE_OPERATOR(size_t, Writef, (DeeObject *self, void const *buffer,
                                      size_t bufsize, Dee_ioflag_t flags)) {
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_write)(DeeFileObject *__restrict self, void const *buffer,
			                        size_t bufsize, Dee_ioflag_t flags);
			ft_write = DeeType_AsFileType(tp_self)->ft_write;
			if likely(ft_write != NULL)
				return LOCAL_DeeFileType_invoke_ft_write(tp_self, ft_write, self, buffer, bufsize, flags);
		} while (DeeFileType_InheritWrite(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TWritef(tp_self, self, buffer, bufsize, flags));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	}
	/* TODO: Need a way to construct temporary Bytes objects: `DeeBytes_NewTempView() + DeeBytes_ReleaseTempView()',
	 *       where `DeeBytes_ReleaseTempView()' makes it so the bytes object can't be accessed anymore.
	 *       (and yes: I realize that means adding a lock to `DeeBytesObject') */
	/* TODO: Make a call to DeeObject_InvokeOperatorf() */
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_READ);
}

DEFINE_FILE_OPERATOR(Dee_pos_t, Seek, (DeeObject *__restrict self, Dee_off_t off, int whence)) {
	Dee_pos_t result;
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			Dee_pos_t (DCALL *ft_seek)(DeeFileObject *__restrict self, Dee_off_t off, int whence);
			ft_seek = DeeType_AsFileType(tp_self)->ft_seek;
			if likely(ft_seek != NULL)
				return LOCAL_DeeFileType_invoke_ft_seek(tp_self, ft_seek, self, off, whence);
		} while (DeeFileType_InheritSeek(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TSeek(tp_self, self, off, whence));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperatorf(FILE_OPERATOR_SEEK,
	                                            PCKd64 "d", (int64_t)off, whence);
	if unlikely(!result_ob)
		goto err;
	if unlikely(DeeObject_AsUIntX(result_ob, &result))
		goto err_result_ob;
	if unlikely(result == (Dee_pos_t)-1)
		goto err_result_ob_overflow;
	Dee_Decref(result_ob);
	return result;
err_result_ob_overflow:
	err_integer_overflow(result_ob, sizeof(Dee_pos_t) * 8, true);
err_result_ob:
	Dee_Decref(result_ob);
err:
	return (Dee_pos_t)-1;
}

DEFINE_FILE_OPERATOR(int, Sync, (DeeObject *__restrict self)) {
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_sync)(DeeFileObject *__restrict self);
			ft_sync = DeeType_AsFileType(tp_self)->ft_sync;
			if likely(ft_sync != NULL)
				return LOCAL_DeeFileType_invoke_ft_sync(tp_self, ft_sync, self);
		} while (DeeFileType_InheritSync(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TSync(tp_self, self));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperator(FILE_OPERATOR_SYNC, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

DEFINE_FILE_OPERATOR(int, Trunc, (DeeObject *__restrict self, Dee_pos_t size)) {
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_trunc)(DeeFileObject *__restrict self, Dee_pos_t size);
			ft_trunc = DeeType_AsFileType(tp_self)->ft_trunc;
			if likely(ft_trunc != NULL)
				return LOCAL_DeeFileType_invoke_ft_trunc(tp_self, ft_trunc, self, size);
		} while (DeeFileType_InheritTrunc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TTrunc(tp_self, self, size));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperatorf(FILE_OPERATOR_TRUNC,
	                                            PCKu64, (uint64_t)size);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

DEFINE_FILE_OPERATOR(int, TruncHere, (DeeObject *__restrict self, Dee_pos_t *p_size)) {
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_trunc)(DeeFileObject *__restrict self, Dee_pos_t size);
			Dee_pos_t (DCALL *ft_seek)(DeeFileObject *__restrict self, Dee_off_t off, int whence);
			ft_trunc = DeeType_AsFileType(tp_self)->ft_trunc;
			ft_seek  = DeeType_AsFileType(tp_self)->ft_seek;
			if likely(ft_trunc != NULL && ft_seek != NULL) {
				int result;
				Dee_pos_t trunc_pos;
				/* Determine the current position and truncate the file there. */
				trunc_pos = LOCAL_DeeFileType_invoke_ft_seek(tp_self, ft_seek, self, 0, SEEK_CUR);
				if unlikely(trunc_pos == (Dee_pos_t)-1) {
					result = -1;
				} else {
					result = LOCAL_DeeFileType_invoke_ft_trunc(tp_self, ft_trunc, self, trunc_pos);
				}
				if (p_size != NULL)
					*p_size = trunc_pos;
				return result;
			}
		} while (DeeFileType_InheritTrunc(DeeType_AsFileType(tp_self)) ||
		         DeeFileType_InheritSeek(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TTruncHere(tp_self, self, p_size));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperator(FILE_OPERATOR_TRUNC, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

DEFINE_FILE_OPERATOR(int, Close, (DeeObject *__restrict self)) {
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_close)(DeeFileObject *__restrict self);
			ft_close = DeeType_AsFileType(tp_self)->ft_close;
			if likely(ft_close != NULL)
				return LOCAL_DeeFileType_invoke_ft_close(tp_self, ft_close, self);
		} while (DeeFileType_InheritClose(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TClose(tp_self, self));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperator(FILE_OPERATOR_CLOSE, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	Dee_Decref(result_ob);
	return 0;
err:
	return -1;
}

DEFINE_FILE_OPERATOR(int, Ungetc, (DeeObject *__restrict self, int ch)) {
	int temp;
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_ungetc)(DeeFileObject *__restrict self, int ch);
			ft_ungetc = DeeType_AsFileType(tp_self)->ft_ungetc;
			if likely(ft_ungetc != NULL)
				return LOCAL_DeeFileType_invoke_ft_ungetc(tp_self, ft_ungetc, self, ch);
		} while (DeeFileType_InheritUngetc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TUngetc(tp_self, self, ch));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperatorf(FILE_OPERATOR_UNGETC,
	                                            PCKuN(1), (byte_t)ch);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return ch;
	}
	temp = DeeObject_BoolInherited(result_ob);
	if unlikely(temp < 0)
		goto err;
	return temp ? ch : GETC_EOF;
err:
	return GETC_ERR;
}

#ifndef DEFINE_TYPED_OPERATORS
DEFINE_FILE_OPERATOR(int, Getc, (DeeObject *__restrict self)) {
	int temp;
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
			ft_getc = DeeType_AsFileType(tp_self)->ft_getc;
			if likely(ft_getc != NULL)
				return LOCAL_DeeFileType_invoke_ft_getc(tp_self, ft_getc, self, Dee_FILEIO_FNORMAL);
		} while (DeeFileType_InheritGetc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TGetc(tp_self, self));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperator(FILE_OPERATOR_GETC, 0, NULL);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return GETC_EOF;
	}
	if unlikely(DeeObject_AsInt(result_ob, &temp))
		goto err_result_ob;
	if likely(temp == GETC_EOF || (temp >= 0 && temp <= 0xff)) {
		Dee_Decref(result_ob);
		return temp;
	}
	err_integer_overflow(result_ob, 8, temp >= 0);
err_result_ob:
	Dee_Decref(result_ob);
err:
	return GETC_ERR;
}

DEFINE_FILE_OPERATOR(int, Putc, (DeeObject *__restrict self, int ch)) {
	int temp;
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_putc)(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
			ft_putc = DeeType_AsFileType(tp_self)->ft_putc;
			if likely(ft_putc != NULL)
				return LOCAL_DeeFileType_invoke_ft_putc(tp_self, ft_putc, self, ch, Dee_FILEIO_FNORMAL);
		} while (DeeFileType_InheritPutc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TPutc(tp_self, self, ch));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperatorf(FILE_OPERATOR_PUTC,
	                                            PCKuN(1), (byte_t)ch);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return ch;
	}
	temp = DeeObject_BoolInherited(result_ob);
	if unlikely(temp < 0)
		goto err;
	return temp ? ch : GETC_EOF;
err:
	return GETC_ERR;
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_FILE_OPERATOR(int, Getcf, (DeeObject *__restrict self, Dee_ioflag_t flags)) {
	int temp;
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
			ft_getc = DeeType_AsFileType(tp_self)->ft_getc;
			if likely(ft_getc != NULL)
				return LOCAL_DeeFileType_invoke_ft_getc(tp_self, ft_getc, self, flags);
		} while (DeeFileType_InheritGetc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TGetcf(tp_self, self, flags));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperatorf(FILE_OPERATOR_GETC, "u", flags);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return GETC_EOF;
	}
	if unlikely(DeeObject_AsInt(result_ob, &temp))
		goto err_result_ob;
	if likely(temp == GETC_EOF || (temp >= 0 && temp <= 0xff)) {
		Dee_Decref(result_ob);
		return temp;
	}
	err_integer_overflow(result_ob, 8, temp >= 0);
err_result_ob:
	Dee_Decref(result_ob);
err:
	return GETC_ERR;
}

DEFINE_FILE_OPERATOR(int, Putcf, (DeeObject *__restrict self, int ch, Dee_ioflag_t flags)) {
	int temp;
	DREF DeeObject *result_ob;
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			int (DCALL *ft_putc)(DeeFileObject *__restrict self, int ch, Dee_ioflag_t flags);
			ft_putc = DeeType_AsFileType(tp_self)->ft_putc;
			if likely(ft_putc != NULL)
				return LOCAL_DeeFileType_invoke_ft_putc(tp_self, ft_putc, self, ch, flags);
		} while (DeeFileType_InheritPutc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TPutcf(tp_self, self, ch, flags));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	result_ob = LOCAL_DeeObject_InvokeOperatorf(FILE_OPERATOR_PUTC,
	                                            PCKuN(1) "u", (byte_t)ch, flags);
	if unlikely(!result_ob)
		goto err;
	if (DeeNone_Check(result_ob)) {
		Dee_DecrefNokill(result_ob);
		return ch;
	}
	temp = DeeObject_BoolInherited(result_ob);
	if unlikely(temp < 0)
		goto err;
	return temp ? ch : GETC_EOF;
err:
	return GETC_ERR;
}


#ifndef DEFINE_TYPED_OPERATORS
DEFINE_FILE_OPERATOR(size_t, PRead, (DeeObject *__restrict self, void *buffer,
                                     size_t bufsize, Dee_pos_t pos)) {
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pread)(DeeFileObject *__restrict self, void *buffer,
			                         size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pread = DeeType_AsFileType(tp_self)->ft_pread;
			if likely(ft_pread != NULL)
				return LOCAL_DeeFileType_invoke_ft_pread(tp_self, ft_pread, self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL);
		} while (DeeFileType_InheritPRead(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TPRead(tp_self, self, buffer, bufsize, pos));
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return IF_TYPED_OR_ELSE(DeeFile_TPReadf(tp_self, self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL),
	                        DeeFile_PReadf(self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL));
}

DEFINE_FILE_OPERATOR(size_t, PWrite, (DeeObject *__restrict self, void const *buffer,
                                      size_t bufsize, Dee_pos_t pos)) {
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pwrite)(DeeFileObject *__restrict self, void const *buffer,
			                          size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pwrite = DeeType_AsFileType(tp_self)->ft_pwrite;
			if likely(ft_pwrite != NULL)
				return LOCAL_DeeFileType_invoke_ft_pwrite(tp_self, ft_pwrite, self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL);
		} while (DeeFileType_InheritPWrite(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TPWrite(tp_self, self, buffer, bufsize, pos));
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	return IF_TYPED_OR_ELSE(DeeFile_TPWritef(tp_self, self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL),
	                        DeeFile_PWritef(self, buffer, bufsize, pos, Dee_FILEIO_FNORMAL));
}
#endif /* !DEFINE_TYPED_OPERATORS */

DEFINE_FILE_OPERATOR(size_t, PReadf, (DeeObject *__restrict self, void *buffer,
                                      size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags)) {
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pread)(DeeFileObject *__restrict self, void *buffer,
			                         size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pread = DeeType_AsFileType(tp_self)->ft_pread;
			if likely(ft_pread != NULL)
				return LOCAL_DeeFileType_invoke_ft_pread(tp_self, ft_pread, self, buffer, bufsize, pos, flags);
		} while (DeeFileType_InheritPRead(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TPReadf(tp_self, self, buffer, bufsize, pos, flags));
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	/* TODO: Need a way to construct temporary Bytes objects: `DeeBytes_NewTempView() + DeeBytes_ReleaseTempView()',
	 *       where `DeeBytes_ReleaseTempView()' makes it so the bytes object can't be accessed anymore.
	 *       (and yes: I realize that means adding a lock to `DeeBytesObject') */
	/* TODO: Make a call to DeeObject_InvokeOperatorf() */
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_PREAD);
}

DEFINE_FILE_OPERATOR(size_t, PWritef, (DeeObject *__restrict self, void const *buffer,
                                       size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags)) {
	LOAD_TP_SELF;
IF_TYPED(again:)
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			size_t (DCALL *ft_pwrite)(DeeFileObject *__restrict self, void const *buffer,
			                          size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
			ft_pwrite = DeeType_AsFileType(tp_self)->ft_pwrite;
			if likely(ft_pwrite != NULL)
				return LOCAL_DeeFileType_invoke_ft_pwrite(tp_self, ft_pwrite, self, buffer, bufsize, pos, flags);
		} while (DeeFileType_InheritPWrite(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		IF_TYPED_OR_ELSE(goto again, return DeeFile_TPWritef(tp_self, self, buffer, bufsize, pos, flags));
	} else if (tp_self == &DeeNone_Type) {
		return 0;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	/* TODO: Need a way to construct temporary Bytes objects: `DeeBytes_NewTempView() + DeeBytes_ReleaseTempView()',
	 *       where `DeeBytes_ReleaseTempView()' makes it so the bytes object can't be accessed anymore.
	 *       (and yes: I realize that means adding a lock to `DeeBytesObject') */
	/* TODO: Make a call to DeeObject_InvokeOperatorf() */
	return (size_t)err_unimplemented_operator(tp_self, FILE_OPERATOR_PWRITE);
}

DECL_END

#undef LOCAL_DeeFileType_invoke_ft_read
#undef LOCAL_DeeFileType_invoke_ft_write
#undef LOCAL_DeeFileType_invoke_ft_seek
#undef LOCAL_DeeFileType_invoke_ft_sync
#undef LOCAL_DeeFileType_invoke_ft_trunc
#undef LOCAL_DeeFileType_invoke_ft_close
#undef LOCAL_DeeFileType_invoke_ft_pread
#undef LOCAL_DeeFileType_invoke_ft_pwrite
#undef LOCAL_DeeFileType_invoke_ft_getc
#undef LOCAL_DeeFileType_invoke_ft_ungetc
#undef LOCAL_DeeFileType_invoke_ft_putc
#undef LOCAL_DeeObject_InvokeOperator
#undef LOCAL_DeeObject_InvokeOperatorf
#undef LOAD_TP_SELF
#undef IF_TYPED
#undef IF_TYPED_OR_ELSE
#undef SUPER_PRIVATE_EXPANDARGS
#undef DEFINE_FILE_OPERATOR
#undef DEFINE_TYPED_OPERATORS
