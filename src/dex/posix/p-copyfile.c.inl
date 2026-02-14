/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_COPYFILE_C_INL
#define GUARD_DEX_POSIX_P_COPYFILE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>             /* DeeArg_UnpackStructKw, UNP* */
#include <deemon/dex.h>             /* DEXSYM_READONLY, DEX_MEMBER_F */
#include <deemon/error.h>           /* DeeError_* */
#include <deemon/file.h>            /* DeeFile_Filename, DeeFile_Open, DeeSystemFile_Check, DeeSystem_FILE_USE_nt_HANDLE, OPEN_F* */
#include <deemon/filetypes.h>       /* DeeSystemFile_GetHandle */
#include <deemon/int.h>             /* DeeInt_NewUInt64 */
#include <deemon/mapfile.h>         /* DeeMapFile_F_ATSTART, DeeMapFile_F_NORMAL */
#include <deemon/none.h>            /* DeeNone_Check, Dee_None, return_none */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_AsUInt64, DeeObject_Type, DeeTypeObject, Dee_Decref, Dee_Incref, Dee_visit_t, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/system-features.h> /* AT_EMPTY_PATH, AT_SYMLINK_NOFOLLOW, RENAME_NOREPLACE */
#include <deemon/type.h>            /* DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visit, METHOD_FNOREFESCAPE, METHOD_FNORMAL, STRUCT_*, TF_NONE, TP_FNORMAL, TYPE_*, type_getset, type_member */
#include <deemon/object.h>
#include <deemon/objmethod.h> /*  */
#include <deemon/serial.h>    /* DeeSerial*, Dee_seraddr_t */
#include <deemon/string.h>    /* DeeString_Check, DeeString_IsEmpty */

#include "p-readlink.c.inl" /* Needed for `lcopyfile()' to check for symbolic links. */
#include "p-remove.c.inl"   /* Needed for `lcopyfile()' to remove existing files when `RENAME_NOREPLACE' isn't given. */
#include "p-symlink.c.inl"  /* Needed for `lcopyfile()' to create copies of symbolic links. */

#include <stdbool.h> /* false */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint64_t */

DECL_BEGIN



/* Figure out how to implement `fcopyfile()' */
#undef posix_fcopyfile_USE_posix_copyfile_fileio
#undef posix_fcopyfile_USE_STUB
#if 1 /* Always supported */
#define posix_fcopyfile_USE_posix_copyfile_fileio
#else /* ... */
#define posix_fcopyfile_USE_STUB
#endif /* !... */



/* Figure out how to implement `copyfile()' */
#undef posix_copyfile_USE_posix_copyfile_fileio
#undef posix_copyfile_USE_STUB
#if 1 /* Always supported */
#define posix_copyfile_USE_posix_copyfile_fileio
#else /* ... */
#define posix_copyfile_USE_STUB
#endif /* !... */



/* Figure out how to implement `lcopyfile()' */
#undef posix_lcopyfile_USE_posix_copyfile_fileio
#undef posix_lcopyfile_USE_STUB
#if 1 /* Always supported */
#define posix_lcopyfile_USE_posix_copyfile_fileio
#else /* ... */
#define posix_lcopyfile_USE_STUB
#endif /* !... */



/* Figure out how to implement `copyfileat()' */
#undef posix_copyfileat_USE_copyfile__AND__lcopyfile__AND__fcopyfile
#undef posix_copyfileat_USE_STUB
#if (!defined(posix_copyfile_USE_STUB) &&  \
     !defined(posix_lcopyfile_USE_STUB) && \
     !defined(posix_fcopyfile_USE_STUB))
#define posix_copyfileat_USE_copyfile__AND__lcopyfile__AND__fcopyfile
#else /* ... */
#define posix_copyfileat_USE_STUB
#endif /* !... */



/*[[[deemon import("rt.gen.dexutils").gw("fcopyfile",
	"oldfd:?X2?DFile?Dint,newpath:?X3?Dstring?DFile?Dint,flags:u=0,"
	"progress:?DCallable=Dee_None,bufsize:?Dint=Dee_None", libname: "posix"); ]]]*/
