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
#ifndef GUARD_DEEMON_OBJECTS_FILE_C
#define GUARD_DEEMON_OBJECTS_FILE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/mapfile.h>
#include <deemon/module.h>
#include <deemon/none-operator.h>
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

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "file-type-operators.h"

#undef byte_t
#define byte_t __BYTE_TYPE__

/* Trace self-optimizing operator inheritance. */
#if 1
#define LOG_INHERIT(base, self, what) \
	Dee_DPRINTF("[RT] Inherit `" what "' from %k into %k\n", base, self)
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

PRIVATE WUNUSED NONNULL((1)) int
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

PRIVATE WUNUSED NONNULL((1)) uint32_t
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

PUBLIC WUNUSED NONNULL((1)) uint32_t DCALL
DeeFile_GetUtf8(DeeObject *__restrict self) {
	int result;
	int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			ft_getc = DeeType_AsFileType(tp_self)->ft_getc;
			if likely(ft_getc != NULL) {
				if (ft_getc == &instance_getc && Dee_TYPE(self) != tp_self)
					goto do_invoke_generic_ft_getc;
				goto do_invoke_ft_getc;
			}
		} while (DeeFileType_InheritGetc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
do_invoke_generic_ft_getc:
	ft_getc = (int (DCALL *)(DeeFileObject *__restrict, Dee_ioflag_t))&DeeFile_Getcf;
do_invoke_ft_getc:
	result = (*ft_getc)((DeeFileObject *)self, Dee_FILEIO_FNORMAL);
	if likely(result < 0xc0)
		return (uint32_t)result;
	return DeeFile_unicode_readutf8((DeeFileObject *)self, ft_getc,
	                                (byte_t)result, Dee_FILEIO_FNORMAL);
}

PUBLIC WUNUSED NONNULL((1)) uint32_t DCALL
DeeFile_GetUtf8f(DeeObject *__restrict self, Dee_ioflag_t flags) {
	int result;
	int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			ft_getc = DeeType_AsFileType(tp_self)->ft_getc;
			if likely(ft_getc != NULL) {
				if (ft_getc == &instance_getc && Dee_TYPE(self) != tp_self)
					goto do_invoke_generic_ft_getc;
				goto do_invoke_ft_getc;
			}
		} while (DeeFileType_InheritGetc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
do_invoke_generic_ft_getc:
	ft_getc = (int (DCALL *)(DeeFileObject *__restrict, Dee_ioflag_t))&DeeFile_Getcf;
do_invoke_ft_getc:
	result = DeeFile_Getcf(self, flags);
	if likely(result < 0xc0)
		return (uint32_t)result;
	return DeeFile_unicode_readutf8((DeeFileObject *)self, ft_getc,
	                                (byte_t)result, flags);
}

