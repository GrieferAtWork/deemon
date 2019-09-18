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
#ifndef GUARD_DEX_IPC_GENERIC_C_INL
#define GUARD_DEX_IPC_GENERIC_C_INL 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/exec.h>
#include <deemon/file.h>
#include <deemon/filetypes.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>

#include "libipc.h"

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
} Process;

PRIVATE Process this_process = {
	OBJECT_HEAD_INIT(&DeeProcess_Type),
};

PRIVATE int DCALL ipc_unimplemented(void) {
	return DeeError_Throwf(&DeeError_UnsupportedAPI,
	                       "IPI functionality is not supported");
}

PRIVATE DREF DeeObject *DCALL
process_start(Process *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":start"))
		goto err;
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_detach(Process *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":detach"))
		goto err;
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_id(Process *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":id"))
		goto err;
	if (self == &this_process)
		return_reference_(&DeeInt_Zero);
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_terminate(Process *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	uint32_t exit_code = 0;
	if (DeeArg_Unpack(argc, argv, "|I32u:terminate", &exit_code))
		goto err;
	if (self == &this_process) {
		Dee_Exit((int)exit_code, false);
		goto err;
	}
	ipc_unimplemented();
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
process_join(Process *__restrict self, size_t argc,
             DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":join"))
		goto err;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_tryjoin(Process *__restrict self, size_t argc,
                DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":tryjoin"))
		goto err;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_timedjoin(Process *__restrict self, size_t argc,
                  DeeObject **__restrict argv) {
	uint64_t timeout;
	if (DeeArg_Unpack(argc, argv, "I64d:timedjoin", &timeout))
		goto err;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_started(Process *__restrict self, size_t argc,
                DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":started"))
		goto err;
	if (self == &this_process)
		return_true;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_detached(Process *__restrict self, size_t argc,
                 DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":detached"))
		goto err;
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_isachild(Process *__restrict self, size_t argc,
                 DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":isachild"))
		goto err;
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
process_terminated(Process *__restrict self, size_t argc,
                   DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":terminated"))
		goto err;
	if (self == &this_process)
		return_false;
	ipc_unimplemented();
err:
	return NULL;
}


PRIVATE struct type_method process_methods[] = {
	{ "start", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_start,
	  DOC("->?Dbool\n"
	      "@interrupt\n"
	      "@throw FileNotFound The specified executable could not be found\n"
	      "@throw FileAccessError The current user does not have permissions to access the "
	      "executable, or the executable is lacking execute permissions\n"
	      "@throw SystemError Failed to start the process for some reason\n"
	      "Begin execution of the process") },
	{ "join", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_join,
	  DOC("->?Dint\n"
	      "@interrupt\n"
	      "@throw ValueError @this process was never started\n"
	      "@throw SystemError Failed to join @this process for some reason\n"
	      "@return The exit code of the process\n"
	      "Joins @this process and returns the return value of its main function\n"
	      "In the event") },
	{ "tryjoin", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_tryjoin,
	  DOC("->?Dint\n"
	      "()\n"
	      "@throw ValueError @this process was never started\n"
	      "@throw SystemError Failed to join @this process for some reason\n"
	      "Same as #join, but don't check for interrupts and fail immediately") },
	{ "timedjoin", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_timedjoin,
	  DOC("(timeout_in_microseconds:?Dint)->?Dint\n"
	      "(timeout_in_microseconds:?Dint)\n"
	      "@throw ValueError @this process was never started\n"
	      "@throw SystemError Failed to join @this process for some reason\n"
	      "Same as #join, but only attempt to join for a given @timeout_in_microseconds") },
	{ "detach", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_detach,
	  DOC("->?Dbool\n"
	      "@throw ValueError @this process was never started\n"
	      "@throw ValueError @this process is not a child of the calling process\n"
	      "@throw SystemError Failed to detach @this process for some reason\n"
	      "@return true: The :process has been detached\n"
	      "@return false: The :process was already detached\n"
	      "Detaches @this process") },
	{ "terminate", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_terminate,
	  DOC("(exitcode=!0)->?Dbool\n"
	      "@throw ValueError @this process was never started\n"
	      "@throw SystemError Failed to terminate @this process for some reason\n"
	      "@return true: The :process has been terminated\n"
	      "@return false: The :process was already terminated\n"
	      "Terminate @this process with the given @exitcode") },
	{ "started", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_started,
	  DOC("->?Dbool\n"
	      "Returns :true if @this process was started") },
	{ "detached", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_detached,
	  DOC("->?Dbool\n"
	      "Returns :true if @this process has been detached") },
	{ "terminated", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_terminated,
	  DOC("->?Dbool\n"
	      "Returns :true if @this process has terminated") },
	{ "isachild", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_isachild,
	  DOC("->?Dbool\n"
	      "Returns :true if @this process is a child of the calling process") },
	{ "id", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict))&process_id,
	  DOC("->?Dint\n"
	      "@throw ValueError The process hasn't been started yet\n"
	      "@throw SystemError The system does not provide a way to query process ids\n"
	      "Returns an operating-system specific id of @this process") },
	{ NULL }
};


