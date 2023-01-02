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
#ifndef GUARD_DEEMON_OBJECTS_TRACEBACK_C
#define GUARD_DEEMON_OBJECTS_TRACEBACK_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/code.h>
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

#include <hybrid/atomic.h>

#include <stdarg.h>
#include <stddef.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

#ifndef CONFIG_NO_THREADS
INTDEF DeeThreadObject DeeThread_Main;
#else /* CONFIG_NO_THREADS */
DATDEF DeeThreadObject DeeThread_Main;
#endif /* !CONFIG_NO_THREADS */

INTERN struct empty_traceback_object empty_traceback = {
	OBJECT_HEAD_INIT(&DeeTraceback_Type),
	/* .tb_thread    = */ &DeeThread_Main,
#ifndef CONFIG_NO_THREADS
	/* .tb_lock      = */ ATOMIC_LOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
	/* .tb_numframes = */ 0,
	/* .tb_padding   = */ { 0, 0, 0 }
};



/* Try to create a new traceback, but don't throw
 * an error and return `NULL' if doing so failed.
 * NOTE: The given `thread' must be the caller's. */
INTERN WUNUSED NONNULL((1)) DREF DeeTracebackObject *DCALL
DeeTraceback_New(struct thread_object *__restrict thread) {
	DREF DeeTracebackObject *result;
	ASSERT(thread == DeeThread_Self());
	result = (DREF DeeTracebackObject *)DeeGCObject_TryMalloc(offsetof(DeeTracebackObject, tb_frames) +
	                                                          thread->t_execsz * sizeof(struct code_frame));
	if likely(result) {
		struct code_frame *dst, *src;
		DeeObject_Init(result, &DeeTraceback_Type);
		result->tb_numframes = thread->t_execsz;
		result->tb_thread    = thread;
		Dee_Incref(thread);
		atomic_lock_init(&result->tb_lock);
		dst = result->tb_frames + thread->t_execsz;
		src = thread->t_exec;
		while (dst > result->tb_frames) {
			DeeCodeObject *code;
			DeeObject *dont_track_this = NULL;
			--dst;
			ASSERT(src != NULL);
			ASSERT(src != CODE_FRAME_NOT_EXECUTING);
			/* Do a shallow memcopy of the execution frame. */
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
				dst->cf_argv = (DREF DeeObject **)Dee_TryMalloc(dst->cf_argc *
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
			dst->cf_frame = (DREF DeeObject **)Dee_TryMalloc(code->co_localc *
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
		DeeGC_Track((DeeObject *)result);
	}
	return result;
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
	atomic_lock_acquire(&self->tb_lock);
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
	dst->cf_stack = (DREF DeeObject **)Dee_TryMalloc(stacksz * sizeof(DREF DeeObject *));
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
	atomic_lock_release(&self->tb_lock);
}


typedef struct {
	OBJECT_HEAD
	DREF DeeTracebackObject *ti_trace; /* [1..1][const] The traceback that is being iterated. */
	struct code_frame       *ti_next;  /* [1..1][in(ti_trace->tb_frames)][atomic]
	                                    * The next frame (yielded in reverse order) */
} TraceIterator;
INTDEF DeeTypeObject DeeTracebackIterator_Type;

#ifdef CONFIG_NO_THREADS
#define READ_NEXT(x)     ((x)->ti_next)
#define WRITE_NEXT(x, y) ((x)->ti_next = (y))
#else /* CONFIG_NO_THREADS */
#define READ_NEXT(x)     ATOMIC_READ((x)->ti_next)
#define WRITE_NEXT(x, y) ATOMIC_WRITE((x)->ti_next, y)
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) int DCALL
traceiter_ctor(TraceIterator *__restrict self) {
	self->ti_trace = (DREF DeeTracebackObject *)&empty_traceback;
	self->ti_next  = self->ti_trace->tb_frames;
	Dee_Incref(&empty_traceback);
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
#ifdef CONFIG_NO_THREADS
	result_frame = self->ti_next;
	if unlikely(result_frame < self->ti_trace->tb_frames)
		return ITER_DONE;
	self->ti_next = result_frame - 1;
#else /* CONFIG_NO_THREADS */
	do {
		result_frame = ATOMIC_READ(self->ti_next);
		if unlikely(result_frame < self->ti_trace->tb_frames)
			return ITER_DONE;
	} while (!ATOMIC_CMPXCH(self->ti_next, result_frame, result_frame - 1));
#endif /* !CONFIG_NO_THREADS */
	/* Create a new frame wrapper for this entry. */
	return DeeFrame_NewReferenceWithLock((DeeObject *)self->ti_trace,
	                                     result_frame,
	                                     DEEFRAME_FREADONLY,
	                                     &self->ti_trace->tb_lock);
}

PRIVATE struct type_member tpconst traceiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(TraceIterator, ti_trace), "->?DTraceback"),
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
#ifdef CONFIG_NO_THREADS
	result_frame = self->ti_next;
	if unlikely(result_frame < self->ti_trace->tb_frames)
		return 1;
	self->ti_next = result_frame - 1;
#else /* CONFIG_NO_THREADS */
	do {
		result_frame = ATOMIC_READ(self->ti_next);
		if unlikely(result_frame < self->ti_trace->tb_frames)
			return 1;
	} while (!ATOMIC_CMPXCH(self->ti_next, result_frame, result_frame - 1));
#endif /* !CONFIG_NO_THREADS */
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

#define DEFINE_TRACEITER_COMPARE(name, op)                                \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                 \
	name(TraceIterator *self, TraceIterator *other) {                     \
		if (DeeObject_AssertTypeExact(other, &DeeTracebackIterator_Type)) \
			goto err;                                                     \
		return_bool(READ_NEXT(other) op READ_NEXT(self));                 \
	err:                                                                  \
		return NULL;                                                      \
	}
DEFINE_TRACEITER_COMPARE(traceiter_eq, ==)
DEFINE_TRACEITER_COMPARE(traceiter_ne, !=)
DEFINE_TRACEITER_COMPARE(traceiter_lo, <)
DEFINE_TRACEITER_COMPARE(traceiter_le, <=)
DEFINE_TRACEITER_COMPARE(traceiter_gr, >)
DEFINE_TRACEITER_COMPARE(traceiter_ge, >=)
#undef DEFINE_TRACEITER_COMPARE

PRIVATE struct type_cmp traceiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&traceiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&traceiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&traceiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&traceiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&traceiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&traceiter_ge,
	/* .tp_nii  = */ &traceiter_nii
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&traceiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&traceiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &traceiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ traceiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED DREF DeeTracebackObject *DCALL traceback_new(void) {
	return DeeTraceback_New(DeeThread_Self());
}

PRIVATE NONNULL((1)) void DCALL
traceback_fini(DeeTracebackObject *__restrict self) {
	struct code_frame *frame;
	size_t i, frame_index;
	Dee_Decref(self->tb_thread);
	frame_index = self->tb_numframes;
	while (frame_index--) {
		frame = &self->tb_frames[frame_index];
		ASSERT_OBJECT_TYPE(frame->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE(frame->cf_func->fo_code, &DeeCode_Type);
		/* Decref local variables. */
		i = frame->cf_func->fo_code->co_localc;
		while (i--)
			Dee_XDecref(frame->cf_frame[i]);
		Dee_Free(frame->cf_frame);
		/* Decref stack objects. */
		if (frame->cf_stacksz) {
			i = frame->cf_stacksz;
			while (i--)
				Dee_Decref(frame->cf_stack[i]);
			Dee_Free(frame->cf_stack);
		}
		/* Decref argument objects. */
		i = frame->cf_argc;
		while (i--)
			Dee_Decref(frame->cf_argv[i]);
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
	atomic_lock_acquire(&self->tb_lock);
	Dee_Visit(self->tb_thread);
	end = (iter = self->tb_frames) + self->tb_numframes;
	for (; iter < end; ++iter) {
		DREF DeeObject **oiter, **oend;
		ASSERT_OBJECT_TYPE(iter->cf_func, &DeeFunction_Type);
		ASSERT_OBJECT_TYPE(iter->cf_func->fo_code, &DeeCode_Type);
		/* Visit local variables. */
		oend = (oiter = iter->cf_frame) + iter->cf_func->fo_code->co_localc;
		if (oiter) {
			for (; oiter != oend; ++oiter)
				Dee_XVisit(*oiter);
		}
		/* Visit stack objects. */
		oend = (oiter = iter->cf_stack) + iter->cf_stacksz;
		for (; oiter < oend; ++oiter)
			Dee_Visit(*oiter);
		/* Visit argument objects. */
		oend = (oiter = (DREF DeeObject **)iter->cf_argv) + iter->cf_argc;
		for (; oiter < oend; ++oiter)
			Dee_Visit(*oiter);
		Dee_Visit(iter->cf_func);
		Dee_XVisit(iter->cf_this);
		Dee_XVisit(iter->cf_vargs);
		if (ITER_ISOK(iter->cf_result))
			Dee_Visit(iter->cf_result);
	}
	atomic_lock_release(&self->tb_lock);
}

PRIVATE NONNULL((1)) void DCALL
traceback_clear(DeeTracebackObject *__restrict self) {
	DREF DeeObject *decref_later_buffer[64], **decref_later;
	struct code_frame *iter, *end;
again:
	decref_later = decref_later_buffer;
	atomic_lock_acquire(&self->tb_lock);
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
			*oiter = Dee_None;
			Dee_Incref(Dee_None);
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
			*oiter = Dee_None;
			Dee_Incref(Dee_None);
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
	atomic_lock_release(&self->tb_lock);
	while (decref_later != decref_later_buffer) {
		--decref_later;
		Dee_Decref(*decref_later);
	}
	return;
clear_buffer:
	atomic_lock_release(&self->tb_lock);
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

INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL
print_ddi(struct ascii_printer *__restrict printer,
          DeeCodeObject *__restrict code, code_addr_t ip);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
traceback_repr(DeeTracebackObject *__restrict self) {
	struct ascii_printer printer = ASCII_PRINTER_INIT;
	uint16_t i                   = self->tb_numframes;
	while (i--) {
		DREF DeeCodeObject *code;
		code_addr_t ip;
		dssize_t error;
		atomic_lock_acquire(&self->tb_lock);
		code = self->tb_frames[i].cf_func->fo_code;
		Dee_Incref(code);
		ip = (code_addr_t)(self->tb_frames[i].cf_ip - code->co_code);
		atomic_lock_release(&self->tb_lock);
		error = print_ddi(&printer, code, ip);
		Dee_Decref(code);
		if unlikely(error < 0)
			goto err;
	}
	return ascii_printer_pack(&printer);
err:
	ascii_printer_fini(&printer);
	return NULL;
}


PRIVATE struct type_seq traceback_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceback_iter,
	/* .tp_size      = */ NULL,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
traceback_current(DeeObject *__restrict UNUSED(self)) {
	DREF DeeThreadObject *thread;
	DREF DeeObject *result;
	thread = DeeThread_Self();
	if unlikely(!thread->t_except) {
		err_no_active_exception();
		return NULL;
	}
	result = (DREF DeeObject *)except_frame_gettb(thread->t_except);
	if unlikely(!result)
		result = (DREF DeeObject *)&empty_traceback;
	Dee_Incref(result);
	return result;
}

PRIVATE struct type_getset tpconst traceback_class_getsets[] = {
	{ "current", &traceback_current, NULL, NULL,
	  DOC("->?.\n"
	      "@throw RuntimeError No exception was being handled\n"
	      "Returns the traceback associated with the current exception") },
	{ NULL }
};

PRIVATE struct type_member tpconst traceback_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeTracebackIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_gc tpconst traceback_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&traceback_clear
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
traceback_sizeof(DeeTracebackObject *self) {
	size_t result;
	uint16_t i;
	result = offsetof(DeeTracebackObject, tb_frames) +
	         (self->tb_numframes * sizeof(struct code_frame));
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
	{ STR___sizeof__,
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceback_sizeof, NULL, NULL,
	  DOC("->?Dint") },
	{ NULL }
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&traceback_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&traceback_visit,
	/* .tp_gc            = */ &traceback_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &traceback_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ traceback_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ traceback_class_getsets,
	/* .tp_class_members = */ traceback_class_members
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TRACEBACK_C */
