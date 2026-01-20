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
#ifndef GUARD_DEEMON_SYSTEM_SYSTEM_NT_C_INL
#define GUARD_DEEMON_SYSTEM_SYSTEM_NT_C_INL 1

#include <deemon/api.h>

/* NT-specific system functions */

#ifdef CONFIG_HOST_WINDOWS

#include <deemon/alloc.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/file.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/system.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>

#include <hybrid/align.h>
#include <hybrid/debug-alignment.h>
#include <hybrid/typecore.h>
#include <hybrid/unaligned.h>
#include <hybrid/wordbits.h>

#include "../runtime/strings.h"

#include <stdarg.h>  /* va_end, va_list, va_start */
#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* UINT32_C, uint8_t, uint16_t, uint32_t, uintptr_t */
/**/

#include <Windows.h>
/**/

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

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifdef _WIN32_WCE
#undef GetProcAddress
#define GetProcAddress GetProcAddressA
#endif /* _WIN32_WCE */

#ifndef CONFIG_HAVE_wcsend
#ifndef CONFIG_HAVE_wcslen
#define CONFIG_HAVE_wcslen
#undef wcslen
#define wcslen dee_wcslen
DeeSystem_DEFINE_wcslen(dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#undef wcsend
#define wcsend(str) ((str) + wcslen(str))
#endif /* !CONFIG_HAVE_wcsend */

#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */


#if defined(Dee_fd_GETSET) && defined(Dee_fd_t_IS_HANDLE)
#define GETATTR_osfhandle(ob) DeeObject_GetAttr(ob, Dee_AsObject(&str_getsysfd))
#else /* Dee_fd_GETSET && Dee_fd_t_IS_HANDLE */
#define GETATTR_osfhandle(ob) DeeObject_GetAttrString(ob, Dee_fd_osfhandle_GETSET)
#endif /* !Dee_fd_GETSET || !Dee_fd_t_IS_HANDLE */

#ifndef GETATTR_fileno
#if defined(Dee_fd_GETSET) && defined(Dee_fd_t_IS_FILE)
#define GETATTR_fileno(ob) DeeObject_GetAttr(ob, Dee_AsObject(&str_getsysfd))
#else /* Dee_fd_GETSET && Dee_fd_t_IS_FILE */
#define GETATTR_fileno(ob) DeeObject_GetAttrString(ob, Dee_fd_fileno_GETSET)
#endif /* !Dee_fd_GETSET || !Dee_fd_t_IS_FILE */
#endif /* !GETATTR_fileno */


/* Retrieve the Windows handle associated with a given object.
 * The translation is done by performing the following:
 * >> #ifdef Dee_fd_t_IS_HANDLE
 * >> if (DeeFile_Check(ob))
 * >>     return DeeFile_GetSysFD(ob);
 * >> #endif
 * >> #ifdef Dee_fd_t_IS_int
 * >> if (DeeFile_Check(ob))
 * >>     return get_osfhandle(DeeFile_GetSysFD(ob));
 * >> #endif
 * >> if (DeeNone_Check(ob))
 * >>     return (void *)(HANDLE)NULL;
 * >> if (DeeInt_Check(ob))
 * >>     return get_osfhandle(DeeInt_AsInt(ob));
 * >> try return DeeObject_AsInt(DeeObject_GetAttr(ob, Dee_fd_osfhandle_GETSET)); catch (AttributeError);
 * >> try return get_osfhandle(DeeObject_AsInt(DeeObject_GetAttr(ob, Dee_fd_fileno_GETSET))); catch (AttributeError);
 * >> return get_osfhandle(DeeObject_AsInt(ob));
 * Note that both msvc, as well as cygwin define `get_osfhandle()' as one
 * of the available functions, meaning that in both scenarios we are able
 * to get access to the underlying HANDLE. However, should deemon ever be
 * linked against a windows libc without this function, then only the
 * `Dee_fd_osfhandle_GETSET' variant will be usable.
 * @return: * :                   Success (the actual handle value)
 * @return: INVALID_HANDLE_VALUE: Error (handle translation failed)
 *                                In case the actual handle value stored inside
 *                                of `ob' was `INVALID_HANDLE_VALUE', then an
 *                                `DeeError_FileClosed' error is thrown. */
PUBLIC WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_GetHandle(DeeObject *__restrict ob) {
	return DeeNTSystem_GetHandleEx(ob, NULL);
}

/* Same as `DeeNTSystem_GetHandleEx()', but also writes to `p_fd' (when non-NULL):
 * - `-1': If `get_osfhandle()' wasn't used
 * - `*':  The file descriptor number passed to `get_osfhandle()' */
PUBLIC WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_GetHandleEx(DeeObject *__restrict ob, int *p_fd) {
	DREF DeeObject *attr;
#if (defined(Dee_fd_t_IS_HANDLE) || \
     (defined(Dee_fd_t_IS_int) && defined(CONFIG_HAVE_get_osfhandle)))
	if (DeeFile_Check(ob)) {
		Dee_fd_t sysfd;
		if (p_fd)
			*p_fd = -1;
		sysfd = DeeFile_GetSysFD(ob);
		if (sysfd == Dee_fd_INVALID)
			return INVALID_HANDLE_VALUE;
#ifdef Dee_fd_t_IS_HANDLE
		return (void *)(HANDLE)sysfd;
#endif /* Dee_fd_t_IS_HANDLE */

#if defined(Dee_fd_t_IS_int) && defined(CONFIG_HAVE_get_osfhandle)
		{
			HANDLE hResult;
			hResult = (HANDLE)get_osfhandle(sysfd);
			if unlikely(hResult == INVALID_HANDLE_VALUE) {
				DeeError_Throwf(&DeeError_FileClosed,
				                "File descriptor %d bound by %r was closed",
				                sysfd, ob);
			}
			return (void *)hResult;
		}
#endif /* Dee_fd_t_IS_int && CONFIG_HAVE_get_osfhandle */
	}
#endif /* Dee_fd_t_IS_HANDLE || Dee_fd_t_IS_int */

#ifdef CONFIG_HAVE_get_osfhandle
	if (DeeInt_Check(ob)) {
		HANDLE hResult;
		int fd;
		if (DeeInt_AsInt(ob, &fd)) {
			if (p_fd)
				*p_fd = -1;
			return INVALID_HANDLE_VALUE;
		}
		if (p_fd)
			*p_fd = fd;
		hResult = (HANDLE)get_osfhandle(fd);
		if unlikely(hResult == INVALID_HANDLE_VALUE) {
			DeeError_Throwf(&DeeError_FileClosed,
			                "File descriptor %d was closed",
			                fd);
		}
		return (void *)hResult;
	}
#endif /* CONFIG_HAVE_get_osfhandle */

	if (DeeNone_Check(ob)) {
		if (p_fd)
			*p_fd = -1;
		return (void *)(HANDLE)NULL;
	}

	attr = GETATTR_osfhandle(ob);
	if (attr) {
		HANDLE hResult;
		int error;
		error = DeeObject_AsUIntptr(attr, (uintptr_t *)&hResult);
		Dee_Decref(attr);
		if unlikely(error)
			goto err;
		if unlikely(hResult == INVALID_HANDLE_VALUE) {
			DeeError_Throwf(&DeeError_FileClosed,
			                "Handle pointed-to by %r was closed",
			                ob);
		}
		if (p_fd)
			*p_fd = -1;
		return hResult;
	}

#ifdef CONFIG_HAVE_get_osfhandle
	if (DeeError_Catch(&DeeError_AttributeError) ||
	    DeeError_Catch(&DeeError_NotImplemented)) {
		HANDLE hResult;
		int fd, error;
		attr = GETATTR_fileno(ob);
		if (attr) {
			error = DeeObject_AsInt(attr, &fd);
			Dee_Decref(attr);
		} else {
			if (!DeeError_Catch(&DeeError_AttributeError) &&
			    !DeeError_Catch(&DeeError_NotImplemented))
				goto err;
			/* Fallback: Convert an `int'-object into a unix file descriptor. */
			error = DeeObject_AsInt(ob, &fd);
		}
		if unlikely(error)
			goto err;
		if (p_fd)
			*p_fd = fd;
		hResult = (HANDLE)get_osfhandle(fd);
		if unlikely(hResult == INVALID_HANDLE_VALUE) {
			DeeError_Throwf(&DeeError_FileClosed,
			                "File descriptor %d bound by %r was closed",
			                fd, ob);
		}
		return (void *)hResult;
	}
#endif /* CONFIG_HAVE_get_osfhandle */

err:
	if (p_fd)
		*p_fd = -1;
	return INVALID_HANDLE_VALUE;
}


/* Similar to `DeeNTSystem_GetHandle()', but allow `ob' to refer to INVALID_HANDLE_VALUE,
 * instead of unconditionally throwing an `DeeError_FileClosed' error when such a handle
 * value is encountered.
 * @return: 0:  Success (the handle value was stored in `*pHandle', and is allowed to be `INVALID_HANDLE_VALUE')
 * @return: -1: Error (a deemon error was thrown; s.a. `DeeError_Throw()') */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeNTSystem_TryGetHandle)(DeeObject *__restrict ob,
                                 /*PHANDLE*/ void **pHandle) {
	DREF DeeObject *attr;
#if (defined(Dee_fd_t_IS_HANDLE) || \
     (defined(Dee_fd_t_IS_int) && defined(CONFIG_HAVE_get_osfhandle)))
	if (DeeFile_Check(ob)) {
		Dee_fd_t sysfd;
		sysfd = DeeFile_GetSysFD(ob);
		if (sysfd == Dee_fd_INVALID) {
			if (!DeeError_Catch(&DeeError_FileClosed))
				goto err;
			*pHandle = INVALID_HANDLE_VALUE;
			return 0;
		}
#ifdef Dee_fd_t_IS_HANDLE
		*pHandle = (void *)(HANDLE)sysfd;
		return 0;
#endif /* Dee_fd_t_IS_HANDLE */

#if defined(Dee_fd_t_IS_int) && defined(CONFIG_HAVE_get_osfhandle)
		*pHandle = (void *)(HANDLE)get_osfhandle(sysfd);
		return 0;
#endif /* Dee_fd_t_IS_int && CONFIG_HAVE_get_osfhandle */
	}
#endif /* Dee_fd_t_IS_HANDLE || Dee_fd_t_IS_int */

#ifdef CONFIG_HAVE_get_osfhandle
	if (DeeInt_Check(ob)) {
		int fd;
		if (DeeInt_AsInt(ob, &fd))
			goto err;
		*pHandle = (void *)(HANDLE)get_osfhandle(fd);
		return 0;
	}
#endif /* CONFIG_HAVE_get_osfhandle */

	if (DeeNone_Check(ob)) {
		*pHandle = (void *)(HANDLE)NULL;
		return 0;
	}

	attr = GETATTR_osfhandle(ob);
	if (attr) {
		int result;
		result = DeeObject_AsUIntptr(attr, (uintptr_t *)pHandle);
		Dee_Decref(attr);
		return result;
	}

#ifdef CONFIG_HAVE_get_osfhandle
	if (DeeError_Catch(&DeeError_AttributeError) ||
	    DeeError_Catch(&DeeError_NotImplemented)) {
		int fd, error;
		attr = GETATTR_fileno(ob);
		if (attr) {
			error = DeeObject_AsInt(attr, &fd);
			Dee_Decref(attr);
		} else {
			if (!DeeError_Catch(&DeeError_AttributeError) &&
			    !DeeError_Catch(&DeeError_NotImplemented))
				goto err;
			/* Fallback: Convert an `int'-object into a unix file descriptor. */
			error = DeeObject_AsInt(ob, &fd);
		}
		if unlikely(error)
			goto err;
		*pHandle = (void *)(HANDLE)get_osfhandle(fd);
		return 0;
	}
#endif /* CONFIG_HAVE_get_osfhandle */

err:
	return -1;
}



/* Fix the given filename and extend it to an absolute UNC path. */
PUBLIC WUNUSED NONNULL((1)) DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FixUncPath(/*String*/ DeeObject *__restrict filename) {
	DREF DeeObject *result;
	size_t filename_size;
	ASSERT_OBJECT_TYPE_EXACT(filename, &DeeString_Type);
	filename = DeeSystem_MakeNormalAndAbsolute(filename);
	if unlikely(!filename)
		goto err;
	filename_size = DeeString_SIZE(filename);
	if (filename_size < 4 ||
	    UNALIGNED_GET32(DeeString_STR(filename)) != ENCODE_INT32('\\', '\\', '.', '\\')) {
		if (!DeeObject_IsShared(filename) &&
		    DeeString_WIDTH(filename) == STRING_WIDTH_1BYTE) {
			DeeString_FreeWidth(filename);
			result = DeeString_ResizeBuffer(filename, 4 + filename_size);
			if unlikely(!result)
				goto err_filename;
			memmoveupc(DeeString_GetBuffer(result) + 4,
			           DeeString_GetBuffer(result),
			           filename_size,
			           sizeof(char));
			/* Set the prefix. */
			UNALIGNED_SET32(DeeString_GetBuffer(result),
			                ENCODE_INT32('\\', '\\', '.', '\\'));
			return result;
		} else {
			struct unicode_printer printer = UNICODE_PRINTER_INIT;
			/* Prepend "\\.\". */
			if unlikely(unicode_printer_print8(&printer, (uint8_t *)"\\\\.\\", 4) < 0)
				goto err_printer;
			if unlikely(unicode_printer_printstring(&printer, filename) < 0)
				goto err_printer;
			result = unicode_printer_pack(&printer);
			Dee_Decref(filename);
			return result;
err_printer:
			unicode_printer_fini(&printer);
			Dee_Decref(filename);
			return NULL;
		}
	}
	return filename;
err_filename:
	Dee_Decref(filename);
err:
	return NULL;
}


/* Check if a given error code indicates a UNC-path problem that should be
 * addressed by fixing the path using `DeeNTSystem_FixUncPath()', then trying again. */
PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsUncError(DeeNT_DWORD error) {
	switch (error) {

		/* TODO: Figure out the real UNC error codes. */
#ifdef ERROR_FILE_NOT_FOUND
	case ERROR_FILE_NOT_FOUND:
#endif /* ERROR_FILE_NOT_FOUND */
#ifdef ERROR_PATH_NOT_FOUND
	case ERROR_PATH_NOT_FOUND:
#endif /* ERROR_PATH_NOT_FOUND */
#ifdef ERROR_ACCESS_DENIED
	case ERROR_ACCESS_DENIED:
#endif /* ERROR_ACCESS_DENIED */
#ifdef ERROR_INVALID_ACCESS
	/*case ERROR_INVALID_ACCESS:*/
#endif /* ERROR_INVALID_ACCESS */
#ifdef ERROR_INVALID_DRIVE
	case ERROR_INVALID_DRIVE:
#endif /* ERROR_INVALID_DRIVE */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsFileNotFoundError(DeeNT_DWORD error) {
	switch (error) {

		/* XXX: Check if these are all the possible
		 *      invalid-path / file-not-found errors. */
#ifdef ERROR_FILE_NOT_FOUND
	case ERROR_FILE_NOT_FOUND:
#endif /* ERROR_FILE_NOT_FOUND */
#ifdef ERROR_PATH_NOT_FOUND
	case ERROR_PATH_NOT_FOUND:
#endif /* ERROR_PATH_NOT_FOUND */
#ifdef ERROR_INVALID_DRIVE
	case ERROR_INVALID_DRIVE:
#endif /* ERROR_INVALID_DRIVE */
#ifdef ERROR_BAD_NETPATH
	case ERROR_BAD_NETPATH:
#endif /* ERROR_BAD_NETPATH */
#ifdef ERROR_BAD_PATHNAME
	case ERROR_BAD_PATHNAME:
#endif /* ERROR_BAD_PATHNAME */
#ifdef ERROR_INVALID_NAME
	case ERROR_INVALID_NAME:
#endif /* ERROR_INVALID_NAME */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsAccessDeniedError(DeeNT_DWORD error) {
	switch (error) {

#ifdef ERROR_ACCESS_DENIED
	case ERROR_ACCESS_DENIED:
#endif /* ERROR_ACCESS_DENIED */
#ifdef ERROR_CANT_ACCESS_FILE
	case ERROR_CANT_ACCESS_FILE:
#endif /* ERROR_CANT_ACCESS_FILE */
#ifdef ERROR_CTX_WINSTATION_ACCESS_DENIED
	case ERROR_CTX_WINSTATION_ACCESS_DENIED:
#endif /* ERROR_CTX_WINSTATION_ACCESS_DENIED */
#ifdef ERROR_DS_DRA_ACCESS_DENIED
	case ERROR_DS_DRA_ACCESS_DENIED:
#endif /* ERROR_DS_DRA_ACCESS_DENIED */
#ifdef ERROR_DS_INSUFF_ACCESS_RIGHTS
	case ERROR_DS_INSUFF_ACCESS_RIGHTS:
#endif /* ERROR_DS_INSUFF_ACCESS_RIGHTS */
#ifdef ERROR_EA_ACCESS_DENIED
	case ERROR_EA_ACCESS_DENIED:
#endif /* ERROR_EA_ACCESS_DENIED */
#ifdef ERROR_HV_ACCESS_DENIED
	case ERROR_HV_ACCESS_DENIED:
#endif /* ERROR_HV_ACCESS_DENIED */
#ifdef ERROR_NETWORK_ACCESS_DENIED
	case ERROR_NETWORK_ACCESS_DENIED:
#endif /* ERROR_NETWORK_ACCESS_DENIED */
#ifdef ERROR_NO_ADMIN_ACCESS_POINT
	case ERROR_NO_ADMIN_ACCESS_POINT:
#endif /* ERROR_NO_ADMIN_ACCESS_POINT */
#ifdef ERROR_VHD_PARENT_VHD_ACCESS_DENIED
	case ERROR_VHD_PARENT_VHD_ACCESS_DENIED:
#endif /* ERROR_VHD_PARENT_VHD_ACCESS_DENIED */
#ifdef ERROR_ACCESS_DISABLED_BY_POLICY
	case ERROR_ACCESS_DISABLED_BY_POLICY:
#endif /* ERROR_ACCESS_DISABLED_BY_POLICY */
#ifdef ERROR_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY
	case ERROR_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY:
#endif /* ERROR_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY */
#ifdef ERROR_ACCESS_DISABLED_WEBBLADE
	case ERROR_ACCESS_DISABLED_WEBBLADE:
#endif /* ERROR_ACCESS_DISABLED_WEBBLADE */
#ifdef ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER
	case ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER:
#endif /* ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER */
#ifdef ERROR_CANT_ACCESS_DOMAIN_INFO
	case ERROR_CANT_ACCESS_DOMAIN_INFO:
#endif /* ERROR_CANT_ACCESS_DOMAIN_INFO */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsBadAllocError(DeeNT_DWORD error) {
	switch (error) {

#ifdef ERROR_OUTOFMEMORY
	case ERROR_OUTOFMEMORY:
#endif /* !ERROR_OUTOFMEMORY */
#ifdef ERROR_NOT_ENOUGH_SERVER_MEMORY
	case ERROR_NOT_ENOUGH_SERVER_MEMORY:
#endif /* !ERROR_NOT_ENOUGH_SERVER_MEMORY */
#ifdef ERROR_IPSEC_IKE_OUT_OF_MEMORY
	case ERROR_IPSEC_IKE_OUT_OF_MEMORY:
#endif /* !ERROR_IPSEC_IKE_OUT_OF_MEMORY */
#ifdef E_OUTOFMEMORY
	case E_OUTOFMEMORY:
#endif /* !E_OUTOFMEMORY */
#ifdef CO_E_INIT_MEMORY_ALLOCATOR
	case CO_E_INIT_MEMORY_ALLOCATOR:
#endif /* !CO_E_INIT_MEMORY_ALLOCATOR */
#ifdef STG_E_INSUFFICIENTMEMORY
	case STG_E_INSUFFICIENTMEMORY:
#endif /* !STG_E_INSUFFICIENTMEMORY */
#ifdef NTE_NO_MEMORY
	case NTE_NO_MEMORY:
#endif /* !NTE_NO_MEMORY */
#ifdef SEC_E_INSUFFICIENT_MEMORY
	case SEC_E_INSUFFICIENT_MEMORY:
#endif /* !SEC_E_INSUFFICIENT_MEMORY */
#ifdef OSS_OUT_MEMORY
	case OSS_OUT_MEMORY:
#endif /* !OSS_OUT_MEMORY */
#ifdef CRYPT_E_ASN1_MEMORY
	case CRYPT_E_ASN1_MEMORY:
#endif /* !CRYPT_E_ASN1_MEMORY */
#ifdef SCARD_E_NO_MEMORY
	case SCARD_E_NO_MEMORY:
#endif /* !SCARD_E_NO_MEMORY */
#ifdef ERROR_GRAPHICS_NO_VIDEO_MEMORY
	case ERROR_GRAPHICS_NO_VIDEO_MEMORY:
#endif /* !ERROR_GRAPHICS_NO_VIDEO_MEMORY */
#ifdef TPMAPI_E_OUT_OF_MEMORY
	case TPMAPI_E_OUT_OF_MEMORY:
#endif /* !TPMAPI_E_OUT_OF_MEMORY */
#ifdef TBSIMP_E_OUT_OF_MEMORY
	case TBSIMP_E_OUT_OF_MEMORY:
#endif /* !TBSIMP_E_OUT_OF_MEMORY */
#ifdef ERROR_HV_INSUFFICIENT_MEMORY
	case ERROR_HV_INSUFFICIENT_MEMORY:
#endif /* !ERROR_HV_INSUFFICIENT_MEMORY */
#ifdef E_MBN_SMS_MEMORY_FULL
	case E_MBN_SMS_MEMORY_FULL:
#endif /* !E_MBN_SMS_MEMORY_FULL */
#ifdef DXGI_ERROR_REMOTE_OUTOFMEMORY
	case DXGI_ERROR_REMOTE_OUTOFMEMORY:
#endif /* !DXGI_ERROR_REMOTE_OUTOFMEMORY */
#ifdef UCEERR_MEMORYFAILURE
	case UCEERR_MEMORYFAILURE:
#endif /* !UCEERR_MEMORYFAILURE */
#if defined(DNS_ERROR_NO_MEMORY) && DNS_ERROR_NO_MEMORY != ERROR_OUTOFMEMORY
	case DNS_ERROR_NO_MEMORY:
#endif /* !ERROR_OUTOFMEMORY */
		return true;

	default: break;
	}
	return false;
}




PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsBusy(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_CURRENT_DIRECTORY
	case ERROR_CURRENT_DIRECTORY:
#endif /* !ERROR_CURRENT_DIRECTORY */
#ifdef ERROR_BUSY
	case ERROR_BUSY:
#endif /* !ERROR_BUSY */
#ifdef ERROR_NETWORK_BUSY
	case ERROR_NETWORK_BUSY:
#endif /* !ERROR_NETWORK_BUSY */
#ifdef ERROR_DEVICE_SUPPORT_IN_PROGRESS
	case ERROR_DEVICE_SUPPORT_IN_PROGRESS:
#endif /* !ERROR_DEVICE_SUPPORT_IN_PROGRESS */
#ifdef DXGI_ERROR_MODE_CHANGE_IN_PROGRESS
	case DXGI_ERROR_MODE_CHANGE_IN_PROGRESS:
#endif /* !DXGI_ERROR_MODE_CHANGE_IN_PROGRESS */
#ifdef ERROR_CLUSTER_BACKUP_IN_PROGRESS
	case ERROR_CLUSTER_BACKUP_IN_PROGRESS:
#endif /* !ERROR_CLUSTER_BACKUP_IN_PROGRESS */
#ifdef ERROR_CLUSTER_DATABASE_TRANSACTION_IN_PROGRESS
	case ERROR_CLUSTER_DATABASE_TRANSACTION_IN_PROGRESS:
#endif /* !ERROR_CLUSTER_DATABASE_TRANSACTION_IN_PROGRESS */
#ifdef ERROR_CLUSTER_JOIN_IN_PROGRESS
	case ERROR_CLUSTER_JOIN_IN_PROGRESS:
#endif /* !ERROR_CLUSTER_JOIN_IN_PROGRESS */
#ifdef ERROR_CLUSTER_NODE_DRAIN_IN_PROGRESS
	case ERROR_CLUSTER_NODE_DRAIN_IN_PROGRESS:
#endif /* !ERROR_CLUSTER_NODE_DRAIN_IN_PROGRESS */
#ifdef ERROR_DS_DOMAIN_RENAME_IN_PROGRESS
	case ERROR_DS_DOMAIN_RENAME_IN_PROGRESS:
#endif /* !ERROR_DS_DOMAIN_RENAME_IN_PROGRESS */
#ifdef ERROR_DS_PDC_OPERATION_IN_PROGRESS
	case ERROR_DS_PDC_OPERATION_IN_PROGRESS:
#endif /* !ERROR_DS_PDC_OPERATION_IN_PROGRESS */
#ifdef ERROR_FTP_TRANSFER_IN_PROGRESS
	case ERROR_FTP_TRANSFER_IN_PROGRESS:
#endif /* !ERROR_FTP_TRANSFER_IN_PROGRESS */
#ifdef ERROR_GRAPHICS_OPM_SESSION_TYPE_CHANGE_IN_PROGRESS
	case ERROR_GRAPHICS_OPM_SESSION_TYPE_CHANGE_IN_PROGRESS:
#endif /* !ERROR_GRAPHICS_OPM_SESSION_TYPE_CHANGE_IN_PROGRESS */
#ifdef ERROR_GRAPHICS_SESSION_TYPE_CHANGE_IN_PROGRESS
	case ERROR_GRAPHICS_SESSION_TYPE_CHANGE_IN_PROGRESS:
#endif /* !ERROR_GRAPHICS_SESSION_TYPE_CHANGE_IN_PROGRESS */
#ifdef ERROR_JOURNAL_DELETE_IN_PROGRESS
	case ERROR_JOURNAL_DELETE_IN_PROGRESS:
#endif /* !ERROR_JOURNAL_DELETE_IN_PROGRESS */
#ifdef ERROR_LOG_ARCHIVE_IN_PROGRESS
	case ERROR_LOG_ARCHIVE_IN_PROGRESS:
#endif /* !ERROR_LOG_ARCHIVE_IN_PROGRESS */
#ifdef ERROR_LOG_FULL_HANDLER_IN_PROGRESS
	case ERROR_LOG_FULL_HANDLER_IN_PROGRESS:
#endif /* !ERROR_LOG_FULL_HANDLER_IN_PROGRESS */
#ifdef ERROR_NDIS_RESET_IN_PROGRESS
	case ERROR_NDIS_RESET_IN_PROGRESS:
#endif /* !ERROR_NDIS_RESET_IN_PROGRESS */
#ifdef ERROR_OPERATION_IN_PROGRESS
	case ERROR_OPERATION_IN_PROGRESS:
#endif /* !ERROR_OPERATION_IN_PROGRESS */
#ifdef ERROR_OPLOCK_BREAK_IN_PROGRESS
	case ERROR_OPLOCK_BREAK_IN_PROGRESS:
#endif /* !ERROR_OPLOCK_BREAK_IN_PROGRESS */
#ifdef ERROR_RUNLEVEL_SWITCH_IN_PROGRESS
	case ERROR_RUNLEVEL_SWITCH_IN_PROGRESS:
#endif /* !ERROR_RUNLEVEL_SWITCH_IN_PROGRESS */
#ifdef ERROR_SERVER_SHUTDOWN_IN_PROGRESS
	case ERROR_SERVER_SHUTDOWN_IN_PROGRESS:
#endif /* !ERROR_SERVER_SHUTDOWN_IN_PROGRESS */
#ifdef ERROR_SHUTDOWN_IN_PROGRESS
	case ERROR_SHUTDOWN_IN_PROGRESS:
#endif /* !ERROR_SHUTDOWN_IN_PROGRESS */
#ifdef ERROR_TIERING_VOLUME_DISMOUNT_IN_PROGRESS
	case ERROR_TIERING_VOLUME_DISMOUNT_IN_PROGRESS:
#endif /* !ERROR_TIERING_VOLUME_DISMOUNT_IN_PROGRESS */
#ifdef ERROR_TRANSACTION_FREEZE_IN_PROGRESS
	case ERROR_TRANSACTION_FREEZE_IN_PROGRESS:
#endif /* !ERROR_TRANSACTION_FREEZE_IN_PROGRESS */
#ifdef ERROR_VOLMGR_TRANSACTION_IN_PROGRESS
	case ERROR_VOLMGR_TRANSACTION_IN_PROGRESS:
#endif /* !ERROR_VOLMGR_TRANSACTION_IN_PROGRESS */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsExists(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_FILE_EXISTS
	case ERROR_FILE_EXISTS:
#endif /* !ERROR_FILE_EXISTS */
#ifdef ERROR_ALREADY_EXISTS
	case ERROR_ALREADY_EXISTS:
#endif /* !ERROR_ALREADY_EXISTS */
#ifdef ERROR_ALIAS_EXISTS
	case ERROR_ALIAS_EXISTS:
#endif /* !ERROR_ALIAS_EXISTS */
#ifdef ERROR_OBJECT_NAME_EXISTS
	case ERROR_OBJECT_NAME_EXISTS:
#endif /* !ERROR_OBJECT_NAME_EXISTS */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsNotDir(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_DIRECTORY
	case ERROR_DIRECTORY:
#endif /* !ERROR_DIRECTORY */
#ifdef ERROR_PATH_NOT_FOUND
	case ERROR_PATH_NOT_FOUND:
#endif /* !ERROR_PATH_NOT_FOUND */
#ifdef ERROR_INVALID_DRIVE
	case ERROR_INVALID_DRIVE:
#endif /* !ERROR_INVALID_DRIVE */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsNotEmpty(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_DIR_NOT_EMPTY
	case ERROR_DIR_NOT_EMPTY:
#endif /* !ERROR_DIR_NOT_EMPTY */
#ifdef ERROR_NOT_EMPTY
	case ERROR_NOT_EMPTY:
#endif /* !ERROR_NOT_EMPTY */
#ifdef ERROR_TXF_DIR_NOT_EMPTY
	case ERROR_TXF_DIR_NOT_EMPTY:
#endif /* !ERROR_TXF_DIR_NOT_EMPTY */
#ifdef ERROR_VOLMGR_DISK_NOT_EMPTY
	case ERROR_VOLMGR_DISK_NOT_EMPTY:
#endif /* !ERROR_VOLMGR_DISK_NOT_EMPTY */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsBadF(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_INVALID_HANDLE
	case ERROR_INVALID_HANDLE:
#endif /* !ERROR_INVALID_HANDLE */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsXDev(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_NOT_SAME_DEVICE
	case ERROR_NOT_SAME_DEVICE:
#endif /* !ERROR_NOT_SAME_DEVICE */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsUnsupportedError(DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_NOT_SUPPORTED
	case ERROR_NOT_SUPPORTED:
#endif /* !ERROR_NOT_SUPPORTED */
#ifdef ERROR_INVALID_FUNCTION
	case ERROR_INVALID_FUNCTION:
#endif /* !ERROR_INVALID_FUNCTION */
#ifdef ERROR_ATOMIC_LOCKS_NOT_SUPPORTED
	case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED:
#endif /* !ERROR_ATOMIC_LOCKS_NOT_SUPPORTED */
#ifdef ERROR_EAS_NOT_SUPPORTED
	case ERROR_EAS_NOT_SUPPORTED:
#endif /* !ERROR_EAS_NOT_SUPPORTED */
#ifdef ERROR_DEVICE_FEATURE_NOT_SUPPORTED
	case ERROR_DEVICE_FEATURE_NOT_SUPPORTED:
#endif /* !ERROR_DEVICE_FEATURE_NOT_SUPPORTED */
#ifdef ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED
	case ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED:
#endif /* !ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED */
#ifdef ERROR_RESIDENT_FILE_NOT_SUPPORTED
	case ERROR_RESIDENT_FILE_NOT_SUPPORTED:
#endif /* !ERROR_RESIDENT_FILE_NOT_SUPPORTED */
#ifdef ERROR_DIRECTORY_NOT_SUPPORTED
	case ERROR_DIRECTORY_NOT_SUPPORTED:
#endif /* !ERROR_DIRECTORY_NOT_SUPPORTED */
#ifdef ERROR_UNSUPPORTED_COMPRESSION
	case ERROR_UNSUPPORTED_COMPRESSION:
#endif /* !ERROR_UNSUPPORTED_COMPRESSION */
#ifdef ERROR_CARDBUS_NOT_SUPPORTED
	case ERROR_CARDBUS_NOT_SUPPORTED:
#endif /* !ERROR_CARDBUS_NOT_SUPPORTED */
#ifdef ERROR_NOT_SUPPORTED_ON_SBS
	case ERROR_NOT_SUPPORTED_ON_SBS:
#endif /* !ERROR_NOT_SUPPORTED_ON_SBS */
#ifdef ERROR_SYMLINK_NOT_SUPPORTED
	case ERROR_SYMLINK_NOT_SUPPORTED:
#endif /* !ERROR_SYMLINK_NOT_SUPPORTED */
#ifdef ERROR_INSTALL_LANGUAGE_UNSUPPORTED
	case ERROR_INSTALL_LANGUAGE_UNSUPPORTED:
#endif /* !ERROR_INSTALL_LANGUAGE_UNSUPPORTED */
#ifdef ERROR_UNSUPPORTED_TYPE
	case ERROR_UNSUPPORTED_TYPE:
#endif /* !ERROR_UNSUPPORTED_TYPE */
#ifdef ERROR_INSTALL_PLATFORM_UNSUPPORTED
	case ERROR_INSTALL_PLATFORM_UNSUPPORTED:
#endif /* !ERROR_INSTALL_PLATFORM_UNSUPPORTED */
#ifdef ERROR_PATCH_PACKAGE_UNSUPPORTED
	case ERROR_PATCH_PACKAGE_UNSUPPORTED:
#endif /* !ERROR_PATCH_PACKAGE_UNSUPPORTED */
#ifdef ERROR_PATCH_REMOVAL_UNSUPPORTED
	case ERROR_PATCH_REMOVAL_UNSUPPORTED:
#endif /* !ERROR_PATCH_REMOVAL_UNSUPPORTED */
#ifdef RPC_S_PROTSEQ_NOT_SUPPORTED
	case RPC_S_PROTSEQ_NOT_SUPPORTED:
#endif /* !RPC_S_PROTSEQ_NOT_SUPPORTED */
#ifdef RPC_S_UNSUPPORTED_TRANS_SYN
	case RPC_S_UNSUPPORTED_TRANS_SYN:
#endif /* !RPC_S_UNSUPPORTED_TRANS_SYN */
#ifdef RPC_S_UNSUPPORTED_TYPE
	case RPC_S_UNSUPPORTED_TYPE:
#endif /* !RPC_S_UNSUPPORTED_TYPE */
#ifdef RPC_S_UNSUPPORTED_NAME_SYNTAX
	case RPC_S_UNSUPPORTED_NAME_SYNTAX:
#endif /* !RPC_S_UNSUPPORTED_NAME_SYNTAX */
#ifdef RPC_S_CANNOT_SUPPORT
	case RPC_S_CANNOT_SUPPORT:
#endif /* !RPC_S_CANNOT_SUPPORT */
#ifdef RPC_S_UNSUPPORTED_AUTHN_LEVEL
	case RPC_S_UNSUPPORTED_AUTHN_LEVEL:
#endif /* !RPC_S_UNSUPPORTED_AUTHN_LEVEL */
#ifdef ERROR_METAFILE_NOT_SUPPORTED
	case ERROR_METAFILE_NOT_SUPPORTED:
#endif /* !ERROR_METAFILE_NOT_SUPPORTED */
#ifdef ERROR_TRANSFORM_NOT_SUPPORTED
	case ERROR_TRANSFORM_NOT_SUPPORTED:
#endif /* !ERROR_TRANSFORM_NOT_SUPPORTED */
#ifdef ERROR_CLIPPING_NOT_SUPPORTED
	case ERROR_CLIPPING_NOT_SUPPORTED:
#endif /* !ERROR_CLIPPING_NOT_SUPPORTED */
#ifdef PEERDIST_ERROR_CONTENTINFO_VERSION_UNSUPPORTED
	case PEERDIST_ERROR_CONTENTINFO_VERSION_UNSUPPORTED:
#endif /* !PEERDIST_ERROR_CONTENTINFO_VERSION_UNSUPPORTED */
#ifdef PEERDIST_ERROR_VERSION_UNSUPPORTED
	case PEERDIST_ERROR_VERSION_UNSUPPORTED:
#endif /* !PEERDIST_ERROR_VERSION_UNSUPPORTED */
#ifdef ERROR_NOT_SUPPORTED_IN_APPCONTAINER
	case ERROR_NOT_SUPPORTED_IN_APPCONTAINER:
#endif /* !ERROR_NOT_SUPPORTED_IN_APPCONTAINER */
#ifdef ERROR_NO_SUPPORTING_DRIVES
	case ERROR_NO_SUPPORTING_DRIVES:
#endif /* !ERROR_NO_SUPPORTING_DRIVES */
#ifdef ERROR_OFFLOAD_READ_FLT_NOT_SUPPORTED
	case ERROR_OFFLOAD_READ_FLT_NOT_SUPPORTED:
#endif /* !ERROR_OFFLOAD_READ_FLT_NOT_SUPPORTED */
#ifdef ERROR_OFFLOAD_WRITE_FLT_NOT_SUPPORTED
	case ERROR_OFFLOAD_WRITE_FLT_NOT_SUPPORTED:
#endif /* !ERROR_OFFLOAD_WRITE_FLT_NOT_SUPPORTED */
#ifdef ERROR_OFFLOAD_READ_FILE_NOT_SUPPORTED
	case ERROR_OFFLOAD_READ_FILE_NOT_SUPPORTED:
#endif /* !ERROR_OFFLOAD_READ_FILE_NOT_SUPPORTED */
#ifdef ERROR_OFFLOAD_WRITE_FILE_NOT_SUPPORTED
	case ERROR_OFFLOAD_WRITE_FILE_NOT_SUPPORTED:
#endif /* !ERROR_OFFLOAD_WRITE_FILE_NOT_SUPPORTED */
#ifdef ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED
	case ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED:
#endif /* !ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED */
#ifdef ERROR_CLUSTER_RESOURCE_CONTAINS_UNSUPPORTED_DIFF_AREA_FOR_SHARED_VOLUMES
	case ERROR_CLUSTER_RESOURCE_CONTAINS_UNSUPPORTED_DIFF_AREA_FOR_SHARED_VOLUMES:
#endif /* !ERROR_CLUSTER_RESOURCE_CONTAINS_UNSUPPORTED_DIFF_AREA_FOR_SHARED_VOLUMES */
#ifdef ERROR_VOLUME_NOT_SUPPORT_EFS
	case ERROR_VOLUME_NOT_SUPPORT_EFS:
#endif /* !ERROR_VOLUME_NOT_SUPPORT_EFS */
#ifdef ERROR_EFS_VERSION_NOT_SUPPORT
	case ERROR_EFS_VERSION_NOT_SUPPORT:
#endif /* !ERROR_EFS_VERSION_NOT_SUPPORT */
#ifdef ERROR_CS_ENCRYPTION_UNSUPPORTED_SERVER
	case ERROR_CS_ENCRYPTION_UNSUPPORTED_SERVER:
#endif /* !ERROR_CS_ENCRYPTION_UNSUPPORTED_SERVER */
#ifdef ERROR_IMPLICIT_TRANSACTION_NOT_SUPPORTED
	case ERROR_IMPLICIT_TRANSACTION_NOT_SUPPORTED:
#endif /* !ERROR_IMPLICIT_TRANSACTION_NOT_SUPPORTED */
#ifdef ERROR_TRANSACTIONS_UNSUPPORTED_REMOTE
	case ERROR_TRANSACTIONS_UNSUPPORTED_REMOTE:
#endif /* !ERROR_TRANSACTIONS_UNSUPPORTED_REMOTE */
#ifdef ERROR_TRANSACTED_MAPPING_UNSUPPORTED_REMOTE
	case ERROR_TRANSACTED_MAPPING_UNSUPPORTED_REMOTE:
#endif /* !ERROR_TRANSACTED_MAPPING_UNSUPPORTED_REMOTE */
#ifdef ERROR_OPERATION_NOT_SUPPORTED_IN_TRANSACTION
	case ERROR_OPERATION_NOT_SUPPORTED_IN_TRANSACTION:
#endif /* !ERROR_OPERATION_NOT_SUPPORTED_IN_TRANSACTION */
#ifdef ERROR_DS_AUTH_METHOD_NOT_SUPPORTED
	case ERROR_DS_AUTH_METHOD_NOT_SUPPORTED:
#endif /* !ERROR_DS_AUTH_METHOD_NOT_SUPPORTED */
#ifdef ERROR_DS_NOT_SUPPORTED
	case ERROR_DS_NOT_SUPPORTED:
#endif /* !ERROR_DS_NOT_SUPPORTED */
#ifdef ERROR_DS_DRA_NOT_SUPPORTED
	case ERROR_DS_DRA_NOT_SUPPORTED:
#endif /* !ERROR_DS_DRA_NOT_SUPPORTED */
#ifdef ERROR_DS_NOT_SUPPORTED_SORT_ORDER
	case ERROR_DS_NOT_SUPPORTED_SORT_ORDER:
#endif /* !ERROR_DS_NOT_SUPPORTED_SORT_ORDER */
#ifdef ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER
	case ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER:
#endif /* !ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER */
#ifdef DNS_ERROR_UNSUPPORTED_ALGORITHM
	case DNS_ERROR_UNSUPPORTED_ALGORITHM:
#endif /* !DNS_ERROR_UNSUPPORTED_ALGORITHM */
#ifdef DNS_ERROR_KSP_DOES_NOT_SUPPORT_PROTECTION
	case DNS_ERROR_KSP_DOES_NOT_SUPPORT_PROTECTION:
#endif /* !DNS_ERROR_KSP_DOES_NOT_SUPPORT_PROTECTION */
#ifdef WSAENOPROTOOPT
	case WSAENOPROTOOPT:
#endif /* !WSAENOPROTOOPT */
#ifdef WSAEPROTONOSUPPORT
	case WSAEPROTONOSUPPORT:
#endif /* !WSAEPROTONOSUPPORT */
#ifdef WSAESOCKTNOSUPPORT
	case WSAESOCKTNOSUPPORT:
#endif /* !WSAESOCKTNOSUPPORT */
#ifdef WSAEOPNOTSUPP
	case WSAEOPNOTSUPP:
#endif /* !WSAEOPNOTSUPP */
#ifdef WSAEPFNOSUPPORT
	case WSAEPFNOSUPPORT:
#endif /* !WSAEPFNOSUPPORT */
#ifdef WSAEAFNOSUPPORT
	case WSAEAFNOSUPPORT:
#endif /* !WSAEAFNOSUPPORT */
#ifdef WSAVERNOTSUPPORTED
	case WSAVERNOTSUPPORTED:
#endif /* !WSAVERNOTSUPPORTED */
#ifdef ERROR_IPSEC_IKE_UNSUPPORTED_ID
	case ERROR_IPSEC_IKE_UNSUPPORTED_ID:
#endif /* !ERROR_IPSEC_IKE_UNSUPPORTED_ID */
#ifdef ERROR_IPSEC_IKE_PEER_DOESNT_SUPPORT_MOBIKE
	case ERROR_IPSEC_IKE_PEER_DOESNT_SUPPORT_MOBIKE:
#endif /* !ERROR_IPSEC_IKE_PEER_DOESNT_SUPPORT_MOBIKE */
#ifdef ERROR_EVT_FILTER_UNSUPPORTEDOP
	case ERROR_EVT_FILTER_UNSUPPORTEDOP:
#endif /* !ERROR_EVT_FILTER_UNSUPPORTEDOP */
#ifdef ERROR_MRM_UNSUPPORTED_DIRECTORY_TYPE
	case ERROR_MRM_UNSUPPORTED_DIRECTORY_TYPE:
#endif /* !ERROR_MRM_UNSUPPORTED_DIRECTORY_TYPE */
#ifdef ERROR_MRM_UNSUPPORTED_PROFILE_TYPE
	case ERROR_MRM_UNSUPPORTED_PROFILE_TYPE:
#endif /* !ERROR_MRM_UNSUPPORTED_PROFILE_TYPE */
#ifdef ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE
	case ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE:
#endif /* !ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_MERGE */
#ifdef ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE
	case ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE:
#endif /* !ERROR_MRM_UNSUPPORTED_FILE_TYPE_FOR_LOAD_UNLOAD_PRI_FILE */
#ifdef ERROR_MCA_UNSUPPORTED_MCCS_VERSION
	case ERROR_MCA_UNSUPPORTED_MCCS_VERSION:
#endif /* !ERROR_MCA_UNSUPPORTED_MCCS_VERSION */
#ifdef ERROR_MCA_UNSUPPORTED_COLOR_TEMPERATURE
	case ERROR_MCA_UNSUPPORTED_COLOR_TEMPERATURE:
#endif /* !ERROR_MCA_UNSUPPORTED_COLOR_TEMPERATURE */
#ifdef ERROR_HASH_NOT_SUPPORTED
	case ERROR_HASH_NOT_SUPPORTED:
#endif /* !ERROR_HASH_NOT_SUPPORTED */
#ifdef ERROR_GPIO_VERSION_NOT_SUPPORTED
	case ERROR_GPIO_VERSION_NOT_SUPPORTED:
#endif /* !ERROR_GPIO_VERSION_NOT_SUPPORTED */
#ifdef RO_E_UNSUPPORTED_FROM_MTA
	case RO_E_UNSUPPORTED_FROM_MTA:
#endif /* !RO_E_UNSUPPORTED_FROM_MTA */
#ifdef CO_E_NOT_SUPPORTED
	case CO_E_NOT_SUPPORTED:
#endif /* !CO_E_NOT_SUPPORTED */
#ifdef OLE_E_ADVISENOTSUPPORTED
	case OLE_E_ADVISENOTSUPPORTED:
#endif /* !OLE_E_ADVISENOTSUPPORTED */
#ifdef CACHE_S_FORMATETC_NOTSUPPORTED
	case CACHE_S_FORMATETC_NOTSUPPORTED:
#endif /* !CACHE_S_FORMATETC_NOTSUPPORTED */
#ifdef SCHED_E_UNSUPPORTED_ACCOUNT_OPTION
	case SCHED_E_UNSUPPORTED_ACCOUNT_OPTION:
#endif /* !SCHED_E_UNSUPPORTED_ACCOUNT_OPTION */
#ifdef NTE_NOT_SUPPORTED
	case NTE_NOT_SUPPORTED:
#endif /* !NTE_NOT_SUPPORTED */
#ifdef NTE_HMAC_NOT_SUPPORTED
	case NTE_HMAC_NOT_SUPPORTED:
#endif /* !NTE_HMAC_NOT_SUPPORTED */
#ifdef SEC_E_UNSUPPORTED_FUNCTION
	case SEC_E_UNSUPPORTED_FUNCTION:
#endif /* !SEC_E_UNSUPPORTED_FUNCTION */
#ifdef SEC_E_QOP_NOT_SUPPORTED
	case SEC_E_QOP_NOT_SUPPORTED:
#endif /* !SEC_E_QOP_NOT_SUPPORTED */
#ifdef SEC_E_STRONG_CRYPTO_NOT_SUPPORTED
	case SEC_E_STRONG_CRYPTO_NOT_SUPPORTED:
#endif /* !SEC_E_STRONG_CRYPTO_NOT_SUPPORTED */
#ifdef SEC_E_UNSUPPORTED_PREAUTH
	case SEC_E_UNSUPPORTED_PREAUTH:
#endif /* !SEC_E_UNSUPPORTED_PREAUTH */
#ifdef SEC_E_NO_S4U_PROT_SUPPORT
	case SEC_E_NO_S4U_PROT_SUPPORT:
#endif /* !SEC_E_NO_S4U_PROT_SUPPORT */
#ifdef OSS_INDEFINITE_NOT_SUPPORTED
	case OSS_INDEFINITE_NOT_SUPPORTED:
#endif /* !OSS_INDEFINITE_NOT_SUPPORTED */
#ifdef CERTSRV_E_KEY_ATTESTATION_NOT_SUPPORTED
	case CERTSRV_E_KEY_ATTESTATION_NOT_SUPPORTED:
#endif /* !CERTSRV_E_KEY_ATTESTATION_NOT_SUPPORTED */
#ifdef CERTSRV_E_UNSUPPORTED_CERT_TYPE
	case CERTSRV_E_UNSUPPORTED_CERT_TYPE:
#endif /* !CERTSRV_E_UNSUPPORTED_CERT_TYPE */
#ifdef SPAPI_E_REMOTE_REQUEST_UNSUPPORTED
	case SPAPI_E_REMOTE_REQUEST_UNSUPPORTED:
#endif /* !SPAPI_E_REMOTE_REQUEST_UNSUPPORTED */
#ifdef SCARD_E_READER_UNSUPPORTED
	case SCARD_E_READER_UNSUPPORTED:
#endif /* !SCARD_E_READER_UNSUPPORTED */
#ifdef SCARD_E_CARD_UNSUPPORTED
	case SCARD_E_CARD_UNSUPPORTED:
#endif /* !SCARD_E_CARD_UNSUPPORTED */
#ifdef SCARD_E_UNSUPPORTED_FEATURE
	case SCARD_E_UNSUPPORTED_FEATURE:
#endif /* !SCARD_E_UNSUPPORTED_FEATURE */
#ifdef SCARD_W_UNSUPPORTED_CARD
	case SCARD_W_UNSUPPORTED_CARD:
#endif /* !SCARD_W_UNSUPPORTED_CARD */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsIntr(/*DWORD*/ DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_OPERATION_ABORTED
	case ERROR_OPERATION_ABORTED:
#endif /* !ERROR_OPERATION_ABORTED */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsBufferTooSmall(/*DWORD*/ DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_BUFFER_OVERFLOW
	case ERROR_BUFFER_OVERFLOW:
#endif /* ERROR_BUFFER_OVERFLOW */
#ifdef ERROR_INSUFFICIENT_BUFFER
	case ERROR_INSUFFICIENT_BUFFER:
#endif /* ERROR_INSUFFICIENT_BUFFER */
#ifdef ERROR_MORE_DATA
	case ERROR_MORE_DATA:
#endif /* ERROR_MORE_DATA */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsInvalidArgument(/*DWORD*/ DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_BAD_USERNAME
	case ERROR_BAD_USERNAME:
#endif /* ERROR_BAD_USERNAME */
#ifdef ERROR_EA_LIST_INCONSISTENT
	case ERROR_EA_LIST_INCONSISTENT:
#endif /* ERROR_EA_LIST_INCONSISTENT */
#ifdef ERROR_INVALID_ADDRESS
	case ERROR_INVALID_ADDRESS:
#endif /* ERROR_INVALID_ADDRESS */
#ifdef ERROR_INVALID_EA_NAME
	case ERROR_INVALID_EA_NAME:
#endif /* ERROR_INVALID_EA_NAME */
#ifdef ERROR_INVALID_SIGNAL_NUMBER
	case ERROR_INVALID_SIGNAL_NUMBER:
#endif /* ERROR_INVALID_SIGNAL_NUMBER */
#ifdef ERROR_META_EXPANSION_TOO_LONG
	case ERROR_META_EXPANSION_TOO_LONG:
#endif /* ERROR_META_EXPANSION_TOO_LONG */
#ifdef ERROR_NONE_MAPPED
	case ERROR_NONE_MAPPED:
#endif /* ERROR_NONE_MAPPED */
#ifdef ERROR_NO_TOKEN
	case ERROR_NO_TOKEN:
#endif /* ERROR_NO_TOKEN */
#ifdef ERROR_SECTOR_NOT_FOUND
	case ERROR_SECTOR_NOT_FOUND:
#endif /* ERROR_SECTOR_NOT_FOUND */
#ifdef ERROR_SEEK
	case ERROR_SEEK:
#endif /* ERROR_SEEK */
#ifdef ERROR_THREAD_1_INACTIVE
	case ERROR_THREAD_1_INACTIVE:
#endif /* ERROR_THREAD_1_INACTIVE */
#ifdef ERROR_INVALID_ACCESS
	case ERROR_INVALID_ACCESS:
#endif /* ERROR_INVALID_ACCESS */
#ifdef ERROR_INVALID_DATA
	case ERROR_INVALID_DATA:
#endif /* ERROR_INVALID_DATA */
#ifdef ERROR_INVALID_PARAMETER
	case ERROR_INVALID_PARAMETER:
#endif /* ERROR_INVALID_PARAMETER */
#ifdef ERROR_NEGATIVE_SEEK
	case ERROR_NEGATIVE_SEEK:
#endif /* ERROR_NEGATIVE_SEEK */
#ifdef ERROR_INVALID_FUNCTION
	case ERROR_INVALID_FUNCTION:
#endif /* ERROR_INVALID_FUNCTION */
		return true;

	default: break;
	}
	return false;
}

PUBLIC ATTR_CONST WUNUSED bool DCALL
DeeNTSystem_IsNoLink(/*DWORD*/ DeeNT_DWORD dwError) {
	switch (dwError) {

#ifdef ERROR_NOT_A_REPARSE_POINT
	case ERROR_NOT_A_REPARSE_POINT:
#endif /* ERROR_NOT_A_REPARSE_POINT */
		return true;

	default: break;
	}
	return false;
}



/* Figure out how to implement `DeeNTSystem_TranslateErrno()' */
#undef DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS
#undef DeeNTSystem_TranslateErrno_USE_DOSMAPERR
#undef DeeNTSystem_TranslateErrno_USE_FALLBACK
#if defined(CONFIG_HAVE_errno_nt2kos)
#define DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS 1
#elif defined(CONFIG_HAVE__dosmaperr) && defined(CONFIG_HAVE_errno) && 0
#define DeeNTSystem_TranslateErrno_USE_DOSMAPERR 1
#else
#define DeeNTSystem_TranslateErrno_USE_FALLBACK 1
#endif


/* Figure out how to implement `DeeNTSystem_TranslateNtError()' */
#undef DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT
#undef DeeNTSystem_TranslateNtError_USE_FALLBACK
#if defined(CONFIG_HAVE_errno_kos2nt)
#define DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT 1
#else
#define DeeNTSystem_TranslateNtError_USE_FALLBACK 1
#endif


#ifdef DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS
#ifndef __errno_nt2kos_defined
#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */
extern ATTR_DLLIMPORT /*errno_t*/ int __LIBCCALL errno_nt2kos(DeeNT_DWORD dwError);
#endif /* !__errno_nt2kos_defined */
#endif /* DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS */

#ifdef DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT
#ifndef __errno_kos2nt_defined
#ifndef __LIBCCALL
#define __LIBCCALL /* nothing */
#endif /* !__LIBCCALL */
extern ATTR_DLLIMPORT DeeNT_DWORD __LIBCCALL errno_kos2nt(/*errno_t*/ int errno_value);
#endif /* !__errno_kos2nt_defined */
#endif /* DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT */

#ifdef DeeNTSystem_TranslateErrno_USE_DOSMAPERR
#ifdef CONFIG_HAVE__doserrno
#define IF_CONFIG_HAVE__doserrno(x) x
#else /* CONFIG_HAVE__doserrno */
#define IF_CONFIG_HAVE__doserrno(x) /* nothing */
#endif /* !CONFIG_HAVE__doserrno */
extern ATTR_DLLIMPORT void ATTR_CDECL _dosmaperr(DeeNT_DWORD dwError);
#endif /* DeeNTSystem_TranslateErrno_USE_DOSMAPERR */

#if (defined(DeeNTSystem_TranslateErrno_USE_FALLBACK) || \
     defined(DeeNTSystem_TranslateNtError_USE_FALLBACK))
struct nt2errno_ent {
	DeeNT_DWORD     etval; /* NT error code */
	/*errno_t*/ int erval; /* errno code */
};

#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
#define __CYGWIN__ __CYGWIN32__
#endif /* __CYGWIN32__ && !__CYGWIN__ */

#ifdef _MSC_VER
#define NT2ERRNO_PREFER_MSVC
#elif defined(__CYGWIN__)
#undef NT2ERRNO_PREFER_MSVC
#elif 1 /* Fallback */
#undef NT2ERRNO_PREFER_MSVC
#else
#define NT2ERRNO_PREFER_MSVC
#endif
#define NT2ERRNO_SRC_CYGWIN /* Include cygwin-specific translations */
#define NT2ERRNO_SRC_MSVC   /* Include msvc-specific translations */

PRIVATE struct nt2errno_ent const nt2errno[] = {

	/* NT error codes that only appear in CYGWIN's translation table. */
#ifdef NT2ERRNO_SRC_CYGWIN
#if defined(ERROR_ACTIVE_CONNECTIONS) && defined(EAGAIN)
	{ ERROR_ACTIVE_CONNECTIONS, EAGAIN },
#endif /* ERROR_ACTIVE_CONNECTIONS && EAGAIN */
#if defined(ERROR_BAD_DEVICE) && defined(ENODEV)
	{ ERROR_BAD_DEVICE, ENODEV },
#endif /* ERROR_BAD_DEVICE && ENODEV */
#if defined(ERROR_BAD_EXE_FORMAT) && defined(ENOEXEC)
	{ ERROR_BAD_EXE_FORMAT, ENOEXEC },
#endif /* ERROR_BAD_EXE_FORMAT && ENOEXEC */
#if defined(ERROR_BAD_NET_RESP) && defined(ENOSYS)
	{ ERROR_BAD_NET_RESP, ENOSYS },
#endif /* ERROR_BAD_NET_RESP && ENOSYS */
#if defined(ERROR_BAD_PIPE) && defined(EINVAL)
	{ ERROR_BAD_PIPE, EINVAL },
#endif /* ERROR_BAD_PIPE && EINVAL */
#if defined(ERROR_BAD_UNIT) && defined(ENODEV)
	{ ERROR_BAD_UNIT, ENODEV },
#endif /* ERROR_BAD_UNIT && ENODEV */
#if defined(ERROR_BAD_USERNAME) && defined(EINVAL)
	{ ERROR_BAD_USERNAME, EINVAL },
#endif /* ERROR_BAD_USERNAME && EINVAL */
#if defined(ERROR_BEGINNING_OF_MEDIA) && defined(EIO)
	{ ERROR_BEGINNING_OF_MEDIA, EIO },
#endif /* ERROR_BEGINNING_OF_MEDIA && EIO */
#if defined(ERROR_BUSY) && defined(EBUSY)
	{ ERROR_BUSY, EBUSY },
#endif /* ERROR_BUSY && EBUSY */
#if defined(ERROR_BUS_RESET) && defined(EIO)
	{ ERROR_BUS_RESET, EIO },
#endif /* ERROR_BUS_RESET && EIO */
#if defined(ERROR_CALL_NOT_IMPLEMENTED) && defined(ENOSYS)
	{ ERROR_CALL_NOT_IMPLEMENTED, ENOSYS },
#endif /* ERROR_CALL_NOT_IMPLEMENTED && ENOSYS */
#if defined(ERROR_CANCELLED) && defined(EINTR)
	{ ERROR_CANCELLED, EINTR },
#endif /* ERROR_CANCELLED && EINTR */
#if defined(ERROR_COMMITMENT_LIMIT) && defined(EAGAIN)
	{ ERROR_COMMITMENT_LIMIT, EAGAIN },
#endif /* ERROR_COMMITMENT_LIMIT && EAGAIN */
#if defined(ERROR_CONNECTION_REFUSED) && defined(ECONNREFUSED)
	{ ERROR_CONNECTION_REFUSED, ECONNREFUSED },
#endif /* ERROR_CONNECTION_REFUSED && ECONNREFUSED */
#if defined(ERROR_CRC) && defined(EIO)
	{ ERROR_CRC, EIO },
#endif /* ERROR_CRC && EIO */
#if defined(ERROR_DEVICE_DOOR_OPEN) && defined(EIO)
	{ ERROR_DEVICE_DOOR_OPEN, EIO },
#endif /* ERROR_DEVICE_DOOR_OPEN && EIO */
#if defined(ERROR_DEVICE_IN_USE) && defined(EAGAIN)
	{ ERROR_DEVICE_IN_USE, EAGAIN },
#endif /* ERROR_DEVICE_IN_USE && EAGAIN */
#if defined(ERROR_DEVICE_REQUIRES_CLEANING) && defined(EIO)
	{ ERROR_DEVICE_REQUIRES_CLEANING, EIO },
#endif /* ERROR_DEVICE_REQUIRES_CLEANING && EIO */
#if defined(ERROR_DEV_NOT_EXIST) && defined(ENOENT)
	{ ERROR_DEV_NOT_EXIST, ENOENT },
#endif /* ERROR_DEV_NOT_EXIST && ENOENT */
#if defined(ERROR_DIRECTORY) && defined(ENOTDIR)
	{ ERROR_DIRECTORY, ENOTDIR },
#endif /* ERROR_DIRECTORY && ENOTDIR */
#if defined(ERROR_DISK_CORRUPT) && defined(EIO)
	{ ERROR_DISK_CORRUPT, EIO },
#endif /* ERROR_DISK_CORRUPT && EIO */
#if defined(ERROR_DS_GENERIC_ERROR) && defined(EIO)
	{ ERROR_DS_GENERIC_ERROR, EIO },
#endif /* ERROR_DS_GENERIC_ERROR && EIO */
#if defined(ERROR_DUP_NAME) && defined(ENOTUNIQ)
	{ ERROR_DUP_NAME, ENOTUNIQ },
#endif /* ERROR_DUP_NAME && ENOTUNIQ */
#if defined(ERROR_EAS_DIDNT_FIT) && defined(ENOSPC)
	{ ERROR_EAS_DIDNT_FIT, ENOSPC },
#endif /* ERROR_EAS_DIDNT_FIT && ENOSPC */
#if defined(ERROR_EAS_NOT_SUPPORTED) && defined(ENOTSUP)
	{ ERROR_EAS_NOT_SUPPORTED, ENOTSUP },
#endif /* ERROR_EAS_NOT_SUPPORTED && ENOTSUP */
#if defined(ERROR_EA_LIST_INCONSISTENT) && defined(EINVAL)
	{ ERROR_EA_LIST_INCONSISTENT, EINVAL },
#endif /* ERROR_EA_LIST_INCONSISTENT && EINVAL */
#if defined(ERROR_EA_TABLE_FULL) && defined(ENOSPC)
	{ ERROR_EA_TABLE_FULL, ENOSPC },
#endif /* ERROR_EA_TABLE_FULL && ENOSPC */
#if defined(ERROR_END_OF_MEDIA) && defined(ENOSPC)
	{ ERROR_END_OF_MEDIA, ENOSPC },
#endif /* ERROR_END_OF_MEDIA && ENOSPC */
#if defined(ERROR_EOM_OVERFLOW) && defined(EIO)
	{ ERROR_EOM_OVERFLOW, EIO },
#endif /* ERROR_EOM_OVERFLOW && EIO */
#if defined(ERROR_EXE_MACHINE_TYPE_MISMATCH) && defined(ENOEXEC)
	{ ERROR_EXE_MACHINE_TYPE_MISMATCH, ENOEXEC },
#endif /* ERROR_EXE_MACHINE_TYPE_MISMATCH && ENOEXEC */
#if defined(ERROR_EXE_MARKED_INVALID) && defined(ENOEXEC)
	{ ERROR_EXE_MARKED_INVALID, ENOEXEC },
#endif /* ERROR_EXE_MARKED_INVALID && ENOEXEC */
#if defined(ERROR_FILEMARK_DETECTED) && defined(EIO)
	{ ERROR_FILEMARK_DETECTED, EIO },
#endif /* ERROR_FILEMARK_DETECTED && EIO */
#if defined(ERROR_FILE_CORRUPT) && defined(EEXIST)
	{ ERROR_FILE_CORRUPT, EEXIST },
#endif /* ERROR_FILE_CORRUPT && EEXIST */
#if defined(ERROR_FILE_INVALID) && defined(ENXIO)
	{ ERROR_FILE_INVALID, ENXIO },
#endif /* ERROR_FILE_INVALID && ENXIO */
#if defined(ERROR_HANDLE_DISK_FULL) && defined(ENOSPC)
	{ ERROR_HANDLE_DISK_FULL, ENOSPC },
#endif /* ERROR_HANDLE_DISK_FULL && ENOSPC */
#if defined(ERROR_HANDLE_EOF) && defined(ENODATA)
	{ ERROR_HANDLE_EOF, ENODATA },
#endif /* ERROR_HANDLE_EOF && ENODATA */
#if defined(ERROR_INVALID_ADDRESS) && defined(EINVAL)
	{ ERROR_INVALID_ADDRESS, EINVAL },
#endif /* ERROR_INVALID_ADDRESS && EINVAL */
#if defined(ERROR_INVALID_AT_INTERRUPT_TIME) && defined(EINTR)
	{ ERROR_INVALID_AT_INTERRUPT_TIME, EINTR },
#endif /* ERROR_INVALID_AT_INTERRUPT_TIME && EINTR */
#if defined(ERROR_INVALID_BLOCK_LENGTH) && defined(EIO)
	{ ERROR_INVALID_BLOCK_LENGTH, EIO },
#endif /* ERROR_INVALID_BLOCK_LENGTH && EIO */
#if defined(ERROR_INVALID_EA_NAME) && defined(EINVAL)
	{ ERROR_INVALID_EA_NAME, EINVAL },
#endif /* ERROR_INVALID_EA_NAME && EINVAL */
#if defined(ERROR_INVALID_EXE_SIGNATURE) && defined(ENOEXEC)
	{ ERROR_INVALID_EXE_SIGNATURE, ENOEXEC },
#endif /* ERROR_INVALID_EXE_SIGNATURE && ENOEXEC */
#if defined(ERROR_INVALID_NAME) && defined(ENOENT)
	{ ERROR_INVALID_NAME, ENOENT },
#endif /* ERROR_INVALID_NAME && ENOENT */
#if defined(ERROR_INVALID_SIGNAL_NUMBER) && defined(EINVAL)
	{ ERROR_INVALID_SIGNAL_NUMBER, EINVAL },
#endif /* ERROR_INVALID_SIGNAL_NUMBER && EINVAL */
#if defined(ERROR_IOPL_NOT_ENABLED) && defined(ENOEXEC)
	{ ERROR_IOPL_NOT_ENABLED, ENOEXEC },
#endif /* ERROR_IOPL_NOT_ENABLED && ENOEXEC */
#if defined(ERROR_IO_DEVICE) && defined(EIO)
	{ ERROR_IO_DEVICE, EIO },
#endif /* ERROR_IO_DEVICE && EIO */
#if defined(ERROR_IO_INCOMPLETE) && defined(EAGAIN)
	{ ERROR_IO_INCOMPLETE, EAGAIN },
#endif /* ERROR_IO_INCOMPLETE && EAGAIN */
#if defined(ERROR_IO_PENDING) && defined(EAGAIN)
	{ ERROR_IO_PENDING, EAGAIN },
#endif /* ERROR_IO_PENDING && EAGAIN */
#if defined(ERROR_META_EXPANSION_TOO_LONG) && defined(EINVAL)
	{ ERROR_META_EXPANSION_TOO_LONG, EINVAL },
#endif /* ERROR_META_EXPANSION_TOO_LONG && EINVAL */
#if defined(ERROR_MOD_NOT_FOUND) && defined(ENOENT)
	{ ERROR_MOD_NOT_FOUND, ENOENT },
#endif /* ERROR_MOD_NOT_FOUND && ENOENT */
#if defined(ERROR_MORE_DATA) && defined(EMSGSIZE)
	{ ERROR_MORE_DATA, EMSGSIZE },
#endif /* ERROR_MORE_DATA && EMSGSIZE */
#if defined(ERROR_NETNAME_DELETED) && defined(ENOENT)
	{ ERROR_NETNAME_DELETED, ENOENT },
#endif /* ERROR_NETNAME_DELETED && ENOENT */
#if defined(ERROR_NOACCESS) && defined(EFAULT)
	{ ERROR_NOACCESS, EFAULT },
#endif /* ERROR_NOACCESS && EFAULT */
#if defined(ERROR_NONE_MAPPED) && defined(EINVAL)
	{ ERROR_NONE_MAPPED, EINVAL },
#endif /* ERROR_NONE_MAPPED && EINVAL */
#if defined(ERROR_NONPAGED_SYSTEM_RESOURCES) && defined(EAGAIN)
	{ ERROR_NONPAGED_SYSTEM_RESOURCES, EAGAIN },
#endif /* ERROR_NONPAGED_SYSTEM_RESOURCES && EAGAIN */
#if defined(ERROR_NOT_CONNECTED) && defined(ENOLINK)
	{ ERROR_NOT_CONNECTED, ENOLINK },
#endif /* ERROR_NOT_CONNECTED && ENOLINK */
#if defined(ERROR_NOT_OWNER) && defined(EPERM)
	{ ERROR_NOT_OWNER, EPERM },
#endif /* ERROR_NOT_OWNER && EPERM */
#if defined(ERROR_NOT_READY) && defined(ENOMEDIUM)
	{ ERROR_NOT_READY, ENOMEDIUM },
#endif /* ERROR_NOT_READY && ENOMEDIUM */
#if defined(ERROR_NOT_SUPPORTED) && defined(ENOSYS)
	{ ERROR_NOT_SUPPORTED, ENOSYS },
#endif /* ERROR_NOT_SUPPORTED && ENOSYS */
#if defined(ERROR_NO_DATA) && defined(EPIPE)
	{ ERROR_NO_DATA, EPIPE },
#endif /* ERROR_NO_DATA && EPIPE */
#if defined(ERROR_NO_DATA_DETECTED) && defined(EIO)
	{ ERROR_NO_DATA_DETECTED, EIO },
#endif /* ERROR_NO_DATA_DETECTED && EIO */
#if defined(ERROR_NO_MEDIA_IN_DRIVE) && defined(ENOMEDIUM)
	{ ERROR_NO_MEDIA_IN_DRIVE, ENOMEDIUM },
#endif /* ERROR_NO_MEDIA_IN_DRIVE && ENOMEDIUM */
#if defined(ERROR_NO_MORE_ITEMS) && defined(ENMFILE)
	{ ERROR_NO_MORE_ITEMS, ENMFILE },
#endif /* ERROR_NO_MORE_ITEMS && ENMFILE */
#if defined(ERROR_NO_MORE_SEARCH_HANDLES) && defined(ENFILE)
	{ ERROR_NO_MORE_SEARCH_HANDLES, ENFILE },
#endif /* ERROR_NO_MORE_SEARCH_HANDLES && ENFILE */
#if defined(ERROR_NO_SIGNAL_SENT) && defined(EIO)
	{ ERROR_NO_SIGNAL_SENT, EIO },
#endif /* ERROR_NO_SIGNAL_SENT && EIO */
#if defined(ERROR_NO_SYSTEM_RESOURCES) && defined(EFBIG)
	{ ERROR_NO_SYSTEM_RESOURCES, EFBIG },
#endif /* ERROR_NO_SYSTEM_RESOURCES && EFBIG */
#if defined(ERROR_NO_TOKEN) && defined(EINVAL)
	{ ERROR_NO_TOKEN, EINVAL },
#endif /* ERROR_NO_TOKEN && EINVAL */
#if defined(ERROR_OPEN_FAILED) && defined(EIO)
	{ ERROR_OPEN_FAILED, EIO },
#endif /* ERROR_OPEN_FAILED && EIO */
#if defined(ERROR_OPEN_FILES) && defined(EAGAIN)
	{ ERROR_OPEN_FILES, EAGAIN },
#endif /* ERROR_OPEN_FILES && EAGAIN */
#if defined(ERROR_OUTOFMEMORY) && defined(ENOMEM)
	{ ERROR_OUTOFMEMORY, ENOMEM },
#endif /* ERROR_OUTOFMEMORY && ENOMEM */
#if defined(ERROR_PAGED_SYSTEM_RESOURCES) && defined(EAGAIN)
	{ ERROR_PAGED_SYSTEM_RESOURCES, EAGAIN },
#endif /* ERROR_PAGED_SYSTEM_RESOURCES && EAGAIN */
#if defined(ERROR_PAGEFILE_QUOTA) && defined(EAGAIN)
	{ ERROR_PAGEFILE_QUOTA, EAGAIN },
#endif /* ERROR_PAGEFILE_QUOTA && EAGAIN */
#if defined(ERROR_PIPE_BUSY) && defined(EBUSY)
	{ ERROR_PIPE_BUSY, EBUSY },
#endif /* ERROR_PIPE_BUSY && EBUSY */
#if defined(ERROR_PIPE_CONNECTED) && defined(EBUSY)
	{ ERROR_PIPE_CONNECTED, EBUSY },
#endif /* ERROR_PIPE_CONNECTED && EBUSY */
#if defined(ERROR_PIPE_LISTENING) && defined(ECOMM)
	{ ERROR_PIPE_LISTENING, ECOMM },
#endif /* ERROR_PIPE_LISTENING && ECOMM */
#if defined(ERROR_PIPE_NOT_CONNECTED) && defined(ECOMM)
	{ ERROR_PIPE_NOT_CONNECTED, ECOMM },
#endif /* ERROR_PIPE_NOT_CONNECTED && ECOMM */
#if defined(ERROR_POSSIBLE_DEADLOCK) && defined(EDEADLOCK)
	{ ERROR_POSSIBLE_DEADLOCK, EDEADLOCK },
#endif /* ERROR_POSSIBLE_DEADLOCK && EDEADLOCK */
#if defined(ERROR_PRIVILEGE_NOT_HELD) && defined(EPERM)
	{ ERROR_PRIVILEGE_NOT_HELD, EPERM },
#endif /* ERROR_PRIVILEGE_NOT_HELD && EPERM */
#if defined(ERROR_PROCESS_ABORTED) && defined(EFAULT)
	{ ERROR_PROCESS_ABORTED, EFAULT },
#endif /* ERROR_PROCESS_ABORTED && EFAULT */
#if defined(ERROR_PROC_NOT_FOUND) && defined(ESRCH)
	{ ERROR_PROC_NOT_FOUND, ESRCH },
#endif /* ERROR_PROC_NOT_FOUND && ESRCH */
#if defined(ERROR_REM_NOT_LIST) && defined(ENONET)
	{ ERROR_REM_NOT_LIST, ENONET },
#endif /* ERROR_REM_NOT_LIST && ENONET */
#if defined(ERROR_SECTOR_NOT_FOUND) && defined(EINVAL)
	{ ERROR_SECTOR_NOT_FOUND, EINVAL },
#endif /* ERROR_SECTOR_NOT_FOUND && EINVAL */
#if defined(ERROR_SEEK) && defined(EINVAL)
	{ ERROR_SEEK, EINVAL },
#endif /* ERROR_SEEK && EINVAL */
#if defined(ERROR_SERVICE_REQUEST_TIMEOUT) && defined(EBUSY)
	{ ERROR_SERVICE_REQUEST_TIMEOUT, EBUSY },
#endif /* ERROR_SERVICE_REQUEST_TIMEOUT && EBUSY */
#if defined(ERROR_SETMARK_DETECTED) && defined(EIO)
	{ ERROR_SETMARK_DETECTED, EIO },
#endif /* ERROR_SETMARK_DETECTED && EIO */
#if defined(ERROR_SHARING_BUFFER_EXCEEDED) && defined(ENOLCK)
	{ ERROR_SHARING_BUFFER_EXCEEDED, ENOLCK },
#endif /* ERROR_SHARING_BUFFER_EXCEEDED && ENOLCK */
#if defined(ERROR_SHARING_VIOLATION) && defined(EBUSY)
	{ ERROR_SHARING_VIOLATION, EBUSY },
#endif /* ERROR_SHARING_VIOLATION && EBUSY */
#if defined(ERROR_SIGNAL_PENDING) && defined(EBUSY)
	{ ERROR_SIGNAL_PENDING, EBUSY },
#endif /* ERROR_SIGNAL_PENDING && EBUSY */
#if defined(ERROR_SIGNAL_REFUSED) && defined(EIO)
	{ ERROR_SIGNAL_REFUSED, EIO },
#endif /* ERROR_SIGNAL_REFUSED && EIO */
#if defined(ERROR_SXS_CANT_GEN_ACTCTX) && defined(ELIBBAD)
	{ ERROR_SXS_CANT_GEN_ACTCTX, ELIBBAD },
#endif /* ERROR_SXS_CANT_GEN_ACTCTX && ELIBBAD */
#if defined(ERROR_THREAD_1_INACTIVE) && defined(EINVAL)
	{ ERROR_THREAD_1_INACTIVE, EINVAL },
#endif /* ERROR_THREAD_1_INACTIVE && EINVAL */
#if defined(ERROR_TIMEOUT) && defined(EBUSY)
	{ ERROR_TIMEOUT, EBUSY },
#endif /* ERROR_TIMEOUT && EBUSY */
#if defined(ERROR_TOO_MANY_LINKS) && defined(EMLINK)
	{ ERROR_TOO_MANY_LINKS, EMLINK },
#endif /* ERROR_TOO_MANY_LINKS && EMLINK */
#if defined(ERROR_UNEXP_NET_ERR) && defined(EIO)
	{ ERROR_UNEXP_NET_ERR, EIO },
#endif /* ERROR_UNEXP_NET_ERR && EIO */
#if defined(ERROR_WORKING_SET_QUOTA) && defined(EAGAIN)
	{ ERROR_WORKING_SET_QUOTA, EAGAIN },
#endif /* ERROR_WORKING_SET_QUOTA && EAGAIN */
#if defined(ERROR_WRITE_PROTECT) && defined(EROFS)
	{ ERROR_WRITE_PROTECT, EROFS },
#endif /* ERROR_WRITE_PROTECT && EROFS */
#endif /* NT2ERRNO_SRC_CYGWIN */


	/* NT error codes that only appear in MSVC's translation table. */
#ifdef NT2ERRNO_SRC_MSVC
#if defined(ERROR_ARENA_TRASHED) && defined(ENOMEM)
	{ ERROR_ARENA_TRASHED, ENOMEM },
#endif /* ERROR_ARENA_TRASHED && ENOMEM */
#if defined(ERROR_INVALID_BLOCK) && defined(ENOMEM)
	{ ERROR_INVALID_BLOCK, ENOMEM },
#endif /* ERROR_INVALID_BLOCK && ENOMEM */
#if defined(ERROR_BAD_ENVIRONMENT) && defined(E2BIG)
	{ ERROR_BAD_ENVIRONMENT, E2BIG },
#endif /* ERROR_BAD_ENVIRONMENT && E2BIG */
#if defined(ERROR_BAD_FORMAT) && defined(ENOEXEC)
	{ ERROR_BAD_FORMAT, ENOEXEC },
#endif /* ERROR_BAD_FORMAT && ENOEXEC */
#if defined(ERROR_INVALID_ACCESS) && defined(EINVAL)
	{ ERROR_INVALID_ACCESS, EINVAL },
#endif /* ERROR_INVALID_ACCESS && EINVAL */
#if defined(ERROR_CURRENT_DIRECTORY) && defined(EACCES)
	{ ERROR_CURRENT_DIRECTORY, EACCES },
#endif /* ERROR_CURRENT_DIRECTORY && EACCES */
#if defined(ERROR_NETWORK_ACCESS_DENIED) && defined(EACCES)
	{ ERROR_NETWORK_ACCESS_DENIED, EACCES },
#endif /* ERROR_NETWORK_ACCESS_DENIED && EACCES */
#if defined(ERROR_FAIL_I24) && defined(EACCES)
	{ ERROR_FAIL_I24, EACCES },
#endif /* ERROR_FAIL_I24 && EACCES */
#if defined(ERROR_DRIVE_LOCKED) && defined(EACCES)
	{ ERROR_DRIVE_LOCKED, EACCES },
#endif /* ERROR_DRIVE_LOCKED && EACCES */
#if defined(ERROR_INVALID_TARGET_HANDLE) && defined(EBADF)
	{ ERROR_INVALID_TARGET_HANDLE, EBADF },
#endif /* ERROR_INVALID_TARGET_HANDLE && EBADF */
#if defined(ERROR_DIRECT_ACCESS_HANDLE) && defined(EBADF)
	{ ERROR_DIRECT_ACCESS_HANDLE, EBADF },
#endif /* ERROR_DIRECT_ACCESS_HANDLE && EBADF */
#if defined(ERROR_SEEK_ON_DEVICE) && defined(EACCES)
	{ ERROR_SEEK_ON_DEVICE, EACCES },
#endif /* ERROR_SEEK_ON_DEVICE && EACCES */
#if defined(ERROR_NOT_LOCKED) && defined(EACCES)
	{ ERROR_NOT_LOCKED, EACCES },
#endif /* ERROR_NOT_LOCKED && EACCES */
#if defined(ERROR_LOCK_FAILED) && defined(EACCES)
	{ ERROR_LOCK_FAILED, EACCES },
#endif /* ERROR_LOCK_FAILED && EACCES */
#if defined(ERROR_NESTING_NOT_ALLOWED) && defined(EAGAIN)
	{ ERROR_NESTING_NOT_ALLOWED, EAGAIN },
#endif /* ERROR_NESTING_NOT_ALLOWED && EAGAIN */
#endif /* NT2ERRNO_SRC_MSVC */

	/* NT error codes for which CYGWIN and MSVC have the same answer */
#if defined(NT2ERRNO_SRC_MSVC) || defined(NT2ERRNO_SRC_CYGWIN)
#if defined(ERROR_FILE_NOT_FOUND) && defined(ENOENT)
	{ ERROR_FILE_NOT_FOUND, ENOENT },
#endif /* ERROR_FILE_NOT_FOUND && ENOENT */
#if defined(ERROR_PATH_NOT_FOUND) && defined(ENOENT)
	{ ERROR_PATH_NOT_FOUND, ENOENT },
#endif /* ERROR_PATH_NOT_FOUND && ENOENT */
#if defined(ERROR_TOO_MANY_OPEN_FILES) && defined(EMFILE)
	{ ERROR_TOO_MANY_OPEN_FILES, EMFILE },
#endif /* ERROR_TOO_MANY_OPEN_FILES && EMFILE */
#if defined(ERROR_ACCESS_DENIED) && defined(EACCES)
	{ ERROR_ACCESS_DENIED, EACCES },
#endif /* ERROR_ACCESS_DENIED && EACCES */
#if defined(ERROR_NOT_ENOUGH_MEMORY) && defined(ENOMEM)
	{ ERROR_NOT_ENOUGH_MEMORY, ENOMEM },
#endif /* ERROR_NOT_ENOUGH_MEMORY && ENOMEM */
#if defined(ERROR_INVALID_DATA) && defined(EINVAL)
	{ ERROR_INVALID_DATA, EINVAL },
#endif /* ERROR_INVALID_DATA && EINVAL */
#if defined(ERROR_NOT_SAME_DEVICE) && defined(EXDEV)
	{ ERROR_NOT_SAME_DEVICE, EXDEV },
#endif /* ERROR_NOT_SAME_DEVICE && EXDEV */
#if defined(ERROR_BAD_NETPATH) && defined(ENOENT)
	{ ERROR_BAD_NETPATH, ENOENT },
#endif /* ERROR_BAD_NETPATH && ENOENT */
#if defined(ERROR_BAD_NET_NAME) && defined(ENOENT)
	{ ERROR_BAD_NET_NAME, ENOENT },
#endif /* ERROR_BAD_NET_NAME && ENOENT */
#if defined(ERROR_FILE_EXISTS) && defined(EEXIST)
	{ ERROR_FILE_EXISTS, EEXIST },
#endif /* ERROR_FILE_EXISTS && EEXIST */
#if defined(ERROR_INVALID_PARAMETER) && defined(EINVAL)
	{ ERROR_INVALID_PARAMETER, EINVAL },
#endif /* ERROR_INVALID_PARAMETER && EINVAL */
#if defined(ERROR_NO_PROC_SLOTS) && defined(EAGAIN)
	{ ERROR_NO_PROC_SLOTS, EAGAIN },
#endif /* ERROR_NO_PROC_SLOTS && EAGAIN */
#if defined(ERROR_BROKEN_PIPE) && defined(EPIPE)
	{ ERROR_BROKEN_PIPE, EPIPE },
#endif /* ERROR_BROKEN_PIPE && EPIPE */
#if defined(ERROR_DISK_FULL) && defined(ENOSPC)
	{ ERROR_DISK_FULL, ENOSPC },
#endif /* ERROR_DISK_FULL && ENOSPC */
#if defined(ERROR_WAIT_NO_CHILDREN) && defined(ECHILD)
	{ ERROR_WAIT_NO_CHILDREN, ECHILD },
#endif /* ERROR_WAIT_NO_CHILDREN && ECHILD */
#if defined(ERROR_NEGATIVE_SEEK) && defined(EINVAL)
	{ ERROR_NEGATIVE_SEEK, EINVAL },
#endif /* ERROR_NEGATIVE_SEEK && EINVAL */
#if defined(ERROR_DIR_NOT_EMPTY) && defined(ENOTEMPTY)
	{ ERROR_DIR_NOT_EMPTY, ENOTEMPTY },
#endif /* ERROR_DIR_NOT_EMPTY && ENOTEMPTY */
#if defined(ERROR_BAD_PATHNAME) && defined(ENOENT)
	{ ERROR_BAD_PATHNAME, ENOENT },
#endif /* ERROR_BAD_PATHNAME && ENOENT */
#if defined(ERROR_MAX_THRDS_REACHED) && defined(EAGAIN)
	{ ERROR_MAX_THRDS_REACHED, EAGAIN },
#endif /* ERROR_MAX_THRDS_REACHED && EAGAIN */
#if defined(ERROR_ALREADY_EXISTS) && defined(EEXIST)
	{ ERROR_ALREADY_EXISTS, EEXIST },
#endif /* ERROR_ALREADY_EXISTS && EEXIST */
#endif /* NT2ERRNO_SRC_MSVC || NT2ERRNO_SRC_CYGWIN */

	/* NT error codes for which CYGWIN and MSVC have different answers */
#if defined(NT2ERRNO_SRC_CYGWIN) || defined(NT2ERRNO_SRC_MSVC)
#ifdef ERROR_INVALID_FUNCTION
#if defined(EBADRQC) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_INVALID_FUNCTION, EBADRQC },
#elif defined(EINVAL)
	{ ERROR_INVALID_FUNCTION, EINVAL },
#elif defined(EBADRQC)
	{ ERROR_INVALID_FUNCTION, EBADRQC },
#endif
#endif /* ERROR_INVALID_FUNCTION */

#ifdef ERROR_INVALID_DRIVE
#if defined(ENODEV) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_INVALID_DRIVE, ENODEV },
#elif defined(ENOENT)
	{ ERROR_INVALID_DRIVE, ENOENT },
#elif defined(ENODEV)
	{ ERROR_INVALID_DRIVE, ENODEV },
#endif
#endif /* ERROR_INVALID_DRIVE */

#ifdef ERROR_NO_MORE_FILES
#if defined(ENMFILE) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_NO_MORE_FILES, ENMFILE },
#elif defined(ENOENT)
	{ ERROR_NO_MORE_FILES, ENOENT },
#elif defined(ENMFILE)
	{ ERROR_NO_MORE_FILES, ENMFILE },
#endif
#endif /* ERROR_NO_MORE_FILES */

#ifdef ERROR_INVALID_DRIVE
#if defined(EBUSY) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_LOCK_VIOLATION, EBUSY },
#elif defined(EACCES)
	{ ERROR_LOCK_VIOLATION, EACCES },
#elif defined(EBUSY)
	{ ERROR_LOCK_VIOLATION, EBUSY },
#endif
#endif /* ERROR_INVALID_DRIVE */

#ifdef ERROR_CANNOT_MAKE
#if defined(EPERM) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_CANNOT_MAKE, EPERM },
#elif defined(EACCES)
	{ ERROR_CANNOT_MAKE, EACCES },
#elif defined(EPERM)
	{ ERROR_CANNOT_MAKE, EPERM },
#endif
#endif /* ERROR_CANNOT_MAKE */

#ifdef ERROR_INVALID_HANDLE
#if defined(EBADF) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_INVALID_HANDLE, EBADF },
#elif defined(EINVAL)
	{ ERROR_INVALID_HANDLE, EINVAL },
#elif defined(EBADF)
	{ ERROR_INVALID_HANDLE, EBADF },
#endif
#endif /* ERROR_INVALID_HANDLE */

#ifdef ERROR_CHILD_NOT_COMPLETE
#if defined(EBUSY) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_CHILD_NOT_COMPLETE, EBUSY },
#elif defined(ECHILD)
	{ ERROR_CHILD_NOT_COMPLETE, ECHILD },
#elif defined(EBUSY)
	{ ERROR_CHILD_NOT_COMPLETE, EBUSY },
#endif
#endif /* ERROR_CHILD_NOT_COMPLETE */

#ifdef ERROR_FILENAME_EXCED_RANGE
#if defined(ENAMETOOLONG) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_FILENAME_EXCED_RANGE, ENAMETOOLONG },
#elif defined(ENOENT)
	{ ERROR_FILENAME_EXCED_RANGE, ENOENT },
#elif defined(ENAMETOOLONG)
	{ ERROR_FILENAME_EXCED_RANGE, ENAMETOOLONG },
#endif
#endif /* ERROR_FILENAME_EXCED_RANGE */

#ifdef ERROR_NOT_ENOUGH_QUOTA
#if defined(EIO) && !defined(NT2ERRNO_PREFER_MSVC)
	{ ERROR_NOT_ENOUGH_QUOTA, EIO },
#elif defined(ENOMEM)
	{ ERROR_NOT_ENOUGH_QUOTA, ENOMEM },
#elif defined(EIO)
	{ ERROR_NOT_ENOUGH_QUOTA, EIO },
#endif
#endif /* ERROR_NOT_ENOUGH_QUOTA */
#endif /* NT2ERRNO_SRC_CYGWIN || NT2ERRNO_SRC_MSVC */

	{ 0, 0 }
};
#endif /* DeeNTSystem_Translate(Errno|NtError)_USE_FALLBACK */

/* Translate a given `dwError' into the appropriate `errno' error code.
 * If the translation failed, return a fallback value.
 * Note that (if possible), the implementation of this function is handled by the
 * linked C library, using MSVC's `_dosmaperr()' (if available). Otherwise, the
 * translation is performed identical to what is known to be done by the linked
 * C library, or a combination of CYGWIN and MSVC if some other libc is hosting
 * deemon within a windows environment.
 * NOTE: This function is also used by `DeeNTSystem_ThrowErrorf()' to translate
 *       the given NT error code into an errno. */
PUBLIC ATTR_CONST WUNUSED /*errno_t*/ int DCALL
DeeNTSystem_TranslateErrno(/*DWORD*/ DeeNT_DWORD dwError) {
#ifdef DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS
	return errno_nt2kos(dwError);
#endif /* DeeNTSystem_TranslateErrno_USE_ERRNO_NT2KOS */
#ifdef DeeNTSystem_TranslateErrno_USE_DOSMAPERR
	/*errno_t*/ int result, old_errno;
	IF_CONFIG_HAVE__doserrno(DeeNT_DWORD old_doserrno);
	if (dwError == 0)
		return 0; /* Special case: No error */
	old_errno = DeeSystem_GetErrno();
	IF_CONFIG_HAVE__doserrno(old_doserrno = _doserrno);
	_dosmaperr(dwError);
	result = DeeSystem_GetErrno();
	IF_CONFIG_HAVE__doserrno(_doserrno = old_doserrno);
	DeeSystem_SetErrno(old_errno);
	return result;
#endif /* DeeNTSystem_TranslateErrno_USE_DOSMAPERR */
#ifdef DeeNTSystem_TranslateErrno_USE_FALLBACK
	unsigned int i;
	if (dwError == 0)
		return 0; /* Special case: No error */
	for (i = 0; nt2errno[i].etval; ++i) {
		if (nt2errno[i].etval == dwError)
			return nt2errno[i].erval;
	}
	/* Fallback: Check if we recognize the error */
#ifdef ENOENT
	if (DeeNTSystem_IsFileNotFoundError(dwError))
		return ENOENT;
#endif /* ENOENT */
#ifdef EACCES
	if (DeeNTSystem_IsAccessDeniedError(dwError))
		return EACCES;
#endif /* EACCES */
#ifdef ENOMEM
	if (DeeNTSystem_IsBadAllocError(dwError))
		return ENOMEM;
#endif /* ENOMEM */
#ifdef EBUSY
	if (DeeNTSystem_IsBusy(dwError))
		return EBUSY;
#endif /* EBUSY */
#ifdef EEXIST
	if (DeeNTSystem_IsExists(dwError))
		return EEXIST;
#endif /* EEXIST */
#ifdef ENOTDIR
	if (DeeNTSystem_IsNotDir(dwError))
		return ENOTDIR;
#endif /* ENOTDIR */
#ifdef ENOTEMPTY
	if (DeeNTSystem_IsNotEmpty(dwError))
		return ENOTEMPTY;
#endif /* ENOTEMPTY */
#ifdef EBADF
	if (DeeNTSystem_IsBadF(dwError))
		return EBADF;
#endif /* EBADF */
#ifdef EXDEV
	if (DeeNTSystem_IsXDev(dwError))
		return EXDEV;
#endif /* EXDEV */
#ifdef ENOSYS
	if (DeeNTSystem_IsUnsupportedError(dwError))
		return ENOSYS;
#endif /* ENOSYS */
#ifdef EINVAL
	if (DeeNTSystem_IsInvalidArgument(dwError))
		return EINVAL;
#endif /* EINVAL */
	/* Fallback `EACCES' */
#undef HAVE_RETURN
#if defined(NT2ERRNO_SRC_CYGWIN) || defined(NT2ERRNO_SRC_MSVC)
#if defined(EACCES) && !defined(NT2ERRNO_PREFER_MSVC)
#define HAVE_RETURN 1
	return EACCES;
#elif defined(EINVAL)
#define HAVE_RETURN 1
	return EINVAL;
#elif defined(EACCES)
#define HAVE_RETURN 1
	return EACCES;
#endif /* ... */
#endif /* NT2ERRNO_SRC_CYGWIN || NT2ERRNO_SRC_MSVC */
#ifndef HAVE_RETURN
	return Dee_SYSTEM_ERROR_UNKNOWN;
#endif /* !HAVE_RETURN */
#endif /* DeeNTSystem_TranslateErrno_USE_FALLBACK */
}



/* Do the reverse of `DeeNTSystem_TranslateErrno()' */
PUBLIC ATTR_CONST WUNUSED /*DWORD*/ DeeNT_DWORD DCALL
DeeNTSystem_TranslateNtError(/*errno_t*/ int errno_value) {
#ifdef DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT
	return errno_kos2nt(errno_value);
#endif /* DeeNTSystem_TranslateNtError_USE_ERRNO_KOS2NT */

#ifdef DeeNTSystem_TranslateNtError_USE_FALLBACK
	unsigned int i;
	if (errno_value == 0)
		return 0; /* Special case: No error */
	for (i = 0; nt2errno[i].etval; ++i) {
		if (nt2errno[i].erval == errno_value)
			return nt2errno[i].etval;
	}
	/* Fallback... */
	return ERROR_INVALID_FUNCTION;
#endif /* DeeNTSystem_TranslateNtError_USE_FALLBACK */
}



/* Throw NT system errors, given an error code as returned by `GetLastError()'
 * When no error code is given, `GetLastError()' is called internally.
 * When `tp' is `NULL', the proper error type is automatically determined using the `DeeNTSystem_Is*' functions.
 * @return: -1: These functions always return -1 */
PUBLIC ATTR_COLD NONNULL((3)) int
(DCALL DeeNTSystem_VThrowErrorf)(DeeTypeObject *tp,
                                 DeeNT_DWORD dwError,
                                 char const *__restrict format,
                                 va_list args) {
	int result;
	if (tp == NULL) {
		/* Automatically select the proper error class. */
		if (DeeNTSystem_IsFileNotFoundError(dwError)) {
			tp = &DeeError_FileNotFound;
		} else if (DeeNTSystem_IsExists(dwError)) {
			tp = &DeeError_FileExists;
		} else if (DeeNTSystem_IsBadF(dwError)) {
			tp = &DeeError_FileClosed;
		} else if (DeeNTSystem_IsAccessDeniedError(dwError)) {
			tp = &DeeError_FileAccessError;
		} else if (DeeNTSystem_IsBadAllocError(dwError)) {
			tp = &DeeError_NoMemory;
		} else if (DeeNTSystem_IsUnsupportedError(dwError)) {
			tp = &DeeError_UnsupportedAPI;
		} else if (DeeNTSystem_IsInvalidArgument(dwError)) {
			tp = &DeeError_ValueError;
		} else if (DeeNTSystem_IsBusy(dwError)) {
			tp = &DeeError_BusyFile;
		} else if (DeeNTSystem_IsNotDir(dwError)) {
			tp = &DeeError_NoDirectory;
		} else if (DeeNTSystem_IsNotEmpty(dwError)) {
			tp = &DeeError_DirectoryNotEmpty;
		} else if (DeeNTSystem_IsXDev(dwError)) {
			tp = &DeeError_CrossDeviceLink;
		} else if (DeeNTSystem_IsNoLink(dwError)) {
			tp = &DeeError_NoSymlink;
		} else {
			/* Fallback: Just use a SystemError */
			tp = &DeeError_SystemError;
		}
	}

	/* Check for error types derived from `errors.SystemError' */
	if (DeeType_Check(tp) &&
	    DeeType_Extends(tp, &DeeError_SystemError)) {
		DREF DeeSystemErrorObject *error;
		DREF DeeStringObject *message;
		error = (DREF DeeSystemErrorObject *)DeeObject_MALLOC(DeeSystemErrorObject);
		if unlikely(!error)
			goto err;
		message = (DREF DeeStringObject *)DeeString_VNewf(format, args);
		if unlikely(!message) {
			DeeObject_FREE(error);
			goto err;
		}
		error->e_msg    = (DeeObject *)message; /* Inherit reference */
		error->e_cause      = NULL;
		error->se_errno     = DeeNTSystem_TranslateErrno(dwError);
		error->se_lasterror = dwError;
		DeeObject_Init(error, tp);
		result = DeeError_ThrowInherited((DeeObject *)error);
	} else {
		/* Unlikely to happen: Just throw a regular, old error. */
		result = DeeError_VThrowf(tp, format, args);
	}
	return result;
err:
	return -1;
}

PUBLIC ATTR_COLD NONNULL((2)) int
(DCALL DeeNTSystem_VThrowLastErrorf)(DeeTypeObject *tp,
                                     char const *__restrict format,
                                     va_list args) {
	int result;
	DeeNT_DWORD dwError;
	DBG_ALIGNMENT_DISABLE();
	dwError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	result = DeeNTSystem_VThrowErrorf(tp,
	                                  dwError,
	                                  format,
	                                  args);
	return result;
}

PUBLIC ATTR_COLD NONNULL((3)) int
(DeeNTSystem_ThrowErrorf)(DeeTypeObject *tp,
                          DeeNT_DWORD dwError,
                          char const *__restrict format, ...) {
	int result;
	va_list args;
	va_start(args, format);
	result = DeeNTSystem_VThrowErrorf(tp, dwError, format, args);
	va_end(args);
	return result;
}

PUBLIC ATTR_COLD NONNULL((2)) int
(DeeNTSystem_ThrowLastErrorf)(DeeTypeObject *tp,
                              char const *__restrict format, ...) {
	int result;
	va_list args;
	va_start(args, format);
	result = DeeNTSystem_VThrowLastErrorf(tp, format, args);
	va_end(args);
	return result;
}


/* Work around a problem with long path names. (Note: also handles interrupts)
 * @return: * :                   The new handle.
 * @return: NULL:                 A deemon callback failed and an error was thrown.
 * @return: INVALID_HANDLE_VALUE: The system call failed (s.a. `GetLastError()') */
PUBLIC WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_CreateFile(/*String*/ DeeObject *__restrict lpFileName,
                       /*DWORD*/ DeeNT_DWORD dwDesiredAccess,
                       /*DWORD*/ DeeNT_DWORD dwShareMode,
                       /*LPSECURITY_ATTRIBUTES*/ void *lpSecurityAttributes,
                       /*DWORD*/ DeeNT_DWORD dwCreationDisposition,
                       /*DWORD*/ DeeNT_DWORD dwFlagsAndAttributes,
                       /*HANDLE*/ void *hTemplateFile) {
	HANDLE hResult;
	LPWSTR lpwName;
	ASSERT_OBJECT_TYPE_EXACT(lpFileName, &DeeString_Type);
#ifdef CONFIG_WANT_WINDOWS_STD_FILES
#define eqnocase(a, b_upper) ((a) == (b_upper) || (a) == ((b_upper) - 'A' + 'a'))
	if (DeeString_SIZE(lpFileName) >= 6 &&
	    eqnocase(DeeString_STR(lpFileName)[0], 'S') &&
	    eqnocase(DeeString_STR(lpFileName)[1], 'T') &&
	    eqnocase(DeeString_STR(lpFileName)[2], 'D')) {
		if (DeeString_SIZE(lpFileName) == 6 &&
		    eqnocase(DeeString_STR(lpFileName)[3], 'I') &&
		    eqnocase(DeeString_STR(lpFileName)[4], 'N') &&
		    DeeString_STR(lpFileName)[5] == '$') {
			DBG_ALIGNMENT_DISABLE();
			hResult = GetStdHandle(STD_INPUT_HANDLE);
do_copy_and_return_hResult:
			if (dwCreationDisposition == CREATE_NEW ||
			    dwCreationDisposition == CREATE_ALWAYS ||
			    dwCreationDisposition == TRUNCATE_EXISTING) {
				/* These modes cannot be used to access STD* files. */
				(void)SetLastError(ERROR_INVALID_PARAMETER);
				DBG_ALIGNMENT_ENABLE();
				return INVALID_HANDLE_VALUE;
			}

			if (!DuplicateHandle(GetCurrentProcess(), hResult,
			                     GetCurrentProcess(), &hResult,
			                     dwDesiredAccess, FALSE, 0)) {
				DBG_ALIGNMENT_ENABLE();
				return INVALID_HANDLE_VALUE;
			}
			DBG_ALIGNMENT_ENABLE();
			if unlikely(hResult == NULL)
				hResult = INVALID_HANDLE_VALUE;
			return hResult;
		}
		if (DeeString_SIZE(lpFileName) == 7 &&
		    DeeString_STR(lpFileName)[6] == '$') {
			if (eqnocase(DeeString_STR(lpFileName)[3], 'O') &&
			    eqnocase(DeeString_STR(lpFileName)[4], 'U') &&
			    eqnocase(DeeString_STR(lpFileName)[5], 'T')) {
				DBG_ALIGNMENT_DISABLE();
				hResult = GetStdHandle(STD_OUTPUT_HANDLE);
				goto do_copy_and_return_hResult;
			}
			if (eqnocase(DeeString_STR(lpFileName)[3], 'E') &&
			    eqnocase(DeeString_STR(lpFileName)[4], 'R') &&
			    eqnocase(DeeString_STR(lpFileName)[5], 'R')) {
				DBG_ALIGNMENT_DISABLE();
				hResult = GetStdHandle(STD_ERROR_HANDLE);
				goto do_copy_and_return_hResult;
			}
		}
	}
#undef eqnocase
#endif /* CONFIG_WANT_WINDOWS_STD_FILES */
	lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
	if unlikely(!lpwName)
		goto err;
again_createfile:
	DBG_ALIGNMENT_DISABLE();
	hResult = CreateFileW(lpwName,
	                      (DWORD)dwDesiredAccess,
	                      (DWORD)dwShareMode,
	                      (LPSECURITY_ATTRIBUTES)lpSecurityAttributes,
	                      (DWORD)dwCreationDisposition,
	                      (DWORD)dwFlagsAndAttributes,
	                      (HANDLE)hTemplateFile);
	if (hResult == INVALID_HANDLE_VALUE && *lpwName) {
		DWORD dwError = GetLastError();
		if (DeeNTSystem_IsUncError(dwError)) {
			/* Fix the filename and try again. */
			DBG_ALIGNMENT_ENABLE();
			lpFileName = DeeNTSystem_FixUncPath(lpFileName);
			if unlikely(!lpFileName)
				goto err;
			lpwName = (LPWSTR)DeeString_AsWide(lpFileName);
			if unlikely(!lpwName) {
				Dee_Decref(lpFileName);
				goto err;
			}
			DBG_ALIGNMENT_DISABLE();
			hResult = CreateFileW(lpwName,
			                     (DWORD)dwDesiredAccess,
			                     (DWORD)dwShareMode,
			                     (LPSECURITY_ATTRIBUTES)lpSecurityAttributes,
			                     (DWORD)dwCreationDisposition,
			                     (DWORD)dwFlagsAndAttributes,
			                     (HANDLE)hTemplateFile);
			DBG_ALIGNMENT_ENABLE();
			Dee_Decref(lpFileName);
		} else if (DeeNTSystem_IsIntr(dwError)) {
			DBG_ALIGNMENT_ENABLE();
			if (DeeThread_CheckInterrupt())
				goto err;
			goto again_createfile;
		} else if (DeeNTSystem_IsBadAllocError(dwError)) {
			DBG_ALIGNMENT_ENABLE();
			if (Dee_ReleaseSystemMemory())
				goto again_createfile;
			goto err;
		}
	}
	DBG_ALIGNMENT_ENABLE();
	if unlikely(hResult == NULL)
		hResult = INVALID_HANDLE_VALUE; /* Shouldn't happen... */
	return hResult;
err:
	return NULL;
}

/* Same as `DeeNTSystem_CreateFile()', but try not to modify the file's last-accessed timestamp
 * @return: * :                   The new handle.
 * @return: NULL:                 A deemon callback failed and an error was thrown.
 * @return: INVALID_HANDLE_VALUE: The system call failed (s.a. `GetLastError()') */
PUBLIC WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_CreateFileNoATime(/*String*/ DeeObject *__restrict lpFileName,
                              /*DWORD*/ DeeNT_DWORD dwDesiredAccess,
                              /*DWORD*/ DeeNT_DWORD dwShareMode,
                              /*LPSECURITY_ATTRIBUTES*/ void *lpSecurityAttributes,
                              /*DWORD*/ DeeNT_DWORD dwCreationDisposition,
                              /*DWORD*/ DeeNT_DWORD dwFlagsAndAttributes,
                              /*HANDLE*/ void *hTemplateFile) {
	HANDLE hResult;
	/* Need to add `FILE_WRITE_ATTRIBUTES' so we're allowed to call `SetFileTime()' */
	hResult = DeeNTSystem_CreateFile(lpFileName,
	                                 dwDesiredAccess | FILE_WRITE_ATTRIBUTES,
	                                 dwShareMode,
	                                 lpSecurityAttributes,
	                                 dwCreationDisposition,
	                                 dwFlagsAndAttributes,
	                                 hTemplateFile);
	if (hResult == INVALID_HANDLE_VALUE) {
		if (!(dwDesiredAccess & FILE_WRITE_ATTRIBUTES)) {
			hResult = DeeNTSystem_CreateFile(lpFileName,
			                                 dwDesiredAccess,
			                                 dwShareMode,
			                                 lpSecurityAttributes,
			                                 dwCreationDisposition,
			                                 dwFlagsAndAttributes,
			                                 hTemplateFile);
		}
	} else if (hResult != NULL) {
		BOOL bResult;
		FILETIME ftLastAccessed;
		ftLastAccessed.dwLowDateTime  = (DWORD)UINT32_C(0xffffffff);
		ftLastAccessed.dwHighDateTime = (DWORD)UINT32_C(0xffffffff);
		DBG_ALIGNMENT_DISABLE();
		bResult = SetFileTime((HANDLE)hResult, NULL, &ftLastAccessed, NULL);
		DBG_ALIGNMENT_ENABLE();
#ifdef Dee_DPRINT_IS_NOOP
		(void)bResult;
#else /* Dee_DPRINT_IS_NOOP */
		if (!bResult) {
			DWORD dwError;
			DBG_ALIGNMENT_DISABLE();
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			Dee_DPRINTF("DeeNTSystem_CreateFileNoATime: SetFileTime: GetLastError: %lu\n", dwError);
		}
#endif /* !Dee_DPRINT_IS_NOOP */
	}
	return hResult;
}


/* Wrapper around `DeeNTSystem_CreateFile()' and `DeeNTSystem_CreateFileNoATime()'
 * that is used to implement `posix.open()' and `File.open()' by taking unix-like
 * oflags and mode.
 * @param: oflags: Set of `Dee_OPEN_F*'
 * @param: mode:   When no bits from `0444' are set, use `FILE_ATTRIBUTE_READONLY'
 * @return: * :    The new handle.
 * @return: NULL:  A deemon callback failed and an error was thrown.
 * @return: INVALID_HANDLE_VALUE: File not found (`!Dee_OPEN_FCREAT') or already
 *                                exists (`Dee_OPEN_FCREAT | Dee_OPEN_FEXCL') */
PUBLIC WUNUSED NONNULL((1)) /*HANDLE*/ void *DCALL
DeeNTSystem_OpenFile(/*String*/ DeeObject *__restrict filename, int oflags, int mode) {
	PRIVATE DWORD const generic_access[4] = {
		/* [OPEN_FRDONLY] = */ FILE_GENERIC_READ,
		/* [OPEN_FWRONLY] = */ FILE_GENERIC_WRITE,
		/* [OPEN_FRDWR]   = */ FILE_GENERIC_READ | FILE_GENERIC_WRITE,
		/* [0x3]          = */ FILE_GENERIC_READ | FILE_GENERIC_WRITE
	};
	HANDLE hFile;
	DWORD dwDesiredAccess, dwShareMode;
	DWORD dwCreationDisposition, dwFlagsAndAttributes;
	dwDesiredAccess      = generic_access[oflags & OPEN_FACCMODE];
	dwShareMode          = (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
	dwFlagsAndAttributes = (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS);
	/* Apply exclusivity flags. */
	if (oflags & OPEN_FXREAD)
		dwShareMode &= ~(FILE_SHARE_READ);
	if (oflags & OPEN_FXWRITE)
		dwShareMode &= ~(FILE_SHARE_WRITE);
	if (oflags & OPEN_FCREAT) {
		if (oflags & OPEN_FEXCL) {
			dwCreationDisposition = CREATE_NEW;
		} else {
			dwCreationDisposition = ((oflags & OPEN_FTRUNC)
			                         ? CREATE_ALWAYS
			                         : OPEN_ALWAYS);
		}
		if (!(mode & 0444))
			dwFlagsAndAttributes |= FILE_ATTRIBUTE_READONLY;
	} else {
		dwCreationDisposition = ((oflags & OPEN_FTRUNC)
		                         ? TRUNCATE_EXISTING
		                         : OPEN_EXISTING);
	}
	if ((oflags & OPEN_FAPPEND) &&
	    (oflags & OPEN_FACCMODE) != OPEN_FRDONLY) {
#if (FILE_GENERIC_WRITE & FILE_APPEND_DATA) == 0
		dwDesiredAccess |= FILE_APPEND_DATA;
#endif /* (FILE_GENERIC_WRITE & FILE_APPEND_DATA) == 0 */
		dwDesiredAccess &= ~FILE_WRITE_DATA;
	}
	if (oflags & (OPEN_FDIRECT | OPEN_FSYNC))
		dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
	if (oflags & OPEN_FHIDDEN)
		dwFlagsAndAttributes |= FILE_ATTRIBUTE_HIDDEN;
	if (oflags & OPEN_FNOFOLLOW)
		dwFlagsAndAttributes |= FILE_FLAG_OPEN_REPARSE_POINT;
again:
	if (oflags & OPEN_FNOATIME) {
		hFile = DeeNTSystem_CreateFileNoATime(filename, dwDesiredAccess, dwShareMode, NULL,
		                                      dwCreationDisposition, dwFlagsAndAttributes, NULL);
	} else {
		hFile = DeeNTSystem_CreateFile(filename, dwDesiredAccess, dwShareMode, NULL,
		                               dwCreationDisposition, dwFlagsAndAttributes, NULL);
	}
	if unlikely(!hFile)
		goto err;
	if unlikely(hFile == INVALID_HANDLE_VALUE) {
		DWORD error;
		DBG_ALIGNMENT_DISABLE();
		error = GetLastError();
		DBG_ALIGNMENT_ENABLE();

/*check_nt_error:*/
		/* Handle file already-exists. */
		if ((error == ERROR_FILE_EXISTS) && (oflags & OPEN_FEXCL))
			return INVALID_HANDLE_VALUE;

		/* Throw the error as an NT error. */
		if (DeeNTSystem_IsBadAllocError(error)) {
			if (Dee_ReleaseSystemMemory())
				goto again;
		} else if (DeeNTSystem_IsNotDir(error) ||
		           DeeNTSystem_IsFileNotFoundError(error)) {
			if (!(oflags & OPEN_FCREAT))
				return INVALID_HANDLE_VALUE;
			DeeNTSystem_ThrowErrorf(&DeeError_FileNotFound, error,
			                        "File %r could not be found",
			                        filename);
		} else if (DeeNTSystem_IsAccessDeniedError(error)) {
			DeeNTSystem_ThrowErrorf(&DeeError_FileAccessError, error,
			                        "Access has not been granted for file %r",
			                        filename);
		} else {
			DeeNTSystem_ThrowErrorf(&DeeError_FSError, error,
			                        "Failed to obtain a writable handle for %r",
			                        filename);
		}
		goto err;
	}

	/* When the file is a pipe (which it can be if `filename' starts with r"\\.\pipe\"),
	 * then we have to change the pipe to non-blocking if the caller wants it to be so.
	 *
	 * NOTE: We only need to do this if the caller set the NONBLOCK flag, since otherwise
	 *       the default behavior of a named pipe is to block:
	 * """PIPE_WAIT [...] This mode is the default if no wait-mode flag is specified""" */
	if (oflags & OPEN_FNONBLOCK) {
		static char const pipe_prefix[] = "\\\\.\\pipe\\";
		char const *filename_str = DeeString_STR(filename);
		if (memcasecmp(filename_str, pipe_prefix, COMPILER_STRLEN(pipe_prefix) * sizeof(char)) == 0) {
			DBG_ALIGNMENT_DISABLE();
			if (GetFileType(hFile) == FILE_TYPE_PIPE) {
				DWORD new_mode = /*!(oflags & OPEN_FNONBLOCK) ? PIPE_WAIT :*/ PIPE_NOWAIT;
				(void)SetNamedPipeHandleState(hFile, &new_mode, NULL, NULL);
			}
			DBG_ALIGNMENT_ENABLE();
		}
	}

#if 0 /* Technically we'd need to do this, but then again: \
       * Windows doesn't even have fork (natively...).     \
       * Also: ipc.Process() automatically manages this flag! */
	if (!(oflags & OPEN_FCLOEXEC)) {
		DBG_ALIGNMENT_DISABLE();
		SetHandleInformation(hFile, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		DBG_ALIGNMENT_ENABLE();
	}
#endif

	return hFile;
/*
err_fp:
	(void)CloseHandle(hFile);*/
err:
	return NULL;
}


/* Determine the filename from a handle, as returned by `DeeNTSystem_CreateFile()' */
PUBLIC WUNUSED /*String*/ DREF DeeObject *DCALL
DeeNTSystem_GetFilenameOfHandle(/*HANDLE*/ void *hFile) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeNTSystem_PrintFilenameOfHandle(&printer, hFile);
	if unlikely(error != 0) {
		if (error > 0) {
			DeeNTSystem_ThrowLastErrorf(NULL,
			                            "Failed to print path of HANDLE %p",
			                            hFile);
		}
		goto err;
	}
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

/* Same as `DeeNTSystem_GetFilenameOfHandle()', but return `ITER_DONE' rather than
 * throwing a SystemError when `DeeNTSystem_PrintFilenameOfHandle()' returns `1' */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_TryGetFilenameOfHandle(/*HANDLE*/ void *hFile) {
	int error;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeNTSystem_PrintFilenameOfHandle(&printer, hFile);
	if unlikely(error != 0) {
		if (error > 0) {
			unicode_printer_fini(&printer);
			return ITER_DONE;
		}
		goto err;
	}
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}



typedef DWORD (WINAPI *LPGETFINALPATHNAMEBYHANDLEW)(HANDLE hFile, LPWSTR lpszFilePath,
                                                    DWORD cchFilePath, DWORD dwFlags);
PRIVATE LPGETFINALPATHNAMEBYHANDLEW pdyn_GetFinalPathNameByHandleW = NULL;

#ifndef DEFINED_GET_KERNEL32_HANDLE
#define DEFINED_GET_KERNEL32_HANDLE 1
PRIVATE WCHAR const wKernel32[]    = { 'K', 'E', 'R', 'N', 'E', 'L', '3', '2', 0 };
PRIVATE WCHAR const wKernel32Dll[] = { 'K', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0 };
PRIVATE WUNUSED HMODULE DCALL GetKernel32Handle(void) {
	HMODULE hKernel32;
	hKernel32 = GetModuleHandleW(wKernel32);
	if (!hKernel32)
		hKernel32 = LoadLibraryW(wKernel32Dll);
	return hKernel32;
}
#endif /* !DEFINED_GET_KERNEL32_HANDLE */

/* Wrapper for the `GetFinalPathNameByHandle()' system call.
 * @return: 2:  Unsupported.
 * @return: 1:  The system call failed (s.a. `GetLastError()').
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_PrintFinalPathNameByHandle(struct unicode_printer *__restrict printer,
                                       /*HANDLE*/ void *hFile,
                                       /*DWORD*/ DeeNT_DWORD dwFlags) {
	LPWSTR lpNewBuffer, lpBuffer;
	DWORD dwNewBufSize, dwBufSize;
	if (!pdyn_GetFinalPathNameByHandleW) {
		/* Try to load `GetFinalPathNameByHandleW()' */
		HMODULE hKernel32;
		DBG_ALIGNMENT_DISABLE();
		hKernel32 = GetKernel32Handle();
		if (!hKernel32) {
			atomic_write((void **)&pdyn_GetFinalPathNameByHandleW, (void *)(uintptr_t)-1);
		} else {
			LPGETFINALPATHNAMEBYHANDLEW func;
			func = (LPGETFINALPATHNAMEBYHANDLEW)GetProcAddress(hKernel32, "GetFinalPathNameByHandleW");
			if (!func)
				*(void **)&func = (void *)(uintptr_t)-1;
			atomic_write(&pdyn_GetFinalPathNameByHandleW, func);
		}
		DBG_ALIGNMENT_ENABLE();
	}
	if (*(void **)&pdyn_GetFinalPathNameByHandleW == (void *)(uintptr_t)-1)
		return 2;

	/* Make use of `GetFinalPathNameByHandleW()' */
	dwBufSize = PATH_MAX;
	lpBuffer = unicode_printer_alloc_wchar(printer, dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwNewBufSize = (*pdyn_GetFinalPathNameByHandleW)((HANDLE)hFile,
		                                                 lpBuffer,
		                                                 dwBufSize,
		                                                 dwFlags);
		if unlikely(!dwNewBufSize) {
			DWORD dwError;
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_lpBuffer;
				continue;
			}
			if (DeeNTSystem_IsBufferTooSmall(dwError)) {
				dwNewBufSize = dwBufSize * 2;
				goto do_resize_buffer;
			}
			unicode_printer_free_wchar(printer, lpBuffer);
			DBG_ALIGNMENT_DISABLE();
			(void)SetLastError(dwError);
			DBG_ALIGNMENT_ENABLE();
			/* Some files don't support this function. -> treat these cases as unsupported. */
			if (dwError == ERROR_INVALID_FUNCTION ||
			    dwError == ERROR_INVALID_PARAMETER)
				return 2;
			return 1;
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwNewBufSize <= dwBufSize)
			break;
		--dwNewBufSize; /* This would include the trailing NUL-character */
do_resize_buffer:
		lpNewBuffer = unicode_printer_resize_wchar(printer, lpBuffer, dwNewBufSize);
		if unlikely(!lpNewBuffer) {
err_lpBuffer:
			unicode_printer_free_wchar(printer, lpBuffer);
			goto err;
		}
		dwBufSize = dwNewBufSize;
	}
	if unlikely(unicode_printer_commit_wchar(printer, lpBuffer, dwNewBufSize) < 0)
		goto err;
	return 0;
err:
	return -1;
}


typedef NTSTATUS (NTAPI *LPNTQUERYOBJECT)(HANDLE Handle, int /*OBJECT_INFORMATION_CLASS*/ ObjectInformationClass,
                                          PVOID ObjectInformation, ULONG ObjectInformationLength, PULONG ReturnLength);
PRIVATE LPNTQUERYOBJECT pdyn_NtQueryObject = NULL;

#ifndef DEFINED_GET_NTDLL_HANDLE
#define DEFINED_GET_NTDLL_HANDLE 1
PRIVATE WCHAR const wNtdll[]    = { 'N', 'T', 'D', 'L', 'L', 0 };
PRIVATE WCHAR const wNtdllDll[] = { 'N', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l', 0 };
PRIVATE WUNUSED HMODULE DCALL GetNtdllHandle(void) {
	HMODULE hNtdll;
	hNtdll = GetModuleHandleW(wNtdll);
	if (!hNtdll)
		hNtdll = LoadLibraryW(wNtdllDll);
	return hNtdll;
}
#endif /* !DEFINED_GET_NTDLL_HANDLE */


typedef struct {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} NT_UNICODE_STRING;

typedef struct {
	NT_UNICODE_STRING Name;
	WCHAR NameBuffer[1];
} NT_OBJECT_NAME_INFORMATION;

/* @return: 2:  Unsupported.
 * @return: 1:  The system call failed (s.a. `GetLastError()').
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_PrintNtQueryObject_ObjectNameInformation(struct unicode_printer *__restrict printer,
                                                     HANDLE hFile) {
	LPWSTR lpNewBuffer, lpBuffer;
	DWORD dwBufSize, dwBufSizeBytes;
	ULONG ulRetBufSize;
	NT_OBJECT_NAME_INFORMATION *ntInfo;
	if (!pdyn_NtQueryObject) {
		/* Try to load `NtQueryObject()' */
		HMODULE hNtdll;
		DBG_ALIGNMENT_DISABLE();
		hNtdll = GetNtdllHandle();
		if (!hNtdll) {
			atomic_write((void **)&pdyn_NtQueryObject, (void *)(uintptr_t)-1);
		} else {
			LPNTQUERYOBJECT func;
			func = (LPNTQUERYOBJECT)GetProcAddress(hNtdll, "NtQueryObject");
			if (!func)
				*(void **)&func = (void *)(uintptr_t)-1;
			atomic_write(&pdyn_NtQueryObject, func);
		}
		DBG_ALIGNMENT_ENABLE();
	}
	if (*(void **)&pdyn_NtQueryObject == (void *)(uintptr_t)-1)
		return 2;

#define LOCAL_ALLOC_BUFSIZE_FOR(n_chars) \
	((n_chars) + CEILDIV(offsetof(NT_OBJECT_NAME_INFORMATION, NameBuffer), sizeof(WCHAR)))

	/* Make use of `NtQueryObject()' */
	dwBufSize = PATH_MAX;
	lpBuffer  = unicode_printer_alloc_wchar(printer, LOCAL_ALLOC_BUFSIZE_FOR(dwBufSize));
	if unlikely(!lpBuffer)
		goto err;
	for (;;) {
		NTSTATUS ntResult;
		ulRetBufSize   = 0;
		dwBufSizeBytes = LOCAL_ALLOC_BUFSIZE_FOR(dwBufSize) * sizeof(WCHAR);
		DBG_ALIGNMENT_DISABLE();
		ntResult = (*pdyn_NtQueryObject)((HANDLE)hFile,
		                                 1 /* ObjectNameInformation*/,
		                                 lpBuffer, dwBufSizeBytes, &ulRetBufSize);
		DBG_ALIGNMENT_ENABLE();
		if (ntResult == 0)
			break;
		ntResult &= 0xffff;
		if (DeeNTSystem_IsIntr(ntResult)) {
			if (DeeThread_CheckInterrupt())
				goto err_lpBuffer;
			continue;
		}
		if (DeeNTSystem_IsBufferTooSmall(ntResult)) {
			if (!ulRetBufSize)
				ulRetBufSize = dwBufSize * 2;
			goto do_resize_buffer;
		}
		unicode_printer_free_wchar(printer, lpBuffer);
		if (ntResult == ERROR_INVALID_FUNCTION ||
		    ntResult == ERROR_INVALID_PARAMETER)
			return 2;
		DBG_ALIGNMENT_DISABLE();
		(void)SetLastError(ntResult);
		DBG_ALIGNMENT_ENABLE();
		return 1;
do_resize_buffer:
		lpNewBuffer = unicode_printer_resize_wchar(printer, lpBuffer, LOCAL_ALLOC_BUFSIZE_FOR(ulRetBufSize));
		if unlikely(!lpNewBuffer) {
err_lpBuffer:
			unicode_printer_free_wchar(printer, lpBuffer);
			goto err;
		}
	}

	/* Validate what's written in the returned buffer. */
	ntInfo = (NT_OBJECT_NAME_INFORMATION *)lpBuffer;
	if (dwBufSizeBytes <= offsetof(NT_OBJECT_NAME_INFORMATION, NameBuffer))
		goto err_unsupported;
	if (ntInfo->Name.Length > (dwBufSizeBytes - offsetof(NT_OBJECT_NAME_INFORMATION, NameBuffer)))
		goto err_unsupported;
	if ((void *)ntInfo->Name.Buffer < (void *)ntInfo->NameBuffer)
		goto err_unsupported;
	if (((byte_t *)ntInfo->Name.Buffer + ntInfo->Name.Length) >=
	    ((byte_t *)ntInfo + dwBufSizeBytes))
		goto err_unsupported;

	/* Move the actual NT name to where we need it. */
	dwBufSize = ntInfo->Name.Length / sizeof(WCHAR);
	memmovedownw((void *)ntInfo, (void *)ntInfo->Name.Buffer, dwBufSize);
	if unlikely(unicode_printer_commit_wchar(printer, lpBuffer, dwBufSize) < 0)
		goto err;
	return 0;
err_unsupported:
	unicode_printer_free_wchar(printer, lpBuffer);
	return 2;
err:
	return -1;
#undef LOCAL_ALLOC_BUFSIZE_FOR
}

