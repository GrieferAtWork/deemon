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
#ifndef GUARD_DEX_POSIX_P_CHMOD_C_INL
#define GUARD_DEX_POSIX_P_CHMOD_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include "p-readlink.c.inl" /* For `posix_chmod_USE_posix_readlink__AND__posix_lchmod()' */
#include "p-path.c.inl"     /* For `posix_chmod_USE_posix_readlink__AND__posix_lchmod()' */
/**/

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Re-link system functions to try and use 64-bit variants (if available) */
/*[[[deemon
local functions = {
	"open",
	"openat",
	"creat",
	"wopen",
	"wopenat",
	"wcreat",
};
for (local f: functions) {
	print("#ifdef CONFIG_HAVE_", f, "64");
	print("#undef CONFIG_HAVE_", f);
	print("#define CONFIG_HAVE_", f);
	print("#undef ", f);
	print("#define ", f, " ", f, "64");
	print("#endif /" "* CONFIG_HAVE_", f, "64 *" "/");
}
]]]*/
#ifdef CONFIG_HAVE_open64
#undef CONFIG_HAVE_open
#define CONFIG_HAVE_open
#undef open
#define open open64
#endif /* CONFIG_HAVE_open64 */
#ifdef CONFIG_HAVE_openat64
#undef CONFIG_HAVE_openat
#define CONFIG_HAVE_openat
#undef openat
#define openat openat64
#endif /* CONFIG_HAVE_openat64 */
#ifdef CONFIG_HAVE_creat64
#undef CONFIG_HAVE_creat
#define CONFIG_HAVE_creat
#undef creat
#define creat creat64
#endif /* CONFIG_HAVE_creat64 */
#ifdef CONFIG_HAVE_wopen64
#undef CONFIG_HAVE_wopen
#define CONFIG_HAVE_wopen
#undef wopen
#define wopen wopen64
#endif /* CONFIG_HAVE_wopen64 */
#ifdef CONFIG_HAVE_wopenat64
#undef CONFIG_HAVE_wopenat
#define CONFIG_HAVE_wopenat
#undef wopenat
#define wopenat wopenat64
#endif /* CONFIG_HAVE_wopenat64 */
#ifdef CONFIG_HAVE_wcreat64
#undef CONFIG_HAVE_wcreat
#define CONFIG_HAVE_wcreat
#undef wcreat
#define wcreat wcreat64
#endif /* CONFIG_HAVE_wcreat64 */
/*[[[end]]]*/


/* Figure out how we want to implement `chmod()' */
#undef posix_chmod_USE_wchmod
#undef posix_chmod_USE_chmod
#undef posix_chmod_USE_wopen_AND_fchmod
#undef posix_chmod_USE_open_AND_fchmod
#undef posix_chmod_USE_posix_readlink__AND__posix_lchmod
#undef posix_chmod_USE_STUB
#if defined(CONFIG_HAVE_wchmod) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_chmod_USE_wchmod
#elif defined(CONFIG_HAVE_chmod)
#define posix_chmod_USE_chmod
#elif defined(CONFIG_HAVE_wchmod)
#define posix_chmod_USE_wchmod
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_fchmod) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_chmod_USE_wopen_AND_fchmod
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_fchmod)
#define posix_chmod_USE_open_AND_fchmod
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_fchmod)
#define posix_chmod_USE_wopen_AND_fchmod
#else /* ... */
#define posix_chmod_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `lchmod()' */
#undef posix_lchmod_USE_wlchmod
#undef posix_lchmod_USE_lchmod
#undef posix_lchmod_USE_wopen_AND_fchmod
#undef posix_lchmod_USE_open_AND_fchmod
#undef posix_lchmod_USE_STUB
#if defined(CONFIG_HAVE_wlchmod) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lchmod_USE_wlchmod
#elif defined(CONFIG_HAVE_lchmod)
#define posix_lchmod_USE_lchmod
#elif defined(CONFIG_HAVE_wlchmod)
#define posix_lchmod_USE_wlchmod
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_fchmod) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lchmod_USE_wopen_AND_fchmod
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_fchmod)
#define posix_lchmod_USE_open_AND_fchmod
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_fchmod)
#define posix_lchmod_USE_wopen_AND_fchmod
#else /* ... */
#define posix_lchmod_USE_STUB
#endif /* !... */

