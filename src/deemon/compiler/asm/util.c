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
#ifndef GUARD_DEEMON_COMPILER_ASM_UTIL_C
#define GUARD_DEEMON_COMPILER_ASM_UTIL_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

#include "../../runtime/builtin.h"

DECL_BEGIN

INTERN WUNUSED int DCALL
asm_gpush_stack(uint16_t absolute_stack_addr) {
	uint16_t offset;
	ASSERTF(current_assembler.a_stackcur > absolute_stack_addr,
	        "Invalid stack address");
	offset = (current_assembler.a_stackcur - 1) - absolute_stack_addr;
	return offset == 0 ? asm_gdup() : asm_gdup_n(offset - 1);
}

INTERN WUNUSED int DCALL
asm_gpop_stack(uint16_t absolute_stack_addr) {
	uint16_t offset;
	ASSERTF(current_assembler.a_stackcur > absolute_stack_addr,
	        "Invalid stack address");
	offset = (current_assembler.a_stackcur - 1) - absolute_stack_addr;
	/* XXX: `offset == 0' doesn't ~really~ make sense as that would
	 *       mean to pop a stack value into itself, then discard it.
	 *       Though we still allow doing this... */
	return offset == 0 ? asm_gpop() : asm_gpop_n(offset - 1);
}

INTERN WUNUSED int DCALL
asm_gadjstack(int16_t offset) {
	switch (offset) {
	case -1: return asm_gpop();
	case 0: return 0;
	case 1: return asm_gpush_none();
	}
	return _asm_gadjstack(offset);
}

INTERN WUNUSED int DCALL
asm_gsetstack(uint16_t absolute_stack_size) {
	return asm_gadjstack((int16_t)absolute_stack_size -
	                     (int16_t)current_assembler.a_stackcur);
}

INTERN WUNUSED int DCALL
asm_glrot(uint16_t num_slots) {
	if (num_slots <= 1)
		return 0;
	if (num_slots == 2)
		return asm_gswap();
	return _asm_glrot(num_slots - 3);
}

INTERN WUNUSED int DCALL
asm_grrot(uint16_t num_slots) {
	if (num_slots <= 1)
		return 0;
	if (num_slots == 2)
		return asm_gswap();
	return _asm_grrot(num_slots - 3);
}

INTERN WUNUSED int DCALL
asm_gpush_u32(uint32_t value) {
	DREF DeeObject *obj;
	int32_t cid;
	obj = DeeInt_NewU32(value);
	if unlikely(!obj)
		goto err;
	cid = asm_newconst(obj);
	Dee_Decref(obj);
	if unlikely(cid < 0)
		goto err;
	return asm_gpush_const((uint16_t)cid);
err:
	return -1;
}

INTERN WUNUSED int DCALL
asm_gpush_s32(int32_t value) {
	DREF DeeObject *obj;
	int32_t cid;
	obj = DeeInt_NewS32(value);
	if unlikely(!obj)
		goto err;
	cid = asm_newconst(obj);
	Dee_Decref(obj);
	if unlikely(cid < 0)
		goto err;
	return asm_gpush_const((uint16_t)cid);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
tuple_getrange_i(DeeTupleObject *__restrict self,
                 dssize_t begin, dssize_t end);

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
list_getrange_as_tuple(DeeListObject *__restrict self,
                       dssize_t begin, dssize_t end) {
	DREF DeeTupleObject *result;
	size_t i;
again:
	DeeList_LockRead(self);
	if unlikely(begin < 0)
		begin += DeeList_SIZE(self);
	if unlikely(end < 0)
		end += DeeList_SIZE(self);
	if unlikely((size_t)begin >= DeeList_SIZE(self) ||
	            (size_t)begin >= (size_t)end) {
		/* Empty list. */
		DeeList_LockEndRead(self);
		return DeeList_New();
	}
	if unlikely((size_t)end > DeeList_SIZE(self))
		end = (dssize_t)DeeList_SIZE(self);
	end -= begin;
	ASSERT(end != 0);
	result = (DREF DeeTupleObject *)DeeObject_TryMalloc(offsetof(DeeTupleObject, t_elem) +
	                                                    (size_t)end * sizeof(DREF DeeObject *));
	if unlikely(!result) {
		DeeList_LockEndRead(self);
		if (Dee_CollectMemory(offsetof(DeeTupleObject, t_elem) +
		                      (size_t)end * sizeof(DREF DeeObject *)))
			goto again;
		return NULL;
	}
	/* Copy vector elements. */
	for (i = 0; i < (size_t)end; ++i) {
		result->t_elem[i] = DeeList_GET(self, (size_t)begin + i);
		Dee_Incref(result->t_elem[i]);
	}
	DeeList_LockEndRead(self);
	DeeObject_Init(result, &DeeTuple_Type);
	result->t_size = (size_t)end;
	return (DREF DeeObject *)result;
}


/* NOTE: _Always_ inherits references to `key' and `value' */
PRIVATE NONNULL((1, 3, 4)) void DCALL
rodict_insert_nocheck(DeeRoDictObject *__restrict self,
                      dhash_t hash,
                      DREF DeeObject *__restrict key,
                      DREF DeeObject *__restrict value) {
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = RODICT_HASHST(self, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->di_key)
			break;
	}
	/* Fill in the item. */
	item->di_hash  = hash;
	item->di_key   = key;   /* Inherit reference. */
	item->di_value = value; /* Inherit reference. */
}


/* NOTE: _Always_ inherits references to `key' */
PRIVATE NONNULL((1, 3)) void DCALL
roset_insert_nocheck(DeeRoSetObject *__restrict self,
                     dhash_t hash,
                     DREF DeeObject *__restrict key) {
	size_t i, perturb;
	struct roset_item *item;
	perturb = i = ROSET_HASHST(self, hash);
	for (;; ROSET_HASHNX(i, perturb)) {
		item = ROSET_HASHIT(self, i);
		if (!item->si_key)
			break;
	}
	/* Fill in the item. */
	item->si_hash = hash;
	item->si_key  = key; /* Inherit reference. */
}

#define dummy (&DeeDict_Dummy)
#define SIZEOF_RODICT(mask) \
	(offsetof(DeeRoDictObject, rd_elem) + (((mask) + 1) * sizeof(struct rodict_item)))
#define RODICT_INITIAL_MASK 0x1f

#define SIZEOF_ROSET(mask) \
	(offsetof(DeeRoSetObject, rs_elem) + (((mask) + 1) * sizeof(struct roset_item)))
#define ROSET_INITIAL_MASK 0x1f