/* @return: 2:  Unsupported.
 * @return: 1:  The system call failed (s.a. `GetLastError()').
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
PRIVATE WUNUSED int DCALL
DeeNTSystem_PrintMappedFileNameWrapper(struct unicode_printer *__restrict printer,
                                       HANDLE hFile) {
	/* GetMappedFileName(MapViewOfFile(CreateFileMapping(hFile))); */
	int result;
	HANDLE hFileMap;
	DWORD dwError;
	LPVOID lpFileMapAddr;
	DBG_ALIGNMENT_DISABLE();
	hFileMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hFileMap == NULL)
		goto os_err;
	lpFileMapAddr = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);
	if unlikely(lpFileMapAddr == NULL)
		goto os_err_hFileMap;
	result = DeeNTSystem_PrintMappedFileName(printer, GetCurrentProcess(), lpFileMapAddr);
	if (result == 1)
		goto os_err_hFileMap_lpFileMapAddr;
	(void)UnmapViewOfFile(lpFileMapAddr);
	(void)CloseHandle(hFileMap);
	return result;
os_err_hFileMap_lpFileMapAddr:
	dwError = GetLastError();
	(void)UnmapViewOfFile(lpFileMapAddr);
	(void)SetLastError(dwError);
os_err_hFileMap:
	dwError = GetLastError();
	(void)CloseHandle(hFileMap);
	(void)SetLastError(dwError);
