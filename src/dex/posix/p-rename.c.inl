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
#ifndef GUARD_DEX_POSIX_P_RENAME_C_INL
#define GUARD_DEX_POSIX_P_RENAME_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>

#include <hybrid/debug-alignment.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

/* Figure out how to implement `rename()' */
#undef posix_rename_USE_nt_MoveFileEx
#undef posix_rename_USE_wrename
#undef posix_rename_USE_rename
#undef posix_rename_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define posix_rename_USE_nt_MoveFileEx
#elif defined(CONFIG_HAVE_wrename) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_rename_USE_wrename
#elif defined(CONFIG_HAVE_rename)
#define posix_rename_USE_rename
#elif defined(CONFIG_HAVE_wrename)
#define posix_rename_USE_wrename
#else /* ... */
#define posix_rename_USE_STUB
#endif /* !... */



/* Figure out how to implement `frename()' */
#undef posix_frename_USE_posix_rename
#undef posix_frename_USE_STUB
#ifndef posix_rename_USE_STUB
#define posix_frename_USE_posix_rename
#else /* ... */
#define posix_frename_USE_STUB
#endif /* !... */



/* Figure out how to implement `renameat()' */
#undef posix_renameat_USE_posix_rename
#undef posix_renameat_USE_renameat2
#undef posix_renameat_USE_renameat
#undef posix_renameat_USE_STUB
#if (defined(CONFIG_HAVE_renameat2) && \
     (defined(AT_RENAME_NOREPLACE) || defined(AT_RENAME_EXCHANGE) || defined(AT_RENAME_WHITEOUT)))
#define posix_renameat_USE_posix_rename
#define posix_renameat_USE_renameat2
#elif defined(CONFIG_HAVE_renameat)
#define posix_renameat_USE_posix_rename
#define posix_renameat_USE_renameat
#elif !defined(posix_rename_USE_STUB)
#define posix_renameat_USE_posix_rename
#else /* ... */
#define posix_renameat_USE_STUB
#endif /* !... */



/* Figure out how to implement `renameat2()' */
#undef posix_renameat2_USE_posix_renameat
#undef posix_renameat2_USE_renameat2
#undef posix_renameat2_USE_nt_MoveFileEx
#undef posix_renameat2_USE_STUB
#ifdef CONFIG_HAVE_renameat2
#define posix_renameat2_USE_renameat2
#elif defined(CONFIG_HOST_WINDOWS)
#define posix_renameat2_USE_nt_MoveFileEx
#elif !defined(posix_renameat_USE_STUB)
#define posix_renameat2_USE_posix_renameat
#else /* ... */
#define posix_renameat2_USE_STUB
#endif /* !... */



/* Figure out how to implement `link()' */
#undef posix_link_USE_nt_CreateHardLink
#undef posix_link_USE_wlink
#undef posix_link_USE_link
#undef posix_link_USE_STUB
#if defined(CONFIG_HOST_WINDOWS)
#define posix_link_USE_nt_CreateHardLink
#elif defined(CONFIG_HAVE_wlink) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_link_USE_wlink
#elif defined(CONFIG_HAVE_link)
#define posix_link_USE_link
#elif defined(CONFIG_HAVE_wlink)
#define posix_link_USE_wlink
#else /* ... */
#define posix_link_USE_STUB
#endif /* !... */



/* Figure out how to implement `flink()' */
#undef posix_flink_USE_posix_link
#undef posix_flink_USE_linkat
#undef posix_flink_USE_STUB
#if (defined(CONFIG_HAVE_linkat) &&   \
     defined(CONFIG_HAVE_AT_FDCWD) && \
     defined(CONFIG_HAVE_AT_EMPTY_PATH))
#define posix_flink_USE_linkat
#define posix_flink_USE_posix_link
#elif !defined(posix_link_USE_STUB)
#define posix_flink_USE_posix_link
#else /* ... */
#define posix_flink_USE_STUB
#endif /* !... */



/* Figure out how to implement `linkat()' */
#undef posix_linkat_USE_posix_link
#undef posix_linkat_USE_linkat
#undef posix_linkat_USE_STUB
#ifdef CONFIG_HAVE_linkat
#define posix_linkat_USE_linkat
#define posix_linkat_USE_posix_link
#elif !defined(posix_link_USE_STUB) || !defined(posix_flink_USE_STUB)
#define posix_linkat_USE_posix_link
#else /* ... */
#define posix_linkat_USE_STUB
#endif /* !... */


