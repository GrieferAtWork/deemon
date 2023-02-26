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
#ifndef GUARD_DEX_IPC_GENERIC_C_INL
#define GUARD_DEX_IPC_GENERIC_C_INL 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include "_res.h"
#include "libipc.h"

/**/
#include "generic-cmdline.c.inl"

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
} Process;

PRIVATE Process this_process = {
	OBJECT_HEAD_INIT(&DeeProcess_Type),
};

#ifndef IPC_UNIMPLEMENTED_DEFINED
#define IPC_UNIMPLEMENTED_DEFINED 1
PRIVATE ATTR_COLD int DCALL ipc_unimplemented(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "IPI functionality is not supported");
}
#endif /* !IPC_UNIMPLEMENTED_DEFINED */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_start(Process *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_start_name))
		goto err;
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_detach(Process *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_detach_name))
		goto err;
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_id(Process *__restrict self) {
	if (self == &this_process)
		return_reference_(&DeeInt_Zero);
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_terminate(Process *self, size_t argc, DeeObject *const *argv) {
	uint32_t exit_code = 0;
	if (DeeArg_Unpack(argc, argv, "|" UNPu32 ":" S_Process_function_terminate_name, &exit_code))
		goto err;
	if (self == &this_process) {
		Dee_Exit((int)exit_code, false);
		goto err;
	}
	ipc_unimplemented();
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_join(Process *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_join_name))
		goto err;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_tryjoin(Process *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":" S_Process_function_tryjoin_name))
		goto err;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_timedjoin(Process *self, size_t argc, DeeObject *const *argv) {
	uint64_t timeout_in_nanoseconds;
	if (DeeArg_Unpack(argc, argv,
	                  UNPd64 ":" S_Process_function_timedjoin_name,
	                  &timeout_in_nanoseconds))
		goto err;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_hasstarted(Process *__restrict self) {
	if (self == &this_process)
		return_true;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_wasdetached(Process *__restrict self) {
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_isachild(Process *self) {
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_wasterminated(Process *self) {
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}


PRIVATE struct type_method tpconst process_methods[] = {
	TYPE_METHOD(S_Process_function_start_name, &process_start, S_Process_function_start_doc),
	TYPE_METHOD(S_Process_function_join_name, &process_join, S_Process_function_join_doc),
	TYPE_METHOD(S_Process_function_tryjoin_name, &process_tryjoin, S_Process_function_tryjoin_doc),
	TYPE_METHOD(S_Process_function_timedjoin_name, &process_timedjoin, S_Process_function_timedjoin_doc),
	TYPE_METHOD(S_Process_function_detach_name, &process_detach, S_Process_function_detach_doc),
	TYPE_METHOD(S_Process_function_terminate_name, &process_terminate, S_Process_function_terminate_doc),
	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_files(Process *__restrict UNUSED(self)) {
	ipc_unimplemented();
	return NULL;
}

#define DEFINE_PROCESS_STD_FUNCTIONS(stdxxx, DEE_STDXXX)    \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL      \
	process_get_##stdxxx(Process *__restrict self) {        \
		if (self == &this_process)                          \
			return DeeFile_GetStd(DEE_STDXXX);              \
		ipc_unimplemented();                                \
		return NULL;                                        \
	}                                                       \
	PRIVATE WUNUSED NONNULL((1)) int DCALL                  \
	process_del_##stdxxx(Process *__restrict self) {        \
		if (self == &this_process) {                        \
			DREF DeeObject *oldval;                         \
			oldval = DeeFile_SetStd(DEE_STDXXX, NULL);      \
			if (ITER_ISOK(oldval))                          \
				Dee_Decref(oldval);                         \
			return 0;                                       \
		}                                                   \
		return ipc_unimplemented();                         \
	}                                                       \
	PRIVATE WUNUSED NONNULL((1, 2)) int DCALL               \
	process_set_##stdxxx(Process *self, DeeObject *value) { \
		if (self == &this_process) {                        \
			DREF DeeObject *oldval;                         \
			oldval = DeeFile_SetStd(DEE_STDXXX, value);     \
			if (ITER_ISOK(oldval))                          \
				Dee_Decref(oldval);                         \
			return 0;                                       \
		}                                                   \
		return ipc_unimplemented();                         \
	}
DEFINE_PROCESS_STD_FUNCTIONS(stdin, DEE_STDIN)
DEFINE_PROCESS_STD_FUNCTIONS(stdout, DEE_STDOUT)
DEFINE_PROCESS_STD_FUNCTIONS(stderr, DEE_STDERR)
#undef DEFINE_PROCESS_STD_FUNCTIONS

/*[[[deemon
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_posix", "posix");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_getcwd", "getcwd");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_chdir", "chdir");
(PRIVATE_DEFINE_STRING from rt.gen.string)("str_environ", "environ");
]]]*/
PRIVATE DEFINE_STRING_EX(str_posix, "posix", 0x8a12ee56, 0xfc8c64936b261e96);
PRIVATE DEFINE_STRING_EX(str_getcwd, "getcwd", 0x2a8d58b5, 0xd71ca646f3668c32);
PRIVATE DEFINE_STRING_EX(str_chdir, "chdir", 0xd4e07810, 0x5d1f1c76d494fde6);
PRIVATE DEFINE_STRING_EX(str_environ, "environ", 0xd8ecb380, 0x8d2a0a9c2432f221);
/*[[[end]]]*/

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_threads(Process *__restrict self) {
	if (self == &this_process)
		return DeeObject_GetAttrString((DeeObject *)&DeeThread_Type, "enumerate");
	ipc_unimplemented();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_environ(Process *__restrict self) {
	if (self == &this_process) {
		return DeeModule_GetExtern((DeeObject *)&str_posix,
		                           (DeeObject *)&str_environ);
	}
	ipc_unimplemented();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_memory(Process *__restrict self) {
#if 0
	if (self == &this_process)
		; /* TODO */
#endif
	ipc_unimplemented();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_environ(Process *self, DeeObject *value) {
	if (self == &this_process) {
		DREF DeeObject *temp;
		int result;
		temp = DeeModule_GetExtern((DeeObject *)&str_posix,
		                           (DeeObject *)&str_environ);
		if unlikely(!temp)
			goto err;
		result = DeeObject_Assign(temp, value ? value : Dee_None);
		Dee_Decref(temp);
		return result;
	}
	return ipc_unimplemented();
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_environ(Process *__restrict self) {
	return process_set_environ(self, NULL);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_pwd(Process *__restrict self) {
	if (self == &this_process) {
		return DeeModule_CallExtern((DeeObject *)&str_posix,
		                            (DeeObject *)&str_getcwd,
		                            0, NULL);
	}
	ipc_unimplemented();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_pwd(Process *self, DeeObject *value) {
	if (value &&
	    DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if (self == &this_process) {
		DREF DeeObject *temp;
		if unlikely(!value)
			goto err_running;
		temp = DeeModule_CallExtern((DeeObject *)&str_posix,
		                            (DeeObject *)&str_chdir,
		                            1, &value);
		if unlikely(!temp)
			goto err;
		Dee_Decref(temp);
		return 0;
	}
	ipc_unimplemented();
	goto err;
err_running:
	DeeError_Throwf(&DeeError_ValueError,
	                "Cannot %s pwd of running process %k",
	                value ? "change"
	                      : "delete",
	                self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_pwd(Process *__restrict self) {
	return process_set_pwd(self, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_exe(Process *__restrict self) {
	if (self == &this_process)
		return_reference_((DeeObject *)DeeModule_GetDeemon()->mo_name);
	ipc_unimplemented();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_exe(Process *self, DeeObject *value) {
	if (value &&
	    DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if (self == &this_process) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot set exe for running process %k",
		                self);
err:
		return -1;
	}
	ipc_unimplemented();
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_exe(Process *__restrict self) {
	return process_set_exe(self, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_cmdline(Process *__restrict self) {
	if (self == &this_process) {
		size_t i;
		DREF DeeObject *argv = Dee_GetArgv();
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		DeeStringObject *deemon_str = DeeModule_GetDeemon()->mo_name;
		if (unicode_printer_print(&printer,
		                          DeeString_STR(deemon_str),
		                          DeeString_SIZE(deemon_str)) < 0)
			goto err_printer;
		for (i = 0; i < DeeTuple_SIZE(argv); ++i) {
			char *arg;
			if (unicode_printer_putascii(&printer, ' '))
				goto err_printer;
			ASSERT_OBJECT_TYPE_EXACT(DeeTuple_GET(argv, i), &DeeString_Type);
			arg = DeeString_AsUtf8(DeeTuple_GET(argv, i));
			if unlikely(!arg)
				goto err_printer;
			if (process_cmdline_encode_argument(&unicode_printer_print,
			                                    &printer,
			                                    arg, WSTR_LENGTH(arg)) < 0)
				goto err_printer;
		}
		Dee_Decref(argv);
		return unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
		Dee_Decref(argv);
	} else {
		ipc_unimplemented();
	}
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_cmdline(Process *self, DeeObject *value) {
	if (value &&
	    DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	if (self == &this_process) {
		DeeError_Throwf(&DeeError_ValueError,
		                "Cannot set cmdline for running process %k",
		                self);
err:
		return -1;
	}
	ipc_unimplemented();
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_cmdline(Process *__restrict self) {
	return process_set_cmdline(self, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_argv(Process *__restrict self) {
	if (self == &this_process) {
		size_t i;
		DREF DeeObject *argv, *result, *temp;
		argv   = Dee_GetArgv();
		result = DeeTuple_NewUninitialized(DeeTuple_SIZE(argv) + 1);
		if unlikely(!result)
			goto err;
		temp = (DeeObject *)DeeModule_GetDeemon()->mo_name;
		Dee_Incref(temp);
		DeeTuple_SET(result, 0, temp);
		for (i = 0; i < DeeTuple_SIZE(argv); ++i) {
			temp = DeeTuple_GET(argv, i);
			Dee_Incref(temp);
			DeeTuple_SET(result, i + 1, temp);
		}
		Dee_Decref(argv);
		return result;
err_argv:
		Dee_Decref(argv);
	} else {
		ipc_unimplemented();
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_set_argv(Process *self, DeeObject *value) {
	ipc_unimplemented();
err:
	return -1;
}


PRIVATE struct type_getset tpconst process_getsets[] = {
	TYPE_GETSET(S_Process_getset_stdin_name, &process_get_stdin, &process_del_stdin, &process_set_stdin, S_Process_getset_stdin_doc),
	TYPE_GETSET(S_Process_getset_stdout_name, &process_get_stdout, &process_del_stdout, &process_set_stdout, S_Process_getset_stdout_doc),
	TYPE_GETSET(S_Process_getset_stderr_name, &process_get_stderr, &process_del_stderr, &process_set_stderr, S_Process_getset_stderr_doc),
	TYPE_GETSET(S_Process_getset_pwd_name, &process_get_pwd, &process_del_pwd, &process_set_pwd, S_Process_getset_pwd_doc),
	TYPE_GETSET(S_Process_getset_exe_name, &process_get_exe, &process_del_exe, &process_set_exe, S_Process_getset_exe_doc),
	TYPE_GETSET(S_Process_getset_cmdline_name, &process_get_cmdline, &process_del_cmdline, &process_set_cmdline, S_Process_getset_cmdline_doc),
	TYPE_GETSET(S_Process_getset_argv_name, &process_get_argv, &process_del_cmdline, &process_set_argv, S_Process_getset_argv_doc),
	TYPE_GETSET(S_Process_getset_cmd_name, &process_get_cmdline, &process_del_cmdline, &process_set_cmdline, S_Process_getset_cmd_doc),
	TYPE_GETSET(S_Process_getset_environ_name, &process_get_environ, &process_del_environ, &process_set_environ, S_Process_getset_environ_doc),
	TYPE_GETTER(S_Process_getset_hasstarted_name, &process_hasstarted, S_Process_getset_hasstarted_doc),
	TYPE_GETTER(S_Process_getset_wasdetached_name, &process_wasdetached, S_Process_getset_wasdetached_doc),
	TYPE_GETTER(S_Process_getset_hasterminated_name, &process_wasterminated, S_Process_getset_hasterminated_doc),
	TYPE_GETTER(S_Process_getset_isachild_name, &process_isachild, S_Process_getset_isachild_doc),
	TYPE_GETTER(S_Process_getset_id_name, &process_id, S_Process_getset_id_doc),
	TYPE_GETTER(S_Process_getset_threads_name, &process_get_threads, S_Process_getset_threads_doc),
	TYPE_GETTER(S_Process_getset_files_name, &process_get_files, S_Process_getset_files_doc),
	TYPE_GETTER(S_Process_getset_memory_name, &process_get_memory, S_Process_getset_memory_doc),
	TYPE_GETSET_END
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_class_self(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":" S_Process_class_function_self_name))
		goto err;
	return_reference_((DeeObject *)&this_process);
err:
	return NULL;
}

PRIVATE struct type_method tpconst process_class_methods[] = {
	TYPE_METHOD(S_Process_class_function_self_name, &process_class_self, S_Process_class_function_self_doc),
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst process_class_members[] = {
	TYPE_MEMBER_CONST_DOC(S_Process_member_current_name,
	                      &this_process,
	                      S_Process_member_current_doc),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
process_init(Process *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeObject *temp;
	if (DeeArg_Unpack(argc, argv, "o|o:" S_Process_tp_name, &temp, &temp))
		goto err;
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject DeeProcess_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_Process_tp_name,
	/* .tp_doc      = */ S_Process_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&process_init,
				TYPE_FIXED_ALLOCATOR(Process)
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
	/* .tp_methods       = */ process_methods,
	/* .tp_getsets       = */ process_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ process_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ process_class_members
};



INTERN DeeTypeObject DeeProcEnumIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_ProcEnumIterator_tp_name,
	/* .tp_doc      = */ S_ProcEnumIterator_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
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
};

PRIVATE struct type_member tpconst enumproc_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeProcEnumIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeProcEnum_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ S_ProcEnum_tp_name,
	/* .tp_doc      = */ S_ProcEnum_tp_doc,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
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
	/* .tp_class_members = */ enumproc_class_members
};

DECL_END

#endif /* !GUARD_DEX_IPC_GENERIC_C_INL */