#define STACK_PACK_THRESHOLD 16
INTERN WUNUSED NONNULL((1)) int
(DCALL asm_gpush_constexpr)(DeeObject *__restrict value) {
	int32_t cid;
	ASSERT_OBJECT(value);
	if (DeeBool_Check(value)) {
		/* Push a boolean builtin singleton. */
		return asm_gpush_bool(DeeBool_IsTrue(value));
	}
	if (DeeNone_Check(value))
		return asm_gpush_none();
	if (DeeSuper_Check(value)) {
		/* Construct a super-wrapper. */
		if (asm_gpush_constexpr((DeeObject *)DeeSuper_SELF(value)))
			goto err;
		if (asm_gpush_constexpr((DeeObject *)DeeSuper_TYPE(value)))
			goto err;
		return asm_gsuper();
	}
	if (DeeTuple_Check(value)) {
		/* Special case for tuples: If only constant expressions are
		 * contained, then we can push the tuple as a constant expression, too. */
		size_t start, end, len = DeeTuple_SIZE(value);
		bool is_first_part;
		/* Special case: empty tuple. */
		if (!len)
			return asm_gpack_tuple(0);
		/* Check if the tuple can be pushed as a constant expression. */
		for (start = 0; start < len; ++start) {
			if (!asm_allowconst(DeeTuple_GET(value, start)))
				goto push_tuple_parts;
		}
		goto push_constexpr;
push_tuple_parts:
		/* If not, push constant parts as individual segments. */
		end = start, start = 0;
		is_first_part = start == end;
		if (!is_first_part) {
			DREF DeeObject *subrange;
			subrange = tuple_getrange_i((DeeTupleObject *)value,
			                            (dssize_t)start, (dssize_t)end);
			if unlikely(!subrange)
				goto err;
			cid = asm_newconst(subrange);
			Dee_Decref(subrange);
			if unlikely(cid < 0)
				goto err;
			if (asm_gpush_const((uint16_t)cid))
				goto err;
		}
		while (end < len) {
			uint16_t pack_length = 0;
			while (end < len && pack_length < STACK_PACK_THRESHOLD &&
			       !asm_allowconst(DeeTuple_GET(value, end))) {
				if (asm_gpush_constexpr(DeeTuple_GET(value, end)))
					goto err;
				++pack_length, ++end;
			}
			ASSERT(pack_length != 0);
			ASSERT(pack_length <= STACK_PACK_THRESHOLD);
			if (is_first_part) {
				is_first_part = false;
				if (asm_gpack_tuple(pack_length))
					goto err;
			} else {
				if (asm_gextend(pack_length))
					goto err;
			}
			start = end;
			/* Collect constant parts. */
			while (end < len && asm_allowconst(DeeTuple_GET(value, end)))
				++end;
			if (end > start) {
				DREF DeeObject *subrange;
				if (end == start + 1) {
					if (asm_gpush_constexpr(DeeTuple_GET(value, start)))
						goto err;
					if (asm_gextend(1))
						goto err;
				} else {
					subrange = tuple_getrange_i((DeeTupleObject *)value,
					                            (dssize_t)start, (dssize_t)end);
					if unlikely(!subrange)
						goto err;
					cid = asm_newconst(subrange);
					Dee_Decref(subrange);
					if unlikely(cid < 0)
						goto err;
					if (asm_gpush_const((uint16_t)cid))
						goto err;
					if (asm_gconcat())
						goto err;
				}
			}
		}
		return 0;
	}
	if (DeeList_CheckExact(value)) {
		size_t start, end;
		int temp;
		bool is_first_part = true;
		DeeList_LockRead(value);
		/* Special case: empty list. */
		if (!DeeList_SIZE(value)) {
			DeeList_LockEndRead(value);
			return asm_gpack_list(0);
		}
		end = 0;
		while (end < DeeList_SIZE(value)) {
			uint16_t pack_length = 0;
			while (end < DeeList_SIZE(value) &&
			       pack_length < STACK_PACK_THRESHOLD) {
				DeeObject *item = DeeList_GET(value, end);
				if (asm_allowconst(item))
					break;
				Dee_Incref(item);
				DeeList_LockEndRead(value);
				temp = asm_gpush_constexpr(item);
				Dee_Decref(item);
				if unlikely(temp)
					goto err;
				DeeList_LockRead(value);
				++pack_length, ++end;
			}
			if (pack_length) {
				ASSERT(pack_length <= STACK_PACK_THRESHOLD);
				DeeList_LockEndRead(value);
				if (is_first_part) {
					if (asm_gpack_list(pack_length))
						goto err;
					is_first_part = false;
				} else {
					/* Append this part to the previous portion. */
					if (asm_gextend(pack_length))
						goto err;
				}
				DeeList_LockRead(value);
			}
			/* Collect constant parts. */
			start = end;
			while (end < DeeList_SIZE(value) &&
			       asm_allowconst(DeeList_GET(value, end)))
				++end;
			if (end > start) {
				DREF DeeObject *subrange;
				if (end == start + 1) {
					/* Special case: encode as  `pack List, #1' */
					subrange = DeeList_GET(value, start);
					Dee_Incref(subrange);
					DeeList_LockEndRead(value);
					temp = asm_gpush_constexpr(subrange);
					Dee_Decref(subrange);
					if unlikely(temp)
						goto err;
					if (is_first_part) {
						if (asm_gpack_list(1))
							goto err;
						is_first_part = false;
					} else {
						if (asm_gextend(1))
							goto err;
					}
				} else {
					DeeList_LockEndRead(value);
					subrange = list_getrange_as_tuple((DeeListObject *)value,
					                                  (dssize_t)start, (dssize_t)end);
					if unlikely(!subrange)
						goto err;
					cid = asm_newconst(subrange);
					Dee_Decref(subrange);
					if unlikely(cid < 0)
						goto err;
					if (asm_gpush_const((uint16_t)cid))
						goto err;
					if (is_first_part) {
						if (asm_gcast_list())
							goto err;
						is_first_part = false;
					} else {
						if (asm_gconcat())
							goto err;
					}
				}
				DeeList_LockRead(value);
			}
		}
		return 0;
	}
#if 0 /* There's not really an intended way of creating these at runtime anyways, \
       * and these types of sequence objects should only really be created when   \
       * all contained items qualify as constants anyways, so we might as well    \
       * forget about this (for now) */
	if (DeeRoDict_Check(value)) {
		/* TO-DO: Check if all contained key/value-pairs are allocated as constants. */
	}
	if (DeeRoSet_Check(value)) {
		/* TO-DO: Check if all contained keys are allocated as constants. */
	}
#endif
	if (DeeDict_Check(value)) {
		/* Construct dicts in one of 2 ways:
		 *   #1: If all Dict elements are allocated as constants,
		 *       push as a read-only Dict, then cast to a regular
		 *       Dict.
		 *   #2: Otherwise, push all Dict key/item pairs manually,
		 *       before packing everything together as a Dict. */
		size_t i, mask, ro_mask, num_items;
		struct dict_item *elem;
		DREF DeeRoDictObject *rodict;
check_dict_again:
		DeeDict_LockRead(value);
		if (!((DeeDictObject *)value)->d_used) {
			/* Simple case: The Dict is empty, so we can just pack an empty Dict at runtime. */
			DeeDict_LockEndRead(value);
			return asm_gpack_dict(0);
		}
		mask = ((DeeDictObject *)value)->d_mask;
		elem = ((DeeDictObject *)value)->d_elem;
		for (i = 0; i <= mask; ++i) {
			if (!elem[i].di_key)
				continue;
			if (elem[i].di_key == dummy)
				continue;
			if (!asm_allowconst(elem[i].di_key) ||
			    !asm_allowconst(elem[i].di_value)) {
				DeeDict_LockEndRead(value);
				goto push_dict_parts;
			}
		}
		num_items = ((DeeDictObject *)value)->d_used;
		ro_mask   = RODICT_INITIAL_MASK;
		while (ro_mask <= num_items)
			ro_mask = (ro_mask << 1) | 1;
		ro_mask = (ro_mask << 1) | 1;
		rodict  = (DREF DeeRoDictObject *)DeeObject_TryCalloc(SIZEOF_RODICT(ro_mask));
		if unlikely(!rodict) {
			DeeDict_LockEndRead(value);
			if (Dee_CollectMemory(SIZEOF_RODICT(ro_mask)))
				goto check_dict_again;
			goto err;
		}
		rodict->rd_size = num_items;
		rodict->rd_mask = ro_mask;
		/* Pack all key-value pairs into the ro-Dict. */
		for (i = 0; i <= mask; ++i) {
			if (!elem[i].di_key)
				continue;
			if (elem[i].di_key == dummy)
				continue;
			Dee_Incref(elem[i].di_key);
			Dee_Incref(elem[i].di_value);
			rodict_insert_nocheck(rodict,
			                      elem[i].di_hash,
			                      elem[i].di_key,
			                      elem[i].di_value);
		}
		DeeDict_LockEndRead(value);
		DeeObject_Init(rodict, &DeeRoDict_Type);
		/* All right! we've got the ro-Dict all packed together!
		 * -> Register it as a constant. */
		cid = asm_newconst((DeeObject *)rodict);
		Dee_Decref(rodict);
		if unlikely(cid < 0)
			goto err;
		/* Now push the ro-Dict, then cast it to a regular one. */
		if (asm_gpush_const((uint16_t)cid))
			goto err;
		if (asm_gcast_dict())
			goto err;
		return 0;
push_dict_parts:
		/* Construct a Dict by pushing its individual parts. */
		num_items = 0;
		DeeDict_LockRead(value);
		for (i = 0; i <= ((DeeDictObject *)value)->d_mask; ++i) {
			struct dict_item *item;
			int error;
			DREF DeeObject *item_key, *item_value;
			item     = &((DeeDictObject *)value)->d_elem[i];
			item_key = item->di_key;
			if (!item_key || item_key == dummy)
				continue;
			item_value = item->di_value;
			Dee_Incref(item_key);
			Dee_Incref(item_value);
			DeeDict_LockEndRead(value);
			/* Push the key & item. */
			error = asm_gpush_constexpr(item_key);
			if likely(!error)
				error = asm_gpush_constexpr(item_value);
			Dee_Decref(item_value);
			Dee_Decref(item_key);
			if unlikely(error)
				goto err;
			++num_items;
			DeeDict_LockRead(value);
		}
		DeeDict_LockEndRead(value);
		/* With everything pushed, pack together the Dict. */
		return asm_gpack_dict((uint16_t)num_items);
	}
	if (DeeHashSet_Check(value)) {
		/* Construct hash-sets in one of 2 ways:
		 *   #1: If all set elements are allocated as constants,
		 *       push as a read-only set, then cast to a regular
		 *       set.
		 *   #2: Otherwise, push all set keys manually, before
		 *       packing everything together as a set. */
		size_t i, mask, ro_mask, num_items;
		struct hashset_item *elem;
		DREF DeeRoSetObject *roset;
check_set_again:
		DeeHashSet_LockRead(value);
		if (!((DeeHashSetObject *)value)->s_used) {
			/* Simple case: The set is empty, so we can just pack an empty HashSet at runtime. */
			DeeHashSet_LockRead(value);
			return asm_gpack_hashset(0);
		}
		mask = ((DeeHashSetObject *)value)->s_mask;
		elem = ((DeeHashSetObject *)value)->s_elem;
		for (i = 0; i <= mask; ++i) {
			if (!elem[i].si_key)
				continue;
			if (elem[i].si_key == dummy)
				continue;
			if (!asm_allowconst(elem[i].si_key)) {
				DeeHashSet_LockEndRead(value);
				goto push_set_parts;
			}
		}
		num_items = ((DeeHashSetObject *)value)->s_used;
		ro_mask   = ROSET_INITIAL_MASK;
		while (ro_mask <= num_items)
			ro_mask = (ro_mask << 1) | 1;
		ro_mask = (ro_mask << 1) | 1;
		roset   = (DREF DeeRoSetObject *)DeeObject_TryCalloc(SIZEOF_ROSET(ro_mask));
		if unlikely(!roset) {
			DeeHashSet_LockEndRead(value);
			if (Dee_CollectMemory(SIZEOF_ROSET(ro_mask)))
				goto check_set_again;
			goto err;
		}
		roset->rs_size = num_items;
		roset->rs_mask = ro_mask;
		/* Pack all key-value pairs into the ro-set. */
		for (i = 0; i <= mask; ++i) {
			if (!elem[i].si_key)
				continue;
			if (elem[i].si_key == dummy)
				continue;
			Dee_Incref(elem[i].si_key); /* Inherited by `roset_insert_nocheck()' */
			roset_insert_nocheck(roset,
			                     elem[i].si_hash,
			                     elem[i].si_key);
		}
		DeeHashSet_LockEndRead(value);
		DeeObject_Init(roset, &DeeRoSet_Type);
		/* All right! we've got the ro-set all packed together!
		 * -> Register it as a constant. */
		cid = asm_newconst((DeeObject *)roset);
		Dee_Decref(roset);
		if unlikely(cid < 0)
			goto err;
		/* Now push the ro-set, then cast it to a regular one. */
		if (asm_gpush_const((uint16_t)cid))
			goto err;
		if (asm_gcast_hashset())
			goto err;
		return 0;
push_set_parts:
		/* Construct a set by pushing its individual parts. */
		num_items = 0;
		DeeHashSet_LockRead(value);
		for (i = 0; i <= ((DeeHashSetObject *)value)->s_mask; ++i) {
			struct hashset_item *item;
			int error;
			DREF DeeObject *item_key;
			item     = &((DeeHashSetObject *)value)->s_elem[i];
			item_key = item->si_key;
			if (!item_key || item_key == dummy)
				continue;
			Dee_Incref(item_key);
			DeeHashSet_LockEndRead(value);
			/* Push the key & item. */
			error = asm_gpush_constexpr(item_key);
			Dee_Decref(item_key);
			if unlikely(error)
				goto err;
			++num_items;
			DeeHashSet_LockRead(value);
		}
		DeeHashSet_LockEndRead(value);
		/* With everything pushed, pack together the set. */
		return asm_gpack_hashset((uint16_t)num_items);
	}

push_constexpr:
	/* Fallback: Push a constant variable  */
	cid = asm_newconst(value);
	if unlikely(cid < 0)
		goto err;
	return asm_gpush_const((uint16_t)cid);
err:
	return -1;
}


