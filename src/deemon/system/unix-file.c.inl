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
#ifndef GUARD_DEEMON_SYSTEM_UNIX_FILE_C_INL
#define GUARD_DEEMON_SYSTEM_UNIX_FILE_C_INL 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>

#include <hybrid/minmax.h>

#include <string.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeSystemFileObject SystemFile;

PUBLIC WUNUSED DREF /*SystemFile*/ DeeObject *DCALL
DeeFile_OpenFd(DeeSysFD fd,
               /*String*/ DeeObject *filename,
               int UNUSED(oflags), bool inherit_fd) {
	SystemFile *result;
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto done;
	result->sf_handle    = fd;
	result->sf_ownhandle = inherit_fd ? fd : (DeeSysFD)-1; /* Inherit. */
	result->sf_filename  = filename;
	Dee_XIncref(filename);
	DeeLFileObject_Init(result, &DeeSystemFile_Type);
done:
	return (DREF DeeObject *)result;
}

PRIVATE ATTR_COLD int DCALL err_file_closed(void) {
	return DeeError_Throwf(&DeeError_FileClosed,
	                       "File was closed");
}

PRIVATE ATTR_COLD int DCALL error_file_io(SystemFile *__restrict self) {
	if (self->sf_handle == (DeeSysFD)-1)
		return err_file_closed();
	return DeeUnixSystem_ThrowErrorf(&DeeError_FSError,
	                                 DeeSystem_GetErrno(),
	                                 "I/O Operation failed");
}

