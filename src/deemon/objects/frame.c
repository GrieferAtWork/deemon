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
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <stdarg.h>
#include <stddef.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeFrameObject Frame;

#ifndef CONFIG_NO_THREADS
#define PLOCK_READ(x)                                               \
	((x)->f_plock ? ((x)->f_flags & DEEFRAME_FRECLOCK               \
	                 ? (void)Dee_ratomic_rwlock_read((x)->f_prlock) \
	                 : (void)Dee_atomic_rwlock_read((x)->f_plock))  \
	              : (void)0)
#define PLOCK_TRYREAD(x)                                         \
	((x)->f_plock ? ((x)->f_flags & DEEFRAME_FRECLOCK            \
	                 ? Dee_ratomic_rwlock_tryread((x)->f_prlock) \
	                 : Dee_atomic_rwlock_tryread((x)->f_plock))  \
	              : true)
#define PLOCK_WRITE(x)                                                    \
	((x)->f_flags & DEEFRAME_FWRITABLE                                    \
	 ? ((x)->f_plock ? ((x)->f_flags & DEEFRAME_FRECLOCK                  \
	                    ? (Dee_ratomic_rwlock_write((x)->f_prlock), true) \
	                    : (Dee_atomic_rwlock_write((x)->f_plock)),        \
	                    true)                                             \
	                 : true)                                              \
	 : false)
#define PLOCK_TRYWRITE(x)                                            \
	((x)->f_flags & DEEFRAME_FWRITABLE                               \
	 ? ((x)->f_plock ? ((x)->f_flags & DEEFRAME_FRECLOCK             \
	                    ? Dee_ratomic_rwlock_trywrite((x)->f_prlock) \
	                    : Dee_atomic_rwlock_trywrite((x)->f_plock))  \
	                 : true)                                         \
	 : false)
#define PLOCK_ENDREAD(x)                                               \
	((x)->f_plock ? ((x)->f_flags & DEEFRAME_FRECLOCK                  \
	                 ? (void)Dee_ratomic_rwlock_endread((x)->f_prlock) \
	                 : (void)Dee_atomic_rwlock_endread((x)->f_plock))  \
	              : (void)0)
#define PLOCK_ENDWRITE(x)                                               \
	((x)->f_plock ? ((x)->f_flags & DEEFRAME_FRECLOCK                   \
	                 ? (void)Dee_ratomic_rwlock_endwrite((x)->f_prlock) \
	                 : (void)Dee_atomic_rwlock_endwrite((x)->f_plock))  \
	              : (void)0)
#else /* !CONFIG_NO_THREADS */
#define PLOCK_READ(x)     (void)0
#define PLOCK_TRYREAD(x)  true
#define PLOCK_WRITE(x)    ((x)->f_flags & DEEFRAME_FWRITABLE)
#define PLOCK_TRYWRITE(x) ((x)->f_flags & DEEFRAME_FWRITABLE)
#define PLOCK_ENDREAD(x)  (void)0
#define PLOCK_ENDWRITE(x) (void)0
#endif /* CONFIG_NO_THREADS */

/* Construct a frame object owned by `owner'
 * The intended use of this is for tracebacks and yield_function-iterators.
 * @param: flags: Set of `DEEFRAME_F*' */
#ifndef CONFIG_NO_THREADS
PUBLIC WUNUSED NONNULL((2)) DREF DeeObject *
(DCALL DeeFrame_NewReferenceWithLock)(DeeObject *owner,
                                      struct code_frame *__restrict frame,
                                      uint16_t flags, void *lock)
#else /* !CONFIG_NO_THREADS */
PUBLIC WUNUSED NONNULL((2)) DREF DeeObject *
(DCALL DeeFrame_NewReference)(DeeObject *owner,
                              struct code_frame *__restrict frame,
                              uint16_t flags)
