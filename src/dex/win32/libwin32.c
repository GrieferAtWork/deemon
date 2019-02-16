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
#ifndef GUARD_DEX_WIN32_LIBWIN32_C
#define GUARD_DEX_WIN32_LIBWIN32_C 1
#define CONFIG_BUILDING_LIBWIN32 1
#define DEE_SOURCE 1

#include "libwin32.h"
#if defined(CONFIG_HOST_WINDOWS) || defined(__DEEMON__)

#include <deemon/arg.h>
#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/bool.h>
#include <deemon/none.h>
#include <deemon/objmethod.h>
#include <stdio.h>

DECL_BEGIN

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
#endif

#ifndef DRIVE_UNKNOWN
#define DRIVE_UNKNOWN       0
#endif
#ifndef DRIVE_NO_ROOT_DIR
#define DRIVE_NO_ROOT_DIR   1
#endif
#ifndef DRIVE_REMOVABLE
#define DRIVE_REMOVABLE     2
#endif
#ifndef DRIVE_FIXED
#define DRIVE_FIXED         3
#endif
#ifndef DRIVE_REMOTE
#define DRIVE_REMOTE        4
#endif
#ifndef DRIVE_CDROM
#define DRIVE_CDROM         5
#endif
#ifndef DRIVE_RAMDISK
#define DRIVE_RAMDISK       6
#endif


/*[[[deemon
import * from _dexutils;
include("constants.def");
gii("FILE_READ_DATA");
gii("FILE_LIST_DIRECTORY");
gii("FILE_WRITE_DATA");
gii("FILE_ADD_FILE");
gii("FILE_APPEND_DATA");
gii("FILE_ADD_SUBDIRECTORY");
gii("FILE_CREATE_PIPE_INSTANCE");
gii("FILE_READ_EA");
gii("FILE_WRITE_EA");
gii("FILE_EXECUTE");
gii("FILE_TRAVERSE");
gii("FILE_DELETE_CHILD");
gii("FILE_READ_ATTRIBUTES");
gii("FILE_WRITE_ATTRIBUTES");
gii("FILE_ALL_ACCESS");
gii("FILE_GENERIC_READ");
gii("FILE_GENERIC_WRITE");
gii("FILE_GENERIC_EXECUTE");
gii("FILE_SHARE_READ");
gii("FILE_SHARE_WRITE");
gii("FILE_SHARE_DELETE");
gii("FILE_ATTRIBUTE_READONLY");
gii("FILE_ATTRIBUTE_HIDDEN");
gii("FILE_ATTRIBUTE_SYSTEM");
gii("FILE_ATTRIBUTE_DIRECTORY");
gii("FILE_ATTRIBUTE_ARCHIVE");
gii("FILE_ATTRIBUTE_DEVICE");
gii("FILE_ATTRIBUTE_NORMAL");
gii("FILE_ATTRIBUTE_TEMPORARY");
gii("FILE_ATTRIBUTE_SPARSE_FILE");
gii("FILE_ATTRIBUTE_REPARSE_POINT");
gii("FILE_ATTRIBUTE_COMPRESSED");
gii("FILE_ATTRIBUTE_OFFLINE");
gii("FILE_ATTRIBUTE_NOT_CONTENT_INDEXED");
gii("FILE_ATTRIBUTE_ENCRYPTED");
gii("FILE_ATTRIBUTE_INTEGRITY_STREAM");
gii("FILE_ATTRIBUTE_VIRTUAL");
gii("FILE_ATTRIBUTE_NO_SCRUB_DATA");
gii("FILE_ATTRIBUTE_EA");
gii("FILE_NOTIFY_CHANGE_FILE_NAME");
gii("FILE_NOTIFY_CHANGE_DIR_NAME");
gii("FILE_NOTIFY_CHANGE_ATTRIBUTES");
gii("FILE_NOTIFY_CHANGE_SIZE");
gii("FILE_NOTIFY_CHANGE_LAST_WRITE");
gii("FILE_NOTIFY_CHANGE_LAST_ACCESS");
gii("FILE_NOTIFY_CHANGE_CREATION");
gii("FILE_NOTIFY_CHANGE_SECURITY");
gii("FILE_ACTION_ADDED");
gii("FILE_ACTION_REMOVED");
gii("FILE_ACTION_MODIFIED");
gii("FILE_ACTION_RENAMED_OLD_NAME");
gii("FILE_ACTION_RENAMED_NEW_NAME");
gii("FILE_CASE_SENSITIVE_SEARCH");
gii("FILE_CASE_PRESERVED_NAMES");
gii("FILE_UNICODE_ON_DISK");
gii("FILE_PERSISTENT_ACLS");
gii("FILE_FILE_COMPRESSION");
gii("FILE_VOLUME_QUOTAS");
gii("FILE_SUPPORTS_SPARSE_FILES");
gii("FILE_SUPPORTS_REPARSE_POINTS");
gii("FILE_SUPPORTS_REMOTE_STORAGE");
gii("FILE_VOLUME_IS_COMPRESSED");
gii("FILE_SUPPORTS_OBJECT_IDS");
gii("FILE_SUPPORTS_ENCRYPTION");
gii("FILE_NAMED_STREAMS");
gii("FILE_READ_ONLY_VOLUME");
gii("FILE_SEQUENTIAL_WRITE_ONCE");
gii("FILE_SUPPORTS_TRANSACTIONS");
gii("FILE_SUPPORTS_HARD_LINKS");
gii("FILE_SUPPORTS_EXTENDED_ATTRIBUTES");
gii("FILE_SUPPORTS_OPEN_BY_FILE_ID");
gii("FILE_SUPPORTS_USN_JOURNAL");
gii("FILE_SUPPORTS_INTEGRITY_STREAMS");
gii("CREATE_NEW");
gii("CREATE_ALWAYS");
gii("OPEN_EXISTING");
gii("OPEN_ALWAYS");
gii("TRUNCATE_EXISTING");
gii("FILE_BEGIN");
gii("FILE_CURRENT");
gii("FILE_END");

gii("FILE_TYPE_UNKNOWN");
gii("FILE_TYPE_DISK");
gii("FILE_TYPE_CHAR");
gii("FILE_TYPE_PIPE");
gii("FILE_TYPE_REMOTE");

gii("DRIVE_UNKNOWN");
gii("DRIVE_NO_ROOT_DIR");
gii("DRIVE_REMOVABLE");
gii("DRIVE_FIXED");
gii("DRIVE_REMOTE");
gii("DRIVE_CDROM");
gii("DRIVE_RAMDISK");

]]]*/
#include "constants.def"
//[[[end]]]






