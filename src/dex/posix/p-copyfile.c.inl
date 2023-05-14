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
#ifndef GUARD_DEX_POSIX_P_COPYFILE_C_INL
#define GUARD_DEX_POSIX_P_COPYFILE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX
#define DEE_SOURCE

#include "libposix.h"
/**/

#include <deemon/alloc.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/object.h>

/* Include posix dependencies */
#include "p-readlink.c.inl" /* Needed for `lcopyfile()' to check for symbolic links. */
#include "p-remove.c.inl"   /* Needed for `lcopyfile()' to remove existing files when `RENAME_NOREPLACE' isn't given. */
#include "p-symlink.c.inl"  /* Needed for `lcopyfile()' to create copies of symbolic links. */

DECL_BEGIN



/* Figure out how to implement `fcopyfile()' */
#undef posix_fcopyfile_USE_posix_copyfile_fileio
#undef posix_fcopyfile_USE_STUB
#if !defined(DEESYSTEM_FILE_USE_STUB)
#define posix_fcopyfile_USE_posix_copyfile_fileio
#else /* ... */
#define posix_fcopyfile_USE_STUB
#endif /* !... */

/* Figure out how to implement `copyfile()' */
#undef posix_copyfile_USE_posix_copyfile_fileio
#undef posix_copyfile_USE_STUB
#if !defined(DEESYSTEM_FILE_USE_STUB)
#define posix_copyfile_USE_posix_copyfile_fileio
#else /* ... */
#define posix_copyfile_USE_STUB
#endif /* !... */

/* Figure out how to implement `lcopyfile()' */
#undef posix_lcopyfile_USE_posix_copyfile_fileio
#undef posix_lcopyfile_USE_STUB
#if !defined(DEESYSTEM_FILE_USE_STUB)
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fcopyfile_f_impl(DeeObject *oldfd, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fcopyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FCOPYFILE_DEF { "fcopyfile", (DeeObject *)&posix_fcopyfile, MODSYM_FNORMAL, DOC("(oldfd:?X2?DFile?Dint,newpath:?X3?Dstring?DFile?Dint,flags:?Dint=!0,progress:?DCallable=!N,bufsize:?Dint=!N)") },
#define POSIX_FCOPYFILE_DEF_DOC(doc) { "fcopyfile", (DeeObject *)&posix_fcopyfile, MODSYM_FNORMAL, DOC("(oldfd:?X2?DFile?Dint,newpath:?X3?Dstring?DFile?Dint,flags:?Dint=!0,progress:?DCallable=!N,bufsize:?Dint=!N)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_fcopyfile, &posix_fcopyfile_f);
#ifndef POSIX_KWDS_OLDFD_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED
#define POSIX_KWDS_OLDFD_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_oldfd_newpath_flags_progress_bufsize, { K(oldfd), K(newpath), K(flags), K(progress), K(bufsize), KEND });
#endif /* !POSIX_KWDS_OLDFD_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_fcopyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *oldfd;
	DeeObject *newpath;
	unsigned int flags = 0;
	DeeObject *progress = Dee_None;
	DeeObject *bufsize = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldfd_newpath_flags_progress_bufsize, "oo|uoo:fcopyfile", &oldfd, &newpath, &flags, &progress, &bufsize))
		goto err;
	return posix_fcopyfile_f_impl(oldfd, newpath, flags, progress, bufsize);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_fcopyfile_f_impl(DeeObject *oldfd, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize)
/*[[[end]]]*/
{
#ifdef posix_fcopyfile_USE_posix_copyfile_fileio
	DREF DeeObject *src_file;
	DREF DeeObject *dst_file;
	if (DeeObject_AssertTypeExact(newpath, &DeeString_Type))
		goto err;
	if (flags & ~(RENAME_NOREPLACE)) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}

	/* Open arguments as deemon file objects. */
	src_file = posix_fd_openfile(oldfd, OPEN_FRDONLY);
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
		dst_file = posix_fd_openfile(newpath, OPEN_FWRONLY);
