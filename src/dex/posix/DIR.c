/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_DIR_C
#define GUARD_DEX_POSIX_DIR_C 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"
/**/

#include <deemon/format.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>

#include <hybrid/atomic.h>

#include "../fs/libfs.h" /* For `STAT_*' constants */


#undef posix_opendir_NEED_STAT_EXTENSION
#ifdef posix_opendir_USE_opendir
#ifdef CONFIG_HAVE_readdir64
#undef dirent
#undef readdir
#define dirent  dirent64
#define readdir readdir64
#endif /* CONFIG_HAVE_readdir64 */

#if (!defined(CONFIG_HAVE_DIRENT_D_TYPE) || \
     (!defined(CONFIG_HAVE_DIRENT_D_INO) && !defined(CONFIG_NO_DIRENT_D_FILENO)))
#define posix_opendir_NEED_STAT_EXTENSION
#endif /* !... */
#endif /* posix_opendir_USE_opendir */

/* Figure out how we can best implement lstat() */
#ifdef posix_opendir_NEED_STAT_EXTENSION
#ifdef CONFIG_HAVE_fstatat64
#undef lstatat
#define lstatat(dfd, filename, info) fstatat64(dfd, filename, info, AT_SYMLINK_NOFOLLOW)
#define struct_stat                  struct stat64
#elif defined(CONFIG_HAVE_fstatat)
#undef lstatat
#define lstatat(dfd, filename, info) fstatat(dfd, filename, info, AT_SYMLINK_NOFOLLOW)
#define struct_stat                  struct stat
#elif defined(CONFIG_HAVE_lstat64)
#undef lstat
#define lstat       lstat64
#define struct_stat struct stat64
#elif defined(CONFIG_HAVE_lstat)
#define struct_stat stat
#elif defined(CONFIG_HAVE_stat64)
#undef lstat
#define lstat       stat64
#define struct_stat struct stat64
#elif defined(CONFIG_HAVE_stat)
#undef lstat
#define lstat       stat
#define struct_stat struct stat
#else /* ... */
#undef posix_opendir_NEED_STAT_EXTENSION
#endif /* !... */
#endif /* posix_opendir_NEED_STAT_EXTENSION */

#ifdef posix_opendir_USE_opendir
#ifdef CONFIG_HAVE_DIRENT_D_NAMLEN
#define dirent_namelen(x) (x)->d_namlen
#elif defined(_D_EXACT_NAMLEN)
#define dirent_namelen(x) _D_EXACT_NAMLEN(x)
#else /* ... */
#define dirent_namelen(x) strlen((x)->d_name)
#endif /* !... */
#endif /* posix_opendir_USE_opendir */


DECL_BEGIN

/* NOTE: For performance, the fields of some given `dirent' will only remain valid
 *       as long as the next entry has yet to be read from the directory stream.
 * As such, you must take special care and not do something like `List(opendir(...))',
 * which will produce a list of dirent objects, where  */
typedef struct dir_iterator_object DeeDirIteratorObject;
typedef struct dir_object DeeDirObject;


struct dir_iterator_object {
	OBJECT_HEAD
	DREF DeeObject  *di_path;     /* [1..1][const] String, File, or int */
	DREF DeeObject  *di_pathstr;  /* [0..1][lock(WRITE_ONCE)] String */
	bool             di_skipdots; /* [const] When true, skip '.' and '..' entires. */
#ifdef posix_opendir_USE_FindFirstFileExW
	bool             di_first;    /* [lock(di_lock)] When true, we're at the first entry. */
	HANDLE           di_hnd;      /* [0..1|NULL(INVALID_HANDLE_VALUE)][lock(di_lock)]
	                               * The iteration handle or INVALID_HANDLE_VALUE when exhausted. */
	WIN32_FIND_DATAW di_data;     /* [lock(di_lock)][valid_if(di_hnd != INVALID_HANDLE_VALUE)]
	                               * The file data for the next matching entry. */
#ifndef CONFIG_NO_THREADS
	rwlock_t         di_lock;     /* Lock for te above fields. */
#endif /* !CONFIG_NO_THREADS */
#elif defined(posix_opendir_USE_opendir)
#ifdef posix_opendir_NEED_STAT_EXTENSION
	bool             di_stvalid;  /* [lock(di_lock)] Set to true if `di_st' has been loaded. */
	struct_stat      di_st;       /* [lock(di_lock)] Additional stat information (lazily loaded). */
#endif /* posix_opendir_NEED_STAT_EXTENSION */
	struct dirent   *di_ent;      /* [0..1] Last-read directory entry (or `NULL' for end-of-directory) */
	DIR             *di_dir;      /* [1..1][const] The directory access stream. */
#ifndef CONFIG_NO_THREADS
	rwlock_t         di_lock;     /* Lock for te above fields. */
#endif /* !CONFIG_NO_THREADS */
#endif /* ... */
};