/* @return: Dee_GETC_ERR: Error */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeFile_UngetUtf8(DeeObject *__restrict self, uint32_t ch) {
	int result;
	char buf[UNICODE_UTF8_CURLEN], *endp;
	int (DCALL *ft_ungetc)(DeeFileObject *__restrict self, int ch);
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			ft_ungetc = DeeType_AsFileType(tp_self)->ft_ungetc;
			if likely(ft_ungetc != NULL) {
				if (ft_ungetc == &instance_ungetc && Dee_TYPE(self) != tp_self)
					goto do_invoke_generic_ft_ungetc;
				goto do_invoke_ft_ungetc;
			}
		} while (DeeFileType_InheritUngetc(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
do_invoke_generic_ft_ungetc:
	ft_ungetc = (int (DCALL *)(DeeFileObject *__restrict, int))&DeeFile_Ungetc;
do_invoke_ft_ungetc:
	endp = unicode_writeutf8(buf, ch);
	do {
		--endp;
		result = (*ft_ungetc)((DeeFileObject *)self, (int)(unsigned int)(unsigned char)*endp);
		if unlikely(result != 0)
			break;
	} while (endp > buf);
	return result;
}


PUBLIC WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_ReadAll(DeeObject *self,
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

PUBLIC WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DPRINTER_CC
DeeFile_WriteAll(DeeObject *self,
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

PUBLIC WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL
DeeFile_PReadAll(DeeObject *self,
                 void *buffer,
                 size_t bufsize, Dee_pos_t pos) {
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

PUBLIC WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
DeeFile_PWriteAll(DeeObject *self,
                  void const *buffer,
                  size_t bufsize, Dee_pos_t pos) {
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

PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeFile_IsAtty(DeeObject *__restrict self) {
	DREF DeeObject *result_ob;

	/* Very simply: Just lookup the `isatty' property. */
	result_ob = DeeObject_GetAttr(self, (DeeObject *)&str_isatty);
	if unlikely(!result_ob)
		goto err_call;
	return DeeObject_BoolInherited(result_ob);
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
PUBLIC WUNUSED NONNULL((1)) Dee_fd_t DCALL
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
PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
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
PUBLIC WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_ReadLine(DeeObject *__restrict self,
                 size_t maxbytes, bool keep_lf) {
	struct bytes_printer printer;
	int (DCALL *ft_getc)(DeeFileObject *__restrict self, Dee_ioflag_t flags);
	int (DCALL *ft_ungetc)(DeeFileObject *__restrict self, int ch);
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			ft_getc   = DeeType_AsFileType(tp_self)->ft_getc;
			ft_ungetc = DeeType_AsFileType(tp_self)->ft_ungetc;
			if likely(ft_getc != NULL && ft_ungetc != NULL) {
				if (ft_getc == &instance_getc && Dee_TYPE(self) != tp_self)
					ft_getc = (int (DCALL *)(DeeFileObject *__restrict, Dee_ioflag_t))&DeeFile_Getcf;
				if (ft_ungetc == &instance_ungetc && Dee_TYPE(self) != tp_self)
					ft_ungetc = (int (DCALL *)(DeeFileObject *__restrict, int))&DeeFile_Ungetc;
				goto do_operate_using_ft_getc_and_ft_ungetc;
			}
		} while (DeeFileType_InheritGetc(DeeType_AsFileType(tp_self)) ||
		         DeeFileType_InheritUngetc(DeeType_AsFileType(tp_self)));
		if (DeeType_AsFileType(tp_self)->ft_getc != NULL) {
			/* If getc() is implemented, but ungetc() isn't, indicate the correct missing operator. */
			err_unimplemented_operator(tp_self, FILE_OPERATOR_UNGETC);
			goto err;
		}
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
	ft_getc   = (int (DCALL *)(DeeFileObject *__restrict, Dee_ioflag_t))&DeeFile_Getcf;
	ft_ungetc = (int (DCALL *)(DeeFileObject *__restrict, int))&DeeFile_Ungetc;
do_operate_using_ft_getc_and_ft_ungetc:
	bytes_printer_init(&printer);

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

INTDEF WUNUSED NONNULL((1)) ATTR_OUTS(2, 3) size_t DCALL
sysfile_read(DeeSystemFileObject *__restrict self,
             void *buffer, size_t bufsize,
             dioflag_t flags);
INTERN WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
sysfile_pread(DeeSystemFileObject *__restrict self,
              void *buffer, size_t bufsize,
              Dee_pos_t pos, dioflag_t flags);

PRIVATE WUNUSED DREF /*Bytes*/ DeeObject *DCALL
file_read_trymap(Dee_fd_t fd, size_t maxbytes,
                 Dee_pos_t pos, bool readall) {
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

PUBLIC WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_ReadBytes(DeeObject *__restrict self,
                  size_t maxbytes, bool readall) {
	size_t (DCALL *ft_read)(DeeFileObject *__restrict self, void *buffer,
	                        size_t bufsize, Dee_ioflag_t flags);
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			ft_read = DeeType_AsFileType(tp_self)->ft_read;
			if likely(ft_read != NULL) {
				if (ft_read == &instance_read && Dee_TYPE(self) != tp_self)
					goto do_invoke_generic_ft_read;
				goto do_invoke_ft_read;
			}
		} while (DeeFileType_InheritRead(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
do_invoke_generic_ft_read:
	ft_read = (size_t (DCALL *)(DeeFileObject *__restrict self, void *__restrict, size_t, dioflag_t flags))&DeeFile_Readf;
do_invoke_ft_read:
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
		result = file_read_trymap(os_fd, maxbytes, (Dee_pos_t)-1, readall);
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
			if (!read_size || (!readall && read_size < bufsize))
				break; /* EOF */
			maxbytes -= read_size;
			if (read_size >= bufsize)
				readtext_bufsize *= 2;
		}
/*done_printer:*/
		return bytes_printer_pack(&printer);
err_printer:
		bytes_printer_fini(&printer);
	} /* Scope... */
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF /*Bytes*/ DeeObject *DCALL
DeeFile_PReadBytes(DeeObject *__restrict self,
                   size_t maxbytes, Dee_pos_t pos,
                   bool readall) {
	size_t (DCALL *ft_pread)(DeeFileObject *__restrict self, void *buffer,
	                         size_t bufsize, Dee_pos_t pos, Dee_ioflag_t flags);
	DeeTypeObject *tp_self = Dee_TYPE(self);
again:
	if likely(DeeFileType_CheckExact(tp_self)) {
do_handle_filetype:
		do {
			ft_pread = DeeType_AsFileType(tp_self)->ft_pread;
			if likely(ft_pread != NULL) {
				if (ft_pread == &instance_pread && Dee_TYPE(self) != tp_self)
					goto do_invoke_generic_ft_pread;
				goto do_invoke_ft_pread;
			}
		} while (DeeFileType_InheritPRead(DeeType_AsFileType(tp_self)));
	} else if (tp_self == &DeeSuper_Type) {
		tp_self = DeeSuper_TYPE(self);
		self    = DeeSuper_SELF(self);
		goto again;
	} else if (DeeFileType_Check(tp_self)) {
		goto do_handle_filetype;
	}
do_invoke_generic_ft_pread:
	ft_pread = (size_t (DCALL *)(DeeFileObject *__restrict self, void *__restrict, size_t, Dee_pos_t, dioflag_t flags))&DeeFile_PReadf;
do_invoke_ft_pread:
#ifdef HAVE_file_read_trymap
	/* if `ft_pread' belongs to `DeeSystemFile_Type', and `maxbytes' is larger
	 * than some threshold (>= 2*PAGESIZE), then try to create a file view using
	 * `DeeMapFile_InitSysFd()', which is then wrapped by file view holder object,
	 * which can then be wrapped by a regular `Bytes' object.
	 * -> That way, we can provide the user with O(1) reads from large files! */
	if ((maxbytes >= FILE_READ_MMAP_THRESHOLD) &&
	    (ft_pread == (size_t (DCALL *)(DeeFileObject *__restrict self, void *__restrict, size_t, Dee_pos_t, dioflag_t flags))&sysfile_pread)) {
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
			if (read_size >= bufsize)
				readtext_bufsize *= 2;
		}
/*done_printer:*/
		return bytes_printer_pack(&printer);
err_printer:
		bytes_printer_fini(&printer);
	} /* Scope... */
/*err:*/
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
print_sp(DeeObject *__restrict self) {
	size_t result = DeeFile_WriteAll(self, " ", sizeof(char));
	return unlikely(result == (size_t)-1) ? -1 : 0;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeFile_PrintNl)(DeeObject *__restrict self) {
	size_t result = DeeFile_WriteAll(self, "\n", sizeof(char));
	return unlikely(result == (size_t)-1) ? -1 : 0;
}

#define print_ob_str(self, ob) \
	DeeObject_Print(ob, (Dee_formatprinter_t)&DeeFile_WriteAll, self)
#define print_ob_repr(self, ob) \
	DeeObject_PrintRepr(ob, (Dee_formatprinter_t)&DeeFile_WriteAll, self)


PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintObject)(DeeObject *self,
                            DeeObject *ob) {
	if unlikely(print_ob_str(self, ob) < 0)
		goto err;
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintObjectSp)(DeeObject *self,
                              DeeObject *ob) {
	if unlikely(print_ob_str(self, ob) < 0)
		goto err;
	return print_sp(self);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintObjectNl)(DeeObject *self,
                              DeeObject *ob) {
	if unlikely(print_ob_str(self, ob) < 0)
		goto err;
	return DeeFile_PrintNl(self);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintObjectRepr)(DeeObject *self,
                                DeeObject *ob) {
	if unlikely(print_ob_repr(self, ob) < 0)
		goto err;
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintObjectReprSp)(DeeObject *self,
                                  DeeObject *ob) {
	if unlikely(print_ob_repr(self, ob) < 0)
		goto err;
	return print_sp(self);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintObjectReprNl)(DeeObject *self,
                                  DeeObject *ob) {
	if unlikely(print_ob_repr(self, ob) < 0)
		goto err;
	return DeeFile_PrintNl(self);
err:
	return -1;
}

struct file_printall_foreach_data {
	DeeObject *fpafd_file;  /* [1..1] The file to print into. */
	bool       fpafd_first; /* True if this is the first element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
file_printall_foreach_cb(void *arg, DeeObject *elem) {
	int result;
	struct file_printall_foreach_data *data;
	data = (struct file_printall_foreach_data *)arg;
	if (!data->fpafd_first) {
		result = print_sp(data->fpafd_file);
		if unlikely(result)
			return result;
	}
	data->fpafd_first = false;
	return DeeFile_PrintObject(data->fpafd_file, elem);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintAll)(DeeObject *self, DeeObject *ob) {
	struct file_printall_foreach_data data;
	data.fpafd_file  = self;
	data.fpafd_first = true;
	return (int)DeeObject_Foreach(ob, &file_printall_foreach_cb, &data);
}

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define file_printall_sp_foreach_cb_PTR ((Dee_foreach_t)&DeeFile_PrintObjectSp)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
#define file_printall_sp_foreach_cb_PTR &file_printall_sp_foreach_cb
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
file_printall_sp_foreach_cb(void *arg, DeeObject *elem) {
	return (Dee_ssize_t)DeeFile_PrintObjectSp((DeeObject *)arg, elem);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintAllSp)(DeeObject *self, DeeObject *ob) {
	return (int)DeeObject_Foreach(ob, file_printall_sp_foreach_cb_PTR, self);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeFile_PrintAllNl)(DeeObject *self,
                           DeeObject *ob) {
	int result = DeeFile_PrintAll(self, ob);
	if likely(result == 0)
		result = DeeFile_PrintNl(self);
	return result;
}




/* HINT: `DeeFile_Printf' is literally implemented as
 *       `DeeFormat_Printf(&DeeFile_WriteAll, self, format, ...)'
 * @return: -1: Error */
PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t
DeeFile_Printf(DeeObject *__restrict self,
               char const *__restrict format, ...) {
	va_list args;
	Dee_ssize_t result;
	va_start(args, format);
	result = DeeFile_VPrintf(self, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeFile_VPrintf(DeeObject *__restrict self,
                char const *__restrict format, va_list args) {
	return DeeFormat_VPrintf((Dee_formatprinter_t)&DeeFile_WriteAll, self, format, args);
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
				/* .tp_ctor      = */ (Dee_funptr_t)&filetype_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_GC(DeeFileTypeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&type_str),
		/* .tp_repr = */ DEFIMPL(&type_repr),
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&type_print),
		/* .tp_printrepr = */ DEFIMPL(&type_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__E2C81DE60D62A07B),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL((DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&DeeObject_New),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__97D8C6894F54F5E3),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ file_type_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(file_type_operators),
};






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
file_class_open(DeeObject *UNUSED(self), size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result, *new_result;
	uint8_t flags;
	int oflags;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("open", params: """
	DeeObject *path:?Dstring;
	DeeObject *oflags:?X2?Dstring?Dint=!Pr = NULL;
	int mode = 0644;
""", docStringPrefix: "file_class");]]]*/
#define file_class_open_params "path:?Dstring,oflags:?X2?Dstring?Dint=!Pr,mode=!0644"
	struct {
		DeeObject *path;
		DeeObject *oflags;
		int mode;
	} args;
	args.oflags = NULL;
	args.mode = 0644;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__path_oflags_mode, "o|od:open", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.path, &DeeString_Type))
		goto err;
	if (!args.oflags) {
		/* Default to `r' */
		flags  = OPEN_EXFLAG_FTEXT;
		oflags = OPEN_FRDONLY;
	} else if (!DeeString_Check(args.oflags)) {
		flags = OPEN_EXFLAG_FNORMAL;
		if (DeeObject_AsInt(args.oflags, &oflags))
			goto err;
	} else {
		char const *iter;
		iter   = DeeString_STR(args.oflags);
		flags  = OPEN_EXFLAG_FNORMAL;
		oflags = 0;
		for (;;) {
			bool open_binary;
			unsigned int i;
			size_t optlen;
			char const *next = strchr(iter, ',');
			if (!next)
				next = strend(iter);
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
	result = DeeFile_Open(args.path, oflags, args.mode);
	if unlikely(!ITER_ISOK(result)) {
		if unlikely(!result)
			goto err;
		/* Handle file-not-found / file-already-exists errors. */
		if ((oflags & (OPEN_FCREAT | OPEN_FEXCL)) == (OPEN_FCREAT | OPEN_FEXCL)) {
			DeeError_Throwf(&DeeError_FileExists,
			                "File %r already exists",
			                args.path);
		} else {
			DeeError_Throwf(&DeeError_FileNotFound,
			                "File %r could not be found",
			                args.path);
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
	                args.oflags);
err:
	return NULL;
}


PRIVATE struct type_method tpconst file_class_methods[] = {
	TYPE_KWMETHOD("open", &file_class_open,
	              "(" file_class_open_params ")->?.\n"
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
 * `DeeFile_GetStd()' will throw an `UnboundAttribute' error if the stream isn't assigned. */
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

PRIVATE WUNUSED int DCALL
file_std_isbound(unsigned int id) {
	DeeObject *result;
	ASSERT(id < DEE_STDCNT);
	result = Dee_atomic_read_with_atomic_rwlock(&dee_std[id], &dee_std_lock);
	return Dee_BOUND_FROMBOOL(result != NULL);
}

#define DEFINE_FILE_CLASS_STD_FUNCTIONS(stdxxx, DEE_STDXXX)         \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL              \
	file_class_get_##stdxxx(DeeObject *__restrict UNUSED(self)) {   \
		return DeeFile_GetStd(DEE_STDXXX);                          \
	}                                                               \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                          \
	file_class_bound_##stdxxx(DeeObject *__restrict UNUSED(self)) { \
		return file_std_isbound(DEE_STDXXX);                        \
	}                                                               \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                          \
	file_class_del_##stdxxx(DeeObject *__restrict UNUSED(self)) {   \
		DREF DeeObject *old_stream;                                 \
		old_stream = DeeFile_SetStd(DEE_STDXXX, NULL);              \
		Dee_XDecref(old_stream);                                    \
		return 0;                                                   \
	}                                                               \
	PRIVATE WUNUSED NONNULL((1, 2)) int DCALL                       \
	file_class_set_##stdxxx(DeeObject *UNUSED(self),                \
	                        DeeObject *value) {                     \
		DREF DeeObject *old_stream;                                 \
		old_stream = DeeFile_SetStd(DEE_STDXXX, value);             \
		if (ITER_ISOK(old_stream))                                  \
			Dee_Decref(old_stream);                                 \
		return 0;                                                   \
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
	TYPE_GETSET_BOUND("stdin",
	                  &file_class_get_stdin,
	                  &file_class_del_stdin,
	                  &file_class_set_stdin,
	                  &file_class_bound_stdin,
	                  "->?DFile\n"
	                  "The standard input stream"),
	TYPE_GETSET_BOUND("stdout",
	                  &file_class_get_stdout,
	                  &file_class_del_stdout,
	                  &file_class_set_stdout,
	                  &file_class_bound_stdout,
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
	TYPE_GETSET_BOUND("stderr",
	                  &file_class_get_stderr,
	                  &file_class_del_stderr,
	                  &file_class_set_stderr,
	                  &file_class_bound_stderr,
	                  "->?DFile\n"
	                  "The standard error stream"),
	TYPE_GETTER_AB("default_stdin",
	               &file_class_default_stdin,
	               "->?DFile\n"
	               "The default standard input stream"),
	TYPE_GETTER_AB("default_stdout",
	               &file_class_default_stdout,
	               "->?DFile\n"
	               "The default standard output stream"),
	TYPE_GETTER_AB("default_stderr",
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
#define OBJ_file_SEEK_SET DeeInt_Zero
#else /* SEEK_SET == 0 */
#define OBJ_file_SEEK_SET ((DeeObject *)&file_SEEK_SET)
PRIVATE DEFINE_UINT32(file_SEEK_SET, SEEK_SET);
#endif /* SEEK_SET != 0 */
#if SEEK_CUR == 1
#define OBJ_file_SEEK_CUR DeeInt_One
#else /* SEEK_CUR == 1 */
#define OBJ_file_SEEK_CUR ((DeeObject *)&file_SEEK_CUR)
PRIVATE DEFINE_UINT32(file_SEEK_CUR, SEEK_CUR);
#endif /* SEEK_CUR != 1 */
#define OBJ_file_SEEK_END ((DeeObject *)&file_SEEK_END)
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
	TYPE_MEMBER_CONST_DOC("SEEK_SET", OBJ_file_SEEK_SET,
	                      "Deprecated argument for ?#seek (Use the string $\"set\" instead)"),
	TYPE_MEMBER_CONST_DOC("SEEK_CUR", OBJ_file_SEEK_CUR,
	                      "Deprecated argument for ?#seek (Use the string $\"cur\" instead)"),
	TYPE_MEMBER_CONST_DOC("SEEK_END", OBJ_file_SEEK_END,
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
PUBLIC WUNUSED NONNULL((1)) Dee_pos_t DCALL
DeeFile_GetSize(DeeObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_CallAttr(self, (DeeObject *)&str_size, 0, NULL);
	if likely(result)
		return (Dee_pos_t)DeeObject_AsDirectUInt64Inherited(result);

	/* Failed to call the size() member function. */
	if (DeeFileType_CheckExact(Dee_TYPE(self)) &&
	    DeeError_Catch(&DeeError_AttributeError)) {
		/* Translate missing size() attributes to doesnt-implement-seek errors. */
		err_unimplemented_operator(Dee_TYPE(self), /* TODO: Pass orig error as "inner" */
		                           FILE_OPERATOR_SEEK);
	}
	return (Dee_pos_t)-1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_read(DeeObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("read", params: "
	size_t maxbytes = (size_t)-1;
	bool readall    = false;
", docStringPrefix: "file");]]]*/
#define file_read_params "maxbytes=!-1,readall=!f"
	struct {
		size_t maxbytes;
		bool readall;
	} args;
	args.maxbytes = (size_t)-1;
	args.readall = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__maxbytes_readall, "|" UNPxSIZ "b:read", &args))
		goto err;
/*[[[end]]]*/
	return DeeFile_ReadBytes(self, args.maxbytes, args.readall);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_readinto(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeBuffer buffer;
	size_t result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("readinto", params: "
	dst: obj:buffer;
	bool readall = false;
", docStringPrefix: "file");]]]*/
#define file_readinto_params "dst:?DBytes,readall=!f"
	struct {
		DeeObject *dst;
		bool readall;
	} args;
	args.readall = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dst_readall, "o|b:readinto", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_GetBuf(args.dst, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
	result = args.readall
	         ? DeeFile_ReadAll(self, buffer.bb_base, buffer.bb_size)
	         : DeeFile_Read(self, buffer.bb_base, buffer.bb_size);
	DeeObject_PutBuf(args.dst, &buffer, Dee_BUFFER_FWRITABLE);
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
	size_t result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("write", params: "
	data: obj:buffer;
	bool writeall = true;
", docStringPrefix: "file");]]]*/
#define file_write_params "data:?DBytes,writeall=!t"
	struct {
		DeeObject *data;
		bool writeall;
	} args;
	args.writeall = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_writeall, "o|b:write", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_GetBuf(args.data, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = args.writeall
	         ? DeeFile_WriteAll(self, buffer.bb_base, buffer.bb_size)
	         : DeeFile_Write(self, buffer.bb_base, buffer.bb_size);
	DeeObject_PutBuf(args.data, &buffer, Dee_BUFFER_FREADONLY);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_pread(DeeObject *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("pread", params: "
	Dee_pos_t pos;
	size_t maxbytes = (size_t)-1;
	bool readall      = false;
", docStringPrefix: "file");]]]*/
#define file_pread_params "pos:?Dint,maxbytes=!-1,readall=!f"
	struct {
		Dee_pos_t pos;
		size_t maxbytes;
		bool readall;
	} args;
	args.maxbytes = (size_t)-1;
	args.readall = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__pos_maxbytes_readall, UNPuN(Dee_SIZEOF_POS_T) "|" UNPxSIZ "b:pread", &args))
		goto err;
/*[[[end]]]*/
	return DeeFile_PReadBytes(self, args.maxbytes, args.pos, args.readall);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_preadinto(DeeObject *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	DeeBuffer buffer;
	size_t result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("preadinto", params: "
	dst: obj:buffer;
	Dee_pos_t pos;
	bool readall = false;
", docStringPrefix: "file");]]]*/
#define file_preadinto_params "dst:?DBytes,pos:?Dint,readall=!f"
	struct {
		DeeObject *dst;
		Dee_pos_t pos;
		bool readall;
	} args;
	args.readall = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dst_pos_readall, "o" UNPuN(Dee_SIZEOF_POS_T) "|b:preadinto", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_GetBuf(args.dst, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
	result = args.readall ? DeeFile_PReadAll(self, buffer.bb_base, buffer.bb_size, args.pos)
	                      : DeeFile_PRead(self, buffer.bb_base, buffer.bb_size, args.pos);
	DeeObject_PutBuf(args.dst, &buffer, Dee_BUFFER_FWRITABLE);
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
	size_t result;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("pwrite", params: "
	data: obj:buffer;
	Dee_pos_t pos;
	bool writeall = true;
", docStringPrefix: "file");]]]*/
#define file_pwrite_params "data:?DBytes,pos:?Dint,writeall=!t"
	struct {
		DeeObject *data;
		Dee_pos_t pos;
		bool writeall;
	} args;
	args.writeall = true;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__data_pos_writeall, "o" UNPuN(Dee_SIZEOF_POS_T) "|b:pwrite", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_GetBuf(args.data, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	result = args.writeall ? DeeFile_PWriteAll(self, buffer.bb_base, buffer.bb_size, args.pos)
	                       : DeeFile_PWrite(self, buffer.bb_base, buffer.bb_size, args.pos);
	DeeObject_PutBuf(args.data, &buffer, Dee_BUFFER_FREADONLY);
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
	Dee_pos_t result;
	int whence = SEEK_SET;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("seek", params: "
	Dee_off_t off;
	DeeObject *whence = NULL;
");]]]*/
	struct {
		Dee_off_t off;
		DeeObject *whence;
	} args;
	args.whence = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__off_whence, UNPdN(Dee_SIZEOF_OFF_T) "|o:seek", &args))
		goto err;
/*[[[end]]]*/
	if (args.whence) {
		if (DeeString_Check(args.whence)) {
			char const *name = DeeString_STR(args.whence);
			size_t length    = DeeString_SIZE(args.whence);
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
			                args.whence);
			goto err;
		}

		/* Fallback: Convert the whence-object to an integer. */
		if (DeeObject_AsInt(args.whence, &whence))
			goto err;
	}
got_whence:
	result = DeeFile_Seek(self, args.off, whence);
	if unlikely(result == (Dee_pos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_tell(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_pos_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("tell", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "tell");
/*[[[end]]]*/
	result = DeeFile_Tell(self);
	if unlikely(result == (Dee_pos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_rewind(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rewind", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "rewind");
/*[[[end]]]*/
	if (DeeFile_Rewind(self) == (Dee_pos_t)-1)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_trunc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_pos_t trunc_pos;
	if (argc == 0) {
		/* Truncate at the current position. */
		if (DeeFile_TruncHere(self, &trunc_pos))
			goto err;
	} else {
		/* Truncate at the current position. */
		if (DeeArg_UnpackStruct(argc, argv, "|" UNPuN(Dee_SIZEOF_POS_T) ":trunc", &trunc_pos))
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
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("sync", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "sync");
/*[[[end]]]*/
	if (DeeFile_Sync(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_close(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("close", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "close");
/*[[[end]]]*/
	if (DeeFile_Close(self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("getc", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "getc");
/*[[[end]]]*/
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
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("ungetc", params: "
	int ch;
", docStringPrefix: "file");]]]*/
#define file_ungetc_params "ch:?Dint"
	struct {
		int ch;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, "d:ungetc", &args))
		goto err;
/*[[[end]]]*/
	result = DeeFile_Ungetc(self, args.ch);
	if unlikely(result == GETC_ERR)
		goto err;
	return_bool(result != GETC_EOF);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_putc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("putc", params: "
	byte_t byte;
", docStringPrefix: "file");]]]*/
#define file_putc_params "byte:?Dint"
	struct {
		byte_t byte;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, UNPuB ":putc", &args))
		goto err;
/*[[[end]]]*/
	result = DeeFile_Putc(self, (int)(unsigned int)args.byte);
	if unlikely(result == GETC_ERR)
		goto err;
	return_bool(result != GETC_EOF);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_getutf8(DeeObject *self, size_t argc, DeeObject *const *argv) {
	uint32_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("getutf8", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "getutf8");
/*[[[end]]]*/
	result = DeeFile_GetUtf8(self);
	if unlikely(result == (uint32_t)GETC_ERR)
		goto err;
	if (result == (uint32_t)GETC_EOF)
		return DeeString_NewEmpty();
	return DeeString_Chr(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_ungetutf8(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("ungetutf8", params: "
	uint32_t ch;
", docStringPrefix: "file");]]]*/
#define file_ungetutf8_params "ch:?Dint"
	struct {
		uint32_t ch;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, UNPu32 ":ungetutf8", &args))
		goto err;
/*[[[end]]]*/
	result = DeeFile_UngetUtf8(self, args.ch);
	if unlikely(result == GETC_ERR)
		goto err;
	return_bool(result != GETC_EOF);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_pututf8(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *str;
	char const *utf8;
	size_t written;
	DeeArg_Unpack1(err, argc, argv, "pututf8", &str);
	if (DeeObject_AssertTypeExact(str, &DeeString_Type))
		goto err;
	utf8 = DeeString_AsUtf8(str);
	if unlikely(!utf8)
		goto err;
	written = DeeFile_Write(self, utf8, WSTR_LENGTH(utf8));
	if unlikely(written == (size_t)-1)
		goto err;
	ASSERT(written <= WSTR_LENGTH(utf8));
	return_bool(written >= WSTR_LENGTH(utf8));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_size(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_pos_t old_pos, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("size", params: "");]]]*/
	DeeArg_Unpack0(err, argc, argv, "size");
/*[[[end]]]*/
	old_pos = DeeFile_Seek(self, 0, SEEK_CUR);
	if unlikely(old_pos == (Dee_pos_t)-1)
		goto err;
	result = DeeFile_Seek(self, 0, SEEK_END);
	if unlikely(result == (Dee_pos_t)-1)
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
	struct {
		size_t maxbytes;
		bool keeplf;
	} args;
	args.maxbytes = (size_t)-1;
	args.keeplf   = true;
	if (argc == 1 && DeeBool_Check(argv[0])) {
		args.keeplf = DeeBool_IsTrue(argv[0]);
	} else {
		if (DeeArg_UnpackStruct(argc, argv, "|" UNPxSIZ "b:readline", &args))
			goto err;
	}
	result = DeeFile_ReadLine(self, args.maxbytes, args.keeplf);
	if (result == ITER_DONE)
		result = DeeNone_NewRef();
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_readall(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("readall", params: "
	size_t maxbytes = (size_t)-1;
", docStringPrefix: "file");]]]*/
#define file_readall_params "maxbytes=!-1"
	struct {
		size_t maxbytes;
	} args;
	args.maxbytes = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, "|" UNPxSIZ ":readall", &args))
		goto err;
/*[[[end]]]*/
	return DeeFile_ReadBytes(self, args.maxbytes, true);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_preadall(DeeObject *self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("preadall", params: "
	Dee_pos_t pos;
	size_t maxbytes = (size_t)-1;
", docStringPrefix: "file");]]]*/
#define file_preadall_params "pos:?Dint,maxbytes=!-1"
	struct {
		Dee_pos_t pos;
		size_t maxbytes;
	} args;
	args.maxbytes = (size_t)-1;
	if (DeeArg_UnpackStruct(argc, argv, UNPuN(Dee_SIZEOF_POS_T) "|" UNPxSIZ ":preadall", &args))
		goto err;
/*[[[end]]]*/
	return DeeFile_PReadBytes(self, args.maxbytes, args.pos, true);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
file_mmap(DeeObject *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeMapFileObject *mapob;
	unsigned int mapflags;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("mapfile", params: "
	size_t minbytes  = 0;
	size_t maxbytes  = (size_t)-1;
	Dee_pos_t offset = (Dee_pos_t)-1;
	size_t nulbytes  = 0;
	bool readall     = false;
	bool mustmmap    = false;
	bool mapshared   = false;
", docStringPrefix: "file");]]]*/
#define file_mapfile_params "minbytes=!0,maxbytes=!-1,offset=!-1,nulbytes=!0,readall=!f,mustmmap=!f,mapshared=!f"
	struct {
		size_t minbytes;
		size_t maxbytes;
		Dee_pos_t offset;
		size_t nulbytes;
		bool readall;
		bool mustmmap;
		bool mapshared;
	} args;
	args.minbytes = 0;
	args.maxbytes = (size_t)-1;
	args.offset = (Dee_pos_t)-1;
	args.nulbytes = 0;
	args.readall = false;
	args.mustmmap = false;
	args.mapshared = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__minbytes_maxbytes_offset_nulbytes_readall_mustmmap_mapshared, "|" UNPuSIZ UNPxSIZ UNPxN(Dee_SIZEOF_POS_T) UNPuSIZ "bbb:mapfile", &args))
		goto err;
/*[[[end]]]*/
	mapflags = 0;

	/* Package flags */
	if (args.readall)
		mapflags |= DEE_MAPFILE_F_READALL;
	if (args.mustmmap)
		mapflags |= DEE_MAPFILE_F_MUSTMMAP;
	if (args.mapshared)
		mapflags |= DEE_MAPFILE_F_MUSTMMAP | DEE_MAPFILE_F_MAPSHARED;
	mapob = DeeObject_MALLOC(DREF DeeMapFileObject);
	if unlikely(!mapob)
		goto err;

	/* Create the actual file mapping */
	if unlikely(DeeMapFile_InitFile(&mapob->mf_map, self, args.offset,
	                                args.minbytes, args.maxbytes, args.nulbytes,
	                                mapflags))
		goto err_r;
	mapob->mf_rsize = DeeMapFile_GetSize(&mapob->mf_map) + args.nulbytes;
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
	              "(" file_read_params ")->?DBytes\n"
	              "Read and return at most @maxbytes of data from the file stream. "
	              /**/ "When @readall is ?t, keep on reading data until the buffer is full, or the "
	              /**/ "read-callback returns $0, rather than until it returns something other than "
	              /**/ "the internal buffer size used when reading data."),
	TYPE_METHOD("readall", &file_readall,
	            "(" file_readall_params ")->?DBytes\n"
	            "Alias for ${this.read(maxbytes, true)}"),
	TYPE_KWMETHOD("readinto", &file_readinto,
	              "(" file_readinto_params ")->?Dint\n"
	              "Read data into the given buffer @dst and return the number of bytes read. "
	              /**/ "When @readall is ?t, keep on reading data until the buffer is full, or the "
	              /**/ "read-callback returns $0, rather than until it returns something other than "
	              /**/ "the requested read size."),
	TYPE_KWMETHOD("write", &file_write,
	              "(" file_write_params ")->?Dint\n"
	              "Write @data to the file stream and return the actual number of bytes written. "
	              /**/ "When @writeall is ?t, keep writing data until the write-callback "
	              /**/ "returns $0 or until all data has been written, rather than invoke "
	              /**/ "the write-callback only a single time."),
	TYPE_KWMETHOD("pread", &file_pread,
	              "(" file_pread_params ")->?DBytes\n"
	              "Similar to ?#read, but read data from a given file-offset "
	              /**/ "@pos, rather than from the current file position"),
	TYPE_METHOD("preadall", &file_preadall,
	            "(" file_preadall_params ")->?DBytes\n"
	            "Alias for ${this.pread(pos, maxbytes, true)}"),
	TYPE_KWMETHOD("preadinto", &file_preadinto,
	              "(" file_preadinto_params ")->?DBytes\n"
	              "Similar to ?#readinto, but read data from a given file-offset "
	              /**/ "@pos, rather than from the current file position"),
	TYPE_KWMETHOD("pwrite", &file_pwrite,
	              "(" file_pwrite_params ")->?Dint\n"
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
	            "(" file_ungetc_params ")->?Dbool\n"
	            "Unget a given character @ch to be re-read the next time ?#getc or ?#read is called. "
	            /**/ "If the file's start has already been reached, ?f is returned and the character "
	            /**/ "will not be re-read from this file."),
	TYPE_METHOD("putc", &file_putc,
	            "(" file_putc_params ")->?Dbool\n"
	            "Append a single @byte at the end of @this File, returning ?t on "
	            /**/ "success, or ?f if the file has entered an end-of-file state."),

	/* Unicode (utf-8) read/write functions */
	TYPE_METHOD("getutf8", &file_getutf8,
	            "->?Dstring\n"
	            "Read and return a single unicode character (utf-8) from then "
	            /**/ "file, or return $\"\" if the file's end has been reached."),
	TYPE_METHOD("ungetutf8", &file_ungetutf8,
	            "(" file_ungetutf8_params ")->?Dbool\n"
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
	            "(maxbytes=!A!Dint!PSIZE_MAX,keeplf=!t)->?X2?DBytes?N\n"
	            "Read one line from the file stream, but read at most @maxbytes bytes.\n"
	            "When @keeplf is ?f, strip the trailing linefeed from the returned ?DBytes object.\n"
	            "Once EOF is reached, return ?N instead."),

	/* mmap support */
	TYPE_KWMETHOD("mmap", &file_mmap,
	              "(" file_mapfile_params ")->?DBytes\n"
	              "#pminbytes{The min number of bytes (excluding @nulbytes) that should be mapped "
	              /*      */ "starting at @offset. If the file is smaller than this, or indicates EOF before "
	              /*      */ "this number of bytes has been reached, nul bytes are mapped for its remainder.}"
	              "#pmaxbytes{The max number of bytes (excluding @nulbytes) that should be mapped starting "
	              /*      */ "at @offset. If the file is smaller than this, or indicates EOF before this "
	              /*      */ "number of bytes has been reached, simply stop there.}"
	              "#poffset{Starting offset of mapping (absolute), or ${-1} to use ?#pos}"
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
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_KWMETHOD("readat", &file_pread,
	              "(" file_pread_params ")->?DBytes\n"
	              "Deprecated alias for ?#pread"),
	TYPE_KWMETHOD("writeat", &file_pwrite,
	              "(" file_pwrite_params ")->?Dint\n"
	              "Deprecated alias for ?#pwrite"),
	TYPE_METHOD("readallat", &file_preadall,
	            "(" file_preadall_params ")->?DBytes\n"
	            "Deprecated alias for ?#preadall"),
	TYPE_KWMETHOD("setpos", &file_seek,
	              "(pos:?Dint)->?Dint\n"
	              "Deprecated alias for ?#seek"),
	TYPE_METHOD("flush", &file_sync,
	            "()\n"
	            "Deprecated alias for ?#sync"),
	TYPE_KWMETHOD("puts", &file_write,
	              "(data:?DBytes)->?Dint\n"
	              "Deprecated alias for ?#write"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

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
	Dee_pos_t result;
	result = DeeFile_Tell((DeeObject *)self);
	if unlikely(result == (Dee_pos_t)-1)
		goto err;
	return DeeInt_NewUInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
file_pos_del(DeeFileObject *__restrict self) {
	if unlikely(DeeFile_Rewind((DeeObject *)self) == (Dee_pos_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
file_pos_set(DeeFileObject *self, DeeObject *value) {
	Dee_pos_t newpos;
	if (DeeObject_AsUInt64(value, &newpos))
		goto err;
	if unlikely(DeeFile_SetPos((DeeObject *)self, newpos) == (Dee_pos_t)-1)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE struct type_getset tpconst file_getsets[] = {
	TYPE_GETSET("pos", &file_pos_get, &file_pos_del, &file_pos_set,
	            "->?Dint\n"
	            "Control the current file position"),

	/* Maintain at least a tiny bit of compatibility to the Iterator interface... */
	TYPE_GETTER(STR_seq, &DeeObject_NewRef, "->?DFile"),

	TYPE_GETSET_END
};


PRIVATE struct type_seq file_seq = {
	/* .tp_iter = */ &DeeObject_NewRef
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
       * `Sequence' already implements that operator as a union (in the case
       * of file: of all lines in either file).
       * Besides: A lot of sequence functions expect to be able to re-run
       *          iterators multiple times, yet file iterators expect the
       *          user to rewind the file before they can be iterated again,
       *          meaning that in that respect, `File' isn't 100% compliant
       *          to the `Sequence' interface, meaning we're kind-of not
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
					/* .tp_ctor      = */ (Dee_funptr_t)&DeeNone_OperatorCtor,
					/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
					/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
					/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
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
			/* .tp_visit         = */ NULL,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ &file_math,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ &file_seq,
		/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&file_next,
		/* .tp_iterator      = */ NULL,
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

#ifndef __INTELLISENSE__
#include "file-operator.c.inl"
#define DEFINE_TYPED_OPERATORS
#include "file-operator.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_FILE_C */
