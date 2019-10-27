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
#ifndef GUARD_DEEMON_SYSTEM_SYSTEM_UNIX_C_INL
#define GUARD_DEEMON_SYSTEM_SYSTEM_UNIX_C_INL 1

#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#ifndef PATH_MAX
#ifdef PATHMAX
#   define PATH_MAX PATHMAX
#elif defined(MAX_PATH)
#   define PATH_MAX MAX_PATH
#elif defined(MAXPATH)
#   define PATH_MAX MAXPATH
#else
#   define PATH_MAX 260
#endif
#endif /* !PATH_MAX */

DECL_BEGIN

/* Figure out how to implement `DeeUnixSystem_Readlink()' */
#undef DeeUnixSystem_Readlink_USE_WINDOWS
#undef DEEUNIXSYSTEM_READLINK_USE_FREADLINKAT
#undef DeeUnixSystem_Readlink_USE_READLINK
#undef DeeUnixSystem_Readlink_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define DeeUnixSystem_Readlink_USE_WINDOWS 1
#elif defined(CONFIG_HAVE_freadlinkat) && defined(AT_READLINK_REQSIZE) && defined(AT_FDCWD) && 0
#define DEEUNIXSYSTEM_READLINK_USE_FREADLINKAT 1 /* TODO */
#elif defined(CONFIG_HAVE_readlink) && defined(CONFIG_HAVE_lstat)
#define DeeUnixSystem_Readlink_USE_READLINK 1
#else
#define DeeUnixSystem_Readlink_USE_STUB 1
#endif


/* Read the contexts of the given link.
 * If the host doesn't support symbolic links, throw an error.
 * @return: 0 / * :        Success.
 * @return: -1 / NULL:     Error.
 * @return: 1 / ITER_DONE: The specified file isn't a symbolic link. */
PUBLIC WUNUSED int DCALL
DeeUnixSystem_Printlink(struct unicode_printer *__restrict printer,
                        /*String*/ DeeObject *__restrict filename) {
#ifdef DeeUnixSystem_Readlink_USE_WINDOWS
	/* TODO */
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_Readlink_USE_WINDOWS */

#if defined(DEEUNIXSYSTEM_READLINK_USE_FREADLINKAT) || \
    defined(DeeUnixSystem_Readlink_USE_READLINK)
	int result;
	char const *utf8_filename;
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		return -1;
	result = DeeUnixSystem_PrintlinkString(printer, utf8_filename);
	return result;
#endif /* DEEUNIXSYSTEM_READLINK_USE_FREADLINKAT || DeeUnixSystem_Readlink_USE_READLINK */

#ifdef DeeUnixSystem_Readlink_USE_STUB
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_Readlink_USE_STUB */
}