#define POSIX_FCOPYFILE_DEF          DEX_MEMBER_F("fcopyfile", &posix_fcopyfile, DEXSYM_READONLY, "(oldfd:?X2?DFile?Dint,newpath:?X3?Dstring?DFile?Dint,flags=!0,progress:?DCallable=!N,bufsize:?Dint=!N)"),
#define POSIX_FCOPYFILE_DEF_DOC(doc) DEX_MEMBER_F("fcopyfile", &posix_fcopyfile, DEXSYM_READONLY, "(oldfd:?X2?DFile?Dint,newpath:?X3?Dstring?DFile?Dint,flags=!0,progress:?DCallable=!N,bufsize:?Dint=!N)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL posix_fcopyfile_f_impl(DeeObject *oldfd, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize);
#ifndef DEFINED_kwlist__oldfd_newpath_flags_progress_bufsize
#define DEFINED_kwlist__oldfd_newpath_flags_progress_bufsize
PRIVATE DEFINE_KWLIST(kwlist__oldfd_newpath_flags_progress_bufsize, { KEX("oldfd", 0x5a92fcdb, 0x3de145419f68339e), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("progress", 0x2b0d4ed7, 0xcace2890c513a747), KEX("bufsize", 0x24b3a7e0, 0x94bf4a0770b058aa), KEND });
#endif /* !DEFINED_kwlist__oldfd_newpath_flags_progress_bufsize */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fcopyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *oldfd;
		DeeObject *newpath;
		unsigned int flags;
		DeeObject *progress;
		DeeObject *bufsize;
	} args;
	args.flags = 0;
	args.progress = Dee_None;
	args.bufsize = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__oldfd_newpath_flags_progress_bufsize, "oo|uoo:fcopyfile", &args))
		goto err;
	return posix_fcopyfile_f_impl(args.oldfd, args.newpath, args.flags, args.progress, args.bufsize);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_fcopyfile, &posix_fcopyfile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL posix_fcopyfile_f_impl(DeeObject *oldfd, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize)
/*[[[end]]]*/
{
#ifdef posix_fcopyfile_USE_posix_copyfile_fileio
	DREF DeeObject *src_file;
	DREF DeeObject *dst_file;
	if (flags & ~(RENAME_NOREPLACE)) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}

	/* Open arguments as deemon file objects. */
	src_file = posix_fd_openfile(oldfd, OPEN_FRDONLY | OPEN_FCLOEXEC);
#define NEED_posix_fd_openfile
	if unlikely(!src_file)
		goto err;

	/* Load `newpath' as a deemon file object. */
	if (DeeString_Check(newpath)) {
		if (flags & RENAME_NOREPLACE) {
			dst_file = DeeFile_Open(newpath, OPEN_FWRONLY | OPEN_FCREAT | OPEN_FTRUNC | OPEN_FEXCL, 0644);
			if unlikely(dst_file == ITER_DONE) {
				DeeError_Throwf(&DeeError_FileExists, "File %r already exists", newpath);
				goto err_src_file;
			}
		} else {
			dst_file = DeeFile_Open(newpath, OPEN_FWRONLY | OPEN_FCREAT | OPEN_FTRUNC, 0644);
			ASSERT(dst_file != ITER_DONE);
		}
	} else {
		dst_file = posix_fd_openfile(newpath, OPEN_FWRONLY | OPEN_FCLOEXEC);
#define NEED_posix_fd_openfile
	}
	if unlikely(!dst_file)
		goto err_src_file;

	/* Do the actual copy. */
	if unlikely(posix_copyfile_fileio(src_file, dst_file, progress, bufsize, DeeMapFile_F_NORMAL))
		goto err_src_file_dst_file;
#define NEED_posix_copyfile_fileio

	/* Cleanup & indicate success. */
	Dee_Decref(dst_file);
	Dee_Decref(src_file);
	return_none;
err_src_file_dst_file:
	Dee_Decref(dst_file);
err_src_file:
	Dee_Decref(src_file);
err:
	return NULL;
#endif /* posix_fcopyfile_USE_posix_copyfile_fileio */

#ifdef posix_fcopyfile_USE_STUB
	(void)oldfd;
	(void)newpath;
	(void)progress;
	(void)bufsize;
#define NEED_posix_err_unsupported
	posix_err_unsupported("fcopyfile");
	return NULL;
#endif /* posix_fcopyfile_USE_STUB */
}

/*[[[deemon import("rt.gen.dexutils").gw("copyfile",
	"oldpath:?X3?DFile?Dint?Dstring,newpath:?X3?Dstring?DFile?Dint,flags:u=0,"
	"progress:?DCallable=Dee_None,bufsize:?Dint=Dee_None", libname: "posix"); ]]]*/
