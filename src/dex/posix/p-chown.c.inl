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
#ifndef GUARD_DEX_POSIX_P_CHOWN_C_INL
#define GUARD_DEX_POSIX_P_CHOWN_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include "p-readlink.c.inl" /* For `posix_chown_USE_posix_readlink__AND__posix_lchown()' */
#include "p-path.c.inl"     /* For `posix_chown_USE_posix_readlink__AND__posix_lchown()' */

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


/* TODO: Come up with a way to implement uid/gid on windows.
 * Idea: Since we've got infinite-length integers, we could
 *       just encode whole SIDs as integers! */


/* Figure out how we want to implement `chown()' */
#undef posix_chown_USE_wchown
#undef posix_chown_USE_chown
#undef posix_chown_USE_wopen_AND_fchown
#undef posix_chown_USE_open_AND_fchown
#undef posix_chown_USE_STUB
#undef posix_chown_USE_posix_readlink__AND__posix_lchown
#if defined(CONFIG_HAVE_wchown) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_chown_USE_wchown
#elif defined(CONFIG_HAVE_chown)
#define posix_chown_USE_chown
#elif defined(CONFIG_HAVE_wchown)
#define posix_chown_USE_wchown
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_fchown) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_chown_USE_wopen_AND_fchown
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_fchown)
#define posix_chown_USE_open_AND_fchown
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_fchown)
#define posix_chown_USE_wopen_AND_fchown
#else /* ... */
#define posix_chown_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `lchown()' */
#undef posix_lchown_USE_nt_SetNamedSecurityInfo
#undef posix_lchown_USE_wlchown
#undef posix_lchown_USE_lchown
#undef posix_lchown_USE_wopen_AND_fchown
#undef posix_lchown_USE_open_AND_fchown
#undef posix_lchown_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_lchown_USE_nt_SetNamedSecurityInfo
#elif defined(CONFIG_HAVE_wlchown) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lchown_USE_wlchown
#elif defined(CONFIG_HAVE_lchown)
#define posix_lchown_USE_lchown
#elif defined(CONFIG_HAVE_wlchown)
#define posix_lchown_USE_wlchown
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_fchown) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_lchown_USE_wopen_AND_fchown
#elif defined(CONFIG_HAVE_open) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_fchown)
#define posix_lchown_USE_open_AND_fchown
#elif defined(CONFIG_HAVE_wopen) && defined(CONFIG_HAVE_O_RDWR) && defined(CONFIG_HAVE_O_NOFOLLOW) && defined(CONFIG_HAVE_fchown)
#define posix_lchown_USE_wopen_AND_fchown
#else /* ... */
#define posix_lchown_USE_STUB
#endif /* !... */

/* Check if we should emulate
 * >> chown(path, uid, gid);
 * As:
 * >> lchown(try joinpath(headof(path), readlink(path)) catch (NoSymlink) path, uid, gid); */
#if defined(posix_chown_USE_STUB) && !defined(posix_lchown_USE_STUB) && !defined(posix_readlink_USE_STUB)
#undef posix_chown_USE_STUB
#define posix_chown_USE_posix_readlink__AND__posix_lchown
#endif /* posix_chown_USE_STUB && !posix_lchown_USE_STUB && !posix_readlink_USE_STUB */



/* Figure out how we want to implement `fchown()' */
#undef posix_fchown_USE_nt_SetSecurityInfo
#undef posix_fchown_USE_fchown
#undef posix_fchown_USE_posix_lchown
#undef posix_fchown_USE_posix_chown
#undef posix_fchown_USE_STUB
#ifdef CONFIG_HOST_WINDOWS
#define posix_fchown_USE_nt_SetSecurityInfo
#elif defined(CONFIG_HAVE_fchown)
#define posix_fchown_USE_fchown
#elif !defined(posix_lchown_USE_STUB)
#define posix_fchown_USE_posix_lchown
#elif !defined(posix_chown_USE_STUB)
#define posix_fchown_USE_posix_chown
#else /* ... */
#define posix_fchown_USE_STUB
#endif /* !... */



