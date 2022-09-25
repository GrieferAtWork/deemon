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
#ifndef GUARD_DEX_POSIX_LIBPOSIX_H
#define GUARD_DEX_POSIX_LIBPOSIX_H 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/dex.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/sched/yield.h>

#include <stdbool.h>

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

/* Imported module access. */
#define FS_MODULE DEX.d_imports[0]


#if defined(EINTR) && !defined(__INTELLISENSE__)
#define EINTR_LABEL(again) \
	again:
#define HANDLE_EINTR(error, again, err_label) \
	if ((error) == EINTR) {                   \
		if (DeeThread_CheckInterrupt())       \
			goto err_label;                   \
		goto again;                           \
	}
#else
#define EINTR_LABEL(again)                    /* nothing */
#define HANDLE_EINTR(error, again, err_label) /* nothing */
#endif

#define HANDLE_ENOENT(error, err_label, ...)                                   \
	DeeSystem_IF_E1(error, ENOENT, {                                           \
		DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, error, __VA_ARGS__); \
		goto err_label;                                                        \
	});
#define HANDLE_ENOTDIR(error, err_label, ...)                                         \
	DeeSystem_IF_E1(error, ENOTDIR, {                                                 \
		DREF DeeTypeObject *tp;                                                       \
		tp = (DREF DeeTypeObject *)DeeObject_GetAttrString(FS_MODULE, "NoDirectory"); \
		if (tp) {                                                                     \
			DeeUnixSystem_ThrowErrorf(tp, error, __VA_ARGS__);                        \
			Dee_Decref(tp);                                                           \
		}                                                                             \
		goto err_label;                                                               \
	});
#define HANDLE_ENOENT_ENOTDIR(error, err_label, ...)                           \
	DeeSystem_IF_E2(error, ENOENT, ENOTDIR, {                                  \
		DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, error, __VA_ARGS__); \
		goto err_label;                                                        \
	});
#define HANDLE_ENXIO_EISDIR(error, err_label, ...)                             \
	DeeSystem_IF_E2(error, ENXIO, EISDIR, {                                    \
		DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, error, __VA_ARGS__); \
		goto err_label;                                                        \
	});
#define HANDLE_EROFS_ETXTBSY(error, err_label, ...)                            \
	DeeSystem_IF_E2(error, EROFS, ETXTBSY, {                                   \
		DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, error, __VA_ARGS__); \
		goto err_label;                                                        \
	});
#define HANDLE_EACCES(error, err_label, ...)                                      \
	DeeSystem_IF_E1(error, EACCES, {                                              \
		DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error, __VA_ARGS__); \
		goto err_label;                                                           \
	});
#define HANDLE_EEXIST_IF(error, cond, err_label, ...)                                 \
	DeeSystem_IF_E1(error, EEXIST, {                                                  \
		if (cond) {                                                                   \
			DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error, __VA_ARGS__); \
			goto err_label;                                                           \
		}                                                                             \
	});
#define HANDLE_EINVAL(error, err_label, ...)                \
	DeeSystem_IF_E1(error, EINVAL, {                        \
		DeeError_Throwf(&DeeError_ValueError, __VA_ARGS__); \
		goto err_label;                                     \
	});
#define HANDLE_ENOMEM(error, err_label, ...)              \
	DeeSystem_IF_E1(error, ENOMEM, {                      \
		DeeError_Throwf(&DeeError_NoMemory, __VA_ARGS__); \
		goto err_label;                                   \
	});
#define HANDLE_EBADF(error, err_label, ...)                 \
	DeeSystem_IF_E1(error, EBADF, {                         \
		DeeError_Throwf(&DeeError_FileClosed, __VA_ARGS__); \
		goto err_label;                                     \
	});
#define HANDLE_EBADF_ENOENT(error, err_label, ...)          \
	DeeSystem_IF_E2(error, EBADF, ENOENT, {                 \
		DeeError_Throwf(&DeeError_FileClosed, __VA_ARGS__); \
		goto err_label;                                     \
	});
#define HANDLE_EFBIG_EINVAL(error, err_label, ...)               \
	DeeSystem_IF_E2(error, EFBIG, EINVAL, {                      \
		DeeError_Throwf(&DeeError_IntegerOverflow, __VA_ARGS__); \
		goto err_label;                                          \
	});
#define HANDLE_ENOSYS(error, err_label, name)             \
	DeeSystem_IF_E3(error, ENOSYS, ENOTSUP, EOPNOTSUPP, { \
		posix_err_unsupported(name);                      \
		goto err_label;                                   \
	});


