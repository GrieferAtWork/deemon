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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_MULTIPLE_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_MULTIPLE_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>             /* Dee_Free, Dee_Reallocc */
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/dict.h>              /* DeeDict_New */
#include <deemon/error.h>             /* DeeError_Handled, ERROR_HANDLED_RESTORE */
#include <deemon/hashset.h>           /* DeeHashSet_Insert, DeeHashSet_New */
#include <deemon/list.h>              /* DeeListObject, DeeList_* */
#include <deemon/map.h>               /* DeeMapping_NewEmpty */
#include <deemon/none.h>              /* DeeNone_NewRef */
#include <deemon/object.h>            /* DREF, DeeObject, DeeObject_SetItem, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_Decrefv, Dee_Incref */
#include <deemon/pair.h>              /* CONFIG_ENABLE_SEQ_ONE_TYPE, CONFIG_ENABLE_SEQ_PAIR_TYPE, DeeSeq_OfOne, DeeSeq_OfPair */
#include <deemon/rodict.h>            /* Dee_rodict_builder* */
#include <deemon/seq.h>               /* DeeSeq_* */
#include <deemon/set.h>               /* DeeSet_NewEmpty, DeeSet_Type */
#include <deemon/system-features.h>   /* memcpy*, memmovedownc, memmoveupc */
#include <deemon/tuple.h>             /* DeeTuple* */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */

DECL_BEGIN

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_optimize_multiple)(struct ast_optimize_stack *__restrict stack,
                              struct ast *__restrict self, bool result_used) {
	struct ast **iter, **end;
	bool is_unreachable, only_constexpr;
	ASSERT(self->a_type == AST_MULTIPLE);
	if (!result_used &&
	    self->a_flag != AST_FMULTIPLE_KEEPLAST) {
		self->a_flag = AST_FMULTIPLE_KEEPLAST;
		OPTIMIZE_VERBOSE("Replace unused sequence expression with keep-last multi-branch");
		++optimizer_count;
	}

	/* Optimize all asts within.
	 * In the event of a keep-last AST, delete a branches
	 * without side-effects except for the last. */
	end = (iter = self->a_multiple.m_astv) +
	      self->a_multiple.m_astc;
	is_unreachable = false;
	only_constexpr = true;
multiple_continue_at_iter:
	for (; iter < end; ++iter) {
		bool using_value = result_used;
		int temp;
		if (self->a_flag == AST_FMULTIPLE_KEEPLAST && iter != end - 1)
			using_value = false;
		if (ast_optimize(stack, *iter, using_value))
			goto err;
		if ((*iter)->a_type != AST_CONSTEXPR)
			only_constexpr = false;
		temp = ast_doesnt_return(*iter, AST_DOESNT_RETURN_FNORMAL);
		if (temp < 0) {
			is_unreachable = (temp == -2);
		} else if (is_unreachable ||
		           /* Delete branches that are unreachable or have no side-effects. */
		           (self->a_flag == AST_FMULTIPLE_KEEPLAST &&
		            (iter != end - 1 || !result_used) &&
		            !ast_has_sideeffects(*iter))) {
			/* Get rid of this one. */
			OPTIMIZE_VERBOSEAT(*iter, is_unreachable ? "Delete unreachable AST\n"
			                                         : "Delete AST without side-effects\n");
			ast_decref(*iter);
			--end;
			--self->a_multiple.m_astc;
			memmovedownc(iter,
			             iter + 1,
			             (size_t)(end - iter),
			             sizeof(DREF struct ast *));
			++optimizer_count;
			goto multiple_continue_at_iter;
		} else if (temp > 0) {
			/* If the AST doesn't return, everything that follows is no-return. */
			is_unreachable = true;
		}
	}
	if (only_constexpr) {
		if (self->a_multiple.m_astc == 1 &&
		    !self->a_ddi.l_file) {
			memcpy(&self->a_ddi,
			       &self->a_multiple.m_astv[0]->a_ddi,
			       sizeof(struct ast_loc));
			if (self->a_ddi.l_file)
				TPPFile_Incref(self->a_ddi.l_file);
		}
		if (self->a_multiple.m_astc == 0) {
			DREF DeeObject *cexpr;
			switch (self->a_flag) {

			case AST_FMULTIPLE_KEEPLAST:
				cexpr = DeeNone_NewRef();
				break;

			case AST_FMULTIPLE_TUPLE:
				cexpr = DeeTuple_NewEmpty();
				break;

			case AST_FMULTIPLE_GENERIC:
				cexpr = DeeSeq_NewEmpty();
				break;

			case AST_FMULTIPLE_GENERIC_SET:
				cexpr = DeeSet_NewEmpty();
				break;

			case AST_FMULTIPLE_GENERIC_MAP:
				cexpr = DeeMapping_NewEmpty();
				break;

			case AST_FMULTIPLE_LIST:
				cexpr = DeeList_New();
				if unlikely(!cexpr)
					goto err;
				break;

			case AST_FMULTIPLE_HASHSET:
				cexpr = DeeHashSet_New();
				if unlikely(!cexpr)
					goto err;
				break;

			case AST_FMULTIPLE_DICT:
				cexpr = DeeDict_New();
				if unlikely(!cexpr)
					goto err;
				break;

			default:
				goto after_multiple_constexpr;
			}
			Dee_Free(self->a_multiple.m_astv);
			self->a_constexpr = cexpr; /* Inherit reference */
		} else if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
			if unlikely(ast_graft_onto(self, self->a_multiple.m_astv[self->a_multiple.m_astc - 1]))
				goto err;
#ifdef CONFIG_ENABLE_SEQ_ONE_TYPE
		} else if (self->a_flag == AST_FMULTIPLE_GENERIC && self->a_multiple.m_astc == 1) {
			DREF DeeObject *seqone;
			seqone = DeeSeq_OfOne(self->a_multiple.m_astv[0]->a_constexpr);
			if unlikely(!seqone)
				goto err;
			ast_decref(self->a_multiple.m_astv[0]);
			Dee_Free(self->a_multiple.m_astv);
			self->a_constexpr = seqone; /* Inherit reference. */
#endif /* CONFIG_ENABLE_SEQ_ONE_TYPE */
#ifdef CONFIG_ENABLE_SEQ_PAIR_TYPE
		} else if (self->a_flag == AST_FMULTIPLE_GENERIC && self->a_multiple.m_astc == 2) {
			DREF DeeObject *pair;
			pair = DeeSeq_OfPair(self->a_multiple.m_astv[0]->a_constexpr,
			                      self->a_multiple.m_astv[1]->a_constexpr);
			if unlikely(!pair)
				goto err;
			ast_decref(self->a_multiple.m_astv[1]);
			ast_decref(self->a_multiple.m_astv[0]);
			Dee_Free(self->a_multiple.m_astv);
			self->a_constexpr = pair; /* Inherit reference. */
#endif /* CONFIG_ENABLE_SEQ_PAIR_TYPE */
		} else if (self->a_flag == AST_FMULTIPLE_TUPLE ||
		           self->a_flag == AST_FMULTIPLE_GENERIC) {
			DREF DeeTupleObject *new_tuple;
			size_t i;
			new_tuple = DeeTuple_NewUninitialized(self->a_multiple.m_astc);
			if unlikely(!new_tuple)
				goto err;
			for (i = 0; i < self->a_multiple.m_astc; ++i) {
				struct ast *branch = self->a_multiple.m_astv[i];
				DeeObject *value   = branch->a_constexpr;
				DeeTuple_SET(new_tuple, i, value);
				Dee_Incref(value);
				ast_decref(branch);
			}
			Dee_Free(self->a_multiple.m_astv);
			self->a_constexpr = Dee_AsObject(new_tuple); /* Inherit reference. */
		} else if (self->a_flag == AST_FMULTIPLE_LIST) {
			DREF DeeListObject *new_list;
			size_t i;
			new_list = DeeList_NewUninitialized(self->a_multiple.m_astc);
			if unlikely(!new_list)
				goto err;
			for (i = 0; i < self->a_multiple.m_astc; ++i) {
				struct ast *branch = self->a_multiple.m_astv[i];
				DeeObject *value   = branch->a_constexpr;
				DeeList_SET(new_list, i, value);
				Dee_Incref(value);
				ast_decref(branch);
			}
			Dee_Free(self->a_multiple.m_astv);
			self->a_constexpr = DeeList_FinalizeUninitialized(new_list); /* Inherit reference. */
		} else if (self->a_flag == AST_FMULTIPLE_HASHSET) {
			DREF DeeObject *new_set;
			size_t i;
			new_set = DeeHashSet_New();
			if unlikely(!new_set)
				goto err;
			for (i = 0; i < self->a_multiple.m_astc; ++i) {
				if (DeeHashSet_Insert(new_set, self->a_multiple.m_astv[i]->a_constexpr) < 0) {
					Dee_Decref(new_set);
					goto err;
				}
			}
			ast_decrefv(self->a_multiple.m_astv, self->a_multiple.m_astc);
			Dee_Free(self->a_multiple.m_astv);
			self->a_constexpr = new_set;
		} else if (self->a_flag == AST_FMULTIPLE_DICT) {
			DREF DeeObject *new_dict;
			size_t i;
			new_dict = DeeDict_New();
			if unlikely(!new_dict)
				goto err;
			for (i = 0; i < self->a_multiple.m_astc / 2; ++i) {
				DeeObject *key  = self->a_multiple.m_astv[(i * 2) + 0]->a_constexpr;
				DeeObject *item = self->a_multiple.m_astv[(i * 2) + 1]->a_constexpr;
				if (DeeObject_SetItem(new_dict, key, item)) {
					Dee_Decref(new_dict);
					goto err;
				}
			}
			ast_decrefv(self->a_multiple.m_astv, self->a_multiple.m_astc);
			Dee_Free(self->a_multiple.m_astv);
			self->a_constexpr = new_dict; /* Inherit reference. */
		} else if (self->a_flag == AST_FMULTIPLE_GENERIC_MAP) {
			struct Dee_rodict_builder new_dict;
			size_t i, length = self->a_multiple.m_astc / 2;
			Dee_rodict_builder_init_with_hint(&new_dict, length);
			for (i = 0; i < length; ++i) {
				DeeObject *key  = self->a_multiple.m_astv[(i * 2) + 0]->a_constexpr;
				DeeObject *item = self->a_multiple.m_astv[(i * 2) + 1]->a_constexpr;
				if (Dee_rodict_builder_setitem(&new_dict, key, item)) {
					Dee_rodict_builder_fini(&new_dict);
					goto err;
				}
			}
			ast_decrefv(self->a_multiple.m_astv, self->a_multiple.m_astc);
			Dee_Free(self->a_multiple.m_astv);
			self->a_constexpr = Dee_AsObject(Dee_rodict_builder_pack(&new_dict)); /* Inherit reference. */
		} else {
			goto after_multiple_constexpr;
		}
		self->a_flag = AST_FNORMAL;
		self->a_type = AST_CONSTEXPR;
		OPTIMIZE_VERBOSE("Reduce constant-expression multi-ast to %r\n",
		                 self->a_constexpr);
		goto did_optimize;
	}