/*[[[deemon import("_dexutils").gw("GetLastError","->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetLastError_f_impl(void);
PRIVATE DREF DeeObject *DCALL libwin32_GetLastError_f(size_t argc, DeeObject **__restrict argv);
#define LIBWIN32_GETLASTERROR_DEF { "GetLastError", (DeeObject *)&libwin32_GetLastError, MODSYM_FNORMAL, DOC("()->?Dint") },
#define LIBWIN32_GETLASTERROR_DEF_DOC(doc) { "GetLastError", (DeeObject *)&libwin32_GetLastError, MODSYM_FNORMAL, DOC("()->?Dint\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetLastError,libwin32_GetLastError_f);
PRIVATE DREF DeeObject *DCALL libwin32_GetLastError_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":GetLastError"))
	    goto err;
	return libwin32_GetLastError_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetLastError_f_impl(void)
//[[[end]]]
{
 return DeeInt_NewU32((uint32_t)GetLastError());
}

/*[[[deemon import("_dexutils").gw("SetLastError","dwErrCode:nt:DWORD"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_SetLastError_f_impl(DWORD dwErrCode);
PRIVATE DREF DeeObject *DCALL libwin32_SetLastError_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_SETLASTERROR_DEF { "SetLastError", (DeeObject *)&libwin32_SetLastError, MODSYM_FNORMAL, DOC("(dwErrCode:?Dint)") },
#define LIBWIN32_SETLASTERROR_DEF_DOC(doc) { "SetLastError", (DeeObject *)&libwin32_SetLastError, MODSYM_FNORMAL, DOC("(dwErrCode:?Dint)\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetLastError,libwin32_SetLastError_f);
#ifndef LIBWIN32_KWDS_DWERRCODE_DEFINED
#define LIBWIN32_KWDS_DWERRCODE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_dwErrCode,{ K(dwErrCode), KEND });
#endif /* !LIBWIN32_KWDS_DWERRCODE_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_SetLastError_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	DWORD dwErrCode;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_dwErrCode,"I32u:SetLastError",&dwErrCode))
	    goto err;
	return libwin32_SetLastError_f_impl(dwErrCode);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_SetLastError_f_impl(DWORD dwErrCode)
//[[[end]]]
{
 SetLastError(dwErrCode);
 return_none;
}

/*[[[deemon import("_dexutils").gw("CloseHandle","hObject:nt:HANDLE->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_CloseHandle_f_impl(HANDLE hObject);
PRIVATE DREF DeeObject *DCALL libwin32_CloseHandle_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_CLOSEHANDLE_DEF { "CloseHandle", (DeeObject *)&libwin32_CloseHandle, MODSYM_FNORMAL, DOC("(hObject:?Dint)->?Dbool") },
#define LIBWIN32_CLOSEHANDLE_DEF_DOC(doc) { "CloseHandle", (DeeObject *)&libwin32_CloseHandle, MODSYM_FNORMAL, DOC("(hObject:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CloseHandle,libwin32_CloseHandle_f);
#ifndef LIBWIN32_KWDS_HOBJECT_DEFINED
#define LIBWIN32_KWDS_HOBJECT_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hObject,{ K(hObject), KEND });
#endif /* !LIBWIN32_KWDS_HOBJECT_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_CloseHandle_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hObject;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hObject,"Iu:CloseHandle",&hObject))
	    goto err;
	return libwin32_CloseHandle_f_impl(hObject);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_CloseHandle_f_impl(HANDLE hObject)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = CloseHandle(hObject);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED)
     goto again;
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("DuplicateHandle",
      "hSourceProcessHandle:nt:HANDLE"
     ",hSourceHandle:nt:HANDLE"
     ",hTargetProcessHandle:nt:HANDLE"
     ",dwDesiredAccess:nt:DWORD=0"
     ",bInheritHandle:c:bool=true"
     ",dwOptions:nt:DWORD=DUPLICATE_SAME_ACCESS"
     "->?X2?Dint?N"
     ); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_DuplicateHandle_f_impl(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwOptions);
PRIVATE DREF DeeObject *DCALL libwin32_DuplicateHandle_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_DUPLICATEHANDLE_DEF { "DuplicateHandle", (DeeObject *)&libwin32_DuplicateHandle, MODSYM_FNORMAL, DOC("(hSourceProcessHandle:?Dint,hSourceHandle:?Dint,hTargetProcessHandle:?Dint,dwDesiredAccess:?Dint=0,bInheritHandle:?Dbool=!t,dwOptions:?Dint=DUPLICATE_SAME_ACCESS)->?X2?Dint?N") },
#define LIBWIN32_DUPLICATEHANDLE_DEF_DOC(doc) { "DuplicateHandle", (DeeObject *)&libwin32_DuplicateHandle, MODSYM_FNORMAL, DOC("(hSourceProcessHandle:?Dint,hSourceHandle:?Dint,hTargetProcessHandle:?Dint,dwDesiredAccess:?Dint=0,bInheritHandle:?Dbool=!t,dwOptions:?Dint=DUPLICATE_SAME_ACCESS)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_DuplicateHandle,libwin32_DuplicateHandle_f);
#ifndef LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED
#define LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions,{ K(hSourceProcessHandle), K(hSourceHandle), K(hTargetProcessHandle), K(dwDesiredAccess), K(bInheritHandle), K(dwOptions), KEND });
#endif /* !LIBWIN32_KWDS_HSOURCEPROCESSHANDLE_HSOURCEHANDLE_HTARGETPROCESSHANDLE_DWDESIREDACCESS_BINHERITHANDLE_DWOPTIONS_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_DuplicateHandle_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hSourceProcessHandle;
	HANDLE hSourceHandle;
	HANDLE hTargetProcessHandle;
	DWORD dwDesiredAccess = 0;
	bool bInheritHandle = true;
	DWORD dwOptions = DUPLICATE_SAME_ACCESS;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hSourceProcessHandle_hSourceHandle_hTargetProcessHandle_dwDesiredAccess_bInheritHandle_dwOptions,"IuIuIu|I32ubI32u:DuplicateHandle",&hSourceProcessHandle,&hSourceHandle,&hTargetProcessHandle,&dwDesiredAccess,&bInheritHandle,&dwOptions))
	    goto err;
	return libwin32_DuplicateHandle_f_impl(hSourceProcessHandle,hSourceHandle,hTargetProcessHandle,dwDesiredAccess,bInheritHandle,dwOptions);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_DuplicateHandle_f_impl(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, DWORD dwDesiredAccess, bool bInheritHandle, DWORD dwOptions)
//[[[end]]]
{
 HANDLE hResult;
 DREF DeeObject *result;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 if (!DuplicateHandle(hSourceProcessHandle,
                      hSourceHandle,
                      hTargetProcessHandle,
                     &hResult,
                      dwDesiredAccess,
                      bInheritHandle,
                      dwOptions)) {
  if (GetLastError() == ERROR_OPERATION_ABORTED) {
   DBG_ALIGNMENT_ENABLE();
   goto again;
  }
  DBG_ALIGNMENT_ENABLE();
  return_none;
 }
 DBG_ALIGNMENT_ENABLE();
 result = DeeInt_NewSize((size_t)hResult);
 if unlikely(!result)
    CloseHandle(hResult);
 return result;
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("CreateFile",
      "lpFileName:nt:LPCWSTR"
     ",dwDesiredAccess:nt:DWORD=FILE_GENERIC_READ"
     ",dwShareMode:nt:DWORD=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE"
     ",lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL"
     ",dwCreationDisposition:nt:DWORD=OPEN_EXISTING"
     ",dwFlagsAndAttributes:nt:DWORD=FILE_ATTRIBUTE_NORMAL"
     ",hTemplateFile:nt:HANDLE=0"
     "->?X2?Dint?N"
     ); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_CreateFile_f_impl(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DeeObject *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
PRIVATE DREF DeeObject *DCALL libwin32_CreateFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_CREATEFILE_DEF { "CreateFile", (DeeObject *)&libwin32_CreateFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwDesiredAccess:?Dint=FILE_GENERIC_READ,dwShareMode:?Dint=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES,dwCreationDisposition:?Dint=OPEN_EXISTING,dwFlagsAndAttributes:?Dint=FILE_ATTRIBUTE_NORMAL,hTemplateFile:?Dint=0)->?X2?Dint?N") },
#define LIBWIN32_CREATEFILE_DEF_DOC(doc) { "CreateFile", (DeeObject *)&libwin32_CreateFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwDesiredAccess:?Dint=FILE_GENERIC_READ,dwShareMode:?Dint=FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES,dwCreationDisposition:?Dint=OPEN_EXISTING,dwFlagsAndAttributes:?Dint=FILE_ATTRIBUTE_NORMAL,hTemplateFile:?Dint=0)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateFile,libwin32_CreateFile_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile,{ K(lpFileName), K(dwDesiredAccess), K(dwShareMode), K(lpSecurityAttributes), K(dwCreationDisposition), K(dwFlagsAndAttributes), K(hTemplateFile), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWDESIREDACCESS_DWSHAREMODE_LPSECURITYATTRIBUTES_DWCREATIONDISPOSITION_DWFLAGSANDATTRIBUTES_HTEMPLATEFILE_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_CreateFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	DWORD dwDesiredAccess = FILE_GENERIC_READ;
	DWORD dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
	DeeObject *lpSecurityAttributes = NULL;
	DWORD dwCreationDisposition = OPEN_EXISTING;
	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	HANDLE hTemplateFile = 0;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpFileName_dwDesiredAccess_dwShareMode_lpSecurityAttributes_dwCreationDisposition_dwFlagsAndAttributes_hTemplateFile,"o|I32uI32uoI32uI32uIu:CreateFile",&lpFileName,&dwDesiredAccess,&dwShareMode,&lpSecurityAttributes,&dwCreationDisposition,&dwFlagsAndAttributes,&hTemplateFile))
	    goto err;
	if (DeeObject_AssertTypeExact(lpFileName,&DeeString_Type))
	    goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
	    goto err;
	return libwin32_CreateFile_f_impl(lpFileName_str,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_CreateFile_f_impl(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DeeObject *lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
//[[[end]]]
{
 HANDLE hResult;
 DREF DeeObject *result;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 (void)lpSecurityAttributes; /* TODO */
 hResult = CreateFileW(lpFileName,
                       dwDesiredAccess,
                       dwShareMode,
                       NULL,
                       dwCreationDisposition,
                       dwFlagsAndAttributes,
                       hTemplateFile);
 if unlikely(hResult == INVALID_HANDLE_VALUE) {
  if (GetLastError() == ERROR_OPERATION_ABORTED) {
   DBG_ALIGNMENT_ENABLE();
   goto again;
  }
  DBG_ALIGNMENT_ENABLE();
  return_none;
 }
 DBG_ALIGNMENT_ENABLE();
 result = DeeInt_NewSize((size_t)hResult);
 if likely(result)
    return result;
 CloseHandle(hResult);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("WriteFile",
      "hFile:nt:HANDLE"
     ",lpBuffer:obj:buffer"
     ",lpOverlapped:?GOVERLAPPED=NULL"
     "->?X2?Dint?N"
     ); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_WriteFile_f_impl(HANDLE hFile, DeeObject *__restrict lpBuffer, DeeObject *lpOverlapped);
PRIVATE DREF DeeObject *DCALL libwin32_WriteFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_WRITEFILE_DEF { "WriteFile", (DeeObject *)&libwin32_WriteFile, MODSYM_FNORMAL, DOC("(hFile:?Dint,lpBuffer:?Dbytes,lpOverlapped?:?GOVERLAPPED)->?X2?Dint?N") },
#define LIBWIN32_WRITEFILE_DEF_DOC(doc) { "WriteFile", (DeeObject *)&libwin32_WriteFile, MODSYM_FNORMAL, DOC("(hFile:?Dint,lpBuffer:?Dbytes,lpOverlapped?:?GOVERLAPPED)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_WriteFile,libwin32_WriteFile_f);
#ifndef LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED
#define LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpBuffer_lpOverlapped,{ K(hFile), K(lpBuffer), K(lpOverlapped), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_WriteFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	DeeObject *lpBuffer;
	DeeObject *lpOverlapped = NULL;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile_lpBuffer_lpOverlapped,"Iuo|o:WriteFile",&hFile,&lpBuffer,&lpOverlapped))
	    goto err;
	return libwin32_WriteFile_f_impl(hFile,lpBuffer,lpOverlapped);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_WriteFile_f_impl(HANDLE hFile, DeeObject *__restrict lpBuffer, DeeObject *lpOverlapped)
//[[[end]]]
{
 DREF DeeObject *result;
 DeeBuffer buffer;
 DWORD dwNumberOfBytesWritten;
 if (DeeObject_GetBuf(lpBuffer,&buffer,Dee_BUFFER_FREADONLY))
     goto err;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 (void)lpOverlapped; /* TODO */
 if (!WriteFile(hFile,
               (LPVOID)buffer.bb_base,
               (DWORD)buffer.bb_size,
               &dwNumberOfBytesWritten,
                NULL)) {
  if (GetLastError() == ERROR_OPERATION_ABORTED) {
   DBG_ALIGNMENT_ENABLE();
   goto again;
  }
  DBG_ALIGNMENT_ENABLE();
  result = Dee_None;
  Dee_Incref(Dee_None);
 } else {
  DBG_ALIGNMENT_ENABLE();
  result = DeeInt_NewU32((uint32_t)dwNumberOfBytesWritten);
 }
 DeeObject_PutBuf(lpBuffer,&buffer,Dee_BUFFER_FREADONLY);
 return result;
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("ReadFile",
      "hFile:nt:HANDLE"
     ",lpBuffer:obj:buffer"
     ",lpOverlapped:?GOVERLAPPED=NULL"
     "->?X2?Dint?N"
     ); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_ReadFile_f_impl(HANDLE hFile, DeeObject *__restrict lpBuffer, DeeObject *lpOverlapped);
PRIVATE DREF DeeObject *DCALL libwin32_ReadFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_READFILE_DEF { "ReadFile", (DeeObject *)&libwin32_ReadFile, MODSYM_FNORMAL, DOC("(hFile:?Dint,lpBuffer:?Dbytes,lpOverlapped?:?GOVERLAPPED)->?X2?Dint?N") },
#define LIBWIN32_READFILE_DEF_DOC(doc) { "ReadFile", (DeeObject *)&libwin32_ReadFile, MODSYM_FNORMAL, DOC("(hFile:?Dint,lpBuffer:?Dbytes,lpOverlapped?:?GOVERLAPPED)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_ReadFile,libwin32_ReadFile_f);
#ifndef LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED
#define LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpBuffer_lpOverlapped,{ K(hFile), K(lpBuffer), K(lpOverlapped), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPBUFFER_LPOVERLAPPED_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_ReadFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	DeeObject *lpBuffer;
	DeeObject *lpOverlapped = NULL;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile_lpBuffer_lpOverlapped,"Iuo|o:ReadFile",&hFile,&lpBuffer,&lpOverlapped))
	    goto err;
	return libwin32_ReadFile_f_impl(hFile,lpBuffer,lpOverlapped);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_ReadFile_f_impl(HANDLE hFile, DeeObject *__restrict lpBuffer, DeeObject *lpOverlapped)
