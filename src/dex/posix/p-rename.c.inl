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
#ifndef GUARD_DEX_POSIX_P_RENAME_C_INL
#define GUARD_DEX_POSIX_P_RENAME_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include "p-stat.c.inl"

DECL_BEGIN

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

#undef posix_frename_USE_posix_rename
#undef posix_frename_USE_STUB
#ifndef posix_rename_USE_STUB
#define posix_frename_USE_posix_rename
#else /* ... */
#define posix_frename_USE_STUB
#endif /* !... */

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


/*[[[deemon import("_dexutils").gw("rename", "oldname:?Dstring,newname:?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_rename_f_impl(DeeObject *oldname, DeeObject *newname);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rename_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_RENAME_DEF { "rename", (DeeObject *)&posix_rename, MODSYM_FNORMAL, DOC("(oldname:?Dstring,newname:?Dstring)") },
#define POSIX_RENAME_DEF_DOC(doc) { "rename", (DeeObject *)&posix_rename, MODSYM_FNORMAL, DOC("(oldname:?Dstring,newname:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_rename, posix_rename_f);
#ifndef POSIX_KWDS_OLDNAME_NEWNAME_DEFINED
#define POSIX_KWDS_OLDNAME_NEWNAME_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_oldname_newname, { K(oldname), K(newname), KEND });
#endif /* !POSIX_KWDS_OLDNAME_NEWNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_rename_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *oldname;
	DeeObject *newname;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldname_newname, "oo:rename", &oldname, &newname))
		goto err;
	return posix_rename_f_impl(oldname, newname);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_rename_f_impl(DeeObject *oldname, DeeObject *newname)
/*[[[end]]]*/
{
#ifdef posix_rename_USE_nt_MoveFileEx
	int error;
	DWORD dwError;
	if (DeeObject_AssertTypeExact(oldname, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(newname, &DeeString_Type))
		goto err;
again:
#define NEED_nt_MoveFileEx
	error = nt_MoveFileEx(oldname, newname, MOVEFILE_REPLACE_EXISTING);
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
	err_nt_rename(dwError, oldname, newname);
err:
	return NULL;
#endif /* posix_rename_USE_nt_MoveFileEx */

#if defined(posix_rename_USE_wrename) || defined(posix_rename_USE_rename)
	int error;
#ifdef posix_rename_USE_wrename
	dwchar_t *wide_oldname, *wide_newname;
#else /* posix_rename_USE_wrename */
	char *utf8_oldname, *utf8_newname;
#endif /* !posix_rename_USE_wrename */
	if (DeeObject_AssertTypeExact(oldname, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(newname, &DeeString_Type))
		goto err;
#ifdef posix_rename_USE_wrename
	wide_oldname = DeeString_AsWide(oldname);
	if unlikely(!wide_oldname)
		goto err;
	wide_newname = DeeString_AsWide(newname);
	if unlikely(!wide_newname)
		goto err;
#else /* posix_rename_USE_wrename */
	utf8_oldname = DeeString_AsUtf8(oldname);
	if unlikely(!utf8_oldname)
		goto err;
	utf8_newname = DeeString_AsUtf8(newname);
	if unlikely(!utf8_newname)
		goto err;
#endif /* !posix_rename_USE_wrename */
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_rename_USE_wrename
	error = wrename(wide_oldname, wide_newname);
#else /* posix_rename_USE_wrename */
	error = rename(utf8_oldname, utf8_newname);
#endif /* !posix_rename_USE_wrename */
	DBG_ALIGNMENT_ENABLE();
	if likely(error == 0)
		return_none;
	error = DeeSystem_GetErrno();
	EINTR_HANDLE(error, again, err);
#define NEED_err_unix_rename
	err_unix_rename(error, oldname, newname);
err:
	return NULL;
#endif /* posix_rename_USE_wrename || posix_rename_USE_rename */

#ifdef posix_rename_USE_STUB
	(void)oldname;
	(void)newname;
#define NEED_posix_err_unsupported
	posix_err_unsupported("rename");
	return NULL;
#endif /* posix_rename_USE_STUB */
}


/*[[[deemon import("_dexutils").gw("frename", "oldfd:?X2?DFile?Dint,newname:?Dstring", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_frename_f_impl(DeeObject *oldfd, DeeObject *newname);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_frename_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FRENAME_DEF { "frename", (DeeObject *)&posix_frename, MODSYM_FNORMAL, DOC("(oldfd:?X2?DFile?Dint,newname:?Dstring)") },
#define POSIX_FRENAME_DEF_DOC(doc) { "frename", (DeeObject *)&posix_frename, MODSYM_FNORMAL, DOC("(oldfd:?X2?DFile?Dint,newname:?Dstring)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_frename, posix_frename_f);
#ifndef POSIX_KWDS_OLDFD_NEWNAME_DEFINED
#define POSIX_KWDS_OLDFD_NEWNAME_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_oldfd_newname, { K(oldfd), K(newname), KEND });
#endif /* !POSIX_KWDS_OLDFD_NEWNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_frename_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *oldfd;
	DeeObject *newname;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldfd_newname, "oo:frename", &oldfd, &newname))
		goto err;
	return posix_frename_f_impl(oldfd, newname);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_frename_f_impl(DeeObject *oldfd, DeeObject *newname)
/*[[[end]]]*/
{
#ifdef posix_frename_USE_posix_rename
	DREF DeeObject *result, *oldfd_abspath;
	oldfd_abspath = posix_fd_abspath(oldfd);
	if unlikely(!oldfd_abspath)
		goto err;
	result = posix_rename_f_impl(oldfd_abspath, newname);
	Dee_Decref(oldfd_abspath);
	return result;
err:
	return NULL;
#endif /* posix_frename_USE_posix_rename */

#ifdef posix_frename_USE_STUB
	(void)oldfd;
	(void)newname;
#define NEED_posix_err_unsupported
	posix_err_unsupported("frename");
	return NULL;
#endif /* posix_frename_USE_STUB */
}


/*[[[deemon import("_dexutils").gw("renameat",
	"olddfd:?X3?DFile?Dint?Dstring,oldname:?Dstring,"
	"newdfd:?X3?DFile?Dint?Dstring,newname:?Dstring,"
	"atflags:u=0", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_renameat_f_impl(DeeObject *olddfd, DeeObject *oldname, DeeObject *newdfd, DeeObject *newname, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_renameat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_RENAMEAT_DEF { "renameat", (DeeObject *)&posix_renameat, MODSYM_FNORMAL, DOC("(olddfd:?X3?DFile?Dint?Dstring,oldname:?Dstring,newdfd:?X3?DFile?Dint?Dstring,newname:?Dstring,atflags:?Dint=!0)") },
#define POSIX_RENAMEAT_DEF_DOC(doc) { "renameat", (DeeObject *)&posix_renameat, MODSYM_FNORMAL, DOC("(olddfd:?X3?DFile?Dint?Dstring,oldname:?Dstring,newdfd:?X3?DFile?Dint?Dstring,newname:?Dstring,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_renameat, posix_renameat_f);
#ifndef POSIX_KWDS_OLDDFD_OLDNAME_NEWDFD_NEWNAME_ATFLAGS_DEFINED
#define POSIX_KWDS_OLDDFD_OLDNAME_NEWDFD_NEWNAME_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_olddfd_oldname_newdfd_newname_atflags, { K(olddfd), K(oldname), K(newdfd), K(newname), K(atflags), KEND });
#endif /* !POSIX_KWDS_OLDDFD_OLDNAME_NEWDFD_NEWNAME_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_renameat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *olddfd;
	DeeObject *oldname;
	DeeObject *newdfd;
	DeeObject *newname;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_olddfd_oldname_newdfd_newname_atflags, "oooo|u:renameat", &olddfd, &oldname, &newdfd, &newname, &atflags))
		goto err;
	return posix_renameat_f_impl(olddfd, oldname, newdfd, newname, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_renameat_f_impl(DeeObject *olddfd, DeeObject *oldname, DeeObject *newdfd, DeeObject *newname, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_renameat_USE_posix_rename
	DREF DeeObject *result;
	DREF DeeObject *abs_oldname;
	DREF DeeObject *abs_newname;
#if defined(posix_renameat_USE_renameat2) || defined(posix_renameat_USE_renameat)
	if ((!DeeString_Check(olddfd) && DeeString_Check(oldname)) &&
	    (!DeeString_Check(newdfd) && DeeString_Check(newname))
#ifndef posix_renameat_USE_renameat2
	    && (atflags == 0)
#endif /* !posix_renameat_USE_renameat2 */
	    ) {
		int os_olddfd, os_newdfd;
		char *utf8_oldname, *utf8_newname;
		os_olddfd = DeeUnixSystem_GetFD(olddfd);
		if unlikely(os_olddfd == -1)
			goto err;
		os_newdfd = DeeUnixSystem_GetFD(newdfd);
		if unlikely(os_newdfd == -1)
			goto err;
		utf8_oldname = DeeString_AsUtf8(oldname);
		if unlikely(!utf8_oldname)
			goto err;
		utf8_newname = DeeString_AsUtf8(newname);
		if unlikely(!utf8_newname)
			goto err;
#ifdef posix_renameat_USE_renameat2
		if (renameat2(os_olddfd, utf8_oldname, os_newdfd, utf8_newname, atflags) == 0)
#else /* posix_renameat_USE_renameat2 */
		if (renameat(os_olddfd, utf8_oldname, os_newdfd, utf8_newname) == 0)
#endif /* !posix_renameat_USE_renameat2 */
		{
			return_none;
		}
		/* Fallthru to fallback path below */
	}
#endif /* posix_renameat_USE_renameat2 || posix_renameat_USE_renameat */

	abs_oldname = posix_dfd_abspath(olddfd, oldname, atflags);
	if unlikely(!abs_oldname)
		goto err;
	abs_newname = posix_dfd_abspath(newdfd, newname, atflags);
	if unlikely(!abs_newname)
		goto err_abs_oldname;
	result = posix_rename_f_impl(abs_oldname, abs_newname);
	Dee_Decref(abs_newname);
	Dee_Decref(abs_oldname);
	return result;
err_abs_oldname:
	Dee_Decref(abs_oldname);
err:
	return NULL;
#endif /* posix_renameat_USE_posix_rename */

#ifdef posix_renameat_USE_STUB
	(void)olddfd;
	(void)oldname;
	(void)newdfd;
	(void)newname;
	(void)atflags;
#define NEED_posix_err_unsupported
	posix_err_unsupported("renameat");
	return NULL;
#endif /* posix_renameat_USE_STUB */
}