INTERN DeeSysFD DCALL
DeeSystemFile_Fileno(/*FileSystem*/ DeeObject *__restrict self) {
	DeeSysFD result;
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
	result = (DeeSysFD)((SystemFile *)self)->sf_handle;
	if (result == (DeeSysFD)-1)
		error_file_io((SystemFile *)self);
	return result;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSystemFile_Filename(/*SystemFile*/ DeeObject *__restrict self) {
	SystemFile *me = (SystemFile *)self;
	DREF DeeObject *result;
	ASSERT_OBJECT_TYPE(self, (DeeTypeObject *)&DeeSystemFile_Type);
again:
	result = me->sf_filename;
	if (result)
		Dee_Incref(result);
	else {
		int fd = (int)me->sf_handle;
		if unlikely(fd < 0) {
			err_file_closed();
			goto done;
		}
		result = DeeSystem_GetFilenameOfFD(fd);
		if unlikely(!result)
			goto done;
		/* Lazily cache the generated filename. */
		DeeFile_LockWrite(me);
		if (me->sf_filename) {
			DREF DeeObject *new_result;
			new_result = me->sf_filename;
			Dee_Incref(new_result);
			DeeFile_LockEndWrite(me);
			Dee_Decref(result);
			result = new_result;
		} else {
			/* Make sure that the fd is still the same. */
			if likely((int)me->sf_handle == fd) {
				Dee_Incref(result);
				me->sf_filename = result;
				DeeFile_LockEndWrite(me);
			} else {
				/* Different fd... */
				DeeFile_LockEndWrite(me);
				Dee_Decref(result);
				goto again;
			}
		}
	}
done:
	return result;
}

/* Fix names of aliasing flags. */
#if !defined(O_CLOEXEC) && defined(_O_NOINHERIT)
#define O_CLOEXEC  _O_NOINHERIT
#endif


#if !defined(O_CREAT)
#error "Missing system support for `O_CREAT'"
#endif
#if !defined(O_EXCL)
#error "Missing system support for `O_EXCL'"
#endif
#if !defined(O_TRUNC)
#error "Missing system support for `O_TRUNC'"
#endif
#if !defined(O_APPEND)
#error "Missing system support for `O_APPEND'"
#endif

#define PRIVATE_SHARED_FLAGS_0   0
#define PRIVATE_NOSUPP_FLAGS_0   0
#define PRIVATE_NOSHAR_FLAGS_0   0

#if OPEN_FCREAT == O_CREAT
#define PRIVATE_NOSHAR_FLAGS_1   PRIVATE_NOSHAR_FLAGS_0
#define PRIVATE_SHARED_FLAGS_1   PRIVATE_SHARED_FLAGS_0 | OPEN_FCREAT
#else
#define PRIVATE_NOSHAR_FLAGS_1   PRIVATE_NOSHAR_FLAGS_0 | OPEN_FCREAT
#define PRIVATE_SHARED_FLAGS_1   PRIVATE_SHARED_FLAGS_0
#endif
#if OPEN_FEXCL == O_EXCL
#define PRIVATE_NOSHAR_FLAGS_2   PRIVATE_NOSHAR_FLAGS_1
#define PRIVATE_SHARED_FLAGS_2   PRIVATE_SHARED_FLAGS_1 | OPEN_FEXCL
#else
#define PRIVATE_NOSHAR_FLAGS_2   PRIVATE_NOSHAR_FLAGS_1 | OPEN_FEXCL
#define PRIVATE_SHARED_FLAGS_2   PRIVATE_SHARED_FLAGS_1
#endif
#if OPEN_FTRUNC == O_TRUNC
#define PRIVATE_NOSHAR_FLAGS_3   PRIVATE_NOSHAR_FLAGS_2
#define PRIVATE_SHARED_FLAGS_3   PRIVATE_SHARED_FLAGS_2 | OPEN_FTRUNC
#else
#define PRIVATE_NOSHAR_FLAGS_3   PRIVATE_NOSHAR_FLAGS_2 | OPEN_FTRUNC
#define PRIVATE_SHARED_FLAGS_3   PRIVATE_SHARED_FLAGS_2
#endif
#if OPEN_FAPPEND == O_APPEND
#define PRIVATE_NOSHAR_FLAGS_4   PRIVATE_NOSHAR_FLAGS_3
#define PRIVATE_SHARED_FLAGS_4   PRIVATE_SHARED_FLAGS_3 | OPEN_FAPPEND
#else
#define PRIVATE_NOSHAR_FLAGS_4   PRIVATE_NOSHAR_FLAGS_3 | OPEN_FAPPEND
#define PRIVATE_SHARED_FLAGS_4   PRIVATE_SHARED_FLAGS_3
#endif

/* Optional flags. */
#define PRIVATE_NOSUPP_FLAGS_4   PRIVATE_NOSUPP_FLAGS_0
#ifndef O_NONBLOCK
#define PRIVATE_NOSUPP_FLAGS_5   PRIVATE_NOSUPP_FLAGS_4|OPEN_FNONBLOCK
#define PRIVATE_SHARED_FLAGS_5   PRIVATE_SHARED_FLAGS_4
#define PRIVATE_NOSHAR_FLAGS_5   PRIVATE_NOSHAR_FLAGS_4
#else
#define PRIVATE_NOSUPP_FLAGS_5   PRIVATE_NOSUPP_FLAGS_4
#if OPEN_FNONBLOCK == O_NONBLOCK
#define PRIVATE_NOSHAR_FLAGS_5   PRIVATE_NOSHAR_FLAGS_4
#define PRIVATE_SHARED_FLAGS_5   PRIVATE_SHARED_FLAGS_4|OPEN_FNONBLOCK
#else
#define PRIVATE_NOSHAR_FLAGS_5   PRIVATE_NOSHAR_FLAGS_4|OPEN_FNONBLOCK
#define PRIVATE_SHARED_FLAGS_5   PRIVATE_SHARED_FLAGS_4
#endif
#endif
#ifndef O_SYNC
#define PRIVATE_NOSUPP_FLAGS_6   PRIVATE_NOSUPP_FLAGS_5|OPEN_FSYNC
#define PRIVATE_SHARED_FLAGS_6   PRIVATE_SHARED_FLAGS_5
#define PRIVATE_NOSHAR_FLAGS_6   PRIVATE_NOSHAR_FLAGS_5
#else
#define PRIVATE_NOSUPP_FLAGS_6   PRIVATE_NOSUPP_FLAGS_5
#if OPEN_FSYNC == O_SYNC
#define PRIVATE_NOSHAR_FLAGS_6   PRIVATE_NOSHAR_FLAGS_5
#define PRIVATE_SHARED_FLAGS_6   PRIVATE_SHARED_FLAGS_5|OPEN_FSYNC
#else
#define PRIVATE_NOSHAR_FLAGS_6   PRIVATE_NOSHAR_FLAGS_5|OPEN_FSYNC
#define PRIVATE_SHARED_FLAGS_6   PRIVATE_SHARED_FLAGS_5
#endif
#endif
#ifndef O_DIRECT
#define PRIVATE_NOSUPP_FLAGS_7   PRIVATE_NOSUPP_FLAGS_6|OPEN_FDIRECT
#define PRIVATE_SHARED_FLAGS_7   PRIVATE_SHARED_FLAGS_6
#define PRIVATE_NOSHAR_FLAGS_7   PRIVATE_NOSHAR_FLAGS_6
#else
#define PRIVATE_NOSUPP_FLAGS_7   PRIVATE_NOSUPP_FLAGS_6
#if OPEN_FDIRECT == O_DIRECT
#define PRIVATE_NOSHAR_FLAGS_7   PRIVATE_NOSHAR_FLAGS_6
#define PRIVATE_SHARED_FLAGS_7   PRIVATE_SHARED_FLAGS_6|OPEN_FDIRECT
#else
#define PRIVATE_NOSHAR_FLAGS_7   PRIVATE_NOSHAR_FLAGS_6|OPEN_FDIRECT
#define PRIVATE_SHARED_FLAGS_7   PRIVATE_SHARED_FLAGS_6
#endif
#endif
#ifndef O_NOFOLLOW
#define PRIVATE_NOSUPP_FLAGS_8   PRIVATE_NOSUPP_FLAGS_7|OPEN_FNOFOLLOW
#define PRIVATE_SHARED_FLAGS_8   PRIVATE_SHARED_FLAGS_7
#define PRIVATE_NOSHAR_FLAGS_8   PRIVATE_NOSHAR_FLAGS_7
#else
#define PRIVATE_NOSUPP_FLAGS_8   PRIVATE_NOSUPP_FLAGS_7
#if OPEN_FNOFOLLOW == O_NOFOLLOW
#define PRIVATE_NOSHAR_FLAGS_8   PRIVATE_NOSHAR_FLAGS_7
#define PRIVATE_SHARED_FLAGS_8   PRIVATE_SHARED_FLAGS_7|OPEN_FNOFOLLOW
#else
#define PRIVATE_NOSHAR_FLAGS_8   PRIVATE_NOSHAR_FLAGS_7|OPEN_FNOFOLLOW
#define PRIVATE_SHARED_FLAGS_8   PRIVATE_SHARED_FLAGS_7
#endif
#endif
#ifndef O_NOATIME
#define PRIVATE_NOSUPP_FLAGS_9   PRIVATE_NOSUPP_FLAGS_8|OPEN_FNOATIME
#define PRIVATE_SHARED_FLAGS_9   PRIVATE_SHARED_FLAGS_8
#define PRIVATE_NOSHAR_FLAGS_9   PRIVATE_NOSHAR_FLAGS_8
#else
#define PRIVATE_NOSUPP_FLAGS_9   PRIVATE_NOSUPP_FLAGS_8
#if OPEN_FNOATIME == O_NOATIME
#define PRIVATE_NOSHAR_FLAGS_9   PRIVATE_NOSHAR_FLAGS_8
#define PRIVATE_SHARED_FLAGS_9   PRIVATE_SHARED_FLAGS_8|OPEN_FNOATIME
#else
#define PRIVATE_NOSHAR_FLAGS_9   PRIVATE_NOSHAR_FLAGS_8|OPEN_FNOATIME
#define PRIVATE_SHARED_FLAGS_9   PRIVATE_SHARED_FLAGS_8
#endif
#endif

#ifndef O_CLOEXEC
#define PRIVATE_NOSUPP_FLAGS_A   PRIVATE_NOSUPP_FLAGS_9|OPEN_FCLOEXEC
#define PRIVATE_SHARED_FLAGS_A   PRIVATE_SHARED_FLAGS_9
#define PRIVATE_NOSHAR_FLAGS_A   PRIVATE_NOSHAR_FLAGS_9
#else
#define PRIVATE_NOSUPP_FLAGS_A   PRIVATE_NOSUPP_FLAGS_9
#if OPEN_FCLOEXEC == O_CLOEXEC
#define PRIVATE_NOSHAR_FLAGS_A   PRIVATE_NOSHAR_FLAGS_9
#define PRIVATE_SHARED_FLAGS_A   PRIVATE_SHARED_FLAGS_9|OPEN_FCLOEXEC
#else
#define PRIVATE_NOSHAR_FLAGS_A   PRIVATE_NOSHAR_FLAGS_9|OPEN_FCLOEXEC
#define PRIVATE_SHARED_FLAGS_A   PRIVATE_SHARED_FLAGS_9
#endif
#endif


#define NOSUPP_FLAGS   (PRIVATE_NOSUPP_FLAGS_A) /* Unsupported options. */
#define SHARED_FLAGS   (PRIVATE_SHARED_FLAGS_A) /* Options with which we share bits. */
#define NOSHAR_FLAGS   (PRIVATE_NOSHAR_FLAGS_A) /* Options with which we don't share bits. */

#ifndef O_RDONLY
#define O_RDONLY 0
#endif /* !O_RDONLY */

#ifndef O_WRONLY
#define O_WRONLY 0
#endif /* !O_WRONLY */

#ifndef O_RDWR
#define O_RDWR 0
#endif /* !O_RDWR */


PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_Open(/*String*/ DeeObject *__restrict filename, int oflags, int mode) {
	DREF SystemFile *result;
	int fd, used_oflags;
	char *utf8_filename;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
#if O_RDONLY == OPEN_FRDONLY && \
    O_WRONLY == OPEN_FWRONLY && \
    O_RDWR == OPEN_FRDWR
	used_oflags = oflags & (SHARED_FLAGS | O_RDONLY | O_WRONLY | O_RDWR);
#else
	{
		PRIVATE int const accmode_flags[4] = {
			/* [OPEN_FRDONLY] = */ O_RDONLY,
			/* [OPEN_FWRONLY] = */ O_WRONLY,
			/* [OPEN_FRDWR  ] = */ O_RDWR,
			/* [OPEN_F???   ] = */ O_RDWR
		};
		used_oflags = accmode_flags[oflags & OPEN_FACCMODE];
#if SHARED_FLAGS != 0
		used_oflags |= oflags & SHARED_FLAGS;
#endif
	}
#endif

	/* Copy bits that we don't share. */
#if NOSHAR_FLAGS & OPEN_FCREAT
	if (oflags & OPEN_FCREAT)
		used_oflags |= O_CREAT;
#endif /* NOSHAR_FLAGS & OPEN_FCREAT */
#if NOSHAR_FLAGS & OPEN_FEXCL
	if (oflags & OPEN_FEXCL)
		used_oflags |= O_EXCL;
#endif /* NOSHAR_FLAGS & OPEN_FEXCL */
#if NOSHAR_FLAGS & OPEN_FTRUNC
	if (oflags & OPEN_FTRUNC)
		used_oflags |= O_TRUNC;
#endif /* NOSHAR_FLAGS & OPEN_FTRUNC */
#if NOSHAR_FLAGS & OPEN_FAPPEND
	if (oflags & OPEN_FAPPEND)
		used_oflags |= O_APPEND;
#endif /* NOSHAR_FLAGS & OPEN_FAPPEND */
#ifdef O_NONBLOCK
#if NOSHAR_FLAGS & OPEN_FNONBLOCK
	if (oflags & OPEN_FNONBLOCK)
		used_oflags |= O_NONBLOCK;
#endif /* NOSHAR_FLAGS & OPEN_FNONBLOCK */
#endif /* O_NONBLOCK */
#ifdef O_SYNC
#if NOSHAR_FLAGS & OPEN_FSYNC
	if (oflags & OPEN_FSYNC)
		used_oflags |= O_SYNC;
#endif /* NOSHAR_FLAGS & OPEN_FSYNC */
#endif /* O_SYNC */
#ifdef O_DIRECT
#if NOSHAR_FLAGS & OPEN_FDIRECT
	if (oflags & OPEN_FDIRECT)
		used_oflags |= O_DIRECT;
#endif /* NOSHAR_FLAGS & OPEN_FDIRECT */
#endif /* O_DIRECT */
#ifdef O_NOFOLLOW
#if NOSHAR_FLAGS & OPEN_FNOFOLLOW
	if (oflags & OPEN_FNOFOLLOW)
		used_oflags |= O_NOFOLLOW;
#endif /* NOSHAR_FLAGS & OPEN_FNOFOLLOW */
#endif /* O_NOFOLLOW */
#ifdef O_NOATIME
#if NOSHAR_FLAGS & OPEN_FNOATIME
	if (oflags & OPEN_FNOATIME)
		used_oflags |= O_NOATIME;
#endif /* NOSHAR_FLAGS & OPEN_FNOATIME */
#endif /* O_NOATIME */
#ifdef O_CLOEXEC
#if NOSHAR_FLAGS & OPEN_FCLOEXEC
	if (oflags & OPEN_FCLOEXEC)
		used_oflags |= O_CLOEXEC;
#endif /* NOSHAR_FLAGS & OPEN_FCLOEXEC */
#endif /* O_CLOEXEC */

#ifdef _O_OBTAIN_DIR
	/* Allow the opening of directories. */
	used_oflags |= _O_OBTAIN_DIR;
#endif /* _O_OBTAIN_DIR */

#ifdef _O_BINARY
	/* Prevent the system from tinkering with the data. */
	used_oflags |= _O_BINARY;
#endif /* _O_BINARY */

	/* Do the open. */
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename)
		goto err;

	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_open64
	fd = open64(utf8_filename, used_oflags, mode);
#else /* CONFIG_HAVE_open64 */
	fd  = open(utf8_filename, used_oflags, mode);
#endif /* !CONFIG_HAVE_open64 */
	DBG_ALIGNMENT_ENABLE();
#if defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen)
	if (fd < 0) { /* Re-try in wide-character mode if supported by the host. */
		wchar_t *str = (wchar_t *)DeeString_AsWide(filename);
		if unlikely(!str)
			goto err;
		DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_wopen64
		fd = wopen64(str, used_oflags, mode);
#else /* CONFIG_HAVE_wopen64 */
		fd = wopen(str, used_oflags, mode);
#endif /* !CONFIG_HAVE_wopen64 */
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_wopen64 || CONFIG_HAVE_wopen */
	if (fd < 0) {
		int error = DeeSystem_GetErrno();
		/* Handle file-already-exists. */
		if (error == EEXIST && (oflags & OPEN_FEXCL))
			return ITER_DONE;
		/* Handle file-not-found. */
		if (error == ENOENT && !(oflags & OPEN_FCREAT))
			return ITER_DONE;
#ifdef ENOTDIR
		if (error == ENOTDIR)
			return ITER_DONE;
#endif /* ENOTDIR */
#ifdef EACCES
		if (error == EACCES) {
			DeeUnixSystem_ThrowErrorf(&DeeError_FileAccessError, error,
			                          "Failed to access %r", filename);
		} else
#endif /* EACCES */
#if defined(ENXIO) || defined(EROFS) || defined(EISDIR) || defined(ETXTBSY)
		if (0
#ifdef ENXIO
		    || error == ENXIO
#endif /* ENXIO */
#ifdef EROFS
		    || error == EROFS
#endif /* EROFS */
#ifdef EISDIR
		    || error == EISDIR
#endif /* EISDIR */
#ifdef ETXTBSY
		    || error == ETXTBSY
#endif /* ETXTBSY */
		    ) {
			DeeUnixSystem_ThrowErrorf(&DeeError_ReadOnlyFile, error,
			                          "Cannot open directory %r for writing",
			                          filename);
		} else
#endif /* ENXIO || EROFS || EISDIR || ETXTBSY */
		{
			DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
			                          "Failed to open %r",
			                          filename);
		}
		goto err;
	}
	result = DeeObject_MALLOC(SystemFile);
	if unlikely(!result)
		goto err_fd;
	DeeLFileObject_Init(result, &DeeFSFile_Type);
	result->sf_handle    = (DeeSysFD)fd;
	result->sf_ownhandle = (DeeSysFD)fd; /* Inherit stream. */
	result->sf_filename  = filename;
	Dee_Incref(filename);
	return (DREF DeeObject *)result;
err_fd:
#ifdef CONFIG_HAVE_close
	close(fd);
#endif /* CONFIG_HAVE_close */
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeFile_OpenString(char const *__restrict filename,
                   int oflags, int mode) {
	DREF DeeObject *result, *nameob;
	/* Due to the whole wide-string mess on windows, this is
	 * just a thin wrapper around the string-object version. */
	nameob = DeeString_New(filename);
	if unlikely(!nameob)
		goto err;
	result = DeeFile_Open(nameob, oflags, mode);
	Dee_Decref(nameob);
	return result;
err:
	return NULL;
}



PRIVATE SystemFile sysf_std[] = {
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, STDIN_FILENO, -1 },
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, STDOUT_FILENO, -1 },
	{ LFILE_OBJECT_HEAD_INIT(&DeeSystemFile_Type), NULL, STDERR_FILENO, -1 },
};