PUBLIC WUNUSED int DCALL
DeeUnixSystem_PrintlinkString(struct unicode_printer *__restrict printer,
                              /*utf-8*/ char const *filename) {
#ifdef DeeUnixSystem_Readlink_USE_WINDOWS
	/* TODO */
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_Readlink_USE_WINDOWS */

#ifdef DEEUNIXSYSTEM_READLINK_USE_FREADLINKAT
	/* TODO */
	(void)printer;
	(void)filename;
	return 1;
#endif /* DEEUNIXSYSTEM_READLINK_USE_FREADLINKAT */

#ifdef DeeUnixSystem_Readlink_USE_READLINK
	char *buffer, *new_buffer;
	int error;
	size_t bufsize, new_size;
	dssize_t req_size;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	bufsize = PATH_MAX;
	buffer  = unicode_printer_alloc_utf8(&printer, bufsize);
	if unlikely(!buffer)
		goto err;
	for (;;) {
		struct stat st;
		if (DeeThread_CheckInterrupt())
			goto err_buf;
		DBG_ALIGNMENT_DISABLE();
		req_size = readlink(filename, buffer, bufsize + 1);
		if unlikely(req_size < 0) {
handle_error:
			DBG_ALIGNMENT_ENABLE();
			error = errno;
			DeeError_SysThrowf(&DeeError_FSError, error,
			                   "Failed to read symbolic link %q",
			                   filename);
			goto err_buf;
		}
		DBG_ALIGNMENT_ENABLE();
		if ((size_t)req_size <= bufsize)
			break;
		DBG_ALIGNMENT_DISABLE();
		if (lstat(filename, &st))
			goto handle_error;
		DBG_ALIGNMENT_ENABLE();
		/* Ensure that this is still a symbolic link. */
		if (!S_ISLNK(st.st_mode)) {
			unicode_printer_free_utf8(&printer, buffer);
			unicode_printer_fini(&printer);
			return ITER_DONE;
		}
		new_size = (size_t)st.st_size;
		if (new_size <= bufsize)
			break; /* Shouldn't happen, but might due to race conditions? */
		new_buffer = unicode_printer_resize_utf8(&printer, buffer, new_size);
		if unlikely(!new_buffer)
			goto err_buf;
		buffer  = new_buffer;
		bufsize = new_size;
	}
	/* Release unused data. */
	if (unicode_printer_confirm_utf8(&printer, buffer, (size_t)req_size) < 0)
		goto err_buf;
	bufsize = UNICODE_PRINTER_LENGTH(&printer);
	while (bufsize && UNICODE_PRINTER_GETCHAR(&printer, bufsize - 1) != '/')
		--bufsize;
	while (bufsize && UNICODE_PRINTER_GETCHAR(&printer, bufsize - 1) == '/')
		--bufsize;
	UNICODE_PRINTER_SETCHAR(&printer, bufsize, '/');
	unicode_printer_truncate(&printer, bufsize + 1);
	return unicode_printer_pack(&printer);
err_buf:
	unicode_printer_free_utf8(&printer, buffer);
err:
	unicode_printer_fini(&printer);
	return NULL;
#endif /* DeeUnixSystem_Readlink_USE_READLINK */

#ifdef DeeUnixSystem_Readlink_USE_STUB
	(void)printer;
	(void)filename;
	return 1;
#endif /* DeeUnixSystem_Readlink_USE_STUB */
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeUnixSystem_Readlink(/*String*/ DeeObject *__restrict filename) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeUnixSystem_Printlink(&printer, filename);
	if (error == 0)
		return unicode_printer_pack(&printer);
	unicode_printer_fini(&printer);
	if (error > 0)
		return ITER_DONE;
	return NULL;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeUnixSystem_ReadlinkString(/*utf-8*/ char const *filename) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeUnixSystem_PrintlinkString(&printer, filename);
	if (error == 0)
		return unicode_printer_pack(&printer);
	unicode_printer_fini(&printer);
	if (error > 0)
		return ITER_DONE;
	return NULL;
}




/* Figure out how to implement `DeeSystem_GetFilenameOfFD()' */
#undef DeeSystem_PrintFilenameOfFD_USE_NT_HANDLE
#undef DeeSystem_PrintFilenameOfFD_USE_PROCFS
#undef DeeSystem_PrintFilenameOfFD_USE_STUB
#if defined(CONFIG_HOST_WINDOWS) && defined(CONFIG_HAVE_get_osfhandle)
#define DeeSystem_PrintFilenameOfFD_USE_NT_HANDLE 1
#elif !defined(DeeUnixSystem_Readlink_USE_STUB)
#define DeeSystem_PrintFilenameOfFD_USE_PROCFS 1
#else
#define DeeSystem_PrintFilenameOfFD_USE_STUB 1
#endif

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


#ifdef DeeSystem_PrintFilenameOfFD_USE_NT_HANDLE
PRIVATE ATTR_COLD int DCALL
err_PrintFilenameOfFD_BADF(int fd) {
	return DeeError_SysThrowf(&DeeError_SystemError, EBADF,
	                          "Bad file descriptor %d", fd);
}
#endif /* DeeSystem_PrintFilenameOfFD_USE_NT_HANDLE */

/* @return: 0:  Success.
 * @return: -1: Error. */
PUBLIC WUNUSED int DCALL
DeeSystem_PrintFilenameOfFD(struct unicode_printer *__restrict printer, int fd) {
#ifdef DeeSystem_PrintFilenameOfFD_USE_NT_HANDLE
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
#endif /* DeeSystem_PrintFilenameOfFD_USE_NT_HANDLE */

#ifdef DeeSystem_PrintFilenameOfFD_USE_PROCFS
	char buf[64];
	Dee_sprintf(buf, "/proc/self/fd/%d", fd);
	return DeeUnixSystem_PrintlinkString(printer, buf);
#endif /* DeeSystem_PrintFilenameOfFD_USE_PROCFS */

#ifdef DeeSystem_PrintFilenameOfFD_USE_STUB
	(void)printer;
	(void)fd;
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "Unsupported function: GetFilenameOfFD");
#endif /* DeeSystem_PrintFilenameOfFD_USE_STUB */
}






DECL_END

#endif /* !GUARD_DEEMON_SYSTEM_SYSTEM_UNIX_C_INL */
