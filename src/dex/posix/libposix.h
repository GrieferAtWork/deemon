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
#include <aclapi.h>
#endif /* CONFIG_HOST_WINDOWS */


 /* MSVC thought it smart to rename `st_birthtime' to `st_ctime' */
#undef CONFIG_STAT_CTIME_IS_ACTUALLY_BIRTHTIME
#if (defined(_WIN32) || defined(__WIN32__)) && !defined(__CYGWIN__)
#define CONFIG_STAT_CTIME_IS_ACTUALLY_BIRTHTIME
#endif /* ... */

#define DEFAULT_BLOCKSIZE 512
#define DEFAULT_BLOCKS_FROM_FILESIZE(fz) \
	(((fz) + DEFAULT_BLOCKSIZE - 1) / DEFAULT_BLOCKSIZE)



#undef OPT_close
#ifdef CONFIG_HAVE_close
#define OPT_close(fd) (void)close(fd)
#else /* CONFIG_HAVE_close */
#define OPT_close(fd) (void)0
#endif /* !CONFIG_HAVE_close */


/* Check for special case where we roll our own `open(2)' function */
#undef posix_open_USE_open_osfhandle__AND__CreateFile
#if defined(CONFIG_HAVE_open_osfhandle) && defined(CONFIG_HOST_WINDOWS)
/* On windows, must use open_osfhandle(CreateFile()), else our
 * custom `STDIN$', `STDOUT$', `STDERR$' magic won't work! */
#define posix_open_USE_open_osfhandle__AND__CreateFile
#endif /* !... */

#if defined(posix_open_USE_open_osfhandle__AND__CreateFile) || defined(__DEEMON__)
/* Use our own set of O_* flags! */
#undef CONFIG_HAVE_O_RDONLY
#undef CONFIG_HAVE_O_WRONLY
#undef CONFIG_HAVE_O_RDWR
#undef CONFIG_HAVE_O_APPEND
#undef CONFIG_HAVE_O_CREAT
#undef CONFIG_HAVE_O_TRUNC
#undef CONFIG_HAVE_O_EXCL
#undef CONFIG_HAVE_O_TEXT
#undef CONFIG_HAVE_O_BINARY
#undef CONFIG_HAVE_O_WTEXT
#undef CONFIG_HAVE_O_U16TEXT
#undef CONFIG_HAVE_O_U8TEXT
#undef CONFIG_HAVE_O_CLOEXEC
#undef CONFIG_HAVE_O_TEMPORARY
#undef CONFIG_HAVE_O_SHORT_LIVED
#undef CONFIG_HAVE_O_OBTAIN_DIR
#undef CONFIG_HAVE_O_SEQUENTIAL
#undef CONFIG_HAVE_O_RANDOM
#undef CONFIG_HAVE_O_NOCTTY
#undef CONFIG_HAVE_O_NONBLOCK
#undef CONFIG_HAVE_O_SYNC
#undef CONFIG_HAVE_O_RSYNC
#undef CONFIG_HAVE_O_DSYNC
#undef CONFIG_HAVE_O_ASYNC
#undef CONFIG_HAVE_O_DIRECT
#undef CONFIG_HAVE_O_LARGEFILE
#undef CONFIG_HAVE_O_DIRECTORY
#undef CONFIG_HAVE_O_NOFOLLOW
#undef CONFIG_HAVE_O_NOATIME
#undef CONFIG_HAVE_O_PATH
#undef CONFIG_HAVE_O_TMPFILE
#undef CONFIG_HAVE_O_CLOFORK
#undef CONFIG_HAVE_O_SYMLINK
#undef CONFIG_HAVE_O_DOSPATH
#undef CONFIG_HAVE_O_SHLOCK
#undef CONFIG_HAVE_O_EXLOCK
#undef CONFIG_HAVE_O_XATTR
#undef CONFIG_HAVE_O_EXEC
#undef CONFIG_HAVE_O_SEARCH
#undef CONFIG_HAVE_O_TTY_INIT
#undef CONFIG_HAVE_O_NOLINKS