#if (defined(posix_renameat2_USE_renameat2) || \
     defined(posix_renameat2_USE_posix_renameat))
#include "p-stat.c.inl"
#endif /* posix_renameat2_USE_renameat2 || posix_renameat2_USE_posix_renameat */


/*[[[deemon import("rt.gen.dexutils").gw("rename", "oldpath:?Dstring,newpath:?Dstring", libname: "posix");]]]*/
#define POSIX_RENAME_DEF          DEX_MEMBER_F("rename", &posix_rename, DEXSYM_READONLY, "(oldpath:?Dstring,newpath:?Dstring)"),
#define POSIX_RENAME_DEF_DOC(doc) DEX_MEMBER_F("rename", &posix_rename, DEXSYM_READONLY, "(oldpath:?Dstring,newpath:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_rename_f_impl(DeeObject *oldpath, DeeObject *newpath);
#ifndef DEFINED_kwlist__oldpath_newpath
#define DEFINED_kwlist__oldpath_newpath
PRIVATE DEFINE_KWLIST(kwlist__oldpath_newpath, { KEX("oldpath", 0x6af2d717, 0x74cfc4ae2e46bac3), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEND });
#endif /* !DEFINED_kwlist__oldpath_newpath */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rename_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *oldpath;
		DeeObject *newpath;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__oldpath_newpath, "oo:rename", &args))
		goto err;
	return posix_rename_f_impl(args.oldpath, args.newpath);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_rename, &posix_rename_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_rename_f_impl(DeeObject *oldpath, DeeObject *newpath)
/*[[[end]]]*/
{
#ifdef posix_rename_USE_nt_MoveFileEx
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(oldpath, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(newpath, &DeeString_Type))
		goto err;
again:
#define NEED_nt_MoveFileEx
	error = nt_MoveFileEx(oldpath, newpath, MOVEFILE_REPLACE_EXISTING);
	if likely(error == 0)
		return_none;
	if unlikely(error < 0)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}
#define NEED_err_nt_rename
	err_nt_rename(dwError, oldpath, newpath);
err:
	return NULL;
#endif /* posix_rename_USE_nt_MoveFileEx */

#if defined(posix_rename_USE_wrename) || defined(posix_rename_USE_rename)
	int error;
#ifdef posix_rename_USE_wrename
	Dee_wchar_t const *wide_oldpath, *wide_newpath;
#else /* posix_rename_USE_wrename */
	char const *utf8_oldpath, *utf8_newpath;
#endif /* !posix_rename_USE_wrename */
	if (DeeObject_AssertTypeExact(oldpath, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(newpath, &DeeString_Type))
		goto err;
#ifdef posix_rename_USE_wrename
	wide_oldpath = DeeString_AsWide(oldpath);
	if unlikely(!wide_oldpath)
		goto err;
	wide_newpath = DeeString_AsWide(newpath);
	if unlikely(!wide_newpath)
		goto err;
#else /* posix_rename_USE_wrename */
	utf8_oldpath = DeeString_AsUtf8(oldpath);
	if unlikely(!utf8_oldpath)
		goto err;
	utf8_newpath = DeeString_AsUtf8(newpath);
	if unlikely(!utf8_newpath)
		goto err;
#endif /* !posix_rename_USE_wrename */
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_rename_USE_wrename
	error = wrename((wchar_t *)wide_oldpath, (wchar_t *)wide_newpath);
#else /* posix_rename_USE_wrename */
	error = rename((char *)utf8_oldpath, (char *)utf8_newpath);
#endif /* !posix_rename_USE_wrename */
	DBG_ALIGNMENT_ENABLE();
	if likely(error == 0)
		return_none;
	error = DeeSystem_GetErrno();
	EINTR_HANDLE(error, again, err);
#define NEED_err_unix_rename
	err_unix_rename(error, oldpath, newpath);
err:
	return NULL;
#endif /* posix_rename_USE_wrename || posix_rename_USE_rename */

#ifdef posix_rename_USE_STUB
	(void)oldpath;
	(void)newpath;
#define NEED_posix_err_unsupported
	posix_err_unsupported("rename");
	return NULL;
#endif /* posix_rename_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("frename", "oldfd:?X2?DFile?Dint,newpath:?Dstring", libname: "posix");]]]*/
#define POSIX_FRENAME_DEF          DEX_MEMBER_F("frename", &posix_frename, DEXSYM_READONLY, "(oldfd:?X2?DFile?Dint,newpath:?Dstring)"),
#define POSIX_FRENAME_DEF_DOC(doc) DEX_MEMBER_F("frename", &posix_frename, DEXSYM_READONLY, "(oldfd:?X2?DFile?Dint,newpath:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_frename_f_impl(DeeObject *oldfd, DeeObject *newpath);
#ifndef DEFINED_kwlist__oldfd_newpath
#define DEFINED_kwlist__oldfd_newpath
PRIVATE DEFINE_KWLIST(kwlist__oldfd_newpath, { KEX("oldfd", 0x5a92fcdb, 0x3de145419f68339e), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEND });
#endif /* !DEFINED_kwlist__oldfd_newpath */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_frename_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *oldfd;
		DeeObject *newpath;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__oldfd_newpath, "oo:frename", &args))
		goto err;
	return posix_frename_f_impl(args.oldfd, args.newpath);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_frename, &posix_frename_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_frename_f_impl(DeeObject *oldfd, DeeObject *newpath)
