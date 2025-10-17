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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_INHERIT_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_INHERIT_C 1

#include <deemon/api.h>

#include <deemon/object.h>
#include <deemon/system-features.h>

/************************************************************************/
/* Operator inheritance (or what's left of it...)                       */
/************************************************************************/

/* Trace self-optimizing operator inheritance. */
#if 1
#define LOG_INHERIT(base, self, what) \
	Dee_DPRINTF("[RT] Inherit `" what "' from %k into %k\n", base, self)
#else
#define LOG_INHERIT(base, self, what) (void)0
#endif

DECL_BEGIN

/* CONFIG: Allow types that are inheriting their constructors to
 *         become GC objects when the object that they're inheriting
 *         from wasn't one.
 *         This is probably not something that should ever happen
 *         and if this were ever to occur, it would probably be
 *         a mistake.
 *         But still: It is something that ?~could~? make sense to allow
 *         In any case: A GC-enabled object providing inheritable constructors
 *                      to non-GC objects is something that's definitely illegal!
 * Anyways: Since the specs only state that an active VAR-flag must be inherited
 *          by all sub-classes, yet remains silent on how the GC type-flag must
 *          behave when it comes to inherited constructors, enabling this option
 *          is the best course of action, considering it opens up the possibility
 *          of further, quite well-defined behavioral options or GC-objects
 *          inheriting their constructor from non-GC sub-classes.
 */
#undef CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS
#define CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS

#define DeeType_Optimize_tp_deepload(self, tp_deepload)       tp_deepload
#define DeeType_Optimize_tp_assign(self, tp_assign)           tp_assign
#define DeeType_Optimize_tp_move_assign(self, tp_move_assign) tp_move_assign

INTERN NONNULL((1)) bool DCALL
DeeType_InheritConstructors(DeeTypeObject *__restrict self) {
	DeeTypeObject *base;
	if (!(self->tp_flags & TP_FINHERITCTOR))
		return false;
	base = self->tp_base;
	if (base == NULL)
		return false;
	DeeType_InheritConstructors(base);
	ASSERT((base->tp_flags & TP_FVARIABLE) ==
	       (self->tp_flags & TP_FVARIABLE));
	LOG_INHERIT(base, self, "operator constructor");
	if (self->tp_flags & TP_FVARIABLE) {
		self->tp_init.tp_var.tp_ctor        = base->tp_init.tp_var.tp_ctor;
		self->tp_init.tp_var.tp_copy_ctor   = base->tp_init.tp_var.tp_copy_ctor;
		self->tp_init.tp_var.tp_deep_ctor   = base->tp_init.tp_var.tp_deep_ctor;
		self->tp_init.tp_var.tp_any_ctor    = base->tp_init.tp_var.tp_any_ctor;
		self->tp_init.tp_var.tp_free        = base->tp_init.tp_var.tp_free;
		self->tp_init.tp_var.tp_any_ctor_kw = base->tp_init.tp_var.tp_any_ctor_kw;
	} else {
#if 0 /* Allocators should not be inheritable! */
		if (base->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS
			ASSERT((base->tp_flags & TP_FGC) == (self->tp_flags & TP_FGC));
#else /* !CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS */
			ASSERTF(!(base->tp_flags & TP_FGC) || (self->tp_flags & TP_FGC),
			        "Non-GC object is inheriting its constructors for a GC-enabled object");
#endif /* CONFIG_ALLOW_INHERIT_TYPE_GC_ALLOCATORS */
			self->tp_init.tp_alloc.tp_alloc = base->tp_init.tp_alloc.tp_alloc;
			self->tp_init.tp_alloc.tp_free  = base->tp_init.tp_alloc.tp_free;
		}
#endif
		self->tp_init.tp_alloc.tp_ctor        = base->tp_init.tp_alloc.tp_ctor;
		self->tp_init.tp_alloc.tp_copy_ctor   = base->tp_init.tp_alloc.tp_copy_ctor;
		self->tp_init.tp_alloc.tp_deep_ctor   = base->tp_init.tp_alloc.tp_deep_ctor;
		self->tp_init.tp_alloc.tp_any_ctor    = base->tp_init.tp_alloc.tp_any_ctor;
		self->tp_init.tp_alloc.tp_any_ctor_kw = base->tp_init.tp_alloc.tp_any_ctor_kw;
	}
	self->tp_init.tp_deepload = DeeType_Optimize_tp_deepload(self, base->tp_init.tp_deepload);

	/* Only inherit assign operators if the class itself doesn't define any already. */
	if (self->tp_init.tp_assign == NULL)
		self->tp_init.tp_assign = DeeType_Optimize_tp_assign(self, base->tp_init.tp_assign);
	if (self->tp_init.tp_move_assign == NULL)
		self->tp_init.tp_move_assign = DeeType_Optimize_tp_move_assign(self, base->tp_init.tp_move_assign);
	return true;
}

INTERN NONNULL((1)) bool DCALL
DeeType_InheritBuffer(DeeTypeObject *__restrict self) {
	struct type_buffer *base_buffer;
	DeeTypeMRO mro;
	DeeTypeObject *base = DeeTypeMRO_Init(&mro, self);
	while ((base = DeeTypeMRO_NextDirectBase(&mro, base)) != NULL) {
		base_buffer = base->tp_buffer;
		if (base_buffer == NULL || !base_buffer->tp_getbuf) {
			if (!DeeType_InheritBuffer(base))
				continue;
			base_buffer = base->tp_buffer;
			ASSERT(base_buffer);
		}
		LOG_INHERIT(base, self, "<BUFFER>");
		if unlikely(self->tp_buffer) {
			memcpy(self->tp_buffer, base_buffer, sizeof(struct type_buffer));
		} else {
			self->tp_buffer = base_buffer;
		}
		return true;
	}
	return false;
}

#undef LOG_INHERIT

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_INHERIT_C */