os_err:
	DBG_ALIGNMENT_ENABLE();
	return 1;
}

PRIVATE WUNUSED LPWSTR DCALL DeeNTSystem_GetLogicalDriveStrings(void) {
	LPWSTR lpBuffer, lpNewBuffer;
	DWORD dwBufSize = PATH_MAX, dwError;
	lpBuffer        = (LPWSTR)Dee_Mallocc(dwBufSize, sizeof(WCHAR));
	if unlikely(!lpBuffer)
		goto err;
again:
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwError = GetLogicalDriveStringsW(dwBufSize + 1, lpBuffer);
		if (!dwError) {
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_lpBuffer;
				goto again;
			}
			if (dwError != NO_ERROR) {
				DeeNTSystem_ThrowErrorf(NULL, dwError, "Failed to query logical drive strings");
				goto err_lpBuffer;
			}
		} else {
			DBG_ALIGNMENT_ENABLE();
		}
		if (dwError <= dwBufSize)
			break;
		/* Resize to fit. */
		lpNewBuffer = (LPWSTR)Dee_Reallocc(lpBuffer, dwError, sizeof(WCHAR));
		if unlikely(!lpNewBuffer)
			goto err_lpBuffer;
		lpBuffer  = lpNewBuffer;
		dwBufSize = dwError;
	}

	/* Ensure that there are 2 trailing NUL-characters */
	{
		DWORD dwStrippedSize = dwError;
		while (dwStrippedSize && lpBuffer[dwStrippedSize - 1] == '\0')
			--dwStrippedSize;
		dwStrippedSize += 2;
		if unlikely(dwStrippedSize != dwError) {
			if unlikely(dwStrippedSize < dwError) {
				lpNewBuffer = (LPWSTR)Dee_TryReallocc(lpBuffer, dwStrippedSize, sizeof(WCHAR));
				if likely(lpNewBuffer)
					lpBuffer = lpNewBuffer;
			} else {
				lpNewBuffer = (LPWSTR)Dee_Reallocc(lpBuffer, dwStrippedSize, sizeof(WCHAR));
				if unlikely(!lpNewBuffer)
					goto err_lpBuffer;
				lpNewBuffer[dwStrippedSize - 2] = '\0';
				lpNewBuffer[dwStrippedSize - 1] = '\0';
				lpBuffer = lpNewBuffer;
			}
		}
	}
	return lpBuffer;