INTERN WUNUSED NONNULL((1, 2)) int DCALL
asm_check_thiscall(struct symbol *__restrict sym,
                   struct ast *__restrict warn_ast) {
	/* Throw a compiler-error when one attempts to
	 * access an instance member from a class method.
	 * We must check this now because at runtime, the fast-mode interpreter
	 * assumes that a this-argument is present when an instruction using it
	 * is encountered, while the safe-mode interpreter throws an error if not.
	 * But since we're trying to generate code for fast-mode, we need to check this now. */
	if (asm_symbol_accessible(sym))
		return 0;
	/* Hint to the user that they may write `MyClass.symbol' instead of
	 * `symbol', if they to access the attribute in its unbound form:
	 * >> class MyClass {
	 * >>     func() {
	 * >>         print "Hello";
	 * >>     }
	 * >>     static classfun() {
	 * >>         local x = MyClass();
	 * >>         func(x);         // Illegal (this is when we get here)
	 * >>         MyClass.func(x); // Allowed (this is how you access the unbound function)
	 * >>     }
	 * >> }
	 */
	return PERRAST(warn_ast, W_ASM_INSTANCE_MEMBER_FROM_CLASS_METHOD, sym,
	               current_basescope->bs_name ? current_basescope->bs_name->k_name : "?");
}

/* Generate a call to `function' that pops `num_args' arguments from the stack,
 * then pushes its return value back onto the stack. */
INTERN WUNUSED NONNULL((1, 3)) int
(DCALL asm_gcall_symbol_n)(struct symbol *__restrict function, uint8_t argc,
                           struct ast *__restrict warn_ast) {
	int32_t symid;
	/* Attempt a direct call to the symbol. */
check_function_class:
	if (!SYMBOL_MUST_REFERENCE(function)) {
		switch (function->s_type) {

		case SYMBOL_TYPE_ALIAS:
			function = function->s_alias;
			goto check_function_class;

		case SYMBOL_TYPE_EXTERN:
			ASSERT(function->s_extern.e_module);
			ASSERT(function->s_extern.e_symbol);
			if (function->s_extern.e_symbol->ss_flags & MODSYM_FPROPERTY)
				break;
			if (function->s_extern.e_symbol->ss_flags & MODSYM_FEXTERN) {
				symid = function->s_extern.e_symbol->ss_extern.ss_impid;
				ASSERT(symid < function->s_extern.e_module->mo_importc);
				symid = asm_newmodule(function->s_extern.e_module->mo_importv[symid]);
			} else {
				symid = asm_esymid(function);
			}
			if unlikely(symid < 0)
				goto err;
			if (asm_gcall_extern((uint16_t)symid, function->s_extern.e_symbol->ss_index, argc))
				goto err;
			goto done;

		case SYMBOL_TYPE_GLOBAL:
			if unlikely((symid = asm_gsymid_for_read(function, warn_ast)) < 0)
				goto err;
			return asm_gcall_global((uint16_t)symid, argc);

		case SYMBOL_TYPE_LOCAL:
			if unlikely((symid = asm_lsymid_for_read(function, warn_ast)) < 0)
				goto err;
			return asm_gcall_local((uint16_t)symid, argc);

		case SYMBOL_TYPE_CATTR: {
			struct symbol *class_sym, *this_sym;
			struct class_attribute *attr;
			int32_t symid2;
			class_sym = function->s_attr.a_class;
			this_sym  = function->s_attr.a_this;
			attr      = function->s_attr.a_attr;
			SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
			if (!this_sym) {
				if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
					symid = asm_rsymid(class_sym);
					if unlikely(symid < 0)
						goto err;
					if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr)) /* args..., func */
						goto err;
				} else {
					if (asm_gpush_symbol(class_sym, warn_ast)) /* args..., class_sym */
						goto err;
					if (asm_ggetcmember(attr->ca_addr))        /* args..., func */
						goto err;
				}
				if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					/* Must invoke the getter callback. */
					if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
						if (asm_gpush_symbol(class_sym, warn_ast))
							goto err; /* args..., getter, class */
						if (asm_gcall(1))
							goto err; /* args..., func */
					} else {
						if (asm_gcall(0))
							goto err; /* args..., func */
					}
				} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					if (argc != (uint8_t)-1) {
						if (asm_grrot(argc + 1))
							goto err; /* func, args... */
						if (asm_gpush_symbol(class_sym, warn_ast))
							goto err; /* func, args..., class_sym */
						if (asm_grrot(argc + 1))
							goto err;               /* func, class_sym, args... */
						return asm_gcall(argc + 1); /* result */
					}
					if (asm_gpush_symbol(class_sym, warn_ast))
						goto err; /* args..., func, class_sym */
					symid = asm_newmodule(DeeModule_GetDeemon());
					if unlikely(symid < 0)
						goto err; /* Call as an InstanceMethod */
					if (asm_gcall_extern((uint16_t)symid, id_InstanceMethod, 2))
						goto err; /* args..., class_sym.func */
					              /* Fallthrough to invoke the InstanceMethod normally. */
				}
				if (asm_grrot(argc + 1))
					goto err;           /* func, args... */
				return asm_gcall(argc); /* result */
			}

			/* The attribute must be accessed as virtual. */
			if unlikely(asm_check_thiscall(function, warn_ast))
				goto err;
			SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
			if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
				symid2 = asm_newconst((DeeObject *)attr->ca_name);
				if unlikely(symid2 < 0)
					goto err;
				if (this_sym->s_type == SYMBOL_TYPE_THIS &&
				    !SYMBOL_MUST_REFERENCE_THIS(this_sym))
					return asm_gcallattr_this_const((uint16_t)symid2, argc);
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err; /* args..., this */
				if (asm_grrot(argc))
					goto err;                                       /* this, args... */
				return asm_gcallattr_const((uint16_t)symid2, argc); /* result */
			}
			/* Regular, old member variable. */
			if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
				if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
					symid = asm_rsymid(class_sym);
					if unlikely(symid < 0)
						goto err;
					if ((attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) &&
					    this_sym->s_type == SYMBOL_TYPE_THIS &&
					    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
						if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
							/* Invoke the getter callback. */
							if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_GET, 0))
								goto err;
							goto got_attribute_value;
						}
						return asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr, argc);
					}
					if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr))
						goto err;
				} else {
					if (asm_gpush_symbol(class_sym, warn_ast))
						goto err;
					if (asm_ggetcmember(attr->ca_addr))
						goto err;
				}
			} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
			           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err;
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err;
				if (asm_ggetmember(attr->ca_addr))
					goto err;
			} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if unlikely(symid < 0)
					goto err;
				if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr))
					goto err;
			} else {
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err;
				if (asm_ggetmember_this(attr->ca_addr))
					goto err;
			}
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				/* Call the getter of the attribute. */
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
					if (asm_gcall(0))
						goto err; /* Directly invoke. */
				} else {
					/* Invoke as a this-call. */
					if (asm_gpush_symbol(this_sym, warn_ast))
						goto err;
					if (asm_gcall(1))
						goto err;
				}
			} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				/* Access to an instance member function (must produce a bound method). */
				/* args..., func */
				if unlikely(argc != (uint8_t)-1) {
					if (asm_grrot(argc))
						goto err; /* func, args... */
					if (asm_gpush_symbol(this_sym, warn_ast))
						goto err; /* func, args..., this */
					if (asm_grrot(argc))
						goto err; /* func, this, args... */
					return asm_gcall(argc + 1);
				}
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err; /* args..., func, this */
				symid = asm_newmodule(DeeModule_GetDeemon());
				if unlikely(symid < 0)
					goto err; /* Call as an InstanceMethod */
				if (asm_gcall_extern((uint16_t)symid, id_InstanceMethod, 2))
					goto err; /* args..., this.func */
				              /* Fallthrough to invoke the InstanceMethod normally. */
			}
			/* args..., func */
got_attribute_value:
			if (asm_grrot(argc))
				goto err; /* func, args... */
			return asm_gcall(argc);
		}	break;

		default: break;
		}
	}
	/* Fallback: Generate an extended call. */
	if unlikely(asm_gpush_symbol(function, warn_ast))
		goto err;
	if (asm_grrot(argc + 1))
		goto err;
	if (asm_gcall(argc))
		goto err;
done:
	return 0;
err:
	return -1;
}

PRIVATE ATTR_COLD WUNUSED int
(DCALL asm_warn_ambiguous_symbol)(struct symbol *__restrict sym) {
	ASSERT(sym->s_type == SYMBOL_TYPE_AMBIG);
	return ASM_WARN(W_ASM_AMBIGUOUS_SYMBOL, sym);
}


