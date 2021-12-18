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
#ifndef GUARD_DEX_POSIX_P_READWRITE_C_INL
#define GUARD_DEX_POSIX_P_READWRITE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"
#include <hybrid/byteorder.h>

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
for (local x: allDecls)
	print("\tPOSIX_", x, "_DEF \\");
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
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		uint64_t  Offset;
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
		uint32_t  Offset;
		uint32_t  OffsetHigh;
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	}   me;
	OVERLAPPED    ms;
} my_OVERLAPPED;
#endif /* CONFIG_HOST_WINDOWS */

/* Figure out how to implement `read()' */
#undef posix_read_USE_READ
#undef posix_read_USE_STUB
#ifdef CONFIG_HAVE_read
#define posix_read_USE_READ 1
#else /* CONFIG_HAVE_read */
#define posix_read_USE_STUB 1
#endif /* !CONFIG_HAVE_read */

/* Figure out how to implement `lseek()' */
#undef posix_lseek_USE_READ
#undef posix_lseek_USE_STUB
#undef posix_lseek_IS64
#ifdef CONFIG_HAVE_lseek64
#define posix_lseek_IS64 1
#define posix_lseek_USE_LSEEK64 1
#elif defined(CONFIG_HAVE_lseek)
#define posix_lseek_USE_LSEEK 1
#else /* CONFIG_HAVE_lseek */
#define posix_lseek_USE_STUB 1
#endif /* !CONFIG_HAVE_lseek */

/* Figure out how to implement `pread()' */
#undef posix_pread_USE_PREAD64
#undef posix_pread_USE_READFILE
#undef posix_pread_USE_PREAD
#undef posix_pread_USE_LSEEK_READ
#undef posix_pread_USE_STUB
#undef posix_pread_IS64
#if defined(CONFIG_HAVE_pread64)
#define posix_pread_IS64 1
#define posix_pread_USE_PREAD64 1
#elif defined(CONFIG_HAVE_get_osfhandle) && defined(CONFIG_HOST_WINDOWS)
#define posix_pread_IS64 1
#define posix_pread_USE_READFILE 1
#elif (defined(CONFIG_HAVE_pread) && \
       (defined(posix_lseek_USE_STUB) || defined(posix_read_USE_STUB) || !defined(posix_lseek_IS64)))
#define posix_pread_USE_PREAD 1
#elif (!defined(posix_read_USE_STUB) && \
       (defined(CONFIG_HAVE_lseek) || defined(CONFIG_HAVE_lseek64)))
#ifdef CONFIG_HAVE_lseek64
#define posix_pread_IS64 1
#endif /* CONFIG_HAVE_lseek64 */
#define posix_pread_USE_LSEEK_READ 1
#else
#define posix_pread_USE_STUB 1
#endif

#ifdef posix_pread_IS64
#define PREAD_OFF_T  int64_t
#define PREAD_LSEEK  lseek64
#define PREAD_PRIOFF "I64d"
#else /* posix_pread_IS64 */
#define PREAD_OFF_T  int32_t
#define PREAD_LSEEK  lseek
#define PREAD_PRIOFF "I32d"
#endif /* !posix_pread_IS64 */

#ifndef POSIX_READ_BUFSIZE
#define POSIX_READ_BUFSIZE 1024
#endif /* !POSIX_READ_BUFSIZE */


/* Figure out how to implement `write()' */
#undef posix_write_USE_WRITE
#undef posix_write_USE_STUB
#ifdef CONFIG_HAVE_write
#define posix_write_USE_WRITE 1
#else /* CONFIG_HAVE_write */
#define posix_write_USE_STUB 1
#endif /* !CONFIG_HAVE_write */

