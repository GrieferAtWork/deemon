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
#ifndef GUARD_DEX_JIT_FUNCTION_C
#define GUARD_DEX_JIT_FUNCTION_C 1
#define DEE_SOURCE 1

#include "libjit.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

DECL_BEGIN

typedef JITFunctionObject JITFunction;

INTERN bool DCALL
JITFunction_TryRehashArguments(JITFunction *__restrict self,
                               size_t new_mask) {
	size_t i, j, perturb;
	struct jit_object_entry *new_table;
	ASSERT(new_mask >= self->jf_args.ot_used);
	ASSERT(new_mask != 0);
	new_table = (struct jit_object_entry *)Dee_TryCalloc((new_mask + 1) *
	                                                     sizeof(struct jit_object_entry));
	if
		unlikely(!new_table)
	return false;
	if (self->jf_args.ot_list != jit_empty_object_list) {
		struct jit_object_entry *old_table;
		old_table = self->jf_args.ot_list;
		for (i = 0; i <= self->jf_args.ot_mask; ++i) {
			uint16_t argi;
			struct jit_object_entry *old_entry, *new_entry;
			old_entry = &old_table[i];
			if (!ITER_ISOK(old_entry->oe_namestr))
				continue; /* Unused or deleted. */
			perturb = j = old_entry->oe_namehsh & new_mask;
			for (;; JITObjectTable_NEXT(j, perturb)) {
				new_entry = &new_table[j & new_mask];
				if (!new_entry->oe_namestr)
					break;
			}
			j &= new_mask;
			/* Re-hash argument indices. */
			if (self->jf_selfarg == i)
				self->jf_selfarg = j;
			else if (self->jf_varargs == i)
				self->jf_varargs = j;
			else if (self->jf_varkwds == i)
				self->jf_varkwds = j;
			else {
				for (argi = 0; argi < self->jf_argc_max; ++argi) {
					if (self->jf_argv[argi] != i)
						continue;
					self->jf_argv[argi] = j;
					break;
				}
			}
			/* Copy into the new entry. */
			memcpy(new_entry, old_entry, sizeof(struct jit_object_entry));
		}
		Dee_Free(old_table);
		/* Indicate that all deleted entries have been removed. */
		self->jf_args.ot_used = self->jf_args.ot_size;
	} else {
		ASSERT(self->jf_args.ot_used == 0);
		ASSERT(self->jf_args.ot_size == 0);
		ASSERT(self->jf_args.ot_mask == 0);
	}
	self->jf_args.ot_list = new_table;
	self->jf_args.ot_mask = new_mask;
	return true;
}


/* Add a new symbol entry for an argument to `self->jf_args'
 * This is similar to using `JITObjectTable_Create()', however
 * when re-hashing, this function will also update indices contained
 * within the `self->jf_argv' vector, as well as the `self->jf_selfarg',
 * `self->jf_varargs' and `self->jf_varkwds' fields. */
INTERN struct jit_object_entry *DCALL
JITFunction_CreateArgument(JITFunction *__restrict self,
                           /*utf-8*/ char const *namestr,
                           size_t namelen) {
	dhash_t i, perturb;
	struct jit_object_entry *result_entry;
	dhash_t namehsh = Dee_HashUtf8(namestr, namelen);
again:
	result_entry = NULL;
	perturb = i = namehsh & self->jf_args.ot_mask;
	for (;; JITObjectTable_NEXT(i, perturb)) {
		struct jit_object_entry *entry;
		entry = &self->jf_args.ot_list[i & self->jf_args.ot_mask];
		if (entry->oe_namestr == (char *)ITER_DONE) {
			/* Re-use deleted entries. */
			if (!result_entry)
				result_entry = entry;
			continue;
		}
		if (!entry->oe_namestr) {
			if (!result_entry) {
				/* Check if we must re-hash the table. */
				if (self->jf_args.ot_size + 1 >= (self->jf_args.ot_mask * 2) / 3) {
					size_t new_mask;
					new_mask = (self->jf_args.ot_mask << 1) | 1;
					if (self->jf_args.ot_used < self->jf_args.ot_size)
						new_mask = self->jf_args.ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
					if (new_mask < 7)
						new_mask = 7;
					if
						likely(JITFunction_TryRehashArguments(self, new_mask))
					goto again;
					if (self->jf_args.ot_size == self->jf_args.ot_mask) {
						new_mask = (self->jf_args.ot_mask << 1) | 1;
						if (self->jf_args.ot_used < self->jf_args.ot_size)
							new_mask = self->jf_args.ot_mask; /* It's enough if we just rehash to get rid of deleted entries. */
						for (;;) {
							if
								likely(JITFunction_TryRehashArguments(self, new_mask))
							goto again;
							if
								unlikely(!Dee_CollectMemory((new_mask + 1) * sizeof(struct jit_object_entry)))
							return NULL;
						}
					}
				}
				++self->jf_args.ot_size;
				result_entry = entry;
			}
			break;
		}
		if (entry->oe_namehsh != namehsh)
			continue;
		if (entry->oe_namelen != namelen)
			continue;
		if (memcmp(entry->oe_namestr, namestr, namelen * sizeof(char)) != 0)
			continue;
		/* Existing entry! */
		return entry;
	}
	++self->jf_args.ot_used;
	result_entry->oe_namestr = namestr;
	result_entry->oe_namelen = namelen;
	result_entry->oe_namehsh = namehsh;
	result_entry->oe_value   = NULL;
	return result_entry;
}

