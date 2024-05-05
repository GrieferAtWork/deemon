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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_ASSUMES_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_ASSUMES_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/error.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */

DECL_BEGIN

#ifdef OPTIMIZE_FASSUME

/* Only extend assumptions to local and stack variables:
 *   - global variables must not be optimized away, and may
 *     be modified arbitrarily by other modules, or threads.
 *   - static variables may change unpredictably after multiple
 *     executions. Also, write-once statics are optimized by
 *    `OPTIMIZE_FCONSTSYMS', so they're mostly covered already.
 *   - And other types of variables don't even qualify at all!
 */
#define SYMBOL_ALLOW_ASSUMPTIONS(sym) \
	((sym)->s_type == SYMBOL_TYPE_LOCAL || (sym)->s_type == SYMBOL_TYPE_STACK)



PRIVATE WUNUSED NONNULL((1)) int
(DCALL ast_assumes_rehashsymbol)(struct ast_assumes *__restrict self) {
	struct ast_symbol_assume *new_vector, *iter, *end;
	size_t new_mask = self->aa_syms.sa_mask;
	new_mask        = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 16 - 1;
	ASSERT(self->aa_syms.sa_size < new_mask);
	new_vector = (struct ast_symbol_assume *)Dee_Callocc(new_mask + 1, sizeof(struct ast_symbol_assume));
	if unlikely(!new_vector)
		goto err;
	if (self->aa_syms.sa_elem) {
		/* Re-insert all existing items into the new table vector. */
		end = (iter = self->aa_syms.sa_elem) + (self->aa_syms.sa_mask + 1);
		for (; iter < end; ++iter) {
			struct ast_symbol_assume *item;
			dhash_t i, perturb;
			/* Skip NULL entries. */
			if (!iter->sa_sym)
				continue;
			perturb = i = AST_SYMBOL_ASSUME_HASH(iter) & new_mask;
			for (;; AST_SYMBOL_ASSUMES_HASHNX(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->sa_sym)
					break; /* Empty slot found. */
			}
			/* Transfer this object. */
			memcpy(item, iter, sizeof(struct ast_symbol_assume));
		}
		Dee_Free(self->aa_syms.sa_elem);
	}
	self->aa_syms.sa_mask = new_mask;
	self->aa_syms.sa_elem = new_vector;
	Dee_CHECKMEMORY();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) struct ast_symbol_assume *DCALL
ast_assumes_getsymbol(struct ast_assumes *__restrict self,
                      struct symbol *__restrict sym) {
	dhash_t i, perturb, hash;
	hash = sym->s_name->k_id;
	if (!self->aa_syms.sa_elem)
		goto nope;
	i = perturb = hash & self->aa_syms.sa_mask;
	for (;; AST_SYMBOL_ASSUMES_HASHNX(i, perturb)) {
		struct ast_symbol_assume *item;
		item = AST_SYMBOL_ASSUMES_HASHIT(&self->aa_syms, i);
		if (!item->sa_sym)
			break;
		if (item->sa_sym == sym)
			return item;
	}
	Dee_CHECKMEMORY();
nope:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) struct ast_symbol_assume *DCALL
ast_assumes_newsymbol(struct ast_assumes *__restrict self,
                      struct symbol *__restrict sym) {
	dhash_t i, perturb, hash;
	hash = sym->s_name->k_id;
	if (!self->aa_syms.sa_elem) {
		if unlikely(ast_assumes_rehashsymbol(self))
			goto err;
	}
again:
	i = perturb = hash & self->aa_syms.sa_mask;
	for (;; AST_SYMBOL_ASSUMES_HASHNX(i, perturb)) {
		struct ast_symbol_assume *item;
		item = AST_SYMBOL_ASSUMES_HASHIT(&self->aa_syms, i);
		if (!item->sa_sym) {
			if (self->aa_syms.sa_size >= self->aa_syms.sa_mask) {
				if unlikely(ast_assumes_rehashsymbol(self))
					goto err;
				goto again;
			}
			item->sa_sym = sym;
			++self->aa_syms.sa_size;
			return item;
		}
		if (item->sa_sym == sym) {
			Dee_XDecref(item->sa_value);
			return item;
		}
	}
err:
	return NULL;
}

