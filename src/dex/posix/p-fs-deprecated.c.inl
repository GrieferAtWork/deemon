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
#ifndef GUARD_DEX_POSIX_P_FS_DEPRECATED_C_INL
#define GUARD_DEX_POSIX_P_FS_DEPRECATED_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"

DECL_BEGIN

#undef chmod
#undef lchmod
#undef chown
#undef lchown
#undef mkdir
#undef rename
#undef link
#undef symlink

#define DEFINE_LIBFS_FORWARD_WRAPPER(name, symbol_name)                                \
	PRIVATE DEFINE_STRING(libposix_libfs_name_##name, symbol_name);                    \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                              \
	libposix_getfs_##name##_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {   \
		return DeeObject_GetAttr(FS_MODULE, (DeeObject *)&libposix_libfs_name_##name); \
	}                                                                                  \
	PRIVATE DEFINE_CMETHOD(libposix_getfs_##name, &libposix_getfs_##name##_f);
#define DEFINE_LIBFS_FORWARD_WRAPPER_S(name) \
	DEFINE_LIBFS_FORWARD_WRAPPER(name, #name)
DEFINE_LIBFS_FORWARD_WRAPPER_S(chmod)
DEFINE_LIBFS_FORWARD_WRAPPER_S(lchmod)
DEFINE_LIBFS_FORWARD_WRAPPER_S(chown)
DEFINE_LIBFS_FORWARD_WRAPPER_S(lchown)
DEFINE_LIBFS_FORWARD_WRAPPER_S(mkdir)
DEFINE_LIBFS_FORWARD_WRAPPER_S(rename)
DEFINE_LIBFS_FORWARD_WRAPPER_S(link)
DEFINE_LIBFS_FORWARD_WRAPPER_S(symlink)
#undef DEFINE_LIBFS_FORWARD_WRAPPER_S
#undef DEFINE_LIBFS_FORWARD_WRAPPER





/************************************************************************/
/* fchownat()                                                           */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("fchownat", "dfd:d,filename:c:char[],owner:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint,atflags:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchownat_f_impl(int dfd, /*utf-8*/ char const *filename, DeeObject *owner, DeeObject *group, int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchownat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHOWNAT_DEF { "fchownat", (DeeObject *)&posix_fchownat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,owner:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint,atflags:?Dint)->?Dint") },
#define POSIX_FCHOWNAT_DEF_DOC(doc) { "fchownat", (DeeObject *)&posix_fchownat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,owner:?X3?Efs:User?Dstring?Dint,group:?X3?Efs:Group?Dstring?Dint,atflags:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchownat, posix_fchownat_f);
#ifndef POSIX_KWDS_DFD_FILENAME_OWNER_GROUP_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_FILENAME_OWNER_GROUP_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_filename_owner_group_atflags, { K(dfd), K(filename), K(owner), K(group), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_FILENAME_OWNER_GROUP_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchownat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int dfd;
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	DeeObject *owner;
	DeeObject *group;
	int atflags;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_filename_owner_group_atflags, "doood:fchownat", &dfd, &filename, &owner, &group, &atflags))
		goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
		goto err;
	return posix_fchownat_f_impl(dfd, filename_str, owner, group, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchownat_f_impl(int dfd, /*utf-8*/ char const *filename, DeeObject *owner, DeeObject *group, int atflags)
//[[[end]]]
{
#ifdef CONFIG_HAVE_fchownat
	uid_t owner_uid;
	gid_t group_gid;
	int result;
	if (DeeInt_Check(owner)) {
		if (DeeObject_AsUINT(owner, &owner_uid))
			goto err;
	} else {
		owner = DeeObject_CallAttrString(FS_MODULE, "User", 1, (DeeObject **)&owner);
		if unlikely(!owner)
			goto err;
		result = DeeObject_AsUINT(owner, &owner_uid);
		Dee_Decref(owner);
		if unlikely(result)
			goto err;
	}
	if (DeeInt_Check(group)) {
		if (DeeObject_AsUINT(group, &group_gid))
			goto err;
	} else {
		group = DeeObject_CallAttrString(FS_MODULE, "Group", 1, (DeeObject **)&group);
		if unlikely(!group)
			goto err;
		result = DeeObject_AsUINT(group, &group_gid);
		Dee_Decref(group);
		if unlikely(result)
			goto err;
	}
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = fchownat(dfd, filename, owner_uid, group_gid, atflags);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		HANDLE_EINTR(result, again, err)
		HANDLE_ENOSYS(result, err, "fchownat")
		HANDLE_EINVAL(result, err, "Invalid at-flags")
		HANDLE_ENOMEM(result, err, "Insufficient kernel memory to change ownership of %d:%q", dfd, filename)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory %d:%q could not be found", dfd, filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file %d:%q", dfd, filename)
		HANDLE_EBADF(result, err, "Invalid handle %d", dfd)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, result,
		                          "Failed to change ownership of %d:%q",
		                          dfd, filename);
		goto err;
	}
	return DeeInt_NewInt(result);
