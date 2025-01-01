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
#ifndef GUARD_DEEMON_SYSTEM_SYSTEM_UNIX_C_INL
#define GUARD_DEEMON_SYSTEM_SYSTEM_UNIX_C_INL 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifndef PATH_MAX
#ifdef PATHMAX
#define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#define PATH_MAX MAXPATH
#else /* ... */
#define PATH_MAX 260
#endif /* !... */
#endif /* !PATH_MAX */

DECL_BEGIN


/* Throw an exception alongside an errno error-code `error'
 * When `tp' is `NULL', automatically select an appropriate
 * error type based on the value of `error' */
PUBLIC ATTR_COLD NONNULL((3)) int
(DCALL DeeUnixSystem_VThrowErrorf)(DeeTypeObject *tp, /*errno_t*/ int errno_value,
                                   char const *__restrict format, va_list args) {
	int result;
	if (!tp) {
		/* Automatically determine the error type to-be thrown. */
		DeeSystem_IF_E1(errno_value, ENOENT, /*                */ { tp = &DeeError_FileNotFound; goto got_tp; });
		DeeSystem_IF_E1(errno_value, EEXIST, /*                */ { tp = &DeeError_FileExists; goto got_tp; });
		DeeSystem_IF_E1(errno_value, EBADF, /*                 */ { tp = &DeeError_FileClosed; goto got_tp; });
		DeeSystem_IF_E1(errno_value, EACCES, /*                */ { tp = &DeeError_FileAccessError; goto got_tp; });
		DeeSystem_IF_E1(errno_value, ENOMEM, /*                */ { tp = &DeeError_NoMemory; goto got_tp; });
		DeeSystem_IF_E3(errno_value, ENOSYS, ENOTSUP, EOPNOTSUPP, { tp = &DeeError_UnsupportedAPI; goto got_tp; });
		DeeSystem_IF_E1(errno_value, EINVAL, /*                */ { tp = &DeeError_ValueError; goto got_tp; });
		DeeSystem_IF_E2(errno_value, EROFS, ETXTBSY, /*        */ { tp = &DeeError_ReadOnlyFile; goto got_tp; });
		DeeSystem_IF_E1(errno_value, EFBIG, /*                 */ { tp = &DeeError_IntegerOverflow; goto got_tp; });
		DeeSystem_IF_E1(errno_value, EINTR, /*                 */ { tp = &DeeError_Interrupt; goto got_tp; });
		DeeSystem_IF_E1(errno_value, EBUSY, /*                 */ { tp = &DeeError_BusyFile; goto got_tp; });
		DeeSystem_IF_E1(errno_value, ENOTDIR, /*               */ { tp = &DeeError_NoDirectory; goto got_tp; });
		DeeSystem_IF_E1(errno_value, ENOTEMPTY, /*             */ { tp = &DeeError_DirectoryNotEmpty; goto got_tp; });
		DeeSystem_IF_E1(errno_value, EXDEV, /*                 */ { tp = &DeeError_CrossDeviceLink; goto got_tp; });
		DeeSystem_IF_E1(errno_value, ENOLINK, /*               */ { tp = &DeeError_NoSymlink; goto got_tp; });
		/* Fallback: Just use a SystemError */
		tp = &DeeError_SystemError;
		goto got_tp;
	}

	/* Check for error types derived from `errors.SystemError' */
got_tp:
	if (DeeType_Check(tp) &&
	    DeeType_Extends(tp, &DeeError_SystemError)) {
		DREF DeeSystemErrorObject *error;
		DREF DeeStringObject *message;
		error = (DREF DeeSystemErrorObject *)DeeObject_MALLOC(DeeSystemErrorObject);
		if unlikely(!error)
			goto err;
		message = (DREF DeeStringObject *)DeeString_VNewf(format, args);
		if unlikely(!message) {
			DeeObject_FREE(error);
			goto err;
		}
		error->e_message = message; /* Inherit reference */
		error->e_inner   = NULL;
		error->se_errno  = errno_value;
#ifdef CONFIG_HOST_WINDOWS
		error->se_lasterror = DeeNTSystem_TranslateNtError(errno_value);
#endif /* CONFIG_HOST_WINDOWS */
		DeeObject_Init(error, tp);
		result = DeeError_Throw((DeeObject *)error);
		Dee_Decref(error);
	} else {
		/* Unlikely to happen: Just throw a regular, old error. */
		result = DeeError_VThrowf(tp, format, args);
	}
done:
	return result;
err:
	result = -1;
	goto done;
}