after_multiple_constexpr:
	ASSERT(self->a_type == AST_MULTIPLE);
	if (self->a_flag == AST_FMULTIPLE_KEEPLAST) {
		switch (self->a_multiple.m_astc) {

		case 1:
			if (self->a_scope == self->a_multiple.m_astv[0]->a_scope) {
				/* We can simply inherit this single branch, simplifying the entire AST. */
				if (ast_assign(self, self->a_multiple.m_astv[0]))
					goto err;
				OPTIMIZE_VERBOSE("Collapse single-branch multi-ast\n");
				goto did_optimize;
			}
			break;

		case 0:
			/* Convert this branch into `none' */
			Dee_Free(self->a_multiple.m_astv);
			self->a_type      = AST_CONSTEXPR;
			self->a_flag      = AST_FNORMAL;
			self->a_constexpr = DeeNone_NewRef();
			OPTIMIZE_VERBOSE("Replace empty multi-ast with `none'\n");
			goto did_optimize;

		default:
			break;
		}
	} else {
		if (self->a_multiple.m_astc == 1 &&
		    self->a_multiple.m_astv[0]->a_type == AST_EXPAND) {
			/* Something like `{ x... }' can be optimized to `x' when
			 * it is already  known that `x' has some sequence typing.
			 *
			 * TODO: This can only be done when "type(x)" doesn't re-
			 *       implement any sequence function using some custom
			 *       override (e.g. "string" defines a Sequence-incompatible
			 *       "find" function and thus defines "__seq_find__" as
			 *       a compatible alternative; same goes for "Bytes")
			 *
			 * Generally, if the type defines "__seq_*" member, or has
			 * a __seq_class__ that differs from "Sequence", then the
			 * cast-to-sequence cannot be optimized away. */
			struct ast *expanded_expr;
			DeeTypeObject *expanded_type;
			DeeTypeObject *needed_type;
			if (self->a_flag == AST_FMULTIPLE_TUPLE) {
				needed_type = &DeeTuple_Type;
			} else if (self->a_flag == AST_FMULTIPLE_GENERIC) {
				needed_type = &DeeSeq_Type;
			} else if (self->a_flag == AST_FMULTIPLE_GENERIC_SET) {
				needed_type = &DeeSet_Type;
			} else {
				goto done_seq_cast_optimization;
			}
			expanded_expr = self->a_multiple.m_astv[0]->a_expand;
			expanded_type = ast_predict_type(expanded_expr);
			/* TODO: When "needed_type == &DeeSeq_Type", also accept all
			 *       sequence-compatible proxy types (e.g. StringSplit) */
			if (expanded_type && expanded_type == needed_type) {
				if (ast_assign(self, expanded_expr))
					goto err;
				OPTIMIZE_VERBOSE("Replace `{ x... }' with `x' of type %k\n",
				                 expanded_type);
				goto did_optimize;
			}
		}
done_seq_cast_optimization:

		/* Try to optimize something like `[10, [x, y]...]' to `[10, x, y]' */
		iter = self->a_multiple.m_astv;
		end  = iter + self->a_multiple.m_astc;
continue_inline_at_iter:
		for (; iter < end; ++iter) {
			struct ast *inner;
			if ((*iter)->a_type != AST_EXPAND)
				continue;
			inner = (*iter)->a_expand;
			if (inner->a_type == AST_MULTIPLE &&
			    inner->a_flag != AST_FMULTIPLE_KEEPLAST) {
				DREF struct ast **new_astv;
				struct ast *expand = *iter;
				size_t move_count;
				OPTIMIZE_VERBOSEAT(inner, "Inline expanded multi-branch into containing sequence\n");
				++optimizer_count;
				/* Simply inline all of the expanded sequence expressions. */
				if (!inner->a_multiple.m_astc) {
					ast_decref(expand);
					--end;
					memmovedownc(iter,
					             iter + 1,
					             (size_t)(end - iter),
					             sizeof(DREF struct ast *));
					--self->a_multiple.m_astc;
					goto continue_inline_at_iter;
				}
				new_astv = (DREF struct ast **)Dee_Reallocc(self->a_multiple.m_astv,
				                                            self->a_multiple.m_astc - 1 +
				                                            inner->a_multiple.m_astc,
				                                            sizeof(DREF struct ast *));
				if unlikely(!new_astv)
					goto err;
				move_count = (size_t)((end - iter) - 1);
				iter = new_astv + (iter - self->a_multiple.m_astv);
				self->a_multiple.m_astv = new_astv;
				self->a_multiple.m_astc += inner->a_multiple.m_astc - 1;
				end = new_astv + self->a_multiple.m_astc;
				memmoveupc(iter + inner->a_multiple.m_astc,
				           iter + 1, move_count,
				           sizeof(struct ast *));
				memcpyc(iter,
				        inner->a_multiple.m_astv,
				        inner->a_multiple.m_astc,
				        sizeof(struct ast *));
				Dee_Free(inner->a_multiple.m_astv);
				inner->a_multiple.m_astc = 0;
				inner->a_multiple.m_astv = NULL;
				ast_decref(expand);
				goto continue_inline_at_iter;
			} else if (inner->a_type == AST_CONSTEXPR) {
				DREF struct ast **new_astv;
				DREF DeeObject **vec;
				size_t len, i;
				struct ast *expand = *iter;
				size_t move_count;
				vec = DeeSeq_AsHeapVector(inner->a_constexpr, &len);
				if unlikely(!vec) {
					DeeError_Handled(ERROR_HANDLED_RESTORE);
					continue;
				}
				OPTIMIZE_VERBOSEAT(inner, "Inline expanded constant-branch into containing sequence\n");
				++optimizer_count;
				if (!len) {
					ast_decref(expand);
					--end;
					memmovedownc(iter, iter + 1, (size_t)(end - iter),
					             sizeof(DREF struct ast *));
					--self->a_multiple.m_astc;
					goto continue_inline_at_iter;
				}
				new_astv = (DREF struct ast **)Dee_Reallocc(self->a_multiple.m_astv,
				                                            self->a_multiple.m_astc - 1 + len,
				                                            sizeof(DREF struct ast *));
				if unlikely(!new_astv) {
err_expand_vec:
					Dee_Decrefv(vec, len);
					Dee_Free(vec);
					goto err;
				}
				move_count = (size_t)((end - iter) - 1);
				iter = new_astv + (iter - self->a_multiple.m_astv);
				self->a_multiple.m_astv = new_astv;
				self->a_multiple.m_astc += len - 1;
				end = new_astv + self->a_multiple.m_astc;
				/* NOTE: len >= 1 */
				memmoveupc(iter + len, iter + 1, move_count,
				           sizeof(DREF struct ast *));
				/* Wrap all of the sequence elements of the
				 * inner expression into constant-branches. */
				for (i = 0; i < len; ++i) {
					DREF struct ast *constant_ast;
					constant_ast = ast_setddi(ast_constexpr(vec[i]), &inner->a_ddi);
					if unlikely(!constant_ast) {
						memmoveupc(iter + i + len,
						           iter + i + 1,
						           move_count - i,
						           sizeof(DREF struct ast *));
						self->a_multiple.m_astc -= len - i;
						goto err_expand_vec;
					}
					iter[i] = constant_ast; /* Inherit reference */
				}
				Dee_Decrefv(vec, len);
				Dee_Free(vec);
				ast_decref(expand);
				goto continue_inline_at_iter;
			}
		}
	}
	return 0;
err:
	return -1;
did_optimize:
	++optimizer_count;
	return 0;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_OPT_MULTIPLE_C */
