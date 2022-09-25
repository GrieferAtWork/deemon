/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_REALPATH_C_INL
#define GUARD_DEX_POSIX_P_REALPATH_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/system.h>

DECL_BEGIN

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

/************************************************************************/
/* realpath()                                                           */
/************************************************************************/

#undef posix_realpath_USE_FREALPATHAT
#undef posix_realpath_USE_REALPATH
#undef posix_realpath_USE_RESOLVEPATH
#undef posix_realpath_USE_WINDOWS
#undef posix_realpath_USE_OPEN
#undef posix_realpath_USE_FALLBACK
#if defined(CONFIG_HAVE_frealpathat) && defined(AT_FDCWD)
#define posix_realpath_USE_FREALPATHAT
#elif defined(CONFIG_HAVE_resolvepath)
#define posix_realpath_USE_RESOLVEPATH
#elif defined(CONFIG_HAVE_realpath)
#define posix_realpath_USE_REALPATH
#elif defined(CONFIG_HOST_WINDOWS)
#define posix_realpath_USE_WINDOWS
#elif (defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen) || \
       defined(CONFIG_HAVE_open64) || defined(CONFIG_HAVE_open))
#define posix_realpath_USE_OPEN
#else /* ... */
#define posix_realpath_USE_FALLBACK
#endif /* !... */


#if (defined(posix_realpath_USE_FREALPATHAT) || \
     defined(posix_realpath_USE_RESOLVEPATH) || \
     defined(posix_realpath_USE_REALPATH))
#ifndef CONFIG_HAVE_strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */
#endif /* posix_realpath_USE_... */



PRIVATE WUNUSED DREF DeeObject *DCALL
posix_realpath_f(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *filename;
	if (DeeArg_Unpack(argc, argv, "o:realpath", &filename))
		goto err;
	if (DeeString_Check(filename)) {
#if (defined(posix_realpath_USE_FREALPATHAT) || \
     defined(posix_realpath_USE_RESOLVEPATH))
		{
			char *buf;
			char const *utf8;
			size_t buflen;
			utf8 = DeeString_AsUtf8(filename);
			if unlikely(!utf8)
				goto err;
			buf = (char *)DeeString_New1ByteBuffer(PATH_MAX);
			if unlikely(!buf)
				goto err;
again_frealpathat:
			DBG_ALIGNMENT_DISABLE();
#ifdef posix_realpath_USE_FREALPATHAT
			if (frealpathat(AT_FDCWD, utf8, buf, buflen, 0) != NULL)
#else /* posix_realpath_USE_FREALPATHAT */
			if (resolvepath(utf8, buf, buflen) != -1)
#endif /* !posix_realpath_USE_FREALPATHAT */
			{
				size_t used_buflen;
				DBG_ALIGNMENT_ENABLE();
				used_buflen = strnlen(buf, buflen);
				if (used_buflen == buflen) {
					size_t new_buflen;
					char *newbuf;
increase_frealpathat_buflen:
					new_buflen = buflen * 2;
					newbuf     = (char *)DeeString_TryResize1ByteBuffer((uint8_t *)buf, new_buflen);
					if unlikely (!newbuf) {
						new_buflen = buflen + 1;
						newbuf     = (char *)DeeString_Resize1ByteBuffer((uint8_t *)buf, new_buflen);
						if unlikely (!newbuf) {
							DeeString_Free1ByteBuffer((uint8_t *)buf);
							goto err;
						}
					}
					buf    = newbuf;
					buflen = new_buflen;
					goto again_frealpathat;
				}
			} else {
				int error;
				error = DeeSystem_GetErrno();
				DBG_ALIGNMENT_ENABLE();
				HANDLE_EINTR(error, again_frealpathat, err);
#ifdef ERANGE
				if (error == ERANGE)
					goto increase_frealpathat_buflen;
#endif /* ERANGE */
				/* Make use of one of the alternatives below... */
#define posix_realpath_NEED_ALTERNATIVE
			}
			DeeString_Free1ByteBuffer((uint8_t *)buf);
		} /* Scope... */
#endif /* posix_realpath_USE_FREALPATHAT || posix_realpath_USE_RESOLVEPATH */

#ifdef posix_realpath_USE_REALPATH
		{
			char *buf;
			char const *utf8;
			utf8 = DeeString_AsUtf8(filename);
			if unlikely(!utf8)
				goto err;
			buf = (char *)DeeString_New1ByteBuffer(PATH_MAX);
			if unlikely(!buf)
				goto err;
again_realpath:
			DBG_ALIGNMENT_DISABLE();
			if (realpath(utf8, buf) != NULL) {
				size_t usedlen;
				DBG_ALIGNMENT_ENABLE();
				usedlen = strnlen(buf, PATH_MAX);
				if (usedlen != PATH_MAX) {
					buf = (char *)DeeString_Truncate1ByteBuffer((uint8_t *)buf, usedlen);
					return DeeString_PackUtf8Buffer((uint8_t *)buf, STRING_ERROR_FIGNORE);
				}
			} else {
				int error;
				error = DeeSystem_GetErrno();
				DBG_ALIGNMENT_ENABLE();
				HANDLE_EINTR(error, again_realpath, err);
				/* Make use of one of the alternatives below... */
#define posix_realpath_NEED_ALTERNATIVE
			}
			DeeString_Free1ByteBuffer((uint8_t *)buf);
		} /* Scope... */
#endif /* posix_realpath_USE_REALPATH */

#ifdef posix_realpath_NEED_ALTERNATIVE
#undef posix_realpath_NEED_ALTERNATIVE
#if (!defined(posix_realpath_USE_WINDOWS) && \
     !defined(posix_realpath_USE_OPEN) &&    \
     !defined(posix_realpath_USE_FALLBACK))