PRIVATE DREF DeeObject *DCALL
JITLexer_ParseDefaultValue(JITLexer *__restrict self,
                           DeeModuleObject *impbase,
                           DeeObject *globals) {
	JITContext ctx      = JITCONTEXT_INIT;
	DeeThreadObject *ts = DeeThread_Self();
	DREF DeeObject *result;
	self->jl_context        = &ctx;
	self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
	ctx.jc_impbase          = impbase;
	ctx.jc_locals.otp_tab   = NULL;
	ctx.jc_locals.otp_ind   = 0;
	ctx.jc_globals          = globals;
	ctx.jc_retval           = NULL;
	ctx.jc_except           = ts->t_exceptsz;
	ctx.jc_flags            = 0;
	result                  = JITLexer_EvalExpression(self, JITLEXER_EVAL_FALLOWISBOUND);
	if (result == JIT_LVALUE) {
		result = JITLValue_GetValue(&self->jl_lvalue, &ctx);
		JITLValue_Fini(&self->jl_lvalue);
		self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
	}
	ASSERT(ts->t_exceptsz >= ctx.jc_except);
	if (!result)
		result = ctx.jc_retval;
	/* Check for non-propagated exceptions. */
	if (ts->t_exceptsz > ctx.jc_except) {
		if (ctx.jc_retval != JITCONTEXT_RETVAL_UNSET) {
			if (JITCONTEXT_RETVAL_ISSET(ctx.jc_retval))
				Dee_Decref(ctx.jc_retval);
			ctx.jc_retval = JITCONTEXT_RETVAL_UNSET;
		}
		Dee_XClear(result);
		while (ts->t_exceptsz > ctx.jc_except + 1) {
			DeeError_Print("Discarding secondary error\n",
			               ERROR_PRINT_DOHANDLE);
		}
	}
	if (!result) {
		if (ctx.jc_retval != JITCONTEXT_RETVAL_UNSET) {
			if (JITCONTEXT_RETVAL_ISSET(ctx.jc_retval)) {
				result = ctx.jc_retval;
			} else {
				/* Exited code via unconventional means, such as `break' or `continue' */
				DeeError_Throwf(&DeeError_SyntaxError,
				                "Attempted to use `break' or `continue' outside of a loop");
				JITLValue_Fini(&self->jl_lvalue);
				self->jl_errpos = self->jl_tokstart;
				/*result = NULL;*/
			}
		}
	}
	if (ctx.jc_globals != globals) {
		ASSERT(ctx.jc_globals);
		Dee_Decref(ctx.jc_globals);
	}
	return result;
}