/*[[[deemon import("_dexutils").gw("renameat2",
	"olddfd:?X3?DFile?Dint?Dstring,oldname:?Dstring,"
	"newdfd:?X3?DFile?Dint?Dstring,newname:?Dstring,"
	"flags:u=0,atflags:u=0", libname: "posix");]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_renameat2_f_impl(DeeObject *olddfd, DeeObject *oldname, DeeObject *newdfd, DeeObject *newname, unsigned int flags, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_renameat2_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_RENAMEAT2_DEF { "renameat2", (DeeObject *)&posix_renameat2, MODSYM_FNORMAL, DOC("(olddfd:?X3?DFile?Dint?Dstring,oldname:?Dstring,newdfd:?X3?DFile?Dint?Dstring,newname:?Dstring,flags:?Dint=!0,atflags:?Dint=!0)") },
#define POSIX_RENAMEAT2_DEF_DOC(doc) { "renameat2", (DeeObject *)&posix_renameat2, MODSYM_FNORMAL, DOC("(olddfd:?X3?DFile?Dint?Dstring,oldname:?Dstring,newdfd:?X3?DFile?Dint?Dstring,newname:?Dstring,flags:?Dint=!0,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_renameat2, posix_renameat2_f);
#ifndef POSIX_KWDS_OLDDFD_OLDNAME_NEWDFD_NEWNAME_FLAGS_ATFLAGS_DEFINED
#define POSIX_KWDS_OLDDFD_OLDNAME_NEWDFD_NEWNAME_FLAGS_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_olddfd_oldname_newdfd_newname_flags_atflags, { K(olddfd), K(oldname), K(newdfd), K(newname), K(flags), K(atflags), KEND });
#endif /* !POSIX_KWDS_OLDDFD_OLDNAME_NEWDFD_NEWNAME_FLAGS_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_renameat2_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *olddfd;
	DeeObject *oldname;
	DeeObject *newdfd;
	DeeObject *newname;
	unsigned int flags = 0;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_olddfd_oldname_newdfd_newname_flags_atflags, "oooo|uu:renameat2", &olddfd, &oldname, &newdfd, &newname, &flags, &atflags))
		goto err;
	return posix_renameat2_f_impl(olddfd, oldname, newdfd, newname, flags, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_renameat2_f_impl(DeeObject *olddfd, DeeObject *oldname, DeeObject *newdfd, DeeObject *newname, unsigned int flags, unsigned int atflags)
/*[[[end]]]*/
{
#ifdef posix_renameat2_USE_nt_MoveFileEx
	DREF DeeObject *abs_oldname;
	DREF DeeObject *abs_newname;
	if (flags & RENAME_NOREPLACE) {
		int error;
		DWORD dwError;
		abs_oldname = posix_dfd_abspath(olddfd, oldname, atflags);
		if unlikely(!abs_oldname)
			goto err;
		abs_newname = posix_dfd_abspath(newdfd, newname, atflags);
		if unlikely(!abs_newname)
			goto err_abs_oldname;
again:
#define NEED_nt_MoveFileEx
		error = nt_MoveFileEx(abs_oldname, abs_newname, 0);
		if likely(error == 0) {
			Dee_Decref(abs_newname);
			Dee_Decref(abs_oldname);
			return_none;
		}
		if unlikely(error < 0)
			goto err_abs_oldname_abs_newname;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(dwError)) {
			if (DeeThread_CheckInterrupt())
				goto err_abs_oldname_abs_newname;
			goto again;
		}
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_CollectMemory(1))
				goto again;
			goto err_abs_oldname_abs_newname;
		}