/*[[[end]]]*/
{
#ifdef posix_frename_USE_posix_rename
	DREF DeeObject *result, *oldfd_abspath;
#define NEED_posix_fd_makepath
	oldfd_abspath = posix_fd_makepath(oldfd);
	if unlikely(!oldfd_abspath)
		goto err;
	result = posix_rename_f_impl(oldfd_abspath, newpath);
	Dee_Decref(oldfd_abspath);
	return result;
err:
	return NULL;
#endif /* posix_frename_USE_posix_rename */

#ifdef posix_frename_USE_STUB
	(void)oldfd;
	(void)newpath;
#define NEED_posix_err_unsupported
	posix_err_unsupported("frename");
	return NULL;
#endif /* posix_frename_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("renameat",
	"olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,"
	"newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,"
	"atflags:u=0", libname: "posix");]]]*/
#define POSIX_RENAMEAT_DEF          DEX_MEMBER_F("renameat", &posix_renameat, DEXSYM_READONLY, "(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,atflags=!0)"),
#define POSIX_RENAMEAT_DEF_DOC(doc) DEX_MEMBER_F("renameat", &posix_renameat, DEXSYM_READONLY, "(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL posix_renameat_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int atflags);
#ifndef DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_atflags
#define DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_atflags
PRIVATE DEFINE_KWLIST(kwlist__olddirfd_oldpath_newdirfd_newpath_atflags, { KEX("olddirfd", 0xfce5716b, 0x69852ead3adcc550), KEX("oldpath", 0x6af2d717, 0x74cfc4ae2e46bac3), KEX("newdirfd", 0xcef3d13f, 0x767804c5c500a418), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_renameat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *olddirfd;
		DeeObject *oldpath;
		DeeObject *newdirfd;
		DeeObject *newpath;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__olddirfd_oldpath_newdirfd_newpath_atflags, "oooo|u:renameat", &args))
		goto err;
	return posix_renameat_f_impl(args.olddirfd, args.oldpath, args.newdirfd, args.newpath, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_renameat, &posix_renameat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL posix_renameat_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_renameat_USE_posix_rename
	DREF DeeObject *result;
	DREF DeeObject *abs_oldpath;
	DREF DeeObject *abs_newpath;
#if defined(posix_renameat_USE_renameat2) || defined(posix_renameat_USE_renameat)
	if ((!DeeString_Check(olddirfd) && DeeString_Check(oldpath)) &&
	    (!DeeString_Check(newdirfd) && DeeString_Check(newpath))
#ifndef posix_renameat_USE_renameat2
	    && (atflags == 0)
#endif /* !posix_renameat_USE_renameat2 */
	    ) {
		int os_olddirfd, os_newdirfd;
		char const *utf8_oldpath, *utf8_newpath;
		os_olddirfd = DeeUnixSystem_GetFD(olddirfd);
		if unlikely(os_olddirfd == -1)
			goto err;
		os_newdirfd = DeeUnixSystem_GetFD(newdirfd);
		if unlikely(os_newdirfd == -1)
			goto err;
		utf8_oldpath = DeeString_AsUtf8(oldpath);
		if unlikely(!utf8_oldpath)
			goto err;
		utf8_newpath = DeeString_AsUtf8(newpath);
		if unlikely(!utf8_newpath)
			goto err;
#ifdef posix_renameat_USE_renameat2
		if (renameat2(os_olddirfd, (char *)utf8_oldpath, os_newdirfd, (char *)utf8_newpath, atflags) == 0)
#else /* posix_renameat_USE_renameat2 */
		if (renameat(os_olddirfd, (char *)utf8_oldpath, os_newdirfd, (char *)utf8_newpath) == 0)
#endif /* !posix_renameat_USE_renameat2 */
		{
			return_none;
		}
		/* Fallthru to fallback path below */
	}
#endif /* posix_renameat_USE_renameat2 || posix_renameat_USE_renameat */

#define NEED_posix_dfd_makepath
	abs_oldpath = posix_dfd_makepath(olddirfd, oldpath, atflags);
	if unlikely(!abs_oldpath)
		goto err;
#define NEED_posix_dfd_makepath
	abs_newpath = posix_dfd_makepath(newdirfd, newpath, atflags);
	if unlikely(!abs_newpath)
		goto err_abs_oldpath;
	result = posix_rename_f_impl(abs_oldpath, abs_newpath);
	Dee_Decref(abs_newpath);
	Dee_Decref(abs_oldpath);
	return result;
err_abs_oldpath:
	Dee_Decref(abs_oldpath);
err:
	return NULL;
#endif /* posix_renameat_USE_posix_rename */

#ifdef posix_renameat_USE_STUB
	(void)olddirfd;
	(void)oldpath;
	(void)newdirfd;
	(void)newpath;
	(void)atflags;
#define NEED_posix_err_unsupported
	posix_err_unsupported("renameat");
	return NULL;
#endif /* posix_renameat_USE_STUB */
}