INTERN DREF DeeObject *DCALL
JITFunction_New(/*utf-8*/ char const *name_start,
                /*utf-8*/ char const *name_end,
                /*utf-8*/ char const *params_start,
                /*utf-8*/ char const *params_end,
                /*utf-8*/ char const *__restrict source_start,
                /*utf-8*/ char const *__restrict source_end,
                JITObjectTable *parent_object_table,
                DeeObject *__restrict source,
                DeeModuleObject *impbase,
                DeeObject *globals,
                uint16_t flags) {
	JITLexer lex;
	DREF JITFunction *result;
	result = DeeObject_MALLOC(JITFunction);
	if
		unlikely(!result)
	goto done;
	result->jf_source_start = source_start;
	result->jf_source_end   = source_end;
	result->jf_source       = source;
	result->jf_impbase      = impbase;
	result->jf_globals      = globals;
	JITObjectTable_Init(&result->jf_args);
	JITObjectTable_Init(&result->jf_refs);
	result->jf_args.ot_prev.otp_ind = 2;
	result->jf_args.ot_prev.otp_tab = &result->jf_refs;
	result->jf_refs.ot_prev.otp_ind = 0;
	result->jf_refs.ot_prev.otp_tab = NULL;
	result->jf_argv                 = NULL;
	result->jf_selfarg              = (size_t)-1;
	result->jf_varargs              = (size_t)-1;
	result->jf_varkwds              = (size_t)-1;
	result->jf_argc_min             = 0;
	result->jf_argc_max             = 0;
	result->jf_flags                = flags;
	Dee_Incref(source);
	Dee_XIncref(impbase);
	Dee_XIncref(globals);
	DeeObject_Init(result, &JITFunction_Type);

	if (params_end > params_start) {
		uint16_t arga = 0;
		/* Analyze & parse the parameter list. */
		JITLexer_Start(&lex,
		               (unsigned char *)params_start,
		               (unsigned char *)params_end);
		while (lex.jl_tok) {
			/* Special case: unnamed varargs. */
			struct jit_object_entry *argent;
			if (lex.jl_tok == TOK_DOTS) {
				if (result->jf_varargs != (size_t)-1) {
err_varargs_already_defined:
					DeeError_Throwf(&DeeError_SyntaxError,
					                "Variable arguments have already been defined");
					goto err_r;
				}
				argent = JITFunction_CreateArgument(result,
				                                    (char const *)lex.jl_tokstart,
				                                    (size_t)(lex.jl_tokend - lex.jl_tokstart));
				if
					unlikely(!argent)
				goto err_r;
				result->jf_varargs = (size_t)(argent - result->jf_args.ot_list);
				JITLexer_Yield(&lex);
			} else {
				while (lex.jl_tok == JIT_KEYWORD &&
				       (JITLexer_ISTOK(&lex, "local") ||
				        JITLexer_ISTOK(&lex, "final") ||
				        JITLexer_ISTOK(&lex, "varying")))
					JITLexer_Yield(&lex);
				/* Check for keyword arguments parameter. */
				if (lex.jl_tok == TOK_POW) {
					if (result->jf_varkwds != (size_t)-1) {
						DeeError_Throwf(&DeeError_SyntaxError,
						                "Variable keywords have already been defined");
						goto err_r;
					}
					JITLexer_Yield(&lex);
					if (lex.jl_tok != JIT_KEYWORD) {
err_no_keyword_for_argument:
						DeeError_Throwf(&DeeError_SyntaxError,
						                "Expected a keyword as argument name, but got `%$s'",
						                (size_t)(lex.jl_tokend - lex.jl_tokstart), lex.jl_tokstart);
						goto err_r;
					}
					argent = JITFunction_CreateArgument(result,
					                                    (char const *)lex.jl_tokstart,
					                                    (size_t)(lex.jl_tokend - lex.jl_tokstart));
					if
						unlikely(!argent)
					goto err_r;
					JITLexer_Yield(&lex);
					result->jf_varkwds = (size_t)(argent - result->jf_args.ot_list);
				} else {
					if (lex.jl_tok != JIT_KEYWORD)
						goto err_no_keyword_for_argument;
					argent = JITFunction_CreateArgument(result,
					                                    (char const *)lex.jl_tokstart,
					                                    (size_t)(lex.jl_tokend - lex.jl_tokstart));
					if
						unlikely(!argent)
					goto err_r;
					JITLexer_Yield(&lex);
					if (lex.jl_tok == TOK_DOTS) {
						/* Varargs... */
						if (result->jf_varargs != (size_t)-1)
							goto err_varargs_already_defined;
						result->jf_varargs = (size_t)(argent - result->jf_args.ot_list);
						JITLexer_Yield(&lex);
					} else {
						if (result->jf_varargs != (size_t)-1 ||
						    result->jf_varkwds != (size_t)-1) {
							DeeError_Throwf(&DeeError_SyntaxError,
							                "Positional argument `%$s' encountered after varargs or varkwds",
							                argent->oe_namelen, argent->oe_namestr);
							goto err_r;
						}
						if (lex.jl_tok == '?') {
							/* Optional argument */
							JITLexer_Yield(&lex);
						} else if (lex.jl_tok == '=') {
							DREF DeeObject *default_value;
							JITLexer_Yield(&lex);
							lex.jl_text = source;
							/* Parse the default value. */
							default_value = JITLexer_ParseDefaultValue(&lex, impbase, globals);
							if
								unlikely(!default_value)
							goto err_r;
							if
								unlikely(argent->oe_value)
							Dee_Clear(argent->oe_value);
							argent->oe_value = default_value; /* Inherit reference. */
						} else {
							if (result->jf_argc_min != result->jf_argc_max) {
								DeeError_Throwf(&DeeError_SyntaxError,
								                "Mandatory positional argument `%$s' encountered after optional or default argument",
								                argent->oe_namelen, argent->oe_namestr);
								goto err_r;
							}
							++result->jf_argc_min;
						}
						/* Resize `jf_argv' if necessary. */
						ASSERT(result->jf_argc_max <= arga);
						if (result->jf_argc_max >= arga) {
							uint16_t new_arga;
							size_t *new_argv;
							new_arga = arga * 2;
							if
								unlikely(!new_arga)
							new_arga = 2;
							if
								unlikely(new_arga < arga)
							{
								new_arga = result->jf_argc_max + 1;
								if
									unlikely(new_arga < arga)
								{
									DeeError_Throwf(&DeeError_SyntaxError,
									                "Too many arguments");
									goto err_r;
								}
							}
							new_argv = (size_t *)Dee_TryRealloc(result->jf_argv,
							                                    new_arga * sizeof(size_t));
							if
								unlikely(!new_argv)
							{
								new_arga = result->jf_argc_max + 1;
								new_argv = (size_t *)Dee_Realloc(result->jf_argv,
								                                 new_arga * sizeof(size_t));
								if
									unlikely(!new_argv)
								goto err_r;
							}
							result->jf_argv = new_argv;
							arga            = new_arga;
						}
						result->jf_argv[result->jf_argc_max] = (size_t)(argent - result->jf_args.ot_list);
						++result->jf_argc_max;
					}
				}
			}
			if (!lex.jl_tok)
				break;
			if (lex.jl_tok != ',') {
				DeeError_Throwf(&DeeError_SyntaxError,
				                "Expected `,' after argument, but got `%$s'",
				                (size_t)(lex.jl_tokend - lex.jl_tokstart), lex.jl_tokstart);
				break;
			}
			JITLexer_Yield(&lex);
			/* ... */
		}
		ASSERT(result->jf_argc_max <= arga);
		if (result->jf_argc_max < arga) {
			/* Release unused memory */
			size_t *new_argv;
			new_argv = (size_t *)Dee_TryRealloc(result->jf_argv,
			                                    result->jf_argc_max * sizeof(size_t));
			if
				likely(new_argv)
			result->jf_argv = new_argv;
		}
	}

	if (name_start < name_end) {
		struct jit_object_entry *ent;
		size_t len = (size_t)(name_end - name_start);
		ent        = JITFunction_CreateArgument(result,
                                         name_start,
                                         len);
		if
			unlikely(!ent)
		goto err_r;
		result->jf_selfarg = (size_t)(ent - result->jf_args.ot_list);
	}

	/* Scan the function body for keywords and search the chain of
	 * object tables provided by the parent for those keywords when
	 * interpreted as identifiers. - Any matching identifier should
	 * then be copied into our function's reference table. */
	JITLexer_Start(&lex,
	               (unsigned char *)source_start,
	               (unsigned char *)source_end);
	lex.jl_scandata.jl_function = result;
	lex.jl_scandata.jl_parobtab = parent_object_table;
	lex.jl_scandata.jl_flags    = JIT_SCANDATA_FNORMAL;
	/* Scan the source code of the function for yield statements, as
	 * well as references to symbols found outside of the function. */
	while (lex.jl_tok) {
		unsigned char *stmt_start;
		stmt_start = lex.jl_tokstart;
		if (result->jf_flags & JIT_FUNCTION_FRETEXPR) {
			JITLexer_ScanExpression(&lex, true);
		} else {
			JITLexer_ScanStatement(&lex);
		}
		/* Check for scanning errors. */
		if (lex.jl_scandata.jl_flags & JIT_SCANDATA_FERROR)
			goto err_r;
		/* Force-advance to the next token if we're hung up on the current one. */
		if (stmt_start == lex.jl_tokstart) {
#if 0
			DeeError_Throwf(&DeeError_SyntaxError,
			                "Failed to scan token `%$s'",
			                (size_t)(lex.jl_tokend - lex.jl_tokstart),
			                lex.jl_tokstart);
			goto err_r;
#else
			JITLexer_Yield(&lex);
#endif
		}
	}
done:
	return (DREF DeeObject *)result;
err_r:
	Dee_DecrefDokill(result);
	return NULL;
}