INTERN WUNUSED NONNULL((1, 2)) int
(DCALL asm_gpush_symbol)(struct symbol *__restrict sym,
                         struct ast *__restrict warn_ast) {
	int32_t symid;
check_sym_class:
	if (SYMBOL_MUST_REFERENCE(sym)) {
		if (current_assembler.a_flag & ASM_FARGREFS) {
			symid = asm_asymid_r(sym);
			if unlikely(symid < 0)
				goto err;
			return asm_gpush_arg((uint16_t)symid);
		}
		symid = asm_rsymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_gpush_ref((uint16_t)symid);
	}
	switch (sym->s_type) {

	case SYMBOL_TYPE_NONE:
		return asm_gpush_none();

	case SYMBOL_TYPE_ALIAS:
		sym = sym->s_alias;
		goto check_sym_class;

	case SYMBOL_TYPE_EXTERN:
		symid = asm_esymid(sym);
		if unlikely(symid < 0)
			goto err;
		if (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FPROPERTY) {
			/* Generate an external call to the getter. */
			return asm_gcall_extern((uint16_t)symid,
			                        SYMBOL_EXTERN_SYMBOL(sym)->ss_index + MODULE_PROPERTY_GET,
			                        0);
		}
		ASSERT(SYMBOL_EXTERN_SYMBOL(sym));
		return asm_gpush_extern((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(sym)->ss_index);

	case SYMBOL_TYPE_GLOBAL:
		symid = asm_gsymid_for_read(sym, warn_ast);
		if unlikely(symid < 0)
			goto err;
		return asm_gpush_global((uint16_t)symid);

	case SYMBOL_TYPE_LOCAL:
		symid = asm_lsymid_for_read(sym, warn_ast);
		if unlikely(symid < 0)
			goto err;
		return asm_gpush_local((uint16_t)symid);

	case SYMBOL_TYPE_STATIC:
		symid = asm_ssymid_for_read(sym, warn_ast);
		if unlikely(symid < 0)
			goto err;
		return asm_gpush_static((uint16_t)symid);

	case SYMBOL_TYPE_STACK: {
		uint16_t offset, absolute_stack_addr;
		if unlikely(!(sym->s_flag & SYMBOL_FALLOC)) {
			if (ASM_WARN(W_ASM_STACK_VARIABLE_NOT_INITIALIZED, sym))
				goto err;
			return asm_gpush_none();
		}
		absolute_stack_addr = SYMBOL_STACK_OFFSET(sym);
		if (current_assembler.a_stackcur <= absolute_stack_addr) {
			/* This can happen in code like this:
				 * __stack local foo;
				 * {
				 *     __stack local bar = "ValueOfBar";
				 *     foo = "ValueOfFoo";
				 * }
				 * print foo; // Error here
				 */
			if (ASM_WARN(W_ASM_STACK_VARIABLE_WAS_DEALLOCATED, sym))
				goto err;
			return asm_gpush_none();
		}
		offset = (current_assembler.a_stackcur - 1) - absolute_stack_addr;
		return offset == 0 ? asm_gdup() : asm_gdup_n(offset - 1);
	}	break;

	case SYMBOL_TYPE_ARG:
		return asm_gpush_varg(sym->s_symid);

	case SYMBOL_TYPE_CATTR: {
		struct symbol *class_sym, *this_sym;
		struct class_attribute *attr;
		int32_t symid2;
		class_sym = sym->s_attr.a_class;
		this_sym  = sym->s_attr.a_this;
		attr      = sym->s_attr.a_attr;
		SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
		if (!this_sym) {
			if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if unlikely(symid < 0)
					goto err;
				if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr)) /* func */
					goto err;
			} else {
				if (asm_gpush_symbol(class_sym, warn_ast)) /* class_sym */
					goto err;
				if (asm_ggetcmember(attr->ca_addr))        /* func */
					goto err;
			}
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				/* Must invoke the getter callback. */
				if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
					if (asm_gpush_symbol(class_sym, warn_ast))
						goto err; /* getter, class */
					if (asm_gcall(1))
						goto err; /* func */
				} else {
					if (asm_gcall(0))
						goto err; /* func */
				}
			} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err; /* func, class_sym */
				symid = asm_newmodule(DeeModule_GetDeemon());
				if unlikely(symid < 0)
					goto err; /* Call as an InstanceMethod */
				if (asm_gcall_extern((uint16_t)symid, id_InstanceMethod, 2))
					goto err; /* class_sym.func */
			}
			return 0;
		}
		/* The attribute must be accessed as virtual. */
		if unlikely(asm_check_thiscall(sym, warn_ast))
			goto err;
		SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
		if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
			symid2 = asm_newconst((DeeObject *)attr->ca_name);
			if unlikely(symid2 < 0)
				goto err;
			if (this_sym->s_type == SYMBOL_TYPE_THIS &&
			    !SYMBOL_MUST_REFERENCE_THIS(this_sym))
				return asm_ggetattr_this_const((uint16_t)symid2);
			if (asm_gpush_symbol(this_sym, warn_ast))
				goto err;
			return asm_ggetattr_const((uint16_t)symid2);
		}
		/* Regular, old member variable. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
			if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if unlikely(symid < 0)
					goto err;
				if ((attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD)) ==
				    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD) &&
				    this_sym->s_type == SYMBOL_TYPE_THIS &&
				    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
					/* Invoke the getter callback. */
					return asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_GET, 0);
				}
				if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr))
					goto err;
			} else {
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err;
				if (asm_ggetcmember(attr->ca_addr))
					goto err;
			}
		} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
		           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
			if (asm_gpush_symbol(this_sym, warn_ast))
				goto err;
			if (asm_gpush_symbol(class_sym, warn_ast))
				goto err;
			if (asm_ggetmember(attr->ca_addr))
				goto err;
		} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
			symid = asm_rsymid(class_sym);
			if unlikely(symid < 0)
				goto err;
			if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr))
				goto err;
		} else {
			if (asm_gpush_symbol(class_sym, warn_ast))
				goto err;
			if (asm_ggetmember_this(attr->ca_addr))
				goto err;
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			/* Call the getter of the attribute. */
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD))
				return asm_gcall(0); /* Directly invoke. */
			/* Invoke as a this-call. */
			if (asm_gpush_symbol(this_sym, warn_ast))
				goto err;
			return asm_gcall(1);
		}
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
			/* Access to an instance member function (must produce a bound method). */
			symid = asm_newmodule(DeeModule_GetDeemon());
			if unlikely(symid < 0)
				goto err;
			if (asm_gpush_symbol(this_sym, warn_ast))
				goto err;
			return asm_gcall_extern((uint16_t)symid, id_InstanceMethod, 2);
		}
		return 0;
	}	break;

	case SYMBOL_TYPE_MODULE:
		symid = asm_msymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_gpush_module((uint16_t)symid);

	case SYMBOL_TYPE_GETSET:
		if (!sym->s_getset.gs_get) {
			if (ASM_WARN(W_ASM_PROPERTY_VARIABLE_NOT_READABLE, sym))
				goto err;
			return asm_gpush_none();
		}
		/* Generate a zero-argument call to the getter symbol. */
		return asm_gcall_symbol_n(sym->s_getset.gs_get, 0, warn_ast);

		/* Misc. symbol classes. */
	case SYMBOL_TYPE_EXCEPT:
		return asm_gpush_except();

	case SYMBOL_TYPE_MYMOD:
		return asm_gpush_this_module();

	case SYMBOL_TYPE_MYFUNC:
		if (current_basescope->bs_flags & CODE_FTHISCALL) {
			/* Must bind the function. */
			if (asm_gpush_this_function())
				goto err;
			if (asm_gpush_this())
				goto err;
			symid = asm_newmodule(DeeModule_GetDeemon());
			if unlikely(symid < 0)
				goto err;
			return asm_gcall_extern((uint16_t)symid, id_InstanceMethod, 2);
		}
		return asm_gpush_this_function();

	case SYMBOL_TYPE_THIS:
		return asm_gpush_this();

	case SYMBOL_TYPE_AMBIG:
		if (asm_warn_ambiguous_symbol(sym))
			goto err;
		return asm_gpush_none();

	case SYMBOL_TYPE_FWD:
		return DeeError_Throwf(&DeeError_SymbolError,
		                       "Unresolved forward symbol %$q",
		                       sym->s_name->k_size,
		                       sym->s_name->k_name);

	case SYMBOL_TYPE_CONST:
		return asm_gpush_constexpr(sym->s_const);

	default:
		ASSERTF(0, "Unsupporetd variable type");
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) bool DCALL
asm_can_prefix_symbol(struct symbol *__restrict sym) {
check_sym_class:
	if (SYMBOL_MUST_REFERENCE(sym))
		return false;
	switch (sym->s_type) {

	case SYMBOL_TYPE_ALIAS:
		sym = sym->s_alias;
		goto check_sym_class;

	case SYMBOL_TYPE_GLOBAL:
		if unlikely((sym->s_flag & SYMBOL_FFINAL) && (sym->s_nwrite > 1))
			return false; /* Must be assigned as `this_module.<ATTRIBUTE> = ...' */
		ATTR_FALLTHROUGH
	case SYMBOL_TYPE_LOCAL:
	case SYMBOL_TYPE_STATIC:
		return true;

	case SYMBOL_TYPE_EXTERN:
		if (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & (MODSYM_FPROPERTY | MODSYM_FREADONLY))
			break; /* Cannot write-prefix properties, or read-only symbols. */
		return true;

	case SYMBOL_TYPE_STACK:
		/* Only allocated stack symbols can be used in prefix expressions. */
		return (sym->s_flag & SYMBOL_FALLOC) != 0;

	default: break;
	}
	return false;
}

