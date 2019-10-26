/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_POSIX_P_READWRITE_C_INL
#define GUARD_DEX_POSIX_P_READWRITE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN

/*[[[deemon
import * from deemon;
import * from _dexutils;
MODULE_NAME = "posix";
local orig_stdout = File.stdout;

include("p-readwrite-constants.def");

local allDecls = [];

function rwgii(name, doc = none) {
	allDecls.append(name);
	gii(name, doc: doc);
}

rwgii("SEEK_SET");
rwgii("SEEK_CUR");
rwgii("SEEK_END");
rwgii("SEEK_HOLE");
rwgii("SEEK_DATA");

File.stdout = orig_stdout;
print "#define POSIX_READWRITE_DEFS \\";
for (local x: allDecls) {
	print "\tPOSIX_",;
	print x,;
	print "_DEF \\";
}
print "/" "**" "/";

]]]*/
#include "p-readwrite-constants.def"
#define POSIX_READWRITE_DEFS \
	POSIX_SEEK_SET_DEF \
	POSIX_SEEK_CUR_DEF \
	POSIX_SEEK_END_DEF \
	POSIX_SEEK_HOLE_DEF \
	POSIX_SEEK_DATA_DEF \
/**/
//[[[end]]]



#ifdef CONFIG_HOST_WINDOWS
typedef union {
	struct {
		/* Use our own structure so it gets aligned by 64 bits,
		 * and `Offset' can be assigned directly, without some
		 * sh1tty wrapper code. */
		uintptr_t Internal;
		uintptr_t InternalHigh;
#ifdef CONFIG_LITTLE_ENDIAN
		uint64_t  Offset;
#else /* CONFIG_LITTLE_ENDIAN */
		uint32_t  Offset;
		uint32_t  OffsetHigh;
#endif /* !CONFIG_LITTLE_ENDIAN */
	}   me;
	OVERLAPPED    ms;
} my_OVERLAPPED;
#endif /* CONFIG_HOST_WINDOWS */





/************************************************************************/
/* read()                                                               */
/************************************************************************/
/*[[[deemon import("_dexutils").gw("read", "fd:d,buf:obj:buffer=Dee_None,count:Iud=-1->?X2?Dint?DBytes", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_read_f_impl(int fd, DeeObject *buf, size_t count);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_read_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_READ_DEF { "read", (DeeObject *)&posix_read, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes=!N,count:?Dint=!-1)->?X2?Dint?DBytes") },
#define POSIX_READ_DEF_DOC(doc) { "read", (DeeObject *)&posix_read, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes=!N,count:?Dint=!-1)->?X2?Dint?DBytes\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_read, posix_read_f);
#ifndef POSIX_KWDS_FD_BUF_COUNT_DEFINED
#define POSIX_KWDS_FD_BUF_COUNT_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_buf_count, { K(fd), K(buf), K(count), KEND });
#endif /* !POSIX_KWDS_FD_BUF_COUNT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_read_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	DeeObject *buf = Dee_None;
	size_t count = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_buf_count, "d|oId:read", &fd, &buf, &count))
	    goto err;
	return posix_read_f_impl(fd, buf, count);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_read_f_impl(int fd, DeeObject *buf, size_t count)
//[[[end]]]
{
#ifdef CONFIG_HAVE_read
	if (DeeNone_Check(buf)) {
		/* TODO: Read up to `count' bytes into a newly allocated buffer */
		DERROR_NOTIMPLEMENTED();
		goto err;
	} else {
		DeeBuffer buffer;
		Dee_ssize_t result_value;
		if (DeeObject_GetBuf(buf, &buffer, Dee_BUFFER_FWRITABLE))
			goto err;
		if (buffer.bb_size > count)
			buffer.bb_size = count;
EINTR_LABEL(again)
		if (DeeThread_CheckInterrupt())
			goto err;
		DBG_ALIGNMENT_DISABLE();
		result_value = (Dee_ssize_t)read(fd, buffer.bb_base, buffer.bb_size);
		DBG_ALIGNMENT_ENABLE();
		if (result_value < 0) {
			int error = DeeSystem_GetErrno();
			HANDLE_EINTR(error, again, err)
			DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FWRITABLE);
			HANDLE_EBADF(error, err, "Invalid handle %d", fd)
			DeeError_SysThrowf(&DeeError_FSError, error,
			                   "Failed to read from %d", fd);
			goto err;
		}
		DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FWRITABLE);
		return DeeInt_NewSize((size_t)result_value);
	}
