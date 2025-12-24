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
#ifndef GUARD_DEEMON_COMPILER_LEXER_CLASS_C
#define GUARD_DEEMON_COMPILER_LEXER_CLASS_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/lexer.h>
#include <deemon/compiler/symbol.h>
#include <deemon/compiler/tpp.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memset(), bzero(), ... */
#include <deemon/tuple.h>

#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
#include <deemon/compiler/doctext.h>
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */

#include "../../runtime/strings.h"
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint32_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

INTDEF int DCALL skip_lf(void);
#define is_semicolon() (tok == ';' || tok == '\n')
PRIVATE tok_t DCALL yield_semicolon(void) {
	tok_t result = yield();
	if (result == '\n') {
		uint32_t old_flags;
		old_flags = TPPLexer_Current->l_flags;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		result = yield();
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	}
	return result;
}


INTDEF struct class_operator empty_class_operators[];
INTDEF struct class_attribute empty_class_attributes[];

struct class_maker {
	DREF struct ast           *cm_base;        /* [0..1] An AST evaluating to the base of the class. */
	DREF DeeClassDescriptorObject *cm_desc;    /* [1..1] The descriptor for the class. */
	size_t                     cm_iattr_size;  /* Number of used slots in `cm_desc->cd_iattr_list' */
	size_t                     cm_cattr_size;  /* Number of used slots in `cm_desc->cd_cattr_list' */
	Dee_operator_t             cm_clsop_size;  /* Number of used slots in `cm_desc->cd_clsop_list' */
	uint16_t                   cm_null_member; /* The address of a class member that is always unbound (used for
	                                            * deleted operator), or `(uint16_t)-1' when no such address has
	                                            * yet to be designated. */
#define CLASS_MAKER_CTOR_FNORMAL    0x0000     /* Normal constructor flags. */
#define CLASS_MAKER_CTOR_FDELETED   0x0001     /* The constructor has been deleted. */
#define CLASS_MAKER_CTOR_FSUPER     0x0002     /* The constructor has explicitly been inherited from the super-type. */
#define CLASS_MAKER_CTOR_FDEFAULT   0x0004     /* The constructor has explicitly been defined to be default-implemented. */
#define CLASS_MAKER_CTOR_FSUPERKWDS 0x0008     /* The superargs operator returns an (args, kwds) pair. */
	uint16_t                   cm_ctor_flags;  /* Special flags concerning the constructor (Set of `CLASS_MAKER_CTOR_F*') */
#define CLASS_MAKER_FEAT_FNORMAL    0x0000     /* Normal class features. */
#define CLASS_MAKER_FEAT_FATTROPS   0x0001     /* The class is defining attribute operators */
#define CLASS_MAKER_FEAT_FNOFINPUB  0x0002     /* The class contains at least one non-final, public member */
	uint16_t                   cm_features;    /* Encountered class features (only used for warnings; set of `CLASS_MAKER_FEAT_F*') */
	DREF struct ast           *cm_ctor;        /* [0..1][(!= NULL) == (cm_ctor_scope != NULL) == (cm_initc != 0)]
	                                            * The class's constructor function AST (AST_FUNCTION) */
	DREF DeeBaseScopeObject   *cm_ctor_scope;  /* [0..1][(!= NULL) == (cm_ctor != NULL) == (cm_initc != 0)] (lazy-alloc)
	                                            * The base-scope in which member initializers and the constructor are executed. */
	struct unicode_printer     cm_doc;         /* A string printer for the documentation of this class. */
	size_t                     cm_initc;       /* [(!= 0) == (cm_ctor_scope != NULL) == (cm_ctor != NULL)]
	                                            * Amount of ASTs executed before the actual constructor. */
	size_t                     cm_inita;       /* [>= cm_initc] Allocated amount of ASTs executed before the actual constructor. */
	DREF struct ast          **cm_initv;       /* [0..cm_initc|ALLOC(cm_inita)][owned][[*]->a_scope == cm_ctor_scope]
	                                            * Vector of ASTs executed before the actual constructor.
	                                            * NOTE: Each of these asts is generated in the context of the `cm_ctor_scope' scope. */
	size_t                     cm_class_initc; /* Amount of class member initializers. */
	size_t                     cm_class_inita; /* Allocate amount of class member initializers. */
	struct class_member       *cm_class_initv; /* [0..cm_class_initc|ALLOC(cm_class_inita)][owned]
	                                            * Vector of class member initializers.
	                                            * This contains stuff like creation of operator callbacks and
	                                            * instance methods using the `CLASS_ATTRIBUTE_FCLASSMEM' flag. */
	struct symbol             *cm_classsym;    /* [1..1] The symbol describing the class in the scope it is defined in.
	                                            * This symbol is assigned in the base scope of every member
	                                            * function/operator that is parsed. */
	struct symbol             *cm_supersym;    /* [0..1] Same as `cm_classsym', but instead describes the class's super-class. */
	struct symbol             *cm_thissym;     /* [1..1] The this-symbol associated with `cm_classsym'. */
};


PRIVATE NONNULL((1, 2)) void DCALL
relocate_attribute(struct class_attribute *__restrict old_attr,
                   struct class_attribute *__restrict new_attr) {
	struct symbol **biter, **bend, *iter;
	ASSERT(DeeScope_IsClassScope(current_scope));
	biter = current_scope->s_map;
	bend  = biter + current_scope->s_mapa;
	for (; biter < bend; ++biter) {
		for (iter = *biter; iter; iter = iter->s_next) {
			if (iter->s_type == SYMBOL_TYPE_CATTR &&
			    iter->s_attr.a_attr == old_attr)
				iter->s_attr.a_attr = new_attr;
		}
	}
	for (iter = current_scope->s_del; iter; iter = iter->s_next) {
		if (iter->s_type == SYMBOL_TYPE_CATTR &&
		    iter->s_attr.a_attr == old_attr)
			iter->s_attr.a_attr = new_attr;
	}
}

#define class_attribute_is_nonfinal_public(ca_flag)                          \
	(((ca_flag) & (CLASS_ATTRIBUTE_FFINAL | CLASS_ATTRIBUTE_FVISIBILITY)) == \
	 /*        */ (CLASS_ATTRIBUTE_FPUBLIC))

PRIVATE WUNUSED NONNULL((1)) struct class_attribute *DCALL
cdesc_find_nonfinal_public_symbol(DeeClassDescriptorObject *__restrict self) {
	size_t i;
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		struct class_attribute *attr;
		attr = &self->cd_iattr_list[i];
		if (!attr->ca_name)
			continue;
		if (!class_attribute_is_nonfinal_public(attr->ca_flag))
			continue;
		return attr;
	}
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
warn_nonfinal_public_with_attr_operators(struct ast_loc *__restrict loc,
                                         char const *__restrict attr_name,
                                         uint16_t attr_flags) {
	char const *category_name;
	if (attr_flags & CLASS_ATTRIBUTE_FGETSET) {
		category_name = "property";
	} else if (attr_flags & CLASS_ATTRIBUTE_FMETHOD) {
		category_name = STR_function;
	} else {
		category_name = "member";
	}
	return WARNAT(loc, W_ATTR_OPERATOR_WITH_NONFINAL_PUBLIC_MEMBER,
	              category_name, attr_name);
}


LOCAL WUNUSED NONNULL((1)) int DCALL
rehash_class_attributes(DeeClassDescriptorObject *__restrict self) {
	size_t new_mask, i, j, perturb;
	struct class_attribute *new_table;
	new_mask = (self->cd_cattr_mask << 1) | 1;
	if (new_mask <= 1)
		new_mask = 7;
	new_table = (struct class_attribute *)Dee_Callocc(new_mask + 1,
	                                                  sizeof(struct class_attribute));
	if unlikely(!new_table)
		goto err;

	/* Rehash all of the old attributes. */
	for (i = 0; i <= self->cd_cattr_mask; ++i) {
		struct class_attribute *old_attr, *new_attr;
		old_attr = &self->cd_cattr_list[i];
		if (!old_attr->ca_name)
			continue;

		/* Relocate this attribute. */
		j = perturb = old_attr->ca_hash & new_mask;
		for (;; DeeClassDescriptor_IATTRNEXT(j, perturb)) {
			new_attr = &new_table[j & new_mask];
			if (!new_attr->ca_name)
				break;
		}
		memcpy(new_attr, old_attr, sizeof(struct class_attribute));
		relocate_attribute(old_attr, new_attr);
	}

	/* Install the new table and mask. */
	if (self->cd_cattr_list != empty_class_attributes)
		Dee_Free(self->cd_cattr_list);
	self->cd_cattr_list = new_table;
	self->cd_cattr_mask = new_mask;
	return 0;
err:
	return -1;
}