//[[[end]]]
{
 DREF DeeObject *result;
 DeeBuffer buffer;
 DWORD dwNumberOfBytesRead;
 if (DeeObject_GetBuf(lpBuffer,&buffer,Dee_BUFFER_FWRITABLE))
     goto err;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 (void)lpOverlapped; /* TODO */
 if (!ReadFile(hFile,
              (LPVOID)buffer.bb_base,
              (DWORD)buffer.bb_size,
              &dwNumberOfBytesRead,
               NULL)) {
  if (GetLastError() == ERROR_OPERATION_ABORTED) {
   DBG_ALIGNMENT_ENABLE();
   goto again;
  }
  DBG_ALIGNMENT_ENABLE();
  result = Dee_None;
  Dee_Incref(Dee_None);
 } else {
  DBG_ALIGNMENT_ENABLE();
  result = DeeInt_NewU32((uint32_t)dwNumberOfBytesRead);
 }
 DeeObject_PutBuf(lpBuffer,&buffer,Dee_BUFFER_FWRITABLE);
 return result;
err:
 return NULL;
}


/*[[[deemon import("_dexutils").gw("CreateDirectory","lpPathName:nt:LPCWSTR,lpSecurityAttributes:?GSECURITY_ATTRIBUTES=NULL->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_CreateDirectory_f_impl(LPCWSTR lpPathName, DeeObject *lpSecurityAttributes);
PRIVATE DREF DeeObject *DCALL libwin32_CreateDirectory_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_CREATEDIRECTORY_DEF { "CreateDirectory", (DeeObject *)&libwin32_CreateDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)->?Dbool") },
#define LIBWIN32_CREATEDIRECTORY_DEF_DOC(doc) { "CreateDirectory", (DeeObject *)&libwin32_CreateDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring,lpSecurityAttributes?:?GSECURITY_ATTRIBUTES)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_CreateDirectory,libwin32_CreateDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName_lpSecurityAttributes,{ K(lpPathName), K(lpSecurityAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_LPSECURITYATTRIBUTES_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_CreateDirectory_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpPathName_str;
	DeeStringObject *lpPathName;
	DeeObject *lpSecurityAttributes = NULL;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpPathName_lpSecurityAttributes,"o|o:CreateDirectory",&lpPathName,&lpSecurityAttributes))
	    goto err;
	if (DeeObject_AssertTypeExact(lpPathName,&DeeString_Type))
	    goto err;
	lpPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpPathName);
	if unlikely(!lpPathName_str)
	    goto err;
	return libwin32_CreateDirectory_f_impl(lpPathName_str,lpSecurityAttributes);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_CreateDirectory_f_impl(LPCWSTR lpPathName, DeeObject *lpSecurityAttributes)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 (void)lpSecurityAttributes; /* TODO */
 bResult = CreateDirectoryW(lpPathName,NULL);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("RemoveDirectory","lpPathName:nt:LPCWSTR->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_RemoveDirectory_f_impl(LPCWSTR lpPathName);
PRIVATE DREF DeeObject *DCALL libwin32_RemoveDirectory_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_REMOVEDIRECTORY_DEF { "RemoveDirectory", (DeeObject *)&libwin32_RemoveDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)->?Dbool") },
#define LIBWIN32_REMOVEDIRECTORY_DEF_DOC(doc) { "RemoveDirectory", (DeeObject *)&libwin32_RemoveDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_RemoveDirectory,libwin32_RemoveDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName,{ K(lpPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_RemoveDirectory_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpPathName_str;
	DeeStringObject *lpPathName;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpPathName,"o:RemoveDirectory",&lpPathName))
	    goto err;
	if (DeeObject_AssertTypeExact(lpPathName,&DeeString_Type))
	    goto err;
	lpPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpPathName);
	if unlikely(!lpPathName_str)
	    goto err;
	return libwin32_RemoveDirectory_f_impl(lpPathName_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_RemoveDirectory_f_impl(LPCWSTR lpPathName)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = RemoveDirectoryW(lpPathName);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("DeleteFile","lpFileName:nt:LPCWSTR->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_DeleteFile_f_impl(LPCWSTR lpFileName);
PRIVATE DREF DeeObject *DCALL libwin32_DeleteFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_DELETEFILE_DEF { "DeleteFile", (DeeObject *)&libwin32_DeleteFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dbool") },
#define LIBWIN32_DELETEFILE_DEF_DOC(doc) { "DeleteFile", (DeeObject *)&libwin32_DeleteFile, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_DeleteFile,libwin32_DeleteFile_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName,{ K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_DeleteFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpFileName,"o:DeleteFile",&lpFileName))
	    goto err;
	if (DeeObject_AssertTypeExact(lpFileName,&DeeString_Type))
	    goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
	    goto err;
	return libwin32_DeleteFile_f_impl(lpFileName_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_DeleteFile_f_impl(LPCWSTR lpFileName)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = DeleteFileW(lpFileName);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("SetEndOfFile","hFile:nt:HANDLE->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_SetEndOfFile_f_impl(HANDLE hFile);
PRIVATE DREF DeeObject *DCALL libwin32_SetEndOfFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_SETENDOFFILE_DEF { "SetEndOfFile", (DeeObject *)&libwin32_SetEndOfFile, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?Dbool") },
#define LIBWIN32_SETENDOFFILE_DEF_DOC(doc) { "SetEndOfFile", (DeeObject *)&libwin32_SetEndOfFile, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetEndOfFile,libwin32_SetEndOfFile_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile,{ K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_SetEndOfFile_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile,"Iu:SetEndOfFile",&hFile))
	    goto err;
	return libwin32_SetEndOfFile_f_impl(hFile);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_SetEndOfFile_f_impl(HANDLE hFile)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = SetEndOfFile(hFile);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("SetFileAttributesW","lpFileName:nt:LPCWSTR,dwFileAttributes:nt:DWORD->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFileAttributesW_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes);
PRIVATE DREF DeeObject *DCALL libwin32_SetFileAttributesW_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_SETFILEATTRIBUTESW_DEF { "SetFileAttributesW", (DeeObject *)&libwin32_SetFileAttributesW, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)->?Dbool") },
#define LIBWIN32_SETFILEATTRIBUTESW_DEF_DOC(doc) { "SetFileAttributesW", (DeeObject *)&libwin32_SetFileAttributesW, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileAttributesW,libwin32_SetFileAttributesW_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwFileAttributes,{ K(lpFileName), K(dwFileAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_SetFileAttributesW_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	DWORD dwFileAttributes;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpFileName_dwFileAttributes,"oI32u:SetFileAttributesW",&lpFileName,&dwFileAttributes))
	    goto err;
	if (DeeObject_AssertTypeExact(lpFileName,&DeeString_Type))
	    goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
	    goto err;
	return libwin32_SetFileAttributesW_f_impl(lpFileName_str,dwFileAttributes);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFileAttributesW_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = SetFileAttributesW(lpFileName,dwFileAttributes);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}


/*[[[deemon import("_dexutils").gw("SetFilePointer","hFile:nt:HANDLE,lDistanceToMove:I64d,dwMoveMethod:nt:DWORD=FILE_BEGIN->?Dint"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFilePointer_f_impl(HANDLE hFile, int64_t lDistanceToMove, DWORD dwMoveMethod);
PRIVATE DREF DeeObject *DCALL libwin32_SetFilePointer_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_SETFILEPOINTER_DEF { "SetFilePointer", (DeeObject *)&libwin32_SetFilePointer, MODSYM_FNORMAL, DOC("(hFile:?Dint,lDistanceToMove:?Dint,dwMoveMethod:?Dint=FILE_BEGIN)->?Dint") },
#define LIBWIN32_SETFILEPOINTER_DEF_DOC(doc) { "SetFilePointer", (DeeObject *)&libwin32_SetFilePointer, MODSYM_FNORMAL, DOC("(hFile:?Dint,lDistanceToMove:?Dint,dwMoveMethod:?Dint=FILE_BEGIN)->?Dint\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFilePointer,libwin32_SetFilePointer_f);
#ifndef LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED
#define LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lDistanceToMove_dwMoveMethod,{ K(hFile), K(lDistanceToMove), K(dwMoveMethod), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LDISTANCETOMOVE_DWMOVEMETHOD_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_SetFilePointer_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	int64_t lDistanceToMove;
	DWORD dwMoveMethod = FILE_BEGIN;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile_lDistanceToMove_dwMoveMethod,"IuI64d|I32u:SetFilePointer",&hFile,&lDistanceToMove,&dwMoveMethod))
	    goto err;
	return libwin32_SetFilePointer_f_impl(hFile,lDistanceToMove,dwMoveMethod);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFilePointer_f_impl(HANDLE hFile, int64_t lDistanceToMove, DWORD dwMoveMethod)
//[[[end]]]
{
 DWORD dwResult;
 LONG lDistanceToMoveLow;
 LONG lDistanceToMoveHigh;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 lDistanceToMoveLow  = (LONG)(lDistanceToMove);
 lDistanceToMoveHigh = (LONG)(lDistanceToMove >> 32);
 DBG_ALIGNMENT_DISABLE();
 dwResult = SetFilePointer(hFile,
                           lDistanceToMoveLow,
                          &lDistanceToMoveHigh,
                           dwMoveMethod);
 if unlikely(dwResult == INVALID_SET_FILE_POINTER) {
  DWORD error = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (error != NO_ERROR) {
   if (error == ERROR_OPERATION_ABORTED)
       goto again;
   DBG_ALIGNMENT_ENABLE();
   return_none;
  }
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeInt_NewU64((uint64_t)(uint32_t)lDistanceToMoveHigh << 32 |
                      (uint64_t)(uint32_t)dwResult);
err:
 return NULL;
}


typedef union {
    uint64_t u64;
    FILETIME ft;
} ALIGNED_FILETIME;


/*[[[deemon import("_dexutils").gw("GetFileTime","hFile:nt:HANDLE->?X2?T3?Dint?Dint?Dint?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetFileTime_f_impl(HANDLE hFile);
PRIVATE DREF DeeObject *DCALL libwin32_GetFileTime_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETFILETIME_DEF { "GetFileTime", (DeeObject *)&libwin32_GetFileTime, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?X2?T3?Dint?Dint?Dint?N") },
#define LIBWIN32_GETFILETIME_DEF_DOC(doc) { "GetFileTime", (DeeObject *)&libwin32_GetFileTime, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?X2?T3?Dint?Dint?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileTime,libwin32_GetFileTime_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile,{ K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetFileTime_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile,"Iu:GetFileTime",&hFile))
	    goto err;
	return libwin32_GetFileTime_f_impl(hFile);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetFileTime_f_impl(HANDLE hFile)
//[[[end]]]
{
 BOOL bResult;
 ALIGNED_FILETIME ftCreationTime;
 ALIGNED_FILETIME ftLastAccessTime;
 ALIGNED_FILETIME ftLastWriteTime;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = GetFileTime(hFile,
                       &ftCreationTime.ft,
                       &ftLastAccessTime.ft,
                       &ftLastWriteTime.ft);
 if (!bResult) {
  if (GetLastError() == ERROR_OPERATION_ABORTED) {
   DBG_ALIGNMENT_ENABLE();
   goto again;
  }
  DBG_ALIGNMENT_ENABLE();
  return_none;
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeTuple_Newf(DEE_FMT_UINT64
                      DEE_FMT_UINT64
                      DEE_FMT_UINT64,
                      ftCreationTime.u64,
                      ftLastAccessTime.u64,
                      ftLastWriteTime.u64);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("SetFileTime","hFile:nt:HANDLE,lpCreationTime:?Dint=NULL,lpLastAccessTime:?Dint=NULL,lpLastWriteTime:?Dint=NULL->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFileTime_f_impl(HANDLE hFile, DeeObject *lpCreationTime, DeeObject *lpLastAccessTime, DeeObject *lpLastWriteTime);
PRIVATE DREF DeeObject *DCALL libwin32_SetFileTime_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_SETFILETIME_DEF { "SetFileTime", (DeeObject *)&libwin32_SetFileTime, MODSYM_FNORMAL, DOC("(hFile:?Dint,lpCreationTime?:?Dint,lpLastAccessTime?:?Dint,lpLastWriteTime?:?Dint)->?Dbool") },
#define LIBWIN32_SETFILETIME_DEF_DOC(doc) { "SetFileTime", (DeeObject *)&libwin32_SetFileTime, MODSYM_FNORMAL, DOC("(hFile:?Dint,lpCreationTime?:?Dint,lpLastAccessTime?:?Dint,lpLastWriteTime?:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileTime,libwin32_SetFileTime_f);
#ifndef LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED
#define LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime,{ K(hFile), K(lpCreationTime), K(lpLastAccessTime), K(lpLastWriteTime), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_LPCREATIONTIME_LPLASTACCESSTIME_LPLASTWRITETIME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_SetFileTime_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	DeeObject *lpCreationTime = NULL;
	DeeObject *lpLastAccessTime = NULL;
	DeeObject *lpLastWriteTime = NULL;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile_lpCreationTime_lpLastAccessTime_lpLastWriteTime,"Iu|ooo:SetFileTime",&hFile,&lpCreationTime,&lpLastAccessTime,&lpLastWriteTime))
	    goto err;
	return libwin32_SetFileTime_f_impl(hFile,lpCreationTime,lpLastAccessTime,lpLastWriteTime);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFileTime_f_impl(HANDLE hFile, DeeObject *lpCreationTime, DeeObject *lpLastAccessTime, DeeObject *lpLastWriteTime)
//[[[end]]]
{
 ALIGNED_FILETIME ftCreationTime;
 ALIGNED_FILETIME ftLastAccessTime;
 ALIGNED_FILETIME ftLastWriteTime;
 BOOL bResult;
 if (lpCreationTime && DeeObject_AsUInt64(lpCreationTime,&ftCreationTime.u64))
     goto err;
 if (lpLastAccessTime && DeeObject_AsUInt64(lpLastAccessTime,&ftLastAccessTime.u64))
     goto err;
 if (lpLastWriteTime && DeeObject_AsUInt64(lpLastWriteTime,&ftLastWriteTime.u64))
     goto err;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = SetFileTime(hFile,
                       lpCreationTime ? &ftCreationTime.ft : NULL,
                       lpLastAccessTime ? &ftLastAccessTime.ft : NULL,
                       lpLastWriteTime ? &ftLastWriteTime.ft : NULL);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("SetFileValidData","hFile:nt:HANDLE,ValidDataLength:I64u->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFileValidData_f_impl(HANDLE hFile, uint64_t ValidDataLength);
PRIVATE DREF DeeObject *DCALL libwin32_SetFileValidData_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_SETFILEVALIDDATA_DEF { "SetFileValidData", (DeeObject *)&libwin32_SetFileValidData, MODSYM_FNORMAL, DOC("(hFile:?Dint,ValidDataLength:?Dint)->?Dbool") },
#define LIBWIN32_SETFILEVALIDDATA_DEF_DOC(doc) { "SetFileValidData", (DeeObject *)&libwin32_SetFileValidData, MODSYM_FNORMAL, DOC("(hFile:?Dint,ValidDataLength:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileValidData,libwin32_SetFileValidData_f);
#ifndef LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED
#define LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile_ValidDataLength,{ K(hFile), K(ValidDataLength), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_VALIDDATALENGTH_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_SetFileValidData_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	uint64_t ValidDataLength;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile_ValidDataLength,"IuI64u:SetFileValidData",&hFile,&ValidDataLength))
	    goto err;
	return libwin32_SetFileValidData_f_impl(hFile,ValidDataLength);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFileValidData_f_impl(HANDLE hFile, uint64_t ValidDataLength)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = SetFileValidData(hFile,ValidDataLength);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}


/*[[[deemon import("_dexutils").gw("GetTempPath","->?X2?Dstring?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetTempPath_f_impl(void);
PRIVATE DREF DeeObject *DCALL libwin32_GetTempPath_f(size_t argc, DeeObject **__restrict argv);
#define LIBWIN32_GETTEMPPATH_DEF { "GetTempPath", (DeeObject *)&libwin32_GetTempPath, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N") },
#define LIBWIN32_GETTEMPPATH_DEF_DOC(doc) { "GetTempPath", (DeeObject *)&libwin32_GetTempPath, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetTempPath,libwin32_GetTempPath_f);
PRIVATE DREF DeeObject *DCALL libwin32_GetTempPath_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":GetTempPath"))
	    goto err;
	return libwin32_GetTempPath_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetTempPath_f_impl(void)
//[[[end]]]
{
 LPWSTR lpBuffer,lpNewBuffer;
 DWORD dwBufSize = PATH_MAX,dwError;
 lpBuffer = DeeString_NewWideBuffer(dwBufSize);
 if unlikely(!lpBuffer) goto err;
again:
 if (DeeThread_CheckInterrupt()) goto err_result;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  dwError = GetTempPathW(dwBufSize+1,lpBuffer);
  if (!dwError) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_OPERATION_ABORTED)
       goto again;
   if (dwError != 0)
       return_none;
  }
  DBG_ALIGNMENT_ENABLE();
  if (dwError <= dwBufSize) break;
  /* Resize to fit. */
  lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer,dwError);
  if unlikely(!lpNewBuffer) goto err_result;
  lpBuffer  = lpNewBuffer;
  dwBufSize = dwError;
 }
 lpNewBuffer = DeeString_TryResizeWideBuffer(lpBuffer,dwError);
 if likely(lpNewBuffer) lpBuffer = lpNewBuffer;
 return DeeString_PackWideBuffer(lpBuffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(lpBuffer);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("GetDllDirectory","->?X2?Dstring?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetDllDirectory_f_impl(void);
PRIVATE DREF DeeObject *DCALL libwin32_GetDllDirectory_f(size_t argc, DeeObject **__restrict argv);
#define LIBWIN32_GETDLLDIRECTORY_DEF { "GetDllDirectory", (DeeObject *)&libwin32_GetDllDirectory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N") },
#define LIBWIN32_GETDLLDIRECTORY_DEF_DOC(doc) { "GetDllDirectory", (DeeObject *)&libwin32_GetDllDirectory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetDllDirectory,libwin32_GetDllDirectory_f);
PRIVATE DREF DeeObject *DCALL libwin32_GetDllDirectory_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":GetDllDirectory"))
	    goto err;
	return libwin32_GetDllDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetDllDirectory_f_impl(void)
//[[[end]]]
{
 LPWSTR lpBuffer,lpNewBuffer;
 DWORD dwBufSize = PATH_MAX,dwError;
 typedef DWORD (WINAPI *LPGETDLLDIRECTORYW)(DWORD nBufferLength, LPWSTR lpBuffer);
 PRIVATE LPGETDLLDIRECTORYW pGetDllDirectoryW = NULL;
 if (pGetDllDirectoryW == NULL) {
  LPGETDLLDIRECTORYW new_pGetDllDirectoryW;
  DBG_ALIGNMENT_DISABLE();
  new_pGetDllDirectoryW = (LPGETDLLDIRECTORYW)GetProcAddress(GetModuleHandle(TEXT("KERNEL32")),
                                                             "GetDllDirectoryW");
  DBG_ALIGNMENT_ENABLE();
  if (new_pGetDllDirectoryW == NULL)
      new_pGetDllDirectoryW = (LPGETDLLDIRECTORYW)(void *)(uintptr_t)-1;
  pGetDllDirectoryW = new_pGetDllDirectoryW;
 }
 if (pGetDllDirectoryW == (LPGETDLLDIRECTORYW)(void *)(uintptr_t)-1) {
  SetLastError(ERROR_INVALID_FUNCTION);
  return_none;
 }

 lpBuffer = DeeString_NewWideBuffer(dwBufSize);
 if unlikely(!lpBuffer) goto err;
again:
 if (DeeThread_CheckInterrupt()) goto err_result;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  dwError = (*pGetDllDirectoryW)(dwBufSize+1,lpBuffer);
  if (!dwError) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_OPERATION_ABORTED)
       goto again;
   if (dwError != 0)
       return_none;
  }
  DBG_ALIGNMENT_ENABLE();
  if (dwError <= dwBufSize) break;
  /* Resize to fit. */
  lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer,dwError);
  if unlikely(!lpNewBuffer) goto err_result;
  lpBuffer  = lpNewBuffer;
  dwBufSize = dwError;
 }
 lpNewBuffer = DeeString_TryResizeWideBuffer(lpBuffer,dwError);
 if likely(lpNewBuffer) lpBuffer = lpNewBuffer;
 return DeeString_PackWideBuffer(lpBuffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(lpBuffer);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("SetDllDirectory","lpPathName:nt:LPCWSTR->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_SetDllDirectory_f_impl(LPCWSTR lpPathName);
PRIVATE DREF DeeObject *DCALL libwin32_SetDllDirectory_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_SETDLLDIRECTORY_DEF { "SetDllDirectory", (DeeObject *)&libwin32_SetDllDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)->?Dbool") },
#define LIBWIN32_SETDLLDIRECTORY_DEF_DOC(doc) { "SetDllDirectory", (DeeObject *)&libwin32_SetDllDirectory, MODSYM_FNORMAL, DOC("(lpPathName:?Dstring)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetDllDirectory,libwin32_SetDllDirectory_f);
#ifndef LIBWIN32_KWDS_LPPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPPATHNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpPathName,{ K(lpPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPPATHNAME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_SetDllDirectory_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpPathName_str;
	DeeStringObject *lpPathName;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpPathName,"o:SetDllDirectory",&lpPathName))
	    goto err;
	if (DeeObject_AssertTypeExact(lpPathName,&DeeString_Type))
	    goto err;
	lpPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpPathName);
	if unlikely(!lpPathName_str)
	    goto err;
	return libwin32_SetDllDirectory_f_impl(lpPathName_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_SetDllDirectory_f_impl(LPCWSTR lpPathName)
//[[[end]]]
{
 BOOL bResult;
 typedef BOOL (WINAPI *LPSETDLLDIRECTORYW)(LPCWSTR lpPathName);
 PRIVATE LPSETDLLDIRECTORYW pSetDllDirectoryW = NULL;
 if (pSetDllDirectoryW == NULL) {
  LPSETDLLDIRECTORYW new_pSetDllDirectoryW;
  DBG_ALIGNMENT_DISABLE();
  new_pSetDllDirectoryW = (LPSETDLLDIRECTORYW)GetProcAddress(GetModuleHandle(TEXT("KERNEL32")),
                                                             "SetDllDirectoryW");
  DBG_ALIGNMENT_ENABLE();
  if (new_pSetDllDirectoryW == NULL)
      new_pSetDllDirectoryW = (LPSETDLLDIRECTORYW)(void *)(uintptr_t)-1;
  pSetDllDirectoryW = new_pSetDllDirectoryW;
 }
 if (pSetDllDirectoryW == (LPSETDLLDIRECTORYW)(void *)(uintptr_t)-1) {
  SetLastError(ERROR_INVALID_FUNCTION);
  return_none;
 }
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = (*pSetDllDirectoryW)(lpPathName);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}

/*[[[deemon import("_dexutils").gw("GetDiskFreeSpace","lpRootPathName:nt:LPCWSTR->?X2?T4?Dint?Dint?Dint?Dint?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f_impl(LPCWSTR lpRootPathName);
PRIVATE DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETDISKFREESPACE_DEF { "GetDiskFreeSpace", (DeeObject *)&libwin32_GetDiskFreeSpace, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?X2?T4?Dint?Dint?Dint?Dint?N") },
#define LIBWIN32_GETDISKFREESPACE_DEF_DOC(doc) { "GetDiskFreeSpace", (DeeObject *)&libwin32_GetDiskFreeSpace, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?X2?T4?Dint?Dint?Dint?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDiskFreeSpace,libwin32_GetDiskFreeSpace_f);
#ifndef LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpRootPathName,{ K(lpRootPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpRootPathName_str;
	DeeStringObject *lpRootPathName;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpRootPathName,"o:GetDiskFreeSpace",&lpRootPathName))
	    goto err;
	if (DeeObject_AssertTypeExact(lpRootPathName,&DeeString_Type))
	    goto err;
	lpRootPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpRootPathName);
	if unlikely(!lpRootPathName_str)
	    goto err;
	return libwin32_GetDiskFreeSpace_f_impl(lpRootPathName_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetDiskFreeSpace_f_impl(LPCWSTR lpRootPathName)
//[[[end]]]
{
 BOOL bResult;
 DWORD dwSectorsPerCluster;
 DWORD dwBytesPerSector;
 DWORD dwNumberOfFreeClusters;
 DWORD dwTotalNumberOfClusters;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = GetDiskFreeSpaceW(lpRootPathName,
                            &dwSectorsPerCluster,
                            &dwBytesPerSector,
                            &dwNumberOfFreeClusters,
                            &dwTotalNumberOfClusters);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeTuple_Newf(DEE_FMT_UINT32
                      DEE_FMT_UINT32
                      DEE_FMT_UINT32
                      DEE_FMT_UINT32,
                      dwSectorsPerCluster,
                      dwBytesPerSector,
                      dwNumberOfFreeClusters,
                      dwTotalNumberOfClusters);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("GetDiskFreeSpaceEx","lpDirectoryName:nt:LPCWSTR->?X2?T3?Dint?Dint?Dint?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f_impl(LPCWSTR lpDirectoryName);
PRIVATE DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETDISKFREESPACEEX_DEF { "GetDiskFreeSpaceEx", (DeeObject *)&libwin32_GetDiskFreeSpaceEx, MODSYM_FNORMAL, DOC("(lpDirectoryName:?Dstring)->?X2?T3?Dint?Dint?Dint?N") },
#define LIBWIN32_GETDISKFREESPACEEX_DEF_DOC(doc) { "GetDiskFreeSpaceEx", (DeeObject *)&libwin32_GetDiskFreeSpaceEx, MODSYM_FNORMAL, DOC("(lpDirectoryName:?Dstring)->?X2?T3?Dint?Dint?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDiskFreeSpaceEx,libwin32_GetDiskFreeSpaceEx_f);
#ifndef LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED
#define LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpDirectoryName,{ K(lpDirectoryName), KEND });
#endif /* !LIBWIN32_KWDS_LPDIRECTORYNAME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpDirectoryName_str;
	DeeStringObject *lpDirectoryName;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpDirectoryName,"o:GetDiskFreeSpaceEx",&lpDirectoryName))
	    goto err;
	if (DeeObject_AssertTypeExact(lpDirectoryName,&DeeString_Type))
	    goto err;
	lpDirectoryName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpDirectoryName);
	if unlikely(!lpDirectoryName_str)
	    goto err;
	return libwin32_GetDiskFreeSpaceEx_f_impl(lpDirectoryName_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetDiskFreeSpaceEx_f_impl(LPCWSTR lpDirectoryName)
//[[[end]]]
{
 BOOL bResult;
 ULARGE_INTEGER uFreeBytesAvailableToCaller;
 ULARGE_INTEGER uTotalNumberOfBytes;
 ULARGE_INTEGER uTotalNumberOfFreeBytes;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = GetDiskFreeSpaceExW(lpDirectoryName,
                              &uFreeBytesAvailableToCaller,
                              &uTotalNumberOfBytes,
                              &uTotalNumberOfFreeBytes);
 if (!bResult) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_OPERATION_ABORTED)
      goto again;
  return_none;
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeTuple_Newf(DEE_FMT_UINT64
                      DEE_FMT_UINT64
                      DEE_FMT_UINT64,
                      uFreeBytesAvailableToCaller.QuadPart,
                      uTotalNumberOfBytes.QuadPart,
                      uTotalNumberOfFreeBytes.QuadPart);
err:
 return NULL;
}


/*[[[deemon import("_dexutils").gw("GetDriveType","lpRootPathName:nt:LPCWSTR->?X2?Dint?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetDriveType_f_impl(LPCWSTR lpRootPathName);
PRIVATE DREF DeeObject *DCALL libwin32_GetDriveType_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETDRIVETYPE_DEF { "GetDriveType", (DeeObject *)&libwin32_GetDriveType, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?X2?Dint?N") },
#define LIBWIN32_GETDRIVETYPE_DEF_DOC(doc) { "GetDriveType", (DeeObject *)&libwin32_GetDriveType, MODSYM_FNORMAL, DOC("(lpRootPathName:?Dstring)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetDriveType,libwin32_GetDriveType_f);
#ifndef LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED
#define LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpRootPathName,{ K(lpRootPathName), KEND });
#endif /* !LIBWIN32_KWDS_LPROOTPATHNAME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetDriveType_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpRootPathName_str;
	DeeStringObject *lpRootPathName;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpRootPathName,"o:GetDriveType",&lpRootPathName))
	    goto err;
	if (DeeObject_AssertTypeExact(lpRootPathName,&DeeString_Type))
	    goto err;
	lpRootPathName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpRootPathName);
	if unlikely(!lpRootPathName_str)
	    goto err;
	return libwin32_GetDriveType_f_impl(lpRootPathName_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetDriveType_f_impl(LPCWSTR lpRootPathName)
//[[[end]]]
{
 UINT uResult;
 DWORD dwError;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 SetLastError(NO_ERROR);
 uResult = GetDriveTypeW(lpRootPathName);
 dwError = GetLastError();
 DBG_ALIGNMENT_ENABLE();
 if (dwError == ERROR_OPERATION_ABORTED)
     goto again;
 if (dwError != NO_ERROR)
     return_none;
 return DeeInt_NewUInt(uResult);
err:
 return NULL;
}




/*[[[deemon import("_dexutils").gw("GetModuleFileName","hModule:nt:HANDLE->?X2?Dstring?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetModuleFileName_f_impl(HANDLE hModule);
PRIVATE DREF DeeObject *DCALL libwin32_GetModuleFileName_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETMODULEFILENAME_DEF { "GetModuleFileName", (DeeObject *)&libwin32_GetModuleFileName, MODSYM_FNORMAL, DOC("(hModule:?Dint)->?X2?Dstring?N") },
#define LIBWIN32_GETMODULEFILENAME_DEF_DOC(doc) { "GetModuleFileName", (DeeObject *)&libwin32_GetModuleFileName, MODSYM_FNORMAL, DOC("(hModule:?Dint)->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetModuleFileName,libwin32_GetModuleFileName_f);
#ifndef LIBWIN32_KWDS_HMODULE_DEFINED
#define LIBWIN32_KWDS_HMODULE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hModule,{ K(hModule), KEND });
#endif /* !LIBWIN32_KWDS_HMODULE_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetModuleFileName_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hModule;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hModule,"Iu:GetModuleFileName",&hModule))
	    goto err;
	return libwin32_GetModuleFileName_f_impl(hModule);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetModuleFileName_f_impl(HANDLE hModule)
//[[[end]]]
{
 LPWSTR lpBuffer,lpNewBuffer;
 DWORD dwBufSize = PATH_MAX,dwError;
 lpBuffer = DeeString_NewWideBuffer(dwBufSize);
 if unlikely(!lpBuffer) goto err;
again:
 if (DeeThread_CheckInterrupt()) goto err_buffer;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  dwError = GetModuleFileNameW((HMODULE)hModule,lpBuffer,dwBufSize + 1);
  if (!dwError) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_OPERATION_ABORTED)
       goto again;
   DeeString_FreeWideBuffer(lpBuffer);
   return_none;
  }
  DBG_ALIGNMENT_ENABLE();
  if (dwError <= dwBufSize) {
   if (dwError < dwBufSize)
       break;
   DBG_ALIGNMENT_DISABLE();
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError != ERROR_INSUFFICIENT_BUFFER)
       break;
  }
  /* Increase buffer size. */
  dwBufSize  *= 2;
  lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer,dwBufSize);
  if unlikely(!lpNewBuffer) goto err_buffer;
  lpBuffer = lpNewBuffer;
 }
 lpBuffer = DeeString_TruncateWideBuffer(lpBuffer,dwError);
 return DeeString_PackWideBuffer(lpBuffer,STRING_ERROR_FREPLAC);