PRIVATE DREF DeeObject *DCALL
process_get_files(Process *__restrict self) {
	ipc_unimplemented();
	return NULL;
}

#define DEFINE_PROCESS_STD_FUNCTIONS(stdxxx, DEE_STDXXX)                          \
	PRIVATE DREF DeeObject *DCALL                                                 \
	process_get_##stdxxx(Process *__restrict self) {                              \
		if (self == &this_process)                                                \
			return DeeFile_GetStd(DEE_STDXXX);                                    \
		ipc_unimplemented();                                                      \
		return NULL;                                                              \
	}                                                                             \
	PRIVATE int DCALL                                                             \
	process_del_##stdxxx(Process *__restrict self) {                              \
		if (self == &this_process) {                                              \
			DREF DeeObject *oldval;                                               \
			oldval = DeeFile_SetStd(DEE_STDXXX, NULL);                            \
			if (ITER_ISOK(oldval))                                                \
				Dee_Decref(oldval);                                               \
			return 0;                                                             \
		}                                                                         \
		return ipc_unimplemented();                                               \
	}                                                                             \
	PRIVATE int DCALL                                                             \
	process_set_##stdxxx(Process *__restrict self, DeeObject *__restrict value) { \
		if (self == &this_process) {                                              \
			DREF DeeObject *oldval;                                               \
			oldval = DeeFile_SetStd(DEE_STDXXX, value);                           \
			if (ITER_ISOK(oldval))                                                \
				Dee_Decref(oldval);                                               \
			return 0;                                                             \
		}                                                                         \
		return ipc_unimplemented();                                               \
	}
DEFINE_PROCESS_STD_FUNCTIONS(stdin, DEE_STDIN)
DEFINE_PROCESS_STD_FUNCTIONS(stdout, DEE_STDOUT)
DEFINE_PROCESS_STD_FUNCTIONS(stderr, DEE_STDERR)
#undef DEFINE_PROCESS_STD_FUNCTIONS

