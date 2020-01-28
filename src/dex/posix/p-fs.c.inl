/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_FS_C_INL
#define GUARD_DEX_POSIX_P_FS_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN

#undef environ

#undef stat
#undef lstat
#undef getcwd
#undef gethostname
#undef chdir
#undef chmod
#undef lchmod
#undef chown
#undef lchown
#undef mkdir
#undef rmdir
#undef unlink
#undef remove
#undef rename
#undef link
#undef symlink
#undef readlink

#undef S_IFMT
#undef S_IFDIR
#undef S_IFCHR
#undef S_IFBLK
#undef S_IFREG
#undef S_IFIFO
#undef S_IFLNK
#undef S_IFSOCK
#undef S_ISUID
#undef S_ISGID
#undef S_ISVTX
#undef S_IRUSR
#undef S_IWUSR
#undef S_IXUSR
#undef S_IRGRP
#undef S_IWGRP
#undef S_IXGRP
#undef S_IROTH
#undef S_IWOTH
#undef S_IXOTH

#undef S_ISDIR
#undef S_ISCHR
#undef S_ISBLK
#undef S_ISREG
#undef S_ISFIFO
#undef S_ISLNK
#undef S_ISSOCK

#define DEFINE_LIBFS_FORWARD_WRAPPER(name, symbol_name)                                \
	PRIVATE DEFINE_STRING(libposix_libfs_name_##name, symbol_name);                    \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                              \
	libposix_getfs_##name##_f(size_t UNUSED(argc), DeeObject *const *UNUSED(argv)) {   \
		return DeeObject_GetAttr(FS_MODULE, (DeeObject *)&libposix_libfs_name_##name); \
	}                                                                                  \
	PRIVATE DEFINE_CMETHOD(libposix_getfs_##name, &libposix_getfs_##name##_f);
#define DEFINE_LIBFS_FORWARD_WRAPPER_S(name) \
	DEFINE_LIBFS_FORWARD_WRAPPER(name, #name)
DEFINE_LIBFS_FORWARD_WRAPPER(opendir, "dir")
DEFINE_LIBFS_FORWARD_WRAPPER_S(environ)
DEFINE_LIBFS_FORWARD_WRAPPER_S(stat)
DEFINE_LIBFS_FORWARD_WRAPPER_S(lstat)
DEFINE_LIBFS_FORWARD_WRAPPER_S(getcwd)
DEFINE_LIBFS_FORWARD_WRAPPER_S(gethostname)
DEFINE_LIBFS_FORWARD_WRAPPER_S(chdir)
DEFINE_LIBFS_FORWARD_WRAPPER_S(chmod)
DEFINE_LIBFS_FORWARD_WRAPPER_S(lchmod)
DEFINE_LIBFS_FORWARD_WRAPPER_S(chown)
DEFINE_LIBFS_FORWARD_WRAPPER_S(lchown)
DEFINE_LIBFS_FORWARD_WRAPPER_S(mkdir)
DEFINE_LIBFS_FORWARD_WRAPPER_S(rmdir)
DEFINE_LIBFS_FORWARD_WRAPPER_S(unlink)
DEFINE_LIBFS_FORWARD_WRAPPER_S(remove)
DEFINE_LIBFS_FORWARD_WRAPPER_S(rename)
DEFINE_LIBFS_FORWARD_WRAPPER_S(link)
DEFINE_LIBFS_FORWARD_WRAPPER_S(symlink)
DEFINE_LIBFS_FORWARD_WRAPPER_S(readlink)

DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFMT)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFDIR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFCHR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFBLK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFREG)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFIFO)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFLNK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IFSOCK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISUID)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISGID)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISVTX)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IRUSR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IWUSR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IXUSR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IRGRP)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IWGRP)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IXGRP)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IROTH)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IWOTH)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_IXOTH)

DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISDIR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISCHR)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISBLK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISREG)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISFIFO)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISLNK)
DEFINE_LIBFS_FORWARD_WRAPPER_S(S_ISSOCK)

#undef DEFINE_LIBFS_FORWARD_WRAPPER_S
#undef DEFINE_LIBFS_FORWARD_WRAPPER