/*[[[deemon
function feature(a, b) {
	print("#undef ", a);
	print("#define CONFIG_HAVE_", a);
	print("#define ", a, " ", b);
}
feature("O_RDONLY",   "Dee_OPEN_FRDONLY");
feature("O_WRONLY",   "Dee_OPEN_FWRONLY");
feature("O_RDWR",     "Dee_OPEN_FRDWR");
feature("O_ACCMODE",  "Dee_OPEN_FACCMODE");
feature("O_CREAT",    "Dee_OPEN_FCREAT");
feature("O_EXCL",     "Dee_OPEN_FEXCL");
feature("O_TRUNC",    "Dee_OPEN_FTRUNC");
feature("O_NONBLOCK", "Dee_OPEN_FNONBLOCK");
feature("O_SYNC",     "Dee_OPEN_FSYNC");
feature("O_DIRECT",   "Dee_OPEN_FDIRECT");
feature("O_NOFOLLOW", "Dee_OPEN_FNOFOLLOW");
feature("O_NOATIME",  "Dee_OPEN_FNOATIME");
feature("O_CLOEXEC",  "Dee_OPEN_FCLOEXEC");
feature("O_SHLOCK",   "Dee_OPEN_FXREAD");
feature("O_EXLOCK",   "Dee_OPEN_FXWRITE");
feature("O_HIDDEN",   "Dee_OPEN_FHIDDEN");
]]]*/
#undef O_RDONLY
#define CONFIG_HAVE_O_RDONLY
#define O_RDONLY Dee_OPEN_FRDONLY
#undef O_WRONLY
#define CONFIG_HAVE_O_WRONLY
#define O_WRONLY Dee_OPEN_FWRONLY
#undef O_RDWR
#define CONFIG_HAVE_O_RDWR
#define O_RDWR Dee_OPEN_FRDWR
#undef O_ACCMODE
#define CONFIG_HAVE_O_ACCMODE
#define O_ACCMODE Dee_OPEN_FACCMODE
#undef O_CREAT
#define CONFIG_HAVE_O_CREAT
#define O_CREAT Dee_OPEN_FCREAT
#undef O_EXCL
#define CONFIG_HAVE_O_EXCL
#define O_EXCL Dee_OPEN_FEXCL
#undef O_TRUNC
#define CONFIG_HAVE_O_TRUNC
#define O_TRUNC Dee_OPEN_FTRUNC
#undef O_NONBLOCK
#define CONFIG_HAVE_O_NONBLOCK
#define O_NONBLOCK Dee_OPEN_FNONBLOCK
#undef O_SYNC
#define CONFIG_HAVE_O_SYNC
#define O_SYNC Dee_OPEN_FSYNC
#undef O_DIRECT
#define CONFIG_HAVE_O_DIRECT
#define O_DIRECT Dee_OPEN_FDIRECT
#undef O_NOFOLLOW
#define CONFIG_HAVE_O_NOFOLLOW
#define O_NOFOLLOW Dee_OPEN_FNOFOLLOW
#undef O_NOATIME
#define CONFIG_HAVE_O_NOATIME
#define O_NOATIME Dee_OPEN_FNOATIME
#undef O_CLOEXEC
#define CONFIG_HAVE_O_CLOEXEC
#define O_CLOEXEC Dee_OPEN_FCLOEXEC
#undef O_SHLOCK
#define CONFIG_HAVE_O_SHLOCK
#define O_SHLOCK Dee_OPEN_FXREAD
#undef O_EXLOCK
#define CONFIG_HAVE_O_EXLOCK
#define O_EXLOCK Dee_OPEN_FXWRITE
#undef O_HIDDEN
#define CONFIG_HAVE_O_HIDDEN
#define O_HIDDEN Dee_OPEN_FHIDDEN
/*[[[end]]]*/

/* This one needs support from `open_osfhandle()' */
#ifdef _O_APPEND
#undef O_APPEND
#define CONFIG_HAVE_O_APPEND
#define O_APPEND Dee_OPEN_FAPPEND
#endif /* _O_APPEND */
#endif /* posix_open_USE_open_osfhandle__AND__CreateFile */


DECL_BEGIN

#ifdef EINTR
#define EINTR_LABEL(again) \
	again:
#define EINTR_HANDLE(error, again, err_label) \
	if ((error) == EINTR) {                   \
		if (DeeThread_CheckInterrupt())       \
			goto err_label;                   \
		goto again;                           \
	}
#else /* EINTR */
#define EINTR_LABEL(again)                    /* nothing */
#define EINTR_HANDLE(error, again, err_label) /* nothing */
#endif /* !EINTR */

#ifdef ENOMEM
#define ENOMEM_LABEL(again) \
	again:
#define ENOMEM_HANDLE(error, again, err_label) \
	if ((error) == ENOMEM) {                   \
		if (Dee_CollectMemory(1))              \
			goto again;                        \
		goto err_label;                        \
	}
#else /* ENOMEM */
#define ENOMEM_LABEL(again)                    /* nothing */
#define ENOMEM_HANDLE(error, again, err_label) /* nothing */
#endif /* !ENOMEM */

#if defined(EINTR) || defined(ENOMEM)
#define EINTR_ENOMEM_LABEL(again) again:
#else /* EINTR || ENOMEM */
#define EINTR_ENOMEM_LABEL(again) /* nothing */
#endif /* !EINTR && !ENOMEM */
#define EINTR_ENOMEM_HANDLE(error, again, err_label) \
	EINTR_HANDLE(error, again, err_label)            \
	ENOMEM_HANDLE(error, again, err_label)