PUBLIC ATTR_COLD NONNULL((3)) int
(DeeUnixSystem_ThrowErrorf)(DeeTypeObject *tp, /*errno_t*/ int errno_value,
                            char const *__restrict format, ...) {
	va_list args;
	int result;
	va_start(args, format);
	result = DeeUnixSystem_VThrowErrorf(tp, errno_value, format, args);
	va_end(args);
	return result;
}




/* Figure out how to implement `DeeUnixSystem_ReadLink()' */
#undef DeeUnixSystem_ReadLink_USE_WINDOWS
#undef DeeUnixSystem_ReadLink_USE_freadlinkat
#undef DeeUnixSystem_ReadLink_USE_readlink
#undef DeeUnixSystem_ReadLink_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define DeeUnixSystem_ReadLink_USE_WINDOWS
#elif defined(CONFIG_HAVE_freadlinkat) && defined(AT_READLINK_REQSIZE) && defined(AT_FDCWD) && 0
#define DeeUnixSystem_ReadLink_USE_freadlinkat /* TODO */
#elif defined(CONFIG_HAVE_readlink) && defined(CONFIG_HAVE_lstat)
#define DeeUnixSystem_ReadLink_USE_readlink
#else /* ... */
#define DeeUnixSystem_ReadLink_USE_STUB
#endif /* !... */


/* Read the contexts of the given link.
 * If the host doesn't support symbolic links, throw an error.
 * @return: 0 / * :        Success.
 * @return: -1 / NULL:     Error.
 * @return: 1 / ITER_DONE: The specified file isn't a symbolic link. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeUnixSystem_PrintLink(struct unicode_printer *__restrict printer,
                        /*String*/ DeeObject *__restrict filename) {
#ifdef DeeUnixSystem_ReadLink_USE_WINDOWS
	/* TODO */
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_ReadLink_USE_WINDOWS */

#if (defined(DeeUnixSystem_ReadLink_USE_freadlinkat) || \
     defined(DeeUnixSystem_ReadLink_USE_readlink))
	int result;
	char const *utf8_filename;
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		goto err;
	result = DeeUnixSystem_PrintLinkString(printer, utf8_filename);
	return result;
err:
	return -1;
#endif /* DeeUnixSystem_ReadLink_USE_freadlinkat || DeeUnixSystem_ReadLink_USE_readlink */

