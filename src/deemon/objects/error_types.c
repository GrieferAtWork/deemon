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
#ifndef GUARD_DEEMON_OBJECTS_ERROR_TYPES_C
#define GUARD_DEEMON_OBJECTS_ERROR_TYPES_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/compiler/tpp.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/error_types.h>
#include <deemon/exec.h>
#include <deemon/format.h>
#include <deemon/kwds.h>
#include <deemon/module.h>
#include <deemon/none-operator.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/struct.h>
#include <deemon/system-features.h> /* bzero(), ... */
#include <deemon/system.h>
#include <deemon/tuple.h>

#include <hybrid/typecore.h>
/**/

#include <stddef.h> /* size_t, offsetof */
/**/

#include "../runtime/kwlist.h"
#include "../runtime/strings.h"

#undef token
#undef tok
#undef yield
#undef yieldnb
#undef yieldnbif
#undef skip

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

DECL_BEGIN

/* BEGIN::SystemError */
PRIVATE struct type_member tpconst systemerror_class_members[] = {
	TYPE_MEMBER_CONST("UnsupportedAPI", &DeeError_UnsupportedAPI),
	TYPE_MEMBER_CONST("FSError", &DeeError_FSError),
	TYPE_MEMBER_CONST_DOC("IOError", &DeeError_FSError, "Deprecated alias for ?#FSError"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
systemerror_init_kw(DeeSystemErrorObject *__restrict self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
#if defined(CONFIG_HOST_WINDOWS) || defined(__DEEMON__)
	DWORD dwLastError = GetLastError();
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("SystemError", params: """
	DeeObject *msg:?Dstring = NULL;
	DeeObject *cause:?DError = NULL;
	DeeObject *errno:?X2?Dint?Dstring = NULL;
	DeeObject *nterr_np:?Dint = NULL;
""");]]]*/
	struct {
		DeeObject *msg;
		DeeObject *cause;
		DeeObject *errno_;
		DeeObject *nterr_np;
	} args;
	args.msg = NULL;
	args.cause = NULL;
	args.errno_ = NULL;
	args.nterr_np = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__msg_cause_errno_nterr_np, "|oooo:SystemError", &args))
		goto err;
/*[[[end]]]*/
	if (args.nterr_np) {
		if (DeeObject_AsUInt32(args.nterr_np, &self->se_lasterror))
			goto err;
		if (args.errno_) {
			if (DeeObject_AsInt(args.errno_, &self->se_errno))
				goto err;
		} else {
			self->se_errno = DeeNTSystem_TranslateErrno(self->se_lasterror);
		}
	} else if (args.errno_) {
		if (DeeObject_AsInt(args.errno_, &self->se_errno))
			goto err;
		self->se_lasterror = DeeNTSystem_TranslateNtError(self->se_errno);
	} else {
		self->se_lasterror = dwLastError;
		self->se_errno     = DeeNTSystem_TranslateErrno(dwLastError);
	}
#endif /* CONFIG_HOST_WINDOWS */
#if !defined(CONFIG_HOST_WINDOWS) || defined(__DEEMON__)
	int last_errno = DeeSystem_GetErrno();
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("SystemError", params: """
	DeeObject *msg:?Dstring = NULL;
	DeeObject *cause:?DError = NULL;
	DeeObject *errno:?X2?Dint?Dstring = NULL;
""");]]]*/
	struct {
		DeeObject *msg;
		DeeObject *cause;
		DeeObject *errno_;
	} args;
	args.msg = NULL;
	args.cause = NULL;
	args.errno_ = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__msg_cause_errno, "|ooo:SystemError", &args))
		goto err;
/*[[[end]]]*/
	if (args.errno_) {
		if (DeeObject_AsInt(args.errno_, &self->se_errno))
			goto err;
	} else {
		self->se_errno = last_errno;
	}
