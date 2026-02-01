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
#ifndef GUARD_DEEMON_COMPILER_OPTIMIZE_CONSTANT_TRAITS_C
#define GUARD_DEEMON_COMPILER_OPTIMIZE_CONSTANT_TRAITS_C 1

#include <deemon/api.h>

#include <deemon/bool.h>               /* DeeBool_Type, Dee_False, Dee_True */
#include <deemon/class.h>              /* DeeClassDescriptor_Type */
#include <deemon/code.h>               /* DeeCode_Type */
#include <deemon/compiler/assembler.h>
#include <deemon/compiler/optimize.h>
#include <deemon/dec.h>                /* DEC_BUILTINID_UNKNOWN, Dec_BuiltinID */
#include <deemon/dict.h>               /* DeeDictObject, DeeDict_*, Dee_dict_item, _DeeDict_GetVirtVTab */
#include <deemon/float.h>              /* DeeFloat_Type */
#include <deemon/hashset.h>            /* DeeHashSetObject, DeeHashSet_* */
#include <deemon/int.h>                /* DeeInt_Type */
#include <deemon/kwds.h>               /* DeeKwds_Type */
#include <deemon/list.h>               /* DeeListObject, DeeList_* */
#include <deemon/module.h>             /* DeeModule_Type */
#include <deemon/none.h>               /* DeeNone_Type */
#include <deemon/object.h>
#include <deemon/objmethod.h>          /* DeeKwObjMethod_Type, DeeObjMethodObject, DeeObjMethod_Type */
#include <deemon/rodict.h>             /* DeeRoDictObject, DeeRoDict_Type, _DeeRoDict_GetRealVTab */
#include <deemon/roset.h>              /* DeeRoSetObject, DeeRoSet_Type, Dee_roset_item */
#include <deemon/string.h>             /* DeeString_Type */
#include <deemon/super.h>              /* DeeSuper* */
#include <deemon/thread.h>             /* DeeThread_Type */
#include <deemon/tuple.h>              /* DeeTuple* */
#include <deemon/util/hash-io.h>       /* Dee_hash_vidx_tovirt, Dee_hash_vidx_virt_lt_real */

#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
#include <deemon/callable.h>  /* DeeCallable_Type */
#include <deemon/cell.h>      /* DeeCell_Type */
#include <deemon/error.h>     /* DeeError_* */
#include <deemon/map.h>       /* DeeMapping_Type, Dee_EmptyMapping */
#include <deemon/numeric.h>   /* DeeNumeric_Type */
#include <deemon/seq.h>       /* DeeIterator_Type, DeeSeq_Type, Dee_EmptySeq */
#include <deemon/set.h>       /* Dee_EmptySet */
#include <deemon/traceback.h> /* DeeTraceback_Type */
#include <deemon/weakref.h>   /* DeeWeakRefAble_Type, DeeWeakRef_Type */
#endif /* CONFIG_EXPERIMENTAL_MMAP_DEC */
/**/

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* size_t */

DECL_BEGIN