err_lpBuffer:
	Dee_Free(lpBuffer);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
unicode_printer_memcasecmp8(struct Dee_unicode_printer const *__restrict self,
                            uint8_t const *rhs, size_t lhs_start, size_t num_chars) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(lhs_start + num_chars >= lhs_start);
	ASSERT(lhs_start + num_chars <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0) || !num_chars);
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		return memcasecmp(str.cp8 + lhs_start, rhs, num_chars);

	CASE_WIDTH_2BYTE: {
		size_t i;
		str.cp16 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint16_t l = str.cp16[i];
			uint8_t r  = rhs[i];
			if (l != r) {
				l = (uint16_t)DeeUni_ToLower(l);
				r = (uint8_t)DeeUni_ToLower(r);
				if (l != r)
					return l < r ? -1 : 1;
			}
		}
	}	break;

	CASE_WIDTH_4BYTE: {
		size_t i;
		str.cp32 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint32_t l = str.cp32[i];
			uint8_t r  = rhs[i];
			if (l != r) {
				l = (uint32_t)DeeUni_ToLower(l);
				r = (uint8_t)DeeUni_ToLower(r);
				if (l != r)
					return l < r ? -1 : 1;
			}
		}
	}	break;

	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
unicode_printer_memcasecmp16(struct Dee_unicode_printer const *__restrict self,
                             uint16_t const *rhs, size_t lhs_start, size_t num_chars) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(lhs_start + num_chars >= lhs_start);
	ASSERT(lhs_start + num_chars <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0) || !num_chars);
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE: {
		size_t i;
		str.cp8 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint8_t l  = str.cp8[i];
			uint16_t r = rhs[i];
			if (l != r) {
				l = (uint8_t)DeeUni_ToLower(l);
				r = (uint16_t)DeeUni_ToLower(r);
				if (l != r)
					return l < r ? -1 : 1;
			}
		}
	}	break;

	CASE_WIDTH_2BYTE: {
		size_t i;
		str.cp16 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint16_t l = str.cp16[i];
			uint16_t r = rhs[i];
			if (l != r) {
				l = (uint16_t)DeeUni_ToLower(l);
				r = (uint16_t)DeeUni_ToLower(r);
				if (l != r)
					return l < r ? -1 : 1;
			}
		}
	}	break;

	CASE_WIDTH_4BYTE: {
		size_t i;
		str.cp32 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint32_t l = str.cp32[i];
			uint16_t r = rhs[i];
			if (l != r) {
				l = (uint32_t)DeeUni_ToLower(l);
				r = (uint16_t)DeeUni_ToLower(r);
				if (l != r)
					return l < r ? -1 : 1;
			}
		}
	}	break;

	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
