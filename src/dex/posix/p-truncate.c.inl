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
#ifndef GUARD_DEX_POSIX_P_TRUNCATE_C_INL
#define GUARD_DEX_POSIX_P_TRUNCATE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

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


/* Figure out how we want to implement `truncate()' */
#undef posix_truncate_USE_wtruncate64
#undef posix_truncate_USE_wtruncate
#undef posix_truncate_USE_truncate64
#undef posix_truncate_USE_truncate
#undef posix_truncate_USE_wopen_AND_ftruncate64
#undef posix_truncate_USE_wopen_AND_ftruncate
#undef posix_truncate_USE_open_AND_ftruncate64
#undef posix_truncate_USE_open_AND_ftruncate
#undef posix_truncate_USE_STUB
#if defined(CONFIG_HAVE_wtruncate64)
#define posix_truncate_USE_wtruncate64
#elif defined(CONFIG_HAVE_wtruncate)
#define posix_truncate_USE_wtruncate
#elif defined(CONFIG_HAVE_truncate64)
#define posix_truncate_USE_truncate64
#elif defined(CONFIG_HAVE_truncate)
#define posix_truncate_USE_truncate
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_ftruncate64) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_truncate_USE_wopen_AND_ftruncate64
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_ftruncate) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_truncate_USE_wopen_AND_ftruncate
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_ftruncate64)
#define posix_truncate_USE_open_AND_ftruncate64
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_ftruncate)
#define posix_truncate_USE_open_AND_ftruncate
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_ftruncate64)
#define posix_truncate_USE_wopen_AND_ftruncate64
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_ftruncate)
#define posix_truncate_USE_wopen_AND_ftruncate
#else /* ... */
#define posix_truncate_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `ftruncate()' */
#undef posix_ftruncate_USE_DeeFile_Trunc
#undef posix_ftruncate_USE_ftruncate64
#undef posix_ftruncate_USE_ftruncate
#undef posix_ftruncate_USE_posix_truncate
#undef posix_ftruncate_USE_STUB
#if defined(CONFIG_HAVE_ftruncate64)
#define posix_ftruncate_USE_DeeFile_Trunc
#define posix_ftruncate_USE_ftruncate64
#elif defined(CONFIG_HAVE_ftruncate)
#define posix_ftruncate_USE_DeeFile_Trunc
#define posix_ftruncate_USE_ftruncate
#elif !defined(posix_truncate_USE_STUB)
#define posix_ftruncate_USE_DeeFile_Trunc
#define posix_ftruncate_USE_posix_truncate
#elif 1
#define posix_ftruncate_USE_DeeFile_Trunc
#else /* ... */
#define posix_ftruncate_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `ftruncateat()' */
#undef posix_ftruncateat_USE_posix_ftruncate
#undef posix_ftruncateat_USE_posix_truncate
#undef posix_ftruncateat_USE_STUB
#if !defined(posix_truncate_USE_STUB)
#define posix_ftruncateat_USE_posix_ftruncate
#define posix_ftruncateat_USE_posix_truncate
#else /* ... */
#define posix_ftruncateat_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `truncateat()' */
#undef posix_truncateat_USE_posix_truncate
#undef posix_truncateat_USE_posix_ftruncateat
#undef posix_truncateat_USE_STUB
#if !defined(posix_truncate_USE_STUB)
#define posix_truncateat_USE_posix_truncate
#elif !defined(posix_ftruncateat_USE_STUB)
#define posix_truncateat_USE_posix_ftruncateat
#else /* ... */
#define posix_truncateat_USE_STUB
#endif /* !... */




/* Figure out which file-size type to use. */
#undef posix_truncate_USE_pos_t
#if (defined(posix_truncate_USE_truncate64) ||           \
     defined(posix_truncate_USE_wtruncate64) ||          \
     defined(posix_truncate_USE_open_AND_ftruncate64) || \
     defined(posix_truncate_USE_wopen_AND_ftruncate64))
#define posix_truncate_USE_pos_t uint64_t
#elif (defined(posix_truncate_USE_truncate) ||           \
       defined(posix_truncate_USE_wtruncate) ||          \
       defined(posix_truncate_USE_open_AND_ftruncate) || \
       defined(posix_truncate_USE_wopen_AND_ftruncate))