/*[[[deemon import("rt.gen.dexutils").gw("renameat2",
	"olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,"
	"newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,"
	"flags:u=0,atflags:u=0", libname: "posix");]]]*/
#define POSIX_RENAMEAT2_DEF          DEX_MEMBER_F("renameat2", &posix_renameat2, DEXSYM_READONLY, "(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,flags=!0,atflags=!0)"),
#define POSIX_RENAMEAT2_DEF_DOC(doc) DEX_MEMBER_F("renameat2", &posix_renameat2, DEXSYM_READONLY, "(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,flags=!0,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL posix_renameat2_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int flags, unsigned int atflags);
#ifndef DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags
#define DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags
PRIVATE DEFINE_KWLIST(kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags, { KEX("olddirfd", 0xfce5716b, 0x69852ead3adcc550), KEX("oldpath", 0x6af2d717, 0x74cfc4ae2e46bac3), KEX("newdirfd", 0xcef3d13f, 0x767804c5c500a418), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_renameat2_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *olddirfd;
		DeeObject *oldpath;
		DeeObject *newdirfd;
		DeeObject *newpath;
		unsigned int flags;
		unsigned int atflags;
	} args;
	args.flags = 0;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags, "oooo|uu:renameat2", &args))
		goto err;
	return posix_renameat2_f_impl(args.olddirfd, args.oldpath, args.newdirfd, args.newpath, args.flags, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_renameat2, &posix_renameat2_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL posix_renameat2_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int flags, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_renameat2_USE_nt_MoveFileEx
	DREF DeeObject *abs_oldpath;
	DREF DeeObject *abs_newpath;
	if (flags & RENAME_NOREPLACE) {
		int error;
		DWORD dwError;
		abs_oldpath = posix_dfd_makepath(olddirfd, oldpath, atflags);
		if unlikely(!abs_oldpath)
			goto err;
#define NEED_posix_dfd_makepath
		abs_newpath = posix_dfd_makepath(newdirfd, newpath, atflags);
		if unlikely(!abs_newpath)
			goto err_abs_oldpath;
again:
#define NEED_nt_MoveFileEx
		error = nt_MoveFileEx(abs_oldpath, abs_newpath, 0);
		if likely(error == 0) {
			Dee_Decref(abs_newpath);
			Dee_Decref(abs_oldpath);
			return_none;
		}
		if unlikely(error < 0)
			goto err_abs_oldpath_abs_newpath;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_abs_oldpath_abs_newpath;
			goto again;
		}
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err_abs_oldpath_abs_newpath;
		}
#define NEED_err_nt_rename
		err_nt_rename(dwError, abs_oldpath, abs_newpath);
		goto err_abs_oldpath_abs_newpath;
	}
	if (flags != 0) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}
	return posix_renameat_f_impl(olddirfd, oldpath, newdirfd, newpath, atflags);