PUBLIC ATTR_RETNONNULL
DeeObject *DCALL DeeFile_DefaultStd(unsigned int id) {
	return (DeeObject *)&sysf_std[id];
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
sysfile_read(SystemFile *__restrict self,
             void *__restrict buffer, size_t bufsize,
             dioflag_t flags) {
#ifdef CONFIG_HAVE_read
	/* TODO: Use `select()' to check if reading will block for `Dee_FILEIO_FNONBLOCKING' */
	/* TODO: Use KOS's writef() function */
	Dee_ssize_t result;
	DBG_ALIGNMENT_DISABLE();
	result = read((int)self->sf_handle, buffer, bufsize);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result < 0) {
		error_file_io(self);
		result = -1;
	}
	return (dssize_t)result;
#else /* CONFIG_HAVE_read */
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_READ);
#endif /* !CONFIG_HAVE_read */
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
sysfile_write(SystemFile *__restrict self,
              void const *__restrict buffer, size_t bufsize,
              dioflag_t flags) {
#ifdef CONFIG_HAVE_write
	/* TODO: Use `select()' to check if writing will block for `Dee_FILEIO_FNONBLOCKING' */
	/* TODO: Use KOS's writef() function */
	Dee_ssize_t result;
	DBG_ALIGNMENT_DISABLE();
	result = write((int)self->sf_handle, buffer, bufsize);
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result < 0) {
		error_file_io(self);
		result = -1;
	}
	return (dssize_t)result;