err:
	return NULL;
#else /* CONFIG_HAVE_read */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)buf;
	posix_err_unsupported("read");
	return NULL;
#endif /* !CONFIG_HAVE_read */
}





/************************************************************************/
/* write()                                                              */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("write", "fd:d,buf:obj:buffer,count:Iud=-1->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_write_f_impl(int fd, DeeObject *buf, size_t count);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_write_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_WRITE_DEF { "write", (DeeObject *)&posix_write, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,count:?Dint=!-1)->?Dint") },
#define POSIX_WRITE_DEF_DOC(doc) { "write", (DeeObject *)&posix_write, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,count:?Dint=!-1)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_write, posix_write_f);
#ifndef POSIX_KWDS_FD_BUF_COUNT_DEFINED
#define POSIX_KWDS_FD_BUF_COUNT_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_buf_count, { K(fd), K(buf), K(count), KEND });
#endif /* !POSIX_KWDS_FD_BUF_COUNT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_write_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	size_t count = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_buf_count, "do|Id:write", &fd, &buf, &count))
	    goto err;
	return posix_write_f_impl(fd, buf, count);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_write_f_impl(int fd, DeeObject *buf, size_t count)
//[[[end]]]
{
#ifdef CONFIG_HAVE_write
	DeeBuffer buffer;
	Dee_ssize_t result_value;
	if (DeeObject_GetBuf(buf, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	if (buffer.bb_size > count)
		buffer.bb_size = count;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result_value = (Dee_ssize_t)write(fd, buffer.bb_base, buffer.bb_size);
	DBG_ALIGNMENT_ENABLE();
	if (result_value < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FREADONLY);
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to write to %d", fd);
		goto err;
	}
	DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FREADONLY);
	return DeeInt_NewSSize(result_value);
#else /* CONFIG_HAVE_write */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)buf;
	posix_err_unsupported("write");
#endif /* !CONFIG_HAVE_write */
err:
	return NULL;
}





/************************************************************************/
/* pread()                                                              */
/************************************************************************/

#if defined(CONFIG_HAVE_pread64) || \
    (defined(CONFIG_HAVE__get_osfhandle) && defined(CONFIG_HOST_WINDOWS)) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("pread", "fd:d,buf:obj:buffer,offset:I64d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pread_f_impl(int fd, DeeObject *buf, int64_t offset);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pread_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_PREAD_DEF { "pread", (DeeObject *)&posix_pread, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint") },
#define POSIX_PREAD_DEF_DOC(doc) { "pread", (DeeObject *)&posix_pread, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_pread, posix_pread_f);
#ifndef POSIX_KWDS_FD_BUF_OFFSET_DEFINED
#define POSIX_KWDS_FD_BUF_OFFSET_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_buf_offset, { K(fd), K(buf), K(offset), KEND });
#endif /* !POSIX_KWDS_FD_BUF_OFFSET_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pread_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	int64_t offset;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_buf_offset, "doI64d:pread", &fd, &buf, &offset))
	    goto err;
	return posix_pread_f_impl(fd, buf, offset);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pread_f_impl(int fd, DeeObject *buf, int64_t offset)
//[[[end]]]
#endif
#if !(defined(CONFIG_HAVE_pread64) || \
      (defined(CONFIG_HAVE__get_osfhandle) && defined(CONFIG_HOST_WINDOWS))) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("pread", "fd:d,buf:obj:buffer,offset:I32d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pread_f_impl(int fd, DeeObject *buf, int32_t offset);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pread_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_PREAD_DEF { "pread", (DeeObject *)&posix_pread, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint") },