/* Figure out how to implement `pwrite()' */
#undef posix_pwrite_USE_PWRITE64
#undef posix_pwrite_USE_WRITEFILE
#undef posix_pwrite_USE_PWRITE
#undef posix_pwrite_USE_LSEEK_WRITE
#undef posix_pwrite_USE_STUB
#undef posix_pwrite_IS64
#if defined(CONFIG_HAVE_pwrite64)
#define posix_pwrite_IS64 1
#define posix_pwrite_USE_PWRITE64 1
#elif defined(CONFIG_HAVE_get_osfhandle) && defined(CONFIG_HOST_WINDOWS)
#define posix_pwrite_IS64 1
#define posix_pwrite_USE_WRITEFILE 1
#elif (defined(CONFIG_HAVE_pwrite) && \
       (defined(posix_lseek_USE_STUB) || defined(posix_write_USE_STUB) || !defined(posix_lseek_IS64)))
#define posix_pwrite_USE_PWRITE 1
#elif (!defined(CONFIG_HAVE_write) && \
       (defined(CONFIG_HAVE_lseek) || defined(CONFIG_HAVE_lseek64)))
#ifdef CONFIG_HAVE_lseek64
#define posix_pwrite_IS64 1
#endif /* CONFIG_HAVE_lseek64 */
#define posix_pwrite_USE_LSEEK_WRITE 1
#else
#define posix_pwrite_USE_STUB 1
#endif

#ifdef posix_pwrite_IS64
#define PWRITE_OFF_T  int64_t
#define PWRITE_LSEEK  lseek64
#define PWRITE_PRIOFF "I64d"
#else /* posix_pwrite_IS64 */
#define PWRITE_OFF_T  int32_t
#define PWRITE_LSEEK  lseek
#define PWRITE_PRIOFF "I32d"
#endif /* !posix_pwrite_IS64 */






/************************************************************************/
/* read()                                                               */
/************************************************************************/
#ifndef posix_read_USE_STUB
FORCELOCAL WUNUSED dssize_t DCALL
posix_read_f_impl(int fd, void *buf, size_t count) {
#ifdef posix_read_USE_READ
	dssize_t result_value;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result_value = (dssize_t)read(fd, buf, count);
	DBG_ALIGNMENT_ENABLE();
	if (result_value == -1) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to read from %d", fd);
		goto err;
	}
	return result_value;
err:
	return -1;
#endif /* posix_read_USE_READ */

#ifdef posix_read_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)buf;
	(void)count;
	posix_err_unsupported("read");
	return -1;
#endif /* !posix_read_USE_STUB */
}
#endif /* !posix_read_USE_STUB */

#define POSIX_READ_DEF                                     \
	{ "read", (DeeObject *)&posix_read, MODSYM_FNORMAL,    \
	  DOC("(fd:?X2?Dint?DFile,count:?Dint=!-1)->?DBytes\n" \
		  "(fd:?X2?Dint?DFile,buf:?DBytes,count:?Dint=!-1)->?Dint") },
#define POSIX_READ_DEF_DOC(doc)                            \
	{ "read", (DeeObject *)&posix_read, MODSYM_FNORMAL,    \
	  DOC("(fd:?X2?Dint?DFile,count:?Dint=!-1)->?DBytes\n" \
		  "(fd:?X2?Dint?DFile,buf:?DBytes,count:?Dint=!-1)->?Dint\n" doc) },