/* Add an assumption that the value of `sym' currently is set to `value'.
 * When `value' is `NULL', assume that the value of `sym' is now undefined.
 * NOTE: Depending on the type of `sym', no assumption may be made, such as
 *       in the case of external, or global variables, which may arbitrarily
 *       be modified by other threads running independently on the caller.
 * @return:  0: OK.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_assumes_setsymval)(struct ast_assumes *__restrict self,
                              struct symbol *__restrict sym,
                              DeeObject *value) {
	struct ast_symbol_assume *assumption;
	if (!SYMBOL_ALLOW_ASSUMPTIONS(sym))
		goto done;
	assumption = ast_assumes_newsymbol(self, sym);
	if unlikely(!assumption)
		goto err;
	assumption->sa_value = value;
	Dee_XIncref(value);
done:
	return 0;
err:
	return -1;
}

/* Lookup the assumed value of a given symbol `sym', and return a reference to it.
 * NOTE: When no such assumption is available, or the symbol is assumed to be
 *       unknown, `NULL' is returned, but no error is thrown. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL ast_assumes_getsymval)(struct ast_assumes *__restrict self,
                              struct symbol *__restrict sym) {
	if (!SYMBOL_ALLOW_ASSUMPTIONS(sym))
		goto done;
	do {
		struct ast_symbol_assume *assumption;
		assumption = ast_assumes_getsymbol(self, sym);
		if (assumption) {
			Dee_XIncref(assumption->sa_value);
			return assumption->sa_value;
		}
	} while ((self = (struct ast_assumes *)self->aa_prev) != NULL);
done:
	return NULL;
}


/* Update `self' to be the state of assumptions as it would be
 * if `ast' would have been optimized using those assumptions.
 * However, `ast' will not actually be optimized!
 * This is used in cases where it is necessary to determine changes
 * in assumptions made ahead of time, in order to determine which
 * assumptions will continue to hold, which have changed, etc, such
 * as in a loop:
 * >> local x = "foobar";
 * >> for (local y: get_items()) {
 * >>     // If we were to blindly optimize the loop with previous assumptions,
 * >>     // this use of `x' would get optimized into a constant `"foobar"',
 * >>     // despite the fact that `x' will be re-written further down below.
 * >>     print x;
 * >>     x = y;
 * >> }
 * In cases such as this, we will gather all assumptions made by a
 * loop ahead of time, before actually going ahead and performing
 * optimizations.
 * Then, knowing the assumptions at the start and end of the loop,
 * we can merge then using `ast_assumes_mergecond()' (the logic here
 * being that a loop is a conditional branch in that it will either
 * continue running, in which case the new state will be used, or will
 * exit, in which case the old state will be used), thus meaning that
 * during a generic iteration, the true state of assumptions is the
 * intersections of assumptions made before and after the loop.
 * @return:  0: OK.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_assumes_gather)(struct ast_assumes *__restrict self,
                           struct ast *__restrict branch,
                           bool result_used) {
	switch (branch->a_type) {

	case AST_CONSTEXPR: /* Doesn't affect any symbols. */
	case AST_GOTO:      /* We don't care about jumps, or behavior after no-return. */
	case AST_FUNCTION:  /* Inner functions can't modify any of the symbols we're after. */
	case AST_LOOPCTL:   /* Same as `AST_GOTO': we don't care about jumps. */
		break;

	case AST_SYM:
		if (branch->a_flag) /* Untracked write to symbol -> Symbol now has an undefined value. */
			return ast_assumes_setsymval(self, branch->a_sym, NULL);
		break;

	case AST_UNBIND: /* Symbol gets unbound (XXX: maybe track this as `ITER_DONE'?) */
		return ast_assumes_setsymval(self, branch->a_sym, NULL);

		{
			size_t i;
		case AST_MULTIPLE:
			for (i = 0; i < branch->a_multiple.m_astc; ++i) {
				if (ast_assumes_gather(self, branch->a_multiple.m_astv[i],
				                       result_used &&
				                       i == branch->a_multiple.m_astc - 1))
					goto err;
			}
		}
		break;

	case AST_RETURN:
	case AST_YIELD:
	case AST_THROW:
		if (branch->a_return)
			return ast_assumes_gather(self, branch->a_return, true);
		break;

		/* TODO: All the other branch types. */
		/* TODO: Remember the special handling required for
		 *      `AST_CONDITIONAL', `AST_LOOP' and `AST_TRY'. */


	default:
		/*case AST_LABEL:*/ /* Labels are way too unpredictable to allow for this type of optimization... */
		/* Default case: anything we don't recognize clears all assumptions,
		 *               so we remain as future-proof as possible with new
		 *               types of branches. */
		return ast_assumes_undefined_all(self);
	}
	return 0;