INTERN WUNUSED NONNULL((1)) bool DCALL
asm_can_prefix_symbol_for_read(struct symbol *__restrict sym) {
check_sym_class:
	if (SYMBOL_MUST_REFERENCE(sym))
		return false;
	switch (sym->s_type) {

	case SYMBOL_TYPE_ALIAS:
		sym = sym->s_alias;
		goto check_sym_class;

	case SYMBOL_TYPE_GLOBAL:
	case SYMBOL_TYPE_LOCAL:
	case SYMBOL_TYPE_STATIC:
		return true;

	case SYMBOL_TYPE_EXTERN:
		if (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FPROPERTY)
			break; /* Cannot prefix properties. */
		return true;

	case SYMBOL_TYPE_STACK:
		/* Only allocated stack symbols can be used in prefix expressions. */
		return (sym->s_flag & SYMBOL_FALLOC) != 0;

	default: break;
	}
	return false;
}


INTERN WUNUSED NONNULL((1, 2)) int
(DCALL asm_gprefix_symbol)(struct symbol *__restrict sym,
                           struct ast *__restrict warn_ast) {
	int32_t symid;
	(void)warn_ast;
check_sym_class:
	ASSERT(!SYMBOL_MUST_REFERENCE(sym));
	switch (sym->s_type) {

	case SYMBOL_TYPE_ALIAS:
		sym = sym->s_alias;
		goto check_sym_class;

	case SYMBOL_TYPE_EXTERN:
		ASSERTF(!(SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FPROPERTY),
		        "Cannot prefix property symbols");
		symid = asm_esymid(sym);
		if unlikely(symid < 0)
			goto err;
		ASSERT(SYMBOL_EXTERN_SYMBOL(sym));
		return asm_pextern((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(sym)->ss_index);

	case SYMBOL_TYPE_GLOBAL:
		ASSERTF(!(sym->s_flag & SYMBOL_FFINAL) || (sym->s_nwrite <= 1),
		        "This should have caused `asm_can_prefix_symbol()' to return false");
		symid = asm_gsymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_pglobal((uint16_t)symid);

	case SYMBOL_TYPE_LOCAL:
		symid = asm_lsymid(sym);
		if unlikely(symid < 0)
			goto err;
		if unlikely((sym->s_flag & SYMBOL_FFINAL) && (sym->s_nwrite > 1)) {
			if (ASM_WARN(W_ASM_MULTIPLE_WRITES_TO_FINAL, sym))
				goto err;
			if (asm_gcheck_final_local_bound((uint16_t)symid))
				goto err;
		}
		return asm_plocal((uint16_t)symid);

	case SYMBOL_TYPE_STATIC:
		if unlikely((sym->s_flag & SYMBOL_FFINAL) && (sym->s_nwrite > 1)) {
			if (ASM_WARN(W_ASM_UNSUPPORTED_FINAL_SYMBOL_TYPE, sym))
				goto err;
		}
		symid = asm_ssymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_pstatic((uint16_t)symid);

	case SYMBOL_TYPE_STACK:
		if unlikely((sym->s_flag & SYMBOL_FFINAL) && (sym->s_nwrite > 1)) {
			if (ASM_WARN(W_ASM_UNSUPPORTED_FINAL_SYMBOL_TYPE, sym))
				goto err;
		}
		ASSERT(sym->s_flag & SYMBOL_FALLOC);
		if (current_assembler.a_stackcur <= SYMBOL_STACK_OFFSET(sym)) {
			/* This can happen in code like this:
			 * __stack local foo;
			 * {
			 *     __stack local bar = "ValueOfBar";
			 *     foo = "ValueOfFoo";
			 * }
			 * print foo; // Error here
			 */
			ASM_ERR(W_ASM_STACK_VARIABLE_WAS_DEALLOCATED, sym);
			goto err;
		}
		return asm_pstack(SYMBOL_STACK_OFFSET(sym));

	default:
		ASM_ERR(W_ASM_CANNOT_PREFIX_SYMBOL_CLASS, sym);
		goto err;
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL asm_gprefix_symbol_for_read)(struct symbol *__restrict sym,
                                    struct ast *__restrict warn_ast) {
	int32_t symid;
	(void)warn_ast;
check_sym_class:
	ASSERT(!SYMBOL_MUST_REFERENCE(sym));
	switch (sym->s_type) {

	case SYMBOL_TYPE_ALIAS:
		sym = sym->s_alias;
		goto check_sym_class;

	case SYMBOL_TYPE_EXTERN:
		ASSERTF(!(SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FPROPERTY),
		        "Cannot prefix property symbols");
		symid = asm_esymid(sym);
		if unlikely(symid < 0)
			goto err;
		ASSERT(SYMBOL_EXTERN_SYMBOL(sym));
		return asm_pextern((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(sym)->ss_index);

	case SYMBOL_TYPE_GLOBAL:
		symid = asm_gsymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_pglobal((uint16_t)symid);

	case SYMBOL_TYPE_LOCAL:
		symid = asm_lsymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_plocal((uint16_t)symid);

	case SYMBOL_TYPE_STATIC:
		symid = asm_ssymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_pstatic((uint16_t)symid);

	case SYMBOL_TYPE_STACK:
		ASSERT(sym->s_flag & SYMBOL_FALLOC);
		if (current_assembler.a_stackcur <= SYMBOL_STACK_OFFSET(sym)) {
			/* This can happen in code like this:
			 * __stack local foo;
			 * {
			 *     __stack local bar = "ValueOfBar";
			 *     foo = "ValueOfFoo";
			 * }
			 * print foo; // Error here
			 */
			ASM_ERR(W_ASM_STACK_VARIABLE_WAS_DEALLOCATED, sym);
			goto err;
		}
		return asm_pstack(SYMBOL_STACK_OFFSET(sym));

	default:
		ASM_ERR(W_ASM_CANNOT_PREFIX_SYMBOL_CLASS, sym);
		goto err;
	}
	__builtin_unreachable();
err:
	return -1;
}

/* Define to generate inline code for testing the binding of a class property.
 * This should be kept disabled, because the generated code is quite excessively
 * large, as it includes an exception handler for capturing UnboundAttribute errors.
 * Essentially this switch does the following:
 * >> class MyClass {
 * >>     myprop = {
 * >>         get() {
 * >>             return 42;
 * >>         }
 * >>     }
 * >>
 * >>     test() {
 * >>         print myprop is bound;
 * >>         // Compiled as the equivalent of:
 * >>#ifdef CONFIG_INLINE_GETSET_BINDING_CHECKER
 * >>         print try ({ myprop; true; }) catch ((Error from deemon).AttributeError.UnboundAttribute) false;
 * >>#else
 * >>#endif
 * >>     }
 * >> }
 */
#undef CONFIG_INLINE_GETSET_BINDING_CHECKER
//#define CONFIG_INLINE_GETSET_BINDING_CHECKER


INTERN WUNUSED NONNULL((1, 2)) int
(DCALL asm_gpush_bnd_symbol)(struct symbol *__restrict sym,
                             struct ast *__restrict warn_ast) {
	int32_t symid;
check_sym_class:
	if (SYMBOL_MUST_REFERENCE(sym))
		goto fallback;
	switch (sym->s_type) {

	case SYMBOL_TYPE_ALIAS:
		sym = sym->s_alias;
		goto check_sym_class;
	case SYMBOL_TYPE_ARG: {
		DeeObject *defl;
		if (sym->s_symid < current_basescope->bs_argc_min ||
		    sym->s_symid >= current_basescope->bs_argc_max ||
		    DeeBaseScope_IsVarargs(current_basescope, sym) ||
		    DeeBaseScope_IsVarkwds(current_basescope, sym))
			goto fallback;
		defl = current_basescope->bs_default[sym->s_symid - current_basescope->bs_argc_min];
		if (defl)
			goto fallback; /* Non-optional arguments are always bound. */
		return asm_gpush_bnd_arg(sym->s_symid);
	}	break;

	case SYMBOL_TYPE_EXTERN:
		if (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FPROPERTY)
			goto fallback;
		symid = asm_esymid(sym);
		if unlikely(symid < 0)
			goto err;
		ASSERT(SYMBOL_EXTERN_SYMBOL(sym));
		return asm_gpush_bnd_extern((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(sym)->ss_index);

	case SYMBOL_TYPE_GLOBAL:
		if (!sym->s_nwrite)
			return asm_gpush_false();
		symid = asm_gsymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_gpush_bnd_global((uint16_t)symid);

	case SYMBOL_TYPE_LOCAL:
		if (!sym->s_nwrite)
			return asm_gpush_false();
		symid = asm_lsymid(sym);
		if unlikely(symid < 0)
			goto err;
		return asm_gpush_bnd_local((uint16_t)symid);

	case SYMBOL_TYPE_CATTR: {
		struct symbol *class_sym, *this_sym;
		struct class_attribute *attr;
		class_sym = sym->s_attr.a_class;
		this_sym  = sym->s_attr.a_this;
		attr      = sym->s_attr.a_attr;
		SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
		if (!this_sym) {
			/* Do a regular attribute lookup on the class itself. */
test_class_attribute:
			if (asm_gpush_symbol(class_sym, warn_ast))
				goto err;
			if (asm_gpush_constexpr((DeeObject *)attr->ca_name))
				goto err;
			return asm_gboundattr();
		}
		/* The attribute must be accessed as virtual. */
		if unlikely(asm_check_thiscall(sym, warn_ast))
			goto err;
		SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
		if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))
#ifndef CONFIG_INLINE_GETSET_BINDING_CHECKER
		    || (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
#endif /* !CONFIG_INLINE_GETSET_BINDING_CHECKER */
		    ) {
			if (asm_gpush_symbol(this_sym, warn_ast))
				goto err;
			if (asm_gpush_constexpr((DeeObject *)attr->ca_name))
				goto err;
			return asm_gboundattr();
		}
		/* Regular, old member variable. */
#ifdef CONFIG_INLINE_GETSET_BINDING_CHECKER
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
			/* Special case: must invoke the attribute as a getter, and make
			 * sure that doing so doesn't produce any UnboundAttribute errors. */
			struct asm_sym *guard_begin, *guard_end;
			struct asm_exc *exception_handler;
			guard_begin = asm_newsym();
			if unlikely(!guard_begin)
				goto err;
			asm_defsym(guard_begin);
			if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
				if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
					symid = asm_rsymid(class_sym);
					if unlikely(symid < 0)
						goto err;
					if ((attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD)) ==
					    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD) &&
					    this_sym->s_type == SYMBOL_TYPE_THIS &&
					    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
						/* Invoke the getter callback. */
						if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_GET, 0))
							goto err;
						goto got_attribute_value;
					}
					if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_GET))
						goto err;
				} else {
					if (asm_gpush_symbol(class_sym, warn_ast))
						goto err;
					if (asm_ggetcmember(attr->ca_addr + CLASS_GETSET_GET))
						goto err;
				}
			} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
			           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err;
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err;
				if (asm_ggetmember(attr->ca_addr + CLASS_GETSET_GET))
					goto err;
			} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if unlikely(symid < 0)
					goto err;
				if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_GET))
					goto err;
			} else {
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err;
				if (asm_ggetmember_this(attr->ca_addr + CLASS_GETSET_GET))
					goto err;
			}
			/* At this point, we've acquired to getter callback. - Now to invoke it. */
			if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
				if (asm_gcall(0))
					goto err;
			} else {
				/* Invoke as a this-call. */
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err;
				if (asm_gcall(1))
					goto err;
			}