#ifdef DeeUnixSystem_ReadLink_USE_STUB
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_ReadLink_USE_STUB */
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeUnixSystem_PrintLinkString(struct unicode_printer *__restrict printer,
                              /*utf-8*/ char const *filename) {
#ifdef DeeUnixSystem_ReadLink_USE_WINDOWS
	/* TODO */
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_ReadLink_USE_WINDOWS */

#ifdef DeeUnixSystem_ReadLink_USE_freadlinkat
	/* TODO */
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_ReadLink_USE_freadlinkat */

#ifdef DeeUnixSystem_ReadLink_USE_readlink
	char *buffer, *new_buffer;
	int error;
	size_t bufsize, new_size;
	size_t req_size;
	bufsize = PATH_MAX;
	buffer  = unicode_printer_alloc_utf8(printer, bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		struct stat st;
		if (DeeThread_CheckInterrupt())
			goto err_buf;
		DBG_ALIGNMENT_DISABLE();
		req_size = (size_t)readlink(filename, buffer, bufsize + 1);
		if unlikely(req_size == (size_t)-1) {
handle_error:
			DBG_ALIGNMENT_ENABLE();
			error = DeeSystem_GetErrno();
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to read symbolic link %q",
			                          filename);
			goto err_buf;
		}
		DBG_ALIGNMENT_ENABLE();
		if (req_size <= bufsize)
			break;
		DBG_ALIGNMENT_DISABLE();
		if (lstat(filename, &st))
			goto handle_error;
		DBG_ALIGNMENT_ENABLE();
		/* Ensure that this is still a symbolic link. */
		if (!S_ISLNK(st.st_mode)) {
			unicode_printer_free_utf8(printer, buffer);
			return 1;
		}
		new_size = (size_t)st.st_size;
		if (new_size <= bufsize)
			break; /* Shouldn't happen, but might due to race conditions? */
		new_buffer = unicode_printer_resize_utf8(printer, buffer, new_size);
		if unlikely(!new_buffer)
			goto err_buf;
		buffer  = new_buffer;
		bufsize = new_size;
	}
	/* Commit buffer data at the end of the printer. */
	if unlikely(unicode_printer_commit_utf8(printer, buffer, req_size) < 0)
		goto err_buf;
	return 0;
err_buf:
	unicode_printer_free_utf8(printer, buffer);
err:
	return -1;
#endif /* DeeUnixSystem_ReadLink_USE_readlink */

#ifdef DeeUnixSystem_ReadLink_USE_STUB
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_ReadLink_USE_STUB */
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeUnixSystem_ReadLink(/*String*/ DeeObject *__restrict filename) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeUnixSystem_PrintLink(&printer, filename);
	if (error == 0)
		return unicode_printer_pack(&printer);
	unicode_printer_fini(&printer);
	if (error > 0)
		return ITER_DONE;
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeUnixSystem_ReadLinkString(/*utf-8*/ char const *filename) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeUnixSystem_PrintLinkString(&printer, filename);
	if (error == 0)
		return unicode_printer_pack(&printer);
	unicode_printer_fini(&printer);
	if (error > 0)
		return ITER_DONE;
	return NULL;
}




/* Figure out how to implement `DeeSystem_GetFilenameOfFD()'
 * NOTE: This listing is duplicated in `src/dex/posix/p-realpath.c.inl' */
#undef DeeSystem_PrintFilenameOfFD_USE_get_osfhandle__AND__PrintFilenameOfHandle
#undef DeeSystem_PrintFilenameOfFD_USE_frealpath
#undef DeeSystem_PrintFilenameOfFD_USE_readlink_procfs
#undef DeeSystem_PrintFilenameOfFD_USE_STUB
#if defined(CONFIG_HOST_WINDOWS) && defined(CONFIG_HAVE_get_osfhandle)
#define DeeSystem_PrintFilenameOfFD_USE_get_osfhandle__AND__PrintFilenameOfHandle
#elif defined(CONFIG_HAVE_frealpath)
#define DeeSystem_PrintFilenameOfFD_USE_frealpath
#elif defined(CONFIG_HAVE_frealpath4)
#define frealpath(fd, resolved, buflen) frealpath4(fd, resolved, buflen, 0)
#define DeeSystem_PrintFilenameOfFD_USE_frealpath
#elif !defined(DeeUnixSystem_ReadLink_USE_STUB)
#define DeeSystem_PrintFilenameOfFD_USE_readlink_procfs
#else /* ... */
#define DeeSystem_PrintFilenameOfFD_USE_STUB
#endif /* !... */

/* Determine the filename from a file descriptor, as returned by `open()'
 * If the host doesn't support FD-based file descriptors, throw an error. */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeSystem_GetFilenameOfFD(int fd) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (DeeSystem_PrintFilenameOfFD(&printer, fd))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}


#ifdef DeeSystem_PrintFilenameOfFD_USE_get_osfhandle__AND__PrintFilenameOfHandle
PRIVATE ATTR_COLD int DCALL
err_PrintFilenameOfFD_BADF(int fd) {
	return DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, EBADF,
	                                 "Bad file descriptor %d", fd);
}
#endif /* DeeSystem_PrintFilenameOfFD_USE_get_osfhandle__AND__PrintFilenameOfHandle */