err_buffer:
 DeeString_FreeWideBuffer(lpBuffer);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("GetSystemDirectory","->?X2?Dstring?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetSystemDirectory_f_impl(void);
PRIVATE DREF DeeObject *DCALL libwin32_GetSystemDirectory_f(size_t argc, DeeObject **__restrict argv);
#define LIBWIN32_GETSYSTEMDIRECTORY_DEF { "GetSystemDirectory", (DeeObject *)&libwin32_GetSystemDirectory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N") },
#define LIBWIN32_GETSYSTEMDIRECTORY_DEF_DOC(doc) { "GetSystemDirectory", (DeeObject *)&libwin32_GetSystemDirectory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemDirectory,libwin32_GetSystemDirectory_f);
PRIVATE DREF DeeObject *DCALL libwin32_GetSystemDirectory_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":GetSystemDirectory"))
	    goto err;
	return libwin32_GetSystemDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetSystemDirectory_f_impl(void)
//[[[end]]]
{
 LPWSTR lpBuffer,lpNewBuffer;
 DWORD dwBufSize = PATH_MAX,dwError;
 lpBuffer = DeeString_NewWideBuffer(dwBufSize);
 if unlikely(!lpBuffer) goto err;
again:
 if (DeeThread_CheckInterrupt()) goto err_result;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  dwError = GetSystemDirectoryW(lpBuffer,dwBufSize+1);
  if (!dwError) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_OPERATION_ABORTED)
       goto again;
   if (dwError != 0)
       return_none;
  }
  DBG_ALIGNMENT_ENABLE();
  if (dwError <= dwBufSize) break;
  /* Resize to fit. */
  lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer,dwError);
  if unlikely(!lpNewBuffer) goto err_result;
  lpBuffer  = lpNewBuffer;
  dwBufSize = dwError;
 }
 lpNewBuffer = DeeString_TryResizeWideBuffer(lpBuffer,dwError);
 if likely(lpNewBuffer) lpBuffer = lpNewBuffer;
 return DeeString_PackWideBuffer(lpBuffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(lpBuffer);
err:
 return NULL;
}