#endif /* !CONFIG_HOST_WINDOWS */
	self->e_msg = args.msg;
	self->e_cause   = args.cause;
	Dee_XIncref(self->e_msg);
	Dee_XIncref(self->e_cause);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_call_posix_function(DeeSystemErrorObject *__restrict self,
                                char const *__restrict name) {
	return DeeModule_CallExternStringf("posix", name, "d", self->se_errno);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_getstrerrorname(DeeSystemErrorObject *__restrict self) {
	return systemerror_call_posix_function(self, "strerrorname");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_getstrerror(DeeSystemErrorObject *__restrict self) {
	return systemerror_call_posix_function(self, "strerror");
}

#ifdef CONFIG_HOST_WINDOWS
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
systemerror_getnterrmsg_np(DeeSystemErrorObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeNTSystem_FormatErrorMessage(self->se_lasterror);
	ASSERT(!result || DeeString_Check(result));
	if (result && DeeString_IsEmpty(result)) {
		Dee_Decref_unlikely(result);
		result = DeeNone_NewRef();
	}
	return result;
}
#endif /* CONFIG_HOST_WINDOWS */

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
systemerror_print(DeeSystemErrorObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	DREF DeeObject *errno_name, *errno_desc;
	result = DeeObject_Print(self->e_msg
	                         ? self->e_msg
	                         : (DeeObject *)Dee_TYPE(self),
	                         printer, arg);
	if unlikely(result < 0)
		goto done;
	if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
		errno_name = systemerror_getstrerrorname(self);
		if unlikely(!errno_name) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL)) {
				temp = -1;
				goto err;
			}
			errno_name = DeeNone_NewRef();
		}
		errno_desc = systemerror_getstrerror(self);
		if unlikely(!errno_desc) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL)) {
				temp = -1;
				goto err_errno_name;
			}
			errno_desc = DeeNone_NewRef();
		}
		DO(err_errno_desc, DeeFormat_Printf(printer, arg, "\nerrno(%d)", self->se_errno));
		if (!DeeNone_Check(errno_name) || !DeeNone_Check(errno_desc)) {
#ifdef CONFIG_HOST_WINDOWS
			if (self->se_lasterror != NO_ERROR) {
				size_t unix_code_length;
				size_t windows_code_length;
				unix_code_length    = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%d", self->se_errno);
				windows_code_length = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%#I32x", self->se_lasterror);
				if (windows_code_length > unix_code_length) {
					DO(err_errno_desc, DeeFormat_Repeat(printer, arg, ' ',
					                                    windows_code_length -
					                                    unix_code_length));
				}
			}
#endif /* CONFIG_HOST_WINDOWS */
			DO(err_errno_desc, DeeFormat_PRINT(printer, arg, ": "));
			DO(err_errno_desc,
			   DeeNone_Check(errno_name) ? DeeFormat_PRINT(printer, arg, "?")
			                             : DeeFormat_PrintObject(printer, arg, errno_name));
			if (!DeeNone_Check(errno_desc)) {
				DO(err_errno_desc, DeeFormat_PRINT(printer, arg, " ("));
				DO(err_errno_desc, DeeFormat_PrintObject(printer, arg, errno_desc));
				DO(err_errno_desc, DeeFormat_PRINT(printer, arg, ")"));
			}
		}
		Dee_Decref(errno_desc);
		Dee_Decref(errno_name);
	}

#ifdef CONFIG_HOST_WINDOWS
	if (self->se_lasterror != NO_ERROR) {
		bool success;
		DO(err, DeeFormat_Printf(printer, arg, "\nnterr(%#I32x)", self->se_lasterror));
		if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
			size_t unix_code_length;
			size_t windows_code_length;
			unix_code_length    = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%d", self->se_errno);
			windows_code_length = (size_t)(uintptr_t)Dee_snprintf(NULL, 0, "%#I32x", self->se_lasterror);
			if (unix_code_length > windows_code_length) {
				DO(err, DeeFormat_Repeat(printer, arg, ' ',
				                         unix_code_length -
				                         windows_code_length));
			}
		}
		DO(err, DeeFormat_PRINT(printer, arg, ": "));
		DO(err, DeeNTSystem_PrintFormatMessage(printer, arg, FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		                                       (DWORD)self->se_lasterror,
		                                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		                                       NULL, &success));
		if unlikely(!success) {
			/* No msg string printed... */
			DO(err, DeeFormat_PRINT(printer, arg, "?"));
		}
	}
#endif /* CONFIG_HOST_WINDOWS */
done:
	return result;
err_errno_desc:
	Dee_Decref(errno_desc);
