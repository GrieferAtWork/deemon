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
#ifndef GUARD_DEX_POSIX_P_TRUNCATE_C_INL
#define GUARD_DEX_POSIX_P_TRUNCATE_C_INL 1
#define CONFIG_BUILDING_LIBPOSIX 1
#define DEE_SOURCE 1

#include "libposix.h"

DECL_BEGIN

/* Figure out how we want to implement `truncate()' */
#if defined(CONFIG_HAVE_wtruncate64)
#define posix_truncate_USE_WTRUNCATE64 1
#elif defined(CONFIG_HAVE_wtruncate)
#define posix_truncate_USE_WTRUNCATE 1
#elif defined(CONFIG_HAVE_truncate64)
#define posix_truncate_USE_TRUNCATE64 1
#elif defined(CONFIG_HAVE_truncate)
#define posix_truncate_USE_TRUNCATE 1
#elif (defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen)) && defined(CONFIG_HAVE_ftruncate64) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_truncate_USE_WOPEN_FTRUNCATE64 1
#elif (defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen)) && defined(CONFIG_HAVE_ftruncate) && defined(CONFIG_PREFER_WCHAR_FUNCTIONS)
#define posix_truncate_USE_WOPEN_FTRUNCATE 1
#elif (defined(CONFIG_HAVE_open64) || defined(CONFIG_HAVE_open)) && defined(CONFIG_HAVE_ftruncate64)
#define posix_truncate_USE_OPEN_FTRUNCATE64 1
#elif (defined(CONFIG_HAVE_open64) || defined(CONFIG_HAVE_open)) && defined(CONFIG_HAVE_ftruncate)
#define posix_truncate_USE_OPEN_FTRUNCATE 1
#elif (defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen)) && defined(CONFIG_HAVE_ftruncate64)
#define posix_truncate_USE_WOPEN_FTRUNCATE64 1
#elif (defined(CONFIG_HAVE_wopen64) || defined(CONFIG_HAVE_wopen)) && defined(CONFIG_HAVE_ftruncate)
#define posix_truncate_USE_WOPEN_FTRUNCATE 1
#else
#define posix_truncate_USE_STUB 1
#endif


#if defined(posix_truncate_USE_WTRUNCATE64) || defined(posix_truncate_USE_WOPEN_FTRUNCATE64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("truncate", "filename:c:wchar_t[],len:I64d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(dwchar_t const *filename, int64_t len);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)") },
#define POSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_truncate, posix_truncate_f);
#ifndef POSIX_KWDS_FILENAME_LEN_DEFINED
#define POSIX_KWDS_FILENAME_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_len, { K(filename), K(len), KEND });
#endif /* !POSIX_KWDS_FILENAME_LEN_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	int64_t len;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_len, "oI64d:truncate", &filename, &len))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_truncate_f_impl(filename_str, len);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(dwchar_t const *filename, int64_t len)
//[[[end]]]
#endif /* posix_truncate_USE_WTRUNCATE64 || posix_truncate_USE_WOPEN_FTRUNCATE64 */
#if (!(defined(posix_truncate_USE_WTRUNCATE64) || defined(posix_truncate_USE_WOPEN_FTRUNCATE64)) && \
      (defined(posix_truncate_USE_WTRUNCATE) || defined(posix_truncate_USE_WOPEN_FTRUNCATE))) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("truncate", "filename:c:wchar_t[],len:I32d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(dwchar_t const *filename, int32_t len);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)") },
#define POSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_truncate, posix_truncate_f);
#ifndef POSIX_KWDS_FILENAME_LEN_DEFINED
#define POSIX_KWDS_FILENAME_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_len, { K(filename), K(len), KEND });
#endif /* !POSIX_KWDS_FILENAME_LEN_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	dwchar_t const *filename_str;
	DeeStringObject *filename;
	int32_t len;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_len, "oI32d:truncate", &filename, &len))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = (dwchar_t const *)DeeString_AsWide((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_truncate_f_impl(filename_str, len);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(dwchar_t const *filename, int32_t len)
//[[[end]]]
#endif /* posix_truncate_USE_WTRUNCATE || posix_truncate_USE_WOPEN_FTRUNCATE */
#if (!(defined(posix_truncate_USE_WTRUNCATE64) || defined(posix_truncate_USE_WOPEN_FTRUNCATE64) || \
       defined(posix_truncate_USE_WTRUNCATE) || defined(posix_truncate_USE_WOPEN_FTRUNCATE)) && \
      (defined(posix_truncate_USE_TRUNCATE64) || defined(posix_truncate_USE_OPEN_FTRUNCATE64))) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("truncate", "filename:c:char[],len:I64d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(/*utf-8*/ char const *filename, int64_t len);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)") },
