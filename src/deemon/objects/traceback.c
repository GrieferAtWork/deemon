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
#ifndef GUARD_DEEMON_OBJECTS_TRACEBACK_C
#define GUARD_DEEMON_OBJECTS_TRACEBACK_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpy(), ... */
#include <deemon/thread.h>
#include <deemon/traceback.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
/**/

#include <stddef.h> /* size_t, offsetof */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

INTDEF DeeThreadObject DeeThread_Main;
INTERN struct empty_traceback_object DeeTraceback_Empty = {
	OBJECT_HEAD_INIT(&DeeTraceback_Type),
	/* .tb_thread    = */ &DeeThread_Main,
#ifndef CONFIG_NO_THREADS
	/* .tb_lock      = */ DEE_ATOMIC_LOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	/* .tb_numframes = */ 0,
	/* .tb_padding   = */ { 0, 0, 0 }
};



/* Same as `DeeTraceback_New()', but throw errors when returning NULL. */
INTERN WUNUSED NONNULL((1)) DREF DeeTracebackObject *DCALL
DeeTraceback_NewWithException(struct thread_object *__restrict thread) {
	DREF DeeTracebackObject *result;
	/* `DeeTraceback_New()' is special in that it doesn't throw exceptions.
	 * However, we want there to be an out-of-memory exception when we're
	 * unable to generate a traceback, so implement that behavior here. */
	do {
		result = DeeTraceback_New(thread);
	} while (result == NULL && Dee_CollectMemory(1));
	return result;
}


/* Try to create a new traceback, but don't throw
 * an error and return `NULL' if doing so failed.
 * NOTE: The given `thread' must be the caller's. */