struct dir_object {
	OBJECT_HEAD
	DREF DeeObject *d_path;      /* [1..1][const] String, File, or int */
	bool            d_skipdots;  /* [const] When true, skip '.' and '..' entires. */
	bool            d_inheritfd; /* [const] When true, creating an iterator when `d_path' is
	                              * something other than a string (i.e. a HANDLE or fd_t),
	                              * then the iterator inherits that handle (iow: closes it
	                              * at some point during its lifetime).
	                              * When this dir_object was created by `fdopendir()', then
	                              * this option is enabled by default. */
};


#ifdef posix_opendir_USE_FindFirstFileExW
#ifndef CONFIG_HAVE_wcslen
#define wcslen          dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* posix_opendir_USE_FindFirstFileExW */


#ifdef posix_opendir_USE_FindFirstFileExW
#undef CONFIG_HAVE_DT_FIFO
#undef CONFIG_HAVE_DT_CHR
#undef CONFIG_HAVE_DT_DIR
#undef CONFIG_HAVE_DT_BLK
#undef CONFIG_HAVE_DT_REG
#undef CONFIG_HAVE_DT_LNK
#undef CONFIG_HAVE_DT_SOCK
#endif /* posix_opendir_USE_FindFirstFileExW */

#define USED_DT_UNKNOWN DT_UNKNOWN
#define USED_DT_FIFO    DT_FIFO
#define USED_DT_CHR     DT_CHR
#define USED_DT_DIR     DT_DIR
#define USED_DT_BLK     DT_BLK
#define USED_DT_REG     DT_REG
#define USED_DT_LNK     DT_LNK
#define USED_DT_SOCK    DT_SOCK
#define USED_DT_WHT     DT_WHT

#if (!defined(CONFIG_HAVE_DT_FIFO) || !defined(CONFIG_HAVE_DT_CHR) || !defined(CONFIG_HAVE_DT_DIR) || \
     !defined(CONFIG_HAVE_DT_BLK) || !defined(CONFIG_HAVE_DT_REG) || !defined(CONFIG_HAVE_DT_LNK) ||  \
     !defined(CONFIG_HAVE_DT_SOCK))
#undef USED_DT_FIFO
#undef USED_DT_CHR
#undef USED_DT_DIR
#undef USED_DT_BLK
#undef USED_DT_REG
#undef USED_DT_LNK
#undef USED_DT_SOCK
#define USED_DT_FIFO 1
#define USED_DT_CHR  2
#define USED_DT_DIR  4
#define USED_DT_BLK  6
#define USED_DT_REG  8
#define USED_DT_LNK  10
#define USED_DT_SOCK 12
#undef CONFIG_HAVE_DT_UNKNOWN
#undef CONFIG_HAVE_DT_WHT
#define posix_opendir_USE_GENERIC_DT_CONSTANTS
#endif /* !CONFIG_HAVE_... */

#ifndef CONFIG_HAVE_DT_UNKNOWN
#undef USED_DT_UNKNOWN
#define USED_DT_UNKNOWN 0
#endif /* !CONFIG_HAVE_DT_UNKNOWN */
#ifndef CONFIG_HAVE_DT_WHT
#undef USED_DT_WHT
#define USED_DT_WHT 14
#endif /* !CONFIG_HAVE_DT_WHT */


#ifdef CONFIG_HAVE_DIRENT_D_TYPE_SZ_1
#define TYPEOF_DIRENT_D_TYPE   uint8_t
#define FMTOF_DIRENT_D_TYPE    DEE_FMT_UINT8
#define DEFINE_D_TYPE_CONSTANT DEFINE_INT15
#define DeeInt_New_D_TYPE      DeeInt_NewU8
#elif defined(CONFIG_HAVE_DIRENT_D_TYPE_SZ_2)
#define TYPEOF_DIRENT_D_TYPE   uint16_t
#define FMTOF_DIRENT_D_TYPE    DEE_FMT_UINT16
#define DEFINE_D_TYPE_CONSTANT DEFINE_INT16
#define DeeInt_New_D_TYPE      DeeInt_NewU16
#elif defined(CONFIG_HAVE_DIRENT_D_TYPE_SZ_4)
#define TYPEOF_DIRENT_D_TYPE   uint32_t
#define FMTOF_DIRENT_D_TYPE    DEE_FMT_UINT32
#define DEFINE_D_TYPE_CONSTANT DEFINE_INT32
#define DeeInt_New_D_TYPE      DeeInt_NewU32
#else /* ... */
#define TYPEOF_DIRENT_D_TYPE   uint64_t
#define FMTOF_DIRENT_D_TYPE    DEE_FMT_UINT64
#define DEFINE_D_TYPE_CONSTANT DEFINE_INT64
#define DeeInt_New_D_TYPE      DeeInt_NewU64
#endif /* !... */

/* Define the DT_* constants for export. */
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_UNKNOWN, USED_DT_UNKNOWN);
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_FIFO, USED_DT_FIFO);
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_CHR, USED_DT_CHR);
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_DIR, USED_DT_DIR);
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_BLK, USED_DT_BLK);
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_REG, USED_DT_REG);
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_LNK, USED_DT_LNK);
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_SOCK, USED_DT_SOCK);
INTERN DEFINE_D_TYPE_CONSTANT(posix_DT_WHT, USED_DT_WHT);

