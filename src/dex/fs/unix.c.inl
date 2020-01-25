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
#ifndef GUARD_DEX_FS_UNIX_C_INL
#define GUARD_DEX_FS_UNIX_C_INL 1
#ifndef DEE_SOURCE
#define DEE_SOURCE      1
#endif /* !DEE_SOURCE */
#ifndef _KOS_SOURCE
#define _KOS_SOURCE     1
#endif /* !_KOS_SOURCE */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE     1
#endif /* !_BSD_SOURCE */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE     1
#endif /* !_GNU_SOURCE */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* !_POSIX_C_SOURCE */
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE   500
#endif /* !_XOPEN_SOURCE */

#include "libfs.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system.h>
#include <deemon/system-features.h>

#include "_res.h"

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../time/libtime.h"

#ifdef _MSC_VER
typedef int ssize_t;
#define __USE_LARGEFILE64 1
#include <io.h>
#include <direct.h>
#include <wchar.h>
#define __stat64  _stat64
#define stat64    _stat64
#define lstat64   _stat64
#define _wstat    _wstat
#define _wlstat   _wstat
#define _wstat64  _wstat64
#define _wlstat64 _wstat64
#define fstat64   _fstat64

#include <errno.h>
#include <Windows.h>

DECL_BEGIN

#ifdef __INTELLISENSE__
#define _WIO_DEFINED     1
#define _WDIRECT_DEFINED 1
#endif /* __INTELLISENSE__ */

#ifdef __USE_LARGEFILE64
#define dirent64 dirent
#endif /* __USE_LARGEFILE64 */

struct dirent {
	/* Members below are arranged for binary compatibility with `struct _finddata32_t' */
	uint32_t d_attrib;
	uint32_t d_time_create;
	uint32_t d_time_access;
	uint32_t d_time_write;
	uint32_t d_size;
	char     d_name[260];
};

#undef  _DIRENT_HAVE_D_RECLEN
#undef  _DIRENT_HAVE_D_NAMLEN
#undef  _DIRENT_HAVE_D_TYPE
#define _DIRENT_MATCHES_DIRENT64 1

typedef struct __dirstream DIR;
struct __dirstream {
    intptr_t      d_hnd;
    int           d_isfirst;
    struct dirent d_ent;
};

LOCAL int (link)(char const *existing_path,
                 char const *new_path) {
	if (!CreateHardLinkA(new_path, existing_path, NULL)) {
		_set_doserrno(GetLastError());
		return -1;
	}
	return 0;
}

LOCAL int (_wlink)(wchar_t const *existing_path,
                   wchar_t const *new_path) {
	if (!CreateHardLinkW(new_path, existing_path, NULL)) {
		_set_doserrno(GetLastError());
		return -1;
	}
	return 0;
}

LOCAL int (symlink)(char const *UNUSED(target_text),
                    char const *UNUSED(link_path)) {
	_set_errno(ENOSYS);
	return -1;
}

LOCAL ssize_t (readlink)(char const *UNUSED(path),
                         char *UNUSED(buffer),
                         size_t UNUSED(bufsize)) {
	_set_errno(ENOSYS);
	return -1;
}


LOCAL DIR *(opendir)(char const *name) {
	DIR *result;
	size_t namelen = strlen(name);
	char *query    = (char *)malloc((namelen + 3) * sizeof(char));
	if unlikely(!query)
		return NULL;
	result = (DIR *)malloc(sizeof(DIR));
	if unlikely(!result)
		goto done;
	memcpy(query, name, namelen * sizeof(char));
	query[namelen]     = '\\';
	query[namelen + 1] = '*';
	query[namelen + 2] = '\0';
	result->d_isfirst  = 1;
	result->d_hnd      = _findfirst32(query, (struct _finddata32_t *)&result->d_ent.d_attrib);
	if unlikely(result->d_hnd == -1) {
		free(result);
		result = 0;
	}
done:
	free(query);
	return result;
}

LOCAL int (closedir)(DIR *dirp) {
#if 0
	if unlikely(!dirp) {
		_set_errno(EINVAL);
		return -1;
	}
#endif
	_findclose(dirp->d_hnd);
	free(dirp);
	return 0;
}

LOCAL struct dirent *(readdir)(DIR *dirp) {
	if unlikely(!dirp) {
		_set_errno(EINVAL);
		return NULL;
	}
	if (!dirp->d_isfirst) {
		if (_findnext32(dirp->d_hnd, (struct _finddata32_t *)&dirp->d_ent.d_attrib))
			return NULL;
	}
	dirp->d_isfirst = 0;
	return &dirp->d_ent;
}

#ifdef __USE_LARGEFILE64
#define readdir64(dir) readdir(dir)
#endif /* __USE_LARGEFILE64 */

DECL_END

#undef getcwd
#define getcwd  _getcwd

#else /* _MSC_VER */
#include <unistd.h>
#include <dirent.h>
#endif /* !_MSC_VER */

#ifndef __INTELLISENSE__
/* Pull in definitions for an stdlib-style environ. */
#include "environ.c.inl"
#endif /* !__INTELLISENSE__ */

#ifdef _WDIRECT_DEFINED
#include <wchar.h>
#endif /* _WDIRECT_DEFINED */


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

#ifndef S_ISDIR
#define S_ISDIR(x) (((x)&STAT_IFMT) == STAT_IFDIR)
#endif /* !S_ISDIR */
#ifndef S_ISCHR
#define S_ISCHR(x) (((x)&STAT_IFMT) == STAT_IFCHR)
#endif /* !S_ISCHR */
#ifndef S_ISBLK
#define S_ISBLK(x) (((x)&STAT_IFMT) == STAT_IFBLK)
#endif /* !S_ISBLK */
#ifndef S_ISREG
#define S_ISREG(x) (((x)&STAT_IFMT) == STAT_IFREG)
#endif /* !S_ISREG */
#ifndef S_ISFIFO
#define S_ISFIFO(x) (((x)&STAT_IFMT) == STAT_IFIFO)
#endif /* !S_ISFIFO */
#ifndef S_ISLNK
#define S_ISLNK(x) (((x)&STAT_IFMT) == STAT_IFLNK)
#endif /* !S_ISLNK */
#ifndef S_ISSOCK
#define S_ISSOCK(x) (((x)&STAT_IFMT) == STAT_IFSOCK)
#endif /* !S_ISSOCK */


#ifdef __KOS__
/* Even though it may be provided, don't use KOS's WIO API, since
 * it's implementation always just converts everything into UTF-8,
 * before passing everything to the normal functions */
#undef _WIO_DEFINED
#endif /* __KOS__ */



DECL_BEGIN

#ifndef CONFIG_HAVE_memrchr
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

#ifndef CONFIG_HAVE_strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */




#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif /* !HOST_NAME_MAX */

#if !HOST_NAME_MAX
#undef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif /* !HOST_NAME_MAX */