err_abs_oldpath_abs_newpath:
	Dee_Decref(abs_newpath);
err_abs_oldpath:
	Dee_Decref(abs_oldpath);
err:
	return NULL;
#endif /* posix_renameat2_USE_nt_MoveFileEx */

#ifdef posix_renameat2_USE_renameat2
#if !defined(AT_RENAME_NOREPLACE) && !defined(AT_RENAME_EXCHANGE) && !defined(AT_RENAME_WHITEOUT)
	if (atflags == 0)
#endif /* !AT_RENAME_NOREPLACE && !AT_RENAME_EXCHANGE && !AT_RENAME_WHITEOUT */
	{
		int errno_value;
		int os_olddirfd, os_newdirfd;
		char const *utf8_oldpath, *utf8_newpath;
		DREF DeeObject *abs_oldpath;
		DREF DeeObject *abs_newpath;
		os_olddirfd = DeeUnixSystem_GetFD(olddirfd);
		if unlikely(os_olddirfd == -1)
			goto err;
		os_newdirfd = DeeUnixSystem_GetFD(newdirfd);
		if unlikely(os_newdirfd == -1)
			goto err;
		utf8_oldpath = DeeString_AsUtf8(oldpath);
		if unlikely(!utf8_oldpath)
			goto err;
		utf8_newpath = DeeString_AsUtf8(newpath);
		if unlikely(!utf8_newpath)
			goto err;

		/* Name the call to `renameat2()' */
EINTR_ENOMEM_LABEL(again_renameat2)
#if defined(AT_RENAME_NOREPLACE) || defined(AT_RENAME_EXCHANGE) || defined(AT_RENAME_WHITEOUT)
		if (renameat2(os_olddirfd, (char *)utf8_oldpath, os_newdirfd, (char *)utf8_newpath, flags | atflags) == 0)
#else /* AT_RENAME_NOREPLACE || AT_RENAME_EXCHANGE || AT_RENAME_WHITEOUT */
		if (renameat2(os_olddirfd, (char *)utf8_oldpath, os_newdirfd, (char *)utf8_newpath, flags) == 0)
#endif /* !AT_RENAME_NOREPLACE && !AT_RENAME_EXCHANGE && !AT_RENAME_WHITEOUT */
		{
			return_none;
		}

		/* Directly handle rename errors */
		errno_value = DeeSystem_GetErrno();
		EINTR_ENOMEM_HANDLE(errno_value, again_renameat2, err);
#define NEED_posix_dfd_makepath
		abs_oldpath = posix_dfd_makepath(olddirfd, oldpath, atflags & POSIX_DFD_MAKEPATH_ATFLAGS_MASK);
		if unlikely(!abs_oldpath)
			goto err;
#define NEED_posix_dfd_makepath
		abs_newpath = posix_dfd_makepath(newdirfd, newpath, atflags & POSIX_DFD_MAKEPATH_ATFLAGS_MASK);
		if unlikely(!abs_newpath)
			goto err_abs_oldpath;
		err_unix_rename(errno_value, abs_oldpath, abs_newpath);
		Dee_Decref(abs_newpath);
		Dee_Decref(abs_oldpath);
		goto err;
err_abs_oldpath:
		Dee_Decref(abs_oldpath);
		goto err;
	}
#if !defined(AT_RENAME_NOREPLACE) && !defined(AT_RENAME_EXCHANGE) && !defined(AT_RENAME_WHITEOUT)
#undef posix_renameat2_USE_posix_renameat
#define posix_renameat2_USE_posix_renameat
	/* Fallthru to `posix_renameat2_USE_posix_renameat' */
#else /* !AT_RENAME_NOREPLACE && !AT_RENAME_EXCHANGE && !AT_RENAME_WHITEOUT */
err:
	return NULL;
#endif /* AT_RENAME_NOREPLACE || AT_RENAME_EXCHANGE || AT_RENAME_WHITEOUT */
#endif /* posix_renameat2_USE_renameat2 */