#define POSIX_PREAD_DEF_DOC(doc) { "pread", (DeeObject *)&posix_pread, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_pread, posix_pread_f);
#ifndef POSIX_KWDS_FD_BUF_OFFSET_DEFINED
#define POSIX_KWDS_FD_BUF_OFFSET_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_buf_offset, { K(fd), K(buf), K(offset), KEND });
#endif /* !POSIX_KWDS_FD_BUF_OFFSET_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pread_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	int32_t offset;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_buf_offset, "doI32d:pread", &fd, &buf, &offset))
	    goto err;
	return posix_pread_f_impl(fd, buf, offset);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pread_f_impl(int fd, DeeObject *buf, int32_t offset)
//[[[end]]]
#endif
{
#if defined(CONFIG_HAVE_pread64) || defined(CONFIG_HAVE_pread) || \
    (defined(CONFIG_HAVE__get_osfhandle) && defined(CONFIG_HOST_WINDOWS))
	DeeBuffer buffer;
	Dee_ssize_t result_value;
	if (DeeObject_GetBuf(buf, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#if defined(CONFIG_HAVE_pread64)
	result_value = (Dee_ssize_t)pread64(fd, buffer.bb_base, buffer.bb_size, offset);
#elif !defined(CONFIG_HAVE__get_osfhandle) || !defined(CONFIG_HOST_WINDOWS)
	result_value = (Dee_ssize_t)pread(fd, buffer.bb_base, buffer.bb_size, offset);
#else
	{
		HANDLE h;
		h = (HANDLE)_get_osfhandle(fd);
		if (h == INVALID_HANDLE_VALUE)
			result_value = -1;
		else {
			DWORD bytes_written;
			my_OVERLAPPED overlapped;
#if __SIZEOF_SIZE_T__ > 4
			if unlikely(buffer.bb_size > UINT32_MAX)
				buffer.bb_size = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
again_ReadFile:
			memset(&overlapped, 0, sizeof(overlapped));
#ifdef CONFIG_LITTLE_ENDIAN
			overlapped.me.Offset = (uint64_t)offset;
#else /* CONFIG_LITTLE_ENDIAN */
			overlapped.me.Offset = (uint32_t)(uint64_t)offset;
			overlapped.me.OffsetHigh = (uint32_t)((uint64_t)offset >> 32);
#endif /* !CONFIG_LITTLE_ENDIAN */
			if unlikely(!ReadFile(h, buffer.bb_base,
			                      (DWORD)buffer.bb_size,
			                      &bytes_written,
			                      (LPOVERLAPPED)&overlapped)) {
				DWORD error = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (DeeNTSystem_IsIntr(error)) {
					if (DeeThread_CheckInterrupt())
						goto err;
					DBG_ALIGNMENT_DISABLE();
					goto again_ReadFile;
				}
				DeeError_SysThrowf(&DeeError_FSError, error,
				                   "Failed to read from %d", fd);
				goto err;
			}
			DBG_ALIGNMENT_ENABLE();
			result_value = (dssize_t)bytes_written;
		}
	}
#endif
	DBG_ALIGNMENT_ENABLE();
	if (result_value < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FWRITABLE);
		HANDLE_ENOSYS(error, err, "pread")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to read from %d", fd);
		goto err;
	}
	DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FWRITABLE);
	return DeeInt_NewSSize(result_value);
err:
#else /* Supported */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)buf;
	(void)offset;
	posix_err_unsupported("pwrite");
#endif /* !Supported */
	return NULL;
}





/************************************************************************/
/* pwrite()                                                             */
/************************************************************************/

#if defined(CONFIG_HAVE_pwrite64) || \
    defined(CONFIG_HAVE__get_osfhandle) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("pwrite", "fd:d,buf:obj:buffer,offset:I64d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pwrite_f_impl(int fd, DeeObject *buf, int64_t offset);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pwrite_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_PWRITE_DEF { "pwrite", (DeeObject *)&posix_pwrite, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint") },
#define POSIX_PWRITE_DEF_DOC(doc) { "pwrite", (DeeObject *)&posix_pwrite, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_pwrite, posix_pwrite_f);
#ifndef POSIX_KWDS_FD_BUF_OFFSET_DEFINED
#define POSIX_KWDS_FD_BUF_OFFSET_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_buf_offset, { K(fd), K(buf), K(offset), KEND });
#endif /* !POSIX_KWDS_FD_BUF_OFFSET_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pwrite_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	int64_t offset;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_buf_offset, "doI64d:pwrite", &fd, &buf, &offset))
	    goto err;
	return posix_pwrite_f_impl(fd, buf, offset);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pwrite_f_impl(int fd, DeeObject *buf, int64_t offset)