PRIVATE void DCALL
jf_fini(JITFunction *__restrict self) {
	Dee_Decref(self->jf_source);
	Dee_XDecref(self->jf_impbase);
	Dee_XDecref(self->jf_globals);
	JITObjectTable_Fini(&self->jf_args);
	JITObjectTable_Fini(&self->jf_refs);
	Dee_Free(self->jf_argv);
}

PRIVATE void DCALL
jf_visit(JITFunction *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	Dee_Visit(self->jf_source);
	Dee_XVisit(self->jf_impbase);
	Dee_XVisit(self->jf_globals);
	if (self->jf_args.ot_list != jit_empty_object_list) {
		for (i = 0; i <= self->jf_args.ot_mask; ++i) {
			if (!ITER_ISOK(self->jf_args.ot_list[i].oe_namestr))
				continue;
			Dee_XVisit(self->jf_args.ot_list[i].oe_value);
		}
	}
	if (self->jf_refs.ot_list != jit_empty_object_list) {
		for (i = 0; i <= self->jf_refs.ot_mask; ++i) {
			if (!ITER_ISOK(self->jf_refs.ot_list[i].oe_namestr))
				continue;
			Dee_XVisit(self->jf_refs.ot_list[i].oe_value);
		}
	}
}


PRIVATE DREF DeeObject *DCALL
jf_str(JITFunction *__restrict self) {
	struct jit_object_entry *ent;
	if (self->jf_selfarg == (size_t)-1)
		return DeeString_New("<anonymous>");
	ent = &self->jf_args.ot_list[self->jf_selfarg];
	return DeeString_NewUtf8((char const *)ent->oe_namestr,
	                         ent->oe_namelen,
	                         STRING_ERROR_FSTRICT);
}