/*[[[deemon import("_dexutils").gw("GetWindowsDirectory","->?X2?Dstring?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f_impl(void);
PRIVATE DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f(size_t argc, DeeObject **__restrict argv);
#define LIBWIN32_GETWINDOWSDIRECTORY_DEF { "GetWindowsDirectory", (DeeObject *)&libwin32_GetWindowsDirectory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N") },
#define LIBWIN32_GETWINDOWSDIRECTORY_DEF_DOC(doc) { "GetWindowsDirectory", (DeeObject *)&libwin32_GetWindowsDirectory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetWindowsDirectory,libwin32_GetWindowsDirectory_f);
PRIVATE DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":GetWindowsDirectory"))
	    goto err;
	return libwin32_GetWindowsDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetWindowsDirectory_f_impl(void)
//[[[end]]]
{
 LPWSTR lpBuffer,lpNewBuffer;
 DWORD dwBufSize = PATH_MAX,dwError;
 lpBuffer = DeeString_NewWideBuffer(dwBufSize);
 if unlikely(!lpBuffer) goto err;
again:
 if (DeeThread_CheckInterrupt()) goto err_result;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  dwError = GetWindowsDirectoryW(lpBuffer,dwBufSize+1);
  if (!dwError) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_OPERATION_ABORTED)
       goto again;
   if (dwError != 0)
       return_none;
  }
  DBG_ALIGNMENT_ENABLE();
  if (dwError <= dwBufSize) break;
  /* Resize to fit. */
  lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer,dwError);
  if unlikely(!lpNewBuffer) goto err_result;
  lpBuffer  = lpNewBuffer;
  dwBufSize = dwError;
 }
 lpNewBuffer = DeeString_TryResizeWideBuffer(lpBuffer,dwError);
 if likely(lpNewBuffer) lpBuffer = lpNewBuffer;
 return DeeString_PackWideBuffer(lpBuffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(lpBuffer);
err:
 return NULL;
}