unicode_printer_replace_substring16(struct unicode_printer *__restrict printer,
                                    size_t replace_off, size_t replace_len,
                                    uint16_t const *newstr, size_t newstr_len) {
	if unlikely(newstr_len > replace_len) {
		size_t copy_src = replace_off + replace_len;
		size_t copy_dst = replace_off + newstr_len;
		size_t missing  = newstr_len - replace_len;
		size_t copy_siz = UNICODE_PRINTER_LENGTH(printer) - copy_src;
		/* Just need to do something to the necessary extra space! */
		if unlikely(unicode_printer_print16(printer, newstr, missing) < 0)
			goto err;
		unicode_printer_memmove(printer, copy_dst, copy_src, copy_siz);
	} else if likely(newstr_len < replace_len) {
		size_t copy_src = replace_off + replace_len;
		size_t copy_siz = UNICODE_PRINTER_LENGTH(printer) - copy_src;
		size_t copy_dst = replace_off + newstr_len;
		unicode_printer_memmove(printer, copy_dst, copy_src, copy_siz);
		unicode_printer_truncate(printer, copy_dst + copy_siz);
	}
	unicode_printer_memcpy16(printer, newstr, replace_off, newstr_len);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_ConvertNtDrivePathToDosPath(struct unicode_printer *__restrict printer,
                                        size_t start_offset) {
	/* Handle r"\Device\HarddiskVolume{N}\"-like prefixes.
	 *
	 * For this purpose, enumerate `QueryDosDevice()' with strings
	 * returned by `GetLogicalDriveStringsW()' to find the drive
	 * that contains the given device:
	 *
	 * Program:
	 * >> import * from win32;
	 * >> for (local drive: GetLogicalDriveStrings()) {
	 * >>     local dosdev = QueryDosDevice(drive.rstrip("\\")).first;
	 * >>     print repr drive, repr dosdev;
	 * >> }
	 *
	 * Out:
	 * >> "C:\\" "\\Device\\HarddiskVolume3"
	 * >> "D:\\" "\\Device\\HarddiskVolume5"
	 * >> "E:\\" "\\Device\\HarddiskVolume6"
	 */
	LPWSTR wDriveStrings, wDriveString, wNextDriveString;
	LPWSTR wDeviceBuffer;
	DWORD dwDeviceBufLen;
	wDriveStrings = DeeNTSystem_GetLogicalDriveStrings();
	if unlikely(!wDriveStrings)
		goto err;
	dwDeviceBufLen = (DWORD)(UNICODE_PRINTER_LENGTH(printer) - start_offset) + 1;
	wDeviceBuffer  = (LPWSTR)Dee_Mallocac(dwDeviceBufLen, sizeof(WCHAR));
	if unlikely(!wDeviceBuffer)
		goto err_wDriveStrings;

	for (wDriveString = wDriveStrings; *wDriveString;
	     wDriveString = wNextDriveString) {
		int diff;
		DWORD dwDeviceLen;
		LPWSTR wDriveStringEnd;
		size_t wDriveStringLen;
		wDriveStringEnd  = wcsend(wDriveString);
		wNextDriveString = wDriveStringEnd + 1;

		/* Trim trailing '\'-characters, since `QueryDosDeviceW()' doesn't like those */
		while (wDriveStringEnd > wDriveString && wDriveStringEnd[-1] == (WCHAR)'\\')
			*--wDriveStringEnd = '\0';

		/* Query the device name attached to the drive. */
		DBG_ALIGNMENT_DISABLE();
		dwDeviceLen = QueryDosDeviceW(wDriveString, wDeviceBuffer, dwDeviceBufLen);
		DBG_ALIGNMENT_ENABLE();
		if unlikely(dwDeviceLen > dwDeviceBufLen)
			continue; /* Too long (can't fit) */

		/* Don't include trailing NUL characters in the compare below. */
		while (dwDeviceLen && !wDeviceBuffer[dwDeviceLen - 1])
			--dwDeviceLen;
		if (!dwDeviceLen)
			continue; /* It can't be this one. */

		diff = unicode_printer_memcasecmp16(printer, (uint16_t const *)wDeviceBuffer,
		                                    start_offset, dwDeviceLen);
		if (diff != 0)
			continue;

		/* Got it!
		 *
		 * Now we must replace the text in `printer':
		 * >> printer[start_offset:start_offset+dwDeviceLen] = wDriveString...wDriveStringEnd */
		wDriveStringLen = (size_t)(wDriveStringEnd - wDriveString);
		if unlikely(unicode_printer_replace_substring16(printer, start_offset, dwDeviceLen,
		                                                (uint16_t const *)wDriveString, wDriveStringLen))
			goto err_wDriveStrings_wDeviceBuffer;
		goto done;
	}

	/* Unsupported prefix :(
	 *
	 * -> Return as-is, even though that'll probably lead to later problems,
	 *    but at least doing so allows us to easily see unsupported patterns
	 *    and add support for them later */
done:
	Dee_Freea(wDeviceBuffer);
	Dee_Free(wDriveStrings);
	return 0;
err_wDriveStrings_wDeviceBuffer:
	Dee_Freea(wDeviceBuffer);
err_wDriveStrings:
	Dee_Free(wDriveStrings);
err:
	return -1;
}


/* @return: NULL:      An error was thrown
 * @return: ITER_DONE: System error (ignore) */
PRIVATE WUNUSED NONNULL((1, 2)) LPWSTR DCALL
DeeNTSystem_RegQueryValueExW(HKEY hKey, LPCWSTR lpValueName) {
	DWORD dwType;
	LPWSTR wResult, wNewResult;
	DWORD dwResultLen;
	DWORD dwResultReqLen;
	LSTATUS lStatus;

	dwResultLen = 64 * sizeof(WCHAR);
	wResult     = (LPWSTR)Dee_Malloc(dwResultLen);
	if unlikely(!wResult)
		goto err;
again:
	dwResultReqLen = dwResultLen;
	DBG_ALIGNMENT_DISABLE();
	lStatus = RegQueryValueExW(hKey, lpValueName, NULL, &dwType,
	                           (LPBYTE)wResult, &dwResultReqLen);
	DBG_ALIGNMENT_ENABLE();
	if (lStatus == ERROR_MORE_DATA) {
		if (dwResultReqLen <= dwResultLen)
			dwResultReqLen = dwResultLen * 2;
	} else if (lStatus != 0) {
		goto err_r_unsupported;
	}

	/* Make sure it's a string. */
	if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
		goto err_r_unsupported;

	/* Shouldn't happen: misaligned buffer size. */
	if unlikely(dwResultReqLen & 1)
		++dwResultReqLen;

	/* Try to allocate to the requested buffer size. */
	wNewResult = (LPWSTR)Dee_TryRealloc(wResult, dwResultReqLen);
	if likely(wNewResult) {
		wResult = wNewResult;
		if (lStatus == ERROR_MORE_DATA) {
			dwResultLen = dwResultReqLen;
			goto again;
		}
	} else if (lStatus == ERROR_MORE_DATA) {
		/* Deal with case where we *need* more space. */
		wNewResult = (LPWSTR)Dee_Realloc(wResult, dwResultReqLen);
		if unlikely(!wNewResult)
			goto err_r;
		wResult     = wNewResult;
		dwResultLen = dwResultReqLen;
		goto again;
	}
	return wNewResult;
err_r_unsupported:
	Dee_Free(wResult);
	return (LPWSTR)ITER_DONE;
err_r:
	Dee_Free(wResult);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_ConvertNtComPathToDosPath(struct unicode_printer *__restrict printer,
                                      size_t start_offset) {
	union dcharptr str;
	static WCHAR const wstr_Hardware_DeviceMap_SerialComm[] = {
		'H', 'a', 'r', 'd', 'w', 'a', 'r', 'e', '\\',
		'D', 'e', 'v', 'i', 'c', 'e', 'M', 'a', 'p', '\\',
		'S', 'e', 'r', 'i', 'a', 'l', 'C', 'o', 'm', 'm', 0
	};
	int error;
	HKEY hKey;
	LPWSTR wKeyValue;
	size_t wKeyValueLen;
	LSTATUS lStatus;
	DBG_ALIGNMENT_DISABLE();
	lStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE, wstr_Hardware_DeviceMap_SerialComm,
	                        0, KEY_QUERY_VALUE, &hKey);
	DBG_ALIGNMENT_ENABLE();
	if (lStatus != 0)
		goto done;
	str.ptr = printer->up_buffer;
	SWITCH_SIZEOF_WIDTH(printer->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_2BYTE:
		wKeyValue = DeeNTSystem_RegQueryValueExW(hKey, (LPCWSTR)(str.cp16 + start_offset));
		break;

	CASE_WIDTH_1BYTE:
	CASE_WIDTH_4BYTE: {
		LPWSTR wStr;
		size_t i, wStrLen;
		wStrLen = UNICODE_PRINTER_LENGTH(printer) - start_offset;
		wStr    = (LPWSTR)Dee_Mallocac(wStrLen + 1, sizeof(WCHAR));
		if unlikely(!wStr)
			goto err;
		for (i = 0; i < wStrLen; ++i) {
			wStr[i] = (WCHAR)UNICODE_PRINTER_GETCHAR(printer, start_offset + i);
		}
		wStr[wStrLen] = (WCHAR)'\0';
		wKeyValue = DeeNTSystem_RegQueryValueExW(hKey, wStr);
		Dee_Freea(wStr);
	}	break;

	}
	(void)RegCloseKey(hKey);
	if (wKeyValue == (LPWSTR)ITER_DONE)
		goto done;
	if unlikely(!wKeyValue)
		goto err;

	/* Replace the filename with the COM name. */
	wKeyValueLen = wcslen(wKeyValue);
	error = unicode_printer_replace_substring16(printer, start_offset,
	                                            UNICODE_PRINTER_LENGTH(printer) - start_offset,
	                                            (uint16_t const *)wKeyValue, wKeyValueLen);
	Dee_Free(wKeyValue);
	if unlikely(error)
		goto err;
done:
	return 0;
err:
	return -1;
}

/* Convert "\Device\..." path names to their DOS equivalent
 * @param: start_offset: The starting offset of the path in `printer'
 * @return: 0:  Success.
 * @return: -1: An error was thrown. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_ConvertNtPathToDosPath(struct unicode_printer *__restrict printer,
                                   size_t start_offset) {
	size_t end_offset = UNICODE_PRINTER_LENGTH(printer);
	/* NOTE: Most of the special handling here is derived from:
	 *       https://stackoverflow.com/a/18792477/3296587 */

	if (end_offset >= start_offset + 8 &&
	    unicode_printer_memcasecmp8(printer, (uint8_t const *)"\\Device\\", start_offset, 8) == 0) {

		/* Check for r"\Device\Mup\<SERVER>\<SHARE>"-like prefixes. */
		if (end_offset >= start_offset + 12 &&
		    unicode_printer_memcasecmp8(printer, (uint8_t const *)"Mup\\", start_offset + 8, 4) == 0) {
			++start_offset; /* Keep the first r'\' */
			unicode_printer_memmove(printer, start_offset, start_offset + 10,
			                        (end_offset - start_offset) - 10);
			unicode_printer_truncate(printer, end_offset - 10);
			return 0;
		}

		/* Check for r"\Device\LanmanRedirector\<SERVER>\<SHARE>"-like prefixes. */
		if (end_offset >= start_offset + 26 &&
		    unicode_printer_memcasecmp8(printer, (uint8_t const *)"LanmanRedirector\\", start_offset + 8, 16) == 0) {
			++start_offset; /* Keep the first r'\' */
			unicode_printer_memmove(printer, start_offset, start_offset + 24,
			                        (end_offset - start_offset) - 24);
			unicode_printer_truncate(printer, end_offset - 24);
			return 0;
		}

		/* Check for r"\Device\Null" */
		if (end_offset == start_offset + 12 &&
		    unicode_printer_memcasecmp8(printer, (uint8_t const *)"Null", start_offset + 8, 4) == 0) {
			UNICODE_PRINTER_SETCHAR(printer, start_offset + 0, 'N');
			UNICODE_PRINTER_SETCHAR(printer, start_offset + 1, 'U');
			UNICODE_PRINTER_SETCHAR(printer, start_offset + 2, 'L');
			unicode_printer_truncate(printer, start_offset + 3);
			return 0;
		}

		/* Check for r"\Device\ConDrv" */
		if (end_offset == start_offset + 14 &&
		    unicode_printer_memcasecmp8(printer, (uint8_t const *)"ConDrv", start_offset + 8, 6) == 0) {
			UNICODE_PRINTER_SETCHAR(printer, start_offset + 0, 'C');
			UNICODE_PRINTER_SETCHAR(printer, start_offset + 1, 'O');
			UNICODE_PRINTER_SETCHAR(printer, start_offset + 2, 'N');
			unicode_printer_truncate(printer, start_offset + 3);
			return 0;
		}

		/* Check for r"\Device\Serial[...]" and r"\Device\UsbSer[...]",
		 * which are files that can be mapped to `COM1', `COM2', etc... */
		if (end_offset >= start_offset + 14 &&
		    (unicode_printer_memcasecmp8(printer, (uint8_t const *)"Serial", start_offset + 8, 6) == 0 ||
		     unicode_printer_memcasecmp8(printer, (uint8_t const *)"UsbSer", start_offset + 8, 6) == 0))
			return DeeNTSystem_ConvertNtComPathToDosPath(printer, start_offset);
	}
	return DeeNTSystem_ConvertNtDrivePathToDosPath(printer, start_offset);
}