//[[[end]]]
#endif
#if !(defined(CONFIG_HAVE_pwrite64) || defined(CONFIG_HAVE__get_osfhandle)) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("pwrite", "fd:d,buf:obj:buffer,offset:I32d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pwrite_f_impl(int fd, DeeObject *buf, int32_t offset);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pwrite_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_PWRITE_DEF { "pwrite", (DeeObject *)&posix_pwrite, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint") },
#define POSIX_PWRITE_DEF_DOC(doc) { "pwrite", (DeeObject *)&posix_pwrite, MODSYM_FNORMAL, DOC("(fd:?Dint,buf:?DBytes,offset:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_pwrite, posix_pwrite_f);
#ifndef POSIX_KWDS_FD_BUF_OFFSET_DEFINED
#define POSIX_KWDS_FD_BUF_OFFSET_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_buf_offset, { K(fd), K(buf), K(offset), KEND });
#endif /* !POSIX_KWDS_FD_BUF_OFFSET_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pwrite_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	DeeObject *buf;
	int32_t offset;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_buf_offset, "doI32d:pwrite", &fd, &buf, &offset))
	    goto err;
	return posix_pwrite_f_impl(fd, buf, offset);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_pwrite_f_impl(int fd, DeeObject *buf, int32_t offset)
//[[[end]]]
#endif
{
#if defined(CONFIG_HAVE_pwrite64) || defined(CONFIG_HAVE_pwrite) || \
    (defined(CONFIG_HAVE__get_osfhandle) && defined(CONFIG_HOST_WINDOWS))
	DeeBuffer buffer;
	Dee_ssize_t result_value;
	if (DeeObject_GetBuf(buf, &buffer, Dee_BUFFER_FWRITABLE))
		goto err;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#if defined(CONFIG_HAVE_pwrite64)
	result_value = (Dee_ssize_t)pwrite64(fd, buffer.bb_base, buffer.bb_size, offset);
#elif !defined(CONFIG_HAVE__get_osfhandle) || !defined(CONFIG_HOST_WINDOWS)
	result_value = (Dee_ssize_t)pwrite(fd, buffer.bb_base, buffer.bb_size, offset);
#else
	{
		HANDLE h;
		h = (HANDLE)_get_osfhandle(fd);
		if (h == INVALID_HANDLE_VALUE)
			result_value = -1;
		else {
			DWORD bytes_written;
			my_OVERLAPPED overlapped;
#if __SIZEOF_SIZE_T__ > 4
			if unlikely(buffer.bb_size > UINT32_MAX)
				buffer.bb_size = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
again_WriteFile:
			memset(&overlapped, 0, sizeof(overlapped));
#ifdef CONFIG_LITTLE_ENDIAN
			overlapped.me.Offset = (uint64_t)offset;
#else /* CONFIG_LITTLE_ENDIAN */
			overlapped.me.Offset = (uint32_t)(uint64_t)offset;
			overlapped.me.OffsetHigh = (uint32_t)((uint64_t)offset >> 32);
#endif /* !CONFIG_LITTLE_ENDIAN */
			if unlikely(!WriteFile(h, buffer.bb_base,
			                      (DWORD)buffer.bb_size,
			                      &bytes_written,
			                      (LPOVERLAPPED)&overlapped)) {
				DWORD error = GetLastError();
				DBG_ALIGNMENT_ENABLE();
				if (DeeNTSystem_IsIntr(error)) {
					if (DeeThread_CheckInterrupt())
						goto err;
					DBG_ALIGNMENT_DISABLE();
					goto again_WriteFile;
				}
				DeeError_SysThrowf(&DeeError_FSError, error,
				                   "Failed to write from %d", fd);
				goto err;
			}
			DBG_ALIGNMENT_ENABLE();
			result_value = (dssize_t)bytes_written;
		}
	}
#endif
	DBG_ALIGNMENT_ENABLE();
	if (result_value < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FWRITABLE);
		HANDLE_ENOSYS(error, err, "pwrite")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to write from %d", fd);
		goto err;
	}
	DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FWRITABLE);
	return DeeInt_NewSSize(result_value);