#ifdef posix_opendir_USE_GENERIC_DT_CONSTANTS
#define USED_IFTODT(mode)    (((mode)&0170000) >> 12)
#define USED_DTTOIF(dirtype) ((dirtype) << 12)
#ifdef posix_opendir_USE_opendir
#define NATIVE_DT_TO_USED_DT dee_NATIVE_DT_TO_USED_DT
LOCAL TYPEOF_DIRENT_D_TYPE dee_NATIVE_DT_TO_USED_DT(unsigned int dt) {
	switch (dt) {
#ifdef CONFIG_HAVE_DT_DIR
	case DT_DIR:  return USED_DT_DIR;
#endif /* CONFIG_HAVE_DT_DIR */
#ifdef CONFIG_HAVE_DT_CHR
	case DT_CHR:  return USED_DT_CHR;
#endif /* CONFIG_HAVE_DT_CHR */
#ifdef CONFIG_HAVE_DT_BLK
	case DT_BLK:  return USED_DT_BLK;
#endif /* CONFIG_HAVE_DT_BLK */
#ifdef CONFIG_HAVE_DT_REG
	case DT_REG:  return USED_DT_REG;
#endif /* CONFIG_HAVE_DT_REG */
#ifdef CONFIG_HAVE_DT_FIFO
	case DT_FIFO: return USED_DT_FIFO;
#endif /* CONFIG_HAVE_DT_FIFO */
#ifdef CONFIG_HAVE_DT_LNK
	case DT_LNK:  return USED_DT_LNK;
#endif /* CONFIG_HAVE_DT_LNK */
#ifdef CONFIG_HAVE_DT_SOCK
	case DT_SOCK: return USED_DT_SOCK;
#endif /* CONFIG_HAVE_DT_SOCK */
	default:      return USED_DT_UNKNOWN;
	}
}
#endif /* posix_opendir_USE_opendir */
#else /* posix_opendir_USE_GENERIC_DT_CONSTANTS */
#define NATIVE_DT_TO_USED_DT(dt) dt
#ifdef CONFIG_HAVE_IFTODT
#define USED_IFTODT IFTODT
#else /* CONFIG_HAVE_IFTODT */
#define USED_IFTODT dee_IFTODT
LOCAL TYPEOF_DIRENT_D_TYPE dee_IFTODT(unsigned int if_) {
	switch (if_ & STAT_IFMT) {
	case STAT_IFDIR:  return USED_DT_DIR;
	case STAT_IFCHR:  return USED_DT_CHR;
	case STAT_IFBLK:  return USED_DT_BLK;
	case STAT_IFREG:  return USED_DT_REG;
	case STAT_IFIFO:  return USED_DT_FIFO;
	case STAT_IFLNK:  return USED_DT_LNK;
	case STAT_IFSOCK: return USED_DT_SOCK;
	default:          return USED_DT_UNKNOWN;
	}
}
#endif /* !CONFIG_HAVE_IFTODT */
#ifdef CONFIG_HAVE_DTTOIF
#define USED_DTTOIF DTTOIF
#else /* CONFIG_HAVE_DTTOIF */
#define USED_DTTOIF dee_DTTOIF
LOCAL unsigned int dee_DTTOIF(TYPEOF_DIRENT_D_TYPE dt) {
	switch (dt) {
	case USED_DT_DIR:  return STAT_IFDIR;
	case USED_DT_CHR:  return STAT_IFCHR;
	case USED_DT_BLK:  return STAT_IFBLK;
	case USED_DT_REG:  return STAT_IFREG;
	case USED_DT_FIFO: return STAT_IFIFO;
	case USED_DT_LNK:  return STAT_IFLNK;
	case USED_DT_SOCK: return STAT_IFSOCK;
	default:           return 0;
	}
}
#endif /* !CONFIG_HAVE_DTTOIF */
#endif /* !posix_opendir_USE_GENERIC_DT_CONSTANTS */