#define POSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_truncate, posix_truncate_f);
#ifndef POSIX_KWDS_FILENAME_LEN_DEFINED
#define POSIX_KWDS_FILENAME_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_len, { K(filename), K(len), KEND });
#endif /* !POSIX_KWDS_FILENAME_LEN_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	int64_t len;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_len, "oI64d:truncate", &filename, &len))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_truncate_f_impl(filename_str, len);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(/*utf-8*/ char const *filename, int64_t len)
//[[[end]]]
#endif /* posix_truncate_USE_TRUNCATE64 || posix_truncate_USE_OPEN_FTRUNCATE64 */
#if (!(defined(posix_truncate_USE_WTRUNCATE64) || defined(posix_truncate_USE_WOPEN_FTRUNCATE64) || \
       defined(posix_truncate_USE_WTRUNCATE) || defined(posix_truncate_USE_WOPEN_FTRUNCATE) || \
       defined(posix_truncate_USE_TRUNCATE64) || defined(posix_truncate_USE_OPEN_FTRUNCATE64))) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("truncate", "filename:c:char[],len:I32d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(/*utf-8*/ char const *filename, int32_t len);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_TRUNCATE_DEF { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)") },
#define POSIX_TRUNCATE_DEF_DOC(doc) { "truncate", (DeeObject *)&posix_truncate, MODSYM_FNORMAL, DOC("(filename:?Dstring,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_truncate, posix_truncate_f);
#ifndef POSIX_KWDS_FILENAME_LEN_DEFINED
#define POSIX_KWDS_FILENAME_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_filename_len, { K(filename), K(len), KEND });
#endif /* !POSIX_KWDS_FILENAME_LEN_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_truncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	/*utf-8*/ char const *filename_str;
	DeeStringObject *filename;
	int32_t len;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_filename_len, "oI32d:truncate", &filename, &len))
	    goto err;
	if (DeeObject_AssertTypeExact(filename, &DeeString_Type))
	    goto err;
	filename_str = DeeString_AsUtf8((DeeObject *)filename);
	if unlikely(!filename_str)
	    goto err;
	return posix_truncate_f_impl(filename_str, len);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_truncate_f_impl(/*utf-8*/ char const *filename, int32_t len)
//[[[end]]]
#endif /* posix_truncate_USE_TRUNCATE || TRUNCATE_IMPL_WOPEN_TRUNCATE */
{
#ifdef posix_truncate_USE_STUB
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)len;
	posix_err_unsupported("truncate");
#else /* posix_truncate_USE_STUB */
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#if defined(posix_truncate_USE_WTRUNCATE64)
#define TRUNCATE_PRINTF_FILENAME "%lq"
	result = wtruncate64(filename, len);
#elif defined(posix_truncate_USE_WTRUNCATE)
#define TRUNCATE_PRINTF_FILENAME "%lq"
	result = wtruncate(filename, len);
#elif defined(posix_truncate_USE_TRUNCATE64)
#define TRUNCATE_PRINTF_FILENAME "%q"
	result = truncate64(filename, len);
#elif defined(posix_truncate_USE_TRUNCATE)
#define TRUNCATE_PRINTF_FILENAME "%q"
	result = truncate(filename, len);
#else /* Single-call */
	{
#if defined(posix_truncate_USE_WOPEN_FTRUNCATE) || \
    defined(posix_truncate_USE_WOPEN_FTRUNCATE64)
#define TRUNCATE_PRINTF_FILENAME "%lq"
#ifdef CONFIG_HAVE_wopen64
		int fd = wopen64(filename, O_RDWR);
#else /* CONFIG_HAVE_wopen64 */
		int fd = wopen(filename, O_RDWR);
#endif /* !CONFIG_HAVE_wopen64 */
#else
#define TRUNCATE_PRINTF_FILENAME "%q"
#ifdef CONFIG_HAVE_open64
		int fd = open64(filename, O_RDWR);
#else /* CONFIG_HAVE_open64 */
		int fd = open(filename, O_RDWR);
#endif /* !CONFIG_HAVE_open64 */
#endif
		result = fd;
		if (fd >= 0) {
#if defined(posix_truncate_USE_OPEN_FTRUNCATE64) || \
    defined(posix_truncate_USE_WOPEN_FTRUNCATE64)
			result = ftruncate64(fd, len);
#else
			result = ftruncate(fd, len);
#endif
#ifdef CONFIG_HAVE_close
			close(fd);
#endif /* CONFIG_HAVE_close */
		}
	}
#endif /* FD-wrapped */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		result = DeeSystem_GetErrno();
		HANDLE_EINTR(result, again, err)
		HANDLE_ENOENT_ENOTDIR(result, err, "File or directory " TRUNCATE_PRINTF_FILENAME " could not be found", filename)
		HANDLE_ENOSYS(result, err, "truncate")
		HANDLE_EACCES(result, err, "Failed to access " TRUNCATE_PRINTF_FILENAME, filename)
		HANDLE_EFBIG_EINVAL(result, err, "Cannot truncate " TRUNCATE_PRINTF_FILENAME ": Invalid size %I64d", filename, len)
		HANDLE_ENXIO_EISDIR(result, err, "Cannot truncate directory " TRUNCATE_PRINTF_FILENAME, filename)
		HANDLE_EROFS_ETXTBSY(result, err, "Read-only file " TRUNCATE_PRINTF_FILENAME, filename)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, result,
		                          "Failed to truncate " TRUNCATE_PRINTF_FILENAME,
		                          filename);
		goto err;
	}
	return_none;
