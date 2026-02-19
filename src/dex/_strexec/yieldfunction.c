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
#ifndef GUARD_DEX_STREXEC_YIELDFUNCTION_C
#define GUARD_DEX_STREXEC_YIELDFUNCTION_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_Free, Dee_Mallocc, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, _Dee_MallococBufsize */
#include <deemon/error.h>           /* DeeError_*, ERROR_HANDLED_INTERRUPT, ERROR_PRINT_DOHANDLE */
#include <deemon/format.h>          /* PRFu16 */
#include <deemon/gc.h>              /* DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGC_TRACK */
#include <deemon/kwds.h>            /* DeeKwds_Check, DeeKwds_SIZE */
#include <deemon/none.h>            /* DeeNone_Check */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_Decrefv, Dee_Incref, Dee_XDecref, Dee_XIncref, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/seq.h>             /* DeeIterator_Type, DeeRefVector_NewReadonly, DeeSeq_Type */
#include <deemon/serial.h>          /* DeeSerial*, Dee_SERADDR_INVALID, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/system-features.h> /* memcpy* */
#include <deemon/thread.h>          /* DeeThreadObject, DeeThread_CheckInterrupt, DeeThread_Self, Dee_except_frame, Dee_except_frame_free */
#include <deemon/type.h>            /* DeeObject_Init, DeeObject_IsInterrupt, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_Visit, Dee_Visitv, Dee_XVisit, Dee_visit_t, METHOD_FNOREFESCAPE, STRUCT_OBJECT, STRUCT_OBJECT_AB, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/hash.h>       /* Dee_HashUtf8 */
#include <deemon/util/rlock.h>      /* Dee_rshared_lock_init */

#include <hybrid/unaligned.h> /* UNALIGNED_GET* */
#include <hybrid/wordbits.h>  /* ENCODE_INT16, ENCODE_INT32 */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uint8_t, uint16_t, uint32_t */

DECL_BEGIN