got_attribute_value:
			guard_end = asm_newsym();
			if unlikely(!guard_end)
				goto err;
			asm_defsym(guard_end);
			if (asm_gpop())
				goto err; /* The return value of the getter. */
			if (asm_gpush_true())
				goto err; /* The symbol is bound! */
			asm_decsp();
			/* define the exception handler cleanup code. */
			exception_handler = asm_newexc();
			if unlikely(!exception_handler)
				goto err;
			exception_handler->ex_start = guard_begin;
			exception_handler->ex_end   = guard_end;
			++guard_begin->as_used;
			++guard_end->as_used;
			guard_begin = asm_newsym();
			if unlikely(!guard_begin)
				goto err;
			guard_end = asm_newsym();
			if unlikely(!guard_end)
				goto err;
			/* Setup an exception handler for dealing with unbound-attribute errors. */
			exception_handler->ex_addr  = guard_begin;
			exception_handler->ex_flags = EXCEPTION_HANDLER_FHANDLED;
			exception_handler->ex_mask  = &DeeError_UnboundAttribute;
			Dee_Incref(&DeeError_UnboundAttribute);
			++guard_begin->as_used;
			symid = asm_getcur();
			if (symid == SECTION_COLD) {
				if (asm_gjmp(ASM_JMP, guard_end))
					goto err;
				asm_defsym(guard_begin);
				if (asm_gpush_false())
					goto err;
				asm_defsym(guard_end);
			} else {
				asm_setcur(SECTION_COLD);
				asm_defsym(guard_begin);
				if (asm_gpush_false())
					goto err;
				if (asm_gjmp(ASM_JMP, guard_end))
					goto err;
				asm_setcur((uint16_t)symid);
				asm_defsym(guard_end);
			}
			return 0;
		}
#endif /* CONFIG_INLINE_GETSET_BINDING_CHECKER */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
			goto test_class_attribute;
		if (this_sym->s_type != SYMBOL_TYPE_THIS ||
		    SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
			if (asm_gpush_symbol(this_sym, warn_ast))
				goto err;
			if (asm_gpush_symbol(class_sym, warn_ast))
				goto err;
			if (asm_gboundmember(attr->ca_addr))
				goto err;
		} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
			symid = asm_rsymid(class_sym);
			if unlikely(symid < 0)
				goto err;
			if (asm_gboundmember_this_r((uint16_t)symid, attr->ca_addr))
				goto err;
		} else {
			if (asm_gpush_symbol(class_sym, warn_ast))
				goto err;
			if (asm_gboundmember_this(attr->ca_addr))
				goto err;
		}
		return 0;
	}	break;

	case SYMBOL_TYPE_AMBIG:
		if (asm_warn_ambiguous_symbol(sym))
			goto err;
		goto fallback;

	default:
fallback:
		return asm_gpush_true();
	}
	__builtin_unreachable();
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL asm_gdel_symbol)(struct symbol *__restrict sym,
                        struct ast *__restrict warn_ast) {
	int32_t symid;
check_sym_class:
	if (!SYMBOL_MUST_REFERENCE(sym)) {
		switch (sym->s_type) {

		case SYMBOL_TYPE_ALIAS:
			sym = sym->s_alias;
			goto check_sym_class;

		case SYMBOL_TYPE_GLOBAL:
			if unlikely(sym->s_flag & SYMBOL_FFINAL) {
				if (ASM_WARN(W_ASM_UNBIND_FINAL_SYMBOL, sym))
					goto err;
			}
			if (!(sym->s_flag & SYMBOL_FALLOC) && !sym->s_nwrite)
				return 0;
			symid = asm_gsymid(sym);
			if unlikely(symid < 0)
				goto err;
			return asm_gdel_global((uint16_t)symid);

		case SYMBOL_TYPE_LOCAL:
			if unlikely(sym->s_flag & SYMBOL_FFINAL) {
				if (ASM_WARN(W_ASM_UNBIND_FINAL_SYMBOL, sym))
					goto err;
			}
			if (!(sym->s_flag & SYMBOL_FALLOC) && !sym->s_nwrite)
				return 0;
			symid = asm_lsymid(sym);
			if unlikely(symid < 0)
				goto err;
			return asm_gdel_local((uint16_t)symid);

		case SYMBOL_TYPE_EXTERN: {
			struct module_symbol *modsym;
			int32_t mid;
			modsym = SYMBOL_EXTERN_SYMBOL(sym);
			mid    = asm_esymid(sym);
			if unlikely(mid < 0)
				goto err;
			if (modsym->ss_flags & MODSYM_FPROPERTY) {
				/* Call an external property:
				 * >> call    extern <mid>:<gid + MODULE_PROPERTY_DEL>, #0
				 * >> pop     top */
				if (asm_gcall_extern((uint16_t)mid,
				                     modsym->ss_index + MODULE_PROPERTY_DEL,
				                     0))
					goto err;
				return asm_gpop();
			} else {
				/* Delete an external global variable:
				 * >> push    module <mid>
				 * >> delattr pop, const DeeModule_GlobalName(<module>, <gid>) */
				int32_t cid;
				DREF DeeStringObject *name_obj;
				name_obj = module_symbol_getnameobj(modsym);
				if unlikely(!name_obj)
					goto err;
				cid = asm_newconst((DeeObject *)name_obj);
				Dee_Decref(name_obj);
				if unlikely(cid < 0)
					goto err;
				if (asm_gpush_module((uint16_t)mid))
					goto err;
				return asm_gdelattr_const((uint16_t)cid);
			}
		}	break;

		case SYMBOL_TYPE_STACK:
			if unlikely(sym->s_flag & SYMBOL_FFINAL) {
				if (ASM_WARN(W_ASM_UNBIND_FINAL_SYMBOL, sym))
					goto err;
			}
			/* If `bound()' is used on the symbol, warn about the fact that
			 * doing this will not actually unbind the symbol, but only delete
			 * the value that was being stored. */
			if (sym->s_nbound != 0 &&
			    !(sym->s_flag & SYMBOL_FSTACK_NOUNBIND_OK) &&
			    WARN(W_ASM_DELETED_STACK_VARIABLE_ISNT_UNBOUND, sym))
				goto err;
			if (!(sym->s_flag & SYMBOL_FALLOC)) {
				/* If the stack variable hasn't been allocated, but is being written
				 * to at some point, then we can't actually unbind it by overwriting
				 * it, meaning that we can't generate the code that the user would
				 * expect from us. */
				if (sym->s_nwrite != 0 &&
				    WARN(W_ASM_CANNOT_UNBIND_UNDESIGNATED_STACK_VARIABLE, sym))
					goto err;
				return 0;
			}
			/* Overwrite the stack-variable with `none':
			 * >> mov stack#..., none */
			if (asm_pstack(SYMBOL_STACK_OFFSET(sym)))
				goto err;
			return asm_gpush_none_p();

		case SYMBOL_TYPE_CATTR: {
			struct symbol *class_sym, *this_sym;
			struct class_attribute *attr;
			class_sym = sym->s_attr.a_class;
			this_sym  = sym->s_attr.a_this;
			attr      = sym->s_attr.a_attr;
			SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
			if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
				break; /* TODO: Dedicated warning. */
			if (!this_sym) {
del_class_attribute:
				if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					/* Must invoke the getter callback. */
					if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
						symid = asm_rsymid(class_sym);
						if unlikely(symid < 0)
							goto err;
						if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_DEL)) /* func */
							goto err;
					} else {
						if (asm_gpush_symbol(class_sym, warn_ast))             /* class_sym */
							goto err;
						if (asm_ggetcmember(attr->ca_addr + CLASS_GETSET_DEL)) /* func */
							goto err;
					}
					if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
						if (asm_gpush_symbol(class_sym, warn_ast))
							goto err; /* delete, class */
						if (asm_gcall(1))
							goto err; /* discard */
					} else {
						if (asm_gcall(0))
							goto err; /* discard */
					}
					return asm_gpop();
				}
				/* Because there is no ASM_DELCMEMBER instruction, we simply
				 * encode this as a delattr, which, while not static, still
				 * does exactly what we want it to do! */
				symid = asm_newconst((DeeObject *)attr->ca_name);
				if unlikely(symid < 0)
					goto err;
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err;
				return asm_gdelattr_const((uint16_t)symid);
			}
			/* The attribute must be accessed as virtual. */
			if unlikely(asm_check_thiscall(sym, warn_ast))
				goto err;
			SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
			if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
				int32_t symid2;
				symid2 = asm_newconst((DeeObject *)attr->ca_name);
				if unlikely(symid2 < 0)
					goto err;
				if (this_sym->s_type == SYMBOL_TYPE_THIS &&
				    !SYMBOL_MUST_REFERENCE_THIS(this_sym))
					return asm_gdelattr_this_const((uint16_t)symid2);
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err;
				return asm_gdelattr_const((uint16_t)symid2);
			}
			/* Regular, old member variable. */
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				/* Call the delete function of the attribute. */
				if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
					if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
						symid = asm_rsymid(class_sym);
						if unlikely(symid < 0)
							goto err;
						if ((attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD)) ==
						    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD) &&
						    this_sym->s_type == SYMBOL_TYPE_THIS &&
						    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
							/* Invoke the delete callback. */
							if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_DEL, 0))
								goto err;
							goto pop_unused_result;
						}
						if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_DEL))
							goto err;
					} else {
						if (asm_gpush_symbol(class_sym, warn_ast))
							goto err;
						if (asm_ggetcmember(attr->ca_addr + CLASS_GETSET_DEL))
							goto err;
					}
				} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
				           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
					if (asm_gpush_symbol(this_sym, warn_ast))
						goto err;
					if (asm_gpush_symbol(class_sym, warn_ast))
						goto err;
					if (asm_ggetmember(attr->ca_addr + CLASS_GETSET_DEL))
						goto err;
				} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
					symid = asm_rsymid(class_sym);
					if unlikely(symid < 0)
						goto err;
					if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_DEL))
						goto err;
				} else {
					if (asm_gpush_symbol(class_sym, warn_ast))
						goto err;
					if (asm_ggetmember_this(attr->ca_addr + CLASS_GETSET_DEL))
						goto err;
				}
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
					if (asm_gcall(0))
						goto err; /* Directly invoke. */
				} else {
					/* Invoke as a this-call. */
					if (asm_gpush_symbol(this_sym, warn_ast))
						goto err;
					if (asm_gcall(1))
						goto err;
				}