/* @return: 1:  The system call failed (s.a. `GetLastError()').
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_PrintFilenameOfHandle(struct unicode_printer *__restrict printer,
                                  /*HANDLE*/ void *hFile) {
	int error;
	size_t length;
	length = UNICODE_PRINTER_LENGTH(printer);
#if 1
	error  = DeeNTSystem_PrintFinalPathNameByHandle(printer, hFile, 0);
#else
	error  = 2;
#endif
	if (error == 0) {
		size_t new_length;
		/* Try to get rid of the \\?\ prefix */
		new_length = UNICODE_PRINTER_LENGTH(printer);
		if (new_length >= length + 6 &&
		    UNICODE_PRINTER_GETCHAR(printer, length + 0) == '\\' &&
		    UNICODE_PRINTER_GETCHAR(printer, length + 1) == '\\' &&
		    UNICODE_PRINTER_GETCHAR(printer, length + 2) == '?' &&
		    UNICODE_PRINTER_GETCHAR(printer, length + 3) == '\\') {
			if (DeeUni_IsAlpha(UNICODE_PRINTER_GETCHAR(printer, length + 4))) {
				size_t drive_end = length + 5;
				for (;;) {
					uint32_t ch;
					ch = UNICODE_PRINTER_GETCHAR(printer, drive_end);
					if (ch == ':')
						break;
					if (!DeeUni_IsAlpha(ch))
						goto not_a_drive_prefix;
					++drive_end;
				}
				/* This is a r"\\?\<DRIVE_LETTER(S)>:"-like prefix */
				unicode_printer_memmove(printer, length, length + 4,
				                        (new_length - length) - 4);
				unicode_printer_truncate(printer, new_length - 4);
				return 0;
			}
not_a_drive_prefix:
			/* Check for r"\\?\UNC\<SERVER>\<SHARE>"-like prefixes.
			 * These then have to be converted into r"\\<SERVER>\<SHARE>",
			 * so we can simply delete the r"?\UNC\" portion. */
			if (UNICODE_PRINTER_GETCHAR(printer, length + 4) == 'U' &&
			    UNICODE_PRINTER_GETCHAR(printer, length + 5) == 'N' &&
			    UNICODE_PRINTER_GETCHAR(printer, length + 6) == 'C' &&
			    UNICODE_PRINTER_GETCHAR(printer, length + 7) == '\\') {
				unicode_printer_memmove(printer, length + 2, length + 8,
				                        (new_length - length) - 6);
				unicode_printer_truncate(printer, new_length - 6);
				return 0;
			}
		}
		return 0;
	}
	if (error != 2)
		return error; /* Error (-1) or System error (1) */

	/* Try to use `NtQueryObject(ObjectNameInformation)' */
	error = DeeNTSystem_PrintNtQueryObject_ObjectNameInformation(printer, (HANDLE)hFile);
	if (error == 0)
		return DeeNTSystem_ConvertNtPathToDosPath(printer, length);
	if (error != 2)
		return error; /* Error (-1) or System error (1) */

	/* Try to use `GetMappedFileName(MapViewOfFile(CreateFileMapping(hFile)))' */
	error = DeeNTSystem_PrintMappedFileNameWrapper(printer, (HANDLE)hFile);
	if (error == 0)
		return DeeNTSystem_ConvertNtPathToDosPath(printer, length);
	if (error != 2)
		return error; /* Error (-1) or System error (1) */

	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "No way to print the filename of handle %p",
	                       hFile);
}


