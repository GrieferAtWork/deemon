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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_CONSTANT_TRAITS_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_CONSTANT_TRAITS_C 1

#include <deemon/compiler/compiler.h>

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/ast.h>
#include <deemon/compiler/optimize.h>
#include <deemon/dec.h>
#include <deemon/dict.h>
#include <deemon/float.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/list.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>

DECL_BEGIN

PRIVATE DeeTypeObject *constant_types[] = {
	/* Non-object-sequence types that can be encoded using DEC type codes. */
	&DeeInt_Type,
	&DeeFloat_Type,
	&DeeString_Type,
	&DeeNone_Type,
	&DeeBool_Type,
	&DeeClassDescriptor_Type, /* Required for class declarations. */
	&DeeKwds_Type,            /* Required for functions calls with keywords. */
	&DeeCode_Type,            /* Not really, but must count because code objects live in constant slots. */
	&DeeRelInt_Type           /* Required so-as to support constant relocations.
	                           * Objects of this type don't actually show up */
};

struct constexpr_frame {
	struct constexpr_frame *cf_prev; /* [0..1] Previous frame. */
	DeeObject              *cf_obj;  /* [1..1] The object being checked. */
};

#define CONSTEXPR_FRAME_BEGIN(obj)           \
	do {                                     \
		struct constexpr_frame _frame;       \
		_frame.cf_prev   = constexpr_frames; \
		_frame.cf_obj    = (obj);            \
		constexpr_frames = &_frame;          \
		do
#define CONSTEXPR_FRAME_BREAK \
		constexpr_frames = _frame.cf_prev
#define CONSTEXPR_FRAME_END    \
		__WHILE0;              \
		CONSTEXPR_FRAME_BREAK; \
	}	__WHILE0


/* [lock(DeeCompiler_Lock)] */
PRIVATE struct constexpr_frame *constexpr_frames = NULL;
LOCAL bool DCALL constexpr_onstack(DeeObject *__restrict self) {
	struct constexpr_frame *iter;
	for (iter = constexpr_frames; iter;
	     iter = iter->cf_prev) {
		if (iter->cf_obj == self)
			return true;
	}
	return false;
}

/* Check if a given constant value can safely appear in constant variable slots.
 * If this is not the case, `asm_gpush_constexpr' should be used to automatically
 * generate code capable of pushing the given value onto the stack. */
INTERN WUNUSED NONNULL((1)) bool DCALL
asm_allowconst(DeeObject *__restrict self) {
	size_t i;
	DeeTypeObject *type;
	type = Dee_TYPE(self);
	/* Allow some basic types. */
	for (i = 0; i < COMPILER_LENOF(constant_types); ++i) {
		if (type == constant_types[i])
			goto allowed;
	}
	if (Dec_BuiltinID(self) != DEC_BUILTINID_UNKNOWN)
		goto allowed;
	if (type == &DeeTuple_Type) {
		/* Special case: Only allow tuples of constant expressions. */
		for (i = 0; i < DeeTuple_SIZE(self); ++i) {
			if (!asm_allowconst(DeeTuple_GET(self, i)))
				goto illegal;
		}
		goto allowed;
	}
	if (type == &DeeRoSet_Type) {
		/* Special case: Only allow read-only sets of constant expressions. */
		struct roset_item *iter, *end;
		iter = ((DeeRoSetObject *)self)->rs_elem;
		end  = iter + ((DeeRoSetObject *)self)->rs_mask + 1;
		for (; iter < end; ++iter) {
			if (!iter->rsi_key)
				continue;
			if (!asm_allowconst(iter->rsi_key))
				goto illegal;
		}
		goto allowed;
	}
	if (type == &DeeRoDict_Type) {
		/* Special case: Only allow read-only dicts of constant expressions. */
		struct rodict_item *iter, *end;
		iter = ((DeeRoDictObject *)self)->rd_elem;
		end  = iter + ((DeeRoDictObject *)self)->rd_mask + 1;
		for (; iter < end; ++iter) {
			if (!iter->rdi_key)
				continue;
			if (!asm_allowconst(iter->rdi_key))
				goto illegal;
			if (!asm_allowconst(iter->rdi_value))
				goto illegal;
		}
		goto allowed;
	}
illegal:
	return false;
allowed:
	return true;
}