#define NEED_err_nt_rename
		err_nt_rename(dwError, abs_oldname, abs_newname);
		goto err_abs_oldname_abs_newname;
	}
	if (flags != 0) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}
	return posix_renameat_f_impl(olddfd, oldname, newdfd, newname, atflags);
err_abs_oldname_abs_newname:
	Dee_Decref(abs_newname);
err_abs_oldname:
	Dee_Decref(abs_oldname);
err:
	return NULL;
#endif /* posix_renameat2_USE_nt_MoveFileEx */

#ifdef posix_renameat2_USE_renameat2
#if !defined(AT_RENAME_NOREPLACE) && !defined(AT_RENAME_EXCHANGE) && !defined(AT_RENAME_WHITEOUT)
	if (atflags == 0)
#endif /* !AT_RENAME_NOREPLACE && !AT_RENAME_EXCHANGE && !AT_RENAME_WHITEOUT */
	{
		int errno_value;
		int os_olddfd, os_newdfd;
		char *utf8_oldname, *utf8_newname;
		DREF DeeObject *abs_oldname;
		DREF DeeObject *abs_newname;
		os_olddfd = DeeUnixSystem_GetFD(olddfd);
		if unlikely(os_olddfd == -1)
			goto err;
		os_newdfd = DeeUnixSystem_GetFD(newdfd);
		if unlikely(os_newdfd == -1)
			goto err;
		utf8_oldname = DeeString_AsUtf8(oldname);
		if unlikely(!utf8_oldname)
			goto err;
		utf8_newname = DeeString_AsUtf8(newname);
		if unlikely(!utf8_newname)
			goto err;

		/* Name the call to `renameat2()' */
EINTR_ENOMEM_LABEL(again_renameat2)
#if defined(AT_RENAME_NOREPLACE) || defined(AT_RENAME_EXCHANGE) || defined(AT_RENAME_WHITEOUT)
		if (renameat2(os_olddfd, utf8_oldname, os_newdfd, utf8_newname, flags | atflags) == 0)
#else /* AT_RENAME_NOREPLACE || AT_RENAME_EXCHANGE || AT_RENAME_WHITEOUT */
		if (renameat2(os_olddfd, utf8_oldname, os_newdfd, utf8_newname, flags) == 0)
#endif /* !AT_RENAME_NOREPLACE && !AT_RENAME_EXCHANGE && !AT_RENAME_WHITEOUT */
		{
			return_none;
		}

		/* Directly handle rename errors */
		errno_value = DeeSystem_GetErrno();
		EINTR_ENOMEM_HANDLE(errno_value, again_renameat2, err);
		abs_oldname = posix_dfd_abspath(olddfd, oldname, atflags);
		if unlikely(!abs_oldname)
			goto err;
		abs_newname = posix_dfd_abspath(newdfd, newname, atflags);
		if unlikely(!abs_newname)
			goto err_abs_oldname;
		err_unix_rename(errno_value, abs_oldname, abs_newname);
		Dee_Decref(abs_newname);
		Dee_Decref(abs_oldname);
		goto err;
err_abs_oldname:
		Dee_Decref(abs_oldname);
		goto err;
	}