/* TODO: Remove all of the following (replaced with `err_unix_*') */
#define HANDLE_ENOENT(error, err_label, ...)                                   \
	DeeSystem_IF_E1(error, ENOENT, {                                           \
		DeeUnixSystem_ThrowErrorf(&DeeError_FileNotFound, error, __VA_ARGS__); \
		goto err_label;                                                        \
	});
#define HANDLE_ENOTDIR(error, err_label, ...)                                 \
	DeeSystem_IF_E1(error, ENOTDIR, {                                         \
		DeeUnixSystem_ThrowErrorf(&DeeError_NoDirectory, error, __VA_ARGS__); \
		goto err_label;                                                       \
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
#define HANDLE_EEXIST_IF(error, cond, err_label, ...)                            \
	DeeSystem_IF_E1(error, EEXIST, {                                             \
		if (cond) {                                                              \
			DeeUnixSystem_ThrowErrorf(&DeeError_FileExists, error, __VA_ARGS__); \
			goto err_label;                                                      \
		}                                                                        \
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
#define STAT_IFSOCK S_IFSOCK
#elif defined(__S_IFSOCK)
#define STAT_IFSOCK __S_IFSOCK
#elif defined(_S_IFSOCK)
#define STAT_IFSOCK _S_IFSOCK
#elif defined(_IFSOCK)
#define STAT_IFSOCK _IFSOCK
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

/* Windows doesn't natively have an `AT_FDCWD', but we
 * need something to check for in `posix_dfd_makepath()' */
#ifndef CONFIG_HAVE_AT_FDCWD
#undef AT_FDCWD
#define AT_FDCWD (-1)
#endif /* !CONFIG_HAVE_AT_FDCWD */

/* If the hosting operating system doesn't provide it,
 * we still need a value for `AT_SYMLINK_NOFOLLOW' that
 * we can then use in requests. */
#ifndef CONFIG_HAVE_AT_SYMLINK_NOFOLLOW
#undef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x0100
#endif /* CONFIG_HAVE_AT_SYMLINK_NOFOLLOW */

#ifndef CONFIG_HAVE_AT_REMOVEDIR
#undef AT_REMOVEDIR
#define AT_REMOVEDIR 0x10000000
#endif /* !CONFIG_HAVE_AT_REMOVEDIR */

#ifndef CONFIG_HAVE_AT_REMOVEREG
#undef AT_REMOVEREG
#define AT_REMOVEREG 0x20000000
#endif /* !CONFIG_HAVE_AT_REMOVEREG */

#ifndef CONFIG_HAVE_AT_EMPTY_PATH
#undef AT_EMPTY_PATH
#define AT_EMPTY_PATH 0x40000000
#endif /* !CONFIG_HAVE_AT_EMPTY_PATH */

#ifndef CONFIG_HAVE_RENAME_NOREPLACE
#undef RENAME_NOREPLACE
#define RENAME_NOREPLACE 0x80000000
#endif /* !CONFIG_HAVE_RENAME_NOREPLACE */

/* Figure out if the host supports uid_t/gid_t */
#undef CONFIG_HAVE_uid_t
#undef CONFIG_HAVE_gid_t
#if (defined(CONFIG_HAVE_chown) ||   \
     defined(CONFIG_HAVE__chown) ||  \
     defined(CONFIG_HAVE_wchown) ||  \
     defined(CONFIG_HAVE__wchown) || \
     defined(CONFIG_HAVE_lchown) ||  \
     defined(CONFIG_HAVE_fchown) ||  \
     defined(CONFIG_HAVE_fchownat))
#define CONFIG_HAVE_uid_t
#define CONFIG_HAVE_gid_t
#endif /* ... */

#ifndef CONFIG_HAVE_uid_t
#undef uid_t
#define uid_t uint32_t
#endif /* !CONFIG_HAVE_uid_t */

#ifndef CONFIG_HAVE_gid_t
#undef gid_t
#define gid_t uint32_t
#endif /* !CONFIG_HAVE_gid_t */



/************************************************************************/
/* environ functions                                                    */
/************************************************************************/

/* @return:  1: Exists
 * @return:  0: Doesn't exist
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
posix_environ_hasenv(DeeStringObject *__restrict name);

/* Caller must call: `err_unknown_env_var((DeeObject *)name);'  */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_environ_trygetenv(DeeStringObject *name);

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

/* Construct a path from `dfd:path'
 * @param: dfd:  Can be a `File', `int', `string', or [nt:`HANDLE']
 * @param: path: Must be a `string' */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
posix_dfd_makepath(DeeObject *dfd, DeeObject *path, unsigned int atflags);
#define POSIX_DFD_MAKEPATH_ATFLAGS_MASK 0 /* Bitset atflags supported by `posix_dfd_makepath' */
#define POSIX_DFD_MAKEPATH_ATFLAGS_FROM_OFLAGS(x) (0)

/* Construct a path that refers to the file described by `fd'
 * @param: fd: Can be a `File', `int', or [nt:`HANDLE'] */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
posix_fd_makepath(DeeObject *__restrict fd);

/* Construct a path that refers to the file described by `os_fd' */
INTDEF WUNUSED DREF DeeObject *DCALL
posix_fd_makepath_fd(int os_fd);

/* Open a HANDLE/fd-compatible object as a `File' */
INTDEF WUNUSED NONNULL((1)) /*File*/ DREF DeeObject *DCALL
posix_fd_openfile(DeeObject *__restrict fd, int oflags);

/* Copy all data from `src' to `dst', both of with are deemon File objects.
 * @param: src_mmap_hints: Set of `0 | DEE_MAPFILE_F_ATSTART'
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
posix_copyfile_fileio(/*File*/ DeeObject *src,
                      /*File*/ DeeObject *dst,
                      DeeObject *progress,
                      DeeObject *bufsize,
                      unsigned int src_mmap_hints);

/* Parse a `chmod(1)'-style mode-string, or convert `mode' into an integer
 * @return: * : The new st_mode to-be used for the file.
 * @return: (unsigned int)-1: An error was thrown. */
INTDEF WUNUSED NONNULL((1, 2)) unsigned int DCALL posix_chmod_getmode(DeeObject *path, DeeObject *mode);
INTDEF WUNUSED NONNULL((1, 2)) unsigned int DCALL posix_lchmod_getmode(DeeObject *path, DeeObject *mode);

/* Parse a chmod(1)-style mode-string
 * @return: * : The new file-mode
 * @return: (unsigned int)-1: Error
 * @return: (unsigned int)-2: The given `st_mode == (unsigned int)-1', but would be needed. */
INTDEF WUNUSED NONNULL((1)) unsigned int DCALL
posix_chmod_parsemode(char const *__restrict mode_str,
                      unsigned int st_mode);

/* Get the current file-mode for the given argument. */
INTDEF WUNUSED NONNULL((1)) unsigned int DCALL posix_stat_getmode(DeeObject *__restrict path);
INTDEF WUNUSED NONNULL((1)) unsigned int DCALL posix_lstat_getmode(DeeObject *__restrict path);
INTDEF WUNUSED NONNULL((1)) unsigned int DCALL posix_xstat_getmode(DeeObject *__restrict path_or_fd, unsigned int stat_flags);

/* Parse a UID/GID from a given deemon object (with can either be an integer, or a string)
 * @return: 0 : Success
 * @return: -1: Error */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
posix_chown_unix_parseuid(DeeObject *__restrict uid,
                          uid_t *__restrict p_result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
posix_chown_unix_parsegid(DeeObject *__restrict gid,
                          gid_t *__restrict p_result);


/* For utime() implementations using `utime(2)' */
struct utimbuf;
struct utimbuf32;
struct utimbuf64;
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 6)) int DCALL
posix_utime_unix_parse_utimbuf(struct utimbuf *__restrict p_result,
                               DeeObject *atime, DeeObject *mtime,
                               DeeObject *ctime, DeeObject *birthtime,
                               DeeObject *path_or_fd,
                               unsigned int stat_flags);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 6)) int DCALL