typedef JITFunctionObject JITFunction;
typedef JITYieldFunctionObject JITYieldFunction;
typedef JITYieldFunctionIteratorObject JITYieldFunctionIterator;

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL jf_getname(JITFunction *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL jf_getdoc(JITFunction *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL jf_getkwds(JITFunction *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL jf_gettext(JITFunction *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL jf_getrefs(JITFunction *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL jf_getrefsbyname(JITFunction *__restrict self);


INTERN NONNULL((1)) void DCALL
jit_state_fini(struct jit_state *__restrict self) {
	switch (self->js_kind) {

	case JIT_STATE_KIND_FOREACH:
		Dee_Decref(self->js_foreach.f_iter);
		JITLValue_Fini(&self->js_foreach.f_elem);
		break;

	case JIT_STATE_KIND_FOREACH2:
		Dee_Decref(self->js_foreach2.f_iter);
		JITLValueList_Fini(&self->js_foreach2.f_elem);
		break;

	case JIT_STATE_KIND_WITH:
		/* Drop our held reference. */
		Dee_Decref(self->js_with.w_obj);
		break;

	default:
		break;
	}
}


PRIVATE NONNULL((1)) void DCALL
jy_fini(JITYieldFunction *__restrict self) {
	Dee_Decrefv(self->jy_argv, self->jy_argc);
	Dee_XDecref(self->jy_kw);
	Dee_Decref(self->jy_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
jy_visit(JITYieldFunction *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visitv(self->jy_argv, self->jy_argc);
	Dee_XVisit(self->jy_kw);
	Dee_Visit(self->jy_func);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL
jy_serialize(JITYieldFunction *__restrict self, DeeSerial *__restrict writer) {
	JITYieldFunction *out;
	size_t sizeof_self = _Dee_MallococBufsize(offsetof(JITYieldFunction, jy_argv),
	                                          self->jy_argc, sizeof(DREF DeeObject *));
	Dee_seraddr_t out_addr = DeeSerial_ObjectMalloc(writer, sizeof_self, self);
#define ADDROF(field) (out_addr + offsetof(JITYieldFunction, field))
	if unlikely(!Dee_SERADDR_ISOK(out_addr))
		goto err;
	out = DeeSerial_Addr2Mem(writer, out_addr, JITYieldFunction);
	out->jy_argc = self->jy_argc;
	if (DeeSerial_PutObject(writer, ADDROF(jy_func), self->jy_func))
		goto err;
	if (DeeSerial_XPutObject(writer, ADDROF(jy_kw), self->jy_kw))
		goto err;
	if (DeeSerial_PutObjectv(writer, ADDROF(jy_argv), self->jy_argv, self->jy_argc))
		goto err;
	return out_addr;
err:
	return Dee_SERADDR_INVALID;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) DREF JITYieldFunctionIterator *DCALL
jy_iter(JITYieldFunction *__restrict self) {
	DREF JITYieldFunctionIterator *result;
	JITFunction *jf = self->jy_func;
	result = DeeGCObject_MALLOC(JITYieldFunctionIterator);
	if unlikely(!result)
		goto err;
	ASSERT(jf->jf_args.ot_prev.otp_ind >= 2);
	ASSERT(jf->jf_args.ot_prev.otp_tab == &jf->jf_refs);
	memcpy(&result->ji_loc, &jf->jf_args, sizeof(JITObjectTable));
	ASSERT(result->ji_loc.ot_prev.otp_tab != NULL);
	result->ji_loc.ot_list = (struct jit_object_entry *)Dee_Mallocc(result->ji_loc.ot_mask + 1,
	                                                                sizeof(struct jit_object_entry));
	if unlikely(!result->ji_loc.ot_list)
		goto err_r;
	memcpyc(result->ji_loc.ot_list, jf->jf_args.ot_list,
	        result->ji_loc.ot_mask + 1,
	        sizeof(struct jit_object_entry));

	/* Define the self-argument.
	 * NOTE: Do this before loading arguments, in case one of the arguments
	 *       uses the same name as the function, in which case that argument
	 *       must be loaded, rather than the function loading itself! */
	if (jf->jf_selfarg != (size_t)-1) {
		ASSERT(result->ji_loc.ot_list[jf->jf_selfarg].oe_value == NULL);
		result->ji_loc.ot_list[jf->jf_selfarg].oe_value = Dee_AsObject(self);
	}

	if (self->jy_kw) {
		size_t i;
		/* TODO: Load arguments! */
		(void)self->jy_argc;
		(void)self->jy_argv;
		(void)self->jy_kw;
		/* Assign references to all objects from base-locals */
		for (i = 0; i <= result->ji_loc.ot_mask; ++i) {
			if (!ITER_ISOK(result->ji_loc.ot_list[i].oe_namestr))
				continue;
			Dee_XIncref(result->ji_loc.ot_list[i].oe_value);
		}
	} else {
		size_t i;
		/* Load arguments! */
		if (self->jy_argc < jf->jf_argc_min)
			goto err_r_loc_bad_argc;
		if (self->jy_argc > jf->jf_argc_max && jf->jf_varargs == (size_t)-1)
			goto err_r_loc_bad_argc;
		/* Load positional arguments. */
		for (i = 0; i < self->jy_argc; ++i)
			result->ji_loc.ot_list[jf->jf_argv[i]].oe_value = self->jy_argv[i];
		/* Assign references to all objects from base-locals */
		for (i = 0; i <= result->ji_loc.ot_mask; ++i) {
			if (!ITER_ISOK(result->ji_loc.ot_list[i].oe_namestr))
				continue;
			Dee_XIncref(result->ji_loc.ot_list[i].oe_value);
		}
	}

	result->ji_func = self;
	Dee_Incref(self);
	Dee_rshared_lock_init(&result->ji_lock);
	result->ji_lex.jl_text    = jf->jf_source;
	result->ji_lex.jl_context = &result->ji_ctx;
	JITLValue_Init(&result->ji_lex.jl_lvalue);
	result->ji_ctx.jc_impbase        = jf->jf_impbase;
	result->ji_ctx.jc_globals        = jf->jf_globals;
	result->ji_ctx.jc_retval         = JITCONTEXT_RETVAL_UNSET;
	result->ji_ctx.jc_locals.otp_ind = 1;
	result->ji_ctx.jc_locals.otp_tab = &result->ji_loc;
	/* Initialize the base-compiler-context-state as a block-scope. */
	result->ji_state         = &result->ji_bstat;
	result->ji_bstat.js_prev = NULL;
	result->ji_bstat.js_kind = JIT_STATE_KIND_SCOPE;
	result->ji_bstat.js_flag = JIT_STATE_FLAG_BLOCK;
	JITLexer_Start(&result->ji_lex,
	               (unsigned char const *)jf->jf_source_start,
	               (unsigned char const *)jf->jf_source_end);
	DeeObject_Init(result, &JITYieldFunctionIterator_Type);
	return DeeGC_TRACK(JITYieldFunctionIterator, result);
err_r_loc_bad_argc:
	if (jf->jf_selfarg == (size_t)-1) {
		err_invalid_argc_len(NULL,
		                     0,
		                     self->jy_argc,
		                     jf->jf_argc_min,
		                     jf->jf_argc_max);
	} else {
		struct jit_object_entry *ent;
		ent = &jf->jf_args.ot_list[jf->jf_selfarg];
		err_invalid_argc_len((char const *)ent->oe_namestr,
		                     ent->oe_namelen,
		                     self->jy_argc,
		                     jf->jf_argc_min,
		                     jf->jf_argc_max);
	}
	Dee_Free(result->ji_loc.ot_list);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}


PRIVATE struct type_seq jy_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jy_iter,
	/* .tp_sizeob   = */ NULL,
	/* .tp_contains = */ NULL,
	/* .tp_getitem  = */ NULL,
	/* .tp_delitem  = */ NULL,
	/* .tp_setitem  = */ NULL,
	/* .tp_getrange = */ NULL,
	/* .tp_delrange = */ NULL,
	/* .tp_setrange = */ NULL
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jy_getargs(JITYieldFunction *__restrict self) {
	size_t arg_offset = 0;
	/* Adjust for shared arguments. */
	if (self->jy_kw && DeeKwds_Check(self->jy_kw))
		arg_offset = DeeKwds_SIZE(self->jy_kw);
	if (arg_offset > self->jy_argc)
		arg_offset = self->jy_argc;
	return DeeRefVector_NewReadonly(Dee_AsObject(self),
	                                self->jy_argc - arg_offset,
	                                self->jy_argv + arg_offset);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jy_getname(JITYieldFunction *__restrict self) {
	return jf_getname(self->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jy_getdoc(JITYieldFunction *__restrict self) {
	return jf_getdoc(self->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jy_getkwds(JITYieldFunction *__restrict self) {
	return jf_getkwds(self->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jy_gettext(JITYieldFunction *__restrict self) {
	return jf_gettext(self->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jy_getrefs(JITYieldFunction *__restrict self) {
	return jf_getrefs(self->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
jy_getrefsbyname(JITYieldFunction *__restrict self) {
	return jf_getrefsbyname(self->jy_func);
}

PRIVATE struct type_getset tpconst jy_getsets[] = {
	TYPE_GETTER_F("__name__", &jy_getname, METHOD_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "Alias for ?A__name__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__doc__", &jy_getdoc, METHOD_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "Alias for ?A__doc__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__kwds__", &jy_getkwds, METHOD_FNOREFESCAPE,
	              "->?S?Dstring\n"
	              "Alias for ?A__kwds__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__text__", &jy_gettext, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "Alias for ?A__text__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__args__", &jy_getargs, METHOD_FNOREFESCAPE,
	              "->?S?O\n"
	              "Returns a sequence representing the positional arguments passed to the function"),
	TYPE_GETTER_F("__refs__", &jy_getrefs, METHOD_FNOREFESCAPE,
	              "->?S?O\n"
	              "Alias for ?A__refs__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__refsbyname__", &jy_getrefsbyname, METHOD_FNOREFESCAPE,
	              "->?M?Dstring?O\n"
	              "Alias for ?A__refsbyname__?GFunction though ?#__func__"),
	/* TODO: __argsbyname__ */
	/* TODO: __defaults__ */
	/* TODO: __type__ */
	/* TODO: __operator__ */
	/* TODO: __operatorname__ */
	/* TODO: __property__ */
	/* TODO: __statics__ */
	/* TODO: __staticsbyname__ */
	/* TODO: __symbols__ */
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst jy_members[] = {
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT_AB, offsetof(JITYieldFunction, jy_func), "->?GFunction"),
	TYPE_MEMBER_FIELD_DOC("__kw__", STRUCT_OBJECT, offsetof(JITYieldFunction, jy_kw), "->?M?Dstring?X2?Dint?O"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst jy_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &JITYieldFunctionIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject JITYieldFunction_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_JitYieldFunction",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &jy_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&jy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&jy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &jy_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ jy_getsets,
	/* .tp_members       = */ jy_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ jy_class_members,
	/* .tp_method_hints  = */ NULL,
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
JITLexer_JumpToDoWhileCondition(JITLexer *__restrict self,
                                struct jit_state *__restrict st) {
	if (st->js_dowhile.f_cond) {
		JITLexer_YieldAt(self, st->js_dowhile.f_cond);
	} else {
		JITLexer_YieldAt(self, st->js_dowhile.f_loop);
		if (JITLexer_SkipStatement(self))
			goto err;
		if (JITLexer_ISKWD(self, "while")) {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_while_after_do(self);
			goto err;
		}
		if (self->jl_tok == '(') {
			JITLexer_Yield(self);
		} else {
			syn_dowhile_expected_lparen_after_while(self);
			goto err;
		}
		st->js_dowhile.f_cond = self->jl_tokstart;
	}
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
JITLexer_EvalFinallyStatements(JITLexer *__restrict self) {
	for (;;) {
		/* XXX: Full tagging support? */
		if (self->jl_tok == '@') {
			JITLexer_Yield(self);
			if (self->jl_tok == '[') {
				JITLexer_Yield(self);
				if (JITLexer_SkipPair(self, '[', ']'))
					goto err;
			}
		}
		if (JITLexer_ISKWD(self, "finally")) {
			DREF DeeObject *value;
			JITLexer_Yield(self);
			/* Evaluate the finally-statement (NOTE: JIT doesn't allow yield-in-finally) */
			value = JITLexer_EvalStatement(self);
			if unlikely(!value)
				goto err;
			if (value == JIT_LVALUE) {
				JITLValue_Fini(&self->jl_lvalue);
				JITLValue_Init(&self->jl_lvalue);
			} else {
				Dee_Decref(value);
			}
		} else if (JITLexer_ISKWD(self, "catch")) {
			/* Simply skip catch statements. */
			JITLexer_Yield(self);
			if unlikely(self->jl_tok != '(') {
				syn_try_expected_lparen_after_catch(self);
				goto err;
			}
			JITLexer_Yield(self);
			if (JITLexer_SkipPair(self, '(', ')'))
				goto err;
			if (JITLexer_SkipStatement(self))
				goto err;
		} else {
			break;
		}
	}
	return 0;
err:
	return -1;
}

/* @return:  1: The state wasn't removed, but the lexer position was updated
 *              such that execution should resume normally (this is used to
 *              implement loop statements)
 * @return:  0: The state was successfully removed
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
JITYieldFunctionIterator_PopState(JITYieldFunctionIterator *__restrict self) {
	struct jit_state *st;
	unsigned char const *old_pos;
	st = self->ji_state;
	ASSERT(st != &self->ji_bstat);
	switch (st->js_kind) {

	case JIT_STATE_KIND_SCOPE:
		goto do_pop_state;

	case JIT_STATE_KIND_TRY:
		/* Search for, and execute finally-blocks. */
		if unlikely(JITLexer_EvalFinallyStatements(&self->ji_lex))
			goto err;
		goto do_pop_state;

	case JIT_STATE_KIND_DOWHILE:
		if (st->js_dowhile.f_cond) {
			JITLexer_YieldAt(&self->ji_lex, st->js_dowhile.f_cond);
		} else {
			if (JITLexer_ISKWD(&self->ji_lex, "while")) {
				JITLexer_Yield(&self->ji_lex);
			} else {
				syn_dowhile_expected_while_after_do(&self->ji_lex);
				goto err;
			}
			if (self->ji_lex.jl_tok == '(') {
				JITLexer_Yield(&self->ji_lex);
			} else {
				syn_dowhile_expected_lparen_after_while(&self->ji_lex);
				goto err;
			}
			st->js_dowhile.f_cond = self->ji_lex.jl_tokstart;
		}
		/* Evaluate the expression value. */
		{
			DREF DeeObject *value;
			int temp;
			value = JITLexer_EvalRValue(&self->ji_lex);
			if unlikely(!value)
				goto err;
			temp = DeeObject_BoolInherited(value);
			if unlikely(temp < 0)
				goto err;
			if (!temp) {
				if (self->ji_lex.jl_tok == ')') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_dowhile_expected_rparen_after_while(&self->ji_lex);
					goto err;
				}
				if (self->ji_lex.jl_tok == ';') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_dowhile_expected_semi_after_while(&self->ji_lex);
					goto err;
				}
				goto do_pop_state;
			}
		}
		/* Check for interrupts before jumping backwards. */
		if (DeeThread_CheckInterrupt())
			goto err;
		/* Jump back to the start of the do-while block. */
		JITLexer_YieldAt(&self->ji_lex, st->js_dowhile.f_loop);
		/* Execute the loop block once again */
		return 1;

	case JIT_STATE_KIND_WHILE: {
		DREF DeeObject *value;
		int temp;
		old_pos = self->ji_lex.jl_tokstart;
		JITLexer_YieldAt(&self->ji_lex, st->js_while.f_cond);
		/* Invoke the cond-expression. */
		value = JITLexer_EvalRValueDecl(&self->ji_lex);
		if unlikely(!value)
			goto err;
		/* Check if loop iteration should continue. */
		temp = DeeObject_BoolInherited(value);
		if unlikely(temp < 0)
			goto err;
		if (!temp) {
			/* Restore the old position (which is presumably just after the loop block) */
			JITLexer_YieldAt(&self->ji_lex, old_pos);
			/* Don't jump back, but break out of the loop. */
			goto do_pop_state_scope;
		}
		/* Check for interrupts before jumping backwards */
		if (DeeThread_CheckInterrupt())
			goto err;
		/* Resume execution of the loop. */
		JITLexer_YieldAt(&self->ji_lex, st->js_while.f_loop);
		return 1;
	}

	case JIT_STATE_KIND_FOR:
		old_pos = self->ji_lex.jl_tokstart;
		if (st->js_for.f_next) {
			DREF DeeObject *temp;
			JITLexer_YieldAt(&self->ji_lex, st->js_for.f_next);
			/* Invoke the next-expression. */
			temp = JITLexer_EvalExpression(&self->ji_lex,
			                               JITLEXER_EVAL_FNORMAL);
			if unlikely(!temp)
				goto err;
			if (temp == JIT_LVALUE) {
				JITLValue_Fini(&self->ji_lex.jl_lvalue);
				JITLValue_Init(&self->ji_lex.jl_lvalue);
			} else {
				Dee_Decref(temp);
			}
		}
		if (st->js_for.f_cond) {
			DREF DeeObject *value;
			int temp;
			JITLexer_YieldAt(&self->ji_lex, st->js_for.f_cond);
			/* Invoke the cond-expression. */
			value = JITLexer_EvalRValue(&self->ji_lex);
			if unlikely(!value)
				goto err;
			/* Check if loop iteration should continue. */
			temp = DeeObject_BoolInherited(value);
			if unlikely(temp < 0)
				goto err;
			if (!temp) {
				/* Restore the old position (which is presumably just after the loop block) */
				JITLexer_YieldAt(&self->ji_lex, old_pos);
				/* Don't jump back, but break out of the loop. */
				goto do_pop_state_scope;
			}
		}
		/* Check for interrupts before jumping backwards */
		if (DeeThread_CheckInterrupt())
			goto err;
		/* Resume execution of the loop. */
		JITLexer_YieldAt(&self->ji_lex, st->js_for.f_loop);
		return 1;

	case JIT_STATE_KIND_FOREACH: {
		DREF DeeObject *elem;
		elem = DeeObject_IterNext(st->js_foreach.f_iter);
		if (ITER_ISOK(elem)) {
			/* Loop back to the start of the foreach loop. */
			int temp;
			temp = JITLValue_SetValue(&st->js_foreach.f_elem,
			                          &self->ji_ctx,
			                          elem);
			Dee_Decref(elem);
			if unlikely(temp)
				goto err;
			/* Check for interrupts before looping. */
			if (DeeThread_CheckInterrupt())
				goto err;
			JITLexer_YieldAt(&self->ji_lex,
			                 st->js_foreach.f_loop);
			return 1;
		}
		if (!elem)
			goto err;
		/* ITER_DONE... */
		goto do_pop_state_scope;
	}	break;

	case JIT_STATE_KIND_FOREACH2: {
		DREF DeeObject *elem;
		elem = DeeObject_IterNext(st->js_foreach2.f_iter);
		if (ITER_ISOK(elem)) {
			/* Loop back to the start of the foreach loop. */
			int temp;
			temp = JITLValueList_UnpackAssign(&st->js_foreach2.f_elem,
			                                  &self->ji_ctx,
			                                  elem);
			Dee_Decref(elem);
			if unlikely(temp)
				goto err;
			/* Check for interrupts before looping. */
			if (DeeThread_CheckInterrupt())
				goto err;
			JITLexer_YieldAt(&self->ji_lex,
			                 st->js_foreach2.f_loop);
			return 1;
		}
		if (!elem)
			goto err;
		/* ITER_DONE... */
		goto do_pop_state_scope;
	}	break;

	case JIT_STATE_KIND_SKIPELSE:
		/* Check for `else' and `elif' */
		if (self->ji_lex.jl_tok == JIT_KEYWORD &&
		    self->ji_lex.jl_tokend == self->ji_lex.jl_tokstart + 4) {
			uint32_t name;
			name = UNALIGNED_GET32(self->ji_lex.jl_tokstart);
			if (name == ENCODE_INT32('e', 'l', 's', 'e')) {
				JITLexer_Yield(&self->ji_lex);
do_skip_else:
				/* Skip the accompanying block. */
				if unlikely(JITLexer_SkipStatement(&self->ji_lex))
					goto err;
			} else if (name == ENCODE_INT32('e', 'l', 'i', 'f')) {
				self->ji_lex.jl_tokstart += 2; /* Transform into an `if' */
				goto do_skip_else;
			}
		}
		goto do_pop_state_scope;

	case JIT_STATE_KIND_WITH:
		if unlikely(DeeObject_Leave(st->js_with.w_obj)) {
			/* Still pop the state upon error! */
			self->ji_state = st->js_prev;
			jit_state_destroy(st);
			goto err;
		}
		goto do_pop_state_scope;

	/*case JIT_STATE_KIND_SCOPE2:*/
	default: break;
	}
do_pop_state_scope:
	JITContext_PopScope(&self->ji_ctx);
do_pop_state:
	/* Default: Remove the state. */
	self->ji_state = st->js_prev;
	jit_state_destroy(st);
	return 0;
err:
	return -1;
}


/* Unwind the active state-stack until `new_curr_state', such that upon
 * successfully return, `new_curr_state' will be the currently active state. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
JITYieldFunctionIterator_UnwindUntil(JITYieldFunctionIterator *__restrict self,
                                     struct jit_state *__restrict new_curr_state) {
	while (self->ji_state != new_curr_state) {
		struct jit_state *curr;
		struct jit_state *prev;
		curr = self->ji_state;
		ASSERT(curr != &self->ji_bstat);
		/* Adjust for the scopes that are lost by this jump. */
		if (JIT_STATE_KIND_HASSCOPE(curr->js_kind))
			JITContext_PopScope(&self->ji_ctx);
		if (!(curr->js_flag & JIT_STATE_FLAG_SINGLE))
			JITContext_PopScope(&self->ji_ctx);
		prev = curr->js_prev;
		if (curr->js_kind == JIT_STATE_KIND_WITH &&
		    unlikely(DeeObject_Leave(curr->js_with.w_obj))) {
			/* Still pop the state upon error. */
			jit_state_destroy(curr);
			self->ji_state = prev;
			goto err;
		}
		jit_state_destroy(curr);
		self->ji_state = prev;
	}
	return 0;
err:
	return -1;
}

/* Handle a break/continue loop control command by searching for
 * the nearest JIT state that behaves as a loop controller.
 * @return:  1: The lexer state was updated, the state was unwound,
 *              and the loop control command was handled successfully.
 * @return:  0: Failed to locate a valid receiver for the loop control command.
 * @return: -1: An error occurred. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
JITYieldFunctionIterator_HandleLoopctl(JITYieldFunctionIterator *__restrict self,
                                       bool ctl_is_break) {
	/* Search for the nearest state that can handle the loop control command,
	 * then proceed to unwind the state stack up to that state and load the
	 * lexer position to which the jump should be performed.
	 * Once this is done, return `1' (but remember that in the case of `break',
	 * the loop state entry itself must also be removed, as the target location
	 * exists after the loop itself, meaning that the loop context will have
	 * ended at that point) */
	struct jit_state *st = self->ji_state;
	for (; st != &self->ji_bstat; st = st->js_prev) {
		switch (st->js_kind) {

		case JIT_STATE_KIND_DOWHILE:
		case JIT_STATE_KIND_WHILE:
		case JIT_STATE_KIND_FOR:
		case JIT_STATE_KIND_FOREACH:
		case JIT_STATE_KIND_FOREACH2:
			/* Unwind up until the target state. */
			if unlikely(JITYieldFunctionIterator_UnwindUntil(self, st))
				goto err;
			/* Leave the scope of the loop body (the scope will
			 * be re-entered when the loop body is entered again) */
			if (!(st->js_flag & JIT_STATE_FLAG_SINGLE)) {
				JITContext_PopScope(&self->ji_ctx);
				st->js_flag |= JIT_STATE_FLAG_SINGLE;
			}
			switch (st->js_kind) {

			case JIT_STATE_KIND_DOWHILE:
				if unlikely(JITLexer_JumpToDoWhileCondition(&self->ji_lex, st))
					goto err;
				if (ctl_is_break) {
					/* Skip over the condition expression and break out of the loop */
					if (JITLexer_SkipExpression(&self->ji_lex, JITLEXER_EVAL_FNORMAL))
						goto err;
do_break_dowhile_loop:
					if (self->ji_lex.jl_tok == ')') {
						JITLexer_Yield(&self->ji_lex);
					} else {
						syn_dowhile_expected_rparen_after_while(&self->ji_lex);
						goto err;
					}
					if (self->ji_lex.jl_tok == ';') {
						JITLexer_Yield(&self->ji_lex);
					} else {
						syn_dowhile_expected_semi_after_while(&self->ji_lex);
						goto err;
					}
					self->ji_state = st->js_prev;
					jit_state_destroy(st);
					return 1;
				}
				/* Evaluate the condition expression. */
				{
					DREF DeeObject *value;
					int temp;
					value = JITLexer_EvalRValue(&self->ji_lex);
					if unlikely(!value)
						goto err;
					temp = DeeObject_BoolInherited(value);
					if unlikely(temp < 0)
						goto err;
					if (!temp)
						goto do_break_dowhile_loop;
				}
				/* Check for interrupts before jumping backwards. */
				if (DeeThread_CheckInterrupt())
					goto err;
				/* Jump back to the start of the do-while block. */
				JITLexer_YieldAt(&self->ji_lex, st->js_dowhile.f_loop);
				/* Execute the loop block once again */
				return 1;

			case JIT_STATE_KIND_WHILE:
				if (ctl_is_break) {
					JITLexer_YieldAt(&self->ji_lex, st->js_while.f_loop);
				} else {
					DREF DeeObject *value;
					int temp;
					/* Evaluate the cond-expression */
					JITLexer_YieldAt(&self->ji_lex, st->js_while.f_cond);
					value = JITLexer_EvalRValueDecl(&self->ji_lex);
					if unlikely(!value)
						goto err;
					temp = DeeObject_BoolInherited(value);
					if unlikely(temp < 0)
						goto err;
					if (!temp) {
						JITLexer_YieldAt(&self->ji_lex, st->js_while.f_loop);
						goto do_break_loop; /* Break out of the loop */
					}
					/* Check for interrupts before jumping back. */
					if (DeeThread_CheckInterrupt())
						goto err;
					/* Jump to the start of the loop block. */
					JITLexer_YieldAt(&self->ji_lex, st->js_while.f_loop);
				}
				break;

			case JIT_STATE_KIND_FOR:
				if (ctl_is_break) {
					JITLexer_YieldAt(&self->ji_lex, st->js_for.f_loop);
				} else {
					DREF DeeObject *value;
					if (st->js_for.f_next) {
						/* Evaluate the next-expression */
						JITLexer_YieldAt(&self->ji_lex, st->js_for.f_next);
						value = JITLexer_EvalExpression(&self->ji_lex, JITLEXER_EVAL_FNORMAL);
						if unlikely(!value)
							goto err;
						if (value == JIT_LVALUE) {
							JITLValue_Fini(&self->ji_lex.jl_lvalue);
							JITLValue_Init(&self->ji_lex.jl_lvalue);
						} else {
							Dee_Decref(value);
						}
					}
					if (st->js_for.f_cond) {
						int temp;
						/* Evaluate the cond-expression */
						JITLexer_YieldAt(&self->ji_lex, st->js_for.f_cond);
						value = JITLexer_EvalRValue(&self->ji_lex);
						if unlikely(!value)
							goto err;
						temp = DeeObject_BoolInherited(value);
						if unlikely(temp < 0)
							goto err;
						if (!temp) {
							JITLexer_YieldAt(&self->ji_lex, st->js_for.f_loop);
							goto do_break_loop; /* Break out of the loop */
						}
					}
					/* Check for interrupts before jumping back. */
					if (DeeThread_CheckInterrupt())
						goto err;
					/* Jump to the start of the loop block. */
					JITLexer_YieldAt(&self->ji_lex, st->js_for.f_loop);
				}
				break;

			case JIT_STATE_KIND_FOREACH:
				JITLexer_YieldAt(&self->ji_lex, st->js_foreach.f_loop);
				if (!ctl_is_break) {
					DREF DeeObject *elem;
					int temp;
					/* Load the next item */
					elem = DeeObject_IterNext(st->js_foreach.f_iter);
					if (!ITER_ISOK(elem)) {
						if unlikely(!elem)
							goto err;
						/* The iterator has been exhausted. */
						goto do_break_loop;
					}
					/* Store the new iterator element. */
					temp = JITLValue_SetValue(&st->js_foreach.f_elem,
					                          &self->ji_ctx,
					                          elem);
					Dee_Decref(elem);
					if unlikely(temp)
						goto err;
					/* Check for interrupts before jumping back. */
					if (DeeThread_CheckInterrupt())
						goto err;
				}
				break;

			case JIT_STATE_KIND_FOREACH2:
				JITLexer_YieldAt(&self->ji_lex, st->js_foreach2.f_loop);
				if (!ctl_is_break) {
					DREF DeeObject *elem;
					int temp;
					/* Load the next item */
					elem = DeeObject_IterNext(st->js_foreach2.f_iter);
					if (!ITER_ISOK(elem)) {
						if unlikely(!elem)
							goto err;
						/* The iterator has been exhausted. */
						goto do_break_loop;
					}
					/* Unpack + store the new iterator element. */
					temp = JITLValueList_UnpackAssign(&st->js_foreach2.f_elem,
					                                  &self->ji_ctx,
					                                  elem);
					Dee_Decref(elem);
					if unlikely(temp)
						goto err;
					/* Check for interrupts before jumping back. */
					if (DeeThread_CheckInterrupt())
						goto err;
				}
				break;

			default: __builtin_unreachable();
			}
			if (ctl_is_break) {
do_break_loop:
				/* Jump to the end of the loop block, and pop the loop itself. */
				if unlikely(JITLexer_SkipStatement(&self->ji_lex))
					goto err;
				ASSERT(JIT_STATE_KIND_HASSCOPE(st->js_kind));
				ASSERT(st->js_kind != JIT_STATE_KIND_WITH);
				JITContext_PopScope(&self->ji_ctx);
				self->ji_state = st->js_prev;
				jit_state_destroy(st);
			}
			return 1;

		default: break;
		}
	}
	return 0;
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ji_next(JITYieldFunctionIterator *__restrict self) {
	DREF DeeObject *result;
	int error;
	DeeThreadObject *ts = DeeThread_Self();
	if (JITYieldFunctionIterator_Acquire(self))
		return NULL;
	self->ji_ctx.jc_except = ts->t_exceptsz;
	self->ji_ctx.jc_flags  = JITCONTEXT_FNORMAL;

parse_again:
	/* Pop all single-statement states, allowing for loop blocks to re-evaluate
	 * their context and potentially jump back to continue execution at a prior
	 * point of execution. */
	while (self->ji_state->js_flag & JIT_STATE_FLAG_SINGLE) {
		error = JITYieldFunctionIterator_PopState(self);
		if unlikely(error < 0)
			goto err;
		if (error)
			break;
	}
parse_again_same_statement:
	switch (self->ji_lex.jl_tok) {

	case TOK_EOF:
		/* The iterator has been exhausted! */
		result = ITER_DONE;
		goto done;

	case '{': {
		struct jit_state *st;
		st = self->ji_state;

		/* Transform the current state to become block-scopes. */
		if (st->js_flag & JIT_STATE_FLAG_SINGLE) {
			st->js_flag &= ~JIT_STATE_FLAG_SINGLE;
		} else {
			/* Recursively defined block scope. */
			st = jit_state_alloc();
			if unlikely(!st)
				goto err;
			st->js_prev    = self->ji_state;
			st->js_kind    = JIT_STATE_KIND_SCOPE;
			st->js_flag    = JIT_STATE_FLAG_BLOCK;
			self->ji_state = st;
		}
		JITLexer_Yield(&self->ji_lex);
		/* Push an additional block-scope */
		JITContext_PushScope(&self->ji_ctx);
		goto parse_again_same_statement;
	}

	case '}': {
		struct jit_state *st;
		st = self->ji_state;
		if (st->js_flag & JIT_STATE_FLAG_SINGLE) {
			DeeError_Throwf(&DeeError_SyntaxError,
			                "Expected statement before `}' within `yield'-function");
			self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
			self->ji_ctx.jc_flags |= JITCONTEXT_FSYNERR;
			goto err;
		}
		if (st == &self->ji_bstat) {
			DeeError_Throwf(&DeeError_SyntaxError,
			                "Unmatched `}' encountered within `yield'-function");
			self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
			self->ji_ctx.jc_flags |= JITCONTEXT_FSYNERR;
			goto err;
		}
		/* Pop the additional block scope. */
		JITContext_PopScope(&self->ji_ctx);
		if (st->js_kind != JIT_STATE_KIND_SCOPE) {
			st->js_flag |= JIT_STATE_FLAG_SINGLE;
		} else {
			error = JITYieldFunctionIterator_PopState(self);
			if unlikely(error < 0)
				goto err;
			if (error)
				goto parse_again_same_statement;
		}
		JITLexer_Yield(&self->ji_lex);
		goto parse_again;
	}

	case JIT_KEYWORD: {
		char const *tok_begin;
		size_t tok_length;
		uint32_t name;
		tok_begin  = (char const *)self->ji_lex.jl_tokstart;
		tok_length = (size_t)((char const *)self->ji_lex.jl_tokend - tok_begin);
		switch (tok_length) {

		case 2:
			if (tok_begin[0] == 'i' &&
			    tok_begin[1] == 'f') {
				DREF DeeObject *value;
				int temp;
				JITLexer_Yield(&self->ji_lex);
				if likely(self->ji_lex.jl_tok == '(') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_if_expected_lparen_after_if(&self->ji_lex);
					goto err;
				}
				JITContext_PushScope(&self->ji_ctx);
				value = JITLexer_EvalRValueDecl(&self->ji_lex);
				if unlikely(!value)
					goto err_scope;
				temp = DeeObject_BoolInherited(value);
				if unlikely(temp < 0)
					goto err_scope;
				if likely(self->ji_lex.jl_tok == ')') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_if_expected_rparen_after_if(&self->ji_lex);
					goto err_scope;
				}
				if (temp) {
					/* Choose the true-branch (push a skip-else state). */
					struct jit_state *st;
					st = jit_state_alloc();
					if unlikely(!st)
						goto err_scope;
					st->js_kind    = JIT_STATE_KIND_SKIPELSE;
					st->js_flag    = JIT_STATE_FLAG_SINGLE;
					st->js_prev    = self->ji_state;
					self->ji_state = st;
					goto parse_again_same_statement;
				}
				/* Choose the false-branch (should it exist) */
				if unlikely(JITLexer_SkipStatement(&self->ji_lex))
					goto err_scope;
				if (self->ji_lex.jl_tok == JIT_KEYWORD &&
				    self->ji_lex.jl_tokend == self->ji_lex.jl_tokstart + 4) {
					uint32_t next_name;
					next_name = UNALIGNED_GET32(self->ji_lex.jl_tokstart);
					if (next_name == ENCODE_INT32('e', 'l', 's', 'e')) {
						JITLexer_Yield(&self->ji_lex);
						goto parse_else_after_if;
					} else if (next_name == ENCODE_INT32('e', 'l', 'i', 'f')) {
						self->ji_lex.jl_tokstart += 2; /* Transform into an `if' */
parse_else_after_if:
#if 1 /* Optimization: No need to push a scope if no declaration was made \
       *               within the condition expression of the if-statement. */
						if (self->ji_ctx.jc_locals.otp_ind >= 2) {
							--self->ji_ctx.jc_locals.otp_ind;
						} else
#endif
						{
							/* Must push a simple scope state to keep alive a declaration made within
							 * the condition of the if-branch. */
							struct jit_state *st;
							st = jit_state_alloc();
							if unlikely(!st)
								goto err_scope;
							st->js_kind    = JIT_STATE_KIND_SCOPE2;
							st->js_flag    = JIT_STATE_FLAG_SINGLE;
							st->js_prev    = self->ji_state;
							self->ji_state = st;
						}
						goto parse_again_same_statement;
					}
				}
				JITContext_PopScope(&self->ji_ctx);
				/* No live branch existed within the if-statement (move on to execute the next statement) */
				goto parse_again;
			}
			if (tok_begin[0] == 'd' &&
			    tok_begin[1] == 'o') {
				struct jit_state *st;
				/* Push a state for the do-while loop. */
				st = jit_state_alloc();
				if unlikely(!st)
					goto err;
				JITLexer_Yield(&self->ji_lex);
				st->js_kind           = JIT_STATE_KIND_DOWHILE;
				st->js_flag           = JIT_STATE_FLAG_SINGLE;
				st->js_dowhile.f_loop = self->ji_lex.jl_tokstart;
				st->js_dowhile.f_cond = NULL;
				st->js_prev           = self->ji_state;
				self->ji_state        = st;
				goto parse_again_same_statement;
			}
			break;

		case 3:
			if (tok_begin[0] == 't' && tok_begin[1] == 'r' &&
			    tok_begin[2] == 'y') {
				struct jit_state *st;
				JITLexer_Yield(&self->ji_lex);
				st = jit_state_alloc();
				if unlikely(!st)
					goto err;
				st->js_kind        = JIT_STATE_KIND_TRY;
				st->js_flag        = JIT_STATE_FLAG_SINGLE;
				st->js_try.t_guard = self->ji_lex.jl_tokstart;
				st->js_prev        = self->ji_state;
				self->ji_state     = st;
				goto parse_again_same_statement;
			}
			if (tok_begin[0] == 'f' && tok_begin[1] == 'o' &&
			    tok_begin[2] == 'r') {
				struct jit_state *st;
				JITLexer_Yield(&self->ji_lex);
				if likely(self->ji_lex.jl_tok == '(') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_for_expected_lparen_after_for(&self->ji_lex);
					goto err;
				}
				JITContext_PushScope(&self->ji_ctx);
				/* Check for special case: there is no initializer! */
				if (self->ji_lex.jl_tok == ';')
					goto do_normal_for_noinit;
				result = JITLexer_EvalComma(&self->ji_lex,
				                            JIT_AST_COMMA_NORMAL |
				                            JIT_AST_COMMA_ALLOWVARDECLS,
				                            NULL,
				                            NULL);
				if unlikely(!result)
					goto err_scope;
				if (self->ji_lex.jl_tok == ':') {
					JITLValue elem_lvalue;
					DREF DeeObject *iter;
					int temp;
					/* TODO: Multiple targets (`for (local x, y, z: triples)') */
					/* Initialize the foreach element target. */
					if (result == JIT_LVALUE) {
						elem_lvalue = self->ji_lex.jl_lvalue;
						JITLValue_Init(&self->ji_lex.jl_lvalue);
					} else {
						elem_lvalue.lv_kind   = JIT_LVALUE_RVALUE;
						elem_lvalue.lv_rvalue = result; /* Inherit reference */
					}
					JITLexer_Yield(&self->ji_lex);
					/* Parse the sequence expression. */
					result = JITLexer_EvalRValue(&self->ji_lex);
					if unlikely(!result) {
err_elem_lvalue_scope:
						JITLValue_Fini(&elem_lvalue);
						goto err_scope;
					}
					if likely(self->ji_lex.jl_tok == ')') {
						JITLexer_Yield(&self->ji_lex);
					} else {
						syn_for_expected_rparen_after_foreach(&self->ji_lex);
						Dee_Decref(result);
						goto err_elem_lvalue_scope;
					}
					iter = DeeObject_Iter(result);
					Dee_Decref(result);
					if unlikely(!iter)
						goto err_elem_lvalue_scope;
					/* We're now where the loop starts. */
					/* -> Load the first iterator element. */
					result = DeeObject_IterNext(iter);
					if (!ITER_ISOK(result)) {
						/* Iterator exhausted, or error. */
						Dee_Decref(iter);
						JITLValue_Fini(&elem_lvalue);
						JITContext_PopScope(&self->ji_ctx);
						if unlikely(!result)
							goto err;
						/* Skip the body of the for-statement */
						if (JITLexer_SkipStatement(&self->ji_lex))
							goto err;
						/* Parse the next statement. */
						goto parse_again;
					}
					/* Assign the first loop element to the iterator element. */
					temp = JITLValue_SetValue(&elem_lvalue,
					                          &self->ji_ctx,
					                          result);
					Dee_Decref(result);
					if unlikely(temp) {
err_iter_scope_lvalue:
						Dee_Decref(iter);
						goto err_elem_lvalue_scope;
					}
					/* Allocate the state used to track this foreach loop. */
					st = jit_state_alloc();
					if unlikely(!st)
						goto err_iter_scope_lvalue;
					st->js_flag           = JIT_STATE_FLAG_SINGLE;
					st->js_kind           = JIT_STATE_KIND_FOREACH;
					st->js_foreach.f_elem = elem_lvalue; /* Inherit reference */
					st->js_foreach.f_iter = iter;        /* Inherit reference */
					st->js_foreach.f_loop = self->ji_lex.jl_tokstart;
				} else {
					unsigned char const *cond_start;
					unsigned char const *next_start;
					/* Regular for statement. */
					/* Start by getting rid of the initializer value (we no longer need that one). */
					if (result == JIT_LVALUE) {
						JITLValue_Fini(&self->ji_lex.jl_lvalue);
						JITLValue_Init(&self->ji_lex.jl_lvalue);
					} else {
						Dee_Decref(result);
					}
					if (self->ji_lex.jl_tok == ';') {
do_normal_for_noinit:
						JITLexer_Yield(&self->ji_lex);
					} else {
						syn_for_expected_semi1_after_for(&self->ji_lex);
						goto err_scope;
					}
					cond_start = next_start = NULL;
					if (self->ji_lex.jl_tok == ';') {
						JITLexer_Yield(&self->ji_lex);
					} else {
						DREF DeeObject *value;
						int temp;
						/* Parse the initial condition expression,
						 * and check if the loop is even entered. */
						cond_start = self->ji_lex.jl_tokstart;
						value      = JITLexer_EvalRValue(&self->ji_lex);
						if unlikely(!value)
							goto err_scope;
						temp = DeeObject_BoolInherited(value);
						if unlikely(temp < 0)
							goto err_scope;
						if (self->ji_lex.jl_tok == ';') {
							JITLexer_Yield(&self->ji_lex);
						} else {
							syn_for_expected_semi2_after_for(&self->ji_lex);
							goto err_scope;
						}
						if (!temp) {
							/* The loop is never entered! (skip the entire statement) */
							JITContext_PopScope(&self->ji_ctx);
							if (self->ji_lex.jl_tok != ')' &&
							    JITLexer_SkipExpression(&self->ji_lex, JITLEXER_EVAL_FNORMAL))
								goto err;
							if likely(self->ji_lex.jl_tok == ')') {
								JITLexer_Yield(&self->ji_lex);
							} else {
err_missing_rparen_after_for:
								syn_for_expected_rparen_after_for(&self->ji_lex);
								goto err_scope;
							}
							/* Skip the body of the for-statement */
							if (JITLexer_SkipStatement(&self->ji_lex))
								goto err;
							/* Parse the next statement. */
							goto parse_again;
						}
					}
					/* Check if there is a next-expression, and parse it if there is one. */
					if (self->ji_lex.jl_tok == ')') {
						JITLexer_Yield(&self->ji_lex);
					} else {
						next_start = self->ji_lex.jl_tokstart;
						if (JITLexer_SkipExpression(&self->ji_lex, JITLEXER_EVAL_FNORMAL))
							goto err_scope;
						if likely(self->ji_lex.jl_tok == ')') {
							JITLexer_Yield(&self->ji_lex);
						} else {
							goto err_missing_rparen_after_for;
						}
					}

					/* Allocate the state used to track this for-loop. */
					st = jit_state_alloc();
					if unlikely(!st)
						goto err_scope;
					st->js_flag       = JIT_STATE_FLAG_SINGLE;
					st->js_kind       = JIT_STATE_KIND_FOR;
					st->js_for.f_cond = cond_start;
					st->js_for.f_next = next_start;
					st->js_for.f_loop = self->ji_lex.jl_tokstart;
				}
				/* Push the newly created state, and continue parsing the current statement. */
				st->js_prev    = self->ji_state;
				self->ji_state = st;
				goto parse_again_same_statement;
			}
			break;

		case 4:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('w', 'i', 't', 'h')) {
				struct jit_state *st;
				DREF DeeObject *obj;
				JITLexer_Yield(&self->ji_lex);
				if likely(self->ji_lex.jl_tok == '(') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_with_expected_lparen_after_with(&self->ji_lex);
					goto err;
				}
				JITContext_PushScope(&self->ji_ctx);
				obj = JITLexer_EvalRValueDecl(&self->ji_lex);
				if unlikely(!obj)
					goto err_scope;
				if likely(self->ji_lex.jl_tok == ')') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_with_expected_rparen_after_with(&self->ji_lex);
err_obj_scope:
					Dee_Decref(obj);
					goto err_scope;
				}
				st = jit_state_alloc();
				if unlikely(!st)
					goto err_obj_scope;
				/* Invoke `operator enter()' on the with-object */
				if unlikely(DeeObject_Enter(obj)) {
					jit_state_free(st);
					goto err_obj_scope;
				}
				st->js_kind       = JIT_STATE_KIND_WITH;
				st->js_flag       = JIT_STATE_FLAG_SINGLE;
				st->js_with.w_obj = obj; /* Inherit reference */
				st->js_prev       = self->ji_state;
				self->ji_state    = st;
				goto parse_again_same_statement;
			}
			break;

		case 5:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('y', 'i', 'e', 'l') &&
			    *(uint8_t *)(tok_begin + 4) == 'd') {
				/* The thing that we're actually after: `yield' statements! */
				JITLexer_Yield(&self->ji_lex);
				result = JITLexer_EvalRValue(&self->ji_lex);
				/* Consume the trailing `;' that is required for yield statements. */
				if likely(self->ji_lex.jl_tok == ';') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_yield_expected_semi_after_yield(&self->ji_lex);
					goto err_r;
				}
				goto got_yield_value;
			}
			if (name == ENCODE_INT32('w', 'h', 'i', 'l') &&
			    *(uint8_t *)(tok_begin + 4) == 'e') {
				struct jit_state *st;
				unsigned char const *cond_start;
				DREF DeeObject *value;
				int temp;
				JITLexer_Yield(&self->ji_lex);
				if likely(self->ji_lex.jl_tok == '(') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_while_expected_lparen_after_while(&self->ji_lex);
					goto err;
				}
				/* Evaluate the while-condition for the first time. */
				JITContext_PushScope(&self->ji_ctx);
				cond_start = self->ji_lex.jl_tokstart;
				value      = JITLexer_EvalRValueDecl(&self->ji_lex);
				if unlikely(!value)
					goto err_scope;
				if likely(self->ji_lex.jl_tok == ')') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					Dee_Decref(value);
					syn_while_expected_rparen_after_while(&self->ji_lex);
					goto err_scope;
				}
				temp = DeeObject_BoolInherited(value);
				if unlikely(temp < 0)
					goto err_scope;
				if (!temp) {
					/* The while-statement's body is never reached. */
					JITContext_PopScope(&self->ji_ctx);
					if (JITLexer_SkipStatement(&self->ji_lex))
						goto err;
					goto parse_again;
				}
				/* Push a state that can be used to describe the behavior of the while-loop */
				st = jit_state_alloc();
				if unlikely(!st)
					goto err_scope;
				st->js_kind         = JIT_STATE_KIND_WHILE;
				st->js_flag         = JIT_STATE_FLAG_SINGLE;
				st->js_while.f_cond = cond_start;
				st->js_while.f_loop = self->ji_lex.jl_tokstart;
				/* Push the new state. */
				st->js_prev    = self->ji_state;
				self->ji_state = st;
				goto parse_again_same_statement;
			}
			break;

		case 6:
			name = UNALIGNED_GET32(tok_begin);
#if 0 /* TODO */
			if (name == ENCODE_INT32('s', 'w', 'i', 't') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('c', 'h')) {
				DeeError_NOTIMPLEMENTED();
				goto err;
			}
#endif
			break;

		case 7:
			name = UNALIGNED_GET32(tok_begin);
			if (name == ENCODE_INT32('f', 'o', 'r', 'e') &&
			    UNALIGNED_GET16(tok_begin + 4) == ENCODE_INT16('a', 'c') &&
			    UNALIGNED_GET8(tok_begin + 6) == 'h') {
				struct jit_state *st;
				int temp;
				JITLValue elem_lvalue;
				DREF DeeObject *iter;
				JITLexer_Yield(&self->ji_lex);
				if likely(self->ji_lex.jl_tok == '(') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_foreach_expected_lparen_after_foreach(&self->ji_lex);
					goto err;
				}
				JITContext_PushScope(&self->ji_ctx);
				result = JITLexer_EvalComma(&self->ji_lex,
				                            JIT_AST_COMMA_NORMAL |
				                            JIT_AST_COMMA_ALLOWVARDECLS,
				                            NULL,
				                            NULL);
				if unlikely(!result)
					goto err_scope;
				if (self->ji_lex.jl_tok == ':') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					if (result == JIT_LVALUE) {
						JITLValue_Fini(&self->ji_lex.jl_lvalue);
						JITLValue_Init(&self->ji_lex.jl_lvalue);
					} else {
						Dee_Decref(result);
					}
					syn_foreach_expected_colon_after_foreach(&self->ji_lex);
					goto err_scope;
				}
				/* TODO: Multiple targets (`for (local x, y, z: triples)') */
				/* Initialize the foreach element target. */
				if (result == JIT_LVALUE) {
					elem_lvalue = self->ji_lex.jl_lvalue;
					JITLValue_Init(&self->ji_lex.jl_lvalue);
				} else {
					elem_lvalue.lv_kind   = JIT_LVALUE_RVALUE;
					elem_lvalue.lv_rvalue = result; /* Inherit reference */
				}
				JITLexer_Yield(&self->ji_lex);
				/* Parse the sequence expression. */
				iter = JITLexer_EvalRValue(&self->ji_lex);
				if unlikely(!iter) {
err_elem_lvalue_scope_2:
					JITLValue_Fini(&elem_lvalue);
					goto err_scope;
				}
				if likely(self->ji_lex.jl_tok == ')') {
					JITLexer_Yield(&self->ji_lex);
				} else {
					syn_foreach_expected_rparen_after_foreach(&self->ji_lex);
					Dee_Decref(result);
					goto err_elem_lvalue_scope_2;
				}
				/* We're now where the loop starts. */
				/* -> Load the first iterator element. */
				result = DeeObject_IterNext(iter);
				if (!ITER_ISOK(result)) {
					/* Iterator exhausted, or error. */
					Dee_Decref(iter);
					JITLValue_Fini(&elem_lvalue);
					JITContext_PopScope(&self->ji_ctx);
					if unlikely(!result)
						goto err;
					/* Skip the body of the for-statement */
					if (JITLexer_SkipStatement(&self->ji_lex))
						goto err;
					/* Parse the next statement. */
					goto parse_again;
				}
				/* Assign the first loop element to the iterator element. */
				temp = JITLValue_SetValue(&elem_lvalue,
				                          &self->ji_ctx,
				                          result);
				Dee_Decref(result);
				if unlikely(temp) {
err_iter_scope_lvalue_2:
					Dee_Decref(iter);
					goto err_elem_lvalue_scope_2;
				}
				/* Allocate the state used to track this foreach loop. */
				st = jit_state_alloc();
				if unlikely(!st)
					goto err_iter_scope_lvalue_2;
				st->js_flag           = JIT_STATE_FLAG_SINGLE;
				st->js_kind           = JIT_STATE_KIND_FOREACH;
				st->js_foreach.f_elem = elem_lvalue; /* Inherit reference */
				st->js_foreach.f_iter = iter;        /* Inherit reference */
				st->js_foreach.f_loop = self->ji_lex.jl_tokstart;
				/* Push the newly created state, and continue parsing the current statement. */
				st->js_prev    = self->ji_state;
				self->ji_state = st;
				goto parse_again_same_statement;
			}
			break;

		default: break;
		}
		goto parse_generic_statement;
	}	break;

	default:
parse_generic_statement:
		/* Fallback: Parse a regular statement. */
		result = JITLexer_EvalStatement(&self->ji_lex);
		if unlikely(!result)
			goto err;
		if (result == JIT_LVALUE) {
			JITLValue_Fini(&self->ji_lex.jl_lvalue);
			self->ji_lex.jl_lvalue.lv_kind = JIT_LVALUE_NONE;
		} else {
			Dee_Decref(result);
		}
		goto parse_again;
	}
got_yield_value:

	ASSERT(result != JIT_LVALUE);
	if (!result) {
err:
		if (self->ji_ctx.jc_retval != JITCONTEXT_RETVAL_UNSET) {
			if (self->ji_ctx.jc_retval == JITCONTEXT_RETVAL_BREAK ||
			    self->ji_ctx.jc_retval == JITCONTEXT_RETVAL_CONTINUE) {
				bool is_break = self->ji_ctx.jc_retval == JITCONTEXT_RETVAL_BREAK;
				/* Try to service the break/continue control command using
				 * the currently active compiler context state stack. */
				error = JITYieldFunctionIterator_HandleLoopctl(self, is_break);
				if unlikely(error < 0)
					goto err;
				if (error) { /* Resume execution at the location that was unwound */
					self->ji_ctx.jc_retval = JITCONTEXT_RETVAL_UNSET;
					goto parse_again_same_statement;
				}
			}
			if (JITCONTEXT_RETVAL_ISSET(self->ji_ctx.jc_retval)) {
				/* Deal with use of the return statement. */
				result                 = self->ji_ctx.jc_retval;
				self->ji_ctx.jc_retval = JITCONTEXT_RETVAL_UNSET;
				if (DeeNone_Check(result)) {
					/* `return;' on its own causes `return none',
					 * which we must interpret as stop-iteration. */
					Dee_Decref(result);
					result = ITER_DONE;
				} else {
					Dee_Decref(result);
					DeeError_Throwf(&DeeError_SyntaxError,
					                "`return' statement encountered within `yield'-function");
					self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
					goto handle_error;
				}
			} else {
				/* Exited code via unconventional means, such as `break' or `continue' */
				DeeError_Throwf(&DeeError_SyntaxError,
				                "Attempted to use `break' or `continue' outside of a loop");
				self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
				goto handle_error;
			}
		} else {
			/* Only service try-handlers if it wasn't a syntax
			 * error that caused the currently active exception. */
			if (!(self->ji_ctx.jc_flags & JITCONTEXT_FSYNERR)) {
				/* Not a syntax-error. - Check for catch/finally blocks and handle
				 * catch expressions, as well as execute finally-statements! */
				struct jit_state *st;
again_check_try_statements:
				st = self->ji_state;
				for (; st != &self->ji_bstat; st = st->js_prev) {
					if (st->js_kind != JIT_STATE_KIND_TRY)
						continue;
					/* Found a try-state (unwind to it) */
					if unlikely(JITYieldFunctionIterator_UnwindUntil(self, st))
						goto err;
					/* If the try was followed by a block, we must unwind that block as well */
					if (!(st->js_flag & JIT_STATE_FLAG_SINGLE))
						JITContext_PopScope(&self->ji_ctx);
					/* Jump to the start of handlers of this try-statement. */
					JITLexer_YieldAt(&self->ji_lex, st->js_try.t_guard);
					if (JITLexer_SkipStatement(&self->ji_lex))
						goto err;
					/* Pop the state of the try-block (so that its handlers aren't guarding themself!) */
					self->ji_state = st->js_prev;
					jit_state_free(st);
					/* Service handlers. */
service_exception_handlers:
					for (;;) {
						bool allow_interrupts = false;
						/* XXX: Full tagging support? */
						if (self->ji_lex.jl_tok == '@') {
							JITLexer_Yield(&self->ji_lex);
							if (self->ji_lex.jl_tok == '[') {
								JITLexer_Yield(&self->ji_lex);
								if (JITLexer_ISKWD(&self->ji_lex, "interrupt")) {
									JITLexer_Yield(&self->ji_lex);
									allow_interrupts = true;
								}
							}
							if (self->ji_lex.jl_tok == ']') {
								JITLexer_Yield(&self->ji_lex);
							} else {
								syn_anno_expected_rbracket(&self->ji_lex);
								goto err;
							}
						}
						if (JITLexer_ISKWD(&self->ji_lex, "finally")) {
							DREF DeeObject *value;
							JITLexer_Yield(&self->ji_lex);
							/* Evaluate the finally-statement (NOTE: JIT doesn't allow yield-in-finally) */
							value = JITLexer_EvalStatement(&self->ji_lex);
							if unlikely(!value)
								goto err;
							if (value == JIT_LVALUE) {
								JITLValue_Fini(&self->ji_lex.jl_lvalue);
								JITLValue_Init(&self->ji_lex.jl_lvalue);
							} else {
								Dee_Decref(value);
							}
						} else if (JITLexer_ISKWD(&self->ji_lex, "catch")) {
							DREF DeeObject *typemask;
							DeeObject *current;
							char const *symbol_name;
							size_t symbol_size;
							/* Simply skip catch statements. */
							JITLexer_Yield(&self->ji_lex);
							if unlikely(self->ji_lex.jl_tok != '(') {
								syn_try_expected_lparen_after_catch(&self->ji_lex);
								goto err;
							}
							JITLexer_Yield(&self->ji_lex);
							JITContext_PushScope(&self->ji_ctx);
							/* Parse the mask of this catch statement! */
							if unlikely(JITLexer_ParseCatchMask(&self->ji_lex,
							                                    &typemask,
							                                    &symbol_name,
							                                    &symbol_size))
								goto err_scope;
							current = DeeError_Current();
							ASSERT(current != NULL);

							/* Check if we're allowed to handle this exception! */
							if ((!typemask || JIT_IsCatchable(current, typemask)) &&
							    (allow_interrupts || !DeeObject_IsInterrupt(current))) {
								uint16_t old_except_sz;
								unsigned char const *handler_start;
								Dee_XDecref(typemask);
								/* Save the current exception as a local variable. */
								if (symbol_size) {
									JITObjectTable *tab;
									tab = JITContext_GetRWLocals(&self->ji_ctx);
									if unlikely(!tab)
										goto err_scope;
									if (JITObjectTable_Update(tab,
									                          symbol_name,
									                          symbol_size,
									                          Dee_HashUtf8(symbol_name, symbol_size),
									                          current,
									                          true) < 0)
										goto err_scope;
								}
								/* Evaluate the catch-statement */
								old_except_sz = ts->t_exceptsz;
								handler_start = self->ji_lex.jl_tokstart;
								result        = JITLexer_EvalStatement(&self->ji_lex);
								if unlikely(!result) {
									if (self->ji_ctx.jc_flags & JITCONTEXT_FSYNERR)
										goto err_scope;
									JITContext_PopScope(&self->ji_ctx);
									JITLexer_YieldAt(&self->ji_lex, handler_start);
									if (JITLexer_SkipStatement(&self->ji_lex))
										goto err;
									/* Must still handle the original exception. */
									ASSERT(ts->t_exceptsz >= old_except_sz);
									if (ts->t_exceptsz == old_except_sz) {
										ASSERT(ts->t_except);
										ASSERT(ts->t_except->ef_error == current);
										goto service_exception_handlers; /* The previous exception was simply re-thrown */
									}
									/* A new exception was thrown on-top of ours. (we must still handle our old one) */
									{
										uint16_t ind = ts->t_exceptsz - old_except_sz;
										struct Dee_except_frame *exc, **p_exc;
										exc = *(p_exc = &ts->t_except);
										while (ind--) {
											p_exc = &exc->ef_prev;
											exc  = *p_exc;
										}
										*p_exc = exc->ef_prev;
										--ts->t_exceptsz;
										/* Destroy the frame in question. */
										if (ITER_ISOK(exc->ef_trace))
											Dee_Decref((DeeObject *)exc->ef_trace);
										Dee_Decref(exc->ef_error);
										Dee_except_frame_free(exc);
									}
									goto service_exception_handlers;
								}
								/* The exception was handled normally. */
								DeeError_Handled(ERROR_HANDLED_INTERRUPT);
								JITContext_PopScope(&self->ji_ctx);
								/* Discard the value returned by the catch-statement */
								if (result == JIT_LVALUE) {
									JITLValue_Fini(&self->ji_lex.jl_lvalue);
									JITLValue_Init(&self->ji_lex.jl_lvalue);
								} else {
									Dee_Decref(result);
								}
								/* Execute all additional finally-blocks, but skip any other catch-blocks! */
								if unlikely(JITLexer_EvalFinallyStatements(&self->ji_lex))
									goto err;
								/* Continue parsing, now that the exception has been handled. */
								goto parse_again;
							}
							Dee_XDecref(typemask);
							JITContext_PopScope(&self->ji_ctx);
							if (JITLexer_SkipStatement(&self->ji_lex))
								goto err;
						} else {
							break;
						}
					}
					goto again_check_try_statements;
				}
			}
			if (!self->ji_lex.jl_errpos)
				self->ji_lex.jl_errpos = self->ji_lex.jl_tokstart;
handle_error:
			JITLValue_Fini(&self->ji_lex.jl_lvalue);
			self->ji_lex.jl_lvalue.lv_kind = JIT_LVALUE_NONE;
			result                         = NULL;
			/* TODO: Somehow remember that the error happened at `lexer.jl_errpos' */
		}
		self->ji_lex.jl_tok = TOK_EOF; /* Don't iterate again. */
	}
	ASSERT(ts->t_exceptsz >= self->ji_ctx.jc_except);
	if (ts->t_exceptsz > self->ji_ctx.jc_except) {
		if (self->ji_ctx.jc_retval != JITCONTEXT_RETVAL_UNSET) {
			if (JITCONTEXT_RETVAL_ISSET(self->ji_ctx.jc_retval))
				Dee_Decref(self->ji_ctx.jc_retval);
			self->ji_ctx.jc_retval = JITCONTEXT_RETVAL_UNSET;
		}
		if (ITER_ISOK(result))
			Dee_Decref(result);
		result = NULL;
		while (ts->t_exceptsz > self->ji_ctx.jc_except + 1) {
			DeeError_Print("Discarding secondary error",
			               ERROR_PRINT_DOHANDLE);
		}
	}
done:
	JITYieldFunctionIterator_Release(self);
	ASSERTF(result ? ts->t_exceptsz == self->ji_ctx.jc_except + 0
	               : ts->t_exceptsz == self->ji_ctx.jc_except + 1,
	        "ts->t_exceptsz         = %" PRFu16 "\n"
	        "self->ji_ctx.jc_except = %" PRFu16,
	        ts->t_exceptsz, self->ji_ctx.jc_except);
	return result;
err_scope:
	JITContext_PopScope(&self->ji_ctx);
	goto err;
err_r:
	if (result == JIT_LVALUE) {
		JITLValue_Fini(&self->ji_lex.jl_lvalue);
		JITLValue_Init(&self->ji_lex.jl_lvalue);
	} else {
		Dee_Decref(result);
	}
	goto err;
}



PRIVATE NONNULL((1)) void DCALL
ji_fini(JITYieldFunctionIterator *__restrict self) {
	JITFunctionObject *jf = self->ji_func->jy_func;
	ASSERT(!jf->jf_globals || self->ji_ctx.jc_globals == jf->jf_globals);
	while (self->ji_state != &self->ji_bstat) {
		struct jit_state *prev, *curr;
		curr = self->ji_state;
		prev = curr->js_prev;
		switch (curr->js_kind) {

		case JIT_STATE_KIND_TRY: {
			DeeThreadObject *ts;
			/* Service finally-blocks. */
			if (self->ji_ctx.jc_flags & JITCONTEXT_FSYNERR)
				break; /* Don't service if a syntax-error occurred. */
			JITLexer_YieldAt(&self->ji_lex, curr->js_try.t_guard);
			ts                     = DeeThread_Self();
			self->ji_ctx.jc_except = ts->t_exceptsz;
			/* Skip the guarded statement block. */
			if (JITLexer_SkipStatement(&self->ji_lex))
				goto err_try;
			/* Service all finally statements that were used to guard this block. */
			if (JITLexer_EvalFinallyStatements(&self->ji_lex))
				goto err_try;
err_try:
			/* Dump all unhandled exceptions caused by */
			while (ts->t_exceptsz > self->ji_ctx.jc_except)
				DeeError_Print(NULL, ERROR_PRINT_DOHANDLE);
		}	break;

		case JIT_STATE_KIND_WITH:
			if unlikely(DeeObject_Leave(curr->js_with.w_obj)) {
				/* Dump the unhandled exception! */
				DeeError_Print("Unhandled exception in `operator leave'",
				               ERROR_PRINT_DOHANDLE);
			}
			break;

		default: break;
		}
		jit_state_destroy(curr);
		self->ji_state = prev;
	}
	/* Destroy any remaining L-value expressions. */
	JITLValue_Fini(&self->ji_lex.jl_lvalue);
	/* Pop remaining scopes. */
	while (self->ji_ctx.jc_locals.otp_tab != &self->ji_loc) {
		ASSERT(self->ji_ctx.jc_locals.otp_tab != NULL);
		self->ji_ctx.jc_locals.otp_ind = 0;
		_JITContext_PopLocals(&self->ji_ctx);
	}
	/* Cleanup custom globals (if those had been accessed at one point) */
	if (self->ji_ctx.jc_globals != jf->jf_globals)
		Dee_Decref(self->ji_ctx.jc_globals);
	/* Destroy the function's base-scope. */
	JITObjectTable_Fini(&self->ji_loc);
	/* Drop our reference to the associated function. */
	Dee_Decref(self->ji_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
ji_visit(JITYieldFunctionIterator *__restrict self, Dee_visit_t proc, void *arg) {
	JITFunctionObject *jf = self->ji_func->jy_func;
	JITObjectTable *tab;
	JITYieldFunctionIterator_AcquireNoInt(self); /* Read lock would be enough here... */
	ASSERT(!jf->jf_globals || self->ji_ctx.jc_globals == jf->jf_globals);
	/* Destroy any remaining L-value expressions. */
	JITLValue_Visit(&self->ji_lex.jl_lvalue, proc, arg);
	/* Pop remaining scopes. */
	tab = self->ji_ctx.jc_locals.otp_tab;
	for (;;) {
		ASSERT(tab != NULL);
		JITObjectTable_Visit(tab, proc, arg);
		if (tab == &self->ji_loc)
			break;
		tab = tab->ot_prev.otp_tab;
	}
	if (self->ji_ctx.jc_globals != jf->jf_globals)
		Dee_Visit(self->ji_ctx.jc_globals);
	Dee_Visit(self->ji_func);
	JITYieldFunctionIterator_Release(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF JITFunction *DCALL
ji_getfunc(JITYieldFunctionIterator *__restrict self) {
	return_reference_(self->ji_func->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ji_getargs(JITYieldFunctionIterator *__restrict self) {
	return jy_getargs(self->ji_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ji_getname(JITYieldFunctionIterator *__restrict self) {
	return jf_getname(self->ji_func->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ji_getdoc(JITYieldFunctionIterator *__restrict self) {
	return jf_getdoc(self->ji_func->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ji_getkwds(JITYieldFunctionIterator *__restrict self) {
	return jf_getkwds(self->ji_func->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ji_gettext(JITYieldFunctionIterator *__restrict self) {
	return jf_gettext(self->ji_func->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ji_getrefs(JITYieldFunctionIterator *__restrict self) {
	return jf_getrefs(self->ji_func->jy_func);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ji_getrefsbyname(JITYieldFunctionIterator *__restrict self) {
	return jf_getrefsbyname(self->ji_func->jy_func);
}


PRIVATE struct type_getset tpconst ji_getsets[] = {
	TYPE_GETTER_F("__func__", &ji_getfunc, METHOD_FNOREFESCAPE,
	              "->?GFunction\n"
	              "The JIT function that is being executed"),
	TYPE_GETTER_F("__args__", &ji_getargs, METHOD_FNOREFESCAPE,
	              "->?S?O\n"
	              "Returns a sequence representing the positional arguments passed to the function"),
	TYPE_GETTER_F("__name__", &ji_getname, METHOD_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "Alias for ?A__name__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__doc__", &ji_getdoc, METHOD_FNOREFESCAPE,
	              "->?X2?Dstring?N\n"
	              "Alias for ?A__doc__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__kwds__", &ji_getkwds, METHOD_FNOREFESCAPE,
	              "->?S?Dstring\n"
	              "Alias for ?A__kwds__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__text__", &ji_gettext, METHOD_FNOREFESCAPE,
	              "->?Dstring\n"
	              "Alias for ?A__text__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__refs__", &ji_getrefs, METHOD_FNOREFESCAPE,
	              "->?S?O\n"
	              "Alias for ?A__refs__?GFunction though ?#__func__"),
	TYPE_GETTER_F("__refsbyname__", &ji_getrefsbyname, METHOD_FNOREFESCAPE,
	              "->?M?Dstring?O\n"
	              "Alias for ?A__refsbyname__?GFunction though ?#__func__"),
	/* TODO: __argsbyname__ */
	/* TODO: __defaults__ */
	/* TODO: __type__ */
	/* TODO: __operator__ */
	/* TODO: __operatorname__ */
	/* TODO: __property__ */
	/* TODO: __statics__ */
	/* TODO: __staticsbyname__ */
	/* TODO: __symbols__ */
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ji_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT_AB, offsetof(JITYieldFunctionIterator, ji_func),
	                      "->?GYieldFunction\n"
	                      "Alias for ?#__yfunc__"),
	TYPE_MEMBER_FIELD_DOC("__yfunc__", STRUCT_OBJECT_AB, offsetof(JITYieldFunctionIterator, ji_func),
	                      "->?GYieldFunction\n"
	                      "The underlying yield-function"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject JITYieldFunctionIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_JitYieldFunctionIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ JITYieldFunctionIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ji_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ji_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ji_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ ji_getsets,
	/* .tp_members       = */ ji_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
};


DECL_END

#endif /* !GUARD_DEX_STREXEC_YIELDFUNCTION_C */
