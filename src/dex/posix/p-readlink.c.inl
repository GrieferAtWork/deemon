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
#ifndef GUARD_DEX_POSIX_P_READLINK_C_INL
#define GUARD_DEX_POSIX_P_READLINK_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Figure out how to implement `readlink()' */
#undef posix_readlink_USE_nt_FReadLink
#undef posix_readlink_USE_freadlinkat
#undef posix_readlink_USE_wreadlink
#undef posix_readlink_USE_readlink
#undef posix_readlink_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define posix_readlink_USE_nt_FReadLink
#elif defined(CONFIG_HAVE_freadlinkat)
#define posix_readlink_USE_freadlinkat
#elif defined(CONFIG_HAVE_wreadlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_readlink_USE_wreadlink
#elif defined(CONFIG_HAVE_readlink)
#define posix_readlink_USE_readlink
#elif defined(CONFIG_HAVE_wreadlink)
#define posix_readlink_USE_wreadlink
#else /* ... */
#define posix_readlink_USE_STUB
#endif /* !... */



/* Figure out how to implement `freadlink()' */
#undef posix_freadlink_USE_nt_FReadLink
#undef posix_freadlink_USE_posix_readlink
#undef posix_freadlink_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define posix_freadlink_USE_nt_FReadLink
#elif !defined(posix_readlink_USE_STUB)
#define posix_freadlink_USE_posix_readlink
#else /* ... */
#define posix_freadlink_USE_STUB
#endif /* !... */



/* Figure out how to implement `readlinkat()' */
#undef posix_readlinkat_USE_freadlinkat
#undef posix_readlinkat_USE_readlinkat
#undef posix_readlinkat_USE_posix_readlink
#undef posix_readlinkat_USE_STUB
#if defined(CONFIG_HAVE_freadlinkat)
#define posix_readlinkat_USE_freadlinkat
#define posix_readlinkat_USE_posix_readlink
#elif defined(CONFIG_HAVE_readlinkat)
#define posix_readlinkat_USE_readlinkat
#define posix_readlinkat_USE_posix_readlink
#elif !defined(posix_readlink_USE_STUB)
#define posix_readlinkat_USE_posix_readlink
#else /* ... */
#define posix_readlinkat_USE_STUB
#endif /* !... */


/*[[[deemon import("rt.gen.dexutils").gw("readlink", "file:?Dstring->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_readlink_f_impl(DeeObject *file);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_readlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_READLINK_DEF { "readlink", (DeeObject *)&posix_readlink, MODSYM_FREADONLY, DOC("(file:?Dstring)->?Dstring") },
#define POSIX_READLINK_DEF_DOC(doc) { "readlink", (DeeObject *)&posix_readlink, MODSYM_FREADONLY, DOC("(file:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_readlink, &posix_readlink_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__file
#define DEFINED_kwlist__file
PRIVATE DEFINE_KWLIST(kwlist__file, { KEX("file", 0x1a11b2b3, 0x612e37678ce7db5b), KEND });
#endif /* !DEFINED_kwlist__file */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_readlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *file;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__file, "o:readlink", &args))
		goto err;
	return posix_readlink_f_impl(args.file);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_readlink_f_impl(DeeObject *file)
/*[[[end]]]*/
{
#ifdef posix_readlink_USE_nt_FReadLink
	DREF DeeObject *result;
	HANDLE hLinkFile;
	if (DeeObject_AssertTypeExact(file, &DeeString_Type))
		goto err;
	hLinkFile = DeeNTSystem_CreateFileNoATime(file, FILE_READ_DATA | FILE_READ_ATTRIBUTES,
	                                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
	                                          NULL, OPEN_EXISTING,
	                                          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
	                                          NULL);
	if unlikely(!hLinkFile)
		goto err;
	if unlikely(hLinkFile == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsNotDir(dwError)) {
#define NEED_err_nt_path_not_dir
			err_nt_path_not_dir(dwError, file);
			goto err;
		}
		if (DeeNTSystem_IsAccessDeniedError(dwError)) {
			/* Try to open the file one more time, only this
			 * time _only_ pass the READ_ATTRIBUTES capability. */
			hLinkFile = DeeNTSystem_CreateFileNoATime(file, FILE_READ_ATTRIBUTES,
			                                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			                                          NULL, OPEN_EXISTING,
			                                          FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
			                                          NULL);
			if unlikely(!hLinkFile)
				goto err;
			if unlikely(hLinkFile != INVALID_HANDLE_VALUE)
				goto ok_got_ownds_linkfd;
#define NEED_err_nt_path_no_access
			err_nt_path_no_access(dwError, file);
			goto err;
		}
		if (DeeNTSystem_IsFileNotFoundError(dwError)) {
#define NEED_err_nt_path_not_found
			err_nt_path_not_found(dwError, file);
			goto err;
		}
		DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
		                        "Failed to read symbolic link %r",
		                        file);
		goto err;
	}