PRIVATE WUNUSED DREF DeeObject *DCALL posix_read_f(size_t argc, DeeObject *const *argv) {
#ifndef posix_read_USE_STUB
	int fd_fd;
	DeeObject *fd;
	DeeObject *buf_or_count = &DeeInt_MinusOne;
	size_t count = (size_t)-1;
	dssize_t error;
	if (DeeArg_Unpack(argc, argv, "o|oId:read", &fd, &buf_or_count, &count))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	if (argc == 3 || !DeeInt_Check(buf_or_count)) {
		/* Read into buffer. */
		DeeBuffer buf;
		if (DeeObject_GetBuf(buf_or_count, &buf, Dee_BUFFER_FWRITABLE))
			goto err;
		if (buf.bb_size > count)
			buf.bb_size = count;
		error = posix_read_f_impl(fd_fd, buf.bb_base, buf.bb_size);
		DeeObject_PutBuf(buf_or_count, &buf, Dee_BUFFER_FWRITABLE);
		if unlikely(error == -1)
			goto err;
		return DeeInt_NewSize((size_t)error);
	} else {
		DREF DeeBytesObject *result_bytes;
		if (DeeObject_AsSSize(buf_or_count, (dssize_t *)&count))
			goto err;
		if (count <= POSIX_READ_BUFSIZE) {
			/* Directly read into a Bytes object. */
			result_bytes = (DREF DeeBytesObject *)DeeBytes_NewBufferUninitialized(count);
			if unlikely(!result_bytes)
				goto err;
			error = posix_read_f_impl(fd_fd,
			                          DeeBytes_DATA(result_bytes),
			                          DeeBytes_SIZE(result_bytes));
			if unlikely(error == -1) {
				Dee_Decref(result_bytes);
				goto err;
			}
			result_bytes = (DREF DeeBytesObject *)DeeBytes_TruncateBuffer((DeeObject *)result_bytes,
			                                                              (size_t)error);
		} else {
			/* Construct a new buffer that is then read into. */
			struct bytes_printer p = BYTES_PRINTER_INIT;
			for (;;) {
				size_t buflen;
				uint8_t *buf;
				buflen = count;
				if (buflen > POSIX_READ_BUFSIZE)
					buflen = POSIX_READ_BUFSIZE;
				buf = bytes_printer_alloc(&p, buflen);
				if unlikely(!buf) {
err_bytes_printer:
					bytes_printer_fini(&p);
					goto err;
				}
				error = posix_read_f_impl(fd_fd, buf, buflen);
				if unlikely(error == -1)
					goto err_bytes_printer;
				if ((size_t)error >= count)
					break; /* Done! */
				if ((size_t)error != buflen) {
					/* Incomplete transfer (stop reading) */
					bytes_printer_release(&p, buflen - (size_t)error);
					break;
				}
				count -= (size_t)error;
			}
			result_bytes = (DREF DeeBytesObject *)bytes_printer_pack(&p);
		}
		return (DREF DeeObject *)result_bytes;
	}
err:
	return NULL;
#endif /* !posix_read_USE_STUB */

#ifdef posix_read_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)argc;
	(void)argv;
	posix_err_unsupported("read");
	return NULL;
#endif /* posix_read_USE_STUB */
}
PRIVATE DEFINE_CMETHOD(posix_read, posix_read_f);





/************************************************************************/
/* lseek()                                                              */
/************************************************************************/

#if defined(posix_lseek_IS64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("lseek", "fd:unix:fd,offset:I64d,whence:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lseek_f_impl(int fd, int64_t offset, int whence);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lseek_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_LSEEK_DEF { "lseek", (DeeObject *)&posix_lseek, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,offset:?Dint,whence:?Dint)->?Dint") },
#define POSIX_LSEEK_DEF_DOC(doc) { "lseek", (DeeObject *)&posix_lseek, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,offset:?Dint,whence:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lseek, posix_lseek_f);
#ifndef POSIX_KWDS_FD_OFFSET_WHENCE_DEFINED
#define POSIX_KWDS_FD_OFFSET_WHENCE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd_offset_whence, { K(fd), K(offset), K(whence), KEND });
#endif /* !POSIX_KWDS_FD_OFFSET_WHENCE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lseek_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	int64_t offset;
	int whence;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_offset_whence, "oI64dd:lseek", &fd, &offset, &whence))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	return posix_lseek_f_impl(fd_fd, offset, whence);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lseek_f_impl(int fd, int64_t offset, int whence)