INTDEF ATTR_NOINLINE ATTR_UNUSED ATTR_COLD int DCALL
posix_err_unsupported(char const *__restrict name);
#undef NEED_posix_err_unsupported

INTDEF WUNUSED DREF /*String*/ DeeObject *DCALL
libposix_get_dfd_filename(int dfd, /*utf-8*/ char const *filename, int atflags);
#undef NEED_libposix_get_dfd_filename

#if defined(ENOSYS) || defined(ENOTSUP) || defined(EOPNOTSUPP)
#define NEED_posix_err_unsupported
#endif /* ENOSYS || ENOTSUP || EOPNOTSUPP */

/* STAT bitflags. */

/* (Mostly) standard UNIX stat flag bits. (We try to mirror these on NT) */
#ifdef CONFIG_HOST_WINDOWS
#define STAT_IFMT   0170000 /* These bits determine file type. */
#define STAT_IFDIR  0040000 /* Directory. */
#define STAT_IFCHR  0020000 /* Character device. */
#define STAT_IFBLK  0060000 /* Block device. */
#define STAT_IFREG  0100000 /* Regular file. */
#define STAT_IFIFO  0010000 /* FIFO. */
#define STAT_IFLNK  0120000 /* Symbolic link. */
#define STAT_IFSOCK 0140000 /* Socket. */
#define STAT_ISUID  0004000 /* Set user ID on execution. */
#define STAT_ISGID  0002000 /* Set group ID on execution. */
#define STAT_ISVTX  0001000 /* Save swapped text after use (sticky). */
#else /* CONFIG_HOST_WINDOWS */
#ifdef S_IFMT
#define STAT_IFMT S_IFMT
#elif defined(__S_IFMT)
#define STAT_IFMT __S_IFMT
#elif defined(_S_IFMT)
#define STAT_IFMT _S_IFMT
#elif defined(_IFMT)
#define STAT_IFMT _IFMT
#else /* ... */
#define STAT_IFMT 0170000
#endif /* !... */
#ifdef S_IFDIR
#define STAT_IFDIR S_IFDIR
#elif defined(__S_IFDIR)
#define STAT_IFDIR __S_IFDIR
#elif defined(_S_IFDIR)
#define STAT_IFDIR _S_IFDIR
#elif defined(_IFDIR)
#define STAT_IFDIR _IFDIR
#else /* ... */
#define STAT_IFDIR 0040000 /* Directory. */
#endif /* !... */
#ifdef S_IFCHR
#define STAT_IFCHR S_IFCHR
#elif defined(__S_IFCHR)
#define STAT_IFCHR __S_IFCHR
#elif defined(_S_IFCHR)
#define STAT_IFCHR _S_IFCHR
#elif defined(_IFCHR)
#define STAT_IFCHR _IFCHR
#else /* ... */
#define STAT_IFCHR 0020000 /* Character device. */
#endif /* !... */
#ifdef S_IFBLK
#define STAT_IFBLK S_IFBLK
#elif defined(__S_IFBLK)
#define STAT_IFBLK __S_IFBLK
#elif defined(_S_IFBLK)
#define STAT_IFBLK _S_IFBLK
#elif defined(_IFBLK)
#define STAT_IFBLK _IFBLK
#else /* ... */
#define STAT_IFBLK 0060000 /* Block device. */
#endif /* !... */
#ifdef S_IFREG
#define STAT_IFREG S_IFREG
#elif defined(__S_IFREG)
#define STAT_IFREG __S_IFREG
#elif defined(_S_IFREG)
#define STAT_IFREG _S_IFREG
#elif defined(_IFREG)
#define STAT_IFREG _IFREG
#else /* ... */
#define STAT_IFREG 0100000 /* Regular file. */
#endif /* !... */
#ifdef S_IFIFO
#define STAT_IFIFO S_IFIFO
#elif defined(__S_IFIFO)
#define STAT_IFIFO __S_IFIFO
#elif defined(_S_IFIFO)
#define STAT_IFIFO _S_IFIFO
#elif defined(_IFIFO)
#define STAT_IFIFO _IFIFO
#else /* ... */
#define STAT_IFIFO 0010000 /* FIFO. */
#endif /* !... */
#ifdef S_IFLNK
#define STAT_IFLNK S_IFLNK
#elif defined(__S_IFLNK)
#define STAT_IFLNK __S_IFLNK
#elif defined(_S_IFLNK)
#define STAT_IFLNK _S_IFLNK
#elif defined(_IFLNK)
#define STAT_IFLNK _IFLNK
#else /* ... */
#define STAT_IFLNK 0120000 /* Symbolic link. */
#endif /* !... */
#ifdef S_IFSOCK
#define STAT_IFSOCK S_IFSOC
#elif defined(__S_IFSOC)
#define STAT_IFSOCK __S_IFSOC
#elif defined(_S_IFSOC)
#define STAT_IFSOCK _S_IFSOC
#elif defined(_IFSOC)
#define STAT_IFSOCK _IFSOC
#else /* ... */
#define STAT_IFSOCK 0140000 /* Socket. */
#endif /* !... */
#ifdef S_ISUID
#define STAT_ISUID S_ISUID
#elif defined(__S_ISUID)
#define STAT_ISUID __S_ISUID
#elif defined(_S_ISUID)
#define STAT_ISUID _S_ISUID
#elif defined(_ISUID)
#define STAT_ISUID _ISUID
#else /* ... */
#define STAT_ISUID 0004000 /* Set user ID on execution. */
#endif /* !... */
#ifdef S_ISGID
#define STAT_ISGID S_ISGID
#elif defined(__S_ISGID)
#define STAT_ISGID __S_ISGID
#elif defined(_S_ISGID)
#define STAT_ISGID _S_ISGID
#elif defined(_ISGID)
#define STAT_ISGID _ISGID
#else /* ... */
#define STAT_ISGID 0002000 /* Set group ID on execution. */
#endif /* !... */
#ifdef S_ISVTX
#define STAT_ISVTX S_ISVTX
#elif defined(__S_ISVTX)
#define STAT_ISVTX __S_ISVTX
#elif defined(_S_ISVTX)
#define STAT_ISVTX _S_ISVTX
#elif defined(_ISVTX)
#define STAT_ISVTX _ISVTX
#else /* ... */
#define STAT_ISVTX 0001000 /* Save swapped text after use (sticky). */
#endif /* !... */
#endif /* !CONFIG_HOST_WINDOWS */