ok_got_ownds_linkfd:
#define NEED_nt_FReadLink
	result = nt_FReadLink(hLinkFile, file, true);
	DBG_ALIGNMENT_DISABLE();
	(void)CloseHandle(hLinkFile);
	DBG_ALIGNMENT_ENABLE();
	return result;
err:
	return NULL;
#endif /* posix_readlink_USE_nt_FReadLink */

#if (defined(posix_readlink_USE_wreadlink) || \
     defined(posix_readlink_USE_readlink) ||  \
     defined(posix_readlink_USE_freadlinkat))
#ifdef posix_readlink_USE_wreadlink
	Dee_wchar_t const *wide_file;
	Dee_wchar_t *buffer, *new_buffer;
#else /* posix_readlink_USE_wreadlink */
	char const *utf8_file;
	char *buffer, *new_buffer;
#endif /* !posix_readlink_USE_wreadlink */
	int error;
	size_t bufsize, new_size;
	Dee_ssize_t req_size;
	if (DeeObject_AssertTypeExact(file, &DeeString_Type))
		goto err;
#ifdef posix_readlink_USE_wreadlink
	wide_file = DeeString_AsWide(file);
	if unlikely(!wide_file)
		goto err;
#else /* posix_readlink_USE_wreadlink */
	utf8_file = DeeString_AsUtf8(file);
	if unlikely(!utf8_file)
		goto err;
#endif /* !posix_readlink_USE_wreadlink */
#ifdef PATH_MAX
	bufsize = PATH_MAX;
#else /* PATH_MAX */
	bufsize = 256;
#endif /* !PATH_MAX */
#ifdef posix_readlink_USE_wreadlink
	buffer = DeeString_NewWideBuffer(bufsize);
#else /* posix_readlink_USE_wreadlink */
	buffer = (char *)DeeString_New1ByteBuffer(bufsize);
#endif /* !posix_readlink_USE_wreadlink */
	if unlikely(!buffer)
		goto err;
	for (;;) {
EINTR_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
#ifdef posix_readlink_USE_freadlinkat
		req_size = freadlinkat(AT_FDCWD, (char *)utf8_file, buffer, bufsize + 1, AT_READLINK_REQSIZE);
#elif defined(posix_readlink_USE_wreadlink)
		req_size = wreadlink((wchar_t *)wide_file, (wchar_t *)buffer, bufsize + 1);
#else /* ... */
		req_size = readlink((char *)utf8_file, buffer, bufsize + 1);
#endif /* !... */
		if unlikely(req_size < 0) {
			error = DeeSystem_GetErrno();
			DBG_ALIGNMENT_ENABLE();
			EINTR_HANDLE(error, again, err_buffer);
#ifdef EACCES
			if (error == EACCES) {
#define NEED_err_unix_path_no_access
				err_unix_path_no_access(error, file);
				goto err_buffer;
			}
#endif /* EACCES */
#ifdef ENOTDIR
			if (error == ENOTDIR) {
#define NEED_err_unix_path_not_dir
				err_unix_path_not_dir(error, file);
				goto err_buffer;
			}
#endif /* ENOTDIR */
#ifdef ENOENT
			if (error == ENOENT) {
#define NEED_err_unix_path_not_found
				err_unix_path_not_found(error, file);
				goto err_buffer;
			}
#endif /* ENOENT */
#ifdef EINVAL
			if (error == EINVAL) {
				DeeUnixSystem_ThrowErrorf(&DeeError_NoSymlink, error,
				                          "Path %r is not a symbolic link",
				                          file);
				goto err_buffer;
			}
#endif /* EINVAL */
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to read symbolic link %r",
			                          file);
			goto err_buffer;
		}
		DBG_ALIGNMENT_ENABLE();
#ifdef posix_readlink_USE_freadlinkat
		if ((size_t)req_size <= (bufsize + 1)) {
			--req_size; /* Account for trailing NUL */
			break;
		}
		new_size = (size_t)req_size - 1;
#else /* posix_readlink_USE_freadlinkat */
		if ((size_t)req_size <= bufsize)
			break;
		new_size = bufsize * 2;