err:
#else /* Supported */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)buf;
	(void)offset;
	posix_err_unsupported("pwrite");
#endif /* !Supported */
	return NULL;
}





/************************************************************************/
/* lseek()                                                              */
/************************************************************************/

#if defined(CONFIG_HAVE_lseek64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("lseek", "fd:d,off:I64d,whence:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lseek_f_impl(int fd, int64_t off, int whence);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lseek_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_LSEEK_DEF { "lseek", (DeeObject *)&posix_lseek, MODSYM_FNORMAL, DOC("(fd:?Dint,off:?Dint,whence:?Dint)->?Dint") },
#define POSIX_LSEEK_DEF_DOC(doc) { "lseek", (DeeObject *)&posix_lseek, MODSYM_FNORMAL, DOC("(fd:?Dint,off:?Dint,whence:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lseek, posix_lseek_f);
#ifndef POSIX_KWDS_FD_OFF_WHENCE_DEFINED
#define POSIX_KWDS_FD_OFF_WHENCE_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_off_whence, { K(fd), K(off), K(whence), KEND });
#endif /* !POSIX_KWDS_FD_OFF_WHENCE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lseek_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	int64_t off;
	int whence;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_off_whence, "dI64dd:lseek", &fd, &off, &whence))
	    goto err;
	return posix_lseek_f_impl(fd, off, whence);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lseek_f_impl(int fd, int64_t off, int whence)
//[[[end]]]
#endif
#if !defined(CONFIG_HAVE_lseek64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("lseek", "fd:d,off:I32d,whence:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lseek_f_impl(int fd, int32_t off, int whence);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lseek_f(size_t argc, DeeObject **argv, DeeObject *kw);
#define POSIX_LSEEK_DEF { "lseek", (DeeObject *)&posix_lseek, MODSYM_FNORMAL, DOC("(fd:?Dint,off:?Dint,whence:?Dint)->?Dint") },
#define POSIX_LSEEK_DEF_DOC(doc) { "lseek", (DeeObject *)&posix_lseek, MODSYM_FNORMAL, DOC("(fd:?Dint,off:?Dint,whence:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lseek, posix_lseek_f);
#ifndef POSIX_KWDS_FD_OFF_WHENCE_DEFINED
#define POSIX_KWDS_FD_OFF_WHENCE_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_off_whence, { K(fd), K(off), K(whence), KEND });
#endif /* !POSIX_KWDS_FD_OFF_WHENCE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lseek_f(size_t argc, DeeObject **argv, DeeObject *kw) {
	int fd;
	int32_t off;
	int whence;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_off_whence, "dI32dd:lseek", &fd, &off, &whence))
	    goto err;
	return posix_lseek_f_impl(fd, off, whence);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lseek_f_impl(int fd, int32_t off, int whence)
//[[[end]]]
#endif
{
#if defined(CONFIG_HAVE_lseek64) || defined(CONFIG_HAVE_lseek)
#ifdef CONFIG_HAVE_lseek64
	int64_t result;
#else /* CONFIG_HAVE_lseek64 */
	int32_t result;
#endif /* CONFIG_HAVE_lseek64 */
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_lseek64
	result = lseek64(fd, off, whence);
#else /* CONFIG_HAVE_lseek64 */
	result = lseek(fd, off, whence);
#endif /* !CONFIG_HAVE_lseek64 */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "lseek")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeError_SysThrowf(&DeeError_FSError, error,
		                   "Failed to seek %d", fd);
		goto err;
	}
	return DeeInt_NewS64(result);
err:
#else /* CONFIG_HAVE_lseek64 || CONFIG_HAVE_lseek */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)off;
	(void)whence;
	posix_err_unsupported("lseek");
#endif /* !CONFIG_HAVE_lseek64 && !CONFIG_HAVE_lseek */
	return NULL;
}



DECL_END

#endif /* !GUARD_DEX_POSIX_P_READWRITE_C_INL */