PRIVATE DeeTypeObject *tpconst constant_types[] = {
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


/* Expectations regarding what is- and isn't allowed to be a constant seem way too abstruse
 * and overcomplicated, with systems upon systems built upon legacy code. Since the entire
 * compiler needs to be re-written to properly use CONFIG_EXPERIMENTAL_MMAP_DEC anyways,
 * rather than figuring out what was meant to happen here, just emulate legacy behavior
 * for now... */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
PRIVATE DeeObject *tpconst legacy_Dec_BuiltinID[] = {
	Dee_AsObject(&DeeError_Signal),
	Dee_AsObject(&DeeError_Interrupt),
	Dee_AsObject(&DeeError_StopIteration),
	Dee_AsObject(&DeeError_Error),
	Dee_AsObject(&DeeError_AttributeError),
	Dee_AsObject(&DeeError_UnboundAttribute),
	Dee_AsObject(&DeeError_CompilerError),
	Dee_AsObject(&DeeError_ThreadCrash),
	Dee_AsObject(&DeeError_RuntimeError),
	Dee_AsObject(&DeeError_NotImplemented),
	Dee_AsObject(&DeeError_AssertionError),
	Dee_AsObject(&DeeError_UnboundLocal),
	Dee_AsObject(&DeeError_StackOverflow),
	Dee_AsObject(&DeeError_TypeError),
	Dee_AsObject(&DeeError_ValueError),
	Dee_AsObject(&DeeError_ArithmeticError),
	Dee_AsObject(&DeeError_DivideByZero),
	Dee_AsObject(&DeeError_KeyError),
	Dee_AsObject(&DeeError_IndexError),
	Dee_AsObject(&DeeError_UnboundItem),
	Dee_AsObject(&DeeError_SequenceError),
	Dee_AsObject(&DeeError_UnicodeError),
	Dee_AsObject(&DeeError_ReferenceError),
	Dee_AsObject(&DeeError_UnpackError),
	Dee_AsObject(&DeeError_SystemError),
	Dee_AsObject(&DeeError_FSError),
	Dee_AsObject(&DeeError_FileAccessError),
	Dee_AsObject(&DeeError_FileNotFound),
	Dee_AsObject(&DeeError_FileExists),
	Dee_AsObject(&DeeError_FileClosed),
	Dee_AsObject(&DeeError_NoMemory),
	Dee_AsObject(&DeeError_IntegerOverflow),
	Dee_AsObject(&DeeError_UnknownKey),
	Dee_AsObject(&DeeError_ItemNotFound),
	Dee_AsObject(&DeeError_BufferError),
	Dee_AsObject(&DeeObject_Type),
	Dee_AsObject(&DeeSeq_Type),
	Dee_AsObject(&DeeMapping_Type),
	Dee_AsObject(&DeeIterator_Type),
	Dee_AsObject(&DeeCallable_Type),
	Dee_AsObject(&DeeNumeric_Type),
	Dee_AsObject(&DeeWeakRefAble_Type),
	Dee_AsObject(&DeeList_Type),
	Dee_AsObject(&DeeDict_Type),
	Dee_AsObject(&DeeHashSet_Type),
	Dee_AsObject(&DeeCell_Type),
	Dee_False,
	Dee_True,
	Dee_EmptySeq,
	Dee_EmptySet,
	Dee_EmptyMapping,
	Dee_AsObject(&DeeType_Type),
	Dee_AsObject(&DeeTraceback_Type),
	Dee_AsObject(&DeeThread_Type),
	Dee_AsObject(&DeeSuper_Type),
	Dee_AsObject(&DeeString_Type),
	Dee_AsObject(&DeeNone_Type),
	Dee_AsObject(&DeeInt_Type),
	Dee_AsObject(&DeeFloat_Type),
	Dee_AsObject(&DeeModule_Type),
	Dee_AsObject(&DeeCode_Type),
	Dee_AsObject(&DeeTuple_Type),
	Dee_AsObject(&DeeBool_Type),
	Dee_AsObject(&DeeWeakRef_Type),
};

INTERN WUNUSED NONNULL((1)) bool DCALL
legacy_has_Dec_BuiltinID(DeeObject *__restrict obj) {
	size_t i;
	for (i = 0; i < COMPILER_LENOF(legacy_Dec_BuiltinID); ++i) {
		if (legacy_Dec_BuiltinID[i] == obj)
			return true;
	}
	return false;
}
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
#define legacy_has_Dec_BuiltinID(obj) (Dec_BuiltinID(obj) != DEC_BUILTINID_UNKNOWN)
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */


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
	if (legacy_has_Dec_BuiltinID(self))
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
		struct Dee_roset_item *iter, *end;
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
		size_t j;
		DeeRoDictObject *me = (DeeRoDictObject *)self;
		for (j = 0; j < me->rd_vsize; ++j) {
			struct Dee_dict_item *item;
			item = &_DeeRoDict_GetRealVTab(me)[j];
			if (!asm_allowconst(item->di_key))
				goto illegal;
			if (!asm_allowconst(item->di_value))
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
	goto again0;
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
	if (type == &DeeObjMethod_Type || type == &DeeKwObjMethod_Type) {
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
		self = ((DeeObjMethodObject *)self)->om_this;
		goto again0;
#elif defined(CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES)
		/* ObjMethod objects cannot be encoded in in DEC files. */
		goto illegal;
#else /* ... */
		if (!DeeCompiler_Current->cp_options ||
		    !(DeeCompiler_Current->cp_options->co_assembler & ASM_FNODEC))
			goto illegal;
		/* ObjMethod objects cannot be encoded in in DEC files. */
		self = ((DeeObjMethodObject *)self)->om_this;
		goto again0;
#endif /* !... */
	}

	if (type == &DeeSuper_Type) {
		int temp, result;
		result = CONSTEXPR_ALLOWED;
		temp   = allow_constexpr(Dee_AsObject(DeeSuper_TYPE(self)));
		if (temp == CONSTEXPR_ILLEGAL)
			goto illegal;
		if (temp == CONSTEXPR_USECOPY)
			result = CONSTEXPR_USECOPY;
		temp = allow_constexpr(DeeSuper_SELF(self));
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
		for (i = 0; i < me->rd_vsize; ++i) {
			struct Dee_dict_item *item;
			item = &_DeeRoDict_GetRealVTab(me)[i];
			temp = allow_constexpr(item->di_key);
			if (temp == CONSTEXPR_ILLEGAL)
				goto illegal;
			if (temp == CONSTEXPR_USECOPY)
				result = CONSTEXPR_USECOPY;
			temp = allow_constexpr(item->di_value);
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
			for (i = Dee_hash_vidx_tovirt(0);
			     Dee_hash_vidx_virt_lt_real(i, me->d_vsize); ++i) {
				int temp;
				DeeObject *key;
				struct Dee_dict_item *item;
				item = &_DeeDict_GetVirtVTab(me)[i];
				key = item->di_key;
				if (!key)
					continue;
				temp = allow_constexpr(key);
				if (temp == CONSTEXPR_ILLEGAL ||
				    (temp = allow_constexpr(item->di_value)) == CONSTEXPR_ILLEGAL) {
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
	if (legacy_has_Dec_BuiltinID(self))
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
	if (type == Dee_AsObject(&DeeTuple_Type))
		goto yes;
	if (type == Dee_AsObject(&DeeList_Type))
		goto yes;
	if (type == Dee_AsObject(&DeeHashSet_Type))
		goto yes;
	if (type == Dee_AsObject(&DeeDict_Type))
		goto yes;
	if (type == Dee_AsObject(&DeeRoDict_Type))
		goto yes;
	if (type == Dee_AsObject(&DeeRoSet_Type))
		goto yes;
	if (type == Dee_AsObject(&DeeInt_Type))
		goto yes;
	if (type == Dee_AsObject(&DeeString_Type))
		goto yes;
	return false;
yes:
	return true;
}


DECL_END

#endif /* !GUARD_DEEMON_COMPILER_OPTIMIZE_CONSTANT_TRAITS_C */