/************************************************************************/
/* fchownat()                                                           */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("fchownat", "dfd:d,filename:c:char[],owner:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint,atflags:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fchownat_f_impl(int dfd, /*utf-8*/ char const *filename, DeeObject *owner, DeeObject *group, int atflags);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fchownat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCHOWNAT_DEF { "fchownat", (DeeObject *)&posix_fchownat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,owner:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint,atflags:?Dint)->?Dint") },
#define POSIX_FCHOWNAT_DEF_DOC(doc) { "fchownat", (DeeObject *)&posix_fchownat, MODSYM_FNORMAL, DOC("(dfd:?Dint,filename:?Dstring,owner:?X3?Efs:user?Dstring?Dint,group:?X3?Efs:group?Dstring?Dint,atflags:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fchownat, posix_fchownat_f);
#ifndef POSIX_KWDS_DFD_FILENAME_OWNER_GROUP_ATFLAGS_DEFINED
#define POSIX_KWDS_DFD_FILENAME_OWNER_GROUP_ATFLAGS_DEFINED 1
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
		DeeError_SysThrowf(&DeeError_SystemError, result, "Failed to change ownership of %d:%q", dfd, filename);
		goto err;
	}
	return DeeInt_NewInt(result);
#else /* CONFIG_HAVE_fchownat */
#define NEED_GET_DFD_FILENAME 1
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
#define POSIX_KWDS_DFD_FILENAME_MODE_ATFLAGS_DEFINED 1
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
		DeeError_SysThrowf(&DeeError_SystemError, result, "Failed to change access mode of %d:%q", dfd, filename);
		goto err;
	}
	return DeeInt_NewInt(result);
#else /* CONFIG_HAVE_fchmodat */
#define NEED_GET_DFD_FILENAME 1
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





/************************************************************************/
/* getenv()                                                             */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("getenv", "varname:?Dstring->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getenv_f_impl(DeeObject *varname);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_GETENV_DEF { "getenv", (DeeObject *)&posix_getenv, MODSYM_FNORMAL, DOC("(varname:?Dstring)->?Dstring") },
#define POSIX_GETENV_DEF_DOC(doc) { "getenv", (DeeObject *)&posix_getenv, MODSYM_FNORMAL, DOC("(varname:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_getenv, posix_getenv_f);
#ifndef POSIX_KWDS_VARNAME_DEFINED
#define POSIX_KWDS_VARNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_varname, { K(varname), KEND });
#endif /* !POSIX_KWDS_VARNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_getenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *varname;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_varname, "o:getenv", &varname))
	    goto err;
	return posix_getenv_f_impl(varname);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_getenv_f_impl(DeeObject *varname)
//[[[end]]]
{
	DREF DeeObject *env, *result;
	env = DeeObject_GetAttr(FS_MODULE, (DeeObject *)&libposix_libfs_name_environ);
	if unlikely(!env)
		goto err;
	result = DeeObject_GetItem(env, varname);
	Dee_Decref(env);
	return result;
err:
	return NULL;
}





/************************************************************************/
/* setenv()                                                             */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("setenv", "varname:?Dstring,value:?Dstring,override:c:bool=true->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_setenv_f_impl(DeeObject *varname, DeeObject *value, bool override);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_setenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_SETENV_DEF { "setenv", (DeeObject *)&posix_setenv, MODSYM_FNORMAL, DOC("(varname:?Dstring,value:?Dstring,override:?Dbool=!t)->?Dstring") },
#define POSIX_SETENV_DEF_DOC(doc) { "setenv", (DeeObject *)&posix_setenv, MODSYM_FNORMAL, DOC("(varname:?Dstring,value:?Dstring,override:?Dbool=!t)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_setenv, posix_setenv_f);
#ifndef POSIX_KWDS_VARNAME_VALUE_OVERRIDE_DEFINED
#define POSIX_KWDS_VARNAME_VALUE_OVERRIDE_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_varname_value_override, { K(varname), K(value), K(override), KEND });
#endif /* !POSIX_KWDS_VARNAME_VALUE_OVERRIDE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_setenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *varname;
	DeeObject *value;
	bool override = true;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_varname_value_override, "oo|b:setenv", &varname, &value, &override))
	    goto err;
	return posix_setenv_f_impl(varname, value, override);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_setenv_f_impl(DeeObject *varname, DeeObject *value, bool override)
//[[[end]]]
{
	int error;
	DREF DeeObject *env;
	env = DeeObject_GetAttr(FS_MODULE, (DeeObject *)&libposix_libfs_name_environ);
	if unlikely(!env)
		goto err;
	if (override) {
		error = DeeObject_SetItem(env, varname, value);
	} else {
		error = DeeObject_HasItem(env, varname);
		if (error == 0)
			error = DeeObject_SetItem(env, varname, value);
	}
	Dee_Decref(env);
	if unlikely(error < 0)
		goto err;
	return_none;
err:
	return NULL;
}