err_errno_name:
	Dee_Decref(errno_name);
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
systemerror_printrepr(DeeSystemErrorObject *__restrict self, Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	bool is_first = true;
	result = DeeFormat_Printf(printer, arg, "%k(", Dee_TYPE(self));
	if unlikely(result < 0)
		goto done;
	if (self->e_msg) {
		DO(err, DeeString_PrintRepr(self->e_msg, printer, arg));
		is_first = false;
	}
	if (self->e_cause) {
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "cause: %r", self->e_cause));
		is_first = false;
	}
	if (self->se_errno != Dee_SYSTEM_ERROR_UNKNOWN) {
		DREF DeeObject *errno_name;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_PRINT(printer, arg, "errno: "));
		errno_name = systemerror_getstrerrorname(self);
		if (!errno_name) {
			if (!DeeError_Handled(ERROR_HANDLED_NORMAL))
				goto err;
			errno_name = DeeNone_NewRef();
		}
		temp = DeeNone_Check(errno_name)
		       ? DeeFormat_Printf(printer, arg, "%d", self->se_errno)
		       : DeeFormat_Printf(printer, arg, "posix.%k", errno_name);
		Dee_Decref(errno_name);
		if unlikely(temp < 0)
			goto err;
		result += temp;
#ifdef CONFIG_HOST_WINDOWS
		is_first = false;
#endif /* CONFIG_HOST_WINDOWS */
	}
#ifdef CONFIG_HOST_WINDOWS
	if (self->se_lasterror != NO_ERROR) {
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "nterr_np: %#I32x", self->se_lasterror));
	}
#endif /* CONFIG_HOST_WINDOWS */
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err:
	return temp;
}

#undef DO