#define NEED_posix_fd_openfile
	}
	if unlikely(!dst_file)
		goto err_src_file;

	/* Do the actual copy. */
	if unlikely(posix_copyfile_fileio(src_file, dst_file, progress, bufsize))
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_copyfile_f_impl(DeeObject *oldpath, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_copyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_COPYFILE_DEF { "copyfile", (DeeObject *)&posix_copyfile, MODSYM_FNORMAL, DOC("(oldpath:?X3?DFile?Dint?Dstring,newpath:?X3?Dstring?DFile?Dint,flags:?Dint=!0,progress:?DCallable=!N,bufsize:?Dint=!N)") },
#define POSIX_COPYFILE_DEF_DOC(doc) { "copyfile", (DeeObject *)&posix_copyfile, MODSYM_FNORMAL, DOC("(oldpath:?X3?DFile?Dint?Dstring,newpath:?X3?Dstring?DFile?Dint,flags:?Dint=!0,progress:?DCallable=!N,bufsize:?Dint=!N)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_copyfile, &posix_copyfile_f);
#ifndef POSIX_KWDS_OLDPATH_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED
#define POSIX_KWDS_OLDPATH_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_oldpath_newpath_flags_progress_bufsize, { K(oldpath), K(newpath), K(flags), K(progress), K(bufsize), KEND });
#endif /* !POSIX_KWDS_OLDPATH_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_copyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *oldpath;
	DeeObject *newpath;
	unsigned int flags = 0;
	DeeObject *progress = Dee_None;
	DeeObject *bufsize = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldpath_newpath_flags_progress_bufsize, "oo|uoo:copyfile", &oldpath, &newpath, &flags, &progress, &bufsize))
		goto err;
	return posix_copyfile_f_impl(oldpath, newpath, flags, progress, bufsize);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_copyfile_f_impl(DeeObject *oldpath, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize)
/*[[[end]]]*/
{
#ifdef posix_copyfile_USE_posix_copyfile_fileio
	DREF DeeObject *src_file;
	DREF DeeObject *dst_file;
	if (DeeObject_AssertTypeExact(newpath, &DeeString_Type))
		goto err;
	if (flags & ~(RENAME_NOREPLACE)) {
		DeeError_Throwf(&DeeError_ValueError, "Invalid flags argument %#x", flags);
		goto err;
	}

	/* Open arguments as deemon file objects. */
	if (DeeString_Check(oldpath)) {
		src_file = DeeFile_Open(oldpath, OPEN_FRDONLY, 0);
		if unlikely(src_file == ITER_DONE) {
			DeeError_Throwf(&DeeError_FileNotFound, "File %r could not be found", oldpath);
			goto err_src_file;
		}
	} else {
		src_file = posix_fd_openfile(oldpath, OPEN_FRDONLY);
#define NEED_posix_fd_openfile
	}
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
		dst_file = posix_fd_openfile(newpath, OPEN_FWRONLY);
#define NEED_posix_fd_openfile
	}
	if unlikely(!dst_file)
		goto err_src_file;

	/* Do the actual copy. */
	if unlikely(posix_copyfile_fileio(src_file, dst_file, progress, bufsize))
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
#if (defined(DEESYSTEM_FILE_USE_WINDOWS) && \
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lcopyfile_f_impl(DeeObject *oldpath, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lcopyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_LCOPYFILE_DEF { "lcopyfile", (DeeObject *)&posix_lcopyfile, MODSYM_FNORMAL, DOC("(oldpath:?Dstring,newpath:?X3?Dstring?DFile?Dint,flags:?Dint=!0,progress:?DCallable=!N,bufsize:?Dint=!N)") },
#define POSIX_LCOPYFILE_DEF_DOC(doc) { "lcopyfile", (DeeObject *)&posix_lcopyfile, MODSYM_FNORMAL, DOC("(oldpath:?Dstring,newpath:?X3?Dstring?DFile?Dint,flags:?Dint=!0,progress:?DCallable=!N,bufsize:?Dint=!N)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lcopyfile, &posix_lcopyfile_f);
#ifndef POSIX_KWDS_OLDPATH_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED
#define POSIX_KWDS_OLDPATH_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_oldpath_newpath_flags_progress_bufsize, { K(oldpath), K(newpath), K(flags), K(progress), K(bufsize), KEND });
#endif /* !POSIX_KWDS_OLDPATH_NEWPATH_FLAGS_PROGRESS_BUFSIZE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lcopyfile_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *oldpath;
	DeeObject *newpath;
	unsigned int flags = 0;
	DeeObject *progress = Dee_None;
	DeeObject *bufsize = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_oldpath_newpath_flags_progress_bufsize, "oo|uoo:lcopyfile", &oldpath, &newpath, &flags, &progress, &bufsize))
		goto err;
	return posix_lcopyfile_f_impl(oldpath, newpath, flags, progress, bufsize);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lcopyfile_f_impl(DeeObject *oldpath, DeeObject *newpath, unsigned int flags, DeeObject *progress, DeeObject *bufsize)