/* Figure out how we want to implement `fchownat()' */
#undef posix_fchownat_USE_fchownat
#undef posix_fchownat_USE_posix_chown
#undef posix_fchownat_USE_posix_lchown
#undef posix_fchownat_USE_posix_fchown
#undef posix_fchownat_USE_STUB
#if !defined(posix_chown_USE_STUB) || !defined(posix_lchown_USE_STUB)
#ifdef CONFIG_HAVE_fchownat
#define posix_fchownat_USE_fchownat
#endif /* CONFIG_HAVE_fchownat */
#ifndef posix_chown_USE_STUB
#define posix_fchownat_USE_posix_chown
#endif /* !posix_chown_USE_STUB */
#ifndef posix_lchown_USE_STUB
#define posix_fchownat_USE_posix_lchown
#endif /* !posix_lchown_USE_STUB */
#ifndef posix_fchown_USE_STUB
#define posix_fchownat_USE_posix_fchown
#endif /* !posix_fchown_USE_STUB */
#else /* !posix_chown_USE_STUB */
#define posix_fchownat_USE_STUB
#endif /* posix_chown_USE_STUB */




/* Implement the open+fchown wrappers. */
#ifdef posix_chown_USE_wopen_AND_fchown
#undef fposix_chown_USE_wopen_AND_fchown
#define posix_chown_USE_wchown
#undef wchown
#define wchown dee_wchown
PRIVATE int DCALL dee_wchown(wchar_t const *filename, uid_t uid, gid_t gid) {
	int result;
	result = wopen(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = fchown(result, uid, gid);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_chown_USE_wopen_AND_fchown */

#ifdef posix_chown_USE_open_AND_fchown
#undef fposix_chown_USE_open_AND_fchown
#define posix_chown_USE_chown
#undef chown
#define chown dee_chown
PRIVATE int DCALL dee_chown(wchar_t const *filename, uid_t uid, gid_t gid) {
	int result;
	result = open(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = fchown(result, uid, gid);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_chown_USE_open_AND_fchown */

#ifdef posix_lchown_USE_wopen_AND_fchown
#undef fposix_lchown_USE_wopen_AND_fchown
#define posix_lchown_USE_wlchown
#undef wlchown
#define wlchown dee_wlchown
PRIVATE int DCALL dee_wlchown(wchar_t const *filename, uid_t uid, gid_t gid) {
	int result;
	result = wopen(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = fchown(result, uid, gid);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lchown_USE_wopen_AND_fchown */

#ifdef posix_lchown_USE_open_AND_fchown
#undef fposix_lchown_USE_open_AND_fchown
#define posix_lchown_USE_lchown
#undef lchown
#define lchown dee_lchown
PRIVATE int DCALL dee_lchown(wchar_t const *filename, uid_t uid, gid_t gid) {
	int result;
	result = open(filename, O_RDWR);
	if (result != -1) {
		int error;
		error = fchown(result, uid, gid);
		OPT_close(result);
		result = error;
	}
	return result;
}
#endif /* posix_lchown_USE_open_AND_fchown */

#ifdef posix_chown_USE_posix_readlink__AND__posix_lchown
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lchown_f_impl(DeeObject *path, DeeObject *uid, DeeObject *gid);
#endif /* posix_chown_USE_posix_readlink__AND__posix_lchown */



/*[[[deemon import("rt.gen.dexutils").gw("chown", "path:?Dstring,uid:?X3?Dstring?Dint?N=Dee_None,gid:?X3?Dstring?Dint?N=Dee_None", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_chown_f_impl(DeeObject *path, DeeObject *uid, DeeObject *gid);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_chown_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_CHOWN_DEF { "chown", (DeeObject *)&posix_chown, MODSYM_FNORMAL, DOC("(path:?Dstring,uid:?X3?Dstring?Dint?N=!N,gid:?X3?Dstring?Dint?N=!N)") },
#define POSIX_CHOWN_DEF_DOC(doc) { "chown", (DeeObject *)&posix_chown, MODSYM_FNORMAL, DOC("(path:?Dstring,uid:?X3?Dstring?Dint?N=!N,gid:?X3?Dstring?Dint?N=!N)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_chown, &posix_chown_f);
#ifndef POSIX_KWDS_PATH_UID_GID_DEFINED
#define POSIX_KWDS_PATH_UID_GID_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path_uid_gid, { K(path), K(uid), K(gid), KEND });
#endif /* !POSIX_KWDS_PATH_UID_GID_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_chown_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	DeeObject *uid = Dee_None;
	DeeObject *gid = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path_uid_gid, "o|oo:chown", &path, &uid, &gid))
		goto err;
	return posix_chown_f_impl(path, uid, gid);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_chown_f_impl(DeeObject *path, DeeObject *uid, DeeObject *gid)
/*[[[end]]]*/
{
#ifdef posix_chown_USE_posix_readlink__AND__posix_lchown
	DREF DeeObject *link_text;
	/* Try to readlink() the given `path' to see if it's a symbolic link. */
	link_text = posix_readlink_f_impl(path);
	if (link_text) {
		DREF DeeObject *parts[2], *full_path, *result;
		parts[0] = posix_path_headof_f(path);
		if unlikely(!parts[0])
			goto err_link_text;
		parts[1] = link_text;
		full_path = posix_path_joinpath_f(2, parts);
		Dee_Decref(parts[1]);
		Dee_Decref(parts[0]);
		if unlikely(!full_path)
			goto err;
		result = posix_lchown_f_impl(full_path, uid, gid);
		Dee_Decref(full_path);
		return result;
	}
	if (!DeeError_Catch(&DeeError_NoSymlink))
		goto err;

	/* Not a symbolic link -> lchown() will work to do what we want! */
	return posix_lchown_f_impl(path, uid, gid);
err_link_text:
	Dee_Decref(link_text);
err:
	return NULL;
#elif !defined(posix_chown_USE_STUB)
	int error;
	uid_t used_uid;
	gid_t used_gid;
#ifdef posix_chown_USE_chown
	char const *utf8_path;
#endif /* posix_chown_USE_chown */
#ifdef posix_chown_USE_wchown
	dwchar_t const *wide_path;
#endif /* posix_chown_USE_wchown */

	/* Decode arguments. */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_chown_USE_chown
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_chown_USE_chown */
#ifdef posix_chown_USE_wchown
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_chown_USE_wchown */

	/* Load the chown-uid/gid argument. */
	if unlikely(posix_chown_unix_parseuid(uid, &used_uid))
		goto err;
#define NEED_posix_chown_unix_parseuid
	if unlikely(posix_chown_unix_parsegid(gid, &used_gid))
		goto err;
#define NEED_posix_chown_unix_parsegid

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_chown_USE_wchown
	error = wchown(wide_path, used_uid, used_gid);
#endif /* posix_chown_USE_wchown */
#ifdef posix_chown_USE_chown
	error = chown(utf8_path, used_uid, used_gid);
#endif /* posix_chown_USE_chown */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_chown(error, path, used_uid, used_gid);
#define NEED_err_unix_chown
		goto err;
	}
	return_none;
err:
	return NULL;
#else /* !posix_chown_USE_STUB */
#define NEED_posix_err_unsupported
	(void)path;
	(void)uid;
	(void)gid;
	posix_err_unsupported("chown");
	return NULL;
#endif /* posix_chown_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("lchown", "path:?Dstring,uid:?X3?Dstring?Dint?N=Dee_None,gid:?X3?Dstring?Dint?N=Dee_None", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lchown_f_impl(DeeObject *path, DeeObject *uid, DeeObject *gid);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lchown_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_LCHOWN_DEF { "lchown", (DeeObject *)&posix_lchown, MODSYM_FNORMAL, DOC("(path:?Dstring,uid:?X3?Dstring?Dint?N=!N,gid:?X3?Dstring?Dint?N=!N)") },
#define POSIX_LCHOWN_DEF_DOC(doc) { "lchown", (DeeObject *)&posix_lchown, MODSYM_FNORMAL, DOC("(path:?Dstring,uid:?X3?Dstring?Dint?N=!N,gid:?X3?Dstring?Dint?N=!N)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lchown, &posix_lchown_f);
#ifndef POSIX_KWDS_PATH_UID_GID_DEFINED
#define POSIX_KWDS_PATH_UID_GID_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_path_uid_gid, { K(path), K(uid), K(gid), KEND });
#endif /* !POSIX_KWDS_PATH_UID_GID_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lchown_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *path;
	DeeObject *uid = Dee_None;
	DeeObject *gid = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_path_uid_gid, "o|oo:lchown", &path, &uid, &gid))
		goto err;
	return posix_lchown_f_impl(path, uid, gid);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lchown_f_impl(DeeObject *path, DeeObject *uid, DeeObject *gid)
/*[[[end]]]*/
{
#ifdef posix_lchown_USE_nt_SetNamedSecurityInfo
	NT_SID *nt_uid, *nt_gid;
	int status;
	SECURITY_INFORMATION SecurityInfo;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;

	/* Query uid/gid values */
	nt_uid = nt_QuerySid(uid, false);
#define NEED_nt_QuerySid
	if unlikely(nt_uid == (NT_SID *)ITER_DONE)
		goto err;
	nt_gid = nt_QuerySid(gid, true);
#define NEED_nt_QuerySid
	if unlikely(nt_gid == (NT_SID *)ITER_DONE)
		goto err_nt_uid;

	/* Do the system call. */
	SecurityInfo = 0;
	if (nt_uid != NULL)
		SecurityInfo |= OWNER_SECURITY_INFORMATION;
	if (nt_gid != NULL)
		SecurityInfo |= GROUP_SECURITY_INFORMATION;
	if (SecurityInfo == 0) {
		/* Nothing to do here -- neither uid, nor gid changed */
	} else {
		DWORD dwError;
		status = nt_SetNamedSecurityInfo(path, SE_FILE_OBJECT, SecurityInfo,
		                                 nt_uid, nt_gid, NULL, NULL, &dwError);
#define NEED_nt_SetNamedSecurityInfo
		if (status != 0) {
			if (status < 0)
				goto err_nt_uid_nt_gid;
			err_nt_lchown(dwError, path, uid, gid);
#define NEED_err_nt_lchown
			goto err_nt_uid_nt_gid;
		}
	}
	Dee_Free(nt_gid);
	Dee_Free(nt_uid);
	return_none;
err_nt_uid_nt_gid:
	Dee_Free(nt_gid);
err_nt_uid:
	Dee_Free(nt_uid);
err:
	return NULL;
#elif !defined(posix_lchown_USE_STUB)
	int error;
	uid_t used_uid;
	gid_t used_gid;
#ifdef posix_lchown_USE_lchown
	char const *utf8_path;
#endif /* posix_lchown_USE_lchown */
#ifdef posix_lchown_USE_wlchown
	dwchar_t const *wide_path;
#endif /* posix_lchown_USE_wlchown */

	/* Decode arguments. */
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef posix_lchown_USE_lchown
	utf8_path = DeeString_AsUtf8(path);
	if unlikely(!utf8_path)
		goto err;
#endif /* posix_lchown_USE_lchown */
#ifdef posix_lchown_USE_wlchown
	wide_path = DeeString_AsWide(path);
	if unlikely(!wide_path)
		goto err;
#endif /* posix_lchown_USE_wlchown */

	/* Load the chown-uid/gid argument. */
	if unlikely(posix_chown_unix_parseuid(uid, &used_uid))
		goto err;
#define NEED_posix_chown_unix_parseuid
	if unlikely(posix_chown_unix_parsegid(gid, &used_gid))
		goto err;
#define NEED_posix_chown_unix_parsegid

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_lchown_USE_wlchown
	error = wlchown(wide_path, used_uid, used_gid);
#endif /* posix_lchown_USE_wlchown */
#ifdef posix_lchown_USE_lchown
	error = lchown(utf8_path, used_uid, used_gid);
#endif /* posix_lchown_USE_lchown */
	DBG_ALIGNMENT_ENABLE();

	if unlikely(error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_lchown(error, path, used_uid, used_gid);
#define NEED_err_unix_lchown
		goto err;
	}
	return_none;
err:
	return NULL;
#else /* !posix_lchown_USE_STUB */
#define NEED_posix_err_unsupported
	(void)path;
	(void)uid;
	(void)gid;
	posix_err_unsupported("lchown");
	return NULL;
#endif /* posix_lchown_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("fchown", "fd:?X2?Dint?DFile,uid:?X3?Dstring?Dint?N=Dee_None,gid:?X3?Dstring?Dint?N=Dee_None", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchown_f_impl(DeeObject *fd, DeeObject *uid, DeeObject *gid);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchown_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHOWN_DEF { "fchown", (DeeObject *)&posix_fchown, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,uid:?X3?Dstring?Dint?N=!N,gid:?X3?Dstring?Dint?N=!N)") },
#define POSIX_FCHOWN_DEF_DOC(doc) { "fchown", (DeeObject *)&posix_fchown, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,uid:?X3?Dstring?Dint?N=!N,gid:?X3?Dstring?Dint?N=!N)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchown, &posix_fchown_f);
#ifndef POSIX_KWDS_FD_UID_GID_DEFINED
#define POSIX_KWDS_FD_UID_GID_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd_uid_gid, { K(fd), K(uid), K(gid), KEND });
#endif /* !POSIX_KWDS_FD_UID_GID_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchown_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *fd;
	DeeObject *uid = Dee_None;
	DeeObject *gid = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_uid_gid, "o|oo:fchown", &fd, &uid, &gid))
		goto err;
	return posix_fchown_f_impl(fd, uid, gid);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchown_f_impl(DeeObject *fd, DeeObject *uid, DeeObject *gid)
/*[[[end]]]*/
{
#ifdef posix_fchown_USE_nt_SetSecurityInfo
	NT_SID *nt_uid, *nt_gid;
	int status;
	SECURITY_INFORMATION SecurityInfo;
	HANDLE hFd = DeeNTSystem_GetHandle(fd);
	if unlikely(hFd == INVALID_HANDLE_VALUE)
		goto err;

	/* Query uid/gid values */
	nt_uid = nt_QuerySid(uid, false);
#define NEED_nt_QuerySid
	if unlikely(nt_uid == (NT_SID *)ITER_DONE)
		goto err;
	nt_gid = nt_QuerySid(gid, true);
#define NEED_nt_QuerySid
	if unlikely(nt_gid == (NT_SID *)ITER_DONE)
		goto err_nt_uid;

	/* Do the system call. */
	SecurityInfo = 0;
	if (nt_uid != NULL)
		SecurityInfo |= OWNER_SECURITY_INFORMATION;
	if (nt_gid != NULL)
		SecurityInfo |= GROUP_SECURITY_INFORMATION;
	if (SecurityInfo == 0) {
		/* Nothing to do here -- neither uid, nor gid changed */
	} else {
		DWORD dwError;
		status = nt_SetSecurityInfo(hFd, SE_FILE_OBJECT, SecurityInfo,
		                            nt_uid, nt_gid, NULL, NULL, &dwError);
#define NEED_nt_SetSecurityInfo
		if (status != 0) {
			if (status < 0)
				goto err_nt_uid_nt_gid;
			err_nt_fchown(dwError, fd, uid, gid);
#define NEED_err_nt_fchown
			goto err_nt_uid_nt_gid;
		}
	}
	Dee_Free(nt_gid);
	Dee_Free(nt_uid);
	return_none;
err_nt_uid_nt_gid:
	Dee_Free(nt_gid);
err_nt_uid:
	Dee_Free(nt_uid);
err:
	return NULL;
#elif defined(posix_fchown_USE_fchown)
	int error;
	uid_t used_uid;
	gid_t used_gid;
	int os_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(os_fd == -1)
		goto err;

	/* Load the chown-uid/gid argument. */
	if unlikely(posix_chown_unix_parseuid(uid, &used_uid))
		goto err;
#define NEED_posix_chown_unix_parseuid
	if unlikely(posix_chown_unix_parsegid(gid, &used_gid))
		goto err;
#define NEED_posix_chown_unix_parsegid

EINTR_ENOMEM_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	error = fchown(os_fd, used_uid, used_gid);
	DBG_ALIGNMENT_ENABLE();
	
	if unlikely(error < 0) {
		error = DeeSystem_GetErrno();
		EINTR_HANDLE(error, again, err);
		ENOMEM_HANDLE(error, again, err);
		err_unix_fchown(error, fd, used_uid, used_gid);
#define NEED_err_unix_fchown
		goto err;
	}
	return_none;
err:
	return NULL;
#endif /* posix_fchown_USE_fchown */

#if defined(posix_fchown_USE_posix_lchown) || defined(posix_fchown_USE_posix_chown)
	DREF DeeObject *result;
	DREF DeeObject *abspath;
	abspath = posix_fd_makepath(fd);
#define NEED_posix_fd_makepath
	if unlikely(!abspath)
		goto err;
#ifdef posix_fchown_USE_posix_lchown
	result = posix_lchown_f_impl(abspath, uid, gid);
#else /* posix_fchown_USE_posix_lchown */
	result = posix_chown_f_impl(abspath, uid, gid);
#endif /* !posix_fchown_USE_posix_lchown */
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fchown_USE_posix_lchown || posix_fchown_USE_posix_chown */

#ifdef posix_fchown_USE_STUB
	(void)fd;
	(void)uid;
	(void)gid;
	posix_err_unsupported("fchown");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_fchown_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("fchownat", "dfd:?X3?DFile?Dint?Dstring,path:?Dstring,uid:?X3?Dstring?Dint?N=Dee_None,gid:?X3?Dstring?Dint?N=Dee_None,atflags:u=0", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchownat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *uid, DeeObject *gid, unsigned int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchownat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHOWNAT_DEF { "fchownat", (DeeObject *)&posix_fchownat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,uid:?X3?Dstring?Dint?N=!N,gid:?X3?Dstring?Dint?N=!N,atflags:?Dint=!0)") },
#define POSIX_FCHOWNAT_DEF_DOC(doc) { "fchownat", (DeeObject *)&posix_fchownat, MODSYM_FNORMAL, DOC("(dfd:?X3?DFile?Dint?Dstring,path:?Dstring,uid:?X3?Dstring?Dint?N=!N,gid:?X3?Dstring?Dint?N=!N,atflags:?Dint=!0)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchownat, &posix_fchownat_f);
#ifndef POSIX_KWDS_DFD_PATH_UID_GID_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_PATH_UID_GID_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_path_uid_gid_atflags, { K(dfd), K(path), K(uid), K(gid), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_PATH_UID_GID_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchownat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *dfd;
	DeeObject *path;
	DeeObject *uid = Dee_None;
	DeeObject *gid = Dee_None;
	unsigned int atflags = 0;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_path_uid_gid_atflags, "oo|oou:fchownat", &dfd, &path, &uid, &gid, &atflags))
		goto err;
	return posix_fchownat_f_impl(dfd, path, uid, gid, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchownat_f_impl(DeeObject *dfd, DeeObject *path, DeeObject *uid, DeeObject *gid, unsigned int atflags)
/*[[[end]]]*/
{
#if defined(posix_fchownat_USE_posix_chown) || defined(posix_fchownat_USE_posix_lchown)
	DREF DeeObject *result, *abspath;
#ifdef posix_fchownat_USE_posix_fchown
	if (atflags & AT_EMPTY_PATH) {
		if (!DeeString_Check(dfd) &&
		    (DeeNone_Check(path) || (DeeString_Check(path) &&
		                             DeeString_IsEmpty(path))))
			return posix_fchown_f_impl(dfd, uid, gid);
		atflags &= ~AT_EMPTY_PATH;
	}
#endif /* posix_fchownat_USE_posix_fchown */

#ifdef posix_fchownat_USE_fchownat
	if (!DeeString_Check(dfd)) {
		int os_dfd, error;
		uid_t used_uid;
		gid_t used_gid;
		char const *utf8_path;
		os_dfd = DeeUnixSystem_GetFD(dfd);
		if unlikely(os_dfd == -1)
			goto err;
		if (DeeObject_AssertTypeExact(path, &DeeString_Type))
			goto err;
		utf8_path = DeeString_AsUtf8(path);
		if unlikely(!utf8_path)
			goto err;
		/* Load the chown-uid/gid argument. */
		if unlikely(posix_chown_unix_parseuid(uid, &used_uid))
			goto err;
#define NEED_posix_chown_unix_parseuid
		if unlikely(posix_chown_unix_parsegid(gid, &used_gid))
			goto err;
#define NEED_posix_chown_unix_parsegid
EINTR_ENOMEM_LABEL(again)
		DBG_ALIGNMENT_DISABLE();
		error = fchownat(os_dfd, utf8_path, used_uid, used_gid, atflags);
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
#endif /* posix_fchownat_USE_fchownat */

#ifdef posix_fchownat_USE_posix_lchown
	abspath = posix_dfd_makepath(dfd, path, atflags & ~AT_SYMLINK_NOFOLLOW);
#else /* posix_fchownat_USE_posix_lchown */
	abspath = posix_dfd_makepath(dfd, path, atflags);
#endif /* !posix_fchownat_USE_posix_lchown */
	if unlikely(!abspath)
		goto err;
#ifdef posix_fchownat_USE_posix_lchown
	if (atflags & AT_SYMLINK_NOFOLLOW) {
		result = posix_lchown_f_impl(abspath, uid, gid);
	} else
#endif /* posix_fchownat_USE_posix_lchown */
	{
#ifdef posix_fchownat_USE_posix_chown
		result = posix_chown_f_impl(abspath, uid, gid);
#else /* posix_fchownat_USE_posix_chown */
		Dee_Decref(abspath);
		err_bad_atflags(atflags);
#define NEED_err_bad_atflags
		goto err;
#endif /* !posix_fchownat_USE_posix_chown */
	}
	Dee_Decref(abspath);
	return result;
err:
	return NULL;
#endif /* posix_fchownat_USE_posix_chown || posix_fchownat_USE_posix_lchown */

#ifdef posix_fchownat_USE_STUB
	(void)dfd;
	(void)path;
	(void)uid;
	(void)gid;
	(void)atflags;
	posix_err_unsupported("fchownat");
#define NEED_posix_err_unsupported
	return NULL;
#endif /* posix_fchownat_USE_STUB */
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_CHOWN_C_INL */