PRIVATE struct type_getset tpconst systemerror_getsets[] = {
	TYPE_GETTER_AB_F("strerrorname", &systemerror_getstrerrorname, METHOD_FNOREFESCAPE,
	                 "->?Dstring\n"
	                 "The name of the associated ?#errno (s.a. ?Eposix:strerrorname)\n"
	                 "Returns ?N if ?#errno doesn't have a known name"),
	TYPE_GETTER_AB_F("strerror", &systemerror_getstrerror, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "A description of the associated ?#errno (s.a. ?Eposix:strerror)\n"
	                 "Returns ?N if ?#errno doesn't have a description"),
#ifdef CONFIG_HOST_WINDOWS
	TYPE_GETTER_AB_F("nterrmsg_np", &systemerror_getnterrmsg_np, METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "A description of the associated ?#nterr_np (s.a. ?Ewin32:FormatErrorMessage)\n"
	                 "Returns ?N if no msg description is available"),
#endif /* CONFIG_HOST_WINDOWS */
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst systemerror_members[] = {
	TYPE_MEMBER_FIELD_DOC("errno", STRUCT_CONST | STRUCT_INT,
	                      offsetof(DeeSystemErrorObject, se_errno),
	                      "The associated system errno value (one of #C{E*} from ?R!Mposix])"),
#ifdef CONFIG_HOST_WINDOWS
	TYPE_MEMBER_FIELD_DOC("nterr_np", STRUCT_CONST | STRUCT_UINT32_T,
	                      offsetof(DeeSystemErrorObject, se_lasterror),
	                      "The windows-specific error code, as returned by ?Ewin32:GetLastError"),
#endif /* CONFIG_HOST_WINDOWS */
	TYPE_MEMBER_END
};

#ifdef CONFIG_HOST_WINDOWS
#define SystemError_init_params "msg?:?Dstring,cause?:?DError,errno?:?X2?Dint?Dstring,nterr_np?:?Dint"
#else /* CONFIG_HOST_WINDOWS */
#define SystemError_init_params "msg?:?Dstring,cause?:?DError,errno?:?X2?Dint?Dstring"
#endif /* !CONFIG_HOST_WINDOWS */

PUBLIC DeeTypeObject DeeError_SystemError = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "SystemError",
	/* .tp_doc      = */ DOC("(" SystemError_init_params ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeError_Error,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeSystemErrorObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &systemerror_init_kw,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&systemerror_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&systemerror_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeStructObject_Cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ systemerror_getsets,
	/* .tp_members       = */ systemerror_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ systemerror_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

#define INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS(tp_name, tp_doc, tp_flags,                \
                                               tp_base, T, tp_str, tp_print,             \
                                               tp_methods, tp_getsets, tp_class_members) \
	{                                                                                    \
		OBJECT_HEAD_INIT(&DeeType_Type),                                                 \
		/* .tp_name     = */ tp_name,                                                    \
		/* .tp_doc      = */ DOC(tp_doc),                                                \
		/* .tp_flags    = */ TP_FNORMAL | (tp_flags),                                    \
		/* .tp_weakrefs = */ 0,                                                          \
		/* .tp_features = */ TF_NONE,                                                    \
		/* .tp_base     = */ tp_base,                                                    \
		/* .tp_init = */ {                                                               \
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(                                             \
				/* T:              */ T,                                                 \
				/* tp_ctor:        */ NULL,                                              \
				/* tp_copy_ctor:   */ NULL,                                              \
				/* tp_deep_ctor:   */ NULL,                                              \
				/* tp_any_ctor:    */ NULL,                                              \
				/* tp_any_ctor_kw: */ &systemerror_init_kw,                              \
				/* tp_serialize:   */ NULL /* TODO */                                    \
			),                                                                           \
			/* .tp_dtor        = */ NULL,                                                \
			/* .tp_assign      = */ NULL,                                                \
			/* .tp_move_assign = */ NULL,                                                \
		},                                                                               \
		/* .tp_cast = */ {                                                               \
			/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
			/* .tp_repr      = */ NULL,                                                  \
			/* .tp_bool      = */ NULL,                                                  \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&systemerror_printrepr, \
		},                                                                               \
		/* .tp_visit         = */ NULL,                                                  \
		/* .tp_gc            = */ NULL,                                                  \
		/* .tp_math          = */ NULL,                                                  \
		/* .tp_cmp           = */ &DeeStructObject_Cmp,                                  \
		/* .tp_seq           = */ NULL,                                                  \
		/* .tp_iter_next     = */ NULL,                                                  \
		/* .tp_iterator      = */ NULL,                                                  \
		/* .tp_attr          = */ NULL,                                                  \
		/* .tp_with          = */ NULL,                                                  \
		/* .tp_buffer        = */ NULL,                                                  \
		/* .tp_methods       = */ tp_methods,                                            \
		/* .tp_getsets       = */ tp_getsets,                                            \
		/* .tp_members       = */ NULL,                                                  \
		/* .tp_class_methods = */ NULL,                                                  \
		/* .tp_class_getsets = */ NULL,                                                  \
		/* .tp_class_members = */ tp_class_members,                                      \
	}                                                                                    \

PUBLIC DeeTypeObject DeeError_UnsupportedAPI = /* TODO: Include name of unsupported API */
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("UnsupportedAPI", "(" SystemError_init_params ")",
                                       TP_FNORMAL, &DeeError_SystemError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);

PRIVATE struct type_member tpconst FSError_class_members[] = {
	TYPE_MEMBER_CONST("FileAccessError", &DeeError_FileAccessError),
	TYPE_MEMBER_CONST("FileNotFound", &DeeError_FileNotFound),
	TYPE_MEMBER_CONST("FileExists", &DeeError_FileExists),
	TYPE_MEMBER_CONST("FileClosed", &DeeError_FileClosed),
	TYPE_MEMBER_CONST("CrossDeviceLink", &DeeError_CrossDeviceLink),
	TYPE_MEMBER_CONST("BusyFile", &DeeError_BusyFile),
	TYPE_MEMBER_END
};
#define FSError_init_params SystemError_init_params
PUBLIC DeeTypeObject DeeError_FSError = /* TODO: Include filename of affected file */
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FSError", "(" FSError_init_params ")",
                                       TP_FNORMAL, &DeeError_SystemError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, FSError_class_members);


PRIVATE struct type_member tpconst FileAccessError_class_members[] = {
	TYPE_MEMBER_CONST("ReadOnlyFile", &DeeError_ReadOnlyFile),
	TYPE_MEMBER_END
};
#define FileAccessError_init_params FSError_init_params
PUBLIC DeeTypeObject DeeError_FileAccessError =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FileAccessError",
                                       "(" FileAccessError_init_params ")\n"
                                       "An error derived from :FSError that is thrown when attempting "
                                       /**/ "to access a file or directory without the necessary permissions",
                                       TP_FNORMAL, &DeeError_SystemError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, FileAccessError_class_members);

PUBLIC DeeTypeObject DeeError_ReadOnlyFile =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("ReadOnlyFile",
                                       "(" FileAccessError_init_params ")\n"
                                       "An error derived from :FileAccessError that is thrown when attempting "
                                       /**/ "to modify a file or directory when it or the filesystem is read-only",
                                       TP_FNORMAL, &DeeError_FileAccessError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);