/*[[[end]]]*/
{
#ifdef posix_lcopyfile_USE_posix_copyfile_fileio
	DREF DeeObject *src_file;
	DREF DeeObject *dst_file;
	if (DeeObject_AssertTypeExact(newpath, &DeeString_Type))
		goto err;
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

		src_file = DeeFile_Open(oldpath, OPEN_FRDONLY | OPEN_FNOFOLLOW, 0);
		if unlikely(src_file == ITER_DONE) {
			DeeError_Throwf(&DeeError_FileNotFound, "File %r could not be found", oldpath);
			goto err_src_file;
		}
	} else {
		src_file = posix_fd_openfile(oldpath, OPEN_FRDONLY | OPEN_FNOFOLLOW);
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
		dst_file = posix_fd_openfile(newpath, OPEN_FWRONLY);
#define NEED_posix_fd_openfile
	}
	if unlikely(!dst_file)
		goto err_src_file;

	/* Do the actual copy. */
	if unlikely(posix_copyfile_fileio(src_file, dst_file, progress, bufsize))
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
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_copyfileat_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int flags, unsigned int atflags, DeeObject *progress, DeeObject *bufsize);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_copyfileat_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_COPYFILEAT_DEF { "copyfileat", (DeeObject *)&posix_copyfileat, MODSYM_FNORMAL, DOC("(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,flags:?Dint=!0,atflags:?Dint=!0,progress:?DCallable=!N,bufsize:?Dint=!N)") },
#define POSIX_COPYFILEAT_DEF_DOC(doc) { "copyfileat", (DeeObject *)&posix_copyfileat, MODSYM_FNORMAL, DOC("(olddirfd:?X3?DFile?Dint?Dstring,oldpath:?Dstring,newdirfd:?X3?DFile?Dint?Dstring,newpath:?Dstring,flags:?Dint=!0,atflags:?Dint=!0,progress:?DCallable=!N,bufsize:?Dint=!N)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_copyfileat, &posix_copyfileat_f);
#ifndef POSIX_KWDS_OLDDIRFD_OLDPATH_NEWDIRFD_NEWPATH_FLAGS_ATFLAGS_PROGRESS_BUFSIZE_DEFINED
#define POSIX_KWDS_OLDDIRFD_OLDPATH_NEWDIRFD_NEWPATH_FLAGS_ATFLAGS_PROGRESS_BUFSIZE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_olddirfd_oldpath_newdirfd_newpath_flags_atflags_progress_bufsize, { K(olddirfd), K(oldpath), K(newdirfd), K(newpath), K(flags), K(atflags), K(progress), K(bufsize), KEND });
#endif /* !POSIX_KWDS_OLDDIRFD_OLDPATH_NEWDIRFD_NEWPATH_FLAGS_ATFLAGS_PROGRESS_BUFSIZE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_copyfileat_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *olddirfd;
	DeeObject *oldpath;
	DeeObject *newdirfd;
	DeeObject *newpath;
	unsigned int flags = 0;
	unsigned int atflags = 0;
	DeeObject *progress = Dee_None;
	DeeObject *bufsize = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_olddirfd_oldpath_newdirfd_newpath_flags_atflags_progress_bufsize, "oooo|uuoo:copyfileat", &olddirfd, &oldpath, &newdirfd, &newpath, &flags, &atflags, &progress, &bufsize))
		goto err;
	return posix_copyfileat_f_impl(olddirfd, oldpath, newdirfd, newpath, flags, atflags, progress, bufsize);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_copyfileat_f_impl(DeeObject *olddirfd, DeeObject *oldpath, DeeObject *newdirfd, DeeObject *newpath, unsigned int flags, unsigned int atflags, DeeObject *progress, DeeObject *bufsize)