#define POSIX_COPYFILE_DEF          DEX_MEMBER_F("copyfile", &posix_copyfile, DEXSYM_READONLY, "(oldpath:?X3?DFile?Dint?Dstring,newpath:?X3?Dstring?DFile?Dint,flags=!0,progress:?DCallable=!N,bufsize:?Dint=!N)"),
#define POSIX_COPYFILE_DEF_DOC(doc) DEX_MEMBER_F("copyfile", &posix_copyfile, DEXSYM_READONLY, "(oldpath:?X3?DFile?Dint?Dstring,newpath:?X3?Dstring?DFile?Dint,flags=!0,progress:?DCallable=!N,bufsize:?Dint=!N)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL posix_copyfile_f_impl(DeeObject *oldpath, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize);
#ifndef DEFINED_kwlist__oldpath_newpath_flags_progress_bufsize
#define DEFINED_kwlist__oldpath_newpath_flags_progress_bufsize
PRIVATE DEFINE_KWLIST(kwlist__oldpath_newpath_flags_progress_bufsize, { KEX("oldpath", 0x6af2d717, 0x74cfc4ae2e46bac3), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("progress", 0x2b0d4ed7, 0xcace2890c513a747), KEX("bufsize", 0x24b3a7e0, 0x94bf4a0770b058aa), KEND });
#endif /* !DEFINED_kwlist__oldpath_newpath_flags_progress_bufsize */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_copyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *oldpath;
		DeeObject *newpath;
		unsigned int flags;
		DeeObject *progress;
		DeeObject *bufsize;
	} args;
	args.flags = 0;
	args.progress = Dee_None;
	args.bufsize = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__oldpath_newpath_flags_progress_bufsize, "oo|uoo:copyfile", &args))
		goto err;
	return posix_copyfile_f_impl(args.oldpath, args.newpath, args.flags, args.progress, args.bufsize);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_copyfile, &posix_copyfile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL posix_copyfile_f_impl(DeeObject *oldpath, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize)
/*[[[end]]]*/
{
#ifdef posix_copyfile_USE_posix_copyfile_fileio
	DREF DeeObject *src_file;
	DREF DeeObject *dst_file;
	unsigned int src_file_mmap_hints = DeeMapFile_F_NORMAL;
	if (flags & ~(RENAME_NOREPLACE)) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}

	/* Open arguments as deemon file objects. */
	if (DeeString_Check(oldpath)) {
		src_file = DeeFile_Open(oldpath, OPEN_FRDONLY | OPEN_FCLOEXEC, 0);
		if unlikely(src_file == ITER_DONE) {
			DeeError_Throwf(&DeeError_FileNotFound, "File %r could not be found", oldpath);
			goto err;
		}

		/* We opened the source-file, so we can assume that it's file-pointer is at the start. */
		src_file_mmap_hints |= DeeMapFile_F_ATSTART;
	} else {
		src_file = posix_fd_openfile(oldpath, OPEN_FRDONLY | OPEN_FCLOEXEC);
#define NEED_posix_fd_openfile
	}
	if unlikely(!src_file)
		goto err;

	/* Load `newpath' as a deemon file object. */
	if (DeeString_Check(newpath)) {
		if (flags & RENAME_NOREPLACE) {
			dst_file = DeeFile_Open(newpath, OPEN_FWRONLY | OPEN_FCREAT | OPEN_FTRUNC | OPEN_FEXCL | OPEN_FCLOEXEC, 0644);
			if unlikely(dst_file == ITER_DONE) {
				DeeError_Throwf(&DeeError_FileExists, "File %r already exists", newpath);
				goto err_src_file;
			}
		} else {
			dst_file = DeeFile_Open(newpath, OPEN_FWRONLY | OPEN_FCREAT | OPEN_FTRUNC | OPEN_FCLOEXEC, 0644);
			ASSERT(dst_file != ITER_DONE);
		}
	} else {
		dst_file = posix_fd_openfile(newpath, OPEN_FWRONLY | OPEN_FCLOEXEC);
#define NEED_posix_fd_openfile
	}
	if unlikely(!dst_file)
		goto err_src_file;

	/* Do the actual copy. */
	if unlikely(posix_copyfile_fileio(src_file, dst_file, progress,
	                                  bufsize, src_file_mmap_hints))
		goto err_src_file_dst_file;
#define NEED_posix_copyfile_fileio

	/* Cleanup & indicate success. */
	Dee_Decref(dst_file);
	Dee_Decref(src_file);
	return_none;
err_src_file_dst_file:
	Dee_Decref(dst_file);
err_src_file:
	Dee_Decref(src_file);
err:
	return NULL;
#endif /* posix_copyfile_USE_posix_copyfile_fileio */

#ifdef posix_copyfile_USE_STUB
	(void)oldpath;
	(void)newpath;
	(void)progress;
	(void)bufsize;
#define NEED_posix_err_unsupported
	posix_err_unsupported("copyfile");
	return NULL;
#endif /* posix_copyfile_USE_STUB */
}



/* Figure out how we want to implement the symlink check in `lcopyfile()' */
#undef posix_lcopyfile_USE_symlink_after_open
#undef posix_lcopyfile_USE_symlink_before_open
#if (defined(DeeSystem_FILE_USE_nt_HANDLE) && \
     defined(posix_readlink_USE_nt_FReadLink) && !defined(posix_symlink_USE_STUB))