INTERN WUNUSED DREF /*String*/ DeeObject *DCALL fs_gethostname(void) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	size_t buflen                  = HOST_NAME_MAX;
	char *newbuf;
	char *buf = unicode_printer_alloc_utf8(&printer, buflen);
	if unlikely(!buf)
		goto err_printer;
	DBG_ALIGNMENT_DISABLE();
	while (gethostname(buf, buflen) < 0) {
		int error = errno;
		DBG_ALIGNMENT_ENABLE();
#ifdef ENAMETOOLONG
		if (error == EINVAL || error == ENAMETOOLONG)
#else /* ENAMETOOLONG */
		if (error == EINVAL)
#endif /* !ENAMETOOLONG */
		{
#ifdef ENAMETOOLONG
			if (error == EINVAL)
#endif /* ENAMETOOLONG */
			{
				if (buflen >= 0x10000)
					goto err_generic;
			}
			buflen *= 2;
			newbuf = unicode_printer_resize_utf8(&printer, buf, buflen);
			if unlikely(!newbuf)
				goto err;
		} else {
err_generic:
			DeeError_SysThrowf(&DeeError_SystemError, error,
			                   "Failed to determine host name");
			goto err;
		}
		DBG_ALIGNMENT_DISABLE();
	}
	DBG_ALIGNMENT_ENABLE();
	if (unicode_printer_confirm_utf8(&printer, buf, strnlen(buf, buflen)) < 0)
		goto err_printer;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_free_utf8(&printer, buf);
err_printer:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE DEFINE_STRING(tmpdir_0, "TMPDIR");
PRIVATE DEFINE_STRING(tmpdir_1, "TMP");
PRIVATE DEFINE_STRING(tmpdir_2, "TEMP");
PRIVATE DEFINE_STRING(tmpdir_3, "TEMPDIR");
PRIVATE DeeObject *tmpdir_vars[] = {
	(DeeObject *)&tmpdir_0,
	(DeeObject *)&tmpdir_1,
	(DeeObject *)&tmpdir_2,
	(DeeObject *)&tmpdir_3
};

#ifndef P_tmpdir
#define P_tmpdir  "/tmp"
#endif /* !P_tmpdir */
PRIVATE DEFINE_STRING(tmpdir_default, P_tmpdir);

INTERN WUNUSED DREF DeeObject *DCALL fs_gettmp(void) {
	DREF DeeObject *result;
	size_t i;
	if (DeeThread_CheckInterrupt())
		return NULL;
	for (i = 0; i < COMPILER_STRLEN(tmpdir_vars); ++i)
		if ((result = fs_getenv(tmpdir_vars[i], true)) != NULL)
			goto done;
	/* Fallback: Lookup using windows. */
	result = (DREF DeeObject *)&tmpdir_default;
	Dee_Incref(result);
done:
	return result;
}