err:
	return -1;
}



/* Assume a fully undefined state, creating new negative
 * assumptions for all previously made positive ones.
 * This must be done when encountering a label, as the state
 * of symbols would not be known at this point. */
INTERN WUNUSED NONNULL((1)) int
(DCALL ast_assumes_undefined)(struct ast_assumes *__restrict self) {
	size_t i;
	/* Mark all assumptions made locally as invalid. */
	if (self->aa_syms.sa_elem) {
		for (i = 0; i <= self->aa_syms.sa_mask; ++i) {
			if (!self->aa_syms.sa_elem[i].sa_sym)
				continue;
			Dee_XClear(self->aa_syms.sa_elem[i].sa_value);
		}
	}
	return 0;
}

INTERN WUNUSED NONNULL((1)) int
(DCALL ast_assumes_undefined_all)(struct ast_assumes *__restrict self) {
	size_t i;
	struct ast_assumes const *iter;
	if unlikely(ast_assumes_undefined(self))
		goto err;
	iter = self->aa_prev;
	while (iter) {
		if (iter->aa_syms.sa_size) {
			for (i = 0; i <= iter->aa_syms.sa_mask; ++i) {
				struct ast_symbol_assume *ass;
				ass = &iter->aa_syms.sa_elem[i];
				if (!ass->sa_sym)
					continue;
				if (!ass->sa_value)
					continue; /* Already a negative assumption (don't matter) */
				/* Overrule with a negative assumption. */
				if unlikely(ast_assumes_setsymval(self, ass->sa_sym, NULL))
					goto err;
			}
		}
		if (iter->aa_flag & AST_ASSUMES_FFUNCTION)
			break; /* Function base assumptions set. */
		iter = iter->aa_prev;
	}
	return 0;
err:
	return -1;
}

/* Initialize an empty set of ast assumptions. */
INTERN NONNULL((1)) void DCALL
ast_assumes_init(struct ast_assumes *__restrict self) {
	self->aa_prev         = NULL;
	self->aa_syms.sa_size = 0;
	self->aa_syms.sa_mask = 0;
	self->aa_syms.sa_elem = NULL;
	self->aa_flag         = AST_ASSUMES_FNORMAL;
}

/* Finalize the given ast assumptions. */
INTERN NONNULL((1)) void DCALL
ast_assumes_fini(struct ast_assumes *__restrict self) {
	size_t i;
	if (self->aa_syms.sa_elem) {
		for (i = 0; i <= self->aa_syms.sa_mask; ++i) {
			if (!self->aa_syms.sa_elem[i].sa_sym)
				continue;
			Dee_XDecref(self->aa_syms.sa_elem[i].sa_value);
		}
		Dee_Free(self->aa_syms.sa_elem);
	}
}

/* Setup AST assumption at the start of a conditional branch,
 * where the conditionally executed code is located in `child',
 * while assumptions already made until then are in `parent'
 * @return:  0: OK.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_assumes_initcond)(struct ast_assumes *__restrict child,
                             struct ast_assumes const *__restrict parent) {
	child->aa_prev         = parent;
	child->aa_syms.sa_size = 0;
	child->aa_syms.sa_mask = 0;
	child->aa_syms.sa_elem = NULL;
	child->aa_flag         = AST_ASSUMES_FNORMAL;
	return 0;
}

/* Initialize a set of assumptions for a child-function.
 * This also affects the limit of `ast_assumes_undefined_all()' */
INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_assumes_initfunction)(struct ast_assumes *__restrict child,
                                 struct ast_assumes const *__restrict parent) {
	child->aa_prev         = parent;
	child->aa_syms.sa_size = 0;
	child->aa_syms.sa_mask = 0;
	child->aa_syms.sa_elem = NULL;
	child->aa_flag         = AST_ASSUMES_FFUNCTION;
	return 0;
}


PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
same_constant_value(DeeObject *__restrict a,
                    DeeObject *__restrict b) {
	int temp;
	if (a == b)
		return true;
	if (Dee_TYPE(a) != Dee_TYPE(b))
		return false;
	temp = DeeObject_TryCompareForEquality(a, b);
	if unlikely(temp == Dee_COMPARE_ERR)
		DeeError_Handled(ERROR_HANDLED_RESTORE);
	return temp == 0;
}