typedef DWORD (WINAPI *LPGETMAPPEDFILENAMEW)(HANDLE hProcess, LPVOID lpv,
                                             LPWSTR lpFilename, DWORD nSize);
PRIVATE LPGETMAPPEDFILENAMEW pdyn_GetMappedFileNameW = NULL;
PRIVATE WCHAR const wPsapi[]    = { 'P', 'S', 'A', 'P', 'I', 0 };
PRIVATE WCHAR const wPsapiDll[] = { 'P', 's', 'a', 'p', 'i', '.', 'd', 'l', 'l', 0 };
PRIVATE char const name_GetMappedFileNameW[] = "GetMappedFileNameW";

/* Wrapper for the `GetMappedFileName()' system call.
 * @return: 2:  Unsupported.
 * @return: 1:  The system call failed (s.a. `GetLastError()').
 * @return: 0:  Success.
 * @return: -1: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_PrintMappedFileName(struct Dee_unicode_printer *__restrict printer,
                                /*HANDLE*/ void *hProcess,
                                /*LPVOID*/ void *lpv) {
	LPWSTR lpNewBuffer, lpBuffer;
	DWORD dwNewBufSize, dwBufSize;
	if (pdyn_GetMappedFileNameW == NULL) {
		LPGETMAPPEDFILENAMEW lpGetMappedFileNameW = NULL;
		DBG_ALIGNMENT_DISABLE();
		lpGetMappedFileNameW = (LPGETMAPPEDFILENAMEW)GetProcAddress(GetModuleHandleW(wPsapi),
		                                                            name_GetMappedFileNameW);
		if (!lpGetMappedFileNameW) {
			lpGetMappedFileNameW = (LPGETMAPPEDFILENAMEW)GetProcAddress(GetModuleHandleW(wKernel32),
			                                                            name_GetMappedFileNameW);
		}
		if (!lpGetMappedFileNameW) {
			HMODULE hModule = LoadLibraryW(wPsapiDll);
			if (hModule) {
				lpGetMappedFileNameW = (LPGETMAPPEDFILENAMEW)GetProcAddress(hModule, name_GetMappedFileNameW);
				if (!lpGetMappedFileNameW)
					(void)FreeLibrary(hModule);
			}
		}
		if (!lpGetMappedFileNameW) {
			HMODULE hModule = LoadLibraryW(wKernel32);
			if (hModule) {
				lpGetMappedFileNameW = (LPGETMAPPEDFILENAMEW)GetProcAddress(hModule, name_GetMappedFileNameW);
				if (!lpGetMappedFileNameW)
					(void)FreeLibrary(hModule);
			}
		}
		DBG_ALIGNMENT_ENABLE();
		if (!lpGetMappedFileNameW)
			*(void **)&lpGetMappedFileNameW = (void *)(uintptr_t)-1;
		atomic_write(&pdyn_GetMappedFileNameW, lpGetMappedFileNameW);
	}
	if (*(void **)&pdyn_GetMappedFileNameW == (void *)(uintptr_t)-1)
		return 2; /* Unsupported. */
	/* Make use of `GetMappedFileNameW()' */
	dwBufSize = PATH_MAX;
	lpBuffer = unicode_printer_alloc_wchar(printer, dwBufSize);
	if unlikely(!lpBuffer)
		goto err;
	for (;;) {
		DBG_ALIGNMENT_DISABLE();
		dwNewBufSize = (*pdyn_GetMappedFileNameW)((HANDLE)hProcess,
		                                          (LPVOID)lpv,
		                                          lpBuffer,
		                                          dwBufSize);
		if unlikely(!dwNewBufSize) {
			DWORD dwError;
			dwError = GetLastError();
			DBG_ALIGNMENT_ENABLE();
			if (DeeNTSystem_IsIntr(dwError)) {
				if (DeeThread_CheckInterrupt())
					goto err_lpBuffer;
				continue;
			}
			if (DeeNTSystem_IsBufferTooSmall(dwError)) {
				dwNewBufSize = dwBufSize * 2;
				goto do_resize_buffer;
			}
			unicode_printer_free_wchar(printer, lpBuffer);
			(void)SetLastError(dwError);
			return 1;
		}
		DBG_ALIGNMENT_ENABLE();
		if (dwNewBufSize <= dwBufSize)
			break;
do_resize_buffer:
		lpNewBuffer = unicode_printer_resize_wchar(printer, lpBuffer, dwNewBufSize);
		if unlikely(!lpNewBuffer) {
err_lpBuffer:
			unicode_printer_free_wchar(printer, lpBuffer);
			goto err;
		}
		dwBufSize = dwNewBufSize;
	}
	if unlikely(unicode_printer_commit_wchar(printer, lpBuffer, dwNewBufSize) < 0)
		goto err;
	return 0;
err:
	return -1;
}





/* Wrapper for the `FormatMessageW()' system call.
 * @return: * :        The formatted message.
 * @return: NULL:      A deemon callback failed and an error was thrown.
 * @return: ITER_DONE: The system call failed (s.a. `GetLastError()'). */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FormatMessage(DeeNT_DWORD dwFlags, void const *lpSource,
                          DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                          /* va_list * */ void *Arguments) {
	int error;
	DWORD dwLastError;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	error = DeeNTSystem_UPrintFormatMessage(&printer, dwFlags, lpSource,
	                                        dwMessageId, dwLanguageId, Arguments);
	if (error == 0)
		return unicode_printer_pack(&printer);
	/* Preserve LastError during `unicode_printer_fini()' */
	DBG_ALIGNMENT_DISABLE();
	dwLastError = GetLastError();
	DBG_ALIGNMENT_ENABLE();
	unicode_printer_fini(&printer);
	DBG_ALIGNMENT_DISABLE();
	(void)SetLastError(dwLastError);
	DBG_ALIGNMENT_ENABLE();
	if (error < 0)
		goto err;
	return ITER_DONE;
err:
	return NULL;
}

/* @return: 1:  The system call failed (nothing was printed; s.a. `GetLastError()')
 * @return: 0:  Successfully printed the message.
 * @return: -1: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED NONNULL((1)) int DCALL
DeeNTSystem_UPrintFormatMessage(struct unicode_printer *__restrict printer,
                                DeeNT_DWORD dwFlags, void const *lpSource,
                                DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                                /* va_list * */ void *Arguments) {
	LPWSTR buffer, newBuffer;
	DWORD dwNewBufsize, dwBufSize = 128;
	buffer = unicode_printer_alloc_wchar(printer, dwBufSize);
	if unlikely(!buffer)
		goto err;
again:
	DBG_ALIGNMENT_DISABLE();
	dwNewBufsize = FormatMessageW(dwFlags,
	                              lpSource,
	                              dwMessageId,
	                              dwLanguageId,
	                              buffer,
	                              dwBufSize,
	                              (va_list *)Arguments);
	if unlikely(!dwNewBufsize) {
		DWORD dwError;
		dwError = GetLastError();
		DBG_ALIGNMENT_ENABLE();
		if (DeeNTSystem_IsBadAllocError(dwError)) {
			if (Dee_ReleaseSystemMemory())
				goto again;
		} else {
			/* MSDN says that this string cannot exceed 32*1024
			 * >> So if it does, treat it as a failure of translating the message... */
			if (dwBufSize > 32 * 1024) {
				unicode_printer_free_wchar(printer, buffer);
				return 1;
			}
			dwBufSize *= 2;
			newBuffer = unicode_printer_resize_wchar(printer, buffer, dwBufSize);
			if unlikely(!newBuffer)
				goto err_release;
			buffer = newBuffer;
			goto again;
		}
		goto err_release;
	}
	DBG_ALIGNMENT_ENABLE();
	if (dwNewBufsize > dwBufSize) {
		LPWSTR new_buffer;
		/* Increase the buffer and try again. */
		new_buffer = unicode_printer_resize_wchar(printer, buffer, dwNewBufsize);
		if unlikely(!new_buffer)
			goto err_release;
		dwBufSize = dwNewBufsize;
		goto again;
	}
	/* Trim all trailing space characters */
	while (dwNewBufsize) {
		if (!DeeUni_IsSpace(buffer[dwNewBufsize - 1]))
			break;
		--dwNewBufsize;
	}
	if unlikely(unicode_printer_commit_wchar(printer, buffer, dwNewBufsize) < 0)
		goto err;
	return 0;
err_release:
	unicode_printer_free_wchar(printer, buffer);
err:
	return -1;
}

/* @param: p_success: Set to `false' if the system call failed and nothing was printed (s.a. `GetLastError()')
 * @return: >= 0: Success.
 * @return: < 0:  A deemon callback failed and an error was thrown (`*p_success' is undefined). */
PUBLIC WUNUSED NONNULL((1, 8)) Dee_ssize_t DCALL
DeeNTSystem_PrintFormatMessage(Dee_formatprinter_t printer, void *arg,
                               DeeNT_DWORD dwFlags, void const *lpSource,
                               DeeNT_DWORD dwMessageId, DeeNT_DWORD dwLanguageId,
                               /* va_list * */ void *Arguments,
                               bool *__restrict p_success) {
	if (printer == &unicode_printer_print) {
		/* Special case: fast-forward to the underlying unicode printer. */
		struct unicode_printer *upn = (struct unicode_printer *)arg;
		size_t oldlen;
		int error;
		oldlen = UNICODE_PRINTER_LENGTH(upn);
		error  = DeeNTSystem_UPrintFormatMessage(upn, dwFlags, lpSource,
		                                         dwMessageId, dwLanguageId, Arguments);
		if unlikely(error < 0)
			return -1;
		*p_success = error == 0;
		return (Dee_ssize_t)(UNICODE_PRINTER_LENGTH(upn) - oldlen);
	} else {
		Dee_ssize_t result;
		struct unicode_printer uprinter = UNICODE_PRINTER_INIT;
		int error;
		error = DeeNTSystem_UPrintFormatMessage(&uprinter, dwFlags, lpSource,
		                                        dwMessageId, dwLanguageId, Arguments);
		if unlikely(error < 0) {
			result = -1;
		} else {
			*p_success = error == 0;
			result = unicode_printer_printinto(&uprinter, printer, arg);
		}
		unicode_printer_fini(&uprinter);
		return result;
	}
}


/* Convenience wrapper around `DeeNTSystem_FormatMessage()' for getting error messages.
 * When no error message exists, return an empty string.
 * @return: * :   The error message. (or an empty string)
 * @return: NULL: A deemon callback failed and an error was thrown. */
PUBLIC WUNUSED DREF /*String*/ DeeObject *DCALL
DeeNTSystem_FormatErrorMessage(DeeNT_DWORD dwError) {
	DREF /*String*/ DeeObject *result;
	result = DeeNTSystem_FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
	                                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                                   NULL);
	if (result == ITER_DONE)
		result = DeeString_NewEmpty();
	return result;
}


DECL_END

#endif /* CONFIG_HOST_WINDOWS */

#endif /* !GUARD_DEEMON_SYSTEM_SYSTEM_NT_C_INL */