posix_utime_unix_parse_utimbuf32(struct utimbuf32 *__restrict p_result,
                                 DeeObject *atime, DeeObject *mtime,
                                 DeeObject *ctime, DeeObject *birthtime,
                                 DeeObject *path_or_fd,
                                 unsigned int stat_flags);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 6)) int DCALL
posix_utime_unix_parse_utimbuf64(struct utimbuf64 *__restrict p_result,
                                 DeeObject *atime, DeeObject *mtime,
                                 DeeObject *ctime, DeeObject *birthtime,
                                 DeeObject *path_or_fd,
                                 unsigned int stat_flags);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL
posix_utime_unix_parse_utimbuf_common(int64_t *__restrict p_actime,
                                      int64_t *__restrict p_modtime,
                                      DeeObject *atime, DeeObject *mtime,
                                      DeeObject *path_or_fd,
                                      unsigned int stat_flags);

/* For utime() implementations using `utimes(2)' or `utimens(2)' */
#ifdef CONFIG_HAVE_AT_CHANGE_BTIME
#define POSIX_UTIME_TIMESPEC_COUNT 3
#else /* CONFIG_HAVE_AT_CHANGE_BTIME */
#define POSIX_UTIME_TIMESPEC_COUNT 2
#endif /* !CONFIG_HAVE_AT_CHANGE_BTIME */
#define POSIX_UTIME_TIMEVAL_COUNT 2