//[[[end]]]
#endif /* posix_lseek_IS64 */
#if !defined(posix_lseek_IS64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("lseek", "fd:unix:fd,offset:I32d,whence:d->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lseek_f_impl(int fd, int32_t offset, int whence);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lseek_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_LSEEK_DEF { "lseek", (DeeObject *)&posix_lseek, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,offset:?Dint,whence:?Dint)->?Dint") },
#define POSIX_LSEEK_DEF_DOC(doc) { "lseek", (DeeObject *)&posix_lseek, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,offset:?Dint,whence:?Dint)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_lseek, posix_lseek_f);
#ifndef POSIX_KWDS_FD_OFFSET_WHENCE_DEFINED
#define POSIX_KWDS_FD_OFFSET_WHENCE_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd_offset_whence, { K(fd), K(offset), K(whence), KEND });
#endif /* !POSIX_KWDS_FD_OFFSET_WHENCE_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_lseek_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	int32_t offset;
	int whence;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_offset_whence, "oI32dd:lseek", &fd, &offset, &whence))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	return posix_lseek_f_impl(fd_fd, offset, whence);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_lseek_f_impl(int fd, int32_t offset, int whence)
//[[[end]]]
#endif /* !posix_lseek_IS64 */
{
#if defined(posix_lseek_USE_LSEEK) || defined(posix_lseek_USE_LSEEK64)
#ifdef posix_lseek_IS64
	int64_t result;
#else /* posix_lseek_IS64 */
	int32_t result;
#endif /* posix_lseek_IS64 */
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_lseek_USE_LSEEK64
	result = lseek64(fd, offset, whence);
#else /* posix_lseek_USE_LSEEK64 */
	result = lseek(fd, offset, whence);
#endif /* !posix_lseek_USE_LSEEK64 */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "lseek")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to seek %d", fd);
		goto err;
	}
	return DeeInt_NewS64(result);
err:
	return NULL;
#endif /* posix_lseek_USE_LSEEK || posix_lseek_USE_LSEEK64 */

#ifdef posix_lseek_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)offset;
	(void)whence;
	posix_err_unsupported("lseek");
	return NULL;
#endif /* posix_lseek_USE_STUB */
}





/************************************************************************/
/* pread()                                                              */
/************************************************************************/
#ifndef posix_pread_USE_STUB
FORCELOCAL WUNUSED dssize_t DCALL
posix_pread_f_impl(int fd, void *buf, size_t count, PREAD_OFF_T offset) {

#if defined(posix_pread_USE_PREAD64) || defined(posix_pread_USE_PREAD)
	dssize_t result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_pread_USE_PREAD64
	result = (dssize_t)pread64(fd, buf, count, offset);
#else /* posix_pread_USE_PREAD64 */
	result = (dssize_t)pread(fd, buf, count, offset);
#endif /* !posix_pread_USE_PREAD64 */
	if (result == -1) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "pread")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to read from %d at offset %" PREAD_PRIOFF,
		                          fd, offset);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return result;
err:
	return -1;
#endif /* posix_pread_USE_PREAD64 || posix_pread_USE_PREAD */

#ifdef posix_pread_USE_READFILE
	HANDLE h;
	DWORD bytes_written;
	my_OVERLAPPED overlapped;
	DBG_ALIGNMENT_DISABLE();
	h = (HANDLE)get_osfhandle(fd);
	DBG_ALIGNMENT_ENABLE();
	if (h == INVALID_HANDLE_VALUE) {
		DeeError_Throwf(&DeeError_FileClosed, "Invalid handle %d", fd);
		goto err;
	}
#if __SIZEOF_SIZE_T__ > 4
	if unlikely(count > UINT32_MAX)
		count = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