#if !defined(AT_RENAME_NOREPLACE) && !defined(AT_RENAME_EXCHANGE) && !defined(AT_RENAME_WHITEOUT)
#undef posix_renameat2_USE_posix_renameat
#define posix_renameat2_USE_posix_renameat
	/* Fallthru to `posix_renameat2_USE_posix_renameat' */
#endif /* !AT_RENAME_NOREPLACE && !AT_RENAME_EXCHANGE && !AT_RENAME_WHITEOUT */
#endif /* posix_renameat2_USE_renameat2 */

#ifdef posix_renameat2_USE_posix_renameat
	if (flags & RENAME_NOREPLACE) {
		/* Check if `newname' already exists (and if so: fail)
		 *
		 * Note that this isn't 100% safe since some other process
		 * may create the file after we checked for it, but this is
		 * the best we can do (since without `renameat2(2)', there
		 * literally isn't a system call that would make the check
		 * for us) */
		struct dee_stat st;
		int status;
		status = dee_stat_init(&st, newdfd, newname,
		                       atflags | AT_SYMLINK_NOFOLLOW | DEE_STAT_F_TRY);
		if unlikely(status < 0)
			goto err;
		if (status == 0) {
			/* File already exists */
			DREF DeeObject *abs_newname;
			abs_newname = posix_dfd_abspath(newdfd, newname, atflags);
			if unlikely(!abs_newname)
				goto err;
#define NEED_err_unix_path_exists
#ifdef EEXIST
			err_unix_path_exists(EEXIST, abs_newname);
#elif defined(EINVAL)
			err_unix_path_exists(EINVAL, abs_newname);
#else /* ... */
			err_unix_path_exists(1, abs_newname);
#endif /* !... */
			Dee_Decref(abs_newname);
			goto err;
		}
	}
	if (flags != 0) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}
	return posix_renameat_f_impl(olddfd, oldname, newdfd, newname, atflags);
err:
	return NULL;
#endif /* posix_renameat2_USE_posix_renameat */

#ifdef posix_renameat2_USE_STUB
	(void)olddfd;
	(void)oldname;
	(void)newdfd;
	(void)newname;
	(void)flags;
	(void)atflags;
#define NEED_posix_err_unsupported
	posix_err_unsupported("renameat2");
	return NULL;
#endif /* posix_renameat2_USE_STUB */
}



DECL_END

#endif /* !GUARD_DEX_POSIX_P_RENAME_C_INL */
