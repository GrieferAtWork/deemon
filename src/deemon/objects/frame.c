/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_FRAME_C
#define GUARD_DEEMON_OBJECTS_FRAME_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* strchr() */
#include <deemon/system.h>          /* DeeSystem_ALTSEP */
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <stdarg.h>
#include <stddef.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

/**/
#include "../execute/function-wrappers.h"

DECL_BEGIN

typedef DeeFrameObject Frame;

#define DOC_ReferenceError \
	"#tReferenceError{The Frame has continued execution, or was otherwise released}"

PRIVATE ATTR_COLD int DCALL
err_dead_frame(Frame *__restrict UNUSED(self)) {
	return DeeError_Throwf(&DeeError_ReferenceError,
	                       "Frame access was revoked");
}

PRIVATE ATTR_COLD int DCALL
err_readonly_frame(Frame *__restrict UNUSED(self)) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "The Frame is readonly and cannot be modifed");
}


/* Construct a frame object owned by `owner'
 * The intended use of this is for tracebacks and yield_function-iterators.
 * @param: flags: Set of `DEEFRAME_F*' */
PUBLIC WUNUSED NONNULL((2)) DREF DeeObject *
(DCALL DeeFrame_NewReferenceWithLock)(DeeObject *owner,
                                      struct code_frame *__restrict frame,
                                      uint16_t flags, void *lock) {
	DREF Frame *result;
	ASSERT_OBJECT_OPT(owner);
	result = DeeObject_MALLOC(Frame);
	if unlikely(!result)
		goto done;
	result->f_owner = owner;
	result->f_frame = frame;
	result->f_flags = flags;
	Dee_atomic_rwlock_init(&result->f_lock);
#ifndef CONFIG_NO_THREADS
	result->f_palock = (Dee_atomic_rwlock_t *)lock;
#else /* !CONFIG_NO_THREADS */
	(void)lock; /* Unused... */
#endif /* CONFIG_NO_THREADS */
	Dee_XIncref(owner);
	DeeObject_Init(result, &DeeFrame_Type);
done:
	return (DREF DeeObject *)result;
}


PUBLIC NONNULL((1)) void DCALL
DeeFrame_DecrefShared(DREF DeeObject *__restrict self) {
	Frame *me;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeFrame_Type);
	me = (Frame *)self;
	DeeFrame_LockWrite(me);
	me->f_frame = NULL;
	DeeFrame_LockEndWrite(me);
	Dee_Decref_likely(self);
}



#ifdef DeeSystem_ALTSEP
PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) char const *DCALL
get_relevant_slash(char const *path, char const *file) {
	char path_slash = 0;
	char file_slash = 0;
	for (; *path; ++path) {
		if (DeeSystem_IsSep(*path))
			path_slash = *path;
	}
	for (; *file; ++file) {
		if (DeeSystem_IsSep(*file)) {
			file_slash = *file;
			break;
		}
	}
	if (!path_slash && !file_slash)
		goto return_sep;
	if (path_slash == DeeSystem_SEP)
		goto return_sep;
	if (file_slash == DeeSystem_SEP)
		goto return_sep;
/*return_altsep:*/
	return DeeSystem_ALTSEP_S;
return_sep:
	return DeeSystem_SEP_S;
}
#define TRACEBACK_SLASH(path, file) get_relevant_slash(path, file)
#else /* DeeSystem_ALTSEP */
#define TRACEBACK_SLASH(path, file) DeeSystem_SEP_S
#define TRACEBACK_SLASH_S           DeeSystem_SEP_S
#endif /* !DeeSystem_ALTSEP */