#endif /* CONFIG_NO_THREADS */
{
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
	result->f_plock = (Dee_atomic_rwlock_t *)lock;
#endif /* !CONFIG_NO_THREADS */
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



#ifdef CONFIG_HOST_WINDOWS
#define TRACEBACK_SLASH(path, file) (strchr(path, '\\') || strchr(file, '\\') ? "\\" : "/")
#else /* CONFIG_HOST_WINDOWS */
#define TRACEBACK_SLASH(path, file) "/"
#define TRACEBACK_SLASH_S           "/"
#endif /* !CONFIG_HOST_WINDOWS */

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
	DeeFrame_LockRead(self);
	if (!self->f_frame) {
		DeeFrame_LockEndRead(self);
		return 0;
	}
	PLOCK_READ(self);
	code = self->f_frame->cf_func->fo_code;
	Dee_Incref(code);
	ip = (code_addr_t)(self->f_frame->cf_ip - code->co_code);
	PLOCK_ENDREAD(self);
	DeeFrame_LockEndRead(self);
	result = print_ddi(printer, arg, code, ip);
	Dee_Decref_unlikely(code);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeCodeObject *DCALL
frame_getddi(Frame *__restrict self,
             struct ddi_state *__restrict state,
             code_addr_t *pstartip,
             code_addr_t *pendip,
             unsigned int flags) {
	uint8_t *result;
	code_addr_t startip;
	DREF DeeCodeObject *code;
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		return (DREF DeeCodeObject *)ITER_DONE;
	}
	PLOCK_READ(self);
	code = self->f_frame->cf_func->fo_code;
	Dee_Incref(code);
	startip = (code_addr_t)(self->f_frame->cf_ip -
	                        code->co_code);
	PLOCK_ENDREAD(self);
	DeeFrame_LockEndRead(self);
	if (pstartip)
		*pstartip = startip;
	result = DeeCode_FindDDI((DeeObject *)code,
	                         state,
	                         pendip,
	                         startip,
	                         flags);
	if (DDI_ISOK(result))
		return code;
	Dee_Decref(code);
	return result == DDI_NEXT_DONE
	       ? (DREF DeeCodeObject *)ITER_DONE
	       : (DREF DeeCodeObject *)NULL;
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
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame) {
		DeeFrame_LockEndRead(self);
		return_none;
	}
	PLOCK_READ(self);
	result = self->f_frame->cf_func;
	Dee_Incref(result);
	PLOCK_ENDREAD(self);
	DeeFrame_LockEndRead(self);
	return (DREF DeeObject *)result;
}

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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getcode(Frame *__restrict self) {
	DREF DeeCodeObject *result;
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame)
		goto err_df;
	PLOCK_READ(self);
	result = self->f_frame->cf_func->fo_code;
	Dee_Incref(result);
	PLOCK_ENDREAD(self);
	DeeFrame_LockEndRead(self);
	return (DREF DeeObject *)result;
err_df:
	DeeFrame_LockEndRead(self);
	err_dead_frame(self);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getpc(Frame *__restrict self) {
	code_addr_t pc;
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame)
		goto err_df;
	PLOCK_READ(self);
	pc = (code_addr_t)(self->f_frame->cf_ip -
	                   self->f_frame->cf_func->fo_code->co_code);
	PLOCK_ENDREAD(self);
	DeeFrame_LockEndRead(self);
	return DeeInt_NewU32(pc);
err_df:
	DeeFrame_LockEndRead(self);
	err_dead_frame(self);
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
 * @return: -2: The frame has continued execution, or was otherwise released. (no error was thrown) */