PRIVATE WUNUSED DREF DeeObject *DCALL
posix_DTTOIF_f(size_t argc, /*nonnull_if(argc != 0)*/ DeeObject *const *argv) {
	TYPEOF_DIRENT_D_TYPE dt;
	if (DeeArg_Unpack(argc, argv, FMTOF_DIRENT_D_TYPE, &dt))
		goto err;
	return DeeInt_NewUInt(USED_DTTOIF(dt));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_IFTODT_f(size_t argc, /*nonnull_if(argc != 0)*/ DeeObject *const *argv) {
	unsigned int if_;
	if (DeeArg_Unpack(argc, argv, "u", &if_))
		goto err;
	return DeeInt_New_D_TYPE(USED_IFTODT(if_));
err:
	return NULL;
}

INTERN DEFINE_CMETHOD(posix_DTTOIF, &posix_DTTOIF_f);
INTERN DEFINE_CMETHOD(posix_IFTODT, &posix_IFTODT_f);
















PRIVATE WUNUSED NONNULL((1)) int DCALL
directory_open(DeeDirIteratorObject *__restrict self,
               DeeObject *__restrict path,
               bool skipdots, bool inheritfd) {
#ifdef posix_opendir_USE_FindFirstFileExW
#define NEED_err
	LPWSTR wname, wpattern;
	size_t i, wname_length;
	if (!DeeString_Check(path)) {
		int result;
		HANDLE hPath;
		DREF DeeObject *sPath;
		hPath = DeeNTSystem_GetHandle(path);
		if (hPath == INVALID_HANDLE_VALUE)
			goto err;
		sPath = DeeNTSystem_GetFilenameOfHandle(hPath);
		if unlikely(!sPath)
			goto err;
		result = directory_open(self, sPath, skipdots, inheritfd);
		Dee_Decref(sPath);
		if (likely(result == 0) && inheritfd)
			CloseHandle(hPath); /* Inherited! */
		return result;
	}
	wname = (LPWSTR)DeeString_AsWide(path);
	if unlikely(!wname)
		goto err;
	/* Append the `\\*' to the given path and fix forward-slashes. */
	wname_length = WSTR_LENGTH(wname);
	wpattern     = (LPWSTR)Dee_AMalloc(8 + wname_length * 2);
	if unlikely(!wpattern)
		goto err;
	for (i = 0; i < wname_length; ++i) {
		WCHAR ch = wname[i];
		/* FindFirstFile() actually fails when handed forward-slashes.
		 * That's something I didn't notice in the old deemon, which
		 * caused fs.dir() to (seemingly) fail at random. */
		if (ch == '/')
			ch = '\\';
		wpattern[i] = ch;
	}
	/* Use the current directory if the given name is empty. */
	if (!wname_length)
		wpattern[wname_length++] = '.';
	/* Append a trailing backslash if there isn't one already. */
	if (wpattern[wname_length - 1] != '\\')
		wpattern[wname_length++] = '\\';
	/* Append a match-all wildcard. */
	wpattern[wname_length++] = '*';
	wpattern[wname_length]   = 0;
	DBG_ALIGNMENT_DISABLE();
	self->di_hnd = FindFirstFileExW(wpattern, FindExInfoBasic, &self->di_data,
	                                FindExSearchNameMatch, NULL, 0);
	DBG_ALIGNMENT_ENABLE();
	Dee_AFree(wpattern);
	self->di_first = true;
	if unlikely(self->di_hnd == INVALID_HANDLE_VALUE) {
		DWORD dwError;
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (dwError != ERROR_NO_MORE_FILES) {
			if (DeeNTSystem_IsFileNotFoundError(dwError)) {
				DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, dwError,
				                        "Path %r could not be found",
				                        path);
			} else if (DeeNTSystem_IsNotDir(dwError)) {
				DREF DeeTypeObject *tp;
				tp = (DREF DeeTypeObject *)DeeObject_GetAttrString(FS_MODULE, "NoDirectory");
				if (tp) {
					DeeNTSystem_ThrowErrorf(tp, dwError, "Some part of the path %r is not a directory", path);
					Dee_Decref(tp);
				}
			} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
				DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, dwError,
				                        "Search permissions are not granted for path %r",
				                        path);
			} else {
				DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
				                        "Failed to open directory %r",
				                        path);
			}
			goto err;
		}
		/* Empty directory? ok... */
		self->di_first = false;
	} else if (skipdots) {
		while (self->di_data.cFileName[0] == '.' &&
		       (self->di_data.cFileName[1] == 0 ||
		        (self->di_data.cFileName[1] == '.' &&
		         self->di_data.cFileName[2] == 0))) {
			/* Skip this one... */
again_skipdots:
			DBG_ALIGNMENT_DISABLE();
			if (!FindNextFileW(self->di_hnd, &self->di_data)) {
				HANDLE hnd;
				DWORD dwError = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if unlikely(dwError != ERROR_NO_MORE_FILES) {
					if (DeeNTSystem_IsBadAllocError(dwError)) {
						if (Dee_CollectMemory(1))
							goto again_skipdots;
					} else {
						DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
						                        "Failed to read entires from directory %r",
						                        path);
					}
					DBG_ALIGNMENT_DISABLE();
					FindClose(self->di_hnd);
					DBG_ALIGNMENT_ENABLE();
					goto err;
				}
				hnd          = self->di_hnd;
				self->di_hnd = INVALID_HANDLE_VALUE;
				DBG_ALIGNMENT_DISABLE();
				FindClose(hnd);
			}
			DBG_ALIGNMENT_ENABLE();
		}
	}
	rwlock_init(&self->di_lock);
#elif defined(posix_opendir_USE_opendir)
	DIR *dir;