#define posix_lcopyfile_USE_symlink_after_open
#elif !defined(posix_readlink_USE_STUB) && !defined(posix_symlink_USE_STUB)
#define posix_lcopyfile_USE_symlink_before_open
#endif /* ... */

#if (defined(posix_lcopyfile_USE_symlink_before_open) || \
     defined(posix_lcopyfile_USE_symlink_after_open))
/* Same as `posix__symlink_f_impl()', but unless `RENAME_NOREPLACE' is given,
 * delete an existing file at `path' (though this doesn't delete an existing
 * directory) */
PRIVATE WUNUSED DREF DeeObject *DCALL
posix__symlink_with_optional_replace(DeeObject *text, DeeObject *path,
                                     unsigned int flags) {
	DREF DeeObject *result;
	(void)flags;
#ifndef posix_unlink_USE_STUB
again:
#endif /* !posix_unlink_USE_STUB */
	result = posix__symlink_f_impl(text, path);
#ifndef posix_unlink_USE_STUB
	if (!(flags & RENAME_NOREPLACE) && result == NULL) {
		if (DeeError_Catch(&DeeError_FileExists)) {
			/* Delete the file and try again. */
			result = posix_unlink_f_impl(path);
			if (result) {
				Dee_Decref(result);
				goto again;
			}

			/* Handle race condition where the file was deleted due to an outside force. */
			if (DeeError_Catch(&DeeError_FileNotFound))
				goto again;
		}
	}
#endif /* !posix_unlink_USE_STUB */
	return result;
}
#endif /* posix_lcopyfile_USE_symlink_(before|after)_open */


/*[[[deemon import("rt.gen.dexutils").gw("lcopyfile",
	"oldpath:?Dstring,newpath:?X3?Dstring?DFile?Dint,flags:u=0,"
	"progress:?DCallable=Dee_None,bufsize:?Dint=Dee_None", libname: "posix"); ]]]*/
#define POSIX_LCOPYFILE_DEF          DEX_MEMBER_F("lcopyfile", &posix_lcopyfile, DEXSYM_READONLY, "(oldpath:?Dstring,newpath:?X3?Dstring?DFile?Dint,flags=!0,progress:?DCallable=!N,bufsize:?Dint=!N)"),
#define POSIX_LCOPYFILE_DEF_DOC(doc) DEX_MEMBER_F("lcopyfile", &posix_lcopyfile, DEXSYM_READONLY, "(oldpath:?Dstring,newpath:?X3?Dstring?DFile?Dint,flags=!0,progress:?DCallable=!N,bufsize:?Dint=!N)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL posix_lcopyfile_f_impl(DeeObject *oldpath, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize);
#ifndef DEFINED_kwlist__oldpath_newpath_flags_progress_bufsize
#define DEFINED_kwlist__oldpath_newpath_flags_progress_bufsize
PRIVATE DEFINE_KWLIST(kwlist__oldpath_newpath_flags_progress_bufsize, { KEX("oldpath", 0x6af2d717, 0x74cfc4ae2e46bac3), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("progress", 0x2b0d4ed7, 0xcace2890c513a747), KEX("bufsize", 0x24b3a7e0, 0x94bf4a0770b058aa), KEND });
#endif /* !DEFINED_kwlist__oldpath_newpath_flags_progress_bufsize */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lcopyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *oldpath;
		DeeObject *newpath;
		unsigned int flags;
		DeeObject *progress;
		DeeObject *bufsize;
	} args;
	args.flags = 0;
	args.progress = Dee_None;
	args.bufsize = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__oldpath_newpath_flags_progress_bufsize, "oo|uoo:lcopyfile", &args))
		goto err;
	return posix_lcopyfile_f_impl(args.oldpath, args.newpath, args.flags, args.progress, args.bufsize);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_lcopyfile, &posix_lcopyfile_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL posix_lcopyfile_f_impl(DeeObject *oldpath, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize)