PRIVATE DREF DeeObject *DCALL
jf_repr(JITFunction *__restrict self) {
	size_t i;
	struct jit_object_entry *ent;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if (self->jf_selfarg == (size_t)-1) {
		if
			unlikely(UNICODE_PRINTER_PRINT(&printer, "[]") < 0)
		goto err;
		if (!self->jf_argc_max)
			goto do_print_code;
	} else {
		ent = &self->jf_args.ot_list[self->jf_selfarg];
		if
			unlikely(UNICODE_PRINTER_PRINT(&printer, "function ") < 0)
		goto err;
		if
			unlikely(unicode_printer_print(&printer, ent->oe_namestr, ent->oe_namelen) < 0)
		goto err;
	}
	if
		unlikely(unicode_printer_putc(&printer, '(') < 0)
	goto err;
	for (i = 0; i < self->jf_argc_max; ++i) {
		if (i != 0 && unicode_printer_putascii(&printer, ','))
			goto err;
		ent = &self->jf_args.ot_list[self->jf_argv[i]];
		if
			unlikely(unicode_printer_print(&printer,
			                               (char const *)ent->oe_namestr,
			                               ent->oe_namelen) < 0)
		goto err;
		if (i >= self->jf_argc_min) {
			if (!ent->oe_value) {
				if (unicode_printer_putascii(&printer, '?'))
					goto err;
			} else {
				if
					unlikely(unicode_printer_printf(&printer, " = %r", ent->oe_value) < 0)
				goto err;
			}
		}
	}
	if (self->jf_varargs != (size_t)-1) {
		if (self->jf_argc_max != 0 && unicode_printer_putascii(&printer, ','))
			goto err;
		ent = &self->jf_args.ot_list[self->jf_varargs];
		if
			unlikely(unicode_printer_print(&printer,
			                               (char const *)ent->oe_namestr,
			                               ent->oe_namelen) < 0)
		goto err;
		if
			unlikely(UNICODE_PRINTER_PRINT(&printer, "...") < 0)
		goto err;
	}
	if (self->jf_varkwds != (size_t)-1) {
		if ((self->jf_argc_max != 0 || self->jf_varargs != (size_t)-1) &&
		    unicode_printer_putascii(&printer, ','))
			goto err;
		ent = &self->jf_args.ot_list[self->jf_varkwds];
		if
			unlikely(UNICODE_PRINTER_PRINT(&printer, "**") < 0)
		goto err;
		if
			unlikely(unicode_printer_print(&printer,
			                               (char const *)ent->oe_namestr,
			                               ent->oe_namelen) < 0)
		goto err;
	}
	if
		unlikely(unicode_printer_putc(&printer, ')') < 0)
	goto err;
do_print_code:
	if (self->jf_flags & JIT_FUNCTION_FRETEXPR) {
		if
			unlikely(UNICODE_PRINTER_PRINT(&printer, " -> ") < 0)
		goto err;
		if
			unlikely(unicode_printer_print(&printer, self->jf_source_start,
			                               (size_t)(self->jf_source_end - self->jf_source_start)) < 0)
		goto err;
	} else {
		if
			unlikely(UNICODE_PRINTER_PRINT(&printer, " {\n\t") < 0)
		goto err;
		if
			unlikely(unicode_printer_print(&printer, self->jf_source_start,
			                               (size_t)(self->jf_source_end - self->jf_source_start)) < 0)
		goto err;
		if
			unlikely(UNICODE_PRINTER_PRINT(&printer, "\n}") < 0)
		goto err;
	}
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
jf_call_kw(JITFunction *__restrict self, size_t argc,
           DeeObject **__restrict argv, DeeObject *kw) {
	DREF DeeObject *result;
	JITLexer lexer;
	JITContext context;
	JITObjectTable base_locals;
	DeeThreadObject *ts;
	size_t i;
	if ((self->jf_flags & (JIT_FUNCTION_FYIELDING | JIT_FUNCTION_FRETEXPR)) ==
	    JIT_FUNCTION_FYIELDING) {
		DREF JITYieldFunctionObject *result;
		/* Yield function support */
		result = (DREF JITYieldFunctionObject *)DeeObject_Malloc(offsetof(JITYieldFunctionObject, jy_argv) +
		                                                         (argc * sizeof(DREF DeeObject *)));
		if
			unlikely(!result)
		goto err;
		result->jy_func = self;
		result->jy_kw   = kw;
		result->jy_argc = argc;
		Dee_Incref(self);
		Dee_XIncref(kw);
		for (i = 0; i < argc; ++i) {
			result->jy_argv[i] = argv[i];
			Dee_Incref(argv[i]);
		}
		DeeObject_Init(result, &JITYieldFunction_Type);
		return (DREF DeeObject *)result;
	}
	ts = DeeThread_Self();
	if (DeeThread_CheckInterrupt())
		goto err;
	ASSERT(self->jf_args.ot_prev.otp_ind >= 2);
	ASSERT(self->jf_args.ot_prev.otp_tab == &self->jf_refs);
	memcpy(&base_locals, &self->jf_args, sizeof(JITObjectTable));
	ASSERT(base_locals.ot_prev.otp_tab != NULL);
	base_locals.ot_list = (struct jit_object_entry *)Dee_Malloc((base_locals.ot_mask + 1) *
	                                                            sizeof(struct jit_object_entry));
	if
		unlikely(!base_locals.ot_list)
	goto err;
	memcpy(base_locals.ot_list, self->jf_args.ot_list,
	       (base_locals.ot_mask + 1) * sizeof(struct jit_object_entry));

	/* Define the self-argument.
	 * NOTE: Do this before loading arguments, in case one of the arguments
	 *       uses the same name as the function, in which case that argument
	 *       must be loaded, rather than the function loading itself! */
	if (self->jf_selfarg != (size_t)-1) {
		ASSERT(base_locals.ot_list[self->jf_selfarg].oe_value == NULL);
		base_locals.ot_list[self->jf_selfarg].oe_value = (DREF DeeObject *)self;
	}

	if (kw) {
		/* TODO: Load arguments! */
		(void)argc;
		(void)argv;
		(void)kw;
		/* Assign references to all objects from base-locals */
		for (i = 0; i <= base_locals.ot_mask; ++i) {
			if (!ITER_ISOK(base_locals.ot_list[i].oe_namestr))
				continue;
			Dee_XIncref(base_locals.ot_list[i].oe_value);
		}
	} else {
		size_t i;
		/* Load arguments! */
		if (argc < self->jf_argc_min)
			goto err_argc;
		if (argc > self->jf_argc_max && self->jf_varargs == (size_t)-1)
			goto err_argc;
		/* Load positional arguments. */
		for (i = 0; i < argc; ++i)
			base_locals.ot_list[self->jf_argv[i]].oe_value = argv[i];
		/* Assign references to all objects from base-locals */
		for (i = 0; i <= base_locals.ot_mask; ++i) {
			if (!ITER_ISOK(base_locals.ot_list[i].oe_namestr))
				continue;
			Dee_XIncref(base_locals.ot_list[i].oe_value);
		}
	}

	/* Initialize the lexer and context control structures. */
	lexer.jl_text    = self->jf_source;
	lexer.jl_context = &context;
	JITLValue_Init(&lexer.jl_lvalue);
	context.jc_impbase        = self->jf_impbase;
	context.jc_globals        = self->jf_globals;
	context.jc_retval         = JITCONTEXT_RETVAL_UNSET;
	context.jc_locals.otp_ind = 1;
	context.jc_locals.otp_tab = &base_locals;
	context.jc_except         = ts->t_exceptsz;
	context.jc_flags          = JITCONTEXT_FNORMAL;
	JITLexer_Start(&lexer,
	               (unsigned char *)self->jf_source_start,
	               (unsigned char *)self->jf_source_end);
	if (self->jf_flags & JIT_FUNCTION_FRETEXPR) {
		result = JITLexer_EvalExpression(&lexer, JITLEXER_EVAL_FNORMAL);
		if (result == JIT_LVALUE) {
			result = JITLValue_GetValue(&lexer.jl_lvalue,
			                            &context);
			JITLValue_Fini(&lexer.jl_lvalue);
		}
		if
			likely(result)
		{
			ASSERT(context.jc_retval == JITCONTEXT_RETVAL_UNSET);
			if
				unlikely(lexer.jl_tok != TOK_EOF)
			{
				DeeError_Throwf(&DeeError_SyntaxError,
				                "Expected EOF but got `%$s'",
				                (size_t)(lexer.jl_end - lexer.jl_tokstart),
				                lexer.jl_tokstart);
				lexer.jl_errpos = lexer.jl_tokstart;
				Dee_Clear(result);
				goto handle_error;
			}
		} else load_return_value: if (context.jc_retval != JITCONTEXT_RETVAL_UNSET) {
			if (JITCONTEXT_RETVAL_ISSET(context.jc_retval)) {
				result = context.jc_retval;
			} else {
				/* Exited code via unconventional means, such as `break' or `continue' */
				DeeError_Throwf(&DeeError_SyntaxError,
				                "Attempted to use `break' or `continue' outside of a loop");
				lexer.jl_errpos = lexer.jl_tokstart;
				goto handle_error;
			}
		} else {
			if (!lexer.jl_errpos)
				lexer.jl_errpos = lexer.jl_tokstart;
handle_error:
			JITLValue_Fini(&lexer.jl_lvalue);
			result = NULL;
			/* TODO: Somehow remember that the error happened at `lexer.jl_errpos' */
			;
		}
	} else if unlikely(lexer.jl_tok == TOK_EOF) {
		goto do_return_none;
	} else {
		do {
			result = JITLexer_EvalStatement(&lexer);
			if (ITER_ISOK(result)) {
				Dee_Decref_unlikely(result);
				continue;
			}
			if (result == JIT_LVALUE) {
				JITLValue_Fini(&lexer.jl_lvalue);
				JITLValue_Init(&lexer.jl_lvalue);
				continue;
			}
			/* Error, or return encountered. */
			goto load_return_value;
		} while (lexer.jl_tok != TOK_EOF);
do_return_none:
		result = Dee_None;
		Dee_Incref(Dee_None);
	}
	if (ts->t_exceptsz > context.jc_except) {
		if (context.jc_retval != JITCONTEXT_RETVAL_UNSET) {
			if (JITCONTEXT_RETVAL_ISSET(context.jc_retval))
				Dee_Decref(context.jc_retval);
			context.jc_retval = JITCONTEXT_RETVAL_UNSET;
		}
		Dee_XClear(result);
		while (ts->t_exceptsz > context.jc_except + 1) {
			DeeError_Print("Discarding secondary error\n",
			               ERROR_PRINT_DOHANDLE);
		}
	}
	ASSERT(!self->jf_globals || context.jc_globals == self->jf_globals);
	if (context.jc_globals != self->jf_globals)
		Dee_Decref(context.jc_globals);
	JITObjectTable_Fini(&base_locals);
	return result;
err_base_locals:
	Dee_Free(base_locals.ot_list);
err:
	return NULL;
err_argc:
	if (self->jf_selfarg == (size_t)-1) {
		err_invalid_argc_len(NULL,
		                     0,
		                     argc,
		                     self->jf_argc_min,
		                     self->jf_argc_max);
	} else {
		struct jit_object_entry *ent;
		ent = &self->jf_args.ot_list[self->jf_selfarg];
		err_invalid_argc_len((char const *)ent->oe_namestr,
		                     ent->oe_namelen,
		                     argc,
		                     self->jf_argc_min,
		                     self->jf_argc_max);
	}
	goto err_base_locals;
}


PRIVATE int DCALL
compare_objtabs(JITObjectTable *__restrict a,
                JITObjectTable *__restrict b) {
	size_t i;
	int temp;
	for (i = 0; i <= a->ot_mask; ++i) {
		if (!ITER_ISOK(a->ot_list[i].oe_namestr)) {
			if (b->ot_list[i].oe_namestr != a->ot_list[i].oe_namestr)
				goto nope;
		} else {
			if (!ITER_ISOK(b->ot_list[i].oe_namestr))
				goto nope;
			if (a->ot_list[i].oe_namehsh != b->ot_list[i].oe_namehsh)
				goto nope;
			if (a->ot_list[i].oe_namelen != b->ot_list[i].oe_namelen)
				goto nope;
			if (memcmp(a->ot_list[i].oe_namestr,
			           b->ot_list[i].oe_namestr,
			           a->ot_list[i].oe_namelen * sizeof(char)) != 0)
				goto nope;
			if (a->ot_list[i].oe_value) {
				if (!b->ot_list[i].oe_value)
					goto nope;
				if (a->ot_list[i].oe_value != b->ot_list[i].oe_value) {
					temp = DeeObject_CompareEq(a->ot_list[i].oe_value,
					                           b->ot_list[i].oe_value);
					if
						unlikely(temp <= 0)
					goto err_temp;
				}
			} else {
				if (b->ot_list[i].oe_value)
					goto nope;
			}
		}
	}
	return 1;
nope:
	return 0;
err_temp:
	return temp;
}

PRIVATE int DCALL
jf_equal(JITFunction *__restrict a,
         JITFunction *__restrict b) {
	int temp;
	if (a == b)
		goto yes;
	if (a->jf_selfarg != b->jf_selfarg)
		goto nope;
	if (a->jf_varargs != b->jf_varargs)
		goto nope;
	if (a->jf_varkwds != b->jf_varkwds)
		goto nope;
	if (a->jf_argc_min != b->jf_argc_min)
		goto nope;
	if (a->jf_argc_max != b->jf_argc_max)
		goto nope;
	if ((a->jf_source_end - a->jf_source_start) !=
	    (b->jf_source_end - b->jf_source_start))
		goto nope;
	if (memcmp(a->jf_source_start, b->jf_source_start,
	           (size_t)(a->jf_source_end - a->jf_source_start)) != 0)
		goto nope;
	if (a->jf_flags != b->jf_flags)
		goto nope;
	if (memcmp(a->jf_argv, b->jf_argv, a->jf_argc_max * sizeof(size_t)) != 0)
		goto nope;
	if (a->jf_args.ot_mask != b->jf_args.ot_mask)
		goto nope;
	if (a->jf_args.ot_size != b->jf_args.ot_size)
		goto nope;
	if (a->jf_args.ot_used != b->jf_args.ot_used)
		goto nope;
	if (a->jf_refs.ot_mask != b->jf_refs.ot_mask)
		goto nope;
	if (a->jf_refs.ot_size != b->jf_refs.ot_size)
		goto nope;
	if (a->jf_refs.ot_used != b->jf_refs.ot_used)
		goto nope;
	if (a->jf_globals != b->jf_globals) {
		if (!a->jf_globals || !b->jf_globals)
			goto nope;
		temp = DeeObject_CompareEq(a->jf_globals,
		                           b->jf_globals);
		if (temp <= 0)
			goto err_temp;
	}
	if (a->jf_impbase != b->jf_impbase) {
		if (!a->jf_impbase || !b->jf_impbase)
			goto nope;
		temp = DeeObject_CompareEq((DeeObject *)a->jf_impbase,
		                           (DeeObject *)b->jf_impbase);
		if (temp <= 0)
			goto err_temp;
	}
	temp = compare_objtabs(&a->jf_args, &b->jf_args);
	if (temp <= 0)
		goto err_temp;
	temp = compare_objtabs(&a->jf_refs, &b->jf_refs);
	if (temp <= 0)
		goto err_temp;
yes:
	return 1;
nope:
	return 0;
err_temp:
	return temp;
}

PRIVATE dhash_t DCALL
jf_hash(JITFunction *__restrict self) {
	(void)self;
	/* TODO */
	return 0;
}

PRIVATE DREF DeeObject *DCALL
jf_eq(JITFunction *__restrict self,
      JITFunction *__restrict other) {
	int result;
	if (DeeObject_AssertTypeExact(other, &JITFunction_Type))
		goto err;
	result = jf_equal(self, other);
	if
		unlikely(result < 0)
	goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
jf_ne(JITFunction *__restrict self,
      JITFunction *__restrict other) {
	int result;
	if (DeeObject_AssertTypeExact(other, &JITFunction_Type))
		goto err;
	result = jf_equal(self, other);
	if
		unlikely(result < 0)
	goto err;
	return_bool_(!result);
err:
	return NULL;
}


PRIVATE struct type_cmp jf_cmp = {
	/* .tp_hash = */ (dhash_t(DCALL *)(DeeObject * __restrict)) & jf_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict)) & jf_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, DeeObject *__restrict)) & jf_ne
};