#ifdef posix_renameat2_USE_posix_renameat
	if (flags & RENAME_NOREPLACE) {
		/* Check if `newpath' already exists (and if so: fail)
		 *
		 * Note that this isn't 100% safe since some other process
		 * may create the file after we checked for it, but this is
		 * the best we can do (since without `renameat2(2)', there
		 * literally isn't a system call that would make the check
		 * for us) */
		struct dee_stat st;
		int status;
		/* TODO: Use `faccessat(..., F_OK, AT_SYMLINK_NOFOLLOW)' if available */
		status = dee_stat_init(&st, newdirfd, newpath,
		                       atflags | AT_SYMLINK_NOFOLLOW | DEE_STAT_F_TRY);
		if unlikely(status < 0)
			goto err;
		if (status == 0) {
			/* File already exists */
			DREF DeeObject *abs_newpath;
#define NEED_posix_dfd_makepath
			abs_newpath = posix_dfd_makepath(newdirfd, newpath, atflags & POSIX_DFD_MAKEPATH_ATFLAGS_MASK);
			if unlikely(!abs_newpath)
				goto err;
#define NEED_err_unix_path_exists
#ifdef EEXIST
			err_unix_path_exists(EEXIST, abs_newpath);
#elif defined(EINVAL)
			err_unix_path_exists(EINVAL, abs_newpath);
#else /* ... */
			err_unix_path_exists(1, abs_newpath);
#endif /* !... */
			Dee_Decref(abs_newpath);
			goto err;
		}
	}
	if (flags != 0) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}
	return posix_renameat_f_impl(olddirfd, oldpath, newdirfd, newpath, atflags);
err:
	return NULL;
#endif /* posix_renameat2_USE_posix_renameat */

#ifdef posix_renameat2_USE_STUB
	(void)olddirfd;
	(void)oldpath;
	(void)newdirfd;
	(void)newpath;
	(void)flags;
	(void)atflags;
#define NEED_posix_err_unsupported
	posix_err_unsupported("renameat2");
	return NULL;
#endif /* posix_renameat2_USE_STUB */
}




/*[[[deemon import("rt.gen.dexutils").gw("link", "oldpath:?Dstring,newpath:?Dstring", libname: "posix");]]]*/
#define POSIX_LINK_DEF          DEX_MEMBER_F("link", &posix_link, DEXSYM_READONLY, "(oldpath:?Dstring,newpath:?Dstring)"),
#define POSIX_LINK_DEF_DOC(doc) DEX_MEMBER_F("link", &posix_link, DEXSYM_READONLY, "(oldpath:?Dstring,newpath:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_link_f_impl(DeeObject *oldpath, DeeObject *newpath);
#ifndef DEFINED_kwlist__oldpath_newpath
#define DEFINED_kwlist__oldpath_newpath
PRIVATE DEFINE_KWLIST(kwlist__oldpath_newpath, { KEX("oldpath", 0x6af2d717, 0x74cfc4ae2e46bac3), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEND });
#endif /* !DEFINED_kwlist__oldpath_newpath */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_link_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *oldpath;
		DeeObject *newpath;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__oldpath_newpath, "oo:link", &args))
		goto err;
	return posix_link_f_impl(args.oldpath, args.newpath);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_link, &posix_link_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_link_f_impl(DeeObject *oldpath, DeeObject *newpath)