LOCAL WUNUSED NONNULL((1)) DREF DeeClassDescriptorObject *DCALL
rehash_instance_attributes(DREF DeeClassDescriptorObject *__restrict self) {
	size_t new_mask, i, j, perturb;
	DREF DeeClassDescriptorObject *new_descr;
	new_mask = (self->cd_iattr_mask << 1) | 1;
#if 0 /* The instance-attribute table is pre-initialized
       * to a mask of `7', so this would never trigger. */
	if (new_mask <= 1)
		new_mask = 7;
#endif
	new_descr = (DeeClassDescriptorObject *)DeeObject_Callocc(offsetof(DeeClassDescriptorObject, cd_iattr_list),
	                                                          new_mask + 1, sizeof(struct class_attribute));
	if unlikely(!new_descr)
		goto err;

	/* Rehash all of the old attributes. */
	for (i = 0; i <= self->cd_iattr_mask; ++i) {
		struct class_attribute *old_attr, *new_attr;
		old_attr = &self->cd_iattr_list[i];
		if (!old_attr->ca_name)
			continue;

		/* Relocate this attribute. */
		j = perturb = old_attr->ca_hash & new_mask;
		for (;; DeeClassDescriptor_IATTRNEXT(j, perturb)) {
			new_attr = &new_descr->cd_iattr_list[j & new_mask];
			if (!new_attr->ca_name)
				break;
		}
		memcpy(new_attr, old_attr, sizeof(struct class_attribute));
		relocate_attribute(old_attr, new_attr);
	}

	/* Copy the entire header from the old descriptor (thus stealing all of its data) */
	memcpy(new_descr, self, offsetof(DeeClassDescriptorObject, cd_iattr_list));

	/* Remember the (now) larger mask. */
	new_descr->cd_iattr_mask = new_mask;

	/* Free the old descriptor. */
	DeeObject_Free(self);
	return new_descr;
err:
	return NULL;
}

LOCAL WUNUSED NONNULL((1)) int DCALL
rehash_operator_bindings(DeeClassDescriptorObject *__restrict self) {
	uint16_t new_mask, i, j, perturb;
	struct class_operator *new_table;
	new_mask = (self->cd_clsop_mask << 1) | 1;
	if (new_mask <= 1)
		new_mask = 3;
	new_table = (struct class_operator *)Dee_Mallocc(new_mask + 1,
	                                                 sizeof(struct class_operator));
	if unlikely(!new_table)
		goto err;

	/* Fill the new table with all unused entries. */
	memset(new_table, 0xff,
	       (new_mask + 1) *
	       sizeof(struct class_operator));

	/* Rehash all pre-existing bindings. */
	for (i = 0; i <= self->cd_clsop_mask; ++i) {
		struct class_operator *op, *new_op;
		op = &self->cd_clsop_list[i];
		if (op->co_name == (Dee_operator_t)-1)
			continue; /* Unused entry. */

		/* Insert the entry into the new table. */
		j = perturb = op->co_name & new_mask;
		for (;; DeeClassDescriptor_CLSOPNEXT(j, perturb)) {
			new_op = &new_table[j & new_mask];
			if (new_op->co_name == (Dee_operator_t)-1)
				break;
		}
		memcpy(new_op, op, sizeof(struct class_operator));
	}

	/* Install the new table and mask. */
	if (self->cd_clsop_list != empty_class_operators)
		Dee_Free(self->cd_clsop_list);
	self->cd_clsop_list = new_table;
	self->cd_clsop_mask = new_mask;
	return 0;
err:
	return -1;
}

/* Allocate a new class- or instance-attribute. */
PRIVATE WUNUSED NONNULL((1, 2, 3)) struct class_attribute *DCALL
class_maker_newcattr(struct class_maker *__restrict self,
                     DeeStringObject *__restrict name,
                     struct ast_loc *__restrict loc) {
	Dee_hash_t i, perturb, hash;
	struct class_attribute *result;
	DeeClassDescriptorObject *desc = self->cm_desc;
	if ((desc->cd_cattr_mask == 0 ||
	     (self->cm_cattr_size > (desc->cd_cattr_mask / 3) * 2)) &&
	    unlikely(rehash_class_attributes(desc)))
		goto err;
	hash = DeeString_Hash(name);
	i = perturb = hash & desc->cd_cattr_mask;

	/* Search for a pre-existing matching attribute, or create a new one. */
	for (;; DeeClassDescriptor_CATTRNEXT(i, perturb)) {
		result = &desc->cd_cattr_list[i & desc->cd_cattr_mask];
		if (!result->ca_name)
			break; /* Unused entry. */

		/* Make sure that the member doesn't already exist! */
		if (result->ca_hash != hash)
			continue;
		if (DeeString_SIZE(result->ca_name) !=
		    DeeString_SIZE(name))
			continue;
		if (bcmpc(DeeString_STR(result->ca_name),
		          DeeString_STR(name),
		          DeeString_SIZE(name),
		          sizeof(char)) != 0)
			continue;

		/* Duplicate name */
		if (WARNAT(loc, W_CLASS_MEMBER_ALREADY_DEFINED,
		           DeeString_SIZE(name),
		           DeeString_STR(name)))
			goto err;
		Dee_Decref(result->ca_name);
		Dee_XClear(result->ca_doc);
		break;
	}
	++self->cm_cattr_size;
	result->ca_name = name;
	result->ca_hash = hash;
	Dee_Incref(name);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct class_attribute *DCALL
class_maker_newiattr(struct class_maker *__restrict self,
                     DeeStringObject *__restrict name,
                     struct ast_loc *__restrict loc) {
	Dee_hash_t i, perturb, hash;
	struct class_attribute *result;
	DeeClassDescriptorObject *desc = self->cm_desc;
	if ((desc->cd_iattr_mask == 0) ||
	    (self->cm_iattr_size > (desc->cd_iattr_mask / 3) * 2)) {
		desc = rehash_instance_attributes(desc);
		if unlikely(!desc)
			goto err;
		self->cm_desc = desc; /* Replace reference. */
	}
	hash = DeeString_Hash(name);
	i = perturb = hash & desc->cd_iattr_mask;

	/* Search for a pre-existing matching attribute, or create a new one. */
	for (;; DeeClassDescriptor_IATTRNEXT(i, perturb)) {
		result = &desc->cd_iattr_list[i & desc->cd_iattr_mask];
		if (!result->ca_name)
			break; /* Unused entry. */

		/* Make sure that the member doesn't already exist! */
		if (result->ca_hash != hash)
			continue;
		if (DeeString_SIZE(result->ca_name) !=
		    DeeString_SIZE(name))
			continue;
		if (bcmpc(DeeString_STR(result->ca_name),
		          DeeString_STR(name),
		          DeeString_SIZE(name),
		          sizeof(char)) != 0)
			continue;

		/* Duplicate name */
		if (WARNAT(loc, W_CLASS_MEMBER_ALREADY_DEFINED,
		           DeeString_SIZE(name),
		           DeeString_STR(name)))
			goto err;
		Dee_Decref(result->ca_name);
		Dee_XClear(result->ca_doc);
		break;
	}
	++self->cm_iattr_size;
	result->ca_name = name;
	result->ca_hash = hash;
	Dee_Incref(name);
	return result;
err:
	return NULL;
}

/* Bind operator `name' to a function stored
 * in the class member table under `addr' */
PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
class_maker_bindoperator(struct class_maker *__restrict self,
                         Dee_operator_t name, uint16_t addr,
                         struct ast_loc *__restrict loc) {
	Dee_operator_t i, perturb;
	struct class_operator *result;
	DeeClassDescriptorObject *desc = self->cm_desc;
	if ((desc->cd_clsop_mask == 0 ||
	     (self->cm_clsop_size > (desc->cd_clsop_mask / 3) * 2)) &&
	    unlikely(rehash_operator_bindings(desc)))
		goto err;
	i = perturb = name & desc->cd_clsop_mask;

	/* Search for a pre-existing matching attribute, or create a new one. */
	for (;; DeeClassDescriptor_CLSOPNEXT(i, perturb)) {
		result = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
		if (result->co_name == (Dee_operator_t)-1)
			break; /* Unused entry. */
		if (result->co_name != name)
			continue;
		if (result->co_addr != addr) {
			/* Warn about a duplicate operator */
			struct opinfo const *info = DeeTypeType_GetOperatorById(&DeeType_Type, name);
			if (WARNAT(loc, W_OPERATOR_WAS_ALREADY_DEFINED,
			           info ? info->oi_sname : "?"))
				goto err;
		}
		result->co_addr = addr; /* Overwrite the old binding. */
		return 0;
	}
	++self->cm_clsop_size;
	result->co_name = name;
	result->co_addr = addr;
	return 0;
err:
	return -1;
}


/* Similar to `class_maker_push_ctorscope()', but pushes a basescope
 * that should be used for an other operator or member-method other
 * than the constructor.
 * Upon success (0), the scope can later be popped using `basescope_pop()'. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
class_maker_push_methscope(struct class_maker *__restrict self) {
	if (basescope_push())
		goto err;

	/* Set the appropriate flags to turn this one into a class-scope. */
	current_basescope->bs_flags |= CODE_FTHISCALL;
	current_basescope->bs_flags |= current_tags.at_code_flags;
	current_basescope->bs_this = self->cm_thissym;
	return 0;
err:
	return -1;
}

/* Push the constructor-scope of the given class-maker.
 * Upon success (0), the scope can later be popped using `basescope_pop()'.
 * The constructor scope must be active when member initializers
 * and (obviously) the constructor operator are being parsed. */
PRIVATE WUNUSED NONNULL((1)) int DCALL
class_maker_push_ctorscope(struct class_maker *__restrict self) {
	if (self->cm_ctor_scope) {
		/* The constructor scope already exists. */
		basescope_push_ob(self->cm_ctor_scope);
		return 0;
	}

	/* Must create a new constructor-scope. */
	if unlikely(class_maker_push_methscope(self))
		goto err;

	/* Now just set the constructor-flag in the scope. */
	current_basescope->bs_flags |= CODE_FCONSTRUCTOR;
	self->cm_ctor_scope = current_basescope;
	Dee_Incref((DeeObject *)current_basescope);
	return 0;
err:
	return -1;
}

/* Check if an initializer for a given symbol must be processed in the
 * context of the constructor scope (false) or the outside class scope (true) */
#define SYM_ISCLASSMEMBER(x)       \
	((x)->s_attr.a_this == NULL || \
	 ((x)->s_attr.a_attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))

/* Allocate a new class member, automatically assigning the next VTABLE
 * id, as well as creating symbols in the associated member tables and
 * the current scope that can be used to access the member.
 * HINT: This function also deals with accessing properties.
 * WARNING: Once done, the caller is required to increment `**pp_usage_counter'
 *          by the number of slots that are then being used by the member.
 * @param: name:             The name of the member that should be added.
 * @param: is_class_member:  `true', if the member should be added as a class-member.
 * @param: flags:            Set of `CLASS_ATTRIBUTE_F*'.
 * @param: pp_usage_counter: Filled with a pointer to the usage-counter which must be increment
 *                           by however-many consecutive VTABLE slots will be used by the member.
 * @return: * :              A new symbol classified as `SYM_CLASS_MEMBER' that is now stored in the caller's scope.
 *                           The symbol has already been fully initialized, including the `sym_member.sym_member'
 *                           field which contains the member descriptor apart of the either the `cm_cmem' or `cm_imem' table. */
PRIVATE WUNUSED NONNULL((1, 2, 5, 6)) struct symbol *DCALL
class_maker_addmember(struct class_maker *__restrict self,
                      struct TPPKeyword *__restrict name,
                      bool is_class_member,
                      uint16_t flags,
                      uint16_t **__restrict pp_usage_counter,
                      struct ast_loc *__restrict loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
                      , struct decl_ast *decl
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
                      ) {
	struct symbol *result;
	DREF DeeStringObject *name_str;
	struct class_attribute *attr;
	name_str = (DREF DeeStringObject *)DeeString_NewSized(name->k_name,
	                                                      name->k_size);
	if unlikely(!name_str)
		goto err;

	/* Add a new member entry to the specified table. */
	if (is_class_member) {
		attr = class_maker_newcattr(self, name_str, loc);
	} else {
		/* Warn if declaring an attribute operator with non-final, public members. */
		if (class_attribute_is_nonfinal_public(flags)) {
			if ((self->cm_features & CLASS_MAKER_FEAT_FATTROPS) &&
			    warn_nonfinal_public_with_attr_operators(loc, DeeString_STR(name_str), flags)) {
				Dee_Decref(name_str);
				goto err;
			}
			self->cm_features |= CLASS_MAKER_FEAT_FNOFINPUB;
		}
		attr = class_maker_newiattr(self, name_str, loc);
	}
	Dee_Decref(name_str);
	if unlikely(!attr)
		goto err;

	/* Add a documentation string to the member. */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	if (decl)
#else /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	if (!UNICODE_PRINTER_ISEMPTY(&current_tags.at_doc))
#endif /* !CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	{
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
		/* TODO: The creation of doc strings must be prolonged
		 *       until the entire class is being finalized.
		 *    -> Only after FORWARD symbols have been resolved
		 *       can we actually encode type annotations.
		 * XXX: What about recursive class declarations?
		 *      we'd have to wait until _all_ classes have
		 *      been fully declared, at which point we'd
		 *      have to wait much longer, presumably until
		 *      code starts getting assembled. */
		if unlikely(doctext_compile(&current_tags.at_doc))
			goto err;
		attr->ca_doc = (DREF DeeStringObject *)ast_tags_doc(decl);
#else /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
		attr->ca_doc = (DREF DeeStringObject *)ast_tags_doc();
#endif /* !CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
		if unlikely(!attr->ca_doc)
			goto err;
	}

	attr->ca_flag = flags;
	/* NOTE: Members apart of the instance member vector, but stored
	 *       in the class member vector must obvious count to its usage,
	 *       rather than their own. */
	if ((flags & CLASS_ATTRIBUTE_FCLASSMEM) || is_class_member) {
		*pp_usage_counter = &self->cm_desc->cd_cmemb_size;
	} else {
		*pp_usage_counter = &self->cm_desc->cd_imemb_size;
	}
	{
		size_t addr = **pp_usage_counter;
		/* -2 because 2 == 3-1 and 3 is the max number of slots required for a property */
		if unlikely(addr > UINT16_MAX - 2) {
			PERRAT(loc, W_TOO_MANY_CLASS_MEMBER,
			       self->cm_classsym->s_name->k_name);
			goto err;
		}
		/* Save the starting VTABLE address of this member. */
		attr->ca_addr = (uint16_t)addr;
	}

	/* Make sure that no local symbol exists with the given name.
	 * >> This is the compile-time portion of addressing symbols. */
	result = get_local_symbol(name);
	if unlikely(result) {
		if (SYMBOL_IS_WEAK(result)) {
			SYMBOL_CLEAR_WEAK(result);
		} else if (result->s_type == SYMBOL_TYPE_FWD) {
			/* Define a forward-referenced class symbol. */
		} else {
			PERRAT(loc, W_CLASS_MEMBER_ALREADY_DEFINED,
			       name->k_size, name->k_name);
			goto err;
		}
	} else {
		/* Create a new local symbol for this member. */
		result = new_local_symbol(name, loc);
		if unlikely(!result)
			goto err;
	}

	/* Steal declaration information. */
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	if (decl && result->s_decltype.da_type == DAST_NONE) {
		decl_ast_move(&result->s_decltype, decl);
		decl->da_type = DAST_NONE;
	}
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */

	/* Initialize that symbol to be a member symbol. */
	result->s_type         = SYMBOL_TYPE_CATTR;
	result->s_attr.a_class = self->cm_classsym;
	result->s_attr.a_attr  = attr;
	if (is_class_member) {
		result->s_attr.a_this = NULL;
	} else {
		symbol_incref(self->cm_thissym);
		result->s_attr.a_this = self->cm_thissym;
	}
	symbol_incref(self->cm_classsym);
	if (result->s_attr.a_this)
		SYMBOL_INC_NREAD(result->s_attr.a_this);
	SYMBOL_INC_NREAD(result->s_attr.a_class);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) struct class_member *DCALL