/*[[[end]]]*/
{
#ifdef posix_copyfileat_USE_copyfile__AND__lcopyfile__AND__fcopyfile
	DREF DeeObject *abs_newpath, *result;
	if (atflags & ~(AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW |
	                POSIX_DFD_ABSPATH_ATFLAGS_MASK)) {
#define NEED_err_bad_atflags
		err_bad_atflags(atflags);
		goto err;
	}
	/* TODO: Support for `openat()' to open files without needing to expand paths first! */
	/* TODO: Support for `readlinkat()' to check for symlink copy without needing to expand paths first! */

	abs_newpath = posix_dfd_abspath(newdirfd, newpath, atflags & POSIX_DFD_ABSPATH_ATFLAGS_MASK);
	if unlikely(!abs_newpath)
		goto err;
	if ((atflags & AT_EMPTY_PATH) &&
	    (DeeNone_Check(oldpath) || (DeeString_Check(oldpath) &&
	                                DeeString_IsEmpty(oldpath)))) {
		result = posix_fcopyfile_f_impl(olddirfd, abs_newpath, flags, progress, bufsize);
	} else {
		DREF DeeObject *abs_oldpath;
		abs_oldpath = posix_dfd_abspath(olddirfd, oldpath, atflags & POSIX_DFD_ABSPATH_ATFLAGS_MASK);
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

PRIVATE NONNULL((1, 2)) int DCALL
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

PRIVATE DEFINE_KWLIST(copyfile_progress_init_kwlist, {
	K(srcfile), K(dstfile), K(copied), K(total), KEND });

PRIVATE NONNULL((1)) int DCALL
copyfile_progress_init_kw(DeeCopyFileProgressObject *__restrict self,
                          size_t argc, DeeObject *const *argv, DeeObject *kw) {
	self->cfp_copied  = 0;
	self->cfp_total   = (uint64_t)-1;
	self->cfp_bufsize = POSIX_COPYFILE_DEFAULT_BUFSIZE;
	if (DeeArg_UnpackKw(argc, argv, kw, copyfile_progress_init_kwlist,
	                    "oo|" UNPu64 UNPu64 UNPuSIZ ":attribute",
	                    &self->cfp_srcfile,
	                    &self->cfp_dstfile,
	                    &self->cfp_copied,
	                    &self->cfp_total,
	                    &self->cfp_bufsize))
		goto err;
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
                        dvisit_t proc, void *arg) {
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
	TYPE_MEMBER_FIELD_DOC("srcfile", STRUCT_OBJECT, offsetof(DeeCopyFileProgressObject, cfp_srcfile),
	                      "->?DFile\n"
	                      "The source file from which data is being read"),
	TYPE_MEMBER_FIELD_DOC("dstfile", STRUCT_OBJECT, offsetof(DeeCopyFileProgressObject, cfp_dstfile),
	                      "->?DFile\n"
	                      "The destination file to which data is being written"),
	TYPE_MEMBER_FIELD_DOC("bufsize", STRUCT_SIZE_T, offsetof(DeeCopyFileProgressObject, cfp_bufsize),
	                      "Currently used transfer buffer size"),
	TYPE_MEMBER_FIELD_DOC("copied", STRUCT_UINT64_T, offsetof(DeeCopyFileProgressObject, cfp_copied),
	                      "Number of bytes that have already been copied"),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst copyfile_progress_getsets[] = {
	TYPE_GETSET("total",
	            &copyfile_progress_total_get,
	            &copyfile_progress_total_del,
	            &copyfile_progress_total_set,
	            "->?Dint\n"
	            "Total total number of bytes that need to be copied"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DeeCopyFileProgress_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "CopyFileProgress",
	/* .tp_doc      = */ DOC("(srcfile:?DFile,dstfile:?DFile,copied=!0,total?:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&copyfile_progress_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeCopyFileProgressObject),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&copyfile_progress_init_kw,

			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&copyfile_progress_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&copyfile_progress_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
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