/* Merge assumptions made in `child' and `sibling', such that
 * only assumptions made in both spaces still hold true, saving
 * that intersection in `child'. Or in other words:
 *   -> Remove all of `child's assumptions, not also made by `sibling'
 *   -> child = child & sibling;
 * >> local foo = 7;
 * >> if (bar()) {
 * >>     foo = 14;
 * >>     print "bar() was true";
 * >> } else {
 * >>     foo = 14;
 * >> }
 * >> // ASSUME(foo == 7)
 * Note however that negative assumptions (i.e. assumptions made
 * that state that the value of a symbol currently is unknown),
 * are merged as a union, meaning that it suffices for either `child'
 * or `sibling' to explicitly not know the value of a symbol, which
 * is required in cases such as the following:
 * >> local foo = 7;
 * >> if (bar()) {
 * >>     foo = get_new_foo();
 * >> } else {
 * >>     foo = 14;
 * >> }
 * >> // ASSUME(foo == UNKNOWN) // Even though both branches made assumptions for `foo'
 * When `sibling' is `NULL', only keep negative assumptions.
 * WARNING: `sibling' (when non-NULL) may have its data stolen.
 * @return:  0: OK.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1)) int
(DCALL ast_assumes_mergecond)(struct ast_assumes *__restrict child,
                              struct ast_assumes *sibling) {
	size_t i;
	if (!sibling)
		return ast_assumes_undefined(child);
	ASSERT(child->aa_prev == sibling->aa_prev);
	if (!child->aa_syms.sa_size) {
		Dee_Free(child->aa_syms.sa_elem);
		memcpy(&child->aa_syms, &sibling->aa_syms, sizeof(struct ast_symbol_assumes));
		bzero(&sibling->aa_syms, sizeof(struct ast_symbol_assumes));
		return ast_assumes_undefined(child);
	}
	/* Find all assumptions made by both branches. */
	for (i = 0; i <= child->aa_syms.sa_mask; ++i) {
		struct ast_symbol_assume *cass;
		struct ast_symbol_assume *sass;
		cass = &child->aa_syms.sa_elem[i];
		if (!cass->sa_sym)
			continue;
		if (!cass->sa_value)
			continue; /* Negative assumptions are or'd together. */
		sass = ast_assumes_getsymbol(sibling, cass->sa_sym);
		/* Turn into a negative assumptions if the other branch
		 * doesn't made the same assumptions about the symbol. */
		if (!sass || !sass->sa_value ||
		    !same_constant_value(cass->sa_value, sass->sa_value))
			Dee_Clear(cass->sa_value);
	}
	/* Copy all negative assumptions made by the sibling. */
	for (i = 0; i <= sibling->aa_syms.sa_mask; ++i) {
		struct ast_symbol_assume *sass;
		sass = &child->aa_syms.sa_elem[i];
		if (!sass->sa_sym)
			continue;
		if (sass->sa_value)
			continue; /* Positive assumptions have already been merged. */
		sass = ast_assumes_newsymbol(child, sass->sa_sym);
		if unlikely(!sass)
			goto err;
		sass->sa_value = NULL;
	}
	return 0;
err:
	return -1;
}



/* Merge the assumptions made by `follower' with `self' in a situation
 * where `follower' is a piece of follow-up code to `self', resulting
 * in the same behavior as would have been caused by all assumptions
 * made by `follower' instead having been made in `self'
 * This is used to merge assumptions from conditional branches onto
 * those made by the parent-branch, after they had been merged with
 * each other.
 * WARNING: `follower' may have its data stolen. */
INTERN WUNUSED NONNULL((1, 2)) int
(DCALL ast_assumes_merge)(struct ast_assumes *__restrict self,
                          struct ast_assumes *__restrict follower) {
	size_t i;
	if (follower->aa_syms.sa_size) {
		if (!self->aa_syms.sa_size) {
			Dee_Free(self->aa_syms.sa_elem);
			/* Steal all the assumptions. */
			memcpy(&self->aa_syms, &follower->aa_syms, sizeof(struct ast_symbol_assumes));
			bzero(&follower->aa_syms, sizeof(struct ast_symbol_assumes));
			return 0;
		}
		/* Override existing assumptions with those from `follower' */
		for (i = 0; i <= follower->aa_syms.sa_mask; ++i) {
			struct ast_symbol_assume *fass;
			fass = &follower->aa_syms.sa_elem[i];
			if (!fass->sa_sym)
				continue;
			if unlikely(ast_assumes_setsymval(self, fass->sa_sym, fass->sa_value))
				goto err;
		}
	}
	return 0;
err:
	return -1;
}

#endif /* OPTIMIZE_FASSUME */


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_ASSUMES_C */
