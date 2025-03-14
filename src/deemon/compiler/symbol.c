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
#ifndef GUARD_DEEMON_COMPILER_SYMBOL_C
#define GUARD_DEEMON_COMPILER_SYMBOL_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/util/cache.h>

#include <hybrid/minmax.h>
/**/

#include "../runtime/strings.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t, uint16_t */


DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

INTDEF struct module_symbol empty_module_buckets[];
DEFINE_STRUCT_CACHE_EX(sym, struct symbol,
                       MAX_C(sizeof(struct symbol),
                             sizeof(struct text_label)),
                       64)
#ifndef NDEBUG
#define sym_alloc() sym_dbgalloc(__FILE__, __LINE__)
#endif /* !NDEBUG */

/* Re-use the symbol cache for labels. (As rare as they are, this is the best way to allocate them) */
#define lbl_alloc() ((struct text_label *)sym_alloc()) /* TODO: Use slabs for this! */
#define lbl_free(p) sym_free((struct symbol *)(p))


INTERN DREF DeeScopeObject *current_scope     = NULL;
INTERN DeeBaseScopeObject  *current_basescope = NULL;
INTERN DeeRootScopeObject  *current_rootscope = NULL;
INTERN DeeModuleObject     *current_module    = NULL;


INTDEF char const symclass_names[0x1f + 1][8];
INTERN_CONST char const symclass_names[0x1f + 1][8] = {
	/* [SYMBOL_TYPE_NONE  ] = */ "none",
	/* [SYMBOL_TYPE_GLOBAL] = */ "global",
	/* [SYMBOL_TYPE_EXTERN] = */ "extern",
	/* [SYMBOL_TYPE_MODULE] = */ "module",
	/* [SYMBOL_TYPE_MYMOD ] = */ "mymod",
	/* [SYMBOL_TYPE_GETSET] = */ "getset",
	/* [SYMBOL_TYPE_CATTR ] = */ "cattr",
	/* [SYMBOL_TYPE_ALIAS ] = */ "alias",
	/* [SYMBOL_TYPE_ARG   ] = */ "arg",
	/* [SYMBOL_TYPE_LOCAL ] = */ "local",
	/* [SYMBOL_TYPE_STACK ] = */ "stack",
	/* [SYMBOL_TYPE_STATIC] = */ "static",
	/* [SYMBOL_TYPE_EXCEPT] = */ "except",
	/* [SYMBOL_TYPE_MYFUNC] = */ "myfunc",
	/* [SYMBOL_TYPE_THIS  ] = */ "this",
	/* [SYMBOL_TYPE_AMBIG ] = */ "ambig",
	/* [SYMBOL_TYPE_FWD   ] = */ "fwd",
	/* [SYMBOL_TYPE_CONST ] = */ "const",
	"?", "?", "?", "?", "?", "?", "?",
	"?", "?", "?", "?", "?", "?"
};