/* Check if we should emulate
 * >> chmod(path, mode);
 * As:
 * >> lchmod(try joinpath(headof(path), readlink(path)) catch (NoSymlink) path, mode); */
#if ((defined(posix_chmod_USE_STUB) || defined(posix_chmod_USE_open_AND_fchmod) || \
      defined(posix_chmod_USE_wopen_AND_fchmod)) &&                                \
     (!defined(posix_lchmod_USE_STUB) && !defined(posix_readlink_USE_STUB)))
#undef posix_chmod_USE_STUB
#undef posix_chmod_USE_open_AND_fchmod
#undef posix_chmod_USE_wopen_AND_fchmod
#define posix_chmod_USE_posix_readlink__AND__posix_lchmod
#endif /* ... */



/* Figure out how we want to implement `fchmod()' */
#undef posix_fchmod_USE_fchmod
#undef posix_fchmod_USE_posix_lchmod
#undef posix_fchmod_USE_posix_chmod
#undef posix_fchmod_USE_STUB
#ifdef CONFIG_HAVE_fchmod
#define posix_fchmod_USE_fchmod
#elif !defined(posix_lchmod_USE_STUB)
#define posix_fchmod_USE_posix_lchmod
#elif !defined(posix_chmod_USE_STUB)
#define posix_fchmod_USE_posix_chmod
#else /* ... */
#define posix_fchmod_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `fchmodat()' */
#undef posix_fchmodat_USE_fchmodat
#undef posix_fchmodat_USE_posix_chmod
#undef posix_fchmodat_USE_posix_lchmod
#undef posix_fchmodat_USE_posix_fchmod
#undef posix_fchmodat_USE_STUB
#if !defined(posix_chmod_USE_STUB) || !defined(posix_lchmod_USE_STUB)
#ifdef CONFIG_HAVE_fchmodat
#define posix_fchmodat_USE_fchmodat
#endif /* CONFIG_HAVE_fchmodat */
#ifndef posix_chmod_USE_STUB
#define posix_fchmodat_USE_posix_chmod
#endif /* !posix_chmod_USE_STUB */
#ifndef posix_lchmod_USE_STUB
#define posix_fchmodat_USE_posix_lchmod
#endif /* !posix_lchmod_USE_STUB */
#ifndef posix_fchmod_USE_STUB
#define posix_fchmodat_USE_posix_fchmod
#endif /* !posix_fchmod_USE_STUB */
#else /* !posix_chmod_USE_STUB */
#define posix_fchmodat_USE_STUB
#endif /* posix_chmod_USE_STUB */