#endif /* !posix_readlink_USE_freadlinkat */
#ifdef posix_readlink_USE_wreadlink
		new_buffer = DeeString_ResizeWideBuffer(buffer, bufsize);
#else /* posix_readlink_USE_wreadlink */
		new_buffer = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buffer, bufsize);
#endif /* !posix_readlink_USE_wreadlink */
		if unlikely(!new_buffer)
			goto err_buffer;
		buffer  = new_buffer;
		bufsize = new_size;
	}

	/* Release unused data. */
#ifdef posix_readlink_USE_wreadlink
	buffer = DeeString_TruncateWideBuffer(buffer, (size_t)req_size);
	return DeeString_PackWideBuffer(buffer, STRING_ERROR_FREPLAC);
#else /* posix_readlink_USE_wreadlink */
	buffer = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buffer, (size_t)req_size);
	return DeeString_PackUtf8Buffer((uint8_t *)buffer, STRING_ERROR_FREPLAC);
#endif /* !posix_readlink_USE_wreadlink */
err_buffer:
#ifdef posix_readlink_USE_wreadlink
	DeeString_FreeWideBuffer(buffer);
#else /* posix_readlink_USE_wreadlink */
	DeeString_Free1ByteBuffer((uint8_t *)buffer);
#endif /* !posix_readlink_USE_wreadlink */
err:
	return NULL;
#endif /* posix_readlink_USE_wreadlink || posix_readlink_USE_readlink || posix_readlink_USE_freadlinkat */

#ifdef posix_readlink_USE_STUB
	(void)file;
#define NEED_posix_err_unsupported
	posix_err_unsupported("readlink");
	return NULL;
#endif /* posix_readlink_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("freadlink", "fd:?X2?DFile?Dint->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL posix_freadlink_f_impl(DeeObject *fd);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_freadlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FREADLINK_DEF { "freadlink", (DeeObject *)&posix_freadlink, MODSYM_FREADONLY, DOC("(fd:?X2?DFile?Dint)->?Dstring") },
#define POSIX_FREADLINK_DEF_DOC(doc) { "freadlink", (DeeObject *)&posix_freadlink, MODSYM_FREADONLY, DOC("(fd:?X2?DFile?Dint)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_freadlink, &posix_freadlink_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__fd
#define DEFINED_kwlist__fd
PRIVATE DEFINE_KWLIST(kwlist__fd, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEND });
#endif /* !DEFINED_kwlist__fd */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_freadlink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *fd;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__fd, "o:freadlink", &args))
		goto err;
	return posix_freadlink_f_impl(args.fd);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1))DREF DeeObject *DCALL posix_freadlink_f_impl(DeeObject *fd)
