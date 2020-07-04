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
#ifndef GUARD_DEX_IPC_GENERIC_C_INL
#define GUARD_DEX_IPC_GENERIC_C_INL 1
#define DEE_SOURCE 1

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
	if (DeeArg_Unpack(argc, argv, "|I32u:" S_Process_function_terminate_name, &exit_code))
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
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, "I64d:" S_Process_function_timedjoin_name, &timeout))
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


PRIVATE struct type_method process_methods[] = {
	{ S_Process_function_start_name, (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_start, S_Process_function_start_doc },
	{ S_Process_function_join_name, (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_join, S_Process_function_join_doc },
	{ S_Process_function_tryjoin_name, (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_tryjoin, S_Process_function_tryjoin_doc },
	{ S_Process_function_timedjoin_name, (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_timedjoin, S_Process_function_timedjoin_doc },
	{ S_Process_function_detach_name, (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_detach, S_Process_function_detach_doc },
	{ S_Process_function_terminate_name, (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&process_terminate, S_Process_function_terminate_doc },
	{ NULL }
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_files(Process *__restrict self) {
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
call_extern(DeeObject *module_name, DeeObject *global_name,
            size_t argc, DeeObject *const *argv) {
	DREF DeeObject *module, *result;
	module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!module)
		goto err;
	result = DeeObject_CallAttr(module, global_name, argc, argv);
	Dee_Decref(module);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
get_extern(DeeObject *module_name, DeeObject *global_name) {
	DREF DeeObject *module, *result;
	module = DeeModule_OpenGlobal(module_name, NULL, true);
	if unlikely(!module)
		goto err;
	result = DeeObject_GetAttr(module, global_name);
	Dee_Decref(module);
	return result;
err:
	return NULL;
}

PRIVATE DEFINE_STRING(str_fs, "fs");
PRIVATE DEFINE_STRING(str_getcwd, "getcwd");
PRIVATE DEFINE_STRING(str_chdir, "chdir");
PRIVATE DEFINE_STRING(str_environ, "environ");

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_threads(Process *__restrict self) {
	if (self == &this_process)
		return DeeObject_GetAttrString((DeeObject *)&DeeThread_Type, "enumerate");
	ipc_unimplemented();
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_environ(Process *__restrict self) {
	if (self == &this_process)
		return get_extern((DeeObject *)&str_fs, (DeeObject *)&str_environ);
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
		temp = get_extern((DeeObject *)&str_fs, (DeeObject *)&str_environ);
		if unlikely(!temp)
			return -1;
		result = DeeObject_Assign(temp, value ? value : Dee_None);
		Dee_Decref(temp);
		return result;
	}
	return ipc_unimplemented();
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
process_del_environ(Process *__restrict self) {
	return process_set_environ(self, NULL);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_get_pwd(Process *__restrict self) {
	if (self == &this_process)
		return call_extern((DeeObject *)&str_fs, (DeeObject *)&str_getcwd, 0, NULL);
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
		temp = call_extern((DeeObject *)&str_fs,
		                   (DeeObject *)&str_chdir,
		                   0, NULL);
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


PRIVATE struct type_getset process_getsets[] = {
	{ S_Process_getset_stdin_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_stdin,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stdin,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_stdin,
	  S_Process_getset_stdin_doc },
	{ S_Process_getset_stdout_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_stdout,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stdout,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_stdout,
	  S_Process_getset_stdout_doc },
	{ S_Process_getset_stderr_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_stderr,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stderr,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_stderr,
	  S_Process_getset_stderr_doc },
	{ S_Process_getset_pwd_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_pwd,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_pwd,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_pwd,
	  S_Process_getset_pwd_doc },
	{ S_Process_getset_exe_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_exe,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_exe,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_exe,
	  S_Process_getset_exe_doc },
	{ S_Process_getset_cmdline_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_cmdline,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_cmdline,
	  S_Process_getset_cmdline_doc },
	{ S_Process_getset_argv_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_argv,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline, /* Same thing! */
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_argv,
	  S_Process_getset_argv_doc },
	{ S_Process_getset_cmd_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_cmdline,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_cmdline,
	  S_Process_getset_cmd_doc },
	{ S_Process_getset_environ_name,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_environ,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_environ,
	  (int (DCALL *)(DeeObject *, DeeObject *))&process_set_environ,
	  S_Process_getset_environ_doc },
	{ S_Process_getset_hasstarted_name, (DREF DeeObject *(DCALL *)(DeeObject *))&process_hasstarted, NULL, NULL, S_Process_getset_hasstarted_doc },
	{ S_Process_getset_wasdetached_name, (DREF DeeObject *(DCALL *)(DeeObject *))&process_wasdetached, NULL, NULL, S_Process_getset_wasdetached_doc },
	{ S_Process_getset_hasterminated_name, (DREF DeeObject *(DCALL *)(DeeObject *))&process_wasterminated, NULL, NULL, S_Process_getset_hasterminated_doc },
	{ S_Process_getset_isachild_name, (DREF DeeObject *(DCALL *)(DeeObject *))&process_isachild, NULL, NULL, S_Process_getset_isachild_doc },
	{ S_Process_getset_id_name, (DREF DeeObject *(DCALL *)(DeeObject *))&process_id, NULL, NULL, S_Process_getset_id_doc },
	{ S_Process_getset_threads_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_threads, NULL, NULL, S_Process_getset_threads_doc },
	{ S_Process_getset_files_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_files, NULL, NULL, S_Process_getset_files_doc },
	{ S_Process_getset_memory_name, (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&process_get_memory, NULL, NULL, S_Process_getset_memory_doc },
	{ NULL }
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
process_class_self(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":" S_Process_class_function_self_name))
		return NULL;
	return_reference_((DeeObject *)&this_process);
}

PRIVATE struct type_method process_class_methods[] = {
	{ S_Process_class_function_self_name, &process_class_self, S_Process_class_function_self_doc },
	{ NULL }
};

PRIVATE struct type_member process_class_members[] = {
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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (void *)&process_init,
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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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

PRIVATE struct type_member enumproc_class_members[] = {
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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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