again_ReadFile:
	DBG_ALIGNMENT_DISABLE();
	bzero(&overlapped, sizeof(overlapped));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	overlapped.me.Offset = (uint64_t)offset;
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
	overlapped.me.Offset     = (uint32_t)((uint64_t)offset);
	overlapped.me.OffsetHigh = (uint32_t)((uint64_t)offset >> 32);
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	if unlikely(!ReadFile(h, buf, (DWORD)count,
	                      &bytes_written,
	                      (LPOVERLAPPED)&overlapped)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(error)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again_ReadFile;
		}
		DeeNTSystem_ThrowErrorf(NULL, error,
		                        "Failed to read from %d at offset %" PREAD_PRIOFF,
		                        fd, offset);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	ASSERT((dssize_t)bytes_written != -1);
	return (dssize_t)bytes_written;
err:
	return -1;
#endif /* posix_pread_USE_READFILE */

#ifdef posix_pread_USE_LSEEK_READ
	dssize_t result;
	PREAD_OFF_T oldpos, newpos;
HANDLE_EINTR(again)
	DBG_ALIGNMENT_DISABLE();
	oldpos = PREAD_LSEEK(fd, 0, SEEK_CUR);
	if unlikely(oldpos == (PREAD_OFF_T)-1)
		goto handle_system_error;
	newpos = PREAD_LSEEK(fd, offset, SEEK_SET);
	if unlikely(newpos == (PREAD_OFF_T)-1)
		goto handle_system_error;
	DBG_ALIGNMENT_ENABLE();
	result = posix_read_f_impl(fd, buf, count);
	PREAD_LSEEK(fd, oldpos, SEEK_SET);
	return result;
	{
		int error;
handle_system_error:
		error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "pread")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to read from %d at offset %" PREAD_PRIOFF,
		                          fd, offset);
		goto err;
	}
err:
	return -1;
#endif /* posix_pread_USE_LSEEK_READ */

#ifdef posix_pread_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)buf;
	(void)count;
	(void)offset;
	posix_err_unsupported("pread");
	return -1;
#endif /* posix_pread_USE_STUB */

}
#endif /* !posix_pread_USE_STUB */

#define POSIX_PREAD_DEF                                             \
	{ "pread", (DeeObject *)&posix_pread, MODSYM_FNORMAL,           \
	  DOC("(fd:?X2?Dint?DFile,count:?Dint,offset:?Dint)->?DBytes\n" \
		  "(fd:?X2?Dint?DFile,buf:?DBytes,count:?Dint,offset:?Dint)->?Dint") },