/*[[[deemon import("_dexutils").gw("GetSystemWindowsDirectory","->?X2?Dstring?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f_impl(void);
PRIVATE DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f(size_t argc, DeeObject **__restrict argv);
#define LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF { "GetSystemWindowsDirectory", (DeeObject *)&libwin32_GetSystemWindowsDirectory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N") },
#define LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF_DOC(doc) { "GetSystemWindowsDirectory", (DeeObject *)&libwin32_GetSystemWindowsDirectory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemWindowsDirectory,libwin32_GetSystemWindowsDirectory_f);
PRIVATE DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":GetSystemWindowsDirectory"))
	    goto err;
	return libwin32_GetSystemWindowsDirectory_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetSystemWindowsDirectory_f_impl(void)
//[[[end]]]
{
 LPWSTR lpBuffer,lpNewBuffer;
 DWORD dwBufSize = PATH_MAX,dwError;
 lpBuffer = DeeString_NewWideBuffer(dwBufSize);
 if unlikely(!lpBuffer) goto err;
again:
 if (DeeThread_CheckInterrupt()) goto err_result;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  dwError = GetSystemWindowsDirectoryW(lpBuffer,dwBufSize+1);
  if (!dwError) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_OPERATION_ABORTED)
       goto again;
   if (dwError != 0)
       return_none;
  }
  DBG_ALIGNMENT_ENABLE();
  if (dwError <= dwBufSize) break;
  /* Resize to fit. */
  lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer,dwError);
  if unlikely(!lpNewBuffer) goto err_result;
  lpBuffer  = lpNewBuffer;
  dwBufSize = dwError;
 }
 lpNewBuffer = DeeString_TryResizeWideBuffer(lpBuffer,dwError);
 if likely(lpNewBuffer) lpBuffer = lpNewBuffer;
 return DeeString_PackWideBuffer(lpBuffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(lpBuffer);
err:
 return NULL;
}




/*[[[deemon import("_dexutils").gw("GetSystemWow64Directory","->?X2?Dstring?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f_impl(void);
PRIVATE DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f(size_t argc, DeeObject **__restrict argv);
#define LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF { "GetSystemWow64Directory", (DeeObject *)&libwin32_GetSystemWow64Directory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N") },
#define LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF_DOC(doc) { "GetSystemWow64Directory", (DeeObject *)&libwin32_GetSystemWow64Directory, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetSystemWow64Directory,libwin32_GetSystemWow64Directory_f);
PRIVATE DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":GetSystemWow64Directory"))
	    goto err;
	return libwin32_GetSystemWow64Directory_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetSystemWow64Directory_f_impl(void)
//[[[end]]]
{
 LPWSTR lpBuffer,lpNewBuffer;
 DWORD dwBufSize = PATH_MAX,dwError;
 PRIVATE PGET_SYSTEM_WOW64_DIRECTORY_W pGetSystemWow64DirectoryW = NULL;
 if (pGetSystemWow64DirectoryW == NULL) {
  PGET_SYSTEM_WOW64_DIRECTORY_W new_pGetSystemWow64DirectoryW;
  DBG_ALIGNMENT_DISABLE();
#ifndef GET_SYSTEM_WOW64_DIRECTORY_NAME_W_A
#define GET_SYSTEM_WOW64_DIRECTORY_NAME_W_A      "GetSystemWow64DirectoryW"
#endif
  new_pGetSystemWow64DirectoryW = (PGET_SYSTEM_WOW64_DIRECTORY_W)GetProcAddress(GetModuleHandle(TEXT("KERNEL32")),
                                                                                GET_SYSTEM_WOW64_DIRECTORY_NAME_W_A);
  DBG_ALIGNMENT_ENABLE();
  if (new_pGetSystemWow64DirectoryW == NULL)
      new_pGetSystemWow64DirectoryW = (PGET_SYSTEM_WOW64_DIRECTORY_W)(void *)(uintptr_t)-1;
  pGetSystemWow64DirectoryW = new_pGetSystemWow64DirectoryW;
 }
 if (pGetSystemWow64DirectoryW == (PGET_SYSTEM_WOW64_DIRECTORY_W)(void *)(uintptr_t)-1) {
  SetLastError(ERROR_INVALID_FUNCTION);
  return_none;
 }

 lpBuffer = DeeString_NewWideBuffer(dwBufSize);
 if unlikely(!lpBuffer) goto err;
again:
 if (DeeThread_CheckInterrupt()) goto err_result;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  dwError = (*pGetSystemWow64DirectoryW)(lpBuffer,dwBufSize+1);
  if (!dwError) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_OPERATION_ABORTED)
       goto again;
   if (dwError != 0)
       return_none;
  }
  DBG_ALIGNMENT_ENABLE();
  if (dwError <= dwBufSize) break;
  /* Resize to fit. */
  lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer,dwError);
  if unlikely(!lpNewBuffer) goto err_result;
  lpBuffer  = lpNewBuffer;
  dwBufSize = dwError;
 }
 lpNewBuffer = DeeString_TryResizeWideBuffer(lpBuffer,dwError);
 if likely(lpNewBuffer) lpBuffer = lpNewBuffer;
 return DeeString_PackWideBuffer(lpBuffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(lpBuffer);
err:
 return NULL;
}




/*[[[deemon import("_dexutils").gw("GetLogicalDriveStrings","->?X2?Dstring?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f_impl(void);
PRIVATE DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f(size_t argc, DeeObject **__restrict argv);
#define LIBWIN32_GETLOGICALDRIVESTRINGS_DEF { "GetLogicalDriveStrings", (DeeObject *)&libwin32_GetLogicalDriveStrings, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N") },
#define LIBWIN32_GETLOGICALDRIVESTRINGS_DEF_DOC(doc) { "GetLogicalDriveStrings", (DeeObject *)&libwin32_GetLogicalDriveStrings, MODSYM_FNORMAL, DOC("()->?X2?Dstring?N\n" doc) },
PRIVATE DEFINE_CMETHOD(libwin32_GetLogicalDriveStrings,libwin32_GetLogicalDriveStrings_f);
PRIVATE DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f(size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc,argv,":GetLogicalDriveStrings"))
	    goto err;
	return libwin32_GetLogicalDriveStrings_f_impl();
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetLogicalDriveStrings_f_impl(void)
//[[[end]]]
{
 LPWSTR lpBuffer,lpNewBuffer;
 DWORD dwBufSize = PATH_MAX,dwError;
 lpBuffer = DeeString_NewWideBuffer(dwBufSize);
 if unlikely(!lpBuffer) goto err;
again:
 if (DeeThread_CheckInterrupt()) goto err_result;
 for (;;) {
  DBG_ALIGNMENT_DISABLE();
  dwError = GetLogicalDriveStringsW(dwBufSize+1,lpBuffer);
  if (!dwError) {
   dwError = GetLastError();
   DBG_ALIGNMENT_ENABLE();
   if (dwError == ERROR_OPERATION_ABORTED)
       goto again;
   if (dwError != 0)
       return_none;
  }
  DBG_ALIGNMENT_ENABLE();
  if (dwError <= dwBufSize) break;
  /* Resize to fit. */
  lpNewBuffer = DeeString_ResizeWideBuffer(lpBuffer,dwError);
  if unlikely(!lpNewBuffer) goto err_result;
  lpBuffer  = lpNewBuffer;
  dwBufSize = dwError;
 }
 lpNewBuffer = DeeString_TryResizeWideBuffer(lpBuffer,dwError);
 if likely(lpNewBuffer) lpBuffer = lpNewBuffer;
 return DeeString_PackWideBuffer(lpBuffer,STRING_ERROR_FREPLAC);