/*[[[end]]]*/
{
#ifdef posix_freadlink_USE_nt_FReadLink
	DREF DeeObject *result;
	HANDLE hLinkFile;
	hLinkFile = (HANDLE)DeeNTSystem_GetHandle(fd);
	if (hLinkFile == INVALID_HANDLE_VALUE) {
		DREF DeeObject *filename;
		if (!DeeError_Catch(&DeeError_AttributeError) &&
		    !DeeError_Catch(&DeeError_NotImplemented) &&
		    !DeeError_Catch(&DeeError_FileClosed))
			goto err;
		/* Use the filename of the given `fd'. */
		filename = DeeFile_Filename(fd);
		if unlikely(!filename)
			goto err;
		result = posix_readlink_f_impl(filename);
		Dee_Decref(filename);
	} else {
#define NEED_nt_FReadLink
		result = nt_FReadLink(hLinkFile, fd, true);
	}
	return result;
err:
	return NULL;
#endif /* posix_freadlink_USE_nt_FReadLink */

#ifdef posix_freadlink_USE_posix_readlink
	DREF DeeObject *result, *filename;
#define NEED_posix_fd_makepath
	filename = posix_fd_makepath(fd);
	if unlikely(!filename)
		goto err;
	result = posix_readlink_f_impl(filename);
	Dee_Decref(filename);
	return result;
err:
	return NULL;
#endif /* posix_freadlink_USE_posix_readlink */

#ifdef posix_freadlink_USE_STUB
	(void)fd;
#define NEED_posix_err_unsupported
	posix_err_unsupported("freadlink");
	return NULL;
#endif /* posix_freadlink_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("readlinkat", "dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags:u=0->?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_readlinkat_f_impl(DeeObject *dfd, DeeObject *file, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_readlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_READLINKAT_DEF { "readlinkat", (DeeObject *)&posix_readlinkat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags=!0)->?Dstring") },
#define POSIX_READLINKAT_DEF_DOC(doc) { "readlinkat", (DeeObject *)&posix_readlinkat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,file:?Dstring,atflags=!0)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_readlinkat, &posix_readlinkat_f, METHOD_FNORMAL);
#ifndef DEFINED_kwlist__dfd_file_atflags
#define DEFINED_kwlist__dfd_file_atflags
PRIVATE DEFINE_KWLIST(kwlist__dfd_file_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("file", 0x1a11b2b3, 0x612e37678ce7db5b), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__dfd_file_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_readlinkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *dfd;
		DeeObject *file;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__dfd_file_atflags, "oo|u:readlinkat", &args))
		goto err;
	return posix_readlinkat_f_impl(args.dfd, args.file, args.atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED NONNULL((1, 2))DREF DeeObject *DCALL posix_readlinkat_f_impl(DeeObject *dfd, DeeObject *file, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_readlinkat_USE_posix_readlink
	DREF DeeObject *absfile, *result;
#if defined(posix_readlinkat_USE_freadlinkat) || defined(posix_readlinkat_USE_readlinkat)
	if (!DeeString_Check(dfd) && DeeString_Check(file) &&
#ifdef posix_readlinkat_USE_readlinkat
	    atflags == 0 &&
#endif /* posix_readlinkat_USE_readlinkat */
	    1) {
		int os_dfd = DeeUnixSystem_GetFD(dfd);
		char const *utf8_file;
		char *buffer, *new_buffer;
		size_t bufsize, new_size;
		Dee_ssize_t req_size;
		if unlikely(os_dfd == -1)
			goto err;
		utf8_file = DeeString_AsUtf8(file);
		if unlikely(!utf8_file)
			goto err;
#ifdef PATH_MAX
		bufsize = PATH_MAX;
#else /* PATH_MAX */
		bufsize = 256;
#endif /* !PATH_MAX */
		buffer = (char *)DeeString_New1ByteBuffer(bufsize);
		if unlikely(!buffer)
			goto err;
EINTR_LABEL(again)
		for (;;) {
			DBG_ALIGNMENT_DISABLE();
#ifdef posix_readlinkat_USE_freadlinkat
			req_size = freadlinkat(os_dfd, (char *)utf8_file, buffer, bufsize + 1, AT_READLINK_REQSIZE | atflags);
#else /* ... */
			req_size = readlinkat(os_dfd, (char *)utf8_file, buffer, bufsize + 1);
#endif /* !... */
			if unlikely(req_size < 0) {
				EINTR_HANDLE(DeeSystem_GetErrno(), again, err_buffer);
				goto do_abspath_readlink;
			}
			DBG_ALIGNMENT_ENABLE();
#ifdef posix_readlinkat_USE_freadlinkat
			if ((size_t)req_size <= (bufsize + 1)) {
				--req_size; /* Account for trailing NUL */
				break;
			}
			new_size = (size_t)req_size - 1;
#else /* posix_readlinkat_USE_freadlinkat */
			if ((size_t)req_size <= bufsize)
				break;
			new_size = bufsize * 2;
#endif /* !posix_readlinkat_USE_freadlinkat */
			new_buffer = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buffer, bufsize);
			if unlikely(!new_buffer) {
EINTR_LABEL(err_buffer)
				DeeString_Free1ByteBuffer((uint8_t *)buffer);
				goto err;
			}
			buffer  = new_buffer;
			bufsize = new_size;
		}
	
		/* Release unused data. */
		buffer = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buffer, (size_t)req_size);
		return DeeString_PackUtf8Buffer((uint8_t *)buffer, STRING_ERROR_FREPLAC);
	}
do_abspath_readlink:
#endif /* posix_readlinkat_USE_freadlinkat || posix_readlinkat_USE_readlinkat */

#define NEED_posix_dfd_makepath
	absfile = posix_dfd_makepath(dfd, file, atflags);
	if unlikely(!absfile)
		goto err;
	result = posix_readlink_f_impl(absfile);
	Dee_Decref(absfile);
	return result;
err:
	return NULL;
#endif /* posix_readlinkat_USE_posix_readlink */

#ifdef posix_readlinkat_USE_STUB
	(void)dfd;
	(void)file;
	(void)atflags;
#define NEED_posix_err_unsupported
	posix_err_unsupported("readlinkat");
	return NULL;
#endif /* posix_readlinkat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_READLINK_C_INL */