#define NEED_err
EINTR_LABEL(again)
	if (!DeeString_Check(path)) {
		int fd;
		fd = DeeUnixSystem_GetFD(path);
		if (fd == -1)
			goto err;
#if defined(CONFIG_HAVE_dup) && defined(CONFIG_HAVE_fdopendir)
		if (inheritfd) {
			dir = fdopendir(fd);
		} else {
			DBG_ALIGNMENT_DISABLE();
			fd = dup(fd);
			if (fd == -1)
				dir = NULL;
			else {
				dir = fdopendir(fd);
#ifdef CONFIG_HAVE_close
				if unlikely(!dir) {
					int saved_errno;
					saved_errno = DeeSystem_GetErrno();
					close(fd);
					DeeSystem_SetErrno(saved_errno);
				}
			}
#endif /* CONFIG_HAVE_close */
		}
#else /* CONFIG_HAVE_dup && CONFIG_HAVE_fdopendir */
#ifdef CONFIG_HAVE_fdopendir
		if (inheritfd) {
			dir = fdopendir(fd);
		} else
#endif /* CONFIG_HAVE_fdopendir */
		{
			int result;
			DREF DeeObject *sPath;
			sPath = DeeSystem_GetFilenameOfFD(fd);
			if unlikely(!sPath)
				goto err;
			result = directory_open(self, sPath, skipdots, inheritfd);
			Dee_Decref(sPath);
			return result;
		}
#endif /* !CONFIG_HAVE_dup || !CONFIG_HAVE_fdopendir */
	} else {
		char const *utf8;
		utf8 = DeeString_AsUtf8(path);
		if unlikely(!utf8)
			goto err;
		DBG_ALIGNMENT_DISABLE();
		dir = opendir(utf8);
	}
	if unlikely(dir == NULL) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOENT(error, err, "Path %r could not be found", path)
		HANDLE_ENOTDIR(error, err, "Path %r could not be found", path)
		HANDLE_EACCES(error, err, "Some part of the path %r is not a directory", path)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to open directory %r",
		                          path);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	self->di_dir     = dir;
	self->di_ent     = NULL;
#ifdef posix_opendir_NEED_STAT_EXTENSION
	self->di_stvalid = false;
#endif /* posix_opendir_NEED_STAT_EXTENSION */
	rwlock_init(&self->di_lock);
#endif /* ... */
	self->di_skipdots = skipdots;
	self->di_path     = path;
	self->di_pathstr  = NULL;
	Dee_Incref(path);
	return 0;
#ifdef NEED_err
#undef NEED_err
err:
	return -1;
#endif /* NEED_err */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeDirIteratorObject *DCALL
diriter_next(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
again:
	rwlock_write(&self->di_lock);
	if (!self->di_first) {
		if (self->di_hnd == INVALID_HANDLE_VALUE) {
			rwlock_endwrite(&self->di_lock);
			return (DREF DeeDirIteratorObject *)ITER_DONE;
		}
		DBG_ALIGNMENT_DISABLE();
		if (!FindNextFileW(self->di_hnd, &self->di_data)) {
			HANDLE hnd;
			DWORD dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if unlikely(dwError != ERROR_NO_MORE_FILES) {
				rwlock_endwrite(&self->di_lock);
				if (DeeNTSystem_IsBadAllocError(dwError)) {
					if (Dee_CollectMemory(1))
						goto again;
				} else {
					DeeNTSystem_ThrowErrorf(&DeeError_FSError, dwError,
					                        "Failed to read entires from directory %r",
					                        self->di_path);
				}
				return NULL;
			}
			hnd          = self->di_hnd;
			self->di_hnd = INVALID_HANDLE_VALUE;
			rwlock_endwrite(&self->di_lock);
			DBG_ALIGNMENT_DISABLE();
			FindClose(hnd);
			DBG_ALIGNMENT_ENABLE();
			return (DREF DeeDirIteratorObject *)ITER_DONE;
		}
		DBG_ALIGNMENT_ENABLE();
	}
	self->di_first = false;
	if (self->di_skipdots) {
		if (self->di_data.cFileName[0] == '.' &&
		    (self->di_data.cFileName[1] == 0 ||
		     (self->di_data.cFileName[1] == '.' &&
		      self->di_data.cFileName[2] == 0)))
			goto again; /* Skip this one... */
	}
	rwlock_endwrite(&self->di_lock);
	Dee_Incref(self);
	return self;
#elif defined(CONFIG_HAVE_opendir) && defined(CONFIG_HAVE_readdir)
	rwlock_write(&self->di_lock);
again:
	DBG_ALIGNMENT_DISABLE();
	DeeSystem_SetErrno(0);
	self->di_ent = readdir(self->di_dir);
	if (self->di_ent == NULL) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		rwlock_endwrite(&self->di_lock);
		if (error == 0)
			return (DREF DeeDirIteratorObject *)ITER_DONE; /* End of directory. */
		HANDLE_EINTR(error, again, err)
		DeeUnixSystem_ThrowErrorf(NULL, error,
		                          "Failed to read entires from directory %r",
		                          self->di_path);
err:
		return NULL;
	}
	DBG_ALIGNMENT_ENABLE();
	if (self->di_skipdots) {
		char *name = self->di_ent->d_name;
		if (name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0)))
			goto again; /* Skip this one... */
	}