#define POSIX_PREAD_DEF_DOC(doc)                                    \
	{ "pread", (DeeObject *)&posix_pread, MODSYM_FNORMAL,           \
	  DOC("(fd:?X2?Dint?DFile,count:?Dint,offset:?Dint)->?DBytes\n" \
		  "(fd:?X2?Dint?DFile,buf:?DBytes,count:?Dint,offset:?Dint)->?Dint\n" doc) },
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pread_f(size_t argc, DeeObject *const *argv) {
#ifndef posix_pread_USE_STUB
	dssize_t error;
	if (argc == 3) {
		/* Read into a new buffer. */
		DREF DeeBytesObject *result_bytes;
		DeeObject *fd;
		size_t count;
		PREAD_OFF_T offset;
		int fd_fd;
		/* Read into a given buffer. */
		if (DeeArg_Unpack(argc, argv, "oId" PREAD_PRIOFF ":pread", &fd, &count, &offset))
			goto err;
		fd_fd = DeeUnixSystem_GetFD(fd);
		if unlikely(fd_fd == -1)
			goto err;
		if (count <= POSIX_READ_BUFSIZE) {
			/* Directly read into a Bytes object. */
			result_bytes = (DREF DeeBytesObject *)DeeBytes_NewBufferUninitialized(count);
			if unlikely(!result_bytes)
				goto err;
			error = posix_pread_f_impl(fd_fd,
			                           DeeBytes_DATA(result_bytes),
			                           DeeBytes_SIZE(result_bytes),
			                           offset);
			if unlikely(error == -1) {
				Dee_Decref(result_bytes);
				goto err;
			}
			result_bytes = (DREF DeeBytesObject *)DeeBytes_TruncateBuffer((DeeObject *)result_bytes,
			                                                              (size_t)error);
		} else {
			/* Construct a new buffer that is then read into. */
			struct bytes_printer p = BYTES_PRINTER_INIT;
			for (;;) {
				size_t buflen;
				uint8_t *buf;
				buflen = count;
				if (buflen > POSIX_READ_BUFSIZE)
					buflen = POSIX_READ_BUFSIZE;
				buf = bytes_printer_alloc(&p, buflen);
				if unlikely(!buf) {
err_bytes_printer:
					bytes_printer_fini(&p);
					goto err;
				}
				error = posix_pread_f_impl(fd_fd, buf, buflen, offset);
				if unlikely(error == -1)
					goto err_bytes_printer;
				if ((size_t)error >= count)
					break; /* Done! */
				if ((size_t)error != buflen) {
					/* Incomplete transfer (stop reading) */
					bytes_printer_release(&p, buflen - (size_t)error);
					break;
				}
				count -= (size_t)error;
				offset += (size_t)error;
			}
			result_bytes = (DREF DeeBytesObject *)bytes_printer_pack(&p);
		}
		return (DREF DeeObject *)result_bytes;
	} else {
		DeeObject *fd, *buf_ob;
		size_t count;
		PREAD_OFF_T offset;
		int fd_fd;
		DeeBuffer buf;
		/* Read into a given buffer. */
		if (DeeArg_Unpack(argc, argv, "ooId" PREAD_PRIOFF ":pread", &fd, &buf_ob, &count, &offset))
			goto err;
		fd_fd = DeeUnixSystem_GetFD(fd);
		if unlikely(fd_fd == -1)
			goto err;
		if (DeeObject_GetBuf(buf_ob, &buf, Dee_BUFFER_FWRITABLE))
			goto err;
		if (buf.bb_size > count)
			buf.bb_size = count;
		error = posix_pread_f_impl(fd_fd, buf.bb_base, buf.bb_size, offset);
		DeeObject_PutBuf(buf_ob, &buf, Dee_BUFFER_FWRITABLE);
		if unlikely(error == -1)
			goto err;
		return DeeInt_NewSize((size_t)error);
	}
err:
	return NULL;
#endif /* !posix_pread_USE_STUB */

#ifdef posix_pread_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)argc;
	(void)argv;
	posix_err_unsupported("pread");
	return NULL;
#endif /* posix_pread_USE_STUB */
}
PRIVATE DEFINE_CMETHOD(posix_pread, posix_pread_f);





/************************************************************************/
/* write()                                                              */
/************************************************************************/

/*[[[deemon import("_dexutils").gw("write", "fd:unix:fd,buf:obj:buffer,count:Iud=-1->?Dint", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_write_f_impl(int fd, DeeObject *buf, size_t count);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_write_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_WRITE_DEF { "write", (DeeObject *)&posix_write, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,buf:?DBytes,count:?Dint=!-1)->?Dint") },
#define POSIX_WRITE_DEF_DOC(doc) { "write", (DeeObject *)&posix_write, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,buf:?DBytes,count:?Dint=!-1)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_write, posix_write_f);
#ifndef POSIX_KWDS_FD_BUF_COUNT_DEFINED
#define POSIX_KWDS_FD_BUF_COUNT_DEFINED
PRIVATE DEFINE_KWLIST(posix_kwds_fd_buf_count, { K(fd), K(buf), K(count), KEND });
#endif /* !POSIX_KWDS_FD_BUF_COUNT_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_write_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	DeeObject *buf;
	size_t count = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_buf_count, "oo|Id:write", &fd, &buf, &count))
		goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
		goto err;
	return posix_write_f_impl(fd_fd, buf, count);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_write_f_impl(int fd, DeeObject *buf, size_t count)
//[[[end]]]
{
#ifdef posix_write_USE_WRITE
	DeeBuffer buffer;
	dssize_t result_value;
	if (DeeObject_GetBuf(buf, &buffer, Dee_BUFFER_FREADONLY))
		goto err;
	if (buffer.bb_size > count)
		buffer.bb_size = count;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
	result_value = (dssize_t)write(fd, buffer.bb_base, buffer.bb_size);
	DBG_ALIGNMENT_ENABLE();
	if (result_value < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FREADONLY);
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to write to %d", fd);
		goto err;
	}
	DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FREADONLY);
	return DeeInt_NewSSize(result_value);
err:
	return NULL;
#endif /* posix_write_USE_WRITE */