INTERN WUNUSED NONNULL((1)) DREF DeeTracebackObject *DCALL
DeeTraceback_New(struct thread_object *__restrict thread) {
	struct code_frame *dst, *src;
	DREF DeeTracebackObject *result;
	ASSERTF(thread == DeeThread_Self(), "Traceback for other threads must be created using `DeeThread_Trace()'");
	result = (DREF DeeTracebackObject *)DeeGCObject_TryMallocc(offsetof(DeeTracebackObject, tb_frames),
	                                                           thread->t_execsz, sizeof(struct code_frame));
	if unlikely(!result)
		goto err;

	/* TODO: This somehow needs support for functions running under _hostasm. */
	/* TODO: This somehow needs support for functions running under _jit. */
	DeeObject_Init(result, &DeeTraceback_Type);
	result->tb_numframes = thread->t_execsz;
	result->tb_thread    = thread;
	Dee_Incref(thread);
	Dee_atomic_lock_init(&result->tb_lock);
	dst = result->tb_frames + thread->t_execsz;
	src = thread->t_exec;
	while (dst > result->tb_frames) {
		DeeCodeObject *code;
		DeeObject *dont_track_this = NULL;
		--dst;
		ASSERT(src != NULL);
		ASSERT(src != CODE_FRAME_NOT_EXECUTING);

		/* Do a shallow memcpy of the execution frame. */
		memcpy(dst, src, sizeof(struct code_frame));

		/* Create references and duplicate local variables. */
		ASSERT_OBJECT_TYPE(dst->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE(dst->cf_func->fo_code, &DeeCode_Type);
		Dee_Incref(dst->cf_func);
		code          = dst->cf_func->fo_code;
		dst->cf_flags = code->co_flags;
		if (code->co_flags & CODE_FTHISCALL) {
			ASSERT_OBJECT(dst->cf_this);
			if (!(code->co_flags & CODE_FCONSTRUCTOR)) {
				Dee_Incref(dst->cf_this);
			} else {
				dont_track_this = dst->cf_this;
				dst->cf_this    = NULL;
			}
		} else {
			dst->cf_this = NULL;
		}
		if (!dst->cf_argc) {
			dst->cf_argv = NULL;
		} else {
			dst->cf_argv = (DREF DeeObject **)Dee_TryMallocc(dst->cf_argc,
			                                                 sizeof(DREF DeeObject *));
			if (dst->cf_argv) {
				size_t i;
				for (i = 0; i < dst->cf_argc; ++i) {
					DeeObject *ob = src->cf_argv[i];
					if (ob == dont_track_this)
						ob = Dee_None;
					Dee_Incref(ob);
					((DREF DeeObject **)dst->cf_argv)[i] = ob;
				}
			}
		}
		Dee_XIncref(dst->cf_vargs);
		if (ITER_ISOK(dst->cf_result))
			Dee_Incref(dst->cf_result);

		/* Duplicate local variables. */
		dst->cf_frame = (DREF DeeObject **)Dee_TryMallocc(code->co_localc,
		                                                  sizeof(DREF DeeObject *));
		if (dst->cf_frame) {
			size_t i;
			for (i = 0; i < code->co_localc; ++i) {
				DeeObject *ob = src->cf_frame[i];
				if (ob == dont_track_this)
					ob = Dee_None;
				Dee_XIncref(ob);
				dst->cf_frame[i] = ob;
			}
		}

		/* At this point, the contents of the stack can't be trusted. */
		dst->cf_stack   = (DREF DeeObject **)dont_track_this; /* Save this here so that `DeeTraceback_AddFrame()' sees it. */
		dst->cf_sp      = NULL;
		dst->cf_stacksz = 0;

		/* Continue with the next frame. */
		src = src->cf_prev;
	}
	ASSERT(src == NULL);
	return (DREF DeeTracebackObject *)DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

/* Fill in stack information in the given traceback for `frame'. */
INTERN NONNULL((1, 2)) void DCALL
DeeTraceback_AddFrame(DeeTracebackObject *__restrict self,
                      struct code_frame *__restrict frame,
                      uint16_t frame_id) {
	struct code_frame *dst;
	uint16_t stacksz;
	DeeObject *dont_track_this;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeTraceback_Type);
	if unlikely(frame_id >= self->tb_numframes)
		return; /* Untracked frame. */
	dst = &self->tb_frames[frame_id];
	if unlikely(dst->cf_prev != frame->cf_prev)
		return; /* Different frame. */
	if unlikely(self->tb_thread != DeeThread_Self())
		return; /* Different thread. */
	DeeTraceback_LockAcquire(self);
	if unlikely(dst->cf_stacksz)
		goto done; /* Frame already initialized */
	ASSERT(!dst->cf_sp);
	ASSERT(!dst->cf_stacksz);
	dont_track_this = (DeeObject *)dst->cf_stack;
	dst->cf_stack   = NULL;

	/* Since we can't report errors, only ~try~ to copy stacks. */
	stacksz = (uint16_t)(frame->cf_sp - frame->cf_stack);
	if unlikely(!stacksz)
		goto done;
	dst->cf_stack = (DREF DeeObject **)Dee_TryMallocc(stacksz, sizeof(DREF DeeObject *));
	if likely(dst->cf_stack) {
		dst->cf_stacksz = stacksz;
		dst->cf_sp      = dst->cf_stack + stacksz;

		/* Duplicate the stack. */
		while (stacksz--) {
			dst->cf_stack[stacksz] = frame->cf_stack[stacksz];
			if (dst->cf_stack[stacksz] == dont_track_this)
				dst->cf_stack[stacksz] = Dee_None;
			ASSERT_OBJECT(dst->cf_stack[stacksz]);
			Dee_Incref(dst->cf_stack[stacksz]);
		}
	}
done:
	DeeTraceback_LockRelease(self);
}


typedef struct {
	OBJECT_HEAD
	DREF DeeTracebackObject *ti_trace; /* [1..1][const] The traceback that is being iterated. */
	struct code_frame       *ti_next;  /* [1..1][in(ti_trace->tb_frames)][atomic]
	                                    * The next frame (yielded in reverse order) */
} TraceIterator;
#define READ_NEXT(x)     atomic_read(&(x)->ti_next)
#define WRITE_NEXT(x, y) atomic_write(&(x)->ti_next, y)

INTDEF DeeTypeObject DeeTracebackIterator_Type;

PRIVATE WUNUSED NONNULL((1)) int DCALL
traceiter_ctor(TraceIterator *__restrict self) {
	self->ti_trace = (DREF DeeTracebackObject *)&DeeTraceback_Empty;
	self->ti_next  = self->ti_trace->tb_frames;
	Dee_Incref(&DeeTraceback_Empty);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
traceiter_copy(TraceIterator *__restrict self,
               TraceIterator *__restrict other) {
	self->ti_trace = other->ti_trace;
	self->ti_next  = READ_NEXT(other);
	Dee_Incref(self->ti_trace);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
traceiter_deep(TraceIterator *__restrict self,
               TraceIterator *__restrict other) {
	size_t index;
	self->ti_trace = (DREF DeeTracebackObject *)DeeObject_DeepCopy((DeeObject *)other->ti_trace);
	if unlikely(!self->ti_trace)
		goto err;
	index         = (size_t)(READ_NEXT(other) - other->ti_trace->tb_frames);
	self->ti_next = self->ti_trace->tb_frames + index;
	Dee_Incref(self->ti_trace);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
traceiter_init(TraceIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	size_t index = 0;
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ ":_TracebackIterator",
	                  &self->ti_trace, &index))
		goto err;
	if (DeeObject_AssertTypeExact(self->ti_trace, &DeeTraceback_Type))
		goto err;
	if (index > self->ti_trace->tb_numframes)
		index = self->ti_trace->tb_numframes;
	self->ti_next = self->ti_trace->tb_frames +
	                (self->ti_trace->tb_numframes - (index + 1));
	Dee_Incref(self->ti_trace);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
traceiter_fini(TraceIterator *__restrict self) {
	Dee_Decref(self->ti_trace);
}

PRIVATE NONNULL((1, 2)) void DCALL
traceiter_visit(TraceIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ti_trace);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
traceiter_bool(TraceIterator *__restrict self) {
	return READ_NEXT(self) >= self->ti_trace->tb_frames;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
traceiter_next(TraceIterator *__restrict self) {
	struct code_frame *result_frame;
	do {
		result_frame = atomic_read(&self->ti_next);
		if unlikely(result_frame < self->ti_trace->tb_frames)
			return ITER_DONE;
	} while (!atomic_cmpxch_weak_or_write(&self->ti_next, result_frame, result_frame - 1));

	/* Create a new frame wrapper for this entry. */
	return DeeFrame_NewReferenceWithLock((DeeObject *)self->ti_trace,
	                                     result_frame,
	                                     DEEFRAME_FREADONLY,
	                                     &self->ti_trace->tb_lock);
}

PRIVATE struct type_member tpconst traceiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(TraceIterator, ti_trace), "->?DTraceback"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeTracebackObject *DCALL
traceiter_nii_getseq(TraceIterator *__restrict self) {
	return_reference_(self->ti_trace);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
traceiter_nii_getindex(TraceIterator *__restrict self) {
	return (size_t)((self->ti_trace->tb_frames + (self->ti_trace->tb_numframes - 1)) -
	                READ_NEXT(self));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
traceiter_nii_setindex(TraceIterator *__restrict self, size_t index) {
	if (index > self->ti_trace->tb_numframes)
		index = self->ti_trace->tb_numframes;
	WRITE_NEXT(self,
	           self->ti_trace->tb_frames +
	           ((self->ti_trace->tb_numframes - 1) - index));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
traceiter_nii_rewind(TraceIterator *__restrict self) {
	WRITE_NEXT(self,
	           self->ti_trace->tb_frames +
	           (self->ti_trace->tb_numframes - 1));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
traceiter_nii_peek(TraceIterator *__restrict self) {
	struct code_frame *result_frame;
	result_frame = READ_NEXT(self);
	if unlikely(result_frame < self->ti_trace->tb_frames)
		return ITER_DONE;

	/* Create a new frame wrapper for this entry. */
	return DeeFrame_NewReferenceWithLock((DeeObject *)self->ti_trace,
	                                     result_frame,
	                                     DEEFRAME_FREADONLY,
	                                     &self->ti_trace->tb_lock);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
traceiter_nii_hasprev(TraceIterator *__restrict self) {
	return (READ_NEXT(self) <
	        (self->ti_trace->tb_frames +
	         (self->ti_trace->tb_numframes - 1)));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
traceiter_nii_next(TraceIterator *__restrict self) {
	struct code_frame *result_frame;
	do {
		result_frame = atomic_read(&self->ti_next);
		if unlikely(result_frame < self->ti_trace->tb_frames)
			return 1;
	} while (!atomic_cmpxch_weak_or_write(&self->ti_next, result_frame, result_frame - 1));

	/* Create a new frame wrapper for this entry. */
	return 0;
}


PRIVATE struct type_nii tpconst traceiter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&traceiter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&traceiter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&traceiter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&traceiter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL, //TODO:&traceiter_nii_revert,
			/* .nii_advance  = */ (dfunptr_t)NULL, //TODO:&traceiter_nii_advance,
			/* .nii_prev     = */ (dfunptr_t)NULL, //TODO:&traceiter_nii_prev,
			/* .nii_next     = */ (dfunptr_t)&traceiter_nii_next,
			/* .nii_hasprev  = */ (dfunptr_t)&traceiter_nii_hasprev,
			/* .nii_peek     = */ (dfunptr_t)&traceiter_nii_peek
		}
	}
};


PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
traceiter_hash(TraceIterator *self) {
	return Dee_HashPointer(READ_NEXT(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
traceiter_compare(TraceIterator *self, TraceIterator *other) {
	if (DeeObject_AssertTypeExact(other, &DeeTracebackIterator_Type))
		goto err;
	Dee_return_compareT(struct code_frame *, READ_NEXT(other), /* Yes: reverse because yielding also happens in reverse! */
	                    /*                */ READ_NEXT(self));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp traceiter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&traceiter_hash,
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&traceiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &traceiter_nii,
};


INTERN DeeTypeObject DeeTracebackIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_TracebackIterator",
	/* .tp_doc      = */ DOC("(traceback?:?DTraceback,index=!0)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&traceiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&traceiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&traceiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&traceiter_init,
				TYPE_FIXED_ALLOCATOR(TraceIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&traceiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&traceiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&traceiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &traceiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ traceiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__E31EBEB26CC72F83),
};


PRIVATE WUNUSED DREF DeeTracebackObject *DCALL traceback_new(void) {
	return DeeTraceback_NewWithException(DeeThread_Self());
}

PRIVATE NONNULL((1)) void DCALL
traceback_fini(DeeTracebackObject *__restrict self) {
	struct code_frame *frame;
	size_t frame_index;
	Dee_Decref(self->tb_thread);
	frame_index = self->tb_numframes;
	while (frame_index) {
		--frame_index;
		frame = &self->tb_frames[frame_index];
		ASSERT_OBJECT_TYPE(frame->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE(frame->cf_func->fo_code, &DeeCode_Type);

		/* Decref local variables. */
		Dee_XDecrefv(frame->cf_frame, frame->cf_func->fo_code->co_localc);
		Dee_Free(frame->cf_frame);

		/* Decref stack objects. */
		if (frame->cf_stacksz) {
			Dee_Decrefv(frame->cf_stack, frame->cf_stacksz);
			Dee_Free(frame->cf_stack);
		}

		/* Decref argument objects. */
		Dee_Decrefv(frame->cf_argv, frame->cf_argc);
		Dee_Free((void *)frame->cf_argv);

		/* Decref misc. frame objects. */
		Dee_Decref(frame->cf_func);
		Dee_XDecref(frame->cf_this);
		Dee_XDecref(frame->cf_vargs);
		if (ITER_ISOK(frame->cf_result))
			Dee_Decref(frame->cf_result);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
traceback_visit(DeeTracebackObject *__restrict self,
                dvisit_t proc, void *arg) {
	struct code_frame *iter, *end;
	DeeTraceback_LockAcquire(self);
	Dee_Visit(self->tb_thread);
	end = (iter = self->tb_frames) + self->tb_numframes;
	for (; iter < end; ++iter) {
		ASSERT_OBJECT_TYPE(iter->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE(iter->cf_func->fo_code, &DeeCode_Type);

		/* Visit local variables. */
		if (iter->cf_frame)
			Dee_XVisitv(iter->cf_frame, iter->cf_func->fo_code->co_localc);

		/* Visit stack objects. */
		Dee_Visitv(iter->cf_stack, iter->cf_stacksz);

		/* Visit argument objects. */
		Dee_Visitv(iter->cf_argv, iter->cf_argc);
		Dee_Visit(iter->cf_func);
		Dee_XVisit(iter->cf_this);
		Dee_XVisit(iter->cf_vargs);
		if (ITER_ISOK(iter->cf_result))
			Dee_Visit(iter->cf_result);
	}
	DeeTraceback_LockRelease(self);
}

PRIVATE NONNULL((1)) void DCALL
traceback_clear(DeeTracebackObject *__restrict self) {
	DREF DeeObject *decref_later_buffer[64], **decref_later;
	struct code_frame *iter, *end;
again:
	decref_later = decref_later_buffer;
	DeeTraceback_LockAcquire(self);
	iter = self->tb_frames;
	end  = iter + self->tb_numframes;
	for (; iter < end; ++iter) {
		DREF DeeObject **oiter, **oend;
		ASSERT_OBJECT_TYPE(iter->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE(iter->cf_func->fo_code, &DeeCode_Type);

		/* Decref local variables. */
		oend = (oiter = iter->cf_frame) + iter->cf_func->fo_code->co_localc;
		if (oiter) {
			for (; oiter != oend; ++oiter) {
				DeeObject *ob = *oiter;
				if (!ob)
					continue;
				*oiter = NULL;
				if (!Dee_DecrefIfNotOne(ob)) {
					*decref_later++ = ob;
					if (decref_later == COMPILER_ENDOF(decref_later_buffer))
						goto clear_buffer;
				}
			}
		}

		/* Decref stack objects. */
		oend = (oiter = iter->cf_stack) + iter->cf_stacksz;
		for (; oiter != oend; ++oiter) {
			DeeObject *ob = *oiter;
			if (DeeNone_Check(ob))
				continue;
			*oiter = DeeNone_NewRef();
			if (!Dee_DecrefIfNotOne(ob)) {
				*decref_later++ = ob;
				if (decref_later == COMPILER_ENDOF(decref_later_buffer))
					goto clear_buffer;
			}
		}

		/* Decref argument objects. */
		oend = (oiter = (DREF DeeObject **)iter->cf_argv) + iter->cf_argc;
		for (; oiter < oend; ++oiter) {
			DeeObject *ob = *oiter;
			if (DeeNone_Check(ob))
				continue;
			*oiter = DeeNone_NewRef();
			if (!Dee_DecrefIfNotOne(ob)) {
				*decref_later++ = ob;
				if (decref_later == COMPILER_ENDOF(decref_later_buffer))
					goto clear_buffer;
			}
		}
		if (iter->cf_this) {
			DeeObject *ob = iter->cf_this;
			iter->cf_this = NULL;
			if (!Dee_DecrefIfNotOne(ob)) {
				*decref_later++ = ob;
				if (decref_later == COMPILER_ENDOF(decref_later_buffer))
					goto clear_buffer;
			}
		}
		if (iter->cf_vargs) {
			DeeObject *ob  = (DeeObject *)iter->cf_vargs;
			iter->cf_vargs = NULL;
			if (!Dee_DecrefIfNotOne(ob)) {
				*decref_later++ = ob;
				if (decref_later == COMPILER_ENDOF(decref_later_buffer))
					goto clear_buffer;
			}
		}
		if (ITER_ISOK(iter->cf_result)) {
			DeeObject *ob   = iter->cf_result;
			iter->cf_result = NULL;
			if (!Dee_DecrefIfNotOne(ob)) {
				*decref_later++ = ob;
				if (decref_later == COMPILER_ENDOF(decref_later_buffer))
					goto clear_buffer;
			}
		}
	}
	DeeTraceback_LockRelease(self);
	while (decref_later != decref_later_buffer) {
		--decref_later;
		Dee_Decref(*decref_later);
	}
	return;
clear_buffer:
	DeeTraceback_LockRelease(self);
	while (decref_later != decref_later_buffer) {
		--decref_later;
		Dee_Decref(*decref_later);
	}
	goto again;
}

PRIVATE WUNUSED NONNULL((1)) DREF TraceIterator *DCALL
traceback_iter(DeeTracebackObject *__restrict self) {
	TraceIterator *result;

	/* Create a new traceback iterator object. */
	result = DeeObject_MALLOC(TraceIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeTracebackIterator_Type);
	result->ti_next  = self->tb_frames + self->tb_numframes - 1;
	result->ti_trace = self;
	Dee_Incref(self);
done:
	return result;
}

INTDEF WUNUSED NONNULL((1, 3)) dssize_t DCALL
print_ddi(dformatprinter printer, void *arg,
          DeeCodeObject *__restrict code, code_addr_t ip);

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
traceback_print(DeeTracebackObject *__restrict self,
                dformatprinter printer, void *arg) {
	dssize_t temp, result = 0;
	uint16_t i = self->tb_numframes;
	while (i) {
		DREF DeeCodeObject *code;
		code_addr_t ip;
		--i;
		DeeTraceback_LockAcquire(self);
		code = self->tb_frames[i].cf_func->fo_code;
		Dee_Incref(code);
		ip = (code_addr_t)(self->tb_frames[i].cf_ip - code->co_code);
		DeeTraceback_LockRelease(self);
		temp = print_ddi(printer, arg, code, ip);
		Dee_Decref(code);
		if unlikely(temp < 0)
			goto err;
		result += temp;
	}
	return result;
err:
	return temp;
}


PRIVATE struct type_seq traceback_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceback_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem  = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem  = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem  = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
traceback_current(DeeObject *__restrict UNUSED(self)) {
	DREF DeeThreadObject *thread;
	DREF DeeObject *result;
	thread = DeeThread_Self();
	if unlikely(thread->t_except == NULL)
		goto err_no_except;
	result = (DREF DeeObject *)except_frame_gettb(thread->t_except);
	if unlikely(result == NULL)
		result = (DREF DeeObject *)&DeeTraceback_Empty;
	Dee_Incref(result);
	return result;
err_no_except:
	err_no_active_exception(); /* TODO: Throw UnboundAttribute */
	return NULL;
}

PRIVATE struct type_getset tpconst traceback_class_getsets[] = {
	TYPE_GETTER("current", &traceback_current,
	            "->?.\n"
	            "#tRuntimeError{No exception was being handled}"
	            "Returns the traceback associated with the current exception"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst traceback_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DeeTracebackIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_gc tpconst traceback_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&traceback_clear
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
traceback_sizeof(DeeTracebackObject *self) {
	size_t result;
	uint16_t i;
	result = _Dee_MallococBufsize(offsetof(DeeTracebackObject, tb_frames),
	                              self->tb_numframes,
	                              sizeof(struct code_frame));
	for (i = 0; i < self->tb_numframes; ++i) {
		if (self->tb_frames[i].cf_frame)
			result += self->tb_frames[i].cf_func->fo_code->co_framesize;
		if (self->tb_frames[i].cf_argv)
			result += self->tb_frames[i].cf_argc * sizeof(DREF DeeObject *);
		if (self->tb_frames[i].cf_stack)
			result += self->tb_frames[i].cf_stacksz * sizeof(DREF DeeObject *);
	}
	return DeeInt_NewSize(result);
}

PRIVATE struct type_getset tpconst traceback_getsets[] = {
	TYPE_GETTER_F("__sizeof__", &traceback_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PUBLIC DeeTypeObject DeeTraceback_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Traceback),
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&traceback_new,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&traceback_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str   = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr  = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool  = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_foreach),
		/* .tp_print = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&traceback_print,
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&traceback_visit,
	/* .tp_gc            = */ &traceback_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &traceback_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ traceback_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ traceback_class_getsets,
	/* .tp_class_members = */ traceback_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__5B2E0F4105586532),
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TRACEBACK_C */