#ifdef S_ISDIR
#define STAT_ISDIR(mode) S_ISDIR(mode)
#elif defined(__S_ISDIR)
#define STAT_ISDIR(mode) __S_ISDIR(mode)
#else /* S_ISDIR */
#define STAT_ISDIR(mode) (((mode) & STAT_IFMT) == STAT_IFDIR)
#endif /* !S_ISDIR */

#ifdef S_ISCHR
#define STAT_ISCHR(mode) S_ISCHR(mode)
#elif defined(__S_ISCHR)
#define STAT_ISCHR(mode) __S_ISCHR(mode)
#else /* S_ISCHR */
#define STAT_ISCHR(mode) (((mode) & STAT_IFMT) == STAT_IFCHR)
#endif /* !S_ISCHR */

#ifdef S_ISBLK
#define STAT_ISBLK(mode) S_ISBLK(mode)
#elif defined(__S_ISBLK)
#define STAT_ISBLK(mode) __S_ISBLK(mode)
#else /* S_ISBLK */
#define STAT_ISBLK(mode) (((mode) & STAT_IFMT) == STAT_IFBLK)
#endif /* !S_ISBLK */

#ifdef S_ISDEV
#define STAT_ISDEV(mode) S_ISDEV(mode)
#elif defined(__S_ISDEV)
#define STAT_ISDEV(mode) __S_ISDEV(mode)
#else /* S_ISDEV */
#define STAT_ISDEV(mode) (STAT_ISCHR(mode) || STAT_ISBLK(mode))
#endif /* !S_ISDEV */

#ifdef S_ISREG
#define STAT_ISREG(mode) S_ISREG(mode)
#elif defined(__S_ISREG)
#define STAT_ISREG(mode) __S_ISREG(mode)
#else /* S_ISREG */
#define STAT_ISREG(mode) (((mode) & STAT_IFMT) == STAT_IFREG)
#endif /* !S_ISREG */

#ifdef S_ISFIFO
#define STAT_ISFIFO(mode) S_ISFIFO(mode)
#elif defined(__S_ISFIFO)
#define STAT_ISFIFO(mode) __S_ISFIFO(mode)
#else /* S_ISFIFO */
#define STAT_ISFIFO(mode) (((mode) & STAT_IFMT) == STAT_IFIFO)
#endif /* !S_ISFIFO */

#ifdef S_ISLNK
#define STAT_ISLNK(mode) S_ISLNK(mode)
#elif defined(__S_ISLNK)
#define STAT_ISLNK(mode) __S_ISLNK(mode)
#else /* S_ISLNK */
#define STAT_ISLNK(mode) (((mode) & STAT_IFMT) == STAT_IFLNK)
#endif /* !S_ISLNK */