/*[[[end]]]*/
{
#ifdef posix_lcopyfile_USE_posix_copyfile_fileio
	DREF DeeObject *src_file;
	DREF DeeObject *dst_file;
	unsigned int src_file_mmap_hints = DeeMapFile_F_NORMAL;
	if (flags & ~(RENAME_NOREPLACE)) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}

	/* Open arguments as deemon file objects. */
	if (DeeString_Check(oldpath)) {
		/* Check if `oldpath' refers to a symbolic link. */
#ifdef posix_lcopyfile_USE_symlink_before_open
		if (DeeString_Check(newpath)) {
			DREF DeeObject *symlink_text;
			symlink_text = posix_readlink_f_impl(oldpath);
			if (symlink_text) {
				/* Copy a symbolic link */
				DREF DeeObject *result;
				result = posix__symlink_with_optional_replace(symlink_text, newpath, flags);
				Dee_Decref(symlink_text);
				return result;
			}
			if (!DeeError_Catch(&DeeError_NoSymlink))
				goto err;
			/* Not a symbolic link -> copy as a regular file */
		}
#endif /* posix_lcopyfile_USE_symlink_before_open */

		src_file = DeeFile_Open(oldpath, OPEN_FRDONLY | OPEN_FNOFOLLOW | OPEN_FCLOEXEC, 0);
		if unlikely(src_file == ITER_DONE) {
			DeeError_Throwf(&DeeError_FileNotFound, "File %r could not be found", oldpath);
			goto err;
		}

		/* We opened the source-file, so we can assume that it's file-pointer is at the start. */
		src_file_mmap_hints |= DeeMapFile_F_ATSTART;
	} else {
		src_file = posix_fd_openfile(oldpath, OPEN_FRDONLY | OPEN_FNOFOLLOW | OPEN_FCLOEXEC);
#define NEED_posix_fd_openfile
	}
	if unlikely(!src_file)
		goto err;

	/* Special handling for copying symlinks under windows. */
#ifdef posix_lcopyfile_USE_symlink_after_open
	if (DeeString_Check(newpath) && DeeSystemFile_Check(src_file)) {
		/* Check if this *may* be a symbolic link. */
		DREF DeeObject *src_path;
		DREF DeeObject *symlink_text;
		if (DeeString_Check(oldpath)) {
			src_path = oldpath;
			Dee_Incref(src_path);
		} else {
			src_path = DeeFile_Filename(src_file);
			if unlikely(!src_path)
				goto err_src_file;
		}
		symlink_text = nt_FReadLink(DeeSystemFile_GetHandle(src_file), src_path, false);
		Dee_Decref(src_path);
		if (symlink_text != ITER_DONE) {
			DREF DeeObject *result;
			Dee_Decref(src_file);
			if unlikely(!symlink_text)
				goto err;
			/* Copy a symbolic link */
			result = posix__symlink_with_optional_replace(symlink_text, newpath, flags);
			Dee_Decref(symlink_text);
			return result;
		}
	}
#endif /* posix_lcopyfile_USE_symlink_after_open */

	/* Load `newpath' as a deemon file object. */
	if (DeeString_Check(newpath)) {
		if (flags & RENAME_NOREPLACE) {
			dst_file = DeeFile_Open(newpath, OPEN_FWRONLY | OPEN_FCREAT | OPEN_FTRUNC | OPEN_FEXCL | OPEN_FCLOEXEC, 0644);
			if unlikely(dst_file == ITER_DONE) {
				DeeError_Throwf(&DeeError_FileExists, "File %r already exists", newpath);
				goto err_src_file;
			}
		} else {
			dst_file = DeeFile_Open(newpath, OPEN_FWRONLY | OPEN_FCREAT | OPEN_FTRUNC | OPEN_FCLOEXEC, 0644);
			ASSERT(dst_file != ITER_DONE);
		}
	} else {
		dst_file = posix_fd_openfile(newpath, OPEN_FWRONLY | OPEN_FCLOEXEC);
#define NEED_posix_fd_openfile
	}
	if unlikely(!dst_file)
		goto err_src_file;

	/* Do the actual copy. */
	if unlikely(posix_copyfile_fileio(src_file, dst_file, progress,
	                                  bufsize, src_file_mmap_hints))
		goto err_src_file_dst_file;
#define NEED_posix_copyfile_fileio

	/* Cleanup & indicate success. */
	Dee_Decref(dst_file);
	Dee_Decref(src_file);
	return_none;
err_src_file_dst_file:
	Dee_Decref(dst_file);
err_src_file:
	Dee_Decref(src_file);
err:
	return NULL;
#endif /* posix_lcopyfile_USE_posix_copyfile_fileio */

#ifdef posix_lcopyfile_USE_STUB
	(void)oldpath;
	(void)newpath;
	(void)progress;
	(void)bufsize;
#define NEED_posix_err_unsupported
	posix_err_unsupported("lcopyfile");
	return NULL;
#endif /* posix_lcopyfile_USE_STUB */
}



/*[[[deemon import("rt.gen.dexutils").gw("copyfileat",
	"olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,"
	"newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,flags:u=0,"
	"atflags:u=0,progress:?DCallable=Dee_None,bufsize:?Dint=Dee_None", libname: "posix");]]]*/