pop_unused_result:
				return asm_gpop();
			}
			if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
				goto del_class_attribute;
			if (this_sym->s_type != SYMBOL_TYPE_THIS ||
			    SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err;
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err;
				if (asm_gdelmember(attr->ca_addr))
					goto err;
			} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if unlikely(symid < 0)
					goto err;
				if (asm_gdelmember_this_r((uint16_t)symid, attr->ca_addr))
					goto err;
			} else {
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err;
				if (asm_gdelmember_this(attr->ca_addr))
					goto err;
			}
			return 0;
		}	break;

		case SYMBOL_TYPE_GETSET:
			if (!sym->s_getset.gs_del)
				return 0; /* TODO: Warning */
			/* Generate a zero-argument call to the delete symbol. */
			if (asm_gcall_symbol_n(sym->s_getset.gs_del, 0, warn_ast))
				goto err;
			return asm_gpop();

		case SYMBOL_TYPE_AMBIG:
			if (asm_warn_ambiguous_symbol(sym))
				goto err;
			return 0;

		default: break;
		}
	}
	return ASM_WARN(W_ASM_CANNOT_UNBIND_SYMBOL, sym);
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL asm_gpop_symbol)(struct symbol *__restrict sym,
                        struct ast *__restrict warn_ast) {
	int32_t symid;
check_sym_class:
	if (!SYMBOL_MUST_REFERENCE(sym)) {
		switch (sym->s_type) {

		case SYMBOL_TYPE_ALIAS:
			sym = sym->s_alias;
			goto check_sym_class;

		case SYMBOL_TYPE_EXTERN:
			ASSERT(SYMBOL_EXTERN_SYMBOL(sym));
			if (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FREADONLY) {
				/* ERROR: Can't modify read-only external symbol. */
				if (ASM_WARN(W_ASM_EXTERNAL_SYMBOL_IS_READONLY, sym))
					goto err;
				return asm_gpop(); /* Fallback: Simply discard the value. */
			}
			symid = asm_esymid(sym);
			if unlikely(symid < 0)
				goto err;
			if (SYMBOL_EXTERN_SYMBOL(sym)->ss_flags & MODSYM_FPROPERTY) {
				/* Invoke the external setter callback. */
				return asm_gcall_extern((uint16_t)symid,
				                        SYMBOL_EXTERN_SYMBOL(sym)->ss_index + MODULE_PROPERTY_SET,
				                        1);
			}
			return asm_gpop_extern((uint16_t)symid, SYMBOL_EXTERN_SYMBOL(sym)->ss_index);

		case SYMBOL_TYPE_GLOBAL:
			symid = asm_gsymid(sym);
			if unlikely(symid < 0)
				goto err;
			if unlikely((sym->s_flag & SYMBOL_FFINAL) && (sym->s_nwrite > 1)) {
				if (ASM_WARN(W_ASM_MULTIPLE_WRITES_TO_FINAL, sym))
					goto err;
				symid = asm_newconst_string(sym->s_name->k_name,
				                            sym->s_name->k_size);
				/* Must ensure that the write is only written once! */
				if (asm_gpush_this_module())                /* value, this_module */
					goto err;
				if (asm_gswap())                            /* this_module, value */
					goto err;
				return asm_gsetattr_const((uint16_t)symid); /* this_module.<SYMBOL> = value */
			}
			return asm_gpop_global((uint16_t)symid);

		case SYMBOL_TYPE_LOCAL:
			symid = asm_lsymid(sym);
			if unlikely(symid < 0)
				goto err;
			if unlikely((sym->s_flag & SYMBOL_FFINAL) && (sym->s_nwrite > 1)) {
				if (ASM_WARN(W_ASM_MULTIPLE_WRITES_TO_FINAL, sym))
					goto err;
				if (asm_gcheck_final_local_bound((uint16_t)symid))
					goto err;
			}
			return asm_gpop_local((uint16_t)symid);

		case SYMBOL_TYPE_STATIC:
			symid = asm_ssymid(sym);
			if unlikely(symid < 0)
				goto err;
			if unlikely((sym->s_flag & SYMBOL_FFINAL) && (sym->s_nwrite > 1)) {
				if (ASM_WARN(W_ASM_UNSUPPORTED_FINAL_SYMBOL_TYPE, sym))
					goto err;
			}
			return asm_gpop_static((uint16_t)symid);

		case SYMBOL_TYPE_STACK:
			ASSERT(current_assembler.a_stackcur);
			if unlikely((sym->s_flag & SYMBOL_FFINAL) && (sym->s_nwrite > 1)) {
				if (ASM_WARN(W_ASM_UNSUPPORTED_FINAL_SYMBOL_TYPE, sym))
					goto err;
			}
			if unlikely(!(sym->s_flag & SYMBOL_FALLOC)) {
				/* This is where the magic of lazy stack initialization happens! */
				if (current_assembler.a_flag & ASM_FSTACKDISP) {
					if (current_assembler.a_scope != sym->s_scope) {
						DeeScopeObject *my_scope;
						/* Warn about undefined behavior when the variable isn't from the current scope:
						 * >> __stack local foo;
						 * >> {
						 * >>     __stack local bar = "ValueOfBar";
						 * >>     foo = "ValueOfFoo"; // Warn here.
						 * >> }
						 * >> {
						 * >>     __stack local x = "ValueOfX";
						 * >>     __stack local y = "ValueOfY";
						 * >>     print foo; // What is printed here is undefined and at
						 * >>                // the time of this being written is `ValueOfY'
						 * >>                // Future (unused variable?) optimizations may change this...
						 * >>                // In either case: what's printed probably isn't what you want to see here.
						 * >> }
						 * NOTE: This problem does not arise when stack displacement is disabled...
						 */
						my_scope = current_assembler.a_scope;
						do {
							my_scope = my_scope->s_prev;
						} while (my_scope && my_scope != sym->s_scope);
						if (!my_scope) {
							if (ASM_WARN(W_ASM_STACK_VARIABLE_UNREACHABLE_SCOPE, sym))
								goto err;
							/* If the scope of the symbol is reachable from the current scope, then
							 * we can still allocate the stack-symbol (even though we really shouldn't).
							 * However if it isn't, then we mustn't allow the symbol to be allocated,
							 * because there would be no one to clean up this allocation!
							 * >> ({ __stack local foo; }) = 7;
							 */
							return asm_gpop();
						}
						/* The scope can be reached, so the variable is actually owned by an
						 * active portion of the assembler, meaning it will get cleaned up
						 * when its associated scope ends:
						 * >> {
						 * >>     __stack local foo;
						 * >>     {
						 * >>         foo = 7; // Initialized (and allocated) in non-owning scope
						 * >>     }
						 * >> }
						 */
						if (ASM_WARN(W_ASM_STACK_VARIABLE_DIFFERENT_SCOPE, sym))
							goto err;
#ifndef NDEBUG
						my_scope = current_assembler.a_scope;
						do {
							++my_scope->s_old_stack;
							my_scope = my_scope->s_prev;
						} while (my_scope != sym->s_scope);
#endif /* !NDEBUG */
					}
					sym->s_flag |= SYMBOL_FALLOC;
					sym->s_symid = current_assembler.a_stackcur - 1;
					if (asm_putddi_sbind(SYMBOL_STACK_OFFSET(sym), sym->s_name))
						goto err;
					return 0; /* Leave without popping anything! */
				}
				if (ASM_WARN(W_ASM_STACK_VARIABLE_NOT_INITIALIZED, sym))
					goto err;
				return asm_gpop();
			}
			return asm_gpop_stack(SYMBOL_STACK_OFFSET(sym));

		case SYMBOL_TYPE_CATTR: {
			struct symbol *class_sym, *this_sym;
			struct class_attribute *attr;
			class_sym = sym->s_attr.a_class;
			this_sym  = sym->s_attr.a_this;
			attr      = sym->s_attr.a_attr;
			SYMBOL_INPLACE_UNWIND_ALIAS(class_sym);
			if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
				break; /* TODO: Dedicated warning. */
			if (!this_sym) {
set_class_attribute:
				if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					/* Must invoke the getter callback. */
					if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
						symid = asm_rsymid(class_sym);
						if unlikely(symid < 0)
							goto err;
						if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET)) /* value, func */
							goto err;
					} else {
						if (asm_gpush_symbol(class_sym, warn_ast))             /* value, class_sym */
							goto err;
						if (asm_ggetcmember(attr->ca_addr + CLASS_GETSET_SET)) /* value, func */
							goto err;
					}
					if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
						if (asm_gpush_symbol(class_sym, warn_ast)) /* value, setter, class */
							goto err;
						if (asm_glrot(3))                          /* setter, class, value */
							goto err;
						if (asm_gcall(2))                          /* discard */
							goto err;
					} else {
						if (asm_gswap())
							goto err; /* setter, value */
						if (asm_gcall(1))
							goto err; /* discard */
					}
					return asm_gpop();
				}
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err; /* value, class */
				if (asm_gswap())
					goto err; /* class, value */
				if (asm_gdefcmember(attr->ca_addr))
					goto err;      /* class */
				return asm_gpop(); /* - */
			}
			/* The attribute must be accessed as virtual. */
			if unlikely(asm_check_thiscall(sym, warn_ast))
				goto err;
			SYMBOL_INPLACE_UNWIND_ALIAS(this_sym);
			if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FPRIVATE | CLASS_ATTRIBUTE_FFINAL))) {
				symid = asm_newconst((DeeObject *)attr->ca_name);
				if unlikely(symid < 0)
					goto err;
				if (this_sym->s_type == SYMBOL_TYPE_THIS &&
				    !SYMBOL_MUST_REFERENCE_THIS(this_sym))
					return asm_gsetattr_this_const((uint16_t)symid);
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err;
				if (asm_gswap())
					goto err; /* this, value */
				return asm_gsetattr_const((uint16_t)symid);
			}
			/* Regular, old member variable. */
			if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				/* Call the delete function of the attribute. */
				if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
					if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
						symid = asm_rsymid(class_sym);
						if unlikely(symid < 0)
							goto err;
						if ((attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD)) ==
						    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FMETHOD) &&
						    this_sym->s_type == SYMBOL_TYPE_THIS &&
						    !SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
							/* Invoke the delete callback. */
							if (asm_gcallcmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET, 0))
								goto err;
							goto pop_unused_result;
						}
						if (asm_ggetcmember_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET))
							goto err;
					} else {
						if (asm_gpush_symbol(class_sym, warn_ast))
							goto err;
						if (asm_ggetcmember(attr->ca_addr + CLASS_GETSET_SET))
							goto err;
					}
				} else if (this_sym->s_type != SYMBOL_TYPE_THIS ||
				           SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
					if (asm_gpush_symbol(this_sym, warn_ast))
						goto err;
					if (asm_gpush_symbol(class_sym, warn_ast))
						goto err;
					if (asm_ggetmember(attr->ca_addr + CLASS_GETSET_SET))
						goto err;
				} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
					symid = asm_rsymid(class_sym);
					if unlikely(symid < 0)
						goto err;
					if (asm_ggetmember_this_r((uint16_t)symid, attr->ca_addr + CLASS_GETSET_SET))
						goto err;
				} else {
					if (asm_gpush_symbol(class_sym, warn_ast))
						goto err;
					if (asm_ggetmember_this(attr->ca_addr + CLASS_GETSET_SET))
						goto err;
				}
				/* value, callback */
				if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
					if (asm_gswap())
						goto err; /* callback, value */
					if (asm_gcall(1))
						goto err; /* discard */
				} else {
					/* Invoke as a this-call. */
					if (asm_gpush_symbol(this_sym, warn_ast))
						goto err; /* value, callback, this */
					if (asm_glrot(3))
						goto err; /* callback, this, value */
					if (asm_gcall(2))
						goto err; /* discard */
				}