PRIVATE DREF DeeObject *DCALL
jf_hasvarargs(JITFunction *__restrict self) {
	return_bool_(self->jf_varargs != (size_t)-1);
}

PRIVATE DREF DeeObject *DCALL
jf_hasvarkwds(JITFunction *__restrict self) {
	return_bool_(self->jf_varkwds != (size_t)-1);
}

INTERN DREF DeeObject *DCALL
jf_getname(JITFunction *__restrict self) {
	struct jit_object_entry *ent;
	if (self->jf_selfarg == (size_t)-1)
		return_none;
	ent = &self->jf_args.ot_list[self->jf_selfarg];
	return DeeString_NewUtf8(ent->oe_namestr,
	                         ent->oe_namelen,
	                         STRING_ERROR_FIGNORE);
}

INTERN DREF DeeObject *DCALL
jf_getdoc(JITFunction *__restrict UNUSED(self)) {
	return_none;
}

INTERN DREF DeeObject *DCALL
jf_getkwds(JITFunction *__restrict self) {
	/* XXX: Add a generic sequence proxy type for this! */
	DREF DeeObject *result;
	uint16_t i;
	result = DeeTuple_NewUninitialized(self->jf_argc_max);
	if
		unlikely(!result)
	goto done;
	for (i = 0; i < self->jf_argc_max; ++i) {
		DREF DeeObject *name;
		struct jit_object_entry *ent;
		ent  = &self->jf_args.ot_list[self->jf_argv[i]];
		name = DeeString_NewUtf8(ent->oe_namestr,
		                         ent->oe_namelen,
		                         STRING_ERROR_FIGNORE);
		if
			unlikely(!name)
		goto err_r_i;
		DeeTuple_SET(result, i, name); /* Inherit reference. */
	}
done:
	return result;
err_r_i:
	while (i--)
		Dee_Decref_likely(DeeTuple_GET(result, i));
	DeeTuple_FreeUninitialized(result);
	return NULL;
}