PRIVATE struct type_member tpconst FileNotFound_class_members[] = {
	TYPE_MEMBER_CONST("NoDirectory", &DeeError_NoDirectory),
	TYPE_MEMBER_CONST("NoSymlink", &DeeError_NoSymlink),
	TYPE_MEMBER_END
};
#define FileNotFound_init_params FSError_init_params
PUBLIC DeeTypeObject DeeError_FileNotFound =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FileNotFound", "(" FileNotFound_init_params ")",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, FileNotFound_class_members);

PRIVATE struct type_member tpconst FileExists_class_members[] = {
	TYPE_MEMBER_CONST("IsDirectory", &DeeError_IsDirectory),
	TYPE_MEMBER_CONST("DirectoryNotEmpty", &DeeError_DirectoryNotEmpty),
	TYPE_MEMBER_END
};
#define FileExists_init_params FSError_init_params
PUBLIC DeeTypeObject DeeError_FileExists =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FileExists",
                                       "(" FileExists_init_params ")\n"
                                       "An error derived from :FSError that is thrown when attempting "
                                       /**/ "to create a filesystem object when the target path already exists",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, FileExists_class_members);

PUBLIC DeeTypeObject DeeError_FileClosed =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("FileClosed", "(" FSError_init_params ")",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_NoDirectory =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("NoDirectory",
                                       "(" FileNotFound_init_params ")\n"
                                       "An error derived from :FileNotFound that is thrown when a "
                                       /**/ "directory was expected, but something different was found",
                                       TP_FNORMAL, &DeeError_FileNotFound, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_IsDirectory =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("IsDirectory",
                                       "(" FileExists_init_params ")\n"
                                       "An error derived from :FileExists that is thrown when something "
                                       /**/ "other than a directory was expected, but one was found none-the-less",
                                       TP_FNORMAL, &DeeError_FileExists, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_CrossDeviceLink =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("CrossDeviceLink",
                                       "(" FSError_init_params ")\n"
                                       "An error derived from :FSError that is thrown when attempting "
                                       /**/ "to move a file between different devices or partitions",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_DirectoryNotEmpty =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("DirectoryNotEmpty",
                                       "(" FileExists_init_params ")\n"
                                       "An error derived from :FileExists that is thrown when "
                                       /**/ "attempting to remove a directory that isn't empty",
                                       TP_FNORMAL, &DeeError_FileExists, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_BusyFile =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("BusyFile",
                                       "(" FSError_init_params ")\n"
                                       "An error derived from :FSError that is thrown when "
                                       /**/ "attempting to remove a file or directory that is being "
                                       /**/ "used by another process",
                                       TP_FNORMAL, &DeeError_FSError, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
PUBLIC DeeTypeObject DeeError_NoSymlink =
INIT_CUSTOM_SYSTEM_ERROR_NO_NEW_FIELDS("NoSymlink",
                                       "(" FileNotFound_init_params ")\n"
                                       "An error derived from :FileNotFound that is thrown when attempting "
                                       /**/ "to invoke ?Eposix:readlink on a file that isn't a symbolic link",
                                       TP_FNORMAL, &DeeError_FileNotFound, DeeSystemErrorObject,
                                       NULL, NULL, NULL, NULL, NULL);
/* END::SystemError */



















/************************************************************************/
/* SPECIAL CASE: AppExit                                                */
/************************************************************************/

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif /* !EXIT_SUCCESS */

PRIVATE int DCALL
appexit_init(struct appexit_object *__restrict self,
             size_t argc, DeeObject *const *argv) {
	int result;
	self->ae_exitcode = EXIT_SUCCESS;
	/* Read the exitcode from arguments. */
	result = DeeArg_UnpackStruct(argc, argv,
	                             "|d:appexit",
	                             &self->ae_exitcode);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
appexit_copy(struct appexit_object *__restrict self,
             struct appexit_object *__restrict other) {
	self->ae_exitcode = other->ae_exitcode;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
appexit_print(struct appexit_object *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<AppExit with exitcode %d>", self->ae_exitcode);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
appexit_printrepr(struct appexit_object *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "AppExit(%d)", self->ae_exitcode);
}

PRIVATE struct type_member tpconst appexit_members[] = {
	TYPE_MEMBER_FIELD("exitcode", STRUCT_CONST | STRUCT_INT,
	                  offsetof(struct appexit_object, ae_exitcode)),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
appexit_class_atexit(DeeTypeObject *UNUSED(self),
                     size_t argc, DeeObject *const *argv) {
	DeeObject *callback, *args = Dee_EmptyTuple;
	DeeArg_Unpack1Or2(err, argc, argv, "atexit", &callback, &args);
	if (DeeObject_AssertTypeExact(args, &DeeTuple_Type))
		goto err;
	if (Dee_AtExit(callback, args))
		goto err;
	return_none;
err:
	return NULL;
}

/* Terminate the application the same way `deemon.Error.AppExit.exit()' would,
 * either through use of `exit()' from <stdlib.h>, or by throwing an exception.
 * NOTE: When available, calling stdlib's `exit()' is identical to this.
 * @return: -1: If this function returns at all, it always returns `-1' */
PUBLIC int DCALL Dee_Exit(int exitcode, bool run_atexit) {
	(void)exitcode;
	(void)run_atexit;
	COMPILER_IMPURE();
#ifdef CONFIG_HAVE__Exit
#ifdef CONFIG_HAVE_exit
	if (run_atexit)
		exit(exitcode);
#endif /* CONFIG_HAVE_exit */
	_Exit(exitcode);
#else /* CONFIG_HAVE__Exit */
	/* If callbacks aren't supposed to be executed, discard
	 * all of them and prevent the addition of new ones. */
	if (!run_atexit)
		Dee_RunAtExit(DEE_RUNATEXIT_FDONTRUN);
#ifdef CONFIG_HAVE_exit
	exit(exitcode);
#else /* CONFIG_HAVE_exit */
	/* No stdlib support. Instead, we must throw an AppExit error. */
	{
		struct appexit_object *error;
		error = DeeObject_MALLOC(struct appexit_object);
		if likely(error) {
			/* Initialize the appexit error. */
			error->ae_exitcode = exitcode;
			DeeObject_Init(error, &DeeError_AppExit);
			/* Throw the appexit error. */
			DeeError_ThrowInherited((DeeObject *)error);
		}
	}
	return -1;
#endif /* !CONFIG_HAVE_exit */
#endif /* !CONFIG_HAVE__Exit */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
appexit_class_exit(DeeTypeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	struct {
		int exitcode;
		bool run_atexit;
	} args;
	args.exitcode   = EXIT_SUCCESS;
	args.run_atexit = true;
	if (DeeArg_UnpackStruct(argc, argv, "|db:exit", &args))
		goto err;
	Dee_Exit(args.exitcode, args.run_atexit);
err:
	return NULL;
}

PRIVATE struct type_method tpconst appexit_class_methods[] = {
	TYPE_METHOD("exit", &appexit_class_exit,
	            "()\n"
	            "(exitcode:?Dint,run_atexit=!t)\n"
	            "Terminate execution of deemon after invoking ?#atexit callbacks when @run_atexit is ?t\n"
	            "Termination is done using the C #Cexit or #C_exit functions, if available. However if these "
	            /**/ "functions are not provided by the host, an :AppExit error is thrown instead\n"
	            "When no @exitcode is given, the host's default default value of #CEXIT_SUCCESS, or $1 is used\n"
	            "This function never returns normally"),
	TYPE_METHOD("atexit", &appexit_class_atexit,
	            "(callback:?DCallable,args=!T0)\n"
	            "#tRuntimeError{Additional atexit-callbacks can no longer be registered}"
	            "#tNotImplemented{Deemon was built without support for ?#atexit}"
	            "Register a given @callback to be executed before deemon terminates"),
	TYPE_METHOD_END
};

/* A very special error type that doesn't actually derive from
 * `Error', or even `object' for that matter. It does however
 * have the `TP_FINTERRUPT' flag set, meaning that it can only
 * be caught by interrupt-enabled exception handlers.
 *
 * The main purpose of this error is to allow user-code to throw
 * it (the type is accessible as `(Error from deemon).AppExit'),
 * while also providing for proper stack unwinding and correct
 * destruction of all existing objects.
 *
 * The implementation's main() function should then terminate by
 * returning the contained `ae_exitcode' value. Note that this
 * type is final, meaning that user-classes cannot be further
 * derived from it.
 *
 * Additionally, this type of error is used by the builtin
 * implementation of `exit()' when deemon was built without
 * support for a native exit function. */
PUBLIC DeeTypeObject DeeError_AppExit = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "AppExit",
	/* .tp_doc      = */ DOC("An AppExit object is a special kind of interrupt that "
	                         /**/ "is not derived from ?O, and has no base class at "
	                         /**/ "all. It's purpose is to allow user-code to throw an "
	                         /**/ "instance of it and have the stack unwind itself, alongside "
	                         /**/ "all existing objects being destroyed normally before deemon "
	                         /**/ "will be terminated with the given exitcode\n"
	                         "\n"

	                         "()\n"
	                         "(exitcode:?Dint)\n"
	                         "Construct a new AppExit object using the given @exitcode "
	                         /**/ "or the host's default value for #CEXIT_SUCCESS, or $0"),
	/* .tp_flags    = */ TP_FFINAL | TP_FINTERRUPT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ NULL, /* No base type (to only allow AppExit-interrupt handlers,
	                            * and all-interrupt handlers to catch this error) */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ struct appexit_object,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &appexit_copy,
			/* tp_deep_ctor:   */ &appexit_copy,
			/* tp_any_ctor:    */ &appexit_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str      = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr     = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool     = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_strrepr  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&appexit_print,
		/* .tp_reprrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&appexit_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeStructObject_Cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ appexit_members,
	/* .tp_class_methods = */ appexit_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};






/* Fallback instance when throwing no-memory errors with an unknown size. */
PUBLIC DeeNoMemoryErrorObject DeeError_NoMemory_instance = {
	OBJECT_HEAD_INIT(&DeeError_NoMemory),
	/* .e_msg        = */ Dee_AsObject(&str_nomemory),
	/* .e_cause      = */ NULL,
	/* .nm_allocsize = */ (size_t)-1
};








/* ==== Signal type subsystem ==== */
#define INIT_CUSTOM_SIGNAL(tp_name, tp_doc, tp_flags, tp_base,           \
                           tp_ctor, tp_copy, tp_deep, tp_init,           \
                           tp_init_kw, tp_serialize, tp_visit, T,        \
                           tp_str, tp_print, tp_repr, tp_printrepr,      \
                           tp_methods, tp_getsets, tp_members,           \
                           tp_class_members)                             \
	{                                                                    \
		OBJECT_HEAD_INIT(&DeeType_Type),                                 \
		/* .tp_name     = */ tp_name,                                    \
		/* .tp_doc      = */ DOC(tp_doc),                                \
		/* .tp_flags    = */ tp_flags,                                   \
		/* .tp_weakrefs = */ 0,                                          \
		/* .tp_features = */ TF_NONE | TF_TPVISIT,                       \
		/* .tp_base     = */ tp_base,                                    \
		/* .tp_init = */ {                                               \
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(                             \
				/* T:              */ T,                                 \
				/* tp_ctor:        */ tp_ctor,                           \
				/* tp_copy_ctor:   */ tp_copy,                           \
				/* tp_deep_ctor:   */ tp_deep,                           \
				/* tp_any_ctor:    */ tp_init,                           \
				/* tp_any_ctor_kw: */ tp_init_kw,                        \
				/* tp_serialize:   */ tp_serialize                       \
			),                                                           \
			/* .tp_dtor        = */ NULL,                                \
			/* .tp_assign      = */ NULL,                                \
			/* .tp_move_assign = */ NULL                                 \
		},                                                               \
		/* .tp_cast = */ {                                               \
			/* .tp_str       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_str), \
			/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))(tp_repr), \
			/* .tp_bool      = */ NULL,                                  \
			/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_print), \
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))(tp_printrepr),                                  \
		},                                                               \
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))(tp_visit), \
		/* .tp_gc            = */ NULL,                                  \
		/* .tp_math          = */ NULL,                                  \
		/* .tp_cmp           = */ &DeeStructObject_Cmp,                  \
		/* .tp_seq           = */ NULL,                                  \
		/* .tp_iter_next     = */ NULL,                                  \
		/* .tp_iterator      = */ NULL,                                  \
		/* .tp_attr          = */ NULL,                                  \
		/* .tp_with          = */ NULL,                                  \
		/* .tp_buffer        = */ NULL,                                  \
		/* .tp_methods       = */ tp_methods,                            \
		/* .tp_getsets       = */ tp_getsets,                            \
		/* .tp_members       = */ tp_members,                            \
		/* .tp_class_methods = */ NULL,                                  \
		/* .tp_class_getsets = */ NULL,                                  \
		/* .tp_class_members = */ tp_class_members                       \
	}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
signal_printrepr(DeeSignalObject *__restrict self,
                 Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%k()", Dee_TYPE(self));
}


/************************************************************************/
/* Signal.StopIteration                                                 */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_StopIteration =
INIT_CUSTOM_SIGNAL("StopIteration", NULL, TP_FNORMAL, &DeeError_Signal,
                   &DeeNone_OperatorCtor, &DeeNone_OperatorCopy, &DeeNone_OperatorCopy, NULL,
                   NULL, &DeeNone_OperatorWriteDec, NULL, DeeSignalObject,
                   NULL, NULL, NULL, &signal_printrepr,
                   NULL, NULL, NULL, NULL);

/************************************************************************/
/* Signal.Interrupt                                                     */
/************************************************************************/
PRIVATE struct type_member tpconst interrupt_class_members[] = {
	TYPE_MEMBER_CONST("KeyboardInterrupt", &DeeError_KeyboardInterrupt),
	TYPE_MEMBER_CONST("ThreadExit", &DeeError_ThreadExit),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_Interrupt =
INIT_CUSTOM_SIGNAL("Interrupt", NULL, TP_FNORMAL | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Signal,
                   &DeeNone_OperatorCtor, &DeeNone_OperatorCopy, &DeeNone_OperatorCopy, NULL,
                   NULL, &DeeNone_OperatorWriteDec, NULL, DeeSignalObject,
                   NULL, NULL, NULL, &signal_printrepr,
                   NULL, NULL, NULL, interrupt_class_members);


/************************************************************************/
/* Signal.Interrupt.ThreadExit                                          */
/************************************************************************/
PRIVATE struct type_member tpconst threadexit_members[] = {
	TYPE_MEMBER_FIELD("__result__", STRUCT_OBJECT, offsetof(struct threadexit_object, te_result)),
	TYPE_MEMBER_END
};
PUBLIC DeeTypeObject DeeError_ThreadExit =
INIT_CUSTOM_SIGNAL("ThreadExit", NULL, TP_FNORMAL | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Interrupt,
                   NULL, &DeeStructObject_Copy, &DeeStructObject_Deep, &DeeStructObject_Init,
                   &DeeStructObject_InitKw, &DeeStructObject_Serialize,
                   &DeeStructObject_Visit, struct threadexit_object,
                   NULL, NULL, NULL, &DeeStructObject_PrintRepr,
                   NULL, NULL, threadexit_members, interrupt_class_members);


/************************************************************************/
/* Signal.Interrupt.KeyboardInterrupt                                   */
/************************************************************************/
PUBLIC DeeTypeObject DeeError_KeyboardInterrupt =
INIT_CUSTOM_SIGNAL("KeyboardInterrupt", NULL, TP_FNORMAL | TP_FINTERRUPT /* Interrupt type! */, &DeeError_Interrupt,
                   &DeeNone_OperatorCtor, &DeeNone_OperatorCopy, &DeeNone_OperatorCopy, NULL,
                   NULL, &DeeNone_OperatorWriteDec, NULL, DeeSignalObject,
                   NULL, NULL, NULL, &signal_printrepr,
                   NULL, NULL, NULL, NULL);


/************************************************************************/
/* Signal                                                               */
/************************************************************************/
PRIVATE struct type_member tpconst signal_class_members[] = {
	TYPE_MEMBER_CONST("Interrupt", &DeeError_Interrupt),
	TYPE_MEMBER_CONST("StopIteration", &DeeError_StopIteration),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeError_Signal = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Signal),
	/* .tp_doc      = */ DOC("Base class for signaling exceptions\n"
	                         "\n"
	                         "()"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeSignalObject,
			/* tp_ctor:        */ &DeeNone_OperatorCtor,
			/* tp_copy_ctor:   */ &DeeNone_OperatorCopy,
			/* tp_deep_ctor:   */ &DeeNone_OperatorCopy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &DeeNone_OperatorWriteDec
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&signal_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &DeeStructObject_Cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ signal_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

PUBLIC DeeSignalObject DeeError_StopIteration_instance = {
	OBJECT_HEAD_INIT(&DeeError_StopIteration)
};

PUBLIC DeeSignalObject DeeError_Interrupt_instance = {
	OBJECT_HEAD_INIT(&DeeError_Interrupt)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ERROR_TYPES_C */