#ifdef posix_write_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)buf;
	posix_err_unsupported("write");
	return NULL;
#endif /* !posix_write_USE_STUB */
}





/************************************************************************/
/* pwrite()                                                             */
/************************************************************************/
#ifndef posix_pwrite_USE_STUB
FORCELOCAL WUNUSED dssize_t DCALL
posix_pwrite_f_impl(int fd, void const *buf, size_t count, PWRITE_OFF_T offset) {

#if defined(posix_pwrite_USE_PWRITE64) || defined(posix_pwrite_USE_PWRITE)
	dssize_t result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef posix_pwrite_USE_PWRITE64
	result = (dssize_t)pwrite64(fd, buf, count, offset);
#else /* posix_pwrite_USE_PWRITE64 */
	result = (dssize_t)pwrite(fd, buf, count, offset);
#endif /* !posix_pwrite_USE_PWRITE64 */
	if (result == -1) {
		int error = DeeSystem_GetErrno();
		DBG_ALIGNMENT_ENABLE();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "pwrite")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to write to %d at offset %" PWRITE_PRIOFF,
		                          fd, offset);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	return result;
err:
	return -1;
#endif /* posix_pwrite_USE_PWRITE64 || posix_pwrite_USE_PWRITE */

#ifdef posix_pwrite_USE_WRITEFILE
	HANDLE h;
	DWORD bytes_written;
	my_OVERLAPPED overlapped;
	DBG_ALIGNMENT_DISABLE();
	h = (HANDLE)get_osfhandle(fd);
	DBG_ALIGNMENT_ENABLE();
	if (h == INVALID_HANDLE_VALUE) {
		DeeError_Throwf(&DeeError_FileClosed, "Invalid handle %d", fd);
		goto err;
	}
#if __SIZEOF_SIZE_T__ > 4
	if unlikely(count > UINT32_MAX)
		count = UINT32_MAX;
#endif /* __SIZEOF_SIZE_T__ > 4 */
again_WriteFile:
	DBG_ALIGNMENT_DISABLE();
	bzero(&overlapped, sizeof(overlapped));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	overlapped.me.Offset = (uint64_t)offset;
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
	overlapped.me.Offset     = (uint32_t)((uint64_t)offset);
	overlapped.me.OffsetHigh = (uint32_t)((uint64_t)offset >> 32);
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	if unlikely(!WriteFile(h, buf, (DWORD)count,
	                       &bytes_written,
	                       (LPOVERLAPPED)&overlapped)) {
		DWORD error = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsIntr(error)) {
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again_WriteFile;
		}
		DeeNTSystem_ThrowErrorf(NULL, error,
		                        "Failed to write to %d at offset %" PWRITE_PRIOFF,
		                        fd, offset);
		goto err;
	}
	DBG_ALIGNMENT_ENABLE();
	ASSERT((dssize_t)bytes_written != -1);
	return (dssize_t)bytes_written;
err:
	return -1;
#endif /* posix_pwrite_USE_WRITEFILE */

#ifdef posix_pwrite_USE_LSEEK_WRITE
	int error;
	dssize_t result;
	PWRITE_OFF_T oldpos, newpos;