#ifdef CONFIG_HOST_WINDOWS
#define posix_realpath_USE_WINDOWS
#elif (defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen) || \
       defined(CONFIG_HAVE_open64) || defined(CONFIG_HAVE_open))
#define posix_realpath_USE_OPEN
#else /* ... */
#define posix_realpath_USE_FALLBACK
#endif /* !... */
#endif /* !posix_realpath_USE_... */
#endif /* posix_realpath_NEED_ALTERNATIVE */

#ifdef posix_realpath_USE_WINDOWS
		{
			HANDLE hFile;
			hFile = DeeNTSystem_CreateFileNoATime(filename,
			                                      FILE_READ_ATTRIBUTES,
			                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			                                      NULL,
			                                      OPEN_EXISTING,
			                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
			                                      NULL);
			if unlikely(hFile == NULL)
				goto err;
			if unlikely(hFile == INVALID_HANDLE_VALUE) {
				DeeNTSystem_ThrowLastErrorf(NULL, "Failed to open %r", filename);
				goto err;
			}
			result = DeeNTSystem_GetFilenameOfHandle(hFile);
			CloseHandle(hFile);
			return result;
		} /* Scope... */
#endif /* posix_realpath_USE_WINDOWS */

#ifdef posix_realpath_USE_OPEN
#ifdef CONFIG_HAVE_O_RDONLY
#define OPEN_FLAGS_RDONLY O_RDONLY
#else /* CONFIG_HAVE_O_RDONLY */
#define OPEN_FLAGS_RDONLY 0
#endif /* !CONFIG_HAVE_O_RDONLY */
#ifdef CONFIG_HAVE_O_OBTAIN_DIR
#define OPEN_FLAGS_OBTAIN_DIR O_OBTAIN_DIR
#else /* CONFIG_HAVE_O_OBTAIN_DIR */
#define OPEN_FLAGS_OBTAIN_DIR 0
#endif /* !CONFIG_HAVE_O_OBTAIN_DIR */
		{
			int fd;
#if defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen)
			dwchar_t const *wide;
			wide = DeeString_AsWide(filename);
			if unlikely(!wide)
				goto err;
again_open:
#ifdef CONFIG_HAVE_wopen64
			fd = wopen64(wide, OPEN_FLAGS_RDONLY | OPEN_FLAGS_OBTAIN_DIR, 0);
#else /* CONFIG_HAVE_wopen64 */
			fd = wopen(wide, OPEN_FLAGS_RDONLY | OPEN_FLAGS_OBTAIN_DIR, 0);
#endif /* !CONFIG_HAVE_wopen64 */
#else /* CONFIG_HAVE_wopen64 || CONFIG_HAVE_wopen */
			char const *utf8;
			utf8 = DeeString_AsUtf8(filename);
			if unlikely(!utf8)
				goto err;
again_open:
#ifdef CONFIG_HAVE_open64
			fd = open64(utf8, OPEN_FLAGS_RDONLY | OPEN_FLAGS_OBTAIN_DIR, 0);
#else /* CONFIG_HAVE_open64 */
			fd = open(utf8, OPEN_FLAGS_RDONLY | OPEN_FLAGS_OBTAIN_DIR, 0);
#endif /* !CONFIG_HAVE_open64 */
#endif /* !CONFIG_HAVE_wopen64 && !CONFIG_HAVE_wopen */
			if (fd < 0) {
				fd = DeeSystem_GetErrno();
				HANDLE_EINTR(fd, again_open, err)
				DeeUnixSystem_ThrowErrorf(NULL, fd, "Failed to open %r", filename);
				goto err;
			}
			result = DeeSystem_GetFilenameOfFD(fd);
#ifdef CONFIG_HAVE_close
			close(fd);
#endif /* CONFIG_HAVE_close */
			return result;
		} /* Scope... */
#endif /* posix_realpath_USE_OPEN */

#ifdef posix_realpath_USE_FALLBACK
		{
			DREF DeeObject *file;
			result = NULL;
			file   = DeeFile_Open(filename, Dee_OPEN_FRDONLY, 0);
			if likely(file) {
				result = DeeFile_Filename(file);
				Dee_Decref(file);
			}
			return result;
		} /* Scope... */
#endif /* posix_realpath_USE_FALLBACK */

	} else {
#ifdef CONFIG_HOST_WINDOWS
		HANDLE hnd;
		hnd = DeeNTSystem_GetHandle(filename);
		if unlikely(hnd == INVALID_HANDLE_VALUE)
			goto err;
		result = DeeNTSystem_GetFilenameOfHandle(hnd);
#else /* CONFIG_HOST_WINDOWS */
		int fd = DeeUnixSystem_GetFD(filename);
		if unlikely(fd == -1)
			goto err;
		result = DeeSystem_GetFilenameOfFD(fd);
#endif /* !CONFIG_HOST_WINDOWS */
	}
	return result;
err:
	return NULL;
}


#define POSIX_REALPATH_DEF                                          \
	{ "realpath", (DeeObject *)&posix_realpath, MODSYM_FNORMAL,     \
	  DOC("(filename:?X3?Dstring?DFile?Dint)->?Dstring\n"           \
	      "Resolve all symbolic links in, and normalize $'.'- and " \
	      "$'..'-references within the given @filename. Note that " \
	      "for this purpose, @filename must exist.") },
PRIVATE DEFINE_CMETHOD(posix_realpath, posix_realpath_f);

DECL_END

#endif /* !GUARD_DEX_POSIX_P_REALPATH_C_INL */