INTERN WUNUSED NONNULL((1, 2)) bool DCALL
symbol_uses_symbol_on_get(struct symbol *__restrict self,
                          struct symbol *__restrict other) {
again:
	if (self == other)
		return true;
	switch (self->s_type) {

	case SYMBOL_TYPE_ALIAS:
		self = self->s_alias;
		goto again;

	case SYMBOL_TYPE_CATTR:
		if (self->s_attr.a_this &&
		    symbol_uses_symbol_on_get(self->s_attr.a_this, other))
			return true;
		self = self->s_attr.a_class;
		goto again;

	case SYMBOL_TYPE_GETSET:
		self = self->s_getset.gs_get;
		if (!self)
			break;
		goto again;

		/* TODO: Check for overlap between pre-indexed external symbols */

	default: break;
	}
	switch (other->s_type) {

	case SYMBOL_TYPE_ALIAS:
		other = other->s_alias;
		goto again;

	case SYMBOL_TYPE_CATTR:
		if (other->s_attr.a_this &&
		    symbol_uses_symbol_on_get(self, other->s_attr.a_this))
			return true;
		other = other->s_attr.a_class;
		goto again;

	case SYMBOL_TYPE_GETSET:
		if (other->s_getset.gs_get &&
		    symbol_uses_symbol_on_get(self, other->s_getset.gs_get))
			return true;
		if (other->s_getset.gs_del &&
		    symbol_uses_symbol_on_get(self, other->s_getset.gs_del))
			return true;
		if (other->s_getset.gs_set) {
			other = other->s_getset.gs_set;
			goto again;
		}
		break;

	default: break;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
symbol_uses_symbol_on_del(struct symbol *__restrict self,
                          struct symbol *__restrict other) {
again:
	if (self == other)
		return true;
	switch (self->s_type) {

	case SYMBOL_TYPE_ALIAS:
		self = self->s_alias;
		goto again;

	case SYMBOL_TYPE_CATTR:
		if (self->s_attr.a_this &&
		    symbol_uses_symbol_on_get(self->s_attr.a_this, other))
			return true;
		return symbol_uses_symbol_on_get(self->s_attr.a_class, other);

	case SYMBOL_TYPE_GETSET:
		if (!self->s_getset.gs_del)
			break;
		return symbol_uses_symbol_on_get(self->s_getset.gs_del, other);

		/* TODO: Check for overlap between pre-indexed external symbols */

	default: break;
	}
	switch (other->s_type) {

	case SYMBOL_TYPE_ALIAS:
		other = other->s_alias;
		goto again;

	case SYMBOL_TYPE_CATTR:
		if (other->s_attr.a_this &&
		    symbol_uses_symbol_on_del(self, other->s_attr.a_this))
			return true;
		other = other->s_attr.a_class;
		goto again;

	case SYMBOL_TYPE_GETSET:
		if (other->s_getset.gs_get &&
		    symbol_uses_symbol_on_del(self, other->s_getset.gs_get))
			return true;
		if (other->s_getset.gs_del &&
		    symbol_uses_symbol_on_del(self, other->s_getset.gs_del))
			return true;
		if (other->s_getset.gs_set) {
			other = other->s_getset.gs_set;
			goto again;
		}
		break;

	default: break;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
symbol_uses_symbol_on_set(struct symbol *__restrict self,
                          struct symbol *__restrict other) {
again:
	if (self == other)
		return true;
	switch (self->s_type) {

	case SYMBOL_TYPE_ALIAS:
		self = self->s_alias;
		goto again;

	case SYMBOL_TYPE_CATTR:
		if (self->s_attr.a_this &&
		    symbol_uses_symbol_on_get(self->s_attr.a_this, other))
			return true;
		return symbol_uses_symbol_on_get(self->s_attr.a_class, other);

	case SYMBOL_TYPE_GETSET:
		if (!self->s_getset.gs_set)
			break;
		return symbol_uses_symbol_on_get(self->s_getset.gs_set, other);

		/* TODO: Check for overlap between pre-indexed external symbols */

	default: break;
	}
	switch (other->s_type) {

	case SYMBOL_TYPE_ALIAS:
		other = other->s_alias;
		goto again;

	case SYMBOL_TYPE_CATTR:
		if (other->s_attr.a_this &&
		    symbol_uses_symbol_on_get(self, other->s_attr.a_this))
			return true;
		other = other->s_attr.a_class;
		goto again;

	case SYMBOL_TYPE_GETSET:
		if (other->s_getset.gs_get &&
		    symbol_uses_symbol_on_set(self, other->s_getset.gs_get))
			return true;
		if (other->s_getset.gs_del &&
		    symbol_uses_symbol_on_set(self, other->s_getset.gs_del))
			return true;
		if (other->s_getset.gs_set) {
			other = other->s_getset.gs_set;
			goto again;
		}
		break;

	default: break;
	}
	return false;
}


INTERN WUNUSED NONNULL((1, 2)) bool DCALL
symbol_reachable(struct symbol *__restrict self,
                 DeeScopeObject *__restrict caller_scope) {
again:
	switch (self->s_type) {

	case SYMBOL_TYPE_ALIAS:
		self = self->s_alias;
		goto again;

	case SYMBOL_TYPE_THIS: {
		DeeBaseScopeObject *this_origin;
		DeeBaseScopeObject *sym_base;
		sym_base    = self->s_scope->s_base;
		this_origin = caller_scope->s_base;
		do {
			if (this_origin->bs_this == self) /* Bound & reachable this-symbol. */
				return true;
			if (this_origin == sym_base) /* Unbound this-symbol (valid in any kind of this-call) */
				return (this_origin->bs_flags & CODE_FTHISCALL) != 0;
		} while ((this_origin = this_origin->bs_prev) != NULL);
	}	break;

	case SYMBOL_TYPE_STACK:
	case SYMBOL_TYPE_EXCEPT:
		do {
			/* Only reachable from their declaration scope. */
			if (caller_scope == self->s_scope)
				return true;
		} while ((caller_scope = caller_scope->s_prev) != NULL);
		break;

	case SYMBOL_TYPE_GETSET:
		if (!self->s_getset.gs_get)
			return true;
		self = self->s_getset.gs_get;
		goto again;

	case SYMBOL_TYPE_CATTR:
		/* Make sure that both the this-, as well
		 * as the class-symbols are accessible. */
		if (self->s_attr.a_this &&
		    !symbol_reachable(self->s_attr.a_this, caller_scope))
			break;
		self = self->s_attr.a_class;
		goto again;

	default: {
		DeeBaseScopeObject *this_origin;
		DeeBaseScopeObject *sym_base;
		sym_base    = self->s_scope->s_base;
		this_origin = caller_scope->s_base;
		/* Any other type of symbol: reachable from the same, or a child base-scope. */
		do {
			if (this_origin == sym_base)
				return true;
		} while ((this_origin = this_origin->bs_prev) != NULL);
	}	break;
	}
	return false;
}


INTERN WUNUSED NONNULL((1, 2)) bool DCALL
symbol_get_haseffect(struct symbol *__restrict self,
                     DeeScopeObject *__restrict caller_scope) {
again:
	switch (self->s_type) {

	case SYMBOL_TYPE_ALIAS:
		self = self->s_alias;
		goto again;

	case SYMBOL_TYPE_CATTR:
		if (self->s_attr.a_attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
			return true;
		if (symbol_get_haseffect(self->s_attr.a_class, caller_scope))
			return true;
		if (!self->s_attr.a_this)
			break;
		/* The attribute must be accessed as virtual. */
		if unlikely(!symbol_reachable(self, caller_scope))
			return true;
		return symbol_get_haseffect(self->s_attr.a_this, caller_scope);

	case SYMBOL_TYPE_EXTERN:
		if (self->s_extern.e_symbol->ss_flags & MODSYM_FPROPERTY)
			return true;
		break;

	case SYMBOL_TYPE_GETSET:
		return true;

	default: break;
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
symbol_set_haseffect(struct symbol *__restrict self,
                     DeeScopeObject *__restrict caller_scope) {
again:
	switch (self->s_type) {

	case SYMBOL_TYPE_ALIAS:
		self = self->s_alias;
		goto again;

	case SYMBOL_TYPE_CATTR:
		if (!(self->s_attr.a_attr->ca_flag &
		      /* A non-private symbol may be accessed from the outside, meaning
		       * that writing to this kind of symbol _does_ have side-effects. */
		      CLASS_ATTRIBUTE_FPRIVATE))
			return true;
		if (self->s_attr.a_attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
			return true; /* Properties have side-effects */
		if (symbol_get_haseffect(self->s_attr.a_class, caller_scope))
			return true;
		if (!self->s_attr.a_this)
			break;
		/* The attribute must be accessed as virtual. */
		if unlikely(!symbol_reachable(self, caller_scope))
			return true;
		return symbol_get_haseffect(self->s_attr.a_this, caller_scope);

	case SYMBOL_TYPE_EXTERN:
		if (self->s_extern.e_symbol->ss_flags & MODSYM_FPROPERTY)
			return true;
		break;

	case SYMBOL_TYPE_GETSET:
		return true;

	case SYMBOL_TYPE_STATIC:
		/* Static variables are visible beyond the relevant function
		 * call, meaning they *always* have side-effects and can't be
		 * optimized away. */
		return true;

	default: break;
	}
	return false;
}



INTERN NONNULL((1)) void DCALL
symbol_addambig(struct symbol *__restrict self,
                struct ast_loc *loc) {
	struct ast_loc *new_vec;
	ASSERT(self->s_type == SYMBOL_TYPE_AMBIG);
	new_vec = (struct ast_loc *)Dee_TryReallocc(self->s_ambig.a_declv,
	                                            self->s_ambig.a_declc + 1,
	                                            sizeof(struct ast_loc));
	if unlikely(!new_vec)
		return;
	self->s_ambig.a_declv = new_vec;
	new_vec += self->s_ambig.a_declc++;
	if (loc) {
		if (!tpp_is_reachable_file(loc->l_file))
			goto set_default_location;
		memcpy(new_vec, loc, sizeof(struct ast_loc));
	} else {
set_default_location:
		loc_here(new_vec);
	}
	if (new_vec->l_file)
		TPPFile_Incref(new_vec->l_file);
}

INTERN NONNULL((1)) void DCALL
symbol_fini(struct symbol *__restrict self) {
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	decl_ast_fini(&self->s_decltype);
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	switch (self->s_type) {

	case SYMBOL_TYPE_EXTERN:
	case SYMBOL_TYPE_MODULE:
		Dee_Decref(self->s_module);
		break;

	case SYMBOL_TYPE_GLOBAL:
		Dee_XDecref(self->s_global.g_doc);
		break;

	case SYMBOL_TYPE_ALIAS:
		SYMBOL_SUB_NREAD(self->s_alias, self->s_nread);
		SYMBOL_SUB_NWRITE(self->s_alias, self->s_nwrite);
		SYMBOL_SUB_NBOUND(self->s_alias, self->s_nbound);
		symbol_decref(self->s_alias);
		break;

	case SYMBOL_TYPE_CATTR:
		SYMBOL_DEC_NREAD(self->s_attr.a_class);
		symbol_decref(self->s_attr.a_class);
		if (self->s_attr.a_this) {
			SYMBOL_DEC_NREAD(self->s_attr.a_this);
			symbol_decref(self->s_attr.a_this);
		}
		break;

	case SYMBOL_TYPE_GETSET:
		if (self->s_getset.gs_get)
			SYMBOL_DEC_NREAD(self->s_getset.gs_get),
			symbol_decref(self->s_getset.gs_get);
		if (self->s_getset.gs_del)
			SYMBOL_DEC_NREAD(self->s_getset.gs_del),
			symbol_decref(self->s_getset.gs_del);
		if (self->s_getset.gs_set)
			SYMBOL_DEC_NREAD(self->s_getset.gs_set),
			symbol_decref(self->s_getset.gs_set);
		break;

	case SYMBOL_TYPE_AMBIG: {
		size_t i;
		if (self->s_ambig.a_decl2.l_file)
			TPPFile_Decref(self->s_ambig.a_decl2.l_file);
		for (i = 0; i < self->s_ambig.a_declc; ++i) {
			if (self->s_ambig.a_declv[i].l_file)
				TPPFile_Decref(self->s_ambig.a_declv[i].l_file);
		}
		Dee_Free(self->s_ambig.a_declv);
	}	break;

	case SYMBOL_TYPE_CONST:
		Dee_Decref(self->s_const);
		break;

	default: break;
	}
}


/* -------- DeeScopeObject Implementation -------- */
#ifdef CONFIG_SYMBOL_HAS_REFCNT
INTERN NONNULL((1)) void DCALL
symbol_destroy(struct symbol *__restrict self)
#else /* CONFIG_SYMBOL_HAS_REFCNT */
PRIVATE NONNULL((1)) void DCALL
delsym(struct symbol *__restrict self)
#endif /* !CONFIG_SYMBOL_HAS_REFCNT */
{
	DeeCompiler_DelItem(self);
	if (self->s_decl.l_file)
		TPPFile_Decref(self->s_decl.l_file);
	symbol_fini(self);
	sym_free(self);
}

PRIVATE void DCALL
visitsym(struct symbol *__restrict self, dvisit_t proc, void *arg) {
	switch (self->s_type) {

	case SYMBOL_TYPE_EXTERN:
	case SYMBOL_TYPE_MODULE:
		Dee_Visit(self->s_module);
		break;

#if 0
	case SYMBOL_TYPE_GLOBAL:
		Dee_XVisit(self->s_global.g_doc);
		break;
#endif

	default: break;
	}
}

PRIVATE NONNULL((1)) void DCALL
scope_fini(DeeScopeObject *__restrict self) {
	struct symbol **biter, **bend, *iter, *next;
	weakref_support_fini(self);
	DeeCompiler_DelItem(self);
	biter = self->s_map;
	bend  = biter + self->s_mapa;
	for (; biter < bend; ++biter) {
		iter = *biter;
		while (iter) {
			next = iter->s_next;
			ASSERT(iter->s_scope == self);
#ifdef CONFIG_SYMBOL_HAS_REFCNT
			iter->s_scope = NULL;
			symbol_decref(iter);
#else /* CONFIG_SYMBOL_HAS_REFCNT */
			delsym(iter);
#endif /* !CONFIG_SYMBOL_HAS_REFCNT */
			iter = next;
		}
	}
	Dee_Free(self->s_map);
	iter = self->s_del;
	while (iter) {
		next = iter->s_next;
		ASSERT(iter->s_scope == self);
#ifdef CONFIG_SYMBOL_HAS_REFCNT
		iter->s_scope = NULL;
		symbol_decref(iter);
#else /* CONFIG_SYMBOL_HAS_REFCNT */
		delsym(iter);
#endif /* !CONFIG_SYMBOL_HAS_REFCNT */
		iter = next;
	}
	Dee_XDecref(self->s_prev);
}

PRIVATE NONNULL((1, 2)) void DCALL
scope_visit(DeeScopeObject *__restrict self, dvisit_t proc, void *arg) {
	struct symbol **biter, **bend, *iter;
	DeeCompiler_LockReadNoInt();
	biter = self->s_map;
	bend  = biter + self->s_mapa;
	for (; biter < bend; ++biter) {
		iter = *biter;
		while (iter) {
			visitsym(iter, proc, arg);
			iter = iter->s_next;
		}
	}
	iter = self->s_del;
	while (iter) {
		visitsym(iter, proc, arg);
		iter = iter->s_next;
	}
	Dee_XVisit(self->s_prev);
	DeeCompiler_LockEndRead();
}

INTERN DeeTypeObject DeeScope_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "Scope",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeScopeObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ NULL, /*&DeeObject_Type*/
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeScopeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&scope_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&scope_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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









PRIVATE NONNULL((1)) void DCALL
class_scope_fini(DeeClassScopeObject *__restrict self) {
	ASSERT(self->cs_this->s_scope == (DeeScopeObject *)self);
#ifdef CONFIG_SYMBOL_HAS_REFCNT
	self->cs_this->s_scope = NULL;
	symbol_decref(self->cs_this);
#else /* CONFIG_SYMBOL_HAS_REFCNT */
	sym_free(self->cs_this);
#endif /* !CONFIG_SYMBOL_HAS_REFCNT */
}

INTERN DeeTypeObject DeeClassScope_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "ClassScope",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeScopeObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeScope_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeClassScopeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&class_scope_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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









INTDEF void DCALL
cleanup_switch_cases(struct text_label *switch_cases,
                     struct text_label *switch_default);
INTDEF NONNULL((2)) void DCALL
visit_switch_cases(struct text_label *switch_cases,
                   dvisit_t proc, void *arg);


/* -------- DeeBaseScopeObject Implementation -------- */
PRIVATE NONNULL((1)) void DCALL
base_scope_fini(DeeBaseScopeObject *__restrict self) {
	ASSERT(self->bs_argc_max >= self->bs_argc_min);
	{
		struct text_label **biter, **bend, *iter, *next;
		biter = self->bs_lbl;
		bend  = biter + self->bs_lbla;
		for (; biter < bend; ++biter) {
			iter = *biter;
			while (iter) {
				next = iter->tl_next;
				DeeCompiler_DelItem(iter);
				lbl_free(iter);
				iter = next;
			}
		}
		Dee_Free(self->bs_lbl);
	}
	cleanup_switch_cases(self->bs_swcase,
	                     self->bs_swdefl);
	Dee_XDecrefv(self->bs_default,
	             (self->bs_argc_max - self->bs_argc_min));
	Dee_Free(self->bs_default);
	Dee_Free(self->bs_argv);
}

PRIVATE NONNULL((1, 2)) void DCALL
base_scope_visit(DeeBaseScopeObject *__restrict self,
                 dvisit_t proc, void *arg) {
	DeeCompiler_LockReadNoInt();
	ASSERT(self->bs_argc_max >= self->bs_argc_min);
	visit_switch_cases(self->bs_swcase, proc, arg);
	Dee_XVisitv(self->bs_default,
	            self->bs_argc_max -
	            self->bs_argc_min);
	DeeCompiler_LockEndRead();
}

INTERN DeeTypeObject DeeBaseScope_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "BaseScope",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeScopeObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeScope_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeBaseScopeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&base_scope_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&base_scope_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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










/* -------- DeeRootScopeObject Implementation -------- */
PRIVATE WUNUSED NONNULL((1)) int DCALL
root_scope_init(DeeRootScopeObject *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeModuleObject *module;
	_DeeArg_Unpack1(err, argc, argv, "root_scope", &module);
	if (DeeObject_AssertType(module, &DeeModule_Type))
		goto err;
	bzero((uint8_t *)self + offsetof(DeeScopeObject, s_prev),
	      sizeof(DeeRootScopeObject) - offsetof(DeeScopeObject, s_prev));
	weakref_support_init((DeeScopeObject *)self);
	Dee_Incref(module);
	self->rs_scope.bs_scope.s_base = &self->rs_scope;
	self->rs_scope.bs_root         = self;
	self->rs_module                = module;
	self->rs_bucketv               = empty_module_buckets;
#if CODE_FNORMAL != 0
	self->rs_scope.bs_flags = CODE_FNORMAL;
#endif /* CODE_FNORMAL != 0 */
#if MODULE_FNORMAL != 0
	self->rs_flags = MODULE_FNORMAL;
#endif /* MODULE_FNORMAL != 0 */
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
root_scope_fini(DeeRootScopeObject *__restrict self) {
	Dee_Decref(self->rs_module);
	Dee_XDecref(self->rs_code);
	Dee_Decrefv(self->rs_importv, self->rs_importc);
	Dee_Free(self->rs_importv);
	if (self->rs_bucketv != empty_module_buckets) {
		struct module_symbol *iter, *end;
		end = (iter = self->rs_bucketv) + (self->rs_bucketm + 1);
		for (; iter < end; ++iter) {
			if (!MODULE_SYMBOL_GETNAMESTR(iter))
				continue;
			if (iter->ss_flags & MODSYM_FNAMEOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(MODULE_SYMBOL_GETNAMESTR(iter), DeeStringObject, s_str));
			if (iter->ss_flags & MODSYM_FDOCOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(iter->ss_doc, DeeStringObject, s_str));
		}
		Dee_Free(self->rs_bucketv);
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
root_scope_str(DeeRootScopeObject *__restrict self) {
	return_reference_((DeeObject *)self->rs_module->mo_name);
}

PRIVATE NONNULL((1, 2)) void DCALL
root_scope_visit(DeeRootScopeObject *__restrict self,
                 dvisit_t proc, void *arg) {
	size_t i;
	DeeCompiler_LockReadNoInt();
	Dee_Visit(self->rs_module);
	Dee_XVisit(self->rs_code);
	for (i = 0; i < self->rs_importc; ++i)
		Dee_Visit(self->rs_importv[i]);
	DeeCompiler_LockEndRead();
}

INTERN DeeTypeObject DeeRootScope_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "RootScope",
	/* .tp_doc      = */ DOC("(module:?DModule)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeScopeObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeBaseScope_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&root_scope_init,
				TYPE_FIXED_ALLOCATOR(DeeRootScopeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&root_scope_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&root_scope_str,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&root_scope_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
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



INTERN WUNUSED int (DCALL scope_push)(void) {
	DREF DeeScopeObject *new_scope;
	new_scope = DeeObject_CALLOC(DeeScopeObject);
	if unlikely(!new_scope)
		goto err;
	DeeObject_Init(new_scope, &DeeScope_Type);
	new_scope->s_prev  = current_scope; /* Inherit reference */
	new_scope->s_base  = current_basescope;
	new_scope->s_class = current_scope->s_class;
	current_scope      = new_scope; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN void DCALL scope_pop(void) {
	DeeScopeObject *pop_scope;
	ASSERT(current_scope != NULL);
	ASSERT(current_scope != (DeeScopeObject *)current_basescope);
	ASSERT(current_scope != (DeeScopeObject *)current_rootscope);
	ASSERT(current_scope->s_base == current_basescope);
	ASSERT_OBJECT_TYPE(current_scope->s_prev, &DeeScope_Type);
	pop_scope     = current_scope;
	current_scope = pop_scope->s_prev;
	Dee_Incref(current_scope); /* Keep a reference in `current_scope' */
	Dee_Decref(pop_scope);     /* Drop the reference previously stored in `current_scope' */
}

INTERN WUNUSED int (DCALL classscope_push)(void) {
	DREF DeeClassScopeObject *new_scope;
	struct symbol *this_sym;
	new_scope = DeeObject_CALLOC(DeeClassScopeObject);
	if unlikely(!new_scope)
		goto err;
	this_sym = sym_alloc();
	if unlikely(!this_sym)
		goto err_new_scope;
	DeeObject_Init((DeeObject *)new_scope, &DeeClassScope_Type);
	bzero(this_sym, sizeof(*this_sym));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
	this_sym->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
#if DAST_NONE != 0
	this_sym->s_decltype.da_type = DAST_NONE;
#endif /* DAST_NONE != 0 */
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	this_sym->s_type  = SYMBOL_TYPE_THIS;
	this_sym->s_scope = &new_scope->cs_scope;
	this_sym->s_name  = TPPLexer_LookupKeyword(STR_this, 4, 0);
	ASSERT(this_sym->s_name);
	new_scope->cs_scope.s_class = new_scope;
	new_scope->cs_scope.s_prev  = current_scope; /* Inherit reference */
	new_scope->cs_scope.s_base  = current_basescope;
	new_scope->cs_this          = this_sym;
	current_scope               = &new_scope->cs_scope; /* Inherit reference */
	return 0;
err_new_scope:
	DeeObject_FREE(new_scope);
err:
	return -1;
}

INTERN WUNUSED struct symbol *(DCALL get_current_this)(void) {
	DeeBaseScopeObject *base;
	base = current_basescope;
	do {
		if (base->bs_this)
			return base->bs_this;
	} while ((base = base->bs_prev) != NULL);
	return NULL;
}

INTERN NONNULL((1)) void DCALL
basescope_push_ob(DeeBaseScopeObject *__restrict scope) {
	ASSERT((current_scope != NULL) == (current_basescope != NULL));
	ASSERT(scope->bs_scope.s_prev == current_scope);
	ASSERT(scope->bs_scope.s_class == (current_scope ? current_scope->s_class : NULL));
	ASSERT(scope->bs_prev == current_basescope);
	Dee_Incref((DeeObject *)scope);
	Dee_Decref(current_scope);
	current_scope     = (DREF DeeScopeObject *)scope;
	current_basescope = scope;
}

INTERN WUNUSED int (DCALL basescope_push)(void) {
	DREF DeeBaseScopeObject *new_scope;
	new_scope = DeeObject_CALLOC(DeeBaseScopeObject);
	if unlikely(!new_scope)
		goto err;
	DeeObject_Init((DeeObject *)new_scope, &DeeBaseScope_Type);
	ASSERT(current_scope != NULL);
	ASSERT(current_rootscope == current_basescope->bs_root);
	new_scope->bs_scope.s_prev  = current_scope; /* Inherit reference */
	new_scope->bs_scope.s_base  = new_scope;
	new_scope->bs_scope.s_class = current_scope->s_class;
	new_scope->bs_prev          = current_basescope;
	new_scope->bs_root          = current_rootscope;
	current_basescope           = new_scope;
	current_scope               = (DREF DeeScopeObject *)new_scope; /* Inherit reference */
	return 0;
err:
	return -1;
}

INTERN void DCALL basescope_pop(void) {
	DeeBaseScopeObject *pop_scope;
	ASSERT(current_scope != NULL);
	ASSERT(current_basescope != NULL);
	if (current_scope != (DeeScopeObject *)current_basescope)
		return; /* Can happen during error-cleanup. */
	ASSERT(current_scope != (DeeScopeObject *)current_rootscope);
	ASSERT(current_scope->s_base == current_basescope);
	pop_scope         = current_basescope;
	current_scope     = pop_scope->bs_scope.s_prev;
	current_basescope = pop_scope->bs_prev;
	Dee_Incref(current_scope);
	Dee_Decref((DeeObject *)pop_scope); /* Drop the reference previously stored in `current_scope' */
}


INTERN WUNUSED NONNULL((1)) int DCALL
copy_argument_symbols(DeeBaseScopeObject *__restrict other) {
	unsigned int i, count;
	ASSERT(current_basescope != other);
	ASSERT(!current_basescope->bs_argc);
	ASSERT(!current_basescope->bs_argc_min);
	ASSERT(!current_basescope->bs_argc_max);
	ASSERT(!current_basescope->bs_argv);
	ASSERT(!current_basescope->bs_default);
	ASSERT(!current_basescope->bs_varargs);
	ASSERT(!current_basescope->bs_varkwds);
	ASSERT(!(current_basescope->bs_flags & (CODE_FVARARGS | CODE_FVARKWDS)));

	/* Copy basic flags and counters. */
	current_basescope->bs_flags |= other->bs_flags & (CODE_FVARARGS | CODE_FVARKWDS);
	current_basescope->bs_argc_max = other->bs_argc_max;
	current_basescope->bs_argc_min = other->bs_argc_min;
	current_basescope->bs_argc     = other->bs_argc;

	/* Copy default arguments. */
	count = other->bs_argc_max - other->bs_argc_min;
	if (count) {
		current_basescope->bs_default = (DREF DeeObject **)Dee_Mallocc(count, sizeof(DREF DeeObject *));
		if unlikely(!current_basescope->bs_default)
			goto err;
		Dee_XMovrefv(current_basescope->bs_default,
		             other->bs_default, count);
	}
	count = other->bs_argc;
	if (count) {
		current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(count, sizeof(struct symbol *));
		if unlikely(!current_basescope->bs_argv)
			goto err;
		/* Copy the actual argument symbols. */

		for (i = 0; i < count; ++i) {
			struct symbol *sym, *other_sym;
			other_sym = other->bs_argv[i];
			sym = other_sym->s_name == &TPPKeyword_Empty
			      ? new_unnamed_symbol()
			      : new_local_symbol(other_sym->s_name, &other_sym->s_decl);
			if unlikely(!sym)
				goto err;
			sym->s_type = SYMBOL_TYPE_ARG;
			ASSERT(other_sym->s_symid == (uint16_t)i);
			sym->s_flag  = SYMBOL_FALLOC;
			sym->s_symid = (uint16_t)i;
			/* Save the symbol in our vector. */
			current_basescope->bs_argv[i] = sym;
		}
	}
	if (other->bs_varargs) {
		ASSERT(current_basescope->bs_argv != NULL);
		ASSERT(current_basescope->bs_argc != 0);
		ASSERT(other->bs_varargs->s_symid < current_basescope->bs_argc);
		current_basescope->bs_varargs = current_basescope->bs_argv[other->bs_varargs->s_symid];
	}
	if (other->bs_varkwds) {
		ASSERT(current_basescope->bs_argv != NULL);
		ASSERT(other->bs_varkwds->s_symid < current_basescope->bs_argc);
		current_basescope->bs_varkwds = current_basescope->bs_argv[other->bs_varkwds->s_symid];
	}
	return 0;
err:
	count = other->bs_argc_max - other->bs_argc_min;
	Dee_XDecrefv(current_basescope->bs_default, count);
	Dee_Free(current_basescope->bs_default);
	current_basescope->bs_argc     = 0;
	current_basescope->bs_argc_min = 0;
	current_basescope->bs_argc_max = 0;
	current_basescope->bs_flags &= ~CODE_FVARARGS;
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
link_forward_symbol(struct symbol *__restrict self) {
	DeeScopeObject *iter;
	if (self->s_type != SYMBOL_TYPE_FWD)
		return 0;
	iter = current_scope->s_prev;
	for (; iter; iter = iter->s_prev) {
		struct symbol *outer_match;
		outer_match = scope_lookup(iter, self->s_name);
		if (!outer_match)
			continue;
		/* Setup the symbol as an alias for another, outer symbol. */
		ASSERT(!(self->s_flag & (SYMBOL_FALLOCREF | SYMBOL_FALLOC)));
		self->s_type  = SYMBOL_TYPE_ALIAS;
		self->s_alias = outer_match;
		SYMBOL_ADD_NREAD(outer_match, self->s_nread);
		SYMBOL_ADD_NWRITE(outer_match, self->s_nwrite);
		SYMBOL_ADD_NBOUND(outer_match, self->s_nbound);
		symbol_incref(outer_match);
		return 0;
	}
	if (WARNSYM(self, W_UNKNOWN_VARIABLE, SYMBOL_NAME(self)))
		goto err;
	self->s_type = SYMBOL_TYPE_NONE; /* Prevent the assembler from crashing later... */
	return 0;
err:
	return -1;
}

INTERN WUNUSED int DCALL link_forward_symbols(void) {
	struct symbol **bucket_iter, **bucket_end, *iter;
	ASSERT(DeeScope_IsClassScope(current_scope));
	bucket_end = (bucket_iter = current_scope->s_map) + current_scope->s_mapa;
	for (; bucket_iter < bucket_end; ++bucket_iter) {
		iter = *bucket_iter;
		for (; iter; iter = iter->s_next)
			if unlikely(link_forward_symbol(iter))
		goto err;
	}
	/* Must also perform final linking on symbols that have been deleted. */
	iter = current_scope->s_del;
	for (; iter; iter = iter->s_next)
		if unlikely(link_forward_symbol(iter))
	goto err;
	return 0;
err:
	return -1;
}



INTERN WUNUSED NONNULL((1)) int DCALL
rehash_scope(DeeScopeObject *__restrict iter) {
	struct symbol **new_map, **biter, **bend;
	struct symbol *sym_iter, *s_next, **bucket;
	size_t old_size = iter->s_mapa;
	size_t new_size = old_size;
	if (!new_size)
		new_size = 1;
	new_size *= 2;
rehash_realloc:
	new_map = (struct symbol **)Dee_TryCallocc(new_size, sizeof(struct symbol *));
	if unlikely(!new_map) {
		if (new_size != 1) {
			new_size = 1;
			goto rehash_realloc;
		}
		if (old_size) {
			if (Dee_TryCollectMemory(sizeof(struct symbol *)))
				goto rehash_realloc;
			return 0;
		}
		if (Dee_CollectMemory(sizeof(struct symbol *)))
			goto rehash_realloc;
		return -1;
	}
	/* Rehash all symbols. */
	biter = iter->s_map;
	bend  = biter + old_size;
	for (; biter < bend; ++biter) {
		sym_iter = *biter;
		while (sym_iter) {
			s_next           = sym_iter->s_next;
			bucket           = &new_map[sym_iter->s_name->k_id % new_size];
			sym_iter->s_next = *bucket;
			*bucket          = sym_iter;
			sym_iter         = s_next;
		}
	}
	Dee_Free(iter->s_map);
	iter->s_map  = new_map;
	iter->s_mapa = new_size;
	return 0;
}

INTERN WUNUSED NONNULL((1)) bool DCALL
is_reserved_symbol_name(struct TPPKeyword *__restrict name) {
	/* Quick check: any keywords not registered as builtin are allowed. */
	if (TPP_ISUSERKEYWORD(name->k_id))
		return false;

	/* White-list of non-reserved builtin keywords. */
	switch (name->k_id) {

	case KWD_f:
	case KWD_F:
	case KWD_ifdef:
	case KWD_ifndef:
	case KWD_endif:
	case KWD_undef:
	case KWD_include:
	case KWD_include_next:
	case KWD_line:
	case KWD_error:
	case KWD_warning:
	case KWD_define:
	case KWD_defined:
#if !defined(TPP_CONFIG_EXTENSION_IDENT_SCCS) || TPP_CONFIG_EXTENSION_IDENT_SCCS
	case KWD_ident:
	case KWD_sccs:
#endif /* !defined(TPP_CONFIG_EXTENSION_IDENT_SCCS) || TPP_CONFIG_EXTENSION_IDENT_SCCS */
#if !defined(TPP_CONFIG_EXTENSION_ASSERTIONS) || TPP_CONFIG_EXTENSION_ASSERTIONS
	case KWD_unassert:
#endif /* !defined(TPP_CONFIG_EXTENSION_ASSERTIONS) || TPP_CONFIG_EXTENSION_ASSERTIONS */
	case KWD_pragma:
#if !TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS
#if !defined(TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA) || TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA
	case KWD_tpp_dollar_is_alpha:
#endif /* !defined(TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA) || TPP_CONFIG_EXTENSION_DOLLAR_IS_ALPHA */
#if !defined(TPP_CONFIG_EXTENSION_VA_ARGS) || TPP_CONFIG_EXTENSION_VA_ARGS
	case KWD_tpp_va_args:
#endif /* !defined(TPP_CONFIG_EXTENSION_VA_ARGS) || TPP_CONFIG_EXTENSION_VA_ARGS */
#if !defined(TPP_CONFIG_EXTENSION_GCC_VA_ARGS) || TPP_CONFIG_EXTENSION_GCC_VA_ARGS
	case KWD_tpp_named_va_args:
#endif /* !defined(TPP_CONFIG_EXTENSION_GCC_VA_ARGS) || TPP_CONFIG_EXTENSION_GCC_VA_ARGS */
#if !defined(TPP_CONFIG_EXTENSION_VA_COMMA) || TPP_CONFIG_EXTENSION_VA_COMMA
	case KWD_tpp_va_comma:
#endif /* !defined(TPP_CONFIG_EXTENSION_VA_COMMA) || TPP_CONFIG_EXTENSION_VA_COMMA */
	case KWD_tpp_reemit_unknown_pragmas:
#if !defined(TPP_CONFIG_EXTENSION_MSVC_FIXED_INT) || TPP_CONFIG_EXTENSION_MSVC_FIXED_INT
	case KWD_tpp_msvc_integer_suffix:
#endif /* !defined(TPP_CONFIG_EXTENSION_MSVC_FIXED_INT) || TPP_CONFIG_EXTENSION_MSVC_FIXED_INT */
#if !defined(TPP_CONFIG_EXTENSION_HASH_AT) || TPP_CONFIG_EXTENSION_HASH_AT
	case KWD_tpp_charize_operator:
#endif /* !defined(TPP_CONFIG_EXTENSION_HASH_AT) || TPP_CONFIG_EXTENSION_HASH_AT */
#if !defined(TPP_CONFIG_FEATURE_TRIGRAPHS) || TPP_CONFIG_FEATURE_TRIGRAPHS
	case KWD_tpp_trigraphs:
#endif /* !defined(TPP_CONFIG_FEATURE_TRIGRAPHS) || TPP_CONFIG_FEATURE_TRIGRAPHS */
#if !defined(TPP_CONFIG_FEATURE_DIGRAPHS) || TPP_CONFIG_FEATURE_DIGRAPHS
	case KWD_tpp_digraphs:
#endif /* !defined(TPP_CONFIG_FEATURE_DIGRAPHS) || TPP_CONFIG_FEATURE_DIGRAPHS */
	case KWD_tpp_pragma_push_macro:
	case KWD_tpp_pragma_pop_macro:
	case KWD_tpp_pragma_region:
	case KWD_tpp_pragma_endregion:
	case KWD_tpp_pragma_warning:
	case KWD_tpp_pragma_message:
	case KWD_tpp_pragma_error:
	case KWD_tpp_pragma_once:
	case KWD_tpp_pragma_tpp_exec:
	case KWD_tpp_pragma_deprecated:
	case KWD_tpp_pragma_tpp_set_keyword_flags:
#if !defined(TPP_CONFIG_EXTENSION_INCLUDE_NEXT) || TPP_CONFIG_EXTENSION_INCLUDE_NEXT
	case KWD_tpp_directive_include_next:
#endif /* !defined(TPP_CONFIG_EXTENSION_INCLUDE_NEXT) || TPP_CONFIG_EXTENSION_INCLUDE_NEXT */
#if !defined(TPP_CONFIG_EXTENSION_IMPORT) || TPP_CONFIG_EXTENSION_IMPORT
	case KWD_tpp_directive_import:
#endif /* !defined(TPP_CONFIG_EXTENSION_IMPORT) || TPP_CONFIG_EXTENSION_IMPORT */
#if !defined(TPP_CONFIG_EXTENSION_WARNING) || TPP_CONFIG_EXTENSION_WARNING
	case KWD_tpp_directive_warning:
#endif /* !defined(TPP_CONFIG_EXTENSION_WARNING) || TPP_CONFIG_EXTENSION_WARNING */
#if !defined(TPP_CONFIG_EXTENSION_LXOR) || TPP_CONFIG_EXTENSION_LXOR
	case KWD_tpp_lxor:
#endif /* !defined(TPP_CONFIG_EXTENSION_LXOR) || TPP_CONFIG_EXTENSION_LXOR */
	case KWD_tpp_token_tilde_tilde:
	case KWD_tpp_token_pow:
	case KWD_tpp_token_lxor:
	case KWD_tpp_token_arrow:
	case KWD_tpp_token_colon_assign:
	case KWD_tpp_token_colon_colon:
#if !defined(TPP_CONFIG_EXTENSION_ALTMAC) || TPP_CONFIG_EXTENSION_ALTMAC
	case KWD_tpp_macro_calling_conventions:
#endif /* !defined(TPP_CONFIG_EXTENSION_ALTMAC) || TPP_CONFIG_EXTENSION_ALTMAC */
	case KWD_tpp_strict_whitespace:
	case KWD_tpp_strict_integer_overflow:
	case KWD_tpp_support_ansi_characters:
	case KWD_tpp_emit_lf_after_directive:
#if !defined(TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR) || TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR
	case KWD_tpp_if_cond_expression:
#endif /* !defined(TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR) || TPP_CONFIG_EXTENSION_IFELSE_IN_EXPR */
	case KWD_tpp_debug:
#endif /* !TPP_CONFIG_FASTSTARTUP_KEYWORD_FLAGS */

	case KWD_this:
		return false;

	default: break;
	}

	/* Default case: the builtin keyword is reserved. */
	return true;
}


INTERN WUNUSED NONNULL((2)) struct symbol *DCALL
lookup_symbol(unsigned int mode, struct TPPKeyword *__restrict name,
              struct ast_loc *warn_loc) {
	struct symbol *result, **bucket;
	DeeScopeObject *iter = current_scope;
	ASSERT(iter != NULL);
	/* Warn if a reserved name is used for a symbol. */
	if (is_reserved_symbol_name(name) &&
	    WARNAT(warn_loc, W_RESERVED_SYMBOL_NAME, name))
		goto err;
	if ((mode & LOOKUP_SYM_VMASK) == LOOKUP_SYM_VLOCAL) {
		/* Only lookup variables in the current scope. */
seach_single:
		ASSERT(iter->s_mapc <= iter->s_mapa);
		result = NULL;
		if (iter->s_mapa) {
			result = iter->s_map[name->k_id % iter->s_mapa];
			while (result && result->s_name != name)
				result = result->s_next;
			/* Simple case: If the variable was found, return it. */
			if (result) {
				if (mode & LOOKUP_SYM_STACK) {
					if (result->s_type != SYMBOL_TYPE_STACK &&
					    WARNAT(warn_loc, W_EXPECTED_STACK_VARIABLE, result))
						goto err;
				}
				if (mode & LOOKUP_SYM_STATIC) {
					if (result->s_type != SYMBOL_TYPE_STATIC &&
					    WARNAT(warn_loc, W_EXPECTED_STATIC_VARIABLE, result))
						goto err;
				}
				if ((mode & LOOKUP_SYM_ALLOWDECL) && SYMBOL_IS_WEAK(result)) {
					/* Re-declare this symbol. */
					SYMBOL_CLEAR_WEAK(result);
					if (mode & LOOKUP_SYM_FINAL) {
						result->s_flag |= SYMBOL_FFINAL;
						if (mode & LOOKUP_SYM_VARYING)
							result->s_flag |= SYMBOL_FVARYING;
					}
					if ((mode & LOOKUP_SYM_VGLOBAL) ||
					    ((mode & LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT &&
					     /* Default variable lookup in the root scope creates global variables. */
					     current_scope == (DeeScopeObject *)current_rootscope)) {
						result->s_type         = SYMBOL_TYPE_GLOBAL;
						result->s_global.g_doc = NULL;
					} else if (mode & LOOKUP_SYM_STACK) {
						result->s_type = SYMBOL_TYPE_STACK;
					} else if (mode & LOOKUP_SYM_STATIC) {
						result->s_type = SYMBOL_TYPE_STATIC;
					} else {
						result->s_type = SYMBOL_TYPE_LOCAL;
					}
					return result;
				}
				SYMBOL_MARK_USED(result);
				return result;
			}
		}
		goto create_variable;
	} else if ((mode & LOOKUP_SYM_VMASK) == LOOKUP_SYM_VGLOBAL) {
		/* Only lookup variables in the root scope. */
		iter = (DeeScopeObject *)current_rootscope;
		goto seach_single;
	}
	do {
		/* Look through all scopes. */
		ASSERT(iter->s_mapc <= iter->s_mapa);
		if (iter->s_mapa) {
			result = iter->s_map[name->k_id % iter->s_mapa];
			while (result && result->s_name != name)
				result = result->s_next;
			if (result) {
				SYMBOL_MARK_USED(result);
				return result;
			}
		}
		if (DeeScope_IsClassScope(iter)) {
			/* Reached a class scope.
			 * In order to allow for forward symbol declarations in class declarations,
			 * we must remember that this is where the symbol was first used, and perform
			 * the final linking when the class has been fully declared, at which point
			 * the symbol has either been re-declared as a member of the class, or will
			 * be linked to other declarations that may be visible outside of the class. */
			if unlikely((result = sym_alloc()) == NULL)
				goto err;
			bzero(result, sizeof(*result));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
			result->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
#if DAST_NONE != 0
			result->s_decltype.da_type = DAST_NONE;
#endif /* DAST_NONE != 0 */
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
			result->s_name = name;
			result->s_type = SYMBOL_TYPE_FWD;
			goto add_result_to_iter;
		}
	} while ((iter = iter->s_prev) != NULL);
create_variable:
	if (!(mode & LOOKUP_SYM_ALLOWDECL) &&
	    WARNAT(warn_loc, W_UNKNOWN_VARIABLE, name->k_name))
		goto err;
	if ((mode & LOOKUP_SYM_VGLOBAL) &&
	    current_scope != (DeeScopeObject *)current_rootscope &&
	    WARNAT(warn_loc, W_DECLARING_GLOBAL_IN_NONROOT, name))
		goto err;
	/* Warn if a new variable is declared implicitly outside the global scope. */
	if (!(mode & LOOKUP_SYM_VMASK) &&
	    current_scope != (DeeScopeObject *)current_rootscope &&
	    WARNAT(warn_loc, W_DECLARING_IMPLICIT_VARIABLE, name))
		goto err;

	/* Create a new symbol. */
	if unlikely((result = sym_alloc()) == NULL)
		goto err;
	bzero(result, sizeof(*result));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
	result->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
#if DAST_NONE != 0
	result->s_decltype.da_type = DAST_NONE;
#endif /* DAST_NONE != 0 */
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	result->s_name  = name;
	result->s_scope = current_scope;
	result->s_type  = SYMBOL_TYPE_FWD;
	if (mode & LOOKUP_SYM_FINAL) {
		result->s_flag |= SYMBOL_FFINAL;
		if (mode & LOOKUP_SYM_VARYING)
			result->s_flag |= SYMBOL_FVARYING;
	}
	if ((mode & LOOKUP_SYM_VGLOBAL) ||
	    ((mode & LOOKUP_SYM_VMASK) == LOOKUP_SYM_VDEFAULT &&
	     /* Default variable lookup in the root scope creates global variables. */
	     current_scope == (DeeScopeObject *)current_rootscope)) {
		result->s_type = SYMBOL_TYPE_GLOBAL;
		ASSERT(result->s_global.g_doc == NULL);
		iter = (DeeScopeObject *)current_rootscope;
	} else {
		iter = current_scope;
		if (mode & LOOKUP_SYM_STACK) {
			result->s_type = SYMBOL_TYPE_STACK;
		} else if (mode & LOOKUP_SYM_STATIC) {
			result->s_type = SYMBOL_TYPE_STATIC;
		} else {
			result->s_type = SYMBOL_TYPE_LOCAL;
		}
	}
add_result_to_iter:
	if (++iter->s_mapc > iter->s_mapa) {
		/* Must rehash this scope. */
		if unlikely(rehash_scope(iter))
			goto err_r;
	}
	/* Insert the new symbol. */
	ASSERT(iter->s_mapa != 0);
	bucket          = &iter->s_map[name->k_id % iter->s_mapa];
	result->s_next  = *bucket;
	*bucket         = result;
	result->s_scope = iter;
	if (warn_loc) {
		if (!tpp_is_reachable_file(warn_loc->l_file))
			goto set_default_location;
		memcpy(&result->s_decl,
		       warn_loc,
		       sizeof(struct ast_loc));
	} else {
set_default_location:
		loc_here(&result->s_decl);
	}
	if (result->s_decl.l_file)
		TPPFile_Incref(result->s_decl.l_file);
	return result;
err_r:
	--iter->s_mapc;
	sym_free(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((2)) struct symbol *DCALL
lookup_nth(unsigned int nth, struct TPPKeyword *__restrict name) {
	DeeScopeObject *iter;
	/* Make sure to return `NULL' when `nth' was zero. */
	if unlikely(!nth--)
		goto nope;
	iter = current_scope;
	for (; iter; iter = iter->s_prev) {
		struct symbol *result;
		if (!iter->s_mapa)
			continue;
		result = iter->s_map[name->k_id % iter->s_mapa];
		while (result) {
			if (result->s_name == name) {
				/* Return this instance if it is the one that was requested. */
				if (!nth--) {
					SYMBOL_MARK_USED(result);
					return result;
				}
				break;
			}
			result = result->s_next;
		}
	}
nope:
	return NULL;
}


INTERN WUNUSED NONNULL((1)) struct symbol *DCALL
new_local_symbol(struct TPPKeyword *__restrict name, struct ast_loc *loc) {
	struct symbol *result, **bucket;
	result = sym_alloc();
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(struct symbol));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
	result->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
	result->s_name = name;
	if (++current_scope->s_mapc > current_scope->s_mapa) {
		if unlikely(rehash_scope(current_scope))
			goto err_r;
	}
	ASSERT(current_scope->s_mapa != 0);
	bucket         = &current_scope->s_map[name->k_id % current_scope->s_mapa];
	result->s_next = *bucket;
	*bucket        = result;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	result->s_decltype.da_type = DAST_NONE;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	result->s_flag   = SYMBOL_FNORMAL;
	result->s_nread  = 0;
	result->s_nwrite = 0;
	result->s_nbound = 0;
	result->s_scope  = current_scope;
	if (loc) {
		if (!tpp_is_reachable_file(loc->l_file))
			goto set_default_location;
		memcpy(&result->s_decl, loc, sizeof(struct ast_loc));
	} else {
set_default_location:
		loc_here(&result->s_decl);
	}
	if (result->s_decl.l_file)
		TPPFile_Incref(result->s_decl.l_file);
	return result;
err_r:
	--current_scope->s_mapc;
	sym_free(result);
err:
	return NULL;
}

INTERN WUNUSED struct symbol *DCALL new_unnamed_symbol(void) {
	struct symbol *result;
	result = sym_alloc();
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(struct symbol));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
	result->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	result->s_decltype.da_type = DAST_NONE;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	result->s_name        = &TPPKeyword_Empty;
	result->s_next        = current_scope->s_del;
	current_scope->s_del  = result;
	result->s_flag        = SYMBOL_FNORMAL;
	result->s_nread       = 0;
	result->s_nwrite      = 0;
	result->s_nbound      = 0;
	result->s_scope       = current_scope;
	result->s_decl.l_file = NULL;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) struct symbol *DCALL
new_unnamed_symbol_in_scope(DeeScopeObject *__restrict scope) {
	struct symbol *result;
	result = sym_alloc();
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(struct symbol));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
	result->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	result->s_decltype.da_type = DAST_NONE;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	result->s_name        = &TPPKeyword_Empty;
	result->s_next        = scope->s_del;
	scope->s_del          = result;
	result->s_flag        = SYMBOL_FNORMAL;
	result->s_nread       = 0;
	result->s_nwrite      = 0;
	result->s_nbound      = 0;
	result->s_scope       = scope;
	result->s_decl.l_file = NULL;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) struct symbol *DCALL
new_local_symbol_in_scope(DeeScopeObject *__restrict scope,
                          struct TPPKeyword *__restrict name,
                          struct ast_loc *loc) {
	struct symbol *result, **bucket;
	result = sym_alloc();
	if unlikely(!result)
		goto err;
	DBG_memset(result, 0xcc, sizeof(struct symbol));
#ifdef CONFIG_SYMBOL_HAS_REFCNT
	result->s_refcnt = 1;
#endif /* CONFIG_SYMBOL_HAS_REFCNT */
	result->s_name = name;
	if (++scope->s_mapc > scope->s_mapa) {
		if unlikely(rehash_scope(scope))
			goto err_r;
	}
	ASSERT(scope->s_mapa != 0);
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	result->s_decltype.da_type = DAST_NONE;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	bucket           = &scope->s_map[name->k_id % scope->s_mapa];
	result->s_next   = *bucket;
	*bucket          = result;
	result->s_flag   = SYMBOL_FNORMAL;
	result->s_nread  = 0;
	result->s_nwrite = 0;
	result->s_nbound = 0;
	result->s_scope  = scope;
	if (loc) {
		if (!tpp_is_reachable_file(loc->l_file))
			goto set_default_location;
		memcpy(&result->s_decl, loc, sizeof(struct ast_loc));
	} else {
set_default_location:
		loc_here(&result->s_decl);
	}
	if (result->s_decl.l_file)
		TPPFile_Incref(result->s_decl.l_file);
	return result;
err_r:
	--scope->s_mapc;
	sym_free(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) struct symbol *DCALL
get_local_symbol_in_scope(DeeScopeObject *__restrict scope,
                          struct TPPKeyword *__restrict name) {
	struct symbol *bucket;
	if (!scope->s_mapc)
		return false;
	ASSERT(scope->s_mapa != 0);
	bucket = scope->s_map[name->k_id % scope->s_mapa];
	while (bucket && bucket->s_name != name)
		bucket = bucket->s_next;
	return bucket;
}

INTERN WUNUSED NONNULL((1)) struct symbol *DCALL
get_local_symbol(struct TPPKeyword *__restrict name) {
	struct symbol *bucket;
	if (!current_scope->s_mapc)
		return false;
	ASSERT(current_scope->s_mapa != 0);
	bucket = current_scope->s_map[name->k_id % current_scope->s_mapa];
	while (bucket && bucket->s_name != name)
		bucket = bucket->s_next;
	return bucket;
}

INTERN NONNULL((1)) void DCALL
del_local_symbol(struct symbol *__restrict sym) {
	struct symbol **p_bucket, *bucket;
	ASSERT(sym->s_name != &TPPKeyword_Empty);
	ASSERT(sym->s_scope->s_mapa != 0);
	p_bucket = &sym->s_scope->s_map[sym->s_name->k_id % sym->s_scope->s_mapa];
	while ((bucket = *p_bucket, bucket && bucket != sym))
		p_bucket = &bucket->s_next;
	if (!bucket)
		return; /* Shouldn't happen, but ignore (could happen if the symbol is deleted twice) */

	/* Unlink the symbol from the bucket list. */
	*p_bucket = sym->s_next;

	/* Add the symbol to the chain of deleted (anonymous) symbols. */
	sym->s_next         = sym->s_scope->s_del;
	sym->s_scope->s_del = sym;
}

INTERN WUNUSED NONNULL((1, 2)) struct symbol *DCALL
scope_lookup(DeeScopeObject *__restrict scope,
             struct TPPKeyword *__restrict name) {
	struct symbol *result = NULL;
	if (!scope->s_mapa)
		goto done;
	result = scope->s_map[name->k_id % scope->s_mapa];
	while (result && result->s_name != name)
		result = result->s_next;
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) struct symbol *DCALL
scope_lookup_str(DeeScopeObject *__restrict scope,
                 char const *__restrict name,
                 size_t name_length) {
	struct symbol *result = NULL;
	struct TPPKeyword *keyword;
	if (!scope->s_mapa)
		goto done;
	keyword = TPPLexer_LookupKeyword(name, name_length, 0);
	if (!keyword)
		goto done;
	result = scope->s_map[keyword->k_id % scope->s_mapa];
	while (result && result->s_name != keyword)
		result = result->s_next;
done:
	return result;
}


PRIVATE WUNUSED int DCALL rehash_labels(void) {
	struct text_label **new_map, **biter, **bend;
	struct text_label *lbl_iter, *s_next, **bucket;
	size_t old_size = current_basescope->bs_lbla;
	size_t new_size = old_size;
	if (!new_size)
		new_size = 1;
	new_size *= 2;
rehash_realloc:
	new_map = (struct text_label **)Dee_TryCallocc(new_size, sizeof(struct text_label *));
	if unlikely(!new_map) {
		if (new_size != 1) {
			new_size = 1;
			goto rehash_realloc;
		}
		if (Dee_CollectMemory(sizeof(struct text_label *)))
			goto rehash_realloc;
		return -1;
	}
	/* Rehash all text_labels. */
	biter = current_basescope->bs_lbl;
	bend  = biter + old_size;
	for (; biter < bend; ++biter) {
		lbl_iter = *biter;
		while (lbl_iter) {
			s_next            = lbl_iter->tl_next;
			bucket            = &new_map[lbl_iter->tl_name->k_id % new_size];
			lbl_iter->tl_next = *bucket;
			*bucket           = lbl_iter;
			lbl_iter          = s_next;
		}
	}
	Dee_Free(current_basescope->bs_lbl);
	current_basescope->bs_lbl  = new_map;
	current_basescope->bs_lbla = new_size;
	return 0;
}

INTERN WUNUSED NONNULL((1)) struct text_label *DCALL
lookup_label(struct TPPKeyword *__restrict name) {
	struct text_label *result, **p_result;
	if likely(current_basescope->bs_lbla) {
		result = current_basescope->bs_lbl[name->k_id % current_basescope->bs_lbla];
		while (result) {
			if (result->tl_name == name)
				return result;
			result = result->tl_next;
		}
	}
	if (current_basescope->bs_lblc >= current_basescope->bs_lbla) {
		if (rehash_labels())
			goto err;
	}
	ASSERT(current_basescope->bs_lbla);
	p_result = &current_basescope->bs_lbl[name->k_id % current_basescope->bs_lbla];
	result   = lbl_alloc();
	if unlikely(!result)
		goto err;
	result->tl_next = *p_result;
	result->tl_name = name;
	result->tl_asym = NULL;
	result->tl_goto = 0;
	*p_result       = result;
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) struct text_label *DCALL
new_case_label(struct ast *__restrict expr) {
	struct text_label *result;
	ASSERTF(current_basescope->bs_cflags & BASESCOPE_FSWITCH,
	        "Not inside a switch statement");
	ASSERT_AST(expr);
	result = lbl_alloc();
	if unlikely(!result)
		goto done;
	/* Initialize the new label. */
	ast_incref(expr);
	result->tl_expr = expr;
	result->tl_asym = NULL;
	result->tl_goto = 0;
	/* Add the case to the currently active linked list of them. */
	result->tl_next              = current_basescope->bs_swcase;
	current_basescope->bs_swcase = result;
done:
	return result;
}

INTERN WUNUSED struct text_label *DCALL
new_default_label(void) {
	struct text_label *result;
	ASSERTF(current_basescope->bs_cflags & BASESCOPE_FSWITCH,
	        "Not inside a switch statement");
	result = current_basescope->bs_swdefl;
	if unlikely(result)
		goto done; /* Unlikely, considering actually valid use cases. */
	result = lbl_alloc();
	if unlikely(!result)
		goto done;
	/* Initialize the new label. */
	result->tl_next = NULL;
	result->tl_expr = NULL;
	result->tl_asym = NULL;
	result->tl_goto = 0;
	/* Set the new label as the default label. */
	current_basescope->bs_swdefl = result;
done:
	return result;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_SYMBOL_H */