#define POSIX_COPYFILEAT_DEF          DEX_MEMBER_F("copyfileat", &posix_copyfileat, DEXSYM_READONLY, "(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,flags=!0,atflags=!0,progress:?DCallable=!N,bufsize:?Dint=!N)"),
#define POSIX_COPYFILEAT_DEF_DOC(doc) DEX_MEMBER_F("copyfileat", &posix_copyfileat, DEXSYM_READONLY, "(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,flags=!0,atflags=!0,progress:?DCallable=!N,bufsize:?Dint=!N)\n" doc),
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 7, 8)) DREF DeeObject *DCALL posix_copyfileat_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int flags, unsigned int atflags, DeeObject *progress, DeeObject *bufsize);
#ifndef DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags_progress_bufsize
#define DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags_progress_bufsize
PRIVATE DEFINE_KWLIST(kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags_progress_bufsize, { KEX("olddirfd", 0xfce5716b, 0x69852ead3adcc550), KEX("oldpath", 0x6af2d717, 0x74cfc4ae2e46bac3), KEX("newdirfd", 0xcef3d13f, 0x767804c5c500a418), KEX("newpath", 0x1e4b25cf, 0x18c3eb62ffd9a6ce), KEX("flags", 0xd9e40622, 0x6afda85728fae70d), KEX("atflags", 0x250a5b0d, 0x79142af6dc89e37c), KEX("progress", 0x2b0d4ed7, 0xcace2890c513a747), KEX("bufsize", 0x24b3a7e0, 0x94bf4a0770b058aa), KEND });
#endif /* !DEFINED_kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags_progress_bufsize */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_copyfileat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct {
		DeeObject *olddirfd;
		DeeObject *oldpath;
		DeeObject *newdirfd;
		DeeObject *newpath;
		unsigned int flags;
		unsigned int atflags;
		DeeObject *progress;
		DeeObject *bufsize;
	} args;
	args.flags = 0;
	args.atflags = 0;
	args.progress = Dee_None;
	args.bufsize = Dee_None;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__olddirfd_oldpath_newdirfd_newpath_flags_atflags_progress_bufsize, "oooo|uuoo:copyfileat", &args))
		goto err;
	return posix_copyfileat_f_impl(args.olddirfd, args.oldpath, args.newdirfd, args.newpath, args.flags, args.atflags, args.progress, args.bufsize);
err:
	return NULL;
}
PRIVATE DEFINE_KWCMETHOD(posix_copyfileat, &posix_copyfileat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1, 2, 3, 4, 7, 8)) DREF DeeObject *DCALL posix_copyfileat_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int flags, unsigned int atflags, DeeObject *progress, DeeObject *bufsize)
/*[[[end]]]*/
{
#ifdef posix_copyfileat_USE_copyfile__AND__lcopyfile__AND__fcopyfile
	DREF DeeObject *abs_newpath, *result;
	if (atflags & ~(AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW |
	                POSIX_DFD_MAKEPATH_ATFLAGS_MASK)) {
#define NEED_err_bad_atflags
		err_bad_atflags(atflags);
		goto err;
	}
	/* TODO: Support for `openat()' to open files without needing to expand paths first! */
	/* TODO: Support for `readlinkat()' to check for symlink copy without needing to expand paths first! */

	abs_newpath = posix_dfd_makepath(newdirfd, newpath, atflags & POSIX_DFD_MAKEPATH_ATFLAGS_MASK);
	if unlikely(!abs_newpath)
		goto err;
	if ((atflags & AT_EMPTY_PATH) &&
	    (DeeNone_Check(oldpath) || (DeeString_Check(oldpath) &&
	                                DeeString_IsEmpty(oldpath)))) {
		result = posix_fcopyfile_f_impl(olddirfd, abs_newpath, flags, progress, bufsize);
	} else {
		DREF DeeObject *abs_oldpath;
		abs_oldpath = posix_dfd_makepath(olddirfd, oldpath, atflags & POSIX_DFD_MAKEPATH_ATFLAGS_MASK);
		if unlikely(!abs_oldpath)
			goto err_abs_newpath;
		if (atflags & AT_SYMLINK_NOFOLLOW) {
			result = posix_lcopyfile_f_impl(abs_oldpath, abs_newpath, flags, progress, bufsize);
		} else {
			result = posix_copyfile_f_impl(abs_oldpath, abs_newpath, flags, progress, bufsize);
		}
		Dee_Decref(abs_oldpath);
	}
	Dee_Decref(abs_newpath);
	return result;
err_abs_newpath:
	Dee_Decref(abs_newpath);
err:
	return NULL;
#endif /* posix_copyfileat_USE_copyfile__AND__lcopyfile__AND__fcopyfile */

#ifdef posix_copyfileat_USE_STUB
	(void)olddirfd;
	(void)oldpath;
	(void)newdirfd;
	(void)newpath;
	(void)atflags;
	(void)progress;
	(void)bufsize;
#define NEED_posix_err_unsupported
	posix_err_unsupported("copyfileat");
	return NULL;
#endif /* posix_copyfileat_USE_STUB */
}