#else /* CONFIG_HAVE_write */
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_WRITE);
#endif /* !CONFIG_HAVE_write */
}

PRIVATE doff_t DCALL
sysfile_seek(SystemFile *__restrict self, doff_t off, int whence) {
#if defined(CONFIG_HAVE_lseek) || defined(CONFIG_HAVE_lseek64)
	doff_t result;
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_lseek64
	result = lseek64((int)self->sf_handle, (int64_t)off, whence);
#else /* CONFIG_HAVE_lseek64 */
	result = lseek((int)self->sf_handle, (off_t)off, whence);
#endif /* !CONFIG_HAVE_lseek64 */
	DBG_ALIGNMENT_ENABLE();
	if unlikely(result < 0) {
		error_file_io(self);
		result = -1;
	}
	return result;
#else /* CONFIG_HAVE_lseek || CONFIG_HAVE_lseek64 */
	return err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                  FILE_OPERATOR_SEEK);
#endif /* !CONFIG_HAVE_lseek && !CONFIG_HAVE_lseek64 */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL sysfile_sync(SystemFile *__restrict self) {
#if defined(CONFIG_HAVE_fdatasync)
	DBG_ALIGNMENT_DISABLE();
	if unlikely(fdatasync((int)self->sf_handle) < 0) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
#elif defined(CONFIG_HAVE_fsync)
	DBG_ALIGNMENT_DISABLE();
	if unlikely(fsync((int)self->sf_handle) < 0) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
	DBG_ALIGNMENT_ENABLE();