/************************************************************************/
/* putenv()                                                             */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("putenv", "string:c:char[]->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_putenv_f_impl(/*utf-8*/ char const *string);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_putenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_PUTENV_DEF { "putenv", (DeeObject *)&posix_putenv, MODSYM_FNORMAL, DOC("(string:?Dstring)->?Dstring") },
#define POSIX_PUTENV_DEF_DOC(doc) { "putenv", (DeeObject *)&posix_putenv, MODSYM_FNORMAL, DOC("(string:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_putenv, posix_putenv_f);
#ifndef POSIX_KWDS_STRING_DEFINED
#define POSIX_KWDS_STRING_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_string, { K(string), KEND });
#endif /* !POSIX_KWDS_STRING_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_putenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/*utf-8*/ char const *string_str;
	DeeStringObject *string;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_string, "o:putenv", &string))
	    goto err;
	if (DeeObject_AssertTypeExact(string, &DeeString_Type))
	    goto err;
	string_str = DeeString_AsUtf8((DeeObject *)string);
	if unlikely(!string_str)
	    goto err;
	return posix_putenv_f_impl(string_str);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_putenv_f_impl(/*utf-8*/ char const *string)
//[[[end]]]
{
	char const *eq;
	DREF DeeObject *env;
	int error;
	env = DeeObject_GetAttr(FS_MODULE, (DeeObject *)&libposix_libfs_name_environ);
	if unlikely(!env)
		goto err;
	eq = strchr(string, '=');
	if (!eq) {
		error = DeeObject_DelItemString(env, string,
		                                Dee_HashUtf8(string, strlen(string)));
	} else {
		DREF DeeObject *value;
		char const *valstr = eq + 1;
		size_t keylen = (size_t)(eq - string);
		value = DeeString_NewUtf8(valstr, strlen(valstr),
		                          STRING_ERROR_FIGNORE);
		if unlikely(!value)
			goto err_env;
		error = DeeObject_SetItemStringLen(env, string, keylen,
		                                   Dee_HashUtf8(string, keylen),
		                                   value);
		Dee_Decref(value);
	}
	if unlikely(error < 0)
		goto err_env;
	Dee_Decref(env);
	return_none;
err_env:
	Dee_Decref(env);
err:
	return NULL;
}





/************************************************************************/
/* unsetenv()                                                           */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("unsetenv", "varname:?Dstring->?Dstring", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_unsetenv_f_impl(DeeObject *varname);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unsetenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_UNSETENV_DEF { "unsetenv", (DeeObject *)&posix_unsetenv, MODSYM_FNORMAL, DOC("(varname:?Dstring)->?Dstring") },
#define POSIX_UNSETENV_DEF_DOC(doc) { "unsetenv", (DeeObject *)&posix_unsetenv, MODSYM_FNORMAL, DOC("(varname:?Dstring)->?Dstring\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_unsetenv, posix_unsetenv_f);
#ifndef POSIX_KWDS_VARNAME_DEFINED
#define POSIX_KWDS_VARNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_varname, { K(varname), KEND });
#endif /* !POSIX_KWDS_VARNAME_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_unsetenv_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *varname;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_varname, "o:unsetenv", &varname))
	    goto err;
	return posix_unsetenv_f_impl(varname);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_unsetenv_f_impl(DeeObject *varname)
//[[[end]]]
{
	DREF DeeObject *env;
	env = DeeObject_GetAttr(FS_MODULE, (DeeObject *)&libposix_libfs_name_environ);
	if unlikely(!env)
		goto err;
	if (DeeObject_DelItem(env, varname))
		goto err_env;
	Dee_Decref(env);
	return_none;
err_env:
	Dee_Decref(env);
err:
	return NULL;
}





/************************************************************************/
/* clearenv()                                                           */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("clearenv", "", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_clearenv_f_impl(void);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_clearenv_f(size_t argc, DeeObject *const *argv);
#define POSIX_CLEARENV_DEF { "clearenv", (DeeObject *)&posix_clearenv, MODSYM_FNORMAL, DOC("()") },
#define POSIX_CLEARENV_DEF_DOC(doc) { "clearenv", (DeeObject *)&posix_clearenv, MODSYM_FNORMAL, DOC("()\n" doc) },
PRIVATE DEFINE_CMETHOD(posix_clearenv, posix_clearenv_f);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_clearenv_f(size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clearenv"))
	    goto err;
	return posix_clearenv_f_impl();
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_clearenv_f_impl(void)
//[[[end]]]
{
	DREF DeeObject *env;
	env = DeeObject_GetAttr(FS_MODULE, (DeeObject *)&libposix_libfs_name_environ);
	if unlikely(!env)
		goto err;
	if (DeeObject_Assign(env, Dee_EmptyMapping))
		goto err_env;
	Dee_Decref(env);
	return_none;
err_env:
	Dee_Decref(env);
err:
	return NULL;
}




DECL_END

#endif /* !GUARD_DEX_POSIX_P_FS_C_INL */