INTERN WUNUSED NONNULL((1, 3)) dssize_t DCALL
print_ddi(dformatprinter printer, void *arg,
          DeeCodeObject *__restrict code, code_addr_t ip) {
	dssize_t temp, result;
	struct ddi_state state;
	if (!DeeCode_FindDDI((DeeObject *)code, &state, NULL, ip,
	                     DDI_STATE_FNOTHROW | DDI_STATE_FNONAMES)) {
		result = DeeFormat_Printf(printer, arg, "%s+%.4I32X\n",
		                          DeeCode_NAME(code), ip);
	} else {
		struct ddi_xregs *iter;
		char const *path, *file, *name;
		char const *base_name = DeeCode_NAME(code);
		result = 0;
		DDI_STATE_DO(iter, &state) {
			file = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_file);
			name = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_name);
			if (!state.rs_regs.dr_path--) {
				path = NULL;
			} else {
				path = DeeCode_GetDDIString((DeeObject *)code, iter->dx_base.dr_path);
			}
			temp = DeeFormat_Printf(printer, arg,
			                        "%s%s%s(%d,%d) : %s+%.4I32X",
			                        path ? path : "",
			                        path ? TRACEBACK_SLASH(path, file ? file : "") : "",
			                        file ? file : "",
			                        iter->dx_base.dr_lno + 1,
			                        iter->dx_base.dr_col + 1,
			                        name ? name
			                             : (code->co_flags & CODE_FCONSTRUCTOR
			                                ? "<anonymous_ctor>"
			                                : "<anonymous>"),
			                        ip);
			if unlikely(temp < 0) {
				result = temp;
				break;
			}
			result += temp;
			if (name != base_name && *base_name) {
				/* Also print the name of the base-function */
				temp = DeeFormat_Printf(printer, arg, " (%s)", base_name);
				if unlikely(temp < 0) {
					result = temp;
					break;
				}
				result += temp;
			}
			temp = DeeFormat_PRINT(printer, arg, "\n");
			if unlikely(temp < 0) {
				result = temp;
				break;
			}
			result += temp;
		}
		DDI_STATE_WHILE(iter, &state);
		Dee_ddi_state_fini(&state);
	}
	return result;
}

PRIVATE NONNULL((1)) void DCALL
frame_fini(Frame *__restrict self) {
	Dee_XDecref(self->f_owner);
}