err_result:
 DeeString_FreeWideBuffer(lpBuffer);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("GetFileType","hFile:nt:HANDLE->?X2?Dint?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetFileType_f_impl(HANDLE hFile);
PRIVATE DREF DeeObject *DCALL libwin32_GetFileType_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETFILETYPE_DEF { "GetFileType", (DeeObject *)&libwin32_GetFileType, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?X2?Dint?N") },
#define LIBWIN32_GETFILETYPE_DEF_DOC(doc) { "GetFileType", (DeeObject *)&libwin32_GetFileType, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileType,libwin32_GetFileType_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile,{ K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetFileType_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile,"Iu:GetFileType",&hFile))
	    goto err;
	return libwin32_GetFileType_f_impl(hFile);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetFileType_f_impl(HANDLE hFile)
//[[[end]]]
{
 DWORD dwType;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 dwType = GetFileType(hFile);
 if (dwType == FILE_TYPE_UNKNOWN) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_OPERATION_ABORTED)
      goto again;
  if (dwError != NO_ERROR)
      return_none;
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeInt_NewU32((uint32_t)dwType);
err:
 return NULL;
}




/*[[[deemon import("_dexutils").gw("GetFileSize","hFile:nt:HANDLE->?X2?Dint?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetFileSize_f_impl(HANDLE hFile);
PRIVATE DREF DeeObject *DCALL libwin32_GetFileSize_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETFILESIZE_DEF { "GetFileSize", (DeeObject *)&libwin32_GetFileSize, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?X2?Dint?N") },
#define LIBWIN32_GETFILESIZE_DEF_DOC(doc) { "GetFileSize", (DeeObject *)&libwin32_GetFileSize, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileSize,libwin32_GetFileSize_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile,{ K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetFileSize_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile,"Iu:GetFileSize",&hFile))
	    goto err;
	return libwin32_GetFileSize_f_impl(hFile);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetFileSize_f_impl(HANDLE hFile)
//[[[end]]]
{
 DWORD dwSizeLow,dwSizeHigh;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 dwSizeLow = GetFileSize(hFile,&dwSizeHigh);
 if (dwSizeLow == INVALID_FILE_SIZE) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_OPERATION_ABORTED)
      goto again;
  if (dwError != NO_ERROR)
      return_none;
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeInt_NewU64((uint64_t)dwSizeLow |
                      (uint64_t)dwSizeHigh << 32);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("GetFileAttributes","lpFileName:nt:LPCWSTR->?X2?Dint?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetFileAttributes_f_impl(LPCWSTR lpFileName);
PRIVATE DREF DeeObject *DCALL libwin32_GetFileAttributes_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETFILEATTRIBUTES_DEF { "GetFileAttributes", (DeeObject *)&libwin32_GetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?X2?Dint?N") },
#define LIBWIN32_GETFILEATTRIBUTES_DEF_DOC(doc) { "GetFileAttributes", (DeeObject *)&libwin32_GetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetFileAttributes,libwin32_GetFileAttributes_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName,{ K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetFileAttributes_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpFileName,"o:GetFileAttributes",&lpFileName))
	    goto err;
	if (DeeObject_AssertTypeExact(lpFileName,&DeeString_Type))
	    goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
	    goto err;
	return libwin32_GetFileAttributes_f_impl(lpFileName_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetFileAttributes_f_impl(LPCWSTR lpFileName)
//[[[end]]]
{
 DWORD dwResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 dwResult = GetFileAttributesW(lpFileName);
 if (dwResult == INVALID_FILE_ATTRIBUTES) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_OPERATION_ABORTED)
      goto again;
  if (dwError != NO_ERROR)
      return_none;
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeInt_NewU32((uint32_t)dwResult);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("SetFileAttributes","lpFileName:nt:LPCWSTR,dwFileAttributes:nt:DWORD->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFileAttributes_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes);
PRIVATE DREF DeeObject *DCALL libwin32_SetFileAttributes_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_SETFILEATTRIBUTES_DEF { "SetFileAttributes", (DeeObject *)&libwin32_SetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)->?Dbool") },
#define LIBWIN32_SETFILEATTRIBUTES_DEF_DOC(doc) { "SetFileAttributes", (DeeObject *)&libwin32_SetFileAttributes, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring,dwFileAttributes:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_SetFileAttributes,libwin32_SetFileAttributes_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName_dwFileAttributes,{ K(lpFileName), K(dwFileAttributes), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DWFILEATTRIBUTES_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_SetFileAttributes_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	DWORD dwFileAttributes;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpFileName_dwFileAttributes,"oI32u:SetFileAttributes",&lpFileName,&dwFileAttributes))
	    goto err;
	if (DeeObject_AssertTypeExact(lpFileName,&DeeString_Type))
	    goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
	    goto err;
	return libwin32_SetFileAttributes_f_impl(lpFileName_str,dwFileAttributes);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_SetFileAttributes_f_impl(LPCWSTR lpFileName, DWORD dwFileAttributes)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = SetFileAttributesW(lpFileName,
                              dwFileAttributes);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("GetCompressedFileSize","lpFileName:nt:LPCWSTR->?X2?Dint?N"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f_impl(LPCWSTR lpFileName);
PRIVATE DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_GETCOMPRESSEDFILESIZE_DEF { "GetCompressedFileSize", (DeeObject *)&libwin32_GetCompressedFileSize, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?X2?Dint?N") },
#define LIBWIN32_GETCOMPRESSEDFILESIZE_DEF_DOC(doc) { "GetCompressedFileSize", (DeeObject *)&libwin32_GetCompressedFileSize, MODSYM_FNORMAL, DOC("(lpFileName:?Dstring)->?X2?Dint?N\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_GetCompressedFileSize,libwin32_GetCompressedFileSize_f);
#ifndef LIBWIN32_KWDS_LPFILENAME_DEFINED
#define LIBWIN32_KWDS_LPFILENAME_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_lpFileName,{ K(lpFileName), KEND });
#endif /* !LIBWIN32_KWDS_LPFILENAME_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	LPCWSTR lpFileName_str;
	DeeStringObject *lpFileName;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_lpFileName,"o:GetCompressedFileSize",&lpFileName))
	    goto err;
	if (DeeObject_AssertTypeExact(lpFileName,&DeeString_Type))
	    goto err;
	lpFileName_str = (LPCWSTR)DeeString_AsWide((DeeObject *)lpFileName);
	if unlikely(!lpFileName_str)
	    goto err;
	return libwin32_GetCompressedFileSize_f_impl(lpFileName_str);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_GetCompressedFileSize_f_impl(LPCWSTR lpFileName)
//[[[end]]]
{
 DWORD dwSizeLow,dwSizeHigh;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 dwSizeLow = GetCompressedFileSizeW(lpFileName,&dwSizeHigh);
 if (dwSizeLow == INVALID_FILE_SIZE) {
  DWORD dwError = GetLastError();
  DBG_ALIGNMENT_ENABLE();
  if (dwError == ERROR_OPERATION_ABORTED)
      goto again;
  if (dwError != NO_ERROR)
      return_none;
 }
 DBG_ALIGNMENT_ENABLE();
 return DeeInt_NewU64((uint64_t)dwSizeLow |
                      (uint64_t)dwSizeHigh << 32);
err:
 return NULL;
}



/*[[[deemon import("_dexutils").gw("FlushFileBuffers","hFile:nt:HANDLE->?Dbool"); ]]]*/
FORCELOCAL DREF DeeObject *DCALL libwin32_FlushFileBuffers_f_impl(HANDLE hFile);
PRIVATE DREF DeeObject *DCALL libwin32_FlushFileBuffers_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define LIBWIN32_FLUSHFILEBUFFERS_DEF { "FlushFileBuffers", (DeeObject *)&libwin32_FlushFileBuffers, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?Dbool") },
#define LIBWIN32_FLUSHFILEBUFFERS_DEF_DOC(doc) { "FlushFileBuffers", (DeeObject *)&libwin32_FlushFileBuffers, MODSYM_FNORMAL, DOC("(hFile:?Dint)->?Dbool\n" doc) },
PRIVATE DEFINE_KWCMETHOD(libwin32_FlushFileBuffers,libwin32_FlushFileBuffers_f);
#ifndef LIBWIN32_KWDS_HFILE_DEFINED
#define LIBWIN32_KWDS_HFILE_DEFINED 1
PRIVATE DEFINE_KWLIST(libwin32_kwds_hFile,{ K(hFile), KEND });
#endif /* !LIBWIN32_KWDS_HFILE_DEFINED */
PRIVATE DREF DeeObject *DCALL libwin32_FlushFileBuffers_f(size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
	HANDLE hFile;
	if (DeeArg_UnpackKw(argc,argv,kw,libwin32_kwds_hFile,"Iu:FlushFileBuffers",&hFile))
	    goto err;
	return libwin32_FlushFileBuffers_f_impl(hFile);
err:
	return NULL;
}
FORCELOCAL DREF DeeObject *DCALL libwin32_FlushFileBuffers_f_impl(HANDLE hFile)
//[[[end]]]
{
 BOOL bResult;
again:
 if (DeeThread_CheckInterrupt())
     goto err;
 DBG_ALIGNMENT_DISABLE();
 bResult = FlushFileBuffers(hFile);
 if (!bResult && GetLastError() == ERROR_OPERATION_ABORTED) {
  DBG_ALIGNMENT_ENABLE();
  goto again;
 }
 DBG_ALIGNMENT_ENABLE();
 return_bool_(bResult);
err:
 return NULL;
}















PRIVATE struct dex_symbol symbols[] = {
    /* TODO: Wrapper types for `SECURITY_ATTRIBUTES' and `OVERLAPPED' */
    /* TODO: Wrapper types for `WIN32_FIND_DATA' */

    /* Error-related functions. */
    LIBWIN32_GETLASTERROR_DEF
    LIBWIN32_SETLASTERROR_DEF

    /* Handle-related functions. */
    LIBWIN32_CLOSEHANDLE_DEF
    LIBWIN32_CREATEFILE_DEF_DOC("Returns :none upon error (s.a. #GetLastError)")
    LIBWIN32_DUPLICATEHANDLE_DEF_DOC("Returns :none upon error (s.a. #GetLastError)")
    LIBWIN32_READFILE_DEF_DOC("Returns :none upon error, or dwNumberOfBytesRead upon success (s.a. #GetLastError)")
    LIBWIN32_WRITEFILE_DEF_DOC("Returns :none upon error, or dwNumberOfBytesWritten upon success (s.a. #GetLastError)")
    LIBWIN32_SETENDOFFILE_DEF
    LIBWIN32_SETFILEPOINTER_DEF_DOC("Returns :none upon error, or the new stream position upon success (s.a. #GetLastError)")
    LIBWIN32_GETFILETIME_DEF_DOC("REturns a tuple (CreationTime,LastAccessTime,LastWriteTime) for the given @hFile")
    LIBWIN32_SETFILETIME_DEF
    LIBWIN32_SETFILEVALIDDATA_DEF
    LIBWIN32_GETDISKFREESPACE_DEF_DOC("Returns :none upon error, or a tuple (uSectorsPerCluster,uBytesPerSector,uNumberOfFreeClusters,uTotalNumberOfClusters) (s.a. #GetLastError)")
    LIBWIN32_GETDISKFREESPACEEX_DEF_DOC("Returns :none upon error, or a tuple (uFreeBytesAvailableToCaller,uTotalNumberOfBytes,uTotalNumberOfFreeBytes) (s.a. #GetLastError)")
    LIBWIN32_GETTEMPPATH_DEF_DOC("Returns :none upon error, or a string containing a temporary path (s.a. #GetLastError)")
    LIBWIN32_GETDLLDIRECTORY_DEF_DOC("Returns :none upon error, or a string describing the windows DLL directory (s.a. #GetLastError)")
    LIBWIN32_SETDLLDIRECTORY_DEF_DOC("Set the windows DLL directory, as used when loading dynamic libraries, and as returned by #GetDllDirectory")
    LIBWIN32_GETFILETYPE_DEF_DOC("Return one of `FILE_TYPE_*' or :none upon error (s.a. #GetLastError)")
    LIBWIN32_GETFILESIZE_DEF_DOC("Return the size of the given @hFile or :none upon error (s.a. #GetLastError)")
    LIBWIN32_GETDRIVETYPE_DEF_DOC("Returns the type of drive of @lpRootPathName (one of `DRIVE_*'), or :none upon error (s.a. #GetLastError)")
    LIBWIN32_GETFILEATTRIBUTES_DEF_DOC("Returns attributes for @lpFileName (set of `FILE_ATTRIBUTE_*'), or :none upon error (s.a. #GetLastError)")
    LIBWIN32_SETFILEATTRIBUTES_DEF_DOC("Set attributes for @lpFileName to @dwFileAttributes (set of `FILE_ATTRIBUTE_*')")
    LIBWIN32_GETCOMPRESSEDFILESIZE_DEF
    LIBWIN32_FLUSHFILEBUFFERS_DEF
    /* TODO: GetStdHandle */
    /* TODO: SetStdHandle */
    /* TODO: STD_INPUT_HANDLE */
    /* TODO: STD_OUTPUT_HANDLE */
    /* TODO: STD_ERROR_HANDLE */

    /* Filesystem functions */
    LIBWIN32_REMOVEDIRECTORY_DEF
    LIBWIN32_CREATEDIRECTORY_DEF
    LIBWIN32_DELETEFILE_DEF
    LIBWIN32_GETSYSTEMDIRECTORY_DEF_DOC("Returns :none upon error, the windows system directory ($\"C:\\\\Windows\\\\system32\") (s.a. #GetLastError)")
    LIBWIN32_GETWINDOWSDIRECTORY_DEF_DOC("Returns :none upon error, the windows system directory ($\"C:\\\\Windows\") (s.a. #GetLastError)")
    LIBWIN32_GETSYSTEMWINDOWSDIRECTORY_DEF_DOC("Returns :none upon error, the system windows directory ($\"C:\\\\Windows\") (s.a. #GetLastError)")
    LIBWIN32_GETSYSTEMWOW64DIRECTORY_DEF_DOC("Returns :none upon error, the windows SysWOW64 directory ($\"C:\\\\Windows\\\\SysWOW64\") (s.a. #GetLastError)")
    LIBWIN32_GETLOGICALDRIVESTRINGS_DEF_DOC("Returns :none upon error, or a $\"\\0\"-seperated list of all of the known system drives ($\"C:\\\\;D:\\\\;E:\\\\\") (s.a. #GetLastError)")

    /* DLL functions */
    LIBWIN32_GETMODULEFILENAME_DEF_DOC("Returns :none upon error, or the name of the module (s.a. #GetLastError)")

    /* Constant flags. */
    LIBWIN32_FILE_READ_DATA_DEF
    LIBWIN32_FILE_LIST_DIRECTORY_DEF
    LIBWIN32_FILE_WRITE_DATA_DEF
    LIBWIN32_FILE_ADD_FILE_DEF
    LIBWIN32_FILE_APPEND_DATA_DEF
    LIBWIN32_FILE_ADD_SUBDIRECTORY_DEF
    LIBWIN32_FILE_CREATE_PIPE_INSTANCE_DEF
    LIBWIN32_FILE_READ_EA_DEF
    LIBWIN32_FILE_WRITE_EA_DEF
    LIBWIN32_FILE_EXECUTE_DEF
    LIBWIN32_FILE_TRAVERSE_DEF
    LIBWIN32_FILE_DELETE_CHILD_DEF
    LIBWIN32_FILE_READ_ATTRIBUTES_DEF
    LIBWIN32_FILE_WRITE_ATTRIBUTES_DEF
    LIBWIN32_FILE_ALL_ACCESS_DEF
    LIBWIN32_FILE_GENERIC_READ_DEF
    LIBWIN32_FILE_GENERIC_WRITE_DEF
    LIBWIN32_FILE_GENERIC_EXECUTE_DEF
    LIBWIN32_FILE_SHARE_READ_DEF
    LIBWIN32_FILE_SHARE_WRITE_DEF
    LIBWIN32_FILE_SHARE_DELETE_DEF
    LIBWIN32_FILE_ATTRIBUTE_READONLY_DEF
    LIBWIN32_FILE_ATTRIBUTE_HIDDEN_DEF
    LIBWIN32_FILE_ATTRIBUTE_SYSTEM_DEF
    LIBWIN32_FILE_ATTRIBUTE_DIRECTORY_DEF
    LIBWIN32_FILE_ATTRIBUTE_ARCHIVE_DEF
    LIBWIN32_FILE_ATTRIBUTE_DEVICE_DEF
    LIBWIN32_FILE_ATTRIBUTE_NORMAL_DEF
    LIBWIN32_FILE_ATTRIBUTE_TEMPORARY_DEF
    LIBWIN32_FILE_ATTRIBUTE_SPARSE_FILE_DEF
    LIBWIN32_FILE_ATTRIBUTE_REPARSE_POINT_DEF
    LIBWIN32_FILE_ATTRIBUTE_COMPRESSED_DEF
    LIBWIN32_FILE_ATTRIBUTE_OFFLINE_DEF
    LIBWIN32_FILE_ATTRIBUTE_NOT_CONTENT_INDEXED_DEF
    LIBWIN32_FILE_ATTRIBUTE_ENCRYPTED_DEF
    LIBWIN32_FILE_ATTRIBUTE_INTEGRITY_STREAM_DEF
    LIBWIN32_FILE_ATTRIBUTE_VIRTUAL_DEF
    LIBWIN32_FILE_ATTRIBUTE_NO_SCRUB_DATA_DEF
    LIBWIN32_FILE_ATTRIBUTE_EA_DEF
    LIBWIN32_FILE_NOTIFY_CHANGE_FILE_NAME_DEF
    LIBWIN32_FILE_NOTIFY_CHANGE_DIR_NAME_DEF
    LIBWIN32_FILE_NOTIFY_CHANGE_ATTRIBUTES_DEF
    LIBWIN32_FILE_NOTIFY_CHANGE_SIZE_DEF
    LIBWIN32_FILE_NOTIFY_CHANGE_LAST_WRITE_DEF
    LIBWIN32_FILE_NOTIFY_CHANGE_LAST_ACCESS_DEF
    LIBWIN32_FILE_NOTIFY_CHANGE_CREATION_DEF
    LIBWIN32_FILE_NOTIFY_CHANGE_SECURITY_DEF
    LIBWIN32_FILE_ACTION_ADDED_DEF
    LIBWIN32_FILE_ACTION_REMOVED_DEF
    LIBWIN32_FILE_ACTION_MODIFIED_DEF
    LIBWIN32_FILE_ACTION_RENAMED_OLD_NAME_DEF
    LIBWIN32_FILE_ACTION_RENAMED_NEW_NAME_DEF
    LIBWIN32_FILE_CASE_SENSITIVE_SEARCH_DEF
    LIBWIN32_FILE_CASE_PRESERVED_NAMES_DEF
    LIBWIN32_FILE_UNICODE_ON_DISK_DEF
    LIBWIN32_FILE_PERSISTENT_ACLS_DEF
    LIBWIN32_FILE_FILE_COMPRESSION_DEF
    LIBWIN32_FILE_VOLUME_QUOTAS_DEF
    LIBWIN32_FILE_SUPPORTS_SPARSE_FILES_DEF
    LIBWIN32_FILE_SUPPORTS_REPARSE_POINTS_DEF
    LIBWIN32_FILE_SUPPORTS_REMOTE_STORAGE_DEF
    LIBWIN32_FILE_VOLUME_IS_COMPRESSED_DEF
    LIBWIN32_FILE_SUPPORTS_OBJECT_IDS_DEF
    LIBWIN32_FILE_SUPPORTS_ENCRYPTION_DEF
    LIBWIN32_FILE_NAMED_STREAMS_DEF
    LIBWIN32_FILE_READ_ONLY_VOLUME_DEF
    LIBWIN32_FILE_SEQUENTIAL_WRITE_ONCE_DEF
    LIBWIN32_FILE_SUPPORTS_TRANSACTIONS_DEF
    LIBWIN32_FILE_SUPPORTS_HARD_LINKS_DEF
    LIBWIN32_FILE_SUPPORTS_EXTENDED_ATTRIBUTES_DEF
    LIBWIN32_FILE_SUPPORTS_OPEN_BY_FILE_ID_DEF
    LIBWIN32_FILE_SUPPORTS_USN_JOURNAL_DEF
    LIBWIN32_FILE_SUPPORTS_INTEGRITY_STREAMS_DEF
    LIBWIN32_CREATE_NEW_DEF
    LIBWIN32_CREATE_ALWAYS_DEF
    LIBWIN32_OPEN_EXISTING_DEF
    LIBWIN32_OPEN_ALWAYS_DEF
    LIBWIN32_TRUNCATE_EXISTING_DEF
    LIBWIN32_FILE_BEGIN_DEF
    LIBWIN32_FILE_CURRENT_DEF
    LIBWIN32_FILE_END_DEF
    LIBWIN32_FILE_TYPE_UNKNOWN_DEF
    LIBWIN32_FILE_TYPE_DISK_DEF
    LIBWIN32_FILE_TYPE_CHAR_DEF
    LIBWIN32_FILE_TYPE_PIPE_DEF
    LIBWIN32_FILE_TYPE_REMOTE_DEF
    LIBWIN32_DRIVE_UNKNOWN_DEF
    LIBWIN32_DRIVE_NO_ROOT_DIR_DEF
    LIBWIN32_DRIVE_REMOVABLE_DEF
    LIBWIN32_DRIVE_FIXED_DEF
    LIBWIN32_DRIVE_REMOTE_DEF
    LIBWIN32_DRIVE_CDROM_DEF
    LIBWIN32_DRIVE_RAMDISK_DEF

    { NULL }
};

PUBLIC struct dex DEX = {
    /* .d_symbols = */symbols
};

DECL_END

#else /* CONFIG_HOST_WINDOWS */
DECL_BEGIN
PRIVATE struct dex_symbol symbols[] = { { NULL } };
PUBLIC struct dex DEX = { /* .d_symbols = */symbols };
DECL_END
#endif /* CONFIG_HOST_WINDOWS */

#endif /* !GUARD_DEX_WIN32_LIBWIN32_C */