#else
	(void)self;
#endif
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_trunc(SystemFile *__restrict self, dpos_t size) {
	int result;
#if defined(CONFIG_HAVE_ftruncate) || defined(CONFIG_HAVE_ftruncate64)
	/* Use ftruncate() */
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_ftruncate64
	result = ftruncate64((int)self->sf_handle, (int64_t)size);
#else /* CONFIG_HAVE_ftruncate64 */
	result = ftruncate((int)self->sf_handle, (off_t)size);
#endif /* !CONFIG_HAVE_ftruncate64 */
	DBG_ALIGNMENT_ENABLE();
	if (result)
		error_file_io(self);
#elif defined(CONFIG_HAVE_truncate) || defined(CONFIG_HAVE_truncate64)
	/* Use truncate() */
	DREF DeeObject *filename;
	char *utf8_filename;
	filename = DeeSystemFile_Filename((DeeObject *)self);
	if unlikely(!filename)
		return -1;
	utf8_filename = DeeString_AsUtf8(filename);
	if unlikely(!utf8_filename) {
		Dee_Decref(filename);
		return -1;
	}
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_truncate64
	result = truncate64(utf8_filename, (off64_t)size);
#else /* CONFIG_HAVE_truncate64 */
	result = truncate(utf8_filename, (off64_t)size);
#endif /* !CONFIG_HAVE_truncate64 */
	DBG_ALIGNMENT_ENABLE();
	Dee_Decref(filename);
	if (result)
		error_file_io(self);
#else
	result = err_unimplemented_operator((DeeTypeObject *)&DeeSystemFile_Type,
	                                    FILE_OPERATOR_TRUNC);
#endif
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_close(SystemFile *__restrict self) {
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_close
	if unlikely(close((int)self->sf_ownhandle)) {
		DBG_ALIGNMENT_ENABLE();
		return error_file_io(self);
	}
#endif /* CONFIG_HAVE_close */
	DBG_ALIGNMENT_ENABLE();
	self->sf_handle    = (DeeSysFD)-1;
	self->sf_ownhandle = (DeeSysFD)-1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_fileno(SystemFile *__restrict self) {
	DeeSysFD result;
	result = DeeSystemFile_Fileno((DeeObject *)self);
	if unlikely(result == (DeeSysFD)-1)
		return NULL;
	return DeeInt_NewInt((int)result);
}

#undef sysfile_isatty_USE_ISATTY
#undef sysfile_isatty_USE_ISASTDFILE
#undef sysfile_isatty_USE_RETURN_FALSE
#if defined(CONFIG_HAVE_isatty)
#define sysfile_isatty_USE_ISATTY 1
#elif defined(STDIN_FILENO) || defined(STDOUT_FILENO) || defined(STDERR_FILENO)
#define sysfile_isatty_USE_ISASTDFILE 1
#else
#define sysfile_isatty_USE_RETURN_FALSE 1
#endif


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sysfile_isatty(SystemFile *__restrict self) {
#ifdef sysfile_isatty_USE_ISATTY
	int result;
	DBG_ALIGNMENT_DISABLE();
	result = isatty((int)self->sf_handle);
	DBG_ALIGNMENT_ENABLE();
	if (result)
		return_true;
#if defined(EINVAL) || defined(ENOTTY)
	/* Check our whitelist of errors that indicate not-a-tty. */
	DeeSystem_IF_E2(DeeSystem_GetErrno(),
	                EINVAL,
	                ENOTTY,
	                return_false);
	error_file_io(self);
	return NULL;
#else /* EINVAL || ENOTTY */
	return_false;
#endif /* !EINVAL && !ENOTTY */
#endif /* sysfile_isatty_USE_ISATTY */

#ifdef sysfile_isatty_USE_ISASTDFILE
#ifdef STDIN_FILENO
	if ((int)self->sf_handle == STDIN_FILENO)
		goto is_an_std_file;
#endif /* STDIN_FILENO */
#ifdef STDOUT_FILENO
	if ((int)self->sf_handle == STDOUT_FILENO)
		goto is_an_std_file;
#endif /* STDOUT_FILENO */
#ifdef STDERR_FILENO
	if ((int)self->sf_handle == STDERR_FILENO)
		goto is_an_std_file;
#endif /* STDERR_FILENO */
	return_false;
is_an_std_file:
	return_true;
#endif /* sysfile_isatty_USE_ISASTDFILE */

#ifdef sysfile_isatty_USE_RETURN_FALSE
	(void)self;
	return_false;
#endif /* sysfile_isatty_USE_RETURN_FALSE */
}

#if defined(DeeSysFD_GETSET) && defined(DeeSysFS_IS_FILE)
#define STR_fileno DeeString_STR(&str_getsysfd)
#else /* DeeSysFD_GETSET && DeeSysFS_IS_FILE */
#define STR_fileno DeeSysFD_INT_GETSET
#endif /* !DeeSysFD_GETSET || !DeeSysFS_IS_FILE */

PRIVATE struct type_getset sysfile_getsets[] = {
	{ STR_fileno, (DREF DeeObject *(DCALL *)(DeeObject *))&sysfile_fileno, NULL, NULL, DOC("->?Dint") },
	{ DeeString_STR(&str_isatty), (DREF DeeObject *(DCALL *)(DeeObject *))&sysfile_isatty, NULL, NULL, DOC("->?Dbool") },
	{ "filename", &DeeSystemFile_Filename, NULL, NULL, DOC("->?Dstring") },
	{ NULL }
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
sysfile_init_kw(SystemFile *__restrict self,
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {
	PRIVATE DEFINE_KWLIST(kwlist, { K(fd), K(inherit), K(duplicate), KEND });
	bool inherit = false;
	bool duplicate = false;
	int fd;
	DeeObject *fdobj;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o|bb:_SystemFile",
	                    &fdobj, &inherit, &duplicate))
		goto err;
	fd = DeeUnixSystem_GetFD(fdobj);
	if unlikely(fd == -1)
		goto err;
	if (duplicate) {
#ifdef CONFIG_HAVE_dup
		int dupfd;
#ifdef EINTR
again_dup:
#endif /* EINTR */
		DBG_ALIGNMENT_DISABLE();
		dupfd = dup(fd);
		if unlikely(dupfd < 0) {
			int error = DeeSystem_GetErrno();
			DBG_ALIGNMENT_ENABLE();
#ifdef EINTR
			if (error == EINTR)
				goto again_dup;
#endif /* EINTR */
#ifdef ENOSYS
			if (error == ENOSYS) {
				DeeError_Throwf(&DeeError_UnsupportedAPI,
				                "Unsupported function `dup'");
				goto err;
			}
#endif /* ENOSYS */
#ifdef EBADF
			if (error == EBADF) {
				DeeError_Throwf(&DeeError_FileClosed,
				                "Invalid handle %d", fd);
			}
#endif /* EBADF */
			DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
			                          "Failed to dup %d", fd);
			goto err;
		}
		DBG_ALIGNMENT_ENABLE();
#else /* CONFIG_HAVE_dup */
		DeeError_Throwf(&DeeError_UnsupportedAPI,
		                "Unsupported function `dup'");
		goto err;
#endif /* !CONFIG_HAVE_dup */
	} else {
		self->sf_handle    = (DeeSysFD)fd;
		self->sf_ownhandle = (DeeSysFD)-1;
		if (inherit)
			self->sf_ownhandle = (DeeSysFD)fd;
	}
	self->sf_filename = NULL;
	rwlock_init(&self->fo_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
sysfile_fini(SystemFile *__restrict self) {
#ifdef CONFIG_HAVE_close
	if ((int)self->sf_ownhandle >= 0) {
		DBG_ALIGNMENT_DISABLE();
		close((int)self->sf_ownhandle);
		DBG_ALIGNMENT_ENABLE();
	}
#endif /* CONFIG_HAVE_close */
	Dee_XDecref(self->sf_filename);
}

PRIVATE NONNULL((1, 2)) void DCALL
sysfile_visit(SystemFile *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->sf_filename);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
sysfile_class_sync(DeeObject *__restrict UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":sync"))
		return NULL;
#ifdef CONFIG_HAVE_sync
	DBG_ALIGNMENT_DISABLE();
	sync();
	DBG_ALIGNMENT_ENABLE();
#endif /* CONFIG_HAVE_sync */
	return_none;
}

PRIVATE struct type_method sysfile_class_methods[] = {
	{ "sync", &sysfile_class_sync,
	  DOC("()\n"
	      "Synchronize all unwritten data with the host operating system") },
	{ NULL }
};

PRIVATE struct type_member sysfile_class_members[] = {
	TYPE_MEMBER_CONST("Fs", (DeeTypeObject *)&DeeFSFile_Type),
	TYPE_MEMBER_END
};

PUBLIC DeeFileTypeObject DeeSystemFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_SystemFile",
		/* .tp_doc      = */ DOC("(fd:?X2?Dint?DFile,inherit=!f,duplicate=!f)\n"
		                         "Construct a new SystemFile wrapper for @fd. When @inherit is "
		                         ":true, the given @fd is inherited (and automatically closed "
		                         "once the returned :File is destroyed or #{close}ed. When @duplicate "
		                         "is :true, the given @fd is duplicated, and the duplicated copy "
		                         "will be stored inside (in this case, @inherit is ignored)"),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_HASFILEOPS,
		/* .tp_base     = */ (DeeTypeObject *)&DeeFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ NULL,
					/* .tp_copy_ctor = */ NULL,
					/* .tp_deep_ctor = */ NULL,
					/* .tp_any_ctor  = */ NULL,
					TYPE_FIXED_ALLOCATOR(SystemFile),
					/* .tp_any_ctor_kw = */ &sysfile_init_kw
				}
			},
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sysfile_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
		},
		/* .tp_call          = */ NULL,
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sysfile_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ sysfile_getsets,
		/* .tp_members       = */ NULL,
		/* .tp_class_methods = */ sysfile_class_methods,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ sysfile_class_members
	},
	/* .ft_read   = */ (dssize_t (DCALL *)(DeeFileObject *__restrict, void *__restrict, size_t, dioflag_t))&sysfile_read,
	/* .ft_write  = */ (dssize_t (DCALL *)(DeeFileObject *__restrict, void const *__restrict, size_t, dioflag_t))&sysfile_write,
	/* .ft_seek   = */ (doff_t (DCALL *)(DeeFileObject *__restrict, doff_t, int))&sysfile_seek,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *__restrict))&sysfile_sync,
	/* .ft_trunc  = */ (int (DCALL *)(DeeFileObject *__restrict, dpos_t))&sysfile_trunc,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *__restrict))&sysfile_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};

PUBLIC DeeFileTypeObject DeeFSFile_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "_FSFile",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeeSystemFile_Type,
		/* .tp_init = */ {
			{
				/* .tp_alloc = */ {
					/* .tp_ctor      = */ NULL,
					/* .tp_copy_ctor = */ NULL,
					/* .tp_deep_ctor = */ NULL,
					/* .tp_any_ctor  = */ NULL,
					TYPE_FIXED_ALLOCATOR(SystemFile)
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
	},
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ NULL,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ NULL,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ NULL,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_SYSTEM_UNIX_FILE_C_INL */