#ifdef posix_opendir_NEED_STAT_EXTENSION
	self->di_stvalid = false;
#endif /* posix_opendir_NEED_STAT_EXTENSION */
	rwlock_endwrite(&self->di_lock);
	Dee_Incref(self);
	return self;
#else /* ... */
	(void)self;
	return (DREF DeeDirIteratorObject *)ITER_DONE;
#endif /* !... */
}

PRIVATE NONNULL((1)) void DCALL
diriter_fini(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->di_hnd != INVALID_HANDLE_VALUE) {
		DBG_ALIGNMENT_DISABLE();
		FindClose(self->di_hnd);
		DBG_ALIGNMENT_ENABLE();
	}
#elif defined(posix_opendir_USE_opendir)
	DBG_ALIGNMENT_DISABLE();
	closedir(self->di_dir);
	DBG_ALIGNMENT_ENABLE();
#endif /* ... */
	Dee_XDecref(self->di_pathstr);
	Dee_Decref(self->di_path);
}


PRIVATE NONNULL((1)) int DCALL
diriter_unbound_attr(char const *__restrict name) {
	return DeeError_Throwf(&DeeError_UnboundAttribute,
	                       "Unbound attribute `%k.%s'",
	                       &DeeDirIterator_Type, name);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
diriter_makepathstr(DeeDirIteratorObject *__restrict self) {
	DREF DeeStringObject *result;
	result = (DREF DeeStringObject *)self->di_path;
	if (DeeString_Check(result))
		return_reference_(result);
#ifdef posix_opendir_USE_FindFirstFileExW
	{
		HANDLE hPath;
		hPath = DeeNTSystem_GetHandle((DeeObject *)result);
		if (hPath == INVALID_HANDLE_VALUE)
			return NULL;
		result = (DREF DeeStringObject *)DeeNTSystem_GetFilenameOfHandle(hPath);
	}
	return result;
#elif defined(posix_opendir_USE_opendir)
	{
		int fd;
		fd = DeeUnixSystem_GetFD((DeeObject *)result);
		if (fd == -1)
			return NULL;
		result = (DREF DeeStringObject *)DeeSystem_GetFilenameOfFD(fd);
	}
	return result;
#else /* ... */
	diriter_unbound_attr("pathstr");
	return NULL;
#endif /* !... */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
diriter_getpathstr(DeeDirIteratorObject *__restrict self) {
	DREF DeeStringObject *result;
again:
	result = (DREF DeeStringObject *)self->di_pathstr;
	if (result == NULL) {
		result = diriter_makepathstr(self);
		/* Remember the full path string. */
		if (!ATOMIC_CMPXCH(self->di_pathstr, NULL, result)) {
			Dee_Decref_likely(result);
			goto again;
		}
	}
	return_reference_(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
diriter_get_d_fullname(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
#define HAVE_D_FULLNAME
	if (self->di_hnd == INVALID_HANDLE_VALUE)
#elif defined(posix_opendir_USE_opendir)
#define HAVE_D_FULLNAME
	if (self->di_ent != NULL)
#endif /* ... */
	{
		diriter_unbound_attr("d_name");
		return NULL;
	}
#ifdef HAVE_D_FULLNAME
#undef HAVE_D_FULLNAME
	{
		DREF DeeStringObject *path;
		path = diriter_getpathstr(self);
		if likely(path) {
			struct unicode_printer printer = UNICODE_PRINTER_INIT;
			if unlikely(unicode_printer_printstring(&printer, (DeeObject *)path) < 0) {
				Dee_Decref(path);
err_printer:
				unicode_printer_fini(&printer);
				return NULL;
			}
			Dee_Decref(path);
			if (UNICODE_PRINTER_LENGTH(&printer)) {
				uint32_t lastch;
				lastch = UNICODE_PRINTER_GETCHAR(&printer, UNICODE_PRINTER_LENGTH(&printer) - 1);
#ifdef posix_opendir_USE_FindFirstFileExW
				if (lastch != '/' && lastch != '\\') {
					if (unicode_printer_putc(&printer, '\\'))
						goto err_printer;
				}
#else /* posix_opendir_USE_FindFirstFileExW */
				if (lastch != '/') {
					if (unicode_printer_putc(&printer, '/'))
						goto err_printer;
				}
#endif /* !posix_opendir_USE_FindFirstFileExW */
			}
#ifdef posix_opendir_USE_FindFirstFileExW
			if unlikely(unicode_printer_printwide(&printer, self->di_data.cFileName,
			                                      wcslen(self->di_data.cFileName)) < 0)
				goto err_printer;
#elif defined(posix_opendir_USE_opendir)
			if unlikely(unicode_printer_printutf8(&printer, self->di_ent->d_name,
			                                      dirent_namelen(self->di_ent)) < 0)
				goto err_printer;
#else /* ... */
			diriter_unbound_attr("d_name");
			goto err_printer;
#endif /* !... */
			path = (DREF DeeStringObject *)unicode_printer_pack(&printer);
		}
		return path;
	}
#endif /* HAVE_D_FULLNAME */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_name(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->di_hnd != INVALID_HANDLE_VALUE) {
		return DeeString_NewWide(self->di_data.cFileName,
		                         wcslen(self->di_data.cFileName),
		                         STRING_ERROR_FREPLAC);
	}
#elif defined(posix_opendir_USE_opendir)
	if (self->di_ent != NULL) {
		return DeeString_NewUtf8(self->di_ent->d_name,
		                         dirent_namelen(self->di_ent),
		                         STRING_ERROR_FREPLAC);
	}
#endif /* ... */
	diriter_unbound_attr("d_name");
	return NULL;
}

#ifdef posix_opendir_NEED_STAT_EXTENSION
PRIVATE WUNUSED NONNULL((1)) int DCALL
diriter_loadstat(DeeDirIteratorObject *__restrict self) {
	int error;
	if (self->di_stvalid)
		return 0;
again:
#if defined(lstatat) && defined(CONFIG_HAVE_dirfd)
	if (lstatat(dirfd(self->di_dir), self->di_ent->d_name, &self->di_st) == 0) {
		self->di_stvalid = true;
		return 0;
	}
	error = DeeSystem_GetErrno();
#else /* lstatat && CONFIG_HAVE_dirfd */
	DREF DeeStringObject *fullname;
	char const *utf8;
	fullname = diriter_get_d_fullname(self);
	if unlikely(!fullname)
		return -1;
	utf8 = DeeString_AsUtf8(fullname);
	if unlikely(!utf8) {
		Dee_Decref(fullname);
		return -1;
	}
	if (lstat(utf8, &self->di_st) == 0) {
		self->di_stvalid = true;
		Dee_Decref(fullname);
		return 0;
	}
	error = DeeSystem_GetErrno();
	Dee_Decref(fullname);
#endif /* !lstatat || !CONFIG_HAVE_dirfd */
	HANDLE_EINTR(error, again, err)
	return DeeUnixSystem_ThrowErrorf(NULL, error,
	                                 "Failed to stat %q in %k",
	                                 self->di_ent->d_name, self);
}
#endif /* posix_opendir_NEED_STAT_EXTENSION */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diriter_get_d_type(DeeDirIteratorObject *__restrict self) {
#ifdef posix_opendir_USE_FindFirstFileExW
	if (self->di_hnd != INVALID_HANDLE_VALUE) {
		DeeObject *result;
		if (self->di_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			result = (DeeObject *)&posix_DT_DIR;
		} else if (self->di_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
			result = (DeeObject *)&posix_DT_LNK;
		} else if (self->di_data.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) {
			result = (DeeObject *)&posix_DT_UNKNOWN;
		} else if (self->di_data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {
			/* TODO: Check if it's an old-style cygwin symlink! */
			result = (DeeObject *)&posix_DT_REG;
		} else {
			result = (DeeObject *)&posix_DT_REG;
		}
		return_reference_(result);
	}
#elif defined(posix_opendir_USE_opendir)
	if (self->di_ent != NULL) {
#ifdef CONFIG_HAVE_DIRENT_D_TYPE
		return DeeInt_New_D_TYPE(NATIVE_DT_TO_USED_DT(self->di_ent->d_type));
#elif defined(posix_opendir_NEED_STAT_EXTENSION)
		if unlikely(diriter_loadstat(self))
			return NULL;
		return DeeInt_New_D_TYPE(USED_IFTODT(self->di_st.st_mode));
#endif /* !CONFIG_HAVE_DIRENT_D_TYPE */
	}
#endif /* ... */
	diriter_unbound_attr("d_type");
	return NULL;
}



PRIVATE NONNULL((1, 2)) void DCALL
diriter_visit(DeeDirIteratorObject *__restrict self, dvisit_t proc, void *arg) {
	/* Not needed (and mustn't be enabled; `diriter_visit' is re-used as `dir_visit'!) */
	/*Dee_XVisit(self->di_pathstr);*/
	Dee_Visit(self->di_path);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
diriter_init(DeeDirIteratorObject *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeObject *path;
	bool skipdots  = true;
	bool inheritfd = false;
	if (DeeArg_Unpack(argc, argv, "o|bb:_DirIterator", &path, &skipdots, &inheritfd))
		goto err;
	if (Dee_TYPE(path) == &DeeDir_Type) {
		DeeDirObject *dir;
		dir       = (DeeDirObject *)path;
		skipdots  = dir->d_skipdots;
		inheritfd = dir->d_inheritfd;
		path      = dir->d_path;
	}
	return directory_open(self, path, skipdots, inheritfd);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeDirObject *DCALL
diriter_getseq(DeeDirIteratorObject *__restrict self) {
	DREF DeeDirObject *result;
	result = DeeObject_MALLOC(DeeDirObject);
	if likely(result) {
		DeeObject_Init(result, &DeeDir_Type);
		result->d_path      = self->di_path;
		result->d_skipdots  = self->di_skipdots;
		result->d_inheritfd = false;
		Dee_Incref(result->d_path);
	}
	return result;
}

PRIVATE struct type_getset tpconst diriter_getsets[] = {
	{ "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_getseq, NULL, NULL,
	  DOC("->?GDIR") },
	{ "pathstr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_getpathstr, NULL, NULL,
	  DOC("->?Dstring") },
	{ "d_name", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_get_d_name, NULL, NULL,
	  DOC("->?Dstring\n"
	      "The name of the current file") },
	{ "d_fullname", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_get_d_fullname, NULL, NULL,
	  DOC("->?Dstring\n"
	      "The full filename of the current file") },
	{ "d_type", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diriter_get_d_type, NULL, NULL,
	  DOC("->?Dint\n"
	      "The type of the current file (one of ${DT_*})") },
	/* TODO: d_ino */

	/* TODO: Additional (stat-like) fields (to take advantage of info that windows gives us):
	 *  - d_atime: Time
	 *  - d_mtime: Time
	 *  - d_ctime: Time
	 *  - d_size: int    (file size)
	 */
	{ NULL }
};

PRIVATE struct type_member tpconst diriter_members[] = {
	TYPE_MEMBER_FIELD_DOC("path", STRUCT_OBJECT, offsetof(DeeDirIteratorObject, di_path),
	                      "->?X3?Dstring?DFile?Dint"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeDirIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DirIterator",
	/* .tp_doc      = */ DOC("(dir:?X4?GDIR?Dstring?DFile?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&diriter_init,
				TYPE_FIXED_ALLOCATOR(DeeDirIteratorObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&diriter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
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
	/* .tp_getsets       = */ diriter_getsets,
	/* .tp_members       = */ diriter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dir_copy(DeeDirObject *__restrict self,
         DeeDirObject *__restrict other) {
	self->d_path      = other->d_path;
	self->d_skipdots  = other->d_skipdots;
	self->d_inheritfd = false;
	Dee_Incref(self->d_path);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
dir_fini(DeeDirObject *__restrict self) {
	Dee_Decref(self->d_path);
}

STATIC_ASSERT(offsetof(DeeDirIteratorObject, di_path) ==
              offsetof(DeeDirObject, d_path));
#define dir_visit   diriter_visit
#define dir_members diriter_members

PRIVATE struct keyword opendir_kwlist[] = { K(path), K(skipdots), K(inheritfd), KEND };

PRIVATE WUNUSED NONNULL((1)) int DCALL
dir_init_kw(DeeDirObject *__restrict self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	self->d_skipdots  = true;
	self->d_inheritfd = false;
	if (DeeArg_UnpackKw(argc, argv, kw, opendir_kwlist, "o|bb:opendir",
	                    &self->d_path, &self->d_skipdots, &self->d_inheritfd))
		goto err;
	Dee_Incref(self->d_path);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
posix_fdopendir_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeDirObject *result;
	DeeObject *path;
	bool skipdots  = true;
	bool inheritfd = true;
	if (DeeArg_UnpackKw(argc, argv, kw, opendir_kwlist, "o|bb:fdopendir",
	                    &path, &skipdots, &inheritfd))
		goto err;
	result = DeeObject_MALLOC(DeeDirObject);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeDir_Type);
	Dee_Incref(path);
	result->d_path      = path;
	result->d_skipdots  = skipdots;
	result->d_inheritfd = inheritfd;
	return (DREF DeeObject *)result;
err:
	return NULL;
}

INTERN DEFINE_KWCMETHOD(posix_fdopendir, &posix_fdopendir_f);


INTERN WUNUSED NONNULL((1)) DREF DeeDirIteratorObject *DCALL
dir_iter(DeeDirObject *__restrict self) {
	DREF DeeDirIteratorObject *result;
	result = DeeObject_MALLOC(DeeDirIteratorObject);
	if unlikely(!result)
		goto done;
	if unlikely(directory_open(result,
	                           self->d_path,
	                           self->d_skipdots,
	                           self->d_inheritfd)) {
		DeeObject_FREE(result);
		result = NULL;
		goto done;
	}
	DeeObject_Init(result, &DeeDirIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq dir_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dir_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};


PRIVATE struct type_member tpconst dir_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeDirIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeDir_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Dir",
	/* .tp_doc      = */ DOC("(path:?X3?Dstring?DFile?Dint,skipdots=!t,inheritfd=!f)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor   = */ (dfunptr_t)&dir_copy,
				/* .tp_deep_ctor   = */ (dfunptr_t)&dir_copy,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeDirObject),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&dir_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dir_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
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
	/* .tp_members       = */ dir_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dir_class_members
};

DECL_END

#endif /* !GUARD_DEX_POSIX_DIR_C */