err:
#endif /* !posix_truncate_USE_STUB */
	return NULL;
}



/* TOOD: Use the posix_xxx_USE_XXX notation for ftruncate() */

#if defined(CONFIG_HAVE_ftruncate64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("ftruncate", "fd:unix:fd,len:I64d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_ftruncate_f_impl(int fd, int64_t len);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_ftruncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FTRUNCATE_DEF { "ftruncate", (DeeObject *)&posix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,len:?Dint)") },
#define POSIX_FTRUNCATE_DEF_DOC(doc) { "ftruncate", (DeeObject *)&posix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_ftruncate, posix_ftruncate_f);
#ifndef POSIX_KWDS_FD_LEN_DEFINED
#define POSIX_KWDS_FD_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_len, { K(fd), K(len), KEND });
#endif /* !POSIX_KWDS_FD_LEN_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_ftruncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	int64_t len;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_len, "oI64d:ftruncate", &fd, &len))
	    goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
	    goto err;
	return posix_ftruncate_f_impl(fd_fd, len);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_ftruncate_f_impl(int fd, int64_t len)
//[[[end]]]
#endif /* CONFIG_HAVE_ftruncate64 */
#if !defined(CONFIG_HAVE_ftruncate64) || defined(__DEEMON__)
/*[[[deemon import("_dexutils").gw("ftruncate", "fd:unix:fd,len:I32d", libname: "posix"); ]]]*/
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_ftruncate_f_impl(int fd, int32_t len);
PRIVATE WUNUSED DREF DeeObject *DCALL posix_ftruncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw);
#define POSIX_FTRUNCATE_DEF { "ftruncate", (DeeObject *)&posix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,len:?Dint)") },
#define POSIX_FTRUNCATE_DEF_DOC(doc) { "ftruncate", (DeeObject *)&posix_ftruncate, MODSYM_FNORMAL, DOC("(fd:?X2?Dint?DFile,len:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(posix_ftruncate, posix_ftruncate_f);
#ifndef POSIX_KWDS_FD_LEN_DEFINED
#define POSIX_KWDS_FD_LEN_DEFINED 1
PRIVATE DEFINE_KWLIST(posix_kwds_fd_len, { K(fd), K(len), KEND });
#endif /* !POSIX_KWDS_FD_LEN_DEFINED */
PRIVATE WUNUSED DREF DeeObject *DCALL posix_ftruncate_f(size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int fd_fd;
	DeeObject *fd;
	int32_t len;
	if (DeeArg_UnpackKw(argc, argv, kw, posix_kwds_fd_len, "oI32d:ftruncate", &fd, &len))
	    goto err;
	fd_fd = DeeUnixSystem_GetFD(fd);
	if unlikely(fd_fd == -1)
	    goto err;
	return posix_ftruncate_f_impl(fd_fd, len);
err:
	return NULL;
}
FORCELOCAL WUNUSED DREF DeeObject *DCALL posix_ftruncate_f_impl(int fd, int32_t len)
//[[[end]]]
#endif /* !CONFIG_HAVE_ftruncate64 */
{
#if defined(CONFIG_HAVE_ftruncate) || defined(CONFIG_HAVE_ftruncate64)
	int result;
EINTR_LABEL(again)
	DBG_ALIGNMENT_DISABLE();
#ifdef CONFIG_HAVE_ftruncate64
	result = ftruncate64(fd, len);
#else /* CONFIG_HAVE_ftruncate64 */
	result = ftruncate(fd, len);
#endif /* !CONFIG_HAVE_ftruncate64 */
	DBG_ALIGNMENT_ENABLE();
	if (result < 0) {
		int error = DeeSystem_GetErrno();
		HANDLE_EINTR(error, again, err)
		HANDLE_ENOSYS(result, err, "ftruncate")
		HANDLE_EFBIG_EINVAL(result, err, "Cannot truncate %d: Invalid size %I64d", fd, len)
		HANDLE_EBADF(error, err, "Invalid handle %d", fd)
		DeeUnixSystem_ThrowErrorf(&DeeError_SystemError, error,
		                          "Failed to truncate %d", fd);
		goto err;
	}
	return_none;
err:
#else /* CONFIG_HAVE_ftruncate || CONFIG_HAVE_ftruncate64 */
	/* TODO: Implement using `truncate(frealpath(fd), ...)' */
#define NEED_ERR_UNSUPPORTED 1
	(void)fd;
	(void)len;
	posix_err_unsupported("ftruncate");
#endif /* !CONFIG_HAVE_ftruncate && !CONFIG_HAVE_ftruncate64 */
	return NULL;
}


DECL_END

#endif /* !GUARD_DEX_POSIX_P_TRUNCATE_C_INL */