/* Return true if the optimizer is allowed to perform
 * operations on/with a constant instance `self'.
 * @return: * : One of `CONSTEXPR_*' */
INTERN WUNUSED NONNULL((1)) int
(DCALL allow_constexpr)(DeeObject *__restrict self) {
	DeeTypeObject *type;
again0:
	type = Dee_TYPE(self);
	/* Whitelist! */

	/* Allow some basic types. */
	{
		size_t i;
		for (i = 0; i < COMPILER_LENOF(constant_types); ++i) {
			if (type == constant_types[i])
				goto allowed;
		}
	}

	/* Check for special wrapper objects. */
	if (type == &DeeObjMethod_Type) {
		/* ObjMethod objects cannot be encoded in in DEC files. */
		if (!DeeCompiler_Current->cp_options ||
		    !(DeeCompiler_Current->cp_options->co_assembler & ASM_FNODEC))
			goto illegal;
		self = ((DeeObjMethodObject *)self)->om_this;
		goto again0;
	}

	if (type == &DeeSuper_Type) {
		int temp, result;
		result = CONSTEXPR_ALLOWED;
		temp   = allow_constexpr((DeeObject *)DeeSuper_TYPE(self));
		if (temp == CONSTEXPR_ILLEGAL)
			goto illegal;
		if (temp == CONSTEXPR_USECOPY)
			result = CONSTEXPR_USECOPY;
		temp = allow_constexpr((DeeObject *)DeeSuper_SELF(self));
		if (temp == CONSTEXPR_ILLEGAL)
			goto illegal;
		if (temp == CONSTEXPR_USECOPY)
			result = CONSTEXPR_USECOPY;
		return result;
	}

	if (type == &DeeTuple_Type) {
		/* Allow tuples consisting only of other allowed types. */
		int result;
		size_t i, count;
		result = CONSTEXPR_ALLOWED;
		count  = DeeTuple_SIZE(self);
		for (i = 0; i < count; ++i) {
			int temp;
			DeeObject *elem;
			elem = DeeTuple_GET(self, i);
			temp = allow_constexpr(elem);
			if (temp == CONSTEXPR_ILLEGAL)
				goto illegal;
			if (temp == CONSTEXPR_USECOPY)
				result = CONSTEXPR_USECOPY;
		}
		return result;
	}

	if (type == &DeeRoDict_Type) {
		/* Allow read-only dicts consisting only of other allowed types. */
		DeeRoDictObject *me = (DeeRoDictObject *)self;
		int temp, result;
		size_t i;
		result = CONSTEXPR_ALLOWED;
		for (i = 0; i <= me->rd_mask; ++i) {
			if (!me->rd_elem[i].rdi_key)
				continue;
			temp = allow_constexpr(me->rd_elem[i].rdi_key);
			if (temp == CONSTEXPR_ILLEGAL)
				goto illegal;
			if (temp == CONSTEXPR_USECOPY)
				result = CONSTEXPR_USECOPY;
			temp = allow_constexpr(me->rd_elem[i].rdi_value);
			if (temp == CONSTEXPR_ILLEGAL)
				goto illegal;
			if (temp == CONSTEXPR_USECOPY)
				result = CONSTEXPR_USECOPY;
		}
		return result;
	}

	if (type == &DeeRoSet_Type) {
		/* Allow read-only sets consisting only of other allowed types. */
		DeeRoSetObject *me = (DeeRoSetObject *)self;
		int temp, result;
		size_t i;
		result = CONSTEXPR_ALLOWED;
		for (i = 0; i <= me->rs_mask; ++i) {
			if (!me->rs_elem[i].rsi_key)
				continue;
			temp = allow_constexpr(me->rs_elem[i].rsi_key);
			if (temp == CONSTEXPR_ILLEGAL)
				goto illegal;
			if (temp == CONSTEXPR_USECOPY)
				result = CONSTEXPR_USECOPY;
		}
		return result;
	}

	/* Allow list/dict/set, but require them to always be copied. */
	if (type == &DeeList_Type) {
		/* Recursive GC-type. */
		if (constexpr_onstack(self))
			goto usecopy;
		CONSTEXPR_FRAME_BEGIN(self) {
			size_t i;
			DeeListObject *me = (DeeListObject *)self;
			DeeList_LockRead(me);
			for (i = 0; i < DeeList_SIZE(me); ++i) {
				int temp = allow_constexpr(DeeList_GET(me, i));
				if (temp == CONSTEXPR_ILLEGAL) {
					DeeList_LockEndRead(me);
					CONSTEXPR_FRAME_BREAK;
					goto illegal;
				}
			}
			DeeList_LockEndRead(me);
		}
		CONSTEXPR_FRAME_END;
		goto usecopy;
	}

	if (type == &DeeHashSet_Type) {
		/* Recursive GC-type. */
		if (constexpr_onstack(self))
			goto usecopy;
		CONSTEXPR_FRAME_BEGIN(self) {
			size_t i;
			DeeHashSetObject *me = (DeeHashSetObject *)self;
			DeeHashSet_LockRead(me);
			for (i = 0; i <= me->hs_mask; ++i) {
				int temp;
				DeeObject *key = me->hs_elem[i].hsi_key;
				if (!key)
					continue;
				temp = allow_constexpr(key);
				if (temp == CONSTEXPR_ILLEGAL) {
					DeeHashSet_LockEndRead(me);
					CONSTEXPR_FRAME_BREAK;
					goto illegal;
				}
			}
			DeeHashSet_LockEndRead(me);
		}
		CONSTEXPR_FRAME_END;
		goto usecopy;
	}

	if (type == &DeeDict_Type) {
		/* Recursive GC-type. */
		if (constexpr_onstack(self))
			goto usecopy;
		CONSTEXPR_FRAME_BEGIN(self) {
			size_t i;
			DeeDictObject *me = (DeeDictObject *)self;
			DeeDict_LockRead(me);
			for (i = 0; i <= me->d_mask; ++i) {
				int temp;
				DeeObject *key = me->d_elem[i].di_key;
				if (!key)
					continue;
				temp = allow_constexpr(key);
				if (temp == CONSTEXPR_ILLEGAL ||
				    (temp = allow_constexpr(me->d_elem[i].di_value)) == CONSTEXPR_ILLEGAL) {
					DeeDict_LockEndRead(me);
					CONSTEXPR_FRAME_BREAK;
					goto illegal;
				}
			}
			DeeDict_LockEndRead(me);
		}
		CONSTEXPR_FRAME_END;
		goto usecopy;
	}

	/* Last check: There is a small hand full of constant objects that are always allowed. */
	if (Dec_BuiltinID(self) != DEC_BUILTINID_UNKNOWN)
		goto allowed;
illegal:
	return CONSTEXPR_ILLEGAL;
allowed:
	return CONSTEXPR_ALLOWED;
usecopy:
	return CONSTEXPR_USECOPY;
}

/* Check if a given object `type' is a type that implements a cast-constructor. */
INTERN WUNUSED NONNULL((1)) bool
(DCALL has_cast_constructor)(DeeObject *__restrict type) {
	if (type == (DeeObject *)&DeeTuple_Type)
		goto yes;
	if (type == (DeeObject *)&DeeList_Type)
		goto yes;
	if (type == (DeeObject *)&DeeHashSet_Type)
		goto yes;
	if (type == (DeeObject *)&DeeDict_Type)
		goto yes;
	if (type == (DeeObject *)&DeeRoDict_Type)
		goto yes;
	if (type == (DeeObject *)&DeeRoSet_Type)
		goto yes;
	if (type == (DeeObject *)&DeeInt_Type)
		goto yes;
	if (type == (DeeObject *)&DeeString_Type)
		goto yes;
	return false;
yes:
	return true;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_CONSTANT_TRAITS_C */