PRIVATE WUNUSED NONNULL((1)) int32_t DCALL
frame_getsp(Frame *__restrict self) {
	int32_t result;
	uint16_t flags = atomic_read(&self->f_flags);
	if (!(flags & DEEFRAME_FUNDEFSP)) {
		DeeFrame_LockRead(self);
		if unlikely(!self->f_frame) {
			DeeFrame_LockEndRead(self);
			return -2;
		}
		if (flags & DEEFRAME_FREGENGSP) {
			result = self->f_revsp;
		} else {
			PLOCK_READ(self);
			result = (int32_t)(self->f_frame->cf_sp -
			                   self->f_frame->cf_stack);
			PLOCK_ENDREAD(self);
		}
		DeeFrame_LockEndRead(self);
		return result;
	}
	if (flags & DEEFRAME_FUNDEFSP2)
		return -1;
	/* Try to reverse-engineer stack information. */
	return frame_revengsp(self);
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
	DeeFrame_LockRead(self);
	if unlikely(!self->f_frame)
		goto err_df;
	PLOCK_READ(self);
	self->f_frame->cf_ip = self->f_frame->cf_func->fo_code->co_code + pc;
	PLOCK_ENDREAD(self);
	DeeFrame_LockEndRead(self);
	return 0;
err_df:
	DeeFrame_LockEndRead(self);
	err_dead_frame(self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
frame_getsp_obj(Frame *__restrict self) {
	int32_t result = frame_getsp(self);
	if unlikely(result == -2)
		goto err_df;
	if unlikely(result == -1)
		goto err_unknown;
	return DeeInt_NewU16((uint16_t)result);
err_unknown:
	DeeError_Throwf(&DeeError_ValueError,
	                "Stack depth is unknown");
	goto err;
err_df:
	err_dead_frame(self);
err:
	return NULL;
}

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
	            "->?X2?DFunction?N\n"
	            "Returns the function that is referenced by @this Frame, or ?N if not available"),
	TYPE_GETTER("__code__", &frame_getcode,
	            "->?Ert:Code\n"
	            "@throw ReferenceError The Frame has continued execution, or was otherwise released\n"
	            "The code object that is being executed"),
	TYPE_GETSET("__pc__", &frame_getpc, NULL, &frame_setpc,
	            "->?Dint\n"
	            "@throw ReferenceError The Frame has continued execution, or was otherwise released\n"
	            "@throw ValueError Attempted to set PC within a read-only Frame\n"
	            "The current program counter"),
	TYPE_GETTER("__sp__", &frame_getsp_obj,
	            "->?Dint\n"
	            "@throw ReferenceError The Frame has continued execution, or was otherwise released\n"
	            "@throw ValueError The stack depth was undefined and could not be determined\n"
	            "Get the current stack depth (same as ${##this.__stack__})\n"
	            "To modify this value, use ?#__stack__ to append/pop objects"),
	/* TODO: __stack__ (read-write, custom, modifiable & resizable, sequence-like object)
	 * TODO: __args__ (readonly, custom, sequence-like object for accessing arguments)
	 *                 -> reading is the same as it would for the `push arg ...' instruction
	 * TODO: __thisarg__ (readonly) access to the this argument
	 * TODO: __return__ (read-write) access to the return register
	 * TODO: __locals__ (custom, modifiable, sequence-like object)
	 * TODO: __variables__ (A special sequence that combines `__locals__' with `__stack__',
	 *                      allowing for universal indices to identify objects with local
	 *                      life-time, which are then returned by other members, such as `__names__')
	 *                      When the stack depth is unknown, simply return `__locals__'
	 * TODO: __names__ (Access to a mapping-like object for converting local/stack
	 *                  names to indices into the `__variables__' sequence)
	 *                  >> local vars = f.__variables__;
	 *                  >> for (local name, addr: f.__names__) {
	 *                  >>     print "{}@{} = {}".format({
	 *                  >>         name,
	 *                  >>         addr,
	 *                  >>         vars[addr] is bound ? repr vars[addr] : "<unbound>"
	 *                  >>     });
	 *                  >> }
	 * TODO: __symbols__ (custom, modifiable (for some elements), mapping-like object)
	 *                 -> By using DDI information for names of locals/stack symbols,
	 *                    and combining this information with knowledge of argument
	 *                    names, as well as adding special cases for `this' and `return',
	 *                    provide access to symbols visible within user-code via their
	 *                    individual names.
	 *                    Resolution name order here is (from most->least visible):
	 *                    LOCALS/STACK (DDI) -> ARGUMENTS -> STATIC/CONST -> REF -> `return' + `this'
	 */
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