#define posix_truncate_USE_pos_t __ULONGPTR_TYPE__
#endif /* ... */

#undef posix_ftruncate_USE_pos_t
#ifdef posix_ftruncate_USE_ftruncate64
#define posix_ftruncate_USE_pos_t uint64_t
#elif defined(posix_ftruncate_USE_ftruncate)
#define posix_ftruncate_USE_pos_t __ULONGPTR_TYPE__
#endif /* ... */

/* Implement the open+ftruncate wrappers. */
#ifdef posix_truncate_USE_wopen_AND_ftruncate64
#undef fposix_truncate_USE_wopen_AND_ftruncate64
#define posix_truncate_USE_wtruncate64
#undef wtruncate64
#define wtruncate64 dee_wtruncate64
PRIVATE int DCALL dee_wtruncate64(wchar_t const *filename, uint64_t size) {
	int result;
	result = wopen(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = ftruncate64(result, size);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_truncate_USE_wopen_AND_ftruncate64 */

#ifdef posix_truncate_USE_wopen_AND_ftruncate
#undef fposix_truncate_USE_wopen_AND_ftruncate
#define posix_truncate_USE_wtruncate
#undef wtruncate
#define wtruncate dee_wtruncate
PRIVATE int DCALL dee_wtruncate(wchar_t const *filename, __ULONGPTR_TYPE__ size) {
	int result;
	result = wopen(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = ftruncate(result, size);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_truncate_USE_wopen_AND_ftruncate */

#ifdef posix_truncate_USE_open_AND_ftruncate64
#undef fposix_truncate_USE_open_AND_ftruncate64
#define posix_truncate_USE_truncate64
#undef truncate64
#define truncate64 dee_truncate64
PRIVATE int DCALL dee_truncate64(wchar_t const *filename, uint64_t size) {
	int result;
	result = open(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = ftruncate64(result, size);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_truncate_USE_open_AND_ftruncate64 */

#ifdef posix_truncate_USE_open_AND_ftruncate
#undef fposix_truncate_USE_open_AND_ftruncate
#define posix_truncate_USE_truncate
#undef truncate
#define truncate dee_truncate
PRIVATE int DCALL dee_truncate(wchar_t const *filename, __ULONGPTR_TYPE__ size) {
	int result;
	result = open(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = ftruncate(result, size);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_truncate_USE_open_AND_ftruncate */



/* Figure out which character-encoding to use. */
#undef posix_truncate_USE_UTF8
#undef posix_truncate_USE_WIDE
#if defined(posix_truncate_USE_truncate64) || defined(posix_truncate_USE_truncate)
#define posix_truncate_USE_UTF8
#endif /* posix_truncate_USE_truncate64 || posix_truncate_USE_truncate */
#if defined(posix_truncate_USE_wtruncate64) || defined(posix_truncate_USE_wtruncate)
#define posix_truncate_USE_WIDE
#endif /* posix_truncate_USE_wtruncate64 || posix_truncate_USE_wtruncate */



/*[[[deemon import("rt.gen.dexutils").gw("truncate", "path:?Dstring,length:?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(DeeObject *path, DeeObject *length);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(path:?Dstring,length:?Dint)") },
#define POSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(path:?Dstring,length:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_truncate, &posix_truncate_f);
#ifndef POSIX_KWDS_PATH_LENGTH_DEFINED
#define POSIX_KWDS_PATH_LENGTH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path_length, { KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("length", 0xecef0c1, 0x2993e8eb119cab21), KEND });
#endif /* !POSIX_KWDS_PATH_LENGTH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	DeeObject *length;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path_length, "oo:truncate", &path, &length))
		goto err;
	return posix_truncate_f_impl(path, length);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(DeeObject *path, DeeObject *length)
/*[[[end]]]*/
{
#ifndef posix_truncate_USE_STUB
	int error;
#ifdef posix_truncate_USE_UTF8
	char const *utf8_path;
#endif /* posix_truncate_USE_UTF8 */
#ifdef posix_truncate_USE_WIDE
	dwchar_t const *wide_path;
#endif /* posix_truncate_USE_WIDE */
#ifdef posix_truncate_USE_pos_t
	posix_truncate_USE_pos_t used_length;
#endif /* posix_truncate_USE_pos_t */

	/* Decode arguments. */
#ifdef posix_truncate_USE_UTF8
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_truncate_USE_UTF8 */
#ifdef posix_truncate_USE_WIDE
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_truncate_USE_WIDE */
#ifdef posix_truncate_USE_pos_t
	if (DeeObject_AsUIntX(length, &used_length))
		goto err;
#endif /* posix_truncate_USE_pos_t */


EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_truncate_USE_wtruncate64
	error = wtruncate64(wide_path, used_length);
#endif /* posix_truncate_USE_wtruncate64 */
#ifdef posix_truncate_USE_wtruncate
	error = wtruncate(wide_path, used_length);
#endif /* posix_truncate_USE_wtruncate */
#ifdef posix_truncate_USE_truncate64
	error = truncate64(utf8_path, used_length);
#endif /* posix_truncate_USE_truncate64 */
#ifdef posix_truncate_USE_truncate
	error = truncate(utf8_path, used_length);
#endif /* posix_truncate_USE_truncate */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_truncate(error, path, length);
#define NEED_err_unix_truncate
		goto err;
	}
	return_none;
err:
	return NULL;
#else /* !posix_truncate_USE_STUB */
#define NEED_posix_err_unsupported
	(void)path;
	(void)length;
	posix_err_unsupported("truncate");
	return NULL;
#endif /* posix_truncate_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("ftruncate", "fd:?X2?DFile?Dint,length:?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_ftruncate_f_impl(DeeObject *fd, DeeObject *length);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_ftruncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FTRUNCATE_DEF { "ftruncate", (DeeObject *)&posix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?X2?DFile?Dint,length:?Dint)") },
#define POSIX_FTRUNCATE_DEF_DOC(doc) { "ftruncate", (DeeObject *)&posix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?X2?DFile?Dint,length:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_ftruncate, &posix_ftruncate_f);
#ifndef POSIX_KWDS_FD_LENGTH_DEFINED
#define POSIX_KWDS_FD_LENGTH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd_length, { KEX("fd", 0x10561ad6, 0xce2e588d84c6793), KEX("length", 0xecef0c1, 0x2993e8eb119cab21), KEND });
#endif /* !POSIX_KWDS_FD_LENGTH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_ftruncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *fd;
	DeeObject *length;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_length, "oo:ftruncate", &fd, &length))
		goto err;
	return posix_ftruncate_f_impl(fd, length);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_ftruncate_f_impl(DeeObject *fd, DeeObject *length)
/*[[[end]]]*/
{
#ifdef posix_ftruncate_USE_DeeFile_Trunc
	if (DeeFile_Check(fd)) {
		Dee_pos_t used_length;
		if (DeeObject_AsUIntX(length, &used_length))
			goto err;
		if (DeeFile_Trunc(fd, used_length))
			goto err;
		return_none;
	} else
#endif /* posix_ftruncate_USE_DeeFile_Trunc */
	{
#ifdef posix_ftruncate_USE_posix_truncate
		DREF DeeObject *result;
		DREF DeeObject *abspath;
		abspath = posix_fd_makepath(fd);
#define NEED_posix_fd_makepath
		if unlikely(!abspath)
			goto err;
		result = posix_truncate_f_impl(abspath, length);
		Dee_Decref(abspath);
		return result;
#elif defined(posix_ftruncate_USE_STUB)
		(void)fd;
		(void)length;
		posix_err_unsupported("ftruncate");
#define NEED_posix_err_unsupported
		goto err;
#else /* ... */
		int error;
		int os_fd;
#ifdef posix_truncate_USE_pos_t
		posix_truncate_USE_pos_t used_length;
#endif /* posix_truncate_USE_pos_t */

		/* Decode arguments. */
		os_fd = DeeUnixSystem_GetFD(fd);
		if unlikely(os_fd == -1)
			goto err;
#ifdef posix_truncate_USE_pos_t
		if (DeeObject_AsUIntX(length, &used_length))
			goto err;
#endif /* posix_truncate_USE_pos_t */
	
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_ftruncate64
		error = ftruncate64(os_fd, used_length);
#else /* CONFIG_HAVE_ftruncate64 */
		error = ftruncate(os_fd, used_length);
#endif /* !CONFIG_HAVE_ftruncate64 */
		DBG_ALIGNMENT_ENABLE();
	
		if unlikely(error < 0) {
			error = DeeSystem_GetErrno();
			EINTR_HANDLE(error, again, err);
			ENOMEM_HANDLE(error, again, err);
			err_unix_ftruncate(error, fd, length);
#define NEED_err_unix_ftruncate
			goto err;
		}
		return_none;
#endif /* !... */
	}
err:
	return NULL;
}



/*[[[deemon import("rt.gen.dexutils").gw("ftruncateat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,length:?Dint,atflags:u=0", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_ftruncateat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *length, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_ftruncateat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FTRUNCATEAT_DEF { "ftruncateat", (DeeObject *)&posix_ftruncateat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,length:?Dint,atflags:?Dint=!0)") },
#define POSIX_FTRUNCATEAT_DEF_DOC(doc) { "ftruncateat", (DeeObject *)&posix_ftruncateat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,length:?Dint,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_ftruncateat, &posix_ftruncateat_f);
#ifndef POSIX_KWDS_DFD_PATH_LENGTH_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_LENGTH_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_length_atflags, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("length", 0xecef0c1, 0x2993e8eb119cab21), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_LENGTH_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_ftruncateat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	DeeObject *length;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_length_atflags, "ooo|u:ftruncateat", &dfd, &path, &length, &atflags))
		goto err;
	return posix_ftruncateat_f_impl(dfd, path, length, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_ftruncateat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *length, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_ftruncateat_USE_posix_truncate
	DREF DeeObject *result, *abspath;
#ifdef posix_ftruncateat_USE_posix_ftruncate
	if (atflags & AT_EMPTY_PATH) {
		if (DeeNone_Check(path) || (DeeString_Check(path) &&
		                            DeeString_IsEmpty(path)))
			return posix_ftruncate_f_impl(dfd, length);
		atflags &= ~AT_EMPTY_PATH;
	}
#endif /* posix_ftruncateat_USE_posix_ftruncate */
	abspath = posix_dfd_makepath(dfd, path, atflags);
	if unlikely(!abspath)
		goto err;
	result = posix_truncate_f_impl(abspath, length);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_ftruncateat_USE_posix_truncate */

#ifdef posix_ftruncateat_USE_STUB
	(void)dfd;
	(void)path;
	(void)length;
	(void)atflags;
	posix_err_unsupported("ftruncateat");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_ftruncateat_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("truncateat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,length:?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncateat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *length);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncateat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_TRUNCATEAT_DEF { "truncateat", (DeeObject *)&posix_truncateat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,length:?Dint)") },
#define POSIX_TRUNCATEAT_DEF_DOC(doc) { "truncateat", (DeeObject *)&posix_truncateat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,length:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_truncateat, &posix_truncateat_f);
#ifndef POSIX_KWDS_DFD_PATH_LENGTH_DEFINED
#define POSIX_KWDS_DFD_PATH_LENGTH_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_length, { KEX("dfd", 0x1c30614d, 0x6edb9568429a136f), KEX("path", 0x1ab74e01, 0xc2dd5992f362b3c4), KEX("length", 0xecef0c1, 0x2993e8eb119cab21), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_LENGTH_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncateat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	DeeObject *length;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_length, "ooo:truncateat", &dfd, &path, &length))
		goto err;
	return posix_truncateat_f_impl(dfd, path, length);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncateat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *length)
/*[[[end]]]*/
{
#ifdef posix_truncateat_USE_posix_truncate
	DREF DeeObject *result, *abspath;
	abspath = posix_dfd_makepath(dfd, path, 0);
	if unlikely(!abspath)
		goto err;
	result = posix_truncate_f_impl(abspath, length);
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_truncateat_USE_posix_truncate */

#ifdef posix_truncateat_USE_posix_ftruncateat
	return posix_ftruncateat_f_impl(dfd, path, length, 0);
#endif /* posix_truncateat_USE_posix_ftruncateat */

#ifdef posix_truncateat_USE_STUB
	(void)dfd;
	(void)path;
	(void)length;
	posix_err_unsupported("truncateat");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_truncateat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_TRUNCATE_C_INL */