priv_alloc_class_member(struct class_maker *__restrict self) {
	if (self->cm_class_initc == self->cm_class_inita) {
		struct class_member *new_vector;
		size_t new_alloc = self->cm_class_inita * 2;
		if unlikely(!new_alloc)
			new_alloc = 2;
do_realloc:
		new_vector = (struct class_member *)Dee_TryReallocc(self->cm_class_initv,
		                                                    new_alloc,
		                                                    sizeof(struct class_member));
		if unlikely(!new_vector) {
			if (new_alloc != self->cm_class_initc + 1) {
				new_alloc = self->cm_class_initc + 1;
				goto do_realloc;
			}
			if (Dee_CollectMemoryc(new_alloc, sizeof(struct class_member)))
				goto do_realloc;
			goto err;
		}
		self->cm_class_initv = new_vector;
		self->cm_class_inita = new_alloc;
	}
	return self->cm_class_initv + self->cm_class_initc++;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
priv_reserve_instance_init(struct class_maker *__restrict self) {
	if (self->cm_initc == self->cm_inita) {
		DREF struct ast **new_vector;
		size_t new_alloc = self->cm_inita * 2;
		if unlikely(!new_alloc)
			new_alloc = 2;
do_realloc:
		new_vector = (DREF struct ast **)Dee_TryReallocc(self->cm_initv,
		                                                 new_alloc,
		                                                 sizeof(DREF struct ast *));
		if unlikely(!new_vector) {
			if (new_alloc != self->cm_inita + 1) {
				new_alloc = self->cm_initc + 1;
				goto do_realloc;
			}
			if (Dee_CollectMemoryc(new_alloc, sizeof(DREF struct ast *)))
				goto do_realloc;
			return -1;
		}
		self->cm_initv = new_vector;
		self->cm_inita = new_alloc;
	}
	return 0;
}


/* Add a member initializer for `sym', previously returned by `class_maker_addmember()'.
 * NOTE: If the symbol describes an instance member, `initializer' must have been generated
 *       in the context of the constructor scope (as it will be run immediately before
 *       the __constructor__ operator) */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
class_maker_addinit(struct class_maker *__restrict self,
                    struct symbol *__restrict sym,
                    struct ast *__restrict initializer,
                    struct ast_loc *__restrict loc) {
	struct class_attribute *entry;
	struct class_member *member;
	ASSERT(sym->s_type == SYMBOL_TYPE_CATTR);
	ASSERT(sym->s_attr.a_class == self->cm_classsym);
	ASSERT_AST(initializer);
	ASSERT(self->cm_initc <= self->cm_inita);
	ASSERT(self->cm_class_initc <= self->cm_class_inita);
	entry = sym->s_attr.a_attr;
	if (sym->s_attr.a_this != NULL &&
	    !(entry->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
		DREF struct ast *symbol_ast;

		/* Run the given AST during construction of instances. */
		/* Handle instance member initializers. */
		ASSERTF(self->cm_ctor_scope, "Without the ctor-scope allocated, this AST "
		                             "couldn't have been parsed in its context");
		ASSERTF(self->cm_ctor_scope == initializer->a_scope->s_base,
		        "Initializer ASTs must be parsed in the context of the CTOR scope");
		if (self->cm_ctor_flags & CLASS_MAKER_CTOR_FDELETED)
			return WARNAT(loc, W_MEMBER_INITIALIZER_USED_WHEN_CONSTRUCTOR_IS_DELETED);

		/* Check if we need to allocate more memory for the initializer vector. */
		if unlikely(priv_reserve_instance_init(self))
			goto err;

		/* Create a new AST to store to the given symbol. */
		symbol_ast = ast_sym(sym);
		if unlikely(!symbol_ast)
			goto err;
		initializer = ast_action2(AST_FACTION_STORE, symbol_ast, ast_putddi(initializer, loc));
		initializer = ast_setddi(initializer, loc);
		ast_decref(symbol_ast);
		if unlikely(!initializer)
			goto err;

		/* Override the store-ast's scope with our constructor scope. */
		Dee_Incref((DeeObject *)self->cm_ctor_scope);
		Dee_Decref(initializer->a_scope);
		initializer->a_scope = (DREF DeeScopeObject *)self->cm_ctor_scope;

		/* Place the store-ast in the initialization vector. */
		self->cm_initv[self->cm_initc++] = initializer;
		goto done;
	}

	/* Run the given AST during creation of the class. */
	ASSERTF(self->cm_ctor_scope != initializer->a_scope->s_base,
	        "Class member initializers must not be part of the CTOR scope");
	member = priv_alloc_class_member(self);
	if unlikely(!member)
		goto err;

	/* Add a class member initializer for the slot pointed to by the symbol's entry. */
	member->cm_index = entry->ca_addr;
	member->cm_ast   = initializer;
	ast_incref(initializer);
done:
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
class_maker_addanon(struct class_maker *__restrict self, uint16_t addr,
                    struct ast *__restrict initializer) {
	struct class_member *member;

	/* Run the given AST during creation of the class. */
	ASSERTF(self->cm_ctor_scope != initializer->a_scope->s_base,
	        "Class member initializers must not be part of the CTOR scope");
	member = priv_alloc_class_member(self);
	if unlikely(!member)
		goto err;

	/* Add a class member initializer for the given address. */
	member->cm_index = addr;
	member->cm_ast   = initializer;
	ast_incref(initializer);
	return 0;
err:
	return -1;
}

/* Add a new operator function to a given class. */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
class_maker_addoperator(struct class_maker *__restrict self,
                        Dee_operator_t operator_name,
                        struct ast *__restrict callback) {
	struct class_member *member;
	uint16_t addr;

	/* Allocate a new class member address for the operator. */
	addr = self->cm_desc->cd_cmemb_size;
	if unlikely(addr == UINT16_MAX) {
		PERRAST(callback, W_TOO_MANY_CLASS_MEMBER,
		        self->cm_classsym->s_name->k_name);
		goto err;
	}
	++self->cm_desc->cd_cmemb_size;

	/* Bind the specified operator to the given address. */
	if unlikely(class_maker_bindoperator(self, operator_name, addr, &callback->a_ddi))
		goto err;

	/* Allocate an initializer for the operator-bound class member. */
	member = priv_alloc_class_member(self);
	if unlikely(!member)
		goto err;
	member->cm_index = addr;
	member->cm_ast   = callback;
	ast_incref(callback);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
class_maker_deloperator(struct class_maker *__restrict self,
                        Dee_operator_t operator_name,
                        struct ast_loc *__restrict loc) {
	/* Deleted operator (e.g. `operator str = del;') */
	if (self->cm_null_member == (uint16_t)-1) {
		self->cm_null_member = self->cm_desc->cd_cmemb_size;
		if unlikely(self->cm_null_member == (uint16_t)-1) {
			return PERRAT(loc, W_TOO_MANY_CLASS_MEMBER,
			              self->cm_classsym->s_name->k_name);
		}
		++self->cm_desc->cd_cmemb_size;
	}

	/* Operators are deleted by binding them to a class member
	 * lacking an associated attribute, while always being unbound. */
	return class_maker_bindoperator(self, operator_name,
	                                self->cm_null_member,
	                                loc);
}


#define class_maker_init(self) \
	bzero(self, sizeof(struct class_maker))

/* Finalize everything concerning the given class maker. */
PRIVATE NONNULL((1)) void DCALL
class_maker_fini(struct class_maker *__restrict self) {
	size_t i;

	/* May be `NULL' following an init-failure. */
	Dee_XDecref_unlikely(self->cm_desc);
	ast_xdecref_unlikely(self->cm_base);
	ast_xdecref_unlikely(self->cm_ctor);
	Dee_XDecref_unlikely((DeeObject *)self->cm_ctor_scope);
	unicode_printer_fini(&self->cm_doc);
	ast_decrefv(self->cm_initv, self->cm_initc);
	Dee_Free(self->cm_initv);
	for (i = 0; i < self->cm_class_initc; ++i)
		ast_decref(self->cm_class_initv[i].cm_ast);
	Dee_Free(self->cm_class_initv);
}

/* Pack together an AST to create the class described by `self'. */
PRIVATE WUNUSED NONNULL((1)) DREF struct ast *DCALL
class_maker_pack(struct class_maker *__restrict self) {
	DREF struct ast *result;

	/* If required, create a class member initializer for the constructor. */
	if (self->cm_initc || self->cm_ctor) {
		DREF struct ast *constructor_text;
		DREF struct ast *constructor_function  = NULL;
		DREF struct ast *constructor_anno      = NULL;
		DREF struct ast **constructor_anno_ptr = NULL;

		/* Add the actual constructor when it was defined. */
		if (self->cm_ctor) {
			if unlikely(priv_reserve_instance_init(self))
				goto err;
			constructor_function = self->cm_ctor;
			if (constructor_function->a_type != AST_FUNCTION) {
				/* This can happen when annotations were used on the constructor.
				 * If this was the case, we must move all of those annotations to
				 * instead be applied to the constructor as a whole, rather than
				 * just to the portion used to represent the user's callback. */
				constructor_anno = constructor_function;
				do {
					ASSERT(constructor_function->a_type == AST_OPERATOR);
					ASSERT(constructor_function->a_flag == OPERATOR_CALL);
					ASSERT(constructor_function->a_operator.o_op0);
					ASSERT(constructor_function->a_operator.o_op1);
					ASSERT(!(constructor_function->a_operator.o_exflag & (AST_OPERATOR_FPOSTOP | AST_OPERATOR_FVARARGS)));
					constructor_function = constructor_function->a_operator.o_op1;
					ASSERT(constructor_function->a_type == AST_MULTIPLE);
					ASSERT(constructor_function->a_flag == AST_FMULTIPLE_TUPLE);
					ASSERT(constructor_function->a_multiple.m_astc >= 1);
					constructor_anno_ptr = &constructor_function->a_multiple.m_astv[0];
					constructor_function = *constructor_anno_ptr;
				} while (constructor_function->a_type != AST_FUNCTION);
			}
			self->cm_initv[self->cm_initc++] = constructor_function->a_function.f_code;
			ast_incref(constructor_function->a_function.f_code);
		}
		if (self->cm_initc != self->cm_inita) {
			DREF struct ast **new_vector;
			new_vector = (DREF struct ast **)Dee_TryReallocc(self->cm_initv,
			                                                 self->cm_initc,
			                                                 sizeof(DREF struct ast *));
			if likely(new_vector) {
				self->cm_initv = new_vector;
				self->cm_inita = self->cm_initc;
			}
		}

		/* Pack everything together in a multi-branch AST. */
		constructor_text = ast_multiple(AST_FMULTIPLE_KEEPLAST,
		                                self->cm_initc,
		                                self->cm_initv);
		if unlikely(!constructor_text)
			goto err;

		/* Set the correct scope for the constructor text. */
		Dee_Incref((DeeObject *)self->cm_ctor_scope);
		Dee_Decref(constructor_text->a_scope);
		constructor_text->a_scope = (DREF DeeScopeObject *)self->cm_ctor_scope;
		if (constructor_function) {
			ASSERT(constructor_function->a_type == AST_FUNCTION);
			ASSERT(constructor_function->a_function.f_scope == self->cm_ctor_scope);
			ast_decref(constructor_function->a_function.f_code);
			constructor_function->a_function.f_code = constructor_text; /* Inherit */
			self->cm_ctor = NULL;
		} else {
			constructor_function = ast_function(constructor_text, self->cm_ctor_scope);
			ast_decref(constructor_text);
			if unlikely(!constructor_function)
				goto err;
		}

		/* The constructor has inherited this vector. */
		self->cm_inita = 0;
		self->cm_initc = 0;
		self->cm_initv = NULL;
		if (constructor_anno) {
			/* Re-apply constructor annotations. */
			*constructor_anno_ptr = constructor_function; /* Inherit reference. */
			constructor_function  = constructor_anno;     /* Inherit reference. */
		}

		/* Add the constructor as an operator to the class initializer list. */
		if unlikely(class_maker_addoperator(self,
		                                    OPERATOR_CONSTRUCTOR,
		                                    constructor_function)) {
			ast_decref(constructor_function);
			goto err;
		}
		ast_decref(constructor_function);
	}

	/* Set the constructor-inherited flag for the resulting descriptor. */
	if (self->cm_ctor_flags & CLASS_MAKER_CTOR_FSUPER)
		self->cm_desc->cd_flags |= TP_FINHERITCTOR;
	if (self->cm_ctor_flags & CLASS_MAKER_CTOR_FDEFAULT)
		self->cm_desc->cd_flags |= CLASS_TP_FAUTOINIT;
	if (self->cm_ctor_flags & CLASS_MAKER_CTOR_FSUPERKWDS)
		self->cm_desc->cd_flags |= CLASS_TP_FSUPERKWDS;

	/* With all class members/operators out of the
	 * way, truncate the class operator table. */
	if (self->cm_class_initc != self->cm_class_inita) {
		struct class_member *new_vector;
		new_vector = (struct class_member *)Dee_TryReallocc(self->cm_class_initv,
		                                                    self->cm_class_initc,
		                                                    sizeof(struct class_member));
		if likely(new_vector) {
			self->cm_class_initv = new_vector;
			self->cm_class_inita = self->cm_class_initc;
		}
	}

	/* Truncate the descriptor's instance-attribute vector to
	 * its minimal length, if no instance attributes were ever
	 * defined.
	 * NOTE: Because no instance attributes exist because of this,
	 *       we also don't have to both with relocating them! */
	if (!self->cm_iattr_size) {
		DeeClassDescriptorObject *new_desc;
		new_desc = (DeeClassDescriptorObject *)DeeObject_TryReallocc(self->cm_desc,
		                                                             offsetof(DeeClassDescriptorObject, cd_iattr_list),
		                                                             1, sizeof(struct class_attribute));
		if likely(new_desc)
			self->cm_desc = new_desc;
		self->cm_desc->cd_iattr_mask = 0;
		ASSERT(!self->cm_desc->cd_iattr_list[0].ca_name);
	}

	/* Create a new branch for the documentation string (if it exists). */
	if (UNICODE_PRINTER_LENGTH(&self->cm_doc) != 0) {
		DREF DeeStringObject *doc_str;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
		if unlikely(doctext_compile(&self->cm_doc))
			goto err;
		if unlikely(doctext_escape(&self->cm_doc))
			goto err;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
		doc_str = (DREF DeeStringObject *)unicode_printer_pack(&self->cm_doc);
		unicode_printer_init(&self->cm_doc);
		if unlikely(!doc_str)
			goto err;
		ASSERT(!self->cm_desc->cd_doc);
		self->cm_desc->cd_doc = doc_str; /* Inherit reference. */
	}

	/* Finally, create the actual class AST */
	{
		DREF struct ast *descr_ast;
		descr_ast = ast_constexpr((DeeObject *)self->cm_desc);
		if unlikely(!descr_ast)
			goto err;
		result = ast_class(self->cm_base,
		                   descr_ast,
		                   self->cm_classsym,
		                   self->cm_supersym,
		                   self->cm_class_initc,
		                   self->cm_class_initv);
		ast_decref(descr_ast);
	}
	if likely(result) {
		/* The new ast will have inherit this stuff upon success. */
		self->cm_class_initc = 0;
		self->cm_class_inita = 0;
		self->cm_class_initv = NULL;
	}
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
parse_constructor_initializers(struct class_maker *__restrict self) {
	uint32_t old_flags;
	for (;;) {
		struct ast_loc loc;
		struct TPPKeyword *initializer_name;
		if unlikely(skip_lf())
			goto err;
		if unlikely(!TPP_ISKEYWORD(tok)) {
			if (WARN(W_EXPECTED_KEYWORD_IN_CONSTRUCTOR_INIT))
				goto err;
			break;
		}
		loc_here(&loc);
		initializer_name = token.t_kwd;
		if unlikely(yield() < 0)
			goto err;
		/* Super-initializer:
		 * class MyList: List {
		 *     private member foo;
		 *     private member bar;
		 *     this(args...)
		 *         : super(args...)
		 *         , foo = 10
		 *         , bar(20)
		 *     {
		 *     }
		 * }
		 */
		if (self->cm_base && (initializer_name->k_id == KWD_super ||
		                      (self->cm_base->a_type == AST_SYM &&
		                       self->cm_base->a_sym->s_name == initializer_name))) {
			DREF struct ast *superkwds;
			DREF struct ast *superargs, *merge;
			int temp;
			bool has_paren;
			old_flags = TPPLexer_Current->l_flags;
			TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
			if (paren_begin(&has_paren, W_EXPECTED_LPAREN_AFTER_SUPER_INIT))
				goto err_flags;
			if (has_paren) {
				if (tok == ')') {
					/* Empty super-args argument list.
					 * Since this is what the runtime will do by default in any case,
					 * there's no point in us not simply optimizing for this case already
					 * by just not generating a superargs operator. */
					goto done_superargs;
				}
			} else {
				temp = maybe_expression_begin();
				if (temp <= 0) {
					if unlikely(temp < 0)
						goto err_flags;
					goto done_superargs;
				}
			}
			basescope_pop(); /* Pop the constructor scope. */
			if unlikely(basescope_push())
				goto err_flags; /* Create a new base-scope for super-args. */

			/* Now we must mirror argument symbols from the constructor scope in this new one. */
			if unlikely(copy_argument_symbols(self->cm_ctor_scope))
				goto err_flags;

			/* Now we need to parse the argument list that's going to be provided as super-args. */
			superargs = ast_parse_argument_list(AST_COMMA_FORCEMULTIPLE, &superkwds);
			if unlikely(!superargs)
				goto err_flags;
			if (superkwds) {
				DREF struct ast **pair;
				pair = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
				if unlikely(!pair) {
err_flags_superargs_superkwds:
					Dee_Free(pair);
					ast_decref(superkwds);
					ast_decref(superargs);
					goto err_flags;
				}
				pair[0] = superargs; /* Inherit reference (on success) */
				pair[1] = superkwds; /* Inherit reference (on success) */
				merge   = ast_multiple(AST_FMULTIPLE_TUPLE, 2, pair);
				merge   = ast_setddi(merge, &loc);
				if unlikely(!merge)
					goto err_flags_superargs_superkwds;
				superargs = merge;
				self->cm_ctor_flags |= CLASS_MAKER_CTOR_FSUPERKWDS;
			}

			/* With the argument-tuple at hand, wrap it in a return + function ast. */
			merge = ast_return(superargs);
			merge = ast_setddi(merge, &loc);
			ast_decref(superargs);
			if unlikely(!merge)
				goto err_flags;
			ASSERT(current_scope == (DeeScopeObject *)current_basescope);

			/* Pop the super-args scope. (create the function in the class-scope context) */
			basescope_pop();
			superargs = ast_function(merge, (DeeBaseScopeObject *)merge->a_scope);
			superargs = ast_setddi(superargs, &loc);
			ast_decref(merge);
			if unlikely(!superargs)
				goto err_flags;

			/* All right! now just register the super-args callback as a new class operator. */
			temp = class_maker_addoperator(self, CLASS_OPERATOR_SUPERARGS, superargs);
			ast_decref(superargs);
			if unlikely(temp)
				goto err_flags;

			/* And we're done. - Just a little bit more cleanup to be done. */
			if unlikely(class_maker_push_ctorscope(self))
				goto err_flags;
done_superargs:
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if (paren_end(has_paren, W_EXPECTED_RPAREN_AFTER_SUPER_INIT))
				goto err;
		} else {
			struct symbol *init_symbol;
			DREF struct ast *initializer_ast, *store_ast, *symbol_ast;

			/* Lookup the initializer symbol. */
			init_symbol = lookup_symbol(LOOKUP_SYM_NORMAL, initializer_name, &loc);
			if unlikely(!init_symbol)
				goto err;

			/* Member initializer (c++ style). */
			if (tok == '=') {
				if unlikely(yield() < 0)
					goto err;
				initializer_ast = ast_parse_expr(LOOKUP_SYM_NORMAL);
				if unlikely(!initializer_ast)
					goto err;
			} else {
				bool has_paren;
				old_flags = TPPLexer_Current->l_flags;
				TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
				if (paren_begin(&has_paren, W_EXPECTED_LPAREN_OR_EQUAL_IN_CONSTRUCTOR_INIT))
					goto err_flags;
				if (has_paren && tok == ')') {
					/* Special case: Same as `= none' (aka: initializer to `none') */
constructor_list_none_initializer:
					initializer_ast = ast_constexpr(Dee_None);
					initializer_ast = ast_setddi(initializer_ast, &loc);
				} else {
					if (!has_paren) {
						int error = maybe_expression_begin();
						if (error <= 0) {
							if unlikely(error < 0)
								goto err_flags;
							goto constructor_list_none_initializer;
						}
					}
					initializer_ast = ast_parse_expr(LOOKUP_SYM_NORMAL);
				}
				if unlikely(!initializer_ast)
					goto err_flags;
				TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
				if (paren_end(has_paren, W_EXPECTED_RPAREN_CONSTRUCTOR_INIT)) {
					ast_decref(initializer_ast);
					goto err;
				}
			}

			/* Create a new branch to write to this symbol. */
			symbol_ast = ast_setddi(ast_sym(init_symbol), &loc);
			if unlikely(!symbol_ast)
				goto err;
			store_ast = ast_setddi(ast_action2(AST_FACTION_STORE, symbol_ast, initializer_ast), &loc);
			ast_decref(symbol_ast);
			ast_decref(initializer_ast);
			if unlikely(!store_ast)
				goto err;
			if unlikely(priv_reserve_instance_init(self)) {
				ast_decref(store_ast);
				goto err;
			}

			/* Add a new pre-construction initializer. */
			self->cm_initv[self->cm_initc++] = store_ast; /* Inherit reference. */
		}

		/* Stop if there is no comma to mark the next initializer. */
		if unlikely(skip_lf())
			goto err;
		if (tok != ',')
			break;
		if unlikely(yield() < 0)
			goto err;
	}
	return 0;
err_flags:
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
err:
	return -1;
}

STATIC_ASSERT(CLASS_GETSET_GET < CLASS_GETSET_COUNT);
STATIC_ASSERT(CLASS_GETSET_DEL < CLASS_GETSET_COUNT);
STATIC_ASSERT(CLASS_GETSET_SET < CLASS_GETSET_COUNT);

#define MAX_CALLBACK_NAME_LENGTH 8
struct callback_name {
	char    cn_name[MAX_CALLBACK_NAME_LENGTH]; /* Callback name. */
	uint8_t cn_depr;                           /* Non-zero if deprecated name. */
	uint8_t cn_id;                             /* Property callback index (One of `CLASS_PROPERTY_*'). */
};


STATIC_ASSERT(CLASS_GETSET_GET == 0);
STATIC_ASSERT(CLASS_GETSET_DEL == 1);
STATIC_ASSERT(CLASS_GETSET_SET == 2);
PRIVATE struct callback_name const callback_names[] = {
	/* [CLASS_GETSET_GET] = */ { "get", 0, CLASS_GETSET_GET },
	/* [CLASS_GETSET_DEL] = */ { "del", 0, CLASS_GETSET_DEL },
	/* [CLASS_GETSET_SET] = */ { "set", 0, CLASS_GETSET_SET },

	/* The old deemon accepted _a_ _lot_ of other names for callbacks.
	 * We continue to support them, but we warn if they are used instead
	 * of the preferred `get', `del' and `set' names, as also found as the
	 * names of property/member wrapper types like `ClassMember from rt' */
	{ "__get__", 1, CLASS_GETSET_GET },
	{ "__del__", 1, CLASS_GETSET_DEL },
	{ "__set__", 1, CLASS_GETSET_SET },
	{ "read", 1, CLASS_GETSET_GET },
	{ "delete", 1, CLASS_GETSET_DEL },
	{ "write", 1, CLASS_GETSET_SET },
	{ "put", 1, CLASS_GETSET_SET },
};


/* Parse the contents of a property declaration. */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
parse_property(DREF struct ast *callbacks[CLASS_GETSET_COUNT],
               struct class_maker *__restrict maker, bool is_class_property) {
	uint16_t callback_id;
	struct ast_loc loc;
	DREF struct ast *callback;
	struct ast_annotations annotations;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	struct decl_ast decl;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	bool has_name_prefix, need_semi;
	for (callback_id = 0; callback_id < CLASS_GETSET_COUNT; ++callback_id)
		callbacks[callback_id] = NULL;

	/* Parse the property declaration. */
next_callback:
	has_name_prefix = false;
next:
	loc_here(&loc);

	/* Remain compatible with the old deemon (which
	 * accepted/ignored a lot of weird stuff here). */
	switch (tok) {
	case TOK_EOF:
	case '}':
		goto done;

	case '\n':
		/* Skip empty lines in property declarations. */
		if unlikely(skip_lf())
			goto err;
		goto next;

	case ';':
		/* Rewind and clear context flags.
		 * NOTE: If a type prefix (`function' or `operator')
		 *       was used, warn about it being discarded. */
		if (has_name_prefix &&
		    WARN(W_EXPECTED_PROPERTY_NAME_AFTER_TYPE_PREFIX))
			goto err;
		goto next_callback;

		/* Ignore a `function' / `operator' prefix. */
	case KWD_function:
	case KWD_operator:
		if (has_name_prefix &&
		    WARN(W_PROPERTY_TYPE_PREFIX_ALREADY_GIVEN))
			goto err;
		if unlikely(yield() < 0)
			goto err;
		has_name_prefix = true;
		goto next;

		/* Deprecated symbol-based callback naming. */
	case '.':
		if (WARN(W_DEPRECATED_PROPERTY_NAME, "get' or `set"))
			goto err;
		if unlikely(yield() < 0)
			goto err;
		callback_id = CLASS_GETSET_GET;
		if (tok == '=') {
			callback_id = CLASS_GETSET_SET;
			if unlikely(yield() < 0)
				goto err;
		}
		goto got_callback_id;
	case '-':
		callback_id = CLASS_GETSET_DEL;
		goto warn_deprecated_yield;
	case '=':
		callback_id = CLASS_GETSET_SET;
		goto warn_deprecated_yield;

	default:
		if (TPP_ISKEYWORD(tok)) {
			unsigned int i;
			char const *name = token.t_kwd->k_name;
			size_t size      = token.t_kwd->k_size;
			if (size < MAX_CALLBACK_NAME_LENGTH) {
				for (i = 0; i < COMPILER_LENOF(callback_names); ++i) {
					/* Check if this is the property that was named. */
					if (bcmpc(callback_names[i].cn_name, name, size, sizeof(char)) != 0)
						continue;
					callback_id = callback_names[i].cn_id;

					/* Warn if the callback name has been marked as deprecated. */
					if (callback_names[i].cn_depr) {
warn_deprecated_yield:
						if (WARN(W_DEPRECATED_PROPERTY_NAME,
						         callback_names[callback_id].cn_name))
							goto err;
					}
					if unlikely(yield() < 0)
						goto err;
					if unlikely(tok == '.') {
						/* The old deemon allowed a `.' to follow some property names such as `del .'.
						 * But since such behavior is now deprecated, just disregard all that and
						 * consume a `.' token after a keyword and warn about it being ignored. */
						if (WARN(W_DEPRECATED_PROPERTY_NAME,
						         callback_names[callback_id].cn_name))
							goto err;
						if unlikely(yield() < 0)
							goto err;
					}
					goto got_callback_id;
				}
			}
		}
		if (WARN(W_EXPECTED_CALLBACK_NAME_IN_PROPERTY))
			goto err;
		goto done;
	}

	/* Warn if the callback had already bee defined. */
got_callback_id:
	if (callbacks[callback_id]) {
		if (WARN(W_PROPERTY_CALLBACK_ALREADY_DEFINED))
			goto err;
		/* Clear the callback. */
		ast_decref(callbacks[callback_id]);
		callbacks[callback_id] = NULL;
	}

	need_semi = false;
	ast_annotations_get(&annotations);
	if (is_class_property) {
		/* Parse a new function in the class scope. */
		callback = ast_parse_function(NULL, &need_semi, false, &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
		                              ,
		                              &decl
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
		                              );
	} else {
		if unlikely(class_maker_push_methscope(maker))
			goto err;
		/* Parse a new function in its own member-method scope. */
		callback = ast_parse_function_noscope(NULL, &need_semi, false, &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
		                                      ,
		                                      &decl
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
		                                      );
		basescope_pop();
	}
	if unlikely(!ast_setddi(callback, &loc))
		goto err_anno;

#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	/* TODO: Make use of declaration information! */
	decl_ast_fini(&decl);
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */

	callback = ast_annotations_apply(&annotations, callback);
	if unlikely(!callback)
		goto err;
	callbacks[callback_id] = callback; /* Inherit */

	/* Parse a semicolon if one is required. */
	if (need_semi) {
		if unlikely(likely(is_semicolon())
		            ? (yield_semicolon() < 0)
		            : WARN(W_EXPECTED_SEMICOLON_AFTER_EXPRESSION))
			goto err;
	}
	goto next_callback;
done:
	return 0;
err_anno:
	ast_annotations_free(&annotations);
err:
	for (callback_id = 0; callback_id < CLASS_GETSET_COUNT; ++callback_id)
		ast_xdecref(callbacks[callback_id]);
	return -1;
}

PRIVATE WUNUSED DREF struct ast *DCALL
ast_parse_class_impl(uint16_t class_flags, struct TPPKeyword *name,
                     bool create_symbol, unsigned int symbol_mode) {
	DREF struct ast *result;
	struct class_maker maker;
	struct ast_annotations annotations;
	uint16_t default_member_flags; /* Set of `CLASS_ATTRIBUTE_F*' */
	uint32_t old_flags = TPPLexer_Current->l_flags;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
	struct decl_ast decl;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
	class_maker_init(&maker);

	/* Inherit the documentation string printer. */
	memcpy(&maker.cm_doc, &current_tags.at_doc, sizeof(struct unicode_printer));
	unicode_printer_init(&current_tags.at_doc);

	/* Allocate the initial descriptor for the class. */
	maker.cm_desc = (DREF DeeClassDescriptorObject *)DeeObject_Callocc(offsetof(DeeClassDescriptorObject, cd_iattr_list),
	                                                                   7 + 1, sizeof(struct class_attribute));
	if unlikely(!maker.cm_desc)
		goto err;
	DeeObject_Init(maker.cm_desc, &DeeClassDescriptor_Type);
	maker.cm_desc->cd_flags      = class_flags;
	maker.cm_desc->cd_cattr_list = empty_class_attributes;
	maker.cm_desc->cd_clsop_list = empty_class_operators;
	maker.cm_desc->cd_iattr_mask = 7;
	maker.cm_null_member         = (uint16_t)-1;

	ASSERT(name || !create_symbol);
	if (tok == ':') {
do_parse_class_base:
		if unlikely(yield() < 0)
			goto err;
do_parse_class_base_after_yield:
		/* Parse the class's base expression.
		 * NOTE: We parse it as a unary-base expression, so-as to not
		 *       parse the `{' token that follows as a brace-initializer. */
		maker.cm_base = ast_parse_unaryhead(LOOKUP_SYM_NORMAL);
		if unlikely(!maker.cm_base)
			goto err;
		if (tok == ',') {
			/* Support for multiple bases */
			DREF struct ast **basev;
			size_t basec, basea;
			basev = (DREF struct ast **)Dee_Mallocc(2, sizeof(DREF struct ast *));
			if unlikely(!basev)
				goto err;
			basev[0] = maker.cm_base; /* Inherit reference. */
			basec    = 1;
			basea    = 2;
			do {
				/* Consume the `,' */
				if unlikely(yield() < 0)
					goto err_basev;
				if (basec >= basea) {
					DREF struct ast **new_basev;
					size_t new_basea;
					new_basea = basea * 2;
					new_basev = (DREF struct ast **)Dee_TryReallocc(basev, new_basea,
					                                                sizeof(DREF struct ast *));
					if unlikely(!new_basev) {
						new_basea = basec + 1;
						new_basev = (DREF struct ast **)Dee_Reallocc(basev, new_basea,
						                                             sizeof(DREF struct ast *));
						if unlikely(!new_basev)
							goto err_basev;
					}
					basea = new_basea;
					basev = new_basev;
				}
				basev[basec] = ast_parse_unaryhead(LOOKUP_SYM_NORMAL);
				if unlikely(!basev[basec])
					goto err_basev;
				++basec;
			} while (tok == ',');
			if (basea > basec) {
				DREF struct ast **new_basev;
				new_basev = (DREF struct ast **)Dee_TryReallocc(basev, basec,
				                                                sizeof(DREF struct ast *));
				if likely(new_basev)
					basev = new_basev;
			}
			maker.cm_base = ast_multiple(AST_FMULTIPLE_GENERIC, basec, basev);
			if unlikely(!maker.cm_base) {
err_basev:
				ast_decrefv(basev, basec);
				Dee_Free(basev);
				goto err;
			}
		}
	} else if (tok == '(') {
		/* Just another syntax for class bases that the old
		 * deemon supported and we're supporting as well.
		 * Though I should note that the intended syntax is `class foo: bar { ... }' */
		if (WARN(W_DEPRECATED_CLASS_BASE_PARENS))
			goto err;
		TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
		if unlikely(yield() < 0)
			goto err;
		maker.cm_base = ast_parse_expr(LOOKUP_SYM_NORMAL);
		if unlikely(!maker.cm_base)
			goto err;
		TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
		if (skip(')', W_EXPECTED_RPAREN_AFTER_LPAREN))
			goto err;
	} else {
		/* Since this is the only place that `extends' may appear at,
		 * I decided that it wouldn't merit its own keyword because
		 * this may there is much less overhead. */
		if (TPP_ISKEYWORD(tok) &&
		    (tok == KWD_pack || strcmp(token.t_kwd->k_name, "extends") == 0)) {
			if (tok == KWD_pack) {
				struct ast_loc packloc;
				if (WARN(W_DEPRECATED_CLASS_BASE_PARENS))
					goto err;
				loc_here(&packloc);
				if unlikely(yield() < 0)
					goto err;
				if unlikely(tok != '(' && parser_warn_pack_used(&packloc))
					goto err;
				goto do_parse_class_base_after_yield;
			}
			if (WARN(W_DEPRECATED_CLASS_BASE_EXTENDS))
				goto err;
			goto do_parse_class_base;
		}

		/* Automatically use `Object' as base class when `-fold-style-classes' is active. */
		if (HAS(EXT_OLD_STYLE_CLASSES)) {
			DREF DeeModuleObject *rt_d200_module;
			PRIVATE char const old_base[] = "OldUserClass";
			struct module_symbol *oldbase_sym;
			struct symbol *base_symbol;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
			rt_d200_module = (DREF DeeModuleObject *)DeeModule_OpenEx("rt.d200", COMPILER_STRLEN("rt.d200"),
			                                                          NULL, 0, DeeModule_IMPORT_F_ENOENT,
			                                                          inner_compiler_options);
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			rt_d200_module = (DREF DeeModuleObject *)DeeModule_OpenGlobal((DeeObject *)&str_rt_d200,
			                                                              inner_compiler_options,
			                                                              false);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			if unlikely(!ITER_ISOK(rt_d200_module)) {
				if (rt_d200_module) {
#if 1
					if (WARN(W_MODULE_NOT_FOUND, (DeeObject *)&str_rt_d200))
						goto err;
#else
					if (WARN(W_NO_D200_OLD_USER_CLASS))
						goto err;
#endif
					goto use_object_base;
				}
				goto err;
			}
			oldbase_sym = DeeModule_GetSymbolString(rt_d200_module, old_base);
			if unlikely(!oldbase_sym) {
				Dee_Decref(rt_d200_module);
				if (WARN(W_NO_D200_OLD_USER_CLASS))
					goto err;
				goto use_object_base;
			}

			/* Pack the reference to OldUserClass into a symbol. */
			base_symbol = new_unnamed_symbol();
			if unlikely(!base_symbol) {
				Dee_Decref(rt_d200_module);
				goto err;
			}
			base_symbol->s_type            = SYMBOL_TYPE_EXTERN;
			base_symbol->s_extern.e_module = rt_d200_module; /* Inherit reference. */
			base_symbol->s_extern.e_symbol = oldbase_sym;

			/* Create a symbol-ast for the base expression. */
			maker.cm_base = ast_sym(base_symbol);
		} else {
use_object_base:
			maker.cm_base = ast_constexpr((DeeObject *)&DeeObject_Type);
		}
		if unlikely(!maker.cm_base)
			goto err;
	}

	if (name) {
		DREF DeeStringObject *name_str;
		name_str = (DREF DeeStringObject *)DeeString_NewSized(name->k_name,
		                                                      name->k_size);
		if unlikely(!name_str)
			goto err;
		maker.cm_desc->cd_name = name_str; /* Inherit reference. */
	}
	if (parser_flags & PARSE_FLFSTMT)
		TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
	if (skip('{', W_EXPECTED_LBRACE_AFTER_CLASS))
		goto err;

	/* Create the symbol to assign the class to. */
	if (create_symbol) {
		maker.cm_classsym = lookup_symbol(symbol_mode, name, NULL);
		if unlikely(!maker.cm_classsym)
			goto err;
		if (classscope_push())
			goto err;
	} else {
		if (classscope_push())
			goto err;
		maker.cm_classsym = new_unnamed_symbol();
		if unlikely(!maker.cm_classsym)
			goto err;
		maker.cm_classsym->s_type = SYMBOL_TYPE_STACK;
	}
	maker.cm_thissym = ((DeeClassScopeObject *)current_scope)->cs_this;
	((DeeClassScopeObject *)current_scope)->cs_class = maker.cm_classsym;

	/* Create a symbol for the class's super type. */
	if (maker.cm_base) {
		/* Optimization: If the base expression is a symbol, where accessing
		 *               it doesn't have any side-effects, then we can omit
		 *               the creation of an additional storage symbol for the
		 *               class itself, and simply re-use the same symbol, for
		 *               allowing further optimizations down the line. */
		if (maker.cm_base->a_type == AST_SYM &&
		    !symbol_get_haseffect(maker.cm_base->a_sym, current_scope)) {
			maker.cm_supersym = maker.cm_base->a_sym;
		} else {
			maker.cm_supersym = new_unnamed_symbol();
			if unlikely(!maker.cm_supersym)
				goto err;
			maker.cm_supersym->s_type = SYMBOL_TYPE_STACK;
		}
		((DeeClassScopeObject *)current_scope)->cs_super = maker.cm_supersym;
	}

	default_member_flags = CLASS_ATTRIBUTE_FPUBLIC;
	if (class_flags & TP_FFINAL)
		default_member_flags |= CLASS_ATTRIBUTE_FFINAL;
	for (;;) {
#define MEMBER_CLASS_AUTO   0
#define MEMBER_CLASS_MEMBER 1
#define MEMBER_CLASS_METHOD 2
#define MEMBER_CLASS_GETSET 3
		int member_class;
		uint16_t member_flags;
		bool is_class_member;
		uint16_t *p_usage_counter;
		struct ast_loc loc;
		bool modifiers_encountered;
		bool varying_encountered;
next_member:
		member_class          = MEMBER_CLASS_AUTO;
		member_flags          = default_member_flags;
		is_class_member       = false;
		modifiers_encountered = false;
		varying_encountered   = false;

		/* Reset member tags. */
		if unlikely(ast_tags_clear())
			goto err;
next_modifier:
		switch (tok) {

		case '@':
			/* Allow tags in class blocks. */
			if unlikely(yield() < 0)
				goto err;
			if unlikely(parse_tags())
				goto err;
			modifiers_encountered = true;
			goto next_modifier;

		case '}':
			if (modifiers_encountered &&
			    WARN(W_CLASS_NO_MEMBER_DEFINED))
				goto err;
			TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
			TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
			if unlikely(yield() < 0)
				goto err;
			goto done_class_modal;

		case '\n':
			if unlikely(skip_lf())
				goto err;
			goto next_modifier;

		case ';':
			if (modifiers_encountered &&
			    WARN(W_CLASS_NO_MEMBER_DEFINED))
				goto err;
			if unlikely(yield() < 0)
				goto err;
			goto next_member;

		{
			uint16_t new_visibility;
		case KWD_private:
			new_visibility = CLASS_ATTRIBUTE_FPRIVATE;
			goto set_visibility;
		case KWD_public:
			new_visibility = CLASS_ATTRIBUTE_FPUBLIC;
set_visibility:
			if unlikely(yield() < 0)
				goto err;
			if (tok == ':') {
				/* Default visibility override (rather than member-specific visibility). */
				default_member_flags &= ~CLASS_ATTRIBUTE_FVISIBILITY;
				default_member_flags |= new_visibility;
				if unlikely(yield() < 0)
					goto err;
			}
			member_flags &= ~CLASS_ATTRIBUTE_FVISIBILITY;
			member_flags |= new_visibility;
			modifiers_encountered = true;
		}	goto next_modifier;

		case KWD_final:
			if unlikely(yield() < 0)
				goto err;
			member_flags |= CLASS_ATTRIBUTE_FFINAL;
			if (!varying_encountered)
				member_flags |= CLASS_ATTRIBUTE_FREADONLY;
			modifiers_encountered = true;
			goto next_modifier;

		case KWD_varying:
			if unlikely(yield() < 0)
				goto err;
			member_flags &= ~CLASS_ATTRIBUTE_FREADONLY;
			varying_encountered   = true;
			modifiers_encountered = true;
			goto next_modifier;

		case KWD_class:
			/* Class field. */
			if (is_class_member &&
			    WARN(W_STATIC_FIELD_ALREADY_SPECIFIED))
				goto err;
			loc_here(&loc);
			if unlikely(yield() < 0)
				goto err;
			if (tok == '(' || tok == '{' ||
			    tok == ':' || tok == TOK_ARROW || tok == KWD_pack) {
				/* A deprecated syntax for defining constructors allowed
				 * the use of `class' as another alias for `this' and the
				 * actual name of the class. */
				if (WARN(W_DEPRECATED_USING_CLASS_FOR_CONSTRUCTOR,
				         maker.cm_classsym->s_name->k_name))
					goto err;
				goto define_constructor;
			}

			/* Warn about `class' being used to declare static members being deprecated */
			if (WARN(W_DEPRECATED_USING_CLASS_FOR_STATIC))
				goto err;
			is_class_member       = true;
			modifiers_encountered = true;
			goto next_modifier;


		case KWD_static:
			/* Static (aka. class) field. */
			if (is_class_member &&
			    WARN(W_STATIC_FIELD_ALREADY_SPECIFIED))
				goto err;
			if unlikely(yield() < 0)
				goto err;
			is_class_member       = true;
			modifiers_encountered = true;
			goto next_modifier;

		case KWD_function:
		case KWD_property:
		case KWD_member:
			if (member_class != MEMBER_CLASS_AUTO &&
			    WARN(W_CLASS_MEMBER_TYPE_ALREADY_SPECIFIED))
				goto err;
			member_class = (tok == KWD_function
			                ? MEMBER_CLASS_METHOD
			                : tok == KWD_property
			                  ? MEMBER_CLASS_GETSET
			                  : MEMBER_CLASS_MEMBER);
			if unlikely(yield() < 0)
				goto err;
			modifiers_encountered = true;
			goto next_modifier;

		case KWD_operator: {
			Dee_operator_t operator_name;
			bool need_semi;
			int error;
			struct TPPKeyword *operator_name_kwd;
			DREF struct ast *operator_ast;
			int32_t temp;
			loc_here(&loc);
			if unlikely(yield() < 0)
				goto err;

			/* Parse an operator name, also accepting
			 * class-specific and file-specific operator names. */
			temp = ast_parse_operator_name(P_OPERATOR_FCLASS);
			if unlikely(temp < 0)
				goto err;
			operator_name = (Dee_operator_t)temp;

			/* Special case: The constructor operator. */
define_operator:
			if (operator_name == OPERATOR_CONSTRUCTOR)
				goto define_constructor;

			/* Warn if declaring an attribute operator with non-final, public members. */
			if (!(maker.cm_features & CLASS_MAKER_FEAT_FATTROPS) &&
			    (operator_name == OPERATOR_GETATTR || operator_name == OPERATOR_DELATTR ||
			     operator_name == OPERATOR_SETATTR || operator_name == AST_OPERATOR_GETATTR_OR_SETATTR)) {
				if (maker.cm_features & CLASS_MAKER_FEAT_FNOFINPUB) {
					bool did_warn_any = false;
					size_t i;
					for (i = 0; i <= maker.cm_desc->cd_iattr_mask; ++i) {
						struct TPPKeyword *member_keyword;
						struct class_attribute *attr;
						struct symbol *member_symbol;
						attr = &maker.cm_desc->cd_iattr_list[i];
						if (!attr->ca_name)
							continue;
						if (!class_attribute_is_nonfinal_public(attr->ca_flag))
							continue;
						/* Find the declaration location of this symbol. */
						member_keyword = TPPLexer_LookupKeyword(DeeString_STR(attr->ca_name),
						                                        DeeString_SIZE(attr->ca_name),
						                                        0);
						if unlikely(!member_keyword)
							continue;
						member_symbol = get_local_symbol(member_keyword);
						if unlikely(!member_symbol)
							continue;
						if (warn_nonfinal_public_with_attr_operators(&member_symbol->s_decl,
						                                             DeeString_STR(attr->ca_name),
						                                             attr->ca_flag))
							goto err;
						did_warn_any = true;
					}

					/* Make sure that we always emit at least 1 warning.
					 * But note that at this point, `did_warn_any' should _always_ already be true! */
					if unlikely(!did_warn_any) {
						struct class_attribute *attr;
						attr = cdesc_find_nonfinal_public_symbol(maker.cm_desc);
						if (warn_nonfinal_public_with_attr_operators(&loc,
						                                             attr ? DeeString_STR(attr->ca_name) : "?",
						                                             attr ? attr->ca_flag : CLASS_ATTRIBUTE_FNORMAL))
							goto err;
					}
				}
				maker.cm_features |= CLASS_MAKER_FEAT_FATTROPS;
			}
			ast_annotations_get(&annotations);
			if (tok == '=') {
				if (operator_name >= AST_OPERATOR_MIN &&
				    operator_name <= AST_OPERATOR_MAX_FOR_CLASS) {
					if (WARN(W_AMBIGUOUS_OPERATOR_ASSIGNMENT))
						goto err_anno;
					switch (operator_name) {
					case AST_OPERATOR_POS_OR_ADD:
						operator_name = OPERATOR_ADD;
						break;
					case AST_OPERATOR_NEG_OR_SUB:
						operator_name = OPERATOR_SUB;
						break;
					case AST_OPERATOR_GETITEM_OR_SETITEM:
						operator_name = OPERATOR_GETITEM;
						break;
					case AST_OPERATOR_GETRANGE_OR_SETRANGE:
						operator_name = OPERATOR_GETRANGE;
						break;
					case AST_OPERATOR_GETATTR_OR_SETATTR:
						operator_name = OPERATOR_GETATTR;
						break;
					case AST_OPERATOR_FOR:
						operator_name = OPERATOR_ITER;
						break;
					case AST_OPERATOR_STR_OR_PRINT:
						operator_name = OPERATOR_STR;
						break;
					case AST_OPERATOR_REPR_OR_PRINTREPR:
						operator_name = OPERATOR_REPR;
						break;
					default: break;
					}
				}

				/* Operator callback assignment. */
				if unlikely(yield() < 0)
					goto err_anno;
				need_semi = true;
				if (tok == KWD_del) {
					/* Deleted operator (e.g. `operator str = del;') */
					if unlikely(class_maker_deloperator(&maker, operator_name, &loc))
						goto err_anno;
					if unlikely(yield() < 0)
						goto err_anno;
					if unlikely(ast_annotations_clear(&annotations))
						goto err_anno;
					goto yield_semi_after_operator;
				}
				operator_ast = ast_parse_expr(LOOKUP_SYM_NORMAL);
				goto set_operator_ast;
			}
			if (operator_name == AST_OPERATOR_FOR) {
				/* Special case: `operator for()' is a wrapper around `operator iter()' */
				DREF struct ast *yield_function, *tempast, **argv;
				/* Not actually an operator (shares a slot with `OPERATOR_ITER')
				 * This operator can be used by `DeeClass_SetOperator()' to wrap the
				 * given callback using an internal wrapper type that behaves as follows:
				 * >> class MyClass {
				 * >>
				 * >>     public operator __iter__() {
				 * >>         return () -> {
				 * >>             yield 10;
				 * >>             yield 20;
				 * >>             yield 30;
				 * >>         }().operator __iter__();
				 * >>     }
				 * >>
				 * >>     public operator for() {
				 * >>         // Same as the above.
				 * >>         yield 10;
				 * >>         yield 20;
				 * >>         yield 30;
				 * >>     }
				 * >> }; */
				operator_name_kwd = TPPLexer_LookupKeyword("for", 3, 0);
				ASSERT(operator_name_kwd != NULL);
				if unlikely(class_maker_push_methscope(&maker))
					goto err_anno;

				/* Parse a new function in its own member-method scope. */
				current_basescope->bs_name = operator_name_kwd;
				operator_name              = OPERATOR_ITER;

				/* Inner function scope (yes: `operator for' is a function within a function....)
				 * WRAP: `AST_FUNCTION(AST_RETURN(AST_OPERATOR(OPERATOR_ITER,
				 *                                AST_OPERATOR(OPERATOR_CALL,
				 *                                             ...,  // The inner function
				 *                                             AST_MULTIPLE(AST_FMULTIPLE_TUPLE, [
				 *                                                          AST_SYM(this)]))))' */
				if unlikely(class_maker_push_methscope(&maker))
					goto err_anno;
				yield_function = ast_parse_function_noscope(NULL, &need_semi, false, &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
				                                            ,
				                                            &decl
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
				                                            );
				basescope_pop();
				if unlikely(!yield_function) {
err_operator_ast_ddi:
					operator_ast = NULL;
					goto got_operator_ast;
				}
				tempast = ast_setddi(ast_sym(maker.cm_thissym), &loc);
				if unlikely(!tempast) {
err_yield_function:
					ast_decref(yield_function);
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
					decl_ast_fini(&decl); /* TODO: Encode declaration information in operator docs! */
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
					goto err_operator_ast_ddi;
				}
				argv = (DREF struct ast **)Dee_Mallocc(1, sizeof(DREF struct ast *));
				if unlikely(!argv) {
err_yield_function_temp:
					ast_decref(tempast);
					goto err_yield_function;
				}
				argv[0]      = tempast; /* Inherit reference */
				operator_ast = ast_setddi(ast_multiple(AST_FMULTIPLE_TUPLE, 1, argv), &loc);
				if unlikely(!operator_ast) {
					Dee_Free(argv);
					goto err_yield_function_temp;
				}
				tempast = ast_setddi(ast_operator2(OPERATOR_CALL,
				                                   AST_OPERATOR_FNORMAL,
				                                   yield_function,
				                                   operator_ast),
				                     &loc);
				ast_decref(operator_ast);
				ast_decref(yield_function);
				if unlikely(!tempast)
					goto err_operator_ast_ddi;
				operator_ast = ast_setddi(ast_operator1(OPERATOR_ITER,
				                                        AST_OPERATOR_FNORMAL,
				                                        tempast),
				                          &loc);
				ast_decref(tempast);
				if unlikely(!operator_ast)
					goto err_operator_ast_ddi;
				tempast = ast_setddi(ast_return(operator_ast), &loc);
				ast_decref(operator_ast);
				if unlikely(!tempast)
					goto err_operator_ast_ddi;

				/* And finally: the surrounding function. */
				operator_ast = ast_setddi(ast_function(tempast, current_basescope), &loc);
				ast_decref(tempast);
				if unlikely(!operator_ast)
					goto err_operator_ast_ddi;
				ASSERT(operator_ast->a_scope == current_scope);
				Dee_Incref(current_scope->s_prev);
				Dee_Decref(operator_ast->a_scope);
				operator_ast->a_scope = current_scope->s_prev;
			} else {
				struct opinfo const *info;
				operator_name_kwd = NULL;
				if ((info = DeeTypeType_GetOperatorById(&DeeType_Type, operator_name)) != NULL) {
					char opname[2 + COMPILER_LENOF(info->oi_sname) + 3];
					size_t opnamelen = strlen(info->oi_sname);
					char *p = opname;
					*p++ = '_';
					*p++ = '_';
					p = (char *)mempcpyc(p, info->oi_sname, opnamelen, sizeof(char));
					*p++ = '_';
					*p++ = '_';
					*p++ = '\0';
					operator_name_kwd = TPPLexer_LookupKeyword(opname, opnamelen + 4, 1);
					if unlikely(!operator_name_kwd)
						goto err_anno;
				}
				if unlikely(class_maker_push_methscope(&maker))
					goto err_anno;
				/* Parse a new function in its own member-method scope. */
				current_basescope->bs_name = operator_name_kwd;
				operator_ast = ast_parse_function_noscope(NULL, &need_semi, false, &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
				                                          ,
				                                          &decl
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
				                                          );
			}
got_operator_ast:
			basescope_pop();
			if unlikely(!operator_ast)
				goto err_anno;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
			decl_ast_fini(&decl); /* TODO: Encode declaration information in operator docs! */
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
			ASSERT(operator_ast->a_type == AST_FUNCTION);
			ASSERT(operator_ast->a_function.f_scope);
			if (operator_name >= AST_OPERATOR_MIN &&
			    operator_name <= AST_OPERATOR_MAX_FOR_CLASS) {
				/* Special case: determine the actual operator from
				 *               the argument count of the function. */
				uint16_t argc = operator_ast->a_function.f_scope->bs_argc_min;
				switch (operator_name) {

				case AST_OPERATOR_POS_OR_ADD:
					operator_name = argc == 0
					                ? OPERATOR_POS
					                : OPERATOR_ADD;
					break;

				case AST_OPERATOR_NEG_OR_SUB:
					operator_name = argc == 0
					                ? OPERATOR_NEG
					                : OPERATOR_SUB;
					break;

				case AST_OPERATOR_GETITEM_OR_SETITEM:
					operator_name = argc == 1
					                ? OPERATOR_GETITEM
					                : OPERATOR_SETITEM;
					break;

				case AST_OPERATOR_GETRANGE_OR_SETRANGE:
					operator_name = argc == 2
					                ? OPERATOR_GETRANGE
					                : OPERATOR_SETRANGE;
					break;

				case AST_OPERATOR_GETATTR_OR_SETATTR:
					operator_name = argc == 1
					                ? OPERATOR_GETATTR
					                : OPERATOR_SETATTR;
					break;

				case AST_OPERATOR_STR_OR_PRINT:
					operator_name = argc == 1
					                ? CLASS_OPERATOR_PRINT
					                : OPERATOR_STR;
					break;

				case AST_OPERATOR_REPR_OR_PRINTREPR:
					operator_name = argc == 1
					                ? CLASS_OPERATOR_PRINTREPR
					                : OPERATOR_REPR;
					break;

				default: break;
				}
			}
set_operator_ast:
			/* XXX: Add operator documentation? */
			operator_ast = ast_annotations_apply(&annotations, operator_ast);
			if unlikely(!operator_ast)
				goto err;

			/* Warn when attempting to declare an operator as private. */
			if (member_flags & CLASS_ATTRIBUTE_FPRIVATE) {
				struct opinfo const *info = DeeTypeType_GetOperatorById(&DeeType_Type, operator_name);
				if (WARNAT(&loc, W_PRIVATE_OPERATOR_IS_PUBLIC, info ? info->oi_sname : "?"))
					goto err;
			}

			/* Add the new operator to the class. */
			error = class_maker_addoperator(&maker, operator_name, operator_ast);
			ast_decref(operator_ast);
			if unlikely(error)
				goto err;

			/* Parse a trailing ';' if required to. */
			if (need_semi) {
yield_semi_after_operator:
				if unlikely(likely(is_semicolon())
				            ? (yield_semicolon() < 0)
				            : WARN(W_EXPECTED_SEMICOLON_AFTER_EXPRESSION))
					goto err;
			}
			break;
		case '~':
			loc_here(&loc);
			if unlikely(yield() < 0)
				goto err;
			if unlikely(tok == KWD_class) {
				if (WARN(W_DEPRECATED_USING_CLASS_FOR_CONSTRUCTOR,
				         maker.cm_classsym->s_name->k_name))
					goto err;
				if unlikely(yield() < 0)
					goto err;
			} else {
				if unlikely(likely(tok == KWD_this || (TPP_ISKEYWORD(tok) && token.t_kwd == name))
				            ? (yield() < 0)
				            : WARN(W_EXPECTED_THIS_OR_CLASSNAME_AFTER_TILDE,
				                   maker.cm_classsym->s_name->k_name))
					goto err;
			}
			operator_name = OPERATOR_DESTRUCTOR;
			goto define_operator;

		case KWD_copy:
			if unlikely(yield() < 0)
				goto err;
			operator_name = OPERATOR_COPY;
			goto define_operator;

		case KWD_deepcopy:
			if unlikely(yield() < 0)
				goto err;
			operator_name = OPERATOR_DEEPCOPY;
			goto define_operator;
		}	break;

		default: {
			struct TPPKeyword *member_name;
			struct symbol *member_symbol;
			DREF struct ast *init_ast;
			bool need_semi;
			int error;
			if (!TPP_ISKEYWORD(tok)) {
				if (WARN(W_UNEXPECTED_TOKEN_IN_CLASS))
					goto err;
				TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
				TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
				goto done_class_modal;
			}
			loc_here(&loc);
			member_name = token.t_kwd;
			if (is_reserved_symbol_name(member_name) &&
			    WARN(W_RESERVED_MEMBER_NAME, member_name))
				goto err;
			if (member_name == name) {
		case KWD_this:
				/* Special case: Constructor. */
				if unlikely(yield() < 0)
					goto err;
				loc_here(&loc);
define_constructor:
				if unlikely(maker.cm_ctor) {
					if (WARN(W_CLASS_CONSTRUCTOR_ALREADY_DEFINED,
					         maker.cm_classsym->s_name->k_name))
						goto err;
					ast_decref(maker.cm_ctor);
					maker.cm_ctor = NULL;
				}
				if (tok == '=') {
					/* Special cases:
					 *   - `this = del' (delete the constructor)
					 *   - `this = super' (inherit the constructor from a super-class)
					 *   - `this = ...' (Assign a custom callback that is invoked as the constructor) */
					if unlikely(yield() < 0)
						goto err;
					if (tok == KWD_del) {
						if (!(maker.cm_ctor_flags & CLASS_MAKER_CTOR_FDELETED)) {
							if (maker.cm_ctor_flags & CLASS_MAKER_CTOR_FSUPER) {
								if (WARN(W_CANNOT_DELETE_INHERITED_CONSTRUCTOR,
								         maker.cm_classsym->s_name->k_name))
									goto err;
								maker.cm_ctor_flags &= ~CLASS_MAKER_CTOR_FSUPER;
							}
							if (maker.cm_ctor_flags & CLASS_MAKER_CTOR_FDEFAULT) {
								if (WARN(W_CANNOT_DELETE_DEFAULT_CONSTRUCTOR,
								         maker.cm_classsym->s_name->k_name))
									goto err;
								maker.cm_ctor_flags &= ~CLASS_MAKER_CTOR_FDEFAULT;
							}

							/* Warn about instance member initializers being used,
							 * as well as set a flag that will warn about future
							 * use of instance member initializers. */
							maker.cm_ctor_flags |= CLASS_MAKER_CTOR_FDELETED;

							/* Warn about, and delete all member initializers. */
							while (maker.cm_initc) {
								DREF struct ast *init;
								init = maker.cm_initv[maker.cm_initc - 1];
								if (WARNAST(init, W_MEMBER_INITIALIZER_USED_WHEN_CONSTRUCTOR_IS_DELETED))
									goto err;
								--maker.cm_initc;
								ast_decref(init);
							}
							if unlikely(class_maker_deloperator(&maker, OPERATOR_CONSTRUCTOR, &loc))
								goto err;
						}
						if unlikely(yield() < 0)
							goto err;
						goto do_yield_semicolon;
					}
					if (tok == KWD_super) {
						/* Inherit constructors.
						 * - Set the `TP_FINHERITCTOR' flag, which will instruct the
						 *   class runtime to implement `CLASS_OPERATOR_SUPERARGS' in
						 *   such a way that arguments are forwarded exactly. */
						if (maker.cm_ctor_flags & CLASS_MAKER_CTOR_FDELETED) {
							if (WARNAT(&loc, W_CANNOT_INHERIT_DELETED_CONSTRUCTOR,
							           maker.cm_classsym->s_name->k_name))
								goto err;
							maker.cm_ctor_flags &= ~CLASS_MAKER_CTOR_FDELETED;
						}
						if (maker.cm_ctor_flags & CLASS_MAKER_CTOR_FDEFAULT) {
							if (WARNAT(&loc, W_CANNOT_INHERIT_DEFAULT_CONSTRUCTOR,
							           maker.cm_classsym->s_name->k_name))
								goto err;
							maker.cm_ctor_flags &= ~CLASS_MAKER_CTOR_FDEFAULT;
						}
						maker.cm_ctor_flags |= CLASS_MAKER_CTOR_FSUPER;
						if unlikely(yield() < 0)
							goto err;
						goto do_yield_semicolon;
					}
					if (tok == KWD_default) {
						/* Default constructors. */
						if (maker.cm_ctor_flags & CLASS_MAKER_CTOR_FSUPER) {
							if (WARNAT(&loc, W_CANNOT_DEFAULT_INHERIT_CONSTRUCTOR,
							           maker.cm_classsym->s_name->k_name))
								goto err;
						} else if (maker.cm_ctor_flags & CLASS_MAKER_CTOR_FDELETED) {
							if (WARNAT(&loc, W_CANNOT_DEFAULT_DELETED_CONSTRUCTOR,
							           maker.cm_classsym->s_name->k_name))
								goto err;
						} else {
							maker.cm_ctor_flags |= CLASS_MAKER_CTOR_FDEFAULT;
						}
						if unlikely(yield() < 0)
							goto err;
						goto do_yield_semicolon;
					}
					if (maker.cm_ctor_flags & (CLASS_MAKER_CTOR_FDELETED | CLASS_MAKER_CTOR_FSUPER)) {
						if (WARN(W_CLASS_CONSTRUCTOR_ALREADY_DEFINED,
						         maker.cm_classsym->s_name->k_name))
							goto err;
						maker.cm_ctor_flags &= ~(CLASS_MAKER_CTOR_FDELETED | CLASS_MAKER_CTOR_FSUPER);
					}

					/* Parse an expression that is invoked at the end of the constructor. */
					ast_annotations_get(&annotations);
					if unlikely(class_maker_push_ctorscope(&maker))
						goto err_anno;

					/* Compile the actual constructor in a sub-scope so any local
					 * variables that it may define will not interfere with the
					 * lookup rules in member initializers that may still follow. */
					if (scope_push())
						goto err_anno;
					{
						DREF struct ast *call_branch, *call_args;
						DREF struct ast *ctor_expr;
						ctor_expr = ast_putddi(ast_parse_expr(LOOKUP_SYM_NORMAL), &loc);
						if unlikely(!ctor_expr)
							goto err_anno;
						if (!current_basescope->bs_argc) {
							/* Create anonymous varargs. */
							struct symbol *dots = new_unnamed_symbol_in_scope(&current_basescope->bs_scope);
							if unlikely(!dots)
								goto err_ctor_expr;
							Dee_Free(current_basescope->bs_argv);
							current_basescope->bs_argv = (struct symbol **)Dee_Mallocc(1, sizeof(struct symbol *));
							if unlikely(!current_basescope->bs_argv)
								goto err_ctor_expr;
							dots->s_type  = SYMBOL_TYPE_ARG;
							dots->s_symid = 0;
							dots->s_flag |= SYMBOL_FALLOC;
							current_basescope->bs_argc    = 1;
							current_basescope->bs_argv[0] = dots;
							current_basescope->bs_varargs = dots;
							current_basescope->bs_flags |= CODE_FVARARGS;
						}

						/* Wrap the constructor expression in a call-branch (so-as to invoke it when the time comes) */
						if (current_basescope->bs_varargs) {
							call_args = ast_setddi(ast_sym(current_basescope->bs_varargs), &loc);
						} else {
							call_args = ast_setddi(ast_constexpr(Dee_EmptyTuple), &loc);
						}
						if unlikely(!call_args) {
err_ctor_expr:
							Dee_Decref(ctor_expr);
							goto err_anno;
						}
						call_branch = ast_operator2(OPERATOR_CALL, AST_OPERATOR_FNORMAL,
						                            ctor_expr, call_args);
						ast_decref(call_args);
						ast_decref(ctor_expr);
						if unlikely(!call_branch)
							goto err_anno;
						scope_pop();
						basescope_pop();
						call_branch = ast_annotations_apply(&annotations, call_branch);
						if unlikely(!call_branch)
							goto err;
						if unlikely(priv_reserve_instance_init(&maker))
							goto err;
						maker.cm_initv[maker.cm_initc++] = call_branch; /* Inherit reference. */
					}
					goto do_yield_semicolon;
				}
				if (maker.cm_ctor_flags & (CLASS_MAKER_CTOR_FDELETED | CLASS_MAKER_CTOR_FSUPER)) {
					if (WARN(W_CLASS_CONSTRUCTOR_ALREADY_DEFINED,
					         maker.cm_classsym->s_name->k_name))
						goto err;
					maker.cm_ctor_flags &= ~(CLASS_MAKER_CTOR_FDELETED | CLASS_MAKER_CTOR_FSUPER);
				}
				ast_annotations_get(&annotations);

				/* Explicitly push the constructor-scope when parsing the constructor. */
				if unlikely(class_maker_push_ctorscope(&maker))
					goto err_anno;

				/* NOTE: We do the function processing manually so we can
				 *       correctly handle super-initializer statements. */
				if (tok == '(') {
					/* Argument list. */
					TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
					if unlikely(yield() < 0)
						goto err_anno;
					if unlikely(parse_arglist())
						goto err_anno;
					if (parser_flags & PARSE_FLFSTMT)
						TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
					if (skip(')', W_EXPECTED_RPAREN_AFTER_ARGLIST))
						goto err_anno;
				} else if (tok == KWD_pack) {
					struct ast_loc packloc;
					/* Argument list. */
					TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
					loc_here(&packloc);
					if unlikely(yield() < 0)
						goto err_anno;
					if (tok == '(') {
						if unlikely(yield() < 0)
							goto err_anno;
						if unlikely(parse_arglist())
							goto err_anno;
						if (skip(')', W_EXPECTED_RPAREN_AFTER_ARGLIST))
							goto err_anno;
					} else {
						if unlikely(parser_warn_pack_used(&packloc))
							goto err;
						if unlikely(parse_arglist())
							goto err_anno;
					}
					if (parser_flags & PARSE_FLFSTMT)
						TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
				} else {
					if (WARN(W_DEPRECATED_NO_PARAMETER_LIST))
						goto err_anno;
				}
				if (skip_lf())
					goto err_anno;
				if (tok == ':') {
					/* Constructor initializers (including super-initializers). */
					if unlikely(yield() < 0)
						goto err_anno;
					TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
					if unlikely(parse_constructor_initializers(&maker))
						goto err_anno;
					if (parser_flags & PARSE_FLFSTMT)
						TPPLexer_Current->l_flags |= TPPLEXER_FLAG_WANTLF;
				}

				/* Compile the actual constructor in a sub-scope so any local
				 * variables that it may define will not interfere with the
				 * lookup rules in member initializers that may still follow. */
				if (scope_push())
					goto err_anno;

				/* NOTE: Don't parse a second argument list here. */
				maker.cm_ctor = ast_setddi(ast_parse_function_noscope_noargs(&need_semi), &loc);
				scope_pop();
				basescope_pop();
				if unlikely(!maker.cm_ctor)
					goto err_anno;
				maker.cm_ctor = ast_annotations_apply(&annotations, maker.cm_ctor);
				if unlikely(!maker.cm_ctor)
					goto err;

				/* With the constructor parsed and saved, simply check for a ';' */
				goto check_need_semi;
			}

			/* Automatic member:
			 * >> class Foo {
			 * >>     field_member   [= initializer];
			 * >>     property_member = { ... }
			 * >>     function_member() { ... }
			 * >> }; */
			if unlikely(yield() < 0)
				goto err;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
			decl.da_type = DAST_NONE;
			if (tok == ':') {
				if unlikely(yield() < 0)
					goto err;
				if unlikely(decl_ast_parse(&decl))
					goto err;
			}
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
			if (is_semicolon()) {
				if (member_class == MEMBER_CLASS_AUTO) {
					if (WARNAT(&loc, W_IMPLICIT_MEMBER_DECLARATION, member_name))
						goto err_decl;
				} else if (member_class != MEMBER_CLASS_MEMBER) {
					if (WARNAT(&loc, W_CLASS_MEMBER_TYPE_NOT_ASSIGNED, member_name))
						goto err_decl;
				}

				/* Uninitialized instance/class member. */
				if (!class_maker_addmember(&maker,
				                           member_name,
				                           is_class_member,
				                           member_flags,
				                           &p_usage_counter,
				                           &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
				                           ,
				                           &decl
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
				                           ))
					goto err_decl;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
				decl_ast_fini(&decl);
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
				++*p_usage_counter;
				if unlikely(yield_semicolon() < 0)
					goto err;
				break;
			}
			if (tok == '=') {
				/* Property or member assign. */
				if unlikely(yield() < 0)
					goto err_decl;
				if (tok == '{') {
					DREF struct ast *prop_callbacks[CLASS_GETSET_COUNT];
					uint16_t i, prop_addr;
					if (member_class != MEMBER_CLASS_AUTO &&
					    member_class != MEMBER_CLASS_GETSET &&
					    WARNAT(&loc, W_CLASS_MEMBER_TYPE_NOT_MATCHED, member_name))
						goto err_decl;
					if (varying_encountered &&
					    WARNAT(&loc, W_CLASS_MEMBER_VARYING_IGNORED_FOR_PROPERTY, member_name))
						goto err_decl;
					if unlikely(yield() < 0)
						goto err_decl;

					/* Properties are always allocated in class memory and have the FPROPERTY flag set. */
					member_flags |= (CLASS_ATTRIBUTE_FCLASSMEM | CLASS_ATTRIBUTE_FGETSET);

					/* Set the method flag to invoke property callbacks as this-call functions. */
					if (!is_class_member)
						member_flags |= CLASS_ATTRIBUTE_FMETHOD;

					/* Define the property symbol. */
					member_symbol = class_maker_addmember(&maker,
					                                      member_name,
					                                      is_class_member,
					                                      member_flags,
					                                      &p_usage_counter,
					                                      &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
					                                      ,
					                                      &decl
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
					                                      );
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
					decl_ast_fini(&decl);
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
					if unlikely(!member_symbol)
						goto err;

					/* Parse the property declaration. */
					if unlikely(parse_property(prop_callbacks, &maker, is_class_member))
						goto err;
					if (skip('}', W_EXPECTED_RBRACE_AFTER_PROPERTY))
						goto err_property;

					/* Keep track of VTABLE slots used by the property and its callbacks. */
					if (!prop_callbacks[CLASS_GETSET_DEL] &&
					    !prop_callbacks[CLASS_GETSET_SET]) {
						/* Optimization: when no delete or setting callback
						 *               was given, mark the symbol as read-only. */
						member_symbol->s_attr.a_attr->ca_flag |= CLASS_ATTRIBUTE_FREADONLY;
						*p_usage_counter += 1;
					} else {
						*p_usage_counter += CLASS_GETSET_COUNT;
					}

					/* Load the base address of the property. */
					prop_addr = member_symbol->s_attr.a_attr->ca_addr;
					for (i = 0; i < COMPILER_LENOF(prop_callbacks); ++i) {
						/* Skip callbacks that have not been defined. */
						if (!prop_callbacks[i])
							continue;
						member_flags &= (CLASS_ATTRIBUTE_FVISIBILITY |
						                 CLASS_ATTRIBUTE_FCLASSMEM |
						                 CLASS_ATTRIBUTE_FMETHOD);

						/* Add an initializer for the address. */
						error = class_maker_addanon(&maker,
						                            prop_addr + i,
						                            prop_callbacks[i]);
					}

					/* Clear out property callbacks. */
					for (i = 0; i < COMPILER_LENOF(prop_callbacks); ++i)
						ast_xdecref(prop_callbacks[i]);
					break;
err_property:
					for (i = 0; i < COMPILER_LENOF(prop_callbacks); ++i)
						ast_xdecref(prop_callbacks[i]);
					goto err;
				}
				if (member_class != MEMBER_CLASS_AUTO &&
				    member_class != MEMBER_CLASS_MEMBER &&
				    WARNAT(&loc, W_CLASS_MEMBER_TYPE_NOT_MATCHED, member_name))
					goto err_decl;
				member_symbol = class_maker_addmember(&maker,
				                                      member_name,
				                                      is_class_member,
				                                      member_flags,
				                                      &p_usage_counter,
				                                      &loc
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
				                                      ,
				                                      &decl
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
				                                      );
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
				decl_ast_fini(&decl);
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
				if unlikely(!member_symbol)
					goto err;

				/* Member assignment. (part of the initialization) */
				if (SYM_ISCLASSMEMBER(member_symbol)) {
					/* Class member. */
					init_ast = ast_parse_expr(LOOKUP_SYM_NORMAL);
				} else {
					/* Instance member. */
					if (class_maker_push_ctorscope(&maker))
						goto err;
					init_ast = ast_parse_expr(LOOKUP_SYM_NORMAL);
					basescope_pop();
				}
				if unlikely(!init_ast)
					goto err;

				/* Add an initializer for this symbol. */
				error = class_maker_addinit(&maker, member_symbol, init_ast, &loc);
				ast_decref(init_ast);
				if unlikely(error)
					goto err;

				/* Increment the usage-counter to consume the member slot. */
				++*p_usage_counter;
				if unlikely(likely(is_semicolon())
				            ? (yield_semicolon() < 0)
				            : WARN(W_EXPECTED_SEMICOLON_AFTER_EXPRESSION))
					goto err;
				break;
			}
			if (member_class != MEMBER_CLASS_AUTO &&
			    member_class != MEMBER_CLASS_METHOD &&
			    WARNAT(&loc, W_CLASS_MEMBER_TYPE_NOT_MATCHED, member_name))
				goto err_decl;
			if (varying_encountered &&
			    WARNAT(&loc, W_CLASS_MEMBER_VARYING_IGNORED_FOR_FUNCTION, member_name))
				goto err_decl;

			/* Everything else is parsed as a member function.
			 * NOTE: We mark such members are read-only because the intention is to never overwrite them. */
			member_flags |= (CLASS_ATTRIBUTE_FCLASSMEM | CLASS_ATTRIBUTE_FREADONLY);
			if (!is_class_member)
				member_flags |= CLASS_ATTRIBUTE_FMETHOD;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
			{
				struct ast_tags_printers temp;
				if (decl.da_type != DAST_NONE) {
					/* TODO: Warn if declaration information has already been loaded! */
					decl_ast_fini(&decl);
					decl.da_type = DAST_NONE;
				}

				/* Must load the argument list of the following function
				 * now, so we can include declaration information. */
				member_symbol = class_maker_addmember(&maker,
				                                      member_name,
				                                      is_class_member,
				                                      member_flags,
				                                      &p_usage_counter,
				                                      &loc,
				                                      NULL);
				if unlikely(!member_symbol)
					goto err_decl;
				ast_annotations_get(&annotations);
				if (is_class_member) {
					/* Parse a new function in the class scope. */
					AST_TAGS_BACKUP_PRINTERS(temp);
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
					if unlikely(basescope_push())
						goto err_anno;
					current_basescope->bs_flags |= current_tags.at_code_flags;
					init_ast = ast_parse_function_noscope(member_name, &need_semi, false, &loc, &decl);
					if (init_ast && unlikely(doctext_compile(&temp.at_doc))) {
						ast_decref(init_ast);
						init_ast = NULL;
					}
					basescope_pop();
#else /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
					init_ast = ast_parse_function(member_name, &need_semi, false, &loc, &decl);
#endif /* !CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
					AST_TAGS_RESTORE_PRINTERS(temp);
				} else {
					if unlikely(class_maker_push_methscope(&maker))
						goto err;

					/* Parse a new function in its own member-method scope. */
					AST_TAGS_BACKUP_PRINTERS(temp);
					init_ast = ast_parse_function_noscope(member_name, &need_semi, false, &loc, &decl);
					AST_TAGS_RESTORE_PRINTERS(temp);
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
					if (init_ast && unlikely(doctext_compile(&current_tags.at_doc))) {
						ast_decref(init_ast);
						init_ast = NULL;
					}
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
					basescope_pop();
				}
				if unlikely(!init_ast)
					goto err_anno;

				/* Remember declaration information within the function member */
				if (member_symbol->s_decltype.da_type == DAST_NONE) {
					decl_ast_move(&member_symbol->s_decltype, &decl);
				} else {
					decl_ast_fini(&decl);
				}

				/* Generate the documentation string for the function member. */
				if (!member_symbol->s_attr.a_attr->ca_doc) {
					member_symbol->s_attr.a_attr->ca_doc = (DREF DeeStringObject *)ast_tags_doc(&member_symbol->s_decltype);
					if unlikely(!member_symbol->s_attr.a_attr->ca_doc)
						goto err;
				}
			}
#else /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
			member_symbol = class_maker_addmember(&maker,
			                                      member_name,
			                                      is_class_member,
			                                      member_flags,
			                                      &p_usage_counter,
			                                      &loc);
			if unlikely(!member_symbol)
				goto err;
			ast_annotations_get(&annotations);
			if (is_class_member) {
				/* Parse a new function in the class scope. */
				init_ast = ast_parse_function(member_name, &need_semi, false, &loc);
			} else {
				if unlikely(class_maker_push_methscope(&maker))
					goto err;
				/* Parse a new function in its own member-method scope. */
				init_ast = ast_parse_function_noscope(member_name, &need_semi, false, &loc);
				basescope_pop();
			}
			if unlikely(!init_ast)
				goto err_anno;
#endif /* !CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
			init_ast = ast_annotations_apply(&annotations, init_ast);
			if unlikely(!init_ast)
				goto err;

			/* Add the given AST as an initialization branch to the class. */
			error = class_maker_addinit(&maker, member_symbol, init_ast, &loc);
			ast_decref(init_ast);
			if unlikely(error)
				goto err;
			++*p_usage_counter;
check_need_semi:
			if (need_semi) {
do_yield_semicolon:
				if unlikely(likely(is_semicolon())
				            ? (yield_semicolon() < 0)
				            : WARN(W_EXPECTED_SEMICOLON_AFTER_EXPRESSION))
					goto err;
			}
		}	break;

		}
	}
done_class_modal:
	if unlikely(link_forward_symbols())
		goto err;
	result = class_maker_pack(&maker);
	scope_pop();
	class_maker_fini(&maker);
	return result;
#ifdef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
err_decl:
	decl_ast_fini(&decl);
	goto err;
#endif /* CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
err_anno:
	ast_annotations_free(&annotations);
#ifndef CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION
err_decl:
#endif /* !CONFIG_LANGUAGE_DECLARATION_DOCUMENTATION */
err:
	TPPLexer_Current->l_flags &= ~TPPLEXER_FLAG_WANTLF;
	TPPLexer_Current->l_flags |= old_flags & TPPLEXER_FLAG_WANTLF;
	class_maker_fini(&maker);
	return NULL;
}

/* Parse a class definition, starting at the `{' token (or at `:' when a base exists).
 * The returned AST is of type `AST_CLASS' (create_symbol == false) or `AST_STORE' (create_symbol == true).
 * @param: class_flags:   Set of `TP_F* & 0xf'
 * @param: create_symbol: When true, assign the class to its own symbol (also requiring that `name' != NULL).
 * @param: symbol_mode:   The mode with which to create the class symbol. */
INTERN WUNUSED DREF struct ast *DCALL
ast_parse_class(uint16_t class_flags, struct TPPKeyword *name,
                bool create_symbol, unsigned int symbol_mode) {
	DREF struct ast *result;
	struct ast_annotations annotations;
	ast_annotations_get(&annotations);
	result = ast_parse_class_impl(class_flags, name, create_symbol, symbol_mode);
	if unlikely(!result)
		goto err_anno;
	if (annotations.an_annoc && create_symbol &&
	    result->a_class.c_classsym != NULL) {
		DREF struct ast *class_sym, *merge;
		class_sym = ast_setddi(ast_sym(result->a_class.c_classsym), &result->a_ddi);
		if unlikely(!class_sym)
			goto err_anon_r;
		result = ast_annotations_apply(&annotations, result);
		if unlikely(!result) {
			ast_decref(class_sym);
			goto err;
		}
		merge = ast_setddi(ast_action2(AST_FACTION_STORE, class_sym, result), &class_sym->a_ddi);
		ast_decref(class_sym);
		ast_decref(result);
		return merge;
	}
	return ast_annotations_apply(&annotations, result);
err_anon_r:
	ast_decref(result);
err_anno:
	ast_annotations_free(&annotations);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_COMPILER_LEXER_CLASS_C */