pop_unused_result:
				return asm_gpop();
			}
			if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
				goto set_class_attribute;
			if (this_sym->s_type != SYMBOL_TYPE_THIS ||
			    SYMBOL_MUST_REFERENCE_THIS(this_sym)) {
				if (asm_gpush_symbol(this_sym, warn_ast))
					goto err; /* value, this */
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err; /* value, this, class */
				if (asm_glrot(3))
					goto err; /* this, class, value */
				if (asm_gsetmember(attr->ca_addr))
					goto err; /* - */
			} else if (ASM_SYMBOL_MAY_REFERENCE(class_sym)) {
				symid = asm_rsymid(class_sym);
				if unlikely(symid < 0)
					goto err;
				if (asm_gsetmember_this_r((uint16_t)symid, attr->ca_addr))
					goto err;
			} else {
				if (asm_gpush_symbol(class_sym, warn_ast))
					goto err; /* value, class */
				if (asm_gswap())
					goto err; /* class, value */
				if (asm_gsetmember_this(attr->ca_addr))
					goto err; /* - */
			}
			return 0;
		}	break;

		case SYMBOL_TYPE_GETSET:
			if (!sym->s_getset.gs_set) {
				if (ASM_WARN(W_ASM_PROPERTY_VARIABLE_NOT_WRITABLE, sym))
					goto err;
			} else {
				/* Generate a one-argument call to the setter symbol. */
				if (asm_gcall_symbol_n(sym->s_getset.gs_set, 1, warn_ast))
					goto err;
			}
			/* Pop the return value. */
			return asm_gpop();

		case SYMBOL_TYPE_AMBIG:
			if (asm_warn_ambiguous_symbol(sym))
				goto err;
			return asm_gpop();

		default:
			break;
		}
	}
	/* Warn about the fact that the symbol cannot be written. */
	if (ASM_WARN(W_ASM_CANNOT_WRITE_SYMBOL, sym))
		goto err;
	return asm_gpop(); /* Fallback: Simply discard the value. */
err:
	return -1;
}

/* Push the virtual argument known as `argid' */
INTERN WUNUSED int DCALL
asm_gpush_varg(uint16_t argid) {
	if (argid < current_basescope->bs_argc) {
		struct symbol *sym = current_basescope->bs_argv[argid];
		if (DeeBaseScope_IsVarargs(current_basescope, sym))
			return asm_gpush_varargs();
		if (DeeBaseScope_IsVarkwds(current_basescope, sym))
			return asm_gpush_varkwds();
	}
	return asm_gpush_arg(argid);
}




/* Store the value of the virtual argument `argid' in `dst' */
INTERN WUNUSED NONNULL((1, 3)) int
(DCALL asm_gmov_varg)(struct symbol *__restrict dst, uint16_t argid,
                      struct ast *__restrict warn_ast,
                      bool ignore_unbound) {
	if (ignore_unbound &&
	    argid >= current_basescope->bs_argc_min &&
	    argid < current_basescope->bs_argc_max &&
	    current_basescope->bs_default[argid - current_basescope->bs_argc_min] == NULL) {
		/* Check if the argument is bound, and don't do the mov if it isn't. */
		struct asm_sym *skip_mov;
		if (asm_gpush_bnd_arg(argid))
			goto err;
		skip_mov = asm_newsym();
		if (asm_gjmp(ASM_JF, skip_mov))
			goto err;
		asm_decsp();
		/* Only perform the store if the argument was given. */
		if (asm_gpush_varg(argid))
			goto err;
		if (asm_gpop_symbol(dst, warn_ast))
			goto err;
		asm_defsym(skip_mov);
		return 0;
	}
	/* Fallback: push the argument, then pop it into the symbol. */
	if (asm_gpush_varg(argid))
		goto err;
	return asm_gpop_symbol(dst, warn_ast);
err:
	return -1;
}


/* Generate code to throw RuntimeError when `lid' is bound at runtime. */
INTERN WUNUSED int DCALL
asm_gcheck_final_local_bound(uint16_t lid) {
	/* >>     push   bound local \lid
	 * >>     jt     1f
	 * >> .pushsection .cold
	 * >> 1:  push   $\lid
	 * >>     push   call extern @deemon:@__roloc, #1
	 * >>     pop
	 * >>     jmp    2f
	 * >> .popsection
	 * >> 2: */
	struct asm_sym *within_cold;
	struct asm_sym *after_cold;
	struct asm_sec *sect;
	DREF DeeObject *constlid;
	int32_t temp_id;
	if (asm_gpush_bnd_local(lid))
		goto err;
	within_cold = asm_newsym();
	if unlikely(!within_cold)
		goto err;
	after_cold = asm_newsym();
	if unlikely(!after_cold)
		goto err;
	if (asm_gjmp(ASM_JT, within_cold))
		goto err;
	asm_decsp(); /* Popped by `ASM_JT' */
	sect = current_assembler.a_curr;
	if (sect == &current_assembler.a_sect[SECTION_COLD] &&
	    asm_gjmp(ASM_JMP, after_cold))
		goto err;
	current_assembler.a_curr = &current_assembler.a_sect[SECTION_COLD];
	asm_defsym(within_cold);
	/* We get here only when the local was already bound. */
	constlid = DeeInt_NewU16(lid);
	if unlikely(!constlid)
		goto err;
	temp_id = asm_newconst(constlid);
	Dee_Decref(constlid);
	if unlikely(temp_id < 0)
		goto err;
	if (asm_gpush_const((uint16_t)temp_id))
		goto err; /* Push the ID of the faulting local */
	temp_id = asm_newmodule(DeeModule_GetDeemon());
	if unlikely(temp_id < 0)
		goto err;
	/* Invoke `__roloc from deemon' and let it decide what to do about the error. */
	if (asm_gcall_extern((uint16_t)temp_id, id___roloc, 1))
		goto err;
	if (asm_gpop())
		goto err;
	/* Jump/Switch back to the previous section. */
	if (sect != &current_assembler.a_sect[SECTION_COLD] &&
	    asm_gjmp(ASM_JMP, after_cold))
		goto err;
	current_assembler.a_curr = sect;
	asm_defsym(after_cold);
	return 0;
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_ASM_UTIL_C */