#else /* CONFIG_HAVE_fchownat */
#define NEED_libposix_get_dfd_filename 1
	DREF DeeObject *func;
	DREF DeeObject *result;
	DREF DeeObject *args[3];
	args[0] = libposix_get_dfd_filename(dfd, filename, atflags);
	if unlikely(!args[0])
		goto err;
	func = libposix_getfs_chown_f(0, NULL);
	if unlikely(!func)
		result = NULL;
	else {
		args[1] = owner;
		args[2] = group;
		result = DeeObject_Call(func, 3, args);
		Dee_Decref(func);
	}
	Dee_Decref(args[0]);
	return result;
#endif /* !CONFIG_HAVE_fchownat */
err:
	return NULL;
}





/************************************************************************/
/* fchmodat()                                                           */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("fchmodat", "dfd:d,filename:c:char[],mode:u,atflags:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchmodat_f_impl(int dfd, /*utf-8*/ char const *filename, unsigned int mode, int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchmodat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHMODAT_DEF { "fchmodat", (DeeObject *)&posix_fchmodat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,mode:?Dint,atflags:?Dint)->?Dint") },
#define POSIX_FCHMODAT_DEF_DOC(doc) { "fchmodat", (DeeObject *)&posix_fchmodat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,mode:?Dint,atflags:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchmodat, posix_fchmodat_f);
#ifndef POSIX_KWDS_DFD_FILENAME_MODE_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_FILENAME_MODE_ATFLAGS_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_dfd_filename_mode_atflags, { K(dfd), K(filename), K(mode), K(atflags), KEND });
#endif /* !POSIX_KWDS_DFD_FILENAME_MODE_ATFLAGS_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchmodat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int dfd;
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	unsigned int mode;
	int atflags;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_dfd_filename_mode_atflags, "doud:fchmodat", &dfd, &filename, &mode, &atflags))
		goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
		goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
		goto err;
	return posix_fchmodat_f_impl(dfd, filename_str, mode, atflags);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchmodat_f_impl(int dfd, /*utf-8*/ char const *filename, unsigned int mode, int atflags)
//[[[end]]]
{
#ifdef CONFIG_HAVE_fchmodat
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result = fchmodat(dfd, filename, mode, atflags);
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		HANDLE_EINTR(result, again, err)
		HANDLE_ENOSYS(result, err, "fchmodat")
		HANDLE_EINVAL(result, err, "Invalid at-flags")
		HANDLE_ENOMEM(result, err, "Insufficient kernel memory to change access mode of %d:%q", dfd, filename)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory %d:%q could not be found", dfd, filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file %d:%q", dfd, filename)
		HANDLE_EBADF(result, err, "Invalid handle %d", dfd)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, result,
		                          "Failed to change access mode of %d:%q",
		                          dfd, filename);
		goto err;
	}
	return DeeInt_NewInt(result);
#else /* CONFIG_HAVE_fchmodat */
#define NEED_libposix_get_dfd_filename 1
	DREF DeeObject *func;
	DREF DeeObject *result;
	DREF DeeObject *args[2];
	args[0] = libposix_get_dfd_filename(dfd, filename, atflags);
	if unlikely(!args[0])
		goto err;
	func = libposix_getfs_chmod_f(0, NULL);
	if unlikely(!func)
		result = NULL;
	else {
		args[1] = DeeInt_NewUInt(mode);
		if unlikely(!args[1])
			result = NULL;
		else {
			result = DeeObject_Call(func, 2, args);
			Dee_Decref(args[1]);
		}
		Dee_Decref(func);
	}
	Dee_Decref(args[0]);
	return result;
#endif /* !CONFIG_HAVE_fchmodat */
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_POSIX_P_FS_DEPRECATED_C_INL */