/************************************************************************/
/* CopyFileProgress                                                     */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
copyfile_progress_copy(DeeCopyFileProgressObject *__restrict self,
                       DeeCopyFileProgressObject *__restrict other) {
	self->cfp_srcfile = other->cfp_srcfile;
	self->cfp_dstfile = other->cfp_dstfile;
	self->cfp_bufsize = other->cfp_bufsize;
	self->cfp_copied  = other->cfp_copied;
	self->cfp_total   = other->cfp_total;
	Dee_Incref(self->cfp_srcfile);
	Dee_Incref(self->cfp_dstfile);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
copyfile_progress_serialize(DeeCopyFileProgressObject *__restrict self,
                            DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DeeCopyFileProgressObject, field))
	DeeCopyFileProgressObject *out = DeeSerial_Addr2Mem(writer, addr, DeeCopyFileProgressObject);
	out->cfp_copied  = self->cfp_copied;
	out->cfp_total   = self->cfp_total;
	out->cfp_bufsize = self->cfp_bufsize;
	if (DeeSerial_PutObject(writer, ADDROF(cfp_srcfile), self->cfp_srcfile))
		goto err;
	return DeeSerial_PutObject(writer, ADDROF(cfp_dstfile), self->cfp_dstfile);
err:
	return -1;
#undef ADDROF
}