PRIVATE ATTR_COLD int DCALL
err_path_no_access(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Search permissions are not granted for path %r",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_path_no_write_access(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Write permissions have not been granted for path %r",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_path_exists(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileExists, error,
	                          "Path %r already exists",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_path_no_dir(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_NoDirectory, error,
	                          "Some part of the path %r is not a directory",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_path_is_dir(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_IsDirectory, error,
	                          "Path %r is a directory",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_path_not_empty(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_NotEmpty, error,
	                          "The directory %r cannot be deleted because it is not empty",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_path_readonly(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_ReadOnlyFile, error,
	                          "Path %r is apart of a read-only filesystem",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_path_busy(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_BusyFile, error,
	                          "Path %r cannot be deleted because it is still in use",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_path_not_found(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileNotFound, error,
	                          "Path %r could not be found",
	                          path);
}

PRIVATE ATTR_COLD int DCALL
err_handle_closed(int error, DeeObject *__restrict path) {
	return DeeError_SysThrowf(&DeeError_FileClosed, error,
	                          "The given handle %r has been closed",
	                          path);
}



INTERN WUNUSED NONNULL((1)) int DCALL fs_chdir(DeeObject *__restrict path) {
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (!DeeString_Check(path)) {
#if !defined(CONFIG_NO_FCHDIR) && \
    (defined(CONFIG_HAVE_FCHDIR) || defined(fchdir) || \
     defined(__USE_XOPEN_EXTENDED) || defined(__USE_XOPEN2K8))
#define HAVE_FCHDIR 1
		int fd;
		if (DeeInt_Check(path)) {
			if (DeeObject_AsInt(path, &fd))
				goto err;
		} else {
			fd = DeeUnixSystem_GetFD(path);
			if unlikely(fd == -1) {
				if (DeeError_Catch(&DeeError_AttributeError) ||
				    DeeError_Catch(&DeeError_NotImplemented))
					goto try_filename;
				goto err;
			}
		}
		DBG_ALIGNMENT_DISABLE();
		error = fchdir(fd);
		DBG_ALIGNMENT_ENABLE();
		goto check_error;
try_filename:
#endif
		/* Use the filename of a file stream. */
		path = DeeFile_Filename(path);
		if unlikely(!path)
			goto err;
		error = fs_chdir(path);
		Dee_Decref(path);
		return error;
	}
	/* Allow an empty path as an alias for `chdir(".")' (aka. no-op) */
	if (DeeString_WLEN(path) == 0)
		goto done;
#ifdef _WDIRECT_DEFINED
	{
		wchar_t *wstr = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wstr)
			goto err;
		if (!*wstr && !WSTR_LENGTH(wstr))
			goto done;
		DBG_ALIGNMENT_DISABLE();
		error = _wchdir(wstr);
		DBG_ALIGNMENT_ENABLE();
	}
#else /* _WDIRECT_DEFINED */
	{
		char *utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		if unlikely(!*utf8)
			goto done;
		DBG_ALIGNMENT_DISABLE();
		error = chdir(utf8);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* !_WDIRECT_DEFINED */
#ifdef HAVE_FCHDIR
check_error:
#endif /* HAVE_FCHDIR */
	if unlikely(error) {
		error = errno;
		if (error == EACCES) {
			err_path_no_access(error, path);
		} else if (error == ENOTDIR) {
			err_path_no_dir(error, path);
		} else if (error == ENOENT) {
			err_path_not_found(error, path);
		} else if (error == EBADF) {
			err_handle_closed(error, path);
		} else {
			DeeError_SysThrowf(&DeeError_FSError, error,
			                   "Failed to change the current working directory to %r",
			                   path);
		}
		goto err;
	}
done:
	return 0;
err:
	return -1;
}


/* User (pwent) implementation. */
struct user_object {
	OBJECT_HEAD
	/* TODO: pwent */
	DREF DeeStringObject *u_name; /* [0..1][lock(WRITE_ONCE)] The name of the user. */
	DREF DeeStringObject *u_home; /* [0..1][lock(WRITE_ONCE)] The home folder of the user. */
};

INTERN WUNUSED DREF DeeObject *DCALL
fs_gethome(bool try_get) {
	(void)try_get; /* TODO */
	return_empty_string;
}

INTERN WUNUSED DREF DeeObject *DCALL
fs_getuser(bool try_get) {
	(void)try_get; /* TODO */
	return_empty_string;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_printhome(struct unicode_printer *__restrict printer, bool try_get) {
	(void)printer; /* TODO */
	if (try_get)
		return 1;
	return 0;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_printuser(struct unicode_printer *__restrict printer, bool try_get) {
	(void)printer; /* TODO */
	if (try_get)
		return 1;
	return 0;
}

INTERN DeeTypeObject DeeUser_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_User_tp_name,
	/* .tp_doc      = */ S_User_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DeeUserObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

#ifdef __USE_LARGEFILE64
#define STRUCT_STAT struct stat64
#define STAT        stat64
#define LSTAT       lstat64
#define FSTAT       fstat64
#ifdef _WIO_DEFINED
#define _WSTAT      _wstat64
#define _WLSTAT     _wlstat64
#endif /* _WIO_DEFINED */
#else /* __USE_LARGEFILE64 */
#define STRUCT_STAT struct stat
#define STAT        stat
#define LSTAT       lstat
#define FSTAT       fstat
#ifdef _WIO_DEFINED
#define _WSTAT      _wstat
#define _WLSTAT     _wlstat
#endif /* _WIO_DEFINED */
#endif /* !__USE_LARGEFILE64 */


/* STAT implementation. */
struct stat_object {
	OBJECT_HEAD
	STRUCT_STAT st_stat;
};

/* @return:  1: `try_stat' was true and the given `path' could not be found.
 * @return:  0: Successfully did a stat() in the given `path'.
 * @return: -1: The state failed and an error was thrown. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Stat_Init(STRUCT_STAT *__restrict self,
          DeeObject *__restrict path,
          bool try_stat, bool do_lstat) {
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (!DeeString_Check(path)) {
		int fd;
		if (DeeInt_Check(path)) {
			if (DeeObject_AsInt(path, &fd))
				goto err;
		} else {
			fd = DeeUnixSystem_GetFD(path);
			if unlikely(fd == -1) {
				if (DeeError_Catch(&DeeError_AttributeError) ||
				    DeeError_Catch(&DeeError_NotImplemented)) {
					/* Try the file's filename. */
					path = DeeFile_Filename(path);
					if unlikely(!path)
						goto err;
					error = Stat_Init(self, path, try_stat, do_lstat);
					Dee_Decref(path);
					return error;
				}
				goto err;
			}
		}
		/* Do a file-descriptor stat. */
		DBG_ALIGNMENT_DISABLE();
		error = FSTAT(fd, self);
		DBG_ALIGNMENT_ENABLE();
	} else {
#ifdef _WIO_DEFINED
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			goto err;
		/* Do a filename stat. */
		DBG_ALIGNMENT_DISABLE();
		error = do_lstat ? _WLSTAT(wpath, self)
		                 : _WSTAT(wpath, self);
		DBG_ALIGNMENT_ENABLE();
#else /* _WIO_DEFINED */
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		/* Do a filename stat. */
		DBG_ALIGNMENT_DISABLE();
		error = do_lstat ? LSTAT(utf8, self)
		                 : STAT(utf8, self);
		DBG_ALIGNMENT_ENABLE();
#endif /* !_WIO_DEFINED */
	}
	if unlikely(error) {
		error = errno;
		if (error == EACCES) {
			err_path_no_access(error, path);
		} else if (error == ENOTDIR) {
			if (try_stat)
				return 1;
			err_path_no_dir(error, path);
		} else if (error == ENOENT) {
			if (try_stat)
				return 1;
			err_path_not_found(error, path);
		} else if (error == EBADF) {
			err_handle_closed(error, path);
		} else {
			DeeError_SysThrowf(&DeeError_FSError, error,
			                   "Failed to stat %r",
			                   path);
		}
		goto err;
	}
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
stat_ctor(DeeStatObject *__restrict self,
          size_t argc, DeeObject **argv) {
	DeeObject *path;
	if (DeeArg_Unpack(argc, argv, "o:stat", &path))
		goto err;
	return Stat_Init(&self->st_stat, path, false, false);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
lstat_ctor(DeeStatObject *__restrict self,
           size_t argc, DeeObject **argv) {
	DeeObject *path;
	if (DeeArg_Unpack(argc, argv, "o:lstat", &path))
		goto err;
	return Stat_Init(&self->st_stat, path, false, true);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_dev(DeeStatObject *__restrict self) {
	__STATIC_IF(sizeof(self->st_stat.st_dev) <= 4) {
		return DeeInt_NewU32((uint32_t)self->st_stat.st_dev);
	}
	__STATIC_ELSE(sizeof(self->st_stat.st_dev) <= 4) {
		return DeeInt_NewU64((uint64_t)self->st_stat.st_dev);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_ino(DeeStatObject *__restrict self) {
	__STATIC_IF(sizeof(self->st_stat.st_ino) <= 2) {
		return DeeInt_NewU16((uint16_t)self->st_stat.st_ino);
	}
	__STATIC_ELSE(sizeof(self->st_stat.st_ino) <= 2) {
		__STATIC_IF(sizeof(self->st_stat.st_ino) <= 4) {
			return DeeInt_NewU32((uint32_t)self->st_stat.st_ino);
		}
		__STATIC_ELSE(sizeof(self->st_stat.st_ino) <= 4) {
			return DeeInt_NewU64((uint64_t)self->st_stat.st_ino);
		}
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_mode(DeeStatObject *__restrict self) {
	__STATIC_IF(sizeof(self->st_stat.st_mode) <= 2) {
		return DeeInt_NewU16((uint16_t)self->st_stat.st_mode);
	}
	__STATIC_ELSE(sizeof(self->st_stat.st_mode) <= 2) {
		return DeeInt_NewU32((uint32_t)self->st_stat.st_mode);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_nlink(DeeStatObject *__restrict self) {
	__STATIC_IF(sizeof(self->st_stat.st_nlink) <= 2) {
		return DeeInt_NewU16((uint16_t)self->st_stat.st_nlink);
	}
	__STATIC_ELSE(sizeof(self->st_stat.st_nlink) <= 2) {
		__STATIC_IF(sizeof(self->st_stat.st_nlink) <= 4) {
			return DeeInt_NewU32((uint32_t)self->st_stat.st_nlink);
		}
		__STATIC_ELSE(sizeof(self->st_stat.st_nlink) <= 4) {
			return DeeInt_NewU64((uint64_t)self->st_stat.st_nlink);
		}
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_rdev(DeeStatObject *__restrict self) {
	__STATIC_IF(sizeof(self->st_stat.st_rdev) <= 4) {
		return DeeInt_NewU32((uint32_t)self->st_stat.st_rdev);
	}
	__STATIC_ELSE(sizeof(self->st_stat.st_rdev) <= 4) {
		return DeeInt_NewU64((uint64_t)self->st_stat.st_rdev);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_size(DeeStatObject *__restrict self) {
	__STATIC_IF(sizeof(self->st_stat.st_size) <= 4) {
		return DeeInt_NewU32((uint32_t)self->st_stat.st_size);
	}
	__STATIC_ELSE(sizeof(self->st_stat.st_size) <= 4) {
		return DeeInt_NewU64((uint64_t)self->st_stat.st_size);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_atime(DeeStatObject *__restrict self) {
	/* TODO: Extensions for better resolution. */
	return DeeTime_New((uint64_t)self->st_stat.st_atime * MICROSECONDS_PER_SECOND);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_mtime(DeeStatObject *__restrict self) {
	/* TODO: Extensions for better resolution. */
	return DeeTime_New((uint64_t)self->st_stat.st_mtime * MICROSECONDS_PER_SECOND);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_get_ctime(DeeStatObject *__restrict self) {
	/* TODO: Extensions for better resolution. */
	return DeeTime_New((uint64_t)self->st_stat.st_ctime * MICROSECONDS_PER_SECOND);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isdir(DeeStatObject *__restrict self) {
	return_bool(S_ISDIR(self->st_stat.st_mode));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_ischr(DeeStatObject *__restrict self) {
	return_bool(S_ISCHR(self->st_stat.st_mode));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isblk(DeeStatObject *__restrict self) {
	return_bool(S_ISBLK(self->st_stat.st_mode));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isreg(DeeStatObject *__restrict self) {
	return_bool(S_ISREG(self->st_stat.st_mode));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_isfifo(DeeStatObject *__restrict self) {
	return_bool(S_ISFIFO(self->st_stat.st_mode));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_islnk(DeeStatObject *__restrict self) {
	return_bool(S_ISLNK(self->st_stat.st_mode));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_issock(DeeStatObject *__restrict self) {
	return_bool(S_ISSOCK(self->st_stat.st_mode));
}



PRIVATE struct type_getset stat_getsets[] = {
	{ "st_dev", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_dev, NULL, NULL, S_Stat_getset_st_dev_doc },
	{ "st_ino", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_ino, NULL, NULL, S_Stat_getset_st_ino_doc },
	{ "st_mode", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_mode, NULL, NULL, S_Stat_getset_st_mode_doc },
	{ "st_nlink", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_nlink, NULL, NULL, S_Stat_getset_st_nlink_doc },
	/* >> property st_uid -> user;
	 * >> property st_gid -> group; */
	{ "st_rdev", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_rdev, NULL, NULL, S_Stat_getset_st_rdev_doc },
	{ "st_size", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_size, NULL, NULL, S_Stat_getset_st_size_doc },
	{ "st_atime", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_atime, NULL, NULL, S_Stat_getset_st_atime_doc },
	{ "st_mtime", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_mtime, NULL, NULL, S_Stat_getset_st_mtime_doc },
	{ "st_ctime", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_get_ctime, NULL, NULL, S_Stat_getset_st_ctime_doc },
	{ "isdir", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_isdir, NULL, NULL, S_Stat_getset_isdir_doc },
	{ "ischr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_ischr, NULL, NULL, S_Stat_getset_ischr_doc },
	{ "isblk", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_isblk, NULL, NULL, S_Stat_getset_isblk_doc },
	{ "isreg", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_isreg, NULL, NULL, S_Stat_getset_isreg_doc },
	{ "isfifo", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_isfifo, NULL, NULL, S_Stat_getset_isfifo_doc },
	{ "islnk", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_islnk, NULL, NULL, S_Stat_getset_islnk_doc },
	{ "issock", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&stat_issock, NULL, NULL, S_Stat_getset_issock_doc },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_exists(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_exists_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, self == (DeeObject *)&DeeLStat_Type);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isdir(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_isdir_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, self == (DeeObject *)&DeeLStat_Type);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0 && S_ISDIR(buf.st_mode));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_ischr(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_ischr_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, self == (DeeObject *)&DeeLStat_Type);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0 && S_ISCHR(buf.st_mode));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isblk(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_isblk_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, self == (DeeObject *)&DeeLStat_Type);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0 && S_ISBLK(buf.st_mode));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isreg(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_isreg_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, self == (DeeObject *)&DeeLStat_Type);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0 && S_ISREG(buf.st_mode));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isfifo(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_isfifo_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, self == (DeeObject *)&DeeLStat_Type);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0 && S_ISFIFO(buf.st_mode));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_islnk(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_islnk_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, true); /* Always do an lstat() for this */
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0 && S_ISLNK(buf.st_mode));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_issock(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_issock_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, self == (DeeObject *)&DeeLStat_Type);
	if unlikely(error < 0)
		goto err;
	return_bool(error == 0 && S_ISSOCK(buf.st_mode));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
do_ishidden(DeeObject *__restrict path) {
	int result;
	char *begin, *iter;
	if (!DeeString_Check(path)) {
		path = DeeFile_Filename(path);
		if unlikely(!path)
			goto err;
		result = do_ishidden(path);
		Dee_Decref(path);
		goto done;
	}
	/* Check if the filename starts with a `.' (DOT) */
	iter = (begin = DeeString_STR(path)) + DeeString_SIZE(path);
	while (iter != begin && iter[-1] != '/')
		--iter;
	return *iter == '.';
done:
	return result;
err:
	return -1;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
stat_class_ishidden(DeeObject *__restrict UNUSED(self),
                    size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_ishidden_name, &path))
		goto err;
	error = do_ishidden(path);
	if (error < 0)
		goto err;
	return_bool(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
stat_class_isexe(DeeObject *self, size_t argc, DeeObject **argv) {
	DeeObject *path;
	int error;
	STRUCT_STAT buf;
	if (DeeArg_Unpack(argc, argv, "o:" S_Stat_class_function_isexe_name, &path))
		goto err;
	error = Stat_Init(&buf, path, true, self == (DeeObject *)&DeeLStat_Type);
	if unlikely(error < 0)
		goto err;
	/* Check for execute-permissions. */
	return_bool(error == 0 && (buf.st_mode & 0111));
err:
	return NULL;
}

PRIVATE struct type_method stat_class_methods[] = {
	{ S_Stat_class_function_exists_name, &stat_class_exists, S_Stat_class_function_exists_doc },
	{ S_Stat_class_function_isdir_name, &stat_class_isdir, S_Stat_class_function_isdir_doc },
	{ S_Stat_class_function_ischr_name, &stat_class_ischr, S_Stat_class_function_ischr_doc },
	{ S_Stat_class_function_isblk_name, &stat_class_isblk, S_Stat_class_function_isblk_doc },
	{ S_Stat_class_function_isreg_name, &stat_class_isreg, S_Stat_class_function_isreg_doc },
	{ S_Stat_class_function_isfifo_name, &stat_class_isfifo, S_Stat_class_function_isfifo_doc },
	{ S_Stat_class_function_islnk_name, &stat_class_islnk, S_Stat_class_function_islnk_doc },
	{ S_Stat_class_function_issock_name, &stat_class_issock, S_Stat_class_function_issock_doc },
	{ S_Stat_class_function_ishidden_name, &stat_class_ishidden, S_Stat_class_function_ishidden_doc },
	{ S_Stat_class_function_isexe_name, &stat_class_isexe, S_Stat_class_function_isexe_doc },
	{ NULL }
};

INTERN DeeTypeObject DeeStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Stat_tp_name,
	/* .tp_doc      = */ S_Stat_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&stat_ctor,
				TYPE_FIXED_ALLOCATOR(DeeStatObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ stat_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ stat_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DeeLStat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_LStat_tp_name,
	/* .tp_doc      = */ S_LStat_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeStat_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&lstat_ctor,
				TYPE_FIXED_ALLOCATOR(DeeStatObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE ATTR_COLD int DCALL
handle_rmdir_error(int error, DeeObject *__restrict path) {
	if (error == EACCES) {
no_access:
		DBG_ALIGNMENT_ENABLE();
		return err_path_no_write_access(error, path);
	} else if (error == EBUSY || error == EINVAL) {
		return err_path_busy(error, path);
	} else if (error == ENOENT) {
		return err_path_not_found(error, path);
	} else if (error == ENOTDIR) {
		return err_path_no_dir(error, path);
	} else if (error == ENOTEMPTY) {
		return err_path_not_empty(error, path);
	} else if (error == EROFS) {
		return err_path_readonly(error, path);
	} else if (error == EPERM) {
		STRUCT_STAT st;
		/* Posix states that `EPERM' may be returned because of the sticky bit.
		 * However in that event, we want to throw an access error, not an
		 * unsupported-api error. */
#ifdef _WIO_DEFINED
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!_WLSTAT(wpath, &st) && (st.st_mode & STAT_ISVTX))
			goto no_access;
		DBG_ALIGNMENT_ENABLE();
#else /* _WIO_DEFINED */
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!LSTAT(utf8, &st) && (st.st_mode & STAT_ISVTX))
			goto no_access;
		DBG_ALIGNMENT_ENABLE();
#endif /* !_WIO_DEFINED */
		return DeeError_SysThrowf(&DeeError_UnsupportedAPI, error,
		                          "The filesystem hosting the path %r does "
		                          "not support the removal of directories",
		                          path);
	} else {
		return DeeError_SysThrowf(&DeeError_FSError, error,
		                          "Failed to remove directory %r",
		                          path);
	}
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_rmdir(DeeObject *__restrict path) {
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef _WDIRECT_DEFINED
	{
		wchar_t *wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = _wrmdir(wpath);
		DBG_ALIGNMENT_ENABLE();
	}
#else /* _WIO_DEFINED */
	{
		char *utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = rmdir(utf8);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* !_WIO_DEFINED */
	if unlikely(error)
		return handle_rmdir_error(errno, path);
	return 0;
err:
	return -1;
}

PRIVATE ATTR_COLD int DCALL
handle_unlink_error(int error, DeeObject *__restrict path) {
	if (error == EACCES) {
err_access:
		return err_path_no_write_access(error, path);
	} else if (error == EBUSY) {
		return err_path_busy(error, path);
	} else if (error == EISDIR) {
err_isdir:
		return err_path_is_dir(error, path);
	} else if (error == ENOTDIR) {
		return err_path_no_dir(error, path);
	} else if (error == ENOENT) {
		return err_path_not_found(error, path);
	} else if (error == EPERM) {
		/* Posix states that this is the return value when the path is a directory,
		 * but also if the filesystem does not support unlinking files. */
		STRUCT_STAT st;
#ifdef _WIO_DEFINED
		wchar_t *wpath;
		wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!_WLSTAT(wpath, &st))
#else /* _WIO_DEFINED */
		char *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			return -1;
		DBG_ALIGNMENT_DISABLE();
		if (!LSTAT(utf8, &st))
#endif /* !_WIO_DEFINED */
		{
			DBG_ALIGNMENT_ENABLE();
			if (S_ISDIR(st.st_mode))
				goto err_isdir;
			/* Posix also states that the presence of the
			 * S_ISVTX bit may cause EPERM to be returned. */
			if (st.st_mode & STAT_ISVTX)
				goto err_access;
		}
		DBG_ALIGNMENT_ENABLE();
		return DeeError_SysThrowf(&DeeError_UnsupportedAPI, error,
		                          "The filesystem hosting the path %r "
		                          "does not support unlinking files",
		                          path);
	} else {
		return DeeError_SysThrowf(&DeeError_FSError, error,
		                          "Failed to unlink file %r",
		                          path);
	}
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_unlink(DeeObject *__restrict path) {
	int error;
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef _WIO_DEFINED
	{
		wchar_t *wpath = (wchar_t *)DeeString_AsWide(path);
		if unlikely(!wpath)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = _wunlink(wpath);
		DBG_ALIGNMENT_ENABLE();
	}
#else
	{
		char *utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		error = unlink(utf8);
		DBG_ALIGNMENT_ENABLE();
	}
#endif
	if unlikely(error) {
		handle_unlink_error(errno, path);
		goto err;
	}
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
fs_remove(DeeObject *__restrict path) {
	int error;
#ifdef _WIO_DEFINED
	wchar_t *wpath;
#else
	char *utf8;
#endif
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(path, &DeeString_Type))
		goto err;
#ifdef _WIO_DEFINED
	wpath = (wchar_t *)DeeString_AsWide(path);
	if unlikely(!wpath)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = _wunlink(wpath);
	DBG_ALIGNMENT_ENABLE();
#else
	utf8 = DeeString_AsUtf8(path);
	if unlikely(!utf8)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = unlink(utf8);
	DBG_ALIGNMENT_ENABLE();
#endif
	if (error) {
		error = errno;
		if (error == EISDIR || error == EPERM) {
			/* Delete a directory. */
			DBG_ALIGNMENT_DISABLE();
#ifdef _WIO_DEFINED
			error = _wrmdir(wpath);
#else
			error = rmdir(utf8);
#endif
			DBG_ALIGNMENT_ENABLE();
			if likely(!error)
				goto done;
			handle_rmdir_error(errno, path);
		} else {
			/* Handle errors. */
			handle_unlink_error(error, path);
		}
		goto err;
	}
done:
	return 0;
err:
	return -1;
}

PRIVATE ATTR_COLD int DCALL
err_path_no_access2(int error,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
	return DeeError_SysThrowf(&DeeError_FileAccessError, error,
	                          "Access to %r or %r has not been granted",
	                          existing_path, new_path);
}

PRIVATE ATTR_COLD int DCALL
err_path_not_found2(int error,
                    DeeObject *__restrict existing_path,
                    DeeObject *__restrict new_path) {
	return DeeError_SysThrowf(&DeeError_FileNotFound, error,
	                          "Path %r or %r could not be found",
	                          existing_path, new_path);
}

PRIVATE ATTR_COLD int DCALL
err_path_readonly2(int error,
                   DeeObject *__restrict existing_path,
                   DeeObject *__restrict new_path) {
	return DeeError_SysThrowf(&DeeError_ReadOnlyFile, error,
	                          "Path %r and %r is apart of a read-only filesystem",
	                          existing_path, new_path);
}

PRIVATE ATTR_COLD int DCALL
err_path_crossdev2(int error,
                   DeeObject *__restrict existing_path,
                   DeeObject *__restrict new_path) {
	return DeeError_SysThrowf(&DeeError_CrossDevice, error,
	                          "Paths %r and %r are not apart of the same filesystem",
	                          existing_path, new_path);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_rename(DeeObject *__restrict existing_path,
          DeeObject *__restrict new_path) {
	int error;
#ifdef _WIO_DEFINED
	wchar_t *woldpath;
	wchar_t *wnewpath;
#else
	char *uoldpath;
	char *unewpath;
#endif
	if (!DeeString_Check(existing_path)) {
		existing_path = DeeFile_Filename(existing_path);
		if unlikely(!existing_path)
			goto err;
		error = fs_rename(existing_path, new_path);
		Dee_Decref(existing_path);
		return error;
	}
	if (DeeThread_CheckInterrupt())
		goto err;
	if (DeeObject_AssertTypeExact(new_path, &DeeString_Type))
		goto err;
#ifdef _WIO_DEFINED
	woldpath = (wchar_t *)DeeString_AsWide(existing_path);
	if unlikely(!woldpath)
		goto err;
	wnewpath = (wchar_t *)DeeString_AsWide(new_path);
	if unlikely(!wnewpath)
		goto err;
#else
	uoldpath = DeeString_AsUtf8(existing_path);
	if unlikely(!uoldpath)
		goto err;
	unewpath = DeeString_AsUtf8(new_path);
	if unlikely(!unewpath)
		goto err;
#endif
	DBG_ALIGNMENT_DISABLE();
#ifdef _WIO_DEFINED
	error = _wrename(woldpath, wnewpath);
#else
	error = rename(uoldpath, unewpath);
#endif
	DBG_ALIGNMENT_ENABLE();
	if likely(!error)
		return 0;
	error = errno;
	if (error == EACCES) {
err_access:
		DBG_ALIGNMENT_ENABLE();
		err_path_no_access2(error, existing_path, new_path);
	} else if (error == EBUSY) {
		DeeError_SysThrowf(&DeeError_BusyFile, error,
		                   "Path %r or %r cannot be accessed because it is already in use",
		                   existing_path, new_path);
	} else if (error == EINVAL) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot rename path %r to %r which is a sub-directory of the old path",
		                existing_path, new_path);
	} else if (error == ENOENT) {
		err_path_not_found2(error, existing_path, new_path);
	} else if (error == ENOTEMPTY || error == EEXIST) {
		err_path_exists(error, new_path);
	} else if (error == ENOTDIR) {
		DeeError_SysThrowf(&DeeError_NoDirectory, error,
		                   "Some part of the path %r or %r is not a directory",
		                   existing_path, new_path);
	} else if (error == EPERM) {
		STRUCT_STAT st;
		/* The same deal concerning the sticky bit. */
		DBG_ALIGNMENT_DISABLE();
#ifdef _WIO_DEFINED
		if ((!_WSTAT(woldpath, &st) && (st.st_mode & STAT_ISVTX)) ||
		    (!_WSTAT(wnewpath, &st) && (st.st_mode & STAT_ISVTX)))
			goto err_access;
#else
		if ((!STAT(uoldpath, &st) && (st.st_mode & STAT_ISVTX)) ||
		    (!STAT(unewpath, &st) && (st.st_mode & STAT_ISVTX)))
			goto err_access;
#endif
		DBG_ALIGNMENT_ENABLE();
		DeeError_SysThrowf(&DeeError_UnsupportedAPI, error,
		                   "The filesystem hosting the paths %r and %r "
		                   "does not support renaming of files",
		                   existing_path, new_path);
	} else if (error == EROFS) {
		err_path_readonly2(error, existing_path, new_path);
	} else if (error == EXDEV) {
		err_path_crossdev2(error, existing_path, new_path);
	} else {
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to rename %r to %r",
		                   existing_path, new_path);
	}
err:
	return -1;
}



INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_link(DeeObject *__restrict existing_path,
        DeeObject *__restrict new_path) {
	int error;
#ifdef _WIO_DEFINED
	wchar_t *woldpath;
	wchar_t *wnewpath;
#else
	char *uoldpath;
	char *unewpath;
#endif
	if (!DeeString_Check(existing_path)) {
		existing_path = DeeFile_Filename(existing_path);
		if unlikely(!existing_path)
			goto err;
		error = fs_link(existing_path, new_path);
		Dee_Decref(existing_path);
		return error;
	}
	if (DeeObject_AssertTypeExact(new_path, &DeeString_Type))
		goto err;
#ifdef _WIO_DEFINED
	woldpath = (wchar_t *)DeeString_AsWide(existing_path);
	if unlikely(!woldpath)
		goto err;
	wnewpath = (wchar_t *)DeeString_AsWide(new_path);
	if unlikely(!wnewpath)
		goto err;
#else
	uoldpath = DeeString_AsUtf8(existing_path);
	if unlikely(!uoldpath)
		goto err;
	unewpath = DeeString_AsUtf8(new_path);
	if unlikely(!unewpath)
		goto err;
#endif
	DBG_ALIGNMENT_DISABLE();
#ifdef _WIO_DEFINED
	error = _wlink(woldpath, wnewpath);
#else
	error = link(uoldpath, unewpath);
#endif
	DBG_ALIGNMENT_ENABLE();
	if likely(!error)
		return 0;
	error = errno;
	if (error == EACCES) {
		err_path_no_access2(error, existing_path, new_path);
	} else if (error == EEXIST) {
		err_path_exists(error, new_path);
	} else if (error == ENOENT) {
		err_path_not_found2(error, existing_path, new_path);
	} else if (error == EPERM) {
		DeeError_SysThrowf(&DeeError_UnsupportedAPI, error,
		                   "The filesystem hosting the paths %r and %r "
		                   "does not support creation of hardlinks",
		                   existing_path, new_path);
	} else if (error == EROFS) {
		err_path_readonly2(error, existing_path, new_path);
	} else if (error == EXDEV) {
		err_path_crossdev2(error, existing_path, new_path);
	} else {
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to rename %r to %r",
		                   existing_path, new_path);
	}
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
fs_symlink(DeeObject *__restrict target_text,
           DeeObject *__restrict link_path,
           bool format_target) {
	int error;
	char *utarget_text;
	char *ulink_path;
	(void)format_target;
	if (DeeObject_AssertTypeExact(target_text, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(link_path, &DeeString_Type))
		goto err;
	utarget_text = DeeString_AsUtf8(target_text);
	if unlikely(!utarget_text)
		goto err;
	ulink_path = DeeString_AsUtf8(link_path);
	if unlikely(!ulink_path)
		goto err;
	DBG_ALIGNMENT_DISABLE();
	error = symlink(utarget_text, ulink_path);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(error) {
		DBG_ALIGNMENT_DISABLE();
		error = errno;
		DBG_ALIGNMENT_ENABLE();
		if (error == EACCES) {
			err_path_no_access(error, link_path);
		} else if (error == EEXIST) {
			err_path_exists(error, link_path);
		} else if (error == ENOENT) {
			err_path_not_found(error, link_path);
		} else if (error == ENOTDIR) {
			err_path_no_dir(error, link_path);
		} else if (error == EPERM) {
			DeeError_SysThrowf(&DeeError_UnsupportedAPI, error,
			                   "The filesystem hosting the path %r "
			                   "does not support creation of symbolic links",
			                   link_path);
		} else if (error == EROFS) {
			err_path_readonly(error, link_path);
		} else {
			DeeError_SysThrowf(&DeeError_FSError, error,
			                   "Failed to create a symbolic link %r with text %r",
			                   link_path, target_text);
		}
	}
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
fs_readlink(DeeObject *__restrict path) {
	if (!DeeString_Check(path)) {
		DREF DeeObject *result;
		path = DeeFile_Filename(path);
		if unlikely(!path)
			return NULL;
		result = fs_readlink(path);
		Dee_Decref(path);
		return result;
	} else {
		char *utf8, *buffer, *new_buffer;
		int error;
		size_t bufsize, new_size;
		dssize_t req_size;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		utf8                           = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err_printer;
		bufsize = PATH_MAX;
		buffer  = unicode_printer_alloc_utf8(&printer, bufsize);
		if unlikely(!buffer)
			goto err_printer;
		for (;;) {
			STRUCT_STAT st;
			DBG_ALIGNMENT_DISABLE();
			req_size = readlink(utf8, buffer, bufsize + 1);
			if unlikely(req_size < 0) {
handle_error:
				error = errno;
				DBG_ALIGNMENT_ENABLE();
				if (error == EACCES) {
					err_path_no_access(error, path);
				} else if (error == ENOTDIR) {
					err_path_no_dir(error, path);
				} else if (error == ENOENT) {
					err_path_not_found(error, path);
				} else if (error == EINVAL) {
no_link:
					DeeError_SysThrowf(&DeeError_NoLink, error,
					                   "Path %r is not a symbolic link",
					                   path);
				} else {
					DeeError_SysThrowf(&DeeError_FSError, error,
					                   "Failed to read symbolic link %r",
					                   path);
				}
				goto err;
			}
			DBG_ALIGNMENT_ENABLE();
			if ((size_t)req_size <= bufsize)
				break;
			DBG_ALIGNMENT_DISABLE();
			if (LSTAT(utf8, &st))
				goto handle_error;
			DBG_ALIGNMENT_ENABLE();
			/* Ensure that this is still a symbolic link. */
			if (!S_ISLNK(st.st_mode)) {
				error = EINVAL;
				goto no_link;
			}
			new_size = (size_t)st.st_size;
			if (new_size <= bufsize)
				break; /* Shouldn't happen, but might due to race conditions? */
			new_buffer = unicode_printer_resize_utf8(&printer, buffer, new_size);
			if unlikely(!new_buffer)
				goto err;
			buffer  = new_buffer;
			bufsize = new_size;
		}
		/* Release unused data. */
		if (unicode_printer_confirm_utf8(&printer, buffer, (size_t)req_size) < 0)
			goto err_printer;
		return unicode_printer_pack(&printer);
err:
		unicode_printer_free_utf8(&printer, buffer);
err_printer:
		unicode_printer_fini(&printer);
		return NULL;
	}
}



typedef struct {
	OBJECT_HEAD
	DREF DeeStringObject *d_path; /* [1..1] The path describing this directory. */
} Dir;

typedef struct {
	OBJECT_HEAD
	DREF Dir *d_dir; /* [1..1][const] The associated directory. */
	DREF DIR *d_hnd; /* [0..1][lock(d_lock)] The directory being iterated. */
#ifndef CONFIG_NO_THREADS
	rwlock_t d_lock;
#endif
} DirIterator;

PRIVATE NONNULL((1)) void DCALL
diriter_fini(DirIterator *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
	closedir(self->d_hnd);
	DBG_ALIGNMENT_ENABLE();
	Dee_Decref(self->d_dir);
}

PRIVATE NONNULL((1, 2)) void DCALL
diriter_visit(DirIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->d_dir);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_TryNewSized(char const *__restrict str, size_t len);

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
diriter_next(DirIterator *__restrict self) {
	DREF DeeStringObject *result;
	struct dirent *ent;
	size_t result_length;
/*again:*/
	rwlock_write(&self->d_lock);
	/* Quick check: Has the iterator been exhausted. */
	if (!self->d_hnd) {
		rwlock_endwrite(&self->d_lock);
iter_done:
		return (DREF DeeStringObject *)ITER_DONE;
	}
read_filename:
	errno = 0;
	DBG_ALIGNMENT_DISABLE();
	ent = readdir(self->d_hnd);
	DBG_ALIGNMENT_ENABLE();
	if (!ent) {
		/* End of directory / error. */
		int error = errno;
		if likely(error == 0) {
			DIR *dfd = self->d_hnd;
			/* End of directory. */
			self->d_hnd = NULL;
			rwlock_endwrite(&self->d_lock);
			DBG_ALIGNMENT_DISABLE();
			closedir(dfd);
			DBG_ALIGNMENT_ENABLE();
			goto iter_done;
		}
		rwlock_endwrite(&self->d_lock);
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to read entires from directory %r",
		                   self->d_dir->d_path);
		goto err;
	}
	/* Skip self/parent directories. */
	if (ent->d_name[0] == '.' &&
	    (ent->d_name[1] == '\0' ||
	     (ent->d_name[1] == '.' && ent->d_name[2] == '\0')))
		goto read_filename;
	result_length = strlen(ent->d_name);
	result        = (DREF DeeStringObject *)DeeString_TryNewSized(ent->d_name, result_length);
	rwlock_endwrite(&self->d_lock);
	if unlikely(!result) {
		Dee_BadAlloc(offsetof(DeeStringObject, s_str) +
		             (result_length + 1) * sizeof(char));
		goto err;
	}
	return result;
err:
	return NULL;
}

PRIVATE ATTR_COLD int DCALL
err_handle_opendir(DeeObject *__restrict path) {
	int error;
	DBG_ALIGNMENT_DISABLE();
	error = errno;
	DBG_ALIGNMENT_ENABLE();
	if (error == ENOENT)
		return err_path_not_found(error, path);
	if (error == ENOTDIR)
		return err_path_no_dir(error, path);
	return DeeError_SysThrowf(&DeeError_FSError, error,
	                          "Failed to open directory %r",
	                          path);
}

PRIVATE WUNUSED NONNULL((1)) DREF DirIterator *DCALL
dir_iter(Dir *__restrict self) {
	DREF DirIterator *result;
	char *utf8;
	if (DeeThread_CheckInterrupt())
		goto err;
	result = DeeObject_MALLOC(DirIterator);
	if unlikely(!result)
		goto err;
	/* Open a directory descriptor. */
	utf8 = DeeString_AsUtf8((DeeObject *)self->d_path);
	if unlikely(!utf8)
		goto err_r;
	DBG_ALIGNMENT_DISABLE();
	result->d_hnd = opendir(utf8);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(!result->d_hnd) {
		err_handle_opendir((DeeObject *)self->d_path);
		goto err_r;
	}
	Dee_Incref(self);
	result->d_dir = self;
	rwlock_init(&result->d_lock);
	DeeObject_Init(result, &DeeDirIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_member diriter_members[] = {
	TYPE_MEMBER_FIELD("seq", STRUCT_OBJECT, offsetof(DirIterator, d_dir)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeDirIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_DirIterator_tp_name,
	/* .tp_doc      = */ S_DirIterator_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL, /* (void *)&diriter_copy, */
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(DirIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&diriter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&diriter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ diriter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE NONNULL((1)) void DCALL
dir_fini(Dir *__restrict self) {
	Dee_Decref(self->d_path);
}

PRIVATE NONNULL((1, 2)) void DCALL
dir_visit(Dir *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->d_path);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dir_copy(Dir *__restrict self,
         Dir *__restrict other) {
	self->d_path = other->d_path;
	Dee_Incref(self->d_path);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dir_ctor(Dir *__restrict self,
         size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, "o:" S_Dir_tp_name, &self->d_path))
		goto err;
	if (DeeString_Check(self->d_path)) {
		Dee_Incref(self->d_path);
	} else {
		/* TODO: fopendir() */
		self->d_path = (DREF DeeStringObject *)DeeFile_Filename((DeeObject *)self->d_path);
		if unlikely(!self->d_path)
			goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE struct type_seq dir_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dir_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))NULL
};

PRIVATE struct type_member dir_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeDirIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeDir_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Dir_tp_name,
	/* .tp_doc      = */ S_Dir_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONLOOPING,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ (void *)&dir_copy,
				/* .tp_deep_ctor = */ (void *)&dir_copy,
				/* .tp_any_ctor  = */ (void *)&dir_ctor,
				TYPE_FIXED_ALLOCATOR(Dir)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dir_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dir_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dir_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dir_class_members
};


/* To a wildcard-enabled string comparison on `string' using `pattern'
 * Taken from one of my other projects: `KOS' - `/libs/libc/string.c:libc_wildstrcmp' */
PRIVATE int DCALL
wild_match(char const *string, char const *pattern) {
	char card_post;
	for (;;) {
		if (!*string) {
			/* End of string (if the patter is empty, or only contains '*', we have a match) */
			while (*pattern == '*')
				++pattern;
			return -(int)*pattern;
		}
		if (!*pattern)
			return (int)*string; /* Pattern end doesn't match */
		if (*pattern == '*') {
			/* Skip starts */
			do {
				++pattern;
			} while (*pattern == '*');
			if ((card_post = *pattern++) == '\0')
				return 0; /* Pattern ends with '*' (matches everything) */
			if (card_post == '?')
				goto next; /* Match any --> already found */
			for (;;) {
				char ch = *string++;
				if (ch == card_post) {
					/* Recursively check if the rest of the string and pattern match */
					if (!wild_match(string, pattern))
						return 0;
				} else if (!ch) {
					return -(int)card_post; /* Wildcard suffix not found */
				}
			}
		}
		if (*pattern == *string || *pattern == '?') {
next:
			++string, ++pattern;
			continue; /* single character match */
		}
		break; /* mismatch */
	}
	return *string - *pattern;
}


/* query() types. */
typedef struct {
	DirIterator q_iter; /* The underlying iterator. */
	char const *q_wild; /* The wildcard pattern string with which to match filenames. */
} QueryIterator;

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
queryiter_next(QueryIterator *__restrict self) {
	DREF DeeStringObject *result;
again:
	result = diriter_next(&self->q_iter);
	if (ITER_ISOK(result)) {
		char *utf8 = DeeString_AsUtf8((DeeObject *)result);
		if unlikely(!utf8)
			goto err_r;
		if (wild_match(utf8, self->q_wild) != 0) {
			/* Non-matching entry (read more) */
			Dee_Decref(result);
			goto again;
		}
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}


INTERN DeeTypeObject DeeQueryIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_QueryIterator_tp_name,
	/* .tp_doc      = */ S_QueryIterator_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeDirIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL, /* (void *)&diriter_copy, */
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(QueryIterator)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&queryiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE struct type_member query_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeQueryIterator_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF QueryIterator *DCALL
query_iter(Dir *__restrict self) {
	DREF QueryIterator *result;
	char *query_start, *query_str;
	if (DeeThread_CheckInterrupt())
		goto err;
	result = DeeObject_MALLOC(QueryIterator);
	if unlikely(!result)
		goto err;
	query_str = DeeString_AsUtf8((DeeObject *)self->d_path);
	if unlikely(!query_str)
		goto err;
	query_start = (char *)memrchr(query_str, '/', WSTR_LENGTH(query_str));
	if (!query_start) {
		/* Open a directory descriptor. */
		result->q_iter.d_hnd = opendir(".");
		query_start          = query_str;
	} else {
		/* Open a directory descriptor. */
		if (DeeObject_IsShared(self->d_path)) {
			char *temp_filename;
			int old_errno;
			size_t temp_filesize = (size_t)(query_start - query_str);
			temp_filename        = (char *)Dee_AMalloc((temp_filesize + 1) * sizeof(char));
			if unlikely(!temp_filename)
				goto err_r;
			memcpy(temp_filename, query_str, temp_filesize * sizeof(char));
			temp_filename[temp_filesize] = '\0'; /* Override the '/' to terminate the string. */
			DBG_ALIGNMENT_DISABLE();
			result->q_iter.d_hnd = opendir(temp_filename);
			/* Free the temporary buffer, but preserve errno. */
			old_errno = errno;
			Dee_AFree(temp_filename);
			errno = old_errno;
			DBG_ALIGNMENT_ENABLE();
		} else {
			/* Cheat a bit so we don't have to allocate a second buffer. */
			*query_start = '\0';
			DBG_ALIGNMENT_DISABLE();
			result->q_iter.d_hnd = opendir(query_str);
			DBG_ALIGNMENT_ENABLE();
			*query_start = '/';
		}
		++query_start;
	}
	if unlikely(!result->q_iter.d_hnd) {
		err_handle_opendir((DeeObject *)self->d_path);
		goto err_r;
	}
	Dee_Incref(self);
	result->q_wild       = query_start;
	result->q_iter.d_dir = self;
	rwlock_init(&result->q_iter.d_lock);
	DeeObject_Init(&result->q_iter, &DeeQueryIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_seq query_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&query_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))NULL,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))NULL,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_get = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))NULL,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))NULL
};

INTERN DeeTypeObject DeeQuery_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Query_tp_name,
	/* .tp_doc      = */ S_Query_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeDir_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(Dir)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &query_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ query_class_members
};

DECL_END

#endif /* !GUARD_DEX_FS_UNIX_C_INL */