PRIVATE NONNULL((1, 2)) void DCALL
frame_visit(Frame *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->f_owner);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
frame_print(Frame *__restrict self,
            dformatprinter printer, void *arg) {
	dssize_t result;
	DREF DeeCodeObject *code;
	code_addr_t ip;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		return 0;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	code = self->f_frame->cf_func->fo_code;
	Dee_Incref(code);
	ip = (code_addr_t)(self->f_frame->cf_ip - code->co_code);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	result = print_ddi(printer, arg, code, ip);
	Dee_Decref_unlikely(code);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeCodeObject *DCALL
frame_getddi(Frame *__restrict self,
             struct ddi_state *__restrict state,
             code_addr_t *p_startip,
             code_addr_t *p_endip,
             unsigned int flags) {
	uint8_t *result;
	code_addr_t startip;
	DREF DeeCodeObject *code;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		return (DREF DeeCodeObject *)ITER_DONE;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	code = self->f_frame->cf_func->fo_code;
	Dee_Incref(code);
	startip = (code_addr_t)(self->f_frame->cf_ip -
	                        code->co_code);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	if (p_startip)
		*p_startip = startip;
	result = DeeCode_FindDDI((DeeObject *)code,
	                         state,
	                         p_endip,
	                         startip,
	                         flags);
	if (DDI_ISOK(result))
		return code;
	Dee_Decref(code);
	if unlikely(result != DDI_NEXT_DONE)
		goto err;
	return (DREF DeeCodeObject *)ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
frame_getlocation(Frame *__restrict self) {
	DREF DeeTupleObject *result;
	DREF DeeObject *entry, *fileob, *nameob;
	DREF DeeCodeObject *code;
	struct ddi_xregs *iter;
	struct ddi_state state;
	size_t i, count;
	code = frame_getddi(self, &state, NULL, NULL, DDI_STATE_FNONAMES);
	if unlikely(!code)
		goto err;
	if unlikely(code == (DREF DeeCodeObject *)ITER_DONE)
		return (DREF DeeTupleObject *)DeeTuple_Newf("(nnnn)");
	count = 0;
	DDI_STATE_DO(iter, &state) {
		++count;
	}
	DDI_STATE_WHILE(iter, &state);
	result = DeeTuple_NewUninitialized(count);
	if unlikely(!result)
		goto err_state;
	i = 0;
	DDI_STATE_DO(iter, &state) {
		char *path, *file;
		file = DeeCode_GetDDIString((DeeObject *)code, state.rs_regs.dr_file);
		if unlikely(!file) {
			fileob = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			if (!state.rs_regs.dr_path-- ||
			    (path = DeeCode_GetDDIString((DeeObject *)code, state.rs_regs.dr_file)) == NULL) {
				fileob = DeeString_New(file);
			} else {
#ifdef TRACEBACK_SLASH_S
				fileob = DeeString_Newf("%s" TRACEBACK_SLASH_S "%s", path, file);
#else /* TRACEBACK_SLASH_S */
				fileob = DeeString_Newf("%s%s%s", path, TRACEBACK_SLASH(path, file), file);
#endif /* !TRACEBACK_SLASH_S */
			}
			if unlikely(!fileob)
				goto err_state_r;
		}
		path = DeeCode_GetDDIString((DeeObject *)code, state.rs_regs.dr_name);
		if (!path) {
			nameob = Dee_None;
			Dee_Incref(Dee_None);
		} else {
			nameob = DeeString_New(path);
			if unlikely(!nameob)
				goto err_state_r_fileob;
		}
		entry = DeeTuple_Newf("oddo",
		                      fileob,
		                      state.rs_regs.dr_lno,
		                      state.rs_regs.dr_col,
		                      nameob);
		Dee_Decref(nameob);
		Dee_Decref(fileob);
		if unlikely(!entry)
			goto err_state_r;
		DeeTuple_SET(result, i, entry); /* Inherit reference. */
		++i;
	}
	DDI_STATE_WHILE(iter, &state);
	ASSERT(i == count);
	Dee_ddi_state_fini(&state);
	Dee_Decref(code);
	return result;
err_state_r_fileob:
	Dee_Decref(fileob);
err_state_r:
	Dee_Decrefv(DeeTuple_ELEM(result), i);
	DeeTuple_FreeUninitialized(result);
err_state:
	Dee_ddi_state_fini(&state);
	Dee_Decref(code);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getfile(Frame *__restrict self) {
	DREF DeeObject *result;
	DREF DeeCodeObject *code;
	struct ddi_state state;
	char *path, *file;
	code = frame_getddi(self, &state, NULL, NULL, DDI_STATE_FNONAMES);
	if unlikely(!code)
		goto err;
	file = DeeCode_GetDDIString((DeeObject *)code, state.rs_regs.dr_file);
	if unlikely(!file) {
		result = Dee_None;
		Dee_Incref(Dee_None);
	} else {
		if (!state.rs_regs.dr_path-- ||
		    (path = DeeCode_GetDDIString((DeeObject *)code, state.rs_regs.dr_path)) == NULL) {
			result = DeeString_New(file);
		} else {
#ifdef TRACEBACK_SLASH_S
			result = DeeString_Newf("%s" TRACEBACK_SLASH_S "%s", path, file);
#else /* TRACEBACK_SLASH_S */
			result = DeeString_Newf("%s%s%s", path, TRACEBACK_SLASH(path, file), file);
#endif /* !TRACEBACK_SLASH_S */
		}
	}
	Dee_ddi_state_fini(&state);
	Dee_Decref(code);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getline(Frame *__restrict self) {
	DREF DeeObject *result;
	DREF DeeCodeObject *code;
	struct ddi_state state;
	code = frame_getddi(self, &state, NULL, NULL, DDI_STATE_FNONAMES);
	if unlikely(!code)
		goto err;
	if (code == (DREF DeeCodeObject *)ITER_DONE)
		return_none;
	result = DeeInt_NewInt(state.rs_regs.dr_lno);
	Dee_ddi_state_fini(&state);
	Dee_Decref(code);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getcol(Frame *__restrict self) {
	DREF DeeObject *result;
	DREF DeeCodeObject *code;
	struct ddi_state state;
	code = frame_getddi(self, &state, NULL, NULL, DDI_STATE_FNONAMES);
	if unlikely(!code)
		goto err;
	if (code == (DREF DeeCodeObject *)ITER_DONE)
		return_none;
	result = DeeInt_NewInt(state.rs_regs.dr_col);
	Dee_ddi_state_fini(&state);
	Dee_Decref(code);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getname(Frame *__restrict self) {
	DREF DeeObject *result;
	DREF DeeCodeObject *code;
	struct ddi_state state;
	char *name;
	code = frame_getddi(self, &state, NULL, NULL, DDI_STATE_FNONAMES);
	if unlikely(!code)
		goto err;
	if (code == (DREF DeeCodeObject *)ITER_DONE ||
	    (name = DeeCode_GetDDIString((DeeObject *)code, state.rs_regs.dr_name)) == NULL)
		return_none;
	result = DeeString_New(name);
	Dee_ddi_state_fini(&state);
	Dee_Decref(code);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getfunc(Frame *__restrict self) {
	DREF DeeFunctionObject *result;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		goto err_dead;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	result = self->f_frame->cf_func;
	Dee_Incref(result);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	return (DREF DeeObject *)result;
err_dead:
	err_dead_frame(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getcode(Frame *__restrict self) {
	DREF DeeCodeObject *result;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame)
		goto err_dead;
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	result = self->f_frame->cf_func->fo_code;
	Dee_Incref(result);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	return (DREF DeeObject *)result;
err_dead:
	DeeFrame_LockEndRead(self);
	err_dead_frame(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getpc(Frame *__restrict self) {
	code_addr_t pc;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame)
		goto err_dead;
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	DeeFrame_LockRead(self);
	pc = (code_addr_t)(self->f_frame->cf_ip -
	                   self->f_frame->cf_func->fo_code->co_code);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	return DeeInt_NewUInt32(pc);
err_dead:
	DeeFrame_LockEndRead(self);
	err_dead_frame(self);
err:
	return NULL;
}

/* Try to reverse-engineer SP based on other information
 * @return: * : The actual depth of the stack. (`DEEFRAME_FUNDEFSP' was unset, and `f_frame->cf_sp' was updated)
 *              NOTE: If the `DEEFRAME_FUNDEFSP' was already unset, don't unset it again
 *                    and discard all reversed information before returning whatever was
 *                    stored within `f_frame->cf_sp' at that point. (must be done to prevent a race condition)
 * @return: -1: Information could not be determined. (no error was thrown, but `DEEFRAME_FUNDEFSP2' was set)
 * @return: -2: The frame has continued execution, or was otherwise released. (no error was thrown) */
PRIVATE NONNULL((1)) int32_t DCALL
frame_revengsp(Frame *__restrict self) {
	(void)self;
	/* TODO */
	return -1;
}

/* Similar to `frame_revengsp()', but instead of always trying to reverse
 * the stack depth, check if the depth was already know upon entry, returning
 * the stored depth when `DEEFRAME_FUNDEFSP' isn't set, or always returning
 * `-1' when `DEEFRAME_FUNDEFSP2' is set.
 * @return: * : The actual depth of the stack.
 * @return: -1: Information could not be determined. (no error was thrown)
 * @return: -2: The frame has continued execution, or was otherwise released. (no error was thrown)
 * @return: -3: An error was thrown */
PRIVATE WUNUSED NONNULL((1)) int32_t DCALL
frame_getsp(Frame *__restrict self) {
	int32_t result;
	uint16_t flags;
again:
	flags = atomic_read(&self->f_flags);
	if (!(flags & DEEFRAME_FUNDEFSP)) {
		DeeFrame_LockRead(self);
		if unlikely(!self->f_frame) {
			DeeFrame_LockEndRead(self);
			return -2;
		}
		if (flags & DEEFRAME_FREGENGSP) {
			result = self->f_revsp;
		} else {
			if (!DeeFrame_PLockTryRead(self)) {
				DeeFrame_LockEndRead(self);
				if unlikely(DeeFrame_PLockWaitRead(self))
					goto err;
				goto again;
			}
			result = (int32_t)(self->f_frame->cf_sp -
			                   self->f_frame->cf_stack);
			DeeFrame_PLockEndRead(self);
		}
		DeeFrame_LockEndRead(self);
		return result;
	}
	if (flags & DEEFRAME_FUNDEFSP2)
		return -1;
	/* Try to reverse-engineer stack information. */
	return frame_revengsp(self);
err:
	return -3;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
frame_setpc(Frame *self, DeeObject *value) {
	code_addr_t pc;
	if unlikely(!(self->f_flags & DEEFRAME_FWRITABLE))
		return err_readonly_frame(self);
	if unlikely(DeeObject_AsUInt32(value, &pc))
		goto err;
	/* Make sure that the stack-depth is either entirely unknown,
	 * or has been reverse engineered based on the starting PC.
	 * This is required, since we're about to modify PC, meaning that
	 * if SP was still unknown at this point, trying to reverse it at
	 * a later point in time could yield invalid results. */
	if ((self->f_flags & (DEEFRAME_FUNDEFSP | DEEFRAME_FUNDEFSP2)) == DEEFRAME_FUNDEFSP)
		frame_revengsp(self);
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame)
		goto err_dead;
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	self->f_frame->cf_ip = self->f_frame->cf_func->fo_code->co_code + pc;
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	return 0;
err_dead:
	DeeFrame_LockEndRead(self);
	err_dead_frame(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getsp_obj(Frame *__restrict self) {
	int32_t result = frame_getsp(self);
	if unlikely(result == -3)
		goto err;
	if unlikely(result == -2)
		goto err_df;
	if unlikely(result == -1)
		goto err_unknown;
	return DeeInt_NewUInt16((uint16_t)result);
err_unknown:
	DeeError_Throwf(&DeeError_ValueError,
	                "Stack depth is unknown");
	goto err;
err_df:
	err_dead_frame(self);
err:
	return NULL;
}

/* @return: 1 : Failure (no frame present)
 * @return: 0 : Succes
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1)) int DCALL
frame_lockread_frame_for_bound(Frame *__restrict self) {
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		return 1;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	return 0;
err:
	return -1;
}

/* @return: 0 : Succes
 * @return: -1: Error */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
frame_lockread_frame(Frame *__restrict self) {
	int result = frame_lockread_frame_for_bound(self);
	if unlikely(result > 0)
		result = err_dead_frame(self);
	return result;
}

PRIVATE NONNULL((1)) void DCALL
frame_lockendread_frame(Frame *__restrict self) {
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
frame_lockwrite_frame(Frame *__restrict self) {
	if unlikely(!(self->f_flags & DEEFRAME_FWRITABLE))
		return err_readonly_frame(self);
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		return err_dead_frame(self);
	}
	if (!DeeFrame_PLockTryWrite(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitWrite(self))
			goto err;
		goto again;
	}
	return 0;
err:
	return -1;

}

PRIVATE NONNULL((1)) void DCALL
frame_lockendwrite_frame(Frame *__restrict self) {
	DeeFrame_PLockEndWrite(self);
	DeeFrame_LockEndRead(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_thisarg(Frame *__restrict self) {
	DREF DeeObject *result;
	if unlikely(frame_lockread_frame(self))
		goto err;
	if unlikely(!(self->f_frame->cf_func->fo_code->co_flags & CODE_FTHISCALL))
		goto err_unlock_unbound;
	result = self->f_frame->cf_this;
	if unlikely(!result)
		goto err_unlock_unbound;
	Dee_Incref(result);
	frame_lockendread_frame(self);
	return result;
err_unlock_unbound:
	frame_lockendread_frame(self);
	err_unbound_attribute_string(&DeeFrame_Type, "__thisarg__");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
frame_bound_thisarg(Frame *__restrict self) {
	bool is_bound;
	int temp = frame_lockread_frame_for_bound(self);
	if (temp > 0)
		return 0;
	if unlikely(temp)
		goto err;
	is_bound = (self->f_frame->cf_func->fo_code->co_flags & CODE_FTHISCALL) != 0 &&
	           (self->f_frame->cf_this != NULL);
	frame_lockendread_frame(self);
	return is_bound ? 1 : 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_return(Frame *__restrict self) {
	DREF DeeObject *result;
	if unlikely(frame_lockread_frame(self))
		goto err;
	result = self->f_frame->cf_result;
	if unlikely(!ITER_ISOK(result))
		goto err_unlock_unbound;
	Dee_Incref(result);
	frame_lockendread_frame(self);
	return result;
err_unlock_unbound:
	frame_lockendread_frame(self);
	err_unbound_attribute_string(&DeeFrame_Type, "__return__");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
frame_bound_return(Frame *__restrict self) {
	bool is_bound;
	int temp = frame_lockread_frame_for_bound(self);
	if (temp > 0)
		return 0;
	if unlikely(temp)
		goto err;
	is_bound = ITER_ISOK(self->f_frame->cf_result);
	frame_lockendread_frame(self);
	return is_bound ? 1 : 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
frame_del_return(Frame *__restrict self) {
	DREF DeeObject *old_result;
	if unlikely(frame_lockwrite_frame(self))
		goto err;
	old_result = self->f_frame->cf_result;
	self->f_frame->cf_result = NULL;
	frame_lockendwrite_frame(self);
	if (ITER_ISOK(old_result))
		Dee_Decref(old_result);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
frame_set_return(Frame *__restrict self, DeeObject *value) {
	DREF DeeObject *old_result;
	if unlikely(frame_lockwrite_frame(self))
		goto err;
	Dee_Incref(value);
	old_result = self->f_frame->cf_result;
	self->f_frame->cf_result = value;
	frame_lockendwrite_frame(self);
	if (ITER_ISOK(old_result))
		Dee_Decref(old_result);
	return 0;
err:
	return -1;
}

#ifdef CONFIG_EXPERIMENTAL_STATIC_IN_FUNCTION
PRIVATE WUNUSED NONNULL((1)) int DCALL
frame_bound_frame(Frame *__restrict self) {
	return atomic_read(&self->f_frame) != NULL ? 1 : 0;
}

#define frame_bound_function_statics frame_bound_frame
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_function_statics(Frame *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFunctionObject *func;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		goto err_dead;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	func = self->f_frame->cf_func;
	Dee_Incref(func);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	result = DeeFunction_GetStaticsWrapper(func);
	Dee_Decref_unlikely(func);
	return result;
err_dead:
	err_dead_frame(self);
err:
	return NULL;
}

#define frame_bound_function_refs frame_bound_frame
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_function_refs(Frame *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFunctionObject *func;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		goto err_dead;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	func = self->f_frame->cf_func;
	Dee_Incref(func);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	result = DeeRefVector_NewReadonly((DeeObject *)func,
	                                  func->fo_code->co_refc,
	                                  func->fo_refv);
	Dee_Decref_unlikely(func);
	return result;
err_dead:
	err_dead_frame(self);
err:
	return NULL;
}

#define frame_bound_function_kwds frame_bound_frame
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_function_kwds(Frame *__restrict self) {
	DeeCodeObject *code;
	DREF DeeObject *result;
	DREF DeeFunctionObject *func;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		goto err_dead;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	func = self->f_frame->cf_func;
	Dee_Incref(func);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	code = func->fo_code;
	if unlikely(!code->co_keywords)
		goto err_unbound_func;
	result = DeeRefVector_NewReadonly((DeeObject *)code,
	                                  (size_t)code->co_argc_max,
	                                  (DeeObject *const *)code->co_keywords);
	Dee_Decref_unlikely(func);
	return result;
err_unbound_func:
	Dee_Decref_unlikely(func);
	err_unbound_attribute_string(&DeeFrame_Type, "__kwds__");
err:
	return NULL;
err_dead:
	err_dead_frame(self);
	goto err;
}

#define frame_bound_function_refsbyname frame_bound_frame
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_function_refsbyname(Frame *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFunctionObject *func;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		goto err_dead;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	func = self->f_frame->cf_func;
	Dee_Incref(func);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	result = DeeFunction_GetRefsByNameWrapper(func);
	Dee_Decref_unlikely(func);
	return result;
err_dead:
	err_dead_frame(self);
err:
	return NULL;
}

#define frame_bound_function_staticsbyname frame_bound_frame
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_function_staticsbyname(Frame *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFunctionObject *func;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		goto err_dead;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	func = self->f_frame->cf_func;
	Dee_Incref(func);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	result = DeeFunction_GetStaticsByNameWrapper(func);
	Dee_Decref_unlikely(func);
	return result;
err_dead:
	err_dead_frame(self);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL code_getdefaults(DeeCodeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL code_getconstants(DeeCodeObject *__restrict self);

#define frame_bound_code_defaults frame_bound_frame
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_code_defaults(DeeFrameObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFunctionObject *func;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		goto err_dead;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	func = self->f_frame->cf_func;
	Dee_Incref(func);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	result = code_getdefaults(func->fo_code);
	Dee_Decref_unlikely(func);
	return result;
err_dead:
	err_dead_frame(self);
err:
	return NULL;
}

#define frame_bound_code_constants frame_bound_frame
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_get_code_constants(DeeFrameObject *__restrict self) {
	DREF DeeObject *result;
	DREF DeeFunctionObject *func;
again:
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		goto err_dead;
	}
	if (!DeeFrame_PLockTryRead(self)) {
		DeeFrame_LockEndRead(self);
		if unlikely(DeeFrame_PLockWaitRead(self))
			goto err;
		goto again;
	}
	func = self->f_frame->cf_func;
	Dee_Incref(func);
	DeeFrame_PLockEndRead(self);
	DeeFrame_LockEndRead(self);
	result = code_getconstants(func->fo_code);
	Dee_Decref_unlikely(func);
	return result;
err_dead:
	err_dead_frame(self);
err:
	return NULL;
}
#endif /* CONFIG_EXPERIMENTAL_STATIC_IN_FUNCTION */


PRIVATE struct type_getset tpconst frame_getsets[] = {
	TYPE_GETTER("location", &frame_getlocation,
	            "->?S?T4?X2?Dstring?N?X2?Dint?N?X2?Dint?N?X2?Dstring?N\n"
	            "Returns a sequence of tuples describing the Frame location, "
	            /**/ "the first of which is identical to (?#file, ?#line, ?#col, ?#name)\n"
	            "Rarely ever does the location consist of more than a single "
	            /**/ "location tuple, however if a function call has been inlined "
	            /**/ "as a call from another location, the compiler will generate DDI "
	            /**/ "instrumentation to ensure consistent debug information for both "
	            /**/ "the inlined function, as well as the call-site"),
	TYPE_GETTER("file", &frame_getfile,
	            "->?X2?Dstring?N\n"
	            "The filename of this Frame's source file, or ?N when indeterminate"),
	TYPE_GETTER("line", &frame_getline,
	            "->?X2?Dint?N\n"
	            "The 0-based line number within this Frame's source file, or ?N when indeterminate"),
	TYPE_GETTER("col", &frame_getcol,
	            "->?X2?Dint?N\n"
	            "The 0-based column offset within this Frame's source file, or ?N when indeterminate"),
	TYPE_GETTER("name", &frame_getname,
	            "->?X2?Dstring?N\n"
	            "The name of this Frame's function, or ?N when indeterminate"),
	TYPE_GETTER("func", &frame_getfunc,
	            "->?DFunction\n"
	            DOC_ReferenceError
	            "Returns the function that is referenced by @this Frame"),
	TYPE_GETTER("__func__", &frame_getfunc,
	            "->?DFunction\n"
	            DOC_ReferenceError
	            "Alias for ?#func"),
	TYPE_GETTER("__code__", &frame_getcode,
	            "->?Ert:Code\n"
	            DOC_ReferenceError
	            "The code object that is being executed"),
	TYPE_GETSET("__pc__", &frame_getpc, NULL, &frame_setpc,
	            "->?Dint\n"
	            DOC_ReferenceError
	            "#tValueError{Attempted to set PC within a read-only Frame}"
	            "The current program counter"),
	TYPE_GETTER("__sp__", &frame_getsp_obj,
	            "->?Dint\n"
	            DOC_ReferenceError
	            "#tValueError{The stack depth was undefined and could not be determined}"
	            "Get the current stack depth (same as ${##this.__stack__})\n"
	            "To modify this value, use ?#__stack__ to append/pop objects"),
	TYPE_GETTER_BOUND("__thisarg__", &frame_get_thisarg, &frame_bound_thisarg,
	                  "->?O\n"
	                  DOC_ReferenceError
	                  "#tUnboundAttribute{The associated code doesn't take a this-argument}"
	                  "Returns the special $this argument linked to @this frame"),
	TYPE_GETSET_BOUND("__return__",
	                  &frame_get_return, &frame_del_return, &frame_set_return, &frame_bound_return,
	                  "->?O\n"
	                  DOC_ReferenceError
	                  "#tValueError{The Frame is readonly}"
	                  "#tUnboundAttribute{No return value is assigned at the moment}"
	                  "Read-write access to the currently set frame return value"),
#ifdef CONFIG_EXPERIMENTAL_STATIC_IN_FUNCTION
	TYPE_GETTER_BOUND("__statics__", &frame_get_function_statics, &frame_bound_function_statics,
	                  "->?S?O\n"
	                  DOC_ReferenceError
	                  "Alias for ?A__statics__?DFunction through ?#func"),
	TYPE_GETTER_BOUND("__refs__", &frame_get_function_refs, &frame_bound_function_refs,
	                  "->?S?O\n"
	                  DOC_ReferenceError
	                  "Alias for ?A__refs__?DFunction through ?#func"),
	TYPE_GETTER_BOUND("__kwds__", &frame_get_function_kwds, &frame_bound_function_kwds,
	                  "->?S?Dstring\n"
	                  DOC_ReferenceError
	                  "#tUnboundAttribute{The associated ?#__code__ doesn't have keyword argument support}"
	                  "Alias for ?A__kwds__?DFunction through ?#func"),
	TYPE_GETTER_BOUND("__refsbyname__", &frame_get_function_refsbyname, &frame_bound_function_refsbyname,
	                  "->?M?X2?Dstring?Dint?O\n"
	                  DOC_ReferenceError
	                  "Alias for ?A__refsbyname__?DFunction through ?#func"),
	TYPE_GETTER_BOUND("__staticsbyname__", &frame_get_function_staticsbyname, &frame_bound_function_staticsbyname,
	                  "->?M?X2?Dstring?Dint?O\n"
	                  DOC_ReferenceError
	                  "Alias for ?A__staticsbyname__?DFunction through ?#func"),
	TYPE_GETTER("__stack__", &DeeFrame_GetStackWrapper,
	            "->?S?O\n"
	            "Returns (possibly) resizable (up to a certain limit), (possibly) mutable "
	            /**/ "sequence that can be used to read/write the values of the stack"),
	TYPE_GETTER("__args__", &DeeFrame_GetArgsWrapper,
	            "->?S?O\n"
	            "Returns a read-only sequence for accessing arguments passed to this frame. "
	            /**/ "Use the 0-based index of the argument with this sequence to access an "
	            /**/ "argument the same way the #C{push arg <n>} instruction would"),
	TYPE_GETTER("__locals__", &DeeFrame_GetLocalsWrapper,
	            "->?S?O\n"
	            "Returns fixed-length, (possibly) mutable sequence that can be "
	            /**/ "used to read/write the values of local variables. Note that "
	            /**/ "depending on optimization, some local variables may not exist "
	            /**/ "even when they should still be in-scope, and that some local "
	            /**/ "variables may appear already be assigned prior to initialization "
	            /**/ "as a result of local variable re-use"),
	TYPE_GETTER("__argsbyname__", &DeeFrame_GetArgsByNameWrapper,
	            "->?M?X2?Dstring?Dint?O\n"
	            "Combine ?#__kwds__ with ?#__args__ to access the values of arguments"),
	TYPE_GETTER("__variablesbyname__", &DeeFrame_GetVariablesByNameWrapper,
	            "->?M?X2?Dstring?Dint?O\n"
	            "Combine ?#__locals__ and ?#__stack__ with debug information to form a writable "
	            /**/ "mapping that can be used to manipulate the values of named locals and stack "
	            /**/ "locations. (requires debug information to be present)"),
	TYPE_GETTER("__symbols__", &DeeFrame_GetSymbolsByNameWrapper,
	            "->?M?X2?Dstring?Dint?O\n"
	            "The combination of ?#__refsbyname__, ?#__staticsbyname__, ?#__argsbyname__ "
	            /**/ "and ?#__variablesbyname__, allowing access to all named symbol. "
	            /**/ "(requires debug information to be present)"),
	TYPE_GETTER_BOUND_F("__defaults__", &frame_get_code_defaults, &frame_bound_code_defaults,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?O\n"
	                    DOC_ReferenceError
	                    "Alias for :Function.__defaults__ though ?#__func__"),
	TYPE_GETTER_BOUND_F("__constants__", &frame_get_code_constants, &frame_bound_code_constants,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?S?O\n"
	                    DOC_ReferenceError
	                    "Alias for :Function.__constants__ though ?#__func__"),
#endif /* CONFIG_EXPERIMENTAL_STATIC_IN_FUNCTION */
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst frame_members[] = {
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT, offsetof(Frame, f_owner)),
	TYPE_MEMBER_BITFIELD_DOC("__iswritable__", STRUCT_CONST, Frame, f_flags, DEEFRAME_FWRITABLE,
	                         "Evaluates to ?t if @this Frame is writable"),
	TYPE_MEMBER_END
};

PUBLIC DeeTypeObject DeeFrame_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Frame),
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(Frame)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&frame_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL,
		/* .tp_print = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&frame_print
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&frame_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ frame_getsets,
	/* .tp_members       = */ frame_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_FRAME_C */