/*[[[deemon (print_DEFINE_KWLIST from rt.gen.unpack)({ "srcfile", "dstfile", "copied", "total", "bufsize" });]]]*/
#ifndef DEFINED_kwlist__srcfile_dstfile_copied_total_bufsize
#define DEFINED_kwlist__srcfile_dstfile_copied_total_bufsize
PRIVATE DEFINE_KWLIST(kwlist__srcfile_dstfile_copied_total_bufsize, { KEX("srcfile", 0xe0c1a055, 0x9196764c7ba30338), KEX("dstfile", 0x6c1ba93a, 0xd0ccc50d75a5d500), KEX("copied", 0xaa579ad7, 0x7c66efa2308d3e42), KEX("total", 0x48601e35, 0xd63cdba353c9c786), KEX("bufsize", 0x24b3a7e0, 0x94bf4a0770b058aa), KEND });
#endif /* !DEFINED_kwlist__srcfile_dstfile_copied_total_bufsize */
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) int DCALL
copyfile_progress_init_kw(DeeCopyFileProgressObject *__restrict self,
                          size_t argc, DeeObject *const *argv, DeeObject *kw) {
#if 1
	struct _layout {
		DeeObject *srcfile;
		DeeObject *dstfile;
		uint64_t copied;
		uint64_t total;
		size_t bufsize;
	};
#define RELBASE cfp_srcfile
	STATIC_ASSERT((COMPILER_OFFSETOF(DeeCopyFileProgressObject, cfp_srcfile) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETOF(struct _layout, srcfile));
	STATIC_ASSERT((COMPILER_OFFSETAFTER(DeeCopyFileProgressObject, cfp_srcfile) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETAFTER(struct _layout, srcfile));
	STATIC_ASSERT((COMPILER_OFFSETOF(DeeCopyFileProgressObject, cfp_dstfile) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETOF(struct _layout, dstfile));
	STATIC_ASSERT((COMPILER_OFFSETAFTER(DeeCopyFileProgressObject, cfp_dstfile) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETAFTER(struct _layout, dstfile));
	STATIC_ASSERT((COMPILER_OFFSETOF(DeeCopyFileProgressObject, cfp_copied) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETOF(struct _layout, copied));
	STATIC_ASSERT((COMPILER_OFFSETAFTER(DeeCopyFileProgressObject, cfp_copied) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETAFTER(struct _layout, copied));
	STATIC_ASSERT((COMPILER_OFFSETOF(DeeCopyFileProgressObject, cfp_total) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETOF(struct _layout, total));
	STATIC_ASSERT((COMPILER_OFFSETAFTER(DeeCopyFileProgressObject, cfp_total) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETAFTER(struct _layout, total));
	STATIC_ASSERT((COMPILER_OFFSETOF(DeeCopyFileProgressObject, cfp_bufsize) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETOF(struct _layout, bufsize));
	STATIC_ASSERT((COMPILER_OFFSETAFTER(DeeCopyFileProgressObject, cfp_bufsize) - offsetof(DeeCopyFileProgressObject, RELBASE)) == COMPILER_OFFSETAFTER(struct _layout, bufsize));
	self->cfp_copied  = 0;
	self->cfp_total   = (uint64_t)-1;
	self->cfp_bufsize = POSIX_COPYFILE_DEFAULT_IO_BUFSIZE;
#define posix_CopyFileProgress_params "srcfile:?DFile,dstfile:?DFile,copied=!0,total?:?Dint,bufsize?:?Dint"
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__srcfile_dstfile_copied_total_bufsize,
	                          "oo|" UNPu64 UNPx64 UNPuSIZ ":CopyFileProgress", &self->RELBASE))
		goto err;
#undef RELBASE
#else
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("CopyFileProgress", params: "
	DeeObject *srcfile:?DFile,
	DeeObject *dstfile:?DFile,
	uint64_t   copied = 0,
	uint64_t   total? = (uint64_t)-1,
	size_t     bufsize? = POSIX_COPYFILE_DEFAULT_IO_BUFSIZE,
", docStringPrefix: "posix");]]]*/
#define posix_CopyFileProgress_params "srcfile:?DFile,dstfile:?DFile,copied=!0,total?:?Dint,bufsize?:?Dint"
	struct {
		DeeObject *srcfile;
		DeeObject *dstfile;
		uint64_t copied;
		uint64_t total;
		size_t bufsize;
	} args;
	args.copied = 0;
	args.total = (uint64_t)-1;
	args.bufsize = POSIX_COPYFILE_DEFAULT_IO_BUFSIZE;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__srcfile_dstfile_copied_total_bufsize, "oo|" UNPu64 UNPx64 UNPuSIZ ":CopyFileProgress", &args))
		goto err;
/*[[[end]]]*/
	self->cfp_srcfile = args.srcfile;
	self->cfp_dstfile = args.dstfile;
	self->cfp_copied  = args.copied;
	self->cfp_total   = args.total;
	self->cfp_bufsize = args.bufsize;
#endif
	if unlikely(self->cfp_bufsize == 0) {
		err_bad_copyfile_bufsize_is_zero();
#define NEED_err_bad_copyfile_bufsize_is_zero
		goto err;
	}
	Dee_Incref(self->cfp_srcfile);
	Dee_Incref(self->cfp_dstfile);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
copyfile_progress_fini(DeeCopyFileProgressObject *__restrict self) {
	Dee_Decref(self->cfp_srcfile);
	Dee_Decref(self->cfp_dstfile);
}

PRIVATE NONNULL((1, 2)) void DCALL
copyfile_progress_visit(DeeCopyFileProgressObject *__restrict self,
                        Dee_visit_t proc, void *arg) {
	Dee_Visit(self->cfp_srcfile);
	Dee_Visit(self->cfp_dstfile);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
copyfile_progress_total_get(DeeCopyFileProgressObject *__restrict self) {
	uint64_t total = self->cfp_total;
	return DeeInt_NewUInt64(total);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
copyfile_progress_total_del(DeeCopyFileProgressObject *__restrict self) {
	self->cfp_total = (uint64_t)-1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
copyfile_progress_total_set(DeeCopyFileProgressObject *self,
                            DeeObject *value) {
	int result;
	uint64_t new_total;
	result = DeeObject_AsUInt64(value, &new_total);
	if likely(result == 0)
		self->cfp_total = new_total;
	return result;
}

PRIVATE struct type_member tpconst copyfile_progress_members[] = {
	TYPE_MEMBER_FIELD_DOC("srcfile", STRUCT_OBJECT_AB, offsetof(DeeCopyFileProgressObject, cfp_srcfile),
	                      "->?DFile\n"
	                      "The source file from which data is being read"),
	TYPE_MEMBER_FIELD_DOC("dstfile", STRUCT_OBJECT_AB, offsetof(DeeCopyFileProgressObject, cfp_dstfile),
	                      "->?DFile\n"
	                      "The destination file to which data is being written"),
	TYPE_MEMBER_FIELD_DOC("bufsize", STRUCT_SIZE_T, offsetof(DeeCopyFileProgressObject, cfp_bufsize),
	                      "Currently used transfer buffer size"),
	TYPE_MEMBER_FIELD_DOC("copied", STRUCT_UINT64_T, offsetof(DeeCopyFileProgressObject, cfp_copied),
	                      "Number of bytes that have already been copied"),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst copyfile_progress_getsets[] = {
	TYPE_GETSET_F("total",
	              &copyfile_progress_total_get,
	              &copyfile_progress_total_del,
	              &copyfile_progress_total_set,
	              METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Total total number of bytes that need to be copied"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeCopyFileProgress_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "CopyFileProgress",
	/* .tp_doc      = */ DOC("The type of object that is passed to the $progress argument of "
	                         /**/ "?Gcopyfile, ?Gcopyfileat, ?Gfcopyfile and ?Glcopyfile\n"
	                         "\n"
	                         "(" posix_CopyFileProgress_params ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeCopyFileProgressObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &copyfile_progress_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &copyfile_progress_init_kw,
			/* tp_serialize:   */ &copyfile_progress_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&copyfile_progress_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&copyfile_progress_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ copyfile_progress_getsets,
	/* .tp_members       = */ copyfile_progress_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEX_POSIX_P_COPYFILE_C_INL */