struct timeval;
struct timeval64;
struct timespec;
struct timespec64;
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
posix_utime_unix_parse_timeval(struct timeval *__restrict p_tsv /*[2]*/, DeeObject *atime, DeeObject *mtime,
                               DeeObject *path_or_fd, unsigned int stat_flags);
INTDEF WUNUSED NONNULL((1, 2, 3, 3)) int DCALL
posix_utime_unix_parse_timeval64(struct timeval64 *__restrict p_tsv /*[2]*/,
                                 DeeObject *atime, DeeObject *mtime,
                                 DeeObject *path_or_fd, unsigned int stat_flags);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
posix_utime_unix_parse_timespec_2(struct timespec *__restrict p_tsv /*[2]*/,
                                  DeeObject *atime, DeeObject *mtime);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL
posix_utime_unix_parse_timespec64_2(struct timespec64 *__restrict p_tsv /*[2]*/,
                                    DeeObject *atime, DeeObject *mtime);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
posix_utime_unix_parse_timespec_3(struct timespec *__restrict p_tsv /*[3]*/,
                                  DeeObject *atime, DeeObject *mtime, DeeObject *birthtime);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
posix_utime_unix_parse_timespec64_3(struct timespec64 *__restrict p_tsv /*[3]*/,
                                    DeeObject *atime, DeeObject *mtime, DeeObject *birthtime);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL posix_utime_unix_object_to_timeval(DeeObject *__restrict self, struct timeval *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL posix_utime_unix_object_to_timeval64(DeeObject *__restrict self, struct timeval64 *__restrict result);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL posix_utime_unix_object_to_timespec(DeeObject *__restrict self, struct timespec *__restrict result); /* Note: also accepts `none' */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL posix_utime_unix_object_to_timespec64(DeeObject *__restrict self, struct timespec64 *__restrict result); /* Note: also accepts `none' */




/* Default buffer size for copyfile */
#ifndef POSIX_COPYFILE_DEFAULT_IO_BUFSIZE
#define POSIX_COPYFILE_DEFAULT_IO_BUFSIZE (64 * 1024) /* 64K */
#endif /* !POSIX_COPYFILE_DEFAULT_IO_BUFSIZE */
#ifndef POSIX_COPYFILE_DEFAULT_SENDFILE_BUFSIZE
#define POSIX_COPYFILE_DEFAULT_SENDFILE_BUFSIZE (16 * 1024 * 1024) /* 64M */
#endif /* !POSIX_COPYFILE_DEFAULT_SENDFILE_BUFSIZE */


/* DTO that is passed to the progress-callback of `copyfile()'
 * The type of this object is exposed as `posix.CopyFileProgress' */
typedef struct {
	OBJECT_HEAD
	DREF DeeObject *cfp_srcfile; /* [1..1][const] Source file */
	DREF DeeObject *cfp_dstfile; /* [1..1][const] Destination file */
	size_t          cfp_bufsize; /* [lock(WEAK(ATOMIC))] Buffer size used during file copy */
	uint64_t        cfp_copied;  /* [lock(WEAK(ATOMIC))] Number of bytes that have been copied */
	uint64_t        cfp_total;   /* [lock(WRITE_ONCE)] Total size of `cfp_srcfile' (or `(uint64_t)-1' if not yet determined) */
} DeeCopyFileProgressObject;

INTDEF DeeTypeObject DeeCopyFileProgress_Type;


INTDEF ATTR_COLD int DCALL err_bad_atflags(unsigned int atflags);
INTDEF ATTR_COLD int DCALL err_bad_copyfile_bufsize_is_zero(void);

INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_chdir(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_remove(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_unlink(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_rmdir(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_mkdir(int errno_value, DeeObject *__restrict path, unsigned int mode);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_rename(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_link(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_symlink(int errno_value, DeeObject *text, DeeObject *path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_truncate(int errno_value, DeeObject *path, DeeObject *length);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_ftruncate(int errno_value, DeeObject *fd, DeeObject *length);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_chmod(int errno_value, DeeObject *path, unsigned int mode);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_lchmod(int errno_value, DeeObject *path, unsigned int mode);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_fchmod(int errno_value, DeeObject *fd, unsigned int mode);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_chown(int errno_value, DeeObject *path, uid_t uid, gid_t gid);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_lchown(int errno_value, DeeObject *path, uid_t uid, gid_t gid);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_fchown(int errno_value, DeeObject *fd, uid_t uid, gid_t gid);
INTDEF ATTR_COLD NONNULL((2, 3, 4, 5, 6)) int DCALL err_unix_utime(int errno_value, DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime);
INTDEF ATTR_COLD NONNULL((2, 3, 4, 5, 6)) int DCALL err_unix_lutime(int errno_value, DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime);
INTDEF ATTR_COLD NONNULL((2, 3, 4, 5, 6)) int DCALL err_unix_futime(int errno_value, DeeObject *fd, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime);
INTDEF ATTR_COLD NONNULL((1, 2, 3)) int DCALL err_utime_cannot_set_ctime_or_btime(DeeObject *path_or_fd, DeeObject *ctime, DeeObject *birthtime);
INTDEF ATTR_COLD NONNULL((1, 2)) int DCALL err_utime_cannot_set_ctime(DeeObject *path_or_fd, DeeObject *ctime);

INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_remove_unsupported(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_unlink_unsupported(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_rmdir_unsupported(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_mkdir_unsupported(int errno_value, DeeObject *__restrict path, unsigned int mode);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_rename_unsupported(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_link_unsupported(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_symlink_unsupported(int errno_value, DeeObject *text, DeeObject *path);

INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_not_dir(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_path_not_dir2(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_not_found(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_path_not_found2(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_no_access(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_path_no_access2(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_chattr_no_access(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_handle_closed(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_exists(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_is_dir(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_readonly(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_path_readonly2(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_file_not_found(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_file_not_writable(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_not_writable(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_busy(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_path_busy2(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_move_to_child(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_path_not_empty(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3, 4, 5, 6)) int DCALL err_unix_utime_no_access(int errno_value, DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *ctime, DeeObject *birthtime);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_path_cross_dev2(int errno_value, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_ftruncate_fbig(int errno_value, DeeObject *fd, DeeObject *length);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_truncate_fbig(int errno_value, DeeObject *path, DeeObject *length);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_ftruncate_isdir(int errno_value, DeeObject *__restrict fd);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_truncate_isdir(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_ftruncate_txtbusy(int errno_value, DeeObject *fd);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_truncate_txtbusy(int errno_value, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_unix_truncate_failed(int errno_value, DeeObject *path, DeeObject *length);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_file_closed(int errno_value, DeeObject *__restrict fd);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_chmod_no_access(int errno_value, DeeObject *__restrict path_or_fd, unsigned int mode);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_unix_chown_no_access(int errno_value, DeeObject *__restrict path_or_fd, uid_t uid, gid_t gid);

/* Missing stat information errors. */
INTDEF ATTR_COLD NONNULL((1)) int DCALL err_stat_no_info(char const *__restrict level);
INTDEF ATTR_COLD int DCALL err_stat_no_mode_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_dev_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_ino_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_link_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_uid_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_gid_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_rdev_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_size_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_blocks_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_blksize_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_atime_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_mtime_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_ctime_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_birthtime_info(void);
INTDEF ATTR_COLD int DCALL err_stat_no_nttype_info(void);
INTDEF ATTR_COLD int DCALL err_integer_overflow(void);

#ifdef CONFIG_HOST_WINDOWS
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_unlink(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_rmdir(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_mkdir(DWORD dwError, DeeObject *__restrict path, unsigned int mode);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_rename(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_link(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_symlink(DWORD dwError, DeeObject *text, DeeObject *path);
INTDEF ATTR_COLD NONNULL((2, 3, 4)) int DCALL err_nt_lchown(DWORD dwError, DeeObject *path, DeeObject *uid, DeeObject *gid);
INTDEF ATTR_COLD NONNULL((2, 3, 4)) int DCALL err_nt_fchown(DWORD dwError, DeeObject *fd, DeeObject *uid, DeeObject *gid);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_unlink_unsupported(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_rmdir_unsupported(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_mkdir_unsupported(DWORD dwError, DeeObject *__restrict path, unsigned int mode);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_rename_unsupported(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_link_unsupported(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_symlink_unsupported(DWORD dwError, DeeObject *text, DeeObject *path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_not_dir(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_path_not_dir2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_not_found(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_path_not_found2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_no_access(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_path_no_access2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_chattr_no_access(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_handle_closed(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_exists(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_is_dir(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_readonly(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_file_not_found(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_file_not_writable(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_not_writable(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_busy(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_path_busy2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_move_to_child(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_not_empty(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3, 4, 5)) int DCALL err_nt_utime_no_access(DWORD dwError, DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *birthtime);
INTDEF ATTR_COLD NONNULL((2, 3)) int DCALL err_nt_path_cross_dev2(DWORD dwError, DeeObject *existing_path, DeeObject *new_path);
INTDEF ATTR_COLD NONNULL((2)) int DCALL err_nt_path_not_link(DWORD dwError, DeeObject *__restrict path);
INTDEF ATTR_COLD NONNULL((2, 3, 4)) int DCALL err_nt_chown_no_access(DWORD dwError, DeeObject *path_or_fd, DeeObject *uid, DeeObject *gid);
INTDEF ATTR_COLD NONNULL((2, 3, 4, 5)) int DCALL err_nt_utime(DWORD dwError, DeeObject *path, DeeObject *atime, DeeObject *mtime, DeeObject *birthtime);
INTDEF ATTR_COLD NONNULL((2, 3, 4, 5)) int DCALL err_nt_futime(DWORD dwError, DeeObject *fd, DeeObject *atime, DeeObject *mtime, DeeObject *birthtime);

/* Helper wrapper around `SetFileTime()' that automatically
 * does all of the necessary conversion of time arguments
 * from nanoseconds-since-01-01-0000 into NT's FILETIME format.
 * @return:  0: Success
 * @return: -1: An error was thrown
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((2, 3, 4)) int DCALL
nt_SetFileTime(HANDLE hFile, DeeObject *atime,
               DeeObject *mtime, DeeObject *birthtime);

typedef struct {
	BYTE Revision;
	BYTE SubAuthorityCount;
	SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
	DWORD SubAuthority[256];
} NT_SID;

#define NT_SID_SIZEOF(SubAuthorityCount) _Dee_MallococBufsize(offsetof(NT_SID, SubAuthority), SubAuthorityCount, sizeof(DWORD))
#define NT_SID_GET_SIZEOF(self)          NT_SID_SIZEOF((self)->SubAuthorityCount)

/* Encode an SID as a deemon integer */
#define nt_EncodeSid(sid) \
	DeeInt_FromBytes(sid, NT_SID_GET_SIZEOF(sid), true, false)

/* Decode a deemon integer into an SID
 * @return: * :        Success (The caller must `Dee_Free()' the returned pointer)
 * @return: ITER_DONE: An error was thrown */
INTDEF WUNUSED NONNULL((1)) NT_SID *DCALL
nt_DecodeSid(/*Int*/ DeeObject *__restrict sid);

/* Similar to `nt_DecodeSid()', but accept more than just strings.
 * @param: argument_is_gid: When false, decode `uid_or_gid' as a UID; else, decode as a GID
 * @return: * :        Success (The caller must `Dee_Free()' the returned pointer)
 * @return: NULL:      Argument was `Dee_None' (NOT AN ERROR)
 * @return: ITER_DONE: An error was thrown */
INTDEF WUNUSED NONNULL((1)) NT_SID *DCALL
nt_QuerySid(DeeObject *__restrict uid_or_gid, bool argument_is_gid);

/* Return an integer for the owner/group SID of a given handle.
 * @return: * :   Integer-representation of SID
 * @return: NULL: Error */
INTDEF WUNUSED DREF DeeObject *DCALL
nt_GetSecurityInfoOwnerSid(HANDLE Handle, SE_OBJECT_TYPE ObjectType);
INTDEF WUNUSED DREF DeeObject *DCALL
nt_GetSecurityInfoGroupSid(HANDLE Handle, SE_OBJECT_TYPE ObjectType);

/* Wrapper around the system function `SetNamedSecurityInfo()'
 * @return: 1 : System error (`*p_dwError' was populated with the error)
 * @return: 0 : Success
 * @return: -1: An error was thrown */
INTDEF WUNUSED NONNULL((1, 8)) int DCALL
nt_SetNamedSecurityInfo(DeeObject *__restrict pObjectName,
                        SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo,
                        NT_SID *psidOwner, NT_SID *psidGroup, PACL pDacl, PACL pSacl,
                        DWORD *__restrict p_dwError);

/* Wrapper around the system function `SetSecurityInfo()'
 * @return: 1 : System error (`*p_dwError' was populated with the error)
 * @return: 0 : Success
 * @return: -1: An error was thrown */
INTDEF WUNUSED NONNULL((8)) int DCALL
nt_SetSecurityInfo(HANDLE handle,
                   SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo,
                   NT_SID const *psidOwner, NT_SID const *psidGroup, PACL pDacl, PACL pSacl,
                   DWORD *__restrict p_dwError);

INTDEF WUNUSED DREF DeeObject *DCALL nt_GetTempPath(void);
INTDEF WUNUSED DREF DeeObject *DCALL nt_GetComputerName(void);

/* Read the contents of a symbolic link
 * @param: path: Only used for error messages
 * @return: * :        Symlink contents
 * @return: NULL:      Error
 * @return: ITER_DONE: Not a symbolic link (only if `throw_error_if_not_a_link == false') */
INTDEF WUNUSED NONNULL((2)) DREF DeeObject *DCALL
nt_FReadLink(HANDLE hLinkFile, DeeObject *__restrict path,
             bool throw_error_if_not_a_link);

/* Work around a problem with long path names.
 * @return:  0: Successfully changed working directories.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_SetCurrentDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_GetFileAttributesEx(DeeObject *__restrict lpFileName,
                       GET_FILEEX_INFO_LEVELS fInfoLevelId,
                       LPVOID lpFileInformation);

/* Work around a problem with long path names.
 * @return:  0: Successfully retrieved attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_GetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD *__restrict p_result);

/* Work around a problem with long path names.
 * @return:  0: Successfully set attributes.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_SetFileAttributes(DeeObject *__restrict lpFileName,
                     DWORD dwFileAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the new directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_CreateDirectory(DeeObject *__restrict lpPathName,
                   LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given directory.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_RemoveDirectory(DeeObject *__restrict lpPathName);

/* Work around a problem with long path names.
 * @return:  0: Successfully removed the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1)) int DCALL
nt_DeleteFile(DeeObject *__restrict lpFileName);

/* Work around a problem with long path names.
 * @return:  0: Successfully moved the given file.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_MoveFileEx(DeeObject *lpExistingFileName,
              DeeObject *lpNewFileName,
              DWORD dwFlags);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the hardlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
nt_CreateHardLink(DeeObject *lpFileName,
                  DeeObject *lpExistingFileName,
                  LPSECURITY_ATTRIBUTES lpSecurityAttributes);

/* Work around a problem with long path names.
 * @return:  0: Successfully created the symlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF NONNULL((1, 2)) int DCALL
nt_CreateSymbolicLink(DeeObject *lpSymlinkFileName,
                      DeeObject *lpTargetFileName,
                      DWORD dwFlags);

/* Same as `nt_CreateSymbolicLink()', but automatically determine proper `dwFlags'
 * @return:  0: Successfully created the symlink.
 * @return: -1: A deemon callback failed and an error was thrown.
 * @return:  1: The system call failed (s.a. `GetLastError()') */
INTDEF NONNULL((1, 2)) int DCALL
nt_CreateSymbolicLinkAuto(DeeObject *lpSymlinkFileName,
                          DeeObject *lpTargetFileName);

/* Try to acquire the named privilege */
INTDEF BOOL DCALL nt_AcquirePrivilege(LPCWSTR lpName);


/* Dynamically loaded functions from `ADVAPI32.dll'
 *
 * We do this because we probably won't actually need these functions, and by
 * loading them lazily, the import of `posix' won't be slowed down by a the
 * dependency on this library. */
typedef BOOL (WINAPI *LPOPENPROCESSTOKEN)(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
typedef BOOL (WINAPI *LPLOOKUPPRIVILEGEVALUEW)(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid);
typedef BOOL (WINAPI *LPADJUSTTOKENPRIVILEGES)(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
typedef DWORD (WINAPI *LPGETSECURITYINFO)(HANDLE Handle, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, NT_SID **ppsidOwner, NT_SID **ppsidGroup, PACL *ppDacl, PACL *ppSacl, PSECURITY_DESCRIPTOR *ppSecurityDescriptor);
typedef DWORD (WINAPI *LPSETSECURITYINFO)(HANDLE handle, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, NT_SID const *psidOwner, NT_SID const *psidGroup, PACL pDacl, PACL pSacl);
typedef DWORD (WINAPI *LPGETNAMEDSECURITYINFOW)(LPCWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, NT_SID **ppsidOwner, NT_SID **ppsidGroup, PACL *ppDacl, PACL *ppSacl, PSECURITY_DESCRIPTOR *ppSecurityDescriptor);
typedef DWORD (WINAPI *LPSETNAMEDSECURITYINFOW)(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, NT_SID const *psidOwner, NT_SID const *psidGroup, PACL pDacl, PACL pSacl);
INTDEF LPOPENPROCESSTOKEN pdyn_OpenProcessToken;
INTDEF LPLOOKUPPRIVILEGEVALUEW pdyn_LookupPrivilegeValueW;
INTDEF LPADJUSTTOKENPRIVILEGES pdyn_AdjustTokenPrivileges;
INTDEF LPGETSECURITYINFO pdyn_GetSecurityInfo;
INTDEF LPSETSECURITYINFO pdyn_SetSecurityInfo;
INTDEF LPGETNAMEDSECURITYINFOW pdyn_GetNamedSecurityInfoW;
INTDEF LPSETNAMEDSECURITYINFOW pdyn_SetNamedSecurityInfoW;
INTDEF void DCALL init_ADVAPI32_dll(void);


#endif /* CONFIG_HOST_WINDOWS */
/************************************************************************/

DECL_END

#endif /* !GUARD_DEX_POSIX_LIBPOSIX_H */