/*[[[end]]]*/
{
#ifdef posix_link_USE_nt_CreateHardLink
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(oldpath, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(newpath, &DeeString_Type))
		goto err;
again:
#define NEED_nt_CreateHardLink
	error = nt_CreateHardLink(oldpath, newpath, NULL);
	if likely(error == 0)
		return_none;
	if unlikely(error < 0)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	if (DeeNTSystem_IsIntr(dwError)) {
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	if (DeeNTSystem_IsBadAllocError(dwError)) {
		if (Dee_CollectMemory(1))
			goto again;
		goto err;
	}
#define NEED_err_nt_link
	err_nt_link(dwError, oldpath, newpath);
err:
	return NULL;
#endif /* posix_link_USE_nt_CreateHardLink */

#if defined(posix_link_USE_wlink) || defined(posix_link_USE_link)
	int error;
#ifdef posix_link_USE_wlink
	Dee_wchar_t const *wide_oldpath, *wide_newpath;
	wide_oldpath = DeeString_AsWide(oldpath);
	if unlikely(!wide_oldpath)
		goto err;
	wide_newpath = DeeString_AsWide(newpath);
	if unlikely(!wide_newpath)
		goto err;
#else /* posix_link_USE_wlink */
	char const *utf8_oldpath, *utf8_newpath;
	utf8_oldpath = DeeString_AsUtf8(oldpath);
	if unlikely(!utf8_oldpath)
		goto err;
	utf8_newpath = DeeString_AsUtf8(newpath);
	if unlikely(!utf8_newpath)
		goto err;
#endif /* !posix_link_USE_wlink */
EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_link_USE_wlink
	error = wlink((wchar_t *)wide_oldpath, (wchar_t *)wide_newpath);
#else /* posix_link_USE_wlink */
	error = link((char *)utf8_oldpath, (char *)utf8_newpath);
#endif /* !posix_link_USE_wlink */
	DBG_ALIGNMENT_ENABLE();
	if likely(error == 0)
		return_none;
	error = DeeSystem_GetErrno();
	EINTR_ENOMEM_HANDLE(error, again, err);
#define NEED_err_unix_link
	err_unix_link(error, oldpath, newpath);
err:
	return NULL;
#endif /* posix_link_USE_wlink || posix_link_USE_link */

#ifdef posix_link_USE_STUB
	(void)oldpath;
	(void)newpath;
#define NEED_posix_err_unsupported
	posix_err_unsupported("link");
	return NULL;
#endif /* posix_link_USE_STUB */
}




/*[[[deemon import("rt.gen.dexutils").gw("flink", "oldfd:?X2?DFile?Dint,newpath:?Dstring", libname: "posix");]]]*/
#define POSIX_FLINK_DEF          DEX_MEMBER_F("flink", &posix_flink, DEXSYM_READONLY, "(oldfd:?X2?DFile?Dint,newpath:?Dstring)"),
#define POSIX_FLINK_DEF_DOC(doc) DEX_MEMBER_F("flink", &posix_flink, DEXSYM_READONLY, "(oldfd:?X2?DFile?Dint,newpath:?Dstring)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_flink_f_impl(DeeObject *oldfd, DeeObject *newpath);
#ifndef DEFINED_kwlist__oldfd_newpath
#define DEFINED_kwlist__oldfd_newpath
PRIVATE DEFINE_KWLIST(kwlist__oldfd_newpath, { KEX("oldfd", 0x5a92fcdb, 0x3de145419f68339e), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEND });
#endif /* !DEFINED_kwlist__oldfd_newpath */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_flink_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *oldfd;
		DeeObject *newpath;
	} args;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__oldfd_newpath, "oo:flink", &args))
		goto err;
	return posix_flink_f_impl(args.oldfd, args.newpath);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_flink, &posix_flink_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL posix_flink_f_impl(DeeObject *oldfd, DeeObject *newpath)
/*[[[end]]]*/
{
#ifdef posix_flink_USE_posix_link
	DREF DeeObject *result;
	DREF DeeObject *oldfd_abspath;
#ifdef posix_flink_USE_linkat
	if (!DeeString_Check(oldfd)) {
		int os_oldfd;
		char const *utf8_newpath;
		os_oldfd = DeeUnixSystem_GetFD(oldfd);
		if unlikely(os_oldfd == -1)
			goto err;
		utf8_newpath = DeeString_AsUtf8(newpath);
		if unlikely(!utf8_newpath)
			goto err;
		if (linkat(os_oldfd, (char *)"", AT_FDCWD, (char *)utf8_newpath, AT_EMPTY_PATH) == 0)
			return_none;
		/* Fallthru to fallback path below */
	}
#endif /* posix_flink_USE_linkat */
#define NEED_posix_fd_makepath
	oldfd_abspath = posix_fd_makepath(oldfd);
	if unlikely(!oldfd_abspath)
		goto err;
	result = posix_link_f_impl(oldfd_abspath, newpath);
	Dee_Decref(oldfd_abspath);
	return result;
err:
	return NULL;
#endif /* posix_flink_USE_posix_link */

#ifdef posix_flink_USE_STUB
	(void)oldfd;
	(void)newpath;
#define NEED_posix_err_unsupported
	posix_err_unsupported("flink");
	return NULL;
#endif /* posix_flink_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("linkat", "olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,atflags:u=0", libname: "posix");]]]*/
#define POSIX_LINKAT_DEF          DEX_MEMBER_F("linkat", &posix_linkat, DEXSYM_READONLY, "(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,atflags=!0)"),
#define POSIX_LINKAT_DEF_DOC(doc) DEX_MEMBER_F("linkat", &posix_linkat, DEXSYM_READONLY, "(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,atflags=!0)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL posix_linkat_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int atflags);
#ifndef DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_atflags
#define DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_atflags
PRIVATE DEFINE_KWLIST(kwlist__olddirfd_oldpath_newdirfd_newpath_atflags, { KEX("olddirfd", 0xfce5716b, 0x69852ead3adcc550), KEX("oldpath", 0x6af2d717, 0x74cfc4ae2e46bac3), KEX("newdirfd", 0xcef3d13f, 0x767804c5c500a418), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEND });
#endif /* !DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_atflags */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_linkat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *olddirfd;
		DeeObject *oldpath;
		DeeObject *newdirfd;
		DeeObject *newpath;
		unsigned int atflags;
	} args;
	args.atflags = 0;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__olddirfd_oldpath_newdirfd_newpath_atflags, "oooo|u:linkat", &args))
		goto err;
	return posix_linkat_f_impl(args.olddirfd, args.oldpath, args.newdirfd, args.newpath, args.atflags);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_linkat, &posix_linkat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL posix_linkat_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_linkat_USE_posix_link
	DREF DeeObject *result;
	DREF DeeObject *newpath_abspath;
	DREF DeeObject *oldpath_abspath;