/* Implement the open+fchmod wrappers. */
#ifdef posix_chmod_USE_wopen_AND_fchmod
#undef fposix_chmod_USE_wopen_AND_fchmod
#define posix_chmod_USE_wchmod
#undef wchmod
#define wchmod dee_wchmod
PRIVATE int DCALL dee_wchmod(wchar_t const *filename, int mode) {
	int result;
	result = wopen(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = fchmod(result, mode);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_chmod_USE_wopen_AND_fchmod */

#ifdef posix_chmod_USE_open_AND_fchmod
#undef fposix_chmod_USE_open_AND_fchmod
#define posix_chmod_USE_chmod
#undef chmod
#define chmod dee_chmod
PRIVATE int DCALL dee_chmod(wchar_t const *filename, int mode) {
	int result;
	result = open(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = fchmod(result, mode);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_chmod_USE_open_AND_fchmod */

#ifdef posix_lchmod_USE_wopen_AND_fchmod
#undef fposix_lchmod_USE_wopen_AND_fchmod
#define posix_lchmod_USE_wlchmod
#undef wlchmod
#define wlchmod dee_wlchmod
PRIVATE int DCALL dee_wlchmod(wchar_t const *filename, int mode) {
	int result;
	result = wopen(filename, O_RDWR | O_NOFOLLOW);
	if (result != -1) {
		int error;
		error = fchmod(result, mode);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lchmod_USE_wopen_AND_fchmod */

#ifdef posix_lchmod_USE_open_AND_fchmod
#undef fposix_lchmod_USE_open_AND_fchmod
#define posix_lchmod_USE_lchmod
#undef lchmod
#define lchmod dee_lchmod
PRIVATE int DCALL dee_lchmod(wchar_t const *filename, int mode) {
	int result;
	result = open(filename, O_RDWR | O_NOFOLLOW);
	if (result != -1) {
		int error;
		error = fchmod(result, mode);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lchmod_USE_open_AND_fchmod */

#ifdef posix_chmod_USE_posix_readlink__AND__posix_lchmod
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lchmod_f_impl(DeeObject *path, DeeObject *mode);
#endif /* posix_chmod_USE_posix_readlink__AND__posix_lchmod */



/*[[[deemon import("rt.gen.dexutils").gw("chmod", "path:?Dstring,mode:?X2?Dstring?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_chmod_f_impl(DeeObject *path, DeeObject *mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_chmod_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_CHMOD_DEF { "chmod", (DeeObject *)&posix_chmod, MODSYM_FREADONLY, DOC("(path:?Dstring,mode:?X2?Dstring?Dint)") },
#define POSIX_CHMOD_DEF_DOC(doc) { "chmod", (DeeObject *)&posix_chmod, MODSYM_FREADONLY, DOC("(path:?Dstring,mode:?X2?Dstring?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_chmod, &posix_chmod_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_PATH_MODE_DEFINED
#define POSIX_KWDS_PATH_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path_mode, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !POSIX_KWDS_PATH_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_chmod_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	DeeObject *mode;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path_mode, "oo:chmod", &path, &mode))
		goto err;
	return posix_chmod_f_impl(path, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_chmod_f_impl(DeeObject *path, DeeObject *mode)
/*[[[end]]]*/
{
#ifdef posix_chmod_USE_posix_readlink__AND__posix_lchmod
	DREF DeeObject *link_text;
	/* Try to readlink() the given `path' to see if it's a symbolic link. */
	link_text = posix_readlink_f_impl(path);
	if (link_text) {
		DREF DeeObject *full_path, *result;
		full_path = (DREF DeeObject *)posix_path_walklink_f((DeeStringObject *)link_text,
		                                                    (DeeStringObject *)path);
		Dee_Decref(link_text);
		if unlikely(!full_path)
			goto err;
		result = posix_lchmod_f_impl(full_path, mode);
		Dee_Decref(full_path);
		return result;
	}
	if (!DeeError_Catch(&DeeError_NoSymlink))
		goto err;

	/* Not a symbolic link -> lchmod() will work to do what we want! */
	return posix_lchmod_f_impl(path, mode);
err:
	return NULL;
#endif /* posix_chmod_USE_posix_readlink__AND__posix_lchmod */

#if defined(posix_chmod_USE_chmod) || defined(posix_chmod_USE_wchmod)
	int error;
	unsigned int used_mode;
#ifdef posix_chmod_USE_chmod
	char const *utf8_path;
#endif /* posix_chmod_USE_chmod */
#ifdef posix_chmod_USE_wchmod
	dwchar_t const *wide_path;
#endif /* posix_chmod_USE_wchmod */

	/* Decode arguments. */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_chmod_USE_chmod
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_chmod_USE_chmod */
#ifdef posix_chmod_USE_wchmod
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_chmod_USE_wchmod */

	/* Load the chmod-mode argument. */
	used_mode = posix_chmod_getmode(path, mode);
#define NEED_posix_chmod_getmode
	if unlikely(used_mode == (unsigned int)-1)
		goto err;

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_chmod_USE_wchmod
	error = wchmod(wide_path, used_mode);
#endif /* posix_chmod_USE_wchmod */
#ifdef posix_chmod_USE_chmod
	error = chmod(utf8_path, used_mode);
#endif /* posix_chmod_USE_chmod */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_chmod(error, path, used_mode);
#define NEED_err_unix_chmod
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_chmod_USE_chmod || posix_chmod_USE_wchmod */

#ifdef posix_chmod_USE_STUB
	(void)path;
	(void)mode;
	posix_err_unsupported("chmod");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_chmod_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("lchmod", "path:?Dstring,mode:?X2?Dstring?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lchmod_f_impl(DeeObject *path, DeeObject *mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lchmod_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_LCHMOD_DEF { "lchmod", (DeeObject *)&posix_lchmod, MODSYM_FREADONLY, DOC("(path:?Dstring,mode:?X2?Dstring?Dint)") },
#define POSIX_LCHMOD_DEF_DOC(doc) { "lchmod", (DeeObject *)&posix_lchmod, MODSYM_FREADONLY, DOC("(path:?Dstring,mode:?X2?Dstring?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lchmod, &posix_lchmod_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_PATH_MODE_DEFINED
#define POSIX_KWDS_PATH_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path_mode, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !POSIX_KWDS_PATH_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lchmod_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	DeeObject *mode;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path_mode, "oo:lchmod", &path, &mode))
		goto err;
	return posix_lchmod_f_impl(path, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lchmod_f_impl(DeeObject *path, DeeObject *mode)
/*[[[end]]]*/
{
#if defined(posix_lchmod_USE_lchmod) || defined(posix_lchmod_USE_wlchmod)
	int error;
	unsigned int used_mode;
#ifdef posix_lchmod_USE_lchmod
	char const *utf8_path;
#endif /* posix_lchmod_USE_lchmod */
#ifdef posix_lchmod_USE_wlchmod
	dwchar_t const *wide_path;
#endif /* posix_lchmod_USE_wlchmod */

	/* Decode arguments. */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_lchmod_USE_lchmod
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_lchmod_USE_lchmod */
#ifdef posix_lchmod_USE_wlchmod
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_lchmod_USE_wlchmod */

	/* Load the lchmod-mode argument. */
	used_mode = posix_lchmod_getmode(path, mode);
#define NEED_posix_lchmod_getmode
	if unlikely(used_mode == (unsigned int)-1)
		goto err;

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_lchmod_USE_wlchmod
	error = wlchmod(wide_path, used_mode);
#endif /* posix_lchmod_USE_wlchmod */
#ifdef posix_lchmod_USE_lchmod
	error = lchmod(utf8_path, used_mode);
#endif /* posix_lchmod_USE_lchmod */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_lchmod(error, path, used_mode);
#define NEED_err_unix_lchmod
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_lchmod_USE_lchmod || posix_lchmod_USE_wlchmod */

#ifdef posix_lchmod_USE_STUB
	(void)path;
	(void)mode;
	posix_err_unsupported("lchmod");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_lchmod_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("fchmod", "fd:?X2?Dint?DFile,mode:?X2?Dstring?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchmod_f_impl(DeeObject *fd, DeeObject *mode);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchmod_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHMOD_DEF { "fchmod", (DeeObject *)&posix_fchmod, MODSYM_FREADONLY, DOC("(fd:?X2?Dint?DFile,mode:?X2?Dstring?Dint)") },
#define POSIX_FCHMOD_DEF_DOC(doc) { "fchmod", (DeeObject *)&posix_fchmod, MODSYM_FREADONLY, DOC("(fd:?X2?Dint?DFile,mode:?X2?Dstring?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchmod, &posix_fchmod_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_FD_MODE_DEFINED
#define POSIX_KWDS_FD_MODE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd_mode, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEND });
#endif /* !POSIX_KWDS_FD_MODE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchmod_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *fd;
	DeeObject *mode;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_mode, "oo:fchmod", &fd, &mode))
		goto err;
	return posix_fchmod_f_impl(fd, mode);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchmod_f_impl(DeeObject *fd, DeeObject *mode)
/*[[[end]]]*/
{
#ifdef posix_fchmod_USE_fchmod
	int error;
	unsigned int used_mode;
	int os_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(os_fd == -1)
		goto err;

	/* Load the chmod-mode argument. */
	used_mode = posix_chmod_getmode(fd, mode);
#define NEED_posix_chmod_getmode
	if unlikely(used_mode == (unsigned int)-1)
		goto err;

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	error = fchmod(os_fd, used_mode);
	DBG_ALIGNMENT_ENABLE();
	
	if unlikely(error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_fchmod(error, fd, used_mode);
#define NEED_err_unix_fchmod
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_fchmod_USE_fchmod */

#if defined(posix_fchmod_USE_posix_lchmod) || defined(posix_fchmod_USE_posix_chmod)
	DREF DeeObject *result;
	DREF DeeObject *abspath;
	abspath = posix_fd_makepath(fd);
#define NEED_posix_fd_makepath
	if unlikely(!abspath)
		goto err;
#ifdef posix_fchmod_USE_posix_lchmod
	result = posix_lchmod_f_impl(abspath, mode);
#else /* posix_fchmod_USE_posix_lchmod */
	result = posix_chmod_f_impl(abspath, mode);
#endif /* !posix_fchmod_USE_posix_lchmod */
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fchmod_USE_posix_lchmod || posix_fchmod_USE_posix_chmod */

#ifdef posix_fchmod_USE_STUB
	(void)fd;
	(void)mode;
	posix_err_unsupported("fchmod");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_fchmod_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("fchmodat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:?X2?Dstring?Dint,atflags:u=0", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchmodat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *mode, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchmodat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHMODAT_DEF { "fchmodat", (DeeObject *)&posix_fchmodat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:?X2?Dstring?Dint,atflags:?Dint=!0)") },
#define POSIX_FCHMODAT_DEF_DOC(doc) { "fchmodat", (DeeObject *)&posix_fchmodat, MODSYM_FREADONLY, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,mode:?X2?Dstring?Dint,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchmodat, &posix_fchmodat_f, METHOD_FNORMAL);
#ifndef POSIX_KWDS_DFD_PATH_MODE_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_MODE_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_mode_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("mode", 0x11abbac9, 0xa978c54b1db00143), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_MODE_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchmodat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	DeeObject *mode;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_mode_atflags, "ooo|u:fchmodat", &dfd, &path, &mode, &atflags))
		goto err;
	return posix_fchmodat_f_impl(dfd, path, mode, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchmodat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *mode, unsigned int atflags)
/*[[[end]]]*/
{
#if defined(posix_fchmodat_USE_posix_chmod) || defined(posix_fchmodat_USE_posix_lchmod)
	DREF DeeObject *result, *abspath;
#ifdef posix_fchmodat_USE_posix_fchmod
	if (atflags & AT_EMPTY_PATH) {
		if (!DeeString_Check(dfd) &&
		    (DeeNone_Check(path) || (DeeString_Check(path) &&
		                             DeeString_IsEmpty(path))))
			return posix_fchmod_f_impl(dfd, mode);
		atflags &= ~AT_EMPTY_PATH;
	}
#endif /* posix_fchmodat_USE_posix_fchmod */

#ifdef posix_fchmodat_USE_fchmodat
	if (!DeeString_Check(dfd) && !DeeString_Check(mode)) {
		int os_dfd, error;
		unsigned int used_mode;
		char const *utf8_path;
		os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
		if (DeeObject_AssertTypeExact(path, &DeeString_Type))
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
		if (DeeObject_AsUInt(mode, &used_mode))
			goto err;
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		error = fchmodat(os_dfd, utf8_path, used_mode, atflags);
		if (error >= 0) {
			DBG_ALIGNMENT_ENABLE();
			return DeeInt_NewUInt((unsigned int)error);
		}
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		/* Fallthru to fallback path below */
	}
#endif /* posix_fchmodat_USE_fchmodat */

#ifdef posix_fchmodat_USE_posix_lchmod
	abspath = posix_dfd_makepath(dfd, path, atflags & ~AT_SYMLINK_NOFOLLOW);
#else /* posix_fchmodat_USE_posix_lchmod */
	abspath = posix_dfd_makepath(dfd, path, atflags);
#endif /* !posix_fchmodat_USE_posix_lchmod */
	if unlikely(!abspath)
		goto err;
#ifdef posix_fchmodat_USE_posix_lchmod
	if (atflags & AT_SYMLINK_NOFOLLOW) {
		result = posix_lchmod_f_impl(abspath, mode);
	} else
#endif /* posix_fchmodat_USE_posix_lchmod */
	{
#ifdef posix_fchmodat_USE_posix_chmod
		result = posix_chmod_f_impl(abspath, mode);
#else /* posix_fchmodat_USE_posix_chmod */
		Dee_Decref(abspath);
		err_bad_atflags(atflags);
#define NEED_err_bad_atflags
		goto err;
#endif /* !posix_fchmodat_USE_posix_chmod */
	}
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fchmodat_USE_posix_chmod || posix_fchmodat_USE_posix_lchmod */

#ifdef posix_fchmodat_USE_STUB
	(void)dfd;
	(void)path;
	(void)mode;
	(void)atflags;
	posix_err_unsupported("fchmodat");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_fchmodat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_CHMOD_C_INL */