HANDLE_EINTR(again)
	DBG_ALIGNMENT_DISABLE();
	oldpos = PWRITE_LSEEK(fd, 0, SEEK_CUR);
	if unlikely(oldpos == (PWRITE_OFF_T)-1)
		goto handle_system_error;
	newpos = PWRITE_LSEEK(fd, offset, SEEK_SET);
	if unlikely(newpos == (PWRITE_OFF_T)-1)
		goto handle_system_error;
	DBG_ALIGNMENT_ENABLE();
	result = write(fd, buf, count);
	if unlikely(result == -1) {
handle_system_error:
		error = DeeSystem_GetErrno();
		PWRITE_LSEEK(fd, oldpos, SEEK_SET);
		DBG_ALIGNMENT_ENABLE();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(error, err, "pwrite")
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_FSError, error,
		                          "Failed to write to %d at offset %" PWRITE_PRIOFF,
		                          fd, offset);
		goto err;
	}
	PWRITE_LSEEK(fd, oldpos, SEEK_SET);
	return result;
err:
	return -1;
#endif /* posix_pwrite_USE_LSEEK_WRITE */

#ifdef posix_pwrite_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)buf;
	(void)count;
	(void)offset;
	posix_err_unsupported("pwrite");
	return -1;
#endif /* posix_pwrite_USE_STUB */

}
#endif /* !posix_pwrite_USE_STUB */

#define POSIX_PWRITE_DEF                                          \
	{ "pwrite", (DeeObject *)&posix_pwrite, MODSYM_FNORMAL,       \
	  DOC("(fd:?X2?Dint?DFile,buf:?DBytes,offset:?Dint)->?Dint\n" \
	      "(fd:?X2?Dint?DFile,buf:?DBytes,count:?Dint,offset:?Dint)->?Dint") },
#define POSIX_PWRITE_DEF_DOC(doc)                                 \
	{ "pwrite", (DeeObject *)&posix_pwrite, MODSYM_FNORMAL,       \
	  DOC("(fd:?X2?Dint?DFile,buf:?DBytes,offset:?Dint)->?Dint\n" \
	      "(fd:?X2?Dint?DFile,buf:?DBytes,count:?Dint,offset:?Dint)->?Dint\n" doc) },
PRIVATE WUNUSED DREF DeeObject *DCALL posix_pwrite_f(size_t argc, DeeObject *const *argv) {
#ifndef posix_pwrite_USE_STUB
	dssize_t result;
	DeeObject *fd, *buf;
	DeeBuffer buffer;
	int fd_fd;
	PWRITE_OFF_T offset;
	if (argc == 3) {
		if (DeeArg_Unpack(argc, argv, "oo" PWRITE_PRIOFF ":pwrite", &fd, &buf, &offset))
			goto err;
		if (DeeObject_GetBuf(buf, &buffer, Dee_BUFFER_FREADONLY))
			goto err;
	} else {
		size_t count;
		if (DeeArg_Unpack(argc, argv, "ooIu" PWRITE_PRIOFF ":pwrite", &fd, &buf, &count, &offset))
			goto err;
		if (DeeObject_GetBuf(buf, &buffer, Dee_BUFFER_FREADONLY))
			goto err;
		if (buffer.bb_size > count)
			buffer.bb_size = count;
	}
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1) {
		DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FREADONLY);
		goto err;
	}
	result = posix_pwrite_f_impl(fd_fd,
	                             buffer.bb_base,
	                             buffer.bb_size,
	                             offset);
	DeeObject_PutBuf(buf, &buffer, Dee_BUFFER_FREADONLY);
	if unlikely(result == -1)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
#endif /* !posix_pwrite_USE_STUB */

#ifdef posix_pwrite_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)argc;
	(void)argv;
	posix_err_unsupported("pwrite");
	return NULL;
#endif /* posix_pwrite_USE_STUB */
}
PRIVATE DEFINE_CMETHOD(posix_pwrite, posix_pwrite_f);



DECL_END

#endif /* !GUARD_DEX_POSIX_P_READWRITE_C_INL */