#ifdef DeeSystem_PrintFilenameOfFD_USE_frealpath
#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */
#endif /* DeeSystem_PrintFilenameOfFD_USE_frealpath */

/* @return: 0:  Success.
 * @return: -1: Error. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeSystem_PrintFilenameOfFD(struct unicode_printer *__restrict printer, int fd) {
#ifdef DeeSystem_PrintFilenameOfFD_USE_get_osfhandle__AND__PrintFilenameOfHandle
	int result;
	HANDLE h;
	h = (HANDLE)get_osfhandle(fd);
	if (h == INVALID_HANDLE_VALUE) {
		err_PrintFilenameOfFD_BADF(fd);
		result = -1;
	} else {
		result = DeeNTSystem_PrintFilenameOfHandle(printer, (void *)h);
	}
	return result;
#endif /* DeeSystem_PrintFilenameOfFD_USE_get_osfhandle__AND__PrintFilenameOfHandle */

#ifdef DeeSystem_PrintFilenameOfFD_USE_frealpath
	char *buf;
	size_t buflen = PATH_MAX;
	buf = unicode_printer_alloc_utf8(printer, PATH_MAX);
	if unlikely(!buf) {
err:
		return -1;
	}
again:
	DBG_ALIGNMENT_DISABLE();
	if (frealpath(fd, buf, buflen) != NULL) {
		size_t final_len;
		DBG_ALIGNMENT_ENABLE();
		final_len = strnlen(buf, buflen);
		if (final_len == buflen) {
			/* The entire buffer was filled, but there may be more data... */
			size_t new_buflen;
			char *newbuf;
#ifdef ERANGE
increase_buffer_size:
#endif /* ERANGE */
			new_buflen = buflen * 2;
			newbuf     = unicode_printer_tryresize_utf8(printer, buf, new_buflen);
			if unlikely(!newbuf) {
				new_buflen = buflen + 1;
				newbuf     = unicode_printer_resize_utf8(printer, buf, new_buflen);
				if unlikely(!newbuf) {
					unicode_printer_free_utf8(printer, buf);
					goto err;
				}
			}
			buf    = newbuf;
			buflen = new_buflen;
			goto again;
		}
		if unlikely(unicode_printer_commit_utf8(printer, buf, final_len) < 0)
			goto err;
		return 0;
	} else {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
		if (error == EINTR) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again;
		}
#endif /* EINTR */
#ifdef ERANGE
		if (error == ERANGE)
			goto increase_buffer_size;
#endif /* ERANGE */
#ifdef ENAMETOOLONG
#ifndef DeeUnixSystem_ReadLink_USE_STUB
		if (error != ENAMETOOLONG)
#define DeeSystem_PrintFilenameOfFD_USE_readlink_procfs
#endif /* !DeeUnixSystem_ReadLink_USE_STUB */
#endif /* ENAMETOOLONG */
		{
			return DeeUnixSystem_ThrowErrorf(NULL, error,
			                                 "Failed to determine filename of fd %d",
			                                 fd);
		}
	}
#endif /* DeeSystem_PrintFilenameOfFD_USE_frealpath */

#ifdef DeeSystem_PrintFilenameOfFD_USE_readlink_procfs
	{
		char buf[64];
		Dee_sprintf(buf, "/proc/self/fd/%d", fd);
		return DeeUnixSystem_PrintLinkString(printer, buf);
	}
#endif /* DeeSystem_PrintFilenameOfFD_USE_readlink_procfs */

#ifdef DeeSystem_PrintFilenameOfFD_USE_STUB
	(void)printer;
	(void)fd;
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "Unsupported function: GetFilenameOfFD");
#endif /* DeeSystem_PrintFilenameOfFD_USE_STUB */
}

DECL_END

#endif /* !GUARD_DEEMON_SYSTEM_SYSTEM_UNIX_C_INL */