INTERN DREF DeeObject *DCALL
jf_gettext(JITFunction *__restrict self) {
	return DeeString_NewUtf8(self->jf_source_start,
	                         (size_t)(self->jf_source_end - self->jf_source_start),
	                         STRING_ERROR_FIGNORE);
}

PRIVATE DREF DeeObject *DCALL
jf_isyielding(JITFunction *__restrict self) {
	return_bool(self->jf_flags & JIT_FUNCTION_FYIELDING);
}

PRIVATE DREF DeeObject *DCALL
jf_getisretexpr(JITFunction *__restrict self) {
	return_bool(self->jf_flags & JIT_FUNCTION_FRETEXPR);
}


PRIVATE struct type_getset jf_getsets[] = {
	{ "__name__",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & jf_getname, NULL, NULL,
	  DOC("->?X2?Dstring?N") },
	{ "__doc__",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & jf_getdoc, NULL, NULL,
	  DOC("->?N\nAlways returns :none (doc strings aren't processed in JIT code)") },
	{ "__kwds__",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & jf_getkwds, NULL, NULL,
	  DOC("->?S?Dstring") },
	{ "__text__",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & jf_gettext, NULL, NULL,
	  DOC("->?Dstring\n"
	      "Returns the source text executed by @this function") },
	{ "isyielding",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & jf_isyielding, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Check if @this function behaves as yielding (i.e. contains a yield statment "
	      "that doesn't appear as part of a recursively defined inner function)") },
	{ "isretexpr",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & jf_getisretexpr, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Evaluates to :true if @this function was defined like ${[] -> 42}, meaning "
	      "that #__text__ is merely the expression that should be returned by the function\n"
	      "When :false, the function was defined like ${[] { return 42; }}") },
	/* TODO: __default__ */
	/* TODO: __refs__ */
	/* TODO: __type__ */
	/* TODO: __operator__ */
	/* TODO: __operatorname__ */
	/* TODO: __property__ */
	/* etc... */
	{ "hasvarargs",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & jf_hasvarargs, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Check if @this function accepts variable arguments as overflow") },
	{ "hasvarkwds",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & jf_hasvarkwds, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Check if @this function accepts variable keyword arguments as overflow") },
	{ NULL }
};

PRIVATE struct type_member jf_members[] = {
	TYPE_MEMBER_FIELD_DOC("__impbase__", STRUCT_OBJECT_OPT, offsetof(JITFunction, jf_impbase),
	                      "->?X2?DModule?N\n"
	                      "Returns the module used for relative module imports"),
	TYPE_MEMBER_FIELD_DOC("__globals__", STRUCT_OBJECT_OPT, offsetof(JITFunction, jf_globals),
	                      "->?X2?S?T2?Dstring?O?N"),
	TYPE_MEMBER_FIELD_DOC("__module__", STRUCT_OBJECT_OPT, offsetof(JITFunction, jf_impbase),
	                      "->?X2?DModule?N\n"
	                      "Alias for #__impbase__"),
	TYPE_MEMBER_FIELD_DOC("__source__", STRUCT_OBJECT, offsetof(JITFunction, jf_source),
	                      "->?X3?Dstring?DBytes?O\n"
	                      "Returns the object that owns the source code executed by @this function (s.a. #__text__)"),
	TYPE_MEMBER_FIELD_DOC("__argc_min__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(JITFunction, jf_argc_min),
	                      "Min amount of arguments required to execute @this function"),
	TYPE_MEMBER_FIELD_DOC("__argc_max__", STRUCT_CONST | STRUCT_UINT16_T, offsetof(JITFunction, jf_argc_max),
	                      "Max amount of arguments accepted by @this function (excluding a varargs or varkwds argument)"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject JITFunction_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_JitFunction",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(JITFunction)
			}
		},
		/* .tp_dtor        = */ (void(DCALL *)(DeeObject *__restrict))&jf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jf_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&jf_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&jf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &jf_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ jf_getsets,
	/* .tp_members       = */ jf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject **__restrict, DeeObject *))&jf_call_kw,
};

DECL_END

#endif /* !GUARD_DEX_JIT_FUNCTION_C */