PRIVATE DREF DeeObject *DCALL
call_extern(DeeObject *__restrict module_name,
            DeeObject *__restrict global_name,
            size_t argc, DeeObject **__restrict argv) {
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

PRIVATE DREF DeeObject *DCALL
get_extern(DeeObject *__restrict module_name,
           DeeObject *__restrict global_name) {
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

#ifndef CONFIG_NO_THREADS
PRIVATE DREF DeeObject *DCALL
process_get_threads(Process *__restrict self) {
	if (self == &this_process)
		return DeeObject_GetAttrString((DeeObject *)&DeeThread_Type, "enumerate");
	ipc_unimplemented();
	return NULL;
}
#endif /* !CONFIG_NO_THREADS */
PRIVATE DREF DeeObject *DCALL
process_get_environ(Process *__restrict self) {
	if (self == &this_process)
		return get_extern((DeeObject *)&str_fs, (DeeObject *)&str_environ);
	ipc_unimplemented();
	return NULL;
}

PRIVATE int DCALL
process_set_environ(Process *__restrict self, DeeObject *value) {
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

PRIVATE int DCALL
process_del_environ(Process *__restrict self) {
	return process_set_environ(self, NULL);
}


PRIVATE DREF DeeObject *DCALL
process_get_pwd(Process *__restrict self) {
	if (self == &this_process)
		return call_extern((DeeObject *)&str_fs, (DeeObject *)&str_getcwd, 0, NULL);
	ipc_unimplemented();
	return NULL;
}

PRIVATE int DCALL
process_set_pwd(Process *__restrict self, DeeObject *value) {
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
	                value ? "change" : "delete", self);
err:
	return -1;
}

PRIVATE int DCALL
process_del_pwd(Process *__restrict self) {
	return process_set_pwd(self, NULL);
}

PRIVATE DREF DeeObject *DCALL
process_get_exe(Process *__restrict UNUSED(self)) {
	ipc_unimplemented();
	return NULL;
}

PRIVATE int DCALL
process_set_exe(Process *__restrict self, DeeObject *value) {
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

PRIVATE int DCALL
process_del_exe(Process *__restrict self) {
	return process_set_exe(self, NULL);
}

PRIVATE DREF DeeObject *DCALL
process_get_cmdline(Process *__restrict UNUSED(self)) {
	ipc_unimplemented();
	return NULL;
}

PRIVATE int DCALL
process_set_cmdline(Process *__restrict self, DeeObject *value) {
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

PRIVATE int DCALL
process_del_cmdline(Process *__restrict self) {
	return process_set_cmdline(self, NULL);
}

PRIVATE DREF DeeObject *DCALL
process_get_argv(Process *__restrict UNUSED(self)) {
	ipc_unimplemented();
	return NULL;
}

PRIVATE int DCALL
process_set_argv(Process *__restrict UNUSED(self),
                 DeeObject *__restrict value) {
	if (DeeObject_AssertTypeExact(value, &DeeString_Type))
		goto err;
	ipc_unimplemented();
err:
	return -1;
}


PRIVATE struct type_getset process_getsets[] = {
#ifndef CONFIG_NO_THREADS
	{ "threads", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_threads, NULL, NULL,
	  DOC("->?S?Dthread\nEnumerate all the threads of @this process") },
#endif
	{ "files", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_files },
	{ "stdin", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_stdin,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stdin,
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_stdin,
	  DOC("->?DFile\nGet or set the standard input stream used by the process") },
	{ "stdout", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_stdout,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stdout,
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_stdout,
	  DOC("->?DFile\nGet or set the standard output stream used by the process") },
	{ "stderr", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_stderr,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_stderr,
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_stderr,
	  DOC("->?DFile\nGet or set the standard error stream used by the process") },
	{ "pwd", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_pwd,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_pwd,
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_pwd,
	  DOC("->?DFile\nGet or set the process working directory used by the process") },
	{ "exe", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_exe,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_exe,
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_exe,
	  DOC("->?Dstring\nThe filename of the image being executed by the process") },
	{ "cmdline", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_cmdline,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline,
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_cmdline,
	  DOC("->?Dstring\nThe commandline used to executed the process") },
	{ "argv", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_argv,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline, /* Same thing! */
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_argv,
	  DOC("->?S?Dstring\nThe argument vector passed to the programms C-main() method") },
	{ "cmd", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_cmdline,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_cmdline,
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_cmdline,
	  DOC("->?Dstring\nDeprecated alias for #cmdline") },
	{ "environ", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&process_get_environ,
	  (int (DCALL *)(DeeObject *__restrict))&process_del_environ,
	  (int (DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&process_set_environ,
	  DOC("->?S?T2?Dstring?Dstring\nThe state of environment variables in the given process") },
	/* Process environment control:
	 * >> property memory -> file; // A file allowing read/write access to the process's VM
	 */
	{ NULL }
};




PRIVATE DREF DeeObject *DCALL
process_class_self(DeeObject *__restrict UNUSED(self),
                   size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":self"))
		return NULL;
	return_reference_((DeeObject *)&this_process);
}

PRIVATE struct type_method process_class_methods[] = {
	{ "self", &process_class_self,
	  DOC("->?.\n"
	      "Deprecated alias for #current") },
	{ NULL }
};

PRIVATE struct type_member process_class_members[] = {
	TYPE_MEMBER_CONST("current", &this_process),
	TYPE_MEMBER_END
};


PRIVATE int DCALL
process_init(Process *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	DeeObject *temp;
	if (DeeArg_Unpack(argc, argv, "o|o:process", &temp, &temp))
		goto err;
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject DeeProcess_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "process",
	/* .tp_doc      = */ DOC("(exe:?Dstring)\n"
	                         "(exe:?Dstring,args:{?Dstring})\n"
	                         "@param args A sequence of :{string}s that should be passed as secondary arguments. "
	                                     "When provided, @exe is prepended first, however the full argument list "
	                                     "can be overwritten using the #argv property\n"
	                         "Construct a new, non-started process for application @exe\n"
	                         "If provided, additional arguments @args are passed "
	                         "to the process's C-level ${main()} function\n"
	                         "To start a process, invoke the #start function, prior to which "
	                         "you are able to override/redirect the full commandline (#cmdline), "
	                         "the full argument list (#argv, including the implicit initial "
	                         "argument that normally defaults to @exe), or the standard file "
	                         "streams (#stdin, #stdout, #stderr) using :pipe readers/writers."),
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
	/* .tp_name     = */ "enumproc.Iterator",
	/* .tp_doc      = */ NULL,
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
	/* .tp_name     = */ "enumproc",
	/* .tp_doc      = */ NULL,
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
	/* .tp_seq           = */ NULL, /* TODO */
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


typedef DeeSystemFileObject SystemFile;

PRIVATE DREF DeeObject *DCALL
pipe_class_new(DeeObject *__restrict UNUSED(self),
               size_t argc, DeeObject **__restrict argv) {
	uint32_t pipe_size;
	if (DeeArg_Unpack(argc, argv, "|I32u:new", &pipe_size))
		goto err;
	ipc_unimplemented();
err:
	return NULL;
}


PRIVATE struct type_member pipe_class_members[] = {
	TYPE_MEMBER_CONST("Reader", (DeeObject *)&DeePipeReader_Type),
	TYPE_MEMBER_CONST("Writer", (DeeObject *)&DeePipeWriter_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_method pipe_class_methods[] = {
	{ "new", &pipe_class_new,
	  DOC("(size_hint=!0)->?T2?#reader?#writer\n"
	      "Creates a new pair of linked pipe files") },
	{ NULL }
};

INTERN DeeFileTypeObject DeePipe_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "pipe",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
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
		/* .tp_class_methods = */ pipe_class_methods,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ pipe_class_members
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

INTERN DeeFileTypeObject DeePipeReader_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "pipe.reader",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeePipe_Type,
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

INTERN DeeFileTypeObject DeePipeWriter_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "pipe.writer",
		/* .tp_doc      = */ NULL,
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ (DeeTypeObject *)&DeePipe_Type,
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

#endif /* !GUARD_DEX_IPC_GENERIC_C_INL */