#ifdef S_ISSOCK
#define STAT_ISSOCK(mode) S_ISSOCK(mode)
#elif defined(__S_ISSOCK)
#define STAT_ISSOCK(mode) __S_ISSOCK(mode)
#else /* S_ISSOCK */
#define STAT_ISSOCK(mode) (((mode) & STAT_IFMT) == STAT_IFSOCK)
#endif /* !S_ISSOCK */

#define STAT_IRUSR 0000400           /* Read by owner. */
#define STAT_IWUSR 0000200           /* Write by owner. */
#define STAT_IXUSR 0000100           /* Execute by owner. */
#define STAT_IRGRP (STAT_IRUSR >> 3) /* Read by group. */
#define STAT_IWGRP (STAT_IWUSR >> 3) /* Write by group. */
#define STAT_IXGRP (STAT_IXUSR >> 3) /* Execute by group. */
#define STAT_IROTH (STAT_IRUSR >> 6) /* Read by other. */
#define STAT_IWOTH (STAT_IWUSR >> 6) /* Write by other. */
#define STAT_IXOTH (STAT_IXUSR >> 6) /* Execute by other. */

/* If the hosting operating system doesn't provide it,
 * we still need a value for `AT_SYMLINK_NOFOLLOW' that
 * we can then use in requests. */
#ifndef CONFIG_HAVE_AT_SYMLINK_NOFOLLOW
#undef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x0100
#endif /* CONFIG_HAVE_AT_SYMLINK_NOFOLLOW */

/* Windows doesn't natively have an `AT_FDCWD', but we
 * need something to check for in `posix_dfd_abspath()' */
#ifdef CONFIG_HOST_WINDOWS
#undef AT_FDCWD
#define AT_FDCWD (-1)
#endif /* CONFIG_HOST_WINDOWS */



/************************************************************************/
/* environ functions                                                    */
/************************************************************************/

/* @return:  1: Exists
 * @return:  0: Doesn't exist
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
posix_environ_hasenv(DeeStringObject *__restrict name);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_environ_getenv(DeeStringObject *name, DeeObject *defl);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
posix_environ_setenv(DeeStringObject *name, DeeStringObject *value, bool replace);

/* @return:  0: Success
 * @return:  1: Not deleted because never defined
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
posix_environ_unsetenv(DeeStringObject *__restrict name);

PRIVATE WUNUSED int DCALL
posix_environ_clearenv(void);

PRIVATE WUNUSED size_t DCALL
posix_environ_getcount(void);

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_unknown_env_var(DeeObject *__restrict name);
/************************************************************************/



/************************************************************************/
/* ON-demand helper functions                                           */
/************************************************************************/

/* Construct an absolute path from `dfd:path'
 * @param: dfd:  Can be a `File', `int', `string', or [nt:`HANDLE']
 * @param: path: Must be a `string' */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
posix_dfd_abspath(DeeObject *dfd, DeeObject *path);

INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_no_access(int error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_no_dir(int error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_not_found(int error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_handle_closed(int error, DeeObject *__restrict path);

#ifdef CONFIG_HOST_WINDOWS
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_no_dir(DWORD error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_not_found(DWORD error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_no_access(DWORD error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_chattr_no_access(DWORD error, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_handle_closed(DWORD error, DeeObject *__restrict path);

INTDEF WUNUSED DREF DeeObject *DCALL nt_GetTempPath(void);

/* Work around a problem with long path names.
 * @return:  0: Successfully changed working directories.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL nt_SetCurrentDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_GetFileAttributesEx(DeeObject *__restrict lpFileName,
                       GET_FILEEX_INFO_LEVELS fInfoLevelId,
                       LPVOID lpFileInformation);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_GetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD *__restrict presult);

/* Work around a problem with long path names.
 * @return:  0: Successfully set attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_SetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD dwFileAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the new directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_CreateDirectory(DeeObject *__restrict lpPathName,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_RemoveDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_DeleteFile(DeeObject *__restrict lpFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_MoveFile(DeeObject *__restrict lpExistingFileName,
            DeeObject *__restrict lpNewFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the hardlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_CreateHardLink(DeeObject *__restrict lpFileName,
                  DeeObject *__restrict lpExistingFileName,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the symlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (See GetLastError()) */
INTDEF int DCALL
nt_CreateSymbolicLink(DeeObject *__restrict lpSymlinkFileName,
                      DeeObject *__restrict lpTargetFileName,
                      DWORD dwFlags);

#endif /* CONFIG_HOST_WINDOWS */
/************************************************************************/

DECL_END

#endif /* !GUARD_DEX_POSIX_LIBPOSIX_H */