#ifdef posix_linkat_USE_linkat
	if ((!DeeString_Check(olddirfd) && DeeString_Check(oldpath)) &&
	    (!DeeString_Check(newdirfd) && DeeString_Check(newpath))) {
		int error;
		int os_olddirfd, os_newdirfd;
		char const *utf8_oldpath, *utf8_newpath;
		os_olddirfd = DeeUnixSystem_GetFD(olddirfd);
		if unlikely(os_olddirfd == -1)
			goto err;
		os_newdirfd = DeeUnixSystem_GetFD(newdirfd);
		if unlikely(os_newdirfd == -1)
			goto err;
		utf8_oldpath = DeeString_AsUtf8(oldpath);
		if unlikely(!utf8_oldpath)
			goto err;
		utf8_newpath = DeeString_AsUtf8(newpath);
		if unlikely(!utf8_newpath)
			goto err;
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		error = linkat(os_olddirfd, (char *)utf8_oldpath,
		               os_newdirfd, (char *)utf8_newpath,
		               atflags);
		DBG_ALIGNMENT_ENABLE();
		if likely(error == 0)
			return_none;
		error = DeeSystem_GetErrno();
		EINTR_ENOMEM_HANDLE(error, again, err);
#define NEED_posix_dfd_makepath
		newpath_abspath = posix_dfd_makepath(newdirfd, newpath, atflags & POSIX_DFD_MAKEPATH_ATFLAGS_MASK);
		if unlikely(!newpath_abspath)
			goto err;
#define NEED_posix_dfd_makepath
		oldpath_abspath = posix_dfd_makepath(olddirfd, oldpath, atflags & POSIX_DFD_MAKEPATH_ATFLAGS_MASK);
		if unlikely(!oldpath_abspath)
			goto err_newfd_abspath;
#define NEED_err_unix_link
		err_unix_link(error, oldpath_abspath, newpath_abspath);
		Dee_Decref(oldpath_abspath);
		Dee_Decref(newpath_abspath);
		goto err;
	}
#endif /* posix_linkat_USE_linkat */

#define NEED_posix_dfd_makepath
	newpath_abspath = posix_dfd_makepath(newdirfd, newpath, atflags & ~AT_EMPTY_PATH);
	if unlikely(!newpath_abspath)
		goto err;
	if ((atflags & AT_EMPTY_PATH) &&
	    (DeeNone_Check(oldpath) || (DeeString_Check(oldpath) && DeeString_IsEmpty(oldpath))) &&
	    !DeeString_Check(olddirfd)) {
		/* Special case: create a hardlink for the file specified by `olddirfd' */
		result = posix_flink_f_impl(olddirfd, newpath_abspath);
	} else {
		oldpath_abspath = posix_dfd_makepath(olddirfd, oldpath, atflags);
		if unlikely(!oldpath_abspath)
			goto err_newfd_abspath;
		result = posix_link_f_impl(oldpath_abspath, newpath_abspath);
		Dee_Decref(oldpath_abspath);
	}
	Dee_Decref(newpath_abspath);
	return result;
err_newfd_abspath:
	Dee_Decref(newpath_abspath);
err:
	return NULL;
#endif /* posix_linkat_USE_posix_link */

#ifdef posix_linkat_USE_STUB
	(void)olddirfd;
	(void)oldpath;
	(void)newdirfd;
	(void)newpath;
	(void)atflags;
#define NEED_posix_err_unsupported
	posix_err_unsupported("linkat");
	return NULL;
#endif /* posix_linkat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_RENAME_C_INL */
